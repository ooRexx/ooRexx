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
/* REXX Kernel                                              RexxActivity.hpp  */
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
#include "SourceLocation.hpp"
#include "ExitHandler.hpp"


class ProtectedObject;                 // needed for look aheads
class RexxSource;
class RexxMethod;

                                       /* interface values for the          */
                                       /* activity_queue method             */
#define QUEUE_FIFO 1
#define QUEUE_LIFO 2

/******************************************************************************/
/* Constants used for trace prefixes                                          */
/******************************************************************************/

enum TracePrefixes {
    TRACE_PREFIX_CLAUSE   ,
    TRACE_PREFIX_ERROR    ,
    TRACE_PREFIX_RESULT   ,
    TRACE_PREFIX_DUMMY    ,
    TRACE_PREFIX_VARIABLE ,
    TRACE_PREFIX_DOTVARIABLE ,
    TRACE_PREFIX_LITERAL  ,
    TRACE_PREFIX_FUNCTION ,
    TRACE_PREFIX_PREFIX   ,
    TRACE_PREFIX_OPERATOR ,
    TRACE_PREFIX_COMPOUND ,
    TRACE_PREFIX_MESSAGE  ,
    TRACE_PREFIX_ARGUMENT ,
};

#define MAX_TRACEBACK_LIST 80      /* 40 messages are displayed */
#define MAX_TRACEBACK_INDENT 20    /* 10 messages are indented */


typedef enum
{
    RecursiveStringError,              // a recursion problem in error handling
    FatalError,                        // bad problem
    UnhandledCondition                 // we had an unhandled condition.
} ActivityException;

// used only internally, can be moved to a differnet value, if the using code is adapted accordingly
#define LAST_EXIT (RXNOOFEXITS - 1)    /* top bound of the exits            */

                                       /* information must be saved and     */
                                       /* restored on nested entries to the */
                                       /* interpreter that use the same     */
                                       /* activity                          */
class NestedActivityState
{
public:
   char       *stackptr;               /* pointer to base of C stack        */
   bool        clauseExitUsed;         /* halt/trace sys exit not set ==> 1 */
   size_t      randomSeed;             /* random number seed                */
   ExitHandler sysexits[LAST_EXIT];    /* Array to hold system exits        */
};

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
  friend class ProtectedObject;
  public:
   void *operator new(size_t);
   inline void *operator new(size_t size, void *ptr) {return ptr;};
   inline void  operator delete(void *) { ; }
   inline void  operator delete(void *, void *) { ; }

   inline RexxActivity(RESTORETYPE restoreType) { ; };
   RexxActivity(bool, int);


   void runThread();
   wholenumber_t error(size_t);
   bool        raiseCondition(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *, RexxDirectory *);
   void        raiseException(wholenumber_t, SourceLocation *, RexxSource *, RexxString *, RexxArray *, RexxObject *);
   void        reportAnException(wholenumber_t, const char *);
   void        reportAnException(wholenumber_t, RexxObject *, const char *);
   void        reportAnException(wholenumber_t, RexxObject *, wholenumber_t);
   void        reportAnException(wholenumber_t, const char *, RexxObject *);
   void        reportAnException(wholenumber_t, const char *, wholenumber_t);
   void        reportAnException(wholenumber_t, const char *, wholenumber_t, RexxObject *);
   void        reportAnException(wholenumber_t, const char *, RexxObject *, wholenumber_t);
   void        reportAnException(wholenumber_t, wholenumber_t);
   void        reportAnException(wholenumber_t, wholenumber_t, wholenumber_t);
   void        reportAnException(wholenumber_t, wholenumber_t, RexxObject *);
   void        reportAnException(wholenumber_t);
   void        reportAnException(wholenumber_t, RexxObject *);
   void        reportAnException(wholenumber_t, RexxObject *, RexxObject *);
   void        reportAnException(wholenumber_t, RexxObject *, RexxObject *, RexxObject *);
   void        reportAnException(wholenumber_t, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
   void        reportAnException(wholenumber_t, const char *, RexxObject *, const char *, RexxObject *);
   void        reraiseException(RexxDirectory *);
   void        raisePropagate(RexxDirectory *);
   RexxObject *display(RexxDirectory *);
   RexxObject *displayDebug(RexxDirectory *);
   RexxString *messageSubstitution(RexxString *, RexxArray *);
   void        live(size_t);
   void        liveGeneral(int reason);
   void        flatten(RexxEnvelope *);
   void        run();
   void        push(RexxActivationBase *);
   void        pop(bool);
   void        pushNil();
   void        popNil();
   void        exitKernel(RexxActivation *);
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
   bool        halt(RexxString *);
   bool        setTrace(bool);
   void        yield(RexxObject *);
   void        yield();
   void        releaseAccess();
   void        requestAccess();
   void        checkStackSpace();
   void        terminateActivity();
   RexxObject *localMethod();
   thread_id_t threadIdMethod();
   bool isThread(thread_id_t id) { return threadid == id; }
   void setShvVal(RexxString *);
   inline bool isClauseExitUsed() { return this->nestedInfo.clauseExitUsed; }
   void queryTrcHlt();
   bool callExit(RexxActivation * activation, const char *exitName, int function, int subfunction, void *exitbuffer);
   void callInitializationExit(RexxActivation *);
   void callTerminationExit(RexxActivation *);
   bool callSayExit(RexxActivation *, RexxString *);
   bool callTraceExit(RexxActivation *, RexxString *);
   bool callTerminalInputExit(RexxActivation *, RexxString *&);
   bool callDebugInputExit(RexxActivation *, RexxString *&);
   bool callFunctionExit(RexxActivation *, RexxString *, RexxObject *, ProtectedObject &, RexxObject **, size_t);
   bool callScriptingExit(RexxActivation *, RexxString *, RexxObject *, ProtectedObject &, RexxObject **, size_t);
   bool callCommandExit(RexxActivation *, RexxString *, RexxString *, RexxString **, RexxObject **);
   bool callPullExit(RexxActivation *, RexxString *&);
   bool callPushExit(RexxActivation *, RexxString *, int);
   bool callQueueSizeExit(RexxActivation *, RexxInteger *&);
   bool callQueueNameExit(RexxActivation *, RexxString *&);
   bool callHaltTestExit(RexxActivation *);
   bool callHaltClearExit(RexxActivation *);
   bool callTraceTestExit(RexxActivation *, bool);
   bool callNovalueExit(RexxActivation *, RexxString *, RexxObject *&);
   bool callValueExit(RexxActivation *, RexxString *, RexxString *, RexxObject *, RexxObject *&);
   void traceOutput(RexxActivation *, RexxString *);
   void sayOutput(RexxActivation *, RexxString *);
   void queue(RexxActivation *, RexxString *, int);
   RexxString *traceInput(RexxActivation *);
   RexxString *pullInput(RexxActivation *);
   RexxObject *lineOut(RexxString *);
   RexxString *lineIn(RexxActivation *);
   void terminateMethod();
   wholenumber_t messageSend(RexxObject *, RexxString *, size_t, RexxObject **, ProtectedObject &);
   void generateRandomNumberSeed();

   void activate() { nestedCount++; }
   void deactivate() { nestedCount--; }
   bool isActive() { return nestedCount > 0; }
   bool isInactive() { return nestedCount == 0; }
   size_t getActivationLevel() { return nestedCount; }
   void restoreActivationLevel(size_t l) { nestedCount = l; }

   bool hasSecurityManager();
   bool callSecurityManager(RexxString *name, RexxDirectory *args);
   RexxObject *nativeRelease(RexxObject *result);
   void inheritSettings(RexxActivity *parent);

   inline RexxActivationBase *current(){ return this->topActivation;}
   inline RexxActivation *getCurrentActivation() {return this->currentActivation;}
   inline size_t getActivationDepth() { return depth; }
   inline NumericSettings *getNumericSettings () {return this->numericSettings;}
   inline RexxObject *runningRequires(RexxString *program) {return this->requiresTable->stringGet(program);}
   inline void        addRunningRequires(RexxString *program) { this->requiresTable->stringAdd((RexxObject *)program, program);}
   inline void        removeRunningRequires(RexxObject *program) {this->requiresTable->remove(program);}
   inline void        resetRunningRequires() {this->requiresTable->reset();}
   inline void        setNextWaitingActivity(RexxActivity *next) { this->nextWaitingActivity = next; }
   inline RexxActivity *getNextWaitingActivity() { return nextWaitingActivity; }
   inline void        waitKernel() { EVWAIT(this->runsem); }
   inline void        clearWait()  { EVSET(this->runsem); }
   inline size_t      getRandomSeed() { return nestedInfo.randomSeed; }
   inline void setRandomSeed(size_t seed) { this->nestedInfo.randomSeed = seed; };
   inline void saveNestedInfo(NestedActivityState &saveInfo) { saveInfo = nestedInfo; }
   inline void restoreNestedInfo(NestedActivityState &saveInfo) { nestedInfo = saveInfo; }
   inline RexxString *getLastMessageName() { return lastMessageName; }
   inline RexxMethod *getLastMethod() { return lastMethod; }
   inline void setLastMethod(RexxString *n, RexxMethod *m) { lastMessageName = n; lastMethod = m; }
   inline int  getPriority() { return priority; }

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

   inline RexxDirectory *getCurrentCondition() { return conditionobj; }
   void setExitHandler(int exitNum, REXXPFN e) { getExitHandler(exitNum).setEntryPoint(e); }
   void setExitHandler(int exitNum, const char *e) { getExitHandler(exitNum).resolve(e); }
   void setExitHandler(RXSYSEXIT &e) { getExitHandler(e.sysexit_code).resolve(e.sysexit_name); }

   // TODO:  This needs to be replaced by a system object.
#ifdef THREADHANDLE
   HANDLE   hThread;                   /* handle to thread                  */
#endif

 protected:

   ExitHandler &getExitHandler(int exitNum) {  return nestedInfo.sysexits[exitNum - 1]; }
   bool isExitEnabled(int exitNum) { return getExitHandler(exitNum).isEnabled(); }


   RexxInternalStack  *activations;    /* stack of activations              */
   RexxActivationStack   frameStack;   /* our stack used for activation frames */
   RexxObject         *saveValue;      /* saved result across activity_yield*/
   RexxDirectory      *conditionobj;   /* condition object for killed activi*/
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
   thread_id_t threadid;               /* thread id                         */
   NumericSettings *numericSettings;   /* current activation setting values */

   int      priority;                  /* activity priority value           */
   bool     stackcheck;                /* stack space is to be checked      */
   bool     exit;                      /* activity loop is to exit          */
   bool     requestingString;          /* in error handling currently       */
   SEV      guardsem;                  /* guard expression semaphore        */
   size_t   nestedCount;               /* extent of the nesting             */
   NestedActivityState nestedInfo;     /* info saved and restored on calls  */
   ProtectedObject *protectedObjects;  // list of stack-based object protectors
   RexxString *lastMessageName;        // class called message
   RexxMethod *lastMethod;             // last called method
 };

#endif
