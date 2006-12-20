/*                                                                       
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
 *                                                                       
 * This file is part of the PIXIL Operating Environment                 
 *                                                                       
 * The use, copying and distribution of this file is governed by one    
 * of two licenses, the PIXIL Commercial License, or the GNU General    
 * Public License, version 2.                                           
 *                                                                       
 * Licensees holding a valid PIXIL Commercial License may use this file 
 * in accordance with the PIXIL Commercial License Agreement provided   
 * with the Software. Others are governed under the terms of the GNU   
 * General Public License version 2.                                    
 *                                                                       
 * This file may be distributed and/or modified under the terms of the  
 * GNU General Public License version 2 as published by the Free        
 * Software Foundation and appearing in the file LICENSE.GPL included   
 * in the packaging of this file.                                      
 *                                                                       
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
 * PARTICULAR PURPOSE.                                                  
 *                                                                       
 * RESTRICTED RIGHTS LEGEND                                             
 *                                                                     
 * Use, duplication, or disclosure by the government is subject to      
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
 * Technical Data and Computer Software clause in DAR 7-104.9(a).       
 *                                                                      
 * See http://www.pixil.org/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://www.pixil.org/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#include "editvars.h"
#include "wstring.h"
#include "editengine.h"

rclError EditorEngine::AppendClip()
{
    long
	row,
	col;

    if (_ReadOnly)
	LastResult = LIMITS_FULL_ERROR;
    else {
	row = Cursor.Row;
	col = Cursor.Column;
	Command(MOVE_BOF, 1);
	Command(PASTE_REGION, 1);
	HasChanged = TRUE;
	MoveTo(row, col);
    }
    return LastResult;
}

rclError EditorEngine::BackSpaceCharacter()
{
    if (_ReadOnly)
	return LIMITS_FULL_ERROR;
    last_wrapped = false;
    LastResult = NOERROR;
    if (LineCount && (Cursor.Row || Cursor.Column))	// if empty or at TOF, do nothing
    {
	HasChanged = TRUE;
	if ((Cursor.Row > 0) && (Lines[Cursor.Row]->Length() == 0)) {	// if just emptied this line, toss it.
	    bool
		moveup =
		TRUE;
	    if (Cursor.Row == LineCount - 1)
		moveup = FALSE;
	    RemoveLine();
	    if (moveup)
		Cursor.Row--;
	    MoveCursor(MOVE_EOL, 1);
	    if (Lines[Cursor.Row]->LastCharacter() == HARD_RETURN);	// if backspaced "under" a hard return, stay put and delete it
	    else {		// otherwise, move left one character before deleting
		MoveCursor(MOVE_LEFT, 1);
	    }
	} else {
	    MoveCursor(MOVE_LEFT, 1);
	}
	if (LastResult == NOERROR)
	    DeleteCharacter();
	// don't need to decrement character count, since DeleteCharacter() does that
    } else
	LastResult = RANGE_ERROR;
    return LastResult;
}

rclError EditorEngine::DeleteCharacter()
{
    if (_ReadOnly)
	return LIMITS_FULL_ERROR;
    last_wrapped = false;
    long
	oldcol =
	Cursor.
	Column;
    long
	oldrow =
	Cursor.
	Row;
    LastResult = NOERROR;
    if (CharCount) {
	HasChanged = TRUE;
	if (Cursor.Row < LineCount - 1)
	    if ((Lines[Cursor.Row]->Get()[Cursor.Column] == HARD_RETURN) || (Cursor.Column == Lines[Cursor.Row]->Length())) {	// delete pressed at EOL, join lines, delete character
		*Lines[Cursor.Row] += *Lines[Cursor.Row + 1];
		if (MoveTo(Cursor.Row + 1, 0) == NOERROR)
		    RemoveLine();
		MoveTo(oldrow, oldcol);
	    }
	Lines[Cursor.Row]->DeleteAt((int) Cursor.Column);
	CharCount--;
	CheckLineFit();
	DeleteChar = true;
	LastResult = ReformatParagraph();
	DeleteChar = false;
    } else
	LastResult = RANGE_ERROR;
    if (!CharCount || !LineCount)
	Clear();
    return LastResult;
}

rclError EditorEngine::Clear()
{
    while (LineCount)
	delete
	    Lines[--LineCount];
    if (Lines)
	free(Lines);
    Lines = NULL;
    Cursor.Row = Cursor.Column = 0;
    BlockStart.Row = BlockStart.Column = 0;
    BlockEnd.Row = BlockEnd.Column = 0;
    CharCount = LineCount = 0;
    HasChanged = TRUE;
    return (LastResult = NOERROR);
}

rclError EditorEngine::Command(EditorCommandTypes command, long argument)
{
    LastResult = NOERROR;
    switch (command) {
    case INSERT_CHARACTER:
	last_wrapped = false;
	LastResult = InsertCharacter((char) argument);
	break;
    case DELETE_CHARACTER:
	last_wrapped = false;
	LastResult = DeleteCharacter();
	break;
    case BACKSPACE_CHARACTER:
	last_wrapped = false;
	LastResult = BackSpaceCharacter();
	break;
    case REPLACE_CHARACTER:
	LastResult = ReplaceCharacter((char) argument);
	break;
    case MOVE_LEFT:
    case MOVE_RIGHT:
    case MOVE_DOWN:
    case MOVE_UP:
    case MOVE_SOL:
    case MOVE_EOL:
    case MOVE_TOF:
    case MOVE_BOF:
    case MOVE_LEFT_WORD:
    case MOVE_RIGHT_WORD:
    case MOVE_ABSOLUTE_ROW:
    case MOVE_ABSOLUTE_COLUMN:
	LastResult = MoveCursor(command, argument);
	break;
    case CLEAR_ALL:
	last_wrapped = false;
	Clear();
	break;
    case MARK_BEGIN:		// begin marking block, set start and end to current position
	BlockStart.Row = BlockEnd.Row = Cursor.Row;
	BlockStart.Column = BlockEnd.Column = Cursor.Column;
	break;
    case MARK_END:
	BlockEnd.Row = Cursor.Row;
	BlockEnd.Column = Cursor.Column;
	break;
    case CLEAR_MARKS:
	BlockStart.Row = 0;
	BlockStart.Column = 0;
	BlockEnd.Row = 0;
	BlockEnd.Column = 0;
	break;
    case DELETE_REGION:
	last_wrapped = false;
	DeleteRegion();
	break;
    case CUT_REGION:
	last_wrapped = false;
	CopyRegion(TRUE);
	break;
    case COPY_REGION:
	CopyRegion(FALSE);
	break;
    case PASTE_REGION:
	last_wrapped = false;
	PasteRegion(FALSE);
	break;
    case REPLACE_REGION:
	last_wrapped = false;
	PasteRegion(TRUE);
	break;
    case SET_IGNORE_CASE:
	if (argument)
	    Flags |= EF_IGNORECASE;
	else
	    Flags &= (~EF_IGNORECASE);
	break;
    case SET_SEARCHWHOLE:
	if (argument)
	    Flags |= EF_SEARCHWHOLE;
	else
	    Flags &= (~EF_SEARCHWHOLE);
	break;
    case INDENT:
	if (TabStops && TabStops[CurrentIndent + 1])
	    CurrentIndent++;
	else
	    LastResult = RANGE_ERROR;
	HasChanged = TRUE;
	break;
    case OUTDENT:
	if (CurrentIndent > 0)
	    CurrentIndent--;
	else
	    LastResult = RANGE_ERROR;
	HasChanged = TRUE;
	break;
    case PARAGRAPH:
	HasChanged = TRUE;
	break;
    case REFORMAT:
	LastResult = ReformatDocument((int) argument);
	break;
    case APPEND_CLIPBOARD:
	last_wrapped = false;
	LastResult = AppendClip();
	break;
    case SET_REPLACEMENT_TEXT:
    case FIND:
    case FIND_REPLACE:
    case INSERT_STRING:
    case INSERT_STRING_MOVE:
	break;
    }
    return LastResult;
}

rclError
    EditorEngine::Command(EditorCommandTypes command, const char *argument)
{
    LastResult = NOERROR;
    switch (command) {
    case SET_REPLACEMENT_TEXT:
	ReplacementText = argument;
	break;
    case FIND:
	FindPhrase(argument, FALSE);
	break;
    case FIND_REPLACE:
	last_wrapped = false;
	FindPhrase(argument, TRUE);
	break;
    case INSERT_STRING:
    case INSERT_STRING_MOVE:
	last_wrapped = false;
	if (!argument[0] || _ReadOnly)
	    break;		// read only or string is empty?  Nothing to do...
	char *temp;
	temp = strchr(argument, HARD_RETURN);
	if (temp)		// newlines in inserted text!
	{
	    while (temp && *temp) {
		char hr[2];
		hr[0] = HARD_RETURN;
		hr[1] = '\0';
		*temp = '\0';
		wString ts(argument);
		ts += hr;
		if (LineCount) {
		    Lines[Cursor.Row]->InsertAt((int) Cursor.Column,
						ts.Get());
		    CharCount += ts.Length();
		    ReformatParagraph();
		} else
		    InsertLine(wString(ts.Get()));
		if (command == INSERT_STRING_MOVE) {
		    MoveCursor(MOVE_RIGHT, ts.Length());
		}
		temp = strchr(temp + 1, HARD_RETURN);	// any more?
	    }
	} else {
	    if (LineCount) {
		Lines[Cursor.Row]->InsertAt((int) Cursor.Column, argument);
		CharCount += strlen(argument);
		ReformatParagraph();
	    } else
		InsertLine(wString(argument));
	    if (command == INSERT_STRING_MOVE) {
		MoveCursor(MOVE_RIGHT, strlen(argument));
	    }
	}
	break;
    case INSERT_CHARACTER:
    case DELETE_CHARACTER:
    case BACKSPACE_CHARACTER:
    case MOVE_LEFT:
    case MOVE_RIGHT:
    case MOVE_UP:
    case MOVE_DOWN:
    case MOVE_SOL:
    case MOVE_EOL:
    case MOVE_TOF:
    case MOVE_BOF:
    case MOVE_LEFT_WORD:
    case MOVE_RIGHT_WORD:
    case MOVE_ABSOLUTE_ROW:
    case MOVE_ABSOLUTE_COLUMN:
    case CLEAR_ALL:
    case MARK_BEGIN:
    case CLEAR_MARKS:
    case MARK_END:
    case DELETE_REGION:
    case CUT_REGION:
    case COPY_REGION:
    case PASTE_REGION:
    case SET_IGNORE_CASE:
    case INDENT:
    case OUTDENT:
    case PARAGRAPH:
    case REFORMAT:
    case APPEND_CLIPBOARD:
    case REPLACE_CHARACTER:
    case SET_SEARCHWHOLE:
    case REPLACE_REGION:
	break;
    }
    return LastResult;
}


void
EditorEngine::ConstrainCursor()
{
  if (!Lines) return;  /* This fixes a bug */

  if (LineCount && Cursor.Row > LineCount - 1)
    Cursor.Row = LineCount - 1;
  if (!(Flags & EF_FREEFORM))
    if (Cursor.Column >= (Lines[Cursor.Row]->Length())) {
      Cursor.Column = Lines[Cursor.Row]->Length();
      if (Lines[Cursor.Row]->LastCharacter() == HARD_RETURN)
	Cursor.Column = Lines[Cursor.Row]->Length() - 1;
      if (Cursor.Column < 0)
	Cursor.Column = 0;
    }
}


rclError EditorEngine::MoveCursor(EditorCommandTypes command, long argument)
{
    short
	count;
    LastResult = NOERROR;
    switch (command) {
    case MOVE_LEFT:
	if (argument == 0)
	    argument = 1;
	for (count = 0; count < argument; count++) {
	    if (Cursor.Column)
		Cursor.Column--;
	    else if (Cursor.Row) {
		Cursor.Row--;
		Cursor.Column =
		    Lines[Cursor.Row]->Length()? Lines[Cursor.Row]->
		    Length() : 0;
	    } else
		LastResult = RANGE_ERROR;
	}
	break;
    case MOVE_RIGHT:
	if (argument == 0)
	    argument = 1;
	for (count = 0; count < argument; count++) {
	    if (!(CharCount && LineCount))
		LastResult = RANGE_ERROR;
	    else if (Special) {
		Cursor.Row++;
		Cursor.Column = 0;
	    } else
		if ((Lines[Cursor.Row]->Get()[Cursor.Column] == HARD_RETURN)
		    && (Cursor.Row < (LineCount - 1))) {
		Cursor.Row++;
		Cursor.Column = 0;
	    } else if (Cursor.Column < Lines[Cursor.Row]->Length()) {
		Cursor.Column++;
	    } else if (Cursor.Row < (LineCount - 1)) {
		Cursor.Row++;
		Cursor.Column = 0;
	    } else if ((Cursor.Row == (LineCount - 1))
		       && (Cursor.Column < Lines[Cursor.Row]->Length())) {
		Cursor.Column++;	// insert at very end of document OK.
	    } else
		LastResult = RANGE_ERROR;
	}
	break;
    case MOVE_UP:
	if (argument == 0)
	    argument = 1;
	for (count = 0; count < argument; count++) {
	    if (Cursor.Row) {
		Cursor.Row--;
		ConstrainCursor();
	    } else
		LastResult = RANGE_ERROR;
	}
	break;
    case MOVE_DOWN:
	if (argument == 0)
	    argument = 1;
	for (count = 0; count < argument; count++) {
	    if (Cursor.Row < (LineCount - 1)) {
		Cursor.Row++;
		ConstrainCursor();
	    } else
		LastResult = RANGE_ERROR;
	}
	break;
    case MOVE_SOL:
	Cursor.Column = 0;
	break;
    case MOVE_EOL:
	if (LineCount)		// any lines at all, even first one?
	{
	    Cursor.Column = Lines[Cursor.Row]->Length();
	    if (Lines[Cursor.Row]->LastCharacter() == HARD_RETURN)
		Cursor.Column--;
	}
	break;
    case MOVE_TOF:
	Cursor.Row = Cursor.Column = 0;
	break;
    case MOVE_BOF:
	if (LineCount)		// any lines at all, even first one?
	{
	    Cursor.Row = LineCount - 1;
	    Cursor.Column = Lines[Cursor.Row]->Length();
	    if (Cursor.Column
		&& Lines[Cursor.Row]->LastCharacter() == HARD_RETURN)
		Cursor.Column--;
	}
	break;
    case MOVE_RIGHT_WORD:
	if (LineCount)		// any lines at all, even first one?
	{
	    long
		count,
		row;
	    bool
		foundspace =
		FALSE;
	    bool
		done =
		FALSE;
	    count = Cursor.Column;	// first line check from cursor
	    for (row = Cursor.Row; (row < LineCount) && !done; row++) {
		if (count > Lines[row]->Length())
		    count = Lines[row]->Length();
		for (; count < Lines[row]->Length(); count++)
		    if (foundspace && !isspace(Lines[row]->Get()[count])) {
			Cursor.Column = count;
			Cursor.Row = row;
			done = TRUE;
			break;
		    } else if (isspace(Lines[row]->Get()[count]))
			foundspace = TRUE;
		count = 0;	// after first line, check from left edge
	    }
	}
	break;
    case MOVE_LEFT_WORD:
	if (Cursor.Row || Cursor.Column)	// if not already at TOF
	{
	    MoveCursor(MOVE_LEFT, 1);	// look at previous character
	    if (isspace(Lines[Cursor.Row]->Get()[Cursor.Column]) || (Lines[Cursor.Row]->Get()[Cursor.Column] == '\0')) {	// if it's a space, skip back to next word
		while (isspace(Lines[Cursor.Row]->Get()[Cursor.Column])
		       || (Lines[Cursor.Row]->Get()[Cursor.Column] == '\0'))
		    if (MoveCursor(MOVE_LEFT, 1) != NOERROR) {
			break;	// Can't move past top of document or if error...
		    }
	    }
	    // then skip to start of the word
	    while (!isspace(Lines[Cursor.Row]->Get()[Cursor.Column]))
		if (MoveCursor(MOVE_LEFT, 1) != NOERROR) {
		    break;
		}
	    if (isspace(Lines[Cursor.Row]->Get()[Cursor.Column])
		|| (Lines[Cursor.Row]->Get()[Cursor.Column] == '\0')) {
		MoveCursor(MOVE_RIGHT_WORD, 1);
	    }
	    LastResult = NOERROR;
	}
	break;
    case MOVE_ABSOLUTE_ROW:
	if (argument < LineCount) {
	    Cursor.Row = (argument >= 0) ? argument : 0;
	    ConstrainCursor();
	} else {
	    LastResult = RANGE_ERROR;
	    Cursor.Row = (LineCount) ? (LineCount - 1) : 0;
	}
	break;
    case MOVE_ABSOLUTE_COLUMN:
	if ((CharCount) && (argument <= Lines[Cursor.Row]->Length())) {
	    Cursor.Column = argument;
	    ConstrainCursor();
	} else
	    LastResult = RANGE_ERROR;
	break;
    case INSERT_CHARACTER:
    case DELETE_CHARACTER:
    case BACKSPACE_CHARACTER:
    case CLEAR_ALL:
    case MARK_BEGIN:
    case CLEAR_MARKS:
    case MARK_END:
    case DELETE_REGION:
    case CUT_REGION:
    case COPY_REGION:
    case PASTE_REGION:
    case SET_IGNORE_CASE:
    case SET_REPLACEMENT_TEXT:
    case FIND:
    case INDENT:
    case OUTDENT:
    case PARAGRAPH:
    case REFORMAT:
    case APPEND_CLIPBOARD:
    case INSERT_STRING:
    case REPLACE_REGION:
    case FIND_REPLACE:
    case REPLACE_CHARACTER:
    case INSERT_STRING_MOVE:
    case SET_SEARCHWHOLE:
	break;
    }
    return (LastResult);
}


rclError EditorEngine::DeleteToEOL()
{
    if (LineCount && !_ReadOnly) {
	if (Lines[Cursor.Row]->LastCharacter() == HARD_RETURN)
	    Lines[Cursor.Row]->DeleteAt((int) Cursor.Column,
					(int) (Lines[Cursor.Row]->Length() -
					       Cursor.Column - 1));
	else
	    Lines[Cursor.Row]->ChopAt((int) Cursor.Column);
	HasChanged = TRUE;
	LastResult = ReformatParagraph();
    }
    return LastResult;
}

//
//      This file implements the file I/O routines for the editor engine.
//      These are simple ASCII I/O routines, just for getting text out of
//      a file, or putting it into one.
//


rclError EditorEngine::LoadFromFile(const char *fname)
{
    rclError
	ret =
	NOERROR;
    FILE *
	afile =
	fopen(fname, "r");
    if (afile) {
	ret = LoadFrom(afile);
	fclose(afile);
    }
    return (ret);
}

rclError EditorEngine::SaveToFile(const char *fname)
{
    rclError
	ret =
	NOERROR;
    FILE *
	afile =
	fopen(fname, "w");
    if (afile) {
	ret = SaveTo(afile);
	fclose(afile);
    }
    return (ret);
}

rclError EditorEngine::LoadFrom(FILE * infile)
{
    LastResult = NOERROR;
    bool
	reallychanged =
	FALSE;			// if empty, don't mark as changed
    bool
	aslast =
	FALSE;
    long
	row =
	Cursor.
	Row;
    long
	col =
	Cursor.
	Column;
    wString
	left;			// left and right split of current line
    wString
	right;			// (if any) for inserting into
    wString
	s;
    if (CharCount) {
	reallychanged = TRUE;
	left = *Lines[Cursor.Row];
    } else
	aslast = TRUE;
    if ((Cursor.Row >= LineCount - 1) || (!CharCount))	// inserting at EOF
	aslast = TRUE;
    left.ChopAt((int) Cursor.Column, &right);	// split current line, insert text
    InsertLine(right, Cursor.Row + 1);	// between two pieces...
    while (!feof(infile)) {
	short
	    ThisCount =
	    0;
	s = "";
//                      if(row == Cursor.Row)                   // first row?  Add left side of split
//                              s = left;
	while (!feof(infile)) {
	    int
		sc =
		fgetc(infile);
	    char
		c = (char)
		sc;
	    if ((sc == EOF) && feof(infile))
		break;
#ifdef WIN32
	    // Windows platform has CRLF pairs, so ignore the soft returns
	    else if (c == SOFT_RETURN)
		c = ' ';
#else
	    // Linux uses the soft returns as hard returns
	    else if (c == SOFT_RETURN)
		c = HARD_RETURN;
#endif
	    else if (c == '\t')	// TAB
	    {
		c = ' ';
		s += "   ";	// add 3 spaces, get fourth one below
		ThisCount += 3;
	    }
	    ThisCount++;
	    s += c;
	    if (c == HARD_RETURN || (s.Length() > Width && c == ' ')) {
		break;		// end of line, so add it.
	    }
	}
//                      if((row == Cursor.Row) && CharCount) // if first row, copy back to same line
//                              {
//                                      *Lines[Cursor.Row] = s;
//                                      ++row;                                          // bump to next row...
//                              }
//                      else
	InsertLine(s, ++row, aslast);	// otherwise insert new line
	if (LastResult == NOMEMORY || LastResult == LIMITS_FULL_ERROR) {
	    //ShowError(NOMEMORY);
	    break;
	}
	if (LastResult != NOERROR)
	    break;
    }
    ReformatParagraph(TRUE, TRUE);
    MoveTo(row, col);
    HasChanged = reallychanged;	// if was empty, don't mark as changed
    return LastResult;
}

rclError EditorEngine::SaveTo(FILE * outfile)
{
    long
	row;
    char
	softbuffer[10];
    LastResult = NOERROR;
    if (CharCount)		// if nothing in it, don't write anything...
    {
#ifdef WIN32
	sprintf(softbuffer, "%c", CARRIAGE_RETURN);
#else
	sprintf(softbuffer, "%c", LINE_FEED);
#endif
	wString
	softreturn(softbuffer);
	wString
	    s;
	LastResult = NOERROR;
	for (row = 0; row < LineCount; row++) {
	    s = *Lines[row];
	    if ((s.LastCharacter() != HARD_RETURN)
		&& (s.LastCharacter() != SOFT_RETURN)) {
		//if(s.LastCharacter() == ' ')          // if last character is a space,
		//      s.DeleteAt(s.Length()-1,1);     // replace with soft return, else 
		//s += softreturn;                                      // just add it.
	    }
	    fputs(s.Get(), outfile);
	}
	softbuffer[0] = softbuffer[1] = '\0';
	fwrite(softbuffer, 1, 2, outfile);
    }
    return (LastResult);
}


rclError EditorEngine::LoadFrom(const char *buffer)
{
    LastResult = NOERROR;
    bool
	reallychanged =
	FALSE;			// if empty, don't mark as changed
    bool
	aslast =
	FALSE;
    bool
	endofbuffer =
	FALSE;
    long
	row =
	Cursor.
	Row;
    long
	buffpos =
	0;
    wString
	left;			// left and right split of current line
    wString
	right;			// (if any) for inserting into
    wString
	s;
    if (CharCount) {
	reallychanged = TRUE;
	left = *Lines[Cursor.Row];
    } else
	aslast = TRUE;
    if ((Cursor.Row >= LineCount - 1) || (!CharCount))	// inserting at EOF
	aslast = TRUE;
    left.ChopAt((int) Cursor.Column, &right);	// split current line, insert text
    while (!endofbuffer) {
	short
	    ThisCount =
	    0;
	s = "";
	if (row == Cursor.Row)	// first row?  Add left side of split
	    s = left;
	while (!endofbuffer) {
	    char
		c =
		buffer[buffpos++];
	    if (!c) {
		endofbuffer = TRUE;
		break;
	    }
	    if (c == SOFT_RETURN)
		c = ' ';
	    s += c;
	    ThisCount++;
	    if (c == HARD_RETURN) {
		break;		// end of line, so add it.
	    }
	}
	if ((row == Cursor.Row) && CharCount)	// if first row, copy back to same line
	{
	    *Lines[Cursor.Row] = s;
	    row++;
	} else
	    InsertLine(s, ++row, aslast);	// otherwise insert new line
	if (endofbuffer && right.Length())	// if last line, include other side of
	    InsertLine(right, ++row, aslast);	// original line split
    }
    ReformatParagraph(FALSE, TRUE);
    HasChanged = reallychanged;
    return LastResult;
}

rclError EditorEngine::SaveTo(char *buffer)
{
    long
	row;
    char
	softbuffer[10];
    LastResult = NOERROR;

    if (CharCount && buffer)	// if nothing in it, don't write anything...
    {
	buffer[0] = '\0';
	sprintf(softbuffer, "%c", SOFT_RETURN);
	wString
	softreturn(softbuffer);
	wString
	    s;
	LastResult = NOERROR;
	for (row = 0; row < LineCount; row++) {
	    s = *Lines[row];
	    if ((s.LastCharacter() != HARD_RETURN)
		&& (s.LastCharacter() != SOFT_RETURN)) {
		//if(s.LastCharacter() == ' ')          // if last character is a space,
		//      s.DeleteAt(s.Length()-1,1);     // replace with soft return, else
		//s += softreturn;                                      // just add it.
	    }
	    strcat(buffer, s.Get());
	    buffer += s.Length();	// so strcat is quicker
	}
    }
    return (LastResult);
}

rclError
    EditorEngine::FindPhrase(const char *searchword, bool replace,
			     bool replaceall)
{
    long oldrow = 0, oldcol = 0;
    long row = 0;
    char *temp = NULL;
    bool found;
    LastResult = NOERROR;
    wString tryline;
    wString sword(searchword);

    LastFindStatus = FALSE;
    if (Flags & EF_IGNORECASE)
	sword.ToLower();
    const char *word = sword.Get();
    if (replace && replaceall) {
	oldrow = Cursor.Row;
	oldcol = Cursor.Column;
    }
    for (row = Cursor.Row; row < LineCount; row++) {
	found = FALSE;
	if (Flags & EF_IGNORECASE) {
	    // if ignoring case, copy the line, make it and the search string lowercase, and search
	    // then move the pointer over onto the real line for the rest of the routine to use.

	    if (row == Cursor.Row)
		tryline =
		    Lines[row]->Copy(Cursor.Column, Lines[row]->Length());
	    else
		tryline = *Lines[row];
	    tryline.ToLower();
	    temp = strstr((char *) tryline.Get(), word);
	    if (Flags & EF_SEARCHWHOLE)
		while (temp) {
		    // if not a whole word, skip and keep looking in rest of line.
		    wString wholeword =
			tryline.GetWordAt(temp - tryline.Get());
		    if (wholeword == word)
			break;
		    else
			temp = strstr(temp + 1, word);
		}
	    if (temp) {
		found = TRUE;
		// now adjust back to actual line buffer for remainder of routine.
		temp = (char *) (Lines[row]->Get() + (temp - tryline.Get()));
		if (row == Cursor.Row)
		    temp += Cursor.Column;
	    }
	} else {
	    // save time by searching directly in lines if not ignoring case
	    if (row == Cursor.Row) {
		if (Cursor.Column < Lines[row]->Length() - 1) {
		    temp =
			strstr(((char *) Lines[row]->Get()) + Cursor.Column,
			       word);
		    if (temp)
			found = TRUE;
		}
	    } else {
		temp = strstr(((char *) Lines[row]->Get()), word);
		if (temp)
		    found = TRUE;
	    }
	}
	LastFindStatus = found;
	if (LastFindStatus) {
	    Cursor.Row = row;
	    Cursor.Column = temp - (char *) Lines[row]->Get();
	    short count;
	    if (replace) {
		HasChanged = TRUE;
		wString s(*Lines[row]);
		wString t("");
		t = s.GetWordAt((int) Cursor.Column);
		s.DeleteAt((int) Cursor.Column, t.Length());
		s.InsertAt((int) Cursor.Column, ReplacementText.Get());
	    }
	    for (count = 0; count < (int) strlen(word); count++) {
		MoveCursor(MOVE_RIGHT, 1);
	    }
	    if (!replaceall)
		break;
	}
    }
    if (replace && replaceall) {
	Cursor.Row = oldrow;
	Cursor.Column = oldcol;
    }
    return LastResult;
}


rclError EditorEngine::ReformatParagraph(bool fromfirst, bool all)
{
    long
	row;
    LastResult = NOERROR;
    wString
	s;

    last_wrapped = false;
    if (fromfirst)
	row = 0;
    else
	row = Cursor.Row;
    long
	realcount =
	CharCount;		// see below
    if (LineCount)
	for (; row < LineCount; row++) {
	    // following is impossible except when loading from someplace else.
	    const char *
		hrptr =
		strchr(Lines[row]->Get(), HARD_RETURN);
	    if (hrptr
		&& (hrptr < (Lines[row]->Get() + Lines[row]->Length() - 1))) {
		long
		    pos =
		    (strchr(Lines[row]->Get(), HARD_RETURN) -
		     Lines[row]->Get()) +
		    1;
		// CR in middle of line, so split it.
		Lines[row]->ChopAt(pos, &s);	// split line
		if (InsertLine(s, row + 1) != NOERROR)
		    break;
		if (row == Cursor.Row)	// keep cursor position where it is, even if string splits
		    if (Cursor.Column >= Lines[row]->Length())
			if (LastResult == NOERROR) {
			    Cursor.Column -= Lines[row]->Length();
			    Cursor.Row++;
			} else
			    ConstrainCursor();
		--row;		// check this line again without the CR
		//last_wrapped = true;
	    } else if (MeasureTextWidth(Lines[row]->Get()) > Width) {
		char *
		    temp = (char *)
		    Lines[row]->
		    Get();
		short
		    word =
		    strlen(temp) -
		    1;
		if (MeasureTextWidth(temp) > Width) {
		    // while line doesn't fit, skip backwards until it does
		    char
			hold =
			1;
		    while (temp[word] && hold) {
			hold = temp[word];
			temp[word] = '\0';
			if (MeasureTextWidth(temp) < Width) {
			    temp[word] = hold;
			    hold = 0;
			} else
			    temp[word] = hold;
			word--;
		    }
		}
		while (isspace(temp[word]))	// if breaking on a space, adjust to next word
		    word++;
		bool
		    single =
		    false;
		if (Flags & EF_NOWRAP)
		    Lines[row]->ChopAt(word, &s);	// split line
		else {
		    Lines[row]->WrapAt(word, &s);	// break at width or next word (no exact break on space...)
		    if (0 == strcmp(s, " ")) {
			single = true;
			if (!DeleteChar
			    && Lines[Cursor.Row]->Length() ==
			    Cursor.Column - 1) {
			    Special = true;
			    MoveCursor(MOVE_RIGHT, 1);
			    Special = false;
			    int
				lines =
				GetLineCount();
			    if (Cursor.Row == lines) {
				if (InsertLine(Cursor.Row, false) != NOERROR)
				    break;
			    }
			    if (InsertCharacter(' ') != NOERROR) {
				break;
			    }
			}
		    }
		}
		if (!single && InsertLine(s, row + 1) != NOERROR)
		    break;
		if (row == Cursor.Row)	// keep cursor position where it is, even if string splits
		    if (Cursor.Column >= Lines[row]->Length())
			if (LastResult == NOERROR) {
			    Cursor.Column -= Lines[row]->Length();
			    Cursor.Row++;
			} else
			    ConstrainCursor();
		last_wrapped = true;
	    } else if (Lines[row]->LastCharacter() == HARD_RETURN) {
		// if doing all, skip this line and go on, else stop here
		if (!all)
		    break;
	    } else if (MeasureTextWidth(Lines[row]->Get()) == Width)
		continue;	// if just right, skip it and go on
	    else if (row < (LineCount - 1)) {	// lines too short?  Join as many as necessary
		if (CanUnwrap(row + 1))	// check if next line should unwrap
		{
		    // if line can unwrap, keep unwrapping until max length is in the line
		    while ((row < (LineCount - 1))
			   && (MeasureTextWidth(Lines[row]->Get()) < Width)) {
			if (Lines[row]->LastCharacter() == HARD_RETURN)
			    break;
			if (row + 1 == Cursor.Row) {	// about to move cursor?
			    Cursor.Row = row;
			    Cursor.Column += Lines[row]->Length();
			}
			*Lines[row] += *Lines[row + 1];	// add next line
			long
			    temprow =
			    Cursor.
			    Row;
			long
			    tempcol =
			    Cursor.
			    Column;
			Cursor.Row = row + 1;
			if (RemoveLine() != NOERROR)
			    break;
			if (MoveTo(temprow, tempcol) == NOERROR)
			    if (Cursor.Row > row)
				Cursor.Row--;
		    }
		    row--;	// make next pass re-examine this line
		}
	    }
	}
    CharCount = realcount;	// splitting lines and then re-inserting them
    // gets them counted twice, so just set the count
    // back like it was when we started
    if (!LineCount || !CharCount)
	Clear();
    return LastResult;
}

rclError EditorEngine::ReformatDocument(short wide)
{
    Width = wide;
    LastResult = ReformatParagraph(TRUE, TRUE);
    return LastResult;
}

//
//  EditorEngine::EditorEngine(short editwidth, bool readonly=FALSE, unsigned long flags=0,long maxch=0,long maxlin=0, char *clipname=NULL)
//
//  This constructor just sets up an editor engine.  The only required parameter
//  is editwidth, which sets how wide the editor's lines should be.  This might be 
//  characters, pixels, inches, or whatever else you want, depending on how you 
//  define the MeasureTextWidth() function.  By default, it is characters.
//

EditorEngine::EditorEngine(short editwidth, bool readonly,
			   unsigned long flags, long maxch, long maxlin)
{
    CharCount = LineCount = 0;
    Lines = NULL;

    Clear();
    EditStatus = LastResult = NOERROR;
    LastFindStatus = FALSE;
    last_wrapped = false;
    Width = editwidth;
    if (Width > 32000)
	Width = 32000;		// wString objects have signed integer indeces...
    Flags = flags;
    MaxChars = maxch;
    MaxLines = maxlin;
    _ReadOnly = readonly;
    TabStops = NULL;
    HasChanged = FALSE;
    BlockStart.Row = BlockEnd.Row = BlockStart.Column = BlockEnd.Column = 0;
    DeleteChar = false;
    Special = false;
}


//
//  EditorEngine::~EditorEngine()
//
//  Destroys the editor engine, freeing up all associated memory.  If
//  you don't save the document before this, it's gone.
//

EditorEngine::~EditorEngine()
{
    long count;

    if (TabStops)
	delete[]TabStops;
    for (count = 0; count < LineCount; count++)
	delete Lines[count];	// if any lines allocated, free them
    if (Lines)
	free(Lines);		// free the line array itself
}


wString EditorEngine::GetLine(long whichline) const
{
    const char *
	what =
	"";

    if (whichline >= 0 && whichline < LineCount)
	what = (*Lines[whichline]).Get();
    return wString(what);
}

rclError EditorEngine::ReplaceLineAt(long line, wString & newstr)
{
    if (_ReadOnly)
	return LIMITS_FULL_ERROR;
    rclError
	err =
	NOERROR;
    if (line >= 0 && line < LineCount)
	(*Lines[line]) = newstr;
    else
	err = RANGE_ERROR;
    return (err);
}

bool
PosInRange(const EditPosition & pos, const EditPosition & from,
	   const EditPosition & to)
{
    // report whether a position is in the given range. ASSUMES RANGE IS ALREADY SORTED
    bool
	inrange =
	false;
    // if between start and end rows, it's in range regardless of column
    if ((pos.Row > from.Row) && (pos.Row < to.Row))
	inrange = true;
    // else if on start row, is in range if column after start column
    else if ((pos.Row == from.Row) && (pos.Column >= from.Column))
	inrange = true;
    // else if on end row, is in range if column before end column
    else if ((pos.Row == to.Row) && (pos.Column <= to.Column))
	inrange = true;
    return inrange;
}


rclError EditorEngine::ReplaceCharacter(char character)
{
    last_wrapped = false;
    if (_ReadOnly) {
	LastResult = LIMITS_FULL_ERROR;
    } else {
	LastResult = NOERROR;
	bool
	    insert =
	    FALSE;
	if (LineCount) {
	    if (character == HARD_RETURN)
		insert = TRUE;
	    else if (Lines[Cursor.Row]->Get()[Cursor.Column] == HARD_RETURN)
		insert = TRUE;
	    else if (Lines[Cursor.Row]->Get()[Cursor.Column] == '\0')
		insert = TRUE;
	    else if (Lines[Cursor.Row]->Length() < Cursor.Column)
		insert = TRUE;
	    else {
		Lines[Cursor.Row]->DeleteAt((int) Cursor.Column, 1);
		Lines[Cursor.Row]->InsertAt((int) Cursor.Column, character);
		MoveCursor(MOVE_RIGHT, 1);
	    }
	} else
	    insert = TRUE;
	if (insert)
	    LastResult = InsertCharacter(character);
	HasChanged = TRUE;
    }
    return LastResult;
}

rclError EditorEngine::InsertCharacter(char character)
{

    //  printf("InsertCharacter(%c)\n\n", character);
    last_wrapped = false;

    if (_ReadOnly) {
	//      printf("1\n");
	LastResult = LIMITS_FULL_ERROR;
    } else {
	//      printf("2\n");
	LastResult = NOERROR;

	if (CharCount && (Flags & EF_BYTELIMITS)) {
	    //      printf("2a\n");
	    if (CharCount >= MaxChars) {
		//              printf("2a0\n");
		LastResult = LIMITS_FULL_ERROR;
	    }

	}

	if (LastResult == NOERROR) {
	    //      printf("2b\n");
	    HasChanged = TRUE;

	    if (!CharCount) {
		//              printf("2b0\n");
		// Possible optimization in InsertLine()?
		LastResult = InsertLine();
	    }

	    if (Flags & EF_FREEFORM) {
		//              printf("2b1\n");
		if (Cursor.Column > Lines[Cursor.Row]->Length()) {
		    wString
			s(' ',
			  (int) (Cursor.Column -
				 Lines[Cursor.Row]->Length()));

		    if (Lines[Cursor.Row]->LastCharacter() == HARD_RETURN)
			Lines[Cursor.Row]->InsertAt(Lines[Cursor.Row]->
						    Length() - 1, s);
		    else
			*Lines[Cursor.Row] += s;

		}

	    }

	    long
		lastlen =
		Lines[Cursor.Row]->
		Length();
	    //      printf("2b InsertAt()\n");
	    Lines[Cursor.Row]->InsertAt((int) Cursor.Column, character);

	    if (lastlen < Lines[Cursor.Row]->Length())
		CharCount++;

	    if (character == HARD_RETURN)	// is user hit return, split line at cursor
	    {
		wString
		    s;
		Lines[Cursor.Row]->ChopAt((int) Cursor.Column + 1, &s);
		LastResult = InsertLine(s, Cursor.Row + 1);

		if (LastResult == NOERROR) {
		    Cursor.Row++;
		    Cursor.Column = 0;
		    LastResult = ReformatParagraph();
		}

	    } else {
		//              printf("2b3\n");

		MoveCursor(MOVE_RIGHT, 1);

		if (isspace(character) && Cursor.Row) {
		    //  printf("2b3a\n");
		    CheckLineFit();
		}


		const char *
		    t =
		    Lines[Cursor.Row]->
		    Get();
		short
		    lwide =
		    MeasureTextWidth(t);

		// short lwide = 1;

		if (lwide > Width) {
		    //              printf("2b3b\n");
		    LastResult = ReformatParagraph();
		}
	    }

	}
    }

    //  printf("2c\n\n\n");
    return LastResult;

}


rclError EditorEngine::InsertLine(long atline, bool aslast)
{
    if (_ReadOnly)
	LastResult = LIMITS_FULL_ERROR;
    else {
	LastResult = NOMEMORY;	// assume the worst...
	wString
	    s;
	if (s.Capacity())
	    LastResult = InsertLine(s, atline, aslast);
    }
    return LastResult;
}

rclError
    EditorEngine::InsertLine(const wString & str, long atline, bool aslast)
{
    if (_ReadOnly)
	LastResult = LIMITS_FULL_ERROR;
    else {
	if (LineCount && (Flags & EF_LINELIMITS))	// if would be past limit, just return error
	    if (LineCount + 1 >= MaxLines)
		return (LastResult = LIMITS_FULL_ERROR);
	if (CharCount && (Flags & EF_BYTELIMITS))
	    if (CharCount + str.Length() >= MaxChars)	// if would be past limit, just return error
		LastResult = LIMITS_FULL_ERROR;
	LastResult = NOMEMORY;	// assume the worst...
	HasChanged = TRUE;
	wString *s = new wString;
	if (s)
	    if (s->Capacity())	// make sure allocation of buffer went OK
	    {
		(*s) = str;
		wString **newlines = (wString **) calloc(1,
							 (size_t) (sizeof
								   (wString *)
								   *
								   (LineCount
								    + 1)));
		if (newlines) {
		    long count;
		    if (Lines) {
			memcpy(newlines, Lines,
			       (size_t) (sizeof(wString *) * (LineCount)));
			free(Lines);
		    }
		    Lines = newlines;
		    if (aslast || !CharCount || (atline >= LineCount))
			Lines[LineCount] = s;	// add as last line
		    else {	// else shift lines down, insert
			if (atline == -1)
			    atline = Cursor.Row;
			for (count = LineCount; count > atline; count--)	// this is right since LineCount hasn't
			    Lines[count] = Lines[count - 1];	// been incremented yet...
			Lines[atline] = s;
		    }
		    LineCount++;	// don't forget to count the line
		    CharCount += s->Length();
		    LastResult = NOERROR;
		}
	    } else
		delete s;
    }
    return LastResult;
}


short
EditorEngine::MeasureTextWidth(const char *t)
{
    return (strlen(t));		// default is width in characters
}

short
EditorEngine::MeasureTextHeight(const char *t)
{
    return (1);			// default is just 1 (line)
}

void
EditorEngine::CheckLineFit()
{
#if 0
    bool unwrap = FALSE;
#endif

    if (CanUnwrap(Cursor.Row)) {
	// word at start is short enough to fit on end of previous
	long lenprev = Lines[Cursor.Row - 1]->Length();
	long currow = Cursor.Row - 1;
	long curcol = Cursor.Column;
	*Lines[Cursor.Row - 1] += *Lines[Cursor.Row];	// attach entire current line to previous
	RemoveLine();		// and remove current
	Cursor.Row = currow;	// move cursor out, let reformat put it right...
	Cursor.Column = curcol + lenprev;	// move to end of line plus length of word wrapped
	ReformatParagraph();
	last_wrapped = true;	// unwrap sets this true too.
    }
}

bool EditorEngine::CanUnwrap(long lineno)
{
    bool
	unwrap =
	FALSE;

    if (lineno && !isspace(Lines[lineno]->Get()[0]))
	if (Lines[lineno - 1]->LastCharacter() != HARD_RETURN) {
	    short
		count;
	    short
		maxfit =
		Width -
		MeasureTextWidth(Lines[lineno - 1]->Get());
	    // this pointer modifies only the contents of the buffer without making any
	    // other engine calls, so it's safe to use it like this.
	    char *
		buff = (char *)
		Lines[lineno]->
		Get();
	    if (MeasureTextWidth(Lines[lineno]->Get()) <= maxfit)
		unwrap = TRUE;
	    else
		for (count = 0; count <= Lines[lineno]->Length(); count++)
		    if (isspace(buff[count]) || (buff[count] == '\0')) {
			char
			    holdit =
			    buff[count];
			buff[count] = '\0';
			//if(MeasureTextWidth(Lines[Cursor.Row]->Copy(0, count)) <= maxfit)
			if (MeasureTextWidth(buff) <= maxfit)
			    unwrap = TRUE;
			buff[count] = holdit;
			break;
		    }
	}
    return (unwrap);
}



//
//  EditorEngine::CopyRegion(bool cut)
//
//  Copies the currently marked region  into the clipboard.
//  Now this is handled by descendant classes.  This used to use a
//  file-based clipboard, but it was yanked since it's not so useful anymore.
//

rclError EditorEngine::CopyRegion(bool cut)
{
    LastResult = NOERROR;
    if (LastResult == NOERROR || LastResult == NO_REGION)
	Command(CLEAR_MARKS);
    return LastResult;
}

rclError EditorEngine::CopyRegionTo(char *copybuffer)
{
    //  printf("EditorEngine::CopyRegionTo() GetSelectionSize()+1 = %d\n", GetSelectionSize()+1);
    long
	buffpos =
	0;
    memset(copybuffer, 0, GetSelectionSize() + 1);
    EditPosition
	Mark =
	BlockStart;
    EditPosition
	endit =
	Cursor;
    SortPositions(Mark, endit);
    if ((Mark.Row != endit.Row) || (Mark.Column != endit.Column)) {
	wString
	    current =
	    GetLine(Mark.Row);
	if (endit.Row == Mark.Row)	// if single line in selection
	{
	    strncpy(copybuffer, &(current.Get()[Mark.Column]),
		    endit.Column - Mark.Column);
	} else {
	    strcpy(copybuffer, &(current.Get()[Mark.Column]));
	    buffpos += strlen(copybuffer);
	    for (long line = Mark.Row + 1; line < endit.Row; line++) {
		current = GetLine(line);
		strcpy(copybuffer + buffpos, current.Get());
		buffpos += current.Length();
	    }
	    current = GetLine(endit.Row);
	    strncpy(copybuffer + buffpos, current.Get(), endit.Column);
	}
    }
    return (NOERROR);
}

rclError EditorEngine::DeleteRegion()
{
    if (_ReadOnly)
	return NOERROR;
    last_wrapped = false;
    LastResult = NOERROR;
    if ((BlockStart.Row != BlockEnd.Row)
	|| (BlockStart.Column != BlockEnd.Column)) {
	SortRegion();
	long
	    crow,
	    srcrow;
	HasChanged = TRUE;
	bool
	    realloclines =
	    FALSE;
	if (BlockStart.Row == BlockEnd.Row) {	// deleting from single line?
	    Lines[BlockStart.Row]->DeleteAt((int) BlockStart.Column,
					    (int) (BlockEnd.Column -
						   BlockStart.Column));
	    if (Cursor.Column > BlockStart.Column
		&& Cursor.Column < BlockEnd.Column)
		Cursor.Column = BlockStart.Column;
	    else if (Cursor.Column > BlockEnd.Column)
		Cursor.Column -= (int) (BlockEnd.Column - BlockStart.Column);
	} else if (BlockStart.Column == 0 && BlockEnd.Column == 0) {
	    for (short drow = (int)BlockStart.Row; drow < BlockEnd.Row;
		 drow++) {
		CharCount -= Lines[drow]->Length();
		delete
		    Lines[drow];
	    }
	    realloclines = TRUE;
	    crow = BlockStart.Row;
	    srcrow = BlockEnd.Row;
	    while (srcrow < LineCount)
		Lines[crow++] = Lines[srcrow++];
	    LineCount -= (BlockEnd.Row - BlockStart.Row);
	} else {
	    CharCount -=
		(int) (Lines[BlockStart.Row]->Length() - BlockStart.Column);
	    Lines[BlockStart.Row]->ChopAt((int) BlockStart.Column);
	    if (BlockEnd.Column)	// if in column zero, nothing to do...
	    {
		CharCount -= BlockEnd.Column;
		Lines[BlockEnd.Row]->DeleteAt(0, (int) BlockEnd.Column);
	    }
	    *Lines[BlockStart.Row] += *Lines[BlockEnd.Row];	// close split in block
	    delete
		Lines[BlockEnd.Row];
	    for (crow = BlockStart.Row + 1; crow < BlockEnd.Row; crow++) {	// first delete all the lines as necessary
		CharCount -= Lines[crow]->Length();
		delete
		    Lines[crow];
		Lines[crow] = NULL;
	    }
	    realloclines = TRUE;
	    crow = BlockStart.Row + 1;
	    srcrow = BlockEnd.Row + 1;
	    while (srcrow < LineCount)
		Lines[crow++] = Lines[srcrow++];
	    LineCount -= (BlockEnd.Row - BlockStart.Row);
	}
	if (realloclines) {
	    if (LineCount > 0) {
		wString **
		    newlines = (wString **)
		    calloc(1,
			   (size_t) (sizeof(wString *) * (LineCount)));
		if (newlines) {
		    if (Lines) {
			memcpy(newlines, Lines,
			       (size_t) (sizeof(wString *) * (LineCount)));
			free(Lines);
		    }
		    Lines = newlines;
		}
	    } else
		Clear();
	    if (Cursor.Row >= BlockStart.Row)	// need to move cursor?
		if (Cursor.Row < BlockEnd.Row || (Cursor.Row == BlockEnd.Row && Cursor.Column <= BlockEnd.Column)) {	// cursor was inside block?
		    MoveTo(BlockStart.Row, BlockStart.Column);
		} else {	// else past end of block, subtract row, columns as necessary
		    Cursor.Row -= (BlockEnd.Row - BlockStart.Row - 1);
		    Cursor.Column -= BlockEnd.Column;
		}
	}
	ConstrainCursor();
    } else
	LastResult = NO_REGION;
    if (LastResult == NOERROR)
	Command(CLEAR_MARKS);
    if (CharCount < 0) {
	CharCount = 0;
    }
    if (LineCount < 0) {
	LineCount = CharCount = 0;
    }
    if (CharCount) {
	CheckLineFit();
	ReformatParagraph();
    } else
	Clear();
    return LastResult;
}


int
EditorEngine::FindSelectPos(long line, long *start, long *end)
{
    EditPosition holdstart = BlockStart;
    EditPosition holdend = BlockEnd;
    int inselection = 0;
    *start = *end = 0;
    if (line > LineCount)
	return (0);
    if (BlockStart.Row + BlockStart.Column + BlockEnd.Row + BlockEnd.Column ==
	0)
	return (0);
    SortRegion();
    inselection = ((line >= BlockStart.Row) && (line <= BlockEnd.Row));
    if (inselection) {		// line inside selection area.
	*end = Lines[line]->Length();
	if (line == BlockStart.Row)
	    *start = BlockStart.Column;
	if (line == BlockEnd.Row)
	    *end = BlockEnd.Column;
    }
    BlockStart = holdstart;
    BlockEnd = holdend;
    return (inselection);
}

long
EditorEngine::GetSelectionSize()
{
    long size = 0;
    EditPosition holdstart = BlockStart;
    EditPosition holdend = BlockEnd;

    if (BlockStart.Row + BlockStart.Column + BlockEnd.Row + BlockEnd.Column ==
	0)
	return (0);
    if ((BlockStart.Row == BlockEnd.Row)
	&& (BlockStart.Column == BlockEnd.Column))
	return (0);
    SortRegion();
    for (long line = BlockStart.Row; line <= BlockEnd.Row; line++)
	size += Lines[line]->Length();
    size -= BlockStart.Column;
    size -= Lines[BlockEnd.Row]->Length() - BlockEnd.Column;
    BlockStart = holdstart;
    BlockEnd = holdend;
    return (size);
}


rclError EditorEngine::RemoveLine(long line)
{
    if (LineCount && !_ReadOnly) {
	long
	    row =
	    Cursor.
	    Row;
	long
	    col =
	    Cursor.
	    Column;

	if (MoveCursor(MOVE_ABSOLUTE_ROW, line) == NOERROR) {
	    LastResult = RemoveLine();
	    if (row > Cursor.Row)
		row--;
	    MoveCursor(MOVE_ABSOLUTE_ROW, row);
	    MoveCursor(MOVE_ABSOLUTE_COLUMN, col);
	    ConstrainCursor();
	}
    }
    return LastResult;
}

rclError EditorEngine::RemoveLine()
{
    if (_ReadOnly)
	return NOERROR;
    LastResult = NOERROR;
    if (LineCount > 1) {
	HasChanged = TRUE;
	long
	    count;
	wString *
	    s =
	    Lines[Cursor.Row];	// remember string but don't delete till realloc OK
	wString **
	    newlines = (wString **)
	    calloc(1,
		   (size_t) (sizeof(wString *) * (LineCount - 1)));
	if (newlines) {
	    if (Lines)		// there better be some lines if it got to here!
	    {
		for (count = Cursor.Row; count < LineCount - 1; count++)
		    Lines[count] = Lines[count + 1];
		memcpy(newlines, Lines,
		       (size_t) (sizeof(wString *) * (LineCount - 1)));
		free(Lines);
	    }
	    LineCount--;
	    Lines = newlines;
	    if (s)		// alloc was OK, so we can delete this now.
		delete
		    s;		// (if alloc fails, this string needs to remain in place...)
	} else
	    LastResult = NOMEMORY;
	if (!(Flags & EF_FREEFORM))
	    ConstrainCursor();
    } else {
	MoveCursor(MOVE_ABSOLUTE_COLUMN, 0);
	DeleteToEOL();
    }
    return LastResult;
}


// This is now handled by descendant classes

rclError EditorEngine::PasteRegion(bool replace)
{
    last_wrapped = false;
    LastResult = NOERROR;
    return LastResult;
}

RCLDLL void
SortPositions(EditPosition & first, EditPosition & last)
{
    long
	temprow,
	tempcol;

    if (first.Row == last.Row) {
	if (last.Column < first.Column)	// if on same row, just swap columns
	{
	    tempcol = first.Column;
	    first.Column = last.Column;
	    last.Column = tempcol;
	}
    } else if (last.Row < first.Row)	// else must swap both
    {
	temprow = first.Row;
	tempcol = first.Column;
	first.Row = last.Row;
	first.Column = last.Column;
	last.Row = temprow;
	last.Column = tempcol;
    }
}
