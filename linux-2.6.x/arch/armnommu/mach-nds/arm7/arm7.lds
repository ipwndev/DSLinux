ENTRY(stext)

__iwram_start	=	0x03800000;
__iwram_end	=	0x03810000;
__sp_irq	=	__iwram_end - 0x60;
__sp_usr	=	__sp_irq    - 0x100;

SECTIONS
{
 . = __iwram_start;
 .text : {
  _stext = .;
  *(.text)
 }
}
