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
/* REXX Kernel                                                  RexxActivity.hpp  */
/*                                                                            */
/* Primitive Activity Class Definitions                                       */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxActivity
#define Included_RexxActivity

#include "ListClass.hpp"
#include "RexxInternalStack.hpp"
#include "RexxActivationStack.hpp"
#include "ExpressionStack.hpp"
#include "RexxInternalStack.hpp"
#include "RexxLocalVariables.hpp"
                                       /* interface values for the          */
                                       /* activity_queue method             */
#define QUEUE_FIFO 1
#define QUEUE_LIFO 2

#ifndef THREADS
#define dispatchable_activities (TheActivityClass->class_waitacts)
#endif

#define resource_semaphore     rexx_resource_semaphore
#define kernel_semaphore       rexx_kernel_semaphore
#define start_semaphore        rexx_start_semaphore

                                       /* system exit definitions           */
                                       /* (these must match the externally  */
                                       /* defined constants in REXXSAA.H)   */

#define RXFNC    2                     /* Process external functions.       */
#define RXCMD    3                     /* Process host commands.            */
#define RXMSQ    4                     /* Manipulate queue.                 */
#define RXSIO    5                     /* Session I/O.                      */
#define RXHLT    7                     /* Halt processing.                  */
#define RXTRC    8                     /* Test ext trace indicator.         */
#define RXINI    9                     /* Initialization processing.        */
#define RXTER   10                     /* Termination processing.           */
#define RXDBG   11                     /* Test ext trace indicator before   */
// used only internally, can be moved to a differnet value, if the using code is adapted accordingly
#define RXEXF   12                     /* external function call replacer   */
#define LAST_EXIT 12                   /* top bound of the exits            */


extern SMTX rexx_kernel_semaphore;     /* global kernel semaphore           */
extern SMTX rexx_resource_semaphore;   /* global kernel semaphore           */
extern SMTX rexx_start_semaphore;      /* startup semaphore                 */

extern SEV    rexxTimeSliceSemaphore;
extern ULONG  RexxTimeSliceTimer;
extern ULONG  rexxTimeSliceTimerOwner;


void kernelTerminate(int terminateType);
#define NORMAL_TERMINATION  0UL
#define FORCED_TERMINATION  1UL
                                       /* information must be saved and     */
                                       /* restored on nested entries to the */
                                       /* interpreter that use the same     */
                                       /* activity                          */
typedef struct nestedinfo {
   char       *stackptr;               /* pointer to base of C stack        */
   RexxString *currentExit;            /* current executing system exit     */
   RexxString *shvexitvalue;           /* ret'd val from varpool RXHSV_EXIT */
   ULONG       randomSeed;             /* random number seed                */
   BOOL        exitset;                /* halt/trace sys exit not set ==> 1 */
   RexxString *sysexits[LAST_EXIT];    /* Array to hold system exits        */
   jmp_buf     jmpenv;                 /* setjmp buffer                     */
}  nestedActivityInfo;

                                       /* NOTE:  The following object       */
                                       /* definitions are only included in  */
                                       /* a module if the define            */
                                       /* INCL_ACTIVITY_DEFINITIONS is used */
                                       /* since they include data types that*/
                                       /* are not generally required (or    */
                                       /* available in other classes that   */
                                       /* might be using the activity class */
                                       /* methods                           */
 class RexxActivity : public RexxInternalObject {
  public:
   inline RexxActivity(RESTORETYPE restoreType) { ; };
   void *operator new(size_t);
   inline void *operator new(size_t size, void *ptr) {return ptr;};
   RexxActivity(BOOL, long, RexxDirectory *);
   long error(size_t);
   BOOL        raiseCondition(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *, RexxDirectory *);
   void        raiseException(LONG, LOCATIONINFO *, RexxSource *, RexxString *, RexxArray *, RexxObject *);
   void        reportException(LONG, PCHAR);
   void        reportException(LONG, LONG);
   void        reportAnException(LONG);
   void        reportAnException(LONG, RexxObject *);
   void        reportAnException(LONG, RexxObject *, RexxObject *);
   void        reportAnException(LONG, RexxObject *, RexxObject *, RexxObject *);
   void        reportAnException(LONG, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
   void        reraiseException(RexxDirectory *);
   void        raisePropagate(RexxDirectory *);
   RexxObject *display(RexxDirectory *);
   RexxObject *displayDebug(RexxDirectory *);
   RexxString *messageSubstitution(RexxString *, RexxArray *);
   void        live();
   void        liveGeneral();
   void        flatten(RexxEnvelope *);
   void        run();
   void        push(RexxActivationBase *);
   void        pop(BOOL);
   void        pushNil();
   void        popNil();
   void        exitKernel(RexxActivation *, RexxString *, BOOL);
   void        enterKernel();
   RexxObject *previous();
   void        waitReserve(RexxObject *);
   void        guardWait();
   void        guardPost();
   void        guardSet();
   void        checkDeadLock(RexxActivity *);
   void        postRelease();
   void        kill(RexxDirectory *);
   RexxActivationBase *sender(RexxActivationBase *);
   void        joinKernelQueue();
   void        relinquish();
   void        yield(RexxObject *);
   void        releaseKernel();
   void        requestKernel();
   void        stackSpace();
   long        priorityMethod();
   RexxObject *localMethod();
   long threadIdMethod();
   void setShvVal(RexxString *);
   inline BOOL querySet() { return this->nestedInfo.exitset; }
   void queryTrcHlt();
   void sysExitInit(RexxActivation *);
   void sysExitTerm(RexxActivation *);
   BOOL sysExitSioSay(RexxActivation *, RexxString *);
   BOOL sysExitSioTrc(RexxActivation *, RexxString *);
   BOOL sysExitSioTrd(RexxActivation *, RexxString **);
   BOOL sysExitSioDtr(RexxActivation *, RexxString **);
   BOOL sysExitFunc(RexxActivation *, RexxString *, RexxObject *, RexxObject **, RexxObject **, size_t);
   BOOL sysExitCmd(RexxActivation *, RexxString *, RexxString *, RexxString **, RexxObject **);
   BOOL sysExitMsqPll(RexxActivation *, RexxString **);
   BOOL sysExitMsqPsh(RexxActivation *, RexxString *, int);
   BOOL sysExitMsqSiz(RexxActivation *, RexxInteger **);
   BOOL sysExitMsqNam(RexxActivation *, RexxString **);
   BOOL sysExitHltTst(RexxActivation *);
   BOOL sysExitHltClr(RexxActivation *);
   BOOL sysExitTrcTst(RexxActivation *, BOOL);
   BOOL sysExitDbgTst(RexxActivation *, BOOL, BOOL);
   void traceOutput(RexxActivation *, RexxString *);
   void sayOutput(RexxActivation *, RexxString *);
   void queue(RexxActivation *, RexxString *, INT);
   RexxString *traceInput(RexxActivation *);
   RexxString *pullInput(RexxActivation *);
   RexxObject *lineOut(RexxString *);
   RexxString *lineIn(RexxActivation *);
   void addUninitObject(RexxObject *);
   void removeUninitObject(RexxObject *);
   BOOL isPendingUninit(RexxObject *);
   void uninitObject(RexxObject *);
   void checkUninits();
   void startMessages();
   void terminateMethod();
   LONG messageSend(RexxObject *, RexxString *, LONG, RexxObject **, RexxObject **);
   void generateRandomNumberSeed();


   inline RexxActivationBase *current(){ return this->topActivation;}
   inline RexxActivation *currentAct() {return this->currentActivation;}
   inline ACTIVATION_SETTINGS *getSettings () {return this->settings;}
   inline void                 setProcessobj(RexxObject *p) {this->processObj = p;}
   inline RexxObject *runningRequires(RexxString *program) {return this->requiresTable->stringGet(program);}
   inline void        addRunningRequires(RexxString *program) { this->requiresTable->stringAdd((RexxObject *)program, program);}
   inline void        removeRunningRequires(RexxObject *program) {this->requiresTable->remove(program);}
   inline void        resetRunningRequires() {this->requiresTable->reset();}
   inline void        setNextWaitingActivity(RexxActivity *next) { this->nextWaitingActivity = next; }
   inline void        waitKernel() { EVWAIT(this->runsem); }
   inline void        clearWait()  { EVSET(this->runsem); }
   inline void        setCurrentExit(RexxString *newExit) { this->nestedInfo.currentExit = newExit; }
   inline RexxString *getCurrentExit() { return this->nestedInfo.currentExit; }
   inline void setRandomSeed(long seed) { this->nestedInfo.randomSeed = seed; };
   inline RexxObject *getProcessObj(){ return this->processObj;};
   inline void setSysExit(long exitNum, RexxString *exitName) { this->nestedInfo.sysexits[exitNum -1] = exitName;}
   inline RexxString *querySysExits(long exitNum) {return this->nestedInfo.sysexits[exitNum -1];}
   inline RexxString **getSysExits() {return this->nestedInfo.sysexits;}
   inline void clearExits() { memset((PVOID)&this->nestedInfo.sysexits, 0, sizeof(this->nestedInfo.sysexits)); }
   inline void saveNestedInfo(nestedActivityInfo *saveInfo) { memcpy((PVOID)saveInfo, (PVOID)&this->nestedInfo, sizeof(nestedActivityInfo)); }
   inline void restoreNestedInfo(nestedActivityInfo *saveInfo) { memcpy((PVOID)&this->nestedInfo, (PVOID)saveInfo, sizeof(nestedActivityInfo)); }
   inline void allocateStackFrame(RexxExpressionStack *stack, size_t entries)
   {
       stack->setFrame(frameStack.allocateFrame(entries), entries);
   }

   inline RexxObject **allocateFrame(size_t entries)
   {
       return frameStack.allocateFrame(entries);
   }

   inline void releaseStackFrame(RexxObject **frame)
   {
       frameStack.releaseFrame(frame);
   }

   inline void allocateLocalVariableFrame(RexxLocalVariables *locals)
   {
       locals->setFrame(frameStack.allocateFrame(locals->size));
   }

   RexxInternalStack  *activations;    /* stack of activations              */
   RexxActivationStack   frameStack;   /* our stack used for activation frames */
   RexxObject         *save;           /* saved result across activity_yield*/
   RexxDirectory      *local;          /* the local environment directory   */
   RexxDirectory      *conditionobj;   /* condition object for killed activi*/
   RexxObject         *processObj;     /* Process identifier Object.        */
   RexxTable          *requiresTable;  /* Current ::REQUIRES being installed*/
                                       /* current REXX activation           */
   RexxActivation     *currentActivation;
   RexxActivationBase *topActivation;  /* current top activation            */
                                       /* next element in the wait queue    */
   RexxActivity       *nextWaitingActivity;
   RexxString         *currentExit;    /* current executing system exit     */
   RexxObject         *waitingObject;  /* object activity is waiting on     */
   SEV      runsem;                    /* activity run control semaphore    */
   size_t   size;                      /* size of activation stack          */
   size_t   depth;                     /* depth of activation stack         */
   LONG     threadid;                  /* thread id                         */
#ifdef THREADHANDLE
   HANDLE   hThread;                   /* handle to thread                  */
#endif
   ACTIVATION_SETTINGS *settings;      /* current activation setting values */
                                       /* current activation defaults       */
   ACTIVATION_SETTINGS default_settings;
   int      priority;                  /* activity priority value           */
   BOOL     stackcheck;                /* stack space is to be checked      */
   BOOL     exit;                      /* activity loop is to exit          */
   BOOL     exitObjects;               // return ptrs to objects for exit handlers
   BOOL     requestingString;          /* in error handling currently       */
   SEV      guardsem;                  /* guard expression semaphore        */
   SYSWINDOWINFO  *windowInfo;         /* Information needed for windowing  */
                                       /* system  on an Activity basis      */
   LONG     nestedCount;               /* extent of the nesting             */
   nestedActivityInfo nestedInfo;      /* info saved and restored on calls  */
   jmp_buf  stringError;               /* string request error buffer       */

   BOOL     DBCS_codepage;             /* DBCS characters possible          */
   ULONG    codepage;                  /* current codepage id               */
   UCHAR    DBCS_table[256];           /* DBCS first byte table             */
 };

 class RexxActivityClass : public RexxClass {
  public:
   RexxActivityClass(RESTORETYPE restoreType) { ; };
   RexxActivityClass() { this->init(); }
   void *operator new(size_t size, void *ptr) {return ptr;};
   void *operator new(size_t size, long size1, RexxBehaviour *classBehave, RexxBehaviour *instance) { return new (size, classBehave, instance) RexxClass; }
   RexxActivity *newActivity( long, RexxObject *);
   RexxActivation *newActivation(RexxObject *, RexxMethod *, RexxActivity *, RexxString *, RexxActivation *, INT);
   void            cacheActivation(RexxActivation *);

   void init();
   void live();
   void liveGeneral();
   void addUninitObject (RexxObject *, RexxObject *);
   void removeUninitObject(RexxObject *, RexxObject *);
   void runUninits();
   BOOL addMessageObject(RexxObject *, RexxObject *);
   void terminateFreeActs();
   void addWaitingActivity(RexxActivity *, BOOL);
   RexxActivity *getActivity();
   void returnActivity(RexxActivity *);

   inline RexxObjectTable  *getUninitTables() {return this->uninitTables;}
   inline RexxObjectTable  *getUninitTable(RexxObject *processObj) {return (RexxObjectTable *)this->uninitTables->get(processObj);}
   inline RexxObjectTable  *removeUninitTable(RexxObject *processObj) {return (RexxObjectTable *)this->uninitTables->remove(processObj);}
   inline RexxObject *getMessageList(RexxObject *processObj) {return this->messageTable->get(processObj);}
   inline RexxMessage *removeNextMessageObject(RexxObject *list) {return (RexxMessage *)((RexxList *)list)->removeLast();}
   inline void removeMessageList(RexxObject *processObj) {this->messageTable->remove(processObj);}
   inline void killMessageList(RexxObject *processObj) {this->messageTable->put(processObj, (RexxObject *)TheFalseObject);}
   inline RexxObjectTable  *getSubClassTable() {return this->subClasses;}
   inline void newSubClass(RexxClass *newClass, RexxClass *superClass) {this->subClasses->add(newClass, superClass);}
   inline RexxActivity *waitingActivity() { return this->firstWaitingActivity; }
   inline void addPendingUninit() { pendingUninits++; }
   inline void checkUninitQueue() { if (pendingUninits > 0) runUninits(); }

                                       /* activities in use                 */
   RexxObjectTable  *classUsedActivities;
                                       /* free activities                   */
   RexxObjectTable  *classFreeActivities;
                                       /* table of all localact             */
   RexxObjectTable  *classAllActivities;
   RexxObjectTable  *uninitTables;     /* UNINIT tables, one per process    */
   size_t            pendingUninits;   /* objects waiting to have uninits run */
   BOOL              processingUninits; /* TRUE when we are processing the uninit table */
   RexxObjectTable  *subClasses;       /* SubClasses...one per system       */
   RexxObjectTable  *messageTable;     /* message Lists, one per process    */
   RexxStack        *activations;      /* cached activations                */
                                       /* head of the waiting activity queue*/
   RexxActivity     *firstWaitingActivity;
                                       /* tail of the waiting activity queue*/
   RexxActivity     *lastWaitingActivity;
                                       /* size of the activation cache      */
   LONG              activationCacheSize;
   #ifndef THREADS
   int               class_waitacts;   /* number of waiting activities      */
   #endif
 };


inline void reportException(wholenumber_t error)
{
    CurrentActivity->reportAnException(error);
}

inline void reportException(wholenumber_t error, RexxObject *a1)
{
    CurrentActivity->reportAnException(error, a1);
}

inline void reportException(wholenumber_t error, RexxObject *a1, RexxObject *a2)
{
    CurrentActivity->reportAnException(error, a1, a2);
}

inline void reportException(wholenumber_t error, RexxObject *a1, RexxObject *a2, RexxObject *a3)
{
    CurrentActivity->reportAnException(error, a1, a2, a3);
}

inline void reportException(wholenumber_t error, RexxObject *a1, RexxObject *a2, RexxObject *a3, RexxObject *a4)
{
    CurrentActivity->reportAnException(error, a1, a2, a3, a4);
}

inline void reportException(wholenumber_t error, wholenumber_t a1)
{
    CurrentActivity->reportAnException(error, new_integer(a1));
}

void activity_create (void);
void activity_restore (void);
BOOL activity_halt (LONG, RexxString *);
BOOL activity_set_trace (LONG, BOOL);
void activity_set_yield(void);
BOOL activity_sysyield(LONG threadid, RexxObject * description);
void activity_lock_kernel(void);
void activity_unlock_kernel(void);
#endif
