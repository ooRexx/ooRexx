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
#include <windows.h>
#include <mmsystem.h>
#define INCL_REXXSAA
#include <rexx.h>
#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include "oovutil.h"
#include <commctrl.h>

#define FILENAME_BUFFER_LEN 65535

extern LONG SetRexxStem(CHAR * name, INT id, char * secname, CHAR * data);
WORD NumDIBColorEntries(LPBITMAPINFO lpBmpInfo);
extern LPBITMAPINFO LoadDIB(LPSTR szFile);
extern LONG EvaluateListStyle(CHAR * styledesc);


ULONG APIENTRY PlaySoundFile(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

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


ULONG APIENTRY PlaySoundFileInLoop(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

{
   UINT opts;

   CHECKARG(1);

   opts = SND_ASYNC;

   if (sndPlaySound(argv[0].strptr,opts|SND_LOOP | SND_NODEFAULT))
      RETC(0)
   else
      RETC(1)
}


ULONG APIENTRY StopSoundFile(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

{
   UINT opts;

   opts = SND_SYNC;

   if (sndPlaySound(NULL,opts | SND_NODEFAULT))
      RETC(0)
   else
      RETC(1)
}


UINT APIENTRY OFNSetForegroundHookProc(
    HWND hdlg,    // handle to child dialog window
    UINT uiMsg,    // message identifier
    WPARAM wParam,    // message parameter
    LPARAM lParam     // message parameter
)
{
    if (uiMsg == WM_INITDIALOG)
    {
        HWND h = GetParent(hdlg);
        if (!h) h = hdlg;
        SetForegroundWindow(h);
    }
    return 0;   /* 0 means default routine handles message */
}


BOOL OpenFileDlg( BOOL load, PCHAR szFile, PCHAR szInitialDir, PCHAR szFilter, HWND hw, PCHAR title, PCHAR DefExt, BOOL multi, CHAR chSepChar) /* @DOE005M */
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

ULONG APIENTRY GetFileNameWindow(
    PUCHAR funcname,
    ULONG argc,
    RXSTRING argv[],
    PUCHAR qname,
    PRXSTRING retstr )

{
    BOOL    fSuccess;
    CHAR *  title;
    PCHAR   defext = "TXT";
    BOOL    load = TRUE;
    BOOL    multi = FALSE;
    HWND    hWnd;
    PCHAR   szFilter = "Text Files (*.TXT)\0*.TXT\0All Files (*.*)\0*.*\0";
    PCHAR   pszFiles = NULL;
    PCHAR   pszInitialDir = NULL;
    CHAR    chSepChar = ' ';  /* default separation character  /              */
                              /* allow to change separation character to      */
                              /* handle filenames with blank character        */

    pszFiles = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, FILENAME_BUFFER_LEN);
    if (!pszFiles)
        RETERR

    if (VALIDARG(1))
    {
        if ( argv[0].strptr[argv[0].strlength - 1] == '\\' )
        {
            pszInitialDir = LocalAlloc(LPTR, _MAX_PATH);
            if ( !pszInitialDir )
              RETERR
            rxstrlcpy(pszInitialDir, argv[0]);
        }
        else
        {
          rxstrlcpy(pszFiles, argv[0]);
        }
    }
    if (VALIDARG(2)) hWnd = (HWND) atol(argv[1].strptr); else hWnd = NULL;
    if (VALIDARG(3)) szFilter= argv[2].strptr;
    if (VALIDARG(4)) load = IsYes(argv[3].strptr);
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


ULONG APIENTRY PlaySnd(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

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


ULONG APIENTRY SleepMS(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

{
   CHECKARG(1);

   Sleep(atoi(argv[0].strptr));
   RETC(0)
}


ULONG APIENTRY WinTimer(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

{
   UINT timer;
   MSG msg;

   CHECKARG(2);
   if (!stricmp(argv[0].strptr, "START"))
   {
        timer = SetTimer(NULL, 1001, atoi(argv[1].strptr), NULL);
        RETVAL(timer)
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





HIMAGELIST CreateImageList(INT start, HWND h, RXSTRING argv[], ULONG argc)
{
   HBITMAP hBmp = NULL;
   HIMAGELIST iL;
   INT cx,cy, nr;
   BITMAP bmpInfo;

   if (atol(argv[start].strptr) > 0)
       hBmp = (HBITMAP)atol(argv[start].strptr);
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




//*-------------------------------------------------------------------
//| Title:
//|     SubClassProc
//|
//| Parameters:
//|     hWnd            - Handle to the message's destination window
//|     wMessage        - Message number of the current message
//|     wParam          - Additional info associated with the message
//|     lParam          - Additional info associated with the message
//|
//| Purpose:
//|     This is the window procedure used to subclass the edit control.
//*---------------------------------------------------------------------

WNDPROC lpOldEditProc = NULL;


long FAR PASCAL CatchReturnSubProc(HWND hWnd, WORD wMessage,WORD wParam,LONG lParam)
{
    switch (wMessage)
    {
        case WM_GETDLGCODE:
            return (DLGC_WANTALLKEYS |
                    CallWindowProc(lpOldEditProc, hWnd, wMessage,
                                   wParam, lParam));

        case WM_CHAR:
             //Process this message to avoid message beeps.
            if ((wParam == VK_RETURN) || (wParam == VK_ESCAPE))
                return 0;
            else
                return (CallWindowProc(lpOldEditProc, hWnd,wMessage, wParam, lParam));

        default:
            return (CallWindowProc(lpOldEditProc, hWnd, wMessage,
                                      wParam, lParam));
            break;

    } /* end switch */
}




ULONG APIENTRY HandleTreeCtrl(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   HWND h;

   CHECKARGL(2);

   h = (HWND) atol(argv[1].strptr);
   if (!h) RETERR;

   if (!strcmp(argv[0].strptr, "INS"))
   {
       TV_INSERTSTRUCT ins;
       TV_ITEM * tvi = &ins.item;

       CHECKARG(9);
       ins.hParent = (HTREEITEM)atol(argv[2].strptr);
       if (!ins.hParent && !strcmp(argv[2].strptr,"ROOT")) ins.hParent = TVI_ROOT;
       ins.hInsertAfter = (HTREEITEM)atol(argv[3].strptr);
       if (!ins.hInsertAfter)
       {
           if (!strcmp(argv[3].strptr,"FIRST")) ins.hInsertAfter = TVI_FIRST;
           else if (!strcmp(argv[3].strptr,"SORT")) ins.hInsertAfter = TVI_SORT;
           else ins.hInsertAfter = TVI_LAST;
       }

       tvi->mask = TVIF_TEXT;

       tvi->pszText = argv[4].strptr;
       tvi->cchTextMax = argv[4].strlength;

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

       RETVAL((LONG)TreeView_InsertItem(h, &ins));
   }
   else
   if (!strcmp(argv[0].strptr, "DEL"))
   {
       HTREEITEM hItem;

       CHECKARG(3);
       hItem = (HTREEITEM)atol(argv[2].strptr);
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

       tvi.hItem = (HTREEITEM)atol(argv[2].strptr);

       // tvi.mask = TVIF_HANDLE;
       tvi.mask = 0;

       if (argv[3].strlength)
       {
           tvi.pszText = argv[3].strptr;
           tvi.cchTextMax = argv[3].strlength;
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

       tvi.hItem = (HTREEITEM)atol(argv[2].strptr);
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

       hItem = (HTREEITEM)atol(argv[2].strptr);

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
       RETVAL((LONG)TreeView_GetNextItem(h, hItem, flag))
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

       hItem = (HTREEITEM)atol(argv[2].strptr);

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

       hItem = (HTREEITEM)atol(argv[2].strptr);

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

       hItem = (HTREEITEM)atol(argv[2].strptr);

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

       RETVAL((LONG)TreeView_EditLabel(h, (HTREEITEM)atol(argv[2].strptr)))
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

       RETC(!TreeView_SortChildren(h, (HTREEITEM)atol(argv[2].strptr), IsYes(argv[3].strptr)))
   }
   else
   if (!strcmp(argv[0].strptr, "SETIMG"))
   {
       HIMAGELIST iL;

       CHECKARG(5);
       iL = CreateImageList(2, h, argv, argc);

       if (iL) RETVAL((LONG)TreeView_SetImageList(h, iL, TVSIL_NORMAL))
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
           WNDPROC oldProc = (WNDPROC)SetWindowLong(ew, GWL_WNDPROC, (LONG)CatchReturnSubProc);
           if (oldProc != (WNDPROC)CatchReturnSubProc) lpOldEditProc = oldProc;
           RETVAL((LONG)oldProc)
       }
       else RETC(0)
   }
   else
   if (!strcmp(argv[0].strptr, "RESUB_EDIT"))
   {
       HWND ew = TreeView_GetEditControl(h);
       if (ew)
       {
           SetWindowLong(ew, GWL_WNDPROC, (LONG)lpOldEditProc);
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
           ltoa((LONG)hItem, retstr->strptr, 10); /* removed compiler warning */
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






ULONG APIENTRY HandleListCtrl(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   HWND h;

   CHECKARGL(3);

   h = (HWND) atol(argv[2].strptr);
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

           lvi.pszText = argv[4].strptr;
           lvi.cchTextMax = argv[4].strlength;

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

           lvi.pszText = argv[5].strptr;
           lvi.cchTextMax = argv[5].strlength;

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
               len = SendMessage(h, LVM_GETITEMTEXT, lvi.iItem, (LPARAM)&lvi);
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

           if (iL) RETVAL((LONG)ListView_SetImageList(h, iL, ilt))
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

           RETVAL((LONG)ListView_EditLabel(h, atol(argv[3].strptr)))
       }
       else
       if (!strcmp(argv[1].strptr, "SUBCL_EDIT"))
       {
           HWND ew = ListView_GetEditControl(h);
           if (ew)
           {
               WNDPROC oldProc = (WNDPROC)SetWindowLong(ew, GWL_WNDPROC, (LONG)CatchReturnSubProc);
               if (oldProc != (WNDPROC)CatchReturnSubProc) lpOldEditProc = oldProc;
               RETVAL((LONG)oldProc)
           }
           else RETC(0)
       }
       else
       if (!strcmp(argv[1].strptr, "RESUB_EDIT"))
       {
           HWND ew = ListView_GetEditControl(h);
           if (ew)
           {
               SetWindowLong(ew, GWL_WNDPROC, (LONG)lpOldEditProc);
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
           LV_COLUMN lvi;

           CHECKARG(7);

           lvi.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;

           lvi.iSubItem = atoi(argv[3].strptr);

           lvi.pszText = argv[4].strptr;
           lvi.cchTextMax = argv[4].strlength;

           lvi.cx = atoi(argv[5].strptr);
           if (lvi.cx >= 0) lvi.mask |= LVCF_WIDTH;

           if (strstr(argv[6].strptr,"CENTER")) lvi.fmt = LVCFMT_CENTER;
           else if (strstr(argv[6].strptr,"RIGHT")) lvi.fmt = LVCFMT_RIGHT;
           else lvi.fmt = LVCFMT_LEFT;

           RETVAL(ListView_InsertColumn(h, lvi.iSubItem, &lvi));
       }
       else
       if (!strcmp(argv[1].strptr, "SET"))
       {
           LV_COLUMN lvi;
           LONG nr;

           CHECKARG(7);

           lvi.mask = 0;

           nr = atoi(argv[3].strptr);

           lvi.pszText = argv[4].strptr;
           lvi.cchTextMax = argv[4].strlength;
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
               if (lvi.fmt == LVCFMT_CENTER) strcpy(data, "CENTER");
               else if (lvi.fmt == LVCFMT_RIGHT) strcpy(data, "RIGHT");
               else strcpy(data, "LEFT");
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



ULONG APIENTRY HandleOtherNewCtrls(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   HWND h;

   CHECKARGL(3);

   h = (HWND) atol(argv[2].strptr);
   if (!h) RETERR;

   if (!strcmp(argv[0].strptr, "PROGRESS"))
   {
       if (!strcmp(argv[1].strptr, "STEP"))
       {
           RETVAL(SendMessage(h, PBM_STEPIT, 0, 0))
       }
       else
       if (!strcmp(argv[1].strptr, "POS"))
       {
           CHECKARG(4);
           RETVAL(SendMessage(h, PBM_SETPOS, atoi(argv[3].strptr), 0))
       }
       else
       if (!strcmp(argv[1].strptr, "SETSTEP"))
       {
           CHECKARG(4);
           RETVAL(SendMessage(h, PBM_SETSTEP, atoi(argv[3].strptr), 0))
       }
       else
       if (!strcmp(argv[1].strptr, "DELTA"))
       {
           CHECKARG(4);
           RETVAL(SendMessage(h, PBM_DELTAPOS, atoi(argv[3].strptr), 0))
       }
       else
       if (!strcmp(argv[1].strptr, "RANGE"))
       {
           CHECKARG(5);
           RETVAL(SendMessage(h, PBM_SETRANGE, 0, MAKELPARAM(atoi(argv[3].strptr), atoi(argv[4].strptr))))
       }
   }
   else
   if (!strcmp(argv[0].strptr, "SLIDER"))
   {
       if (!strcmp(argv[1].strptr, "POS"))
       {
           CHECKARGL(4);
           if (argc == 4)
               RETVAL(SendMessage(h, TBM_GETPOS, 0, 0))
           else
               SendMessage(h, TBM_SETPOS, IsYes(argv[4].strptr), atol(argv[3].strptr));
       }
       else
       if (!strcmp(argv[1].strptr, "SETRANGE"))
       {
           CHECKARG(6);
           if (argv[3].strptr[0] == 'L')
               SendMessage(h, TBM_SETRANGEMIN, IsYes(argv[5].strptr), atol(argv[4].strptr));
           else if (argv[3].strptr[0] == 'H')
               SendMessage(h, TBM_SETRANGEMAX, IsYes(argv[5].strptr), atol(argv[4].strptr));
           else
               SendMessage(h, TBM_SETRANGE, IsYes(argv[5].strptr), MAKELONG(atol(argv[3].strptr), atol(argv[4].strptr)));
       }
       else
       if (!strcmp(argv[1].strptr, "GETRANGE"))
       {
           sprintf(retstr->strptr, "%d %d",SendMessage(h, TBM_GETRANGEMIN, 0,0), SendMessage(h, TBM_GETRANGEMAX, 0,0));
           retstr->strlength = strlen(retstr->strptr);
           return 0;
       }
       else
       if (!strcmp(argv[1].strptr, "TICS"))
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
               RETVAL(SendMessage(h, TBM_GETNUMTICS, 0, 0))
           }
           else if (argv[3].strptr[0] == 'G')
           {
               CHECKARG(5);
               RETVAL(SendMessage(h, TBM_GETTIC, atoi(argv[4].strptr), 0))
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
       else
       if (!strcmp(argv[1].strptr, "GETSTEPS"))
       {
           CHECKARG(4);
           if (argv[3].strptr[0] == 'L')
               RETVAL(SendMessage(h, TBM_GETLINESIZE, 0, 0))
           else RETVAL(SendMessage(h, TBM_GETPAGESIZE, 0, 0));
       }
       else
       if (!strcmp(argv[1].strptr, "SETSTEPS"))
       {
           CHECKARG(5);
           if (argv[3].strptr[0] == 'L')
               RETVAL(SendMessage(h, TBM_SETLINESIZE, 0, atol(argv[4].strptr)))
           else RETVAL(SendMessage(h, TBM_SETPAGESIZE, 0, atol(argv[4].strptr)));
       }
       else
       if (!strcmp(argv[1].strptr, "SETSEL"))
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
       else
       if (!strcmp(argv[1].strptr, "GETSEL"))
       {
           sprintf(retstr->strptr, "%d %d",SendMessage(h, TBM_GETSELSTART, 0,0), SendMessage(h, TBM_GETSELEND, 0,0));
           retstr->strlength = strlen(retstr->strptr);
           return 0;
       }
   }
   else
   if (!strcmp(argv[0].strptr, "TAB"))
   {
       if (!strcmp(argv[1].strptr, "INS"))
       {
           TC_ITEM tab;
           INT item;
           CHECKARGL(6);

           item = atoi(argv[3].strptr);
           tab.mask = TCIF_TEXT;

           tab.pszText = argv[4].strptr;
           tab.cchTextMax = argv[4].strlength;

           tab.iImage = atoi(argv[5].strptr);
           if (tab.iImage >= 0) tab.mask |= TCIF_IMAGE;

           if (argc == 7)
           {
               tab.lParam = atol(argv[6].strptr);
               tab.mask |= TCIF_PARAM;
           }
           RETVAL(TabCtrl_InsertItem(h, item, &tab));
       }
       else
       if (!strcmp(argv[1].strptr, "SET"))
       {
           TC_ITEM tab;
           INT item;
           CHECKARGL(6);

           item = atoi(argv[3].strptr);

           if (strlen(argv[4].strptr)) tab.mask = TCIF_TEXT; else tab.mask = 0;
           tab.pszText = argv[4].strptr;
           tab.cchTextMax = argv[4].strlength;

           tab.iImage = atoi(argv[5].strptr);
           if (tab.iImage >= 0) tab.mask |= TCIF_IMAGE;

           if (argc == 7)
           {
               tab.lParam = atol(argv[6].strptr);
               tab.mask |= TCIF_PARAM;
           }
           RETC(!TabCtrl_SetItem(h, item, &tab));
       }
       else
       if (!strcmp(argv[1].strptr, "GET"))
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
               itoa(tab.lParam, data, 10);
               SetRexxStem(argv[4].strptr, -1, "!Param", data);
               RETC(0)
           }
           else RETVAL(-1)
       }
       else
       if (!strcmp(argv[1].strptr, "SEL"))
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
       else
       if (!strcmp(argv[1].strptr, "FOCUS"))
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
       else
       if (!strcmp(argv[1].strptr, "DEL"))
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
       else
       if (!strcmp(argv[1].strptr, "CNT"))
       {
           RETVAL(TabCtrl_GetItemCount(h))
       }
       else
       if (!strcmp(argv[1].strptr, "ROWCNT"))
       {
           RETVAL(TabCtrl_GetRowCount(h))
       }
       else
       if (!strcmp(argv[1].strptr, "SETIMG"))
       {
           HIMAGELIST iL;

           CHECKARG(6);
           iL = CreateImageList(3, h, argv, argc);
           if (iL) RETVAL((LONG)TabCtrl_SetImageList(h, iL))
           else RETC(0)
       }
       else
       if (!strcmp(argv[1].strptr, "UNSETIMG"))
       {
           HIMAGELIST iL;

           iL = TabCtrl_GetImageList(h);
           if (!iL) RETC(1)
           TabCtrl_SetImageList(h, NULL);

           RETC(!ImageList_Destroy( iL))
       }
       else
       if (!strcmp(argv[1].strptr, "PADDING"))
       {
           CHECKARG(5);

           TabCtrl_SetPadding(h, atoi(argv[3].strptr), atoi(argv[4].strptr));
           RETC(0);
       }
       else
       if (!strcmp(argv[1].strptr, "SIZE"))
       {
           LONG prevsize;
           CHECKARG(5);

           prevsize = TabCtrl_SetItemSize(h, atoi(argv[3].strptr), atoi(argv[4].strptr));
           sprintf(retstr->strptr, "%d %d", LOWORD(prevsize), HIWORD(prevsize));
           retstr->strlength = strlen(retstr->strptr);
           return 0;
       }
       else
       if (!strcmp(argv[1].strptr, "RECT"))
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
       else
       if (!strcmp(argv[1].strptr, "ADJUST"))
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
