/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
#define NTDDI_VERSION   NTDDI_WINXPSP2
#define _WIN32_WINNT    0x0501
#define WINVER          0x0501

#define STRICT
#define OEMRESOURCE

#include <windows.h>
#include <mmsystem.h>
#include "oorexxapi.h"
#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include <errno.h>
#include <shlwapi.h>
#include <commctrl.h>
#include "oovutil.h"

// Map strings representing constant defines to their int values.  For
// translating things like "IDI_APPLICATION" from the user to the proper API
// value.
#include <string>
#include <map>
using namespace std;
typedef map<string, int, less<string> > String2Int;

#define FILENAME_BUFFER_LEN 65535

extern LONG SetRexxStem(const char * name, INT id, const char * secname, const char * data);
WORD NumDIBColorEntries(LPBITMAPINFO lpBmpInfo);
extern LPBITMAPINFO LoadDIB(const char *szFile);
extern LONG EvaluateListStyle(const char * styledesc);
extern BOOL AddDialogMessage(CHAR *, CHAR *);
extern LONG setKeyPressData(KEYPRESSDATA *, CONSTRXSTRING, CONSTRXSTRING, const char *);
extern UINT seekKeyPressMethod(KEYPRESSDATA *, const char *);
extern void removeKeyPressMethod(KEYPRESSDATA *, UINT);
extern void processKeyPress(KEYPRESSDATA *, WPARAM, LPARAM, PCHAR);
extern void freeKeyPressData(KEYPRESSDATA *);

/* Local functions */
static ULONG SetStyle(HWND, LONG, PRXSTRING);
static void freeSubclassData(SUBCLASSDATA *);
static BOOL removeKeyPressSubclass(SUBCLASSDATA *, HWND, INT);

bool screenToDlgUnit(HWND hwnd, POINT *point);
void screenToDlgUnit(HDC hdc, POINT *point);

/* Enum for the type of a dialog control. Types to be added as needed. */
typedef enum {oodcStatic, oodcButton, oodcEdit, oodcProgressBar,} oodControl_t;


/**
 * Defines and structs for the DlgUtil class.
 */
#define DLGUTILCLASS                 ".DlgUtil"
#define COMCTL_ERR_TITLE             "ooDialog - Windows Common Controls Error"
#define GENERIC_ERR_TITLE            "ooDialog - Error"
#define DLLGETVERSION_FUNCTION       "DllGetVersion"
#define COMMON_CONTROL_DLL           "comctl32.dll"

extern DWORD ComCtl32Version = 0;


/**
 * Defines and structs for Button controls: .ButtonControl, .GroupBox, etc..
 */
#define BUTTONCONTROLCLASS   ".ButtonControl"
#define RADIOBUTTONCLASS     ".RadioButton"
#define CHECKBOXCLASS        ".CheckBox"
#define GROUPBOXCLASS        ".GroupBox"
#define ANIMATEDBUTTONCLASS  ".AnimatedButton"

#define BC_SETSTYLE_OPTS     "PUSHBOX, DEFPUSHBUTTON, CHECKBOX, AUTOCHECKBOX, 3STATE, AUTO3STATE, "        \
                             "RADIO, AUTORADIO, GROUPBOX, OWNERDRAW, LEFTTEXT, RIGHTBUTTON, NOTLEFTTEXT, " \
                             "TEXT, ICON, BITMAP, LEFT, RIGHT, HCENTER, TOP, BOTTOM, VCENTER, PUSHLIKE, "  \
                             "NOTPUSHLIKE, MULTILINE, NOTMULTILINE, NOTIFY, NOTNOTIFY, FLAT, NOTFLAT"

#define BC_SETSTATE_OPTS     "CHECKED, UNCHECKED, INDETERMINATE, FOCUS, PUSH, NOTPUSHED"
#define BS_IMAGEMASK         0x000000c0
#define MIN_HALFHEIGHT_GB    12

typedef enum {push, check, radio, group, owner, notButton} BUTTONTYPE, *PBUTTONTYPE;
typedef enum {def, autoCheck, threeState, autoThreeState, noSubtype } BUTTONSUBTYPE, *PBUTTONSUBTYPE;

/**
 * Defines, structs, etc., for the .ImageList class.
 */

#define IMAGELISTCLASS             ".ImageList"


// ImageList helper functions.
HIMAGELIST rxGetImageList(RexxMethodContext *, RexxObjectPtr, int);
RexxObjectPtr rxNewImageList(RexxMethodContext *, HIMAGELIST);

#define IL_DEFAULT_FLAGS           ILC_COLOR32 | ILC_MASK
#define IL_DEFAULT_COUNT           6
#define IL_DEFAULT_GROW            0


/**
 * Defines, structs, etc., for the .Image class.
 */

#define IMAGECLASS                 ".Image"


// Helper functions.
CSTRING getImageTypeName(uint8_t);
RexxObjectPtr rxNewImageFromControl(RexxMethodContext *, HWND, HANDLE, uint8_t, oodControl_t);
RexxObjectPtr rxNewEmptyImage(RexxMethodContext *, DWORD);
RexxObjectPtr rxNewValidImage(RexxMethodContext *, HANDLE, uint8_t, PSIZE, uint32_t, bool);

#define IMAGE_TYPE_LIST            "Bitmap, Icon, Cursor, Enhanced Metafile"

typedef struct _OODIMAGE
{
    SIZE     size;
    HANDLE   hImage;
    LONG     type;
    DWORD    flags;
    DWORD    lastError;
    CSTRING  typeName;
    CSTRING  fileName;   // Not currently used, may change to char[256].

    bool     srcOOD;     // True - comes from ooDialog code using LoadImage(),
                         // False comes from a raw retrieved handle.
    bool     canRelease;
    bool     isValid;
} OODIMAGE, *POODIMAGE;

POODIMAGE rxGetOodImage(RexxMethodContext *, RexxObjectPtr, int);
POODIMAGE rxGetImageIcon(RexxMethodContext *, RexxObjectPtr, int);
POODIMAGE rxGetImageBitmap(RexxMethodContext *, RexxObjectPtr, int);


/**
 * Defines and structs for the .ResourceImage class.
 */
#define RESOURCEIMAGECLASS  ".ResourceImage"

typedef struct _RESOURCEIMAGE
{
    HMODULE  hMod;
    DWORD    lastError;
    bool     canRelease;
    bool     isValid;
} RESOURCEIMAGE, *PRESOURCEIMAGE;


/**
 * Defines and structs for the .ProgressBar class.
 */
#define PROGRESSBARCLASS  ".ProgressBar"


/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry PlaySoundFile(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   UINT opts;

   CHECKARGL(1);

   if ((argc > 1) && (IsYes(argv[1].strptr)))
      opts = SND_ASYNC;
   else
      opts = SND_SYNC;

   if (sndPlaySound(argv[0].strptr,opts | SND_NODEFAULT))
      RETC(0)
   else
      RETC(1)
}

/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry PlaySoundFileInLoop(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   UINT opts;

   CHECKARG(1);

   opts = SND_ASYNC;

   if (sndPlaySound(argv[0].strptr,opts|SND_LOOP | SND_NODEFAULT))
      RETC(0)
   else
      RETC(1)
}

/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry StopSoundFile(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   UINT opts;

   opts = SND_SYNC;

   if (sndPlaySound(NULL,opts | SND_NODEFAULT))
      RETC(0)
   else
      RETC(1)
}


UINT_PTR CALLBACK  OFNSetForegroundHookProc(
    HWND hdlg,    // handle to child dialog window
    UINT uiMsg,    // message identifier
    WPARAM wParam,    // message parameter
    LPARAM lParam)    // message parameter
{
    if (uiMsg == WM_INITDIALOG)
    {
        HWND h = GetParent(hdlg);
        if (!h) h = hdlg;
        SetForegroundWindow(h);
    }
    return 0;   /* 0 means default routine handles message */
}


BOOL OpenFileDlg( BOOL load, PCHAR szFile, const char *szInitialDir, const char *szFilter, HWND hw, const char *title, const char *DefExt, BOOL multi, CHAR chSepChar) /* @DOE005M */
{
   OPENFILENAME OpenFileName;
   BOOL         fRc;

   OpenFileName.lStructSize       = sizeof(OPENFILENAME);
   OpenFileName.hwndOwner         = hw;
   OpenFileName.hInstance         = 0;
   OpenFileName.lpstrFilter       = szFilter;
   OpenFileName.lpstrCustomFilter = (LPSTR) NULL;
   OpenFileName.nMaxCustFilter    = 0L;
   OpenFileName.nFilterIndex      = 1L;
   OpenFileName.lpstrFile         = szFile;
   OpenFileName.nMaxFile          = FILENAME_BUFFER_LEN;
   OpenFileName.lpstrFileTitle    = NULL; /* we don't need the selected file */
   OpenFileName.nMaxFileTitle     = 0;    /* we don't need the selected file */
   OpenFileName.lpstrInitialDir   = szInitialDir;
   OpenFileName.lpstrTitle        = title;
   OpenFileName.nFileOffset       = 0;
   OpenFileName.nFileExtension    = 0;
   OpenFileName.lpstrDefExt       = DefExt;
   OpenFileName.lCustData         = 0;
   OpenFileName.lpfnHook          = OFNSetForegroundHookProc;   /* hook to set dialog to foreground */

   /* The OFN_EXPLORER flag is required when using OFN_ENABLEHOOK, otherwise the dialog is old style and does not change directory */
   OpenFileName.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_EXPLORER;   /* enable hook */

   if (load && multi) OpenFileName.Flags |= OFN_ALLOWMULTISELECT;

   if (load)
   {
       OpenFileName.Flags |= OFN_FILEMUSTEXIST;
       fRc = GetOpenFileName(&OpenFileName);

       if (fRc && multi)
       {
         /* OFN_EXPLORER returns the selected name separated with ASCII 0 instead of spaces */
         PCHAR pChr = szFile;

         while( (*pChr != 0) || (*(pChr+1) != 0))
         {
           if (*pChr == 0)
             *pChr =  chSepChar;
           pChr++;
         }
       }

       return fRc;
   }
   else
   {
       OpenFileName.Flags |= OFN_OVERWRITEPROMPT;
       return GetSaveFileName(&OpenFileName);
   }
}



#define VALIDARG(argn) (argc >= argn) && argv[argn-1].strptr && argv[argn-1].strptr[0]

/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry GetFileNameWindow(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    BOOL    fSuccess;
    const char *  title;
    const char *defext = "TXT";
    BOOL    load = TRUE;
    BOOL    multi = FALSE;
    HWND    hWnd;
    const char *szFilter = "Text Files (*.TXT)\0*.TXT\0All Files (*.*)\0*.*\0";
    PCHAR   pszFiles = NULL;
    PCHAR   pszInitialDir = NULL;
    CHAR    chSepChar = ' ';  /* default separation character  /              */
                              /* allow to change separation character to      */
                              /* handle filenames with blank character        */

    pszFiles = (char *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, FILENAME_BUFFER_LEN);
    if (!pszFiles)
        RETERR

    if (VALIDARG(1))
    {
        if ( argv[0].strptr[argv[0].strlength - 1] == '\\' )
        {
            pszInitialDir = (char *)LocalAlloc(LPTR, _MAX_PATH);
            if ( !pszInitialDir )
              RETERR
            rxstrlcpy(pszInitialDir, argv[0]);
        }
        else
        {
          rxstrlcpy(pszFiles, argv[0]);
        }
    }
    if (VALIDARG(2)) hWnd = GET_HWND(argv[1]); else hWnd = NULL;
    if (VALIDARG(3)) szFilter= argv[2].strptr;
    if (VALIDARG(4)) load = (argv[3].strptr[0] != '0');
    if (VALIDARG(5)) title = argv[4].strptr;
    else {
        if (load) title = "Open a File";
        else title = "Save File As";
    }
    if ((argc >= 6) && (argv[5].strptr)) defext = argv[5].strptr;
    if (VALIDARG(7)) multi = IsYes(argv[6].strptr);

    if (VALIDARG(8)) chSepChar =  argv[7].strptr[0];

    retstr->strlength = 0;
    fSuccess = OpenFileDlg(load, pszFiles, pszInitialDir, szFilter, hWnd, title,
                           defext, multi, chSepChar);

    if ( pszInitialDir )
        LocalFree(pszInitialDir);
    if ( fSuccess )
    {
        /* we simply use the allocated string as return code and let REXX free it */
        retstr->strptr = pszFiles;
        retstr->strlength = strlen(pszFiles);
        return 0;
    }

    if (CommDlgExtendedError())
        RETERR
    else
        RETC(0);
}


size_t RexxEntry PlaySnd(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   UINT opts;
   DEF_ADM;

   CHECKARGL(2);

   GET_ADM;

   if (!dlgAdm) RETERR

   if ((argc > 2) && (IsYes(argv[2].strptr)))
      opts = SND_ASYNC;
   else
      opts = SND_SYNC;

   if (PlaySound(MAKEINTRESOURCE(atoi(argv[1].strptr)),dlgAdm->TheInstance, SND_RESOURCE | opts | SND_NODEFAULT))
      RETC(0)
   else
      RETC(1)
}

/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry SleepMS(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   CHECKARG(1);

   Sleep(atoi(argv[0].strptr));
   RETC(0)
}

/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry WinTimer(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    UINT_PTR timerID;
    MSG msg;

    CHECKARG(2);
    if ( !stricmp(argv[0].strptr, "START") )
    {
        timerID = SetTimer(NULL, 1001, atoi(argv[1].strptr), NULL);
        RETPTR(timerID)
    }
    else if ( !stricmp(argv[0].strptr, "STOP") )
    {
        timerID = (UINT_PTR)GET_POINTER(argv[1]);
        if ( KillTimer(NULL, timerID) == 0 )
        {
            RETVAL(GetLastError())
        }
        RETC(0)
    }
    else if ( !stricmp(argv[0].strptr, "WAIT") )
    {
        timerID = (UINT_PTR)GET_POINTER(argv[1]);
        while ( !PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE) || (msg.wParam != timerID) )
        {
            ; // do nothing
        }
        RETC(0)
    }
    RETC(1)
}


HIMAGELIST CreateImageList(INT start, HWND h, CONSTRXSTRING *argv, size_t argc)
{
   HBITMAP hBmp = NULL;
   HIMAGELIST iL;
   INT cx,cy, nr;
   BITMAP bmpInfo;

   if (atol(argv[start].strptr) > 0)
       hBmp = (HBITMAP)GET_HANDLE(argv[start]);
   else {
       LPBITMAPINFO lpBit = LoadDIB(argv[start].strptr);
       if (lpBit)
       {
           HDC dc;

           dc = GetDC(h);
           hBmp = CreateDIBitmap(dc,    // handle to device context
                (BITMAPINFOHEADER*)lpBit,
                CBM_INIT,
                DIB_PBITS(lpBit),        // bits
                DIB_PBI(lpBit),          // BITMAPINFO
                DIB_RGB_COLORS);
           ReleaseDC(h, dc);
           LocalFree((void *)lpBit);
       }
   }
   if (!hBmp) return NULL;

   cx = atoi(argv[start+1].strptr);
   cy = atoi(argv[start+2].strptr);

   GetObject(hBmp, sizeof(BITMAP), &bmpInfo);

   if (!cx) cx = bmpInfo.bmHeight;  /* height is correct! */
   if (!cy) cy = bmpInfo.bmHeight;
   nr = bmpInfo.bmWidth / cx;

   iL = ImageList_Create( cx, cy, ILC_COLOR8, nr, 0);

   if (ImageList_Add(iL, hBmp, NULL) == -1) {
       ImageList_Destroy( iL);
       return NULL;
   }
   DeleteObject(hBmp);
   return iL;
}

/**
 * This is the window procedure used to subclass the edit control for both the
 * ListControl and TreeControl objects.  It would be nice to convert this to use
 * the better API: SetWindowSubclass / RemoveWindowSubclass.
 */
WNDPROC wpOldEditProc = NULL;

LONG_PTR CALLBACK CatchReturnSubProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
        case WM_GETDLGCODE:
            return (DLGC_WANTALLKEYS | CallWindowProc(wpOldEditProc, hWnd, uMsg, wParam, lParam));

        case WM_CHAR:
             //Process this message to avoid message beeps.
            if ((wParam == VK_RETURN) || (wParam == VK_ESCAPE))
                return 0;
            else
                return CallWindowProc(wpOldEditProc, hWnd,uMsg, wParam, lParam);

        default:
            return CallWindowProc(wpOldEditProc, hWnd, uMsg, wParam, lParam);
            break;

    } /* end switch */
}


/**
 * Subclass procedure for any dialog control.  Reports key press events to
 * ooDialog for those key presses connected to an ooDialog method by the user.
 *
 * All messages are passed on unchanged to the control.
 *
 * processKeyPress() is used to actually decipher the key press data and set
 * up the ooDialog method invocation.  That function documents what is sent on
 * to the ooDialog method.
 */
LRESULT CALLBACK KeyPressSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
  LPARAM lParam, UINT_PTR id, DWORD_PTR dwData)
{
    KEYPRESSDATA *pKeyData;
    SUBCLASSDATA *pData = (SUBCLASSDATA *)dwData;
    if ( ! pData ) return DefSubclassProc(hwnd, msg, wParam, lParam);

    pKeyData = pData->pKeyPressData;

    switch ( msg )
    {
        case WM_GETDLGCODE:
            /* Don't do anything for now. This message has some interesting
             * uses, perhaps a future enhancement.
             */
            break;

        case WM_SYSKEYDOWN :
            /* Sent when the alt key is down.  We need both WM_SYSKEYDOWN and
             * WM_KEYDOWN to catch everything that a keyboard hook catches.
             */
            if (  pKeyData->key[wParam] && !(lParam & KEY_REALEASE) && !(lParam & KEY_WASDOWN) )
                processKeyPress(pKeyData, wParam, lParam, pData->pMessageQueue);
            break;

        case WM_KEYDOWN:
            /* WM_KEYDOWN will never have KEY_RELEASE set. */
            if (  pKeyData->key[wParam] && !(lParam & KEY_WASDOWN) )
                processKeyPress(pKeyData, wParam, lParam, pData->pMessageQueue);
            break;

        case WM_NCDESTROY:
            /* The window is being destroyed, remove the subclass, clean up
             * memory.
             */
            RemoveWindowSubclass(hwnd, KeyPressSubclassProc, id);
            if ( pData )
                freeSubclassData(pData);
            break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

/**
 * If there is subclass data, free it.
 */
static void freeSubclassData(SUBCLASSDATA * pData)
{
    if ( pData )
    {
        freeKeyPressData(pData->pKeyPressData);
        LocalFree((void *)pData);
    }
}

/**
 * Convenience function to remove the key press subclass procedure and free the
 * associated memory.
 *
 */
static BOOL removeKeyPressSubclass(SUBCLASSDATA *pData, HWND hDlg, INT id)
{
    BOOL success = SendMessage(hDlg, WM_USER_SUBCLASS_REMOVE, (WPARAM)&KeyPressSubclassProc, (LPARAM)id) != 0;
    if ( success ) freeSubclassData(pData);
    return success;
}


size_t RexxEntry HandleTreeCtrl(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND h;

   CHECKARGL(2);

   h = GET_HWND(argv[1]);
   if (!h) RETERR;

   if (!strcmp(argv[0].strptr, "INS"))
   {
       TV_INSERTSTRUCT ins;
       TV_ITEM * tvi = &ins.item;

       CHECKARG(9);
       ins.hParent = (HTREEITEM)GET_HANDLE(argv[2]);
       if (!ins.hParent && !strcmp(argv[2].strptr,"ROOT")) ins.hParent = TVI_ROOT;
       ins.hInsertAfter = (HTREEITEM)GET_HANDLE(argv[3]);
       if (!ins.hInsertAfter)
       {
           if (!strcmp(argv[3].strptr,"FIRST")) ins.hInsertAfter = TVI_FIRST;
           else if (!strcmp(argv[3].strptr,"SORT")) ins.hInsertAfter = TVI_SORT;
           else ins.hInsertAfter = TVI_LAST;
       }

       tvi->mask = TVIF_TEXT;

       tvi->pszText = (LPSTR)argv[4].strptr;
       tvi->cchTextMax = (int)argv[4].strlength;

       tvi->iImage = atoi(argv[5].strptr);
       if (tvi->iImage >= 0) tvi->mask |= TVIF_IMAGE;

       tvi->state= 0;

       if (strstr(argv[6].strptr,"BOLD")) tvi->state |= TVIS_BOLD;
       if (strstr(argv[6].strptr,"EXPANDED")) tvi->state |= TVIS_EXPANDED;
       tvi->stateMask = tvi->state;

       if (tvi->state != 0) tvi->mask |= TVIF_STATE;

       tvi->cChildren = atoi(argv[7].strptr);
       if (tvi->cChildren > 0) tvi->mask |= TVIF_CHILDREN;

       tvi->iSelectedImage = atoi(argv[8].strptr);
       if (tvi->iSelectedImage > -1) tvi->mask |= TVIF_SELECTEDIMAGE;

       RETHANDLE(TreeView_InsertItem(h, &ins));
   }
   else
   if (!strcmp(argv[0].strptr, "DEL"))
   {
       HTREEITEM hItem;

       CHECKARG(3);
       hItem = (HTREEITEM)GET_HANDLE(argv[2]);
       if (!hItem && !strcmp(argv[2].strptr,"ROOT"))
          RETC(!TreeView_DeleteAllItems(h))
       else if (hItem)
       {
           if (TreeView_GetCount(h) >0)
               RETC(!TreeView_DeleteItem(h, hItem))
           else RETVAL(-1)
       }
   }
   else
   if (!strcmp(argv[0].strptr, "SET"))
   {
       TV_ITEM tvi;

       CHECKARG(8);

       tvi.hItem = (HTREEITEM)GET_HANDLE(argv[2]);

       // tvi.mask = TVIF_HANDLE;
       tvi.mask = 0;

       if (argv[3].strlength)
       {
           tvi.pszText = (LPSTR)argv[3].strptr;
           tvi.cchTextMax = (int)argv[3].strlength;
           tvi.mask |= TVIF_TEXT;
       }

       tvi.iImage = atoi(argv[4].strptr);
       if (tvi.iImage >= 0) tvi.mask |= TVIF_IMAGE;

       tvi.state= 0;
       tvi.stateMask= 0;

       if (strstr(argv[5].strptr,"NOTBOLD")) tvi.stateMask |= TVIS_BOLD;
       else if (strstr(argv[5].strptr,"BOLD")) {tvi.state |= TVIS_BOLD; tvi.stateMask |= TVIS_BOLD;}
       if (strstr(argv[5].strptr,"NOTDROP")) tvi.stateMask |= TVIS_DROPHILITED;
       else if (strstr(argv[5].strptr,"DROP")) {tvi.state |= TVIS_DROPHILITED; tvi.stateMask |= TVIS_DROPHILITED;}
       if (strstr(argv[5].strptr,"NOTSELECTED")) tvi.stateMask |= TVIS_SELECTED;
       else if (strstr(argv[5].strptr,"SELECTED")) {tvi.state |= TVIS_SELECTED; tvi.stateMask |= TVIS_SELECTED;}
       if (strstr(argv[5].strptr,"NOTCUT")) tvi.stateMask |= TVIS_CUT;
       else if (strstr(argv[5].strptr,"CUT")) {tvi.state |= TVIS_CUT; tvi.stateMask |= TVIS_CUT;}
       if (strstr(argv[5].strptr,"NOTEXPANDEDONCE")) tvi.stateMask |= TVIS_EXPANDEDONCE;
       else if (strstr(argv[5].strptr,"EXPANDEDONCE")) {tvi.state |= TVIS_EXPANDEDONCE; tvi.stateMask |= TVIS_EXPANDEDONCE;}
       else if (strstr(argv[5].strptr,"NOTEXPANDED")) tvi.stateMask |= TVIS_EXPANDED;
       else if (strstr(argv[5].strptr,"EXPANDED")) {tvi.state |= TVIS_EXPANDED; tvi.stateMask |= TVIS_EXPANDED;}
       if ((tvi.state != 0) || (tvi.stateMask!= 0)) tvi.mask |= TVIF_STATE;

       tvi.cChildren = atoi(argv[6].strptr);
       if (tvi.cChildren > -1) tvi.mask |= TVIF_CHILDREN;

       tvi.iSelectedImage = atoi(argv[7].strptr);
       if (tvi.iSelectedImage > -1) tvi.mask |= TVIF_SELECTEDIMAGE;

       RETVAL(TreeView_SetItem(h, &tvi));
   }
   else
   if (!strcmp(argv[0].strptr, "GET"))
   {
       TV_ITEM tvi;
       CHAR data[256];

       CHECKARG(4);

       tvi.hItem = (HTREEITEM)GET_HANDLE(argv[2]);
       tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_CHILDREN | TVIF_SELECTEDIMAGE;
       tvi.pszText = data;
       tvi.cchTextMax = 255;
       tvi.stateMask = TVIS_EXPANDED | TVIS_BOLD | TVIS_SELECTED | TVIS_EXPANDEDONCE | TVIS_DROPHILITED | TVIS_CUT;

       if (TreeView_GetItem(h, &tvi))
       {
           SetRexxStem(argv[3].strptr, -1, "!Text", tvi.pszText);
           itoa(tvi.cChildren, data, 10);
           SetRexxStem(argv[3].strptr, -1, "!Children", data);
           itoa(tvi.iImage, data, 10);
           SetRexxStem(argv[3].strptr, -1, "!Image", data);
           itoa(tvi.iSelectedImage, data, 10);
           SetRexxStem(argv[3].strptr, -1, "!SelectedImage", data);
           data[0] = '\0';
           if (tvi.state & TVIS_EXPANDED) strcat(data, "EXPANDED ");
           if (tvi.state & TVIS_BOLD) strcat(data, "BOLD ");
           if (tvi.state & TVIS_SELECTED) strcat(data, "SELECTED ");
           if (tvi.state & TVIS_EXPANDEDONCE) strcat(data, "EXPANDEDONCE ");
           if (tvi.state & TVIS_DROPHILITED) strcat(data, "INDROP ");
           if (tvi.state & TVIS_CUT) strcat(data, "CUT ");
           SetRexxStem(argv[3].strptr, -1, "!State", data);
           RETC(0)
       }
       else RETVAL(-1)
   }
   else
   if (!strcmp(argv[0].strptr, "GETHND"))
   {
       HTREEITEM hItem;
       ULONG flag;

       CHECKARG(4);

       hItem = (HTREEITEM)GET_HANDLE(argv[2]);

       if (!strcmp(argv[3].strptr,"CARET")) flag = TVGN_CARET;
       else if (!strcmp(argv[3].strptr,"CHILD")) flag = TVGN_CHILD;
       else if (!strcmp(argv[3].strptr,"DROP")) flag = TVGN_DROPHILITE;
       else if (!strcmp(argv[3].strptr,"FIRSTVISIBLE")) flag = TVGN_FIRSTVISIBLE;
       else if (!strcmp(argv[3].strptr,"NEXT")) flag = TVGN_NEXT;
       else if (!strcmp(argv[3].strptr,"NEXTVISIBLE")) flag = TVGN_NEXTVISIBLE;
       else if (!strcmp(argv[3].strptr,"PARENT")) flag = TVGN_PARENT;
       else if (!strcmp(argv[3].strptr,"PREVIOUS")) flag = TVGN_PREVIOUS;
       else if (!strcmp(argv[3].strptr,"PREVIOUSVISIBLE")) flag = TVGN_PREVIOUSVISIBLE;
       else if (!strcmp(argv[3].strptr,"ROOT")) flag = TVGN_ROOT;
       RETHANDLE(TreeView_GetNextItem(h, hItem, flag))
   }
   else
   if (!strcmp(argv[0].strptr, "CNT"))
   {
       RETVAL(TreeView_GetCount(h))
   }
   else
   if (!strcmp(argv[0].strptr, "CNTVIS"))
   {
       RETVAL(TreeView_GetVisibleCount(h))
   }
   else
   if (!strcmp(argv[0].strptr, "SEL"))
   {
       HTREEITEM hItem;
       ULONG flag;

       CHECKARG(4);

       hItem = (HTREEITEM)GET_HANDLE(argv[2]);

       if (!strcmp(argv[3].strptr,"DROP")) flag = TVGN_DROPHILITE;
       else if (!strcmp(argv[3].strptr,"FIRSTVIS")) flag = TVGN_FIRSTVISIBLE;
       else flag = TVGN_CARET;
       RETC(!TreeView_Select(h, hItem, flag))
   }
   if (!strcmp(argv[0].strptr, "EXPAND"))
   {
       HTREEITEM hItem;
       ULONG flag = 0;

       CHECKARG(4);

       hItem = (HTREEITEM)GET_HANDLE(argv[2]);

       if (!strcmp(argv[3].strptr,"EXPAND")) flag = TVE_EXPAND;
       else if (!strcmp(argv[3].strptr,"TOGGLE")) flag = TVE_TOGGLE;
       else {
           if (strstr(argv[3].strptr,"COLLAPSE")) flag |= TVE_COLLAPSE;
           if (strstr(argv[3].strptr,"RESET")) flag |= TVE_COLLAPSERESET;
       }
       RETC(!TreeView_Expand(h, hItem, flag))
   }
   else
   if (!strcmp(argv[0].strptr, "ENVIS"))
   {
       HTREEITEM hItem;

       CHECKARG(3);

       hItem = (HTREEITEM)GET_HANDLE(argv[2]);

       RETC(!TreeView_EnsureVisible(h, hItem))
   }
   else
   if (!strcmp(argv[0].strptr, "GETIND"))
   {
       RETVAL(TreeView_GetIndent(h))
   }
   else
   if (!strcmp(argv[0].strptr, "SETIND"))
   {
       CHECKARG(3);

       TreeView_SetIndent(h, atoi(argv[2].strptr));
       RETC(0)
   }
   else
   if (!strcmp(argv[0].strptr, "EDIT"))
   {
       CHECKARG(3);

       HTREEITEM hItem = (HTREEITEM)GET_HANDLE(argv[2]);
       RETHANDLE(TreeView_EditLabel(h, (HTREEITEM)hItem));
   }
   else
   if (!strcmp(argv[0].strptr, "EEDIT"))
   {
       CHECKARG(3);

       RETC(!TreeView_EndEditLabelNow(h, IsYes(argv[2].strptr)))
   }
   else
   if (!strcmp(argv[0].strptr, "SORT"))
   {
       CHECKARG(4);

       HTREEITEM hItem = (HTREEITEM)GET_HANDLE(argv[2]);
       RETC(!TreeView_SortChildren(h, (HTREEITEM)hItem, IsYes(argv[3].strptr)))
   }
   else
   if (!strcmp(argv[0].strptr, "SETIMG"))
   {
       HIMAGELIST iL;

       CHECKARG(5);
       iL = CreateImageList(2, h, argv, argc);

       if (iL) RETHANDLE(TreeView_SetImageList(h, iL, TVSIL_NORMAL))
       else RETC(0)
   }
   else
   if (!strcmp(argv[0].strptr, "UNSETIMG"))
   {
       HIMAGELIST iL;

       iL = TreeView_GetImageList(h, TVSIL_NORMAL);
       if (!iL) RETC(1)
       TreeView_SetImageList(h, 0, TVSIL_NORMAL);

       RETC(!ImageList_Destroy( iL))
   }
   else
   if (!strcmp(argv[0].strptr, "SUBCL_EDIT"))
   {
       HWND ew = TreeView_GetEditControl(h);
       if (ew)
       {
           WNDPROC oldProc = (WNDPROC)setWindowPtr(ew, GWLP_WNDPROC, (LONG_PTR)CatchReturnSubProc);
           if (oldProc != (WNDPROC)CatchReturnSubProc) wpOldEditProc = oldProc;
           RETPTR(oldProc)
       }
       else RETC(0)
   }
   else
   if (!strcmp(argv[0].strptr, "RESUB_EDIT"))
   {
       HWND ew = TreeView_GetEditControl(h);
       if (ew)
       {
           setWindowPtr((HWND)ew, GWLP_WNDPROC, (LONG_PTR)wpOldEditProc);
           RETC(0)
       }
       RETVAL(-1)
   }
   else
   if (!strcmp(argv[0].strptr, "HIT"))
   {
       TV_HITTESTINFO hti;
       HTREEITEM hItem;

       CHECKARG(4);
       hti.pt.x = atol(argv[2].strptr);
       hti.pt.y = atol(argv[3].strptr);
       hItem = TreeView_HitTest(h, &hti);
       if (hItem)
       {
           pointer2string(retstr, (void *)hItem);
           if (hti.flags & TVHT_ABOVE) strcat(retstr->strptr, " ABOVE");
           if (hti.flags & TVHT_BELOW) strcat(retstr->strptr, " BELOW");
           if (hti.flags & TVHT_NOWHERE) strcat(retstr->strptr, " NOWHERE");
           if (hti.flags & TVHT_ONITEM) strcat(retstr->strptr, " ONITEM");
           if (hti.flags & TVHT_ONITEMBUTTON) strcat(retstr->strptr, " ONBUTTON");
           if (hti.flags & TVHT_ONITEMICON) strcat(retstr->strptr, " ONICON");
           if (hti.flags & TVHT_ONITEMINDENT) strcat(retstr->strptr, " ONINDENT");
           if (hti.flags & TVHT_ONITEMLABEL) strcat(retstr->strptr, " ONLABEL");
           if (hti.flags & TVHT_ONITEMRIGHT) strcat(retstr->strptr, " ONRIGHT");
           if (hti.flags & TVHT_ONITEMSTATEICON) strcat(retstr->strptr, " ONSTATEICON");
           if (hti.flags & TVHT_TOLEFT) strcat(retstr->strptr, " TOLEFT");
           if (hti.flags & TVHT_TORIGHT) strcat(retstr->strptr, " TORIGHT");
           retstr->strlength = strlen(retstr->strptr);
           return 0;
       }
       RETC(0)
   }
   RETC(0)
}

/**
 * Set a window style and check for error.
 *
 * @param hwnd    Handle of window having its style changed / set.
 * @param lStyle  The stlye to be set.
 * @param retstr  The string being returned to ooRexx.
 *
 * @return The string returned to ooRexx is the previous window style on
 *         success, or the negated system error on failure.
 *
 *         The function itself always returns 0.
 */
static ULONG SetStyle(HWND hwnd, LONG lStyle, RXSTRING *retstr)
{
    LONG lErr;

    SetLastError(0);
    lStyle = SetWindowLong(hwnd, GWL_STYLE, lStyle);

    /* SetWindowLong returns 0 on error, or the value of the previous long at
     * the specified index.  Very unlikely that the last style was 0, but assume
     * it is possible.  In that case, 0 is only an error if GetLastError does
     * not return 0.
     */
    if ( ! lStyle )
    {
        lErr = (LONG)GetLastError();
        if ( ! lErr ) lStyle = -lErr;
        ltoa(lStyle, retstr->strptr, 10);
        retstr->strlength = strlen(retstr->strptr);
    }
    else
    {
        ultoa((ULONG)lStyle, retstr->strptr, 10);
        retstr->strlength = strlen(retstr->strptr);
    }
    return 0;
}

/**
 * Take an edit control's window flags and construct a Rexx string that
 * represents the control's style.
 */
ULONG EditStyleToString(LONG lStyle, RXSTRING *retstr)
{
    if ( lStyle & WS_VISIBLE ) strcpy(retstr->strptr, "VISIBLE");
    else strcpy(retstr->strptr, "HIDDEN");

    if ( lStyle & WS_TABSTOP ) strcat(retstr->strptr, " TAB");
    else strcat(retstr->strptr, " NOTAB");

    if ( lStyle & WS_DISABLED ) strcat(retstr->strptr, " DISABLED");
    else strcat(retstr->strptr, " ENABLED");

    if ( lStyle & WS_GROUP )       strcat(retstr->strptr, " GROUP");
    if ( lStyle & WS_HSCROLL )     strcat(retstr->strptr, " HSCROLL");
    if ( lStyle & WS_VSCROLL )     strcat(retstr->strptr, " VSCROLL");
    if ( lStyle & ES_PASSWORD )    strcat(retstr->strptr, " PASSWORD");
    if ( lStyle & ES_MULTILINE )   strcat(retstr->strptr, " MULTILINE");
    if ( lStyle & ES_AUTOHSCROLL ) strcat(retstr->strptr, " AUTOSCROLLH");
    if ( lStyle & ES_AUTOVSCROLL ) strcat(retstr->strptr, " AUTOSCROLLV");
    if ( lStyle & ES_READONLY )    strcat(retstr->strptr, " READONLY");
    if ( lStyle & ES_WANTRETURN )  strcat(retstr->strptr, " WANTRETURN");
    if ( lStyle & ES_NOHIDESEL )   strcat(retstr->strptr, " KEEPSELECTION");
    if ( lStyle & ES_UPPERCASE )   strcat(retstr->strptr, " UPPER");
    if ( lStyle & ES_LOWERCASE )   strcat(retstr->strptr, " LOWER");
    if ( lStyle & ES_NUMBER )      strcat(retstr->strptr, " NUMBER");
    if ( lStyle & ES_OEMCONVERT )  strcat(retstr->strptr, " OEM");

    if ( lStyle & ES_RIGHT ) strcat(retstr->strptr, " RIGHT");
    else if ( lStyle & ES_CENTER ) strcat(retstr->strptr, " CENTER");
    else strcat(retstr->strptr, " LEFT");

    retstr->strlength = strlen(retstr->strptr);
    return 0;
}

/**
 * Parse an edit control style string sent from ooDialog into the corresponding
 * style flags.
 *
 * Note that this is meant to only deal with the styles that can be changed
 * after the control is created through SetWindowLong.
 */
LONG ParseEditStyle(const char * style)
{
    LONG lStyle = 0;

    if (strstr(style, "UPPER"     )) lStyle |= ES_UPPERCASE;
    if (strstr(style, "LOWER"     )) lStyle |= ES_LOWERCASE;
    if (strstr(style, "NUMBER"    )) lStyle |= ES_NUMBER;
    if (strstr(style, "WANTRETURN")) lStyle |= ES_WANTRETURN;
    if (strstr(style, "OEM"       )) lStyle |= ES_OEMCONVERT;

    /* Although these styles can be changed by individual ooDialog methods, as
     * a convenience, allow the programmer to include them when changing
     * multiple styles at once.
     */
    if (strstr(style, "TABSTOP" )) lStyle |= WS_TABSTOP;
    if (strstr(style, "GROUP"   )) lStyle |= WS_GROUP;

    return lStyle;
}

/**
 * Parse a list-view control extended style string sent from ooDialog into the
 * corresponding style flags.
 *
 * The extended list-view styles are set (and retrieved) in a different manner
 * than other window styles.  This function is used only to parse those extended
 * styles.  The normal list-view styles are parsed using EvaluateListStyle.
 */
DWORD ParseExtendedListStyle(const char * style)
{
    DWORD dwStyle = 0;

    if ( strstr(style, "BORDERSELECT"    ) ) dwStyle |= LVS_EX_BORDERSELECT;
    if ( strstr(style, "CHECKBOXES"      ) ) dwStyle |= LVS_EX_CHECKBOXES;
    if ( strstr(style, "FLATSB"          ) ) dwStyle |= LVS_EX_FLATSB;
    if ( strstr(style, "FULLROWSELECT"   ) ) dwStyle |= LVS_EX_FULLROWSELECT;
    if ( strstr(style, "GRIDLINES"       ) ) dwStyle |= LVS_EX_GRIDLINES;
    if ( strstr(style, "HEADERDRAGDROP"  ) ) dwStyle |= LVS_EX_HEADERDRAGDROP;
    if ( strstr(style, "INFOTIP"         ) ) dwStyle |= LVS_EX_INFOTIP;
    if ( strstr(style, "MULTIWORKAREAS"  ) ) dwStyle |= LVS_EX_MULTIWORKAREAS;
    if ( strstr(style, "ONECLICKACTIVATE") ) dwStyle |= LVS_EX_ONECLICKACTIVATE;
    if ( strstr(style, "REGIONAL"        ) ) dwStyle |= LVS_EX_REGIONAL;
    if ( strstr(style, "SUBITEMIMAGES"   ) ) dwStyle |= LVS_EX_SUBITEMIMAGES;
    if ( strstr(style, "TRACKSELECT"     ) ) dwStyle |= LVS_EX_TRACKSELECT;
    if ( strstr(style, "TWOCLICKACTIVATE") ) dwStyle |= LVS_EX_TWOCLICKACTIVATE;
    if ( strstr(style, "UNDERLINECOLD"   ) ) dwStyle |= LVS_EX_UNDERLINECOLD;
    if ( strstr(style, "UNDERLINEHOT"    ) ) dwStyle |= LVS_EX_UNDERLINEHOT;

    // Needs Comctl32.dll version 5.8 or higher
    if ( ComCtl32Version >= COMCTL32_5_8 )
    {
      if ( strstr(style, "LABELTIP") ) dwStyle |= LVS_EX_LABELTIP;
    }

    // Needs Comctl32 version 6.0 or higher
    if ( ComCtl32Version >= COMCTL32_6_0 )
    {
      if ( strstr(style, "DOUBLEBUFFER") ) dwStyle |= LVS_EX_DOUBLEBUFFER;
      if ( strstr(style, "SIMPLESELECT") ) dwStyle |= LVS_EX_SIMPLESELECT;
    }
    return dwStyle;
}


/**
 * Produce a string representation of a List-View's extended styles.
 */
DWORD ListExtendedStyleToString(HWND hList, RXSTRING *retstr)
{
    DWORD dwStyle = ListView_GetExtendedListViewStyle(hList);
    retstr->strptr[0] = '\0';

    if ( dwStyle & LVS_EX_BORDERSELECT )     strcat(retstr->strptr, "BORDERSELECT ");
    if ( dwStyle & LVS_EX_CHECKBOXES )       strcat(retstr->strptr, "CHECKBOXES ");
    if ( dwStyle & LVS_EX_FLATSB )           strcat(retstr->strptr, "FLATSB ");
    if ( dwStyle & LVS_EX_FULLROWSELECT )    strcat(retstr->strptr, "FULLROWSELECT ");
    if ( dwStyle & LVS_EX_GRIDLINES )        strcat(retstr->strptr, "GRIDLINES ");
    if ( dwStyle & LVS_EX_HEADERDRAGDROP )   strcat(retstr->strptr, "HEADERDRAGDROP ");
    if ( dwStyle & LVS_EX_INFOTIP )          strcat(retstr->strptr, "INFOTIP ");
    if ( dwStyle & LVS_EX_MULTIWORKAREAS )   strcat(retstr->strptr, "MULTIWORKAREAS ");
    if ( dwStyle & LVS_EX_ONECLICKACTIVATE ) strcat(retstr->strptr, "ONECLICKACTIVATE ");
    if ( dwStyle & LVS_EX_REGIONAL )         strcat(retstr->strptr, "REGIONAL ");
    if ( dwStyle & LVS_EX_SUBITEMIMAGES )    strcat(retstr->strptr, "SUBITEMIMAGES ");
    if ( dwStyle & LVS_EX_TRACKSELECT )      strcat(retstr->strptr, "TRACKSELECT ");
    if ( dwStyle & LVS_EX_TWOCLICKACTIVATE ) strcat(retstr->strptr, "TWOCLICKACTIVATE ");
    if ( dwStyle & LVS_EX_UNDERLINECOLD )    strcat(retstr->strptr, "UNDERLINECOLD ");
    if ( dwStyle & LVS_EX_UNDERLINEHOT )     strcat(retstr->strptr, "UNDERLINEHOT ");
    if ( dwStyle & LVS_EX_LABELTIP )         strcat(retstr->strptr, "LABELTIP ");
    if ( dwStyle & LVS_EX_DOUBLEBUFFER )     strcat(retstr->strptr, "DOUBLEBUFFER ");
    if ( dwStyle & LVS_EX_SIMPLESELECT )     strcat(retstr->strptr, "SIMPLESELECT ");

    if ( retstr->strptr[0] != '\0' ) retstr->strptr[strlen(retstr->strptr) - 1] = '\0';

    retstr->strlength = strlen(retstr->strptr);
    return 0;
}

/**
 * Determine if a window belongs to the specified window class.
 */
static bool checkWindowClass(HWND hwnd, TCHAR *pClass)
{
    TCHAR buf[64];

    if ( ! RealGetWindowClass(hwnd, buf, sizeof(buf)) || strcmp(buf, pClass) )
        return false;
    return true;
}

/**
 * Extended Common Control functionality.  This function implements capabilities
 * for the common controls that were not available at the time of the original
 * IBM ooDialog, or were available but not put into ooDialog.
 *
 * For sending many messages to dialog controls, the SendWinMsg() function is
 * usually adequate.  This function is used when special processing needs to
 * take place to construct the message parameters, or for Windows API calls that
 * do not involve SendWindowMessage.
 *
 * The parameters sent from ooRexx as an array of RXString:
 *
 * argv[0]  Dialog handle.
 *
 * argv[1]  Control ID.
 *
 * argv[2]  In general, the control type, i.e. "EDIT", "STATIC", etc..  However,
 *          also used for some generic functions.
 *
 * argv[3]  Sub-function qualifier.
 *
 * argv[4] ... arv[n]  Varies depending on the function.
 *
 * Return to ooRexx, in general:
 *  < -4 a negated system error code
 *    -4 unsupported ComCtl32 Version
 *    -3 problem with an argument
 *    -2 problem with the dialog handle
 *    -1 problem with the dialog control (id or handle)
 *     0 the Windows API call succeeds
 *     1 the Windows API call failed
 *  >  1 dependent on the function, usually a returned value not a return code
 */
size_t RexxEntry HandleControlEx(
    const char *funcname,
    size_t argc,
    CONSTRXSTRING *argv,
    const char *qname,
    RXSTRING *retstr)
{
    HWND hDlg;
    HWND hCtrl;
    int id;

    /* Minimum of 4 args. */
    CHECKARGL(4);

    hDlg = GET_HWND(argv[0]);
    if ( hDlg == 0 || ! IsWindow(hDlg) ) RETVAL(-2)

    id = atoi(argv[1].strptr);
    if ( id == 0 ) RETVAL(-1)

    hCtrl = (HWND)GetDlgItem(hDlg, id);
    if ( ! hCtrl ) RETVAL(-(LONG)GetLastError())

    /* Determine the control, or other function.  The single first letter is
     * checked.
     */
    if ( argv[2].strptr[0] == 'E' )      /* Edit control function */
    {
        if ( ! checkWindowClass(hCtrl, WC_EDIT) )
            RETVAL(-1)

        if ( strcmp(argv[3].strptr, "TXT") == 0 )       /* Set or get the control's text. */
        {
            if ( argc > 4 )
            {
                if ( SetWindowText(hCtrl, argv[4].strptr) == 0 )
                    RETVAL(-(LONG)GetLastError())
                else
                    RETVAL(0)
            }
            else
            {
                ULONG count = (ULONG)GetWindowTextLength(hCtrl);

                if ( count == 0 )
                {
                    retstr->strptr[0] = '\0';
                    retstr->strlength = 0;
                }
                else
                {
                    if ( ++count > RXAUTOBUFLEN )
                    {
                        PVOID p = GlobalAlloc(GMEM_FIXED, count);
                        if ( ! p )
                        {
                            RETVAL(-(LONG)GetLastError())
                        }

                        retstr->strptr = (PCHAR)p;
                    }
                    count = GetWindowText(hCtrl, (LPTSTR)retstr->strptr, count);

                    retstr->strlength = count;
                    if ( count == 0 )
                    {
                        retstr->strptr[0] = '\0';
                    }
                }
            }
            return 0;
        }
        else if ( !strcmp(argv[3].strptr, "MSG") ) /* Send an edit message (EM_*) */
        {
            CHECKARGL(5);

            if ( !strcmp(argv[4].strptr, "SEL" ) )       /* Get selection character positions */
            {
                DWORD start, end;

                SendMessage(hCtrl, EM_GETSEL, (WPARAM)(LPDWORD)&start, (LPARAM)(LPDWORD)&end);

                sprintf(retstr->strptr, "%u %u", start + 1, end + 1);
                retstr->strlength = strlen(retstr->strptr);
                return 0;
            }
            else if ( !strcmp(argv[4].strptr, "TIP" ) )  /* Show or hide ballon tip */
            {
                /* Requires XP Common Controls version 6.0 */
                if ( ComCtl32Version < COMCTL32_6_0 ) RETVAL(-4)

                if ( argc == 5 )  /* EM_HIDEBALLONTIP */
                {
                    if ( Edit_HideBalloonTip(hCtrl) )
                        RETVAL(0)
                    else
                        RETVAL(1)
                }
                else if ( argc >= 7 )  /* EM_SHOWBALLONTIP */
                {
                    EDITBALLOONTIP tip;
                    WCHAR wszTitle[128];
                    WCHAR wszText[1024];

                    /* The title string has a limit of 99 characters. */
                    if ( argv[5].strlength > 99 || argv[6].strlength > 1023 )
                        RETVAL(-3)

                    if ( MultiByteToWideChar(CP_ACP, 0, argv[5].strptr, -1, wszTitle, 128) == 0 )
                        RETVAL(-3)

                    if ( MultiByteToWideChar(CP_ACP, 0, argv[6].strptr, -1, wszText, 1024) == 0 )
                        RETVAL(-3)

                    tip.cbStruct = sizeof(tip);
                    tip.pszText = wszText;
                    tip.pszTitle = wszTitle;
                    tip.ttiIcon = TTI_INFO;

                    if ( argc > 7 )
                    {
                        if ( argv[7].strptr[0] == 'E' ) tip.ttiIcon = TTI_ERROR;
                        if ( argv[7].strptr[0] == 'N' ) tip.ttiIcon = TTI_NONE;
                        if ( argv[7].strptr[0] == 'W' ) tip.ttiIcon = TTI_WARNING;
                    }
                    RETVAL(!Edit_ShowBalloonTip(hCtrl, &tip))
                }
                else RETERR
            }
            else if ( !strcmp(argv[4].strptr, "CUE" ) )  /* Set or get cue banner */
            {
                /* Note that the EM_GETCUEBANNER simply does not work.  At least
                 * on XP.  So the code is removed.  But, it might be worth
                 * trying on Vista.
                 */
                WCHAR wszCue[256];

                /* Requires Common Controls version 6.0 (XP) */
                if ( ComCtl32Version < COMCTL32_6_0 ) RETVAL(-4)

                if ( argc == 6 )
                {
                    if ( argv[5].strlength > 255 )
                        RETVAL(-3)

                    if ( MultiByteToWideChar(CP_ACP, 0, argv[5].strptr, -1, wszCue, 256) == 0 )
                        RETVAL(-3)

                    RETVAL(!Edit_SetCueBannerText(hCtrl, wszCue))
                }
                else RETERR
            }
            else RETERR
        }
        else RETERR
    }
    else if ( argv[2].strptr[0] == 'X' ) /* eXtended work with the window style */
    {
        LONG lStyle = GetWindowLong(hCtrl, GWL_STYLE);
        if ( ! lStyle ) RETC(-3)

        CHECKARGL(4);

        if ( !strcmp(argv[3].strptr, "TAB") )   /* Set or remove tab stop  style */
        {
            CHECKARGL(5);
            if ( argv[4].strptr[0] == '1' )
                lStyle |= WS_TABSTOP;
            else
                lStyle &= ~WS_TABSTOP;
        }
        else if ( !strcmp(argv[3].strptr, "GROUP") )  /* Set or remove group style */
        {
            CHECKARGL(5);
            if ( argv[4].strptr[0] == '1' )
                lStyle |= WS_GROUP;
            else
                lStyle &= ~WS_GROUP;
        }
        else if ( !strcmp(argv[3].strptr, "EDIT" ) )  /* Work with edit control style */
        {
            LONG lChangeStyle;

            CHECKARGL(5);

            /* If 'G' get the current style and return its string form. */
            if ( argv[4].strptr[0] == 'G' ) return EditStyleToString(lStyle, retstr);

            CHECKARGL(6);

            lChangeStyle = ParseEditStyle(argv[5].strptr);
            if ( ! lChangeStyle ) RETVAL(-3)

            if ( argv[4].strptr[0] == 'S' )           /* Set style */
            {
                lStyle |= lChangeStyle;
            }
            else if ( argv[4].strptr[0] == 'C' )      /* Clear style */
            {
                lStyle &= ~lChangeStyle;
            }
            else if ( argv[4].strptr[0] == 'R' )      /* Replace style */
            {
                LONG lAddedStyle;

                CHECKARG(7);

                lAddedStyle = ParseEditStyle(argv[6].strptr);
                if ( ! lAddedStyle ) RETVAL(-3)

                lStyle = (lStyle & ~lChangeStyle) | lAddedStyle;
            }
            else RETERR
        }
        else RETERR

        return SetStyle(hCtrl, lStyle, retstr);
    }
    else if ( argv[2].strptr[0] == 'U' ) /* Get, set 4-byte user data */
    {
        if ( argc == 3 )
        {
            RETPTR(getWindowPtr(hCtrl, GWLP_USERDATA));
        }
        else if ( argc == 4 )
        {
            RETPTR(setWindowPtr(hCtrl, GWLP_USERDATA, atol(argv[3].strptr)));
        }
        else RETERR
    }
    else if ( argv[2].strptr[0] == 'K' ) /* subclass control for Keypress */
    {
        SUBCLASSDATA * pData= NULL;

        /* Requires Common Controls version 6.0 (XP SP 2) */
        if ( ComCtl32Version < COMCTL32_6_0 ) RETVAL(-4)

        if ( argv[3].strptr[0] == 'Q' )           /* Query if already sub-classed */
        {
            BOOL success = GetWindowSubclass(hCtrl, KeyPressSubclassProc, id, (DWORD_PTR *)&pData);
            if ( ! success ) RETVAL(0)
            if ( argc == 4 ) RETVAL(1)

            CHECKARGL(5);

            if ( ! pData ) RETVAL(0)
            RETVAL(seekKeyPressMethod(pData->pKeyPressData, argv[4].strptr) ? 1 : 0);
        }
        else if ( argv[3].strptr[0] == 'C' )      /* Connect key press subclass   */
        {
            SUBCLASSDATA *pData = NULL;
            DIALOGADMIN  *dlgAdm;
            LONG ret = 0;
            BOOL success = GetWindowSubclass(hCtrl, KeyPressSubclassProc, id, (DWORD_PTR *)&pData);

            CHECKARGL(7);

            dlgAdm = (DIALOGADMIN *)GET_POINTER(argv[4]);
            if ( ! dlgAdm )  RETVAL(-3)

            if ( argv[5].strlength == 0 || argv[6].strlength == 0 ) return -1;

            /* If the subclass is already installed, just update the data block.
             * If not installed, then the data block needs to be allocated and
             * the subclass installed.
             */
            if ( pData )
            {
                /* If not success something is wrong, just quit. */
                if ( ! success ) RETVAL(-6)
                if ( argc > 7 )
                    ret = setKeyPressData(pData->pKeyPressData, argv[5], argv[6], argv[7].strptr);
                else
                    ret = setKeyPressData(pData->pKeyPressData, argv[5], argv[6], NULL);
                RETVAL(ret)
            }
            else
            {
                pData = (SUBCLASSDATA *)LocalAlloc(LPTR, sizeof(SUBCLASSDATA));
                if ( ! pData ) RETVAL(-5)

                pData->pKeyPressData = (KEYPRESSDATA *)LocalAlloc(LPTR, sizeof(KEYPRESSDATA));
                if ( ! pData->pKeyPressData )
                {
                    LocalFree(pData);
                    RETVAL(-5)
                }

                pData->hCtrl = hCtrl;
                pData->uID = id;
                pData->pMessageQueue = dlgAdm->pMessageQueue;

                if ( argc > 7 )
                    ret = setKeyPressData(pData->pKeyPressData, argv[5], argv[6], argv[7].strptr);
                else
                    ret = setKeyPressData(pData->pKeyPressData, argv[5], argv[6], NULL);

                if ( ret == -5 )
                {
                    /* Memory allocation failure.  Clean up and return. */
                    LocalFree(pData);
                    RETVAL(ret)
                }

                RETVAL(! SendMessage(hDlg, WM_USER_SUBCLASS, (WPARAM)KeyPressSubclassProc, (LPARAM)pData))
            }
        }
        else if ( argv[3].strptr[0] == 'R' )      /* Remove the subclass */
        {
            SUBCLASSDATA * pData = NULL;
            BOOL success = GetWindowSubclass(hCtrl, KeyPressSubclassProc, id, (DWORD_PTR *)&pData);

            /* If success, the subclass is still installed, otherwise the
             * subclass has already been removed, (or never existed.)
             */
            if ( success )
            {
                UINT index;
                success = FALSE;  /* Reuse the success variable. */

                /* If no method name, remove the whole thing. */
                if ( argc == 4 ) RETVAL(! removeKeyPressSubclass(pData, hDlg, id))

                CHECKARGL(5)

                index = seekKeyPressMethod(pData->pKeyPressData, argv[4].strptr);
                if ( ! index ) RETVAL(-1)

                /* If only 1 method left, remove the subclass entirely.
                 * Otherwise, remove the subclass, fix up the subclass data
                 * block, then reinstall the subclass.
                 */
                if ( pData->pKeyPressData->usedMethods == 1 ) RETVAL(removeKeyPressSubclass(pData, hDlg, id))

                if ( SendMessage(hDlg, WM_USER_SUBCLASS_REMOVE, (WPARAM)KeyPressSubclassProc, (LPARAM)id) )
                {
                    removeKeyPressMethod(pData->pKeyPressData, index);
                    success = SendMessage(hDlg, WM_USER_SUBCLASS, (WPARAM)KeyPressSubclassProc, (LPARAM)pData) != 0;
                }
                RETVAL(! success)
            }

            RETVAL(-2)  /* Subclass procedure is not installed. */
        }
    }
    RETERR
}

/**
 * Extended List-View control functionality.  Implements capabilities not
 * present in the original ooDialog ListControl.  In general, this will be
 * capabilities that Microsoft has added to the control.
 *
 * The parameters sent from ooRexx as an array of RXString:
 *
 * argv[0]  Window handle of list-view control.
 *
 * argv[1]  Major designator:  M for message, etc..  Only the first letter of
 *          the string is tested and it must be capitalized.
 *
 * argv[2]  Minor designator:  STYLE for (extended) list style, etc..  The whole
 *          capitalized word is used.
 *
 * argv[3]  Dependent on function.
 *
 * Return to ooRexx, in general:
 *  < -4 a negated system error code
 *    -4 unsupported ComCtl32 Version
 *    -3 problem with an argument
 *    -2 operation not supported by this list-view control
 *    -1 problem with the list control id or handle
 *     0 the Windows API call succeeds
 *     1 the Windows API call failed
 *  >  1 dependent on the function, usually a returned value not a return code
 */
size_t RexxEntry HandleListCtrlEx(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    HWND hList;

    /* Minimum of 3 args. */
    CHECKARGL(3);

    hList = GET_HWND(argv[0]);
    if ( hList == 0 || ! IsWindow(hList) ) RETVAL(-1);

    /* M - window message related function */
    if ( argv[1].strptr[0] == 'M' )
    {
        if ( !strcmp(argv[2].strptr, "STYLE") )     /* work with extended styles */
        {
            DWORD dwMask;

            CHECKARGL(4);

            if ( argv[3].strptr[0] == 'G' )         /* Get style, string representation. */
            {
                return ListExtendedStyleToString(hList, retstr);
            }
            else if ( argv[3].strptr[0] == 'L' )    /* Get style, as a Long. */
            {
                RETVAL(ListView_GetExtendedListViewStyle(hList));
            }

            CHECKARGL(5);
            dwMask = ParseExtendedListStyle(argv[4].strptr);
            if ( ! dwMask ) RETVAL(-3);

            /* No return value from this API, so return 0 to ooRexx. */

            if ( argv[3].strptr[0] == 'C' )         /* Clear (remove) style*/
            {
                ListView_SetExtendedListViewStyleEx(hList, dwMask, 0);
            }
            else if ( argv[3].strptr[0] == 'S' )    /* Set style */
            {
                ListView_SetExtendedListViewStyleEx(hList, dwMask, dwMask);
            }
            else if ( argv[3].strptr[0] == 'R' )    /* Replace style */
            {
                DWORD dwNew;

                CHECKARGL(6);
                dwNew = ParseExtendedListStyle(argv[5].strptr);
                if ( ! dwNew ) RETVAL(-3);

                ListView_SetExtendedListViewStyleEx(hList, dwMask, 0);
                ListView_SetExtendedListViewStyleEx(hList, dwNew, dwNew);
            }
            else RETERR;

            RETVAL(0);
        }
        else if ( !strcmp(argv[2].strptr, "HOVER") )    /* Set, get hover time */
        {
            if ( argc == 3 )
            {
                RETVAL(ListView_GetHoverTime(hList));
            }
            else if ( argc == 4 )
            {
                RETVAL(ListView_SetHoverTime(hList, atol(argv[3].strptr)));
            }
            else RETERR;
        }
        else if ( !strcmp(argv[2].strptr, "CHK") )    /* Set, get check box state */
        {
            LONG item;
            if ( ! (ListView_GetExtendedListViewStyle(hList) & LVS_EX_CHECKBOXES) )
                RETVAL(-2)

            CHECKARGL(4)

            item = atol(argv[3].strptr);
            if ( item < -1 || item > (ListView_GetItemCount(hList) - 1) ) RETVAL(-3);

            if ( argc == 4 )
            {
                if ( item < 0 ) RETVAL(-3);
                RETVAL(!(ListView_GetCheckState(hList, (UINT)item) == 0));
            }
            else if ( argc == 5 )
            {
                int check = atol(argv[4].strptr);
                if ( check != 0 && check != 1 ) RETVAL(-3);

                /* No return value with these APIs. */
                if ( item == -1 )
                {
                    ListView_SetItemState(hList, item, INDEXTOSTATEIMAGEMASK(check + 1), LVIS_STATEIMAGEMASK);
                }
                else
                {
                    ListView_SetCheckState(hList, (UINT)item, (BOOL)check);
                }
                RETVAL(0);
            }
            else RETERR;
        }
        else if ( !strcmp(argv[2].strptr, "TOOL") )     /* Set tool tip text */
        {
            /* Place holder. The user will be able to set the tool tip text for
             * individual list items.  A generic function will display the tool
             * tip upon receiving a LVN_GETINFOTIP message. (If the user has
             * enabled tool tips and the list item has tool tip text set for
             * it.)
             */
            RETVAL(1);  // Return 1 (failed) until this is implemented.
        }
    }
    RETERR;
}


size_t RexxEntry HandleListCtrl(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND h;

   CHECKARGL(3);

   h = GET_HWND(argv[2]);
   if (!h) RETERR;

   if (argv[0].strptr[0] == 'I')
   {
       if (!strcmp(argv[1].strptr, "INS"))
       {
           LV_ITEM lvi;

           CHECKARG(6);

           lvi.mask = LVIF_TEXT;

           lvi.iItem = atoi(argv[3].strptr);
           lvi.iSubItem = 0;

           lvi.pszText = (LPSTR)argv[4].strptr;
           lvi.cchTextMax = (int)argv[4].strlength;

           lvi.iImage = atoi(argv[5].strptr);
           if (lvi.iImage >= 0) lvi.mask |= LVIF_IMAGE;

           RETVAL(ListView_InsertItem(h, &lvi));
       }
       else
       if (!strcmp(argv[1].strptr, "SET"))
       {
           LV_ITEM lvi;

           CHECKARG(7);

           lvi.mask = 0;

           lvi.iItem = atoi(argv[3].strptr);
           lvi.iSubItem = atoi(argv[4].strptr);

           lvi.pszText = (LPSTR)argv[5].strptr;
           lvi.cchTextMax = (int)argv[5].strlength;

           if (!strcmp(argv[6].strptr,"TXT"))
           {
               lvi.mask |= LVIF_TEXT;
               RETC(!SendMessage(h, LVM_SETITEMTEXT, lvi.iItem, (LPARAM)&lvi));
           }
           else if (!strcmp(argv[6].strptr,"STATE"))
           {
               lvi.state = 0;
               lvi.stateMask = 0;

               if (strstr(argv[5].strptr, "NOTCUT"))  lvi.stateMask |= LVIS_CUT;
               else if (strstr(argv[5].strptr, "CUT"))  {lvi.state |= LVIS_CUT; lvi.stateMask |= LVIS_CUT;}
               if (strstr(argv[5].strptr, "NOTDROP"))  lvi.stateMask |= LVIS_DROPHILITED;
               else if (strstr(argv[5].strptr, "DROP"))  {lvi.state |= LVIS_DROPHILITED; lvi.stateMask |= LVIS_DROPHILITED;}
               if (strstr(argv[5].strptr, "NOTFOCUSED"))  lvi.stateMask |= LVIS_FOCUSED;
               else if (strstr(argv[5].strptr, "FOCUSED"))  {lvi.state |= LVIS_FOCUSED; lvi.stateMask |= LVIS_FOCUSED;}
               if (strstr(argv[5].strptr, "NOTSELECTED"))  lvi.stateMask |= LVIS_SELECTED;
               else if (strstr(argv[5].strptr, "SELECTED"))  {lvi.state |= LVIS_SELECTED; lvi.stateMask |= LVIS_SELECTED;}

               RETC(!SendMessage(h, LVM_SETITEMSTATE, lvi.iItem, (LPARAM)&lvi));
           }
           else
           {
               if (lvi.cchTextMax) lvi.mask |= LVIF_TEXT;

               lvi.iImage = atoi(argv[6].strptr);
               if (lvi.iImage >= 0) lvi.mask |= LVIF_IMAGE;
               RETC(!ListView_SetItem(h, &lvi));
           }
       }
       else
       if (!strcmp(argv[1].strptr, "GET"))
       {
           LV_ITEM lvi;
           CHAR data[256];

           CHECKARG(7);

           lvi.iItem = atoi(argv[3].strptr);
           lvi.iSubItem = atoi(argv[4].strptr);
           lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
           lvi.pszText = data;
           lvi.cchTextMax = 255;
           lvi.stateMask = LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;

           if (!strcmp(argv[6].strptr,"TXT"))
           {
               INT len;
               lvi.pszText = retstr->strptr;
               len = (int)SendMessage(h, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)&lvi);
               retstr->strlength = len;
               return 0;
           }
           else if (!strcmp(argv[6].strptr,"STATE"))
           {
               UINT state;

               state = ListView_GetItemState(h, lvi.iItem, lvi.stateMask);
               retstr->strptr[0] = '\0';
               if (state & LVIS_CUT) strcat(retstr->strptr, "CUT ");
               if (state & LVIS_DROPHILITED) strcat(retstr->strptr, "DROP ");
               if (state & LVIS_FOCUSED) strcat(retstr->strptr, "FOCUSED ");
               if (state & LVIS_SELECTED) strcat(retstr->strptr, "SELECTED ");
               retstr->strlength = strlen(retstr->strptr);
               return 0;
           }
           else if (ListView_GetItem(h, &lvi))
           {
               SetRexxStem(argv[5].strptr, -1, "!Text", lvi.pszText);
               itoa(lvi.iImage, data, 10);
               SetRexxStem(argv[5].strptr, -1, "!Image", data);
               data[0] = '\0';
               if (lvi.state & LVIS_CUT) strcat(data, "CUT ");
               if (lvi.state & LVIS_DROPHILITED) strcat(data, "DROP ");
               if (lvi.state & LVIS_FOCUSED) strcat(data, "FOCUSED ");
               if (lvi.state & LVIS_SELECTED) strcat(data, "SELECTED ");
               SetRexxStem(argv[5].strptr, -1, "!State", data);
               RETC(0)
           }
           RETVAL(-1);
       }
       else
       if (!strcmp(argv[1].strptr, "DEL"))
       {
           INT item;
           CHECKARG(4);
           item = atoi(argv[3].strptr);
           if (!item && !strcmp(argv[3].strptr,"ALL"))
              RETC(!ListView_DeleteAllItems(h))
           else if (ListView_GetItemCount(h) >0)
              RETC(!ListView_DeleteItem(h, item))
           RETVAL(-1)
       }
       else
       if (!strcmp(argv[1].strptr, "GETNEXT"))
       {
           ULONG flag;
           LONG startItem;

           CHECKARG(5);

           startItem = atol(argv[3].strptr);

           if (!strcmp(argv[4].strptr, "FIRSTVISIBLE"))
               RETVAL(ListView_GetTopIndex(h))

           flag = 0;
           if (strstr(argv[4].strptr,"ABOVE")) flag |= LVNI_ABOVE;
           if (strstr(argv[4].strptr,"BELOW")) flag |= LVNI_BELOW;
           if (strstr(argv[4].strptr,"TOLEFT")) flag |= LVNI_TOLEFT;
           if (strstr(argv[4].strptr,"TORIGHT")) flag |= LVNI_TORIGHT;
           if (!flag) flag = LVNI_ALL;

           if (strstr(argv[4].strptr,"CUT")) flag |= LVNI_CUT;
           else if (strstr(argv[4].strptr,"DROP")) flag |= LVNI_DROPHILITED;
           else if (strstr(argv[4].strptr,"FOCUSED")) flag |= LVNI_FOCUSED;
           else if (strstr(argv[4].strptr,"SELECTED")) flag |= LVNI_SELECTED;

           RETVAL(ListView_GetNextItem(h, startItem, flag))
       }
       else
       if (!strcmp(argv[1].strptr, "SETIMG"))
       {
           HIMAGELIST iL;
           WORD ilt;

           CHECKARG(7);
           iL = CreateImageList(3, h, argv, argc);

           if (!strcmp(argv[6].strptr,"SMALL")) ilt = LVSIL_SMALL;
           else ilt = LVSIL_NORMAL;

           if (iL) RETHANDLE(ListView_SetImageList(h, iL, ilt))
           else RETC(0)
       }
       else
       if (!strcmp(argv[1].strptr, "UNSETIMG"))
       {
           HIMAGELIST iL;
           WORD ilt;

           CHECKARG(4);
           if (!strcmp(argv[3].strptr,"SMALL")) ilt = LVSIL_SMALL;
           else ilt = LVSIL_NORMAL;

           iL = ListView_GetImageList(h, ilt);
           if (!iL) RETC(1)
           ListView_SetImageList(h, 0, ilt);

           RETC(!ImageList_Destroy( iL))
       }
       else
       if (!strcmp(argv[1].strptr, "FIND"))
       {
           LONG startItem;
           LV_FINDINFO finfo;

           CHECKARGL(6);

           startItem = atol(argv[3].strptr);

           if (strstr(argv[4].strptr,"NEAREST")) finfo.flags = LVFI_NEARESTXY;
           else finfo.flags = LVFI_STRING;

           if (strstr(argv[4].strptr,"PARTIAL")) finfo.flags |= LVFI_PARTIAL;
           if (strstr(argv[4].strptr,"WRAP")) finfo.flags |= LVFI_WRAP;

           if ((finfo.flags & LVFI_STRING) == LVFI_STRING)
               finfo.psz = argv[5].strptr;
           else {
               CHECKARG(8);
               finfo.pt.x = atol(argv[5].strptr);
               finfo.pt.y = atol(argv[6].strptr);
               if (!strcmp(argv[7].strptr,"UP")) finfo.vkDirection = VK_UP;
               else if (!strcmp(argv[7].strptr,"LEFT")) finfo.vkDirection  = VK_LEFT;
               else if (!strcmp(argv[7].strptr,"RIGHT")) finfo.vkDirection  = VK_RIGHT;
               else finfo.vkDirection  = VK_DOWN;
           }

           RETVAL(ListView_FindItem(h, startItem, &finfo))
       }
       else
       if (!strcmp(argv[1].strptr, "EDIT"))
       {
           CHECKARG(4);

           RETHANDLE(ListView_EditLabel(h, atol(argv[3].strptr)))
       }
       else
       if (!strcmp(argv[1].strptr, "SUBCL_EDIT"))
       {
           HWND ew = ListView_GetEditControl(h);
           if (ew)
           {
               WNDPROC oldProc = (WNDPROC)setWindowPtr(ew, GWLP_WNDPROC, (LONG_PTR)CatchReturnSubProc);
               if (oldProc != (WNDPROC)CatchReturnSubProc) wpOldEditProc = oldProc;
               RETPTR(oldProc)
           }
           else RETC(0)
       }
       else
       if (!strcmp(argv[1].strptr, "RESUB_EDIT"))
       {
           HWND ew = ListView_GetEditControl(h);
           if (ew)
           {
               setWindowPtr(ew, GWLP_WNDPROC, (LONG_PTR)wpOldEditProc);
               RETC(0)
           }
           RETVAL(-1)
       }
   }
   else
   if (argv[0].strptr[0] == 'M')
   {
       if (!strcmp(argv[1].strptr, "CNT"))
       {
           RETVAL(ListView_GetItemCount(h))
       }
       else
       if (!strcmp(argv[1].strptr, "CNTSEL"))
       {
           RETVAL(ListView_GetSelectedCount(h))
       }
       else
       if (!strcmp(argv[1].strptr, "REDRAW"))
       {
           CHECKARG(5);

           RETC(!ListView_RedrawItems(h, atol(argv[3].strptr), atol(argv[4].strptr)));
       }
       else
       if (!strcmp(argv[1].strptr, "UPDATE"))
       {
           CHECKARG(4);

           RETC(!ListView_Update(h, atol(argv[3].strptr)));
       }
       else
       if (!strcmp(argv[1].strptr, "ENVIS"))
       {
           CHECKARG(5);
           RETC(!ListView_EnsureVisible(h, atol(argv[3].strptr), IsYes(argv[4].strptr)))
       }
       else
       if (!strcmp(argv[1].strptr, "CNTPP"))
       {
           RETVAL(ListView_GetCountPerPage(h))
       }
       else
       if (!strcmp(argv[1].strptr, "SCROLL"))
       {
           CHECKARG(5);
                                      /* dx */                /* dy */
           RETC(!ListView_Scroll(h, atoi(argv[3].strptr), atoi(argv[4].strptr)))
       }
       else
       if (!strcmp(argv[1].strptr, "COLOR"))
       {
           CHECKARGL(4);

           if (argv[3].strptr[0] == 'G')
           {
               COLORREF cr;
               INT i;
               if (!strcmp(argv[3].strptr, "GETBK")) cr = ListView_GetBkColor(h);
               else if (!strcmp(argv[3].strptr, "GETTXT")) cr = ListView_GetTextColor(h);
               else if (!strcmp(argv[3].strptr, "GETTXTBK")) cr = ListView_GetTextBkColor(h);
               for (i = 0; i< 256; i++) if (cr == PALETTEINDEX(i)) RETVAL(i);
               RETVAL(-1);
           }
           else
           {
               CHECKARG(5);
               if (!strcmp(argv[3].strptr, "SETBK")) RETC(!ListView_SetBkColor(h, PALETTEINDEX(atoi(argv[4].strptr))));
               if (!strcmp(argv[3].strptr, "SETTXT")) RETC(!ListView_SetTextColor(h, PALETTEINDEX(atoi(argv[4].strptr))));
               if (!strcmp(argv[3].strptr, "SETTXTBK")) RETC(!ListView_SetTextBkColor(h, PALETTEINDEX(atoi(argv[4].strptr))));
           }
       }
       else
       if (!strcmp(argv[1].strptr, "STRWIDTH"))
       {
           CHECKARG(4);

           RETVAL(ListView_GetStringWidth(h, argv[3].strptr));
       }
       else
       if (!strcmp(argv[1].strptr, "ARRANGE"))
       {
           UINT flag;

           CHECKARG(4);

           if (!strcmp(argv[3].strptr,"LEFT")) flag = LVA_ALIGNLEFT;
           else if (!strcmp(argv[3].strptr,"TOP")) flag = LVA_ALIGNTOP;
           else if (!strcmp(argv[3].strptr,"SNAPTOGRID")) flag = LVA_SNAPTOGRID;
           else flag = LVA_DEFAULT;

           RETC(!ListView_Arrange(h, flag))
       }
       else
       if (!strcmp(argv[1].strptr, "SETCNT"))
       {
           CHECKARG(4);
           ListView_SetItemCount(h, atol(argv[3].strptr));
           RETC(0)
       }
       else
       if (!strcmp(argv[1].strptr, "GETPOS"))
       {
           POINT pt;
           CHECKARG(4);
           if (ListView_GetItemPosition(h, atol(argv[3].strptr), &pt))
           {
               sprintf(retstr->strptr, "%d %d",pt.x, pt.y);
               retstr->strlength = strlen(retstr->strptr);
               return 0;
           }
           else RETC(0);
       }
       else
       if (!strcmp(argv[1].strptr, "SETPOS"))
       {
           CHECKARG(6);

           RETC(!SendMessage(h, LVM_SETITEMPOSITION, (WPARAM)atol(argv[3].strptr), \
               MAKELPARAM((int) atoi(argv[4].strptr), (int) atoi(argv[5].strptr))))
       }
       else
       if (!strcmp(argv[1].strptr,"GETSPC"))
       {
           CHECKARG(4);
           RETVAL(ListView_GetItemSpacing(h, IsYes(argv[3].strptr)));
       }
       else
       if (!strcmp(argv[1].strptr,"SETSTYLE"))
       {
           LONG lStyle;
           CHECKARG(5);

           lStyle = GetWindowLong(h, GWL_STYLE);
           if (!lStyle) RETC(0);
           if (argv[3].strptr[0] == 'A')
           {
               lStyle |= EvaluateListStyle(argv[4].strptr);
               RETVAL(SetWindowLong(h, GWL_STYLE, lStyle));
           }
           else if (argv[3].strptr[0] == 'R')
           {
               lStyle &=~EvaluateListStyle(argv[4].strptr);
               RETVAL(SetWindowLong(h, GWL_STYLE, lStyle));
           }
       }
   }
   else
   if (argv[0].strptr[0] == 'C')
   {
       if (!strcmp(argv[1].strptr, "INS"))
       {
           LVCOLUMN lvi;
           int retVal;

           CHECKARG(7);

           lvi.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;

           lvi.iSubItem = atoi(argv[3].strptr);

           lvi.pszText = (LPSTR)argv[4].strptr;
           lvi.cchTextMax = (int)argv[4].strlength;

           lvi.cx = atoi(argv[5].strptr);
           if (lvi.cx >= 0) lvi.mask |= LVCF_WIDTH;

           if (strstr(argv[6].strptr,"CENTER")) lvi.fmt = LVCFMT_CENTER;
           else if (strstr(argv[6].strptr,"RIGHT")) lvi.fmt = LVCFMT_RIGHT;
           else lvi.fmt = LVCFMT_LEFT;

           retVal = ListView_InsertColumn(h, lvi.iSubItem, &lvi);
           if ( retVal != -1 && lvi.fmt != LVCFMT_LEFT && lvi.iSubItem == 0 )
           {
               /* According to the MSDN docs: "If a column is added to a
                * list-view control with index 0 (the leftmost column) and with
                * LVCFMT_RIGHT or LVCFMT_CENTER specified, the text is not
                * right-aligned or centered." This is the suggested work around.
                */
               lvi.iSubItem = 1;
               ListView_InsertColumn(h, lvi.iSubItem, &lvi);
               ListView_DeleteColumn(h, 0);
           }
           RETVAL(retVal);
       }
       else
       if (!strcmp(argv[1].strptr, "SET"))
       {
           LV_COLUMN lvi;
           LONG nr;

           CHECKARG(7);

           lvi.mask = 0;

           nr = atoi(argv[3].strptr);

           lvi.pszText = (LPSTR)argv[4].strptr;
           lvi.cchTextMax = (int)argv[4].strlength;
           if (lvi.cchTextMax) lvi.mask |= LVCF_TEXT;

           lvi.cx = atoi(argv[5].strptr);
           if (lvi.cx >= 0) lvi.mask |= LVCF_WIDTH;

           if (argv[6].strlength)
           {
               if (strstr(argv[6].strptr,"CENTER")) lvi.fmt = LVCFMT_CENTER;
               else if (strstr(argv[6].strptr,"RIGHT")) lvi.fmt = LVCFMT_RIGHT;
               else lvi.fmt = LVCFMT_LEFT;
              lvi.mask |= LVCF_FMT;
           }
           RETC(!ListView_SetColumn(h, nr, &lvi));
       }
       else
       if (!strcmp(argv[1].strptr, "GET"))
       {
           LV_COLUMN lvi;
           CHAR data[256];
           LONG nr;

           CHECKARG(5);

           nr = atoi(argv[3].strptr);

           lvi.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;
           lvi.pszText = data;
           lvi.cchTextMax = 255;

           if (ListView_GetColumn(h, nr, &lvi))
           {
               SetRexxStem(argv[4].strptr, -1, "!Text", lvi.pszText);
               itoa(lvi.iSubItem, data, 10);
               SetRexxStem(argv[4].strptr, -1, "!Column", data);
               itoa(lvi.cx, data, 10);
               SetRexxStem(argv[4].strptr, -1, "!Width", data);

               data[0] = '\0';
               if ( (LVCFMT_JUSTIFYMASK & lvi.fmt) == LVCFMT_CENTER )
               {
                   strcpy(data, "CENTER");
               }
               else if ( (LVCFMT_JUSTIFYMASK & lvi.fmt) == LVCFMT_RIGHT )
               {
                   strcpy(data, "RIGHT");
               }
               else
               {
                   strcpy(data, "LEFT");
               }
               SetRexxStem(argv[4].strptr, -1, "!Align", data);
               RETC(0)
           }
           else RETVAL(-1)
       }
       else
       if (!strcmp(argv[1].strptr, "GETWIDTH"))
       {
           CHECKARG(4);

           RETVAL(ListView_GetColumnWidth(h, atoi(argv[3].strptr)));
       }
       else
       if (!strcmp(argv[1].strptr, "SETWIDTH"))
       {
           LONG cx;
           CHECKARG(5);

           if (!strcmp(argv[4].strptr, "AUTO")) cx = LVSCW_AUTOSIZE;
           else if (!strcmp(argv[4].strptr, "AUTOHEADER")) cx = LVSCW_AUTOSIZE_USEHEADER;
           else cx = atoi(argv[4].strptr);

           RETC(!ListView_SetColumnWidth(h, atoi(argv[3].strptr), cx));
       }
       else
       if (!strcmp(argv[1].strptr, "DEL"))
       {
           CHECKARG(4);

           RETC(!ListView_DeleteColumn(h, atoi(argv[3].strptr)));
       }
   }
   RETC(0)
}


size_t RexxEntry HandleOtherNewCtrls(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND h;

   CHECKARGL(3);

   h = GET_HWND(argv[2]);
   if (!h) RETERR;

   if (!strcmp(argv[0].strptr, "SLIDER"))
   {
       if (!strcmp(argv[1].strptr, "POS"))
       {
           CHECKARGL(4);
           if (argc == 4)
               RETVAL((long)SendMessage(h, TBM_GETPOS, 0, 0))
           else
               SendMessage(h, TBM_SETPOS, IsYes(argv[4].strptr), atol(argv[3].strptr));
       }
       else if (!strcmp(argv[1].strptr, "SETRANGE"))
       {
           CHECKARG(6);
           if (argv[3].strptr[0] == 'L')
               SendMessage(h, TBM_SETRANGEMIN, IsYes(argv[5].strptr), atol(argv[4].strptr));
           else if (argv[3].strptr[0] == 'H')
               SendMessage(h, TBM_SETRANGEMAX, IsYes(argv[5].strptr), atol(argv[4].strptr));
           else
               SendMessage(h, TBM_SETRANGE, IsYes(argv[5].strptr), MAKELONG(atol(argv[3].strptr), atol(argv[4].strptr)));
       }
       else if (!strcmp(argv[1].strptr, "GETRANGE"))
       {
           sprintf(retstr->strptr, "%d %d",SendMessage(h, TBM_GETRANGEMIN, 0,0), SendMessage(h, TBM_GETRANGEMAX, 0,0));
           retstr->strlength = strlen(retstr->strptr);
           return 0;
       }
       else if (!strcmp(argv[1].strptr, "TICS"))
       {
           /*CHECKARG(5); */
           /* 4 arguments for 'N', 5 for all the others */
           if (argv[3].strptr[0] == 'C')
           {
               CHECKARG(5);
               SendMessage(h, TBM_CLEARTICS, IsYes(argv[4].strptr), 0);
           }
           else if (argv[3].strptr[0] == 'N')
           {
               CHECKARG(4);
               RETVAL((long)SendMessage(h, TBM_GETNUMTICS, 0, 0))
           }
           else if (argv[3].strptr[0] == 'G')
           {
               CHECKARG(5);
               RETVAL((long)SendMessage(h, TBM_GETTIC, atoi(argv[4].strptr), 0))
           }
           else if (argv[3].strptr[0] == 'S')
           {
               CHECKARG(5);
               RETC(!SendMessage(h, TBM_SETTIC, 0, atol(argv[4].strptr)))
           }
           else if (argv[3].strptr[0] == 'F')
           {
               CHECKARG(5);
               SendMessage(h, TBM_SETTICFREQ, atoi(argv[4].strptr), 0);
           }
           RETC(0);
       }
       else if (!strcmp(argv[1].strptr, "GETSTEPS"))
       {
           CHECKARG(4);
           if (argv[3].strptr[0] == 'L')
               RETVAL((long)SendMessage(h, TBM_GETLINESIZE, 0, 0))
           else RETVAL((long)SendMessage(h, TBM_GETPAGESIZE, 0, 0));
       }
       else if (!strcmp(argv[1].strptr, "SETSTEPS"))
       {
           CHECKARG(5);
           if (argv[3].strptr[0] == 'L')
               RETVAL((long)SendMessage(h, TBM_SETLINESIZE, 0, atol(argv[4].strptr)))
           else RETVAL((long)SendMessage(h, TBM_SETPAGESIZE, 0, atol(argv[4].strptr)));
       }
       else if (!strcmp(argv[1].strptr, "SETSEL"))
       {
           CHECKARGL(5);
           if (argv[3].strptr[0] == 'C')
           {
               SendMessage(h, TBM_CLEARSEL, IsYes(argv[4].strptr), 0);
               RETC(0);
           }
           CHECKARG(6);
           if (argv[3].strptr[0] == 'S')
               SendMessage(h, TBM_SETSELSTART, IsYes(argv[5].strptr), atol(argv[4].strptr));
           else if (argv[3].strptr[0] == 'E')
               SendMessage(h, TBM_SETSELEND, IsYes(argv[5].strptr), atol(argv[4].strptr));
           else
               SendMessage(h, TBM_SETSEL, IsYes(argv[5].strptr), MAKELONG(atol(argv[3].strptr), atol(argv[4].strptr)));
       }
       else if (!strcmp(argv[1].strptr, "GETSEL"))
       {
           sprintf(retstr->strptr, "%d %d",SendMessage(h, TBM_GETSELSTART, 0,0), SendMessage(h, TBM_GETSELEND, 0,0));
           retstr->strlength = strlen(retstr->strptr);
           return 0;
       }
   }
   else if (!strcmp(argv[0].strptr, "TAB"))
   {
       if (!strcmp(argv[1].strptr, "INS"))
       {
           TC_ITEM tab;
           INT item;
           CHECKARGL(6);

           item = atoi(argv[3].strptr);
           tab.mask = TCIF_TEXT;

           tab.pszText = (LPSTR)argv[4].strptr;
           tab.cchTextMax = (int)argv[4].strlength;

           tab.iImage = atoi(argv[5].strptr);
           if (tab.iImage >= 0) tab.mask |= TCIF_IMAGE;

           if (argc == 7)
           {
               tab.lParam = atol(argv[6].strptr);
               tab.mask |= TCIF_PARAM;
           }
           RETVAL(TabCtrl_InsertItem(h, item, &tab));
       }
       else if (!strcmp(argv[1].strptr, "SET"))
       {
           TC_ITEM tab;
           INT item;
           CHECKARGL(6);

           item = atoi(argv[3].strptr);

           if (strlen(argv[4].strptr)) tab.mask = TCIF_TEXT; else tab.mask = 0;
           tab.pszText = (LPSTR)argv[4].strptr;
           tab.cchTextMax = (int)argv[4].strlength;

           tab.iImage = atoi(argv[5].strptr);
           if (tab.iImage >= 0) tab.mask |= TCIF_IMAGE;

           if (argc == 7)
           {
               tab.lParam = atol(argv[6].strptr);
               tab.mask |= TCIF_PARAM;
           }
           RETC(!TabCtrl_SetItem(h, item, &tab));
       }
       else if (!strcmp(argv[1].strptr, "GET"))
       {
           TC_ITEM tab;
           INT item;
           CHAR data[32];
           CHECKARG(5);

           item = atoi(argv[3].strptr);

           tab.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
           tab.pszText = retstr->strptr;
           tab.cchTextMax = 255;

           if (TabCtrl_GetItem(h, item, &tab))
           {
               SetRexxStem(argv[4].strptr, -1, "!Text", tab.pszText);
               itoa(tab.iImage, data, 10);
               SetRexxStem(argv[4].strptr, -1, "!Image", data);
               itoa((int)tab.lParam, data, 10);
               SetRexxStem(argv[4].strptr, -1, "!Param", data);
               RETC(0)
           }
           else RETVAL(-1)
       }
       else if (!strcmp(argv[1].strptr, "SEL"))
       {
           TC_ITEM tab;
           CHECKARGL(4);

           if (!strcmp(argv[3].strptr, "GN"))
               RETVAL(TabCtrl_GetCurSel(h))
           else if (!strcmp(argv[3].strptr, "GT"))
           {
               tab.mask = TCIF_TEXT;

               tab.pszText = retstr->strptr;
               tab.cchTextMax = 255;

               if (TabCtrl_GetItem(h, TabCtrl_GetCurSel(h), &tab))
               {
                   retstr->strlength = strlen(retstr->strptr);
                   return 0;
               } else RETC(0);
           }
           else
           {
               CHECKARG(5);
               if (!strcmp(argv[3].strptr, "SN"))
                   RETVAL(TabCtrl_SetCurSel(h, atoi(argv[4].strptr)))
               else
               {
                   LONG cnt, i = 0;
                   cnt = TabCtrl_GetItemCount(h);
                   if (!cnt) RETVAL(-1);

                   while (i<cnt)
                   {
                       tab.mask = TCIF_TEXT;
                       tab.pszText = retstr->strptr;
                       tab.cchTextMax = 255;
                       if (!TabCtrl_GetItem(h, i, &tab)) RETVAL(-1);
                       if (!stricmp(tab.pszText, argv[4].strptr)) RETVAL(TabCtrl_SetCurSel(h, i));
                       i++;
                   }
                   RETVAL(-1);
               }
           }
       }
       else if (!strcmp(argv[1].strptr, "FOCUS"))
       {
           CHECKARG(4);
           if (argv[3].strptr[0] == 'G')
               RETVAL(TabCtrl_GetCurFocus(h))
           else
           {
               TabCtrl_SetCurFocus(h, atoi(argv[3].strptr));
               RETC(0);
           }
       }
       else if (!strcmp(argv[1].strptr, "DEL"))
       {
           INT item;
           CHECKARG(4);
           item = atoi(argv[3].strptr);
           if (!item && !strcmp(argv[3].strptr,"ALL"))
              RETC(!TabCtrl_DeleteAllItems(h))
           else if (TabCtrl_GetItemCount(h) >0)
              RETC(!TabCtrl_DeleteItem(h, item))
           RETVAL(-1)
       }
       else if (!strcmp(argv[1].strptr, "CNT"))
       {
           RETVAL(TabCtrl_GetItemCount(h))
       }
       else if (!strcmp(argv[1].strptr, "ROWCNT"))
       {
           RETVAL(TabCtrl_GetRowCount(h))
       }
       else if (!strcmp(argv[1].strptr, "SETIMG"))
       {
           HIMAGELIST iL;

           CHECKARG(6);
           iL = CreateImageList(3, h, argv, argc);
           if (iL) RETHANDLE(TabCtrl_SetImageList(h, iL))
           else RETC(0)
       }
       else if (!strcmp(argv[1].strptr, "UNSETIMG"))
       {
           HIMAGELIST iL;

           iL = TabCtrl_GetImageList(h);
           if (!iL) RETC(1)
           TabCtrl_SetImageList(h, NULL);

           RETC(!ImageList_Destroy( iL))
       }
       else if (!strcmp(argv[1].strptr, "PADDING"))
       {
           CHECKARG(5);

           TabCtrl_SetPadding(h, atoi(argv[3].strptr), atoi(argv[4].strptr));
           RETC(0);
       }
       else if (!strcmp(argv[1].strptr, "SIZE"))
       {
           LONG prevsize;
           CHECKARG(5);

           prevsize = TabCtrl_SetItemSize(h, atoi(argv[3].strptr), atoi(argv[4].strptr));
           sprintf(retstr->strptr, "%d %d", LOWORD(prevsize), HIWORD(prevsize));
           retstr->strlength = strlen(retstr->strptr);
           return 0;
       }
       else if (!strcmp(argv[1].strptr, "RECT"))
       {
           RECT r;
           CHECKARG(4);

           retstr->strlength = 0;
           if (TabCtrl_GetItemRect(h, atoi(argv[3].strptr), &r))
           {
                sprintf(retstr->strptr, "%d %d %d %d", r.left, r.top, r.right, r.bottom);
                retstr->strlength = strlen(retstr->strptr);
           }
           return 0;
       }
       else if (!strcmp(argv[1].strptr, "ADJUST"))
       {
           RECT r;
           BOOL adapt;  /* or only query */
           CHECKARG(8);

           adapt = IsYes(argv[3].strptr);

           r.left = atol(argv[4].strptr);
           r.top = atol(argv[5].strptr);
           r.right = atol(argv[6].strptr);
           r.bottom = atol(argv[7].strptr);

           /* if adapt, the rectangle is adjusted to the given rectangle,
              if not adapt, the window size that could store the given handle is returned */
           TabCtrl_AdjustRect(h, adapt, &r);

           sprintf(retstr->strptr, "%d %d %d %d", r.left, r.top, r.right, r.bottom);
           retstr->strlength = strlen(retstr->strptr);
           return 0;
       }
   }
   RETC(0)
}

static int dateTimeOperation(HWND hCtrl, char *buffer, size_t length, size_t type)
{
    SYSTEMTIME sysTime = {0};
    int ret = 1;

    switch ( type )
    {
        case DTO_GETDTP :
            switch ( DateTime_GetSystemtime(hCtrl, &sysTime) )
            {
                case GDT_VALID:
                    _snprintf(buffer, length,
                              "%hu:%02hu:%02hu.%hu %hu %hu %hu %hu",
                              sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds,
                              sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wDayOfWeek);

                    ret = (int)strlen(buffer);
                    break;

                case GDT_NONE:
                    buffer[0] = '0';
                    buffer[1] = '\0';
                    break;

                case GDT_ERROR:
                default :
                    /* Failed */
                    buffer[0] = '1';
                    buffer[1] = '\0';
                    break;
            }
            break;

        case DTO_SETDTP :
        {
            int hr, min, sec, ms, dy, mn, yr;
            ret = sscanf(buffer, "%hu:%02hu:%02hu.%hu %hu %hu %hu", &hr, &min, &sec, &ms, &dy, &mn, &yr);

            if ( ret == 8 )
            {
                sysTime.wHour = hr;
                sysTime.wMinute = min;
                sysTime.wSecond = sec;
                sysTime.wMilliseconds = ms;
                sysTime.wDay = dy;
                sysTime.wMonth = mn;
                sysTime.wYear = yr;

                if ( DateTime_SetSystemtime(hCtrl, GDT_VALID, &sysTime) == 0 )
                {
                    /* Failed */
                    ret = 1;
                }
                else
                {
                    /* Good */
                    ret = 0;
                }
            }
            else
            {
                ret = -3;
            }
        } break;

        case DTO_GETMONTH :
            if ( MonthCal_GetCurSel(hCtrl, &sysTime) == 0 )
            {
                /* Failed */
                buffer[0] = '1';
                buffer[1] = '\0';
            }
            else
            {
                _snprintf(buffer, length, "%hu %hu %hu %hu", sysTime.wDay,
                          sysTime.wMonth, sysTime.wYear, sysTime.wDayOfWeek);
                ret = (int)strlen(buffer);
            }
            break;

        case DTO_SETMONTH :
        {
            int dy, mn, yr;
            ret = sscanf(buffer, "%hu %hu %hu", &dy, &mn, &yr);
            if ( ret == 3 )
            {
                sysTime.wDay = dy;
                sysTime.wMonth = mn;
                sysTime.wYear = yr;
                if ( MonthCal_SetCurSel(hCtrl, &sysTime) == 0 )
                {
                    /* Failed */
                    ret = 1;
                }
                else
                {
                    /* Good */
                    ret = 0;
                }
            }
            else
            {
                ret = -3;
            }
        } break;

        default :
            /* Shouldn't happen, just set an error code. */
            buffer[0] = '1';
            buffer[1] = '\0';
            break;
    }
    return ret;
}

/**
 * Implements the interface to the Month Calendar control.
 *
 * The parameters sent from ooRexx as an array of RXString:
 *
 * argv[0]  Window handle of the month calendar control.
 *
 * argv[1]  Major designator:  G for get, etc..  Only the first letter of
 *          the string is tested and it must be capitalized.
 *
 * argv[2]  Minor designator:  COL for get color, etc..  The whole capitalized
 *          substring is used.
 *
 * argv[3]  Dependent on function.
 *
 * Return to ooRexx, in general:
 *  < -5 a negated system error code
 *    -5 not implemented yet
 *    -4 unsupported ComCtl32 Version
 *    -3 problem with an argument
 *    -2 operation not supported by this month calendar control
 *    -1 problem with the month calendar control id or handle
 *     0 the Windows API call succeeds
 *     1 the Windows API call failed
 *  >  1 dependent on the function, usually a returned value not a return code
 */
size_t RexxEntry HandleMonthCalendar(const char *funcname, size_t argc, CONSTRXSTRING *argv,
                                     const char *qname, RXSTRING *retstr)
{
    HWND       hwnd;
    SYSTEMTIME sysTime = {0};

    /* Minimum of 2 args. */
    CHECKARGL(2);

    hwnd = GET_HWND(argv[0]);
    if ( hwnd == 0 || ! IsWindow(hwnd) )
    {
        RETVAL(-1);
    }

    /* G - 'get' something function */
    if ( argv[1].strptr[0] == 'G' )
    {
        if ( strcmp(argv[2].strptr, "COL") == 0 )          /* GetColor()  */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "CUR") == 0 )     /* GetCurSel()  */
        {
            retstr->strlength = dateTimeOperation(hwnd, retstr->strptr, RXAUTOBUFLEN, DTO_GETMONTH);
            return 0;
        }
        else if ( strcmp(argv[2].strptr, "FIR") == 0 )     /* GetFirstDayOfWeek() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "MSEL") == 0 )    /* GetMaxSelCount() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "MTOD") == 0 )     /* GetMaxTodayWidth() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "MIN") == 0 )     /* GetMinReqRect () */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "MDEL") == 0 )     /* GetMonthDelta() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "MRAN") == 0 )     /* GetMonthRange() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "RAN") == 0 )     /* GetRange() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "SEL") == 0 )     /* GetSelRange() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "TOD") == 0 )     /* GetToday() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "UNI") == 0 )     /* GetUnicodeFormat() */
        {
            RETC(MonthCal_GetUnicodeFormat(hwnd) ? 1 : 0)
        }
        else RETERR;
    }
    else if ( argv[1].strptr[0] == 'S' )
    {
        if ( strcmp(argv[2].strptr, "COL") == 0 )          /* SetColor()  */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "CUR") == 0 )     /* SetCurSel()  */
        {
            CHECKARG(4)

            /* buffer length is not used for the 'SET' operations. */
            RETVAL(dateTimeOperation(hwnd, (char *)argv[3].strptr, 0, DTO_SETMONTH));
        }
        else if ( strcmp(argv[2].strptr, "FIR") == 0 )     /* SetFirstDayOfWeek() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "MSEL") == 0 )    /* SetMaxSelCount() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "MTOD") == 0 )    /* SetMaxTodayWidth() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "MIN") == 0 )     /* SetMinReqRect () */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "MDEL") == 0 )    /* SetMonthDelta() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "MRAN") == 0 )    /* SetMonthRange() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "RAN") == 0 )     /* SetRange() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "SEL") == 0 )     /* SetSelRange() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "TOD") == 0 )     /* SetToday() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "UNI") == 0 )     /* SetUnicodeFormat() */
        {
            RETC(0)
        }
        else RETERR;
    }
    else if ( argv[1].strptr[0] == 'H' )               /* HitTest() */
    {
        RETC(0)
    }

    RETERR;
}


/**
 * Implements the interface to the Date and Time Picker control.
 *
 * The parameters sent from ooRexx as an array of RXString:
 *
 * argv[0]  Window handle of the date and time picker control.
 *
 * argv[1]  Major designator:  G for get, etc..  Only the first letter of
 *          the string is tested and it must be capitalized.
 *
 * argv[2]  Minor designator:  STYLE for (extended) list style, etc..  The whole
 *          capitalized word is used.
 *
 * argv[3]  Dependent on function.
 *
 * Return to ooRexx, in general:
 *  < -5 a negated system error code
 *    -5 not implemented yet
 *    -4 unsupported ComCtl32 Version
 *    -3 problem with an argument
 *    -2 operation not supported by this month calendar control
 *    -1 problem with the month calendar control id or handle
 *     0 the Windows API call succeeds
 *     1 the Windows API call failed
 *  >  1 dependent on the function, usually a returned value not a return code
 */
size_t RexxEntry HandleDateTimePicker(const char *funcname, size_t argc, CONSTRXSTRING *argv,
                                      const char *qname, RXSTRING *retstr)
{
    HWND       hwnd;
    SYSTEMTIME sysTime = {0};

    /* Minimum of 3 args. */
    CHECKARGL(3);

    hwnd = GET_HWND(argv[0]);
    if ( hwnd == 0 || ! IsWindow(hwnd) )
    {
        RETVAL(-1);
    }

    /* G - 'get' something function */
    if ( argv[1].strptr[0] == 'G' )
    {
        if ( strcmp(argv[2].strptr, "CAL")== 0  )          /* GetMonthCal()  */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "COL")== 0  )     /* GetMonthCalColor()  */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "FON")== 0  )     /* GetMonthCalFont() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "RAN")== 0  )    /* GetRange() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "SYS")== 0  )    /* GetSystemtime() */
        {
            retstr->strlength = dateTimeOperation(hwnd, retstr->strptr, RXAUTOBUFLEN, DTO_GETDTP);
            return 0;
        }
        else
        {
            RETERR;
        }
    }
    else if ( argv[1].strptr[0] == 'S' )
    {
        if ( strcmp(argv[2].strptr, "FOR") == 0 )          /* SetFormat()  */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "COL") == 0 )     /* SetMonthCalColor()  */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "FON") == 0 )     /* SetMonthCalFont() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "RAN") == 0 )    /* SetRange() */
        {
            RETC(0)
        }
        else if ( strcmp(argv[2].strptr, "SYS") == 0 )    /* SetSystemtime() */
        {
            if ( argc < 4 )
            {
                /* Set the DTP control to "no date" and clear its check box. */
                if ( DateTime_SetSystemtime(hwnd, GDT_NONE, &sysTime) == 0 )
                {
                    /* Failed */
                    RETC(1)
                }
                RETC(0)
            }
            else
            {
                CHECKARGL(4);

                /* Buffer length is not used for the 'SET' operations. */
                RETVAL(dateTimeOperation(hwnd, (char *)argv[3].strptr, 0, DTO_SETDTP));
            }
        }
        else RETERR;
    }

    RETERR;
}


/* These inline (and non-inline) convenience functions will be moved so that
 * they are accessible by all of ooDialog at some point.  Right now they are
 * just used by native method functions in this source file.
 */

bool rxStr2Number(RexxMethodContext *, CSTRING, uint64_t *, int);

#define OOD_ID_EXCEPTION -9

#define NO_HMODULE_MSG            "failed to obtain %s module handle; OS error code %d"
#define NO_PROC_MSG               "failed to get procedeure adddress for %s(); OS error code %d"
#define API_FAILED_MSG            "system API %s() failed; OS error code %d"
#define COM_API_FAILED_MSG        "system API %s() failed; COM code 0x%08x"
#define NO_COMMCTRL_MSG           "failed to initialize %s; OS error code %d"

#define COMCTL32_FULL_PART        0
#define COMCTL32_NUMBER_PART      1
#define COMCTL32_OS_PART          2

const char *comctl32VersionPart(DWORD id, DWORD type)
{
    const char *part;
    switch ( id )
    {
        case COMCTL32_4_0 :
            if ( type == COMCTL32_NUMBER_PART )
            {
                part = "4.0";
            }
            else if ( type == COMCTL32_NUMBER_PART )
            {
                part = "W95 / NT4";
            }
            else
            {
                part = "comctl32.dll version 4.0 (W95 / NT4)";
            }
            break;

        case COMCTL32_4_7 :
            if ( type == COMCTL32_NUMBER_PART )
            {
                part = "4.7";
            }
            else if ( type == COMCTL32_OS_PART )
            {
                part = "IE 3.x";
            }
            else
            {
                part = "comctl32.dll version 4.7 (IE 3.x)";
            }
            break;

        case COMCTL32_4_71 :
            if ( type == COMCTL32_NUMBER_PART )
            {
                part = "4.71";
            }
            else if ( type == COMCTL32_OS_PART )
            {
                part = "IE 4.0";
            }
            else
            {
                part = "comctl32.dll version 4.71 (IE 4.0)";
            }
            break;

        case COMCTL32_4_72 :
            if ( type == COMCTL32_NUMBER_PART )
            {
                part = "4.72";
            }
            else if ( type == COMCTL32_OS_PART )
            {
                part = "W98 / IE 4.01";
            }
            else
            {
                part = "comctl32.dll version 4.72 (W98 / IE 4.01)";
            }
            break;

        case COMCTL32_5_8 :
            if ( type == COMCTL32_NUMBER_PART )
            {
                part = "5.8";
            }
            else if ( type == COMCTL32_OS_PART )
            {
                part = "IE 5";
            }
            else
            {
                part = "comctl32.dll version 5.8 (IE 5)";
            }
            break;

        case COMCTL32_5_81 :
            if ( type == COMCTL32_NUMBER_PART )
            {
                part = "5.81";
            }
            else if ( type == COMCTL32_OS_PART )
            {
                part = "W2K / ME";
            }
            else
            {
                part = "comctl32.dll version 5.81 (W2K / ME)";
            }
            break;

        case COMCTL32_6_0 :
            if ( type == COMCTL32_NUMBER_PART )
            {
                part = "6.0";
            }
            else if ( type == COMCTL32_OS_PART )
            {
                part = "XP";
            }
            else
            {
                part = "comctl32.dll version 6.0 (XP)";
            }
            break;

        default :
            part = "Unknown";
            break;
    }
    return part;
}

inline const char *comctl32VersionName(DWORD id)
{
    return comctl32VersionPart(id, COMCTL32_FULL_PART);
}


POINTER rxGetPointerAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name)
{
    CSTRING value = "";
    RexxObjectPtr rxString = context->SendMessage0(obj, name);
    if ( rxString != NULLOBJECT )
    {
        value = context->ObjectToStringValue(rxString);
    }
    return string2pointer(value);
}

CSTRING rxGetStringAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name)
{
    CSTRING value = NULL;
    RexxObjectPtr rxString = context->SendMessage0(obj, name);
    if ( rxString != NULLOBJECT )
    {
        value = context->ObjectToStringValue(rxString);
    }
    return value;
}

DIALOGADMIN *rxGetDlgAdm(RexxMethodContext *context, RexxObjectPtr dlg)
{
    DIALOGADMIN *adm = (DIALOGADMIN *)rxGetPointerAttribute(context, dlg, "ADM");
    if ( adm == NULL )
    {
         // Want this message: Could not retrieve the "value" information for "object"
         // similar to old 98.921

        TCHAR buf[128];
        RexxObjectPtr name = context->SendMessage0(dlg, "OBJECTNAME");
        _snprintf(buf, sizeof(buf), "Could not retrieve the dialog administration block information for %s",
                  context->ObjectToStringValue(name));

        context->RaiseException1(Rexx_Error_Execution_user_defined, context->String(buf));
    }
    return adm;
}

inline HWND rxGetWindowHandle(RexxMethodContext * context, RexxObjectPtr self)
{
    return (HWND)rxGetPointerAttribute(context, self, "HWND");
}

void systemServiceException(RexxMethodContext *context, char *msg)
{
    context->RaiseException1(Rexx_Error_System_service_user_defined, context->String(msg));
}

void systemServiceException(RexxMethodContext *context, char *msg, const char *sub)
{
    if ( sub != NULL )
    {
        TCHAR buffer[128];
        _snprintf(buffer, sizeof(buffer), msg, sub);
        systemServiceException(context, buffer);
    }
    else
    {
        systemServiceException(context, msg);
    }
}

void systemServiceExceptionCode(RexxMethodContext *context, const char *msg, const char *arg1)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), msg, arg1, GetLastError());
    systemServiceException(context, buffer);
}

void systemServiceExceptionComCode(RexxMethodContext *context, const char *msg, const char *arg1, HRESULT hr)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), msg, arg1, hr);
    systemServiceException(context, buffer);
}

void outOfMemoryException(RexxMethodContext *c)
{
    systemServiceException(c, "Failed to allocate memory");
}

void userDefinedMsgException(RexxMethodContext *c, CSTRING msg)
{
    c->RaiseException1(Rexx_Error_Incorrect_method_user_defined, c->String(msg));
}

void userDefinedMsgException(RexxMethodContext *c, int pos, CSTRING msg)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Method argument %d %s", pos, msg);
    userDefinedMsgException(c, buffer);
}

void *wrongClassException(RexxMethodContext *c, int pos, const char *n)
{
    c->RaiseException2(Rexx_Error_Incorrect_method_noclass, c->WholeNumber(pos), c->String(n));
    return NULL;
}

void invalidTypeException(RexxMethodContext *c, int pos, const char *type)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Method argument %d is not a valid %s", pos, type);
    userDefinedMsgException(c, buffer);
}

void invalidImageException(RexxMethodContext *c, int pos, CSTRING type, CSTRING actual)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Method argument %d must be a %s image; found %s", pos, type, actual);
    userDefinedMsgException(c, buffer);
}

void wrongObjInArrayException(RexxMethodContext *c, int argPos, size_t index, CSTRING obj)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Method argument %d is an array and index %d is not a %s", argPos, index, obj);
    userDefinedMsgException(c, buffer);
}

void wrongObjInDirectoryException(RexxMethodContext *c, int argPos, CSTRING index, CSTRING needed, RexxObjectPtr actual)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer),
              "Index, %s, of method argument %d must be %s; found \"%s\"",
              index, argPos, needed, c->ObjectToStringValue(actual));
    userDefinedMsgException(c, buffer);
}

void emptyArrayException(RexxMethodContext *c, int argPos)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Method argument %d must be a non-empty array", argPos);
    userDefinedMsgException(c, buffer);
}

void nullObjectException(RexxMethodContext *c, CSTRING name, int pos)
{
    TCHAR buffer[256];
    if ( pos == 0 )
    {
        _snprintf(buffer, sizeof(buffer), "The %s object must not be null", name);
    }
    else
    {
        _snprintf(buffer, sizeof(buffer), "Method argument %d, the %s object, must not be null", pos, name);
    }
    userDefinedMsgException(c, buffer);
}

void nullObjectException(RexxMethodContext *c, CSTRING name)
{
    nullObjectException(c, name, 0);
}

void nullPointerException(RexxMethodContext *c, int pos)
{
    c->RaiseException1(Rexx_Error_Incorrect_method_null, c->WholeNumber(pos));
}

void wrongRangeException(RexxMethodContext *c, int pos, int min, int max, RexxObjectPtr actual)
{
    c->RaiseException(Rexx_Error_Invalid_argument_range,
                      c->ArrayOfFour(c->WholeNumber(pos), c->WholeNumber(min), c->WholeNumber(max), actual));
}

void wrongRangeException(RexxMethodContext *c, int pos, int min, int max, int actual)
{
    wrongRangeException(c, pos, min, max, c->WholeNumber(actual));
}

void wrongArgValueException(RexxMethodContext *c, int pos, const char *list, RexxObjectPtr actual)
{
    c->RaiseException(Rexx_Error_Incorrect_method_list,
                      c->ArrayOfThree(c->WholeNumber(pos), c->String(list), actual));
}

void wrongArgValueException(RexxMethodContext *c, int pos, const char *list, const char *actual)
{
    wrongArgValueException(c, pos, list, c->String(actual));
}

void wrongWindowStyleException(RexxMethodContext *c, const char *obj, const char *style)
{
    char msg[128];
    _snprintf(msg, sizeof(msg), "This %s does not have the %s style", obj, style);
    userDefinedMsgException(c, msg);
}

bool requiredComCtl32Version(RexxMethodContext *context, const char *methodName, DWORD minimum)
{
    if ( ComCtl32Version < minimum )
    {
        char msg[256];
        _snprintf(msg, sizeof(msg), "The %s() method requires %s or later", methodName, comctl32VersionName(minimum));
        context->RaiseException1(Rexx_Error_System_service_user_defined, context->String(msg));
        return false;
    }
    return true;
}

bool requiredClass(RexxMethodContext *c, RexxObjectPtr obj, const char *name, int pos)
{
    if ( obj == NULLOBJECT || ! c->IsOfType(obj, name) )
    {
        wrongClassException(c, pos, name);
        return false;
    }
    return true;
}

/**
 * Return the number of existing arguments in an ooRexx method invocation.  In
 * others words, it is intended to count neither the omitted args in the ooRexx
 * method, nor the pseudo-arguments to the native API function, like OSELF,
 * CSELF, etc..
 *
 * @param context  The method context pointer.
 *
 * @return The count of existing arguments in an ooRexx method invocation.
 */
size_t rxArgCount(RexxMethodContext * context)
{
    RexxObjectPtr items = context->SendMessage0(context->GetArguments(), "ITEMS");

    wholenumber_t count;
    context->ObjectToWholeNumber(items, &count);
    return (size_t)count;
}

bool rxStr2Number(RexxMethodContext *c, CSTRING str, uint64_t *number, int pos)
{
    char *end;
    *number = _strtoui64(str, &end, 0);
    if ( (end - str != strlen(str)) || errno == EINVAL || *number == _UI64_MAX )
    {
        invalidTypeException(c, pos, "number");
        return false;
    }
    return true;

}

RexxClassObject rxGetContextClass(RexxMethodContext *c, CSTRING name)
{
    RexxClassObject theClass = c->FindContextClass(name);
    if ( theClass == NULL )
    {
        c->RaiseException1(Rexx_Error_Execution_noclass, c->String(name));
    }
    return theClass;
}

PRECT rxGetRect(RexxMethodContext *context, RexxObjectPtr r, int argPos)
{
    if ( requiredClass(context, r, "Rect", argPos) )
    {
        return (PRECT)context->ObjectToCSelf(r);
    }
    return NULL;
}

RexxObjectPtr rxNewRect(RexxMethodContext *context, long l, long t, long r, long b)
{
    RexxObjectPtr rect = NULL;

    RexxClassObject RectClass = rxGetContextClass(context, "RECT");
    if ( RectClass != NULL )
    {
        RexxArrayObject args = context->NewArray(4);
        context->ArrayAppend(args, context->WholeNumber(l));
        context->ArrayAppend(args, context->WholeNumber(t));
        context->ArrayAppend(args, context->WholeNumber(r));
        context->ArrayAppend(args, context->WholeNumber(b));

        rect = context->SendMessage(RectClass, "NEW", args);
    }
    return rect;
}

PPOINT rxGetPoint(RexxMethodContext *context, RexxObjectPtr p, int argPos)
{
    if ( requiredClass(context, p, "Point", argPos) )
    {
        return (PPOINT)context->ObjectToCSelf(p);
    }
    return NULL;
}

RexxObjectPtr rxNewPoint(RexxMethodContext *c, long x, long y)
{
    RexxObjectPtr point = NULL;
    RexxClassObject PointClass = rxGetContextClass(c, "POINT");
    if ( PointClass != NULL )
    {
        point = c->SendMessage2(PointClass, "NEW", c->WholeNumber(x), c->WholeNumber(y));
    }
    return point;
}

PSIZE rxGetSize(RexxMethodContext *context, RexxObjectPtr s, int argPos)
{
    if ( requiredClass(context, s, "Size", argPos) )
    {
        return (PSIZE)context->ObjectToCSelf(s);
    }
    return NULL;
}

RexxObjectPtr rxNewSize(RexxMethodContext *c, long cx, long cy)
{
    RexxObjectPtr size = NULL;
    RexxClassObject SizeClass = rxGetContextClass(c, "SIZE");
    if ( SizeClass != NULL )
    {
        size = c->SendMessage2(SizeClass, "NEW", c->WholeNumber(cx), c->WholeNumber(cy));
    }
    return size;
}

/**
 * Sets an object variable value and returns the existing value.  With the
 * caveat that if the object variable did not have a value set, .nil is
 * returned.
 *
 * @param c        The method context we are operating in.
 * @param varName  The object variable's name.
 * @param val      The value to set.
 *
 * @return The previous value of the object variable, if it was set, otherwise
 *         .nil.
 */
RexxObjectPtr rxSetObjVar(RexxMethodContext *c, CSTRING varName, RexxObjectPtr val)
{
    RexxObjectPtr result = c->GetObjectVariable(varName);
    if ( result == NULLOBJECT )
    {
        result = c->Nil();
    }
    c->SetObjectVariable(varName, val);

    return result;
}

inline bool hasStyle(HWND hwnd, LONG style)
{
    if ( (GetWindowLong(hwnd, GWL_STYLE) & style) || (GetWindowLong(hwnd, GWL_EXSTYLE) & style) )
    {
        return true;
    }
    return false;
}

/**
 * Correctly converts from a device coordinate (pixel) to a dialog unit
 * coordinate, for any dialog.
 *
 * MapDialogRect() correctly converts from dialog units to pixels for any
 * dialog.  But, there is no conversion the other way, from pixels to dialog
 * units.
 *
 * MSDN gives these formulas to convert from pixel to dialog unit:
 *
 * templateunitX = MulDiv(pixelX, 4, baseUnitX);
 * templateunitY = MulDiv(pixelY, 8, baseUnitY);
 *
 * Now, you just need to get the correct dialog base unit.
 *
 * GetDialogBaseUnits() always assumes the font is the system font.  If the
 * dialog uses any other font, the base units returned will be incorrect.
 *
 * MSDN, again, has two methods for calculating the correct base units for any
 * font.  This way is the simplest, but it requires the window handle to the
 * dialog.
 *
 * Rect rect( 0, 0, 4, 8 );
 * MapDialogRect( &rc );
 * int baseUnitY = rc.bottom;
 * int baseUnitX = rc.right;
 *
 * @param hwnd   Window handle of the dialog.  If this is not a dialog window
 *               handle, this method will fail.
 *
 * @param point  Pointer to a POINT struct.  Not that a SIZE struct and a POINT
 *               struct are binary equivalents.  They both have two fields, each
 *               of which is a long.  Only the field names differ, cx and cy for
 *               a SIZE and x and y for a POINT.  Therefore you can cast a
 *               SIZE pointer to a POINT pointer.
 *
 * @return true on success, false otherwise.
 *
 * Dialog class: #32770
 */
bool screenToDlgUnit(HWND hwnd, POINT *point)
{
    RECT r = {0, 0, 4, 8};

    if ( MapDialogRect(hwnd, &r) )
    {
        point->x = MulDiv(point->x, 4, r.right);
        point->y = MulDiv(point->y, 8, r.bottom);
        return true;
    }
    return false;
}

/**
 * Given a device context with the correct font already selected into it,
 * correctly converts from a device coordinate (pixel) to a dialog unit
 * coordinate.  The correct font means, the font actually used by the dialog.
 *
 * See screenToDlgUnit(HWND, POINT *) for a discussion of this
 * conversion.
 *
 * @param hdc    Handle to a device context with the dialog's font selected into
 *               it.
 *
 * @param point  Pointer to a POINT struct.  Not that a SIZE struct and a POINT
 *               struct are binary equivalents.  They both have two fields, each
 *               of which is a long.  Only the field names differ, cx and cy for
 *               a SIZE and x and y for a POINT.  Therefore you can cast a
 *               SIZE pointer to a POINT pointer.
 *
 * @return true on success, false otherwise.
 *
 */
void screenToDlgUnit(HDC hdc, POINT *point)
{
    TEXTMETRIC tm;
    SIZE size;
    GetTextMetrics(hdc, &tm);
    int baseUnitY = tm.tmHeight;

    GetTextExtentPoint32(hdc, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size);
    int baseUnitX = (size.cx / 26 + 1) / 2;

    point->x = MulDiv(point->x, 4, baseUnitX);
    point->y = MulDiv(point->y, 8, baseUnitY);
}

/**
 * Returns an upper-cased copy of the string with all space and ampersand
 * characters removed.
 *
 * @param str   The string to copy and upper case.
 *
 * @return      A pointer to a new string, or null on a memory allocation
 *              failure.
 *
 * @note        The caller is responsible for freeing the returned string.
 */
char *strdupupr_nospace(const char *str)
{
    char *retStr = NULL;
    if ( str )
    {
        size_t l = strlen(str);
        retStr = (char *)malloc(l + 1);
        if ( retStr )
        {
            char *p;
            for ( p = retStr; *str; ++str )
            {
                if ( *str == ' ' || *str == '&' )
                {
                    continue;
                }
                if ( ('a' <= *str) && (*str <= 'z') )
                {
                    *p++ = *str - ('a' - 'A');
                }
                else
                {
                    *p++ = *str;
                }
            }
            *p = '\0';
        }
    }
    return retStr;
}

/**
 * Returns an upper-cased copy of the string.
 *
 * @param str   The string to copy and upper case.
 *
 * @return      A pointer to a new string, or null on a memory allocation
 *              failure.
 *
 * The caller is responsible for freeing the returned string.
 */
char *strdupupr(const char *str)
{
    char *retStr = NULL;
    if ( str )
    {
        size_t l = strlen(str);
        retStr = (char *)malloc(l + 1);
        if ( retStr )
        {
            char *p;
            for ( p = retStr; *str; ++str )
            {
                if ( ('a' <= *str) && (*str <= 'z') )
                {
                    *p++ = *str - ('a' - 'A');
                }
                else
                {
                    *p++ = *str;
                }
            }
            *p = '\0';
        }
    }
    return retStr;
}

DWORD oodGetSysErrCode(RexxMethodContext *c)
{
    uint32_t code = 0;

    RexxDirectoryObject local = c->GetLocalEnvironment();
    if ( local != NULLOBJECT )
    {
        RexxObjectPtr rxCode = c->DirectoryAt(local, "SYSTEMERRORCODE");
        if ( rxCode != NULLOBJECT )
        {
            c->UnsignedInt32(rxCode, &code);
        }
    }
    return (DWORD)code;
}

void oodSetSysErrCode(RexxMethodContext *context, DWORD code)
{
    RexxDirectoryObject local = context->GetLocalEnvironment();
    if ( local != NULLOBJECT )
    {
        context->DirectoryPut(local, context->WholeNumberToObject(code), "SYSTEMERRORCODE");
    }
}

inline void oodSetSysErrCode(RexxMethodContext *context)
{
    oodSetSysErrCode(context, GetLastError());
}

/**
 * Resolves a resource ID used in a native API method call to its numeric value.
 * The resource ID may be numeric or symbolic.  An exception is raised if the ID
 * can not be resolved.
 *
 * @param context    Method context for the method call.
 * @param dlg        ooDialog dialog object. <Assumed>
 * @param id         Resource ID.
 * @param argPosDlg  Arg position of the assumed dialog object.  Used for raised
 *                   exceptions.
 * @param argPosID   Arg position of the ID, used for raised exceptions.
 *
 * @return int       The resolved numeric ID, or OOD_ID_EXCEPTION
 *
 * Note:  This function raises an execption if the ID does not resolve.
 * Therefore, it should not be used for existing ooDialog methods that used to
 * return -1 when the ID was not resolved.  Will need to see what strategy can
 * be implemented going forward.
 */
int oodResolveSymbolicID(RexxMethodContext *context, RexxObjectPtr dlg, RexxObjectPtr id,
                         int argPosDlg, int argPosID)
{
    if ( ! requiredClass(context, dlg, "ResourceUtils", argPosDlg) )
    {
        return OOD_ID_EXCEPTION;
    }

    int result = -1;
    char *symbol = NULL;

    if ( ! context->ObjectToInt32(id, &result) )
    {
        RexxDirectoryObject constDir = (RexxDirectoryObject)context->SendMessage0(dlg, "CONSTDIR");
        if ( constDir != NULLOBJECT )
        {
            /* The original ooDialog code uses:
             *   self~ConstDir[id~space(0)~translate]
             * Why they allowed a space in a symbolic ID, I don't understand.
             * But, I guess we need to preserve that.
             */

            symbol = strdupupr_nospace(context->ObjectToStringValue(id));
            if ( symbol == NULL )
            {
                outOfMemoryException(context);
                return OOD_ID_EXCEPTION;
            }

            RexxObjectPtr item = context->DirectoryAt(constDir, symbol);
            if ( item != NULLOBJECT )
            {
                 context->ObjectToInt32(item, &result);
            }
        }
    }

    safeFree(symbol);

    if ( result < 1 )
    {
        wrongArgValueException(context, argPosID, "a valid numeric ID or a valid symbloic ID" , id);
        return OOD_ID_EXCEPTION;
    }

    return result;
}

RexxObjectPtr oodSetImageAttribute(RexxMethodContext *c, CSTRING varName, RexxObjectPtr image, HWND hwnd,
                                   HANDLE hOldImage, uint8_t type, oodControl_t ctrl)
{
    RexxObjectPtr result = c->GetObjectVariable(varName);
    if ( result == NULLOBJECT )
    {
        result = c->Nil();
    }
    c->SetObjectVariable(varName, image);

    // It could be that the existing image was set from a resource DLL.  In
    // which case we need to create an .Image object.
    if ( result == c->Nil() && hOldImage != NULL )
    {
        result = rxNewImageFromControl(c, hwnd, hOldImage, type, ctrl);
    }
    return result;
}

RexxObjectPtr oodGetImageAttribute(RexxMethodContext *c, OSELF self, CSTRING varName,
                                   UINT msg, WPARAM wParam, uint8_t type, oodControl_t ctrl)
{
    // If we already have an image in the object variable, just use it.
    RexxObjectPtr result = c->GetObjectVariable(varName);
    if ( result == NULLOBJECT )
    {
        HWND hwnd = rxGetWindowHandle(c, self);
        HANDLE hImage = (HANDLE)SendMessage(hwnd, msg, wParam, 0);

        if ( hImage == NULL )
        {
            result = c->Nil();
        }
        else
        {
            // Create a new .Image object from the image handle.
            result = rxNewImageFromControl(c, hwnd, hImage, type, ctrl);
        }

        // Set the result in the object variable.  If there is a next time we
        // can retrieve it easily.
        c->SetObjectVariable(varName, result);
    }
    return result;
}

/**
 * Uses GetTextExtentPoint32() to get the size needed for a string using the
 * specified font and device context.
 *
 * @param font   The font being used for the string.
 * @param hdc    The device context to use.
 * @param text   The string.
 * @param size   Pointer to a SIZE struct used to return the size.
 *
 * @return True if  GetTextExtentPoint32() succeeds, otherwise false.
 *
 * @note   GetTextExtentPoint32() sets last error and SelectObject() does not.
 *         Therefore if this function fails, GetLastError() will return the
 *         correct error code for the failed GetTextExtentPoint32().
 */
bool getTextExtent(HFONT font, HDC hdc, CSTRING text, SIZE *size)
{
    bool success = true;
    HFONT hOldFont = (HFONT)SelectObject(hdc, font);

    if ( GetTextExtentPoint32(hdc, text, (int)strlen(text), size) == 0 )
    {
        success = false;
    }
    SelectObject(hdc, hOldFont);
    return success;
}

HFONT createFontFromName(HDC hdc, CSTRING name, uint32_t size)
{
    LOGFONT lf={0};

    strcpy(lf.lfFaceName, name);
    lf.lfHeight = -MulDiv(size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    return CreateFontIndirect(&lf);
}

bool textSizeIndirect(RexxMethodContext *context, CSTRING text, CSTRING fontName, uint32_t fontSize,
                      SIZE *size, HWND hwnd)
{
    bool success = true;

    // If hwnd is null, GetDC() returns a device context for the whole screen,
    // and that suites our purpose here.
    HDC hdc = GetDC(hwnd);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "GetDC");
        return false;
    }

    HFONT font = createFontFromName(hdc, fontName, fontSize);
    if ( font == NULL )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "CreateFontIndirect");
        ReleaseDC(hwnd, hdc);
        return false;
    }

    if ( ! getTextExtent(font, hdc, text, size) )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "GetTextExtentPoint32");
        success = false;
    }

    DeleteObject(font);
    if ( ReleaseDC(hwnd, hdc) == 0 )
    {
        printf("RelaseDC() failed\n");
    }

    return success;
}

bool textSizeFromWindow(RexxMethodContext *context, CSTRING text, SIZE *size, HWND hwnd)
{
    HDC hdc = GetDC(hwnd);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "GetDC");
        return false;
    }

    // Dialogs and controls need to have been issued a WM_SETFONT or else they
    // return null here.  If null, they are using the stock system font.
    HFONT font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    if ( font == NULL )
    {
        font = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    bool success = true;
    if ( ! getTextExtent(font, hdc, text, size) )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "GetTextExtentPoint32");
        success = false;
    }

    ReleaseDC(hwnd, hdc);
    return success;
}

RexxObjectPtr getTextSize(RexxMethodContext *context, CSTRING text, CSTRING fontName, uint32_t fontSize,
                          HWND hwndFontSrc, RexxObjectPtr dlgObj)
{
    // hwndDlg can be null if this is happening before the real dialog is created.
    HWND hwndDlg = rxGetWindowHandle(context, dlgObj);

    // We may not have a window handle, but using null is okay.
    HWND hwndForDC = (hwndFontSrc != NULL ? hwndFontSrc : hwndDlg);

    SIZE textSize = {0};

    if ( fontName != NULL )
    {
        if ( ! textSizeIndirect(context, text, fontName, fontSize, &textSize, hwndForDC) )
        {
            goto error_out;
        }
    }
    else if ( hwndFontSrc != NULL )
    {
        if ( ! textSizeFromWindow(context, text, &textSize, hwndFontSrc) )
        {
            goto error_out;
        }
    }

    // Even if we use a font other than the dialog font to calculate the text
    // size, we always have to get the dialog font and select it into a HDC to
    // correctly calculate the dialog units.
    HDC hdc = GetDC(hwndForDC);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "GetDC");
        goto error_out;
    }

    HFONT dlgFont = NULL;
    bool createdFont = false;

    if ( hwndDlg == NULL )
    {
        fontSize = 0;
        fontName = rxGetStringAttribute(context, dlgObj, "FONTNAME");

        RexxObjectPtr rxSize = context->SendMessage0(dlgObj, "FONTSIZE");
        if ( rxSize != NULLOBJECT )
        {
            context->ObjectToUnsignedInt32(rxSize, &fontSize);
        }

        if ( fontName != NULL && fontSize != 0 )
        {
            dlgFont = createFontFromName(hdc, fontName, fontSize);
            if ( dlgFont != NULL )
            {
                createdFont = true;
            }
        }
    }
    else
    {
        dlgFont = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0);
    }

    // If dlgFont is null, then, (almost for sure,) the dialog will be using the
    // default system font.  The exception to this is if the user calls the
    // getTextSizeDlg() method before the create() method, and then defines a
    // custom font in create().  The docs tell the user not to do that, but
    // there is nothing to do about it if they do.
    if ( dlgFont == NULL )
    {
        dlgFont = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    HFONT hOldFont = (HFONT)SelectObject(hdc, dlgFont);
    if ( textSize.cx == 0 )
    {
        GetTextExtentPoint32(hdc, text, (int)strlen(text), &textSize);
    }

    screenToDlgUnit(hdc, (POINT *)&textSize);

    SelectObject(hdc, hOldFont);
    ReleaseDC(hwndForDC, hdc);

    if ( createdFont )
    {
        DeleteObject(dlgFont);
    }

    return rxNewSize(context, textSize.cx, textSize.cy);

error_out:
    return NULLOBJECT;
}


/**
 *  Methods for the .WindowBase mixin class.
 */
#define WINDOWBASE_CLASS       "WindowBase"

RexxMethod1(uint32_t, wb_getStyleRaw, OSELF, self)
{
    return GetWindowLong(rxGetWindowHandle(context, self), GWL_STYLE);
}

RexxMethod1(uint32_t, wb_getExStyleRaw, OSELF, self)
{
    return GetWindowLong(rxGetWindowHandle(context, self), GWL_EXSTYLE);
}


/**
 *  Methods for the .PlainBaseDialog class.
 */
#define PLAINBASEDIALOG_CLASS  "PlainBaseDialog"

#define DEFAULT_FONTNAME       "MS Shell Dlg"
#define DEFAULT_FONTSIZE       8

RexxMethod0(RexxObjectPtr, pbdlg_init_cls)
{
    context->SetObjectVariable("FONTNAME", context->String(DEFAULT_FONTNAME));
    context->SetObjectVariable("FONTSIZE", context->WholeNumber(DEFAULT_FONTSIZE));
    return NULLOBJECT;
}

RexxMethod2(RexxObjectPtr, pbdlg_setDefaultFont_cls, CSTRING, fontName, uint32_t, fontSize)
{
    context->SetObjectVariable("FONTNAME", context->String(fontName));
    context->SetObjectVariable("FONTSIZE", context->WholeNumber(fontSize));
    return NULLOBJECT;
}

RexxMethod0(RexxObjectPtr, pbdlg_getFontName_cls)
{
    return context->GetObjectVariable("FONTNAME");
}
RexxMethod0(RexxObjectPtr, pbdlg_getFontSize_cls)
{
    return context->GetObjectVariable("FONTSIZE");
}

/** PlainBaseDialog::getTextSizeDlg()
 *
 *  Gets the size (width and height) in dialog units for any given string, for
 *  the font specified.
 *
 *  @param  text         The string whose size is needed.
 *
 *  @param  fontName     Optional. If specified, use this font to calculate the
 *                       size.
 *
 *  @param  fontSize     Optional. If specified, use this font size with
 *                       fontName to calculate the size.  The default if omitted
 *                       is 8.  This arg is ignored if fontName is omitted.
 *
 *  @param  hwndFontSrc  Optional. Use this window's font to calculate the size.
 *                       This arg is always ignored if fontName is specified.
 *
 */
RexxMethod5(RexxObjectPtr, pbdlg_getTextSizeDlg, CSTRING, text, OPTIONAL_CSTRING, fontName,
            OPTIONAL_uint32_t, fontSize, OPTIONAL_POINTERSTRING, hwndFontSrc, OSELF, self)
{
    HWND hwndSrc = NULL;
    if ( argumentExists(2) )
    {
        if ( argumentOmitted(3) )
        {
            fontSize = DEFAULT_FONTSIZE;
        }
    }
    else if ( argumentExists(4) )
    {
        if ( hwndFontSrc == NULL )
        {
            nullObjectException(context, "window handle", 4);
            goto error_out;
        }
        hwndSrc = (HWND)hwndFontSrc;
    }
    return getTextSize(context, text, fontName, fontSize, hwndSrc, self);

error_out:
    return NULLOBJECT;
}


/**
 *  Methods for the .ResDialog class.
 */
#define RESDIALOG_CLASS        "ResDialog"


RexxMethod1(RexxObjectPtr, resdlg_setFontAttrib_pvt, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    HFONT font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    if ( font == NULL )
    {
        font = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    HDC hdc = GetDC(hwnd);
    if ( hdc )
    {
        HFONT oldFont = (HFONT)SelectObject(hdc, font);

        char fontName[64];
        TEXTMETRIC tm;

        GetTextMetrics(hdc, &tm);
        GetTextFace(hdc, sizeof(fontName), fontName);

        long fontSize = MulDiv((tm.tmHeight - tm.tmInternalLeading), 72, GetDeviceCaps(hdc, LOGPIXELSY));

        context->SendMessage1(self, "FONTNAME=", context->String(fontName));
        context->SendMessage1(self, "FONTSIZE=", context->WholeNumber(fontSize));

        SelectObject(hdc, oldFont);
        ReleaseDC(hwnd, hdc);
    }
    return NULLOBJECT;
}


/**
 *  Methods for the .WindowExtensions class.
 */
#define WINDOWEXTENSIONS_CLASS        "WindowExtensions"


/** WindowExtensions::getTextSizeScreen()
 *
 *  Gets the size, width and height, in pixels, needed to display a string in a
 *  specific font.
 *
 *  @param text      The text to calculate the size of.  If this is the only
 *                   argument then the font of this object is used for the
 *                   calculation.
 *
 *  @param type      Optional.  If the text arg is not the only argument, then
 *                   type is required.  It signals what fontSrc is.  The allowed
 *                   types are:
 *
 *                   Indirect -> fontSrc is a font name and fontSize is the size
 *                   of the font.  The calculation is done indirectly by
 *                   temporarily obtaining a logical font.
 *
 *                   DC -> fontSrc is a handle to a device context.  The correct
 *                   font for the calculation must already be selected into this
 *                   device context.  fontSize is ignored.
 *
 *                   Font -> fontSrc is a handle to a font.  fontSize is
 *                   ignored.
 *
 *                   Only the first letter of type is needed and case is not
 *                   significant.
 *
 *  @param fontSrc   Optional.  An object to use for calculating the size of
 *                   text.  The type argument determines how this object is
 *                   interpreted.
 *
 *  @param fontSize  Optional.  The size of the font.  This argument is always
 *                   ignored unless the type argument is Indirect.  If type is
 *                   Indirect and this argument is omitted then the defualt font
 *                   size is used.  (Currently the default size is 8.)
 *
 *  @return  A .Size object containg the width and height for the text in
 *           pixels.
 */
RexxMethod5(RexxObjectPtr, winex_getTextSizeScreen, CSTRING, text, OPTIONAL_CSTRING, type,
            OPTIONAL_CSTRING, fontSrc, OPTIONAL_uint32_t, fontSize, OSELF, self)
{
    SIZE size = {0};

    HWND hwnd = rxGetWindowHandle(context, self);
    if ( hwnd == NULL )
    {
        nullObjectException(context, "window handle");
        goto error_out;
    }

    if ( rxArgCount(context) == 1 )
    {
        if ( ! textSizeFromWindow(context, text, &size, hwnd) )
        {
            goto error_out;
        }
    }
    else if ( argumentOmitted(2) )
    {
        context->RaiseException1(Rexx_Error_Incorrect_method_noarg, context->WholeNumber(2));
        goto error_out;
    }
    else
    {
        if ( argumentOmitted(3) )
        {
            context->RaiseException1(Rexx_Error_Incorrect_method_noarg, context->WholeNumber(3));
            goto error_out;
        }

        char m = toupper(*type);
        if ( m == 'I' )
        {
            if ( argumentOmitted(4) )
            {
                fontSize = DEFAULT_FONTSIZE;
            }
            if ( ! textSizeIndirect(context, text, fontSrc, fontSize, &size, hwnd) )
            {
                goto error_out;
            }
        }
        else if ( m == 'D' )
        {
            HDC hdc = (HDC)string2pointer(fontSrc);
            if ( hdc == NULL )
            {
                invalidTypeException(context, 3, "handle to a device context");
            }
            GetTextExtentPoint32(hdc, text, (int)strlen(text), &size);
        }
        else if ( m == 'F' )
        {
            HFONT hFont = (HFONT)string2pointer(fontSrc);
            if ( hFont == NULL )
            {
                invalidTypeException(context, 3, "handle to a font");
            }

            HDC hdc = GetDC(hwnd);
            if ( hdc == NULL )
            {
                systemServiceExceptionCode(context, API_FAILED_MSG, "GetDC");
                goto error_out;
            }

            bool success = true;
            if ( ! getTextExtent(hFont, hdc, text, &size) )
            {
                systemServiceExceptionCode(context, API_FAILED_MSG, "GetTextExtentPoint32");
                success = false;
            }

            ReleaseDC(hwnd, hdc);
            if ( ! success )
            {
                goto error_out;
            }
        }
        else
        {
            context->RaiseException2(Rexx_Error_Incorrect_method_option, context->String("I, D, F"),
                                     context->String(type));
            goto error_out;
        }
    }

    return rxNewSize(context, size.cx, size.cy);

error_out:
    return NULLOBJECT;
}

/** WindowExtensions::getFont()
 *
 *  Returns the font in use for the dialog or dialog control.
 *
 *  @note  If the window returns NULL for the font, then it has not been set
 *         through a WM_SETFONT message.  In this case it is using the stock
 *         system font. Rather than return 0, we return the stock system font to
 *         the ooDialog programmer.
 *
 */
RexxMethod1(POINTERSTRING, winex_getFont, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    if ( hwnd == NULL )
    {
        nullObjectException(context, "window handle");
        goto error_out;
    }

    HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    if ( hFont == NULL )
    {
        hFont = (HFONT)GetStockObject(SYSTEM_FONT);
    }
    return hFont;

error_out:
    return NULLOBJECT;
}

/** WindowExtensions::setFont()
 *
 *  Sets the font used for text in a dialog or dialog control.
 *
 *  @param font  Handle to the new font.
 *
 *  @param redraw  Optional. If true, the window will redraw itself. (According
 *                 to MSDN.) The defualt if this argument is omitted is true.
 *
 *  @return 0, always. The WM_SETFONT message does not return a value.
 */
RexxMethod3(int, winex_setFont, POINTERSTRING, font, OPTIONAL_logical_t, redraw, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    if ( hwnd == NULL )
    {
        nullObjectException(context, "window handle");
    }
    else
    {
        if ( argumentOmitted(2) )
        {
            redraw = TRUE;
        }
        SendMessage(hwnd, WM_SETFONT, (WPARAM)font, redraw);
    }
    return 0;
}

bool rxLogicalFromDirectory(RexxMethodContext *context, RexxDirectoryObject d, CSTRING index,
                            BOOL *logical, int argPos)
{
    logical_t value;
    RexxObjectPtr obj = context->DirectoryAt(d, index);
    if ( obj != NULLOBJECT )
    {
        if ( ! context->Logical(obj, &value) )
        {
            wrongObjInDirectoryException(context, argPos, index, "a logical", obj);
            return false;
        }
        *logical = (BOOL)value;
    }
    return true;
}

bool rxNumberFromDirectory(RexxMethodContext *context, RexxDirectoryObject d, CSTRING index,
                           DWORD *number, int argPos)
{
    DWORD value;
    RexxObjectPtr obj = context->DirectoryAt(d, index);
    if ( obj != NULLOBJECT )
    {
        if ( ! context->UnsignedInt32(obj, (uint32_t*)&value) )
        {
            wrongObjInDirectoryException(context, argPos, index, "a positive whole number", obj);
            return false;
        }
        *number = value;
    }
    return true;
}

extern int getWeight(CSTRING opts);

/** WindowExtensions::createFont()
 *
 *  Creates a logical font with the specified characteristics.
 *
 *  This implementation is broken.  It is the original ooDialog implementation.
 *  It incorrectly maps the point size to the font height and it defaults the
 *  average character width to the point size.
 *
 *  It is maintained "as is" for program compatibility.
 *
 *  @param fontName  Optional.  The typeface name.  The default is System.
 *
 *  @param fSize     Optional.  The point size of the font.  The default is 10.
 *
 *  @param fontStyle Optional.  A string containing 0 or more of the style
 *                              keywords separated by blanks. The default is a
 *                              normal font style.
 *
 *  @param fWidth    Optional.  The average character width.  The default is the
 *                              point size.
 *
 *  @note  The most broken thing with this implementation is defaulting the
 *         average character width to the point size.  Using a 0 for fWidth
 *         rather than omitting the argument will fix this.  0 causes the font
 *         mapper to pick the best font that matches the height.
 *
 */
RexxMethod4(POINTERSTRING, winex_createFont, OPTIONAL_CSTRING, fontName, OPTIONAL_CSTRING, fSize,
            OPTIONAL_CSTRING, fontStyle, OPTIONAL_CSTRING, fWidth)
{
    if ( argumentOmitted(1) )
    {
        fontName = "System";
    }

    int fontSize = 10;
    if ( argumentExists(2) )
    {
        fontSize = atoi(fSize);
    }

    int fontWidth = fontSize;
    if ( argumentExists(4) )
    {
        fontWidth = atoi(fWidth);
    }

    int weight = FW_NORMAL;
    BOOL italic = FALSE;
    BOOL underline = FALSE;
    BOOL strikeout = FALSE;

    if ( argumentExists(3) )
    {
        italic = StrStrI(fontStyle, "ITALIC") != NULL;
        underline = StrStrI(fontStyle, "UNDERLINE") != NULL;
        strikeout = StrStrI(fontStyle, "STRIKEOUT") != NULL;
        weight = getWeight(fontStyle);
    }

    HFONT hFont = CreateFont(fontSize, fontWidth, 0, 0, weight, italic, underline, strikeout,
                             DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, FF_DONTCARE, fontName);
    return hFont;
}

/** WindowExtensions::createFontEx()
 *
 *  Creates a logical font with the specified characteristics.
 *
 *  This is a correct implementation of createFont() and should be used as a
 *  replacement for that method.  In addition it extends createFont() by giving
 *  the ooRexx progammer access to all of the options of the CreateFont API.
 *
 *  @param fontName  Required.  The typeface name.
 *
 *  @param fontSize  Optional.  The point size of the font, the default is 8.
 *
 *  @param args      Optional.  A .Directory object whose indexes can contain
 *                              over-rides for the default values of all other
 *                              arguments to CreateFont.
 *
 *  @return  Handle to the logical font.  On error, a null handle is returned
 *           and the ooDialog System error code (.SystemErrorCode) is set.
 *
 *  @note    All the 'other' arguments to CreateFont() have a default value. If
 *           the args Directory object has no index for a value, the default is
 *           used.  If the Directory object does have the index, then the value
 *           of the index is used for that arg.
 */
RexxMethod4(POINTERSTRING, winex_createFontEx, CSTRING, fontName, OPTIONAL_int, fontSize,
            OPTIONAL_RexxObjectPtr, args, OSELF, self)
{
    int   width = 0;                              // average character width
    int   escapement = 0;                         // angle of escapement
    int   orientation = 0;                        // base-line orientation angle
    int   weight = FW_NORMAL;                     // font weight
    BOOL  italic = FALSE;                         // italic attribute option
    BOOL  underline = FALSE;                      // underline attribute option
    BOOL  strikeOut = FALSE;                      // strikeout attribute option
    DWORD charSet = DEFAULT_CHARSET;              // character set identifier
    DWORD outputPrecision = OUT_TT_PRECIS;        // output precision
    DWORD clipPrecision = CLIP_DEFAULT_PRECIS;    // clipping precision
    DWORD quality = DEFAULT_QUALITY;              // output quality
    DWORD pitchAndFamily = FF_DONTCARE;           // pitch and family

    oodSetSysErrCode(context, 0);

    if ( argumentOmitted(2) )
    {
        fontSize = 8;
    }

    HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
    int height = -MulDiv(fontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    DeleteDC(hdc);

    if ( argumentExists(3) )
    {
        if ( ! context->IsDirectory(args) )
        {
            wrongClassException(context, 3, "Directory");
            goto error_out;
        }
        RexxDirectoryObject d = (RexxDirectoryObject)args;

        if ( ! rxNumberFromDirectory(context, d, "WIDTH", (DWORD *)&width, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "ESCAPEMENT", (DWORD *)&escapement, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "ORIENTATION", (DWORD *)&orientation, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "WEIGHT", (DWORD *)&weight, 3) )
        {
            goto error_out;
        }
        if ( ! rxLogicalFromDirectory(context, d, "ITALIC", &italic, 3) )
        {
            goto error_out;
        }
        if ( ! rxLogicalFromDirectory(context, d, "UNDERLINE", &underline, 3) )
        {
            goto error_out;
        }
        if ( ! rxLogicalFromDirectory(context, d, "STRIKEOUT", &strikeOut, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "CHARSET", &charSet, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "OUTPUTPRECISION", &outputPrecision, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "CLIPPRECISION", &clipPrecision, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "QUALITY", &quality, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "PITCHANDFAMILY", &pitchAndFamily, 3) )
        {
            goto error_out;
        }
    }

    HFONT font = CreateFont(height, width, escapement, orientation, weight, italic, underline, strikeOut,
                            charSet, outputPrecision, clipPrecision, quality, pitchAndFamily, fontName);

    if ( font == NULL )
    {
        oodSetSysErrCode(context);
    }
    return font;

error_out:
  return NULLOBJECT;
}

/**
 *  Methods for the .DialogControl class.
 */
#define DIALOGCONTROL_CLASS        "DialogControl"


/** DialogControl::getTextSizeDlg()
 *
 *  Gets the size (width and height) in dialog units for any given string for
 *  the font specified.
 *
 *  @param  text         The string whose size is needed.
 *
 *  @param  fontName     Optional. If specified, use this font to calculate the
 *                       size.
 *
 *  @param  fontSize     Optional. If specified, use this font size with
 *                       fontName to calculate the size.  The default if omitted
 *                       is 8.  This arg is ignored if fontName is omitted.
 *
 *  @param  hwndFontSrc  Optional. Use this window's font to calculate the size.
 *                       This arg is always ignored if fontName is specified.
 *
 */
RexxMethod5(RexxObjectPtr, dlgctrl_getTextSizeDlg, CSTRING, text, OPTIONAL_CSTRING, fontName,
            OPTIONAL_uint32_t, fontSize, OPTIONAL_POINTERSTRING, hwndFontSrc, OSELF, self)
{
    HWND hwndSrc = NULL;
    if ( argumentExists(2) )
    {
        if ( argumentOmitted(3) )
        {
            fontSize = DEFAULT_FONTSIZE;
        }
    }
    else if ( argumentExists(4) )
    {
        if ( hwndFontSrc == NULL )
        {
            nullObjectException(context, "window handle", 4);
            goto error_out;
        }
        hwndSrc = (HWND)hwndFontSrc;
    }

    RexxObjectPtr dlgObj = context->SendMessage0(self, "ODLG");
    if ( dlgObj == NULLOBJECT )
    {
        // The interpreter kernel will have raised a syntax exception in this
        // case.  But, the ooDialog framework traps the exception and puts up a
        // message box saying ODLG is not a method of xx control.  I think that
        // will be confusing to the users, since they have no idea about this
        // call to oDlg. So, raise a more specific exception.
        context->RaiseException1(Rexx_Error_Interpretation_user_defined,
                                 context->String("Inconsistency: this .DialogControl object does not have "
                                                 "the oDlg (owner dialog) attribute"));
        goto error_out;
    }

    return getTextSize(context, text, fontName, fontSize, hwndSrc, dlgObj);

error_out:
    return NULLOBJECT;
}


/**
 *  Methods for the .AdvancedControls class.
 */
#define ADVANCEDCONTROLS_CLASS        "AdvancedControls"
#define ADVCTRLCONTROLBAG_ATTRIBUTE   "!advCtrlDlgControlBag"


RexxObjectPtr advGetControl(RexxMethodContext *c, ARGLIST args, OSELF self, CSTRING ctrl)
{
    RexxObjectPtr result = c->Nil();

    HWND hwnd = rxGetWindowHandle(c, self);
    if ( hwnd == NULL )
    {
        // This happens if the user were to invoke a getXXXControl() method
        // before the underlying dialog has been created.
        goto out;
    }

    HWND hControl = NULL;
    RexxObjectPtr rxControl = NULLOBJECT;

    // We have the dialog window handle.  We also need the resource ID to get
    // the control.
    RexxObjectPtr rxID = c->ArrayAt(args, 1);
    if ( rxID == NULLOBJECT )
    {
        c->RaiseException1(Rexx_Error_Incorrect_method_noarg, c->WholeNumber(1));
        goto out;
    }

    // Using 0 for the dialog arg position is okay for now.
    int id = oodResolveSymbolicID(c, self, rxID, 0, 1);
    if ( id == OOD_ID_EXCEPTION )
    {
        // We clear the condition because all the AdvancedControls::getXXCtrl()
        // methods prior to 4.0.0 would return a .nil object for this error.  It
        // would be better to raise an exception, but ...
        c->ClearCondition();
        goto out;
    }
    else
    {
        hControl = GetDlgItem(hwnd, id);
    }

    if ( hControl != NULL )
    {
        rxControl = (RexxObjectPtr)getWindowPtr(hControl, GWLP_USERDATA);
        if ( rxControl != NULLOBJECT )
        {
            // Okay, this specific control has already had a control object
            // instantiated to represent it.  We return this object.
            result = rxControl;
            goto out;
        }
    }

    // No pointer is stored in the user data area, so no control object has been
    // instantiated for this specific control, yet.  We instantiate one now and
    // then store the object in the user data area of the control window.
    c->ArrayPut(args, c->String(ctrl), 3);
    rxControl = c->ForwardMessage(NULLOBJECT, "GetControl", NULLOBJECT, args);
    if ( rxControl == NULLOBJECT )
    {
        goto out;
    }
    result = rxControl;

    if ( result != c->Nil() )
    {
        // Good object.
        setWindowPtr(hControl, GWLP_USERDATA, (LONG_PTR)result);
        c->SendMessage1(self, "putControl", result);
    }

out:
    return result;
}

RexxMethod2(RexxObjectPtr, advCtrl_getStaticControl, ARGLIST, args, OSELF, self)
{
    return advGetControl(context, args, self, "ST");
}

RexxMethod2(RexxObjectPtr, advCtrl_getButtonControl, ARGLIST, args, OSELF, self)
{
    return advGetControl(context, args, self, "BUT");
}

RexxMethod2(RexxObjectPtr, advCtrl_getListControl, ARGLIST, args, OSELF, self)
{
    return advGetControl(context, args, self, "LC");
}

RexxMethod2(RexxObjectPtr, advCtrl_getTreeControl, ARGLIST, args, OSELF, self)
{
    return advGetControl(context, args, self, "TC");
}

RexxMethod2(RexxObjectPtr, advCtrl_getTabControl, ARGLIST, args, OSELF, self)
{
    return advGetControl(context, args, self, "TAB");
}

RexxMethod2(RexxObjectPtr, advCtrl_putControl_pvt, RexxObjectPtr, control, OSELF, self)
{
    // This should never fail, do we need an exception if it does?

    RexxObjectPtr bag = context->GetObjectVariable(ADVCTRLCONTROLBAG_ATTRIBUTE);
    if ( bag == NULLOBJECT )
    {
        RexxObjectPtr theBagClass = context->FindClass("BAG");
        if ( theBagClass != NULLOBJECT )
        {
            bag = context->SendMessage0(theBagClass, "NEW");
            context->SetObjectVariable(ADVCTRLCONTROLBAG_ATTRIBUTE, bag);
        }
    }

    if ( bag != NULLOBJECT )
    {
        context->SendMessage2(bag, "PUT", control, control);
    }

    return context->Nil();
}


/**
 * Methods for the ProgressBar class.
 */
#define PROGRESSBAR_CLASS   "ProgressBar"


/**
 * Step the progress bar by the step increment or do a delta position.  A delta
 * position moves the progress bar from its current position by the specified
 * amount.
 *
 * Note this difference between stepping and doing a delta.  When the progress
 * bar is stepped and the step amount results in a position past the end of the
 * progress bar, the progress bar restarts at the minimum position.  When a
 * delta position is done, if the end of the progress bar is reached, it will
 * just stay at the end.
 *
 * @param  delta [Optional]  If present a delta position is done using this
 *               values.  If absent, then a step is done.
 *
 * @return  For both cases the previous position is returned.
 */
RexxMethod2(int, pbc_stepIt, OPTIONAL_int32_t, delta, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    if ( argumentOmitted(1) )
    {
        return (int)SendMessage(hwnd, PBM_STEPIT, 0, 0);
    }
    else
    {
        return (int)SendMessage(hwnd, PBM_DELTAPOS, delta, 0);
    }
}

/**
 * Set the position of the progress bar.
 *
 * @param newPos  Set the position to this value.
 *
 * @return The the old progress bar position.
 */
RexxMethod2(int, pbc_setPos, int32_t, newPos, OSELF, self)
{
    return (int)SendMessage(rxGetWindowHandle(context, self), PBM_SETPOS, newPos, 0);
}

RexxMethod1(int, pbc_getPos, OSELF, self)
{
    return (int)SendMessage(rxGetWindowHandle(context, self), PBM_GETPOS, 0, 0);
}

/** ProgressBar::setRange()
 *
 *  Sets the range for the progress bar using the full 32-bit numbers for the
 *  range.
 *
 *  @param min   Optional.  The low end of the range.  0 is the default.
 *  @param max   Optional.  The high end of the range.  100 is the default.
 *
 *  @return  The previous range in the form of a string with word(1) being the
 *           low end of the previous range and word(2) being the previous high
 *           end of the range.
 *
 *  @note    The returned range is not necessarily correct if the previous range
 *           has been set using the full 32-bit numbers now allowed by the
 *           progress bar control.  The returned numbers are restricted to
 *           0xFFFF.
 *
 *           The range is returned as a string because that was the way it was
 *           previously documented.
 *
 *           Use the getRange() method to get the correct range.
 *
 */
RexxMethod3(RexxStringObject, pbc_setRange, OPTIONAL_int32_t, min, OPTIONAL_int32_t, max, OSELF, self)
{
    TCHAR buf[64];
    HWND hwnd = rxGetWindowHandle(context, self);

    if ( argumentOmitted(1) )
    {
        min = 0;
    }
    if ( argumentOmitted(2) )
    {
        max = 100;
    }

    DWORD range = (DWORD)SendMessage(hwnd, PBM_SETRANGE32, min, max);
    _snprintf(buf, sizeof(buf), "%d %d", LOWORD(range), HIWORD(range));

    return context->String(buf);
}

RexxMethod1(RexxObjectPtr, pbc_getRange, OSELF, self)
{
    PBRANGE pbr;
    SendMessage(rxGetWindowHandle(context, self), PBM_GETRANGE, TRUE, (LPARAM)&pbr);

    RexxDirectoryObject d = context->NewDirectory();
    context->DirectoryPut(d, context->Int32(pbr.iLow), "MIN");
    context->DirectoryPut(d, context->Int32(pbr.iHigh), "MAX");

    return d;
}

RexxMethod2(int, pbc_setStep, OPTIONAL_int32_t, newStep, OSELF, self)
{
    if ( argumentOmitted(1) )
    {
        newStep = 10;
    }
    return (int)SendMessage(rxGetWindowHandle(context, self), PBM_SETSTEP, newStep, 0);
}

/**
 *  ProgressBar::setMarquee()  Turn marquee mode on or off.
 *
 *  @param   on     [Optional]  Stop or start marquee mode.  Default is to
 *                  start.
 *
 *  @param   pause  [Optional]  Time in milliseconds between updates.  Default
 *                  is 1000 (1 second.)
 *
 *  @return  True (always.)
 *
 *  Requires XP Common Controls version 6.0 or greater.
 */
RexxMethod3(logical_t, pbc_setMarquee, OPTIONAL_logical_t, on, OPTIONAL_uint32_t, pause, OSELF, self)
{
    if ( ! requiredComCtl32Version(context, "setMarquee", COMCTL32_6_0) )
    {
        return 0;
    }

    HWND hwnd = rxGetWindowHandle(context, self);

    if ( ! hasStyle(hwnd, PBS_MARQUEE) )
    {
        wrongWindowStyleException(context, "progress bar", "PBS_MARQUEE");
        return 0;
    }

    if ( argumentOmitted(1) )
    {
        on = 1;
    }
    if ( argumentOmitted(2) )
    {
        pause = 1000;
    }

    /* The Windows message always returns 1, return 1 for .true (succeeded.) */
    SendMessage(hwnd, PBM_SETMARQUEE, on, pause);
    return 1;
}

/**
 *  ProgressBar::backgroundColor()
 *
 *  Sets the background color of the progress bar.
 *
 *  @param   colorRef  [Required]  A COLOREF, the new background color.
 *
 *  @return  The previous background color, or CLR_DEFAULT if the previous color
 *           was the defualt.  This is returned as a COLORREF number.
 *
 *  The progress bar control only supports this function under Windows Classic
 *  Theme.
 */
RexxMethod2(uint32_t, pbc_setBkColor, uint32_t, colorRef, OSELF, self)
{
    return (uint32_t)SendMessage(rxGetWindowHandle(context, self), PBM_SETBKCOLOR, 0, colorRef);
}

/**
 *  ProgressBar::barColor()
 *
 *  Sets the bar color of the progress bar.
 *
 *  @param   colorRef  [Required]  A COLOREF, the new bar color.
 *
 *  @return  The previous bar color, or CLR_DEFAULT if the previous color
 *           was the defualt.  This is returned as a COLORREF number.
 *
 *  The progress bar control only supports this function under Windows Classic
 *  Theme.
 */
RexxMethod2(uint32_t, pbc_setBarColor, uint32_t, colorRef, OSELF, self)
{
    return (uint32_t)SendMessage(rxGetWindowHandle(context, self), PBM_SETBARCOLOR, 0, colorRef);
}


/**
 *  Methods for the .ListControl class.
 */
#define LISTCONTROL_CLASS         "ListControl"

#define LVSTATE_ATTRIBUTE         "LV!STATEIMAGELIST"
#define LVSMALL_ATTRIBUTE         "LV!SMALLIMAGELIST"
#define LVNORMAL_ATTRIBUTE        "LV!NORMALIMAGELIST"

static inline int getLVColumnCount(HWND hList)
{
    return Header_GetItemCount(ListView_GetHeader(hList));
}

static inline CSTRING lvGetAttributeName(uint8_t type)
{
    switch ( type )
    {
        case LVSIL_STATE :
            return LVSTATE_ATTRIBUTE;
        case LVSIL_SMALL :
            return LVSMALL_ATTRIBUTE;
        case LVSIL_NORMAL :
        default :
            return LVNORMAL_ATTRIBUTE;
    }
}

/** ListControl::setImageList()
 *
 *  Sets or removes one of a list-view's image lists.
 *
 *  @param imageList  An .ImageList object that references the image list to be
 *                    set, or .nil. If .nil, an existing image list, if any is
 *                    removed.
 *
 *  @param type       One of the list-view image list types and specifies which
 *                    image list we are concerned with, normal, small or state.
 *
 *  @return           Returns the exsiting .ImageList object if there is one, or
 *                    .nil if there is not an existing object.
 *
 */
RexxMethod3(RexxObjectPtr, lv_setImageList, RexxObjectPtr, imageList, OPTIONAL_uint8_t, type, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    HIMAGELIST himl = NULL;

    if ( imageList != context->Nil() )
    {
        // imageList is not .nil, so it has to be a .ImageList, or error.
        himl = rxGetImageList(context, imageList, 1);
        if ( himl == NULL )
        {
            goto err_out;
        }
    }

    if ( argumentOmitted(2) )
    {
        type = LVSIL_NORMAL;
    }
    else if ( type > LVSIL_STATE )
    {
        wrongRangeException(context, 2, LVSIL_NORMAL, LVSIL_STATE, type);
        goto err_out;
    }

    ListView_SetImageList(hwnd, himl, type);
    return rxSetObjVar(context, lvGetAttributeName(type), imageList);

err_out:
    return NULLOBJECT;
}

/** ListControl::getImageList()
 *
 *  Gets the list-view's specifed image list.
 *
 *  @param  type [optional] Identifies which image list to get.  Normal, small,
 *          or state. Normal is the default.
 *
 *  @return  The image list, if it exists, otherwise .nil.
 */
RexxMethod2(RexxObjectPtr, lv_getImageList, OPTIONAL_uint8_t, type, OSELF, self)
{
    if ( argumentOmitted(1) )
    {
        type = LVSIL_NORMAL;
    }
    else if ( type > LVSIL_STATE )
    {
        wrongRangeException(context, 1, LVSIL_NORMAL, LVSIL_STATE, type);
        return NULLOBJECT;
    }

    RexxObjectPtr result = context->GetObjectVariable(lvGetAttributeName(type));
    if ( result == NULLOBJECT )
    {
        result = context->Nil();
    }
    return result;
}

RexxMethod1(int, lv_getColumnCount, OSELF, self)
{
    return getLVColumnCount(rxGetWindowHandle(context, self));
}

RexxMethod1(RexxObjectPtr, lv_getColumnOrder, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    int count = getLVColumnCount(hwnd);
    if ( count == -1 )
    {
        return context->Nil();
    }

    RexxArrayObject order = context->NewArray(count);
    RexxObjectPtr result = order;

    // the empty array covers the case when count == 0

    if ( count == 1 )
    {
        context->ArrayPut(order, context->Int32(0), 1);
    }
    else if ( count > 1 )
    {
        int *pOrder = (int *)malloc(count * sizeof(int));
        if ( pOrder == NULL )
        {
            outOfMemoryException(context);
        }
        else
        {
            if ( ListView_GetColumnOrderArray(hwnd, count, pOrder) == 0 )
            {
                result = context->Nil();
            }
            else
            {
                for ( int i = 0; i < count; i++)
                {
                    context->ArrayPut(order, context->Int32(pOrder[i]), i + 1);
                }
            }
            free(pOrder);
        }
    }
    return result;
}

RexxMethod2(logical_t, lv_setColumnOrder, RexxArrayObject, order, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    size_t    items   = context->ArrayItems(order);
    int       count   = getLVColumnCount(hwnd);
    int      *pOrder  = NULL;
    logical_t success = FALSE;

    if ( count != -1 )
    {
        if ( count != items )
        {
            userDefinedMsgException(context, "the number of items in the order array does not match the number of columns");
            goto done;
        }

        int *pOrder = (int *)malloc(items * sizeof(int));
        if ( pOrder != NULL )
        {
            RexxObjectPtr item;
            int column;

            for ( size_t i = 0; i < items; i++)
            {
                item = context->ArrayAt(order, i + 1);
                if ( item == NULLOBJECT || ! context->ObjectToInt32(item, &column) )
                {
                    wrongObjInArrayException(context, 1, i + 1, "valid column number");
                    goto done;
                }
                pOrder[i] = column;
                printf("Item: %d value:%d\n", i, column);
            }

            if ( ListView_SetColumnOrderArray(hwnd, count, pOrder) )
            {
                // If we don't redraw the list view and it is already displayed
                // on the screen, it will look mangled.
                RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
                success = TRUE;
            }
        }
        else
        {
            outOfMemoryException(context);
        }
    }

done:
    safeFree(pOrder);
    return success;
}

/**
 *  Methods for the .TreeControl class.
 */
#define TREECONTROL_CLASS         "TreeControl"

#define TVSTATE_ATTRIBUTE         "TV!STATEIMAGELIST"
#define TVNORMAL_ATTRIBUTE        "TV!NORMALIMAGELIST"

CSTRING tvGetAttributeName(uint8_t type)
{
    switch ( type )
    {
        case TVSIL_STATE :
            return TVSTATE_ATTRIBUTE;
        case TVSIL_NORMAL :
        default :
            return TVNORMAL_ATTRIBUTE;
    }
}

/** TreeControl::setImageList()
 *
 *  Sets or removes one of a tree-view's image lists.
 *
 *  @param imageList  An .ImageList object that references the image list to be
 *                    set, or .nil. If .nil, an existing image list, if any, is
 *                    removed.
 *
 *  @param type       [optional] One of the tree-view image list types and
 *                    specifies which image list we are concerned with, normal,
 *                    or state. The default is normal.
 *
 *  @return           Returns the exsiting .ImageList object if there is one, or
 *                    .nil if there is not an existing object.
 *
 */
RexxMethod3(RexxObjectPtr, tv_setImageList, RexxObjectPtr, imageList, OPTIONAL_uint8_t, type, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    HIMAGELIST himl = NULL;

    if ( imageList != context->Nil() )
    {
        // imageList is not .nil, so it has to be a .ImageList, or error.
        himl = rxGetImageList(context, imageList, 1);
        if ( himl == NULL )
        {
            goto err_out;
        }
    }

    if ( argumentOmitted(2) )
    {
        type = TVSIL_NORMAL;
    }
    else if ( type != TVSIL_STATE && type != TVSIL_NORMAL )
    {
        invalidTypeException(context, 2, "TVSIL_XXX flag");
        goto err_out;
    }

    TreeView_SetImageList(hwnd, himl, type);
    return rxSetObjVar(context, tvGetAttributeName(type), imageList);

err_out:
    return NULLOBJECT;
}

/** TreeControl::getImageList()
 *
 *  Gets the tree-view's specifed image list.
 *
 *  @param  type [optional] Identifies which image list to get, normal, or
 *               state. Normal is the default.
 *
 *  @return  The image list, if it exists, otherwise .nil.
 */
RexxMethod2(RexxObjectPtr, tv_getImageList, OPTIONAL_uint8_t, type, OSELF, self)
{
    if ( argumentOmitted(1) )
    {
        type = TVSIL_NORMAL;
    }
    else if ( type != TVSIL_STATE && type != TVSIL_NORMAL )
    {
        invalidTypeException(context, 2, "TVSIL_XXX flag");
        return NULLOBJECT;
    }

    RexxObjectPtr result = context->GetObjectVariable(tvGetAttributeName(type));
    if ( result == NULLOBJECT )
    {
        result = context->Nil();
    }
    return result;
}


/**
 *  Methods for the .TabControl class.
 */
#define TABCONTROL_CLASS          "TabControl"

#define TABIMAGELIST_ATTRIBUTE    "TAB!IMAGELIST"

/** TabControl::setImageList()
 *
 *  Sets or removes the image list for a Tab control.
 *
 *  @param imageList  An .ImageList object that references the image list to be
 *                    set, or .nil. If .nil, an existing image list, if any, is
 *                    removed.
 *
 *  @return           Returns the exsiting .ImageList object, or .nil if there
 *                    is not an existing object.
 */
RexxMethod2(RexxObjectPtr, tab_setImageList, RexxObjectPtr, imageList, OSELF, self)
{
    HIMAGELIST himl = NULL;

    if ( imageList != context->Nil() )
    {
        himl = rxGetImageList(context, imageList, 1);
        if ( himl == NULL )
        {
            goto err_out;
        }
    }

    HWND hwnd = rxGetWindowHandle(context, self);
    TabCtrl_SetImageList(hwnd, himl);
    return rxSetObjVar(context, TABIMAGELIST_ATTRIBUTE, imageList);

err_out:
    return NULLOBJECT;
}

/** TabControl::getImageList()
 *
 *  Gets the Tab control's image list.
 *
 *  @return  The image list, if it exists, otherwise .nil.
 */
RexxMethod1(RexxObjectPtr, tab_getImageList, OSELF, self)
{
    RexxObjectPtr result = context->GetObjectVariable(TABIMAGELIST_ATTRIBUTE);
    return (result == NULLOBJECT) ? context->Nil() : result;
}


/**
 *  Methods for the .StaticControl.
 */
#define STATIC_CLASS              "StaticControl"
#define STATICIMAGE_ATTRIBUTE     "!STATICIMAGE"

/** StaticControl::setText()
 *
 *
 */
RexxMethod2(uint32_t, stc_setText, CSTRING, text, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    uint32_t rc = 0;

    if ( SetWindowText(hwnd, text) == 0 )
    {
        rc = GetLastError();
    }
    return rc;
}

/** StaticControl::getText()
 *
 *
 */
RexxMethod1(RexxStringObject, stc_getText, OSELF, self)
{
    RexxStringObject result = NULLOBJECT;
    HWND hwnd = rxGetWindowHandle(context, self);
    oodSetSysErrCode(context, 0);

    ULONG count = (ULONG)GetWindowTextLength(hwnd);
    if ( count == 0 )
    {
        result = context->String("");
    }
    else
    {
        char *buf = (char *)malloc(count);
        if ( ! buf )
        {
            outOfMemoryException(context);
        }
        else
        {
            *buf = '\0';
            if ( GetWindowText(hwnd, buf, count) == 0 )
            {
                oodSetSysErrCode(context);
            }
            result = context->String(buf);
            free(buf);
        }
    }
    return result;
}

/** StaticControl::setIcon()
 *
 *  Sets or removes the icon image for this static control.
 *
 *  @param  icon  The new icon image for the the static control, or .nil to
 *                remove the existing icon.
 *
 *  @return  The existing icon, or .nil if there is no existing icon.
 */
RexxMethod2(RexxObjectPtr, stc_setIcon, RexxObjectPtr, icon, OSELF, self)
{
    RexxObjectPtr result = NULLOBJECT;

    HANDLE hNewIcon = NULL;
    if ( icon != context->Nil() )
    {
        POODIMAGE oi = rxGetImageIcon(context, icon, 1);
        if ( oi == NULL )
        {
            goto out;
        }
        hNewIcon = oi->hImage;
    }

    HWND hwnd = rxGetWindowHandle(context, self);
    HICON hIcon = (HICON)SendMessage(hwnd, STM_SETICON, (WPARAM)hNewIcon, 0);

    result = oodSetImageAttribute(context, STATICIMAGE_ATTRIBUTE, icon, hwnd, hIcon, IMAGE_ICON, oodcStatic);
out:
    return result;
}

/** StaticControl::getIcon()
 *
 *  Gets the icon image for this static control.
 *
 *  @return  The icon image, or .nil if there is no icon image.
 */
RexxMethod1(RexxObjectPtr, stc_getIcon, OSELF, self)
{
    return oodGetImageAttribute(context, self, STATICIMAGE_ATTRIBUTE, STM_GETICON, 0, IMAGE_ICON, oodcStatic);
}

/** StaticControl::setImage()
 *
 *  Sets or removes the image for this static control.
 *
 *  @param  rxNewImage  The new image for the the control, or .nil to remove the
 *                      existing image.
 *
 * @return  The old image, if there is one, otherwise .nil.
 */
RexxMethod2(RexxObjectPtr, stc_setImage, RexxObjectPtr, rxNewImage, OSELF, self)
{
    RexxObjectPtr result = NULLOBJECT;

    long type = 0;
    HANDLE hImage = NULL;

    if ( rxNewImage != context->Nil() )
    {
        POODIMAGE oi = rxGetOodImage(context, rxNewImage, 1);
        if ( oi == NULL )
        {
            goto out;
        }
        type = oi->type;
        hImage = oi->hImage;
    }

    HWND hwnd = rxGetWindowHandle(context, self);
    HANDLE oldHandle = (HANDLE)SendMessage(hwnd, STM_SETIMAGE, (WPARAM)type, (LPARAM)hImage);

    result = oodSetImageAttribute(context, STATICIMAGE_ATTRIBUTE, rxNewImage, hwnd, oldHandle, -1, oodcStatic);
out:
    return result;
}

/** StaticControl::getImage()
 *
 *  Gets the image for this static control, if any.
 *
 *  @param  type  [optional]  Signals the type of image, one of the image type
 *                IDs.  The default is IMAGE_BITMAP.
 *
 * @return  The existing image, if there is one, otherwise .nil
 */
RexxMethod2(RexxObjectPtr, stc_getImage, OPTIONAL_uint8_t, type, OSELF, self)
{
    if ( argumentOmitted(1) )
    {
        type = IMAGE_BITMAP;
    }
    if ( type > IMAGE_ENHMETAFILE )
    {
        wrongArgValueException(context, 1, IMAGE_TYPE_LIST, getImageTypeName(type));
        return NULLOBJECT;
    }
    return oodGetImageAttribute(context, self, STATICIMAGE_ATTRIBUTE, STM_GETIMAGE, type, -1, oodcStatic);
}

/**
 *  Methods for the ooDialog class: .ButtonControl and its subclasses
 *  .RadioButton and .CheckBox.
 */
#define BUTTON_CLASS                 "ButtonControl"

#define BUTTONIMAGELIST_ATTRIBUTE    "!BUTTONIMAGELIST"
#define BUTTONIMAGE_ATTRIBUTE        "!BUTTONIMAGE"

BUTTONTYPE getButtonInfo(HWND hwnd, PBUTTONSUBTYPE sub, DWORD *style)
{
    BUTTONTYPE type = notButton;

    if ( ! checkWindowClass(hwnd, WC_BUTTON) )
    {
        if ( sub != NULL )
        {
            *sub = noSubtype;
        }
        if ( style != NULL )
        {
            *style = 0;
        }
        return type;
    }

    LONG _style = GetWindowLong(hwnd, GWL_STYLE);
    BUTTONSUBTYPE _sub;

    switch ( _style & BS_TYPEMASK )
    {
        case BS_PUSHBUTTON :
        case BS_PUSHBOX :
            type = push;
            _sub = noSubtype;
            break;

        case BS_DEFPUSHBUTTON :
            type = push;
            _sub = def;
            break;

        case BS_CHECKBOX :
            type = check;
            _sub = noSubtype;
            break;

        case BS_AUTOCHECKBOX :
            type = check;
            _sub = autoCheck;
            break;

        case BS_3STATE :
            type = check;
            _sub = threeState;
            break;

        case BS_AUTO3STATE :
            type = check;
            _sub = autoThreeState;
            break;

        case BS_RADIOBUTTON :
            type = radio;
            _sub = noSubtype;
            break;

        case BS_AUTORADIOBUTTON :
            type = radio;
            _sub = autoCheck;
            break;

        case BS_GROUPBOX :
            type = group;
            _sub = noSubtype;
            break;

        case BS_USERBUTTON :
        case BS_OWNERDRAW :
            type = owner;
            _sub = noSubtype;
            break;

        default :
            // Can not happen.
            type = notButton;
            _sub = noSubtype;
            break;
     }

    if ( style != NULL )
    {
        *style = _style & ~BS_TYPEMASK;
    }
    if ( sub != NULL )
    {
        *sub = _sub;
    }
    return type;
}

/**
 * Changes the default push button in a dialog to that of the dialog control
 * specified.
 *
 * @param hCtrl  The push button that is to become the default push button.
 *
 * @return True on success, otherwise false.
 *
 * @assumes hCtrl is a push button control in a dialog.
 */
HWND changeDefPushButton(HWND hCtrl)
{
    HWND hDlg = GetParent(hCtrl);
    int  id = GetDlgCtrlID(hCtrl);
    HWND hOldDef = NULL;

    if ( hDlg != NULL )
    {
        LRESULT result = SendMessage(hDlg, DM_GETDEFID, 0, 0);

        if ( HIWORD(result) == DC_HASDEFID )
        {
            if ( LOWORD(result) == id )
            {
                /* This control already is the default push button, just return.
                 */
                return hOldDef;
            }

            /* The DM_SETDEFID message does not remove the default push button
             * highlighting, we have to do that ourselves.
             */
            hOldDef = (HWND)GetDlgItem(hDlg, LOWORD(result));
        }

        SendMessage(hDlg, DM_SETDEFID, (WPARAM)id, 0);

        if ( hOldDef )
        {
            SendMessage(hOldDef, BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, (LPARAM)TRUE);
        }
    }
    return hOldDef;
}

/**
 * Gets the button image list information for this button, which includes the
 * image list itself, the image alignment, and the margin around the image.
 *
 * @param c     The method context we are executing in.
 * @param self  The ButtonControl object.
 *
 * @return  A directory object containing the image list information, if there
 *          is an image list.  Otherwise .nil.
 *
 * @note    Button image lists can not be set from a resource file, so if there
 *          is an image list for this button, it had to be set from code.
 *          Meaning, if there is not an image list in the object variable, then
 *          this button does not have an image list.
 */
RexxObjectPtr bcGetImageList(RexxMethodContext *c, RexxObjectPtr self)
{
    RexxObjectPtr result = c->Nil();

    RexxObjectPtr imageList = c->GetObjectVariable(BUTTONIMAGELIST_ATTRIBUTE);
    if ( imageList != NULLOBJECT && imageList != c->Nil() )
    {
        HWND hwnd = rxGetWindowHandle(c, self);
        BUTTON_IMAGELIST biml;

        Button_GetImageList(hwnd, &biml);
        RexxDirectoryObject table = c->NewDirectory();
        if ( table != NULLOBJECT )
        {
            c->DirectoryPut(table, imageList, "IMAGELIST");

            RexxObjectPtr rect = rxNewRect(c, biml.margin.left, biml.margin.top,
                                           biml.margin.right, biml.margin.bottom);
            if ( rect != NULL )
            {
                c->DirectoryPut(table, rect, "RECT");
            }

            RexxObjectPtr alignment = c->WholeNumber(biml.uAlign);
            if ( alignment != NULLOBJECT )
            {
                c->DirectoryPut(table, alignment, "ALIGNMENT");
            }
            result = table;
        }
    }
    return result;
}

RexxObjectPtr bcRemoveImageList(RexxMethodContext *c, RexxObjectPtr self)
{
    RexxObjectPtr result = bcGetImageList(c, self);

    if ( result != c->Nil() )
    {
        HWND hwnd = rxGetWindowHandle(c, self);
        BUTTON_IMAGELIST biml = {0};
        biml.himl = ImageList_Create(32, 32, ILC_COLOR8, 2, 0);

        Button_SetImageList(hwnd, &biml);
        ImageList_Destroy(biml.himl);
        RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
    }
    return result;
}

/** GroupBox::style=()
 *
 * A group box is a button, but the only style change that makes much sense is
 * the right or left alignment of the text.  Other changes either have no
 * effect, or cause the group box / dialog to paint in a weird way.
 */
RexxMethod2(int, gb_setStyle, OSELF, self, CSTRING, opts)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    LONG style = GetWindowLong(hwnd, GWL_STYLE);

    if ( stricmp(opts, "RIGHT") == 0 )
    {
        style = (style & ~BS_CENTER) | BS_RIGHT;
    }
    else if ( stricmp(opts, "LEFT") == 0 )
    {
        style = (style & ~BS_CENTER) | BS_LEFT;
    }
    else
    {
        wrongArgValueException(context, 1, "RIGHT, LEFT", opts);
        return 0;
    }

    /**
     * When the alignment changes, we need to force the dialog to redraw the
     * area occupied by the group box.  Otherwise the old text remains on the
     * screen.  But, it is only the top part of the group box that needs to be
     * redrawn, so we only invalidate the top half of the group box.
     */

    HWND hDlg = GetParent(hwnd);
    RECT r;

    // Get the screen area of the group box and map it to the client area of the
    // dialog.
    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    LONG halfHeight = ((r.bottom - r.top) / 2);
    r.bottom = (halfHeight >= MIN_HALFHEIGHT_GB ? r.top + halfHeight : r.bottom);

    // Change the group box style, force the dialog to repaint.
    SetWindowLong(hwnd, GWL_STYLE, style);
    SendMessage(hwnd, BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);

    InvalidateRect(hDlg, &r, TRUE);
    UpdateWindow(hDlg);

    return 0;
}

RexxMethod2(RexxObjectPtr, bc_setState, CSTRING, opts, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    BUTTONTYPE type = getButtonInfo(hwnd, NULL, NULL);
    UINT msg = 0;
    WPARAM wp = 0;

    char *token;
    char *str = strdupupr(opts);
    if ( ! str )
    {
        outOfMemoryException(context);
        return NULLOBJECT;
    }

    token = strtok(str, " ");
    while ( token != NULL )
    {
        if ( strcmp(token, "CHECKED") == 0 )
        {
            if ( (type == check || type == radio) )
            {
                msg = BM_SETCHECK;
                wp = (WPARAM)BST_CHECKED;
            }
        }
        else if ( strcmp(token, "UNCHECKED") == 0 )
        {
            if ( (type == check || type == radio) )
            {
                msg = BM_SETCHECK;
                wp = (WPARAM)BST_UNCHECKED;
            }
        }
        else if ( strcmp(token, "INDETERMINATE") == 0 )
        {
            if ( type == check )
            {
                msg = BM_SETCHECK;
                wp = (WPARAM)BST_INDETERMINATE;
            }
        }
        else if ( strcmp(token, "FOCUS") == 0 )
        {
            msg = 0;
            SendMessage(GetParent(hwnd), WM_NEXTDLGCTL, (WPARAM)hwnd, TRUE);
        }
        else if ( strcmp(token, "PUSHED") == 0 )
        {
            msg = BM_SETSTATE;
            wp = (WPARAM)TRUE;
        }
        else if ( strcmp(token, "NOTPUSHED") == 0 )
        {
            msg = BM_SETSTATE;
            wp = (WPARAM)FALSE;
        }
        else
        {
            wrongArgValueException(context, 1, BC_SETSTATE_OPTS, token);
            free(str);
            return NULLOBJECT;
        }

        if ( msg != 0 )
        {
            SendMessage(hwnd, msg, wp, 0);
            msg = 0;
        }
        token = strtok(NULL, " ");
    }

    safeFree(str);
    return NULLOBJECT;
}

RexxMethod1(RexxStringObject, bc_getState, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    BUTTONTYPE type = getButtonInfo(hwnd, NULL, NULL);

    TCHAR buf[64] = {'\0'};
    LRESULT l;

    if ( type == radio || type == check )
    {
        l = SendMessage(hwnd, BM_GETCHECK, 0, 0);
        if ( l == BST_CHECKED )
        {
            strcpy(buf, "CHECKED ");
        }
        else if ( l == BST_INDETERMINATE )
        {
            strcpy(buf,  "INDETERMINATE ");
        }
        else
        {
            strcpy(buf, "UNCHECKED ");
        }
    }

    l = SendMessage(hwnd, BM_GETSTATE, 0, 0);
    if ( l & BST_FOCUS )
        strcat(buf, "FOCUS ");
    {
    }
    if ( l & BST_PUSHED )
    {
        strcat(buf, "PUSHED");
    }

    return context->String(buf);
}


RexxMethod2(RexxObjectPtr, bc_setStyle, OSELF, self, CSTRING, opts)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    BUTTONSUBTYPE sub;
    DWORD style, oldStyle;
    BUTTONTYPE type;
    DWORD typeStyle, oldTypeStyle;
    bool changeDefButton = false;

    if ( strlen(opts) == 0 )
    {
        // No change.
        return NULLOBJECT;
    }

    type = getButtonInfo(hwnd, &sub, &style);
    oldStyle = style;
    oldTypeStyle = ((DWORD)GetWindowLong(hwnd, GWL_STYLE) & BS_TYPEMASK);
    typeStyle = oldTypeStyle;

    char *token;
    char *str = strdupupr(opts);
    if ( ! str )
    {
        outOfMemoryException(context);
        return NULLOBJECT;
    }

    token = strtok(str, " ");
    while ( token != NULL )
    {
        if ( strcmp(token, "PUSHBOX") == 0 )
        {
            if ( type == push )
            {
                typeStyle = BS_PUSHBOX;
            }
        }
        else if ( strcmp(token, "DEFPUSHBUTTON") == 0 )
        {
            if ( type == push  && sub != def )
            {
                typeStyle = BS_DEFPUSHBUTTON;
                changeDefButton = true;
            }
        }
        else if ( strcmp(token, "CHECKBOX") == 0 )
        {
            if ( type == check )
            {
                typeStyle = BS_CHECKBOX;
            }
        }
        else if ( strcmp(token, "AUTOCHECKBOX") == 0 )
        {
            if ( type == check )
            {
                typeStyle = BS_AUTOCHECKBOX;
            }
        }
        else if ( strcmp(token, "3STATE") == 0 )
        {
            if ( type == check )
            {
                typeStyle = BS_3STATE;
            }
        }
        else if ( strcmp(token, "AUTO3STATE") == 0 )
        {
            if ( type == check )
            {
                typeStyle = BS_AUTO3STATE;
            }
        }
        else if ( strcmp(token, "RADIO") == 0 )
        {
            if ( type == radio )
            {
                typeStyle = BS_RADIOBUTTON;
            }
        }
        else if ( strcmp(token, "AUTORADIO") == 0 )
        {
            if ( type == radio )
            {
                typeStyle = BS_AUTORADIOBUTTON;
            }
        }
        else if ( strcmp(token, "GROUPBOX") == 0 || strcmp(token, "OWNERDRAW") == 0 )
        {
            ; // Ignored.
        }
        else if ( strcmp(token, "LEFTTEXT") == 0 || strcmp(token, "RIGHTBUTTON") == 0 )
        {
            style |= BS_LEFTTEXT;
        }
        else if ( strcmp(token, "NOTLEFTTEXT") == 0 )
        {
            style &= ~BS_LEFTTEXT;
        }
        else if ( strcmp(token, "TEXT") == 0 )
        {
            style &= ~(BS_ICON | BS_BITMAP);
        }
        else if ( strcmp(token, "ICON") == 0 )
        {
            style = (style & ~BS_BITMAP) | BS_ICON;
        }
        else if ( strcmp(token, "BITMAP") == 0 )
        {
            style = (style & ~BS_ICON) | BS_BITMAP;
        }
        else if ( strcmp(token, "LEFT") == 0 )
        {
            style = (style & ~BS_CENTER) | BS_LEFT;
        }
        else if ( strcmp(token, "RIGHT") == 0 )
        {
            style = (style & ~BS_CENTER) | BS_RIGHT;
        }
        else if ( strcmp(token, "HCENTER") == 0 )
        {
            style |= BS_CENTER;
        }
        else if ( strcmp(token, "TOP") == 0 )
        {
            style = (style & ~BS_VCENTER) | BS_TOP;
        }
        else if ( strcmp(token, "BOTTOM") == 0 )
        {
            style = (style & ~BS_VCENTER) | BS_BOTTOM;
        }
        else if ( strcmp(token, "VCENTER") == 0 )
        {
            style |= BS_VCENTER;
        }
        else if ( strcmp(token, "PUSHLIKE") == 0 )
        {
            if ( type == check || type == radio )
            {
                style |= BS_PUSHLIKE;
            }
        }
        else if ( strcmp(token, "MULTILINE") == 0 )
        {
            style |= BS_MULTILINE;
        }
        else if ( strcmp(token, "NOTIFY") == 0 )
        {
            style |= BS_NOTIFY;
        }
        else if ( strcmp(token, "FLAT") == 0 )
        {
            style |= BS_FLAT;
        }
        else if ( strcmp(token, "NOTPUSHLIKE") == 0 )
        {
            if ( type == check || type == radio )
            {
                style &= ~BS_PUSHLIKE;
            }
        }
        else if ( strcmp(token, "NOTMULTILINE") == 0 )
        {
            style &= ~BS_MULTILINE;
        }
        else if ( strcmp(token, "NOTNOTIFY") == 0 )
        {
            style &= ~BS_NOTIFY;
        }
        else if ( strcmp(token, "NOTFLAT") == 0 )
        {
            style &= ~BS_FLAT;
        }
        else
        {
            wrongArgValueException(context, 1, BC_SETSTYLE_OPTS, token);
            free(str);
            return NULLOBJECT;
        }

        token = strtok(NULL, " ");
    }

    style |= typeStyle;

    HWND oldDefButton = NULL;
    if ( changeDefButton )
    {
        oldDefButton = changeDefPushButton(hwnd);
    }

    if ( style != (oldStyle | oldTypeStyle) )
    {
        SetWindowLong(hwnd, GWL_STYLE, style);
        SendMessage(hwnd, BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);

        RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

        if ( oldDefButton )
        {
            RedrawWindow(oldDefButton, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
        }
    }

    safeFree(str);
    return NULLOBJECT;
}

RexxMethod1(RexxObjectPtr, bc_click, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    SendMessage(hwnd, BM_CLICK, 0, 0);
    return NULLOBJECT;
}

RexxMethod1(RexxObjectPtr, bc_getTextMargin, OSELF, self)
{
    if ( ! requiredComCtl32Version(context, "getTextMargin", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }

    HWND hwnd = rxGetWindowHandle(context, self);
    RexxObjectPtr result = NULLOBJECT;

    RECT r;
    if ( Button_GetTextMargin(hwnd, &r) )
    {
        result = rxNewRect(context, r.left, r.top, r.right, r.bottom);
    }
    return (result == NULL) ? context->Nil() : result;
}

RexxMethod2(logical_t, bc_setTextMargin, OSELF, self, RexxObjectPtr, r)
{
    if ( ! requiredComCtl32Version(context, "setTextMargin", COMCTL32_6_0) )
    {
        return 0;
    }

    HWND hwnd = rxGetWindowHandle(context, self);

    PRECT pRect = rxGetRect(context, r, 1);
    if ( pRect != NULL )
    {
        if ( Button_SetTextMargin(hwnd, pRect) )
        {
            return 1;
        }
    }
    return 0;
}

RexxMethod1(RexxObjectPtr, bc_getIdealSize, OSELF, self)
{
    if ( ! requiredComCtl32Version(context, "getIdealSize", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }

    HWND hwnd = rxGetWindowHandle(context, self);
    RexxObjectPtr result = NULLOBJECT;

    SIZE size;
    if ( Button_GetIdealSize(hwnd, &size) )
    {
        result = rxNewSize(context, size.cx, size.cy);
    }
    return (result == NULLOBJECT) ? context->Nil() : result;
}

RexxMethod2(RexxObjectPtr, bc_getImage, OPTIONAL_uint8_t, type, OSELF, self)
{
    if ( argumentOmitted(1) )
    {
        type = IMAGE_BITMAP;
    }
    if ( type > IMAGE_CURSOR )
    {
        wrongArgValueException(context, 1, "Bitmap, Icon, Cursor", getImageTypeName(type));
        return NULLOBJECT;
    }
    WPARAM wParam = (type == IMAGE_BITMAP) ? IMAGE_BITMAP : IMAGE_ICON;

    return oodGetImageAttribute(context, self, BUTTONIMAGE_ATTRIBUTE, BM_GETIMAGE, wParam, type, oodcButton);
}

/** ButtonControl::setImage()
 *
 *  Sets or removes the image for a button control.
 *
 *  @param  rxNewImage  The new image for the button, or .nil to remove the
 *                      existing image.
 *
 *  @return  The existing image, if there is one, or .nil if there is not.
 *
 *  @note  Only bitmap, icon, or cursor images are valid.
 */
RexxMethod2(RexxObjectPtr, bc_setImage, RexxObjectPtr, rxNewImage, OSELF, self)
{
    RexxObjectPtr result = NULLOBJECT;

    long type = IMAGE_BITMAP;
    HANDLE hImage = NULL;

    if ( rxNewImage != context->Nil() )
    {
        POODIMAGE oi = rxGetOodImage(context, rxNewImage, 1);
        if ( oi == NULL )
        {
            goto out;
        }

        if ( oi->type > IMAGE_CURSOR )
        {
            wrongArgValueException(context, 1, "Bitmap, Icon, Cursor", oi->typeName);
            goto out;
        }
        hImage = oi->hImage;
        type = oi->type == IMAGE_BITMAP ? IMAGE_BITMAP : IMAGE_ICON;
    }

    HWND hwnd = rxGetWindowHandle(context, self);
    HANDLE oldHandle = (HANDLE)SendMessage(hwnd, BM_SETIMAGE, (WPARAM)type, (LPARAM)hImage);

    result = oodSetImageAttribute(context, BUTTONIMAGE_ATTRIBUTE, rxNewImage, hwnd, oldHandle, -1, oodcButton);

out:
    return result;
}

/** ButtonControl::getImageList()
 *
 * Gets the image list for the button, if there is one.
 *
 * @return  .nil if this the button control does not have an image list.
 *          Otherwise, a .Directory object with the following indexes.  The
 *          indexes contain the image list related information:
 *
 *          d~imageList -> The .ImageList object set by setImageList().
 *          d~rect      -> A .Rect object containing the margins.
 *          d~alignment -> The image alignment in the button.
 *
 * @requires  Common Controls version 6.0 or later.
 *
 * @exception  A syntax error is raised for wrong comctl version.
 *
 * @note  The only way to have an image list is for it have been put there by
 *        setImageList().  That method stores the .ImageList object as an
 *        attribute of the ButtonControl ojbect.  That stored object is the
 *        object returned.
 */
RexxMethod1(RexxObjectPtr, bc_getImageList, OSELF, self)
{
    if ( ! requiredComCtl32Version(context, "getImageList", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }
    return bcGetImageList(context, self);

}

/** ButtonControl::setImageList()
 *
 * Sets or removes an image list for the button.
 *
 * @param   imageList  [required]  The new image list for the button, or .nil to
 *                     remove the current image list.
 *
 * @param   margin     [optional]  A .Rect object containing the margins around
 *                     the image.  The default is no margin on either side.
 *
 * @param   align      [optional]  One of the BUTTON_IMAGELIST_ALIGN_xxx
 *                     constant values.  The default is center.
 *
 * @return  The old image list information, if there was an existing image list.
 *          .nil is returned on error and if there was not an existing image
 *          list..
 *
 * @requires  Common Controls version 6.0 or later.
 *
 * @exception  Syntax errors are raised for incorrect arguments and wrong comctl
 *             version.
 *
 * @remarks This method sets the ooDialog System error code (.SystemErrorCode)
 *          if the args seem valid but one of the Windows APIs fails.
 *
 * @see bcGetImageList() for the format of the returned image list information.
 */
RexxMethod4(RexxObjectPtr, bc_setImageList, RexxObjectPtr, imageList, OPTIONAL_RexxObjectPtr, margin,
            OPTIONAL_uint8_t, align, OSELF, self)
{
    BUTTON_IMAGELIST biml = {0};
    oodSetSysErrCode(context, 0);
    RexxObjectPtr result = NULLOBJECT;

    if ( ! requiredComCtl32Version(context, "setImageList", COMCTL32_6_0) )
    {
        goto err_out;
    }

    if ( imageList == context->Nil() )
    {
        // This is a request to remove the image list.
        result = bcRemoveImageList(context, self);
        goto good_out;
    }

    biml.himl = rxGetImageList(context, imageList, 1);
    if ( biml.himl == NULL )
    {
        goto err_out;
    }

    // Default would be a 0 margin
    if ( argumentExists(2) )
    {
        PRECT pRect = rxGetRect(context, margin, 2);
        if ( pRect == NULL )
        {
            goto err_out;
        }
        biml.margin.top = pRect->top;
        biml.margin.left = pRect->left;
        biml.margin.right = pRect->right;
        biml.margin.bottom = pRect->bottom;
    }

    if ( argumentExists(3) )
    {
        if ( align > BUTTON_IMAGELIST_ALIGN_CENTER )
        {
            wrongRangeException(context, 3, BUTTON_IMAGELIST_ALIGN_LEFT,
                                BUTTON_IMAGELIST_ALIGN_CENTER, align);
            goto err_out;
        }
        biml.uAlign =  align;
    }
    else
    {
        biml.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
    }

    result = bcGetImageList(context, self);
    HWND hwnd = rxGetWindowHandle(context, self);

    if ( Button_SetImageList(hwnd, &biml) == 0 )
    {
        oodSetSysErrCode(context);
        goto err_out;
    }
    else
    {
        RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
        context->SetObjectVariable(BUTTONIMAGELIST_ATTRIBUTE, imageList);
    }

good_out:
    return result;

err_out:
    return NULLOBJECT;
}

RexxMethod4(int, rb_checkInGroup_cls, RexxObjectPtr, dlg, RexxObjectPtr, idFirst,
            RexxObjectPtr, idLast, RexxObjectPtr, idCheck)
{
    int result = 0;
    if ( requiredClass(context, dlg, "PlainBaseDialog", 1) )
    {
        HWND hwnd = rxGetWindowHandle(context, dlg);

        int first = oodResolveSymbolicID(context, dlg, idFirst, 1, 2);
        int last = oodResolveSymbolicID(context, dlg, idLast, 1, 3);
        int check = oodResolveSymbolicID(context, dlg, idCheck, 1, 4);

        if ( first != OOD_ID_EXCEPTION && last != OOD_ID_EXCEPTION && check != OOD_ID_EXCEPTION )
        {
            if ( CheckRadioButton(hwnd, first, last, check) == 0 )
            {
                result = (int)GetLastError();
            }
        }

    }
    return result;
}

RexxMethod1(logical_t, rb_checked, OSELF, self)
{
    return (SendMessage(rxGetWindowHandle(context, self), BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
}

CSTRING getIsChecked(HWND hwnd)
{
    char * state = "UNKNOWN";
    BUTTONTYPE type = getButtonInfo(hwnd, NULL, NULL);

    if ( type == check || type == radio )
    {
        switch ( SendMessage(hwnd, BM_GETCHECK, 0, 0) )
        {
            case BST_CHECKED :
                state = "CHECKED";
                break;
            case BST_UNCHECKED :
                state = "UNCHECKED";
                break;
            case BST_INDETERMINATE :
                state = getButtonInfo(hwnd, NULL, NULL) == check ? "INDETERMINATE" : "UNKNOWN";
                break;
            default :
                break;
        }
    }
    return state;

}
RexxMethod1(CSTRING, rb_getCheckState, OSELF, self)
{
    return getIsChecked(rxGetWindowHandle(context, self));
}
RexxMethod1(int, rb_check, OSELF, self)
{
    SendMessage(rxGetWindowHandle(context, self), BM_SETCHECK, BST_CHECKED, 0);
    return 0;
}

RexxMethod1(int, rb_uncheck, OSELF, self)
{
    SendMessage(rxGetWindowHandle(context, self), BM_SETCHECK, BST_UNCHECKED, 0);
    return 0;
}

/* DEPRECATED */
RexxMethod1(CSTRING, rb_isChecked, OSELF, self)
{
    return getIsChecked(rxGetWindowHandle(context, self));
}

/* DEPRECATED */
RexxMethod1(int, rb_indeterminate, OSELF, self)
{
    SendMessage(rxGetWindowHandle(context, self), BM_SETCHECK, BST_INDETERMINATE, 0);
    return 0;
}

RexxMethod1(logical_t, ckbx_isIndeterminate, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    if ( getButtonInfo(hwnd, NULL, NULL) == check  )
    {
        return (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_INDETERMINATE ? 1 : 0);
    }
    return 0;
}

RexxMethod1(int, ckbx_setIndeterminate, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    if ( getButtonInfo(hwnd, NULL, NULL) == check  )
    {
        SendMessage(hwnd, BM_SETCHECK, BST_INDETERMINATE, 0);
    }
    return 0;
}


/* This method is used as a convenient way to test code. */
RexxMethod1(int, bc_test, RexxObjectPtr, obj)
{
    return 0;
}


/**
 * Methods for the .ImageList class.
 */
#define IMAGELIST_CLASS "ImageList"


HIMAGELIST rxGetImageList(RexxMethodContext *context, RexxObjectPtr il, int argPos)
{
    HIMAGELIST himl = NULL;
    if ( requiredClass(context, il, "ImageList", argPos) )
    {
        // Make sure we don't use a null ImageList.
        himl = (HIMAGELIST)context->ObjectToCSelf(il);
        if ( himl == NULL )
        {
            nullObjectException(context, IMAGELISTCLASS, argPos);
        }
    }
    return himl;
}

RexxObjectPtr rxNewImageList(RexxMethodContext *c, HIMAGELIST himl)
{
    RexxObjectPtr imageList = NULL;

    RexxClassObject theClass = rxGetContextClass(c, "IMAGELIST");
    if ( theClass != NULL )
    {
        imageList = c->SendMessage1(theClass, "NEW", c->NewPointer(himl));
    }
    return imageList;
}

/** ImageList::init()
 *
 *
 *  @note  As far as I can see, all of the ImageList_xxx functions do not blow
 *         up if an invalid handle is used, even if it is null.  We could just
 *         set CSELF to the pointer value unconditionally and not have to worry
 *         about an interpreter crash.  The ooRexx programmer would just have a
 *         .ImageList object that didn't work.
 *
 *         A valid image list can be released and then becomes invalid (isNull
 *         returns true.)  Since this is the same behavior as .ResourceImage and
 *         .Image objects, both of which allow a null object to be instantiated
 *         (isNull returns true,) we allow a null ImageList to be created.
 *
 *         However, if p is not null, we test that p is actually a valid
 *         ImageList and raise an exception if it is not. All image lists have a
 *         size, if ImageListGetIconSize() fails, then p is not an image list
 *         handle.
 */
RexxMethod1(RexxObjectPtr, il_init, POINTER, p)
{
    if ( p == NULL )
    {
        context->SetObjectVariable("CSELF", context->NewPointer(NULL));
        goto out;
    }
    HIMAGELIST himl = (HIMAGELIST)p;

    // Test that the pointer is really a valid handle to an image list.
    int cx = 2, cy = 2;
    if ( ! ImageList_GetIconSize(himl, &cx, &cy) )
    {
        invalidTypeException(context, 1, "ImageList handle");
        goto out;
    }
    context->SetObjectVariable("CSELF", context->NewPointer(himl));

out:
    return NULLOBJECT;
}

RexxMethod4(RexxObjectPtr, il_create_cls, OPTIONAL_RexxObjectPtr, size,  OPTIONAL_uint32_t, flags,
            OPTIONAL_int32_t, count, OPTIONAL_int32_t, grow)
{
    RexxMethodContext *c = context;
    RexxObjectPtr result = c->Nil();

    SIZE s = {0};
    if ( argumentExists(1) )
    {
        SIZE *p = rxGetSize(c, size, 3);
        if ( p == NULL )
        {
            goto out;
        }
        s.cx = p->cx;
        s.cy = p->cy;
    }
    else
    {
        s.cx = GetSystemMetrics(SM_CXICON);
        s.cy = GetSystemMetrics(SM_CYICON);
    }

    if ( argumentExists(2) )
    {
        if ( (flags & (ILC_MIRROR | ILC_PERITEMMIRROR)) && (! requiredComCtl32Version(c, "init", COMCTL32_6_0)) )
        {
            goto out;
        }
    }
    else
    {
        flags = IL_DEFAULT_FLAGS;
    }

    if ( argumentOmitted(3) )
    {
        count = IL_DEFAULT_COUNT;
    }
    if ( argumentOmitted(4) )
    {
        grow = IL_DEFAULT_GROW;
    }

    HIMAGELIST himl = ImageList_Create(s.cx, s.cy, flags, count, grow);
    result = rxNewImageList(c, himl);

out:
    return result;
}

RexxMethod3(int, il_add, RexxObjectPtr, image, OPTIONAL_RexxObjectPtr, optMask, CSELF, il)
{
    RexxMethodContext *c = context;
    int result = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(c, IMAGELISTCLASS);
        goto out;
    }

    POODIMAGE oi = rxGetImageBitmap(c, image, 1);
    if ( oi == NULL )
    {
        goto out;
    }

    HBITMAP mask = NULL;
    if ( argumentExists(2) )
    {
        POODIMAGE tmp = rxGetImageBitmap(c, optMask, 2);
        if ( tmp == NULL )
        {
            goto out;
        }
        mask =  (HBITMAP)tmp->hImage;
    }

    result = ImageList_Add(himl, (HBITMAP)oi->hImage, mask);

out:
    return result;
}

RexxMethod3(int, il_addMasked, RexxObjectPtr, image, uint32_t, cRef, CSELF, il)
{
    int result = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(context, IMAGELISTCLASS);
        goto out;
    }

    POODIMAGE oi = rxGetImageBitmap(context, image, 1);
    if ( oi == NULL )
    {
        goto out;
    }
    result = ImageList_AddMasked(himl, (HBITMAP)oi->hImage, cRef);

out:
    return result;
}

RexxMethod2(int, il_addIcon, RexxObjectPtr, image, CSELF, il)
{
    int result = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(context, IMAGELISTCLASS);
        goto out;
    }
    POODIMAGE oi = rxGetImageIcon(context, image, 1);
    if ( oi == NULL )
    {
        goto out;
    }
    result = ImageList_AddIcon(himl, (HICON)oi->hImage);

out:
    return result;
}

RexxMethod3(int, il_addImages, RexxArrayObject, images, OPTIONAL_uint32_t, cRef, CSELF, il)
{
    RexxMethodContext *c = context;
    int result = -1;
    int tmpResult = -1;

    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl == NULL )
    {
        nullObjectException(context, IMAGELISTCLASS);
        goto out;
    }
    size_t count = c->ArraySize(images);
    if ( count < 1 )
    {
        emptyArrayException(c, 1);
        goto out;
    }

    uint8_t imageType = 0;
    bool doMasked = false;

    for ( size_t i = 1; i <= count; i++)
    {
        RexxObjectPtr image = c->ArrayAt(images, i);
        if ( image == NULLOBJECT || ! c->IsOfType(image, "Image") )
        {
            wrongObjInArrayException(c, 1, i, "Image");
            goto out;
        }
        POODIMAGE oi = (POODIMAGE)context->ObjectToCSelf(image);
        if ( oi->hImage == NULL )
        {
            wrongObjInArrayException(c, 1, i, "non-null Image");
            goto out;
        }

        if ( imageType == 0 )
        {
            imageType = -1;
            if ( oi->type == IMAGE_CURSOR || oi->type == IMAGE_ICON )
            {
                imageType = IMAGE_ICON;
            }
            else if ( oi->type == IMAGE_BITMAP )
            {
                imageType = IMAGE_BITMAP;
                doMasked = argumentExists(2) ? true : false;
            }

            if ( imageType == -1 )
            {
                wrongObjInArrayException(c, 1, i, "bitmap, icon, or cursor Image");
                goto out;
            }
        }

        switch ( oi->type )
        {
            case IMAGE_ICON :
            case IMAGE_CURSOR :
                if ( imageType != IMAGE_ICON )
                {
                    wrongObjInArrayException(c, 1, i, "cursor or icon Image");
                    goto out;
                }
                tmpResult = ImageList_AddIcon(himl, (HICON)oi->hImage);
                break;

            case IMAGE_BITMAP :
                if ( imageType != IMAGE_BITMAP )
                {
                    wrongObjInArrayException(c, 1, i, "bitmap Image");
                    goto out;
                }
                if ( doMasked )
                {
                    tmpResult = ImageList_AddMasked(himl, (HBITMAP)oi->hImage, cRef);
                }
                else
                {
                    tmpResult = ImageList_Add(himl, (HBITMAP)oi->hImage, NULL);
                }
                break;

            default :
                wrongObjInArrayException(c, 1, i, "bitmap, icon, or cursor Image");
                goto out;

        }

        if ( tmpResult == -1 )
        {
            break;
        }
        result = tmpResult;
    }

out:
    return result;
}

/** ImageList::getCount()
 *
 *
 *  @return  The count of images in the image list.
 */
RexxMethod1(int, il_getCount, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        return ImageList_GetImageCount(himl);
    }
    nullObjectException(context, IMAGELISTCLASS);
    return NULL;
}

/** ImageList::getImageSize()
 *
 *
 * @return  A .Size object containing the size of an image on success, or .nil
 *          on failure.
 */
RexxMethod1(RexxObjectPtr, il_getImageSize, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        SIZE s;
        if ( ImageList_GetIconSize(himl, (int *)&s.cx, (int *)&s.cy) == 0 )
        {
            return context->Nil();
        }
        else
        {
            return rxNewSize(context, s.cx, s.cy);
        }
    }
    nullObjectException(context, IMAGELISTCLASS);
    return NULL;
}

RexxMethod1(RexxObjectPtr, il_duplicate, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        return rxNewImageList(context, ImageList_Duplicate(himl));
    }
    nullObjectException(context, IMAGELISTCLASS);
    return NULL;
}

RexxMethod2(logical_t, il_remove, int, index, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        return ImageList_Remove(himl, index);
    }
    nullObjectException(context, IMAGELISTCLASS);
    return NULL;
}

RexxMethod1(logical_t, il_removeAll, CSELF, il)
{
    HIMAGELIST himl = (HIMAGELIST)il;
    if ( himl != NULL )
    {
        return ImageList_RemoveAll(himl);
    }
    nullObjectException(context, IMAGELISTCLASS);
    return NULL;
}

RexxMethod1(uint32_t, il_release, CSELF, il)
{
    if ( il != NULL )
    {
        ImageList_Destroy((HIMAGELIST)il);
        context->SetObjectVariable("CSELF", context->NewPointer(NULL));
    }
    return 0;
}

RexxMethod1(POINTER, il_handle, CSELF, il)
{
    if ( il == NULL )
    {
        nullObjectException(context, IMAGELISTCLASS);
    }
    return il;
}

RexxMethod1(logical_t, il_isNull, CSELF, il) { return ( il == NULL);  }


/**
 * Methods for the .Image class.
 */
#define IMAGE_CLASS "Image"

CSTRING getImageTypeName(uint8_t type)
{
    switch ( type )
    {
        case IMAGE_ICON :
            return "Icon";
        case IMAGE_BITMAP :
            return "Bitmap";
        case IMAGE_CURSOR :
            return "Cursor";
        case IMAGE_ENHMETAFILE :
            return "Enhanced Metafile";
        default :
            return "Unknown";
    }
}

POODIMAGE rxGetOodImage(RexxMethodContext *context, RexxObjectPtr o, int argPos)
{
    if ( requiredClass(context, o, "Image", argPos) )
    {
        POODIMAGE oi = (POODIMAGE)context->ObjectToCSelf(o);
        if ( oi->isValid )
        {
            return oi;
        }
        nullObjectException(context, IMAGECLASS, argPos);
    }
    return NULL;
}

/**
 * Extracts a valid oodImage pointer from a RexxObjectPtr, ensuring that the
 * image is either an icon or a cursor.  (Cursors are icons.)
 *
 * @param c    The method context we are executing in.
 * @param o    The, assumed, .Image object.
 * @param pos  The argument position in the invocation from ooRexx.  Used for
 *             exception messages.
 *
 * @return A pointer to an OODIMAGE struct on success, othewise NULL.
 */
POODIMAGE rxGetImageIcon(RexxMethodContext *c, RexxObjectPtr o, int pos)
{
    POODIMAGE oi = rxGetOodImage(c, o, pos);
    if ( oi != NULL && (oi->type == IMAGE_ICON || oi->type == IMAGE_CURSOR) )
    {
        return oi;
    }
    wrongArgValueException(c, pos, "Icon, Cursor", oi->typeName);
    return NULL;
}

POODIMAGE rxGetImageBitmap(RexxMethodContext *c, RexxObjectPtr o, int pos)
{
    POODIMAGE oi = rxGetOodImage(c, o, pos);
    if ( oi != NULL && oi->type != IMAGE_BITMAP )
    {
        invalidImageException(c, pos, "Bitmap", oi->typeName);
        return NULL;
    }
    return oi;
}

RexxObjectPtr rxNewImageObject(RexxMethodContext *c, RexxBufferObject bufferObj)
{
    RexxObjectPtr image = NULLOBJECT;

    RexxClassObject ImageClass = rxGetContextClass(c, "Image");
    if ( ImageClass != NULL )
    {
        image = c->SendMessage1(ImageClass, "NEW", bufferObj);
    }
    return image;
}

RexxObjectPtr rxNewEmptyImage(RexxMethodContext *c, DWORD rc)
{
    RexxBufferObject bufferObj = c->NewBuffer(sizeof(OODIMAGE));
    POODIMAGE cself = (POODIMAGE)c->BufferData(bufferObj);

    // Set everything to invalid.
    memset(cself, 0, sizeof(OODIMAGE));
    cself->type = -1;
    cself->size.cx = -1;
    cself->size.cy = -1;
    cself->lastError = rc;

    return rxNewImageObject(c, bufferObj);
}

/**
 * Creates an .Image object from an image handle retrieved from a dialog
 * control.
 *
 * If the image had been set from ooDialog code, the .Image object would be
 * known.  Therefore, this an image assigned to the control, loaded from a
 * resource DLL.  The assumption then is, that the OS loaded the image as
 * LR_SHARED.  (Is this true?)
 *
 * We need to create an .Image object. If the image type is not passed in,
 * (type=-1,) we can deduce the type (possibly) from the control style, but not
 * the size or all the flags.  However, we do use the LR_SHARED flag based on
 * the above assumption.
 *
 * When the process that loaded the image ends, the OS will clean up the image
 * resource (MSDN.)  Using LR_SHARED will prevent the user from releasing an
 * image that shouldn't be.
 *
 * @param c       Method context we are operating in.
 * @param hwnd    Window handle of the dialog control.
 * @param hImage  Handle to the image.
 * @param type    Image type.
 * @param ctrl    Type of dialog control.
 *
 * @return   A new .Image object.
 */
RexxObjectPtr rxNewImageFromControl(RexxMethodContext *c, HWND hwnd, HANDLE hImage, uint8_t type,
                                    oodControl_t ctrl)
{
    SIZE s = {0};

    // If the caller did not know the type, try to deduce it.
    if ( type == -1 )
    {
        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        switch ( ctrl )
        {
            case oodcStatic :
                // If it is a cursor image, the control type is SS_ICON.
                switch ( style & SS_TYPEMASK )
                {
                    case SS_BITMAP :
                        type = IMAGE_BITMAP;
                        break;
                    case SS_ENHMETAFILE :
                        type = IMAGE_ENHMETAFILE;
                        break;
                    case SS_ICON :
                    default :
                        type = IMAGE_ICON;
                        break;
                }
                break;

            case oodcButton :
                switch ( style & BS_IMAGEMASK )
                {
                    case BS_BITMAP :
                        type = IMAGE_BITMAP;
                        break;
                    case BS_ICON :
                    default :
                        type = IMAGE_ICON;
                        break;
                }
                break;

            default :
                // Shouldn't happen
                type = IMAGE_BITMAP;
                break;

        }
    }
    return rxNewValidImage(c, hImage, type, &s, LR_SHARED, false);
}

/**
 * Instantiates a new, non-null, .Image object.
 *
 * @param context
 * @param hImage
 * @param type
 * @param s
 * @param flags
 * @param src       True, ooDialog created using LoadImage(). False created from
 *                  a handle (so type, size, flags may not be correct.)
 *
 * @return  A new .Image object.
 */
RexxObjectPtr rxNewValidImage(RexxMethodContext *c, HANDLE hImage, uint8_t type, PSIZE s, uint32_t flags, bool src)
{
    RexxBufferObject bufferObj = c->NewBuffer(sizeof(OODIMAGE));
    POODIMAGE cself = (POODIMAGE)c->BufferData(bufferObj);

    cself->hImage = hImage;
    cself->type = type;
    cself->size.cx = s->cx;
    cself->size.cy = s->cy;
    cself->flags = flags;
    cself->isValid = true;
    cself->srcOOD = src;
    cself->canRelease = ! (flags & LR_SHARED);
    cself->typeName = getImageTypeName(type);
    cself->lastError = 0;
    cself->fileName = "";

    return rxNewImageObject(c, bufferObj);
}

/**
 * Removes and releases all .Image objects in an .Array object.  This is an
 * internal method assuming the args are correct.  The array must contain *only*
 * .Image objects, but can be a sparse array.
 *
 *
 * @param c
 * @param a
 * @param last
 */
void rxReleaseAllImages(RexxMethodContext *c, RexxArrayObject a, size_t last)
{
    for ( size_t i = 1; i <= last; i++)
    {
        RexxObjectPtr image = c->ArrayAt(a, i);
        if ( image != NULLOBJECT )
        {
            c->SendMessage0(image, "RELEASE");
        }
    }
}

RexxArrayObject rxImagesFromArrayOfInts(RexxMethodContext *c, RexxArrayObject ids, HINSTANCE hModule,
                                        uint8_t type, PSIZE s, uint32_t flags)
{
    int resourceID;
    size_t count = c->ArraySize(ids);
    RexxArrayObject result = c->NewArray(count);

    for ( size_t i = 1; i <= count; i++ )
    {
        RexxObjectPtr id = c->ArrayAt(ids, i);
        if ( id == NULLOBJECT || ! c->Int32(id, &resourceID) )
        {
            // Shared images should not be released.
            if ( (flags & LR_SHARED) == 0 )
            {
                rxReleaseAllImages(c, result, i - 1);
            }
            wrongObjInArrayException(c, 1, i, "number");
            result = NULLOBJECT;
            goto out;
        }

        HANDLE hImage = LoadImage(hModule, MAKEINTRESOURCE(resourceID), type, s->cx, s->cy, flags);
        if ( hImage == NULL )
        {
            // Set the system error code and leave this slot in the array blank.
            oodSetSysErrCode(c);
        }
        else
        {
            // Theoretically, image could come back null, but this seems very
            // unlikely.  Still, we'll check for it.  If it is null, an
            // exception has already been raised.
            RexxObjectPtr image = rxNewValidImage(c, hImage, type, s, flags, true);
            if ( image == NULLOBJECT )
            {
                if ( (flags & LR_SHARED) == 0 )
                {
                    rxReleaseAllImages(c, result, i - 1);
                }
                result = NULLOBJECT;
                goto out;
            }
            c->ArrayPut(result, image, i);
        }
    }
out:
    return result;
}

bool getStandardImageArgs(RexxMethodContext *context, uint8_t *type, uint8_t defType, RexxObjectPtr size,
                          SIZE *defSize, uint32_t *flags, uint32_t defFlags)
{
    oodSetSysErrCode(context, 0);

    if ( argumentOmitted(2) )
    {
        *type = defType;
    }

    if ( argumentExists(3) )
    {
        SIZE *p = rxGetSize(context, size, 3);
        if ( p == NULL )
        {
            return false;
        }
        defSize->cx = p->cx;
        defSize->cy = p->cy;
    }

    if ( argumentOmitted(4) )
    {
        *flags = defFlags;
    }
    else
    {
        // The user specified flags.  Use some safeguards, determined by the
        // value of the default flags.  In all other cases, assume the user
        // knows best.

        if ( defFlags == LR_LOADFROMFILE )
        {
            // Ensure the user did not use shared and did use load from file.
            *flags = (*flags &  ~LR_SHARED) | LR_LOADFROMFILE;
        }
        else if ( defFlags == (LR_SHARED | LR_DEFAULTSIZE) )
        {
            // Ensure the user did not use load from file and did use shared.
            *flags = (*flags &  ~LR_LOADFROMFILE) | LR_SHARED;
        }
    }
    return true;
}

/**
 * Look up the int value of a string.
 */
int getConstantValue(String2Int *cMap, const char * str)
{
    String2Int::iterator itr;
    itr = cMap->find(str);
    if ( itr != cMap->end() )
    {
        return itr->second;
    }
    return -1;
}

/**
 * Initializes the string to int map for IDs and flags used by images and image
 * lists.  This will included things like a button control's alignment flags for
 * an image list, image list creation flags, OEM icon IDs, etc..
 *
 * @return String2Int*
 *
 * @note  All IDs are included here, except the obsolete ones, and things like
 *        OBM_OLD*, all of which were for 16-bit Windows.
 */
static String2Int *imageInitMap(void)
{
    String2Int *cMap = new String2Int;

    cMap->insert(String2Int::value_type("IDI_APPLICATION", 32512));
    cMap->insert(String2Int::value_type("IDI_HAND",        32513));
    cMap->insert(String2Int::value_type("IDI_QUESTION",    32514));
    cMap->insert(String2Int::value_type("IDI_EXCLAMATION", 32515));
    cMap->insert(String2Int::value_type("IDI_ASTERISK",    32516));
    cMap->insert(String2Int::value_type("IDI_WINLOGO",     32517));

    cMap->insert(String2Int::value_type("IMAGE_BITMAP",      0));
    cMap->insert(String2Int::value_type("IMAGE_ICON",        1));
    cMap->insert(String2Int::value_type("IMAGE_CURSOR",      2));
    cMap->insert(String2Int::value_type("IMAGE_ENHMETAFILE", 3));

    cMap->insert(String2Int::value_type("OCR_NORMAL",      32512));
    cMap->insert(String2Int::value_type("OCR_IBEAM",       32513));
    cMap->insert(String2Int::value_type("OCR_WAIT",        32514));
    cMap->insert(String2Int::value_type("OCR_CROSS",       32515));
    cMap->insert(String2Int::value_type("OCR_UP",          32516));
    cMap->insert(String2Int::value_type("OCR_SIZENWSE",    32642));
    cMap->insert(String2Int::value_type("OCR_SIZENESW",    32643));
    cMap->insert(String2Int::value_type("OCR_SIZEWE",      32644));
    cMap->insert(String2Int::value_type("OCR_SIZENS",      32645));
    cMap->insert(String2Int::value_type("OCR_SIZEALL",     32646));
    cMap->insert(String2Int::value_type("OCR_NO",          32648));
    cMap->insert(String2Int::value_type("OCR_HAND",        32649));
    cMap->insert(String2Int::value_type("OCR_APPSTARTING", 32650));

    cMap->insert(String2Int::value_type("OBM_CLOSE",      32754));
    cMap->insert(String2Int::value_type("OBM_UPARROW",    32753));
    cMap->insert(String2Int::value_type("OBM_DNARROW",    32752));
    cMap->insert(String2Int::value_type("OBM_RGARROW",    32751));
    cMap->insert(String2Int::value_type("OBM_LFARROW",    32750));
    cMap->insert(String2Int::value_type("OBM_REDUCE",     32749));
    cMap->insert(String2Int::value_type("OBM_ZOOM",       32748));
    cMap->insert(String2Int::value_type("OBM_RESTORE",    32747));
    cMap->insert(String2Int::value_type("OBM_REDUCED",    32746));
    cMap->insert(String2Int::value_type("OBM_ZOOMD",      32745));
    cMap->insert(String2Int::value_type("OBM_RESTORED",   32744));
    cMap->insert(String2Int::value_type("OBM_UPARROWD",   32743));
    cMap->insert(String2Int::value_type("OBM_DNARROWD",   32742));
    cMap->insert(String2Int::value_type("OBM_RGARROWD",   32741));
    cMap->insert(String2Int::value_type("OBM_LFARROWD",   32740));
    cMap->insert(String2Int::value_type("OBM_MNARROW",    32739));
    cMap->insert(String2Int::value_type("OBM_COMBO",      32738));
    cMap->insert(String2Int::value_type("OBM_UPARROWI",   32737));
    cMap->insert(String2Int::value_type("OBM_DNARROWI",   32736));
    cMap->insert(String2Int::value_type("OBM_RGARROWI",   32735));
    cMap->insert(String2Int::value_type("OBM_LFARROWI",   32734));
    cMap->insert(String2Int::value_type("OBM_SIZE",       32766));
    cMap->insert(String2Int::value_type("OBM_BTSIZE",     32761));
    cMap->insert(String2Int::value_type("OBM_CHECK",      32760));
    cMap->insert(String2Int::value_type("OBM_CHECKBOXES", 32759));
    cMap->insert(String2Int::value_type("OBM_BTNCORNERS", 32758));

    cMap->insert(String2Int::value_type("LR_DEFAULTCOLOR",     0x0000));
    cMap->insert(String2Int::value_type("LR_MONOCHROME",       0x0001));
    cMap->insert(String2Int::value_type("LR_COLOR",            0x0002));
    cMap->insert(String2Int::value_type("LR_COPYRETURNORG",    0x0004));
    cMap->insert(String2Int::value_type("LR_COPYDELETEORG",    0x0008));
    cMap->insert(String2Int::value_type("LR_LOADFROMFILE",     0x0010));
    cMap->insert(String2Int::value_type("LR_LOADTRANSPARENT",  0x0020));
    cMap->insert(String2Int::value_type("LR_DEFAULTSIZE",      0x0040));
    cMap->insert(String2Int::value_type("LR_VGACOLOR",         0x0080));
    cMap->insert(String2Int::value_type("LR_LOADMAP3DCOLORS",  0x1000));
    cMap->insert(String2Int::value_type("LR_CREATEDIBSECTION", 0x2000));
    cMap->insert(String2Int::value_type("LR_COPYFROMRESOURCE", 0x4000));
    cMap->insert(String2Int::value_type("LR_SHARED",           0x8000));

    // ImageList_Create flags
    cMap->insert(String2Int::value_type("ILC_MASK", 0x0001));
    cMap->insert(String2Int::value_type("ILC_COLOR", 0x0000));
    cMap->insert(String2Int::value_type("ILC_COLORDDB", 0x00FE));
    cMap->insert(String2Int::value_type("ILC_COLOR4", 0x0004));
    cMap->insert(String2Int::value_type("ILC_COLOR8", 0x0008));
    cMap->insert(String2Int::value_type("ILC_COLOR16", 0x0010));
    cMap->insert(String2Int::value_type("ILC_COLOR24", 0x0018));
    cMap->insert(String2Int::value_type("ILC_COLOR32", 0x0020));
    cMap->insert(String2Int::value_type("ILC_PALETTE", 0x0800));
    cMap->insert(String2Int::value_type("ILC_MIRROR", 0x2000));
    cMap->insert(String2Int::value_type("ILC_PERITEMMIRROR", 0x8000));

    // Button image list alignment values
    cMap->insert(String2Int::value_type("BUTTON_IMAGELIST_ALIGN_LEFT",   0));
    cMap->insert(String2Int::value_type("BUTTON_IMAGELIST_ALIGN_RIGHT",  1));
    cMap->insert(String2Int::value_type("BUTTON_IMAGELIST_ALIGN_TOP",    2));
    cMap->insert(String2Int::value_type("BUTTON_IMAGELIST_ALIGN_BOTTOM", 3));
    cMap->insert(String2Int::value_type("BUTTON_IMAGELIST_ALIGN_CENTER", 4));

    cMap->insert(String2Int::value_type("LVSIL_NORMAL", 0));
    cMap->insert(String2Int::value_type("LVSIL_SMALL", 1));
    cMap->insert(String2Int::value_type("LVSIL_STATE", 2));

    cMap->insert(String2Int::value_type("TVSIL_NORMAL", 0));
    cMap->insert(String2Int::value_type("TVSIL_STATE", 2));

    //cMap->insert(String2Int::value_type("", ));

    return cMap;
}

RexxMethod1(uint32_t, image_id_cls, CSTRING, id)
{
    static String2Int *imageConstantsMap = NULL;

    if ( imageConstantsMap == NULL )
    {
        imageConstantsMap = imageInitMap();
    }
    int idValue = getConstantValue(imageConstantsMap, id);
    if ( idValue == -1 )
    {
        wrongArgValueException(context, 1, "the Image class symbol IDs", id);
    }
    return (uint32_t)idValue;
}


/** Image::getImage()  [class method]
 *
 *  Load a stand alone image from a file or one of the system images.
 *
 *  @param   id  Either the numeric resource id of a system image, or the file
 *               name of a stand-alone image file.
 *
 *  @note  This method is designed to always return an .Image object, or raise
 *         an exception.  The user would need to test the returned .Image object
 *         for null to be sure it is good.  I.e.:
 *
 *        image = .Image~getImage(...)
 *        if image~isNull then do
 *          -- error
 *        end
 */
RexxMethod4(RexxObjectPtr, image_getImage_cls, RexxObjectPtr, id, OPTIONAL_uint8_t, type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_uint32_t, flags)
{
    RexxObjectPtr result = NULLOBJECT;
    SIZE s = {0};

    bool fromFile = true;
    LPCTSTR name = NULL;

    if ( context->IsString(id) )
    {
        name = context->StringData((RexxStringObject)id);
    }
    else
    {
        int resourceID;

        if ( ! context->Int32(id, &resourceID) )
        {
            wrongArgValueException(context, 1, "either an image file name, or a system image ID", id);
            goto out;
        }
        name = MAKEINTRESOURCE(resourceID);
        fromFile = false;
    }

    if ( ! getStandardImageArgs(context, &type, IMAGE_BITMAP, size, &s, &flags,
                                fromFile ? LR_LOADFROMFILE : LR_SHARED | LR_DEFAULTSIZE) )
    {
        goto out;
    }

    HANDLE hImage = LoadImage(NULL, name, type, s.cx, s.cy, flags);
    if ( hImage == NULL )
    {
        DWORD rc = GetLastError();
        oodSetSysErrCode(context, rc);
        result = rxNewEmptyImage(context, rc);
        goto out;
    }

    result = rxNewValidImage(context, hImage, type, &s, flags, true);

out:
    return result;
}


RexxMethod4(RexxObjectPtr, image_fromFiles_cls, RexxArrayObject, files, OPTIONAL_uint8_t, type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_uint32_t, flags)
{
    RexxMethodContext *c = context;
    RexxArrayObject result = NULLOBJECT;
    SIZE s = {0};

    if ( ! getStandardImageArgs(context, &type, IMAGE_BITMAP, size, &s, &flags, LR_LOADFROMFILE) )
    {
        goto out;
    }

    size_t count = c->ArraySize(files);
    result = c->NewArray(count);

    for ( size_t i = 1; i <= count; i++ )
    {
        RexxObjectPtr f = c->ArrayAt(files, i);
        if ( f == NULLOBJECT || ! context->IsString(f) )
        {
            rxReleaseAllImages(c, result, i - 1);
            c->RaiseException1(Rexx_Error_Incorrect_method_nostring_inarray , c->WholeNumber(1));
            result = NULLOBJECT;
            goto out;
        }

        const char *file = context->ObjectToStringValue(f);
        HANDLE hImage = LoadImage(NULL, file, type, s.cx, s.cy, flags);
        if ( hImage == NULL )
        {
            // Set the system error code and leave this slot in the array blank.
            oodSetSysErrCode(context);
        }
        else
        {
            // Theoretically, image could come back null, but this seems very
            // unlikely.  Still, we'll check for it.  If it is null, an
            // exception has already been raised.
            RexxObjectPtr image = rxNewValidImage(context, hImage, type, &s, flags, true);
            if ( image == NULLOBJECT )
            {
                rxReleaseAllImages(c, result, i - 1);
                result = NULLOBJECT;
                goto out;
            }
            c->ArrayPut(result, image, i);
        }
    }

out:
    return result;
}

RexxMethod4(RexxObjectPtr, image_fromIDs_cls, RexxArrayObject, ids, OPTIONAL_uint8_t, type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_uint32_t, flags)
{
    RexxMethodContext *c = context;
    RexxArrayObject result = NULLOBJECT;
    SIZE s = {0};

    if ( ! getStandardImageArgs(context, &type, IMAGE_ICON, size, &s, &flags, LR_SHARED | LR_DEFAULTSIZE) )
    {
        goto out;
    }

    result = rxImagesFromArrayOfInts(context, ids, NULL, type, &s, flags);

out:
    return result;
}

/** Image::colorRef()  [class]
 *
 *  Returns a COLORREF composed from the specified RGB valuses.
 *
 *  @param r  The red component, or special case CLR_DEFAULT / CLR_NONE.
 *  @param g  The green component
 *  @param b  The blue component
 *
 *  @return The COLORREF.
 *
 *  @note  For any omitted arg, the value of the arg will be 0.  Since 0 is the
 *         default value for the g and b args, we do not need to check for
 *         ommitted args.
 */
RexxMethod3(uint32_t, image_colorRef_cls, OPTIONAL_RexxObjectPtr, rVal,
            OPTIONAL_uint8_t, g, OPTIONAL_uint8_t, b)
{
    uint8_t r = 0;
    if ( argumentExists(1) )
    {
        CSTRING tmp = context->ObjectToStringValue(rVal);
        if ( *tmp && toupper(*tmp) == 'C' )
        {
            if ( stricmp(tmp, "CLR_DEFAULT") == 0 )
            {
                return CLR_DEFAULT;
            }
            else if ( stricmp(tmp, "CLR_NONE") == 0 )
            {
                return CLR_NONE;
            }
            else
            {
                goto error_out;
            }
        }

        uint32_t tmpR;
        if ( ! context->ObjectToUnsignedInt32(rVal, &tmpR) || tmpR > 255 )
        {
            goto error_out;
        }
        r = (uint8_t)tmpR;
    }
    return RGB(r, g, b);

error_out:
    wrongArgValueException(context, 1, "CLR_DEFAULT, CLR_NONE, or a number from 0 through 255", rVal);
    return 0;
}

RexxMethod1(uint8_t, image_getRValue_cls, uint32_t, colorRef) { return GetRValue(colorRef); }
RexxMethod1(uint8_t, image_getGValue_cls, uint32_t, colorRef) { return GetGValue(colorRef); }
RexxMethod1(uint8_t, image_getBValue_cls, uint32_t, colorRef) { return GetBValue(colorRef); }


/** Image::init()
 *
 *
 */
RexxMethod1(RexxObjectPtr, image_init, RexxObjectPtr, cselfObj)
{
    if ( requiredClass(context, cselfObj, "Buffer", 1) )
    {
        context->SetObjectVariable("CSELF", cselfObj);
    }
    return NULLOBJECT;
}

RexxMethod1(uint32_t, image_release, CSELF, oi)
{
    uint32_t rc = 0;
    POODIMAGE pOI = (POODIMAGE)oi;

    if ( pOI->canRelease )
    {
        switch ( pOI->type )
        {
            case IMAGE_ICON :
                if ( DestroyIcon((HICON)pOI->hImage) == 0 )
                {
                    rc = GetLastError();
                }
                break;

            case IMAGE_BITMAP :
                if ( DeleteObject((HGDIOBJ)pOI->hImage) == 0 )
                {
                    rc = GetLastError();
                }
                break;

            case IMAGE_CURSOR :
                if ( DestroyCursor((HCURSOR)pOI->hImage) == 0 )
                {
                    rc = GetLastError();
                }
                break;

            case IMAGE_ENHMETAFILE :
                // Currently no way in ooDialog to have this type of image.
                // Left for future enhancement.
                break;

            default :
                // Should be impossible.
                break;
        }
    }

    pOI->hImage = NULL;
    pOI->type = -1;
    pOI->size.cx = -1;
    pOI->size.cy = -1;
    pOI->flags = 0;
    pOI->isValid = false;
    pOI->srcOOD = false;
    pOI->canRelease = false;
    pOI->lastError = rc;

    oodSetSysErrCode(context, pOI->lastError);

    return rc;
}

RexxMethod1(POINTER, image_handle, CSELF, oi)
{
    if ( ! ((POODIMAGE)oi)->isValid )
    {
        nullObjectException(context, IMAGECLASS);
    }
    return ((POODIMAGE)oi)->hImage;
}

RexxMethod1(logical_t, image_isNull, CSELF, oi) { return ( ! ((POODIMAGE)oi)->isValid ); }
RexxMethod1(uint32_t, image_systemErrorCode, CSELF, oi) { return ((POODIMAGE)oi)->lastError; }


/**
 * Methods for the ooDialog .ResourceImage class.
 */
#define RESOURCE_IMAGE_CLASS  "ResourceImage"


/** ResouceImage::init()
 *
 *
 */
RexxMethod2(RexxObjectPtr, ri_init, CSTRING, file, OPTIONAL_RexxObjectPtr, dlg)
{
    oodSetSysErrCode(context, 0);

    RexxBufferObject cself = context->NewBuffer(sizeof(RESOURCEIMAGE));
    context->SetObjectVariable("CSELF", cself);

    PRESOURCEIMAGE ri = (PRESOURCEIMAGE)context->BufferData(cself);
    memset(ri, 0, sizeof(RESOURCEIMAGE));

    if ( argumentOmitted(2) )
    {
        ri->hMod = LoadLibraryEx(file, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if ( ri->hMod == NULL )
        {
            ri->lastError = GetLastError();
            oodSetSysErrCode(context, ri->lastError);
        }
        else
        {
            ri->canRelease = true;
            ri->isValid = true;
        }
    }
    else
    {
        if ( ! requiredClass(context, dlg, "PlainBaseDialog", 2) )
        {
            goto err_out;
        }

        if ( stricmp(OODDLL, file) == 0 )
        {
            ri->hMod = MyInstance;
            ri->isValid = true;
        }
        else
        {
            if ( ! requiredClass(context, dlg, "ResDialog", 2) )
            {
                goto err_out;
            }

            DIALOGADMIN *adm = rxGetDlgAdm(context, dlg);
            if ( adm == NULL )
            {
                goto err_out;
            }

            ri->hMod = adm->TheInstance;
            ri->isValid = true;
        }
    }

    return NULLOBJECT;

err_out:

    // 1812 ERROR_RESOURCE_DATA_NOT_FOUND
    // The specified image file did not contain a resource section.

    ri->lastError = 1812;
    oodSetSysErrCode(context, ri->lastError);
    return NULLOBJECT;
}

/** ResourceImage::getImage()
 *
 * Loads an image from this resource binary.
 *
 *
 * @return  An instantiated .Image object, which may be a null Image if an error
 *          occurred.
 *
 * @note  This method is designed to always return an .Image object, or raise an
 *        exception.  The user would need to test the returned .Image object for
 *        null to be sure it is good.  I.e.:
 *
 *        mod = .ResourceImage~new(...)
 *        ...
 *        image = mod~getImage(...)
 *        if image~isNull then do
 *          -- error
 *        end
 */
RexxMethod5(RexxObjectPtr, ri_getImage, int, id, OPTIONAL_uint8_t, type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_uint32_t, flags, CSELF, cself)
{

    RexxObjectPtr result = NULLOBJECT;
    SIZE s = {0};

    PRESOURCEIMAGE ri = (PRESOURCEIMAGE)cself;
    if ( ! ri->isValid )
    {
        nullObjectException(context, RESOURCEIMAGECLASS);
        goto out;
    }
    ri->lastError = 0;

    if ( ! getStandardImageArgs(context, &type, IMAGE_BITMAP, size, &s, &flags, LR_SHARED) )
    {
        goto out;
    }

    HANDLE hImage = LoadImage(ri->hMod, MAKEINTRESOURCE(id), type, s.cx, s.cy, flags);
    if ( hImage == NULL )
    {
        ri->lastError = GetLastError();
        oodSetSysErrCode(context, ri->lastError);
        result = rxNewEmptyImage(context, ri->lastError);
        goto out;
    }

    result = rxNewValidImage(context, hImage, type, &s, flags, true);

out:
    return result;
}

RexxMethod5(RexxObjectPtr, ri_getImages, RexxArrayObject, ids, OPTIONAL_uint8_t, type,
            OPTIONAL_RexxObjectPtr, size, OPTIONAL_uint32_t, flags, CSELF, cself)
{
    RexxObjectPtr result = NULLOBJECT;
    SIZE s = {0};

    PRESOURCEIMAGE ri = (PRESOURCEIMAGE)cself;
    if ( ! ri->isValid )
    {
        nullObjectException(context, RESOURCEIMAGECLASS);
        goto out;
    }
    ri->lastError = 0;

    if ( ! getStandardImageArgs(context, &type, IMAGE_BITMAP, size, &s, &flags, LR_SHARED) )
    {
        goto out;
    }

    result = rxImagesFromArrayOfInts(context, ids, ri->hMod, type, &s, flags);
    if ( result == NULLOBJECT )
    {
        ri->lastError = oodGetSysErrCode(context);
    }

out:
    return result;
}

RexxMethod1(uint32_t, ri_release, CSELF, r)
{
    uint32_t rc = 0;
    PRESOURCEIMAGE ri = (PRESOURCEIMAGE)r;

    if ( ri->canRelease )
    {
        if ( ! FreeLibrary((HMODULE)ri->hMod) )
        {
            rc = GetLastError();
        }
    }

    ri->canRelease = false;
    ri->isValid = false;
    ri->hMod = NULL;
    ri->lastError = rc;
    oodSetSysErrCode(context, ri->lastError);

    return rc;
}

RexxMethod1(POINTER, ri_handle, CSELF, ri)
{
    if ( ! ((PRESOURCEIMAGE)ri)->isValid )
    {
        nullObjectException(context, RESOURCEIMAGECLASS);
    }
    return ((PRESOURCEIMAGE)ri)->hMod;
}

RexxMethod1(logical_t, ri_isNull, CSELF, ri) { return ( ! ((PRESOURCEIMAGE)ri)->isValid); }
RexxMethod1(uint32_t, ri_systemErrorCode, CSELF, ri) { return ((PRESOURCEIMAGE)ri)->lastError; }


/**
 * Methods for the ooDialog .Point class.
 */
#define POINT_CLASS  "Point"


RexxMethod2(RexxObjectPtr, point_init, OPTIONAL_int32_t,  x, OPTIONAL_int32_t, y)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(POINT));
    context->SetObjectVariable("CSELF", obj);

    POINT *p = (POINT *)context->BufferData(obj);

    p->x = argumentExists(1) ? x : 0;
    p->y = argumentExists(2) ? y : p->x;

    return NULLOBJECT;
}

RexxMethod1(int32_t, point_x, CSELF, p) { return ((POINT *)p)->x; }
RexxMethod1(int32_t, point_y, CSELF, p) { return ((POINT *)p)->y; }
RexxMethod2(RexxObjectPtr, point_setX, CSELF, p, int32_t, x) { ((POINT *)p)->x = x; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, point_setY, CSELF, p, int32_t, y) { ((POINT *)p)->y = y; return NULLOBJECT; }

/**
 * Methods for the ooDialog .Size class.
 */
#define SIZE_CLASE  "Size"

RexxMethod2(RexxObjectPtr, size_init, OPTIONAL_int32_t, cx, OPTIONAL_int32_t, cy)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(SIZE));
    context->SetObjectVariable("CSELF", obj);

    SIZE *s = (SIZE *)context->BufferData(obj);

    s->cx = argumentExists(1) ? cx : 0;
    s->cy = argumentExists(2) ? cy : s->cx;

    return NULLOBJECT;
}

RexxMethod1(int32_t, size_cx, CSELF, s) { return ((SIZE *)s)->cx; }
RexxMethod1(int32_t, size_cy, CSELF, s) { return ((SIZE *)s)->cy; }
RexxMethod2(RexxObjectPtr, size_setCX, CSELF, s, int32_t, cx) { ((SIZE *)s)->cx = cx; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, size_setCY, CSELF, s, int32_t, cy) { ((SIZE *)s)->cy = cy; return NULLOBJECT; }

/**
 * Methods for the ooDialog .Rect class.
 */
#define RECT_CLASS  "Rect"

RexxMethod4(RexxObjectPtr, rect_init, OPTIONAL_int32_t, left, OPTIONAL_int32_t, top,
            OPTIONAL_int32_t, right, OPTIONAL_int32_t, bottom)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(RECT));
    context->SetObjectVariable("CSELF", obj);

    RECT *r = (RECT *)context->BufferData(obj);

    r->left = argumentExists(1) ? left : 0;
    r->top = argumentExists(2) ? top : r->left;
    r->right = argumentExists(3) ? right : r->left;
    r->bottom = argumentExists(4) ? bottom : r->left;

    return NULLOBJECT;
}

RexxMethod1(int32_t, rect_left, CSELF, pRect) { return ((RECT *)pRect)->left; }
RexxMethod1(int32_t, rect_top, CSELF, pRect) { return ((RECT *)pRect)->top; }
RexxMethod1(int32_t, rect_right, CSELF, pRect) { return ((RECT *)pRect)->right; }
RexxMethod1(int32_t, rect_bottom, CSELF, pRect) { return ((RECT *)pRect)->bottom; }
RexxMethod2(RexxObjectPtr, rect_setLeft, CSELF, pRect, int32_t, left) { ((RECT *)pRect)->left = left; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, rect_setTop, CSELF, pRect, int32_t, top) { ((RECT *)pRect)->top = top; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, rect_setRight, CSELF, pRect, int32_t, right) { ((RECT *)pRect)->right = right; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, rect_setBottom, CSELF, pRect, int32_t, bottom) { ((RECT *)pRect)->bottom = bottom; return NULLOBJECT; }

/**
 * Methods for the .DlgUtil class.
 */
#define DLG_UTIL_CLASS  "DlgUtil"

/**
 * Convenience function to put up an error message box.
 *
 * @param pszMsg    The message.
 * @param pszTitle  The title of for the message box.
 */
static void internalErrorMsg(CSTRING pszMsg, CSTRING pszTitle)
{
    MessageBox(0, pszMsg, pszTitle, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
}

/**
 * Determines the version of comctl32.dll and compares it against a minimum
 * required version.
 *
 * @param  context      The ooRexx method context.
 * @param  pDllVersion  The loaded version of comctl32.dll is returned here as a
 *                      packed unsigned long. This number is created using
 *                      Microsoft's suggested process and can be used for
 *                      numeric comparisons.
 * @param  minVersion   The minimum acceptable version.
 * @param  packageName  The name of the package initiating this check.
 * @param  errTitle     The title for the error dialog if it is displayed.
 *
 * @note  If this function fails, an exception is raised.
 */
bool getComCtl32Version(RexxMethodContext *context, DWORD *pDllVersion, DWORD minVersion,
                         CSTRING packageName, CSTRING errTitle)
{
    bool success = false;
    *pDllVersion = 0;

    HINSTANCE hinst = LoadLibrary(TEXT(COMMON_CONTROL_DLL));
    if ( hinst )
    {
        DLLGETVERSIONPROC pDllGetVersion;

        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinst, DLLGETVERSION_FUNCTION);
        if ( pDllGetVersion )
        {
            HRESULT hr;
            DLLVERSIONINFO info;

            ZeroMemory(&info, sizeof(info));
            info.cbSize = sizeof(info);

            hr = (*pDllGetVersion)(&info);
            if ( SUCCEEDED(hr) )
            {
                *pDllVersion = MAKEVERSION(info.dwMajorVersion, info.dwMinorVersion);
                success = true;
            }
            else
            {
                systemServiceExceptionComCode(context, COM_API_FAILED_MSG, DLLGETVERSION_FUNCTION, hr);
            }
        }
        else
        {
            systemServiceExceptionCode(context, NO_PROC_MSG, DLLGETVERSION_FUNCTION);
        }
        FreeLibrary(hinst);
    }
    else
    {
        systemServiceExceptionCode(context, NO_HMODULE_MSG, COMMON_CONTROL_DLL);
    }

    if ( *pDllVersion == 0 )
    {
        CHAR msg[256];
        _snprintf(msg, sizeof(msg),
                  "The version of the Windows Common Controls library (%s)\n"
                  "could not be determined.  %s can not continue",
                  COMMON_CONTROL_DLL, packageName);

        internalErrorMsg(msg, errTitle);
        success = false;
    }
    else if ( *pDllVersion < minVersion )
    {
        CHAR msg[256];
        _snprintf(msg, sizeof(msg),
                  "%s can not continue with this version of the Windows\n"
                  "Common Controls library(%s.)  The minimum\n"
                  "version required is: %s.\n\n"
                  "This system has: %s\n",
                  packageName, COMMON_CONTROL_DLL, comctl32VersionName(minVersion),
                  comctl32VersionName(*pDllVersion));

        internalErrorMsg(msg, errTitle);
        *pDllVersion = 0;
        success = false;
    }
    return success;
}

/**
 * Initializes the common control library for the specified classes.
 *
 * @param classes       Flag specifing the classes to be initialized.
 * @param  packageName  The name of the package initializing the classes.
 * @param  errTitle     The title for the error dialog if it is displayed.
 *
 * @return True on success, otherwise false.
 *
 * @note   An exception has been raised when false is returned.
 */
bool initCommonControls(RexxMethodContext *context, DWORD classes, CSTRING packageName, CSTRING errTitle)
{
    INITCOMMONCONTROLSEX ctrlex;

    ctrlex.dwSize = sizeof(ctrlex);
    ctrlex.dwICC = classes;

    if ( ! InitCommonControlsEx(&ctrlex) )
    {
        systemServiceExceptionCode(context, NO_COMMCTRL_MSG, "Common Control Library");

        CHAR msg[128];
        _snprintf(msg, sizeof(msg),
                  "Initializing the Windows Common Controls\n"
                  "library failed.  %s can not continue.\n\n"
                  "Windows System Error Code: %d\n", packageName, GetLastError());

        internalErrorMsg(msg, errTitle);
        return false;
    }
    return true;
}

/** DlgUtil::init() [class method]
 *
 * The .DlgUtil class init() method.  It executes when the .DlgUtil class is
 * constructed, which is done during the processing of the ::requires directive
 * for oodPlain.cls.  This makes it the ideal place for any initialization that
 * must be done prior to ooDialog starting.
 *
 * Note that an exception raised here effectively terminates ooDialog before any
 * user code is executed.
 *
 * The method:
 *
 * 1.) Determines the version of comctl32.dll and initializes the common
 * controls.  The minimum acceptable version of 4.71 is supported on Windows 95
 * with Internet Explorer 4.0, Windows NT 4.0 with Internet Explorer 4.0,
 * Windows 98, and Windows 2000.
 *
 * 2.) Initializes a null pointer Pointer object and places it in the .local
 * directory. (.NullHandle)  This allows ooRexx code to use a null handle for an
 * argument where appropriate.
 *
 * 3.) Places the SystemErrorCode (.SystemErrorCode) variable in the .local
 * directory.
 *
 * @return .true if comctl32.dll is at least version 4.71, otherwise .false.
 */
RexxMethod0(logical_t, dlgutil_init_cls)
{
    if ( ! getComCtl32Version(context, &ComCtl32Version, COMCTL32_4_71, "ooDialog", COMCTL_ERR_TITLE) )
    {
        return false;
    }

    if ( ! initCommonControls(context, ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES, "ooDialog", COMCTL_ERR_TITLE) )
    {
        ComCtl32Version = 0;
        return false;
    }

    RexxDirectoryObject local = context->GetLocalEnvironment();
    if ( local != NULLOBJECT )
    {
        context->DirectoryPut(local, context->NewPointer(NULL), "NULLHANDLE");
        context->DirectoryPut(local, context->WholeNumberToObject(0), "SYSTEMERRORCODE");
    }

    return true;
}

/** DlgUtil::comCtl32Version()  [class method]
 *
 * Returns the comctl32.dll version that ooDialog is currently using in one of
 * the 4 formats listed below.
 *
 * @param format  [optional] Keyword indicating the format, only the first
 *                letter is needed, case not significant.  The default is short.
 *                Incorrect strings are ignored and the default is used.
 *
 * The formats are:
 *   Full:  A complete string including the DLL name, version number and the
 *          minimum Windows OS name that the DLL is found on.
 *
 *   Number:  The version number part of the full string.
 *   Short:   Same as number.
 *   OS:      The OS name part of the full string.
 */
RexxMethod1(RexxStringObject, dlgutil_comctl32Version_cls, OPTIONAL_CSTRING, format)
{
    const char *ver;
    char f = argumentOmitted(1) ? 'S' : *format;

    switch ( f )
    {
        case 'f' :
        case 'F' :
            ver = comctl32VersionName(ComCtl32Version);
            break;

        case 'o' :
        case 'O' :
            ver = comctl32VersionPart(ComCtl32Version, COMCTL32_OS_PART);
            break;

        case 's' :
        case 'S' :
        case 'n' :
        case 'N' :
        default :
            ver = comctl32VersionPart(ComCtl32Version, COMCTL32_NUMBER_PART);
            break;
    }
    return context->String(ver);
}

/** DlgUtil::version()  [class method]
 *
 *  Returns the ooDialog version string, either the full string, or just the
 *  number part of the string.
 *
 * @param  format  [optional]  Keyword indicating which format the returned
 *                 string should be in.  Currently, if the arg is not omitted
 *                 and the first letter of the keyword is either S or s the
 *                 short form (number part of the string) is returned.  In all
 *                 other cases the full string is returned.
 */
RexxMethod1(RexxStringObject, dlgutil_version_cls, OPTIONAL_CSTRING, format)
{
    char buf[64];

    if ( argumentExists(1) && (*format == 'S' || *format == 's') )
    {
        _snprintf(buf, sizeof(buf), "%u.%u.%u.%u", ORX_VER, ORX_REL, ORX_MOD, OOREXX_BLD);
    }
    else
    {
        _snprintf(buf, sizeof(buf), "ooDialog Version %u.%u.%u.%u (an ooRexx Windows Extension)", ORX_VER, ORX_REL, ORX_MOD, OOREXX_BLD);
    }
    return context->String(buf);
}

RexxMethod1(logical_t, dlgutil_test_cls, RexxObjectPtr, obj)
{
    return 0;
}

RexxMethod1(uint16_t, dlgutil_hiWord_cls, uint32_t, dw) { return HIWORD(dw); }
RexxMethod1(uint16_t, dlgutil_loWord_cls, uint32_t, dw) { return LOWORD(dw); }

RexxMethod2(uint64_t, dlgutil_and_cls, CSTRING, s1, CSTRING, s2)
{
    uint64_t n1, n2;
    if ( ! rxStr2Number(context, s1, &n1, 1) || ! rxStr2Number(context, s2, &n2, 2) )
    {
        return 0;
    }
    return (n1 & n2);
}

RexxMethod1(uint64_t, dlgutil_or_cls, ARGLIST, args)
{
    RexxMethodContext *c = context;
    uint64_t result, n1;

    size_t count = c->ArraySize(args);
    if ( count == 0 )
    {
        return 0;
    }

    CSTRING s1 = c->ObjectToStringValue(c->ArrayAt(args, 1));
    if ( ! rxStr2Number(c, s1, &result, 1) )
    {
        return 0;
    }

    for ( size_t i = 2; i <= count; i++)
    {
        s1 = c->ObjectToStringValue(c->ArrayAt(args, i));
        if ( ! rxStr2Number(c, s1, &n1, (int)i) )
        {
            return 0;
        }
        result |= n1;
    }

    return result;
}

/** DlgUtil::getSystemMetrics()  [class method]
 *
 *  Returns the system metric for the give index.
 *
 *  @param index  The index of the system metric to look up.
 *
 *  @note There was a classic Rexx external function documented prior to 4.0.0
 *        with the function name of GetSysMetrics.  That function is now marked
 *        deprecated in the docs and the places where it was used are mapped to
 *        this .DlgUtil method.
 *
 *        The intent was to extend this function in the future to get multiple,
 *        and perhaps all, values at once.  This method could be enhanced to do
 *        that.
 *
 *        MSDN documents that GetLastError does not provide extended error
 *        information.
 */
RexxMethod1(uint32_t, dlgutil_getSystemMetrics_cls, int32_t, index)
{
    return GetSystemMetrics(index);
}

/**
 * A temporary utility to convert from a handle that is still being stored in
 * ooDialog in string form ("0xFFFFAAAA") to its actual pointer value.  The
 * interface is needed to facilitate testing Windows extensions that have been
 * converted to only use pointer valules.
 */
RexxMethod1(POINTER, dlgutil_handleToPointer_cls, POINTERSTRING, handle)
{
    return handle;
}
