#include <asm/hardware.h>
#include <asm/io.h>

static __inline__ void do_outw(unsigned int reg, const void* buf, unsigned int count)
{
	register const unsigned short* wbuf = (const unsigned short*) buf;
	while(count--)
		outw(*wbuf++, reg);
}

static __inline__ void do_inw(unsigned int reg, void* buf, unsigned int count)
{
	register unsigned short* wbuf = (unsigned short*) buf;
	while(count--)
		*wbuf++  = inw(reg);
}


void outsw(unsigned to_reg, const void *from, int len_in_words)
{
	do_outw(to_reg, from, len_in_words);
}

void outswb(unsigned  to_reg, const void *from, int len_in_bytes)
{
	do_outw(to_reg, from, len_in_bytes/2);
}



void insw(unsigned from_port, void *to, int len_in_words)
{
	do_inw(from_port, to, len_in_words);
}

void  inswb(unsigned from_port, void *to, int len_in_bytes)
{
	do_inw(from_port, to, len_in_bytes/2);
}


void arch_hard_reset (void)
{
	for (;;)
		;
}

