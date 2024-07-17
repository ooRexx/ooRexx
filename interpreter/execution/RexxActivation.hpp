/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2024 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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

#include "ActivationBase.hpp"
#include "ExpressionStack.hpp"
#include "DoBlock.hpp"

#include "RexxCode.hpp"
#include "ActivityManager.hpp"
#include "CompoundVariableTail.hpp"
#include "ContextClass.hpp"
#include "StemClass.hpp"
#include "ActivationSettings.hpp"
#include "BaseExecutable.hpp"
#include "ProtectedObject.hpp"
#include "SystemInterpreter.hpp"

class RexxInstructionTrapBase;
class ProtectedObject;
class SupplierClass;
class PackageClass;
class StackFrameClass;
class RequiresDirective;
class CommandIOConfiguration;
class CommandIOContext;
class RoutineClass;

/**
 * An activation of a section of Rexx code.
 */
class RexxActivation : public ActivationBase
{
  public:

    // execution_state values
    typedef enum
    {
        ACTIVE,
        REPLIED,
        RETURNED,
    } ExecutionState;

    /**
     * An enumeration of the different trace prefixes.  The
     * trace prefix table must match these.
     */
    typedef enum
    {
        TRACE_PREFIX_CLAUSE   ,         //  0
        TRACE_PREFIX_ERROR    ,         //  1
        TRACE_PREFIX_RESULT   ,         //  2
        TRACE_PREFIX_DUMMY    ,         //  3
        TRACE_PREFIX_VARIABLE ,         //  4
        TRACE_PREFIX_DOTVARIABLE ,      //  5
        TRACE_PREFIX_LITERAL  ,         //  6
        TRACE_PREFIX_FUNCTION ,         //  7
        TRACE_PREFIX_PREFIX   ,         //  8
        TRACE_PREFIX_OPERATOR ,         //  9
        TRACE_PREFIX_COMPOUND ,         // 10
        TRACE_PREFIX_MESSAGE  ,         // 11
        TRACE_PREFIX_ARGUMENT ,         // 12
        TRACE_PREFIX_ASSIGNMENT,        // 13
        TRACE_PREFIX_INVOCATION,        // 14
        TRACE_PREFIX_NAMESPACE,         // 15
        TRACE_PREFIX_KEYWORD,           // 16
        TRACE_PREFIX_ALIAS,             // 17
        TRACE_PREFIX_INVOCATION_EXIT,   // 18

        // note: these values are for tagging, not for retrieving strings from the prefix table
        TRACE_OUTPUT_SOURCE = 30,   // for: void RexxActivation::traceSourceString()
        TRACE_OUTPUT,               // for: bool RexxActivation::doDebugPause(), void Activity::displayDebug(DirectoryClass *exobj),
                                    //      void Activity::display(DirectoryClass *exobj),
    } TracePrefix;

   void *operator new(size_t);
   inline void  operator delete(void *) { ; }

   inline RexxActivation(RESTORETYPE restoreType) { ; };
   RexxActivation();
   RexxActivation(Activity* _activity, MethodClass *_method, RexxCode *_code);
   RexxActivation(Activity *_activity, RoutineClass *_routine, RexxCode *_code, RexxString *calltype, RexxString *env, ActivationContext context);
   RexxActivation(Activity *_activity, RexxActivation *_parent, RexxCode *_code, ActivationContext context);

   void live(size_t) override;
   void liveGeneral(MarkReason reason) override;

   uint32_t getIdntfr();
   RexxObject *dispatch() override;
   wholenumber_t digits() override;
   wholenumber_t fuzz() override;
   bool form() override;
   void setDigits(wholenumber_t) override;
   void setFuzz(wholenumber_t) override;
   void setForm(bool) override;
   bool trap(RexxString *, DirectoryClass *) override;
   bool willTrap(RexxString *) override;

   void setObjNotify(MessageClass *) override;
   void termination() override;
   SecurityManager *getSecurityManager() override;
   bool isForwarded() override { return settings.isForwarded(); }
   const NumericSettings *getNumericSettings() override;
   RexxActivation  *getRexxContext() override;
   RexxActivation  *findRexxContext() override;
   RexxObject      *getReceiver() override;
   bool             isRexxContext() override;
   PackageClass    *getPackage() override;

   MethodClass     *getMethod();

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
   RexxObject      * forward(RexxObject *, RexxString *, RexxClass *, RexxObject **, size_t, bool);
   void              returnFrom(RexxObject *result);
   void              exitFrom(RexxObject *);
   void              procedureExpose(RexxVariableBase **variables, size_t count);
   void              expose(RexxVariableBase **variables, size_t count);
   void              autoExpose(RexxVariableBase **variables, size_t count);
   void              setTrace(const TraceSetting &);
   void              setTrace(RexxString *);
   void              raise(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *, DirectoryClass *);
   void              toggleAddress();
   void              guardOn();
   void              raiseExit(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *, DirectoryClass *);
   ActivationBase  * senderActivation(RexxString *conditionName);
   RexxActivation  * external();
   void              interpret(RexxString *);
   void              signalTo(RexxInstruction *);
   void              guardWait();
   void              debugSkip(wholenumber_t);
   RexxString      * traceSetting();
   void              iterate(RexxString *);
   void              leaveLoop(RexxString *);
   void              trapOn(RexxString *, RexxInstructionTrapBase *, bool);
   void              trapOff(RexxString *, bool);
   void              setAddress(RexxString *, CommandIOConfiguration *config);
   void              signalValue(RexxString *);
   RexxString      * trapState(RexxString *);
   void              trapDelay(RexxString *);
   void              trapUndelay(RexxString *);
   bool              callExternalRexx(RexxString *, RexxObject **, size_t, RexxString *, ProtectedObject &);
   RexxObject      * externalCall(RoutineClass *&, RexxString *, RexxObject **, size_t, RexxString *, ProtectedObject &);
   RexxObject      * externalCall(RexxString *, RoutineClass *, RexxObject **, size_t, RexxString *, ProtectedObject &);
   RexxObject      * internalCall(RexxString *, RexxInstruction *, RexxObject **, size_t, ProtectedObject &);
   RexxObject      * internalCallTrap(RexxString *, RexxInstruction *, DirectoryClass *, ProtectedObject &);
   bool              callMacroSpaceFunction(RexxString *, RexxObject **, size_t, RexxString *, int, ProtectedObject &);
   static RoutineClass* getMacroCode(RexxString *macroName);
   RexxString       *resolveProgramName(RexxString *name, ResolveType type);
   RexxClass        *findClass(RexxString *name);
   RexxObject       *resolveDotVariable(RexxString *name, RexxObject *&);
   void              command(RexxString *, RexxString *, CommandIOConfiguration *config);
   int64_t           getElapsed();
   RexxDateTime      getTime();
   RexxInteger     * random(RexxInteger *, RexxInteger *, RexxInteger *);
   size_t            currentLine();
   void              arguments(RexxObject *);
   void              traceValue(RexxObject *, TracePrefix);
   void              traceCompoundValue(TracePrefix prefix, RexxString *stemName, RexxInternalObject **tails, size_t tailCount, CompoundVariableTail &tail);
   void              traceCompoundValue(TracePrefix prefix, RexxString *stem, RexxInternalObject **tails, size_t tailCount, const char *marker, RexxObject * value);
   void              traceTaggedValue(TracePrefix prefix, const char *tagPrefix, bool quoteTag, RexxString *tag, const char *marker, RexxObject *value);
   void              traceOperatorValue(TracePrefix prefix, const char *tag, RexxObject *value);
   void              traceSourceString();
   void              traceClause(RexxInstruction *, TracePrefix);
   void              traceEntry();
   void              traceEntryOrExit(TracePrefix);
   void              resetElapsed();
   RexxString      * formatTrace(RexxInstruction *, PackageClass *);
   RexxString      * getTraceBack();
   DirectoryClass  * local();
   RexxString      * formatSourcelessTraceLine(RexxString *packageName);
   ArrayClass      * getStackFrames(bool skipFirst);
   void              implicitExit();

   void              unwindTrap(RexxActivation *);
   RexxString      * sourceString();
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
   void              loadRequires(RequiresDirective *);
   void              loadLibrary(RexxString *target, RexxInstruction *instruction, PackageClass *package);
   RexxObject      * rexxVariable(RexxString *);
   void              pushEnvironment(RexxObject *);
   RexxObject      * popEnvironment();
   bool              processTraps();
   void              mergeTraps(QueueClass *);
   uint64_t          getRandomSeed(RexxInteger *);
   VariableDictionary *getObjectVariables();
   StringTable     * getLabels();
   RexxString      * getProgramName();
   const char      * displayProgramName();
   RexxObject      * popControl();
   void              pushControl(RexxObject *);
   void              closeStreams();
   void              checkTrapTable();
   void              checkIOConfigTable();
   CommandIOConfiguration *getIOConfig(RexxString *environment);
   void              addIOConfig(RexxString *environment, CommandIOConfiguration *config);
   CommandIOContext *resolveAddressIOConfig(RexxString *address, CommandIOConfiguration *localConfig);
   RexxObject       *resolveStream(RexxString *name, bool input, Protected<RexxString> &fullName, bool *added);
   StringTable      *getStreams();
   RexxObject       *novalueHandler(RexxString *);
   RexxVariableBase *retriever(RexxString *);
   RexxVariableBase *directRetriever(RexxString *);
   RexxObject       *handleNovalueEvent(RexxString *name, RexxObject *defaultValue, RexxVariable *variable);
   PackageClass     *getPackageObject() { return packageObject; }
   inline PackageClass *getEffectivePackageObject()
   {
       return isInterpret() ? executable->getPackageObject() : packageObject;
   }

   RexxObject       *getLocalEnvironment(RexxString *name);
   void              setReturnStatus(ReturnStatus status);

   inline void setCallType(RexxString *type) {settings.calltype = type; }
   inline void pushBlockInstruction(DoBlock *block) { block->setPrevious(doStack); doStack = block; }
   inline void popBlockInstruction()
   {
       removeBlockInstruction();
       DoBlock *temp = doStack;
       doStack = temp->getPrevious();
       settings.traceIndent = temp->getIndent();
       temp->setHasNoReferences();
   }

   inline DoBlock *topBlockInstruction() { return doStack; }
   inline void terminateBlockInstruction(size_t _indent) { popBlockInstruction(); settings.traceIndent = _indent; }
   inline void terminateBlockInstruction() { popBlockInstruction();  }
   inline void newBlockInstruction(DoBlock *block) { pushBlockInstruction(block); blockNest++; settings.traceIndent++;}
   inline void removeBlockInstruction()
   {
       blockNest--;
       unindent();
   };
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


   inline void              traceIntermediate(RexxObject * v, TracePrefix p) { if (settings.intermediateTrace) traceValue(v, p); };
   inline void              traceArgument(RexxObject * v) { if (settings.intermediateTrace) traceValue(v, TRACE_PREFIX_ARGUMENT); };
   inline void              traceVariable(RexxString *n, RexxObject *v)
       { if (settings.intermediateTrace) { traceTaggedValue(TRACE_PREFIX_VARIABLE, NULL, false, n, VALUE_MARKER, v); } };
   inline void              traceDotVariable(RexxString *n, RexxObject *v)
       { if (settings.intermediateTrace) { traceTaggedValue(TRACE_PREFIX_DOTVARIABLE, ".", false, n, VALUE_MARKER, v); } };
   inline void              traceSpecialDotVariable(RexxString *n, RexxObject *v)
       { if (settings.intermediateTrace) { traceTaggedValue(TRACE_PREFIX_DOTVARIABLE, NULL, false, n, VALUE_MARKER, v); } };
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
   inline void              traceClassResolution(RexxString *n, RexxString *c, RexxObject *v)
       { if (settings.intermediateTrace) { traceTaggedValue(TRACE_PREFIX_NAMESPACE, NULL, false, n->concatWith(c, ':'), VALUE_MARKER, v); } };
   inline void              traceCompoundName(RexxString *stemVar, RexxInternalObject **tails, size_t tailCount, CompoundVariableTail &tail) { if (settings.intermediateTrace) traceCompoundValue(TRACE_PREFIX_COMPOUND, stemVar, tails, tailCount, VALUE_MARKER, tail.createCompoundName(stemVar)); };
   inline void              traceCompoundName(RexxString *stemVar, RexxInternalObject **tails, size_t tailCount, RexxString *tail) { if (settings.intermediateTrace) traceCompoundValue(TRACE_PREFIX_COMPOUND, stemVar, tails, tailCount, VALUE_MARKER, stemVar->concat(tail)); };
   inline void              traceCompound(RexxString *stemVar, RexxInternalObject **tails, size_t tailCount, RexxObject *value) { if (settings.intermediateTrace) traceCompoundValue(TRACE_PREFIX_VARIABLE, stemVar, tails, tailCount, VALUE_MARKER, value); };
   inline void              traceCompoundAssignment(RexxString *stemVar, RexxInternalObject **tails, size_t tailCount, RexxObject *value) { if (settings.intermediateTrace) traceCompoundValue(TRACE_PREFIX_ASSIGNMENT, stemVar, tails, tailCount, ASSIGNMENT_MARKER, value); };
   inline void              clearTraceSettings() { settings.packageSettings.traceSettings.setTraceOff(); settings.intermediateTrace = false; }
   inline bool              tracingResults() {return settings.packageSettings.traceSettings.tracingResults(); }
   inline bool              tracingAll() {return settings.packageSettings.traceSettings.tracingAll(); }
   inline bool              tracingLabels() {return settings.packageSettings.traceSettings.tracingLabels(); }
   inline bool              inDebug() { return settings.packageSettings.traceSettings.isDebug() && !debugPause;}
   inline void              traceResult(RexxObject * v) { if (tracingResults()) traceValue(v, TRACE_PREFIX_RESULT); };
   inline void              traceKeywordResult(RexxString *k, RexxObject *v) { if (tracingResults()) traceTaggedValue(TRACE_PREFIX_KEYWORD, NULL, true, k, VALUE_MARKER, v); }
   inline void              traceVariableAlias(RexxString *k, RexxString *v) { if (tracingResults()) traceTaggedValue(TRACE_PREFIX_ALIAS, NULL, true, k, VALUE_MARKER, v); }
   inline void              traceResultValue(RexxObject * v) { traceValue(v, TRACE_PREFIX_RESULT); };
   inline bool              tracingInstructions() { return tracingAll(); }
   inline bool              tracingErrors() { return settings.packageSettings.traceSettings.tracingErrors(); }
   inline bool              tracingFailures() { return settings.packageSettings.traceSettings.tracingFailures(); }
   inline void              traceInstruction(RexxInstruction * v) { if (tracingAll()) traceClause(v, TRACE_PREFIX_CLAUSE); }
   inline void              traceLabel(RexxInstruction * v) { if (settings.packageSettings.traceSettings.tracingLabels()) traceClause(v, TRACE_PREFIX_CLAUSE); };
   inline void              traceCommand(RexxInstruction * v) { if (tracingCommands()) traceClause(v, TRACE_PREFIX_CLAUSE); }
   inline bool              tracingCommands() { return settings.packageSettings.traceSettings.tracingCommands(); }
   inline bool              pausingInstructions() { return settings.packageSettings.traceSettings.pausingInstructions(); }
   inline void              pauseInstruction() {  if (pausingInstructions()) doDebugPause(); };
   inline int               conditionalPauseInstruction() { return pausingInstructions() ? doDebugPause(): false; };
   inline void              pauseLabel() { if (settings.packageSettings.traceSettings.pausingLabels()) doDebugPause(); };
   inline void              pauseCommand() { if (settings.packageSettings.traceSettings.pausingCommands()) doDebugPause(); };
   inline void              resetDebug()
   {
       settings.packageSettings.traceSettings.resetDebug();
       settings.setDebugBypass(true);
   }
   inline bool              isErrorSyntaxEnabled() { return settings.packageSettings.isErrorSyntaxEnabled(); }
   inline void              disableErrorSyntax() { return settings.packageSettings.disableErrorSyntax(); }
   inline bool              isFailureSyntaxEnabled() { return settings.packageSettings.isFailureSyntaxEnabled(); }
   inline void              disableFailureSyntax() { return settings.packageSettings.disableFailureSyntax(); }
   inline bool              isLostdigitsSyntaxEnabled() { return settings.packageSettings.isLostdigitsSyntaxEnabled(); }
   inline void              disableLostdigitsSyntax() { return settings.packageSettings.disableLostdigitsSyntax(); }
   inline bool              isNostringSyntaxEnabled() { return settings.packageSettings.isNostringSyntaxEnabled(); }
   inline void              disableNostringSyntax() { return settings.packageSettings.disableNostringSyntax(); }
   inline bool              isNotreadySyntaxEnabled() { return settings.packageSettings.isNotreadySyntaxEnabled(); }
   inline void              disableNotreadySyntax() { return settings.packageSettings.disableNotreadySyntax(); }
   inline bool              isNovalueSyntaxEnabled() { return settings.packageSettings.isNovalueSyntaxEnabled(); }
   inline void              disableNovalueSyntax() { return settings.packageSettings.disableNovalueSyntax(); }


   inline void              stopExecution(ExecutionState state)
   {
       executionState = state;
       next = OREF_NULL;
   }

   inline bool              noTracing(RexxObject *value) { return (settings.isTraceSuppressed() || debugPause || value == OREF_NULL || !code->isTraceable()); }
   inline bool              noTracing() { return (settings.isTraceSuppressed() || debugPause || !code->isTraceable()); }

          SecurityManager  *getEffectiveSecurityManager();
   inline bool              isTopLevel() { return (activationContext&TOP_LEVEL_CALL) != 0; }
   inline bool              isGuarded() { return settings.isGuarded(); }
   inline void              setGuarded() { settings.setGuarded(true); }
   inline bool              isObjectScopeLocked() { return this->objectScope == SCOPE_RESERVED; } // for concurrency trace
   unsigned short           getReserveCount() { VariableDictionary *ovd = this->getVariableDictionary(); return ovd ? ovd->getReserveCount() : 0; } // for concurrency trace. Try to get the ovd counter, even if not yet assigned to current activation.
   VariableDictionary *     getVariableDictionary() { return this->receiver ? this->receiver->getObjectVariables(this->scope) : NULL; } // for concurrency trace. Try to get the ovd, even if not yet assigned to current activation.

          void              enableExternalTrace();
          void              disableExternalTrace();

   inline bool              isElapsedTimerReset() { return settings.isElapsedTimerReset(); }
   inline void              setElapsedTimerInvalid() { settings.setElapsedTimerReset(true); }
   inline void              setElapsedTimerValid() { settings.setElapsedTimerReset(false); }


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

   inline DirectoryClass *getAllLocalVariables()
   {
       return getLocalVariables()->getVariableDirectory();
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

   inline void aliasLocalVariable(RexxString *name, size_t index, RexxVariable *variable)
   {
       settings.localVariables.aliasVariable(name, index, variable);
   }


   RexxObject *evaluateLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount);
   RexxObject *getLocalCompoundVariableValue(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount);
   RexxObject *getLocalCompoundVariableRealValue(RexxString *localstem, size_t index, RexxInternalObject **tail, size_t tailCount);
   CompoundTableElement *getLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount);
   CompoundTableElement *exposeLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount);
   bool localCompoundVariableExists(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount);
   void assignLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount, RexxObject *value);
   void setLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount, RexxObject *value);
   void dropLocalCompoundVariable(RexxString *stemName, size_t index, RexxInternalObject **tail, size_t tailCount);

   inline bool novalueEnabled() { return settings.localVariables.getNovalue(); }

   inline RexxVariable *newLocalVariable(RexxString *name)
   {
       RexxVariable *newVariable = new_variable(name);
       newVariable->setCreator(this);
       return newVariable;
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
   }

   inline void setLocalVariableDictionary(VariableDictionary *dict) {settings.localVariables.setDictionary(dict); }

   void sayOutput(RexxString *line);
   RexxString *pullInput();
   RexxString *lineIn();
   void queue(RexxString *, Activity::QueueOrder);
   StringTable *getFileNames();
   void removeFileName(RexxString *name);
   bool notCaseSensitive() { return settings.caseInsensitive; }

   static const size_t yieldInstructions = 50;            // the number of instructions we'll execute before a yield check

   static const uint64_t RANDOM_FACTOR = 25214903917LL;   // random multiplication factor
   static const uint64_t RANDOM_ADDER = 11LL;
                                       // randomize a seed number
   static inline uint64_t RANDOMIZE(uint64_t seed) { return (seed * RANDOM_FACTOR + RANDOM_ADDER); }
                                        // size of a size_t value in bits
   static const size_t SIZE_BITS = sizeof(void *) * 8;

   // some values for random processing
   static const size_t DefaultRandomMin = 0;               // default lower bounds
   static const size_t DefaultRandomMax = 999;             // default upper bounds
   static const size_t MaxRandomRange = 999999999;         // the maximum range between lower and upper bounds.

   // marker used for tagged traces to separate tag from the value
   static const char *VALUE_MARKER;
   // marker used for tagged traces to separate tag from the value
   static const char *ASSIGNMENT_MARKER;

   void displayUsingTraceOutput(Activity *, RexxString *);

   static StringTable *getStackFrameAsStringTable(StackFrameClass *);

 protected:

    static StringTable * createTraceObject(Activity *, RexxActivation *, RexxString *line, TracePrefix tracePrefix, RexxString *tag, RexxObject *value);
    void processTraceInfo(Activity *, RexxString *traceLine, TracePrefix tracePrefix, RexxString *tag, RexxObject *value);

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
    bool                 traceEntryAllowed; // true if first instruction (other than expose)
    bool                 traceEntryDone;// true if the entry in a routine or method has been traced
    RexxObject          *result;        // result of execution
    ArrayClass          *trapInfo;      // current trap handler
    RexxContext         *contextObject; // the context object representing the execution context
                                        // current activation state
    ExecutionState       executionState;
                                        // type of activation activity
    ActivationContext    activationContext;
    MessageClass        *notifyObject;  // an object to notify if excep occur
                                        // list of Saved Local environments
    QueueClass          *environmentList;
                                        // queue of trapped conditions
    QueueClass          *conditionQueue;// queue of trapped conditions
    uint64_t             randomSeed;    // random number seed
    bool                 randomSet;     // random seed has been set
    size_t               blockNest;     // block instruction nesting level
    size_t               instructionCount;  // The number of instructions since we last yielded control
    uint32_t             idntfr;        // idntfr for concurrency trace
 };
 #endif
