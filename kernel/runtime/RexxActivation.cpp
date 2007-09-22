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
/* REXX Kernel                                                  RexxActivation.c    */
/*                                                                            */
/* Primitive Activation Class                                                 */
/*                                                                            */
/* NOTE:  activations are an execution time only object.  They are never      */
/*        flattened or saved in the image, and hence will never be in old     */
/*        space.  Because of this, activations "cheat" and do not use         */
/*        OrefSet to assign values to get better performance.  Care must be   */
/*        used to maintain this situation.                                    */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
#include <ctype.h>
#include <setjmp.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "RexxBuffer.hpp"
#include "DirectoryClass.hpp"
#include "RexxVariableDictionary.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "MethodClass.hpp"
#include "MessageClass.hpp"
#include "RexxCode.hpp"
#include "SourceFile.hpp"
#include "RexxInstruction.hpp"
#include "CallInstruction.hpp"
#include "DoBlock.hpp"
#include "DoInstruction.hpp"
#include "ExpressionBaseVariable.hpp"
#include "ExpressionDotVariable.hpp"
#include "ExpressionVariable.hpp"
#include "ExpressionStem.hpp"
#include "ExpressionCompoundVariable.hpp"

#define INCL_RXSYSEXIT
#define INCL_RXMACRO   /* we now need macrospace definitions */
#define INCL_RXOBJECT  /* use Object REXX defines from REXXSAA.H */
#include SYSREXXSAA


#ifdef FIXEDTIMERS
extern RexxActivity * LastRunningActivity;
#endif

extern RexxDirectory *ProcessLocalEnv;

/* max instructions without a yield */
#define MAX_INSTRUCTIONS  100
                                       /* routine to restore the Environment*/
                                       /* defined in xxxEXTF.C              */
void RestoreEnvironment(void *);



                                       /* default template for a new        */
                                       /* activation.  This must be changed */
                                       /* whenever the settings definition  */
                                       /* changes                           */
static ACTSETTINGS activationSettingsTemplate;

extern INT  lookup[];


void * RexxActivation::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new activation object                                  */
/******************************************************************************/
{
  RexxActivation * newObject;          /* new activation object             */

                                       /* Get new object                    */
  newObject = (RexxActivation *)new_object(size);
                                       /* Give new object its behaviour     */
  BehaviourSet(newObject, TheActivationBehaviour);
  return (RexxObject *)newObject;      /* return the new object             */
}

RexxActivation::RexxActivation(
     RexxObject     * receiver,        /* message receiver                  */
     RexxMethod     * method,          /* method to run                     */
     RexxActivity   * activity,        /* current activity                  */
     RexxString     * msgname,         /* message name processed            */
     RexxActivation * activation,      /* parent activation                 */
     int              context )        /* execution context                 */
/******************************************************************************/
/* Function:  Initialize an activation object instance.                       */
/*            NOTE:  This object is created by the activity class instead of  */
/*            directly by the activation class.  The activity class protects  */
/*            the new object so that it is not necessary to protect this      */
/*            from garbage collection during the INIT method.                 */
/******************************************************************************/
{
  ClearObject(this);                   /* start with a fresh object         */
  if (context == DEBUGPAUSE) {         /* actually a debug pause?           */
    this->debug_pause = TRUE;          /* set up for debugging intercepts   */
    context = INTERPRET;               /* this is really an interpret       */
  }
  this->settings.intermediate_trace = FALSE;
  this->activation_context = context;  /* save the context                  */
  this->receiver = receiver;           /* save the message receiver         */
  this->method = method;               /* save the method pointer           */
  this->code = method->rexxCode;       /* get the REXX method object        */
  this->source = this->code->u_source; /* save the source                   */
  this->activity = activity;           /* save the activity pointer         */
                                       /* save the sender activation        */
  this->sender = activity->currentAct();
  this->execution_state = ACTIVE;      /* we are now in active execution    */
  this->object_scope = SCOPE_RELEASED; /* scope not reserved yet            */
                                       /* default to method for now         */
  this->settings.calltype = OREF_METHODNAME;
  /* create a new evaluation stack.  This must be done before a */
  /* local variable frame is created. */
  SetObjectHasNoReferences(this);      /* during allocateStack..            */
                                       /* a live marking can happen without */
                                       /* a properly set up stack (::live() */
                                       /* is called). Setting the NoRefBit  */
                                       /* when creating the stack avoids it.*/
  activity->allocateStackFrame(&this->stack, this->code->maxStack);
  SetObjectHasReferences(this);
  if (context&INTERNAL_LEVEL_CALL) {   /* internal call or interpret?       */
                                       /* inherit parents settings          */
    activation->putSettings(&this->settings);
    if (context == INTERNALCALL) {     /* internal call?                    */
                                       /* force a new copy of the traps     */
                                       /* table to be created whenever it   */
                                       /* is changed                        */
      this->settings.flags &= ~traps_copied;
      this->settings.flags &= ~reply_issued; /* this is a new activation that can use its own return */
                                       /* invalidate the timestamp          */
      this->settings.timestamp.valid = FALSE;
    }
    /* this is a nested call until we issue a procedure */
    settings.local_variables.setNested();
  }
  else {                               /* external method activation        */
                                       /* get initial settings template     */
    memcpy((PVOID)&this->settings, (PVOID)&activationSettingsTemplate, sizeof(this->settings));
                                       /* set up for internal calls         */
    this->settings.parent_method = this->method;
                                       /* save the source also              */
    this->settings.parent_source = this->source;

    /* allocate a frame for the local variables from activity stack */
    settings.local_variables.init(this, code->vdictSize);
    this->activity->allocateLocalVariableFrame(&settings.local_variables);
                                       /* set the initial and initial       */
                                       /* alternate address settings        */
    this->settings.current_env = SysInitialAddressName();
    this->settings.alternate_env = this->settings.current_env;
                                       /* get initial random seed value     */
    this->random_seed = this->activity->nestedInfo.randomSeed;
                                       /* copy the source security manager  */
    this->settings.securityManager = this->source->securityManager;
                                       /* default to method for now         */
    this->settings.calltype = OREF_METHODNAME;
  }
  this->settings.msgname = msgname;    /* use the passed message name       */
}


RexxObject * RexxActivation::dispatch()
/******************************************************************************/
/* Function:  Re-dispatch an activation after a REPLY                         */
/******************************************************************************/
{
                                       /* go run this                       */
  RexxObject * result;
  result = this->run(NULL, 0, OREF_NULL);
  if (result != OREF_NULL) discard(result);
  return result;
}


RexxObject * RexxActivation::run(
     RexxObject      **arglist,        /* argument list to the activity     */
     size_t            argcount,       /* the argument count                */
     RexxInstruction * start)          /* starting instruction              */
/******************************************************************************/
/* Function:  Run a REXX method...this is it!  This is the heart of the       */
/*            interpreter that makes the whole thing run!                     */
/******************************************************************************/
{
  RexxActivationBase*activation;       /* used for unwinding activations    */
  LONG               i;                /* loop counter                      */
  RexxActivity      *oldActivity;      /* old activity                      */
#ifndef FIXEDTIMERS                    /* currently disabled                */
  LONG               instructionCount; /* instructions without yielding     */
#endif
  RexxExpressionStack *stack;          /* current execution stack           */
  RexxInstruction     *next;           /* next instruction to execute       */
  RexxObject          *result;         /* the return result                 */
  BOOL                fDebuggerSet;    /* debug exits installed?            */

                                       /* not a reply restart situation?    */
  if (this->execution_state != REPLIED) {
                                       /* exits possible?                   */
    if (!this->method->isInternal() && this->activity->querySet()) {
                                       /* check at the end of each clause   */
      this->settings.flags |= clause_boundary;
                                       /* remember that we have sys exits   */
      this->settings.flags |= clause_exits;
    }
    this->arglist = arglist;           /* set the argument list             */
    this->argcount = argcount;
                                       /* first entry into here?            */
    if (this->activation_context&TOP_LEVEL_CALL) {
                                       /* save entry argument list for      */
                                       /* variable pool fetch private       */
                                       /* access                            */
      settings.parent_arglist = arglist;
      settings.parent_argcount = argcount;
      this->source->install(this);     /* do any required installations     */
      this->next = this->code->start;  /* ask the method for the start point*/
      this->current = this->next;      /* set the current too (for errors)  */
                                       /* not an internal method?           */
      if (this->activation_context&PROGRAM_LEVEL_CALL) {
                                       /* run initialization exit           */
        this->activity->sysExitInit(this);
        SysSetupProgram(this);         /* do any system specific setup      */
      }
      else {
                                       /* guarded method?                   */
        if (this->method->isGuarded()) {
                                       /* get the object variables          */
          this->settings.object_variables = this->receiver->getObjectVariables(this->method->scope);
                                       /* reserve the variable scope        */
          this->settings.object_variables->reserve(this->activity);
                                       /* and remember for later            */
          this->object_scope = SCOPE_RESERVED;
        }
                                       /* initialize the this variable      */
        this->setLocalVariable(OREF_SELF, VARIABLE_SELF, this->receiver);
                                       /* initialize the SUPER variable     */
        this->setLocalVariable(OREF_SUPER, VARIABLE_SUPER, this->receiver->superScope(this->method->scope));
      }
    }
    else {
      if (start == OREF_NULL)          /* no starting location given?       */
        this->next = this->code->start;/* ask the method for the start point*/
      else
        this->next = start;            /* set that as the current           */
      this->current = this->next;      /* set the current too (for errors)  */
    }
  }
  else {                               /* resuming after a reply            */
                                       /* need to reaquire the lock?        */
    if (this->settings.flags&transfer_failed) {
                                       /* re-lock the variable dictionary   */
      this->settings.object_variables->reserve(this->activity);
                                       /* turn off the failure flag         */
      this->settings.flags &= ~transfer_failed;
    }
  }
                                       /* internal call?                    */
  if (this->activation_context == INTERNALCALL) {
    start = this->next;                /* get the starting point            */
                                       /* scan over the internal labels     */
    while (start != OREF_NULL && start->instructionInfo.type == KEYWORD_LABEL)
      start = start->nextInstruction;  /* step to the next one              */
                                       /* this a procedure instruction      */
    if (start != OREF_NULL && start->instructionInfo.type == KEYWORD_PROCEDURE)
                                       /* flip on the procedure flag        */
      this->settings.flags |= procedure_valid;
  }
  this->execution_state = ACTIVE;      /* we are now actively processing    */
                                       /* have we had a condition or        */
                                       /* exception of some kind raised?    */
  if (setjmp(this->conditionjump) != 0) {
                                       /* unwind the activation stack       */
    while ((activation = this->activity->current()) != this) {
      activation->termination();       /* prepare the activation for termin */
      this->activity->pop(FALSE);      /* pop the activation off the stack  */
    }
    this->stack.clear();               /* Force the stack clear             */
                                       /* invalidate the timestamp          */
    this->settings.timestamp.valid = FALSE;
    if (this->debug_pause) {           /* in a debug pause?                 */
      this->execution_state = RETURNED;/* cause termination                 */
      this->next = OREF_NULL;          /* turn off execution engine         */
    }
                                       /* have pending conditions?          */
    if (this->condition_queue != OREF_NULL)
                                       /* get the pending count             */
      this->pending_count = this->condition_queue->getSize();
    if (this->pending_count != 0) {    /* do we have trapped conditions?    */
      this->processTraps();            /* go dispatch the traps             */
      if (this->pending_count != 0)    /* have deferred conditions?         */
                                       /* need to check each time around    */
        this->settings.flags |= clause_boundary;
    }
                                       /* now fall back into processing loop*/
  }
  stack = &this->stack;                /* load up the stack                 */
#ifndef FIXEDTIMERS                    /* currently disabled                */
  instructionCount = 0;                /* no instructions yet               */
#endif
  next = this->next;                   /* get the next instruction          */
  SysClauseBoundary(this);             /* take one shot at clause stuff     */

                                       /* if no debug exit is set, calls in */
                                       /* the execution loop can be skipped */
  fDebuggerSet = this->activity->nestedInfo.exitset;
                                       /* loop until we get a terminating   */
  while (next != OREF_NULL) {          /* condition                         */

#ifdef FIXEDTIMERS                     /* currently disabled (active on Win)*/
                                       /* has time Slice expired?           */
    if (SysTimeSliceElapsed()) {
      this->activity->relinquish();    /* yield control to the activity     */
    }
#else
                                       /* need to give someone else a shot? */
    if (++instructionCount > MAX_INSTRUCTIONS) {
      this->activity->relinquish();    /* yield control to the activity     */
      instructionCount = 0;            /* reset to zero                     */
    }
#endif

    this->current = next;              /* set the next instruction          */
    this->next = next->nextInstruction;/* prefetch the next clause          */

    if (fDebuggerSet && !(this->settings.dbg_flags&dbg_trace)) this->callDbgExit();

    next->execute(this, stack);        /* execute the instruction           */
    stack->clear();                    /* Force the stack clear             */
    if (fDebuggerSet)
      this->dbgCheckEndStepOver();       /* clear stepover flag */

    this->settings.timestamp.valid = FALSE;

    SysClauseBoundary(this);           /* process any required system stuff */
                                       /* need to process inter-clause stuff*/
    if (this->settings.flags&clause_boundary)
      this->processClauseBoundary();   /* go do the clause boundary stuff   */

    next = this->next;                 /* get the next instruction          */
  }

  if (fDebuggerSet)
    this->dbgCheckEndStepOut();

  if (this->execution_state == ACTIVE) /* implicit exit?                    */
    this->implicitExit();              /* treat this like an EXIT           */
                                       /* is this a return situation?       */
  if (this->execution_state == RETURNED) {
    this->termination();               /* do activation termination process */
    if (this->activation_context == INTERPRET) {
      /* save the nested setting */
      BOOL nested = this->sender->settings.local_variables.isNested();
                                       /* propagate parent's settings back  */
      this->sender->getSettings(&this->settings);
      if (!nested) {
          /* if our calling variable context was not nested, we */
          /* need to clear it. */
          this->sender->settings.local_variables.clearNested();
      }
                                       /* merge any pending conditions      */
      this->sender->mergeTraps(this->condition_queue, this->handler_queue);
    }
    result = this->result;             /* save the result                   */
    if (result != OREF_NULL) save(result);
    this->activity->pop(FALSE);        /* now pop the current activity      */
    /* now go run the uninit stuff       */
    TheActivityClass->checkUninitQueue();
  }
  else {                               /* execution_state is REPLIED        */
    result = this->result;             /* save the result                   */
    if (result != OREF_NULL) save(result);
                                       /* reset the next instruction        */
    this->next = this->current->nextInstruction;
    oldActivity = this->activity;      /* save the current activity         */
                                       /* clone the current activity        */
    this->activity = new_activity(oldActivity->local);
    for (i = 1; i <= LAST_EXIT; i++)   /* copy any exit handlers            */
                                       /* from old activity to the new one  */
      this->activity->setSysExit(i, oldActivity->querySysExits(i));

    /* save the pointer to the start of our stack frame.  We're */
    /* going to need to release this after we migrate everything */
    /* over. */
    RexxObject **framePtr = stack->getFrame();
    /* migrate the local variables and the expression stack to the */
    /* new activity.  NOTE:  these must be done in this order to */
    /* get them allocated from the new activity in the correct */
    /* order. */
    stack->migrate(this->activity);
    settings.local_variables.migrate(this->activity);
    /* if we have arguments, we need to migrate those also, as they are subject to overwriting once we return to the parent activation.  */
    if (argcount > 0) {
        RexxObject **newArguments = activity->allocateFrame(argcount);
        memcpy(newArguments, arglist, sizeof(RexxObject *) * argcount);
        this->arglist = newArguments;  /* must be set on "this"  */
        settings.parent_arglist = newArguments;
    }

    /* return our stack frame space back to the old activity. */
    oldActivity->releaseStackFrame(framePtr);

    this->activity->push(this);        /* push it on to the activity stack  */
    oldActivity->pop(TRUE);            /* pop existing one off the stack    */
                                       /* is the scope reserved?            */
    if (this->object_scope == SCOPE_RESERVED) {
                                       /* transfer the reservation          */
      if (!this->settings.object_variables->transfer(this->activity))
                                       /* remember the failure              */
        this->settings.flags |= transfer_failed;
    }
                                       /* we're now the top activation      */
    this->sender = (RexxActivation *)TheNilObject;
    this->activity->run();             /* continue running the new activity */
    oldActivity->yield(OREF_NULL);     /* give other activity a chance to go*/
  }
  return result;                       /* return the result object          */
}

void RexxActivation::processTraps()
/******************************************************************************/
/* Function:  process pending condition traps before going on to execute a    */
/*            clause                                                          */
/******************************************************************************/
{
  RexxArray     * trapHandler;         /* current trap handler              */
  RexxDirectory * conditionObj;        /* associated condition object       */
  RexxObject    * rc;                  /* return code value                 */
  LONG            i;                   /* loop counter                      */

  i = this->pending_count;             /* get the pending count             */
  while (i--) {                        /* while pending conditions          */
                                       /* get the handler off the queue     */
    trapHandler = (RexxArray *)this->handler_queue->pullRexx();
                                       /* condition in DELAY state?         */
    if ((RexxString *)this->trapState((RexxString *)trapHandler->get(3)) == OREF_DELAY) {
                                       /* add to the end of the queue       */
      this->handler_queue->addLast(trapHandler);
                                       /* move condition object to the end  */
      this->condition_queue->addLast(this->condition_queue->pullRexx());
    }
    else {
      this->pending_count--;           /* decrement the pending count       */
                                       /* get the current condition object  */
      conditionObj = (RexxDirectory *)this->condition_queue->pullRexx();
      rc = conditionObj->at(OREF_RC);  /* get any return code information   */
      if (rc != OREF_NULL)             /* have something to assign to RC?   */
                                       /* initialize the RC variable        */
        this->setLocalVariable(OREF_RC, VARIABLE_RC, rc);
                                       /* call the condition handler        */
      ((RexxInstructionCallBase *)trapHandler->get(1))->trap(this, conditionObj);
    }
  }
}


void RexxActivation::debugSkip(
    long skipcount,                    /* clauses to skip pausing           */
    BOOL notrace )                     /* tracing suppression flag          */
/******************************************************************************/
/* Function:  Process a numeric "debug skip" TRACE instruction to suppress    */
/*            pauses or tracing for a given number of instructions.           */
/******************************************************************************/
{
  if (!this->debug_pause)              /* not an allowed state?             */
                                       /* report the error                  */
    report_exception(Error_Invalid_trace_debug);
                                       /* copy the execution count          */
  this->settings.trace_skip = skipcount;
                                       /* set the skip flag                 */
  if (notrace)                         /* turning suppression on?           */
                                       /* flip on the flag                  */
    this->settings.flags |= trace_suppress;
  else                                 /* skipping pauses only              */
    this->settings.flags &= ~trace_suppress;
  this->settings.flags |= debug_bypass;/* let debug prompt know of changes  */
}

RexxString * RexxActivation::traceSetting()
/******************************************************************************/
/* Function:  Generate a string form of the current trace setting             */
/******************************************************************************/
{
  UCHAR        setting[3];             /* returned trace setting            */
  RexxString  *result;                 /* returned result                   */

  setting[0] = '\0';                   /* start with a null string          */
                                       /* debug mode?                       */
  if (this->settings.flags&trace_debug) {
    setting[0] = '?';                  /* add the question mark             */
                                       /* add current trace option          */
    setting[1] = this->settings.traceoption;
                                       /* create a string form              */
    result = new_string((PCHAR)setting, 2);
  }
  else {                               /* no debug prefix                   */
                                       /* add current trace option          */
    setting[0] = this->settings.traceoption;
                                       /* create a string form              */
    result = new_string((PCHAR)setting, 1);
  }
  return result;                       /* return the setting                */
}


void RexxActivation::setTrace(
    int   traceoption,                 /* new trace setting                 */
    int   debugoption )                /* new interactive debug setting     */
/******************************************************************************/
/* Function:  Set a new trace setting for a REXX program                      */
/******************************************************************************/
{
                                       /* turn off the trace suppression    */
  this->settings.flags &= ~trace_suppress;
  this->settings.trace_skip = 0;       /* and allow debug pauses            */

  switch (debugoption) {

    case DEBUG_ON:                     /* turn on interactive debug         */
                                       /* switch the setting on             */
      this->settings.flags |= trace_debug;
      break;

    case DEBUG_OFF:                    /* turn off interactive debug        */
                                       /* switch the setting off            */
      this->settings.flags &= ~trace_debug;
      break;

    case DEBUG_TOGGLE:                 /* toggole interactive debug setting */
                                       /* switch to the opposite setting    */
                                       /* already on?                       */
      if (this->settings.flags & trace_debug)
                                       /* switch the setting off            */
        this->settings.flags &= ~trace_debug;
      else
                                       /* switch the setting on             */
        this->settings.flags |= trace_debug;
      break;

    case DEBUG_IGNORE:                 /* no changes to debug setting       */
      break;
  }
  switch (traceoption) {

    case TRACE_ALL:                    /* TRACE ALL;                        */

      clearTraceSettings();            /* turn off existing flags           */
                                       /* trace instructions, labels and    */
                                       /* all commands                      */
      this->settings.flags |= (trace_all | trace_labels | trace_commands);
                                       /* save the trace option             */
      this->settings.traceoption = traceoption;
      break;

    case TRACE_COMMANDS:               /* TRACE COMMANDS;                   */
      clearTraceSettings();            /* turn off existing flags           */
                                       /* just trace commands               */
      this->settings.flags |= trace_commands;
                                       /* save the trace option             */
      this->settings.traceoption = traceoption;
      break;

    case TRACE_LABELS:                 /* TRACE LABELS                      */
      clearTraceSettings();            /* turn off existing flags           */
                                       /* just trace labels                 */
      this->settings.flags |= trace_labels;
                                       /* save the trace option             */
      this->settings.traceoption = traceoption;
      break;

    case TRACE_NORMAL:                 /* TRACE NORMAL                      */
    case TRACE_FAILURES:               /* TRACE FAILURES                    */
      clearTraceSettings();            /* turn off existing flags           */
                                       /* just trace command failures       */
      this->settings.flags |= trace_failures;
                                       /* save the trace option             */
      this->settings.traceoption = traceoption;
      break;

    case TRACE_ERRORS:                 /* TRACE ERRORS                      */
      clearTraceSettings();            /* turn off existing flags           */
                                       /* trace command failures and error  */
      this->settings.flags |= (trace_failures | trace_errors);
                                       /* save the trace option             */
      this->settings.traceoption = traceoption;
      break;

    case TRACE_RESULTS:                /* TRACE RESULTS                     */
      clearTraceSettings();            /* turn off existing flags           */
                                       /* trace instructions, labels,       */
                                       /* commands, and results             */
      this->settings.flags |= (trace_all | trace_labels | trace_results | trace_commands);
                                       /* save the trace option             */
      this->settings.traceoption = traceoption;
      break;

    case TRACE_INTERMEDIATES:          /* TRACE INTERMEDIATES               */
      clearTraceSettings();            /* turn off existing flags           */
                                       /* trace just about every things     */
      this->settings.flags |= (trace_all | trace_labels | trace_results | trace_commands | trace_intermediates);
                                       /* save the trace option             */
      this->settings.traceoption = traceoption;
      /* turn on the special fast-path test */
      this->settings.intermediate_trace = TRUE;
      break;

    case TRACE_OFF:                    /* TRACE OFF                         */
                                       /* save the trace option             */
      clearTraceSettings();            /* turn off existing flags           */
                                       /* nothing traced at all             */
      this->settings.flags &= ~trace_debug;
      this->settings.traceoption = traceoption;
                                       /* ALWAYS switch debug off with OFF  */
      this->settings.flags &= ~trace_debug;
      break;

    case TRACE_IGNORE:                 /* don't change trace setting        */
      break;
  }
                                       /* now double check for debug on     */
                                       /* while trace is off                */
  if (this->settings.traceoption == TRACE_OFF)
                                       /* ALWAYS switch debug off with OFF  */
    this->settings.flags &= ~trace_debug;
  if (this->debug_pause)               /* issued from a debug prompt?       */
                                       /* let debug prompt know of changes  */
    this->settings.flags |= debug_bypass;
}

void RexxActivation::live()
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->method);
  memory_mark(this->code);
  memory_mark(this->source);
  memory_mark(this->settings.securityManager);
  memory_mark(this->receiver);
  memory_mark(this->activity);
  memory_mark(this->sender);
  memory_mark(this->dostack);
  /* the stack and the local variables handle their own marking. */
  this->stack.live();
  this->settings.local_variables.live();
  memory_mark(this->current);
  memory_mark(this->next);
  memory_mark(this->result);
  memory_mark(this->trapinfo);
  memory_mark(this->objnotify);
  memory_mark(this->environmentList);
  memory_mark(this->handler_queue);
  memory_mark(this->condition_queue);
  memory_mark(this->settings.traps);
  memory_mark(this->settings.conditionObj);
  memory_mark(this->settings.parent_method);
  memory_mark(this->settings.parent_source);
  memory_mark(this->settings.current_env);
  memory_mark(this->settings.alternate_env);
  memory_mark(this->settings.msgname);
  memory_mark(this->settings.object_variables);
  memory_mark(this->settings.calltype);
  memory_mark(this->settings.streams);
  memory_mark(this->settings.halt_description);

  /* We're hold a pointer back to our arguments directly where they */
  /* are created.  Since in some places, this argument list comes */
  /* from the C stack, we need to handle the marker ourselves. */
  size_t i;
  for (i = 0; i < argcount; i++) {
      memory_mark(arglist[i]);
  }

  for (i = 0; i < settings.parent_argcount; i++) {
      memory_mark(settings.parent_arglist[i]);
  }

  cleanUpMemoryMark
}

void RexxActivation::liveGeneral()
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
  setUpMemoryMarkGeneral
  memory_mark_general(this->method);
  memory_mark_general(this->code);
  memory_mark_general(this->source);
  memory_mark_general(this->settings.securityManager);
  memory_mark_general(this->receiver);
  memory_mark_general(this->activity);
  memory_mark_general(this->sender);
  memory_mark_general(this->dostack);
  /* the stack and the local variables handle their own marking. */
  this->stack.liveGeneral();
  this->settings.local_variables.liveGeneral();
  memory_mark_general(this->current);
  memory_mark_general(this->next);
  memory_mark_general(this->result);
  memory_mark_general(this->trapinfo);
  memory_mark_general(this->objnotify);
  memory_mark_general(this->environmentList);
  memory_mark_general(this->handler_queue);
  memory_mark_general(this->condition_queue);
  memory_mark_general(this->settings.traps);
  memory_mark_general(this->settings.conditionObj);
  memory_mark_general(this->settings.parent_method);
  memory_mark_general(this->settings.parent_source);
  memory_mark_general(this->settings.current_env);
  memory_mark_general(this->settings.alternate_env);
  memory_mark_general(this->settings.msgname);
  memory_mark_general(this->settings.object_variables);
  memory_mark_general(this->settings.calltype);
  memory_mark_general(this->settings.streams);
  memory_mark_general(this->settings.halt_description);

  /* We're hold a pointer back to our arguments directly where they */
  /* are created.  Since in some places, this argument list comes */
  /* from the C stack, we need to handle the marker ourselves. */
  size_t i;
  for (i = 0; i < argcount; i++) {
      memory_mark_general(arglist[i]);
  }

  for (i = 0; i < settings.parent_argcount; i++) {
      memory_mark_general(settings.parent_arglist[i]);
  }
  cleanUpMemoryMarkGeneral
}

void RexxActivation::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
                                       /* Activations don't get moved,      */
                                       /*  we just return OREF_NULL. we may */
                                       /*  create a special proxy for this  */
                                       /*  to re-establish an activation on */
                                       /*  system.                          */
  return;
}

void RexxActivation::reply(
     RexxObject * result)              /* returned REPLY result             */
/******************************************************************************/
/* Function:  Process a REXX REPLY instruction                                */
/******************************************************************************/
{
                                       /* already had a reply issued?       */
  if (this->settings.flags&reply_issued)
                                       /* flag this as an error             */
    report_exception(Error_Execution_reply);
  this->settings.flags |= reply_issued;/* turn on the replied flag          */
                                       /* change execution state to         */
  this->execution_state = REPLIED;     /* terminate the main loop           */
  this->next = OREF_NULL;              /* turn off execution engine         */
  this->result = result;               /* save the result value             */
}


void RexxActivation::returnFrom(
     RexxObject * result)              /* returned RETURN/EXIT result       */
/******************************************************************************/
/* Function:  process a REXX RETURN instruction                               */
/******************************************************************************/
{
                                       /* already had a reply issued?       */
  if (this->settings.flags&reply_issued && result != OREF_NULL)
                                       /* flag this as an error             */
    report_exception(Error_Execution_reply_return);
                                       /* processing an Interpret           */
  if (this->activation_context == INTERPRET) {
    this->execution_state = RETURNED;  /* this is a returned state          */
    this->next = OREF_NULL;            /* turn off execution engine         */
                                       /* cause a return in the sender      */
    this->sender->returnFrom(result);  /* activity                          */
  }
  else {
    this->execution_state = RETURNED;  /* the state is returned             */
    this->next = OREF_NULL;            /* turn off execution engine         */
    this->result = result;             /* save the return result            */
                                       /* real program call?                */
    if (this->activation_context&PROGRAM_LEVEL_CALL)
                                       /* run termination exit              */
      this->activity->sysExitTerm(this);
  }
                                       /* switch debug off to avoid debug   */
                                       /* pause after exit entered from an  */
  this->settings.flags &= ~trace_debug;/* interactive debug prompt          */
  this->settings.flags |= debug_bypass;/* let debug prompt know of changes  */
}


void RexxActivation::iterate(
     RexxString * name )               /* name specified on iterate         */
/******************************************************************************/
/* Function:  Process a REXX ITERATE instruction                              */
/******************************************************************************/
{
  RexxDoBlock       * doblock;         /* current DO block                  */
  RexxBlockInstruction * loop;         /* actual loop instruction           */

  doblock = this->topBlock();          /* get the first stack item          */

  while (doblock != OREF_NULL) {       /* while still DO blocks to process  */
    loop = doblock->getParent();       /* get the actual loop instruction   */
    if (name == OREF_NULL)             // leaving the inner-most loop?
    {
        // we only recognize LOOP constructs for this.
        if (loop->isLoop())
        {
                                       /* reset the indentation             */
            this->setIndent(doblock->getIndent());
            ((RexxInstructionDo *)loop)->reExecute(this, &this->stack, doblock);
            return;                          /* we're finished                    */
        }

    }
    // a named LEAVE can be either a labeled block or a loop.
    else if (loop->isLabel(name))
    {
        if (!loop->isLoop())
        {
            report_exception1(Error_Invalid_leave_iterate_name, name);
        }
                                   /* reset the indentation             */
        this->setIndent(doblock->getIndent());
        ((RexxInstructionDo *)loop)->reExecute(this, &this->stack, doblock);
        return;                          /* we're finished                    */
    }
    this->popBlock();                  /* cause termination cleanup         */
    this->removeBlock();               /* remove the execution nest         */
    doblock = this->topBlock();        /* get the new stack top             */
  }
  if (name != OREF_NULL)               /* have a name?                      */
                                       /* report exception with the name    */
    report_exception1(Error_Invalid_leave_iteratevar, name);
  else
                                       /* have a misplaced ITERATE          */
    report_exception(Error_Invalid_leave_iterate);
}


void RexxActivation::leaveLoop(
     RexxString * name )               /* name specified on leave           */
/******************************************************************************/
/* Function:  Process a REXX LEAVE instruction                                */
/******************************************************************************/
{
  RexxDoBlock       * doblock;         /* current DO block                  */

  doblock = this->topBlock();          /* get the first stack item          */

  while (doblock != OREF_NULL) {       /* while still DO blocks to process  */
    RexxBlockInstruction *loop = doblock->getParent();       /* get the actual loop instruction   */
    if (name == OREF_NULL)             // leaving the inner-most loop?
    {
        // we only recognize LOOP constructs for this.
        if (loop->isLoop())
        {
            loop->terminate(this, doblock);  /* terminate the loop                */
            return;                          /* we're finished                    */
        }

    }
    // a named LEAVE can be either a labeled block or a loop.
    else if (loop->isLabel(name))
    {
        loop->terminate(this, doblock);  /* terminate the loop                */
        return;                          /* we're finished                    */
    }
    this->popBlock();                  /* cause termination cleanup         */
    this->removeBlock();               /* remove the execution nest         */
                                       /* get the first stack item again    */
    doblock = this->topBlock();        /* get the new stack top             */
  }
  if (name != OREF_NULL)               /* have a name?                      */
                                       /* report exception with the name    */
    report_exception1(Error_Invalid_leave_leavevar, name);
  else
                                       /* have a misplaced LEAVE            */
    report_exception(Error_Invalid_leave_leave);
}

long RexxActivation::currentLine()
/******************************************************************************/
/* Function:  Return the line number of the current instruction               */
/******************************************************************************/
{
  if (this->current != OREF_NULL)      /* have a current line?              */
    return this->current->getLine();   /* return the line number            */
  else
    return 1;                          /* error on the loading              */
}


void RexxActivation::procedureExpose(
    RexxVariableBase **variables, size_t count)
/******************************************************************************/
/* Function:  Expose variables for a PROCEDURE instruction                    */
/******************************************************************************/
{
                                       /* procedure not allowed here?       */
  if (!(this->settings.flags&procedure_valid))
                                       /* raise the appropriate error!      */
    report_exception(Error_Unexpected_procedure_call);
                                       /* disable further procedures        */
  this->settings.flags &= ~procedure_valid;

  /* get a new  */
  activity->allocateLocalVariableFrame(&settings.local_variables);
  /* make sure we clear out the dictionary, otherwise we'll see the */
  /* dynamic entries from the previous level. */
  settings.local_variables.procedure(this);

  size_t i;
  /* now expose each individual variable */
  for (i = 0; i < count; i++) {
      variables[i]->procedureExpose(this, sender, &stack);
  }
}


void RexxActivation::expose(
    RexxVariableBase **variables, size_t count)
/******************************************************************************/
/* Function:  Expose variables for an EXPOSE instruction                      */
/******************************************************************************/
{
  size_t i;
  /* get the variable set for this object */
  RexxVariableDictionary * object_variables = getObjectVariables();

  /* now expose each individual variable */
  for (i = 0; i < count; i++) {
      variables[i]->expose(this, &stack, object_variables);
  }
}


RexxObject *RexxActivation::forward(
    RexxObject  * target,              /* target object                     */
    RexxString  * message,             /* message to send                   */
    RexxObject  * superClass,          /* class over ride                   */
    RexxObject ** arguments,           /* message arguments                 */
    size_t        argcount,            /* count of message arguments        */
    BOOL          continuing)          /* return/continue flag              */
/******************************************************************************/
/* Function:  Process a REXX FORWARD instruction                              */
/******************************************************************************/
{
  RexxObject *result;                  /* message result                    */

  if (target == OREF_NULL)             /* no target?                        */
    target = this->receiver;           /* use this                          */
  if (message == OREF_NULL)            /* no message override?              */
    message = this->settings.msgname;  /* use same message name             */
  if (arguments == OREF_NULL) {        /* no arguments given?               */
    arguments = this->arglist;         /* use the same arguments            */
    argcount = this->argcount;
  }
  if (continuing) {                    /* just processing the message?      */
    if (superClass == OREF_NULL)       /* no override?                      */
                                       /* issue the message and return      */
      return target->messageSend(message, argcount, arguments);
    else
                                       /* issue the message with override   */
      return target->messageSend(message, argcount, arguments, superClass);
  }
  else {                               /* got to shut down and issue        */
    this->settings.flags |= forwarded; /* we are now a phantom activation   */
                                       /* already had a reply issued?       */
    if (this->settings.flags&reply_issued  && this->result != OREF_NULL)
                                       /* flag this as an error             */
      report_exception(Error_Execution_reply_exit);
    this->execution_state = RETURNED;  /* this is an EXIT for real          */
    this->next = OREF_NULL;            /* turn off execution engine         */
                                       /* switch debug off to avoid debug   */
                                       /* pause after exit entered from an  */
                                       /* interactive debug prompt          */
    this->settings.flags &= ~trace_debug;
                                       /* let debug prompt know of changes  */
    this->settings.flags |= debug_bypass;
    if (superClass == OREF_NULL)       /* no over ride?                     */
                                       /* issue the simple message          */
      result = target->messageSend(message, argcount, arguments);
    else
                                       /* use the full override             */
      result = target->messageSend(message, argcount, arguments, superClass);
    this->result = result;             /* save the result value             */
                                       /* already had a reply issued?       */
    if (this->settings.flags&reply_issued && result != OREF_NULL)
                                       /* flag this as an error             */
      report_exception(Error_Execution_reply_exit);
    this->termination();               /* run "program" termination method  */
                                       /* if there are stream objects       */
    return OREF_NULL;                  /* just return nothing               */
  }
}

void RexxActivation::exitFrom(
     RexxObject * result)              /* EXIT result                       */
/******************************************************************************/
/* Function:  Process a REXX exit instruction                                 */
/******************************************************************************/
{
  RexxActivation *activation;          /* unwound activation                */

  this->execution_state = RETURNED;    /* this is an EXIT for real          */
  this->next = OREF_NULL;              /* turn off execution engine         */
  this->result = result;               /* save the result value             */
                                       /* switch debug off to avoid debug   */
                                       /* pause after exit entered from an  */
  this->settings.flags &= ~trace_debug;/* interactive debug prompt          */
  this->settings.flags |= debug_bypass;/* let debug prompt know of changes  */
                                       /* at a main program level?          */
  if (this->activation_context&TOP_LEVEL_CALL) {
                                       /* already had a reply issued?       */
    if (this->settings.flags&reply_issued && result != OREF_NULL)
                                       /* flag this as an error             */
      report_exception(Error_Execution_reply_exit);
                                       /* real program call?                */
    if (this->activation_context&PROGRAM_LEVEL_CALL)
                                       /* run termination exit              */
      this->activity->sysExitTerm(this);
  }
  else {                               /* internal routine or Interpret     */
    /* start terminating with this level */
    activation = this;
    do {
      activation->termination();       /* make sure this level cleans up    */
      CurrentActivity->pop(FALSE);     /* pop this level off                */
                                       /* get the next level                */
      activation = CurrentActivity->currentAct();
    } while (!activation->isTopLevel());

    activation->exitFrom(result);      /* tell this level to terminate      */
                                       /* unwind and process the termination*/
    longjmp(activation->conditionjump,1);
  }
}

#if 0
void RexxActivation::implicitExit()
/******************************************************************************/
/* Function:  Process a "fall of the end" exit condition                      */
/******************************************************************************/
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
#endif

void RexxActivation::termination()
/******************************************************************************/
/* Function: do any cleanup due to a program terminating.                     */
/******************************************************************************/
{
  this->guardOff();                    /* Remove any guards for this activatio*/

                                       /* were there any SETLOCAL calls for */
                                       /* this method?  And are there any   */
                                       /* that didn't have a matching ENDLOC*/
  if (this->environmentList != OREF_NULL && this->environmentList->getSize() != 0) {
                                       /* Yes, then restore the environment */
                                       /*  to the ist on added.             */
    RestoreEnvironment(((RexxBuffer *)this->environmentList->lastItem())->data);
  }
  this->environmentList = OREF_NULL;   /* Clear out the env list            */
  this->closeStreams();                /* close any open streams            */
  /* release the stack frame, which also releases the frame for the */
  /* variable cache. */
  this->activity->releaseStackFrame(stack.getFrame());
  /* do the variable termination       */
  cleanupLocalVariables();
}


void RexxActivation::checkTrapTable()
/******************************************************************************/
/* Function:  Create/copy a trap table as needed                              */
/******************************************************************************/
{
                                       /* no trap table created yet?        */
  if (this->settings.traps == OREF_NULL)
                                       /* create the trap table             */
    this->settings.traps = new_directory();
                                       /* have to copy the trap table for an*/
                                       /* internal routine call?            */
  else if (this->activation_context == INTERNALCALL && !(this->settings.flags&traps_copied)) {
                                       /* copy the table                    */
    this->settings.traps = (RexxDirectory *)this->settings.traps->copy();
                                       /* record that we've copied this     */
    this->settings.flags |= traps_copied;
  }
}

void RexxActivation::trapOn(
     RexxString * condition,           /* condition name                    */
                                       /* handler for this trap             */
     RexxInstructionCallBase * handler )
/******************************************************************************/
/* Function:  Activate a condition trap                                       */
/******************************************************************************/
{
  this->checkTrapTable();              /* make sure we have a table         */
                                       /* add the trap to the table         */
  this->settings.traps->put(new_array3((RexxObject *)handler, OREF_ON, condition), condition);
                                       /* novalue condition or any?         */
  if (condition->strCompare(CHAR_NOVALUE) || condition->strCompare(CHAR_ANY))
                                       /* tag the method dictionary         */
    this->settings.local_variables.setNovalueOn();
}


void RexxActivation::trapOff(
     RexxString * condition)           /* condition name                    */
/******************************************************************************/
/* Function:  Disable a condition trap                                        */
/******************************************************************************/
{
  this->checkTrapTable();              /* make sure we have a table         */
                                       /* remove the trap                   */
  this->settings.traps->remove(condition);
                                       /* novalue condition?                */
  if (this->activation_context != INTERNALCALL && condition->strCompare(CHAR_NOVALUE)) {
                                       /* not also trapping ANY?            */
    if (this->settings.traps->at(OREF_ANY) == OREF_NULL)
                                       /* tag the method dictionary         */
      this->settings.local_variables.setNovalueOff();
  }
}

RexxActivation * RexxActivation::external()
/******************************************************************************/
/* Function:  Return the top level external activation                        */
/******************************************************************************/
{
                                       /* if an internal call or an         */
                                       /* interpret, we need to pass this   */
                                       /* along                             */
  if (this->activation_context&INTERNAL_LEVEL_CALL)
    return this->sender->external();   /* get our sender method             */
  else
    return this;                       /* already at the top level          */
}


void RexxActivation::raiseExit(
     RexxString    * condition,        /* condition to raise                */
     RexxObject    * rc,               /* information assigned to RC        */
     RexxString    * description,      /* description of the condition      */
     RexxObject    * additional,       /* extra descriptive information     */
     RexxObject    * result,           /* return result                     */
     RexxDirectory * conditionobj )    /* propagated condition object       */
/******************************************************************************/
/* Function:  Raise a condition using exit semantics for the returned value.  */
/******************************************************************************/
{
                                       /* not internal routine or Interpret */
                                       /* instruction activation?           */
  if (this->activation_context&TOP_LEVEL_CALL) {
                                       /* do the real condition raise       */
    this->raise(condition, rc, description, additional, result, conditionobj);
    return;                            /* return if processed               */
  }

                                       /* reached the top level?            */
  if (this->sender == (RexxActivation *)TheNilObject) {
    this->exitFrom(result);            /* turn into an exit instruction     */
  }
  else {
                                       /* real program call?                */
    if (this->activation_context&PROGRAM_LEVEL_CALL)
                                       /* run termination exit              */
      this->activity->sysExitTerm(this);
    hold(this);                        /* move the activation to hold stack */
    this->termination();               /* remove guarded status on object   */
    this->activity->pop(FALSE);        /* pop ourselves off active list     */
                                       /* propogate the condition backward  */
    this->sender->raiseExit(condition, rc, description, additional, result, conditionobj);
  }
}


void RexxActivation::raise(
     RexxString    * condition,        /* condition to raise                */
     RexxObject    * rc,               /* information assigned to RC        */
     RexxString    * description,      /* description of the condition      */
     RexxObject    * additional,       /* extra descriptive information     */
     RexxObject    * result,           /* return result                     */
     RexxDirectory * conditionobj )    /* propagated condition object       */
/******************************************************************************/
/* Function:  Raise a give REXX condition                                     */
/******************************************************************************/
{
  RexxActivation *sender;              /* "invoker" of current activation   */
  BOOL            propagated;          /* propagated syntax condition       */

                                       /* propagating an existing condition?*/
  if (condition->strCompare(CHAR_PROPAGATE)) {
                                       /* get the original condition name   */
    condition = (RexxString *)conditionobj->at(OREF_CONDITION);
    propagated = TRUE;                 /* this is propagated                */
                                       /* fill in the propagation status    */
    conditionobj->put(TheTrueObject, OREF_PROPAGATED);
    if (result == OREF_NULL)           /* no result specified?              */
      result = conditionobj->at(OREF_RESULT);
  }
  else {                               /* build a condition object          */
    conditionobj = new_directory();    /* get a new directory               */
                                       /* put in the condition name         */
    conditionobj->put(condition, OREF_CONDITION);
                                       /* fill in default description       */
    conditionobj->put(OREF_NULLSTRING, OREF_DESCRIPTION);
                                       /* fill in the propagation status    */
    conditionobj->put(TheFalseObject, OREF_PROPAGATED);
    propagated = FALSE;                /* remember for later                */
  }
  if (rc != OREF_NULL)                 /* have an RC value?                 */
    conditionobj->put(rc, OREF_RC);    /* add to the condition argument     */
  if (description != OREF_NULL)        /* any description to add?           */
    conditionobj->put(description, OREF_DESCRIPTION);
  if (additional != OREF_NULL)         /* or additional information         */
    conditionobj->put(additional, OREF_ADDITIONAL);
  if (result != OREF_NULL)             /* or a result object                */
    conditionobj->put(result, OREF_RESULT);

                                       /* fatal SYNTAX error?               */
  if (condition->strCompare(CHAR_SYNTAX)) {
    hold(this);                        /* move the activation to hold stack */
    if (propagated) {                  /* reraising a condition?            */
      this->termination();             /* do the termination cleanup on ourselves */
      this->activity->pop(FALSE);      /* pop ourselves off active list     */
                                       /* go propagate the condition        */
      CurrentActivity->reraiseException(conditionobj);
    }
    else
                                       /* go raise the error                */
      CurrentActivity->raiseException(((RexxInteger *)rc)->value, NULL, OREF_NULL, description, (RexxArray *)additional, result);
  }
  else {                               /* normal condition trapping         */
                                       /* get the sender object (if any)    */
    sender = this->senderAct();
                                       /* do we have a sender that is       */
                                       /* trapping this condition?          */
                                       /* do we have a sender?              */
    if (sender != (RexxActivation *)TheNilObject)
                                       /* "tickle them" with this           */
      this->sender->trap(condition, conditionobj);
    this->returnFrom(result);          /* process the return part           */
    longjmp(this->conditionjump,1);    /* unwind and process the termination*/
  }
}

RexxVariableDictionary * RexxActivation::getObjectVariables()
/******************************************************************************/
/* Function:  Return the associated object variables vdict                    */
/******************************************************************************/
{
                                       /* no retrieved yet?                 */
  if (this->settings.object_variables == OREF_NULL) {
                                       /* get the object variables          */
    this->settings.object_variables = this->receiver->getObjectVariables(this->method->scope);
    if (this->method->isGuarded()) {   /* guarded method?                   */
                                       /* reserve the variable scope        */
      this->settings.object_variables->reserve(this->activity);
                                       /* and remember for later            */
      this->object_scope = SCOPE_RESERVED;
    }
  }
                                       /* return the vdict                  */
  return this->settings.object_variables;
}

RexxDirectory *RexxActivation::getStreams()
/******************************************************************************/
/* Function:  Return the associated object variables stream table             */
/******************************************************************************/
{
                                       /* not created yet?                  */
  if (this->settings.streams == OREF_NULL) {
                                       /* first entry into here?            */
    if (this->activation_context&PROGRAM_OR_METHOD) {
                                       /* always use a new directory        */
      this->settings.streams = new_directory();
    }
    else
                                       /* alway's use caller's for internal */
                                       /* call, external call or interpret  */
      this->settings.streams = this->sender->getStreams();
  }
  return this->settings.streams;       /* return the stream table           */
}

void RexxActivation::signalTo(
     RexxInstruction * target )        /* target instruction                */
/******************************************************************************/
/* Function:  Signal to a targer instruction                                  */
/******************************************************************************/
{
  long lineNum;
                                       /* internal routine or Interpret     */
                                       /* instruction activation?           */
  if (this->activation_context == INTERPRET) {
    this->execution_state = RETURNED;  /* signal interpret termination      */
    this->next = OREF_NULL;            /* turn off execution engine         */
    this->sender->signalTo(target);    /* propogate the signal backward     */
  }
  else {
                                       /* initialize the SIGL variable      */
    lineNum = this->current->getLine();/* get the instruction line number   */
    this->setLocalVariable(OREF_SIGL, VARIABLE_SIGL, new_integer(lineNum));
    this->next = target;               /* set the new target location       */
    this->dostack = OREF_NULL;         /* clear the do loop stack           */
    this->blockNest = 0;               /* no more active blocks             */
    this->settings.traceindent = 0;    /* reset trace indentation           */
  }
}

void RexxActivation::toggleAddress()
/******************************************************************************/
/* Function:  Toggle the address setting between the current and alternate    */
/******************************************************************************/
{
  RexxString *temp;                    /* temporary swap variable           */

  temp = this->settings.current_env;   /* save the current environment      */
                                       /* make the alternate the current    */
  this->settings.current_env = this->settings.alternate_env;
  this->settings.alternate_env = temp; /* old current is now the alternate  */
}


void RexxActivation::setAddress(
    RexxString * address )             /* new address environment           */
/******************************************************************************/
/* Function:  Set the new current address, moving the current one to the      */
/*            alternate address                                               */
/******************************************************************************/
{
                                       /* old current is now the alternate  */
  this->settings.alternate_env = this->settings.current_env;
  this->settings.current_env = address;/* set new current environment       */
}


void RexxActivation::setDefaultAddress(
    RexxString * address )             /* new address environment           */
/******************************************************************************/
/* Function:  Set up a default address environment so that both the primary   */
/*            and the alternate address are the same value                    */
/******************************************************************************/
{
                                       /* old current is the new one        */
  this->settings.alternate_env = address;
  this->settings.current_env = address;/* set new current environment       */
}


void RexxActivation::signalValue(
    RexxString * name )                /* target label name                 */
/******************************************************************************/
/* Function:  Signal to a computed label target                               */
/******************************************************************************/
{
  RexxInstruction * target;            /* target for the SIGNAL             */
  RexxDirectory   * labels;            /* current method labels             */

  target = OREF_NULL;                  /* no target yet                     */
  labels = this->getLabels();          /* get the labels                    */
  if (labels != OREF_NULL)             /* have labels?                      */
                                       /* look up label and go to normal    */
                                       /* signal processing                 */
    target = (RexxInstruction *)labels->at(name);
  if (target == OREF_NULL)             /* unknown label?                    */
                                       /* raise an error                    */
    report_exception1(Error_Label_not_found_name, name);
  this->signalTo(target);              /* now switch to the label location  */
}


void RexxActivation::guardOn()
/******************************************************************************/
/* Function:  Turn on the activations guarded state                           */
/******************************************************************************/
{
                                       /* currently in unguarded state?     */
  if (this->object_scope == SCOPE_RELEASED) {
                                       /* not retrieved yet?                */
    if (this->settings.object_variables == OREF_NULL)
                                       /* get the object variables          */
      this->settings.object_variables = this->receiver->getObjectVariables(this->method->scope);
                                       /* lock the variable dictionary      */
    this->settings.object_variables->reserve(this->activity);
                                       /* set the state here also           */
    this->object_scope = SCOPE_RESERVED;
  }
}

long RexxActivation::digits()
/******************************************************************************/
/* Function:  Return the current digits setting                               */
/******************************************************************************/
{
  return this->settings.global_settings.digits;
}

long RexxActivation::fuzz()
/******************************************************************************/
/* Function:  Return the current fuzz setting                                 */
/******************************************************************************/
{
  return this->settings.global_settings.fuzz;
}

BOOL RexxActivation::form()
/******************************************************************************/
/* Function:  Return the current FORM setting                                 */
/******************************************************************************/
{
  return this->settings.global_settings.form;
}

void RexxActivation::setDigits(
    long digits)                       /* new digits setting                */
/******************************************************************************/
/* Function:  Set a new digits setting                                        */
/******************************************************************************/
{
  this->settings.global_settings.digits = digits;
}


void RexxActivation::setFuzz(
    long fuzz)                         /* set a new FUZZ setting            */
/******************************************************************************/
/* Function:  Set a new FUZZ setting                                          */
/******************************************************************************/
{
  this->settings.global_settings.fuzz = fuzz;
}


void RexxActivation::setForm(
    BOOL form)                         /* the new FORM setting              */
/******************************************************************************/
/* Function:  Set the new current NUMERIC FORM setting                        */
/******************************************************************************/
{
  this->settings.global_settings.form = form;
}


RexxString * RexxActivation::trapState(
             RexxString * condition)   /* condition trapped                 */
/******************************************************************************/
/* Function:  Return the current state for a trap as either ON, OFF, or DELAY */
/******************************************************************************/
{
  RexxString    * state;               /* current condition trap state      */
  RexxArray     * traphandler;         /* trap handler instruction          */

  state = OREF_OFF;                    /* default to OFF state              */
                                       /* actually have any traps?          */
  if (this->settings.traps != OREF_NULL) {
                                       /* see if this trap is enabled       */
    traphandler = (RexxArray *)this->settings.traps->at(condition);
    if (traphandler != OREF_NULL)      /* have a trap for this?             */
                                       /* get the trap state                */
      state = (RexxString *)traphandler->get(2);
  }
  return state;                        /* return this state                 */
}


void RexxActivation::trapDelay(
    RexxString * condition)            /* condition trapped                 */
/******************************************************************************/
/* Function:  Put a condition trap into the delay state.                      */
/******************************************************************************/
{
  RexxArray * traphandler;             /* trap handler instruction          */

  this->checkTrapTable();              /* make sure we've got the tables    */
                                       /* see if this one is enabled        */
  traphandler = (RexxArray *)this->settings.traps->at(condition);
  if (traphandler != OREF_NULL)        /* have a trap for this?             */
    traphandler->put(OREF_DELAY, 2);   /* change the trap state             */
}


void RexxActivation::trapUndelay(
    RexxString * condition)            /* condition trapped                 */
/******************************************************************************/
/* Function:  Remove a trap from the DELAY state                              */
/******************************************************************************/
{
  RexxArray *traphandler;              /* trap handler instruction          */

  this->checkTrapTable();              /* make sure we've got the tables    */
                                       /* see if this one is enabled        */
  traphandler = (RexxArray *)this->settings.traps->at(condition);
  if (traphandler != OREF_NULL)        /* have a trap for this?             */
    traphandler->put(OREF_ON, 2);      /* change the trap state             */
}


BOOL RexxActivation::trap(             /* trap a condition                  */
    RexxString    * condition,         /* condition to process              */
    RexxDirectory * exception_object)  /* associated condition object       */
/******************************************************************************/
/* Function:  Check the activation to see if this is trapping a condition.    */
/*            For SIGNAL traps, control goes back to the point of the trap    */
/*            via longjmp.  For CALL ON traps, the condition is saved, and    */
/*            the method returns TRUE to indicate the trap was handled.       */
/******************************************************************************/
{
  RexxArray     *traphandler;          /* trap handler instruction          */
  BOOL           handled;              /* condition has been trapped        */
  RexxInstructionCallBase * handler;   /* actual trapping instruction       */
  RexxString    *instruction;          /* actual trapping instruction       */
  RexxActivation *activation;          /* predecessor activation            */

  if (this->settings.flags&forwarded) {/* in the act of forwarding?         */
    activation = this->sender;         /* get the sender activation         */
                                       /* have a predecessor?               */
    while (activation != (RexxActivation *)TheNilObject) {
      if (!activation->isForwarded())  /* non forwarded?                    */
                                       /* pretend he is we                  */
        return activation->trap(condition, exception_object);
      activation = activation->sender; /* step to the next one              */
    }
    return FALSE;                      /* not really here, can't handle     */
  }
                                       /* do we need to notify a message    */
                                       /*obj?                               */
  if (this->objnotify != OREF_NULL && condition->strCompare(CHAR_SYNTAX)) {
                                       /* yes, send error message and       */
                                       /*condition object                   */
    this->objnotify->error(exception_object);
  }
  handled = FALSE;                     /* not handled yet                   */
  traphandler = OREF_NULL;             /* no traps to process yet           */
  if (this->debug_pause) {             /* working from the debug prompt?    */
                                       /* non-terminal condition?           */
    if (!condition->strCompare(CHAR_SYNTAX))
      return FALSE;                    /* flag as not-handled               */
                                       /* go display the messages           */
    this->activity->displayDebug(exception_object);
    longjmp(this->conditionjump,1);    /* unwind and process the trap       */
  }
                                       /* no trap table yet?                */
  if (this->settings.traps == OREF_NULL)
    return FALSE;                      /* can't very well handle this!      */
                                       /* see if this one is enabled        */
  traphandler = (RexxArray *)this->settings.traps->at(condition);

  if (traphandler == OREF_NULL) {      /* not there?  try for an ANY handler*/
                                       /* get this from the same table      */
    traphandler = (RexxArray *)this->settings.traps->at(OREF_ANY);
    if (traphandler != OREF_NULL) {    /* have an any handler?              */
                                       /* get the handler info              */
      handler = (RexxInstructionCallBase *)traphandler->get(1);
                                       /* condition not trappable with CALL?*/
      if (handler->instructionInfo.type == KEYWORD_CALL &&
          (condition->strCompare(CHAR_SYNTAX) ||
           condition->strCompare(CHAR_NOVALUE) ||
           condition->strCompare(CHAR_LOSTDIGITS) ||
           condition->strCompare(CHAR_NOMETHOD) ||
           condition->strCompare(CHAR_NOSTRING)))
        return FALSE;                  /* not trapped                       */
    }
  }
  /* if the condition is being trapped, do the CALL or SIGNAL */
  if (traphandler != OREF_NULL) {      /* have a trap for this?             */
                                       /* if this is a HALT                 */
    if (condition->strCompare(CHAR_HALT))
                                       /* call the sys exit to clear it     */
      this->activity->sysExitHltClr(this);
                                       /* get the handler info              */
    handler = (RexxInstructionCallBase *)traphandler->get(1);
                                       /* no condition queue yet?           */
    if (this->condition_queue == OREF_NULL) {
                                       /* create a pending queue            */
      this->condition_queue = new_queue();
                                       /* and a handler queue               */
      this->handler_queue = new_queue();
    }
    if (handler->instructionInfo.type == KEYWORD_SIGNAL)
      instruction = OREF_SIGNAL;       /* this is trapped by a SIGNAL       */
    else
      instruction = OREF_CALL;         /* this is trapped by a CALL         */
                                       /* add the instruction trap info     */
    exception_object->put(instruction, OREF_INSTRUCTION);
                                       /* create a new condition object and */
                                       /* add the condition object to queue */
    this->condition_queue->addLast(exception_object);
                                       /* and the corresponding trap info   */
    this->handler_queue->addLast(traphandler);
    this->pending_count++;             /* bump pending condition count      */
                                       /* is this a signal instruction      */
                                       /* no the non-returnable PROPAGATE?  */
    if (handler->instructionInfo.type == KEYWORD_SIGNAL) {
                                       /* not an Interpret instruction?     */
      if (this->activation_context != INTERPRET)
        longjmp(this->conditionjump,1);/* unwind and process the trap       */
      else {                           /* unwind interpret activations      */
                                       /* merge the traps                   */
        this->sender->mergeTraps(this->condition_queue, this->handler_queue);
        this->sender->unwindTrap(this);/* go unwind this                    */
      }
    }
    else {
      handled = TRUE;                  /* tell caller we're processing later*/
                                       /* tell clause boundary to process   */
      this->settings.flags |= clause_boundary;
    }
  }
  return handled;                      /* let call know if we've handled    */
}


void RexxActivation::mergeTraps(
    RexxQueue  * condition_queue,      /* previous condition queue          */
    RexxQueue  * handler_queue )       /* current condition handlers queue  */
/******************************************************************************/
/* Function:  Merge a list of trapped conditions from an interpret into the   */
/*            parent activation's queues.                                     */
/******************************************************************************/
{
  LONG   items;                        /* number of items to merge          */

  if (condition_queue != OREF_NULL) {  /* have something to add?            */
                                       /* no condition queue yet?           */
    if (this->condition_queue == OREF_NULL) {
                                       /* just copy over                    */
      this->condition_queue = condition_queue;
                                       /* ...both queues                    */
      this->handler_queue = handler_queue;
    }
    else {
                                       /* get the item count                */
      items = condition_queue->getSize();
      while (items--) {                /* while more items                  */
                                       /* add to the end of the queue       */
        this->handler_queue->addLast(handler_queue->pullRexx());
                                       /* move condition object to the end  */
        this->condition_queue->addLast(condition_queue->pullRexx());
      }
    }
                                       /* reset the counter size            */
    this->pending_count = this->condition_queue->getSize();
  }
}


void RexxActivation::unwindTrap(
    RexxActivation * child )           /* child interpret activation        */
/******************************************************************************/
/* Function:  Unwind a chain of interpret activations to process a SIGNAL ON  */
/*            or PROPAGATE condition trap.  This ensures that the SIGNAL      */
/*            or PROPAGATE returns to the correct condition level             */
/******************************************************************************/
{
                                       /* still an interpret level?         */
  if (this->activation_context == INTERPRET) {
                                       /* merge the traps                   */
    this->sender->mergeTraps(this->condition_queue, this->handler_queue);
    this->sender->unwindTrap(child);   /* unwind another level              */
  }
  else {                               /* reached the "parent" level        */
                                       /* pull back the settings            */
    child->putSettings(&this->settings);
    longjmp(this->conditionjump,1);    /* unwind and process the trap       */
  }
}

RexxActivation * RexxActivation::senderAct()
/******************************************************************************/
/* Function:  Retrieve the activation that activated this activation (whew)   */
/******************************************************************************/
{
  RexxActivation     *sender;          /* sender activation                 */

                                       /* get the sender from the activity  */
  sender = (RexxActivation *)this->getSender();
                                       /* spin down to non-native activation*/
  while (sender != (RexxActivation *)TheNilObject && OTYPE(NativeActivation,sender))
    sender = (RexxActivation *)sender->getSender();
  return sender;                       /* return that activation            */
}

void RexxActivation::interpret(
     RexxString * codestring)          /* string to interpret               */
/******************************************************************************/
/* Function:  Translate and interpret a string of data as a piece of REXX     */
/*            code within the current program context.                        */
/******************************************************************************/
{
  RexxMethod     * newMethod;          /* new method to process             */
  RexxActivation * newActivation;      /* new activation for call           */
  RexxObject     * result;

  this->activity->stackSpace();        /* perform a stack space check       */
                                       /* translate the code                */
  newMethod = this->source->interpret(codestring, this->getLabels(), this->current->getLine());
                                       /* create a new activation           */
  newActivation = TheActivityClass->newActivation(this->receiver, newMethod, this->activity, this->settings.msgname, this, INTERPRET);
  this->activity->push(newActivation); /* push on the activity stack        */
                                       /* run the internal routine on the   */
                                       /* new activation                    */
  result = newActivation->run(arglist, argcount, OREF_NULL);
  if (result != OREF_NULL) discard(result);
}


void RexxActivation::debugInterpret(   /* interpret interactive debug input */
     RexxString * codestring)          /* entered instruction               */
/******************************************************************************/
/* Function:  Interpret a string created for interactive debug                */
/******************************************************************************/
{
  RexxMethod     * newMethod;          /* new method to process             */
  RexxActivation * newActivation;      /* new activation for call           */
  jmp_buf  previous_jump;              /* target error handler              */
  RexxObject     * result;

                                       /* save previous jump handler        */
  memcpy(&previous_jump, &this->conditionjump, sizeof(jmp_buf));
  this->debug_pause = TRUE;            /* now in debug pause                */
                                       /* translation error?                */
  if (setjmp(this->conditionjump) != 0) {
    this->debug_pause = FALSE;         /* no longer in debug                */
                                       /* restore the jump handler          */
    memcpy(&this->conditionjump, &previous_jump, sizeof(jmp_buf));
    return;                            /* this is finished                  */
  }
                                       /* translate the code                */
  newMethod = this->source->interpret(codestring, this->getLabels(), this->current->getLine());

  /* if debug exit is set, debug_pause must also be set during execution */
  if (!(this->activity->nestedInfo.exitset && this->settings.dbg_flags&dbg_trace))
      this->debug_pause = FALSE;           /* no longer in debug                */
                                       /* restore the jump handler          */
  memcpy(&this->conditionjump, &previous_jump, sizeof(jmp_buf));
                                       /* create a new activation           */
  newActivation = TheActivityClass->newActivation(this->receiver, newMethod, this->activity, this->settings.msgname, this, DEBUGPAUSE);
  this->activity->push(newActivation); /* push on the activity stack        */
                                       /* run the internal routine on the   */
                                       /* new activation                    */
  result = newActivation->run(arglist, argcount, OREF_NULL);
  if (result != OREF_NULL) discard(result);

  if (this->activity->nestedInfo.exitset && this->settings.dbg_flags&dbg_trace)
      this->debug_pause = FALSE;           /* no longer in debug                */
}

RexxObject * RexxActivation::rexxVariable(   /* retrieve a program entry          */
     RexxString * name )                     /* target program entry name         */
/******************************************************************************/
/* Function:  Retrieve a REXX defined "dot" environment variable              */
/******************************************************************************/
{
  RexxObject * result;                 /* command return code               */
  result = OREF_NULL;                  /* default to unknown                */

  if (name->strCompare(CHAR_METHODS)) {/* is this ".methods"                */
                                       /* get the methods directory         */
    result = (RexxObject *)this->settings.parent_source->getMethods();
  }
  else if (name->strCompare(CHAR_RS)) {/* command return status (".rs")?    */
    if (this->settings.flags&return_status_set)
                                       /* returned as an integer object     */
      result = new_integer(this->settings.return_status);
    else                               /* just return the name              */
      result = name->concatToCstring(".");
  }
  else if (name->strCompare(CHAR_LINE))  /* current line (".line")?    */
  {
      // if this is an interpret, we need to report the line number of
      // the context that calls the interpret.
      if (this->activation_context == INTERPRET)
      {
          result = sender->rexxVariable(name);
      }
      else
      {

          result = new_integer(this->current->getLine());
      }
  }
  return result;                       /* return the result                 */
}

RexxObject * RexxActivation::externalCall(
    RexxString          * target,      /* target of the call                */
    size_t                argcount,    /* count of arguments                */
    RexxExpressionStack * stack,       /* stack of arguments                */
    RexxString          * calltype )   /* FUNCTION or ROUTINE               */
/******************************************************************************/
/* Function:  Process an external function call                               */
/******************************************************************************/
{
  RexxMethod   * routine;              /* resolved call pointer             */
  RexxObject   * result;               /* command return code               */
  RexxObject   **arguments;            /* argument array                    */
  BOOL           found;

  routine = OREF_NULL;                 /* set not found condition           */
                                       /* get the arguments array           */
  arguments = stack->arguments(argcount);
                                       /* see if we have a ::ROUTINE here   */
  routine = this->settings.parent_source->resolveRoutine(target);
  if (routine == OREF_NULL) {          /* still not found?                  */
                                       /* if exit declines call             */
    if (this->activity->sysExitFunc(this, target, calltype, &result, arguments, argcount)) {
                                       /* exist in functions definition?    */
      routine = (RexxMethod *)TheFunctionsDirectory->get(target);
      if (routine == OREF_NULL)        /* not found yet?                    */
      {
                                           /* do external search and execute    */
          result = SysExternalFunction(this, this->activity, target,
              this->code->getProgramName(), arguments, argcount, calltype, &found);

#ifdef SCRIPTING
          if (!found) {
            char newCalltype[32] = "AX";
            sprintf(newCalltype+2,"%p",calltype); // store info that is is called in engine
                                                  // context, also store calltype information
            found = !this->activity->sysExitFunc(this, target, new_cstring(newCalltype), &result, arguments, argcount);
            //if (found)
            //  result = routine->call(this->activity, (RexxObject *)this, target, arguments, calltype, OREF_NULL, EXTERNALCALL);
          }
#endif
          if (!found)
          {
              routine = (RexxMethod*) ThePublicRoutines->entry(target);
              if (routine == OREF_NULL)        /* not found yet?                    */
                  report_exception1(Error_Routine_not_found_name, target);
              else
                  result = routine->call(this->activity, (RexxObject *)this, target, arguments, argcount, calltype, OREF_NULL, EXTERNALCALL);
          }
      }

      else                             /* we found a routine so run it      */
                                       /* run a special way                 */
        result = routine->call(this->activity, (RexxObject *)this, target, arguments, argcount, calltype, OREF_NULL, EXTERNALCALL);
    }
  }
  else                                 /* we found a routine so run it      */
                                       /* run a special way                 */
    result = routine->call(this->activity, (RexxObject *)this, target, arguments, argcount, calltype, OREF_NULL, EXTERNALCALL);
  return result;                       /* return the function result        */
}


BOOL RexxActivation::callExternalRexx(
  RexxString *      target,            /* Name of external function         */
  RexxString *      parent,            /* name of the parent file           */
  RexxObject **     arguments,         /* Argument array                    */
  size_t            argcount,          /* number of arguments in the call   */
  RexxString *      calltype,          /* Type of call                      */
  RexxObject **     result )           /* Result of function call           */
/******************************************************************************/
/* Function:  Call a rexx protram as an external routine                      */
/******************************************************************************/
{
  RexxString *filename;                /* Full name (string REXXOBJECT)     */
  RexxMethod *routine;                 /* Method object from target         */

                                       /* Get full name including path      */
  filename = SysResolveProgramName(target, parent);
  if (filename != OREF_NULL) {         /* found something?                  */
    this->stack.push(filename);        /* protect the file name here        */
                                       /* try to restore saved image        */
    routine = SysRestoreProgram(filename);
    if (routine == OREF_NULL) {        /* unable to restore?                */
                                       /* go translate the image            */
      routine = TheMethodClass->newFile(filename);
                                       /* go save this method               */
      SysSaveProgram(filename, routine);
    }
    this->stack.pop();                 /* remove the protected name         */
    if (routine == OREF_NULL)          /* Do we have a method???            */
      return FALSE;                    /* No, return not found              */
    else {                             /* Try to run method                 */
      save(routine);
                                       /* run as a call                     */
      *result = routine->call(this->activity, (RexxObject *)this, target, arguments, argcount, calltype, this->settings.current_env, EXTERNALCALL);
                                       /* now merge all of the public info  */
      this->settings.parent_source->mergeRequired(routine->code->u_source);
      discard(routine);
      return TRUE;                     /* Return routine found flag         */
    }
  }
  else
    return FALSE;                      /* this wasn't found                 */
}


RexxObject * RexxActivation::loadRequired(
    RexxString      * target,          /* target of the call                */
    RexxInstruction * instruction )    /* ::REQUIRES instruction from source*/
/******************************************************************************/
/* Function:  Load a routine for a ::REQUIRED directive instruction           */
/******************************************************************************/
{
  RexxString    * fullname = OREF_NULL;/* fully resolved install name       */
  RexxMethod    * method = OREF_NULL;  /* method to invoke                  */
  RexxDirectory * securityArgs;        /* security check arguments          */
  RexxObject    * result;
  USHORT          usMacroPosition;     /* macro search order                */
  BOOL            fFileExists = TRUE;  /* does required file exist          */
  BOOL            fMacroExists = FALSE;/* does required macro exist         */

                                       /* set the current instruction for   */
  this->current = instruction;         /* error reporting                   */
  if (this->hasSecurityManager()) {    /* need to perform a security check? */
    securityArgs = new_directory();    /* get the security arguments        */
                                       /* add the program name              */
    securityArgs->put(target, OREF_NAME);
                                       /* did manager handle this?          */
    if (this->callSecurityManager(OREF_REQUIRES, securityArgs))
                                       /* get the resolved name             */
      fullname = (RexxString *)securityArgs->fastAt(OREF_NAME);
    else
                                       /* go resolve the name               */
      fullname = SysResolveProgramName(target, this->code->getProgramName());
  }
  else
                                       /* go resolve the name               */
    fullname = SysResolveProgramName(target, this->code->getProgramName());

  /* if fullname is still OREF_NULL then no file was found, remember this */
  if (fullname == OREF_NULL)
  {
    fFileExists = FALSE;
    fullname = target; /* use target name for all other things */
  }

  /* check to see whether a macro exists */
  fMacroExists = (RexxQueryMacro(target->stringData, &usMacroPosition) == 0);
  if (fMacroExists && (usMacroPosition == RXMACRO_SEARCH_BEFORE))
    fullname = target; /* use target name for references because macro will be used */

  this->stack.push(fullname);           /* protect the name                  */
  /* Check whether or not required file is listed in the "STATIC_REQUIRES" directory */
  if (TheStaticRequires->entry(fullname) != OREF_NULL)
  {
      this->stack.pop();                   /* now remove the protection         */
      return TheNilObject;                 /* already loaded, just quit without loading */
  }
                                       /* Are we already trying to install  */
                                       /*this ::REQUIRES?                   */
  if (this->activity->runningRequires(fullname) != OREF_NULL) {
    report_exception(Error_Translation_duplicate_requires);
  }

  /* first see if we have something in macrospace with this name */
  if (fMacroExists && (usMacroPosition == RXMACRO_SEARCH_BEFORE))
    method = SysGetMacroCode(target);

  /* if not in PRE macrospace search order, try to load a file */
  if (fFileExists && (method == OREF_NULL))
  {
    method = SysRestoreProgram(fullname);/* try to restore saved image      */
    if (method == OREF_NULL) {           /* unable to restore?              */
                                         /* go translate the image          */
      method = TheMethodClass->newFile(fullname);
      SysSaveProgram(fullname, method);  /* go save this method             */
    }
  }

  /* if still not found try to load POST macro */
  if ((method == OREF_NULL) && fMacroExists)
    method = SysGetMacroCode(target);

  if (method == OREF_NULL)             /* couldn't create this?             */
                                       /* report an error                   */
    report_exception1(Error_Routine_not_found_requires, target);
                                       /* Indicate this routine is being    */
                                       /*installed                          */
  save(method);
  this->activity->addRunningRequires(fullname);
  if (this->hasSecurityManager()) {
    result = securityArgs->fastAt(new_cstring(CHAR_SECURITYMANAGER));
    if (result && result != TheNilObject)
      method->setSecurityManager(result);
  }
  this->stack.pop();                   /* now remove the protection         */
                                       /* run a special way                 */
  result = method->call(this->activity, (RexxObject *)this, target, NULL, 0, OREF_ROUTINENAME, OREF_NULL, EXTERNALCALL);
  if ((result != OREF_NULL) && method->isRexxMethod()) discard(result);
                                       /* No longer installing routine.     */
  this->activity->removeRunningRequires(fullname);
                                       /* now merge all of the info         */
  this->settings.parent_source->mergeRequired(method->code->u_source);
  discard(method);
  return method;                       /* return the method  (but not needed!)  */
}


RexxObject * RexxActivation::internalCall(
    RexxInstruction     *target,       /* target of the call                */
    size_t               argcount,     /* count of arguments                */
    RexxExpressionStack *stack )       /* stack of arguments                */
/******************************************************************************/
/* Function:  Process an internal function or subroutine call                 */
/******************************************************************************/
{
  RexxActivation * newActivation;      /* new activation for call           */
  long             lineNum;            /* line number of the call           */
  RexxObject *     returnObject;
  RexxObject **    arguments = stack->arguments(argcount);

  lineNum = this->current->getLine();  /* get the current line number       */
                                       /* initialize the SIGL variable      */
  this->setLocalVariable(OREF_SIGL, VARIABLE_SIGL, new_integer(lineNum));
                                       /* create a new activation           */
  newActivation = TheActivityClass->newActivation(this->receiver, this->settings.parent_method,
                 this->activity, this->settings.msgname, this, INTERNALCALL);

  this->activity->push(newActivation); /* push on the activity stack        */
                                       /* run the internal routine on the   */
                                       /* new activation                    */
  newActivation->dbgDisableStepOver();
  dbgEnterSubroutine();
  returnObject = newActivation->run(arguments, argcount, target);
  dbgLeaveSubroutine();
  newActivation->dbgPassTrace2Parent(this);
  return returnObject;
}

RexxObject * RexxActivation::internalCallTrap(
    RexxInstruction * target,          /* target of the call                */
    RexxDirectory   * conditionObj )   /* processed condition object        */
/******************************************************************************/
/* Function:  Call an internal condition trap                                 */
/******************************************************************************/
{
  RexxActivation   *newActivation;     /* new activation for call           */
  size_t lineNum;                      /* source line number                */

  this->stack.push(conditionObj);      /* protect the condition object      */
  lineNum = this->current->getLine();  /* get the current line number       */
                                       /* initialize the SIGL variable      */
  this->setLocalVariable(OREF_SIGL, VARIABLE_SIGL, new_integer(lineNum));
                                       /* create a new activation           */
  newActivation = TheActivityClass->newActivation(this->receiver, this->settings.parent_method,
                 this->activity, this->settings.msgname, this, INTERNALCALL);
                                       /* set the new condition object      */
  newActivation->setConditionObj(conditionObj);
  this->activity->push(newActivation); /* push on the activity stack        */
                                       /* run the internal routine on the   */
                                       /* new activation                    */
  return newActivation->run(NULL, 0, target);
}



#ifdef NEWGUARD
BOOL RexxActivation::guardWait()
#else
void RexxActivation::guardWait()
#endif
/******************************************************************************/
/* Function:  Wait for a variable in a guard expression to get updated.       */
/******************************************************************************/
{
  INT   initial_state;                 /* initial guard state               */

  initial_state = this->object_scope;  /* save the initial state            */
                                       /* have the scope reserved?          */
  if (this->object_scope == SCOPE_RESERVED) {
                                       /* tell the receiver to release this */
    this->settings.object_variables->release(this->activity);
                                       /* and change our local state        */
    this->object_scope = SCOPE_RELEASED;    /* do an assignment! */
  }
  this->activity->guardWait();         /* wait on a variable inform event   */
#ifdef NEWGUARD
  return initial_state;
#endif
                                       /* did we release the scope?         */
  if (initial_state == SCOPE_RESERVED) {
                                       /* tell the receiver to reserve this */
    this->settings.object_variables->reserve(this->activity);
                                       /* and change our local state        */
    this->object_scope = SCOPE_RESERVED;    /* do an assignment! */
  }
}


#ifdef NEWGUARD
void RexxActivation::guardWaitScope(BOOL initial_state)
/******************************************************************************/
/* Function:  Wait for the variable scope after guard                         */
/******************************************************************************/
{
  if (initial_state == SCOPE_RESERVED) {
                                       /* tell the receiver to reserve this */
    this->settings.object_variables->reserve(this->activity);
                                       /* and change our local state        */
    this->object_scope == SCOPE_RESERVED;
  }
}
#endif



void RexxActivation::traceBack(
     RexxList   * traceback_list )     /* list of traceback items           */
/******************************************************************************/
/* Function:  Add the activation's current line to an error trace back list   */
/******************************************************************************/
{
  RexxSource   * source;               /* current method source             */
  RexxString   * line;                 /* traceback line                    */

  source = this->code->u_source;       /* get the source object             */
  if (source->traceable()) {           /* if we still have real source      */
    line = this->formatTrace(this->current, source);
    if (line != OREF_NULL)             /* have a real line?                 */
      traceback_list->addLast(line);   /* add the next traceback item       */
  }
}


RexxDateTime RexxActivation::getTime()
/******************************************************************************/
/* Function:  Retrieve the current activation timestamp, retrieving a new     */
/*            timestamp if this is the first call for a clause                */
/******************************************************************************/
{
                                       /* not a valid time stamp?           */
  if (!this->settings.timestamp.valid)
  {
                                       /* get a fresh time stamp            */
    SysGetCurrentTime(&this->settings.timestamp);
                                       /* got a new one                     */
    this->settings.timestamp.valid = true;
  }
                                       /* return the current time           */
  return this->settings.timestamp;
}


int64_t RexxActivation::getElapsed()
/******************************************************************************/
/* Function:  Retrieve the current elapsed time counter start time, starting  */
/*            the counter from the current time stamp if this is the first    */
/*            call                                                            */
/******************************************************************************/
{
  // no active elapsed time clock yet?
  if (this->settings.elapsed_time == 0)
  {

      settings.elapsed_time = settings.timestamp.getBaseTime();
  }
  return settings.elapsed_time;
}

void RexxActivation::resetElapsed()     /* reset activation elapsed time     */
/******************************************************************************/
/* Function:  Retrieve the current elapsed time counter start time, starting  */
/*            the counter from the current time stamp if this is the first    */
/*            call                                                            */
/******************************************************************************/
{
                                       /* turn on the reset flag            */
  this->settings.elapsed_time = settings.timestamp.getBaseTime();
}

#define DEFAULT_MIN 0                  /* default random minimum value      */
#define DEFAULT_MAX 999                /* default random maximum value      */
#define MAX_DIFFERENCE 100000          /* max spread between min and max    */


ULONG RexxActivation::getRandomSeed(
  RexxInteger * seed )                 /* user specified seed               */
/******************************************************************************/
/* Function:  Return the current random seed                                  */
/******************************************************************************/
{
  LONG  seed_value;                    /* supplied seed value               */
  INT   i;                             /* loop counter                      */

                                       /* currently in an internal routine  */
                                       /* or interpret instruction?         */
  if (this->activation_context&INTERNAL_LEVEL_CALL)
                                       /* forward along                     */
    return this->sender->getRandomSeed(seed);

  if (seed != OREF_NULL) {             /* have a seed supplied?             */
    seed_value = seed->value;          /* get the value                     */
    if (seed_value < 0)                /* negative value?                   */
                                       /* got an error                      */
      report_exception3(Error_Incorrect_call_nonnegative, new_cstring(CHAR_RANDOM), IntegerThree, seed);
                                       /* set the saved seed value          */
    this->random_seed = (ULONG)seed_value;
                                       /* flip all of the bits              */
    this->random_seed = this->random_seed^(0xffffffff);
    for (i = 0; i < 13; i++) {         /* randomize the seed number a bit   */
                                       /* scramble the seed a bit           */
      this->random_seed = RANDOMIZE(this->random_seed);
    }
  }
                                       /* step the randomization            */
  this->random_seed = RANDOMIZE(this->random_seed);
                                       /* set the seed at the activity level*/
  this->activity->setRandomSeed(this->random_seed);
  return this->random_seed;            /* return the seed value             */
}


RexxInteger * RexxActivation::random(
  RexxInteger * randmin,               /* RANDOM minimum range              */
  RexxInteger * randmax,               /* RANDOM maximum range              */
  RexxInteger * randseed )             /* RANDOM seed                       */
/******************************************************************************/
/* Function:  Process the random function, using the current activation       */
/*            seed value.                                                     */
/******************************************************************************/
{
   LONG  minimum;                      /* minimum value                     */
   LONG  maximum;                      /* maximum value                     */
   ULONG work;                         /* working random number             */
   ULONG seed;                         /* copy of the seed number           */
   INT   i;                            /* loop counter                      */

                                       /* go get the seed value             */
   seed = this->getRandomSeed(randseed);

   minimum = DEFAULT_MIN;              /* get the default MIN value         */
   maximum = DEFAULT_MAX;              /* get the default MAX value         */
   if (randmin != OREF_NULL) {         /* minimum specified?                */
     if ((randmax == OREF_NULL) &&     /* no maximum value specified        */
        (randseed == OREF_NULL))       /* and no seed specified             */
       maximum = randmin->value;       /* this is actually a max value      */
     else if ((randmin != OREF_NULL) &&/* minimum value specified           */
              (randmax == OREF_NULL) &&/* maximum value not specified       */
              (randseed != OREF_NULL)) /* seed specified                    */
       minimum = randmin->value;
     else {
       minimum = randmin->value;       /* give both max and min values      */
       maximum = randmax->value;
     }
   }
   else if (randmax != OREF_NULL)      /* only given a maximum?             */
     maximum = randmax->value;         /* use the supplied maximum          */

   if (minimum < 0)                    /* minimum too small?                */
     report_exception3(Error_Incorrect_call_nonnegative, new_cstring(CHAR_RANDOM), IntegerOne, randmin);
   if (maximum < 0)                    /* maximum too small?                */
     report_exception3(Error_Incorrect_call_nonnegative, new_cstring(CHAR_RANDOM), IntegerTwo, randmax);
   if (maximum < minimum)              /* range problem?                    */
                                       /* this is an error                  */
     report_exception2(Error_Incorrect_call_random, randmin, randmax);
                                       /* to big of a spread ?              */
   if (maximum - minimum > MAX_DIFFERENCE)
                                       /* this is an error                  */
     report_exception2(Error_Incorrect_call_random_range, randmin, randmax);

   if (minimum != maximum) {           /* have real work to do?             */
     work = 0;                         /* start with zero                   */
     for (i = 0; i < 32; i++) {        /* loop through all the bits         */
       work <<= 1;                     /* shift working num left one        */
                                       /* add in next seed bit value        */
       work = work | (seed & 0x00000001);
       seed >>= 1;                     /* shift off the right most seed bit */
     }
                                       /* adjust for requested range        */
     minimum += (work % (maximum - minimum + 1));
   }
   return new_integer(minimum);        /* return the random number          */
}

static PCHAR trace_prefix_table[] = {  /* table of trace prefixes           */
  "*-*",                               /* TRACE_PREFIX_CLAUSE               */
  "+++",                               /* TRACE_PREFIX_ERROR                */
  ">>>",                               /* TRACE_PREFIX_RESULT               */
  ">.>",                               /* TRACE_PREFIX_DUMMY                */
  ">V>",                               /* TRACE_PREFIX_VARIABLE             */
  ">E>",                               /* TRACE_PREFIX_DOTVARIABLE          */
  ">L>",                               /* TRACE_PREFIX_LITERAL              */
  ">F>",                               /* TRACE_PREFIX_FUNCTION             */
  ">P>",                               /* TRACE_PREFIX_PREFIX               */
  ">O>",                               /* TRACE_PREFIX_OPERATOR             */
  ">C>",                               /* TRACE_PREFIX_COMPOUND             */
  ">M>",                               /* TRACE_PREFIX_MESSAGE              */
  ">A>",                               /* TRACE_PREFIX_ARGUMENT             */
};

                                       /* extra space required to format a  */
                                       /* result line.  This overhead is    */
                                       /* 6 leading spaces for the line     */
                                       /* number, + 1 space + length of the */
                                       /* message prefix (3) + 1 space +    */
                                       /* 2 for an indent + 2 for the       */
                                       /* quotes surrounding the value      */
#define TRACE_OVERHEAD 15
                                       /* overhead for a traced instruction */
                                       /* (6 digit line number, blank,      */
                                       /* 3 character prefix, and a blank   */
#define INSTRUCTION_OVERHEAD 11
#define LINENUMBER 6                   /* size of a line number             */
#define PREFIX_OFFSET (LINENUMBER + 1) /* location of the prefix field      */
#define PREFIX_LENGTH 3                /* length of the prefix flag         */
#define INDENT_SPACING 2               /* spaces per indentation amount     */
// marker used for tagged traces to separate tag from the value
#define VALUE_MARKER " => "
// over head for adding quotes
#define QUOTES_OVERHEAD 2


void RexxActivation::traceValue(       /* trace an intermediate value       */
     RexxObject * value,               /* value to trace                    */
     int          prefix )             /* traced result type                */
/******************************************************************************/
/* Function:  Trace an intermediate value or instruction result value         */
/******************************************************************************/
{
  RexxString * buffer;                 /* buffer for building result        */
  RexxString * stringvalue;            /* object string value               */
  long         outlength;              /* output length                     */

                                       /* tracing currently suppressed or   */
                                       /* no value was received?            */
  if (this->settings.flags&trace_suppress || this->debug_pause || value == OREF_NULL)
    return;                            /* just ignore this call             */

  if (!this->source->traceable())      /* if we don't have real source      */
    return;                            /* just ignore for this              */
                                       /* get the string version            */
  stringvalue = value->stringValue();
  if (this->settings.traceindent < 0)  /* indentation go negative somehow?  */
    this->settings.traceindent = 0;    /* reset to zero                     */
                                       /* get a string large enough to      */
  outlength = stringvalue->length + TRACE_OVERHEAD + this->settings.traceindent * INDENT_SPACING;
  buffer = raw_string(outlength);      /* get an output string              */
  save(buffer);                        /* needs protection, as a following clone can force GC THU021A*/
                                       /* insert the leading blanks         */
  buffer->set(0, ' ', TRACE_OVERHEAD + this->settings.traceindent * INDENT_SPACING);
                                       /* add the trace prefix              */
  buffer->put(PREFIX_OFFSET, trace_prefix_table[prefix], PREFIX_LENGTH);
                                       /* add a quotation mark              */
  buffer->putChar(TRACE_OVERHEAD - 2 + this->settings.traceindent * INDENT_SPACING, '\"');
                                       /* copy the string value             */
  buffer->put(TRACE_OVERHEAD - 1 + this->settings.traceindent * INDENT_SPACING, stringvalue->stringData, stringvalue->length);
  buffer->putChar(outlength - 1, '\"');/* add the trailing quotation mark   */
  buffer->generateHash();              /* done building the string          */
                                       /* write out the line                */
  this->activity->traceOutput(this, buffer);
  discard(buffer);
}


/**
 * Trace an entry that's of the form 'tag => "value"'.
 *
 * @param prefix    The trace prefix tag to use.
 * @param tagPrefix Any prefix string added to the tag.  Use mostly for adding
 *                  the "." to traced environment variables.
 * @param quoteTag  Indicates whether the tag should be quoted or not.  Operator
 *                  names are quoted.
 * @param tag       The tag name.
 * @param value     The associated trace value.
 */
void RexxActivation::traceTaggedValue(int prefix, stringchar_t *tagPrefix, bool quoteTag,
     RexxString *tag, RexxObject * value)
{
    // the trace settings would normally require us to trace this, but there are conditions
    // where we just skip doing this anyway.
    if (this->settings.flags&trace_suppress || this->debug_pause || value == OREF_NULL || !source->traceable())
    {
        return;
    }

    // get the string value from the traced object.
    RexxString *stringValue = value->stringValue();
    // protect against negative indent values (belt and braces)
    if (this->settings.traceindent < 0)
    {

        this->settings.traceindent = 0;
    }

    // now calculate the length of the traced string
    stringsize_t outLength = tag->getLength() + stringValue->getLength();
    // these are fixed overheads
    outLength += TRACE_OVERHEAD + strlen(VALUE_MARKER);
    // now the indent spacing
    outLength += this->settings.traceindent * INDENT_SPACING;
    // now other conditionals
    outLength += quoteTag ? QUOTES_OVERHEAD : 0;
    // this is usually null, but dot variables add a "." to the tag.
    outLength += tagPrefix == NULL ? 0 : strlen((char *)tagPrefix);

    // now get a buffer to write this out into
    RexxString *buffer = raw_string(outLength);
    save(buffer);

    // get a cursor for filling in the formatted data
    stringsize_t dataOffset = TRACE_OVERHEAD + this->settings.traceindent * INDENT_SPACING - 2;
                                       /* insert the leading blanks         */
    buffer->set(0, ' ', TRACE_OVERHEAD + this->settings.traceindent * INDENT_SPACING);
                                       /* add the trace prefix              */
    buffer->put(PREFIX_OFFSET, trace_prefix_table[prefix], PREFIX_LENGTH);

    // if this is a quoted tag (operators do this), add quotes before coping the tag
    if (quoteTag)
    {
        buffer->putChar(dataOffset, '\"');
        dataOffset++;
    }
    // is the tag prefixed?  Add this before the name
    if (tagPrefix != NULL)
    {
        stringsize_t prefixLength = strlen((char *)tagPrefix);
        buffer->put(dataOffset, tagPrefix, prefixLength);
        dataOffset += prefixLength;
    }

    // add in the tag name
    buffer->put(dataOffset, tag);
    dataOffset += tag->getLength();

    // might need a closing quote.
    if (quoteTag)
    {
        buffer->putChar(dataOffset, '\"');
        dataOffset++;
    }

    // now add the data marker
    buffer->put(dataOffset, VALUE_MARKER, strlen(VALUE_MARKER));
    dataOffset += strlen(VALUE_MARKER);

    // the leading quote around the value
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    // the traced value
    buffer->put(dataOffset, stringValue);
    dataOffset += stringValue->getLength();

    // and finally, the trailing quote
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    buffer->generateHash();              /* done building the string          */
                                       /* write out the line                */
    this->activity->traceOutput(this, buffer);
    discard(buffer);
}


/**
 * Trace an entry that's of the form 'tag => "value"'.
 *
 * @param prefix    The trace prefix tag to use.
 * @param tagPrefix Any prefix string added to the tag.  Use mostly for adding
 *                  the "." to traced environment variables.
 * @param quoteTag  Indicates whether the tag should be quoted or not.  Operator
 *                  names are quoted.
 * @param tag       The tag name.
 * @param value     The associated trace value.
 */
void RexxActivation::traceOperatorValue(int prefix, const char *tag, RexxObject *value)
{
    // the trace settings would normally require us to trace this, but there are conditions
    // where we just skip doing this anyway.
    if (this->settings.flags&trace_suppress || this->debug_pause || value == OREF_NULL || !source->traceable())
    {
        return;
    }

    // get the string value from the traced object.
    RexxString *stringValue = value->stringValue();
    // protect against negative indent values (belt and braces)
    if (this->settings.traceindent < 0)
    {

        this->settings.traceindent = 0;
    }

    // now calculate the length of the traced string
    stringsize_t outLength = strlen(tag) + stringValue->getLength();
    // these are fixed overheads
    outLength += TRACE_OVERHEAD + strlen(VALUE_MARKER);
    // now the indent spacing
    outLength += this->settings.traceindent * INDENT_SPACING;
    // now other conditionals
    outLength += QUOTES_OVERHEAD;

    // now get a buffer to write this out into
    RexxString *buffer = raw_string(outLength);
    save(buffer);

    // get a cursor for filling in the formatted data
    stringsize_t dataOffset = TRACE_OVERHEAD + this->settings.traceindent * INDENT_SPACING - 2;
                                       /* insert the leading blanks         */
    buffer->set(0, ' ', TRACE_OVERHEAD + this->settings.traceindent * INDENT_SPACING);
                                       /* add the trace prefix              */
    buffer->put(PREFIX_OFFSET, trace_prefix_table[prefix], PREFIX_LENGTH);

    // operators are quoted.
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    // add in the tag name
    buffer->put(dataOffset, tag, strlen(tag));
    dataOffset += strlen(tag);

    // need a closing quote.
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    // now add the data marker
    buffer->put(dataOffset, VALUE_MARKER, strlen(VALUE_MARKER));
    dataOffset += strlen(VALUE_MARKER);

    // the leading quote around the value
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    // the traced value
    buffer->put(dataOffset, stringValue);
    dataOffset += stringValue->getLength();

    // and finally, the trailing quote
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    buffer->generateHash();              /* done building the string          */
                                       /* write out the line                */
    this->activity->traceOutput(this, buffer);
    discard(buffer);
}


/**
 * Trace a compound variable entry that's of the form 'tag =>
 * "value"'.
 *
 * @param prefix    The trace prefix tag to use.
 * @param stem      The stem name of the compound.
 * @param tails     The array of tail elements (unresolved).
 * @param tailCount The count of tail elements.
 * @param value     The associated trace value.
 */
void RexxActivation::traceCompoundValue(int prefix, RexxString *stem, RexxObject **tails, size_t tailCount,
     RexxObject * value)
{
    // the trace settings would normally require us to trace this, but there are conditions
    // where we just skip doing this anyway.
    if (this->settings.flags&trace_suppress || this->debug_pause || value == OREF_NULL || !source->traceable())
    {
        return;
    }

    // get the string value from the traced object.
    RexxString *stringValue = value->stringValue();
    // protect against negative indent values (belt and braces)
    if (this->settings.traceindent < 0)
    {

        this->settings.traceindent = 0;
    }


    // now calculate the length of the traced string
    stringsize_t outLength = stem->getLength() + stringValue->getLength();

    // build an unresolved tail name
    RexxCompoundTail tail(tails, tailCount, false);

    outLength += tail.getLength();

    // add in the number of added dots
    outLength += tailCount - 1;

    // these are fixed overheads
    outLength += TRACE_OVERHEAD + strlen(VALUE_MARKER);
    // now the indent spacing
    outLength += this->settings.traceindent * INDENT_SPACING;

    // now get a buffer to write this out into
    RexxString *buffer = raw_string(outLength);
    save(buffer);

    // get a cursor for filling in the formatted data
    stringsize_t dataOffset = TRACE_OVERHEAD + this->settings.traceindent * INDENT_SPACING - 2;
                                       /* insert the leading blanks         */
    buffer->set(0, ' ', TRACE_OVERHEAD + this->settings.traceindent * INDENT_SPACING);
                                       /* add the trace prefix              */
    buffer->put(PREFIX_OFFSET, trace_prefix_table[prefix], PREFIX_LENGTH);

    // add in the stem name
    buffer->put(dataOffset, stem);
    dataOffset += stem->getLength();

    // copy the tail portion of the compound name
    buffer->put(dataOffset, tail.getTail(), tail.getLength());
    dataOffset += tail.getLength();

    // now add the data marker
    buffer->put(dataOffset, VALUE_MARKER, strlen(VALUE_MARKER));
    dataOffset += strlen(VALUE_MARKER);

    // the leading quote around the value
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    // the traced value
    buffer->put(dataOffset, stringValue);
    dataOffset += stringValue->getLength();

    // and finally, the trailing quote
    buffer->putChar(dataOffset, '\"');
    dataOffset++;

    buffer->generateHash();              /* done building the string          */
                                       /* write out the line                */
    this->activity->traceOutput(this, buffer);
    discard(buffer);
}


void RexxActivation::traceSourceString()
/******************************************************************************/
/* Function:  Trace the source string at debug mode start                     */
/******************************************************************************/
{
  RexxString * buffer;                 /* buffer for building result        */
  RexxString * string;                 /* the source string                 */
  long         outlength;              /* output length                     */

                                       /* already traced?                   */
  if (this->settings.flags&source_traced)
    return;                            /* don't do it again                 */
                                       /* tag this as traced                */
  this->settings.flags |= source_traced;
                                       /* get the string version            */
  string = this->sourceString();       /* get the source string             */
                                       /* get a string large enough to      */
  outlength = string->length + INSTRUCTION_OVERHEAD + 2;
  buffer = raw_string(outlength);      /* get an output string              */
                                       /* insert the leading blanks         */
  buffer->set(0, ' ', INSTRUCTION_OVERHEAD);
                                       /* add the trace prefix              */
  buffer->put(PREFIX_OFFSET, trace_prefix_table[TRACE_PREFIX_ERROR], PREFIX_LENGTH);
                                       /* add a quotation mark              */
  buffer->putChar(INSTRUCTION_OVERHEAD, '\"');
                                       /* copy the string value             */
  buffer->put(INSTRUCTION_OVERHEAD + 1, string->stringData, string->length);
  buffer->putChar(outlength - 1, '\"');/* add the trailing quotation mark   */
  buffer->generateHash();              /* done building the string          */
                                       /* write out the line                */
  this->activity->traceOutput(this, buffer);
}


RexxString * RexxActivation::formatTrace(
   RexxInstruction *  instruction,     /* instruction to trace              */
   RexxSource      *  source )         /* program source                    */
/******************************************************************************/
/* Function:  Format a source line for traceback or tracing                   */
/******************************************************************************/
{
  LOCATIONINFO  location;              /* location of the clause            */

  if (instruction == OREF_NULL)        /* no current instruction?           */
    return OREF_NULL;                  /* nothing to trace here             */
  instruction->getLocation(&location); /* get the instruction location      */
  if (this->settings.traceindent < 0)  /* indentation go negative somehow?  */
    this->settings.traceindent = 0;    /* reset to zero                     */
                                       /* extract the source string         */
                                       /* (formatted for tracing)           */
  if (this->settings.traceindent < MAX_TRACEBACK_INDENT)
      return source->traceBack(&location, this->settings.traceindent, TRUE);
  else return source->traceBack(&location, MAX_TRACEBACK_INDENT, TRUE);
}

                                       /* process clause boundary stuff     */
void RexxActivation::processClauseBoundary()
/******************************************************************************/
/* Function:  Handle all clause boundary processing (raising of halt          */
/*            conditions, turning on of external traces, and calling of halt  */
/*            and trace clause boundary exits                                 */
/******************************************************************************/
{
  if (this->pending_count != 0)        /* do we have trapped conditions?    */
    this->processTraps();              /* go dispatch the traps             */

  this->activity->sysExitHltTst(this); /* Sys exit want to raise a halt?    */
                                       /* did sysexit change trace state    */
  if (!this->activity->sysExitTrcTst(this, this->isExternalTraceOn())) {
                                       /* remember new state...             */
    if (this->isExternalTraceOn())     /* if current setting is on          */
      this->setExternalTraceOff();     /* turn it off                       */
    else                               /* otherwise                         */
      this->setExternalTraceOn();      /* turn it on                        */
  }
                                       /* yield situation occurred?         */
  if (this->settings.flags&external_yield) {
                                       /* turn off the yield flag           */
    this->settings.flags &= ~external_yield;
    this->activity->relinquish();      /* yield control to the activity     */
  }
                                       /* halt condition occurred?          */
  if (this->settings.flags&halt_condition) {
                                       /* turn off the halt flag            */
    this->settings.flags &= ~halt_condition;
                                       /* yes, raise the flag               */
    report_halt(this->settings.halt_description);
  }
                                       /* need to turn on tracing?          */
  if (this->settings.flags&set_trace_on) {
                                       /* turn off the trace flag           */
    this->settings.flags &= ~set_trace_on;
    this->setExternalTraceOn();        /* and save the current state        */
                                       /* turn on tracing                   */
    this->setTrace(TRACE_RESULTS, DEBUG_ON);
  }
                                       /* need to turn off tracing?         */
  if (this->settings.flags&set_trace_off) {
                                       /* turn off the trace flag           */
    this->settings.flags &= ~set_trace_off;
    this->setExternalTraceOff();       /* and save the current state        */
                                       /* turn on tracing                   */
    this->setTrace(TRACE_OFF, DEBUG_OFF);
  }
                                       /* no clause exits and all conditions*/
                                       /* have been processed?              */
  if (!(this->settings.flags&clause_exits) && this->pending_count == 0)
                                       /* turn off boundary processing      */
    this->settings.flags &= ~clause_boundary;
}


void RexxActivation::halt(             /* turn on a HALT condition          */
   RexxString * description )          /* HALT condition description        */
/******************************************************************************/
/* Function:  Flip on the HALT condition bit and store the HALT condition     */
/*            descriptive string (which may be OREF_NULL)                     */
/******************************************************************************/
{
                                       /* store the description             */
  this->settings.halt_description = description;
                                       /* turn on the HALT flag             */
  this->settings.flags |= halt_condition;
                                       /* turn on clause boundary checking  */
  this->settings.flags |= clause_boundary;

}

void RexxActivation::yield()
/******************************************************************************/
/* Function:  Flip ON the externally activated TRACE bit.                     */
/******************************************************************************/
{
                                       /* turn on the yield flag            */
  this->settings.flags |= external_yield;
                                       /* turn on clause boundary checking  */
  this->settings.flags |= clause_boundary;
}

void RexxActivation::externalTraceOn()
/******************************************************************************/
/* Function:  Flip ON the externally activated TRACE bit.                     */
/******************************************************************************/
{
  this->settings.flags |= set_trace_on;/* turn on the tracing flag          */
                                       /* turn on clause boundary checking  */
  this->settings.flags |= clause_boundary;
                                       /* turn on tracing                   */
  this->setTrace(TRACE_RESULTS, DEBUG_ON);
}

void RexxActivation::externalTraceOff()
/******************************************************************************/
/* Function:  Flip OFF the externally activated TRACE bit.                    */
/******************************************************************************/
{
                                       /* turn off the tracing flag         */
  this->settings.flags |= set_trace_off;
                                       /* turn on clause boundary checking  */
  this->settings.flags |= clause_boundary;
}

void RexxActivation::externalDbgStepIn()
{
    this->settings.dbg_flags = dbg_trace;
    this->settings.flags |= clause_boundary;
}

void RexxActivation::externalDbgStepOver()
{
    this->settings.dbg_flags = dbg_stepover;
    this->settings.flags |= clause_boundary;
}

void RexxActivation::externalDbgStepOut()
{
    this->settings.dbg_flags = dbg_stepout;
    this->settings.flags |= clause_boundary;
}

void RexxActivation::externalDbgEndStepO()
{
    this->settings.dbg_flags = dbg_stepout | dbg_endstep;
    this->settings.flags |= clause_boundary;
}

void RexxActivation::externalDbgStepAgain()
{
    this->settings.dbg_flags = dbg_trace | dbg_stepagain;
    this->settings.flags |= clause_boundary;
}

BOOL RexxActivation::debugPause(RexxInstruction * instr)
/******************************************************************************/
/* Function:  Process an individual debug pause for an instruction            */
/******************************************************************************/
{
  RexxString       * response;         /* response to the debug prompt      */
  RexxInstruction  * current;          /* current instruction               */

  if (this->debug_pause)               /* instruction during debug pause?   */
    return FALSE;                      /* just get out quick                */

  if (this->settings.flags&debug_bypass)
                                       /* turn off for the next time        */
    this->settings.flags &= ~debug_bypass;
                                       /* debug pauses suppressed?          */
  else if (this->settings.trace_skip > 0) {
    this->settings.trace_skip--;       /* account for this one              */
    if (this->settings.trace_skip == 0)/* gone to zero?                     */
                                       /* turn tracing back on again (this  */
                                       /* ensures the next pause also has   */
                                       /* the instruction traced            */
    this->settings.flags &= ~trace_suppress;
  }
  else {                               /* real work to do                   */
    if (!this->source->traceable())    /* if we don't have real source      */
      return FALSE;                    /* just ignore for this              */
                                       /* newly into debug mode?            */
    if (!(this->settings.flags&debug_prompt_issued)) {
                                       /* write the initial prompt          */
      this->activity->traceOutput(this, (RexxString *)SysMessageText(Message_Translations_debug_prompt));
                                       /* remember we've issued this        */
      this->settings.flags |= debug_prompt_issued;
    }
    current = this->next;              /* save the next location target     */
    for (;;) {                         /* loop until no longer pausing      */
                                       /* read a line from the screen       */
      this->callDbgLineLocate(instr);  /* call the debug exit with line information */
      do {
         response = this->activity->traceInput(this);
         this->callDbgExit();             /* call the debug exit to check for stepin/out/over */
      } while (this->settings.dbg_flags&dbg_stepagain && !(this->settings.flags&halt_condition));
      /* dbg_stepagain is set, when a tracepoint is set immediately after a step_out/step_over */


      if (response->length == 0)       /* just a "null" line entered?       */
        break;                         /* just end the pausing              */
                                       /* a re-execute request?             */
      else if (response->length == 1 && response->getChar(0) == '=') {
        this->next = this->current;    /* reset the execution pointer       */
        return TRUE;                   /* finished (inform block instrs)    */
      }
      else {                           /* need to execute an instruction    */
        this->debugInterpret(response);/* go execute this                   */
        if (current != this->next)     /* flow of control change?           */
          break;                       /* end of this pause                 */
                                       /* has the use changed the trace     */
                                       /* setting on us?                    */
        else if (this->settings.flags&debug_bypass) {
                                       /* turn off for the next time        */
          this->settings.flags &= ~debug_bypass;
          break;                       /* we also skip repausing            */
        }
      }
    }
  }
  return FALSE;                        /* no re-execute                     */
}

void RexxActivation::traceClause(      /* trace a REXX instruction          */
     RexxInstruction * clause,         /* value to trace                    */
     int               prefix )        /* prefix to use                     */
/******************************************************************************/
/* Function:  Trace an individual line of a source file                       */
/******************************************************************************/
{
  RexxString  *line;                   /* actual line data                  */

                                       /* tracing currently suppressed?     */
  if (this->settings.flags&trace_suppress || this->debug_pause)
    return;                            /* just ignore this call             */
  if (!this->source->traceable())      /* if we don't have real source      */
    return;                            /* just ignore for this              */
                                       /* format the line                   */
  line = this->formatTrace(clause, this->code->u_source);
  if (line != OREF_NULL) {             /* have a source line?               */
                                       /* newly into debug mode?            */
    if ((this->settings.flags&trace_debug && !(this->settings.flags&debug_prompt_issued))
        || this->settings.dbg_flags&dbg_trace)
      this->traceSourceString();       /* trace the source string           */
                                       /* write out the line                */
    this->activity->traceOutput(this, line);
  }
}

RexxObject * RexxActivation::command(
     RexxString * command,             /* command to issue                  */
     RexxString * address )            /* target address environment        */
/******************************************************************************/
/* Function:  Issue a command to a host environment                           */
/******************************************************************************/
{
  RexxObject * rc;                     /* return code                       */
  RexxString * condition;              /* raised conditions                 */
  RexxString * rc_trace;               /* traced return code                */
  BOOL         instruction_traced;     /* instruction has been traced       */

                                       /* instruction already traced?       */
  if ((this->settings.flags&trace_all) || (this->settings.flags&trace_commands))
    instruction_traced = TRUE;         /* remember we traced this           */
  else
    instruction_traced = FALSE;        /* not traced yet                    */
                                       /* if exit declines call             */
  if (this->activity->sysExitCmd(this, command, address, &condition, &rc))
                                       /* go issue the command              */
    rc = SysCommand(this, this->activity, address, command, &condition);
  this->stack.push(rc);                /* save on the expression stack      */
  if (!this->debug_pause) {            /* not a debug pause?                */
                                       /* set the RC variable to the        */
                                       /* command return value              */
    this->setLocalVariable(OREF_RC, VARIABLE_RC, rc);
                                       /* tracing command errors or fails?  */
    if ((condition == OREF_ERRORNAME && (this->settings.flags&trace_errors)) ||
        (condition == OREF_FAILURENAME && (this->settings.flags&trace_failures))) {
                                       /* trace the current instruction     */
      this->traceClause(this->current, TRACE_PREFIX_CLAUSE);
                                       /* then we always trace full command */
      this->traceValue(command, TRACE_PREFIX_RESULT);
      instruction_traced = TRUE;       /* we've now traced this             */
    }
                                       /* need to trace the RC info too?    */
    if (instruction_traced && rc->longValue(9) != 0) {
                                       /* get RC as a string                */
      rc_trace = rc->stringValue();
                                       /* tack on the return code           */
      rc_trace = rc_trace->concatToCstring("RC(");
                                       /* add the closing part              */
      rc_trace = rc_trace->concatWithCstring(")");
                                       /* trace the return code             */
      this->traceValue(rc_trace, TRACE_PREFIX_ERROR);
    }
                                       /* now have a return status          */
    this->settings.flags |= return_status_set;
    this->settings.return_status = 0;  /* set default return status         */
                                       /* have a failure condition?         */
    if (condition == OREF_FAILURENAME) {
                                       /* return status is negative         */
      this->settings.return_status = -1;
                                       /* try to raise the condition        */
      if (!CurrentActivity->raiseCondition(condition, rc, command, OREF_NULL, OREF_NULL, OREF_NULL))
                                       /* try to raise an error condition   */
        CurrentActivity->raiseCondition(OREF_ERRORNAME, rc, command, OREF_NULL, OREF_NULL, OREF_NULL);
    }
                                       /* have an error condition?          */
    else if (condition == OREF_ERRORNAME) {
      this->settings.return_status = 1;/* return status is positive         */
                                       /* try to raise the condition        */
      CurrentActivity->raiseCondition(condition, rc, command, OREF_NULL, OREF_NULL, OREF_NULL);
    }
  }
                                       /* do debug pause if necessary       */
                                       /* necessary is defined by:  we are  */
                                       /* tracing ALL or COMMANDS, OR, we   */
                                       /* using TRACE NORMAL and a FAILURE  */
                                       /* return code was received OR we are*/
                                       /* receive an ERROR return code and  */
                                       /* have TRACE ERROR in effect.       */
  if (instruction_traced && this->settings.flags&trace_debug)
    this->debugPause();                /* do the debug pause                */
  return rc;
}

RexxString * RexxActivation::programName()
/******************************************************************************/
/* Function:  Return the name of the current program file                     */
/******************************************************************************/
{
  return this->code->getProgramName(); /* get the name from the code        */
}

RexxDirectory * RexxActivation::getLabels()
/******************************************************************************/
/* Function:  Return the directory of labels for this method                  */
/******************************************************************************/
{
  return this->code->labels;           /* get the labels from the code      */
}

RexxString * RexxActivation::sourceString()
/******************************************************************************/
/* Function:  Create the source string returned by parse source               */
/******************************************************************************/
{
                                       /* produce the system specific string*/
  return SysSourceString(this->settings.calltype, this->code->getProgramName());
}


void RexxActivation::setObjNotify(
     RexxMessage    * notify)          /* activation to notify              */
/******************************************************************************/
/* Function:  Set an error notification tag on the activation.                */
/******************************************************************************/
{
  this->objnotify = notify;
}


void RexxActivation::setDBCS(
     BOOL  setting )                   /* DBCS ON/OFF flag                  */
/******************************************************************************/
/* Function:  Set the DBCS string processing state.                           */
/******************************************************************************/
{
                                       /* set the flag indicator            */
  this->settings.global_settings.exmode = setting;
}

void RexxActivation::pushEnvironment(
     RexxObject * environment)         /* new local environment buffer        */
/******************************************************************************/
/* Function:  Push the new environment buffer onto the EnvLIst                */
/******************************************************************************/
{
                                       /* internal call or interpret?         */
  if (this->activation_context&TOP_LEVEL_CALL) {
                                       /* nope, push environment here.        */
                                       /* DO we have a environment list?      */
    if (!this->environmentList) {
                                       /* nope, create one                    */
      this->environmentList = new_list();

    }
    this->environmentList->addFirst(environment);
  }
  else {                               /* nope, process up the chain.         */
                                       /* Yes, forward on the message.        */
    this->sender->pushEnvironment(environment);
  }
}

RexxObject * RexxActivation::popEnvironment()
/******************************************************************************/
/* Function:  return the top level local Environemnt                          */
/******************************************************************************/
{
                                       /* internal call or interpret?         */
  if (this->activation_context&TOP_LEVEL_CALL) {
                                       /* nope, we puop Environemnt here      */
                                       /* DO we have a environment list?      */
    if (this->environmentList) {
                                       /* yup, return first element           */
      return  this->environmentList->removeFirst();

    }
    else                               /* nope, return .nil                   */
      return TheNilObject;
  }
  else {                               /* nope, pass on up the chain.         */
                                       /* Yes, forward on the message.        */
     return this->sender->popEnvironment();
  }
}

void RexxActivation::closeStreams()
/******************************************************************************/
/* Function:  Close any streams opened by the I/O builtin functions           */
/******************************************************************************/
{
  RexxDirectory *streams;              /* stream directory                  */
  RexxString    *index;                /* index for stream directory        */
  long j;                              /* position for stream directory     */

                                       /* exiting a bottom level?           */
  if (this->activation_context&PROGRAM_OR_METHOD) {
    streams = this->settings.streams;  /* get the streams directory         */
    if (streams != OREF_NULL) {        /* actually have a table?            */
                                       /* traverse this                     */
      for (j = streams->first(); (index = (RexxString *)streams->index(j)) != OREF_NULL; j = streams->next(j)) {
                                       /* closing each stream               */
        streams->at(index)->sendMessage(OREF_CLOSE);
      }
    }
  }
}


BOOL RexxActivation::callSecurityManager(
    RexxString    *methodName,         /* name of the security method       */
    RexxDirectory *arguments )         /* security argument directory       */
/******************************************************************************/
/* Function:  Invoke the security manager and return success/failure          */
/******************************************************************************/
{
  RexxObject * result;                 /* return result                     */

  this->stack.push(arguments);         /* protect arguments on stack        */
                                       /* call the security manager and     */
                                       /* return the pass/handled setting   */
  result = this->settings.securityManager->sendMessage(methodName, arguments);
  if (result == OREF_NULL)             /* no return result?                 */
                                       /* need to raise an exception        */
    report_exception1(Error_No_result_object_message, methodName);
  this->stack.pop();                   /* free up the arguments             */
  hold(arguments);                     /* protect them for a bit            */
                                       /* return the pass/handled flag      */
  return result->truthValue(Error_Logical_value_authorization);
}


void RexxActivation::sysDbgSubroutineCall(BOOL enter)
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the subroutine call debug system exit.                         */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RexxString   *filename;              /* Exit routine name                 */
  RexxString   *routine;               /* Exit routine name                 */
  RXDBGTST_PARM exit_parm;             /* exit parameters                   */
  LOCATIONINFO  location;              /* location of the clause            */

                                       /* get the exit handler              */
  exitname = activity->querySysExits(RXDBG);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */

    exit_parm.rxdbg_flags.rxftrace = 0;
    filename = this->code->getProgramName();
        MAKERXSTRING(exit_parm.rxdbg_filename, filename->stringData, filename->length);
    exit_parm.rxdbg_line = this->getCurrent()->lineNumber;
    this->getCurrent()->getLocation(&location); /* get the instruction location      */
    if (this->source)
    {
        routine = this->source->extract(&location);
        MAKERXSTRING(exit_parm.rxdbg_routine, routine->stringData, routine->length);
    }
    else
        MAKERXSTRING(exit_parm.rxdbg_routine, "no info available", 17);

                                       /* call the handler                  */
    SysExitHandler(activity, this, exitname, RXDBG, (enter ? RXDBGENTERSUB : RXDBGLEAVESUB), (PVOID)&exit_parm, FALSE);
  }
}

void RexxActivation::sysDbgLineLocate(RexxInstruction * instr)
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the locate line debug system exit.                             */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RexxString   *filename;              /* Exit routine name                 */
  RXDBGTST_PARM exit_parm;             /* exit parameters                   */

                                       /* get the exit handler              */
  exitname = activity->querySysExits(RXDBG);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */
    if (!this->code->u_source->traceable() || (this->code->u_source->flags&_interpret)
        || (this->code->u_source->sourceBuffer == OREF_NULL)) return;
    exit_parm.rxdbg_flags.rxftrace = 0;
    filename = this->code->getProgramName();
        MAKERXSTRING(exit_parm.rxdbg_filename, filename->stringData, filename->length);
    if (instr == OREF_NULL) instr = this->getCurrent();
    exit_parm.rxdbg_line = instr->lineNumber;
                                       /* call the handler                  */
    SysExitHandler(activity, this, exitname, RXDBG, RXDBGLOCATELINE, (PVOID)&exit_parm, FALSE);
  }
}


void RexxActivation::sysDbgSignal(void)
/******************************************************************************/
/* Function:   Calls the SysExitHandler method on the System Object to run    */
/*             the call stack signal system exit.                             */
/******************************************************************************/
{
  RexxString   *exitname;              /* Exit routine name                 */
  RXDBGTST_PARM exit_parm;       /* exit parameters                   */
                                       /* get the exit handler              */
  exitname = activity->querySysExits(RXDBG);
  if (exitname != OREF_NULL) {         /* exit enabled?                     */

    exit_parm.rxdbg_flags.rxftrace = 0;
    exit_parm.rxdbg_filename.strptr = OREF_NULL;
    exit_parm.rxdbg_line = 0;
    exit_parm.rxdbg_routine.strptr = OREF_NULL;
                                       /* call the handler                  */
    SysExitHandler(activity, this, exitname, RXDBG, RXDBGSIGNAL, (PVOID)&exit_parm, FALSE);
  }
}

RexxObject  *RexxActivation::novalueHandler(
     RexxString *name )                /* name to retrieve                  */
/******************************************************************************/
/* Function:  process unitialized variable over rides                         */
/******************************************************************************/
{
  RexxObject *novalue_handler;         /* external novalue handler          */

                                       /* get the handler from .local       */
  novalue_handler = CurrentActivity->local->fastAt(OREF_NOVALUE);
  if (novalue_handler != OREF_NULL)    /* have a novalue handler?           */
                                       /* ask it to process this            */
    return send_message1(novalue_handler, OREF_NOVALUE, name);
  return TheNilObject;                 /* return the handled result         */
}

RexxVariableBase  *RexxActivation::getVariableRetriever(
     RexxString  *variable )           /* name of the variable              */
/******************************************************************************/
/* Arguments:  Name of variable to generate retriever                         */
/*                                                                            */
/*  Returned:  Retriever for variable (returns OREF_NULL for invalids)        */
/******************************************************************************/
{
  INT         Type;                    /* type returned from string method  */
  RexxVariableBase *retriever;         /* created variable retriever        */

  variable = variable->upper();        /* upper case the variable           */
  Type = variable->isSymbol();         /* validate the symbol               */
  switch (Type) {                      /* create a retriever object         */
    case STRING_BAD_VARIABLE:          /* if it didn't validate             */
      retriever = OREF_NULL;           /* don't return a retriever object   */
      break;

    case STRING_LITERAL_DOT:           /* if is is a literal                */
    case STRING_NUMERIC:
                                       /* these are literals                */
      retriever = (RexxVariableBase *)variable;
      break;

    // Dot variables retrieve from the environment
    case STRING_LITERAL:
      retriever = (RexxVariableBase *)new RexxDotVariable(variable->extract(1, variable->getLength() - 1));
      break;

                                       /* if it is a stem                   */
    case STRING_STEM:
                                       /* create a new stem retriever       */
      retriever = (RexxVariableBase *)new RexxStemVariable(variable, 0);
      break;
                                       /* if it is a compound               */
    case STRING_COMPOUND_NAME:
                                       /* create a new compound retriever   */
      retriever = (RexxVariableBase *)buildCompoundVariable(variable, FALSE);
      break;
                                       /* if it is a simple                 */
    case STRING_NAME:
                                       /* create a new variable retriever   */
      retriever = (RexxVariableBase *)new RexxParseVariable(variable, 0);
      break;
                                       /* if we don't know what it is       */
    default:
      retriever = OREF_NULL;           /* don't return a retriever object   */
      break;
  }
  return retriever;                    /* return this retriever             */
}


RexxVariableBase  *RexxActivation::getDirectVariableRetriever(
     RexxString *variable )            /* name of the variable              */
/******************************************************************************/
/* Function:  Return a retriever for a variable using direct access (i.e.     */
/*            no substitution in compound variable tails)                     */
/******************************************************************************/
{
  LONG        scan;                    /* string scan pointer               */
  BOOL        literal;                 /* literal indicator                 */
  LONG        length;                  /* length of variable name           */
  RexxVariableBase *retriever;         /* created variable retriever        */
  UCHAR       character;               /* current character                 */
  LONG        nonnumeric;              /* count of non-numeric characters   */
  UCHAR       last;                    /* previous character                */
  INT         compound;                /* count of period characters        */

  retriever = OREF_NULL;               /* return NULL for all errors        */
  length = variable->length;           /* get the name length               */
                                       /* get the first character           */
  character = (UCHAR)variable->getChar(0);
                                       /* constant symbol?                  */
  if (character == '.' || (character >= '0' && character <= '9'))
    literal = TRUE;                    /* this is a literal value           */
  else
    literal = FALSE;                   /* not a literal value               */
                                       /* have a valid length?              */
  if (length <= MAX_SYMBOL_LENGTH && length > 0) {
    compound = 0;                      /* no periods yet                    */
    scan = 0;                          /* start at string beginning         */
    nonnumeric = 0;                    /* count of non-numeric characters   */
    last = 0;                          /* no last character                 */
    while (scan < length) {            /* while more to scan                */
                                       /* get the next character            */
      character = (UCHAR)variable->getChar(scan);
      if (character == '.') {          /* have a period?                    */
        if (!literal)                  /* not a literal value?              */
                                       /* don't process past here           */
          return (RexxVariableBase *)buildCompoundVariable(variable, TRUE);
        else
          compound++;                  /* count the character               */
      }
                                       /* may have a special character      */
      else if (lookup[character] == 0) {
                                       /* maybe exponential form?           */
        if (character == '+' || character == '-') {
                                       /* front part not valid?             */
          if (compound > 1 || nonnumeric > 1 || last != 'E')
            return OREF_NULL;          /* got a bad symbol                  */
          scan++;                      /* step over the sign                */
          if (scan >= length)          /* sign as last character?           */
            return OREF_NULL;          /* this is bad also                  */
          while (scan < length) {      /* scan remainder                    */
                                       /* get the next character            */
            character = (UCHAR)variable->getChar(scan);
                                       /* outside numeric range?            */
            if (character < '0' || character > '9')
              return OREF_NULL;        /* not valid either                  */
            scan++;                    /* step scan position                */
          }
          break;                       /* done with scanning                */
        }
      }
                                       /* non-numeric character?            */
      else if (character < '0' || character > '9')
        nonnumeric++;                  /* count the non-numeric             */
                                       /* lower case character?             */
      else if (lookup[character] != character)
        return OREF_NULL;              /* this is bad, return               */
      last = character;                /* remember last one                 */
      scan++;                          /* step the pointer                  */
    }
  }
  if (literal)                         /* was this a literal?               */
                                       /* these are both just literals      */
    retriever = (RexxVariableBase *)variable;
  else                                 /* simple variable                   */
                                       /* create a new variable retriever   */
    retriever = (RexxVariableBase *)new RexxParseVariable(variable, 0);
  return retriever;                    /* return this retriever             */
}


RexxObject *buildCompoundVariable(
    RexxString *variable_name,          /* full variable name of compound    */
    BOOL direct)                        /* this is direct access             */
/******************************************************************************/
/* Function:  Build a dynamically created compound variable                   */
/******************************************************************************/
{
  RexxString *   stem;                 /* stem part of compound variable    */
  RexxString *   tail;                 /* tail section string value         */
  RexxQueue  *   tails;                /* tail elements                     */
  RexxObject *   tailPart;             /* tail element retriever            */
  INT     position;                    /* scan position within compound name*/
  INT     start;                       /* starting scan position            */
  INT     length;                      /* length of tail section            */

  length = variable_name->length;      /* get the string length             */
  position = 0;                        /* start scanning at first character */
                                       /* scan to the first period          */
  while (variable_name->getChar(position) != '.') {
    position++;                        /* step to the next character        */
    length--;                          /* reduce the length also            */
  }
                                       /* extract the stem part             */
  stem = variable_name->extract(0, position + 1);
  save(stem);                          /* lock the stem part                */
                                       /* processing to decompose the name  */
                                       /* into its component parts          */

  tails = new_queue();                 /* get a new list for the tails      */
  save(tails);                         /* protect the stem name             */
  position++;                          /* step past previous period         */
  length--;                            /* adjust the length                 */
  if (direct == TRUE) {                /* direct access?                    */
                                       /* extract the tail part             */
    tail = variable_name->extract(position, length);
    tails->push(tail);                 /* add to the tail piece list        */
  }
  else {
    while (length > 0) {               /* process rest of the variable      */
      start = position;                /* save the start position           */
                                       /* scan for the next period          */
      while (length > 0 && variable_name->getChar(position) != '.') {
        position++;                    /* step to the next character        */
        length--;                      /* reduce the length also            */
      }
                                       /* extract the tail part             */
      tail = variable_name->extract(start, position - start);
                                       /* have a null tail piece or         */
                                       /* section begin with a digit?       */
      /* ASCII '0' to '9' to recognize a digit                              */
      if (tail->length == 0 || (tail->getChar(0) >= '0' && tail->getChar(0) <= '9'))
        tailPart = (RexxObject *)tail; /* this is a literal piece           */
      else {
                                       /* create a new variable retriever   */
          tailPart = (RexxObject *)new RexxParseVariable(tail, 0);
      }
      tails->push(tailPart);           /* add to the tail piece list        */
      position++;                      /* step past previous period         */
      length--;                        /* adjust the length                 */
    }
                                       /* have a trailing period?           */
    if (variable_name->getChar(position - 1) == '.')
      tails->push(OREF_NULLSTRING);    /* add to the tail piece list        */
  }
  discard_hold(stem);                  /* release the stem                  */
  discard_hold(tails);                 /* and the tails                     */
                                       /* create and return a new compound  */
  return (RexxObject *)new (tails->getSize()) RexxCompoundVariable(stem, 0, tails, tails->getSize());
}
