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
#include "oodDeviceGraphics.hpp"
#include "oodControl.hpp"
#include "oodMessaging.hpp"
#include "oodResourceIDs.hpp"

//#define USE_DS_CONTROL

#ifndef USE_DS_CONTROL
BOOL IsNestedDialogMessage(DIALOGADMIN * dlgAdm, LPMSG lpmsg);
#endif


class LoopThreadArgs
{
public:
    DLGTEMPLATE       *dlgTemplate;
    pCPlainBaseDialog  pcpbd;
    bool              *release;
};

/**
 * Windows message loop for a UserDialog or UserDialog subclass.
 *
 * @param args
 *
 * @return DWORD WINAPI
 */
DWORD WINAPI WindowUsrLoopThread(LoopThreadArgs * args)
{
    bool *release = args->release;
    pCPlainBaseDialog pcpbd = args->pcpbd;
    DIALOGADMIN *dlgAdm = pcpbd->dlgAdm;

    dlgAdm->TheDlg = CreateDialogIndirectParam(MyInstance, args->dlgTemplate, NULL, (DLGPROC)RexxDlgProc, (LPARAM)pcpbd);
    dlgAdm->ChildDlg[0] = dlgAdm->TheDlg;

    if ( dlgAdm->TheDlg )
    {
        MSG msg;
        pcpbd->isActive = true;
        *release = true;

        while ( GetMessage(&msg,NULL, 0,0) && dialogInAdminTable(dlgAdm) && (!dlgAdm->LeaveDialog) )
        {
#ifdef USE_DS_CONTROL
            if ( dlgAdm && !IsDialogMessage(dlgAdm->TheDlg, &msg)
                 && !IsDialogMessage(dlgAdm->AktChild, &msg) )
            {
#else
            if ( dlgAdm && (!IsNestedDialogMessage(dlgAdm, &msg)) )
            {
#endif
                DispatchMessage(&msg);
            }
        }
    }
    else
    {
        *release = true;
    }

    // Need to synchronize here, otherwise dlgAdm might still be in the table
    // but delDialog is already running.
    EnterCriticalSection(&crit_sec);
    if ( dialogInAdminTable(dlgAdm) )
    {
        delDialog(pcpbd);
        dlgAdm->TheThread = NULL;
    }
    LeaveCriticalSection(&crit_sec);

    if ( pcpbd->dlgProcContext != NULL )
    {
        pcpbd->dlgProcContext->DetachThread();
        pcpbd->dlgProcContext = NULL;
    }

    return 0;
}


#define DEFAULT_EXPECTED_DIALOG_ITEMS   200
#define FONT_NAME_ARG_POS                 8
#define FONT_SIZE_ARG_POS                 9
#define EXPECTED_ITEM_COUNT_ARG_POS      10


static inline logical_t illegalBuffer(void)
{
    MessageBox(0, "Illegal resource buffer", "Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
    return FALSE;
}


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
 * Starts the in-memory dialog template.  This uses the older DLGTEMPLTE
 * structure not the extended (DLGTEMPLATEEX) structure.
 *
 * @param c
 * @param ppBase
 * @param pcdd
 * @param count
 * @param x
 * @param y
 * @param cx
 * @param cy
 * @param dlgClass
 * @param title
 * @param fontname
 * @param fontsize
 * @param style
 *
 * @return True on success, false on error.
 *
 * @note  putUnicodeText() is designed to handle, and do the 'right' thing, for
 *        both a null pointer and an empty string.  That is why no check for
 *        null is needed for title, dlgClass, or fontname.
 */
bool startDialogTemplate(RexxMethodContext *c, DLGTEMPLATE **ppBase, pCDynamicDialog pcdd, uint32_t count,
                         int x, int y, int cx, int cy, const char * dlgClass, const char * title,
                         const char * fontName, int fontSize, uint32_t style)
{
    WORD *p = (PWORD)LocalAlloc(LPTR, (count+3)*256);
    if ( p == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }
    *ppBase = (DLGTEMPLATE *)p;

    // Start to fill in the dlgtemplate information.  Addressing is by WORDs.
    *p++ = LOWORD(style);           // Style (DWORD in size.)
    *p++ = HIWORD(style);
    *p++ = 0;                       // Extended Style (DWORD in size.)
    *p++ = 0;
    *p++ = count;                   // Number of dialog controls
    *p++ = x;                       // x
    *p++ = y;                       // y
    *p++ = cx;                      // cx
    *p++ = cy;                      // cy
    *p++ = 0;                       // Menu

    // Copy the class of the dialog.  Really there should be a check that style
    // does not contain WS_CHILD, but currently dlgClass is always null so this
    // works.
    p += putUnicodeText(p, dlgClass);

    // Copy the title of the dialog.
    p += putUnicodeText(p, title);

    // Add in the wPointSize and szFontName here.  Really this should only be if
    // the DS_SETFONT bit on. But currently it is always set.
    *p++ = fontSize;
    p += putUnicodeText(p, fontName);

    // make sure the first item starts on a DWORD boundary
    p = lpwAlign(p);

    // Update the active pointer to reflect where we are in the template.
    pcdd->active = p;
    return true;
}


/**
 *  Adds a dialog control item to the in-memory dialog template.  We are using
 *  the DLGITEMTEMPLATE structure here, not the extended structure.
 *
 * @param pcdd
 * @param kind
 * @param className
 * @param id
 * @param x
 * @param y
 * @param cx
 * @param cy
 * @param txt
 * @param style
 *
 * @note  putUnicodeText() is designed to handle, and do the 'right' thing, for
 *        both a null pointer and an empty string.  That is why no check for
 *        null is needed for txt.  On the other hand className must be checked
 *        because that is how we determine if the control is being identified
 *        by the control atom or by the class name.
 */
void addToDialogTemplate(pCDynamicDialog pcdd, SHORT kind, const char *className, int id, int x, int y, int cx, int cy,
                         const char * txt, uint32_t style)
{
   WORD *p = (WORD *)pcdd->active;

   *p++ = LOWORD(style);
   *p++ = HIWORD(style);
   *p++ = 0;          // LOWORD (lExtendedStyle)
   *p++ = 0;          // HIWORD (lExtendedStyle)
   *p++ = x;          // x
   *p++ = y;          // y
   *p++ = cx;         // cx
   *p++ = cy;         // cy
   *p++ = id;         // ID

   if ( className == NULL )
   {
       *p++ = (WORD)0xffff;
       *p++ = (WORD)kind;
   }
   else
   {
       p += putUnicodeText(p, className);
   }

   p += putUnicodeText(p, txt);

   *p++ = 0;  // advance pointer over nExtraStuff WORD

   // make sure the next item starts on a DWORD boundary
   p = lpwAlign(p);

   // Update the active pointer and the number of dialog items so far.
   pcdd->active = p;
   pcdd->count++;
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


inline pCPlainBaseDialog getDDpcpbd(void *pCSelf)
{
    return (((pCDynamicDialog)pCSelf)->pcpbd);
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
        if ( StrStrI(opts, "BORDER") != NULL && StrStrI(opts, "NOBORDER") == NULL ) style |= WS_BORDER;
    }

    if ( defaultTab )
    {
        if ( StrStrI(opts, "NOTAB") == NULL ) style |= WS_TABSTOP;
    }
    else
    {
        if ( StrStrI(opts, "TAB") != NULL && StrStrI(opts, "NOTAB") == NULL ) style |= WS_TABSTOP;
    }
    return style;
}


uint32_t getCommonButtonStyles(uint32_t style, CSTRING opts, oodControl_t button)
{
    style |= getCommonWindowStyles(opts, false, button != winRadioButton);

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
    if ( StrStrI(opts, "LTEXT")     != NULL ) style |= BS_LEFTTEXT;
    if ( StrStrI(opts, "LEFT")      != NULL ) style |= BS_LEFT;
    if ( StrStrI(opts, "RBUTTON")   != NULL ) style |= BS_RIGHTBUTTON;
    if ( StrStrI(opts, "RIGHT")     != NULL ) style |= BS_RIGHT;

    return style;
}


uint32_t listViewStyle(CSTRING opts, uint32_t style)
{
    if ( StrStrI(opts, "VSCROLL"      ) != NULL ) style |= WS_VSCROLL;
    if ( StrStrI(opts, "HSCROLL"      ) != NULL ) style |= WS_HSCROLL;
    if ( StrStrI(opts, "EDIT"         ) != NULL ) style |= LVS_EDITLABELS;
    if ( StrStrI(opts, "SHOWSELALWAYS") != NULL ) style |= LVS_SHOWSELALWAYS;
    if ( StrStrI(opts, "ALIGNLEFT"    ) != NULL ) style |= LVS_ALIGNLEFT;
    if ( StrStrI(opts, "ALIGNTOP"     ) != NULL ) style |= LVS_ALIGNTOP;
    if ( StrStrI(opts, "AUTOARRANGE"  ) != NULL ) style |= LVS_AUTOARRANGE;
    if ( StrStrI(opts, "ICON"         ) != NULL ) style |= LVS_ICON;
    if ( StrStrI(opts, "SMALLICON"    ) != NULL ) style |= LVS_SMALLICON;
    if ( StrStrI(opts, "LIST"         ) != NULL ) style |= LVS_LIST;
    if ( StrStrI(opts, "REPORT"       ) != NULL ) style |= LVS_REPORT;
    if ( StrStrI(opts, "NOHEADER"     ) != NULL ) style |= LVS_NOCOLUMNHEADER;
    if ( StrStrI(opts, "NOWRAP"       ) != NULL ) style |= LVS_NOLABELWRAP;
    if ( StrStrI(opts, "NOSCROLL"     ) != NULL ) style |= LVS_NOSCROLL;
    if ( StrStrI(opts, "NOSORTHEADER" ) != NULL ) style |= LVS_NOSORTHEADER;
    if ( StrStrI(opts, "SHAREIMAGES"  ) != NULL ) style |= LVS_SHAREIMAGELISTS;
    if ( StrStrI(opts, "SINGLESEL"    ) != NULL ) style |= LVS_SINGLESEL;
    if ( StrStrI(opts, "ASCENDING"    ) != NULL ) style |= LVS_SORTASCENDING;
    if ( StrStrI(opts, "DESCENDING"   ) != NULL ) style |= LVS_SORTDESCENDING;
    return style;
}


uint32_t treeViewStyle(CSTRING opts, uint32_t style)
{
    if ( StrStrI(opts,"ALL") != NULL )
    {
        style |=  TVS_HASLINES | WS_VSCROLL | WS_HSCROLL | TVS_EDITLABELS | TVS_HASBUTTONS |
                  TVS_LINESATROOT | TVS_SHOWSELALWAYS;
        return style;
    }

    if ( StrStrI(opts, "VSCROLL"      ) != NULL ) style |= WS_VSCROLL;
    if ( StrStrI(opts, "HSCROLL"      ) != NULL ) style |= WS_HSCROLL;
    if ( StrStrI(opts, "NODRAG"       ) != NULL ) style |= TVS_DISABLEDRAGDROP;
    if ( StrStrI(opts, "EDIT"         ) != NULL ) style |= TVS_EDITLABELS;
    if ( StrStrI(opts, "BUTTONS"      ) != NULL ) style |= TVS_HASBUTTONS;
    if ( StrStrI(opts, "LINES"        ) != NULL ) style |= TVS_HASLINES;
    if ( StrStrI(opts, "ATROOT"       ) != NULL ) style |= TVS_LINESATROOT;
    if ( StrStrI(opts, "SHOWSELALWAYS") != NULL ) style |= TVS_SHOWSELALWAYS;
    return style;
}


uint32_t progressBarStyle(CSTRING opts, uint32_t style)
{
    if ( StrStrI(opts, "VERTICAL") != NULL ) style |= PBS_VERTICAL;
    if ( StrStrI(opts, "SMOOTH"  ) != NULL ) style |= PBS_SMOOTH;

    if ( StrStrI(opts, "MARQUEE") != NULL  && ComCtl32Version >= COMCTL32_6_0 ) style |= PBS_MARQUEE;
    return style;
}


uint32_t trackBarStyle(CSTRING opts, uint32_t style)
{
    if ( StrStrI(opts, "AUTOTICKS"     ) != NULL ) style |= TBS_AUTOTICKS;
    if ( StrStrI(opts, "NOTICKS"       ) != NULL ) style |= TBS_NOTICKS;
    if ( StrStrI(opts, "VERTICAL"      ) != NULL ) style |= TBS_VERT;
    if ( StrStrI(opts, "HORIZONTAL"    ) != NULL ) style |= TBS_HORZ;
    if ( StrStrI(opts, "TOP"           ) != NULL ) style |= TBS_TOP;
    if ( StrStrI(opts, "BOTTOM"        ) != NULL ) style |= TBS_BOTTOM;
    if ( StrStrI(opts, "LEFT"          ) != NULL ) style |= TBS_LEFT;
    if ( StrStrI(opts, "RIGHT"         ) != NULL ) style |= TBS_RIGHT;
    if ( StrStrI(opts, "BOTH"          ) != NULL ) style |= TBS_BOTH;
    if ( StrStrI(opts, "ENABLESELRANGE") != NULL ) style |= TBS_ENABLESELRANGE;
    return style;
}


uint32_t tabStyle(CSTRING opts, uint32_t style)
{
    style |= (StrStrI(opts, "BUTTONS")   != NULL ? TCS_BUTTONS   : TCS_TABS);
    style |= (StrStrI(opts, "MULTILINE") != NULL ? TCS_MULTILINE : TCS_SINGLELINE);

    if ( StrStrI(opts, "FIXED"       ) != NULL ) style |= TCS_FIXEDWIDTH;
    if ( StrStrI(opts, "FOCUSNEVER"  ) != NULL ) style |= TCS_FOCUSNEVER;
    if ( StrStrI(opts, "FOCUSONDOWN" ) != NULL ) style |= TCS_FOCUSONBUTTONDOWN;
    if ( StrStrI(opts, "ICONLEFT"    ) != NULL ) style |= TCS_FORCEICONLEFT;
    if ( StrStrI(opts, "LABELLEFT"   ) != NULL ) style |= TCS_FORCELABELLEFT;
    if ( StrStrI(opts, "ALIGNRIGHT"  ) != NULL ) style |= TCS_RIGHTJUSTIFY;
    if ( StrStrI(opts, "CLIPSIBLINGS") != NULL ) style |= WS_CLIPSIBLINGS;
    return style;
}


uint32_t dateTimePickerStyle(CSTRING opts, uint32_t style)
{
    if ( StrStrI(opts, "LONG") != NULL )
    {
        style |= DTS_LONGDATEFORMAT;
    }
    else if ( StrStrI(opts, "SHORT") != NULL )
    {
        style |= DTS_SHORTDATEFORMAT;
    }
    else if ( StrStrI(opts, "CENTURY") != NULL  && ComCtl32Version >= COMCTL32_5_8 )
    {
        style |= DTS_SHORTDATECENTURYFORMAT;
    }
    else if ( StrStrI(opts, "TIME") != NULL )
    {
        style |= DTS_TIMEFORMAT;
    }
    else
    {
        style |= DTS_TIMEFORMAT;
    }

    if ( StrStrI(opts, "PARSE" ) != NULL ) style |= DTS_APPCANPARSE;
    if ( StrStrI(opts, "RIGHT" ) != NULL ) style |= DTS_RIGHTALIGN;
    if ( StrStrI(opts, "SHOWNONE"  ) != NULL ) style |= DTS_SHOWNONE;
    if ( StrStrI(opts, "UPDOWN") != NULL ) style |= DTS_UPDOWN;
    return style;
}


uint32_t monthCalendarStyle(CSTRING opts, uint32_t style)
{
    if ( StrStrI(opts, "DAYSTATE"   ) != NULL ) style |= MCS_DAYSTATE;
    if ( StrStrI(opts, "MULTI"      ) != NULL ) style |= MCS_MULTISELECT;
    if ( StrStrI(opts, "NOTODAY"    ) != NULL ) style |= MCS_NOTODAY;
    if ( StrStrI(opts, "NOCIRCLE"   ) != NULL ) style |= MCS_NOTODAYCIRCLE;
    if ( StrStrI(opts, "WEEKNUMBERS") != NULL ) style |= MCS_WEEKNUMBERS;

    if ( _isAtLeastVista() )
    {
        if ( StrStrI(opts, "NOTRAILING" ) != NULL ) style |= MCS_NOTRAILINGDATES;
        if ( StrStrI(opts, "SHORTDAYS"  ) != NULL ) style |= MCS_SHORTDAYSOFWEEK;
        if ( StrStrI(opts, "NOSELCHANGE") != NULL ) style |= MCS_NOSELCHANGEONNAV;
    }
    return style;
}


uint32_t upDownStyle(CSTRING opts, uint32_t style)
{
    if ( StrStrI(opts,      "LEFT" ) != NULL ) style |= UDS_ALIGNLEFT;
    else if ( StrStrI(opts, "RIGHT") != NULL ) style |= UDS_ALIGNRIGHT;
    else style |= UDS_ALIGNRIGHT;

    if ( StrStrI(opts, "ARROWKEYS"  ) != NULL ) style |= UDS_ARROWKEYS;
    if ( StrStrI(opts, "AUTOBUDDY"  ) != NULL ) style |= UDS_AUTOBUDDY;
    if ( StrStrI(opts, "HORIZONTAL" ) != NULL ) style |= UDS_HORZ;
    if ( StrStrI(opts, "HOTTRACK"   ) != NULL ) style |= UDS_HOTTRACK;
    if ( StrStrI(opts, "NOTHOUSANDS") != NULL ) style |= UDS_NOTHOUSANDS;
    if ( StrStrI(opts, "BUDDYINT"   ) != NULL ) style |= UDS_SETBUDDYINT;
    if ( StrStrI(opts, "WRAP"       ) != NULL ) style |= UDS_WRAP;
    return style;
}


uint32_t getControlStyle(oodControl_t ctrl, CSTRING opts)
{
    uint32_t style = WS_CHILD;

    switch ( ctrl )
    {
        case winListView :
            style |= getCommonWindowStyles(opts, true, true);
            style = listViewStyle(opts, style);
            break;

        case winTreeView :
            style |= getCommonWindowStyles(opts, true, true);
            style = treeViewStyle(opts, style);
            break;

        case winProgressBar :
            style |= getCommonWindowStyles(opts, false, false);
            style = progressBarStyle(opts, style);
            break;

        case winTrackBar :
            style |= getCommonWindowStyles(opts, false, true);
            style = trackBarStyle(opts, style);
            break;

        case winTab :
            style |= getCommonWindowStyles(opts, false, true);
            style = tabStyle(opts, style);
            break;

        case winDateTimePicker :
            style |= getCommonWindowStyles(opts, false, true);
            style = dateTimePickerStyle(opts, style);
            break;

        case winMonthCalendar :
            style |= getCommonWindowStyles(opts, false, true);
            style = monthCalendarStyle(opts, style);
            break;

        case winUpDown :
            style |= getCommonWindowStyles(opts, false, false);
            style = upDownStyle(opts, style);
            break;

        default :
            // Can not happen.
            break;

    }
    return style;
}


int32_t createStaticText(RexxMethodContext *c, RexxObjectPtr rxID, int x, int y, uint32_t cx, uint32_t cy,
                         CSTRING opts, CSTRING text, pCDynamicDialog pcdd)
{
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;
    int32_t id = IDC_STATIC;

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    id = checkID(c, rxID, pcdd->pcpbd->rexxSelf);
    if ( id < IDC_STATIC )
    {
        return -1;
    }

    if ( cx == 0 || cy == 0 )
    {
        SIZE textSize = {0};
        char *tempText = ( *text == '\0' ? "Tg" : text );

        if ( ! getTextSize(c, text, pcpbd->fontName, pcpbd->fontSize, NULL, pcpbd->rexxSelf, &textSize) )
        {
            // An exception is raised.
            return -2;
        }
        if ( cx == 0 )
        {
            // The magic number 2 comes from old ooDialog Rexx code, is it good?
            cx = textSize.cx + 2;
        }
        if ( cy == 0 )
        {
            cy = textSize.cy;
        }
    }

    uint32_t style = WS_CHILD | SS_LEFT;
    style |= getCommonWindowStyles(opts, false, false);

    if ( *opts != '\0' )
    {
        if ( StrStrI(opts, "CENTER" ) != NULL ) style |= SS_CENTER;
        else if ( StrStrI(opts, "RIGHT"     ) != NULL ) style |= SS_RIGHT;
        else if ( StrStrI(opts, "SIMPLE"    ) != NULL ) style |= SS_SIMPLE;
        else if ( StrStrI(opts, "LEFTNOWRAP") != NULL ) style |= SS_LEFTNOWORDWRAP;

        // Used to center text vertically.
        if ( StrStrI(opts, "CENTERIMAGE") != NULL ) style |= SS_CENTERIMAGE;

        if ( StrStrI(opts, "NOTIFY"      ) != NULL ) style |= SS_NOTIFY;
        if ( StrStrI(opts, "SUNKEN"      ) != NULL ) style |= SS_SUNKEN;
        if ( StrStrI(opts, "EDITCONTROL" ) != NULL ) style |= SS_EDITCONTROL;
        if ( StrStrI(opts, "ENDELLIPSIS" ) != NULL ) style |= SS_ENDELLIPSIS;
        if ( StrStrI(opts, "NOPREFIX"    ) != NULL ) style |= SS_NOPREFIX;
        if ( StrStrI(opts, "PATHELLIPSIS") != NULL ) style |= SS_PATHELLIPSIS;
        if ( StrStrI(opts, "WORDELLIPSIS") != NULL ) style |= SS_WORDELLIPSIS;
    }

    addToDialogTemplate(pcdd, StaticAtom, NULL, id, x, y, cx, cy, text, style);
    return 0;
}


int32_t createStaticImage(RexxMethodContext *c, RexxObjectPtr rxID, int x, int y, uint32_t cx, uint32_t cy,
                         CSTRING opts, pCDynamicDialog pcdd)
{
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    uint32_t id = checkID(c, rxID, pcdd->pcpbd->rexxSelf);
    if ( id < 0 )
    {
        return id;
    }

    uint32_t style = WS_CHILD;
    style |= getCommonWindowStyles(opts, false, false);

    if ( StrStrI(opts, "METAFILE" ) != NULL ) style |= SS_ENHMETAFILE;
    else if ( StrStrI(opts, "BITMAP" ) != NULL ) style |= SS_BITMAP;
    else style |= SS_ICON;

    if ( StrStrI(opts, "NOTIFY"      )  != NULL ) style |= SS_NOTIFY;
    if ( StrStrI(opts, "SUNKEN"      )  != NULL ) style |= SS_SUNKEN;
    if ( StrStrI(opts, "CENTERIMAGE" )  != NULL ) style |= SS_CENTERIMAGE;
    if ( StrStrI(opts, "RIGHTJUST"    ) != NULL ) style |= SS_RIGHTJUST;
    if ( StrStrI(opts, "SIZECONTROL" )  != NULL ) style |= SS_REALSIZECONTROL;
    if ( StrStrI(opts, "SIZEIMGE"    )  != NULL ) style |= SS_REALSIZEIMAGE;

    addToDialogTemplate(pcdd, StaticAtom, NULL, id, x, y, cx, cy, NULL, style);
    return 0;
}


int32_t createStaticFrame(RexxMethodContext *c, RexxObjectPtr rxID, int x, int y, uint32_t cx, uint32_t cy,
                         CSTRING opts, CSTRING type, uint32_t frameStyle, pCDynamicDialog pcdd)
{
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;
    int32_t id = IDC_STATIC;

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    id = checkID(c, rxID, pcdd->pcpbd->rexxSelf);
    if ( id < IDC_STATIC )
    {
        return id;
    }

    uint32_t style = WS_CHILD;
    style |= getCommonWindowStyles(opts, false, false);

    if ( type == NULL )
    {
        style |= frameStyle;
    }
    else
    {
        if ( strcmp(type, "WHITERECT") == 0 ) style |= SS_WHITERECT;
        else if ( strcmp(type, "GRAYRECT"       ) == 0 ) style |= SS_GRAYRECT;
        else if ( strcmp(type, "BLACKRECT"      ) == 0 ) style |= SS_BLACKRECT;
        else if ( strcmp(type, "WHITEFRAME"     ) == 0 ) style |= SS_WHITEFRAME;
        else if ( strcmp(type, "GRAYFRAME"      ) == 0 ) style |= SS_GRAYFRAME;
        else if ( strcmp(type, "BLACKFRAME"     ) == 0 ) style |= SS_BLACKFRAME;
        else if ( strcmp(type, "ETCHEDFRAME"    ) == 0 ) style |= SS_ETCHEDFRAME;
        else if ( strcmp(type, "ETCHEDHORZONTAL") == 0 ) style |= SS_ETCHEDHORZ;
        else if ( strcmp(type, "ETCHEDVERTICAL" ) == 0 ) style |= SS_ETCHEDVERT;
        else if ( strcmp(type, "STATICFRAME"    ) == 0 )
        {
            if ( StrStrI(opts, "WHITERECT") != NULL ) style |= SS_WHITERECT;
            else if ( StrStrI(opts, "GRAYRECT"  ) != NULL ) style |= SS_GRAYRECT;
            else if ( StrStrI(opts, "BLACKRECT" ) != NULL ) style |= SS_BLACKRECT;
            else if ( StrStrI(opts, "WHITEFRAME") != NULL ) style |= SS_WHITEFRAME;
            else if ( StrStrI(opts, "GRAYFRAME" ) != NULL ) style |= SS_GRAYFRAME;
            else if ( StrStrI(opts, "BLACKFRAME") != NULL ) style |= SS_BLACKFRAME;
            else if ( StrStrI(opts, "ETCHED"    ) != NULL ) style |= SS_ETCHEDFRAME;
            else if ( StrStrI(opts, "HORZ"      ) != NULL ) style |= SS_ETCHEDHORZ;
            else if ( StrStrI(opts, "VERT"      ) != NULL ) style |= SS_ETCHEDVERT;
        }
    }

    if ( StrStrI(opts, "NOTIFY") != NULL ) style |= SS_NOTIFY;
    if ( StrStrI(opts, "SUNKEN") != NULL ) style |= SS_SUNKEN;

    addToDialogTemplate(pcdd, StaticAtom, NULL, id, x, y, cx, cy, NULL, style);
    return 0;
}


int32_t connectCreatedControl(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr rxID, int32_t id,
                              CSTRING attributeName, oodControl_t ctrl)
{
    DIALOGADMIN * dlgAdm = pcpbd->dlgAdm;
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(c->threadContext, pcpbd->rexxSelf);
        return -2;
    }

    char buf[64];
    if ( attributeName == NULL || *attributeName == '\0' )
    {
        _snprintf(buf, sizeof(buf), "DATA%d", id);
        attributeName = buf;
    }

    uint32_t category = getCategoryNumber(c, pcpbd->rexxSelf);

    c->SendMessage2(pcpbd->rexxSelf, "ADDATTRIBUTE", rxID, c->String(attributeName));

    return addToDataTable(c, dlgAdm, id, ctrl, category);
}


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
 *           created, there is no backing Rexx dialog object.  Child dialogs are
 *           created for the 'category' pages of a CategoryDialog or its
 *           subclasses.
 */
RexxMethod9(logical_t, dyndlg_create, uint32_t, x, int32_t, y, int32_t, cx, uint32_t, cy, CSTRING, title,
            OPTIONAL_RexxStringObject, _opts, OPTIONAL_CSTRING, dlgClass, ARGLIST, args, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    uint32_t style = DS_SETFONT | WS_CAPTION | WS_SYSMENU;
    dlgClass = NULL;        // The dialog class is always ignored, at this time.

    if ( argumentExists(6) )
    {
        CSTRING opts = c->StringData(c->StringUpper(_opts));

        if ( strstr(opts, "VISIBLE")     != 0 ) style |= WS_VISIBLE;
        if ( strstr(opts, "NOMENU")      != 0 ) style &= ~WS_SYSMENU;
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
        goto err_out;
    }

    uint32_t expected = getExpectedCount(context, args);
    if ( expected == 0 )
    {
        goto err_out;
    }

    // We need to pass in and set the base address separately because the
    // category diaogs also use startDialogTemplate() and they need to keep
    // track of the different child dialog base addresses.
    DLGTEMPLATE *pBase;

    if ( ! startDialogTemplate(context, &pBase, pcdd, expected, x, y, cx, cy, dlgClass, title,
                               pcpbd->fontName, pcpbd->fontSize, style) )
    {
       goto err_out;
    }
    pcdd->base = pBase;
    pcpbd->wndBase->sizeX = cx;
    pcpbd->wndBase->sizeY = cy;

    c->SendMessage0(pcdd->rexxSelf, "DEFINEDIALOG");
    return pcdd->active != NULL;

err_out:
    // No underlying windows dialog is created, but we still need to clean up
    // the admin block, which was allocated when the Rexx dialog object was
    // instantiated.  This admin block is now in the DialogTab.

    delDialog(pcpbd);
    return FALSE;

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
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    DIALOGADMIN *dlgAdm = pcpbd->dlgAdm;
    if ( dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(context->threadContext, pcpbd->rexxSelf);
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
    threadArgs.pcpbd = pcpbd;
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

    // The dialog creation failed, so do some final clean up.  Note that the
    // dialog admin block can not be freed in delDialog() because of its use
    // here.
    //
    // When the dialog creation fails in the WindowUsrLoop thread a delDialog()
    // is immediately done, as it fails to enter the message processing loop.
    dlgAdm->OnTheTop = FALSE;
    if ( dlgAdm->previous )
    {
        ((DIALOGADMIN *)(dlgAdm->previous))->OnTheTop = TRUE;
    }
    if ( dlgAdm->previous && !IsWindowEnabled(((DIALOGADMIN *)dlgAdm->previous)->TheDlg) )
    {
        EnableWindow(((DIALOGADMIN *)dlgAdm->previous)->TheDlg, TRUE);
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


/** DynamicDialog::createStatic()
 *
 */
RexxMethod8(int32_t, dyndlg_createStatic, OPTIONAL_RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, text, CSELF, pCSelf)
{
    if ( argumentOmitted(1) )
    {
        rxID = TheNegativeOneObj;
    }
    if ( argumentOmitted(6) )
    {
        opts = "TEXT";
    }
    if ( argumentOmitted(7) )
    {
        text = "";
    }

    if ( StrStrI(opts, "TEXT") != NULL )
    {
        return createStaticText(context, rxID, x, y, cx, cy, opts, text, (pCDynamicDialog)pCSelf);
    }

    if ( StrStrI(opts, "BITMAP") != NULL || StrStrI(opts, "METAFILE") != NULL || StrStrI(opts, "ICON") != NULL )
    {
        return createStaticImage(context, rxID, x, y, cx, cy, opts, (pCDynamicDialog)pCSelf);
    }

    uint32_t frameStyle = 0;
    if ( StrStrI(opts, "WHITERECT") != NULL ) frameStyle = SS_WHITERECT;
    else if ( StrStrI(opts, "GRAYRECT"  ) != NULL ) frameStyle = SS_GRAYRECT;
    else if ( StrStrI(opts, "BLACKRECT" ) != NULL ) frameStyle = SS_BLACKRECT;
    else if ( StrStrI(opts, "WHITEFRAME") != NULL ) frameStyle = SS_WHITEFRAME;
    else if ( StrStrI(opts, "GRAYFRAME" ) != NULL ) frameStyle = SS_GRAYFRAME;
    else if ( StrStrI(opts, "BLACKFRAME") != NULL ) frameStyle = SS_BLACKFRAME;
    else if ( StrStrI(opts, "ETCHED"    ) != NULL ) frameStyle = SS_ETCHEDFRAME;
    else if ( StrStrI(opts, "HORZ"      ) != NULL ) frameStyle = SS_ETCHEDHORZ;
    else if ( StrStrI(opts, "VERT"      ) != NULL ) frameStyle = SS_ETCHEDVERT;

    if ( frameStyle != 0 )
    {
        return createStaticFrame(context, rxID, x, y, cx, cy, opts, NULL, frameStyle, (pCDynamicDialog)pCSelf);
    }

    return createStaticText(context, rxID, x, y, cx, cy, opts, text, (pCDynamicDialog)pCSelf);
}


/** DynamicDialog::createStaticText()
 *
 */
RexxMethod8(int32_t, dyndlg_createStaticText, OPTIONAL_RexxObjectPtr, rxID,
            int, x, int, y, OPTIONAL_uint32_t, cx, OPTIONAL_uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, text, CSELF, pCSelf)
{
    if ( argumentOmitted(1) )
    {
        rxID = TheNegativeOneObj;
    }
    if ( argumentOmitted(6) )
    {
        opts = "";
    }
    if ( argumentOmitted(7) )
    {
        text = "";
    }
    return createStaticText(context, rxID, x, y, cx, cy, opts, text, (pCDynamicDialog)pCSelf);
}


/** DynamicDialog::createStaticImage()
 *
 */
RexxMethod7(int32_t, dyndlg_createStaticImage, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, CSELF, pCSelf)
{
    if ( argumentOmitted(6) )
    {
        opts = "";
    }
    return createStaticImage(context, rxID, x, y, cx, cy, opts, (pCDynamicDialog)pCSelf);
}


/** DynamicDialog::createStaticFrame()
 *
 */
RexxMethod8(int32_t, dyndlg_createStaticFrame, OPTIONAL_RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, NAME, msgName, CSELF, pCSelf)
{
    if ( argumentOmitted(1) )
    {
        rxID = TheNegativeOneObj;
    }
    if ( argumentOmitted(6) )
    {
        opts = "";
    }
    return createStaticFrame(context, rxID, x, y, cx, cy, opts, msgName + 6, 0, (pCDynamicDialog)pCSelf);
}

/**
 * Used by create push buttons, radio buttons, and check boxes to determine if
 * an automatic event connection is desired.
 *
 * @param opts
 * @param ctrl
 *
 * @return bool
 */
inline bool needButtonConnect(CSTRING opts, oodControl_t ctrl)
{
    if ( opts != NULL )
    {
        switch ( ctrl )
        {
            case winCheckBox :
                if ( StrStrI(opts, "CONNECTCHECKS") != NULL )
                {
                    return true;
                }
                break;
            case winRadioButton :
                if ( StrStrI(opts, "CONNECTRADIOS") != NULL )
                {
                    return true;
                }
                break;
            case winPushButton :
                if ( StrStrI(opts, "CONNECTBUTTONS") != NULL )
                {
                    return true;
                }
                break;
            default :
                break;
        }
    }
    return false;
}

/** DynamicDialog::createPushButton()
 *
 */
RexxMethod10(int32_t, dyndlg_createPushButton, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
             OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, label, OPTIONAL_CSTRING, msgToRaise, OPTIONAL_CSTRING, loadOptions,
             CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id = checkID(context, rxID, pcdd->pcpbd->rexxSelf);
    if ( id < 1 )
    {
        return -1;
    }

    if ( argumentOmitted(6) )
    {
        opts = "";
    }
    if ( argumentOmitted(7) )
    {
        label = "";
    }

    uint32_t style = WS_CHILD;
    style |= ( StrStrI(opts, "DEFAULT") != NULL ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON );
    style = getCommonButtonStyles(style, opts, winPushButton);

    addToDialogTemplate(pcdd, ButtonAtom, NULL, id, x, y, cx, cy, label, style);

    if ( id < IDCANCEL || id == IDHELP )
    {
        return 0;
    }

    CSTRING methName = NULL;
    int32_t result   = 0;

    if ( needButtonConnect(loadOptions, winPushButton) )
    {
        methName = strdup_2methodName(label);
    }
    else if ( argumentExists(8) )
    {
        methName = strdup_nospace(msgToRaise);
    }

    if ( methName != NULL && strlen(methName) != 0 )
    {
        result = addCommandMessage(pcdd->pcpbd->enCSelf, id, UINTPTR_MAX, 0, 0, methName, 0) ? 0 : 1;
    }

    safeFree((void *)methName);
    return result;
}


/** DynamicDialog::createRadioButton() / DynamicDialog::createCheckBox()
 *
 *  @remarks  The code for both createRadioButton() and createCheckBox() is so
 *            parallel it just doesn't make sense to have 2 separate native
 *            methods.
 */
RexxMethod10(int32_t, dyndlg_createRadioButton, RexxObjectPtr, rxID, int, x, int, y,
             OPTIONAL_uint32_t, cx, OPTIONAL_uint32_t, cy, OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, label,
             OPTIONAL_CSTRING, attributeName, OPTIONAL_CSTRING, loadOptions, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id = checkID(context, rxID, pcpbd->rexxSelf);
    if ( id < 1 )
    {
        return -1;
    }
    if ( argumentOmitted(7) )
    {
        label = "";
    }
    if ( argumentOmitted(4) || argumentOmitted(5) )
    {
        SIZE textSize = {0};
        char *tempText = ( *label == '\0' ? "Tig" : label );

        if ( ! getTextSize(context, label, pcpbd->fontName, pcpbd->fontSize, NULL, pcpbd->rexxSelf, &textSize) )
        {
            // An exception is raised.
            return -2;
        }
        if ( cx == 0 )
        {
            // The magic number 12 comes from old ooDialog Rexx code, is it good?
            cx = textSize.cx + 12;
        }
        if ( cy == 0 )
        {
            cy = textSize.cy;
        }
    }
    if ( argumentOmitted(6) )
    {
        opts = "";
    }
    if ( argumentOmitted(8) )
    {
        attributeName = label;
    }

    oodControl_t ctrl = winCheckBox;
    if ( strcmp("CREATERADIOBUTTON", context->GetMessageName()) == 0 )
    {
        ctrl = winRadioButton;
    }

    uint32_t style = WS_CHILD;
    if ( ctrl == winRadioButton )
    {
        style |= BS_AUTORADIOBUTTON;
    }
    else
    {
        style |= ( StrStrI(opts, "3STATE") != NULL ? BS_AUTO3STATE : BS_AUTOCHECKBOX );
    }
    style = getCommonButtonStyles(style, opts, ctrl);

    addToDialogTemplate(pcdd, ButtonAtom, NULL, id, x, y, cx, cy, label, style);

    int32_t result = 0;

    if ( needButtonConnect(loadOptions, ctrl) )
    {
        CSTRING methName = strdup_2methodName(label);
        if ( methName == NULL )
        {
            outOfMemoryException(context->threadContext);
            return -2;
        }

        char *finalName = (char *)malloc(strlen(methName) + 3);
        if ( finalName == NULL )
        {
            outOfMemoryException(context->threadContext);
            return -2;
        }
        strcpy(finalName, "ID");
        strcat(finalName, methName);

        result = addCommandMessage(pcpbd->enCSelf, id, UINTPTR_MAX, 0, 0, finalName, 0) ? 0 : 1;
        free((void *)methName);
        free((void *)finalName);
    }

    // Connect the data attribute if we need to.
    if ( result == 0 && pcpbd->autoDetect && StrStrI(opts, "CAT") == NULL )
    {
        result = connectCreatedControl(context, pcpbd, rxID, id, attributeName, ctrl);
    }
    return result;
}

/** DynamicDialog::createGroupBox()
 *
 */
RexxMethod8(int32_t, dyndlg_createGroupBox, OPTIONAL_RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, text, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id = IDC_STATIC;
    if ( argumentExists(1) )
    {
        id = checkID(context, rxID, pcdd->pcpbd->rexxSelf);
        if ( id < IDC_STATIC )
        {
            return -1;
        }
    }
    if ( argumentOmitted(6) )
    {
        opts = "";
    }
    if ( argumentOmitted(7) )
    {
        text = "";
    }

    // For a groupbox, we support right or left aligned text.  By default the
    // alignment is left so we only need to check for the RIGHT key word.

    uint32_t  style = WS_CHILD | BS_GROUPBOX;
    style |= getCommonWindowStyles(opts, false, false);
    if ( StrStrI(opts, "RIGHT") != NULL )
    {
        style |= BS_RIGHT;
    }

    addToDialogTemplate(pcdd, ButtonAtom, NULL, id, x, y, cx, cy, text, style);
    return 0;
}


/** DynamicDialog::createEdit()
 *
 */
RexxMethod8(int32_t, dyndlg_createEdit, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, OPTIONAL_uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, attributeName, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id = checkID(context, rxID, pcpbd->rexxSelf);
    if ( id < 1 )
    {
        return -1;
    }

    // If the user does not specify a height for the edit control, we calculate
    // it and add a border of 2 dialog units.
    if ( argumentOmitted(5) )
    {
        SIZE textSize = {0};
        if ( ! getTextSize(context, "Tg", pcpbd->fontName, pcpbd->fontSize, NULL, pcpbd->rexxSelf, &textSize) )
        {
            // An exception is raised.
            return -2;
        }
        cy = textSize.cy + 4;
    }
    if ( argumentOmitted(8) )
    {
        opts = "";
    }

    uint32_t style = WS_CHILD;
    style |= getCommonWindowStyles(opts, true, true);

    if ( StrStrI(opts,"MULTILINE") )
    {
        style |= ES_MULTILINE;
        if ( StrStrI(opts, "NOWANTRETURN")  == NULL ) style |= ES_WANTRETURN;
        if ( StrStrI(opts, "HIDESELECTION") == NULL ) style |= ES_NOHIDESEL;
    }

    if ( StrStrI(opts, "CENTER") )
    {
        style |= ES_CENTER;
    }
    else
    {
        style |= ( StrStrI(opts, "RIGHT") != NULL ? ES_RIGHT : ES_LEFT );
    }

    if ( StrStrI(opts, "PASSWORD"     ) != NULL ) style |= ES_PASSWORD;
    if ( StrStrI(opts, "AUTOSCROLLH"  ) != NULL ) style |= ES_AUTOHSCROLL;
    if ( StrStrI(opts, "AUTOSCROLLV"  ) != NULL ) style |= ES_AUTOVSCROLL;
    if ( StrStrI(opts, "HSCROLL"      ) != NULL ) style |= WS_HSCROLL;
    if ( StrStrI(opts, "VSCROLL"      ) != NULL ) style |= WS_VSCROLL;
    if ( StrStrI(opts, "READONLY"     ) != NULL ) style |= ES_READONLY;
    if ( StrStrI(opts, "KEEPSELECTION") != NULL ) style |= ES_NOHIDESEL;
    if ( StrStrI(opts, "UPPER"        ) != NULL ) style |= ES_UPPERCASE;
    if ( StrStrI(opts, "LOWER"        ) != NULL ) style |= ES_LOWERCASE;
    if ( StrStrI(opts, "NUMBER"       ) != NULL ) style |= ES_NUMBER;
    if ( StrStrI(opts, "OEM"          ) != NULL ) style |= ES_OEMCONVERT;

    if ( strcmp("CREATEPASSWORDEDIT", context->GetMessageName()) == 0 )
    {
        style |= ES_PASSWORD;
    }

    addToDialogTemplate(pcdd, EditAtom, NULL, id, x, y, cx, cy, NULL, style);

    int32_t result = 0;

    // Connect the data attribute if we need to.
    if ( pcpbd->autoDetect )
    {
        result = connectCreatedControl(context, pcpbd, rxID, id, attributeName, winEdit);
    }
    return result;
}


/** DynamicDialog::createScrollBar()
 *
 */
RexxMethod7(int32_t, dyndlg_createScrollBar, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id = checkID(context, rxID, pcdd->pcpbd->rexxSelf);
    if ( id < 1 )
    {
        return -1;
    }
    if ( argumentOmitted(6) )
    {
        opts = "";
    }

    uint32_t style = WS_CHILD;
    style |= getCommonWindowStyles(opts, false, false);
    style |= ( StrStrI(opts, "HORIZONTAL") != NULL ? SBS_HORZ : SBS_VERT );

    if ( StrStrI(opts, "TOPLEFT")    != NULL ) style |= SBS_TOPALIGN;
    if ( StrStrI(opts, "BOTTOMRIGH") != NULL ) style |= SBS_BOTTOMALIGN;

    addToDialogTemplate(pcdd, ScrollBarAtom, NULL, id, x, y, cx, cy, NULL, style);
    return 0;
}


/** DynamicDialog::createListBox()
 *
 */
RexxMethod8(int32_t, dyndlg_createListBox, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, attributeName, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id = checkID(context, rxID, pcpbd->rexxSelf);
    if ( id < 1 )
    {
        return -1;
    }
    if ( argumentOmitted(6) )
    {
        opts = "";
    }

    uint32_t style = WS_CHILD;
    style |= getCommonWindowStyles(opts, true, true);

    if ( StrStrI(opts, "COLUMNS" ) != NULL ) style |= LBS_USETABSTOPS;
    if ( StrStrI(opts, "VSCROLL" ) != NULL ) style |= WS_VSCROLL;
    if ( StrStrI(opts, "HSCROLL" ) != NULL ) style |= WS_HSCROLL;
    if ( StrStrI(opts, "SORT"    ) != NULL ) style |= LBS_STANDARD;
    if ( StrStrI(opts, "NOTIFY"  ) != NULL ) style |= LBS_NOTIFY;
    if ( StrStrI(opts, "MULTI"   ) != NULL ) style |= LBS_MULTIPLESEL;
    if ( StrStrI(opts, "MCOLUMN" ) != NULL ) style |= LBS_MULTICOLUMN;
    if ( StrStrI(opts, "PARTIAL" ) != NULL ) style |= LBS_NOINTEGRALHEIGHT;
    if ( StrStrI(opts, "SBALWAYS") != NULL ) style |= LBS_DISABLENOSCROLL;
    if ( StrStrI(opts, "KEYINPUT") != NULL ) style |= LBS_WANTKEYBOARDINPUT;
    if ( StrStrI(opts, "EXTSEL"  ) != NULL ) style |= LBS_EXTENDEDSEL;

    addToDialogTemplate(pcdd, ListBoxAtom, NULL, id, x, y, cx, cy, NULL, style);

    int32_t result = 0;

    // Connect the data attribute if we need to.
    if ( pcpbd->autoDetect )
    {
        result = connectCreatedControl(context, pcpbd, rxID, id, attributeName, winListBox);
    }
    return result;
}


/** DynamicDialog::createComboBox()
 *
 */
RexxMethod8(int32_t, dyndlg_createComboBox, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, attributeName, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id = checkID(context, rxID, pcpbd->rexxSelf);
    if ( id < 1 )
    {
        return -1;
    }
    if ( argumentOmitted(6) )
    {
        opts = "";
    }

    uint32_t style = WS_CHILD;
    style |= getCommonWindowStyles(opts, true, true);

    if ( StrStrI(opts,"SIMPLE") )    style |= CBS_SIMPLE;
    else if ( StrStrI(opts,"LIST") ) style |= CBS_DROPDOWNLIST;
    else                             style |= CBS_DROPDOWN;

    if ( StrStrI(opts, "NOHSCROLL" ) == NULL ) style |= CBS_AUTOHSCROLL;
    if ( StrStrI(opts, "VSCROLL"   ) != NULL ) style |= WS_VSCROLL;
    if ( StrStrI(opts, "SORT"      ) != NULL ) style |= CBS_SORT;
    if ( StrStrI(opts, "PARTIAL"   ) != NULL ) style |= CBS_NOINTEGRALHEIGHT;

    addToDialogTemplate(pcdd, ComboBoxAtom, NULL, id, x, y, cx, cy, NULL, style);

    int32_t result = 0;

    // Connect the data attribute if we need to.
    if ( pcpbd->autoDetect && StrStrI(opts, "CAT") == NULL )
    {
        result = connectCreatedControl(context, pcpbd, rxID, id, attributeName, winComboBox);
    }
    return result;
}

/** DynamicDialog::createProgressBar()
 *
 */
RexxMethod7(int32_t, dyndlg_createProgressBar, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id = checkID(context, rxID, pcpbd->rexxSelf);
    if ( id < 1 )
    {
        return -1;
    }
    if ( argumentOmitted(6) )
    {
        opts = "";
    }

    uint32_t style = getControlStyle(winProgressBar, opts);

    addToDialogTemplate(pcdd, 0, PROGRESS_CLASS, id, x, y, cx, cy, NULL, style);
    return 0;
}


/** DynamicDialog::createNamedControl()
 *
 */
RexxMethod9(int32_t, dyndlg_createNamedControl, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, attributeName, NAME, msgName, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id = checkID(context, rxID, pcpbd->rexxSelf);
    if ( id < 1 )
    {
        return -1;
    }
    if ( argumentOmitted(6) )
    {
        opts = "";
    }

    oodControl_t ctrl = oodName2controlType(msgName + 6);
    CSTRING windowClass = controlType2winName(ctrl);

    uint32_t style = getControlStyle(ctrl, opts);

    addToDialogTemplate(pcdd, 0, windowClass, id, x, y, cx, cy, NULL, style);

    int32_t result = 0;

    // Connect the data attribute if we need to.
    if ( pcpbd->autoDetect )
    {
        if ( ! (ctrl == winTab && StrStrI(opts, "CAT") != NULL) )
        {
            result = connectCreatedControl(context, pcpbd, rxID, id, attributeName, ctrl);
        }
    }
    return result;
}

/** DynamicDialog::addButton()
 *  [deprecated] forward to createPushButton()
 */
RexxMethod10(RexxObjectPtr, dyndlg_addButton, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, label, OPTIONAL_CSTRING, msgToRaise, OPTIONAL_CSTRING, opts,
            OPTIONAL_CSTRING, loadOptions, ARGLIST, args)
{
    RexxArrayObject newArgs = context->NewArray(9);

    for ( int i = 1; i <= 5; i++ )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, i), i);
    }
    if ( argumentExists(6) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 6), 7);
    }
    if ( argumentExists(7) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 7), 8);
    }
    if ( argumentExists(8) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 8), 6);
    }
    if ( argumentExists(9) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 9), 9);
    }
    return context->ForwardMessage(NULL, "createPushButton", NULL, newArgs);
}

/** DynamicDialog::addRadioButton() / DynamicDialog::addCheckBox()
 *
 *  [deprecated forward to createRadioButton]
 */
RexxMethod10(RexxObjectPtr, dyndlg_addRadioButton, RexxObjectPtr, rxID, OPTIONAL_CSTRING, attributeName,
             int, x, int, y, OPTIONAL_uint32_t, cx, OPTIONAL_uint32_t, cy,
             CSTRING, label, OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, loadOptions, ARGLIST, args)
{
    RexxArrayObject newArgs = context->NewArray(9);

    context->ArrayPut(newArgs,     context->ArrayAt(args, 1), 1);
    if ( argumentExists(2) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 2), 8);
    }
    context->ArrayPut(newArgs,     context->ArrayAt(args, 3), 2);
    context->ArrayPut(newArgs,     context->ArrayAt(args, 4), 3);
    if ( argumentExists(5) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 5), 4);
    }
    if ( argumentExists(6) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 6), 5);
    }
    context->ArrayPut(newArgs,     context->ArrayAt(args, 7), 7);
    if ( argumentExists(8) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 8), 6);
    }
    if ( argumentExists(9) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 9), 9);
    }

    CSTRING msgName = "CREATECHECKBOX";
    if ( strcmp("ADDRADIOBUTTON", context->GetMessageName()) == 0 )
    {
        msgName = "CREATERADIOBUTTON";
    }
    return context->ForwardMessage(NULL, msgName, NULL, newArgs);
}

RexxMethod8(RexxObjectPtr, dyndlg_addGroupBox, int, x, int, y, uint32_t, cx, uint32_t, cy, OPTIONAL_CSTRING, text,
            OPTIONAL_CSTRING, opts, OPTIONAL_RexxObjectPtr, rxID, ARGLIST, args)
{
    RexxArrayObject newArgs = context->NewArray(7);

    for ( int i = 1; i <= 4; i++ )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, i), i + 1);
    }
    if ( argumentExists(5) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 5), 7);
    }
    if ( argumentExists(6) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 6), 6);
    }
    if ( argumentExists(7) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 7), 1);
    }
    return context->ForwardMessage(NULL, "createGroupBox", NULL, newArgs);
}

/** DynamicDialog::addEntryLine  [deprecated forward to createEdit]
 */
RexxMethod8(RexxObjectPtr, dyndlg_addEntryLine, RexxObjectPtr, rxID, OPTIONAL_CSTRING, attributeName, int, x, int, y,
             uint32_t, cx, OPTIONAL_uint32_t, cy, OPTIONAL_CSTRING, opts, ARGLIST, args)
{
    RexxArrayObject newArgs = context->NewArray(7);

    context->ArrayPut(newArgs,     context->ArrayAt(args, 1), 1);
    if ( argumentExists(2) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 2), 7);
    }
    context->ArrayPut(newArgs,     context->ArrayAt(args, 3), 2);
    context->ArrayPut(newArgs,     context->ArrayAt(args, 4), 3);
    context->ArrayPut(newArgs,     context->ArrayAt(args, 5), 4);
    if ( argumentExists(6) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 6), 5);
    }
    if ( argumentExists(7) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 7), 6);
    }

    const char *msgName = "createEdit";
    if ( strcmp("ADDPASSWORDLINE", context->GetMessageName()) == 0 )
    {
        msgName = "createPasswordEdit";
    }
    return context->ForwardMessage(NULL, msgName, NULL, newArgs);
}

/** DynamicDialog::addMethod()  [deprecated forward to appropriate createMethod]
 */
RexxMethod9(RexxObjectPtr, dyndlg_addMethod, RexxObjectPtr, rxID, OPTIONAL_CSTRING, attributeName, int, x, int, y,
             uint32_t, cx, uint32_t, cy, OPTIONAL_CSTRING, opts, NAME, msgName, ARGLIST, args)
{
    RexxArrayObject newArgs = context->NewArray(7);

    context->ArrayPut(newArgs, context->ArrayAt(args, 1), 1);
    if ( argumentExists(2) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 2), 7);
    }
    for ( int i = 3; i <= 7; i++)
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, i), i - 1);
    }
    if ( argumentExists(7) )
    {
        context->ArrayPut(newArgs, context->ArrayAt(args, 7), 6);
    }

    if ( strcmp("ADDLISTBOX", msgName) == 0 ) msgName = "createListBox";
    else if ( strcmp("ADDCOMBOBOX",      msgName) == 0 ) msgName = "createComboBox";
    else if ( strcmp("ADDTREECONTROL",   msgName) == 0 ) msgName = "createTreeView";
    else if ( strcmp("ADDLISTCONTROL",   msgName) == 0 ) msgName = "createListView";
    else if ( strcmp("ADDSLIDERCONTROL", msgName) == 0 ) msgName = "createTrackBar";
    else if ( strcmp("ADDTABCONTROL",    msgName) == 0 ) msgName = "createTab";

    return context->ForwardMessage(NULL, msgName, NULL, newArgs);
}

/** DynamicDialog::addIconResource
 *
 *  Adds an icon resource file name to the dialog.  The file name can either
 *  be added by the programmer or is automatically added when a resource script
 *  is parsed and contains an ICON resource statement.
 *
 *  The programmer can then use the icon either as the application icon, or use
 *  it as an image in her program by using the class method, userIcon() of the
 *  .Image class.  Both of these methods will load the icon image from the file
 *  system.
 *
 *  @param  rxID      The resource ID for the icon, can be numeric or symbolic.
 *  @param  fileName  The file name of the icon resource.
 *
 *  @return Zero on success, negative one for error.
 *
 *  @note  The icons added to the dialog are located by resource ID.  If an icon
 *         with the same ID is already in the dialog, it is replaced by the icon
 *         specified.
 */
RexxMethod3(int32_t, dyndlg_addIconResource, RexxObjectPtr, rxID, CSTRING, fileName, CSELF, pCSelf)
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

        char *buf = (char *)LocalAlloc(LPTR, strlen(fileName) + 1);
        if ( buf == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }
        strcpy(buf, fileName);
        StrTrim(buf, " \"'");

        dlgAdm->IconTab[i].fileName = buf;
        dlgAdm->IconTab[i].iconID = iconID;
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


/** DynamicDialog::stop()  [private]
 */
RexxMethod1(RexxObjectPtr, dyndlg_stop, CSELF, pCSelf)
{
    stopDialog(getDDpcpbd(pCSelf));
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
RexxMethod2(RexxObjectPtr, dyndlg_stopDynamic_pvt, OPTIONAL_RexxObjectPtr, ignored, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = (pCDynamicDialog)pCSelf;

    if ( pcdd->base != NULL )
    {
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


/**
 * Given a CategoryDialog, retrieves the dialog handle corresponding to the
 * given page ID.
 *
 * Note that it is expected that this function will be called from native
 * methods where the pageID argument is possibly omitted in the Rexx method. The
 * category number of 0, is the main dialog, the parent dialog of a
 * CategoryDialog.
 *
 * The original implementation of ooDialog had a convention, if the category
 * number is is omitted and the dialog is a category dialog, the category number
 * should be the category number of the current category page.
 *
 * @param c            Method context we are operating under.
 *
 * @param categoryDlg  The CategoryDialog object.
 *
 * @param pageID       [in / out]  The page ID of the category dialog.  This
 *                     argument is unchanged on return if a lookup of the active
 *                     page is not done.  When the active page is looked up, the
 *                     resolved page ID is returned here.
 *
 * @param hDlg         [out]  The handle of the page dialog is returned here, on
 *                     success.
 *
 * @return  True if no error, otherwise false.
 *
 * @assumes The caller has already checked that categoryDlg is in fact a
 *          category dialog object.
 *
 * @remarks  In the init() method of a CategoryDialog, the 'CATALOG' attribute
 *           is set to a directory, and the 'handles', and 'category' entries
 *           are added. They must be there or ooRexx is broken. So, no check for
 *           NULLOBJECT is done for them. Note that the indexes of the CATALOG
 *           directory are all lower case.
 */
bool getCategoryHDlg(RexxMethodContext *context, RexxObjectPtr categoryDlg, uint32_t *pageID, HWND *hDlg,
                         int argPosPageID)
{
    bool result = false;

    RexxDirectoryObject catalog = (RexxDirectoryObject)context->SendMessage0(categoryDlg, "CATALOG");
    RexxArrayObject handles = (RexxArrayObject)context->DirectoryAt(catalog, "handles");

    if ( argumentOmitted(argPosPageID) )
    {
        // Look up the active page number.
        RexxObjectPtr rxPageID = context->DirectoryAt(catalog, "category");

        if ( ! context->UnsignedInt32(rxPageID, pageID) )
        {
            failedToRetrieveException(context->threadContext, "current category page", categoryDlg);
            goto done_out;
        }
    }
    else
    {
        if ( *pageID == 0 )
        {
            invalidCategoryPageException(context, 0, argPosPageID);
            goto done_out;
        }
    }

    RexxObjectPtr rxHwnd = context->ArrayAt(handles, *pageID);
    if ( rxHwnd == NULLOBJECT )
    {
        invalidCategoryPageException(context, *pageID, argPosPageID);
    }
    else
    {
        *hDlg = (HWND)string2pointer(context->ObjectToStringValue(rxHwnd));
        result = true;
    }

done_out:
    return result;
}


HWND getCategoryHCtrl(RexxMethodContext *context, pCPlainBaseDialog pcpbd, RexxObjectPtr rxID, uint32_t pageID)
{
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(context, pcpbd->rexxSelf);
        return NULL;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return NULL;
    }

    HWND hDlg;
    if ( ! getCategoryHDlg(context, pcpbd->rexxSelf, &pageID, &hDlg, 3) )
    {
        return NULL;
    }
    return GetDlgItem(hDlg, id);
}


/**
 * Gets the current 'category page number' for a dialog.  By definition this is
 * 0 if the dialog is not a CategoryDialog.
 *
 * This is used for things like the data connection table where it is necessary
 * to identify which 'page' a dialog control is in.
 *
 * @param c        Method context we are operating in.
 * @param oodDlg   ooDialog dialog object.
 *
 * @return The page number of the currently active dialog.  For
 *         non-CategoryDialogs, this will always be 0.
 */
uint32_t getCategoryNumber(RexxMethodContext *c, RexxObjectPtr oodDlg)
{
    // The 'category' number is 0 for all non-category dialogs;
    uint32_t category = 0;

    if ( c->IsOfType(oodDlg, "CATEGORYDIALOG") )
    {
        // Figure out the category number.  Since this *is* a category dialog,
        // there should be no way things could fail.
        RexxObjectPtr rxPageID = c->SendMessage0(oodDlg, "CURRENTCATEGORY");
        c->UnsignedInt32(rxPageID, &category);
    }
    return category;
}

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

    DLGTEMPLATE *pBase;
    if ( ! startDialogTemplate(context, &pBase, pcdd, expected, x, y, cx, cy, NULL, NULL, fontName, fontSize, style) )
    {
        return FALSE;
    }

    //  self~catalog['base'][self~catalog['category']] = base
    RexxDirectoryObject catalog = (RexxDirectoryObject)c->SendMessage0(pcpbd->rexxSelf, "CATALOG");
    RexxArrayObject     bases   = (RexxArrayObject)c->DirectoryAt(catalog, "base");
    RexxObjectPtr       rxPageID = c->DirectoryAt(catalog, "category");

    size_t i;
    c->StringSize(rxPageID, &i);
    c->ArrayPut(bases, pointer2string(context, pBase), i);

    return TRUE;
}


/** CategoryDialog::getControlDataPage()
 *
 *  Gets the 'data' from a single dialog control on a category page.
 *
 *  The original ooDialog implementation seemed to use the abstraction that the
 *  state of a dialog control was its 'data' and this abstraction influenced the
 *  naming of many of the instance methods.  I.e., getData() setData().
 *
 *  The method getControlDataPage() is, in the Rexx code, a general purpose
 *  method that replaces getCategoryValue() after 4.0.0.  getCategoryValue()
 *  forwards to getControlDataPage().  The old doc:
 *
 *  "The getValue method gets the value of a dialog item, regardless of its
 *  type. The item must have been connected before."
 *
 *  @param  rxID  The resource ID of control.
 *
 *  @return  The 'data' value of the dialog control.  This of course varies
 *           depending on the type of the dialog control.
 *
 *  @remarks  The control type is determined by the invoking method name.  When
 *            the general purpose getControlDataPage + 3 name is passed to
 *            oodName2controlType() it won't resolve and winUnknown will be
 *            returned.  This is the value that signals getControlDataPage() to
 *            do a data table look up by resource ID.  Otherwise, methods like
 *            getEditDataPage() getListBoxDataPage() etc., resolve to the proper
 *            dialog control type.  Edit, ListBox, etc..
 */
RexxMethod4(RexxObjectPtr, catdlg_getControlDataPage, RexxObjectPtr, rxID,  OPTIONAL_uint32_t, pageID,
            NAME, msgName, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(context, pcpbd->rexxSelf);
        return TheNegativeOneObj;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return TheNegativeOneObj;
    }

    HWND hDlg;
    if ( ! getCategoryHDlg(context, pcpbd->rexxSelf, &pageID, &hDlg, 2) )
    {
        return TheNegativeOneObj;
    }

    oodControl_t ctrlType = oodName2controlType(msgName + 3);

    return getControlData(context, pcpbd, id, hDlg, ctrlType);
}


/** CategoryDialog::setControlDataPage()
 *
 *  Sets the 'data' for a single dialog control on a category page.
 *
 *  @param  rxID    The resource ID of control.
 *  @param  pageID  The ID (number) of the page the control is on.  If this
 *                  argument is omitted, the current category page is assummed.
 *
 *  @return  0 for succes, -1 for a resource ID error, and 1 for other errors.
 *
 *  @see catdlg_getControlDataPage()
 */
RexxMethod5(int32_t, catdlg_setControlDataPage, RexxObjectPtr, rxID, CSTRING, data,  OPTIONAL_uint32_t, pageID,
            NAME, msgName, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(context, pcpbd->rexxSelf);
        return -1;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return -1;
    }

    HWND hDlg;
    if ( ! getCategoryHDlg(context, pcpbd->rexxSelf, &pageID, &hDlg, 3) )
    {
        return -1;
    }

    oodControl_t ctrlType = oodName2controlType(msgName + 3);

    return setControlData(context, pcpbd, id, data, hDlg, ctrlType);
}


/** CategoryDialog::sendMessageToCategoryControl()
 *
 *  Sends a window message to the specified dialog control.
 *
 *  @param  rxID    The resource ID of the control, may be numeric or symbolic.
 *
 *  @param  wm_msg  The Windows window message ID.  This can be specified in
 *                  either "0xFFFF" or numeric format.
 *
 *  @param  _wParam  The WPARAM value for the message.
 *  @param  _lParam  The LPARAM value for the message.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.  This is returned as a number
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  sendMessageToCategoryItem(), which forwards to this method, was a
 *            documented method prior to the 4.0.0 release.  Therefore,
 *            sendMessageToCategoryControl() can not raise an exception for a
 *            bad resource ID.
 *
 *            There is the feeling that the implementation of the CategoryDialog
 *            is less than ideal, so there are no plans to enhance or extend
 *            that class, no sendMessageToCategoryControlH() will be added.
 */
RexxMethod6(RexxObjectPtr, catdlg_sendMessageToCategoryControl, RexxObjectPtr, rxID, CSTRING, wm_msg,
            RexxObjectPtr, _wParam, RexxObjectPtr, _lParam, OPTIONAL_uint32_t, pageID, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNegativeOneObj;

    HWND hCtrl = getCategoryHCtrl(context, (pCPlainBaseDialog)pCSelf, rxID, pageID);
    if ( hCtrl != NULL )
    {
        result = sendWinMsgGeneric(context, hCtrl, wm_msg, _wParam, _lParam, 2, true);
    }
    return result;
}


#ifndef USE_DS_CONTROL

extern BOOL SHIFTkey = FALSE;

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



