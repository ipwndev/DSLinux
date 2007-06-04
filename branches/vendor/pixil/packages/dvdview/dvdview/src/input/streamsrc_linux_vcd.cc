/********************************************************************************
    Copyright (C) 1999  Dirk Farin

    This program is distributed under GNU Public License (GPL) as
    outlined in the COPYING file that comes with the source distribution.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************************************/

#include "input/streamsrc_linux_vcd.hh"
#include "error.hh"

#include <math.h>
#include <iostream.h>
#include <string.h>


StreamSource_VCD::StreamSource_VCD()
  : d_fd(0),
    d_buflen(0)
{
}


StreamSource_VCD::~StreamSource_VCD()
{
  close(d_fd);
}


void StreamSource_VCD::Init(const char* device)
{
  d_fd = open(device,O_RDONLY|O_NONBLOCK);
  if (d_fd==-1)
    { perror("CDROM open: "); throw "error opening cdrom"; }

  struct cdrom_tochdr tochdr;
  if (ioctl(d_fd,CDROMREADTOCHDR,&tochdr)==-1)
    { perror("read CDROM toc header: "); throw "error reading CDROM toc header"; }

  d_nTracks = tochdr.cdth_trk1-1;

#if 0
  cout << "First track: " << ((int)tochdr.cdth_trk0) << endl;
  cout << "Last  track: " << ((int)tochdr.cdth_trk1) << endl;


  for (int i=tochdr.cdth_trk0 ; i<=tochdr.cdth_trk1 ; i++)
    {
      struct cdrom_tocentry tocentry;

      tocentry.cdte_track  = i;
      tocentry.cdte_format = CDROM_MSF;

      if (ioctl(d_fd,CDROMREADTOCENTRY,&tocentry)==-1)
	{ perror("read CDROM toc entry: "); throw "error reading CDROM toc entry"; }

      cout << "track:  " << ((int)tocentry.cdte_track) << endl
	   << "adr:    " << ((int)tocentry.cdte_adr) << endl
	   << "ctrl:   " << ((int)tocentry.cdte_ctrl) << endl
	   << "format: " << ((int)tocentry.cdte_format) << endl
	   << " min:   " << ((int)tocentry.cdte_addr.msf.minute) << endl
	   << " sec:   " << ((int)tocentry.cdte_addr.msf.second) << endl
	   << " frame: " << ((int)tocentry.cdte_addr.msf.frame) << endl
	   << "datamode: " << ((int)tocentry.cdte_datamode) << endl;

      cout << endl;
    }
#endif

  SkipToTrack(2);
}

int  StreamSource_VCD::AskNTracks() const
{
  return d_nTracks;
}


int  StreamSource_VCD::AskCurrentTrack() const
{
  NotImplemented;
}


void StreamSource_VCD::SkipToTrack(int track)
{
  d_entry.cdte_format = CDROM_MSF;
  d_entry.cdte_track  = track+1;
  if (ioctl(d_fd, CDROMREADTOCENTRY, &d_entry)) {
    perror("ioctl dif1");
    return;
  }       
}


uint32 StreamSource_VCD::FillBuffer(uint8* mem,uint32 maxlen)
{
  if (d_buflen!=0)
    {
      int size = min(maxlen,d_buflen);
      memcpy(mem,&d_buf[d_bufidx],size);
      d_buflen -= size;
      d_bufidx += size;
      return size;
    }
  else
    {
      memcpy(d_buf,&d_entry.cdte_addr.msf,sizeof(struct cdrom_msf));
      ioctl(d_fd,CDROMREADRAW,d_buf);

      d_entry.cdte_addr.msf.frame++;
      if (d_entry.cdte_addr.msf.frame==75)
        {                     
          d_entry.cdte_addr.msf.frame=0;
          d_entry.cdte_addr.msf.second++;
        }                     
      if (d_entry.cdte_addr.msf.second==60)
        {                     
          d_entry.cdte_addr.msf.second=0;
          d_entry.cdte_addr.msf.minute++;
        }

      if (maxlen>=2324)
        {
          memcpy(mem,&d_buf[24],2324);
          return 2324;
        }
      else
        {
          memcpy(mem,&d_buf[24],maxlen);
          d_bufidx = 24+maxlen;
          d_buflen = 2324-maxlen;
          return maxlen;
        }
    }
}


bool   StreamSource_VCD::MoreDataPending() const
{
  // TODO
  return true;
}


uint64 StreamSource_VCD::AskStreamLength() const
{
  // TODO
  return 0;
}


uint64 StreamSource_VCD::AskCurrentPosition() const
{
  // TODO
  return 0;
}


uint64 StreamSource_VCD::SetCurrentPosition(uint64 pos)
{
  // TODO
  return pos;
}
