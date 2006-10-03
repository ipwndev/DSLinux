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

#include "error.hh"
#include <iostream.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


static class MessageDisplay_cerr : public MessageDisplay
{
public:
  void ShowMessage(ErrorSeverity severity,const char* text) const
    {
      cout.flush(); /* Flush program output. Do this for better sync of
                       debug and error display. */

      switch(severity)
        {
          //case ErrSev_Empty:     cerr << "EMPTY-ERROR ! This should not occur !\n"; exit(10); break;
        case ErrSev_Note:      cerr << "Note: ";    break;
        case ErrSev_Warning:   cerr << "Warning: "; break;
        case ErrSev_Error:     cerr << "Error: ";   break;
        case ErrSev_Critical:  cerr << "CRITICAL ERROR: "; break;
        case ErrSev_Assertion: cerr << "ASSERTION FAILED: "; break;
        }

      cerr << text << endl;
    }

  void ShowMessage(const Excpt_Base& e) const
    {
      ShowMessage(e.m_severity , e.m_text);
    }
} msgdisplay_cerr;


void MessageDisplay::Show(ErrorSeverity sev,const char* text)
{
  assert(std_msgdisplay);
  std_msgdisplay->ShowMessage(sev,text);
}

void MessageDisplay::Show(const Excpt_Base& excpt)
{
  assert(std_msgdisplay);
  std_msgdisplay->ShowMessage(excpt);
}

void MessageDisplay::SetStandardDisplay(MessageDisplay* disp)
{
  assert(disp);
  std_msgdisplay = disp;
}

const MessageDisplay* MessageDisplay::std_msgdisplay = &msgdisplay_cerr;





Excpt_Base::Excpt_Base(ErrorSeverity sev)
  : m_severity(sev)
{
  assert(m_severity != ErrSev_Note);

  m_text[0] = 0;
}

Excpt_Base::Excpt_Base(ErrorSeverity sev,const char* txt)
  : m_severity(sev)
{
  assert(m_severity != ErrSev_Note);
  assert(strlen(txt) <= c_MaxTextLen);
  strcpy(m_text,txt);
}

Excpt_Assertion::Excpt_Assertion(const char* expr,const char* file, const char* function,int line)
  : Excpt_Base(ErrSev_Assertion)
{
  sprintf(m_text,
          "file '%s', '%s', line %d (%s).\n",
          file,function,line,expr);
}


Excpt_NotImplemented::Excpt_NotImplemented(const char* file, int line)
  : Excpt_Base(ErrSev_Critical)
{
  sprintf(m_text,"NOT-IMPLEMENTED-YET point reached in file '%s', line %d.\n",file,line);
}

#if 0
void ShowNote(ErrorSeverity sev,const char* txt)
{
  Assert(sev == ErrSev_Note ||
         sev == ErrSev_Warning);

  if (sev==ErrSev_Note) cerr << "Note: ";
  else cerr << "Warning: ";
  cerr << txt << endl;
}

#if 0
void Error(ErrorSeverity errsev,const char* txt)
{
  switch(errsev)
    {
    case ErrSev_Note:     cerr << "Note: ";    break;
    case ErrSev_Warning:  cerr << "Warning: "; break;
    case ErrSev_Error:    cerr << "Error: ";   break;
    case ErrSev_Critical: cerr << "CRITICAL ERROR: "; break;
    }

  cerr << txt << endl;
}
#else
void Error(ErrorSeverity sev,const char* txt)
{
  throw Excpt_Base(sev,txt);
}
#endif
#endif
