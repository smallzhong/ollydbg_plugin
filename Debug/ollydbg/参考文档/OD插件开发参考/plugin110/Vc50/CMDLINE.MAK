
OUTDIR=.
INTDIR=.
INCLUDE=c:\vc\include
LIBPATH=c:\vc\lib

ALL : "$(OUTDIR)\cmdline.dll"

CLEAN :
	-@erase "$(INTDIR)\Cmdexec.obj"
	-@erase "$(INTDIR)\Command.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\cmdline.dll"
	-@erase "$(OUTDIR)\cmdline.exp"
	-@erase "$(OUTDIR)\cmdline.lib"
	-@erase "$(OUTDIR)\cmdline.pdb"

CPP=cl.exe
CPP_PROJ=/nologo /Zp1 /ML /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\cmdline.pch" /I"$(INCLUDE)" /YX /J /FD /c 
CPP_OBJS=.
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cmdline.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\cmdline.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)\cmdline.dll" /implib:"$(OUTDIR)\cmdline.lib"\
 /libpath:"$(LIBPATH)"
LINK32_OBJS= \
	"$(INTDIR)\Cmdexec.obj" \
	"$(INTDIR)\Command.obj" \
	".\Ollydbg.lib"

"$(OUTDIR)\cmdline.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


SOURCE=.\Cmdexec.c
DEP_CPP_CMDEX=\
	".\plugin.h"\
	

"$(INTDIR)\Cmdexec.obj" : $(SOURCE) $(DEP_CPP_CMDEX) "$(INTDIR)"


SOURCE=.\Command.c
DEP_CPP_COMMA=\
	".\plugin.h"\
	

"$(INTDIR)\Command.obj" : $(SOURCE) $(DEP_CPP_COMMA) "$(INTDIR)"
