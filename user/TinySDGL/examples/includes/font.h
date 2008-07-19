//void txtOut(float x, float y, char *s, float size );

GLuint fontBase = 59000;



float _A[] = { 1,1, 1,5, 5,5, 5,1, 5,3, 1,3, -1 };
float _B[] = { 1,1, 1,5, 3,5, 3,3, 1,3, 5,3, 5,1, 1,1, -1 };
float _C[] = { 5,1, 1,1, 1,5, 5,5, -1 };
float _D[] = { 1,1, 1,5, 3,5, 5,3, 5,1, 1,1, -1 };
float _E[] = { 5,1, 1,1, 1,3, 3,3, 1,3, 1,5, 5,5, -1 };
float _F[] = { 1,1, 1,3, 3,3, 1,3, 1,5, 5,5, -1 };
float _G[] = { 5,5, 1,5, 1,1, 5,1, 5,3, 2,3, -1 };
float _H[] = { 1,1, 1,5, 1,3, 5,3, 5,5, 5,1, -1 };
float _I[] = { 2,1, 4,1, 3,1, 3,5, 2,5, 4,5, -1 };
float _J[] = { 2,5, 5,5, 5,1, 1,1, 1,3, -1 };
float _K[] = { 1,1, 1,5, 1,3, 5,5, 1,3, 5,1, -1 };
float _L[] = { 1,5, 1,1, 5,1, -1 };
float _M[] = { 1,1, 1,5, 3,5, 3,3, 3,5, 5,5, 5,1, -1 };
float _N[] = { 1,1, 1,5, 5,1, 5,5, -1 };
float _O[] = { 1,1, 1,5, 5,5, 5,1, 1,1, -1 };
float _P[] = { 1,1, 1,5, 5,5, 5,3, 1,3, -1 };
float _Q[] = { 3,3, 5,1, 1,1, 1,5, 5,5, 5,1, -1 };
float _R[] = { 1,1, 1,5, 5,5, 5,3, 1,3, 5,1, -1 };
float _S[] = { 5,5, 1,5, 1,3, 5,3, 5,1, 1,1, -1 };
float _T[] = { 1,5, 5,5, 3,5, 3,1, -1 };
float _U[] = { 1,5, 1,1, 5,1, 5,5, -1 };
float _V[] = { 1,5, 3,1, 5,5, -1 };
float _W[] = { 1,5, 1,1, 3,1, 3,3, 3,1, 5,1, 5,5, -1 };
float _X[] = { 1,5, 5,1, 3,3, 1,1, 3,3, 5,5, -1 };
float _Y[] = { 1,5, 3,3, 3,1, 3,3, 5,5, -1 };
float _Z[] = { 1,5, 5,5, 1,1, 5,1, -1 };

float _a[] = { 1,3, 4,3, 4,1, 1,1, 1,2, 4,2, -1 };
float _b[] = { 1,5, 1,1, 4,1, 4,3, 1,3, -1 };
float _c[] = { 4,1, 1,1, 1,3, 4,3, -1 };
float _d[] = { 4,5, 4,1, 1,1, 1,3, 4,3, -1 };
float _e[] = { 4,1, 1,1, 1,3, 4,3, 4,2, 1,2, -1 };
float _f[] = { 3,0, 3,3, 2,3, 5,3, 3,3, 3,5, 5,5, -1 };
float _g[] = { 1,0, 4,0, 4,3, 1,3, 1,1, 4,1, -1 };
float _h[] = { 1,1, 1,5, 1,3, 4,3, 4,1, -1 };
float _i[] = { 2,1, 4,1, 3,1, 3,3, 2,3, 4,3, -1 };
float _j[] = { 3,3, 4,3, 4,0, 2,0, -1 };
float _k[] = { 1,1, 1,5, 1,2, 3,3, 1,2, 4,1, -1 };
float _l[] = { 2,5, 2,1, 3,1, -1 };
float _m[] = { 1,1, 1,3, 3,3, 3,1, 3,3, 5,3, 5,1, -1 };
float _n[] = { 1,1, 1,3, 4,3, 4,1, -1 };
float _o[] = { 1,1, 1,3, 4,3, 4,1, 1,1, -1 };
float _p[] = { 1,0, 1,3, 4,3, 4,1, 1,1, -1 };
float _q[] = { 4,0, 4,3, 1,3, 1,1, 4,1, -1 };
float _r[] = { 1,1, 1,3, 1,2, 4,3, -1 };
float _s[] = { 4,3, 1,3, 1,2, 4,2, 4,1, 1,1, -1 };
float _t[] = { 3,1, 3,3, 2,3, 5,3, 3,3, 3,5, -1 };
float _u[] = { 1,3, 1,1, 4,1, 4,3, -1 };
float _v[] = { 1,3, 3,1, 5,3, -1 };
float _w[] = { 1,3, 1,1, 3,1, 3,2, 3,1, 5,1, 5,3, -1 };
float _x[] = { 1,3, 5,1, 3,2, 1,1, 3,2, 5,3, -1 };
float _y[] = { 1,3, 1,1, 4,1, 4,3, 4,0, 1,0, -1 };
float _z[] = { 1,3, 4,3, 1,1, 4,1, -1 };

float _0[] = { 1,1, 1,5, 5,5, 5,1, 1,1, 5,5, -1 };
float _1[] = { 1,1, 5,1, 3,1, 3,5, 1,3, -1 };
float _2[] = { 1,5, 5,5, 5,3, 1,3, 1,1, 5,1, -1 };
float _3[] = { 1,5, 5,5, 5,3, 3,3, 5,3, 5,1, 1,1, -1 };
float _4[] = { 5,2, 1,2, 4,5, 4,1, -1 };
float _5[] = { 5,5, 1,5, 1,3, 5,3, 1,1, -1 };
float _6[] = { 1,5, 1,1, 5,1, 5,3, 1,3, -1 };
float _7[] = { 1,5, 5,5, 1,1, -1 };
float _8[] = { 1,3, 1,1, 5,1, 5,5, 1,5, 1,3, 5,3, -1 };
float _9[] = { 5,1, 5,5, 1,5, 1,3, 5,3, -1 };

float _dot[] = { 2,1, 3,1, -1 };
float _2dot[] = { 1,1, 3,1, 1,3, 1,1, -1 };
float _aposf[] = { 2,5, 3,3, -1 };
float _coma[] = { 2,0, 3,2, -1 };
float _plus[] = { 1,3, 5,3, 3,3, 3,5, 3,1, -1 };
float _minus[] = { 1,3, 4,3, -1 };
float _lbrac[] = { 3,5, 2,3, 3,1, -1 };
float _rbrac[] = { 3,5, 4,3, 3,1, -1 };
float _less[] = { 4,1, 1,3, 4,5, -1 };
float _great[] = { 1,1, 4,3, 1,5, -1 };
float _div[] = { 1,1, 4,5, -1 };
float _and[] = { 4,0, 4,3, 1,3, 1,5, 3,5, 3,3, 1,3, 1,1, 6,1, -1 };
float _equal[] = { 1,1, 1,5, 5,3, 1,1,  -1 };


void drawLetter(float l[])
{
	
	int i=0;

	glBegin(GL_LINE_STRIP);

		while(l[i]!= -1)
		{

			glVertex2f(l[i], l[i+1]);

			i+= 2;
		}
	glEnd();
	
	glTranslatef(6.0, 0.0, 0.0);	   

}

/*  Create a display list for each of 6 characters	*/
void initFONT(void)
{

   fontBase = glGenLists (128);
   glNewList(fontBase+'A', GL_COMPILE); drawLetter(_A); glEndList();
   glNewList(fontBase+'B', GL_COMPILE); drawLetter(_B); glEndList();
   glNewList(fontBase+'C', GL_COMPILE); drawLetter(_C); glEndList();
   glNewList(fontBase+'D', GL_COMPILE); drawLetter(_D); glEndList();
   glNewList(fontBase+'E', GL_COMPILE); drawLetter(_E); glEndList();
   glNewList(fontBase+'F', GL_COMPILE); drawLetter(_F); glEndList();
   glNewList(fontBase+'G', GL_COMPILE); drawLetter(_G); glEndList();
   glNewList(fontBase+'H', GL_COMPILE); drawLetter(_H); glEndList();
   glNewList(fontBase+'I', GL_COMPILE); drawLetter(_I); glEndList();
   glNewList(fontBase+'J', GL_COMPILE); drawLetter(_J); glEndList();
   glNewList(fontBase+'K', GL_COMPILE); drawLetter(_K); glEndList();
   glNewList(fontBase+'L', GL_COMPILE); drawLetter(_L); glEndList();
   glNewList(fontBase+'M', GL_COMPILE); drawLetter(_M); glEndList();
   glNewList(fontBase+'N', GL_COMPILE); drawLetter(_N); glEndList();
   glNewList(fontBase+'O', GL_COMPILE); drawLetter(_O); glEndList();
   glNewList(fontBase+'P', GL_COMPILE); drawLetter(_P); glEndList();
   glNewList(fontBase+'Q', GL_COMPILE); drawLetter(_Q); glEndList();
   glNewList(fontBase+'R', GL_COMPILE); drawLetter(_R); glEndList();
   glNewList(fontBase+'S', GL_COMPILE); drawLetter(_S); glEndList();
   glNewList(fontBase+'T', GL_COMPILE); drawLetter(_T); glEndList();
   glNewList(fontBase+'U', GL_COMPILE); drawLetter(_U); glEndList();
   glNewList(fontBase+'V', GL_COMPILE); drawLetter(_V); glEndList();
   glNewList(fontBase+'W', GL_COMPILE); drawLetter(_W); glEndList();
   glNewList(fontBase+'X', GL_COMPILE); drawLetter(_X); glEndList();
   glNewList(fontBase+'Y', GL_COMPILE); drawLetter(_Y); glEndList();
   glNewList(fontBase+'Z', GL_COMPILE); drawLetter(_Z); glEndList();

   glNewList(fontBase+'a', GL_COMPILE); drawLetter(_a); glEndList();
   glNewList(fontBase+'b', GL_COMPILE); drawLetter(_b); glEndList();
   glNewList(fontBase+'c', GL_COMPILE); drawLetter(_c); glEndList();
   glNewList(fontBase+'d', GL_COMPILE); drawLetter(_d); glEndList();
   glNewList(fontBase+'e', GL_COMPILE); drawLetter(_e); glEndList();
   glNewList(fontBase+'f', GL_COMPILE); drawLetter(_f); glEndList();
   glNewList(fontBase+'g', GL_COMPILE); drawLetter(_g); glEndList();
   glNewList(fontBase+'h', GL_COMPILE); drawLetter(_h); glEndList();
   glNewList(fontBase+'i', GL_COMPILE); drawLetter(_i); glEndList();
   glNewList(fontBase+'j', GL_COMPILE); drawLetter(_j); glEndList();
   glNewList(fontBase+'k', GL_COMPILE); drawLetter(_k); glEndList();
   glNewList(fontBase+'l', GL_COMPILE); drawLetter(_l); glEndList();
   glNewList(fontBase+'m', GL_COMPILE); drawLetter(_m); glEndList();
   glNewList(fontBase+'n', GL_COMPILE); drawLetter(_n); glEndList();
   glNewList(fontBase+'o', GL_COMPILE); drawLetter(_o); glEndList();
   glNewList(fontBase+'p', GL_COMPILE); drawLetter(_p); glEndList();
   glNewList(fontBase+'q', GL_COMPILE); drawLetter(_q); glEndList();
   glNewList(fontBase+'r', GL_COMPILE); drawLetter(_r); glEndList();
   glNewList(fontBase+'s', GL_COMPILE); drawLetter(_s); glEndList();
   glNewList(fontBase+'t', GL_COMPILE); drawLetter(_t); glEndList();
   glNewList(fontBase+'u', GL_COMPILE); drawLetter(_u); glEndList();
   glNewList(fontBase+'v', GL_COMPILE); drawLetter(_v); glEndList();
   glNewList(fontBase+'w', GL_COMPILE); drawLetter(_w); glEndList();
   glNewList(fontBase+'x', GL_COMPILE); drawLetter(_x); glEndList();
   glNewList(fontBase+'y', GL_COMPILE); drawLetter(_y); glEndList();
   glNewList(fontBase+'z', GL_COMPILE); drawLetter(_z); glEndList();

   glNewList(fontBase+'0', GL_COMPILE); drawLetter(_0); glEndList();
   glNewList(fontBase+'1', GL_COMPILE); drawLetter(_1); glEndList();
   glNewList(fontBase+'2', GL_COMPILE); drawLetter(_2); glEndList();
   glNewList(fontBase+'3', GL_COMPILE); drawLetter(_3); glEndList();
   glNewList(fontBase+'4', GL_COMPILE); drawLetter(_4); glEndList();
   glNewList(fontBase+'5', GL_COMPILE); drawLetter(_5); glEndList();
   glNewList(fontBase+'6', GL_COMPILE); drawLetter(_6); glEndList();
   glNewList(fontBase+'7', GL_COMPILE); drawLetter(_7); glEndList();
   glNewList(fontBase+'8', GL_COMPILE); drawLetter(_8); glEndList();
   glNewList(fontBase+'9', GL_COMPILE); drawLetter(_9); glEndList();

   glNewList(fontBase+'.', GL_COMPILE); drawLetter(_dot); glEndList();
   glNewList(fontBase+':', GL_COMPILE); drawLetter(_2dot); glEndList();
   glNewList(fontBase+'\'', GL_COMPILE); drawLetter(_aposf); glEndList();
   glNewList(fontBase+',', GL_COMPILE); drawLetter(_coma); glEndList();
   glNewList(fontBase+'+', GL_COMPILE); drawLetter(_plus); glEndList();
   glNewList(fontBase+'-', GL_COMPILE); drawLetter(_minus); glEndList();
   glNewList(fontBase+'(', GL_COMPILE); drawLetter(_lbrac); glEndList();
   glNewList(fontBase+')', GL_COMPILE); drawLetter(_rbrac); glEndList();
   glNewList(fontBase+'<', GL_COMPILE); drawLetter(_less); glEndList();
   glNewList(fontBase+'>', GL_COMPILE); drawLetter(_great); glEndList();
   glNewList(fontBase+'/', GL_COMPILE); drawLetter(_div); glEndList();
   glNewList(fontBase+'&', GL_COMPILE); drawLetter(_and); glEndList();
   glNewList(fontBase+'=', GL_COMPILE); drawLetter(_equal); glEndList();

   glNewList(fontBase+' ', GL_COMPILE); glTranslatef(6.0, 0.0, 0.0); glEndList();
}


void txtOut(float x, float y, char *s, float size )
{


   glPushMatrix();

	 glTranslatef(x, y+20, 0);
	 glScalef( size*0.75, -size, 0);

	   GLsizei len = strlen(s);
           int i;
	   for(i=0;i<len;i++)
	     glCallList( fontBase+s[i] );
		
   
   glPopMatrix();

}

