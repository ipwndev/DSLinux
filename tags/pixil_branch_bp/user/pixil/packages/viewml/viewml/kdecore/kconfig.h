/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
// $Id$
//
// $Log$
// Revision 1.1  2006/10/03 11:26:26  dslinux_amadeus
// adding pristine copy of pixil to HEAD so I can branch from it
//
// Revision 1.1  2003/09/08 19:42:04  jasonk
// Addition of packages directory and associated files.
//
// Revision 1.1.1.1  2003/08/07 21:18:32  jasonk
// Initial import of PIXIL into new cvs repository.
//
// Revision 1.1.1.1  2003/06/23 22:04:23  jasonk
//
//
// Revision 1.1.1.1  2000/07/07 16:10:58  jasonk
// Initial import of ViewML
//
// Revision 1.7  1998/01/18 14:38:48  kulow
// reverted the changes, Jacek commited.
// Only the RCS comments were affected, but to keep them consistent, I
// thought, it's better to revert them.
// I checked twice, that only comments are affected ;)
//
// Revision 1.5  1997/12/12 14:46:01  denis
// Reverting to lower case true/false
//
// Revision 1.4  1997/12/12 14:33:45  denis
// replaced true by TRUE and false by FALSE
//
// Revision 1.3  1997/10/21 20:44:44  kulow
// removed all NULLs and replaced it with 0L or "".
// There are some left in mediatool, but this is not C++
//
// Revision 1.2  1997/10/16 11:14:30  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.1  1997/10/04 19:50:58  kalle
// new KConfig
//

#ifndef _KCONFIG_H
#define _KCONFIG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kconfigbase.h>

/** 
* KDE Configuration entries
*
* This class implements KDE's default-based configuration system.
*
* @author Kalle Dalheimer (kalle@kde.org)
* @version $Id$
* @see KApplication::getConfig KConfigBase KSimpleConfig
* @short KDE Configuration Management  class
*/
class KConfig : public KConfigBase
{
  Q_OBJECT

  // copy-construction and assignment are not allowed
  KConfig( const KConfig& );
  KConfig& operator= ( const KConfig& rConfig );

protected:
  /** Open all appropriate configuration files and pass them on to
	* parseOneConfigFile()
	*/
  virtual void parseConfigFiles();

  /** Write back the configuration data.
	*/
  bool writeConfigFile( QFile& rFile, bool bGlobal = false );

public:
  /** 
	* Construct a KConfig object. 
	*
	* @param pGlobalAppFile A file to parse in addition to the
	*  normally parsed files.  
	* @param pLocalAppFile Another file to parse in addition to the
	* normally parsed files (has priority over pGlobalAppFile 
	*/
  KConfig( const char* pGlobalAppFile = 0L, 
		   const char* pLocalAppFile = 0L );

  /** 
	* Destructor. 
	*
	* Writes back any dirty configuration entries.
	*/
  virtual ~KConfig();

  /**
	* Write back the config cache.
	*/
  virtual void sync();
};
  


#endif
