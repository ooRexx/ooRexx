/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2012-2021 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
 * oodToolTipp.cpp
 *
 * Contains methods for the ToolTip control.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <shlwapi.h>
#include <WindowsX.h>

#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodMessaging.hpp"
#include "oodControl.hpp"
#include "oodResources.hpp"
#include "oodShared.hpp"


/**
  * Convert TTF_* flags into a string of keywwords specific to a TTN_NEEDTEXT
  * notification.
  *
  * @param flags
  *
  * @return RexxStringObject
  *
  * @remarks  This is an extern fucntion, not static.
  */
RexxStringObject ttdiFlags2keyword(RexxThreadContext *c, uint32_t flags)
{
    char buf[512];
    *buf = '\0';

    if ( flags & TTF_IDISHWND   ) strcat(buf, "IDISHWND ");
    if ( flags & TTF_RTLREADING ) strcat(buf, "RTLREADING ");
    if ( flags & TTF_DI_SETITEM ) strcat(buf, "DI_SETITEM ");

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return c->String(buf);
}


/**
 * Parse TTF_X keywords into the corresponding value specific to a TTN_NEEDTEXT
 * notification..
 *
 * @param flags   Keyword string to parse
 *
 * @return The combined flag value
 */
static uint32_t keyword2ttdiFlags(CSTRING flags)
{
    uint32_t f = 0;

    if ( StrStrI(flags, "IDISHWND"   ) != NULL ) f |= TTF_IDISHWND;
    if ( StrStrI(flags, "RTLREADING" ) != NULL ) f |= TTF_RTLREADING;
    if ( StrStrI(flags, "DI_SETITEM" ) != NULL ) f |= TTF_DI_SETITEM;

    return f;
}

/**
 * Convert a mouse window message code to a name.
 */
static RexxStringObject mousemsg2name(RexxThreadContext *c, uint32_t msg)
{
    switch ( msg )
    {
        case WM_MOUSEMOVE      : return c->String("mouseMove    ");
        case WM_LBUTTONDOWN    : return c->String("lButtonDown  ");
        case WM_LBUTTONUP      : return c->String("lButtonUp    ");
        case WM_LBUTTONDBLCLK  : return c->String("lButtonDblClk");
        case WM_RBUTTONDOWN    : return c->String("rButtonDown  ");
        case WM_RBUTTONUP      : return c->String("rButtonUp    ");
        case WM_RBUTTONDBLCLK  : return c->String("rButtonDblClk");
        case WM_MBUTTONDOWN    : return c->String("mButtonDown  ");
        case WM_MBUTTONUP      : return c->String("mButtonUp    ");
        case WM_MBUTTONDBLCLK  : return c->String("mButtonDblClk");
        case WM_XBUTTONDOWN    : return c->String("xButtonDown  ");
        case WM_XBUTTONUP      : return c->String("xButtonUp    ");
        case WM_XBUTTONDBLCLK  : return c->String("xButtonDblClk");
        case WM_MOUSEWHEEL     : return c->String("mouseWheel   ");
    }
    return c->String("unknown");
}


/**
 * Allocates a Rexx buffer to be used, ultimately, for a .ToolInfo object.
 * Optionally, allocate a buffer for the TOOLINFO.lpszText field
 *
 * @param c
 * @param allocTextBuf
 *
 * @return The Rexx buffer object on success, .nil on error.
 *
 * @remarks  If the allocation for the text buffer fails, we return NULLOBJECT.
 *           The Rexx buffer object will not be protected anywhere, so it will
 *           immediately be elgible for garbage collection
 */
static RexxBufferObject rexxBufferForToolInfo(RexxMethodContext *c, LPTOOLINFO *ppTI, bool allocTextBuf)
{
    RexxBufferObject tiBuf = c->NewBuffer(sizeof(TOOLINFO));
    if ( tiBuf == NULLOBJECT )
    {
        outOfMemoryException(c->threadContext);
        return NULLOBJECT;
    }

    LPTOOLINFO pTI = (LPTOOLINFO)c->BufferData(tiBuf);
    memset(pTI, 0, sizeof(TOOLINFO));

    pTI->cbSize = sizeof(TOOLINFO);

    if ( allocTextBuf )
    {
        pTI->lpszText = (char *)LocalAlloc(LPTR, MAX_TOOLINFO_TEXT_LENGTH + 1);
        if ( pTI->lpszText == NULL )
        {
            outOfMemoryException(c->threadContext);
            c->ReleaseLocalReference(tiBuf);
            return NULLOBJECT;
        }
    }
    *ppTI = pTI;

    return tiBuf;
}


/**
 * Attempts to return the Rexx objects that represents the TOOLINFO.hwnd and the
 * TOOLINFO.uId fields.
 *
 * @param c
 * @param pTI
 * @param hwndSupplier
 * @param idSupplier
 *
 * @remarks  Currently, we are called with a pTI that was obtained from the
 *           operating system.  I don't think that can change.  I'm not sure to
 *           trust the TTF_IDISHWND flag or not.  It would seem that has to be
 *           correct, or the tool tip would not be functioning correctly.
 *
 *           If we can trust TTF_IDISHWND, then if it is present, uId has to be
 *           the hwnd of a dialog control and hwnd has to be the hwnd of a
 *           dialog.
 *
 *           If it is absent, then uId is a number and hwnd could be a dialog or
 *           a dialog control.
 *
 *           For now, we will trust the flag, but this is something to check if
 *           we see odd results.
 *
 *           Also, currently, it is not possible to add a tool to a tool tip
 *           from Rexx code without using a Rexx dialog control object, if the
 *           TOOLINFO.hwnd is a dialog control.  This means that the
 *           pCDialogControl pointer must exist in the control's window words.
 */
void getToolIdentifiers(RexxMethodContext *c, LPTOOLINFO pTI, RexxObjectPtr *hwndSupplier, RexxObjectPtr *idSupplier)
{
    RexxObjectPtr rxHwnd = TheNilObj;
    RexxObjectPtr rxID   = TheNilObj;

    SetLastError(0);

    if ( pTI->uFlags & TTF_IDISHWND )
    {
        pCPlainBaseDialog pcpbd   = (pCPlainBaseDialog)getWindowPtr(pTI->hwnd, GWLP_USERDATA);
        oodControl_t      ctrType = controlHwnd2controlType((HWND)pTI->uId);

        rxHwnd = pcpbd->rexxSelf;
        rxID   = createControlFromHwnd(c, pcpbd, (HWND)pTI->uId, ctrType, true);
    }
    else
    {
        rxID = c->Uintptr(pTI->uId);

        // If ctrlType is winUnknown, the hwnd must be a dialog.
        oodControl_t ctrlType = controlHwnd2controlType(pTI->hwnd);
        if ( ctrlType == winUnknown || ctrlType == winDialog )
        {
            pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)getWindowPtr(pTI->hwnd, GWLP_USERDATA);
            rxHwnd = pcpbd->rexxSelf;
        }
        else
        {
            rxHwnd = (RexxObjectPtr)getWindowPtr(pTI->hwnd, GWLP_USERDATA);
        }
    }

    *hwndSupplier = rxHwnd;
    *idSupplier   = rxID;
}


/**
 * Uses a pointer to a TOOLINFO struct and a Rexx buffer to instantiate a new
 * .ToolInfo.
 *
 * @param c
 * @param tiBuf
 * @param pTI
 * @param memAllocated  If true signals that a buffer was allocated for th text
 *                      field of the TOOLINFO struct.
 *
 * @return RexxObjectPtr
 */
static RexxObjectPtr ti2rexxToolInfo(RexxMethodContext *c, RexxBufferObject tiBuf, LPTOOLINFO pTI, bool memAllocated)
{
    RexxObjectPtr result = TheNilObj;

    RexxClassObject tiCls = rxGetContextClass(c, "TOOLINFO");
    if ( tiCls != NULLOBJECT )
    {
        RexxObjectPtr hwndSupplier;
        RexxObjectPtr idSupplier;
        getToolIdentifiers(c, pTI, &hwndSupplier, &idSupplier);

        RexxArrayObject args = c->NewArray(5);
        c->ArrayPut(args, tiBuf, 1);
        c->ArrayPut(args, hwndSupplier, 2);
        c->ArrayPut(args, idSupplier, 5);

        result = c->SendMessage(tiCls, "NEW", args);
        if ( result == NULLOBJECT )
        {
            goto done_out;
        }

        if ( memAllocated )
        {
            c->SendMessage1(result, "TEXTMEMORYISALLOCATED=", TheTrueObj);
        }
    }

done_out:
    return result;
}


/**
 * Allocates a buffer for the tool info struct, copies the specified text into
 * it, and updates the .ToolInfo object if specified.
 *
 * @param c
 * @param pTI
 * @param text
 * @param argPos
 * @param toolInfo  Can be NULLOBJECT, which indicates we are called from within
 *                  a .ToolInfo object's context.  If not null, we are called
 *                  from some other context and we need to update the
 *                  MEMALLOCATED attribute by sending a messag to this object.
 *
 * @return bool
 */
static bool setToolInfoText(RexxMethodContext *c, LPTOOLINFO pTI, CSTRING text, size_t argPos, RexxObjectPtr toolInfo)
{
    size_t l = strlen(text);
    if ( l > MAX_TOOLINFO_TEXT_LENGTH )
    {
        stringTooLongException(c->threadContext, argPos, MAX_TOOLINFO_TEXT_LENGTH, l);
        return false;
    }

    if ( l == 0 || StrCmpI("TEXTCALLBACK", text) == 0 )
    {
        pTI->lpszText = LPSTR_TEXTCALLBACK;
    }
    else
    {
        pTI->lpszText = (LPSTR)LocalAlloc(LPTR, MAX_TOOLINFO_TEXT_LENGTH + 1);
        if ( pTI->lpszText == NULL )
        {
            outOfMemoryException(c->threadContext);
            return false;
        }
        strcpy(pTI->lpszText, text);

        if ( toolInfo == NULLOBJECT )
        {
            c->SetObjectVariable(TOOLINFO_MEMALLOCATED_VAR, TheTrueObj);
        }
        else
        {
            c->SendMessage1(toolInfo, "SETTEXTMEMORYISALLOCATED", TheTrueObj);
        }
    }
    return true;
}



/**
 *  Methods for the .ToolTip class.
 */
#define TOOLTIP_CLASS             "ToolTip"

#define TOOLTIP_CREATED_MSG       "when the underlying Windows ToolTip has already been created"
#define TOOLTIP_NOT_CREATED_MSG   "when the underlying Windows ToolTip has not been created"
#define WRONG_EVENT_NAME_MSG      "exactly one of the keywords: RELAY, NEEDTEXT, SHOW, POP, LINKCLICK, or NORELAY"

static bool lazyInitToolTipTable(RexxMethodContext *c, pCPlainBaseDialog pcpbd)
{
    pcpbd->ToolTipTab = (TOOLTIPTABLEENTRY *)LocalAlloc(LPTR, sizeof(TOOLTIPTABLEENTRY) * DEF_MAX_TTT_ENTRIES);
    if ( ! pcpbd->ToolTipTab )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }
    pcpbd->TTT_size      = DEF_MAX_TTT_ENTRIES;
    pcpbd->TTT_nextIndex = 0;

    return true;
}

/**
 * Adds a new tool tip entry to the tool tip table, allocating a bigger table if
 * needed.
 *
 * @param c
 * @param pcpbd
 * @param toolTip
 * @param hToolTip
 * @param id
 *
 * @return bool
 */
static bool addToolTipToTable(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr toolTip, HWND hToolTip, uint32_t id)
{
    if ( pcpbd->TTT_nextIndex >= pcpbd->TTT_size )
    {
        HLOCAL temp = LocalReAlloc(pcpbd->ToolTipTab, sizeof(TOOLTIPTABLEENTRY) * pcpbd->TTT_size * 2, LMEM_ZEROINIT | LMEM_MOVEABLE);
        if ( temp == NULL )
        {
            char buf[512];
            _snprintf(buf, sizeof(buf), "ToolTip controles have exceeded the maximum\n"
                       "number of allocated table entries, and the table\n"
                       "could not be expanded.\n\n"
                       "  System error code: %d\n\n"
                       "No ToolTip control can be added.", GetLastError());

            MessageBox(NULL, buf, "Error", MB_OK | MB_ICONHAND);
            return false;
        }

        pcpbd->TTT_size *= 2;
        pcpbd->ToolTipTab = (TOOLTIPTABLEENTRY *)temp;
    }

    setWindowPtr(hToolTip, GWLP_USERDATA, (LONG_PTR)toolTip);
    c->SendMessage1(pcpbd->rexxSelf, "PUTCONTROL", toolTip);

    pCDialogControl pcdc = controlToCSelf(c, toolTip);

    pcpbd->ToolTipTab[pcpbd->TTT_nextIndex].id = id;
    pcpbd->ToolTipTab[pcpbd->TTT_nextIndex].hToolTip = hToolTip;
    pcpbd->ToolTipTab[pcpbd->TTT_nextIndex].rexxSelf = toolTip;

    pcdc->toolTipEntry = &pcpbd->ToolTipTab[pcpbd->TTT_nextIndex];

    pcpbd->TTT_nextIndex++;

    return true;
}


/**
 *  Parses a list of tool tip style keywords and retuns the corresponding style
 *  flag.
 *
 * @param flags
 *
 * @return uint32_t
 *
 * @remarks  flags may be null.
 */
static uint32_t parseToolTipStyle(CSTRING flags)
{
    uint32_t style = WS_POPUP;

    if ( flags == NULL )
    {
        style |=  TTS_NOPREFIX | TTS_ALWAYSTIP;
        goto done_out;
    }

    if ( StrStrI(flags, "ALWAYSTIP"     ) != NULL ) style |= TTS_ALWAYSTIP;
    if ( StrStrI(flags, "BALLOON"       ) != NULL ) style |= TTS_BALLOON;
    if ( StrStrI(flags, "CLOSE"         ) != NULL ) style |= TTS_CLOSE;
    if ( StrStrI(flags, "NOANIMATE"     ) != NULL ) style |= TTS_NOANIMATE;
    if ( StrStrI(flags, "NOFADE"        ) != NULL ) style |= TTS_NOFADE;
    if ( StrStrI(flags, "NOPREFIX"      ) != NULL ) style |= TTS_NOPREFIX;
    if ( StrStrI(flags, "USEVISUALSTYLE") != NULL ) style |= TTS_USEVISUALSTYLE;

done_out:
    return style;
}

/**
 * Parse TTF_X keywords into the corresponding value.
 *
 * @param flags   Keyword string to parse
 *
 * @return The combined flag value
 */
static uint32_t keyword2ttfFlags(RexxMethodContext *c, CSTRING flags)
{
    uint32_t f = 0;

    if ( StrStrI(flags, "ABSOLUTE"   ) != NULL ) f |= TTF_ABSOLUTE;
    if ( StrStrI(flags, "CENTERTIP"  ) != NULL ) f |= TTF_CENTERTIP;
    if ( StrStrI(flags, "IDISHWND"   ) != NULL ) f |= TTF_IDISHWND;
    if ( StrStrI(flags, "RTLREADING" ) != NULL ) f |= TTF_RTLREADING;
    if ( StrStrI(flags, "SUBCLASS"   ) != NULL ) f |= TTF_SUBCLASS;
    if ( StrStrI(flags, "TRACK"      ) != NULL ) f |= TTF_TRACK;
    if ( StrStrI(flags, "TRANSPARENT") != NULL ) f |= TTF_TRANSPARENT;
    if ( StrStrI(flags, "PARSELINKS" ) != NULL )
    {
        if ( ! requiredComCtl32Version(c, "PARSELINKS", COMCTL32_6_0) )
        {
            return OOD_ID_EXCEPTION;
        }
        f |= TTF_PARSELINKS;
    }

    return f;
}

 /**
  * Convert TTF_* flags into a string of keywwords.
  *
  * @param flags
  *
  * @return RexxStringObject
  */
static RexxStringObject ttfFlags2keyword(RexxMethodContext *c, uint32_t flags)
{
    char buf[512];
    *buf = '\0';

    if ( flags & TTF_ABSOLUTE   ) strcat(buf, "ABSOLUTE ");
    if ( flags & TTF_CENTERTIP  ) strcat(buf, "CENTERTIP ");
    if ( flags & TTF_IDISHWND   ) strcat(buf, "IDISHWND ");
    if ( flags & TTF_PARSELINKS ) strcat(buf, "PARSELINKS ");
    if ( flags & TTF_RTLREADING ) strcat(buf, "RTLREADING ");
    if ( flags & TTF_SUBCLASS   ) strcat(buf, "SUBCLASS ");
    if ( flags & TTF_TRACK      ) strcat(buf, "TRACK ");
    if ( flags & TTF_TRANSPARENT) strcat(buf, "TRANSPARENT ");

    if ( *buf != '\0' )
    {
        *(buf + strlen(buf) - 1) = '\0';
    }
    return c->String(buf);
}


/**
 * Generic function to fill in the hwnd and uID fields of the tool info struct.
 * Called from several of the methods that deal with tool tips.
 *
 * If rxObj is a ToolInfo object, then we have all the information we need.
 *
 * When rxObj is not a ToolInfo object and uID is omitted, then rxObj must be a
 * dialog control.  The owner dialog of the dialog control is set as hwnd and
 * the dialog control hwnd is set as the uID.
 *
 * Otherwise, rxObj could be a dialog or a dialog control and uID could be a
 * dialog control or a numeric ID.
 *
 * @param c
 * @param rxObj
 * @param uID
 * @param pTI
 * @param hwndSupplier
 * @param uIDSupplier
 *
 * @return True on success, false on failure.
 *
 * @remarks  In some cases this function is invoked when instantiating a new
 *           ToolInfo object.  In those cases, it is convenient to return the
 *           hwndSupplier and uIDSupplier, and rxObj will never be a Rexx
 *           ToolInfo object.
 *
 *           When rxObj is already a Rexx ToolInfo, then the caller has no need
 *           of hwndSupplier or uIDSupplier and so we completely ignore
 *           hwndSupplier and uIDSupplier.
 */
static bool genericToolID(RexxMethodContext *c, RexxObjectPtr rxObj, RexxObjectPtr rxID, LPTOOLINFO pTI,
                          RexxObjectPtr *hwndSupplier, RexxObjectPtr *uIDSupplier, size_t arg1Pos)
{
    bool success = false;

    if ( c->IsOfType(rxObj, "TOOLINFO") )
    {
        LPTOOLINFO pToolInfo = (LPTOOLINFO)c->ObjectToCSelf(rxObj);

        pTI->hwnd = pToolInfo->hwnd;
        pTI->uId  = pToolInfo->uId;

        success = true;
        goto done_out;
    }

    if ( rxID == NULLOBJECT )
    {
        if ( ! requiredClass(c->threadContext, rxObj, "DIALOGCONTROL", arg1Pos) )
        {
            goto done_out;
        }
        pCDialogControl pcdc = controlToCSelf(c, rxObj);

        pTI->hwnd = pcdc->hDlg;
        pTI->uId  = (UINT_PTR)pcdc->hCtrl;

        pTI->uFlags |= TTF_IDISHWND;

        if ( hwndSupplier != NULL )
        {
            *hwndSupplier = pcdc->oDlg;
            *uIDSupplier  = pcdc->rexxSelf;
        }

        success = true;
        goto done_out;
    }

    if ( c->IsOfType(rxObj, "PLAINBASEDIALOG") )
    {
        pCPlainBaseDialog pcpbd = dlgToCSelf(c, rxObj);

        pTI->hwnd = pcpbd->hDlg;
        if ( hwndSupplier != NULL )
        {
            *hwndSupplier = rxObj;
        }

        if ( c->IsOfType(rxID, "DIALOGCONTROL") )
        {
            pCDialogControl pcdc = controlToCSelf(c, rxID);

            pTI->uId = (UINT_PTR)pcdc->hCtrl;
            pTI->uFlags |= TTF_IDISHWND;
        }
        else
        {
            uint32_t id = oodResolveSymbolicID(c->threadContext, pcpbd->rexxSelf, rxID, -1, arg1Pos + 1, false);
            if ( id == OOD_ID_EXCEPTION  )
            {
                goto done_out;
            }

            pTI->uId = (UINT_PTR)id;
        }

        if ( uIDSupplier != NULL )
        {
            *uIDSupplier = rxID;
        }

        success = true;
    }
    else if ( c->IsOfType(rxObj, "DIALOGCONTROL") )
    {
        pCDialogControl pcdc = controlToCSelf(c, rxObj);

        uint32_t id = oodResolveSymbolicID(c->threadContext, pcdc->oDlg, rxID, -1, arg1Pos + 1, false);
        if ( id == OOD_ID_EXCEPTION  )
        {
            goto done_out;
        }

        pTI->hwnd = pcdc->hCtrl;
        pTI->uId = (UINT_PTR)id;

        if ( hwndSupplier != NULL )
        {
            *hwndSupplier = rxObj;
            *uIDSupplier  = rxID;
        }

        success = true;
    }
    else
    {
        userDefinedMsgException(c, "Invalid tool ID specification");
    }


done_out:
    return success;
}


static uint32_t matchEvent2index(RexxMethodContext *c, CSTRING evtName, CSTRING *mthName, size_t i, size_t argPos)
{
    if ( StrCmpI(evtName, "RELAY") == 0 )
    {
        *mthName = "onRelay";
        return RE_RELAYEVENT_IDX;
    }
    else if ( StrCmpI(evtName, "NEEDTEXT") == 0 )
    {
        *mthName = "onNeedText";
        return RE_NEEDTEXT_IDX;
    }
    else if ( StrCmpI(evtName, "SHOW") == 0 )
    {
        *mthName = "onShow";
        return RE_SHOW_IDX;
    }
    else if ( StrCmpI(evtName, "POP") == 0 )
    {
        *mthName = "onPop";
        return RE_POP_IDX;
    }
    else if ( StrCmpI(evtName, "LINKCLICK") == 0 )
    {
        *mthName = "onLinkClick";
        return RE_LINKCLICK_IDX;
    }
    else if ( StrCmpI(evtName, "NORELAY") == 0 )
    {
        *mthName = "";
        return RE_NORELAY_IDX;
    }
    else
    {
        wrongObjInArrayException(c->threadContext, argPos, i, WRONG_EVENT_NAME_MSG, evtName);
        return OOD_ID_EXCEPTION;
    }
}

void freeRelayData(pSubClassData pSCData)
{
    EnterCriticalSection(&crit_sec);

    if ( pSCData )
    {
        pRelayEventData pred = (pRelayEventData)pSCData->pData;

        if ( pred )
        {
            for ( size_t i = 0; i < RE_COUNT_RELAYEVENTS; i++ )
            {
                safeLocalFree((void *)pred->methods[i]);
            }
            LocalFree((void *)pred);
        }

        if ( pSCData->pcdc != NULL )
        {
            pSCData->pcdc->pRelayEvent = NULL;
        }
    }

    LeaveCriticalSection(&crit_sec);
}


/**
 * Checks that reply is not null and that the context does not have a pending
 * condition.
 *
 * @param c
 * @param pcpbd
 * @param reply
 * @param methodName
 * @param clear
 *
 * @return True if reply is not null and there is no pending condition.
 *
 * @note  If there is a condition, it is just printed, but the dialog is not
 *        ended.  This results in a message to the screen, if the user is
 *        running from a console window, but the dialog keeps running.
 *
 * @note  The call to checkForCondition() after noMsgReturnException() is what
 *        causes the condition to be printed to the screen.
 */
bool checkReplyIsGood(RexxThreadContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr reply, CSTRING methodName, bool clear)
{
    bool haveCondition = checkForCondition(c, clear);

    if ( ! haveCondition && reply == NULLOBJECT )
    {
        noMsgReturnException(c, methodName);
        haveCondition = true;
        checkForCondition(c, clear);
    }
    return ! haveCondition;
}


/**
 * Checks that no condition has been raised, and that reply is either true or
 * false. If not, an exception is raised.
 *
 * @param c
 * @param pcpbd
 * @param reply
 * @param method
 * @param clear
 *
 * @return TheTrueObj or TheFalseObj on success, NULLOBJECT on failure.
 *
 * @note  The local reference to the reply object is always released in this
 *        function.
 */
RexxObjectPtr checkForBoolean(RexxThreadContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr reply,
                               CSTRING method, bool clear)
{
    RexxObjectPtr result = NULLOBJECT;

    if ( checkReplyIsGood(c, pcpbd, reply, method, false) )
    {
        result = convertToTrueOrFalse(c, reply);
        if ( result == NULLOBJECT )
        {
            wrongReplyNotBooleanException(c, method, reply);
            checkForCondition(c, false);
        }
    }

    if ( reply != NULLOBJECT )
    {
        c->ReleaseLocalReference(reply);
    }

    return result;
}

/**
 * The subclass window procedure for a dialog control tool.
 *
 * @param hwnd
 * @param msg
 * @param wParam
 * @param lParam
 * @param id
 * @param dwData
 *
 * @return LRESULT
 */
LRESULT CALLBACK ManageAtypicalToolProc(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR dwData)
{
    pSubClassData      pData = (pSubClassData)dwData;
    pRelayEventData    pred  = (pRelayEventData)pData->pData;
    RexxThreadContext *c     = pData->pcpbd->dlgProcContext;

    if ( ((msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) || msg == WM_NCMOUSEMOVE) && ! pred->skipRelay )
    {
        MSG _msg;
        _msg.hwnd = hwnd;
        _msg.message = msg;
        _msg.wParam = wParam;
        _msg.lParam = lParam;

        if ( pred->doEvent[RE_RELAYEVENT_IDX] )
        {
            CSTRING       method  = pred->methods[RE_RELAYEVENT_IDX];
            RexxObjectPtr rxPoint = rxNewPoint(c, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            RexxObjectPtr rxMMsg  = mousemsg2name(c, msg);
            RexxArrayObject args  = c->ArrayOfFour(pred->rxToolTip, rxPoint, rxMMsg, pData->pcdc->rexxSelf);

            RexxObjectPtr reply = c->SendMessage(pData->pcpbd->rexxSelf, method, args);

            if ( msgReplyIsGood(c, pData->pcpbd, reply, method, false) )
            {
                c->ReleaseLocalReference(reply);
                c->ReleaseLocalReference(rxPoint);
                c->ReleaseLocalReference(rxMMsg);
                c->ReleaseLocalReference(args);
            }
            else
            {
                SendMessage(pred->hToolTip, TTM_ACTIVATE, 0, 0);
                endDialogPremature(pData->pcpbd, pData->pcpbd->hDlg, RexxConditionRaised);
                return FALSE;
            }
        }

        SendMessage(pred->hToolTip, TTM_RELAYEVENT, 0, (LPARAM)&_msg);
    }

    if ( msg == WM_NOTIFY )
    {
        uint32_t code = ((NMHDR *)lParam)->code;

        switch ( code )
        {
            case TTN_SHOW :
            {
                if ( pred->doEvent[RE_SHOW_IDX] )
                {
                    RexxObjectPtr   rxToolID  = getToolIDFromLParam(c, lParam);
                    CSTRING         method    = pred->methods[RE_SHOW_IDX];
                    LPARAM          result    = FALSE;
                    RexxArrayObject args      = c->ArrayOfThree(pred->rxToolTip, pData->pcdc->rexxSelf, rxToolID);

                    RexxObjectPtr reply = c->SendMessage(pData->pcpbd->rexxSelf, method, args);

                    reply = checkForBoolean(c, pData->pcpbd, reply, method, false);
                    if ( reply == NULLOBJECT )
                    {
                        SendMessage(pred->hToolTip, TTM_ACTIVATE, 0, 0);

                        endDialogPremature(pData->pcpbd, pData->pcpbd->hDlg, RexxConditionRaised);
                        return FALSE;
                    }
                    if ( reply == TheTrueObj )
                    {
                        result = TRUE;
                    }

                    c->ReleaseLocalReference(args);
                    if ( rxToolID != TheNilObj )
                    {
                        c->ReleaseLocalReference(rxToolID);
                    }

                    return result;
                }
            }
            break;

            case TTN_GETDISPINFOA :
            case TTN_GETDISPINFOW :
            {
                if ( pred->doEvent[RE_NEEDTEXT_IDX] )
                {
                    LPNMTTDISPINFO nmtdi  = (LPNMTTDISPINFO)lParam;
                    CSTRING        method = pred->methods[RE_NEEDTEXT_IDX];

                    RexxDirectoryObject info = c->NewDirectory();

                    RexxObjectPtr    rxToolID = getToolIDFromLParam(c, lParam);
                    RexxObjectPtr    userData = nmtdi->lParam == NULL ? TheNilObj : (RexxObjectPtr)nmtdi->lParam;
                    RexxStringObject flags    = ttdiFlags2keyword(c, nmtdi->uFlags);

                    c->DirectoryPut(info, c->NullString(), "TEXT");
                    c->DirectoryPut(info, userData, "USERDATA");
                    c->DirectoryPut(info, flags, "FLAGS");
                    c->DirectoryPut(info, rxToolID, "TOOLID");

                    RexxArrayObject args = c->ArrayOfThree(pred->rxToolTip, pData->pcdc->rexxSelf, info);

                    RexxObjectPtr reply = c->SendMessage(pData->pcpbd->rexxSelf, method, args);

                    // Local reference for reply is released in checkForBoolean
                    // Returned reply is TheTrueObj or TheFalseObj - do not need
                    // to release.
                    reply = checkForBoolean(c, pData->pcpbd, reply, method, false);
                    if ( reply == NULLOBJECT )
                    {
                        SendMessage(pred->hToolTip, TTM_ACTIVATE, 0, 0);

                        endDialogPremature(pData->pcpbd, pData->pcpbd->hDlg, RexxConditionRaised);
                        return FALSE;
                    }

                    RexxObjectPtr _text = c->DirectoryAt(info, "TEXT");
                    CSTRING       text  = c->ObjectToStringValue(_text);
                    size_t        len   = strlen(text);

                    if ( len > MAX_TOOLINFO_TEXT_LENGTH )
                    {
                        SendMessage(pred->hToolTip, TTM_ACTIVATE, 0, 0);

                        stringTooLongReplyException(c, method, MAX_TOOLINFO_TEXT_LENGTH + 1, len);
                        checkForCondition(c, false);
                        endDialogPremature(pData->pcpbd, pData->pcpbd->hDlg, RexxConditionRaised);
                        return FALSE;
                    }

                    if ( len > 0 )
                    {
                        pCDialogControl pcdc = controlToCSelf(c, pred->rxToolTip);

                        if ( code == TTN_GETDISPINFOW )
                        {
                            safeLocalFree(pcdc->toolTipEntry->wcharBuf);

                            pcdc->toolTipEntry->wcharBuf = ansi2unicode(text);
                            nmtdi->lpszText = (LPSTR)pcdc->toolTipEntry->wcharBuf;
                        }
                        else
                        {
                            strcpy(pcdc->toolTipEntry->textBuf, text);
                            nmtdi->lpszText = pcdc->toolTipEntry->textBuf;
                        }
                    }
                    if ( reply == TheTrueObj )
                    {
                        nmtdi->uFlags |= TTF_DI_SETITEM;
                    }

                    c->ReleaseLocalReference(_text);
                    c->ReleaseLocalReference(flags);
                    c->ReleaseLocalReference(info);
                    c->ReleaseLocalReference(args);
                    if ( rxToolID != TheNilObj )
                    {
                        c->ReleaseLocalReference(rxToolID);
                    }

                    return TRUE;
                }
            }
            break;

            case TTN_POP :
            {
                if ( pred->doEvent[RE_POP_IDX] )
                {
                    CSTRING         method   = pred->methods[RE_POP_IDX];
                    RexxObjectPtr   rxToolID = getToolIDFromLParam(c, lParam);
                    RexxArrayObject args     = c->ArrayOfThree(pred->rxToolTip, pData->pcdc->rexxSelf, rxToolID);

                    RexxObjectPtr reply = c->SendMessage(pData->pcpbd->rexxSelf, method, args);

                    // ignore return.

                    c->ReleaseLocalReference(args);
                    if ( rxToolID != TheNilObj )
                    {
                        c->ReleaseLocalReference(rxToolID);
                    }

                    return 0;
                }
            }
            break;

            case TTN_LINKCLICK :
            {
                if ( pred->doEvent[RE_LINKCLICK_IDX] )
                {
                    CSTRING         method   = pred->methods[RE_LINKCLICK_IDX];
                    RexxObjectPtr   rxToolID = getToolIDFromLParam(c, lParam);
                    RexxArrayObject args     = c->ArrayOfThree(pred->rxToolTip, pData->pcdc->rexxSelf, rxToolID);

                    RexxObjectPtr reply = c->SendMessage(pData->pcpbd->rexxSelf, method, args);

                    if ( ! checkReplyIsGood(c, pData->pcpbd, reply, method, false) )
                    {
                        SendMessage(pred->hToolTip, TTM_ACTIVATE, 0, 0);

                        endDialogPremature(pData->pcpbd, pData->pcpbd->hDlg, RexxConditionRaised);
                        return 0;
                    }

                    c->ReleaseLocalReference(reply);
                    c->ReleaseLocalReference(args);
                    if ( rxToolID != TheNilObj )
                    {
                        c->ReleaseLocalReference(rxToolID);
                    }

                    return 0;
                }
            }
            break;

            default :
                break;
        }
    }

    if ( msg == WM_DESTROY )
    {
        SendMessage(pred->hToolTip, TTM_ACTIVATE, 0, 0);
    }
    if ( msg == WM_NCDESTROY )
    {
        /* The window is being destroyed, remove the subclass, clean up memory.
         * Note that with the current ooDialog architecture, this message,
         * usually, never gets here.  Freeing the subclass data struct has to be
         * done in the dialog control uninit() for those cases.  So, we just
         * always do it from the uninit().
         */
        RemoveWindowSubclass(hwnd, ManageAtypicalToolProc, id);
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}


/** PlainBaseDialog::createToolTip()
 *
 * Creates the Windows tool tip control and instantiates the Rexx ToolTip
 * object.
 *
 * @param rxID
 * @param styleFlags
 *
 * @return The Rexx ToolTip object on success, the .nil object on error.
 *
 * @notes ToolTip controls are somewhat different than other dialog controls in
 *        that they are not part of the dialog template, but rather independent
 *        windows owned by the dialog.  Because of this, if the .nil object is
 *        returned on error, it is possible that the .systemErrorCode may be of
 *        help in determining the cause of the error.
 *
 *        Sets the .systemErrorCode.
 *
 * @remarks  All other dialog controls are instantiated through pbdlg_newControl
 *           which carries the legacy baggage of having to accomadate the
 *           deprecated CategoryDialog.  The newer ToolTip control has a number
 *           of differences from other dialog controls, so it has its own
 *           'create' method here and its own 'new' method below.  Both methods
 *           are still a PlainBaseDialog method, we just put them here to keep
 *           the ToolTip stuff together.  We need to remember that the context
 *           is not the DialogControl context, it is the PlainBaseDialog
 *           context.
 *
 *           Tool tip controls are different than most other Window controls in
 *           that they are actually popup windows owned by the dialog, rather
 *           than child windows of the dialog.  Because of this we need to keep
 *           track of them by adding them to a table.
 *
 *           Because they are popup windows, we can not find an existing tool
 *           tip through GetDlgItem().  So, we add each created tool tip to a
 *           table in createToolTip() and look up an existing tool tip through
 *           its ID for the newToolTip() method. The sole purpose of this is to
 *           allow the Rexx programmer to do: newToolTip(ID) at any point in her
 *           program and get back the same object, as is possible with other
 *           dialog controls.
 *
 *           We raise a syntax condition if the user invokes createToolTip using
 *           an already existing ToolTip ID and raise a syntax condition if the
 *           user invokes newToolTip() on a non-existent ToolTip.
 */
RexxMethod3(RexxObjectPtr, pbdlg_createToolTip, RexxObjectPtr, rxID, OPTIONAL_CSTRING, styleFlags, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr toolTip = TheNilObj;
    CREATETOOLTIP ctt     = {0};

    if ( pCSelf == NULL )
    {
        baseClassInitializationException(context);
        goto out;
    }
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    HWND hDlg = pcpbd->hDlg;
    if ( hDlg == NULL )
    {
        noWindowsDialogException(context, "createToolTip", pcpbd->rexxSelf);
        goto out;
    }

    uint32_t id = oodResolveSymbolicID(context->threadContext, pcpbd->rexxSelf, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION  )
    {
        goto out;
    }

    if ( pcpbd->ToolTipTab == NULL && ! lazyInitToolTipTable(context, pcpbd) )
    {
        goto out;
    }

    PTOOLTIPTABLEENTRY ptte = findToolTipForID(pcpbd, id);
    if ( ptte != NULL )
    {
        methodCanNotBeInvokedException(context, "createToolTip", pcpbd->rexxSelf, TOOLTIP_CREATED_MSG);
        goto out;
    }

    RexxClassObject controlCls = oodClass4controlType(context, winToolTip);
    if ( controlCls == NULLOBJECT )
    {
        goto out;
    }

    uint32_t style = parseToolTipStyle(styleFlags);
    HWND hToolTip  = NULL;

    // Tool tips need to be created on the same thread as the dialog window procedure.
    if ( isDlgThread(pcpbd) )
    {
        hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, style, CW_USEDEFAULT, CW_USEDEFAULT,
                                  CW_USEDEFAULT, CW_USEDEFAULT, hDlg, NULL, pcpbd->hInstance, NULL);
        ctt.errRC = GetLastError();
    }
    else
    {
        ctt.hInstance = pcpbd->hInstance;
        ctt.style     = style;
        hToolTip      = (HWND)SendMessage(hDlg, WM_USER_CREATETOOLTIP, (WPARAM)&ctt, 0);
    }

    if ( hToolTip == NULL )
    {
        oodSetSysErrCode(context->threadContext, ctt.errRC);
        goto out;
    }

    SetWindowPos(hToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    PNEWCONTROLPARAMS pArgs = (PNEWCONTROLPARAMS)malloc(sizeof(NEWCONTROLPARAMS));
    if ( pArgs == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto out;
    }

    pArgs->isCatDlg    = false;
    pArgs->controlType = winToolTip;
    pArgs->hwnd        = hToolTip;
    pArgs->pcpbd       = pcpbd;
    pArgs->id          = id;

    RexxObjectPtr result = context->SendMessage1(controlCls, "NEW", context->NewPointer(pArgs));
    free(pArgs);

    if ( result != NULLOBJECT && result != TheNilObj )
    {
        if ( addToolTipToTable(context, pcpbd, result, hToolTip, id) )
        {
            toolTip = result;
        }
    }

out:
    return toolTip;
}


/** PlainBaseDialog::newToolTip()
 *
 * Returns an already instantiated Rexx ToolTip object.
 *
 * @param rxID  The resource ID of the ToolTip.
 *
 * @return The Rexx ToolTip object on success, the .nil object on error.
 *
 * @note  ToolTip controls are somewhat different than other dialog controls in
 *        that they are not part of the dialog template, but rather independent
 *        windows owned by the dialog.  Because of this, the underlying ToolTip
 *        control needs to be created by the programmer using the createToolTip
 *        method, after the underlying dialog has been created.
 *
 *        The createToolTip() method, creates the underlying ToolTip,
 *        instantiates the Rexx object, and returns it.  The newToolTip() method
 *        simply returns the alread instantiated Rexx object, as the other newXX
 *        methods do.
 *
 * @note  All other dialog controls are instantiated through pbdlg_newControl
 *        which carries the legacy baggage of having to accomadate the
 *        deprecated CategoryDialog.  The newer ToolTip control has a number of
 *        differences from other dialog controls, so it has its own 'new' method
 *        here.  The newToolTip() method is still a PlainBaseDialog method, we
 *        just put it here to keep the ToolTip stuff together.  We need to
 *        remember that the context is not the DialogControl context, it is the
 *        PlainBaseDialog context.
 *
 *        Because ToolTips are popup windows, we can not find an existing tool
 *        tip through GetDlgItem().  So, each created tool tip is added to a
 *        table and here, we look up an existing tool tip through its ID.  The
 *        sole purpose of this is to allow the Rexx programmer to do:
 *        newToolTip(ID) at any point in her program and get back the same
 *        object, as is possible with other dialog controls.
 */
RexxMethod2(RexxObjectPtr, pbdlg_newToolTip, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr toolTip = TheNilObj;
    CREATETOOLTIP ctt     = {0};

    if ( pCSelf == NULL )
    {
        baseClassInitializationException(context);
        goto out;
    }
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    HWND hDlg = pcpbd->hDlg;
    if ( hDlg == NULL )
    {
        noWindowsDialogException(context, "newToolTip", pcpbd->rexxSelf);
        goto out;
    }

    uint32_t id = oodResolveSymbolicID(context->threadContext, pcpbd->rexxSelf, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION  )
    {
        goto out;
    }

    if ( pcpbd->ToolTipTab == NULL )
    {
        methodCanNotBeInvokedException(context, "newToolTip", pcpbd->rexxSelf, TOOLTIP_NOT_CREATED_MSG);
        goto out;
    }

    PTOOLTIPTABLEENTRY ptte = findToolTipForID(pcpbd, id);
    if ( ptte == NULL )
    {
        methodCanNotBeInvokedException(context, "newToolTip", pcpbd->rexxSelf, TOOLTIP_NOT_CREATED_MSG);
        goto out;
    }

    toolTip = ptte->rexxSelf;

out:
    return toolTip;
}


/** ToolTip::activate()
 *
 *  Activates or deactivates this tool tip.
 *
 *  @param activate  [optional] If true activates this tool tip, if false
 *                   deactivates this tool tip.  The default if omitted is true.
 *
 *  @return  0, always.
 *
 */
RexxMethod2(uint32_t, tt_activate, OPTIONAL_logical_t, activate, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return FALSE;
    }

    if ( argumentOmitted(1) )
    {
        activate = TRUE;
    }

    SendMessage(pcdc->hCtrl, TTM_ACTIVATE, activate, 0);

    return 0;
}


/** ToolTip::addTool()
 *
 *  Adds a tool to this tool tip.
 *
 *  @param tool   [required]  The dialog control that defines the tool being
 *                added.
 *
 *  @param text   [optional] Text for the tool.  If omitted, or the empty
 *                string, or the string: TEXTCALLBACK then the tool tip sends
 *                the NEEDTEXT notification and the program supplies the text.
 *
 *                The maximum length of the text is 1023 characters, which
 *                includes any possible 0x0D0A sequences.
 *
 *  @param flags  [optional] Keywords for the TTF_* flags.  If omitted flags are
 *                automatically set to TTF_IDISHWND | TTF_SUBCLASS.  If not
 *                omitted, flags are set to whatever is specified.  However,
 *                TTF_IDISHWND and TTF_SUBCLASS are always set.  (Because uID is
 *                always set to hwnd of tool, and for the simple case that
 *                addTool() is intended for, the dialog control should always be
 *                subclassed.)
 *
 *  @param userData  [optional]  A user data value to be associated with the
 *                   tool.  Note that the value is associated with the tool, not
 *                   the tool tip.
 *
 *  @return  True on success, false on error.
 *
 *  @notes    The addTool() method is intended to be a convenient method for use
 *            in the most common case of adding a tool.  The case where the tool
 *            is a simple dialog control.  Use addToolRect() or addToolEx() to
 *            add a tool with characteristics that need to be more explicitly
 *            defined.
 *
 *            Many methods require the indentity of the tool.  A tool id is
 *            defined by a hwnd and a unique ID. For a tool added through
 *            addTool(), the identity will always be the hwnd of the owner
 *            dialog of the tool (the dialog control passed in as the tool arg)
 *            and the unique ID is always the hwnd of the tool (the dialog
 *            control passed in as the tool arg.)
 */
RexxMethod5(logical_t, tt_addTool, RexxObjectPtr, tool, OPTIONAL_CSTRING, text, OPTIONAL_CSTRING, _flags,
            OPTIONAL_RexxObjectPtr, userData, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return FALSE;
    }

    if ( ! requiredClass(context->threadContext, tool, "DIALOGCONTROL", 1) )
    {
        return FALSE;
    }
    pCDialogControl pcdcTool = controlToCSelf(context, tool);

    if ( argumentOmitted(2) )
    {
        text = "";
    }

    size_t l = strlen(text);
    if ( l > MAX_TOOLINFO_TEXT_LENGTH )
    {
        stringTooLongException(context->threadContext, 2, MAX_TOOLINFO_TEXT_LENGTH, l);
        return false;
    }
    if ( l == 0 || StrCmpI("TEXTCALLBACK", text) == 0 )
    {
        text = LPSTR_TEXTCALLBACK;
    }

    uint32_t flags = TTF_IDISHWND | TTF_SUBCLASS;
    if ( argumentExists(3) )
    {
        uint32_t temp = keyword2ttfFlags(context, _flags);
        if ( temp == OOD_ID_EXCEPTION )
        {
            return false;
        }
        flags |= temp;
    }

    TOOLINFO ti = { sizeof(ti) };

    ti.uFlags   = flags;
    ti.uId      = (UINT_PTR)pcdcTool->hCtrl;
    ti.hwnd     = pcdc->hDlg;
    ti.lpszText = (LPSTR)text;
    ti.lParam   = (LPARAM)(argumentExists(4) ? userData : 0);

    return SendMessage(pcdc->hCtrl, TTM_ADDTOOL, 0, (LPARAM)&ti);
}


/** ToolTip::addToolEx()
 *
 *  Adds the tool specified to this tool tip.
 *
 *  @param  toolInfo  [required]  A .ToolInfo object that defines the tool being
 *                    added.
 *
 *  @return  True on success, false on error.
 *
 *  @notes  There are a number of different attributes that can be set when
 *          adding a tool to a tool tip.  The addToolEx() method is designed to
 *          let the programmer specify any valid combination of attributes
 *          allowed by the operating system.  To do this, it requires the
 *          specifier of the tool to be a .ToolInfo object.  The programmer is
 *          responsible for setting the tool attriubtes as he wishes.
 *
 *          For the simple case of adding a tool that is a dialog control, the
 *          addTool() is a more convenient method.
 */
RexxMethod2(logical_t, tt_addToolEx, RexxObjectPtr, toolInfo, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return FALSE;
    }
    if ( ! requiredClass(context->threadContext, toolInfo, "ToolInfo", 1) )
    {
        return FALSE;
    }

    LPTOOLINFO pTI = (LPTOOLINFO)context->ObjectToCSelf(toolInfo);
    return SendMessage(pcdc->hCtrl, TTM_ADDTOOL, 0, (LPARAM)pTI);
}


/** ToolTip::addToolRect()
 *
 *  Adds a tool that uses a rectangular area in the dialog as its trigger point.
 *
 *  @param  dlg       The dialog the tool belongs to.
 *  @param  id        An ID that uniquely identifies the tool being added.
 *  @param  rect      The rectangle, relative to the client area of the dialog,
 *                    that the tool tip shows for.
 *  @param  text      The text for the tool tip.
 *  @param  flags     The flags for the tool tip.
 *  @param  userData  User data to associate with the tool.  Note that the user
 *                    data is associated with the tool, not the tool tip.
 *
 */
RexxMethod7(logical_t, tt_addToolRect, RexxObjectPtr, dlg, RexxObjectPtr, rxID, RexxObjectPtr, _rect,
            OPTIONAL_CSTRING, text, OPTIONAL_CSTRING, _flags, OPTIONAL_RexxObjectPtr, userData, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return FALSE;
    }

    if ( ! requiredClass(context->threadContext, dlg, "PLAINBASEDIALOG", 1) )
    {
        return FALSE;
    }
    pCPlainBaseDialog pcpbd = dlgToCSelf(context, dlg);

    uint32_t id = oodResolveSymbolicID(context->threadContext, pcpbd->rexxSelf, rxID, -1, 2, false);
    if ( id == OOD_ID_EXCEPTION  )
    {
        return FALSE;
    }

    PRECT pRect = (PRECT)rxGetRect(context, _rect, 3);
    if ( pRect == NULL )
    {
        return FALSE;
    }

    if ( argumentOmitted(4) )
    {
        text = "";
    }

    size_t l = strlen(text);
    if ( l > MAX_TOOLINFO_TEXT_LENGTH )
    {
        stringTooLongException(context->threadContext, 4, MAX_TOOLINFO_TEXT_LENGTH, l);
        return FALSE;
    }
    if ( l == 0 || StrCmpI("TEXTCALLBACK", text) == 0 )
    {
        text = LPSTR_TEXTCALLBACK;
    }

    uint32_t flags = TTF_SUBCLASS;
    if ( argumentExists(5) )
    {
        uint32_t temp = keyword2ttfFlags(context, _flags);
        if ( temp == OOD_ID_EXCEPTION )
        {
            return FALSE;
        }
        flags |= temp;
        flags &= ~TTF_IDISHWND;
    }

    TOOLINFO ti = {sizeof(ti)};

    ti.uFlags   = flags;
    ti.hwnd     = pcdc->hDlg;
    ti.uId      = id;
    ti.lpszText = (LPSTR)text;

    CopyRect(&ti.rect, pRect);

    if ( argumentExists(6) )
    {
        ti.lParam   = (LPARAM)userData;
    }
    return SendMessage(pcdc->hCtrl, TTM_ADDTOOL, 0, (LPARAM)&ti);
}


/** ToolTip::adjustRect()
 *
 *  Calculates a ToolTip control's text display rectangle from its window
 *  rectangle, or the ToolTip window rectangle needed to display a specified
 *  text display rectangle.
 *
 *  @param rect  [required] A Rectangle object used to specify the rectangle to
 *               adjust.  On a successful return, the co-ordinates in the
 *               rectangle will be adjusted as specified byt the larger arguent.
 *
 *  @param larger  [optional] True or false to specify how the rectangle is
 *                 adjusted.  If omitted, the default is false.
 *
 *                 If true, rect is used to specify a text-display rectangle and
 *                 it receives the corresponding window rectangle.  The received
 *                 rectangle is *larger* in this case.  If false, rect is used
 *                 to specify a window rectangle and it receives the
 *                 corresponding text display rectangle.
 */
RexxMethod3(logical_t, tt_adjustRect, RexxObjectPtr, _rect, OPTIONAL_logical_t, larger, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return FALSE;
    }

    PRECT r = (PRECT)rxGetRect(context, _rect, 1);
    if ( r == NULL )
    {
        return FALSE;
    }

    return SendMessage(pcdc->hCtrl, TTM_ADJUSTRECT, larger, (LPARAM)r);
}


/** ToolTip::delTool()
 *
 *  Removes the specified tool from this tool tip.
 *
 *  @param toolHwnd   [required]
 *
 *  @param toolID     [optional]
 *
 *  @return  Returns 0 always.
 *
 *  @notes  toolHwnd and toolId are the Rexx object combination that uniquely
 *          specifies a tool to this tool tip.
 *
 */
RexxMethod3(uint32_t, tt_delTool, RexxObjectPtr, toolID, OPTIONAL_RexxObjectPtr, uID, CSELF, pCSelf)
{
    TOOLINFO ti = { sizeof(ti) };

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    if ( ! genericToolID(context, toolID, uID, &ti, NULL, NULL, 1) )
    {
        goto done_out;
    }

    SendMessage(pcdc->hCtrl, TTM_DELTOOL, 0, (LPARAM)&ti);

done_out:
    return 0;
}


/** ToolTip::enumTools()
 *
 *  Retrieves a .ToolInfo object for the tool at the specified index.
 *
 *  @param index  [optional]  Index of the tool to retrieve.  MSDN does not
 *                specify, but it seems to be zero-based indexes.  We will use
 *                1-based indexes.  If omitted, defaults to index 1.
 *
 *  @return  The tool at the specified index, as a .ToolInfo object, or .nil if
 *           no tool exists at the index.
 */
RexxMethod2(RexxObjectPtr, tt_enumTools, OPTIONAL_uint32_t, index, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    if ( argumentExists(1) )
    {
        if ( index < 1 )
        {
            wrongRangeException(context, 1, 1, UINT32_MAX, index);
            goto done_out;
        }
        index--;
    }
    else
    {
        index = 1;
    }

    LPTOOLINFO       pTI;
    RexxBufferObject tiBuf = rexxBufferForToolInfo(context, &pTI, true);
    if ( tiBuf == NULLOBJECT )
    {
        goto done_out;
    }

    if ( SendMessage(pcdc->hCtrl, TTM_ENUMTOOLS, index, (LPARAM)pTI) )
    {
        result = ti2rexxToolInfo(context, tiBuf, pTI, true);
    }

    if ( result == TheNilObj )
    {
        safeLocalFree(pTI->lpszText);
    }

done_out:
    return result;
}


/** ToolTip::getBubbleSize()
 *
 *  Returns the width and height, as a .Size object, of a tool tip control.
 *
 *  @param toolHwnd   [required]
 *
 *  @param toolID     [optional]
 *
 *  @returns  A .Size object specifying the width and height of this tool tip
 *            control.
 *
 *  @notes  toolHwnd and toolId are the Rexx object combination that uniquely
 *          specifies a tool to this tool tip.
 */
RexxMethod3(RexxObjectPtr, tt_getBubbleSize, RexxObjectPtr, toolHwnd, OPTIONAL_RexxObjectPtr, toolID, CSELF, pCSelf)
{
    RexxObjectPtr size = TheNilObj;
    TOOLINFO      ti   = { sizeof(ti) };

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    LPTOOLINFO pTI = &ti;
    if ( context->IsOfType(toolHwnd, "TOOLINFO") )
    {
        pTI = (LPTOOLINFO)context->ObjectToCSelf(toolHwnd);
    }
    else if ( ! genericToolID(context, toolHwnd, toolID, pTI, NULL, NULL, 1) )
    {
        goto done_out;
    }

    LPARAM lResult = SendMessage(pcdc->hCtrl, TTM_GETBUBBLESIZE, 0, (LPARAM)pTI);

    size = rxNewSize(context, LOWORD(lResult), HIWORD(lResult));

done_out:
    return size;
}


/** ToolTip::getCurrentTool()
 *
 *  Retrieves a .ToolInfo object whose attributes represent the current tool in
 *  this tool tip.
 *
 *  @return  Returns the current tool as a .ToolInfo object, if one exists, or
 *           the .nil object if there is not a current tool.
 *
 *  @remarks  We use getToolIdentifiers() to try and obtain the Rexx objects
 *            that would represent TOOLINFO.hwnd and TOOLINFO.uID.  This is not
 *            foolproof, so if the function fails, it sets hwndSupplier and
 *            idSupplier to the .nil objects.
 */
RexxMethod1(RexxObjectPtr, tt_getCurrentTool, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    LPTOOLINFO       pTI;
    RexxBufferObject tiBuf = rexxBufferForToolInfo(context, &pTI, true);
    if ( tiBuf == NULLOBJECT )
    {
        goto done_out;
    }

    if ( SendMessage(pcdc->hCtrl, TTM_GETCURRENTTOOL, 0, (LPARAM)pTI) )
    {
        result = ti2rexxToolInfo(context, tiBuf, pTI, true);
    }

    if ( result == TheNilObj )
    {
        safeLocalFree(pTI->lpszText);
    }

done_out:
    return result;
}


/** ToolTip::getDelayTime()
 *
 *  Retrieves one of the 3 delay times currently set for this tool tip.
 *
 *  @param  [optional] Keyword indicating which of the 3 possible delay times is
 *          sought. If omitted, defaults to AUTOPOP.  Valid keywords are:
 *          AUTOPOP, INITIAL, RESHOW, case is not significant.
 *
 *  @return The delay time for the specified event in milliseconds.
 *
 *  @notes  A tool tip sets timers for 3 diffent events.  They are:
 *
 *          AUTOPOP - The amount of time the ToolTip window remains visible if
 *          the pointer is stationary within a tool's bounding rectangle.
 *
 *          INITIAL - The amount of time the pointer must remain stationary
 *          within a tool's bounding rectangle before the ToolTip window
 *          appears.
 *
 *          RESHOW - The amount of time it takes for subsequent ToolTip windows
 *          to appear as the pointer moves from one tool to another.
 *
 *          Raises a syntax condition if an incorrect keyword is used.
 */
RexxMethod2(int32_t, tt_getDelayTime, OPTIONAL_CSTRING, type, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return FALSE;
    }

    uint32_t flag = TTDT_AUTOPOP;
    if ( argumentExists(1) )
    {
        if ( StrCmpI(type,      "AUTOPOP") == 0 ) flag = TTDT_AUTOPOP;
        else if ( StrCmpI(type, "INITIAL") == 0 ) flag = TTDT_INITIAL;
        else if ( StrCmpI(type, "RESHOW" ) == 0 ) flag = TTDT_RESHOW;
        else
        {
            wrongArgValueException(context->threadContext, 1, "AUTOPOP, INITIAL, or RESHOW", type);
            return 0;
        }
    }

    return (int32_t)SendMessage(pcdc->hCtrl, TTM_GETDELAYTIME, flag, 0);
}


/** ToolTip::getMargin()
 *
 *  Returns a rectangle that describes the margins of the tool tip.
 *
 *  @notes  The attributes of the returned rectangle do not define a bounding
 *          rectangle. For the purpose of this method, the structure members are
 *          interpreted as follows:
 *
 *          top    - Distance between top border and top of ToolTip text, in
 *                   pixels.
 *
 *          left   - Distance between left border and left end of ToolTip
 *                   text, in pixels.
 *
 *          bottom - Distance between bottom border and bottom of ToolTip text,
 *                   in pixels.
 *
 *          right  - Distance between right border and right end of ToolTip
 *                   text, in pixels.
 *
 */
RexxMethod1(RexxObjectPtr, tt_getMargin, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return FALSE;
    }

    RECT r;
    SendMessage(pcdc->hCtrl, TTM_GETMARGIN, 0, (LPARAM)&r);

    return rxNewRect(context, (PORXRECT)&r);
}


/** ToolTip::getMaxTipWidth()
 *
 *  Retrieves the maximum width for a ToolTip window.
 *
 *  @return Returns a whole number value that represents the maximum ToolTip
 *          width, in pixels. If no maximum width was set previously, the
 *          method returns -1.
 *
 *  @note   The maximum ToolTip width value does not indicate a ToolTip window's
 *          actual width. Rather, if a ToolTip string exceeds the maximum width,
 *          the control breaks the text into multiple lines, using spaces or
 *          newline characters to determine line breaks. If the text cannot be
 *          segmented into multiple lines, it will be displayed on a single
 *          line. The length of this line may exceed the maximum ToolTip width.
 */
RexxMethod1(int32_t, tt_getMaxTipWidth, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return FALSE;
    }

    return (int32_t)SendMessage(pcdc->hCtrl, TTM_GETMAXTIPWIDTH, 0, 0);
}


/** ToolTip::getText()
 *
 *  Retrieves the text information this ToolTip control maintains about the
 *  specified tool.
 *
 *  @param toolHwnd   [required]
 *
 *  @param toolID     [optional]
 *
 *  @return  The text string for the specified tool
 *
 *  @notes  toolHwnd and toolId are the Rexx object combination that uniquely
 *          specifies a tool to this tool tip.
 *
 *  @remarks  It appears that even if the TOOLINF.lpszText for a tool is set to
 *            LPSTR_TEXTCALLBACK, the TTM_GETTEXT message will not set
 *            TOOLINFO.lpszText to that value.  It would seem we could rely on
 *            an empty string, and not need worry aobut getting
 *            LPSTR_TEXTCALLBACK.  Still, we leave the check in, it seems little
 *            cost to ensure the interpreter does not blow up for someone.
 */
RexxMethod3(RexxObjectPtr, tt_getText, RexxObjectPtr, toolID, OPTIONAL_RexxObjectPtr, uID, CSELF, pCSelf)
{
    RexxObjectPtr result = context->NullString();
    TOOLINFO      ti     = { sizeof(ti) };

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    if ( ! genericToolID(context, toolID, uID, &ti, NULL, NULL, 1) )
    {
        goto done_out;
    }

    ti.lpszText = (LPSTR)LocalAlloc(LPTR, MAX_TOOLINFO_TEXT_LENGTH + 1);
    if ( ti.lpszText == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    if ( _isAtLeastVista() )
    {
        SendMessage(pcdc->hCtrl, TTM_GETTEXT, MAX_TOOLINFO_TEXT_LENGTH + 1, (LPARAM)&ti);
    }
    else
    {
        SendMessage(pcdc->hCtrl, TTM_GETTEXT, 0, (LPARAM)&ti);
    }

    if ( ti.lpszText == LPSTR_TEXTCALLBACK )
    {
        result = context->String("TextCallBack");
    }
    else
    {
        result = context->String(ti.lpszText);
    }

    LocalFree(ti.lpszText);

done_out:
    return result;
}


/** ToolTip::getTipBkColor()
 *
 *  Retrieves the background color for this ToolTip window.
 *
 *  @return  A COLORREF value that represents the background color.
 */
RexxMethod1(uint32_t, tt_getTipBkColor, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }
    return (uint32_t)SendMessage(pcdc->hCtrl, TTM_GETTIPBKCOLOR, 0, 0);
}


/** ToolTip::getTipTextColor()
 *
 *  Retrieves the text color fot this ToolTip window.
 *
 *  @return  A COLORREF value that represents the text color.
 */
RexxMethod1(uint32_t, tt_getTipTextColor, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }
    return (uint32_t)SendMessage(pcdc->hCtrl, TTM_GETTIPTEXTCOLOR, 0, 0);
}


/** ToolTip::getTitle()
 *
 *  Retrieve information concerning the title and icon of this ToolTip control.
 *
 *  @return  A directory object whose indexes contain the title and icon
 *           information for this ToolTip.  Indexes are:
 *
 *           TITLE     - The title text.
 *
 *           ICON      - The image icon, or the keyword if the icon is a system
 *                       icon.
 *
 *           ISKEYWORD - True if the value at index ICON is a keyword, false if
 *                       it is an icon image
 *
 *  @note  The underlying Windows API for getTitle appears to be idiosyncratic.
 *
 *         In testing, when setting the icon to an actual icon image, getting
 *         the icon always returns the INFOLARGE keyword, rather than the icon
 *         image. In addition, when setting the icon to any of the LARGE keyword
 *         values, getting the icon always returns the non-large keyword.  This
 *         anomly is mentioned in several places on the web.  The following
 *         shows this behaviour:
 *
 *         setTitle icon value -> hIcon        getTitle -> INFOLARGE
 *
 *         setTitle icon value -> INFOLARGE    getTitle -> INFO
 *
 *         setTitle icon value -> WARNINGLARGE getTitle -> WARNING
 *
 *         setTitle icon value -> ERRORLARGE   getTitle -> ERROR
 */
RexxMethod1(RexxObjectPtr, tt_getTitle, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }

    WCHAR buf[TT_CCH_TOOLTITLE_BUF] = { 0 };

    TTGETTITLE tt = { 0 };

    tt.dwSize   = sizeof(TTGETTITLE);
    tt.pszTitle = buf;
    tt.cch      = TT_CCH_TOOLTITLE_BUF;

    SendMessage(pcdc->hCtrl, TTM_GETTITLE, 0, (LPARAM)&tt);

    RexxObjectPtr rxIcon;
    RexxObjectPtr isKeyword = TheTrueObj;
    if ( tt.uTitleBitmap > TT_MAX_ICON_KEYWORD )
    {
        SIZE s = {16, 16};

        rxIcon = rxNewValidImage(context, (HWND)(UINT_PTR)tt.uTitleBitmap, IMAGE_ICON, &s, 0, false); // double cast avoids C4312
        if ( rxIcon == NULLOBJECT )
        {
            rxIcon = TheNilObj;
        }
        isKeyword = TheFalseObj;
    }
    else
    {
        CSTRING str = "";

        if (      tt.uTitleBitmap == TTI_NONE          ) str = "NONE";
        else if ( tt.uTitleBitmap == TTI_ERROR         ) str = "ERROR";
        else if ( tt.uTitleBitmap == TTI_ERROR_LARGE   ) str = "ERRORLARGE";
        else if ( tt.uTitleBitmap == TTI_INFO          ) str = "INFO";
        else if ( tt.uTitleBitmap == TTI_INFO_LARGE    ) str = "INFOLARGE";
        else if ( tt.uTitleBitmap == TTI_WARNING       ) str = "WARNING";
        else if ( tt.uTitleBitmap == TTI_WARNING_LARGE ) str = "WARNINGLARGE";

        rxIcon = context->String(str);
    }
    RexxDirectoryObject d = context->NewDirectory();

    context->DirectoryPut(d, unicode2string(context, buf), "TITLE");
    context->DirectoryPut(d, rxIcon, "ICON");
    context->DirectoryPut(d, isKeyword, "ISKEYWORD");

    return d;
}


/** ToolTip::getToolCount()
 *
 *  Retrieves the number of tools this tool tip contains.
 *
 *  @return  The number of tools this tool tip contains.
 */
RexxMethod1(uint32_t, tt_getToolCount, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }

    return (uint32_t)SendMessage(pcdc->hCtrl, TTM_GETTOOLCOUNT, 0, 0);
}


/** ToolTip::getToolInfo()
 *
 *  Retrieves the information, as a .ToolInfo object, that this tool tip control
 *  maintains about the specified tool.
 *
 *  @param toolHwnd   [required]
 *
 *  @param toolID     [optional]
 *
 *  @return  A .ToolInfo object whose attributes reflect the information that
 *           this tool tip maintains about the specified tool.  On error, .nil
 *           is returned.
 *
 *  @notes   toolHwnd and toolId are the Rexx object combination that uniquely
 *           specifies a tool to this tool tip.
 */
RexxMethod3(RexxObjectPtr, tt_getToolInfo, RexxObjectPtr, toolHwnd, OPTIONAL_RexxObjectPtr, toolID, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    LPTOOLINFO       pTI;
    RexxBufferObject tiBuf = rexxBufferForToolInfo(context, &pTI, true);
    if ( tiBuf == NULLOBJECT )
    {
        goto done_out;
    }

    if ( ! genericToolID(context, toolHwnd, toolID, pTI, NULL, NULL, 1) )
    {
        goto done_out;
    }

    if ( SendMessage(pcdc->hCtrl, TTM_GETTOOLINFO, 0, (LPARAM)pTI) )
    {
        result = ti2rexxToolInfo(context, tiBuf, pTI, true);
    }

    if ( result == TheNilObj )
    {
        safeLocalFree(pTI->lpszText);
    }

done_out:
    return result;
}


/** ToolTip::hasCurrentTool()
 *
 *  Tests if this tool tip has a current tool.
 *
 *  @return  True if there is a current tool, false if there is not.
 *
 *  @notes  In essence, this convenience method tests if getCurrentTool()
 *          will return a .ToolInfo object or the .nil object.
 */
RexxMethod1(logical_t, tt_hasCurrentTool, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }
    return SendMessage(pcdc->hCtrl, TTM_GETCURRENTTOOL, 0, NULL);
}


/** ToolTip::hitTestInfo()
 *
 *  Tests a point to determine whether it is within the bounding rectangle of a
 *  tool within the window specified and, if it is, retrieves information about
 *  the tool.
 *
 *  @param  toolInfo   [required in / out]  A .ToolInfo object whose rexxHwnd
 *                     attribute specifies which tool window to test.
 *
 *                     If the point tested is within a tool of the tool window,
 *                     the retrieved tool information is returned in this
 *                     object. The tool info object should be instantiated
 *                     using the ToolInfo::forHitTest() class method.
 *
 *  @param pointArgs   [required]  The position to test, in client coordinates
 *                     of the window specified in the toolInfo argument.
 *
 *                     The position can be specified in these formats:
 *
 *      Form 1:  A .Point object.
 *      Form 2:  x, y
 *
 *  @return True if the point being tested is within the window specified,
 *          otherwise false.
 *
 *  @remarks  Erroneously called hitTest in ooDialog 4.2.1.  Now both hitTest
 *            and hitTestInfo are mapped to this method.  Need to preserve
 *            hitTest for backwards compatibility, but only hitTestInfo is
 *            documented from 4.2.2 onwards.
 */
RexxMethod3(logical_t, tt_hitTestInfo, RexxObjectPtr, toolInfo, ARGLIST, args, CSELF, pCSelf)
{
    TTHITTESTINFO hi  = { 0 };

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto err_out;
    }

    if ( ! context->IsOfType(toolInfo, "TOOLINFO") )
    {
        wrongClassException(context->threadContext, 1, "ToolInfo", toolInfo);
        goto err_out;
    }

    size_t arraySize;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 2, 3, &arraySize, &argsUsed) )
    {
        goto err_out;
    }

    LPTOOLINFO pTI = (LPTOOLINFO)context->ObjectToCSelf(toolInfo);

    hi.ti.lpszText = (LPSTR)LocalAlloc(LPTR, MAX_TOOLINFO_TEXT_LENGTH + 1);
    if ( hi.ti.lpszText == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto err_out;
    }

    hi.ti.cbSize   = sizeof(TOOLINFO);
    hi.pt.x        = point.x;
    hi.pt.y        = point.y;
    hi.hwnd        = pTI->hwnd;

    memset(pTI, 0, sizeof(TOOLINFO));
    pTI->cbSize = sizeof(TOOLINFO);

    if ( SendMessage(pcdc->hCtrl, TTM_HITTEST, 0, (LPARAM)(LPHITTESTINFO)&hi) )
    {
        memcpy(pTI, &hi.ti, sizeof(TOOLINFO));

        RexxObjectPtr hwndSupplier;
        RexxObjectPtr uIDSupplier;

        getToolIdentifiers(context, pTI, &hwndSupplier, &uIDSupplier);

        context->SendMessage2(toolInfo, "HITTESTHELPER", hwndSupplier, uIDSupplier);

        return TRUE;
    }
    else
    {
        LocalFree(hi.ti.lpszText);
    }

err_out:
    return FALSE;
}


/** ToolTip::manageAtypicalTool
 *
 *  Initiates the management of a tool tip tool that is a dialog control.
 *
 *  @param  toolObject  [required] The tool dialog control to be managed.
 *
 *  @param  events      [optional] The tool tip events that should invoke a
 *                      method in the Rexx dialog.  The array can not be sparse.
 *                      Each index in the array should contain the keyword for a
 *                      tool tip event.  If an event keyword is present at an
 *                      index, then that event is connected to a method in the
 *                      Rexx dialog.  By default the event is connect to these
 *                      methods:
 *
 *                      RELAY     -> onRelay
 *                      NEEDTEXT  -> onNeedText
 *                      SHOW      -> onShow
 *                      POP       -> onPop
 *                      LINKCLICK -> onLinkClick
 *
 *                      If the default method name is not suitable, for whatever
 *                      reason, then the programmer can supply her own name in
 *                      the methods array.  The alternative method name must be
 *                      at the same position in the methods array as the
 *                      keywords postion in the events array.
 *
 *                      E.g., if at the keyword SHOW is present at index 2 in
 *                      the events array and the programmer wants to over-ride
 *                      the default method name of onShow, the this can be done
 *                      by putting the alternative method name at index 2 in the
 *                      methods array.  There is no requirement to put any name
 *                      at index 1.
 *
 *                      In addition to the 5 event keywords, the keyword NORELAY
 *                      can be used.  This keyword has the effect of turning off
 *                      the relaying of the mouse messages to the tool.  If this
 *                      keyword is used, there is no matching method to be
 *                      invoked in the Rexx dialog.
 *
 *  @param  methods     [optional] Alternative method names to use for the
 *                      matching event in the events arguemnt.  This array can
 *                      be sparse.  For each event in the events array, if the
 *                      same index in the methods array has a value, then that
 *                      value is used as the method to be invoked for that
 *                      event.
 *
 *  @notes  Requires common control library 6.0.  Raises a syntax condition if
 *          incorrect usage is detected.
 *
 *          The management of the tool *always* relays the mouse events to the
 *          the tool tip control, unless the event keyword NoRelay is used. If
 *          the optional events and methods arguments are omitted, then the only
 *          thing done is the relay event.
 *
 *  @remarks
 */
RexxMethod4(logical_t, tt_manageAtypicalTool, RexxObjectPtr, toolObject, OPTIONAL_RexxArrayObject, events,
            OPTIONAL_RexxArrayObject, methods, CSELF, pCSelf)
{
    pCDialogControl subClassCtrl = NULL;
    pSubClassData   pSCData      = NULL;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto err_out;
    }

    if ( ! requiredComCtl32Version(context,  "manageAtypicalTool", COMCTL32_6_0) )
    {
        goto err_out;
    }

    if ( context->IsOfType(toolObject, "PLAINBASEDIALOG") )
    {
        userDefinedMsgException(context->threadContext, 1, "manageAtypicalTool() can not be used on dialog tools");
        goto err_out;
    }
    else if ( context->IsOfType(toolObject, "DIALOGCONTROL") )
    {
        subClassCtrl = controlToCSelf(context, toolObject);
    }
    else
    {
        wrongClassListException(context->threadContext, 1, "PlainBaseDialog or DialogControl", toolObject);
        goto err_out;
    }

    if ( subClassCtrl->pRelayEvent != NULL )
    {
        char buf[256];

        _snprintf(buf, sizeof(buf), "the windows %s tool is already managed",
                 controlType2winName(subClassCtrl->controlType));
        userDefinedMsgException(context, 1, buf);
        goto err_out;
    }

    pSCData = (pSubClassData)LocalAlloc(LPTR, sizeof(SubClassData));
    if ( pSCData == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto err_out;
    }

    pSCData->pcpbd = subClassCtrl->pcpbd;
    pSCData->pcdc  = subClassCtrl;
    pSCData->hCtrl = subClassCtrl->hCtrl;
    pSCData->id    = subClassCtrl->id;

    subClassCtrl->pRelayEvent = pSCData;

    pRelayEventData pred = (pRelayEventData)LocalAlloc(LPTR, sizeof(RelayEventData));
    if ( pred == NULL )
    {
        freeRelayData(pSCData);
        LocalFree(pSCData);
        outOfMemoryException(context->threadContext);
        goto err_out;
    }

    pSCData->pData  = pred;
    pSCData->pfn    = freeRelayData;
    pred->hToolTip  = pcdc->hCtrl;
    pred->rxToolTip = pcdc->rexxSelf;

    if ( argumentExists(2) )
    {
        size_t count = context->ArrayItems(events);

        for ( size_t i = 1; i <= count; i++ )
        {
            RexxObjectPtr eventName = context->ArrayAt(events, i);
            if ( eventName == NULLOBJECT )
            {
                sparseArrayException(context->threadContext, 2, i);
                goto err_out;
            }

            CSTRING  mthName;
            CSTRING  evtName = context->ObjectToStringValue(eventName);
            uint32_t idx     = matchEvent2index(context, evtName, &mthName, i, 2);
            if ( idx == OOD_ID_EXCEPTION )
            {
                goto err_out;
            }

            if ( idx == RE_NORELAY_IDX )
            {
                pred->skipRelay = true;
                continue;
            }

            if ( argumentExists(3) )
            {
                RexxObjectPtr rxName = context->ArrayAt(methods, i);
                if ( rxName != NULLOBJECT )
                {
                    mthName = context->ObjectToStringValue(rxName);
                }
            }

            pred->doEvent[idx] = true;
            pred->methods[idx] = (char *)LocalAlloc(LPTR, strlen(mthName) + 1);
            if ( pred->methods[idx] == NULL )
            {
                outOfMemoryException(context->threadContext);
                goto err_out;
            }
            strcpy(pred->methods[idx], mthName);
        }

    }

    BOOL success;
    if ( isDlgThread(pcdc->pcpbd) )
    {
        success = SetWindowSubclass(subClassCtrl->hCtrl, ManageAtypicalToolProc, subClassCtrl->id, (DWORD_PTR)pSCData);
    }
    else
    {
        success = (BOOL)SendMessage(subClassCtrl->pcpbd->hDlg, WM_USER_SUBCLASS, (WPARAM)ManageAtypicalToolProc, (LPARAM)pSCData);
    }

    if ( ! success )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "SetWindowSubclass");
        goto err_out;
    }

    return TRUE;

err_out:
    freeRelayData(pSCData);
    LocalFree(pSCData);
    return FALSE;
}


/** ToolTip::newToolRect()
 *
 *  Sets a new bounding rectangle for a tool.
 *
 *  @param rect       [required]  The new bounding rectangle for the tool.
 *
 *  @param toolHwnd   [required]
 *
 *  @param toolID     [optional]
 *
 *  @return  Returns 0 always.
 *
 *  @notes  toolHwnd and toolId are the Rexx object combination that uniquely
 *          specifies a tool to this tool tip.
 *
 */
RexxMethod4(uint32_t, tt_newToolRect, RexxObjectPtr, r, RexxObjectPtr, toolID, OPTIONAL_RexxObjectPtr, uID, CSELF, pCSelf)
{
    TOOLINFO ti = { sizeof(ti) };

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    if ( ! genericToolID(context, toolID, uID, &ti, NULL, NULL, 2) )
    {
        goto done_out;
    }

    PRECT pRect = (PRECT)rxGetRect(context, r, 1);
    if ( pRect == NULL )
    {
        goto done_out;
    }
    CopyRect(&ti.rect, pRect);

    SendMessage(pcdc->hCtrl, TTM_NEWTOOLRECT, 0, (LPARAM)&ti);

done_out:
    return 0;
}


/** ToolTip::pop()
 *
 *  Removes a displayed ToolTip window from view.
 *
 *  @return  0, always.
 */
RexxMethod1(uint32_t, tt_pop, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }

    SendMessage(pcdc->hCtrl, TTM_POP, 0, 0);

    return 0;
}


/** ToolTip::popUp()
 *
 *  Causes this ToolTip to display at the coordinates of the last mouse message.
 *
 *  @return  0, always.
 *
 *  @notes Requires common control library version 6.
 */
RexxMethod1(uint32_t, tt_popUp, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }

    if ( ! requiredComCtl32Version(context, "popup", COMCTL32_6_0) )
    {
        return 0;
    }

    SendMessage(pcdc->hCtrl, TTM_POPUP, 0, 0);

    return 0;
}


/** ToolTip::setDelayTime()
 *
 *  Sets the initial, pop-up, and reshow durations for this ToolTip control.
 *
 *  @param  which  [optional] Keyword that indicates which time to set.  The
 *                 keyword can be one of the following:
 *
 *                 AUTOPOP   - Set the amount of time a ToolTip window remains
 *                 visible if the pointer is stationary within a tool's bounding
 *                 rectangle. To return the autopop delay time to its default
 *                 value, set <time> to -1.
 *
 *                 INITIAL   - Set the amount of time a pointer must remain
 *                 stationary within a tool's bounding rectangle before the
 *                 ToolTip window appears. To return the initial delay time to
 *                 its default value, set <time> to -1.
 *
 *                 RESHOW    - Set the amount of time it takes for subsequent
 *                 ToolTip windows to appear as the pointer moves from one tool
 *                 to another. To return the reshow delay time to its default
 *                 value, set iTime to -1.
 *
 *                 AUTOMATIC - Sets all 3 delaty times to their default
 *                 propotions. The autopop time will be ten times the initial
 *                 time and the reshow time will be one fifth the initial time.
 *                 When using this keyword, use a positive value for <time> to
 *                 specify the initial time, in milliseconds.  Use a negative
 *                 <time> to return all three delay times to their default
 *                 values.  This is the defualt.
 *
 *  @param  time  [optional]  The time in milliseconds to set the specified
 *                delay time.  The default is -1.
 *
 *  @return  0, always.
 *
 *  @notes
 */
RexxMethod3(uint32_t, tt_setDelayTime, OPTIONAL_CSTRING, which, OPTIONAL_int32_t, time, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }

    uint32_t flag = TTDT_AUTOMATIC;
    if ( argumentExists(1) )
    {
        if ( StrCmpI(which,      "AUTOPOP"  ) == 0 ) flag = TTDT_AUTOPOP;
        else if ( StrCmpI(which, "INITIAL"  ) == 0 ) flag = TTDT_INITIAL;
        else if ( StrCmpI(which, "RESHOW"   ) == 0 ) flag = TTDT_RESHOW;
        else if ( StrCmpI(which, "AUTOMATIC") == 0 ) flag = TTDT_AUTOMATIC;
        else
        {
            wrongArgValueException(context->threadContext, 1, "AUTOPOP, INITIAL, RESHOW, or AUTOMATIC", which);
            return 0;
        }
    }

    if ( argumentOmitted(2) )
    {
        time = -1;
    }

    SendMessage(pcdc->hCtrl, TTM_SETDELAYTIME, flag, (LPARAM)MAKELONG(time, 0));

    return 0;
}


/** ToolTip::setMargin()
 *
 *  Sets the top, left, bottom, and right margins for a ToolTip window. A margin
 *  is the distance, in pixels, between the ToolTip window border and the text
 *  contained within the ToolTip window.
 *
 *  @param  margins  [Required]  A .Rect object that specifies the margins.
 *
 *  @return  0, always.
 *
 *  @notes  <margins> does not define a bounding rectangle. For the purpose of
 *          this method, the attributes of the rectangle are interpreted as
 *          follows:
 *
 *          left   - Distance between left border and left end of ToolTip text,
 *          in pixels.
 *
 *          top    - Distance between top border and top of ToolTip text, in
 *          pixels.
 *
 *          right  - Distance between right border and right end of ToolTip
 *          text, in pixels.
 *
 *          bottom - Distance between bottom border and bottom of ToolTip text,
 *          in pixels.
 *
 */
RexxMethod2(uint32_t, tt_setMargin, RexxObjectPtr, _r, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }

    PRECT r = (PRECT)rxGetRect(context, _r, 1);
    if ( r == NULL )
    {
        return 0;
    }

    SendMessage(pcdc->hCtrl, TTM_SETMARGIN, 0, (LPARAM)r);

    return 0;
}


/** ToolTip::setMaxTipWidth()
 *
 *  Sets the maximum width for a ToolTip window.
 *
 *  @param  max  [required] The maximum width for the ToolTip, or -1 to allow
 *               any width.
 *
 *  @return  Returns the previous maximum ToolTip width.
 *
 *  @notes  The maximum width value does not indicate a ToolTip window's actual
 *          width. Rather, if a ToolTip string exceeds the maximum width, the
 *          control breaks the text into multiple lines, using spaces to
 *          determine line breaks. If the text cannot be segmented into multiple
 *          lines, it is displayed on a single line, which may exceed the
 *          maximum ToolTip width.
 *
 *          For instance, if the maximum width is set to 20 and there is a word
 *          within the text that is longer than 20, the word is not broken in
 *          two.  It is displayed on a single line, causing that line to be
 *          longer than 20.
 *
 *          The text can contain embedded new line characters and the ToolTip
 *          will break the text at the new line indicator.
 */
RexxMethod2(int32_t, tt_setMaxTipWidth, int32_t, max, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }
    return (int32_t)SendMessage(pcdc->hCtrl, TTM_SETMAXTIPWIDTH, 0, max);
}


/** ToolTip::setTipBkColor()
 *
 *  Sets the background color for this ToolTip window.
 *
 *  @param  color  [required]  The new color for the background of the window.
 *
 *  @return  Returns 0, always
 *
 *  @notes  The color is expressed as a COLORREF.  See ...
 */
RexxMethod2(uint32_t, tt_setTipBkColor, uint32_t, color, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }
    SendMessage(pcdc->hCtrl, TTM_SETTIPBKCOLOR, color, 0);

    return 0;
}


/** ToolTip::setTipTextColor()
 *
 *  Sets the text color for this ToolTip window.
 *
 *  @param  color  [required]  The new color for the text of the window.
 *
 *  @return  Returns 0, always
 *
 *  @notes  The color is expressed as a COLORREF.  See ...
 */
RexxMethod2(uint32_t, tt_setTipTextColor, uint32_t, color, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }
    SendMessage(pcdc->hCtrl, TTM_SETTIPTEXTCOLOR, color, 0);

    return 0;
}


/** ToolTip::setTitle()
 *
 *  Adds a title string and optionally a standard icon to this ToolTip.
 *
 *  @param  title [required]  The title for this ToolTip.
 *
 *  @param  icon [optional]  Either an icon .Image object, or one of the
 *          following keywords to use a system icon:  NONE ERROR ERRORLARGE INFO
 *          INFOLARGE WARNING WARNINGLARGE.  If this argument is omitted it
 *          defaults to NONE.
 *
 *  @return  True on success, othewise false.
 *
 *  @notes  The title string can not be longer than 99 characters.
 */
RexxMethod3(logical_t, tt_setTitle, CSTRING, title, OPTIONAL_RexxObjectPtr, _icon, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto err_out;
    }

    size_t l = strlen(title);
    if ( l > MAX_TOOLTITLE_TEXT_LENGTH )
    {
        stringTooLongException(context->threadContext, 1, MAX_TOOLTITLE_TEXT_LENGTH, l);
        goto err_out;
    }

    uintptr_t icon = TTI_NONE;
    if ( argumentExists(2) )
    {
        if ( context->IsOfType(_icon, "IMAGE") )
        {
            POODIMAGE oi = rxGetImageIcon(context, _icon, 2);
            if ( oi == NULL )
            {
               goto err_out;
            }
            icon = (uintptr_t)oi->hImage;
        }
        else
        {
            CSTRING keyWord = context->ObjectToStringValue(_icon);

            if ( StrCmpI(keyWord,      "NONE"         ) == 0 ) icon = TTI_NONE;
            else if ( StrCmpI(keyWord, "ERROR"        ) == 0 ) icon = TTI_ERROR;
            else if ( StrCmpI(keyWord, "ERRORLARGE"   ) == 0 ) icon = TTI_ERROR_LARGE;
            else if ( StrCmpI(keyWord, "INFO"         ) == 0 ) icon = TTI_INFO;
            else if ( StrCmpI(keyWord, "INFOLARGE"    ) == 0 ) icon = TTI_INFO_LARGE;
            else if ( StrCmpI(keyWord, "WARNING"      ) == 0 ) icon = TTI_WARNING;
            else if ( StrCmpI(keyWord, "WARNINGLARGE" ) == 0 ) icon = TTI_WARNING_LARGE;
            else
            {
                wrongArgValueException(context->threadContext, 2, TT_VALID_ICON_VALUES, _icon);
                goto err_out;
            }
        }
    }

    return SendMessage(pcdc->hCtrl, TTM_SETTITLE, (WPARAM)icon, (LPARAM)title);

err_out:
    return FALSE;
}


/** ToolTip::setToolInfo()
 *
 *  Sets the information that this ToolTip control maintains for a tool.
 *
 *  @param  toolInfo   [required]  A .ToolInfo object whose attributes specify
 *                     the information this ToolTip should use for a tool.  See
 *                     the remarks.
 *
 *  @return Returns 0, always.
 *
 *  @notes  Some internal properties of a tool are established when the tool is
 *          created, and are not recomputed by the ToolTip when the
 *          setToolInfo() method is invoked.  If a .ToolInfo object is simply
 *          instantiated and its attributes assigned values and passed to the
 *          ToolTip control through the setToolInfo method, these properties
 *          may be lost.
 *
 *          Instead, the programmer should first request the tool's current
 *          .ToolInfo object by using the getToolInfo method. Then, modify the
 *          attributes of this object as needed and pass it back to the ToolTip
 *          control using the setToolInfo method.
 */
RexxMethod2(uint32_t, tt_setToolInfo, RexxObjectPtr, toolInfo, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    if ( ! context->IsOfType(toolInfo, "TOOLINFO") )
    {
        wrongClassException(context->threadContext, 1, "ToolInfo", toolInfo);
        goto done_out;
    }

    LPTOOLINFO pTI = (LPTOOLINFO)context->ObjectToCSelf(toolInfo);

    SendMessage(pcdc->hCtrl, TTM_SETTOOLINFO, 0, (LPARAM)pTI);

done_out:
    return 0;
}


/** ToolTip::setWindowTheme()
 *
 *  Sets the visual style of a ToolTip control.
 *
 *  @param  style  [required]  A string specifying the visual style.
 *
 *  @return  0, always.
 *
 *  @notes Requires common control library version 6.
 *
 *  @remarks  I do not have a clue as to what string to use for the visual
 *            style.  Some info on the web indicates you might need to use the
 *            TTS_USEVISUALSTYLE flag the tooltips. Not sure how reliable that
 *            is.
 */
RexxMethod2(uint32_t, tt_setWindowTheme, CSTRING, style, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }

    if ( ! requiredComCtl32Version(context, "setWindowTheme", COMCTL32_6_0) )
    {
        return 0;
    }

    LPWSTR theme = ansi2unicode(style);
    if ( theme != NULL )
    {
        SendMessage(pcdc->hCtrl, TTM_SETWINDOWTHEME, 0, (LPARAM)theme);
        LocalFree(theme);
    }

    return 0;
}


/** ToolTip::trackActivate()
 *
 *  Activates or deactivates a tracking ToolTip.
 *
 *  @param  toolHwnd  [required]
 *
 *  @param  toolId    [optional]
 *
 *  @param  activate  [optional]  If true the ToolTip is activated, if false the
 *                    ToolTip is deactivated.  If omitted, the default is true.
 *
 *  @return  Returns 0 always.
 *
 *  @notes  toolHwnd and toolId are the Rexx object combination that uniquely
 *          specifies a tool to this tool tip.
 *
 *          Tracking ToolTips must be manually activated and deactivated by
 *          using the trackActivate method. Activation or deactivation also
 *          shows or hides the ToolTip, respectively.
 *
 *          While a tracking ToolTip is active, the application must specify
 *          the location of the ToolTip by invoking the trackPosition method.
 */
RexxMethod4(uint32_t, tt_trackActivate, RexxObjectPtr, toolHwnd, OPTIONAL_RexxObjectPtr, toolID,
            OPTIONAL_logical_t, activate, CSELF, pCSelf)
{
    TOOLINFO ti = { sizeof(ti) };

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    if ( ! genericToolID(context, toolHwnd, toolID, &ti, NULL, NULL, 1) )
    {
        goto done_out;
    }

    if ( argumentOmitted(3) )
    {
        activate = TRUE;
    }

    SendMessage(pcdc->hCtrl, TTM_TRACKACTIVATE, activate, (LPARAM)&ti);

done_out:
    return 0;
}


/** ToolTip::trackPosition()
 *
 *  Sets the position of a tracking ToolTip.
 *
 *  @param  coordinates  [required] The position (x, y) to set.  The position
 *                       can either be specified using a .Point object or by
 *                       using a x and a y arguments.
 *
 *      Form 1:  A .Point object.
 *      Form 2:  x, y
 *
 *  @return  0, always.
 *
 *  @notes  Tracking ToolTips change position on the screen dynamically.  While
 *          a tracking ToolTip is active, the application must specify the
 *          location of the ToolTip by invoking this method.
 *
 *          The ToolTip control chooses where to display the ToolTip window
 *          based on the coordinates provided throug this method. This causes
 *          the ToolTip window to appear beside the tool to which it
 *          corresponds. To have ToolTip windows displayed at specific
 *          coordinates, include the ABSOLUTE keyword in the <flags> argument
 *          when adding the tool through <addTool>, <addToolEx>, or
 *          <addToolRect>.
 */
RexxMethod2(uint32_t, tt_trackPosition, ARGLIST, args, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;;
    }

    size_t countArgs;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 1, 2, &countArgs, &argsUsed) )
    {
        return 0;
    }

    if ( countArgs > argsUsed )
    {
        tooManyArgsException(context->threadContext, argsUsed + 1);
        return 0;
    }

    SendMessage(pcdc->hCtrl, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(point.x, point.y));

    return 0;
}


/** ToolTip::update()
 *
 *  Forces the current tool to be redrawn.
 *
 *  @return  0, always.
 */
RexxMethod1(uint32_t, tt_update, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }

    SendMessage(pcdc->hCtrl, TTM_UPDATE, 0, 0);

    return 0;
}


/** ToolTip::updateTipText()
 *
 *  Sets the ToolTip text for a tool.
 *
 *  @param  toolInfo  [required]  A .ToolInfo object that specifies the tool and
 *                    the text for the update.
 *
 *  @return  0, always.
 *
 *  @notes  The best way to use this method is to instantiate a new .ToolInfo
 *          object through the ToolInfo::forID() method and then set the <text>
 *          attribute of the object to the new text desired.
 */
RexxMethod2(uint32_t, tt_updateTipText, RexxObjectPtr, toolInfo, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    if ( ! context->IsOfType(toolInfo, "TOOLINFO") )
    {
        wrongClassException(context->threadContext, 1, "ToolInfo", toolInfo);
        goto done_out;
    }

    LPTOOLINFO pTI = (LPTOOLINFO)context->ObjectToCSelf(toolInfo);

    SendMessage(pcdc->hCtrl, TTM_UPDATETIPTEXT, 0, (LPARAM)pTI);

done_out:
    return 0;
}


/** ToolTip::windowFromPoint()
 *
 *  Allows a subclass procedure to cause a ToolTip to display text for a window
 *  other than the one beneath the mouse cursor.
 *
 *  This method is not implemented by ooDialog.  Rather, this method header is
 *  just placed here as a documentation reminder if the ability to subclass a
 *  ToolTip control is every added to ooDialog.
 *
 *  @notes  This message is intended to be processed by an application that
 *          subclasses a ToolTip. It is not intended to be sent by an
 *          application. A ToolTip sends this message to itself before
 *          displaying the text for a window. By changing the coordinates of the
 *          point specified by lppt, the subclass procedure can cause the
 *          ToolTip to display text for a window other than the one beneath the
 *          mouse cursor.
 */



/**
 *  Methods for the .ToolInfo class.
 */
#define TOOLINFO_CLASS            "ToolInfo"


/** ToolInfo::forHitTest()     [class]
 *
 *  Returns a new .ToolInfo object to be used in the ToolTip::hitTest() method.
 *
 *  @param  hwndObj  [required]  The tool or object that contains a tool that is
 *                   going to be hit tested.
 *
 *  @notes  The new ToolInfo object is especially constructed to be of use in
 *          the ToolTip::hitTest() method.  To be explicit, the ToolInfo
 *          returned is initialized to an empty state and the normal tool
 *          indentifiers are not set.  The returned ToolInfo object is not
 *          suitable to be used in any other methods that required a ToolInfo
 *          object, except the hitTest() method.
 */
RexxMethod2(RexxObjectPtr, ti_forHitTest_cls, RexxObjectPtr, hwndObj, OSELF, self)
{
    RexxObjectPtr result = TheNilObj;
    LPTOOLINFO    pTI;

    RexxBufferObject tiBuf  = rexxBufferForToolInfo(context, &pTI, false);
    if ( tiBuf == NULLOBJECT )
    {
        goto done_out;
    }

    if ( context->IsOfType(hwndObj, "PLAINBASEDIALOG") )
    {
        pCPlainBaseDialog pcpbd = dlgToCSelf(context, hwndObj);

        pTI->hwnd = pcpbd->hDlg;
    }
    else if ( context->IsOfType(hwndObj, "DIALOGCONTROL") )
    {
        pCDialogControl pcdc = controlToCSelf(context, hwndObj);

        pTI->hwnd = pcdc->hCtrl;
    }
    else
    {
        wrongClassListException(context->threadContext, 1, "PlainBaseDialog or DialogControl", hwndObj);
        goto done_out;
    }

    RexxArrayObject args = context->NewArray(5);
    context->ArrayPut(args, tiBuf, 1);
    context->ArrayPut(args, hwndObj, 2);
    context->ArrayPut(args, TheNilObj, 5);

    result = context->SendMessage(self, "NEW", args);

done_out:
    return result;
}


/** ToolInfo::forID()     [class]
 *
 *  Returns a new .ToolInfo object to be used as a tool Identifier.
 *
 *  @remarks  The intent of this method is to quickly return an object that can
 *            be used as a tool indentifier.  So, no other setting of the
 *            TOOLINFO struct is done.
 */
RexxMethod3(RexxObjectPtr, ti_forID_cls, RexxObjectPtr, hwndObj, OPTIONAL_RexxObjectPtr, rxID, OSELF, self)
{
    RexxObjectPtr result = TheNilObj;
    LPTOOLINFO    pTI;

    RexxBufferObject tiBuf  = rexxBufferForToolInfo(context, &pTI, false);
    if ( tiBuf == NULLOBJECT )
    {
        goto done_out;
    }

    RexxObjectPtr hwndSupplier;
    RexxObjectPtr uIDSupplier;

    if ( ! genericToolID(context, hwndObj, rxID, pTI, &hwndSupplier, &uIDSupplier, 1) )
    {
        goto done_out;
    }

    RexxArrayObject args = context->NewArray(5);
    context->ArrayPut(args, tiBuf, 1);
    context->ArrayPut(args, hwndSupplier, 2);
    context->ArrayPut(args, uIDSupplier, 5);

    result = context->SendMessage(self, "NEW", args);

done_out:
    return result;
}



/** ToolInfo::uninit()
 *
 */
RexxMethod1(RexxObjectPtr, ti_unInit, CSELF, pCSelf)
{
#if 0
    printf("In ti_unInit() pCSelf=%p\n", pCSelf);
#endif

    if ( pCSelf != NULLOBJECT )
    {
        LPTOOLINFO pTI = (LPTOOLINFO)pCSelf;

#if 0
    printf("In ti_unInit() lpszText=%p\n", pTI->lpszText != NULL ? pTI->lpszText : "null");
#endif

        if ( context->GetObjectVariable(TOOLINFO_MEMALLOCATED_VAR) == TheTrueObj )
        {
#if 0
    printf("In ti_unInit() will safeLocalFree() on text\n");
#endif
            safeLocalFree(pTI->lpszText);
        }
    }
    return NULLOBJECT;
}

/** ToolInfo::init()
 *
 *  @param  hwndObj  [required]  Tool hwnd object.  May be a Buffer object to
 *                   create a .ToolInfo from native code.
 *
 *  @param  rxID     [optional]
 *
 *  @param  text     [optional]
 *
 *  @param  flags    [optional]
 *
 *  @param  rect     [optional]
 *
 *  @param  userData [optional]
 *
 *  @param  _resource  [reserved]  This is reserved for a future enhancement.
 *                     If we ever add the ability to use string resources to the
 *                     .Resource class, then this argument will be a .Resource
 *                     object and text will be changed to a Rexx object.  At
 *                     that time, if _resource is used, text will be assumed to
 *                     be a resource identifier.  For now it is ignored.
 *
 *  @remarks  Note that genericToolID will set the TTF_IDISHWND flag when it
 *            determines that the uID field of the TOOLINFO struct is the hwnd
 *            of a dialog control.  So, we need to || (binary or) the return
 *            from keyword2ttfflags() with the existing setting.  However, it
 *            may be that the user incorrectly adds the TTF_IDISHWND flag and
 *            cause a crash if the uID is not a hwnd.  So, we also try to
 *            prevent that.
 *
 *            When creating from native code, set arg 1 to the TOOLINFO buffer,
 *            set arg 2 to the object supplying the hwnd field of the struct,
 *            and set arg 5 to the object supplying the uID field of the struct.
 *            The hwnd and uID supplying objects are set as context variables so
 *            that they can be returned from the attributes of the ToolInfo
 *            object.
 */
RexxMethod7(RexxObjectPtr, ti_init, RexxObjectPtr, hwndObj, OPTIONAL_RexxObjectPtr, rxID, OPTIONAL_CSTRING, text,
            OPTIONAL_CSTRING, _flags, OPTIONAL_RexxObjectPtr, _rect, OPTIONAL_RexxObjectPtr, userData,
            OPTIONAL_RexxObjectPtr, _resource)
{
    if ( context->IsBuffer(hwndObj) )
    {
        context->SetObjectVariable("CSELF", hwndObj);
        context->SetObjectVariable(TOOLINFO_HWND_OBJECT_VAR, rxID);
        context->SetObjectVariable(TOOLINFO_UID_OBJECT_VAR, _rect);
        goto done_out;
    }

    LPTOOLINFO       pTI;
    RexxBufferObject bufObj = rexxBufferForToolInfo(context, &pTI, false);
    if ( bufObj == NULLOBJECT )
    {
        goto done_out;
    }
    context->SetObjectVariable("CSELF", bufObj);

    RexxObjectPtr hwndSupplier;
    RexxObjectPtr uIDSupplier;

    if ( ! genericToolID(context, hwndObj, rxID, pTI, &hwndSupplier, &uIDSupplier, 1) )
    {
        goto done_out;
    }
    context->SetObjectVariable(TOOLINFO_HWND_OBJECT_VAR, hwndSupplier);
    context->SetObjectVariable(TOOLINFO_UID_OBJECT_VAR, uIDSupplier);

    if ( argumentExists(3) )
    {
        if ( ! setToolInfoText(context, pTI, text, 3, NULLOBJECT) )
        {
            goto done_out;
        }
    }
    else
    {
        pTI->lpszText = LPSTR_TEXTCALLBACK;
    }

    if ( argumentExists(4) )
    {
        uint32_t flags = keyword2ttfFlags(context, _flags);
        if ( flags == OOD_ID_EXCEPTION )
        {
            goto done_out;
        }

        // If genericToolID() determined that ID is not a hwnd, do not let the
        // user add that flag:

        if ( ! (pTI->uFlags & TTF_IDISHWND) )
        {
            flags &= ~TTF_IDISHWND;
        }

        pTI->uFlags |= flags;
    }

    if ( argumentExists(5) )
    {
        PRECT r = (PRECT)rxGetRect(context, _rect, 5);
        if ( r == NULL )
        {
            goto done_out;
        }
        CopyRect(&pTI->rect, r);
    }

    if ( argumentExists(6) )
    {
        pTI->lParam = (LPARAM)userData;
    }

done_out:
    return NULLOBJECT;
}


/** ToolInfo::flags                [attribute]
 */
RexxMethod1(RexxStringObject, ti_flags, CSELF, pTI)
{
    return ttfFlags2keyword(context, ((LPTOOLINFO)pTI)->uFlags);
}
RexxMethod2(RexxObjectPtr, ti_setFlags, CSTRING, flags, CSELF, pTI)
{
    uint32_t temp = keyword2ttfFlags(context, flags);
    if ( temp != OOD_ID_EXCEPTION )
    {
        ((LPTOOLINFO)pTI)->uFlags = temp;
    }
    return NULLOBJECT;
}

/** ToolInfo::rect                [attribute]
 *
 *  @note  The return can be an empty rect.
 */
RexxMethod1(RexxObjectPtr, ti_rect, CSELF, cSelf)
{
    LPTOOLINFO pTI = (LPTOOLINFO)cSelf;
    PRECT pRect = &pTI->rect;

    return rxNewRect(context, (PORXRECT)pRect);
}
RexxMethod2(RexxObjectPtr, ti_setRect, RexxObjectPtr, rect, CSELF, cSelf)
{
    LPTOOLINFO pTI = (LPTOOLINFO)cSelf;
    PRECT pRect = &pTI->rect;

    PRECT r = (PRECT)rxGetRect(context, rect, 1);
    if ( r != NULL )
    {
        CopyRect(&pTI->rect, r);
    }
    return NULLOBJECT;
}

/** ToolInfo::resource                [attribute]
 *
 *  The resource attribute is reserved at this time.  If the .Resource class is
 *  enhanced to work with string resources (a good possibility) then this
 *  attribute will be used for the hinst field of the tool info struct.
 *
 *  @remarks  For now, we always return .nil and do not allow the user to set
 *            the attribute.
 */
RexxMethod1(RexxObjectPtr, ti_resource, CSELF, cSelf)
{
    return TheNilObj;
}

/** ToolInfo::rexxHwnd                [attribute]
 *
 *  @note  No setter for this attribute.
 */
RexxMethod1(RexxObjectPtr, ti_rexxHwnd, CSELF, pTI)
{
    RexxObjectPtr hwndObj = context->GetObjectVariable(TOOLINFO_HWND_OBJECT_VAR);
    return (hwndObj ? hwndObj : TheNilObj);
}

/** ToolInfo::rexxID                [attribute]
 *
 *  @note  No setter for this attribute.
 */
RexxMethod1(RexxObjectPtr, ti_rexxID, CSELF, pTI)
{
    RexxObjectPtr idObj = context->GetObjectVariable(TOOLINFO_UID_OBJECT_VAR);
    return (idObj ? idObj : TheNilObj);
}

/** ToolInfo::text                [attribute]
 *
 *  @remarks  When getting the text attribute we need to determine if the call
 *            back feature is in use.  It is also possible that pTI->lpszText is
 *            NULL.
 *
 *            When setting the text attribute, it could be that new text is
 *            replacing existing text.  In that case, we need to determine if
 *            the old text was allocated, and if so, free the old text buffer.
 */
RexxMethod1(RexxObjectPtr, ti_text, CSELF, cSelf)
{
    LPTOOLINFO pTI = (LPTOOLINFO)cSelf;

    if ( pTI->lpszText == NULL )
    {
        return context->NullString();
    }
    else if ( pTI->lpszText == LPSTR_TEXTCALLBACK )
    {
        return context->String("TextCallBack");
    }
    else
    {
        return context->String(pTI->lpszText);
    }
}
RexxMethod2(RexxObjectPtr, ti_setText, CSTRING, text, CSELF, cSelf)
{
    LPTOOLINFO pTI = (LPTOOLINFO)cSelf;

    if ( context->GetObjectVariable(TOOLINFO_MEMALLOCATED_VAR) == TheTrueObj )
    {
        safeLocalFree(pTI->lpszText);
    }
    setToolInfoText(context, pTI, text, 1, NULLOBJECT);
    return NULLOBJECT;
}

/** ToolInfo::userData                [attribute]
 */
RexxMethod1(RexxObjectPtr, ti_userData, CSELF, pTI)
{
    return ( ((LPTOOLINFO)pTI)->lParam ? (RexxObjectPtr)((LPTOOLINFO)pTI)->lParam : TheNilObj );
}
RexxMethod2(RexxObjectPtr, ti_setUserData, RexxObjectPtr, userData, CSELF, pTI)
{
    ((LPTOOLINFO)pTI)->lParam = (LPARAM)userData;
    return NULLOBJECT;
}


/**
 * Internal use only.  Allows native code to instantiate a new ToolInfo object
 * and set the text memory is allocted flag.
 */
RexxMethod1(RexxObjectPtr, ti_setTextMemoryIsAllocated, RexxObjectPtr, allocated)
{
    context->SetObjectVariable(TOOLINFO_MEMALLOCATED_VAR, allocated == TheTrueObj ? TheTrueObj : TheFalseObj);
    return NULLOBJECT;
}


/**
 * Internal use only.  Allows native code to reset the context variables for
 * rexxHwnd and rexxID.  Memory for the text attribute has always been allocated
 * in ToolTip::hitTest().
 */
RexxMethod2(RexxObjectPtr, ti_hitTestHelper, RexxObjectPtr, hwndSupplier, RexxObjectPtr, idSupplier)
{
    context->SetObjectVariable(TOOLINFO_MEMALLOCATED_VAR, TheTrueObj);
    context->SetObjectVariable(TOOLINFO_HWND_OBJECT_VAR, hwndSupplier);
    context->SetObjectVariable(TOOLINFO_UID_OBJECT_VAR, idSupplier);
    return TheTrueObj;
}


