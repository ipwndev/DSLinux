/*************************************************************************

 4 Operation Calculator for the Linux Console
 By Raymond Jones<SeeDX@disinfo.net> Feb 2nd/2000
 
 This is just a basic 4 operation calculator for Linux that uses ANSI
 color cods instead of using curses.

***************************************************************************/



#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define QUIT_PROG 5
unsigned int get_menu_choice(void);
int display_asterisk_line(void);
int clear_screen_buffer(void);
int product(int x, int y);
int sum(int x, int y);
int divi(int x, int y);
int dif(int x, int y);
int oper1(void);
int oper2(void);
int oper3(void);
int oper4(void);

	
int		main(int argv, char *argc[])

{



		int choice = 0;
				while(choice != QUIT_PROG)
				{
				choice = get_menu_choice();



if(choice == 1)


		oper1();


else



if(choice == 2)


		oper2();


else


if(choice == 3)


		oper3();



else



if(choice == 4)



		oper4();


else


if(choice == 5)

	
   printf("\a\n\033[40;34;01;05mBye for now. Any questions,comments forward them to SeeDX@disinfo.net!\n");

printf("\033[40;47;00m");
 
  }

return 0;
}




unsigned int get_menu_choice(void)

{

	int selection;


do
{
system("clear");
	printf("\n\t\t\t\033[40;34;01mWelcome to LCalc\n");
	printf("\t\t\t\033[40;35m**********************");
	printf("\t\t\t\n");
	printf("\t\t\t\033[40;32m1-aDD");
	printf("\n\t\t\t\033[40;33m2-SuBtRacT");
	printf("\n\t\t\t\033[40;35m3-MulTiplY");
	printf("\n\t\t\t\033[40;37m4-DiViDE");
	printf("\n\t\t\t\033[40;32m5-QuiT\n");
	printf("\t\t\t\033[40;35m**********************");
	printf("\n");
	printf("\n\t\t\033[40;31m Please Enter your SelectioN: ");
	scanf("%d", &selection);

	}while(selection < 1 || selection > 5);
	return selection;

}





int oper1(void)

{


		int a,b,c;


	printf("\n\t\t\tEnter a number value: ");
		scanf("%d", &a);

	printf("\n\t\t\tEnter another number value: ");
		scanf("%d", &b);


	c = sum(a,b);

	printf("\n\t\t\t%d + %d = %d", a,b,c);


return 0;
}



int oper2(void)

{

	
			int a,b,c;

	printf("\n\t\t\tEnter a number value: ");
		scanf("%d", &a);


	printf("\n\t\t\tEnter another number value: ");
		scanf("%d", &b);



	c = dif(a,b);


		printf("\n\t\t\t%d - %d = %d", a,b,c);

return 0;
}







int oper3(void)


{

		int a,b,c;



		printf("\n\t\t\tEnter a number value: ");
			scanf("%d", &a);



		printf("\n\t\t\tEnter another number value: ");
			scanf("%d", &b);



	c = product(a,b);


		printf("\n\t\t\t%d * %d = %d", a,b,c);


return 0;
}



int oper4(void)


{

		int a,b,c;


		printf("\n\t\t\tEnter a number value: ");
			scanf("%d", &a);


		printf("\n\t\tEnter another number value: ");
			scanf("%d", &b);


	c = divi(a,b);

		printf("\n\t\t\t%d / %d = %d", a,b,c);


return 0;

}



int product(int x, int y)



{

	return(x * y);

}



int sum(int x, int y)

{
	return(x + y);


}


int dif(int x, int y)

{

	return(x - y);


}


int divi(int x, int y)

{

	return(x / y);


}




int clear_screen_buffer(void)


{
	int count,count2;


		for(count = 0; count < 80; count++, printf("\n" ) )
			for(count2 = 0; count2  < 25; count2++)

		printf("");


return 0;
}






