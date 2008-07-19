# Microsoft Developer Studio Project File - Name="PixilDT" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=PixilDT - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PixilDT.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PixilDT.mak" CFG="PixilDT - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PixilDT - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "PixilDT - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PixilDT - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "c:\fltk-1.0.11" /I "c:\flvw\1.0" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /map /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "PixilDT - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "c:\fltk-1.0.11" /I "c:\flvw\1.0" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "PixilDT - Win32 Release"
# Name "PixilDT - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AddressBook.cpp
# End Source File
# Begin Source File

SOURCE=.\AddressBookCategoryDB.cpp
# End Source File
# Begin Source File

SOURCE=.\AddressBookDB.cpp
# End Source File
# Begin Source File

SOURCE=.\AddressBookDetails.cpp
# End Source File
# Begin Source File

SOURCE=.\AddressBookDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AddressBookList.cpp
# End Source File
# Begin Source File

SOURCE=.\CategoryDB.cpp
# End Source File
# Begin Source File

SOURCE=.\CategoryEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\CategoryEditorList.cpp
# End Source File
# Begin Source File

SOURCE=.\CustomFieldDB.cpp
# End Source File
# Begin Source File

SOURCE=.\CustomFieldEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\DatePicker.cpp
# End Source File
# Begin Source File

SOURCE=.\DatePickerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FindDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FindList.cpp
# End Source File
# Begin Source File

SOURCE=.\FLTKUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Images.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoDB.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\IniDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\InputBox.cpp
# End Source File
# Begin Source File

SOURCE=.\InputNotify.cpp
# End Source File
# Begin Source File

SOURCE=.\LeftGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\ListByDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Note.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteDB.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteDetails.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteEditorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteList.cpp
# End Source File
# Begin Source File

SOURCE=.\Notes.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesCategoryDB.cpp
# End Source File
# Begin Source File

SOURCE=.\NxDbAccess.cpp
# End Source File
# Begin Source File

SOURCE=.\NxDbColumn.cpp
# End Source File
# Begin Source File

SOURCE=.\NxDbRow.cpp
# End Source File
# Begin Source File

SOURCE=.\Options.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PixilDT.cpp
# End Source File
# Begin Source File

SOURCE=.\PixilDT.rc
# End Source File
# Begin Source File

SOURCE=.\PixilDTApp.cpp
# End Source File
# Begin Source File

SOURCE=.\PixilMainWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\Printer.cpp
# End Source File
# Begin Source File

SOURCE=.\ScheduleContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\ScheduleDay.cpp
# End Source File
# Begin Source File

SOURCE=.\ScheduleEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\ScheduleHours.cpp
# End Source File
# Begin Source File

SOURCE=.\Scheduler.cpp
# End Source File
# Begin Source File

SOURCE=.\SchedulerCategoryDB.cpp
# End Source File
# Begin Source File

SOURCE=.\SchedulerChangeTypeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SchedulerDaily.cpp
# End Source File
# Begin Source File

SOURCE=.\SchedulerDB.cpp
# End Source File
# Begin Source File

SOURCE=.\SchedulerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SchedulerMonthly.cpp
# End Source File
# Begin Source File

SOURCE=.\SchedulerRepeatData.cpp
# End Source File
# Begin Source File

SOURCE=.\SchedulerRepeatDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SchedulerTimeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SchedulerWeekly.cpp
# End Source File
# Begin Source File

SOURCE=.\SchedulerYearly.cpp
# End Source File
# Begin Source File

SOURCE=.\SpinInput.cpp
# End Source File
# Begin Source File

SOURCE=.\SplashDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TableBase.cpp
# End Source File
# Begin Source File

SOURCE=.\TimeFunc.cpp
# End Source File
# Begin Source File

SOURCE=.\ToDoList.cpp
# End Source File
# Begin Source File

SOURCE=.\ToDoListCategoryDB.cpp
# End Source File
# Begin Source File

SOURCE=.\ToDoListDB.cpp
# End Source File
# Begin Source File

SOURCE=.\ToDoListDetails.cpp
# End Source File
# Begin Source File

SOURCE=.\ToDoListList.cpp
# End Source File
# Begin Source File

SOURCE=.\ToDoListShowDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Toolbar.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolbarButton.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AddressBook.h
# End Source File
# Begin Source File

SOURCE=.\AddressBookCategoryDB.h
# End Source File
# Begin Source File

SOURCE=.\AddressBookDB.h
# End Source File
# Begin Source File

SOURCE=.\AddressBookDBDef.h
# End Source File
# Begin Source File

SOURCE=.\AddressBookDetails.h
# End Source File
# Begin Source File

SOURCE=.\AddressBookDlg.h
# End Source File
# Begin Source File

SOURCE=.\AddressBookList.h
# End Source File
# Begin Source File

SOURCE=.\CategoryDB.h
# End Source File
# Begin Source File

SOURCE=.\CategoryDBDef.h
# End Source File
# Begin Source File

SOURCE=.\CategoryDeleteDlg.h
# End Source File
# Begin Source File

SOURCE=.\CategoryEditor.h
# End Source File
# Begin Source File

SOURCE=.\CategoryEditorList.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\CustomFieldDB.h
# End Source File
# Begin Source File

SOURCE=.\CustomFieldDBDef.h
# End Source File
# Begin Source File

SOURCE=.\CustomFieldEditor.h
# End Source File
# Begin Source File

SOURCE=.\DatePicker.h
# End Source File
# Begin Source File

SOURCE=.\DatePickerDlg.h
# End Source File
# Begin Source File

SOURCE=.\Dialog.h
# End Source File
# Begin Source File

SOURCE=.\FindDlg.h
# End Source File
# Begin Source File

SOURCE=.\FindList.h
# End Source File
# Begin Source File

SOURCE=.\FLTKUtil.h
# End Source File
# Begin Source File

SOURCE=.\HelpID.h
# End Source File
# Begin Source File

SOURCE=.\ImageBox.h
# End Source File
# Begin Source File

SOURCE=.\Images.h
# End Source File
# Begin Source File

SOURCE=.\InfoDB.h
# End Source File
# Begin Source File

SOURCE=.\InfoDBDef.h
# End Source File
# Begin Source File

SOURCE=.\InfoGroup.h
# End Source File
# Begin Source File

SOURCE=.\IniDlg.h
# End Source File
# Begin Source File

SOURCE=.\InputBox.h
# End Source File
# Begin Source File

SOURCE=.\InputNotify.h
# End Source File
# Begin Source File

SOURCE=.\LeftGroup.h
# End Source File
# Begin Source File

SOURCE=.\ListByDlg.h
# End Source File
# Begin Source File

SOURCE=.\Messages.h
# End Source File
# Begin Source File

SOURCE=.\Note.h
# End Source File
# Begin Source File

SOURCE=.\NoteDB.h
# End Source File
# Begin Source File

SOURCE=.\NoteDBDef.h
# End Source File
# Begin Source File

SOURCE=.\NoteDetails.h
# End Source File
# Begin Source File

SOURCE=.\NoteEditor.h
# End Source File
# Begin Source File

SOURCE=.\NoteEditorDlg.h
# End Source File
# Begin Source File

SOURCE=.\NoteList.h
# End Source File
# Begin Source File

SOURCE=.\Notes.h
# End Source File
# Begin Source File

SOURCE=.\NotesCategoryDB.h
# End Source File
# Begin Source File

SOURCE=.\NxDbAccess.h
# End Source File
# Begin Source File

SOURCE=.\NxDbColumn.h
# End Source File
# Begin Source File

SOURCE=.\NxDbRow.h
# End Source File
# Begin Source File

SOURCE=.\Options.h
# End Source File
# Begin Source File

SOURCE=.\OptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\PixilDT.h
# End Source File
# Begin Source File

SOURCE=.\PixilMainWnd.h
# End Source File
# Begin Source File

SOURCE=.\Printer.h
# End Source File
# Begin Source File

SOURCE=.\ScheduleContainer.h
# End Source File
# Begin Source File

SOURCE=.\ScheduleDay.h
# End Source File
# Begin Source File

SOURCE=.\ScheduleEvent.h
# End Source File
# Begin Source File

SOURCE=.\ScheduleHours.h
# End Source File
# Begin Source File

SOURCE=.\Scheduler.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerCategoryDB.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerChangeTypeDlg.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerDaily.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerDB.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerDBDef.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerDlg.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerMonthly.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerRepeatData.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerRepeatDlg.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerTimeDlg.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerWeekly.h
# End Source File
# Begin Source File

SOURCE=.\SchedulerYearly.h
# End Source File
# Begin Source File

SOURCE=.\SpinInput.h
# End Source File
# Begin Source File

SOURCE=.\SplashDlg.h
# End Source File
# Begin Source File

SOURCE=.\TableBase.h
# End Source File
# Begin Source File

SOURCE=.\TimeFunc.h
# End Source File
# Begin Source File

SOURCE=.\ToDoList.h
# End Source File
# Begin Source File

SOURCE=.\ToDoListCategoryDB.h
# End Source File
# Begin Source File

SOURCE=.\ToDoListDB.h
# End Source File
# Begin Source File

SOURCE=.\ToDoListDBDef.h
# End Source File
# Begin Source File

SOURCE=.\ToDoListDetails.h
# End Source File
# Begin Source File

SOURCE=.\ToDoListList.h
# End Source File
# Begin Source File

SOURCE=.\ToDoListShowDlg.h
# End Source File
# Begin Source File

SOURCE=.\Toolbar.h
# End Source File
# Begin Source File

SOURCE=.\ToolbarButton.h
# End Source File
# Begin Source File

SOURCE=.\VCMemoryLeak.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\pixil_icon.ico
# End Source File
# End Group
# End Target
# End Project
