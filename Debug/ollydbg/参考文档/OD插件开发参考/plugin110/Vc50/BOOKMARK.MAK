
OUTDIR=.
INTDIR=.
INCLUDE=c:\vc\include
LIBPATH=c:\vc\lib

ALL : "$(OUTDIR)\bookmark.dll"

CLEAN :
	-@erase "$(INTDIR)\Bookmark.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\bookmark.dll"
	-@erase "$(OUTDIR)\bookmark.exp"
	-@erase "$(OUTDIR)\bookmark.lib"
	-@erase "$(OUTDIR)\bookmark.pdb"

CPP=cl.exe
CPP_PROJ=/nologo /Zp1 /ML /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\bookmark.pch" /I"$(INCLUDE)" /YX /J /FD /c 
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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\bookmark.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\bookmark.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)\bookmark.dll" /implib:"$(OUTDIR)\bookmark.lib"\
 /libpath:"$(LIBPATH)" 
LINK32_OBJS= \
	"$(INTDIR)\Bookmark.obj" \
	".\ollydbg.lib"

"$(OUTDIR)\bookmark.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE=.\Bookmark.c
DEP_CPP_BOOKM=\
	".\plugin.h"\
	

"$(INTDIR)\Bookmark.obj" : $(SOURCE) $(DEP_CPP_BOOKM) "$(INTDIR)"

