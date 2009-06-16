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
#include "ooDialog.h"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include <limits.h>
#include "oodCommon.h"


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

       if (SetScrollRange(w, SB_CTL, (LONG)ret, (LONG)atof(argv[3].strptr), IsYes(argv[4].strptr)))
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
       if (SetScrollPos(w, SB_CTL, atol(argv[2].strptr), IsYes(argv[3].strptr)))
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


