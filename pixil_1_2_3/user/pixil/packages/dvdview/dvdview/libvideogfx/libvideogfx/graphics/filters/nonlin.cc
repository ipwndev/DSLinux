
#include "libvideogfx/graphics/filters/nonlin.hh"

struct SortData
{
  int val;
  int u,v;
};

template <class Pel> void Median_YUV_3x3(const Image_YUV<Pel>& img,Image_YUV<Pel>& dest)
{
  // Get image parameters and assure that they are in the right format.
  ImageParam_YUV param;
  img.GetParam(param);

  ImageParam_YUV param2;
  dest.GetParam(param2);

  const bool color = !(param.nocolor);

  if (color)
    {
      assert(param.chroma ==Chroma444);
      assert(param2.chroma==Chroma444);
    }
  assert(&img != &dest);  // Median needs two image buffers for correct operation.

  // Give hint as the destination image will be completely overwritten.
  dest.Hint_ContentsIsNotUsedAnymore();

  Pel*const* yp2  = dest.AskFrameY();
  Pel*const* up2  = (color ? dest.AskFrameU() : 0);
  Pel*const* vp2  = (color ? dest.AskFrameV() : 0);

  const Pel*const* yp  = img.AskFrameY_const();
  const Pel*const* up  = (color ? img.AskFrameU_const() : 0);
  const Pel*const* vp  = (color ? img.AskFrameV_const() : 0);

  SortData a[9];


  /* Do median filtering.
     We filter all of the image except a one pixel wide border because the
     filter size is 3x3. This border will simply be copied from the original
     image.
  */
     
  int w = param.width;
  int h = param.height;

  for (int y=1;y<param.height-1;y++)
    for (int x=1;x<param.width-1;x++)
      {
	// Fill array with filter kernel for sorting.

	a[0].val = yp[y-1][x-1];
	a[1].val = yp[y-1][x  ];
	a[2].val = yp[y-1][x+1];
	a[3].val = yp[y  ][x-1];
	a[4].val = yp[y  ][x  ];
	a[5].val = yp[y  ][x+1];
	a[6].val = yp[y+1][x-1];
	a[7].val = yp[y+1][x  ];
	a[8].val = yp[y+1][x+1];

	if (color)
	  {
	    a[0].u   = up[y-1][x-1];
	    a[0].v   = vp[y-1][x-1];
	    a[1].u   = up[y-1][x  ];
	    a[1].v   = vp[y-1][x  ];
	    a[2].u   = up[y-1][x+1];
	    a[2].v   = vp[y-1][x+1];
	    a[3].u   = up[y  ][x-1];
	    a[3].v   = vp[y  ][x-1];
	    a[4].u   = up[y  ][x  ];
	    a[4].v   = vp[y  ][x  ];
	    a[5].u   = up[y  ][x+1];
	    a[5].v   = vp[y  ][x+1];
	    a[6].u   = up[y+1][x-1];
	    a[6].v   = vp[y+1][x-1];
	    a[7].u   = up[y+1][x  ];
	    a[7].v   = vp[y+1][x  ];
	    a[8].u   = up[y+1][x+1];
	    a[8].v   = vp[y+1][x+1];
	  }

#if 0
	// --- Bubble Sort ---

	/* We only need 5 passes over the array because then the middle element of the
	   array keeps stable. */
	for (int i=0;i<5;i++)
	  for (int j=0;j<8-i;j++)
	    {
	      if (a[j].d.val>a[j+1].d.val)
		{
		  int m;
		  m = a[j].dummy; a[j].dummy=a[j+1].dummy; a[j+1].dummy=m;
		}
	    }

	// copy result into image

	yp2[y][x] = a[4].d.val;
	up2[y][x] = a[4].d.u;
	vp2[y][x] = a[4].d.v;
#endif

#if 1
	/* This is an alternative implementation of finding the median.
	   It works by sorting out the minimum and maximum element four times
	   which leaves the median.
	   This version is equivalent to the above bubble sort implementation
	   except ties (which could result in different chrominance information).
	   It is a little bit faster than the bubble sort.
	*/

	int last=8;
	for (int i=0;i<4;i++)
	  {
	    int maxid=0,maxval=a[0].val;
	    int minid=0,minval=a[0].val;

	    // Find minimum and maximum element.

	    for (int j=1;j<=last;j++)
	      {
		if (a[j].val > maxval) { maxval=a[j].val; maxid=j; }
		else if (a[j].val < minval) { minval=a[j].val; minid=j; }
	      }

	    // Compress the array by leaving out the two extreme elements.
	    if (minid==last)
	      {
		a[maxid]=a[last-1];
	      }
	    else
	      {
		a[maxid]=a[last  ];
		a[minid]=a[last-1];
	      }

	    last-=2;
	  }

	// copy result into image

	yp2[y][x] = a[0].val;
	if (color)
	  {
	    up2[y][x] = a[0].u;
	    vp2[y][x] = a[0].v;
	  }
#endif
      }

  // Copy border from old image to filtered one.

  for (int x=0;x<param.width;x++)
    {
      yp2[  0][x]=yp[  0][x]; if (color) { up2[  0][x]=up[  0][x]; vp2[  0][x]=vp[  0][x]; }
      yp2[h-1][x]=yp[h-1][x]; if (color) { up2[h-1][x]=up[h-1][x]; vp2[h-1][x]=vp[h-1][x]; }
    }

  for (int y=0;y<param.height;y++)
    {
      yp2[y][  0]=yp[y][  0]; if (color) { up2[y][0  ]=up[y][  0]; vp2[y][  0]=vp[y][  0]; }
      yp2[y][w-1]=yp[y][w-1]; if (color) { up2[y][w-1]=up[y][w-1]; vp2[y][w-1]=vp[y][w-1]; }
    }
}

