
0 1 2 3 4 5 6 7 8 9

#include <stdio.h>     /* hello */
#include <stdlib.h>

#define MAX_HUFF_KOD 0x110  /* hellow */
# define forall(i,n) for(i=0;i<(n);i++)

#if hello=5
#else

/* ez
itt
egy
komment */

  /* Belso hasznalatra! */
static short huff_tabl_sorted[MAX_HUFF_KOD];
static short *huff_tabl_sorted_ptr;
static int huff_tabl_size;
static short huff_bits_counter[17];

typedef struct {
  short stat[256];
  short dyn[MAX_HUFF_KOD*2];
} huff_tree;

void make_huffman_bits(int code_db,int szaml_tabl[],int bits_tabl[])
{
int ptr_tabl1[MAX_HUFF_KOD];
int ptr_tabl2[MAX_HUFF_KOD];
int szaml_nextptr[MAX_HUFF_KOD+1];
int i;
int si,di,ax,bx,cx,dx,bp;

forall(i,code_db){
  ptr_tabl1[i]=i;
  ptr_tabl2[i]=-1;
  bits_tabl[i]=0;
}

di=-1;
forall(i,code_db){
  if(szaml_tabl[i]>0){
    szaml_nextptr[di+1]=i;
    di=i;
  }
}
szaml_nextptr[di+1]=-1;

focikl:

/* KET LEGKISEBB SZAM KERESESE */
bx=ax=0x7fff;
si=szaml_nextptr[0];dx=-1;
while(si>=0){
  if(szaml_tabl[si]<ax){
    /*  ax,di  bx,bp  */
    ax=szaml_tabl[si];di=si;
    if(ax<bx){
      i=ax;ax=bx;bx=i;
      i=di;di=bp;bp=i;
      cx=dx;
    }
  }
  dx=si;si=szaml_nextptr[si+1];
}
if(ax==0x7fff) return;    /* !!!!!!!! */

/* A 2 SZAM OSSZEVONASA: */
szaml_tabl[di]=ax+bx;
szaml_tabl[bp]=0;
szaml_nextptr[cx+1]=szaml_nextptr[bp+1];

/* Lancolt lista #1: */
si=bp;
do{
  bx=ptr_tabl1[si];
  si=ptr_tabl2[si];
  ++bits_tabl[bx];
}while(si>=0);

/* Lancolt lista #2: */
si=di;
do{
  bx=ptr_tabl1[si];
  ++bits_tabl[bx];
  i=si;
  si=ptr_tabl2[si];
}while(si>=0);
ptr_tabl2[i]=bp; /* lista1 hozzafuzese a lista2 vegehez */

goto focikl;
}


void print_bin(int d,int i){
int j;
  forall(j,i){
     printf("%c",d&0x8000 ? '1' : '0');
     d<<=1;
  }
  printf("\n");
}

void huff_make_node(huff_tree* tree,int EBX,int EDI,int AX){
int ECX;
unsigned int tmp;

if((--huff_bits_counter[EBX])>=0){
   ECX=AX;
   AX=(*huff_tabl_sorted_ptr++);
   tree->dyn[EDI]=-AX;
   if(EBX>8) return;
   EDI=ECX; ECX=EBX; 
   tmp=EDI<<ECX; EDI=( tmp | (tmp>>16) )&255;
   AX|=(EBX<<11);
   EBX=(1<<ECX);
   do{
     tree->stat[EDI]=AX;
     EDI+=EBX;
   } while(EDI<0x100);
   return;
}

  tree->dyn[EDI]=2*huff_tabl_size;
  if(EBX==8){
    EDI= ((AX&0xFF)<<8) | (AX>>8);
    tree->stat[EDI]=(2*huff_tabl_size)|0xF800;
  }
  EDI=huff_tabl_size; huff_tabl_size+=2; ++EBX;
  AX>>=1;
  huff_make_node(tree,EBX,EDI,AX);
  AX|=0x8000; ++EDI;
  huff_make_node(tree,EBX,EDI,AX);

}


int huffman_bits_2_tree(int code_db,int bits_tabl[],huff_tree *tree){
int first[17];
int next[MAX_HUFF_KOD];
int i,j,b,d;
FILE *f;

forall(i,17){ first[i]=-1; huff_bits_counter[i]=0;}

forall(i,code_db){
  b=bits_tabl[i];
  ++huff_bits_counter[b];
  next[i]=first[b];
  first[b]=i;
}

j=0;
for(i=1;i<17;i++){
  d=first[i];
  while(d>=0){
    huff_tabl_sorted[j++]=d;
    d=next[d];
  }
}

if(j==0) return 1;  /* minden kod nem lehet 0 hosszu */

/*
f=fopen("sorted.dat","wb");
fwrite(huff_tabl_sorted,sizeof(short),j,f);
fclose(f);
*/

huff_bits_counter[0]=0;
huff_tabl_sorted_ptr=huff_tabl_sorted;  /* ESI */
huff_tabl_size=1;                       /* EDX */
huff_make_node(tree,0,0,0);

return 0;
}



main(){
short int szaml_tabl[256];
int szaml_tabl2[256];
int bits_tabl[256];
char bits2_tabl[256];
int data_tabl[256];
huff_tree huffti1;
FILE *f;
int i,j,d;

f=fopen("szaml.dat","rb");
fread(szaml_tabl,2,256,f);
fclose(f);

forall(j,100){
  forall(i,256) szaml_tabl2[i]=szaml_tabl[i];
  make_huffman_bits(256,szaml_tabl2,bits_tabl);
/*
forall(i,256) bits2_tabl[i]=bits_tabl[i];
f=fopen("bits.dat","wb");
fwrite(bits2_tabl,1,256,f);
fclose(f);
*/
  huffman_bits_2_tree(256,bits_tabl,&huffti1);
}

f=fopen("huffti1.dat","wb");
fwrite(huffti1.stat,sizeof(short),256+huff_tabl_size,f);
fclose(f);

printf("EDI=%d\n",2*huff_tabl_size);

}






