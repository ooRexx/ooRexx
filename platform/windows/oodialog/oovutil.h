/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* Neither the name of Rexx Language Association nor the names                */
/* of its contributors may be used to endorse or promote products             */
/* derived from this software without specific prior written permission.      */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/******************************************************************************/
/* Object REXX OODialog                                             oovutil.h */
/*                                                                            */
/******************************************************************************/

#define MAXREXXNAME 128
#define MAXLENQUEUE 2056
#define NR_BUFFER 15
#define STR_BUFFER 256
#define LONGSTR_BUFFER 1024
#define DATA_BUFFER 4096*2      // was 256 before ES_MULTILINE
#define MAX_P 64
#define MAX_MT_ENTRIES  500
#define MAX_BT_ENTRIES  300
#define MAX_DT_ENTRIES  750
#define MAX_CT_ENTRIES 1000
#define MAXCHILDDIALOGS 20
#define CREATECHILD 0x0a01
#define INTERRUPTSCROLL 0x0a02
#define GETSETFOCUS 0x0a03
#define GETSETCAPTURE 0x0a04
#define GETKEYSTATE 0x0a05
#define MAXDIALOGS 20

#define VISDLL "OODIALOG.DLL"
#define DLLVER 2130

#define MSG_TERMINATE "1DLGDELETED1"

extern LONG HandleError(PRXSTRING r, CHAR * text);

/* macros to check the number of arguments */
#define CHECKARG(argexpct) { \
   if (argc != argexpct) \
      return HandleError(retstr, "Wrong number of arguments"); }


#define CHECKARGL(argexpct) { \
   if (argc < argexpct) \
      return HandleError(retstr, "Too few arguments"); }


#define CHECKARGLH(argexpctl, argexpcth) { \
   if (argc < argexpctl) return HandleError(retstr, "Too few arguments"); \
   if (argc > argexpcth) return HandleError(retstr, "Too many arguments"); }


/* macros for a easier return code */
#define RETC(retcode) { \
                   retstr->strlength = 1;\
                   if (retcode) retstr->strptr[0] = '1'; else retstr->strptr[0] = '0'; \
                   retstr->strptr[1] = '\0'; \
                   return 0; \
                }

#define RETERR  { \
                   retstr->strlength = 1;\
                   retstr->strptr[0] = '1'; \
                   retstr->strptr[1] = '\0'; \
                   return 40; \
                }


#define RETVAL(retvalue)  { \
                   ltoa(retvalue, retstr->strptr, 10); \
                   retstr->strlength = strlen(retstr->strptr);\
                   return 0; \
                }

/* macros for searching and checking the bitmap table */
#define SEARCHBMP(addr, ndx, id) \
   {                     \
      ndx = 0;\
      if (addr && addr->BmpTab)              \
      while ((ndx < addr->BT_size) && (addr->BmpTab[ndx].buttonID != (ULONG)id))\
         ndx++;                                                  \
   }

#define VALIDBMP(addr, ndx, id) \
   (addr && &addr->BmpTab[ndx] && (ndx < addr->BT_size) && (addr->BmpTab[ndx].buttonID == (ULONG)id))


/* macros for searching and checking the bitmap table */
#define SEARCHDATA(addr, ndx, id) \
   {                     \
      ndx = 0;              \
      if (addr && addr->DataTab)              \
      while ((ndx < addr->DT_size) && (addr->DataTab[ndx].id != (ULONG)id))\
         ndx++;                                                  \
   }

#define VALIDDATA(addr, ndx, id) \
   (addr && (ndx < addr->DT_size) && (addr->DataTab[ndx].id == (ULONG)id))


#define SEARCHBRUSH(addr, ndx, id, brush) \
   {                     \
      ndx = 0;\
      if (addr && addr->ColorTab) {              \
          while ((ndx < addr->CT_size) && (addr->ColorTab[ndx].itemID != (ULONG)id))\
             ndx++;                                                  \
          if (ndx < addr->CT_size) brush = addr->ColorTab[ndx].ColorBrush; \
      } \
   }


#define ISHEX(value) \
   ((value[0] == '0') && (toupper(value[1]) == 'X'))


//
// macros to access the fields in a BITMAPINFO struct
// field_value = macro(pBitmapInfo)
//

#define BI_WIDTH(pBI)       (int)((pBI)->bmiHeader.biWidth)
#define BI_HEIGHT(pBI)      (int)((pBI)->bmiHeader.biHeight)
#define BI_PLANES(pBI)      ((pBI)->bmiHeader.biPlanes)
#define BI_BITCOUNT(pBI)    ((pBI)->bmiHeader.biBitCount)

//
// macros to access BITMAPINFO fields in a DIB
// field_value = macro(pDIB)
//

#define DIB_WIDTH(pDIB)     (BI_WIDTH((LPBITMAPINFO)(pDIB)))
#define DIB_HEIGHT(pDIB)    (BI_HEIGHT((LPBITMAPINFO)(pDIB)))
#define DIB_PLANES(pDIB)    (BI_PLANES((LPBITMAPINFO)(pDIB)))
#define DIB_BITCOUNT(pDIB)  (BI_BITCOUNT((LPBITMAPINFO)(pDIB)))
//#define DIB_COLORS(pDIB)    (((LPBITMAPINFO)pDIB)->bmiHeader.biClrUsed)
#define DIB_COLORS(pDIB)    (NumDIBColorEntries((LPBITMAPINFO)(pDIB)))
#define DIB_BISIZE(pDIB)    (sizeof(BITMAPINFOHEADER) \
                            + DIB_COLORS(pDIB) * sizeof(RGBQUAD))
#define DIB_PBITS(pDIB)     (((LPSTR)((LPBITMAPINFO)(pDIB))) \
                            + DIB_BISIZE(pDIB))
#define DIB_PBI(pDIB)       ((LPBITMAPINFO)(pDIB))


#define SEEK_DLGADM_TABLE(hDlg, addressedTo) \
   if (topDlg && ((topDlg->TheDlg == hDlg) || (topDlg->AktChild == hDlg))) \
       addressedTo = topDlg; \
   else { register INT i=0; \
       while ((i<StoredDialogs) && (DialogTab[i]->TheDlg != hDlg) && (DialogTab[i]->AktChild != hDlg)) i++; \
       if (i<StoredDialogs) addressedTo = DialogTab[i]; else addressedTo = NULL;   } \

#define DEF_ADM     DIALOGADMIN * dlgAdm = NULL
#define GET_ADM     dlgAdm = (DIALOGADMIN *)atol(argv[0].strptr)



/* structures to manage the dialogs */
typedef struct {
   ULONG msg;
   ULONG filterM;
   ULONG wParam;
   ULONG filterP;
   ULONG lParam;
   ULONG filterL;
   PCHAR rexxProgram;
} MESSAGETABLEENTRY;

typedef struct {
   ULONG id;
   USHORT typ;
   USHORT category;
} DATATABLEENTRY;

typedef struct {
   ULONG buttonID;
   HBITMAP bitmapID;
   HBITMAP bmpFocusID;
   HBITMAP bmpSelectID;
   HBITMAP bmpDisableID;
   SHORT Frame;
   SHORT Loaded;
   SHORT displacex;
   SHORT displacey;
} BITMAPTABLEENTRY;


typedef struct {
   ULONG itemID;
   INT ColorBk;
   INT ColorFG;
   HBRUSH ColorBrush;
} COLORTABLEENTRY;

typedef struct
{
   void * previous;
   INT TableEntry;
   MESSAGETABLEENTRY * MsgTab;
   INT MT_size;
   DATATABLEENTRY * DataTab;
   INT DT_size;
   BITMAPTABLEENTRY * BmpTab;
   INT BT_size;
   COLORTABLEENTRY * ColorTab;
   INT CT_size;
   HWND TheDlg;
   HWND ChildDlg[MAXCHILDDIALOGS+1];
   HWND AktChild;
   HINSTANCE TheInstance;
   HANDLE TheThread;
   BOOL OnTheTop;
   ULONG LeaveDialog;
   BOOL Use3DControls;
   HBRUSH BkgBrush;
   HBITMAP BkgBitmap;
   HPALETTE ColorPalette;
   HMENU menu;
   HICON SysMenuIcon;
   WPARAM StopScroll;
   CHAR * pMessageQueue;
} DIALOGADMIN;



#ifdef EXTERNALFUNCS
typedef LONG APIENTRY GETITEMDATAEXTERNALFN (HANDLE, ULONG, UINT, PCHAR, ULONG);
typedef LONG APIENTRY SETITEMDATAEXTERNALFN (DIALOGADMIN *, HANDLE, ULONG, UINT, PCHAR);
typedef LONG APIENTRY GETSTEMDATAEXTERNALFN (HANDLE, ULONG, ULONG, PCHAR, ULONG);
typedef LONG APIENTRY SETSTEMDATAEXTERNALFN (DIALOGADMIN *, HANDLE, ULONG, ULONG, PCHAR);
#endif

#ifdef CREATEDLL
/* tools */
extern void rxstrlcpy(CHAR * tar, RXSTRING src);
extern void rxdatacpy(CHAR * tar, RXSTRING src);
extern BOOL IsYes(CHAR * s);

/* global variables */
#ifndef NOGLOBALVARIABLES
extern _declspec(dllexport) HINSTANCE MyInstance;
extern _declspec(dllexport) DIALOGADMIN * DialogTab[MAXDIALOGS];
extern _declspec(dllexport) DIALOGADMIN * topDlg;
extern _declspec(dllexport) INT StoredDialogs;
extern _declspec(dllexport) BOOL ReleaseMain;
#ifdef EXTERNALFUNCS
extern _declspec(dllexport) GETITEMDATAEXTERNALFN * GetItemDataExternal = NULL;
extern _declspec(dllexport) SETITEMDATAEXTERNALFN * SetItemDataExternal = NULL;
extern _declspec(dllexport) GETSTEMDATAEXTERNALFN * GetStemDataExternal = NULL;
extern _declspec(dllexport) SETSTEMDATAEXTERNALFN * SetStemDataExternal = NULL;
#endif

#endif

#else
extern _declspec(dllimport) HINSTANCE MyInstance;
extern _declspec(dllimport) DIALOGADMIN * DialogTab[MAXDIALOGS];
extern _declspec(dllimport) DIALOGADMIN * topDlg;
extern _declspec(dllimport) INT StoredDialogs;
extern _declspec(dllimport) BOOL ReleaseMain;
#ifdef EXTERNALFUNCS
extern _declspec(dllimport) GETITEMDATAEXTERNALFN * GetItemDataExternal;
extern _declspec(dllimport) SETITEMDATAEXTERNALFN * SetItemDataExternal;
extern _declspec(dllimport) GETSTEMDATAEXTERNALFN * GetStemDataExternal;
extern _declspec(dllimport) SETSTEMDATAEXTERNALFN * SetStemDataExternal;
#endif

#endif
