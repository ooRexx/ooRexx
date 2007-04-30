/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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

#define INCL_RXQUEUE           /* needed for RxQueue...() calls      */

#if defined( HAVE_FEATURES_H )
# include <features.h>
#endif

#if defined( HAVE_NL_TYPES_H )
# include <nl_types.h>
#endif

#include "PlatformDefinitions.h"
#include <limits.h>
#include <stdio.h>             /* needed for screen output           */
#include <stdlib.h>            /* needed for miscellaneous functions */
#include <string.h>            /* needed for string functions        */
#include "RexxAPIManager.h"
#include "rexx.h"              /* needed for queue functions & codes */
#include "RexxMessageNumbers.h"

#define RXQUEUE_CLEAR    -2    /* used for queue mode CLEAR flag     */
#define BAD_MESSAGE      -6    /* Exit RC for message not found.     */

#define MSG_BUF_SIZE    256    /* Error message buffer size          */

#define CCHMAXPATH PATH_MAX+1
#ifdef LINUX                   /*  AIX already defined               */
#define CATD_ERR -1
#endif

extern BOOL CALL_BY_RXQUEUE;

CHAR  line[4096];              /* buffer for data to add to queue    */
CHAR  work[256];               /* buffer for queue name, if default  */
LONG  queuemode=-1;            /* mode for access to queue           */

VOID  options_error(           /* function called on errors          */
          LONG  type,
          PSZ   queuename ) ;

                               /* function to read stdin             */
ULONG get_line(PCHAR, ULONG, PULONG);


int main(
  int   argc,
  PSZ   argv[] )
{
  LONG      i;                 /* loop counter for arguments         */
  LONG      rc;                /* return code from API calls         */
  ULONG     entries;           /* number of entries in queue         */
  DATETIME  dt;                /* date/time structure for reading    */
  PSZ       quename=NULL;      /* initialize queuename to NULL       */
  ULONG     linelen ;          /* input line length                  */
  RXSTRING  queuedata;         /* data added to the queue            */
  PSZ       t;                 /* argument pointer                   */


/*$PE*/
/*********************************************************************/
/*  Initialize string buffers to empty strings:                      */
/*********************************************************************/

  CALL_BY_RXQUEUE = TRUE;

  memset(line, '\0', sizeof(line)); /* clear buffer 'line' -for data */
  memset(work, '\0', sizeof(work)); /* clear buffer 'work' -for      */
                                    /*   queuename                   */


/*********************************************************************/
/*  Interpret options from invocation and set appropriate values:    */
/*********************************************************************/

  for(i=1; i<argc; i++) {      /* go through the argument list...    */
    t = argv[i];               /* point to next argument             */
    if((t[0]=='/') || (t[0]=='-')) {/*if options character in        */
      t[0]='/';                /* argument,  then make character '/' */
      if( !rxstricmp(t,"/FIFO") && /* if FIFO flag and               */
          queuemode==-1)       /*   no queuemode selected yet, then  */
        queuemode=RXQUEUE_FIFO;/*   set queuemode to FIFO, otherwise */
      else if( !rxstricmp(t,"/LIFO") &&  /* if LIFO flag and         */
               queuemode==-1)  /*   no queuemode selected yet, then  */
        queuemode=RXQUEUE_LIFO;/*   set queuemode to LIFO, otherwise */
      else if( !rxstricmp(t,"/CLEAR") &&  /* if CLEAR flag and       */
               queuemode==-1)  /*   no queuemode selected yet, then  */
        queuemode=RXQUEUE_CLEAR;/*   set queue for CLEAR, otherwise  */
      else
            options_error(      /*  there was an error in invokation */
                  0,
                  quename ) ;
    }
    else if(!quename)          /* if not option and no queue name    */
      quename=t;               /*   yet, then assign name            */
    else
          options_error(       /* otherwise illegal parameter        */
                  0,
                  quename ) ;
  }
  if(queuemode==-1)            /* if no queue mode requested, then   */
    queuemode=RXQUEUE_FIFO;    /*   use default value of FIFO        */


/*********************************************************************/
/*  Make sure there is a queue name before API calls:                */
/*********************************************************************/

  if(!quename)                 /* if there is no queue specified:    */
                               /* scan current environment           */
    if (!(quename = getenv("RXQUEUE")) || !quename)    /*THU008M */
      quename = "SESSION";     /* use session if not found           */

/*$PE*/
/*********************************************************************/
/*  Call RxQueueQuery() to check for the existence of the queue:     */
/*********************************************************************/

  if (rc=RexxQueryQueue(quename,/* search for existence of 'quename' */
                      &entries ))/*ÀŽ> get number of entries in queue*/
    options_error( rc,         /* generate error if API fails        */
                   quename );

/*********************************************************************/
/*  Get all input data and write each line to the proper queue       */
/*  (not CLEAR):                                                     */
/*********************************************************************/

  if(queuemode != RXQUEUE_CLEAR) {     /* if not CLEAR operation...  */
    while(!get_line( line,             /* while more data passed in: */
                     sizeof( line ),
                     &linelen))
    {                                  /* express in RXSTRING form   */
      MAKERXSTRING(queuedata, line, linelen);
      if(rc=RexxAddQueue(              /*   write info to the queue  */
                        quename,       /*     queue to write into    */
                        &queuedata,    /*information to add to queue */
                        queuemode))    /* size of information to add */
                                       /* FIFO || LIFO mode          */
        options_error( rc,             /* generate error if API fails*/
                       quename ) ;
    }
  }
  else {
    MAKERXSTRING(queuedata,NULL,0);    /* make an empty RXSTRING     */
    rc = RexxPullQueue(quename, &queuedata, &dt, RXQUEUE_NOWAIT);
    while (!rc) {
      if (queuedata.strlength)
        free(queuedata.strptr);
      MAKERXSTRING(queuedata,NULL,0);  /* make a new empty RXSTRING  */
      rc = RexxPullQueue(quename, &queuedata, &dt, RXQUEUE_NOWAIT);
    }
  }
  exit(0);
}


/*********************************************************************/
/*                             End Of Main Program                   */
/*********************************************************************/
/*$PE*/
/*$FD.options_error()*/
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

VOID options_error( LONG  type,        /* Error type.                */
                    PSZ   quename )    /* Name of offending queue.   */
{
  LONG     rc = 0;                     /* Exit return code.          */
  char     DataArea[ MSG_BUF_SIZE ];   /* Message buffer.            */
  char     achIMessage[2*MSG_BUF_SIZE];/* Message with insertion.    */
  ULONG    MsgNumber;                  /* Message number.            */
  ULONG    MsgLength;                  /* Length of returned message */
#if defined( HAVE_NL_TYPES_H )
  nl_catd  catd;                       /* catalog descriptor         */
#endif
  int      set_num = 1;                /* message set 1 from catalog */
  PSZ      pszMessage;                 /* message pointer            */
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

    case RXQUEUE_SIZE:
      MsgNumber = Error_RXQUE_size_msg;
      break;

    case RXQUEUE_NOEMEM:
      MsgNumber = Error_RXQUE_nomem_msg;
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

    case RXQUEUE_MEMFAIL:
      MsgNumber = Error_RXQUE_memfail_msg;
      break;

    default:
      MsgNumber = Error_RXQUE_syntax_msg;
  } /* endswitch */


#if defined( HAVE_CATOPEN )
  /* Open the message catalog via environment variable NLSPATH ----------- */
  if ((catd = catopen(REXXMESSAGEFILE, 0)) == (nl_catd)CATD_ERR)
  {
     sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
     if ((catd = catopen(DataArea, 0)) == (nl_catd)CATD_ERR)
        printf("\nCannot open REXX message catalog %s.\nNot in NLSPATH or %s.\n",
               REXXMESSAGEFILE, ORX_CATDIR);
  }
                                /* retrieve message from repository        */
  pszMessage = catgets(catd, set_num, MsgNumber, NULL);

  if (!pszMessage)
  {
     sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
     if ((catd = catopen(DataArea, 0)) == (nl_catd)CATD_ERR)
     {
        sprintf(DataArea, "\nCannot open REXX message catalog %s.\nNot in NLSPATH or %s.\n",
                          REXXMESSAGEFILE, ORX_CATDIR);
     }
     else
     {
         pszMessage = catgets(catd, set_num, MsgNumber, NULL);
         if(!pszMessage)                    /* got a message ?                */
           strcpy(DataArea,"Error message not found!");
        else
           strcpy(DataArea, pszMessage);
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
     pszMessage = &DataArea[0];

  printf("\nREX%d: %s\n", MsgNumber, pszMessage); /* print the msg         */

  exit(type);
}
/*$PE*/
/*$FD.get_line()*/
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

ULONG       get_line( PCHAR buffer,   /* Read buffer                */
                      ULONG  bufsize,  /* Buffer size                */
                      PULONG linelen)  /* length of line             */
{
  static CHAR savechar = '\0';         /* cached character           */
  static BOOL eof = FALSE;             /* not hit eof yet            */
  ULONG actual;                        /* actual bytes read          */
  ULONG rc;                            /* DosRead return code        */
  CHAR  newchar;                       /* character read             */
  ULONG length;                        /* length read                */

  if (eof)                             /* already hit end?           */
    return TRUE;                       /* all done                   */

  length = 0;                          /* nothing read yet           */
  if (savechar) {                      /* have a saved character     */
    *buffer++ = savechar;              /* save this                  */
    length++;                          /* add to count               */
    savechar = '\0';                   /* zap for next time          */
  }
                                       /* read first character       */
  actual =  fread(&newchar, 1, 1, stdin);
  while (!ferror(stdin)) {             /* while no error             */
    if (!actual) {                     /* EOF?                       */
      *linelen = length;               /* set length                 */
      if (!length)                     /* nothing read?              */
        return TRUE;                   /* raise end of file          */
      else {
        eof = TRUE;                    /* quick out next time        */
        return FALSE;                  /* have real line here        */
      }
    }
    if (newchar == '\r') {             /* end of line                */
      *linelen = length;               /* passback length read       */
                                       /* read next character        */
      actual = fread(&newchar, 1, 1, stdin);
                                       /* newline char?              */
      if (!ferror(stdin) && actual && newchar != '\n')
        savechar = newchar;            /* save this for next time    */
      return FALSE;                    /* should be ok this time     */
    }
    else if (newchar == '\n') {        /* end of line                */
      *linelen = length;               /* passback length read       */
      return FALSE;                    /* should be ok this time     */
    }
    else if (newchar == 0x1a) {        /* EOF character?             */
      *linelen = length;               /* give length                */
      eof = TRUE;                      /* set flag for next time     */
      if (length)                      /* if something read          */
        return TRUE;                   /* this is EOF now            */
      else return FALSE;               /* no error yet               */
    }
    else {                             /* real character             */
      if (length < bufsize) {          /* room for this?             */
        length++;                      /* remember it                */
        *buffer++ = newchar;           /* copy over character        */
      }
    }
                                       /* read next character        */
    actual = fread(&newchar, 1, 1, stdin);
  }
                                       /* had an error               */
  if (length) {                        /* something read?            */
    *linelen = length;                 /* return this                */
    eof = TRUE;                        /* can't read more            */
    return FALSE;                      /* but no error yet           */
  }
  else
    return TRUE;                       /* treat this as an EOF       */
}


