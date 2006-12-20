
#include "libvideogfx/utility/bitstream/bitbuffer.hh"
#include <iostream.h>

void ShowBuffer(const uint8* b,int len)
{
  for (int i=0;i<len;i++)
    {
      if (i>0 && i%8==0) cout << endl;
      if (i%8 != 0) cout << " ";

      for (int n=0x80;n;n>>=1)
	if (b[i]&n) cout << '1';
	else cout << '0';
    }

  cout << endl;
}

main()
{
  BitBuffer buf;

  buf.WriteBitsMasked(~0,7);
  buf.WriteBitsMasked( 0,14);
  buf.WriteBitsMasked(~0,20);

  buf.WriteBitsMasked( 0,1);
  buf.WriteBitsMasked(~0,7);
  buf.WriteBitsMasked( 0,14);
  buf.WriteBitsMasked(~0,20);

  buf.WriteBitsMasked( 0,1);
  buf.WriteBitsMasked(~0,7);
  buf.WriteBitsMasked( 0,14);
  buf.WriteBitsMasked(~0,20);

  buf.Flush();

  ShowBuffer(buf.AskBuffer(),buf.AskBufferSize());
}
