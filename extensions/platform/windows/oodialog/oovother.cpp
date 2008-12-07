/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2008 Rexx Language Association. All rights reserved.    */
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

#include <windows.h>
#include <mmsystem.h>
#include "oorexxapi.h"
#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include <shlwapi.h>
#include <commctrl.h>
#include "oovutil.h"

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


size_t RexxEntry SleepMS(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   CHECKARG(1);

   Sleep(atoi(argv[0].strptr));
   RETC(0)
}


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
           WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(ew, GWLP_WNDPROC, (LONG_PTR)CatchReturnSubProc);
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
           SetWindowLongPtr((HWND)ew, GWLP_WNDPROC, (LONG_PTR)wpOldEditProc);
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
    if ( argv[2].strptr[0] == 'E' )      /* Edit control function (or also static) */
    {
        if ( strcmp(argv[3].strptr, "TXT") == 0 )       /* Set or get the control's text. */
        {
            /* The same function is used to set / get the text for an edit
             * control or for a static control.
             */
            if ( ! (checkWindowClass(hCtrl, WC_EDIT) || checkWindowClass(hCtrl, WC_STATIC)) )
                RETVAL(-1)

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

        /* The remaining functions are for an edit control only */
        if ( ! checkWindowClass(hCtrl, WC_EDIT) )
            RETVAL(-1)

        if ( !strcmp(argv[3].strptr, "MSG") ) /* Send an edit message (EM_*) */
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

        if ( !strcmp(argv[3].strptr, "GET") )         /* Get the window style */
        {
            /* Return the window style as an unsigned long for any dialog control. */
            ultoa((ULONG)lStyle, retstr->strptr, 10);
            retstr->strlength = strlen(retstr->strptr);
            return 0;
        }
         else if ( !strcmp(argv[3].strptr, "TAB") )   /* Set or remove tab stop  style */
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
            RETPTR(GetWindowLongPtr(hCtrl, GWLP_USERDATA));
        }
        else if ( argc == 4 )
        {
            RETPTR(SetWindowLongPtr(hCtrl, GWLP_USERDATA, atol(argv[3].strptr)));
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
    else if ( argv[2].strptr[0] == 'F' )    /* Font */
    {
        if ( strcmp(argv[3].strptr, "GET" ) == 0)
        {
            RETHANDLE(SendMessage(hCtrl, WM_GETFONT, 0, 0));
        }
        else RETERR
    }
    RETERR
}


static inline int getLVColumnCount(HWND hList)
{
    return Header_GetItemCount(ListView_GetHeader(hList));
}

/**
 * What is a reasonable number of columns in a list-view?  Seems silly to
 * restrict the user, someone will always want 2 more.  On the other hand, is
 * someone going to have 1,000 columns?  10,000 columns?
 *
 * @param  count  The number of colums
 *
 * @return The size of a space separated ascii string of numbers big enough to
 *         hold count numbers.
 */
static inline size_t getColumnOrderStrlen(int count)
{
    if ( count < 100 )
    {
        return 3 * count;
    }
    else if ( count < 1000 )
    {
        return (3 * 99) + (4 * (count - 99));
    }
    else if ( count < 10000 )
    {
        return (3 * 99) + (4 * 900) + (5 * (count - 999));
    }
    return 0;
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
        else if ( strcmp(argv[2].strptr, "ORDER") == 0 )  /* Set, get column Order */
        {
            int count;
            int *order;
            int i = 0;
            int retVal = 1;

            count = getLVColumnCount(hList);
            if ( count < 2 )
            {
                /* The return is 0 or 1 columns, or -1 for an error. */
                RETVAL(count == -1 ? -2 : count)
            }

            order = (int *)malloc(count * sizeof(int));
            if ( order == NULL )
            {
                RETVAL(-(LONG)GetLastError())
            }

            if ( argc == 3 )
            {
                char buf[4];

                size_t l = getColumnOrderStrlen(count);
                if ( l == 0 )
                {
                    retVal = -2;
                }
                else
                {
                    if ( l > RXAUTOBUFLEN )
                    {
                        PVOID p = GlobalAlloc(GMEM_FIXED, count);
                        if ( ! p )
                        {
                            free(order);
                            RETVAL(-(LONG)GetLastError())
                        }

                        retstr->strptr = (PCHAR)p;
                    }

                    if ( ListView_GetColumnOrderArray(hList, count, order) == 0 )
                    {
                        retVal = -2;
                    }
                    else
                    {
                        retstr->strptr[0] = '\0';
                        for ( i = 0; i < count; i++, order++ )
                        {
                            strcat(retstr->strptr, ltoa(*order, buf, 10));
                            strcat(retstr->strptr, " ");
                        }
                        retstr->strlength = strlen(retstr->strptr);
                    }
                }
            }
            else if ( argc == 4 )
            {
                char *token;
                char *str = _strdup(argv[3].strptr);

                token = strtok(str, " ");
                while( token != NULL && i++ < count )
                {
                    *order++ = atoi(token);
                    token = strtok(NULL, " ");
                }
                free(str);

                retVal = ListView_SetColumnOrderArray(hList, count, order) == 0 ? -2 : 0;
            }
            else
            {
                retVal = -3;     /* Error with argument. */
            }

            free(order);
            if ( retVal != 1 )
            {
                RETVAL(retVal)
            }
            else
            {
                /* The return string is already set, just return 0. */
                return 0;
            }
        }
        else RETERR;
    }
    /* G - Get something function */
    else if ( argv[1].strptr[0] == 'G' )
    {
        if ( !strcmp(argv[2].strptr, "COLCOUNT") )  /* Get List-view column count. */
        {
            RETVAL(getLVColumnCount(hList));
        }
        else RETERR;
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
               WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(ew, GWLP_WNDPROC, (LONG_PTR)CatchReturnSubProc);
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
               SetWindowLongPtr(ew, GWLP_WNDPROC, (LONG_PTR)wpOldEditProc);
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

#define OOD_ID_EXCEPTION -9

#define NO_HMODULE_MSG            "failed to obtain %s module handle; OS error code %d"
#define NO_PROC_MSG               "failed to get procedeure adddress for %s(); OS error code %d"
#define API_FAILED_MSG            "system API %s() failed; COM code 0x%08x"
#define NO_COMMCTRL_MSG           "failed to initialize %s; OS error code %d"

const char *comctl32VersionName(DWORD id)
{
    const char *name;
    switch ( id )
    {
        case COMCTL32_4_0 :
            name = "comctl32.dll version 4.0 (W95 / NT4)";
            break;

        case COMCTL32_4_7 :
            name = "comctl32.dll version 4.7 (IE 3.x)";
            break;

        case COMCTL32_4_71 :
            name = "comctl32.dll version 4.71 (IE 4.0)";
            break;
        case COMCTL32_4_72 :
            name = "comctl32.dll version 4.72 (W98 / IE 4.01)";
            break;
        case COMCTL32_5_8 :
            name = "comctl32.dll version 5.8 (IE 5)";
            break;
        case COMCTL32_5_81 :
            name = "comctl32.dll version 5.81 (W2K / ME)";
            break;
        case COMCTL32_6_0 :
            name = "comctl32.dll version 6.0 (XP)";
            break;
        default :
            name = "Unknown";
            break;
    }
    return name;
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

        context->RaiseException1(Rexx_Error_Execution_user_defined, context->NewStringFromAsciiz(buf));
    }
    return adm;
}


inline HWND rxGetWindowHandle(RexxMethodContext * context, RexxObjectPtr self)
{
    return (HWND)rxGetPointerAttribute(context, self, "HWND");
}


void systemServiceException(RexxMethodContext *context, char *msg)
{
    context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz(msg));
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

inline void outOfMemoryException(RexxMethodContext *c)
{
    systemServiceException(c, "Failed to allocate memory");
}

inline void *wrongClassException(RexxMethodContext *c, int pos, const char *n)
{
    c->RaiseException2(Rexx_Error_Incorrect_method_noclass, c->WholeNumberToObject(pos), c->NewStringFromAsciiz(n));
    return NULL;
}

void wrongArgValueException(RexxMethodContext *c, int pos, const char *list, RexxObjectPtr actual)
{
    RexxArrayObject a = c->NewArray(3);
    c->ArrayAppend(a, c->WholeNumberToObject(pos));
    c->ArrayAppend(a, c->NewStringFromAsciiz(list));
    c->ArrayAppend(a, actual);

    c->RaiseException(Rexx_Error_Incorrect_method_list, a);
}

void wrongArgValueException(RexxMethodContext *c, int pos, const char *list, const char *actual)
{
    wrongArgValueException(c, pos, list, c->NewStringFromAsciiz(actual));
}

bool requiredComCtl32Version(RexxMethodContext *context, const char *methodName, DWORD minimum)
{
    if ( ComCtl32Version < minimum )
    {
        char msg[256];
        _snprintf(msg, sizeof(msg), "The %s() method requires %s or later", methodName, comctl32VersionName(minimum));
        context->RaiseException1(Rexx_Error_System_service_user_defined, context->NewStringFromAsciiz(msg));
        return false;
    }
    return true;
}

bool requiredClass(RexxMethodContext *context, RexxObjectPtr obj, const char *name, int argPos)
{
    RexxClassObject rxClass = context->FindContextClass(name);
    if ( ! context->IsInstanceOf(obj, rxClass) )
    {
        wrongClassException(context, argPos, name);
        return false;
    }
    return true;
}

void wrongWindowStyleException(RexxMethodContext *context, const char *obj, const char *style)
{
    char msg[128];
    _snprintf(msg, sizeof(msg), "This %s does not have the %s style", obj, style);
    context->RaiseException1(Rexx_Error_Incorrect_method_user_defined, context->NewStringFromAsciiz(msg));
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
    RexxClassObject RectClass = context->FindContextClass("RECT");
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

RexxObjectPtr rxNewPoint(RexxMethodContext *context, long x, long y)
{
    RexxObjectPtr point = NULL;
    RexxClassObject PointClass = context->FindContextClass("POINT");
    if ( PointClass != NULL )
    {
        point = context->SendMessage2(PointClass, "NEW", context->WholeNumber(x), context->WholeNumber(y));
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

RexxObjectPtr rxNewSize(RexxMethodContext *context, long cx, long cy)
{
    RexxObjectPtr size = NULL;
    RexxClassObject SizeClass = context->FindContextClass("SIZE");
    if ( SizeClass != NULL )
    {
        size = context->SendMessage2(SizeClass, "NEW", context->WholeNumber(cx), context->WholeNumber(cy));
    }
    return size;
}

inline bool hasStyle(HWND hwnd, DWORD_PTR style)
{
    if ( (GetWindowLongPtr(hwnd, GWL_STYLE) & style) || (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & style) )
    {
        return true;
    }
    return false;
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

void oodSetSysErrCode(RexxMethodContext *context, DWORD code)
{
    RexxDirectoryObject local = context->GetLocalEnvironment();
    if ( local != NULLOBJECT )
    {
        context->DirectoryPut(local, context->WholeNumberToObject(code), "SYSTEMERRORCODE");
    }
}
void oodSetSysErrCode(RexxMethodContext *context)
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
 * @param argPosID   Arg positionof the ID, used for raised exceptions.
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
RexxMethod2(int, pbc_stepIt, OSELF, self, OPTIONAL_uint32_t, delta)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    if ( argumentOmitted(2) )
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
RexxMethod2(int, pbc_setPos, OSELF, self, int32_t, newPos)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    return (int)SendMessage(hwnd, PBM_SETPOS, newPos, 0);
}

RexxMethod1(int, pbc_getPos, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    return (int)SendMessage(hwnd, PBM_GETPOS, 0, 0);
}

RexxMethod3(RexxStringObject, pbc_setRange, OSELF, self, OPTIONAL_int32_t, min, OPTIONAL_int32_t, max)
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

    return context->NewStringFromAsciiz(buf);
}

RexxMethod1(RexxStringObject, pbc_getRange, OSELF, self)
{
    TCHAR buf[64];
    HWND hwnd = rxGetWindowHandle(context, self);
    PBRANGE pbr;

    SendMessage(hwnd, PBM_GETRANGE, TRUE, (LPARAM)&pbr);
    _snprintf(buf, sizeof(buf), "%d %d", pbr.iLow, pbr.iHigh);

    return context->NewStringFromAsciiz(buf);
}

RexxMethod2(int, pbc_setStep, OSELF, self, OPTIONAL_int32_t, newStep)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    if ( argumentOmitted(1) )
    {
        newStep = 10;
    }
    return (int)SendMessage(hwnd, PBM_SETSTEP, newStep, 0);
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
RexxMethod3(logical_t, pbc_setMarquee, OSELF, self, OPTIONAL_logical_t, on, OPTIONAL_uint32_t, pause)
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
 *  @param   r      [Required]  This can be either the COLORREF value, if the
 *                  number of args is exactly one, or the red value of a
 *                  COLORREF if the args are exactly 3.
 *
 *  @param   g      [Optional]  The green value of a COLORREF.  This arg is only
 *                  optional if arg 1 is a COLORREF.
 *
 *  @param   b      [Optional]  The blue value of a COLORREF.  This arg is only
 *                  optional if arg 1 is a COLORREF.
 *
 *  @return  The previous background color, or CLR_DEFAULT if the previous color
 *           was the defualt.  This is returned as a COLORREF number.
 *
 *  The progress bar control only supports this function under Windows Classic
 *  Theme.
 */
RexxMethod4(uint32_t, pbc_setBkColor, OSELF, self, uint32_t, r, OPTIONAL_uint8_t, g, OPTIONAL_uint8_t, b)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    size_t count = rxArgCount(context);
    COLORREF rgb;

    if ( count == 1 )
    {
        rgb = r;
    }
    else if ( count == 3 )
    {
        rgb = RGB((uint8_t)r, g, b);
    }
    else
    {
        context->RaiseException1(Rexx_Error_Incorrect_method_minarg, context->WholeNumberToObject(3));
        return 0;
    }

    return (uint32_t)SendMessage(hwnd, PBM_SETBKCOLOR, 0, rgb);
}

/**
 *  ProgressBar::barColor()
 *
 *  Sets the bar color of the progress bar.
 *
 *  @param   r      [Required]  This can be either the COLORREF value, if the
 *                  number of args is exactly one, or the red value of a
 *                  COLORREF if the args are exactly 3.
 *
 *  @param   g      [Optional]  The green value of a COLORREF.  This arg is only
 *                  optional if arg 1 is a COLORREF.
 *
 *  @param   b      [Optional]  The blue value of a COLORREF.  This arg is only
 *                  optional if arg 1 is a COLORREF.
 *
 *  @return  The previous bar color, or CLR_DEFAULT if the previous color
 *           was the defualt.  This is returned as a COLORREF number.
 *
 *  The progress bar control only supports this function under Windows Classic
 *  Theme.
 */
RexxMethod4(uint32_t, pbc_setBarColor, OSELF, self, uint32_t, r, OPTIONAL_uint8_t, g, OPTIONAL_uint8_t, b)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    size_t count = rxArgCount(context);
    COLORREF rgb;

    if ( count == 1 )
    {
        rgb = r;
    }
    else if ( count == 3 )
    {
        rgb = RGB((uint8_t)r, g, b);
    }
    else
    {
        context->RaiseException1(Rexx_Error_Incorrect_method_minarg, context->WholeNumberToObject(3));
        return 0;
    }
    return (uint32_t)SendMessage(hwnd, PBM_SETBARCOLOR, 0, rgb);
}

/**
 * This function stub is used for testing.
 */
RexxMethod5(size_t, pbc_test, OPTIONAL_int32_t, n1,
            OPTIONAL_int32_t, n2, OSELF, self, OPTIONAL_int32_t, n3, OPTIONAL_int32_t, n4)
{
    return rxArgCount(context);
}

/**
 *  Methods for the ooDialog class: .ButtonControl and its subclasses
 *  .RadioButton and .CheckBox.
 */

#define BC_SETSTYLE_OPTS "PUSHBOX, DEFPUSHBUTTON, CHECKBOX, AUTOCHECKBOX, 3STATE, AUTO3STATE, "        \
                         "RADIO, AUTORADIO, GROUPBOX, OWNERDRAW, LEFTTEXT, RIGHTBUTTON, NOTLEFTTEXT, " \
                         "TEXT, ICON, BITMAP, LEFT, RIGHT, HCENTER, TOP, BOTTOM, VCENTER, PUSHLIKE, "  \
                         "NOTPUSHLIKE, MULTILINE, NOTMULTILINE, NOTIFY, NOTNOTIFY, FLAT, NOTFLAT"

#define BC_SETSTATE_OPTS "CHECKED, UNCHECKED, INDETERMINATE, FOCUS, PUSH, NOTPUSHED"

typedef enum {push, check, radio, group, owner, notButton} BUTTONTYPE, *PBUTTONTYPE;
typedef enum {def, autoCheck, threeState, autoThreeState, noSubtype } BUTTONSUBTYPE, *PBUTTONSUBTYPE;

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

    DWORD _style = (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE);
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

/** ButtonControl::releaseImageList()  [class method]
 *
 *  Used to destroy an image list.
 *
 *  @param  himl  [required]  The image list handle.
 *
 */
RexxMethod1(RexxObjectPtr, bc_cls_releaseImageList, POINTER, himl)
{
    if ( himl != NULL )
    {
        ImageList_Destroy((HIMAGELIST)himl);
    }
    return NULLOBJECT;
}

RexxMethod4(int, bc_cls_checkInGroup, RexxObjectPtr, dlg, RexxObjectPtr, idFirst,
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

RexxMethod2(RexxObjectPtr, bc_setState, OSELF, self, CSTRING, opts)
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

    return context->NewStringFromAsciiz(buf);
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


#define MIN_HALFHEIGHT_GB 12

/** .GroupBox~style =
 *
 * A group box is a button, but the only style change that makes much sense is
 * the right or left alignment of the text.  Other changes either have no
 * effect, or cause the group box / dialog to paint in a weird way.
 */
RexxMethod2(int, gb_setStyle, OSELF, self, CSTRING, opts)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    DWORD style = (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE);

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
    SetWindowLongPtr(hwnd, GWL_STYLE, style);
    SendMessage(hwnd, BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);

    InvalidateRect(hDlg, &r, TRUE);
    UpdateWindow(hDlg);

    return 0;
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
    oldTypeStyle = ((DWORD)GetWindowLongPtr(hwnd, GWL_STYLE) & BS_TYPEMASK);
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
        SetWindowLongPtr(hwnd, GWL_STYLE, style);
        SendMessage(hwnd, BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);

        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);

        if ( oldDefButton )
        {
            InvalidateRect(oldDefButton, NULL, TRUE);
            UpdateWindow(oldDefButton);
        }
    }

    safeFree(str);
    return NULLOBJECT;
}

static int getImageType(RexxMethodContext *context, int argPos, const char *opt)
{
    int type = IMAGE_BITMAP;
    if ( argumentExists(1) )
    {
        switch ( *opt )
        {
            case 'b' :
            case 'B' :
                // Do nothing type is already IMAGE_BITMAP.
                break;

            case 'i' :
            case 'I' :
                type = IMAGE_ICON;
                break;

            default :
                wrongArgValueException(context, 1, "Bitmap, Icon", opt);
                return 0;
        }
    }
    return type;
}

RexxMethod2(POINTER, bc_getImage, OSELF, self, OPTIONAL_CSTRING, opt)
{
    int type = getImageType(context, 1, opt);
    HWND hwnd = rxGetWindowHandle(context, self);
    return (void *)SendMessage(hwnd, BM_GETIMAGE, type, 0);
}

RexxMethod3(POINTER, bc_setImage, OSELF, self, POINTER, hImage, OPTIONAL_CSTRING, opt)
{
    int type = getImageType(context, 1, opt);
    HWND hwnd = rxGetWindowHandle(context, self);
    return (void *)SendMessage(hwnd, BM_SETIMAGE, type, (LPARAM)hImage);
}

RexxMethod1(RexxObjectPtr, bc_click, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    SendMessage(hwnd, BM_CLICK, 0, 0);
    return NULLOBJECT;
}

RexxMethod1(logical_t, bc_checked, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    return (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
}

RexxMethod1(CSTRING, bc_isChecked, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    char * state = "UNCHECKED";

    switch ( SendMessage(hwnd, BM_GETCHECK, 0, 0) )
    {
        case BST_CHECKED :
            state = "CHECKED";
            break;
        case BST_INDETERMINATE :
            state = "INDETERMINATE";
    }
    return state;
}

RexxMethod1(logical_t, bc_isIndeterminate, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    if ( getButtonInfo(hwnd, NULL, NULL) == check  )
    {
        return (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_INDETERMINATE ? 1 : 0);
    }
    return 0;
}

RexxMethod1(int, bc_indeterminate, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    if ( getButtonInfo(hwnd, NULL, NULL) == check  )
    {
        SendMessage(hwnd, BM_SETCHECK, BST_INDETERMINATE, 0);
    }
    return 0;
}

RexxMethod1(int, bc_check, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
    return 0;
}

RexxMethod1(int, bc_uncheck, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    SendMessage(hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
    return 0;
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

RexxMethod1(RexxObjectPtr, bc_getImageList, OSELF, self)
{
    if ( ! requiredComCtl32Version(context, "getImageList", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }

    HWND hwnd = rxGetWindowHandle(context, self);
    BUTTON_IMAGELIST biml;
    RexxObjectPtr result = context->Nil();

    if ( Button_GetImageList(hwnd, &biml) )
    {
        RexxDirectoryObject table = context->NewDirectory();
        if ( table != NULLOBJECT )
        {
            RexxObjectPtr ptr = (RexxObjectPtr)context->NewPointer(biml.himl);
            if ( ptr != NULLOBJECT )
            {
                context->DirectoryPut(table, ptr, "HANDLE");
            }

            RexxObjectPtr rect = rxNewRect(context, biml.margin.left, biml.margin.top,
                                           biml.margin.right, biml.margin.bottom);
            if ( rect != NULL )
            {
                context->DirectoryPut(table, rect, "RECT");
            }

            char *align;
            switch ( biml.uAlign )
            {
                case BUTTON_IMAGELIST_ALIGN_LEFT :
                    align = "LEFT";
                    break;
                case BUTTON_IMAGELIST_ALIGN_RIGHT :
                    align = "RIGHT";
                    break;
                case BUTTON_IMAGELIST_ALIGN_TOP :
                    align = "TOP";
                    break;
                case BUTTON_IMAGELIST_ALIGN_BOTTOM :
                    align = "BOTTOM";
                    break;
                default :
                    align = "CENTER";
                    break;
            }
            RexxStringObject alignment = context->String(align);
            if ( alignment != NULLOBJECT )
            {
                context->DirectoryPut(table, alignment, "ALIGNMENT");
            }

            result = table;
        }
    }
    return result;
}

static HIMAGELIST getImageListFromObject(RexxMethodContext *context, RexxObjectPtr imageList, int argPos)
{
    /* At this time, it must be a directory object. */
    if ( ! context->IsDirectory(imageList) )
    {
        wrongClassException(context, argPos, "Directory");
        return NULL;
    }

    HIMAGELIST himl = NULL;

    // Note that size is both the directory entry name and the class name.
    RexxObjectPtr size = context->DirectoryAt((RexxDirectoryObject)imageList, "SIZE");
    RexxClassObject rxClass = context->FindContextClass("SIZE");
    if ( size == NULLOBJECT || ! context->IsInstanceOf(size, rxClass) )
    {
        goto bad_directory;
    }
    PSIZE pSize = (PSIZE)context->ObjectToCSelf(size);

    uint32_t flag;
    RexxObjectPtr f = context->DirectoryAt((RexxDirectoryObject)imageList, "FLAGS");
    if ( f == NULLOBJECT || ! context->ObjectToUnsignedInt32(f, &flag) )
    {
        goto bad_directory;
    }

    f = context->DirectoryAt((RexxDirectoryObject)imageList, "FILES");
    if ( f == NULLOBJECT || ! context->IsArray(f) )
    {
        goto bad_directory;
    }

    RexxArrayObject files = (RexxArrayObject)f;
    int count = (int)context->ArraySize(files);
    if ( count < 1 )
    {
        goto bad_directory;
    }

    himl = ImageList_Create(pSize->cx, pSize->cy, flag, count, count);
    if ( himl == NULL )
    {
        oodSetSysErrCode(context);
        goto winapi_failed;
    }

    HANDLE hBitmap;
    for ( int i = 1; i <= count; i++ )
    {
        f = context->ArrayAt(files, i);
        if ( f == NULLOBJECT || ! context->IsString(f) )
        {
            goto bad_directory;
        }

        const char *file = context->ObjectToStringValue(f);

        hBitmap = LoadImage(NULL, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

        if ( hBitmap == NULL )
        {
            oodSetSysErrCode(context);
            goto winapi_failed;
        }

        // REVISIT ImageList_ADD() returns -1 if the add fails, should we quit,
        // throw an exception, or just continue?
        //
        // One problem is that last error is not set, so oodSetSysErrCode() does
        // nothing to give the user a clue as to what the problem is. If we just
        // continue, then the user will not get bitmaps at the right indexes, if
        // only one or two add fails.  Or get no bitmaps if they all fail. But,
        // will not have a clue that there was problem.  An execption would at
        // least let him know where the problem is.
        ImageList_Add(himl, (HBITMAP)hBitmap, NULL);
        DeleteObject(hBitmap);
    }
    return himl;

bad_directory:
    char msg[256];
    _snprintf(msg, sizeof(msg),
              "The Directory object, method argument %d, does not have the proper entries", argPos);
    context->RaiseException1(Rexx_Error_Incorrect_method_user_defined, context->String(msg));

winapi_failed:
    if ( himl != NULL )
    {
        ImageList_Destroy(himl);
    }
    return NULL;
}

/** ButtonControl::setImageList()
 *
 * Sets an image list for the button.
 *
 * @param   imageList  [required]  An object describing the image list. Curently
 *                     this must be a .Directory object with the format
 *                     described below.  A future enhancement is to allow a
 *                     .ImageList, but that class is not yet implemented.
 *
 * @param   margin     [optional]  A .Rect object containing the margins around
 *                     the image.
 *
 * @param   align      [optional]  One of the BUTTON_IMAGELIST_ALIGN_xxx
 *                     constant values.
 *
 * @return  A .Pointer object containing the handle to the image list used in
 *          the BUTTON_IMAGELIST struct.  This will be a null .Pointer on a
 *          Win32 API error.
 *
 * @remarks This method sets the ooDialog System error code (.SystemErrorCode)
 *          if the args seem valid but one of the Windows APIs fails.
 *
 * @note  This method is intended to accept either a .ImageList object, or the
 *        current .Directory object. Since the .ImageList class has not been
 *        added to ooDialog, yet, it currently only accepts the .Directory
 *        object.
 *
 * .Directory object format to create the image list:
 *   d~files -> an array of file names
 *     The array must contain index 1 and any item 1 through 6 must be a string.
 *     The count of files in the array can be any number, but only the first 6
 *     are used.  For any number of items in the the array must not be sparse.
 *     Otherwise no restrictions.
 *
 *     Note that in the BCM_SETIMAGELIST MSDN doc, the enum PUSHBUTTONSTATES in
 *     the remarks section perfectly names the ooRexx indexes.
 *
 *   d~size -> a .Size object
 *     The bitmap dimensions.
 *
 *   d~flags -> a number  This is the ILC_ flags for ImageList_Create(), the
 *     user will need to figure out the correct number.  ILC_COLOR24 is 0x18 (24
 *     decimal for instance.)
 */
RexxMethod4(POINTER, bc_setImageList, RexxObjectPtr, imageList, OPTIONAL_RexxObjectPtr, margin,
            OPTIONAL_uint8_t, align, OSELF, self)
{
    if ( ! requiredComCtl32Version(context, "setImageList", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }

    oodSetSysErrCode(context, 0);

    HWND hwnd = rxGetWindowHandle(context, self);

    HIMAGELIST himl = getImageListFromObject(context, imageList, 1);
    if ( himl == NULL )
    {
        return NULLOBJECT;
    }

    BUTTON_IMAGELIST biml = {0};
    biml.himl = himl;

    if ( argumentExists(2) )
    {
        PRECT pRect = rxGetRect(context, margin, 2);
        if ( pRect == NULL )
        {
            return NULLOBJECT;
        }
        biml.margin.top = pRect->top;
        biml.margin.left = pRect->left;
        biml.margin.right = pRect->right;
        biml.margin.bottom = pRect->bottom;
    }

    biml.uAlign = argumentExists(3) ? align : BUTTON_IMAGELIST_ALIGN_CENTER;

    void *result = NULL;
    if ( Button_SetImageList(hwnd, &biml) )
    {
        result = himl;
    }
    else
    {
        oodSetSysErrCode(context);
        ImageList_Destroy(himl);
    }
    return result;
}

/* This method is used as a convenient way to test code. */
RexxMethod2(int, bc_test, RexxObjectPtr, dlg, RexxObjectPtr, id)
{
    return 0;
}

/**
 * Methods for the ooDialog .Point class.
 */
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
RexxMethod2(RexxObjectPtr, size_init, OPTIONAL_int32_t,  cx, OPTIONAL_int32_t, cy)
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
RexxMethod2(RexxObjectPtr, rect_setRight, CSELF, pRect, int32_t, right) { ((RECT *)pRect)->right; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, rect_setBottom, CSELF, pRect, int32_t, bottom) { ((RECT *)pRect)->bottom = bottom; return NULLOBJECT; }

#define COMCTL_ERR_TITLE    "ooDialog - Windows Common Controls Error"
#define GENERIC_ERR_TITLE   "ooDialog - Error"

extern DWORD ComCtl32Version = 0;

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

#define DLLGETVERSION_FUNCTION       "DllGetVersion"
#define COMMON_CONTROL_DLL           "comctl32.dll"

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
                systemServiceExceptionComCode(context, API_FAILED_MSG, DLLGETVERSION_FUNCTION, hr);
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
 * directory. (.NullPointer)  This allows ooRexx code to test for a null
 * pointer.
 *
 * 3.) Places the SystemErrorCode (.SystemErrorCode) variable in the .local
 * directory.
 *
 * @return .true if comctl32.dll is at least version 4.71, otherwise .false.
 */
RexxMethod0(logical_t, dlgutil_init)
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
        context->DirectoryPut(local, context->NewPointer(NULL), "NULLPOINTER");
        context->DirectoryPut(local, context->WholeNumberToObject(0), "SYSTEMERRORCODE");
    }

    return true;
}

RexxMethod0(RexxStringObject, dlgutil_comctl32Version)
{
    return context->NewStringFromAsciiz(comctl32VersionName(ComCtl32Version));
}

RexxMethod0(RexxStringObject, dlgutil_version)
{
    char buf[64];
    _snprintf(buf, sizeof(buf), "ooDialog Version %u.%u.%u.%u (an ooRexx Windows Extension)", ORX_VER, ORX_REL, ORX_MOD, OOREXX_BLD);
    return context->NewStringFromAsciiz(buf);
}

RexxMethod3(uint32_t, dlgutil_colorRef, RexxObjectPtr, r, OPTIONAL_uint8_t, g, OPTIONAL_uint8_t, b)
{
    size_t count = rxArgCount(context);

    if ( count == 1 )
    {
        if (! context->IsString(r) )
        {
            wrongClassException(context, 1, "String");
            return 0;
        }
        const char * s = context->ObjectToStringValue(r);
        if ( *s == 'D' || *s == 'd' )
        {
            return CLR_DEFAULT;
        }
        else if ( *s == 'N' || *s == 'n' )
        {
            return CLR_NONE;
        }
        else
        {
            wrongArgValueException(context, 1, "DEFAULT, NONE", s);
            return 0;
        }
    }

    if ( count != 3 )
    {
        context->RaiseException1(Rexx_Error_Incorrect_method_minarg, context->WholeNumberToObject(3));
        return 0;
    }

    uint32_t red;
    if (!context->ObjectToUnsignedInt32(r, &red))
    {
        context->RaiseException2(Rexx_Error_Incorrect_method_whole, context->WholeNumberToObject(1), r);
        return 0;
    }

    return RGB((uint8_t)red, g, b);
}

RexxMethod1(uint8_t, dlgutil_getRValue, uint32_t, colorRef) { return GetRValue(colorRef); }
RexxMethod1(uint8_t, dlgutil_getGValue, uint32_t, colorRef) { return GetGValue(colorRef); }
RexxMethod1(uint8_t, dlgutil_getBValue, uint32_t, colorRef) { return GetBValue(colorRef); }
RexxMethod1(uint16_t, dlgutil_hiWord, uint32_t, dw) { return HIWORD(dw); }
RexxMethod1(uint16_t, dlgutil_loWord, uint32_t, dw) { return LOWORD(dw); }

/**
 * A temporary utility to convert from a handle that is still being stored in
 * ooDialog in string form ("0xFFFFAAAA") to its actual pointer value.  The
 * interface is needed to facilitate testing Windows extensions that have been
 * converted to only use pointer valules.
 */
RexxMethod1(POINTER, dlgutil_handleToPointer, CSTRING, handle)
{
    return string2pointer(handle);
}

