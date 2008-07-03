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
#define CCHMAXPATH PATH_MAX+1
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

RexxString * SysMessageText(           /* simplified whole code             */
    wholenumber_t code )               /* message code to extract           */
/****************************************************************************/
/* Function:  Return a message from the message catalog, including header.  */
/****************************************************************************/
{
#if defined( HAVE_NL_TYPES_H )
 nl_catd        catd;                  /* catalog descriptor from catopen() */
#endif
 int            set_num = 1;           /* message set 1 from catalog        */
 ERROR_MESSAGE *p;                     /* message table scan pointer        */
 int            msgid;                 /* message number                    */
 char           DataArea[256];         /* buf to return message             */
 char          *message;
                                       /* loop through looking for the      */
                                       /* error code                        */
#if defined( HAVE_CATOPEN )
 for (p = Message_table; p->code != 0; p++) {
   if (p->code == code) {              /* found the target code?            */

     msgid = p->msgid;                 /* get msg number associated w/ error*/
                                       /* open message catalog in NLSPATH   */
     if ((catd = catopen(REXXMESSAGEFILE, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
     {
       sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
       if ((catd = catopen(DataArea, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
       {
         sprintf(DataArea, "\nCannot open REXX message catalog %s.\nNot in NLSPATH or %s.\n",
                           REXXMESSAGEFILE, ORX_CATDIR);
         return new_string((char *)&DataArea, strlen(DataArea));
       }
     }                                   /* retrieve message from repository  */
     message = catgets(catd, set_num, msgid, NULL);
     if(!message)                    /* got a message ?                   */
     {
# if defined(OPSYS_LINUX) && !defined(OPSYS_SUN)
       sprintf(DataArea, "%s/%s", ORX_CATDIR, REXXMESSAGEFILE);
       if ((catd = catopen(DataArea, SECOND_PARAMETER)) == (nl_catd)CATD_ERR)
       {
         sprintf(DataArea, "\nCannot open REXX message catalog %s.\nNot in NLSPATH or %s.\n",
                           REXXMESSAGEFILE, ORX_CATDIR);
         return new_string((char *)&DataArea, strlen(DataArea));
       }
       else
       {
         message = catgets(catd, set_num, msgid, NULL);
         if(!message)                    /* got a message ?                   */
           strcpy(DataArea,"Error message not found!");
         else
           strcpy(DataArea, message);
       }
# else
       strcpy(DataArea,"Error message not found!");
# endif
     }
     else
       strcpy(DataArea, message);
     catclose(catd);                 /* close the catalog                 */
                                     /* convert and return the message    */
     return new_string((char *)&DataArea, strlen(DataArea));
   }
 }
 return OREF_NULL;                     /* no message retrieved              */
#else
 sprintf(DataArea,"Cannot get description for error %d",msgid);
 return new_string((char *)&DataArea, strlen(DataArea));
#endif
}


RexxString * SysMessageHeader(
    wholenumber_t code )               /* message code to extract           */
/****************************************************************************/
/* Function:  Return a message header derived from the in-store error table */
/****************************************************************************/
{
 ERROR_MESSAGE *p;                     /* table scan pointer                */
 int            msgid;                 /* message number                    */
 char           DataArea[20];          /* buf addr to return message        */
                                       /* loop through looking for the      */
                                       /* error code                        */
 for (p = Message_table; p->code != 0; p++) {
   if (p->code == code) {              /* found the target code?            */
     msgid = p->msgid;                 /* get msg number associated w/ error*/
                                       /* format as a message header        */
     sprintf(DataArea, "REX%4.4dE: ", msgid);
     return new_string(DataArea);     /* return as a string object         */
   }
 }
 return OREF_NULL;                     /* no message retrieved              */
}




/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   SysRestoreEnvironment                        */
/*                                                                   */
/*   Descriptive Name:  restores environment saved by Setlocal()     */
/*                                                                   */
/*   Function:          restores the environment variables, current  */
/*                      directory and drive.                         */
/*                                                                   */
/*********************************************************************/

void SystemInterpreter::restoreEnvironment(
  void *CurrentEnv)                    /* saved environment          */
{
  char *current;                       /* ptr to saved environment   */
  unsigned long size;                  /* size of the saved space    */
  unsigned long length;                /* string length              */
  char *begin;                         /* begin of saved space       */
  char **Environment;                  /* environment pointer        */

  char *  del = NULL;                   /* ptr to old unused memory   */
  char * Env_Var_String;                /* enviornment entry          */
  char   namebufsave[256],namebufcurr[256];
  char *  np;
  int i;

  Environment = environ;               /* get the current environment*/

  begin = current = (char *)CurrentEnv; /* get the saved space        */
  size = ((ENVENTRY*)current)->size;   /* first read out the size    */
  current += 4;                        /* update the pointer         */
  if(chdir(current) == -1)             /* restore the curr dir       */
      reportException(Error_System_service_service, (const stringchar_t *)"ERROR CHANGING DIRECTORY");
  current += strlen(current);          /* update the pointer         */
  current++;                           /* jump over '\0'             */
  if(!putflag){                        /* first change in the        */
                                       /* environment ?              */
    /* copy all entries to dynamic memory                            */
                                       /*for all entries in the env  */
    for(;*Environment != NULL;Environment++){
      length = strlen(*Environment)+1; /* get the size of the string */
                                       /* and alloc space for it     */
      Env_Var_String = (char *)malloc(length);
      memcpy(Env_Var_String,*Environment,length);/* copy the string  */
      putenv(Env_Var_String);          /* and make it part of env    */
    }
    putflag = 1;                       /* prevent do it again        */
  }
                                       /* Loop through the saved env */
                                       /* entries and restore them   */
  for(;((unsigned long)(current-begin))<size;current+=(strlen(current)+1)){
    Environment = environ;             /* get the environment        */
    del = NULL;
    np = current;
                                       /* extract the the name       */
                                       /* from the saved enviroment  */
    for(i=0;(*np!='=')&&(i<255);np++,i++){
      memcpy(&(namebufsave[i]),np,1);  /* copy the character         */
    }
    memcpy(&(namebufsave[i]),"\0",1);  /* copy the terminator        */
                                       /* find the entry in the env  */
    for(;*Environment != NULL;Environment++){
      np = *Environment;
                                       /* extract the the name       */
                                       /* from the current env       */
      for(i=0;(*np!='=')&&(i<255);np++,i++){
        memcpy(&(namebufcurr[i]),np,1);/* copy the character         */
      }
      memcpy(&(namebufcurr[i]),"\0",1);/* copy the terminator        */

      if(!strcmp(namebufsave,namebufcurr)){/* have a match ?         */
        del = *Environment;            /* remember it for deletion   */
        break;                         /* found, so get out of here  */
      }
    }
    if(putenv(current) == -1)
      reportException(Error_System_service_service, (const stringchar_t *)"ERROR RESTORING ENVIRONMENT VARIABLE");
    if(del)                            /* if there was an old entry  */
      free(del);                       /* free it                    */
  }
}


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   BuildEnvlist                                 */
/*                                                                   */
/*   Descriptive Name:  Build saved environment block                */
/*                                                                   */
/*   Function:          Builds a block containing all of the         */
/*                      environment variables, the current drive     */
/*                      and the current directory.                   */
/*                                                                   */
/*********************************************************************/
RexxObject * SystemInterpreter::buildEnvlist()
{
  RexxObject    *newBuffer;            /* Buffer object to hold env  */
  char          **Environment;         /* environment pointer        */
  size_t        size = 0;              /* size of the new buffer     */
  char          *curr_dir;             /* current directory          */
  char          *New;                  /* starting address of buffer */

  Environment = environ;               /* get the ptr to the environ */

  for(;*Environment != NULL;Environment++){
    size += strlen(*Environment);      /* calculate the size for all */
    size++;                            /* environment variables+'\0' */
  }                                    /* now get current dir        */
  if(!size)
    return NULLOBJECT;                 /* this worked ok                    */
  if (!(curr_dir=(char *)malloc(CCHMAXPATH+2)))/* malloc storage for cwd*/
    reportException(Error_System_service);

  if (!getcwd(curr_dir,CCHMAXPATH))    /* get current directory      */
  {
     strncpy( achRexxCurDir, getenv("PWD"), CCHMAXPATH);
     achRexxCurDir[CCHMAXPATH - 1] = '\0';
     if (achRexxCurDir[0] != '/' )
       reportException(Error_System_service);/* Complain if it fails*/
  }
  size += strlen(curr_dir);            /* add the space for curr dir */
  size++;                              /* and its terminating '\0'   */
  size += 4;                           /* this is for the size itself*/
                                       /* Now we have the size for   */
                                       /* allocating the new buffer  */
  newBuffer = (RexxObject *)new_buffer(NULL, size);  /* let's do it     */
                                       /* Get starting address of buf*/
  New = (char*)newBuffer;
  ((ENVENTRY*)New)->size = size;       /* first write the size       */
  New +=4;                             /* update the pointer         */
                                       /* now write the curr dir     */
  memcpy(New,curr_dir,strlen(curr_dir));
  New += strlen(curr_dir);             /* update the pointer         */
  memcpy(New,"\0",1);                  /* write the terminator       */
  New++;                               /* update the pointer         */
  Environment = environ;               /* reset to begin of environ  */
                                       /* Loop through environment   */
                                       /* and copy all entries to the*/
                                       /* buffer, each terminating   */
                                       /* with '\0'                  */
  for(;*Environment != NULL;Environment++){
                                       /* copy the entry             */
    memcpy(New,*Environment,strlen(*Environment));
    New += strlen(*Environment);       /* update the pointer         */
    memcpy(New,"\0",1);                /* write the terminator       */
    New++;                             /* update the pointer         */
  }
  free(curr_dir);                      /* free curr dir buffer       */
  return newBuffer;                    /* return the pointer         */
}


/**
 * Push a new environment for the SysSetLocal() BIF.
 *
 * @param context The current activation context.
 *
 * @return Returns TRUE if the environment was successfully pushed.
 */
RexxObject *SystemInterpreter::pushEnvironment(RexxActivation *context)
{
    RexxObject     *Current;             /* new save block                    */

    Current = BuildEnvlist();            /* build the new save block          */
    if (NULLOBJECT == Current)           /* if unsuccessful return zero       */
      return TheFalseObject;
    else {
                                         /* Have Native Actiovation           */
      context->pushEnvironment(Current);          /*  update environemnt list          */
      return TheTrueObject;              /* this returns one                  */
    }
}

/**
 * Pop an environment for the SysEndLocal() BIF.
 *
 * @param context The current activation context.
 *
 * @return Always returns FALSE.  This is a NOP on Windows.
 */
RexxObject *SystemInterpreter::popEnvironment(RexxActivation *context)
{
        RexxObject    *Current;          /* new save block                    */

    Current =  context->popEnvironment();/*  block, if ixisted.               */
    if (TheNilObject == Current)         /* nothing saved?                    */
      return TheFalseObject;             /* return failure value              */
    else {
                                         /* restore everything                */
      restoreEnvironment(Current);
      return TheTrueObject;              /* this worked ok                    */
    }
}



