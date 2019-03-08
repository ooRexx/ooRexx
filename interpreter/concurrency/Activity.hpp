/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* REXX Kernel                                                  Activity.hpp  */
/*                                                                            */
/* Primitive Activity Class Definitions                                       */
/*                                                                            */
/******************************************************************************/
#ifndef Included_Activity
#define Included_Activity

#include "ListClass.hpp"
#include "InternalStack.hpp"
#include "ActivationStack.hpp"
#include "ExpressionStack.hpp"
#include "InternalStack.hpp"
#include "RexxLocalVariables.hpp"
#include "SourceLocation.hpp"
#include "ExitHandler.hpp"
#include "ActivationApiContexts.hpp"
#include "SysActivity.hpp"
#include "StringTableClass.hpp"
#include "InterpreterInstance.hpp"
#include "RexxErrorCodes.h"



class ProtectedObject;
class ProtectedBase;
class PackageClass;
class MethodClass;
class InterpreterInstance;
class ActivityDispatcher;
class CallbackDispatcher;
class TrappingDispatcher;
class CommandHandler;
class ActivationFrame;
class ActivationBase;
class NativeActivation;
class RexxActivation;
class GlobalProtectedObject;
class MutexSemaphoreClass;

typedef enum
{
    RecursiveStringError,              // a recursion problem in error handling
    FatalError,                        // bad problem
    UnhandledCondition                 // we had an unhandled condition.
} ActivityException;

// used only internally, can be moved to a differnet value, if the using code is adapted accordingly
#define LAST_EXIT (RXNOOFEXITS - 1)    /* top bound of the exits            */

class Activity : public RexxInternalObject
{
  friend class ProtectedBase;
  friend class ActivationFrame;
 public:

    typedef enum
    {
        QUEUE_FIFO,
        QUEUE_LIFO,
    } QueueOrder;



    void *operator new(size_t);
    inline void  operator delete(void *) { ; }

    inline Activity(RESTORETYPE restoreType) { ; };
    Activity(GlobalProtectedObject &, bool);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void reset();
    void runThread();
    wholenumber_t error();
    wholenumber_t error(ActivationBase *, DirectoryClass *errorInfo);
    wholenumber_t errorNumber(DirectoryClass *conditionObject);
    wholenumber_t displayCondition(DirectoryClass *conditionObject);
    bool        raiseCondition(RexxString *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
    bool        raiseCondition(DirectoryClass *);
    bool        checkCondition(RexxString *condition);
    DirectoryClass *createConditionObject(RexxString *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
    void        raiseException(RexxErrorCodes, RexxString *, ArrayClass *, RexxObject *);
    DirectoryClass *createExceptionObject(RexxErrorCodes, RexxString *, ArrayClass *, RexxObject *);
    void        generateProgramInformation(DirectoryClass *exObj);
    void        reportAnException(RexxErrorCodes, const char *);
    void        reportAnException(RexxErrorCodes, const char *, const char *);
    void        reportAnException(RexxErrorCodes, RexxObject *, const char *);
    void        reportAnException(RexxErrorCodes, RexxObject *, wholenumber_t);
    void        reportAnException(RexxErrorCodes, const char *, RexxObject *);
    void        reportAnException(RexxErrorCodes, const char *, wholenumber_t);
    void        reportAnException(RexxErrorCodes, const char *, wholenumber_t, RexxObject *);
    void        reportAnException(RexxErrorCodes, const char *, RexxObject *, wholenumber_t);
    void        reportAnException(RexxErrorCodes, wholenumber_t);
    void        reportAnException(RexxErrorCodes, wholenumber_t, wholenumber_t);
    void        reportAnException(RexxErrorCodes, wholenumber_t, RexxObject *);
    void        reportAnException(RexxErrorCodes);
    void        reportAnException(RexxErrorCodes, RexxObject *);
    void        reportAnException(RexxErrorCodes, RexxObject *, RexxObject *);
    void        reportAnException(RexxErrorCodes, RexxObject *, RexxObject *, RexxObject *);
    void        reportAnException(RexxErrorCodes, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
    void        reportAnException(RexxErrorCodes, const char *, RexxObject *, const char *, RexxObject *);
    void        reraiseException(DirectoryClass *);
    void        raisePropagate(DirectoryClass *);
    void        display(DirectoryClass *);
    void        displayDebug(DirectoryClass *);
    RexxString *buildMessage(wholenumber_t, ArrayClass *);
    RexxString *messageSubstitution(RexxString *, ArrayClass *);
    void        run();
    void        run(MessageClass *target);
    void        checkActivationStack();
    void        updateFrameMarkers();
    void        pushStackFrame(ActivationBase *new_activation);
    void        createNewActivationStack();
    void        popStackFrame(bool  reply);
    void        popStackFrame(ActivationBase *);
    void        unwindStackFrame();
    void        unwindToDepth(size_t depth);
    void        unwindToFrame(RexxActivation *frame);
    void        cleanupStackFrame(ActivationBase *poppedStackFrame);
    ArrayClass  *generateStackFrames(bool skipFirst);
    Activity *spawnReply();

    void        exitKernel();
    void        enterKernel();
    RexxObject *previous();
    void        waitReserve(RexxInternalObject *);
    void        guardWait();
    void        guardPost();
    void        guardSet();
    void        checkDeadLock(Activity *);
    void        postDispatch();
    void        kill(DirectoryClass *);
    void        joinKernelQueue();
    void        relinquish();
    bool        halt(RexxString *);
    bool        setTrace(bool);
    inline void yieldControl() { releaseAccess(); requestAccess(); }
    void        yield();
    void        releaseAccess();
    void        requestApiAccess();
    void        requestAccess();
    void        setupCurrentActivity();
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
    bool callObjectFunctionExit(RexxActivation *, RexxString *, bool, ProtectedObject &, RexxObject **, size_t);
    bool callFunctionExit(RexxActivation *, RexxString *, bool, ProtectedObject &, RexxObject **, size_t);
    bool callScriptingExit(RexxActivation *, RexxString *, bool, ProtectedObject &, RexxObject **, size_t);
    bool callCommandExit(RexxActivation *, RexxString *, RexxString *, ProtectedObject &result, ProtectedObject &condition);
    bool callPullExit(RexxActivation *, RexxString *&);
    bool callPushExit(RexxActivation *, RexxString *, QueueOrder);
    bool callQueueSizeExit(RexxActivation *, RexxInteger *&);
    bool callQueueNameExit(RexxActivation *, RexxString *&);
    bool callHaltTestExit(RexxActivation *);
    bool callHaltClearExit(RexxActivation *);
    bool callTraceTestExit(RexxActivation *, bool);
    bool callNovalueExit(RexxActivation *, RexxString *, RexxObject *&);
    bool callValueExit(RexxActivation *, RexxString *, RexxString *, RexxObject *, ProtectedObject&);
    void traceOutput(RexxActivation *, RexxString *);
    void sayOutput(RexxActivation *, RexxString *);
    void queue(RexxActivation *, RexxString *, QueueOrder);
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

    void nestAttach();
    void returnAttach();
    inline bool isNestedAttach() { return attachCount > 1 || (attachCount == 1 && !newThreadAttached); }
    inline void activate() { nestedCount++; }
    inline void deactivate() { nestedCount--; }
    inline bool isActive() { return nestedCount > 0; }
    inline bool isInactive() { return nestedCount == 0; }
    inline size_t getActivationLevel() { return nestedCount; }
    inline void restoreActivationLevel(size_t l) { nestedCount = l; }
    inline bool isSuspended() { return suspended; }
    inline void clearWaitingForDispatch() { waitingForDispatch = false; }
    inline void setWaitingForDispatch() { waitingForDispatch = true; }
    inline bool isWaitingForDispatch() { return waitingForDispatch; }
    inline void clearWaitingForApiAccess() { waitingForApiAccess = false; }
    inline void setWaitingForApiAccess() { waitingForApiAccess = true; }
    inline bool isWaitingForApiAccess() { return waitingForApiAccess; }
    inline bool isNestable() { return waitingOnSemaphore; }
    inline bool isWaiting() { return waitingForDispatch || waitingOnSemaphore; }
    inline void setSuspended(bool s) { suspended = s; }
    inline bool isInterpreterRoot() { return interpreterRoot; }
    inline void setInterpreterRoot() { interpreterRoot = true; }
    inline void setNestedActivity(Activity *a) { nestedActivity = a; }
    inline Activity *getNestedActivity() { return nestedActivity; }
    inline bool isAttached() { return attachCount > 0; }
           bool isSameActivityStack(Activity *a);
           void validateThread();

    SecurityManager *getEffectiveSecurityManager();
    SecurityManager *getInstanceSecurityManager();
    void inheritSettings(Activity *parent);
    void setupExits();
    void enterCurrentThread();
    void exitCurrentThread();
    void run(ActivityDispatcher &target);
    void run(CallbackDispatcher &target);
    void run(TrappingDispatcher &target);

    inline bool  waitForDispatch()
    {
        waitingOnSemaphore = true;
        bool timedOut = false;
        // we wait for a small interval, then post the semaphore
        // ourselves if we time out so the dispatcher knows we're no longer
        // waiting.
        if (!runSem.wait(MaxDispatchWait))
        {
            // our return indicates we timed out and are still in the wait queue
            timedOut = true;
            postDispatch();
        }
        waitingOnSemaphore = false;
        return timedOut;
    }

    inline void        waitForRunPermission() { waitingOnSemaphore = true; runSem.wait(); waitingOnSemaphore = false; }
    inline bool        hasRunPermission() { return dispatchPosted; }
    inline void        waitForGuardPermission() { waitingOnSemaphore = true; guardSem.wait(); waitingOnSemaphore = false; }
           void        waitForKernel();

    inline RexxActivation *getCurrentRexxFrame() {return currentRexxFrame;}
    inline ActivationBase *getTopStackFrame() { return topStackFrame; }
    inline size_t getActivationDepth() { return stackFrameDepth; }
    inline const NumericSettings *getNumericSettings () {return numericSettings;}
    inline RexxInternalObject *runningRequires(RexxString *program) {return requiresTable->get(program);}
    inline void        addRunningRequires(RexxString *program) { requiresTable->put(program, program);}
    inline void        removeRunningRequires(RexxInternalObject *program) { requiresTable->remove(program);}
    inline void        resetRunningRequires() { requiresTable->empty();}
    inline bool        checkRequires(RexxString *n) { return runningRequires(n) != OREF_NULL; }
    inline void        clearRunWait()  { runSem.reset(); dispatchPosted = false; }
    inline void        clearGuardWait()  { guardSem.reset(); }
           uint64_t    getRandomSeed();
           RexxString *getLastMessageName();

    inline RexxThreadContext *getThreadContext() { return &threadContext.threadContext; }
    inline NativeActivation *getApiContext() { return (NativeActivation *)topStackFrame; }

    inline void allocateStackFrame(ExpressionStack *stack, size_t entries)
    {
        stack->setFrame(frameStack.allocateFrame(entries), entries);
    }

    inline RexxInternalObject **allocateFrame(size_t entries)
    {
        return frameStack.allocateFrame(entries);
    }

    inline void releaseStackFrame(RexxInternalObject **frame)
    {
        frameStack.releaseFrame(frame);
    }

    inline void allocateLocalVariableFrame(RexxLocalVariables *locals)
    {
        locals->setFrame(frameStack.allocateFrame(locals->getSize()));
    }

    inline DirectoryClass *getCurrentCondition() { return conditionobj; }
    inline void           clearCurrentCondition() { conditionobj = OREF_NULL; }
    void setExitHandler(int exitNum, REXXPFN e) { getExitHandler(exitNum).setEntryPoint(e); }
    void setExitHandler(int exitNum, const char *e) { getExitHandler(exitNum).resolve(e); }
    void setExitHandler(RXSYSEXIT &e) { getExitHandler(e.sysexit_code).resolve(e.sysexit_name); }
    RexxString *resolveProgramName(RexxString *, RexxString *, RexxString *);
    void createMethodContext(MethodContext &context, NativeActivation *owner);
    void createCallContext(CallContext &context, NativeActivation *owner);
    void createExitContext(ExitContext &context, NativeActivation *owner);
    void createRedirectorContext(RedirectorContext &context, NativeActivation *owner);
    RexxObject *getLocalEnvironment(RexxString *name);
    DirectoryClass *getLocal();
    CommandHandler *resolveCommandHandler(RexxString *);
    void addMutex(MutexSemaphoreClass *m);
    void removeMutex(MutexSemaphoreClass *m);
    void cleanupMutexes();

    static void initializeThreadContext();

 protected:

    // the maximum time we'll wait while waiting for dispatch permission, in milliseconds.
    static const uint32_t MaxDispatchWait = 50;

    ExitHandler &getExitHandler(int exitNum) {  return instance->getExitHandler(exitNum); }
    inline bool isExitEnabled(int exitNum) { return getExitHandler(exitNum).isEnabled(); }
    inline void disableExit(int exitNum) { getExitHandler(exitNum).disable(); }


    InterpreterInstance *instance;         // the interpreter we're running under
    ActivityContext      threadContext;    // the handed out activity context
    Activity *oldActivity;                 // pushed nested activity
    ActivationStack      frameStack;       // our stack used for activation frames
    DirectoryClass      *conditionobj;     // condition object for killed activi
    StringTable         *requiresTable;    // Current ::REQUIRES being installed
    MessageClass        *dispatchMessage;  // a message object to run on this thread


    // the activation frame stack.  This stack is one RexxActivation or
    // NativeActivation for every level of the call stack.  The activationStackSize
    // is the current size of the stack (which is expanded, if necessary).  The
    // activationStackDepth is the current count of frames in the stack.
    InternalStack *activations;
    size_t   activationStackSize;
    size_t   stackFrameDepth;

    // the following two fields represent the current top of the activation stack
    // and the top Rexx frame in the stack.  Generally, if executing Rexx code,
    // then currentRexxFrame == topStackFrame.  If we're at the base of the stack
    // topStackFrame will be the root stack element (a NativeActivation instance)
    // and the currentRexxFrame will be OREF_NULL.  If we've made a callout from a
    // Rexx context, then the topStackFrame will be the NativeActivation that
    // made the callout and the currentRexxFrame will be the predecessor frame.
    RexxActivation     *currentRexxFrame;
    ActivationBase     *topStackFrame;  // top-most activation frame (can be either native or Rexx).
    RexxString         *currentExit;    // current executing system exit
    RexxInternalObject *waitingObject;  // object activity is waiting on
    SysSemaphore        runSem;         // activity run control semaphore
    SysSemaphore        guardSem;       // guard expression semaphore
    SysActivity currentThread;          // descriptor for this thread
    const NumericSettings *numericSettings; // current activation setting values

    bool     stackcheck;                // stack space is to be checked
    bool     exit;                      // activity loop is to exit
    bool     requestingString;          // in error handling currently
    bool     suspended;                 // the suspension flag
    bool     interpreterRoot;           // This is the root activity for an interpreter instance
    bool     waitingForApiAccess;       // This activity is waiting for access for an API callback
    bool     waitingForDispatch;        // This activity is in the dispatch queue waiting to be dispatched.
    bool     waitingOnSemaphore;        // activity is blocked while waiting for any semaphore
    bool     dispatchPosted;            // we have been given permission to run
    size_t   nestedCount;               // extent of the nesting
    size_t   attachCount;               // extent of nested attaches
    bool     newThreadAttached;         // Indicates this thread was a "side door" attach.
    char       *stackBase;              // pointer to base of C stack
    bool        clauseExitUsed;         // halt/trace sys exit not set ==> 1
    uint64_t    randomSeed;             // random number seed
    ProtectedBase *protectedObjects;    // list of stack-based object protectors
    ActivationFrame *activationFrames;  // list of stack-based object protectors
    Activity *nestedActivity;           // used to push down activities in threads with more than one instance
    IdentityTable *heldMutexes;         // a list of Mutex objects owned by this activity.

    // structures containing the various interface vectors
    static RexxThreadInterface threadContextFunctions;
    static MethodContextInterface methodContextFunctions;
    static CallContextInterface callContextFunctions;
    static ExitContextInterface exitContextFunctions;
    static IORedirectorInterface ioRedirectorContextFunctions;
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
inline NativeActivation *contextToActivation(RexxThreadContext *c)
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
inline NativeActivation *contextToActivation(RexxCallContext *c)
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
inline NativeActivation *contextToActivation(RexxExitContext *c)
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
inline NativeActivation *contextToActivation(RexxMethodContext *c)
{
    return ((MethodContext *)c)->context;
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
inline NativeActivation *contextToActivation(RexxIORedirectorContext *c)
{
    return ((RedirectorContext *)c)->context;
}

#endif
