/* -*-C++-*- 

   "$Id$"
   
   Copyright 1999-2000 by the Flek development team.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.
  
   Please report all bugs and problems to "flek-devel@sourceforge.net".
   
*/

#ifndef _FFILE_H_
#define _FFILE_H_

#include <stdio.h>

typedef unsigned long ulong;
typedef unsigned long * ulongPtr;
typedef unsigned short ushort;
typedef unsigned short * ushortPtr;
typedef unsigned char uchar;
typedef unsigned char * ucharPtr;

enum FFileMode {
  FFileNull = 0,
  FFileRead = 1,
  FFileReadPlus,
  FFileWrite,
  FFileWritePlus,
  FFileAppend,
  FFileAppendPlus
};

/** @package libflek_core
 * fFile provides efficient file io routines in a c++ encapsulation.  
 * On most systems it should perform better than c++ io streams.
 * Ultimately, this class should be similar to Ruby's
 * <a href="http://www.ruby-lang.org/en/man-1.4/File.html">File Class</a>.
 */
class FFile {

 public:

  /**
   * The Constructor.
   *
   * @param filename The name of the file to open.
   * @param mode The mode the file should be opened in:
   * <ul>
   * <li><b>FFileRead</b> - Open  text  file  for reading.  The stream is 
   *     positioned at the beginning of the file.</li>
   * <li><b>FFileReadPlus</b> - Open for reading and writing.  The stream is 
   *     positioned at the beginning of the file.</li>
   * <li><b>FFileWrite</b> - Truncate file to zero length or create text file
   *     for writing.   The stream is positioned at the
   *     beginning of the file.</li>
   * <li><b>FFileWritePlus</b> - Open  for reading and writing.  The file is created
   *     if it does not exist, otherwise  it  is  truncated.
   *     The  stream  is  positioned at the beginning of the file.</li>
   * <li><b>FFileAppend</b> - Open for writing.  The file is created if  it  does
   * not  exist.  The stream is positioned at the end of the file.</li>
   * <li><b>FFileAppendPlus</b> - Open for reading and writing.  The file is  created
   * if  it does not exist.  The stream is positioned at
   * the end of the file.</li>
   *</ul>
   */
  FFile (char *filename=0, FFileMode mode=FFileRead) {
    Fd = 0;
    Error = 0;
    
    if (filename)
      open (filename, mode);
  }

  /**
   * The destructor closes the file if it is still open.
   */
  ~FFile () {
    if (Fd)
      fclose (Fd);
  }

  /**
   * Open a file with the given filename, and a mode.  The mode should be 
   * one of the standard fopen modes (see the constructor for a listing).
   */
  void open (char *filename, FFileMode mode);

  /**
   * Put a single character into the file stream using Hi (MSB) byte order.
   */
  inline void put_hi (char c) {
    fputc (c, Fd);
  }

  /**
   * Put a single unsigned character into the file stream using Hi (MSB) byte order.
   */
  inline void put_hi (uchar c) {
    fputc (c, Fd);
  }

  /**
   * Put a single unsigned short into the file stream using Hi (MSB) byte order.
   */
  inline void put_hi (unsigned short c) {
    unsigned char t[2];
    t[0] = ((c >> 8) & 0xff);
    t[1] = (c & 0xff);
    int size = fwrite (t, 2, 1, Fd);
    if (size != 1) Error = 1;
  }

  /**
   * Put a single unsigned short into the file stream using Hi (MSB) byte order.
   */
  inline void put_hi (short c) {
    put_hi ((unsigned short) c);
  }
  
  /**
   * Put a single unsigned long into the file stream using Hi (MSB) byte order.
   */
  inline void put_hi (unsigned long c) {
    unsigned char t[4];
    t[0] = (unsigned char)((c >> 24) & 0xff);
    t[1] = (unsigned char)((c >> 16) & 0xff);
    t[2] = (unsigned char)((c >> 8) & 0xff);
    t[3] = (unsigned char)(c & 0xff);
    int size = fwrite (t, 1, 4, Fd);
    if (size != 4) Error = 1;
  }

  /**
   * Put a single long into the file stream using Hi (MSB) byte order.
   */
  inline void put_hi (long c) {
    unsigned char t[4];
    t[0] = (unsigned char)((c >> 24) & 0xff);
    t[1] = (unsigned char)((c >> 16) & 0xff);
    t[2] = (unsigned char)((c >> 8) & 0xff);
    t[3] = (unsigned char)(c & 0xff);
    int size = fwrite (t, 1, 4, Fd);
    if (size != 4) Error = 1;
  }

  /**
   * Write an array of characters into the file stream.
   */
  inline void write (char *c, int len) {
    int size = fwrite ((void *)c, 1, len, Fd);
    if (size != len) Error = 1;
  }

  /**
   * Get a single unsigned character from the file stream using Hi (MSB) byte order.
   */
  void get_hi (uchar& r) { 
    fread (&r, 1, 1, Fd);
  } 

  /**
   * Get a single short int from the file stream using Hi (MSB) byte order.
   */
  void get_hi (short& r) { 
    uchar b[2];
    fread (b, 2, 1, Fd);
    r = ((b[0] << 8) | b[1]); 
  } 

  /**
   * Get a single unsigned short int from the file stream using Hi (MSB) byte order.
   */
  void get_hi (ushort& r) { 
    uchar b[2];
    fread (b, 2, 1, Fd);
    r = ((b[0] << 8) | b[1]); 
  } 

  /**
   * Get a single unsigned long int from the file stream using Hi (MSB) byte order.
   */
  void get_hi (ulong& r) { 
    uchar b[4];
    fread (b, 4, 1, Fd);
    r = ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]); 
  } 

  /**
   * Read an array of characters into the file stream.
   */
  inline void read (char *c, int len) {
    int size = fread ((void *)c, 1, len, Fd);
    if (size != len) Error = 1;
  }
  
  /**
   * Read an array of unsigned characters into the file stream.
   */
  inline void read (uchar *c, int len) {
    int size = fread ((void *)c, 1, len, Fd);
    if (size != len) Error = 1;
  }

  /**
   * Set the file postion indicator to pos.  Seek can set pos relative to
   * the beginning (mode=SEEK_SET), the end (mode=SEEK_END), or the current
   * position (mode=SEEK_CUR).
   */
  inline void seek (long pos, int mode = SEEK_SET) {
    Error = fseek (Fd, pos, mode);
  }

  /**
   * Report the current file position.
   */
  long tell () {
    long rval = ftell (Fd);
    if (rval == -1) Error = -1;
    return rval;
  }

  /** 
   * Report if there has been any errors.
   */
  bool bad () {
    if (Error == 0) return false;
    return true;
  }
  
  /**
   * Close the file stream.
   */
  void close () {
    fclose (Fd);
    Fd = 0;
  }
  
  /**
   * Return the C file handle.
   */
  FILE *fd () {
    return Fd;
  }

  char* expand (char* name);
  
protected:

  FILE *Fd;
  char *filename;
  int Error;

};

/**
 * Convert an array of MSB longs to an array of LSB longs.
 */
void lo_to_hi (ulong *buffer, int len);

#endif
