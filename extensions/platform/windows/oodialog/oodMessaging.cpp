/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
 * Open Object REXX OODialog - ooDialog Messaging function
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <shlwapi.h>
#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodMouse.hpp"
#include "oodData.hpp"
#include "oodPropertySheetDialog.hpp"
#include "oodResizableDialog.hpp"
#include "oodShared.hpp"
#include "oodMessaging.hpp"

inline bool isToolbarCustomizationCode(uint32_t code)
{
    return code == TBN_CUSTHELP      || code == TBN_DELETINGBUTTON || code == TBN_GETBUTTONINFO ||
           code == TBN_INITCUSTOMIZE || code == TBN_QUERYDELETE    || code == TBN_QUERYINSERT   ||
           code == TBN_RESET         || code == TBN_RESTORE        || code == TBN_SAVE          ||
           code == TBN_TOOLBARCHANGE;
}

/**
 * The dialog procedure function for all regular ooDialog dialogs.  Handles and
 * processes all window messages for the dialog.
 *
 * @param hDlg
 * @param uMsg
 * @param wParam
 * @param lParam
 *
 * @return LRESULT CALLBACK
 *
 * @remarks  The WM_INITDIALOG message.
 *
 *           In CreateDialogParam() / CreateDialogIndirectParam() we pass the
 *           pointer to the PlainBaseDialog CSelf as the param.  The OS then
 *           sends us this value as the LPARAM argument in the WM_INITDIALOG
 *           message. The pointer is stored in the user data field of the window
 *           words for this dialog.  We do the same thing for the child dialogs,
 *           see the WM_USER_CREATECHILD message.
 *
 *           Note that when the child dialogs of the category dialog get
 *           created, we recieve a WM_INITDIALOG for each of them.  These child
 *           dialogs are all running on the same thread as the parent category
 *           dialog.  We don't want to do a bunch of nested AttachThreads()
 *           because we only do 1 DetachThread() for each window message loop.
 *           So, we check to see if dlgProcContext is null before doing the
 *           AttachThread().  Note that as of ooDialog 4.2.0, the CategoryDialog
 *           class is deprecated.
 *
 *           The WM_USER_CREATECHILD message.
 *
 *           This user message's purpose is to create a child dialog of this
 *           dialog and return its window handle. Child dialogs are only created
 *           to implement the CategoryDialog and at this time are always created
 *           dynamically (from an in-memory template.) The dialog template
 *           pointer is passed here as the LPARAM arg from
 *           DynamicDialog::startChildDialog().
 *
 *           These child dialogs do not have a backing Rexx dialog. There is no
 *           unique CPlainBaseDialog struct for them.  Instead, at this time,
 *           all interaction with the child dialogs is done through the
 *           CPlainBaseDialog struct of the parent.  For each child dialog, we
 *           set the CPlainBaseDialog struct of the parent in the window words
 *           of the child dialog.  Prior to the conversion of ooDialog to the
 *           C++ API, when a message came in for a child dialog, a search was
 *           made through the DialogTable to try and find the parent dialog.
 *           This has been disposed of and the CPlainBaseDialog struct is just
 *           pulled out of the window words.  Note that as of ooDialog 4.2.0,
 *           the CategoryDialog class is deprecated.
 *
 *           The WM_USER_CREATECONTROL_DLG
 *               WM_USER_CREATECONTROL_RESDLG messages.
 *
 *           These user messages create true child dialogs with a backing Rexx
 *           dialog and individual CSelfs (CPlainBaseDialog structs).
 *
 *           The WM_USER_CREATEPROPSHEET_DLG message.
 *
 *           This user messages creates a modeless Windows PropertySheet dialog.
 *           This dialog does have a backing Rexx dialog along with an
 *           individual CSelf.
 *
 *   @remarks  The existing, old, architecture of ooDialog uses delDialog() to
 *             both terminate the dialog and clean up the CSelf struct. No
 *             WM_COSE is sent to the window procdure.  Rather, in delDialog() a
 *             DestroyWindow() and PostQuitMessage() is done.  But, the
 *             PostQuitMessage() is done before DestroyWindow() AND
 *             DestroyWindow() can not be used on a window in a different
 *             thread.  Because of this, 1.) This procedure never gets a
 *             WM_DESTROY message.  2.) The DestroyWindow() probably usually
 *             fails.  This is likely the cause of memory leaks.
 *
 *             This whole architecture need to be re-thought out.  Rather than
 *             use delDialog() to terminate the dialog, it seems to me it would
 *             be better to send a WM_CLOSE message, do a DestroyWindow() in the
 *             WM_CLOSE processing, and use the WM_DESTROY processing to clean
 *             up.  I.e., use the normal Windows strategy.
 *
 *             It is tempting to set pcpbd->hDlg here.  But, if we do we break
 *             the deprecated CategoryDialog.  So, for now we don't do that.  At
 *             some point, it would be best to simply say the CategoryDialog
 *             support is removed and do that.  Note that the chidl dialogs of
 *             the CategoryDialog, *only* use this window procedure.  So, it is
 *             only here that we need to worry about this.  It is the
 *             customDrawCheckIDs() that needs the handle.  No category child
 *             dialog can have custom draw, so we set pcpbd->hDlg if we are
 *             going to call that function.
 */
LRESULT CALLBACK RexxDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_INITDIALOG )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)lParam;
        if ( pcpbd == NULL )
        {
            // Theoretically impossible.  But ... if it should happen, abort.
            return endDialogPremature(pcpbd, hDlg, NoPCPBDpased);
        }

        if ( pcpbd->dlgProcContext == NULL )
        {
            RexxThreadContext *context;
            if ( ! pcpbd->interpreter->AttachThread(&context) )
            {
                // Again, this shouldn't happen ... but
                return endDialogPremature(pcpbd, hDlg, NoThreadAttach);
            }
            pcpbd->dlgProcContext = context;

            RexxSetProcessMessages(FALSE);
        }

        setWindowPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pcpbd);

        if ( pcpbd->isCustomDrawDlg && pcpbd->idsNotChecked )
        {
            pcpbd->hDlg = hDlg;

            // We don't care what the outcome of this is, customDrawCheckIDs
            // will take care of aborting this dialog if the IDs are bad.
            customDrawCheckIDs(pcpbd);
        }

        return TRUE;
    }

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)getWindowPtr(hDlg, GWLP_USERDATA);
    if ( pcpbd == NULL )
    {
        // A number of messages arrive before WM_INITDIALOG, we just ignore them.
        return FALSE;
    }

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
        // Under all normal circumstances, WM_DESTROY never gets here.  But if
        // it does, it is because of some unexplained / unanticpated error.
        // PostQuitMessage() will cause the window message loop to quit and
        // things should then (hopefully) unwind cleanly.  See the remarks in he
        // header comment.
        PostQuitMessage(3);
        return TRUE;
    }

    // The Toolbar customization dialog is a modal dialog. When it starts our
    // dialog is disabled and the toolbar dialog starts sending our dialog TBN_
    // messages.  We never get them with msgEnabled false.  So, we have added
    // another function to test for that special case.
    bool msgEnabled = (IsWindowEnabled(hDlg) ||
                       (uMsg == WM_NOTIFY && isToolbarCustomizationCode(((NMHDR *)lParam)->code))) ? true : false;

    // Do not search message table for WM_PAINT to improve redraw.
    if ( msgEnabled && uMsg != WM_PAINT && uMsg != WM_NCPAINT )
    {
        MsgReplyType searchReply = searchMessageTables(uMsg, wParam, lParam, pcpbd);
        if ( searchReply != ContinueProcessing )
        {
            // Note pre 4.0.1, we always returned FALSE, (pass on to the system
            // to process.) But, post 4.0.1 we sometimes reply TRUE, the message
            // has been handled.
            return (searchReply == ReplyTrue ? TRUE : FALSE);
        }
    }

    if ( uMsg >= WM_USER_REXX_FIRST && uMsg <= WM_USER_REXX_LAST )
    {
        return handleWmUser(pcpbd, hDlg, uMsg, wParam, lParam, false);
    }

    switch ( uMsg )
    {
        case WM_PAINT:
            return drawBackgroundBmp(pcpbd, hDlg);

        case WM_DRAWITEM:
            return drawBitmapButton(pcpbd, lParam, msgEnabled);

        case WM_CTLCOLORDLG:
            return handleDlgColor(pcpbd);

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORSCROLLBAR:
            return handleCtlColor(pcpbd, hDlg, uMsg, wParam, lParam);

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            return paletteMessage(pcpbd, hDlg, uMsg, wParam, lParam);

        default:
            break;
    }

    return FALSE;
}

/**
 * The dialog procedure for control dialogs, i.e. those created with the
 * DS_CONTROL.
 *
 * These are 'nested' dialogs, or dialogs within a top-level dialog.  For the
 * most part, the procedure is exactly the same as for top-level dialogs.
 *
 * @param hDlg
 * @param uMsg
 * @param wParam
 * @param lParam
 *
 * @return LRESULT CALLBACK
 */
LRESULT CALLBACK RexxChildDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_INITDIALOG )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)lParam;
        if ( pcpbd == NULL )
        {
            // Theoretically impossible.  But ... if it should happen, abort.
            return endDialogPremature(pcpbd, hDlg, NoPCPBDpased);
        }

        pcpbd->hDlg = hDlg;
        setWindowPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pcpbd);

        if ( pcpbd->isCustomDrawDlg && pcpbd->idsNotChecked )
        {
            // We don't care what the outcome of this is, customDrawCheckIDs
            // will take care of aborting this dialog if the IDs are bad.
            customDrawCheckIDs(pcpbd);
        }

        if ( pcpbd->isResizableDlg )
        {
            return initializeResizableDialog(hDlg, pcpbd->dlgProcContext, pcpbd);
        }
        return TRUE;
    }

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)getWindowPtr(hDlg, GWLP_USERDATA);
    if ( pcpbd == NULL )
    {
        // A number of messages arrive before WM_INITDIALOG, we just ignore them.
        return FALSE;
    }

    if ( pcpbd->dlgProcContext == NULL )
    {
        if ( ! pcpbd->isActive )
        {
            return FALSE;
        }

        // Once again, theoretically impossible ...
        return endDialogPremature(pcpbd, hDlg, NoThreadContext);
    }

    // Don't process WM_DESTROY messages.

    // We first deal with resizable stuff, then handle the rest with the normal
    // ooDialog process.
    if ( pcpbd->isResizableDlg )
    {
        MsgReplyType resizingReply = handleResizing(hDlg, uMsg, wParam, lParam, pcpbd);
        if ( resizingReply != ContinueProcessing )
        {
            return (resizingReply == ReplyTrue ? TRUE : FALSE);
        }
    }

    bool msgEnabled = IsWindowEnabled(hDlg) ? true : false;

    // Do not search message table for WM_PAINT to improve redraw.
    if ( msgEnabled && uMsg != WM_PAINT && uMsg != WM_NCPAINT )
    {
        MsgReplyType searchReply = searchMessageTables(uMsg, wParam, lParam, pcpbd);
        if ( searchReply != ContinueProcessing )
        {
            // Note pre 4.0.1, we always returned FALSE, (pass on to the system
            // to process.) But, post 4.0.1 we sometimes reply TRUE, the message
            // has been handled.
            return (searchReply == ReplyTrue ? TRUE : FALSE);
        }
    }

    if ( uMsg >= WM_USER_REXX_FIRST && uMsg <= WM_USER_REXX_LAST )
    {
        return handleWmUser(pcpbd, hDlg, uMsg, wParam, lParam, true);
    }

    switch ( uMsg )
    {
        case WM_PAINT:
            return drawBackgroundBmp(pcpbd, hDlg);

        case WM_DRAWITEM:
            return drawBitmapButton(pcpbd, lParam, msgEnabled);

        case WM_CTLCOLORDLG:
            return handleDlgColor(pcpbd);

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORSCROLLBAR:
            return handleCtlColor(pcpbd, hDlg, uMsg, wParam, lParam);

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            return paletteMessage(pcpbd, hDlg, uMsg, wParam, lParam);

        case WM_COMMAND:
            return handleWmCommand(pcpbd, hDlg, wParam, lParam, true);

        default:
            break;
    }

    return FALSE;
}


/**
 * Checks if the specified message is one that we don't allow in a nested
 * dialog.
 *
 * @param uMsg
 *
 * @return bool
 *
 * @remarks  For now, we don't let the user created nested, nested dialogs.  In
 *           addition, keyboard hooks should only be created in a top-level
 *           dialog.
 */
inline bool isRestrictedUserMsg(uint32_t uMsg)
{
    if ( uMsg == WM_USER_CREATECHILD          ||
         uMsg == WM_USER_CREATECONTROL_DLG    ||
         uMsg == WM_USER_CREATECONTROL_RESDLG ||
         uMsg == WM_USER_HOOK )
    {
        return true;
    }
    return false;
}


/**
 *  Handles all our WM_USER_xxx messages for any Rexx dialog procedure.
 *
 *  Note that we check if the message is from a nested dialog procedure, like a
 *  property sheet page dialog, and don't allow certain of the messages.
 *
 * @param pcpbd
 * @param hDlg
 * @param uMsg
 * @param wParam
 * @param lParam
 * @param isNestedDlg
 *
 * @return LRESULT
 *
 * @remarks  We are only in this functrion for our own Rexx user messages, and
 *           we process all of them.  So, we always return true.  If we add a
 *           message and forget to process it, returning true will do no harm.
 *           It should be easy enough to find the bug when we look at why the
 *           new message doesn't work.
 */
LRESULT handleWmUser(pCPlainBaseDialog pcpbd, HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, bool isNestedDlg)
{
    if ( isNestedDlg && isRestrictedUserMsg(uMsg) )
    {
        ReplyMessage((LRESULT)NULL);
        return TRUE;
    }

    switch ( uMsg )
    {
        case WM_USER_CREATECHILD:
        {
            HWND hChild = CreateDialogIndirectParam(MyInstance, (LPCDLGTEMPLATE)lParam, hDlg, (DLGPROC)RexxDlgProc,
                                                    (LPARAM)pcpbd);
            ReplyMessage((LRESULT)hChild);
            return TRUE;
        }

        case WM_USER_CREATECONTROL_DLG:
        {
            pCPlainBaseDialog p = (pCPlainBaseDialog)wParam;
            HWND hChild = CreateDialogIndirectParam(MyInstance, (LPCDLGTEMPLATE)lParam, p->hOwnerDlg, (DLGPROC)RexxChildDlgProc,
                                                    (LPARAM)p);
            ReplyMessage((LRESULT)hChild);
            return TRUE;
        }

        case WM_USER_CREATECONTROL_RESDLG:
        {
            pCPlainBaseDialog p = (pCPlainBaseDialog)wParam;

            HWND hChild = CreateDialogParam(p->hInstance, MAKEINTRESOURCE((uint32_t)lParam), p->hOwnerDlg,
                                            (DLGPROC)RexxChildDlgProc, (LPARAM)p);

            ReplyMessage((LRESULT)hChild);
            return TRUE;
        }

        case WM_USER_CREATEPROPSHEET_DLG:
        {
            pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)lParam;

            assignPSDThreadContext(pcpsd, pcpbd->dlgProcContext, GetCurrentThreadId());

            if ( setPropSheetHook(pcpsd) )
            {
                SetLastError(0);
                intptr_t ret = PropertySheet((PROPSHEETHEADER *)wParam);
                oodSetSysErrCode(pcpbd->dlgProcContext);
                ReplyMessage((LRESULT)ret);
            }
            else
            {
                ReplyMessage((LRESULT)-1);
            }

            return TRUE;
        }

        case WM_USER_CREATETOOLTIP:
        {
            PCREATETOOLTIP ctt = (PCREATETOOLTIP)wParam;

            HWND hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, ctt->style, CW_USEDEFAULT, CW_USEDEFAULT,
                                           CW_USEDEFAULT, CW_USEDEFAULT, hDlg, NULL, ctt->hInstance, NULL);

            if ( hToolTip == NULL )
            {
                ctt->errRC = GetLastError();
            }
            ReplyMessage((LRESULT)hToolTip);
            return TRUE;
        }

        case WM_USER_INTERRUPTSCROLL:
            pcpbd->stopScroll = wParam;
            return TRUE;

        case WM_USER_GETFOCUS:
            ReplyMessage((LRESULT)GetFocus());
            return TRUE;

        case WM_USER_MOUSE_MISC:
        {
            switch ( wParam )
            {
                case MF_RELEASECAPTURE :
                {
                    uint32_t rc = 0;
                    if ( ReleaseCapture() == 0 )
                    {
                        rc = GetLastError();
                    }
                    ReplyMessage((LRESULT)rc);
                    break;
                }

                case MF_GETCAPTURE :
                    ReplyMessage((LRESULT)GetCapture());
                    break;

                case MF_SETCAPTURE :
                    ReplyMessage((LRESULT)SetCapture((HWND)lParam));
                    break;

                case MF_BUTTONDOWN :
                    ReplyMessage((LRESULT)GetAsyncKeyState((int)lParam));
                    break;

                case MF_SHOWCURSOR :
                    ReplyMessage((LRESULT)ShowCursor((BOOL)lParam));
                    break;

                default :
                    // Maybe we should raise an internal exception here.  But,
                    // as long as the internal code is consistent, we can not
                    // get here.
                    break;
            }
            return TRUE;
        }

        case WM_USER_SUBCLASS:
        {
            pSubClassData pData = (pSubClassData)lParam;

            BOOL success = SetWindowSubclass(pData->hCtrl, (SUBCLASSPROC)wParam, pData->id, (DWORD_PTR)pData);

            ReplyMessage((LRESULT)success);
            return TRUE;
        }

        case WM_USER_SUBCLASS_REMOVE:
            ReplyMessage((LRESULT)RemoveWindowSubclass(GetDlgItem(hDlg, (int)lParam), (SUBCLASSPROC)wParam, (int)lParam));
            return TRUE;

        case WM_USER_HOOK:
        {
            ReplyMessage((LRESULT)SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)wParam, NULL, GetCurrentThreadId()));
            return TRUE;
        }

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

    //
    return TRUE;
}


/**
 * Handles the WM_COMMAND message in the dialog window procedure.  Called from
 * any of the various Rexx dialog procedures in ooDialog.
 *
 * @param pcpbd
 * @param hDlg
 * @param wParam
 * @param lParam
 * @param isNestedDlg
 *
 * @return LRESULT
 *
 * @remarks  Note that for nested dialogs, we have always returned false since
 *           they were introduced.  Not sure if it might not be better to return
 *           true.
 *
 *           We only process the IDOK and IDCANCEL identifier when the high
 *           word is 0.  The high word being 0 would indicate that it was a
 *           button click (BN_CLICK is 0) or sent from a menu command.
 *
 *           However, we should never actually get to process anything here
 *           because both IDOK and IDCANCEL should have been interecepted in
 *           searchMessageTables().  But - sometimes we do get one or the other,
 *           very rarely.  It is on some abnormal error.  The processing seems
 *           to handle that error.
 */
LRESULT handleWmCommand(pCPlainBaseDialog pcpbd, HWND hDlg, WPARAM wParam, LPARAM lParam, bool isNestedDlg)
{
    if ( isNestedDlg )
    {
        return FALSE;
    }

    switch ( LOWORD(wParam) )
    {
        case IDOK:
        case IDCANCEL:
            if ( HIWORD(wParam) == 0 )
            {
                //
                pcpbd->abnormalHalt = true;
                DestroyWindow(hDlg);

                return TRUE;
            }
    }

    return FALSE;
}

static RexxStringObject wmsz2string(RexxThreadContext *c, WPARAM wParam)
{
    CSTRING s;

    switch ( wParam )
    {
        case WMSZ_BOTTOM :
            s = "BOTTOM";
            break;
        case WMSZ_BOTTOMLEFT :
            s = "BOTTOMLEFT";
            break;
        case WMSZ_BOTTOMRIGHT :
            s = "BOTTOMRIGHT";
            break;
        case WMSZ_LEFT :
            s = "LEFT";
            break;
        case WMSZ_RIGHT :
            s = "RIGHT";
            break;
        case WMSZ_TOP :
            s = "TOP";
            break;
        case WMSZ_TOPLEFT :
            s = "TOPLEFT";
            break;
        case WMSZ_TOPRIGHT :
            s = "TOPRIGHT";
            break;
        default :
            s = "UNKNOWN";
            break;
    }
    return c->String(s);
}

/**
 * Checks if a SYSTEMTIME struct's values are all 0.
 *
 * @param sysTime  Pointer to the struct to check.
 *
 * @return True if all 0 and false other wise.
 *
 * @remarks  We don't actually check all the fields of the struct, year, month,
 *           day, hour seem sufficient.
 */
inline bool isZeroDate(SYSTEMTIME *sysTime)
{
    return sysTime->wYear == 0 && sysTime->wMonth == 0 && sysTime->wDay == 0 &&
           sysTime->wHour == 0;
}

/**
 * Return true if code is a tool tip notification codes.  TTN_*
 */
inline bool isTTN(uint32_t code)
{
    return (code >= TTN_LAST && code <= TTN_FIRST);
}

/**
 * Return true if code matches the lpFilter / lParam combination in a message
 * table entry.
 */
inline bool isCodeMatch(MESSAGETABLEENTRY *m, uint32_t code, register size_t i)
{
    return ((code & m[i].lpFilter) == (uint32_t)m[i].lParam);
}


/**
 *  Converts arguments to a Rexx method to their C values when adding a window
 *  message filter to a MESSAGETABLE.  Used by both the dialog object and the
 *  dialog control object.
 *
 *  Should go without saying that the arguments to any Rexx method that uses
 *  this helper function need be the same and in the same argument position.
 *
 * @param pwmf  Pointer to a WinMessageFilter struct that is used to pass in the
 *              Rexx arguments and return their C values on success.
 *
 * @return  True on success, false on error.  On error, an exceptions has been
 *          raised.
 */
bool parseWinMessageFilter(RexxMethodContext *context, pWinMessageFilter pwmf)
{
    bool result = false;

    if ( pwmf->method[0] == '\0' )
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_null, TheOneObj);
        goto done_out;
    }

    uint32_t winMessage;
    uint32_t wmFilter;
    if ( ! rxStr2Number32(context, pwmf->_wm, &winMessage, 2) )
    {
        goto done_out;
    }

    if ( argumentOmitted(3) )
    {
        wmFilter = 0xFFFFFFFF;
    }
    else
    {
        if ( ! rxStr2Number32(context, pwmf->_wmFilter, &wmFilter, 3) )
        {
            goto done_out;
        }
    }

    uint64_t  filter;
    WPARAM    wParam;
    ULONG_PTR wpFilter;

    if ( ! oodGetWParam(context, pwmf->_wp, &wParam, 4, false) )
    {
        goto done_out;
    }
    if ( argumentOmitted(5) )
    {
        wpFilter = 0;
    }
    else
    {
        if ( ! rxStr2Number(context, pwmf->_wpFilter, &filter, 5) )
        {
            goto done_out;
        }
        wpFilter = (filter == 0xFFFFFFFF ? (ULONG_PTR)SIZE_MAX : (ULONG_PTR)filter);
    }

    LPARAM    lParam;
    ULONG_PTR lpFilter;

    if ( ! oodGetLParam(context, pwmf->_lp, &lParam, 6, false) )
    {
        goto done_out;
    }
    if ( argumentOmitted(7) )
    {
        lpFilter = 0;
    }
    else
    {
        if ( ! rxStr2Number(context, pwmf->_lpFilter, &filter, 7) )
        {
            goto done_out;
        }
        lpFilter = (filter == 0xFFFFFFFF ? (ULONG_PTR)SIZE_MAX : (ULONG_PTR)filter);
    }

    if ( (winMessage | wParam | lParam) == 0 )
    {
        userDefinedMsgException(context->threadContext, "The wm, wp, and lp arguements can not all be 0" );
        goto done_out;
    }

    pwmf->wm       = winMessage;
    pwmf->wmFilter = wmFilter;
    pwmf->wp       = wParam;
    pwmf->wpFilter = wpFilter;
    pwmf->lp       = lParam;
    pwmf->lpFilter = lpFilter;
    result = true;

done_out:
  return result;
}

/**
 * Convert an owner draw type to the ooDialog control type
 *
 * @param odt
 * @return oodControl_t
 */
inline oodControl_t odt2oodt(uint32_t odt)
{
    switch (odt)
    {
        case ODT_BUTTON   : return winPushButton;
        case ODT_COMBOBOX : return winComboBox;
        case ODT_HEADER   : return winUnknown;
        case ODT_LISTBOX  : return winListBox;
        case ODT_LISTVIEW : return winListView;
        case ODT_MENU     : return winUnknown;
        case ODT_STATIC   : return winStatic;
        case ODT_TAB      : return winTab;
        default           : return winUnknown;
    }
}

/**
 * Turn the different owner draw flags in to a list of keywords
 *
 * @param c
 * @param lpDI
 *
 * @return RexxStringObject
 */
static RexxStringObject od2keywords(RexxThreadContext *c, LPDRAWITEMSTRUCT lpDI)
{
    char buf[512];
    buf[0] = '\0';

    if (      lpDI->CtlType == ODT_BUTTON )    strcat(buf, "ODT_BUTTON ");
    else if ( lpDI->CtlType == ODT_COMBOBOX )  strcat(buf, "ODT_COMBOBOX ");
    else if ( lpDI->CtlType == ODT_HEADER )    strcat(buf, "ODT_HEADER ");
    else if ( lpDI->CtlType == ODT_LISTBOX )   strcat(buf, "ODT_LISTBOX ");
    else if ( lpDI->CtlType == ODT_LISTVIEW )  strcat(buf, "ODT_LISTVIEW ");
    else if ( lpDI->CtlType == ODT_MENU )      strcat(buf, "ODT_MENU ");
    else if ( lpDI->CtlType == ODT_STATIC )    strcat(buf, "ODT_STATIC ");
    else if ( lpDI->CtlType == ODT_TAB )       strcat(buf, "ODT_TAB ");
    else                                       strcat(buf, "ODT_UNKNOWN ");

    if ( lpDI->itemAction & ODA_DRAWENTIRE )   strcat(buf, "ODA_DRAWENTIRE ");
    if ( lpDI->itemAction & ODA_FOCUS      )   strcat(buf, "ODA_FOCUS ");
    if ( lpDI->itemAction & ODA_SELECT     )   strcat(buf, "ODA_SELECT ");

    if ( lpDI->itemState & ODS_CHECKED      )  strcat(buf, "ODS_CHECKED ");
    if ( lpDI->itemState & ODS_COMBOBOXEDIT )  strcat(buf, "ODS_COMBOBOXEDIT ");
    if ( lpDI->itemState & ODS_DEFAULT      )  strcat(buf, "ODS_DEFAULT ");
    if ( lpDI->itemState & ODS_DISABLED     )  strcat(buf, "ODS_DISABLED ");
    if ( lpDI->itemState & ODS_FOCUS        )  strcat(buf, "ODS_FOCUS ");
    if ( lpDI->itemState & ODS_GRAYED       )  strcat(buf, "ODS_GRAYED ");
    if ( lpDI->itemState & ODS_HOTLIGHT     )  strcat(buf, "ODS_HOTLIGHT ");
    if ( lpDI->itemState & ODS_INACTIVE     )  strcat(buf, "ODS_INACTIVE ");
    if ( lpDI->itemState & ODS_NOACCEL      )  strcat(buf, "ODS_NOACCEL ");
    if ( lpDI->itemState & ODS_NOFOCUSRECT  )  strcat(buf, "ODS_NOFOCUSRECT ");
    if ( lpDI->itemState & ODS_SELECTED     )  strcat(buf, "ODS_SELECTED ");

    if ( buf[0] != '\0' )
    {
        buf[strlen(buf) - 1] = '\0';
    }
    return c->String(buf);
}


// Some HTx values are negative
int32_t keyword2ncHitTestt(CSTRING keyword)
{
    int32_t ht;

    if ( StrCmpI(keyword,      "ERROR"              ) == 0 ) ht = HTERROR               ;
    else if ( StrCmpI(keyword, "TRANSPARENT"        ) == 0 ) ht = HTTRANSPARENT         ;
    else if ( StrCmpI(keyword, "NOWHERE"            ) == 0 ) ht = HTNOWHERE             ;
    else if ( StrCmpI(keyword, "CLIENT"             ) == 0 ) ht = HTCLIENT              ;
    else if ( StrCmpI(keyword, "CAPTION"            ) == 0 ) ht = HTCAPTION             ;
    else if ( StrCmpI(keyword, "SYSMENU"            ) == 0 ) ht = HTSYSMENU             ;
    else if ( StrCmpI(keyword, "SIZE"               ) == 0 ) ht = HTSIZE                ;
    else if ( StrCmpI(keyword, "MENU"               ) == 0 ) ht = HTMENU                ;
    else if ( StrCmpI(keyword, "HSCROLL"            ) == 0 ) ht = HTHSCROLL             ;
    else if ( StrCmpI(keyword, "VSCROLL"            ) == 0 ) ht = HTVSCROLL             ;
    else if ( StrCmpI(keyword, "MINBUTTON"          ) == 0 ) ht = HTMINBUTTON           ;
    else if ( StrCmpI(keyword, "MAXBUTTON"          ) == 0 ) ht = HTMAXBUTTON           ;
    else if ( StrCmpI(keyword, "LEFT"               ) == 0 ) ht = HTLEFT                ;
    else if ( StrCmpI(keyword, "RIGHT"              ) == 0 ) ht = HTRIGHT               ;
    else if ( StrCmpI(keyword, "TOP"                ) == 0 ) ht = HTTOP                 ;
    else if ( StrCmpI(keyword, "TOPLEFT"            ) == 0 ) ht = HTTOPLEFT             ;
    else if ( StrCmpI(keyword, "TOPRIGHT"           ) == 0 ) ht = HTTOPRIGHT            ;
    else if ( StrCmpI(keyword, "BOTTOM"             ) == 0 ) ht = HTBOTTOM              ;
    else if ( StrCmpI(keyword, "BOTTOMLEFT"         ) == 0 ) ht = HTBOTTOMLEFT          ;
    else if ( StrCmpI(keyword, "BOTTOMRIGHT"        ) == 0 ) ht = HTBOTTOMRIGHT         ;
    else if ( StrCmpI(keyword, "BORDER"             ) == 0 ) ht = HTBORDER              ;
    else if ( StrCmpI(keyword, "OBJECT"             ) == 0 ) ht = HTOBJECT              ;
    else if ( StrCmpI(keyword, "CLOSE"              ) == 0 ) ht = HTCLOSE               ;
    else if ( StrCmpI(keyword, "HELP"               ) == 0 ) ht = HTHELP                ;
    else
    {
        // Would like to raise an exception, but then we need to do the whole
        // print condition, end dialog premature, etc., etc..

        //wrongArgValueException(c->threadContext, 2, TBN_KEYWORDS, keyword);
        ;
    }

    return ht;
}

CSTRING ncHitTest2string(WPARAM hit)
{
    switch ( hit )
    {
        case HTERROR       : return "ERROR";
        case HTTRANSPARENT : return "TRANSPARENT";
        case HTNOWHERE     : return "NOWHERE";
        case HTCLIENT      : return "CLIENT";
        case HTCAPTION     : return "CAPTION";
        case HTSYSMENU     : return "SYSMENU";
        case HTSIZE        : return "SIZE";
        case HTMENU        : return "MENU";
        case HTHSCROLL     : return "HSCROLL";
        case HTVSCROLL     : return "VSCROLL";
        case HTMINBUTTON   : return "MINBUTTON";
        case HTMAXBUTTON   : return "MAXBUTTON";
        case HTLEFT        : return "LEFT";
        case HTRIGHT       : return "RIGHT";
        case HTTOP         : return "TOP";
        case HTTOPLEFT     : return "TOPLEFT";
        case HTTOPRIGHT    : return "TOPRIGHT";
        case HTBOTTOM      : return "BOTTOM";
        case HTBOTTOMLEFT  : return "BOTTOMLEFT";
        case HTBOTTOMRIGHT : return "BOTTOMRIGHT";
        case HTBORDER      : return "BORDER";
        case HTOBJECT      : return "OBJECT";
        case HTCLOSE       : return "CLOSE";
        case HTHELP        : return "HELP";
    }
    return "err";
}


RexxObjectPtr ncHitTest2string(RexxThreadContext *c, WPARAM hit)
{
    CSTRING keyword;

    switch ( hit )
    {
        case HTERROR       : keyword = "ERROR"; break;
        case HTTRANSPARENT : keyword = "TRANSPARENT"; break;
        case HTNOWHERE     : keyword = "NOWHERE"; break;
        case HTCLIENT      : keyword = "CLIENT"; break;
        case HTCAPTION     : keyword = "CAPTION"; break;
        case HTSYSMENU     : keyword = "SYSMENU"; break;
        case HTSIZE        : keyword = "SIZE"; break;
        case HTMENU        : keyword = "MENU"; break;
        case HTHSCROLL     : keyword = "HSCROLL"; break;
        case HTVSCROLL     : keyword = "VSCROLL"; break;
        case HTMINBUTTON   : keyword = "MINBUTTON"; break;
        case HTMAXBUTTON   : keyword = "MAXBUTTON"; break;
        case HTLEFT        : keyword = "LEFT"; break;
        case HTRIGHT       : keyword = "RIGHT"; break;
        case HTTOP         : keyword = "TOP"; break;
        case HTTOPLEFT     : keyword = "TOPLEFT"; break;
        case HTTOPRIGHT    : keyword = "TOPRIGHT"; break;
        case HTBOTTOM      : keyword = "BOTTOM"; break;
        case HTBOTTOMLEFT  : keyword = "BOTTOMLEFT"; break;
        case HTBOTTOMRIGHT : keyword = "BOTTOMRIGHT"; break;
        case HTBORDER      : keyword = "BORDER"; break;
        case HTOBJECT      : keyword = "OBJECT"; break;
        case HTCLOSE       : keyword = "CLOSE"; break;
        case HTHELP        : keyword = "HELP"; break;
        default            : keyword = "err"; break;
    }
    return c->String(keyword);
}


/**
 * This function will (should) cleanly end the dialog and the Rexx dialog object
 * when things needed to be terminated for unusual reason.  It can put up a
 * message box to inform the user of the circumstances depending on the args.
 *
 * @param pcpbd  Pointer to CSelf struct for the dialog (the PlainBaseDialog
 *               CSelf.)
 * @param hDlg   Window handle of the dialog.
 * @param t      Error type.
 *
 * @return False always.
 *
 * @remarks  For all the original types of ooDialog dialogs (4.0.1 and
 *           previous,) DestroyWindow() causes a WM_DESTROY message to reach
 *           RexxDlgProc(). The dialog procedure then posts a quit message, we
 *           fall out of the message loop and things unwind cleanly.
 *
 *           Setting abnormalHalt to true ensures we don't invoke leaving() and
 *           that the dialog return gets set to 2.  Calling ensureFinished()
 *           terminates the thread waiting on the finished instance variable.
 *
 *           This works fine for modeless property sheets, but modal property
 *           sheets hang.  The abortPropertySheet() essentially does a
 *           programmatic close and prevents any of the property sheet pages
 *           from nixing the close.
 *
 *           Note that if we do not call ensureFinished here, we get a crash in
 *           the Window message processing loop.
 */
BOOL endDialogPremature(pCPlainBaseDialog pcpbd, HWND hDlg, DlgProcErrType t)
{
    char buf[256];
    bool noMsg = false;

    switch ( t )
    {
        case NoPCPBDpased :
            _snprintf(buf, sizeof(buf), NO_PCPBD_PASSED_MSG, pcpbd, hDlg);
            break;
        case NoPCPSPpased :
            _snprintf(buf, sizeof(buf), NO_PCPSP_PASSED_MSG, pcpbd, hDlg);
            break;
        case NoThreadAttach :
            _snprintf(buf, sizeof(buf), NO_THREAD_ATTACH_MSG, pcpbd, hDlg);
            break;
        case NoThreadAttachOther :
            _snprintf(buf, sizeof(buf), NO_THREAD_ATTACH_OTHER_MSG, pcpbd, hDlg);
            break;
        case NoThreadContext :
            _snprintf(buf, sizeof(buf), NO_THREAD_CONTEXT_MSG, pcpbd->dlgProcContext, hDlg);
            break;
        case RexxConditionRaised :
            noMsg = true;
            break;
        default :
            noMsg = true;
            break;
    }

    if ( ! noMsg )
    {
        internalErrorMsgBox(buf, "ooDialog Dialog Procedure Error");
    }

    if ( pcpbd != NULL )
    {
        pcpbd->abnormalHalt = true;

        if ( pcpbd->isPropSheetDlg && ! ((pCPropertySheetDialog)(pcpbd->dlgPrivate))->modeless )
        {
            abortPropertySheet((pCPropertySheetDialog)pcpbd->dlgPrivate, hDlg, t);
            return FALSE;
        }

        if ( pcpbd->isPageDlg )
        {
            abortPropertySheetPage((pCPropertySheetPage)pcpbd->dlgPrivate, hDlg, t);
            return FALSE;
        }
        if ( pcpbd->isOwnedDlg )
        {
            abortOwnedDlg(pcpbd, hDlg, t);
            return FALSE;
        }
        ensureFinished(pcpbd, pcpbd->dlgProcContext, TheTrueObj);
    }
    else
    {
        DestroyWindow(hDlg);
    }


    return FALSE;
}


/**
 * Process WM_QUERYNEWPALETTE and WM_PALETTECHANGED messages, called from
 * RexxDlgProc().
 *
 * @param pcpbd
 * @param hDlg
 * @param msg
 * @param wParam
 * @param lParam
 *
 * @return LRESULT
 */
LRESULT paletteMessage(pCPlainBaseDialog pcpbd, HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch ( msg )
    {
        case WM_PALETTECHANGED:
            if ( (HWND)wParam == hDlg )
            {
                // Nothing to do (it was us.)
                return FALSE;
            }
        case WM_QUERYNEWPALETTE:
            if ( pcpbd->colorPalette )
            {
                HDC hDC = GetDC(hDlg);
                SelectPalette(hDC, pcpbd->colorPalette, 0);

                unsigned int ret = RealizePalette(hDC);
                ReleaseDC(hDlg, hDC);

                if ( ret != GDI_ERROR )
                {
                    // Have everything repainted.
                    InvalidateRect(hDlg, NULL, TRUE);
                    return TRUE;
                }
            }
            break;

        default:
            break;
    }

    return FALSE;
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
 *        running from a console window, but the dialog keeps running.  I'm not
 *        sure this is the right way to do it.  It is just what I did during
 *        development.  We should maybe alwasy end the dialog.
 *
 * @note  The call to checkForCondition() after noMsgReturnException() is what
 *        causes the condition to be printed to the screen.  Without that call,
 *        the dialog just quits with no reason printed, so it is needed.
 */
bool msgReplyIsGood(RexxThreadContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr reply, CSTRING methodName, bool clear)
{
    bool haveCondition = checkForCondition(c, clear);

    if ( ! haveCondition && reply == NULLOBJECT )
    {
        noMsgReturnException(c, methodName);
        haveCondition = true;
        checkForCondition(c, clear);
        endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
    }
    return ! haveCondition;
}


/**
 * Checks that there is no pending condition and ends the dialog if there is
 * one.  This is like msgReplyIsGood(), but is used when we do not enforce that
 * the user returns a value from the event handler.
 *
 * @param c
 * @param pcpbd
 * @param methodName
 * @param clear
 *
 * @return True if there is a pending condition and the dialog was ended,
 *         otherwise false.
 */
bool endOnCondition(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING methodName, bool clear)
{
    if ( checkForCondition(c, clear) )
    {
        endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
        return true;
    }
    return false;
}


/**
 * Checks that no condition has been raised, and that reply is either true or
 * false. If not, an exception is raised and the dialog is ended.
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
 *        function.  The return is always TheTrueObj, TheFalseObj, or
 *        NULLOBJECT, so the returned object should never be released.
 */
RexxObjectPtr requiredBooleanReply(RexxThreadContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr reply,
                                   CSTRING method, bool clear)
{
    RexxObjectPtr result = NULLOBJECT;

    if ( msgReplyIsGood(c, pcpbd, reply, method, false) )
    {
        result = convertToTrueOrFalse(c, reply);
        if ( result == NULLOBJECT )
        {
            wrongReplyNotBooleanException(c, method, reply);
            checkForCondition(c, false);
        }
    }

    if ( reply != NULLOBJECT && reply != TheTrueObj && reply != TheFalseObj )
    {
        c->ReleaseLocalReference(reply);
    }

    if ( result == NULLOBJECT )
    {
        endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
    }
    return result;
}

/**
 * Invokes the Rexx dialog's event handling method for a Windows message.
 *
 * The method invocation is done indirectly using startWith().  This allows us
 * to return quickly to the window message processing loop.
 *
 * @param c       Thread context we are operating in.
 * @param pcpbd   The Rexx dialog CSelf whose method will be invoked.
 * @param method  The name of the method being invoked
 * @param args    The argument array for the method being invoked, args can be
 *                NULL.
 *
 * @return The reply type for the Windows dialog procedure, always true to
 *         indicate the message was processed.
 *
 * @remarks  Earlier versions of ooDialog, on the C++ side, constructed a method
 *           invocation string, placed it on a queue, and returned immediately
 *           to the message processing loop.  On the Rexx side, the string was
 *           pulled from the queue and the event hanler method invoked through
 *           interpret.  This meant that the Rexx programmer could never block
 *           the window loop, but also could never reply to any window message.
 *
 *           This function should be used when:
 *
 *           a.) The reply to the window message is ignored anyway and, when
 *           applicable, the Rexx programmer has specified not to wait for a
 *           reply.
 *
 *           b.) To maintain backward compatibility with pre-existing event
 *           connections, so that the Rexx programmer does not inadvertently
 *           block the message loop.
 */
MsgReplyType invokeDispatch(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING method, RexxArrayObject args)
{
    RexxStringObject mth = c->String(method);

    if ( args == NULLOBJECT )
    {
        c->SendMessage1(pcpbd->rexxSelf, "START", mth);
    }
    else
    {
        c->SendMessage2(pcpbd->rexxSelf, "STARTWITH", mth, args);
    }

    c->ReleaseLocalReference(mth);
    return ReplyTrue;
}

/**
 * Invokes the Rexx dialog's event handling method for a Windows message.
 *
 * The method invocation is done directly by sending a message to the method.
 *
 * @param c       Thread context we are operating in.
 * @param obj     The Rexx dialog whose method will be invoked.
 * @param method  The name of the method being invoked
 * @param args    The argument array for the method being invoked
 *
 * @return True for no problems, false if a condition was raised during the
 *         execution of the Rexx method or if no value was returned from the
 *         method.
 *
 * @remarks  Earlier versions of ooDialog, on the C++ side, constructed a method
 *           invocation string, placed it on a queue, and returned immediately
 *           to the message processing loop.  On the Rexx side, the string was
 *           pulled from the queue and the event handler method invoked through
 *           interpret.  This meant that the Rexx programmer could never block
 *           the window loop, but also could never reply to any window message.
 *
 *           This function should be used when:
 *
 *           a.) The reply to the window message is ignored and either the
 *           default behaviour is to wait for the reply, or the Rexx programmer
 *           has specified to wait for a reply.
 */
bool invokeDirect(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING methodName, RexxArrayObject args)
{
    RexxObjectPtr rexxReply;

    if ( args == NULLOBJECT )
    {
        rexxReply = c->SendMessage0(pcpbd->rexxSelf, methodName);
    }
    else
    {
        rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
    }

    bool isGood = msgReplyIsGood(c, pcpbd, rexxReply, methodName, false);
    c->ReleaseLocalReference(rexxReply);

    return isGood;
}

/**
 * Invokes the Rexx dialog's event handling method for a Windows message.
 *
 * The method is invoked directly through SenMessage to the method name and the
 * reply is used to set DWLP_MSGRESULT in the window words of the dialog.
 *
 * @param c       Thread context we are operating in.
 * @param obj     The Rexx dialog whose method will be invoked.
 * @param method  The name of the method being invoked
 * @param args    The argument array for the method being invoked
 *
 * @return True for no problems, otherwise false.  If false is returned a
 *         condition was raised during the execution of the Rexx method or a
 *         condition was raised by us for improper use by the programmer.
 *
 * @remarks  This function should only be called for processing a messaged added
 *           through addUserMessage() where the user specified willReply ==
 *           true.  In this case we use the reply from the user to reply to the
 *           operating system.
 *
 *           Since we can only reply to the operating system, we enforce that
 *           the user has returned a number, and if so, we just use it.
 *           Theoretically, it would seem that it is possible that this will
 *           crash.  In the doc we warn the user of this and it if crashes ...
 *           oh well.
 */
bool invokeDirectUseReturn(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING methodName, RexxArrayObject args)
{
    RexxObjectPtr rexxReply;

    if ( args == NULLOBJECT )
    {
        rexxReply = c->SendMessage0(pcpbd->rexxSelf, methodName);
    }
    else
    {
        rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
    }

    bool isGood = msgReplyIsGood(c, pcpbd, rexxReply, methodName, false);
    if ( isGood )
    {
        intptr_t reply;

        if ( c->Intptr(rexxReply, &reply) )
        {
            setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, reply);
        }
        else
        {
            invalidReturnWholeNumberException(c, methodName, rexxReply, true);

            isGood = false;
            checkForCondition(c, false);
            endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
        }
    }
    c->ReleaseLocalReference(rexxReply);

    return isGood;
}

/**
 * Invokes the Rexx dialog's event handling method for a Windows message.
 *
 * The method invocation is done directly by sending a message to the method.
 *
 * @param c       Thread context we are operating in.
 * @param obj     The Rexx dialog whose method will be invoked.
 * @param method  The name of the method being invoked
 * @param args    The argument array for the method being invoked
 *
 * @return True for no problems, false if a condition was raised during the
 *         execution of the Rexx method.
 *
 * @remarks  This function is exactly like invokeDirect(), except it does not
 *           check that the Rexx method returned a value.  It is used to 'sync'
 *           the Windows message handling loop with Rexx.  For each Windows
 *           message, we won't return from the loop until we get a reply back
 *           from Rexx.
 *
 *           Earlier versions of ooDialog, on the C++ side, constructed a method
 *           invocation string, placed it on a queue, and returned immediately
 *           to the message processing loop.  On the Rexx side, the string was
 *           pulled from the queue and the event handler method invoked through
 *           interpret.  This meant that the Rexx programmer could never block
 *           the window loop, but also could never reply to any window message.
 *
 *           This function should be used when:
 *
 *           a.) The reply to the window message is ignored and either the
 *           default behaviour is to wait for the reply, or the Rexx programmer
 *           has specified to wait for a reply, and checking that the method
 *           actually returned a value should be skipped.
 *
 *           b.) When it has been determined that invoking the method directly
 *           improves things, but it is expected that there are exisiing
 *           programs connecting the event that do not return a value.  A
 *           perfect example is the WM_SIZE message.
 */
bool invokeSync(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING methodName, RexxArrayObject args)
{
    RexxObjectPtr rexxReply;

    if ( args == NULLOBJECT )
    {
        rexxReply = c->SendMessage0(pcpbd->rexxSelf, methodName);
    }
    else
    {
        rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
    }

    bool haveCondition = endOnCondition(c, pcpbd, methodName, false);
    c->ReleaseLocalReference(rexxReply);
    return ! haveCondition;
}

/**
 *  This function sorts out how the event handling method should be invoked by
 *  examining the tag.
 *
 * @param pcpbd
 * @param method
 * @param args
 * @param tag
 *
 * @return MsgReplyType
 */
MsgReplyType genericInvoke(pCPlainBaseDialog pcpbd, CSTRING method, RexxArrayObject args, uint32_t tag)
{
    RexxThreadContext *c = pcpbd->dlgProcContext;

    if ( (tag & TAG_EXTRAMASK) == TAG_REPLYFROMREXX )
    {
        if ( ((tag & TAG_CTRLMASK) == TAG_USERADDED) || ((tag & TAG_USE_RETURN) == TAG_USE_RETURN) )
        {
            invokeDirectUseReturn(c, pcpbd, method, args);
        }
        else
        {
            invokeDirect(c, pcpbd, method, args);
        }
    }
    else if ( (tag & TAG_EXTRAMASK) == TAG_SYNC )
    {
        invokeSync(c, pcpbd, method, args);
    }
    else
    {
        invokeDispatch(c, pcpbd, method, args);
    }

    return ReplyTrue;
}


static MsgReplyType genericNotifyInvoke(pCPlainBaseDialog pcpbd, CSTRING methodName, uint32_t tag,
                                        WPARAM wParam, LPARAM lParam)
{
    RexxThreadContext *c = pcpbd->dlgProcContext;

    oodControl_t  ctrlType   = controlHwnd2controlType(((NMHDR *)lParam)->hwndFrom);
    RexxObjectPtr idFrom     = idFrom2rexxArg(c, lParam);
    RexxObjectPtr hwndFrom   = hwndFrom2rexxArg(c, lParam);
    RexxObjectPtr notifyCode = notifyCode2rexxArg(c, lParam);
    RexxObjectPtr rxCtrl     = controlFrom2rexxArg(pcpbd, lParam, ctrlType);

    RexxArrayObject args = c->ArrayOfFour(idFrom, hwndFrom, notifyCode, rxCtrl);
    genericInvoke(pcpbd, methodName, args, tag);

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(hwndFrom);
    c->ReleaseLocalReference(notifyCode);
    if ( rxCtrl != TheNilObj )
    {
        c->ReleaseLocalReference(rxCtrl);
    }
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}


/* genericCommandInvoke
 *
 * The simplest form of invoking the Rexx method connected to a WM_COMMAND
 * message.  And actually, all WM_COMMAND messages are invoked through this
 * function. As far as I can tell, the return to the operating system is never
 * meaningful.
 *
 * The Rexx method is invoked with two arguments, the WPARAM and LPARAM
 * paramters of the WM_COMMAND message.  There is never any more meaningful
 * information we could send to the event handler (*1).  Since the return is
 * ignored by the OS, the return from the Rexx event handler is never
 * meaningful.
 *
 * Note that for WM_COMMAND messages, lParam is always the window handle of the
 * dialog control, if a control initiated the message.  For menu items and
 * accelerators, it is always 0. So, converting lParam to a pseudo pointer is
 * always the correct thing to do.
 *
 * (1)  We could create the Rexx control object from the hwnd.  But if we decide
 * to do that, we will just add it as the third argument.
 *
 * @remarks  The original ooDialog always sent the numeric value of wParam as
 *           the first arg and the numeric value of lParam as the second arg to
 *           the event handler.  Sending the control ID, the notification code
 *           and the Rexx dialog object constructed from the window handle makes
 *           much more sense.  But those args must be added as the next 3 args
 *           for backwards compatibility.  If thenotification comes from a menu
 *           or accelerator, the Rexx dialog control object should come out as
 *           the .nil object.
 */
static MsgReplyType genericCommandInvoke(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING methodName,
                                         uint32_t tag, WPARAM wParam, LPARAM lParam)
{
    oodControl_t ctrlType = controlHwnd2controlType((HWND)lParam);

    RexxObjectPtr notifyCode = c->UnsignedInt32(HIWORD(wParam));
    RexxObjectPtr id         = c->UnsignedInt32(LOWORD(wParam));
    RexxObjectPtr wp         = c->Uintptr(wParam);
    RexxObjectPtr lp         = pointer2string(c, (void *)lParam);
    RexxObjectPtr ctrlObj    = createControlFromHwnd(pcpbd->dlgProcContext, pcpbd, (HWND)lParam, ctrlType, true);

    RexxArrayObject args = c->ArrayOfFour(wp, lp, id, notifyCode);
    c->ArrayPut(args, ctrlObj, 5);

    genericInvoke(pcpbd, methodName, args, tag);

    c->ReleaseLocalReference(wp);
    c->ReleaseLocalReference(lp);
    c->ReleaseLocalReference(id);
    c->ReleaseLocalReference(notifyCode);
    if ( ctrlObj != TheNilObj )
    {
        c->ReleaseLocalReference(ctrlObj);
    }
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}

inline RexxStringObject mcnViewChange2rexxString(RexxThreadContext *c, uint32_t view)
{
    char * v = "";
    if (      view == MCMV_MONTH )   v = "month";
    else if ( view == MCMV_YEAR )    v = "year";
    else if ( view == MCMV_DECADE )  v = "decade";
    else if ( view == MCMV_CENTURY ) v = "century";
    return c->String(v);
}


inline RexxObjectPtr getToolTipFromLParam(LPARAM lParam)
{
    return (RexxObjectPtr)getWindowPtr(((NMHDR *)lParam)->hwndFrom, GWLP_USERDATA);
}

/**
 * Attempts to get the proper ID for a tool tip tool.
 *
 * @param c
 * @param lParam
 *
 * @return RexxObjectPtr
 *
 * @remarks  The tool tip tool ID can either be a dialog control window handle,
 *           or a unique number.  There is no real way to tell whether the
 *           number in idFrom is a HWND or a number.
 */
RexxObjectPtr getToolIDFromLParam(RexxThreadContext *c, LPARAM lParam)
{
    RexxObjectPtr rxID = TheNilObj;
    UINT_PTR      id   = ((NMHDR *)lParam)->idFrom;

    SetLastError(0);
    if ( IsWindow((HWND)id) && GetDlgCtrlID((HWND)id) != 0 && GetLastError() == 0 )
    {
        rxID = (RexxObjectPtr)getWindowPtr((HWND)id, GWLP_USERDATA);
        if ( rxID == NULLOBJECT )
        {
            rxID = c->Uintptr(id);
        }
    }
    else
    {
        rxID = c->Uintptr(id);
    }
    return rxID;
}


/**
 * Generic handle for the mouse click notifications, NM_CLICK, etc.  Currently
 * this supports ToolBar, and StatusBar.
 *
 * @param pcpbd
 * @param lParam
 * @param methodName
 * @param tag
 * @param code
 *
 * @return MsgReplyType
 *
 * @remarks  For all of these notifications, the user must reply true or false.
 *
 *           For a Status bar, dwItemSpec is the section index, use one-based
 *           index.  For a ToolBar, dwItemSpec is the resurce id of the button
 *           pushed.
 *
 *           For both controls dwItemSpec can be -1.  However, for a status bar
 *           in simple mode, we get 255.  Adding 1 to get the Rexx index gives
 *           us 256.  Which doesn't make sense to me.  256 would be a valid part
 *           index, the last one.  MSDN says that in simple mode there is only 1
 *           part.  So we check for this and use an idex of 1, the first and
 *           only part.
 *
 *           use arg id, nCode, index, pt, ctrl
 */
MsgReplyType genericNmClick(pCPlainBaseDialog pcpbd, LPARAM lParam, CSTRING methodName, oodControl_t ctrlType, uint32_t code)
{
    RexxThreadContext *c    = pcpbd->dlgProcContext;
    NMMOUSE           *nmm  = (NMMOUSE *)lParam;
    MsgReplyType      reply = ReplyTrue;

    RexxObjectPtr idFrom       = idFrom2rexxArg(c, lParam);
    RexxObjectPtr nCode        = notifyCode2rexxArg(c, lParam);
    RexxObjectPtr rxCtrl       = controlFrom2rexxArg(pcpbd, lParam, ctrlType);
    RexxObjectPtr rxPt         = rxNewPoint(c, (PORXPOINT)&(nmm->pt));

    RexxObjectPtr rxSectIndex;
    if ( (intptr_t)nmm->dwItemSpec == -1 )
    {
        rxSectIndex = TheZeroObj;
    }
    else if ( ctrlType == winStatusBar )
    {
        if ( nmm->dwItemSpec == 0xff && isSimple(nmm->hdr.hwndFrom) )
        {
            rxSectIndex = TheOneObj;
        }
        else
        {
            rxSectIndex = c->Uintptr(nmm->dwItemSpec + 1);
        }
    }
    else
    {
        rxSectIndex = c->Uintptr(nmm->dwItemSpec);
    }

    RexxArrayObject args = c->ArrayOfFour(idFrom, nCode, rxSectIndex, rxPt);
    c->ArrayPut(args, rxCtrl, 5);

    RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

    msgReply = requiredBooleanReply(c, pcpbd, msgReply, methodName, false);
    setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, msgReply == TheTrueObj ? TRUE : FALSE);

    if ( msgReply == NULL || msgReply == TheFalseObj )
    {
        reply = ReplyFalse;
    }

    if ( rxSectIndex != TheZeroObj && rxSectIndex != TheOneObj)
    {
        c->ReleaseLocalReference(rxSectIndex);
    }
    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(nCode);
    c->ReleaseLocalReference(rxCtrl);
    c->ReleaseLocalReference(rxPt);
    c->ReleaseLocalReference(args);

    return reply;
}


/**
 * Several controls use the NM_RELEASEDCAPTURE notification.  We standardize the
 * args to the event handler by routing things though this generic function.
 *
 * @param pcpbd
 * @param methodName
 * @param tag
 * @param lParam
 * @param type
 *
 * @return MsgReplyType
 *
 * @remarks  Two arguments are sent to the event handler.  The resource ID of
 *           the control sending the notification and the Rexx control object
 *           itself.
 */
MsgReplyType genericReleasedCapture(pCPlainBaseDialog pcpbd, CSTRING methodName, uint32_t tag,
                                    LPARAM lParam, oodControl_t type)
{
    RexxThreadContext *c = pcpbd->dlgProcContext;

    RexxObjectPtr   idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr   rxRB   = createControlFromHwnd(c, pcpbd, ((NMHDR *)lParam)->hwndFrom, type, true);
    RexxArrayObject args   = c->ArrayOfTwo(idFrom, rxRB);

    genericInvoke(pcpbd, methodName, args, tag);

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(rxRB);
    c->ReleaseLocalReference(args);

    return ReplyTrue;
}

/**
 * Processes button control notifications.
 *
 * Note this is only invoked when TAG_BUTTON is set in the tag.  Button controls
 * currently only have 2 WM_NOTIFY messages.  The other event notifications are
 * sent in WM_COMMAND messages, which are handled in searchCommandTable()
 *
 * @param c
 * @param methodName
 * @param tag
 * @param code  BCN_* code
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 */
MsgReplyType processBCN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    oodControl_t buttonType = controlHwnd2controlType(((NMHDR *)lParam)->hwndFrom);

    RexxObjectPtr   idFrom   = idFrom2rexxArg(c, lParam);
    RexxObjectPtr   rxButton = controlFrom2rexxArg(pcpbd, lParam, buttonType);
    RexxArrayObject args;

    switch ( code )
    {
        case BCN_HOTITEMCHANGE :
        {
            RexxObjectPtr entering = (((NMBCHOTITEM *)lParam)->dwFlags & HICF_ENTERING) ? TheTrueObj : TheFalseObj;

            args = c->ArrayOfThree(idFrom, entering, rxButton);
            genericInvoke(pcpbd, methodName, args, tag);
        }
        break;

        case BCN_DROPDOWN :
        {
            NMBCDROPDOWN  *pDropDown  = (NMBCDROPDOWN*)lParam;
            RexxObjectPtr  buttonRect = rxNewRect(c, (PORXRECT)&pDropDown->rcButton);

            args = c->ArrayOfThree(idFrom, buttonRect, rxButton);
            genericInvoke(pcpbd, methodName, args, tag);

            c->ReleaseLocalReference(buttonRect);
        }
        break;

        default :
            // Can't happen, other button notifications are not routed here.
            break;
    }

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(rxButton);
    c->ReleaseLocalReference(args);
    return ReplyTrue;
}

/**
 * Processes date time picker notification messages.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param code
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @remarks  DTN_FORMAT, DTN_FORMATQUERY, DTN_USERSTRING, and DTN_WMKEYDOWN are
 *           always direct reply, (documented that way,) while with the others,
 *           the user can specify which they want.
 */
MsgReplyType processDTN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    RexxObjectPtr rexxReply = NULLOBJECT;
    RexxObjectPtr args      = NULLOBJECT;
    RexxObjectPtr idFrom    = idFrom2rexxArg(c, lParam);
    RexxObjectPtr hwndFrom  = hwndFrom2rexxArg(c, lParam);
    RexxObjectPtr rxCode    = c->UnsignedInt32(code);
    RexxObjectPtr rxDTP     = createControlFromHwnd(c, pcpbd, ((NMHDR *)lParam)->hwndFrom, winDateTimePicker, true);

    switch ( code )
    {
        case DTN_DATETIMECHANGE:
        {
            LPNMDATETIMECHANGE pChange = (LPNMDATETIMECHANGE)lParam;

            RexxObjectPtr dt;
            RexxObjectPtr valid;

            if ( pChange->dwFlags == GDT_VALID )
            {
                sysTime2dt(c, &(pChange->st), &dt, dtFull);
                valid = TheTrueObj;
            }
            else
            {
                sysTime2dt(c, NULL, &dt, dtNow);
                valid = TheFalseObj;
            }

            RexxArrayObject args = c->ArrayOfFour(dt, valid, idFrom, hwndFrom);
            c->ArrayPut(args, rxCode, 5);
            c->ArrayPut(args, rxDTP, 6);

            genericInvoke(pcpbd, methodName, args, tag);

            c->ReleaseLocalReference(dt);
            break;
        }

        case DTN_FORMAT:
        {
            LPNMDATETIMEFORMAT pFormat = (LPNMDATETIMEFORMAT)lParam;

            RexxObjectPtr format = c->String(pFormat->pszFormat);
            RexxObjectPtr dt;
            sysTime2dt(c, &(pFormat->st), &dt, dtFull);

            RexxArrayObject args = c->ArrayOfFour(c->String(pFormat->pszFormat), dt, idFrom, hwndFrom);
            c->ArrayPut(args, rxCode, 5);
            c->ArrayPut(args, rxDTP, 6);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

            if ( msgReplyIsGood(c, pcpbd, rexxReply, methodName, false) )
            {
                CSTRING display = c->ObjectToStringValue(rexxReply);
                if ( strlen(display) < 64 )
                {
                    strcpy(pFormat->szDisplay, display);
                }
                else
                {
                    stringTooLongException(c, 1, 63, strlen(display));
                    checkForCondition(c, false);
                }
            }

            c->ReleaseLocalReference(dt);
            c->ReleaseLocalReference(format);
            safeLocalRelease(c, rexxReply);

            break;
        }

        case DTN_FORMATQUERY:
        {
            LPNMDATETIMEFORMATQUERY pQuery = (LPNMDATETIMEFORMATQUERY)lParam;

            RexxObjectPtr _size  = rxNewSize(c, 0, 0);
            RexxObjectPtr format = c->String(pQuery->pszFormat);

            RexxArrayObject args = c->ArrayOfFour(format, _size, idFrom, hwndFrom);
            c->ArrayPut(args, rxCode, 5);
            c->ArrayPut(args, rxDTP, 6);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

            if ( msgReplyIsGood(c, pcpbd, rexxReply, methodName, false) )
            {
                PSIZE size = (PSIZE)c->ObjectToCSelf(_size);

                pQuery->szMax.cx = size->cx;
                pQuery->szMax.cy = size->cy;
            }

            c->ReleaseLocalReference(_size);
            c->ReleaseLocalReference(format);
            safeLocalRelease(c, rexxReply);

            break;
        }

        case DTN_USERSTRING:
        {
            LPNMDATETIMESTRING pdts = (LPNMDATETIMESTRING)lParam;

            RexxObjectPtr dt;
            sysTime2dt(c, &(pdts->st), &dt, dtFull);

            RexxStringObject userString = c->String(pdts->pszUserString);

            RexxArrayObject args = c->ArrayOfFour(dt, userString, idFrom, hwndFrom);
            c->ArrayPut(args, rxCode, 5);
            c->ArrayPut(args, rxDTP, 6);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

            if ( msgReplyIsGood(c, pcpbd, rexxReply, methodName, false) )
            {
                if ( c->IsOfType(rexxReply, "DATETIME") )
                {
                    if ( ! dt2sysTime(c, rexxReply, &(pdts->st), dtFull) )
                    {
                        checkForCondition(c, false);
                    }
                    else
                    {
                        pdts->dwFlags = GDT_VALID;
                    }
                }
                else if ( rexxReply == TheNilObj )
                {
                    if ( ! isShowNoneDTP(pdts->nmhdr.hwndFrom) )
                    {
                        wrongReplyMsgException(c, methodName, "can only be .nil if the DTP control has the SHOWNONE style");
                        checkForCondition(c, false);
                    }
                    else
                    {
                        pdts->dwFlags = GDT_NONE;
                    }
                }
            }

            c->ReleaseLocalReference(dt);
            c->ReleaseLocalReference(userString);
            safeLocalRelease(c, rexxReply);

            break;
        }

        case DTN_WMKEYDOWN:
        {
            LPNMDATETIMEWMKEYDOWN pQuery = (LPNMDATETIMEWMKEYDOWN)lParam;

            RexxObjectPtr dt;
            sysTime2dt(c, &(pQuery->st), &dt, dtFull);

            RexxObjectPtr format = c->String(pQuery->pszFormat);
            RexxObjectPtr vKey   = c->Int32(pQuery->nVirtKey);

            RexxArrayObject args = c->ArrayOfFour(format, dt, vKey, idFrom);
            c->ArrayPut(args, hwndFrom, 5);
            c->ArrayPut(args, rxCode, 6);
            c->ArrayPut(args, rxDTP, 7);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

            if ( msgReplyIsGood(c, pcpbd, rexxReply, methodName, false) )
            {
                if ( rexxReply != dt )
                {
                    if ( c->IsOfType(rexxReply, "DATETIME") )
                    {
                        dt2sysTime(c, rexxReply, &(pQuery->st), dtFull);
                    }
                    else
                    {
                        wrongClassReplyException(c, methodName, "DateTime");
                        checkForCondition(c, false);
                    }
                }
            }

            c->ReleaseLocalReference(dt);
            c->ReleaseLocalReference(format);
            c->ReleaseLocalReference(vKey);
            safeLocalRelease(c, rexxReply);

            break;
        }

        case DTN_CLOSEUP:
        case DTN_DROPDOWN:
        case NM_KILLFOCUS:
        case NM_SETFOCUS:
        {
            RexxArrayObject args = c->ArrayOfFour(idFrom, hwndFrom, rxCode, rxDTP);

            genericInvoke(pcpbd, methodName, args, tag);

            break;
        }

        default :
            // Theoretically we can not get here because all date time
            // picker notification codes that have a tag are accounted
            // for.
            break;
    }

    safeLocalRelease(c, rexxReply);
    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(hwndFrom);
    c->ReleaseLocalReference(rxCode);
    c->ReleaseLocalReference(rxDTP);
    safeLocalRelease(c, args);

    return ReplyTrue;
}

MsgReplyType processLVN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    MsgReplyType  msgReply = ReplyFalse;

    switch ( code )
    {
        case LVN_BEGINDRAG :
        case LVN_BEGINRDRAG :
            msgReply = lvnBeginDrag(pcpbd, lParam, methodName, tag, code);
            break;

        case LVN_BEGINSCROLL :
        case LVN_ENDSCROLL   :
            msgReply = lvnBeginEndScroll(pcpbd, lParam, methodName, tag, code);
            break;

        case LVN_BEGINLABELEDIT :
            msgReply = lvnBeginLabelEdit(pcpbd, lParam, methodName, tag);
            break;

        case LVN_COLUMNCLICK :
            msgReply = lvnColumnClick(pcpbd, lParam, methodName, tag);
            break;

        case LVN_ENDLABELEDIT :
            msgReply = lvnEndLabelEdit(pcpbd, lParam, methodName, tag);
            break;

        case LVN_GETINFOTIP :
            msgReply = lvnGetInfoTip(pcpbd, lParam, methodName, tag);
            break;

        case LVN_ITEMCHANGED:
            msgReply = lvnItemChanged(pcpbd, lParam, methodName, tag);
            break;

        case LVN_KEYDOWN :
            msgReply = lvnKeyDown(pcpbd, lParam, methodName, tag);
            break;

        case NM_CLICK:
        case NM_DBLCLK:
            msgReply = lvnNmClick(pcpbd, lParam, methodName, tag, code);
            break;

        default :
            break;
    }

    return msgReply;
}

/**
 * Process month calendar notification messages
 *
 * @param c
 * @param methodName
 * @param tag
 * @param code
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * TODO clean up this function and do the proper release local references.
 */
MsgReplyType processMCN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    RexxObjectPtr rexxReply;
    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr hwndFrom = hwndFrom2rexxArg(c, lParam);
    bool          expectReply = (tag & TAG_REPLYFROMREXX) == TAG_REPLYFROMREXX;

    switch ( code )
    {
        case MCN_GETDAYSTATE :
        {
            LPNMDAYSTATE pDayState = (LPNMDAYSTATE)lParam;

            RexxObjectPtr dt = NULLOBJECT;
            sysTime2dt(c, &(pDayState->stStart), &dt, dtDate);

            RexxArrayObject args = c->ArrayOfFour(dt, c->Int32(pDayState->cDayState), idFrom, hwndFrom);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

            if ( msgReplyIsGood(c, pcpbd, rexxReply, methodName, false) )
            {
                if ( c->IsOfType(rexxReply, "BUFFER") )
                {
                    pDayState->prgDayState = (MONTHDAYSTATE *)c->BufferData((RexxBufferObject)rexxReply);
                }
                else
                {
                    wrongClassReplyException(c, methodName, "DayStateBuffer");
                }
            }

            break;
        }

        case MCN_SELECT :
        case MCN_SELCHANGE :
        {
            LPNMSELCHANGE pSelChange = (LPNMSELCHANGE)lParam;

            RexxObjectPtr dtStart;
            sysTime2dt(c, &(pSelChange->stSelStart), &dtStart, dtDate);

            RexxObjectPtr dtEnd;
            if ( isZeroDate(&(pSelChange->stSelEnd)) )
            {
                dtEnd = dtStart;
            }
            else
            {
                sysTime2dt(c, &(pSelChange->stSelEnd), &dtEnd, dtDate);
            }

            RexxArrayObject args = c->ArrayOfFour(dtStart, dtEnd, idFrom, hwndFrom);

            if ( expectReply )
            {
                invokeDirect(c, pcpbd, methodName, args);
            }
            else
            {
                invokeDispatch(c, pcpbd, methodName, args);
            }

            break;
        }

        case MCN_VIEWCHANGE :
        {
            LPNMVIEWCHANGE pViewChange = (LPNMVIEWCHANGE)lParam;

            RexxStringObject newView = mcnViewChange2rexxString(c, pViewChange->dwNewView);
            RexxStringObject oldView = mcnViewChange2rexxString(c, pViewChange->dwOldView);

            RexxArrayObject args = c->ArrayOfFour(newView, oldView, idFrom, hwndFrom);

            if ( expectReply )
            {
                invokeDirect(c, pcpbd, methodName, args);
            }
            else
            {
                invokeDispatch(c, pcpbd, methodName, args);
            }

            break;
        }

        case NM_RELEASEDCAPTURE :
        {
            RexxArrayObject args = c->ArrayOfTwo(idFrom, hwndFrom);
            genericInvoke(pcpbd, methodName, args, tag);

            c->ReleaseLocalReference(args);
            break;
        }

        default :
            // Theoretically we can not get here because all month
            // calendar notification codes that have a tag are
            // accounted for.
            break;
    }

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(hwndFrom);

    return ReplyTrue;
}

/**
 * Handles the connected ReBar event notifications.
 *
 * The tag code must have included the TAG_REBAR flag for this function to be
 * invoked.  Since the ReBar control is newer that ooDialog 4.2.0, all ReBar
 * event connections have that tag.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param code
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @remarks
 *
 */
MsgReplyType processRBN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code,
                        LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    switch ( code )
    {
        case NM_NCHITTEST :
            return rbnNcHitTest(pcpbd, methodName, tag, lParam);

        case NM_RELEASEDCAPTURE :
            return genericReleasedCapture(pcpbd, methodName, tag, lParam, winReBar);

        case RBN_AUTOBREAK :
            return rbnAutobreak(pcpbd, methodName, tag, lParam);

        default :
            break;
    }

    // This should never happen, we can't get here.
    return ReplyTrue;
}

/**
 * Handles the connected StatusBar event notifications.
 *
 * The tag code must have included the TAG_STATUSBAR flag for this function to
 * be invoked.  Since the StatusBar control is newer than ooDialog 4.2.0, all
 * StatusBar event connections have that tag.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param code
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @remarks
 *
 */
MsgReplyType processSBN(pCPlainBaseDialog pcpbd, CSTRING methodName, uint32_t tag, uint32_t code, LPARAM lParam)
{
    switch ( code )
    {
        case NM_CLICK :
        case NM_DBLCLK :
        case NM_RCLICK :
        case NM_RDBLCLK :
            return genericNmClick(pcpbd, lParam, methodName, winStatusBar, code);

        case SBN_SIMPLEMODECHANGE :
            return sbnSimpleModeChange(pcpbd, methodName, tag, lParam);

        default :
            break;
    }

    // This should never happen, we can't get here.
    return ReplyFalse;
}

/**
 * Processes tab control notifications.
 *
 * Note this is only invoked when TAG_TAB is set in the tag.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param code
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 */
MsgReplyType processTCN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    RexxObjectPtr   idFrom   = idFrom2rexxArg(c, lParam);
    RexxObjectPtr   hwndFrom = hwndFrom2rexxArg(c, lParam);
    RexxObjectPtr   rxTab    = controlFrom2rexxArg(pcpbd, lParam, winTab);
    RexxArrayObject args     = c->ArrayOfThree(idFrom, hwndFrom, rxTab);

    bool willReply = (tag & TAG_EXTRAMASK) == TAG_REPLYFROMREXX;

    switch ( code )
    {
        case TCN_SELCHANGING :
        {
            if ( willReply )
            {
                // The Rexx programmer returns .true, changing the tab is okay, or .false do not change tabs.
                RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

                msgReply = requiredBooleanReply(c, pcpbd, msgReply, methodName, false);
                if ( msgReply == TheTrueObj || msgReply == TheFalseObj )
                {
                    // Return true to prevent the change.
                    setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, msgReply == TheTrueObj ? FALSE : TRUE);
                }
            }
            else
            {
                genericInvoke(pcpbd, methodName, args, tag);
            }
            break;
        }

        case TCN_KEYDOWN :
        {
            uint32_t      vKey  = (uint32_t)((NMTCKEYDOWN *)lParam)->wVKey;
            RexxObjectPtr rxKey = c->UnsignedInt32(vKey);

            c->ArrayPut(args, rxKey, 2);
            genericInvoke(pcpbd, methodName, args, tag);

            c->ReleaseLocalReference(rxKey);
            break;
        }

        case TCN_SELCHANGE :
        {
            genericInvoke(pcpbd, methodName, args, tag);
            break;
        }

        default :
            // Can't happen, other tab notifications are not routed here.
            break;
    }

    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(hwndFrom);
    c->ReleaseLocalReference(rxTab);
    c->ReleaseLocalReference(args);
    return ReplyTrue;
}


/**
 * Handles the connected ToolBar event notifications.
 *
 * The tag code must have included the TAG_TOOLBAR flag for this function to be
 * invoked.  Since the ToolBar control is newer than ooDialog 4.2.0, all ToolBar
 * event connections have that tag.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param code
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @remarks
 *
 */
MsgReplyType processTBN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code,
                        LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    switch ( code )
    {

        case NM_CLICK :
        case NM_DBLCLK :
        case NM_RCLICK :
        case NM_RDBLCLK :
            return genericNmClick(pcpbd, lParam, methodName, winToolBar, code);

        case TBN_BEGINADJUST :
            return tbnSimple(c, methodName, tag, lParam, pcpbd);

        case TBN_CUSTHELP :
            return tbnSimple(c, methodName, tag, lParam, pcpbd);

        case TBN_DELETINGBUTTON :
            return tbnDeletingButton(c, methodName, tag, lParam, pcpbd);

        case TBN_ENDADJUST :
            return tbnSimple(c, methodName, tag, lParam, pcpbd);

        case TBN_GETBUTTONINFO :
            return tbnGetButtonInfo(c, methodName, tag, lParam, pcpbd);

        case TBN_INITCUSTOMIZE :
            return tbnInitCustomize(c, methodName, tag, lParam, pcpbd);

        case TBN_QUERYDELETE :
            return tbnQuery(c, methodName, tag, lParam, pcpbd, code);

        case TBN_QUERYINSERT :
            return tbnQuery(c, methodName, tag, lParam, pcpbd, code);

        case NM_RELEASEDCAPTURE :
            return genericReleasedCapture(pcpbd, methodName, tag, lParam, winToolBar);

        case TBN_TOOLBARCHANGE :
            return tbnSimple(c, methodName, tag, lParam, pcpbd);

        default :
            break;
    }

    // This should never happen, we can't get here.
    return ReplyTrue;
}

/**
 * Process tool tip notification messages.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param code
 * @param wParam
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @remarks  Testing has shown that wParam, what the MSDN doc labels as idTT, is
 *           the uId field of the TOOLINFO struct. In addition, wParam and
 *           ((NMHDR *)lParam)->idFrom seem to always have the same value.
 *
 *           However, for the TTN_LINKCLICK notification, this value is always
 *           0.  (Seems to be always 0, and this sort of matches what MSDN
 *           says.)
 *
 *           The TTN_NEEDTEXT notification presents a problem.  For strings
 *           longer than 80 characters, MSDN says you need to point the lpszText
 *           array to your own private buffer when the text used in the ToolTip
 *           exceeds 80 TCHARS in length.  If we alloc the memory, when do we
 *           free it?  Currently, we use a static buffer in the tool tip table
 *           entry.  We are assuming that only 1 tool tip text can display at
 *           any one time.  When we get TTN_NEEDTEXT, we copy the text returned
 *           from the user into this static buffer.  Then we do not need to
 *           worry about allocing and freeing the memory.
 */
MsgReplyType processTTN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code, WPARAM wParam, LPARAM lParam,
                        pCPlainBaseDialog pcpbd)
{
    RexxObjectPtr rexxReply = NULLOBJECT;
    bool          willReply = (tag & TAG_REPLYFROMREXX) == TAG_REPLYFROMREXX;

    RexxObjectPtr   rxToolTip = getToolTipFromLParam(lParam);
    RexxObjectPtr   rxToolID  = getToolIDFromLParam(c, lParam);

    switch ( code )
    {
        case TTN_LINKCLICK :
        {
            RexxArrayObject args = c->ArrayOfTwo(rxToolID, rxToolTip);
            if ( willReply )
            {
                invokeDirect(c, pcpbd, methodName, args);
            }
            else
            {
                invokeDispatch(c, pcpbd, methodName, args);
            }

            c->ReleaseLocalReference(args);
            break;
        }

        case TTN_NEEDTEXT :
        {
            LPNMTTDISPINFO nmtdi = (LPNMTTDISPINFO)lParam;

            RexxDirectoryObject info = c->NewDirectory();

            RexxObjectPtr    userData = nmtdi->lParam == NULL ? TheNilObj : (RexxObjectPtr)nmtdi->lParam;
            RexxStringObject flags    = ttdiFlags2keyword(c, nmtdi->uFlags);

            c->DirectoryPut(info, c->NullString(), "TEXT");
            c->DirectoryPut(info, userData, "USERDATA");
            c->DirectoryPut(info, flags, "FLAGS");

            RexxArrayObject args = c->ArrayOfThree(rxToolID, rxToolTip, info);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
            rexxReply = requiredBooleanReply(c, pcpbd, rexxReply, methodName, false);
            if ( rexxReply == NULLOBJECT )
            {
                return ReplyFalse;
            }

            RexxObjectPtr _text = c->DirectoryAt(info, "TEXT");
            CSTRING       text  = c->ObjectToStringValue(_text);
            size_t        len   = strlen(text);

            if ( len > MAX_TOOLINFO_TEXT_LENGTH )
            {
                stringTooLongException(c, 1, MAX_TOOLINFO_TEXT_LENGTH, len);
                checkForCondition(c, false);
                endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
                return ReplyFalse;
            }

            pCDialogControl pcdc = controlToCSelf(c, rxToolTip);
            strcpy(pcdc->toolTipEntry->textBuf, text);

            nmtdi->lpszText = pcdc->toolTipEntry->textBuf;

            if ( rexxReply == TheTrueObj )
            {
                nmtdi->uFlags |= TTF_DI_SETITEM;
            }
            c->ReleaseLocalReference(_text);

            c->ReleaseLocalReference(flags);
            c->ReleaseLocalReference(info);
            c->ReleaseLocalReference(args);
            break;
        }

        case TTN_POP :
        {
            RexxArrayObject args = c->ArrayOfTwo(rxToolID, rxToolTip);

            if ( willReply )
            {
                invokeDirect(c, pcpbd, methodName, args);
            }
            else
            {
                invokeDispatch(c, pcpbd, methodName, args);
            }

            c->ReleaseLocalReference(args);

            break;
        }

        case TTN_SHOW :
        {
            RexxArrayObject args = c->ArrayOfTwo(rxToolID, rxToolTip);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
            rexxReply = requiredBooleanReply(c, pcpbd, rexxReply, methodName, false);
            if ( rexxReply == NULLOBJECT )
            {
                return ReplyFalse;
            }
            if ( rexxReply == TheTrueObj )
            {
                setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, TRUE);
            }

            c->ReleaseLocalReference(args);

            break;
        }

        default :
            // Theoretically we can not get here because all tool tip
            // notification codes that have a tag are accounted for.
            break;
    }

    c->ReleaseLocalReference(rxToolTip);
    c->ReleaseLocalReference(rxToolID);

    return ReplyTrue;
}

/**
 * Handles the connected tree-vien event notifications.
 *
 * The tag code must have included the TAG_TREEVIEW flag for this function to be
 * invoked.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param code
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @remarks
 *
 */
MsgReplyType processTVN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    switch ( code )
    {
        case TVN_BEGINDRAG :
        case TVN_BEGINRDRAG :
            return tvnBeginDrag(c, methodName, tag, lParam, pcpbd, code);

        case TVN_BEGINLABELEDIT :
            return tvnBeginLabelEdit(c, methodName, tag, lParam, pcpbd);

        case TVN_DELETEITEM :
            return tvnDeleteItem(c, methodName, tag, lParam, pcpbd);

        case TVN_ENDLABELEDIT :
            return tvnEndLabelEdit(c, methodName, tag, lParam, pcpbd);

        case TVN_GETINFOTIP :
            return tvnGetInfoTip(c, methodName, tag, lParam, pcpbd);

        case TVN_KEYDOWN :
            return tvnKeyDown(c, methodName, tag, lParam, pcpbd);

        case TVN_ITEMEXPANDED :
        case TVN_ITEMEXPANDING :
            return tvnItemExpand(c, methodName, tag, lParam, pcpbd, code);

        case TVN_SELCHANGED :
        case TVN_SELCHANGING :
            return tvnSelChange(c, methodName, tag, lParam, pcpbd, code);

        default :
            break;
    }

    // This should never happen, we can't get here.
    return ReplyFalse;
}

/**
 * Handles the WM_NOTIFY messages for an up down control that the Rexx
 * programmer has connected.
 *
 * There is currently only one notify message for an up-down; UDN_DELTAPOS, so
 * the processing is straight forward.
 *
 * Arguments sent to Rexx:
 *
 *   use arg pos, delta, id, hwnd, upDown
 *
 * The event handler must reply with
 *
 * @param c
 * @param methodName
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 */
MsgReplyType processUDN(RexxThreadContext *c, CSTRING methodName, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    MsgReplyType winReply = ReplyTrue;
    LPNMUPDOWN   pUPD     = (LPNMUPDOWN)lParam;

    RexxObjectPtr  pos    = c->Int32(pUPD->iPos);
    RexxObjectPtr  delta  = c->Int32(pUPD->iDelta);
    RexxObjectPtr  idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr  rxHwnd = hwndFrom2rexxArg(c, lParam);
    RexxObjectPtr  rxUpd   = createControlFromHwnd(c, pcpbd, pUPD->hdr.hwndFrom, winUpDown, true);

    RexxArrayObject args = c->ArrayOfFour(pos, delta, idFrom, rxHwnd);
    c->ArrayPut(args, rxUpd, 5);

    RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

    if ( msgReplyIsGood(c, pcpbd, msgReply, methodName, false) )
    {
        if ( c->IsOfType(msgReply, "BUFFER") )
        {
            PDELTAPOSREPLY pdpr = (PDELTAPOSREPLY)c->BufferData((RexxBufferObject)msgReply);
            if ( pdpr->change )
            {
                if ( pdpr->cancel )
                {
                    setWindowPtr(GetParent(pUPD->hdr.hwndFrom), DWLP_MSGRESULT, 1);
                }
                else
                {
                    pUPD->iDelta = pdpr->newDelta;
                }
            }
        }
        else
        {
            wrongClassReplyException(c, methodName, "PositionChangeReplyBuffer");
            checkForCondition(c, false);
            endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
            winReply = ReplyFalse;
        }
    }
    else
    {
        winReply = ReplyFalse;
    }

    c->ReleaseLocalReference(pos);
    c->ReleaseLocalReference(delta);
    c->ReleaseLocalReference(idFrom);
    c->ReleaseLocalReference(rxHwnd);
    c->ReleaseLocalReference(rxUpd);
    c->ReleaseLocalReference(msgReply);
    c->ReleaseLocalReference(args);

    return winReply;
}

/**
 * Handles the NM_CUSTOMDRAW event notification message.
 *
 * @param c
 * @param methodName
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @notes  Currently we are only handling the list-view and tree-view custom
 *         draw, and only the simple case.  Everything else is just ignored.
 *         Custom draw for more controls will be added over time.
 *
 *         The simple case may be the only practical case.  But, the intent is
 *         to add complete support for custom draw, with the anticipation that
 *         it may be too slow in ooDialog.
 */
MsgReplyType processCustomDraw(RexxThreadContext *c, CSTRING methodName, uint32_t tag, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    LPARAM reply = CDRF_DODEFAULT;

    if ( ! pcpbd->badIDs )
    {
        switch ( tag & TAG_FLAGMASK )
        {
            case TAG_CD_LISTVIEW :
                if ( (tag & TAG_EXTRAMASK) == TAG_CD_SIMPLE )
                {
                    return lvSimpleCustomDraw(c, methodName, lParam, pcpbd);
                }

                break;

            case TAG_CD_TREEVIEW :
                if ( (tag & TAG_EXTRAMASK) == TAG_CD_SIMPLE )
                {
                    return tvSimpleCustomDraw(c, methodName, lParam, pcpbd);
                }

                break;

            default :
                break;
        }
    }

    setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, (LPARAM)reply);
    return ReplyTrue;
}

/**
 * Searches through the notify (WM_NOTIFY) message table for a table entry that
 * matches the parameters of a WM_NOTIFY message.  If found the matching method
 * in the Rexx dialog is invoked.
 *
 * @param wParam  The WPARAM parameter of the WM_NOTIFY message.
 * @param lParam  The LPARAM parameter of the WM_NOTIFY message.
 * @param pcpbd   The PlainBaseDialog CSelf for the dialog the WM_NOTIFY was
 *                directed to.
 *
 * @return The result of the search.  Either no entry was found, an entry was
 *         found reply true in the dialog procedure, or an entry was found reply
 *         false in the dialog procedure.
 *
 * @remarks We search through the message table looking for an entry where,
 *          after applying the filters to the WPARAM and LPARAM arguments, we
 *          have a match.  When we find a match, we check for a "tag" in the
 *          table entry.  If there is a tag, we can narrow the processing down.
 *          This concept of a "tag" is an addition to the original ooDialog
 *          code.
 *
 *          When we have matched a message in the table, but don't match a tag,
 *          the original ooDialog code would have invoked the the Rexx method
 *          with two arguments wParam and lParam turned into numbers. This is
 *          not very useful. The first arg would be idFrom and the second a
 *          meaningless number, a pointer to the NMHDR struct.
 *
 *          However, with the newer C++ native API, and since we know this is a
 *          WM_NOTIFY message, we can invoke the method with 4 arguments: the
 *          control id that sent the notification, the hwnd of the control that
 *          sent the notification, the notification code itself, and the Rexx
 *          control object that represents the control that sent the
 *          notification.
 *
 *          This can actually be useful ... but I'm not sure about backwards
 *          compatibility with that change.  But, it seems like an okay change.
 *          The first arg sent will be the same.  The second arg will appear to
 *          be the same, a seemingly meaningless number.  Any existing code
 *          could not have done anything with that number, new code could use it
 *          since it is a valid window handle.  Existing code would not have
 *          used args 3 and 4, they didn't exist.  New code can use them
 *          effectively.
 *
 *          MSDN says, or at least implies, that *all* WM_NOTIFY messages are
 *          from dialog controls.
 */
MsgReplyType searchNotifyTable(WPARAM wParam, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    MESSAGETABLEENTRY *m = pcpbd->enCSelf->notifyMsgs;
    if ( m == NULL )
    {
        return ContinueProcessing;
    }

    uint32_t code     = ((NMHDR *)lParam)->code;
    HWND     hwndFrom = ((NMHDR *)lParam)->hwndFrom;

    for ( register size_t i = 0; i < pcpbd->enCSelf->nmNextIndex; i++ )
    {
        if ( (((wParam & m[i].wpFilter) == m[i].wParam) && isCodeMatch(m, code, i)) ||
             (isTTN(code) && isCodeMatch(m, code, i) && (((WPARAM)hwndFrom & m[i].wpFilter) == m[i].wParam)) )
        {
            RexxThreadContext *c = pcpbd->dlgProcContext;

            switch ( m[i].tag & TAG_CTRLMASK )
            {
                case TAG_NOTHING :
                    break;

                case TAG_BUTTON :
                    return processBCN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);

                case TAG_CUSTOMDRAW :
                    return processCustomDraw(c, m[i].rexxMethod, m[i].tag, lParam, pcpbd);

                case TAG_DATETIMEPICKER :
                    return processDTN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);

                case TAG_LISTVIEW :
                {
                    MsgReplyType ret = processLVN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);
                    if ( ret == ContinueSearching )
                    {
                        continue;
                    }
                    return ret;
                }

                case TAG_MONTHCALENDAR :
                    return processMCN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);

                case TAG_REBAR :
                    return processRBN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);

                case TAG_STATUSBAR :
                    return processSBN(pcpbd, m[i].rexxMethod, m[i].tag, code, lParam);

                case TAG_TAB :
                    return processTCN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);

                case TAG_TOOLBAR :
                    return processTBN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);

                case TAG_TOOLTIP :
                    return processTTN(c, m[i].rexxMethod, m[i].tag, code, wParam, lParam, pcpbd);

                case TAG_TREEVIEW :
                    return processTVN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);

                case TAG_UPDOWN :
                    return processUDN(c, m[i].rexxMethod, lParam, pcpbd);

                default :
                    break;
            }

            // We matched an entry in the table, but not a tag.  Invoke the Rexx
            // method with some useful arguments.
            return genericNotifyInvoke(pcpbd, m[i].rexxMethod, m[i].tag, wParam, lParam);
        }
    }

    return ReplyFalse;
}


/**
 * Searches through the command (WM_COMMAND) message table for a table entry
 * that matches the parameters of a WM_COMMAND.
 *
 * @param wParam  The WPARAM parameter of the WM_COMMAND message.
 * @param lParam  The LPARAM parameter of the WM_COMMAND message.
 * @param pcpbd   The PlainBaseDialog CSelf for the dialog the WM_COMMAND was
 *                directed to.
 *
 * @return The result of the search.  Either no entry was found, an entry was
 *         found reply true in the dialog procedure, or an entry was found reply
 *         false in the dialog procedure.
 *
 * @remarks  The command message table is always allocated, so we shouldn't need
 *           to check that commandMsgs is null.  But, it might be possible that
 *           it was freed by delDialog() and we don't as yet have adequate
 *           checks for that.
 *
 *           At this time, there is no special processing for any WM_COMMAND
 *           message.  So, if we find a match in the command message table there
 *           is not much to do.
 *
 *           Note that for WM_COMMAND messages, lParam is always the window
 *           handle of the dialog control, if a control initiated the message.
 *           For menu items and accelerators, it is always 0. So, converting to
 *           a pseudo pointer is always the correct thing to do.
 *
 *           Currently we just pass the wParam and lParam values as numbers to
 *           the event handler.  It might be nice to try and convert the window
 *           handle to the dialog control object and add that to the args.  This
 *           might be especially useful for the addUserMessage() cases.
 */
MsgReplyType searchCommandTable(WPARAM wParam, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    MESSAGETABLEENTRY *m = pcpbd->enCSelf->commandMsgs;
    if ( m == NULL )
    {
        return ContinueProcessing;
    }

    size_t tableSize = pcpbd->enCSelf->cmNextIndex;
    register size_t i = 0;

    for ( i = 0; i < tableSize; i++ )
    {
        if ( ((wParam & m[i].wpFilter) == m[i].wParam) && ((lParam & m[i].lpFilter) == m[i].lParam) )
        {
            return genericCommandInvoke(pcpbd->dlgProcContext, pcpbd, m[i].rexxMethod, m[i].tag, wParam, lParam);
        }
    }
    return ContinueProcessing;
}


/**
 * Searches through the miscellaneous (anything not WM_COMMAND or WM_NOTIFY)
 * message table for a table entry that matches the message and its parameters.
 * If a match is found, the Rexx dialog method is invoked.
 *
 * @param msg     The windows message.
 * @param wParam  The WPARAM parameter of the message.
 * @param lParam  The LPARAM parameter of the message.
 * @param pcpbd   The PlainBaseDialog CSelf for the dialog the message was
 *                directed to.
 *
 * @return The result of the search.  Either no entry was found, an entry was
 *         found reply true in the dialog procedure, or an entry was found reply
 *         false in the dialog procedure.
 */
MsgReplyType searchMiscTable(uint32_t msg, WPARAM wParam, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    MESSAGETABLEENTRY *m = pcpbd->enCSelf->miscMsgs;
    if ( m == NULL )
    {
        return ContinueProcessing;
    }

    size_t tableSize = pcpbd->enCSelf->mmNextIndex;
    for ( register size_t i = 0; i < tableSize; i++ )
    {
        if ( (msg & m[i].msgFilter) == m[i].msg && (wParam & m[i].wpFilter) == m[i].wParam && (lParam & m[i].lpFilter) == m[i].lParam )
        {
            RexxThreadContext *c = pcpbd->dlgProcContext;
            RexxArrayObject    args;
            char              *method = m[i].rexxMethod;
            uint32_t           tag    = m[i].tag;

            bool willReply = (tag & TAG_EXTRAMASK) == TAG_REPLYFROMREXX;
            bool sync      = (tag & TAG_EXTRAMASK) == TAG_SYNC;

            switch ( tag & TAG_CTRLMASK )
            {
                case TAG_NOTHING :
                    break;

                case TAG_DIALOG :
                    switch ( tag & TAG_FLAGMASK )
                    {
                        case TAG_HELP :
                        {
                            LPHELPINFO phi = (LPHELPINFO)lParam;

                            RexxObjectPtr ctrlID   = c->Int32(phi->iCtrlId);
                            RexxObjectPtr cntxType = c->String(phi->iContextType == HELPINFO_WINDOW ? "WINDOW" : "MENU");
                            RexxObjectPtr x        = c->Int32(phi->MousePos.x);
                            RexxObjectPtr y        = c->Int32(phi->MousePos.y);
                            RexxObjectPtr cntxID   = c->Uintptr(phi->dwContextId);

                            RexxObjectPtr sendingObj = TheNilObj;
                            bool          release    = false;
                            if ( phi->iContextType == HELPINFO_WINDOW )
                            {
                                HWND         hwnd     = (HWND)phi->hItemHandle;
                                oodControl_t ctrlType = controlHwnd2controlType(hwnd);
                                if ( ctrlType == winDialog )
                                {
                                    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)getWindowPtr(hwnd, GWLP_USERDATA);
                                    sendingObj = (pcpbd == NULL ? TheNilObj : pcpbd->rexxSelf);
                                }
                                else
                                {
                                    sendingObj = createControlFromHwnd(c, pcpbd, hwnd, ctrlType, true);
                                    release = true;
                                }
                            }

                            args = c->ArrayOfFour(ctrlID, cntxType, x, y);
                            c->ArrayPut(args, cntxID, 5);
                            c->ArrayPut(args, sendingObj, 6);

                            genericInvoke(pcpbd, method, args, tag);

                            c->ReleaseLocalReference(ctrlID);
                            c->ReleaseLocalReference(cntxType);
                            c->ReleaseLocalReference(x);
                            c->ReleaseLocalReference(y);
                            c->ReleaseLocalReference(cntxID);
                            if ( release )
                            {
                                c->ReleaseLocalReference(sendingObj);
                            }
                            c->ReleaseLocalReference(args);

                            return ReplyTrue;
                        }
                        break;

                        default :
                            break;
                    }
                    break;

                case TAG_MENU :
                    return processMenuMsg(pcpbd, msg, wParam, lParam, method, tag);

                case TAG_MOUSE :
                    return processMouseMsg(c, method, tag, msg, wParam, lParam, pcpbd);

                case TAG_USERADDED :
                {
                    /* The user added this message through addUserMessage().
                     * Since we have no idea what the message might be, we have
                     * to just pass wParam and lParam to the method.  We also
                     * simply honor the user's willReplly set up.
                     */
                    RexxObjectPtr wp = c->Uintptr(wParam);
                    RexxObjectPtr lp = c->Intptr(lParam);

                    args = c->ArrayOfTwo(wp, lp);
                    genericInvoke(pcpbd, method, args, tag);

                    c->ReleaseLocalReference(wp);
                    c->ReleaseLocalReference(lp);
                    c->ReleaseLocalReference(args);

                    return ReplyTrue;
                }

                default :
                    break;
            }

            if ( msg == WM_HSCROLL || msg == WM_VSCROLL )
            {
                RexxObjectPtr wp = c->Uintptr(wParam);
                RexxObjectPtr h  = pointer2string(c, (void *)lParam);

                oodControl_t  ctrlType = controlHwnd2controlType((HWND)lParam);
                RexxObjectPtr sb       = createControlFromHwnd(pcpbd->dlgProcContext, pcpbd, (HWND)lParam, ctrlType, true);

                args = c->ArrayOfThree(wp, h, sb);
                genericInvoke(pcpbd, method, args, tag);

                c->ReleaseLocalReference(wp);
                c->ReleaseLocalReference(h);
                if ( sb != TheNilObj )
                {
                    c->ReleaseLocalReference(sb);
                }
                c->ReleaseLocalReference(args);

                return ReplyTrue;
            }
            else if ( msg == WM_ACTIVATE)
            {
                // The user must reply.
                RexxObjectPtr isMinimized = HIWORD(wParam) ? TheTrueObj : TheFalseObj;

                RexxStringObject flag = c->NullString();
                switch ( LOWORD(wParam) )
                {
                    case WA_ACTIVE :
                        flag = c->String("ACTIVE");
                        break;
                    case WA_CLICKACTIVE :
                        flag = c->String("CLICKACTIVE");
                        break;
                    case WA_INACTIVE :
                        flag = c->String("INACTIVE");
                        break;
                }

                RexxStringObject hwnd = pointer2string(c, (void *)lParam);
                RexxStringObject hFocus = pointer2string(c, (void *)GetFocus());

                MsgReplyType reply = ReplyFalse;
                args = c->ArrayOfFour(flag, hwnd, hFocus, isMinimized);

                RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, method, args);

                msgReply = requiredBooleanReply(c, pcpbd, msgReply, method, false);
                if ( msgReply == TheTrueObj )
                {
                    reply = ReplyTrue;
                }

                c->ReleaseLocalReference(isMinimized);
                c->ReleaseLocalReference(flag);
                c->ReleaseLocalReference(hwnd);
                c->ReleaseLocalReference(hFocus);
                c->ReleaseLocalReference(args);

                return reply;
            }
            else if ( msg == WM_EXITSIZEMOVE )
            {
                // We handle this individually because MSDN documents that 0
                // should be returned if processed, the opposite of the usual.
                // There are no arguments to the event handler.
                genericInvoke(pcpbd, method, NULLOBJECT, tag);
                return ReplyFalse;
            }
            else if ( msg == WM_SIZING )
            {
                /* The user must reply.  Args to ooRexx: The sizing RECT,
                 * WMSZ_xx keyword.
                 */
                MsgReplyType reply = ReplyFalse;
                PRECT        wRect = (PRECT)lParam;

                RexxStringObject wmsz = wmsz2string(c, wParam);
                RexxObjectPtr    rect = rxNewRect(c, (PORXRECT)wRect);

                args = c->ArrayOfTwo(rect, wmsz);

                RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, method, args);

                msgReply = requiredBooleanReply(c, pcpbd, msgReply, method, false);
                if ( msgReply == TheTrueObj )
                {
                    PRECT r = (PRECT)c->ObjectToCSelf(rect);
                    wRect->top    = r->top;
                    wRect->left   = r->left;
                    wRect->bottom = r->bottom;
                    wRect->right  = r->right;
                    reply = ReplyTrue;
                }

                c->ReleaseLocalReference(wmsz);
                c->ReleaseLocalReference(rect);
                c->ReleaseLocalReference(args);

                return reply;
            }
            else if ( msg == WM_DRAWITEM )
            {
                LPDRAWITEMSTRUCT lpDI = (LPDRAWITEMSTRUCT)lParam;

                /* use arg ctrlID, lp, rexxObj, itemID, flags, hdc, rect, itemData */

                RexxObjectPtr ctrlID = c->UnsignedInt32((uint32_t)wParam);
                RexxObjectPtr lp     = c->Intptr(lParam);

                RexxObjectPtr flags   = od2keywords(c, lpDI);
                RexxObjectPtr hdc     = pointer2string(c, (void *)lpDI->hDC);
                RexxObjectPtr rexxObj = createControlFromHwnd(c, pcpbd, lpDI->hwndItem, odt2oodt(lpDI->CtlType), true);
                RexxObjectPtr rect    = rxNewRect(c, (PORXRECT)&(lpDI->rcItem));

                RexxObjectPtr itemID;
                if ( lpDI->CtlType == ODT_MENU )
                {
                    itemID = c->UnsignedInt32(lpDI->itemID);
                }
                else
                {
                    itemID = c->UnsignedInt32(lpDI->itemID + 1);
                }

                RexxObjectPtr itemData;
                if ( lpDI->itemData == 0 )
                {
                    itemData = TheNilObj;
                }
                else
                {
                    itemData = (RexxObjectPtr)lpDI->itemData;
                }

                args = c->ArrayOfFour(ctrlID, lp, rexxObj, itemID);
                c->ArrayPut(args, flags, 5);
                c->ArrayPut(args, hdc, 6);
                c->ArrayPut(args, rect, 7);
                c->ArrayPut(args, itemData, 8);

                // Adding TAG_USE_RETURN causes genericInvoke() to return the
                // user's return value in the dialog's DWLP_MSGRESULT window
                // word.
                //
                // Originally I thought we needed this for WM_DRAWITEM, but
                // returning false or true seems to have no effect.
                tag = willReply ? tag | TAG_USE_RETURN : tag;
                genericInvoke(pcpbd, method, args, tag);

                c->ReleaseLocalReference(ctrlID);
                c->ReleaseLocalReference(lp);
                c->ReleaseLocalReference(rexxObj);
                c->ReleaseLocalReference(itemID);
                c->ReleaseLocalReference(flags);
                c->ReleaseLocalReference(hdc);
                c->ReleaseLocalReference(rect);
                c->ReleaseLocalReference(args);

                return ReplyTrue;
            }

            // lParam might not come out right ...
            RexxObjectPtr wp = c->Uintptr(wParam);
            RexxObjectPtr lp = c->Intptr(lParam);

            args = c->ArrayOfTwo(wp, lp);

            tag = willReply ? tag | TAG_USE_RETURN : tag;
            genericInvoke(pcpbd, method, args, tag);

            c->ReleaseLocalReference(wp);
            c->ReleaseLocalReference(lp);
            c->ReleaseLocalReference(args);

            return ReplyTrue;
        }
    }
    return ContinueProcessing;
}


MsgReplyType searchMessageTables(ULONG message, WPARAM param, LPARAM lparam, pCPlainBaseDialog pcpbd)
{
    switch ( message )
    {
        case WM_COMMAND : return searchCommandTable(param, lparam, pcpbd);
        case WM_NOTIFY  : return searchNotifyTable(param, lparam, pcpbd);
        default         : return searchMiscTable(message, param, lparam, pcpbd);
    }
}

/**
 * Adds an event connection for a command (WM_COMMAND) message to the table.
 *
 * @param pcen
 * @param c
 * @param wParam
 * @param wpFilter
 * @param lParam
 * @param lpFilter
 * @param method
 * @param tag
 *
 * @return True on success, false for a memory allocation error.
 *
 * @remarks  The command message table is allocated during the plain base dialog
 *           init process, so we should not need to check that it has been
 *           allocated. However, there are some errors the programmer can make
 *           that can cause us, rarely, to get here with commandMsgs not
 *           allocated.  Better to check and raise a condition for this
 *           situation.
 *
 *           Caller must ensure that 'method' is not an empty string and that
 *           winMsg, wParam, lParam are not all 0.
 *
 *           If the message table is full we reallocate a table double in size
 *           through LocalReAlloc().  Without the LMEM_MOVABLE flag, the realloc
 *           can fail.  Note that even though the LMEM_MOVABLE flag is used, if
 *           the memory object being reallocated is fixed, the returned memory
 *           object is still fixed.  LocalLock() does not need to be used.
 *
 *           LMEM_MOVEABLE:  If uBytes is nonzero, enables the system to move
 *           the reallocated block to a new location without changing the
 *           movable or fixed attribute of the memory object. If the object is
 *           fixed, the handle returned may be different from the handle
 *           specified in the hMem parameter.
 */
bool addCommandMessage(pCEventNotification pcen, RexxMethodContext *c, WPARAM wParam, ULONG_PTR wpFilter,
                       LPARAM lParam, ULONG_PTR lpFilter, CSTRING method, uint32_t tag)
{
    if ( pcen == NULL || pcen->commandMsgs == NULL )
    {
        baseClassInitializationException(c);
        return false;
    }

    size_t index = pcen->cmNextIndex;
    if ( index >= pcen->cmSize )
    {
        HLOCAL temp = LocalReAlloc(pcen->commandMsgs, sizeof(MESSAGETABLEENTRY) * pcen->cmSize * 2, LMEM_ZEROINIT | LMEM_MOVEABLE);
        if ( temp == NULL )
        {
            MessageBox(0, "Command message connections have exceeded the maximum\n"
                          "number of allocated table entries, and the table could\n"
                          "not be expanded.\n\n"
                          "No more command message connections can be added.\n",
                       "Error", MB_OK | MB_ICONHAND);
            return false;
        }

        pcen->cmSize *= 2;
        pcen->commandMsgs = (MESSAGETABLEENTRY *)temp;
    }

    pcen->commandMsgs[index].rexxMethod = (char *)LocalAlloc(LMEM_FIXED, strlen(method) + 1);
    if ( pcen->commandMsgs[index].rexxMethod == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }
    strcpy(pcen->commandMsgs[index].rexxMethod, method);

    pcen->commandMsgs[index].msg = WM_COMMAND;
    pcen->commandMsgs[index].msgFilter = 0xFFFFFFFF;
    pcen->commandMsgs[index].wParam = wParam;
    pcen->commandMsgs[index].wpFilter = wpFilter;
    pcen->commandMsgs[index].lParam = lParam;
    pcen->commandMsgs[index].lpFilter = lpFilter;
    pcen->commandMsgs[index].tag = tag;

    pcen->cmNextIndex++;
    return true;
}


/**
 * Adds an event connection for a notification (WM_NOTIFY) message to the
 * table.
 *
 * @param pcen
 * @param c
 * @param wParam
 * @param wpFilter
 * @param lParam
 * @param lpFilter
 * @param method
 * @param tag
 *
 * @return True on success, false if the message table is full, or for a memory
 *         allocation error.
 *
 * @remarks  Caller must ensure that 'method' is not an empty string and that
 *           winMsg, wParam, lParam are not all 0.
 *
 *           See remarks in addCommandMessages() for some relevant information.
 */
bool addNotifyMessage(pCEventNotification pcen, RexxMethodContext *c, WPARAM wParam, ULONG_PTR wpFilter,
                      LPARAM lParam, ULONG_PTR lpFilter, CSTRING method, uint32_t tag)
{
    if ( pcen->notifyMsgs == NULL )
    {
        pcen->notifyMsgs = (MESSAGETABLEENTRY *)LocalAlloc(LPTR, sizeof(MESSAGETABLEENTRY) * DEF_MAX_NOTIFY_MSGS);
        if ( pcen->notifyMsgs == NULL )
        {
            outOfMemoryException(c->threadContext);
            return false;
        }
        pcen->nmNextIndex = 0;
        pcen->nmSize = DEF_MAX_NOTIFY_MSGS;
    }

    size_t index = pcen->nmNextIndex;

    if ( index >= pcen->nmSize )
    {
        HLOCAL temp = LocalReAlloc(pcen->notifyMsgs, sizeof(MESSAGETABLEENTRY) * pcen->nmSize * 2, LMEM_ZEROINIT | LMEM_MOVEABLE);
        if ( temp == NULL )
        {
            MessageBox(0, "Notify message connections have exceeded the maximum\n"
                          "number of allocated table entries, and the table could\n"
                          "not be expanded.\n\n"
                          "No more notify message connections can be added.\n",
                       "Error", MB_OK | MB_ICONHAND);
            return false;
        }

        pcen->nmSize *= 2;
        pcen->notifyMsgs = (MESSAGETABLEENTRY *)temp;
    }

    pcen->notifyMsgs[index].rexxMethod = (char *)LocalAlloc(LMEM_FIXED, strlen(method) + 1);
    if ( pcen->notifyMsgs[index].rexxMethod == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }
    strcpy(pcen->notifyMsgs[index].rexxMethod, method);

    pcen->notifyMsgs[index].msg = WM_NOTIFY;
    pcen->notifyMsgs[index].msgFilter = 0xFFFFFFFF;
    pcen->notifyMsgs[index].wParam = wParam;
    pcen->notifyMsgs[index].wpFilter = wpFilter;
    pcen->notifyMsgs[index].lParam = lParam;
    pcen->notifyMsgs[index].lpFilter = lpFilter;
    pcen->notifyMsgs[index].tag = tag;

    pcen->nmNextIndex++;
    return true;
}


/**
 * Adds an event connection for any Windows message that is not a WM_COMMAND or
 * WM_NOTIFY message to the table.
 *
 * @param pcen
 * @param winMsg
 * @param wmFilter
 * @param wParam
 * @param wpFilter
 * @param lParam
 * @param lpFilter
 * @param method
 * @param tag
 *
 * @return True on success, false if the message table is full, or for a memory
 *         allocation error.
 *
 * @remarks  Caller must ensure that 'method' is not an empty string and that
 *           winMsg, wParam, lParam are not all 0.
 *
 *           See remarks in addCommandMessages() for some relevant information.
 *
 *           Note that the message filtering code was designed when 32 bit
 *           Windows was all that was available, and for some time ooDialog was
 *           32 bit only.  With the advent of 64-bit ooDialog, the filtering may
 *           be incorrect.  Take for instance the case where filter is on a
 *           window handle.  All the ooDialog code would set the filter to be
 *           0xFFFFFFFF.  But, a 64 bit window handle could be 0x00021cdffffffff
 *           and the filter would fail.
 *
 *           In C / C++ code the proper thing to do is to use UINTPTR_MAX rather
 *           than 0xFFFFFFFF for a filter, in cases where the thing being
 *           filtered could be a 64 bit value.  In most all cases, this has been
 *           fixed, but it is something to watch out for.
 */
bool addMiscMessage(pCEventNotification pcen, RexxMethodContext *c, uint32_t winMsg, uint32_t wmFilter,
                    WPARAM wParam, ULONG_PTR wpFilter, LPARAM lParam, ULONG_PTR lpFilter,
                    CSTRING method, uint32_t tag)
{
    if ( pcen->miscMsgs == NULL )
    {
        pcen->miscMsgs = (MESSAGETABLEENTRY *)LocalAlloc(LPTR, sizeof(MESSAGETABLEENTRY) * DEF_MAX_MISC_MSGS);
        if ( pcen->miscMsgs == NULL )
        {
            outOfMemoryException(c->threadContext);
            return false;
        }
        pcen->mmNextIndex = 0;
        pcen->mmSize = DEF_MAX_MISC_MSGS;
    }

    size_t index = pcen->mmNextIndex;

    if ( index >= pcen->mmSize )
    {
        HLOCAL temp = LocalReAlloc(pcen->miscMsgs, sizeof(MESSAGETABLEENTRY) * pcen->mmSize * 2, LMEM_ZEROINIT | LMEM_MOVEABLE);
        if ( temp == NULL )
        {
            MessageBox(0, "Miscellaneous message connections have exceeded the maximum\n"
                          "number of allocated table entries, and the table could\n"
                          "not be expanded.\n\n"
                          "No more miscellaneous message connections can be added.\n",
                       "Error", MB_OK | MB_ICONHAND);
            return false;
        }

        pcen->mmSize *= 2;
        pcen->miscMsgs = (MESSAGETABLEENTRY *)temp;
    }

    pcen->miscMsgs[index].rexxMethod = (char *)LocalAlloc(LMEM_FIXED, strlen(method) + 1);
    if ( pcen->miscMsgs[index].rexxMethod == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }
    strcpy(pcen->miscMsgs[index].rexxMethod, method);

    pcen->miscMsgs[index].msg = winMsg;
    pcen->miscMsgs[index].msgFilter = wmFilter;
    pcen->miscMsgs[index].wParam = wParam;
    pcen->miscMsgs[index].wpFilter = wpFilter;
    pcen->miscMsgs[index].lParam = lParam;
    pcen->miscMsgs[index].lpFilter = lpFilter;
    pcen->miscMsgs[index].tag = tag;

    pcen->mmNextIndex++;
    return true;
}


/**
 * The command message table is always initialized when a new dialog object is
 * instantiated.  The Ok, Cancel, and Help commands are always trapped and
 * default event handlers are defined in the PlainBaseDialog class.
 *
 * However, this causes problems in property sheet page dialogs. When the user
 * hits enter when a page has the focus, the default Ok event handler runs. The
 * default Ok handler invokes validate() with the wrong arguments for
 * PropertySheetPage::validate().
 *
 * We don't want that, so for page and contol dialogs we don't automatically
 * trap the command messages. There are other ways to fix this, but this seems
 * best.  The Ok, Cancel, and Help commands should not really be trapped for
 * either page or control dialogs.
 *
 * @param c
 * @param pcen
 * @param pcpbd
 *
 * @return bool
 */
bool initCommandMessagesTable(RexxMethodContext *c, pCEventNotification pcen, pCPlainBaseDialog pcpbd)
{
    pcen->commandMsgs = (MESSAGETABLEENTRY *)LocalAlloc(LPTR, sizeof(MESSAGETABLEENTRY) * DEF_MAX_COMMAND_MSGS);
    if ( ! pcen->commandMsgs )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }
    pcen->cmSize = DEF_MAX_COMMAND_MSGS;
    pcen->cmNextIndex = 0;

    // We do not want to trap these events for page or control dialogs.
    if ( ! (pcpbd->isPageDlg || pcpbd->isControlDlg) )
    {
        if ( ! addCommandMessage(pcen, c, IDOK, UINTPTR_MAX, 0, 0, "OK", TAG_NOTHING) )
        {
            return false;
        }
        if ( ! addCommandMessage(pcen, c, IDCANCEL, UINTPTR_MAX, 0, 0, "Cancel", TAG_NOTHING) )
        {
            return false;
        }
        if ( ! addCommandMessage(pcen, c, IDHELP, UINTPTR_MAX, 0, 0, "Help", TAG_NOTHING) )
        {
            return false;
        }
    }

    return true;
}

bool initEventNotification(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr self, pCEventNotification *ppCEN)
{
    RexxBufferObject obj = c->NewBuffer(sizeof(CEventNotification));
    if ( obj == NULLOBJECT )
    {
        return false;
    }

    pCEventNotification pcen = (pCEventNotification)c->BufferData(obj);
    memset(pcen, 0, sizeof(pCEventNotification));

    pcen->magic     = EVENTNOTIFICATION_MAGIC;
    pcen->rexxSelf  = self;
    pcen->pDlgCSelf = pcpbd;

    if ( ! initCommandMessagesTable(c, pcen, pcpbd) )
    {
        return false;
    }

    // This can not fail, init_eventNotification only fails if called from Rexx.
    c->SendMessage1(self, "INIT_EVENTNOTIFICATION", obj);
    *ppCEN = pcen;
    return true;
}


/**
 *  Methods for the .EventNotification mixin class.
 */
#define EVENTNOTIFICATION_CLASS       "EventNotification"


#define DTPN_KEYWORDS                 "CloseUp, DateTimeChange, DropDown, FormatQuery, Format, KillFocus, SetFocus, UserString, or KeyDown"
#define MCN_KEYWORDS                  "GetDayState, Released, SelChange, Select, or ViewChange"
#define SBN_KEYWORDS                  "Click DblClk RClick RDblClk SimpleModeChange"
#define RBN_KEYWORDS                  "NcHitTest, ReleasedCapture, AutoBreak, AutoSize, BeginDrag, ChevronPushed, ChildSize, DeletedBand, DeletingBand, EndDrag, GetObject, HeightChange, LayoutChanged, MinMax, or SplitterDrag"
#define TBN_KEYWORDS                  "one of the documented ToolBar notification keywords"
#define TTN_KEYWORDS                  "LinkClick, NeedText, Pop, or Show"

// Fake notification code to use for the list box WM_VKEYTOITEM event.
#define LBN_VKEYTOITEM  8

/**
 * Converts the _willReply argument in a connect even method to the proper tag.
 *
 * Orignally, the will reply argument was boolean, true for will reply and false
 * otherwise.  Since then the 'sync' tag was added allowing the user to specify
 * that the interpreter should wait for the event handler, but not require that
 * the user actually return a value.
 *
 * But the true / false value had already been documented for a number of the
 * methods.  To maintain backwards compatibility, and to keep things consistent,
 * we allow true / false or the keyword "sync" for the _willreply argument.
 *
 * In addition, the default for all new controls is true and the default for all
 * old controls must be false.
 *
 * @param context      Method context we are operating in, used argumentOmitted
 * @param _willReply   The _willReply argument
 * @param defaultTrue  True, if the default for the method is true.
 *
 * @return The tag that should be used, TAG_NOTHING, TAG_REPLYFROMREXX, or
 *         TAG_SYNC.
 */
static uint32_t _willReplyToTag(RexxMethodContext *context, RexxObjectPtr _willReply, bool defaultTrue, size_t argPos)
{
    if ( argumentOmitted(argPos) )
    {
        return defaultTrue ? TAG_REPLYFROMREXX : TAG_NOTHING;
    }

    logical_t willReply;
    if ( context->Logical(_willReply, &willReply) )
    {
        return willReply ? TAG_REPLYFROMREXX : TAG_NOTHING;
    }

    CSTRING keyword = context->ObjectToStringValue(_willReply);
    if ( StrCmpI(keyword, "SYNC") == 0 )
    {
        return TAG_SYNC;
    }

    wrongArgValueException(context->threadContext, argPos, ".true, .false, or 'SYNC'", _willReply);
    return TAG_INVALID;
}

/**
 * Convert a button notification code to a method name.
 */
inline CSTRING bcn2name(uint32_t bcn)
{
    switch ( bcn )
    {
        case BN_CLICKED        : return "onClicked";
        case BN_PAINT          : return "onPaint";
        case BN_HILITE         : return "onHiLite";
        case BN_UNHILITE       : return "onUnHiLite";
        case BN_DISABLE        : return "onDisable";
        case BN_DBLCLK         : return "onDblClk";
        case BN_SETFOCUS       : return "onGotFocus";
        case BN_KILLFOCUS      : return "onLostFocus";
        case BCN_HOTITEMCHANGE : return "onHotItem";
        case BCN_DROPDOWN      : return "onDropDown";
    }
    return "onBCN";
}

/**
 * Convert a keyword to the proper button notification code.
 *
 * We know the keyword arg position is 2.  We can not raise an exception here.
 * For buttons, most of the notification codes are BN_xx and are sent in a
 * WM_COMMAND message.  Only two, at this time, are sent in a WM_NOTIFY message.
 */
static bool keyword2bcn(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t bcn;

    if ( StrCmpI(keyword,      "CLICKED")   == 0 ) bcn = BN_CLICKED;
    else if ( StrCmpI(keyword, "PAINT")     == 0 ) bcn = BN_PAINT;
    else if ( StrCmpI(keyword, "HILITE")    == 0 ) bcn = BN_HILITE;
    else if ( StrCmpI(keyword, "UNHILITE")  == 0 ) bcn = BN_UNHILITE;
    else if ( StrCmpI(keyword, "DISABLE")   == 0 ) bcn = BN_DISABLE;
    else if ( StrCmpI(keyword, "DBLCLK")    == 0 ) bcn = BN_DBLCLK;
    else if ( StrCmpI(keyword, "GOTFOCUS")  == 0 ) bcn = BN_SETFOCUS;
    else if ( StrCmpI(keyword, "LOSTFOCUS") == 0 ) bcn = BN_KILLFOCUS;
    else if ( StrCmpI(keyword, "HOTITEM")   == 0 )
    {
        if ( ! requiredComCtl32Version(c, COMCTL32_6_0, "connecting the HOTITEM event") )
        {
            return false;
        }
        bcn = BCN_HOTITEMCHANGE;
    }
    else if ( StrCmpI(keyword, "DROPDOWN")   == 0 )
    {
        if ( ! requiredOS(c, Vista_OS, "connecting the DROPDOWN event", "Vista") )
        {
            return false;
        }
        bcn = BCN_DROPDOWN;
    }
    else
    {
        return false;
    }
    *flag = bcn;
    return true;
}

/**
 * Convert a combobox notification code to a method name.
 */
inline CSTRING cbn2name(uint32_t cbn)
{
    switch ( cbn )
    {
        case CBN_ERRSPACE      : return "onErrSpace"     ;
        case CBN_SELCHANGE     : return "onSelChange"    ;
        case CBN_DBLCLK        : return "onDblClk"       ;
        case CBN_SETFOCUS      : return "onGotFocus"     ;
        case CBN_KILLFOCUS     : return "onLostFocus"    ;
        case CBN_EDITCHANGE    : return "onChange"       ;
        case CBN_EDITUPDATE    : return "onUpdate"       ;
        case CBN_DROPDOWN      : return "onDropDown"     ;
        case CBN_CLOSEUP       : return "onCloseUp"      ;
        case CBN_SELENDOK      : return "onSelEndOk"     ;
        case CBN_SELENDCANCEL  : return "onSelEndCancel" ;
    }
    return "onCBN";
}

/**
 * Convert a keyword to the proper combobo notification code.
 *
 * We can not raise an exception here, this is original ooDialog implementation.
 */
static bool keyword2cbn(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t cbn;

    if ( StrCmpI(keyword,      "ERRSPACE"     ) == 0 ) cbn = CBN_ERRSPACE     ;
    else if ( StrCmpI(keyword, "SELCHANGE"    ) == 0 ) cbn = CBN_SELCHANGE    ;
    else if ( StrCmpI(keyword, "DBLCLK"       ) == 0 ) cbn = CBN_DBLCLK       ;
    else if ( StrCmpI(keyword, "GOTFOCUS"     ) == 0 ) cbn = CBN_SETFOCUS     ;
    else if ( StrCmpI(keyword, "LOSTFOCUS"    ) == 0 ) cbn = CBN_KILLFOCUS    ;
    else if ( StrCmpI(keyword, "CHANGE"       ) == 0 ) cbn = CBN_EDITCHANGE   ;
    else if ( StrCmpI(keyword, "UPDATE"       ) == 0 ) cbn = CBN_EDITUPDATE   ;
    else if ( StrCmpI(keyword, "DROPDOWN"     ) == 0 ) cbn = CBN_DROPDOWN     ;
    else if ( StrCmpI(keyword, "CLOSEUP"      ) == 0 ) cbn = CBN_CLOSEUP      ;
    else if ( StrCmpI(keyword, "SELENDOK"     ) == 0 ) cbn = CBN_SELENDOK     ;
    else if ( StrCmpI(keyword, "SELENDCANCEL" ) == 0 ) cbn = CBN_SELENDCANCEL ;
    else
    {
        return false;
    }
    *flag = cbn;
    return true;
}

/**
 * Convert a date time picker notification code to a method name.
 */
inline CSTRING dtpn2name(uint32_t dtpn)
{
    switch ( dtpn )
    {
        case DTN_CLOSEUP        : return "onCloseUp";
        case DTN_DATETIMECHANGE : return "onDateTimeChange";
        case DTN_DROPDOWN       : return "onDropDown";
        case DTN_FORMAT         : return "onFormat";
        case DTN_FORMATQUERY    : return "onFormatQuery";
        case DTN_USERSTRING     : return "onUserString";
        case DTN_WMKEYDOWN      : return "onWmKeyDown";
        case NM_KILLFOCUS       : return "onKillFocus";
        case NM_SETFOCUS        : return "onSetFocus";
    }
    return "onDTPN";
}

/**
 * Convert a keyword to the proper date time picker notification code.
 *
 * We know the keyword arg position is 2.  The DateTimePicker control is post
 * ooRexx 4.0.1 so we raise an exception on error.
 */
static bool keyword2dtpn(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t dtpn;

    if ( StrCmpI(keyword,      "CLOSEUP")        == 0 ) dtpn = DTN_CLOSEUP;
    else if ( StrCmpI(keyword, "DATETIMECHANGE") == 0 ) dtpn = DTN_DATETIMECHANGE;
    else if ( StrCmpI(keyword, "DROPDOWN")       == 0 ) dtpn = DTN_DROPDOWN;
    else if ( StrCmpI(keyword, "FORMATQUERY")    == 0 ) dtpn = DTN_FORMATQUERY;
    else if ( StrCmpI(keyword, "FORMAT")         == 0 ) dtpn = DTN_FORMAT;
    else if ( StrCmpI(keyword, "KILLFOCUS")      == 0 ) dtpn = NM_KILLFOCUS;
    else if ( StrCmpI(keyword, "SETFOCUS")       == 0 ) dtpn = NM_SETFOCUS;
    else if ( StrCmpI(keyword, "USERSTRING")     == 0 ) dtpn = DTN_USERSTRING;
    else if ( StrCmpI(keyword, "KEYDOWN")        == 0 ) dtpn = DTN_WMKEYDOWN;
    else
    {
        wrongArgValueException(c->threadContext, 2, DTPN_KEYWORDS, keyword);
        return false;
    }
    *flag = dtpn;
    return true;
}

/**
 * Determines if the reply to a date time picker notification code has any
 * meaning, or if it is ignored.  For the notifications listed, the Rexx dialog
 * object method is always invoked directly, i.e., the user must always reply.
 */
inline bool isMustReplyDtpn(uint32_t dtpn)
{
    return (dtpn == DTN_FORMAT) || (dtpn == DTN_FORMATQUERY) ||
           (dtpn == DTN_USERSTRING) || (dtpn == DTN_WMKEYDOWN);
}

/**
 * Convert an edit notification code to a method name.
 */
inline CSTRING en2name(uint32_t en)
{
    switch ( en )
    {
        case EN_SETFOCUS       : return "onGotFocus"  ;
        case EN_KILLFOCUS      : return "onLostFocus" ;
        case EN_CHANGE         : return "onChange"    ;
        case EN_UPDATE         : return "onUpDate"    ;
        case EN_ERRSPACE       : return "onErrSpace"  ;
        case EN_MAXTEXT        : return "onMaxText "  ;
        case EN_HSCROLL        : return "onHScroll"   ;
        case EN_VSCROLL        : return "onVScroll"   ;
    }
    return "onEN";
}

/**
 * Convert a keyword to the proper edit notification code.
 *
 * We can not raise an exception here, this is original ooDialog implementation.
 */
static bool keyword2en(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t en;

    if ( StrCmpI(keyword,      "GOTFOCUS"   ) == 0 ) en = EN_SETFOCUS    ;
    else if ( StrCmpI(keyword, "LOSTFOCUS"  ) == 0 ) en = EN_KILLFOCUS   ;
    else if ( StrCmpI(keyword, "CHANGE"     ) == 0 ) en = EN_CHANGE      ;
    else if ( StrCmpI(keyword, "UPDATE"     ) == 0 ) en = EN_UPDATE      ;
    else if ( StrCmpI(keyword, "ERRSPACE"   ) == 0 ) en = EN_ERRSPACE    ;
    else if ( StrCmpI(keyword, "MAXTEXT"    ) == 0 ) en = EN_MAXTEXT     ;
    else if ( StrCmpI(keyword, "HSCROLL"    ) == 0 ) en = EN_HSCROLL     ;
    else if ( StrCmpI(keyword, "VSCROLL"    ) == 0 ) en = EN_VSCROLL     ;
    else
    {
        return false;
    }
    *flag = en;
    return true;
}

/**
 * Convert a list box notification code to a method name.
 */
inline CSTRING lbn2name(uint32_t lbn)
{
    switch ( lbn )
    {
        case LBN_ERRSPACE      : return "onErrSpace"    ;
        case LBN_SELCHANGE     : return "onSelChange"   ;
        case LBN_DBLCLK        : return "onDblClk"      ;
        case LBN_SELCANCEL     : return "onSelCancel"   ;
        case LBN_SETFOCUS      : return "onGotFocus"    ;
        case LBN_KILLFOCUS     : return "onLostFocus"   ;
        case LBN_VKEYTOITEM    : return "onKeyDown"     ;
    }
    return "onLBN";
}

/**
 * Convert a keyword to the proper list box notification code.
 *
 * We can not raise an exception here, this is original ooDialog implementation.
 */
static bool keyword2lbn(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t lbn;

    if ( StrCmpI(keyword,      "ERRSPACE"     ) == 0 ) lbn = LBN_ERRSPACE     ;
    else if ( StrCmpI(keyword, "SELCHANGE"    ) == 0 ) lbn = LBN_SELCHANGE    ;
    else if ( StrCmpI(keyword, "DBLCLK"       ) == 0 ) lbn = LBN_DBLCLK       ;
    else if ( StrCmpI(keyword, "SELCANCEL"    ) == 0 ) lbn = LBN_SELCANCEL    ;
    else if ( StrCmpI(keyword, "GOTFOCUS"     ) == 0 ) lbn = LBN_SETFOCUS     ;
    else if ( StrCmpI(keyword, "LOSTFOCUS"    ) == 0 ) lbn = LBN_KILLFOCUS    ;
    else if ( StrCmpI(keyword, "KEYDOWN"      ) == 0 ) lbn = LBN_VKEYTOITEM   ;
    else
    {
        return false;
    }
    *flag = lbn;
    return true;
}

/**
 * Convert a list view notification code and tag to a method name.
 */
inline CSTRING lvn2name(uint32_t lvn, uint32_t tag)
{
    switch ( lvn )
    {
        case LVN_ITEMCHANGING   : return "onChanging";
        case LVN_INSERTITEM     : return "onInserted";
        case LVN_DELETEITEM     : return "onDelete";
        case LVN_DELETEALLITEMS : return "onDeleteAll";
        case LVN_BEGINLABELEDIT : return "onBeginedit";
        case LVN_ENDLABELEDIT   : return "onEndedit";
        case LVN_COLUMNCLICK    : return "onColumnclick";
        case LVN_BEGINDRAG      : return "onBegindrag";
        case LVN_BEGINRDRAG     : return "onBeginrdrag";
        case LVN_BEGINSCROLL    : return "onBeginScroll";
        case LVN_ENDSCROLL      : return "onEndScroll";
        case LVN_ITEMACTIVATE   : return "onActivate";
        case LVN_GETINFOTIP     : return "onGetInfoTip";
        case NM_CLICK           : return "onClick";
        case LVN_KEYDOWN :
            if ( (tag & TAG_FLAGMASK) == TAG_PRESERVE_OLD )
            {
                return "onKeyDown";
            }
            else
            {
                return "onKeydownEx";
            }

        case LVN_ITEMCHANGED :
            tag &= ~(TAG_REPLYFROMREXX | TAG_LISTVIEW | TAG_STATECHANGED);

            switch ( tag )
            {
                case TAG_NOTHING :
                    return "onChanged";

                case TAG_CHECKBOXCHANGED :
                    return "onCheckBoxChanged";

                case TAG_SELECTCHANGED :
                    return "onSelectChanged";

                case TAG_FOCUSCHANGED :
                    return "onFocusChanged";

                case TAG_SELECTCHANGED | TAG_FOCUSCHANGED :
                    return "onSelectFocus";
            }
    }
    return "onLVN";
}

/**
 * Convert a keyword to the proper list view notification code.
 *
 *
 */
static bool keyword2lvn(RexxMethodContext *c, CSTRING keyword, uint32_t *code, uint32_t *tag, bool *isDefEdit,
                        RexxObjectPtr willReply)
{
    uint32_t lvn = 0;

    *isDefEdit = false;
    *tag = 0;

    if ( StrCmpI(keyword,      "ACTIVATE")    == 0 ) lvn = LVN_ITEMACTIVATE;
    else if ( StrCmpI(keyword, "CHANGED")     == 0 ) lvn = LVN_ITEMCHANGED;
    else if ( StrCmpI(keyword, "CHANGING")    == 0 ) lvn = LVN_ITEMCHANGING;
    else if ( StrCmpI(keyword, "DELETE")      == 0 ) lvn = LVN_DELETEITEM;
    else if ( StrCmpI(keyword, "DELETEALL")   == 0 ) lvn = LVN_DELETEALLITEMS;
    else if ( StrCmpI(keyword, "INSERTED")    == 0 ) lvn = LVN_INSERTITEM;
    else if ( StrCmpI(keyword, "DEFAULTEDIT") == 0 )
    {
        *isDefEdit = true;
        *tag = TAG_LISTVIEW | TAG_PRESERVE_OLD;
    }
    else if ( StrCmpI(keyword, "BEGINDRAG")   == 0 )
    {
        lvn = LVN_BEGINDRAG;
        *tag = TAG_LISTVIEW | TAG_PRESERVE_OLD;
    }
    else if ( StrCmpI(keyword, "BEGINEDIT") == 0 )
    {
        lvn = LVN_BEGINLABELEDIT;
        *tag = TAG_LISTVIEW;
    }
    else if ( StrCmpI(keyword, "BEGINRDRAG")  == 0 )
    {
        lvn = LVN_BEGINRDRAG;
        *tag = TAG_LISTVIEW | TAG_PRESERVE_OLD;
    }
    else if ( StrCmpI(keyword, "BEGINSCROLL") == 0 )
    {
        lvn = LVN_BEGINSCROLL;
        *tag = TAG_LISTVIEW;
    }
    else if ( StrCmpI(keyword, "CHECKBOXCHANGED") == 0 )
    {
        lvn = LVN_ITEMCHANGED;
        *tag = TAG_LISTVIEW | TAG_STATECHANGED | TAG_CHECKBOXCHANGED;
    }
    else if ( StrCmpI(keyword, "CLICK") == 0 )
    {
        lvn = NM_CLICK;
        *tag = TAG_LISTVIEW;
    }
    else if ( StrCmpI(keyword, "COLUMNCLICK") == 0 )
    {
        lvn = LVN_COLUMNCLICK;
        *tag = TAG_LISTVIEW;
    }
    else if ( StrCmpI(keyword, "DBLCLK") == 0 )
    {
        lvn = NM_DBLCLK;
        *tag = TAG_LISTVIEW;
    }
    else if ( StrCmpI(keyword, "ENDEDIT") == 0 )
    {
         lvn = LVN_ENDLABELEDIT;
         *tag = TAG_LISTVIEW;
    }
    else if ( StrCmpI(keyword, "ENDSCROLL") == 0 )
    {
        lvn = LVN_ENDSCROLL;
        *tag = TAG_LISTVIEW;
    }
    else if ( StrCmpI(keyword, "FOCUSCHANGED") == 0 )
    {
        lvn = LVN_ITEMCHANGED;
        *tag = TAG_LISTVIEW | TAG_STATECHANGED | TAG_FOCUSCHANGED;
    }
    else if ( StrCmpI(keyword, "GETINFOTIP") == 0 )
    {
        lvn = LVN_GETINFOTIP;
        *tag = TAG_LISTVIEW | TAG_REPLYFROMREXX;
    }
    else if ( StrCmpI(keyword, "KEYDOWN") == 0 )
    {
        lvn = LVN_KEYDOWN;
        *tag = TAG_LISTVIEW | TAG_PRESERVE_OLD;
    }
    else if ( StrCmpI(keyword, "KEYDOWNEX") == 0 )
    {
        lvn = LVN_KEYDOWN;
        *tag = TAG_LISTVIEW;
    }
    else if ( StrCmpI(keyword, "SELECTCHANGED") == 0 )
    {
        lvn = LVN_ITEMCHANGED;
        *tag = TAG_LISTVIEW | TAG_STATECHANGED | TAG_SELECTCHANGED;
    }
    else if ( StrCmpI(keyword, "SELECTFOCUS") == 0 )
    {
        lvn = LVN_ITEMCHANGED;
        *tag = TAG_LISTVIEW | TAG_STATECHANGED | TAG_SELECTCHANGED | TAG_FOCUSCHANGED;
    }
    else
    {
        return false;
    }

    // GETINFOTIP above is forced to TAG_REPLYFROMREXX, so, if not GETINFOTIP,
    // then honor the willReply argument.
    if ( (*tag & TAG_REPLYFROMREXX) !=  TAG_REPLYFROMREXX )
    {
        uint32_t extraTag = _willReplyToTag(c, willReply, false, 4);
        if ( extraTag == TAG_INVALID )
        {
            return false;
        }
        *tag = *tag | extraTag;
    }

    *code = lvn;
    return true;
}

/**
 * Convert a month calendar notification code to a method name.
 */
inline CSTRING mcn2name(uint32_t mcn)
{
    switch ( mcn )
    {
        case MCN_GETDAYSTATE    : return "onGetDayState";
        case NM_RELEASEDCAPTURE : return "onReleased";
        case MCN_SELCHANGE      : return "onSelChange";
        case MCN_SELECT         : return "onSelect";
        case MCN_VIEWCHANGE     : return "onViewChange";
    }
    return "onMCN";
}

/**
 * Convert a keyword to the proper month calendar notification code.
 *
 * We know the keyword arg position is 2.  The MonthCalendar control is post
 * ooRexx 4.0.1 so we raise an exception on error.
 */
static bool keyword2mcn(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t mcn;

    if ( StrCmpI(keyword,      "GETDAYSTATE") == 0 ) mcn = MCN_GETDAYSTATE;
    else if ( StrCmpI(keyword, "RELEASED")    == 0 ) mcn = NM_RELEASEDCAPTURE;
    else if ( StrCmpI(keyword, "SELCHANGE")   == 0 ) mcn = MCN_SELCHANGE;
    else if ( StrCmpI(keyword, "SELECT")      == 0 ) mcn = MCN_SELECT;
    else if ( StrCmpI(keyword, "VIEWCHANGE")  == 0 ) mcn = MCN_VIEWCHANGE;
    else
    {
        wrongArgValueException(c->threadContext, 2, MCN_KEYWORDS, keyword);
        return false;
    }
    *flag = mcn;
    return true;
}

/**
 * Convert a generic NM_ notification code to a method name.
 */
inline CSTRING nm2name(uint32_t nm)
{
    switch ( nm )
    {
        case NM_OUTOFMEMORY    : return "onOutOfMEMORY";
        case NM_CLICK          : return "onClick"      ;
        case NM_DBLCLK         : return "onDblClk"     ;
        case NM_RETURN         : return "onEnter"      ;
        case NM_RCLICK         : return "onRClick"     ;
        case NM_RDBLCLK        : return "onRDblClk"    ;
        case NM_SETFOCUS       : return "onGotFocus"   ;
        case NM_KILLFOCUS      : return "onLostFocus"  ;
    }
    return "onNM";
}

/**
 * Convert a keyword to the proper generic NM_ notification code.
 *
 * We know the keyword arg position is 2.  We can not raise an exception here,
 * this is original ooDialog implementation.
 */
static bool keyword2nm(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t nm;

    if ( StrCmpI(keyword,      "OUTOFMEMORY") == 0 ) nm = NM_OUTOFMEMORY ;
    else if ( StrCmpI(keyword, "CLICK"      ) == 0 ) nm = NM_CLICK       ;
    else if ( StrCmpI(keyword, "DBLCLK"     ) == 0 ) nm = NM_DBLCLK      ;
    else if ( StrCmpI(keyword, "ENTER"      ) == 0 ) nm = NM_RETURN      ;
    else if ( StrCmpI(keyword, "RCLICK"     ) == 0 ) nm = NM_RCLICK      ;
    else if ( StrCmpI(keyword, "RDBLCLK"    ) == 0 ) nm = NM_RDBLCLK     ;
    else if ( StrCmpI(keyword, "GOTFOCUS"   ) == 0 ) nm = NM_SETFOCUS    ;
    else if ( StrCmpI(keyword, "LOSTFOCUS"  ) == 0 ) nm = NM_KILLFOCUS   ;
    else
    {
        return false;
    }
    *flag = nm;
    return true;
}

/**
 * Convert a rebar notification code to a method name.
 */
inline CSTRING rbn2name(uint32_t rbn)
{
    switch ( rbn )
    {
        case NM_NCHITTEST       : return "onNcHitTest";
        case NM_RELEASEDCAPTURE : return "onReleasedCapture";
        case RBN_AUTOBREAK      : return "onAutoBreak";
        case RBN_AUTOSIZE       : return "onAutoSize";
        case RBN_BEGINDRAG      : return "onBeginDrag";
        case RBN_CHEVRONPUSHED  : return "onChevronPushed";
        case RBN_CHILDSIZE      : return "onChildSize";
        case RBN_DELETEDBAND    : return "onDeletedBand";
        case RBN_DELETINGBAND   : return "onDeletingBand";
        case RBN_ENDDRAG        : return "onEndDrag";
        case RBN_GETOBJECT      : return "onGetObject";
        case RBN_HEIGHTCHANGE   : return "onHeightChange";
        case RBN_LAYOUTCHANGED  : return "onLayoutChanged";
        case RBN_MINMAX         : return "onMinMax";
        case RBN_SPLITTERDRAG   : return "onSplitterDrag";
    }
    return "onRBN";
}

/**
 * Convert a keyword to the proper rebar notification code.
 *
 * We know the keyword arg position is 2.  The ReBar control is post
 * ooRexx 4.0.1 so we raise an exception on error.
 */
static bool keyword2rbn(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t rbn;

    if ( StrCmpI(keyword,      "NCHITTEST")       == 0 ) rbn = NM_NCHITTEST      ;
    else if ( StrCmpI(keyword, "RELEASEDCAPTURE") == 0 ) rbn = NM_RELEASEDCAPTURE;
    else if ( StrCmpI(keyword, "AUTOBREAK")       == 0 ) rbn = RBN_AUTOBREAK     ;
    else if ( StrCmpI(keyword, "AUTOSIZE")        == 0 ) rbn = RBN_AUTOSIZE      ;
    else if ( StrCmpI(keyword, "BEGINDRAG")       == 0 ) rbn = RBN_BEGINDRAG     ;
    else if ( StrCmpI(keyword, "CHEVRONPUSHED")   == 0 ) rbn = RBN_CHEVRONPUSHED ;
    else if ( StrCmpI(keyword, "CHILDSIZE")       == 0 ) rbn = RBN_CHILDSIZE     ;
    else if ( StrCmpI(keyword, "DELETEDBAND")     == 0 ) rbn = RBN_DELETEDBAND   ;
    else if ( StrCmpI(keyword, "DELETINGBAND")    == 0 ) rbn = RBN_DELETINGBAND  ;
    else if ( StrCmpI(keyword, "ENDDRAG")         == 0 ) rbn = RBN_ENDDRAG       ;
    else if ( StrCmpI(keyword, "GETOBJECT")       == 0 ) rbn = RBN_GETOBJECT     ;
    else if ( StrCmpI(keyword, "HEIGHTCHANGE")    == 0 ) rbn = RBN_HEIGHTCHANGE  ;
    else if ( StrCmpI(keyword, "LAYOUTCHANGED")   == 0 ) rbn = RBN_LAYOUTCHANGED ;
    else if ( StrCmpI(keyword, "MINMAX")          == 0 ) rbn = RBN_MINMAX        ;
    else if ( StrCmpI(keyword, "SPLITTERDRAG")    == 0 ) rbn = RBN_SPLITTERDRAG  ;
    else
    {
        wrongArgValueException(c->threadContext, 2, RBN_KEYWORDS, keyword);
        return false;
    }
    *flag = rbn;
    return true;
}

inline bool isMustReplyRbn(uint32_t rbn)
{
    return rbn ==  NM_NCHITTEST  || rbn ==  RBN_AUTOBREAK || rbn ==  RBN_BEGINDRAG ||
           rbn ==  RBN_CHILDSIZE || rbn ==  RBN_GETOBJECT || rbn ==  RBN_MINMAX;
}

/**
 * Convert a status bar notification code to a method name.
 */
inline CSTRING sbn2name(uint32_t sbn)
{
    switch ( sbn )
    {
        case NM_CLICK              : return "onClick";
        case NM_DBLCLK             : return "onDblClk";
        case NM_RCLICK             : return "onRClick";
        case NM_RDBLCLK            : return "onRDblClk";
        case SBN_SIMPLEMODECHANGE  : return "onSimpleModeChange";
    }
    return "onSBN";
}

/**
 * Convert a keyword to the proper rebar notification code.
 *
 * We know the keyword arg position is 2.  The ReBar control is post
 * ooRexx 4.0.1 so we raise an exception on error.
 */
static bool keyword2sbn(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t sbn;

    if ( StrCmpI(keyword,      "CLICK")             == 0 ) sbn = NM_CLICK              ;
    else if ( StrCmpI(keyword, "DBLCLK")            == 0 ) sbn = NM_DBLCLK             ;
    else if ( StrCmpI(keyword, "RCLICK")            == 0 ) sbn = NM_RCLICK             ;
    else if ( StrCmpI(keyword, "RDBLCLK")           == 0 ) sbn = NM_RDBLCLK            ;
    else if ( StrCmpI(keyword, "SIMPLEMODECHANGE")  == 0 ) sbn = SBN_SIMPLEMODECHANGE  ;
    else
    {
        wrongArgValueException(c->threadContext, 2, SBN_KEYWORDS, keyword);
        return false;
    }
    *flag = sbn;
    return true;
}

inline bool isMustReplySbn(uint32_t sbn)
{
    return sbn ==  NM_CLICK   || sbn ==  NM_DBLCLK || sbn ==  NM_RCLICK ||
           sbn ==  NM_RDBLCLK;
}

/**
 * Convert a scroll bar notification code to a method name.
 *
 * For SB_LINEUP   / SB_LINELEFT  -> onUp
 * For SB_LINEDOWN / SB_LINERIGHT -> onDown
 * For SB_PAGEUP   / SB_PAGELEFT  -> onPageUp
 * For SB_PAGEDOWN / SB_PAGERIGHT -> onPageDown
 * For SB_TOP      / SB_LEFT      -> onTop
 * For SB_BOTTOM   / SB_RIGHT     -> onBottom
 */
inline CSTRING scbn2name(uint32_t sbn)
{
    switch ( sbn )
    {
        case SB_LINEUP        : return "onUp       ";
        case SB_LINEDOWN      : return "onDown     ";
        case SB_PAGEUP        : return "onPageUp   ";
        case SB_PAGEDOWN      : return "onPageDown ";
        case SB_THUMBPOSITION : return "onPosition ";
        case SB_THUMBTRACK    : return "onDrag     ";
        case SB_TOP           : return "onTop      ";
        case SB_BOTTOM        : return "onBottom   ";
        case SB_ENDSCROLL     : return "onEndScroll";
    }
    return "onSBN";
}

/**
 * Convert a keyword to the proper scroll bar notification code.
 *
 * We know the keyword arg position is 2.  No exception can be raised here.
 */
static bool keyword2scbn(CSTRING keyword, uint32_t *flag)
{
    uint32_t sbn;

    if ( StrCmpI(keyword,      "UP"       ) == 0 ) sbn = SB_LINEUP;        // Old word, confusing.
    else if ( StrCmpI(keyword, "LINEUP"   ) == 0 ) sbn = SB_LINEUP;
    else if ( StrCmpI(keyword, "LINELEFT" ) == 0 ) sbn = SB_LINELEFT;
    else if ( StrCmpI(keyword, "DOWN"     ) == 0 ) sbn = SB_LINEDOWN;      // Old word, confusing.
    else if ( StrCmpI(keyword, "LINEDOWN" ) == 0 ) sbn = SB_LINEDOWN;
    else if ( StrCmpI(keyword, "LINERIGHT") == 0 ) sbn = SB_LINERIGHT;
    else if ( StrCmpI(keyword, "PAGEUP"   ) == 0 ) sbn = SB_PAGEUP;
    else if ( StrCmpI(keyword, "PAGELEFT" ) == 0 ) sbn = SB_PAGELEFT;
    else if ( StrCmpI(keyword, "PAGEDOWN" ) == 0 ) sbn = SB_PAGEDOWN;
    else if ( StrCmpI(keyword, "PAGERIGHT") == 0 ) sbn = SB_PAGERIGHT;
    else if ( StrCmpI(keyword, "POSITION" ) == 0 ) sbn = SB_THUMBPOSITION;
    else if ( StrCmpI(keyword, "DRAG"     ) == 0 ) sbn = SB_THUMBTRACK;
    else if ( StrCmpI(keyword, "TOP"      ) == 0 ) sbn = SB_TOP;
    else if ( StrCmpI(keyword, "LEFT"     ) == 0 ) sbn = SB_LEFT;
    else if ( StrCmpI(keyword, "BOTTOM"   ) == 0 ) sbn = SB_BOTTOM;
    else if ( StrCmpI(keyword, "RIGHT"    ) == 0 ) sbn = SB_RIGHT;
    else if ( StrCmpI(keyword, "ENDSCROLL") == 0 ) sbn = SB_ENDSCROLL;
    else
    {
        return false;
    }

    *flag = sbn;
    return true;
}

/**
 * Convert a static control notification code to a method name.
 */
inline CSTRING stn2name(uint32_t stn)
{
    switch ( stn )
    {
        case STN_CLICKED       : return "onClick"       ;
        case STN_DBLCLK        : return "onDblClk"      ;
        case STN_ENABLE        : return "onEnable"      ;
        case STN_DISABLE       : return "onDisable"     ;
    }
    return "onEN";
}

/**
 * Convert a keyword to the proper static control notification code.
 *
 * We can not raise an exception here, this is original ooDialog implementation.
 */
static bool keyword2stn(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t stn;

    if ( StrCmpI(keyword,      "CLICK"      ) == 0 ) stn = STN_CLICKED    ;
    else if ( StrCmpI(keyword, "DBLCLK"     ) == 0 ) stn = STN_DBLCLK     ;
    else if ( StrCmpI(keyword, "ENABLE"     ) == 0 ) stn = STN_ENABLE     ;
    else if ( StrCmpI(keyword, "DISABLE"    ) == 0 ) stn = STN_DISABLE    ;
    else
    {
        return false;
    }
    *flag = stn;
    return true;
}

/**
 * Convert a track bar notification code to a method name.
 */
inline CSTRING tb2name(uint32_t tb)
{
    switch ( tb )
    {
        case TB_LINEUP            : return "onUp"           ;
        case TB_LINEDOWN          : return "onDown"         ;
        case TB_PAGEUP            : return "onPageUp"       ;
        case TB_PAGEDOWN          : return "onPageDown"     ;
        case TB_THUMBPOSITION     : return "onPosition"     ;
        case TB_THUMBTRACK        : return "onDrag"         ;
        case TB_TOP               : return "onTop"          ;
        case TB_BOTTOM            : return "onBottom"       ;
        case TB_ENDTRACK          : return "onEndTrack"     ;
    }
    return "onEN";
}

/**
 * Convert a keyword to the proper track bar notification code.
 *
 * We can not raise an exception here, this is original ooDialog implemtbntation.
 */
static bool keyword2tb(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t tb;

    if ( StrCmpI(keyword,      "UP"         ) == 0 ) tb = TB_LINEUP         ;
    else if ( StrCmpI(keyword, "DOWN"       ) == 0 ) tb = TB_LINEDOWN       ;
    else if ( StrCmpI(keyword, "PAGEUP"     ) == 0 ) tb = TB_PAGEUP         ;
    else if ( StrCmpI(keyword, "PAGEDOWN"   ) == 0 ) tb = TB_PAGEDOWN       ;
    else if ( StrCmpI(keyword, "POSITION"   ) == 0 ) tb = TB_THUMBPOSITION  ;
    else if ( StrCmpI(keyword, "DRAG"       ) == 0 ) tb = TB_THUMBTRACK     ;
    else if ( StrCmpI(keyword, "TOP"        ) == 0 ) tb = TB_TOP            ;
    else if ( StrCmpI(keyword, "BOTTOM"     ) == 0 ) tb = TB_BOTTOM         ;
    else if ( StrCmpI(keyword, "ENDTRACK"   ) == 0 ) tb = TB_ENDTRACK       ;
    else
    {
        return false;
    }
    *flag = tb;
    return true;
}

/**
 * Convert a tab control notification code to a method name.
 */
inline CSTRING tcn2name(uint32_t tcn)
{
    switch ( tcn )
    {
        case TCN_KEYDOWN        : return "onKEYDOWN"       ;
        case TCN_SELCHANGE      : return "onSELCHANGE"     ;
        case TCN_SELCHANGING    : return "onSELCHANGING"   ;
    }
    return "onTCN";
}

/**
 * Convert a keyword to the proper tab control notification code.
 *
 * We know the keyword arg position is 2.  No exceptions, this is the original
 * ooDialog implementation.
 */
static bool keyword2tcn(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t tcn;

    if ( StrCmpI(keyword,      "KEYDOWN"       ) == 0 ) tcn = TCN_KEYDOWN       ;
    else if ( StrCmpI(keyword, "SELCHANGE"     ) == 0 ) tcn = TCN_SELCHANGE     ;
    else if ( StrCmpI(keyword, "SELCHANGING"   ) == 0 ) tcn = TCN_SELCHANGING   ;
    else
    {
        return false;
    }
    *flag = tcn;
    return true;
}

/**
 * Convert a toolbar notification code to a method name.
 */
inline CSTRING tbn2name(uint32_t tbn)
{
    switch ( tbn )
    {
        case NM_CHAR               : return "onChar"             ;
        case NM_CLICK              : return "onClick"            ;
        case NM_DBLCLK             : return "onDblClk"           ;
        case NM_KEYDOWN            : return "onKeyDown"          ;
        case NM_LDOWN              : return "onLDown"            ;
        case NM_RCLICK             : return "onRClick"           ;
        case NM_RDBLCLK            : return "onRDblClk"          ;
        case NM_RELEASEDCAPTURE    : return "onReleasedCapture"  ;
        case NM_TOOLTIPSCREATED    : return "onToolTipsCreated"  ;
        case TBN_BEGINADJUST       : return "onBeginAdjust"      ;
        case TBN_BEGINDRAG         : return "onBeginDrag"        ;
        case TBN_CUSTHELP          : return "onCustHelp"         ;
        case TBN_DELETINGBUTTON    : return "onDeletingButton"   ;
        case TBN_DRAGOUT           : return "onDragOut"          ;
        case TBN_DRAGOVER          : return "onDragOver"         ;
        case TBN_DROPDOWN          : return "onDropDown"         ;
        case TBN_DUPACCELERATOR    : return "onDupAccelerator"   ;
        case TBN_ENDADJUST         : return "onEndAdjust"        ;
        case TBN_ENDDRAG           : return "onEndDrag"          ;
        case TBN_GETBUTTONINFO     : return "onGetButtonInfo"    ;
        case TBN_GETDISPINFO       : return "onGetDISPinfo"      ;
        case TBN_GETINFOTIP        : return "onGetInfoTip"       ;
        case TBN_GETOBJECT         : return "onGetObject"        ;
        case TBN_HOTITEMCHANGE     : return "onHotItemChange"    ;
        case TBN_INITCUSTOMIZE     : return "onInitCustomize"    ;
        case TBN_MAPACCELERATOR    : return "onMapAccelerator"   ;
        case TBN_QUERYDELETE       : return "onQueryDelete"      ;
        case TBN_QUERYINSERT       : return "onQueryInsert"      ;
        case TBN_RESET             : return "onReset"            ;
        case TBN_RESTORE           : return "onRestore"          ;
        case TBN_SAVE              : return "onSave"             ;
        case TBN_TOOLBARCHANGE     : return "onToolbarChange"    ;
        case TBN_WRAPACCELERATOR   : return "onWrapAccelerator"  ;
        case TBN_WRAPHOTITEM       : return "onWrapHotItem"      ;
    }
    return "onTBN";
}

/**
 * Convert a keyword to the proper toolbar notification code.
 *
 * We know the keyword arg position is 2.  The ToolBar control is post
 * ooRexx 4.0.1 so we raise an exception on error.
 */
static bool keyword2tbn(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t tbn;

    if ( StrCmpI(keyword,      "CHAR"               ) == 0 ) tbn = NM_CHAR               ;
    else if ( StrCmpI(keyword, "CLICK"              ) == 0 ) tbn = NM_CLICK              ;
    else if ( StrCmpI(keyword, "DBLCLK"             ) == 0 ) tbn = NM_DBLCLK             ;
    else if ( StrCmpI(keyword, "KEYDOWN"            ) == 0 ) tbn = NM_KEYDOWN            ;
    else if ( StrCmpI(keyword, "LDOWN"              ) == 0 ) tbn = NM_LDOWN              ;
    else if ( StrCmpI(keyword, "RCLICK"             ) == 0 ) tbn = NM_RCLICK             ;
    else if ( StrCmpI(keyword, "RDBLCLK"            ) == 0 ) tbn = NM_RDBLCLK            ;
    else if ( StrCmpI(keyword, "RELEASEDCAPTURE"    ) == 0 ) tbn = NM_RELEASEDCAPTURE    ;
    else if ( StrCmpI(keyword, "TOOLTIPSCREATED"    ) == 0 ) tbn = NM_TOOLTIPSCREATED    ;
    else if ( StrCmpI(keyword, "BEGINADJUST"        ) == 0 ) tbn = TBN_BEGINADJUST       ;
    else if ( StrCmpI(keyword, "BEGINDRAG"          ) == 0 ) tbn = TBN_BEGINDRAG         ;
    else if ( StrCmpI(keyword, "CUSTHELP"           ) == 0 ) tbn = TBN_CUSTHELP          ;
    else if ( StrCmpI(keyword, "DELETINGBUTTON"     ) == 0 ) tbn = TBN_DELETINGBUTTON    ;
    else if ( StrCmpI(keyword, "DRAGOUT"            ) == 0 ) tbn = TBN_DRAGOUT           ;
    else if ( StrCmpI(keyword, "DRAGOVER"           ) == 0 ) tbn = TBN_DRAGOVER          ;
    else if ( StrCmpI(keyword, "DROPDOWN"           ) == 0 ) tbn = TBN_DROPDOWN          ;
    else if ( StrCmpI(keyword, "DUPACCELERATOR"     ) == 0 ) tbn = TBN_DUPACCELERATOR    ;
    else if ( StrCmpI(keyword, "ENDADJUST"          ) == 0 ) tbn = TBN_ENDADJUST         ;
    else if ( StrCmpI(keyword, "ENDDRAG"            ) == 0 ) tbn = TBN_ENDDRAG           ;
    else if ( StrCmpI(keyword, "GETBUTTONINFO"      ) == 0 ) tbn = TBN_GETBUTTONINFO     ;
    else if ( StrCmpI(keyword, "GETDISPINFO"        ) == 0 ) tbn = TBN_GETDISPINFO       ;
    else if ( StrCmpI(keyword, "GETINFOTIP"         ) == 0 ) tbn = TBN_GETINFOTIP        ;
    else if ( StrCmpI(keyword, "GETOBJECT"          ) == 0 ) tbn = TBN_GETOBJECT         ;
    else if ( StrCmpI(keyword, "HOTITEMCHANGE"      ) == 0 ) tbn = TBN_HOTITEMCHANGE     ;
    else if ( StrCmpI(keyword, "INITCUSTOMIZE"      ) == 0 ) tbn = TBN_INITCUSTOMIZE     ;
    else if ( StrCmpI(keyword, "MAPACCELERATOR"     ) == 0 ) tbn = TBN_MAPACCELERATOR    ;
    else if ( StrCmpI(keyword, "QUERYDELETE"        ) == 0 ) tbn = TBN_QUERYDELETE       ;
    else if ( StrCmpI(keyword, "QUERYINSERT"        ) == 0 ) tbn = TBN_QUERYINSERT       ;
    else if ( StrCmpI(keyword, "RESET"              ) == 0 ) tbn = TBN_RESET             ;
    else if ( StrCmpI(keyword, "RESTORE"            ) == 0 ) tbn = TBN_RESTORE           ;
    else if ( StrCmpI(keyword, "SAVE"               ) == 0 ) tbn = TBN_SAVE              ;
    else if ( StrCmpI(keyword, "TOOLBARCHANGE"      ) == 0 ) tbn = TBN_TOOLBARCHANGE     ;
    else if ( StrCmpI(keyword, "WRAPACCELERATOR"    ) == 0 ) tbn = TBN_WRAPACCELERATOR   ;
    else if ( StrCmpI(keyword, "WRAPHOTITEM"        ) == 0 ) tbn = TBN_WRAPHOTITEM       ;
    else
    {
        wrongArgValueException(c->threadContext, 2, TBN_KEYWORDS, keyword);
        return false;
    }
    *flag = tbn;
    return true;
}

inline bool isMustReplyTbn(uint32_t tbn)
{
    return tbn == NM_CHAR            || tbn == NM_CLICK          || tbn == NM_KEYDOWN        ||
           tbn == NM_RCLICK          || tbn == NM_RDBLCLK        || tbn == TBN_DROPDOWN      ||
           tbn == TBN_GETBUTTONINFO  || tbn == TBN_GETDISPINFO   || tbn == TBN_GETINFOTIP    ||
           tbn == TBN_GETOBJECT      || tbn == TBN_HOTITEMCHANGE || tbn == TBN_INITCUSTOMIZE ||
           tbn == TBN_MAPACCELERATOR || tbn == TBN_QUERYDELETE   || tbn == TBN_QUERYINSERT   ||
           tbn == TBN_RESTORE;
}

/**
 * Convert a tool tip notification code to a method name.
 */
inline CSTRING ttn2name(uint32_t ttn)
{
    switch ( ttn )
    {
        case TTN_LINKCLICK : return "onLinkClick";
        case TTN_NEEDTEXT  : return "onNeedText";
        case TTN_POP       : return "onPop";
        case TTN_SHOW      : return "onShow";
    }
    return "onTTN";
}

/**
 * Convert a keyword to the proper tool tip notification code.
 *
 * We know the keyword arg position is 2.  The ToolTip control is post
 * ooRexx 4.0.1 so we raise an exception on error.
 */
static bool keyword2ttn(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t ttn;

    if ( StrCmpI(keyword, "LINKCLICK") == 0 )
    {
        if ( ! requiredComCtl32Version(c, "LINKCLICK", COMCTL32_6_0) )
        {
            return false;
        }
        ttn = TTN_LINKCLICK;
    }
    else if ( StrCmpI(keyword, "NEEDTEXT")  == 0 ) ttn = TTN_NEEDTEXT;
    else if ( StrCmpI(keyword, "POP")       == 0 ) ttn = TTN_POP;
    else if ( StrCmpI(keyword, "SHOW")      == 0 ) ttn = TTN_SHOW;
    else
    {
        wrongArgValueException(c->threadContext, 2, TTN_KEYWORDS, keyword);
        return false;
    }
    *flag = ttn;
    return true;
}

/**
 * Convert a tree view notification code and tag to a method name.
 */
inline CSTRING tvn2name(uint32_t tvn, uint32_t tag)
{
    switch ( tvn )
    {
        case TVN_SELCHANGING    : return "onSelChanging";
        case TVN_SELCHANGED     : return "onSelChanged";
        case TVN_BEGINDRAG      : return "onBeginDrag";
        case TVN_BEGINRDRAG     : return "onBeginRDrag";
        case TVN_DELETEITEM     : return "onDelete";
        case TVN_BEGINLABELEDIT : return "onBeginEdit";
        case TVN_ENDLABELEDIT   : return "onEndEdit";
        case TVN_ITEMEXPANDING  : return "onExpanding";
        case TVN_ITEMEXPANDED   : return "onExpanded";
        case TVN_GETINFOTIP     : return "onGetInfoTip";
        case TVN_KEYDOWN        :
            if ( (tag & TAG_FLAGMASK) == TAG_PRESERVE_OLD )
            {
                return "onKeyDown";
            }
            else
            {
                return "onKeydownEx";
            }

    }
    return "onTVN";
}

/**
 * Convert a keyword to the proper tree-view notification code.
 *
 */
static bool keyword2tvn(RexxMethodContext *c, CSTRING keyword, uint32_t *code, uint32_t *pTag, bool *isDefEdit,
                        RexxObjectPtr willReply, bool willReplyUsed)
{
    uint32_t tvn  = 0;
    uint32_t tag = 0;

    *isDefEdit = false;

    if ( StrCmpI(keyword, "BEGINDRAG"  ) == 0 )
    {
        tvn = TVN_BEGINDRAG;
        tag = TAG_TREEVIEW;

        if ( ! willReplyUsed )
        {
            tag |= TAG_PRESERVE_OLD;
        }
    }
    else if ( StrCmpI(keyword, "BEGINEDIT"  ) == 0 )
    {
        tvn = TVN_BEGINLABELEDIT;
        tag = TAG_TREEVIEW;

        if ( ! willReplyUsed )
        {
            tag |= TAG_PRESERVE_OLD;
        }
    }
    else if ( StrCmpI(keyword, "BEGINRDRAG" ) == 0 )
    {
        tvn = TVN_BEGINRDRAG;
        tag = TAG_TREEVIEW;

        if ( ! willReplyUsed )
        {
            tag |= TAG_PRESERVE_OLD;
        }
    }
    else if ( StrCmpI(keyword, "DEFAULTEDIT") == 0 )
    {
        *isDefEdit = true;
        tag = TAG_TREEVIEW | TAG_PRESERVE_OLD;
    }
    else if ( StrCmpI(keyword, "DELETE"     ) == 0 )
    {
        tvn = TVN_DELETEITEM;
        tag = TAG_TREEVIEW;

        if ( ! willReplyUsed )
        {
            tag |= TAG_PRESERVE_OLD;
        }
    }
    else if ( StrCmpI(keyword, "ENDEDIT"    ) == 0 )
    {
        tvn = TVN_ENDLABELEDIT;
        tag = TAG_TREEVIEW;

        if ( ! willReplyUsed )
        {
            tag |= TAG_PRESERVE_OLD;
        }
    }
    else if ( StrCmpI(keyword, "EXPANDED") == 0 )
    {
        tvn = TVN_ITEMEXPANDED;
        tag = TAG_TREEVIEW;
    }
    else if ( StrCmpI(keyword, "EXPANDING"  ) == 0 )
    {
        tvn = TVN_ITEMEXPANDING;
        tag = TAG_TREEVIEW;
    }
    else if ( StrCmpI(keyword, "GETINFOTIP") == 0 )
    {
        tvn = TVN_GETINFOTIP;
        tag = TAG_TREEVIEW | TAG_REPLYFROMREXX;
    }
    else if ( StrCmpI(keyword, "KEYDOWN"    ) == 0 )
    {
        tvn = TVN_KEYDOWN;
        tag = TAG_TREEVIEW | TAG_PRESERVE_OLD;
    }
    else if ( StrCmpI(keyword, "KEYDOWNEX") == 0 )
    {
        tvn = TVN_KEYDOWN;
        tag = TAG_TREEVIEW;
    }
    else if ( StrCmpI(keyword, "SELCHANGED" ) == 0 )
    {
        tvn = TVN_SELCHANGED;
        tag = TAG_TREEVIEW;

        if ( ! willReplyUsed )
        {
            tag |= TAG_PRESERVE_OLD;
        }
    }
    else if ( StrCmpI(keyword, "SELCHANGING") == 0 )
    {
        tvn = TVN_SELCHANGING;
        tag = TAG_TREEVIEW;

        if ( ! willReplyUsed )
        {
            tag |= TAG_PRESERVE_OLD;
        }
    }
    else
    {
        return false;
    }

    if ( willReplyUsed )
    {
        uint32_t extraTag = _willReplyToTag(c, willReply, false, 4);
        if ( extraTag == TAG_INVALID )
        {
            return false;
        }
        tag |= extraTag;
    }

    *code = tvn;
    *pTag = tag;
    return true;
}

static uint32_t methodName2windowMessage(CSTRING methodName)
{
    uint32_t msg = 0;

    if (      strcmp(methodName, "CONNECTACTIVATE")      == 0 ) msg = WM_ACTIVATE;
    else if ( strcmp(methodName, "CONNECTHELP")          == 0 ) msg = WM_HELP;     // (F1)
    else if ( strcmp(methodName, "CONNECTMOVE")          == 0 ) msg = WM_MOVE;
    else if ( strcmp(methodName, "CONNECTPOSCHANGED")    == 0 ) msg = WM_WINDOWPOSCHANGED;
    else if ( strcmp(methodName, "CONNECTRESIZE")        == 0 ) msg = WM_SIZE;
    else if ( strcmp(methodName, "CONNECTRESIZING")      == 0 ) msg = WM_SIZING;
    else if ( strcmp(methodName, "CONNECTSIZEMOVEENDED") == 0 ) msg = WM_EXITSIZEMOVE;

    return msg;
}

/**
 * Checks if the specified track bar is a vertical track bar.
 */
static inline bool isVerticalTrackBar(HWND hTrackBar)
{
    return (((uint32_t)GetWindowLong(hTrackBar, GWL_STYLE) & TBS_VERT) == TBS_VERT);
}

static RexxObjectPtr notSupported(HWND hDlg)
{
    static char *title = "Unsupported Method Invocation";
    static char *msg   = "The connectSliderNotify() method is not supported\n"
                         "for the deprecated CategoryDialog dialog\n\n"
                         "Please remove this invocation from your ooDialog\n"
                         "program.  If you did not write this program, contact\n"
                         "the developer for a fix.";

    MessageBox(hDlg, msg, title, MB_OK | MB_ICONHAND | MB_APPLMODAL);
    return TheNegativeOneObj;
}

/**
 * Creates a Rexx argument array for, presumably, the invocation of a Rexx
 * method connected to some type of key board event.
 *
 * @param c           Thread context we are operating in.
 * @param wParam      The WPARAM for the key event, which is presumed to be the
 *                    charater code for the key event.
 * @param isExtended  Was this an extended key.
 * @param rexxControl The rexx dialog control object.  May be null.
 *
 * @return RexxArrayObject
 *
 * @remarks  This function is currently called for connectKeyPress() and
 *           connectCharEvent() processing and could probably be used for other
 *           key event connections if any are added to ooDialog.
 *
 *           The isExtended parameter is needed to be able to distinguish
 *           between a WM_CHAR '.' and the extended Delete key.
 */
RexxArrayObject getKeyEventRexxArgs(RexxThreadContext *c, WPARAM wParam, bool isExtended, RexxObjectPtr rexxControl)
{
    BOOL bShift = (GetAsyncKeyState(VK_SHIFT) & ISDOWN) ? 1 : 0;
    BOOL bControl = (GetAsyncKeyState(VK_CONTROL) & ISDOWN) ? 1 : 0;
    BOOL bAlt = (GetAsyncKeyState(VK_MENU) & ISDOWN) ? 1 : 0;

    CHAR info[64] = {'\0'};

    GetKeyState(VK_NUMLOCK) & TOGGLED ? strcpy(info, "numOn") : strcpy(info, "numOff");

    GetKeyState(VK_CAPITAL) & TOGGLED ? strcat(info, " capsOn") : strcat(info, " capsOff");

    if ( bShift )
    {
        GetAsyncKeyState(VK_RSHIFT) & ISDOWN ? strcat(info, " rShift") : strcat(info, " lShift");
    }
    if ( bControl )
    {
        GetAsyncKeyState(VK_RCONTROL) & ISDOWN ? strcat(info, " rControl") : strcat(info, " lControl");
    }
    if ( bAlt )
    {
        GetAsyncKeyState(VK_RMENU) & ISDOWN ? strcat(info, " rAlt") : strcat(info, " lAlt");
    }

    GetKeyState(VK_SCROLL) & TOGGLED ? strcat(info, " scrollOn") : strcat(info, " scrollOff");

    if ( isExtended )
    {
        strcat(info, " extended");
    }
    RexxArrayObject args = c->ArrayOfFour(c->WholeNumber(wParam), c->Logical(bShift),
                                          c->Logical(bControl), c->Logical(bAlt));
    c->ArrayPut(args, c->String(info), 5);
    c->ArrayPut(args, rexxControl ? rexxControl : TheNilObj, 6);

    return args;
}


/**
 * Releases the local references to the arg array created in
 * getKeyEventRexxArgs().  We cheat here and use our knowledge of the function
 * above and release the first 5 args in the array.  The 6th arg is either
 * TheNilObj, or a dialog control ojbect.  Neither one of these should be
 * released.
 *
 * @param c
 * @param args
 */
void releaseKeyEventRexxArgs(RexxThreadContext *c, RexxArrayObject args)
{
    for ( size_t i = 1; i <= 5; i++ )
    {
        RexxObjectPtr o = c->ArrayAt(args, i);
        c->ReleaseLocalReference(o);
    }
    c->ReleaseLocalReference(args);
}


/**
 * The keyboard hook procedure.
 *
 * This is a thread specific hook, not a global hook. This function executes in
 * the same thread as the dialog's window procedure.  The thread ID is used to
 * locate the correct dialog CSelf struct, which in turn lets us get the CSelf
 * struct for the event notification object.
 *
 * The key is examined to see if the user has set an ooDialog method for it and
 * if it is a key down event.  If so, the key data is sent on to
 * processKeyData() where the actual ooDialog method invocation is set up.  If
 * the user has also set a filter, there may be no method invocation after all.
 *
 */
LRESULT CALLBACK keyboardHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    register size_t i;
    DWORD id = GetCurrentThreadId();

    /* No matter what, we need to find the event notification struct, it is the
     * only way to get the hook handle.
     */
    for ( i = 0; i < CountDialogs; i++ )
    {
        if ( DialogTable[i]->dlgProcThreadID == id )
        {
            break;
        }
    }

    /* If we can't find it, there is nothing to do about it.  We can't call the
     * next hook, so just return 0.
     */
    if ( i >= CountDialogs )
    {
        return 0;
    }

    pCEventNotification pcen    = DialogTable[i]->enCSelf;
    pSubClassData       pSCData = (pSubClassData)pcen->pHookData;
    KEYPRESSDATA *pKeyPressData = (KEYPRESSDATA *)pSCData->pData;

    if ( (code == HC_ACTION) && pKeyPressData->key[wParam] )
    {
        if ( !(lParam & KEY_RELEASED) && !(lParam & KEY_WASDOWN) )
        {
            processKeyPress(pSCData, wParam, lParam);
        }
    }
	return CallNextHookEx(pcen->hHook, code, wParam, lParam);
}


/**
 * Sets the Windows keyboard hook (WH_KEYBOARD.)  SetWindowsHookEx() has to run
 * in the same thread as the dialog, so a user message is sent to the dialog
 * window procedure to do the actual work.
 *
 * If the hook is not set, all the memory allocation is cleaned up.
 */
static keyPressErr_t setKBHook(pCEventNotification pcen)
{
    pcen->hHook = (HHOOK)SendMessage(pcen->hDlg, WM_USER_HOOK, (WPARAM)&keyboardHookProc, (LPARAM)pcen->pHookData);
    if ( ! pcen->hHook )
    {
        freeKeyPressData((pSubClassData)pcen->pHookData);
        pcen->pHookData = NULL;
        return winAPIErr;
    }
    return noErr;
}


/**
 * Allocates memory for the key press structure and sets up all the data used by
 * the keyboard hook procedure.  Once everything is good the hook is set.
 *
 * If anything fails, all memory is freed.
 */
static keyPressErr_t installKBHook(pCEventNotification pcen, CSTRING method, CSTRING keys, CSTRING filter)
{
    pSubClassData pSCData = (pSubClassData)LocalAlloc(LPTR, sizeof(SubClassData));
    if ( pSCData == NULL )
    {
        return memoryErr;
    }

    KEYPRESSDATA *pKeyData = (KEYPRESSDATA *)LocalAlloc(LPTR, sizeof(KEYPRESSDATA));
    if ( pKeyData == NULL )
    {
        LocalFree(pSCData);
        return memoryErr;
    }

    keyPressErr_t result = setKeyPressData(pKeyData, method, keys, filter);
    if ( result == noErr )
    {
        pSCData->pData = pKeyData;
        pcen->pHookData = pSCData;

        // Note that setKBHook() frees all memeory if it fails;
        result = setKBHook(pcen);
    }
    else
    {
        // To safely use freeKeyPressData() to clean up we need to set pData to
        // pKeyData.
        pSCData->pData = pKeyData;
        freeKeyPressData(pSCData);
    }

    return result;
}


/**
 * Connects a method in the Rexx dialog to a key press(es.)
 *
 * If the hook is not installed, it is installed.  Otherwise, the method is
 * simply added to key press table.
 *
 * @param c
 * @param pcen
 * @param methodName
 * @param keys
 * @param filter
 *
 * @return keyPressErr_t
 */
static keyPressErr_t connectKeyPressHook(RexxMethodContext *c, pCEventNotification pcen, CSTRING methodName,
                                         CSTRING keys, CSTRING filter)
{
    if ( *methodName == '\0' )
    {
        c->RaiseException1(Rexx_Error_Invalid_argument_null, TheOneObj);
        return nameErr;
    }
    if ( *keys == '\0' )
    {
        c->RaiseException1(Rexx_Error_Invalid_argument_null, TheTwoObj);
        return nameErr;
    }

    pSubClassData pSCData;

    HWND hDlg = pcen->hDlg;
    if ( hDlg == NULL || ! IsWindow(hDlg) )
    {
        noWindowsDialogException(c, pcen->rexxSelf);
        return nameErr;
    }

    /* If there is no existing hook, install one and connect the method
     * to it, otherwise connect the method to the existing hook.
     */
    if ( pcen->hHook == NULL )
    {
        keyPressErr_t result = installKBHook(pcen, methodName, keys, filter);
        if ( result == noErr )
        {
            pSCData = (pSubClassData)pcen->pHookData;
            pSCData->pcpbd = dlgToCSelf(c, pcen->rexxSelf);
        }

        return result;
    }

    pSCData = (pSubClassData)pcen->pHookData;
    return setKeyPressData((KEYPRESSDATA *)pSCData->pData, methodName, keys, filter);
}


/**
 * If the hook exists, unhook.  If the key press data exists, free it.
 */
void removeKBHook(pCEventNotification pcen)
{
    if ( pcen->hHook )
    {
        UnhookWindowsHookEx(pcen->hHook);
    }

    freeKeyPressData((pSubClassData)pcen->pHookData);
    pcen->hHook = 0;
    pcen->pHookData = NULL;
}


/**
 * Takes a key event that has an ooDialog method connected to it and invokes the
 * method.
 *
 * It is possible for the key event to be filtered out and no ooDialog method is
 * then invoked.
 *
 * The ooDialog event method gets 6 arguments:
 *   key:         decimal value of the key code.
 *
 *   shift:       true / false, true if the shift key was depressed for this
 *                event.
 *
 *   control:     true / false, true if control key was depressed.
 *
 *   alt:         true / false, ditto.
 *
 *   info:        Keyword string that specifies if right or left shift / control
 *                / alt were down and the state of the num lock, caps lock, and
 *                scroll lock keys.  The string contains some combination of:
 *
 *                rShift, lShift, rControl lControl, rAlt, lAlt, numOn, numOff,
 *                capsOn, capsOff, scrollOn, scrollOf
 *
 *   rexxControl: If key press is connected to a dialog control window, this is
 *                the Rexx dialog control.  If connected to a dialog window,
 *                this is .nil
 *
 * @remarks  The method name (pMethod) can not be longer than 197 chars.  This
 *           is checked for in setKeyPressData()
 *
 *           By default we use GetAsyncKeyState() to check the status of Ctrl,
 *           Alt, and Shift keys.  But, that gets the physical state of the
 *           keyboard.  If other applications insert key events into message
 *           queue of the Rexx application, when it is looking for, say Ctrl-S
 *           and the inserted key event is Ctrl-S, we won't find it here,
 *           because the physical state of the control key will be not pressed.
 *
 *           The GetKeyState() API adjusts the state of the control, alt, shift
 *           keys as it processes the message queue.  So, the Rexx programmer
 *           can specify that she wants to detect this situation by adding
 *           'VIRT' to the key filter.
 */
void processKeyPress(pSubClassData pSCData, WPARAM wParam, LPARAM lParam)
{
    KEYPRESSDATA *pKeyData = (KEYPRESSDATA *)pSCData->pData;

    BOOL passed   = TRUE;
    BOOL bShift   = FALSE;
    BOOL bControl = FALSE;
    BOOL bAlt     = FALSE;

    size_t i = pKeyData->key[wParam];
    char *pMethod = pKeyData->pMethods[i];
    KEYFILTER *pFilter = pKeyData->pFilters[i];

    if ( pFilter && pFilter->virt )
    {
        bShift = (GetKeyState(VK_SHIFT) & ISDOWN) ? 1 : 0;
        bControl = (GetKeyState(VK_CONTROL) & ISDOWN) ? 1 : 0;
        bAlt = (GetKeyState(VK_MENU) & ISDOWN) ? 1 : 0;
    }
    else
    {
        bShift = (GetAsyncKeyState(VK_SHIFT) & ISDOWN) ? 1 : 0;
        bControl = (GetAsyncKeyState(VK_CONTROL) & ISDOWN) ? 1 : 0;
        bAlt = (GetAsyncKeyState(VK_MENU) & ISDOWN) ? 1 : 0;
    }

    if ( pFilter )
    {
        if ( pFilter->none )
        {
            passed = !bShift && !bControl && !bAlt;
        }
        else if ( pFilter->and )
        {
            passed = (pFilter->shift ? bShift : !bShift) &&
                     (pFilter->control ? bControl : !bControl) &&
                     (pFilter->alt ? bAlt : !bAlt);
        }
        else
        {
            passed = (pFilter->shift && bShift) ||
                     (pFilter->control && bControl) ||
                     (pFilter->alt && bAlt);
        }
    }

    if ( passed )
    {
        RexxThreadContext *c = pSCData->pcpbd->dlgProcContext;

        RexxArrayObject  args = getKeyEventRexxArgs(c, wParam,
                                                    lParam & KEY_ISEXTENDED ? true : false,
                                                    pSCData->pcdc == NULL   ? NULL : pSCData->pcdc->rexxSelf);

        invokeDispatch(c, pSCData->pcpbd, pMethod, args);

        releaseKeyEventRexxArgs(c, args);
    }
}

/**
 * Removes the method at index from the key press data structure.  Assumes that
 * the keyboad hook is unhooked, or that the window subclass is removed.
 *
 */
void removeKeyPressMethod(KEYPRESSDATA *pData, uint32_t index)
{
    uint32_t i;

    if ( index >= 1 && index <= MAX_KEYPRESS_METHODS )
    {
        for ( i = 0; i < COUNT_KEYPRESS_KEYS; i++)
        {
            if ( pData->key[i] == index )
            {
                pData->key[i] = 0;
            }
        }

        if ( pData->pMethods[index] )
        {
            LocalFree(pData->pMethods[index]);
        }
        if ( pData->pFilters[index] )
        {
            LocalFree(pData->pFilters[index]);
        }
        pData->pMethods[index] = NULL;
        pData->pFilters[index] = NULL;
        pData->nextFreeQ[pData->topOfQ] = index;

        /* Theoretically we can't walk off the end of the array, but make sure
         * we don't.
         */
        if ( pData->topOfQ <= MAX_KEYPRESS_METHODS )
        {
            pData->topOfQ++;
        }
        pData->usedMethods--;
    }
}

/**
 * Frees the key press data structure.  Note that methods can be removed leaving
 * holes in the array.
 *
 * @assumes Caller has passed in the correct pointer, i.e., that pSCData's pData
 *          points to a KEYPRESSDATA struct.
 */
void freeKeyPressData(pSubClassData pSCData)
{
    size_t i;
    if ( pSCData )
    {
        KEYPRESSDATA *p = (KEYPRESSDATA *)pSCData->pData;
        for ( i = 1; i <= MAX_KEYPRESS_METHODS; i++ )
        {
            safeLocalFree((void *)p->pMethods[i]);
            safeLocalFree((void *)p->pFilters[i]);
        }
        LocalFree((void *)p);

        if ( pSCData->pcdc != NULL )
        {
            pSCData->pcdc->pKeyPress = NULL;
        }

        LocalFree((void *)pSCData);
    }
}

/**
 * Searches the event method table for a matching method.  Returns 0 if no match
 * is found, otherwise the index into the table for the method.
 *
 */
uint32_t seekKeyPressMethod(KEYPRESSDATA *pData, CSTRING method)
{
    uint32_t i;
    uint32_t index = 0;

    if ( pData )
    {
        for ( i = 1; i <= MAX_KEYPRESS_METHODS; i++)
        {
            if ( pData->pMethods[i] && strcmp(pData->pMethods[i], method) == 0 )
            {
                index = i;
                break;
            }
        }
    }
    return index;
}

/**
 * Parses a key token which could be: a keyword, a single number, or a number
 * range (n-m)  The results are checked to guarentee they are within bounds.
 *
 * Returns true on success, false on error.
 *
 * On true: *pFirst is guarenteed to be non-zero.  If the token represents a
 * single digit, *pLast is guarenteed to be zero.
 *
 * On false: *pFirst and *pLast are undefined.
 */
static BOOL parseKeyToken(PCHAR token, PUINT pFirst, PUINT pLast)
{
    PCHAR  p;
    BOOL   ret = TRUE;

    *pFirst = 0;
    *pLast = 0;

    if ( !strcmp(token, "ALL") )
    {
        *pFirst = VK_LBUTTON;
        *pLast =  VK_OEM_CLEAR;
    }
    else if ( !strcmp(token, "FKEYS") )
    {
        *pFirst = VK_F2;    /* F1 is handled by connectHelp in ooDialog */
        *pLast =  VK_F24;
    }
    else if ( !strcmp(token, "ALPHA") )
    {
        *pFirst = VK_A;
        *pLast =  VK_Z;
    }
    else if ( !strcmp(token, "NUMERIC") )
    {
        *pFirst = VK_0;
        *pLast =  VK_9;
    }
    else if ( !strcmp(token, "ALPHANUMERIC") )
    {
        *pFirst = VK_0;
        *pLast =  VK_Z;
    }
    else if ( (p = strchr(token, '-')) != NULL )
    {
        *p++ = '\0';
        *pFirst = atol(token);
        *pLast = atol(p);
        if ( (! *pFirst || ! *pLast) || (*pFirst > *pLast)       ||
             (*pFirst < VK_LBUTTON)  || (*pFirst > VK_OEM_CLEAR) ||
             (*pLast < VK_LBUTTON)   || (*pLast > VK_OEM_CLEAR)) ret = FALSE;
    }
    else
    {
        *pFirst = atol(token);
        *pLast = 0;
        if ( ! *pFirst || (*pFirst < VK_LBUTTON)   || (*pFirst > VK_OEM_CLEAR) ) ret = FALSE;
    }
    return ret;
}


/**
 *
 * @param pData
 * @param method
 * @param ppMethodName
 *
 * @return keyPressErr_t
 */
static keyPressErr_t kpCheckMethod(KEYPRESSDATA *pData, CSTRING method, char **ppMethodName)
{
    // We need room, a copy of the method name, and an unique method name.
    if ( pData->usedMethods >= (MAX_KEYPRESS_METHODS) )
    {
        return maxMethodsErr;
    }

    char *tmpName = (char *)LocalAlloc(LPTR, strlen(method) + 1);
    if ( tmpName == NULL )
    {
        return memoryErr;
    }

    strcpy(tmpName, method);
    strupr(tmpName);

    if ( seekKeyPressMethod(pData, tmpName) )
    {
        LocalFree(tmpName);
        return dupMethodErr;
    }

    *ppMethodName = tmpName;
    return noErr;
}


static keyPressErr_t kpCheckFilter(CSTRING filter, KEYFILTER **ppFilter)
{
    if ( filter == NULL )
    {
        return noErr;
    }

    KEYFILTER *tmpFilter = (KEYFILTER *)LocalAlloc(LPTR, sizeof(KEYFILTER));
    if ( tmpFilter == NULL )
    {
        return memoryErr;
    }

    if ( StrStrI(filter, "NONE" ) )
    {
        tmpFilter->none = TRUE;
    }
    else
    {
        if ( StrStrI(filter, "AND"    ) ) tmpFilter->and = TRUE;
        if ( StrStrI(filter, "SHIFT"  ) ) tmpFilter->shift = TRUE;
        if ( StrStrI(filter, "CONTROL") ) tmpFilter->control = TRUE;
        if ( StrStrI(filter, "ALT"    ) ) tmpFilter->alt = TRUE;
    }

    if ( StrStrI(filter, "VIRT") )
    {
        tmpFilter->virt = TRUE;
    }

    // Some combinations are not filters, so they are ignored.
    if ( ((! tmpFilter->and) && tmpFilter->shift && tmpFilter->control && tmpFilter->alt) ||
         (tmpFilter->and && ! tmpFilter->shift && ! tmpFilter->control && ! tmpFilter->alt) )
    {
        if ( ! tmpFilter->virt )
        {
            LocalFree(tmpFilter);
            return badFilterErr;
        }
    }

    // Okay, we are good.
    *ppFilter = tmpFilter;
    return noErr;
}

/**
 *  Maps keyboard key(s) to a method.  Each key found in the keys string is set
 *  to index.
 *
 * @param pData  Pointer to the key press data block.
 * @param keys   A string containing the keys the user wants mapped to a single
 *               method.
 * @param index  The index in the method table of the method to be mapped to the
 *               key(s).
 *
 * @return  No error, memory error, or key map error.
 *
 * @remarks  If a token in the keys string is invalid, it is basically ignored
 *           and the rest of the tokens (if any) are parsed.  So, it is easily
 *           possible that some keys are set and some are not.
 */
static keyPressErr_t kpMapKeys(KEYPRESSDATA *pData, CSTRING keys, uint32_t index)
{
    uint32_t firstKey, lastKey;
    char *token = NULL;
    char *str = NULL;
    keyPressErr_t reply = memoryErr;

    str = strdupupr_nospace(keys);
    if ( str != NULL )
    {
        reply = noErr;
        token = strtok(str, ",");
        while( token != NULL )
        {
            if ( parseKeyToken(token, &firstKey, &lastKey) )
            {
                EnterCriticalSection(&crit_sec);
                if ( lastKey )
                {
                    UINT i;
                    for ( i = firstKey; i <= lastKey; i++)
                        pData->key[i] = index;
                }
                else
                {
                    pData->key[firstKey] = index;
                }
                LeaveCriticalSection(&crit_sec);
            }
            else
            {
                reply = keyMapErr;
            }
            token = strtok(NULL, ",");
        }
    }

    safeFree(str);
    return reply;
}

/**
 * Set up the key press data for the specified method.
 *
 * It is possible that a key press comes in to the UI thread while this function
 * is executing:
 *
 * The key will have an already existing index or 0 while the method and filter
 * are being set up.  When a key slot is to be updated with the index of the new
 * method, it is protected by a critical section.
 *
 * setKeyPressData() is called when the subclass or hook is first installed, and
 * it can also be called to just update the data block after the subclass is
 * installed. All errors are reported, but the caller is responsible for
 * determining what to do on error.
 */
keyPressErr_t setKeyPressData(KEYPRESSDATA *pData, CSTRING method, CSTRING keys, CSTRING filter)
{
    char      *pMethod = NULL;
    KEYFILTER *pFilter = NULL;

    keyPressErr_t  result = kpCheckMethod(pData, method, &pMethod);
    if ( result != noErr )
    {
        goto err_out;
    }

    // Only a memory error is fatal, if there is an error with the filter, the
    // filter is simply not set.
    result = kpCheckFilter(filter, &pFilter);
    if ( result == memoryErr )
    {
        goto err_out;
    }

    // In order to map the keys, we need to set the method name into the table
    // and get the index we will use.  So, from here on out, we no longer try to
    // clean up.  Any errors are just ignored, other than returning an error
    // code that will indicate some, or maybe all, of the keys did not get
    // mapped.

    // Get the index of where to put the method.  If the next free queue is not
    // empty, pull the index from the queue, otherwise we are still adding
    // methods sequentially.
    uint32_t index;
    if ( pData->topOfQ )
    {
        index = pData->nextFreeQ[0];
        memmove(pData->nextFreeQ, pData->nextFreeQ + 1, (pData->topOfQ - 1) * sizeof(UINT));
        pData->topOfQ--;
    }
    else
    {
        index = pData->usedMethods + 1;
    }
    pData->pMethods[index] = pMethod;
    pData->usedMethods++;

    if ( pFilter )
    {
        pData->pFilters[index] = pFilter;
    }

    // If there is an error here, it is safe to just ignore it, we jump past
    // err_out. So, pMethods[index] is not null, if the caller decides to abort,
    // pMethod will be freed during clean up.  If the caller decides to not
    // abort, pMethods[index] might not have any keys mapped to it, but that
    // does no harm, the error is reported.  If things are not working for the
    // user, he can check the return codes to find out why.

    result = kpMapKeys(pData, keys, index);
    goto done_out;

err_out:
    safeFree(pMethod);

done_out:
    return result;
}


/** EventNotification::init_eventNotification()
 *
 *  Private method whose sole purpose is to set the cSelf buffer.
 *
 *  This method is invoked from PlainBaseDialog::init() and the
 *  CEventNotification struct is allocated and set up there.
 *
 *  Can not be successfully invoked from Rexx code, by design, because it
 *  requires a Rexx Buffer object as its argument.
 */
RexxMethod1(logical_t, en_init_eventNotification, RexxObjectPtr, cSelf)
{
    RexxMethodContext *c = context;
    if ( ! context->IsBuffer(cSelf) )
    {
        wrongClassException(context->threadContext, 1, "Buffer");
        return FALSE;
    }

    context->SetObjectVariable("CSELF", cSelf);
    return TRUE;
}

/** EventNotification::addUserMessage()
 *
 *  Adds a message to the message table.
 *
 *  Each entry in the message table connects a Windows event message to a method
 *  in a Rexx dialog.  The fields for the entry consist of the Windows message,
 *  the WPARAM and LPARAM for the message, a filter for the message and its
 *  parameters, and the method name. Using the proper filters for the Windows
 *  message and its parameters allows the mapping of a very specific Windows
 *  event to the named method.
 *
 *  @param  methodName   [required]  The method name to be connected.
 *  @param  wm           [required]  The Windows event message
 *  @param  _wmFilter    [optional]  Filter applied to the Windows message.  If
 *                       omitted the filter is 0xFFFFFFFF.
 *  @param  wp           [optional]  WPARAM for the message
 *  @param  _wpFilter    [optional]  Filter applied to the WPARAM.  If omitted a
 *                       filter of all hex Fs is applied
 *  @param  lp           [optional]  LPARAM for the message.
 *  @param  _lpFilter    [optional]  Filter applied to LPARAM.  If omitted the
 *                       filter is all hex Fs.
 *  @param  _willReply   [optional]  The conventional _willReply arg, true,
 *                       false, or sync.
 *
 *  @return  0 on success, 1 on failure.
 *
 *  @note     Method name can not be the empty string. The Window message,
 *            WPARAM, and LPARAM arguments can not all be 0.
 *
 *            If incorrect arguments are detected a syntax condition is raised.
 *
 *  @remarks  All internal use of this method has been removed.  Therefore the
 *            old internal use of the _tag argument, which was never documented,
 *            is no longer needed. We change its use in 4.2.4 to allow the user
 *            to specify will reply or sync. This allows the user to specify
 *            invoke dispatch, invoke direct, or invoke sync
 *
 *            Although it would make more sense to return true on succes and
 *            false on failure, the method has been documented as returning 0
 *            for success and 1 for error.
 *
 *            Then only reason we pass methodName to parseWinMessageFilter() is
 *            to have the function check for the emtpy string.  We use
 *            methodName as is if there is no error.
 */
RexxMethod9(uint32_t, en_addUserMessage, CSTRING, methodName, CSTRING, wm, OPTIONAL_CSTRING, _wmFilter,
            OPTIONAL_RexxObjectPtr, wp, OPTIONAL_CSTRING, _wpFilter, OPTIONAL_RexxObjectPtr, lp, OPTIONAL_CSTRING, _lpFilter,
            OPTIONAL_RexxObjectPtr, _willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;
    uint32_t result = 1;

    WinMessageFilter wmf = {0};
    wmf.method   = methodName;
    wmf._wm       = wm;
    wmf._wmFilter = _wmFilter;
    wmf._wp       = wp;
    wmf._wpFilter = _wpFilter;
    wmf._lp       = lp;
    wmf._lpFilter = _lpFilter;

    if ( ! parseWinMessageFilter(context, &wmf) )
    {
        goto done_out;
    }

    uint32_t tag = _willReplyToTag(context, _willReply, false, 8);
    if ( tag == TAG_INVALID )
    {
        goto done_out;
    }

    tag |= TAG_USERADDED;

    bool success;
    if ( (wmf.wm & wmf.wmFilter) == WM_COMMAND )
    {
        success = addCommandMessage(pcen, context, wmf.wp, wmf.wpFilter, wmf.lp, wmf.lpFilter, methodName, tag);
    }
    else if ( (wmf.wm & wmf.wmFilter) == WM_NOTIFY )
    {
        success = addNotifyMessage(pcen, context, wmf.wp, wmf.wpFilter, wmf.lp, wmf.lpFilter, methodName, tag);
    }
    else
    {
        success = addMiscMessage(pcen, context, wmf.wm, wmf.wmFilter, wmf.wp, wmf.wpFilter, wmf.lp, wmf.lpFilter, methodName, tag);
    }

    result = (success ? 0 : 1);

done_out:
    return result;
}

RexxMethod4(int32_t, en_connectKeyPress, CSTRING, methodName, CSTRING, keys, OPTIONAL_CSTRING, filter, CSELF, pCSelf)
{
    keyPressErr_t result = connectKeyPressHook(context, (pCEventNotification)pCSelf, methodName, keys, filter);
    if ( result == memoryErr )
    {
        outOfMemoryException(context->threadContext);
    }
    return -(int32_t)result;
}


RexxMethod2(int32_t, en_connectFKeyPress, CSTRING, methodName, CSELF, pCSelf)
{
    keyPressErr_t result = connectKeyPressHook(context, (pCEventNotification)pCSelf, methodName, "FKEYS", NULL);
    if ( result == memoryErr )
    {
        outOfMemoryException(context->threadContext);
    }
    return -(int32_t)result;
}


RexxMethod2(int32_t, en_disconnectKeyPress, OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;
    keyPressErr_t result = nameErr;
    char *tmpName = NULL;

    if ( pcen->hDlg == NULL || ! IsWindow(pcen->hDlg) )
    {
        noWindowsDialogException(context, pcen->rexxSelf);
        goto done_out;
    }

    if ( pcen->hHook && pcen->pHookData )
    {
        pSubClassData pSCData = (pSubClassData)pcen->pHookData;
        KEYPRESSDATA *pKeyPressData = (KEYPRESSDATA *)pSCData->pData;

        // If there is no methodName argument, remove the entire hook, otherwise
        // disconnect the named method.
        if ( argumentOmitted(1) )
        {
            removeKBHook(pcen);
            result = noErr;
        }
        else
        {
            tmpName = strdupupr(methodName);
            if ( tmpName == NULL )
            {
                result = memoryErr;
                goto done_out;
            }
            uint32_t index = seekKeyPressMethod(pKeyPressData, tmpName);
            if ( index == 0 )
            {
                result = nameErr;
                goto done_out;
            }

            // If there is only 1 method connected to the hook, remove the hook
            // completely.  Otherwise, unhook the hook, fix up the key press
            //data, and reset the hook.
            if ( pKeyPressData->usedMethods == 1 )
            {
                removeKBHook(pcen);
                result = noErr;
            }
            else
            {
                UnhookWindowsHookEx(pcen->hHook);
                removeKeyPressMethod(pKeyPressData, index);

                // If setKBHook fails, it frees all memory.
                result = setKBHook(pcen);

                if ( result != noErr )
                {
                    pcen->pHookData = NULL;
                }
            }
        }
    }

done_out:
    safeFree(tmpName);
    return -(int32_t)result;
}


RexxMethod2(logical_t, en_hasKeyPressConnection, OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    if ( pcen->hHook == NULL )
    {
        return FALSE;
    }

    if ( argumentOmitted(1) )
    {
        return TRUE;
    }

    char *tmpName = strdupupr(methodName);
    if ( tmpName == NULL )
    {
        return FALSE;
    }

    pSubClassData pSCData = (pSubClassData)pcen->pHookData;

    BOOL exists = (seekKeyPressMethod((KEYPRESSDATA *)pSCData->pData, tmpName) > 0);
    free(tmpName);
    return exists;
}

/** EventNotification::connectButtonEvent()
 *
 *
 */
RexxMethod5(RexxObjectPtr, en_connectButtonEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return TheNegativeOneObj;
    }

    uint32_t tag = _willReplyToTag(context, willReply, false, 4);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }

    uint32_t notificationCode;
    if ( ! keyword2bcn(context, event, &notificationCode) )
    {
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = bcn2name(notificationCode);
    }
    if ( notificationCode == BN_CLICKED && (id < 3 || id == 9) )
    {
        // Already connected.
        return TheZeroObj;
    }

    if ( (notificationCode == BCN_HOTITEMCHANGE || notificationCode == BCN_DROPDOWN) )
    {
        tag |= TAG_BUTTON;
        if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
        {
            return TheZeroObj;
        }
    }
    else
    {
        if ( addCommandMessage(pcen, context, MAKEWPARAM(id, notificationCode), 0xFFFFFFFF, 0, 0, methodName, tag) )
        {
            return TheZeroObj;
        }
    }

    return TheOneObj;
}

/** EventNotification::connectComboBoxEvent()
 *
 *
 */
RexxMethod5(RexxObjectPtr, en_connectComboBoxEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return TheNegativeOneObj;
    }

    uint32_t tag = _willReplyToTag(context, willReply, false, 4);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }

    uint32_t notificationCode;

    if ( ! keyword2cbn(context, event, &notificationCode) )
    {
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = cbn2name(notificationCode);
    }

    if ( addCommandMessage(pcen, context, MAKEWPARAM(id, notificationCode), 0xFFFFFFFF, 0, 0, methodName, tag) )
    {
        return TheZeroObj;
    }

    return TheOneObj;
}

/** EventNotification::connectCommandEvents()
 *
 *  Connects a Rexx dialog method to all the WM_COMMAND event notifications sent
 *  by a Windows dialog control to its parent.
 *
 *  The number of different notifications and the meanings of the notifications
 *  are dependent on the type of dialog control specified.  Therefore, it is
 *  more advisable to use the connectXXXEvent() method for the specific control.
 *
 *  @param  rxID        The resource ID of the dialog control.  Can be numeric
 *                      or symbolic.
 *  @param  methodName  The name of the method to be invoked in the Rexx dialog
 *                      when this event occurs.  The method name can not be the
 *                      empty string.  If the programmer does not provide a
 *                      matching method in the Rexx dialog, a syntax condition
 *                      will be raised if this event happens.
 *
 *  @return 0 on success, -1 for a resource ID error and 1 for other errors.
 *
 *  @details  Two arguments are sent to methodName.  The first contains the
 *            notification code in the high word and the ID of the control in
 *            the low word.  The second argument is the window handle of the
 *            control.
 */
RexxMethod4(RexxObjectPtr, en_connectCommandEvents, RexxObjectPtr, rxID, CSTRING, methodName,
            OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return TheNegativeOneObj;
    }
    if ( *methodName == '\0' )
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_null, TheTwoObj);
        return TheOneObj;
    }
    uint32_t tag = _willReplyToTag(context, willReply, false, 3);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }

    return (addCommandMessage(pcen, context, id, 0x0000FFFF, 0, 0, methodName, tag) ? TheZeroObj : TheOneObj);
}

/** EventNotification::connectDateTimePickerEvent()
 *
 *  Connects a Rexx dialog method with a date time picker control event.
 *
 *  @param  rxID        The resource ID of the dialog control.  Can be numeric
 *                      or symbolic.
 *
 *  @param  event       Keyword specifying which event to connect.  Keywords at
 *                      this time:
 *
 *                      CLOSEUP
 *                      DATETIMECHANGE
 *                      DROPDOWN
 *                      FORMAT
 *                      FORMATQUERY
 *                      USERSTRING
 *                      KEYDOWN
 *                      KILLFOCUS
 *                      SETFOCUS
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted then the
 *                      method name is constructed by prefixing the event
 *                      keyword with 'on'.  For instance onUserString.
 *
 *  @param  willReply   [OPTIONAL] Specifies if the method invocation should be
 *                      direct or indirect. With a direct invocation, the
 *                      interpreter waits in the Windows message loop for the
 *                      return from the Rexx method. With indirect, the Rexx
 *                      method is invoked through ~startWith(), which of course
 *                      returns immediately. The default is true, i.e. the Rexx
 *                      programmer is always expected to reply.
 *
 *  @note   If a symbolic ID is  used and it can not be resolved to a numeric
 *          number an exception is raised.
 *
 *          willReply is ignored for USERSTRING, KEYDOWN, FORMAT, and
 *          FORMATQUERY, the programmer must always reply in the event handler
 *          for those events.
 *
 *  @remarks  This method is new since the 4.0.0 release, therefore an exception
 *            is raised for a bad resource ID rather than returning -1.
 */
RexxMethod5(RexxObjectPtr, en_connectDateTimePickerEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, _willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id = oodResolveSymbolicID(context->threadContext, pcen->rexxSelf, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto err_out;
    }

    uint32_t notificationCode;
    if ( ! keyword2dtpn(context, event, &notificationCode) )
    {
        goto err_out;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = dtpn2name(notificationCode);
    }

    uint32_t tag = TAG_DATETIMEPICKER;

    if ( isMustReplyDtpn(notificationCode) )
    {
        tag |= TAG_REPLYFROMREXX;
    }
    else
    {
        tag |= _willReplyToTag(context, _willReply, true, 4);
        if ( (tag & TAG_INVALID) == TAG_INVALID )
        {
            goto err_out;
        }
    }

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
}

/** EventNotification::connectDraw()
 *
 * @remarks  This is the original ooDialog implementation, plus willReply. We
 *           now send the DRAWITEMSTRUCT values to the event handler.
 */
RexxMethod4(RexxObjectPtr, en_connectDraw, OPTIONAL_RexxObjectPtr, rxID, OPTIONAL_CSTRING, methodName,
            OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id = 0;
    if ( argumentExists(1) )
    {
        // We need to allow 0 here, but not -1.
        if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, false) || id == -1 )
        {
            return TheNegativeOneObj;
        }
    }

    uint32_t tag = _willReplyToTag(context, willReply, false, 3);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(2) || *methodName == '\0' )
    {
        methodName = "onDraw";
    }

    if ( id == 0 )
    {
        if ( addMiscMessage(pcen, context, WM_DRAWITEM, 0xFFFFFFFF, 0, 0, 0, 0, methodName, tag) )
        {
            return TheZeroObj;
        }
    }
    else
    {
        if ( addMiscMessage(pcen, context, WM_DRAWITEM, 0xFFFFFFFF, MAKEWPARAM(id, 0), 0xFFFFFFFF, 0, 0, methodName, tag) )
        {
            return TheZeroObj;
        }
    }

    return TheOneObj;
}

/** EventNotification::connectEditEvent()
 *
 *
 */
RexxMethod5(RexxObjectPtr, en_connectEditEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return TheNegativeOneObj;
    }

    uint32_t tag = _willReplyToTag(context, willReply, false, 4);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }

    uint32_t notificationCode;
    if ( ! keyword2en(context, event, &notificationCode) )
    {
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = en2name(notificationCode);
    }

    if ( addCommandMessage(pcen, context, MAKEWPARAM(id, notificationCode), 0xFFFFFFFF, 0, 0, methodName, tag) )
    {
        return TheZeroObj;
    }

    return TheOneObj;
}

/** EventNotification::connectListBoxEvent()
 *
 *
 *  @note  The KEYDOWN event has never been documented and in the original
 *         implementation connects WM_VKEYTOITEM, for which the reply is
 *         significant.  For now, we follow the original implmentation which
 *         would have just sent wParam, lParam as args 1 and 2 and no reply was
 *         possible.  This would be an event that could use an enhancement, but
 *         for now, we leave the implementation alone and do not document the
 *         keyword.  TODO.
 */
RexxMethod5(RexxObjectPtr, en_connectListBoxEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return TheNegativeOneObj;
    }

    uint32_t tag = _willReplyToTag(context, willReply, false, 4);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }

    uint32_t notificationCode;
    if ( ! keyword2lbn(context, event, &notificationCode) )
    {
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = lbn2name(notificationCode);
    }

    if ( notificationCode == LBN_VKEYTOITEM )
    {
        // tag |= TAG_???;  Will probably want an extra tag here.
        HWND hListBox = GetDlgItem(pcen->hDlg, id);  // Add exception if no handle ?
        if ( hListBox != NULL )
        {
            if ( addMiscMessage(pcen, context, WM_VKEYTOITEM, 0xFFFFFFFF, 0, 0, (LPARAM)hListBox, UINTPTR_MAX, methodName, tag) )
            {
                return TheZeroObj;
            }
        }
    }
    else
    {
        if ( addCommandMessage(pcen, context, MAKEWPARAM(id, notificationCode), 0xFFFFFFFF, 0, 0, methodName, tag) )
        {
            return TheZeroObj;
        }
    }

    return TheOneObj;
}

/** EventNotification::connectListViewEvent()
 *
 *  Connects a Rexx dialog method with a list view event.
 *
 *  @param  rxID        The resource ID of the dialog control.  Can be numeric
 *                      or symbolic.
 *
 *  @param  event       Keyword specifying which event to connect.  Keywords at
 *                      this time:
 *
 *                      ACTIVATE
 *                      BEGINDRAG
 *                      BEGINRDRAG
 *                      BEGINEDIT
 *                      BEGINSCROLL
 *                      CHANGED
 *                      CHANGING
 *                      COLUMNCLICK
 *                      DEFAULTEDIT
 *                      DELETE
 *                      DELETEALL
 *                      ENDEDIT
 *                      ENDSCROLL
 *                      INSERTED
 *                      KEYDOWN
 *
 *                      CHECKBOXCHANGED
 *                      CLICK
 *                      FOCUSCHANGED
 *                      SELECTCHANGED
 *                      SELECTFOCUS
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted then the
 *                      method name is constructed by prefixing the event
 *                      keyword with 'on'.  For instance onUserString.
 *
 *  @param  willReply   [OPTIONAL] Specifies if the method invocation should be
 *                      direct or indirect. With a direct invocation, the
 *                      interpreter waits in the Windows message loop for the
 *                      return from the Rexx method. With indirect, the Rexx
 *                      method is invoked through ~startWith(), which of course
 *                      returns immediately.
 *
 *                      For list views, at this time, the default is false, i.e.
 *                      the Rexx programmer needs to specify that she wants to
 *                      reply.  This could change if new key words are added.
 *
 *  @return 0 for no error, -1 for a bad resource ID or incorrect event keyword,
 *          1 if the event could not be connected.  The event can not be
 *          connected if there is a problem with the message table, full or out
 *          of memory error.
 *
 *  @remarks   For the current keywords, if a symbolic ID is  used and it can
 *             not be resolved to a numeric number -1 has to be returned for
 *             backwards compatibility.  Essentially, for this method, all
 *             behaviour needs to be pre-4.2.0.  The only change is that for
 *             tagged list view events, the user can specify to reply directly.
 *
 *             The processing for beginlabeledit and endlabeledit that was done
 *             for the DEFAULTEDIT keyword is not needed for a list-view.
 *             defListEditStarter and defListEditHandler methods are not needed
 *             and the methods are removed from the list-view.  For backwards
 *             compatibility, if the keyword DEFAULTEDIT, we only connect the
 *             defListEditHandler.  We need that to catch the message.  A tag is
 *             added for preserve old behavior and within processLVN() we simply
 *             do what the old defListEditHandler did.  Set the label text if
 *             the user did not cancel, don't set the label if the user did
 *             cancel.
 *
 *             For reference.  The arguments sent to the event handler for
 *             LVN_ENDLABELEDIT were never documented correctly, if at all.
 *             They were as follows.  If the user did *not* cancel the edit:
 *               arg 1 list-view id                   (from wParam)
 *               arg 2 item being edited id (0 based)
 *               arg 3 text user entered.
 *
 *             If the user did cancel the edit:
 *               arg 1 list-view id                   (from wParam)
 *               arg 2 pointer to the NMLVDISPINFO struct as a decimal value
 *                                                    (from lParam)
 *
 *             Note: it is highly unlikely that anyone ever connected
 *             LVN_ENDLABELEDIT in the old ooDialog, but if they did, the
 *             willReply argument would be omitted.  We do a special check for
 *             this and preserve what would have been the old behaviour.  That
 *             is: use invoke dispatch and use the arguments listed above.
 *
 *             The arguments to the event handler for LVN_BEGINLABELEDIT were
 *             never documented at all.  For reference they were:
 *               arg 1 list-view id                   (from wParam)
 *               arg 2 pointer to the NMLVDISPINFO struct as a decimal value
 *                                                    (from lParam)
 */
RexxMethod5(RexxObjectPtr, en_connectListViewEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return TheNegativeOneObj;
    }

    uint32_t tag = 0;
    bool     isDefEdit = false;
    uint32_t notificationCode;

    if ( ! keyword2lvn(context, event, &notificationCode, &tag, &isDefEdit, willReply) )
    {
        return TheNegativeOneObj;
    }

    // Deal with DEFAULTEDIT separately.
    if ( isDefEdit )
    {
        if ( ! addNotifyMessage(pcen, context, id, 0xFFFFFFFF, LVN_BEGINLABELEDIT, 0xFFFFFFFF, "DefListEditStarter", tag) )
        {
            return TheNegativeOneObj;
        }
        if ( ! addNotifyMessage(pcen, context, id, 0xFFFFFFFF, LVN_ENDLABELEDIT, 0xFFFFFFFF, "DefListEditHandler", tag) )
        {
            return TheNegativeOneObj;
        }
        return TheZeroObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = lvn2name(notificationCode, tag);
    }

    if ( (notificationCode == LVN_BEGINLABELEDIT || notificationCode == LVN_ENDLABELEDIT) && argumentOmitted(4) )
    {
        tag |= TAG_PRESERVE_OLD;
    }

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheZeroObj;
    }

    return TheOneObj;
}


/** EventNotification::connectMonthCalendarEvent()
 *
 *  Connects a Rexx dialog method with a month calendar control event.
 *
 *  @param  rxID        The resource ID of the dialog control.  Can be numeric
 *                      or symbolic.
 *
 *  @param  event       Keyword specifying which event to connect.  Keywords at
 *                      this time:
 *
 *                      GETDAYSTATE
 *                      SELCHANGE
 *                      SELECT
 *                      VIEWCHANGE
 *                      RELEASED
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted then the
 *                      method name is constructed by prefixing the event
 *                      keyword with 'on'.  For instance onGetDayState.
 *
 *  @param  willReply   [OPTIONAL] Specifies if the method invocation should be
 *                      direct or indirect. With a direct invocation, the
 *                      interpreter waits in the Windows message loop for the
 *                      return from the Rexx method. With indirect, the Rexx
 *                      method is invoked through ~startWith(), which of course
 *                      returns immediately.
 *
 *  @return  True if the event notification was connected, otherwsie false.
 *
 *  @note   If a symbolic ID is  used and it can not be resolved to a numeric
 *          number an exception is raised.
 *
 *  @remarks  This method is new since the 4.0.0 release, therefore an exception
 *            is raised for a bad resource ID rather than returning -1.
 *
 *            For controls new since 4.0.0, event notifications that have a
 *            reply are documented as always being 'direct' reply and
 *            notifications that ignore the return are documented as allowing
 *            the programmer to specify.  This means that willReply is ignored
 *            for MCN_GETDAYSTATE and not ignored for all other notifications.
 */
RexxMethod5(RexxObjectPtr, en_connectMonthCalendarEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, _willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id = oodResolveSymbolicID(context->threadContext, pcen->rexxSelf, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto err_out;
    }

    uint32_t notificationCode;
    if ( ! keyword2mcn(context, event, &notificationCode) )
    {
        goto err_out;
    }
    if ( notificationCode == MCN_VIEWCHANGE && ! requiredOS(context, Vista_OS, "ViewChange notification", "Vista") )
    {
        goto err_out;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = mcn2name(notificationCode);
    }

    uint32_t tag = 0;
    if ( notificationCode == MCN_GETDAYSTATE )
    {
        tag = TAG_REPLYFROMREXX;
    }
    else
    {
        tag = _willReplyToTag(context, _willReply, true, 4);
        if ( tag == TAG_INVALID )
        {
            goto err_out;
        }
    }
    tag |= TAG_MONTHCALENDAR;

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
}

/** EventNotification::connectNotifyEvent()
 *
 *
 */
RexxMethod5(RexxObjectPtr, en_connectNotifyEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return TheNegativeOneObj;
    }

    uint32_t tag = _willReplyToTag(context, willReply, false, 4);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }

    uint32_t notificationCode;
    if ( ! keyword2nm(context, event, &notificationCode) )
    {
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = nm2name(notificationCode);
    }

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheZeroObj;
    }

    return TheOneObj;
}

/** EventNotification::connectRebarEvent()
 *
 *  Connects a Rexx dialog method with a rebar control event.
 *
 *  @param  rxID        The resource ID of the dialog control.  Can be numeric
 *                      or symbolic.
 *
 *  @param  event       Keyword specifying which event to connect.  Keywords at
 *                      this time:
 *
 *                      AUTOBREAK
 *
 *
 *
 *
 *
 *
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted then the
 *                      method name is constructed by prefixing the event
 *                      keyword with 'on'.  For instance onAutoBreak.
 *
 *  @param  willReply   [OPTIONAL] Specifies if the method invocation should be
 *                      direct or indirect. With a direct invocation, the
 *                      interpreter waits in the Windows message loop for the
 *                      return from the Rexx method. With indirect, the Rexx
 *                      method is invoked through ~startWith(), which of course
 *                      returns immediately.  By default willReply is set to
 *                      true.
 *
 *  @return  True if the event notification was connected, otherwsie false.
 *
 *  @note   If a symbolic ID is  used and it can not be resolved to a numeric
 *          number an exception is raised.
 *
 *  @remarks  This method is new since the 4.0.0 release, therefore an exception
 *            is raised for a bad resource ID rather than returning -1.
 *
 *            For controls new since 4.0.0, event notifications that have a
 *            reply are documented as always being 'direct' reply and
 *            notifications that ignore the return are documented as allowing
 *            the programmer to specify.  This means that willReply is ignored
 *            for RBN_MINMAX and not ignored for RBN_LAYOUTCHANGED, for
 *            example.
 */
RexxMethod5(RexxObjectPtr, en_connectReBarEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, _willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id = oodResolveSymbolicID(context->threadContext, pcen->rexxSelf, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto err_out;
    }

    uint32_t notificationCode;
    if ( ! keyword2rbn(context, event, &notificationCode) )
    {
        goto err_out;
    }
    if ( notificationCode == RBN_SPLITTERDRAG && ! requiredOS(context, Vista_OS, "Splitter notification", "Vista") )
    {
        goto err_out;
    }
    if ( notificationCode == RBN_AUTOBREAK && ! requiredComCtl32Version(context, COMCTL32_6_0, "AutoBreak notification") )
    {
        goto err_out;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = rbn2name(notificationCode);
    }

    uint32_t tag = 0;
    if ( isMustReplyRbn(notificationCode) )
    {
        tag = TAG_REPLYFROMREXX;
    }
    else
    {
        tag = _willReplyToTag(context, _willReply, true, 4);
        if ( tag == TAG_INVALID )
        {
            goto err_out;
        }
    }
    tag |= TAG_REBAR;

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
}

/** EventNotification::connectScrollBarEvent()
 *
 *  Connects a Rexx dialog method with a scroll bar event.
 *
 *  @param  rxID        The resource ID of the dialog control.  Can be numeric
 *                      or symbolic.
 *
 *  @param  event       Keyword specifying which event to connect.  Keywords at
 *                      this time:
 *
 *                      KeyWord      Windows Code    Notes
 *                      -----------------------------------
 *                      UP           0               Old word, confusing.
 *                      LINEUP       0
 *                      LINELEFT     0
 *                      DOWN         1               Old word, confusing.
 *                      LINEDOWN     1
 *                      LINERIGHT    1
 *                      PAGEUP       2
 *                      PAGELEFT     2
 *                      PAGEDOWN     3
 *                      PAGERIGHT    3
 *                      POSITION     4
 *                      DRAG         5
 *                      TOP          6
 *                      LEFT         6
 *                      BOTTOM       7
 *                      RIGHT        7
 *                      ENDSCROLL    8
 *
 *
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted or the empty
 *                      string then the method name is constructed by prefixing
 *                      the event keyword with 'on'.  For instance onPageDown.
 *
 *  @param  willReply   [OPTIONAL] Specifies if the method invocation should be
 *                      direct or indirect. With a direct invocation, the
 *                      interpreter waits in the Windows message loop for the
 *                      return from the Rexx method. With indirect, the Rexx
 *                      method is invoked through ~startWith(), which of course
 *                      returns immediately.
 *
 *                      For scroll bars, at this time, the default is false,
 *                      i.e. the Rexx programmer needs to specify that she wants
 *                      to reply.  This could change if new key words are added.
 *
 *  @return 0 for no error, -1 for a bad resource ID or incorrect event keyword,
 *          1 if the event could not be connected, or other errors.  The event
 *          can not be connected if there is a problem with the message table,
 *          full or out of memory error.
 *
 *  @note  Because this method requires the window handle of the scroll bar
 *         control, it can only be invoked after the underlying dialog has been
 *         created.  This is enforced by raising a syntax condition if needed.
 *
 *         This method does not distiguish between vertical and horizontal
 *         scroll bars because Windows uses the same values for the up and left
 *         flags, and the same values for the down and right flags.  I.e., if
 *         the event is LINEUP and the scroll bar is a vertical scroll bar, it
 *         means the user scrolled 1 unit up, but if the scroll bar is a
 *         horizontal scroll bar it means the user scrolled 1 unit to the left.
 *
 *         Therefore, UP, LINEUP, and LINELEFT all mean the same thing and can
 *         be used with either horizontal or vertical scroll bars.  The same
 *         thing is true for: DOWN, LINEDOWN, and LINERIGHT, for: TOP, LEFT, and
 *         for: BOTTOM, RIGHT.
 *
 *         Raises syntax conditions if incorrect arguments are detected.  Sets
 *         the .SystemErrorCode.
 *
 *         In addition to scroll bar controls, scroll bars can be added to any
 *         window by simply using the WS_VSCROLL or WS_HSCROLL styles to the
 *         window. In this case there is no resource ID for the scroll bar, or
 *         window handle.  We've added support here to connect the event by
 *         using 0 for rxID, but there is still no support in ooDialog for using
 *         the scroll bar methods in this case.  To be added later after 4.2.0
 *         is released.
 *
 *  @remarks   For the current keywords, if a symbolic ID is  used and it can
 *             not be resolved to a numeric number -1 has to be returned for
 *             backwards compatibility.  Essentially, for this method, all
 *             behaviour needs to be pre-4.2.0.  The only change is that the
 *             user can specify to reply directly.
 */
RexxMethod5(RexxObjectPtr, en_connectScrollBarEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCEventNotification pcen = (pCEventNotification)pCSelf;

    if ( pcen->hDlg == NULL )
    {
        return noWindowsDialogException(context, pcen->rexxSelf);
    }

    int32_t id;
    if ( ! context->ObjectToInt32(rxID, &id) )
    {
        if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
        {
            return TheNegativeOneObj;
        }
    }

    uint32_t notificationCode;
    if ( ! keyword2scbn(event, &notificationCode) )
    {
        return TheNegativeOneObj;
    }

    HWND hCtrl = GetDlgItem(pcen->hDlg, id);
    if ( id != 0 && hCtrl == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = scbn2name(notificationCode);
    }

    uint32_t tag = _willReplyToTag(context, willReply, false, 4);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }

    if ( addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, notificationCode, 0x0000FFFF, (LPARAM)hCtrl,
                        UINTPTR_MAX, methodName, tag) )
    {
        if ( addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, notificationCode, 0x0000FFFF, (LPARAM)hCtrl,
                            UINTPTR_MAX, methodName, tag) )
        {
            return TheZeroObj;
        }
    }

    return TheOneObj;
}

/** EventNotification::connectAllScrollBarEvents()
 *
 *  Connects all scroll bar events to the method specified.
 *
 * @param  rxID      [required]
 * @param  methName  [required]
 * @param  min       [optional]
 * @param  max       [optional]
 * @param  pos       [optional]
 */
RexxMethod7(RexxObjectPtr, en_connectAllSBEvents, RexxObjectPtr, rxID, CSTRING, msg,
            OPTIONAL_int32_t, min, OPTIONAL_int32_t, max, OPTIONAL_int32_t, pos,
            OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCEventNotification pcen = (pCEventNotification)pCSelf;

    if ( pcen->hDlg == NULL )
    {
        return noWindowsDialogException(context, pcen->rexxSelf);
    }

    int32_t id;
    if ( ! context->ObjectToInt32(rxID, &id) )
    {
        if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
        {
            return TheNegativeOneObj;
        }
    }

    HWND hCtrl = GetDlgItem(pcen->hDlg, id);
    if ( id != 0 && hCtrl == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return TheNegativeOneObj;
    }

    if ( *msg == '\0' )
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_null, context->WholeNumber(2));
        goto err_out;
    }

    uint32_t tag = _willReplyToTag(context, willReply, false, 6);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }

    if ( (argumentExists(3) && argumentExists(4)) || argumentExists(5) )
    {
        SCROLLINFO si = {0};

        si.cbSize = sizeof(si);
        if ( argumentExists(3) && argumentExists(4) )
        {
            si.fMask = SIF_RANGE;
            si.nMin = min;
            si.nMax = max;
        }
        if ( argumentExists(5) )
        {
            si.fMask |= SIF_POS;
            si.nPos = pos;
        }
        SetScrollInfo(hCtrl, SB_CTL, &si, TRUE);
    }

    bool ok;

    ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_LINEUP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);
    ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_LINEUP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);

    ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_LINEDOWN, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);
    ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_LINEDOWN, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);

    if ( ! ok )
    {
        goto err_out;
    }

    ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_PAGEUP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);
    ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_PAGEUP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);

    ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_PAGEDOWN, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);
    ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_PAGEDOWN, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);

    if ( ! ok )
    {
        goto err_out;
    }

    ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_TOP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);
    ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_TOP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);

    ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_BOTTOM, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);
    ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_BOTTOM, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);

    if ( ! ok )
    {
        goto err_out;
    }

    ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_THUMBPOSITION, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);
    ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_THUMBPOSITION, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);

    ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_THUMBTRACK, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);
    ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_THUMBTRACK, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);

    ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_ENDSCROLL, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);
    ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_ENDSCROLL, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, msg, tag);

    if ( ! ok )
    {
        goto err_out;
    }

    return TheZeroObj;

err_out:
    return TheOneObj;
}

/** EventNotification::connectEachScrollBarEvent()
 *
 *  Connects the LINEUP and LINEDOWN scroll bar events to the methods specified.
 *  In addition, for each optional method specified, will connect the specified
 *  event.
 *
 *  Arguments. Only the first 3 are required:
 *
 *  use arg id, progUp, progDn, progPos, min, max, pos, progPgUp, progPgDn,
 *          progTop, progBottom, progTrack, progEndSc, willReply
 *
 *
 * @param rxID
 * @param methUp
 * @param methDown
 * @param methPos
 * @param min
 * @param max
 * @param pos
 * @param methPgUp
 * @param methPgDown
 * @param methTop
 * @param methBottom
 * @param methTrack
 * @param methEndScroll
 * @param willReply
 */
RexxMethod10(RexxObjectPtr, en_connectEachSBEvent, RexxObjectPtr, rxID, CSTRING, methUp,
             CSTRING, methDown, OPTIONAL_CSTRING, methPos, OPTIONAL_int32_t, min, OPTIONAL_int32_t, max,
             OPTIONAL_int32_t, pos, OPTIONAL_CSTRING, methPgUp, ARGLIST, args, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    oodResetSysErrCode(context->threadContext);

    pCEventNotification pcen = (pCEventNotification)pCSelf;

    if ( pcen->hDlg == NULL )
    {
        return noWindowsDialogException(context, pcen->rexxSelf);
    }

    int32_t id;
    if ( ! context->ObjectToInt32(rxID, &id) )
    {
        if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
        {
            return TheNegativeOneObj;
        }
    }

    HWND hCtrl = GetDlgItem(pcen->hDlg, id);
    if ( id != 0 && hCtrl == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return TheNegativeOneObj;
    }

    if ( *methUp == '\0' )
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_null, c->WholeNumber(2));
        goto err_out;
    }
    if ( *methDown == '\0' )
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_null, c->WholeNumber(3));
        goto err_out;
    }

    RexxObjectPtr _willReply = c->ArrayAt(args, 14);
    uint32_t      tag        = _willReplyToTag(context, _willReply, false, 14);
    if ( tag == TAG_INVALID )
    {
        goto err_out;
    }

    if ( (argumentExists(5) && argumentExists(6)) || argumentExists(7) )
    {
        SCROLLINFO si = {0};

        si.cbSize = sizeof(si);
        if ( argumentExists(5) && argumentExists(6) )
        {
            si.fMask = SIF_RANGE;
            si.nMin = min;
            si.nMax = max;
        }
        if ( argumentExists(7) )
        {
            si.fMask |= SIF_POS;
            si.nPos = pos;
        }
        SetScrollInfo(hCtrl, SB_CTL, &si, TRUE);
    }

    bool ok;

    ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_LINEUP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, methUp, tag);
    ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_LINEUP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, methUp, tag);

    ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_LINEDOWN, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, methDown, tag);
    ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_LINEDOWN, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, methDown, tag);

    if ( ! ok )
    {
        goto err_out;
    }

    if ( argumentExists(4) )
    {
        if ( *methPos == '\0' )
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_null, c->WholeNumber(4));
            goto err_out;
        }

        ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_THUMBPOSITION, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, methPos, tag);
        ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_THUMBPOSITION, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, methPos, tag);

        if ( ! ok )
        {
            goto err_out;
        }
    }
    if ( argumentExists(8) )
    {
        if ( *methPgUp == '\0' )
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_null, c->WholeNumber(8));
            goto err_out;
        }

        ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_PAGEUP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, methPgUp, tag);
        ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_PAGEUP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, methPgUp, tag);

        if ( ! ok )
        {
            goto err_out;
        }
    }

    // We can not use argumentExists() for arguments in ARGLIST, only for named
    // arguments. So for each arg, we check for its presence in the arg array.

    RexxObjectPtr _methName = c->ArrayAt(args, 9);
    if ( _methName != NULLOBJECT )
    {
        CSTRING meth = c->ObjectToStringValue(_methName);

        if ( meth == NULLOBJECT || *meth == '\0' )
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_null, c->WholeNumber(9));
            goto err_out;
        }

        ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_PAGEDOWN, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, meth, tag);
        ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_PAGEDOWN, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, meth, tag);

        if ( ! ok )
        {
            goto err_out;
        }
    }

    _methName = c->ArrayAt(args, 10);
    if ( _methName != NULLOBJECT )
    {
        CSTRING meth = c->ObjectToStringValue(_methName);

        if ( meth == NULLOBJECT || *meth == '\0' )
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_null, c->WholeNumber(10));
            goto err_out;
        }

        ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_TOP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, meth, tag);
        ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_TOP, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, meth, tag);

        if ( ! ok )
        {
            goto err_out;
        }
    }

    _methName = context->ArrayAt(args, 11);
    if ( _methName != NULLOBJECT )
    {
        CSTRING meth = context->ObjectToStringValue(_methName);

        if ( meth == NULLOBJECT || *meth == '\0' )
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_null, c->WholeNumber(11));
            goto err_out;
        }

        ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_BOTTOM, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, meth, tag);
        ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_BOTTOM, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, meth, tag);

        if ( ! ok )
        {
            goto err_out;
        }
    }

    _methName = context->ArrayAt(args, 12);
    if ( _methName != NULLOBJECT )
    {
        CSTRING meth = context->ObjectToStringValue(_methName);

        if ( meth == NULLOBJECT || *meth == '\0' )
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_null, c->WholeNumber(12));
            goto err_out;
        }

        ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_THUMBTRACK, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, meth, tag);
        ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_THUMBTRACK, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, meth, tag);

        if ( ! ok )
        {
            goto err_out;
        }
    }

    _methName = context->ArrayAt(args, 13);
    if ( _methName != NULLOBJECT )
    {
        CSTRING meth = context->ObjectToStringValue(_methName);

        if ( meth == NULLOBJECT || *meth == '\0' )
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_null, c->WholeNumber(13));
            goto err_out;
        }

        ok = addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, SB_ENDSCROLL, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, meth, tag);
        ok = addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, SB_ENDSCROLL, 0x0000FFFF, (LPARAM)hCtrl, UINTPTR_MAX, meth, tag);

        if ( ! ok )
        {
            goto err_out;
        }
    }

    return TheZeroObj;

err_out:
    return TheOneObj;
}

/** EventNotification::connectStaticEvent()
 *
 *
 */
RexxMethod5(RexxObjectPtr, en_connectStaticEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return TheNegativeOneObj;
    }

    uint32_t tag = _willReplyToTag(context, willReply, false, 4);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }

    uint32_t notificationCode;
    if ( ! keyword2stn(context, event, &notificationCode) )
    {
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = stn2name(notificationCode);
    }

    if ( addCommandMessage(pcen, context, MAKEWPARAM(id, notificationCode), 0xFFFFFFFF, 0, 0, methodName, tag) )
    {
        return TheZeroObj;
    }

    return TheOneObj;
}

/** EventNotification::connectStatusBarEvent()
 *
 *  Connects a Rexx dialog method with a status bare control event.
 *
 *  @param  rxID        The resource ID of the dialog control.  Can be numeric
 *                      or symbolic.
 *
 *  @param  event       Keyword specifying which event to connect.  Keywords at
 *                      this time:
 *
 *
 *
 *
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted then the
 *                      method name is constructed by prefixing the event
 *                      keyword with 'on'.  For instance onAutoBreak.
 *
 *  @param  willReply   [OPTIONAL] Common willReply behavior.  By default
 *                      willReply is set to true.  Note that the mouse clicks
 *                      all require a reply.
 *
 *  @return  True if the event notification was connected, otherwsie false.
 *
 *  @note   If a symbolic ID is  used and it can not be resolved to a numeric
 *          number an exception is raised.
 *
 *  @remarks  This method is new since the 4.0.0 release, therefore an exception
 *            is raised for a bad resource ID rather than returning -1.
 *
 *            For controls new since 4.0.0, event notifications that have a
 *            reply are usually documented as always requiring a reply. This may
 *            be over-kill for the mouse clicks, but the clicks are processed
 *            using the generic mouse click function, so we keep this as
 *            requiring a reply.
 */
RexxMethod5(RexxObjectPtr, en_connectStatusBarEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, _willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id = oodResolveSymbolicID(context->threadContext, pcen->rexxSelf, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto err_out;
    }

    uint32_t notificationCode;
    if ( ! keyword2sbn(context, event, &notificationCode) )
    {
        goto err_out;
    }
    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = sbn2name(notificationCode);
    }

    uint32_t tag = 0;
    if ( isMustReplySbn(notificationCode) )
    {
        tag = TAG_REPLYFROMREXX;
    }
    else
    {
        tag = _willReplyToTag(context, _willReply, true, 4);
        if ( tag == TAG_INVALID )
        {
            goto err_out;
        }
    }
    tag |= TAG_STATUSBAR;

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
}

/** EventNotification::connectTabEvent()
 *
 *
 *
 */
RexxMethod5(RexxObjectPtr, en_connectTabEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return TheNegativeOneObj;
    }

    uint32_t tag = _willReplyToTag(context, willReply, false, 4);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }
    tag |= TAG_TAB;

    uint32_t notificationCode;
    if ( ! keyword2tcn(context, event, &notificationCode) )
    {
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = tcn2name(notificationCode);
    }

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheZeroObj;
    }

    return TheOneObj;
}

/** EventNotification::connectToolBarEvent()
 *
 *  Connects a Rexx dialog method with a toolbar control event.
 *
 *  @param  rxID        The resource ID of the dialog control.  Can be numeric
 *                      or symbolic.
 *
 *  @param  event       Keyword specifying which event to connect.  Keywords at
 *                      this time:
 *
 *                      ?AUTOBREAK?
 *
 *
 *
 *
 *
 *
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted then the
 *                      method name is constructed by prefixing the event
 *                      keyword with 'on'.  For instance onAutoBreak.
 *
 *  @param  willReply   [OPTIONAL] Specifies if the method invocation should be
 *                      direct or indirect. With a direct invocation, the
 *                      interpreter waits in the Windows message loop for the
 *                      return from the Rexx method. With indirect, the Rexx
 *                      method is invoked through ~startWith(), which of course
 *                      returns immediately.  By default willReply is set to
 *                      true.
 *
 *  @return  True if the event notification was connected, otherwsie false.
 *
 *  @note   If a symbolic ID is used and it can not be resolved to a numeric
 *          number an exception is raised.
 *
 *  @remarks  This method is new since the 4.0.0 release, therefore an exception
 *            is raised for a bad resource ID rather than returning -1.
 *
 *            For controls new since 4.0.0, event notifications that have a
 *            reply are documented as always being 'direct' reply and
 *            notifications that ignore the return are documented as allowing
 *            the programmer to specify.  This means that willReply is ignored
 *            for [see the list in isMustReplyTbn] and not ignored for [see the
 *            list in isMustReplyTbn], for example.
 */
RexxMethod5(RexxObjectPtr, en_connectToolBarEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, _willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id = oodResolveSymbolicID(context->threadContext, pcen->rexxSelf, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto err_out;
    }

    uint32_t notificationCode;
    if ( ! keyword2tbn(context, event, &notificationCode) )
    {
        goto err_out;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = tbn2name(notificationCode);
    }

    uint32_t tag = 0;
    if ( isMustReplyTbn(notificationCode) )
    {
        tag = TAG_REPLYFROMREXX;
    }
    else
    {
        tag = _willReplyToTag(context, _willReply, true, 4);
        if ( tag == TAG_INVALID )
        {
            goto err_out;
        }
    }
    tag |= TAG_TOOLBAR;

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
}

/** EventNotification::connectToolTipEvent()
 *
 *  Connects a Rexx dialog method with a tool tip control event.
 *
 *  @param  rxID        The resource ID of the tool tip control.  Can be numeric
 *                      or symbolic.
 *
 *  @param  event       Keyword specifying which event to connect.  Keywords at
 *                      this time:
 *
 *                      LINKCLICK
 *                      NEEDTEXT
 *                      POP
 *                      SHOW
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted then the
 *                      method name is constructed by prefixing the event
 *                      keyword with 'on'.  For instance onPop.
 *
 *  @param  willReply   [OPTIONAL] Specifies if the method invocation should be
 *                      direct or indirect. With a direct invocation, the
 *                      interpreter waits in the Windows message loop for the
 *                      return from the Rexx method. With indirect, the Rexx
 *                      method is invoked through ~startWith(), which of course
 *                      returns immediately.
 *
 *  @return  True if the event notification was connected, otherwsie false.
 *
 *  @note   If a symbolic ID is  used and it can not be resolved to a numeric
 *          number an exception is raised.
 *
 *  @remarks  This method is new since the 4.0.0 release, therefore an exception
 *            is raised for a bad resource ID rather than returning -1.
 *
 *            For controls new since 4.0.0, event notifications that have a
 *            reply are documented as always being 'direct' reply and
 *            notifications that ignore the return are documented as allowing
 *            the programmer to specify.  This means that willReply is ignored
 *            for TTN_NEEDTEXT and TTN_SHOW and not ignored for TTN_LINKCLICK
 *            and TTNPOP.
 *
 *            Note that the underlying tool tip does not have a ID.  We require
 *            the user to supply an ID when instantiating a tool tip.  We then
 *            maintain a table for all tool tips for the dialog and use the ID
 *            to do table look ups.  In addition
 */
RexxMethod5(RexxObjectPtr, en_connectToolTipEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, _willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id = oodResolveSymbolicID(context->threadContext, pcen->rexxSelf, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto err_out;
    }

    PTOOLTIPTABLEENTRY ptte = findToolTipForID((pCPlainBaseDialog)pcen->pDlgCSelf, id);
    if ( ptte == NULL )
    {
        noToolTipException(context, pcen->rexxSelf);
        goto err_out;
    }

    uint32_t notificationCode;
    if ( ! keyword2ttn(context, event, &notificationCode) )
    {
        goto err_out;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = ttn2name(notificationCode);
    }

    uint32_t tag = 0;
    if ( notificationCode == TTN_NEEDTEXT || notificationCode == TTN_SHOW )
    {
        tag = TAG_REPLYFROMREXX;
    }
    else
    {
        tag = _willReplyToTag(context, _willReply, true, 4);
        if ( tag == TAG_INVALID )
        {
            goto err_out;
        }
    }
    tag |= TAG_TOOLTIP;

    if ( addNotifyMessage(pcen, context, (WPARAM)ptte->hToolTip, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
}

/** EventNotification::connectTrackBarEvent()
 *
 *
 *  @note  The connectTrackBar() method can only work if it is called after the
 *         underlying Windows dialog has been created.  Essentially this means
 *         the method should be used in the initDialog() method, (or later in
 *         the life-cycle of the dialog.)
 *
 *  @remarks  In the original ooDialog implementation, they allowed an optional
 *            4th argument that would be the window handle of a CategoryDialog.
 *            This allowed them to get the window handle of the track bar in a
 *            category page.  That fourth argument was never documented and no
 *            example code ever used it.  As of ooDialog 4.2.4, using
 *            connectTrackBarEvent, (or connectSliderNotify) in a Category is
 *            not supported.  Although it is doubtful, that was ever done, we
 *            will put up a message box stating this is not supported if we
 *            detect this situation.
 */
RexxMethod5(RexxObjectPtr, en_connectTrackBarEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return TheNegativeOneObj;
    }
    if ( context->IsOfType(pcen->rexxSelf, "CATEGORYDIALOG") && argumentExists(4) )
    {
        return notSupported(pcen->hDlg);
    }

    HWND hTrackBar = GetDlgItem(pcen->hDlg, id);
    if ( hTrackBar == NULL )
    {
        return TheNegativeOneObj;
    }

    uint32_t tag = _willReplyToTag(context, willReply, false, 4);
    if ( tag == TAG_INVALID )
    {
        return TheNegativeOneObj;
    }

    uint32_t notificationCode;
    if ( ! keyword2tb(context, event, &notificationCode) )
    {
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = tb2name(notificationCode);
    }

    if ( isVerticalTrackBar(hTrackBar) )
    {
        if ( addMiscMessage(pcen, context, WM_VSCROLL, 0xFFFFFFFF, notificationCode, 0x0000FFFF,
                            (LPARAM)hTrackBar, UINTPTR_MAX, methodName, tag) )
        {
            return TheZeroObj;
        }
    }
    else
    {
        if ( addMiscMessage(pcen, context, WM_HSCROLL, 0xFFFFFFFF, notificationCode, 0x0000FFFF,
                            (LPARAM)hTrackBar, UINTPTR_MAX, methodName, tag) )
        {
            return TheZeroObj;
        }
    }

    return TheOneObj;
}

/** EventNotification::connectTreeViewEvent()
 *
 *  Connects a Rexx dialog method with a tree-view event.
 *
 *  @param  rxID        The resource ID of the dialog control.  Can be numeric
 *                      or symbolic.
 *
 *  @param  event       Keyword specifying which event to connect.  Keywords at
 *                      this time:
 *
 *                      SELCHANGING
 *                      SELCHANGED
 *                      EXPANDING
 *                      EXPANDED
 *                      BEGINDRAG
 *                      BEGINRDRAG
 *                      DELETE
 *                      BEGINEDIT
 *                      ENDEDIT
 *                      DEFAULTEDIT
 *                      KEYDOWN
 *                      GETINFOTIP
 *
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted then the
 *                      method name is constructed by prefixing the event
 *                      keyword with 'on'.  For instance onExpanding.
 *
 *  @param  willReply   [OPTIONAL] Specifies if the method invocation should be
 *                      direct or indirect. With a direct invocation, the
 *                      interpreter waits in the Windows message loop for the
 *                      return from the Rexx method. With indirect, the Rexx
 *                      method is invoked through ~startWith(), which of course
 *                      returns immediately.
 *
 *                      For tree-views, at this time, the default is false, i.e.
 *                      the Rexx programmer needs to specify that she wants to
 *                      reply.  This could change if new key words are added.
 *
 *  @return 0 for no error, -1 for a bad resource ID or incorrect event keyword,
 *          1 if the event could not be connected.  The event can not be
 *          connected if there is a problem with the message table, full or out
 *          of memory error.
 *
 *  @remarks   For the current keywords, if a symbolic ID is  used and it can
 *             not be resolved to a numeric number -1 has to be returned for
 *             backwards compatibility.  Essentially, except for the keywords
 *             listed below, for this method, all behaviour needs to be
 *             pre-4.2.0.
 *
 *             EXPANDING / EXPANDED  The willReply request is honored
 *
 *             BEGINLABELEDIT / ENDLABELEDIT
 *
 *             INFOTIP  new keyword - will reply is always set to true for this
 *             keyword.
 *
 *             The processing for beginlabeledit and endlabeledit that was done
 *             for the DEFAULTEDIT keyword is not all needed for a tree-view.
 *             This is changed in a similar manner as the list-view was changed.
 *             However, for a tree-view if we do not subclass the edit control,
 *             the enter and the esc key close the dialog.  What we do here is
 *             very similar to what we do in the connect list-view event
 *             function.  See that header doc if more detail is needed.
 *
 *             For reference.  The arguments sent to the event handler for
 *             TVN_ENDLABELEDIT were never documented correctly, if at all.
 *             They were as follows.  If the user did *not* cancel the edit:
 *               arg 1 tree-view id    (from wParam)
 *               arg 2 handle of item being edited
 *               arg 3 text user entered.
 *
 *             If the user did cancel the edit:
 *               arg 1 tree-view id    (from wParam)
 *               arg 2 pointer to the NMLVDISPINFO struct as a decimal value
 *                                     (from lParam)
 *
 *             The arguments to the event handler for TVN_BEGINLABELEDIT were
 *             never documented at all.  For reference they were:
 *               arg 1 tree-view id    (from wParam)
 *               arg 2 pointer to the NMLVDISPINFO struct as a decimal value
 *                                     (from lParam)
 */
RexxMethod5(RexxObjectPtr, en_connectTreeViewEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return TheNegativeOneObj;
    }

    uint32_t notificationCode = 0;
    bool     willReplyExists  = argumentExists(4) ? true : false;
    bool     isDefEdit        = false;
    uint32_t tag              = 0;

    if ( ! keyword2tvn(context, event, &notificationCode, &tag, &isDefEdit, willReply, willReplyExists) )
    {
        return TheNegativeOneObj;
    }

    // Deal with DEFAULTEDIT separately.
    if ( isDefEdit )
    {
        if ( ! addNotifyMessage(pcen, context, id, 0xFFFFFFFF, TVN_BEGINLABELEDIT, 0xFFFFFFFF, "DefTreeEditStarter", tag) )
        {
            return TheNegativeOneObj;
        }
        if ( ! addNotifyMessage(pcen, context, id, 0xFFFFFFFF, TVN_ENDLABELEDIT, 0xFFFFFFFF, "DefTreeEditHandler", tag) )
        {
            return TheNegativeOneObj;
        }
        return TheZeroObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = tvn2name(notificationCode, tag);
    }

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheZeroObj;
    }

    return TheOneObj;
}


/** EventNotification::connectUpDownEvent()
 *
 *  Connects a Rexx dialog method with an up down control event.
 *
 *  @param  rxID        The resource ID of the dialog control.  Can be numeric
 *                      or symbolic.
 *
 *  @param  event       Keyword specifying which event to connect.  Only one at
 *                      this time:
 *
 *                      DELTAPOS
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted then the
 *                      method name is constructed by prefixing the event
 *                      keyword with 'on'.  For instance onDeltaPos.
 *
 *  @note   If a symbolic ID is  used and it can not be resolved to a numeric
 *          number an exception is raised.
 *
 *  @remarks  This method is new since the 4.0.0 release, therefore an exception
 *            is raised for a bad resource ID rather than returning -1.
 *
 *            MSDN docs say that the up-down control also sends the
 *            NM_RELEASEDCAPTURE message.  I see no evidence it is ever sent.
 *            And there is this from a news group:
 *
 *            Christian ASTOR    View profile
 *
 *            Alexander Grigoriev wrote:
 *            > I need to handle NM_RELEASEDCAPTURE notification from an up-down
 *            > control in a dialog. But they don't seem to send it, no matter
 *            > how I'm trying.
 *
 *            Up-Down control doesn't send NM_RELEASEDCAPTURE notification.
 *            (docs are wrong...)
 *
 *            Since DELTAPOS is the only notification and it is always direct
 *            reply, we don't have the the optional fourth, willReply, argument.
 */
RexxMethod4(RexxObjectPtr, en_connectUpDownEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id = oodResolveSymbolicID(context->threadContext, pcen->rexxSelf, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto err_out;
    }

    uint32_t notificationCode;

    if ( StrStrI(event, "DELTAPOS") != NULL )
    {
        notificationCode = UDN_DELTAPOS;
    }
    else
    {
        wrongArgValueException(context->threadContext, 2, "DeltaPos", event);
        goto err_out;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = "onDeltaPos";
    }

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, TAG_UPDOWN) )
    {
        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
}

/** EventNotification::connectWmEvent()
 *
 *  This is a generic method used for a number of connectXXX methods.  It is
 *  used to connect miscellaneous WM_xx messages,  WM_ACTIVE, WM_SIZE, etc..
 *
 *  We key on the method name to decide what action to take.  Note that this is
 *  the msgName argument.
 *
 * @remarks  Some of these are the original ooDialog implementations, plus
 *           willReply.  Be careful if refactoring to maintain compatibility.
 *
 *           Some are newer methods, but with already documented arguments and
 *           defaults.  For those we must keep what is documented.
 *
 *           However, the general principal going forward is to use TAG_SYNC
 *           instead of TAG_WILLREPLY.  As newer methods are added, the behavior
 *           can be determined on a case by case basis.
 */
RexxMethod4(RexxObjectPtr, en_connectWmEvent, OPTIONAL_CSTRING, methodName, OPTIONAL_RexxObjectPtr, willReply,
            NAME, msgName, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    uint32_t tag = _willReplyToTag(context, willReply, false, 2);
    if ( tag == TAG_INVALID )
    {
        return TheOneObj;
    }

    uint32_t winMsg = methodName2windowMessage(msgName);
    switch ( winMsg )
    {
        case WM_ACTIVATE :
            if ( argumentOmitted(1) || *methodName == '\0' ) methodName = "onActivate";

            // over-ride any user input, user must always reply.
            tag = TAG_REPLYFROMREXX;
            break;

        case WM_HELP :
            if ( argumentOmitted(1) || *methodName == '\0' ) methodName = "onHelp";
            tag |= TAG_DIALOG | TAG_HELP;
            break;

        case WM_MOVE :
            if ( argumentOmitted(1) || *methodName == '\0' ) methodName = "onMove";
            break;

        case WM_WINDOWPOSCHANGED :
            if ( argumentOmitted(1) || *methodName == '\0' ) methodName = "onPosChanged";
            break;

        case WM_SIZE :
            if ( argumentOmitted(1) || *methodName == '\0' ) methodName = "onResize";

            // Before the sync option was added to the _willReply argument,
            // .true was documented for connectResize() as waiting for the event
            // handler, but not requiring a return value.  .true was also
            // documented as the default if willReply is omitted.  This needs to
            // be maintained.
            //
            // So, if tag is reply from Rexx, we change it to sync.  If the arg
            // is omitted we change it to sync. This should not break anything.
            if ( argumentOmitted(2) ||  tag == TAG_REPLYFROMREXX )
            {
                tag = TAG_SYNC;
            }
            break;

        case WM_SIZING :
            if ( argumentOmitted(1) || *methodName == '\0' ) methodName = "onResizing";

            // over-ride any user input, user must always reply.
            tag = TAG_REPLYFROMREXX;
            break;

        case WM_EXITSIZEMOVE :
            if ( argumentOmitted(1) || *methodName == '\0' ) methodName = "onSizeMoveEnded";

            // Already documented as the default being true.
            tag = _willReplyToTag(context, willReply, true, 2);
            if ( tag == TAG_INVALID )
            {
                return TheOneObj;
            }
            break;

        default :
            return TheOneObj;
            break;
    }

    if ( addMiscMessage(pcen, context, winMsg, 0xFFFFFFFF, 0, 0, 0, 0, methodName, tag) )
    {
        return TheZeroObj;
    }
    return TheOneObj;
}

