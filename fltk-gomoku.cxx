/*
 FLTK Gomoku.

 (c) 2017 wcout wcout<gmx.net>

 A minimal implementation of the "5 in a row" game.

 This code is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This code is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY;  without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details:
 http://www.gnu.org/licenses/.

*/
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#ifdef FLTK_USE_NANOSVG
#include <FL/Fl_SVG_Image.H>
#endif
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Tiled_Image.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <ctime>

using namespace std;

static const Fl_Color FL_DARK_GRAY = fl_darker( FL_GRAY );
//static const Fl_Color FL_LIGHT_YELLOW = fl_lighter( FL_YELLOW );
//static const Fl_Color FL_BROWN = fl_rgb_color( 0x67, 0x4d, 0x0f );
static const Fl_Color BOARD_COLOR = fl_rgb_color( 0xdc, 0xb3, 0x5c );
static const Fl_Color BOARD_GRID_COLOR = FL_BLACK;
static const char PLAYER = 1;
static const char COMPUTER = 2;

//-------------------------------------------------------------------------------
struct PosInfo
//-------------------------------------------------------------------------------
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
	bool canWin() const { return n + f1 + f2 >= 5 && ( f1 && f2 ); }
	bool has4() const { return n == 4 && canWin(); }
	bool has3() const { return n == 3 && canWin(); }
};

typedef char Board[24][24];

//-------------------------------------------------------------------------------
struct Eval
//-------------------------------------------------------------------------------
{
	PosInfo info[4 + 1];
	void init() { for ( int i = 1; i <= 4; i++ )
			      info[i].init();
	}
	bool wins() const
	{
		return info[1].wins() || info[2].wins() || info[3].wins() || info[4].wins();
	}
	bool has4() const
	{
		return info[1].has4() || info[2].has4() || info[3].has4() || info[4].has4();
	}
	bool has3() const
	{
		return info[1].has3() || info[2].has3() || info[3].has3() || info[4].has3();
	}
	bool has3Fork() const
	{
		return ( info[1].has3() || info[1].has4() ) +
		       ( info[2].has3() || info[2].has4() ) +
		       ( info[3].has3() || info[3].has4() ) +
		       ( info[4].has3() || info[4].has4() ) >= 2;
	}
};

//-------------------------------------------------------------------------------
struct Move
//-------------------------------------------------------------------------------
{
	int x;
	int y;
	int value;
	Eval eval;
	Move( int x_ = 0, int y_ = 0, int value_ = 0 ) :
		x( x_ ),
		y( y_ ),
		value( value_ )
	{}
	void init( int x_ = 0, int y_ = 0, int value_ = 0 )
	{
		x = x_;
		y = y_;
		value = value_;
		eval.init();
	}
};

static int count( int x_, int y_, int dx_, int dy_, PosInfo &info_,
                  const Board &board_ )
//-------------------------------------------------------------------------------
{
	info_.init();
	int c = board_[x_][y_];
	if ( c <= 0 )
		return 0;
	info_.n = 1;
	int x( x_ );
	int y( y_ );
	x += dx_;
	y += dy_;
	while ( board_[x][y] == c )
	{
		info_.n++;
		x += dx_;
		y += dy_;
	}
	while ( board_[x][y] == 0 )
	{
		info_.f1++;
		x += dx_;
		y += dy_;
		if ( info_.f1 + info_.f2 + info_.n >= 5 )
			break;
	}
	x = x_;
	y = y_;
	dx_ = -dx_;
	dy_ = -dy_;
	x += dx_;
	y += dy_;
	while ( board_[x][y] == c )
	{
		info_.n++;
		x += dx_;
		y += dy_;
	}
	while ( board_[x][y] == 0 )
	{
		info_.f2++;
		x += dx_;
		y += dy_;
		if ( info_.f1 + info_.f2 + info_.n >= 5 )
			break;
	}
	return info_.n;
} // count

//-------------------------------------------------------------------------------
struct Args
//-------------------------------------------------------------------------------
{
	string bgImageFile;
	string boardFile;
};

//-------------------------------------------------------------------------------
class Gomoku : public Fl_Double_Window
//-------------------------------------------------------------------------------
{
#define DBG(a) { if ( _debug ) std::cout << a << endl; }
	typedef Fl_Double_Window Inherited;
public:
	Gomoku( int argc_ = 0, char *argv_[] = 0 );
	~Gomoku();
	void clearBoard();
	void loadBoardFromFile( const string& f_ );
	void dumpBoard() const;
	void makeMove();
	void setPiece( const Move& move_, int who_ );
	void wait( double delay_ );
	virtual int handle( int e_ );
	virtual void draw();
protected:
	void drawBoard( bool bg_ = false ) const;
	void drawPiece( int color_, int x_, int y_ ) const;
	void nextMove();
	void setIcon();
	void parseArgs( int argc_, char *argv_[] );
	bool waitKey();
private:
	int xp( int x_ ) const;
	int yp( int y_ ) const;
	void onMove();
	static void cb_move( void *d_ );
	static void cb_ponder( void *d_ );
	int count( int x_, int y_, int dx_, int dy_, PosInfo& info_ ) const;
	void countPos( int x_, int y_, Eval &pos_, const Board &board_ ) const;
	void countPos( int x_, int y_, Eval &pos_ ) const;
	bool checkWin( int x_, int y_ ) const;
	bool findMove( Move& move_ ) const;
	bool randomMove( Move& move_ ) const;
	int eval( Move& move_ ) const;
	void onDelay();
	static void cb_delay( void *d_ );
	void pondering( bool pondering_ ) { _pondering = pondering_; }
private:
	int _G;
	Board _board;
	Eval _eval[24][24];
	bool _player;
	bool _first_player;
	bool _pondering;
	Move _move;
	bool _waiting;
	int _games;
	int _moves;
	int _player_wins;
	int _computer_wins;
	bool _wait_click;
	bool _replay;
	bool _abortReplay;
	vector<Move> _history;
	vector<Move> _replayMoves;
	int _debug; // Note: using int instead of bool for signature of preferences
	int _alert; // Nite: as above
	Fl_Preferences *_cfg;
	string _message;
	string _dmsg;
	Args _args;
};

Gomoku::Gomoku( int argc_/* = 0*/, char *argv_[]/* = 0*/ ) :
	Inherited( 600, 600, "FLTK Gomoku (\"5 in a row\")" ),
	_G( 18 ),
	_player( true ),
	_first_player( _player ),
	_pondering( false ),
	_waiting( false ),
	_games( 0 ),
	_moves( 0 ),
	_player_wins( 0 ),
	_computer_wins( 0 ),
	_wait_click( false ),
	_replay( false ),
	_abortReplay( false ),
	_debug( false ),
	_alert( false )
//-------------------------------------------------------------------------------
{
	parseArgs( argc_, argv_ );
	Fl_Box *bg = new Fl_Box( 0, 0, w(), h() );
	end();
	bg->box( FL_FLAT_BOX );

	_cfg = new Fl_Preferences( Fl_Preferences::USER, "CG", "fltk-gomoku" );
	char *bg_image;
	_cfg->get( "bg_image", bg_image, "bg.gif" );
	if ( _args.bgImageFile.size() )
		bg_image = strdup( _args.bgImageFile.c_str() ); // overrule by cmd line arg
	Fl_Image *bg_tile = Fl_Shared_Image::get( bg_image );
	if ( bg_tile && bg_tile->w() > 0 && bg_tile->h() > 0 )
		bg->image( new Fl_Tiled_Image( bg_tile ) );
	free( bg_image );

	int W, X, Y;
	_cfg->get( "games", _games, 0 );
	_cfg->get( "moves", _moves, 0 );
	_cfg->get( "player_wins", _player_wins, 0 );
	_cfg->get( "computer_wins", _computer_wins, 0 );

	_cfg->get( "W", W, w() );
	_cfg->get( "X", X, x() );
	_cfg->get( "Y", Y, y() );

	_cfg->get( "debug", _debug, _debug );
	_cfg->get( "alert", _alert, _alert );

	fl_message_title_default( label() );
	clearBoard();
	resizable( this );
	size_range( ( _G + 2 ) * 10, ( _G + 2 ) * 10, 0, 0, ( _G + 2 ), ( _G + 2 ), 1 );

	resize( X, Y, W, W );
	setIcon();
	show();
	nextMove();
}

void Gomoku::nextMove()
//-------------------------------------------------------------------------------
{
	if ( _history.empty() )
		_first_player = _player;
	if ( _replay )
	{
		if ( !waitKey() )
			return;
		fl_cursor( FL_CURSOR_WAIT );
		_move.init();
		if ( !_abortReplay && _history.size() < _replayMoves.size() )
		{
			_move = _replayMoves[ _history.size() ];
		}
		Fl::add_timeout( .1, cb_move, this );
		return;
	}
	if ( _player )
	{
		_message = "Your move";
		fl_cursor( FL_CURSOR_HAND );
	}
	else
	{
		_message = "Thinking...";
		makeMove();
	}
}

Gomoku::~Gomoku()
//-------------------------------------------------------------------------------
{
	_cfg->set( "W", w() );
	_cfg->set( "X", x() );
	_cfg->set( "Y", y() );

	_cfg->set( "debug", _debug );
	_cfg->set( "alert", _alert );
	_cfg->flush();
}

void Gomoku::parseArgs( int argc_, char *argv_[] )
//-------------------------------------------------------------------------------
{
	for ( int i = 1; i < argc_; i++ )
	{
		string arg = argv_[i];
		if ( arg == "-b" )
		{
			i++;
			if ( i < argc_ )
				_args.boardFile = argv_[i];
		}
		else if ( arg[0] != '-' )
		{
			_args.bgImageFile = argv_[i];
		}
	}
}

void Gomoku::clearBoard()
//-------------------------------------------------------------------------------
{
	memset( _board, -1, sizeof( _board ) );
	for ( int x = 1; x <= _G + 1; x++ )
		for ( int y = 1; y <= _G + 1; y++ )
			_board[x][y] = 0;
	if ( _replayMoves.empty() )
		_replayMoves = _history;
	_history.clear();
	if ( _args.boardFile.size() )
	loadBoardFromFile( _args.boardFile );
}

void Gomoku::loadBoardFromFile( const string& f_ )
//-------------------------------------------------------------------------------
{
	ifstream ifs( f_.c_str() );
	if ( !ifs.is_open() )
		return;
	int y = 0;
	int last_moved = 0;
	Move last_move;
	string line;
	while ( getline( ifs, line ) )
	{
		if ( y >= 1 && y <= _G + 1 )
		{
			for ( int x = 1; x <= _G + 1; x++ )
			{
				char c = line[ x * 2];
				if ( c == 'p' || c == 'P' )
					_board[x][y] = PLAYER;
				if ( c == 'c' || c == 'C' )
					_board[x][y] = COMPUTER;
				if ( c == 'P' || c == 'C' )
				{
					last_move.init( x, y );
				}
				if ( c == 'P' )
					last_moved = PLAYER;
				if ( c == 'C' )
					last_moved = COMPUTER;
			}
		}
		y++;
	}
	_player = last_moved == COMPUTER;
	_move = last_move;
}

void Gomoku::dumpBoard() const
//-------------------------------------------------------------------------------
{
	cout << " ";
	for ( int x = 1; x <= _G + 1; x++ )
		cout << " " << (char)('a' + x - 1);
	cout << endl;
	for ( int y = 1; y <= _G + 1; y++ )
	{
		cout << (char)('A' + y - 1) << " ";
		for ( int x = 1; x <= _G + 1; x++ )
		{
			int who = _board[x][y];
			bool last = x == _move.x && y == _move.y;
			char player = last ? 'P' : 'p';
			char computer = last ? 'C' : 'c';
			cout << ( who ? ( who == PLAYER ? player  : computer ) : '.' ) << ' ';
		}
		cout << endl;
	}
	cout << endl;
}

int Gomoku::xp( int x_ ) const
//-------------------------------------------------------------------------------
{
	int W = w() < h() ? w() : h();
	return W / ( _G + 2 ) * x_;
}

int Gomoku::yp( int y_ ) const
//-------------------------------------------------------------------------------
{
	int W = w() < h() ? w() : h();
	return W / ( _G + 2 ) * y_;
}

void Gomoku::drawPiece( int color_, int x_, int y_ ) const
//-------------------------------------------------------------------------------
{
#ifdef FLTK_USE_NANOSVG
	#include "go_w_svg.h"
	#include "go_b_svg.h"
	#include "last_piece.h"
	#include "win_w.h"
	#include "win_b.h"
	static Fl_SVG_Image *svg_white_piece = 0;
	static Fl_SVG_Image *svg_black_piece = 0;
	static Fl_SVG_Image *svg_last_piece = 0;
	static Fl_SVG_Image *svg_win_white_piece = 0;
	static Fl_SVG_Image *svg_win_black_piece = 0;
#endif
	// calc. dimensions
	int x = xp( x_ );
	int y = yp( y_ );
	int rw = xp( 1 );
	int rh = yp( 1 );
	rw -= ceil( (double)rw / 10 );
	rh -= ceil( (double)rh / 10 );
#ifdef FLTK_USE_NANOSVG
	if ( !svg_white_piece )
		svg_white_piece = new Fl_SVG_Image( NULL, Go_White_Piece );
	if ( !svg_black_piece )
		svg_black_piece = new Fl_SVG_Image( NULL, Go_Black_Piece );
	if ( !svg_last_piece )
		svg_last_piece = new Fl_SVG_Image( NULL, Last_Piece );
	if ( !svg_win_white_piece )
		svg_win_white_piece = new Fl_SVG_Image( NULL, Win_White_Piece );
	if ( !svg_win_black_piece )
		svg_win_black_piece = new Fl_SVG_Image( NULL, Win_Black_Piece );
	Fl_SVG_Image *svg_piece = color_ == 1 ? svg_white_piece : svg_black_piece;
	svg_piece->resize( rw, rh );
	svg_piece->draw( x - rw / 2, y - rh / 2 );
#else
	// white or black piece
	fl_color( color_ == 1 ? FL_WHITE : FL_BLACK );
	fl_pie( x - rw / 2, y - rh / 2, rw, rh, 0, 360 );

	// outline
	fl_color( FL_DARK_GRAY );
	fl_arc( x - rw / 2, y - rh / 2, rw, rh, 0, 360 );
#endif

	// highlight piece(s)
	bool winning_piece = checkWin( x_, y_ );
	bool last_piece = _move.x == x_ && _move.y == y_;
	if ( last_piece || winning_piece )
	{
#ifdef FLTK_USE_NANOSVG
		Fl_SVG_Image *svg_hi_piece = last_piece ? svg_last_piece :
		                             color_ == 1 ? svg_win_white_piece :
		                             svg_win_black_piece;
		svg_hi_piece->resize( rw, rh );
		svg_hi_piece->draw( x - rw / 2, y - rh / 2 );
#else
		fl_line_style( FL_SOLID, ceil( (double)rw / 20 ) );
		fl_color( last_piece ? FL_CYAN : color_ == 1 ? FL_GREEN : FL_RED );
		fl_arc( x - rw / 2, y - rh / 2, rw, rh, 0, 360 );
		fl_line_style( 0, 0 );
#endif
	}
} // drawPiece

void Gomoku::onMove()
//-------------------------------------------------------------------------------
{
	setPiece( _move, _player ? PLAYER : COMPUTER );
}

/*static*/
void Gomoku::cb_ponder( void *d_ )
//-------------------------------------------------------------------------------
{
	(static_cast<Gomoku *>(d_))->pondering( false );
}

/*static*/
void Gomoku::cb_move( void *d_ )
//-------------------------------------------------------------------------------
{
	(static_cast<Gomoku *>(d_))->onMove();
}

bool Gomoku::randomMove( Move& move_ ) const
//-------------------------------------------------------------------------------
{
	vector<Move> moves;
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
					moves.insert( moves.begin(), Move( x, y ) );
					r++;
				}
				else
					moves.push_back( Move( x, y ) );
			}
		}
	}
	if ( moves.empty() )
		return false;
	if ( !r )
		r = moves.size();
	r = random() % r;
	move_ = moves[r];
	return true;
} // randomMove

void Gomoku::makeMove()
//-------------------------------------------------------------------------------
{
	_pondering = true;
	fl_cursor( FL_CURSOR_WAIT );
	Fl::add_timeout( 1.0, cb_ponder, this );
	Move move;
	if ( !findMove( move ) )
	{
		randomMove( move );
		DBG( "randomMove at " << move.x << "/" << move.y );
	}
	while ( _pondering )
		Fl::check();
	fl_cursor( FL_CURSOR_DEFAULT );
	_pondering = false;
	Fl::remove_timeout( cb_ponder, this );
	_move = move;
	onMove();
}

int Gomoku::count( int x_, int y_, int dx_, int dy_, PosInfo& info_ ) const
//-------------------------------------------------------------------------------
{
	return ::count( x_, y_, dx_, dy_, info_, _board );
}

void Gomoku::countPos( int x_, int y_, Eval &pos_, const Board &board_ ) const
//-------------------------------------------------------------------------------
{
	::count( x_, y_,  1,  0, pos_.info[1], board_ );
	::count( x_, y_,  0,  1, pos_.info[2], board_ );
	::count( x_, y_, -1, -1, pos_.info[3], board_ );
	::count( x_, y_,  1, -1, pos_.info[4], board_ );
}

void Gomoku::countPos( int x_, int y_, Eval &pos_ ) const
//-------------------------------------------------------------------------------
{
	countPos( x_, y_, pos_, _board );
}

bool Gomoku::checkWin( int x_, int y_ ) const
//-------------------------------------------------------------------------------
{
	Eval e;
	countPos( x_, y_, e );
	return e.wins();
}

bool Gomoku::findMove( Move& move_ ) const
//-------------------------------------------------------------------------------
{
	vector<Move> moves;
	for ( int x = 1; x <= _G + 1; x++ )
	{
		for ( int y = 1; y <= _G + 1; y++ )
		{
			if ( _board[x][y] == 0 )
			{
				Move move( x, y );
				int value = eval( move );
				if ( value)
					moves.push_back( move );
			}
		}
	}
	DBG( moves.size() << " moves evaluated" );
	if ( moves.empty() )
		return false;

	int max_value = 0;
	vector<Move> equal;
	for ( size_t i = 0; i < moves.size(); i++ )
	{
		if ( moves[i].value > max_value )
		{
			equal.clear();
			max_value = moves[i].value;
			equal.push_back( moves[i] );
		}
		else if ( moves[i].value == max_value )
		{
			equal.push_back( moves[i] );
		}
	}
	DBG( equal.size() << " moves with value " << max_value );
	int move = random() % equal.size();
	move_ = equal[move];
	return true;
} // findMove

int Gomoku::eval( Move& move_ ) const
//-------------------------------------------------------------------------------
{
	Board board;
	memcpy( &board, &_board, sizeof( board ) );

	int value = 0;
	int x = move_.x;
	int y = move_.y;

	board[x][y] = COMPUTER;
	Eval ec;
	countPos( x, y, ec, board );
	if ( ec.wins() )
	{
		DBG( "eval COMPUTER wins at " << x << "/" << y );
		value += 10000;
	}

	board[x][y] = PLAYER;
	Eval ep;
	countPos( x, y, ep, board );

	if ( ep.wins() )
	{
		DBG( "eval PLAYER wins at " << x << "/" << y );
		value += 9000;
	}

	if ( ec.has4() )
	{
		DBG( "eval has4 COMPUTER at " << x << "/" << y );
		value += 8000;
	}

	if ( ep.has4() )
	{
		DBG( "eval has4 PLAYER at " << x << "/" << y );
		value += 7000;
	}

	if ( ec.has3Fork() )
	{
		DBG( "eval has3Fork COMPUTER at " << x << "/" << y );
		value += 2000;
	}

	if ( ep.has3Fork() )
	{
		DBG( "eval has3Fork PLAYER at " << x << "/" << y );
		value += 1000;
	}

	if ( ec.has3() )
	{
		DBG( "eval has3 COMPUTER at " << x << "/" << y );
		value += 500;
	}

	if ( ep.has3() )
	{
		DBG( "eval has3 PLAYER at " << x << "/" << y );
		value += 400;
	}

	move_.value = value;
	return value;
} // eval

void Gomoku::setPiece( const Move& move_, int who_ )
//-------------------------------------------------------------------------------
{
	if ( !_replay )
		_moves++;
	else if ( _debug && move_.x )
	{
		ostringstream ss;
		ss << "Value: " << move_.value;
		_dmsg = ss.str();
	}
	bool adraw = ( move_.x ==  0 || move_.y == 0 );
	if ( !adraw )
	{
		_history.push_back( move_ );
		_move = move_;
		_board[ move_.x ][ move_.y ] = who_;
		DBG( "Move #" << _history.size() << ": " <<
		     (char)( move_.y + 'A' - 1 ) << (char)( move_.x + 'a' - 1 ) <<
			  " (" << move_.x << "/" << move_.y << ") value: " << move_.value );
		redraw();
	}
	if ( _player )
	{
		Move move;
		adraw = !randomMove( move );
	}
	if ( _debug )
		dumpBoard();
	if ( adraw || checkWin( move_.x, move_.y ) )
	{
		if ( !_replay )
		{
			// update game stats
			_games++;
			if ( !adraw )
				who_ == PLAYER ? _player_wins++ : _computer_wins++;
			_cfg->set( "games", _games );
			_cfg->set( "moves", _moves );
			_cfg->set( "player_wins", _player_wins );
			_cfg->set( "computer_wins", _computer_wins );
			_cfg->flush();
		}
		wait( 0.5 );
		fl_beep( FL_BEEP_MESSAGE );

		ostringstream stat;
		if ( _replay )
		{
			stat << "** Replay end **";
		}
		else
		{
			stat << _games << " games - " <<
			_player_wins << " : " << _computer_wins <<
			endl << "(average moves: " << _moves / _games << ")";
		}
		ostringstream msg;
		if ( !_abortReplay )
		{
			msg << ( adraw ? "No more moves!\n\nGame ends adraw." :
			         who_ == PLAYER ? "You managed to win!" : "FLTK wins!" ) <<
			         endl << endl;
		}
		msg << stat.str();
		DBG( msg.str() );
		if ( _alert )
		{
			fl_alert( "%s", msg.str().c_str() );
		}
		else
		{
			_message = msg.str();
			redraw();
		}

		if ( !waitKey() )
			return;

		int answer = fl_choice( "Do you want to replay\nthe game?", "NO" , "YES", 0 );
		_replay = answer == 1;
		_abortReplay = false;
		_message.erase();
		_dmsg.erase();
		if ( _replay )
		{
			_message = "Replay mode";
			_player = !_first_player;
		}
		else
		{
			_replayMoves.clear();
			_player = _first_player;
		}
		clearBoard();
		redraw();
	}
	_player = !_player;
	nextMove();
} // setPiece

void Gomoku::onDelay()
//-------------------------------------------------------------------------------
{
	_waiting = false;
}

/*static*/
void Gomoku::cb_delay( void *d_ )
//-------------------------------------------------------------------------------
{
	static_cast<Gomoku *>( d_ )->onDelay();
}

void Gomoku::wait( double delay_ )
//-------------------------------------------------------------------------------
{
	Fl::remove_timeout( cb_delay, this );
	Fl::add_timeout( delay_, cb_delay, this );
	_waiting = true;
	while ( _waiting )
		Fl::check();
	Fl::remove_timeout( cb_delay, this );
	_waiting = false;
}

bool Gomoku::waitKey()
//-------------------------------------------------------------------------------
{
	_wait_click = true;
	cursor( FL_CURSOR_MOVE );
	while ( _wait_click )
		Fl::check();
	if ( !shown() ) // user closed program
		return false;
	return true;
}

/*virtual */
int Gomoku::handle( int e_ )
//-------------------------------------------------------------------------------
{
	if ( e_ == FL_HIDE ) // window closed, don't block exit!
		_wait_click = false;

	if ( e_ == FL_KEYDOWN && Fl::event_key( 'd' ) )
	{
		_debug = !_debug;
		cout << "debug " << ( _debug ? "ON" : "OFF" ) << endl;
		redraw();
	}
	if ( _wait_click )
	{
		if ( e_ == FL_PUSH || ( e_ == FL_KEYDOWN &&
		     ( Fl::event_key( ' ' ) || Fl::event_key( FL_Escape ) ) ) )
		{
			_wait_click = false;
			if ( Fl::event_key( FL_Escape ) )
				_abortReplay = true;
			return 1;
		}
		else if ( _replay && e_ == FL_KEYDOWN && Fl::event_key( FL_BackSpace ) )
		{
			if ( _history.size() )
			{
				Move move = _history.back();
				_history.pop_back();
				_board[ move.x ][ move.y ] = 0;
				_player = !_player;
				move.init();
				if ( _history.size() )
					move = _history.back();
				_move = move;
				redraw();
				if ( _debug )
					dumpBoard();
			}
		}
		return Inherited::handle( e_ );
	}
	if ( e_ == FL_PUSH && _player && Fl::event_button() == 1 )
	{
		int x = ( Fl::event_x() + xp( 1 ) / 2 ) / xp( 1 );
		int y = ( Fl::event_y() + yp( 1 ) / 2 ) / yp( 1 );
		if ( x >= 1 && x <= _G + 1 && y >= 1 && y <= _G + 1 && _board[x][y] == 0 )
			setPiece( Move( x, y ), PLAYER );
		else
			fl_beep( FL_BEEP_ERROR );
		return 1;
	}
	else if ( e_ == FL_KEYDOWN && _player && Fl::event_key( FL_BackSpace ) )
	{
		if ( _history.size() >= 2 )
		{
			for ( int i = 0; i < 2; i++ )
			{
				Move move = _history.back();
				_history.pop_back();
				_board[ move.x ][ move.y ] = 0;
				_moves--;
				DBG( "Undo move " << move.x << "/" << move.y );
			}
			Move move;
			if ( _history.size() )
				move = _history.back();
			_move = move;
			redraw();
			if ( _debug )
				dumpBoard();
		}
	}
	else if ( e_ == FL_MOVE && _player && _move.x )
	{
		static int moved = 0;
		moved++;
		if ( moved > 10 )
		{
			_move.x = 0;
			moved = 0;
			redraw();
		}
	}
	return Inherited::handle( e_ );
} // handle

void Gomoku::drawBoard( bool bg_/* = false*/ ) const
//-------------------------------------------------------------------------------
{
	if ( bg_ )
	{
		fl_rectf( 0, 0, w(), h(), FL_DARK_GRAY );
		fl_rectf( 0, 0, xp( _G + 2 ), yp( _G + 2 ), BOARD_COLOR );
	}

	// draw grid
	if ( xp( 1 ) > 8 )
		fl_line_style( FL_SOLID, 2 ); // for outline
	fl_rect( -2, -2, xp( _G + 2 ) + 2, yp( _G + 2 ) + 2, FL_DARK_GRAY );
	fl_rect( xp( 1 ), yp( 1 ), xp ( _G ) + 1, yp( _G ) + 1, BOARD_GRID_COLOR );
	fl_line_style( 0 , 0 );
	for ( int x = 0; x <= _G; x++ )
		fl_line( xp( x + 1 ), yp( 1 ), xp( x + 1 ), yp( 1  + _G ) );

	for ( int y = 0; y <= _G; y++ )
		fl_line( xp( 1 ), yp( y + 1 ), xp( 1 + _G ), yp( y + 1 ) );

	// draw center
	if ( xp( 1 ) > 8 )
	{
		int r = ceil( (double)xp( 1 ) / 12 );
		fl_color( FL_BLACK );
		fl_circle( xp( _G / 2 + 1 ), yp( _G / 2 + 1 ), r );
		fl_circle( xp( _G / 4 + 1 ), yp( _G / 4 + 1 ), r );
		fl_circle( xp( _G / 4 + 1 ), yp( _G / 2 + 1 ), r );
		fl_circle( xp( _G / 2 + 1 ), yp( _G / 4 + 1 ), r );
		fl_circle( xp( _G - _G / 4 + 1 ), yp( _G / 4 + 1 ), r );
		fl_circle( xp( _G - _G / 4 + 1 ), yp( _G / 2 + 1 ), r );
		fl_circle( xp( _G / 4 + 1 ), yp( _G - _G / 4 + 1 ), r );
		fl_circle( xp( _G / 2 + 1 ), yp( _G - _G / 4 + 1 ), r );
		fl_circle( xp( _G - _G / 4 + 1 ), yp( _G - _G / 4 + 1 ), r );
	}

	if ( _debug )
	{
		// draw labels
		fl_font( FL_COURIER, xp( 1 ) / 3 );
		for ( int x = 1; x <= _G + 1; x++ )
		{
			ostringstream os;
			os << (char)('a' + x - 1);
			fl_draw( os.str().c_str(), xp( x ) - xp( 1 ) / 8, yp( 0 ) + yp( 1 ) / 2 );
		}
		for ( int y = 1; y <= _G + 1; y++ )
		{
			ostringstream os;
			os << (char)('A' + y - 1);
			fl_draw( os.str().c_str(), xp( 0 ) + xp( 1 ) / 4, yp( y ) + yp( 1 ) / 8 );
		}
	}

} // drawBoard

void Gomoku::setIcon()
//-------------------------------------------------------------------------------
{
	Fl_Image_Surface *rgb_surf = new Fl_Image_Surface( w(), h(), 1 );
	rgb_surf->set_current();
	drawBoard( true );
	Fl_RGB_Image *icon_image = rgb_surf->image();
	delete rgb_surf;
	if ( icon_image )
	{
		Fl_RGB_Image *icon = (Fl_RGB_Image *)icon_image->copy( 32, 32 );
		this->icon( icon );
		delete icon;
	}
	delete icon_image;
	Fl_Display_Device::display_device()->set_current();
}

/*virtual*/
void Gomoku::draw()
//-------------------------------------------------------------------------------
{
	// FIXME: window sizes to full desktop when double click
	//        on title bar (does not keep aspect), so we need
	//        a background clip the bg image to board size.
	fl_rectf( 0, 0, w(), h(), FL_DARK_GRAY );

	bool bg = child(0)->image();
	if ( bg )
	{
		int W = xp( _G + 2 );
		fl_push_clip( 0, 0, W, W );
		Inherited::draw();
		fl_pop_clip();
	}

	// draw board
	drawBoard( !bg );

	// draw pieces
	for ( int x = 1; x <= _G + 1; x++ )
	{
		for ( int y = 1; y <= _G + 1; y++ )
		{
			if ( _board[x][y] )
			{
				drawPiece( _board[x][y], x, y );
			}
		}
	}

	// draw messages
	if ( _message.size() )
	{
		fl_color( FL_DARK_GRAY );
		fl_font( FL_HELVETICA|FL_BOLD, .8 * xp( 1 ) );
		fl_draw( _message.c_str(), xp( 1 ) + 2, yp( 1 ) + 2, xp( 18 ), yp( 18 ),
			FL_ALIGN_CENTER | FL_ALIGN_TOP, 0, 0 );
		fl_color( FL_YELLOW );
		fl_draw( _message.c_str(), xp( 1 ) , yp( 1 ), xp( 18 ), yp( 18 ),
			FL_ALIGN_CENTER | FL_ALIGN_TOP, 0, 0 );
	}

	if ( _dmsg.size() )
	{
		fl_color( FL_WHITE );
		fl_font( FL_HELVETICA|FL_BOLD, xp( 1 ) / 3 );
		fl_draw( _dmsg.c_str(), xp( 1 ), yp( _G + 1 ) + yp( 1 ) / 2, xp( 18 ), yp( 18 ),
			FL_ALIGN_CENTER | FL_ALIGN_TOP, 0, 0 );
	}
} // draw

//-------------------------------------------------------------------------------
int main( int argc_, char *argv_[] )
//-------------------------------------------------------------------------------
{
	Fl::scheme( "gtk+" );
	Fl::get_system_colors();
	fl_register_images();
	srand( time( 0 ) );
	Gomoku g( argc_, argv_ );
	return Fl::run();
}
