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
/* REXX Unix Support                                            aixerr.c      */
/*                                                                            */
/* Retrieve message from message repository using the X/Open catopen(),       */
/* catgets() and catclose() function calls.                                   */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>                     /* include standard headers          */
#include <string.h>
#include <ctype.h>
#include <limits.h>

#if defined( HAVE_NL_TYPES_H )
# include <nl_types.h>
#endif

#if defined( HAVE_MESG_H )
# include <mesg.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#define ERROR_TABLE                    /* include error message table       */
#include "RexxCore.h"                    /* incl general definitions        */
#include "StringClass.hpp"
#include "SystemInterpreter.hpp"

                                       /* define macros to bulid entries in */
                                       /* the msgEntry table for msg lookup */
#define MAJOR(code)   {code, code##_msg},/* Major error codes                 */
#define MINOR(code)   {code, code##_msg},/* Minor error codes (sub-codes)     */

typedef struct msgEntry {              /* define for error table entries    */
 int    code;                          /* error message code                */
 int    msgid;                         /* error message number              */
} ERROR_MESSAGE;

#include "RexxMessageNumbers.h"        /* include  definition of errorcodes */
#include "RexxMessageTable.h"          /* include actual table definition   */

#ifdef LINUX
//#define SECOND_PARAMETER MCLoadAll   /* different sign. Lin-AIX           */
#define SECOND_PARAMETER 1             /* different sign. Lin-AIX           */
#define CATD_ERR -1                    /* Duplicate for AIX                 */
#else
#define SECOND_PARAMETER 0             /* 0 for no  NL_CAT_LOCALE           */
#endif


/**
 * Retrieve the message text for a give error code.
 *
 * @param code   The Rexx error code
 *
 * @return The error message associated with that code.
 */
RexxString *SystemInterpreter::getMessageText(wholenumber_t code )
{
#if defined( HAVE_NL_TYPES_H )
    nl_catd        catd;                  /* catalog descriptor from catopen() */
#endif
    int            set_num = 1;           /* message set 1 from catalog        */
    ERROR_MESSAGE *p;                     /* message table scan pointer        */
    int            msgid;                 /* message number                    */
    char           DataArea[256];         /* buf to return message             */
    const char *   message;
    /* loop through looking for the      */
    /* error code                        */
#if defined( HAVE_CATOPEN )
    for (p = Message_table; p->code != 0; p++)
    {
        if (p->code == code)
        {              /* found the target code?            */

            msgid = p->msgid;                 /* get msg number associated w/ error*/
                                              /* open message catalog in NLSPATH   */
            if ((catd = catopen(REXXMESSAGEFILE, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
            {
                sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
                if ((catd = catopen(DataArea, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
                {
                    sprintf(DataArea, "Cannot open REXX message catalog %s.  Not in NLSPATH or %s.",
                            REXXMESSAGEFILE, ORX_CATDIR);
                    return new_string(DataArea);
                }
            }                                   /* retrieve message from repository  */
            message = catgets(catd, set_num, msgid, NULL);
            if (!message)                    /* got a message ?                   */
            {
#if defined(OPSYS_LINUX) && !defined(OPSYS_SUN)
                sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
                if ((catd = catopen(DataArea, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
                {
                    sprintf(DataArea, "Cannot open REXX message catalog %s.  Not in NLSPATH or %s.",
                            REXXMESSAGEFILE, ORX_CATDIR);
                    return new_string(DataArea);
                }
                else
                {
                    message = catgets(catd, set_num, msgid, NULL);
                    if (!message)                    /* got a message ?                   */
                    {
                        strcpy(DataArea,"Error message not found!");
                    }
                    else
                    {
                        strcpy(DataArea, message);
                    }
                }
#else
                strcpy(DataArea,"Error message not found!");
#endif
            }
            else
            {
                strcpy(DataArea, message);
            }
            catclose(catd);                 /* close the catalog                 */
                                            /* convert and return the message    */
            return new_string(DataArea);
        }
    }
    return OREF_NULL;                     /* no message retrieved              */
#else
    sprintf(DataArea,"Cannot get description for error %d",msgid);
    return new_string(&DataArea);
#endif
}


/**
 * Return a message header for a given error message.
 *
 * @param code   The error code
 *
 * @return The formatted message header
 */
RexxString *SystemInterpreter::getMessageHeader(wholenumber_t code )
{
    ERROR_MESSAGE *p;                     /* table scan pointer                */
    int            msgid;                 /* message number                    */
    char           DataArea[20];          /* buf addr to return message        */
                                          /* loop through looking for the      */
                                          /* error code                        */
    for (p = Message_table; p->code != 0; p++)
    {
        if (p->code == code)
        {              /* found the target code?            */
            msgid = p->msgid;                 /* get msg number associated w/ error*/
                                              /* format as a message header        */
            sprintf(DataArea, "REX%4.4dE: ", msgid);
            return new_string(DataArea);     /* return as a string object         */
        }
    }
    return OREF_NULL;                     /* no message retrieved              */
}



