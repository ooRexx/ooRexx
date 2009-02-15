/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
#include <windows.h>
#include <rexx.h>
#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif
#include "oovutil.h"

extern WPARAM InterruptScroll = 0;
extern HWND ScrollingButton = NULL;
extern HWND RedrawScrollingButton = NULL;
HANDLE TimerEvent = NULL;
ULONG TimerCount = 0;
ULONG_PTR Timer = 0;

#define GETWEIGHT(opts, weight) \
      if (strstr(opts, "THIN")) weight = FW_THIN; else \
      if (strstr(opts, "EXTRALIGHT")) weight = FW_EXTRALIGHT; else \
      if (strstr(opts, "LIGHT")) weight = FW_LIGHT; else \
      if (strstr(opts, "MEDIUM")) weight = FW_MEDIUM; else \
      if (strstr(opts, "SEMIBOLD")) weight = FW_SEMIBOLD; else \
      if (strstr(opts, "EXTRABOLD")) weight = FW_EXTRABOLD; else \
      if (strstr(opts, "BOLD")) weight = FW_BOLD; else \
      if (strstr(opts, "HEAVY")) weight = FW_HEAVY; else \
      weight = FW_NORMAL;



void DrawFontToDC(HDC hDC, INT x, INT y, const char * text, INT size, const char * opts, const char * fontn, INT fgColor, INT bkColor)
{
   HFONT hFont, oldF;
   INT weight, oldMode=0;
   COLORREF oldFg, oldBk;

   GETWEIGHT(opts, weight)

   hFont = CreateFont(size, size, 0, 0, weight, strstr(opts, "ITALIC") != 0, strstr(opts, "UNDERLINE") != 0,
               strstr(opts, "STRIKEOUT") != 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, fontn);

   oldF = (HFONT)SelectObject(hDC, hFont);
   if (strstr(opts, "TRANSPARENT")) oldMode = SetBkMode(hDC, TRANSPARENT);
   else if (strstr(opts, "OPAQUE")) oldMode = SetBkMode(hDC, OPAQUE);
   if (fgColor != -1) oldFg = SetTextColor(hDC, PALETTEINDEX(fgColor));
   if (bkColor != -1) oldBk = SetBkColor(hDC, PALETTEINDEX(bkColor));


   TextOut(hDC, x, y, text, (int)strlen(text));
   SelectObject(hDC, oldF);
   DeleteObject(hFont);
   if (oldMode) SetBkMode(hDC, oldMode);
   if (fgColor != -1) SetTextColor(hDC, oldFg);
   if (bkColor != -1) SetBkColor(hDC, oldBk);
}


size_t RexxEntry WriteText(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND w;
   HDC hDC;

   if (argc >= 7)
   {
       INT bkC = -1, fgC = -1;
       w = GET_HWND(argv[0]);

       if (strstr(argv[6].strptr, "CLIENT"))
           hDC = GetDC(w);
       else
           hDC = GetWindowDC(w);

       if (argc >= 8) fgC = atoi(argv[7].strptr);
       if (argc >= 9) bkC = atoi(argv[8].strptr);

       if (hDC)
       {
                             /* x                     y                    text                size                options      fontname */
          DrawFontToDC(hDC, atoi(argv[1].strptr), atoi(argv[2].strptr), argv[3].strptr, atoi(argv[5].strptr), argv[6].strptr, argv[4].strptr, fgC, bkC);
          ReleaseDC(w, hDC);
          RETC(0)
       }
   }
   else
   {
       CHECKARG(4);

       hDC = (HDC)GET_HANDLE(argv[0]);

       if (NULL != hDC)
       {
          TextOut(hDC, atoi(argv[1].strptr), atoi(argv[2].strptr), argv[3].strptr, (int)argv[3].strlength);
          RETC(0)
       }
   }
   RETC(1)
}


VOID CALLBACK ScrollTimerProc(
    HWND  hwnd,    // handle of window for timer messages
    UINT  uMsg,    // WM_TIMER message
    UINT  idEvent,    // timer identifier
    DWORD  dwTime     // current system time
   )
{
    SetEvent(TimerEvent);
}



size_t RexxEntry ScrollText(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    INT disply;
    const char *opts;
    INT size;
    const char *text;
    INT col;

    const char * tp;
    HWND w;
    HDC hDC;
    HFONT hFont, oldF;
    HPEN hpen, oP;
    HBRUSH oB, hbr;
    RECT r, rs, rclip;
    SIZE s, sone;
    INT i, rc, sl, step, j, disp, weight;
    UINT sleep;
    DEF_ADM;

    CHECKARG(10);

    GET_ADM;

    if (!dlgAdm) RETERR

        text = argv[2].strptr;
    size = atoi(argv[4].strptr);
    opts = argv[5].strptr;
    disply = atoi(argv[6].strptr);
    col = atoi(argv[9].strptr);

    w = GET_HWND(argv[1]);
    step = atoi(argv[7].strptr);
    tp = text;

    if (NULL != (hDC = GetWindowDC(w)))
    {
        GetWindowRect(w, &r);

        GETWEIGHT(opts, weight)

        hFont = CreateFont(size, size, 0, 0, weight, strstr(opts, "ITALIC") != NULL, strstr(opts, "UNDERLINE") != NULL,
                           strstr(opts, "STRIKEOUT") != NULL, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, FF_DONTCARE, argv[3].strptr);

        oldF = (HFONT)SelectObject(hDC, hFont);

        hpen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
        hbr = GetSysColorBrush(COLOR_BTNFACE);

        SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
        oP = (HPEN)SelectObject(hDC, hpen);
        oB = (HBRUSH)SelectObject(hDC, hbr);

        if (col > 0) SetTextColor(hDC, PALETTEINDEX(col));
        sl = (int)strlen(text);
        rc = GetTextExtentPoint32(hDC, text, sl, &s);

        r.right = r.right - r.left;
        r.bottom = r.bottom - r.top + disply;
        r.top = disply;
        r.left = 0;

        j = 0;
        disp = 0;
        rc = GetTextExtentPoint32(hDC, tp, 1, &sone);
        rclip= r;
        rs.top = r.top;
        rs.bottom = r.top+s.cy+2;
        rs.right = r.right;
        rs.left = r.right;

        sleep = atoi(argv[8].strptr);
        if (sleep)
        {
            if (!TimerEvent)
            {
                TimerEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
                Timer = SetTimer(NULL,GetCurrentThreadId(), sleep, (TIMERPROC)ScrollTimerProc);
                TimerCount++;
            }
            else TimerCount++;
        }
        else Timer = 0;

        ScrollingButton = w;
        for (i=step; i<=r.right+s.cx; i+=step)
        {
            if (i>=s.cx+step)
            {
                Rectangle(hDC, r.right-i+s.cx, r.top, r.right -i+s.cx+step+step, r.top + s.cy + 2);
            }

            if (j<sl)
            {
                if (RedrawScrollingButton == w)
                {
                    rc = TextOut(hDC, r.right - i, r.top, text, sl);
                }
                else
                {
                    rc = TextOut(hDC, r.right - i+disp, r.top, tp, 1);
                }
            }
            if ((j<sl) && (!rc)) break;

            RedrawScrollingButton = NULL;

            if (i-disp>sone.cx)
            {
                tp++;
                j++;
                disp += sone.cx;
                rc = GetTextExtentPoint32(hDC, tp, 1, &sone);
            }

            rs.left -= step;
            rc = 0;

            if (dlgAdm->StopScroll == (WPARAM) w)
            {
                dlgAdm->StopScroll = 0;
                break;
            }

            if (!ScrollDC(hDC, -step, 0, &rs, &rclip, NULL, NULL)) break;

            if (Timer)
            {
                WaitForSingleObject(TimerEvent, (DWORD)((double)sleep*1.5));
                ResetEvent(TimerEvent);
            }

        }

        if (Timer)
        {
            if (TimerCount == 1)
            {
                KillTimer(NULL, Timer);
                if (TimerEvent) CloseHandle(TimerEvent);
                TimerEvent = NULL;
                TimerCount = 0;
                Timer = 0;
            }
            else TimerCount--;
        }

        ScrollingButton = NULL;

        if (!dlgAdm || (!IsWindow(dlgAdm->TheDlg)))
        {
            RETC(1);
        }
        Rectangle(hDC, r.left, r.top, r.right, r.bottom);
        SelectObject(hDC, oldF);
        SelectObject(hDC, oP);
        SelectObject(hDC, oB);

        // Don't delete hbr, its a system cached brush
        DeleteObject(hpen);
        DeleteObject(hFont);
        ReleaseDC(w, hDC);
        RETC(0);
    }
    RETC(1);
}



size_t RexxEntry HandleFont(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    CHECKARG(5);

    if ( !strcmp(argv[0].strptr, "CREATE") )
    {
        HFONT hFont;
        INT weight;

        GETWEIGHT(argv[3].strptr, weight)

        hFont = CreateFont(atoi(argv[2].strptr), atoi(argv[4].strptr), 0, 0, weight,
                           strstr(argv[3].strptr, "ITALIC") != NULL,
                           strstr(argv[3].strptr, "UNDERLINE") != NULL,
                           strstr(argv[3].strptr, "STRIKEOUT") != NULL,
                           DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                           DEFAULT_QUALITY, FF_DONTCARE, argv[1].strptr);

        if ( hFont )
        {
            RETHANDLE(hFont);
        }
        RETC(0);
    }
    RETC(1);
}
