# Microsoft Developer Studio Project File - Name="24term" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** 編集しないでください **

# TARGTYPE "Win32 (WCE MIPS) Application" 0x0a01
# TARGTYPE "Win32 (WCE SH) Application" 0x0901

CFG=24term - Win32 (WCE MIPS) Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "24term.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "24term.mak" CFG="24term - Win32 (WCE MIPS) Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "24term - Win32 (WCE MIPS) Release" ("Win32 (WCE MIPS) Application" 用)
!MESSAGE "24term - Win32 (WCE MIPS) Debug" ("Win32 (WCE MIPS) Application" 用)
!MESSAGE "24term - Win32 (WCE SH) Release" ("Win32 (WCE SH) Application" 用)
!MESSAGE "24term - Win32 (WCE SH) Debug" ("Win32 (WCE SH) Application" 用)
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
# PROP WCE_Configuration "H/PC Ver. 2.00"

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WMIPSRel"
# PROP BASE Intermediate_Dir "WMIPSRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WMIPSRel"
# PROP Intermediate_Dir "WMIPSRel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /ML /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /YX /QMRWCE /c
# ADD CPP /nologo /ML /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /YX"windows.h" /QMRWCE /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /NOLOGO
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /machine:MIPS /subsystem:$(CESubsystem)
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib winsock.lib /nologo /machine:MIPS /subsystem:$(CESubsystem)
# SUBTRACT LINK32 /pdb:none /map /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WMIPSDbg"
# PROP BASE Intermediate_Dir "WMIPSDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WMIPSDbg"
# PROP Intermediate_Dir "WMIPSDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /MLd /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /YX /QMRWCE /c
# ADD CPP /nologo /MLd /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /FR /YX"windows.h" /QMRWCE /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /NOLOGO
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /debug /machine:MIPS /subsystem:$(CESubsystem)
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib winsock.lib /nologo /debug /machine:MIPS /subsystem:$(CESubsystem)
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WCESHRel"
# PROP BASE Intermediate_Dir "WCESHRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WCESHRel"
# PROP Intermediate_Dir "WCESHRel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /ML /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /YX /c
# ADD CPP /nologo /ML /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /YX"windows.h" /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /NOLOGO
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /machine:SH3 /subsystem:$(CESubsystem)
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib winsock.lib /nologo /machine:SH3 /subsystem:$(CESubsystem)
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCESHDbg"
# PROP BASE Intermediate_Dir "WCESHDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCESHDbg"
# PROP Intermediate_Dir "WCESHDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /MLd /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /YX /c
# ADD CPP /nologo /MLd /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /FR /YX"windows.h" /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x411 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /NOLOGO
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /debug /machine:SH3 /subsystem:$(CESubsystem)
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib winsock.lib /nologo /debug /machine:SH3 /subsystem:$(CESubsystem)
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ENDIF 

# Begin Target

# Name "24term - Win32 (WCE MIPS) Release"
# Name "24term - Win32 (WCE MIPS) Debug"
# Name "24term - Win32 (WCE SH) Release"
# Name "24term - Win32 (WCE SH) Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\24term.rc

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bold.cpp

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

DEP_CPP_BOLD_=\
	".\common.h"\
	".\draw.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

DEP_CPP_BOLD_=\
	".\common.h"\
	".\draw.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

DEP_CPP_BOLD_=\
	".\common.h"\
	".\draw.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\config.cpp

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

DEP_CPP_CONFI=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\draw8.h"\
	".\main.h"\
	".\pic16.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

DEP_CPP_CONFI=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\draw8.h"\
	".\main.h"\
	".\pic16.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

DEP_CPP_CONFI=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\draw8.h"\
	".\main.h"\
	".\pic16.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

DEP_CPP_CONFI=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\draw8.h"\
	".\main.h"\
	".\pic16.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\draw.cpp

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

DEP_CPP_DRAW_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

DEP_CPP_DRAW_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

DEP_CPP_DRAW_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

DEP_CPP_DRAW_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\screen.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\draw16.cpp

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

DEP_CPP_DRAW1=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

DEP_CPP_DRAW1=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

DEP_CPP_DRAW1=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

DEP_CPP_DRAW1=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\screen.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\draw2.cpp

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

DEP_CPP_DRAW2=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw2.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

DEP_CPP_DRAW2=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw2.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

DEP_CPP_DRAW2=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw2.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

DEP_CPP_DRAW2=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw2.h"\
	".\screen.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\draw8.cpp

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

DEP_CPP_DRAW8=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

DEP_CPP_DRAW8=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

DEP_CPP_DRAW8=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

DEP_CPP_DRAW8=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw8.h"\
	".\screen.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\main.cpp

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

DEP_CPP_MAIN_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\draw2.h"\
	".\draw8.h"\
	".\main.h"\
	".\pic16.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

DEP_CPP_MAIN_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\draw2.h"\
	".\draw8.h"\
	".\main.h"\
	".\pic16.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

DEP_CPP_MAIN_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\draw2.h"\
	".\draw8.h"\
	".\main.h"\
	".\pic16.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

DEP_CPP_MAIN_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\draw2.h"\
	".\draw8.h"\
	".\main.h"\
	".\pic16.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pic16.cpp

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

DEP_CPP_PIC16=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\pic16.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

DEP_CPP_PIC16=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\pic16.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

DEP_CPP_PIC16=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\pic16.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

DEP_CPP_PIC16=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw16.h"\
	".\pic16.h"\
	".\screen.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pic8.cpp

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

DEP_CPP_PIC8_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw8.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

DEP_CPP_PIC8_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw8.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

DEP_CPP_PIC8_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw8.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

DEP_CPP_PIC8_=\
	".\common.h"\
	".\config.h"\
	".\draw.h"\
	".\draw8.h"\
	".\pic8.h"\
	".\screen.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\screen.cpp

!IF  "$(CFG)" == "24term - Win32 (WCE MIPS) Release"

DEP_CPP_SCREE=\
	".\common.h"\
	".\config.h"\
	".\screen.h"\
	{$(INCLUDE)}"af_irda.h"\
	{$(INCLUDE)}"sslsock.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE MIPS) Debug"

DEP_CPP_SCREE=\
	".\common.h"\
	".\config.h"\
	".\screen.h"\
	
NODEP_CPP_SCREE=\
	".\f_irda.h"\
	".\slsock.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Release"

DEP_CPP_SCREE=\
	".\common.h"\
	".\config.h"\
	".\screen.h"\
	
NODEP_CPP_SCREE=\
	".\f_irda.h"\
	".\slsock.h"\
	

!ELSEIF  "$(CFG)" == "24term - Win32 (WCE SH) Debug"

DEP_CPP_SCREE=\
	".\common.h"\
	".\config.h"\
	".\screen.h"\
	
NODEP_CPP_SCREE=\
	".\f_irda.h"\
	

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\draw.h
# End Source File
# Begin Source File

SOURCE=.\draw16.h
# End Source File
# Begin Source File

SOURCE=.\draw2.h
# End Source File
# Begin Source File

SOURCE=.\draw8.h
# End Source File
# Begin Source File

SOURCE=.\main.h
# End Source File
# Begin Source File

SOURCE=.\newres.h
# End Source File
# Begin Source File

SOURCE=.\pic16.h
# End Source File
# Begin Source File

SOURCE=.\pic8.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\screen.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\arrow.ico
# End Source File
# Begin Source File

SOURCE=.\config.ico
# End Source File
# Begin Source File

SOURCE=.\gammalh.ico
# End Source File
# Begin Source File

SOURCE=.\gammam.ico
# End Source File
# Begin Source File

SOURCE=.\main.ico
# End Source File
# Begin Source File

SOURCE=.\menu.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\source.txt
# End Source File
# End Target
# End Project
