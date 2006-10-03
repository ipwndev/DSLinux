
#include "vpostproc/buffer.hh"
#include "video12/constants.hh"


#define FLAG_InQueue 1
#define FLAG_InUse   2
#define FLAG_NotAllocated 4

VideoImageBuffer::VideoImageBuffer(int MaxPoolSize)
  : d_queuelength(0),
    d_maxqueuelength(MaxPoolSize),
    d_maxpoolsize(MaxPoolSize)
{
  d_queue      = new DecodedImageData*[d_maxpoolsize];
  d_bufferpool = new DecodedImageData*[d_maxpoolsize];
  d_flags      = new uint8[d_maxpoolsize];

  for (int i=0;i<d_maxpoolsize;i++)
    d_flags[i]=FLAG_NotAllocated;
}


VideoImageBuffer::~VideoImageBuffer()
{
  for (int i=0;i<d_maxpoolsize;i++)
    {
      //cout << (d_flags[i] & (FLAG_InUse|FLAG_InQueue)) << endl;
      //assert((d_flags[i] & (FLAG_InUse|FLAG_InQueue))==0);
    }

  for (int i=0;i<d_maxpoolsize;i++)
    {
      if (!(d_flags[i] & FLAG_NotAllocated))
	{
	  if (d_bufferpool[i]->m_picdata1) delete d_bufferpool[i]->m_picdata1;
	  if (d_bufferpool[i]->m_picdata2) delete d_bufferpool[i]->m_picdata2;
	  delete d_bufferpool[i];
	}
    }

  delete[] d_bufferpool;
  delete[] d_queue;
  delete[] d_flags;
}


bool VideoImageBuffer::BufferFull() const
{
  return d_queuelength == d_maxqueuelength;
}


DecodedImageData* VideoImageBuffer::GetNextImage()
{
  Assert(d_queuelength>=1); // queue not empty

  DecodedImageData* d=d_queue[0];

  for (int i=0;i<d_queuelength-1;i++)
    d_queue[i]=d_queue[i+1];
  d_queuelength--;

  return d;
}


void VideoImageBuffer::FreeImage(DecodedImageData* d)
{
  for (int i=0;i<d_maxpoolsize;i++)
    if (d_bufferpool[i]==d)
      d_flags[i] &= ~FLAG_InQueue;
}

#if 0
DecodedImageData* VideoImageBuffer::GetOutputBuffer(ImageSpec_YUV spec,
						    uint3 picture_type,
						    bool maymodify)
{
  Assert(0);
#if 0
  for (int i=0;i<d_maxpoolsize;i++)
    if ((d_flags[i] & (FLAG_InUse|FLAG_InQueue))==0)
      {
	if (d_flags[i] & FLAG_NotAllocated)
	  {
#if 0
	    cout << "ALLOCATION !!!\n";
#endif

	    d_bufferpool[i] = new DecodedImageData;
	    d_bufferpool[i]->m_image.Create(spec);
	  }

	d_flags[i]=FLAG_InUse;
	return d_bufferpool[i];
      }

  cout << "CAPACITY EXCEEDED\n";

  Assert(0); // Capacity exceeded.
#endif
}
#endif

void VideoImageBuffer::ShowMBRows(DecodedImageData* d)
{
  /*
  cout << "ShowOutputBuffer at ";
  if (d->m_timing.HasPTS) cout << d->m_timing.pts << endl;
  else cout << "undefined\n";
  */

  for (int i=0;i<d_maxpoolsize;i++)
    if (d_bufferpool[i]==d)
      {
	d_flags[i] |= FLAG_InQueue;
      }

  Assert(d_queuelength<d_maxqueuelength);
  d_queue[d_queuelength]=d;
  d_queuelength++;
}

#if 0
void VideoImageBuffer::FreeOutputBuffer(DecodedImageData* d)
{
  Assert(0);
#if 0
  assert(d->m_picdata1==NULL);

  for (int i=0;i<d_maxpoolsize;i++)
    if (d_bufferpool[i]==d)
      {
	d_flags[i] &= ~FLAG_InUse;
      }
#endif
}
#endif
