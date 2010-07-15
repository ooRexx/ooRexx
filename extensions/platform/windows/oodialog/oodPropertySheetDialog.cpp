/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2010 Rexx Language Association. All rights reserved.    */
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

/**
 * oodPropertySheetDialog.cpp
 *
 * Contains the classes used to implement a Property Sheet dialog, (as oppossed
 * to the PropertySheet.cls which is not a Windows property sheet at all.)
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <commctrl.h>
#include <shlwapi.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodMessaging.hpp"
#include "oodUser.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodData.hpp"
#include "oodResourceIDs.hpp"
#include "oodPropertySheetDialog.hpp"


PROPSHEETHOOKDATA *PropSheetHookData[MAX_PROPSHEET_DIALOGS];
size_t CountPropSheetHooks = 0;


class PropSheetThreadArgs
{
public:
    pCPropertySheetDialog  pcpsd;
    PROPSHEETHEADER       *psh;
    bool                  *release; // Used to signal thread initialization complete
};

#define VALID_PSNRET_LIST       "PSNRET_NOERROR, PSNRET_INVALID, or PSNRET_INVALID_NOCHANGEPAGE"
#define VALID_PSNRET_MSG_LIST   "PSNRET_NOERROR or PSNRET_MESSAGEHANDLED"

bool psnCheckForCondition(RexxThreadContext *c, pCPropertySheetDialog pcpsd)
{
    if ( checkForCondition(c, true) )
    {
        endDialogPremature(pcpsd->pcpbd, pcpsd->hDlg, RexxConditionRaised);
        return true;
    }
    return false;
}

bool goodReply(RexxThreadContext *c, pCPropertySheetDialog pcpsd, RexxObjectPtr r, CSTRING method)
{
    if ( ! psnCheckForCondition(c, pcpsd) )
    {
        if ( r == NULLOBJECT )
        {
            c->RaiseException1(Rexx_Error_No_result_object_message, c->String(method));
            psnCheckForCondition(c, pcpsd);
            return false;
        }
        return true;
    }
    return false;
}

inline void psnMemoryErr(RexxThreadContext *c, pCPropertySheetDialog pcpsd)
{
    outOfMemoryException(c);
    psnCheckForCondition(c, pcpsd);
}


/**
 *  98.900
 *  Error 98 - Execution error
 *        The language processor detected a specific error during execution.
 *
 *  The return from method "name"() must a whole number from "min" to "max";
 *  found "value"
 *
 *  The return from method setActive() must be a whole number from -1 to
 *  2,147,483,647; found an Array
 *
 *  The exception is raised, printed, and the dialog is ended.
 */
void wrongPSNRangeException(RexxThreadContext *c, CSTRING method, int32_t min, int32_t max, RexxObjectPtr actual,
                            pCPropertySheetDialog pcpsd)
{
    TCHAR buf[256];
    _snprintf(buf, sizeof(buf), "The return from method %s() must be a whole number from %d to %d; found %s",
              method, min, max, c->ObjectToStringValue(actual));

    c->RaiseException1(Rexx_Error_Execution_user_defined, c->String(buf));
    psnCheckForCondition(c, pcpsd);
}


/**
 *  98.900
 *  Error 98 - Execution error
 *        The language processor detected a specific error during execution.
 *
 *  The return from method "name"() must one of "a list"; found "value"
 *
 *  The return from method killActive() must be one of PSNRET_NOERROR,
 *  PSNRET_INVALID, or PSNRET_INVALID_NOCHANGEPAGE; found 15
 *
 *  The exception is raised, printed, and the dialog is ended.
 */
void invalidPSNReturnListException(RexxThreadContext *c, CSTRING method, CSTRING list, RexxObjectPtr actual,
                                   pCPropertySheetDialog pcpsd)
{
    TCHAR buf[256];
    _snprintf(buf, sizeof(buf), "The return from method %s() must be one of %s; found %s",
              method, list, c->ObjectToStringValue(actual));

    c->RaiseException1(Rexx_Error_Execution_user_defined, c->String(buf));
    psnCheckForCondition(c, pcpsd);
}


/**
 * Resolves a resource ID returned from an invocation of a Rexx method from a
 * dialog procdure. The resource ID may be numeric or symbolic.
 *
 * @param c          Thread context we are operating in.
 * @param oodObj     ooDialog object that has inherited .ResourceUtils.
 * @param id         Resource ID.
 *
 * @return The resolved numeric ID on success, OOD_MEMORY_ERR or
 *         OOD_ID_EXCEPTION on error.
 *
 * @remarks  The caller must ensure that oodObj has inherited ResourceUtils, no
 *           check is done.
 *
 *           This is currently only used for PropertySheetDialogs, but could be
 *           made generic if needed for other dialogs.
 */
uint32_t oodResolveSymbolicID(RexxThreadContext *c, RexxObjectPtr oodObj, RexxObjectPtr id)
{
    uint32_t result = OOD_ID_EXCEPTION;

    char *symbol = NULL;

    if ( ! c->ObjectToUnsignedInt32(id, &result) )
    {
        RexxDirectoryObject constDir = (RexxDirectoryObject)c->SendMessage0(oodObj, "CONSTDIR");
        if ( constDir != NULLOBJECT )
        {
            /* The original ooDialog code uses:
             *   self~ConstDir[id~space(0)~translate]
             * Why they allowed a space in a symbolic ID, I don't understand.
             * But, I guess we need to preserve that.
             */

            symbol = strdupupr_nospace(c->ObjectToStringValue(id));
            if ( symbol == NULL )
            {
                result = OOD_MEMORY_ERR;
                goto done_out;
            }

            RexxObjectPtr item = c->DirectoryAt(constDir, symbol);
            if ( item != NULLOBJECT )
            {
                 c->ObjectToUnsignedInt32(item, &result);
            }
        }
    }

    safeFree(symbol);

done_out:
    return result;
}

RexxStringObject wm2string(RexxThreadContext *c, uint32_t wmMsg)
{
    char *str = "";
    switch ( wmMsg )
    {
        case WM_KEYDOWN :
            str = "KEYDOWN";
            break;
        case WM_KEYUP       :
            str = "KEYUP";
            break;
        case WM_CHAR        :
            str = "CHAR";
            break;
        case WM_DEADCHAR    :
            str = "DEADCHAR";
            break;
        case WM_SYSKEYDOWN  :
            str = "SYSKEYDOWN";
            break;
        case WM_SYSKEYUP    :
            str = "SYSKEYUP";
            break;
        case WM_SYSCHAR     :
            str = "SYSCHAR";
            break;
        case WM_SYSDEADCHAR :
            str = "SYSDEADCHAR";
            break;
    }
    return c->String(str);
}

/**
 * Creates a Rexx argument array for the property sheet PSN_TRANSLATEACCELERATOR
 * notification.
 *
 * @param c       Thread context we are operating in.
 * @param wmMsg   The windows message (WM_xxx).
 * @param wParam  The WPARAM for the message, the charater code for the key.
 * @param lParam  The LPARAM for the message, contains state info.
 *
 * @return An array of args.
 *
 * @remarks
 */
RexxArrayObject getTranslateAccelatorArgs(RexxThreadContext *c, uint32_t _wmMsg, WPARAM wParam, LPARAM lParam,
                                          RexxObjectPtr propSheet)
{
    RexxStringObject wmMsg = wm2string(c, _wmMsg);
    RexxObjectPtr    keyCode = c->WholeNumber(wParam);

    BOOL released   = (lParam & KEY_RELEASED)   ? 1 : 0;
    BOOL wasDown    = (lParam & KEY_WASDOWN)    ? 1 : 0;
    BOOL isExtended = (lParam & KEY_ISEXTENDED) ? 1 : 0;
    BOOL altHeld    = (lParam & KEY_ALTHELD)    ? 1 : 0;

    BOOL shiftHeld   = (GetAsyncKeyState(VK_SHIFT) & ISDOWN)   ? 1 : 0;
    BOOL controlHeld = (GetAsyncKeyState(VK_CONTROL) & ISDOWN) ? 1 : 0;

    RexxDirectoryObject d = (RexxDirectoryObject)rxNewBuiltinObject(c, "DIRECTORY");

    c->DirectoryPut(d, c->Logical(released),    "RELEASED");
    c->DirectoryPut(d, c->Logical(wasDown),     "WASDOWN");
    c->DirectoryPut(d, c->Logical(isExtended),  "ISEXTENDED");
    c->DirectoryPut(d, c->Logical(altHeld),     "ALTHELD");
    c->DirectoryPut(d, c->Logical(shiftHeld),   "SHIFTHELD");
    c->DirectoryPut(d, c->Logical(controlHeld), "CONTROLHELD");

    RexxArrayObject args = c->ArrayOfFour(wmMsg, keyCode, d, propSheet);

    return args;
}


/**
 * Translates the Rexx reply for a PSN_SETACTIVE notification to the Windows
 * equivalent.
 *
 * Note that the replies to PSN_SETACTIVE, PSN_WIZBACK, and PSN_WIZNEXT are all
 * handled by this function.
 *
 * @param c
 * @param pcpsd
 * @param result
 * @param name
 *
 * @return INT_PRT
 */
INT_PTR getSetActiveValue(RexxThreadContext *c, pCPropertySheetDialog pcpsd, RexxObjectPtr result, CSTRING name)
{
    INT_PTR ret = -2;

    if ( c->IsPointer(result) )
    {
        return (INT_PTR)c->PointerValue((RexxPointerObject)result);
    }

    int max = (int)pcpsd->pageCount;

    int32_t index;
    if ( ! c->Int32(result, &index) || (index < -1 || index > max) )
    {
        TCHAR buf[256];
        _snprintf(buf, sizeof(buf), "The return from method %s() must be a whole number from %d to %d; found %s",
                  name, -1, max, c->ObjectToStringValue(result));

        c->RaiseException1(Rexx_Error_Execution_user_defined, c->String(buf));
        return ret;
    }

    if ( index < 1 )
    {
        ret = index;
    }
    else
    {
        pCPropertySheetPage pcpsp = pcpsd->cppPages[index - 1];
        ret = pcpsp->pageID;
    }
    return ret;
}


bool setPropSheetHook(pCPropertySheetDialog pcpsd)
{
    PROPSHEETHOOKDATA *pshd = NULL;
    bool               result = false;
    uint32_t           threadID = 0;

    pshd = (PROPSHEETHOOKDATA *)LocalAlloc(LPTR, sizeof(PROPSHEETHOOKDATA));
    if ( pshd != NULL )
    {
        pshd->pcpsd = pcpsd;
        SetLastError(0);

        EnterCriticalSection(&ps_crit_sec);

        if ( CountPropSheetHooks < MAX_PROPSHEET_DIALOGS )
        {
            threadID = GetCurrentThreadId();
            pshd->hHook = SetWindowsHookEx(WH_CBT, (HOOKPROC)PropSheetCBTProc, (HINSTANCE)NULL, GetCurrentThreadId());

            if ( pshd->hHook != NULL )
            {
                pshd->threadID = threadID;
                PropSheetHookData[CountPropSheetHooks] = pshd;
                CountPropSheetHooks++;
                result = true;
            }
        }

        LeaveCriticalSection(&ps_crit_sec);

        if ( result == true )
        {
            goto done_out;
        }
    }

    // Only here if we had an error, figure out which one.
    if ( pshd == NULL )
    {
        outOfMemoryException(pcpsd->dlgProcContext);
    }
    else if ( threadID == 0 )
    {
        userDefinedMsgException(pcpsd->dlgProcContext, TOO_MANY_PROPSHEET_DIALOGS, MAX_PROPSHEET_DIALOGS);
    }
    else
    {
        systemServiceExceptionCode(pcpsd->dlgProcContext, API_FAILED_MSG, "SetWindowsHookEx", GetLastError());
    }

    safeLocalFree(pshd);

done_out:
   return result;
}

/**
 * Searches for a match of the property sheet hook data struct.  The match is
 * keyed on the current thread ID and hwnd.
 *
 * This function is called twice.  The first time from PropCBTProc() where the
 * hwnd has not yet been placed into the struct; the match is by thread ID only.
 * If found, the struct is returned and PropSheetCBTProc() adds the hwnd to the
 * struct and unhooks the hook.
 *
 * The second time this function is called, it is from the PropSheetCallback()
 * function and this invocation passes in the hwnd.  If found, when hwnd is not
 * null, the struct is removed from the table and the caller frees its memory.
 *
 * The use of the hwnd and second invocation is because of the AeroWizard style
 * property sheet.  Placing the CSelf struct in the GWLP_USEDATA
 *
 *
 * @param hwnd
 *
 * @return PROPSHEETHOOKDATA*
 */
PROPSHEETHOOKDATA *getPropSheetHookData(HWND hwnd)
{
    register size_t i;
    DWORD id = GetCurrentThreadId();
    PROPSHEETHOOKDATA *pshd = NULL;

    EnterCriticalSection(&ps_crit_sec);

    for ( i = 0; i < CountPropSheetHooks; i++ )
    {
        if ( PropSheetHookData[i]->threadID == id && PropSheetHookData[i]->hPropSheet == hwnd )
        {
            pshd = PropSheetHookData[i];
            break;
        }
    }

    // It seems almost impossible that pshd could be null, but there is nothing
    // to do about it if it is.
    if ( pshd != NULL && hwnd != NULL )
    {
        // If the slot found is the last, just set it to null.
        if ( i == CountPropSheetHooks - 1 )
        {
            PropSheetHookData[i] = NULL;
        }
        else
        {
            // Since it is not the last, i + 1 must be valid memory.
            memcpy(&PropSheetHookData[i], &PropSheetHookData[i + 1], (CountPropSheetHooks - i - 1) * sizeof(PROPSHEETHOOKDATA *));
            PropSheetHookData[CountPropSheetHooks - 1] = NULL;
        }
        CountPropSheetHooks--;
    }

    LeaveCriticalSection(&ps_crit_sec);

    return pshd;
}


/**
 * Called from the property sheet callback function when signaled that the
 * property sheet dialog is being initialized.  Performs the initialization
 * normally done in the window loop thread function and the execute() method for
 * normal ooDialog dialogs.
 *
 * @param hPropSheet  Window handle of the property sheet.
 */
static void initializePropSheet(HWND hPropSheet)
{
    PROPSHEETHOOKDATA *pshd = getPropSheetHookData(hPropSheet);

    if ( pshd != NULL )
    {
        pCPropertySheetDialog pcpsd = pshd->pcpsd;
        RexxThreadContext *c = pcpsd->dlgProcContext;
        pCPlainBaseDialog pcpbd = pcpsd->pcpbd;

        LocalFree(pshd);

        pcpbd->hDlg = hPropSheet;

        // Not sure about using the whole top dialog thing for property sheets.
        installNecessaryStuff(pcpbd, NULL);

        pcpbd->hDlg = hPropSheet;
        pcpbd->isActive = true;
        pcpbd->childDlg[0] = hPropSheet;

        setDlgHandle(c, pcpbd);
        setFontAttrib(c, pcpbd);

        pcpbd->onTheTop = true;
        pcpbd->threadID = GetCurrentThreadId();

        // Do we have a modal dialog?  TODO need to check this for modeless property sheet.
        checkModal((pCPlainBaseDialog)pcpbd->previous, pcpsd->modeless);

        c->SendMessage0(pcpsd->rexxSelf, "INITDIALOG");
    }
}


/**
 * Called from WM_INITDIALOG for a property sheet page.  Performs the
 * initialization for the dialog that is usually done in the window loop thread
 * furnction and the execute() method for regular ooDialog dialogs.
 *
 * @param hPage
 * @param pcpsp
 */
static void initializePropSheetPage(HWND hPage, pCPropertySheetPage pcpsp)
{
    pCPlainBaseDialog pcpbd = pcpsp->pcpbd;
    RexxThreadContext *c = pcpsp->dlgProcContext;

    setWindowPtr(hPage, GWLP_USERDATA, (LONG_PTR)pcpsp);

    pcpsp->hPage = hPage;
    pcpsp->activated = true;

    pcpbd->hDlg = hPage;
    pcpbd->isActive = true;
    pcpbd->childDlg[0] = hPage;

    setDlgHandle(c, pcpbd);
    if ( pcpsp->pageType == oodResPSPDialog )
    {
        setFontAttrib(c, pcpbd);

        if ( pcpbd->autoDetect )
        {
            if ( doDataAutoDetection(pcpbd) == OOD_MEMORY_ERR )
            {
                outOfMemoryException(c);
                return;
            }
        }
    }

    c->SendMessage0(pcpsp->rexxSelf, "Execute");
}

/**
 * Attempts to match the window being created in a CBTProc hook function as a
 * property sheet window
 *
 * @param cs  Pointer to the window create struct.
 *
 * @return True for a match, otherwise false.
 *
 * @remarks  Tough to be sure we get the right window in the CBTProc function.
 *           But, for out use, I think it is impossible to create two windows in
 *           the same thread without the property sheet window being first,
 *           anyway. Every property sheet window I've looked at will match the
 *           style and extended style masks used here.
 */
inline bool isPropSheetMatch(LPCREATESTRUCT cs)
{
    uint32_t styleMask = WS_POPUP | WS_CAPTION | WS_SYSMENU | 0x000000c4;
    uint32_t exStyleMask = WS_EX_CONTROLPARENT | WS_EX_DLGMODALFRAME;

    if ( cs->lpszClass == WC_DIALOG && (cs->style & styleMask) == styleMask && (cs->dwExStyle & exStyleMask) == exStyleMask )
    {
        return true;
    }
    return false;
}

/**
 * Forces a modal property sheet dialog to end.  Setting each page's abort flag
 * prevents the page from canceling the close.  We then programmatically press
 * the Cancel key.
 *
 * This would of course also work with modeless property sheet dialogs, but it
 * is not needed as DestroyWindow() works fine with modeless property sheets.
 *
 * @param pcpsd
 * @param hDlg
 * @param t
 */
void destroyModalPropSheet(pCPropertySheetDialog pcpsd, HWND hDlg, DlgProcErrType t)
{
    size_t count = pcpsd->pageCount;
    for ( size_t i = 0; i < count; i++ )
    {
        pcpsd->cppPages[i]->abort = true;
    }

    PropSheet_PressButton(hDlg, PSBTN_CANCEL);
}


/**
 * Filter for property sheet notification messages.
 *
 * @param uMsg    The windowm message id.
 * @param lParam  The LPARAM argument for the message.
 *
 * @return Return true if the message is a property sheet notification message,
 *         otherwise false.
 */
inline bool isPSNMsg(uint32_t uMsg, LPARAM lParam)
{
    uint32_t code;
    return (uMsg == WM_NOTIFY && ((code = ((NMHDR *)lParam)->code) >= PSN_LAST  && code <= PSN_FIRST));
}


/**
 * The PSN_SETACTIVE, PSN_WIZBACK, and PSN_WIZNEXT notifications all use
 * identical code.
 *
 * @param c
 * @param pcpsd
 * @param hPage
 * @param name
 */
void doSetActiveCommon(RexxThreadContext *c, pCPropertySheetPage pcpsp, HWND hPage, CSTRING name)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pcpsp->cppPropSheet;
    INT_PTR reply = 0;

    RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, name, pcpsd->rexxSelf);

    if ( goodReply(c, pcpsd, result, name) )
    {
        reply = getSetActiveValue(c, pcpsd, result, name);
        psnCheckForCondition(c, pcpsd);
    }
    setWindowPtr(hPage, DWLP_MSGRESULT, (LPARAM)reply);
}

/**
 * Common code for PSN_WIZFINISH and PSN_QUERYINITIALFOCUS
 *
 *
 * @param c
 * @param pcpsd
 * @param result
 * @param hPage
 * @param name
 */
void doWizFinishCommon(RexxThreadContext *c, pCPropertySheetPage pcpsp, RexxObjectPtr result, HWND hPage, CSTRING name)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pcpsp->cppPropSheet;
    LPARAM reply = NULL;

    if ( goodReply(c, pcpsd, result, name) )
    {
        uint32_t id = oodResolveSymbolicID(c, pcpsp->rexxSelf, result);
        if ( id == OOD_MEMORY_ERR )
        {
            psnMemoryErr(c, pcpsd);
        }
        else if ( id == OOD_ID_EXCEPTION )
        {
            invalidPSNReturnListException(c, name, "a valid resource ID or 0", result, pcpsd);
        }
        else
        {
            reply = (LPARAM)GetDlgItem(hPage, id);
        }
    }
    setWindowPtr(hPage, DWLP_MSGRESULT, reply);
}


/**
 * Handler for all the property sheet notification messages.
 *
 * Rather than use a connectPropertySheetEvent() method, the property sheet
 * dialog handles every notification.  The dialog class supplies the correct
 * default implementation for each notification.  The Rexx programmer over-rides
 * the default implementation for any notification he wants to handle.
 *
 * @param pcpsp
 * @param pcpbd
 * @param wParam
 * @param lParam
 */
static void doPSNMessage(pCPropertySheetPage pcpsp, pCPlainBaseDialog pcpbd, WPARAM wParam, LPARAM lParam)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pcpsp->cppPropSheet;
    HWND hPropSheet = pcpsd->hDlg;
    HWND hPage = pcpsp->hPage;
    RexxThreadContext *c = pcpbd->dlgProcContext;

    switch ( ((NMHDR *)lParam)->code )
    {
        case PSN_APPLY :
        {
            LPPSHNOTIFY lppsn        = (LPPSHNOTIFY)lParam;
            uint32_t reply           = PSNRET_NOERROR;
            RexxObjectPtr isOkButton = lppsn->lParam ? TheTrueObj : TheFalseObj;

            RexxObjectPtr result = c->SendMessage2(pcpsp->rexxSelf, APPLY_MSG, isOkButton, pcpsd->rexxSelf);

            if ( goodReply(c, pcpsd, result, APPLY_MSG) )
            {
                if ( ! c->UnsignedInt32(result, &reply) || ! (reply >= PSNRET_NOERROR && reply <= PSNRET_INVALID_NOCHANGEPAGE) )
                {
                    invalidPSNReturnListException(c, APPLY_MSG, VALID_PSNRET_LIST, result, pcpsd);
                }
            }
            setWindowPtr(hPage, DWLP_MSGRESULT, (LPARAM)reply);
            break;
        }

        case PSN_GETOBJECT :
        {
            // I have not been able to produce this notification, so there is no implementation for it.
            if ( pcpsp->wantGetObject )
            {
                printf("No implementation for PSN_GETOBJECT\n");
            }

            break;
        }

        case PSN_HELP :
        {
            RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, HELP_MSG, pcpsd->rexxSelf);
            goodReply(c, pcpsd, result, HELP_MSG);
            break;
        }

        case PSN_KILLACTIVE :
        {
            // Send TRUE to *cancel* the page change.  The Rexx programmer
            // should send .false to cancel.
            long reply = FALSE;

            RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, KILLACTIVE_MSG, pcpsd->rexxSelf);

            if ( goodReply(c, pcpsd, result, KILLACTIVE_MSG) )
            {
                if ( result != TheTrueObj && result != TheFalseObj )
                {
                    invalidPSNReturnListException(c, KILLACTIVE_MSG, "true or false", result, pcpsd);
                }
                else if ( result == TheFalseObj )
                {
                    reply = TRUE;
                }
            }
            setWindowPtr(hPage, DWLP_MSGRESULT, (LPARAM)reply);
            break;
        }

        case PSN_QUERYCANCEL :
        {
            // Send TRUE to *prevent* the dialog from closing.  The Rexx
            // programmer should send .false to disallow the dialog from
            // closing.
            long reply = FALSE;

            if ( ! pcpsp->abort )
            {
                RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, QUERYCANCEL_MSG, pcpsd->rexxSelf);

                if ( goodReply(c, pcpsd, result, QUERYCANCEL_MSG) )
                {
                    if ( result != TheTrueObj && result != TheFalseObj )
                    {
                        invalidPSNReturnListException(c, QUERYCANCEL_MSG, "true or false", result, pcpsd);
                    }
                    else if ( result == TheFalseObj )
                    {
                        reply = TRUE;
                    }
                }
            }

            setWindowPtr(hPage, DWLP_MSGRESULT, (LPARAM)reply);
            break;
        }

        case PSN_RESET :
        {
            LPPSHNOTIFY lppsn            = (LPPSHNOTIFY)lParam;
            RexxObjectPtr isCancelButton = lppsn->lParam ? TheTrueObj : TheFalseObj;

            RexxObjectPtr result = c->SendMessage2(pcpsp->rexxSelf, RESET_MSG, isCancelButton, pcpsd->rexxSelf);
            goodReply(c, pcpsd, result, RESET_MSG);
            break;
        }

        case PSN_TRANSLATEACCELERATOR :
        {
            if ( pcpsp->wantAccelerators )
            {
            LPMSG    pMsg  = (LPMSG)((LPPSHNOTIFY)lParam)->lParam;
            uint32_t reply = PSNRET_NOERROR;

            RexxArrayObject args = getTranslateAccelatorArgs(c, pMsg->message, pMsg->wParam, pMsg->lParam, pcpsd->rexxSelf);
            RexxObjectPtr result = c->SendMessage(pcpsp->rexxSelf, TRANSLATEACCELERATOR_MSG, args);

            if ( goodReply(c, pcpsd, result, TRANSLATEACCELERATOR_MSG) )
            {
                if ( ! c->UnsignedInt32(result, &reply) || (reply != PSNRET_NOERROR && reply != PSNRET_MESSAGEHANDLED) )
                {
                    invalidPSNReturnListException(c, TRANSLATEACCELERATOR_MSG, VALID_PSNRET_MSG_LIST, result, pcpsd);
                }
            }
            setWindowPtr(hPage, DWLP_MSGRESULT, reply);
            }
            break;
        }

        case PSN_SETACTIVE :
            doSetActiveCommon(c, pcpsp, hPage, SETACTIVE_MSG);
            break;

        case PSN_WIZBACK :
            doSetActiveCommon(c, pcpsp, hPage, WIZBACK_MSG);
            break;

        case PSN_WIZNEXT :
            doSetActiveCommon(c, pcpsp, hPage, WIZNEXT_MSG);
            break;

        case PSN_WIZFINISH :
        {
            RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, WIZFINISH_MSG, pcpsd->rexxSelf);

            doWizFinishCommon(c, pcpsp, result, hPage, WIZFINISH_MSG);
            break;
        }

        case PSN_QUERYINITIALFOCUS :
        {
            int id = GetDlgCtrlID((HWND)((LPPSHNOTIFY)lParam)->lParam);

            RexxObjectPtr result = c->SendMessage2(pcpsp->rexxSelf, QUERYINITIALFOCUS_MSG, c->Int32(id), pcpsd->rexxSelf);

            doWizFinishCommon(c, pcpsp, result, hPage, QUERYINITIALFOCUS_MSG);
            break;
        }

        default :
            break;
    }
}

/**
 * The thread and message pump function for a modeless property sheet.
 *
 *
 * @param arg
 *
 * @return DWORD WINAPI
 */
DWORD WINAPI PropSheetLoopThread(void *arg)
{
    ULONG ret;
    PropSheetThreadArgs *args = (PropSheetThreadArgs *)arg;

    pCPropertySheetDialog pcpsd = args->pcpsd;
    pCPlainBaseDialog pcpbd     = pcpsd->pcpbd;
    PROPSHEETHEADER *psh        = args->psh;
    bool *release               = args->release;

    // We need a thread context, so if we can not attach, we bail.
    RexxThreadContext *c = NULL;
    if ( ! pcpbd->interpreter->AttachThread(&c) )
    {
        *release = true;
        goto done_out;
    }

    RexxSetProcessMessages(FALSE);
    pcpbd->dlgProcContext = c;
    assignPSDThreadContext(pcpsd, c);

    HWND hPropSheet = NULL;

    if ( setPropSheetHook(pcpsd) )
    {
        hPropSheet = (HWND)PropertySheet(psh);
    }
    *release = true;

    if ( hPropSheet == NULL )
    {
        goto done_out;
    }

    c->SendMessage1(pcpbd->rexxSelf, "START", c->String("WAITFORDIALOG"));

    MSG msg;
    BOOL result;

    while ( (result = GetMessage(&msg, NULL, 0, 0)) != 0 && pcpbd->dlgAllocated )
    {
        if ( result == -1 )
        {
            break;
        }
        if ( ! PropSheet_IsDialogMessage(hPropSheet, &msg) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if ( PropSheet_GetCurrentPageHwnd(hPropSheet) == NULL )
        {
            break;
        }
    }

done_out:

    // Need to synchronize here, otherwise dlgAllocated may still be true, but
    // delDialog() is already running.
    EnterCriticalSection(&crit_sec);
    if ( pcpbd->dlgAllocated )
    {
        ret = delDialog(pcpbd, pcpbd->dlgProcContext);
        pcpbd->hDlgProcThread = NULL;
    }
    LeaveCriticalSection(&crit_sec);

    if ( hPropSheet != NULL )
    {
        ensureFinished(pcpbd, c, TheFalseObj);
    }

    if ( c != NULL )
    {
        c->DetachThread();
        pcpbd->dlgProcContext = NULL;
        pcpsd->dlgProcContext = NULL;
    }

    safeLocalFree((void *)psh->ppsp);
    safeLocalFree(psh);
    return ret;
}


/**
 * The dialog procedure for control dialogs used by the PropertySheetDialog.
 *
 * These are 'nested' dialogs, or dialogs within a top-level dialog.  For the
 * most part, the procedure is exactly the same as for top-level dialogs.  See
 * the remarks for the differences
 *
 * @param hDlg
 * @param uMsg
 * @param wParam
 * @param lParam
 *
 * @return LRESULT CALLBACK
 *
 * @remarks  PropertySheetPage dialogs are not created by the ooDialog
 *           framework, but rather by the OS.  So, in WM_INITDIALOG we do the
 *           Rexx dialog set up normally done in the window loop thread and the
 *           execute() method.
 *
 *           In WM_DESTROY we do the final steps / clean up normally done in the
 *           execute() method.
 */
LRESULT CALLBACK RexxPropertySheetDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_INITDIALOG )
    {
        PROPSHEETPAGE *p = (PROPSHEETPAGE *)lParam;
        pCPropertySheetPage pcpsp = (pCPropertySheetPage)p->lParam;

        if ( pcpsp == NULL )
        {
            // Theoretically impossible.  But ... if it should happen, abort.
            return endDialogPremature(NULL, hDlg, NoPCPSPpased);
        }

        initializePropSheetPage(hDlg, pcpsp);

        return TRUE;
    }

    pCPropertySheetPage pcpsp = (pCPropertySheetPage)getWindowPtr(hDlg, GWLP_USERDATA);
    if ( pcpsp == NULL )
    {
        // A number of messages arrive before WM_INITDIALOG, we just ignore them.
        return FALSE;
    }

    pCPlainBaseDialog pcpbd = pcpsp->pcpbd;
    if ( pcpbd->dlgProcContext == NULL )
    {
        if ( ! pcpbd->isActive )
        {
            return FALSE;
        }

        // Once again, theoretically impossible ...
        return endDialogPremature(pcpbd, hDlg, NoThreadContext);
    }

    if ( uMsg == WM_DESTROY )
    {
        pcpsp->dlgProcContext->SendMessage0(pcpsp->rexxSelf, "ENDEXECUTE");
        return TRUE;
    }

    bool msgEnabled = IsWindowEnabled(hDlg) ? true : false;

    // Do not search message table for WM_PAINT to improve redraw.
    if ( msgEnabled && uMsg != WM_PAINT && uMsg != WM_NCPAINT )
    {
        if ( isPSNMsg(uMsg, lParam) )
        {
            doPSNMessage(pcpsp, pcpbd, wParam, lParam);
            return TRUE;
        }

        MsgReplyType searchReply = searchMessageTables(uMsg, wParam, lParam, pcpbd);
        if ( searchReply != ContinueProcessing )
        {
            // Note pre 4.0.1, we always returned FALSE, (pass on to the system
            // to process.) But, post 4.0.1 we sometimes reply TRUE, the message
            // has been handled.
            return (searchReply == ReplyTrue ? TRUE : FALSE);
        }
    }

    switch ( uMsg )
    {
        case WM_PAINT:
            if ( pcpbd->bkgBitmap != NULL )
            {
                drawBackgroundBmp(pcpbd, hDlg);
            }
            break;

        case WM_DRAWITEM:
            if ( lParam != 0 )
            {
                return drawBitmapButton(pcpbd, lParam, msgEnabled);
            }
            break;

        case WM_CTLCOLORDLG:
            if ( pcpbd->bkgBrush )
            {
                return(LRESULT)pcpbd->bkgBrush;
            }
            break;

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORSCROLLBAR:
        {
            HBRUSH hbrush = NULL;

            if ( pcpbd->CT_size > 0 )
            {
                // See of the user has set the dialog item with a different
                // color.
                long id = GetWindowLong((HWND)lParam, GWL_ID);
                if ( id > 0 )
                {
                    register size_t i = 0;
                    while ( i < pcpbd->CT_size && pcpbd->ColorTab[i].itemID != id )
                    {
                        i++;
                    }
                    if ( i < pcpbd->CT_size )
                    {
                        hbrush = pcpbd->ColorTab[i].ColorBrush;
                    }

                    if ( hbrush )
                    {
                        if ( pcpbd->ColorTab[i].isSysBrush )
                        {
                            SetBkColor((HDC)wParam, GetSysColor(pcpbd->ColorTab[i].ColorBk));
                            if ( pcpbd->ColorTab[i].ColorFG != -1 )
                            {
                                SetTextColor((HDC)wParam, GetSysColor(pcpbd->ColorTab[i].ColorFG));
                            }
                        }
                        else
                        {
                            SetBkColor((HDC)wParam, PALETTEINDEX(pcpbd->ColorTab[i].ColorBk));
                            if ( pcpbd->ColorTab[i].ColorFG != -1 )
                            {
                                SetTextColor((HDC)wParam, PALETTEINDEX(pcpbd->ColorTab[i].ColorFG));
                            }
                        }
                    }
                }
            }
            if ( hbrush )
                return(LRESULT)hbrush;
            else
                return DefWindowProc(hDlg, uMsg, wParam, lParam);
        }

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            return paletteMessage(pcpbd, hDlg, uMsg, wParam, lParam);

        // For now, don't let the user created nested, nested dialogs.  In
        // addition, keyboard hooks should only be created in a top-level
        // dialog.
        case WM_USER_CREATECHILD:
        case WM_USER_CREATECONTROL_DLG:
        case WM_USER_CREATECONTROL_RESDLG:
        case WM_USER_HOOK:
            ReplyMessage((LRESULT)NULL);
            return TRUE;

        case WM_USER_INTERRUPTSCROLL:
            pcpbd->stopScroll = wParam;
            return TRUE;

        case WM_USER_GETFOCUS:
            ReplyMessage((LRESULT)GetFocus());
            return TRUE;

        case WM_USER_GETSETCAPTURE:
            if ( wParam == 0 )
            {
                ReplyMessage((LRESULT)GetCapture());
            }
            else if ( wParam == 2 )
            {
                uint32_t rc = 0;
                if ( ReleaseCapture() == 0 )
                {
                    rc = GetLastError();
                }
                ReplyMessage((LRESULT)rc);
            }
            else
            {
                ReplyMessage((LRESULT)SetCapture((HWND)lParam));
            }
            return TRUE;

        case WM_USER_GETKEYSTATE:
            ReplyMessage((LRESULT)GetAsyncKeyState((int)wParam));
            return TRUE;

        case WM_USER_SUBCLASS:
        {
            SUBCLASSDATA *pData = (SUBCLASSDATA *)lParam;

            pData->dlgProcContext = pcpbd->dlgProcContext;
            pData->rexxDialog = pcpbd->rexxSelf;

            BOOL success = SetWindowSubclass(pData->hCtrl, (SUBCLASSPROC)wParam, pData->uID, (DWORD_PTR)pData);

            ReplyMessage((LRESULT)success);
            return TRUE;
        }

        case WM_USER_SUBCLASS_REMOVE:
            ReplyMessage((LRESULT)RemoveWindowSubclass(GetDlgItem(hDlg, (int)lParam), (SUBCLASSPROC)wParam, (int)lParam));
            return TRUE;

        case WM_USER_CONTEXT_MENU:
        {
            PTRACKPOP ptp = (PTRACKPOP)wParam;
            uint32_t cmd;

            SetLastError(0);
            cmd = (uint32_t)TrackPopupMenuEx(ptp->hMenu, ptp->flags, ptp->point.x, ptp->point.y,
                                             ptp->hWnd, ptp->lptpm);

            // If TPM_RETURNCMD is specified, the return is the menu item
            // selected.  Otherwise, the return is 0 for failure and
            // non-zero for success.
            if ( ! (ptp->flags & TPM_RETURNCMD) )
            {
                cmd = (cmd == 0 ? FALSE : TRUE);
                if ( cmd == FALSE )
                {
                    ptp->dwErr = GetLastError();
                }
            }
            ReplyMessage((LRESULT)cmd);
            return TRUE;
        }
    }

    return FALSE;
}


LRESULT CALLBACK PropSheetCBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    PROPSHEETHOOKDATA *pshd = getPropSheetHookData(NULL);
    if ( pshd == NULL )
    {
        // Without the hook handle, there is nothing that can be done.
        return 0;
    }

    HHOOK hHook = pshd->hHook;

    if ( nCode == HCBT_CREATEWND )
    {
        LPCREATESTRUCT cs = (LPCREATESTRUCT)((CBT_CREATEWND *)lParam)->lpcs;


        if ( isPropSheetMatch(cs) )
        {
            pshd->pcpsd->hDlg = (HWND)wParam;
            pshd->hPropSheet = (HWND)wParam;
            UnhookWindowsHookEx(hHook);
        }
        else
        {
            printf("PropSheetCBTProc() DID NOT MATCH !! hwnd=%p className=%s name=%s style=0x%08x exStyle=0x%08x\n",
                    wParam, cs->lpszClass, cs->lpszName, cs->style, cs->dwExStyle);
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}


/**
 * An implementation of the PropSheetProc callback.  The system calls this
 * function when the property sheet is being created and initialized.
 *
 * There are only 3 uMsg messages sent.
 *
 * PSCB_PRECREATE Sent before the PropertySheet is created. lParam is a pointer
 * to the dialog template.  hwndPropSheet is null.  This callback could be used
 * to change something in the dialog template.  Don't see any good use for it at
 * this point.
 *
 * PSCB_INITIALIZED  Sent when the PropertySheet is initializing.  We use this
 * callback to do the Rexx dialog initialization that is usually done in the
 * thread creation function and the execute() method.
 *
 * PSCB_BUTTONPRESSED  Sent when a button in the property sheet is pressed.
 * Seems as though it would be useful.  But, it actually does not provide any
 * thing not already provided by the regular notification messages.  Not used at
 * this point.
 *
 * @param hwndPropSheet
 * @param uMsg
 * @param lParam
 *
 * @return void
 *
 * @remarks  Typical implementation would use a switch statement.  Since only 1
 *           uMsg is of interest, I've removed the switch.
 */
void CALLBACK PropSheetCallback(HWND hwndPropSheet, UINT uMsg, LPARAM lParam)
{
    if ( uMsg == PSCB_INITIALIZED )
    {
        initializePropSheet(hwndPropSheet);
    }
}


/**
 *  Methods for the .PropertySheetDialog class.
 */
#define PROPERTYSHEETDIALOG_CLASS  "PropertySheetDialog"


bool setCaption(RexxMethodContext *c, pCPropertySheetDialog pcpsd, CSTRING text)
{
    pcpsd->caption = (char *)LocalAlloc(LPTR, strlen(text) + 1);
    if ( pcpsd->caption == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    strcpy(pcpsd->caption, text);
    return true;
}


static bool parsePropSheetOpts(RexxMethodContext *c, pCPropertySheetDialog pcpsd, CSTRING options)
{
    uint32_t opts = PSH_PROPSHEETPAGE | PSH_USECALLBACK;

    if ( options == NULL )
    {
        pcpsd->isNotWizard = true;
        goto done_out;
    }

    if ( StrStrI(options, "USEPAGELANG") != NULL ) opts |= PSH_USEPAGELANG;
    if ( StrStrI(options, "RTLREADING ") != NULL ) opts |= PSH_RTLREADING;

    if ( StrStrI(options, "AEROWIZARD") != NULL )
    {
        if ( ! requiredComCtl32Version(c, COMCTL32_6_0, "the AEROWIZARD style") ||
             ! requiredOS(c, Vista_OS, "the AEROWIZARD style", "Vista") )
        {
            goto err_out;
        }

        pcpsd->isAeroWiz = true;
        opts |= PSH_AEROWIZARD | PSH_WIZARD;

        if ( StrStrI(options, "RESIZABLE") != NULL ) opts |= PSH_RESIZABLE;
        if ( StrStrI(options, "NOMARGIN")  != NULL ) opts |= PSH_NOMARGIN;
    }
    else if ( StrStrI(options, "WIZARD97") != NULL )
    {
        pcpsd->isWiz97 = true;
        opts |= PSH_WIZARD97;
    }
    else if ( StrStrI(options, "WIZARDLITE") != NULL )
    {
        pcpsd->isWizLite = true;
        opts |= PSH_WIZARD_LITE;
    }
    else
    {
        pcpsd->isNotWizard = true;
        if ( StrStrI(options, "NOCONTEXTHELP") != NULL )
        {
            opts |= PSH_NOCONTEXTHELP;
        }
    }

    if ( ! pcpsd->isAeroWiz )
    {
        if ( StrStrI(options, "HASHELP")    != NULL ) opts |= PSH_HASHELP;
        if ( StrStrI(options, "NOAPPLYNOW") != NULL ) opts |= PSH_NOAPPLYNOW;
        if ( StrStrI(options, "PROPTITLE")  != NULL ) opts |= PSH_PROPTITLE;
    }

    if ( (pcpsd->isAeroWiz || pcpsd->isWiz97) && StrStrI(options, "WIZARDHASFINISH") != NULL )
    {
        opts |= PSH_WIZARDHASFINISH;
    }

    if ( (pcpsd->isWiz97 || pcpsd->isWizLite) && StrStrI(options, "WIZARDCONTEXTHELP") != NULL )
    {
        opts |= PSH_WIZARDCONTEXTHELP;
    }

done_out:
    pcpsd->propSheetFlags = opts;

    return true;

err_out:
    return false;
}

/**
 * Checks that the Rexx owner dialog passed in to execute() is valid.
 *
 * @param c       Method context we are operating in.
 * @param owner   Rexx object to check.
 * @param argPos  Argument position of the Rexx object.
 *
 * @return Windown handle of the dialog, if okay, otherise NULL.  If NULL is
 *         returned, an exceptions has been raised.
 */
static HWND checkPropSheetOwner(RexxMethodContext *c, RexxObjectPtr owner, size_t argPos)
{
    pCPlainBaseDialog pcpbdOwner = requiredDlgCSelf(c, owner, oodPlainBaseDialog, argPos);
    if ( pcpbdOwner == NULL )
    {
        return NULL;
    }

    if ( pcpbdOwner->isControlDlg || pcpbdOwner->isPageDlg )
    {
        userDefinedMsgException(c, argPos, "The owner dialog of a PropertySheetDialog can not be a control dialog.");
        return NULL;
    }

    if ( pcpbdOwner->hDlg == NULL )
    {
        methodCanNotBeInvokedException(c, "execute()", pcpbdOwner->rexxSelf,
                                       "using an owner dialog whose Windows dialog does not exist");
        return NULL;
    }
    return pcpbdOwner->hDlg;
}


void assignPSDThreadContext(pCPropertySheetDialog pcpsd, RexxThreadContext *c)
{
    pcpsd->dlgProcContext = c;

    size_t count = pcpsd->pageCount;
    for ( size_t i = 0; i < count; i++ )
    {
        pcpsd->cppPages[i]->dlgProcContext = c;
        pcpsd->cppPages[i]->pcpbd->dlgProcContext = c;
    }
}


bool psdInitSuper(RexxMethodContext *context, RexxClassObject super, RexxStringObject hFile)
{
    RexxArrayObject newArgs = context->NewArray(4);

    context->ArrayPut(newArgs, context->NullString(), 1);
    context->ArrayPut(newArgs, TheZeroObj, 2);
    if ( hFile != NULLOBJECT )
    {
        context->ArrayPut(newArgs, hFile, 4);
    }
    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, newArgs);

    return isInt(0, result, context->threadContext);
}

/**
 * Allocates the memory for the property sheet page structs and fills in the
 * struct for all the pages of the property sheet.
 *
 * @param c        Method context we are operating in.
 * @param pcpsd    Pointer to the CSelf struct for a PropertySheetDialog.
 * @param hParent  Possible handle of parent dialog, can be null.
 *
 * @return The pointer to the property sheet page struct(s) on success, null on
 *         failure.
 */
PROPSHEETPAGE *initPropSheetPages(RexxMethodContext *c, pCPropertySheetDialog pcpsd)
{
    PROPSHEETPAGE *psp = NULL;
    size_t count = pcpsd->pageCount;

    psp = (PROPSHEETPAGE *)LocalAlloc(LPTR, count * sizeof(PROPSHEETPAGE));
    if ( psp == NULL )
    {
        outOfMemoryException(c->threadContext);
        goto done_out;
    }

    for ( size_t i = 0; i < count; i++ )
    {
        RexxObjectPtr       dlg   = pcpsd->rexxPages[i];
        pCPropertySheetPage pcpsp = pcpsd->cppPages[i];
        pCPlainBaseDialog   pcpbd = pcpsp->pcpbd;
        pCDynamicDialog     pcdd  = pcpsp->pcdd;
        uint32_t            flags = pcpsp->pageFlags;

        RexxObjectPtr result = c->SendMessage0(dlg, "INITTEMPLATE");
        if ( result == TheFalseObj )
        {
            LocalFree(psp);
            psp = NULL;
            noWindowsPageDlgException(c, i + 1);
            goto done_out;
        }

        psp[i].dwSize      = sizeof(PROPSHEETPAGE);
        psp[i].pfnDlgProc  = (DLGPROC)RexxPropertySheetDlgProc;
        psp[i].lParam      = (LPARAM)pcpsp;

        if ( pcpsp->pageTitle != NULL )
        {
            flags |= PSP_USETITLE;
            psp[i].pszTitle = pcpsp->pageTitle;
        }

        if ( pcpsp->pageType == oodResPSPDialog )
        {
            psp[i].hInstance   = pcpbd->hInstance;
            psp[i].pszTemplate = MAKEINTRESOURCE(pcpsp->resID);
        }
        else
        {
            flags |= PSP_DLGINDIRECT;
            psp[i].pResource = pcdd->base;
        }

        psp[i].pszIcon = NULL;
        psp[i].dwFlags = flags;
    }

done_out:
    return psp;
}


/**
 * Allocates the memory for the property sheet header struct and fills in the
 * struct.
 *
 * @param c        Method context we are operating in.
 * @param pcpsd    Pointer to the CSelf struct for a PropertySheetDialog.
 * @param psp      Pointer to the property sheet page struct(s) for the property
 *                 sheet.
 * @param hParent  Possible handle of parent dialog, can be null.
 *
 * @return The pointer to the property sheet header on success, null on failure.
 */
PROPSHEETHEADER *initPropSheetHeader(RexxMethodContext *c, pCPropertySheetDialog pcpsd, LPCPROPSHEETPAGE psp, HWND hParent)
{
    PROPSHEETHEADER *psh = (PROPSHEETHEADER *)LocalAlloc(LPTR, sizeof(PROPSHEETHEADER));
    if ( psh == NULL )
    {
        outOfMemoryException(c->threadContext);
        return NULL;
    }

    uint32_t flags = pcpsd->propSheetFlags;

    psh->dwSize           = sizeof(PROPSHEETHEADER);
    psh->hwndParent       = hParent;
    psh->pszCaption       = pcpsd->caption;
    psh->hInstance        = MyInstance;
    psh->pszIcon          = MAKEINTRESOURCE(IDI_DLG_OOREXX);
    psh->nPages           = (uint32_t)pcpsd->pageCount;
    psh->ppsp             = (LPCPROPSHEETPAGE)psp;
    psh->pfnCallback      = (PFNPROPSHEETCALLBACK)PropSheetCallback;

    if ( pcpsd->modeless )
    {
        flags |= PSH_MODELESS;
    }

    psh->dwFlags = flags;

    return psh;
}


HWND getValidPageHwnd(RexxMethodContext *c, pCPropertySheetDialog pcpsd, RexxObjectPtr page, size_t pos)
{
    RexxObjectPtr match = NULL;

    size_t count = pcpsd->pageCount;
    for ( size_t i = 0; i < count; i++ )
    {
        if ( page == pcpsd->rexxPages[i] )
        {
            match = pcpsd->rexxPages[i];
            break;
        }
    }

    if ( match == NULL )
    {
        noSuchPageException(c, page, pos);
        return NULL;
    }

    HWND hPage = dlgToPSPHDlg(c, match);
    if ( hPage == NULL )
    {
        noWindowsPageException(c, dlgToPSPCSelf(c, page)->pageNumber + 1, pos);
}

    return hPage;
}


/** PropertySheetDialog::pages          [Attrbiute Get]
 *
 *
 */
RexxMethod1(RexxObjectPtr, psdlg_getPages_atr, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    size_t count = pcpsd->pageCount;
    RexxArrayObject pages = context->NewArray(count);

    for ( size_t i = 0; i < count; i++ )
    {
        context->ArrayPut(pages, pcpsd->rexxPages[i], i + 1);
    }
    return pages;
}


/** PropertySheetDialg::caption()       [Attribute get]
 *
 */
RexxMethod1(RexxObjectPtr, psdlg_getCaption, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    return pcpsd->caption == NULL ? TheNilObj : context->String(pcpsd->caption);
}


/** PropertySheetDialog::caption()      [Attribute set]
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_setCaption, CSTRING, text, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    setCaption(context, pcpsd, text);
    return NULLOBJECT;
}


/** PropertySheetDialog::getPage()
 *
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_getPage, uint32_t, index, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    size_t count = pcpsd->pageCount;
    if ( index < 1 || index > count )
    {
        wrongRangeException(context->threadContext, 1, 1, (int)count, index);
        return NULLOBJECT;
    }

    return pcpsd->rexxPages[index - 1];
}


/** PropertySheetDialog::init()
 *
 *  The initialization of the property sheet dialog.
 *
 *
 */
RexxMethod6(wholenumber_t, psdlg_init, RexxArrayObject, pages, OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, caption,
            OPTIONAL_RexxStringObject, hFile,
            SUPER, super, OSELF, self)
{
    // This is an error return.
    wholenumber_t result = 1;

    if ( ! psdInitSuper(context, super, hFile) )
    {
        goto done_out;
    }

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();

    // Get a buffer for the PropertySheetDialog CSelf.
    RexxBufferObject cselfBuffer = context->NewBuffer(sizeof(CPropertySheetDialog));
    if ( cselfBuffer == NULLOBJECT )
    {
        goto done_out;
    }

    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)context->BufferData(cselfBuffer);
    memset(pcpsd, 0, sizeof(CPropertySheetDialog));

    pcpbd->dlgPrivate = pcpsd;
    pcpsd->pcpbd = pcpbd;
    pcpsd->rexxSelf = self;
    context->SetObjectVariable("CSELF", cselfBuffer);

    // Now process the arguments and do the rest of the initialization.
    uint32_t count = (uint32_t)context->ArrayItems(pages);
    if ( count == 0 )
    {
        emptyArrayException(context->threadContext, 1);
        goto done_out;
    }
    pcpsd->pageCount = count;

    pCPropertySheetPage *cppPages = (pCPropertySheetPage *)LocalAlloc(LPTR, count * sizeof(pCPropertySheetPage *));
    RexxObjectPtr *rexxPages = (RexxObjectPtr *)LocalAlloc(LPTR, count * sizeof(RexxObjectPtr *));

    if ( cppPages == NULL || rexxPages == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    pCPropertySheetPage *pPage = cppPages;
    RexxObjectPtr *pRexxPage = rexxPages;
    for ( uint32_t i = 1; i <= count; i++, pPage++, pRexxPage++ )
    {
        RexxObjectPtr dlg = context->ArrayAt(pages, i);
        if ( dlg == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 1, i);
            goto done_out;
        }
        if ( ! context->IsOfType(dlg, "PROPERTYSHEETPAGE") )
        {
            wrongObjInArrayException(context->threadContext, 1, i, "a PropertySheetPage", dlg);
            goto done_out;
        }

        pCPropertySheetPage pcpsp = dlgToPSPCSelf(context, dlg);
        pcpsp->pageNumber = i;
        pcpsp->rexxPropSheet = pcpsd->rexxSelf;
        pcpsp->cppPropSheet = pcpsd;
        pcpsp->isWizardPage = ! pcpsd->isNotWizard;

        *pPage = pcpsp;
        *pRexxPage = dlg;
    }

    pcpsd->cppPages = cppPages;
    pcpsd->rexxPages = rexxPages;

    // When the Rexx page dialogs are stored in the CSelf struct, the garbage
    // collector has no knowledge that they are in use. If a user does not
    // keep a reference to the dialogs, (which they may not because they can
    // access them though the PropertySheetDialog,) they could be garbage
    // collected while in use.
    context->SetObjectVariable("PAGES", pages);

    if ( argumentExists(3) )
    {
        if ( ! setCaption(context, pcpsd, caption) )
        {
            goto done_out;
        }
    }

    if ( parsePropSheetOpts(context, pcpsd, opts) )
    {
        result = 0;
    }


done_out:
    pcpbd->wndBase->initCode = result;
    return result;
}


/** PropertySheetDialog::execute()
 *
 *  Creates a modal property sheet dialog.
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_execute, OPTIONAL_RexxObjectPtr, owner, CSELF, pCSelf)
{
    RexxObjectPtr   result = TheNegativeOneObj;
    HWND            hParent = NULL;

    if ( argumentExists(1) )
    {
        hParent = checkPropSheetOwner(context, owner, 1);
        if ( hParent == NULL )
        {
            goto done_out;
        }
    }

    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    PROPSHEETPAGE *psp = initPropSheetPages(context, pcpsd);
    if ( psp == NULL )
    {
        goto done_out;
    }

    // It is not necessary to set pcpsd->modeless to false, it is false by default.

    PROPSHEETHEADER *psh = initPropSheetHeader(context, pcpsd, psp, hParent);
    if ( psh == NULL )
    {
        goto done_out;
    }

    INT_PTR ret;
    if ( hParent == NULL )
    {
        assignPSDThreadContext(pcpsd, context->threadContext);

        if ( setPropSheetHook(pcpsd) )
        {
            ret = PropertySheet(psh);
        }
        else
        {
            ret = -1;
        }
    }
    else
    {
        ret = (INT_PTR)SendMessage(hParent, WM_USER_CREATEPROPSHEET_DLG, (WPARAM)psh, (LPARAM)pcpsd);
    }

    result = context->WholeNumber(ret);

done_out:
    safeLocalFree(psp);
    safeLocalFree(psh);
    stopDialog(pcpsd->pcpbd, context->threadContext);
    return result;
}


/** PropertySheetDialog::popup()
 *
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_popup, NAME, methodName, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    pCPlainBaseDialog pcpbd = pcpsd->pcpbd;

    PROPSHEETPAGE *psp = initPropSheetPages(context, pcpsd);
    if ( psp == NULL )
    {
        goto err_out;
    }

    pcpsd->modeless = true;

    PROPSHEETHEADER *psh = initPropSheetHeader(context, pcpsd, psp, NULL);
    if ( psh == NULL )
    {
        goto err_out;
    }

    DWORD threadID;
    bool Release = false;

    EnterCriticalSection(&crit_sec);

    PropSheetThreadArgs threadArgs;
    threadArgs.pcpsd = pcpsd;
    threadArgs.psh = psh;
    threadArgs.release = &Release;

    pcpbd->hDlgProcThread = CreateThread(NULL, 2000, PropSheetLoopThread, &threadArgs, 0, &threadID);

    // Wait for thread to signal us to continue, don't wait if the thread was not created.
    while ( ! Release && pcpbd->hDlgProcThread != NULL )
    {
        Sleep(1);
    }
    LeaveCriticalSection(&crit_sec);

    if ( pcpbd->hDlgProcThread != NULL )
    {
        return TheTrueObj;
    }
    else
    {
        // Something failed in the the thread function.  In that case, things
        // are cleaned up in the thread function.
        return TheFalseObj;
    }

err_out:
    safeLocalFree(psp);
    safeLocalFree(psh);
    stopDialog(pcpsd->pcpbd, context->threadContext);
    return TheFalseObj;
}


/** PropertySheetDialog::changed()
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_changed, RexxObjectPtr, _page, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    HWND page = getValidPageHwnd(context, pcpsd, _page, 1);
    if ( page != NULL )
    {
        PropSheet_Changed(pcpsd->hDlg, page);
    }
    return TheZeroObj;
}


/** PropertySheetDialog::unchanged()
 *
 */
RexxMethod2(RexxObjectPtr, psdlg_unchanged, RexxObjectPtr, _page, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    HWND page = getValidPageHwnd(context, pcpsd, _page, 1);
    if ( page != NULL )
    {
        PropSheet_UnChanged(pcpsd->hDlg, page);
    }
    return TheZeroObj;
}


/** PropertySheetDialog::setWizButtons()
 *
 *  Enables or disables the Back, Next, and Finish buttons in a wizard.
 *
 *  @param  opts  Keyword(s) that control which buttons are enabled or disabled.
 *
 *  @return  True this property sheet is a wizard, otherwise false.
 *
 *  @notes  Wizards display either three or four buttons below each page. This
 *          method is used to specify which buttons are enabled. Wizards
 *          normally display Back, Cancel, and either a Next or Finish button.
 *          Typically, enable only the Next button for the welcome page, Next
 *          and Back for interior pages, and Back and Finish for the completion
 *          page.  The Cancel button is always enabled.  Normally, setting
 *          FINISH or DISABLEDFINISH replaces the Next button with a Finish
 *          button.  To display Next and Finish buttons simultaneously, set the
 *          WIZARDHASFINISH keyword in the options when the PropertySheetDialog
 *          is instantiated.  Every page will then display all four buttons.
 *
 *          If this property sheet is not a Wizard, this method has no effect.
 *
 *  @remarks  We do not enforce that this is only called for a Wizard, although
 *            maybe we should.
 *
 *            The prop sheet marco does not return a value.
 */
RexxMethod2(RexxObjectPtr, psdlg_setWizButtons, CSTRING, opts, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;
    if ( pcpsd->isNotWizard )
    {
        return TheFalseObj;
    }

    uint32_t flags = 0;

    if ( StrStrI(opts, "BACK")           != NULL ) flags |= PSWIZB_BACK;
    if ( StrStrI(opts, "NEXT")           != NULL ) flags |= PSWIZB_NEXT;
    if ( StrStrI(opts, "FINISH")         != NULL ) flags |= PSWIZB_FINISH;
    if ( StrStrI(opts, "DISABLEDFINISH") != NULL ) flags |= PSWIZB_DISABLEDFINISH;

    PropSheet_SetWizButtons(pcpsd->hDlg, flags);
    return TheTrueObj;
}


/** PropertySheetDialog::showWizButtons()
 *
 *  Show or hide buttons in a wizard.
 *
 *  @param  opts       One or more of the keyword values that specify which
 *                     property sheet buttons are to be shown. If a button value
 *                     is included in both this argument and the optsButtons
 *                     argument, then the button is shown.
 *
 *  @param optsButtons One or more of the same keywords used in opts. Here,
 *                     they specify which property sheet buttons are to be shown
 *                     or hidden. If a keywor appears in this argument but not
 *                     in opts, it indicates that the button should be hidden.
 *
 *  @param  Returns true if this is a Wizard property sheet on Vista on later,
 *          otherwise false.
 *
 *  @notes  This method requires Vista or later, a condition is raised if the OS
 *          is not Vista or later.
 *
 *          This method has no effect if the property sheet is not a Wizard.
 */
RexxMethod3(RexxObjectPtr, psdlg_showWizButtons, CSTRING, opts, CSTRING, optsButtons, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    if ( ! requiredOS(context, "showWizButtons", "Vista", Vista_OS) || pcpsd->isNotWizard )
    {
        return TheFalseObj;
    }

    uint32_t flags = 0;
    uint32_t buttons = 0;

    if ( StrStrI(opts, "BACK")   != NULL ) flags |= PSWIZB_BACK;
    if ( StrStrI(opts, "NEXT")   != NULL ) flags |= PSWIZB_NEXT;
    if ( StrStrI(opts, "FINISH") != NULL ) flags |= PSWIZB_FINISH;
    if ( StrStrI(opts, "CANCEL") != NULL ) flags |= PSWIZB_CANCEL;

    if ( StrStrI(optsButtons, "BACK")   != NULL ) buttons |= PSWIZB_BACK;
    if ( StrStrI(optsButtons, "NEXT")   != NULL ) buttons |= PSWIZB_NEXT;
    if ( StrStrI(optsButtons, "FINISH") != NULL ) buttons |= PSWIZB_FINISH;
    if ( StrStrI(optsButtons, "CANCEL") != NULL ) buttons |= PSWIZB_CANCEL;

    PropSheet_ShowWizButtons(pcpsd->hDlg, flags, buttons);
    return TheTrueObj;
}


RexxMethod2(RexxObjectPtr, psdlg_indexToID, int32_t, index, CSELF, pCSelf)
{
    pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)pCSelf;

    INT_PTR result = -2;

    int max = (int)pcpsd->pageCount;

    if ( index < -1 || index > max )
    {
        wrongRangeException(context->threadContext, 1, -1, max, index);
    }
    else if ( index < 1 )
    {
        result = index;
    }
    else
    {
        pCPropertySheetPage pcpsp = pcpsd->cppPages[index - 1];
        result = pcpsp->pageID;
    }

    return context->NewPointer((void *)result);
}


/** PropertySheetDialog::test()
 *
 *  Testing method
 */
RexxMethod1(RexxObjectPtr, psdlg_test, CSELF, pCSelf)
{
    printf("No test set up at this time\n");
    printf("Make version for 6.1=%d\n", MAKEVERSION(6, 1));
    printf("Make version for 6.01=%d\n", MAKEVERSION(6, 01));
    return TheZeroObj;
}


/**
 *  Methods for the .PropertySheetPage mixin class.
 */
#define PROPERTYSHEETPAGE_CLASS  "PropertySheetPage"


/**
 * Performs the initialization of the PropertySheetPage mixin class.
 *
 * We create the CSelf struct for the class and then send te struct to the
 * init_propertySheetPage() method.  That method will raise an exception if its
 * arg is not a RexxBufferObject.  This implies that the PropertySheetPage mixin
 * class can only be initialized through the native API.
 *
 * The plain base dialog, dialog control, and window classes in ooDialog inherit
 * WindowBase.
 *
 * @param c        Method context we are operating in.
 *
 * @param
 *
 * @param self     The Rexx object that inherited WindowBase.
 *
 * @return True on success, otherwise false.  If false an exception has been
 *         raised.
 *
 * @remarks
 */
bool initPropertySheetPage(RexxMethodContext *c, pCPlainBaseDialog pcpbd, pCDynamicDialog pcdd,
                           RexxObjectPtr self)
{
    RexxBufferObject obj = c->NewBuffer(sizeof(CPropertySheetPage));
    if ( obj == NULLOBJECT )
    {
        return false;
    }

    pCPropertySheetPage pcpsp = (pCPropertySheetPage)c->BufferData(obj);
    memset(pcpsp, 0, sizeof(CPropertySheetPage));

    pcpsp->pcpbd         = pcpbd;
    pcpsp->pcdd          = pcdd;
    pcpsp->rexxPropSheet = TheNilObj;
    pcpsp->rexxSelf      = self;
    pcpsp->interpreter   = pcpbd->interpreter;

    pcpbd->dlgPrivate = pcpsp;

    c->SendMessage1(self, "INIT_PROPERTYSHEETPAGE", obj);

    return true;
}


bool setPageText(RexxMethodContext *c, pCPropertySheetPage pcpsp, CSTRING text, pagePart_t part)
{
    char *t = (char *)LocalAlloc(LPTR, strlen(text) + 1);
    if ( t == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    strcpy(t, text);

    switch ( part )
    {
        case headerSubtext :
            pcpsp->headerSubtitle = t;
            break;
        case headerText :
            pcpsp->headerTitle = t;
            break;
        case pageText :
            pcpsp->pageTitle = t;
            break;
    }

    return true;
}


static void parsePageOpts(RexxMethodContext *c, pCPropertySheetPage pcpsp, CSTRING options)
{
    uint32_t opts = PSP_DEFAULT;

    if ( pcpsp->pageType != oodResPSPDialog )
    {
        pcpsp->cx = PROP_MED_CXDLG;
        pcpsp->cy = PROP_MED_CYDLG;
        if ( options != NULL )
        {
            if ( StrStrI(options, "LARGE") )
            {
                pcpsp->cx = PROP_LG_CXDLG;
                pcpsp->cy = PROP_LG_CYDLG;
            }
            else if ( StrStrI(options, "SMALL") )
            {
                pcpsp->cx = PROP_SM_CXDLG;
                pcpsp->cy = PROP_SM_CYDLG;
            }
        }
    }

    if ( options != NULL )
    {
        if ( StrStrI(options, "USETITLE")          != NULL ) opts |= PSP_USETITLE;
        if ( StrStrI(options, "RTLREADING")        != NULL ) opts |= PSP_RTLREADING;
        if ( StrStrI(options, "HASHELP")           != NULL ) opts |= PSP_HASHELP;
        if ( StrStrI(options, "USEREFPARENT")      != NULL ) opts |= PSP_USEREFPARENT;
        if ( StrStrI(options, "PREMATURE")         != NULL ) opts |= PSP_PREMATURE;
        if ( StrStrI(options, "HIDEHEADER")        != NULL ) opts |= PSP_HIDEHEADER;
        if ( StrStrI(options, "USEHEADERTITLE")    != NULL ) opts |= PSP_USEHEADERTITLE;
        if ( StrStrI(options, "USEHEADERSUBTITLE") != NULL ) opts |= PSP_USEHEADERSUBTITLE;
        if ( StrStrI(options, "USEFUSIONCONTEXT")  != NULL ) opts |= PSP_USEFUSIONCONTEXT;
    }
    pcpsp->pageFlags = opts;
}

bool initPageDlgFrame(RexxMethodContext *c, pCPropertySheetPage pcpsp)
{
    pCPlainBaseDialog pcpbd = pcpsp->pcpbd;
    pCDynamicDialog pcdd = pcpsp->pcdd;

    if ( pcpsp->pageTitle == NULL )
    {
        char buf[32];
        _snprintf(buf, sizeof(buf), "Page %d", pcpsp->pageNumber);

        if ( ! setPageText(c, pcpsp, buf, pageText) )
        {
            return false;
        }
    }

    pcpbd->wndBase->sizeX = pcpsp->cx;
    pcpbd->wndBase->sizeY = pcpsp->cy;

    uint32_t style = DS_SHELLFONT | DS_3DLOOK | DS_CONTROL | WS_CHILD | WS_TABSTOP;

    DLGTEMPLATE *pBase;

    return startDialogTemplate(c, &pBase, pcdd, 0, 0, pcpsp->cx, pcpsp->cy, NULL, "", pcpbd->fontName, pcpbd->fontSize, style);
}

/**
 * Creates the in-memory dialog template for a user property sheet page.
 *
 * @param c      Method context we are operating in.
 * @param pcpsp  Pointer to the property sheet page CSelf
 *
 * @return True on success false on error.
 *
 * @remarks  For a user defined dialog template we always use "" for the dialog
 *           title in the template.  If the user has not assigned a page title,
 *           we create one for him.  When the
 *           see that the pageTitle field is not null and add the PSP
 */
RexxObjectPtr initUserTemplate(RexxMethodContext *c, pCPropertySheetPage pcpsp)
{
    if ( ! initPageDlgFrame(c, pcpsp) )
    {
        goto err_out;
    }

    pCPlainBaseDialog pcpbd = pcpsp->pcpbd;
    pCDynamicDialog pcdd = pcpsp->pcdd;

    RexxObjectPtr result = c->SendMessage0(pcpbd->rexxSelf, "DEFINEDIALOG");

    if ( ! c->CheckCondition() && pcdd->active != NULL )
    {
        pcpsp->pageID = (INT_PTR)pcdd->base;

        // Set the number of dialog items field in the dialog template.
        ((DLGTEMPLATE *)pcdd->base)->cdit = (WORD)pcdd->count;

        return TheTrueObj;
    }

err_out:
    // No underlying windows dialog can be created.  We still need to clean up
    // the CSelf struct, which was allocated when the Rexx dialog object was
    // instantiated.  But I'm not sure it needs to be done here.
    stopDialog(pcpbd, c->threadContext);
    return TheFalseObj;
}

RexxObjectPtr initRcTemplate(RexxMethodContext *c, pCPropertySheetPage pcpsp)
{
    pCPlainBaseDialog pcpbd = pcpsp->pcpbd;
    pCDynamicDialog pcdd = pcpsp->pcdd;

    RexxObjectPtr result = c->SendMessage1(pcpsp->rexxSelf, "LOADFRAME", c->UnsignedInt32(pcdd->expected));

    // Checking that result is 1 is probably sufficient.
    if ( ! isInt(0, result, c->threadContext) || c->CheckCondition() )
    {
        goto err_out;
    }

    if ( ! initPageDlgFrame(c, pcpsp) )
    {
        goto err_out;
    }

    result = c->SendMessage1(pcpsp->rexxSelf, "LOADITEMS", pcpsp->extraOpts);

    if ( ! isInt(0, result, c->threadContext) || pcdd->active == NULL )
    {
        goto err_out;
    }

    result = c->SendMessage0(pcpbd->rexxSelf, "DEFINEDIALOG");

    if ( ! c->CheckCondition() && pcdd->active != NULL )
    {
        pcpsp->pageID = (INT_PTR)pcdd->base;

        // Set the number of dialog items field in the dialog template.
        ((DLGTEMPLATE *)pcdd->base)->cdit = (WORD)pcdd->count;

        return TheTrueObj;
    }

err_out:
    // No underlying windows dialog can be created.  We still need to clean up
    // the CSelf struct, which was allocated when the Rexx dialog object was
    // instantiated.  But I'm not sure it needs to be done here.
    stopDialog(pcpbd, c->threadContext);
    return TheFalseObj;
}

RexxObjectPtr initResTemplate(RexxMethodContext *c, pCPropertySheetPage pcpsp)
{
    pCPlainBaseDialog pcpbd = pcpsp->pcpbd;

    if ( ! loadResourceDLL(pcpbd, pcpbd->library) )
    {
        goto err_out;
    }
    pcpsp->pageID = (INT_PTR)pcpsp->resID;

    return TheTrueObj;

err_out:
    // No underlying windows dialog can be created.  We still need to clean up
    // the CSelf struct, which was allocated when the Rexx dialog object was
    // instantiated.  But I'm not sure it needs to be done here.
    stopDialog(pcpbd, c->threadContext);
    return TheFalseObj;
}

/** PropertySheetPage::init()  [Class method]
 *
 *  Used to capture the PropertySheetPage class object.  This is used for scoped
 *  look ups of the CSelf.
 */
RexxMethod1(RexxObjectPtr, psp_init_cls, OSELF, self)
{
    ThePropertySheetPageClass = (RexxClassObject)self;
    return NULLOBJECT;
}


/** PropertySheetPage::propSheet()  [Attribute get]
 *
 */
RexxMethod1(RexxObjectPtr, psp_propSheet_atr, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pCSelf;
    return pcpsp->rexxPropSheet;
}


/** PropertySheetPage::wasActivated()  [Attribute get]
 *
 */
RexxMethod1(RexxObjectPtr, psp_wasActivated_atr, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pCSelf;
    return pcpsp->activated ? TheTrueObj : TheFalseObj;
}


/** PropertySheetPage::cx                [Attribute get]
 *  PropertySheetPage::cy
 *
 */
RexxMethod2(uint32_t, psp_getcx, NAME, name, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pCSelf;

    return *(name + 1) == 'X' ? pcpsp->cx : pcpsp->cy;
}


/** PropertySheetPage::cx                [Attribute set]
 *  PropertySheetPage::cy
 *
 */
RexxMethod3(RexxObjectPtr, psp_setcx, uint32_t, dlgUnit, NAME, name, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pCSelf;

    *(name + 1) == 'X' ? pcpsp->cx = dlgUnit : pcpsp->cy = dlgUnit;
    return NULLOBJECT;
}


/** PropertySheetPage::pageTitle()       [Attribute get]
 *  PropertySheetPage::headerTitle()
 *  PropertySheetPage::headerSubtitle()
 */
RexxMethod2(RexxObjectPtr, psp_getPageTitle, NAME, name, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pCSelf;

    switch ( *(name + 7) )
    {
        case 'L' :
            return pcpsp->pageTitle == NULL ? TheNilObj : context->String(pcpsp->pageTitle);
        case 'I' :
            return pcpsp->headerTitle == NULL ? TheNilObj : context->String(pcpsp->headerTitle);
    }
    return pcpsp->headerSubtitle == NULL ? TheNilObj : context->String(pcpsp->headerSubtitle);
}


/** PropertySheetPage::pageTitle()       [Attribute set]
 *  PropertySheetPage::headerTitle()
 *  PropertySheetPage::headerSubtitle()
 */
RexxMethod3(RexxObjectPtr, psp_setPageTitle, CSTRING, text, NAME, name, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pCSelf;

    switch ( *(name + 7) )
    {
        case 'L' :
            setPageText(context, pcpsp, text, pageText);
            break;

        case 'I' :
            setPageText(context, pcpsp, text, headerText);
            break;

        case 'U' :
            setPageText(context, pcpsp, text, headerSubtext);
            break;
    }
    return NULLOBJECT;
}


/** PropertySheetPage::wantAccelerators()   [Attribute]
 *  PropertySheetPage::wantGetObject()
 *
 */
RexxMethod2(RexxObjectPtr, psp_getWantNotification, NAME, methName, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pCSelf;
    RexxObjectPtr result = TheFalseObj;

    if ( *(methName + 4) == 'A' )
    {
        result = pcpsp->wantAccelerators ? TheTrueObj : TheFalseObj;
    }
    else
    {
        result = pcpsp->wantGetObject ? TheTrueObj : TheFalseObj;
    }
    return result;
}
RexxMethod3(RexxObjectPtr, psp_setWantNotification, logical_t, want, NAME, methName, CSELF, pCSelf)
{
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pCSelf;

    if ( *(methName + 4) == 'A' )
    {
        pcpsp->wantAccelerators = want ? true : false;
    }
    else
    {
        pcpsp->wantGetObject = want ? true : false;
    }
    return NULLOBJECT;
}


/** PropertySheetPage::init_propertySheetPage()
 *
 *  The property sheet page initialization.  This can only be done from native
 *  code.
 *
 *  @param cSelf  The Rexx Buffer object containing the CSelf for a property
 *                sheet page.
 */
RexxMethod1(logical_t, psp_init_propertySheetPage, RexxObjectPtr, cSelf)
{
    if ( ! context->IsBuffer(cSelf) )
    {
        wrongClassException(context->threadContext, 1, "Buffer");
        return FALSE;
    }

    context->SetObjectVariable("CSELF", cSelf);
    return TRUE;
}


/** PropertySheetPage::initTemplate()
 *
 *
 */
RexxMethod1(RexxObjectPtr, psp_initTemplate, CSELF, pCSelf)
{
    // TODO validate CSelf.
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pCSelf;

    switch ( pcpsp->pageType )
    {
        case oodUserPSPDialog :
            return initUserTemplate(context, pcpsp);
            break;

        case oodRcPSPDialog :
            return initRcTemplate(context, pcpsp);
            break;

        case oodResPSPDialog :
            return initResTemplate(context, pcpsp);
            break;

        default :
            break;

    }
    return TheFalseObj;
}




/**
 *  Methods for the .UserPSPDialog class.
 */
#define USERPSPDIALOG_CLASS  "UserPSPDialog"


RexxMethod9(RexxObjectPtr, userpspdlg_init, OPTIONAL_RexxObjectPtr, dlgData, OPTIONAL_RexxObjectPtr, includeFile,
            OPTIONAL_CSTRING, opts, OPTIONAL_CSTRING, title, OPTIONAL_CSTRING, fontName, OPTIONAL_uint32_t, fontSize,
            OPTIONAL_uint32_t, expected, SUPER, super, OSELF, self)
{
    RexxArrayObject newArgs = context->NewArray(2);

    if ( argumentExists(1) )
    {
        context->ArrayPut(newArgs, dlgData, 1);
    }
    if ( argumentExists(2) )
    {
        context->ArrayPut(newArgs, includeFile, 2);
    }

    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, newArgs);

    if ( isInt(0, result, context->threadContext) )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();
        pCDynamicDialog pcdd = (pCDynamicDialog)context->ObjectToCSelf(self, TheDynamicDialogClass);

        // We have a 0 result.  We set result back to an error code and then
        // reset it to 0 if we get through all the error checks
        result = TheOneObj;
        pcpbd->wndBase->initCode = 1;

        if ( ! initPropertySheetPage(context, pcpbd, pcdd, self) )
        {
            goto done_out;
        }

        pCPropertySheetPage pcpsp = (pCPropertySheetPage)pcpbd->dlgPrivate;
        pcpsp->pageType = oodUserPSPDialog;

        parsePageOpts(context, pcpsp, opts);

        pcdd->expected = (expected == 0 ? DEFAULT_EXPECTED_DIALOG_ITEMS : expected);
        pcpbd->isPageDlg = true;

        if ( argumentExists(4) )
        {
            if ( ! setPageText(context, pcpsp, title, pageText) )
            {
                goto done_out;
            }
        }
        if ( argumentExists(5) )
        {
            if ( strlen(fontName) > (MAX_DEFAULT_FONTNAME - 1) )
            {
                stringTooLongException(context->threadContext, 4, MAX_DEFAULT_FONTNAME, strlen(fontName));
                goto done_out;
            }
            strcpy(pcpbd->fontName, fontName);
        }
        if ( argumentExists(5) )
        {
            if ( fontSize == 0 )
            {
                context->RaiseException2(Rexx_Error_Invalid_argument_positive, context->WholeNumber(5), TheZeroObj);
                goto done_out;
            }
            pcpbd->fontSize = fontSize;
        }
        result = TheZeroObj;
        pcpbd->wndBase->initCode = 0;
    }

done_out:
    return result;
}


/**
 *  Methods for the .RcPSPDialog class.
 */
#define RCPSPDIALOG_CLASS  "RcPSPDialog"


RexxMethod9(RexxObjectPtr, rcpspdlg_init, RexxStringObject, scriptFile, RexxObjectPtr, resource, OPTIONAL_RexxObjectPtr, dlgData,
            OPTIONAL_RexxObjectPtr, includeFile, OPTIONAL_CSTRING, pageOpts, OPTIONAL_RexxStringObject, connectOpts,
            OPTIONAL_uint32_t, expected, SUPER, super, OSELF, self)
{
    RexxArrayObject newArgs = context->NewArray(4);

    context->ArrayPut(newArgs, scriptFile, 3);
    context->ArrayPut(newArgs, resource, 4);
    if ( argumentExists(3) )
    {
        context->ArrayPut(newArgs, dlgData, 1);
    }
    if ( argumentExists(4) )
    {
        context->ArrayPut(newArgs, includeFile, 2);
    }

    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, newArgs);

    if ( isInt(0, result, context->threadContext) )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();
        pCDynamicDialog pcdd = (pCDynamicDialog)context->ObjectToCSelf(self, TheDynamicDialogClass);

        if ( ! initPropertySheetPage(context, pcpbd, pcdd, self) )
        {
            result = TheOneObj;
            pcpbd->wndBase->initCode = 1;
            goto done_out;
        }

        pCPropertySheetPage pcpsp = (pCPropertySheetPage)pcpbd->dlgPrivate;
        pcpsp->pageType = oodRcPSPDialog;

        parsePageOpts(context, pcpsp, pageOpts);

        pcdd->expected = (expected == 0 ? DEFAULT_EXPECTED_DIALOG_ITEMS : expected);
        pcpbd->isPageDlg = true;

        pcpsp->extraOpts = (argumentExists(6) ? connectOpts : context->NullString());
    }

done_out:
    return result;
}


RexxMethod7(RexxObjectPtr, rcpspdlg_startTemplate, uint32_t, cx, uint32_t, cy, CSTRING, title, CSTRING, fontName,
            uint32_t, fontSize, uint32_t, expected, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)pcpbd->dlgPrivate;
    pCDynamicDialog pcdd = pcpsp->pcdd;

    pcpsp->cx = cx;
    pcpsp->cy = cy;

    if ( strlen(title) > 0 && ! setPageText(context, pcpsp, title, pageText) )
    {
        return TheOneObj;
    }

    size_t len = strlen(fontName);

    if ( len > (MAX_DEFAULT_FONTNAME - 1) )
    {
        stringTooLongException(c->threadContext, 4, MAX_DEFAULT_FONTNAME, len);
        return TheOneObj;
    }
    if ( len > 0 )
    {
        strcpy(pcpbd->fontName, fontName);
    }

    if ( fontSize != 0 )
    {
        pcpbd->fontSize = fontSize;
    }
    pcdd->expected = expected;

    return TheZeroObj;
}


/**
 *  Methods for the .ResPSPDialog class.
 */
#define RESPSPDIALOG_CLASS  "ResPSPDialog"


RexxMethod7(RexxObjectPtr, respspdlg_init, RexxStringObject, dllFile, RexxObjectPtr, resourceID, OPTIONAL_RexxObjectPtr, dlgData,
            OPTIONAL_RexxObjectPtr, includeFile, OPTIONAL_CSTRING, pageOpts, SUPER, super, OSELF, self)
{
    RexxArrayObject newArgs = context->NewArray(4);

    context->ArrayPut(newArgs, dllFile, 1);
    context->ArrayPut(newArgs, resourceID, 2);
    if ( argumentExists(3) )
    {
        context->ArrayPut(newArgs, dlgData, 3);
    }
    if ( argumentExists(4) )
    {
        context->ArrayPut(newArgs, includeFile, 4);
    }

    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, newArgs);

    if ( isInt(0, result, context->threadContext) )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();

        uint32_t resID = oodResolveSymbolicID(context, self, resourceID, -1, 2);
        if ( resID == OOD_ID_EXCEPTION )
        {
            result = TheOneObj;
            pcpbd->wndBase->initCode = 1;
            goto done_out;
        }
        if ( resID == 0 )
        {
            wrongArgValueException(context->threadContext, 2,
                                   "a valid positive numeric resource ID or a valid symbolic ID", resourceID);

            result = TheOneObj;
            pcpbd->wndBase->initCode = 1;
            goto done_out;
        }

        if ( ! initPropertySheetPage(context, pcpbd, NULL, self) )
        {
            result = TheOneObj;
            pcpbd->wndBase->initCode = 1;
            goto done_out;
        }

        pCPropertySheetPage pcpsp = (pCPropertySheetPage)pcpbd->dlgPrivate;

        pcpsp->pageType = oodResPSPDialog;
        pcpsp->resID = resID;

        parsePageOpts(context, pcpsp, pageOpts);

        pcpbd->isPageDlg = true;
    }

done_out:
    return result;
}


#if 0
/**
 * The property sheet page callback function.  Seems like it would have some
 * good uses, but in reality does not provide much that is of use for the
 * ooDialog framework.
 *
 * The thread attach is better done somewhere else.
 *
 * Temporarily saving the code.
 *
 *
 * @param hwnd  Reserved by MS, always NULL.
 * @param uMsg
 * @param ppsp
 *
 * @return UINT CALLBACK
 */
UINT CALLBACK PropSheetPageCallBack(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    pCPropertySheetPage pcpsp = (pCPropertySheetPage)ppsp->lParam;

    switch ( uMsg )
    {
        case PSPCB_ADDREF:
        {
            // Called when a page is being created.

            //printf("PSPCB_ADDREF pcpsp=%p threadID=%d \n", pcpsp, GetCurrentThreadId());

            break;
        }

        case PSPCB_CREATE:
            // Called when the dialog box for the page is being created.

            printf("PSPCB_CREATE pcpsp=%p threadID=%d hDlg? %p\n", pcpsp, GetCurrentThreadId(), pcpsp->pcpbd->hDlg);

            if ( pcpsp->dlgProcContext == NULL )
            {
                RexxThreadContext *context;
                if ( pcpsp->interpreter->AttachThread(&context) )
                {
                    pcpsp->dlgProcContext = context;
                    pcpsp->pcpbd->dlgProcContext = context;
                    ((pCPropertySheetDialog)pcpsp->cppPropSheet)->dlgProcContext = context;
                    RexxSetProcessMessages(FALSE);
                }
            }

            return TRUE;

        case PSPCB_RELEASE:
            // Called when the page is being destroyed

            //printf("PSPCB_RELEASE pcpsp=%p threadID=%d hDlg? %p hCurrent=%p\n", pcpsp, GetCurrentThreadId(), pcpsp->pcpbd->hDlg,
            //       PropSheet_GetCurrentPageHwnd(((pCPropertySheetDialog)pcpsp->cppPropSheet)->hDlg));

            if ( pcpsp->dlgProcContext != NULL )
            {
                pcpsp->dlgProcContext->DetachThread();
                pcpsp->dlgProcContext = NULL;
                pcpsp->pcpbd->dlgProcContext = NULL;
                ((pCPropertySheetDialog)pcpsp->cppPropSheet)->dlgProcContext = NULL;
            }
            break;
    }
    return 0;
}
#endif

