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
#include "oodData.hpp"
#include "oodPropertySheetDialog.hpp"

static LRESULT CALLBACK RexxChildDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
 *           AttachThread().
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
 *           pulled out of the window words.
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
        // things should then (hopefully) unwind cleanly.
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

            assignPSDThreadContext(pcpsd, pcpbd->dlgProcContext);

            if ( setPropSheetHook(pcpsd) )
            {
                SetLastError(0);
                INT_PTR ret = PropertySheet((PROPSHEETHEADER *)wParam);
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

        case WM_USER_HOOK:
        {
            SUBCLASSDATA *pData = (SUBCLASSDATA *)lParam;

            pData->dlgProcContext = pcpbd->dlgProcContext;
            pData->rexxDialog = pcpbd->rexxSelf;

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

static RexxStringObject sc2string(RexxThreadContext *c, WPARAM wParam)
{
    CSTRING s;

    switch ( wParam & 0xFFF0 )
    {
        case SC_SIZE         : s = "SIZE";
        case SC_MOVE         : s = "MOVE";
        case SC_MINIMIZE     : s = "MINIMIZE";
        case SC_MAXIMIZE     : s = "MAXIMIZE";
        case SC_NEXTWINDOW   : s = "NEXTWINDOW";
        case SC_PREVWINDOW   : s = "PREVWINDOW";
        case SC_CLOSE        : s = "CLOSE";
        case SC_VSCROLL      : s = "VSCROLL";
        case SC_HSCROLL      : s = "HSCROLL";
        case SC_MOUSEMENU    : s = "MOUSEMENU ";
        case SC_KEYMENU      : s = "KEYMENU";
        case SC_ARRANGE      : s = "ARRANGE";
        case SC_RESTORE      : s = "RESTORE";
        case SC_TASKLIST     : s = "TASKLIST";
        case SC_SCREENSAVE   : s = "SCREENSAVE";
        case SC_HOTKEY       : s = "HOTKEY";
        case SC_DEFAULT      : s = "DEFAULT";
        case SC_MONITORPOWER : s = "MONITORPOWER";
        case SC_CONTEXTHELP  : s = "CONTEXTHELP";
        case SC_SEPARATOR    : s = "SEPARATOR";

        // SCF_ISSECURE, only defined if WINVER >= 0x0600
        case 0x00000001      : s = "ISSECURE";
        default              : s = "UNKNOWN";
    }
    return c->String(s);
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
 *           sheets hang.  The destroyModalPropSheet() essentially does a
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

        if ( pcpbd->isPropSheetDlg && ! ((pCPropertySheetDialog)(pcpbd->dlgPrivate))->modeless  )
        {
            destroyModalPropSheet((pCPropertySheetDialog)pcpbd->dlgPrivate, hDlg, t);
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
 *           a.) The reply to the window message is ignored anyway.
 *
 *           b.) To maintain backward compatibility with event connections, so
 *           that the Rexx programmer does not inadvertently block the message
 *           loop.
 */
static MsgReplyType invokeDispatch(RexxThreadContext *c, RexxObjectPtr obj, RexxStringObject method, RexxArrayObject args)
{
    c->SendMessage2(obj, "STARTWITH", method, args);
    return ReplyTrue;
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
 * dialog control, if a control iniated the message.  For menu items and
 * accelerators, it is always 0. So, converting to a pseudo pointer is always
 * the correct thing to do.
 */
inline MsgReplyType genericCommandInvoke(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING methodName,
                                         WPARAM wParam, LPARAM lParam)
{
    return invokeDispatch(c, pcpbd->rexxSelf,
                          c->String(methodName),
                          c->ArrayOfTwo(c->Uintptr(wParam), pointer2string(c, (void *)lParam)));
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
                                     char *np, HANDLE handle, int item)
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

    return invokeDispatch(c, pcpbd->rexxSelf, method, args);
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

    size_t tableSize = pcpbd->enCSelf->cmSize;
    register size_t i = 0;

    for ( i = 0; i < tableSize; i++ )
    {
        if ( ((wParam & m[i].wpFilter) == m[i].wParam) && ((lParam & m[i].lpfilter) == (uint32_t)m[i].lParam) )
        {
            return genericCommandInvoke(pcpbd->dlgProcContext, pcpbd, m[i].rexxMethod, wParam, lParam);
        }
    }
    return ContinueProcessing;
}


MsgReplyType processDTN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    RexxObjectPtr rexxReply;
    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr hwndFrom = hwndFrom2rexxArg(c, lParam);

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

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
            checkForCondition(c, false);

            return ReplyFalse;
        }

        case DTN_FORMAT:
        {
            LPNMDATETIMEFORMAT pFormat = (LPNMDATETIMEFORMAT)lParam;

            RexxObjectPtr dt;
            sysTime2dt(c, &(pFormat->st), &dt, dtFull);

            RexxArrayObject args = c->ArrayOfFour(c->String(pFormat->pszFormat), dt, idFrom, hwndFrom);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

            if ( ! checkForCondition(c, false) )
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

            return ReplyFalse;
        }

        case DTN_FORMATQUERY:
        {
            LPNMDATETIMEFORMATQUERY pQuery = (LPNMDATETIMEFORMATQUERY)lParam;

            RexxObjectPtr _size = rxNewSize(c, 0, 0);

            RexxArrayObject args = c->ArrayOfFour(c->String(pQuery->pszFormat), _size, idFrom, hwndFrom);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

            if ( checkForCondition(c, false) )
            {
                // TODO note this is a special case test of ending the dialog on
                // a condition raised.  Need to always do the same thing, end
                // the dialog or not end the dialog.
                endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
                return ReplyTrue;
            }

            PSIZE size = (PSIZE)c->ObjectToCSelf(_size);

            pQuery->szMax.cx = size->cx;
            pQuery->szMax.cy = size->cy;

            return ReplyFalse;
        }

        case DTN_USERSTRING:
        {
            LPNMDATETIMESTRING pdts = (LPNMDATETIMESTRING)lParam;

            RexxDirectoryObject d = (RexxDirectoryObject)rxNewBuiltinObject(c, "DIRECTORY");
            c->DirectoryPut(d, c->String(pdts->pszUserString), "USERSTRING");
            c->DirectoryPut(d, TheNilObj, "DATETIME");
            c->DirectoryPut(d, TheFalseObj, "VALID");

            // Fill in the date time string struct with error values.
            dt2sysTime(c, NULLOBJECT, &(pdts->st), dtNow);
            pdts->dwFlags = GDT_ERROR;

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, c->ArrayOfThree(d, idFrom, hwndFrom));

            if ( checkForCondition(c, false) )
            {
                return ReplyFalse;
            }

            RexxObjectPtr dt = c->DirectoryAt(d, "DATETIME");
            if ( ! c->IsOfType(dt, "DATETIME") )
            {
                wrongObjInDirectoryException(c, 1, "DATETIME", "a DateTime object", dt);
                checkForCondition(c, false);
                return ReplyFalse;
            }

            if ( ! dt2sysTime(c, dt, &(pdts->st), dtFull) )
            {
                checkForCondition(c, false);
                return ReplyFalse;
            }

            if ( isShowNoneDTP(pdts->nmhdr.hwndFrom) )
            {
                RexxObjectPtr _valid = c->DirectoryAt(d, "VALID");
                int32_t val = getLogical(c, _valid);

                if ( val == -1 )
                {
                    wrongObjInDirectoryException(c, 1, "VALID", "Logical", _valid);
                    checkForCondition(c, false);
                    return ReplyFalse;
                }
                pdts->dwFlags = (val == 1 ? GDT_VALID : GDT_NONE);
            }
            else
            {
                pdts->dwFlags = GDT_VALID;
            }
            return ReplyFalse;
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

            if ( ! checkForCondition(c, false) )
            {
                if ( rexxReply != dt )
                {
                    if ( c->IsOfType(rexxReply, "DATETIME") )
                    {
                        dt2sysTime(c, rexxReply, &(pQuery->st), dtFull);
                    }
                    else
                    {
                        wrongClassReplyException(c, "DateTime");
                    }
                    checkForCondition(c, false);
                }
            }

            return ReplyFalse;
        }

        case DTN_CLOSEUP:
        case DTN_DROPDOWN:
        case NM_KILLFOCUS:
        case NM_SETFOCUS:
        {
            return genericNotifyInvoke(c, pcpbd, methodName, idFrom, hwndFrom);
        }

        default :
            // Theoretically we can not get here because all date time
            // picker notification codes that have a tag are accounted
            // for.
            break;
    }

    return ReplyFalse;
}


MsgReplyType processLVN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    char          tmpBuffer[20];
    RexxObjectPtr rexxReply;
    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);

    switch ( code )
    {
        case NM_CLICK:
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

            RexxArrayObject args = c->ArrayOfFour(idFrom, c->Int32(pIA->iItem), c->Int32(pIA->iSubItem), c->String(tmpBuffer));

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
            checkForCondition(c, false);

            return ReplyFalse;
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

                    rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
                    checkForCondition(c, false);

                    return ReplyFalse;
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

                    rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
                    checkForCondition(c, false);

                    return ReplyFalse;
                }
                else if ( matchSelect(tag, pLV) )
                {
                    p = (pLV->uNewState & LVIS_SELECTED) ? "SELECTED" : "UNSELECTED";

                    RexxArrayObject args = c->ArrayOfThree(idFrom, item, c->String(p));

                    rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

                    if ( checkForCondition(c, false) )
                    {
                        return ReplyFalse;
                    }

                    return ContinueSearching;  // Not sure if this is wise with the C++ API
                }
                else if ( matchFocus(tag, pLV) )
                {
                    p = (pLV->uNewState & LVIS_FOCUSED) ? "FOCUSED" : "UNFOCUSED";

                    RexxArrayObject args = c->ArrayOfThree(idFrom, item, c->String(p));

                    rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

                    if ( checkForCondition(c, false) )
                    {
                        return ReplyFalse;
                    }

                    return ContinueSearching;  // Not sure if this is wise with the C++ API
                }
                else
                {
                    // This message in the message table does not match, keep searching.
                    return ContinueSearching;
                }
            }

            break;
        }

        default :
            break;
    }

    return ReplyFalse;
}

MsgReplyType processMCN(RexxThreadContext *c, CSTRING methodName, uint32_t tag, uint32_t code, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    RexxObjectPtr rexxReply;
    RexxObjectPtr idFrom = idFrom2rexxArg(c, lParam);
    RexxObjectPtr hwndFrom = hwndFrom2rexxArg(c, lParam);

    switch ( code )
    {
        case MCN_GETDAYSTATE :
        {
            LPNMDAYSTATE pDayState = (LPNMDAYSTATE)lParam;

            RexxObjectPtr dt = NULLOBJECT;
            sysTime2dt(c, &(pDayState->stStart), &dt, dtDate);

            RexxArrayObject args = c->ArrayOfFour(dt, c->Int32(pDayState->cDayState), idFrom, hwndFrom);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);

            if ( checkForCondition(c, false) )
            {
                return ReplyFalse;
            }

            if ( rexxReply != NULLOBJECT && c->IsOfType(rexxReply, "BUFFER") )
            {
                pDayState->prgDayState = (MONTHDAYSTATE *)c->BufferData((RexxBufferObject)rexxReply);
                return ReplyTrue;
            }
            return ReplyFalse;
        }

        case MCN_SELECT :
        case MCN_SELCHANGE :
        {
            LPNMSELCHANGE pSelChange = (LPNMSELCHANGE)lParam;

            RexxObjectPtr dtStart;
            sysTime2dt(c, &(pSelChange->stSelStart), &dtStart, dtDate);

            RexxObjectPtr dtEnd;
            sysTime2dt(c, &(pSelChange->stSelEnd), &dtEnd, dtDate);

            RexxArrayObject args = c->ArrayOfFour(dtStart, dtEnd, idFrom, hwndFrom);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
            checkForCondition(c, false);
            return ReplyTrue;
        }

        case MCN_VIEWCHANGE :
        {
            LPNMVIEWCHANGE pViewChange = (LPNMVIEWCHANGE)lParam;

            RexxStringObject newView = mcnViewChange2rexxString(c, pViewChange->dwNewView);
            RexxStringObject oldView = mcnViewChange2rexxString(c, pViewChange->dwOldView);

            RexxArrayObject args = c->ArrayOfFour(newView, oldView, idFrom, hwndFrom);

            rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
            checkForCondition(c, false);
            return ReplyTrue;
        }

        case NM_RELEASEDCAPTURE :
        {
            return genericNotifyInvoke(c, pcpbd, methodName, idFrom, hwndFrom);
        }

        default :
            // Theoretically we can not get here because all month
            // calendar notification codes that have a tag are
            // accounted for.
            break;
    }

    return ReplyFalse;
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

    if ( ! checkForCondition(c, false) )
    {
        if ( msgReply != NULLOBJECT && msgReply != TheFalseObj && c->IsOfType(msgReply, "BUFFER") )
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
    size_t tableSize = pcpbd->enCSelf->nmSize;
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
            /* do we have a column click in a report? */
            else if ( code == LVN_COLUMNCLICK )
            {
                wParam = ((NMHDR *)lParam)->idFrom;
                lParam = (ULONG)((NM_LISTVIEW *)lParam)->iSubItem;  /* which column is pressed */
            }
            else if ( code == BCN_HOTITEMCHANGE )
            {
                /* Args to ooRexx will be the control ID, entering = true or false. */
                lParam = (((NMBCHOTITEM *)lParam)->dwFlags & HICF_ENTERING) ? 1 : 0;
            }

            return genericInvokeDispatch(pcpbd, m[i].rexxMethod, wParam, lParam, np, handle, item);
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

    size_t tableSize = pcpbd->enCSelf->mmSize;
    register size_t i = 0;

    for ( i = 0; i < tableSize; i++ )
    {
        if ( (msg & m[i].msgFilter) == m[i].msg && (wParam & m[i].wpFilter) == m[i].wParam && (lParam & m[i].lpfilter) == m[i].lParam )
        {
            RexxThreadContext *c = pcpbd->dlgProcContext;
            RexxArrayObject args;

            char  *np = NULL;
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

                            return invokeDispatch(c, pcpbd->rexxSelf, c->String(m[i].rexxMethod), args);
                        }
                        break;

                        case TAG_CONTEXTMENU :
                        {
                            /* On WM_CONTEXTMENU, if the message is
                             * generated by the keyboard (say SHIFT-F10)
                             * then the x and y coordinates are sent as -1
                             * and -1. Args to ooRexx: hwnd, x, y
                             */
                            args = c->ArrayOfThree(pointer2string(c, (void *)wParam), c->Int32(GET_X_LPARAM(lParam)),
                                                   c->Int32(GET_Y_LPARAM(lParam)));
                            invokeDispatch(c, pcpbd->rexxSelf, c->String(m[i].rexxMethod), args);
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
                            return invokeDispatch(c, pcpbd->rexxSelf, c->String(m[i].rexxMethod), args);
                        }
                        break;

                        case TAG_SYSMENUCOMMAND :
                        {
                            /* Args to ooRexx: The SC_xx command name, x, y
                             */
                            RexxStringObject sc_cmd = sc2string(c, wParam);
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

                            RexxObjectPtr msgReply = c->SendMessage(pcpbd->rexxSelf, m[i].rexxMethod, args);

                            if ( ! checkForCondition(c, false) )
                            {
                                if ( msgReply == TheTrueObj )
                                {
                                    setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, 0);
                                    reply = ReplyTrue;
                                }
                                else
                                {
                                    setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, 1);
                                }
                            }
                            return reply;
                        }
                        break;

                        case TAG_MENUMESSAGE :
                        {
                            // Right now there is only WM_INITMENU and WM_INITMENUPOPUP,
                            // but in the future there could be more.  Both
                            // of these messages are handled the exact same
                            // way as far as what is sent to ooRexx.

                            // Args to ooRexx: hMenu as a pointer.
                            MsgReplyType reply = ReplyFalse;
                            RexxPointerObject rxHMenu = c->NewPointer((POINTER)wParam);

                            RexxObjectPtr msgReply = c->SendMessage1(pcpbd->rexxSelf, m[i].rexxMethod, rxHMenu);

                            if ( ! checkForCondition(c, false) )
                            {
                                if ( msgReply == TheTrueObj )
                                {
                                    setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, 0);
                                    reply = ReplyTrue;
                                }
                                else
                                {
                                    setWindowPtr(pcpbd->hDlg, DWLP_MSGRESULT, 1);
                                }
                            }
                            return reply;
                        }
                        break;

                        default :
                            break;
                    }
                    break;

                default :
                    break;
            }

            if ( msg == WM_HSCROLL || msg == WM_VSCROLL )
            {
                handle = (HANDLE)lParam;
            }

            return genericInvokeDispatch(pcpbd, m[i].rexxMethod, wParam, lParam, np, handle, item);
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
 * @remarks  The command message table is allocated during the plain base dialog
 *           init process, so we do not need to check that it has been
 *           allocated.
 *
 *           Caller must ensure that 'prog' is not an empty string and that
 *           winMsg, wParam, lParam are not all 0.  TODO need to recheck this.
 */
bool addCommandMessage(pCEventNotification pcen, WPARAM wParam, ULONG_PTR wpFilter, LPARAM lParam, ULONG_PTR lpFilter,
                       CSTRING method, uint32_t tag)
{
    size_t index = pcen->cmSize;
    if ( index < MAX_COMMAND_MSGS )
    {
        pcen->commandMsgs[index].rexxMethod = (char *)LocalAlloc(LMEM_FIXED, strlen(method) + 1);
        if ( pcen->commandMsgs[index].rexxMethod == NULL )
        {
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

        pcen->cmSize++;
        return true;
    }
    else
    {
        MessageBox(0, "Command message connections have exceeded the maximum\n"
                      "number of allocated table entries.  No more command\n"
                      "message connections can be added.\n",
                   "Error", MB_OK | MB_ICONHAND);
    }
    return false;
}


/**
 * Adds an event connection for a notification (WM_NOTIFY) message to the
 * table.
 *
 * @param pcen
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
 * @remarks  Caller must ensure that 'prog' is not an empty string and that
 *           winMsg, wParam, lParam are not all 0.  TODO need to recheck this.
 */
bool addNotifyMessage(pCEventNotification pcen, WPARAM wParam, ULONG_PTR wpFilter, LPARAM lParam, ULONG_PTR lpFilter,
                      CSTRING method, uint32_t tag)
{
    if ( pcen->notifyMsgs == NULL )
    {
        pcen->notifyMsgs = (MESSAGETABLEENTRY *)LocalAlloc(LPTR, sizeof(MESSAGETABLEENTRY) * MAX_NOTIFY_MSGS);
        if ( pcen->notifyMsgs == NULL )
        {
            // TODO pass in context and raise a condition instead of this.
            MessageBox(0, "No memory available", "Error", MB_OK | MB_ICONHAND);
            return false;
        }
        pcen->nmSize = 0;
    }

    size_t index = pcen->nmSize;

    if ( index < MAX_NOTIFY_MSGS )
    {
        pcen->notifyMsgs[index].rexxMethod = (char *)LocalAlloc(LMEM_FIXED, strlen(method) + 1);
        if ( pcen->notifyMsgs[index].rexxMethod == NULL )
        {
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

        pcen->nmSize++;
        return true;
    }
    else
    {
        MessageBox(0, "Notify message connections have exceeded the maximum\n"
                      "number of allocated table entries.  No more notify\n"
                      "message connections can be added.\n",
                   "Error", MB_OK | MB_ICONHAND);
    }
    return false;
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
 * @remarks  Caller must ensure that 'prog' is not an empty string and that
 *           winMsg, wParam, lParam are not all 0.  TODO need to recheck this.
 */
bool addMiscMessage(pCEventNotification pcen, uint32_t winMsg, uint32_t wmFilter,
                    WPARAM wParam, ULONG_PTR wpFilter, LPARAM lParam, ULONG_PTR lpFilter,
                    CSTRING method, uint32_t tag)
{
    if ( pcen->miscMsgs == NULL )
    {
        pcen->miscMsgs = (MESSAGETABLEENTRY *)LocalAlloc(LPTR, sizeof(MESSAGETABLEENTRY) * MAX_MISC_MSGS);
        if ( pcen->miscMsgs == NULL )
        {
            // TODO pass in context and raise a condition instead of this.
            MessageBox(0, "No memory available", "Error", MB_OK | MB_ICONHAND);
            return false;
        }
        pcen->mmSize = 0;
    }

    size_t index = pcen->mmSize;

    if ( index < MAX_NOTIFY_MSGS )
    {
        pcen->miscMsgs[index].rexxMethod = (char *)LocalAlloc(LMEM_FIXED, strlen(method) + 1);
        if ( pcen->miscMsgs[index].rexxMethod == NULL )
        {
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

        pcen->mmSize++;
        return true;
    }
    else
    {
        MessageBox(0, "Miscellaneous message connections have exceeded the\n"
                      "maximum number of allocated table entries.  No more\n"
                      "miscellaneous message connections can be added.\n",
                   "Error", MB_OK | MB_ICONHAND);
    }
    return false;
}


bool initCommandMessagesTable(RexxMethodContext *c, pCEventNotification pcen)
{
    pcen->commandMsgs = (MESSAGETABLEENTRY *)LocalAlloc(LPTR, sizeof(MESSAGETABLEENTRY) * MAX_COMMAND_MSGS);
    if ( ! pcen->commandMsgs )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }
    pcen->cmSize = 0;

    // We don't check the return of addCommandMessage() because the message
    // table can not be full at this point, we are just starting out.  A memory
    // allocation failure, which is highly unlikely, will just be ignored.  If
    // this ooRexx process is out of memory, that will quickly show up.
    addCommandMessage(pcen, IDOK,     UINTPTR_MAX, 0, 0, "OK",     TAG_NOTHING);
    addCommandMessage(pcen, IDCANCEL, UINTPTR_MAX, 0, 0, "Cancel", TAG_NOTHING);
    addCommandMessage(pcen, IDHELP,   UINTPTR_MAX, 0, 0, "Help",   TAG_NOTHING);

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

    if ( ! initCommandMessagesTable(c, pcen) )
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


#define DTPN_KEYWORDS                 "CloseUp, DateTimeChange, DropDown, FormatQuery, Format, KillFocus, SetFocus, UserString, or WmKeyDown"
#define MCN_KEYWORDS                  "GetDayState, Released, SelChange, Select, or ViewChange"

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

    if ( StrStrI(keyword,      "GETDAYSTATE") != NULL ) mcn = MCN_GETDAYSTATE;
    else if ( StrStrI(keyword, "RELEASED")    != NULL ) mcn = NM_RELEASEDCAPTURE;
    else if ( StrStrI(keyword, "SELCHANGE")   != NULL ) mcn = MCN_SELCHANGE;
    else if ( StrStrI(keyword, "SELECT")      != NULL ) mcn = MCN_SELECT;
    else if ( StrStrI(keyword, "VIEWCHANGE")  != NULL ) mcn = MCN_VIEWCHANGE;
    else
    {
        wrongArgValueException(c->threadContext, 2, MCN_KEYWORDS, keyword);
        return false;
    }
    *flag = mcn;
    return true;
}


/**
 * Convert a date time pickeer notification code to a method name.
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

    if ( StrStrI(keyword,      "CLOSEUP")        != NULL ) dtpn = DTN_CLOSEUP;
    else if ( StrStrI(keyword, "DATETIMECHANGE") != NULL ) dtpn = DTN_DATETIMECHANGE;
    else if ( StrStrI(keyword, "DROPDOWN")       != NULL ) dtpn = DTN_DROPDOWN;
    else if ( StrStrI(keyword, "FORMATQUERY")    != NULL ) dtpn = DTN_FORMATQUERY;
    else if ( StrStrI(keyword, "FORMAT")         != NULL ) dtpn = DTN_FORMAT;
    else if ( StrStrI(keyword, "KILLFOCUS")      != NULL ) dtpn = NM_KILLFOCUS;
    else if ( StrStrI(keyword, "SETFOCUS")       != NULL ) dtpn = NM_SETFOCUS;
    else if ( StrStrI(keyword, "USERSTRING")     != NULL ) dtpn = DTN_USERSTRING;
    else if ( StrStrI(keyword, "WMKEYDOWN")      != NULL ) dtpn = DTN_WMKEYDOWN;
    else
    {
        wrongArgValueException(c->threadContext, 2, DTPN_KEYWORDS, keyword);
        return false;
    }
    *flag = dtpn;
    return true;
}


/**
 * Creates a Rexx argument array for, presumably, the invocation of a Rexx
 * method connected to some type of key board event.
 *
 * @param c       Thread context we are operating in.
 * @param wParam  The WPARAM for the key event, which is presumed to be the
 *                charater code for the key event.
 *
 * @return RexxArrayObject
 *
 * @remarks  This function is currently called for connectKeyPress() and
 *           connectKeyEvent() processing and could probably be used for other
 *           key event connections if any are added to ooDialog.
 */
RexxArrayObject getKeyEventRexxArgs(RexxThreadContext *c, WPARAM wParam)
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

    RexxArrayObject args = c->ArrayOfFour(c->WholeNumber(wParam), c->Logical(bShift),
                                          c->Logical(bControl), c->Logical(bAlt));
    c->ArrayPut(args, c->String(info), 5);

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
        if ( DialogTable[i]->threadID == id )
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

    pCEventNotification pcen = DialogTable[i]->enCSelf;
    KEYPRESSDATA *pKeyPressData = (KEYPRESSDATA *)pcen->pHookData->pData;

    if ( (code == HC_ACTION) && pKeyPressData->key[wParam] )
    {
        if ( !(lParam & KEY_RELEASED) && !(lParam & KEY_WASDOWN) )
        {
            processKeyPress(pcen->pHookData, wParam, lParam);
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
        freeKeyPressData(pcen->pHookData);
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
    SUBCLASSDATA *pSubclassData = (SUBCLASSDATA *)LocalAlloc(LPTR, sizeof(SUBCLASSDATA));
    if ( pSubclassData == NULL )
    {
        return memoryErr;
    }

    KEYPRESSDATA *pKeyData = (KEYPRESSDATA *)LocalAlloc(LPTR, sizeof(KEYPRESSDATA));
    if ( pKeyData == NULL )
    {
        LocalFree(pSubclassData);
        return memoryErr;
    }

    keyPressErr_t result = setKeyPressData(pKeyData, method, keys, filter);
    if ( result == noErr )
    {
        pSubclassData->pData = pKeyData;
        pcen->pHookData = pSubclassData;

        // Note that setKBHook() frees all memeory if it fails;
        result = setKBHook(pcen);
    }
    else
    {
        freeKeyPressData(pSubclassData);
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
        return installKBHook(pcen, methodName, keys, filter);
    }

    return setKeyPressData((KEYPRESSDATA *)pcen->pHookData->pData, methodName, keys, filter);
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

    freeKeyPressData(pcen->pHookData);
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
 * The ooDialog event method gets 5 arguments:
 *   key:      decimal value of the key code.
 *   shift:    true / false, true if the shift key was depressed for this event.
 *   control:  true / false, true if control key was depressed.
 *   alt:      true / false, ditto.
 *   info:     Keyword string that specifies if right or left shift / control /
 *             alt were down and the state of the num lock, caps lock, and
 *             scroll lock keys.  The string contains some combination of:
 *
 *             rShift, lShift, rControl lControl, rAlt, lAlt, numOn, numOff,
 *             capsOn, capsOff, scrollOn, scrollOf
 *
 * @remarks  The method name (pMethod) can not be longer than 197 chars.  This
 *           is checked for in setKeyPressData()
 */
void processKeyPress(SUBCLASSDATA *pSubclassData, WPARAM wParam, LPARAM lParam)
{
    KEYPRESSDATA *pKeyData = (KEYPRESSDATA *)pSubclassData->pData;

    BOOL passed = TRUE;

    size_t i = pKeyData->key[wParam];
    char *pMethod = pKeyData->pMethods[i];
    KEYFILTER *pFilter = pKeyData->pFilters[i];

    BOOL bShift = (GetAsyncKeyState(VK_SHIFT) & ISDOWN) ? 1 : 0;
    BOOL bControl = (GetAsyncKeyState(VK_CONTROL) & ISDOWN) ? 1 : 0;
    BOOL bAlt = (GetAsyncKeyState(VK_MENU) & ISDOWN) ? 1 : 0;

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
        RexxThreadContext *c = pSubclassData->dlgProcContext;

        RexxArrayObject args = getKeyEventRexxArgs(c, wParam);
        invokeDispatch(c, pSubclassData->rexxDialog, c->String(pMethod), args);
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
 * @assumes Caller has passed in the correct pointer, i.e., that pSubclassData's
 *          pData points to a KEYPRESSDATA struct.
 */
void freeKeyPressData(SUBCLASSDATA *pSubclassData)
{
    size_t i;
    if ( pSubclassData )
    {
        KEYPRESSDATA *p = (KEYPRESSDATA *)pSubclassData->pData;
        for ( i = 1; i <= MAX_KEYPRESS_METHODS; i++ )
        {
            safeLocalFree((void *)p->pMethods[i]);
            safeLocalFree((void *)p->pFilters[i]);
        }
        LocalFree((void *)p);
        LocalFree((void *)pSubclassData);
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

    // Some combinations are not filters, so they are ignored.
    if ( ((! tmpFilter->and) && tmpFilter->shift && tmpFilter->control && tmpFilter->alt) ||
         (tmpFilter->and && ! tmpFilter->shift && ! tmpFilter->control && ! tmpFilter->alt) )
    {
        LocalFree(tmpFilter);
        return badFilterErr;
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
        KEYPRESSDATA *pKeyPressData = (KEYPRESSDATA *)pcen->pHookData->pData;

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

    BOOL exists = (seekKeyPressMethod((KEYPRESSDATA *)pcen->pHookData->pData, tmpName) > 0);
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

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcen->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return -1;
    }
    if ( *methodName == '\0' )
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_null, TheTwoObj);
        return 1;
    }
    return (addCommandMessage(pcen, id, 0x0000FFFF, 0, 0, methodName, 0) ? 0 : 1);
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
 */
RexxMethod4(RexxObjectPtr, en_connectUpDownEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    uint32_t id = oodResolveSymbolicID(context, pcen->rexxSelf, rxID, -1, 1);
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

    if ( addNotifyMessage(pcen, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, TAG_UPDOWN) )
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
 *                      WMKEYDOWN
 *                      KILLFOCUS
 *                      SETFOCUS
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted then the
 *                      method name is constructed by prefixing the event
 *                      keyword with 'on'.  For instance onUserString.
 *
 *  @note   If a symbolic ID is  used and it can not be resolved to a numeric
 *          number an exception is raised.
 *
 *  @remarks  This method is new since the 4.0.0 release, therefore an exception
 *            is raised for a bad resource ID rather than returning -1.
 */
RexxMethod4(RexxObjectPtr, en_connectDateTimePickerEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    uint32_t id = oodResolveSymbolicID(context, pcen->rexxSelf, rxID, -1, 1);
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

    uint32_t tag = TAG_DATETIMEPICKER | TAG_REPLYFROMREXX;

    if ( addNotifyMessage(pcen, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
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
 *
 *  @param  methodName  [OPTIONAL] The name of the method to be invoked in the
 *                      Rexx dialog.  If this argument is omitted then the
 *                      method name is constructed by prefixing the event
 *                      keyword with 'on'.  For instance onGetDayState.
 *
 *  @note   If a symbolic ID is  used and it can not be resolved to a numeric
 *          number an exception is raised.
 *
 *  @remarks  This method is new since the 4.0.0 release, therefore an exception
 *            is raised for a bad resource ID rather than returning -1.
 */
RexxMethod4(RexxObjectPtr, en_connectMonthCalendarEvent, RexxObjectPtr, rxID, CSTRING, event,
            OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;

    uint32_t id = oodResolveSymbolicID(context, pcen->rexxSelf, rxID, -1, 1);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto err_out;
    }

    uint32_t notificationCode;
    if ( ! keyword2mcn(context, event, &notificationCode) )
    {
        goto err_out;
    }
    if ( notificationCode == MCN_VIEWCHANGE && ! _isAtLeastVista() )
    {
        wrongWindowsVersionException(context, "connectMonthCalendarEvent", "Vista");
        goto err_out;
    }

    if ( argumentOmitted(3) || *methodName == '\0' )
    {
        methodName = mcn2name(notificationCode);
    }

    uint32_t tag = TAG_MONTHCALENDAR;
    if ( notificationCode == MCN_GETDAYSTATE )  // TODO rethink this if
    {
        tag |= (TAG_MSGHANDLED | TAG_REPLYFROMREXX);
    }

    if ( addNotifyMessage(pcen, id, 0xFFFFFFFF, notificationCode, 0xFFFFFFFF, methodName, tag) )
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
 *  @param  -tag         [optional]  A tag that allows a further differentiation
 *                       between messages.  This is an internal mechanism not to
 *                       be documented publicly.
 *
 *  @return  0 on success, 1 on failure.  One possible source of error is the
 *           message table being full.
 *
 *  @remarks  Although it would make more sense to return true on succes and
 *            false on failure, there is too much old code that relies on 0 for
 *            success and 1 for error.
 */
RexxMethod9(uint32_t, en_addUserMessage, CSTRING, methodName, CSTRING, wm, OPTIONAL_CSTRING, _wmFilter,
            OPTIONAL_RexxObjectPtr, wp, OPTIONAL_CSTRING, _wpFilter, OPTIONAL_RexxObjectPtr, lp, OPTIONAL_CSTRING, _lpFilter,
            OPTIONAL_CSTRING, _tag, CSELF, pCSelf)
{
    pCEventNotification pcen = (pCEventNotification)pCSelf;
    uint32_t result = 1;

    if ( *methodName == '\0' )
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_null, TheOneObj);
        goto done_out;
    }

    uint32_t winMessage;
    uint32_t wmFilter;
    if ( ! rxStr2Number32(context, wm, &winMessage, 2) )
    {
        goto done_out;
    }

    if ( argumentOmitted(3) )
    {
        wmFilter = 0xFFFFFFFF;
    }
    else
    {
        if ( ! rxStr2Number32(context, _wmFilter, &wmFilter, 3) )
        {
            goto done_out;
        }
    }

    uint64_t  filter;
    WPARAM    wParam;
    ULONG_PTR wpFilter;

    if ( ! oodGetWParam(context, wp, &wParam, 4, false) )
    {
        goto done_out;
    }
    if ( argumentOmitted(5) )
    {
        wpFilter = 0;
    }
    else
    {
        if ( ! rxStr2Number(context, _wpFilter, &filter, 5) )
        {
            goto done_out;
        }
        wpFilter = (filter == 0xFFFFFFFF ? (ULONG_PTR)SIZE_MAX : (ULONG_PTR)filter);
    }

    LPARAM    lParam;
    ULONG_PTR lpFilter;

    if ( ! oodGetLParam(context, lp, &lParam, 6, false) )
    {
        goto done_out;
    }
    if ( argumentOmitted(7) )
    {
        lpFilter = 0;
    }
    else
    {
        if ( ! rxStr2Number(context, _lpFilter, &filter, 7) )
        {
            goto done_out;
        }
        lpFilter = (filter == 0xFFFFFFFF ? (ULONG_PTR)SIZE_MAX : (ULONG_PTR)filter);
    }

    ULONG tag = 0;
    if ( argumentExists(8) )
    {
        if ( ! rxStr2Number(context, _tag, &filter, 8) )
        {
            goto done_out;
        }
        tag = (ULONG)filter;
    }

    if ( (winMessage | wParam | lParam) == 0 )
    {
        userDefinedMsgException(context->threadContext, "The wm, wp, and lp arguements can not all be 0" );
    }
    else
    {
        bool success;
        if ( (winMessage & wmFilter) == WM_COMMAND )
        {
            success = addCommandMessage(pcen, wParam, wpFilter, lParam, lpFilter, methodName, tag);
        }
        else if ( (winMessage & wmFilter) == WM_NOTIFY )
        {
            success = addNotifyMessage(pcen, wParam, wpFilter, lParam, lpFilter, methodName, tag);
        }
        else
        {
            success = addMiscMessage(pcen, winMessage, wmFilter, wParam, wpFilter, lParam, lpFilter, methodName, tag);
        }

        result = (success ? 0 : 1);
    }

done_out:
    return result;
}


