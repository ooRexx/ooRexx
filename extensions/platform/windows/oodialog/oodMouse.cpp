/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2011-2019 Rexx Language Association. All rights reserved.    */
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

#include <shlwapi.h>
#include <WindowsX.h>

#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodShared.hpp"
#include "oodMessaging.hpp"
#include "oodResources.hpp"
#include "oodMouse.hpp"


/**
 *  Methods for the .Mouse class.
 */
#define MOUSE_CLASS        "Mouse"

/**
 * Returns the mouse CSelf, raising an exception if it is null.
 *
 * @param c
 * @param p
 *
 * @return pCMouse
 */
static pCMouse getMouseCSelf(RexxMethodContext *c, void *p)
{
    oodResetSysErrCode(c->threadContext);

    pCMouse pcm = (pCMouse)p;
    if ( pcm == NULL )
    {
        baseClassInitializationException(c);
    }
    return pcm;
}

/**
 * Checks that the underlying dialog is valid and updates the dialog window
 * handle and thread ID in the mouse CSelf struct if needed.
 *
 * @param c    Method context we are operating in.
 * @param pcm  Assumed pointer to mouse CSelf struct.
 *
 * @return The pCMouse pointer, or null on any error.
 *
 * @remarks  The mouse object can be instantiated before the underlying windows
 *           dialog exists.  In which case the hWindow, hDlg, and
 *           dlgProcThreadID members of the mouse CSelf struct will all be null.
 *           This function ensures that these members are updated if needed.
 *
 *           In addition, it is possible that the dialog has been closed and the
 *           Rexx programmer is attempting to use the mouse object after that.
 *           So, we also ensure that this is not happening.
 *
 *           Note that hWindow, hDlg, and dlgProcThreadID will never be null for
 *           a dialog control, because a Rexx dialog control can not be
 *           instantiated until the underlying dialog exists, so these members
 *           are set correctly during Mouse::init().
 */
static pCMouse requiredDlg(RexxMethodContext *c, void *p)
{
    pCMouse pcm = getMouseCSelf(c, p);
    if ( pcm == NULL )
    {
        return NULL;
    }

    // Unlikly, but it is possible the dialog is being destroyed at this
    // instant.
    EnterCriticalSection(&crit_sec);

    if ( ! pcm->dlgCSelf->dlgAllocated )
    {
        methodCanNotBeInvokedException(c, c->GetMessageName(), DLG_HAS_ENDED_MSG, pcm->rexxSelf);
        pcm = NULL;
        goto done_out;
    }

    if ( pcm->hWindow == NULL )
    {
        if ( pcm->dlgCSelf->hDlg == NULL )
        {
            noWindowsDialogException(c, pcm->rexxSelf);
            pcm = NULL;
            goto done_out;
        }

        pcm->dlgProcThreadID = pcm->dlgCSelf->dlgProcThreadID;
        pcm->hWindow         = pcm->dlgCSelf->hDlg;

        pcm->hDlg = pcm->hWindow;
    }

done_out:
    LeaveCriticalSection(&crit_sec);
    return pcm;
}


static pCEventNotification getMousePCEN(RexxMethodContext *c, pCMouse pcm)
{
    oodResetSysErrCode(c->threadContext);

    pCEventNotification pcen = NULL;

    if ( pcm == NULL )
    {
        baseClassInitializationException(c);
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

    if ( StrCmpI(keyword,      "MOUSEMOVE")      == 0 ) wmMsg = WM_MOUSEMOVE;
    else if ( StrCmpI(keyword, "MOUSEWHEEL")     == 0 ) wmMsg = WM_MOUSEWHEEL;
    else if ( StrCmpI(keyword, "MOUSELEAVE")     == 0 ) wmMsg = WM_MOUSELEAVE;
    else if ( StrCmpI(keyword, "MOUSEHOVER")     == 0 ) wmMsg = WM_MOUSEHOVER;
    else if ( StrCmpI(keyword, "NCMOUSELEAVE")   == 0 ) wmMsg = WM_NCMOUSELEAVE;
    else if ( StrCmpI(keyword, "NCMOUSEHOVER")   == 0 ) wmMsg = WM_NCMOUSEHOVER;
    else if ( StrCmpI(keyword, "LBUTTONDOWN")    == 0 ) wmMsg = WM_LBUTTONDOWN;
    else if ( StrCmpI(keyword, "LBUTTONUP")      == 0 ) wmMsg = WM_LBUTTONUP;
    else if ( StrCmpI(keyword, "LBUTTONDBLCLK")  == 0 ) wmMsg = WM_LBUTTONDBLCLK;
    else if ( StrCmpI(keyword, "MBUTTONDOWN")    == 0 ) wmMsg = WM_MBUTTONDOWN;
    else if ( StrCmpI(keyword, "MBUTTONUP")      == 0 ) wmMsg = WM_MBUTTONUP;
    else if ( StrCmpI(keyword, "MBUTTONDBLCLK")  == 0 ) wmMsg = WM_MBUTTONDBLCLK;
    else if ( StrCmpI(keyword, "RBUTTONDOWN")    == 0 ) wmMsg = WM_RBUTTONDOWN;
    else if ( StrCmpI(keyword, "RBUTTONUP")      == 0 ) wmMsg = WM_RBUTTONUP;
    else if ( StrCmpI(keyword, "RBUTTONDBLCLK")  == 0 ) wmMsg = WM_RBUTTONDBLCLK;
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
 * Convert a keyword to one of the system cursor values.  IDC_ARROW, etc..
 *
 * We know the keyword arg position is 1.  The mouse support is post ooRexx
 * 4.0.1 so we raise an exception on error.
 */
static CSTRING keyword2cursor(RexxMethodContext *c, CSTRING keyword)
{
    CSTRING cursor = NULL;

    if ( StrCmpI(keyword,      "APPSTARTING") == 0 ) cursor = IDC_APPSTARTING;
    else if ( StrCmpI(keyword, "ARROW")       == 0 ) cursor = IDC_ARROW;
    else if ( StrCmpI(keyword, "CROSS")       == 0 ) cursor = IDC_CROSS;
    else if ( StrCmpI(keyword, "HAND")        == 0 ) cursor = IDC_HAND;
    else if ( StrCmpI(keyword, "HELP")        == 0 ) cursor = IDC_HELP;
    else if ( StrCmpI(keyword, "IBEAM")       == 0 ) cursor = IDC_IBEAM;
    else if ( StrCmpI(keyword, "NO")          == 0 ) cursor = IDC_NO;
    else if ( StrCmpI(keyword, "SIZEALL")     == 0 ) cursor = IDC_SIZEALL;
    else if ( StrCmpI(keyword, "SIZENESW")    == 0 ) cursor = IDC_SIZENESW;
    else if ( StrCmpI(keyword, "SIZENS")      == 0 ) cursor = IDC_SIZENS;
    else if ( StrCmpI(keyword, "SIZENWSE")    == 0 ) cursor = IDC_SIZENWSE;
    else if ( StrCmpI(keyword, "SIZEWE")      == 0 ) cursor = IDC_SIZEWE;
    else if ( StrCmpI(keyword, "UPARROW")     == 0 ) cursor = IDC_UPARROW;
    else if ( StrCmpI(keyword, "WAIT")        == 0 ) cursor = IDC_WAIT;
    else
    {
        wrongArgValueException(c->threadContext, 1, SYSTEM_CURSOR_KEYWORDS, keyword);
    }

    return cursor;
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
        case WM_NCMOUSELEAVE   : return "onNcMouseLeave";
        case WM_NCMOUSEHOVER   : return "onNcMouseHover";
        case WM_LBUTTONDOWN    : return "onLButtonDown";
        case WM_LBUTTONUP      : return "onLButtonUp";
        case WM_LBUTTONDBLCLK  : return "onLButtonDblClk";
        case WM_MBUTTONDOWN    : return "onMButtonDown";
        case WM_MBUTTONUP      : return "onMButtonUp";
        case WM_MBUTTONDBLCLK  : return "onMButtonDblClk";
        case WM_RBUTTONDOWN    : return "onRButtonDown";
        case WM_RBUTTONUP      : return "onRButtonUp";
        case WM_RBUTTONDBLCLK  : return "onRButtonDblClk";
        case WM_CAPTURECHANGED : return "onCaptureChanged";
    }
    return "onWM";
}

/**
 * Produces a rexx argument array for the standard mouse event handler
 * arugments.  Which are: keyState, mousePos, mouseObj.
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
static RexxArrayObject getMouseArgs(RexxThreadContext *c, RexxObjectPtr rxMouse, WPARAM wParam, LPARAM lParam, uint32_t count)
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

        buf[strlen(buf) - 1] = '\0';
    }

    RexxStringObject rxState = c->String(buf);
    RexxObjectPtr    rxPoint = rxNewPoint(c, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

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
 * Invokes the Rexx dialog's event handling method for a mouse related window
 * message.
 *
 * The method invocation is done directly by sending a message to the method.
 *
 * @param c       Thread context we are operating in.
 * @param pcpbd   The Rexx dialog's CSelf whose method will be invoked.
 * @param method  The name of the method being invoked
 * @param args    The argument array for the method being invoked
 *
 * @return The replied Rexx object on success and null object on some error.
 *
 * @remarks  For mouse related messages, at this time, the Rexx method has to be
 *           either true or false, so we verify that here.
 */
static RexxObjectPtr mouseInvokeDirect(RexxThreadContext *c, pCPlainBaseDialog pcpbd, CSTRING methodName, RexxArrayObject args)
{
    RexxObjectPtr rexxReply = c->SendMessage(pcpbd->rexxSelf, methodName, args);
    if ( ! msgReplyIsGood(c, pcpbd, rexxReply, methodName, false) )
    {
        rexxReply = NULLOBJECT;
    }
    else
    {
        RexxObjectPtr converted = convertToTrueOrFalse(c, rexxReply);
        if ( converted == NULLOBJECT )
        {
            wrongReplyNotBooleanException(c, methodName, rexxReply);
            checkForCondition(c, false);
            endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);
            rexxReply = NULLOBJECT;
        }
        else
        {
            rexxReply = converted;
        }
    }

    return rexxReply;
}

/**
 * Generic function to send a WM_MOUSEWHEEL notification to the Rexx dialog
 * object.
 *
 * It is used for Mouse::connectEvent('WHEEL').
 *
 *
 * @param mwd
 * @param wParam
 * @param lParam
 *
 * @return LRESULT
 *
 * @remarks  When we use mouseInvokeDirect() and null object is returned, an
 *           exception has been raised and the dialog should have been ended. So
 *           we just return 0, we don't care what the user asked for.
 *
 *           From the docs, I expected that returning non-zero in a dialog
 *           control subclass procedure would have the effect of passing the
 *           message on to the DefWindowProc, but it does not.  We either need
 *           to directly invoke DefWindowProc(), or do a SendMessage() directly
 *           to the dialog.  Both seem to work the same, the mouse wheel event
 *           reaches the dialog procedure of the parent dialog.  So, we do the
 *           same thing if the tag is either send to DefWindowProc or SendToDlg.
 *
 */
static LRESULT mouseWheelNotify(PMOUSEWHEELDATA mwd, WPARAM wParam, LPARAM lParam)
{
    LRESULT reply = 0;
    RexxThreadContext *c = mwd->pcpbd->dlgProcContext;

    RexxArrayObject args = getMouseArgs(c, mwd->mouse, wParam, lParam, 4);

    RexxObjectPtr rxDelta = c->WholeNumber(GET_WHEEL_DELTA_WPARAM(wParam));
    c->ArrayPut(args, rxDelta, 4);

    uint32_t tag = mwd->tag;
    if ( mwd->isControlMouse )
    {
        if ( mwd->willReply )
        {
            RexxObjectPtr result = mouseInvokeDirect(c, mwd->pcpbd, mwd->method, args);
            if ( result == NULLOBJECT || result == TheFalseObj )
            {
                reply = DefSubclassProc(mwd->hwnd, WM_MOUSEWHEEL, wParam, lParam);
            }
        }
        else
        {
            invokeDispatch(c, mwd->pcpbd, mwd->method, args);
            if ( tag & CTRLTAG_SENDTOCONTROL )
            {
                reply = DefSubclassProc(mwd->hwnd, WM_MOUSEWHEEL, wParam, lParam);
            }
        }
    }
    else
    {
        if ( mwd->willReply )
        {
            RexxObjectPtr result = mouseInvokeDirect(c, mwd->pcpbd, mwd->method, args);
            if ( result == TheTrueObj )
            {
                reply = (LRESULT)TheTrueObj;
            }
            else
            {
                reply = (LRESULT)TheFalseObj;
            }
        }
        else
        {
            invokeDispatch(c, mwd->pcpbd, mwd->method, args);
            if ( tag & TAG_REPLYFALSE )
            {
                reply = (LRESULT)TheFalseObj;
            }
            else
            {
                reply = (LRESULT)TheTrueObj;
            }
        }
    }

    return reply;
}


/**
 * Sets a new cursor and returns the old one as a .Image object.
 *
 * @param c
 * @param pcm
 * @param hCursor
 *
 * @return RexxObjectPtr
 */
static RexxObjectPtr mouseSetCursor(RexxMethodContext *c, pCMouse pcm, HCURSOR hCursor)
{
    RexxObjectPtr result    = TheZeroObj;
    HCURSOR       oldCursor = NULL;

    // TODO need to investigate this, which is the way it was done in all pre
    // 4.2.0 ooDialogs, and worked.  But, MSDN says to set GCLP_HCURSOR to NULL
    // and that SetCursor() returns the previous cursor, if there was one.
    oldCursor = (HCURSOR)setClassPtr(pcm->hWindow, GCLP_HCURSOR, (LONG_PTR)hCursor);
    SetCursor(hCursor);

    if ( oldCursor == NULL )
    {
        goto done_out;
    }

    SIZE s;
    s.cx = GetSystemMetrics(SM_CXCURSOR);
    s.cy = GetSystemMetrics(SM_CYCURSOR);

    // Note that we use true for the last arge, even though we are creating this
    // from a handle, because we are pretty sure of the size and the flags.
    result = rxNewValidImage(c, oldCursor, IMAGE_CURSOR, &s, LR_DEFAULTSIZE | LR_SHARED, true);

done_out:
    return result;
}


static LRESULT invokeControlMethod(RexxThreadContext *c, pCPlainBaseDialog pcpbd, char *methodName,
                                   RexxArrayObject args, uint32_t tag, bool willReply, uint32_t msg, HWND hwnd,
                                   WPARAM wParam, LPARAM lParam)
{
    if ( willReply )
    {
        RexxObjectPtr result = mouseInvokeDirect(c, pcpbd, methodName, args);
        if ( result == TheTrueObj )
        {
            return 0;
        }
    }
    else
    {
        invokeDispatch(c, pcpbd, methodName, args);
        if ( ! (tag & CTRLTAG_SENDTOCONTROL) )
        {
            return 0;
        }
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

/**
 * Invokes a method in the Rexx dialog for a mouse event connected to a dialog
 * window.
 *
 * @param c               Thread context we are operating in.
 * @param pcpbd           Plain base dialog CSelf.
 * @param methodName      Name of the method to invoke.
 * @param args            Argument array to send to the method
 * @param tag             Event tag, dialog tag as opposed to control tag.
 *
 * @return The MsgReplyType enum appropriate.  This is returned in the dialog
 *         window procedure and is either reply true or reply false.
 *
 * @remarks  For the direct invocation, the user can either reply .true or
 *           .false.  .true indicates the event message was processed and .false
 *           indicates the event message was not handled.
 */
static MsgReplyType invokeDialogMethod(RexxThreadContext *c, pCPlainBaseDialog pcpbd, char *methodName,
                                       RexxArrayObject args, uint32_t tag)
{
    MsgReplyType ret = ReplyTrue;
    bool         willReply = (tag & TAG_EXTRAMASK) == TAG_REPLYFROMREXX;

    if ( willReply )
    {
        RexxObjectPtr result = mouseInvokeDirect(c, pcpbd, methodName, args);
        if ( result == TheFalseObj || result == NULLOBJECT )
        {
            ret = ReplyFalse;
        }
    }
    else
    {
        invokeDispatch(c, pcpbd, methodName, args);
        if ( tag & TAG_REPLYFALSE )
        {
            ret = ReplyFalse;
        }
    }
    return ret;
}


/**
 * Process window messages relating to mouse messages sent to a dialog control
 * subclass window procedure. All messages diverted here have been tagged by the
 * ooDialog framework with the mouse tag.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param msg
 * @param hwnd
 * @param wParam
 * @param lParam
 * @param pcdc
 *
 * @return LRESULT
 */
LRESULT processMouseMsg(RexxThreadContext *c, char *methodName, uint32_t tag, uint32_t msg, HWND hwnd,
                        WPARAM wParam, LPARAM lParam, pCDialogControl pcdc)
{
    pCPlainBaseDialog pcpbd = pcdc->pcpbd;

    if ( tag & CTRLTAG_SENDTODLG )
    {
        return SendMessage(pcpbd->hDlg, msg, wParam, lParam);
    }

    LRESULT ret       = 0;
    bool    willReply = (tag & CTRLTAG_EXTRAMASK) == CTRLTAG_REPLYFROMREXX;

    switch ( msg )
    {
        case WM_CAPTURECHANGED :
        {
            // Send the window losing capture handle and the mouse object.
            RexxArrayObject args = c->ArrayOfTwo(pointer2string(c, (void *)lParam), pcdc->rexxMouse);

            return invokeControlMethod(c, pcpbd, methodName, args, tag, willReply, msg, hwnd, wParam, lParam);
        }
        break;

        case WM_LBUTTONDOWN :
        case WM_LBUTTONUP :
        case WM_LBUTTONDBLCLK :
        case WM_MBUTTONDOWN :
        case WM_MBUTTONUP :
        case WM_MBUTTONDBLCLK :
        case WM_MOUSEHOVER :
        case WM_MOUSEMOVE :
        case WM_RBUTTONDOWN :
        case WM_RBUTTONUP :
        case WM_RBUTTONDBLCLK :
        {
            RexxArrayObject args = getMouseArgs(c, pcdc->rexxMouse, wParam, lParam, 3);

            return invokeControlMethod(c, pcpbd, methodName, args, tag, willReply, msg, hwnd, wParam, lParam);
        }
        break;

        case WM_MOUSELEAVE :
        case WM_NCMOUSELEAVE:
        {
            // Send the mouse object, and not even sure we need to do that..
            RexxArrayObject args = c->ArrayOfOne(pcdc->rexxMouse);

            return invokeControlMethod(c, pcpbd, methodName, args, tag, willReply, msg, hwnd, wParam, lParam);
        }
        break;

        case WM_NCMOUSEHOVER :
        {
            RexxObjectPtr rxPoint = rxNewPoint(c, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            CSTRING flags         = ncHitTest2string(wParam);

            RexxArrayObject args = c->ArrayOfThree(c->String(flags), rxPoint, pcdc->rexxMouse);

            return invokeControlMethod(c, pcpbd, methodName, args, tag, willReply, msg, hwnd, wParam, lParam);
        }
        break;

        case WM_MOUSEWHEEL :
        {
            MOUSEWHEELDATA mwd;
            mwd.method         = methodName;
            mwd.mouse          = pcdc->rexxMouse;
            mwd.pcpbd          = pcpbd;
            mwd.hwnd           = hwnd;
            mwd.tag            = tag;
            mwd.isControlMouse = true;
            mwd.willReply      = willReply;

            return mouseWheelNotify(&mwd, wParam, lParam);
        }
        break;

        default :
            break;
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}


/**
 * Process window messages relating to mouse messages sent to the dialog window.
 * All messages diverted here have been tagged by the ooDialog framework with
 * the mouse tag.
 *
 * @param c
 * @param methodName
 * @param tag
 * @param msg
 * @param wParam
 * @param lParam
 * @param pcpbd
 *
 * @return MsgReplyType
 *
 * @remarks  For the mouse object, I don't think we can be here without a valid
 *           mouse object in pcpbd, but we will check any way.
 */
MsgReplyType processMouseMsg(RexxThreadContext *c, char *methodName, uint32_t tag, uint32_t msg,
                             WPARAM wParam, LPARAM lParam, pCPlainBaseDialog pcpbd)
{
    switch ( msg )
    {
        case WM_CAPTURECHANGED :
        {
            // Send the window losing capture handle and the mouse object.
            RexxArrayObject args = c->ArrayOfTwo(pointer2string(c, (void *)lParam), pcpbd->rexxMouse);

            return invokeDialogMethod(c, pcpbd, methodName, args, tag);
        }
        break;


        case WM_LBUTTONDOWN :
        case WM_LBUTTONUP :
        case WM_LBUTTONDBLCLK :
        case WM_MBUTTONDOWN :
        case WM_MBUTTONUP :
        case WM_MBUTTONDBLCLK :
        case WM_MOUSEHOVER :
        case WM_MOUSEMOVE :
        case WM_RBUTTONDOWN :
        case WM_RBUTTONUP :
        case WM_RBUTTONDBLCLK :
        {
            RexxArrayObject args = getMouseArgs(c, pcpbd->rexxMouse, wParam, lParam, 3);

            return invokeDialogMethod(c, pcpbd, methodName, args, tag);
        }
        break;

        case WM_MOUSELEAVE :
        case WM_NCMOUSELEAVE:
        {
            // Send the mouse object, and not even sure we need to do that..
            RexxArrayObject args = c->ArrayOfOne(pcpbd->rexxMouse);

            return invokeDialogMethod(c, pcpbd, methodName, args, tag);
        }
        break;

        case WM_NCMOUSEHOVER :
        {
            RexxObjectPtr rxPoint = rxNewPoint(c, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            CSTRING hit           = ncHitTest2string(wParam);

            RexxArrayObject args = c->ArrayOfThree(c->String(hit), rxPoint, pcpbd->rexxMouse);

            return invokeDialogMethod(c, pcpbd, methodName, args, tag);
        }
        break;

        case WM_MOUSEWHEEL :
        {
            MOUSEWHEELDATA mwd;
            mwd.method         = methodName;
            mwd.mouse          = pcpbd->rexxMouse;
            mwd.pcpbd          = pcpbd;
            mwd.hwnd           = pcpbd->hDlg;  // Not really needed for a dialog mouse.
            mwd.tag            = tag;          // Not really needed for a dialog mouse.
            mwd.isControlMouse = false;
            mwd.willReply      = (tag & TAG_EXTRAMASK) == TAG_REPLYFROMREXX;

            return (MsgReplyType)mouseWheelNotify(&mwd, wParam, lParam);
        }
        break;

        default :
            break;
    }
    return ReplyFalse;
}


/** Mouse::new()                  [Class method]
 *
 *
 */
RexxMethod3(RexxObjectPtr, mouse_new_cls, RexxObjectPtr, ownerWindow, OSELF, self, SUPER, superClass)
{
    RexxMethodContext *c = context;
    RexxObjectPtr mouse = TheNilObj;

    NEWMOUSEPARAMS  nmp  = {0};
    pCDialogControl pcdc = NULL;

    // The owner window has to be a dialog or a dialog control
    pCPlainBaseDialog pcpbd = requiredDlgCSelf(context, ownerWindow, oodUnknown, 1, &pcdc);
    if ( pcpbd == NULL )
    {
        goto done_out;
    }

    bool isDialog = (pcdc != NULL ? false : true);

    // If a mouse object is already instantiated for this owner window, we just
    // return that object, otherwise we instantiate a new mouse object.  All the
    // book keeping details are handled in Mouse::init()
    if ( isDialog )
    {
        if ( pcpbd->rexxMouse != NULLOBJECT )
        {
            mouse = pcpbd->rexxMouse;
            goto done_out;
        }
    }
    else
    {
        if ( pcdc->rexxMouse != NULLOBJECT )
        {
            mouse = pcdc->rexxMouse;
            goto done_out;
        }
    }

    nmp.controlCSelf = pcdc;
    nmp.dlgCSelf     = pcpbd;
    nmp.isDlgWindow  = isDialog;

    // We send a pointer to the new mouse params struct as the argument to init()
    RexxArrayObject args = c->ArrayOfOne(c->NewPointer(&nmp));

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

/** Mouse::doubleClickTime()   [Class method]
 *
 * Gets the current double-click time for the mouse. A double-click is a
 * series of two clicks of the mouse button, the second occurring within a
 * specified time after the first. The double-click time is the maximum number
 * of milliseconds that may occur between the first and second click of a
 * double-click.
 *
 * @return  The current double-click time in milliseconds.
 *
 * @remarks  We do not even need the CSelf struct for this method, so we do not
 *           use.
 */
RexxMethod0(uint32_t, mouse_doubleClickTime_cls)
{
    return GetDoubleClickTime();
}

/** Mouse::setDoubleClickTime()   [Class method]
 *
 * Sets the double-click time for the mouse.
 *
 * A double-click is a series of two clicks of the mouse button, the second
 * occurring within a specified time after the first. The double-click time is
 * the maximum number of milliseconds that may occur between the first and
 * second click of a double-click.
 *
 * @param  interval  The time in milliseconds to set the double-click time to.
 *                   If this is 0, the default system double-click time is
 *                   restored, which is 500 milliseconds.
 *
 * @return  True on success, false on error.
 *
 * @notes  Sets the .SystemErrorCode value. This method sets the double-click
 *         time for all windows in the system.
 *
 * @remarks  We do not even need the CSelf struct for this method, so we do not
 *           use.
 *
 *           Note that if interval is omitted, the value of interval will be 0.
 *           Since this is the default for interval, we do not need to check if
 *           the argument was omitted or not.
 */
RexxMethod1(RexxObjectPtr, mouse_setDoubleClickTime_cls, OPTIONAL_uint32_t, interval)
{
    oodResetSysErrCode(context->threadContext);

    if ( SetDoubleClickTime(interval) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheFalseObj;
    }
    return TheTrueObj;
}


/** Mouse::swapButton()   [Class method]
 *
 * Reverses or restores the meaning of the left and right mouse buttons.
 *
 * @param  swap  [Optional]  Specifies whether the meaning of the mouse buttons
 *               are reversed or restored.
 *
 *               If .true, the left button generates right-button messages
 *               and the right button generates left-button messages. If .false,
 *               the buttons are restored to their original meanings.  The
 *               default is .true
 *
 * @return  If the buttons were swapped before this method is invoked, the
 *          return is .true, if the buttons were not swapped, the return is
 *          .false.
 *
 * @notes   The .SystemErrorCode is set back to 0 by this method, but there is
 *          no situation that would cause it to be changed to an error code.
 *
 *          Button swapping is provided by Windows as a convenience to people
 *          who use the mouse with their left hands. The button swapping
 *          functionality is usually done through the Control Panel only.
 *          Although any application is free to use this functionality, the
 *          mouse is a shared resource and reversing the meaning of its buttons
 *          affects the entire system.
 *
 * @remarks  We do not even need the CSelf struct for this method, so we do not
 *           use.
 */
RexxMethod1(logical_t, mouse_swapButton_cls, OPTIONAL_logical_t, swap)
{
    oodResetSysErrCode(context->threadContext);

    if ( argumentOmitted(1) )
    {
        swap = TRUE;
    }
    return SwapMouseButton((BOOL)swap);
}


/** Mouse::loadCursor()  [Class method]
 *
 *  Returns one of the shared pre-defined system cursors.
 *
 *  @param  cursorName  [Required] Identifies the system cursor to be returned.
 *
 *                      Acceptable keywords are: APPSTARTING, ARROW, CROSS,
 *                      HAND, HELP, IBEAM, NO, SIZEALL, SIZENESW, SIZENS,
 *                      SIZENWSE, SIZEWE, UPARROW, or WAIT
 *
 *  @return  The requested cursor as a .Image object on success, or 0 on error.
 *
 *  @notes  Sets the .SystemErrorCode variable.  Raises syntax conditions when
 *          incorrect usage is detected.  The returned .Image object is a shared
 *          image and should not be released.  The ooDialog framework will
 *          ignore any requests to release the object.
 *
 *  @remarks  According to MSDN docs, the OCR_* constants can be used in the
 *            LoadCusorFromFile() API.  But doing so always causes a crash.
 *            Some google searches turn up info suggesting that the docs are
 *            incorrect.
 */
RexxMethod2(RexxObjectPtr, mouse_loadCursor_cls, CSTRING, name, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr result  = TheZeroObj;

    CSTRING cursor = keyword2cursor(context, name);
    if ( cursor == NULL )
    {
        goto done_out;
    }

    HCURSOR hCursor = (HCURSOR)LoadImage(NULL, cursor, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);

    if ( hCursor == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        goto done_out;
    }

    SIZE s;
    s.cx = GetSystemMetrics(SM_CXCURSOR);
    s.cy = GetSystemMetrics(SM_CYCURSOR);

    // Note that we use true for the last arg, even though we are creating this
    // from a handle, because we are pretty sure of the size and the flags.
    result = rxNewValidImage(context, hCursor, IMAGE_CURSOR, &s, LR_DEFAULTSIZE | LR_SHARED, true);

done_out:
    return result;
}


/** Mouse::loadCursorFromfiles()  [Class method]
 *
 *  Creates a cursor based on data from a file.
 *
 *  @param  fileName  [Required] Identifies the file that the cursor is created
 *                    from.  This argument can either be a fully qualified or a
 *                    relative file name.
 *
 *  @return  The requested cursor as a .Image object on success, or 0 on error.
 *
 *  @notes  Sets the .SystemErrorCode variable.  Raises syntax conditions when
 *          incorrect usage is detected.  The returned .Image object is not a
 *          shared image and can be released when no longer needed, if desired
 *          to free up some (small) amount of system resources..
 *
 *  @remarks  According to MSDN docs, the OCR_* constants can be used in the
 *            LoadCusorFromFile() API.  But doing so always causes a crash.
 *            Some google searches turn up info suggesting that the docs are
 *            incorrect.
 *
 */
RexxMethod2(RexxObjectPtr, mouse_loadCursorFromFile_cls, CSTRING, fName, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr result  = TheZeroObj;

    HCURSOR hCursor = LoadCursorFromFile(fName);
    if ( hCursor == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        goto done_out;
    }

    SIZE s;
    s.cx = GetSystemMetrics(SM_CXCURSOR);
    s.cy = GetSystemMetrics(SM_CYCURSOR);

    // Note that we use true for the last arg, even though we are creating this
    // from a handle, because we are pretty sure of the size and the flags.
    result = rxNewValidImage(context, hCursor, IMAGE_CURSOR, &s, LR_DEFAULTSIZE | LR_LOADFROMFILE, true);

done_out:
    return result;
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

    PNEWMOUSEPARAMS params = (PNEWMOUSEPARAMS)args;

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

    if ( ! params->dlgCSelf->dlgAllocated )
    {
        methodCanNotBeInvokedException(context, "new", DLG_HAS_ENDED_MSG, self);
        goto done_out;
    }

    pcm->rexxSelf        = self;
    pcm->dlgCSelf        = params->dlgCSelf;
    pcm->hDlg            = params->dlgCSelf->hDlg;
    pcm->rexxDlg         = params->dlgCSelf->rexxSelf;
    pcm->dlgProcThreadID = params->dlgCSelf->dlgProcThreadID;
    pcm->isDlgWindow     = params->isDlgWindow;
    pcm->controlCSelf    = params->controlCSelf;

    if ( pcm->isDlgWindow )
    {
        pcm->hWindow     = pcm->dlgCSelf->hDlg;
        pcm->rexxWindow  = pcm->dlgCSelf->rexxSelf;

        pcm->dlgCSelf->mouseCSelf = pcm;
        pcm->dlgCSelf->rexxMouse  = self;
    }
    else
    {
        pcm->hWindow    = pcm->controlCSelf->hCtrl;
        pcm->rexxWindow = pcm->controlCSelf->rexxSelf;

        pcm->controlCSelf->mouseCSelf = pcm;
        pcm->controlCSelf->rexxMouse  = self;
    }

    // Put this mouse object in the dialog's control bag to prevent it from
    // being garbage collected while the Rexx dialog object exists. It does not
    // matter if the mouse's owner window is a dialog control or a dialog.
    context->SendMessage1(pcm->rexxDlg, "PUTCONTROL", self);

done_out:
    LeaveCriticalSection(&crit_sec);
    return 0;
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

    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm == NULL )
    {
        goto error_out;
    }

    tme.cbSize = sizeof(TRACKMOUSEEVENT);

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

        tme.dwFlags = TME_QUERY;

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

        buf[strlen(buf)] = '\0';

        context->DirectoryPut(answer, context->String(buf), "EVENT");
        context->DirectoryPut(answer, context->UnsignedInt32(tme.dwHoverTime), "HOVERTIME");

        goto good_out;
    }
    else
    {
        tme.dwFlags   = flags;
        tme.hwndTrack = pcm->hWindow;

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
    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm == NULL )
    {
        return TheFalseObj;
    }

    PPOINT pt = (PPOINT)rxGetPoint(context, _pt, 1);
    if ( pt == NULL )
    {
        return TheFalseObj;
    }

    return DragDetect(pcm->hWindow, *pt) ? TheTrueObj : TheFalseObj;
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
 *            as the dialog's message processing loop.  So we might need to use
 *            SendMessage with one of the custom window messages.  However, it
 *            is possible that we are already running in that thread.
 *
 *            Note that we do not need the owner window handle for these
 *            methods, just the dialog window handle.
 */
RexxMethod2(RexxObjectPtr, mouse_get_release_capture, NAME, method, CSELF, pCSelf)
{
    RexxObjectPtr result = TheZeroObj;

    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm != NULL )
    {
        HWND hDlg = pcm->hDlg;

        if ( *method == 'G' )
        {
            HWND hwnd;

            if ( isDlgThread(pcm->dlgCSelf) )
            {
                hwnd = GetCapture();
            }
            else
            {
                hwnd = (HWND)SendMessage(hDlg, WM_USER_MOUSE_MISC, MF_GETCAPTURE, 0);
            }
            result = pointer2string(context, hwnd);
        }
        else
        {
            uint32_t rc = 0;

            if ( isDlgThread(pcm->dlgCSelf) )
            {
                if ( ReleaseCapture() == 0 )
                {
                    rc = GetLastError();
                }
            }
            else
            {
                rc = (uint32_t)SendMessage(hDlg, WM_USER_MOUSE_MISC, MF_RELEASECAPTURE, 0);
            }

            if ( rc != 0 )
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
 */
RexxMethod1(RexxObjectPtr, mouse_capture, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNegativeOneObj;

    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm != NULL )
    {
        HWND oldCapture;

        if ( isDlgThread(pcm->dlgCSelf) )
        {
            oldCapture = SetCapture(pcm->hWindow);
        }
        else
        {
            oldCapture = (HWND)SendMessage(pcm->hDlg, WM_USER_MOUSE_MISC, MF_SETCAPTURE, (LPARAM)pcm->hWindow);
        }
        result = pointer2string(context, oldCapture);
    }
    return result;
}


/** Mouse::isButtonDown()
 *
 *  Determines if one of the mouse buttons is down.
 *
 *  @param  whichButton  [OPTIONAL]  Keyword indicating which mouse button
 *                       should be queried. By default it is the left button
 *                       that is queried.
 *
 *  @return  True if the specified mouse button was down, otherwise false
 *
 *  @note  Sets the .SystemErrorCode, but there is nothing that would change it
 *         to not zero.
 *
 *  @remarks  The key state must be handled in the window thread, so
 *            SendMessage() has to be used.
 */
RexxMethod2(RexxObjectPtr, mouse_isButtonDown, OPTIONAL_CSTRING, whichButton, CSELF, pCSelf)
{
    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm == NULL )
    {
        return TheFalseObj;
    }

    int32_t mb = VK_LBUTTON;
    if ( argumentExists(1) )
    {
        if ( StrCmpI(whichButton,      "LEFT" )    == 0 ) mb = VK_LBUTTON;
        else if ( StrCmpI(whichButton, "RIGHT" )   == 0 ) mb = VK_RBUTTON;
        else if ( StrCmpI(whichButton, "MIDDLE")   == 0 ) mb = VK_MBUTTON;
        else if ( StrCmpI(whichButton, "XBUTTON1") == 0 ) mb = VK_XBUTTON1;
        else if ( StrCmpI(whichButton, "XBUTTON2") == 0 ) mb = VK_XBUTTON2;
        else
        {
            return wrongArgValueException(context->threadContext, 1, MOUSE_BUTTON_KEYWORDS, whichButton);
        }
    }

    if ( GetSystemMetrics(SM_SWAPBUTTON) )
    {
        if ( mb == VK_LBUTTON )
        {
            mb = VK_RBUTTON;
        }
        else if ( mb == VK_RBUTTON )
        {
            mb = VK_LBUTTON;
        }
    }

    short state;
    if ( isDlgThread(pcm->dlgCSelf) )
    {
        state = (short)GetAsyncKeyState(mb);
    }
    else
    {
        state = (short)SendMessage(pcm->hDlg, WM_USER_MOUSE_MISC, MF_BUTTONDOWN, mb);
    }

    return (state & ISDOWN) ? TheTrueObj : TheFalseObj;
}

/** Mouse::setCursorPos()
 *
 *  Moves the cursor to the specified position.
 *
 *  @param  newPos  The new position (x, y), in pixels. The amount can be
 *                  specified in these formats:
 *
 *      Form 1:  A .Point object.
 *      Form 2:  x, y
 *
 *  @return  0 for success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  No effort is made to ensure that a .Size object, not a .Point
 *            object is used.
 *
 *            There is no requirement that the underlying dialog exists.
 */
RexxMethod2(RexxObjectPtr, mouse_setCursorPos, ARGLIST, args, CSELF, pCSelf)
{
    pCMouse pcm = getMouseCSelf(context, pCSelf);
    if ( pcm == NULL )
    {
        return TheFalseObj;
    }

    size_t sizeArray;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 1, 2, &sizeArray, &argsUsed) )
    {
        return NULLOBJECT;
    }

    if ( argsUsed == 1 && sizeArray == 2)
    {
        return tooManyArgsException(context->threadContext, 1);
    }

    if ( SetCursorPos(point.x, point.y) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
}


/** Mouse::getCursorPos()
 *
 *  Retrieves the current cursor position in pixels.
 *
 *  @return The cursor position as a .Point object.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  There is no requirement that the underlying dialog exists.
 */
RexxMethod1(RexxObjectPtr, mouse_getCursorPos, CSELF, pCSelf)
{
    pCMouse pcm = getMouseCSelf(context, pCSelf);
    if ( pcm == NULL )
    {
        return TheFalseObj;
    }

    POINT p = {0};
    if ( GetCursorPos(&p) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return rxNewPoint(context, p.x, p.y);
}


/** Mouse::setCursor()
 *
 *  Sets the cursor shape for the owner window of this mouse.
 *
 *  @param  cursor  [Required] Identifies the cursor to be set.  This argument
 *                  can either be a cursor .Image, or a cursor keyword.
 *
 *                  A cursor keyword sets the cursor to one of the System's
 *                  predefined cursors.  Acceptable keywords are: APPSTARTING,
 *                  ARROW, CROSS, HAND, HELP, IBEAM, NO, SIZEALL, SIZENESW,
 *                  SIZENS, SIZENWSE, SIZEWE, UPARROW, or WAIT
 *
 *  Note that this method implementation also provides the implementation for
 *  some shortcut methods carried over from pre 4.2.0, see the remarks.  To do
 *  this we make the first arguement optional, and if the method is actually
 *  setCursor(), we then require the argument.
 *
 *  Pre 4.2.0 only supported these methods, there was no setCursorShape, or
 *  setCursor, so there is no default cursor.  (Only restoreCursorShape() had a
 *  default cursor of IDC_ARROW.)
 *
 *
 *
 *  WindowExtensions::Cursor_Arrow()
 *  WindowExtensions::Cursor_AppStarting()
 *  WindowExtensions::Cursor_Cross()
 *  WindowExtensions::Cursor_No()
 *  WindowExtensions::Cursor_Wait()
 *
 *  For 4.2.0, we only support these short cut methods, which map to the old
 *  methods as is logical:
 *
 *  arrow()
 *  appStarting()
 *  cross()
 *  no()
 *  wait()
 *
 *
 *  old method name, needs to be deleted: winex_setCursorShape
 *
 *
 *
 */
RexxMethod3(RexxObjectPtr, mouse_setCursor, OPTIONAL_RexxObjectPtr, _cursor, NAME, method, CSELF, pCSelf)
{
    RexxObjectPtr result = TheZeroObj;

    pCMouse pcm = getMouseCSelf(context, pCSelf);
    if ( pcm == NULL )
    {
        goto done_out;
    }

    CSTRING cursor = NULL;
    HCURSOR hCursor = NULL;

    if ( StrCmp(method, "SETCURSOR") == 0 )
    {
        if ( argumentOmitted(1) )
        {
            missingArgException(context->threadContext, 1);
            goto done_out;
        }

        if ( context->IsOfType(_cursor, "IMAGE") )
        {
            POODIMAGE poiCursor = rxGetImageCursor(context, _cursor, 1);
            if ( poiCursor == NULLOBJECT )
            {
                goto done_out;
            }
            hCursor = (HCURSOR)poiCursor->hImage;
        }
        else if ( context->IsString(_cursor))
        {
            cursor = keyword2cursor(context, context->ObjectToStringValue(_cursor));
            if ( cursor == NULL )
            {
                goto done_out;
            }
        }
        else
        {
            wrongArgValueException(context->threadContext, 1, "Image or String", _cursor);
            goto done_out;
        }
    }
    else
    {
        switch ( *method )
        {
            case 'A' :
                cursor = (method[1] == 'R' ? IDC_ARROW : IDC_APPSTARTING);
                break;
            case 'C' :
                cursor = IDC_CROSS;
                break;
            case 'N' :
                cursor = IDC_NO;
                break;
            case 'W' :
                cursor = IDC_WAIT;
                break;
            default :
                // Should be impossible, so we don't raise and internal error exception.
                goto done_out;
        }
    }

    if ( cursor != NULL )
    {
        hCursor = (HCURSOR)LoadImage(NULL, cursor, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        if ( hCursor == NULL )
        {
            oodSetSysErrCode(context->threadContext);
            goto done_out;
        }
    }

    result = mouseSetCursor(context, pcm, hCursor);
    if ( result == NULL )
    {
        result = TheZeroObj;
    }

done_out:
    return result;
}


/** Mouse::restoreCursor()
 *
 *  Restores the cursor.  This is a convenience method, the same function could
 *  be achieved using setCursor()
 *
 *  @param  cursor  [OPTIONAL]  A cursor .Image to set the cursor to. If this
 *                     argument is omitted, the cursor is set to the System
 *                     arrow cursor.  See the remarks.
 *
 *  @return  The previous cursor, or a null handle if there was no previous
 *           cursor.
 *
 *  @note  Best practice would most likely be to use restoreCursor() to restore
 *         a saved cursor, returned from setCursor()
 *
 *         Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, mouse_restoreCursor, OPTIONAL_RexxObjectPtr, newCursor, CSELF, pCSelf)
{
    RexxObjectPtr result = TheZeroObj;

    pCMouse pcm = getMouseCSelf(context, pCSelf);
    if ( pcm == NULL )
    {
        goto done_out;
    }

    HCURSOR hCursor;
    if ( argumentExists(1) )
    {
        if ( context->IsOfType(newCursor, "IMAGE") )
        {
            POODIMAGE poiCursor = rxGetImageCursor(context, newCursor, 1);
            if ( poiCursor == NULLOBJECT )
            {
                goto done_out;
            }
            hCursor = (HCURSOR)poiCursor->hImage;
        }
        else
        {
            wrongClassException(context->threadContext, 1, "Image");
            goto done_out;
        }
    }
    else
    {
        hCursor = (HCURSOR)LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
        if ( hCursor == NULL )
        {
            oodSetSysErrCode(context->threadContext);
            goto done_out;
        }
    }

    result = mouseSetCursor(context, pcm, hCursor);

done_out:
    return result;
}


/** Mouse::showCursor()
 *
 * Displays or hides the cursor.
 *
 * @param  show  [Optional]  Specifies whether the internal display counter is
 *               to be incremented or decremented.
 *
 *               If .true, the display count is incremented by one. If .false,
 *               the display count is decremented by one.  The defualt is .true.
 *
 * @return  The new display count.
 *
 * @notes   The .SystemErrorCode is set back to 0 by this method, but there is
 *          no situation that would cause it to be changed to an error code.
 *
 *          The operating system maintains an internal display counter that
 *          determines whether the cursor should be displayed. The cursor is
 *          displayed only if the display count is greater than or equal to 0.
 *          If a mouse is installed, the initial display count is set to 0. If
 *          no mouse is installed, the display count is -1.
 *
 *          In early versions of Windows the internal counter was system wide,
 *          but now it is thread specific.  What this means in terms of ooDialog
 *          is that the cursor state, shown or hidden, will apply to the dialog
 *          and all its child windows.  The dialog control windows, and any
 *          ControlDialog windows if present.
 *
 *          Note that this method does not hide or show the cursor, it changes
 *          the internal counter. This implies that if the programmer invokes
 *          this method 10 times with show equal to .false, it will take at
 *          least 10 invocations of the method with show equal to .true to get
 *          the cursor to show.
 */
RexxMethod2(int32_t, mouse_showCursor, OPTIONAL_logical_t, show, CSELF, pCSelf)
{
    pCMouse pcm = getMouseCSelf(context, pCSelf);
    if ( pcm == NULL )
    {
        return 0;
    }

    if ( argumentOmitted(1) )
    {
        show = TRUE;
    }

    if ( isDlgThread(pcm->dlgCSelf) )
    {
        return ShowCursor((BOOL)show);
    }
    else
    {
        return (int32_t)SendMessage(pcm->hDlg, WM_USER_MOUSE_MISC, MF_SHOWCURSOR, (LPARAM)show);
    }
}


/** Mouse::clipCursor()
 *
 *  Confines the cursor to a rectangular area on the screen. Once the cursor has
 *  been configned, if the cursor is moved outside the rectangle, the operating
 *  system automatically adjusts the position to keep the cursor inside the
 *  rectangular area.
 *
 *  @param  [REQUITED] A bounding rectangle specified as a .Rect object that
 *          defines the area the cursor is confined to.  Use
 *          Mouse::releaseClipCursor() to allow the cursor to move anywhere on
 *          the screen.
 *
 *  @return  If the method succeeds the return is .true.  On error the return is
 *           .false and .SystemErrorCode will be set to the operating system
 *           error code.
 *
 *  @notes   Sets the .SystemErrorCode.
 *
 *           The cursor is a shared resource. If an application confines the
 *           cursor, it must release the cursor at some point by using
 *           releaseClipCursor() before the user can move to another
 *           application using the mouse.
 *
 *           Although the MSDN documentation does not explicity state this,
 *           experimentation shows that if the user brings up the Alt-Tab
 *           switching dialog and moves to another application in that manner,
 *           the cursor is no lnoger confined.  If the user closes the dialog,
 *           the mouse is no longer confined.
 *
 *  @remarks For ClipCursor() MDSN says: The calling process must have
 *           WINSTA_WRITEATTRIBUTES access to the window station.  Tested under
 *           a regular account on Win7 and saw no problem.
 *
 *           TODO using ARGLIST, if all arguments are omitted, there is a syntax
 *           error raised saying arg 1 is required.  This seems an interpreter
 *           bug, or mis-design.  To work around this, releaseClipCursor() is
 *           implemented.  There is no ReleaseClipCursor in the Windows API, but
 *           releaseClipCursor() in the Mouse class follows the same pattern as
 *           capture() / getCapture() / releaseCapture for the mouse.  We now
 *           have clipCursor() / getClipCursor() / releaseClipCursor(), which
 *           may actually be preferable.  Still need to check why the syntax
 *           condition is raised.
 */
RexxMethod2(logical_t, mouse_clipCursor, ARGLIST, args, CSELF, pCSelf)
{
    pCMouse pcm = getMouseCSelf(context, pCSelf);
    if ( pcm == NULL )
    {
        return 0;
    }

    RECT   r = {0};
    size_t arraySize;
    size_t argsUsed;

    if ( ! getRectFromArglist(context, args, (PORXRECT)&r, true, 1, 4, &arraySize, &argsUsed) )
    {
        return FALSE;
    }
    if ( argsUsed < arraySize )
    {
        tooManyArgsException(context->threadContext, argsUsed);
        return FALSE;
    }

    if ( ClipCursor(&r) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return FALSE;
    }
    return TRUE;
}


/** Mouse::releaseClipCursor
 *
 *  Releases the cursor so that it is free to move anywhere on the screen.
 *
 *  @return  True on success, false on error.
 *
 *  @notes   Sets the .SystemErrorCode.
 *
 *           The cursor is a shared resource. If an application confines the
 *           cursor, it must release the cursor at some point by using
 *           releaseClipCursor() before the user can move to another
 *           application using the mouse.
 *
 *           Although the MSDN documentation does not explicity state this,
 *           experimentation shows that if the user brings up the Alt-Tab
 *           switching dialog and moves to another application in that manner,
 *           the cursor is no lnoger confined.  If the user closes the dialog,
 *           the mouse is no longer confined.
 */
RexxMethod1(logical_t, mouse_releaseClipCursor, CSELF, pCSelf)
{
    pCMouse pcm = getMouseCSelf(context, pCSelf);
    if ( pcm == NULL )
    {
        return 0;
    }

    if ( ClipCursor(NULL) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return FALSE;
    }
    return TRUE;
}


/** Mouse::getClipCursor()
 *
 *  Retrieves the screen coordinates of the rectangular area to which the cursor
 *  is confined.
 *
 *  @param  rect  [Required]  A .Rect object in which the coordinates are
 *                returned.
 *
 *  @return  True on success, false on error.
 *
 *  @notes  Sets the .SystemErrorCode variable.
 *
 *          If the cursor is not confined, on return the .Rect object will
 *          contain the dimensions of the screen.
 */
RexxMethod1(logical_t, mouse_getClipCursor, RexxObjectPtr, _rect)
{
    PRECT r = (PRECT)rxGetRect(context, _rect, 1);
    if ( r == NULL )
    {
        return FALSE;
    }

    if ( GetClipCursor(r) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return FALSE;
    }
    return TRUE;
}


/** Mouse::connectEvent()
 *
 *  @param  event       [required]  Event keyword.
 *  @param  methodName  [optional]  methodName.
 *  @param  willReply   [optional]  If the interpreter should wait for the reply
 *                                  from the connected method in the window
 *                                  procedure.  The default is true.
 *  @param  opts        [optional]  Additional keyword options.  These keywords
 *                                  effect how the message is processed by the
 *                                  ooDialog framework in its interaction with
 *                                  the underlying window message loop.
 *
 * @return  True on success, false on error.
 *
 * @note  For mouse events, there are several possible ways for the ooDialog
 *        framework to process the underlying notification message.  The
 *        possible processing is dependent on which type of window the mouse
 *        event is for, a dialog window or a dialog control window.
 *
 *        Dialog window mouse event
 *        ========================
 *
 *        1.)  Tell the operating system that the message was processed.
 *
 *        2.)  Tell the OS the message was not processed.
 *
 *        Dialog control mouse event
 *        ==========================
 *
 *        1.)  Tell the operating system that the message was processed.
 *
 *        2.)  Pass the message up the window chain.
 *
 *        3.)  Pass the message on to the dialog control it was intended for.
 *
 *        The 'willReply' and 'opts' arguments control the generic way the
 *        framework processes the event.  Note that some mouse events may be
 *        documented specifically as behaving different from the generic
 *        processing.  The generic processing is summarized in the following
 *        table:
 *
 *        Event    will   reply   opts          action
 *        Window   Reply  value   arg           taken
 *        ==================================================================
 *        dialog   true   .true   ignored       OS told message processed.
 *        dialog   true   .false  ignored       OS told message not processed.
 *        dialog   false  n/a     none          OS told message processed.
 *        dialog   false  n/a     REPLYFALSE    OS told message not processed.
 *        ------------------------------------------------------------------
 *        control  true   .true   ignored       OS told message processed.
 *        control  true   .false  none          message passed to dialog control.
 *        control  n/a    n/a     SENDTODLG     message passed up to dialog
 *        control  false  n/a     none          OS told message processed.
 *        control  false  n/a     SENDTOCONTROL message passed to dialog control.
 *
 *        Note in the table above, when the opts arg is SENDTODLG and the event
 *        window is a dialog control, no Rexx method is invoked at all, the
 *        message is handled in a way such that the notification message is
 *        passed straight on to the dialog. If a method is connected to the
 *        *dialog* mouse event, then that method will be invoked.  The willReply
 *        argument and the methodName argument are in effect ignored.
 */
RexxMethod5(RexxObjectPtr, mouse_connectEvent, CSTRING, event, OPTIONAL_CSTRING, methodName,
            OPTIONAL_logical_t, _willReply, OPTIONAL_CSTRING, opts, CSELF, pCSelf)
{
    pCEventNotification pcen = getMousePCEN(context, (pCMouse)pCSelf);
    if ( pcen == NULL )
    {
        goto err_out;
    }
    pCMouse pcm = (pCMouse)pCSelf;

    uint32_t wmMsg;
    if ( ! keyword2wm(context, event, &wmMsg) )
    {
        goto err_out;
    }

    if ( argumentOmitted(2) || *methodName == '\0' )
    {
        methodName = wm2name(wmMsg);
    }

    bool willReply = argumentOmitted(3) || _willReply;
    uint32_t tag = TAG_MOUSE;

    if ( pcm->isDlgWindow )
    {
        tag |= willReply ? TAG_REPLYFROMREXX : 0;

        if ( argumentExists(4) )
        {
            if ( StrCmpI(opts, "REPLYFALSE" ) == 0 )
            {
                tag |= willReply ? 0 : TAG_REPLYFALSE;
            }
            else
            {
                wrongArgValueException(context->threadContext, 4, "REPLYFALSE", opts);
                goto err_out;
            }
        }

        if ( addMiscMessage(pcen, context, wmMsg, UINT32_MAX, 0, 0, 0, 0, methodName, tag) )
        {
            return TheTrueObj;
        }
    }
    else
    {
        tag = CTRLTAG_MOUSE;
        tag |= willReply ? CTRLTAG_REPLYFROMREXX : 0;

        if ( argumentExists(4) )
        {
            if ( StrCmpI(opts, "SENDTODLG" ) == 0 )
            {
                tag |= CTRLTAG_SENDTODLG;
            }
            else if ( StrCmpI(opts, "SENDTOCONTROL" ) == 0 )
            {
                tag |= CTRLTAG_SENDTOCONTROL;
            }
            else
            {
                wrongArgValueException(context->threadContext, 4, "SENDTOCONTROL or SENDTODLG", opts);
                goto err_out;
            }
        }

        WinMessageFilter wmf = {0};
        wmf.wm       = wmMsg;
        wmf.wmFilter = 0xFFFFFFFF;
        wmf.method   = methodName;
        wmf.tag      = tag;

        if ( addSubclassMessage(context, pcm->controlCSelf, &wmf) )
        {
            return TheTrueObj;
        }
    }

err_out:
    return TheFalseObj;
}


/** Mouse::Test()
 *
 * Notes on mouse and CS_DBLCLKS style.  Dialogs already have this style, no
 * need for a method to add the style.  Buttons have this style.
 *
 */
RexxMethod1(RexxObjectPtr, mouse_test, CSELF, pCSelf)
{
    pCMouse pcm = requiredDlg(context, pCSelf);
    if ( pcm != NULL )
    {
        HWND hWindow = pcm->hWindow;

        ULONG_PTR style = getClassPtr(hWindow, GCL_STYLE);
        printf("Dialog Control Class style=0x%08zx\n", style);
        style |= CS_DBLCLKS;
        setClassPtr(hWindow, GCL_STYLE, style);
        printf("Adding dblclks new style=0x%08zx last error=%d\n", getClassPtr(hWindow, GCL_STYLE), GetLastError());
    }
    return TheTrueObj;
}


