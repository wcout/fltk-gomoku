/*
	FLTK Gomoku (c) 2017
*/
#include <config.h>
//#undef FLTK_USE_NANOSVG
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#ifdef FLTK_USE_NANOSVG
#include <FL/Fl_SVG_Image.H>
#endif
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <ctime>

using namespace std;

static const Fl_Color FL_DARK_GRAY = fl_darker( FL_GRAY );
static const Fl_Color FL_LIGHT_YELLOW = fl_lighter( FL_YELLOW );
static const Fl_Color FL_BROWN = fl_rgb_color( 0x67, 0x4d, 0x0f );
static const Fl_Color BOARD_COLOR = fl_rgb_color( 0xdc, 0xb3, 0x5c );
static const Fl_Color BOARD_GRID_COLOR = FL_BLACK;
static const char PLAYER = 1;
static const char COMPUTER = 2;


enum Direction
{
	Horizontal = 1,
	Vertical = 2,
	TopLeftBottomRight = 3,
	TopRightBottomLeft = 4
};

struct PosInfo
{
	int n;
	int f1;
	int f2;
	PosInfo() :
		n( 0 ),
		f1( 0 ),
		f2( 0 )
	{}
	void init() { n = 0; f1 = 0; f2 = 0; }
	bool has5() const { return n == 5; }
	bool wins() const { return has5(); }
	bool canWin() const { return n + f1 + f2 >= 5; }
	bool has4() const { return n == 4 && canWin(); }
	bool has3() const { return n == 3 && canWin(); }
};

struct Eval
{
	PosInfo info[4 + 1];
	void init() { for ( int i = 1; i <= 4; i++ ) info[i].init(); }
	bool has3Fork() const
	{
		return ( ( info[1].has3() || info[1].has4() ) +
		         ( info[2].has3() || info[2].has4() ) +
		         ( info[3].has3() || info[3].has4() ) +
		         ( info[4].has3() || info[4].has4() ) >= 2 );
	}
};

struct Pos
{
	int x;
	int y;
	Eval eval;
	Pos( int x_, int y_ ) :
		x( x_ ),
		y( y_ )
	{}
};

class Board : public Fl_Double_Window
{
typedef Fl_Double_Window Inherited;
public:
	Board() :
		Inherited( 600, 600, "FLTK Gomoku" ),
		_G( 18 ),
		_player( true ),
		_pondering( false ),
		_last_x( 0 ),
		_last_y( 0 ),
		_waiting( false ),
		_games( 0 ),
		_moves( 0 ),
		_player_wins( 0 ),
		_computer_wins( 0 )
	{
		fl_message_title_default( label() );
		clearBoard();
		resizable( this );
	}
	void clearBoard()
	{
		memset( _board, -1, sizeof( _board ) );
		for ( int x = 1; x <= _G + 1; x++ )
			for ( int y = 1; y <= _G + 1; y++ )
				_board[x][y] = 0;
	}
	int xp( int x_ ) const
	{
		int W = w() < h() ? w() : h();
		return W / ( _G + 2 ) * x_;
	}
	int yp( int y_ ) const
	{
		int W = w() < h() ? w() : h();
		return W / ( _G + 2 ) * y_;
	}
	void draw_piece( int color_, int x_, int y_ ) const
	{
#ifdef FLTK_USE_NANOSVG
		#include "go_w_svg.h"
		#include "go_b_svg.h"
		static Fl_SVG_Image *white_piece = 0;
		static Fl_SVG_Image *black_piece = 0;
#endif
		// calc. dimensions
		int x = xp( x_ );
		int y = yp( y_ );
		int rw = xp( 1 );
		int rh = yp( 1 );
		rw -= ceil( (double)rw / 10 );
		rh -= ceil( (double)rh / 10 );
#ifdef FLTK_USE_NANOSVG
		if ( !white_piece )
			white_piece = new Fl_SVG_Image( NULL, Go_White_Piece );
		if ( !black_piece )
		{
			black_piece = new Fl_SVG_Image( NULL, Go_Black_Piece );
		}
		Fl_SVG_Image *piece = color_ == 1 ? white_piece : black_piece;
		piece->resize( rw, rh );
		piece->draw( x - rw/2, y - rh/2 );
#else
		// white or black piece
		fl_color( color_ == 1 ? FL_WHITE : FL_BLACK );
		fl_pie( x - rw/2, y - rh/2, rw, rh, 0, 360 );

		// outline
		fl_color( FL_GRAY );
		fl_arc( x - rw/2, y - rh/2, rw, rh, 0, 360 );
#endif
		// highlight
		bool winning_piece = checkLine( x_, y_, 5 );
		fl_color( FL_DARK_GRAY );
		fl_line_style( FL_SOLID, ceil( (double)rw / 20 ) );
		if ( _last_x == x_ && _last_y == y_ )
			fl_color( FL_CYAN );
		else if ( winning_piece )
			fl_color( color_ == 1 ? FL_GREEN : FL_RED );
		else
		{
			fl_line_style( FL_SOLID, 1 );
#ifdef FLTK_USE_NANOSVG
			return;
#endif
		}
#ifdef FLTK_USE_NANOSVG
		fl_arc( x - rw/2, y - rh/2, rw, rh, 0, 360 );
#else
		fl_arc( x - rw/2 + 1, y - rh/2 + 1, rw - 2, rh - 2, 0, 360 );
#endif
		fl_line_style( 0, 0 );
	}
	void onMove()
	{
		setPiece( _last_x, _last_y, COMPUTER );
	}
	static void cb_move( void *d_ )
	{
		(static_cast<Board *>(d_))->pondering( false );
	}
	bool randomMove( int& x_, int& y_ )
	{
		vector<Pos> move;
		int r( 0 );
		for ( int x = 1; x <= _G + 1; x++ )
		{
			for ( int y = 1; y <= _G + 1; y++ )
			{
				if ( _board[x][y] == 0 )
				{
					if ( x > _G / 3 && x < _G - _G / 3 &&
					     y > _G / 3 && y < _G - _G / 3 )
					{
						move.insert( move.begin(), Pos( x, y ) );
						r++;
					}
					else
						move.push_back( Pos( x, y ) );
				}
			}
		}
		if ( move.empty() )
			return false;
		if ( !r )
			r = move.size();
		r = random() % r;
		x_ = move[r].x;
		y_ = move[r].y;
		return true;
	}
	void makeMove()
	{
		_pondering = true;
		fl_cursor( FL_CURSOR_WAIT );
		Fl::add_timeout( 1.0, cb_move, this );
		_last_x = 0;
		_last_y = 0;
		if ( !winningMove( _last_x, _last_y ) && !defensiveMove( _last_x, _last_y ) )
		{
			randomMove( _last_x, _last_y );
			cout << "randomMove at " << _last_x << "/" << _last_y << endl;
		}
		while ( _pondering )
			Fl::check();
		fl_cursor( FL_CURSOR_DEFAULT );
		_pondering = false;
		Fl::remove_timeout( cb_move, this );
		onMove();
	}
	int count( int x_, int y_,  int dx_, int dy_, int& free1_, int& free2_ ) const
	{
		free1_ = 0;
		free2_ = 0;
		int c = _board[x_][y_];
		if ( c <= 0 )
			return 0;
		int n( 1 );
		int x( x_ );
		int y( y_ );
		x += dx_;
		y += dy_;
		while ( _board[x][y] == c )
		{
			n++;
			x += dx_;
			y += dy_;
		}
		while ( _board[x][y] == 0 )
		{
			free1_++;
			x += dx_;
			y += dy_;
			if ( free1_ + free2_ + n >= 5 ) break;
		}
		x = x_;
		y = y_;
		dx_ = -dx_;
		dy_ = -dy_;
		x += dx_;
		y += dy_;
		while ( _board[x][y] == c )
		{
			n++;
			x += dx_;
			y += dy_;
		}
		while ( _board[x][y] == 0 )
		{
			free2_++;
			x += dx_;
			y += dy_;
			if ( free1_ + free2_ + n >= 5 ) break;
		}
		return n;
	}
	int countX( int x_, int y_ ) const
	{
		int f1, f2;
		return count( x_, y_, 1, 0, f1, f2 );
	}
	int countY( int x_, int y_ ) const
	{
		int f1, f2;
		return count( x_, y_, 0, 1, f1, f2 );
	}
	int countXYLeft( int x_, int y_ ) const
	{
		int f1, f2;
		return count( x_, y_, -1, -1, f1, f2 );
	}
	int countXYRight( int x_, int y_ ) const
	{
		int f1, f2;
		return count( x_, y_, 1, -1, f1, f2 );
	}
	void countPos( int x_, int y_, Eval& pos_ ) const
	{
		pos_.info[1].n = count( x_, y_,  1,  0, pos_.info[1].f1, pos_.info[1].f2 );
		pos_.info[2].n = count( x_, y_,  1,  1, pos_.info[2].f1, pos_.info[2].f2 );
		pos_.info[3].n = count( x_, y_, -1, -1, pos_.info[3].f1, pos_.info[3].f2 );
		pos_.info[4].n = count( x_, y_,  1, -1, pos_.info[4].f1, pos_.info[5].f2 );
	}
	bool checkLine( int x_, int y_, int n_ ) const
	{
		int n = countX( x_, y_ );
		if ( n != n_ )
			n = countY( x_, y_ );
		if ( n != n_ )
			n = countXYLeft( x_, y_ );
		if ( n != n_ )
			n = countXYRight( x_, y_ );
		return n == n_;
	}
	void onResized()
	{
		size( xp( _G + 2 ), yp( _G + 2 ) );
	}
	bool tryWin( int x_, int y_, int who_ = COMPUTER )
	{
		if ( _board[x_][y_] != 0 )
			return false;
		_board[x_][y_] = who_;
		bool win = checkLine( x_, y_, 5 );
		_board[x_][y_] = 0;
		return win;
	}
	bool try4( int x_, int y_, int who_ = COMPUTER )
	{
		if ( _board[x_][y_] != 0 )
			return false;
		_board[x_][y_] = who_;
		bool win = checkLine( x_, y_, 4 );
		_board[x_][y_] = 0;
		return win;
	}
	bool try3( int x_, int y_, int who_ = COMPUTER )
	{
		if ( _board[x_][y_] != 0 )
			return false;
		_board[x_][y_] = who_;
		bool win = checkLine( x_, y_, 3 );
		_board[x_][y_] = 0;
		return win;
	}
	bool eval( int x_, int y_, int who_ = COMPUTER )
	{
		if ( _board[x_][y_] != 0 )
			return false;
		_board[x_][y_] = who_;
		Eval& p = _eval[ x_ ][ y_ ];
		p.init();
		countPos( x_, y_, p );
		_board[x_][y_] = 0;
		return p.has3Fork();
	}
	bool winningMove( int& x_, int& y_ )
	{
		for ( int x = 1; x <= _G + 1; x++ )
			for ( int y = 1; y <= _G + 1; y++ )
				if ( tryWin( x, y ) )
				{
					cout << "winnungMove tryWin at " << x << "/" << y << endl;
					x_ = x;
					y_ = y;
					return true;
				}
		for ( int x = 1; x <= _G + 1; x++ )
			for ( int y = 1; y <= _G + 1; y++ )
				if ( tryWin( x, y, PLAYER ) )
				{
					cout << "winnungMove tryWin PLAYER at " << x << "/" << y << endl;
					x_ = x;
					y_ = y;
					return true;
				}

		for ( int x = 1; x <= _G + 1; x++ )
			for ( int y = 1; y <= _G + 1; y++ )
				if ( try4( x, y ) )
				{
					x_ = x;
					y_ = y;
					cout << "winnungMove try4 at " << x << "/" << y << endl;
					return true;
				}
		for ( int x = 1; x <= _G + 1; x++ )
			for ( int y = 1; y <= _G + 1; y++ )
				if ( try4( x, y, PLAYER ) )
				{
					cout << "winnungMove try4 PLAYER at " << x << "/" << y << endl;
					x_ = x;
					y_ = y;
					return true;
				}

		// test 3-fork
		for ( int x = 1; x <= _G + 1; x++ )
		{
			for ( int y = 1; y <= _G + 1; y++ )
			{
				if ( eval( x, y ) )
				{
					cout << "winnungMove eval at " << x << "/" << y << endl;
					x_ = x;
					y_ = y;
					return true;
				}
			}
		}

		for ( int x = 1; x <= _G + 1; x++ )
		{
			for ( int y = 1; y <= _G + 1; y++ )
			{
				if ( eval( x, y, PLAYER ) )
				{
					cout << "winnungMove eval PLAYER at " << x << "/" << y << endl;
					x_ = x;
					y_ = y;
					return true;
				}
			}
		}

		return false;
	}
	bool defensiveMove( int& x_, int& y_ )
	{
		for ( int x = 1; x <= _G + 1; x++ )
			for ( int y = 1; y <= _G + 1; y++ )
				if ( try3( x, y, PLAYER ) )
				{
					cout << "defensiveMove try3 at " << x << "/" << y << endl;
					x_ = x;
					y_ = y;
					return true;
				}

		return false;
	}
	static void cb_resized( void *d_ )
	{
		static_cast<Board *>( d_ )->onResized();
	}
	virtual void resize( int x_, int y_, int w_, int h_ )
	{
		Fl::remove_timeout( cb_resized, this );
		Inherited::resize( x_, y_, w_, h_ );
		if ( w_ != h_ )
			Fl::add_timeout( 0.2, cb_resized, this );
	}
	void setPiece( int x_, int y_, int who_ )
	{
		_moves++;
		bool adraw = ( x_ ==  0 || y_ == 0 );
		if ( !adraw )
		{
			_board[ x_][ y_ ] = who_;
			_last_x = x_;
			_last_y = y_;
			redraw();
		}
		if ( _player )
		{
			int x, y;
			adraw = !randomMove( x, y );
		}
		if ( adraw || checkLine( x_, y_, 5 ) )
		{
			_games++;
			bool player = _board[x_][y_] == PLAYER;
			if ( !adraw && player ) _player_wins++;
			else if ( !adraw ) _computer_wins++;
			ostringstream stat;
			stat << endl << endl << _games << " games - " <<
				_player_wins << " : " << _computer_wins <<
				endl << endl << "( average moves: " << _moves / _games << ")";
			wait( 0.5 );
			fl_alert( adraw ? "No more moves!\n\nGame ends adraw.%s" :
				player ? "You managed to win!%s" : "FLTK wins!%s", stat.str().c_str() );
			clearBoard();
			redraw();
		}
		_player = !_player;
		if ( !_player )
		{
			makeMove();
		}
	}
	void onDelay()
	{
		_waiting = false;
	}
	static void cb_delay( void *d_ )
	{
		static_cast<Board *>( d_ )->onDelay();
	}
	void wait( double delay_ )
	{
		Fl::remove_timeout( cb_delay, this );
		Fl::add_timeout( delay_, cb_delay, this );
		_waiting = true;
		while ( _waiting )
			Fl::check();
		Fl::remove_timeout( cb_delay, this );
		_waiting = false;
	}
	int handle( int e_ )
	{
		if ( e_ == FL_PUSH && _player )
		{
			int x = ( Fl::event_x() + xp( 1 ) / 2 ) / xp( 1 );
			int y = ( Fl::event_y() + yp( 1 ) / 2 ) / yp( 1 );
			setPiece( x, y, PLAYER );
			return 1;
		}
#if 1
		if ( e_ == FL_MOVE && _player )
		{
			_last_x = 0;
			_last_y = 0;
			redraw();
		}
#endif
		return Inherited::handle( e_ );
	}
private:
	virtual void draw()
	{
		// draw board
		fl_rectf( 0, 0, w(), h(), FL_DARK_GRAY );
//		fl_rectf( 0, 0, xp(_G+2), yp(_G+2), FL_LIGHT_YELLOW );
		fl_rectf( 0, 0, xp(_G+2), yp(_G+2), BOARD_COLOR );

		// draw grid
		fl_rect( 0, 0, xp(_G+2), yp(_G+2), BOARD_GRID_COLOR );
		for ( int x = 0; x <= _G; x++ )
			fl_line( xp(x + 1 ), yp( 1 ), xp( x + 1 ), yp( 1  + _G ) );

		for ( int y = 0; y <= _G; y++ )
			fl_line( xp( 1 ), yp( y + 1), xp( 1 + _G ), yp( y + 1 ) );

		// draw center
		int r = ceil( (double)xp( 1 ) / 12 );
		fl_color( FL_BLACK );
		fl_circle( xp( _G / 2 + 1 ), yp( _G / 2 + 1), r );
		fl_circle( xp( _G / 4 + 1 ), yp( _G / 4 + 1), r );
		fl_circle( xp( _G / 4 + 1 ), yp( _G / 2 + 1), r );
		fl_circle( xp( _G / 2 + 1 ), yp( _G / 4 + 1), r );
		fl_circle( xp( _G - _G / 4 + 1 ), yp( _G / 4 + 1), r );
		fl_circle( xp( _G - _G / 4 + 1 ), yp( _G / 2 + 1), r );
		fl_circle( xp(  _G / 4 + 1 ), yp( _G - _G / 4 + 1), r );
		fl_circle( xp(  _G / 2 + 1 ), yp( _G - _G / 4 + 1), r );
		fl_circle( xp( _G - _G / 4 + 1 ), yp( _G - _G / 4 + 1), r );

		// draw pieces
		for ( int x = 1; x <= _G + 1; x++ )
			for ( int y = 1; y <= _G + 1; y++ )
				if ( _board[x][y] )
					draw_piece( _board[x][y], x, y );
	}
	void pondering( bool pondering_ ) { _pondering = pondering_; }
private:
	int _G;
	char _board[24][24];
	Eval _eval[24][24];
	bool _player;
	bool _pondering;
	int _last_x;
	int _last_y;
	bool _waiting;
	int _games;
	int _moves;
	int _player_wins;
	int _computer_wins;
};

int main()
{
	Fl::scheme( "gtk+" );
	Fl::get_system_colors();
	srand( time( 0 ) );
	Board g;
	g.show();
	return Fl::run();
}
