/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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

#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <shlwapi.h>
#include <prsht.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodData.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodControl.hpp"
#include "oodMessaging.hpp"
#include "oodResourceIDs.hpp"
#include "oodResizableDialog.hpp"
#include "oodShared.hpp"
#include "oodUser.hpp"

BOOL IsNestedDialogMessage(pCPlainBaseDialog pcpbd, LPMSG lpmsg);


class LoopThreadArgs
{
public:
    DLGTEMPLATEEX     *dlgTemplate;
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

    DLGPROC dlgProc = (DLGPROC)RexxDlgProc;
    if ( pcpbd->isTabOwnerDlg )
    {
        dlgProc = (DLGPROC)RexxTabOwnerDlgProc;
    }
    else if ( pcpbd->isResizableDlg )
    {
        dlgProc = (DLGPROC)RexxResizableDlgProc;
    }

    pcpbd->hDlg = CreateDialogIndirectParam(MyInstance, (LPCDLGTEMPLATE)args->dlgTemplate, pcpbd->hOwnerDlg,
                                            dlgProc, (LPARAM)pcpbd);

    if ( pcpbd->hDlg )
    {
        pcpbd->childDlg[0] = pcpbd->hDlg;

        MSG msg;
        BOOL result;
        pcpbd->isActive = true;
        *release = true;

        if ( pcpbd->isCategoryDlg )
        {
            while ( (result = GetMessage(&msg, NULL, 0, 0)) != 0 && pcpbd->dlgAllocated )
            {
                if ( result == -1 )
                {
                    break;
                }
                if ( ! IsNestedDialogMessage(pcpbd, &msg) )
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
        else
        {
            while ( (result = GetMessage(&msg, NULL, 0, 0)) != 0 && pcpbd->dlgAllocated )
            {
                if ( result == -1 )
                {
                    break;
                }
                if ( ! IsDialogMessage(pcpbd->hDlg, &msg)  )
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
    }
    else
    {
        *release = true;
    }

    // Need to synchronize here, otherwise dlgAllocated might be true but
    // delDialog() is already running.

    EnterCriticalSection(&crit_sec);
    if ( pcpbd->dlgAllocated )
    {
        delDialog(pcpbd, pcpbd->dlgProcContext);
        pcpbd->hDlgProcThread = NULL;
    }
    LeaveCriticalSection(&crit_sec);

    if ( pcpbd->dlgProcContext != NULL )
    {
        pcpbd->dlgProcContext->DetachThread();
        pcpbd->dlgProcContext = NULL;
    }

    return 0;
}


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
 * omitted them, or used the empty string and 0, we use the default font for the
 * dialog. If the user specified the font, the default font for the dialog is
 * replaced by what the user has specified.
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
        size_t len = strlen(fontName);

        if ( len > (MAX_DEFAULT_FONTNAME - 1) )
        {
            stringTooLongException(c->threadContext, FONT_NAME_ARG_POS, MAX_DEFAULT_FONTNAME, strlen(fontName));
            return false;
        }
        if ( len > 0 )
        {
            strcpy(pcpbd->fontName, fontName);
        }
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
        if ( fontSize != 0 )
        {
            pcpbd->fontSize = fontSize;
        }
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

// Minimum size of the dialog template header, in bytes. All strings would be
// the empty string.
#define DLGTEMPLATE_HEADER_BASE_SIZE 30

// Minimum size of a control in the dialog template, in bytes. All strings would
// be the empty string. Assumes an atom is used, not a class name.
#define DLGTEMPLATE_CONTROL_BASE_SIZE 28

// Average size of a dialog control item in the dialog template.  This figure is
// simply the figure the old ooDialog code used.  How accurate it is, who knows.
#define DLGTEMPLATE_CONTROL_AVG_SIZE  256

// Number added to the dialog item count to give a little extra room.  Like the
// average control size this is just an arbitrary number from the old ooDialog
// code.
#define DLGTEMPLATE_EXTRA_FACTOR        3

#define DLGTEMPLATE_TOO_SMALL_MSG       "the storage allocated for the dialog template is too small"

inline size_t calcTemplateSize(uint32_t count)
{
    return (count + DLGTEMPLATE_EXTRA_FACTOR) * DLGTEMPLATE_CONTROL_AVG_SIZE;
}

/**
 * Calculate the actual size the dialog template header is going to use, in
 * bytes.
 *
 * @param title
 * @param fontName
 *
 * @return size_t
 *
 * @remarks title and fontName should never be null, but we'll pretend they
 *          could be.
 *
 *          The base size already includes the terminating null for strings, so
 *          we don't need to add that in.
 *
 *          The dialog class argument is never used, so we don't need to
 *          calculate a string size for that.
 */
inline size_t calcHeaderSize(const char *title , const char *fontName)
{
    size_t s = DLGTEMPLATE_HEADER_BASE_SIZE;

    s += (title == NULL ? 0 : strlen(title) * 2);
    s += (fontName == NULL ? 0 : strlen(fontName) * 2);
    return s;
}

inline size_t calcControlSize(const char *className , const char *txt)
{
    size_t s = DLGTEMPLATE_CONTROL_BASE_SIZE;

    s += (className == NULL ? 0 : strlen(className) * 2);
    s += (txt == NULL ? 0 : strlen(txt) * 2);
    return s;
}

void cleanUpDialogTemplate(void *pDlgTemplate, pCDynamicDialog pcdd)
{
    if ( pDlgTemplate != NULL )
    {
        LocalFree(pcdd->base);
    }
    pcdd->base = NULL;
    pcdd->active = NULL;
    pcdd->endOfTemplate = 0;
    pcdd->count = 0;
}


/**
 * Starts the in-memory extended dialog template using the DLGTEMPLATEEX
 * structure.
 *
 * @param c
 * @param ppBase
 * @param pcdd
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
 *
 * @remarks  The first part of the DLGTEMPLATEEX struct is fixed in length,
 *           followed by fields that are unicode strings and are variable
 *           length.  We start by filling in what we can of the struct, then
 *           when we get to the variable length stuff, we switch to using a
 *           pointer to keep track of where we are. The memory is zeroed on
 *           allocation, so we could just skip setting things to 0, but it's
 *           nice to see what fields are available.
 *
 *           We originally used 0 for exStyle because MSDN says exStyle is
 *           ignored for dialog boxes.  However, experiment has shown that that
 *           is not the case.  For example, WS_EX_TOOLWINDOW, WS_EX_CONTEXTHELP,
 *           and WS_EX_TRANSPARENT do work.  However, transparent is not much
 *           use, context help does put the question mark icon on the title bar,
 *           but does not seem to generate the WM_HELP event.  Tool window works
 *           as documented.  All in all, some investigation should be done and
 *           we could not parse for all the extended styles.
 *
 *           We set cDlgItems to the expected count. We know it is not correct,
 *           but the field is updated right before the template is actually
 *           used.
 *
 *           Both the menu and the windowClass fields can be variable length
 *           strings, but since we are using 0 we treat them as fixed length.
 *           For a menu, the field specifies the resource ID or resource name of
 *           a menu in an executable file.  To use it the menu would have to be
 *           bound to oodialog.dll.  For windowClass, 0 specifies to use the
 *           default dialog class.
 *
 *           After the dialog title, the rest is for the dialog font.  The
 *           dialog style must include DS_SETFONT or DS_SHELLFONT.  We don't
 *           check for that  because at this time we always set one or the
 *           other.  For weight, MSDN says you can use any FW_* value, but the
 *           value is always changed to FW_NORMAL. After weight are two fields,
 *           italic and character set.  For italic, true sets the font as
 *           italic.  Both italic and character set are byte sized.
 */
bool startDialogTemplate(RexxThreadContext *c, DLGTEMPLATEEX **ppBase, pCDynamicDialog pcdd,
                           int x, int y, int cx, int cy, const char *dlgClass, const char *title,
                           const char *fontName, int fontSize, uint32_t style, uint32_t exStyle)
{
    size_t s = calcTemplateSize(pcdd->expected);

    WORD *p = (PWORD)LocalAlloc(LPTR, s);
    if ( p == NULL )
    {
        outOfMemoryException(c);
        return false;
    }

    if ( calcHeaderSize(title, fontName) >  s )
    {
        cleanUpDialogTemplate(p, pcdd);
        executionErrorException(c, DLGTEMPLATE_TOO_SMALL_MSG);
        return false;
    }

    *ppBase = (DLGTEMPLATEEX *)p;

    pcdd->base = (DLGTEMPLATEEX *)p;
    pcdd->endOfTemplate = (BYTE *)p + s;

    DLGTEMPLATEEX *pDlg = (DLGTEMPLATEEX *)p;

    pDlg->dlgVer      = 0x1;            // Dialog version, must be 1.
    pDlg->signature   = 0xFFFF;         // Extended dialog template signature.
    pDlg->helpID      = 0;              // Help ID.  Not used yet
    pDlg->exStyle     = exStyle;        // Extended style.
    pDlg->style       = style;
    pDlg->cDlgItems   = pcdd->expected;
    pDlg->x           = x;
    pDlg->y           = y;
    pDlg->cx          = cx;
    pDlg->cy          = cy;
    pDlg->menu        = 0;              // 0 for no menu.
    pDlg->windowClass = 0;              // 0 to use default dialog class.

    // Now point to the title, which is variable length.
    p = (WORD *)&(pDlg->title);

    p += putUnicodeText(p, title);      // The title of the dialog.

    *p++ = fontSize;                           // Point size.
    *p++ = FW_NORMAL;                          // Weight.
    *p++ = MAKEWORD(FALSE, DEFAULT_CHARSET);   // Italic / character set.
    p += putUnicodeText(p, fontName);          // Type face name.

    // Be sure first dialog item is double word aligned.
    p = lpwAlign(p);

    // Update the active pointer to reflect where we are in the template.
    pcdd->active = p;
    return true;
}

/**
 *  Adds a dialog control item to the in-memory dialog template using
 *  the DLGITEMTEMPLATEEX structure.
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
 *
 * @TOOO  We have enhanced this to use the extended dialog template, but we need
 *        to add an extra arg so that the user can specify the extended styles.
 *        The rebar control does not behave correctly without the
 *        WS_EX_CONTROLPARENT style. So, for now, we add that manually.
 */
bool addToDialogTemplate(RexxMethodContext *c, pCDynamicDialog pcdd, SHORT kind, const char *className, int id,
                           int x, int y, int cx, int cy, const char * txt, uint32_t style)
{
   WORD *p = (WORD *)pcdd->active;

   if ( ((BYTE *)p + calcControlSize(className, txt)) > pcdd->endOfTemplate )
   {
       cleanUpDialogTemplate(p, pcdd);
       executionErrorException(c->threadContext, DLGTEMPLATE_TOO_SMALL_MSG);
       return false;
   }

   // Use the DLGITEMTEMPLATEEX struct to start off.
   DLGITEMTEMPLATEEX *pItem = (DLGITEMTEMPLATEEX *)p;

   pItem->helpID  = 0;
   pItem->exStyle = 0;
   pItem->style   = style;
   pItem->x       = x;
   pItem->y       = y;
   pItem->cx      = cx;
   pItem->cy      = cy;
   pItem->id      = (uint32_t)id;

   // Beginning at windowClass the fields are variable length.
   p = &(pItem->windowClass);

   if ( className == NULL )
   {
       *p++ = (WORD)0xffff;
       *p++ = (WORD)kind;
   }
   else
   {
       if ( strcmp(className, REBARCLASSNAME) == 0 )
       {
           pItem->exStyle = WS_EX_CONTROLPARENT;
       }
       p += putUnicodeText(p, className);
   }

   p += putUnicodeText(p, txt);

   *p++ = 0;  // extraCount, set to 0.

   // make sure the next item starts on a DWORD boundary
   p = lpwAlign(p);

   // Update the active pointer and the number of dialog items so far.
   pcdd->active = p;
   pcdd->count++;

   return true;
}


/**
 *  Methods for the .UserDialog class.
 */
#define USERDIALOG_CLASS  "UserDialog"

/**
 * Convenience function, checks if the object is a dialog object that is based
 * on using a resource script (.rc) for its dialog template.
 *
 * @param c    Method context we are operating in.
 * @param dlg  The Rexx object to check.
 *
 * @return bool
 */
inline bool isRCbasedDlg(RexxMethodContext *c, RexxObjectPtr dlg)
{
    return c->IsOfType(dlg, "RCDIALOG")         ||
           c->IsOfType(dlg, "RCCONTROLDIALOG")  ||
           c->IsOfType(dlg, "RCPSPDIALOG");
}

/** UserDialog::new()
 *
 *
 */
RexxMethod7(RexxObjectPtr, userdlg_init, OPTIONAL_RexxObjectPtr, dlgData, OPTIONAL_RexxObjectPtr, includeFile,
            OPTIONAL_RexxObjectPtr, libOrOwnerData, OPTIONAL_RexxObjectPtr, resourceID, OPTIONAL_RexxObjectPtr, ownerData,
            SUPER, super, OSELF, self)
{
    RexxArrayObject newArgs = context->NewArray(5);

    if ( argumentExists(1) )
    {
        context->ArrayPut(newArgs, dlgData, 3);
    }
    if ( argumentExists(2) )
    {
        context->ArrayPut(newArgs, includeFile, 4);
    }

    if ( isRCbasedDlg(context, self) )
    {
        if ( argumentOmitted(4) )
        {
            resourceID = context->NullString();
        }
        context->ArrayPut(newArgs, libOrOwnerData, 1);
        context->ArrayPut(newArgs, resourceID, 2);

        if ( argumentExists(5) )
        {
            context->ArrayPut(newArgs, ownerData, 5);
        }
    }
    else
    {
        context->ArrayPut(newArgs, context->NullString(), 1);
        context->ArrayPut(newArgs, TheZeroObj, 2);

        if ( argumentExists(3) )
        {
            context->ArrayPut(newArgs, libOrOwnerData, 5);
        }
    }

    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, newArgs);

    if ( result == TheZeroObj )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();
        RexxPointerObject p = context->NewPointer(pcpbd);

        result = context->SendMessage1(self, "DYNAMICINIT", p);

        if ( pcpbd->isControlDlg && result == TheZeroObj )
        {
            result = context->SendMessage1(self, "CONTROLDLGINIT", p);
        }

        if ( pcpbd->isTabOwnerDlg && result == TheZeroObj )
        {
            result = context->SendMessage1(self, "TABOWNERDLGINIT", p);
        }
    }
    else
    {
        baseClassInitializationException(context, "UserDialog", "DynamicDialog initialization has been skipped");
    }

    return result;
}

RexxMethod1(RexxObjectPtr, userdlg_test, CSELF, pCSelf)
{
    printf("No test at this time\n");
    return TheZeroObj;
}

/**
 *  Methods for the .DynamicDialog class.
 */
#define DYNAMICDIALOG_CLASS  "DynamicDialog"


/**
 * Validates that the CSelf pointer for a DynamicDialog object is not null.
 */
inline pCDynamicDialog validateDDCSelf(RexxMethodContext *c, void *pcdd)
{
    if ( pcdd == NULL )
    {
        baseClassInitializationException(c);
    }
    return (pCDynamicDialog)pcdd;
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


uint32_t ccsStyle(CSTRING opts, uint32_t style)
{
    if ( StrStrI(opts, "CCS_ADJUSTABLE"   ) != NULL ) style |= CCS_ADJUSTABLE        ;
    if ( StrStrI(opts, "CCS_BOTTOM"       ) != NULL ) style |= CCS_BOTTOM            ;
    if ( StrStrI(opts, "CCS_LEFT"         ) != NULL ) style |= CCS_LEFT              ;
    if ( StrStrI(opts, "CCS_NODIVIDER"    ) != NULL ) style |= CCS_NODIVIDER         ;
    if ( StrStrI(opts, "CCS_NOMOVEX"      ) != NULL ) style |= CCS_NOMOVEX           ;
    if ( StrStrI(opts, "CCS_NOMOVEY"      ) != NULL ) style |= CCS_NOMOVEY           ;
    if ( StrStrI(opts, "CCS_NOPARENTALIGN") != NULL ) style |= CCS_NOPARENTALIGN     ;
    if ( StrStrI(opts, "CCS_NORESIZE"     ) != NULL ) style |= CCS_NORESIZE          ;
    if ( StrStrI(opts, "CCS_RIGHT"        ) != NULL ) style |= CCS_RIGHT             ;
    if ( StrStrI(opts, "CCS_TOP"          ) != NULL ) style |= CCS_TOP               ;
    if ( StrStrI(opts, "CCS_VERT"         ) != NULL ) style |= CCS_VERT              ;
    return style;
}


uint32_t getCommonButtonStyles(uint32_t style, CSTRING opts, oodControl_t button)
{
    style |= getCommonWindowStyles(opts, false, button != winRadioButton);

    if ( StrStrI(opts, "OWNER")     != NULL ) style |= BS_OWNERDRAW;
    if ( StrStrI(opts, "BITMAP")    != NULL ) style |= BS_BITMAP;
    if ( StrStrI(opts, "ICON")      != NULL ) style |= BS_ICON;
    if ( StrStrI(opts, "LEFT")      != NULL ) style |= BS_LEFT;
    if ( StrStrI(opts, "RIGHT")     != NULL ) style |= BS_RIGHT;
    if ( StrStrI(opts, "HCENTER")   != NULL ) style |= BS_CENTER;
    if ( StrStrI(opts, "TOP")       != NULL ) style |= BS_TOP;
    if ( StrStrI(opts, "BOTTOM")    != NULL ) style |= BS_BOTTOM;
    if ( StrStrI(opts, "VCENTER")   != NULL ) style |= BS_VCENTER;
    if ( StrStrI(opts, "MULTILINE") != NULL ) style |= BS_MULTILINE;
    if ( StrStrI(opts, "NOTIFY")    != NULL ) style |= BS_NOTIFY;
    if ( StrStrI(opts, "PUSHLIKE")  != NULL ) style |= BS_PUSHLIKE;
    if ( StrStrI(opts, "FLAT")      != NULL ) style |= BS_FLAT;
    if ( StrStrI(opts, "LTEXT")     != NULL ) style |= BS_LEFTTEXT;
    if ( StrStrI(opts, "RBUTTON")   != NULL ) style |= BS_RIGHTBUTTON;

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


uint32_t reBarStyle(CSTRING opts, uint32_t style)
{
    style |= ccsStyle(opts, style);

    if ( StrStrI(opts, "AUTOSIZE"       ) != NULL ) style |= RBS_AUTOSIZE       ;
    if ( StrStrI(opts, "BANDBORDERS"    ) != NULL ) style |= RBS_BANDBORDERS    ;
    if ( StrStrI(opts, "DBLCLKTOGGLE"   ) != NULL ) style |= RBS_DBLCLKTOGGLE   ;
    if ( StrStrI(opts, "FIXEDORDER"     ) != NULL ) style |= RBS_FIXEDORDER     ;
    if ( StrStrI(opts, "REGISTERDROP"   ) != NULL ) style |= RBS_REGISTERDROP   ;
    if ( StrStrI(opts, "TOOLTIPS"       ) != NULL ) style |= RBS_TOOLTIPS       ;
    if ( StrStrI(opts, "VARHEIGHT"      ) != NULL ) style |= RBS_VARHEIGHT      ;
    if ( StrStrI(opts, "VERTICALGRIPPER") != NULL ) style |= RBS_VERTICALGRIPPER;
    return style;
}


uint32_t statusBarStyle(CSTRING opts, uint32_t style)
{
    style |= ccsStyle(opts, style);

    if ( StrStrI(opts, "SIZEGRIP")          != NULL ) style |= SBARS_SIZEGRIP;
    if ( StrStrI(opts, "TOOLTIPS")          != NULL ) style |= SBARS_TOOLTIPS;
    return style;
}


/**
 * Parses the tree-view control styles.
 *
 * @param opts
 * @param style
 *
 * @return uint32_t
 *
 * @note  The original code checked for ALL and then added the styles listed
 *        below for all, and returned.  This had the effect of INFOTIP being
 *        ignored in the keyword string: "ALL INFOTIP"  Now if we detect ALL, we
 *        then drop through and look for any other keywords.
 */
uint32_t treeViewStyle(CSTRING opts, uint32_t style)
{
    if ( StrStrI(opts,"ALL") != NULL )
    {
        style |=  TVS_EDITLABELS    | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT |
                  TVS_SHOWSELALWAYS |  WS_VSCROLL    | WS_HSCROLL;
    }

    if ( StrStrI(opts, "ATROOT"       ) != NULL ) style |= TVS_LINESATROOT;
    if ( StrStrI(opts, "BUTTONS"      ) != NULL ) style |= TVS_HASBUTTONS;
    if ( StrStrI(opts, "CHECKBOXES"   ) != NULL ) style |= TVS_CHECKBOXES;
    if ( StrStrI(opts, "EDIT"         ) != NULL ) style |= TVS_EDITLABELS;
    if ( StrStrI(opts, "FULLROWSELECT") != NULL ) style |= TVS_FULLROWSELECT;
    if ( StrStrI(opts, "HSCROLL"      ) != NULL ) style |= WS_HSCROLL;
    if ( StrStrI(opts, "INFOTIP"      ) != NULL ) style |= TVS_INFOTIP;
    if ( StrStrI(opts, "LINES"        ) != NULL ) style |= TVS_HASLINES;
    if ( StrStrI(opts, "NODRAG"       ) != NULL ) style |= TVS_DISABLEDRAGDROP;
    if ( StrStrI(opts, "NOHSCROLL"    ) != NULL ) style |= TVS_NOHSCROLL;
    if ( StrStrI(opts, "NONEVENHEIGHT") != NULL ) style |= TVS_NONEVENHEIGHT;
    if ( StrStrI(opts, "NOSCROLL"     ) != NULL ) style |= TVS_NOSCROLL;
    if ( StrStrI(opts, "NOTOOLTIPS"   ) != NULL ) style |= TVS_NOTOOLTIPS;
    if ( StrStrI(opts, "RTLREADING"   ) != NULL ) style |= TVS_RTLREADING;
    if ( StrStrI(opts, "SHOWSELALWAYS") != NULL ) style |= TVS_SHOWSELALWAYS;
    if ( StrStrI(opts, "SINGLEEXPAND" ) != NULL ) style |= TVS_SINGLEEXPAND;
    if ( StrStrI(opts, "TRACKSELECT"  ) != NULL ) style |= TVS_TRACKSELECT;
    if ( StrStrI(opts, "VSCROLL"      ) != NULL ) style |= WS_VSCROLL;
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


uint32_t toolBarStyle(CSTRING opts, uint32_t style)
{
    style |= ccsStyle(opts, style);

    if ( StrStrI(opts, "TOOLTIPS"         ) != NULL ) style |= TBSTYLE_TOOLTIPS      ;
    if ( StrStrI(opts, "WRAPABLE"         ) != NULL ) style |= TBSTYLE_WRAPABLE      ;
    if ( StrStrI(opts, "ALTDRAG"          ) != NULL ) style |= TBSTYLE_ALTDRAG       ;
    if ( StrStrI(opts, "FLAT"             ) != NULL ) style |= TBSTYLE_FLAT          ;
    if ( StrStrI(opts, "LIST"             ) != NULL ) style |= TBSTYLE_LIST          ;
    if ( StrStrI(opts, "CUSTOMERASE"      ) != NULL ) style |= TBSTYLE_CUSTOMERASE   ;
    if ( StrStrI(opts, "REGISTERDROP"     ) != NULL ) style |= TBSTYLE_REGISTERDROP  ;
    if ( StrStrI(opts, "TRANSPARENT"      ) != NULL ) style |= TBSTYLE_TRANSPARENT   ;
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

    if ( StrStrI(opts, "CANPARSE") != NULL ) style |= DTS_APPCANPARSE;
    if ( StrStrI(opts, "RIGHT"   ) != NULL ) style |= DTS_RIGHTALIGN;
    if ( StrStrI(opts, "SHOWNONE") != NULL ) style |= DTS_SHOWNONE;
    if ( StrStrI(opts, "UPDOWN"  ) != NULL ) style |= DTS_UPDOWN;
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


extern uint32_t getControlStyle(oodControl_t ctrl, CSTRING opts)
{
    uint32_t style = WS_CHILD;

    switch ( ctrl )
    {
        case winDateTimePicker :
            style |= getCommonWindowStyles(opts, false, true);
            style = dateTimePickerStyle(opts, style);
            break;

        case winListView :
            style |= getCommonWindowStyles(opts, true, true);
            style = listViewStyle(opts, style);
            break;

        case winMonthCalendar :
            style |= getCommonWindowStyles(opts, false, true);
            style = monthCalendarStyle(opts, style);
            break;

        case winProgressBar :
            style |= getCommonWindowStyles(opts, false, false);
            style = progressBarStyle(opts, style);
            break;

        case winReBar :
            style |= getCommonWindowStyles(opts, false, false);
            style = reBarStyle(opts, style);
            break;

        case winStatusBar :
            style |= getCommonWindowStyles(opts, false, false);
            style = statusBarStyle(opts, style);
            break;

        case winTab :
            style |= getCommonWindowStyles(opts, false, true);
            style = tabStyle(opts, style);
            break;

        case winToolBar :
            style |= getCommonWindowStyles(opts, true, true);
            style = toolBarStyle(opts, style);
            break;

        case winTreeView :
            style |= getCommonWindowStyles(opts, true, true);
            style = treeViewStyle(opts, style);
            break;

        case winTrackBar :
            style |= getCommonWindowStyles(opts, false, true);
            style = trackBarStyle(opts, style);
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

/**
 * Determines if one of the keyword options for loading the dialog template from
 * a .rc file is contained in the specified string
 *
 * @param opts
 *
 * @return bool
 */
inline bool isRcLoadItemsArg(CSTRING opts)
{
    if ( opts != NULL )
    {
        if ( StrStrI(opts, "CENTER")         != NULL ||
             StrStrI(opts, "CONNECTBUTTONS") != NULL ||
             StrStrI(opts, "CONNECTCHECKS")  != NULL ||
             StrStrI(opts, "CONNECTRADIOS")  != NULL )
        {
            return true;
        }
    }
    return false;
}

/**
 * Check if the word 'CENTER', case insignificant, is in a string.  Note that
 * CENTER either has to have a space after it, or it has to be the end of the
 * string.
 *
 * @param opts  The string to check.
 *
 * @return True or false.
 */
static bool hasCenterFlag(CSTRING opts)
{
    char *p = StrStrI(opts, "CENTER");
    while ( p != NULL )
    {
        p += 6;
        if ( *p == ' ' || *p == '\0' )
        {
            return true;
        }
        p = StrStrI(p, "CENTER");
    }
    return false;
}


bool fillInButtonUsingIndex(RexxMethodContext *c, RexxStemObject inp, pButtonData btn, uint32_t i, RexxObjectPtr rexxSelf)
{
    char buf[256] = {'\0'};

    _snprintf(buf, sizeof(buf), "%d.ID", i);

    RexxObjectPtr value = c->GetStemElement(inp, buf);
    if ( value == NULLOBJECT )
    {
        missingIndexInStemException(c->threadContext, 5, "ID");
        return false;
    }

    int32_t id = checkID(c, value, rexxSelf);
    if ( id < 1 )
    {
        return false;
    }
    btn->id = (uint32_t)id;

    _snprintf(buf, sizeof(buf), "%d.TEXT", i);

    value = c->GetStemElement(inp, buf);
    btn->text = (value == NULLOBJECT) ? "" : c->ObjectToStringValue(value);

    _snprintf(buf, sizeof(buf), "%d.METHOD", i);

    value = c->GetStemElement(inp, buf);
    btn->methName = (value == NULLOBJECT) ? "" : c->ObjectToStringValue(value);

    uint32_t style = WS_CHILD;
    CSTRING  opts  = NULL;
    _snprintf(buf, sizeof(buf), "%d.OPTS", i);

    value = c->GetStemElement(inp, buf);
    if ( value == NULLOBJECT )
    {
        style |= BS_PUSHBUTTON;
        opts = "";
    }
    else
    {
        opts = c->ObjectToStringValue(value);
        style |= ( StrStrI(opts, "DEFAULT") != NULL ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON );
    }
    btn->opts = getCommonButtonStyles(style, opts, winPushButton);

    return true;
}

bool fillInButtonFromStem(RexxMethodContext *c, RexxStemObject s, pButtonData btn, RexxObjectPtr rexxSelf)
{
    RexxObjectPtr value = c->GetStemElement(s, "ID");
    if ( value == NULLOBJECT )
    {
        missingIndexInStemException(c->threadContext, 5, "ID");
        return false;
    }

    int32_t id = checkID(c, value, rexxSelf);
    if ( id < 1 )
    {
        return false;
    }
    btn->id = (uint32_t)id;

    value = c->GetStemElement(s, "TEXT");
    btn->text = (value == NULLOBJECT) ? "" : c->ObjectToStringValue(value);

    value = c->GetStemElement(s, "METHOD");
    btn->methName = (value == NULLOBJECT) ? "" : c->ObjectToStringValue(value);

    uint32_t style = WS_CHILD;
    CSTRING  opts  = NULL;

    value = c->GetStemElement(s, "OPTS");
    if ( value == NULLOBJECT )
    {
        style |= BS_PUSHBUTTON;
        opts = "";
    }
    else
    {
        CSTRING opts = c->ObjectToStringValue(value);
        style |= ( StrStrI(opts, "DEFAULT") != NULL ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON );
    }
    style = getCommonButtonStyles(style, opts, winPushButton);

    btn->opts = style;

    return true;
}

pButtonData *parseButtonStem(RexxMethodContext *c, uint32_t count, RexxStemObject inp, RexxObjectPtr rexxSelf)
{
    pButtonData *result = NULL;
    size_t       used   = 0;

    result = (pButtonData *)LocalAlloc(LPTR, count * sizeof(pButtonData *));
    if ( result == NULL )
    {
        outOfMemoryException(c->threadContext);
        goto err_out;
    }

    for ( uint32_t i = 1; i <= count; i++)
    {
        pButtonData btn = (pButtonData)LocalAlloc(LPTR, sizeof(ButtonData));
        if ( btn == NULL )
        {
            outOfMemoryException(c->threadContext);
            goto err_out;
        }

        result[i - 1] = btn;
        used++;

        RexxStemObject s = (RexxStemObject)c->GetStemArrayElement(inp, i);
        if ( s != NULLOBJECT )
        {
            if ( ! fillInButtonFromStem(c, s, btn, rexxSelf) )
            {
                goto err_out;
            }
        }
        else
        {
            if ( ! fillInButtonUsingIndex(c, inp, btn, i, rexxSelf) )
            {
                goto err_out;
            }
        }
    }

    return result;

err_out:

    for ( size_t i = 0; i < used; i++)
    {
        LocalFree(result[i]);
    }
    safeLocalFree(result);
    return NULL;
}

/**
 * Checks if the type of control is one that has "data".
 *
 * This is related to the original ooDialog developer's odd notion that a dialog
 * control had "data" and therefore a "data attribute" made sense.
 *
 *
 * @param ctrl
 *
 * @return bool
 */
inline bool controlHasData(oodControl_t ctrl)
{
    return ! (ctrl == winReBar || ctrl == winStatusBar) || ctrl == winToolBar || ctrl == winToolTip;
}

int32_t connectCreatedControl(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr rxID, int32_t id,
                              CSTRING attributeName, oodControl_t ctrl)
{
    char buf[64];
    if ( attributeName == NULL || *attributeName == '\0' )
    {
        _snprintf(buf, sizeof(buf), "DATA%d", id);
        attributeName = buf;
    }

    uint32_t category = getCategoryNumber(c, pcpbd->rexxSelf);

    c->SendMessage2(pcpbd->rexxSelf, "ADDATTRIBUTE", rxID, c->String(attributeName));

    uint32_t result = addToDataTable(c->threadContext, pcpbd, id, ctrl, category);
    if ( result == OOD_MEMORY_ERR )
    {
        return -2;
    }
    return (int32_t)result;
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
        // If a static text control has none of these styles, then by default it
        // is SS_LEFT.  (SS_LEFT == 0)
        if ( hasCenterFlag(opts) )                      style |= SS_CENTER;
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

    if ( ! addToDialogTemplate(c, pcdd, StaticAtom, NULL, id, x, y, cx, cy, text, style) )
    {
        return -2;
    }
    int32_t result = 0;

    // Connect the data attribute if we need to, and the id is not IDC_STATIC.
    // This is done to fix a bug where radio buttons within a group are not
    // detected if the user has given the group style to a static label, and
    // given the label a resource ID.
    //
    // However, this has the potential of breaking old programs because
    // originally no static control was added to the data table.  Now, by adding
    // it, the text will get set to "" if auto detection is on and the attribute
    // is not set to some text. (Which it will be in old programs.)  Not only
    // that, but if the user is using the dialog data stem, the dialog data stem
    // will set the static text to "".
    //
    // We actually need to do several things here, get the attribute name and
    // set it to the text of static control, check if the dialog data attribute
    // is being used, and if so set the stem at id (i.e. dlgData.2008) to the
    // text. This is a case where it is actually eaiser done in Rexx code.  So,
    // we have an internal use only method, setStaticTextAttribute() of the
    // UserDialog, and let it do the work.
    //
    // This also makes auto detection work more consistently.

    if ( pcpbd->autoDetect && id != -1 )
    {
        result = connectCreatedControl(c, pcpbd, rxID, id, NULL, winStatic);
        if ( result == OOD_NO_ERROR )
        {
            c->SendMessage2(pcpbd->rexxSelf, "SETSTATICTEXTATTRIBUTE", c->Int32(id), c->String(text));
        }
    }
    return result;
}


int32_t createStaticImage(RexxMethodContext *c, RexxObjectPtr rxID, int x, int y, uint32_t cx, uint32_t cy,
                         CSTRING opts, pCDynamicDialog pcdd)
{
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    uint32_t id = checkID(c, rxID, pcpbd->rexxSelf);
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

    if ( ! addToDialogTemplate(c, pcdd, StaticAtom, NULL, id, x, y, cx, cy, NULL, style) )
    {
        return -2;
    }

    // Connect the data attribute if we need to, and the id is not IDC_STATIC.
    // This is done so that setting radio buttons will work if the user has
    // given the group style to the static image, which is possible but probably
    // not likely.  We have no attribute name so we pass in null
    int32_t result = 0;
    if ( pcpbd->autoDetect && id != -1 )
    {
        result = connectCreatedControl(c, pcpbd, rxID, id, NULL, winStatic);
    }
    return result;
}


int32_t createStaticFrame(RexxMethodContext *c, RexxObjectPtr rxID, int x, int y, uint32_t cx, uint32_t cy,
                         CSTRING opts, CSTRING type, uint32_t frameStyle, pCDynamicDialog pcdd)
{
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    int32_t id = checkID(c, rxID, pcpbd->rexxSelf);
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
            else style |= SS_WHITERECT;
        }
    }

    if ( StrStrI(opts, "NOTIFY") != NULL ) style |= SS_NOTIFY;
    if ( StrStrI(opts, "SUNKEN") != NULL ) style |= SS_SUNKEN;

    if ( ! addToDialogTemplate(c, pcdd, StaticAtom, NULL, id, x, y, cx, cy, NULL, style) )
    {
        return -2;
    }
    int32_t result = 0;

    // Connect the data attribute if we need to, and the id is not IDC_STATIC.
    // This is done so that setting radio buttons will work if the user has
    // given the group style to the static frame, which is possible but probably
    // not likely. We have no attribute name so we pass in null
    if ( pcpbd->autoDetect && id != -1 )
    {
        result = connectCreatedControl(c, pcpbd, rxID, id, NULL, winStatic);
    }
    return result;
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
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd != NULL )
    {
        return pointer2string(context, pcdd->base);
    }
    return NULLOBJECT;
}
RexxMethod2(RexxObjectPtr, dyndlg_setBasePtr, CSTRING, ptrStr, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd != NULL )
    {
        pcdd->base = (DLGTEMPLATEEX *)string2pointer(ptrStr);
    }
    return NULLOBJECT;
}

/** DynamicDialog::activePtr  [attribute private]
 */
RexxMethod1(RexxObjectPtr, dyndlg_getActivePtr, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd != NULL )
    {
        return pointer2string(context, pcdd->active);
    }
    return NULLOBJECT;
}
RexxMethod2(RexxObjectPtr, dyndlg_setActivePtr, CSTRING, ptrStr, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd != NULL )
    {
        pcdd->active = string2pointer(ptrStr);
    }
    return NULLOBJECT;
}

/** DynamicDialog::dialogItemCount  [attribute private]
 */
RexxMethod1(uint32_t, dyndlg_getDialogItemCount, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd != NULL )
    {
        return pcdd->count;
    }
    return NULLOBJECT;
}
RexxMethod2(RexxObjectPtr, dyndlg_setDialogItemCount, uint32_t, count, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd != NULL )
    {
        pcdd->count = count;
    }
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
        return TheOneObj;
    }

    pCDynamicDialog pcdd = (pCDynamicDialog)context->BufferData(pddBuffer);
    memset(pcdd, 0, sizeof(CDynamicDialog));

    pcdd->pcpbd = pcpbd;
    pcdd->rexxSelf = self;
    context->SetObjectVariable("CSELF", pddBuffer);

    return TheZeroObj;
}

/**
 *
 * @param  x          ( 1 required) X co-ordinate
 * @param  y          ( 2 required) Y co-ordinate
 * @param  cx         ( 3 required) width
 * @param  cy         ( 4 required) height
 * @param  title      ( 5 optional) Title for the caption bar
 * @param  opts       ( 6 optional) Style 0ptions for the dialog
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
 *
 *           For a resizable dialog (inherits ResizingAdmin) we add the needed
 *           styles to force the dialog to be resizable. Testing seems to show
 *           that adding WS_THICKFRAME during WM_INITDIALOG does not work if
 *           WS_MAXIMIZEBOX is not used in the dialog template.  But, if
 *           WS_THICKFRAME is added to the dialog template, WS_MAXIMIZEBOX is
 *           not needed.  So we do that here.
 */
RexxMethod9(logical_t, dyndlg_create, uint32_t, x, int32_t, y, int32_t, cx, uint32_t, cy, OPTIONAL_CSTRING, title,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, dlgClass, ARGLIST, args, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return FALSE;
    }

    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    dlgClass = NULL;        // The dialog class is always ignored, at this time.

    uint32_t style = DS_SETFONT | WS_CAPTION | WS_SYSMENU;
    if ( pcpbd->isControlDlg )
    {
        style = DS_SETFONT | DS_CONTROL | WS_CHILD;
    }

    if ( pcpbd->isResizableDlg )
    {
        style |= WS_THICKFRAME;
    }

    uint32_t exStyle = 0;
    if ( pcpbd->isTabOwnerDlg )
    {
        exStyle = WS_EX_CONTROLPARENT;
    }

    if ( argumentOmitted(5) )
    {
        title = "";
    }

    if ( argumentExists(6) )
    {
        if ( pcpbd->isControlDlg || StrStrI(opts, "CONTROL") != NULL )
        {
            // The correct style for a ControlDialog is set above.  Here we just
            // want to catch the "CONTROL" keyword and make sure the user does
            // not set the wrong style for a ControlDialog.
            if ( ! pcpbd->isControlDlg )
            {
                style = DS_SETFONT | DS_CONTROL;
                if ( StrStrI(opts, "NOTCHILD") == NULL )
                {
                    style |= WS_CHILD;
                }
            }
        }
        else
        {
            if ( StrStrI(opts, "NOMENU")       != NULL ) style &= ~WS_SYSMENU;
            if ( StrStrI(opts, "NOTMODAL")     == NULL ) style |= DS_MODALFRAME;
            if ( StrStrI(opts, "SYSTEMMODAL")  != NULL ) style |= DS_SYSMODAL;
            if ( StrStrI(opts, "CENTER")       != NULL ) style |= DS_CENTER;
            if ( StrStrI(opts, "THICKFRAME")   != NULL ) style |= WS_THICKFRAME;
            if ( StrStrI(opts, "MINIMIZEBOX")  != NULL ) style |= WS_MINIMIZEBOX;
            if ( StrStrI(opts, "MAXIMIZEBOX")  != NULL ) style |= WS_MAXIMIZEBOX;
            if ( StrStrI(opts, "VSCROLL")      != NULL ) style |= WS_VSCROLL;
            if ( StrStrI(opts, "HSCROLL")      != NULL ) style |= WS_HSCROLL;
            if ( StrStrI(opts, "OVERLAPPED")   != NULL ) style |= WS_OVERLAPPED;
            if ( StrStrI(opts, "POPUP")        != NULL ) style |= WS_POPUP;
            if ( StrStrI(opts, "CLIBSIBLINGS") != NULL ) style |= WS_CLIPSIBLINGS;
        }

        if ( StrStrI(opts, "VISIBLE") != NULL )
        {
            style |= WS_VISIBLE;
        }

        // Allow extended styles.  We don't check that the style makes sense for
        // a dialog box, we just let the user assign them if they want.  At some
        // point we could determine if any of the styles should not be used.

        if ( StrStrI(opts, "EX_ACCEPTFILES")      != NULL ) exStyle |= WS_EX_ACCEPTFILES;
        if ( StrStrI(opts, "EX_APPWINDOW")        != NULL ) exStyle |= WS_EX_APPWINDOW;
        if ( StrStrI(opts, "EX_CLIENTEDGE")       != NULL ) exStyle |= WS_EX_CLIENTEDGE;
        if ( StrStrI(opts, "EX_CONTEXTHELP")      != NULL ) exStyle |= WS_EX_CONTEXTHELP;
        if ( StrStrI(opts, "EX_CONTROLPARENT")    != NULL ) exStyle |= WS_EX_CONTROLPARENT;
        if ( StrStrI(opts, "EX_DLGMODALFRAME")    != NULL ) exStyle |= WS_EX_DLGMODALFRAME;
        if ( StrStrI(opts, "EX_LEFTSCROLLBAR")    != NULL ) exStyle |= WS_EX_LEFTSCROLLBAR;
        else if ( StrStrI(opts, "EX_LEFT")        != NULL ) exStyle |= WS_EX_LEFT;
        if ( StrStrI(opts, "EX_LTRREADING")       != NULL ) exStyle |= WS_EX_LTRREADING;
        if ( StrStrI(opts, "EX_MDICHILD")         != NULL ) exStyle |= WS_EX_MDICHILD;
        if ( StrStrI(opts, "EX_NOPARENTNOTIFY")   != NULL ) exStyle |= WS_EX_NOPARENTNOTIFY;
        if ( StrStrI(opts, "EX_OVERLAPPEDWINDOW") != NULL ) exStyle |= WS_EX_OVERLAPPEDWINDOW;
        if ( StrStrI(opts, "EX_PALETTEWINDOW")    != NULL ) exStyle |= WS_EX_PALETTEWINDOW;
        if ( StrStrI(opts, "EX_RIGHTSCROLLBAR")   != NULL ) exStyle |= WS_EX_RIGHTSCROLLBAR;
        else if ( StrStrI(opts, "EX_RIGHT")       != NULL ) exStyle |= WS_EX_RIGHT;
        if ( StrStrI(opts, "EX_RTLREADING")       != NULL ) exStyle |= WS_EX_RTLREADING;
        if ( StrStrI(opts, "EX_STATICEDGE")       != NULL ) exStyle |= WS_EX_STATICEDGE;
        if ( StrStrI(opts, "EX_TOOLWINDOW")       != NULL ) exStyle |= WS_EX_TOOLWINDOW;
        if ( StrStrI(opts, "EX_TOPMOST")          != NULL ) exStyle |= WS_EX_TOPMOST;
        if ( StrStrI(opts, "EX_TRANSPARENT")      != NULL ) exStyle |= WS_EX_TRANSPARENT;
        if ( StrStrI(opts, "EX_WINDOWEDGE")       != NULL ) exStyle |= WS_EX_WINDOWEDGE;
    }

    if ( ! adjustDialogFont(context, args, pcpbd) )
    {
        goto err_out;
    }

    pcdd->expected = getExpectedCount(context, args);
    if ( pcdd->expected == 0 )
    {
        goto err_out;
    }

    // We need to pass in and set the base address separately because the
    // category diaogs also use startDialogTemplate() and they need to keep
    // track of the different child dialog base addresses.
    DLGTEMPLATEEX *pBase;

    if ( ! startDialogTemplate(context->threadContext, &pBase, pcdd, x, y, cx, cy, dlgClass, title,
                               pcpbd->fontName, pcpbd->fontSize, style, exStyle) )
    {
       goto err_out;
    }

    pcpbd->wndBase->sizeX = cx;
    pcpbd->wndBase->sizeY = cy;

    RexxObjectPtr result = context->SendMessage0(pcdd->rexxSelf, "DEFINEDIALOG");

    // If it is a category dialog, the underlying dialog(s) have already been
    // created and the template cleaned up ... need to work on this some.
    if ( context->IsOfType(pcpbd->rexxSelf, "CATEGORYDIALOG") )
    {
        return TRUE;
    }

    if ( pcdd->active != NULL )
    {
        return TRUE;
    }

err_out:
    // No underlying windows dialog is created, but we still need to clean up
    // the CSelf struct, which was allocated when the Rexx dialog object was
    // instantiated.  This admin block is now in the DialogTable.
    EnterCriticalSection(&crit_sec);
    delDialog(pcpbd, context->threadContext);
    LeaveCriticalSection(&crit_sec);

    return FALSE;
}


/** DyamicDialog::startParentDialog()
 *
 *  Creates the underlying Windows dialog for a user dialog (or one of its
 *  subclasses) object.  This is the counterpart to the ResDialog::startDialog()
 *  which is only used to create the underlying Windows dialog for ResDialog
 *  dialogs.
 *
 */
RexxMethod3(logical_t, dyndlg_startParentDialog, uint32_t, iconID, logical_t, modeless, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return FALSE;
    }

    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    DLGTEMPLATEEX *p = pcdd->base;
    if ( p == NULL )
    {
        return illegalBuffer();
    }

    if ( pcpbd->isOwnedDlg )
    {
        modeless = TRUE;
        if ( ! processOwnedDialog(context, pcpbd) )
        {
            return false;
        }
    }

    ULONG thID;
    bool Release = false;

    // Set the number of dialog items field in the dialog template.
    p->cDlgItems = (WORD)pcdd->count;

    EnterCriticalSection(&crit_sec);

    // installNecessaryStuff() can not fail for a UserDialog.
    installNecessaryStuff(pcpbd, NULL);

    LoopThreadArgs threadArgs;
    threadArgs.dlgTemplate = p;
    threadArgs.pcpbd = pcpbd;
    threadArgs.release = &Release;

    pcpbd->hDlgProcThread = CreateThread(NULL, 2000, (LPTHREAD_START_ROUTINE)WindowUsrLoopThread, &threadArgs, 0, &thID);

    // Wait for the dialog to start, don't wait if the thread was not created.
    while ( ! Release && pcpbd->hDlgProcThread )
    {
        Sleep(1);
    }
    LeaveCriticalSection(&crit_sec);

    // Free the memory allocated for template.
    cleanUpDialogTemplate(p, pcdd);

    if ( pcpbd->hDlg )
    {
        setDlgHandle(pcpbd);

        // Set the thread priority higher for faster drawing.
        SetThreadPriority(pcpbd->hDlgProcThread, THREAD_PRIORITY_ABOVE_NORMAL);
        pcpbd->onTheTop = true;
        pcpbd->dlgProcThreadID = thID;

        // Do we have a modal dialog?
        checkModal((pCPlainBaseDialog)pcpbd->previous, modeless);

        if ( GetWindowLong(pcpbd->hDlg, GWL_STYLE) & WS_SYSMENU )
        {
            HICON hBig = NULL;
            HICON hSmall = NULL;

            if ( getDialogIcons(pcpbd, iconID, ICON_FILE, (PHANDLE)&hBig, (PHANDLE)&hSmall) )
            {
                pcpbd->sysMenuIcon = (HICON)setClassPtr(pcpbd->hDlg, GCLP_HICON, (LONG_PTR)hBig);
                pcpbd->titleBarIcon = (HICON)setClassPtr(pcpbd->hDlg, GCLP_HICONSM, (LONG_PTR)hSmall);
                pcpbd->didChangeIcon = true;

                SendMessage(pcpbd->hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hSmall);
            }
        }

        return TRUE;
    }

    // The dialog creation failed, return falls.  When the dialog creation fails
    // in the WindowUsrLoop thread a delDialog() is immediately done, as it
    // fails to enter the message processing loop.
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
 *  @remarks  TODO these comments need to be rewritten, they are very out of
 *            date and inaccurate.
 *
 *            The child dialog needs to be created in the window procedure
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
    RexxObjectPtr result = TheZeroObj;

    DLGTEMPLATEEX *p = (DLGTEMPLATEEX *)basePtr;
    if ( p == NULL )
    {
        illegalBuffer();
        goto done_out;
    }

    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        goto done_out;
    }

    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( pcpbd->isControlDlg && ! pcpbd->isManagedDlg && ! validControlDlg(context, pcpbd) )
    {
        goto done_out;
    }

    // Set the field for the number of dialog controls in the dialog template.
    p->cDlgItems = (WORD)pcdd->count;

    HWND hChild;
    if ( pcpbd->isControlDlg )
    {
        hChild = (HWND)SendMessage(pcpbd->hOwnerDlg, WM_USER_CREATECONTROL_DLG, (WPARAM)pcpbd, (LPARAM)p);
        if ( hChild )
        {
            pcpbd->hDlg = hChild;
            setDlgHandle(pcpbd);
            pcpbd->isActive = true;
            ((pCControlDialog)pcpbd->dlgPrivate)->activated = true;
        }
    }
    else
    {
        hChild = (HWND)SendMessage(pcpbd->hDlg, WM_USER_CREATECHILD, 0, (LPARAM)p);
    }

    // The child dialog may not have been created.
    if ( hChild == NULL )
    {
        goto done_out;
    }

    pcpbd->childDlg[childIndex] = hChild;
    result = pointer2string(context, hChild);

done_out:
    // Free the memory allocated for template.  This is safe if p is null.
    cleanUpDialogTemplate(p, pcdd);

    return result;
}


/** DynamicDialog::createStatic()
 *
 */
RexxMethod8(int32_t, dyndlg_createStatic, OPTIONAL_RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, text, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }

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
        return createStaticText(context, rxID, x, y, cx, cy, opts, text, pcdd);
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
        return createStaticFrame(context, rxID, x, y, cx, cy, opts, NULL, frameStyle, pcdd);
    }

    return createStaticText(context, rxID, x, y, cx, cy, opts, text, pcdd);
}


/** DynamicDialog::createStaticText()
 *
 */
RexxMethod8(int32_t, dyndlg_createStaticText, OPTIONAL_RexxObjectPtr, rxID,
            int, x, int, y, OPTIONAL_uint32_t, cx, OPTIONAL_uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, text, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }

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
    return createStaticText(context, rxID, x, y, cx, cy, opts, text, pcdd);
}


/** DynamicDialog::createStaticImage()
 *
 */
RexxMethod7(int32_t, dyndlg_createStaticImage, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }

    if ( argumentOmitted(6) )
    {
        opts = "";
    }
    return createStaticImage(context, rxID, x, y, cx, cy, opts, pcdd);
}


/** DynamicDialog::createStaticFrame()
 *
 */
RexxMethod8(int32_t, dyndlg_createStaticFrame, OPTIONAL_RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, NAME, msgName, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }

    if ( argumentOmitted(1) )
    {
        rxID = TheNegativeOneObj;
    }
    if ( argumentOmitted(6) )
    {
        opts = "";
    }
    return createStaticFrame(context, rxID, x, y, cx, cy, opts, msgName + 6, 0, pcdd);
}

/** DynamicDialog::createPushButton()
 *
 */
RexxMethod10(int32_t, dyndlg_createPushButton, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
             OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, label, OPTIONAL_CSTRING, msgToRaise, OPTIONAL_CSTRING, loadOptions,
             CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }
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

    if ( ! addToDialogTemplate(context, pcdd, ButtonAtom, NULL, id, x, y, cx, cy, label, style) )
    {
        return -2;
    }

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
    else if ( argumentExists(8) && ! isRcLoadItemsArg(loadOptions) )
    {
        methName = strdup_nospace(msgToRaise);
    }

    if ( methName != NULL && strlen(methName) != 0 )
    {
        result = addCommandMessage(pcdd->pcpbd->enCSelf, context, id, UINTPTR_MAX, 0, 0, methName, 0) ? 0 : 1;
    }

    free((void *)methName);
    return result;
}

/** DynamicDialog::createPushButtonStem()
 *
 *  Creates a group of push buttons using data supplied in a Stem object.
 *
 *  @param   x   [optional]  X co-ordinate of upper left corner of the 1st push
 *               button in the group.  If omitted, or less than 0 this will be
 *               calculated.
 *
 *  @param   y   [optional]  Y co-ordinate of upper left corner of the 1st push
 *               button in the group. If omitted, or less than 0 this will be
 *               calculated.
 *
 *  ...
 *
 */
RexxMethod8(int32_t, dyndlg_createPushButtonStem, OPTIONAL_int32_t, x, OPTIONAL_int32_t, y, OPTIONAL_int32_t, cx,
             OPTIONAL_int32_t, cy, RexxStemObject, inp, OPTIONAL_logical_t, row, OPTIONAL_CSTRING, leftTop, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }

    pButtonData *dataArray = NULL;
    int32_t      rc        = -2;

    if ( pcdd->active == NULL )
    {
        goto done_out;
    }

    uint32_t      count;
    RexxObjectPtr _count = c->GetStemArrayElement(inp, 0);
    if ( _count == NULLOBJECT || ! c->UnsignedInt32(_count, &count) )
    {
        stemIndexZeroException(context, 5);
        goto done_out;
    }

    dataArray = parseButtonStem(context, count, inp, pcdd->pcpbd->rexxSelf);
    if ( dataArray == NULL )
    {
        goto done_out;
    }

    bool bottom = true;
    bool right  = true;
    if ( argumentExists(7) )
    {
        if ( StrStrI(leftTop, "LEFT") != NULL )  right  = false;
        if ( StrStrI(leftTop, "TOP")  != NULL )  bottom = false;
    }

    bool calcXY  = (argumentOmitted(1) || argumentOmitted(2) || x <  0 || y <  0) ? true : false;
    bool calcCXY = (argumentOmitted(2) || argumentOmitted(3) || cx < 0 || cy < 0) ? true : false;

    // Maybe do some calculations
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    cx = 50;
    cy = 14;
    if ( calcCXY )
    {
        int32_t maxCX = 0;
        int32_t maxCY = 0;
        SIZE    s     = {0};

        for ( uint32_t i = 0; i < count; i++ )
        {
            pButtonData btn = dataArray[i];

            // We just ignore errors here, almost impossible to happen anyway.
            if ( pcpbd->hDlg == NULL )
            {
                getTextSizeDuInactiveDlg(context, pcpbd, btn->text, &s);
            }
            else
            {
                getTextSizeDuActiveDlg(context, pcpbd, btn->text, &s);
            }
            maxCX = (s.cx > maxCX) ? s.cx : maxCX;
            maxCY = (s.cy > maxCY) ? s.cy : maxCY;
        }

        maxCX += 6;
        maxCY += 6;

        cx = (maxCX > cx) ? maxCX : cx;
        cy = (maxCY > cy) ? maxCY : maxCY;
    }

    if ( calcXY )
    {
        if ( row )
        {
            if ( argumentOmitted(2) )
            {
                if ( bottom )
                {
                    y = pcpbd->wndBase->sizeY - 7 - cy;
                }
                else
                {
                    y = 7;
                }
            }
            if ( right )
            {
                x = pcpbd->wndBase->sizeX - 7 - (count * cx) - ((count - 1) * 4);
            }
            else
            {
                x = 7;
            }
        }
        else
        {
            if ( bottom )
            {
                y = pcpbd->wndBase->sizeY - 7 - (count * cy) - ((count - 1) * 4);
            }
            else
            {
                y = 7;
            }
            if ( argumentOmitted(1) )
            {
                if ( right )
                {
                    x = pcpbd->wndBase->sizeX - 7 - cx;
                }
                else
                {
                    x = 7;
                }
            }
        }
    }

    for ( uint32_t i = 0; i < count; i++ )
    {
        pButtonData btn = dataArray[i];

        if ( ! addToDialogTemplate(context, pcdd, ButtonAtom, NULL, btn->id, x, y, cx, cy, btn->text, btn->opts) )
        {
            goto done_out;
        }

        if ( row )
        {
            x += cx + 4;
        }
        else
        {
            y += cy + 4;
        }

        if ( btn->id == IDCANCEL || btn->id == IDHELP || strlen(btn->methName) == 0 )
        {
            continue;
        }

        if ( ! addCommandMessage(pcdd->pcpbd->enCSelf, context, btn->id, UINTPTR_MAX, 0, 0, btn->methName, 0) )
        {
            rc = 1;
            goto done_out;
        }
    }

    rc = 0;

done_out:

    if ( dataArray != NULL )
    {
        for ( size_t i = 0; i < count; i++)
        {
            safeLocalFree(dataArray[i]);
        }
        safeLocalFree(dataArray);
    }

    return rc;
}


/** DynamicDialog::createRadioButton() / DynamicDialog::createCheckBox()
 *
 *  @remarks  The loadOptions() argument comes from the parsing of a .rc file
 *            and would be CONNECTCHECKS or CONNECTRADIOS. However the argument
 *            exists whether the dialog is a RcDialog or an UserDialog. The
 *            original thinking was to document the argument so that users could
 *            invoke createCheckBox() or createRadioButton() in a UserDialog and
 *            automatically have the clicked event connected in the same way as
 *            push buttons are.  We allow the users to specify their own method
 *            name when using a UserDialog, but for a RcDialog the code needs to
 *            remain the same if the argument is CONNECTCHECKS or CONNECTRADIOS.
 *
 *  @remarks  The code for both createRadioButton() and createCheckBox() is so
 *            parallel it just doesn't make sense to have 2 separate native
 *            methods.
 */
RexxMethod10(int32_t, dyndlg_createRadioButton, RexxObjectPtr, rxID, int, x, int, y,
             OPTIONAL_uint32_t, cx, OPTIONAL_uint32_t, cy, OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, label,
             OPTIONAL_CSTRING, attributeName, OPTIONAL_CSTRING, loadOptions, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }
    if ( pcdd->active == NULL )
    {
        return -2;
    }

    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

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

    if ( ! addToDialogTemplate(context, pcdd, ButtonAtom, NULL, id, x, y, cx, cy, label, style) )
    {
        return -2;
    }

    int32_t result = 0;

    if ( argumentExists(9) && *loadOptions != '\0' )
    {
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
                free((void *)methName);
                return -2;
            }
            strcpy(finalName, "ID");
            strcat(finalName, methName);

            result = addCommandMessage(pcpbd->enCSelf, context, id, UINTPTR_MAX, 0, 0, finalName, 0) ? 0 : 1;
            free((void *)methName);
            free((void *)finalName);
        }
        else if ( ! isRcLoadItemsArg(loadOptions) )
        {
            result = addCommandMessage(pcpbd->enCSelf, context, id, UINTPTR_MAX, 0, 0, loadOptions, 0) ? 0 : 1;
        }
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
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }
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

    // For a groupbox, we support right, center, or left aligned text.  By
    // default the alignment is left so we only need to check for the RIGHT and
    // CENTER key word.

    uint32_t  style = WS_CHILD | BS_GROUPBOX;
    style |= getCommonWindowStyles(opts, false, false);
    if ( StrStrI(opts, "RIGHT") != NULL )
    {
        style |= BS_RIGHT;
    }
    else if ( StrStrI(opts, "CENTER") != NULL )
    {
        style |= BS_CENTER;
    }

    if ( ! addToDialogTemplate(context, pcdd, ButtonAtom, NULL, id, x, y, cx, cy, text, style) )
    {
        return -2;
    }
    int32_t result = 0;

    // Connect the data attribute if we need to, and the id is not IDC_STATIC.
    // This is done so that setting radio buttons will work if the user has
    // given the group style to the group box, which is common practice.  We
    // have no attribute name so we pass in null
    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

    if ( pcpbd->autoDetect && id != -1 )
    {
        result = connectCreatedControl(context, pcpbd, rxID, id, NULL, winGroupBox);
    }
    return result;
}


/** DynamicDialog::createEdit()
 *
 */
RexxMethod8(int32_t, dyndlg_createEdit, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, OPTIONAL_uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, attributeName, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }
    if ( pcdd->active == NULL )
    {
        return -2;
    }

    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

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

    if ( ! addToDialogTemplate(context, pcdd, EditAtom, NULL, id, x, y, cx, cy, NULL, style) )
    {
        return -2;
    }

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
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }
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

    if ( ! addToDialogTemplate(context, pcdd, ScrollBarAtom, NULL, id, x, y, cx, cy, NULL, style) )
    {
        return -2;
    }
    return 0;
}


/** DynamicDialog::createListBox()
 *
 */
RexxMethod8(int32_t, dyndlg_createListBox, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, attributeName, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }
    if ( pcdd->active == NULL )
    {
        return -2;
    }

    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

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

    if ( ! addToDialogTemplate(context, pcdd, ListBoxAtom, NULL, id, x, y, cx, cy, NULL, style) )
    {
        return -2;
    }

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
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }
    if ( pcdd->active == NULL )
    {
        return -2;
    }

    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

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

    if ( StrStrI(opts, "NOHSCROLL"       ) == NULL ) style |= CBS_AUTOHSCROLL;
    if ( StrStrI(opts, "DISABLENOSCROLL" ) != NULL ) style |= CBS_DISABLENOSCROLL;
    if ( StrStrI(opts, "LOWER"           ) != NULL ) style |= CBS_LOWERCASE;
    if ( StrStrI(opts, "PARTIAL"         ) != NULL ) style |= CBS_NOINTEGRALHEIGHT;
    if ( StrStrI(opts, "SORT"            ) != NULL ) style |= CBS_SORT;
    if ( StrStrI(opts, "UPPER"           ) != NULL ) style |= CBS_UPPERCASE;
    if ( StrStrI(opts, "VSCROLL"         ) != NULL ) style |= WS_VSCROLL;
    if ( StrStrI(opts, "HSCROLL"         ) != NULL ) style |= WS_HSCROLL;

    if ( ! addToDialogTemplate(context, pcdd, ComboBoxAtom, NULL, id, x, y, cx, cy, NULL, style) )
    {
        return -2;
    }

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
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }

    if ( pcdd->active == NULL )
    {
        return -2;
    }

    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

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

    if ( ! addToDialogTemplate(context, pcdd, 0, PROGRESS_CLASS, id, x, y, cx, cy, NULL, style) )
    {
        return -2;
    }
    return 0;
}

/** DynamicDialog::createNamedControl()
 *
 */
RexxMethod9(int32_t, dyndlg_createNamedControl, RexxObjectPtr, rxID, int, x, int, y, uint32_t, cx, uint32_t, cy,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, attributeName, NAME, msgName, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }
    if ( pcdd->active == NULL )
    {
        return -2;
    }

    pCPlainBaseDialog pcpbd = pcdd->pcpbd;

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

    if ( ! addToDialogTemplate(context, pcdd, 0, windowClass, id, x, y, cx, cy, NULL, style) )
    {
        return -2;
    }

    int32_t result = 0;

    // Connect the data attribute if we need to.
    if ( pcpbd->autoDetect && controlHasData(ctrl) )
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
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd == NULL )
    {
        return 0;
    }

    pCPlainBaseDialog pcpbd = pcdd->pcpbd;
    int32_t rc = -1;

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

    if ( pcpbd->IconTab == NULL )
    {
        pcpbd->IconTab = (ICONTABLEENTRY *)LocalAlloc(LPTR, sizeof(ICONTABLEENTRY) * DEF_MAX_IT_ENTRIES);
        if ( pcpbd->IconTab == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }
        pcpbd->IT_nextIndex = 0;
        pcpbd->IT_size = DEF_MAX_IT_ENTRIES;
    }

    if ( pcpbd->IT_nextIndex >= pcpbd->IT_size )
    {
        HLOCAL temp = LocalReAlloc(pcpbd->IconTab, sizeof(ICONTABLEENTRY) * pcpbd->IT_size * 2, LMEM_ZEROINIT | LMEM_MOVEABLE);
        if ( temp == NULL )
        {
            internalErrorMsgBox(OOD_ADDICONFILE_ERR_MSG, OOD_RESOURCE_ERR_TITLE);
            goto done_out;
        }

        pcpbd->IT_size *= 2;
        pcpbd->IconTab = (ICONTABLEENTRY *)temp;
    }

    size_t i;

    // If there is already a resource with this ID, it is replaced.
    for ( i = 0; i < pcpbd->IT_nextIndex; i++ )
    {
        if ( pcpbd->IconTab[i].iconID == iconID )
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

    pcpbd->IconTab[i].fileName = buf;
    pcpbd->IconTab[i].iconID = iconID;
    if ( i == pcpbd->IT_nextIndex )
    {
        pcpbd->IT_nextIndex++;
    }
    rc = 0;

done_out:
    return rc;
}


/** DynamicDialog::stop()  [private]
 *
 *  Called when a dialog is being constructred from a resource script file and
 *  there is an error from loadFrame().  In this case the underlying dialog can
 *  not be created, yet the CSelf struct still needs to be cleaned up.
 */
RexxMethod1(RexxObjectPtr, dyndlg_stop, CSELF, pCSelf)
{
    pCDynamicDialog pcdd = validateDDCSelf(context, pCSelf);
    if ( pcdd != NULL )
    {
        stopDialog(pcdd->pcpbd, context->threadContext);
    }

    return NULLOBJECT;
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

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1, true) )
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
    pcdd->expected = expected;

    DLGTEMPLATEEX *pBase;
    if ( ! startDialogTemplate(context->threadContext, &pBase, pcdd, x, y, cx, cy, NULL, NULL, fontName, fontSize, style, 0) )
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

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1, true) )
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

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1, true) )
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


extern BOOL SHIFTkey = FALSE;

/**
 * Although not examined throughly, this appears to be a custom implementation
 * of the Windows dialog manager to deal with ooDialog's CategoryDialog.
 *
 * @param pcpbd  Pointer to the dialog CSelf struct.
 * @param lpmsg
 *
 * @return BOOL
 *
 * @remarks  This function is *only* called if the dialog is a CategoryDialog.
 *           CategoryDialogs are deprecated and the intent is to not fix bugs in
 *           deprecated classes or methods.  This function is unchanged from the
 *           original.  If there is a bug in it, then that's the way it is.
 */
BOOL IsNestedDialogMessage(pCPlainBaseDialog pcpbd, PMSG  lpmsg)
{
    HWND hW, hParent, hW2;
    bool prev = false;

    if ( ! pcpbd->childDlg || ! pcpbd->activeChild )
    {
        return IsDialogMessage(pcpbd->hDlg, lpmsg);
    }

    switch ( lpmsg->message )
    {
        case WM_KEYDOWN:
            switch ( lpmsg->wParam )
            {
                case VK_SHIFT:
                    SHIFTkey = TRUE;
                    break;

                case VK_TAB:

                    if ( IsChild(pcpbd->activeChild, lpmsg->hwnd) )
                    {
                        hParent = pcpbd->activeChild;
                    }
                    else
                    {
                        hParent = pcpbd->hDlg;
                    }

                    hW = GetNextDlgTabItem(hParent, lpmsg->hwnd, SHIFTkey);
                    hW2 = GetNextDlgTabItem(hParent, NULL, SHIFTkey);

                    /* see if we have to switch to the other dialog */
                    if ( hW == hW2 )
                    {
                        if ( hParent == pcpbd->hDlg )
                        {
                            hParent = pcpbd->activeChild;
                        }

                        hW = GetNextDlgTabItem(hParent, NULL, SHIFTkey);
                        return SendMessage(hParent, WM_NEXTDLGCTL, (WPARAM)hW, (LPARAM)TRUE) != 0;
                    }
                    else
                    {
                        return SendMessage(hParent, WM_NEXTDLGCTL, (WPARAM)hW, (LPARAM)TRUE) != 0;
                    }
                    return TRUE;

                case VK_LEFT:
                case VK_UP:
                    prev = true;

                case VK_RIGHT:
                case VK_DOWN:
                    if ( IsChild(pcpbd->activeChild, lpmsg->hwnd) )
                    {
                        hParent = pcpbd->activeChild;
                    }
                    else
                    {
                        hParent = pcpbd->hDlg;
                    }

                    hW = GetNextDlgGroupItem(hParent, lpmsg->hwnd, prev);
                    hW2 = GetNextDlgGroupItem(hParent, NULL, prev);

                    /* see if we have to switch to the other dialog */
                    if ( hW == hW2 )
                    {
                        if ( hParent == pcpbd->hDlg )
                        {
                            hParent = pcpbd->activeChild;
                        }
                        else
                        {
                            hParent = pcpbd->hDlg;
                        }
                        return IsDialogMessage(hParent, lpmsg);
                    }
                    else
                    {
                        return IsDialogMessage(hParent, lpmsg);
                    }
                    return TRUE;

                case VK_CANCEL:
                case VK_RETURN:
                    return IsDialogMessage(pcpbd->hDlg, lpmsg);

                default:
                    hParent = (HWND)getWindowPtr(lpmsg->hwnd, GWLP_HWNDPARENT);
                    if ( ! hParent )
                    {
                        return FALSE;
                    }
                    return IsDialogMessage(hParent, lpmsg);
            }
            break;

        case WM_KEYUP:
            if ( lpmsg->wParam == VK_SHIFT )
            {
                SHIFTkey = FALSE;
            }
            break;
    }

    hParent = (HWND)getWindowPtr(lpmsg->hwnd, GWLP_HWNDPARENT);

    if ( hParent )
    {
        return IsDialogMessage(hParent, lpmsg);
    }
    else
    {
        return IsDialogMessage(pcpbd->hDlg, lpmsg);
    }
}



