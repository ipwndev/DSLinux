
extern void swiWaitForVBlank( void );

int main( void )
{
	while(1)
	{
		swiWaitForVBlank();
	}
}
