#ifndef __QDIR_H
#define __QDIR_H

#include "qstring.h"
#include "fltk-qdefs.h"

class QDir
{
 public:
  enum FilterSpec { Dirs = 0x001, Files = 0x002, Drives = 0x004, 
		    NoSymLinks = 0x008, All = 0x007, TypeMask = 0x00F,
		    Readable = 0x010, Writable = 0x020, Executable = 0x040, 
		    RWEMask = 0x070, Modified = 0x080, Hidden = 0x100, 
		    System = 0x200, AccessMask = 0x3F0, DefaultFilter = -1 };
  enum SortSpec { Name, Time, Size, Unssorted, 
		  SortByMask = 0x03, DirsFirst = 0x04, Reversed = 0x08,
		  IgnoreCase = 0x10, DefaultSort = -1 };

  QDir() { }
  QDir(const QString & path, const QString & nameFilter = "",
       int sortSpec = Name | IgnoreCase, int filterSpec = All) { }

  static QString cleanDirPath(const QString & dirPath) { return ""; }
  virtual bool exists() const { return true; }
  virtual void setFilter(int filterSpec) { } 
  virtual void setSorting(int sortSpec) { }
};

#endif
