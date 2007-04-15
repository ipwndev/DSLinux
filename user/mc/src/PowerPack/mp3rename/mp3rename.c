#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// space karakter:  lehet '_' is!
#define SPC ' '

void renamer(char *name){
    char temp[256];
    char *s=name;
    char *d=temp;
    int c;
    int prev_c=SPC;
    while((c=*s++)){

	// idezojeleket, !-et, ?-et kihagyjuk mert sux:
	if(c=='`' || c==39 || c==34 || c=='!' || c=='?') continue;

	// egyseges space (lasd fent SPC-t)
	if(c==' ' || c=='_') c=SPC;

	// egymas utan eleg 1 space
	if(c==SPC && prev_c==SPC) continue;

	// egyseges zarojelek
	if(c=='(' || c=='{') c='[';
	if(c==')' || c=='}') c=']';

	// plusz space-k beszurasa kotojel ele/moge es zarojel ele ha nincs:
	if( (c=='-' && prev_c!=SPC && prev_c!='-') ||
	    (c!=SPC && prev_c=='-' && c!='-') ||
	    (c=='[' && prev_c!=SPC) ) *d++=prev_c=SPC;

	// sor es szo elejen nagybetu, tobbi kicsi:
	if(c>='a' && c<='z' && (prev_c==SPC || prev_c=='[')) c-=32; // upcase
	if(c>='A' && c<='Z' &&  prev_c!=SPC && prev_c!='[') c+=32;  // downcase

	*d++=c;prev_c=c;
    }
    *d=0;
//    printf("renamed to '%s'\n",temp);
    rename(name,temp);

}

int main(int argc,char* argv[]){
int i;
for(i=1;i<argc;i++) renamer(argv[i]);
return 0;
}
