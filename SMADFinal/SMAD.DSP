# Microsoft Developer Studio Project File - Name="SMAD" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SMAD - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SMAD.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SMAD.MAK" CFG="SMAD - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SMAD - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SMAD - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SMAD - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "SMAD - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "SMAD - Win32 Release"
# Name "SMAD - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BackupDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BackupFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\CabFileDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CompressFile.cpp
# End Source File
# Begin Source File

SOURCE=.\DumpEventLog.cpp
# End Source File
# Begin Source File

SOURCE=.\EventInsertSet.cpp
# End Source File
# Begin Source File

SOURCE=.\EventLogDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FCIDLL.C
# End Source File
# Begin Source File

SOURCE=.\MDBMaintenance.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadEventLogs.cpp
# End Source File
# Begin Source File

SOURCE=.\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\RegistryDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SaveRegKey.cpp
# End Source File
# Begin Source File

SOURCE=.\ServiceControl.cpp
# End Source File
# Begin Source File

SOURCE=.\ServiceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SMAD.cpp
# End Source File
# Begin Source File

SOURCE=.\SMAD.rc
# End Source File
# Begin Source File

SOURCE=.\SMADDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SMADErrorLog.cpp
# End Source File
# Begin Source File

SOURCE=.\SMADErrorLogDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TestSmad.cpp
# End Source File
# Begin Source File

SOURCE=.\VersionInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\VersionInfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\VersionSet.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BackupDlg.h
# End Source File
# Begin Source File

SOURCE=.\BackupFiles.h
# End Source File
# Begin Source File

SOURCE=.\CabFileDlg.h
# End Source File
# Begin Source File

SOURCE=.\CompressFile.h
# End Source File
# Begin Source File

SOURCE=.\DumpEventLog.h
# End Source File
# Begin Source File

SOURCE=.\EventInsertSet.h
# End Source File
# Begin Source File

SOURCE=.\EventLogDlg.h
# End Source File
# Begin Source File

SOURCE=.\FCI.H
# End Source File
# Begin Source File

SOURCE=.\Macros.h
# End Source File
# Begin Source File

SOURCE=.\MDBMaintenance.h
# End Source File
# Begin Source File

SOURCE=.\ReadEventLogs.h
# End Source File
# Begin Source File

SOURCE=.\Registry.h
# End Source File
# Begin Source File

SOURCE=.\RegistryDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SaveRegKey.h
# End Source File
# Begin Source File

SOURCE=.\ServiceControl.h
# End Source File
# Begin Source File

SOURCE=.\ServiceDlg.h
# End Source File
# Begin Source File

SOURCE=.\SMAD.h
# End Source File
# Begin Source File

SOURCE=.\SMADDlg.h
# End Source File
# Begin Source File

SOURCE=.\SMADErrorLog.h
# End Source File
# Begin Source File

SOURCE=.\SMADErrorLogDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TestSmad.h
# End Source File
# Begin Source File

SOURCE=.\VersionInfo.h
# End Source File
# Begin Source File

SOURCE=.\VersionInfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\VersionSet.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\SMAD.ico
# End Source File
# Begin Source File

SOURCE=.\res\SMAD.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE="D:\Program Files\DevStudio\VC\lib\VERSION.LIB"
# End Source File
# End Target
# End Project
