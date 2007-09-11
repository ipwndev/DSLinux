#include <iostream>

using namespace std;


class test1 {
public:
test1();
virtual ~test1();

virtual const char *self(void);
};

test1::test1() {
	cout << "constructor test1" << endl;
}

test1::~test1() {
	cout << "destructor test1" << endl;
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
	test2 *bubi;

	cout << "start: before new" << endl;

	bubi = new test2;

	cout << bubi->self()<< endl;

	delete bubi;

	return 0;
 };
