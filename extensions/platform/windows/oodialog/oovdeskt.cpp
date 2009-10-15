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
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include "oodCommon.hpp"


/**
 * Gets the window handle of the dialog control that has the focus.  The call to
 * GetFocus() needs to run in the window thread of the dialog to ensure that the
 * correct handle is obtained.
 *
 * @param hDlg    Handle to the dialog of interest.
 * @param retstr  Rexx string to return the result in.
 */
static void getCurrentFocus( HWND hDlg, RXSTRING *retstr )
{
   HWND hW = (HWND)SendMessage(hDlg, WM_USER_GETFOCUS, 0,0);
   pointer2string(retstr, (void *)hW);
}

size_t RexxEntry Wnd_Desktop(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND  hW;

   CHECKARGL(1);

   if (!strcmp(argv[0].strptr,"SETFOC"))
   {
       CHAR qualifier = 'C';
       LRESULT result;
       HWND hDlg, hNextFocus;
       CHECKARGL(3);

       hDlg = GET_HWND(argv[1].strptr);
       if ( ! IsWindow(hDlg) )
          RETVAL(-1)

       /* Get the handle of the window control that had the focus before setting
        * the new focus.  This will be returned to the caller in retstr.
        */
       getCurrentFocus(hDlg, retstr);

       if ( argc > 3 )
          qualifier = argv[3].strptr[0];

       switch ( qualifier )
       {
          case 'F' :  /* set window to Foreground like Wnd_Desktop("TOP"..) but returns hwnd of previous control */
             hNextFocus = GET_HWND(argv[2]);
             if ( ! IsWindow(hNextFocus) )
                RETVAL(-1)
             result = SetForegroundWindow(hNextFocus);
             break;

          case 'N' :  /* set focus on Next tab stop control */
             result = SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)0, (LPARAM)FALSE);
             break;

          case 'P' :  /* set focus on Previous tab stop control */
             result = SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)1, (LPARAM)FALSE);
             break;

          case 'C' :  /* set focus on specified Control */
          default  :
              hNextFocus = GET_HWND(argv[2]);
             if ( ! IsWindow(hNextFocus) )
                RETVAL(-1)
             result = SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hNextFocus, TRUE);
             break;
       }
       if ( ! result )
          RETVAL(-1)  /* change retstr to indicate failure */
       else
          return 0;   /* retstr has handle of control that previously had the focus */
   }
   else if (!strcmp(argv[0].strptr,"GETFOC"))
   {
       CHECKARG(2);

       hW = GET_HWND(argv[1]);

       /* return the handle of the window control that has the focus */
       getCurrentFocus(hW, retstr);
       return 0;
   }
   else if (!strcmp(argv[0].strptr,"TOP"))       /* put window to the foreground */
   {
       CHECKARG(2);
       hW = GetForegroundWindow();
       HWND hTarget = GET_HWND(argv[1]);
       if (hW == hTarget) RETHANDLE(hW)
       if ( SetForegroundWindow(hTarget) )
           RETHANDLE(hW)  /* return the handle of the window that had the focus before */
       else
           RETVAL(0)   /* indicate failure */
   }
   else if (!strcmp(argv[0].strptr,"FG"))       /* get foreground window */
   {
       RETHANDLE(GetForegroundWindow())
   }
   else if (!strcmp(argv[0].strptr,"CAP"))     /* get/set/release the mouse capture */
   {   /* capture must be handled by window thread, therefore sendmessage is used */
       CHECKARG(3);
       hW = GET_HWND(argv[1]);
       if (argv[2].strptr[0] == 'G')
           RETVAL((ULONG)SendMessage(hW, WM_USER_GETSETCAPTURE, 0,0))
       else
       if (argv[2].strptr[0] == 'R')
           RETC(!SendMessage(hW, WM_USER_GETSETCAPTURE, 2,0))
       else {
           hW = GET_HWND(argv[2]);
           if (hW)
           {
              RETVAL((long)SendMessage(hW, WM_USER_GETSETCAPTURE, 1, (LPARAM)hW))
           }
           RETC(0)
       }
       RETERR
   }
   else if (!strcmp(argv[0].strptr,"CURSOR"))     /* get/set the mouse cursor */
   {
       if (argc == 1)
       {
           POINT pt;
           GetCursorPos(&pt);
           retstr->strlength = sprintf(retstr->strptr, "%ld %ld", pt.x, pt.y);
           return 0;
       }
       else
       if (argc == 3)
       {
           RETC(!SetCursorPos(atol(argv[1].strptr), atol(argv[2].strptr)))
       }
       else
       if (argc == 4)
       {
           hW = GET_HWND(argv[1]);
           if (argv[2].strptr[0] == 'S')
           {
               LONG res;
               HCURSOR oC, hC;
               res = atoi(argv[3].strptr);
               hC = LoadCursor(NULL, MAKEINTRESOURCE(res));
               oC = (HCURSOR)setClassPtr(hW, GCLP_HCURSOR, (LONG_PTR)hC);
               SetCursor(hC);
               RETHANDLE(oC)
           }
           else if (argv[2].strptr[0] == 'R')
           {
               HCURSOR hC = (HCURSOR)GET_HANDLE(argv[3]);
               if (hC) {
                   setClassPtr(hW, GCLP_HCURSOR, (LONG_PTR)hC);
                   RETHANDLE(SetCursor(hC))
               }
               else
               {
                   setClassPtr(hW, GCLP_HCURSOR, (LONG_PTR)LoadCursor(NULL, IDC_ARROW));
                   RETHANDLE(SetCursor(LoadCursor(NULL, IDC_ARROW)))
               }
           }
       }
       RETERR
   }
   else if (!strcmp(argv[0].strptr,"KSTAT"))     /* key state and mouse buttons*/
   {    /* keystate must be handled by window thread, therefore sendmessage is used */
       CHECKARG(3);
       hW = GET_HWND(argv[1]);
       if (hW)
       {
           LONG k = atoi(argv[2].strptr);
           if (GetSystemMetrics(SM_SWAPBUTTON))
           {
               if (k == 1) k = 2;
               else if (k==2) k = 1;
           }
           RETVAL((ULONG)SendMessage(hW, WM_USER_GETKEYSTATE, k,0))
       }
       RETC(0)
   }
   else if (!strcmp(argv[0].strptr,"COORD"))     /* screen to client and  vice versa */
   {
       CHECKARG(5);
       hW = GET_HWND(argv[1]);
       retstr->strptr[0] = '\0';
       retstr->strlength = 0;
       if (hW)
       {
           POINT pt;
           BOOL ret;
           pt.x = atol(argv[2].strptr);
           pt.y = atol(argv[3].strptr);
           if (!stricmp(argv[4].strptr, "S2C"))
               ret = ScreenToClient(hW, &pt);
           else
               ret = ClientToScreen(hW, &pt);
           if (ret)
           {
               retstr->strlength = sprintf(retstr->strptr, "%ld %ld", pt.x, pt.y);
               return 0;
           }
       }
       return 0;
   }
   RETERR
}

