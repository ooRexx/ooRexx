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
/******************************************************************************/
/* REXX Kernel                                                  rexxc.c       */
/*                                                                            */
/* Translates a program and saves it in an output file                        */
/*                                                                            */
/* Common code for AIX and LINUX                                              */
/*                                                                            */
/******************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined( HAVE_FEATURES_H )
# include <features.h>
#endif

#if defined( HAVE_NL_TYPES_H )
# include <nl_types.h>
#endif

#include "rexx.h"
#include "RexxMessageNumbers.h"
#define REXXMESSAGEFILE    "rexx.cat"

#define BUFFERLEN         256           /* Length of message bufs used        */
#ifdef LINUX
#define SECOND_PARAMETER 1              /* different sign. Lin-AIX            */
#define CATD_ERR -1
#else
#define SECOND_PARAMETER 0              /* 0 for no  NL_CAT_LOCALE            */
#endif

void DisplayError(int msgid)           /* simplified catalog access@MAE004M */
{
#if defined( HAVE_NL_TYPES_H )
    nl_catd        catd;                  /* catalog descriptor from catopen() */
#endif
    int            set_num = 1;           /* message set 1 from catalog        */
    const char    *message;               /* message pointer                   */
    char           DataArea[256];         /* buf to return message             */

#if defined( HAVE_CATOPEN )
    /* open message catalog in NLSPATH   */
    if ((catd = catopen(REXXMESSAGEFILE, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
    {
        sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
        if ((catd = catopen(DataArea, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
        {
            printf("\n*** Cannot open REXX message catalog %s.\nNot in NLSPATH or %s.\n",
                   REXXMESSAGEFILE, ORX_CATDIR );
            return;                      /* terminate program                   */
        }
    }                                 /* retrieve message from repository  */
    message = catgets(catd, set_num, msgid, NULL);
    if (!message)                    /* got a message ?                     */
#if defined(OPSYS_LINUX) && !defined(OPSYS_SUN)
    {
        sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
        if ((catd = catopen(DataArea, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
        {
            printf("\nCannot open REXX message catalog %s.\nNot in NLSPATH or %s.\n",
                   REXXMESSAGEFILE, ORX_CATDIR);
        }
        else
        {
            message = catgets(catd, set_num, msgid, NULL);
            if (!message)                  /* got a message ?                   */
            {
                printf("\n Error message not found!\n");
            }
            else
            {
                printf("\n%s\n", message);  /* print the message                 */
            }
        }
    }
#else
    {
        printf("*** Error message not found!");
    }
    else
    {
        printf("\n%s\n", message);    /* print the message                 */
    }
#endif
    catclose(catd);                   /* close the catalog                 */
#else
    printf("*** Cannot get description for error %d!", msgid);
#endif
    return;                           /* terminate program                 */
}

int main (int argc, char **argv)
{
    bool silent = false;
    int silentp;
    char *ptr;
    /* check for /s option               */
    for (silentp = 1; silentp < argc; silentp++)
    {
        if (argv[silentp][0] == '-' &&
            (argv[silentp][1] == 's' || argv[silentp][1] == 'S'))
        {
            silent = true;
            break;
        }
    }
    if (!silent)                       /* display version and copyright     */
    {
        ptr = RexxGetVersionInformation();
        printf(ptr);
        if (ptr)
        {
            free(ptr);
        }
    }
    /* Check validity of arguments       */
    if (argc < 2 || argc > 4 ||          /* # args exceeding bounds           */
        (silent && argc==2) ||             /* -s is the first argument          */
        (silent && (silentp + 1 < argc)) ||  /* -s is not the last argument       */
        (!silent && argc==4))           /* 3 arguments, but no /s            */
    {
        if (argc > 2)
        {
            DisplayError((int)Error_REXXC_cmd_parm_incorrect_msg);
        }
        DisplayError((int) Error_REXXC_wrongNrArg_msg);
        DisplayError((int) Error_REXXC_SynCheckInfo_msg);
        exit(-1);                          /* terminate with an error           */
    }                                    /* end additions                     */
    /* modified control logic            */
    if ((argc==4 && silent) || (argc==3 && !silent))
    {
        if (strcmp(argv[1], argv[2]) == 0)
        {
            DisplayError((int)Error_REXXC_outDifferent_msg);
            exit(-2);                        /* terminate with an error           */
        }
        /* translate and save the output     */
        return RexxTranslateProgram(argv[1], argv[2], NULL);
    }
    else                                 /* just doing syntax check           */
    {
        return RexxTranslateProgram(argv[1], NULL, NULL);
    }
}
