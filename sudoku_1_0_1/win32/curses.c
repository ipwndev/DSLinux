/* Dumb curses implementation for Win32 port
 */

#include "curses.h"

static HANDLE _conin;
static HANDLE _conout;

void
wclear( Window * w )
{
    COORD sz, xy = { 0, 0 };
    DWORD wrote;
    sz = GetLargestConsoleWindowSize( _conout );
    FillConsoleOutputCharacter( _conout, ' ', sz.X * sz.Y, xy, &wrote );
}

void
attron( unsigned short attr )
{
    SetConsoleTextAttribute( _conout, attr );
}

void
attroff( unsigned short attr )
{
    SetConsoleTextAttribute( _conout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE );
}

void
move( int y, int x )
{
    COORD xy = { x, y };
    SetConsoleCursorPosition( _conout, xy );
}

void
mvaddstr( int y, int x, const char * str )
{
    DWORD wrote;
    move( y,x );
    WriteConsole( _conout, str, strlen( str ), &wrote, 0 );
}

void
addch( int ch )
{
    DWORD wrote;
    WriteConsole( _conout, &ch, 1, &wrote, 0 );
}

void
mvaddch( int y, int x, int ch )
{
    move( y, x );
    addch( ch );
}

void
wclrtoeol( Window * w )
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    DWORD wrote;
    GetConsoleScreenBufferInfo( _conout, &info );
    FillConsoleOutputCharacter( _conout, ' ', info.dwSize.X - info.dwCursorPosition.X,
                                info.dwCursorPosition, &wrote );
}

void
wrefresh( Window * w )
{
    /* Do nothing - all changes are immediate */
}

int
wgetch( Window * w )
{
    INPUT_RECORD in;
    DWORD got;
    while( 1 )
    {
        do
            ReadConsoleInput( _conin, &in, 1, &got );
        while( KEY_EVENT != in.EventType || 0 == in.Event.KeyEvent.bKeyDown );
        /* Translate direction keys into vi(1) motion */
        switch( in.Event.KeyEvent.wVirtualKeyCode )
        {
        case VK_LEFT:  return 'h';
        case VK_RIGHT: return 'l';
        case VK_UP:    return 'k';
        case VK_DOWN:  return 'j';

        /* Ignore standard modifier keys */
        case VK_SHIFT:
        case VK_CONTROL:
        case VK_MENU:
                continue;
        }
        return in.Event.KeyEvent.uChar.AsciiChar;
    }
}

void
beep( void )
{
    Beep( 650, 250 );
}

void
noecho( void )
{
    /* Do nothing */
}

void
raw( void )
{
    SetConsoleMode( _conin, ENABLE_PROCESSED_INPUT );
}

int
initscr( void )
{
    _conin = GetStdHandle( STD_INPUT_HANDLE );
    _conout = GetStdHandle( STD_OUTPUT_HANDLE );
    SetConsoleTitle( "Sudoku" );
    return 1;
}

void
endwin( void )
{
    CloseHandle( _conin );
    CloseHandle( _conout );
}
