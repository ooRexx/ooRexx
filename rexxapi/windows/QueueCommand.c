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

#include <stdio.h>             /* needed for screen output           */
#include <conio.h>             /* needed for input                   */
#include <stdlib.h>            /* needed for miscellaneous functions */
#include <string.h>            /* needed for string functions        */
#include "RexxAPIManager.h"    /* needed for queue functions & codes */
                               /* used for queue name                */
#include "RexxErrorCodes.h"    /* generated file containing message numbers */

#define RXQUEUE_CLEAR    -2    /* used for queue mode CLEAR flag     */
#define BAD_MESSAGE      -6    /* Exit RC for message not found.     */

// defines necessary for running OS2 code under Windows
#define _stricmp(a,b) _stricmp(a,b)
#define DATETIME SYSTEMTIME
#define ENVBUFSIZE 256

#define DLLNAME "rexx.dll"

HANDLE hStdIn;

char  line[4096];              /* buffer for data to add to queue    */
char  work[256];               /* buffer for queue name, if default  */
LONG  queuemode=-1;            /* mode for access to queue           */

void  options_error(           /* function called on errors          */
          int   type,
          const char *queuename ) ;

                               /* function to read stdin             */
size_t get_line(char *, size_t, size_t *);


int __cdecl main(
  int   argc,
  PSZ   argv[] )
{
  LONG      i;                 /* loop counter for arguments         */
  LONG      rc;                /* return code from API calls         */
  size_t    entries;           /* number of entries in queue         */
  DATETIME  dt;                /* date/time structure for reading    */
  const char *quename=NULL;    /* initialize queuename to NULL       */
  size_t    linelen ;          /* input line length                  */
  CONSTRXSTRING  queuedata;    /* data added to the queue            */
  PSZ       t;                 /* argument pointer                   */
  DWORD     envvalue;
  char      envbuffer[ENVBUFSIZE+1];
  BOOL      Need2Detach = FALSE;

  hStdIn = GetStdHandle(STD_INPUT_HANDLE);

/*********************************************************************/
/*  Initialize string buffers to empty strings:                      */
/*********************************************************************/

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
      if( !_stricmp(t,"/FIFO") && /* if FIFO flag and               */
          queuemode==-1)       /*   no queuemode selected yet, then  */
        queuemode=RXQUEUE_FIFO;/*   set queuemode to FIFO, otherwise */
      else if( !_stricmp(t,"/LIFO") &&  /* if LIFO flag and         */
               queuemode==-1)  /*   no queuemode selected yet, then  */
        queuemode=RXQUEUE_LIFO;/*   set queuemode to LIFO, otherwise */
      else if( !_stricmp(t,"/CLEAR") &&  /* if CLEAR flag and         */
               queuemode==-1)  /*   no queuemode selected yet, then  */
        queuemode=RXQUEUE_CLEAR;/*   set queue for CLEAR, otherwise  */
      else options_error(      /*   there was an error in invokation */
                  0,
                  quename ) ;
    }
    else if(!quename)          /* if not option and no queue name    */
      quename=t;               /*   yet, then assign name            */
    else options_error(        /* otherwise illegal parameter        */
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
  {
    /* RXQUEUE.EXE was probably called as a host command and the PID */
    /* is then stored in RXQUEUESESSION, but first check if RXQUEUE  */
    /* is set.                                                       */
    envvalue = GetEnvironmentVariable("RXQUEUE", (LPTSTR) envbuffer, ENVBUFSIZE);
    if (!envvalue)
   {
      quename = "SESSION";     /* No name -> this is a session queue */
      RxInterProcessInit(TRUE); /* Initialize communication and get  */
                                /* or create SESSION queue. If PID   */
                                /* is stored  in RXQUEUESESSION,     */
                                /* RxInterProcessInit doesn't create */
                                /* create a new queue but increases  */
                                /* the session count of the queue.   */
      Need2Detach = TRUE; /* a new  queue might have been            */
                          /* created so we need to remove it later.  */
                          /* If no queue was created, the session    */
                          /* count was increased and therefore must  */
                          /* be decreased later (in RxQueueDetach).  */
   }
   else
   {
      RxInterProcessInit(FALSE);  /* use the specified queue name */
     quename = (PSZ)&envbuffer;
   }
  } else
      RxInterProcessInit(FALSE);  /* use the supplied queue name */

/*********************************************************************/
/*  Call RxQueueQuery() to check for the existence of the queue:     */
/*********************************************************************/

  if (rc=RexxQueryQueue(quename,/* search for existence of 'quename' */
                      &entries ))/*--> get number of entries in queue*/
    options_error( rc,         /* generate error if API fails        */
                   quename ) ;

/*********************************************************************/
/*  Get all input data and write each line to the proper queue       */
/*  (not CLEAR):                                                     */
/*********************************************************************/

  if(queuemode != RXQUEUE_CLEAR) {     /* if not CLEAR operation...  */
    while(!get_line( line,             /* while more data passed in: */
                     sizeof( line ),
                     &linelen)) {
                                       /* express in RXSTRING form   */
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
    RXSTRING pulldata;

    MAKERXSTRING(pulldata,NULL,0);    /* make an empty RXSTRING     */
    rc = RexxPullQueue(quename, &pulldata, &dt, RXQUEUE_NOWAIT);
    while (!rc) {
      if (pulldata.strlength && pulldata.strptr)
      {
          RexxFreeMemory(pulldata.strptr);  /* GlobalFree instead of free */
      }
      MAKERXSTRING(pulldata,NULL,0);  /* make a new empty RXSTRING  */
      rc = RexxPullQueue(quename, &pulldata, &dt, RXQUEUE_NOWAIT);
    }
  }
  if (Need2Detach)    /* only remove queue when queue was            */
    RxQueueDetach(0); /* created by RXQUEUE.EXE or decrease session  */
                      /* count if it already existed                 */
  return(0);
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
                    const char *quename ) /* Name of offending queue.   */
{
  LONG   rc = 0 ;                      /* Exit return code.          */
  char   DataArea[ 256 ] ;             /* Message buffer.            */
  char   DataArea2[ 256 ] ;            /* Message buffer.            */
  ULONG  MsgNumber ;                   /* Message number.            */
  HINSTANCE hDll=NULL;

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
  /* Begin assign message numbers to error codes */
  switch (type)
  {
    case 0: /* invocation error */
      MsgNumber = Error_RXQUE_syntax;
      break;

    case RXQUEUE_NOTINIT:
      MsgNumber = Error_RXQUE_notinit;
      break;

    case RXQUEUE_SIZE:
      MsgNumber = Error_RXQUE_size;
      break;

    case RXQUEUE_BADQNAME:
      MsgNumber = Error_RXQUE_name;
      break;

    case RXQUEUE_PRIORITY:
      MsgNumber = Error_RXQUE_access;
      break;

    case RXQUEUE_NOTREG:
      MsgNumber = Error_RXQUE_exist;
      break;

    case RXQUEUE_MEMFAIL:
      MsgNumber = Error_RXQUE_memfail;
      break;

    default:
      MsgNumber = Error_RXQUE_syntax;
  } /* endswitch */

  hDll = LoadLibrary(DLLNAME);

  if (hDll)

  {
    if (LoadString(hDll, MsgNumber, DataArea, sizeof(DataArea)))
    {
      /* check for messages with inserts */
      if ((MsgNumber == Error_RXQUE_name) || (MsgNumber == Error_RXQUE_exist))
      {
        char *pInsert = NULL;

        /* search %1 and replace it with %s for message insertion */
        strcpy(DataArea2, DataArea);
        pInsert = strstr(DataArea2, "%1");
        if (pInsert)
        {
          pInsert++; /* advance to 1 of %1 */
          *pInsert = 's';
          sprintf(DataArea, DataArea2, quename);
        }
      }
    }
     else
       strcpy(DataArea,"Error, but no error message available.");
  }
  else
    strcpy(DataArea,"Error, but no error message available because REXX.DLL not loaded.");


  /******************************************************************/
  /* Look up the message text.  If we can find it, print the        */
  /* message and return 0 when we exit.  Otherwise, do not print    */
  /* and return an error code when we exit.                         */
  /******************************************************************/

  #undef printf
  printf("REX%d: %s", MsgNumber, DataArea);
  FreeLibrary(hDll);

  exit(type);
}

/*********************************************************************/
/* Function:           Read a line from stdin into a buffer          */
/*                                                                   */
/* Description:        Read a line from stdin using ReadFile into    */
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

size_t get_line(char *buffer,          /* Read buffer                */
                      size_t bufsize,  /* Buffer size                */
                      size_t *linelen) /* length of line             */
{
  static char savechar = '\0';         /* cached character           */
  static BOOL eof = FALSE;             /* not hit eof yet            */
  ULONG  actual;                       /* actual bytes read          */
  ULONG rc;                            /* ReadFile return code       */
  char  newchar;                       /* character read             */
  size_t length;                       /* length read                */

  if (eof)                             /* already hit end?           */
    return TRUE;                       /* all done                   */

  length = 0;                          /* nothing read yet           */
  if (savechar) {                      /* have a saved character     */
    *buffer++ = savechar;              /* save this                  */
    length++;                          /* add to count               */
    savechar = '\0';                   /* zap for next time          */
  }
                                       /* read first character       */
  rc = !ReadFile(hStdIn, &newchar, 1, &actual, NULL);
  while (!rc) {                        /* while no error             */
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
      rc = !ReadFile(hStdIn, &newchar, 1, &actual, NULL);
                                       /* newline char?              */
      if (!rc && actual && newchar != '\n')
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
    rc = !ReadFile(hStdIn, &newchar, 1, &actual, NULL);
  }
                                       /* had an error               */
  if (actual) {                        /* something read?            */
    *linelen = actual;                 /* return this                */
    eof = TRUE;                        /* can't read more            */
    return FALSE;                      /* but no error yet           */
  }
  else
    return TRUE;                       /* treat this as an EOF       */
}
