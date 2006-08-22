HOSTCC ?= cc

UTIL=args.c util.c execute.c

all: ucfront

ucfront: ucfront.c $(UTIL)
	$(HOSTCC) -g -Werror -Wall -DHAVE_MKSTEMP -o $@ $^

clean:
	$(RM) -f *.o ucfront core tags

ucfront.1: ucfront.yo
	-yodl2man -o ucfront.1 ucfront.yo
