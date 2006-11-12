/* sudoku.c - sudoku game
 *
 * Original notes:
 *
 * Writing a fun Su-Do-Ku game has turned out to be a difficult exercise.
 * The biggest difficulty is keeping the game fun - and this means allowing
 * the user to make mistakes. The game is not much fun if it prevents the
 * user from making moves, or if it informs them of an incorrect move.
 * With movement constraints, the 'game' is little more than an automated
 * solver (and no fun at all).
 *
 * Another challenge is generating good puzzles that are entertaining to
 * solve. It is certainly true that there is an art to creating good
 * Su-Do-Ku puzzles, and that good hand generated puzzles are more
 * entertaining than many computer generated puzzles - I just hope that
 * the algorithm implemented here provides fun puzzles. It is an area
 * that needs work. The puzzle classification is very simple, and could
 * also do with work. Finally, understanding the automatically generated
 * hints is sometimes more work than solving the puzzle - a better, and
 * more human friendly, mechanism is needed.
 *
 * Comments, suggestions, and contributions are always welcome - send email
 * to: mike 'at' laurasia.com.au. Note that this code assumes a single
 * threaded process, makes extensive use of global variables, and has
 * not been written to be reused in other applications. The code makes no
 * use of dynamic memory allocation, and hence, requires no heap. It should
 * also run with minimal stack space.
 *
 * This code and accompanying files have been placed into the public domain
 * by Michael Kennett, July 2005. It is provided without any warranty
 * whatsoever, and in no event shall Michael Kennett be liable for
 * any damages of any kind, however caused, arising from this software.
 *
 * DSLinux notes:
 *
 * This program was modified by Cayenne Boyer (cayennes@gmail.com) for 
 * running under DSLinux.
 *
 * The following changes were made:
 *
 *  * removed the help information from the left side of the screen (it
 *    wouldn't fit) and centered the 
 *  * made the mini help on the right refer to the d-pad
 *  * disabled save option because it was broken (this is a problem with the
 *    unmodified version on my ibook as well)
 *
 *  TODO:
 *  * turn man page into a --help option
 *  * optimize for actual screen size rather than assuming 4x9 font (this
 *    will work but leave it uncentered vertically for the 4x6 font and
 *    probably won't work for the 6x6 font)
 *  * fix saving
 */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <curses.h>

/* Default file locations */
#ifndef TEMPLATE
#define TEMPLATE "/usr/share/games/sudoku/template"
#endif
#ifndef PRECANNED
#define PRECANNED "/usr/share/games/sudoku/precanned"
#endif

static const char * program;        /* argv[0] */

/* Common state encoding in a 32-bit integer:
 *   bits  0-6    index
 *         7-15   state  [bit high signals digits not possible]
 *        16-19   digit
 *           20   fixed  [set if digit initially fixed]
 *           21   choice [set if solver chose this digit]
 *           22   ignore [set if ignored by reapply()]
 *           23   unused
 *        24-26   hint
 *        27-31   unused
 */
#define INDEX_MASK              0x0000007f
#define GET_INDEX(val)          (INDEX_MASK&(val))
#define SET_INDEX(val)          (val)

#define STATE_MASK              0x0000ff80
#define STATE_SHIFT             (7-1)                        /* digits 1..9 */
#define DIGIT_STATE(digit)      (1<<(STATE_SHIFT+(digit)))

#define DIGIT_MASK              0x000f0000
#define DIGIT_SHIFT             16
#define GET_DIGIT(val)          (((val)&DIGIT_MASK)>>(DIGIT_SHIFT))
#define SET_DIGIT(val)          ((val)<<(DIGIT_SHIFT))

#define FIXED                   0x00100000
#define CHOICE                  0x00200000
#define IGNORED                 0x00400000

/* Hint codes (c.f. singles(), pairs(), findmoves()) */
#define HINT_ROW                0x01000000
#define HINT_COLUMN             0x02000000
#define HINT_BLOCK              0x04000000

/* For a general board it may be necessary to do backtracking (i.e. to
 * rewind the board to an earlier state), and make choices during the
 * solution process. This can be implemented naturally using recursion,
 * but it is more efficient to maintain a single board.
 */
static int board[ 81 ];

/* Addressing board elements: linear array 0..80 */
#define ROW(idx)                ((idx)/9)
#define COLUMN(idx)             ((idx)%9)
#define BLOCK(idx)              (3*(ROW(idx)/3)+(COLUMN(idx)/3))
#define INDEX(row,col)          (9*(row)+(col))

/* Blocks indexed 0..9 */
#define IDX_BLOCK(row,col)      (3*((row)/3)+((col)/3))
#define TOP_LEFT(block)         (INDEX(block/3,block%3))

/* Board state */
#define STATE(idx)              ((board[idx])&STATE_MASK)
#define DIGIT(idx)              (GET_DIGIT(board[idx]))
#define HINT(idx)               ((board[idx])&HINT_MASK)
#define IS_EMPTY(idx)           (0 == DIGIT(idx))
#define DISALLOWED(idx,digit)   ((board[idx])&DIGIT_STATE(digit))
#define IS_FIXED(idx)           (board[idx]&FIXED)

/* Record move history, and maintain a counter for the current
 * move number. Concessions are made for the user interface, and
 * allow digit 0 to indicate clearing a square. The move history
 * is used to support 'undo's for the user interface, and hence
 * is larger than required - there is sufficient space to solve
 * the puzzle, undo every move, and then redo the puzzle - and
 * if the user requires more space, then the full history will be
 * lost.
 */
static int idx_history;
static int history[ 3 * 81 ];

/* Possible moves for a given board (c.f. fillmoves()).
 * Also used by choice() when the deterministic solver has failed,
 * and for calculating user hints. The number of hints is stored
 * in num_hints, or -1 if no hints calculated. The number of hints
 * requested by the user since their last move is stored in req_hints;
 * if the user keeps requesting hints, start giving more information.
 * Finally, record the last hint issued to the user; attempt to give
 * different hints each time.
 */
static int idx_possible;
static int possible[ 81 ];
static int num_hints;
static int req_hints;
static int last_hint;

static int pass;    /* count # passes of deterministic solver */

/* Support for template file */
static FILE * ftmplt;
static int n_tmplt;                 /* Number of templates in file */
static int tmplt[ 81 ];             /* Template indices */
static int len_tmplt;               /* Number of template indices */

/* Command line options */
static enum
{
    fStandard,
    fCompact,
    fCSV,
    fPostScript,
    fHTML
} opt_format = fStandard;
static int opt_describe = 0;
static int opt_generate = 0;
static int num_generate = 1;  /* Number boards to generate w/ -g */
static int opt_random = 1;
static int opt_statistics = 0;
static int opt_spoilerhint = 0;
static int opt_solve = 0;
/*static int opt_restrict = 0;*/

/* Reset global state */
static
void
reset( void )
{
    memset( board, 0x00, sizeof( board ) );
    memset( history, 0x00, sizeof( history ) );
    idx_history = 0;
    pass = 0;
}

/* Write text representation to given file */
static
void
text( FILE * f, const char * title )
{
    int i;
    if( fCSV != opt_format )
    {
        if( 0 != title )
            fprintf( f, "%% %s\n", title );
        for( i = 0 ; i < 81 ; ++i )
        {
            if( IS_EMPTY( i ) )
                fprintf( f, fStandard == opt_format ? " ." : "." );
            else
                fprintf( f, fStandard == opt_format ? "%2d" : "%d",
                            GET_DIGIT( board[ i ] ) );
            if( 8 == COLUMN( i ) )
            {
                fprintf( f, "\n" );
                if( fStandard == opt_format && i != 80 && 2 == ROW( i ) % 3 )
                    fprintf( f, "-------+-------+-------\n" );
            }
            else
            if( fStandard == opt_format && 2 == COLUMN( i ) % 3 )
                fprintf( f, " |" );
        }
    }
    else
    {
        for( i = 0 ; i < 81 ; ++i )
        {
            if( !IS_EMPTY( i ) )
                fprintf( f, "%d", GET_DIGIT( board[ i ] ) );
            if( 8 == COLUMN( i ) )
                fprintf( f, "\n" );
            else
                fprintf( f, "," );
        }
    }
}

/* Write PostScript representation to given file */
static
void
postscript( FILE * f, const char * title )
{
#define PS_WIDTH          20   /* Size of each box (points) */
#define PS_MARGIN         5    /* Margin around board (points) */
#define PS_THICK          3    /* Width of thick lines (points) */
#define PS_THIN           1    /* Width of thin lines (points) */
#define PS_BASELINE       5    /* Offset of character base line */

#define PS_TOTWIDTH  (9*PS_WIDTH+2*PS_MARGIN)  /* Total board width */

/* Page size */
#define PS_A4_WIDTH       612
#define PS_A4_HEIGHT      792

#define PS_LEFT_OFFSET    ((PS_A4_WIDTH - PS_TOTWIDTH)/2)
#define PS_BASE_OFFSET    ((PS_A4_HEIGHT - PS_TOTWIDTH)/2)

#define _STR(x)      #x
#define STR(x)       _STR(x)

    int i;
    time_t t;

    time( &t );
    fprintf( f,
"%%!PS-Adobe-3.0 EPSF-3.0\n"
"%%%%BoundingBox: %d %d %d %d\n"
"%%%%Creator: Sudoku by Michael Kennett\n"
"%%%%CreationDate: %s"
, PS_LEFT_OFFSET, PS_BASE_OFFSET
, PS_LEFT_OFFSET + PS_TOTWIDTH, PS_BASE_OFFSET + PS_TOTWIDTH
, ctime( &t ) );
    if( 0 != title )
        fprintf( f, "%%%%Title: %s\n", title );
fprintf( f,
"%%%%EndComments\n"
);

    /* Write the board contents as a string */
    fprintf( f, "(" );
    for( i = 0 ; i < 81 ; ++i )
    {
        if( !IS_EMPTY( i ) )
            fprintf( f, "%d", GET_DIGIT( board[ i ] ) );
        else
            fprintf( f, " " );
    }
    fprintf( f, ")\n" );

    /* Co-ordinate transform */
    fprintf( f, "%d %d translate\n", PS_LEFT_OFFSET, PS_BASE_OFFSET );

    /* Draw board - thin lines first, then thick lines */
    fprintf( f,
"0 setgray\n"
STR( PS_THIN ) " setlinewidth "
"1 8 "                                 /* index, followed by loop count */
"{dup "                                                   /* keep index */
STR( PS_WIDTH ) " mul " STR( PS_MARGIN ) " add "    /* Compute position */
"dup dup dup\n"                                             /* 4 copies */
"  " STR( PS_MARGIN ) " moveto "                       /* vertical line */
"%d lineto "
STR( PS_MARGIN ) " exch moveto "                     /* horizontal line */
"%d exch lineto "
"1 add"                                                 /* update index */
"} repeat pop stroke\n"
STR( PS_THICK ) " setlinewidth "         /* Repeat code for thick lines */
"1 2 "
"{dup "
"%d mul " STR( PS_MARGIN ) " add "
"dup dup dup\n"
"  " STR( PS_MARGIN ) " moveto "
"%d lineto "
STR( PS_MARGIN ) " exch moveto "
"%d exch lineto "
"1 add"
"} repeat pop stroke\n"
/* Draw outside border */
"1 setlinejoin "
STR( PS_MARGIN ) " " STR( PS_MARGIN ) " moveto "
STR( PS_MARGIN ) " %d lineto "
"%d %d lineto "
"%d " STR( PS_MARGIN ) " lineto closepath stroke\n"
, PS_TOTWIDTH - PS_MARGIN, PS_TOTWIDTH - PS_MARGIN
, 3 * PS_WIDTH
, PS_TOTWIDTH - PS_MARGIN, PS_TOTWIDTH - PS_MARGIN
, PS_TOTWIDTH - PS_MARGIN, PS_TOTWIDTH - PS_MARGIN
, PS_TOTWIDTH - PS_MARGIN, PS_TOTWIDTH - PS_MARGIN
);
    /* Now the code for drawing digits */
    fprintf( f, 
"/Helvetica-Bold findfont 12 scalefont setfont\n"
"0 81 "                                /* index, followed by loop count */
"{2 copy 1 getinterval "                              /* load character */
"dup stringwidth pop\n"                        /* and compute the width */
"  " STR( PS_WIDTH ) " exch sub 2 div "              /* and the x-delta */
  "2 index "                                            /* reload index */
  "9 mod " STR( PS_WIDTH ) " mul add "
  STR( PS_MARGIN ) " add\n"                     /* compute x coordinate */
"  8 3 index 9 idiv sub "
  STR( PS_WIDTH ) " mul %d add\n"               /* compute y coordinate */
"  moveto show "                                              /* render */
  "1 add"                                               /* update index */
  "} repeat pop pop\n"
, PS_MARGIN + PS_BASELINE
);
}

static
void
html( FILE * f, const char * title )
{
    int i;

    fprintf( f, "<html><head>" );
    if( 0 != title )
        fprintf( f, "<title>%s</title>", title );
    fprintf( f,
"</head><body>"
"<table align=\"center\" border=\"1\" cellpadding=\"3\" cellspacing=\"1\" rules=\"all\">\n"
);
    for( i = 0 ; i < 81 ; ++i )
    {
        if( 0 == i % 9 )
            fprintf( f, "<tr>" );
        fprintf( f, "<td>" );
        if( IS_EMPTY( i ) )
            fprintf( f, "&nbsp;" );
        else
            fprintf( f, "%d", GET_DIGIT( board[ i ] ) );
        fprintf( f, "</td>" );
        if( 8 == i % 9 )
            fprintf( f, "</tr>\n" );
    }
    fprintf( f,
"</table>"
"</body>"
"</html>\n"
);
}

static
void
print( FILE * f, const char * title )
{
    switch( opt_format )
    {
    case fStandard:
    case fCompact:
    case fCSV:
        text( f, title );
        break;
    case fPostScript:
        postscript( f, title );
        break;
    case fHTML:
        html( f, title );
        break;
    }
}

/* Describe solution history */
static
void
describe( FILE * f )
{
    int i, j;
    for( i = j = 0 ; i < idx_history ; ++i )
        if( 0 == ( history[ i ] & FIXED ) )
        {
            if( i < idx_history && 0 < j )
                fprintf( f, 0 == j % 6 ? "\n" : ", " );
            fprintf( f, "%d %c> (%d,%d)", GET_DIGIT( history[ i ] ),
                                      history[ i ] & CHOICE ? '*' : '-',
                                      1 + ROW( GET_INDEX( history[ i ] ) ),
                                      1 + COLUMN( GET_INDEX( history[ i ] ) ) );
            ++j;
        }
    fprintf( f, "\n" );
}

/* Management of the move history - compression */
static
void
compress( int limit )
{
    int i, j;
    for( i = j = 0 ; i < idx_history && j < limit ; ++i )
        if( !( history[ i ] & IGNORED ) )
            history[ j++ ] = history[ i ];
    for( ; i < idx_history ; ++i )
        history[ j++ ] = history[ i ];
    idx_history = j;
}

/* Management of the move history - adding a move */
static
void
add_move( int idx, int digit, int choice )
{
    int i;

    if( sizeof( history ) / sizeof( int ) == idx_history )
        compress( 81 );

    /* Never ignore the last move */
    history[ idx_history++ ] = SET_INDEX( idx ) | SET_DIGIT( digit ) | choice;

    /* Ignore all previous references to idx */
    for( i = idx_history - 2 ; 0 <= i ; --i )
        if( GET_INDEX( history[ i ] ) == idx )
        {
            history[ i ] |= IGNORED;
            break;
        }
}

/* Iteration over rows/columns/blocks handled by specialised code.
 * Each function returns a block index - call must manage element/idx.
 */
static
int
idx_row( int el, int idx )      /* Index within a row */
{
    return INDEX( el, idx );
}

static
int
idx_column( int el, int idx )   /* Index within a column */
{
    return INDEX( idx, el );
}

static
int
idx_block( int el, int idx )    /* Index within a block */
{
    return INDEX( 3 * ( el / 3 ) + idx / 3, 3 * ( el % 3 ) + idx % 3 );
}

/* Update board state after setting a digit (clearing not handled)
 */
static
void
update( int idx )
{
    const int row = ROW( idx );
    const int col = COLUMN( idx );
    const int block = IDX_BLOCK( row, col );
    const int mask = DIGIT_STATE( DIGIT( idx ) );
    int i;

    board[ idx ] |= STATE_MASK;  /* filled - no choice possible */

    /* Digit cannot appear in row, column or block */
    for( i = 0 ; i < 9 ; ++i )
    {
        board[ idx_row( row, i ) ] |= mask;
        board[ idx_column( col, i ) ] |= mask;
        board[ idx_block( block, i ) ] |= mask;
    }
}

/* Refresh board state, given move history. Note that this can yield
 * an incorrect state if the user has made errors - return -1 if an
 * incorrect state is generated; else return 0 for a correct state.
 */
static
int
reapply( void )
{
    int digit, idx, j;
    int allok = 0;
    memset( board, 0x00, sizeof( board ) );
    for( j = 0 ; j < idx_history ; ++j )
        if( !( history[ j ] & IGNORED ) && 0 != GET_DIGIT( history[ j ] ) )
        {
            idx = GET_INDEX( history[ j ] );
            digit = GET_DIGIT( history[ j ] );
            if( !IS_EMPTY( idx ) || DISALLOWED( idx, digit ) )
                allok = -1;
            board[ idx ] = SET_DIGIT( digit );
            if( history[ j ] & FIXED )
                board[ idx ] |= FIXED;
            update( idx );
        }
    return allok;
}

/* Clear moves, leaving fixed squares
 */
static
void
clear_moves( void )
{
    for( idx_history = 0 ; history[ idx_history ] & FIXED ; ++idx_history )
        ;
    reapply( );
}

static int digits[ 9 ];    /* # digits expressed in element square */
static int counts[ 9 ];    /* Count of digits (c.f. count_set_digits()) */

/* Count # set bits (within STATE_MASK) */
static
int
numset( int mask )
{
    int i, n = 0;
    for( i = STATE_SHIFT + 1 ; i <= STATE_SHIFT + 9 ; ++i )
        if( mask & (1<<i) )
            ++n;
        else
            ++counts[ i - STATE_SHIFT - 1 ];
    return n;
}

static
void
count_set_digits( int el, int (*idx_fn)( int, int ) )
{
    int i;
    memset( counts, 0x00, sizeof( counts ) );
    for( i = 0 ; i < 9 ; ++i )
        digits[ i ] = numset( board[ (*idx_fn)( el, i ) ] );
}

/* Fill square with given digit, and update state.
 * Returns 0 on success, else -1 on error (i.e. invalid fill)
 */
static
int
fill( int idx, int digit )
{
    assert( 0 != digit );

    if( !IS_EMPTY( idx ) )
        return ( DIGIT( idx ) == digit ) ? 0 : -1;

    if( DISALLOWED( idx, digit ) )
        return -1;

    board[ idx ] = SET_DIGIT( digit );
    update( idx );
    add_move( idx, digit, 0 );

    return 0;
}

/* User-level fill square; allowing clears and overwrites, and
 * invalid moves...
 */
static
void
fillx( int idx, int digit )
{
    /* Nothing to do if digit already set or cleared */
    if( DIGIT( idx ) == digit )
        return;

    if( 0 != digit && IS_EMPTY( idx ) )
    {
        board[ idx ] = SET_DIGIT( digit ) | SET_INDEX( idx );
        update( idx );
        add_move( idx, digit, 0 );
    }
    else
    {
        /* Clearing or overwriting is more time consuming */
        add_move( idx, 0, 0 );
        reapply( );

        /* Always apply moves - even in invalid */
        if( 0 != digit )
        {
            history[ idx_history - 1 ] |= SET_DIGIT( digit );
            reapply( );
        }
    }
}

/* Find all squares with a single digit allowed -- do not mutate board */
static
void
singles( int el, int (*idx_fn)( int, int ), int hintcode )
{
    int i, j, idx;

    count_set_digits( el, idx_fn );

    for( i = 0 ; i < 9 ; ++i )
    {
        if( 1 == counts[ i ] )
        {
            /* Digit 'i+1' appears just once in the element */
            for( j = 0 ; j < 9 ; ++j )
            {
                idx = (*idx_fn)( el, j );
                if( !DISALLOWED( idx, i + 1 ) && idx_possible < 81 )
                    possible[ idx_possible++ ] = SET_INDEX( idx )
                                               | SET_DIGIT( i + 1 )
                                               | hintcode;
            }
        }
        if( 8 == digits[ i ] )
        {
            /* 8 digits are masked at this position - just one remaining */
            idx = (*idx_fn)( el, i );
            for( j = 1 ; j <= 9 ; ++j )
                if( 0 == ( STATE( idx ) & DIGIT_STATE( j ) ) && idx_possible < 81 )
                    possible[ idx_possible++ ] = SET_INDEX( idx )
                                               | SET_DIGIT( j )
                                               | hintcode;
        }
    }
}

/* Given the board state, find all possible 'moves' (i.e. squares with just
 * a single digit).
 *
 * Returns the number of (deterministic) moves (and fills the moves array),
 * or 0 if no moves are possible. This function does not mutate the board
 * state, and hence, can return the same move multiple times (with
 * different hints).
 */
static
int
findmoves( void )
{
    int i;

    idx_possible = 0;
    for( i = 0 ; i < 9 ; ++i )
    {
        singles( i, idx_row, HINT_ROW );
        singles( i, idx_column, HINT_COLUMN );
        singles( i, idx_block, HINT_BLOCK );
    }
    return idx_possible;
}

/* Strategies for refining the board state
 *  - 'pairs'     if there are two unfilled squares in a given row/column/
 *                block with the same state, and just two possibilities,
 *                then all other unfilled squares in the row/column/block
 *                CANNOT be either of these digits.
 *  - 'block'     if the unknown squares in a block all appear in the same
 *                row or column, then all unknown squares outside the block
 *                and in the same row/column cannot be any of the unknown
 *                squares in the block.
 *  - 'common'    if all possible locations for a digit in a block appear
 *                in a row or column, then that digit cannot appear outside
 *                the block in the same row or column.
 *  - 'position2' if the positions of 2 unknown digits in a block match
 *                identically in precisely 2 positions, then those 2 positions
 *                can only contain the 2 unknown digits.
 *
 * Recall that each state bit uses a 1 to prevent a digit from
 * filling that square.
 */

static
void
pairs( int el, int (*idx_fn)( int, int ) )
{
    int i, j, k, mask, idx;
    for( i = 0 ; i < 8 ; ++i )
        if( 7 == digits[ i ] ) /* 2 digits unknown */
            for( j = i + 1 ; j < 9 ; ++j )
            {
                idx = (*idx_fn)( el, i );
                if( STATE( idx ) == STATE( (*idx_fn)( el, j ) ) )
                {
                    /* Found a row/column pair - mask other entries */
                    mask = STATE_MASK ^ (STATE_MASK & board[ idx ] );
                    for( k = 0 ; k < i ; ++k )
                        board[ (*idx_fn)( el, k ) ] |= mask;
                    for( k = i + 1 ; k < j ; ++k )
                        board[ (*idx_fn)( el, k ) ] |= mask;
                    for( k = j + 1 ; k < 9 ; ++k )
                        board[ (*idx_fn)( el, k ) ] |= mask;
                    digits[ j ] = -1; /* now processed */
                }
            }
}

/* Worker: mask elements outside block */
static
void
exmask( int mask, int block, int el, int (*idx_fn)( int, int ) )
{
    int i, idx;

    for( i = 0 ; i < 9 ; ++i )
    {
        idx = (*idx_fn)( el, i );
        if( block != BLOCK( idx ) && IS_EMPTY( idx ) )
            board[ idx ] |= mask;
    }
}

/* Worker for block() */
static
void
exblock( int block, int el, int (*idx_fn)( int, int ) )
{
    int i, idx, mask;

    /* By assumption, all unknown squares in the block appear in the
     * same row/column, so to construct a mask for these squares, it
     * is sufficient to invert the mask for the known squares in the
     * block.
     */
    mask = 0;
    for( i = 0 ; i < 9 ; ++i )
    {
        idx = idx_block( block, i );
        if( !IS_EMPTY( idx ) )
            mask |= DIGIT_STATE( DIGIT( idx ) );
    }
    exmask( mask ^ STATE_MASK, block, el, idx_fn );
}

static
void
block( int el )
{
    int i, idx, row, col;

    /* Find first unknown square */
    for( i = 0 ; i < 9 && !IS_EMPTY( idx = idx_block( el, i ) ) ; ++i )
        ;
    if( i < 9 )
    {
        assert( IS_EMPTY( idx ) );
        row = ROW( idx );
        col = COLUMN( idx );
        for( ++i ; i < 9 ; ++i )
        {
            idx = idx_block( el, i );
            if( IS_EMPTY( idx ) )
            {
                if( ROW( idx ) != row )
                    row = -1;
                if( COLUMN( idx ) != col )
                    col = -1;
            }
        }
        if( 0 <= row )
            exblock( el, row, idx_row );
        if( 0 <= col )
            exblock( el, col, idx_column );
    }
}

static
void
common( int el )
{
    int i, idx, row, col, digit, mask;

    for( digit = 1 ; digit <= 9 ; ++digit )
    {
        mask = DIGIT_STATE( digit );
        row = col = -1;  /* Value '9' indicates invalid */
        for( i = 0 ; i < 9 ; ++i )
        {
            /* Digit possible? */
            idx = idx_block( el, i );
            if( IS_EMPTY( idx ) && 0 == ( board[ idx ] & mask ) )
            {
                if( row < 0 )
                    row = ROW( idx );
                else
                if( row != ROW( idx ) )
                    row = 9; /* Digit appears in multiple rows */
                if( col < 0 )
                    col = COLUMN( idx );
                else
                if( col != COLUMN( idx ) )
                    col = 9; /* Digit appears in multiple columns */
            }
        }
        if( -1 != row && row < 9 )
            exmask( mask, el, row, idx_row );
        if( -1 != col && col < 9 )
            exmask( mask, el, col, idx_column );
    }
}

/* Encoding of positions of a digit (c.f. position2()) - abuse DIGIT_STATE */
static int posn_digit[ 10 ];

static
void
position2( int el )
{
    int digit, digit2, i, mask, mask2, posn, count, idx;

    /* Calculate positions of each digit within block */
    for( digit = 1 ; digit <= 9 ; ++digit )
    {
        mask = DIGIT_STATE( digit );
        posn_digit[ digit ] = count = posn = 0;
        for( i = 0 ; i < 9 ; ++i )
            if( 0 == ( mask & board[ idx_block( el, i ) ] ) )
            {
                ++count;
                posn |= DIGIT_STATE( i );
            }
        if( 2 == count )
            posn_digit[ digit ] = posn;
    }
    /* Find pairs of matching positions, and mask */
    for( digit = 1 ; digit < 9 ; ++digit )
        if( 0 != posn_digit[ digit ] )
            for( digit2 = digit + 1 ; digit2 <= 9 ; ++digit2 )
                if( posn_digit[ digit ] == posn_digit[ digit2 ] )
                {
                    mask = STATE_MASK
                           ^ ( DIGIT_STATE( digit ) | DIGIT_STATE( digit2 ) );
                    mask2 = DIGIT_STATE( digit );
                    for( i = 0 ; i < 9 ; ++i )
                    {
                        idx = idx_block( el, i );
                        if( 0 == ( mask2 & board[ idx ] ) )
                        {
                            assert( 0 == (DIGIT_STATE(digit2) & board[idx]) );
                            board[ idx ] |= mask;
                        }
                    }
                    posn_digit[ digit ] = posn_digit[ digit2 ] = 0;
                    break;
                }
}

/* Find some moves for the board; starts with a simple approach (finding
 * singles), and if no moves found, starts using more involved strategies
 * until a move is found. The more advanced strategies can mask states
 * in the board, making this an efficient mechanism, but difficult for
 * a human to understand.
 */
static
int
allmoves( void )
{
    int i, n;

    n = findmoves( );
    if( 0 < n )
        return n;

    for( i = 0 ; i < 9 ; ++i )
    {
        count_set_digits( i, idx_row );
        pairs( i, idx_row );

        count_set_digits( i, idx_column );
        pairs( i, idx_column );

        count_set_digits( i, idx_block );
        pairs( i, idx_block );
    }
    n = findmoves( );
    if( 0 < n )
        return n;

    for( i = 0 ; i < 9 ; ++i )
    {
        block( i );
        common( i );
        position2( i );
    }
    return findmoves( );
}

/* Helper: sort based on index */
static
int
cmpindex( const void * a, const void * b )
{
    return GET_INDEX( *((const int *)b) ) - GET_INDEX( *((const int *)a) );
}

/* Return number of hints. The hints mechanism should attempt to find
 * 'easy' moves first, and if none are possible, then try for more
 * cryptic moves.
 */
int
findhints( void )
{
    int i, n, mutated = 0;

    n = findmoves( );
    if( n < 2 )
    {
        /* Each call to pairs() can mutate the board state, making the
         * hints very, very cryptic... so later undo the mutations.
         */
        for( i = 0 ; i < 9 ; ++i )
        {
            count_set_digits( i, idx_row );
            pairs( i, idx_row );

            count_set_digits( i, idx_column );
            pairs( i, idx_column );

            count_set_digits( i, idx_block );
            pairs( i, idx_block );
        }
        mutated = 1;
        n = findmoves( );
    }
    if( n < 2 )
    {
        for( i = 0 ; i < 9 ; ++i )
        {
            block( i );
            common( i );
        }
        mutated = 1;
        n = findmoves( );
    }

    /* Sort the possible moves, and allow just one hint per square */
    if( 0 < n )
    {
        int i, j;

        qsort( possible, n, sizeof( int ), cmpindex );
        for( i = 0, j = 1 ; j < n ; ++j )
        {
            if( GET_INDEX( possible[ i ] ) == GET_INDEX( possible[ j ] ) )
            {
                /* Let the user make mistakes - do not assume the
                 * board is in a consistent state.
                 */
                if( GET_DIGIT( possible[i] ) == GET_DIGIT( possible[j] ) )
                    possible[ i ] |= possible[ j ];
            }
            else
                i = j;
        }
        n = i + 1;
    }

    /* Undo any mutations of the board state */
    if( mutated )
        reapply( );

    return n;
}

/* Deterministic solver; return 0 on success, else -1 on error.
 */
static
int
deterministic( void )
{
    int i, n;

    n = allmoves( );
    while( 0 < n )
    {
        ++pass;
        for( i = 0 ; i < n ; ++i )
            if( -1 == fill( GET_INDEX( possible[ i ] ),
                            GET_DIGIT( possible[ i ] ) ) )
                return -1;
        n = allmoves( );
    }
    return 0;
}

/* Return index of square for choice.
 *
 * If no choice is possible (i.e. board solved or inconsistent),
 * return -1.
 *
 * The current implementation finds a square with the minimum
 * number of unknown digits (i.e. maximum # masked digits).
 */
static
int
cmp( const void * e1, const void * e2 )
{
    return GET_DIGIT( *(const int *)e2 ) - GET_DIGIT( *(const int *)e1 );
}

static
int
choice( void )
{
    int i, n;
    for( n = i = 0 ; i < 81 ; ++i )
        if( IS_EMPTY( i ) )
        {
            possible[ n ] = SET_INDEX( i ) | SET_DIGIT( numset( board[ i ] ) );

            /* Inconsistency if square unknown, but nothing possible */
            if( 9 == GET_DIGIT( possible[ n ] ) )
                return -2;
            ++n;
        }

    if( 0 == n )
        return -1;      /* All squares known */

    qsort( possible, n, sizeof( possible[ 0 ] ), cmp );
    return GET_INDEX( possible[ 0 ] );
}

/* Choose a digit for the given square.
 * The starting digit is passed as a parameter.
 * Returns -1 if no choice possible.
 */
static
int
choose( int idx, int digit )
{
    for( ; digit <= 9 ; ++digit )
        if( !DISALLOWED( idx, digit ) )
        {
            board[ idx ] = SET_DIGIT( digit );
            update( idx );
            add_move( idx, digit, CHOICE );
            return digit;
        }

    return -1;
}

/* Backtrack to a previous choice point, and attempt to reseed
 * the search. Return -1 if no further choice possible, or
 * the index of the changed square.
 *
 * Assumes that the move history and board are valid.
 */
static
int
backtrack( void )
{
    int digit, idx;

    for( ; 0 <= --idx_history ; )
        if( history[ idx_history ] & CHOICE )
        {
            /* Remember the last choice, and advance */
            idx = GET_INDEX( history[ idx_history ] );
            digit = GET_DIGIT( history[ idx_history ] ) + 1;
            reapply( );
            if( -1 != choose( idx, digit ) )
                return idx;
        }

    return -1;
}

/* Attempt to solve 'board'; return 0 on success else -1 on error.
 *
 * The solution process attempts to fill-in deterministically as
 * much of the board as possible. Once that is no longer possible,
 * need to choose a square to fill in.
 */
static
int
solve( void )
{
    int idx;

    while( 1 )
    {
        if( 0 == deterministic( ) )
        {
            /* Solved, make a new choice, or rewind a previous choice */
            idx = choice( );
            if( -1 == idx )
                return 0;
            else
            if( ( idx < 0 || -1 == choose( idx, 1 ) ) && -1 == backtrack( ) )
                return -1;
        }
        else /* rewind to a previous choice */
        if( -1 == backtrack( ) )
            return -1;
    }
    return -1;
}

/* Find all solutions to a given board, and return the number of
 * solutions (0 if none found).
 */
static
int
number_solutions( void )
{
    int count = 0;
    if( -1 != solve( ) )
    {
        do
            ++count;
        while( -1 != backtrack( ) && -1 != solve( ) );
    }
    return count;
}

/* Build/modify internal representation from file
 *
 *  - lines starting with '#' are ignored
 *  - puzzles start with a line beginning with '%' (with optional title)
 *  - two formats are support: compact, verbose
 *  - templates are always compact
 *  - compact boards have no spaces or block separators
 *  - verbose boards have spaces and block separators
 *
 * When is_tmplt is TRUE, a template is read.
 *
 * Return 0 on success; else -1 on error
 */

static char line[ 80 ];
static char title[ 80 ];

#define COMPACT  0
#define VERBOSE  1

static
int
read_board( FILE * f, int is_tmplt )
{
    char * p, * q;
    int i, row, col, type = COMPACT;

    reset( );
    len_tmplt = 0;

    /* Skip lines until a '%' is found */
    line[ 0 ] = ' ';
    while( '%' != line[ 0 ] )
        if( 0 == fgets( line, sizeof( line ), f ) )
            return -1;

    /* Read optional title, and removing trailing whitespace */
    for( p = line + 1 ; *p && isspace( *p ) ; ++p )
        ;
    if( *p )
    {
        for( q = title ; '\0' != ( *q++ = *p++ ) ; )
            ;
        --q;
        while( isspace( *--q ) )
            ;
        *++q = '\0';
    }
    else
        strcpy( title, "(untitled)" );

    /* Consume comment lines - no leading spaces allowed */
    line[ 0 ] = '#';
    while( '#' == line[ 0 ] )
        if( 0 == fgets( line, sizeof( line ), f ) )
            return -1;

    /* Analyse first line to determine the 'type' - default is COMPACT */
    if( 0 == is_tmplt )
        for( p = line ; *p && COMPACT == type ; ++p )
            if( '|' == *p )
                type = VERBOSE;

    /* Consume grid - allow leading spaces and comments at end */
    for( row = 0 ; row < 9 ; ++row )
    {
        /* Assume line already loaded into buffer; skip leading spaces */
        for( p = line ; *p && isspace( *p ) ; ++p )
            ;

        for( col = 0 ; *p && col < 9 ; ++col, ++p )
            if( is_tmplt )
            {
                if( '*' == *p )
                    tmplt[ len_tmplt++ ] = INDEX( row, col );
            }
            else
            {
                if( VERBOSE == type )
                    while( *p && ( isspace( *p ) || '|' == *p ) )
                        ++p;
                if( isdigit( *p ) )
                {
                    if( 0 != fill( INDEX( row, col ), *p - '0' ) )
                        return -1;
                    board[ INDEX( row, col ) ] |= FIXED;
                }
                /* else assume blank square */
            }

        /* Don't complain about any trailing characters - ignored silently */

        /* Load next line (if needed) */
        if( row < 8 )
        {
            if( 0 == fgets( line, sizeof( line ), f ) )
                return -1;
            if( VERBOSE == type && 2 == row % 3
                && 0 == fgets( line, sizeof( line ), f ) )
                    return -1; /* Skip separators */
        }
    }

    /* Construct move history for a template */
    if( is_tmplt )
    {
        idx_history = 0;
        for( i = 0 ; i < 81 ; ++i )
            if( 0 != DIGIT( i ) )
                history[ idx_history++ ] = i | (DIGIT( i )<<8);
    }

    /* Finally, markup all of these moves as 'fixed' */
    for( i = 0 ; i < idx_history ; ++i )
        history[ i ] |= FIXED;

    return 0;
}

/**
 **  Curses screen interface
 **/

/* Screen geometry */
/* removing instructions from left and centering the rest on the screen */
/* assuming 21x64 for now */
#define WIDTH		 64
#define TOP          4
#define TITLE_LINE   TOP-2
#define HEADER_LINE	 TOP-4
#define LEFT         8/*27*/
#define BOTTOM       (TOP+3*4)
#define RIGHT        (LEFT+3*8)
#define STATUS_LINE  BOTTOM+2
#define FILE_LINE    STATUS_LINE+1

/* Maintain some global state - current cursor position */
static int curx;
static int cury;

static int have_status;  /* True (non-zero) if status line set */
static int have_hint;    /* True (non-zero) If hint displayed */

static char statusline[ 80 ];  /* Buffer for status line */

/* Render board background */
static
void
draw_screen( void )
{
    int i;

    wclear( stdscr );
    attron( A_BOLD );
    mvaddstr( HEADER_LINE, WIDTH/2-4, "Su-Do-Ku!" );
    attroff( A_BOLD );

    for( i = 0 ; i < 3 ; ++i )
    {
        mvaddstr( TOP + 0 + 4 * i, LEFT, "+-------+-------+-------+" );
        mvaddstr( TOP + 1 + 4 * i, LEFT, "|       |       |       |" );
        mvaddstr( TOP + 2 + 4 * i, LEFT, "|       |       |       |" );
        mvaddstr( TOP + 3 + 4 * i, LEFT, "|       |       |       |" );
    }
    mvaddstr( TOP + 4 * 3, LEFT, "+-------+-------+-------+" );

    mvaddstr( TOP + 1, RIGHT + 6, "d-pad move cursor" );
    mvaddstr( TOP + 2, RIGHT + 7, "1-9  place digit" );
    mvaddstr( TOP + 3, RIGHT + 7, "0 .  clear digit" );
    mvaddstr( TOP + 4, RIGHT + 8, "c   clear board" );
    mvaddstr( TOP + 5, RIGHT + 8, "f   fix squares" );
    mvaddstr( TOP + 6, RIGHT + 8, "n   new board" );
    mvaddstr( TOP + 7, RIGHT + 8, "q   quit game" );
    i = TOP + 7;
    /*if( 0 == opt_restrict )
        mvaddstr( ++i, RIGHT + 8, "s   save" );*/
    mvaddstr( ++i, RIGHT + 8, "u   undo last move" );
    mvaddstr( ++i, RIGHT + 8, "v   solve" );
    mvaddstr( ++i, RIGHT + 8, "?   request hint" );
}

/* Write board title */
static
void
write_title( /*const*/ char * title )
{
    move( TITLE_LINE, 0 );
    wclrtoeol( stdscr );
    if( 0 != title )
        mvaddstr( TITLE_LINE, ( WIDTH - strlen( title ) ) / 2, title );
}

/* Move cursor to grid position, and force refresh */
static
void
move_to( int x, int y )
{
    curx = x;
    cury = y;
    move( TOP + 1 + y + y / 3, LEFT + 2 + 2 * ( x + x / 3 ) );
    wrefresh( stdscr );
}

/* Move cursor to next non-fixed square, and force refresh */
void
static
move_next( void )
{
    do
    {
        if( curx < 8 )
            move_to( curx + 1, cury );
        else
        if( cury < 8 )
            move_to( 0, cury + 1 );
        else
            move_to( 0, 0 );
     }
     while( IS_FIXED( INDEX( cury, curx ) ) );
}

/* Render status line */
static
void
set_status( const char * txt )
{
    mvaddstr( STATUS_LINE, ( WIDTH - strlen( txt ) ) / 2, (char *)txt );
    move_to( curx, cury );
    have_status = 1;
}

static
void
clear_status( void )
{
    move( STATUS_LINE, 0 );
    wclrtoeol( stdscr );
    move_to( curx, cury );
    have_status = 0;
}

/* Render current board */
static
void
render( void )
{
    int i, x, y;

    for( i = 0 ; i < 81 ; ++i )
    {
        x = LEFT + 2 + 2 * ( COLUMN( i ) + COLUMN( i ) / 3 );
        y = TOP + 1 + ROW( i ) + ROW( i ) / 3;
        assert( 0 <= DIGIT( i ) );
        assert( DIGIT( i ) <= 9 ); /* XXX FAILING */
        if( IS_FIXED( i ) )
            attron( A_BOLD );
        if( IS_EMPTY( i ) )
            mvaddch( y, x, '.' );
        else
            mvaddch( y, x, '0' + DIGIT( i ) );
        if( IS_FIXED( i ) )
            attroff( A_BOLD );
    }
}

static
void
row_hint( int row )
{
    mvaddch( TOP + 1 + row + row / 3, LEFT - 2, '>' );
    mvaddch( TOP + 1 + row + row / 3, RIGHT + 2, '<' );
    move_to( curx, cury );
    have_hint = 1;
}

static
void
column_hint( int col )
{
    mvaddch( TOP - 1, LEFT + 2 + 2 * ( col + col / 3 ), 'v' );
    mvaddch( BOTTOM + 1, LEFT + 2 + 2 * ( col + col / 3 ), '^' );
    move_to( curx, cury );
    have_hint = 1;
}

static
void
block_hint( int block )
{
    int i, j;
    for( i = 0 ; i < 3 ; ++i )
    {
        j = 3 * ( block / 3 ) + i;
        mvaddch( TOP + 1 + j + j / 3, LEFT - 2, '>' );
        mvaddch( TOP + 1 + j + j / 3, RIGHT + 2, '<' );
        j = 3 * ( block % 3 ) + i;
        mvaddch( TOP - 1, LEFT + 2 + 2 * ( j + j / 3 ), 'v' );
        mvaddch( BOTTOM + 1, LEFT + 2 + 2 * ( j + j / 3 ), '^' );
    }
    move_to( curx, cury );
    have_hint = 1;
}

static
void
clear_hints( void )
{
    int i;
    for( i = 0 ; i < 9 ; ++i )
    {
        mvaddch( TOP + 1 + i + i / 3, LEFT - 2, ' ' );
        mvaddch( TOP + 1 + i + i / 3, RIGHT + 2, ' ' );
        mvaddch( TOP - 1, LEFT + 2 + 2 * ( i + i / 3 ), ' ' );
        mvaddch( BOTTOM + 1, LEFT + 2 + 2 * ( i + i / 3 ), ' ' );
    }
    have_hint = 0;
    move_to( curx, cury );
}

/* Fix all squares - if possible.
 * Returns -1 on error, in which case there is an error in the
 * current board; otherwise returns 0 if all is OK.
 */
static
int
fix( void )
{
    int i;

    if( 0 == reapply( ) )
    {
        compress( idx_history );
        for( i = 0 ; i < idx_history ; ++i )
            history[ i ] |= FIXED;
        reapply( );
        render( );
        if( idx_history < 81 && IS_FIXED( INDEX( cury, curx ) ) )
            move_next( );
        else
            move_to( curx, cury );
        return 0;
    }
    else
        return -1;
}

/* Classify a SuDoKu, given its solution.
 *
 * The classification is based on the average number of possible moves
 * for each pass of the deterministic solver - it is a rather simplistic
 * measure, but gives reasonable results. Note also that the classification
 * is based on the first solution found (but does handle the pathological
 * case of multiple solutions). Note that the average moves per pass
 * depends just on the number of squares initially set... this simplifies
 * the statistics collection immensely, requiring just the number of passes
 * to be counted.
 *
 * Return 0 on error, else a string classification.
 */

static
const char *
classify( void )
{
    int i, score;

    pass = 0;
    clear_moves( );
    if( -1 == solve( ) )
        return 0;

    score = 81;
    for( i = 0 ; i < 81 ; ++i )
        if( IS_FIXED( i ) )
            --score;

    assert( 81 == idx_history );

    for( i = 0 ; i < 81 ; ++i )
        if( history[ i ] & CHOICE )
            score -= 5;

    if( 15 * pass < score )
        return "very easy";
    else
    if( 11 * pass < score )
        return "easy";
    else
    if( 7 * pass < score )
        return "medium";
    else
    if( 4 * pass < score )
        return "hard";
    else
        return "fiendish";
}

/* exchange disjoint, identical length blocks of data */
static
void
exchange( int * a, int * b, int len )
{
    int i, tmp;
    for( i = 0 ; i < len ; ++i )
    {
        tmp = a[ i ];
        a[ i ] = b[ i ];
        b[ i ] = tmp;
    }
}

/* rotate left */
static
void
rotate1_left( int * a, int len )
{
    int i, tmp;
    tmp = a[ 0 ];
    for( i = 1 ; i < len ; ++i )
        a[ i - 1 ] = a[ i ];
    a[ len - 1 ] = tmp;
}

/* rotate right */
static
void
rotate1_right( int * a, int len )
{
    int i, tmp;
    tmp = a[ len - 1 ];
    for( i = len - 1 ; 0 < i ; --i )
        a[ i ] = a[ i - 1 ];
    a[ 0 ] = tmp;
}

/* Generalised left rotation - there is a naturally recursive
 * solution that is best implementation using iteration.
 * Note that it is not necessary to do repeated unit rotations.
 *
 * This function is analogous to 'cutting' a 'pack of cards'.
 *
 * On entry: 0 < idx < len
 */
static
void
rotate( int * a, int len, int idx )
{
    int xdi = len - idx;
    int delta = idx - xdi;

    while( 0 != delta && 0 != idx )
    {
        if( delta < 0 )
        {
            if( 1 == idx )
            {
                rotate1_left( a, len );
                idx = 0;
            }
            else
            {
                exchange( a, a + xdi, idx );
                len = xdi;
            }
        }
        else /* 0 < delta */
        {
            if( 1 == xdi )
            {
                rotate1_right( a, len );
                idx = 0;
            }
            else
            {
                exchange( a, a + idx, xdi );
                a += xdi;
                len = idx;
                idx -= xdi;
            }
        }
        xdi = len - idx;
        delta = idx - xdi;
    }
    if( 0 < idx )
        exchange( a, a + idx, idx );
}

/* Shuffle an array of integers */
static
void
shuffle( int * a, int len )
{
    int i, j, tmp;

    i = len;
    while( 1 <= i )
    {
        j = rand( ) % i;
        tmp = a[ --i ];
        a[ i ] = a[ j ];
        a[ j ] = tmp;
    }
}

/* Generate a SuDoKu puzzle
 *
 * The generation process selects a random template, and then attempts
 * to fill in the exposed squares to generate a board. The order of the
 * digits and of filling in the exposed squares are random.
 */

/* Select random template; sets tmplt, len_tmplt */
static
void
select_template( void )
{
    int i = rand( ) % n_tmplt;
    fseek( ftmplt, 0, SEEK_SET );
    while( 0 <= i && 0 == read_board( ftmplt, 1 ) )
        --i;
}

static
void
generate( void )
{
    static int digits[ 9 ];

    int i;

start:
    for( i = 0 ; i < 9 ; ++i )
        digits[ i ] = i + 1;

    rotate( digits, 9, 1 + rand( ) % 8 );
    shuffle( digits, 9 );
    select_template( );

    rotate( tmplt, len_tmplt, 1 + rand( ) % ( len_tmplt - 1 ) );
    shuffle( tmplt, len_tmplt );

    reset( );  /* construct a new board */

    for( i = 0 ; i < len_tmplt ; ++i )
        fill( tmplt[ i ], digits[ i % 9 ] );

    if( 0 != solve( ) || idx_history < 81 )
        goto start;

    for( i = 0 ; i < len_tmplt ; ++i )
        board[ tmplt[ i ] ] |= FIXED;

    /* Construct fixed squares */
    for( idx_history = i = 0 ; i < 81 ; ++i )
        if( IS_FIXED( i ) )
            history[ idx_history++ ] = SET_INDEX( i )
                                     | SET_DIGIT( DIGIT( i ) )
                                     | FIXED;
    clear_moves( );

    if( 0 != solve( ) || idx_history < 81 )
        goto start;
    if( -1 != backtrack( ) && 0 == solve( ) )
        goto start;

    strcpy( title, "randomly generated - " );
    strcat( title, classify( ) );

    clear_moves( );
}

/* Support for precanned board (optional) */
static FILE * precanned;
static int n_precanned;
static int completed;

static
void
open_precanned( const char * filename )
{
    n_precanned = 0;
    precanned = fopen( filename, "r" );
    if( 0 != precanned )
        while( 0 == read_board( precanned, 0 ) )
            ++n_precanned;
}

static
void
open_template( const char * filename )
{
    n_tmplt = 0;
    ftmplt = fopen( filename, "r" );
    if( 0 != ftmplt )
        while( 0 == read_board( ftmplt, 1 ) )
            ++n_tmplt;
}

/* load a new board - this could be a precanned board,
 * or a randomly generated board - chose between these
 * randomly -- 1 in 3 chance of loading a precanned
 * board (if they exist).
 */
static
void
load_board( void )
{
    int i;

    if( 0 == opt_random || ( 0 == rand( ) % 3 && 0 < n_precanned ) )
    {
        /* Select random board */
        i = rand( ) % n_precanned;
        fseek( precanned, 0, SEEK_SET );
        while( 0 <= i && 0 == read_board( precanned, 0 ) )
            --i;
    }
    else
    {
        set_status( "generating a random board... (please wait)" );
        wrefresh( stdscr );
        generate( );
        clear_status( );
    }

    /* Shameless plug... */
    set_status( "Su-Do-Ku by Michael Kennett" );

    render( );
    write_title( title );

    curx = cury = 8;    /* move_next() takes care of this... */
    move_next( );
    completed = 0;
    num_hints = -1;
}

/* Manage rich text input (define 'virtual' key codes)
 * See the console(4) manpage.
 */

#define VKEY_UP         (256+'A')
#define VKEY_DOWN	(256+'B')
#define VKEY_RIGHT      (256+'C')
#define VKEY_LEFT       (256+'D')
#define VKEY_HOME       (256+'H')

static
int
getkey( void )
{
    int ch = wgetch( stdscr );

    if( 0x1b == ch )
    {
        ch = wgetch( stdscr );
        if( '[' == ch )
        {
            ch = wgetch( stdscr );
            switch( ch )
            {
            case 'A': ch = VKEY_UP; break;
            case 'B': ch = VKEY_DOWN; break;
            case 'C': ch = VKEY_RIGHT; break;
            case 'D': ch = VKEY_LEFT; break;
            case 'H': ch = VKEY_HOME; break;
            }
        }
    }
    return ch;
}

/* Save board representation - the difficulty is managing the user interface
 * (i.e. finding where to save), with a tad more work involved in allowing
 * the board to be written to an arbitrary process.
 *
 * Persist the user filename.
 */
static char userfile[ PATH_MAX ];

static
void
save_board( void )
{
    int ch, i;
    const char * p;
    FILE * f;

    if( '\0' == *userfile )
    {
        userfile[ 0 ] = '>';
        if( 0 == getcwd( userfile + 1, sizeof( userfile ) - 1 - 1 - 6 ) )
            strcpy( userfile + 1, "/path/to/save/file" );
        else
            strcat( userfile, "/board" );
        userfile[ sizeof( userfile ) - 1 ] = '\0';
    }

    clear_status( );
    mvaddstr( STATUS_LINE, 0, "Filename:" );
    mvaddstr( FILE_LINE, 0, userfile );

    wrefresh( stdscr );

    /* Read character input (raw processing mode) */
    i = strlen( userfile );
    while( '\r' != ( ch = getkey( ) ) )
    {
        if( 0x08 == ch || VKEY_LEFT == ch ) /* destructive backspace */
        {
            if( 0 < i )
            {
                userfile[ --i ] = '\0';
                mvaddch( FILE_LINE, i, ' ' );
            }
            else
                beep( );
        }
        else
        if( isprint( ch ) )
        {
            if( i < sizeof( userfile ) - 1 )
            {
                mvaddch( FILE_LINE, i, ch );
                userfile[ i++ ] = ch;
            }
            else
                beep( );
        }
        else
            beep( );

        move( FILE_LINE, i );
        wrefresh( stdscr );
    }
    userfile[ i ] = '\0';

    /* Reset screen */
    clear_status( );
    move( FILE_LINE, 0 );
    wclrtoeol( stdscr );
    move_to( curx, cury );

    p = userfile;
    if( '>' == *p || '|' == *p )
        ++p;
    while( ' ' == *p )
        ++p;

    if( *p )
    {
        if( '|' == userfile[ 0 ] )
        {
            f = popen( p, "w" );
            if( 0 != f )
            {
                print( f, title );
                pclose( f );
            }
        }
        else
        {
            /* Append for standard formats, else separate file */
            switch( opt_format )
            {
            case fCompact: case fStandard:
                f = fopen( p, "a" );
                break;
            default:
                f = fopen( p, "w" );
                break;
            }
            if( 0 != f )
            {
                print( f, title );
                fclose( f );
            }
        }
        if( 0 == f )
        {
            set_status( "Error: writing the file failed" );
            wrefresh( stdscr );
        }
    }
}

/* Generate statistics from boards in 'filename', and/or solve them.
 * Returns a process exit code.
 */
static
int
gen_statistics( void )
{
    const char * classification;

    fseek( precanned, 0, SEEK_SET );
    while( 0 == read_board( precanned, 0 ) )
    {
        /* Ignore insoluble boards */
        if( -1 == solve( ) )
        {
            printf( "Board '%s' has no solution\n", title );
            continue;
        }

        /* If statistics only, ignore boards with multiple solutions */
        if( 0 == opt_solve && -1 != backtrack( ) && 0 == solve( ) )
        {
            printf( "Board '%s' has multiple solutions\n", title );
            continue;
        }

        classification = classify( );
        if( 0 == opt_solve )
            printf( "%2d %-12s : %s\n", pass, classification, title );
        else
        {
            printf( "Solution(s) to '%s' [%s]\n", title, classification );
            clear_moves( );
            if( -1 != solve( ) )
            {
                do
                {
                    print( stdout, title );
                    if( opt_describe )
                    {
                        printf( "Solution history:\n" );
                        describe( stdout );
                    }
                }
                while( -1 != backtrack( ) && -1 != solve( ) );
            }
        }
    }
    return 0;
}

/* Signal catchers - cleanup curses and terminate */
static
void
cleanup( int ignored )
{
    move_to( 23, 0 );
    wrefresh( stdscr );
    endwin( );
    exit( 1 );
}

/* Establish signal handlers */
static
void
signals( void )
{
    sigset_t sigset;
    struct sigaction sighandler;
    struct termios tp;

    sigemptyset( &sigset );
    sighandler.sa_handler = cleanup;
    sighandler.sa_mask = sigset;
    sighandler.sa_flags = 0;

    sigaction( SIGINT,  &sighandler, 0 );
    sigaction( SIGABRT, &sighandler, 0 );
    sigaction( SIGTERM, &sighandler, 0 );
#ifdef SIGHUP
    sigaction( SIGHUP,  &sighandler, 0 );
#endif
#ifdef SIGQUIT
    sigaction( SIGQUIT, &sighandler, 0 );
#endif
#ifdef SIGKILL
    sigaction( SIGKILL, &sighandler, 0 );
#endif

    /* Reenable signal processing */
    if( 0 == tcgetattr( 0, &tp ) )
    {
        tp.c_lflag |= ISIG;
        tcsetattr( 0, TCSANOW, &tp );
    }
}

static
void
usage( void )
{
    fprintf( stderr,
"Usage: %s [options] [<filename>]\n"
"Supported options:\n"
"    -d           describe solution steps (with -v)\n"
"    -f<format>   set output format; supported formats are:\n"
"                    standard   (std)    <default format>\n"
"                    compact\n"
"                    csv                 [comma separated file]\n"
"                    postscript (ps)\n"
"                    html\n"
"    -g[<num>]    generate <num> board(s), and print on stdout\n"
"    -n           no random boards (requires precanned boards <filename>)\n"
/*"    -r           restricted: don't allow boards to be saved\n"*//*temporarily disabled until saving is fixed*/
"    -s           calculate statistics for precanned boards\n"
"    -t<filename> template file\n"
"    -v           solve precanned boards\n"
"    <filename>   'precanned' sudoku boards\n"
, program );
    exit( 1 );
}

int
main( int argc, char **argv )
{
    int ch, i, idx;

    program = argv[ 0 ];

    /* Limited support for options */
    if( 1 < argc )
    {
        char * arg;
        while( 0 < --argc )
        {
            arg = *++argv;
            if( '-' != *arg )
            {
                if( 0 != precanned )
                {
                    fprintf( stderr, "Error: only 1 precanned file allowed\n" );
                    exit( 1 );
                }
                open_precanned( arg );
                strcpy( userfile, arg );  /* Save the filename */
                if( 0 == precanned || 0 == n_precanned )
                {
                    fprintf( stderr, "Error: failed to open '%s'\n", arg );
                    exit( 1 );
                }
            }
            else
                while( '\0' != *++arg )
                    switch( *arg )
                    {
                    case 'd': opt_describe = 1; break;
                    case 'f':
                        if( '\0' == arg[ 1 ] )
                        {
                            arg = *++argv;
                            --argc;
                        }
                        else
                            ++arg;
                        if( 0 == strcmp( "compact", arg ) )
                            opt_format = fCompact;
                        else
                        if( 0 == strcmp( "standard", arg )
                            || 0 == strcmp( "std", arg ) )
                                opt_format = fStandard;
                        else
                        if( 0 == strcmp( "csv", arg ) )
                            opt_format = fCSV;
                        else
                        if( 0 == strcmp( "postscript", arg ) ||
                            0 == strcmp( "ps", arg ) )
                                opt_format = fPostScript;
                        else
                        if( 0 == strcmp( "html", arg ) )
                            opt_format = fHTML;
                        else
                        {
                            fprintf( stderr,
                                     "Error: '%s' is an unknown format\n",
                                     arg );
                            exit( 1 );
                        }
                        arg = "x"; /* dummy to force termination */
                        break;

                    case 'g':
                        opt_generate = 1;
                        if( isdigit( arg[ 1 ] ) )
                        {
                            num_generate = atoi( arg + 1 );
                            arg = "x"; /* dummy to force termination */
                        }
                        else
                        if( '\0' == arg[ 1 ] && 0 != *(argv+1) && isdigit( **(argv+1) ) )
                        {
                            num_generate = atoi( *++argv );
                            --argc;
                        }
                        break;
                    case 'h': opt_spoilerhint = 1; break;
                    case 'n': opt_random = 0; break;
                    /*case 'r': opt_restrict = 1; break;*//*temporarily disabled until saving is fixed*/
                    case 's': opt_statistics = 1; break;
                    case 't':
                        if( '\0' == arg[ 1 ] )
                        {
                            if( 0 == *(argv+1) )
                            {
                                fprintf( stderr,
                                         "Error: expected argument after '-t'\n"
                                       );
                                exit( 1 );
                            }
                            open_template( arg = *++argv );
                            --argc;
                        }
                        else
                        {
                            open_template( ++arg );
                        }
                        if( 0 == ftmplt )
                        {
                            fprintf( stderr,
                                     "Error: failed to open template file '%s'\n",
                                     arg );
                            exit( 1 );
                        }
                        arg = "x"; /* dummy to force termination */
                        break;
                    case 'v': opt_solve = 1; break;
                    default: usage( );
                    }
        }
    }

    if( 0 != opt_statistics && 0 != opt_generate )
    {
        fprintf( stderr, "Error: Cannot set both -g and -s options\n" );
        return -1;
    }

    if( 0 == precanned )
    {
        open_precanned( PRECANNED );

        /* Fallback - try current working directory */
        if( 0 == precanned )
            open_precanned( "precanned" );

        /* Else, can continue happily without any precanned files... */
    }

    if( 0 != opt_statistics || 0 != opt_solve )
    {
        if( 0 == precanned )
        {
            fprintf( stderr, "Error: no precanned boards loaded\n" );
            return -1;
        }
        return gen_statistics( );
    }

    srand( time( 0 ) ^ getpid( ) );

    if( 0 == ftmplt )
    {
        open_template( TEMPLATE );

        /* Fallback - try current working directory */
        if( 0 == ftmplt )
        {
            open_template( "template" );
            if( 0 == ftmplt )
            {
                fprintf( stderr, "Error: failed to open template file\n" );
                exit( 1 );
            }
        }
    }

    if( 0 != opt_generate )
    {
        /* -g0 generates many boards */
        if( 0 == num_generate )
            --num_generate;
        while( 0 != num_generate-- )
        {
            generate( );
            print( stdout, title );
        }
        return 0;
    }

    if( 0 == opt_random && 0 == precanned )
    {
        fprintf( stderr, "Error: option -n requires precanned boards\n" );
        return 1;
    }

    if( !isatty( 0 ) || !isatty( 1 ) )
    {
        fprintf( stderr, "Error: stdin/out cannot be redirected\n" );
        return 1;
    }

    /* Establish process environment */
    if( 0 == initscr( ) )
    {
        fprintf( stderr,
                 "Error: failed to initialise curses screen library\n" );
        return 1;
    }

    /* Any signal will now shutdown curses cleanly */
    signals( );

    draw_screen( );
    noecho( );
    raw( );

    load_board( );

    ch = ' ';
    while( 'q' != ch )
    {
        wrefresh( stdscr );
        ch = getkey( );

        if( have_status )
            clear_status( );

        switch( ch )
        {
        case '.':
            ch = '0'; /* Drop thru' to digit handler */
        case '0':  /* clear location, or add digit to board */
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if( !IS_FIXED( INDEX( cury, curx ) ) )
            {
                num_hints = -1;
                fillx( INDEX( cury, curx ), ch - '0' );
                addch( '0' == ch ? '.' : ch );
                move_to( curx, cury );
                if( have_hint )
                    clear_hints( );
                req_hints = 0;
            }
            else
                beep( );
            break;
        case ' ':  /* move to next */
            move_next( );
            break;
        case 'h':  /* move left */
        case VKEY_LEFT:
            if( 0 < curx )
                move_to( curx - 1, cury );
            else
            	move_to( 8, cury );
            break;
        case 'j': /* move down */
        case VKEY_DOWN:
            if( cury < 8 )
                move_to( curx, cury + 1 );
            else
                move_to( curx, 0 );
            break;
        case 'k': /* move up */
        case VKEY_UP:
            if( 0 < cury )
                move_to( curx, cury - 1 );
            else
                move_to( curx, 8 );
            break;
        case 'l': /* move right */
        case VKEY_RIGHT:
            if( curx < 8 )
                move_to( curx + 1, cury );
            else
                move_to( 0, cury );
            break;
        case 'u': /* undo last move */
            if( history[ idx_history - 1 ] & FIXED )
                beep( );
            else
            {
                /* Don't ignore last reference (if exists) */
                for( i = --idx_history - 1 ; 0 <= i ; --i )
                    if( GET_INDEX( history[ i ] )
                        == GET_INDEX( history[ idx_history ] ) )
                    {
                        history[ i ] &= ~IGNORED;
                        break;
                    }
                num_hints = -1;
                if( have_hint )
                    clear_hints( );
                req_hints = 0;
                reapply( );
                render( );
                if( history[ idx_history - 1 ] & FIXED )
                {
                    curx = cury = 8;
                    move_next( );
                }
                else
                    move_to( ROW( GET_INDEX( history[ idx_history ] ) ),
                             COLUMN( GET_INDEX( history[ idx_history ] ) ) );
            }
            break;
        case 'c': /* clear board */
            completed = 0;
            num_hints = -1;
            if( have_hint )
                clear_hints( );
            reset( );
            render( );
            write_title( 0 );
            move_to( 0, 0 );
            break;
        case 'f': /* fix squares (if possible) */
            if( 0 == idx_history || 0 != ( history[ 0 ] & FIXED ) )
                break;
            if( 0 != fix( ) )
            {
                set_status( "There is an error - no solution possible!" );
                beep( );
            }
            break;
        case 'n': /* load new board */
            if( have_hint )
                clear_hints( );
            reset( );
            render( );
            write_title( 0 );
            wrefresh( stdscr );
            load_board( );
            break;
        case 'v': /* show solution */
            clear_moves( );
            if( have_hint )
                clear_hints( );
            num_hints = -1;
            if( 0 == solve( ) )
                completed = 1;
            else
                beep( );
            render( );
            move_to( curx, cury );
            break;

        /* The main difficulty with saving a board is managing the
         * user interface.
         */
        /*case 's':
            if( 0 == opt_restrict )
                save_board( );
            break;
		*//*temporarily disabled until saving is fixed*/
        case '?': /* Request hint */
            ++req_hints;
            if( have_hint )
                clear_hints( );
            if( -1 == num_hints )
            {
                last_hint = -1;
                num_hints = findhints( );
            }
            if( 0 == num_hints )
                set_status( "No hints available!" );
            else
            {
                int n = 0;

                if( 1 < num_hints )
                    do
                        i = rand( ) % num_hints;
                    while( i == last_hint );
                else
                    i = 0;
                idx = GET_INDEX( possible[ last_hint = i ] );

                /* Count # possible ways of expressing hint */
                if( 0 != ( HINT_ROW & possible[ i ] ) )
                    ++n;
                if( 0 != ( HINT_COLUMN & possible[ i ] ) )
                    ++n;
                if( 0 != ( HINT_BLOCK & possible[ i ] ) )
                    ++n;
                assert( 0 < n );
                if( 1 < n )
                    n = 1 + rand( ) % n;

                if( 0 != ( HINT_ROW & possible[ i ] ) )
                    if( 0 == --n )
                        row_hint( ROW( idx ) );
                if( 0 != ( HINT_COLUMN & possible[ i ] ) )
                    if( 0 == --n )
                        column_hint( COLUMN( idx ) );
                if( 0 != ( HINT_BLOCK & possible[ i ] ) )
                    if( 0 == --n )
                        block_hint( IDX_BLOCK( ROW(idx), COLUMN(idx) ) );

                if( opt_spoilerhint ) /* Useful for testing... */
                {
                    sprintf( statusline, "%d @ row %d, column %d",
                             GET_DIGIT( possible[ i ] ),
                             ROW(idx)+1, COLUMN(idx)+1 );
                    set_status( statusline );
                }
                else
                if( 10 < req_hints || 2 * num_hints < req_hints )
                {
                    sprintf( statusline, "(try the digit %d)",
                             GET_DIGIT( possible[ i ] ) );
                    set_status( statusline );
                }
            }
            break;
        case 'q': /* quit */
            break;
        default:
            beep( );
        }

        /* Check for a solution */
        if( 0 == completed )
        {
            for( i = 0 ; i < 81 && !IS_EMPTY( i ) ; ++i )
                ;
            if( 81 == i && 0 == fix( ) )
            {
                beep( );
                set_status( "Well done - you've completed the puzzle!" );
                completed = 1;
            }
        }
    }

    move( 23, 0 );
    endwin( );

    return 0;
}

