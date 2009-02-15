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

const char *nextArgument(BOOL getprog, const char *argptr, PULONG ndx, PULONG len, BOOL allocate, ULONG maxarglength)
{
    PCHAR ret;
    if (argptr[*ndx] == ' ')                      /* skip blanks of previous argument */
    while ((argptr[*ndx] == ' ') && argptr[*ndx] && (*ndx<maxarglength)) (*ndx)++;
    *len = 0;
    const char *tmp = &argptr[*ndx];

    if (!allocate)
    {
        while ((argptr[*ndx] != ' ') && argptr[*ndx] && (*ndx<maxarglength))
        {
            if (argptr[*ndx] == '\"') do
            {
                if (argptr[*ndx] != '\"') (*len)++;
                (*ndx)++;
            } while ((argptr[*ndx] != '\"') && argptr[*ndx] && (*ndx<maxarglength));
            if (argptr[*ndx]) {
                if (argptr[*ndx] != '\"') (*len)++;
                 (*ndx)++;
            }
        }
    }
    /* get the program name for REXXHIDE */
    else if (getprog)
    {
        if (argptr[*ndx] == '\"')
        do {
            (*len)++;(*ndx)++;
        } while ((argptr[*ndx] != '\"') && argptr[*ndx] && (*ndx<maxarglength));
        while (argptr[*ndx] && (argptr[*ndx] != ' ') && (*ndx<maxarglength)) { (*len)++; (*ndx)++;}
    }
    else while (argptr[*ndx] && (*ndx<maxarglength)) { (*len)++; (*ndx)++;}

    if (*len)
    {
        if (allocate) {
            /* program name must not be enclosed within "" for REXXHIDE */
            if (getprog && (tmp[0] == '\"')) {tmp++;(*len)-=2;};

            ret = (PCHAR) GlobalAlloc(GMEM_FIXED, (*len)+1);
            memcpy(ret, tmp, (*len)+1);
            if (getprog) ret[*len]='\0';
            return ret;
        } else return tmp;
    }
    else return NULL;
}



PCONSTRXSTRING getArguments(const char **program, const char *argptr, size_t *count, PCONSTRXSTRING retarr)
{
    ULONG i, isave, len, maxarglen;
    /* don't forget the break after program_name */

    /* WindowsNT accepts 2048 bytes, Windows95/98 1024 bytes */
    maxarglen=2048;

    i = 0;
    if (program)
        (*program) = nextArgument(TRUE, argptr, &i, &len, TRUE, maxarglen);  /* for REXXHIDE script is first argument */
    else {
        nextArgument(FALSE, argptr, &i, &len, FALSE, maxarglen);             /* skip REXX*.EXE */
        const char *tmp = nextArgument(FALSE, argptr, &i, &len, FALSE, maxarglen);       /* skip REXX script or -e switch */
        /* the following test ensure that the -e switch on rexx.exe is not included in the arguments */
        /* passed to the running program as specified on the command line. Unfortunately it also     */
        /* affects rexxhide, rexxpaws, etc, that all use this code; may not be important             */
        if (tmp && strlen(tmp) > 1 && (tmp[0] == '/' || tmp[0] == '-') && (tmp[1] == 'e' || tmp[1] == 'E') )
          nextArgument(FALSE, argptr, &i, &len, FALSE, maxarglen);           /* skip REXX code*/
    }

    retarr->strptr = NULL;
    isave = i;
    *count = 0;
    if (nextArgument(FALSE, argptr, &i, &len, FALSE, maxarglen)) (*count)++;

    if (*count)
    {
        i = isave;
        retarr->strptr = nextArgument(FALSE, argptr, &i, &len, TRUE, maxarglen);
        retarr->strlength = len;
    }
    return retarr;
}


void freeArguments(const char *program, PCONSTRXSTRING arguments)
{
   if (arguments->strptr) GlobalFree((HGLOBAL)arguments->strptr);
   if (program) GlobalFree((HGLOBAL)program);
}
