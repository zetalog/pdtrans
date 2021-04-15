# Microsoft Developer Studio Project File - Name="libpdt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libpdt - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libpdt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libpdt.mak" CFG="libpdt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libpdt - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libpdt - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libpdt - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\bin\release"
# PROP BASE Intermediate_Dir "..\obj\release\libpdt"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin\release"
# PROP Intermediate_Dir "..\obj\release\libpdt"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\win32" /I "..\include" /I "." /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D HAVE_CONFIG_H=1 /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libpdt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\bin\debug"
# PROP BASE Intermediate_Dir "..\obj\debug\libpdt"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin\debug"
# PROP Intermediate_Dir "..\obj\debug\libpdt"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\win32" /I "..\include" /I "." /D "_DEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D HAVE_CONFIG_H=1 /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libpdt - Win32 Release"
# Name "libpdt - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\asp.c
# End Source File
# Begin Source File

SOURCE=.\cpp.c
# End Source File
# Begin Source File

SOURCE=..\win32\direct.c
# End Source File
# Begin Source File

SOURCE=.\getline.c
# End Source File
# Begin Source File

SOURCE=.\js.c
# End Source File
# Begin Source File

SOURCE=.\misc.c
# End Source File
# Begin Source File

SOURCE=.\plugin.c
# End Source File
# Begin Source File

SOURCE=.\process.c
# End Source File
# Begin Source File

SOURCE=.\property.c
# End Source File
# Begin Source File

SOURCE=.\rc.c
# End Source File
# Begin Source File

SOURCE=..\tools\rc\rc.exe

!IF  "$(CFG)" == "libpdt - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying $(InputPath)...
OutDir=.\..\bin\release
InputPath=..\tools\rc\rc.exe
InputName=rc

"$(OutDir)\$(InputName).exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(OutDir)\$(InputName).exe

# End Custom Build

!ELSEIF  "$(CFG)" == "libpdt - Win32 Debug"

# Begin Custom Build - Copying $(InputPath)...
OutDir=.\..\bin\debug
InputPath=..\tools\rc\rc.exe
InputName=rc

"$(OutDir)\$(InputName).exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(OutDir)\$(InputName).exe

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\tools\rc\rc.hlp

!IF  "$(CFG)" == "libpdt - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying $(InputPath)...
OutDir=.\..\bin\release
InputPath=..\tools\rc\rc.hlp
InputName=rc

"$(OutDir)\$(InputName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(OutDir)\$(InputName).hlp

# End Custom Build

!ELSEIF  "$(CFG)" == "libpdt - Win32 Debug"

# Begin Custom Build - Copying $(InputPath)...
OutDir=.\..\bin\debug
InputPath=..\tools\rc\rc.hlp
InputName=rc

"$(OutDir)\$(InputName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(OutDir)\$(InputName).hlp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\tools\rc\rcdll.dll

!IF  "$(CFG)" == "libpdt - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying $(InputPath)...
OutDir=.\..\bin\release
InputPath=..\tools\rc\rcdll.dll
InputName=rcdll

"$(OutDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(OutDir)\$(InputName).dll

# End Custom Build

!ELSEIF  "$(CFG)" == "libpdt - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying $(InputPath)...
OutDir=.\..\bin\debug
InputPath=..\tools\rc\rcdll.dll
InputName=rcdll

"$(OutDir)\$(InputName).dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(InputPath) $(OutDir)\$(InputName).dll

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\resource.c
# End Source File
# Begin Source File

SOURCE=.\sql.c
# End Source File
# Begin Source File

SOURCE=.\vb.c
# End Source File
# Begin Source File

SOURCE=.\vector.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\win32\config.h
# End Source File
# Begin Source File

SOURCE=..\win32\dirent.h
# End Source File
# Begin Source File

SOURCE=.\getline.h
# End Source File
# Begin Source File

SOURCE=.\pd_asp.h
# End Source File
# Begin Source File

SOURCE=.\pd_cpp.h
# End Source File
# Begin Source File

SOURCE=.\pd_intern.h
# End Source File
# Begin Source File

SOURCE=.\pd_js.h
# End Source File
# Begin Source File

SOURCE=.\pd_misc.h
# End Source File
# Begin Source File

SOURCE=.\pd_plugin.h
# End Source File
# Begin Source File

SOURCE=.\pd_property.h
# End Source File
# Begin Source File

SOURCE=.\pd_rc.h
# End Source File
# Begin Source File

SOURCE=.\pd_resource.h
# End Source File
# Begin Source File

SOURCE=.\pd_sql.h
# End Source File
# Begin Source File

SOURCE=..\include\pd_trans.h
# End Source File
# Begin Source File

SOURCE=.\pd_vb.h
# End Source File
# Begin Source File

SOURCE=.\vector.h
# End Source File
# End Group
# End Target
# End Project
