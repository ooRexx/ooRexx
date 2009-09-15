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
#include <shlwapi.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodData.hpp"
#include "oodResourceIDs.hpp"

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


bool startDialogTemplate(RexxMethodContext *c, WORD **ppTemplate, WORD **p, uint32_t count,
                         INT x, INT y, INT cx, INT cy, const char * dlgClass, const char * title,
                         const char * fontname, INT fontsize, uint32_t lStyle)
{
    int   nchar;

    *ppTemplate = *p = (PWORD) LocalAlloc(LPTR, (count+3)*256);
    if ( p == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    /* start to fill in the dlgtemplate information.  addressing by WORDs */
    **p = LOWORD (lStyle);
    (*p)++;
    **p = HIWORD (lStyle);
    (*p)++;
    **p = 0;          // LOWORD (lExtendedStyle)
    (*p)++;
    **p = 0;          // HIWORD (lExtendedStyle)
    (*p)++;
    **p = count;      // NumberOfItems
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

    /* copy the class of the dialog. currently dlgClass is always null */
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

    /* add in the wPointSize and szFontName here iff the DS_SETFONT bit on.
     * currently DS_SETFONT is always set
     */
    **p = fontsize;   // fontsize
    (*p)++;
    nchar = nCopyAnsiToWideChar (*p, TEXT(fontname));
    (*p) += nchar;

    /* make sure the first item starts on a DWORD boundary */
    (*p) = lpwAlign (*p);
    return true;
}

#define DEFAULT_EXPECTED_DIALOG_ITEMS   200
#define FONT_NAME_ARG_POS                 8
#define FONT_SIZE_ARG_POS                 9
#define EXPECTED_ITEM_COUNT_ARG_POS      10

/**
 * Ensures the font name and font size for a dialog are correct, based on the
 * args for DynamicDialog::create().
 *
 * The font name and font size args to create() are optional.  If the user
 * omitted them, we use the default font for the dialog.  If the user specified
 * the font, the default font for the dialog is replaced by what the user has
 * specified.
 *
 * @param c           Method context we are operating in.
 * @param args        Argument array for the method.
 * @param pcpbd       Pointer to the CSelf for the method's dialog.
 *
 * @return True on no error, false if an exception has been raised.
 */
static bool adjustDialogFont(RexxMethodContext *c, RexxArrayObject args, pCPlainBaseDialog pcpbd)
{
    RexxObjectPtr name = c->ArrayAt(args, FONT_NAME_ARG_POS);
    if ( name != NULLOBJECT )
    {
        if ( ! c->IsString(name) )
        {
            wrongClassException(c->threadContext, FONT_NAME_ARG_POS, "String");
            return false;
        }

        CSTRING fontName = c->ObjectToStringValue(name);
        if ( strlen(fontName) > (MAX_DEFAULT_FONTNAME - 1) )
        {
            stringTooLongException(c->threadContext, 1, MAX_DEFAULT_FONTNAME, strlen(fontName));
            return false;
        }
        strcpy(pcpbd->fontName, fontName);
    }

    RexxObjectPtr size = c->ArrayAt(args, FONT_SIZE_ARG_POS);
    if ( size != NULLOBJECT )
    {
        uint32_t fontSize;
        if ( ! c->UnsignedInt32(size, &fontSize) )
        {
            c->RaiseException2(Rexx_Error_Invalid_argument_positive, c->WholeNumber(FONT_SIZE_ARG_POS), size);
            return false;
        }
        pcpbd->fontSize = fontSize;
    }
    return true;
}

static uint32_t getExpectedCount(RexxMethodContext *c, RexxArrayObject args)
{
    uint32_t expected = DEFAULT_EXPECTED_DIALOG_ITEMS;

    RexxObjectPtr count = c->ArrayAt(args, EXPECTED_ITEM_COUNT_ARG_POS);
    if ( count != NULLOBJECT )
    {
        if ( ! c->UnsignedInt32(count, &expected) || expected == 0 )
        {
            c->RaiseException2(Rexx_Error_Invalid_argument_positive, c->WholeNumber(EXPECTED_ITEM_COUNT_ARG_POS), count);
            expected = 0;
        }
    }
    return expected;
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


void addToDialogTemplate(WORD **p, SHORT kind, INT id, INT x, INT y, INT cx, INT cy, const char * txt, ULONG lStyle)
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

uint32_t getCommonWindowStyles(CSTRING opts, bool defaultBorder, bool defaultTab)
{
    uint32_t style = 0;

    if ( StrStrI(opts, "HIDDEN"  ) == NULL ) style |= WS_VISIBLE;
    if ( StrStrI(opts, "GROUP"   ) != NULL ) style |= WS_GROUP;
    if ( StrStrI(opts, "DISABLED") != NULL ) style |= WS_DISABLED;

    if ( defaultBorder )
    {
        if ( StrStrI(opts, "NOBORDER") == NULL ) style |= WS_BORDER;
    }
    else
    {
        if ( StrStrI(opts, "BORDER")   != NULL ) style |= WS_BORDER;
    }

    if ( defaultTab )
    {
        if ( StrStrI(opts, "NOTAB") == NULL ) style |= WS_TABSTOP;
    }
    else
    {
        if ( StrStrI(opts, "TAB")   != NULL ) style |= WS_TABSTOP;
    }
    return style;
}

size_t RexxEntry UsrAddControl(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   INT buffer[5];
   ULONG lStyle;
   WORD *p = NULL;
   int i;

   CHECKARGL(1);

   if ( !strcmp(argv[0].strptr,"CH") || !strcmp(argv[0].strptr,"RB") )
   {
       CHECKARG(9);

       /* UsrAddControl("CH", self~activePtr, id, x, y, w, h, name, opts) */
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
       addToDialogTemplate(&p, 0x0080, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], argv[7].strptr, lStyle);
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
       addToDialogTemplate(&p, 0x0081, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
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
       addToDialogTemplate(&p, 0x0083, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
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
       addToDialogTemplate(&p, 0x0085, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
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
       addToDialogTemplate(&p, 0x0082, i, buffer[0], buffer[1], buffer[2], buffer[3], argv[7].strptr, lStyle);
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
       addToDialogTemplate(&p, 0x0082, i, buffer[0], buffer[1], buffer[2], buffer[3], NULL, lStyle);
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
       addToDialogTemplate(&p, 0x0082, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], "", lStyle);
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
       addToDialogTemplate(&p, 0x0084, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
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


#define ButtonAtom           0x0080
#define EditAtom             0x0081
#define StaticAtom           0x0082
#define ListBoxAtom          0x0083
#define ScrollBarAtom        0x0084
#define ComboBoxAtom         0x0085


RexxMethod1(RexxObjectPtr, dyndlg_init_cls, OSELF, self)
{
    TheDynamicDialogClass = (RexxClassObject)self;
    return NULLOBJECT;
}

/** DynamicDialog::basePtr  [attribute private]
 */
RexxMethod1(RexxObjectPtr, dyndlg_getBasePtr, CSELF, pCSelf)
{
    RexxObjectPtr ptr = pointer2string(context, ((pCDynamicDialog)pCSelf)->base);
    return ptr;
}
RexxMethod2(RexxObjectPtr, dyndlg_setBasePtr, CSTRING, ptrStr, CSELF, pCSelf)
{
    ((pCDynamicDialog)pCSelf)->base = (DLGTEMPLATE *)string2pointer(ptrStr);
    return NULLOBJECT;
}

/** DynamicDialog::activePtr  [attribute private]
 */
RexxMethod1(RexxObjectPtr, dyndlg_getActivePtr, CSELF, pCSelf)
{
    RexxObjectPtr ptr = pointer2string(context, ((pCDynamicDialog)pCSelf)->active);
    return ptr;
}
RexxMethod2(RexxObjectPtr, dyndlg_setActivePtr, CSTRING, ptrStr, CSELF, pCSelf)
{
    ((pCDynamicDialog)pCSelf)->active = string2pointer(ptrStr);
    return NULLOBJECT;
}

/** DynamicDialog::dialogItemCount  [attribute private]
 */
RexxMethod1(uint32_t, dyndlg_getDialogItemCount, CSELF, pCSelf) { return ( ((pCDynamicDialog)pCSelf)->count ); }
RexxMethod2(RexxObjectPtr, dyndlg_setDialogItemCount, uint32_t, count, CSELF, pCSelf)
{
    ((pCDynamicDialog)pCSelf)->count = count;
    return NULLOBJECT;
}


/** DynamicDialog::dynamicInit()  [private]
 */
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

done_out:
    return TheZeroObj;
}


/**
 *
 * @param  x          ( 1 required) X co-ordinate
 * @param  y          ( 2 required) Y co-ordinate
 * @param  cx         ( 3 required) width
 * @param  cy         ( 4 required) height
 * @param  title      ( 5 required) Title for the caption bar
 * @param  _opts      ( 6 optional) Style 0ptions for the dialog
 * @param  dlgClass   ( 7 optional) The dialog class.  Has never been used.
 * @param  fontName   ( 8 optional) Font name for the dialog
 * @param  fontSize   ( 9 optional) Font size
 * @param  expected   (10 optional) Expected number of dialog items.
 *
 * @return True on success, false if an exception has been raised.
 *
 * @remarks  It is important to remember that when a "Child" dialog is being
 *           created, there is n9 backing Rexx dialog object.  Child dialogs are
 *           created for the 'category' pages of a CategoryDialog or its
 *           subclasses.
 */
RexxMethod9(logical_t, dyndlg_create, uint32_t, x, int32_t, y, int32_t, cx, uint32_t, cy, CSTRING, title,
            OPTIONAL_RexxStringObject, _opts, OPTIONAL_CSTRING, dlgClass, ARGLIST, args, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    uint32_t style = DS_SETFONT | WS_CAPTION;
    dlgClass = NULL;        // The dialog class is always ignored, at this time.

    if ( argumentExists(6) )
    {
        CSTRING opts = c->StringData(c->StringUpper(_opts));

        if ( strstr(opts, "VISIBLE")     != 0 ) style |= WS_VISIBLE;
        if ( strstr(opts, "NOMENU")      == 0 ) style |= WS_SYSMENU;
        if ( strstr(opts, "NOTMODAL")    == 0 ) style |= DS_MODALFRAME;
        if ( strstr(opts, "SYSTEMMODAL") != 0 ) style |= DS_SYSMODAL;
        if ( strstr(opts, "CENTER")      != 0 ) style |= DS_CENTER;
        if ( strstr(opts, "THICKFRAME")  != 0 ) style |= WS_THICKFRAME;
        if ( strstr(opts, "MINIMIZEBOX") != 0 ) style |= WS_MINIMIZEBOX;
        if ( strstr(opts, "MAXIMIZEBOX") != 0 ) style |= WS_MAXIMIZEBOX;
        if ( strstr(opts, "VSCROLL")     != 0 ) style |= WS_VSCROLL;
        if ( strstr(opts, "HSCROLL")     != 0 ) style |= WS_HSCROLL;
        if ( strstr(opts, "OVERLAPPED")  != 0 ) style |= WS_OVERLAPPED;
    }

    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( ! adjustDialogFont(context, args, pcpbd) )
    {
        // See comment below, do we need to do a stop()?
        return FALSE;
    }

    uint32_t expected = getExpectedCount(context, args);
    if ( expected == 0 )
    {
        // See comment below, do we need to do a stop()?
        return FALSE;
    }

    WORD *p;
    WORD *pBase;

    if ( ! startDialogTemplate(context, &pBase, &p, expected, x, y, cx, cy, dlgClass, title,
                               pcpbd->fontName, pcpbd->fontSize, style) )
    {
        // TODO an exception has been raised, so I don't think we need to do any
        // clean up ?  For a regular dialog, the original code did a
        // DynamicDialog::stop(), which does a stopDialog().  For a
        // CategoryDialog, things were just ignored.  Within ooDialog, this
        // exception is not trapped, so the interpreter should just end.  But,
        // what happens if the user traps syntax errors?
        return FALSE;
    }
    pcdd->base = (DLGTEMPLATE *)pBase;
    pcdd->active = p;
    pcpbd->wndBase->sizeX = cx;
    pcpbd->wndBase->sizeY = cy;

    c->SendMessage0(pcdd->rexxSelf, "DEFINEDIALOG");
    return pcdd->active != NULL;
}

/** DyamicDialog::startParentDialog()
 *
 *  Creates the underlying Windows dialog for a user dialog (or one of its
 *  subclasses) object.  This is the counterpart to the ResDialog::startDialog()
 *  which is only used to create the underlying Windows dialog for ResDialog
 *  dialogs.
 *
 *
 */
RexxMethod3(logical_t, dyndlg_startParentDialog, uint32_t, iconID, logical_t, modeless, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;

    DIALOGADMIN *dlgAdm = pcdd->pcpbd->dlgAdm;
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(context->threadContext, pcdd->pcpbd->rexxSelf);
        return FALSE;
    }

    DLGTEMPLATE *p = pcdd->base;
    if ( p == NULL )
    {
        return illegalBuffer();
    }

    ULONG thID;
    bool Release = false;

    // Set the number of dialog items field in the dialog template.
    p->cdit = (WORD)pcdd->count;

    EnterCriticalSection(&crit_sec);

    // InstallNecessaryStuff() can not fail for a UserDialog.
    InstallNecessaryStuff(dlgAdm, NULL);

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
    LocalFree(p);
    pcdd->base = NULL;
    pcdd->active = NULL;
    pcdd->count = 0;

    if ( dlgAdm )   // TODO not sure why this check was done in the original code?  dlgAdm can not be null at this point.
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

            setDlgHandle(context, pcdd->pcpbd, dlgAdm->TheDlg);
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

/** DynamicDialog::startChildDialog()
 *
 *  Creates the underlying Windows child dialog.
 *
 *  Child dialogs are the dialogs used for the pages in a CategoryDialog, or its
 *  subclass the PropertySheet dialog.  The parent window / dialog for the child
 *  dialog is always the category or propert sheet dialog.  At this time, there
 *  is no corresponding Rexx object for child dialogs.
 *
 *  @param  basePtr     Pointer to the in-memory dialog template for the child
 *                      dialog.
 *  @param  childIndex  The index number of the child.  This corresponds to the
 *                      page number the child is used in.  I.e., the child at
 *                      index 1 is the dialog for the first page of the category
 *                      or property sheet dialog, index 2 is the second page,
 *                      etc..
 *
 *  @return  The handle of the underlying Windows dialog, 0 on error.
 *
 *  @remarks  The child dialog needs to be created in the window procedure
 *            thread of the parent.  SendMessage() is used to send a user
 *            message to the message loop thread.  The child dialog is then
 *            created in that thread and the dialog handle returned.  On error,
 *            the returned handle will be NULL.
 *
 *            The basePtr is sent from the CategoryDialog code where it is
 *            stored as such: self~catalog['base'][i] where 'i' is the index of
 *            the child dialog.  This index is sent to us as childIndex.  After
 *            freeing the base pointer, we are relying on the category dialog
 *            code setting self~catalog['base'][i] back to 0.
 *
 *            We could eliminate the basePtr arg altogether and retrieve the
 *            pointer using the childIndex arg.  And, we could also set the
 *            value of self~catalog['base'][i] ourselfs.  TODO, this would make
 *            things more self-contained.
 *
 */
RexxMethod3(RexxObjectPtr, dyndlg_startChildDialog, POINTERSTRING, basePtr, uint32_t, childIndex, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    DIALOGADMIN *dlgAdm = pcpbd->dlgAdm;
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(context->threadContext, pcpbd->rexxSelf);
        return TheZeroObj;
    }

    DLGTEMPLATE *p = (DLGTEMPLATE *)basePtr;
    if ( p == NULL )
    {
        illegalBuffer();
        return TheZeroObj;
    }

    // Set the field for the number of dialog controls in the dialog template.
    p->cdit = (WORD)pcdd->count;

    HWND hChild = (HWND)SendMessage(pcpbd->hDlg, WM_USER_CREATECHILD, 0, (LPARAM)p);

    // Free the memory allocated for template.
    LocalFree(p);
    pcdd->active = NULL;
    pcdd->count = 0;

    // The child dialog may not have been created.
    if ( hChild == NULL )
    {
        return TheZeroObj;
    }

    dlgAdm->ChildDlg[childIndex] = hChild;
    return pointer2string(context, hChild);
}


/** DynamicDialog::addButton()
 *
 */
RexxMethod10(int32_t, dyndlg_addPushButton, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, label, OPTIONAL_CSTRING, msgToRaise, OPTIONAL_CSTRING, opts,
            OPTIONAL_CSTRING, loadOptions, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    if ( pcdd->active == NULL )
    {
        return -2;
    }

    DIALOGADMIN * dlgAdm = pcdd->pcpbd->dlgAdm;
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(context->threadContext, pcdd->rexxSelf);
        return -2;
    }


    int32_t id = checkID(context, rxID, pcdd->pcpbd->rexxSelf);
    if ( id < 1 )
    {
        return -1;
    }

    if ( argumentOmitted(6) )
    {
        label = "";
    }
    if ( argumentOmitted(8) )
    {
        opts = "";
    }

    uint32_t  style = WS_CHILD;
    WORD     *p     = (WORD *)pcdd->active;

    style |= getCommonWindowStyles(opts, false, true);

    if (StrStrI(opts,"DEFAULT")) style |= BS_DEFPUSHBUTTON; else style |= BS_PUSHBUTTON;

    if ( StrStrI(opts, "OWNER")     != NULL ) style |= BS_OWNERDRAW;
    if ( StrStrI(opts, "BITMAP")    != NULL ) style |= BS_BITMAP;
    if ( StrStrI(opts, "ICON")      != NULL ) style |= BS_ICON;
    if ( StrStrI(opts, "HCENTER")   != NULL ) style |= BS_CENTER;
    if ( StrStrI(opts, "TOP")       != NULL ) style |= BS_TOP;
    if ( StrStrI(opts, "BOTTOM")    != NULL ) style |= BS_BOTTOM;
    if ( StrStrI(opts, "VCENTER")   != NULL ) style |= BS_VCENTER;
    if ( StrStrI(opts, "PUSHLIKE")  != NULL ) style |= BS_PUSHLIKE;
    if ( StrStrI(opts, "MULTILINE") != NULL ) style |= BS_MULTILINE;
    if ( StrStrI(opts, "NOTIFY")    != NULL ) style |= BS_NOTIFY;
    if ( StrStrI(opts, "FLAT")      != NULL ) style |= BS_FLAT;

    const char *pLonger = StrStrI(opts, "LEFTTEXT");
    const char *pShorter = StrStrI(opts, "LEFT");
    if ( pLonger )
    {
        style |= BS_LEFTTEXT;
        if ( pShorter != NULL && pShorter != pLonger )
        {
            style |= BS_LEFT;
        }
    }
    else if ( pShorter )
    {
        style |= BS_LEFT;
    }

    pLonger = StrStrI(opts,"RIGHTBUTTON");
    pShorter = StrStrI(opts,"RIGHT");
    if ( pLonger )
    {
        style |= BS_RIGHTBUTTON;
        if ( pShorter != NULL && pShorter != pLonger )
        {
            style |= BS_RIGHT;
        }
    }
    else if ( pShorter )
    {
        style |= BS_RIGHT;
    }

    addToDialogTemplate(&p, ButtonAtom, id, x, y, cx, cy, label, style);
    pcdd->active = p;
    pcdd->count++;

    if ( id < 3 || id == 9 )
    {
        return 0;
    }

    CSTRING methName = NULL;
    int32_t result   = 0;

    if ( argumentExists(9)   )
    {
        if ( StrStrI(opts, "CONNECTBUTTONS") != NULL )
        {
            methName = strdup_2methodName(label);
        }
    }
    else if ( argumentExists(7) )
    {
        methName = strdup_nospace(msgToRaise);
    }

    if ( methName != NULL && strlen(methName) != 0 )
    {
        result = AddTheMessage(dlgAdm, WM_COMMAND, UINT32_MAX, id, UINTPTR_MAX, 0, 0, methName, 0) ? 0 : 1;
    }

    safeFree((void *)methName);
    return result;
}


/** DynamicDialog::addGroupBox()
 *
 */
RexxMethod8(int32_t, dyndlg_addGroupBox, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, text, OPTIONAL_CSTRING, opts, OPTIONAL_RexxObjectPtr, rxID, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id = IDC_STATIC;
    if ( argumentExists(7) )
    {
        id = checkID(context, rxID, pcdd->pcpbd->rexxSelf);
        if ( id < IDC_STATIC )
        {
            return -1;
        }
    }

    if ( argumentOmitted(5) )
    {
        text = "";
    }
    if ( argumentOmitted(6) )
    {
        opts = "";
    }

    // For a groupbox, we support right or left aligned text.  By default the
    // alignment is left so we only need to check for the RIGHT key word.

    uint32_t  style = WS_CHILD | BS_GROUPBOX;
    style |= getCommonWindowStyles(opts, false, false);
    if ( StrStrI(opts, "RIGHT") != NULL ) style |= BS_RIGHT;

    WORD *p = (WORD *)pcdd->active;
    addToDialogTemplate(&p, ButtonAtom, id, x, y, cx, cy, text, style);
    pcdd->active = p;
    pcdd->count++;

    return 0;
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


/** DynamicDialog::itemAdd()  [private]
 *
 *  Provides a central function to do several needed things for each potential
 *  dialog control to be added to the dialog template.
 *
 *  1.) Checks that template is still valid and returns -2 if not.
 *
 *  2.) If the control to be added must have a valid resource ID, resolves the
 *  ID  and returns -1 if the ID can not be resolved.
 *
 *  3.) If the control to be added is one that often uses the static ID (-1),
 *  resolves the resource ID, if one was specified, but uses the static ID if
 *  the specified ID does not resolve.  If the resource ID was not specified,
 *  just uses the static ID.
 *
 *  4.) After step 3, increments the dialog item count and returns the resource
 *  ID to use when adding the control.
 *
 *  During the conversion to the C++ APIs, this method is needed.  Once the
 *  conversion is finished, it can probaly go away.
 */
int32_t itemAdd(RexxMethodContext *c, pCDynamicDialog pcdd, RexxObjectPtr rxID, bool acceptStaticID, int shortCutID)
{
    // For normal dialogs, the base pointer is of course valid, for category
    // dialogs it could already have been set to NULL.  The active pointer is
    // what must be checked.
    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id;
    if ( ! acceptStaticID )
    {
        // checkID() will put up an error message box for any rxID it can not
        // resolve.  But, it will resolve a static ID and return -1.  The
        // control being added can not use a static ID, so we return -1.
        id = checkID(c, rxID, pcdd->pcpbd->rexxSelf);
        if ( id < 1 )
        {
            return -1;
        }
    }
    else
    {
        // The control being added can use a static ID, so we do not want an
        // error message box put up if it can not be resolved, we just want to
        // go with -1 if it does not resolve.
        if ( shortCutID == -1 )
        {
            id = -1;
        }
        else
        {
            id = resolveResourceID(c, rxID, pcdd->pcpbd->rexxSelf);
        }
    }

    pcdd->count++;
    return id;
}
RexxMethod3(int32_t, dyndlg_itemAdd_pvt, RexxObjectPtr, rxID, OPTIONAL_logical_t, staticIDSpecified, CSELF, pCSelf)
{
    if ( argumentExists(2) )
    {
        if ( ! staticIDSpecified )
        {
            return itemAdd(context, (pCDynamicDialog)pCSelf, rxID, true, -1);
        }
        return itemAdd(context, (pCDynamicDialog)pCSelf, rxID, true, 0);
    }
    return itemAdd(context, (pCDynamicDialog)pCSelf, rxID, false, 0);
}


/** DynamicDialog::stop()  [private]
 */
RexxMethod0(RexxObjectPtr, dyndlg_stop)
{
    stopDialog(NULL);
    return NULLOBJECT;
}


/** DynamicDialog::stopDynamic()   [private]
 *
 *  Sets the dialog template pointers back to null and the dialog item count to
 *  0.
 *
 *  This method is probably no longer needed.  Before the conversion to the C++
 *  APIs, I believe it was used as a sort of fail-safe method to ensure the base
 *  pointer attribute was not used after it was freed.  This attribute is now
 *  set to null when it is freed, making this method redundent.
 */
RexxMethod1(RexxObjectPtr, dyndlg_stopDynamic_pvt, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;

    if ( pcdd->base != NULL )
    {
        // TODO remove this printf before release.
        printf("DynamicDialog::stopDynamic() base pointer not null! base=%p\n", pcdd->base);
        LocalFree(pcdd->base);
    }

    pcdd->base = NULL;
    pcdd->active = NULL;
    pcdd->count = 0;

    return TheZeroObj;
}


/**
 *  Methods for the .CategoryDialog class.
 */
#define CATEGORYDIALOG_CLASS  "CategoryDialog"


/** CategoryDialog::createCategoryDialog()
 *
 *  Creates a child dialog for a page of the category dialog, not the top-level
 *  parent category dialog.
 *
 *
 */
RexxMethod8(logical_t, catdlg_createCategoryDialog, int32_t, x, int32_t, y, uint32_t, cx, uint32_t, cy,
            CSTRING, fontName, uint32_t, fontSize, uint32_t, expected, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    uint32_t style = DS_SETFONT | WS_CHILD;

#ifdef USE_DS_CONTROL
    sytle |= DS_CONTROL;
#endif

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    pCDynamicDialog pcdd = (pCDynamicDialog)c->ObjectToCSelf(pcpbd->rexxSelf, TheDynamicDialogClass);

    if ( strlen(fontName) == 0 )
    {
        fontName = pcpbd->fontName;
    }
    if ( fontSize == 0 )
    {
        fontSize = pcpbd->fontSize;
    }

    WORD *p;
    WORD *pBase;
    if ( ! startDialogTemplate(context, &pBase, &p, expected, x, y, cx, cy, NULL, NULL, fontName, fontSize, style) )
    {
        return FALSE;
    }

    pcdd->active = p;

    //  self~catalog['base'][self~catalog['category']] = base
    RexxDirectoryObject catalog = (RexxDirectoryObject)c->SendMessage0(pcpbd->rexxSelf, "CATALOG");
    RexxArrayObject     bases   = (RexxArrayObject)c->DirectoryAt(catalog, "base");
    RexxObjectPtr       rxPageID = c->DirectoryAt(catalog, "category");

    size_t i;
    c->StringSize (rxPageID, &i);
    c->ArrayPut(bases, pointer2string(context, pBase), i);

    return TRUE;
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



