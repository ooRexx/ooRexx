/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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

const char *nextArgument(BOOL getprog, const char *argptr, PULONG ndx, PULONG len, BOOL allocate)
{
    PCHAR ret;
    if (argptr[*ndx] == ' ')                      /* skip blanks of previous argument */
    {
        while ((argptr[*ndx] == ' ') && argptr[*ndx])
        {
            (*ndx)++;
        }
    }
    *len = 0;
    const char *tmp = &argptr[*ndx];

    if (!allocate)
    {
        while ((argptr[*ndx] != ' ') && argptr[*ndx])
        {
            if (argptr[*ndx] == '\"') do
                {
                    if (argptr[*ndx] != '\"')
                    {
                        (*len)++;
                    }
                    (*ndx)++;
                } while ((argptr[*ndx] != '\"') && argptr[*ndx]);
            if (argptr[*ndx])
            {
                if (argptr[*ndx] != '\"')
                {
                    (*len)++;
                }
                (*ndx)++;
            }
        }
    }
    /* get the program name for REXXHIDE */
    else if (getprog)
    {
        if (argptr[*ndx] == '\"')
            do
            {
                (*len)++;
                (*ndx)++;
            } while ((argptr[*ndx] != '\"') && argptr[*ndx]);
        while (argptr[*ndx] && (argptr[*ndx] != ' '))
        {
            (*len)++;
            (*ndx)++;
        }
    }
    else
    {
        while (argptr[*ndx])
        {
            (*len)++;
            (*ndx)++;
        }
    }

    if (*len)
    {
        if (allocate)
        {
            /* program name must not be enclosed within "" for REXXHIDE */
            if (getprog && (tmp[0] == '\"'))
            {
                tmp++;(*len)-=2;
            };

            ret = (PCHAR) GlobalAlloc(GMEM_FIXED, (*len)+1);
            memcpy(ret, tmp, (*len)+1);
            if (getprog)
            {
                ret[*len]='\0';
            }
            return ret;
        }
        else
        {
            return tmp;
        }
    }
    else
    {
        return NULL;
    }
}



PCONSTRXSTRING getArguments(const char **program, const char *argptr, size_t *count, PCONSTRXSTRING retarr)
{
    ULONG i, isave, len;
    /* don't forget the break after program_name */
    BOOL lastSW = false;

    i = 0;
    if (program)
    {
        (*program) = nextArgument(TRUE, argptr, &i, &len, TRUE);  /* for REXXHIDE script is first argument */
    }
    else {
        nextArgument(FALSE, argptr, &i, &len, FALSE);             /* skip REXX*.EXE */
        /* need to skip any switches before the script name/-e switch   */
        const char *tmp = nextArgument(FALSE, argptr, &i, &len, FALSE);       /* skip REXX script or -e switch */
        while (tmp && strlen(tmp) > 1 && (tmp[0] == '/' || tmp[0] == '-') && !lastSW) {
            /* the following test ensure that the -e switch on rexx.exe is not included in the arguments */
            /* passed to the running program as specified on the command line. Unfortunately it also     */
            /* affects rexxhide, rexxpaws, etc, that all use this code; may not be important             */
            if (tmp[1] == 'e' || tmp[1] == 'E') {   /* no more switches after -e    */
                lastSW = true;
            }
            tmp = nextArgument(FALSE, argptr, &i, &len, FALSE);
        }
    }

    retarr->strptr = NULL;
    isave = i;
    *count = 0;
    /* scan for any non-blank characters to determine if arguments are present */
    while (argptr[i] == ' ')
        i++;
    if (argptr[i]) (*count)++;
    if (*count)
    {
        i = isave;
        retarr->strptr = nextArgument(FALSE, argptr, &i, &len, TRUE);
        retarr->strlength = len;
    }
    return retarr;
}


void freeArguments(const char *program, PCONSTRXSTRING arguments)
{
   if (arguments->strptr) GlobalFree((HGLOBAL)arguments->strptr);
   if (program) GlobalFree((HGLOBAL)program);
}

// Utility to parse out a command line string into the unix-style
// argv/argc format.  Used for setting the array of arguments
// in .local

PCHAR* CommandLineToArgvA(PCHAR CmdLine, int32_t* _argc)
{
    char     **argv;
    char      *_argv;
    size_t    len;
    int32_t   argc;
    char      a;
    size_t    i, j;

    BOOLEAN  in_QM;
    BOOLEAN  in_TEXT;
    BOOLEAN  in_SPACE;

    len = strlen(CmdLine);
    i = ((len+2)/2)*sizeof(void *) + sizeof(void *);

    argv = (char**)GlobalAlloc(GMEM_FIXED,
                               i + (len+2)*sizeof(char));

    _argv = (PCHAR)(((PUCHAR)argv)+i);

    argc = 0;
    argv[argc] = _argv;
    in_QM = FALSE;
    in_TEXT = FALSE;
    in_SPACE = TRUE;
    i = 0;
    j = 0;

    while ( a = CmdLine[i] )
    {
        if (in_QM)
        {
            if (a == '\"')
            {
                in_QM = FALSE;
            }
            else
            {
                _argv[j] = a;
                j++;
            }
        }
        else
        {
            switch (a)
            {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if (in_SPACE)
                    {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if (in_TEXT)
                    {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if (in_SPACE)
                    {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
            }
        }
        i++;
    }
    _argv[j] = '\0';
    argv[argc] = NULL;

    (*_argc) = argc;
    return argv;
}
