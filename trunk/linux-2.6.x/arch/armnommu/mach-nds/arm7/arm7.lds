ENTRY(stext)
SECTIONS
{
 . = 0x02004000;
 .text : {
  _stext = .;
  *(.text)
 }
}
