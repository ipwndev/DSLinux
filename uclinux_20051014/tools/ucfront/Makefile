HOSTCC ?= cc

SRC=ucfront.c args.c util.c execute.c vasprintf.c

ucfront: $(SRC)
	$(HOSTCC) -g -Werror -Wall -DHAVE_MKSTEMP -o $@ $(SRC)

clean:
	$(RM) -f *.o ucfront core tags

ucfront.1: ucfront.yo
	-yodl2man -o ucfront.1 ucfront.yo
