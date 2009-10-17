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

   if (!strcmp(argv[0].strptr,"CAP"))     /* get/set/release the mouse capture */
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

