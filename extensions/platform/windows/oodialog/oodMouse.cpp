/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2011 Rexx Language Association. All rights reserved.    */
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
 * oodMouse.cpp
 *
 * This module contains functions and methods for classes related to the mouse
 * and cursors.
 *
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h
//#include "oodControl.hpp"

//#include <stdio.h>
//#include <dlgs.h>
#include <shlwapi.h>
#include <WindowsX.h>

#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodMessaging.hpp"
//#include "oodDeviceGraphics.hpp"


/**
 *  Methods for the .Mouse class.
 */
#define MOUSE_CLASS        "Mouse"

#define TRACK_MOUSE_KEYWORDS    "CANCEL, HOVER, LEAVE, NONCLIENT, or QUERY"
#define WM_MOUSE_KEYWORDS       "Move, Wheel, Leave, Hover, lButtonUp, lButtonDown, or CaptureChanged"
#define DLG_HAS_ENDED_MSG       "windows dialog has executed and been closed"


static HWND getMouseWindow(RexxMethodContext *c, pCMouse pcm)
{
    oodResetSysErrCode(c->threadContext);

    if ( pcm == NULL )
    {
        return (HWND)baseClassIntializationException(c);
    }

    // Unlikly, but it is possible the dialog is being destroyed at this
    // instant.
    EnterCriticalSection(&crit_sec);

    if ( pcm->dlgCSelf == NULL )
    {
        methodCanNotBeInvokedException(c, c->GetMessageName(), DLG_HAS_ENDED_MSG, pcm->rexxSelf);
        pcm->hWindow = NULL;
        goto done_out;
    }

    if ( pcm->hWindow == NULL )
    {
        if ( pcm->dlgCSelf->hDlg == NULL )
        {
            noWindowsDialogException(c, pcm->rexxSelf);
            goto done_out;
        }

        pcm->hWindow = pcm->dlgCSelf->hDlg;
    }

done_out:
    LeaveCriticalSection(&crit_sec);
    return pcm->hWindow;
}


static pCEventNotification getMousePCEN(RexxMethodContext *c, pCMouse pcm)
{
    oodResetSysErrCode(c->threadContext);

    pCEventNotification pcen = NULL;

    if ( pcm == NULL )
    {
        baseClassIntializationException(c);
        goto done_out;
    }

    // Unlikly, but it is possible the dialog is being destroyed at this
    // instant.
    EnterCriticalSection(&crit_sec);

    if ( pcm->dlgCSelf == NULL )
    {
        methodCanNotBeInvokedException(c, c->GetMessageName(), DLG_HAS_ENDED_MSG, pcm->rexxSelf);
        pcm->hWindow = NULL;
        goto done_out;
    }

    pcen = pcm->dlgCSelf->enCSelf;

done_out:
    LeaveCriticalSection(&crit_sec);
    return pcen;
}


/**
 * Convert a keyword to the (mouse) window message code.
 *
 * We know the keyword arg position is 1.  The mouse support is post ooRexx
 * 4.0.1 so we raise an exception on error.
 */
static bool keyword2wm(RexxMethodContext *c, CSTRING keyword, uint32_t *flag)
{
    uint32_t wmMsg;

    if ( StrCmpI(keyword,      "MOVE")           == 0 ) wmMsg = WM_MOUSEMOVE;
    else if ( StrCmpI(keyword, "WHEEL")          == 0 ) wmMsg = WM_MOUSEWHEEL;
    else if ( StrCmpI(keyword, "LEAVE")          == 0 ) wmMsg = WM_MOUSELEAVE;
    else if ( StrCmpI(keyword, "HOVER")          == 0 ) wmMsg = WM_MOUSEHOVER;
    else if ( StrCmpI(keyword, "LBUTTONDOWN")    == 0 ) wmMsg = WM_LBUTTONDOWN;
    else if ( StrCmpI(keyword, "LBUTTONUP")      == 0 ) wmMsg = WM_LBUTTONUP;
    else if ( StrCmpI(keyword, "CAPTURECHANGED") == 0 ) wmMsg = WM_CAPTURECHANGED;
    else
    {
        wrongArgValueException(c->threadContext, 1, WM_MOUSE_KEYWORDS, keyword);
        return false;
    }
    *flag = wmMsg;
    return true;
}


/**
 * Convert a window message code to a method name.
 */
inline CSTRING wm2name(uint32_t mcn)
{
    switch ( mcn )
    {
        case WM_MOUSEMOVE      : return "onMouseMove";
        case WM_MOUSEWHEEL     : return "onMouseWheel";
        case WM_MOUSELEAVE     : return "onMouseLeave";
        case WM_MOUSEHOVER     : return "onMouseHover";
        case WM_LBUTTONDOWN    : return "onLButtonDown";
        case WM_LBUTTONUP      : return "onLButtonUp";
        case WM_CAPTURECHANGED : return "onCaptureChanged";
    }
    return "onWM";
}


/**
 * Produces a rexx argument array for the standard mouse event handler
 * arugments.  Which are: keyState, mousePos, mouseObj.
 *
 * @author Administrator (12/14/2011)
 *
 * @param c
 * @param pcpbd
 * @param wParam
 * @param lParam
 * @param count   If count is greater than 3, the returned array will be this
 *                size.
 *
 * @return RexxArrayObject
 */
RexxArrayObject getMouseArgs(RexxThreadContext *c, pCPlainBaseDialog pcpbd, WPARAM wParam, LPARAM lParam, uint32_t count)
{
    char  buf[256] = {0};
    int   state    = GET_KEYSTATE_WPARAM(wParam);

    if ( state == 0 )
    {
        strcpy(buf, "None");
    }
    else
    {
        if ( state & MK_CONTROL  ) strcat(buf, "Control ");
        if ( state & MK_LBUTTON  ) strcat(buf, "lButton ");
        if ( state & MK_MBUTTON  ) strcat(buf, "mButton ");
        if ( state & MK_RBUTTON  ) strcat(buf, "rButton ");
        if ( state & MK_SHIFT    ) strcat(buf, "Shift ");
        if ( state & MK_XBUTTON1 ) strcat(buf, "xButton1 ");
        if ( state & MK_XBUTTON1 ) strcat(buf, "xButton2 ");

        buf[strlen(buf)] = '\0';
    }

    RexxStringObject rxState = c->String(buf);
    RexxObjectPtr    rxPoint = rxNewPoint(c, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

    // Send the window losing capture and the mouse object.  Note, I
    // don't think we can be here without a valid mouse object in pcpbd,
    // but we will check any way.
    RexxObjectPtr rxMouse = pcpbd->rexxMouse ? pcpbd->rexxMouse : TheNilObj;

    RexxArrayObject args;
    if ( count > 3 )
    {
        args = c->NewArray(count);
        c->ArrayPut(args, rxState, 1);
        c->ArrayPut(args, rxPoint, 2);
        c->ArrayPut(args, rxMouse, 3);
    }
    else
    {
        args = c->ArrayOfThree(rxState, rxPoint, rxMouse);
    }

    return args;
}

/**
 * Generic function to send a WM_MOUSEWHEEL notification to the Rexx dialog
 * object.
 *
 * It is used for Edit::ignoreMouseWheel() and also
 * Mouse::connectEvent('WHEEL').
 *
 *
 * @param mwd
 * @param wParam
 * @param lParam
 *
 * @return bool
 */
bool mouseWheelNotify(PMOUSEWHEELDATA mwd, WPARAM wParam, LPARAM lParam)
{
    RexxThreadContext *c = mwd->dlgProcContext;

    RexxArrayObject args = getMouseArgs(c, mwd->pcpbd, wParam, lParam, 4);

    RexxObjectPtr rxDelta = c->WholeNumber(GET_WHEEL_DELTA_WPARAM(wParam));
    c->ArrayPut(args, rxDelta, 4);

    if ( mwd->willReply )
    {
        return invokeDirect(c, mwd->pcpbd, mwd->method, args);
    }
    else
    {
        invokeDispatch(c, mwd->ownerDlg, c->String(mwd->method), args);
    }
    return true;
}


/** Mouse::new()
 *
 *
 */
RexxMethod3(RexxObjectPtr, mouse_new_cls, RexxObjectPtr, ownerWindow, OSELF, self, SUPER, superClass)
{
    RexxMethodContext *c = context;
    RexxObjectPtr mouse = TheNilObj;

    // We only support the owner window being a dialog at this time.  It seems
    // that dialog control windows could also be supported.
    pCPlainBaseDialog pcpbd = requiredDlgCSelf(context, ownerWindow, oodPlainBaseDialog, 1);
    if ( pcpbd == NULL )
    {
        goto done_out;
    }

    // If a mouse object is already instantiated for this dialog, we just return
    // that object, otherwise we instantiate a new mouse object.  All the book
    // keeping details are handled in Mouse::init()
    if ( pcpbd->rexxMouse != NULLOBJECT )
    {
        mouse = pcpbd->rexxMouse;
        goto done_out;
    }

    // We send the PlainBaseDialog CSelf as the argument to init()
    RexxArrayObject args = c->ArrayOfOne(c->NewPointer(pcpbd));

    // Forwarding this message to the super class will also invoke the init()
    // method of the mouse instance object.
    mouse = c->ForwardMessage(NULLOBJECT, NULL, superClass, args);
    if ( mouse == NULLOBJECT )
    {
        mouse = TheNilObj;
    }

done_out:
    return mouse;
}

/** Mouse::init()
 *
 *
 */
RexxMethod2(uint32_t, mouse_init, OPTIONAL_POINTER, args, OSELF, self)
{
    RexxMethodContext *c = context;

    if ( argumentOmitted(1) || args == NULL )
    {
        goto done_out;
    }

    // Right now, this has to be the CSelf of a dialog.  To support dialog
    // controls, we need to edit the Mouse::new() method and this method.
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)args;

    // Get a buffer for the Mouse CSelf.
    RexxBufferObject cselfBuffer = context->NewBuffer(sizeof(CMouse));
    if ( cselfBuffer == NULLOBJECT )
    {
        return 0;
    }
    context->SetObjectVariable("CSELF", cselfBuffer);

    pCMouse pcm = (pCMouse)context->BufferData(cselfBuffer);
    memset(pcm, 0, sizeof(CMouse));

    // Unlikly, but it is possible the Windows dialog has executed, ended, and
    // the CSelf structure is being / has been freed.
    EnterCriticalSection(&crit_sec);

    if ( ! pcpbd->dlgAllocated )
    {
        methodCanNotBeInvokedException(context, "new", DLG_HAS_ENDED_MSG, self);
        goto done_out;
    }

    // When dialog control windows are supported, we still need to set dlgCSelf.
    // When delDialog() is run, it checks for the mouse CSelf, and if it exists,
    // it sets it to NULL to signal that the underlying dialog no longer exists.
    pcm->hWindow     = pcpbd->hDlg;
    pcm->isDlgWindow = true;
    pcm->rexxSelf    = self;
    pcm->rexxWindow  = pcpbd->rexxSelf;
    pcm->dlgCSelf    = pcpbd;

    pcpbd->mouseCSelf = pcm;
    pcpbd->rexxMouse  = self;

    // Put this mouse object in the dialog's control bag to prevent it from
    // being garbage collected while the Rexx dialog object exists.
    context->SendMessage1(pcpbd->rexxSelf, "PUTCONTROL", self);

done_out:
    LeaveCriticalSection(&crit_sec);
    return 0;
}

/** Mouse::uninit()
 *
 *
 */
RexxMethod1(RexxObjectPtr, mouse_uninit, CSELF, pCSelf)
{
    if ( pCSelf != NULLOBJECT )
    {
        pCMouse pcm = (pCMouse)pCSelf;

#if 1
        printf("Mouse::uninit() Dlg CSelf=%p\n", pcm->dlgCSelf);
#endif

        EnterCriticalSection(&crit_sec);

        if ( pcm->dlgCSelf != NULL )
        {
            pcm->dlgCSelf->mouseCSelf = NULL;
            pcm->dlgCSelf->rexxMouse  = NULLOBJECT;
        }

        LeaveCriticalSection(&crit_sec);

    }

    return TheZeroObj;
}

/** Mouse::trackEvent()
 *
 *
 *  @notes  Requires the underlying dialog to exist.
 */
RexxMethod4(RexxObjectPtr, mouse_trackEvent, OPTIONAL_CSTRING, event, OPTIONAL_uint32_t, hoverTime,
            OPTIONAL_RexxObjectPtr, _answer, CSELF, pCSelf)
{
    TRACKMOUSEEVENT tme = {0};
    tme.cbSize = sizeof(TRACKMOUSEEVENT);

    HWND hwnd = getMouseWindow(context, (pCMouse)pCSelf);
    if ( hwnd == NULL )
    {
        goto error_out;
    }

    DWORD flags = 0;
    if ( argumentOmitted(1) )
    {
        flags = TME_LEAVE;
    }
    else
    {
        if ( StrStrI(event, "CANCEL"   ) != NULL ) flags =  TME_CANCEL;
        if ( StrStrI(event, "HOVER"    ) != NULL ) flags |= TME_HOVER;
        if ( StrStrI(event, "LEAVE"    ) != NULL ) flags |= TME_LEAVE;
        if ( StrStrI(event, "NONCLIENT") != NULL ) flags |= TME_NONCLIENT;
        if ( StrStrI(event, "QUERY"    ) != NULL ) flags |= TME_QUERY;
    }

    if ( flags & TME_QUERY )
    {
        if ( argumentOmitted(3) )
        {
            userDefinedMsgException(context->threadContext, 3, "is required when the QUERY keyword is used");
            goto error_out;
        }

        if ( ! requiredClass(context->threadContext, _answer, "Directory", 3) )
        {
            goto error_out;
        }
        RexxDirectoryObject answer = (RexxDirectoryObject)_answer;

        tme.dwFlags   = TME_QUERY;
        //tme.hwndTrack = (HWND)hwnd;

        if ( TrackMouseEvent(&tme) == 0 )
        {
            oodSetSysErrCode(context->threadContext);
            goto error_out;
        }

        char buf[512] = {'\0'};
        if ( tme.dwFlags & TME_CANCEL    ) strcat(buf, "CANCEL ");
        if ( tme.dwFlags & TME_HOVER     ) strcat(buf, "HOVER ");
        if ( tme.dwFlags & TME_LEAVE     ) strcat(buf, "LEAVE ");
        if ( tme.dwFlags & TME_NONCLIENT ) strcat(buf, "NONCLIENT ");
        if ( tme.dwFlags & TME_QUERY     ) strcat(buf, "QUERY ");

        if ( buf[strlen(buf)] == ' ' )
        {
            buf[strlen(buf)] = '\0';
        }

        context->DirectoryPut(answer, context->String(buf), "EVENT");
        context->DirectoryPut(answer, pointer2string(context, tme.hwndTrack), "HWND");
        context->DirectoryPut(answer, context->UnsignedInt32(tme.dwHoverTime), "HOVERTIME");

        goto good_out;
    }
    else
    {
        tme.dwFlags   = flags;
        tme.hwndTrack = (HWND)hwnd;

        if ( flags & TME_HOVER )
        {
            if ( argumentOmitted(2) )
            {
                tme.dwHoverTime = HOVER_DEFAULT;
            }
            else
            {
                tme.dwHoverTime = hoverTime;
            }
        }

        if ( TrackMouseEvent(&tme) == 0 )
        {
            oodSetSysErrCode(context->threadContext);
            goto error_out;
        }
    }

good_out:
    return TheTrueObj;

error_out:
    return TheFalseObj;
}


/** Mouse::dragDetect()
 *
 *
 *  @notes  Requires the underlying dialog to exist.
 */
RexxMethod2(RexxObjectPtr, mouse_dragDetect, RexxObjectPtr, _pt, CSELF, pCSelf)
{
    HWND hwnd = getMouseWindow(context, (pCMouse)pCSelf);
    if ( hwnd == NULL )
    {
        return TheFalseObj;
    }

    PPOINT pt = rxGetPoint(context, _pt, 1);
    if ( pt == NULL )
    {
        return TheFalseObj;
    }
    return DragDetect((HWND)hwnd, *pt) ? TheTrueObj : TheFalseObj;
}


/** Mouse::getCapture()
 *
 *  Retrieves a handle to the window (if any) that has captured the mouse.
 *
 *  Only one window at a time can capture the mouse; this window receives mouse
 *  input whether or not the cursor is within its borders.
 *
 *  @return  The handle of the window, in this thread, that had previously
 *           captured the mouse, or 0 if no window previosly had the capture.  A
 *           0 (NULL) return value means that no window in the current thread
 *           has captured the mouse. However, it is possible that another
 *           thread or process has captured the mouse.
 *
/** Mouse::releaseCapture()
 *
 *  Releases the mouse capture from a window in the current thread and restores
 *  normal mouse input processing.
 *
 *  @return  0 on success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode, but that only has meaning for
 *         releaseMouseCapture().
 *
 *  @remarks  GetCapture() and ReleaseCapture() need to run on the same thread
 *            as the dialog's message loop.  So we use SendMessage with one of
 *            the custom window messages.
 *
 */
RexxMethod2(RexxObjectPtr, mouse_get_release_capture, NAME, method, CSELF, pCSelf)
{
    RexxObjectPtr result = NULLOBJECT;

    HWND hDlg = getMouseWindow(context, (pCMouse)pCSelf);
    if ( hDlg != NULL )
    {
        if ( *method == 'G' )
        {
            HWND hwnd = (HWND)SendMessage(hDlg, WM_USER_GETSETCAPTURE, MF_GETCAPTURE, 0);
            result = pointer2string(context, hwnd);
        }
        else
        {
            uint32_t rc = (uint32_t)SendMessage(hDlg, WM_USER_GETSETCAPTURE, MF_RELEASECAPTURE,0);
            if ( rc == 0 )
            {
                result = TheZeroObj;
            }
            else
            {
                result = TheOneObj;
                oodSetSysErrCode(context->threadContext, rc);
            }
        }
    }
    return result;
}


/** Mouse::capture
 *
 *  Sets the mouse capture to the owner window of this mouse object.
 *
 *  capture() captures mouse input either when the mouse is over the window, or
 *  when the mouse button was pressed while the mouse was over the window and
 *  the button is still down. Only one window at a time can capture the mouse.
 *
 *  If the mouse cursor is over a window created by another thread, the system
 *  will direct mouse input to the specified window only if a mouse button is
 *  down.
 *
 *  @return  On success, the window handle of the window that previously had
 *           captured the mouse, or 0 if there was no such window. On error, -1.
 *
 *  @note  Sets the .SystemErrorCode,
 *
 *  @remarks  Right now we are only supporting dialog owner windows.  This code
 *            would need to be fixed up when / if we support dialog control
 *            windows.
 */
RexxMethod1(RexxObjectPtr, mouse_capture, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNegativeOneObj;

    HWND hDlg = getMouseWindow(context, (pCMouse)pCSelf);
    if ( hDlg != NULL )
    {
        HWND oldCapture = (HWND)SendMessage(hDlg, WM_USER_GETSETCAPTURE, MF_SETCAPTURE, (LPARAM)hDlg);
        result = pointer2string(context, oldCapture);
    }
    return result;
}


/** Mouse::connectEvent()
 *
 *
 * @return  True on success, false on error.
 */
RexxMethod4(RexxObjectPtr, mouse_connectEvent, CSTRING, event, OPTIONAL_CSTRING, methodName,
            OPTIONAL_logical_t, _willReply, CSELF, pCSelf)
{
    pCEventNotification pcen = getMousePCEN(context, (pCMouse)pCSelf);
    if ( pcen == NULL )
    {
        goto err_out;
    }

    uint32_t wmMsg;
    if ( ! keyword2wm(context, event, &wmMsg) )
    {
        goto err_out;
    }

    if ( argumentOmitted(2) || *methodName == '\0' )
    {
        methodName = wm2name(wmMsg);
    }

    uint32_t tag = TAG_MOUSE;
    bool willReply = argumentOmitted(3) || _willReply;

    tag |= willReply ? TAG_REPLYFROMREXX : 0;

    if ( addMiscMessage(pcen, context, wmMsg, 0xFFFFFFFF, 0, 0, 0, 0, methodName, tag) )
    {
        return TheTrueObj;
    }

err_out:
    return TheFalseObj;
}
