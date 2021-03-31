////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                        SAMPLE PLUGIN FOR OLLYDBG                           //
//                                                                            //
//                   Copyright (C) 2001-2004 Oleh Yuschuk                     //
//                                                                            //
// This plugin allows to set up to 10 code bookmarks using keyboard shortcuts //
// or popup menus in Disassembler and then quickly return to one of the       //
// bookmarks using shortcuts, popup menu or Bookmark window. Bookmarks        //
// are kept between sessions in .udd file.                                    //
//                                                                            //
// This code is distributed "as is", without warranty of any kind, expressed  //
// or implied, including, but not limited to warranty of fitness for any      //
// particular purpose. In no event will Oleh Yuschuk be liable to you for any //
// special, incidental, indirect, consequential or any other damages caused   //
// by the use, misuse, or the inability to use of this code, including any    //
// lost profits or lost savings, even if Oleh Yuschuk has been advised of the //
// possibility of such damages. Or, translated into English: use at your own  //
// risk!                                                                      //
//                                                                            //
// This code is free. You can modify this code, include parts of it into your //
// own programs and redistribute modified code provided that you remove all   //
// copyright messages or, if changes are significant enough, substitute them  //
// with your own copyright.                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// VERY IMPORTANT NOTICE: COMPILE THIS DLL WITH BYTE ALIGNMENT OF STRUCTURES
// AND UNSIGNED CHAR!

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "plugin.h"

HINSTANCE        hinst;                // DLL instance
HWND             hwmain;               // Handle of main OllyDbg window
char             bookmarkwinclass[32]; // Name of bookmark window class

// OllyDbg supports and makes extensive use of special kind of data collections
// called sorted tables. A table consists of descriptor (t_table) and data. All
// data elements has same size and begin with a 3-dword header: address, size
// and type. Table automatically sorts items by address, overlapping is not
// allowed. Our bookmark table consists of elements of type t_bookmark.
typedef struct t_bookmark {
  ulong          index;                // Bookmark index (0..9)
  ulong          size;                 // Size of index, always 1 in our case
  ulong          type;                 // Type of entry, always 0
  ulong          addr;                 // Address of bookmark
} t_bookmark;

t_table          bookmark;             // Bookmark table

// Functions in this file are placed in more or less "chronological" order,
// i.e. order in which they will be called by OllyDbg. This requires forward
// referencing.
int Bookmarksortfunc(t_bookmark *b1,t_bookmark *b2,int sort);
LRESULT CALLBACK Bookmarkwinproc(HWND hw,UINT msg,WPARAM wp,LPARAM lp);
int Bookmarkgettext(char *s,char *mask,int *select,t_sortheader *ph,int column);
void Createbookmarkwindow(void);


// Entry point into a plugin DLL. Many system calls require DLL instance
// which is passed to DllEntryPoint() as one of parameters. Remember it.
// Preferrable way is to place initializations into ODBG_Plugininit() and
// cleanup in ODBG_Plugindestroy().
BOOL WINAPI DllEntryPoint(HINSTANCE hi,DWORD reason,LPVOID reserved) {
  if (reason==DLL_PROCESS_ATTACH)
    hinst=hi;                          // Mark plugin instance
  return 1;                            // Report success
};

// ODBG_Plugindata() is a "must" for valid OllyDbg plugin. It must fill in
// plugin name and return version of plugin interface. If function is absent,
// or version is not compatible, plugin will be not installed. Short name
// identifies it in the Plugins menu. This name is max. 31 alphanumerical
// characters or spaces + terminating '\0' long. To keep life easy for users,
// this name should be descriptive and correlate with the name of DLL.
extc int _export cdecl ODBG_Plugindata(char shortname[32]) {
  strcpy(shortname,"Bookmarks");       // Name of plugin
  return PLUGIN_VERSION;
};

// OllyDbg calls this obligatory function once during startup. Place all
// one-time initializations here. If all resources are successfully allocated,
// function must return 0. On error, it must free partially allocated resources
// and return -1, in this case plugin will be removed. Parameter ollydbgversion
// is the version of OllyDbg, use it to assure that it is compatible with your
// plugin; hw is the handle of main OllyDbg window, keep it if necessary.
// Parameter features is reserved for future extentions, do not use it.
extc int _export cdecl ODBG_Plugininit(
  int ollydbgversion,HWND hw,ulong *features) {
  // This plugin uses all the newest features, check that version of OllyDbg is
  // correct. I will try to keep backward compatibility at least to v1.99.
  if (ollydbgversion<PLUGIN_VERSION)
    return -1;
  // Keep handle of main OllyDbg window. This handle is necessary, for example,
  // to display message box.
  hwmain=hw;
  // Initialize bookmark data. Data consists of elements of type t_bookmark,
  // we reserve space for 10 elements. If necessary, table will allocate more
  // space, but in our case maximal number of bookmarks is 10. Elements do not
  // allocate memory or other resources, so destructor is not necessary.
  if (Createsorteddata(&(bookmark.data),"Bookmarks",
    sizeof(t_bookmark),10,(SORTFUNC *)Bookmarksortfunc,NULL)!=0)
    return -1;                         // Unable to allocate bookmark data
  // Register window class for MDI window that will display plugins. Please
  // note that formally this class belongs to instance of main OllyDbg program,
  // not a plugin DLL. String bookmarkwinclass gets unique name of new class.
  // Keep it to create window and unregister on shutdown.
  if (Registerpluginclass(bookmarkwinclass,NULL,hinst,Bookmarkwinproc)<0) {
    // Failure! Destroy sorted data and exit.
    Destroysorteddata(&(bookmark.data));
    return -1; };
  // Plugin successfully initialized. Now is the best time to report this fact
  // to the log window. To conform OllyDbg look and feel, please use two lines.
  // The first, in black, should describe plugin, the second, gray and indented
  // by two characters, bears copyright notice.
  Addtolist(0,0,"Bookmarks sample plugin v1.10 (plugin demo)");
  Addtolist(0,-1,"  Copyright (C) 2001-2004 Oleh Yuschuk");
  // OllyDbg saves positions of plugin windows with attribute TABLE_SAVEPOS to
  // the .ini file but does not automatically restore them. Let us add this
  // functionality here. I keep information whether window was open when
  // OllyDbg terminated also in ollydbg.ini. This information is saved in
  // ODBG_Pluginclose. To conform to OllyDbg norms, window is restored only
  // if corresponding option is enabled.
  if (Plugingetvalue(VAL_RESTOREWINDOWPOS)!=0 &&
    Pluginreadintfromini(hinst,"Restore bookmarks window",0)!=0)
    Createbookmarkwindow();
  return 0;
};

// To sort sorted data by some criterium, one must supply sort function that
// returns -1 if first element is less than second, 1 if first element is
// greater and 0 if elements are equal according to criterium sort. Usually
// this criterium is the zero-based index of the column in window.
int Bookmarksortfunc(t_bookmark *b1,t_bookmark *b2,int sort) {
  int i=0;
  if (sort==1) {                       // Sort by address of bookmark
    if (b1->addr<b2->addr) i=-1;
    else if (b1->addr>b2->addr) i=1; };
  // If elements are equal or sorting is by the first column, sort by index.
  if (i==0) {
    if (b1->index<b2->index) i=-1;
    else if (b1->index>b2->index) i=1; };
  return i;
};

// Each window class needs its own window procedure. Both standard and custom
// OllyDbg windows must pass some system and OllyDbg-defined messages to
// Tablefunction(). See description of Tablefunction() for more details.
LRESULT CALLBACK Bookmarkwinproc(HWND hw,UINT msg,WPARAM wp,LPARAM lp) {
  int i,shiftkey,controlkey;
  HMENU menu;
  t_bookmark *pb;
  switch (msg) {
    // Standard messages. You can process them, but - unless absolutely sure -
    // always pass them to Tablefunction().
    case WM_DESTROY:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_HSCROLL:
    case WM_VSCROLL:
    case WM_TIMER:
    case WM_SYSKEYDOWN:
      Tablefunction(&bookmark,hw,msg,wp,lp);
      break;                           // Pass message to DefMDIChildProc()
    // Custom messages responsible for scrolling and selection. User-drawn
    // windows must process them, standard OllyDbg windows without extra
    // functionality pass them to Tablefunction().
    case WM_USER_SCR:
    case WM_USER_VABS:
    case WM_USER_VREL:
    case WM_USER_VBYTE:
    case WM_USER_STS:
    case WM_USER_CNTS:
    case WM_USER_CHGS:
      return Tablefunction(&bookmark,hw,msg,wp,lp);
    // WM_WINDOWPOSCHANGED is responsible for always-on-top MDI windows.
    case WM_WINDOWPOSCHANGED:
      return Tablefunction(&bookmark,hw,msg,wp,lp);
    case WM_USER_MENU:
      menu=CreatePopupMenu();
      // Find selected bookmark. Any operations with bookmarks make sense only
      // if at least one bookmark exists and is selected. Note that sorted data
      // has special sort index table which is updated only when necessary.
      // Getsortedbyselection() does this; some other sorted data functions
      // don't and you must call Sortsorteddata(). Read documentation!
      pb=(t_bookmark *)Getsortedbyselection(
        &(bookmark.data),bookmark.data.selected);
      if (menu!=NULL && pb!=NULL) {
        AppendMenu(menu,MF_STRING,1,"&Follow\tEnter");
        AppendMenu(menu,MF_STRING,2,"&Delete\tDel"); };
      // Even when menu is NULL, call to Tablefunction is still meaningful.
      i=Tablefunction(&bookmark,hw,WM_USER_MENU,0,(LPARAM)menu);
      if (menu!=NULL) DestroyMenu(menu);
      if (i==1)                        // Follow bookmark in Disassembler
        Setcpu(0,pb->addr,0,0,CPU_ASMHIST|CPU_ASMCENTER|CPU_ASMFOCUS);
      else if (i==2) {                 // Delete bookmark
        Deletesorteddata(&(bookmark.data),pb->index);
        // There is no automatical window update, do it yourself.
        InvalidateRect(hw,NULL,FALSE); };
      return 0;
    case WM_KEYDOWN:
      // Processing of WM_KEYDOWN messages is - surprise, surprise - very
      // similar to that of corresponding menu entries.
      shiftkey=GetKeyState(VK_SHIFT) & 0x8000;
      controlkey=GetKeyState(VK_CONTROL) & 0x8000;
      if (wp==VK_RETURN && shiftkey==0 && controlkey==0) {
        // Return key follows bookmark in Disassembler.
        pb=(t_bookmark *)Getsortedbyselection(
          &(bookmark.data),bookmark.data.selected);
        if (pb!=NULL)
          Setcpu(0,pb->addr,0,0,CPU_ASMHIST|CPU_ASMCENTER|CPU_ASMFOCUS);
        ; }
      else if (wp==VK_DELETE && shiftkey==0 && controlkey==0) {
        // DEL key deletes bookmark.
        pb=(t_bookmark *)Getsortedbyselection(
          &(bookmark.data),bookmark.data.selected);
        if (pb!=NULL) {
          Deletesorteddata(&(bookmark.data),pb->index);
          InvalidateRect(hw,NULL,FALSE);
        }; }
      else
        // Add all this arrow, home and pageup functionality.
        Tablefunction(&bookmark,hw,msg,wp,lp);
      break;
    case WM_USER_DBLCLK:
      // Doubleclicking row follows bookmark in Disassembler.
      pb=(t_bookmark *)Getsortedbyselection(
        &(bookmark.data),bookmark.data.selected);
      if (pb!=NULL)
        Setcpu(0,pb->addr,0,0,CPU_ASMHIST|CPU_ASMCENTER|CPU_ASMFOCUS);
      return 1;                        // Doubleclick processed
    case WM_USER_CHALL:
    case WM_USER_CHMEM:
      // Something is changed, redraw window.
      InvalidateRect(hw,NULL,FALSE);
      return 0;
    case WM_PAINT:
      // Painting of all OllyDbg windows is done by Painttable(). Make custom
      // drawing only if you have important reasons to do this.
      Painttable(hw,&bookmark,Bookmarkgettext);
      return 0;
    default: break;
  };
  return DefMDIChildProc(hw,msg,wp,lp);
};

// If you define ODBG_Pluginmainloop, this function will be called each time
// from the main Windows loop in OllyDbg. If there is some debug event from
// the debugged application, debugevent points to it, otherwise it is NULL. Do
// not declare this function unnecessarily, as this may negatively influence
// the overall speed!
extc void _export cdecl ODBG_Pluginmainloop(DEBUG_EVENT *debugevent) {
};

// Record types must be unique among OllyDbg and all plugins. The best way to
// assure this is to register record type by OllDbg (Oleh Yuschuk). Registration
// is absolutely free of charge, except for email costs :)
#define TAG_BOOKMARK   0x236D420AL     // Bookmark record type in .udd file

// Time to save data to .udd file! This is done by calling Pluginsaverecord()
// for each data item that must be saved. Global, process-oriented data must
// be saved in main .udd file (named by .exe); module-relevant data must be
// saved in module files. Don't forget to save all addresses relative to
// module's base, so that data will be restored correctly even when module is
// relocated.
extc void _export cdecl ODBG_Pluginsaveudd(t_module *pmod,int ismainmodule) {
  int i;
  ulong data[2];
  t_bookmark *pb;
  if (ismainmodule==0)
    return;                            // Save bookmarks to main file only
  pb=(t_bookmark *)bookmark.data.data;
  for (i=0; i<bookmark.data.n; i++,pb++) {
    data[0]=pb->index;
    data[1]=pb->addr;
    Pluginsaverecord(TAG_BOOKMARK,2*sizeof(ulong),data);
  };
};

// OllyDbg restores data from .udd file. If record belongs to plugin, it must
// process record and return 1, otherwise it must return 0 to pass record to
// other plugins. Note that module descriptor pointed to by pmod can be
// incomplete, i.e. does not necessarily contain all informations, especially
// that from .udd file.
extc int _export cdecl ODBG_Pluginuddrecord(t_module *pmod,int ismainmodule,
  ulong tag,ulong size,void *data) {
  t_bookmark mark;
  if (ismainmodule==0)
    return 0;                          // Bookmarks saved in main file only
  if (tag!=TAG_BOOKMARK)
    return 0;                          // Tag is not recognized
  mark.index=((ulong *)data)[0];
  mark.size=1;
  mark.type=0;
  mark.addr=((ulong *)data)[1];
  Addsorteddata(&(bookmark.data),&mark);
  return 1;                            // Record processed
};

// Function adds items either to main OllyDbg menu (origin=PM_MAIN) or to popup
// menu in one of standard OllyDbg windows. When plugin wants to add own menu
// items, it gathers menu pattern in data and returns 1, otherwise it must
// return 0. Except for static main menu, plugin must not add inactive items.
// Item indices must range in 0..63. Duplicated indices are explicitly allowed.
extc int _export cdecl ODBG_Pluginmenu(int origin,char data[4096],void *item) {
  int i,n;
  t_bookmark *pb;
  t_dump *pd;
  switch (origin) {
    // Menu creation is very simple. You just fill in data with menu pattern.
    // Some examples:
    // 0 Aaa,2 Bbb|3 Ccc|,,  - linear menu with 3items, relative IDs 0, 2 and 3,
    //                         separator between second and third item, last
    //                         separator and commas are ignored;
    // #A{0Aaa,B{1Bbb|2Ccc}} - unconditional separator, followed by popup menu
    //                         A with two elements, second is popup with two
    //                         elements and separator inbetween.
    case PM_MAIN:                      // Plugin menu in main window
      strcpy(data,"0 &Bookmarks|1 &About");
      // If your plugin is more than trivial, I also recommend to include Help.
      return 1;
    case PM_DISASM:                    // Popup menu in Disassembler
      // First check that menu applies.
      pd=(t_dump *)item;
      if (pd==NULL || pd->size==0)
        return 0;                      // Window empty, don't add
      // Start second-level popup menu.
      n=sprintf(data,"Bookmark{");
      // Add item "Insert bookmark n" if there are free bookmarks and some part
      // of Disassembler is selected. Note that OllyDbg correctly interpretes
      // superfluos commas, separators and, to some extent, missed braces.
      pb=(t_bookmark *)bookmark.data.data;
      for (i=0; i<bookmark.data.n; i++)
        if (pb[i].index!=(ulong)i) break;
      if (i<10 && pd->sel1>pd->sel0)
        n+=sprintf(data+n,"%i &Insert bookmark %i\tAlt+Shift+%i,",i,i,i);
      // Add item "Delete bookmark n" for each available bookmark. Menu
      // identifiers are not necessarily consecutive.
      for (i=0; i<bookmark.data.n; i++) {
        n+=sprintf(data+n,"%i Delete bookmark %i,",pb[i].index+10,pb[i].index);
      };
      // Add separator to menu.
      data[n++]='|';
      // Add item "Go to bookmark n" for each available bookmark. Bookmarks
      // set at selected command are not shown.
      for (i=0; i<bookmark.data.n; i++) {
        if (pb[i].addr==pd->sel0) continue;
        n+=sprintf(data+n,"%i Go to bookmark %i\tAlt+%i,",
          pb[i].index+20,pb[i].index,pb[i].index);
        ;
      };
      // Close popup. If you forget to do this, OllyDbg will try to correct
      // your error.
      sprintf(data+n,"}");
      return 1;
    default: break;                    // Any other window
  };
  return 0;                            // Window not supported by plugin
};

// This optional function receives commands from plugin menu in window of type
// origin. Argument action is menu identifier from ODBG_Pluginmenu(). If user
// activates automatically created entry in main menu, action is 0.
extc void _export cdecl ODBG_Pluginaction(int origin,int action,void *item) {
  t_bookmark mark,*pb;
  t_dump *pd;
  if (origin==PM_MAIN) {
    switch (action) {
      case 0:
        // Menu item "Bookmarks", creates bookmark window.
        Createbookmarkwindow();
        break;
      case 1:
        // Menu item "About", displays plugin info. If you write your own code,
        // please replace with own copyright!
        MessageBox(hwmain,
          "Bookmark plugin v1.10\n"
          "(demonstration of plugin capabilities)\n"
          "Copyright (C) 2001-2004 Oleh Yuschuk",
          "Bookmark plugin",MB_OK|MB_ICONINFORMATION);
        break;
      default: break;
    }; }
  else if (origin==PM_DISASM) {
    pd=(t_dump *)item;
    if (action>=0 && action<10) {      // Insert bookmark
      mark.index=action;
      mark.size=1;
      mark.type=0;
      mark.addr=pd->sel0;
      Addsorteddata(&(bookmark.data),&mark);
      if (bookmark.hw!=NULL) InvalidateRect(bookmark.hw,NULL,FALSE); }
    else if (action>=10 && action<20) {// Delete bookmark
      pb=(t_bookmark *)Findsorteddata(&(bookmark.data),action-10);
      if (pb!=NULL) {
        Deletesorteddata(&(bookmark.data),action-10);
        if (bookmark.hw!=NULL) InvalidateRect(bookmark.hw,NULL,FALSE);
      }; }
    else if (action>=20 && action<30) {//Go to bookmark
      pb=(t_bookmark *)Findsorteddata(&(bookmark.data),action-20);
      if (pb!=NULL) {
        Setcpu(0,pb->addr,0,0,CPU_ASMHIST|CPU_ASMCENTER|CPU_ASMFOCUS);
      };
    };
  };
};

// Standard function Painttable() makes most of OllyDbg windows redrawing. You
// only need to supply another function that prepares text strings and
// optionally colours them. Case of custom windows is a bit more complicated,
// please read documentation.
int Bookmarkgettext(char *s,char *mask,int *select,
  t_sortheader *ph,int column) {
  int n;
  ulong cmdsize,decodesize;
  char cmd[MAXCMDSIZE],*pdecode;
  t_memory *pmem;
  t_disasm da;
  t_bookmark *pb=(t_bookmark *)ph;
  if (column==0) {                     // Name of bookmark
    // Column 0 contains name of bookmark in form "Alt+n", where n is the
    // digit from 0 to 9. Mainly for demonstration purposes, I display prefix
    // "Alt+" in grayed and digit in normal text. Standard table windows do
    // not need to bother about selection.
    n=sprintf(s,"Alt+%i",pb->index);
    *select=DRAW_MASK;
    memset(mask,DRAW_GRAY,4);
    mask[4]=DRAW_NORMAL; }
  else if (column==1)                  // Address of bookmark
    n=sprintf(s,"%08X",pb->addr);
  else if (column==2) {                // Disassembled command
    // Function Disasm() requires that calling routine supplies code to be
    // disassembled. Read this code from memory. First determine possible
    // code size.
    pmem=Findmemory(pb->addr);         // Find memory block containing code
    if (pmem==NULL) {
      *select=DRAW_GRAY; return sprintf(s,"???"); };
    cmdsize=pmem->base+pmem->size-pb->addr;
    if (cmdsize>MAXCMDSIZE)
      cmdsize=MAXCMDSIZE;
    if (Readmemory(cmd,pb->addr,cmdsize,MM_RESTORE|MM_SILENT)!=cmdsize) {
      *select=DRAW_GRAY; return sprintf(s,"???"); };
    pdecode=Finddecode(pb->addr,&decodesize);
    if (decodesize<cmdsize) pdecode=NULL;
    Disasm(cmd,cmdsize,pb->addr,pdecode,&da,DISASM_CODE,0);
    strcpy(s,da.result);
    n=strlen(s); }
  else if (column==3)                  // Comment
    // Only user-defined comments are displayed here.
    n=Findname(pb->addr,NM_COMMENT,s);
  else n=0;                            // s is not necessarily 0-terminated
  return n;
};

// OllyDbg makes most of work when creating standard MDI window. Plugin must
// only describe number of columns, their properties and properties of window
// as a whole.
void Createbookmarkwindow(void) {
  // Describe table columns. Note that column names are pointers, so strings
  // must exist as long as table itself.
  if (bookmark.bar.nbar==0) {
    // Bar still uninitialized.
    bookmark.bar.name[0]="Bookmark";   // Name of bookmark
    bookmark.bar.defdx[0]=9;
    bookmark.bar.mode[0]=0;
    bookmark.bar.name[1]="Address";    // Bookmark address
    bookmark.bar.defdx[1]=9;
    bookmark.bar.mode[1]=0;
    bookmark.bar.name[2]="Disassembly";// Disassembled command
    bookmark.bar.defdx[2]=32;
    bookmark.bar.mode[2]=BAR_NOSORT;
    bookmark.bar.name[3]="Comment";    // Comment
    bookmark.bar.defdx[3]=256;
    bookmark.bar.mode[3]=BAR_NOSORT;
    bookmark.bar.nbar=4;
    bookmark.mode=
      TABLE_COPYMENU|TABLE_SORTMENU|TABLE_APPMENU|TABLE_SAVEPOS|TABLE_ONTOP;
    bookmark.drawfunc=Bookmarkgettext; };
  // If window already exists, Quicktablewindow() does not create new window,
  // but restores and brings to top existing. This is the simplest way,
  // Newtablewindow() is more flexible but more complicated. I do not recommend
  // custom (plugin-drawn) windows without very important reasons to do this.
  Quicktablewindow(&bookmark,15,4,bookmarkwinclass,"Bookmarks");
};

// This function receives possible keyboard shortcuts from standard OllyDbg
// windows. If it recognizes shortcut, it must process it and return 1,
// otherwise it returns 0.
extc int _export cdecl ODBG_Pluginshortcut(
  int origin,int ctrl,int alt,int shift,int key,void *item) {
  t_dump *pd;
  t_bookmark mark,*pm;
  // Plugin accepts shortcuts in form Alt+x or Shift+Alt+x, where x is a key
  // '0'..'9'. Shifted shortcut sets bookmark (only in Disassembler),
  // non-shifted jumps to bookmark from everywhere.
  if (ctrl==0 && alt!=0 && key>='0' && key<='9') {
    if (shift!=0 && origin==PM_DISASM && item!=NULL) {
      // Set new or replace existing bookmark.
      pd=(t_dump *)item;
      mark.index=key-'0';
      mark.size=1;
      mark.type=0;
      mark.addr=pd->sel0;
      Addsorteddata(&(bookmark.data),&mark);
      if (bookmark.hw!=NULL) InvalidateRect(bookmark.hw,NULL,FALSE);
      return 1; }                      // Shortcut recognized
    else if (shift==0) {
      // Jump to existing bookmark (from any window).
      pm=Findsorteddata(&(bookmark.data),key-'0');
      if (pm==NULL)
        Flash("Undefined bookmark");
      else
        Setcpu(0,pm->addr,0,0,CPU_ASMHIST|CPU_ASMCENTER|CPU_ASMFOCUS);
      return 1;                        // Shortcut recognized
    };
  };
  return 0;                            // Shortcut not recognized
};

// Function is called when user opens new or restarts current application.
// Plugin should reset internal variables and data structures to initial state.
extc void _export cdecl ODBG_Pluginreset(void) {
  Deletesorteddatarange(&(bookmark.data),0,0xFFFFFFFF);
};

// OllyDbg calls this optional function when user wants to terminate OllyDbg.
// All MDI windows created by plugins still exist. Function must return 0 if
// it is safe to terminate. Any non-zero return will stop closing sequence. Do
// not misuse this possibility! Always inform user about the reasons why
// termination is not good and ask for his decision!
extc int _export cdecl ODBG_Pluginclose(void) {
  // For automatical restoring of open windows, mark in .ini file whether
  // Bookmarks window is still open.
  Pluginwriteinttoini(hinst,"Restore bookmarks window",bookmark.hw!=NULL);
  return 0;
};

// OllyDbg calls this optional function once on exit. At this moment, all MDI
// windows created by plugin are already destroyed (and received WM_DESTROY
// messages). Function must free all internally allocated resources, like
// window classes, files, memory and so on.
extc void _export cdecl ODBG_Plugindestroy(void) {
  Unregisterpluginclass(bookmarkwinclass);
  Destroysorteddata(&(bookmark.data));
};

