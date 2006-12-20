#include <stdlib.h>
#include <stddef.h>
#include <new>

const nothrow_t nothrow = {};

void * operator new(size_t size, const nothrow_t&)
{
	return malloc(size);
}

void operator delete(void *p)
{
	return free(p);
}
