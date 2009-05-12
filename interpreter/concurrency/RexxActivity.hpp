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
#include "ActivationApiContexts.hpp"
#include "SysActivity.hpp"



class ProtectedObject;                 // needed for look aheads
class RexxSource;
class RexxMethod;
class InterpreterInstance;
class ActivityDispatcher;
class CallbackDispatcher;
class CommandHandler;

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
    TRACE_PREFIX_ASSIGNMENT,
};


// marker used for tagged traces to separate tag from the value
#define VALUE_MARKER " => "
// marker used for tagged traces to separate tag from the value
#define ASSIGNMENT_MARKER " <= "


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
   RexxActivity();
   RexxActivity(bool);

   void reset();
   void runThread();
   wholenumber_t error();
   wholenumber_t error(RexxActivationBase *);
   wholenumber_t errorNumber(RexxDirectory *conditionObject);
   bool        raiseCondition(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *);
   bool        raiseCondition(RexxDirectory *);
   static RexxDirectory *createConditionObject(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *);
   void        raiseException(wholenumber_t, SourceLocation *, RexxSource *, RexxString *, RexxArray *, RexxObject *);
   RexxDirectory *createExceptionObject(wholenumber_t, RexxActivation *, SourceLocation *, RexxSource *, RexxString *, RexxArray *, RexxObject *);
   void        reportAnException(wholenumber_t, const char *);
   void        reportAnException(wholenumber_t, const char *, const char *);
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
   void        run(RexxMessage *target);
   void        checkActivationStack();
   void        updateFrameMarkers();
   void        pushStackFrame(RexxActivationBase *new_activation);
   void        createNewActivationStack();
   void        popStackFrame(bool  reply);
   void        popStackFrame(RexxActivationBase *);
   void        unwindStackFrame();
   void        unwindToDepth(size_t depth);
   void        unwindToFrame(RexxActivation *frame);
   void        cleanupStackFrame(RexxActivationBase *poppedStackFrame);
   RexxActivity *spawnReply();

   void        exitKernel();
   void        enterKernel();
   RexxObject *previous();
   void        waitReserve(RexxObject *);
   void        guardWait();
   void        guardPost();
   void        guardSet();
   void        checkDeadLock(RexxActivity *);
   void        postRelease();
   void        kill(RexxDirectory *);
   void        joinKernelQueue();
   void        relinquish();
   bool        halt(RexxString *);
   bool        setTrace(bool);
   void        yieldControl();
   void        yield();
   void        releaseAccess();
   void        requestAccess();
   void        checkStackSpace();
   void        cleanupActivityResources();
   void        terminatePoolActivity();
   thread_id_t threadIdMethod();
   bool isThread(thread_id_t id) { return currentThread.equals(id); }
   inline bool isClauseExitUsed() { return clauseExitUsed; }
   void queryTrcHlt();
   bool callExit(RexxActivation * activation, const char *exitName, int function, int subfunction, void *exitbuffer);
   void callInitializationExit(RexxActivation *);
   void callTerminationExit(RexxActivation *);
   bool callSayExit(RexxActivation *, RexxString *);
   bool callTraceExit(RexxActivation *, RexxString *);
   bool callTerminalInputExit(RexxActivation *, RexxString *&);
   bool callDebugInputExit(RexxActivation *, RexxString *&);
   bool callObjectFunctionExit(RexxActivation *, RexxString *, RexxObject *, ProtectedObject &, RexxObject **, size_t);
   bool callFunctionExit(RexxActivation *, RexxString *, RexxObject *, ProtectedObject &, RexxObject **, size_t);
   bool callScriptingExit(RexxActivation *, RexxString *, RexxObject *, ProtectedObject &, RexxObject **, size_t);
   bool callCommandExit(RexxActivation *, RexxString *, RexxString *, ProtectedObject &result, ProtectedObject &condition);
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
   void generateRandomNumberSeed();
   void setupAttachedActivity(InterpreterInstance *interpreter);
   void addToInstance(InterpreterInstance *interpreter);
   void detachInstance();
   void detachThread();
   inline InterpreterInstance *getInstance() { return instance; }

   inline void nestAttach() { attachCount++; }
   inline bool isNestedAttach() { return attachCount != 0; }
   inline void returnAttach() { attachCount--; }
   inline void activate() { nestedCount++; }
   inline void deactivate() { nestedCount--; }
   inline bool isActive() { return nestedCount > 0; }
   inline bool isInactive() { return nestedCount == 0; }
   inline size_t getActivationLevel() { return nestedCount; }
   inline void restoreActivationLevel(size_t l) { nestedCount = l; }
   inline bool isSuspended() { return suspended; }
   inline void setSuspended(bool s) { suspended = s; }
   inline bool isInterpreterRoot() { return interpreterRoot; }
   inline void setInterpreterRoot() { interpreterRoot = true; }
   inline void setNestedActivity(RexxActivity *a) { nestedActivity = a; }
   inline RexxActivity *getNestedActivity() { return nestedActivity; }
   inline bool isAttached() { return attached; }
          void validateThread();

   SecurityManager *getEffectiveSecurityManager();
   SecurityManager *getInstanceSecurityManager();
   void inheritSettings(RexxActivity *parent);
   void setupExits();
   void enterCurrentThread();
   void exitCurrentThread();
   void run(ActivityDispatcher &target);
   void run(CallbackDispatcher &target);

   inline RexxActivation *getCurrentRexxFrame() {return currentRexxFrame;}
   inline RexxActivationBase *getTopStackFrame() { return topStackFrame; }
   inline size_t getActivationDepth() { return stackFrameDepth; }
   inline NumericSettings *getNumericSettings () {return this->numericSettings;}
   inline RexxObject *runningRequires(RexxString *program) {return this->requiresTable->stringGet(program);}
   inline void        addRunningRequires(RexxString *program) { this->requiresTable->stringAdd((RexxObject *)program, program);}
   inline void        removeRunningRequires(RexxObject *program) {this->requiresTable->remove(program);}
   inline void        resetRunningRequires() {this->requiresTable->reset();}
   inline bool        checkRequires(RexxString *n) { return runningRequires(n) != OREF_NULL; }
   inline void        setNextWaitingActivity(RexxActivity *next) { this->nextWaitingActivity = next; }
   inline RexxActivity *getNextWaitingActivity() { return nextWaitingActivity; }
   inline void        waitKernel() { runsem.wait(); }
   inline void        clearWait()  { runsem.reset(); }
   inline uint64_t    getRandomSeed() { return randomSeed; }
   inline void setRandomSeed(uint64_t seed) { randomSeed = seed; };
   inline RexxString *getLastMessageName() { return lastMessageName; }
   inline RexxMethod *getLastMethod() { return lastMethod; }
   inline void setLastMethod(RexxString *n, RexxMethod *m) { lastMessageName = n; lastMethod = m; }

   inline RexxThreadContext *getThreadContext() { return &threadContext.threadContext; }
   inline RexxNativeActivation *getApiContext() { return (RexxNativeActivation *)topStackFrame; }

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
   RexxString *resolveProgramName(RexxString *, RexxString *, RexxString *);
   void createMethodContext(MethodContext &context, RexxNativeActivation *owner);
   void createCallContext(CallContext &context, RexxNativeActivation *owner);
   void createExitContext(ExitContext &context, RexxNativeActivation *owner);
   RexxObject *getLocalEnvironment(RexxString *name);
   RexxDirectory *getLocal();
   CommandHandler *resolveCommandHandler(RexxString *);

   static void initializeThreadContext();

 protected:

   ExitHandler &getExitHandler(int exitNum) {  return sysexits[exitNum - 1]; }
   inline bool isExitEnabled(int exitNum) { return getExitHandler(exitNum).isEnabled(); }
   inline void disableExit(int exitNum) { getExitHandler(exitNum).disable(); }


   InterpreterInstance *instance;      // the interpreter we're running under
   ActivityContext      threadContext; // the handed out activity context
   RexxActivity *oldActivity;          // pushed nested activity
   RexxActivationStack   frameStack;   /* our stack used for activation frames */
   RexxDirectory      *conditionobj;   /* condition object for killed activi*/
   RexxTable          *requiresTable;  /* Current ::REQUIRES being installed*/
   RexxMessage        *dispatchMessage;  // a message object to run on this thread


   // the activation frame stack.  This stack is one RexxActivation or
   // RexxNativeActivation for every level of the call stack.  The activationStackSize
   // is the current size of the stack (which is expanded, if necessary).  The
   // activationStackDepth is the current count of frames in the stack.
   RexxInternalStack  *activations;
   size_t   activationStackSize;
   size_t   stackFrameDepth;

   // the following two fields represent the current top of the activation stack
   // and the top Rexx frame in the stack.  Generally, if executing Rexx code,
   // then currentRexxFrame == topStackFrame.  If we're at the base of the stack
   // topStackFrame will be the root stack element (a RexxNativeActivation instance)
   // and the currentRexxFrame will be OREF_NULL.  If we've made a callout from a
   // Rexx context, then the topStackFrame will be the RexxNativeActivation that
   // made the callout and the currentRexxFrame will be the predecessor frame.
   RexxActivation     *currentRexxFrame;
   RexxActivationBase *topStackFrame;
                                       /* next element in the wait queue    */
   RexxActivity       *nextWaitingActivity;
   RexxString         *currentExit;    /* current executing system exit     */
   RexxObject         *waitingObject;  /* object activity is waiting on     */
   SysSemaphore        runsem;         /* activity run control semaphore    */
   SysSemaphore        guardsem;       /* guard expression semaphore        */
   SysActivity currentThread;            /* descriptor for this thread        */
   NumericSettings *numericSettings;   /* current activation setting values */

   bool     stackcheck;                /* stack space is to be checked      */
   bool     exit;                      /* activity loop is to exit          */
   bool     requestingString;          /* in error handling currently       */
   bool     suspended;                 // the suspension flag
   bool     interpreterRoot;           // This is the root activity for an interpreter instance
   bool     attached;                  // this is attached to an instance (vs. created directly)
   size_t   nestedCount;               /* extent of the nesting             */
   size_t   attachCount;               // extent of nested attaches
   char       *stackBase;              /* pointer to base of C stack        */
   bool        clauseExitUsed;         /* halt/trace sys exit not set ==> 1 */
   uint64_t    randomSeed;             /* random number seed                */
   ExitHandler sysexits[LAST_EXIT];    /* Array to hold system exits        */
   ProtectedObject *protectedObjects;  // list of stack-based object protectors
   RexxString *lastMessageName;        // class called message
   RexxMethod *lastMethod;             // last called method
   RexxActivity *nestedActivity;       // used to push down activities in threads with more than one instance

   // structures containing the various interface vectors
   static RexxThreadInterface threadContextFunctions;
   static MethodContextInterface methodContextFunctions;
   static CallContextInterface callContextFunctions;
   static ExitContextInterface exitContextFunctions;
 };


/**
 * Convert an API context to into the top native activation
 * context associated with the thread.
 *
 * @param c      The source API context.
 *
 * @return A Native activation context that is the anchor point for the
 *         API activity.
 */
inline RexxNativeActivation *contextToActivation(RexxThreadContext *c)
{
    return contextToActivity(c)->getApiContext();
}


/**
 * Convert an API context to into the top native activation
 * context associated with the thread.
 *
 * @param c      The source API context.
 *
 * @return A Native activation context that is the anchor point for the
 *         API activity.
 */
inline RexxNativeActivation *contextToActivation(RexxCallContext *c)
{
    return ((CallContext *)c)->context;
}


/**
 * Convert an API context to into the top native activation
 * context associated with the thread.
 *
 * @param c      The source API context.
 *
 * @return A Native activation context that is the anchor point for the
 *         API activity.
 */
inline RexxNativeActivation *contextToActivation(RexxExitContext *c)
{
    return ((ExitContext *)c)->context;
}


/**
 * Convert an API context to into the top native activation
 * context associated with the thread.
 *
 * @param c      The source API context.
 *
 * @return A Native activation context that is the anchor point for the
 *         API activity.
 */
inline RexxNativeActivation *contextToActivation(RexxMethodContext *c)
{
    return ((MethodContext *)c)->context;
}

#endif
