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
/* REXX Kernel                                                  RexxActivation.hpp  */
/*                                                                            */
/* Primitive Activation Class Definitions                                     */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxActivation
#define Included_RexxActivation

#include "ExpressionStack.hpp"                /* needs expression stack            */
#include "DoBlock.hpp"                /* need do block definition          */
                                       /* various activation settings       */
#include "RexxLocalVariables.hpp"        /* local variable cache definitions  */
#include "RexxDateTime.hpp"

#define trace_debug         0x00000001 /* interactive trace mode flag       */
#define trace_all           0x00000002 /* trace all instructions            */
#define trace_results       0x00000004 /* trace all results                 */
#define trace_intermediates 0x00000008 /* trace all instructions            */
#define trace_commands      0x00000010 /* trace all commands                */
#define trace_labels        0x00000020 /* trace all labels                  */
#define trace_errors        0x00000040 /* trace all command errors          */
#define trace_failures      0x00000080 /* trace all command failures        */
#define trace_suppress      0x00000100 /* tracing is suppressed during skips*/
#define trace_flags         0x000001fe /* all tracing flags (EXCEPT debug)  */

#define single_step         0x00000800 /* we are single stepping execution  */
#define single_step_nested  0x00001000 /* this is a nested stepping         */
#define debug_prompt_issued 0x00002000 /* debug prompt already issued       */
#define debug_bypass        0x00004000 /* skip next debug pause             */
#define procedure_valid     0x00008000 /* procedure instruction is valid    */
#define clause_boundary     0x00010000 /* work required at clause boundary  */
#define halt_condition      0x00020000 /* a HALT condition occurred         */
#define trace_on            0x00040000 /* external trace condition occurred */
#define source_traced       0x00080000 /* source string has been traced     */
#define clause_exits        0x00100000 /* need to call clause boundary exits*/
#define external_yield      0x00200000 /* activity wants us to yield        */
#define forwarded           0x00400000 /* forward instruction active        */
#define reply_issued        0x00800000 /* reply has already been issued     */
#define set_trace_on        0x01000000 /* trace turned on externally        */
#define set_trace_off       0x02000000 /* trace turned off externally       */
#define traps_copied        0x04000000 /* copy of trap info has been made   */
#define return_status_set   0x08000000 /* had our first host command        */
#define transfer_failed     0x10000000 /* transfer of variable lock failure */

#define dbg_trace           0x00000001
#define dbg_stepover        0x00000002
#define dbg_stepout         0x00000004
#define dbg_endstep         0x00000010
#define dbg_stepagain       0x00000020

/* execution_state values */
#define ACTIVE    0
#define REPLIED   1
#define RETURNED  2



                                       /* NOTE:  a template structure for   */
                                       /* the following is created in       */
                                       /* OKACTIVA.C to allow quick         */
                                       /* initialization of new activations.*/
                                       /* That template MUST be updated     */
                                       /* whenever the settings structure   */
                                       /* changes                           */

class ACTSETTINGS {
    public:
      inline ACTSETTINGS()
      {
          flags = trace_failures;
          traceoption = TRACE_NORMAL;
      }

      RexxDirectory * traps;               /* enabled condition traps           */
      RexxDirectory * conditionObj;        /* current condition object          */
      RexxObject   ** parent_arglist;      /* arguments to top level program    */
      size_t          parent_argcount;     /* number of arguments to the top level program */
      RexxMethod    * parent_method;       /* method object for top level       */
      RexxSource    * parent_source;       /* source of the parent method       */
      RexxString    * current_env;         /* current address environment       */
      RexxString    * alternate_env;       /* alternate address environment     */
      RexxString    * msgname;             /* message sent to the receiver      */
                                           /* object variable dictionary        */
      RexxVariableDictionary *object_variables;
      RexxString    * calltype;            /* (COMMAND/METHOD/FUNCTION/ROUTINE) */
      RexxDirectory * streams;             /* Directory of openned streams      */
      RexxString    * halt_description;    /* description from a HALT condition */
      RexxObject    * securityManager;     /* security manager object           */
      INT  traceoption;                    /* current active trace option       */
      ULONG flags;                         /* trace/numeric and other settings  */
      ULONG dbg_flags;                     /* debug exit settings               */
      LONG trace_skip;                     /* count of trace events to skip     */
      LONG return_status;                  /* command return status             */
      LONG traceindent;                    /* trace indentation                 */
      ACTIVATION_SETTINGS global_settings; /* globally effective settings       */
      int64_t elapsed_time;                /* elapsed time clock                */
      RexxDateTime timestamp;              /* current timestamp                 */
      BOOL intermediate_trace;             /* very quick test for intermediate trace */
      RexxLocalVariables local_variables;  /* the local variables for this activation */
};
typedef ACTSETTINGS *PSETT;

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

RexxObject *buildCompoundVariable(RexxString * variable_name, BOOL direct);

RexxObject * activation_find  (void);

 class RexxActivation : public RexxActivationBase {
  public:
   void *operator new(size_t);
   inline void *operator new(size_t size, void *ptr) {return ptr;};
   inline RexxActivation(RESTORETYPE restoreType) { ; };
   RexxActivation(RexxObject *, RexxMethod *, RexxActivity *, RexxString *, RexxActivation *, int);
   void init(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, int);
   void live();
   void liveGeneral();
   void flatten(RexxEnvelope *);
   RexxObject      * dispatch();
   void              traceBack(RexxList *);
   long              digits();
   long              fuzz();
   BOOL              form();
   void              setDigits(LONG);
   void              setFuzz(LONG);
   void              setForm(BOOL);
   BOOL              trap(RexxString *, RexxDirectory *);
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
   RexxObject      * run(RexxObject **, size_t, RexxInstruction *);
   void              reply(RexxObject *);
   RexxObject      * forward(RexxObject *, RexxString *, RexxObject *, RexxObject **, size_t, BOOL);
   void              returnFrom(RexxObject *result);
   void              exitFrom(RexxObject *);
   void              procedureExpose(RexxVariableBase **variables, size_t count);
   void              expose(RexxVariableBase **variables, size_t count);
   RexxVariableBase *getVariableRetriever(RexxString  *variable);
   RexxVariableBase *getDirectVariableRetriever(RexxString  *variable);
   void              setTrace(int, int);
   void              raise(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *, RexxDirectory *);
   void              toggleAddress();
   void              guardOn();
   void              raiseExit(RexxString *, RexxObject *, RexxString *, RexxObject *, RexxObject *, RexxDirectory *);
   RexxActivation  * senderAct();
   RexxActivation  * external();
   void              interpret(RexxString *);
   void              signalTo(RexxInstruction *);
#ifdef NEWGUARD
   BOOL              guardWait();
   void              guardWaitScope(BOOL initial_state);
#else
   void              guardWait();
#endif
   void              debugSkip(LONG, BOOL);
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
   BOOL              callExternalRexx(RexxString *, RexxString *, RexxObject **, size_t, RexxString *, RexxObject **);
   RexxObject      * externalCall(RexxString *, size_t, RexxExpressionStack *, RexxString *);
   RexxObject      * internalCall(RexxInstruction *, size_t, RexxExpressionStack *);
   RexxObject      * internalCallTrap(RexxInstruction *, RexxDirectory *);
   RexxObject      * command(RexxString *, RexxString *);
   int64_t           getElapsed();
   RexxDateTime      getTime();
   RexxInteger     * random(RexxInteger *, RexxInteger *, RexxInteger *);
   LONG              currentLine();
   void              arguments(RexxObject *);
   void              traceValue(RexxObject *, int);
   void              traceCompoundValue(int prefix, RexxString *stem, RexxObject **tails, size_t tailCount, RexxObject * value);
   void              traceTaggedValue(int prefix, stringchar_t *tagPrefix, bool quoteTag, RexxString *tag, RexxObject * value);
   void              traceOperatorValue(int prefix, const char *tag, RexxObject *value);
   void              traceSourceString();
   void              traceClause(RexxInstruction *, int);
   void              resetElapsed();
   RexxString      * formatTrace(RexxInstruction *, RexxSource *);
   RexxDirectory   * local();
   inline void       implicitExit()
   {
     /* at a main program level or completing an INTERPRET */
     /* instruction? */
     if (this->activation_context&TOP_LEVEL_CALL || this->activation_context == INTERPRET) {
                                          /* real program call?                */
         if (this->activation_context&PROGRAM_LEVEL_CALL) {
                                          /* run termination exit              */
             this->activity->sysExitTerm(this);
         }
         this->execution_state = RETURNED;/* this is an EXIT for real          */
         return;                          /* we're finished here               */
     }
     this->exitFrom(OREF_NULL);           /* we've had a nested exit, we need to process this more fully */
   }

   void              unwindTrap(RexxActivation *);
   RexxString      * sourceString();
   void              debugInterpret(RexxString *);
   BOOL              debugPause(RexxInstruction * instr=OREF_NULL);
   void              processClauseBoundary();
   void              halt(RexxString *);
   void              externalTraceOn();
   void              externalTraceOff();
   void              externalDbgStepIn();
   void              externalDbgStepOver();
   void              externalDbgStepOut();
   void              externalDbgEndStepO();
   void              externalDbgStepAgain();
   void              sysDbgSubroutineCall(BOOL);
   void              sysDbgLineLocate(RexxInstruction * instr = OREF_NULL);
   void              sysDbgSignal();
   void              yield();
   void              propagateExit(RexxObject *);
   void              setDefaultAddress(RexxString *);
   BOOL              internalMethod();
   RexxObject      * loadRequired(RexxString *, RexxInstruction *);
   void              setDBCS(BOOL);
   RexxObject      * rexxVariable(RexxString *);
   void              pushEnvironment(RexxObject *);
   RexxObject      * popEnvironment();
   void              processTraps();
   void              mergeTraps(RexxQueue *, RexxQueue *);
   ULONG             getRandomSeed(RexxInteger *);
   RexxVariableDictionary * getObjectVariables();
   RexxDirectory   * getLabels();
   RexxString      * programName();
   RexxObject      * popControl();
   void              pushControl(RexxObject *);
   void              closeStreams();
   void              checkTrapTable();
   RexxDirectory   * getStreams();
   BOOL              callSecurityManager(RexxString *, RexxDirectory *);
   RexxObject  *novalueHandler(RexxString *);
   RexxVariableBase *retriever(RexxString *);
   RexxVariableBase *directRetriever(RexxString *);

   inline RexxActivationBase  * getSender() { return (RexxActivationBase *)this->sender; }
   inline void              setCallType(RexxString *type) {this->settings.calltype = type; }
   inline RexxObject      * getReceiver() { return this->receiver; };
   inline void              pushBlock(RexxDoBlock *block) { block->setPrevious(this->dostack); this->dostack = block; }
   inline void              popBlock() { RexxDoBlock *temp; temp = this->dostack; this->dostack = temp->previous; SetObjectHasNoReferences(temp); }
   inline RexxDoBlock     * topBlock() { return this->dostack; }
   inline void              terminateBlock(LONG indent) { this->popBlock(); this->blockNest--; this->settings.traceindent = indent; }
   inline void              newDo(RexxDoBlock *block) { this->pushBlock(block); this->blockNest++; this->settings.traceindent++;}
   inline void              removeBlock() { this->blockNest--; };
   inline void              addBlock()    { this->blockNest++; };
   inline BOOL              inMethod()  {return this->activation_context == METHODCALL; }
   inline RexxSource *      getSource() {return this->settings.parent_source; };
   inline void              indent() {this->settings.traceindent++; };
   inline void              unindent() {this->settings.traceindent--; };
   inline void              setIndent(long v) {this->settings.traceindent=(v); };
   inline BOOL              tracingIntermediates() {return this->settings.intermediate_trace;};
   inline void              clearTraceSettings() { settings.flags &= ~trace_flags; settings.intermediate_trace = FALSE; }
   inline BOOL              tracingResults() {return this->settings.flags&trace_results || this->settings.dbg_flags&dbg_trace;};
   inline RexxActivity    * getActivity() {return this->activity;};
   inline RexxMethod      * getMethod() {return this->method;};
   inline RexxString      * getMsgname() {return this->settings.msgname;};
   inline RexxString      * getCallname() {return this->settings.msgname;};
   inline RexxInstruction * getCurrent() {return this->current;};
   inline void              getSettings(ACTSETTINGS * s) {this->settings = *s;};
   inline void              putSettings(ACTSETTINGS * s) {*s = this->settings;};
   inline RexxString      * getAddress() {return this->settings.current_env;};
   inline RexxDirectory   * getConditionObj() {return this->settings.conditionObj;};
   inline void              setConditionObj(RexxDirectory *condition) {this->settings.conditionObj = condition;};
   inline RexxInstruction * getNext() {return this->next;};
   inline void              setNext(RexxInstruction * v) {this->next=v;};
   inline void              setCurrent(RexxInstruction * v) {this->current=v;};
   inline BOOL              inDebug() {return ((this->settings.flags&trace_debug || this->settings.dbg_flags&dbg_trace) && ! this->debug_pause);};
   inline RexxExpressionStack * getStack() {return &this->stack; };
   inline ACTIVATION_SETTINGS * getGlobalSettings() {return &(this->settings.global_settings);};
   inline LONG              getIndent() {return this->settings.traceindent;};
   inline void              traceIntermediate(RexxObject * v, int p) { if (this->settings.intermediate_trace) this->traceValue(v, p); };
   inline void              traceVariable(RexxString *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceTaggedValue(TRACE_PREFIX_VARIABLE, NULL, false, n, v); } };
   inline void              traceDotVariable(RexxString *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceTaggedValue(TRACE_PREFIX_DOTVARIABLE, (stringchar_t *)".", false, n, v); } };
   inline void              traceFunction(RexxString *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceTaggedValue(TRACE_PREFIX_FUNCTION, NULL, false, n, v); } };
   inline void              traceMessage(RexxString *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceTaggedValue(TRACE_PREFIX_MESSAGE, NULL, true, n, v); } };
   inline void              traceOperator(const char *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceOperatorValue(TRACE_PREFIX_OPERATOR, n, v); } };
   inline void              tracePrefix(const char *n, RexxObject *v)
       { if (this->settings.intermediate_trace) { this->traceOperatorValue(TRACE_PREFIX_PREFIX, n, v); } };
   inline void              traceCompoundName(RexxString *stem, RexxObject **tails, size_t tailCount, RexxCompoundTail *tail) { if (this->settings.intermediate_trace) this->traceCompoundValue(TRACE_PREFIX_COMPOUND, stem, tails, tailCount, tail->createCompoundName(stem)); };
   inline void              traceCompoundName(RexxString *stem, RexxObject **tails, size_t tailCount, RexxString *tail) { if (this->settings.intermediate_trace) this->traceCompoundValue(TRACE_PREFIX_COMPOUND, stem, tails, tailCount, stem->concat(tail)); };
   inline void              traceCompound(RexxString *stem, RexxObject **tails, size_t tailCount, RexxObject *value) { if (this->settings.intermediate_trace) this->traceCompoundValue(TRACE_PREFIX_VARIABLE, stem, tails, tailCount, value); };
   inline void              traceResult(RexxObject * v) { if ((this->settings.flags&trace_results) || (this->settings.dbg_flags&dbg_trace)) this->traceValue(v, TRACE_PREFIX_RESULT); };
   inline BOOL              tracingInstructions(void) { return this->settings.flags&trace_all || this->settings.dbg_flags&dbg_trace;};
   inline void              traceInstruction(RexxInstruction * v) { if (this->settings.flags&trace_all)
                                this->traceClause(v, TRACE_PREFIX_CLAUSE); else if (this->settings.dbg_flags&dbg_trace) this->dbgClause(v, TRACE_PREFIX_CLAUSE);};
   inline void              traceLabel(RexxInstruction * v) { if ((this->settings.flags&trace_labels) || (this->settings.dbg_flags&dbg_trace)) this->traceClause(v, TRACE_PREFIX_CLAUSE); };
   inline void              traceCommand(RexxInstruction * v) { if ((this->settings.flags&trace_commands) ) this->traceClause(v, TRACE_PREFIX_CLAUSE);
                                                                else if (this->settings.dbg_flags&dbg_trace) this->dbgClause(v, TRACE_PREFIX_CLAUSE); };
   inline void              pauseInstruction() {  if ((this->settings.flags&(trace_all | trace_debug)) == (trace_all | trace_debug)) this->debugPause(); };
   inline int               conditionalPauseInstruction() { return (((this->settings.flags&(trace_all | trace_debug)) == (trace_all | trace_debug)) ? this->debugPause(): FALSE); };
   inline void              pauseLabel() { if ((this->settings.flags&(trace_labels | trace_debug)) == (trace_labels | trace_debug)) this->debugPause(); };
   inline void              pauseCommand() { if ((this->settings.flags&(trace_commands | trace_debug)) == (trace_commands | trace_debug)) this->debugPause(); };
   inline void              dbgClause(RexxInstruction * v, int t) {this->traceClause(v, t); if (!this->debug_pause && !(this->settings.flags&halt_condition))
                                                                   this->debugPause(v);
                                                                   if (this->settings.flags&clause_boundary) this->processClauseBoundary();};
   inline void              dbgEnterSubroutine() {  if ((this != (RexxActivation *) TheNilObject) && this->activity->nestedInfo.exitset)
                                                        this->sysDbgSubroutineCall(TRUE);};
   inline void              dbgLeaveSubroutine() {  if ((this != (RexxActivation *) TheNilObject) && this->activity->nestedInfo.exitset)
                                                        this->sysDbgSubroutineCall(FALSE);};
   inline void              externalDbgTraceOff() { this->settings.dbg_flags&=~(dbg_trace | dbg_stepagain);};
                                                  /* switch debug flags off (upward recursively if necessary) */
   inline void              externalDbgAllOffRecursive() { this->settings.dbg_flags&=~(dbg_trace | dbg_stepover | dbg_stepout | dbg_endstep | dbg_stepagain);
                                                  if (this->sender && (this->sender != (RexxActivation *) TheNilObject) && this->sender->settings.dbg_flags) this->sender->externalDbgAllOffRecursive(); };
   inline BOOL              externalDbgEnabled() {return (this->settings.flags&clause_boundary && this->activity->nestedInfo.exitset && !this->debug_pause && !(this->settings.flags&halt_condition));};
   inline void              callDbgExit() { if (this->externalDbgEnabled())
                                                this->activity->sysExitDbgTst(this, this->settings.dbg_flags&dbg_trace, this->settings.dbg_flags&dbg_endstep);};
   inline void              callDbgLineLocate(RexxInstruction * v) { if (this->externalDbgEnabled()) this->sysDbgLineLocate(v);};
   inline void              dbgDisableStepOver() {this->settings.dbg_flags&=~dbg_stepover;};
   inline void              dbgPrepareMethod(RexxActivation * p) {if ( p && (p != (RexxActivation *) TheNilObject) && (p->activity->nestedInfo.exitset))
                                                                      {this->settings.dbg_flags = p->settings.dbg_flags&~dbg_stepover;this->debug_pause = p->debug_pause;}};
   inline void              dbgPassTrace2Parent(RexxActivation * p) {if ( p && (p != (RexxActivation *) TheNilObject) && (p->activity->nestedInfo.exitset)
                                                        && (this->settings.dbg_flags&dbg_trace || this->settings.dbg_flags&dbg_stepover))
                                                            p->externalDbgStepIn();};
   inline void              dbgCheckEndStepOver() {if (this->externalDbgEnabled() && this->settings.dbg_flags&dbg_stepover)
                                                           this->externalDbgEndStepO();};

   inline void              dbgCheckEndStepOut() {if (this->externalDbgEnabled() && this->settings.dbg_flags&dbg_stepout && this->sender
                                                       && (this->sender != (RexxActivation *) TheNilObject) && !(this->sender->settings.dbg_flags&dbg_stepout))
                                                           this->sender->externalDbgEndStepO();};
   inline void              dbgSignal() {  if ((this != (RexxActivation *) TheNilObject) && this->activity->nestedInfo.exitset)
                                                        this->sysDbgSignal();};

   inline BOOL              hasSecurityManager() { return this->settings.securityManager != OREF_NULL; }
   inline BOOL              isTopLevel() { return this->activation_context&TOP_LEVEL_CALL; }
   inline BOOL              isForwarded() { return this->settings.flags&forwarded; }
   inline BOOL              isExternalTraceOn() { return this->settings.flags&trace_on; }
   inline void              setExternalTraceOn() { this->settings.flags |= trace_on; }
   inline void              setExternalTraceOff() { this->settings.flags &= ~trace_on; }

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

   inline RexxVariableDictionary *getLocalVariables()
   {
       return settings.local_variables.getDictionary();
   }

   inline RexxArray *getAllLocalVariables()
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
       RexxVariable *stem = getLocalStemVariable(name, index);
       /* create a new stem element and set this */
       stem->set(new RexxStem(name));
   }

   inline BOOL localStemVariableExists(RexxString *stem, size_t index)
   {
     /* get the stem entry from this dictionary */
     RexxVariable *variable = settings.local_variables.find(stem, index);
     /* The stem exists if the stem variable has ever been used. */
     return variable != OREF_NULL;
   }

   inline BOOL localVariableExists(RexxString *name, size_t index)
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

   inline RexxObject *evaluateLocalCompoundVariable(RexxString *stem, size_t index, RexxObject **tail, size_t tailCount)
   {
     RexxStem     *stem_table;            /* retrieved stem table              */
                                          /* new tail for compound             */
     RexxCompoundTail resolved_tail(this, tail, tailCount);

     stem_table = getLocalStem(stem, index);   /* get the stem entry from this dictionary */
     RexxObject *value = stem_table->evaluateCompoundVariableValue(this, &resolved_tail);
                                          /* need to trace?                    */
     if (tracingIntermediates()) {
       traceCompoundName(stem, tail, tailCount, &resolved_tail);
                                          /* trace variable value              */
       traceCompound(stem, tail, tailCount, value);
     }
     return value;
   }


   inline RexxObject *getLocalCompoundVariableValue(RexxString *stem, size_t index, RexxObject **tail, size_t tailCount)
   {
     RexxStem     *stem_table;            /* retrieved stem table              */
                                          /* new tail for compound             */
     RexxCompoundTail resolved_tail(this, tail, tailCount);

     stem_table = getLocalStem(stem, index);   /* get the stem entry from this dictionary */
     return stem_table->getCompoundVariableValue(&resolved_tail);
   }

   inline RexxCompoundElement *getLocalCompoundVariable(RexxString *stem, size_t index, RexxObject **tail, size_t tailCount)
   {
     RexxStem     *stem_table;            /* retrieved stem table              */
                                          /* new tail for compound             */
     RexxCompoundTail resolved_tail(this, tail, tailCount);

     stem_table = getLocalStem(stem, index);   /* get the stem entry from this dictionary */
     return stem_table->getCompoundVariable(&resolved_tail);
   }

   inline RexxCompoundElement *exposeLocalCompoundVariable(RexxString *stem, size_t index, RexxObject **tail, size_t tailCount)
   {
     RexxStem     *stem_table;            /* retrieved stem table              */
                                          /* new tail for compound             */
     RexxCompoundTail resolved_tail(this, tail, tailCount);

     stem_table = getLocalStem(stem, index);   /* get the stem entry from this dictionary */
     return stem_table->exposeCompoundVariable(&resolved_tail);
   }

   inline BOOL localCompoundVariableExists(RexxString *stem, size_t index, RexxObject **tail, size_t tailCount)
   {
     RexxStem     *stem_table;            /* retrieved stem table              */
                                          /* new tail for compound             */
     RexxCompoundTail resolved_tail(this, tail, tailCount);

     stem_table = getLocalStem(stem, index);   /* get the stem entry from this dictionary */
     return stem_table->compoundVariableExists(&resolved_tail);
   }

   inline void assignLocalCompoundVariable(RexxString *stem, size_t index, RexxObject **tail, size_t tailCount, RexxObject *value)
   {
     RexxStem     *stem_table;                 /* retrieved stem table              */
                                               /* new tail for compound             */
     RexxCompoundTail resolved_tail(this, tail, tailCount);

     stem_table = getLocalStem(stem, index);   /* get the stem entry from this dictionary */
                                               /* and set the value                 */
     stem_table->setCompoundVariable(&resolved_tail, value);
     /* trace resolved compound name */
     traceCompoundName(stem, tail, tailCount, &resolved_tail);
   }

   inline void setLocalCompoundVariable(RexxString *stem, size_t index, RexxObject **tail, size_t tailCount, RexxObject *value)
   {
     RexxStem     *stem_table;                 /* retrieved stem table              */
                                               /* new tail for compound             */
     RexxCompoundTail resolved_tail(this, tail, tailCount);

     stem_table = getLocalStem(stem, index);   /* get the stem entry from this dictionary */
                                               /* and set the value                 */
     stem_table->setCompoundVariable(&resolved_tail, value);
   }

   inline void dropLocalCompoundVariable(RexxString *stem, size_t index, RexxObject **tail, size_t tailCount)
   {
     RexxStem     *stem_table;                 /* retrieved stem table              */
                                               /* new tail for compound             */
     RexxCompoundTail resolved_tail(this, tail, tailCount);

     stem_table = getLocalStem(stem, index);   /* get the stem entry from this dictionary */
                                               /* and set the value                 */
     stem_table->dropCompoundVariable(&resolved_tail);
   }

   inline BOOL novalueEnabled() { return settings.local_variables.getNovalue(); }

   inline RexxObject *handleNoValueEvent(RexxString *name)
   {
                                  /* try for an externally supplied    */
                                  /* value                             */
     RexxObject *value = novalueHandler(name);
     if (value == TheNilObject) { /* no external value?                */
         /* novalue trapping enabled?         */
         if (novalueEnabled()) {
                                  /* handle novalue conditions         */
             report_novalue(name);
         }
         value = name;            /* just use the name                 */
     }
     return value;                /* return the default value          */
   }

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
           newVariable = memoryObject.newVariable(name);
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
       if (!settings.local_variables.isNested()) {
           size_t i;
           for (i = 0; i < settings.local_variables.size; i++) {
               RexxVariable *var = settings.local_variables.get(i);
               if (var != OREF_NULL && var->isLocal(this)) {
                   cacheLocalVariable(var);
               }
           }
       }
       /* if we're nested, we need to make sure that any variable */
       /* dictionary created at this level is propagated back to */
       /* the caller. */
       else {
           sender->setLocalVariableDictionary(settings.local_variables.getNestedDictionary());
       }
   }

   inline void setLocalVariableDictionary(RexxVariableDictionary *dict) {settings.local_variables.setDictionary(dict); }

   ACTSETTINGS          settings;      /* inherited REXX settings           */
   RexxExpressionStack  stack;         /* current evaluation stack          */
   RexxMethod          *method;        /* executed method                   */
   RexxCode            *code;          /* rexx method object                */
   RexxSource          *source;        /* rexx method object                */
   RexxObject          *receiver;      /* target of a message invocation    */
   RexxActivity        *activity;      /* current running activation        */
   RexxActivation      *sender;        /* previous running activation       */
   RexxObject         **arglist;       /* activity argument list            */
   size_t               argcount;      /* the count of arguments            */
   RexxDoBlock         *dostack;       /* stack of DO loops                 */
   RexxInstruction     *current;       /* current execution pointer         */
   RexxInstruction     *next;          /* next instruction to execute       */
   BOOL                 debug_pause;   /* executing a debug pause           */
   INT                  object_scope;  /* reserve/release state of variables*/
   RexxObject          *result;        /* result of execution               */
   RexxArray           *trapinfo;      /* current trap handler              */
   jmp_buf              conditionjump; /* condition trap recovery location  */
                                       /* current activation state          */
   INT                  execution_state;
                                       /* type of activation activity       */
   INT                  activation_context;
   RexxMessage         *objnotify;     /* an object to notify if excep occur*/
                                       /* LIst of Saved Local environments  */
   RexxList            *environmentList;
   INT                  pending_count; /* number of pending conditions      */
   RexxQueue           *handler_queue; /* queue of trapped condition handler*/
                                       /* queue of trapped conditions       */
   RexxQueue           *condition_queue;
   ULONG                random_seed;   /* random number seed                */
   BOOL                 random_set;    /* random seed has been set          */
   ULONG                blockNest;     /* block instruction nesting level   */
   size_t               lookaside_size;/* size of the lookaside table       */
 };
 #endif
