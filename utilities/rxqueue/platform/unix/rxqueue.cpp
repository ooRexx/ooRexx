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

/*********************************************************************/
/*                                                                   */
/*  Program Name:     RXQUEUE.EXE                                    */
/*                                                                   */
/*  Description:      Program to act as a filter between an input    */
/*                    stream and the queueing services provided with */
/*                    with REXX-SAA/PL                               */
/*                                                                   */
/*  Usage:            RXQUEUE [ [ [/FIFO] [/LIFO] [/CLEAR] ]         */
/*                    [queuename] ]                                  */
/*                                                                   */
/*********************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined( HAVE_FEATURES_H )
# include <features.h>
#endif

#if defined( HAVE_NL_TYPES_H )
# include <nl_types.h>
#endif

#include <limits.h>
#include <stdio.h>             /* needed for screen output           */
#include <stdlib.h>            /* needed for miscellaneous functions */
#include <string.h>            /* needed for string functions        */
#include "rexx.h"              /* needed for queue functions & codes */
#include "RexxMessageNumbers.h"

#define RXQUEUE_CLEAR    -2    /* used for queue mode CLEAR flag     */
#define BAD_MESSAGE      -6    /* Exit RC for message not found.     */

#define MSG_BUF_SIZE    256    /* Error message buffer size          */

#define REXXMESSAGEFILE    "rexx.cat"
#ifndef CATD_ERR
#define CATD_ERR -1
#endif

char  line[4096];              /* buffer for data to add to queue    */
char  work[256];               /* buffer for queue name, if default  */
int   queuemode=-1;            /* mode for access to queue           */

void  options_error(int type, const char *queuename ) ;

                               /* function to read stdin             */
size_t get_line(char *, size_t, size_t *);


int main(
        int   argc,
        char *argv[] )
{
    int       i;                 /* loop counter for arguments         */
    int       rc;                /* return code from API calls         */
    size_t    entries;           /* number of entries in queue         */
    const char *quename=NULL;    /* initialize queuename to NULL       */
    size_t    linelen ;          /* input line length                  */
    CONSTRXSTRING  queuedata;    /* data added to the queue            */
    char *t;                     /* argument pointer                   */


/*********************************************************************/
/*  Initialize string buffers to empty strings:                      */
/*********************************************************************/

    memset(line, '\0', sizeof(line)); /* clear buffer 'line' -for data */
    memset(work, '\0', sizeof(work)); /* clear buffer 'work' -for      */
                                      /*   queuename                   */


/*********************************************************************/
/*  Interpret options from invocation and set appropriate values:    */
/*********************************************************************/

    for (i=1; i<argc; i++)
    {      /* go through the argument list...    */
        t = argv[i];               /* point to next argument             */
        if ((t[0]=='/') || (t[0]=='-'))
        {/*if options character in        */
            t[0]='/';                /* argument,  then make character '/' */
            if ( !strcasecmp(t,"/FIFO") && /* if FIFO flag and               */
                 queuemode==-1)       /*   no queuemode selected yet, then  */
            {
                queuemode=RXQUEUE_FIFO;/*   set queuemode to FIFO, otherwise */
            }
            else if ( !strcasecmp(t,"/LIFO") &&  /* if LIFO flag and         */
                      queuemode==-1)  /*   no queuemode selected yet, then  */
            {
                queuemode=RXQUEUE_LIFO;/*   set queuemode to LIFO, otherwise */
            }
            else if ( !strcasecmp(t,"/CLEAR") &&  /* if CLEAR flag and       */
                      queuemode==-1)  /*   no queuemode selected yet, then  */
            {
                queuemode=RXQUEUE_CLEAR;/*   set queue for CLEAR, otherwise  */
            }
            else
            {
                options_error(      /*  there was an error in invokation */
                                    0,
                                    quename ) ;
            }
        }
        else if (!quename)          /* if not option and no queue name    */
        {
            quename=t;               /*   yet, then assign name            */
        }
        else
        {
            options_error(       /* otherwise illegal parameter        */
                                 0,
                                 quename ) ;
        }
    }
    if (queuemode==-1)            /* if no queue mode requested, then   */
    {
        queuemode=RXQUEUE_FIFO;    /*   use default value of FIFO        */
    }


/*********************************************************************/
/*  Make sure there is a queue name before API calls:                */
/*********************************************************************/

    if (!quename)                 /* if there is no queue specified:    */
    {
                                  /* scan current environment           */
        if (!(quename = getenv("RXQUEUE")) || !quename)
        {
            quename = "SESSION";     /* use session if not found           */
        }
    }

/*********************************************************************/
/*  Call RxQueueQuery() to check for the existence of the queue:     */
/*********************************************************************/

    if ((rc=RexxQueryQueue(quename,/* search for existence of 'quename' */
                           &entries)))/* get number of entries in queue*/
    {
        options_error( rc,         /* generate error if API fails        */
                       quename );
    }

/*********************************************************************/
/*  Get all input data and write each line to the proper queue       */
/*  (not CLEAR):                                                     */
/*********************************************************************/

    if (queuemode != RXQUEUE_CLEAR)
    {     /* if not CLEAR operation...  */
        while (!get_line( line,             /* while more data passed in: */
                          sizeof( line ),
                          &linelen))
        {                                  /* express in RXSTRING form   */
            MAKERXSTRING(queuedata, line, linelen);
            if ((rc=RexxAddQueue(              /*   write info to the queue  */
                                               quename,       /*     queue to write into    */
                                               &queuedata,    /*information to add to queue */
                                               queuemode)))   /* size of information to add */
            {
                                                              /* FIFO || LIFO mode          */
                options_error( rc,             /* generate error if API fails*/
                               quename ) ;
            }
        }
    }
    else
    {
        // clearing is easy
        RexxClearQueue(quename);
    }
    exit(0);
}


/*********************************************************************/
/*                             End Of Main Program                   */
/*********************************************************************/

/*********************************************************************/
/* Function:           Print errors from RXQUEUE.EXE.                */
/*                                                                   */
/* Description:        Retrieve message from message file, print the */
/*                     message and exit.  If the message printed     */
/*                     successfully, exit with a return code of 0.   */
/*                     Otherwise, exit with a return code of         */
/*                     BAD_MESSAGE (-6).                             */
/*                                                                   */
/* Inputs:             Error code, active queue name.                */
/*                                                                   */
/* Outputs:            Nothing.  Called for side  effects only.      */
/*                                                                   */
/* Side effects:       Message retrieved from message file.  Message */
/*                     written to stdout.  Program exits.            */
/*                                                                   */
/*********************************************************************/

void options_error( int   type,        /* Error type.                */
                    const char *quename )    /* Name of offending queue.   */
{
    char     DataArea[ MSG_BUF_SIZE ];   /* Message buffer.            */
    char     achIMessage[2*MSG_BUF_SIZE];/* Message with insertion.    */
    int      MsgNumber;                  /* Message number.            */
#if defined( HAVE_NL_TYPES_H )
    nl_catd  catd;                       /* catalog descriptor         */
#endif
    int      set_num = 1;                /* message set 1 from catalog */
    const char *pszMessage;              /* message pointer            */
    char    *pInsert = NULL;             /* Pointer for insertion char */

    /*******************************************************************/
    /* Most error messages come from the REXX message file.  Set the   */
    /* initial setting of the file name to point to the REXX message   */
    /* file, and change it if necessary.                               */
    /*******************************************************************/
    ( void ) memset( DataArea,
                     0,
                     sizeof( DataArea ) ) ;

    /*******************************************************************/
    /* Set the message file and the message number from the error      */
    /* type.  If the message has substitution parameters, set them     */
    /* as well.  If the pointer to the substitution parameter is NULL, */
    /* substitute an empty string for the parameter.                   */
    /*******************************************************************/
    /* assign message numbers to error codes          */
    switch (type)
    {
        case 0: /* invocation error */
            MsgNumber = Error_RXQUE_syntax_msg;
            break;

        case RXQUEUE_NOTINIT:
            MsgNumber = Error_RXQUE_notinit_msg;
            break;

        case RXQUEUE_NOEMEM:
            MsgNumber = Error_RXQUE_nomem;
            break;

        case RXQUEUE_SIZE:
            MsgNumber = Error_RXQUE_size_msg;
            break;

        case RXQUEUE_BADQNAME:
            MsgNumber = Error_RXQUE_name_msg;
            break;

        case RXQUEUE_PRIORITY:
            MsgNumber = Error_RXQUE_access_msg;
            break;

        case RXQUEUE_NOTREG:
            MsgNumber = Error_RXQUE_exist_msg;
            break;

        default:
            MsgNumber = Error_RXQUE_syntax_msg;
    }


#if defined( HAVE_CATOPEN )
    /* Open the message catalog via environment variable NLSPATH ----------- */
    if ((catd = catopen(REXXMESSAGEFILE, 0)) == (nl_catd)CATD_ERR)
    {
        sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
        if ((catd = catopen(DataArea, 0)) == (nl_catd)CATD_ERR)
        {
            printf("\nCannot open REXX message catalog %s.  Not in NLSPATH or %s.\n",
                   REXXMESSAGEFILE, ORX_CATDIR);
        }
    }
    /* retrieve message from repository        */
    pszMessage = catgets(catd, set_num, MsgNumber, NULL);

    if (!pszMessage)
    {
        sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
        if ((catd = catopen(DataArea, 0)) == (nl_catd)CATD_ERR)
        {
            sprintf(DataArea, "\nCannot open REXX message catalog %s.  Not in NLSPATH or %s.\n",
                    REXXMESSAGEFILE, ORX_CATDIR);
        }
        else
        {
            pszMessage = catgets(catd, set_num, MsgNumber, NULL);
            if (!pszMessage)                    /* got a message ?                */
            {
                strcpy(DataArea,"Error message not found!");
            }
            else
            {
                strcpy(DataArea, pszMessage);
            }
        }
    }
    else
    {
        /* search %1 and replace it with %s for message insertion       */
        strncpy(DataArea, pszMessage, MSG_BUF_SIZE -1);
    }
    catclose(catd);                     /* close the catalog                 */
#else
    sprintf(DataArea,"*** Cannot get description for error!");
#endif
    /* now do the parameter substitutions in the message template... */
    pInsert = strstr(DataArea, "%1");
    if (pInsert)
    {
        pInsert++; /* advance to 1 of %1 */
        *pInsert = 's';
        sprintf(achIMessage,DataArea,quename);
        pszMessage = achIMessage;
    }
    else
    {
        pszMessage = &DataArea[0];
    }

    printf("\nREX%d: %s\n", MsgNumber, pszMessage); /* print the msg         */

    exit(type);
}


/*********************************************************************/
/* Function:           Read a line from stdin into a buffer          */
/*                                                                   */
/* Description:        Read a line from stdin using DosRead into     */
/*                     the supplied buffer.  If the line is longer   */
/*                     than the buffer, then it will be truncated    */
/*                     and the remainder of the line thrown away.    */
/*                                                                   */
/* Inputs:             Buffer and size of buffer.                    */
/*                                                                   */
/* Outputs:            Success/failure flage,                        */
/*                     plus number of bytes read.                    */
/*                                                                   */
/* Side effects:       Logical line read from stdin.                 */
/*                                                                   */
/*********************************************************************/

size_t      get_line( char  *buffer,   /* Read buffer                */
                      size_t bufsize,  /* Buffer size                */
                      size_t *linelen) /* length of line             */
{
    static char savechar = '\0';         /* cached character           */
    static bool eof = false;             /* not hit eof yet            */
    size_t actual;                       /* actual bytes read          */
    char  newchar;                       /* character read             */
    size_t length;                       /* length read                */

    if (eof)                             /* already hit end?           */
    {
        return true;                       /* all done                   */
    }

    length = 0;                          /* nothing read yet           */
    if (savechar)
    {                      /* have a saved character     */
        *buffer++ = savechar;              /* save this                  */
        length++;                          /* add to count               */
        savechar = '\0';                   /* zap for next time          */
    }
    /* read first character       */
    actual =  fread(&newchar, 1, 1, stdin);
    while (!ferror(stdin))
    {             /* while no error             */
        if (!actual)
        {                     /* EOF?                       */
            *linelen = length;               /* set length                 */
            if (!length)                     /* nothing read?              */
            {
                return true;                   /* raise end of file          */
            }
            else
            {
                eof = true;                    /* quick out next time        */
                return false;                  /* have real line here        */
            }
        }
        if (newchar == '\r')
        {             /* end of line                */
            *linelen = length;               /* passback length read       */
                                             /* read next character        */
            actual = fread(&newchar, 1, 1, stdin);
            /* newline char?              */
            if (!ferror(stdin) && actual && newchar != '\n')
            {
                savechar = newchar;            /* save this for next time    */
            }
            return false;                    /* should be ok this time     */
        }
        else if (newchar == '\n')
        {        /* end of line                */
            *linelen = length;               /* passback length read       */
            return false;                    /* should be ok this time     */
        }
        else if (newchar == 0x1a)
        {        /* EOF character?             */
            *linelen = length;               /* give length                */
            eof = true;                      /* set flag for next time     */
            if (length)                      /* if something read          */
            {
                return true;                   /* this is EOF now            */
            }
            else
            {
                return false;               /* no error yet               */
            }
        }
        else
        {                             /* real character             */
            if (length < bufsize)
            {          /* room for this?             */
                length++;                      /* remember it                */
                *buffer++ = newchar;           /* copy over character        */
            }
        }
        /* read next character        */
        actual = fread(&newchar, 1, 1, stdin);
    }
    /* had an error               */
    if (length)
    {                        /* something read?            */
        *linelen = length;                 /* return this                */
        eof = true;                        /* can't read more            */
        return false;                      /* but no error yet           */
    }
    else
    {
        return true;                       /* treat this as an EOF       */
    }
}


