//#include <iostream.h>
#include <stdio.h> 

class test1 {
public:
test1();
~test1();

virtual const char *self(void);
virtual const char *selfa(void);
virtual const char *selfb(void);
virtual const char *selfc(void);

};

test1::test1() {
	puts("constructor test1\n");
}

test1::~test1() {
	puts("destructor test1\n");
}

const char *test1::self(void) {
	return "test 1";
}

const char *test1::selfa(void) {
	return "test 1a";
}

const char *test1::selfb(void) {
	return "test 1b";
}

const char *test1::selfc(void) {
	return "test 1c";
}

/*
class test2: public test1 {
public:
test2();
virtual ~test2();

virtual const char *self(void);

};

test2::test2() {
	puts("constructor test2\n");
}

test2::~test2() {
	puts("destructor test2\n");
}

const char *test2::self(void) {
	return "test 2";
}
*/


int main()
 {
	test1 *bubi;
//	unsigned int *x;

	puts("start: before new\n");

	bubi = new test1;

//	puts("after new\n");

	/* get address of object */
//	x = (unsigned int *) bubi;
//	printf("%X\n ", x[0]);

	// get vtable contents
//	x = (unsigned int *) x[0];
//	printf("%X ", x[0]);
//	printf("%X ", x[1]);
//	printf("%X\n ", x[2]);

	puts( bubi->self());

	delete bubi;

    	// Dies ist das Hello-World-Programm
//    	cout<<"Hello, world!"<<endl;
	
//	printf("Hello");


	return 0;
 };
