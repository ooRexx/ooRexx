/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2012 Rexx Language Association. All rights reserved.    */
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
#include <WindowsX.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodMessaging.hpp"
#include "oodMouse.hpp"
#include "oodData.hpp"
#include "oodPropertySheetDialog.hpp"

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

            if ( pcpbd->CT_nextIndex > 0 )
            {
                // See of the user has set the dialog item with a different
                // color.
                long id = GetWindowLong((HWND)lParam, GWL_ID);
                if ( id > 0 )
                {
                    register size_t i = 0;
                    while ( i < pcpbd->CT_nextIndex && pcpbd->ColorTab[i].itemID != id )
                    {
                        i++;
                    }
                    if ( i < pcpbd->CT_nextIndex )
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

        case WM_COMMAND:
            switch ( LOWORD(wParam) )
            {
                case IDOK:
                case IDCANCEL:

                    // For both IDOK and IDCANCEL, the notification code
                    // (the high word value) must be 0.
                    if ( HIWORD(wParam) == 0 )
                    {
                        // We should never get here because both IDOK and
                        // IDCANCEL should have be interecepted in
                        // searchMessageTables().  But - sometimes we do, very
                        // rarely.  It is on some abnormal error. See the
                        // comments above for the WM_DESTROY message.
                        pcpbd->abnormalHalt = true;
                        DestroyWindow(hDlg);

                        return TRUE;
                    }
            }
            break;

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            return paletteMessage(pcpbd, hDlg, uMsg, wParam, lParam);

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
                                                    wParam);
            ReplyMessage((LRESULT)hChild);
            return TRUE;
        }

        case WM_USER_CREATECONTROL_RESDLG:
        {
            pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)wParam;

            HWND hChild = CreateDialogParam(pcpbd->hInstance, MAKEINTRESOURCE((uint32_t)lParam), pcpbd->hOwnerDlg,
                                            (DLGPROC)RexxChildDlgProc, (LPARAM)pcpbd);

            ReplyMessage((LRESULT)hChild);
            return TRUE;
        }

        case WM_USER_CREATEPROPSHEET_DLG:
        {
            pCPropertySheetDialog pcpsd = (pCPropertySheetDialog)lParam;

            assignPSDThreadContext(pcpsd, pcpbd->dlgProcContext, pcpbd->dlgProcThreadID);

            if ( setPropSheetHook(pcpsd) )
            {
                SetLastError(0);
                INT_PTR ret = PropertySheet((PROPSHEETHEADER *)wParam);
                oodSetSysErrCode(pcpbd->dlgProcContext);
                ReplyMessage((LRESULT)ret);
            }
            else
            {
                ReplyMessage((LRESULT)-1);
            }

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

        setWindowPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pcpbd);
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

            if ( pcpbd->CT_nextIndex > 0 )
            {
                // See of the user has set the dialog item with a different
                // color.
                long id = GetWindowLong((HWND)lParam, GWL_ID);
                if ( id > 0 )
                {
                    register size_t i = 0;
                    while ( i < pcpbd->CT_nextIndex && pcpbd->ColorTab[i].itemID != id )
                    {
                        i++;
                    }
                    if ( i < pcpbd->CT_nextIndex )
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

static RexxObjectPtr sc2rexx(RexxThreadContext *c, WPARAM wParam)
{
    CSTRING s;

    switch ( wParam & 0xFFF0 )
    {
        case SC_SIZE :
            s = "SIZE";
            break;
        case SC_MOVE :
            s = "MOVE";
            break;
        case SC_MINIMIZE :
            s = "MINIMIZE";
            break;
        case SC_MAXIMIZE :
            s = "MAXIMIZE";
            break;
        case SC_NEXTWINDOW   :
            s = "NEXTWINDOW";
            break;
        case SC_PREVWINDOW   :
            s = "PREVWINDOW";
            break;
        case SC_CLOSE :
            s = "CLOSE";
            break;
        case SC_VSCROLL :
            s = "VSCROLL";
            break;
        case SC_HSCROLL :
            s = "HSCROLL";
            break;
        case SC_MOUSEMENU :
            s = "MOUSEMENU ";
            break;
        case SC_KEYMENU :
            s = "KEYMENU";
            break;
        case SC_ARRANGE :
            s = "ARRANGE";
            break;
        case SC_RESTORE :
            s = "RESTORE";
            break;
        case SC_TASKLIST :
            s = "TASKLIST";
            break;
        case SC_SCREENSAVE :
            s = "SCREENSAVE";
            break;
        case SC_HOTKEY :
            s = "HOTKEY";
            break;
        case SC_DEFAULT :
            s = "DEFAULT";
            break;
        case SC_MONITORPOWER :
            s = "MONITORPOWER";
            break;
        case SC_CONTEXTHELP :
            s = "CONTEXTHELP";
            break;
        case SC_SEPARATOR :
            s = "SEPARATOR";
            break;

        // SCF_ISSECURE, only defined if WINVER >= 0x0600
        case 0x00000001 :
            s = "ISSECURE";
            break;
        default :
            // Could be a menu command item inserted by the user
            return c->WholeNumber(wParam & 0xFFF0);
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

inline bool selectionDidChange(LPNMLISTVIEW p)
{
    return ((p->uNewState & LVIS_SELECTED) != (p->uOldState & LVIS_SELECTED));
}

inline bool focusDidChange(LPNMLISTVIEW p)
{
    return ((p->uNewState & LVIS_FOCUSED) != (p->uOldState & LVIS_FOCUSED));
}

/* matchSelectFocus
 * Check that: (a) tag is for select change and focuse change, and (b) that
 * either the selection or the focus actually changed.
 */
inline bool matchSelectFocus(uint32_t tag, LPNMLISTVIEW p)
{
    return ((tag & TAG_SELECTCHANGED) && (tag & TAG_FOCUSCHANGED)) && (selectionDidChange(p) || focusDidChange(p));
}

/* matchSelect
 * Check that: (a) tag is only for selection change and not focuse change, and (b)
 * that the selection actually changed.
 */
inline bool matchSelect(uint32_t tag, LPNMLISTVIEW p)
{
    return ((tag & TAG_SELECTCHANGED) && !(tag & TAG_FOCUSCHANGED)) && (selectionDidChange(p));
}

/* matchFocus
 * Check that: (a) tag is only for focus change and not selection change, and (b)
 * that the focus actually changed.
 */
inline bool matchFocus(uint32_t tag, LPNMLISTVIEW p)
{
    return ((tag & TAG_FOCUSCHANGED) && !(tag & TAG_SELECTCHANGED)) && (focusDidChange(p));
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
 *           Setting abnormalHalt causes ensureFinished() to be invoked which
 *           terminates the thread waiting on the finished instance variable.
 *
 *           This works fine for modeless property sheets, but modal property
 *           sheets hang.  The abortPropertySheet() essentially does a
 *           programmatic close and prevents any of the property sheet pages
 *           from nixing the close.
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
    }

    DestroyWindow(hDlg);

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
 * Invokes the Rexx dialog's event handling method for a Windows message.
 *
 * The method invocation is done indirectly using startWith().  This allows us
 * to return quickly to the window message processing loop.
 *
 * @param c       Thread context we are operating in.
 * @param obj     The Rexx dialog whose method will be invoked.
 * @param method  The name of the method being invoked
 * @param args    The argument array for the method being invoked
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
MsgReplyType invokeDispatch(RexxThreadContext *c, RexxObjectPtr obj, RexxStringObject method, RexxArrayObject args)
{
    c->SendMessage2(obj, "STARTWITH", method, args);
    return ReplyTrue;
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
 * the user returns a value form the event handler.
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
    bool haveCondition = checkForCondition(c, clear);

    if ( checkForCondition(c, clear) )
    {
        endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
        return true;
    }
    return false;
}


/**
 * Checks that reply is either true or false.  If not, an exception is raised
 * and the dialog is ended.
 *
 * @param c
 * @param pcpbd
 * @param reply
 * @param method
 * @param clear
 *
 * @return TheTrueObj or TheFalseObj on success, NULLOBJECT on failure.
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
            endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
        }
    }
    return result;
}

/**
 * The simplest form of invoking the Rexx method connected to a WM_NOTIFY
 * message.  The Rexx method is invoked with two arguments, the resource ID of
 * the control and the HWND of the control.
 *
 * This function should be used where the return value from the specifiec
 * WM_NOTIFY message is ignored.  In Rexx, the method invocation is done through
 * startWith() and returns immediately.
 *
 * @param c
 * @param pcpbd
 * @param methodName
 * @param idFrom
 * @param hwndFrom
 *
 * @return MsgReplyType
 */
inline MsgReplyType genericNotifyInvoke(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING methodName,
                                        RexxObjectPtr idFrom, RexxObjectPtr hwndFrom)
{
    return invokeDispatch(c, pcpbd->rexxSelf,
                          c->String(methodName),
                          c->ArrayOfTwo(idFrom, hwndFrom));
}

/* genericCommandInvoke
 *
 * The simplest form of invoking the Rexx method connected to a WM_COMMAND
 * message.  The Rexx method is invoked with two arguments, the WPARAM and
 * LPARAM paramters of the WM_COMMAND message.
 *
 * Note that for WM_COMMAND messages, lParam is always the window handle of the
 * dialog control, if a control initiated the message.  For menu items and
 * accelerators, it is always 0. So, converting to a pseudo pointer is always
 * the correct thing to do.
 */
static MsgReplyType genericCommandInvoke(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING methodName,
                                         uint32_t tag, WPARAM wParam, LPARAM lParam)
{
    RexxArrayObject args = c->ArrayOfTwo(c->Uintptr(wParam), pointer2string(c, (void *)lParam));

    if ( tag & TAG_REPLYFROMREXX )
    {
        // We only get here for messages where what the Rexx method returns is
        // discarded / ignored.  This is true now, but we need to be careful
        // when adding new event connections to ooDialog.
        invokeDirect(c, pcpbd, methodName, args);
        return ReplyTrue;
    }
    else
    {
        return invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
    }
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
    RexxObjectPtr rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
    return msgReplyIsGood(c, pcpbd, rexxReply, methodName, false);
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
 *           check that the Rexx method returned a value.
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
    RexxObjectPtr rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
    return ! endOnCondition(c, pcpbd, methodName, false);
}

/**
 * Construct the argument array sent to the Rexx event handling method through
 * dispatchWindowMessage().
 *
 * @param pcpbd
 * @param rexxMethod
 * @param wParam
 * @param lParam
 * @param np
 * @param handle
 * @param item
 * @param tag
 *
 * @return MsgReplyType
 *
 * @remarks  Earlier versions of ooDialog constructed a method invocation string
 *           that was put into a queue.  Rexx pulled the method invocation
 *           string, such as onEndTrack(101, 0x00CA23F0), and invoked the method
 *           through interpret.
 *
 *           For backwards compatibility, this function attempts to construct
 *           the argument array so that the arguments match what the earlier
 *           versions of ooDialog would have used.
 *
 *           It is a little problematic as to whether wParam and lParam are
 *           getting converted properly.
 */
MsgReplyType genericInvokeDispatch(pCPlainBaseDialog pcpbd, char *rexxMethod, WPARAM wParam, LPARAM lParam,
                                     char *np, HANDLE handle, int item, uint32_t tag)
{
    RexxThreadContext *c = pcpbd->dlgProcContext;
    RexxStringObject method = c->String(rexxMethod);
    RexxArrayObject args;

    if ( wParam == NULL && lParam == 0 )
    {
        args = c->NewArray(0);
    }
    else if ( np != NULL )
    {
        if ( handle != NULL )
        {
            args = c->ArrayOfThree(c->Uintptr(wParam), pointer2string(c, (void *)handle), c->String(np));
        }
        else
        {
            args = c->ArrayOfThree(c->Uintptr(wParam), c->Int32(item), c->String(np));
        }
    }
    else if ( handle != NULL )
    {
        if ( item > OOD_INVALID_ITEM_ID )
        {
            args = c->ArrayOfTwo(c->Int32(item), pointer2string(c, (void *)handle));
        }
        else
        {
            args = c->ArrayOfTwo(c->Uintptr(wParam), pointer2string(c, (void *)handle));
        }
    }
    else
    {
        // lParam might not come out right ...
        args = c->ArrayOfTwo(c->Uintptr(wParam), c->Intptr(lParam));
    }

    if ( tag & TAG_REPLYFROMREXX )
    {
        invokeDirect(c, pcpbd, rexxMethod, args);
        return ReplyTrue;
    }
    if ( tag & TAG_SYNC )
    {
        invokeSync(c, pcpbd, rexxMethod, args);
        return ReplyTrue;
    }
    else
    {
        return invokeDispatch(c, pcpbd->rexxSelf, method, args);
    }
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

inline RexxObjectPtr idFrom2rexxArg(RexxThreadContext *c, LPARAM lParam)
{
    return c->Uintptr(((NMHDR *)lParam)->idFrom);
}

inline RexxObjectPtr hwndFrom2rexxArg(RexxThreadContext *c, LPARAM lParam)
{
    return pointer2string(c, ((NMHDR *)lParam)->hwndFrom);
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
 *           handle of the dialog control, if a control iniated the message. For
 *           menu items and accelerators, it is always 0. So, converting to a
 *           pseudo pointer is always the correct thing to do.
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
        if ( ((wParam & m[i].wpFilter) == m[i].wParam) && ((lParam & m[i].lpfilter) == m[i].lParam) )
        {
            return genericCommandInvoke(pcpbd->dlgProcContext, pcpbd, m[i].rexxMethod, m[i].tag, wParam, lParam);
        }
    }
    return ContinueProcessing;
}


/**
 * Helper function to deterimine a list view item's index using a hit test.
 *
 * @param hwnd  Handle of the list view.
 * @param pIA   Pointer to an item activate structure.
 *
 * @remarks  This function should only be used when the list view is in report
 *           mode.  If the subitem hit test does not produce an item index, we
 *           only look for a y position that falls within the bounding rectangle
 *           of a visible item.
 *
 *           We start with the top visible index and look at each item on the
 *           page.  The count per page only includes fully visible items, so we
 *           also check for a last, partially visible item.
 */
static void getItemIndexFromHitPoint(LPNMITEMACTIVATE pIA, HWND hwnd)
{
    LVHITTESTINFO lvhti = {0};
    lvhti.pt.x = pIA->ptAction.x;
    lvhti.pt.y = pIA->ptAction.y;
    lvhti.flags = LVHT_ONITEM;

    ListView_SubItemHitTestEx(hwnd, &lvhti);
    pIA->iItem = lvhti.iItem;

    if ( pIA->iItem == -1 )
    {
        int topIndex = ListView_GetTopIndex(hwnd);
        int count = ListView_GetCountPerPage(hwnd);
        RECT r;

        for ( int i = topIndex; i <= count; i++)
        {
            if ( ListView_GetItemRect(hwnd, i, &r, LVIR_BOUNDS) )
            {
                if ( lvhti.pt.y >= r.top && lvhti.pt.y < r.bottom )
                {
                    pIA->iItem = i;
                    break;
                }
            }
        }
    }
}

MsgReplyType processLVN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    char          tmpBuffer[20];
    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);

    MsgReplyType  msgReply = ReplyFalse;
    bool          expectReply = (tag & TAG_REPLYFROMREXX) == TAG_REPLYFROMREXX;

    switch ( code )
    {
        case NM_CLICK:
        case NM_DBLCLK:
        {
            LPNMITEMACTIVATE pIA = (LPNMITEMACTIVATE)lParam;

            if ( pIA->uKeyFlags == 0 )
            {
                strcpy(tmpBuffer, "NONE");
            }
            else
            {
                tmpBuffer[0] = '\0';

                if ( pIA->uKeyFlags & LVKF_SHIFT )
                    strcpy(tmpBuffer, "SHIFT");
                if ( pIA->uKeyFlags & LVKF_CONTROL )
                    tmpBuffer[0] == '\0' ? strcpy(tmpBuffer, "CONTROL") : strcat(tmpBuffer, " CONTROL");
                if ( pIA->uKeyFlags & LVKF_ALT )
                    tmpBuffer[0] == '\0' ? strcpy(tmpBuffer, "ALT") : strcat(tmpBuffer, " ALT");
            }

            // The user can click on an item in a list view, or on the
            // background of the list view.  For report mode only, the user can
            // also click on a subitem of the item.  When the click is on the
            // background, the item index and column index will be sent to the
            // Rexx method as -1.
            //
            // In report mode, if the list view has the extended full row select
            // stylye, everything works as expected.  But, without that style,
            // if the user clicks anywhere on the row outside of the item icon
            // and item text, the OS does not report the item index.  This looks
            // odd to the user.  For this case we go to some extra trouble to
            // get the correct item index.
            if ( pIA->iItem == -1 && pIA->iSubItem != -1 )
            {
                HWND hwnd = pIA->hdr.hwndFrom;
                if ( isInReportView(hwnd)  )
                {
                    getItemIndexFromHitPoint(pIA, hwnd);
                }
                else
                {
                    // iSubItem is always 0 when not in report mode, but -1 is
                    // more consistent.
                    pIA->iSubItem = -1;
                }
            }

            RexxArrayObject args = c->ArrayOfFour(idFrom, c->Int32(pIA->iItem), c->Int32(pIA->iSubItem), c->String(tmpBuffer));

            if ( expectReply )
            {
                invokeDirect(c, pcpbd, methodName, args);
            }
            else
            {
                invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
            }

            break;
        }

        case LVN_ITEMCHANGED:
        {
            LPNMLISTVIEW pLV = (LPNMLISTVIEW)lParam;

            RexxObjectPtr item = c->Int32(pLV->iItem);
            char *p;

            /* The use of the tag field allows a finer degree of control as to exactly which event
             * the user wants to be notified of, then does the initial message match above.  Because
             * of that, this specific LVN_ITEMCHANGED notification may not match the tag.  So, if we
             * do not match here, we continue the search through the message table because this
             * notification may match some latter entry in the table.
             */
            if ( (tag & TAG_STATECHANGED) && (pLV->uChanged == LVIF_STATE) )
            {
                if ( (tag & TAG_CHECKBOXCHANGED) && (pLV->uNewState & LVIS_STATEIMAGEMASK) )
                {
                    p = (pLV->uNewState == INDEXTOSTATEIMAGEMASK(2) ? "CHECKED" : "UNCHECKED");

                    RexxArrayObject args = c->ArrayOfThree(idFrom, item, c->String(p));

                    if ( expectReply )
                    {
                        invokeDirect(c, pcpbd, methodName, args);
                    }
                    else
                    {
                        invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
                    }
                }
                else if ( matchSelectFocus(tag, pLV) )
                {
                    tmpBuffer[0] = '\0';

                    if ( selectionDidChange(pLV) )
                    {
                        if ( pLV->uNewState & LVIS_SELECTED )
                        {
                            strcpy(tmpBuffer, "SELECTED");
                        }
                        else
                        {
                            strcpy(tmpBuffer, "UNSELECTED");
                        }
                    }

                    if ( focusDidChange(pLV) )
                    {
                        if ( pLV->uNewState & LVIS_FOCUSED )
                        {
                            tmpBuffer[0] == '\0' ? strcpy(tmpBuffer, "FOCUSED") : strcat(tmpBuffer, " FOCUSED");
                        }
                        else
                        {
                            tmpBuffer[0] == '\0' ? strcpy(tmpBuffer, "UNFOCUSED") : strcat(tmpBuffer, " UNFOCUSED");
                        }
                    }

                    RexxArrayObject args = c->ArrayOfThree(idFrom, item, c->String(tmpBuffer));

                    if ( expectReply )
                    {
                        invokeDirect(c, pcpbd, methodName, args);
                    }
                    else
                    {
                        invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
                    }
                }
                else if ( matchSelect(tag, pLV) )
                {
                    p = (pLV->uNewState & LVIS_SELECTED) ? "SELECTED" : "UNSELECTED";

                    RexxArrayObject args = c->ArrayOfThree(idFrom, item, c->String(p));

                    if ( expectReply )
                    {
                        if ( invokeDirect(c, pcpbd, methodName, args) )
                        {
                            // No condition was raised, it is safe to continue searching.
                            msgReply = ContinueSearching;  // Not sure if this is wise with the C++ API
                        }
                    }
                    else
                    {
                        invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
                        msgReply = ContinueSearching;  // Not sure if this is wise with the C++ API
                    }
                }
                else if ( matchFocus(tag, pLV) )
                {
                    p = (pLV->uNewState & LVIS_FOCUSED) ? "FOCUSED" : "UNFOCUSED";

                    RexxArrayObject args = c->ArrayOfThree(idFrom, item, c->String(p));

                    if ( expectReply )
                    {
                        if ( invokeDirect(c, pcpbd, methodName, args) )
                        {
                            // No condition was raised, it is safe to continue searching.
                            msgReply = ContinueSearching;  // Not sure if this is wise with the C++ API
                        }
                    }
                    else
                    {
                        invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
                        msgReply = ContinueSearching;  // Not sure if this is wise with the C++ API
                    }
                }
                else
                {
                    // This message in the message table does not match, keep searching.
                    msgReply = ContinueSearching;
                }
            }

            break;
        }

        case LVN_COLUMNCLICK :
        {
            RexxObjectPtr rxLV = createControlFromHwnd(c, pcpbd, ((NMHDR *)lParam)->hwndFrom, winListView, true);
            uint32_t      col = (ULONG)((NM_LISTVIEW *)lParam)->iSubItem;

            msgReply = ReplyTrue;

            RexxArrayObject args = c->ArrayOfThree(idFrom, c->UnsignedInt32(col), rxLV);

            if ( expectReply )
            {
                invokeDirect(c, pcpbd, methodName, args);
            }
            else
            {
                invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
            }

            break;
        }

        case LVN_KEYDOWN :
        {
            RexxObjectPtr rxLV = createControlFromHwnd(c, pcpbd, ((NMHDR *)lParam)->hwndFrom, winListView, true);
            uint16_t      vKey = ((NMLVKEYDOWN *)lParam)->wVKey;

            // The third argument is whether it is an extended key or not.  That
            // is the only way to tell between ...
            RexxArrayObject args  = getKeyEventRexxArgs(c, (WPARAM)vKey, false, rxLV);

            if ( expectReply )
            {
                invokeDirect(c, pcpbd, methodName, args);
            }
            else
            {
                invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
            }

            msgReply = ReplyTrue;

            break;
        }

        default :
            break;
    }

    return msgReply;
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
    RexxObjectPtr rexxReply;
    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr hwndFrom = hwndFrom2rexxArg(c, lParam);
    bool          expectReply = (tag & TAG_REPLYFROMREXX) == TAG_REPLYFROMREXX;

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

            if ( expectReply )
            {
                invokeDirect(c, pcpbd, methodName, args);
            }
            else
            {
                invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
            }

            break;
        }

        case DTN_FORMAT:
        {
            LPNMDATETIMEFORMAT pFormat = (LPNMDATETIMEFORMAT)lParam;

            RexxObjectPtr dt;
            sysTime2dt(c, &(pFormat->st), &dt, dtFull);

            RexxArrayObject args = c->ArrayOfFour(c->String(pFormat->pszFormat), dt, idFrom, hwndFrom);

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

            break;
        }

        case DTN_FORMATQUERY:
        {
            LPNMDATETIMEFORMATQUERY pQuery = (LPNMDATETIMEFORMATQUERY)lParam;

            RexxObjectPtr _size = rxNewSize(c, 0, 0);

            RexxArrayObject args = c->ArrayOfFour(c->String(pQuery->pszFormat), _size, idFrom, hwndFrom);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

            if ( msgReplyIsGood(c, pcpbd, rexxReply, methodName, false) )
            {
                PSIZE size = (PSIZE)c->ObjectToCSelf(_size);

                pQuery->szMax.cx = size->cx;
                pQuery->szMax.cy = size->cy;
            }

            break;
        }

        case DTN_USERSTRING:
        {
            LPNMDATETIMESTRING pdts = (LPNMDATETIMESTRING)lParam;

            RexxObjectPtr dt;
            sysTime2dt(c, &(pdts->st), &dt, dtFull);

            RexxStringObject userString = c->String(pdts->pszUserString);

            RexxArrayObject args = c->ArrayOfFour(dt, userString, idFrom, hwndFrom);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

            if ( msgReplyIsGood(c, pcpbd, rexxReply, methodName, false) )
            {
                if ( c->IsOfType(rexxReply, "DATETIME") )
                {
                    if ( ! dt2sysTime(c, rexxReply, &(pdts->st), dtFull) )
                    {
                        checkForCondition(c, false);
                        goto done_out;
                    }
                    pdts->dwFlags = GDT_VALID;
                }
                else if ( rexxReply == TheNilObj )
                {
                    if ( ! isShowNoneDTP(pdts->nmhdr.hwndFrom) )
                    {
                        wrongReplyMsgException(c, methodName, "can only be .nil if the DTP control has the SHOWNONE style");
                        checkForCondition(c, false);
                        goto done_out;
                    }
                    pdts->dwFlags = GDT_NONE;
                }
            }


            break;
        }

        case DTN_WMKEYDOWN:
        {
            LPNMDATETIMEWMKEYDOWN pQuery = (LPNMDATETIMEWMKEYDOWN)lParam;

            RexxObjectPtr dt;
            sysTime2dt(c, &(pQuery->st), &dt, dtFull);

            RexxArrayObject args = c->NewArray(5);
            c->ArrayPut(args, c->String(pQuery->pszFormat), 1);
            c->ArrayPut(args, dt, 2);
            c->ArrayPut(args, c->Int32(pQuery->nVirtKey), 3);
            c->ArrayPut(args, idFrom, 4);
            c->ArrayPut(args, hwndFrom, 5);

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
                    }
                    checkForCondition(c, false);
                }
            }

            break;
        }

        case DTN_CLOSEUP:
        case DTN_DROPDOWN:
        case NM_KILLFOCUS:
        case NM_SETFOCUS:
        {
            RexxArrayObject args = c->ArrayOfTwo(idFrom, hwndFrom);

            if ( expectReply )
            {
                invokeDirect(c, pcpbd, methodName, args);
            }
            else
            {
                invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
            }

            break;
        }

        default :
            // Theoretically we can not get here because all date time
            // picker notification codes that have a tag are accounted
            // for.
            break;
    }

done_out:
    return ReplyTrue;
}

/**
 * Processes tab control notifications.
 *
 * Note this is only invoked when TAG_TAB is set in the tag.  At this time,
 * TAG_TAB is only set for TCN_SELCHANGING and TAB_REPLYFROMREXX is always set
 * to true.  So, we could skip the checks, but this may be expanded in the
 * future.
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
    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr hwndFrom = hwndFrom2rexxArg(c, lParam);

    if ( (tag & TAG_EXTRAMASK) == TAG_REPLYFROMREXX )
    {
        switch ( code )
        {
            case TCN_SELCHANGING :
            {
                // The Rexx programmer returns .true, changing the tab is okay, or .false do not change tabs.
                RexxObjectPtr msgReply = c->SendMessage2(pcpbd->rexxSelf, methodName, idFrom, hwndFrom);

                msgReply = requiredBooleanReply(c, pcpbd, msgReply, methodName, false);
                if ( msgReply == TheTrueObj || msgReply == TheFalseObj )
                {
                    // Return true to prevent the change.
                    setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, msgReply == TheTrueObj ? FALSE : TRUE);
                }
                return ReplyTrue;
            }

            default :
                break;
        }
    }

    return genericNotifyInvoke(c, pcpbd, methodName, idFrom, hwndFrom);
}

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
                invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
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
                invokeDispatch(c, pcpbd->rexxSelf, c->String(methodName), args);
            }

            break;
        }

        case NM_RELEASEDCAPTURE :
        {
            if ( expectReply )
            {
                invokeDirect(c, pcpbd, methodName, c->ArrayOfTwo(idFrom, hwndFrom));
            }
            else
            {
                genericNotifyInvoke(c, pcpbd, methodName, idFrom, hwndFrom);
            }

            break;
        }

        default :
            // Theoretically we can not get here because all month
            // calendar notification codes that have a tag are
            // accounted for.
            break;
    }

    return ReplyTrue;
}

/**
 * Handles the WM_NOTIFY messages for an up down control that the Rexx
 * programmer has connected.
 *
 * There is currently only one notify message for an up-down; UDN_DELTAPOS, so
 * the processing is straight forward.
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
    LPNMUPDOWN    pUPD = (LPNMUPDOWN)lParam;

    RexxArrayObject args = c->ArrayOfFour(c->Int32(pUPD->iPos), c->Int32(pUPD->iDelta),
                                          idFrom2rexxArg(c, lParam), hwndFrom2rexxArg(c, lParam));

    RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

    if ( msgReplyIsGood(c, pcpbd, msgReply, methodName, false) )
    {
        if ( msgReply != TheFalseObj )
        {
            if ( c->IsOfType(msgReply, "BUFFER") )
            {
                PDELTAPOSREPLY pdpr = (PDELTAPOSREPLY)c->BufferData((RexxBufferObject)msgReply);
                if ( pdpr->cancel )
                {
                    setWindowPtr(GetParent(pUPD->hdr.hwndFrom), DWLP_MSGRESULT, 1);
                }
                else
                {
                    pUPD->iDelta = pdpr->newDelta;
                }
            }
            else
            {
                wrongClassReplyException(c, methodName, "PositionChangeReplyBuffer");
            }
        }
    }
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
 */
MsgReplyType searchNotifyTable(WPARAM wParam, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    MESSAGETABLEENTRY *m = pcpbd->enCSelf->notifyMsgs;
    if ( m == NULL )
    {
        return ContinueProcessing;
    }

    uint32_t code = ((NMHDR *)lParam)->code;
    size_t tableSize = pcpbd->enCSelf->nmNextIndex;
    register size_t i = 0;

    for ( i = 0; i < tableSize; i++ )
    {
        if ( ((wParam & m[i].wpFilter) == m[i].wParam) && ((code & m[i].lpfilter) == (uint32_t)m[i].lParam) )
        {
            RexxThreadContext *c = pcpbd->dlgProcContext;

            char   tmpBuffer[20];
            char  *np = NULL;
            int    item = OOD_INVALID_ITEM_ID;
            HANDLE handle = NULL;

            switch ( m[i].tag & TAG_CTRLMASK )
            {
                case TAG_NOTHING :
                    break;

                case TAG_LISTVIEW :
                {
                    MsgReplyType ret = processLVN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);
                    if ( ret == ContinueSearching )
                    {
                        continue;
                    }
                    return ret;
                }

                // TODO should we terminate the interpreter if checkForCondition() returns true??
                // We could let the user decide through use of the .application
                // object.

                case TAG_TAB :
                    return processTCN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);

                case TAG_UPDOWN :
                    return processUDN(c, m[i].rexxMethod, lParam, pcpbd);

                case TAG_MONTHCALENDAR :
                    return processMCN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);

                case TAG_DATETIMEPICKER :
                    return processDTN(c, m[i].rexxMethod, m[i].tag, code, lParam, pcpbd);

                default :
                    break;
            }

            /* do we have an end label edit for tree or list view? */
            if ( (code == TVN_ENDLABELEDIT) && ((TV_DISPINFO *)lParam)->item.pszText )
            {
                np = ((TV_DISPINFO *)lParam)->item.pszText;
                handle = ((TV_DISPINFO *)lParam)->item.hItem;
            }
            else if ( (code == LVN_ENDLABELEDIT) && ((LV_DISPINFO *)lParam)->item.pszText )
            {
                np = ((LV_DISPINFO *)lParam)->item.pszText;
                item = ((LV_DISPINFO *)lParam)->item.iItem;
            }
            /* do we have a tree expand/collapse? */
            else if ( (code == TVN_ITEMEXPANDED) || (code == TVN_ITEMEXPANDING) )
            {
                handle = ((NM_TREEVIEW *)lParam)->itemNew.hItem;
                if ( ((NM_TREEVIEW *)lParam)->itemNew.state & TVIS_EXPANDED ) np = "EXPANDED";
                else np = "COLLAPSED";
            }
            /* do we have a key_down? */
            else if ( (code == TVN_KEYDOWN) || (code == LVN_KEYDOWN) || (code == TCN_KEYDOWN) )
            {
                lParam = (ULONG)((TV_KEYDOWN *)lParam)->wVKey;
            }
            /* do we have a list drag and drop? */
            else if ( (code == LVN_BEGINDRAG) || (code == LVN_BEGINRDRAG) )
            {
                item = ((NM_LISTVIEW *)lParam)->iItem;
                wParam = ((NMHDR *)lParam)->idFrom;
                sprintf(tmpBuffer, "%d %d", ((NM_LISTVIEW *)lParam)->ptAction.x, ((NM_LISTVIEW *)lParam)->ptAction.y);
                np = tmpBuffer;
            }
            /* do we have a tree drag and drop? */
            else if ( (code == TVN_BEGINDRAG) || (code == TVN_BEGINRDRAG) )
            {
                handle = ((NM_TREEVIEW *)lParam)->itemNew.hItem;
                wParam = ((NMHDR *)lParam)->idFrom;
                sprintf(tmpBuffer, "%d %d", ((NM_TREEVIEW *)lParam)->ptDrag.x, ((NM_TREEVIEW *)lParam)->ptDrag.y);
                np = tmpBuffer;
            }
            else if ( code == BCN_HOTITEMCHANGE )
            {
                /* Args to ooRexx will be the control ID, entering = true or false. */
                lParam = (((NMBCHOTITEM *)lParam)->dwFlags & HICF_ENTERING) ? 1 : 0;
            }

            return genericInvokeDispatch(pcpbd, m[i].rexxMethod, wParam, lParam, np, handle, item, m[i].tag);
        }
    }

    return ReplyFalse;
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
    register size_t i = 0;

    for ( i = 0; i < tableSize; i++ )
    {
        if ( (msg & m[i].msgFilter) == m[i].msg && (wParam & m[i].wpFilter) == m[i].wParam && (lParam & m[i].lpfilter) == m[i].lParam )
        {
            RexxThreadContext *c = pcpbd->dlgProcContext;
            RexxArrayObject args;

            char  *np = NULL;
            char  *method = m[i].rexxMethod;
            int    item = OOD_INVALID_ITEM_ID;
            HANDLE handle = NULL;

            switch ( m[i].tag & TAG_CTRLMASK )
            {
                case TAG_NOTHING :
                    break;

                case TAG_DIALOG :
                    switch ( m[i].tag & TAG_FLAGMASK )
                    {
                        case TAG_HELP :
                        {
                            LPHELPINFO phi = (LPHELPINFO)lParam;

                            np = (phi->iContextType == HELPINFO_WINDOW ? "WINDOW" : "MENU");

                            args = c->ArrayOfFour(c->Int32(phi->iCtrlId), c->String(np),
                                                  c->Int32(phi->MousePos.x), c->Int32(phi->MousePos.x));
                            c->ArrayPut(args, c->Uintptr(phi->dwContextId), 5);

                            return invokeDispatch(c, pcpbd->rexxSelf, c->String(method), args);
                        }
                        break;

                        case TAG_CONTEXTMENU :
                        {
                            /* On WM_CONTEXTMENU, if the message is
                             * generated by the keyboard (say SHIFT-F10)
                             * then the x and y coordinates are sent as -1
                             * and -1. Args to ooRexx: hwnd, x, y
                             *
                             * Note that the current context menu processing is
                             * dependent on the event handler *not* running in
                             * the window message processing loop.  So
                             * inovkeDispatch() is required.  If this is
                             * changed, then the code using WM_USER_CONTEXT_MENU
                             * needs to be reviewed.
                             */
                            args = c->ArrayOfThree(pointer2string(c, (void *)wParam), c->Int32(GET_X_LPARAM(lParam)),
                                                   c->Int32(GET_Y_LPARAM(lParam)));
                            invokeDispatch(c, pcpbd->rexxSelf, c->String(method), args);
                            return ReplyTrue;
                        }
                        break;

                        case TAG_MENUCOMMAND :
                        {
                            /* Args to ooRexx: index, hMenu.
                             *
                             * TODO we should send the Rexx Menu object rather
                             * than the handle. This would invole constructing
                             * the object, or grabbing it from the Menu user
                             * words.
                             */
                            args = c->ArrayOfTwo(c->WholeNumber(wParam), c->NewPointer((POINTER)lParam));
                            return invokeDispatch(c, pcpbd->rexxSelf, c->String(method), args);
                        }
                        break;

                        case TAG_SYSMENUCOMMAND :
                        {
                            /* Args to ooRexx: The SC_xx command name, x, y
                             */
                            RexxObjectPtr sc_cmd = sc2rexx(c, wParam);
                            RexxObjectPtr x, y;


                            if ( GET_Y_LPARAM(lParam) == -1 )
                            {
                                x = TheNegativeOneObj;
                                y = TheNegativeOneObj;
                            }
                            else if ( GET_Y_LPARAM(lParam) == 0 )
                            {
                                x = TheZeroObj;
                                y = TheZeroObj;
                            }
                            else
                            {
                                x = c->Int32(GET_X_LPARAM(lParam));
                                y = c->Int32(GET_Y_LPARAM(lParam));
                            }

                            MsgReplyType reply = ReplyFalse;
                            RexxArrayObject args = c->ArrayOfThree(sc_cmd, x, y);

                            RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, method, args);

                            msgReply = requiredBooleanReply(c, pcpbd, msgReply, method, false);
                            if ( msgReply == TheTrueObj )
                            {
                                setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, 0);
                                reply = ReplyTrue;
                            }
                            else
                            {
                                setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, 1);
                            }
                            return reply;
                        }
                        break;

                        case TAG_MENUMESSAGE :
                        {
                            // Right now there is only WM_INITMENU and
                            // WM_INITMENUPOPUP, but in the future there could
                            // be more.  Both of these messages are handled in a
                            // similar way. TODO would really be nice to send
                            // the Rexx menu object itself.

                            MsgReplyType      reply = ReplyFalse;
                            RexxPointerObject rxHMenu = c->NewPointer((POINTER)wParam);
                            RexxObjectPtr     msgReply;

                            switch ( msg )
                            {
                                case WM_INITMENU :
                                    // Args to ooRexx: hMenu as a pointer.
                                    args = c->ArrayOfOne(rxHMenu);
                                    break;

                                case WM_INITMENUPOPUP :
                                    // Args to ooRexx: pos, isSystemMenu, hMenu
                                    // as a pointer. Position needs to be
                                    // converted to 1-based.
                                    args = c->ArrayOfThree(c->Int32(LOWORD(lParam) + 1),
                                                           c->Logical(HIWORD(lParam)),
                                                           rxHMenu);
                                    break;

                                default :
                                    return reply;
                            }

                            msgReply = c->SendMessage(pcpbd->rexxSelf, method, args);
                            msgReply = requiredBooleanReply(c, pcpbd, msgReply, method, false);

                            if ( msgReply == TheTrueObj )
                            {
                                setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, 0);
                                reply = ReplyTrue;
                            }
                            else
                            {
                                setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, 1);
                            }
                            return reply;
                        }
                        break;

                        default :
                            break;
                    }
                    break;

                case TAG_MOUSE :
                    return processMouseMsg(c, method, m[i].tag, msg, wParam, lParam, pcpbd);

                default :
                    break;
            }

            if ( msg == WM_HSCROLL || msg == WM_VSCROLL )
            {
                handle = (HANDLE)lParam;
            }
            else if ( msg == WM_ACTIVATE)
            {
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
                RexxArrayObject args = c->ArrayOfFour(flag, hwnd, hFocus, isMinimized);

                RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, method, args);

                msgReply = requiredBooleanReply(c, pcpbd, msgReply, method, false);
                if ( msgReply == TheTrueObj )
                {
                    reply = ReplyTrue;
                }
                return reply;
            }
            else if ( msg == WM_SIZING )
            {
                /* Args to ooRexx: The sizing RECT, WMSZ_xx keyword.
                 */
                PRECT wRect = (PRECT)lParam;

                RexxStringObject wmsz = wmsz2string(c, wParam);
                RexxObjectPtr rect = rxNewRect(c, wRect);

                MsgReplyType reply = ReplyFalse;
                RexxArrayObject args = c->ArrayOfTwo(rect, wmsz);

                RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, method, args);

                msgReply = requiredBooleanReply(c, pcpbd, msgReply, method, false);
                if ( msgReply == TheTrueObj )
                {
                    PRECT r = (PRECT)c->ObjectToCSelf(rect);
                    wRect->top = r->top;
                    wRect->left = r->left;
                    wRect->bottom = r->bottom;
                    wRect->right = r->right;
                    reply = ReplyTrue;
                }
                return reply;
            }

            return genericInvokeDispatch(pcpbd, method, wParam, lParam, np, handle, item, m[i].tag);
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
        baseClassIntializationException(c);
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
    pcen->commandMsgs[index].lpfilter = lpFilter;
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
    pcen->notifyMsgs[index].lpfilter = lpFilter;
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
    pcen->miscMsgs[index].msgFilter = 0xFFFFFFFF;
    pcen->miscMsgs[index].wParam = wParam;
    pcen->miscMsgs[index].wpFilter = wpFilter;
    pcen->miscMsgs[index].lParam = lParam;
    pcen->miscMsgs[index].lpfilter = lpFilter;
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

    pcen->rexxSelf = self;

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

/**
 * Convert a keyword to the proper scroll bar notification code.
 *
 * We know the keyword arg position is 2.  The MonthCalendar control is post
 * ooRexx 4.0.1 so we raise an exception on error.
 */
static bool keyword2sbn(CSTRING keyword, uint32_t *flag)
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
 * Convert a scroll bar notification code to a method name.
 *
 * For SB_LINEUP   / SB_LINELEFT  -> onUp
 * For SB_LINEDOWN / SB_LINERIGHT -> onDown
 * For SB_PAGEUP   / SB_PAGELEFT  -> onPageUp
 * For SB_PAGEDOWN / SB_PAGERIGHT -> onPageDown
 * For SB_TOP      / SB_LEFT      -> onTop
 * For SB_BOTTOM   / SB_RIGHT     -> onBottom
 */
inline CSTRING sbn2name(uint32_t sbn)
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
 * Convert a keyword to the proper list view notification code.
 *
 *
 */
static bool keyword2lvn(RexxMethodContext *c, CSTRING keyword, uint32_t *code, uint32_t *tag, bool *isDefEdit, logical_t willReply)
{
    uint32_t lvn = 0;

    *isDefEdit = false;
    *tag = 0;

    if ( StrCmpI(keyword,      "CHANGING")    == 0 ) lvn = LVN_ITEMCHANGING;
    else if ( StrCmpI(keyword, "CHANGED")     == 0 ) lvn = LVN_ITEMCHANGED;
    else if ( StrCmpI(keyword, "INSERTED")    == 0 ) lvn = LVN_INSERTITEM;
    else if ( StrCmpI(keyword, "DELETE")      == 0 ) lvn = LVN_DELETEITEM;
    else if ( StrCmpI(keyword, "DELETEALL")   == 0 ) lvn = LVN_DELETEALLITEMS;
    else if ( StrCmpI(keyword, "BEGINEDIT")   == 0 ) lvn = LVN_BEGINLABELEDIT;
    else if ( StrCmpI(keyword, "ENDEDIT")     == 0 ) lvn = LVN_ENDLABELEDIT;
    else if ( StrCmpI(keyword, "BEGINDRAG")   == 0 ) lvn = LVN_BEGINDRAG;
    else if ( StrCmpI(keyword, "BEGINRDRAG")  == 0 ) lvn = LVN_BEGINRDRAG;
    else if ( StrCmpI(keyword, "ACTIVATE")    == 0 ) lvn = LVN_ITEMACTIVATE;
    else if ( StrCmpI(keyword, "KEYDOWN")     == 0 ) lvn = LVN_KEYDOWN;
    else if ( StrCmpI(keyword, "DEFAULTEDIT") == 0 ) *isDefEdit = true;
    else if ( StrCmpI(keyword, "CLICK") == 0 )
    {
        lvn = NM_CLICK;
        *tag = TAG_LISTVIEW;
    }
    else if ( StrCmpI(keyword, "DBLCLK") == 0 )
    {
        lvn = NM_DBLCLK;
        *tag = TAG_LISTVIEW;
    }
    else if ( StrCmpI(keyword, "CHECKBOXCHANGED") == 0 )
    {
        lvn = LVN_ITEMCHANGED;
        *tag = TAG_LISTVIEW | TAG_STATECHANGED | TAG_CHECKBOXCHANGED;
    }
    else if ( StrCmpI(keyword, "COLUMNCLICK") == 0 )
    {
        lvn = LVN_COLUMNCLICK;
        *tag = TAG_LISTVIEW;
    }
    else if ( StrCmpI(keyword, "FOCUSCHANGED") == 0 )
    {
        lvn = LVN_ITEMCHANGED;
        *tag = TAG_LISTVIEW | TAG_STATECHANGED | TAG_FOCUSCHANGED;
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

    if ( *tag != 0 && willReply )
    {
        *tag = *tag | TAG_REPLYFROMREXX;
    }

    *code = lvn;
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
        case LVN_ITEMACTIVATE   : return "onActivate";
        case NM_CLICK           : return "onClick";
        case LVN_KEYDOWN :
            if ( tag & TAG_LISTVIEW )
            {
                return "onKeyDownEx";
            }
            else
            {
                return "onKeydown";
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
 * Determines if the reply to a date time picker notification code has any
 * meaning, or if it is ignored.  For the notifications listed, the Rexx dialog
 * object method is always invoked directly, i.e., the user must always reply.
 */
inline bool dtpnReplySignificant(uint32_t dtpn)
{
    return (dtpn == DTN_FORMAT) || (dtpn == DTN_FORMATQUERY) ||
           (dtpn == DTN_USERSTRING) || (dtpn == DTN_WMKEYDOWN);
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

        RexxArrayObject args = getKeyEventRexxArgs(c, wParam,
                                                   lParam & KEY_ISEXTENDED ? true : false,
                                                   pSCData->pcdc == NULL   ? NULL : pSCData->pcdc->rexxSelf);
        invokeDispatch(c, pSCData->pcpbd->rexxSelf, c->String(pMethod), args);
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


/** EventNotification::connectCommandEvents()
 *
 *  Connects a Rexx dialog method to the WM_COMMAND event notifications sent by
 *  a Windows dialog control to its parent.
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
RexxMethod3(int32_t, en_connectCommandEvents, RexxObjectPtr, rxID, CSTRING, methodName, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    int32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1, true) )
    {
        return -1;
    }
    if ( *methodName == '\0' )
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_null, TheTwoObj);
        return 1;
    }
    return (addCommandMessage(pcen, context, id, 0x0000FFFF, 0, 0, methodName, 0) ? 0 : 1);
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
            OPTIONAL_CSTRING, methodName, OPTIONAL_logical_t, willReply, CSELF, pCSelf)
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
    if ( ! keyword2sbn(event, &notificationCode) )
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
        methodName = sbn2name(notificationCode);
    }

    uint32_t tag = willReply ? TAG_REPLYFROMREXX : TAG_NOTHING;

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

    logical_t willReply = FALSE;

    RexxObjectPtr _willReply = c->ArrayAt(args, 14);
    if ( _willReply != NULLOBJECT )
    {
        if ( ! c->Logical(_willReply, &willReply) )
        {
            wrongArgValueException(context->threadContext, 14, ".true or .false", _willReply);
            goto err_out;
        }
    }

    uint32_t tag = willReply ? TAG_REPLYFROMREXX : TAG_NOTHING;

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
            OPTIONAL_logical_t, willReply, CSELF, pCSelf)
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

    if ( *msg == '\0' )
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_null, c->WholeNumber(2));
        goto err_out;
    }

    uint32_t tag = willReply ? TAG_REPLYFROMREXX : TAG_NOTHING;

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
 *                      CHANGED
 *                      CHANGING
 *                      COLUMNCLICK
 *                      DEFAULTEDIT
 *                      DELETE
 *                      DELETEALL
 *                      ENDEDIT
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
 */
RexxMethod5(RexxObjectPtr, en_connectListViewEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, OPTIONAL_logical_t, willReply, CSELF, pCSelf)
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
        if ( ! addNotifyMessage(pcen, context, id, 0xFFFFFFFF, LVN_BEGINLABELEDIT, 0xFFFFFFFF, "DefListEditStarter", 0) )
        {
            return TheNegativeOneObj;
        }
        if ( ! addNotifyMessage(pcen, context, id, 0xFFFFFFFF, LVN_ENDLABELEDIT, 0xFFFFFFFF, "DefListEditHandler", 0) )
        {
            return TheNegativeOneObj;
        }
        return TheZeroObj;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = lvn2name(notificationCode, tag);
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
            OPTIONAL_CSTRING, methodName, OPTIONAL_logical_t, _willReply, CSELF, pCSelf)
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
    bool willReply = argumentOmitted(4) || _willReply;

    if ( dtpnReplySignificant(notificationCode) )
    {
        tag |= TAG_REPLYFROMREXX;
    }
    else
    {
          tag |= willReply ? TAG_REPLYFROMREXX : 0;
    }

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
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
            OPTIONAL_CSTRING, methodName, OPTIONAL_logical_t, _willReply, CSELF, pCSelf)
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

    uint32_t tag = TAG_MONTHCALENDAR;
    bool willReply = argumentOmitted(4) || _willReply;

    if ( notificationCode == MCN_GETDAYSTATE )
    {
        tag |= TAG_REPLYFROMREXX;
    }
    else
    {
        tag |= willReply ? TAG_REPLYFROMREXX : 0;
    }

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
    {
        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
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
 *  @param  _tag         [optional]  A tag that allows a further differentiation
 *                       between messages.  This is an internal mechanism not to
 *                       be documented publicly.
 *
 *  @return  0 on success, 1 on failure.
 *
 *  @note     Method name can not be the empty string. The Window message,
 *            WPARAM, and LPARAM arguments can not all be 0.
 *
 *            If incorrect arguments are detected a syntax condition is raised.
 *
 *  @remarks  Although it would make more sense to return true on succes and
 *            false on failure, there is too much old code that relies on 0 for
 *            success and 1 for error.
 *
 *            Then only reason we pass methodName to parseWinMessageFilter() is
 *            to have the function check for the emtpy string.  We use
 *            methodName as is if there is no error.
 */
RexxMethod9(uint32_t, en_addUserMessage, CSTRING, methodName, CSTRING, wm, OPTIONAL_CSTRING, _wmFilter,
            OPTIONAL_RexxObjectPtr, wp, OPTIONAL_CSTRING, _wpFilter, OPTIONAL_RexxObjectPtr, lp, OPTIONAL_CSTRING, _lpFilter,
            OPTIONAL_CSTRING, _tag, CSELF, pCSelf)
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

    uint64_t filter;
    uint32_t tag = 0;
    if ( argumentExists(8) )
    {
        if ( ! rxStr2Number(context, _tag, &filter, 8) )
        {
            goto done_out;
        }
        tag = (ULONG)filter;
    }

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


