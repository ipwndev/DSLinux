# Microsoft Developer Studio Project File - Name="libncftp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libncftp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libncftp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libncftp.mak" CFG="libncftp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libncftp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libncftp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libncftp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W4 /GX /O2 /I "..\Strn" /I "..\sio" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libncftp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\Strn" /I "..\sio" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libncftp - Win32 Release"
# Name "libncftp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\c_chdir.c
# End Source File
# Begin Source File

SOURCE=.\c_chdir3.c
# End Source File
# Begin Source File

SOURCE=.\c_chdirlist.c
# End Source File
# Begin Source File

SOURCE=.\c_chmod.c
# End Source File
# Begin Source File

SOURCE=.\c_delete.c
# End Source File
# Begin Source File

SOURCE=.\c_exists.c
# End Source File
# Begin Source File

SOURCE=.\c_filetype.c
# End Source File
# Begin Source File

SOURCE=.\c_getcwd.c
# End Source File
# Begin Source File

SOURCE=.\c_mkdir.c
# End Source File
# Begin Source File

SOURCE=.\c_mlist1.c
# End Source File
# Begin Source File

SOURCE=.\c_modtime.c
# End Source File
# Begin Source File

SOURCE=.\c_opennologin.c
# End Source File
# Begin Source File

SOURCE=.\c_rename.c
# End Source File
# Begin Source File

SOURCE=.\c_rhelp.c
# End Source File
# Begin Source File

SOURCE=.\c_rmdir.c
# End Source File
# Begin Source File

SOURCE=.\c_rmdirr.c
# End Source File
# Begin Source File

SOURCE=.\c_size.c
# End Source File
# Begin Source File

SOURCE=.\c_sizemdtm.c
# End Source File
# Begin Source File

SOURCE=.\c_symlink.c
# End Source File
# Begin Source File

SOURCE=.\c_type.c
# End Source File
# Begin Source File

SOURCE=.\c_umask.c
# End Source File
# Begin Source File

SOURCE=.\c_utime.c
# End Source File
# Begin Source File

SOURCE=.\errno.c
# End Source File
# Begin Source File

SOURCE=.\ftp.c
# End Source File
# Begin Source File

SOURCE=.\ftw.c
# End Source File
# Begin Source File

SOURCE=.\io_get.c
# End Source File
# Begin Source File

SOURCE=.\io_getfiles.c
# End Source File
# Begin Source File

SOURCE=.\io_getmem.c
# End Source File
# Begin Source File

SOURCE=.\io_getonefile.c
# End Source File
# Begin Source File

SOURCE=.\io_list.c
# End Source File
# Begin Source File

SOURCE=.\io_listmem.c
# End Source File
# Begin Source File

SOURCE=.\io_put.c
# End Source File
# Begin Source File

SOURCE=.\io_putfiles.c
# End Source File
# Begin Source File

SOURCE=.\io_putmem.c
# End Source File
# Begin Source File

SOURCE=.\io_putonefile.c
# End Source File
# Begin Source File

SOURCE=.\io_util.c
# End Source File
# Begin Source File

SOURCE=.\lglob.c
# End Source File
# Begin Source File

SOURCE=.\lglobr.c
# End Source File
# Begin Source File

SOURCE=.\linelist.c
# End Source File
# Begin Source File

SOURCE=.\open.c
# End Source File
# Begin Source File

SOURCE=.\rcmd.c
# End Source File
# Begin Source File

SOURCE=.\rftw.c
# End Source File
# Begin Source File

SOURCE=.\rglob.c
# End Source File
# Begin Source File

SOURCE=.\rglobr.c
# End Source File
# Begin Source File

SOURCE=.\u_close.c
# End Source File
# Begin Source File

SOURCE=.\u_decodeurl.c
# End Source File
# Begin Source File

SOURCE=.\u_error.c
# End Source File
# Begin Source File

SOURCE=.\u_feat.c
# End Source File
# Begin Source File

SOURCE=.\u_fileextn.c
# End Source File
# Begin Source File

SOURCE=.\u_getcwd.c
# End Source File
# Begin Source File

SOURCE=.\u_gethome.c
# End Source File
# Begin Source File

SOURCE=.\u_getopt.c
# End Source File
# Begin Source File

SOURCE=.\u_getpass.c
# End Source File
# Begin Source File

SOURCE=.\u_getusr.c
# End Source File
# Begin Source File

SOURCE=.\u_getutc.c
# End Source File
# Begin Source File

SOURCE=.\u_gmtime.c
# End Source File
# Begin Source File

SOURCE=.\u_localtime.c
# End Source File
# Begin Source File

SOURCE=.\u_misc.c
# End Source File
# Begin Source File

SOURCE=.\u_mkdirs.c
# End Source File
# Begin Source File

SOURCE=.\u_pathcat.c
# End Source File
# Begin Source File

SOURCE=.\u_printf.c
# End Source File
# Begin Source File

SOURCE=.\u_rebuildci.c
# End Source File
# Begin Source File

SOURCE=.\u_scram.c
# End Source File
# Begin Source File

SOURCE=.\u_shutdownci.c
# End Source File
# Begin Source File

SOURCE=.\u_slash.c
# End Source File
# Begin Source File

SOURCE=.\u_unmdtm.c
# End Source File
# Begin Source File

SOURCE=.\unls.c
# End Source File
# Begin Source File

SOURCE=.\util2.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ftp.h
# End Source File
# Begin Source File

SOURCE=.\ncftp.h
# End Source File
# Begin Source File

SOURCE=.\ncftp_errno.h
# End Source File
# Begin Source File

SOURCE=.\syshdrs.h
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# Begin Source File

SOURCE=.\wincfg.h
# End Source File
# End Group
# End Target
# End Project
