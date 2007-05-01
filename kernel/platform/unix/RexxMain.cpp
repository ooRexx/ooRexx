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
/* REXX AIX Support                                            aixmain.c      */
/*                                                                            */
/* Main interface to the REXX interpreter                                     */
/*                                                                            */
/******************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#if defined( HAVE_SCHED_H )
# include <sched.h>
#endif

#define INCL_REXXSAA
#define INCL_RXMACRO

#include "RexxCore.h"                    /* bring in global defines           */
#include "StringClass.hpp"
#include "RexxBuffer.hpp"
#include "MethodClass.hpp"
#include "RexxCode.hpp"
#include "ArrayClass.hpp"
#include "RexxNativeActivation.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "SourceFile.hpp"
#include "RexxNativeAPI.h"                      /* REXX interface/native method code */
#include SYSREXXSAA

#include "ActivityTable.hpp"
#include "RexxAPIManager.h"
#include "APIDefinitions.h"
#include "SubcommandAPI.h"
#include "APIUtilities.h"
       /* support macros                    */

#ifdef SOM
#include "som.xh"
#include "orxsom.h"
#endif


#define BUFFERLEN         256          /* Length of bufs used                 */
#define BUFFERLEN_OS      1024         /* Length of bufs used                 */
#define DEFAULT_PRECISION  9           /* default precision to use   */
#define COPYRIGHT "Copyright (c) IBM Corporation 1995, 2004.\nCopyright (c) RexxLA 2005-2007.\nAll Rights Reserved.\n" \
                  "This program and the accompanying materials \n" \
                  "are made available under the terms of the Common Public License v1.0\n" \
                  "which accompanies this distribution. \n" \
                  "http://www.oorexx.org/license.html \n\n"


BOOL rexxutil_call = FALSE;
RexxMutex rexxutil_call_sem;


extern SMTX initialize_sem = 0;
extern SEV   RexxTerminated;               /* Termination complete semaphore.     */
BOOL         bProcessExitInitFlag = FALSE;
extern int  SecureFlag = 0;
extern int  thread_counter = 0;


APIRET APIENTRY RexxExecuteMacroFunction ( PSZ, PRXSTRING );

#ifdef TIMESLICE                       /* System Yielding function prototype*/
APIRET REXXENTRY RexxSetYield(PID procid, TID threadid);
#endif /*timeslice*/

extern RexxObject *ProcessLocalServer; /* current local server              */
extern RexxActivity *CurrentActivity;  /* current active activity           */
PCHAR SysFileExtension(PCHAR);
RexxMethod *SysRestoreProgramBuffer(PRXSTRING, RexxString *);
void SysSaveProgramBuffer(PRXSTRING, RexxMethod *);
void SysSaveTranslatedProgram(PCHAR, RexxMethod *);
PCHAR SearchFileName(PCHAR, char);
extern ActivityTable * ProcessLocalActs;
extern BOOL RexxStartedByApplication;

extern "C" {
APIRET REXXENTRY RexxTranslateProgram( PSZ, PSZ);
//LONG APIENTRY RexxMain(LONG, PRXSTRING, PSZ, PRXSTRING, PSZ, LONG, PRXSYSEXIT, PSHORT, PRXSTRING);
}

typedef struct

RexxStartInfo {
  LONG       argcount;                 /* Number of args in arglist         */
  PRXSTRING  arglist;                  /* Array of args                     */
  PSZ        programname;              /* REXX program to run               */
  PRXSTRING  instore;                  /* Instore array                     */
  PSZ        envname;                  /* Initial cmd environment           */
  LONG       calltype;                 /* How the program is called         */
  PRXSYSEXIT exits;                    /* Array of system exit names        */
  PSHORT     retcode;                  /* Integer form of result            */
  PRXSTRING  result;                   /* Result returned from program      */
  PSZ        outputName;               /* compilation output file           */
  BOOL       translating;              /* performing a translation only     */
} RexxStartInfo;


/*********************************************************************/
/* Function SearchPrecision:  Check the precision of the activity and*/
/* use it if possible                                                */
/*********************************************************************/
extern "C" {
void SearchPrecision(
  PULONG    precision)                 /* required precision         */
{
  RexxActivity        *activity;
  RexxExpressionStack *activations;
  RexxActivation      *activation;
  int i, thread_id,threadid;

  *precision = DEFAULT_PRECISION;      /* set default digit count    */

/* give me the numeric digits settings of the current actitity       */

  threadid = (int) pthread_self();
  if (ProcessLocalActs != OREF_NULL)
  { /* activities created?               */
                                       /* get all activities                */
    for (i=ProcessLocalActs->first();ProcessLocalActs->available(i);i=ProcessLocalActs->next(i))
    {
      thread_id = ProcessLocalActs->index(i);   /* get thread id   */
      if (thread_id == threadid)
      {
        activity = (RexxActivity *)ProcessLocalActs->fastAt(thread_id);
        activation = activity->currentAct();
        *precision = activation->digits();
        break;
      }
    }
  }
}
}

/******************************************************************************/
/* Name:       RexxStart                    (internal start via ApiRexxStart) */
/*                                                                            */
/* Arguments:  argcount - Number of args in arglist                           */
/*             arglist - Array of args (array of RXSTRINGs)                   */
/*             programname - REXX program to run                              */
/*             instore - Instore array (array of 2 RXSTRINGs)                 */
/*             envname - Initial cmd environment                              */
/*             calltype - How the program is called                           */
/*             exits - Array of system exit names (array of RXSTRINGs)        */
/*                                                                            */
/* Returned:   result - Result returned from program                          */
/*             rc - Return code from program                                  */
/*                                                                            */
/* Notes:  Primary path into Object REXX.  Makes sure Object REXX is up       */
/*   and runs the requested program.                                          */
/*                                                                            */
/*   Mainline path looks like this:                                           */
/*     RexxStart => server_RexxStart                                          */
/******************************************************************************/
LONG APIENTRY RexxStart(
  LONG argcount,                       /* Number of args in arglist         */
  PRXSTRING arglist,                   /* Array of args                     */
  PSZ programname,                     /* REXX program to run               */
  PRXSTRING instore,                   /* Instore array                     */
  PSZ envname,                         /* Initial cmd environment           */
  LONG calltype,                       /* How the program is called         */
  PRXSYSEXIT exits,                    /* Array of system exit names        */
  PSHORT retcode,                      /* Integer form of result            */
  PRXSTRING result)                    /* Result returned from program      */

{

  LONG     rc;                         /* RexxStart return code             */
  RexxStartInfo RexxStartArguments;    /* entry argument information        */

                                       /* copy all of the arguments into    */
                                       /* the info control block, which is  */
                                       /* passed across the kernel boundary */
                                       /* into the real RexxStart method    */
  RexxStartArguments.argcount = argcount;
  RexxStartArguments.arglist = arglist;
  RexxStartArguments.programname = programname;
  RexxStartArguments.instore = instore;
  RexxStartArguments.envname = envname;
  RexxStartArguments.calltype = calltype;
  RexxStartArguments.exits = exits;
  RexxStartArguments.retcode = retcode;
  RexxStartArguments.result = result;
  RexxStartArguments.outputName = NULL;
                                       /* this is a real execution          */
  RexxStartArguments.translating = FALSE;
#ifdef SOM
  RexxSomInitialize((SOMObject *) NULL);  /* Perform any needed inits          */
#else
  if(!rexxutil_call){                  /* no init if called from a rexxutil */
    RexxInitialize();                  /* Perform any needed inits          */
  }
  else{
    rexxutil_call = FALSE;
    rexxutil_call_sem.release();
  }

#endif

                                       /* pass along to the real method     */
  rc = RexxSendMessage(ProcessLocalServer, CHAR_RUN_PROGRAM, NULL, "vp", NULL, &RexxStartArguments);
  RexxTerminate();                     /* perform needed termination        */
  return -rc;                          /* return the error code (negated)   */
}



//#ifdef AIX
/******************************************************************************/
/* Name:       ApiRexxStart                                  (like RexxStart) */
/*                                                                            */
/* Arguments:  argcount - Number of args in arglist                           */
/*             arglist - Array of args (array of RXSTRINGs)                   */
/*             programname - REXX program to run                              */
/*             instore - Instore array (array of 2 RXSTRINGs)                 */
/*             envname - Initial cmd environment                              */
/*             calltype - How the program is called                           */
/*             exits - Array of system exit names (array of RXSTRINGs)        */
/*                                                                            */
/* Returned:   result - Result returned from program                          */
/*             rc - Return code from program                                  */
/*                                                                            */
/* Notes:  Primary path into Object REXX.  Makes sure Object REXX is up       */
/*   and runs the requested program.                                          */
/*                                                                            */
/*   Mainline path looks like this:                                           */
/*     RexxStart => server_RexxStart                                          */
/******************************************************************************/
LONG APIENTRY ApiRexxStart(
  LONG argcount,                       /* Number of args in arglist         */
  PRXSTRING arglist,                   /* Array of args                     */
  PSZ programname,                     /* REXX program to run               */
  PRXSTRING instore,                   /* Instore array                     */
  PSZ envname,                         /* Initial cmd environment           */
  LONG calltype,                       /* How the program is called         */
  PRXSYSEXIT exits,                    /* Array of system exit names        */
  PSHORT retcode,                      /* Integer form of result            */
  PRXSTRING result)                    /* Result returned from program      */

{


  LONG     rc;                         /* RexxStart return code             */
  RexxStartInfo RexxStartArguments;    /* entry argument information        */

                                       /* copy all of the arguments into    */
                                       /* the info control block, which is  */
                                       /* passed across the kernel boundary */
                                       /* into the real RexxStart method    */
  RexxStartArguments.argcount = argcount;
  RexxStartArguments.arglist = arglist;
  RexxStartArguments.programname = programname;
  RexxStartArguments.instore = instore;
  RexxStartArguments.envname = envname;
  RexxStartArguments.calltype = calltype;
  RexxStartArguments.exits = exits;
  RexxStartArguments.retcode = retcode;
  RexxStartArguments.result = result;
  RexxStartArguments.outputName = NULL;
                                       /* this is a real execution          */
  RexxStartArguments.translating = FALSE;
#ifdef SOM
  RexxSomInitialize((SOMObject *) NULL);  /* Perform any needed inits          */
#else
//  if(!rexxutil_call){                  /* no init if called from a rexxutil */
//    RexxInitialize();                  /* Perform any needed inits        */
//  }
//  else{
    rexxutil_call = FALSE;
    rexxutil_call_sem.release();
//  }

#endif
                                       /* pass along to the real method     */
  rc = RexxSendMessage(ProcessLocalServer, CHAR_RUN_PROGRAM, NULL, "vp", NULL, &RexxStartArguments);
//RexxTerminate();                     /* perform needed termination        */
  return -rc;                          /* return the error code (negated)   */
}
//#endif /* AIX */

/******************************************************************************/
/* Name:       RexxTranslateProgram                                           */
/*                                                                            */
/* Arguments:  inFile   - Input source program                                */
/*             outFile  - Output file name                                    */
/*                                                                            */
/******************************************************************************/
APIRET REXXENTRY RexxTranslateProgram(
   PSZ   inFile,                       /* input source program              */
   PSZ   outFile )                     /* output file name                  */
{
  LONG     rc;                         /* RexxStart return code             */
  RexxStartInfo RexxStartArguments;    /* entry argument information        */

                                       /* clear the RexxStart block         */
  memset((void *)&RexxStartArguments, 0, sizeof(RexxStartInfo));
                                       /* program name is the input file    */
  RexxStartArguments.programname = inFile;
                                       /* and pass along the output name    */
  RexxStartArguments.outputName = outFile;
                                       /* this is a translation step        */
  RexxStartArguments.translating = TRUE;

#ifdef SOM
                                       /* Perform any needed inits          */
  RexxSomInitialize((SOMObject *) NULL);
#else
  RexxInitialize();                    /* Perform any needed inits          */
#endif

                                       /* pass along to the real method     */
  rc = RexxSendMessage(ProcessLocalServer, CHAR_RUN_PROGRAM, NULL, "vp", NULL, &RexxStartArguments);
  RexxTerminate();                     /* perform needed termination        */
  return rc;                           /* return the error code             */
}

#ifdef TIMESLICE
/******************************************************************************/
/* Name:       RexxSetYield                                                   */
/*                                                                            */
/* Arguments:  procid - Process id of target REXX procedure                   */
/*             threadid - Thread id of target REXX procedure                  */
/*                                                                            */
/* Returned:   rc - RXARI_OK (halt condition set)                             */
/*                  RXARI_NOT_FOUND (couldn't find threadid)                  */
/*                                                                            */
/* Notes:  activity_setyield -> activation_yield ->..->activity_relinquish    */
/*         Causes bits in top activation to be flipped which will cause       */
/*         a system yield via activity_relinquish.                            */
/*                                                                            */
/******************************************************************************/
APIRET REXXENTRY RexxSetYield(PID procid, TID threadid)
{
  if (RexxQuery()) {                        /* Are we up?                     */
    if(activity_sysyield(threadid,NULL))    /* Set yield condition?           */
      return (RXARI_OK);                    /* Yes, return okay               */
    else
      return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
    }
  else
    return (RXARI_NOT_FOUND);               /* REXX not running, error...     */
}

#endif /* TIMESLICE*/

/******************************************************************************/
/* Name:       RexxSetHalt                                                    */
/*                                                                            */
/* Arguments:  procid - Process id of target REXX procedure                   */
/*             threadid - Thread id of target REXX procedure                  */
/*                                                                            */
/* Returned:   rc - RXARI_OK (halt condition set)                             */
/*                  RXARI_NOT_FOUND (couldn't find threadid)                  */
/*                                                                            */
/* Notes:      Sends request to the activity to flip on the halt flag in the  */
/*             target activation.                                             */
/******************************************************************************/
APIRET REXXENTRY RexxSetHalt(PID procid, TID threadid)
{
  if (RexxQuery()) {                        /* Are we up?                     */
    if(activity_halt((long)threadid, OREF_NULL)) /* Set halt condition?            */
      return (RXARI_OK);                    /* Yes, return okay               */
    else
      return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
  }
  else
    return (RXARI_NOT_FOUND);               /* REXX not running, error...     */
}

/******************************************************************************/
/* Name:       RexxSetTrace                                                   */
/*                                                                            */
/* Arguments:  procid - Process id of target REXX procedure                   */
/*             threadid - Thread id of target REXX procedure                  */
/*                                                                            */
/* Returned:   rc - RXARI_OK (halt condition set)                             */
/*                  RXARI_NOT_FOUND (couldn't find threadid)                  */
/*                                                                            */
/* Notes:      Sends request to the activity to turn on interactive trace in  */
/*             the target activation.                                         */
/******************************************************************************/
APIRET REXXENTRY RexxSetTrace(PID procid, TID threadid)
{
  if (RexxQuery()) {                        /* Are we up?                     */
    if(activity_set_trace((long)threadid, 1))    /* Set trace on?                  */
      return (RXARI_OK);                    /* Yes, return okay               */
    else
      return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
    }
  else
    return (RXARI_NOT_FOUND);               /* REXX not running, error...     */
}


/******************************************************************************/
/* Name:       RexxResetTrace                                                 */
/*                                                                            */
/* Arguments:  procid - Process id of target REXX procedure                   */
/*             threadid - Thread id of target REXX procedure                  */
/*                                                                            */
/* Returned:   rc - RXARI_OK (halt condition set)                             */
/*                  RXARI_NOT_FOUND (couldn't find threadid)                  */
/*                                                                            */
/* Notes:      Sends request to the activity to turn off interactive trace in */
/*             the target activation.                                         */
/******************************************************************************/
APIRET REXXENTRY RexxResetTrace(PID procid, TID threadid)
{
  if (RexxQuery()) {                        /* Are we up?                     */
    if(activity_set_trace((long)threadid,0))     /* Set trace off??                */
      return (RXARI_OK);                    /* Yes, return okay               */
    else
      return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
    }
  else
    return (RXARI_NOT_FOUND);               /* REXX not running, error...     */
}


VOID REXXENTRY RexxBreakCleanup(VOID){}


void translateSource(
   RexxString           * inputName,   /* input program name                */
   RexxNativeActivation * newNativeAct,/* base activation                   */
   PSZ                    outputName ) /* output file name                  */
/******************************************************************************/
/* Function:  Process instorage execution arguments                           */
/******************************************************************************/
{
  RexxString * fullName;               /* fully resolved input name         */
  RexxMethod * method;                 /* created method                    */
  PCHAR        pszName;

                                       /* go resolve the name               */
  pszName = SearchFileName(inputName->stringData, 'P'); /* PATH search      */
  if (pszName != OREF_NULL) fullName = new_cstring(pszName);
  else
//  if (fullName == OREF_NULL)           /* not found?                        */
                                       /* got an error here                 */
    report_exception1(Error_Program_unreadable_notfound, inputName);
/*    report_exception1(Error_Program_unreadable_notfound, fullName);       */
  newNativeAct->saveObject(fullName);  /* protect from garbage collect      */
                                       /* go translate the image            */
  method = TheMethodClass->newFile(fullName);
  if (outputName != NULL) {            /* want to save this to a file?      */
    newNativeAct->saveObject(method);  /* protect from garbage collect      */
                                       /* go save this method               */
    SysSaveTranslatedProgram((PCHAR)outputName, method);
  }
}

RexxMethod * process_instore(
   PRXSTRING    instore,               /* instore arguments                 */
   RexxString * name )                 /* program name                      */
/******************************************************************************/
/* Function:  Process instorage execution arguments                           */
/******************************************************************************/
{
  RexxMethod * method;                 /* returned method                   */
  RexxBuffer * source_buffer;          /* buffered source                   */
  USHORT   temp;                       /* unused position info              */
  RXSTRING buffer;                     /* instorage buffer                  */
  RexxMethod * Routine;                /* method to execute                 */

  if (instore[0].strptr == NULL && instore[1].strptr == NULL) {
                                       /* see if this exists                */
    if (!RexxQueryMacro(name->stringData, (PUSHORT)&temp)) {
      /* The ExecMacro func returns a ptr to the shared memory. So we must  */
      /* call APISTARTUP to be sure that the ptr remains valid.             */
      APISTARTUP(MACROCHAIN);
                                       /* get the image of function         */
      RexxExecuteMacroFunction(name->stringData, &buffer);
                                       /* unflatten the method now          */
      Routine = SysRestoreProgramBuffer(&buffer, name);
      APICLEANUP(MACROCHAIN);
      return Routine;                  /* return the method                 */
    }
    return OREF_NULL;                  /* temp scaffolding                  */
  }
  if (instore[1].strptr != NULL) {     /* have an image                     */
                                       /* go convert into a method          */
    method = SysRestoreProgramBuffer(&instore[1], name);
    if (method != OREF_NULL) {         /* did it unflatten successfully?    */
      if (instore[0].strptr != NULL) { /* have source also?                 */
                                       /* get a buffer object               */
        source_buffer = new_buffer(instore[0].strlength);
                                       /* copy source into the buffer       */
        memcpy(source_buffer->data, instore[0].strptr, instore[0].strlength);
                                       /* reconnect this with the source    */
        ((RexxCode *)method->code)->u_source->setBufferedSource(source_buffer);
      }
      return method;                   /* go return it                      */
    }
  }
  if (instore[0].strptr != NULL) {     /* have instorage source             */
                                       /* get a buffer object               */
    source_buffer = new_buffer(instore[0].strlength);
                                       /* copy source into the buffer       */
    memcpy(source_buffer->data, instore[0].strptr, instore[0].strlength);

  if (source_buffer->data[0] == '#' && source_buffer->data[1] == '!')
  {
    memcpy(source_buffer->data, "--", 2);
  }
                                       /* translate this source             */
    method = TheMethodClass->newRexxBuffer(name, source_buffer, (RexxClass *)TheNilObject);
                                       /* return this back in instore[1]    */
    SysSaveProgramBuffer(&instore[1], method);
    return method;                     /* return translated source          */
  }
  return OREF_NULL;                    /* processing failed                 */
}

/*********************************************************************/
/*                                                                   */
/*   Function:  Do initial startup on a REXX program                 */
/*                                                                   */
/*********************************************************************/
void  SysRunProgram(
  PVOID   ControlInfo )                /* flattened control information     */
{
  RexxStartInfo *self;                 /* Rexxstart argument info           */
  RexxArray   * new_arglist;           /* passed on argument list           */
  LONG          i;                     /* loop counter                      */
  RexxString  * fullname;              /* fully resolved program name       */
  RexxString  * name;                  /* input program name                */
  RexxMethod  * method;                /* translated file image             */
  RexxString  * source_calltype;       /* parse source call type            */
  BOOL          tokenize_only;         /* don't actually execute program    */
  RexxString  * initial_address;       /* initial address setting           */
  PCHAR         file_extension;        /* potential file extension          */
  RexxString  * program_result;        /* returned program result           */
  RexxNativeActivation * newNativeAct; /* Native Activation to run on       */
  LONG          length;                /* return result length              */
  LONG          return_code;           /* converted return code info        */

  tokenize_only = FALSE;               /* default is to run the program     */
                                       /* create the native method to be run*/
                                       /* on the activity                   */
  newNativeAct = new ((RexxObject *)CurrentActivity, OREF_NULL, CurrentActivity, OREF_PROGRAM, OREF_NULL) RexxNativeActivation;
  CurrentActivity->push(newNativeAct); /* Push new nativeAct onto stack     */
  self = (RexxStartInfo *)ControlInfo; /* address all of the arguments      */
  if (self->programname != NULL)       /* have an actual name?              */
                                       /* get string version of the name    */
    name = new_cstring(self->programname);
  else
    name = OREF_NULLSTRING;            /* use an "unlocatable" name         */
  newNativeAct->saveObject(name);      /* protect from garbage collect      */
  if (self->translating) {             /* just translating this?            */
                                       /* just do the translation step      */
    translateSource(name, newNativeAct, self->outputName);
    return;                            /* and finished                      */
  }

  if (self->exits != NULL) {           /* have exits to process             */
      i = 0;                           /* start with first exit             */
                                       /* while not the list ender          */
      while (self->exits[i].sysexit_code != RXENDLST) {
                                       /* enable this exit                  */
        CurrentActivity->setSysExit(self->exits[i].sysexit_code, new_cstring(self->exits[i].sysexit_name));
        i++;                           /* step to the next exit             */
      }
    }
                                       /* get a new argument array          */
  new_arglist = new_array(self->argcount);
                                       /* lock the argument list            */
  newNativeAct->saveObject(new_arglist);
                                       /* loop through the argument list    */
  for (i = 0; i < self->argcount; i++) {
    if (self->arglist[i].strptr != NULL)     /* have a real argument?             */
                                       /* add to the argument array         */
      new_arglist->put(new_string(self->arglist[i].strptr, self->arglist[i].strlength), i + 1);
  }
  if (self->calltype == RXCOMMAND) {   /* need to process command arg?      */
                                       /* is there an argument?             */
    if (self->arglist != NULL && self->arglist[0].strptr != NULL && self->arglist[0].strlength > 1) {
      if (*(self->arglist[0].strptr) == ' ')   /* is there a leading blank?         */
                                       /* replace the first argument        */
        new_arglist->put(new_string(self->arglist[0].strptr+1, self->arglist[0].strlength - 1), 1);
                                       /* have a "//T" in the argument?     */
      if ( (((RexxString *)(new_arglist->get(1)))->pos(OREF_TOKENIZE_ONLY, 0) !=
                              0) && RexxStartedByApplication)
        tokenize_only = TRUE;          /* don't execute this                */
    }
    RexxStartedByApplication = TRUE;
  }
  switch (self->calltype) {            /* turn calltype into a string       */

    case  RXCOMMAND:                   /* command invocation                */
      source_calltype = OREF_COMMAND;  /* this is the 'COMMAND' string      */
      break;

    case  RXFUNCTION:                  /* function invocation               */
                                       /* 'FUNCTION' string                 */
      source_calltype = OREF_FUNCTIONNAME;
      break;

    case  RXSUBROUTINE:                /* subroutine invocation             */
                                       /* 'SUBROUTINE' string               */
      source_calltype = OREF_SUBROUTINE;
      break;

    default:
      source_calltype = OREF_COMMAND;  /* this is the 'COMMAND' string      */
      break;
  }
  if (self->instore == NULL) {         /* no instore request?               */
                                       /* go resolve the name               */
    fullname = SysResolveProgramName(name, OREF_NULL);
    if (fullname == OREF_NULL)         /* not found?                        */
                                       /* got an error here                 */
      report_exception1(Error_Program_unreadable_notfound, name);
    newNativeAct->saveObject(fullname);/* protect from garbage collect      */
                                       /* try to restore saved image        */
    method = SysRestoreProgram(fullname);
    if (method == OREF_NULL) {         /* unable to restore?                */
                                       /* go translate the image            */
      method = TheMethodClass->newFile(fullname);
      newNativeAct->saveObject(method);/* protect from garbage collect      */
      SysSaveProgram(fullname, method);/* go save this method               */
    }
  }
  else {                               /* have an instore program           */
                                       /* go handle instore parms           */
    method = process_instore(self->instore, name);
    if (method == OREF_NULL)           /* couldn't get it?                  */
                                       /* got an error here                 */
      report_exception1(Error_Program_unreadable_name, name);
    fullname = name;                   /* copy the name                     */
  }
  if (self->envname != NULL)           /* have an address override?         */
                                       /* use the provided one              */
    initial_address = new_cstring(self->envname);
  else {
                                       /* check for a file extension        */
    file_extension = SysFileExtension((PCHAR)fullname->stringData);
    if (file_extension != NULL)      /* have a real one?                  */
                                       /* use extension as the environment  */
      initial_address = new_cstring(file_extension + 1);
    else
                                       /* use system defined default        */
      initial_address = OREF_INITIALADDRESS;
  }
                                       /* protect from garbage collect      */
  newNativeAct->saveObject(initial_address);
                                       /* actually need to run this?        */
  if (method != OREF_NULL && !tokenize_only) {
                                       /* Check to see if halt or trace sys */
    CurrentActivity->queryTrcHlt();    /* were set                          */
                                       /* run and get the result            */
//  program_result = (RexxString *)((RexxObject *)CurrentActivity)->shriekRun(method, source_calltype, initial_address, new_arglist);
    program_result = (RexxString *)((RexxObject *)CurrentActivity)->shriekRun(method, source_calltype, initial_address, new_arglist->data(), new_arglist->size());
    if (self->result != NULL) {        /* if return provided for            */
                                       /* actually have a result to return? */
      if (program_result != OREF_NULL) {
                                       /* force to a string value           */
        program_result = program_result->stringValue();
                                       /* get the result length             */
        length = program_result->length + 1;
                                       /* buffer too short or no return?    */
        if (length > self->result->strlength || self->result->strptr == NULL)
        {
          if (self->result->strptr)    /* if memory allocated               */
          {
             SysReleaseResultMemory(self->result->strptr);
          }
                                       /* allocate a new RXSTRING buffer    */
          self->result->strptr = (char *)SysAllocateResultMemory(length);
        }
                                       /* yes, copy the data (including the */
                                       /* terminating null implied by the   */
                                       /* use of length + 1                 */
        memcpy(self->result->strptr, program_result->stringData, length);
                                       /* give the true data length         */
        self->result->strlength = length - 1;
      }
      else {                           /* make this an invalid string       */
        MAKERXSTRING(*(self->result), NULL, 0);
      }
    }

    if (self->retcode) {               /* asking for the binary return code?*/
      *(self->retcode) = 0;            /* set default rc value              */
                                       /* If there is a return val...       */
      if (program_result != OREF_NULL) {
                                       /* convert to a long value           */
        return_code = program_result->longValue(DEFAULT_DIGITS);
                                       /* if a whole number...              */
        if (return_code != NO_LONG && return_code <= SHRT_MAX && return_code >= SHRT_MIN)
                                       /* ...copy to return code.           */
          *(self->retcode) = (SHORT)return_code;
      }
    }
  }
  CurrentActivity->pop(FALSE);         /* finally, discard our activation   */
}

/* functions for concurrency synchronization/termination               */


#if defined(AIX) || defined(LINUX)

void APIENTRY RexxWaitForTermination(void)
{
/* the while loop is necessary to prevent that in parallel processing              */
/* RexxWaitForTermination is started, before initialization process has completed  */
   while (!SecureFlag)
   {
     SysThreadYield();
   }

   SysEnterCriticalSection();
   if (!RexxTerminated) {
     SysExitCriticalSection();
     return;
   }
   SysExitCriticalSection();

/* For now control can be captured by other REXX-threads. The RexxTerminated       */
/* Event-semaphore is set by kernelshutdown that resides within a critical section */

   EVWAIT(RexxTerminated);
   SysEnterCriticalSection();
   EVCLOSE(RexxTerminated);
   RexxTerminated = NULL;
   SecureFlag = 0;
   thread_counter = 0;
   MTXCL(start_semaphore);
   MTXCL(resource_semaphore);
   MTXCL(kernel_semaphore);
   MTXCL(memoryObject.flattenMutex);
   MTXCL(memoryObject.unflattenMutex);
   MTXCL(memoryObject.envelopeMutex);
   memoryObject.flattenMutex = 0;
   memoryObject.unflattenMutex = 0;
   memoryObject.envelopeMutex = 0;

   SysExitCriticalSection();
   MTXCL(initialize_sem);
   initialize_sem = 0;                                      /*important s. SysThreadInit info */
}


APIRET APIENTRY RexxDidRexxTerminate(void)
{
   if (!RexxTerminated) return TRUE;

   if (RexxTerminated->posted())
   {
       EVCLOSE(RexxTerminated);  /* Close semaphore if it is posted */
       RexxTerminated = NULL;

       MTXCL(initialize_sem);
       MTXCL(start_semaphore);
       MTXCL(resource_semaphore);
       MTXCL(kernel_semaphore);
       MTXCL(memoryObject.flattenMutex);
       MTXCL(memoryObject.unflattenMutex);
       MTXCL(memoryObject.envelopeMutex);
       memoryObject.flattenMutex = 0;
       memoryObject.unflattenMutex = 0;
       memoryObject.envelopeMutex = 0;
       return TRUE;
   }
   else return FALSE;
}
#endif

/*********************************************************************/
/*                                                                   */
/*   Function:  Get REXX version information                         */
/*                                                                   */
/*********************************************************************/

PSZ RexxGetVersionInformation(void)
{
  void *    pVersionString = NULL;

  pVersionString = (PCHAR) malloc(BUFFERLEN_OS);
  if (pVersionString != NULL) {
    sprintf((char *)pVersionString ,"Open Object Rexx Interpreter Version %s for %s\nBuild date: %s\n", PACKAGE_VERSION, ORX_SYS_STR, __DATE__);
    strcat((char *)pVersionString , COPYRIGHT);
  }
  return (char *)pVersionString;
}
