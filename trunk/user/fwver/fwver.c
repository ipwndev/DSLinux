
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#define FWVER_DEBUG

/* cake originally from http://www.myhairyass.com/ascii/Art/?ID=20 */
#define CAKE_WIDTH 39
#define CAKE_HEIGHT 15
static char cake[CAKE_HEIGHT][CAKE_WIDTH]= {
	"                0   0                \n",
	"                |   |                \n",
	"            ____|___|____            \n",
	"         0  |~ ~ ~ ~ ~ ~|   0        \n",
	"         |  |           |   |        \n",
	"      ___|__|___________|___|__      \n",
	"      |/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/|      \n",
	"  0   |       H a p p y       |   0  \n",
	"  |   |/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/|   |  \n",
	"__|___|_______________________|___|__\n",
	"|/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/|\n",
	"|                                   |\n",
	"|         B i r t h d a y! ! !      |\n",
	"| ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ |\n",
	"|___________________________________|\n",
};


/* first byte at this offset is bday month,
 * second byte at this offset is bday day */
#define FW_BDAY_OFFSET	0x03FE03

static void print_cake()
{
	int i;
	for (i = 0; i < CAKE_HEIGHT; i++)
		printf(cake[i]);
	sleep(5);
}

static int its_birthday(FILE *firmware)
{
	int day = 0;
	int month = 0;
	time_t t = 0;
	struct tm *tm = NULL;

	t = time(NULL);
	tm = localtime(&t);
	fseek(firmware, FW_BDAY_OFFSET, SEEK_SET);
	fread(&month, 1, 1, firmware);
	fread(&day, 1, 1, firmware);
#ifdef FWVER_DEBUG
	printf("birthday day: %i\n", day);
	printf("birthday month: %i\n", month);
	printf("current day: %i\n", tm->tm_mday);
	printf("current month: %i\n", tm->tm_mon);
	day = tm->tm_mday;
	month = tm->tm_mon;
#endif
        return ((day == tm->tm_mday) && (month-1 == tm->tm_mon));
        /* month-1 because the month returned by the system is 0..11
         * while the month in the firmware is 1..12 */
 
}


int main( void )
{
    FILE * firmware ;
    unsigned short data ;
    unsigned short checksum ;
    unsigned short flashmever;
    int version = 0;

    firmware = fopen( "/dev/firmware", "r" ) ;
    if ( ! firmware )
    {
        perror( "/dev/firmware" ) ;
        exit(1);
    }

    if (its_birthday(firmware))
	    print_cake();

    fseek( firmware, 0x17c ,SEEK_SET ) ;
    fread( &data, sizeof(short), 1, firmware ) ;

    fseek( firmware, 0x6 ,SEEK_SET ) ;
    fread( &checksum, sizeof(short), 1, firmware ) ;

    fseek( firmware, 0x3f7fc ,SEEK_SET ) ;
    fread( &flashmever, sizeof(short), 1, firmware ) ;

    fclose( firmware ) ;

    switch ( checksum )
    {
        case 0x2C7A:
            version = 1 ;
            break ;
        case 0xE0CE:
            version = 2 ;
            break ;
        case 0xBFBA:
            version = 3 ;
            break ;
        case 0xDFC7:
            version = 4 ;
            break ;
        case 0x73b3:
            version = 5 ;
            break ;
        case 0xF96D:
            printf("iQue firmware detected.\n");
            version = -1 ;
            break ;
        case 0xE843:
            printf("DS Lite firmware detected.  It is safe to run online games.\n");
            version = -1 ;
            break ;
        case 0x0f1f:
            printf("European DS Lite firmware detected.\nIt is safe to run online games.\n");
            version = -1 ;
            break ;
    }

    if ( version > 0 )
    {
        printf("Nintendo firmware v%d detected.  It is safe to run online games.\n",version);
    }
    if ( version >= 4 )
    {
        printf("Your PassMe2 seems to be working!\n");
    }

    if ( version == 0 )
    {
        if ( data == 1 )
        {
            printf("Old FlashMe detected.  Upgrade before running online games.\n");
        }
        else if ( data == 2 )
        {
            printf("FlashMe v%d detected.  It is safe to run online games.\n", flashmever + 3);
        }
        else
        {
            printf("Unknown firmware.\n");
        }
    }
    exit(0);
}

