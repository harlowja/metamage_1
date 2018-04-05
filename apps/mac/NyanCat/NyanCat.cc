/*
	NyanCat.cc
	----------
*/

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#if TARGET_API_MAC_CARBON
#ifndef __MACWINDOWS__
#include <MacWindows.h>
#endif
#else
#ifndef __LOWMEM__
#include <LowMem.h>
#endif
#endif

// missing-macos
#ifdef MAC_OS_X_VERSION_10_7
#ifndef MISSING_QUICKDRAW_H
#include "missing/QDOffscreen.h"
#include "missing/Quickdraw.h"
#endif
#endif

// nyancatlib
#include "nyancat/graphics.hh"

// NyanCat
#include "Bitmap.hh"


using nyancat::bitmap;
using nyancat::n_frames;


const Pattern veryDarkGray = { 0x77, 0xFF, 0xDD, 0xFF, 0x77, 0xFF, 0xDD, 0xFF };

const int zoom = 4;

const int nyan_width  = zoom * 70;
const int nyan_height = zoom * 72;

const int nyan_stride = (nyan_width + 15) / 16 * 2;

const int offscreen_buffer_size = nyan_stride * nyan_height * n_frames;

void bitmap::set_pixel( unsigned x, unsigned y, const Pattern& color )
{
	x *= its_magnifier;
	y *= its_magnifier;
	
	Rect r = { y, x, y + its_magnifier, x + its_magnifier };
	
	FillRect( &r, &color );
}

static const Rect grow_size =
{
	50, 50,
	32767, 32767,
};

static
Point get_window_topLeft( WindowRef w )
{
#if ! TARGET_API_MAC_CARBON
	
	const Rect& bitmap_bounds = w->portBits.bounds;
	
	Point topLeft = { -bitmap_bounds.top, -bitmap_bounds.left };
	
	return topLeft;
	
#endif
	
	Rect bounds;
	
	GetWindowBounds( w, kWindowContentRgn, &bounds );
	
	return *(Point*) &bounds;
}

static
const Rect* drag_bounds()
{
#if ! TARGET_API_MAC_CARBON
	
	return &LMGetGrayRgn()[0]->rgnBBox;
	
#endif
	
	return NULL;  // DragWindow() bounds may be NULL in Carbon
}

#if ! TARGET_API_MAC_CARBON

static inline
const Rect& get_portRect( GrafPtr port )
{
	return port->portRect;
}

static inline
const Rect& get_portRect( CGrafPtr port )
{
	return port->portRect;
}

#else

static inline
Rect get_portRect( GrafPtr port )
{
	Rect bounds;
	GetPortBounds( port, &bounds );
	
	return bounds;
}

static inline
Rect get_portRect( WindowRef window )
{
	return get_portRect( GetWindowPort( window ) );
}

#endif

const int fps = 15;
const int tick_period = 60 / fps;

static Rect buffer_bounds = { 0, 0, nyan_height * n_frames, nyan_width };

static GrafPtr offscreen_port;

static
void make_offscreen_port()
{
	Ptr addr = NewPtr( offscreen_buffer_size );
	
	BitMap bitmap = { addr, nyan_stride, buffer_bounds };
	
#if ! TARGET_API_MAC_CARBON
	
	static GrafPort port;
	
	offscreen_port = &port;
	
	OpenPort( offscreen_port );
	
	RectRgn( offscreen_port->visRgn, &buffer_bounds );
	
	SetPortBits( &bitmap );
	
	PortSize( nyan_width, nyan_height * n_frames );
	
#else
	
	QDErr err = NewGWorld( &offscreen_port, 1, &buffer_bounds, NULL, NULL, 0 );
	
	if ( err )
	{
		ExitToShell();
	}
	
	SetGWorld( offscreen_port, NULL );
	
#endif
}

static
void make_offscreen_buffer()
{
	make_offscreen_port();
	
	bitmap bits( zoom );
	
	FillRect( &buffer_bounds, &veryDarkGray );
	
	draw_frame( bits, 0 );
	
	for ( int i = 1;  i < n_frames;  ++i )
	{
		MovePortTo( 0, nyan_height * i );
		
		draw_frame( bits, i );
	}
	
	MovePortTo( 0, 0 );
}

static
const Rect& main_display_bounds()
{
#if ! TARGET_API_MAC_CARBON
	
	return qd.screenBits.bounds;
	
#endif
	
	return GetMainDevice()[0]->gdRect;
}

static inline
short aligned( short x, short alignment )
{
	const short antimask = alignment - 1;
	const short rounding = antimask / 2;
	
	return x + rounding & ~antimask;
}

static WindowRef main_window;

static
void make_main_window()
{
	const short alignment = 16;
	
	Rect bounds = main_display_bounds();
	
	const short exact_left = (bounds.left + bounds.right - nyan_width ) / 2;
	
	const short aligned_left = aligned( exact_left, alignment );
	
	bounds.left  = aligned_left;
	bounds.right = bounds.left + nyan_width;
	
	bounds.top    = (bounds.top + bounds.bottom - nyan_height - 41) / 2 + 39;
	bounds.bottom = bounds.top + nyan_height;
	
	main_window = NewWindow( NULL,
	                         &bounds,
	                         "\p" "NyanCat",
	                         true,
	                         noGrowDocProc,
	                         (WindowRef) -1,
	                         true,
	                         0 );
}

class timer
{
	private:
		uint32_t its_frame_duration;
		uint32_t its_time_of_play;
		int its_start_frame;
		bool it_is_paused;
	
	public:
		static const uint32_t arbitrarily_long = 0x7FFFFFFF;
		
		static uint32_t clock()  { return TickCount(); }
		
		timer() : its_start_frame(), it_is_paused()
		{
			its_frame_duration = tick_period;  // e.g. 4 (ticks) for 15fps
		}
		
		void play()  { it_is_paused = false;  its_time_of_play = clock(); }
		void pause()  { its_start_frame = frame();  it_is_paused = true; }
		
		void play_pause()
		{
			it_is_paused ? play() : pause();
		}
		
		void advance( int skip )  { its_start_frame += skip; }
		
		int frame() const;
		
		bool paused() const  { return it_is_paused; }
		
		uint32_t ticks_until_next_frame() const
		{
			if ( paused() )
			{
				return arbitrarily_long;
			}
			
			const uint32_t t = clock() - its_time_of_play;
			
			return its_frame_duration - t % its_frame_duration;
		}
};

int timer::frame() const
{
	if ( it_is_paused )
	{
		return its_start_frame;
	}
	
	const uint32_t now = clock();
	
	return (now - its_time_of_play) / its_frame_duration + its_start_frame;
}

static timer animation_timer;

static
unsigned next_sleep()
{
	return animation_timer.ticks_until_next_frame();
}

static
void draw_window( WindowRef window )
{
	SetPortWindowPort( window );
	
	unsigned t = animation_timer.frame();
	
	t %= 12;
	
	const Rect& portRect = get_portRect( window );
	
	Rect srcRect = portRect;
	
	OffsetRect( &srcRect, 0, t * nyan_height );
	
	CopyBits( GetPortBitMapForCopyBits( (CGrafPtr) offscreen_port ),
	          GetPortBitMapForCopyBits( GetWindowPort( window ) ),
	          &srcRect,
	          &portRect,
	          srcCopy,
	          NULL );
}

int main()
{
	Boolean quitting = false;
	
#if ! TARGET_API_MAC_CARBON
	
	InitGraf( &qd.thePort );
	
	InitFonts();
	InitWindows();
	InitMenus();
	
	InitCursor();
	
#endif
	
	SetEventMask( everyEvent );
	
	make_offscreen_buffer();
	
	make_main_window();
	
	animation_timer.play();
	
	static int last_frame = -1;
	
	while ( ! quitting )
	{
		if ( main_window )
		{
			int next_frame = animation_timer.frame() % 12;
			
			if ( next_frame != last_frame )
			{
				draw_window( main_window );
			}
		}
		
		EventRecord event;
		
		if ( WaitNextEvent( everyEvent, &event, next_sleep(), NULL ) )
		{
			WindowRef window;
			
			switch ( event.what )
			{
				case mouseDown:
					switch ( FindWindow( event.where, &window ) )
					{
						case inMenuBar:
							break;
						
						case inDrag:
							DragWindow( window, event.where, drag_bounds() );
							break;
						
						case inGoAway:
							if ( TrackGoAway( window, event.where ) )
							{
								DisposeWindow( main_window );
								
								main_window = NULL;
								quitting = true;
							}
							break;
						
						case inContent:
							animation_timer.play_pause();
							break;
						
						default:
							break;
					}
					break;
					
				case mouseUp:
					break;
					
				case keyDown:
					switch( (char) event.message )
					{
						case 'q':
							quitting = true;
							break;
						
						case ' ':
							animation_timer.play_pause();
							break;
						
						case 0x1C:
							animation_timer.advance( -1 );
							break;
						
						case 0x1D:
							animation_timer.advance( 1 );
							break;
						
						default:
							break;
					}
					break;
				
				case keyUp:
					break;
				
				case updateEvt:
					window = (WindowRef) event.message;
					
					BeginUpdate( window );
					draw_window( window );
					EndUpdate  ( window );
					break;
				
				default:
					break;
			}
		}
	}
	
	ExitToShell();
	
	return 0;
}