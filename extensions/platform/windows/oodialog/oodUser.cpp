/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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

/**
 * oodUser.cpp
 *
 * Implements the function used to build and execute dialogs dynamically in
 * memory.  Historically this has been referred to as "User" functionality for
 * "User defined dialogs."
 */

#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <commctrl.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodData.hpp"
#include "oodSymbols.h"

//#define USE_DS_CONTROL

#ifndef USE_DS_CONTROL
BOOL IsNestedDialogMessage(
    DIALOGADMIN * dlgAdm,
    LPMSG  lpmsg    // address of structure with message
   );
#endif


class LoopThreadArgs
{
public:
    DLGTEMPLATE *dlgTemplate;
    DIALOGADMIN *dlgAdmin;
    bool        *release;
};

/****************************************************************************************************

           Part for user defined Dialogs

****************************************************************************************************/


LPWORD lpwAlign (LPWORD lpIn)
{
  ULONG_PTR ul;

  ul = (ULONG_PTR)lpIn;
  ul +=3;
  ul >>=2;
  ul <<=2;
  return (LPWORD)ul;
}


int nCopyAnsiToWideChar (LPWORD lpWCStr, const char *lpAnsiIn)
{
  int nChar = 0;

  do {
    *lpWCStr++ = (WORD) (UCHAR) *lpAnsiIn;  /* first convert to UCHAR, otherwise Ü,Ä,... are >65000 */
    nChar++;
  } while (*lpAnsiIn++);

  return nChar;
}


void UCreateDlg(WORD ** ppTemplate, WORD **p, INT NrItems, INT x, INT y, INT cx, INT cy,
                const char * dlgClass, const char * title, const char * fontname, INT fontsize, ULONG lStyle)
{
    int   nchar;

    *ppTemplate = *p = (PWORD) LocalAlloc(LPTR, (NrItems+3)*256);

    /* start to fill in the dlgtemplate information.  addressing by WORDs */
    **p = LOWORD (lStyle);
    (*p)++;
    **p = HIWORD (lStyle);
    (*p)++;
    **p = 0;          // LOWORD (lExtendedStyle)
    (*p)++;
    **p = 0;          // HIWORD (lExtendedStyle)
    (*p)++;
    **p = NrItems;    // NumberOfItems
    (*p)++;
    **p = x;          // x
    (*p)++;
    **p = y;          // y
    (*p)++;
    **p = cx;         // cx
    (*p)++;
    **p = cy;         // cy
    (*p)++;
    /* copy the menu of the dialog */

    /* no menu */
    **p = 0;
    (*p)++;

    /* copy the class of the dialog */
    if ( !(lStyle & WS_CHILD) && (dlgClass) )
    {
        nchar = nCopyAnsiToWideChar (*p, TEXT(dlgClass));
        (*p) += nchar;
    }
    else
    {
        **p = 0;
        (*p)++;
    }
    /* copy the title of the dialog */
    if ( title )
    {
        nchar = nCopyAnsiToWideChar (*p, TEXT(title));
        (*p) += nchar;
    }
    else
    {
        **p = 0;
        (*p)++;
    }

    /* add in the wPointSize and szFontName here iff the DS_SETFONT bit on */
    **p = fontsize;   // fontsize
    (*p)++;
    nchar = nCopyAnsiToWideChar (*p, TEXT(fontname));
    (*p) += nchar;

    /* make sure the first item starts on a DWORD boundary */
    (*p) = lpwAlign (*p);
}



size_t RexxEntry UsrDefineDialog(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{

   INT buffer[5];
   const char *opts;

   BOOL child;
   ULONG lStyle;
   WORD *p;
   WORD *pbase;
   int i;

   CHECKARG(10);

   for (i=0;i<4;i++) buffer[i] = atoi(argv[i].strptr);
   opts = argv[8].strptr;

   if (strstr(opts, "CHILD")) child = TRUE; else child = FALSE;
   buffer[4] = atoi(argv[9].strptr);

   if (child) lStyle = DS_SETFONT | WS_CHILD;
   else lStyle = WS_CAPTION | DS_SETFONT;

#ifdef USE_DS_CONTROL
   if (child) lStyle |= DS_CONTROL;
#endif

   if (strstr(opts, "VISIBLE")) lStyle |= WS_VISIBLE;
   if (!strstr(opts, "NOMENU") && !child) lStyle |= WS_SYSMENU;
   if (!strstr(opts, "NOTMODAL") && !child) lStyle |= DS_MODALFRAME;
   if (strstr(opts, "SYSTEMMODAL")) lStyle |= DS_SYSMODAL;
   if (strstr(opts, "CENTER")) lStyle |= DS_CENTER;
   if (strstr(opts, "THICKFRAME")) lStyle |= WS_THICKFRAME;
   if (strstr(opts, "MINIMIZEBOX")) lStyle |= WS_MINIMIZEBOX;
   if (strstr(opts, "MAXIMIZEBOX")) lStyle |= WS_MAXIMIZEBOX;
   if (strstr(opts, "VSCROLL")) lStyle |= WS_VSCROLL;
   if (strstr(opts, "HSCROLL")) lStyle |= WS_HSCROLL;

   if (strstr(opts, "OVERLAPPED")) lStyle |= WS_OVERLAPPED;

   /*                     expected        x          y        cx        cy  */
   UCreateDlg(&pbase, &p, buffer[4], buffer[0], buffer[1], buffer[2], buffer[3],
   /*            class         title            fontname         fontsize */
              argv[4].strptr, argv[5].strptr, argv[6].strptr, atoi(argv[7].strptr), lStyle);
   sprintf(retstr->strptr, "0x%p 0x%p", pbase, p);
   retstr->strlength = strlen(retstr->strptr);
   return 0;
}

/**
 * Windows message loop for a UserDialog or UserDialog subclass.
 *
 * @param args
 *
 * @return DWORD WINAPI
 */
DWORD WINAPI WindowUsrLoopThread(LoopThreadArgs * args)
{
    MSG msg;
    bool *release = args->release;
    DIALOGADMIN *dlgAdm = args->dlgAdmin;

    dlgAdm->TheDlg = CreateDialogIndirectParam(MyInstance, args->dlgTemplate, NULL, (DLGPROC)RexxDlgProc, dlgAdm->Use3DControls);  /* pass 3D flag to WM_INITDIALOG */
    dlgAdm->ChildDlg[0] = dlgAdm->TheDlg;

    if ( dlgAdm->TheDlg )
    {
        *release = true;

        while ( GetMessage(&msg,NULL, 0,0) && dialogInAdminTable(dlgAdm) && (!dlgAdm->LeaveDialog) )
        {
#ifdef USE_DS_CONTROL
            if ( dlgAdm && !IsDialogMessage(dlgAdm->TheDlg, &msg)
                 && !IsDialogMessage(dlgAdm->AktChild, &msg) )
#else
            if ( dlgAdm && (!IsNestedDialogMessage(dlgAdm, &msg)) )
#endif
                DispatchMessage(&msg);
        }
    }
    else
    {
        *release = true;
    }

    // Need to synchronize here, otherwise dlgAdm might still be in the table
    // but DelDialog is already running.
    EnterCriticalSection(&crit_sec);
    if ( dialogInAdminTable(dlgAdm) )
    {
        DelDialog(dlgAdm);
        dlgAdm->TheThread = NULL;
    }
    LeaveCriticalSection(&crit_sec);
    return 0;
}

static inline logical_t illegalBuffer(void)
{
    MessageBox(0, "Illegal resource buffer", "Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
    return FALSE;
}


void UAddControl(WORD **p, SHORT kind, INT id, INT x, INT y, INT cx, INT cy, const char * txt, ULONG lStyle)
{
   int   nchar;

   **p = LOWORD (lStyle);
   (*p)++;
   **p = HIWORD (lStyle);
   (*p)++;
   **p = 0;          // LOWORD (lExtendedStyle)
   (*p)++;
   **p = 0;          // HIWORD (lExtendedStyle)
   (*p)++;
   **p = x;         // x
   (*p)++;
   **p = y;         // y
   (*p)++;
   **p = cx;         // cx
   (*p)++;
   **p = cy;         // cy
   (*p)++;
   **p = id;         // ID
   (*p)++;

   **p = (WORD)0xffff;
   (*p)++;
   **p = (WORD)kind;
   (*p)++;

   if (txt)
   {
      nchar = nCopyAnsiToWideChar (*p, TEXT(txt));
      (*p) += nchar;
   }
   else
   {
     **p = 0;
    (*p)++;
   }

   **p = 0;  // advance pointer over nExtraStuff WORD
   (*p)++;

   /* make sure the next item starts on a DWORD boundary */
   (*p) = lpwAlign (*p);
}


void UAddNamedControl(WORD **p, CHAR * className, INT id, INT x, INT y, INT cx, INT cy, CHAR * txt, ULONG lStyle)
{
   int   nchar;

   **p = LOWORD (lStyle);
   (*p)++;
   **p = HIWORD (lStyle);
   (*p)++;
   **p = 0;          // LOWORD (lExtendedStyle)
   (*p)++;
   **p = 0;          // HIWORD (lExtendedStyle)
   (*p)++;
   **p = x;         // x
   (*p)++;
   **p = y;         // y
   (*p)++;
   **p = cx;         // cx
   (*p)++;
   **p = cy;         // cy
   (*p)++;
   **p = id;         // ID
   (*p)++;

   nchar = nCopyAnsiToWideChar (*p, TEXT(className));
   (*p) += nchar;

   if (txt)
   {
      nchar = nCopyAnsiToWideChar (*p, TEXT(txt));
      (*p) += nchar;
   }
   else
   {
     **p = 0;
    (*p)++;
   }

   **p = 0;  // advance pointer over nExtraStuff WORD
   (*p)++;

   /* make sure the next item starts on a DWORD boundary */
   (*p) = lpwAlign (*p);
}


size_t RexxEntry UsrAddControl(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   INT buffer[5];
   ULONG lStyle;
   WORD *p = NULL;
   int i;

   CHECKARGL(1);

   if (!strcmp(argv[0].strptr,"BUT") || !strcmp(argv[0].strptr,"CH") || !strcmp(argv[0].strptr,"RB"))
   {
       CHECKARG(9);

       /* UsrAddControl("BUT", self~activePtr, id, x, y, w, h, name, opts) */
       for ( i = 0; i < 5; i++ )
       {
           buffer[i] = atoi(argv[i+2].strptr);
       }

       p = (WORD *)GET_POINTER(argv[1]);

       lStyle = WS_CHILD;
       if (strstr(argv[8].strptr,"3STATE")) lStyle |= BS_AUTO3STATE; else
       if (!strcmp(argv[0].strptr,"CH")) lStyle |= BS_AUTOCHECKBOX; else
       if (!strcmp(argv[0].strptr,"RB")) lStyle |= BS_AUTORADIOBUTTON; else
       if (strstr(argv[8].strptr,"DEFAULT")) lStyle |= BS_DEFPUSHBUTTON; else lStyle |= BS_PUSHBUTTON;

       if (strstr(argv[8].strptr,"OWNER")) lStyle |= BS_OWNERDRAW;
       if (strstr(argv[8].strptr,"BITMAP")) lStyle |= BS_BITMAP;
       if (strstr(argv[8].strptr,"ICON")) lStyle |= BS_ICON;
       if (strstr(argv[8].strptr,"HCENTER")) lStyle |= BS_CENTER;
       if (strstr(argv[8].strptr,"TOP")) lStyle |= BS_TOP;
       if (strstr(argv[8].strptr,"BOTTOM")) lStyle |= BS_BOTTOM;
       if (strstr(argv[8].strptr,"VCENTER")) lStyle |= BS_VCENTER;
       if (strstr(argv[8].strptr,"PUSHLIKE")) lStyle |= BS_PUSHLIKE;
       if (strstr(argv[8].strptr,"MULTILINE")) lStyle |= BS_MULTILINE;
       if (strstr(argv[8].strptr,"NOTIFY")) lStyle |= BS_NOTIFY;
       if (strstr(argv[8].strptr,"FLAT")) lStyle |= BS_FLAT;

       const char *pLonger = strstr(argv[8].strptr,"LEFTTEXT");
       const char *pShorter = strstr(argv[8].strptr,"LEFT");
       if ( pLonger )
       {
           lStyle |= BS_LEFTTEXT;
           if ( pShorter != NULL && pShorter != pLonger )
           {
               lStyle |= BS_LEFT;
           }
       }
       else if ( pShorter )
       {
           lStyle |= BS_LEFT;
       }

       pLonger = strstr(argv[8].strptr,"RIGHTBUTTON");
       pShorter = strstr(argv[8].strptr,"RIGHT");
       if ( pLonger )
       {
           lStyle |= BS_RIGHTBUTTON;
           if ( pShorter != NULL && pShorter != pLonger )
           {
               lStyle |= BS_RIGHT;
           }
       }
       else if ( pShorter )
       {
           lStyle |= BS_RIGHT;
       }

       if (!strstr(argv[8].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[8].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[8].strptr,"DISABLED")) lStyle |= WS_DISABLED;
       if (strstr(argv[8].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (!strstr(argv[8].strptr,"NOTAB")) lStyle |= WS_TABSTOP;

       /*                       id         x           y         cx          cy  */
       UAddControl(&p, 0x0080, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], argv[7].strptr, lStyle);
   }
   else if (!strcmp(argv[0].strptr,"GB"))  // A groupbox is actually a button.
   {
       CHECKARGL(8);

       /* UsrAddControl("GB",self~activePtr, x, y, cx, cy, opts, text, id) */
       for ( i = 0; i < 4; i++ )
       {
           buffer[i] = atoi(argv[i+2].strptr);
       }

       if (argc > 8)
          i = atoi(argv[8].strptr);
       else i = -1;

       p = (WORD *)GET_POINTER(argv[1]);

       // We support right or left aligned text.  By default the alignment is
       // left so we only need to check for the RIGHT key word.

       lStyle = WS_CHILD | BS_GROUPBOX;
       if (strstr(argv[6].strptr,"RIGHT")) lStyle |= BS_RIGHT;
       if (!strstr(argv[6].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[6].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[6].strptr,"DISABLED")) lStyle |= WS_DISABLED;
       if (strstr(argv[6].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (strstr(argv[6].strptr,"TAB")) lStyle |= WS_TABSTOP;

       /*                      id      x         y        cx        cy  */
       UAddControl(&p, 0x0080, i, buffer[0], buffer[1], buffer[2], buffer[3], argv[7].strptr, lStyle);
   }
   else if (!strcmp(argv[0].strptr,"EL"))
   {
       CHECKARG(8);

       /* UsrAddControl("EL", self~activePtr, id, x, y, cx, cy, opts) */
       for ( i = 0; i < 5; i++ )
       {
           buffer[i] = atoi(argv[i+2].strptr);
       }

       p = (WORD *)GET_POINTER(argv[1]);

       lStyle = WS_CHILD;
       if (strstr(argv[7].strptr,"PASSWORD")) lStyle |= ES_PASSWORD;
       if (strstr(argv[7].strptr,"MULTILINE"))
       {
           lStyle |= ES_MULTILINE;
           if (!strstr(argv[7].strptr,"NOWANTRETURN")) lStyle |= ES_WANTRETURN;
           if (!strstr(argv[7].strptr,"HIDESELECTION")) lStyle |= ES_NOHIDESEL;
       }
       if (strstr(argv[7].strptr,"AUTOSCROLLH")) lStyle |= ES_AUTOHSCROLL;
       if (strstr(argv[7].strptr,"AUTOSCROLLV")) lStyle |= ES_AUTOVSCROLL;
       if (strstr(argv[7].strptr,"HSCROLL")) lStyle |= WS_HSCROLL;
       if (strstr(argv[7].strptr,"VSCROLL")) lStyle |= WS_VSCROLL;
       if (strstr(argv[7].strptr,"READONLY")) lStyle |= ES_READONLY;
       if (strstr(argv[7].strptr,"KEEPSELECTION")) lStyle |= ES_NOHIDESEL;
       if (strstr(argv[7].strptr,"CENTER")) lStyle |= ES_CENTER;
       else if (strstr(argv[7].strptr,"RIGHT")) lStyle |= ES_RIGHT;
       else lStyle |= ES_LEFT;
       if (strstr(argv[7].strptr,"UPPER")) lStyle |= ES_UPPERCASE;
       if (strstr(argv[7].strptr,"LOWER")) lStyle |= ES_LOWERCASE;
       if (strstr(argv[7].strptr,"NUMBER")) lStyle |= ES_NUMBER;
       if (strstr(argv[7].strptr,"OEM")) lStyle |= ES_OEMCONVERT;
       if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;
       if (!strstr(argv[7].strptr,"NOBORDER")) lStyle |= WS_BORDER;
       if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;

       /*                         id          x       y          cx           cy  */
       UAddControl(&p, 0x0081, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
   }
   else if (!strcmp(argv[0].strptr,"LB"))
   {
       CHECKARG(8);

       for ( i = 0; i < 5; i++ )
       {
           buffer[i] = atoi(argv[i+2].strptr);
       }

       p = (WORD *)GET_POINTER(argv[1]);

       lStyle = WS_CHILD;
       if (strstr(argv[7].strptr,"COLUMNS")) lStyle |= LBS_USETABSTOPS;
       if (strstr(argv[7].strptr,"VSCROLL")) lStyle |= WS_VSCROLL;
       if (strstr(argv[7].strptr,"HSCROLL")) lStyle |= WS_HSCROLL;
       if (strstr(argv[7].strptr,"SORT")) lStyle |= LBS_STANDARD;
       if (strstr(argv[7].strptr,"NOTIFY")) lStyle |= LBS_NOTIFY;
       if (strstr(argv[7].strptr,"MULTI")) lStyle |= LBS_MULTIPLESEL;
       if (strstr(argv[7].strptr,"MCOLUMN")) lStyle |= LBS_MULTICOLUMN;
       if (strstr(argv[7].strptr,"PARTIAL")) lStyle |= LBS_NOINTEGRALHEIGHT;
       if (strstr(argv[7].strptr,"SBALWAYS")) lStyle |= LBS_DISABLENOSCROLL;
       if (strstr(argv[7].strptr,"KEYINPUT")) lStyle |= LBS_WANTKEYBOARDINPUT;
       if (strstr(argv[7].strptr,"EXTSEL")) lStyle |= LBS_EXTENDEDSEL;
       if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;
       if (!strstr(argv[7].strptr,"NOBORDER")) lStyle |= WS_BORDER;
       if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;

       /*                         id       x          y            cx        cy  */
       UAddControl(&p, 0x0083, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
   }
   else if (!strcmp(argv[0].strptr,"CB"))
   {
       CHECKARG(8);

       for ( i = 0; i < 5; i++)
       {
         buffer[i] = atoi(argv[i+2].strptr);
       }

       p = (WORD *)GET_POINTER(argv[1]);

       lStyle = WS_CHILD;

       if (!strstr(argv[7].strptr,"NOHSCROLL")) lStyle |= CBS_AUTOHSCROLL;
       if (strstr(argv[7].strptr,"VSCROLL")) lStyle |= WS_VSCROLL;
       if (strstr(argv[7].strptr,"SORT")) lStyle |= CBS_SORT;
       if (strstr(argv[7].strptr,"SIMPLE")) lStyle |= CBS_SIMPLE;
       else if (strstr(argv[7].strptr,"LIST")) lStyle |= CBS_DROPDOWNLIST;
       else lStyle |= CBS_DROPDOWN;
       if (strstr(argv[7].strptr,"PARTIAL")) lStyle |= CBS_NOINTEGRALHEIGHT;
       if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;
       if (!strstr(argv[7].strptr,"NOBORDER")) lStyle |= WS_BORDER;
       if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;

       /*                         id       x          y            cx        cy  */
       UAddControl(&p, 0x0085, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
   }
   else if (!strcmp(argv[0].strptr,"TXT"))
   {
       CHECKARGL(8);

       /* UsrAddControl("TXT", self~activePtr, x, y, cx, cy, opts, text, id) */
       for ( i = 0; i < 4; i++ )
       {
           buffer[i] = atoi(argv[i+2].strptr);
       }

       p = (WORD *)GET_POINTER(argv[1]);

       if (argc > 8)
          i = atoi(argv[8].strptr);
       else i = -1;

       lStyle = WS_CHILD;
       if (strstr(argv[6].strptr,"CENTER")) lStyle |= SS_CENTER;
       else if (strstr(argv[6].strptr,"RIGHT")) lStyle |= SS_RIGHT;
       else if (strstr(argv[6].strptr,"SIMPLE")) lStyle |= SS_SIMPLE;
       else if (strstr(argv[6].strptr,"LEFTNOWRAP")) lStyle |= SS_LEFTNOWORDWRAP;
       else lStyle |= SS_LEFT;

       // Used to center text vertically.
       if (strstr(argv[6].strptr,"CENTERIMAGE")) lStyle |= SS_CENTERIMAGE;

       if (strstr(argv[6].strptr,"NOTIFY")) lStyle |= SS_NOTIFY;
       if (strstr(argv[6].strptr,"SUNKEN")) lStyle |= SS_SUNKEN;
       if (strstr(argv[6].strptr,"EDITCONTROL")) lStyle |= SS_EDITCONTROL;
       if (strstr(argv[6].strptr,"ENDELLIPSIS")) lStyle |= SS_ENDELLIPSIS;
       if (strstr(argv[6].strptr,"NOPREFIX")) lStyle |= SS_NOPREFIX;
       if (strstr(argv[6].strptr,"PATHELLIPSIS")) lStyle |= SS_PATHELLIPSIS;
       if (strstr(argv[6].strptr,"WORDELLIPSIS")) lStyle |= SS_WORDELLIPSIS;

       if (!strstr(argv[6].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[6].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[6].strptr,"DISABLED")) lStyle |= WS_DISABLED;
       if (strstr(argv[6].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (strstr(argv[6].strptr,"TAB")) lStyle |= WS_TABSTOP;

       /*                      id      x         y         cx       cy  */
       UAddControl(&p, 0x0082, i, buffer[0], buffer[1], buffer[2], buffer[3], argv[7].strptr, lStyle);
   }
   else if (!strcmp(argv[0].strptr,"FRM"))
   {
       CHECKARGL(8);

       for ( i = 0; i < 5; i++ )
       {
           buffer[i] = atoi(argv[i+2].strptr);
       }

       p = (WORD *)GET_POINTER(argv[1]);

       if (argc > 8)
          i = atoi(argv[8].strptr);
       else i = -1;

       lStyle = WS_CHILD;

       if (buffer[4] == 0) lStyle |= SS_WHITERECT; else
       if (buffer[4] == 1) lStyle |= SS_GRAYRECT; else
       if (buffer[4] == 2) lStyle |= SS_BLACKRECT; else
       if (buffer[4] == 3) lStyle |= SS_WHITEFRAME; else
       if (buffer[4] == 4) lStyle |= SS_GRAYFRAME; else
       if (buffer[4] == 5) lStyle  |= SS_BLACKFRAME ; else
       if (buffer[4] == 6) lStyle  |= SS_ETCHEDFRAME ; else
       if (buffer[4] == 7) lStyle  |= SS_ETCHEDHORZ ; else
       lStyle |= SS_ETCHEDVERT;

       if (strstr(argv[7].strptr,"NOTIFY")) lStyle |= SS_NOTIFY;
       if (strstr(argv[7].strptr,"SUNKEN")) lStyle |= SS_SUNKEN;

       if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;
       if (strstr(argv[7].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (strstr(argv[7].strptr,"TAB")) lStyle |= WS_TABSTOP;

       /*                     id    x           y          cx         cy  */
       UAddControl(&p, 0x0082, i, buffer[0], buffer[1], buffer[2], buffer[3], NULL, lStyle);
   }
   else if (!strcmp(argv[0].strptr,"IMG"))
   {
       CHECKARGL(8);

       for ( i = 0; i < 5; i++ )
       {
           buffer[i] = atoi(argv[i+2].strptr);
       }

       p = (WORD *)GET_POINTER(argv[1]);

       lStyle = WS_CHILD;
       if (strstr(argv[7].strptr,"METAFILE")) lStyle |= SS_ENHMETAFILE;
       else if (strstr(argv[7].strptr,"BITMAP")) lStyle |= SS_BITMAP;
       else lStyle |= SS_ICON;

       if (strstr(argv[7].strptr,"NOTIFY")) lStyle |= SS_NOTIFY;
       if (strstr(argv[7].strptr,"CENTERIMAGE")) lStyle |= SS_CENTERIMAGE;
       if (strstr(argv[7].strptr,"RIGHTJUST")) lStyle |= SS_RIGHTJUST;
       if (strstr(argv[7].strptr,"SUNKEN")) lStyle |= SS_SUNKEN;

       if (strstr(argv[7].strptr,"SIZECONTROL")) lStyle |= SS_REALSIZECONTROL; else
       if (strstr(argv[7].strptr,"SIZEIMAGE")) lStyle |= SS_REALSIZEIMAGE;

       if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;
       if (strstr(argv[7].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (strstr(argv[7].strptr,"TAB")) lStyle |= WS_TABSTOP;

       /*                      id           x          y          cx         cy       text  */
       UAddControl(&p, 0x0082, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], "", lStyle);
   }
   else if (!strcmp(argv[0].strptr,"SB"))
   {
       CHECKARG(8);

       for ( i = 0; i < 5; i++ )
       {
           buffer[i] = atoi(argv[i+2].strptr);
       }

       p = (WORD *)GET_POINTER(argv[1]);

       lStyle = WS_CHILD;
       if (strstr(argv[7].strptr,"HORIZONTAL")) lStyle |= SBS_HORZ; else lStyle |= SBS_VERT;
       if (strstr(argv[7].strptr,"TOPLEFT")) lStyle |= SBS_TOPALIGN;
       if (strstr(argv[7].strptr,"BOTTOMRIGHT")) lStyle |= SBS_BOTTOMALIGN;
       if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;
       if (strstr(argv[7].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (strstr(argv[7].strptr,"TAB")) lStyle |= WS_TABSTOP;

       /*                         id       x          y            cx        cy  */
       UAddControl(&p, 0x0084, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
   }

   RETPTR(p);
}



/******************* New 32 Controls ***********************************/


LONG EvaluateListStyle(const char * styledesc)
{
    LONG lStyle = 0;

    if (!strstr(styledesc,"NOBORDER")) lStyle |= WS_BORDER;
    if (!strstr(styledesc,"NOTAB")) lStyle |= WS_TABSTOP;
    if (strstr(styledesc,"VSCROLL")) lStyle |= WS_VSCROLL;
    if (strstr(styledesc,"HSCROLL")) lStyle |= WS_HSCROLL;
    if (strstr(styledesc,"EDIT")) lStyle |= LVS_EDITLABELS;
    if (strstr(styledesc,"SHOWSELALWAYS")) lStyle |= LVS_SHOWSELALWAYS;
    if (strstr(styledesc,"ALIGNLEFT")) lStyle |= LVS_ALIGNLEFT;
    if (strstr(styledesc,"ALIGNTOP")) lStyle |= LVS_ALIGNTOP;
    if (strstr(styledesc,"AUTOARRANGE")) lStyle |= LVS_AUTOARRANGE;
    if (strstr(styledesc,"ICON")) lStyle |= LVS_ICON;
    if (strstr(styledesc,"SMALLICON")) lStyle |= LVS_SMALLICON;
    if (strstr(styledesc,"LIST")) lStyle |= LVS_LIST;
    if (strstr(styledesc,"REPORT")) lStyle |= LVS_REPORT;
    if (strstr(styledesc,"NOHEADER")) lStyle |= LVS_NOCOLUMNHEADER;
    if (strstr(styledesc,"NOWRAP")) lStyle |= LVS_NOLABELWRAP;
    if (strstr(styledesc,"NOSCROLL")) lStyle |= LVS_NOSCROLL;
    if (strstr(styledesc,"NOSORTHEADER")) lStyle |= LVS_NOSORTHEADER;
    if (strstr(styledesc,"SHAREIMAGES")) lStyle |= LVS_SHAREIMAGELISTS;
    if (strstr(styledesc,"SINGLESEL")) lStyle |= LVS_SINGLESEL;
    if (strstr(styledesc,"ASCENDING")) lStyle |= LVS_SORTASCENDING;
    if (strstr(styledesc,"DESCENDING")) lStyle |= LVS_SORTDESCENDING;
    return lStyle;
}

size_t RexxEntry UsrAddNewCtrl(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   INT buffer[5];
   ULONG lStyle;
   WORD *p;
   int i;

   CHECKARG(8);

   for ( i = 0; i < 5; i++ )
   {
       buffer[i] = atoi(argv[i+2].strptr);
   }

   p = (WORD *)GET_POINTER(argv[1]);

   lStyle = WS_CHILD;
   if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
   if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
   if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;

   if (!strcmp(argv[0].strptr,"TREE"))
   {
       if (!strstr(argv[7].strptr,"NOBORDER")) lStyle |= WS_BORDER;
       if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;
       if (strstr(argv[7].strptr,"VSCROLL")) lStyle |= WS_VSCROLL;
       if (strstr(argv[7].strptr,"HSCROLL")) lStyle |= WS_HSCROLL;
       if (strstr(argv[7].strptr,"NODRAG")) lStyle |= TVS_DISABLEDRAGDROP;
       if (strstr(argv[7].strptr,"EDIT")) lStyle |= TVS_EDITLABELS;
       if (strstr(argv[7].strptr,"BUTTONS")) lStyle |= TVS_HASBUTTONS;
       if (strstr(argv[7].strptr,"LINES")) lStyle |= TVS_HASLINES;
       if (strstr(argv[7].strptr,"ATROOT")) lStyle |= TVS_LINESATROOT;
       if (strstr(argv[7].strptr,"SHOWSELALWAYS")) lStyle |= TVS_SHOWSELALWAYS;
        /*                                   id       x          y            cx        cy  */
       UAddNamedControl(&p, WC_TREEVIEW, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
       RETPTR(p)
   }
   else if (!strcmp(argv[0].strptr,"LIST"))
   {
       lStyle |= EvaluateListStyle(argv[7].strptr);
        /*                                   id       x          y            cx        cy  */
       UAddNamedControl(&p, WC_LISTVIEW, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
       RETPTR(p)
   }
   else if (!strcmp(argv[0].strptr,"PROGRESS"))
   {
       if (strstr(argv[7].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (strstr(argv[7].strptr,"TAB")) lStyle |= WS_TABSTOP;
       if (strstr(argv[7].strptr,"VERTICAL")) lStyle |= PBS_VERTICAL;
       if (strstr(argv[7].strptr,"SMOOTH")) lStyle |= PBS_SMOOTH;

       if (strstr(argv[7].strptr,"MARQUEE") && ComCtl32Version >= COMCTL32_6_0) lStyle |= PBS_MARQUEE;

        /*                                     id       x          y            cx        cy  */
       UAddNamedControl(&p, PROGRESS_CLASS, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
       RETPTR(p)
   }
   else if (!strcmp(argv[0].strptr,"SLIDER"))
   {
       if (strstr(argv[7].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;
       if (strstr(argv[7].strptr,"AUTOTICKS")) lStyle |= TBS_AUTOTICKS;
       if (strstr(argv[7].strptr,"NOTICKS")) lStyle |= TBS_NOTICKS;
       if (strstr(argv[7].strptr,"VERTICAL")) lStyle |= TBS_VERT;
       if (strstr(argv[7].strptr,"HORIZONTAL")) lStyle |= TBS_HORZ;
       if (strstr(argv[7].strptr,"TOP")) lStyle |= TBS_TOP;
       if (strstr(argv[7].strptr,"BOTTOM")) lStyle |= TBS_BOTTOM;
       if (strstr(argv[7].strptr,"LEFT")) lStyle |= TBS_LEFT;
       if (strstr(argv[7].strptr,"RIGHT")) lStyle |= TBS_RIGHT;
       if (strstr(argv[7].strptr,"BOTH")) lStyle |= TBS_BOTH;
       if (strstr(argv[7].strptr,"ENABLESELRANGE")) lStyle |= TBS_ENABLESELRANGE;
        /*                                   id       x          y            cx        cy  */
       UAddNamedControl(&p, TRACKBAR_CLASS, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
       RETPTR(p)
   }
   else if (!strcmp(argv[0].strptr,"TAB"))
   {
        if (strstr(argv[7].strptr,"BORDER")) lStyle |= WS_BORDER;
        if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;
        if (strstr(argv[7].strptr,"BUTTONS")) lStyle |= TCS_BUTTONS;
        else lStyle |= TCS_TABS;
        if (strstr(argv[7].strptr,"FIXED")) lStyle |= TCS_FIXEDWIDTH;
        if (strstr(argv[7].strptr,"FOCUSNEVER")) lStyle |= TCS_FOCUSNEVER;
        if (strstr(argv[7].strptr,"FOCUSONDOWN")) lStyle |= TCS_FOCUSONBUTTONDOWN;
        if (strstr(argv[7].strptr,"ICONLEFT")) lStyle |= TCS_FORCEICONLEFT;
        if (strstr(argv[7].strptr,"LABELLEFT")) lStyle |= TCS_FORCELABELLEFT;
        if (strstr(argv[7].strptr,"MULTILINE")) lStyle |= TCS_MULTILINE;
        else lStyle |= TCS_SINGLELINE;
        if (strstr(argv[7].strptr,"ALIGNRIGHT")) lStyle |= TCS_RIGHTJUSTIFY;
        if (strstr(argv[7].strptr,"CLIPSIBLINGS")) lStyle |= WS_CLIPSIBLINGS;  /* used for property sheet to prevent wrong display */

        /*                                   id       x          y            cx        cy  */
        UAddNamedControl(&p, WC_TABCONTROL, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
        RETPTR(p)
   }
   else if (!strcmp(argv[0].strptr,"DTP"))  /* Date and Time Picker control */
   {
        if (strstr(argv[7].strptr, "BORDER")) lStyle |= WS_BORDER;
        if (!strstr(argv[7].strptr, "NOTAB")) lStyle |= WS_TABSTOP;
        if (strstr(argv[7].strptr, "PARSE"))  lStyle |= DTS_APPCANPARSE;
        if (strstr(argv[7].strptr, "RIGHT"))  lStyle |= DTS_RIGHTALIGN;
        if (strstr(argv[7].strptr, "NONE"))   lStyle |= DTS_SHOWNONE;
        if (strstr(argv[7].strptr, "UPDOWN")) lStyle |= DTS_UPDOWN;

        if (strstr(argv[7].strptr, "LONG"))
        {
            lStyle |= DTS_LONGDATEFORMAT;
        }
        else if (strstr(argv[7].strptr, "SHORT"))
        {
            lStyle |= DTS_SHORTDATEFORMAT;
        }
        else if (strstr(argv[7].strptr, "CENTURY") && (ComCtl32Version >= COMCTL32_5_8))
        {
            lStyle |= DTS_SHORTDATECENTURYFORMAT;
        }
        else if (strstr(argv[7].strptr, "TIME"))
        {
            lStyle |= DTS_TIMEFORMAT;
        }
        else
        {
            lStyle |= DTS_TIMEFORMAT;
        }

        /*                                       id         x          y            cx        cy  */
        UAddNamedControl(&p, DATETIMEPICK_CLASS, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
        RETPTR(p)
   }
   else if (!strcmp(argv[0].strptr,"MONTH"))  /* Month Calendar control */
   {
        if (strstr(argv[7].strptr, "BORDER"))      lStyle |= WS_BORDER;
        if (!strstr(argv[7].strptr, "NOTAB"))      lStyle |= WS_TABSTOP;
        if (strstr(argv[7].strptr, "DAYSTATE"))    lStyle |= MCS_DAYSTATE;
        if (strstr(argv[7].strptr, "MULTI"))       lStyle |= MCS_MULTISELECT;
        if (strstr(argv[7].strptr, "NOTODAY"))     lStyle |= MCS_NOTODAY;
        if (strstr(argv[7].strptr, "NOCIRCLE"))    lStyle |= MCS_NOTODAYCIRCLE;
        if (strstr(argv[7].strptr, "WEEKNUMBERS")) lStyle |= MCS_WEEKNUMBERS;

        /*                                   id         x          y            cx        cy  */
        UAddNamedControl(&p, MONTHCAL_CLASS, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
        RETPTR(p)
   }

   RETC(0);
}


/**
 *  Methods for the .UserDialog class.
 */
#define USERDIALOG_CLASS  "UserDialog"

RexxMethod4(RexxObjectPtr, userdlg_init, OPTIONAL_RexxObjectPtr, dlgData, OPTIONAL_RexxObjectPtr, includeFile,
            SUPER, super, OSELF, self)
{
    RexxArrayObject newArgs = context->NewArray(4);

    context->ArrayPut(newArgs, TheZeroObj, 1);
    context->ArrayPut(newArgs, TheZeroObj, 2);
    if ( argumentExists(1) )
    {
        context->ArrayPut(newArgs, dlgData, 3);
    }
    if ( argumentExists(2) )
    {
        context->ArrayPut(newArgs, includeFile, 4);
    }
    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, newArgs);

    if ( isInt(0, result, context) )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();
        context->SendMessage1(self, "DYNAMICINIT", context->NewPointer(pcpbd));
    }

    return result;
}


/**
 *  Methods for the .DynamicDialog class.
 */
#define DYNAMICDIALOG_CLASS  "DynamicDialog"


RexxMethod2(RexxObjectPtr, dyndlg_dynamicInit, POINTER, arg, OSELF, self)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)arg;

    RexxBufferObject pddBuffer = context->NewBuffer(sizeof(CDynamicDialog));
    if ( pddBuffer == NULLOBJECT )
    {
        goto done_out;
    }

    pCDynamicDialog pcdd = (pCDynamicDialog)context->BufferData(pddBuffer);
    memset(pcdd, 0, sizeof(CDynamicDialog));

    pcdd->pcpbd = pcpbd;
    pcdd->rexxSelf = self;
    context->SetObjectVariable("CSELF", pddBuffer);

    context->SendMessage1(self, "BASEPTR=", TheZeroObj);
    context->SendMessage1(self, "ACTIVEPTR=", TheZeroObj);
    context->SendMessage1(self, "DIALOGITEMCOUNT=", TheZeroObj);

done_out:
    return TheZeroObj;
}


RexxMethod6(RexxObjectPtr, dyndlg_startChildDialog, uint32_t, itemCount, POINTERSTRING, basePtr,
            RexxStringObject, hDlgParent, uint32_t, childIndex, OSELF, self, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;

    DIALOGADMIN *dlgAdm = pcdd->pcpbd->dlgAdm;
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(context->threadContext, self);
        return TheZeroObj;
    }

    DLGTEMPLATE *p = (DLGTEMPLATE *)basePtr;
    if ( p == NULL )
    {
        illegalBuffer();
        return TheZeroObj;
    }

    bool Release = false;
    HWND hDlg;

    /* Get the number of dialog controls. */
    p->cdit = (WORD)itemCount;

    /* Get the parent dialog's window handle. */
    hDlg = (HWND)string2pointer(context, hDlgParent);

    /* The child dialog needs to be created in the window procedure thread
     * of the parent.  The child dialog's handle is returned, will be NULL on
     * error.
     */
    hDlg = (HWND)SendMessage(hDlg, WM_USER_CREATECHILD, 0, (LPARAM)p);

    /* We are done with the dialog template. */
    safeLocalFree(p);

    /* The child dialog may not have been created. */
    if ( hDlg == NULL )
    {
        return TheZeroObj;
    }

    dlgAdm->ChildDlg[childIndex] = hDlg;
    return pointer2string(context, hDlg);
}


RexxMethod6(logical_t, dyndlg_startParentDialog, uint32_t, itemCount, POINTERSTRING, basePtr,
            uint32_t, iconID, logical_t, modeless, OSELF, self, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;

    DIALOGADMIN *dlgAdm = pcdd->pcpbd->dlgAdm;
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(context->threadContext, self);
        return FALSE;
    }

    DLGTEMPLATE *p = (DLGTEMPLATE *)basePtr;
    if ( p == NULL )
    {
        return illegalBuffer();
    }

    ULONG thID;
    bool Release = false;

    /* set number of items to dialogtemplate */
    p->cdit = (WORD)itemCount;

    EnterCriticalSection(&crit_sec);

    // InstallNecessaryStuff() can not fail for a UserDialog.
    InstallNecessaryStuff(dlgAdm, NULL);

    // Note: autoDetect is never on for a UserDialog dialog.  If autoDetect is
    // on, then each of the addXXX() automatically connects the control.  So,
    // autoDetect would just be redundent.
    LoopThreadArgs threadArgs;
    threadArgs.dlgTemplate = p;
    threadArgs.dlgAdmin = dlgAdm;
    threadArgs.release = &Release;

    dlgAdm->TheThread = CreateThread(NULL, 2000, (LPTHREAD_START_ROUTINE)WindowUsrLoopThread, &threadArgs, 0, &thID);

    // Wait for the dialog to start.
    while ( ! Release && dlgAdm && (dlgAdm->TheThread) )
    {
        Sleep(1);
    }
    LeaveCriticalSection(&crit_sec);

    // Free the memory allocated for template.
    safeLocalFree(p);

    if ( dlgAdm )
    {
        if ( dlgAdm->TheDlg )
        {
            // Set the thread priority higher for faster drawing.
            SetThreadPriority(dlgAdm->TheThread, THREAD_PRIORITY_ABOVE_NORMAL);
            dlgAdm->OnTheTop = TRUE;
            dlgAdm->threadID = thID;

            // Do we have a modal dialog?
            if ( ! modeless )
            {
                if ( dlgAdm->previous && IsWindowEnabled(((DIALOGADMIN *)dlgAdm->previous)->TheDlg) )
                {
                    EnableWindow(((DIALOGADMIN *)dlgAdm->previous)->TheDlg, FALSE);
                }
            }

            if ( GetWindowLong(dlgAdm->TheDlg, GWL_STYLE) & WS_SYSMENU )
            {
                HICON hBig = NULL;
                HICON hSmall = NULL;

                if ( GetDialogIcons(dlgAdm, iconID, ICON_FILE, (PHANDLE)&hBig, (PHANDLE)&hSmall) )
                {
                    dlgAdm->SysMenuIcon = (HICON)setClassPtr(dlgAdm->TheDlg, GCLP_HICON, (LONG_PTR)hBig);
                    dlgAdm->TitleBarIcon = (HICON)setClassPtr(dlgAdm->TheDlg, GCLP_HICONSM, (LONG_PTR)hSmall);
                    dlgAdm->DidChangeIcon = TRUE;

                    SendMessage(dlgAdm->TheDlg, WM_SETICON, ICON_SMALL, (LPARAM)hSmall);
                }
            }

            // TODO for now we need to convert hDlg back to a Rexx object. Plus, need to figure
            // out what struct we have and what to use here.
            setDlgHandle(context, pcdd->pcpbd, pointer2string(context, dlgAdm->TheDlg));
            return TRUE;
        }

        // The dialog creation failed, so clean up.  For now, with the
        // mixture of old and new native APIs, the freeing of the dialog
        // administration block must be done in the deInstall() or
        // unInit() methods.

        // TODO this seems very wrong.  Why isn't a DelDialog() done here???
        dlgAdm->OnTheTop = FALSE;
        if ( dlgAdm->previous )
        {
            ((DIALOGADMIN *)(dlgAdm->previous))->OnTheTop = TRUE;
        }
        if ( dlgAdm->previous && !IsWindowEnabled(((DIALOGADMIN *)dlgAdm->previous)->TheDlg) )
        {
            EnableWindow(((DIALOGADMIN *)dlgAdm->previous)->TheDlg, TRUE);
        }
    }
    return FALSE;
}

/** DynamicDialog::addIconFile  [private]
 *
 *  Basic implemetation for DyamicDialog::addIcon(resourceID, fileName).
 *
 */
RexxMethod3(int32_t, dyndlg_addIconFile_pvt, RexxObjectPtr, rxID, CSTRING, fileName, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;

    int32_t rc = -1;
    DIALOGADMIN * dlgAdm = pcdd->pcpbd->dlgAdm;
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(context->threadContext, pcdd->rexxSelf);
        goto done_out;
    }

    int32_t iconID = checkID(context, rxID, pcdd->rexxSelf);
    if ( iconID < 0 )
    {
        goto done_out;
    }

    if ( iconID <= IDI_DLG_MAX_ID )
    {
        char szBuf[196];
        sprintf(szBuf, "Icon resource ID: %d is not valid.  Resource\n"
                "IDs from 1 through %d are reserved for ooDialog\n"
                "internal resources.  The icon resource will not\n"
                "be added.", iconID, IDI_DLG_MAX_ID);
        internalErrorMsgBox(szBuf, OOD_RESOURCE_ERR_TITLE);
        goto done_out;
    }

    if ( dlgAdm->IconTab == NULL )
    {
        dlgAdm->IconTab = (ICONTABLEENTRY *)LocalAlloc(LPTR, sizeof(ICONTABLEENTRY) * MAX_IT_ENTRIES);
        if ( dlgAdm->IconTab == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }
        dlgAdm->IT_size = 0;
    }

    if ( dlgAdm->IT_size < MAX_IT_ENTRIES )
    {
        size_t i;

        // If there is already a resource with this ID, it is replaced.
        for ( i = 0; i < dlgAdm->IT_size; i++ )
        {
            if ( dlgAdm->IconTab[i].iconID == iconID )
                break;
        }

        dlgAdm->IconTab[i].fileName = (PCHAR)LocalAlloc(LPTR, strlen(fileName) + 1);
        if ( dlgAdm->IconTab[i].fileName == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }

        dlgAdm->IconTab[i].iconID = iconID;
        strcpy(dlgAdm->IconTab[i].fileName, fileName);
        if ( i == dlgAdm->IT_size )
        {
            dlgAdm->IT_size++;
        }
        rc = 0;
    }
    else
    {
        internalErrorMsgBox(OOD_ADDICONFILE_ERR_MSG, OOD_RESOURCE_ERR_TITLE);
    }

done_out:
    return rc;
}


RexxMethod0(RexxObjectPtr, dyndlg_stop)
{
    stopDialog(NULL);
    return NULLOBJECT;
}


extern BOOL SHIFTkey = FALSE;

#ifndef USE_DS_CONTROL
BOOL IsNestedDialogMessage(
    DIALOGADMIN * dlgAdm,
    PMSG  lpmsg    // address of structure with message
   )
{
   HWND hW, hParent, hW2;
   BOOL prev = FALSE;

   if ((!dlgAdm->ChildDlg) || (!dlgAdm->AktChild))
      return IsDialogMessage(dlgAdm->TheDlg, lpmsg);

   switch (lpmsg->message)
   {
      case WM_KEYDOWN:
         switch (lpmsg->wParam)
            {
            case VK_SHIFT: SHIFTkey = TRUE;
               break;

            case VK_TAB:

               if (IsChild(dlgAdm->AktChild, lpmsg->hwnd)) hParent = dlgAdm->AktChild; else hParent = dlgAdm->TheDlg;

               hW = GetNextDlgTabItem(hParent, lpmsg->hwnd, SHIFTkey);
               hW2 = GetNextDlgTabItem(hParent, NULL, SHIFTkey);

               /* see if we have to switch to the other dialog */
               if (hW == hW2)
               {
                  if (hParent == dlgAdm->TheDlg)
                     hParent = dlgAdm->AktChild;
                  else
                     hParent = dlgAdm->TheDlg;

                  hW = GetNextDlgTabItem(hParent, NULL, SHIFTkey);
                  return SendMessage(hParent, WM_NEXTDLGCTL, (WPARAM)hW, (LPARAM)TRUE) != 0;

               } else return SendMessage(hParent, WM_NEXTDLGCTL, (WPARAM)hW, (LPARAM)TRUE) != 0;

                return TRUE;

            case VK_LEFT:
            case VK_UP:
               prev = TRUE;
            case VK_RIGHT:
            case VK_DOWN:

               if (IsChild(dlgAdm->AktChild, lpmsg->hwnd)) hParent = dlgAdm->AktChild; else hParent = dlgAdm->TheDlg;

               hW = GetNextDlgGroupItem(hParent, lpmsg->hwnd, prev);
               hW2 = GetNextDlgGroupItem(hParent, NULL, prev);

               /* see if we have to switch to the other dialog */
               if (hW == hW2)
               {
                  if (hParent == dlgAdm->TheDlg)
                     hParent = dlgAdm->AktChild;
                  else
                     hParent = dlgAdm->TheDlg;

                   return IsDialogMessage(hParent, lpmsg);

               } else
                return IsDialogMessage(hParent, lpmsg);

                return TRUE;

            case VK_CANCEL:
            case VK_RETURN:
               return IsDialogMessage(dlgAdm->TheDlg, lpmsg);

            default:
               hParent = (HWND)getWindowPtr(lpmsg->hwnd, GWLP_HWNDPARENT);
               if (!hParent) return FALSE;
               return IsDialogMessage(hParent, lpmsg);
           }
         break;

      case WM_KEYUP:
         if (lpmsg->wParam == VK_SHIFT) SHIFTkey = FALSE;
         break;
   }
   hParent = (HWND)getWindowPtr(lpmsg->hwnd, GWLP_HWNDPARENT);
   if (hParent)
      return IsDialogMessage(hParent, lpmsg);
   else return IsDialogMessage(dlgAdm->TheDlg, lpmsg);
}
#endif



