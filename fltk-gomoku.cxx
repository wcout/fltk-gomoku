/*
	FLTK Gomoku (c) 2017
*/
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <cstdlib>
#include <cmath>

#define FL_DARK_GRAY fl_darker( FL_GRAY )

class Board : public Fl_Double_Window
{
typedef Fl_Double_Window Inherited;
public:
	Board() :
		Inherited( 600, 600, "FLTK Gomoku" ),
		_G( 18 ),
		_player( true ),
		_pondering( false )
	{
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
		bool winning_piece = checkWin( x_, y_ );
		fl_color( winning_piece ? color_ == 1 ?
			FL_GREEN : FL_RED : fl_darker( FL_GRAY ) );
		fl_arc( x - rw/2 + 1, y - rh/2 + 1, rw - 2, rh - 2, 0, 360 );
	}
	void onMove()
	{
		// stub
		int G = _G / 2;
		while ( 1 )
		{
			int x = random() % ( G + 1 ) + G / 2 + 1;
			int y = random() % ( G + 1 ) + G / 2 + 1;
			if ( _board[x][y] ) continue;
			setPiece( x, y, 1 );
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
		while ( _pondering )
		{
			Fl::check();
		}
		fl_cursor( FL_CURSOR_DEFAULT );
		onMove();
	}
	int countX( int x_, int y_ ) const
	{
		int c = _board[x_][y_];
		if ( c <= 0 )
			return 0;
		int n( 1 );
		int x( x_ );
		while ( _board[++x][y_] == c )
			n++;
		x = x_;
		while ( _board[--x][y_] == c )
			n++;
		return n;
	}
	int countY( int x_, int y_ ) const
	{
		int c = _board[x_][y_];
		if ( c <= 0 )
			return 0;
		int n( 1 );
		int y( y_ );
		while ( _board[x_][++y] == c )
			n++;
		y = y_;
		while ( _board[x_][--y] == c )
			n++;
		return n;
	}
	int countXYLeft( int x_, int y_ ) const
	{
		int c = _board[x_][y_];
		if ( c <= 0 )
			return 0;
		int n( 1 );
		int y( y_ );
		int x( x_ );
		while ( _board[--x][--y] == c )
			n++;
		y = y_;
		x = x_;
		while ( _board[++x][++y] == c )
			n++;
		return n;
	}
	int countXYRight( int x_, int y_ ) const
	{
		int c = _board[x_][y_];
		if ( c <= 0 )
			return 0;
		int n( 1 );
		int y( y_ );
		int x( x_ );
		while ( _board[++x][--y] == c )
			n++;
		y = y_;
		x = x_;
		while ( _board[--x][++y] == c )
			n++;
		return n;
	}
	bool checkWin( int x_, int y_ ) const
	{
		int n = countX( x_, y_ );
		if ( n != 5 )
			n = countY( x_, y_ );
		if ( n != 5 )
			n = countXYLeft( x_, y_ );
		if ( n != 5 )
			n = countXYRight( x_, y_ );
		return n == 5;
	}
	void onResized()
	{
		size( xp( _G + 2 ), yp( _G + 2 ) );
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
	void setPiece( int x_, int y_, int color_ )
	{
		if ( x_ < 1 || x_ > _G + 1 )
			return;
		if ( y_ < 1 || y_ > _G + 1 )
			return;
		_board[ x_][ y_ ] = color_ + 1;
		redraw();
		if ( checkWin( x_, y_ ) )
		{
			fl_alert( _board[x_][y_] == 1 ? "You win!" : "I win!");
			clearBoard();
			redraw();
		}
		_player = !_player;
		if ( !_player )
		{
			makeMove();
		}
	}
	int handle( int e_ )
	{
		if ( e_ == FL_PUSH && _player )
		{
			int x = ( Fl::event_x() + xp( 1 ) / 2 ) / xp( 1 );
			int y = ( Fl::event_y() + yp( 1 ) / 2 ) / yp( 1 );
			setPiece( x, y, 0 );
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
	int _board[30][30];
	bool _player;
	bool _pondering;
};

int main()
{
	Board g;
	g.show();
	return Fl::run();
}
