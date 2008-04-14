/* Bledy raportowac na degrade16@hotmail.com 
*/

#include "cpscan.h"
#define PORT_LIMIT 1024 /* Defina aqui o default */

int x;
struct sockaddr_in addr;
char rhost[100];

int scaner(port)
int port;
{
 int s;
    x = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (x < 0) {
       say("\e[1;34mBlad:\e[1;37m socket() failed\e[1;00m\n");
       exit(0);
    }

    addr.sin_family = PF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(rhost);

    s = connect(x,(struct sockaddr *) &addr, sizeof(addr));

    close(x);
    if (s==-1) {
       return (1 == 0);
    }

    return (1 == 1);
}

main(argc,argv) 
int argc;
char *argv[];

 {int a,b,c,d,e,f;
 struct hostent *nuk;
 struct servent *tec;

   if (argc < 2) {

fprintf(stderr,"\e[1;34mUzywanie:\e[1;37m %s \e[1;37m<\e[1;34madres\e[1;37m> \e[1;37m<\e[1;34mnajwyzszy port\e[1;37m>\n\e[1;00m",argv[0]);
      exit(0);}

   if (sscanf(argv[1],"%d.%d.%d.%d",&a,&b,&c,&d) != 4) {
      nuk = gethostbyname(argv[1]);
      if (nuk == NULL) {
         fprintf(stderr,"\e[1;34mBlad:\e[1;37m nie mozna znalezc zadanego adresu \e[1;34m%s\n\e[1;00m",argv[1]);
         exit(0);
      }
      sprintf(rhost,"%d.%d.%d.%d",(unsigned char )nuk->h_addr_list[0][0],
              (unsigned char ) nuk->h_addr_list[0][1], 
              (unsigned char ) nuk->h_addr_list[0][2], 
              (unsigned char ) nuk->h_addr_list[0][3]);
   } else {
      strncpy(rhost,argv[1],99);
   }


   if (argc > 2) {
      f = atoi(argv[2]);
   } else
      f = PORT_LIMIT;

   fprintf(stdout,"\e[1;34mSkanowanie %s - \e[1;36mportów \e[1;31m1 \e[1;36mprzez \e[1;31m%d\n",rhost,f);

   for (e=1;e<=f;e++) {
    char serv[100];
      if (scaner(e)) {
         tec = getservbyport(htons(e),"tcp"); 

	say("\e[1;37m[\e[1;31m%d\e[1;37m]\e[1;37m (\e[1;34m%s\e[1;37m)\e[1;34m jest uzywane .\e[1;00m\n",e,(tec == NULL) ? "UNKNOWN" : tec->s_name);  }
   }
}

