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
#define NTDDI_VERSION   NTDDI_WINXPSP2
#define _WIN32_WINNT    0x0501
#define WINVER          0x0501

#define STRICT

#include <windows.h>
#include <mmsystem.h>
#include "oorexxapi.h"
#include <RexxErrorCodes.h>
#include <rexx.h>
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
   UINT_PTR timer;
   MSG msg;

   CHECKARG(2);
   if (!stricmp(argv[0].strptr, "START"))
   {
        timer = SetTimer(NULL, 1001, atoi(argv[1].strptr), NULL);
        RETPTR(timer)
   } else
   if (!stricmp(argv[0].strptr, "STOP"))
   {
        timer = KillTimer(NULL, atoi(argv[1].strptr));
        RETC(0)
   } else
   if (!stricmp(argv[0].strptr, "WAIT"))
   {
      while (!PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE) || (msg.wParam != (ULONG)atoi(argv[1].strptr))) {};
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
size_t RexxEntry HandleControlEx(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
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
            RETVAL(GetWindowLong(hCtrl, GWL_USERDATA));
        }
        else if ( argc == 4 )
        {
            RETVAL(SetWindowLong(hCtrl, GWL_USERDATA, atol(argv[3].strptr)));
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


/* These inline (and non-inline) convenience functions will be move so that they
 * are accessible by all of ooDialog at some point.  Right now they are just
 * used by the Progress Bar native method functions.
 */
inline HWND rxGetWindowHandle(RexxMethodContext * context, RexxObjectPtr self)
{
    RexxStringObject rxString = (RexxStringObject)context->SendMessage0(self, "HWND");
    return GET_HWND(context->StringData(rxString));
}

inline bool rxArgOmitted(RexxMethodContext * context, size_t index)
{
    return context->ArrayHasIndex(context->GetArguments(), index) == 0 ? true : false;
}

inline bool rxArgExists(RexxMethodContext * context, size_t index)
{
    return context->ArrayHasIndex(context->GetArguments(), index) == 1 ? true : false;
}

/**
 * Return the number of existing arguments passed to a native API Rexx method,
 * (as opposed to the size of the argument array.)
 *
 * @param context  The method context pointer for the native method.
 *
 * @return The count of existing arguments in the argument array.
 */
inline size_t rxArgCount(RexxMethodContext * context)
{
    size_t j = 0;
    size_t count = context->ArraySize(context->GetArguments());
    for( size_t i = 1; i <= count; i++ )
    {
        if ( rxArgExists(context, i) )
        {
            j++;
        }
    }
    return j;
}

inline bool hasStyle(HWND hwnd, DWORD_PTR style)
{
    if ( (GetWindowLongPtr(hwnd, GWL_STYLE) & style) || (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & style) )
    {
        return true;
    }
    return false;
}

void comCtl32Exception(RexxMethodContext *context, const char *methodName)
{
    char msg[128];
    sprintf(msg, "The %s method requires Windows XP or later", methodName);
    context->RaiseException1(Error_System_service_user_defined, context->NewStringFromAsciiz(msg));
}

void wrongWindowStyleException(RexxMethodContext *context, const char *obj, const char *style)
{
    char msg[128];
    sprintf(msg, "This %s does not have the %s style", obj, style);
    context->RaiseException1(Error_Incorrect_method_user_defined, context->NewStringFromAsciiz(msg));
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

    if ( rxArgOmitted(context, 1) )
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

RexxMethod3(RexxObjectPtr, pbc_setRange, OSELF, self, OPTIONAL_int32_t, min, OPTIONAL_int32_t, max)
{
    TCHAR buf[64];
    HWND hwnd = rxGetWindowHandle(context, self);

    if ( rxArgOmitted(context, 1) )
    {
        min = 0;
    }
    if ( rxArgOmitted(context, 2) )
    {
        max = 100;
    }

    DWORD range = (DWORD)SendMessage(hwnd, PBM_SETRANGE32, min, max);
    _snprintf(buf, sizeof(buf), "%d %d", LOWORD(range), HIWORD(range));

    return context->NewStringFromAsciiz(buf);
}

RexxMethod1(RexxObjectPtr, pbc_getRange, OSELF, self)
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

    if ( rxArgOmitted(context, 1) )
    {
        newStep = 10;
    }
    return (int)SendMessage(hwnd, PBM_SETSTEP, newStep, 0);
}

RexxMethod3(logical_t, pbc_setMarquee, OSELF, self, OPTIONAL_logical_t, on, OPTIONAL_uint32_t, pause)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    /* Requires XP Common Controls version 6.0 */
    if (  ComCtl32Version < COMCTL32_6_0 )
    {
        comCtl32Exception(context, "setMarquee");
        return 0;
    }

    if ( ! hasStyle(hwnd, PBS_MARQUEE) )
    {
        wrongWindowStyleException(context, "progress bar", "PBS_MARQUEE");
        return 0;
    }

    if ( rxArgOmitted(context, 1) )
    {
        on = 1;
    }
    if ( rxArgOmitted(context, 2) )
    {
        pause = 1000;
    }

    /* The Windows message always returns 1, return 1 for .true (succeeded.) */
    SendMessage(hwnd, PBM_SETMARQUEE, on, pause);
    return 1;
}

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
        context->RaiseException1(Error_Incorrect_method_minarg, context->NewInteger(3));
        return 0;
    }

    return (uint32_t)SendMessage(hwnd, PBM_SETBKCOLOR, 0, rgb);
}

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
        context->RaiseException1(Error_Incorrect_method_minarg, context->NewInteger(3));
        return 0;
    }

    return (uint32_t)SendMessage(hwnd, PBM_SETBARCOLOR, 0, rgb);
}

RexxMethod5(logical_t, pbc_test, OSELF, self, OPTIONAL_int32_t, n1,
            OPTIONAL_int32_t, n2, OPTIONAL_int32_t, n3, OPTIONAL_int32_t, n4)
{
    printf("pbc_test arg count=%d\n", rxArgCount(context));
    printf("pbc_test arg 1 omitted? %d\n", rxArgOmitted(context, 1));
    printf("pbc_test arg 2 omitted? %d\n", rxArgOmitted(context, 2));
    printf("pbc_test arg 3 omitted? %d\n", rxArgOmitted(context, 3));
    printf("pbc_test arg 4 omitted? %d\n", rxArgOmitted(context, 4));

    return 1;
}

#define COMCTL_ERR_TITLE    "ooDialog - Windows Common Controls Error"
#define GENERIC_ERR_TITLE   "ooDialog - Error"

extern DWORD ComCtl32Version = 0;

/**
 * Convenience function to put up an error message box.
 *
 * @param pszMsg    The message.
 * @param pszTitle  The title of for the message box.
 */
static void internalErrorMsg(PSZ pszMsg, PSZ pszTitle)
{
    MessageBox(0, pszMsg, pszTitle, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
}

/**
 * Determines the version of comctl32.dll and initializes the common controls.
 *
 * The minimum version of 4.71 is supported on Windows 95 with Internet Explorer
 * 4.0, Windows NT 4.0 with Internet Explorer 4.0, Windows 98, and Windows 2000.
 *
 * @return .true if comctl32.dll is at least version 4.71, otherwise .false.
 */
RexxMethod0(logical_t, dlgutil_commonInit)
{
    static bool CommonInitDone = false;
    HINSTANCE   hinst;
    bool        success = false;

    if ( CommonInitDone )
    {
        return (ComCtl32Version >= COMCTL32_4_71 ? 1 : 0);
    }

    hinst = LoadLibrary(TEXT("comctl32.dll"));
    if ( hinst )
    {
        DLLGETVERSIONPROC pDllGetVersion;

        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinst, "DllGetVersion");
        if ( pDllGetVersion )
        {
            DLLVERSIONINFO info;

            ZeroMemory(&info, sizeof(info));
            info.cbSize = sizeof(info);
            if ( SUCCEEDED((*pDllGetVersion)(&info)) )
                ComCtl32Version = MAKEVERSION(info.dwMajorVersion, info.dwMinorVersion);
        }
        FreeLibrary(hinst);
    }

    if ( ComCtl32Version == 0 )
    {
        internalErrorMsg("The version of the Windows Common Controls library (comctl32.dll)\n"
                         "could not be determined.  ooDialog will not run", COMCTL_ERR_TITLE);
    }
    else if ( ComCtl32Version < COMCTL32_4_71 )
    {
        CHAR msg[256];
        sprintf(msg, "ooDialog can not run with this version of the Windows Common Controls library\n"
                "(comctl32.dll.)  The minimum version required is 4.71.\n\nThis system has version: %s\n",
                ComCtl32Version == COMCTL32_4_0 ? "4.0" : "4.7" );

        internalErrorMsg(msg, COMCTL_ERR_TITLE);
        ComCtl32Version = 0;
    }
    else
    {
        INITCOMMONCONTROLSEX ctrlex;

        ctrlex.dwSize = sizeof(ctrlex);
        ctrlex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
        if ( ! InitCommonControlsEx(&ctrlex) )
        {
            CHAR msg[128];
            sprintf(msg, "Initializing the Windows Common Controls library (InitCommonControlsEx)\n"
                    "failed.  Windows System Error Code: %d\n", GetLastError());
            internalErrorMsg(msg, COMCTL_ERR_TITLE);
            ComCtl32Version = 0;
        }
        else
        {
            success = true;
        }
    }

    CommonInitDone = true;
    return (success ? 1 : 0);
}


RexxMethod3(uint32_t, dlgutil_colorRef, RexxObjectPtr, r, OPTIONAL_uint8_t, g, OPTIONAL_uint8_t, b)
{
    size_t count = rxArgCount(context);

    if ( count == 1 )
    {
        if ( ! context->IsString(r) )
        {
            context->RaiseException2(Error_Incorrect_method_noclass,
                                     context->NewInteger(1),
                                     context->NewStringFromAsciiz("String"));
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
            context->RaiseException2(Error_Incorrect_method_list,
                                     context->NewInteger(1),
                                     context->NewStringFromAsciiz("DEFAULT, NONE"));
            return 0;
        }
    }

    if ( count != 3 )
    {
        context->RaiseException1(Error_Incorrect_method_minarg, context->NewInteger(3));
        return 0;
    }

    if ( ! context->IsInteger(r) )
    {
        context->RaiseException2(Error_Incorrect_method_whole, context->NewInteger(1), r);
        return 0;
    }

    uint32_t red;
    context->ObjectToUnsignedNumber(r, &red);
    return RGB((uint8_t)red, g, b);
}

