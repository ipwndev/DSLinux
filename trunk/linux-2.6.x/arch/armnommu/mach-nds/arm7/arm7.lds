ENTRY(stext)
SECTIONS
{
 . = 0x03800000;
 .text : {
  _stext = .;
  *(.text)
 }
}
