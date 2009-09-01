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
#include "ooDialog.h"     // Must be first, includes windows.h and oorexxapi.h

#include <mmsystem.h>
#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include <errno.h>
#include <shlwapi.h>
#include <commctrl.h>
#include "APICommon.h"
#include "oodCommon.h"

// Map strings representing constant defines to their int values.  For
// translating things like "IDI_APPLICATION" from the user to the proper API
// value.
#include <string>
#include <map>
using namespace std;
typedef map<string, int, less<string> > String2Int;

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

   if ((argc > 1) && (isYes(argv[1].strptr)))
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


size_t RexxEntry PlaySnd(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   UINT opts;
   DEF_ADM;

   CHECKARGL(2);

   GET_ADM;

   if (!dlgAdm) RETERR

   if ((argc > 2) && (isYes(argv[2].strptr)))
      opts = SND_ASYNC;
   else
      opts = SND_SYNC;

   if (PlaySound(MAKEINTRESOURCE(atoi(argv[1].strptr)),dlgAdm->TheInstance, SND_RESOURCE | opts | SND_NODEFAULT))
      RETC(0)
   else
      RETC(1)
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

       RETC(!TreeView_EndEditLabelNow(h, isYes(argv[2].strptr)))
   }
   else
   if (!strcmp(argv[0].strptr, "SORT"))
   {
       CHECKARG(4);

       HTREEITEM hItem = (HTREEITEM)GET_HANDLE(argv[2]);
       RETC(!TreeView_SortChildren(h, (HTREEITEM)hItem, isYes(argv[3].strptr)))
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
        if ( ! checkControlClass(hCtrl, winEdit) )
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
           RETC(!ListView_EnsureVisible(h, atol(argv[3].strptr), isYes(argv[4].strptr)))
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
           RETVAL(ListView_GetItemSpacing(h, isYes(argv[3].strptr)));
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
               SendMessage(h, TBM_SETPOS, isYes(argv[4].strptr), atol(argv[3].strptr));
       }
       else if (!strcmp(argv[1].strptr, "SETRANGE"))
       {
           CHECKARG(6);
           if (argv[3].strptr[0] == 'L')
               SendMessage(h, TBM_SETRANGEMIN, isYes(argv[5].strptr), atol(argv[4].strptr));
           else if (argv[3].strptr[0] == 'H')
               SendMessage(h, TBM_SETRANGEMAX, isYes(argv[5].strptr), atol(argv[4].strptr));
           else
               SendMessage(h, TBM_SETRANGE, isYes(argv[5].strptr), MAKELONG(atol(argv[3].strptr), atol(argv[4].strptr)));
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
               SendMessage(h, TBM_CLEARTICS, isYes(argv[4].strptr), 0);
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
               SendMessage(h, TBM_CLEARSEL, isYes(argv[4].strptr), 0);
               RETC(0);
           }
           CHECKARG(6);
           if (argv[3].strptr[0] == 'S')
               SendMessage(h, TBM_SETSELSTART, isYes(argv[5].strptr), atol(argv[4].strptr));
           else if (argv[3].strptr[0] == 'E')
               SendMessage(h, TBM_SETSELEND, isYes(argv[5].strptr), atol(argv[4].strptr));
           else
               SendMessage(h, TBM_SETSEL, isYes(argv[5].strptr), MAKELONG(atol(argv[3].strptr), atol(argv[4].strptr)));
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

           adapt = isYes(argv[3].strptr);

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

void controlFailedException(RexxMethodContext *c, const char *msg, const char *func, const char *control)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), msg, func, control);
    systemServiceException(c, buffer);
}

void wrongWindowStyleException(RexxMethodContext *c, const char *obj, const char *style)
{
    char msg[128];
    _snprintf(msg, sizeof(msg), "This %s does not have the %s style", obj, style);
    userDefinedMsgException(c, msg);
}

inline bool hasStyle(HWND hwnd, LONG style)
{
    if ( (GetWindowLong(hwnd, GWL_STYLE) & style) || (GetWindowLong(hwnd, GWL_EXSTYLE) & style) )
    {
        return true;
    }
    return false;
}

RexxObjectPtr oodSetImageAttribute(RexxMethodContext *c, CSTRING varName, RexxObjectPtr image, HWND hwnd,
                                   HANDLE hOldImage, uint8_t type, oodControl_t ctrl)
{
    RexxObjectPtr result = c->GetObjectVariable(varName);
    if ( result == NULLOBJECT )
    {
        result = TheNilObj;
    }
    c->SetObjectVariable(varName, image);

    // It could be that the existing image was set from a resource DLL.  In
    // which case we need to create an .Image object.
    if ( result == TheNilObj && hOldImage != NULL )
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
            result = TheNilObj;
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
                invalidTypeException(context, 3, " handle to a device context");
            }
            GetTextExtentPoint32(hdc, text, (int)strlen(text), &size);
        }
        else if ( m == 'F' )
        {
            HFONT hFont = (HFONT)string2pointer(fontSrc);
            if ( hFont == NULL )
            {
                invalidTypeException(context, 3, " handle to a font");
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

bool rxIntFromDirectory(RexxMethodContext *context, RexxDirectoryObject d, CSTRING index,
                        int *number, int argPos)
{
    int value;
    RexxObjectPtr obj = context->DirectoryAt(d, index);
    if ( obj != NULLOBJECT )
    {
        if ( ! context->Int32(obj, &value) )
        {
            wrongObjInDirectoryException(context, argPos, index, "an integer", obj);
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

    oodResetSysErrCode(context);

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
 * Methods for the DateTimePicker class.
 */
#define DATETIMEPICKER_CLASS     "DateTimePicker"
#define DATETIMEPICKER_WINNAME   "Date and Time Picker"

// This is used for MonthCalendar also
#define SYSTEMTIME_MIN_YEAR    1601

enum DateTimePart {dtFull, dtTime, dtDate, dtNow};

/**
 * Converts a DateTime object to a SYSTEMTIME structure.  The fields of the
 * struct are filled in with the corresponding values of the DateTime object.
 *
 * @param c         The method context we are operating in.
 * @param dateTime  An ooRexx DateTime object.
 * @param sysTime   [in/out] The SYSTEMTIME struct to fill in.
 * @param part      Specifies which fields of the SYSTEMTIME struct fill in.
 *                  Unspecified fields are left alone.
 *
 * @return True if no errors, false if a condition is raised.
 *
 * @note  Assumes the dateTime object is not null and is actually a DateTime
 *        object.
 *
 * @note The year part of the DateTime object must be in range for a SYSTEMTIME.
 *       The lower range for SYSTEMTIME is 1601. The upper range of a DateTime
 *       object is 9999 and of a SYSTEMTIME 30827, so we only check the lower
 *       range.  An exception is raised if out of range.
 */
static bool dt2sysTime(RexxMethodContext *c, RexxObjectPtr dateTime, SYSTEMTIME *sysTime, DateTimePart part)
{
    if ( part == dtNow )
    {
        GetLocalTime(sysTime);
    }
    else
    {
        // format: yyyy-dd-mmThh:mm:ss.uuuuuu.
        RexxObjectPtr dt = c->SendMessage0(dateTime, "ISODATE");
        const char *isoDate = c->CString(dt);

        sscanf(isoDate, "%4hu-%2hu-%2huT%2hu:%2hu:%2hu.%3hu", &(*sysTime).wYear, &(*sysTime).wMonth, &(*sysTime).wDay,
               &(*sysTime).wHour, &(*sysTime).wMinute, &(*sysTime).wSecond, &(*sysTime).wMilliseconds);

        SYSTEMTIME st = {0};
        sscanf(isoDate, "%4hu-%2hu-%2huT%2hu:%2hu:%2hu.%3hu", &st.wYear, &st.wMonth, &st.wDay,
               &st.wHour, &st.wMinute, &st.wSecond, &st.wMilliseconds);

        if ( st.wYear < SYSTEMTIME_MIN_YEAR )
        {
            userDefinedMsgException(c, "The DateTime object can not represent a year prior to 1601");
            goto failed_out;
        }

        switch ( part )
        {
            case dtTime :
                sysTime->wHour = st.wHour;
                sysTime->wMinute = st.wMinute;
                sysTime->wSecond = st.wSecond;
                sysTime->wMilliseconds = st.wMilliseconds;
                break;

            case dtDate :
                sysTime->wYear = st.wYear;
                sysTime->wMonth = st.wMonth;
                sysTime->wDay = st.wDay;
                break;

            case dtFull :
                sysTime->wYear = st.wYear;
                sysTime->wMonth = st.wMonth;
                sysTime->wDay = st.wDay;
                sysTime->wHour = st.wHour;
                sysTime->wMinute = st.wMinute;
                sysTime->wSecond = st.wSecond;
                sysTime->wMilliseconds = st.wMilliseconds;
                break;
        }
    }
    return true;

failed_out:
    return false;
}

/**
 * Creates a DateTime object that represents the time set in a SYSTEMTIME
 * struct.
 *
 * @param c
 * @param sysTime
 * @param dateTime  [in/out]
 */
static void sysTime2dt(RexxMethodContext *c, SYSTEMTIME *sysTime, RexxObjectPtr *dateTime, DateTimePart part)
{
    RexxClassObject dtClass = c->FindClass("DATETIME");

    if ( part == dtNow )
    {
        *dateTime = c->SendMessage0(dtClass, "NEW");
    }
    else
    {
        char buf[64];
        switch ( part )
        {
            case dtDate :
                _snprintf(buf, sizeof(buf), "%hu%02hu%02hu", sysTime->wYear, sysTime->wMonth, sysTime->wDay);
                *dateTime = c->SendMessage1(dtClass, "FROMSTANDARDDATE", c->String(buf));
                break;

            case dtTime :
                _snprintf(buf, sizeof(buf), "%02hu:%02hu:%02hu.%03hu000",
                          sysTime->wHour, sysTime->wMinute, sysTime->wSecond, sysTime->wMilliseconds);
                *dateTime = c->SendMessage1(dtClass, "FROMLONGTIME", c->String(buf));
                break;

            case dtFull :
                _snprintf(buf, sizeof(buf), "%hu-%02hu-%02huT%02hu:%02hu:%02hu.%03hu000",
                          sysTime->wYear, sysTime->wMonth, sysTime->wDay,
                          sysTime->wHour, sysTime->wMinute, sysTime->wSecond, sysTime->wMilliseconds);
                *dateTime = c->SendMessage1(dtClass, "FROMISODATE", c->String(buf));
                break;
        }
    }
}

/** DateTimePicker::dateTime  (attribute)
 *
 *  Retrieves the current selected system time of the date time picker and
 *  returns it as a DateTime object.
 *
 *  If the date time picker has the DTS_SHOWNONE style, it can also be set to
 *  "no date" when the user has unchecked the check box.  If the control is in
 *  this state, the .NullHandle object is returned to the user.
 *
 *  @returns  A DateTime object representing the current selected system time of
 *            the control, or the .NullHandle object if the control is in the
 *            'no date' state.
 */
RexxMethod1(RexxObjectPtr, get_dtp_dateTime, OSELF, self)
{
    RexxMethodContext *c = context;
    SYSTEMTIME sysTime = {0};
    RexxObjectPtr dateTime = NULLOBJECT;

    switch ( DateTime_GetSystemtime(rxGetWindowHandle(context, self), &sysTime) )
    {
        case GDT_VALID:
            sysTime2dt(context, &sysTime, &dateTime, dtFull);
            break;

        case GDT_NONE:
            // This is valid.  It means the DTP is using the DTS_SHOWNONE  style
            // and that the user has the check box is not checked.  We return a
            // null pointer object.
            dateTime = c->NewPointer(NULL);
            break;

        case GDT_ERROR:
        default :
            // Some error with the DTP, raise an exception.
            controlFailedException(context, FUNC_WINCTRL_FAILED_MSG, "DateTime_GetSystemtime", DATETIMEPICKER_WINNAME);
            break;
    }
    return dateTime;
}

/** DateTimePicker::dateTime=  (attribute)
 *
 *  Sets the system time for the date time picker to the time represented by the
 *  DateTime object.  If, and only if, the date time picker has the DTS_SHOWNONE
 *  style, it can also be set to "no date."  The Rexx user can set this state by
 *  passing in the .NullHandle object.
 *
 *  @param dateTime  The date and time to set the control to.
 *
 *  @return   This is an attribute, there is no return.
 *
 *  @note  The minimum year a date time picker can be set to is 1601.  If the
 *         DateTime object represents a year prior to 1601, an exception is
 *         raised.
 *
 */
RexxMethod2(RexxObjectPtr, set_dtp_dateTime, RexxObjectPtr, dateTime, OSELF, self)
{
    RexxMethodContext *c = context;
    SYSTEMTIME sysTime = {0};
    HWND hwnd = rxGetWindowHandle(context, self);

    if ( c->IsOfType(dateTime, "POINTER") )
    {
        DateTime_SetSystemtime(hwnd, GDT_NONE, &sysTime);
    }
    else
    {
        if ( requiredClass(context, dateTime, "DATETIME", 1) )
        {
            if ( dt2sysTime(c, dateTime, &sysTime, dtFull) )
            {
                if ( DateTime_SetSystemtime(hwnd, GDT_VALID, &sysTime) == 0 )
                {
                    controlFailedException(context, FUNC_WINCTRL_FAILED_MSG, "DateTime_SetSystemtime", DATETIMEPICKER_WINNAME);
                }
            }
        }
    }
    return NULLOBJECT;
}


/**
 * Methods for the MonthCalendar class.
 */
#define MONTHCALENDAR_CLASS    "MonthCalendar"
#define MONTHCALENDAR_WINNAME  "Month Calendar"

RexxMethod1(RexxObjectPtr, get_mc_date, OSELF, self)
{
    RexxMethodContext *c = context;
    SYSTEMTIME sysTime = {0};
    RexxObjectPtr dateTime = NULLOBJECT;

    if ( MonthCal_GetCurSel(rxGetWindowHandle(context, self), &sysTime) == 0 )
    {
        controlFailedException(context, FUNC_WINCTRL_FAILED_MSG, "MonthCal_GetCurSel", MONTHCALENDAR_WINNAME);
    }
    else
    {
        sysTime2dt(context, &sysTime, &dateTime, dtDate);
    }
    return dateTime;
}

RexxMethod2(RexxObjectPtr, set_mc_date, RexxObjectPtr, dateTime, OSELF, self)
{
    RexxMethodContext *c = context;
    SYSTEMTIME sysTime = {0};

    if ( requiredClass(context, dateTime, "DATETIME", 1) )
    {
        if ( dt2sysTime(context, dateTime, &sysTime, dtDate) )
        {
            if ( MonthCal_SetCurSel(rxGetWindowHandle(context, self), &sysTime) == 0 )
            {
                controlFailedException(context, FUNC_WINCTRL_FAILED_MSG, "MonthCal_SetCurSel", MONTHCALENDAR_WINNAME);
            }
        }
    }
    return NULLOBJECT;
}

RexxMethod1(logical_t, get_mc_usesUnicode, OSELF, self)
{
    return MonthCal_GetUnicodeFormat(rxGetWindowHandle(context, self)) ? 1 : 0;
}

RexxMethod2(RexxObjectPtr, set_mc_usesUnicode, logical_t, useUnicode, OSELF, self)
{
    MonthCal_SetUnicodeFormat(rxGetWindowHandle(context, self), useUnicode);
    return NULLOBJECT;
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

/**
 * Creates a Windows ImageList and the corresponding ooDialog .ImageList object
 * from a single bitmap.
 *
 * The Windows ImageList supports adding any number of images from a single
 * bitmap.  The individual images are assumed to be side-by-side in the bitmap.
 * The number of images is determined by the width of a single image.
 *
 * At this time, this function is used to allow the ooDialog programmer to
 * assign an image list to a dialog control by just passing in a bitmap, rather
 * than first creating an .ImageList object.  This is much less flexible, but
 * allows the programmer to write fewer lines of code.  In addition, it mimics
 * the behavior of pre-4.0 code allowing that code to be removed.
 *
 * @param c       The method context we are operating in.
 * @param himl    [in / out] The created handle of the ImageList is returned.
 * @param ilSrc   The bitmap.
 * @param width   [optional]  The width of a single image.  When omitted, the
 *                height of the actual bitmap is used for the width.
 * @param height  [optional]  The height of a single image.  If omitted the
 *                height of the actual bitmap is used.
 * @param hwnd    The window handle of the control.  Used to create a device
 *                context if needed.
 *
 * @return An instantiated .ImageList object on success, or NULLOBJECT on
 *         failure.
 *
 * @note These objects are accepted for the image list source (ilSrc): .Image
 *       object, a bitmap file name, a bitmap handle.  A bitmap handle can be
 *       either a pointer string, or a .Pointer object.  The bitmap handle is
 *       needed to provide backward compatibility, but its use is discouraged.
 *
 * @note This function needs to support the original ooDialog design where
 *       bitmaps were loaded as DIBs.  If the image list source is a handle,
 *       GetObject() is used to test if the bitmap is a compatible bitmap (DDB.)
 *       If GetObject() returns 0, it is still a device independent (DIB) and
 *       needs to be converted to a device dependent bitmap.
 */
RexxObjectPtr rxILFromBMP(RexxMethodContext *c, HIMAGELIST *himl, RexxObjectPtr ilSrc,
                          int width, int height, HWND hwnd)
{
    HBITMAP hDDB = NULL;
    RexxObjectPtr imageList = NULLOBJECT;
    bool canRelease = false;
    BITMAP bmpInfo;

    if ( c->IsOfType(ilSrc, "Image") )
    {
        POODIMAGE oi = rxGetImageBitmap(c, ilSrc, 1);
        if ( oi == NULLOBJECT )
        {
            goto done_out;
        }
        hDDB = (HBITMAP)oi->hImage;
    }
    else if ( c->IsString(ilSrc) || c->IsPointer(ilSrc) )
    {
        CSTRING bitmap = c->ObjectToStringValue(ilSrc);

        // See if the user passed in the handle to an already loaded bitmap.
        hDDB = (HBITMAP)GET_HANDLE(bitmap);
        if ( hDDB != NULL )
        {
            if ( GetObject(hDDB, sizeof(BITMAP), &bmpInfo) == 0 )
            {
                HDC dc = GetDC(hwnd);
                hDDB = CreateDIBitmap(dc, (BITMAPINFOHEADER*)hDDB, CBM_INIT, DIB_PBITS(hDDB),
                                      DIB_PBI(hDDB), DIB_RGB_COLORS);
                if ( hDDB == NULL )
                {
                    oodSetSysErrCode(c);
                    ReleaseDC(hwnd, dc);
                    goto done_out;
                }
                ReleaseDC(hwnd, dc);
                canRelease = true;
            }
        }
        else
        {
            hDDB = (HBITMAP)LoadImage(NULL, bitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
            if ( hDDB == NULL )
            {
                oodSetSysErrCode(c);
                goto done_out;
            }
            canRelease = true;
        }
    }
    else
    {
        wrongArgValueException(c, 1, "ImageList, Image, bitmap file name, bitmap handle", ilSrc);
        goto done_out;
    }

    if ( GetObject(hDDB, sizeof(BITMAP), &bmpInfo) == sizeof(BITMAP) )
    {
        if ( width == 0 )
        {
            width = bmpInfo.bmHeight;
        }
        if ( height == 0 )
        {
            height = bmpInfo.bmHeight;
        }
        int count = bmpInfo.bmWidth / width;

        HIMAGELIST il = ImageList_Create(width, height, ILC_COLOR8, count, 0);
        if ( il != NULL )
        {
            if ( ImageList_Add(il, hDDB, NULL) == -1 )
            {
                ImageList_Destroy(il);
                goto done_out;
            }

            imageList = rxNewImageList(c, il);
            *himl = il;
        }
    }

done_out:
    if ( hDDB && canRelease )
    {
        DeleteObject(hDDB);
    }
    return imageList;
}

/** ListControl::setImageList()
 *
 *  Sets or removes one of a list-view's image lists.
 *
 *  @param ilSrc  The image list source. Either an .ImageList object that
 *                references the image list to be set, or a single bitmap from
 *                which the image list is constructed, or .nil.  If ilSRC is
 *                .nil, an existing image list, if any is removed.
 *
 *  @param width  [optional]  This arg serves two purposes.  If ilSrc is .nil or
 *                an .ImageList object, this arg indentifies which of the
 *                list-views image lists is being set, normal, small, or state.
 *                The default is LVSI_NORMAL.
 *
 *                If ilSrc is a bitmap, then this arg is the width of a single
 *                image.  The default is the height of the actual bitmap.
 *
 *  @param height [optional]  This arg is only used if ilSrc is a bitmap, in
 *                which case it is the height of the bitmap.  The default is the
 *                height of the actual bitmap
 *
 *  @param ilType [optional]  Only used if ilSrc is a bitmap.  In that case it
 *                indentifies which of the list-views image lists is being set,
 *                normal, small, or state. The default is LVSI_NORMAL.
 *
 *  @return       Returns the exsiting .ImageList object if there is one, or
 *                .nil if there is not an existing object.
 *
 *  @note  When the ilSrc is a single bitmap, an image list is created from the
 *         bitmap.  This method is not as flexible as if the programmer created
 *         the image list herself.  The bitmap must be a number of images, all
 *         the same size, side-by-side in the bitmap.  The width of a single
 *         image determines the number of images.  The image list is created
 *         using the ILC_COLOR8 flag, only.  No mask can be used.  No room is
 *         reserved for adding more images to the image list, etc..
 */
RexxMethod5(RexxObjectPtr, lv_setImageList, RexxObjectPtr, ilSrc,
            OPTIONAL_int32_t, width, OPTIONAL_int32_t, height, OPTIONAL_int32_t, ilType, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    oodResetSysErrCode(context);

    HIMAGELIST himl = NULL;
    RexxObjectPtr imageList = NULL;
    int type = LVSIL_NORMAL;

    if ( ilSrc == TheNilObj )
    {
        imageList = ilSrc;
        if ( argumentExists(2) )
        {
            type = width;
        }
    }
    else if ( context->IsOfType(ilSrc, "ImageList") )
    {
        imageList = ilSrc;
        himl = rxGetImageList(context, imageList, 1);
        if ( himl == NULL )
        {
            goto err_out;
        }

        if ( argumentExists(2) )
        {
            type = width;
        }
    }
    else
    {
        imageList = rxILFromBMP(context, &himl, ilSrc, width, height, hwnd);
        if ( imageList == NULLOBJECT )
        {
            goto err_out;
        }

        if ( argumentExists(4) )
        {
            type = ilType;
        }
    }

    if ( type > LVSIL_STATE )
    {
        wrongRangeException(context, argumentExists(4) ? 4 : 2, LVSIL_NORMAL, LVSIL_STATE, type);
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
        result = TheNilObj;
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
        return TheNilObj;
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
                result = TheNilObj;
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

// TODO review method name
RexxMethod5(int, lv_insertColumnEx, OPTIONAL_uint16_t, column, CSTRING, text, uint16_t, width,
            OPTIONAL_CSTRING, fmt, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    LVCOLUMN lvi = {0};
    int retVal = 0;
    char szText[256];

    lvi.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT | LVCF_WIDTH;

    // If omitted, column is 0, which is also the default.
    lvi.iSubItem = column;

    lvi.cchTextMax = (int)strlen(text);
    if ( lvi.cchTextMax > (sizeof(szText) - 1) )
    {
        userDefinedMsgException(context, 2, "the column title must be less than 256 characters");
        return 0;
    }
    strcpy(szText, text);
    lvi.pszText = szText;
    lvi.cx = width;

    lvi.fmt = LVCFMT_LEFT;
    if ( argumentExists(4) )
    {
        char f = toupper(*fmt);
        if ( f == 'C' )
        {
            lvi.fmt = LVCFMT_CENTER;
        }
        else if ( f == 'R' )
        {
            lvi.fmt = LVCFMT_RIGHT;
        }
    }

    retVal = ListView_InsertColumn(hwnd, lvi.iSubItem, &lvi);
    if ( retVal != -1 && lvi.fmt != LVCFMT_LEFT && lvi.iSubItem == 0 )
    {
        /* According to the MSDN docs: "If a column is added to a
         * list-view control with index 0 (the leftmost column) and with
         * LVCFMT_RIGHT or LVCFMT_CENTER specified, the text is not
         * right-aligned or centered." This is the suggested work around.
         */
        lvi.iSubItem = 1;
        ListView_InsertColumn(hwnd, lvi.iSubItem, &lvi);
        ListView_DeleteColumn(hwnd, 0);
    }
    return retVal;
}

// TODO review method name
RexxMethod2(int, lv_columnWidthEx, uint16_t, column, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    return ListView_GetColumnWidth(hwnd, column);
}

// TODO review method name
RexxMethod2(int, lv_stringWidthEx, CSTRING, text, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    return ListView_GetStringWidth(hwnd, text);
}

// TODO review method name
RexxMethod5(int, lv_addRowEx, CSTRING, text, OPTIONAL_int, itemIndex, OPTIONAL_int, imageIndex,
            OPTIONAL_RexxObjectPtr, subItems, OSELF, self)
{
    //RexxMethodContext *context;
    HWND hwnd = rxGetWindowHandle(context, self);

    if ( argumentOmitted(2) )
    {
        RexxObjectPtr last = context->SendMessage0(self, "LASTITEM");
        if ( last != NULLOBJECT )
        {
            context->Int32(last, &itemIndex);
            itemIndex++;
        }
        else
        {
            itemIndex = 0;
        }
    }

    if ( argumentOmitted(3) )
    {
        imageIndex = -1;
    }

    LV_ITEM lvi;
    lvi.mask = LVIF_TEXT;

    lvi.iItem = itemIndex;
    lvi.iSubItem = 0;
    lvi.pszText = (LPSTR)text;

    if ( imageIndex > -1 )
    {
        lvi.iImage = imageIndex;
        lvi.mask |= LVIF_IMAGE;
    }

    itemIndex = ListView_InsertItem(hwnd, &lvi);

    if ( itemIndex == -1 )
    {
        goto done_out;
    }
    context->SendMessage1(self, "LASTITEM=", context->Int32(itemIndex));

    if ( argumentOmitted(4) )
    {
        goto done_out;
    }
    if ( ! context->IsArray(subItems) )
    {
        wrongClassException(context, 4, "Array");
        goto done_out;
    }

    size_t count = context->ArrayItems((RexxArrayObject)subItems);
    for ( size_t i = 1; i <= count; i++)
    {
        RexxDirectoryObject subItem = (RexxDirectoryObject)context->ArrayAt((RexxArrayObject)subItems, i);
        if ( subItem == NULLOBJECT || ! context->IsDirectory(subItem) )
        {
            wrongObjInArrayException(context, 4, i, "Directory");
            goto done_out;
        }

        RexxObjectPtr subItemText = context->DirectoryAt(subItem, "TEXT");
        if ( subItemText == NULLOBJECT )
        {
            missingIndexInDirectoryException(context, 4, "TEXT");
            goto done_out;
        }
        imageIndex = -1;
        if ( ! rxIntFromDirectory(context, subItem, "ICON", &imageIndex, 4) )
        {
            goto done_out;
        }

        lvi.mask = LVIF_TEXT;
        lvi.iSubItem = (int)i;
        lvi.pszText = (LPSTR)context->ObjectToStringValue(subItemText);

        if ( imageIndex > -1 )
        {
            lvi.iImage = imageIndex;
            lvi.mask |= LVIF_IMAGE;
        }

        ListView_SetItem(hwnd, &lvi);
    }

done_out:
    return itemIndex;
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
 *  @param ilSrc  The image list source. Either an .ImageList object that
 *                references the image list to be set, or a single bitmap from
 *                which the image list is constructed, or .nil.  If ilSRC is
 *                .nil, an existing image list, if any is removed.
 *
 *  @param width  [optional]  This arg serves two purposes.  If ilSrc is .nil or
 *                an .ImageList object, this arg indentifies which of the
 *                tree-views image lists is being set, normal, or state. The
 *                default is TVSI_NORMAL.
 *
 *                If ilSrc is a bitmap, then this arg is the width of a single
 *                image.  The default is the height of the actual bitmap.
 *
 *  @param height [optional]  This arg is only used if ilSrc is a bitmap, in which case it
 *                is the height of the bitmap.  The default is the height of the
 *                actual bitmap
 *
 *  @return       Returns the exsiting .ImageList object if there is one, or
 *                .nil if there is not an existing object.
 *
 *  @note  When the ilSrc is a single bitmap, an image list is created from the
 *         bitmap.  This method is not as flexible as if the programmer created
 *         the image list herself.  The bitmap must be a number of images, all
 *         the same size, side-by-side in the bitmap.  The width of a single
 *         image determines the number of images.  The image list is created
 *         using the ILC_COLOR8 flag, only.  No mask can be used.  No room is
 *         reserved for adding more images to the image list, etc..
 *
 *         The image list can only be assigned to the normal image list.  There
 *         is no way to use the image list for the state image list.
 */
RexxMethod4(RexxObjectPtr, tv_setImageList, RexxObjectPtr, ilSrc,
            OPTIONAL_int32_t, width, OPTIONAL_int32_t, height, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    oodResetSysErrCode(context);

    HIMAGELIST himl = NULL;
    int type = TVSIL_NORMAL;
    RexxObjectPtr imageList = NULLOBJECT;

    if ( ilSrc == TheNilObj )
    {
        imageList = ilSrc;
        if ( argumentExists(2) )
        {
            type = width;
        }
    }
    else if ( context->IsOfType(ilSrc, "ImageList") )
    {
        imageList = ilSrc;
        himl = rxGetImageList(context, imageList, 1);
        if ( himl == NULL )
        {
            goto err_out;
        }
        if ( argumentExists(2) )
        {
            type = width;
        }
    }
    else
    {
        imageList = rxILFromBMP(context, &himl, ilSrc, width, height, hwnd);
        if ( imageList == NULLOBJECT )
        {
            goto err_out;
        }
    }

    if ( type != TVSIL_STATE && type != TVSIL_NORMAL )
    {
        invalidTypeException(context, 2, " TVSIL_XXX flag");
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
        invalidTypeException(context, 2, " TVSIL_XXX flag");
        return NULLOBJECT;
    }

    RexxObjectPtr result = context->GetObjectVariable(tvGetAttributeName(type));
    if ( result == NULLOBJECT )
    {
        result = TheNilObj;
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
 *  @param ilSrc  The image list source. Either an .ImageList object that
 *                references the image list to be set, or a single bitmap from
 *                which the image list is constructed, or .nil.  If ilSRC is
 *                .nil, an existing image list, if any is removed.
 *
 *  @param width  [optional]  This arg is only used if ilSrc is a single bitmap.
 *                Then this arg is the width of a single image.  The default is
 *                the height of the actual bitmap.
 *
 *  @param height [optional]  This arg is only used if ilSrc is a bitmap, in
 *                which case it is the height of the bitmap.  The default is the
 *                height of the actual bitmap
 *
 *  @return       Returns the exsiting .ImageList object if there is one, or
 *                .nil if there is not an existing object.
 *
 *  @note  When the ilSrc is a single bitmap, an image list is created from the
 *         bitmap.  This method is not as flexible as if the programmer created
 *         the image list herself.  The bitmap must be a number of images, all
 *         the same size, side-by-side in the bitmap.  The width of a single
 *         image determines the number of images.  The image list is created
 *         using the ILC_COLOR8 flag, only.  No mask can be used.  No room is
 *         reserved for adding more images to the image list, etc..
 */
RexxMethod4(RexxObjectPtr, tab_setImageList, RexxObjectPtr, ilSrc,
            OPTIONAL_int32_t, width, OPTIONAL_int32_t, height, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);
    oodResetSysErrCode(context);

    HIMAGELIST himl = NULL;
    RexxObjectPtr imageList = NULLOBJECT;

    if ( ilSrc == TheNilObj )
    {
        imageList = ilSrc;
    }
    else if ( context->IsOfType(ilSrc, "ImageList") )
    {
        imageList = ilSrc;
        himl = rxGetImageList(context, imageList, 1);
        if ( himl == NULL )
        {
            goto err_out;
        }
    }
    else
    {
        imageList = rxILFromBMP(context, &himl, ilSrc, width, height, hwnd);
        if ( imageList == NULLOBJECT )
        {
            goto err_out;
        }
    }

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
    return (result == NULLOBJECT) ? TheNilObj : result;
}


/**
 *  Methods for the .StaticControl.
 */
#define STATIC_CLASS              "StaticControl"
#define STATICIMAGE_ATTRIBUTE     "!STATICIMAGE"

/** StaticControl::setIcon()
 *
 *  Sets or removes the icon image for this static control.
 *
 *  @param  icon  The new icon image for the the static control, or .nil to
 *                remove the existing icon.
 *
 *  @return  The existing icon, or .nil if there is no existing icon.
 */
RexxMethod2(RexxObjectPtr, stc_setIcon, RexxObjectPtr, icon, CSELF, pCSelf)
{
    RexxObjectPtr result = NULLOBJECT;

    HANDLE hNewIcon = NULL;
    if ( icon != TheNilObj )
    {
        POODIMAGE oi = rxGetImageIcon(context, icon, 1);
        if ( oi == NULL )
        {
            goto out;
        }
        hNewIcon = oi->hImage;
    }

    HWND hwnd = ((pCDialogControl)pCSelf)->hCtrl;
    HICON hIcon = (HICON)SendMessage(hwnd, STM_SETICON, (WPARAM)hNewIcon, 0);

    result = oodSetImageAttribute(context, STATICIMAGE_ATTRIBUTE, icon, hwnd, hIcon, IMAGE_ICON, winStatic);
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
    return oodGetImageAttribute(context, self, STATICIMAGE_ATTRIBUTE, STM_GETICON, 0, IMAGE_ICON, winStatic);
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

    if ( rxNewImage != TheNilObj )
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

    result = oodSetImageAttribute(context, STATICIMAGE_ATTRIBUTE, rxNewImage, hwnd, oldHandle, -1, winStatic);
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
    return oodGetImageAttribute(context, self, STATICIMAGE_ATTRIBUTE, STM_GETIMAGE, type, -1, winStatic);
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
    TCHAR buf[64];
    BUTTONTYPE type = notButton;

    if ( ! RealGetWindowClass(hwnd, buf, sizeof(buf)) || strcmp(buf, WC_BUTTON) )
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
    RexxObjectPtr result = TheNilObj;

    RexxObjectPtr imageList = c->GetObjectVariable(BUTTONIMAGELIST_ATTRIBUTE);
    if ( imageList != NULLOBJECT && imageList != TheNilObj )
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

    if ( result != TheNilObj )
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
RexxMethod2(int, gb_setStyle, CSTRING, opts, CSELF, pCSelf)
{
    HWND hwnd = ((pCDialogControl)pCSelf)->hCtrl;
    HWND hDlg = ((pCDialogControl)pCSelf)->hDlg;

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

RexxMethod2(RexxObjectPtr, bc_setState, CSTRING, opts, CSELF, pCSelf)
{
    HWND hwnd = ((pCDialogControl)pCSelf)->hCtrl;
    HWND hDlg = ((pCDialogControl)pCSelf)->hDlg;

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
            SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hwnd, TRUE);
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
    return (result == NULL) ? TheNilObj : result;
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
    return (result == NULLOBJECT) ? TheNilObj : result;
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

    return oodGetImageAttribute(context, self, BUTTONIMAGE_ATTRIBUTE, BM_GETIMAGE, wParam, type, winButton);
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

    if ( rxNewImage != TheNilObj )
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

    result = oodSetImageAttribute(context, BUTTONIMAGE_ATTRIBUTE, rxNewImage, hwnd, oldHandle, -1, winButton);

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
    oodResetSysErrCode(context);
    RexxObjectPtr result = NULLOBJECT;

    if ( ! requiredComCtl32Version(context, "setImageList", COMCTL32_6_0) )
    {
        goto err_out;
    }

    if ( imageList == TheNilObj )
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

        int first = oodResolveSymbolicID(context, dlg, idFirst, -1, 2);
        int last = oodResolveSymbolicID(context, dlg, idLast, -1, 3);
        int check = oodResolveSymbolicID(context, dlg, idCheck, -1, 4);

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

RexxMethod1(logical_t, rb_checked, CSELF, pCSelf)
{
    return (SendMessage(((pCDialogControl)pCSelf)->hCtrl, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
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
        invalidTypeException(context, 1, " ImageList handle");
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
    RexxObjectPtr result = TheNilObj;

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
            return TheNilObj;
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
            case winStatic :
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

            case winButton :
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
    oodResetSysErrCode(context);

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

RexxMethod1(uint32_t, image_toID_cls, CSTRING, symbol)
{
    static String2Int *imageConstantsMap = NULL;

    if ( imageConstantsMap == NULL )
    {
        imageConstantsMap = imageInitMap();
    }
    int idValue = getConstantValue(imageConstantsMap, symbol);
    if ( idValue == -1 )
    {
        wrongArgValueException(context, 1, "the Image class symbol IDs", symbol);
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
    oodResetSysErrCode(context);

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


