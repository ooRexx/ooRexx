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
/* REXX Kernel                                            RexxActivation.hpp  */
/*                                                                            */
/* Primitive Activation Class Definitions                                     */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxActivation
#define Included_RexxActivation

#include "ExpressionStack.hpp"           /* needs expression stack            */
#include "DoBlock.hpp"                   /* need do block definition          */
                                         /* various activation settings       */
#include "RexxLocalVariables.hpp"        /* local variable cache definitions  */
#include "RexxDateTime.hpp"
#include "RexxCode.hpp"
#include "ActivityManager.hpp"
#include "RexxCompoundTail.hpp"
#include "ContextClass.hpp"


class RexxInstructionCallBase;
class ProtectedObject;
class RexxSupplier;
class PackageClass;
class StackFrameClass;


/******************************************************************************/
/* Random number generation constants                                         */
/******************************************************************************/

const uint64_t RANDOM_FACTOR = 25214903917LL;   /* random multiplication factor      */
const uint64_t RANDOM_ADDER = 11LL;
                                       /* randomize a seed number           */
inline uint64_t RANDOMIZE(uint64_t seed) { return (seed * RANDOM_FACTOR + RANDOM_ADDER); }
                                        // size of a size_t value in bits
const size_t SIZE_BITS = sizeof(void *) * 8;

/* execution_state values */
#define ACTIVE    0
#define REPLIED   1
#define RETURNED  2

#define RETURN_STATUS_NORMAL 0
#define RETURN_STATUS_ERROR 1
#define RETURN_STATUS_FAILURE -1


#define MS_PREORDER   0x01                  /* Macro Space Pre-Search         */
#define MS_POSTORDER  0x02                  /* Macro Space Post-Search        */



                                       /* NOTE:  a template structure for   */
                                       /* the following is created in       */
                                       /* OKACTIVA.C to allow quick         */
                                       /* initialization of new activations.*/
                                       /* That template MUST be updated     */
                                       /* whenever the settings structure   */
                                       /* changes                           */

class ActivationSettings
{
    public:
      inline ActivationSettings() {}

      RexxDirectory * traps;               /* enabled condition traps           */
      RexxDirectory * conditionObj;        /* current condition object          */
      RexxObject   ** parent_arglist;      /* arguments to top level program    */
      size_t          parent_argcount;     /* number of arguments to the top level program */
      RexxMethod    * parent_method;       /* method object for top level       */
      RexxCode      * parent_code;         /* source of the parent method       */
      RexxString    * current_env;         /* current address environment       */
      RexxString    * alternate_env;       /* alternate address environment     */
      RexxString    * msgname;             /* message sent to the receiver      */
                                           /* object variable dictionary        */
      RexxVariableDictionary *object_variables;
      RexxString    * calltype;            /* (COMMAND/METHOD/FUNCTION/ROUTINE) */
      RexxDirectory * streams;             /* Directory of openned streams      */
      RexxString    * halt_description;    /* description from a HALT condition */
      SecurityManager * securityManager;   /* security manager object           */
      RexxObject    * scope;               // scope of the method call
      size_t traceOption;                  /* current active trace option       */
      size_t flags;                        /* trace/numeric and other settings  */
      wholenumber_t trace_skip;            /* count of trace events to skip     */
      int  return_status;                  /* command return status             */
      size_t  traceindent;                 /* trace indentation                 */
      NumericSettings numericSettings;     /* globally effective settings       */
      int64_t elapsed_time;                /* elapsed time clock                */
      RexxDateTime timestamp;              /* current timestamp                 */
      bool intermediate_trace;             /* very quick test for intermediate trace */
      RexxLocalVariables local_variables;  /* the local variables for this activation */
};

                                       /* activation_context values         */
                                       /* these are done as bit settings to */
                                       /* allow multiple potential values   */
                                       /* to be checked with a single test  */
#define DEBUGPAUSE   0x00000001
#define METHODCALL   0x00000002
#define INTERNALCALL 0x00000004
#define INTERPRET    0x00000008
#define PROGRAMCALL  0x00000010
#define EXTERNALCALL 0x00000020

                                       /* check for top level execution     */
#define TOP_LEVEL_CALL (PROGRAMCALL | METHODCALL | EXTERNALCALL)
                                       /* non-method top level execution    */
#define PROGRAM_LEVEL_CALL (PROGRAMCALL | EXTERNALCALL)
                                       /* non-method top level execution    */
#define PROGRAM_OR_METHOD  (PROGRAMCALL | METHODCALL)
                                       /* call is within an activation      */
#define INTERNAL_LEVEL_CALL (INTERNALCALL | INTERPRET)

/* object_scope values */
#define SCOPE_RESERVED  1
#define SCOPE_RELEASED  0

 class RexxActivation : public RexxActivationBase {
  friend class RexxSource;
  public:
   void *operator new(size_t);
   inline void *operator new(size_t size, void *ptr) {return ptr;};
   inline void  operator delete(void *) { ; }
   inline void  operator delete(void *, void *) { ; }

   inline RexxActivation(RESTORETYPE restoreType) { ; };
   RexxActivation();
   RexxActivation(RexxActivity* _activity, RexxMethod *_method, RexxCode *_code);
   RexxActivation(RexxActivity *_activity, RoutineClass *_routine, RexxCode *_code, RexxString *calltype, RexxString *env, int context);
   RexxActivation(RexxActivity *_activity, RexxActivation *_parent, RexxCode *_code, int context);

   void live(size_t);
   void liveGeneral(int reason);
   RexxObject      * dispatch();
   size_t            digits();
   size_t            fuzz();
   bool              form();
   void              setDigits(size_t);
   void              setFuzz(size_t);
   void              setForm(bool);
   void              setDigits();
   void              setFuzz();
   void              setForm();
   bool              trap(RexxString *, RexxDirectory *);
   void              setObjNotify(RexxMessage *);
   void              termination();
   inline void       guardOff()
    {
                                           /* currently locked?                 */
      if (this->object_scope == SCOPE_RESERVED) {
                                           /* release the variable dictionary   */
        this->settings.object_variables->release(this->activity);
                                           /* set the state to released         */
        this->object_scope = SCOPE_RELEASED;
      }
    }


   inline bool isInterpret() { return activation_context == INTERPRET; }
   inline bool isInternalCall() { return activation_context == INTERNALCALL; }
   inline bool isMethod() { return activation_context == METHODCALL; }
   inline bool isRoutine() { return activation_context == EXTERNALCALL; }
   inline bool isProgram() { return activation_context == PROGRAMCALL; }
   inline bool isTopLevelCall() { return (activation_context & TOP_LEVEL_CALL) != 0; }
   inline bool isProgramLevelCall() { return (activation_context & PROGRAM_LEVEL_CALL) != 0; }
   inline bool isInternalLevelCall() { return (activation_context & INTERNAL_LEVEL_CALL) != 0; }
   inline bool isProgramOrMethod() { return (activation_context & PROGRAM_OR_METHOD) != 0; }

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
   static size_t     processTraceSetting(size_t traceSetting);
   void              raise(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *, RexxDirectory *);
   void              toggleAddress();
   void              guardOn();
   void              raiseExit(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *, RexxDirectory *);
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
   RexxObject      * externalCall(RexxString *, size_t, RexxExpressionStack *, RexxString *, ProtectedObject &);
   RexxObject      * internalCall(RexxInstruction *, size_t, RexxExpressionStack *, ProtectedObject &);
   RexxObject      * internalCallTrap(RexxInstruction *, RexxDirectory *, ProtectedObject &);
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
   void              traceCompoundValue(int prefix, RexxString *stemName, RexxObject **tails, size_t tailCount, RexxCompoundTail *tail);
   void              traceCompoundValue(int prefix, RexxString *stem, RexxObject **tails, size_t tailCount, const char *marker, RexxObject * value);
   void              traceTaggedValue(int prefix, const char *tagPrefix, bool quoteTag, RexxString *tag, const char *marker, RexxObject * value);
   void              traceOperatorValue(int prefix, const char *tag, RexxObject *value);
   void              traceSourceString();
   void              traceClause(RexxInstruction *, int);
   void              resetElapsed();
   RexxString      * formatTrace(RexxInstruction *, RexxSource *);
   RexxString      * getTraceBack();
   RexxDirectory   * local();
   inline void       implicitExit()
   {
     /* at a main program level or completing an INTERPRET */
     /* instruction? */
     if (this->activation_context&TOP_LEVEL_CALL || this->activation_context == INTERPRET) {
                                          /* real program call?                */
         if (this->activation_context&PROGRAM_LEVEL_CALL)
         {
                                          /* run termination exit              */
             this->activity->callTerminationExit(this);
         }
         this->execution_state = RETURNED;/* this is an EXIT for real          */
         return;                          /* we're finished here               */
     }
     this->exitFrom(OREF_NULL);           /* we've had a nested exit, we need to process this more fully */
   }

   void              unwindTrap(RexxActivation *);
   RexxString      * sourceString();
   void              addLocalRoutine(RexxString *name, RexxMethod *method);
   RexxDirectory    *getPublicRoutines();
   void              debugInterpret(RexxString *);
   bool              debugPause(RexxInstruction * instr=OREF_NULL);
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
   void              mergeTraps(RexxQueue *, RexxQueue *);
   uint64_t          getRandomSeed(RexxInteger *);
   void              adjustRandomSeed() { random_seed += (uint64_t)(uintptr_t)this; }
   RexxVariableDictionary * getObjectVariables();
   RexxDirectory   * getLabels();
   RexxString      * getProgramName();
   RexxObject      * popControl();
   void              pushControl(RexxObject *);
   void              closeStreams();
   void              checkTrapTable();
   RexxObject       *resolveStream(RexxString *name, bool input, RexxString **fullName, bool *added);
   RexxDirectory    *getStreams();
   RexxObject       *novalueHandler(RexxString *);
   RexxVariableBase *retriever(RexxString *);
   RexxVariableBase *directRetriever(RexxString *);
   RexxObject       *handleNovalueEvent(RexxString *name, RexxVariable *variable);
   RexxSource       *getSourceObject() { return sourceObject; }
   inline RexxSource *getEffectiveSourceObject() {
       return isInterpret() ? executable->getSourceObject() : sourceObject;
   }
   PackageClass     *getPackage();
   RexxObject       *getLocalEnvironment(RexxString *name);
   void              setReturnStatus(int status);

   inline void              setCallType(RexxString *type) {this->settings.calltype = type; }
   inline void              pushBlock(RexxDoBlock *block) { block->setPrevious(this->dostack); this->dostack = block; }
   inline void              popBlock() { RexxDoBlock *temp; temp = this->dostack; this->dostack = temp->getPrevious(); temp->setHasNoReferences(); }
   inline RexxDoBlock     * topBlock() { return this->dostack; }
   inline void              terminateBlock(size_t _indent) { this->popBlock(); this->blockNest--; this->settings.traceindent = _indent; }
   inline void              terminateBlock() { this->settings.traceindent = this->dostack->getIndent(); this->popBlock(); this->blockNest--; }
   inline void              newDo(RexxDoBlock *block) { this->pushBlock(block); this->blockNest++; this->settings.traceindent++;}
   inline void              removeBlock() { this->blockNest--; };
   inline void              addBlock()    { this->blockNest++; };
   inline bool              hasActiveBlocks() { return blockNest != 0; }
   inline bool              inMethod()  {return this->activation_context == METHODCALL; }
   inline void              indent() {this->settings.traceindent++; };
   inline void              unindent() {if (this->settings.traceindent > 0) this->settings.traceindent--; };
   inline void              setIndent(size_t v) {this->settings.traceindent=(v); };
   inline size_t            getIndent() {return this->settings.traceindent;};
   inline bool              tracingIntermediates() {return this->settings.intermediate_trace;};
   inline void              clearTraceSettings() { settings.flags &= ~trace_flags; settings.intermediate_trace = false; }
   inline bool              tracingResults() {return (this->settings.flags&trace_results) != 0; }
   inline RexxActivity    * getActivity() {return this->activity;};
   inline RexxString      * getMessageName() {return this->settings.msgname;};
   inline RexxString      * getCallname() {return this->settings.msgname;};
   inline RexxInstruction * getCurrent() {return this->current;};
   inline void              getSettings(ActivationSettings &s) {this->settings = s;};
   inline void              putSettings(ActivationSettings &s) {s = this->settings;};
   inline RexxString      * getAddress() {return this->settings.current_env;};
   inline RexxDirectory   * getConditionObj() {return this->settings.conditionObj;};
   inline void              setConditionObj(RexxDirectory *condition) {this->settings.conditionObj = condition;};
   inline RexxInstruction * getNext() {return this->next;};
   inline void              setNext(RexxInstruction * v) {this->next=v;};
   inline void              setCurrent(RexxInstruction * v) {this->current=v;};
   inline bool              inDebug() { return ((this->settings.flags&trace_debug) != 0) && !this->debug_pause;}

   inline RexxExpressionStack * getStack() {return &this->stack; };

   virtual NumericSettings *getNumericSettings();
   virtual RexxActivation  *getRexxContext();
   virtual RexxActivation  *findRexxContext();
   virtual RexxObject      *getReceiver();
   virtual bool             isRexxContext();

   inline void              traceIntermediate(RexxObject * v, int p) { if (this->settings.intermediate_trace) this->traceValue(v, p); };
   inline void              traceVariable(RexxString *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceTaggedValue(TRACE_PREFIX_VARIABLE, NULL, false, n, VALUE_MARKER, v); } };
   inline void              traceDotVariable(RexxString *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceTaggedValue(TRACE_PREFIX_DOTVARIABLE, ".", false, n, VALUE_MARKER, v); } };
   inline void              traceFunction(RexxString *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceTaggedValue(TRACE_PREFIX_FUNCTION, NULL, false, n, VALUE_MARKER, v); } };
   inline void              traceMessage(RexxString *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceTaggedValue(TRACE_PREFIX_MESSAGE, NULL, true, n, VALUE_MARKER, v); } };
   inline void              traceOperator(const char *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceOperatorValue(TRACE_PREFIX_OPERATOR, n, v); } };
   inline void              tracePrefix(const char *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceOperatorValue(TRACE_PREFIX_PREFIX, n, v); } };
   inline void              traceAssignment(RexxString *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceTaggedValue(TRACE_PREFIX_ASSIGNMENT, NULL, false, n, ASSIGNMENT_MARKER, v); } };
   inline void              traceCompoundName(RexxString *stemVar, RexxObject **tails, size_t tailCount, RexxCompoundTail *tail) { if (this->settings.intermediate_trace) this->traceCompoundValue(TRACE_PREFIX_COMPOUND, stemVar, tails, tailCount, VALUE_MARKER, tail->createCompoundName(stemVar)); };
   inline void              traceCompoundName(RexxString *stemVar, RexxObject **tails, size_t tailCount, RexxString *tail) { if (this->settings.intermediate_trace) this->traceCompoundValue(TRACE_PREFIX_COMPOUND, stemVar, tails, tailCount, VALUE_MARKER, stemVar->concat(tail)); };
   inline void              traceCompound(RexxString *stemVar, RexxObject **tails, size_t tailCount, RexxObject *value) { if (this->settings.intermediate_trace) this->traceCompoundValue(TRACE_PREFIX_VARIABLE, stemVar, tails, tailCount, VALUE_MARKER, value); };
   inline void              traceCompoundAssignment(RexxString *stemVar, RexxObject **tails, size_t tailCount, RexxObject *value) { if (this->settings.intermediate_trace) this->traceCompoundValue(TRACE_PREFIX_ASSIGNMENT, stemVar, tails, tailCount, ASSIGNMENT_MARKER, value); };
   inline void              traceResult(RexxObject * v) { if ((this->settings.flags&trace_results)) this->traceValue(v, TRACE_PREFIX_RESULT); };
   inline bool              tracingInstructions(void) { return (this->settings.flags&trace_all) != 0; }
   inline bool              tracingErrors(void) { return (this->settings.flags&trace_errors) != 0; }
   inline bool              tracingFailures(void) { return (this->settings.flags&trace_failures) != 0; }
   inline void              traceInstruction(RexxInstruction * v) { if (this->settings.flags&trace_all) this->traceClause(v, TRACE_PREFIX_CLAUSE); }
   inline void              traceLabel(RexxInstruction * v) { if ((this->settings.flags&trace_labels) != 0) this->traceClause(v, TRACE_PREFIX_CLAUSE); };
   inline void              traceCommand(RexxInstruction * v) { if ((this->settings.flags&trace_commands) != 0) this->traceClause(v, TRACE_PREFIX_CLAUSE); }
   inline bool              tracingCommands(void) { return (this->settings.flags&trace_commands) != 0; }
   inline bool              tracingAll(void) { return (this->settings.flags&trace_all) != 0; }
   inline void              pauseInstruction() {  if ((this->settings.flags&(trace_all | trace_debug)) == (trace_all | trace_debug)) this->debugPause(); };
   inline int               conditionalPauseInstruction() { return (((this->settings.flags&(trace_all | trace_debug)) == (trace_all | trace_debug)) ? this->debugPause(): false); };
   inline void              pauseLabel() { if ((this->settings.flags&(trace_labels | trace_debug)) == (trace_labels | trace_debug)) this->debugPause(); };
   inline void              pauseCommand() { if ((this->settings.flags&(trace_commands | trace_debug)) == (trace_commands | trace_debug)) this->debugPause(); };

          SecurityManager  *getSecurityManager();
          SecurityManager  *getEffectiveSecurityManager();
   inline bool              isTopLevel() { return (this->activation_context&TOP_LEVEL_CALL) != 0; }
   inline bool              isForwarded() { return (this->settings.flags&forwarded) != 0; }
   inline bool              isGuarded() { return (this->settings.flags&guarded_method) != 0; }
   inline void              setGuarded() { this->settings.flags |= guarded_method; }

   inline bool              isExternalTraceOn() { return (this->settings.flags&trace_on) != 0; }
   inline void              setExternalTraceOn() { this->settings.flags |= trace_on; }
   inline void              setExternalTraceOff() { this->settings.flags &= ~trace_on; }
          void              enableExternalTrace();

   inline bool              isElapsedTimerReset() { return (this->settings.flags&elapsed_reset) != 0; }
   inline void              setElapsedTimerInvalid() { this->settings.flags |= elapsed_reset; }
   inline void              setElapsedTimerValid() { this->settings.flags &= ~elapsed_reset; }


   inline RexxObject     ** getMethodArgumentList() {return this->arglist;};
   inline size_t            getMethodArgumentCount() { return argcount; }
   inline RexxObject *      getMethodArgument(size_t position) {
       if (position > getMethodArgumentCount()) {
           return OREF_NULL;
       }
       else {
           return arglist[position-1];
       }
   }

   inline RexxArray        *getArguments() { return new_array(argcount, arglist); }

   inline RexxObject      **getProgramArgumentlist() {return this->settings.parent_arglist;};
   inline size_t            getProgramArgumentCount() { return settings.parent_argcount; }

   inline RexxObject *      getProgramArgument(size_t position) {
       if (position > getProgramArgumentCount()) {
           return OREF_NULL;
       }
       else {
           return settings.parent_arglist[position-1];
       }
   }

   RexxObject *getContextObject();
   RexxObject *getContextLine();
   size_t      getContextLineNumber();
   RexxObject *getContextReturnStatus();
   StackFrameClass *createStackFrame();

   inline RexxVariableDictionary *getLocalVariables()
   {
       return settings.local_variables.getDictionary();
   }

   inline RexxDirectory *getAllLocalVariables()
   {
       return getLocalVariables()->getAllVariables();
   }

   inline RexxVariable *getLocalVariable(RexxString *name, size_t index)
   {
       RexxVariable *target = settings.local_variables.get(index);
       if (target == OREF_NULL) {
           target = settings.local_variables.lookupVariable(name, index);
       }
       return target;
   }

   inline RexxVariable *getLocalStemVariable(RexxString *name, size_t index)
   {
       RexxVariable *target = settings.local_variables.get(index);
       if (target == OREF_NULL) {
           target = settings.local_variables.lookupStemVariable(name, index);
       }
       return target;
   }

   inline RexxStem *getLocalStem(RexxString *name, size_t index)
   {
       return (RexxStem *)getLocalStemVariable(name, index)->getVariableValue();
   }

   inline void dropLocalStem(RexxString *name, size_t index)
   {
       RexxVariable *stemVar = getLocalStemVariable(name, index);
       /* create a new stem element and set this */
       stemVar->set(new RexxStem(name));
   }

   inline bool localStemVariableExists(RexxString *stemName, size_t index)
   {
     /* get the stem entry from this dictionary */
     RexxVariable *variable = settings.local_variables.find(stemName, index);
     /* The stem exists if the stem variable has ever been used. */
     return variable != OREF_NULL;
   }

   inline bool localVariableExists(RexxString *name, size_t index)
   {
     /* get the stem entry from this dictionary */
     RexxVariable *variable = settings.local_variables.find(name, index);
     /* The stem exists if the stem variable has ever been used. */
     return variable != OREF_NULL && variable->getVariableValue() != OREF_NULL;
   }

   inline void putLocalVariable(RexxVariable *variable, size_t index)
   {
       settings.local_variables.putVariable(variable, index);
   }

   inline void updateLocalVariable(RexxVariable *variable)
   {
       settings.local_variables.updateVariable(variable);
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
   RexxCompoundElement *getLocalCompoundVariable(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount);
   RexxCompoundElement *exposeLocalCompoundVariable(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount);
   bool localCompoundVariableExists(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount);
   void assignLocalCompoundVariable(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount, RexxObject *value);
   void setLocalCompoundVariable(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount, RexxObject *value);
   void dropLocalCompoundVariable(RexxString *stemName, size_t index, RexxObject **tail, size_t tailCount);

   inline bool novalueEnabled() { return settings.local_variables.getNovalue(); }

   /* The following methods be rights should be implemented by the */
   /* RexxMemory class, but aren't because of the difficulties of */
   /* making them inline methods that use the RexxVariable class. */
   /* Therefore, we're going to break the encapsulation rules */
   /* slightly and allow the activation class to manipulate that */
   /* chain directly. */
   inline RexxVariable *newLocalVariable(RexxString *name)
   {
       RexxVariable *newVariable = memoryObject.variableCache;
       if (newVariable != OREF_NULL) {
           memoryObject.variableCache = newVariable->getNext();
           newVariable->reset(name);
       }
       else {
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
       /* if we're nested, we need to make sure that any variable */
       /* dictionary created at this level is propagated back to */
       /* the caller. */
       if (isInternalLevelCall() && settings.local_variables.isNested())
       {
           parent->setLocalVariableDictionary(settings.local_variables.getNestedDictionary());
       }
       else
       {
           // we need to cleanup the local variables and return them to the
           // cache.
           for (size_t i = 0; i < settings.local_variables.size; i++)
           {
               RexxVariable *var = settings.local_variables.get(i);
               if (var != OREF_NULL && var->isLocal(this))
               {
                   cacheLocalVariable(var);
               }
           }
       }
   }

   inline void setLocalVariableDictionary(RexxVariableDictionary *dict) {settings.local_variables.setDictionary(dict); }

 protected:

   ActivationSettings   settings;      /* inherited REXX settings           */
   RexxExpressionStack  stack;         /* current evaluation stack          */
   RexxCode            *code;          /* rexx method object                */
   RexxSource         *sourceObject;   // the source object associated with this instance
   RexxClass           *scope;         // scope of any active method call
   RexxObject          *receiver;      /* target of a message invocation    */
   RexxActivity        *activity;      /* current running activation        */
   RexxActivation      *parent;        // previous running activation for internal call/interpret
   RexxObject         **arglist;       /* activity argument list            */
   size_t               argcount;      /* the count of arguments            */
   RexxDoBlock         *dostack;       /* stack of DO loops                 */
   RexxInstruction     *current;       /* current execution pointer         */
   RexxInstruction     *next;          /* next instruction to execute       */
   bool                 debug_pause;   /* executing a debug pause           */
   int                  object_scope;  /* reserve/release state of variables*/
   RexxObject          *result;        /* result of execution               */
   RexxArray           *trapinfo;      /* current trap handler              */
   RexxContext         *contextObject; // the context object representing the execution context
                                       /* current activation state          */
   int                  execution_state;
                                       /* type of activation activity       */
   int                  activation_context;
   RexxMessage         *objnotify;     /* an object to notify if excep occur*/
                                       /* LIst of Saved Local environments  */
   RexxList            *environmentList;
   size_t               pending_count; /* number of pending conditions      */
   RexxQueue           *handler_queue; /* queue of trapped condition handler*/
                                       /* queue of trapped conditions       */
   RexxQueue           *condition_queue;
   uint64_t             random_seed;   /* random number seed                */
   bool                 random_set;    /* random seed has been set          */
   size_t               blockNest;     /* block instruction nesting level   */
   size_t               lookaside_size;/* size of the lookaside table       */


   // constants

   static const size_t trace_off;           /* no tracing                        */
   static const size_t trace_debug;         /* interactive trace mode flag       */
   static const size_t trace_all;           /* trace all instructions            */
   static const size_t trace_results;       /* trace all results                 */
   static const size_t trace_intermediates; /* trace all instructions            */
   static const size_t trace_commands;      /* trace all commands                */
   static const size_t trace_labels;        /* trace all labels                  */
   static const size_t trace_errors;        /* trace all command errors          */
   static const size_t trace_failures;      /* trace all command failures        */
   static const size_t trace_suppress;      /* tracing is suppressed during skips*/
   static const size_t trace_flags;         /* all tracing flags (EXCEPT debug)  */
   static const size_t default_trace_flags;
   static const size_t trace_all_flags;     // flag set for trace all
   static const size_t trace_results_flags; // flag set for trace results
   static const size_t trace_intermediates_flags; // flag set for trace intermediates

   static const size_t single_step;         /* we are single stepping execution  */
   static const size_t single_step_nested;  /* this is a nested stepping         */
   static const size_t debug_prompt_issued; /* debug prompt already issued       */
   static const size_t debug_bypass;        /* skip next debug pause             */
   static const size_t procedure_valid;     /* procedure instruction is valid    */
   static const size_t clause_boundary;     /* work required at clause boundary  */
   static const size_t halt_condition;      /* a HALT condition occurred         */
   static const size_t trace_on;            /* external trace condition occurred */
   static const size_t source_traced;       /* source string has been traced     */
   static const size_t clause_exits;        /* need to call clause boundary exits*/
   static const size_t external_yield;      /* activity wants us to yield        */
   static const size_t forwarded;           /* forward instruction active        */
   static const size_t reply_issued;        /* reply has already been issued     */
   static const size_t set_trace_on;        /* trace turned on externally        */
   static const size_t set_trace_off;       /* trace turned off externally       */
   static const size_t traps_copied;        /* copy of trap info has been made   */
   static const size_t return_status_set;   /* had our first host command        */
   static const size_t transfer_failed;     /* transfer of variable lock failure */

   static const size_t elapsed_reset;       // The elapsed time stamp was reset via time('r')
   static const size_t guarded_method;      // this is a guarded method
 };
 #endif
