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
 * controls.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <commctrl.h>
#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include <limits.h>
#include "oodCommon.hpp"


/**
 * Methods for the ProgressBar class.
 */
#define PROGRESSBAR_CLASS   "ProgressBar"


/**
 * Step the progress bar by the step increment or do a delta position.  A delta
 * position moves the progress bar from its current position by the specified
 * amount.
 *
 * Note this difference between stepping and doing a delta.  When the progress
 * bar is stepped and the step amount results in a position past the end of the
 * progress bar, the progress bar restarts at the minimum position.  When a
 * delta position is done, if the end of the progress bar is reached, it will
 * just stay at the end.
 *
 * @param  delta [Optional]  If present a delta position is done using this
 *               values.  If absent, then a step is done.
 *
 * @return  For both cases the previous position is returned.
 */
RexxMethod2(int, pbc_stepIt, OPTIONAL_int32_t, delta, OSELF, self)
{
    HWND hwnd = rxGetWindowHandle(context, self);

    if ( argumentOmitted(1) )
    {
        return (int)SendMessage(hwnd, PBM_STEPIT, 0, 0);
    }
    else
    {
        return (int)SendMessage(hwnd, PBM_DELTAPOS, delta, 0);
    }
}

/**
 * Set the position of the progress bar.
 *
 * @param newPos  Set the position to this value.
 *
 * @return The the old progress bar position.
 */
RexxMethod2(int, pbc_setPos, int32_t, newPos, OSELF, self)
{
    return (int)SendMessage(rxGetWindowHandle(context, self), PBM_SETPOS, newPos, 0);
}

RexxMethod1(int, pbc_getPos, OSELF, self)
{
    return (int)SendMessage(rxGetWindowHandle(context, self), PBM_GETPOS, 0, 0);
}

/** ProgressBar::setRange()
 *
 *  Sets the range for the progress bar using the full 32-bit numbers for the
 *  range.
 *
 *  @param min   Optional.  The low end of the range.  0 is the default.
 *  @param max   Optional.  The high end of the range.  100 is the default.
 *
 *  @return  The previous range in the form of a string with word(1) being the
 *           low end of the previous range and word(2) being the previous high
 *           end of the range.
 *
 *  @note    The returned range is not necessarily correct if the previous range
 *           has been set using the full 32-bit numbers now allowed by the
 *           progress bar control.  The returned numbers are restricted to
 *           0xFFFF.
 *
 *           The range is returned as a string because that was the way it was
 *           previously documented.
 *
 *           Use the getRange() method to get the correct range.
 *
 */
RexxMethod3(RexxStringObject, pbc_setRange, OPTIONAL_int32_t, min, OPTIONAL_int32_t, max, OSELF, self)
{
    TCHAR buf[64];
    HWND hwnd = rxGetWindowHandle(context, self);

    if ( argumentOmitted(1) )
    {
        min = 0;
    }
    if ( argumentOmitted(2) )
    {
        max = 100;
    }

    DWORD range = (DWORD)SendMessage(hwnd, PBM_SETRANGE32, min, max);
    _snprintf(buf, sizeof(buf), "%d %d", LOWORD(range), HIWORD(range));

    return context->String(buf);
}

RexxMethod1(RexxObjectPtr, pbc_getRange, OSELF, self)
{
    PBRANGE pbr;
    SendMessage(rxGetWindowHandle(context, self), PBM_GETRANGE, TRUE, (LPARAM)&pbr);

    RexxDirectoryObject d = context->NewDirectory();
    context->DirectoryPut(d, context->Int32(pbr.iLow), "MIN");
    context->DirectoryPut(d, context->Int32(pbr.iHigh), "MAX");

    return d;
}

RexxMethod2(int, pbc_setStep, OPTIONAL_int32_t, newStep, OSELF, self)
{
    if ( argumentOmitted(1) )
    {
        newStep = 10;
    }
    return (int)SendMessage(rxGetWindowHandle(context, self), PBM_SETSTEP, newStep, 0);
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
RexxMethod3(logical_t, pbc_setMarquee, OPTIONAL_logical_t, on, OPTIONAL_uint32_t, pause, OSELF, self)
{
    if ( ! requiredComCtl32Version(context, "setMarquee", COMCTL32_6_0) )
    {
        return 0;
    }

    HWND hwnd = rxGetWindowHandle(context, self);

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
 *  ProgressBar::backgroundColor()
 *
 *  Sets the background color of the progress bar.
 *
 *  @param   colorRef  [Required]  A COLOREF, the new background color.
 *
 *  @return  The previous background color, or CLR_DEFAULT if the previous color
 *           was the defualt.  This is returned as a COLORREF number.
 *
 *  The progress bar control only supports this function under Windows Classic
 *  Theme.
 */
RexxMethod2(uint32_t, pbc_setBkColor, uint32_t, colorRef, OSELF, self)
{
    return (uint32_t)SendMessage(rxGetWindowHandle(context, self), PBM_SETBKCOLOR, 0, colorRef);
}

/**
 *  ProgressBar::barColor()
 *
 *  Sets the bar color of the progress bar.
 *
 *  @param   colorRef  [Required]  A COLOREF, the new bar color.
 *
 *  @return  The previous bar color, or CLR_DEFAULT if the previous color
 *           was the defualt.  This is returned as a COLORREF number.
 *
 *  The progress bar control only supports this function under Windows Classic
 *  Theme.
 */
RexxMethod2(uint32_t, pbc_setBarColor, uint32_t, colorRef, OSELF, self)
{
    return (uint32_t)SendMessage(rxGetWindowHandle(context, self), PBM_SETBARCOLOR, 0, colorRef);
}


/**
 * Methods for the ScrollBar class.
 */
#define SCROLLBAR_CLASS   "ScrollBar"


RexxMethod3(logical_t, sb_setRange, , on, OPTIONAL_uint32_t, pause, OSELF, self)
{
}

size_t RexxEntry HandleScrollBar(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND w;

   CHECKARGL(2);

   if (!strcmp(argv[0].strptr, "SR"))
   {
       DOUBLE ret=-LONG_MAX+1;
       CHECKARG(5);

       w = GET_HWND(argv[1]);
       ret = atof(argv[2].strptr);
       if (ret < LONG_MIN) ret = 1-LONG_MAX;

       if (SetScrollRange(w, SB_CTL, (LONG)ret, (LONG)atof(argv[3].strptr), isYes(argv[4].strptr)))
          RETC(0)
       else
          RETC(1)
    }
    else
    if (!strcmp(argv[0].strptr, "GR"))
    {
       INT min, max;

       w = GET_HWND(argv[1]);
       if (GetScrollRange(w, SB_CTL, &min, &max))
       {
          sprintf(retstr->strptr, "%ld %ld", min, max);
          retstr->strlength = strlen(retstr->strptr);
          return 0;
       }
       else
       {
          retstr->strlength = 0;
          return 0;
       }
    }
    else
    if (!strcmp(argv[0].strptr, "SP"))
    {
       CHECKARG(4);

       w = GET_HWND(argv[1]);
       if (SetScrollPos(w, SB_CTL, atol(argv[2].strptr), isYes(argv[3].strptr)))
          RETC(0)
       else
          RETC(1)
    }
    else
    if (!strcmp(argv[0].strptr, "GP"))
    {
       LONG pos;

       w = GET_HWND(argv[1]);
       pos = GetScrollPos(w, SB_CTL);
       sprintf(retstr->strptr, "%ld", pos);
       retstr->strlength = strlen(retstr->strptr);
       return 0;
    }
    RETERR
}


