/*
	FLTK Gomoku (c) 2017
*/
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <cstdlib>
#include <cmath>

static const Fl_Color FL_DARK_GRAY = fl_darker( FL_GRAY );
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
		_wait( false )
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
		int x = xp( x_ );
		int y = yp( y_ );
		int rw = xp( 1 );
		int rh = yp( 1 );
		rw -= ceil( (double)rw / 10 );
		rh -= ceil( (double)rh / 10 );
		fl_color( color_ == 1 ? FL_WHITE : FL_BLACK );
		fl_pie( x - rw/2, y - rh/2, rw, rh, 0, 360 );
		fl_color( FL_GRAY );
		fl_arc( x - rw/2, y - rh/2, rw, rh, 0, 360 );
		bool winning_piece = checkLine( x_, y_, 5 );
		fl_color( FL_DARK_GRAY );
		fl_line_style( FL_SOLID, ceil( (double)rw / 20 ) );
		if ( _last_x == x_ && _last_y == y_ )
			fl_color( FL_CYAN );
		else if ( winning_piece )
			fl_color( color_ == 1 ? FL_GREEN : FL_RED );
		else
			fl_line_style( FL_SOLID, 1 );
		fl_arc( x - rw/2 + 1, y - rh/2 + 1, rw - 2, rh - 2, 0, 360 );
		fl_line_style( 0, 0 );
	}
	void onMove()
	{
		// stub
		int G = _G / 2;
		while ( 1 )
		{
			int x = _last_x ? _last_x : random() % ( G + 1 ) + G / 2 + 1;
			int y = _last_y ? _last_y : random() % ( G + 1 ) + G / 2 + 1;
			if ( _board[x][y] ) continue;
			setPiece( x, y, COMPUTER );
			break;
		}
	}
	static void cb_move( void *d_ )
	{
		(static_cast<Board *>(d_))->pondering( false );
	}
	void makeMove()
	{
		_pondering = true;
		fl_cursor( FL_CURSOR_WAIT );
		Fl::add_timeout( 1.0, cb_move, this );
		_last_x = 0;
		_last_y = 0;
		while ( _pondering )
		{
			int x, y;
			if ( winningMove( x, y ) || defensiveMove( x, y ) )
			{
				_last_x = x;
				_last_y = y;
			}
			Fl::check();
		}
		fl_cursor( FL_CURSOR_DEFAULT );
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
					x_ = x;
					y_ = y;
					return true;
				}
		for ( int x = 1; x <= _G + 1; x++ )
			for ( int y = 1; y <= _G + 1; y++ )
				if ( tryWin( x, y, PLAYER ) )
				{
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
					return true;
				}
		for ( int x = 1; x <= _G + 1; x++ )
			for ( int y = 1; y <= _G + 1; y++ )
				if ( try4( x, y, PLAYER ) )
				{
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
		if ( x_ < 1 || x_ > _G + 1 )
			return;
		if ( y_ < 1 || y_ > _G + 1 )
			return;
		_board[ x_][ y_ ] = who_;
		_last_x = x_;
		_last_y = y_;
		redraw();
		if ( checkLine( x_, y_, 5 ) )
		{
			wait( 0.5 );
			fl_alert( _board[x_][y_] == PLAYER ? "You win!" : "I win!" );
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
		_wait = false;
	}
	static void cb_delay( void *d_ )
	{
		static_cast<Board *>( d_ )->onDelay();
	}
	void wait( double delay_ )
	{
		Fl::remove_timeout( cb_delay, this );
		Fl::add_timeout( delay_, cb_delay, this );
		_wait = true;
		while ( _wait )
			Fl::check();
		Fl::remove_timeout( cb_delay, this );
		_wait = false;
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
		return Inherited::handle( e_ );
	}
private:
	virtual void draw()
	{
		// draw board
		fl_rectf( 0, 0, w(), h(), FL_DARK_GRAY );
		fl_rectf( 0, 0, xp(_G+2), yp(_G+2), fl_lighter( FL_YELLOW ) );

		// draw grid
		fl_rect( 0, 0, xp(_G+2), yp(_G+2), FL_DARK_GRAY );
		for ( int x = 0; x <= _G; x++ )
			fl_line( xp(x + 1 ), yp( 1 ), xp( x + 1 ), yp( 1  + _G ) );

		for ( int y = 0; y <= _G; y++ )
			fl_line( xp( 1 ), yp( y + 1), xp( 1 + _G ), yp( y + 1 ) );

		// draw center
		fl_color( FL_BLACK );
		fl_circle( xp( _G / 2 + 1 ), yp( _G / 2 + 1), ceil( (double)xp( 1 ) / 8 ) );

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
	bool _wait;
};

int main()
{
	Fl::scheme( "gtk+" );
	Board g;
	g.show();
	return Fl::run();
}
