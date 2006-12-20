
#include <stdio.h>

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
#if 0
        case 0xUNKNOWN:
            version = 5 ;
            break ;
#endif
        case 0xF96D:
            printf("iQue firmware detected.\n");
            version = -1 ;
            break ;
        case 0xE843:
            printf("DS Lite firmware detected.  It is safe to run online games.\n");
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

