//#include <iostream.h>
#include <stdio.h> 

class test1 {
public:
test1();
virtual ~test1();

virtual const char *self(void);

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



int main()
 {
	test1 *bubi;

	bubi = new test1;

	puts( bubi->self());

	delete bubi;

    	// Dies ist das Hello-World-Programm
//    	cout<<"Hello, world!"<<endl;
	
//	printf("Hello");


	return 0;
 };
