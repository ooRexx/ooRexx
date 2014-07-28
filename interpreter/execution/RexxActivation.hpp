/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                            RexxActivation.hpp  */
/*                                                                            */
/* Primitive Activation Class Definitions                                     */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxActivation
#define Included_RexxActivation

#include "ExpressionStack.hpp"           // needs expression stack
#include "DoBlock.hpp"                   // need do block definition
                                         // various activation settings
#include "RexxCode.hpp"
#include "ActivityManager.hpp"
#include "CompoundVariableTail.hpp"
#include "ContextClass.hpp"
#include "StemClass.hpp"
#include "ActivationSetting.hpp"

class RexxInstructionCallBase;
class ProtectedObject;
class SupplierClass;
class PackageClass;
class StackFrameClass;


#define MS_PREORDER   0x01                  // Macro Space Pre-Search
#define MS_POSTORDER  0x02                  // Macro Space Post-Search


/**
 * An activation of a section of Rexx code.
 */
class RexxActivation : public RexxActivationBase
{
  public:

    // execution_state values
    typedef enum
    {
        ACTIVE,
        REPLIED,
        RETURNED,
    } ExecutionState;

    // command return status.
    typedef enum
    {
        /**
         * Guard scope settings.
         */
        RETURN_STATUS_NORMAL = 0,
        RETURN_STATUS_ERROR = 1,
        RETURN_STATUS_FAILURE = -1
    }  ReturnStatus;


    // activationContext values
    // these are done as bit settings to
    // allow multiple potential values
    // to be checked with a single test
    typedef enum
    {
        DEBUGPAUSE   = 0x00000001,
        METHODCALL   = 0x00000002,
        INTERNALCALL = 0x00000004,
        INTERPRET    = 0x00000008,
        PROGRAMCALL  = 0x00000010,
        EXTERNALCALL = 0x00000020,
                                       // check for top level execution
        TOP_LEVEL_CALL = (PROGRAMCALL | METHODCALL | EXTERNALCALL),
                                       // non-method top level execution
        PROGRAM_LEVEL_CALL = (PROGRAMCALL | EXTERNALCALL),
                                       // non-method top level execution
        PROGRAM_OR_METHOD = (PROGRAMCALL | METHODCALL),
                                       // call is within an activation
        INTERNAL_LEVEL_CALL = (INTERNALCALL | INTERPRET),
    } ActivationContext;

    // guard scopy settings
    typedef enum
    {
        SCOPE_RELEASED = 0,
        SCOPE_RESERVED = 1,
    } GuardStatus;


   void *operator new(size_t);
   inline void *operator new(size_t size, void *ptr) {return ptr;};
   inline void  operator delete(void *) { ; }
   inline void  operator delete(void *, void *) { ; }

   inline RexxActivation(RESTORETYPE restoreType) { ; };
   RexxActivation();
   RexxActivation(Activity* _activity, MethodClass *_method, RexxCode *_code);
   RexxActivation(Activity *_activity, RoutineClass *_routine, RexxCode *_code, RexxString *calltype, RexxString *env, ActivationContext context);
   RexxActivation(Activity *_activity, RexxActivation *_parent, RexxCode *_code, ActivationContext context);

   virtual void live(size_t);
   virtual void liveGeneral(MarkReason reason);

   RexxObject *dispatch();
   size_t      digits();
   size_t      fuzz();
   bool        form();
   void        setDigits(size_t);
   void        setFuzz(size_t);
   void        setForm(bool);
   void        setDigits();
   void        setFuzz();
   void        setForm();
   bool        trap(RexxString *, DirectoryClass *);
   void        setObjNotify(MessageClass *);
   void        termination();
   void        inheritPackageSettings();
   void        allocateStackFrame();
   void        allocateLocalVariables();
   inline void guardOff()
   {
       // if currently locked, release the variable dictionary and change the scopey
       if (objectScope == SCOPE_RESERVED)
       {
                                            // release the variable dictionary
           settings.objectVariables->release(activity);
           objectScope = SCOPE_RELEASED;
       }
   }

   inline bool isInterpret() { return activationContext == INTERPRET; }
   inline bool isInternalCall() { return activationContext == INTERNALCALL; }
   inline bool isMethod() { return activationContext == METHODCALL; }
   inline bool isRoutine() { return activationContext == EXTERNALCALL; }
   inline bool isProgram() { return activationContext == PROGRAMCALL; }
   inline bool isTopLevelCall() { return (activationContext & TOP_LEVEL_CALL) != 0; }
   inline bool isProgramLevelCall() { return (activationContext & PROGRAM_LEVEL_CALL) != 0; }
   inline bool isInternalLevelCall() { return (activationContext & INTERNAL_LEVEL_CALL) != 0; }
   inline bool isProgramOrMethod() { return (activationContext & PROGRAM_OR_METHOD) != 0; }
   inline bool isMethodOrRoutine() { return isMethod() || isRoutine(); }

   RexxObject *run(RexxObject *_receiver, RexxString *msgname, RexxObject **_arglist,
       size_t _argcount, RexxInstruction * start, ProtectedObject &resultObj);
   inline     RexxObject *run(RexxObject **_arglist, size_t _argcount, ProtectedObject &_result)
   {
       return run(OREF_NULL, OREF_NULL, _arglist, _argcount, OREF_NULL, _result);
   }
   void              reply(RexxObject *);
   RexxObject      * forward(RexxObject *, RexxString *, RexxObject *, RexxObject **, size_t, bool);
   void              returnFrom(RexxObject *result);
   void              exitFrom(RexxObject *);
   void              procedureExpose(RexxVariableBase **variables, size_t count);
   void              expose(RexxVariableBase **variables, size_t count);
   void              setTrace(size_t, size_t);
   void              setTrace(RexxString *);
   void              raise(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *, DirectoryClass *);
   void              toggleAddress();
   void              guardOn();
   void              raiseExit(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *, DirectoryClass *);
   RexxActivation  * senderActivation();
   RexxActivation  * external();
   void              interpret(RexxString *);
   void              signalTo(RexxInstruction *);
   void              guardWait();
   void              debugSkip(wholenumber_t, bool);
   RexxString      * traceSetting();
   void              iterate(RexxString *);
   void              leaveLoop(RexxString *);
   void              trapOn(RexxString *, RexxInstructionCallBase *);
   void              trapOff(RexxString *);
   void              setAddress(RexxString *);
   void              signalValue(RexxString *);
   RexxString      * trapState(RexxString *);
   void              trapDelay(RexxString *);
   void              trapUndelay(RexxString *);
   bool              callExternalRexx(RexxString *, RexxObject **, size_t, RexxString *, ProtectedObject &);
   RexxObject      * externalCall(RexxString *, size_t, ExpressionStack *, RexxString *, ProtectedObject &);
   RexxObject      * internalCall(RexxString *, RexxInstruction *, size_t, ExpressionStack *, ProtectedObject &);
   RexxObject      * internalCallTrap(RexxString *, RexxInstruction *, DirectoryClass *, ProtectedObject &);
   bool              callMacroSpaceFunction(RexxString *, RexxObject **, size_t, RexxString *, int, ProtectedObject &);
   static RoutineClass* getMacroCode(RexxString *macroName);
   RexxString       *resolveProgramName(RexxString *name);
   RexxClass        *findClass(RexxString *name);
   RexxObject       *resolveDotVariable(RexxString *name);
   void              command(RexxString *, RexxString *);
   int64_t           getElapsed();
   RexxDateTime      getTime();
   RexxInteger     * random(RexxInteger *, RexxInteger *, RexxInteger *);
   size_t            currentLine();
   void              arguments(RexxObject *);
   void              traceValue(RexxObject *, int);
   void              traceCompoundValue(int prefix, RexxString *stemName, RexxObject **tails, size_t tailCount, CompoundVariableTail *tail);
   void              traceCompoundValue(int prefix, RexxString *stem, RexxObject **tails, size_t tailCount, const char *marker, RexxObject * value);
   void              traceTaggedValue(int prefix, const char *tagPrefix, bool quoteTag, RexxString *tag, const char *marker, RexxObject * value);
   void              traceOperatorValue(int prefix, const char *tag, RexxObject *value);
   void              traceSourceString();
   void              traceClause(RexxInstruction *, int);
   void              traceEntry();
   void              resetElapsed();
   RexxString      * formatTrace(RexxInstruction *, PackageClass *);
   RexxString      * getTraceBack();
   DirectoryClass  * local();
   RexxString      * formatSourcelessTraceLine(RexxString *packageName);
   ArrayClass      * getStackFrames(bool skipFirst);
   void              implicitExit();

   void              unwindTrap(RexxActivation *);
   RexxString      * sourceString();
   void              addLocalRoutine(RexxString *name, MethodClass *method);
   StringTable      *getPublicRoutines();
   void              debugInterpret(RexxString *);
   bool              doDebugPause();
   void              processClauseBoundary();
   bool              halt(RexxString *);
   void              externalTraceOn();
   void              externalTraceOff();
   void              yield();
   void              propagateExit(RexxObject *);
   void              setDefaultAddress(RexxString *);
   bool              internalMethod();
   PackageClass    * loadRequires(RexxString *, RexxInstruction *);
   void              loadLibrary(RexxString *target, RexxInstruction *instruction);
   RexxObject      * rexxVariable(RexxString *);
   void              pushEnvironment(RexxObject *);
   RexxObject      * popEnvironment();
   void              processTraps();
   void              mergeTraps(QueueClass *);
   uint64_t          getRandomSeed(RexxInteger *);
   void              adjustRandomSeed() { randomSeed += (uint64_t)(uintptr_t)this; }
   VariableDictionary * getObjectVariables();
   StringTable     * getLabels();
   RexxString      * getProgramName();
   RexxObject      * popControl();
   void              pushControl(RexxObject *);
   void              closeStreams();
   void              checkTrapTable();
   RexxObject       *resolveStream(RexxString *name, bool input, RexxString **fullName, bool *added);
   StringTable      *getStreams();
   RexxObject       *novalueHandler(RexxString *);
   RexxVariableBase *retriever(RexxString *);
   RexxVariableBase *directRetriever(RexxString *);
   RexxInternalObject *handleNovalueEvent(RexxString *name, RexxInternalObject *defaultValue, RexxVariable *variable);
   PackageClass     *getPackageObject() { return packageObject; }
   inline PackageClass *getEffectivePackageObject()
   {
       return isInterpret() ? executable->getPackageObject() : packageObject;
   }

   PackageClass     *getPackage();
   RexxObject       *getLocalEnvironment(RexxString *name);
   void              setReturnStatus(int status);

   inline void              setCallType(RexxString *type) {settings.calltype = type; }
   inline void              pushBlockInstruction(DoBlock *block) { block->setPrevious(doStack); doStack = block; }
   inline void              popBlockInstruction() { DoBlock *temp; temp = doStack; doStack = temp->getPrevious(); temp->setHasNoReferences(); }
   inline DoBlock         * topBlockInstruction() { return doStack; }
   inline void              terminateBlockInstruction(size_t _indent) { popBlockInstruction(); blockNest--; settings.traceIndent = _indent; }
   inline void              terminateBlockInstruction() { settings.traceIndent = doStack->getIndent(); popBlockInstruction(); blockNest--; }
   inline void              newBlockInstruction(DoBlock *block) { pushBlockInstruction(block); blockNest++; settings.traceIndent++;}
   inline void              removeBlockInstruction() { blockNest--; unindent(); };
   inline void              addBlockInstruction()    { blockNest++; indent(); };
   inline bool              hasActiveBlockInstructions() { return blockNest != 0; }
   inline bool              inMethod()  {return activationContext == METHODCALL; }
   inline void              indent() {settings.traceIndent++; };
   inline void              unindent() {if (settings.traceIndent > 0) settings.traceIndent--; };
   inline void              unindentTwice() {if (settings.traceIndent > 1) settings.traceIndent -= 2; };
   inline void              setIndent(size_t v) {settings.traceIndent=(v); };
   inline size_t            getIndent() {return settings.traceIndent;};
   inline bool              tracingIntermediates() {return settings.intermediateTrace;};
   inline Activity        * getActivity() {return activity;};
   inline RexxString      * getMessageName() {return settings.messageName;};
   inline RexxString      * getCallname() {return settings.messageName;};
   inline RexxInstruction * getCurrent() {return current;};
   inline void              getSettings(ActivationSettings &s) { settings = s; }
   inline void              putSettings(ActivationSettings &s) { s = settings; }
   inline RexxString      * getAddress() {return settings.currentAddress;};
   inline DirectoryClass  * getConditionObj() { return settings.conditionObj; }
   inline void              setConditionObj(DirectoryClass *condition) {settings.conditionObj = condition;};
   inline RexxInstruction * getNext() {return next;};
   inline void              setNext(RexxInstruction * v) {next=v;};
   inline void              setCurrent(RexxInstruction * v) {current=v;};

   inline ExpressionStack * getStack() {return &stack; };

   virtual NumericSettings *getNumericSettings();
   virtual RexxActivation  *getRexxContext();
   virtual RexxActivation  *findRexxContext();
   virtual RexxObject      *getReceiver();
   virtual bool             isRexxContext();

   inline void              traceIntermediate(RexxObject * v, int p) { if (settings.intermediateTrace) traceValue(v, p); };
   inline void              traceArgument(RexxObject * v) { if (settings.intermediateTrace) traceValue(v, TRACE_PREFIX_ARGUMENT); };
   inline void              traceVariable(RexxString *n, RexxObject *v)
       { if (settings.intermediateTrace) { traceTaggedValue(TRACE_PREFIX_VARIABLE, NULL, false, n, VALUE_MARKER, v); } };
   inline void              traceDotVariable(RexxString *n, RexxObject *v)
       { if (settings.intermediateTrace) { traceTaggedValue(TRACE_PREFIX_DOTVARIABLE, ".", false, n, VALUE_MARKER, v); } };
   inline void              traceFunction(RexxString *n, RexxObject *v)
       { if (settings.intermediateTrace) { traceTaggedValue(TRACE_PREFIX_FUNCTION, NULL, false, n, VALUE_MARKER, v); } };
   inline void              traceMessage(RexxString *n, RexxObject *v)
       { if (settings.intermediateTrace) { traceTaggedValue(TRACE_PREFIX_MESSAGE, NULL, true, n, VALUE_MARKER, v); } };
   inline void              traceOperator(const char *n, RexxObject *v)
       { if (settings.intermediateTrace) { traceOperatorValue(TRACE_PREFIX_OPERATOR, n, v); } };
   inline void              tracePrefix(const char *n, RexxObject *v)
       { if (settings.intermediateTrace) { traceOperatorValue(TRACE_PREFIX_PREFIX, n, v); } };
   inline void              traceAssignment(RexxString *n, RexxObject *v)
       { if (settings.intermediateTrace) { traceTaggedValue(TRACE_PREFIX_ASSIGNMENT, NULL, false, n, ASSIGNMENT_MARKER, v); } };
   inline void              traceCompoundName(RexxString *stemVar, RexxObject **tails, size_t tailCount, CompoundVariableTail *tail) { if (settings.intermediateTrace) traceCompoundValue(TRACE_PREFIX_COMPOUND, stemVar, tails, tailCount, VALUE_MARKER, tail->createCompoundName(stemVar)); };
   inline void              traceCompoundName(RexxString *stemVar, RexxObject **tails, size_t tailCount, RexxString *tail) { if (settings.intermediateTrace) traceCompoundValue(TRACE_PREFIX_COMPOUND, stemVar, tails, tailCount, VALUE_MARKER, stemVar->concat(tail)); };
   inline void              traceCompound(RexxString *stemVar, RexxObject **tails, size_t tailCount, RexxObject *value) { if (settings.intermediateTrace) traceCompoundValue(TRACE_PREFIX_VARIABLE, stemVar, tails, tailCount, VALUE_MARKER, value); };
   inline void              traceCompoundAssignment(RexxString *stemVar, RexxObject **tails, size_t tailCount, RexxObject *value) { if (settings.intermediateTrace) traceCompoundValue(TRACE_PREFIX_ASSIGNMENT, stemVar, tails, tailCount, ASSIGNMENT_MARKER, value); };
   inline void              clearTraceSettings() { settings.traceSettings.clear(); settings.intermediateTrace = false; }
   inline bool              tracingResults() {return (settings.traceSettings.tracingResults(); }
   inline bool              tracingAll() {return (settings.traceSettings.tracingAll(); }
   inline bool              inDebug() { return settings.traceSettings.inDebug() && !debugPause;}
   inline void              traceResult(RexxObject * v) { if (tracingResults()) traceValue(v, TRACE_PREFIX_RESULT); };
   inline bool              tracingInstructions() { return tracingAll(); }
   inline bool              tracingErrors() { return settings.traceSettings.traceErrors(); }
   inline bool              tracingFailures() { return settings.traceSettings.traceFailures(); }
   inline void              traceInstruction(RexxInstruction * v) { if (tracingAll()) traceClause(v, TRACE_PREFIX_CLAUSE); }
   inline void              traceLabel(RexxInstruction * v) { if (settings.traceSettings.tracingLabels()) traceClause(v, TRACE_PREFIX_CLAUSE); };
   inline void              traceCommand(RexxInstruction * v) { if (tracingCommands()) traceClause(v, TRACE_PREFIX_CLAUSE); }
   inline bool              tracingCommands() { return settings.traceSettings.tracingCommands(); }
   inline bool              pausingInstructions() { return (settings.traceSettings.pausingIntructions(); }
   inline void              pauseInstruction() {  if (pausingInstructions()) doDebugPause(); };
   inline int               conditionalPauseInstruction() { return pausingInstructions() ? doDebugPause(): false; };
   inline void              pauseLabel() { if (settings.traceSettings.pausingLabels()) doDebugPause(); };
   inline void              pauseCommand() { if (settings.traceSettings.pausingCommands()) doDebugPause(); };
   inline void              resetDebug()
   {
       settings.traceSettings.resetDebug();
       settings.stateFlags[debugBypass] = true;
   }
   inline bool              noTracing(RexxObject *value) { return (settings.stateFlags[traceSuppress] || debugPause || value == OREF_NULL || !code->isTraceable()); }
   inline bool              noTracing() { return (settings.stateFlags[traceSuppress] || debugPause || !code->isTraceable()); }

          SecurityManager  *getSecurityManager();
          SecurityManager  *getEffectiveSecurityManager();
   inline bool              isTopLevel() { return (activationContext&TOP_LEVEL_CALL) != 0; }
   inline bool              isForwarded() { return settings.stateFlags[forwarded]; }
   inline bool              isGuarded() { return settings.stateFlags[guardedmethod]; }
   inline void              setGuarded() { settings.stateFlags.set(guardedMethod); }

   inline bool              isExternalTraceOn() { return settings.stateFlags[traceOn]; }
   inline void              setExternalTraceOn() { settings.stateFlags.set(traceOn); }
   inline void              setExternalTraceOff() { settings.stateFlags.reset(traceOn); }
          void              enableExternalTrace();

   inline bool              isElapsedTimerReset() { return (settings.stateFlags[elapsedReset]; }
   inline void              setElapsedTimerInvalid() { settings.stateFlags.set(elapsedReset); }
   inline void              setElapsedTimerValid() { settings.stateFlags.reset(elapsedReset); }


   inline RexxObject     ** getMethodArgumentList() { return argList; };
   inline size_t            getMethodArgumentCount() { return argCount; }
   inline RexxObject *      getMethodArgument(size_t position)
   {
       if (position > getMethodArgumentCount())
       {
           return OREF_NULL;
       }
       else
       {
           return argList[position-1];
       }
   }

   inline ArrayClass       *getArguments() { return new_array(argCount, argList); }

   inline RexxObject      **getProgramArgumentlist() {return settings.parentArgList;};
   inline size_t            getProgramArgumentCount() { return settings.parentArgCount; }

   inline RexxObject *      getProgramArgument(size_t position)
   {
       if (position > getProgramArgumentCount())
       {
           return OREF_NULL;
       }
       else
       {
           return settings.parentArgList[position-1];
       }
   }

   RexxObject *getContextObject();
   RexxObject *getContextLine();
   size_t      getContextLineNumber();
   RexxObject *getContextReturnStatus();
   StackFrameClass *createStackFrame();

   inline VariableDictionary *getLocalVariables()
   {
       return settings.localVariables.getDictionary();
   }

   inline StringTable *getAllLocalVariables()
   {
       return getLocalVariables()->getAllVariables();
   }

   inline RexxVariable *getLocalVariable(RexxString *name, size_t index)
   {
       RexxVariable *target = settings.localVariables.get(index);
       if (target == OREF_NULL)
       {
           target = settings.localVariables.lookupVariable(name, index);
       }
       return target;
   }

   inline RexxVariable *getLocalStemVariable(RexxString *name, size_t index)
   {
       RexxVariable *target = settings.localVariables.get(index);
       if (target == OREF_NULL)
       {
           target = settings.localVariables.lookupStemVariable(name, index);
       }
       return target;
   }

   inline StemClass *getLocalStem(RexxString *name, size_t index)
   {
       return (StemClass *)getLocalStemVariable(name, index)->getVariableValue();
   }

   inline void dropLocalStem(RexxString *name, size_t index)
   {
       RexxVariable *stemVar = getLocalStemVariable(name, index);
       // create a new stem element and set this
       stemVar->set(new StemClass(name));
   }

   inline bool localStemVariableExists(RexxString *stemName, size_t index)
   {
     // get the stem entry from this dictionary
     RexxVariable *variable = settings.localVariables.find(stemName, index);
     // The stem exists if the stem variable has ever been used.
     return variable != OREF_NULL;
   }

   inline bool localVariableExists(RexxString *name, size_t index)
   {
     // get the stem entry from this dictionary
     RexxVariable *variable = settings.localVariables.find(name, index);
     // The stem exists if the stem variable has ever been used.
     return variable != OREF_NULL && variable->getVariableValue() != OREF_NULL;
   }

   inline void putLocalVariable(RexxVariable *variable, size_t index)
   {
       settings.localVariables.putVariable(variable, index);
   }

   inline void updateLocalVariable(RexxVariable *variable)
   {
       settings.localVariables.updateVariable(variable);
   }

   inline void setLocalVariable(RexxString *name, size_t index, RexxObject *value)
   {
       RexxVariable *variable = getLocalVariable(name, index);
       variable->set(value);
   }

   inline void dropLocalVariable(RexxString *name, size_t index)
   {
       RexxVariable *variable = getLocalVariable(name, index);
       variable->drop();
   }

   RexxObject *evaluateLocalCompoundVariable(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount);
   RexxObject *getLocalCompoundVariableValue(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount);
   RexxObject *getLocalCompoundVariableRealValue(RexxString *localstem, size_t index, RexxObject **tail, size_t tailCount);
   CompoundTableElement *getLocalCompoundVariable(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount);
   CompoundTableElement *exposeLocalCompoundVariable(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount);
   bool localCompoundVariableExists(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount);
   void assignLocalCompoundVariable(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount, RexxObject *value);
   void setLocalCompoundVariable(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount, RexxObject *value);
   void dropLocalCompoundVariable(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount);

   inline bool novalueEnabled() { return settings.localVariables.getNovalue(); }

   // The following methods be rights should be implemented by the
   // RexxMemory class, but aren't because of the difficulties of
   // making them inline methods that use the RexxVariable class.
   // Therefore, we're going to break the encapsulation rules
   // slightly and allow the activation class to manipulate that
   // chain directly.
   inline RexxVariable *newLocalVariable(RexxString *name)
   {
       RexxVariable *newVariable = memoryObject.variableCache;
       if (newVariable != OREF_NULL)
       {
           memoryObject.variableCache = newVariable->getNext();
           newVariable->reset(name);
       }
       else
       {
           newVariable = new_variable(name);
       }
       newVariable->setCreator(this);
       return newVariable;
   }

   inline void cacheLocalVariable(RexxVariable *var)
   {
       var->cache(memoryObject.variableCache);
       memoryObject.variableCache = var;
   }

   inline void cleanupLocalVariables()
   {
       // if we're nested, we need to make sure that any variable
       // dictionary created at this level is propagated back to
       // the caller.
       if (isInternalLevelCall() && settings.localVariables.isNested())
       {
           parent->setLocalVariableDictionary(settings.localVariables.getNestedDictionary());
       }
       else
       {
           // we need to cleanup the local variables and return them to the
           // cache.
           for (size_t i = 0; i < settings.localVariables.size; i++)
           {
               RexxVariable *var = settings.localVariables.get(i);
               if (var != OREF_NULL && var->isLocal(this))
               {
                   cacheLocalVariable(var);
               }
           }
       }
   }

   inline void setLocalVariableDictionary(VariableDictionary *dict) {settings.localVariables.setDictionary(dict); }

   static const uint64_t RANDOM_FACTOR = 25214903917LL;   // random multiplication factor
   static const uint64_t RANDOM_ADDER = 11LL;
                                       // randomize a seed number
   static inline uint64_t RANDOMIZE(uint64_t seed) { return (seed * RANDOM_FACTOR + RANDOM_ADDER); }
                                        // size of a size_t value in bits
   static const size_t SIZE_BITS = sizeof(void *) * 8;

 protected:

    ActivationSettings   settings;      // inherited REXX settings
    ExpressionStack      stack;         // current evaluation stack
    RexxCode            *code;          // rexx method object
    PackageClass        *packageObject; // the source object associated with this instance
    RexxClass           *scope;         // scope of any active method call
    RexxObject          *receiver;      // target of a message invocation
    Activity            *activity;      // current running activation
    RexxActivation      *parent;        // previous running activation for internal call/interpret
    RexxObject         **argList;       // activity argument list
    size_t               argCount;      // the count of arguments
    DoBlock             *doStack;       // stack of DO loops
    RexxInstruction     *current;       // current execution pointer
    RexxInstruction     *next;          // next instruction to execute
    bool                 debugPause;    // executing a debug pause
    bool                 clauseBoundary;// special flag for clause boundary checks
    GuardStatus          objectScope;   // reserve/release state of variables
    RexxObject          *result;        // result of execution
    ArrayClass          *trapInfo;      // current trap handler
    RexxContext         *contextObject; // the context object representing the execution context
                                        // current activation state
    ExecutionState       executionState;
                                        // type of activation activity
    ActivationContext    activationContext;
    MessageClass        *notifyObject;  // an object to notify if excep occur
                                        // list of Saved Local environments
    ListClass           *environmentList;
                                        // queue of trapped conditions
    QueueClass          *conditionQueue;// queue of trapped conditions
    // TODO:  create a random number encapsulation class
    uint64_t             randomSeed;    // random number seed
    bool                 randomSet;     // random seed has been set
    size_t               blockNest;     // block instruction nesting level

 };
 #endif
