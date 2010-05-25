/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
 * oodBarControls.cpp
 *
 * Contains methods for the ScrollBar, TrackBar, and ProgressBar dialog
 * controls.  Also the misc control UpDown
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <commctrl.h>
#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include <limits.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"


/**
 * Methods for the ProgressBar class.
 */
#define PROGRESSBAR_CLASS   "ProgressBar"

static RexxDirectoryObject pbGetFullRange(RexxMethodContext *c, HWND hPB)
{
    PBRANGE pbr;
    SendMessage(hPB, PBM_GETRANGE, TRUE, (LPARAM)&pbr);

    RexxDirectoryObject d = c->NewDirectory();
    c->DirectoryPut(d, c->Int32(pbr.iLow), "MIN");
    c->DirectoryPut(d, c->Int32(pbr.iHigh), "MAX");

    return d;
}


/** ProgressBar::setFullRange()
 *
 *  Sets the range for the progress bar using the full 32-bit numbers for the
 *  range.
 *
 *  The range can be specified using one argument, a directory object, or two
 *  arguments
 *
 *  Form 1:
 *
 *  @param   aDirectory  [OPTIONAL]  The directory indexes "min" and "max" are
 *                       used to set the range.  The min index specifies the low
 *                       end of the range and the max index specifies the high
 *                       end of the range.  If either, (or both,) indexes are
 *                       omitted, the low end of the range is set to 0 and the
 *                       high end is set to 100, as appropriate to the missing
 *                       index(es).
 *
 *  Form 2:
 *
 *  @param   min   [OPTIONAL]  The low end of the range.  0 is the default.
 *  @param   max   [OPTIONAL]  The high end of the range.  100 is the default.
 *
 *  @return  The previous range as a directory object with the indexes min and
 *           max.
 *
 *  @remarks The getRange() method is maintained for backwards compatibility. it
 *           returns the range as a string.  For that method, the returned range
 *           is not necessarily correct if the previous range has been set using
 *           the full 32-bit numbers now allowed by the progress bar control.
 *           The returned numbers are restricted to 0xFFFF.
 *
 *           Use the getFullRange() method to get the correct range.
 */
RexxMethod4(RexxObjectPtr, pbc_setFullRange, OPTIONAL_RexxObjectPtr, minObj, OPTIONAL_int32_t, max, NAME, method, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    HWND hwnd = getDChCtrl(pCSelf);
    int32_t min = 0;

    RexxDirectoryObject result = NULLOBJECT;
    bool usingDirectory = false;
    bool fullRange = method[3] == 'F';

    if ( argumentExists(1) )
    {
        if ( c->IsOfType(minObj, "DIRECTORY") )
        {
            usingDirectory = true;
            if ( ! rxIntFromDirectory(context, (RexxDirectoryObject)minObj, "MIN", &min, 1, false) )
            {
                goto done_out;
            }
            max = 100;
            if ( ! rxIntFromDirectory(context, (RexxDirectoryObject)minObj, "MAX", &max, 1, false) )
            {
                goto done_out;
            }
        }
        else
        {
            if ( ! context->Int32(minObj, &min) )
            {
                wrongRangeException(context->threadContext, 1,  INT32_MIN, INT32_MAX, minObj);
            }
        }
    }

    if ( ! usingDirectory && argumentOmitted(2) )
    {
        max = 100;
    }

    if ( fullRange )
    {
        result = pbGetFullRange(context, hwnd);
    }

    uint32_t range = (uint32_t)SendMessage(hwnd, PBM_SETRANGE32, min, max);

    if ( ! fullRange )
    {
        result = context->NewDirectory();
        context->DirectoryPut(result, context->Int32(LOWORD(range)), "MIN");
        context->DirectoryPut(result, context->Int32(HIWORD(range)), "MAX");
    }

done_out:
    return result;
}

RexxMethod1(RexxObjectPtr, pbc_getFullRange, CSELF, pCSelf)
{
    return pbGetFullRange(context, getDChCtrl(pCSelf));
}

/**
 *  ProgressBar::setMarquee()  Turn marquee mode on or off.
 *
 *  @param   on     [Optional]  Stop or start marquee mode.  Default is to
 *                  start.
 *
 *  @param   pause  [Optional]  Time in milliseconds between updates.  Default
 *                  is 1000 (1 second.)
 *
 *  @return  True (always.)
 *
 *  Requires XP Common Controls version 6.0 or greater.
 */
RexxMethod3(logical_t, pbc_setMarquee, OPTIONAL_logical_t, on, OPTIONAL_uint32_t, pause, CSELF, pCSelf)
{
    if ( ! requiredComCtl32Version(context, "setMarquee", COMCTL32_6_0) )
    {
        return 0;
    }

    HWND hwnd = getDChCtrl(pCSelf);

    if ( ! hasStyle(hwnd, PBS_MARQUEE) )
    {
        wrongWindowStyleException(context, "progress bar", "PBS_MARQUEE");
        return 0;
    }

    if ( argumentOmitted(1) )
    {
        on = 1;
    }
    if ( argumentOmitted(2) )
    {
        pause = 1000;
    }

    /* The Windows message always returns 1, return 1 for .true (succeeded.) */
    SendMessage(hwnd, PBM_SETMARQUEE, on, pause);
    return 1;
}


/**
 * Methods for the ScrollBar class.
 */
#define SCROLLBAR_CLASS   "ScrollBar"


RexxMethod4(logical_t, sb_setRange, int32_t, min, int32_t, max, OPTIONAL_logical_t, redraw, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    redraw = (argumentOmitted(3) ? TRUE : redraw);

    SCROLLINFO si = {0};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE;
    si.nMax = max;
    si.nMin = min;

    if ( SetScrollInfo(getDChCtrl(pCSelf), SB_CTL, &si, (BOOL)redraw) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return 1;
    }
    return 0;
}


RexxMethod1(RexxObjectPtr, sb_getRange, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxDirectoryObject result = context->NewDirectory();
    context->DirectoryPut(result, TheNegativeOneObj, "MIN");
    context->DirectoryPut(result, TheNegativeOneObj, "MAX");

    SCROLLINFO si = {0};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE;

    if ( GetScrollInfo(getDChCtrl(pCSelf), SB_CTL, &si) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
    }
    else
    {
        context->DirectoryPut(result, context->Int32(si.nMin), "MIN");
        context->DirectoryPut(result, context->Int32(si.nMax), "MAX");
    }
    return result;
}

RexxMethod3(logical_t, sb_setPosition, int32_t, pos, OPTIONAL_logical_t, redraw, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    redraw = (argumentOmitted(3) ? TRUE : redraw);

    SCROLLINFO si = {0};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS;
    si.nPos = pos;

    if ( SetScrollInfo(getDChCtrl(pCSelf), SB_CTL, &si, (BOOL)redraw) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return 0;
    }
    return 1;
}

RexxMethod1(int32_t, sb_getPosition, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    SCROLLINFO si = {0};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS;

    if ( GetScrollInfo(getDChCtrl(pCSelf), SB_CTL, &si) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return si.nPos;
}


/**
 * Methods for the TrackBar class.  Note that this is the .Slider class in
 * ooDialog.  yghr.
 */
#define TRACK_BAR_CLASS  "TrackBar"
#define SLIDER_CLASS     "Slider"


/** TrackBar::getRange()
 *
 */
RexxMethod1(RexxObjectPtr, tb_getRange, CSELF, pCSelf)
{
    RexxDirectoryObject result = context->NewDirectory();
    HWND hCtrl = getDChCtrl(pCSelf);

    context->DirectoryPut(result, context->Intptr((intptr_t)SendMessage(hCtrl, TBM_GETRANGEMIN, 0,0)), "MIN");
    context->DirectoryPut(result, context->Intptr((intptr_t)SendMessage(hCtrl, TBM_GETRANGEMAX, 0,0)), "MAX");

    return result;
}


/** TrackBar::getSelRange()
 *
 */
RexxMethod1(RexxObjectPtr, tb_getSelRange, CSELF, pCSelf)
{
    RexxDirectoryObject result = context->NewDirectory();
    HWND hCtrl = getDChCtrl(pCSelf);

    context->DirectoryPut(result, context->Intptr((intptr_t)SendMessage(hCtrl, TBM_GETSELSTART, 0,0)), "START");
    context->DirectoryPut(result, context->Intptr((intptr_t)SendMessage(hCtrl, TBM_GETSELEND, 0,0)), "END");

    return result;
}



/**
 * Methods for the UpDown class.
 */
#define UP_DOWN_CLASS      "UpDown"


/** UpDown::deltaPosReply() [Class method]
 *
 *  Constructs the (proper) reply object for a DELTAPOS UpDown event
 *  notification.
 *
 *  The Rexx programmer should use this method as the reply to the notification.
 *
 *  @param change    [optional] If true, the delta position message is to be
 *                   canceled or modified.  If false, there is no change to the
 *                   delta position message, and the other arguments are always
 *                   ignored. The default is false.
 *
 *  @param cancel    [optional] If true, the delta position message is canceled
 *                   completely.  If false the message is not canceled, and the
 *                   delta position is set to newDelta.  The default is false.
 *
 *  @param newDelta  [optional] The amount by which the current position of the
 *                   updown control is changed.  The default is 1.  This
 *                   argument is only used if change is true and cancel is
 *                   false.
 */
RexxMethod3(RexxObjectPtr, ud_deltaPosReply_cls, OPTIONAL_logical_t, change, OPTIONAL_logical_t, cancel, OPTIONAL_int32_t, newDelta)
{
    if ( ! change )
    {
        return TheFalseObj;
    }

    RexxBufferObject _dpr = context->NewBuffer(sizeof(DELTAPOSREPLY));
    if ( _dpr == NULLOBJECT )
    {
        return TheFalseObj;
    }

    PDELTAPOSREPLY pdpr = (PDELTAPOSREPLY)context->BufferData(_dpr);
    pdpr->change = true;
    pdpr->cancel = cancel ? true : false;

    if ( ! pdpr->cancel )
    {
        pdpr->newDelta = argumentExists(1) ? newDelta : 1;
    }
    return _dpr;
}


/** UpDown::setRange()
 *
 *  Sets the range, (minimum and maximum,) for the up down control.
 *
 *  Unlike other some other controls, the maximum position may be less than the
 *  minimum, and in that case clicking the up arrow button decreases the current
 *  position. To put it another way, up means moving towards the maximum
 *  position.
 *
 *  The range can be specified using one argument, a directory object, or two
 *  arguments
 *
 *  Form 1:
 *
 *  @param   aDirectory  [OPTIONAL]  The directory indexes "min" and "max" are
 *                       used to set the range.  The min index specifies the
 *                       minium postition and the max index specifies the
 *                       maximum position.  If an index is omitted, the
 *                       corresponding default for that index is set.  0 for the
 *                       minimum position and 100 for the maximum.
 *
 *  Form 2:
 *
 *  @param   min   [OPTIONAL]  The minimum position.  0 is the default.
 *  @param   max   [OPTIONAL]  The maximum.  100 is the default.
 *
 *  @return  0 always.
 *
 */
RexxMethod3(RexxObjectPtr, ud_setRange, OPTIONAL_RexxObjectPtr, minObj, OPTIONAL_int32_t, max, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    int32_t min = 0;
    bool usingDirectory = false;

    if ( argumentExists(1) )
    {
        if ( context->IsOfType(minObj, "DIRECTORY") )
        {
            usingDirectory = true;
            if ( ! rxIntFromDirectory(context, (RexxDirectoryObject)minObj, "MIN", &min, 1, false) )
            {
                goto done_out;
            }
            max = 100;
            if ( ! rxIntFromDirectory(context, (RexxDirectoryObject)minObj, "MAX", &max, 1, false) )
            {
                goto done_out;
            }
        }
        else
        {
            if ( ! context->Int32(minObj, &min) )
            {
                wrongRangeException(context->threadContext, 1,  INT32_MIN, INT32_MAX, minObj);
            }
        }
    }

    if ( ! usingDirectory && argumentOmitted(2) )
    {
        max = 100;
    }
    SendMessage(hwnd, UDM_SETRANGE32, min, max);

done_out:
    return TheZeroObj;
}


/** UpDown::getRange()
 *
 *
 */
RexxMethod1(RexxObjectPtr, ud_getRange, CSELF, pCSelf)
{
    RexxDirectoryObject result = context->NewDirectory();
    HWND hCtrl = getDChCtrl(pCSelf);

    int32_t min, max;
    SendMessage(hCtrl, UDM_GETRANGE32, (WPARAM)&min, (LPARAM)&max);

    RexxMethodContext *c = context;
    context->DirectoryPut(result, context->Int32(min), "MIN");
    context->DirectoryPut(result, context->Int32(max), "MAX");

    return result;
}


/** UpDown::getPosition()
 *
 *
 *  @note  Sets the .SystemErrorCode.  If the returned position is not correct,
 *         the .SystemErrorCode is set to
 *
 *         ERROR_INVALID_DATA (13)  "The data is invalid"

 */
RexxMethod1(int32_t, ud_getPosition, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    HWND hCtrl = getDChCtrl(pCSelf);

    BOOL error;
    int32_t pos = (int32_t)SendMessage(hCtrl, UDM_GETPOS32, 0, (LPARAM)&error);
    if ( error )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_DATA);
    }
   return pos;
}


RexxMethod1(RexxArrayObject, ud_getAcceleration, CSELF, pCSelf)
{
    HWND hCtrl = getDChCtrl(pCSelf);
    LPUDACCEL pUDA = NULL;

    size_t count = (size_t)SendMessage(hCtrl, UDM_GETACCEL, 0, NULL);
    RexxArrayObject result = context->NewArray(count);

    pUDA = (LPUDACCEL)malloc(count * sizeof(UDACCEL));
    if ( pUDA == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    SendMessage(hCtrl, UDM_GETACCEL, count, (LPARAM)pUDA);
    for ( size_t i = 0; i < count; i++ )
    {
        RexxDirectoryObject d = context->NewDirectory();
        context->DirectoryPut(d, context->UnsignedInt32(pUDA[i].nSec), "SECONDS");
        context->DirectoryPut(d, context->UnsignedInt32(pUDA[i].nInc), "INCREMENT");
        context->ArrayPut(result, d, i + 1);
    }

done_out:
    safeFree(pUDA);
    return result;
}


RexxMethod2(logical_t, ud_setAcceleration, RexxArrayObject, vals, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    HWND hCtrl = getDChCtrl(pCSelf);
    LPUDACCEL pUDA = NULL;

    size_t count = c->ArrayItems(vals);
    if ( count < 1 )
    {
        emptyArrayException(c->threadContext, 1);
        goto done_out;
    }

    pUDA = (LPUDACCEL)malloc(count * sizeof(UDACCEL));
    if ( pUDA == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    RexxDirectoryObject d;
    uint32_t secs, incr;

    for ( size_t i = 1; i <= count; i++ )
    {
        if ( ! rxDirectoryFromArray(context, vals, i, &d, 1) )
        {
            goto done_out;
        }
        if ( ! rxNumberFromDirectory(context, (RexxDirectoryObject)d, "SECONDS", &secs, 1, true) )
        {
            goto done_out;
        }
        if ( ! rxNumberFromDirectory(context, (RexxDirectoryObject)d, "INCREMENT", &incr, 1, true) )
        {
            goto done_out;
        }

        pUDA[i - 1].nInc = incr;
        pUDA[i - 1].nSec = secs;
    }

    SendMessage(hCtrl, UDM_SETACCEL, count, (LPARAM)pUDA);

done_out:
    safeFree(pUDA);
    return 0;
}

