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
/* REXX Kernel                                                  RexxActivityc    */
/*                                                                            */
/* Primitive Activity Class                                                   */
/*                                                                            */
/* NOTE:  activities are an execution time only object.  They are never       */
/*        flattened or saved in the image, and hence will never be in old     */
/*        space.  Because of this, activities "cheat" and do not use          */
/*        OrefSet to assign values to get better performance.  Care must be   */
/*        used to maintain this situation.                                    */
/*                                                                            */
/******************************************************************************/
/* Change History                                                             */
/*                                                                            */
/* HOL001 - defect 217: out of stack using recursion 96/07/25                 */
/* HOL002 - feature 254 + 312:                            96/10/02 + 97/02/10 */
/*             Add a new system exit to ask for trace before execution        */
/* HOL003 - defect 269:                                             96/11/21  */
/*             Close all semaphores to allow RexxStart to be used iteratively */
/* CHM004 - feature 283:                                             01/16/97 */
/*          Remove pragma checkout from REXXSAA.H                             */
/* MAE005 - feature 317:                                             02/13/97 */
/*          Code restructuring for threading in OS/2, WIN, and AIX/LIN        */
/* WEI006 - defect 321:                                              02/27/97 */
/*          trap when executing windows concurrency tests                     */
/* HOL007 - feature 369:                                             97/06/26 */
/*          do thread management in sequentail table                          */
/* CHM008 - feature 391:                                             97/08/12 */
/*          Prepare source files for compact compile                          */
/* HOL009 - feature 399:                                             97/09/02 */
/*             A few performance improvements                                 */
/* HOL010 - feature 421:                                             97/10/21 */
/*             Dynamic savestack and static requires                          */
/* MAE011 - feature 387:                                             97/10/30 */
/*          Code restructering for AIX port of the Linux Code                 */
/*          --> rename variable conflicting with pthreads                     */
/* CHM012 - feature 427:                                             97/11/06 */
/*             Need a special semaphore for savestack manipulation            */
/* HOL013 - feature 519:   NLS Japan 2 + OPTIONS EXMODE              98/05/27 */
/* HOL014 - defect 531:                                              98/06/23 */
/*            Missing NULL ptr check in RexxActivityClass::addWaitingActivity */
/* CHM015 - defect 251:                                              98/09/16 */
/*             Drop and reuse of the same variable name                       */
/* HOL016 - defect 581:                                              98/10/13 */
/*             RXFNC and RegExternalFunction were limited to 20 arguments     */
/* HOL017 - defect 618:                                             98/12/09  */
/*          WB: Line still located although Trace Off was selected            */
/* MAE018 - defect 648:                                             99/02/19  */
/*          Linux: Signal on HALT kills the whole process                     */
/* CHM019 - defect 656:                                              99/03/16 */
/*         Performance degredation dropping many objects in one statement     */
/* THU020 - feature 752:                                             00/01/12 */
/*            memory leaks with the REXXSTART API                             */
/* THU021 - defect 866:                                              00/11/06 */
/*          RXSTRING initialization for Interactive debug trace exit          */
/* MIC022 - feature 871:                                             00/11/10 */
/*            Circumvention for GNU compiler 2.95.2 bug ( u_ added to name )  */
/* ENG023 - feature 892:                                             01/01/15 */
/*          object passing thru classical interface & extra exit              */
/* ENG024 - feature 900:                                             01/01/29 */
/*          improved random seed generation for activities (Windows)          */
/* ENG025 - feature 906:                                             01/02/19 */
/*          change in extra exit (engine call)                                */
/* ENG026 - defect 915:                                              01/03/19 */
/*          access violation when using stream funcs                          */
/* ENG027 - defect 1033:                                             02/01/21 */
/*          exits are deinstalled before some UNINITs of activity run         */
/* MIC028 - feature 1042:                                            02/02/11 */
/*          Changes for SUN/Solaris                                           */
/* ENG029 - defect 1063:                                             02/04/04 */
/*          UNINIT bug (memory leak) for subclasses of ARRAY with UNINIT meth.*/
/* THU030 - defect 1067:                                             02/04/19 */
/*          Thread exhausted when calling REXX from API several times         */
/* ENG031 - feature 1068:                                            02/04/24 */
/*          removal of compiler warnings                                      */
/* ENG032 - defect 1069:                                             02/04/26 */
/*          fix ANSI-C violation with va_arg()                                */
/* ENG033 - feature 1079:                                            02/06/12 */
/*          RXINI: temporarily provide script name in variable of var. pool   */
/* MIC034 - feature 1085:                                            02/07/01 */
/*            DRIVER build updates                                            */
/* ENG035 - defect 1126:                                             02/09/27 */
/*          GC protection rework (see changes for feature 892)                */
/*                                                                            */
#include <setjmp.h>
#include <stdlib.h>
#include <ctype.h>
#if defined(OPSYS_SUN)
#include <sched.h>
#endif
#include "RexxCore.h"
#include "StringClass.hpp"
#include "StackClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "RexxNativeActivation.hpp"
#include "MessageClass.hpp"
#include "ArrayClass.hpp"
#include "TableClass.hpp"
#include "DirectoryClass.hpp"
#include "SourceFile.hpp"
#include "RexxVariableDictionary.hpp"
#include "RexxCode.hpp"
#include "RexxInstruction.hpp"
#include "ActivityTable.hpp"
#include "RexxMemory.hpp"
#include "RexxVariableDictionary.hpp"
#define INCL_RXSYSEXIT
#define INCL_RXOBJECT
#include SYSREXXSAA
                                       /* Local and global memory Pools     */
                                       /*  last one accessed.               */
extern MemorySegmentPool *ProcessCurrentPool;
extern MemorySegmentPool *GlobalCurrentPool;
#define ACT_STACK_SIZE 10
#define ACTIVATION_CACHE_SIZE 5
//#define MIN_C_STACK 1024*16
//#define TOTAL_STACK_SIZE 1024*512
//#define C_STACK_SIZE 60000

extern RexxObject * ProcessLocalServer;
extern int  ProcessNumActs;            /* number of active activities       */
extern BOOL ProcessTerminating;
extern RexxDirectory *ProcessLocalEnv;
extern RexxInteger * ProcessName;      /* processName object, one/Process   */
                                       /* Localacts, is an array of all     */
                                       /* activites per process, ie. there's*/
                                       /* one localacts var for each process*/
                                       /* The threadid for each activity    */
                                       /* is the index into this array      */
#ifdef HIGHTID
extern ActivityTable *ProcessLocalActs;
#else
extern RexxArray *ProcessLocalActs;
#endif

extern SMTX rexx_kernel_semaphore;     /* global kernel semaphore           */
extern SMTX rexx_resource_semaphore;   /* global kernel semaphore           */
extern SMTX rexx_start_semaphore;      /* startup semaphore                 */
extern BOOL SysDBCSSetup(PULONG, PUCHAR);

                                       /* current active activity           */
//RexxActivity *CurrentActivity = OREF_NULL;
                                       /* generic default settings structure*/
ACTIVATION_SETTINGS defaultSettings;
                                       /* default active settings           */
ACTIVATION_SETTINGS *current_settings = &defaultSettings;


#ifdef SOM
  #include "orxsminf.xh"
  #include "repostry.xh"
  #include "somcls.xh"
  #include "orxsom.h"                  /* SOM client declarations */

  OREF resolve_proxy(SOMObject *);
#endif

#ifdef HIGHTID_0
//#define ID2String(id, s) new_cstring(itoa(id,(char *)&s,10))
#define ID2String(id) new_string((char *)&id, sizeof(LONG))
/* make fastAt in activity_find() faster if there's only one thread */
static LONG HighTidLastID = 0;
static RexxActivity * HighTidLastActivity = NULL;
#endif
ULONG thrdCount = 0;

                                       /* allow to be called from C */
extern "C" void activity_thread (RexxActivity *objp);
void kernelShutdown (void);            /* system shutdown routine           */

void activity_thread (
  RexxActivity *objp)                  /* activity assciated with a thread  */
/******************************************************************************/
/* Function:  Main "entry" routine for a newly created thread                 */
/*                                                                            */
/* Remark:    For AIX the pthread_exit is called whenever a thread terminates.*/
/*            This should clear storage.                                      */
/*                                                                            */
/******************************************************************************/
{
  int  jmprc;                          /* setjmp return code                */
  LONG number_activities;              /* count of activities               */

  SYSEXCEPTIONBLOCK exreg;             /* system specific exception info    */

  SysInitializeThread();               /* system specific thread init       */
                                       /* establish the stack base pointer  */
  objp->nestedInfo.stackptr = SysGetThreadStackBase(TOTAL_STACK_SIZE);
  SysRegisterExceptions(&exreg);       /* create needed exception handlers  */
                                       /* Do any initialization for Windowin*/
                                       /*  That may be necessary.           */
  objp->windowInfo = SysInitializeWindowEnv();
                                       /* establish the longjmp return point*/
  jmprc = setjmp(objp->nestedInfo.jmpenv);
  for (;;) {
    if (jmprc == 0) {                  /* if no error                       */
      EVWAIT(objp->runsem);            /* wait for run permission           */
      if (objp->exit)                  /* told to exit?                     */
        break;                         /* we're out of here                 */
                                       /* set our priority appropriately    */
#ifdef THREADHANDLE
      SysSetThreadPriority(objp->threadid, objp->hThread, objp->priority);
#else
      SysSetThreadPriority(objp->threadid, objp->priority);
#endif

      RequestKernelAccess(objp);       /* now get the kernel lock           */
                                       /* get the top activation            */
      objp->topActivation->dispatch(); /* go dispatch it                    */
    }                                  /* had an error return               */
    else {
      jmprc = 0;                       /* reset the jmp point               */
                                       /* do error cleanup, clean all       */
                                       /*activiations, this is top activity */
      objp->error(0);
    }

    TheActivityClass->runUninits();    /* run any needed UNINIT methods now */

    EVSET(objp->runsem);               /* reset the run semaphore and the   */
    EVSET(objp->guardsem);             /* guard semaphore                   */

    if (!ProcessTerminating) {         /* Are we terminating?               */
       SysEnterResourceSection();      /* now in a critical section         */
                                       /* add this as a free activity       */
       TheActivityClass->classFreeActivities->add((RexxObject *)objp, ProcessName);
       SysExitResourceSection();       /* end of the critical section       */
    }
    ReleaseKernelAccess(objp);         /* release the kernel lock           */
    if (ProcessTerminating) {          /* Are we terminating?               */
      break;                           /* yup, go do termination stuff      */
    }
  }
  RequestKernelAccess(objp);           /* get the kernel access             */

                                       /* Cleanup any window resources      */
  SysTerminateWindowEnv(objp->windowInfo);
  objp->windowInfo = NULL;             /* Clear out the SysWindowINfo Ptr   */
  SysDeregisterExceptions(&exreg);     /* remove exception trapping         */
  SysEnterResourceSection();           /* now in a critical section         */
  number_activities = --ProcessNumActs;/* get the current activity count    */
  SysExitResourceSection();            /* end of the critical section       */
  if (number_activities == 0)          /* are we the last thread?           */
    objp->checkUninits();              /* process uninits                   */

  SysTerminateThread((TID)objp->threadid);  /* system specific thread termination*/
  thrdCount--;
  SysEnterResourceSection();           /* now in a critical section         */
  if (ProcessTerminating)
  {
      if (TheActivityClass->classFreeActivities->hasItem((RexxObject *)objp, ProcessName) != OREF_NULL)
          TheActivityClass->classFreeActivities->removeItem((RexxObject *)objp, ProcessName);
      EVCLOSE(objp->runsem);
      EVCLOSE(objp->guardsem);
#ifdef THREADHANDLE
      EVCLOSE(objp->hThread);
#endif
      ProcessLocalActs->put(OREF_NULL, objp->threadid);
  }
                                       /* remove this activity from usage   */
  TheActivityClass->classUsedActivities->remove((RexxObject *)objp);
  SysExitResourceSection();            /* end of the critical section       */
  ReleaseKernelAccess(objp);           /* and release the kernel lock       */
                                       /* Are we terminating?               */

  if (ProcessTerminating && number_activities == 0)
    kernelShutdown();                  /* time to shut things down          */

return;                              /* finished                          */
}

void *RexxActivity::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new activity                                           */
/******************************************************************************/
{
   RexxActivity  *newActivity;

                                       /* get the new activity storage      */
   newActivity  = (RexxActivity *)new_object(size);
                                       /* Give new object its behaviour     */
   BehaviourSet(newActivity, TheActivityBehaviour);
   return newActivity;                 /* and return it                     */
}

RexxActivity::RexxActivity(
    BOOL recycle,                      /* activity is being reused          */
    long priority,                     /* activity priority                 */
    RexxDirectory *local)              /* process local directory           */
/******************************************************************************/
/* Function:  Initialize an activity object                                   */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
  if (!recycle) {                      /* if this is the first time         */
    ClearObject(this);                 /* globally clear the object         */
    this->hashvalue = HASHOREF(this);  /* set the hash value                */
    this->local = local;               /* set the local environment         */
                                       /* Set ProcessName now, before       */
    this->processObj = ProcessName;    /* any more allocations.             */
                                       /* create an activation stack        */
    this->activations = new_internalstack(ACT_STACK_SIZE);
    this->frameStack.init();           /* initialize the frame stack        */
    EVCR(this->runsem);                /* create the run and                */
    EVCR(this->guardsem);              /* guard semaphores                  */
    this->size = ACT_STACK_SIZE;       /* set the activation stack size     */
    this->stackcheck = TRUE;           /* start with stack checking enabled */
                                       /* use default settings set          */
    this->exitObjects = FALSE;         // ENG023A: behaviour of exits: classic REXX
    this->default_settings = defaultSettings;
                                       /* set up current usage set          */
    this->settings = &this->default_settings;
                                       /* get the DBCS info                 */
    this->DBCS_codepage = SysDBCSSetup(&this->codepage, this->DBCS_table);
                                       /* copy the DBCS info                */
    this->settings->DBCS_codepage = this->DBCS_codepage;
    this->settings->codepage = this->codepage;
    this->settings->DBCS_table = this->DBCS_table;

//#ifdef FIXEDTIMERS
//    SysStartTimeSlice();   /* Start a new timeSlice  */
//#endif

    if (priority != NO_THREAD) {       /* need to create a thread?          */
#ifdef FIXEDTIMERS
          /* start the control thread the first time a concurrent thread is used */
      SysStartTimeSlice();   /* Start a new timeSlice                       */
#endif

      EVSET(this->runsem);             /* set the run semaphore             */
                                       /* create a thread                   */
      this->threadid = SysCreateThread((PTHREADFN)activity_thread,C_STACK_SIZE,this);
      SysEnterResourceSection();
      memoryObject.extendSaveStack(++thrdCount);
      SysExitResourceSection();
      this->priority = priority;       /* and the priority                  */
    }
    else {                             /* thread already exists             */
                                       /* query the thread id               */
      this->threadid = SysQueryThreadID();
#ifdef THREADHANDLE
      this->hThread = SysQueryThread();
#endif
#ifdef NEWGUARD
      this->priority = MEDIUM_PRIORITY+10;/* switch to medium priority +1  */
   #ifdef THREADHANDLE
      SysSetThreadPriority(this->threadid, this->hThread, this->priority);
   #else
      SysSetThreadPriority(this->threadid, objp->priority);
   #endif
#else
      this->priority = MEDIUM_PRIORITY;/* switch to medium priority         */
#endif
                                       /* establish the stack base pointer  */
      this->nestedInfo.stackptr = SysGetThreadStackBase(TOTAL_STACK_SIZE);
    }
    this->generateRandomNumberSeed();  /* get a fresh random seed           */
                                       /* Create table for progream being   */
    this->requiresTable = new_table(); /*installed vial ::REQUIRES          */
                                       /* clear out the top activation      */
    this->topActivation = (RexxActivationBase *)TheNilObject;
                                       /* and the current REXX activation   */
    this->currentActivation = (RexxActivation *)TheNilObject;

  }                                    /* recycling                         */
  else {
    this->priority = priority;         /* just set the priority             */
                                       /* Make sure any left over           */
                                       /* ::REQUIRES is cleared out.        */
    this->resetRunningRequires();
  }
}

void RexxActivity::generateRandomNumberSeed()
/******************************************************************************/
/* Function:  Generate a fresh random number seed.                            */
/******************************************************************************/
{
  RexxDateTime  timestamp;             /* current timestamp                 */
  LONG          i;                     /* loop counter                      */
  static int rnd = 0;

  rnd++;
  SysGetCurrentTime(&timestamp);       /* get a fresh time stamp            */
                                       /* take the seed from the time       */
  this->nestedInfo.randomSeed = rnd + (((timestamp.hours * 60 + timestamp.minutes) * 60 + timestamp.seconds) * 1000) + timestamp.microseconds/1000;
  for (i = 0; i < 13; i++) {           /* randomize the seed number a bit   */
                                       /* scramble the seed a bit           */
    this->nestedInfo.randomSeed = RANDOMIZE(this->nestedInfo.randomSeed);
  }
}

long RexxActivity::error(size_t startDepth)
/******************************************************************************/
/* Function:  Force error termination on an activity, returning the resulting */
/*            REXX error code.                                                */
/******************************************************************************/
{
  LONG   rc;                           /* REXX error return code            */

  while (this->depth > startDepth) {   /* while still have activations      */
                                       /* if we have a real activation      */
    if ((RexxObject *)(this->topActivation) != TheNilObject) {
                                       /* force activation termination      */
        this->topActivation->termination();
    }
    this->pop(FALSE);                  /* pop the activation off            */
  }
  rc = Error_Interpretation/1000;      /* set default return code           */
                                       /* did we get a condtion object?     */
  if (this->conditionobj != OREF_NULL) {
                                       /* force it to display               */
    this->display(this->conditionobj);
                                       /* get the failure return code       */
    rc = this->conditionobj->at(OREF_RC)->longValue(DEFAULT_DIGITS);
  }
  return rc;                           /* return the error code             */
}

BOOL RexxActivity::raiseCondition(
    RexxString    *condition,          /* condition to raise                */
    RexxObject    *rc,                 /* return code value                 */
    RexxString    *description,        /* description information           */
    RexxObject    *additional,         /* additional information            */
    RexxObject    *result,             /* result value                      */
    RexxDirectory *exobj)              /* exception object                  */
/******************************************************************************/
/* Function:   Raise an actual condition, causing termination for untrapped   */
/*             conditions                                                     */
/******************************************************************************/
{
  RexxActivationBase *activation;      /* current activation                */
  BOOL                handled;         /* this condition has been handled   */
  RexxDirectory      *conditionobj;    /* object for created condition      */


  handled = FALSE;                     /* condition not handled yet         */
  if (exobj == OREF_NULL) {            /* need to create a condition object?*/
    conditionobj = new_directory();    /* get a new directory               */
                                       /* put in the condition name         */
    conditionobj->put(condition, OREF_CONDITION);
                                       /* fill in default description       */
    conditionobj->put(OREF_NULLSTRING, OREF_DESCRIPTION);
                                       /* fill in the propagation status    */
    conditionobj->put(TheFalseObject, OREF_PROPAGATED);
  }
  else
    conditionobj = exobj;              /* use the existing object           */
  if (rc != OREF_NULL)                 /* have an RC value?                 */
    conditionobj->put(rc, OREF_RC);    /* add to the condition argument     */
  if (description != OREF_NULL)        /* any description to add?           */
    conditionobj->put(description, OREF_DESCRIPTION);
  if (additional != OREF_NULL)         /* or additional information         */
    conditionobj->put(additional, OREF_ADDITIONAL);
  if (result != OREF_NULL)             /* given a return result?            */
    conditionobj->put(result, OREF_RESULT);

                                       /* invoke the error traps, on all    */
                                       /*  nativeacts until reach 1st       */
                                       /*  also give 1st activation a shot. */
  for (activation = this->current() ; activation != (RexxActivation *)TheNilObject; activation = this->sender(activation)) {
    handled = activation->trap(condition, conditionobj);
    if (OTYPE(Activation, activation)) /* reached our 1st activation yet.   */
      break;                           /* yes, break out of loop            */
  }

  /* Control will not return here if the condition was trapped via*/
  /* SIGNAL ON SYNTAX.  For CALL ON conditions, handled will be   */
  /* TRUE if a trap is pending.                                   */

  return handled;                      /* this has been handled             */
}

void RexxActivity::reportAnException(
    LONG errcode )                     /* REXX error code                   */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
                                       /* send along with nothing           */
  this->raiseException(errcode, NULL, OREF_NULL, OREF_NULL, OREF_NULL, OREF_NULL);
}

void RexxActivity::reportAnException(
    LONG errcode,                      /* REXX error code                   */
    RexxObject *substitution1 )        /* substitution information          */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
  this->raiseException(errcode, NULL, OREF_NULL, OREF_NULL, new_array1(substitution1), OREF_NULL);
}

void RexxActivity::reportAnException(
    LONG errcode,                      /* REXX error code                   */
    RexxObject *substitution1,         /* substitution information          */
    RexxObject *substitution2 )        /* substitution information          */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
  this->raiseException(errcode, NULL, OREF_NULL, OREF_NULL, new_array2(substitution1, substitution2), OREF_NULL);
}

void RexxActivity::reportAnException(
    LONG errcode,                      /* REXX error code                   */
    RexxObject *substitution1,         /* substitution information          */
    RexxObject *substitution2,         /* substitution information          */
    RexxObject *substitution3 )        /* substitution information          */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
  this->raiseException(errcode, NULL, OREF_NULL, OREF_NULL, new_array3(substitution1, substitution2, substitution3), OREF_NULL);
}

void RexxActivity::reportAnException(
    LONG errcode,                      /* REXX error code                   */
    RexxObject *substitution1,         /* substitution information          */
    RexxObject *substitution2,         /* substitution information          */
    RexxObject *substitution3,         /* substitution information          */
    RexxObject *substitution4 )        /* substitution information          */
/******************************************************************************/
/* Function:  Forward on an exception condition                               */
/******************************************************************************/
{
  this->raiseException(errcode, NULL, OREF_NULL, OREF_NULL, new_array4(substitution1, substitution2, substitution3, substitution4), OREF_NULL);
}

void RexxActivity::reportException(
    LONG           errcode,            /* REXX error code                   */
    PCHAR          string )            /* single string sustitution parm    */
/******************************************************************************/
/* Function:  Raise an error using a single REXX character string only        */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  report_exception1(errcode, new_cstring(string));
}

void RexxActivity::reportException(
    LONG           errcode,            /* REXX error code                   */
    LONG           integer )           /* single integer substitution parm  */
/******************************************************************************/
/* Function:  Raise an error using a single REXX integer value only           */
/*            as a substitution parameter                                     */
/******************************************************************************/
{
                                       /* convert and forward along         */
  report_exception1(errcode, new_integer(integer));
}

void RexxActivity::raiseException(
    LONG           errcode,            /* REXX error code                   */
    LOCATIONINFO  *location,           /* location information              */
    RexxSource    *source,             /* source file to process            */
    RexxString    *description,        /* descriptive information           */
    RexxArray     *additional,         /* substitution information          */
    RexxObject    *result )            /* result information                */
/******************************************************************************/
/* This routine is used for SYNTAX conditions only.                           */
/*                                                                            */
/* The inserts are passed as objects because that happens to be more          */
/* convenient for the calling code in the two cases where this facility       */
/* is used.                                                                   */
/*                                                                            */
/* NOTE: The building of the excepption obejct (EXOBJ)  has been re-arranged  */
/*  so that a garbage collection in the middle of building traceBack/etc      */
/*  doesn't clean up the newly built objects.  SO we create exobj early on    */
/*  save it, and add newlly built objects to exobj as we go.                  */
/******************************************************************************/
{
  RexxDirectory   *exobj;              /* exception object                  */
  RexxList        *traceback;          /* traceback list                    */
  RexxInteger     *position;           /* exception source line             */
  RexxString      *programname;        /* current program name              */
  RexxInteger     *rc;                 /* integer return code               */
  RexxString      *code;               /* error code in decimal form        */
  RexxActivation  *activation;         /* current activation                */
  RexxActivation  *poppedActivation;   /* activation popped from the stack  */
  RexxString      *errortext;          /* primary error message             */
  RexxString      *message;            /* secondary error message           */
  LONG             primary;            /* primary message code              */
  CHAR             work[10];           /* temp buffer for formatting        */
  LONG             newVal;

  if (this->requestingString)          /* recursive entry to error handler? */
    longjmp(this->stringError, 1);     /* just jump back                    */

  activation = this->currentAct();     /* get the current activation        */
  while (OTYPE(Activation, activation) && activation->isForwarded()) {
    activation->termination();         /* do activation termiantion process */
    this->pop(FALSE);                  /* pop the top activation off        */
    activation = this->currentAct();   /* and get the new current one       */
  }
  primary = (errcode / 1000) * 1000;   /* get the primary message number    */
                                       /* format the number (string) into   */
                                       /*  work buffer.                     */
  sprintf(work,"%d.%1d", errcode/1000, errcode - primary);
  code = new_cstring(work);            /* get the formatted code            */
  newVal = primary/1000;
  rc = new_integer(newVal);            /* get the primary message number    */
                                       /* get the primary message text      */
  errortext = SysMessageText(primary);
  if (errortext == OREF_NULL)          /* no corresponding message          */
                                       /* this is an error                  */
    report_exception1(Error_Execution_error_condition, code);
  if (primary != errcode) {            /* have a secondary message to issue?*/
                                       /* retrieve the secondary message    */
    message = SysMessageText(errcode);
    if (message == OREF_NULL)          /* no corresponding message          */
                                       /* this is an error                  */
      report_exception1(Error_Execution_error_condition, code);
  }
  else
                                       /* don't give a secondary message    */
    message = (RexxString *)TheNilObject;

                                       /* All error detection done. we can  */
                                       /*  build and save exobj now.        */
                                       /* get an exception directory        */
  exobj = (RexxDirectory *)new_directory();
  this->conditionobj = exobj;          /* save this in the activity         */
  exobj->put(rc, OREF_RC);             /* add the return code               */
                                       /* fill in the decimal error code    */
  exobj->put(code, OREF_CODE);
  exobj->put(errortext, OREF_ERRORTEXT);
  exobj->put(message, OREF_NAME_MESSAGE);

  if (additional == OREF_NULL)         /* no description given?             */
                                       /* use a zero size array             */
    additional = (RexxArray *)TheNullArray->copy();
  if (description == OREF_NULL)        /* no description?                   */
    description = OREF_NULLSTRING;     /* use a null string                 */

                                       /* fill in the arguments             */
                                       /* fill in the arguments             */
  if (description != OREF_NULL)
  {
      exobj->put(description, OREF_DESCRIPTION);
  }
  if (additional != OREF_NULL)
  {
      exobj->put(additional, OREF_ADDITIONAL);
  }
  if (source != OREF_NULL)
  {
      exobj->put((RexxObject *)source, OREF_SOURCENAME);
  }
  if (result != OREF_NULL)
  {
      exobj->put(result, OREF_RESULT);
  }

  traceback = OREF_NULL;               /* no traceback info                 */

  if (location != NULL) {              /* have clause information available?*/
                                       /* add the line number information   */
    newVal = location->line;
    position = new_integer(newVal);
  }
                                       /* have an activation?               */
  else if (activation != (RexxActivation *)TheNilObject) {
                                       /* get the activation position       */
    newVal = activation->currentLine();
    position = new_integer(newVal);
  }
  else
    position = OREF_NULL;              /* no position information available */
  exobj->put(position, OREF_POSITION); /* add position to exobj.            */

  traceback = new_list();              /* create a traceback list           */
                                       /* add to the exception object       */
  exobj->put(traceback, OREF_TRACEBACK);
  if (source != OREF_NULL)             /* have source for this?             */
                                       /* extract and add clause in error   */
    traceback->addLast(source->traceBack(location, 0, FALSE));
                                       /* have predecessors?                */
  if (activation != (RexxActivation *)TheNilObject)
    activation->traceBack(traceback);  /* have them add lines to the list   */
  if (source != OREF_NULL)             /* have source for this?             */
    programname = source->programName; /* extract program name              */
                                       /* have predecessors?                */
  else if (activation != (RexxActivation *)TheNilObject)
                                       /* extract program name from activa. */
    programname = activation->programName();
  else
    programname = OREF_NULLSTRING;     /* have no program name              */
  exobj->put(programname, OREF_PROGRAM);
  exobj->put(OREF_SYNTAX, OREF_CONDITION);
                                       /* fill in the propagation status    */
  exobj->put(TheFalseObject, OREF_PROPAGATED);

  if (message != TheNilObject) {       /* have a secondary message?         */
                                       /* do required substitutions         */
    message = this->messageSubstitution(message, additional);
                                       /* replace the original message text */
    exobj->put(message, OREF_NAME_MESSAGE);
  }
                                       /* process as common condition       */
  if (!this->raiseCondition(OREF_SYNTAX, OREF_NULL, OREF_NULL, OREF_NULL, OREF_NULL, exobj)) {
                                       /* fill in the propagation status    */
    exobj->put(TheTrueObject, OREF_PROPAGATED);
                                       /* unwind the activation stack       */
    while ((poppedActivation = (RexxActivation *)this->current()) != activation) {
      poppedActivation->termination(); /* do activation termiantion process */
      this->pop(FALSE);                /* pop the activation off the stack  */
    }

        if ((activation != (RexxActivation *)TheNilObject) &&
                (activation->settings.traceindent > MAX_TRACEBACK_LIST))
                traceback->addLast(new_cstring("     >...<"));

                                       /* actually have an activation?      */
    if (activation != (RexxActivation *)TheNilObject) {
      activation->termination();       /* do activation termiantion process */
      this->pop(FALSE);                /* pop the top activation off        */
    }
    this->raisePropagate(exobj);       /* pass on down the chain            */
  }
}

RexxString *RexxActivity::messageSubstitution(
    RexxString *message,               /* REXX error message                */
    RexxArray  *additional )           /* substitution information          */
/******************************************************************************/
/* Function:  Perform any required message substitutions on the secondary     */
/*            error message.                                                  */
/******************************************************************************/
{
  LONG        substitutions;           /* number of substitutions           */
  LONG        subposition;             /* substitution position             */
  LONG        i;                       /* loop counter                      */
  LONG        selector;                /* substitution position             */
  RexxString *newmessage;              /* resulting new error message       */
  RexxString *front;                   /* front message part                */
  RexxString *back;                    /* back message part                 */
  RexxObject *value;                   /* substituted message value         */
  RexxString *stringValue;             /* converted substitution value      */
  BOOL       isDBCS;

  substitutions = additional->size();  /* get the substitution count        */
  newmessage = OREF_NULLSTRING;        /* start with a null string          */
                                       /* loop through and substitute values*/
  for (i = 1; i <= substitutions; i++) {
                                       /* search for a substitution         */
    isDBCS = current_settings->exmode; /* save EXMODE setting */
    current_settings->exmode = FALSE;  /* don't use DBCSpos */
    subposition = message->pos(OREF_AND, 0);
    current_settings->exmode = isDBCS; /* restore EXMODE setting */
    if (subposition == 0)              /* not found?                        */
      break;                           /* get outta here...                 */
                                       /* get the leading part              */
    front = message->extract(0, subposition - 1);
                                       /* pull off the remainder            */
    back = message->extract(subposition + 1, message->length - (subposition + 1));
                                       /* get the descriptor position       */
    selector = message->getChar(subposition);
                                       /* not a good number?                */
    if (selector < '0' || selector > '9')
                                       /* use a default message             */
      stringValue = new_cstring("<BAD MESSAGE>"); /* must be stringValue, not value, otherwise trap */
    else {
      selector -= '0';                 /* convert to a number               */
      if (selector > substitutions)    /* out of our range?                 */
        stringValue = OREF_NULLSTRING; /* use a null string                 */
      else {                           /* get the indicated selector value  */
        value = additional->get(selector);
        if (value != OREF_NULL) {      /* have a value?                     */
                                       /* set the reentry flag              */
          this->requestingString = TRUE;
          this->stackcheck = FALSE;    /* disable the checking              */
                                       /* now protect against reentry       */
          if (setjmp(this->stringError) == 0)
                                       /* force to character form           */
            stringValue = value->stringValue();
          else                         /* bad string method, use default    */
            stringValue = value->defaultName();
                                       /* we're safe again                  */
          this->requestingString = FALSE;
          this->stackcheck = TRUE;     /* disable the checking              */
        }
        else
                                       /* use a null string                 */
          stringValue = OREF_NULLSTRING;
      }
    }
                                       /* accumulate the front part         */
    newmessage = newmessage->concat(front->concat(stringValue));
    message = back;                    /* replace with the remainder        */
  }
                                       /* add on any remainder              */
  newmessage = newmessage->concat(message);
  return newmessage;                   /* return the message                */
}

void RexxActivity::reraiseException(
    RexxDirectory *exobj )             /* previously created exception obj. */
/******************************************************************************/
/* Function:  Propagate a syntax condition and trapping to earlier levels     */
/******************************************************************************/
{
  RexxActivation *activation;          /* current activation                */
  RexxArray      *additional;          /* passed on information             */
  RexxObject     *errorcode;           /* full error code                   */
  RexxString     *message;             /* secondary error message           */
  LONG            errornumber;         /* binary error number               */
  LONG            primary;             /* primary message code              */
  CHAR            work[10];            /* temp buffer for formatting        */
  LONG            newVal;

  activation = this->currentActivation;/* get the current activation        */
                                       /* have a target activation?         */
  if (activation != (RexxActivation *)TheNilObject)  {
    newVal = activation->currentLine();/* get the activation position       */
    exobj->put(new_integer(newVal), OREF_POSITION);
                                       /* extract program name from activa. */
    exobj->put(activation->programName(), OREF_PROGRAM);
  }
  else {
    exobj->remove(OREF_POSITION);      /* remove the position               */
    exobj->remove(OREF_PROGRAM);       /* remove the program name           */
  }

  errorcode = exobj->at(OREF_CODE);    /* get the error code                */
                                       /* convert to a decimal              */
  errornumber = message_number((RexxString *)errorcode);
                                       /* get the primary message number    */
  primary = (errornumber / 1000) * 1000;
  if (errornumber != primary) {        /* have an actual secondary message? */
                                       /* format the number (string) into   */
                                       /*  work buffer.                     */
    sprintf(work,"%1d%3.3d", errornumber/1000, errornumber - primary);
    errornumber = atol(work);          /* convert to a long value           */
                                       /* retrieve the secondary message    */
    message = SysMessageText(errornumber);
                                       /* Retrieve any additional parameters*/
    additional = (RexxArray *)exobj->at(OREF_ADDITIONAL);
                                       /* do required substitutions         */
    message = this->messageSubstitution(message, additional);
                                       /* replace the original message text */
    exobj->put(message, OREF_NAME_MESSAGE);
  }
  this->raisePropagate(exobj);         /* pass on down the chain            */
}

void RexxActivity::raisePropagate(
    RexxDirectory *conditionobj )      /* condition descriptive information */
/******************************************************************************/
/* Function:   Propagate a condition down the chain of activations            */
/******************************************************************************/
{
  RexxActivationBase *activation;      /* current activation                */
  RexxList           *traceback;       /* traceback information             */
  RexxString         *condition;       /* condition to propagate            */

                                       /* get the condition                 */
  condition = (RexxString *)conditionobj->at(OREF_CONDITION);
                                       /* propagating syntax errors?        */
  if (condition->strCompare(CHAR_SYNTAX))
                                       /* get the traceback                 */
    traceback = (RexxList *)conditionobj->at(OREF_TRACEBACK);
  else
    traceback = OREF_NULL;             /* no trace back to process          */
  activation = this->current();        /* get the current activation        */

                                       /* loop to the top of the stack      */
  while (activation != (RexxActivationBase *)TheNilObject) {
                                       /* give this one a chance to trap    */
                                       /* (will never return for trapped    */
                                       /* PROPAGATE conditions)             */
    activation->trap(condition, conditionobj);
                                       /* this is a propagated condition    */
    conditionobj->put(TheTrueObject, OREF_PROPAGATED);
    if ((traceback != TheNilObject)
                && (((RexxActivation*)activation)->settings.traceindent < MAX_TRACEBACK_LIST))  /* have a traceback? */
      activation->traceBack(traceback);/* add this to the traceback info    */
    activation->termination();         /* do activation termiantion process */
    this->pop(FALSE);                  /* pop top nativeact/activation      */
    activation = this->current();      /* get the sender's sender           */
  }
                                       /* now kill the activity, using the  */
  this->kill(conditionobj);            /* imbedded description object       */
}

RexxObject *RexxActivity::display(RexxDirectory *exobj)
                                       /* display exception object info     */
                                       /* target exception object           */
/******************************************************************************/
/* Function:  Display information from an exception object                    */
/******************************************************************************/
{
  RexxArray  *trace_back;              /* traceback information             */
  RexxList   *trace_backList;          /* traceback information             */
  RexxString *secondary;               /* secondary message                 */
  RexxString *text;                    /* constructed final message         */
  RexxObject *position;                /* position in program error occurred*/
  RexxString *programname;             /* name of program being run         */
  LONG      size;                      /* traceback array size              */
  LONG      i;                         /* loop counter                      */
  LONG      starttrc = 1;              /* number of first trace line        */
  LONG      errorCode;                 /* error message code                */
  RexxObject *rc;

                                       /* get the traceback info            */
  trace_backList = (RexxList *)exobj->at(OREF_TRACEBACK);
  if (trace_backList != OREF_NULL) {   /* have a traceback?                 */
                                       /* convert to an array               */
    trace_back = trace_backList->makeArray();
                                       /* save from gc                      */
    save(trace_back);
                                       /* get the traceback size            */
    size = trace_back->size();

    for (i= starttrc; i <= size; i++) {       /* loop through the traceback starttrc */
      text = (RexxString *)trace_back->get(i);
                                       /* have a real line?                 */
      if (text != OREF_NULL && text != TheNilObject)
                                       /* write out the line                */
        this->traceOutput(this->currentActivation, text);
    }
  discard(hold(trace_back));           /* ok, let gc have it                */
  }
  rc = exobj->at(OREF_RC);             /* get the error code                */
                                       /* get the message number            */
  errorCode = message_number((RexxString *)rc);
                                       /* get the header                    */
  text = (RexxString *)SysMessageHeader(errorCode);
  if (text == OREF_NULL)               /* no header available?              */
                                       /* get the leading part              */
    text = (RexxString *)SysMessageText(Message_Translations_error);
  else                                 /* add to the message text           */
    text = text->concat(SysMessageText(Message_Translations_error));
                                       /* get the name of the program       */
  programname = (RexxString *)exobj->at(OREF_PROGRAM);
                                       /* add on the error number           */
  text = text->concatWith(REQUEST_STRING(rc), ' ');
                                       /* if program exists, add the name   */
                                       /* of the program to the message     */
  if (programname != (RexxString *)OREF_NULL && programname != OREF_NULLSTRING) {
                                       /* add on the "running" part         */
    text = text->concatWith(SysMessageText(Message_Translations_running), ' ');
                                       /* add on the program name           */
    text = text->concatWith(programname, ' ');
                                       /* Get the position/Line number info */
    position = exobj->at(OREF_POSITION);
    if (position != OREF_NULL) {       /* Do we have position/Line no info? */
                                       /* Yes, add on the "line" part       */
      text = text->concatWith(SysMessageText(Message_Translations_line), ' ');
                                       /* add on the line number            */
      text = text->concatWith(REQUEST_STRING(exobj->at(OREF_POSITION)), ' ');
                                       /* add on the ":  "                  */
    }
  }
  text = text->concatWithCstring(":  ");
                                       /* and finally the error message     */
  text = text->concat((RexxString *)exobj->at(OREF_ERRORTEXT));
                                       /* write out the line                */
  this->traceOutput(this->currentActivation, text);
                                       /* get the secondary message         */
  secondary = (RexxString *)exobj->at(OREF_NAME_MESSAGE);
                                       /* have a real message?              */
  if (secondary != OREF_NULL && secondary != (RexxString *)TheNilObject) {
    rc = exobj->at(OREF_CODE);         /* get the error code                */
                                       /* get the message number            */
    errorCode = message_number((RexxString *)rc);
                                       /* get the header                    */
    text = (RexxString *)SysMessageHeader(errorCode);
    if (text == OREF_NULL)             /* no header available?              */
                                       /* get the leading part              */
      text = (RexxString *)SysMessageText(Message_Translations_error);
    else                               /* add to the message text           */
      text = text->concat(SysMessageText(Message_Translations_error));
                                       /* add on the error number           */
    text = text->concatWith((RexxString *)rc, ' ');
                                       /* add on the ":  "                  */
    text = text->concatWithCstring(":  ");
                                       /* and finally the error message     */
    text = text->concat(secondary);
                                       /* write out the line                */
    this->traceOutput(this->currentActivation, text);
  }
  return TheNilObject;                 /* just return .nil                  */
}

RexxObject *RexxActivity::displayDebug(RexxDirectory *exobj)
                                       /* display exception object info     */
                                       /* target exception object           */
/******************************************************************************/
/* Function:  Display information from an exception object                    */
/******************************************************************************/
{
  RexxString *secondary;               /* secondary message                 */
  RexxString *text;                    /* constructed final message         */

                                       /* get the leading part              */
  text = (RexxString *)SysMessageText(Message_Translations_debug_error);
                                       /* add on the error number           */
  text = text->concatWith(REQUEST_STRING(exobj->at(OREF_RC)), ' ');
                                       /* add on the ":  "                  */
  text = text->concatWithCstring(":  ");
                                       /* and finally the error message     */
  text = text->concatWith((RexxString *)exobj->at(OREF_ERRORTEXT), ' ');
                                       /* write out the line                */
  this->traceOutput(this->currentActivation, text);
                                       /* get the secondary message         */
  secondary = (RexxString *)exobj->at(OREF_NAME_MESSAGE);
                                       /* have a real message?              */
  if (secondary != OREF_NULL && secondary != (RexxString *)TheNilObject) {
                                       /* get the leading part              */
    text = (RexxString *)SysMessageText(Message_Translations_debug_error);
                                       /* add on the error number           */
    text = text->concatWith((RexxString *)exobj->at(OREF_CODE), ' ');
                                       /* add on the ":  "                  */
    text = text->concatWithCstring(":  ");
                                       /* and finally the error message     */
    text = text->concat(secondary);
                                       /* write out the line                */
    this->traceOutput(this->currentActivation, text);
  }
  return TheNilObject;                 /* just return .nil                  */
}

void RexxActivity::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  LONG    i;                           /* loop counter                      */

  setUpMemoryMark
  memory_mark(this->activations);
  memory_mark(this->topActivation);
  memory_mark(this->currentActivation);
  memory_mark(this->save);
  memory_mark(this->local);
  memory_mark(this->conditionobj);
  memory_mark(this->requiresTable);
  memory_mark(this->nextWaitingActivity);
  memory_mark(this->waitingObject);
  memory_mark(this->nestedInfo.currentExit);
  memory_mark(this->nestedInfo.shvexitvalue);
  for (i = 0; i < LAST_EXIT; i++)
    memory_mark(this->nestedInfo.sysexits[i]);

  /* have the frame stack do its own marking. */
  frameStack.live();
  cleanUpMemoryMark
}
void RexxActivity::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  LONG    i;                           /* loop counter                      */

  setUpMemoryMarkGeneral
  memory_mark_general(this->activations);
  memory_mark_general(this->topActivation);
  memory_mark_general(this->currentActivation);
  memory_mark_general(this->save);
  memory_mark_general(this->local);
  memory_mark_general(this->conditionobj);
  memory_mark_general(this->requiresTable);
  memory_mark_general(this->nextWaitingActivity);
  memory_mark_general(this->waitingObject);
  memory_mark_general(this->nestedInfo.currentExit);
  memory_mark_general(this->nestedInfo.shvexitvalue);

  for (i = 0; i < LAST_EXIT; i++)
    memory_mark_general(this->nestedInfo.sysexits[i]);

  /* have the frame stack do its own marking. */
  frameStack.liveGeneral();
  cleanUpMemoryMarkGeneral
}

void RexxActivity::flatten(RexxEnvelope* envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
                                       /* Activities don't get moved,       */
                                       /*  we just return OREF_NULL. we may */
                                       /*  create a special proxy for this  */
                                       /*  to re-establish an activation on */
                                       /*  system.                          */
  return;
}

void RexxActivity::run()
/******************************************************************************/
/* Function:  Release an activity to run                                      */
/******************************************************************************/
{
  EVPOST(this->guardsem);              /* and the guard semaphore           */
  EVPOST(this->runsem);                /* post the run semaphore            */
  SysThreadYield();                    /* yield the thread                  */
}

void RexxActivity::push(
    RexxActivationBase *new_activation)/* activation to add                 */
/******************************************************************************/
/* Function:  Push an activation onto the activity stack                      */
/******************************************************************************/
{
  RexxInternalStack *newstack;         /* replacement activation stack      */
  INT  i;                              /* loop counter                      */

  if (this->depth == this->size) {     /* reached the end?                  */
                                       /* get a larger stack                */
    newstack = new_internalstack(this->size*2);
    for (i = this->size; i; i--)       /* loop through the old stack        */
                                       /* copying onto the new stack        */
      newstack->push(this->activations->peek((long)(i-1)));
    this->activations = newstack;      /* replace the old stack             */
    this->size *= 2;                   /* size is twice as big              */
  }

                                       /* add to the stack                  */
  this->activations->push((RexxObject *)new_activation);
  this->topActivation = new_activation;/* set this as the top one           */
                                       /* new REXX activation?              */
  if (OTYPE(Activation, new_activation)) {
                                       /* this is the top REXX one too      */
    this->currentActivation = (RexxActivation *)new_activation;
                                       /* get the activation settings       */
    this->settings = ((RexxActivation *)new_activation)->getGlobalSettings();
                                       /* copy the DBCS information         */
    this->settings->codepage = this->codepage;
    this->settings->DBCS_codepage = this->DBCS_codepage;
    this->settings->DBCS_table = this->DBCS_table;
    if (CurrentActivity == this)       /* this the active activity?         */
                                       /* update the active values          */
      current_settings = this->settings;
  }
  this->depth++;                       /* bump the depth to count this      */
}

void RexxActivity::pushNil()
/******************************************************************************/
/* Function:  Push an empty activaiton marker on the activity stack           */
/******************************************************************************/
{
  RexxInternalStack *newstack;         /* replacement activation stack      */
  INT  i;                              /* loop counter                      */

  if (this->depth == this->size) {     /* reached the end?                  */
                                       /* get a larger stack                */
    newstack = new_internalstack(this->size*2);
    for (i = this->size; i; i--)       /* loop through the old stack        */
                                       /* copying onto the new stack        */
      newstack->push(this->activations->peek((long)(i-1)));
    this->activations = newstack;      /* replace the old stack             */
    this->size *= 2;                   /* size is twice as big              */
  }
                                       /* add to the stack                  */
  this->activations->push(TheNilObject);
                                       /* clear out the cached values       */
  this->topActivation = (RexxActivationBase *)TheNilObject;
                                       /* both of them                      */
  this->currentActivation = (RexxActivation *)TheNilObject;
                                       /* use the default settings          */
  this->settings = &this->default_settings;
  current_settings = this->settings;   /* update the active values          */
  this->depth++;                       /* bump the depth to count this      */
}


void RexxActivity::pop(
    BOOL  reply)                       /* popping for REPLY purposes        */
/******************************************************************************/
/* Function:  Remove an activation from the activity stack                    */
/******************************************************************************/
{
  RexxActivationBase *top_activation;  /* removed activation                */
  RexxActivationBase *old_activation;  /* removed activation                */
  RexxActivationBase *current=NULL;    /* current loop activation           */
  RexxInternalStack *activations;      /* activation stack                  */
  ULONG i;                             /* loop counter                      */

  if (0 == this->depth)                /* At the very top of stack?         */
    return;                            /* just return;                      */

  activations = this->activations;     /* get a local copy                  */
                                       /* pop it off the stack              */
  top_activation = (RexxActivationBase *)activations->fastPop();
  this->depth--;                       /* remove the depth                  */
  if (this->depth == 0) {              /* this the last one?                */
                                       /* clear out the cached values       */
    this->topActivation = (RexxActivationBase *)TheNilObject;
                                       /* both of them                      */
    this->currentActivation = (RexxActivation *)TheNilObject;
                                       /* use the default settings          */
    this->settings = &this->default_settings;
  }
  else {                               /* probably have a previous one      */
                                       /* get the top item                  */
    old_activation = (RexxActivationBase *)activations->getTop();
                                       /* this is the top one               */
    this->topActivation = old_activation;
                                       /* popping a REXX activation?        */
    if (OTYPE(Activation, top_activation)) {
                                       /* clear this out                    */
      old_activation = (RexxActivationBase *)TheNilObject;
                                       /* spin down the stack               */
      for (i = 0; current != (RexxActivationBase *)TheNilObject && i < this->depth; i++) {
                                       /* get the next item                 */
        current = (RexxActivationBase *)activations->peek(i);
                                       /* find a REXX one?                  */
        if (OTYPE(Activation, current)) {
          old_activation = current;    /* save this one                     */
          break;                       /* and exit the loop                 */
        }
      }
                                       /* set this as current               */
      this->currentActivation = (RexxActivation *)old_activation;
                                       /* last activation?                  */
      if (old_activation == (RexxActivationBase*)TheNilObject)
                                       /* use the default settings          */
        this->settings = &this->default_settings;
      else
                                       /* get the activation settings       */
        this->settings = ((RexxActivation *)old_activation)->getGlobalSettings();
      if (CurrentActivity == this)     /* this the active activity?         */
                                       /* update the active values          */
        current_settings = this->settings;
      if (!reply)                      /* not a reply removal?              */
                                       /* add this to the cache             */
        TheActivityClass->cacheActivation((RexxActivation *)top_activation);
    }
                                       /* did we pop off .NIL?              */
    else if (top_activation == (RexxActivationBase *)TheNilObject) {
      activations->push(TheNilObject); /* Yes, force back on.               */
      this->depth++;                   /* step the depth back up            */
    }
  }
}

void RexxActivity::popNil()
/******************************************************************************/
/* Function:  Remove an activation marker from the activity stack             */
/******************************************************************************/
{
  RexxActivationBase *old_activation;  /* removed activation                */
  RexxActivationBase *current = OREF_NULL; /* current loop activation,      */
  RexxInternalStack *activations;      /* activation stack                  */
  size_t i;                            /* loop counter                      */

  activations = this->activations;     /* get a local copy                  */
  activations->fastPop();              /* pop it off the stack              */
  this->depth--;                       /* remove the depth                  */
  if (this->depth <= 0) {              /* this the last one?                */
                                       /* clear out the cached values       */
    this->topActivation = (RexxActivationBase *)TheNilObject;
                                       /* both of them                      */
    this->currentActivation = (RexxActivation *)TheNilObject;
                                       /* use the default settings          */
    this->settings = &this->default_settings;
    this->depth = 0;                   /* make sure this is zero            */
  }
  else {                               /* probably have a previous one      */
                                       /* get the top item                  */
    old_activation = (RexxActivationBase *)activations->getTop();
                                       /* this is the top one               */
    this->topActivation = old_activation;
                                       /* clear this out                    */
    old_activation = (RexxActivationBase *)TheNilObject;
                                       /* spin down the stack               */
    for (i = 0; current != (RexxActivationBase *)TheNilObject && i < this->depth; i++) {
                                       /* get the next item                 */
      current = (RexxActivationBase *)activations->peek(i);
                                       /* find a REXX one?                  */
      if (OTYPE(Activation, current)) {
        old_activation = current;      /* save this one                     */
        break;                         /* and exit the loop                 */
      }
    }
                                       /* set this as current               */
    this->currentActivation = (RexxActivation *)old_activation;
                                       /* last activation?                  */
    if (old_activation == (RexxActivationBase *)TheNilObject)
                                       /* use the default settings          */
      this->settings = &this->default_settings;
    else
                                       /* get the activation settings       */
      this->settings = ((RexxActivation *)old_activation)->getGlobalSettings();
  }
}

void RexxActivity::exitKernel(
  RexxActivation *activation,          /* activation going external on      */
  RexxString     *message_name,        /* reason for going external         */
  BOOL            enable )             /* variable pool enabled flag        */
/******************************************************************************/
/*  Function:  Release the kernel access because code is going to "leave"     */
/*             the kernel.  This prepares for this by adding a native         */
/*             activation on to the stack to act as a server for the          */
/*             external call.  This way new native methods do not need to     */
/*             be created just to get an activation created                   */
/******************************************************************************/
{
  RexxNativeActivation *new_activation;/* new native activation             */

                                       /* create a new native activation    */
  new_activation = new ((RexxObject *)activation, (RexxMethod *)OREF_NULL, this, message_name, (RexxActivationBase *)activation) RexxNativeActivation;
                                       /* push it on the activity stack     */
  this->push((RexxActivationBase *)new_activation);
  if (enable)                          /* need variable pool access?        */
                                       /* turn it on now                    */
    new_activation->enableVariablepool();
  ReleaseKernelAccess(this);           /* now give up control               */
}
void RexxActivity::enterKernel()
/******************************************************************************/
/*  Function:  Recover the kernel access and pop the native activation        */
/*             created by activity_exit_kernel from the activity stack.       */
/******************************************************************************/
{
  RequestKernelAccess(this);           /* get the kernel lock back          */
  ((RexxNativeActivation *)this->topActivation)->disableVariablepool();
  this->pop(FALSE);                    /* pop the top activation            */
}

void RexxActivity::checkDeadLock(
  RexxActivity *targetActivity)        /* activity currently reserving      */
/******************************************************************************/
/* Function:  Check for a circular wait dead lock error                       */
/******************************************************************************/
{
  RexxActivity *owningActivity;        /* activity owning the resource      */
                                       /* currently waiting on something?   */
  if (this->waitingObject != OREF_NULL) {
                                       /* waiting on a message object?      */
    if (OTYPE(Message, this->waitingObject))
                                       /* get the activity message is on    */
      owningActivity = ((RexxMessage *)this->waitingObject)->startActivity;
    else
                                       /* get the locking activity for vdict*/
      owningActivity = ((RexxVariableDictionary *)this->waitingObject)->reservingActivity;
                                       /* have a circular wait              */
    if (owningActivity == targetActivity)
                                       /* have a deaklock                   */
      report_exception(Error_Execution_deadlock);
    if (owningActivity != OREF_NULL)   /* have a real activity?             */
                                       /* pass it along the chain           */
      owningActivity->checkDeadLock(targetActivity);
  }
}

void RexxActivity::waitReserve(
  RexxObject *resource )               /* resource we are waiting on        */
/******************************************************************************/
/* Function:  Wait for a new run event to occur                               */
/******************************************************************************/
{
  EVSET(this->runsem);                 /* clear the run semaphore           */
  this->waitingObject = resource;      /* save the waiting resource         */
  ReleaseKernelAccess(this);           /* release the kernel access         */
  EVWAIT(this->runsem);                /* wait for the run to be posted     */
  RequestKernelAccess(this);           /* reaquire the kernel access        */
}

void RexxActivity::guardWait()
/******************************************************************************/
/* Function:  Wait for a guard post event                                     */
/******************************************************************************/
{
  ReleaseKernelAccess(this);           /* release kernel access             */
  EVWAIT(this->guardsem);              /* wait on the guard semaphore       */
#ifndef NEWGUARD
  RequestKernelAccess(this);           /* reaquire the kernel lock          */
#endif
}

void RexxActivity::guardPost()
/******************************************************************************/
/* Function:  Post a guard expression wake up notice                          */
/******************************************************************************/
{
                                       /* make sure we have access to sem   */
  EVOPEN(this->guardsem);              /* may be called from another process*/
  EVPOST(this->guardsem);              /* OK for it to already be posted    */
  EVCL(this->guardsem);                /* release access to sem.            */
}

void RexxActivity::guardSet()
/******************************************************************************/
/* Function:  Clear a guard expression semaphore in preparation to perform a  */
/*            guard wait                                                      */
/******************************************************************************/
{
  EVSET(this->guardsem);               /* set up for guard call             */
}

void RexxActivity::postRelease()
/******************************************************************************/
/* Function:  Post an activities run semaphore                                */
/******************************************************************************/
{
  this->waitingObject = OREF_NULL;     /* no longer waiting                 */
                                       /* make sure we have access to sem   */
  EVOPEN(this->runsem);                /* may be called from another process*/
  EVPOST(this->runsem);                /* OK for it to already be posted    */
  EVCL(this->runsem);                  /* release access to sem.            */
}

void RexxActivity::kill(
    RexxDirectory *conditionobj)       /* associated "kill" object          */
/******************************************************************************/
/* Function:  Kill a running activity,                                        */
/******************************************************************************/
{
  this->conditionobj = conditionobj;   /* save the condition object         */
  longjmp(this->nestedInfo.jmpenv,1);  /* jump back to the error handler    */
}

RexxActivationBase *RexxActivity::sender(
    RexxActivationBase *activation)    /* target activation                 */
/******************************************************************************/
/* Function:  Return a reference to the activation prior to the target        */
/*            activation                                                      */
/******************************************************************************/
{
  size_t i;                            /* loop counter                      */

  for (i = 0; i < this->depth &&       /* loop to the target activation     */
      (RexxActivationBase *)this->activations->peek(i) != activation; i++) ;
  if (i < this->depth-1)               /* anything left over?               */
                                       /* return the previous version       */
    return (RexxActivation *)this->activations->peek(i+1);
  else
                                       /* just return OREF_NIL              */
    return (RexxActivation *) TheNilObject;
}

void RexxActivity::relinquish()
/******************************************************************************/
/*  Function: Relinquish to other system processes                            */
/******************************************************************************/
{
                                       /* other's waiting to go?            */
  if (TheActivityClass->waitingActivity() != OREF_NULL) {
                                       /* now join the line                 */
    TheActivityClass->addWaitingActivity(this, TRUE);
  }
  SysRelinquish();                     /* now allow system stuff to run     */
}

void RexxActivity::yield(RexxObject *result)
/******************************************************************************/
/* Function:  Yield control so some other activity can run                    */
/******************************************************************************/
{
                                       /* other's waiting to go?            */
  if (TheActivityClass->waitingActivity() != OREF_NULL) {
    this->save = result;               /* save the result value             */
                                       /* now join the line                 */
    TheActivityClass->addWaitingActivity(this, TRUE);
    hold(result);                      /* hold the result                   */
    this->save = OREF_NULL;            /* release the saved value           */
  }
}

void RexxActivity::releaseKernel()
/******************************************************************************/
/* Function:  Release exclusive access to the kernel                          */
/******************************************************************************/
{
  CurrentActivity = OREF_NULL;         /* no current activity               */
  current_settings = &defaultSettings; /* use default settings              */
  MTXRL(kernel_semaphore);             /* release the kernel semaphore      */
}

void RexxActivity::requestKernel()
/******************************************************************************/
/* Function:  Acquire exclusive access to the kernel                          */
/******************************************************************************/
{
                                       /* only one there?                   */
  if (TheActivityClass->waitingActivity() == OREF_NULL) {
    if (MTXRI(kernel_semaphore) == 0) {/* try directly requesting first     */
                                       /* we left the kernel ?              */
      if (GlobalCurrentPool != ProcessCurrentPool)
                                       /* yes, get access to them.          */
        memoryObject.accessPools(ProcessCurrentPool);

      CurrentActivity = this;          /* set new current activity          */
                                       /* and new active settings           */
      current_settings = this->getSettings();
      return;                          /* get out if we have it             */
    }
  }
                                       /* can't get it, go stand in line    */
  TheActivityClass->addWaitingActivity(this, FALSE);
}

void RexxActivity::stackSpace()
/******************************************************************************/
/* Function:  Make sure there is enough stack space to run a method           */
/******************************************************************************/
{
#ifdef STACKCHECK
  long temp;                           /* if checking and there isn't room  */
  if (PTRSUB2(&temp,this->nestedInfo.stackptr) < MIN_C_STACK && this->stackcheck == TRUE)
                                       /* go raise an exception             */
    report_exception(Error_Control_stack_full);
#endif
}

long RexxActivity::priorityMethod()
/******************************************************************************/
/* Function:  Retrieve the activations priority                               */
/******************************************************************************/
{
  return this->priority;               /* just return the priority          */
}

RexxObject *RexxActivity::localMethod()
/******************************************************************************/
/* Function:  Retrive the activities local environment                        */
/******************************************************************************/
{
  return this->local;                  /* just return the .local directory  */
}

long  RexxActivity::threadIdMethod()
/******************************************************************************/
/* Function:  Retrieve the activities threadid                                */
/******************************************************************************/
{
  return (long)this->threadid;         /* just return the thread id info    */
}

void  RexxActivity::setShvVal(
    RexxString *retval)
/******************************************************************************/
/* Function:  Set a return value as a result of a call to the variable pool   */
/*            with the RXSHV_EXIT shvcode specified                           */
/******************************************************************************/
{
                                       /* set the ret'd value from var pool */
  this->nestedInfo.shvexitvalue = retval;
}

void RexxActivity::queryTrcHlt()
/******************************************************************************/
/* Function:  Determine if Halt or Trace System exits are set                 */
/*            and set a flag to indicate this.                                */
/******************************************************************************/
{                                      /* is HALT sys exit set              */
  if (this->nestedInfo.sysexits[RXHLT - 1] != OREF_NULL) {
    this->nestedInfo.exitset = TRUE;   /* set flag to indicate one is found */
    return;                            /* and return                        */
  }
                                       /* is TRACE sys exit set             */
  if (this->nestedInfo.sysexits[RXTRC - 1] != OREF_NULL) {
    this->nestedInfo.exitset = TRUE;   /* set flag to indicate one is found */
    return;                            /* and return                        */
  }

  if (this->nestedInfo.sysexits[RXDBG - 1] != OREF_NULL) {
    this->nestedInfo.exitset = TRUE;   /* set flag to indicate one is found */
    return;                            /* and return                        */
  }

  this->nestedInfo.exitset = FALSE;    /* remember that none are set        */
  return;                              /* and return                        */
}

void RexxActivity::sysExitInit(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the initialization system exit.                                */
/******************************************************************************/
{
  RexxString *exitname;                /* Exit routine name                 */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXINI);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */

                                       /* add the variable RXPROGRAMNAME to */
                                       /* the variable pool, it contains the*/
                                       /* script name that is currently run */
    RexxString   *varName = new_cstring("RXPROGRAMNAME");
    RexxString   *sourceStr = activation->code->getProgramName();
    RexxVariableDictionary *vdict = activation->getLocalVariables();
    RexxVariable *pgmName = vdict->createVariable(varName);
    pgmName->set(sourceStr);
                                       /* call the handler                  */
    SysExitHandler(this, activation, exitname, RXINI, RXINIEXT, NULL, TRUE);
                                       /* if variable was not changed, then */
                                       /* remove it from the var. pool again*/
    if (sourceStr == pgmName->getVariableValue()) {
      vdict->contents->remove(varName);
    }
  }
}

void RexxActivity::sysExitTerm(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the termination system exit.                                   */
/******************************************************************************/
{
  RexxString *exitname;                /* Exit routine name                 */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXTER);
  if (exitname != OREF_NULL)           /* exit enabled?                     */
                                       /* call the handler                  */
    SysExitHandler(this, activation, exitname, RXTER, RXTEREXT, NULL, TRUE);
}

BOOL  RexxActivity::sysExitSioSay(
    RexxActivation *activation,        /* sending activation                */
    RexxString     *sayoutput)         /* line to write out                 */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXSIOSAY_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXSIO);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* make into RXSTRING form           */
    MAKERXSTRING(exit_parm.rxsio_string, sayoutput->stringData,  sayoutput->length);
                                       /* call the handler                  */
    return SysExitHandler(this, activation, exitname, RXSIO, RXSIOSAY, (PVOID)&exit_parm, FALSE);
  }
  return TRUE;                         /* exit didn't handle                */
}

BOOL RexxActivity::sysExitSioTrc(
    RexxActivation *activation,        /* sending activation                */
    RexxString     *traceoutput)       /* line to write out                 */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
  RexxString *  exitname;              /* Exit routine name                 */
  RXSIOTRC_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXSIO);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* make into RXSTRING form           */
    MAKERXSTRING(exit_parm.rxsio_string, traceoutput->stringData, traceoutput->length);
                                       /* call the handler                  */
    return SysExitHandler(this, activation, exitname, RXSIO, RXSIOTRC, (PVOID)&exit_parm, FALSE);
  }
  return TRUE;                         /* exit didn't handle                */
}

BOOL RexxActivity::sysExitSioTrd(
    RexxActivation *activation,        /* sending activation                */
    RexxString **inputstring)          /* returned input string             */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXSIOTRD_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */

  *retbuffer = '\0';
                                       /* get the exit handler              */
  exitname = this->querySysExits(RXSIO);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Pass along default RXSTRING       */
/*     MAKERXSTRING(exit_parm.rxsiotrd_retc, retbuffer, DEFRXSTRING); */
    MAKERXSTRING(exit_parm.rxsiotrd_retc, retbuffer, 0);
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXSIO, RXSIOTRD, (PVOID)&exit_parm, FALSE))
      return TRUE;                     /* this wasn't handled               */
                                       /* shv_exit return a value?          */
    if (this->nestedInfo.shvexitvalue != OREF_NULL) {
                                       /* return this information           */
      *inputstring = (RexxString *)this->nestedInfo.shvexitvalue;
      return FALSE;                    /* return that request was handled   */
    }
                                       /* Get input string and return it    */
    *inputstring = (RexxString *)new_string((PCHAR)exit_parm.rxsiotrd_retc.strptr, exit_parm.rxsiotrd_retc.strlength);
                                       /* user give us a new buffer?        */
    if (exit_parm.rxsiotrd_retc.strptr != retbuffer)
                                       /* free it                           */
      SysReleaseResultMemory(exit_parm.rxsiotrd_retc.strptr);
    return FALSE;                      /* this was handled                  */
  }
  return TRUE;                         /* not handled                       */
}

BOOL RexxActivity::sysExitSioDtr(
    RexxActivation *activation,        /* sending activation                */
    RexxString    **inputstring)       /* returned input value              */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the I/O system exit.                                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXSIODTR_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */

  *retbuffer = '\0';
                                       /* get the exit handler              */
  exitname = this->querySysExits(RXSIO);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Pass along default RXSTRING       */
/*     MAKERXSTRING(exit_parm.rxsiodtr_retc, retbuffer, DEFRXSTRING); */
    MAKERXSTRING(exit_parm.rxsiodtr_retc, retbuffer, 0);
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXSIO, RXSIODTR, (PVOID)&exit_parm, FALSE))
      return TRUE;                     /* this wasn't handled               */
                                       /* shv_exit return a value?          */
    if (this->nestedInfo.shvexitvalue != OREF_NULL) {
                                       /* return this information           */
      *inputstring = (RexxString *)this->nestedInfo.shvexitvalue;
      return FALSE;                    /* return that request was handled   */
    }
                                       /* Get input string and return it    */
    *inputstring = (RexxString *)new_string((PCHAR)exit_parm.rxsiodtr_retc.strptr, exit_parm.rxsiodtr_retc.strlength);
                                       /* user give us a new buffer?        */
    if (exit_parm.rxsiodtr_retc.strptr != retbuffer)
                                       /* free it                           */
      SysReleaseResultMemory(exit_parm.rxsiodtr_retc.strptr);
    return FALSE;                      /* this was handled                  */
  }
  return TRUE;                         /* not handled                       */
}

BOOL RexxActivity::sysExitFunc(
    RexxActivation *activation,        /* calling activation                */
    RexxString     *rname,             /* routine name                      */
    RexxObject     *calltype,          /* type of call                      */
    RexxObject     **funcresult,       /* function result                   */
    RexxObject    **arguments,         /* argument array                    */
    size_t          argcount)          /* argument count                    */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the function system exit.                                      */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXFNCCAL_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */
  long          argindex;              /* Counter for arg array             */
  PRXSTRING     argrxarray;            /* Array of args in PRXSTRING form   */
  RexxString   *temp;                  /* temporary argument                */
  RexxString   *stdqueue;              /* current REXX queue                */
  RexxDirectory *securityArgs;         /* security check arguments          */
  BOOL         wasNotHandled;

                                       /* need to do a security check?      */
  if (activation->hasSecurityManager()) {
    securityArgs = new_directory();    /* get a directory item              */
                                       /* add the function name             */
    securityArgs->put(rname, OREF_NAME);
                                       /* and the arguments                 */
    securityArgs->put(new (argcount, arguments) RexxArray, OREF_ARGUMENTS);
                                       /* did manager handle this?          */
    if (activation->callSecurityManager(OREF_CALL, securityArgs)) {
                                       /* get the return code               */
      *funcresult = securityArgs->fastAt(OREF_RESULT);
      return FALSE;                    /* we've handled this                */
    }
  }
                                       /* get the exit handler              */
  exitname = this->querySysExits(RXFNC);
#ifdef SCRIPTING
  if (exitname == OREF_NULL) {
    char *data = ((RexxString*) calltype)->stringData;
    // special "calltype"? make a real one out of it and check
    if (data[0] == 'A' && data[1] == 'X') {
      sscanf(((RexxString*) calltype)->stringData+2,"%p",&calltype);
      if (calltype)
        exitname = this->querySysExits(RXEXF); // external call (Script Engine?)
    }
  }
#endif
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Start building the exit block  */
    exit_parm.rxfnc_flags.rxfferr = 0; /* Initialize error codes to zero */
    exit_parm.rxfnc_flags.rxffnfnd = 0;
    if (calltype == OREF_FUNCTIONNAME) /* function call?                    */
                                       /* set the function flag             */
      exit_parm.rxfnc_flags.rxffsub = 0;
    else
                                       /* this is the CALL instruction      */
      exit_parm.rxfnc_flags.rxffsub = 1;
                                       /* fill in the name parameter        */
    exit_parm.rxfnc_namel = rname->length;
    exit_parm.rxfnc_name = (PUCHAR)rname->stringData;

                                       /* Get current active queue name     */
    stdqueue = (RexxString *)SysGetCurrentQueue();
                                       /* fill in the name                  */
    exit_parm.rxfnc_que = (PUCHAR)stdqueue->stringData;
                                       /* and the length                    */
    exit_parm.rxfnc_quel = stdqueue->length;
                                       /* Build arg array of RXSTRINGs      */
                                       /* get number of args                */
    exit_parm.rxfnc_argc = argcount;


    /* allocate enough memory for all arguments.           */
    /* At least one item needs to be allocated in order to avoid an error   */
    /* in the sysexithandler!                                               */
    argrxarray = (PRXSTRING) SysAllocateResultMemory(
                    sizeof(RXSTRING) * max(exit_parm.rxfnc_argc,1));
    if (argrxarray == OREF_NULL)       /* memory error?                     */
      report_exception(Error_System_resources);
                                       /* give the arg array pointer        */
    exit_parm.rxfnc_argv = argrxarray;
                                       /* construct the arg array           */
    for (argindex=0; argindex < exit_parm.rxfnc_argc; argindex++) {
                                       /* get the next argument             */
      if (this->exitObjects == TRUE) {
        // store pointers to rexx objects
        argrxarray[argindex].strlength = 8; // pointer length in ASCII
        argrxarray[argindex].strptr = (char*)SysAllocateExternalMemory(16);
        sprintf(argrxarray[argindex].strptr,"%p",arguments[argindex]); // ptr to object
      } else {
        // classic REXX style interface
        temp = (RexxString *)arguments[argindex];
        if (temp != OREF_NULL) {         /* got a real argument?              */
                                         /* force the argument to a string    */
          temp = (RexxString *)REQUEST_STRING(temp);
                                         /* point to the string               */
          argrxarray[argindex].strlength = temp->length;
          argrxarray[argindex].strptr = (PCHAR)temp->stringData;
        }
        else {
                                         /* empty argument                    */
          argrxarray[argindex].strlength = 0;
          argrxarray[argindex].strptr = (PCHAR)NULL;
        }
      }
    }
                                       /* Pass default result RXSTRING      */
    MAKERXSTRING(exit_parm.rxfnc_retc, retbuffer, DEFRXSTRING);
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    wasNotHandled = SysExitHandler(this, activation, exitname, RXFNC, RXFNCCAL, (PVOID)&exit_parm, TRUE);

    /* release the memory allocated for the arguments */
    // free memory that was needed to pass objects
    if (this->exitObjects == TRUE)
    {
        for (argindex=0; argindex < exit_parm.rxfnc_argc; argindex++)
        {
            SysFreeExternalMemory(argrxarray[argindex].strptr);
        }
    }
    SysReleaseResultMemory(argrxarray);

    if (wasNotHandled)
      return TRUE;                     /* this wasn't handled               */


    if (exit_parm.rxfnc_flags.rxfferr) /* function error?                   */
                                       /* raise an error                    */
      report_exception1(Error_Incorrect_call_external, rname);
                                       /* Did we find the function??        */
    else if (exit_parm.rxfnc_flags.rxffnfnd)
                                       /* also an error                     */
      report_exception1(Error_Routine_not_found_name,rname);
                                       /* shv_exit return a value?          */
    if (this->nestedInfo.shvexitvalue != OREF_NULL) {
                                       /* return this information           */
      *funcresult = this->nestedInfo.shvexitvalue;
      return FALSE;                    /* return that request was handled   */
    }
                                       /* Was it a function call??          */
    if (exit_parm.rxfnc_retc.strptr == OREF_NULL && calltype == OREF_FUNCTIONNAME)
                                       /* Have to return data               */
      report_exception1(Error_Function_no_data_function,rname);

    if (exit_parm.rxfnc_retc.strptr) { /* If we have result, return it      */
      // is this a real object?
      if (this->exitObjects == TRUE) {
        RexxObject *transfer = NULL;
        if (sscanf(exit_parm.rxfnc_retc.strptr,"%p",&transfer) == 1)
          *funcresult = transfer;
        else
          report_exception1(Error_Function_no_data_function,rname);
      } else
                                       /* Get input string and return it    */
        *funcresult = new_string((PCHAR)exit_parm.rxfnc_retc.strptr, exit_parm.rxfnc_retc.strlength);
                                       /* user give us a new buffer?        */
      if (exit_parm.rxfnc_retc.strptr != retbuffer)
                                       /* free it                           */
        SysReleaseResultMemory(exit_parm.rxfnc_retc.strptr);
    }
    return FALSE;                      /* this was handled                  */
  }
  return TRUE;                         /* not handled                       */
}

BOOL RexxActivity::sysExitCmd(
     RexxActivation *activation,       /* issuing activation                */
     RexxString *cmdname,              /* command name                      */
     RexxString *environment,          /* environment                       */
     RexxString **conditions,          /* failure/error conditions status   */
     RexxObject **cmdresult)           /* function result                   */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the command system exit.                                       */
/******************************************************************************/
{
  RexxString    *exitname;             /* Exit routine name                 */
  RXCMDHST_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */
  RexxDirectory *securityArgs;         /* security check arguments          */

                                       /* need to do a security check?      */
  if (activation->hasSecurityManager()) {
    securityArgs = new_directory();    /* get a directory item              */
                                       /* add the command                   */
    securityArgs->put(cmdname, OREF_COMMAND);
                                       /* and the target                    */
    securityArgs->put(environment, OREF_ADDRESS);
                                       /* did manager handle this?          */
    if (activation->callSecurityManager(OREF_COMMAND, securityArgs)) {
                                       /* get the return code               */
      *cmdresult = securityArgs->fastAt(OREF_RC);
      if (*cmdresult == OREF_NULL)     /* no return code provide?           */
        *cmdresult = IntegerZero;      /* use a zero return code            */
                                       /* failure indicated?                */
      if (securityArgs->fastAt(OREF_FAILURENAME) != OREF_NULL)
        *conditions = OREF_FAILURENAME;/* raise a FAILURE condition         */
                                       /* how about an error condition?     */
      else if (securityArgs->fastAt(OREF_ERRORNAME) != OREF_NULL)
        *conditions = OREF_ERRORNAME;  /* raise an ERROR condition          */
      return FALSE;                    /* we've handled this                */
    }
  }
                                       /* get the exit handler              */
  exitname = this->querySysExits(RXCMD);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Start building the exit block     */
    exit_parm.rxcmd_flags.rxfcfail = 0;/* Initialize failure/error to zero  */
    exit_parm.rxcmd_flags.rxfcerr = 0;
                                       /* fill in the environment parm      */
    exit_parm.rxcmd_addressl = environment->length;
    exit_parm.rxcmd_address = (PUCHAR)environment->stringData;
                                       /* make cmdaname into RXSTRING form  */
    MAKERXSTRING(exit_parm.rxcmd_command, cmdname->stringData, cmdname->length);

    exit_parm.rxcmd_dll = NULL;        /* Currently no DLL support          */
    exit_parm.rxcmd_dll_len = 0;       /* 0 means .EXE style                */
                                       /* Pass default result RXSTRING      */
    MAKERXSTRING(exit_parm.rxcmd_retc, retbuffer, DEFRXSTRING);
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXCMD, RXCMDHST, (PVOID)&exit_parm, TRUE))
      return TRUE;                     /* this wasn't handled               */
    if (exit_parm.rxcmd_flags.rxfcfail)/* need to raise failure condition?  */

      *conditions = OREF_FAILURENAME;  /* raise an FAILURE condition        */
                                       /* Did we find the function??        */
    else if (exit_parm.rxcmd_flags.rxfcerr)
                                       /* Need to raise error condition?    */
      *conditions = OREF_ERRORNAME;    /* raise an ERROR condition          */
                                       /* shv_exit return a value?          */
    if (this->nestedInfo.shvexitvalue != OREF_NULL) {
                                       /* return this information           */
      *cmdresult = this->nestedInfo.shvexitvalue;
      return FALSE;                    /* return that request was handled   */
    }
                                       /* Get input string and return it    */
    *cmdresult = new_string((PCHAR)exit_parm.rxcmd_retc.strptr, exit_parm.rxcmd_retc.strlength);
                                       /* user give us a new buffer?        */
    if (exit_parm.rxcmd_retc.strptr != retbuffer)
                                       /* free it                           */
      SysReleaseResultMemory(exit_parm.rxcmd_retc.strptr);

    return FALSE;                      /* this was handled                  */
  }
  return TRUE;                         /* not handled                       */
}

BOOL  RexxActivity::sysExitMsqPll(
    RexxActivation *activation,        /* sending activation                */
    RexxString **inputstring)          /* returned input string             */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXMSQPLL_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXMSQ);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Pass along default RXSTRING       */
    MAKERXSTRING(exit_parm.rxmsq_retc, retbuffer, DEFRXSTRING);
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXMSQ, RXMSQPLL, (PVOID)&exit_parm, FALSE))
      return TRUE;                     /* this wasn't handled               */
                                       /* Get input string and return it    */
                                       /* shv_exit return a value?          */
    if (this->nestedInfo.shvexitvalue != OREF_NULL) {
                                       /* return this information           */
      *inputstring = (RexxString *)this->nestedInfo.shvexitvalue;
      return FALSE;                    /* return that request was handled   */
    }
    if (!(exit_parm.rxmsq_retc.strptr))/* if rxstring not null              */
                                       /* no value returned,                */
                                       /* return NIL to note empty stack    */
      *inputstring = (RexxString *)TheNilObject;
    else                               /* return resulting object           */
      *inputstring = (RexxString *)new_string((PCHAR)exit_parm.rxmsq_retc.strptr, exit_parm.rxmsq_retc.strlength);
                                       /* user give us a new buffer?        */
    if (exit_parm.rxmsq_retc.strptr != retbuffer)
                                       /* free it                           */
      SysReleaseResultMemory(exit_parm.rxmsq_retc.strptr);
    return FALSE;                      /* this was handled                  */
  }
  return TRUE;                         /* not handled                       */
}

BOOL  RexxActivity::sysExitMsqPsh(
    RexxActivation *activation,        /* sending activation                */
    RexxString *inputstring,           /* string to be placed on the queue  */
    int lifo_flag)                     /* flag to indicate LIFO or FIFO     */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXMSQPSH_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXMSQ);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
    if (lifo_flag == QUEUE_LIFO)       /* LIFO queuing requested?           */
                                       /* set the flag appropriately        */
      exit_parm.rxmsq_flags.rxfmlifo = 1;
    else
                                       /* this is a FIFO request            */
      exit_parm.rxmsq_flags.rxfmlifo = 0;
                                       /* make into RXSTRING form           */
    MAKERXSTRING(exit_parm.rxmsq_value, inputstring->stringData, inputstring->length);
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXMSQ, RXMSQPSH, (PVOID)&exit_parm, FALSE))
      return TRUE;                     /* this wasn't handled               */

    return FALSE;                      /* this was handled                  */
  }
  return TRUE;                         /* not handled                       */
}

BOOL  RexxActivity::sysExitMsqSiz(
    RexxActivation *activation,        /* sending activation                */
    RexxInteger **returnsize)          /* returned queue size               */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXMSQSIZ_PARM exit_parm;             /* exit parameters                   */
  long          tempSize;

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXMSQ);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXMSQ, RXMSQSIZ, (PVOID)&exit_parm, FALSE))
      return TRUE;                     /* this wasn't handled               */
                                       /* Get queue size and return it      */
    tempSize = exit_parm.rxmsq_size;   /* temporary place holder for new_integer */
    *returnsize = (RexxInteger *)new_integer(tempSize);

    return FALSE;                      /* this was handled                  */
  }
  return TRUE;                         /* not handled                       */
}

BOOL  RexxActivity::sysExitMsqNam(
    RexxActivation *activation,        /* sending activation                */
    RexxString    **inputstring )      /* name of external queue            */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the External Data queue system exit.                           */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXMSQNAM_PARM exit_parm;             /* exit parameters                   */
  char          retbuffer[DEFRXSTRING];/* Default result buffer             */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXMSQ);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* make into RXSTRING form           */
    MAKERXSTRING(exit_parm.rxmsq_name, (*inputstring)->stringData, (*inputstring)->length);
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXMSQ, RXMSQNAM, (PVOID)&exit_parm, FALSE))
      return TRUE;                     /* this wasn't handled               */
    *inputstring = (RexxString *)new_string((PCHAR)exit_parm.rxmsq_name.strptr, exit_parm.rxmsq_name.strlength);
                                       /* user give us a new buffer?        */
    if (exit_parm.rxmsq_name.strptr != retbuffer)
                                       /* free it                           */
      SysReleaseResultMemory(exit_parm.rxmsq_name.strptr);
    return FALSE;                      /* this was handled                  */
  }
  return TRUE;                         /* not handled                       */
}

BOOL RexxActivity::sysExitHltTst(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the Test Halt system exit.                                     */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RexxString   *condition;             /* Condition(d) information          */
  RXHLTTST_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXHLT);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Clear halt so as not to be sloppy */
    exit_parm.rxhlt_flags.rxfhhalt = 0;
                                       /* init shvexit return buffer        */
    this->nestedInfo.shvexitvalue = OREF_NULL;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXHLT, RXHLTTST, (PVOID)&exit_parm, FALSE))
      return TRUE;                     /* this wasn't handled               */
                                       /* Was halt requested?               */
    if (exit_parm.rxhlt_flags.rxfhhalt == 1) {
                                       /* shv_exit return a value?          */
      if (this->nestedInfo.shvexitvalue == OREF_NULL) {
                                       /* return this information           */
        condition = this->nestedInfo.shvexitvalue;
      }
      else condition = OREF_NULL;
                                       /* Then honor the halt request       */
      activation->halt(condition);
    }

    return FALSE;                      /* this was handled                  */
  }
  return TRUE;                         /* not handled                       */
}

BOOL RexxActivity::sysExitHltClr(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the Clear Halt system exit.                                    */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXHLTTST_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXHLT);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXHLT, RXHLTCLR, (PVOID)&exit_parm, FALSE))
      return TRUE;                     /* this wasn't handled               */
    return FALSE;                      /* this was handled                  */
  }
  return TRUE;                         /* not handled                       */
}

BOOL  RexxActivity::sysExitTrcTst(
     RexxActivation *activation,       /* sending activation                */
     BOOL currentsetting)              /* sending activation                */
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the Test external trace indicator system exit.                 */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXTRCTST_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXTRC);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
                                       /* Clear Trace bit before  call      */
    exit_parm.rxtrc_flags.rxftrace = 0;
                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXTRC, RXTRCTST, (PVOID)&exit_parm, FALSE))
      return TRUE;                     /* this wasn't handled               */
                                       /* if not tracing, and now it is     */
                                       /* requsted                          */
    if ((currentsetting == 0) &&  (exit_parm.rxtrc_flags.rxftrace == 1)) {
                                       /* call routine to handle this       */
      activation->externalTraceOn();
      return FALSE;                    /* this was handled                  */
    }
    else                               /* if currently tracing, and now     */
                                       /* requsted to stop                  */
      if ((currentsetting != 0) &&  (exit_parm.rxtrc_flags.rxftrace != 1)) {
                                       /* call routine to handle this       */
        activation->externalTraceOff();
        return FALSE;                  /* this was handled                  */
      }
  }
  return TRUE;                         /* not handled                       */
}



BOOL  RexxActivity::sysExitDbgTst(
     RexxActivation *activation,       /* sending activation                */
     BOOL currentsetting,
     BOOL EndStepOut_Over)
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the Test external trace indicator system exit.                 */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RexxString   *filename;              /* Exit routine name                 */
  RXDBGTST_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = this->querySysExits(RXDBG);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
    if (!activation->code->u_source->traceable() || (activation->code->u_source->sourceBuffer == OREF_NULL))
       return TRUE;
    if (EndStepOut_Over)
        exit_parm.rxdbg_flags.rxftrace = RXDBGENDSTEP;
    else
        exit_parm.rxdbg_flags.rxftrace = (currentsetting != 0);

    filename = activation->code->getProgramName();
    MAKERXSTRING(exit_parm.rxdbg_filename, filename->stringData, filename->length);
    if (activation->getCurrent() != OREF_NULL)
        exit_parm.rxdbg_line = activation->getCurrent()->lineNumber;
    else
        exit_parm.rxdbg_line = 0;
    exit_parm.rxdbg_routine.strlength = 0;
    exit_parm.rxdbg_routine.strptr = NULL;

                                       /* call the handler                  */
    if (SysExitHandler(this, activation, exitname, RXDBG, RXDBGTST, (PVOID)&exit_parm, FALSE))
      return TRUE;                     /* this wasn't handled               */
                                       /* if not tracing, and now it is     */
                                       /* requsted                          */
     switch (exit_parm.rxdbg_flags.rxftrace)
     {
        case RXDBGSTEPIN: activation->externalDbgStepIn();
                return FALSE;                    /* this was handled                  */
        case RXDBGSTEPOVER: activation->externalDbgStepOver();
                return FALSE;                    /* this was handled                  */
        case RXDBGSTEPOUT: activation->externalDbgStepOut();
                return FALSE;                    /* this was handled                  */
        case RXDBGLOCATELINE: return FALSE;
        case RXDBGSTEPAGAIN: activation->externalDbgStepAgain();
                return FALSE;                    /* this was handled                  */
        case RXDBGRECURSIVEOFF: activation->externalDbgAllOffRecursive();    /* switch off current activation and all parents */
                return FALSE;                    /* this was handled                  */
        default:activation->externalDbgTraceOff();
                return FALSE;                    /* this was handled                  */
     }
  }
  return TRUE;                         /* not handled                       */
}


void  RexxActivity::traceOutput(       /* write a line of trace information */
      RexxActivation *activation,      /* sending activation                */
      RexxString *line)                /* line to write out                 */
/******************************************************************************/
/* Function:  Write a line of trace output to the .ERROR stream               */
/******************************************************************************/
{
  RexxObject *stream;                  /* target stream                     */

  line = line->stringTrace();          /* get traceable form of this        */
                                       /* if exit declines the call         */
  if (this->sysExitSioTrc(activation, line)) {
                                       /* get the default output stream     */
    stream = this->local->at(OREF_ERRORNAME);
                                       /* have .local set up yet?           */
    if (stream != OREF_NULL && stream != TheNilObject)
                                       /* do the lineout                    */
      stream->sendMessage(OREF_LINEOUT, line);
    else                               /* use the "real" default stream     */
      this->lineOut(line);
  }
}

void RexxActivity::sayOutput(          /* write a line of say information   */
     RexxActivation *activation,       /* sending activation                */
     RexxString *line)                 /* line to write out                 */
/******************************************************************************/
/* Function:  Write a line of SAY output to the .OUTPUT stream                */
/******************************************************************************/
{
  RexxObject *stream;                  /* target stream                     */

                                       /* if exit declines the call         */
  if (this->sysExitSioSay(activation, line)) {
                                       /* get the default output stream     */
    stream = this->local->at(OREF_OUTPUT);
                                       /* have .local set up yet?           */
    if (stream != OREF_NULL && stream != TheNilObject)
                                       /* do the lineout                    */
      stream->sendMessage(OREF_SAY, line);
    else                               /* use the "real" default stream     */
      this->lineOut(line);
  }
}

RexxString *RexxActivity::traceInput(  /* read a line of trace input        */
     RexxActivation *activation)       /* sending activation                */
/******************************************************************************/
/* Function:  Read a line for interactive debug input                         */
/******************************************************************************/
{
  RexxString *value;                   /* returned value                    */
  RexxObject *stream;                  /* target stream                     */

                                       /* if exit declines the call         */
  if (this->sysExitSioDtr(activation, &value)) {
                                       /* get the input stream              */
    stream = this->local->at(OREF_INPUT);
    if (stream != OREF_NULL) {         /* have a stream?                    */
                                       /* read from it                      */
      value = (RexxString *)stream->sendMessage(OREF_LINEIN);
                                       /* get something real?               */
      if (value == (RexxString *)TheNilObject)
        value = OREF_NULLSTRING;       /* just us a null string if not      */
    }
    else
      value = OREF_NULLSTRING;         /* default to a null string          */
  }
  return value;                        /* return value from exit            */
}

RexxString *RexxActivity::pullInput(   /* read a line of pull input         */
     RexxActivation *activation)       /* sending activation                */
/******************************************************************************/
/* Function:  Read a line for the PULL instruction                            */
/******************************************************************************/
{
  RexxString *value;                   /* returned value                    */
  RexxObject *stream;                  /* target stream object              */

                                       /* if exit declines call             */
  if (this->sysExitMsqPll(activation, &value)) {
                                       /* get the external data queue       */
    stream = this->local->at(OREF_REXXQUEUE);
    if (stream != OREF_NULL) {         /* have a data queue?                */
                                       /* pull from the queue               */
      value = (RexxString *)stream->sendMessage(OREF_PULL);
                                       /* get something real?               */
      if (value == (RexxString *)TheNilObject)
                                       /* read from the linein stream       */
        value = this->lineIn(activation);
    }
  }
  return value;                        /* return the read value             */
}

RexxObject *RexxActivity::lineOut(
    RexxString *line)                  /* line to write out                 */
/******************************************************************************/
/* Function:  Write a line out to the real default I/O stream                 */
/******************************************************************************/
{
  LONG  length;                        /* length to write out               */
  PCHAR data;                          /* data pointer                      */

  length = line->length;               /* get the string length and the     */
  data = line->stringData;             /* data pointer                      */
  printf("%.*s\n",(int)length, data);  /* print it                          */
  return (RexxObject *)IntegerZero;    /* return on residual count          */
}

RexxString *RexxActivity::lineIn(
    RexxActivation *activation)        /* sending activation                */
/******************************************************************************/
/* Function:  Read a line for the PARSE LINEIN instruction                    */
/******************************************************************************/
{
  RexxObject *stream;                  /* input stream                      */
  RexxString *value;                   /* returned value                    */

                                       /* if exit declines call             */
  if (this->sysExitSioTrd(activation, &value)) {
                                       /* get the input stream              */
    stream = this->local->at(OREF_INPUT);
    if (stream != OREF_NULL) {         /* have a stream?                    */
                                       /* read from it                      */
      value = (RexxString *)stream->sendMessage(OREF_LINEIN);
                                       /* get something real?               */
      if (value == (RexxString *)TheNilObject)
        value = OREF_NULLSTRING;       /* just use a null string if not     */
    }
    else
      value = OREF_NULLSTRING;         /* default to a null string          */
  }
  return value;                        /* return value from exit            */
}

void RexxActivity::queue(              /* write a line to the queue         */
     RexxActivation *activation,       /* sending activation                */
     RexxString *line,                 /* line to write                     */
     int order)                        /* LIFO or FIFO order                */
/******************************************************************************/
/* Function:  Write a line to the external data queue                         */
/******************************************************************************/
{
  RexxObject *queue;                   /* target queue                      */

                                       /* if exit declines call             */
  if (this->sysExitMsqPsh(activation, line, order)) {
                                       /* get the default queue             */
    queue = this->local->at(OREF_REXXQUEUE);
    if (queue != OREF_NULL) {          /* have a data queue?                */
                                       /* pull from the queue               */
      if (order == QUEUE_LIFO)         /* push instruction?                 */
                                       /* push a line                       */
        queue->sendMessage(OREF_PUSH, (RexxObject *)line);
      else
                                       /* queue a line                      */
        queue->sendMessage(OREF_QUEUENAME, (RexxObject *)line);
     }
   }
}

BOOL RexxActivity::isPendingUninit(RexxObject *obj)
/******************************************************************************/
/* Function:  Test if an object is going to require its uninit method run.    */
/******************************************************************************/
{
    /* uninit table for this process     */
    RexxObjectTable * uninittable;
    /* do we have an uninit table? */
    if ((uninittable = TheActivityClass->getUninitTable(this->processObj)) != OREF_NULL) {
        /* is object in the table?           */
        if (uninittable->get(obj) != OREF_NULL) {
            /* this object may require an uninit method run */
            return TRUE;
        }
    }
    return FALSE;
}

void   RexxActivity::addUninitObject(
    RexxObject *obj)                   /* object to add                     */
/******************************************************************************/
/* Function:  Add an object to the process unit table                         */
/******************************************************************************/
{
                                       /* add obj to uninit table for     */
                                       /*  this my process.               */
  TheActivityClass->addUninitObject((RexxObject *)obj, this->processObj);
}

void RexxActivity::removeUninitObject(
    RexxObject *obj)                   /* object to remove                  */
/******************************************************************************/
/* Function:  Remove an object from the process uninit table                  */
/******************************************************************************/
{
                                       /* remove obj from the uninit table*/
                                       /*  for this my process.           */
  TheActivityClass->removeUninitObject((RexxObject *)obj, this->processObj);
}

void  RexxActivity::uninitObject(RexxObject *dropObj)
/******************************************************************************/
/* Function:  Run UNINIT method for a specific object in this activity        */
/******************************************************************************/
/* NOTE: This method will only be called from the drop function in order to   */
/*  run the uninit for the object that has just been dropped. The object will */
/*  then be removed from the uninit table so that uninit will not be called   */
/*  again.                                                                    */
/*                                                                            */
/******************************************************************************/
{
  RexxObjectTable * uninittable;       /* uninit table for this process     */

                                       /* retrive the UNINIT table for      */
                                       /*  process, if it exists            */
  if ((uninittable = TheActivityClass->getUninitTable(this->processObj)) != OREF_NULL) {
                                       /* uninitTabe exists, run UNINIT     */
    if (uninittable->get(dropObj) == TheTrueObject)  {
                                       /* make sure we don't recurse        */
      uninittable->put(TheFalseObject, dropObj);
      dropObj->uninit();               /* yes, indicate run the UNINIT      */
                                       /* remove zombie from uninit table   */
      TheActivityClass->removeUninitObject(dropObj, this->processObj);
    }
  }
}

void  RexxActivity::checkUninits()
/******************************************************************************/
/* FUNCTION: we will run the UNINIT method of all objetcs in the UNINIT       */
/*  table for our process.  Even if the object is "dead", this is because the */
/*  process is going away an its our last chance.  Instead of removing the    */
/*  objects as we go, we run the entire table and then reset table.           */
/*                                                                            */
/******************************************************************************/
{
  RexxObjectTable  * uninittable;      /* uninit table for this process     */
  RexxObject * zombieObj;              /* obj that needs uninit run.        */
  long iterTable;                      /* iterator for table.             */

                                       /* retrive the UNINIT table for    */
                                       /*  process.                       */
                                       /* and protect from GC,            */
                                       /* it is OK to do the save and     */
                                       /* allow general marking of this   */
                                       /* table now, since we are cleaning*/
                                       /* up and UNINITing all meths.     */
  uninittable = TheActivityClass->removeUninitTable(this->processObj);
                                       /* does an uninit table exist for  */
                                       /* process?                        */
  if (uninittable != OREF_NULL) {
    save(uninittable);                 /* yes, protect table from GC      */
                                       /* for all objects in the table    */
    for (iterTable = uninittable->first();
         (zombieObj = uninittable->index(iterTable)) != OREF_NULL;
         iterTable = uninittable->next(iterTable)) {
                                       /* establish the longjmp return point*/
      if (setjmp(this->nestedInfo.jmpenv) == 0)
        zombieObj->uninit();           /* run the UNINIT method           */
    }                                  /* now go check next object in tabl*/

                                       /* all UNINITed, remove uninit tabl*/
    discard(uninittable);              /* from save table                 */
  }
}

void  RexxActivity::startMessages()
/******************************************************************************/
/* Function: Get the list of pending message object that need to be started   */
/*  on this process, and start them.                                          */
/******************************************************************************/
{
  RexxObject  * queuedMessages;        /* collection of queued messages     */
  RexxMessage * currentMessage;        /* Current message object to start.  */

                                       /* Get the list of messages for    */
                                       /* this process.                   */
  queuedMessages = TheActivityClass->getMessageList(this->processObj);
  if (queuedMessages != OREF_NULL) {   /* Is the a list for this process? */
                                       /* Yes, iterate through until no   */
                                       /* more message objects in list.   */
    while ((currentMessage = TheActivityClass->removeNextMessageObject(queuedMessages)) != TheNilObject) {
                                       /* Start the message object-ASYNCH */
      currentMessage->start(OREF_NULL);
    }
  }
}

void  RexxActivity::terminateMethod()
/******************************************************************************/
/* Function: Mark this FREE activity for termination.  Set its exit flag to 1 */
/*   and POST its run semaphore.                                              */
/******************************************************************************/
{
  this->exit = 1;                      /* Activity should exit          */
  EVPOST(this->runsem);                /* let him run so he knows to exi*/
}

void RexxActivityClass::init()
/******************************************************************************/
/* Function:  Initialize the activity class                                   */
/******************************************************************************/
{
  RexxObjectTable *subClassTable;      /* existing subclass table           */

  subClassTable = this->subClasses;    /* save this before clearing         */
  ClearObject(this);                   /* clear the activity class          */
  OrefSet(this,this->classUsedActivities,new_object_table());
  OrefSet(this,this->classFreeActivities,new_object_table());
  OrefSet(this,this->classAllActivities,new_object_table());
  OrefSet(this,this->messageTable, new_object_table());
                                       /* Don't use OrefSet, keep uninit    */
                                       /*  out of Old2New.                  */
  this->uninitTables = new_object_table();
  processingUninits = FALSE;           /* we're not handling uninits currently */
  pendingUninits = 0;                  /* nothing pending currently         */
  if (memoryObject.restoringImage())   /* restoring the image?              */
                                       /* copy this to get it out of the    */
                                       /* old space area                    */
    this->subClasses = (RexxObjectTable *)subClassTable->copy();
  else                                 /* add an object table               */
    this->subClasses = new_object_table();
  OrefSet(this, this->activations, new_stack(ACTIVATION_CACHE_SIZE));
  CurrentActivity = NULL;              /* start out fresh                 */
  current_settings = &defaultSettings; /* use default settings set          */
}

void RexxActivityClass::live()
/******************************************************************************/
/* NOTE: we do not mark the UninitTables.  MEMORY will request the table      */
/*  and mark it for us.  This is so that it can determine if there are        */
/*  any objects that a "dead" and need uninit run.  Activity will run the     */
/*  UNINIT, but we let Garbage Collection, handle detection/etc.              */
/* NOTE: we also do not mark the subClasses table.  This will be managed      */
/*  by memory so that we can reclaim classes once all of the instances have   */
/*  also been reclaimed.                                                      */
/******************************************************************************/
{
  this->RexxClass::live();
  setUpMemoryMark
  memory_mark(this->classUsedActivities);
  memory_mark(this->classFreeActivities);
  memory_mark(this->classAllActivities);
  memory_mark(this->activations);
  memory_mark(this->messageTable);;
  memory_mark(this->firstWaitingActivity);
  memory_mark(this->lastWaitingActivity);
  cleanUpMemoryMark
}

void RexxActivityClass::liveGeneral()
/******************************************************************************/
/* NOTE: we do not mark the UninitTables.  MEMORY will request the table      */
/*  and mark it for us.  This is so that it can determine if there are        */
/*  any objects that a "dead" and need uninit run.  Activity will run the     */
/*  UNINIT, but we let Garbage Collection, handle detection/etc.              */
/*  The subClasses table is only marked during a save image, so that the      */
/*  classes will still have the proper subclass definitions.                  */
/******************************************************************************/
{
  this->RexxClass::liveGeneral();
  if (!memoryObject.savingImage()) {
    setUpMemoryMarkGeneral
    memory_mark_general(this->classUsedActivities);
    memory_mark_general(this->classFreeActivities);
    memory_mark_general(this->classAllActivities);
    memory_mark_general(this->activations);
    memory_mark_general(this->messageTable);
    memory_mark_general(this->firstWaitingActivity);
    memory_mark_general(this->lastWaitingActivity);
    if (memoryObject.restoringImage()) /* restoring or saving the image?    */
                                       /* need to mark the subclass table   */
      memory_mark_general(this->subClasses);
    cleanUpMemoryMarkGeneral
  } else {
    OrefSet(this,this->classUsedActivities,OREF_NULL);
    OrefSet(this,this->classFreeActivities,OREF_NULL);
    OrefSet(this,this->classAllActivities,OREF_NULL);
    OrefSet(this,this->activations,OREF_NULL);
    OrefSet(this,this->uninitTables,OREF_NULL);
    OrefSet(this,this->messageTable,OREF_NULL);
    OrefSet(this,this->firstWaitingActivity,OREF_NULL);
    OrefSet(this,this->lastWaitingActivity,OREF_NULL);
                                       /* this one is marked                */
    memory_mark_general(this->subClasses);
  }
}


void  RexxActivityClass::runUninits()
/******************************************************************************/
/* Function:  Run any UNINIT methods for this activity                        */
/******************************************************************************/
/* NOTE: The routine to iterate across uninit Table isn't quite right, since  */
/*  the removal of zombieObj may move another zombieObj and then doing        */
/*  the next will skip this zombie, we should however catch it next time      */
/*  through.                                                                  */
/*                                                                            */
/******************************************************************************/
{
  RexxObjectTable * uninittable;       /* uninit table for this process     */
  RexxObject * zombieObj;              /* obj that needs uninit run.        */
  long iterTable;                      /* iterator for table.               */

  /* if we're already processing this, don't try to do this */
  /* recursively. */
  if (processingUninits) {
      return;
  }

  /* turn on the recursion flag, and also zero out the count of */
  /* pending uninits to run */
  processingUninits = TRUE;
  pendingUninits = 0;

                                       /* retrive the UNINIT table for      */
                                       /*  process, if it exists            */
  if ((uninittable = getUninitTable(CurrentActivity->processObj)) != OREF_NULL) {
                                       /* uninitTabe exists, run UNINIT     */
    for (iterTable = uninittable->first();
         (zombieObj = uninittable->index(iterTable)) != OREF_NULL;
         iterTable = uninittable->next(iterTable)) {

                                       /* is this object readyfor UNINIT?   */
      if (uninittable->value(iterTable) == TheTrueObject)  {
                                       /* make sure we don't recurse        */
        uninittable->put(TheFalseObject, zombieObj);
        zombieObj->uninit();           /* yes, indicate run the UNINIT      */
                                       /* remove zombie from uninit table   */
        removeUninitObject(zombieObj, CurrentActivity->processObj);
      }
    }                                  /* now go check next object in table */
  }
  /* make sure we remove the recursion protection */
  processingUninits = FALSE;
}


void RexxActivityClass::addWaitingActivity(
    RexxActivity *newActivity,         /* new activity to add to the queue  */
    BOOL          release )            /* need to release the run semaphore */
/******************************************************************************/
/* Function:  Add an activity to the round robin wait queue                   */
/******************************************************************************/
{
  SysEnterResourceSection();           /* now in a critical section         */
                                       /* NOTE:  The following assignments  */
                                       /* do not use OrefSet intentionally. */
                                       /* because we do have yet have kernel*/
                                       /* access, we can't allow memory to  */
                                       /* allocate a new counter object for */
                                       /* this.  This leads to memory       */
                                       /* corruption and unpredictable traps*/
                                       /* nobody waiting yet?               */
  if (this->firstWaitingActivity == OREF_NULL) {
                                       /* this is the head of the chain     */
    this->firstWaitingActivity = newActivity;
                                       /* and the tail                      */
    this->lastWaitingActivity = newActivity;
    SysExitResourceSection();          /* end of the critical section       */
  }
  else {                               /* move to the end of the line       */
                                       /* chain off of the existing one     */
    this->lastWaitingActivity->setNextWaitingActivity(newActivity);
                                       /* this is the new last one          */
    this->lastWaitingActivity = newActivity;
    newActivity->clearWait();          /* clear the run semaphore           */
    SysExitResourceSection();          /* end of the critical section       */
    if (release)                       /* current semaphore owner?          */
      MTXRL(kernel_semaphore);         /* release the lock                  */
    SysThreadYield();                  /* yield the thread                  */
    newActivity->waitKernel();         /* and wait for permission           */
  }
  MTXRQ(kernel_semaphore);             /* request the lock now              */
  SysEnterResourceSection();           /* now remove the waiting one        */
                                       /* NOTE:  The following assignments  */
                                       /* do not use OrefSet intentionally. */
                                       /* because we do have yet have kernel*/
                                       /* access, we can't allow memory to  */
                                       /* allocate a new counter object for */
                                       /* this.  This leads to memory       */
                                       /* corruption and unpredictable traps*/
                                       /* dechain the activity              */

  /* this->firstWaitingActivity will be released, so set first to next of first
     The original outcommented code was setting the first to the next of the
     activity that got the semaphore. This could corrupt the list if threads
     are not released in fifo */

//  this->firstWaitingActivity = newActivity->nextWaitingActivity;   /* this was the orig. wrong connection */
  if (this->firstWaitingActivity) this->firstWaitingActivity = this->firstWaitingActivity->nextWaitingActivity;
                                       /* clear out the chain               */
  /* if we are here, newActivity must have been this->firstWaitingActivity sometime
     before and therefore we can set next pointer to NULL without disturbing
     the linked list */

  newActivity->setNextWaitingActivity(OREF_NULL);
                                       /* was this the only one?            */
  if (!this->firstWaitingActivity)
  {
                                       /* clear out the last one            */
      this->lastWaitingActivity = OREF_NULL;
  }
  else                                 /* release the next one to run       */
  {
      this->firstWaitingActivity->postRelease();
  }
  CurrentActivity = newActivity;       /* set new current activity          */
                                       /* and new active settings           */
  current_settings = newActivity->settings;
  SysExitResourceSection();            /* end of the critical section       */
                                       /* have more pools been added since  */
                                       /* we left the kernel ?              */
  if (GlobalCurrentPool != ProcessCurrentPool)
                                       /* yes, get access to them.          */
    memoryObject.accessPools(ProcessCurrentPool);
}

RexxActivation *RexxActivityClass::newActivation(
     RexxObject     *receiver,         /* message receiver                  */
     RexxMethod     *method,           /* method to run                     */
     RexxActivity   *activity,         /* current activity                  */
     RexxString     *msgname,          /* message name processed            */
     RexxActivation *activation,       /* parent activation                 */
     INT             context )         /* execution context                 */
/******************************************************************************/
/* Function:  Get an activation from cache or create new one                  */
/******************************************************************************/
{
  RexxActivation *newActivation;       /* newly create activation           */

  if (this->activationCacheSize != 0) {/* have a cached entry?              */
    this->activationCacheSize--;       /* remove an entry from the count    */
                                       /* get the top cached entry          */
    newActivation = (RexxActivation *)this->activations->stackTop();
                                       /* reactivate this                   */
    SetObjectHasReferences(newActivation);
    newActivation = new (newActivation) RexxActivation (receiver, method, activity, msgname, activation, context);
    this->activations->pop();          /* Remove reused activation from stac*/
  }
  else {                               /* need to create a new one          */
                                       /* Create new Activation.            */
    newActivation = new RexxActivation (receiver, method, activity, msgname, activation, context);
  }
  return newActivation;                /* return the new activation         */
}

void RexxActivityClass::cacheActivation(
  RexxActivation *activation )         /* activation to process             */
/******************************************************************************/
/* Function:  Save an activation to the cache.                                */
/******************************************************************************/
{
                                       /* still room in the cache?          */
  if (this->activationCacheSize < ACTIVATION_CACHE_SIZE) {
                                       /* free everything for reclamation   */
    SetObjectHasNoReferences(activation);
    this->activationCacheSize++;       /* add the this to the count         */
                                       /* and add the activation            */
    this->activations->push((RexxObject *)activation);
  }
}

RexxActivity *RexxActivityClass::newActivity( long priority, RexxObject *local)
/******************************************************************************/
/* Function:  Create or reuse an activity object                              */
/******************************************************************************/
{
  RexxActivity    *activity;           /* activity to use                   */
  LONG   tid;                          /* current threadid                  */

  if (ProcessName == OREF_NULL) {      /* 1st activity on process?          */
                                       /* Yes, then we initialize processNam*/
                                       /* get the process name              */
                                       /* This will be set into new activity*/
                                       /*  so will be safe from GC soon     */
    ProcessName = (RexxInteger *)SysProcessName();
  }
  save(ProcessName);                   /* GC protect                        */

  SysEnterResourceSection();           /* now in a critical section         */

  activity = OREF_NULL;                /* no activity yet                   */
  if ((int)priority != NO_THREAD)      /* can we reuse one?                 */
  {
                                       /* try to get one from the free table*/
    activity =  (RexxActivity *)TheActivityClass->classFreeActivities->remove(ProcessName);
  }

  if (activity == OREF_NULL) {         /* no luck?                          */
    SysExitResourceSection();          /* end of the critical section       */
                                       /* Create a new activity object      */
    activity = new RexxActivity(FALSE, priority, (RexxDirectory *)local);
    SysEnterResourceSection();         /* now in a critical section         */
                                       /* Add this activity to the table of */
                                       /*  all activities.                  */
    TheActivityClass->classUsedActivities->add(ProcessName, (RexxObject *)activity);
                                       /* add the activity to thread table  */
                                       /*Find the thread id of this activity*/
    tid = activity->threadid;

#ifndef HIGHTID
                                       /* local activities? (process local) */
    if (ProcessLocalActs != OREF_NULL) {
                                       /* Yup, Get current max thread number*/
      size_t maxacts = ProcessLocalActs->size();
      if (maxacts < tid) {             /* Can current array handle this     */
                                       /* no, need to grow the array        */
                                       /* Get new array, based on threadid, */
                                       /*  and round up to 8, (extra room   */
                                       /* to grow                           */
        ProcessLocalActs = ProcessLocalActs->extend(roundup8(tid));
                                       /* Add localacts to all_activities,  */
                                       /*  keep being garbage collected.    */
        TheActivityClass->classAllActivities->put((RexxObject *)ProcessLocalActs, ProcessName);
      }
    }
    else {
                                       /* 1st loacl activity, create        */
                                       /*  localacts array, size based on   */
                                       /* current thread id                 */
      ProcessLocalActs = (RexxArray *)new_array(roundup8(tid));
                                       /* Add localacts array to keep table */
      TheActivityClass->classAllActivities->put((RexxObject *)ProcessLocalActs, ProcessName);
    }
                                       /* Add new activity to localacts     */
    ProcessLocalActs->put((RexxObject *)activity, tid);
    ProcessNumActs++;                  /* got an active activity            */

#else
    if (ProcessLocalActs != OREF_NULL) {
                                       /* Yup, Get current max thread number*/
                                       /* Add localacts to all_activities,  */
                                       /*  keep being garbage collected.    */
//        TheActivityClass->classAllActivities->put((RexxObject *)ProcessLocalActs, ProcessName);
    }
    else {
                                       /* 1st loacl activity, create        */
                                       /*  localacts array, size based on   */
                                       /* current thread id                 */
      ProcessLocalActs = new ActivityTable();
                                       /* Add localacts array to keep table */
//      TheActivityClass->classAllActivities->put((RexxObject *)ProcessLocalActs, ProcessName);

    }
//    HighTidLastID = 0;
                                       /* Add new activity to localacts     */
    ProcessLocalActs->put((RexxObject *)activity, tid);
    ProcessNumActs++;                  /* got an active activity            */
#endif

  }
  else {
                                       /* We are able to reuse an activity, */
                                       /*  so just re-initialize it.        */
    new (activity) RexxActivity(TRUE, priority, (RexxDirectory *)local);
                                       /* add activity to loclaActs.  Array */
                                       /* is big enough since existing      */
                                       /*Activity                           */
#ifdef HIGHTID_0
    ProcessLocalActs->put((RexxObject *)activity, ID2String(activity->threadid));
    /* keep higher performance on HIGHTID up to date */
    HighTidLastID = 0;
#else
    ProcessLocalActs->put((RexxObject *)activity, activity->threadid);
#endif
  }
  SysExitResourceSection();            /* end of the critical section       */
  discard(ProcessName);                /* GC "unprotect"                    */
  return activity;                     /* return the activity               */
}

void RexxActivityClass::addUninitObject(
    RexxObject *obj,                   /* object to add                     */
    RexxObject *processobj)            /* object's target process           */
/******************************************************************************/
/* Function:  Add an object with an uninit method to the uninit table for     */
/*            a process                                                       */
/******************************************************************************/
{
   RexxObjectTable *uninitTable;       /* uninit Table for assoc w/ procc   */
                                       /* retrieve process uninit table     */
                                       /* does an uninit Table exist for    */
                                       /* this process?                     */
   if ((uninitTable = (RexxObjectTable *)this->uninitTables->get(processobj)) == OREF_NULL) {
     uninitTable = new_object_table(); /* No, create a new table            */
                                       /* and add it to the uninitTables    */
                                       /* under this process name.          */
     this->uninitTables->put(uninitTable, processobj);
   }
                                       /* is object already in table?       */
   if (uninitTable->get(obj) == OREF_NULL) {
                                       /* nope, add obj to uninitTable,     */
                                       /*  initial value is NIL             */
     uninitTable->put(TheNilObject, obj);
   }

}

void  RexxActivityClass::removeUninitObject(
    RexxObject *obj,                   /* object to remove                  */
    RexxObject *processobj)            /* target process                    */
/******************************************************************************/
/* Function:  Remove an object from the uninit tables                         */
/******************************************************************************/
{
  RexxObjectTable *uninitTable;        /* uninitTable for this process      */
                                       /* retrieve the uninitTbale for      */
                                       /* this process.                     */
  uninitTable = (RexxObjectTable *)this->uninitTables->get(processobj);
                                       /* remove obj fromthe uninit table   */
                                       /*  for the process identified by    */
                                       /*  proccessobj                      */
  uninitTable->remove(obj);
  if (uninitTable->items() == 0) {     /* anything left in the table?       */
                                       /* no remove process table from      */
                                       /* uninit Tables.                    */
    this->uninitTables->remove(processobj);
  }
}

BOOL  RexxActivityClass::addMessageObject(
    RexxObject *msgObj,                /* added message object              */
    RexxObject *processobj)            /* target process id                 */
/******************************************************************************/
/* Function:  Add a message object to another processes message table         */
/******************************************************************************/
{
   RexxList * messageList;             /* message Table for assoc w/ procc  */
                                       /* retrieve process uninit table     */
                                       /* does an uninit Tbale exist for    */
                                       /* this process?                     */
   if ((messageList = (RexxList *)this->messageTable->get(processobj)) == OREF_NULL) {
     messageList = new_list();         /* No, create a new list             */
                                       /* and add it to the messageTable    */
                                       /* under this process name.          */
     this->messageTable->put((RexxObject *)messageList, processobj);
   }
                                       /* did process terminate?            */
   else if ((RexxObject *)messageList == (RexxObject *)TheFalseObject)
     return FALSE;                     /* indicate couldn't queue message   */

   messageList->addFirst(msgObj);      /* put new msg obj at front of list  */
   return TRUE;
}

void RexxActivityClass::terminateFreeActs()
/******************************************************************************/
/* Function:   see if there are any Uninit messages need to be send before    */
/*             the process goes away.                                         */
/******************************************************************************/
{
  RexxObject * activity;               /* activity to free up               */

                                       /* have more pools been added since  */
                                       /* we left the kernel ?              */
  if (GlobalCurrentPool != ProcessCurrentPool)
                                       /* yes, get access to them.          */
    memoryObject.accessPools(ProcessCurrentPool);
                                       /* for all activities on freeList    */
  for (activity = this->classFreeActivities->remove(ProcessName);
       activity != OREF_NULL;
       activity = this->classFreeActivities->remove(ProcessName)) {
                                       /* Make sure they terminate.         */
    ((RexxActivity *)activity)->terminateMethod();
  }
}

BOOL activity_halt(
     LONG         thread_id,           /* target thread id                  */
     RexxString * description )        /* description to use                */
/******************************************************************************/
/* Function:   Flip on a bit in a target activities top activation            */
/******************************************************************************/
{
  RexxActivity *activity;              /* target activity                   */
  RexxActivation *activation;          /* target activation                 */
  BOOL result;                         /* return value                      */

  result = FALSE;                      /* assume failure                    */
  /* no need for critical section, since access here is read-only */
  MTXRQ(resource_semaphore);           /* lock activity changes             */
  if (ProcessLocalActs != OREF_NULL) { /* activities created?               */
                                       /* possible thread_id?               */
#ifndef HIGHTID
    if (ProcessLocalActs->size() >= thread_id) {
      activity = (RexxActivity *)ProcessLocalActs->get(thread_id);
#else
    {
      activity = (RexxActivity *)ProcessLocalActs->fastAt(thread_id);
#endif
                                       /* Activity exist for thread?        */
      if (activity != (RexxActivity *)OREF_NULL) {
                                       /* get the current activation        */
        activation = (RexxActivation *)activity->currentActivation;
                                       /* got an activation?                */
        if (activation != (RexxActivation *)TheNilObject) {
                                       /* process the halt                  */
          activation->halt(description);
          result = TRUE;               /* this actually worked              */
        }
      }
    }
  }
  MTXRL(resource_semaphore);           /* unlock the resources              */
  return result;                       /* return the result                 */
}

BOOL activity_sysyield(
     LONG         thread_id,           /* target thread id                  */
     RexxObject * description )        /* description to use                */
/****************************************************************************/
/* Function:   Flip on a bit in a target activities top activation          */
/*             called from rexxsetyield                                     */
/****************************************************************************/
{
  RexxActivity *activity;              /* target activity                   */
  RexxActivation *activation;          /* target activation                 */
  BOOL result;                         /* return value                      */

  result = FALSE;                      /* assume failure                    */
  /* no need for critical section, since access here is read-only */
  MTXRQ(resource_semaphore);           /* lock activity changes             */
  if (ProcessLocalActs != OREF_NULL) { /* activities created?               */
                                       /* possible thread_id?               */
#ifndef FIXEDTIMERS0
#ifndef HIGHTID
    if (ProcessLocalActs->size() >= thread_id) {
      activity = (RexxActivity *)ProcessLocalActs->get(thread_id);
#else
    {
      activity = (RexxActivity *)ProcessLocalActs->fastAt(thread_id);
#endif
      if (activity != OREF_NULL) {
                                       /* get the current activation        */
        activation = activity->currentActivation;
                                       /* got an activation?                */
        if ((activation != NULL) &&
           (activation != (RexxActivation *)TheNilObject)) {
                                       /* process the yield                 */
          activation->yield();
          result = TRUE;               /* this actually worked              */
        }
      }
    }
#else
        if (LastRunningActivity)
        {
       activation = LastRunningActivity->currentActivation;
                                       /* got an activation?                */
       if ((activation != NULL) &&
          (activation != (RexxActivation *)TheNilObject))
       {                                       /* process the yield                 */
          activation->yield();
          result = TRUE;               /* this actually worked              */
       }
        }
        else
        /* get the first valid activity and activation */
    while ((!result) && (ProcessLocalActs->size() >= thread_id))
    {
      activity = (RexxActivity *)ProcessLocalActs->get(thread_id);
      if (activity != OREF_NULL) {
                                       /* get the current activation        */
         activation = activity->currentActivation;
                                       /* got an activation?                */
         if ((activation != NULL) &&
            (activation != (RexxActivation *)TheNilObject)) {
                                       /* process the yield                 */
            activation->yield();
            result = TRUE;               /* this actually worked              */
         }
      }
          thread_id++;
    }
#endif
  }
  MTXRL(resource_semaphore);           /* unlock the resources              */
  return result;                       /* return the result                 */
}

BOOL activity_set_trace(
     LONG  thread_id,                  /* target thread id                  */
     BOOL  on_or_off )                 /* trace on/off flag                 */
/******************************************************************************/
/* Function:   Flip on a bit in a target activities top activation            */
/******************************************************************************/
{
  RexxActivity   *activity;            /* target activity                   */
  RexxActivation *activation;          /* target activation                 */
  BOOL result;                         /* return value                      */

  result = FALSE;                      /* assume failure                    */
  /* no need for critical section, since access here is read-only */
  MTXRQ(resource_semaphore);           /* lock activity changes             */
  if (ProcessLocalActs != OREF_NULL) { /* activities created?               */
                                       /* possible thread_id?               */
#ifndef HIGHTID
    if (ProcessLocalActs->size() >= thread_id) {
      activity = (RexxActivity *)ProcessLocalActs->get(thread_id);
#else
    {
      activity = (RexxActivity *)ProcessLocalActs->fastAt(thread_id);
#endif
                                       /* Activity exist for thread?        */
      if (activity != OREF_NULL) {
                                       /* get the current activation        */
        activation = activity->currentAct();
                                       /* got an activation?                */
        if ((activation != NULL) &&
           (activation != (RexxActivation *)TheNilObject)) {
          if (on_or_off)               /* turning this on?                  */
                                       /* turn tracing on                   */
            activation->externalTraceOn();
          else
                                       /* turn tracing off                  */
            activation->externalTraceOff();
          result = TRUE;               /* this actually worked              */
        }
      }
    }
  }
  MTXRL(resource_semaphore);           /* unlock the resources              */
  return result;                       /* return the result                 */
}

void activity_set_yield(void)
/******************************************************************************/
/* Function:   Signal an activation to yield control                          */
/******************************************************************************/
{
  RexxActivation *activation;          /* target activation                 */

  /* no need for critical section, since access here is read-only */
  MTXRQ(resource_semaphore);           /* lock activity changes             */
  if (CurrentActivity != OREF_NULL) {  /* something working?                */
                                       /* get the current activation        */
    activation = CurrentActivity->currentActivation;
                                       /* got an activation?                */
    if ((activation != NULL) &&
       (activation != (RexxActivation *)TheNilObject))
                                       /* tell it to yield                  */
      activation->yield();
  }
  MTXRL(resource_semaphore);           /* unlock the resources              */
}

void activity_create (void)
/******************************************************************************/
/* Function:  Create the activity class during save image processing          */
/******************************************************************************/
{
                                       /* create the class object           */
  create_udsubClass(Activity, RexxActivityClass);
                                       /* and do class-specific init        */
  new (TheActivityClass) RexxActivityClass();
                                       /* hook up the class and behaviour   */
  ((RexxBehaviour *)TheActivityBehaviour)->setClass((RexxClass *)TheActivityClass);
}

void activity_restore (void)
/******************************************************************************/
/* Function:  Restore the activity class during start up                      */
/******************************************************************************/
{
                                       /* now go reinitialize               */
  TheActivityClass->init();
}


RexxActivity *activity_find (void)
/******************************************************************************/
/* Function:  Locate the activity associated with a thread                    */
/******************************************************************************/
{
  RexxActivity *activity;              /* returned activity                 */
  int  threadid;                       /* current thread id                 */

  activity = OREF_NULL;                /* assume failure                    */
  if (ProcessLocalActs != OREF_NULL) { /* something active?                 */
    threadid = SysQueryThreadID();     /* get the thread id                 */
                                       /* got one possible?                 */

                                       /* pull it from the array            */
#ifndef HIGHTID
    if (ProcessLocalActs->size() >= threadid) {
      activity = (RexxActivity *)ProcessLocalActs->get(threadid);
        }
#else
          /* to get a better performance */
//      if ((HighTidLastID) && (HighTidLastID == threadid))
//             activity = HighTidLastActivity;

//      if ((!HighTidLastID) || (HighTidLastID != threadid) || (!activity))
//          {
//         activity = (RexxActivity *)ProcessLocalActs->fastAt(ID2String(threadid));
//                 HighTidLastActivity = activity;
//                 HighTidLastID = threadid;
//          }
    activity = (RexxActivity *)ProcessLocalActs->fastAt(threadid);
#endif
  }
  return activity;                     /* return the located activity       */
}

void activity_exit (int retcode)
/******************************************************************************/
/* Function:  Really shut down--this exits the process                        */
/******************************************************************************/
{
  exit(retcode);
}

void activity_lock_kernel(void)
/******************************************************************************/
/* Function:  Request access to the kernel                                    */
/******************************************************************************/
{
  MTXRQ(kernel_semaphore);             /* just request the semaphore        */
                                       /* have more pools been added since  */
                                       /* we left the kernel ?              */
  if (GlobalCurrentPool != ProcessCurrentPool)
                                       /* yes, get access to them.          */
    memoryObject.accessPools(ProcessCurrentPool);
}

void activity_unlock_kernel(void)
/******************************************************************************/
/* Function:  Release the kernel access                                       */
/******************************************************************************/
{
  CurrentActivity = OREF_NULL;         /* no current activation             */
  MTXRL(kernel_semaphore);             /* release the kernel semaphore      */
}

void RexxActivityClass::returnActivity(
    RexxActivity *activityObject )     /* returned activity                 */
/******************************************************************************/
/* Function:  Return access to an activity previously obtained from           */
/*            getActivity().  This will handle activity nesting and also      */
/*            release the kernel semaphore.                                   */
/******************************************************************************/
{
  LONG   numberActivities;             /* current number of activities      */

  activityObject->nestedCount--;       /* decrease the nesting              */
  numberActivities = --ProcessNumActs; /* decrement number of activities    */
                                       /* not a nested call?                */
  if (activityObject->nestedCount == 0) {
    if (numberActivities == 0)         /* are we the last thread?           */
      activityObject->checkUninits();  /* process uninits                   */
    SysEnterResourceSection();         /* now in a critical section         */
                                       /* This activity is nolonger in use. */
    this->classUsedActivities->remove((RexxObject *)activityObject);
                                       /* remove from the local acts list   */
#ifdef HIGHTID
    /* keep higher performance on HIGHTID up to date */
//    HighTidLastID = 0;
    ProcessLocalActs->put(OREF_NULL, activityObject->threadid);
    EVCLOSE(activityObject->runsem);
    EVCLOSE(activityObject->guardsem);
#ifdef THREADHANDLE
    EVCLOSE(activityObject->hThread);
#endif
#else
    ProcessLocalActs->put(OREF_NULL, activityObject->threadid);
#endif
    SysExitResourceSection();          /* end of the critical section       */
                                       /* Are we terminating?               */
    if (ProcessTerminating && numberActivities == 0)
      kernelShutdown();                /* time to shut things down          */
  }
  ReleaseKernelAccess(activityObject); /* release the kernel semaphore      */
}

RexxActivity *RexxActivityClass::getActivity(void)
/******************************************************************************/
/* Function:  Determine the activity (or create a new one) for the thread     */
/*            we are about to enter kernel to send a message.  We return      */
/*            the activity object to be used to send the message.             */
/*            The routine requests kernel access on the new activity before   */
/*            returning.                                                      */
/******************************************************************************/
{
  RexxActivity *activityObject;        /* Activity object for this thread   */

                                       /* first see if activity already     */
                                       /* exists for this thread in this    */
  activityObject = activity_find();    /* process.                          */
  if (activityObject == OREF_NULL) {   /* Nope, 1st time through here.      */
    MTXRQ(kernel_semaphore);           /* get the kernel semophore          */
                                       /* have more pools been added since  */
                                       /* we left the kernel ?              */
    if (GlobalCurrentPool != ProcessCurrentPool)
                                       /* yes, get access to them.          */
      memoryObject.accessPools(ProcessCurrentPool);
                                       /* Get a new activity object.        */
    activityObject = this->newActivity(NO_THREAD, ProcessLocalEnv);
    MTXRL(kernel_semaphore);           /* release kernel semaphore          */
                                       /* Reacquire kernel semaphore on new */
                                       /*  activity.                        */
    RequestKernelAccess(activityObject);
  }
  else {
                                       /* Activity already existed for this */
                                       /* get kernel semophore in activity  */
    RequestKernelAccess(activityObject);
    ProcessNumActs++;                  /* got an active activity            */
    SysEnterResourceSection();         /* now in a critical section         */
                                       /* See if this activity is already in*/
                                       /*  use, are we re-entering kernel?  */
    if (this->classUsedActivities->get((RexxObject *)activityObject) == OREF_NULL) {
                                       /* not re-entering, but were here    */
                                       /* before add this activity to inuse */
                                       /* activities                        */
      this->classUsedActivities->add(ProcessName, (RexxObject *)activityObject);
    }
    SysExitResourceSection();          /* end of the critical section       */
  }
  activityObject->nestedCount++;       /* step the nesting count            */
  return activityObject;               /* Return the activity for thread    */
}

void process_message_arguments(
  va_list  *arguments,                 /* variable argument list pointer    */
  PCHAR     interfacedefn,             /* interface definition              */
  RexxList *argument_list )            /* returned list of arguments        */
/******************************************************************************/
/* Function:  Send a message to an object on behalf of an outside agent.      */
/*            Message arguments and return type are described by the          */
/*            interface string.                                               */
/******************************************************************************/
{
  LONG     i;                          /* loop counter/array index          */
  PVOID    tempPointer;                /* temp converted pointer            */
  RXSTRING tempRXSTRING;               /* temp argument rxstring            */
  OREF     tempOREF;                   /* temp argument object reference    */
  LONG     tempLong;                   /* temp converted long               */
  ULONG    tempULong;                  /* temp converted long               */
  CHAR     tempChar;                   /* temp character value              */
  double   tempDouble;                 /* temp double value                 */
  va_list *subArguments;               /* indirect argument descriptor      */
  PCHAR    subInterface;               /* indirect interface definition     */

  while (*interfacedefn) {             /* process each argument             */
    switch (*interfacedefn++) {        /* process the next argument         */

      case '*':                        /* indirect reference                */
                                       /* get the real interface pointer    */
        subInterface = va_arg(*arguments, PCHAR);
                                       /* get the indirect pointer          */
        subArguments = va_arg(*arguments, va_list *);
                                       /* go process recursively            */
        process_message_arguments(subArguments, subInterface, argument_list);
        break;

      case 'b':                        /* BYTE                              */
      case 'c':                        /* CHARACTER                         */
                                       /* get the character                 */
        tempChar = (CHAR) va_arg(*arguments, INT);
                                       /* create a string object            */
        argument_list->addLast(new_string(&tempChar, 1));
        break;

      case 'i':                        /* INT                               */
                                       /* get the number                    */
        tempLong = va_arg(*arguments, INT);
                                       /* create an integer object          */
        argument_list->addLast(new_integer(tempLong));
        break;

      case 's':                        /* SHORT                             */
                                       /* get the number                    */
        tempLong = (LONG) (SHORT) va_arg(*arguments, INT);
                                       /* create an integer object          */
        argument_list->addLast(new_integer(tempLong));
        break;

      case 'd':                        /* double                            */
      case 'f':                        /* floating point                    */
                                       /* get a double value                */
        tempDouble = va_arg(*arguments, double);
                                       /* convert to string form            */
        argument_list->addLast(new_stringd(&tempDouble));
        break;

      case 'g':                        /* ULONG                             */
                                       /* get the number                    */
        tempULong = va_arg(*arguments, ULONG);
                                       /* create an integer object          */
        argument_list->addLast(new_numberstring((stringsize_t)tempULong));
        break;

      case 'h':                        /* USHORT                            */
                                       /* get the number                    */
        tempLong = (LONG) (USHORT) va_arg(*arguments, INT);
                                       /* create an integer object          */
        argument_list->addLast(new_integer(tempLong));
        break;

      case 'l':                        /* LONG                              */
                                       /* get the number                    */
        tempLong = va_arg(*arguments, LONG);
                                       /* create an integer object          */
        argument_list->addLast(new_integer(tempLong));
        break;

      case 'o':                        /* REXX object reference             */
                                       /* get the OREF                      */
        tempOREF = va_arg(*arguments, OREF);
                                       /* insert directly into the array    */
        argument_list->addLast(tempOREF);
        break;

#ifdef SOM
      case 'O':                        /* SOM object reference              */
                                       /* get the SOM object pointer        */
        tempPointer = va_arg(*arguments, PVOID);
                                       /* convert to a real SOM object      */
        tempOREF = resolve_proxy((SOMObject *)tempPointer);
                                       /* put the proxy into the array      */
        argument_list->addLast(tempOREF);
        break;
#endif

      case 'A':                        /* REXX array of objects             */
                                       /* get the OREF                      */
        {
          RexxArray *tempArray;
          tempArray = va_arg(*arguments, RexxArray *);
                                       /* get the array size                */
          tempLong = tempArray->size();
                                       /* for each argument,                */
          for (i = 1; i <= tempLong; i++) {
                                       /* copy into the argument list       */
            argument_list->addLast(tempArray->get(i));
          }
        }
        break;

      case 'r':                        /* RXSTRING                          */
                                       /* get the RXSTRING                  */
        tempRXSTRING = va_arg(*arguments, RXSTRING);
                                       /* create a string object            */
        argument_list->addLast(new_string(tempRXSTRING.strptr, tempRXSTRING.strlength));
        break;

      case 'n':                        /* pointer to somId                  */
      case 'p':                        /* POINTER                           */
      case 't':                        /* Token                             */
      case 'B':                        /* Byte pointer                      */
      case 'C':                        /* Character pointer                 */
      case 'L':                        /* Pointer to LONG                   */
      case 'V':                        /* VOID *?                           */
      case 'R':                        /* RXSTRING *                        */
                                       /* get the pointer                   */
        tempPointer = va_arg(*arguments, void *);
                                       /* create a pointer object           */
        argument_list->addLast(new_pointer(tempPointer));
        break;

      case 'Z':                        /* ASCII-Z string - from SOM         */
      case 'z':                        /* ASCII-Z string                    */
                                       /* get the pointer                   */
        tempPointer = va_arg(*arguments, void *);
                                       /* create a string object            */
        argument_list->addLast(new_cstring((PCHAR)tempPointer));
        break;
    }
  }
}

void process_message_result(
  RexxObject *value,                   /* returned value                    */
  PVOID    return_pointer,             /* pointer to return value location  */
  CHAR     interfacedefn )             /* interface definition              */
/******************************************************************************/
/* Function:  Convert an OREF return value into the requested message return  */
/*            type.                                                           */
/******************************************************************************/
{
  RexxObject *object_id = (RexxObject*) IntegerZero; /* object SOM id       */

  switch (interfacedefn) {             /* process the return type           */

      case 'b':                        /* BOOLEAN                           */
                                       /* get the number                    */
        (*((BOOL *)return_pointer)) = value->longValue(NO_LONG);
        break;
      case 'c':                        /* CHARACTER                         */
                                       /* get the first character           */
        (*((CHAR *)return_pointer)) = ((RexxString *)value)->getChar(0);
        break;

      case 'i':                        /* INT                               */
                                       /* get the number                    */
        (*((INT *)return_pointer)) = (INT)value->longValue(NO_LONG);
        break;

      case 's':                        /* SHORT                             */
                                       /* get the number                    */
        (*((SHORT *)return_pointer)) = (SHORT)value->longValue(NO_LONG);
        break;

      case 'd':                        /* double                            */
      case 'f':                        /* floating point                    */
                                       /* get the double                    */
        (*((double *)return_pointer)) = value->doubleValue();
        break;

      case 'g':                        /* ULONG                             */
                                       /* get the number                    */
        (*((ULONG *)return_pointer)) = (ULONG)value->longValue(NO_LONG);
        break;

      case 'h':                        /* USHORT                            */
                                       /* get the number                    */
        (*((USHORT *)return_pointer)) = (USHORT)value->longValue(NO_LONG);
        break;

      case 'l':                        /* LONG                              */
                                       /* get the number                    */
        (*((ULONG *)return_pointer)) = (ULONG)value->longValue(NO_LONG);
        break;

      case 'o':                        /* REXX object reference             */
                                       /* copy the value directly           */
        (*((OREF *)return_pointer)) = value;
        break;

#ifdef SOM
      case 'O':                        /* SOM object reference              */
                                       /* get the object's id               */
        object_id = send_message1(ProcessLocalServer, OREF_SOMLOOK, value);
                                       /* copy the value directly           */
        if (object_id == TheNilObject) /* no id for this one?               */
                                       /* don't return anything             */
          (*((PVOID *)return_pointer)) = NULL;
        else
                                       /* return the SOM object pointer     */
          (*((PVOID *)return_pointer)) = (void *)((RexxInteger *)object_id)->value;
        break;
#endif

      case 'n':                        /* pointer to somId                  */
      case 'p':                        /* POINTER                           */
      case 't':                        /* Token                             */
      case 'B':                        /* Byte pointer                      */
      case 'C':                        /* Character pointer                 */
      case 'L':                        /* Pointer to LONG                   */
      case 'V':                        /* VOID *?                           */
      case 'R':                        /* RXSTRING *                        */
                                       /* get the pointer value             */
          (*((PVOID *)return_pointer)) = (void *)((RexxInteger *)object_id)->value;
        break;

      case 'v':                        /* nothing returned at all           */
        break;
      case 'z':                        /* ASCII-Z string                    */
                                       /* Force to a string.                */
        value = value->stringValue();
        (*((char **)return_pointer)) = ((RexxString *)value)->stringData;
        break;
#ifdef SOM
      case 'Z':                        /* ASCII-Z string - for SOM          */
                                       /* storage is obtained, through      */
                                       /*  SOMMalloc, caller is expected to */
                                       /*  SOMFree.                         */
       {                               /* Force to a string.                */
        RexxString *stringValue;
        stringValue = value->stringValue();
                                       /* SOMMalloc storgae we need         */
        (*((char **)return_pointer)) = (char *)SOMMalloc(stringValue->length);
                                       /* copy the string data into new buff*/
        strcpy((*((char **)return_pointer)), stringValue->stringData);
       }
        break;
#endif
  }
}

LONG RexxActivity::messageSend(
    RexxObject      *receiver,         /* target object                     */
    RexxString      *msgname,          /* name of the message to process    */
    LONG             count,            /* count of arguments                */
    RexxObject     **arguments,        /* array of arguments                */
    RexxObject     **result )          /* message result                    */
/******************************************************************************/
/* Function:    send a message (with message lookup) to an object.  This      */
/*              method will do any needed activity setup before hand.         */
/******************************************************************************/
{
  INT     jmprc;                       /* setjmp return code                */
  LONG    rc;                          /* message return code               */
  SYSEXCEPTIONBLOCK exreg;             /* system specific exception info    */
  long    startDepth;                  /* starting depth of activation stack*/
  nestedActivityInfo saveInfo;         /* saved activity info               */

  rc = 0;                              /* default to a clean return         */
  *result = OREF_NULL;                 /* default to no return value        */
  this->saveNestedInfo(&saveInfo);     /* save critical nesting info        */
                                       /* make sure we have the stack base  */
  this->nestedInfo.stackptr = SysGetThreadStackBase(TOTAL_STACK_SIZE);
  this->clearExits();                  /* make sure the exits are cleared   */
  this->generateRandomNumberSeed();    /* get a fresh random seed           */
                                       /* Push marker onto stack so we know */
  this->pushNil();                     /* what level we entered.            */
  startDepth = this->depth;            /* Remember activation stack depth   */

  SysRegisterSignals(&exreg);          /* register our signal handlers      */
                                       /* INitialize and Windowing stuff    */
  this->windowInfo = SysInitializeWindowEnv();
                                       /* set up setjmp environment         */
  jmprc = setjmp(this->nestedInfo.jmpenv);
  if (jmprc != 0)                      /* did we get an error return?       */
    rc = this->error(startDepth);      /* do error cleanup                  */
  else
                                       /* issue a straight message send     */
    *result = receiver->messageSend(msgname, count, arguments);
                                       /* objects with UNINIT defined may be*/
                                       /* on the savestack. to ensure that  */
                                       /* UNINIT can run, the stack has to  */
                                       /* be cleared and a GC cycle has to  */
  TheMemoryObject->clearSaveStack();   /* be forced to remove any potential */
  TheMemoryObject->collect();          /* locks from the UNINIT table       */
  TheActivityClass->runUninits();      /* be sure to finish UNINIT methods  */
  this->restoreNestedInfo(&saveInfo);  /* now restore to previous nesting   */
                                       /* Do WIndow cleanup stuff.          */
  SysTerminateWindowEnv(this->windowInfo);
  this->windowInfo = NULL;             /* and clean up the window info      */
  SysDeregisterSignals(&exreg);        /* deregister the signal handlers    */
  this->popNil();                      /* remove the nil marker             */
  return rc;                           /* return the error code             */
}

#include "RexxNativeAPI.h"             /* bring in the external definitions */

LONG VLAREXXENTRY RexxSendMessage (
  REXXOBJECT  receiver,                /* receiving object                  */
  PCHAR msgname,                       /* message to send                   */
  REXXOBJECT  start_class,             /* lookup starting class             */
  PCHAR interfacedefn,                 /* argument, return value definition */
  void *result_pointer,                /* pointer to returned result        */
  ... )                                /* variable number of arguments      */
/******************************************************************************/
/* Function:  Send a message to an object on behalf of an outside agent.      */
/*            Message arguments and return type are described by the          */
/*            interface string.                                               */
/******************************************************************************/
{
  INT  jmprc;                          /* setjmp return code                */
  RexxActivity *activity;              /* target activity                   */
  RexxObject *result;                  /* returned result object            */
  RexxArray  *argument_array;          /* array of arguments                */
  RexxList   *argument_list;           /* temp list of arguments            */
  CHAR returnType;                     /* type of return value              */
  LONG rc;                             /* message return code               */
  va_list arguments;                   /* message argument list             */
  SYSEXCEPTIONBLOCK exreg;             /* system specific exception info    */
  nestedActivityInfo saveInfo;         /* saved activity info               */
  long startDepth;

  jmprc = 0;
  rc = 0;                              /* default to a clean return         */
                                       /* Find an activity for this thread  */
                                       /* (will create one if necessary)    */
  activity = TheActivityClass->getActivity();
  activity->saveNestedInfo(&saveInfo); /* save the nested stuff             */
  activity->clearExits();              /* make sure the exits are cleared   */
  activity->generateRandomNumberSeed();/* get a fresh random seed           */
                                       /* Push marker onto stack so we know */
  activity->pushNil();                 /* what level we entered.            */
  startDepth = activity->depth;        /* Remember activation stack depth   */
  SysRegisterSignals(&exreg);          /* register our signal handlers      */
                                       /* INitialize and Windowing stuff    */
  activity->windowInfo = SysInitializeWindowEnv();
                                       /* set up setjmp environment         */
  jmprc = setjmp(activity->nestedInfo.jmpenv);
  if (jmprc != 0) {                    /* did we get an error return?       */
                                       /* do error cleanup                  */
    rc = activity->error(startDepth);
    result = OREF_NULL;                /* no result in this case            */
  }
  else {
#ifdef SOM
    if (*interfacedefn == '&') {       /* is the receiver a som object?     */
                                       /* Yes, resolve to OREXX proxy       */
      receiver = resolve_proxy((SOMObject *)receiver);
      interfacedefn++;                 /* bump past receiver interface      */

      if (start_class != OREF_NULL) {  /* Was a string class specified?     */
                                       /* Yup, its was as a SOMClass.       */
                                       /*  so resolve to OREXX class.       */
        start_class = send_message2(ProcessLocalServer, OREF_IMPORT, new_cstring(((SOMClass*)start_class)->somGetName()), TheNilObject);
      }
    }
#endif
    returnType = *interfacedefn++;     /* Get the return type.              */
                                       /* get the argument list start       */
    va_start(arguments, result_pointer);
                                       /* create an argument list           */
    argument_list  = (RexxList *)save(new_list());
                                       /* go convert the arguments          */
    process_message_arguments(&arguments, interfacedefn, argument_list);
                                       /* now convert to an array           */
    argument_array = (RexxArray *)save(argument_list->makeArray());
    discard(argument_list);            /* No longer need the list.          */
    va_end(arguments);                 /* end of argument processing        */
    if (start_class == OREF_NULL)      /* no start scope given?             */
                                       /* issue a straight message send     */
      result = receiver->messageSend(new_cstring(msgname)->upper(),
          argument_array->size(), argument_array->data());
    else
                                       /* go issue the message with override*/
      result = receiver->messageSend(new_cstring(msgname)->upper(),
          argument_array->size(), argument_array->data(), start_class);
    discard(argument_array);           /* no longer need the argument array */
    if (result != OREF_NULL) {         /* if we got a result, protect it.   */
      result = save(result);           /* because might not have references */
                                       /* convert the return result         */
      process_message_result(result, result_pointer, returnType);
    }
  }
  TheActivityClass->runUninits();      /* be sure to finish UNINIT methods  */
                                       /* restore the nested information    */
  activity->restoreNestedInfo(&saveInfo);
                                       /* Do WIndow cleanup stuff.          */
  SysTerminateWindowEnv(activity->windowInfo);
  activity->windowInfo = NULL;         /* clear the window info             */
  SysDeregisterSignals(&exreg);        /* deregister the signal handlers    */
                                       /* if had a real result object       */
                                       /*  and not returning an OREF....    */
  if (result != OREF_NULL) {
    if (returnType == 'o' || returnType == 'z')
                                       /* Need to keep result obj around.   */
       send_message0(ProcessLocalServer, (RexxString *)new_cstring("SAVE_RESULT"));

     discard(hold(result));            /* release it and hole a bit longer  */
  }
  activity->popNil();                  /* remove the nil marker             */
                                       /* release our activity usage        */
  TheActivityClass->returnActivity(activity);
  return rc;                           /* return the error code             */
}

REXXOBJECT REXXENTRY RexxDispatch (
  REXXOBJECT argList)                  /* ArgLIst array.                    */
/******************************************************************************/
/* Function:  Dispatch message to rexx object.  All args are in a Rexx Array  */
/******************************************************************************/
{
  RexxObject *receiver;                /* receiver of message               */
  RexxString *message;                 /* message name to send to receiver  */
  RexxArray  *args;                    /* argument array for message        */
  RexxObject *result;                  /* returned result object            */

  MTXRQ(kernel_semaphore);             /* get the kernel semophore          */
                                       /* have more pools been added since  */
                                       /* we left the kernel ?              */
  if (GlobalCurrentPool != ProcessCurrentPool)
                                       /* yes, get access to them.          */
    memoryObject.accessPools(ProcessCurrentPool);


  receiver = ((RexxArray *)argList)->get(1);
  message  = (RexxString *)((RexxArray *)argList)->get(2);
  args     = (RexxArray *)((RexxArray *)argList)->get(3);
  MTXRL(kernel_semaphore);             /* release kernel lock.              */
                                       /* process the message               */
  RexxSendMessage(receiver, message->stringData, OREF_NULL, "oA", &result, (RexxObject *)args);
  return result;                       /* return the message result         */
}
