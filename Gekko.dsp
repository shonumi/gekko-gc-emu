# Microsoft Developer Studio Project File - Name="Gekko" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Gekko - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Gekko.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Gekko.mak" CFG="Gekko - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Gekko - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Gekko - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Gekko - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /Zm500 /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 winmm.lib opengl32.lib glu32.lib glaux.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib comdlg32.lib dsound.lib /nologo /subsystem:console /machine:I386 /nodefaultlib:"libcmt.lib" /out:"../GekkoLatest.exe"

!ELSEIF  "$(CFG)" == "Gekko - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /Zm500 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib opengl32.lib glu32.lib glaux.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib comdlg32.lib dsound.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"LIBCMT.LIB" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "Gekko - Win32 Release"
# Name "Gekko - Win32 Debug"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\gekko_ico.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\icon3.ico
# End Source File
# Begin Source File

SOURCE=.\JAPAN.ICO
# End Source File
# Begin Source File

SOURCE=.\USA.ICO
# End Source File
# End Group
# Begin Group "Emulator"

# PROP Default_Filter ""
# Begin Group "low level"

# PROP Default_Filter ""
# Begin Group "plugins"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\low level\plugins\dolwinspec.h"
# End Source File
# Begin Source File

SOURCE=".\low level\plugins\plugins.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\plugins\plugins.h"
# End Source File
# End Group
# Begin Group "data"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\low level\data\font_ansi.h"
# End Source File
# Begin Source File

SOURCE=".\low level\data\font_sjis.h"
# End Source File
# End Group
# Begin Group "hardware core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\low level\hardware core\hw_ai.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_ai.h"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_cp.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_cp.h"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_di.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_di.h"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_dsp.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_dsp.h"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_exi.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_exi.h"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_gx.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_gx.h"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_pe.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_pe.h"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_pi.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_pi.h"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_si.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_si.h"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_vi.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\hardware core\hw_vi.h"
# End Source File
# End Group
# Begin Source File

SOURCE=".\low level\flipper.cpp"
# End Source File
# Begin Source File

SOURCE=".\low level\flipper.h"
# End Source File
# End Group
# Begin Group "high level"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\high level\highlevel.cpp"
# End Source File
# Begin Source File

SOURCE=".\high level\highlevel.h"
# End Source File
# Begin Source File

SOURCE=".\high level\hle_audio.cpp"
# End Source File
# Begin Source File

SOURCE=".\high level\hle_crc.h"
# End Source File
# Begin Source File

SOURCE=".\high level\hle_dsp.cpp"
# End Source File
# Begin Source File

SOURCE=".\high level\hle_dsp.h"
# End Source File
# Begin Source File

SOURCE=".\high level\hle_func.h"
# End Source File
# Begin Source File

SOURCE=".\high level\hle_general.cpp"
# End Source File
# Begin Source File

SOURCE=".\high level\hle_general.h"
# End Source File
# Begin Source File

SOURCE=".\high level\hle_math.cpp"
# End Source File
# Begin Source File

SOURCE=".\high level\hle_math.h"
# End Source File
# End Group
# Begin Group "dvd interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\dvd interface\apploader.cpp"
# End Source File
# Begin Source File

SOURCE=".\dvd interface\apploader.h"
# End Source File
# Begin Source File

SOURCE=".\dvd interface\bootrom.cpp"
# End Source File
# Begin Source File

SOURCE=".\dvd interface\bootrom.h"
# End Source File
# Begin Source File

SOURCE=".\dvd interface\dol.cpp"
# End Source File
# Begin Source File

SOURCE=".\dvd interface\elf.cpp"
# End Source File
# Begin Source File

SOURCE=".\dvd interface\gcm.cpp"
# End Source File
# Begin Source File

SOURCE=".\dvd interface\loaders.h"
# End Source File
# Begin Source File

SOURCE=".\dvd interface\realdvd.cpp"
# End Source File
# Begin Source File

SOURCE=".\dvd interface\realdvd.h"
# End Source File
# End Group
# Begin Group "user interface"

# PROP Default_Filter ""
# Begin Group "debugger"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\user interface\debugger\CtrlDisAsmView.cpp"
# End Source File
# Begin Source File

SOURCE=".\user interface\debugger\CtrlDisAsmView.h"
# End Source File
# Begin Source File

SOURCE=".\user interface\debugger\CtrlMemView.cpp"
# End Source File
# Begin Source File

SOURCE=".\user interface\debugger\CtrlMemView.h"
# End Source File
# Begin Source File

SOURCE=".\user interface\debugger\DebugInterface.h"
# End Source File
# Begin Source File

SOURCE=".\user interface\debugger\GekkoDbgInterface.cpp"
# End Source File
# Begin Source File

SOURCE=".\user interface\debugger\GekkoDbgInterface.h"
# End Source File
# Begin Source File

SOURCE=".\user interface\debugger\Symbols.h"
# End Source File
# End Group
# Begin Source File

SOURCE=".\user interface\emu_browser.cpp"
# End Source File
# Begin Source File

SOURCE=".\user interface\emu_browser.h"
# End Source File
# Begin Source File

SOURCE=".\user interface\emu_config.cpp"
# End Source File
# Begin Source File

SOURCE=".\user interface\emu_config.h"
# End Source File
# Begin Source File

SOURCE=".\user interface\emu_dbg.cpp"
# End Source File
# Begin Source File

SOURCE=".\user interface\emu_dbg.h"
# End Source File
# Begin Source File

SOURCE=".\user interface\emu_ini.cpp"
# End Source File
# Begin Source File

SOURCE=".\user interface\emu_ini.h"
# End Source File
# Begin Source File

SOURCE=".\user interface\emu_win.cpp"
# End Source File
# Begin Source File

SOURCE=".\user interface\emu_win.h"
# End Source File
# End Group
# Begin Group "cpu core"

# PROP Default_Filter ""
# Begin Group "interpreter"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\cpu core\interpreter\cpu_int.cpp"
# End Source File
# Begin Source File

SOURCE=".\cpu core\interpreter\cpu_int.h"
# End Source File
# Begin Source File

SOURCE=".\cpu core\interpreter\cpu_int_opcodes.cpp"
# End Source File
# Begin Source File

SOURCE=".\cpu core\interpreter\cpu_int_opsgroup.h"
# End Source File
# End Group
# Begin Group "disassembler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\cpu core\disassembler\ppc_disasm.cpp"
# End Source File
# Begin Source File

SOURCE=".\cpu core\disassembler\ppc_disasm.h"
# End Source File
# End Group
# Begin Source File

SOURCE=".\cpu core\cpu_core.cpp"
# End Source File
# Begin Source File

SOURCE=".\cpu core\cpu_core.h"
# End Source File
# End Group
# Begin Source File

SOURCE=.\crc.cpp
# End Source File
# Begin Source File

SOURCE=.\crc.h
# End Source File
# Begin Source File

SOURCE=.\emu.cpp
# End Source File
# Begin Source File

SOURCE=.\emu.h
# End Source File
# Begin Source File

SOURCE=.\general.h
# End Source File
# Begin Source File

SOURCE=.\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\memory.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Target
# End Project
