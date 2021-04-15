# Microsoft Developer Studio Project File - Name="pd_test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=pd_test - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pd_test.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pd_test.mak" CFG="pd_test - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pd_test - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "pd_test - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pd_test - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\bin\release"
# PROP BASE Intermediate_Dir "..\obj\release\pd_test"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin\release"
# PROP Intermediate_Dir "..\obj\release\pd_test"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\tools\cunit\include" /I "..\lib" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 cunit32.lib kernel32.lib user32.lib gdi32.lib winspool.lib /nologo /subsystem:console /machine:I386 /libpath:"..\tools\cunit\lib"
# Begin Special Build Tool
TargetPath=\CVSROOT\My_translate\bin\release\pd_test.exe
SOURCE="$(InputPath)"
PostBuild_Desc=Unit testing...
PostBuild_Cmds=md ..\obj\test	md ..\obj\test\vb	md ..\obj\test\asp	md ..\obj\test\inc	$(TargetPath)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "pd_test - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\bin\debug"
# PROP BASE Intermediate_Dir "..\obj\debug\pd_test"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin\debug"
# PROP Intermediate_Dir "..\obj\debug\pd_test"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\tools\cunit\include" /I "..\include" /I "..\lib" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cunit32.lib kernel32.lib user32.lib gdi32.lib winspool.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\tools\cunit\lib"
# Begin Special Build Tool
TargetPath=\CVSROOT\My_translate\bin\debug\pd_test.exe
SOURCE="$(InputPath)"
PostBuild_Desc=Unit testing...
PostBuild_Cmds=md ..\obj\test	md ..\obj\test\vb	md ..\obj\test\asp	md ..\obj\test\inc	$(TargetPath)
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "pd_test - Win32 Release"
# Name "pd_test - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\tools\cunit\bin\cunit32.dll

!IF  "$(CFG)" == "pd_test - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying $(InputPath)...
OutDir=.\..\bin\release
InputPath=..\tools\cunit\bin\cunit32.dll
InputName=cunit32

"$(OutDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(OutDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "pd_test - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying $(InputPath)...
OutDir=.\..\bin\debug
InputPath=..\tools\cunit\bin\cunit32.dll
InputName=cunit32

"$(OutDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(OutDir)\$(InputName).dll

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pd_test.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\tools\cunit\include\cunit.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
