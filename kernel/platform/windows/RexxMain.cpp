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
/* REXX Win  Support                                            winmain.c     */
/*                                                                            */
/* Main interface to the REXX interpreter                                     */
/*                                                                            */
/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <signal.h>

#define INCL_REXXSAA
#define INCL_DOSPROCESS
#define INCL_DOSMEMMGR
#define INCL_DOSEXCEPTIONS
#define INCL_DOSFILEMGR
#define INCL_DOSQUEUES
#define INCL_DOSSEMAPHORES
// #define MAX_REXX_PROGS 0xFFFF

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
#include "DirectoryClass.hpp"
#include "RexxVariableDictionary.hpp"
#include "IntegerClass.hpp"
#include "RexxNativeAPI.h"                      /* REXX interface/native method code */
#include SYSREXXSAA
#include "APIServiceTables.h"
#include "SubcommandAPI.h"
#include "RexxAPIManager.h"
#include "ActivityTable.hpp"

#define DEFAULT_PRECISION  9

#include <fcntl.h>
#include <io.h>

#ifdef SOM
#include "som.xh"
#include "orxsom.h"
#endif


#ifdef TIMESLICE                       /* System Yielding function prototype*/
INT REXXENTRY RexxSetYield(PID procid, TID threadid);
#ifdef HIGHTID
extern ActivityTable * ProcessLocalActs; /* needed for halt and trace */
#endif
#endif /*timeslice*/


extern CRITICAL_SECTION Crit_Sec = {0};      /* also used by OKACTIVI and OKMEMORY */

extern SEV   RexxTerminated;           /* Termination complete semaphore.   */
extern BOOL UseMessageLoop;  /* speciality for VAC++ */

extern RexxDirectory *ProcessLocalEnv; /* process local environment (.local)*/
extern RexxObject *ProcessLocalServer; /* current local server              */
extern RexxActivity *CurrentActivity;  /* current active activity           */
PCHAR SysFileExtension(PCHAR);
RexxMethod *SysRestoreProgramBuffer(PRXSTRING, RexxString *);
RexxMethod *SysRestoreInstoreProgram(PRXSTRING, RexxString *);
void SysSaveProgramBuffer(PRXSTRING, RexxMethod *);
void SysSaveTranslatedProgram(PCHAR, RexxMethod *);
BOOL SearchFileName(PCHAR, PCHAR);
RexxActivity *activity_getActivity(BOOL *nested);
extern BOOL RexxStartedByApplication;

extern "C" {
APIRET REXXENTRY RexxTranslateProgram( PSZ, PSZ, PRXSYSEXIT);
}

// SIGHANDLER * oldSigIntHandler, * oldSigBreakHandler;


#define TRANSLATE     0                /* performing a translation only     */
#define EXECUTE       1                /* translate and execute Rexx Proc.  */
#define CREATEMETHOD  2                /* translate and return method obj.  */
#define RUNMETHOD     3                /* run method object                 */
#define LOADMETHOD    4                /* unflatten data                    */
#define STOREMETHOD   5                /* flatten method object             */

typedef struct _RexxScriptInfo {       /* Control info used by various API's*/
  SHORT runtype;
  PSZ index;                           /* index to directory which contains */
                                       /* objects being kept for process.   */

  PRXSTRING ProgramBuffer;             /* Data Buffer                       */
  RexxMethod * *pmethod;               /* Method Object                     */
  RexxObject * *presult;               /* Result Object                     */
  // these changes add the capability to use RexxRunMethod with
  // a. parameters and b. exits
  // as fas as I could make out, this function has never(!) been used up to now...
  RexxArray* (__stdcall *func)(void*); // callback for converting arbitrary data into REXX data types (stored in a RexxArray)
  void *args;                          // arguments for callback. if func == NULL, this will be treated as a RexxArray
  PRXSYSEXIT exits;                    // Array of system exit names used analogous to RexxStart() exits...
} RexxScriptInfo;

typedef struct _ConditionData {
  long   code;
  long   rc;
  RXSTRING message;
  RXSTRING errortext;
  long   position;
  RXSTRING program;
} ConditionData;

typedef struct _RexxStartInfo {
  SHORT runtype;                       /* How source should be handled      */
  LONG       argcount;                 /* Number of args in arglist         */
  PRXSTRING  arglist;                  /* Array of args                     */
  PSZ        programname;              /* REXX program to run               */
  PRXSTRING  instore;                  /* Instore array                     */
  PSZ        envname;                  /* Initial cmd environment           */
  SHORT      calltype;                 /* How the program is called         */
  PRXSYSEXIT exits;                    /* Array of system exit names        */
  PSHORT     retcode;                  /* Integer form of result            */
  PRXSTRING  result;                   /* Result returned from program      */
  PSZ        outputName;               /* compilation output file           */
} RexxStartInfo;

BOOL HandleException = TRUE;           /* Global switch for Exception Handling */
extern CRITICAL_SECTION waitProtect;

void REXXENTRY RexxCreateDirectory(PCHAR dirname)
{

  RexxString  *index;
  RexxActivity *activity;              /* target activity                   */

                                       /* (will create one if necessary)    */
  activity = TheActivityClass->getActivity();

                                       /* Create string object              */
  index = new_cstring(dirname);

                                       /* Create directory and hang of off  */
                                       /* local env. directory.             */
  ProcessLocalEnv->put(new_directory(), index);
  TheActivityClass->returnActivity(CurrentActivity);       /* release the kernel semaphore      */

  return;
}
/*********************************************************************/
/* Function SearchPrecision:  Check the precision of the activity and*/
/* use it if possible                                                */
/*********************************************************************/
extern "C" {
void SearchPrecision(
  PULONG    precision)                 /* required precision         */
{
  RexxActivity        *activity;
  RexxActivation      *activation;
  int i, thread_id,threadid;

  *precision = DEFAULT_PRECISION;      /* set default digit count    */

/* give me the numeric digits settings of the current actitity       */

  threadid = GetCurrentThreadId();
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

RexxActivity *RunActivity = NULL;

// this function can be used to retrieve the value of "top level" variables
// on exit (RXTER). for each variable name that is found a call to the passed-in
// function is made [a pair ((char*) varname,(RexxObject*) varvalue)].
// please note that this does not work for multi-threaded environments!
void WinGetVariables(void (__stdcall *f)(void*))
{
  void* args[2];
  RexxArray *result;

  if (RunActivity) {
    result = RunActivity->currentActivation->getAllLocalVariables();
    for (size_t i=result->size(); i>0; i--) {
        RexxVariable *variable = (RexxVariable *)result->get(i);
        args[0] = (void *) variable->getName()->stringData;
        args[1] = (void*) variable->getResolvedValue();
        f(args);
    }
  }
  return;
}

RexxActivity *WinStore = NULL;

void WinEnterKernel(bool execute)
{
  if (execute) {
    RexxInitialize();
    TheActivityClass->getActivity(); // create a "current" activity
  }

  RexxNativeActivation *newNativeAct = new ((RexxObject *)CurrentActivity, OREF_NULL, CurrentActivity, OREF_PROGRAM, OREF_NULL) RexxNativeActivation;
  CurrentActivity->push(newNativeAct); /* Push new nativeAct onto stack     */
  WinStore = CurrentActivity;
}

void WinLeaveKernel(bool execute)
{
  if (CurrentActivity == NULL)
    CurrentActivity = WinStore;
  CurrentActivity->pop(FALSE);
  if (execute) {
    TheActivityClass->returnActivity(CurrentActivity);
    RexxTerminate();
  }

}

/******************************************************************************/
/* Name:       RexxRemoveDirectory                                            */
/*                                                                            */
/* Arguments:  dirname - Name of directory object to be created.              */
/*                                                                            */
/* Notes: Remove directory object from local environment.                     */
/******************************************************************************/

void REXXENTRY RexxRemoveDirectory(PCHAR dirname)
{
  RexxString  *index;
  RexxActivity *activity;              /* target activity                   */

                                       /* (will create one if necessary)    */
  activity = TheActivityClass->getActivity();

                                       /* Create string object              */
  index = new_cstring(dirname);
                                       /* Remove directory from Local env.  */
  ProcessLocalEnv->remove(index);
                                       /* release the kernel semaphore      */
  TheActivityClass->returnActivity(CurrentActivity);
  return;
}

/******************************************************************************/
/* Name:       RexxDispose                                                    */
/*                                                                            */
/* Arguments:  dirname - Name of directory object to remove object from       */
/*             RexxObj - Object to be removed                                 */
/*                                                                            */
/* Notes: Remove object from the directory of saved objects.                  */
/*                                                                            */
/* Returned:   TRUE - Object disposed ok                                      */
/*             FALSE - Invalid REXX object                                    */
/******************************************************************************/

BOOL REXXENTRY RexxDispose(PCHAR dirname, RexxObject *RexxObj)
{
  RexxDirectory *locked_objects;       /* directory used to keep objects    */
  RexxString  *index;
  RexxActivity *activity;              /* target activity                   */

                                       /* Find an activity for this thread  */
                                       /* (will create one if necessary)    */
  activity = TheActivityClass->getActivity();

                                       /* Create string object              */
  index = new_cstring(dirname);
                                       /* Get directory of locked objects   */
  locked_objects = (RexxDirectory *)ProcessLocalEnv->at(index);
                                       /* Remove object                     */
  RexxObj = locked_objects->remove(new_string((PCHAR)&RexxObj, sizeof(LONG)));
                                       /* release the kernel semaphore      */
  TheActivityClass->returnActivity(CurrentActivity);
  if (RexxObj == TheNilObject)
    return FALSE;
  else
    return TRUE;
}

/******************************************************************************/
/* Name:       RexxResultString                                               */
/*                                                                            */
/* Arguments:  result - (Input) result object containing Result String.       */
/*             pResultBuffer - (Output) Result String.                        */
/*                                                                            */
/* Notes: Return resulting string from Result Object                          */
/*                                                                            */
/* Returned:   1 - General error                                              */
/*            -1 - Invalid Object                                             */
/******************************************************************************/

APIRET REXXENTRY RexxResultString(RexxObject * result, PRXSTRING pResultBuffer)
{
  LONG  length;
  RexxActivity *activity;              /* target activity                   */
  ULONG rc = 0;
  RexxString *string_result;

  activity = TheActivityClass->getActivity();

                                       /* force to a string value           */
  string_result = result->stringValue();
  if (string_result != NULL) {                /* didn't convert?                   */
                                       /* get the result length             */
    length = string_result->length +1;
                                       /* allocate a new RXSTRING buffer    */
    pResultBuffer->strptr = (char *)SysAllocateResultMemory(length);
    if (pResultBuffer->strptr) {       /* Got storage ok ?                  */

                                       /* yes, copy the data (including the */
                                       /* terminating null implied by the   */
                                       /* use of length + 1                 */
      memcpy(pResultBuffer->strptr, string_result->stringData, length);
                                       /* give the true data length         */
      pResultBuffer->strlength = length - 1;
    } else
      rc = 1;
  } else
    rc = -1;
  TheActivityClass->returnActivity(CurrentActivity);       /* release the kernel semaphore      */
  return rc;
}

/******************************************************************************/
/* Name:       RexxSetOSAInfo                                                 */
/*                                                                            */
/* Arguments:  method - (Input) method object to hang OSA info off of.        */
/*             selector - (Input) Index into OSAinfo directory                */
/*             value - (Input) OSA information                                */
/*                                                                            */
/* Notes: Keep OSA information along with method object                       */
/*                                                                            */
/* Returned:  -1 - Invalid Object                                             */
/*                                                                            */
/******************************************************************************/
APIRET REXXENTRY RexxSetOSAInfo(RexxObject * method, ULONG selector, LONG value)
{
  RexxDirectory *OSAinfoDir;

  RexxActivity *activity;              /* target activity                   */
  ULONG rc = 0;

  activity = TheActivityClass->getActivity();

  if (OTYPENUM(method, method)) {      /* Make sure this is a method object  */
    OSAinfoDir = ((RexxMethod *)method)->getInterface();
                                         /* save value in directory          */
    OSAinfoDir->put(new_integer(value), new_string((PCHAR)&selector, sizeof(ULONG)));
  } else
    rc = -1;

  TheActivityClass->returnActivity(CurrentActivity);       /* release the kernel semaphore      */
  return rc;
}

/******************************************************************************/
/* Name:       RexxGetOSAInfo                                                 */
/*                                                                            */
/* Arguments:  method - (Input) method object to get OSA info from.           */
/*             selector - (Input) Index into OSAinfo directory                */
/*             value - (Output) OSA information                               */
/*                                                                            */
/* Notes: Get OSA information associated with method object                   */
/*                                                                            */
/* Returned:   1 - General Error                                              */
/*            -1 - Invalid Object                                             */
/*                                                                            */
/******************************************************************************/

APIRET REXXENTRY RexxGetOSAInfo(RexxObject * method, ULONG selector, LONG * value)
{
  RexxInteger *selected;
  ULONG rc = 0;
  RexxDirectory *OSAinfoDir;
  RexxActivity *activity;              /* target activity                   */


  activity = TheActivityClass->getActivity();

  if (OTYPENUM(method, method)) {      /* Make sure this is a method object */
    OSAinfoDir = ((RexxMethod *)method)->getInterface();
                                       /* Get value                         */
    selected = (RexxInteger *)OSAinfoDir->at(new_string((PCHAR)&selector, sizeof(ULONG)));
    if (selected  != OREF_NULL) {      /* Have a value ?                    */
                                       /* Yes, Get value from integer object*/
      *value = selected->value;
    } else
      rc = 1;
  } else
    rc = -1;                           /* Not a method.                     */

  TheActivityClass->returnActivity(CurrentActivity);       /* release the kernel semaphore      */
  return rc;
}

/******************************************************************************/
/* Name:       RexxGetSource                                                  */
/*                                                                            */
/* Arguments:  method - (Input) method object to get source from.             */
/*             pSourceBuffer - (Output) pointer to RXSTRING containing Source */
/*                                                                            */
/* Notes: Get OSA information associated with method object                   */
/*                                                                            */
/* Returned:   1 - General Error                                              */
/*            -1 - Invalid Object                                             */
/*                                                                            */
/******************************************************************************/

APIRET REXXENTRY RexxGetSource(RexxObject * method, PRXSTRING pSourceBuffer)
{
  RexxString *source;
  LONG length;
  RexxDirectory *OSAinfoDir;
  RexxActivity *activity;              /* target activity                   */
  ULONG rc = 0;


  activity = TheActivityClass->getActivity();

  if (OTYPENUM(method, method)) {      /* Make sure this is a method object */
    OSAinfoDir = ((RexxMethod *)method)->getInterface();
    source = (RexxString *)OSAinfoDir->at(new_cstring("SOURCE"));

    length = source->length +1;
                                       /* allocate a new RXSTRING buffer    */
    pSourceBuffer->strptr = (char *)SysAllocateResultMemory(length);
    if (pSourceBuffer->strptr) {       /* Got storage ok ?                  */
                                       /* yes, copy the data (including the */
                                       /* terminating null implied by the   */
                                       /* use of length + 1                 */
      memcpy(pSourceBuffer->strptr, source->stringData, length);
                                       /* give the true data length         */
      pSourceBuffer->strlength = length - 1;
    } else
      rc = 1;
  } else
    rc = -1;
  TheActivityClass->returnActivity(CurrentActivity);       /* release the kernel semaphore      */
  return rc;
}

/******************************************************************************/
/* Name:       RexxCopyMethod                                                 */
/*                                                                            */
/* Arguments:  dirname - (Input) Name of directory object used for anchoring  */
/*                          objects that need to be kept around.              */
/*             method - (Input) method object to be copied.                   */
/*             pmethod - (Output) pointer to new method                       */
/*                                                                            */
/* Notes: Clone a method                                                      */
/*                                                                            */
/* Returned:   1 - General Error                                              */
/*            -1 - Invalid Object                                             */
/*                                                                            */
/******************************************************************************/

APIRET REXXENTRY RexxCopyMethod(PCHAR dirname, RexxObject * method, RexxObject * *pmethod)
{
  RexxDirectory *locked_objects;
  RexxActivity *activity;              /* target activity                   */
  ULONG rc = 0;

                                       /* Find an activity for this thread  */
                                       /* (will create one if necessary)    */

  activity = TheActivityClass->getActivity();
  if (OTYPENUM(method, method)) {      /* Make sure this is a method object */

    if ((*pmethod = (RexxMethod *)method->copy()) != OREF_NULL) {

                                       /* Need to keep around for process   */
                                       /* duration.                         */
      locked_objects = (RexxDirectory *)ProcessLocalEnv->at(new_cstring(dirname));
      locked_objects->put(*pmethod, new_string((PCHAR)pmethod, sizeof(LONG)));
    } else
      rc = 1;
  } else
    rc = -1;

  TheActivityClass->returnActivity(CurrentActivity);       /* release the kernel semaphore      */
  return rc;
}

/******************************************************************************/
/* Name:       RexxValidObject                                                */
/*                                                                            */
/* Arguments:  dirname - (Input) Name of directory object used for anchoring  */
/*                          objects that need to be kept around.              */
/*             object - (Input) object to be validated                        */
/*                                                                            */
/* Notes: Make sure that the object specified is acnchored in the directory   */
/*        specified by dirname.                                               */
/*                                                                            */
/* Returned:   TRUE - Valid living REXX OBJECT                                */
/*             FALSE - Invalid REXX object                                    */
/*                                                                            */
/******************************************************************************/

BOOL REXXENTRY RexxValidObject(PCHAR dirname, RexxObject * object)
{
  RexxDirectory *locked_objects;       /* directory used to keep objects    */
  RexxActivity *activity;              /* target activity                   */

                                       /* Find an activity for this thread  */
                                       /* (will create one if necessary)    */
  activity = TheActivityClass->getActivity();

                                       /* Get directory of locked objects   */
  locked_objects = (RexxDirectory *)ProcessLocalEnv->at(new_cstring(dirname));
                                       /* See if object exists              */
  object = locked_objects->at(new_string((PCHAR)&object, sizeof(LONG)));
                                       /* release the kernel semaphore      */
  TheActivityClass->returnActivity(CurrentActivity);
  if (object == OREF_NULL)             /* Was the object in the directory ? */
    return FALSE;                      /* Invalid object                    */
  else
    return TRUE;                       /* Valid object                      */
}



/******************************************************************************/
/* Name:       RexxMain                                                       */
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
/*   Mainline (32-bit) path looks like this:                                  */
/*     RexxStart => RexxMain => server_RexxStart                              */
/*   Mainline (16-bit) path looks like this:                                  */
/*     REXXSAA => RexxStart32 => RexxStart => RexxMain => server_RexxStart    */
/******************************************************************************/
APIRET APIENTRY RexxStart(
  LONG argcount,                       /* Number of args in arglist         */
  PRXSTRING arglist,                   /* Array of args                     */
  PCSZ programname,                    /* REXX program to run               */
  PRXSTRING instore,                   /* Instore array                     */
  PCSZ envname,                        /* Initial cmd environment           */
  LONG  calltype,                      /* How the program is called         */
  PRXSYSEXIT exits,                    /* Array of system exit names        */
  PSHORT retcode,                      /* Integer form of result            */
  PRXSTRING result)                    /* Result returned from program      */

{

  LONG     rc;                         /* RexxStart return code             */
// HANDLE   orexx_active_sem;

  WinBeginExceptions                   /* Begin of exception handling       */

  /* the current block is to implement a global counter of how many REXX programs
  are running on the system */
// I don't see why we should need this block and it causes trouble
//  orexx_active_sem = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "OBJECTREXX_RUNNING");
//  if (orexx_active_sem)
//     ReleaseSemaphore(orexx_active_sem,1,NULL);
//  else
//     orexx_active_sem = CreateSemaphore(NULL, 1, MAX_REXX_PROGS, "OBJECTREXX_RUNNING");

  RexxStartInfo RexxStartArguments;    /* entry argument information        */
  RexxObject *  tempArgument;          /* temporary argument item           */
  RexxObject *  resultObject;          /* dummy returned result             */

                                       /* copy all of the arguments into    */
                                       /* the info control block, which is  */
                                       /* passed across the kernel boundary */
                                       /* into the real RexxStart method    */
                                       /* this is a real execution          */
  RexxStartArguments.runtype = EXECUTE;
  RexxStartArguments.argcount = argcount;
  RexxStartArguments.arglist = arglist;
  RexxStartArguments.programname = (PSZ) programname;
  RexxStartArguments.instore = instore;
  RexxStartArguments.envname = (PSZ) envname;
  RexxStartArguments.calltype = (SHORT)calltype;
  RexxStartArguments.exits = exits;
  RexxStartArguments.retcode = retcode;
  RexxStartArguments.result = result;
  RexxStartArguments.outputName = NULL;


  /* Because of using the stand-alone runtime library or when using different compilers,
     the std-streams of the calling program and the REXX.DLL might be located at different
     addresses and therefore _file might be -1. If so, std-streams are reassigned to the
     file standard handles returned by the system */
  if ((stdin->_file == -1) && (GetFileType(GetStdHandle(STD_INPUT_HANDLE)) != FILE_TYPE_UNKNOWN))
      *stdin = *_fdopen(_open_osfhandle((LONG)GetStdHandle(STD_INPUT_HANDLE),_O_RDONLY), "r");
  if ((stdout->_file == -1) && (GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) != FILE_TYPE_UNKNOWN))
      *stdout = *_fdopen(_open_osfhandle((LONG)GetStdHandle(STD_OUTPUT_HANDLE),_O_APPEND), "a");
  if ((stderr->_file == -1) && (GetFileType(GetStdHandle(STD_ERROR_HANDLE)) != FILE_TYPE_UNKNOWN))
      *stderr = *_fdopen(_open_osfhandle((LONG)GetStdHandle(STD_ERROR_HANDLE),_O_APPEND), "a");

#ifdef SOM
                                       /* Perform any needed inits          */
  RexxSomInitialize((SOMObject *) NULL);
#else
  RexxInitialize();                    /* Perform any needed inits          */
#endif

  TheActivityClass->getActivity();     /* get a base activity under us      */
                                       /* wrap up the argument              */
  tempArgument = (RexxObject *)new_integer((LONG)&RexxStartArguments);
                                       /* pass along to the real method     */
  rc = CurrentActivity->messageSend(ProcessLocalServer, OREF_RUN_PROGRAM, 1, &tempArgument, &resultObject);
  TheActivityClass->returnActivity(CurrentActivity);
  RexxTerminate();                     /* perform needed termination        */

//  if (orexx_active_sem)
//  {
//     WaitForSingleObject(orexx_active_sem, INFINITE); /* decrease semaphore count */  /* @HOL003M  changed 0 to INFINITE */
//     CloseHandle( orexx_active_sem);
//  }

  WinEndExceptions                     /* End of Exception handling         */

  return -rc;                          /* return the error code (negated)   */
}

void CreateRexxCondData(
  RexxDirectory *conditionobj,         /* condition object                  */
  ConditionData *pRexxCondData)        /* returned condition data           */

{
  LONG  length;
  RexxString *message;
  RexxString *errortext;
  RexxString *program;

  memset(pRexxCondData,0,sizeof(ConditionData));
  pRexxCondData->code = message_number((RexxString *)conditionobj->at(OREF_CODE));

  pRexxCondData->rc = message_number((RexxString *)conditionobj->at(OREF_RC));

  message = (RexxString *)conditionobj->at(OREF_NAME_MESSAGE);
  if ( (RexxObject*) message != RexxNil) {
                                       /* get the length                    */
    length = message->length + 1;
                                       /* allocate a new RXSTRING buffer    */
    pRexxCondData->message.strptr = (char *)SysAllocateResultMemory(length);
                                       /* yes, copy the data (including the */
                                       /* terminating null implied by the   */
                                       /* use of length + 1                 */
    memcpy(pRexxCondData->message.strptr, message->stringData, length);
                                       /* give the true data length         */
    pRexxCondData->message.strlength = length - 1;
  }

  errortext = (RexxString *)conditionobj->at(OREF_ERRORTEXT);
                                       /* get the result length             */
  length = errortext->length +1;
                                       /* allocate a new RXSTRING buffer    */
  pRexxCondData->errortext.strptr = (char *)SysAllocateResultMemory(length);
                                       /* yes, copy the data (including the */
                                       /* terminating null implied by the   */
                                       /* use of length + 1                 */
  memcpy(pRexxCondData->errortext.strptr, errortext->stringData, length);
                                       /* give the true data length         */
  pRexxCondData->errortext.strlength = length - 1;

  pRexxCondData->position = ((RexxInteger *)(conditionobj->at(OREF_POSITION)))->value;

  program = (RexxString *)conditionobj->at(OREF_PROGRAM);
                                       /* get the result length             */
  length = program->length +1;
                                       /* allocate a new RXSTRING buffer    */
  pRexxCondData->program.strptr = (char *)SysAllocateResultMemory(length);
                                       /* yes, copy the data (including the */
                                       /* terminating null implied by the   */
                                       /* use of length + 1                 */
  memcpy(pRexxCondData->program.strptr, program->stringData, length);
                                       /* give the true data length         */
  pRexxCondData->program.strlength = length - 1;
}


/* functions for concurrency synchronization/termination */

void APIENTRY RexxWaitForTermination(void)
{
   EnterCriticalSection(&waitProtect);
   if (!RexxTerminated) {
     LeaveCriticalSection(&waitProtect);
     return;
   }
   EVWAIT(RexxTerminated);
   EVCLOSE(RexxTerminated);
   RexxTerminated = NULL;

   MTXCL(memoryObject.flattenMutex);
   MTXCL(memoryObject.unflattenMutex);
   MTXCL(memoryObject.envelopeMutex);
   // clear out semaphores!
   memoryObject.flattenMutex =
   memoryObject.unflattenMutex =
   memoryObject.envelopeMutex = 0;
   LeaveCriticalSection(&waitProtect);
}


APIRET APIENTRY RexxDidRexxTerminate(void)
{
   BOOL rc = TRUE;
   EnterCriticalSection(&waitProtect);
   if (!RexxTerminated) {
     LeaveCriticalSection(&waitProtect);
     return rc;
   }

   if (WaitForSingleObject(RexxTerminated, 0) == WAIT_OBJECT_0)
   {
       EVCLOSE(RexxTerminated);  /* Close semaphore if it is posted */
       RexxTerminated = NULL;

       /* commentary: see RexxWaitForTermination */
       MTXCL(memoryObject.flattenMutex);
       MTXCL(memoryObject.unflattenMutex);
       MTXCL(memoryObject.envelopeMutex);
       // clear out semaphores!
       memoryObject.flattenMutex =
       memoryObject.unflattenMutex =
       memoryObject.envelopeMutex = 0;

       //return TRUE;
   }
   else rc = FALSE;
   LeaveCriticalSection(&waitProtect);
   return rc;
}


BOOL APIENTRY RexxSetProcessMessages(BOOL onoff)
{
   BOOL old;
   old = UseMessageLoop;
   UseMessageLoop = onoff;
   return old;
}


/******************************************************************************/
/* Name:       RexxCreateMethod                                               */
/*                                                                            */
/* Arguments:  dirname - (Input) Name of directory object used for anchoring  */
/*                          method object.                                    */
/*             sourceData - (Input) Buffer with Rexx source code              */
/*             pmethod - (Output) pointer to new method object                */
/*             pRexxCondData (Output) Pointer to condition data               */
/*                                                                            */
/* Notes: Create a Rexx Method Object                                         */
/*                                                                            */
/* Returned:   rc - Return code from program                                  */
/*                                                                            */
/******************************************************************************/

APIRET REXXENTRY RexxCreateMethod(
  PCHAR dirname,                       /* directory name to save new method */
  PRXSTRING sourceData,                /* Buffer with Rexx source code      */
  RexxObject * *pmethod,               /* returned method object            */
  ConditionData *pRexxCondData)        /* returned condition data           */
{
  LONG     rc;                         /* RexxStart return code             */
  RexxScriptInfo RexxScriptArgs;
  RexxObject *  tempArgument;          /* temporary argument item           */
  RexxObject *  resultObject;          /* dummy returned result             */


                                       /* clear the RexxScriptArgs block    */
  memset((void *)&RexxScriptArgs, 0, sizeof(RexxScriptInfo));
  RexxScriptArgs.runtype = CREATEMETHOD;
                                       /* Create string object              */
  RexxScriptArgs.index = dirname;
  RexxScriptArgs.ProgramBuffer = sourceData;
  RexxScriptArgs.pmethod = (RexxMethod * *)pmethod;

#ifdef SOM
 #ifdef __cplusplus
  RexxSomInitialize((SOMObject *) NULL);  /* Perform any needed inits       */
 #else
  RexxSomInitialize((SOMAny *) NULL);  /* Perform any needed inits          */
 #endif
#else
  RexxInitialize();                    /* Perform any needed inits          */
#endif

  TheActivityClass->getActivity();     /* get a base activity under us      */
                                       /* wrap up the argument              */
  tempArgument = (RexxObject *)new_integer((LONG)&RexxScriptArgs);
                                       /* pass along to the real method     */
  rc = CurrentActivity->messageSend(ProcessLocalServer, OREF_RUN_PROGRAM, 1, &tempArgument, &resultObject);

                                       /* if error, get condition data from */
                                       /* condition object.                 */
  if (rc && CurrentActivity->conditionobj != OREF_NULL)
    CreateRexxCondData(CurrentActivity->conditionobj, pRexxCondData);
  TheActivityClass->returnActivity(CurrentActivity);
  RexxTerminate();                     /* perform needed termination        */
  return rc;                           /* return the error code             */
}

#pragma data_seg(".sdata")
 RexxObject* (__stdcall *WSHPropertyChange)(RexxString*,RexxObject*,int,int*) = NULL;

extern "C" {
void REXXENTRY SetNovalueCallback( RexxObject* (__stdcall *f)(void*) )
{
  NovalueCallback = f;
}
void REXXENTRY SetWSHPropertyChange( RexxObject* (__stdcall *f)(RexxString*,RexxObject*,int,int*) )
{
  WSHPropertyChange = f;
}
}

/******************************************************************************/
/* Name:       RexxRunMethod                                                  */
/*                                                                            */
/* Arguments:  dirname - (Input) Name of directory object used for anchoring  */
/*                          method object.                                    */
/*             method - (Input) Method object to run                          */
/*             args - (input) argument array (can be null)                    */
/*             f - (input) callback function (can be null)                    */
/*             exitlist - (input) exit list (can be null)                     */
/*             presult - (Output) Result object returned                      */
/*             pRexxCondData (Output) Pointer to condition data               */
/*                                                                            */
/* Notes: Run a REXX Method Object and return result object                   */
/*                                                                            */
/* Returned:   rc - Return code from program                                  */
/*                                                                            */
/******************************************************************************/
extern "C" {
APIRET REXXENTRY RexxRunMethod(
  PCHAR dirname,
  RexxObject * method,
  void * args,
  RexxArray* (__stdcall *f)(void*),
  PRXSYSEXIT exit_list,
  RexxObject * *presult,
  RexxObject *securityManager,
  ConditionData *pRexxCondData)        /* returned condition data           */
{
  LONG     rc;                         /* RexxStart return code             */
  RexxScriptInfo RexxScriptArgs;
  RexxObject *  tempArgument;          /* temporary argument item           */
  RexxObject *  resultObject;          /* dummy returned result             */
  RexxActivity *tempActivity;
  RexxActivity *store;

                                       /* clear the RexxScriptArgs block    */
  memset((void *)&RexxScriptArgs, 0, sizeof(RexxScriptInfo));

  RexxScriptArgs.runtype = RUNMETHOD;
                                       /* Create string object              */
  RexxScriptArgs.index = dirname;
  RexxScriptArgs.pmethod = (RexxMethod * *)&method;
  RexxScriptArgs.presult = presult;
  RexxScriptArgs.args = args;
  RexxScriptArgs.func = f;
  RexxScriptArgs.exits = exit_list;

#ifdef SOM
 #ifdef __cplusplus
                                       /* Perform any needed inits          */
  RexxSomInitialize((SOMObject *) NULL);
 #else
  RexxSomInitialize((SOMAny *) NULL);  /* Perform any needed inits          */
 #endif
#else
  RexxInitialize();                    /* Perform any needed inits          */
#endif

  if (securityManager) ((RexxMethod*) method)->setSecurityManager(securityManager);
  tempActivity = TheActivityClass->getActivity();     /* get a base activity under us      */
  store = RunActivity; // store old one
  RunActivity = tempActivity; // set to current
  tempActivity->exitObjects = TRUE;   // enable object passing thru classic rexx interface!
                                       /* wrap up the argument              */
  tempArgument = (RexxObject *)new_integer((LONG)&RexxScriptArgs);
                                       /* pass along to the real method     */
  rc = tempActivity->messageSend(ProcessLocalServer, OREF_RUN_PROGRAM, 1, &tempArgument, &resultObject);

                                       /* if error, get condition data from */
                                       /* condition object.                 */
  if (rc && tempActivity->conditionobj != OREF_NULL)
    CreateRexxCondData(tempActivity->conditionobj, pRexxCondData);
  TheActivityClass->returnActivity(tempActivity);
  RexxTerminate();                     /* perform needed termination        */
  RunActivity = store; // restore old RunActivity (NULL or from a nested call)
  return rc;                           /* return the error code             */
}
} // end extern "C"

/******************************************************************************/
/* Name:       RexxStoreMethod                                                */
/*                                                                            */
/* Arguments:  method - (Input) Method object to flatten                      */
/*             scriptData - (Output) RXSTRING containing flattened data       */
/*                                                                            */
/* Notes: Flatten a REXX method object                                        */
/*                                                                            */
/* Returned:   rc - Return code from program                                  */
/*                                                                            */
/******************************************************************************/

APIRET REXXENTRY RexxStoreMethod(RexxObject * method, PRXSTRING scriptData)

{
  LONG     rc;                         /* RexxStart return code             */
  RexxScriptInfo RexxScriptArgs;
  RexxObject *  tempArgument;          /* temporary argument item           */
  RexxObject *  resultObject;          /* dummy returned result             */

                                       /* clear the RexxScriptArgs block    */
  memset((void *)&RexxScriptArgs, 0, sizeof(RexxScriptInfo));
  RexxScriptArgs.runtype = STOREMETHOD;
  RexxScriptArgs.pmethod = (RexxMethod * *)&method;
  RexxScriptArgs.ProgramBuffer = scriptData;

#ifdef SOM
 #ifdef __cplusplus
  RexxSomInitialize((SOMObject *) NULL);  /* Perform any needed inits       */
 #else
  RexxSomInitialize((SOMAny *) NULL);  /* Perform any needed inits          */
 #endif
#else
  RexxInitialize();                    /* Perform any needed inits          */
#endif


  TheActivityClass->getActivity();     /* get a base activity under us      */
                                       /* wrap up the argument              */
  tempArgument = (RexxObject *)new_integer((LONG)&RexxScriptArgs);
                                       /* pass along to the real method     */
  rc = CurrentActivity->messageSend(ProcessLocalServer, OREF_RUN_PROGRAM, 1, &tempArgument, &resultObject);
  TheActivityClass->returnActivity(CurrentActivity);
  RexxTerminate();                     /* perform needed termination        */
  return rc;                           /* return the error code             */
}

/******************************************************************************/
/* Name:       RexxLoadMethod                                                 */
/*                                                                            */
/* Arguments:  dirname - (Input) Name of directory object used for anchoring  */
/*                          method object.                                    */
/*             scriptData - (Input) Buffer containing flattened code          */
/*             pmethod - (Output) pointer to new method object                */
/*                                                                            */
/* Notes: Unflatten a REXX method from a RXSTRING into a method object.       */
/*                                                                            */
/* Returned:   rc - Return code from program                                  */
/*                                                                            */
/******************************************************************************/

APIRET REXXENTRY RexxLoadMethod(PCHAR dirname, PRXSTRING scriptData, RexxObject * *pmethod)

{
  LONG     rc;                         /* RexxStart return code             */
  RexxScriptInfo RexxScriptArgs;
  RexxObject *  tempArgument;          /* temporary argument item           */
  RexxObject *  resultObject;          /* dummy returned result             */

                                       /* clear the RexxScriptArgs block    */
  memset((void *)&RexxScriptArgs, 0, sizeof(RexxScriptInfo));
  RexxScriptArgs.runtype = LOADMETHOD;
                                       /* Create string object              */
  RexxScriptArgs.index = dirname;
  RexxScriptArgs.ProgramBuffer = scriptData;
  RexxScriptArgs.pmethod = (RexxMethod * *)pmethod;

#ifdef SOM
 #ifdef __cplusplus
  RexxSomInitialize((SOMObject *) NULL);  /* Perform any needed inits       */
 #else
  RexxSomInitialize((SOMAny *) NULL);  /* Perform any needed inits          */
 #endif
#else
  RexxInitialize();                    /* Perform any needed inits          */
#endif

  TheActivityClass->getActivity();     /* get a base activity under us      */
                                       /* wrap up the argument              */
  tempArgument = (RexxObject *)new_integer((LONG)&RexxScriptArgs);
                                       /* pass along to the real method     */
  rc = CurrentActivity->messageSend(ProcessLocalServer, OREF_RUN_PROGRAM, 1, &tempArgument, &resultObject);
  TheActivityClass->returnActivity(CurrentActivity);

  RexxTerminate();                     /* perform needed termination        */

  if (*RexxScriptArgs.pmethod == OREF_NULL)
    rc = 1;                            /* scriptData must have been corrupt */

  return rc;                           /* return the error code             */

}

/******************************************************************************/
/* Name:       RexxTranslateProgram                                           */
/*                                                                            */
/* Arguments:  inFile   - Input source program                                */
/*             outFile  - Output file name                                    */
/*             exits    - Pointer to system exits                             */
/*                                                                            */
/******************************************************************************/
APIRET REXXENTRY RexxTranslateProgram(
   PSZ   inFile,                       /* input source program              */
   PSZ   outFile,                      /* output file name                  */
   PRXSYSEXIT   exits)                     /* system exits                  */
{
  LONG     rc;                         /* RexxStart return code             */
  RexxStartInfo RexxStartArguments;    /* entry argument information        */
  RexxObject *  tempArgument;          /* temporary argument item           */
  RexxObject *  resultObject;          /* dummy returned result             */

                                       /* clear the RexxStart block         */
  memset((void *)&RexxStartArguments, 0, sizeof(RexxStartInfo));
                                       /* program name is the input file    */
  RexxStartArguments.programname = inFile;
                                       /* and pass along the output name    */
  RexxStartArguments.outputName = outFile;
                                       /* this is a translation step        */
  RexxStartArguments.runtype = TRANSLATE;

  RexxStartArguments.exits = exits;

#ifdef SOM
                                       /* Perform any needed inits          */
  RexxSomInitialize((SOMObject *) NULL);
#else
  RexxInitialize();                    /* Perform any needed inits          */
#endif

  TheActivityClass->getActivity();     /* get a base activity under us      */
                                       /* wrap up the argument              */
  tempArgument = (RexxObject *)new_integer((LONG)&RexxStartArguments);
                                       /* pass along to the real method     */
  rc = CurrentActivity->messageSend(ProcessLocalServer, OREF_RUN_PROGRAM, 1, &tempArgument, &resultObject);
  TheActivityClass->returnActivity(CurrentActivity);
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
INT REXXENTRY RexxSetYield(PID procid, TID threadid)
{
//  threadid=1L;                              // until we can figure out tid
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
  APIRET res = RXARI_NOT_FOUND;
  INT i;

  if (RexxQuery()) {                        /* Are we up?                     */
     if (!threadid)
     {
#ifdef HIGHTID
        res = RXARI_OK;
        for (i=ProcessLocalActs->first();ProcessLocalActs->available(i);i=ProcessLocalActs->next(i))
        {
           /* get the threadid from the directory by casting index to a RexxString */
           /* and casting the RexxString to an integer (vers visa to ID2String */
           if(!activity_halt(ProcessLocalActs->index(i), OREF_NULL))  /* Set halt condition? */
              res = RXARI_NOT_FOUND;             /* Couldn't find threadid         */
        }
#else
        res = RXARI_NOT_FOUND;
#endif
     }
     else
     {
        if(!activity_halt(threadid, OREF_NULL))  /* Set halt condition?            */
           return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
        else
           return (RXARI_OK);
     }
  }
  else
     return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
  return (res);               /* REXX not running, error...     */
}




/******************************************************************************/
/* Name:       InternSetResetTrace                                            */
/*                                                                            */
/* Arguments:  procid - Process id of target REXX procedure                   */
/*             threadid - Thread id of target REXX procedure                  */
/*             flag - set trace on (1) or off (0) ?                           */
/*                                                                            */
/* Returned:   rc - RXARI_OK (halt condition set)                             */
/*                  RXARI_NOT_FOUND (couldn't find threadid)                  */
/*                                                                            */
/* Note: by IH to reuse code in RexxSetTrace and RexxResetTrace              */
/*                                                                            */
/******************************************************************************/

APIRET InternSetResetTrace(PID procid, TID threadid, BOOL flag)
{
  APIRET res = RXARI_NOT_FOUND;
  INT i;

  if (RexxQuery()) {                        /* Are we up?                     */
     if (!threadid)
     {
#ifdef HIGHTID
        res = RXARI_OK;
        for (i=ProcessLocalActs->first();ProcessLocalActs->available(i);i=ProcessLocalActs->next(i))
        {
           /* get the threadid from the directory by casting index to a RexxString */
           /* and casting the RexxString to an integer (vers visa to ID2String */
           if(!activity_set_trace(ProcessLocalActs->index(i), flag))  /* Set trace ? */
              res = RXARI_NOT_FOUND;             /* Couldn't find threadid         */
        }
#else
        res = RXARI_NOT_FOUND;
#endif
     }
     else
     {
        if(!activity_set_trace(threadid, flag))  /* Set trace on/off  ?            */
           return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
        else
           return (RXARI_OK);
     }
  }
  else
     return (RXARI_NOT_FOUND);             /* Couldn't find threadid         */
  return (res);               /* REXX not running, error...     */
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
  return (InternSetResetTrace(procid, threadid, 1));     /* 1 to set trace on */
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
  return (InternSetResetTrace(procid, threadid, 0));  /* 0 to set trace off */
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
  CHAR         name[CCHMAXPATH + 2];   /* temporary name buffer             */
  BOOL            fileFound;
  RexxActivity*activity;               /* the current activity              */

  activity = CurrentActivity;          /* save the current activity         */
  ReleaseKernelAccess(activity);       /* release the kernel access         */
                                       /* go resolve the name               */
  fileFound = SearchFileName(inputName->stringData, name);
  RequestKernelAccess(activity);       /* get the semaphore back            */

  if (!fileFound)
                                       /* got an error here                 */
    report_exception1(Error_Program_unreadable_notfound, inputName);

  fullName = new_cstring(name);        /* get as a string value             */
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

  if (instore[0].strptr == NULL && instore[1].strptr == NULL) {
                                       /* see if this exists                */
    if (!RexxQueryMacro(name->stringData, (PUSHORT)&temp)) {
                                       /* get image of function             */
      RexxExecuteMacroFunction(name->stringData, &buffer);
                                       /* go convert into a method          */
      method = SysRestoreProgramBuffer(&buffer, name);

      /* On Windows we need to free the allocated buffer for the macro */
      if (buffer.strptr) GlobalFree(buffer.strptr);
      return method;
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
                                       /* check for a compiled program      */
    method = SysRestoreInstoreProgram(&instore[0], name);
    if (method == OREF_NULL) {         /* not precompiled?                  */
                                       /* get a buffer object               */
      source_buffer = new_buffer(instore[0].strlength);
                                       /* copy source into the buffer       */
      memcpy(source_buffer->data, instore[0].strptr, instore[0].strlength);
                                       /* translate this source             */
      method = TheMethodClass->newRexxBuffer(name, source_buffer, (RexxClass *)TheNilObject);
                                       /* return this back in instore[1]    */
      SysSaveProgramBuffer(&instore[1], method);
    }
    return method;                     /* return translated source          */
  }
  return OREF_NULL;                    /* processing failed                 */
}

void CreateMethod(
  RexxScriptInfo *pRexxScriptArgs,
  RexxNativeActivation *newNativeAct)
{

 RexxBuffer *source_buffer;            /* buffered source                   */
 RexxString *name;                     /* input program name                */
 RexxDirectory *locked_objects;        /* directory used to keep objects    */
                                       /* around for process duration.      */
 RexxDirectory *OSAinfoDir;

 name = OREF_NULLSTRING;               /* use an "unlocatable" name         */
 newNativeAct->saveObject(name);      /* protect from garbage collect      */
                                       /* get a buffer object               */
 source_buffer = new_buffer(pRexxScriptArgs->ProgramBuffer->strlength);
                                       /* copy source into the buffer       */
 memcpy(source_buffer->data,
   pRexxScriptArgs->ProgramBuffer->strptr, pRexxScriptArgs->ProgramBuffer->strlength);
                                       /* translate this source             */
 *pRexxScriptArgs->pmethod = TheMethodClass->newRexxBuffer(name, source_buffer, (RexxClass *)TheNilObject);

 if (pRexxScriptArgs->index == NULLOBJ)
                                       /* protect from garbage collect      */
   newNativeAct->saveObject(*pRexxScriptArgs->pmethod);     /* protect from garbage collect      */
 else {
                                       /* Need to keep around for process   */
                                       /* duration.                         */
   locked_objects = (RexxDirectory *)ProcessLocalEnv->at(new_cstring(pRexxScriptArgs->index));
   locked_objects->put(*pRexxScriptArgs->pmethod, new_string((PCHAR)pRexxScriptArgs->pmethod, sizeof(LONG)));
 }
                                       /* Create directory object that will */
                                       /* contain OSAinfo.                  */
 OSAinfoDir = new_directory();
 ((RexxMethod *)(*pRexxScriptArgs->pmethod))->setInterface(OSAinfoDir);

                                       /* Hang source off of OSAinfo direct,*/
                                       /* so we have around for RexxGetSrc. */
                                       /* This may be tmp. need to talk to  */
                                       /* Rick about saving source object   */
                                       /* when flattening.                  */

 OSAinfoDir->put(new_string(pRexxScriptArgs->ProgramBuffer->strptr, pRexxScriptArgs->ProgramBuffer->strlength),
                  new_cstring("SOURCE"));

                                       /* finally, discard our activation   */
 CurrentActivity->pop(FALSE);
 return;
}

void RunMethod(
  RexxScriptInfo *pRexxScriptArgs,
  RexxNativeActivation *newNativeAct)
{
  RexxString *initial_address;         /* initial address setting           */
  RexxArray *new_arglist;              /* passed on argument list           */
  RexxActivity *savedAct;              /* store current activity */
  RexxDirectory *locked_objects;       /* directory used to keep objects    */
                                       /* around for process duration.      */
  ULONG argcount=1;
  ULONG arglength=0;
  PCHAR rexxargument="";
  PCHAR envname="CMD";                 /* ADDRESS environment name          */
  int i;                               // for exit installation
  RexxString *fullname;

  // callback activated?
  if (pRexxScriptArgs->func) {
    savedAct = CurrentActivity; // save the activity...
    // note: imho this is "dirty". a process global variable (CurrentActivity)
    // might (will) be overwritten, but to save the program flow (it will be
    // referenced again later on), i save it in a temporary variable...
    new_arglist = (RexxArray*) pRexxScriptArgs->func(pRexxScriptArgs->args);
    CurrentActivity = savedAct; // restore activity
  }
  else if (pRexxScriptArgs->args) {
    // func == NULL && args != NULL => treat as RexxArray;
    new_arglist = (RexxArray*) pRexxScriptArgs->args;
    newNativeAct->saveObject(new_arglist);

  }
  // use dummy argument array
  else {
                                         /* get a new argument array          */
    new_arglist = new_array(argcount);   /* lock the argument list            */
    newNativeAct->saveObject(new_arglist);
                                         /* add to the argument array         */
    new_arglist->put(new_string(rexxargument, arglength), 1);
  }

  initial_address = new_cstring(envname);
                                       /* protect from garbage collect      */
  newNativeAct->saveObject(initial_address);

  /* install exits */
  if (pRexxScriptArgs->exits != NULL) {/* have exits to process             */
    i = 0;                             /* start with first exit             */
                                       /* while not the list ender          */
    while (pRexxScriptArgs->exits[i].sysexit_code != RXENDLST) {
                                       /* convert to a string object        */
      fullname = new_cstring(pRexxScriptArgs->exits[i].sysexit_name);
                                       /* protect from garbage collect      */
      newNativeAct->saveObject(fullname);
                                       /* enable this exit                  */
      CurrentActivity->setSysExit(pRexxScriptArgs->exits[i].sysexit_code, fullname);
      i++;                             /* step to the next exit             */
    }
  }

                                       /* Check to see if halt or trace sys */
                                       /* were set                          */
  CurrentActivity->queryTrcHlt();
                                       /* run and get the result            */
  *pRexxScriptArgs->presult = (RexxString *)((RexxObject *)CurrentActivity)->shriekRun(*pRexxScriptArgs->pmethod, OREF_COMMAND, initial_address, new_arglist->data(), new_arglist->size());
  if (pRexxScriptArgs->index != OREF_NULL && *pRexxScriptArgs->presult != OREF_NULL) {
                                       /* Need to keep around for process   */
                                       /* duration.                         */
   locked_objects = (RexxDirectory *)ProcessLocalEnv->at(new_cstring(pRexxScriptArgs->index));
   locked_objects->put(*pRexxScriptArgs->presult, new_string((PCHAR)pRexxScriptArgs->presult, sizeof(LONG)));
 }

                                       /* finally, discard our activation   */
  CurrentActivity->pop(FALSE);
  return;
}

void LoadMethod(
  RexxScriptInfo *pRexxScriptArgs,
  RexxNativeActivation *newNativeAct)
{

 RexxString *name;                     /* input program name                */
 RexxDirectory *locked_objects;        /* directory used to keep objects    */
                                       /* around for process duration.      */

 name = OREF_NULLSTRING;               /* use an "unlocatable" name         */
                                       /* protect from garbage collect      */
 newNativeAct->saveObject(name);
 *pRexxScriptArgs->pmethod =
    SysRestoreProgramBuffer(pRexxScriptArgs->ProgramBuffer, name);

 if (pRexxScriptArgs->index != NULLOBJ && *pRexxScriptArgs->pmethod != OREF_NULL) {
                                       /* Need to keep around for process   */
                                       /* duration.                         */
   locked_objects = (RexxDirectory *)ProcessLocalEnv->at(new_cstring(pRexxScriptArgs->index));
   locked_objects->put(*pRexxScriptArgs->pmethod, new_string((PCHAR)pRexxScriptArgs->pmethod, sizeof(LONG)));
 }

                                       /* finally, discard our activation   */
 CurrentActivity->pop(FALSE);
 return;
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
  size_t        length;                /* return result length              */
  LONG          return_code;           /* converted return code info        */

  tokenize_only = FALSE;               /* default is to run the program     */
                                       /* create the native method to be run*/
                                       /* on the activity                   */
  newNativeAct = new ((RexxObject *)CurrentActivity, OREF_NULL, CurrentActivity, OREF_PROGRAM, OREF_NULL) RexxNativeActivation;
  CurrentActivity->push(newNativeAct); /* Push new nativeAct onto stack     */
  switch (*((PSHORT)ControlInfo)) {
    case CREATEMETHOD:
      CreateMethod((RexxScriptInfo *)ControlInfo, newNativeAct);
      return;
    case RUNMETHOD:
      RunMethod((RexxScriptInfo *)ControlInfo, newNativeAct);
      return;
    case STOREMETHOD:
      SysSaveProgramBuffer(((RexxScriptInfo *)ControlInfo)->ProgramBuffer,
                            *(((RexxScriptInfo *)ControlInfo)->pmethod));


                                       /* finally, discard our activation   */
      CurrentActivity->pop(FALSE);
      return;
    case LOADMETHOD:
      LoadMethod((RexxScriptInfo *)ControlInfo, newNativeAct);
      return;
    default:
      break;
  }
  self = (RexxStartInfo *)ControlInfo; /* address all of the arguments      */
  if (self->programname != NULL)       /* have an actual name?              */
                                       /* get string version of the name    */
    name = new_cstring(self->programname);
  else
    name = OREF_NULLSTRING;            /* use an "unlocatable" name         */
  newNativeAct->saveObject(name);      /* protect from garbage collect      */
  CurrentActivity->clearExits();       /* make sure the exits are cleared   */

  if (self->exits != NULL) {           /* have exits to process             */
      i = 0;                           /* start with first exit             */
                                       /* while not the list ender          */
      while (self->exits[i].sysexit_code != RXENDLST) {
                                       /* convert to a string object        */
        fullname = new_cstring(self->exits[i].sysexit_name);
                                       /* protect from garbage collect      */
        newNativeAct->saveObject(fullname);
                                       /* enable this exit                  */
        CurrentActivity->setSysExit(self->exits[i].sysexit_code, fullname);
        i++;                           /* step to the next exit             */
      }
    }

                                       /* get a new argument array          */
  if (self->runtype==TRANSLATE) {      /* just translating this?            */
                                       /* just do the translation step      */
    translateSource(name, newNativeAct, self->outputName);
    return;                            /* and finished                      */
  }

  new_arglist = new_array(self->argcount);
                                       /* lock the argument list            */
  newNativeAct->saveObject(new_arglist);
                                       /* loop through the argument list    */
  for (i = 0; i < self->argcount; i++) {
                                       /* have a real argument?             */
    if (self->arglist[i].strptr != NULL)
                                       /* add to the argument array         */
      new_arglist->put(new_string(self->arglist[i].strptr, self->arglist[i].strlength), i + 1);
  }
  if (self->calltype == RXCOMMAND) {   /* need to process command arg?      */
                                       /* is there an argument?             */
    /* also check self->argcount to be a not null number         */
    if (self->argcount != 0 && self->arglist != NULL &&
        self->arglist[0].strptr != NULL && self->arglist[0].strlength > 1) {
                                       /* is there a leading blank?         */
      if (*(self->arglist[0].strptr) == ' ')
                                       /* replace the first argument        */
        new_arglist->put(new_string(self->arglist[0].strptr+1, self->arglist[0].strlength - 1), 1);
                                       /* have a "//T" in the argument?     */
      if ( (((RexxString *)(new_arglist->get(1)))->caselessPos(OREF_TOKENIZE_ONLY, 0) !=
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
    program_result = (RexxString *)((RexxObject *)CurrentActivity)->shriekRun(method, source_calltype, initial_address, new_arglist->data(), new_arglist->size());
    if (self->result != NULL) {        /* if return provided for            */
                                       /* actually have a result to return? */
      if (program_result != OREF_NULL) {
                                       /* force to a string value           */
        program_result = program_result->stringValue();
                                       /* get the result length             */
        length = (LONG)program_result->length + 1;
                                       /* buffer too short or no return?    */
        if (length > self->result->strlength || self->result->strptr == NULL)
                                       /* allocate a new RXSTRING buffer    */
          self->result->strptr = (char *)SysAllocateResultMemory(length);
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


extern _declspec(dllexport) PCHAR RexxGetVersionInformation(void);

PCHAR RexxGetVersionInformation(void)
{
    char ver[20];
    sprintf( ver, " %d.%d.%d", ORX_VER, ORX_REL, ORX_MOD );
    char vbuf0[] = "Open Object Rexx %s Version";
  #ifdef _DEBUG
    char vbuf1[] = " - Internal Test Version\nBuild date: ";
  #else
    char vbuf1[] = "\nBuild date: ";
  #endif
    char vbuf2[] = "\nCopyright (c) IBM Corporation 1995, 2004.\nCopyright (c) RexxLA 2005-2007.\nAll Rights Reserved.";
    char vbuf3[] = "\nThis program and the accompanying materials";
    char vbuf4[] = "\nare made available under the terms of the Common Public License v1.0";
    char vbuf5[] = "\nwhich accompanies this distribution.";
    char vbuf6[] = "\nhttp://www.oorexx.org/license.html";
    INT s0 = strlen(vbuf0);
    INT s1 = strlen(vbuf1);
    INT s2 = strlen(vbuf2);
    INT s3 = strlen(vbuf3);
    INT s4 = strlen(vbuf4);
    INT s5 = strlen(vbuf5);
    INT s6 = strlen(vbuf6);
    INT sd = strlen(__DATE__);
    INT sv = strlen(ver);
    PCHAR ptr = (PCHAR) GlobalAlloc(GMEM_FIXED, sv+s0+s1+s2+s3+s4+s5+s6+sd+1);
    if (ptr)
    {
        memcpy(ptr, vbuf0, s0);
        memcpy(ptr+s0, ver, sv);
        memcpy(ptr+s0+sv, vbuf1, s1);
        memcpy(ptr+s0+sv+s1, __DATE__, sd);
        memcpy(ptr+s0+sv+s1+sd, vbuf2, s2);
        memcpy(ptr+s0+sv+s1+sd+s2, vbuf3, s3);
        memcpy(ptr+s0+sv+s1+sd+s2+s3, vbuf4, s4);
        memcpy(ptr+s0+sv+s1+sd+s2+s3+s4, vbuf5, s5);
        memcpy(ptr+s0+sv+s1+sd+s2+s3+s4+s5, vbuf6, s6+1);
    }
    return ptr;
}
