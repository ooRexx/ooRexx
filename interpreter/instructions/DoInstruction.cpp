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
/* REXX Translator                                                            */
/*                                                                            */
/* Primitive Do Parse Class                                                   */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "RexxActivity.hpp"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"
#include "EndInstruction.hpp"
#include "Token.hpp"
#include "SourceFile.hpp"
#include "ExpressionBaseVariable.hpp"

void RexxInstructionDo::matchLabel(
     RexxInstructionEnd *_end,         /* end to match up                   */
     RexxSource         *source )      /* parsed source file (for errors)   */
/******************************************************************************/
/* Function:  Verify that the name on an END and the END statement match      */
/******************************************************************************/
{
    RexxString *name = _end->name;                   /* get then END name                 */
    SourceLocation location = _end->getLocation();      /* get location of END instruction   */

    if (name != OREF_NULL)             /* was a name given?                 */
    {
        size_t lineNum = this->getLineNumber();   /* Instruction line number           */
        RexxString *myLabel = getLabel();
        if (myLabel == OREF_NULL)          /* name given on non-control form?   */
        {
                                           /* have a mismatched end             */
            source->error(Error_Unexpected_end_nocontrol, location, new_array(name, new_integer(lineNum)));
        }
        else if (name != getLabel())       /* not the same name?                */
        {
            source->error(Error_Unexpected_end_control, location, new_array(name, myLabel, new_integer(lineNum)));
        }
    }
}

void RexxInstructionDo::matchEnd(
     RexxInstructionEnd *partner,      /* partner END instruction for block */
     RexxSource         *source )      /* parsed source file (for errors)   */
/******************************************************************************/
/* Make sure we have a match between and END and a DO                         */
/******************************************************************************/
{
    this->matchLabel(partner, source);   /* match up the names                */
    OrefSet(this, this->end, partner);   /* match up with the END instruction */
    if (this->type != SIMPLE_DO)       /* not a simple DO form?             */
    {
        partner->setStyle(LOOP_BLOCK);     /* this is a loop form               */
    }
    else
    {
        // for a simple DO, we need to check if this has a label
        if (getLabel() != OREF_NULL)
        {
            partner->setStyle(LABELED_DO_BLOCK);
        }
        else
        {
            partner->setStyle(DO_BLOCK);
        }
    }
}


/**
 * Check for a label match on a block instruction.
 *
 * @param name   The target block name.
 *
 * @return True if this is a name match, false otherwise.
 */
bool RexxInstructionDo::isLabel(RexxString *name)
{
    return label == name;
}

/**
 * Get the label for this block instruction.
 *
 * @return The label for the loop.  Returns OREF_NULL if there is no label.
 */
RexxString *RexxInstructionDo::getLabel()
{
    return label;
}

/**
 * Tests to see if this is a loop instruction.
 *
 * @return True if this is a repetitive loop, false otherwise.
 */
bool RexxInstructionDo::isLoop()
{
    return this->type != SIMPLE_DO;
}


void RexxInstructionDo::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/******************************************************************************/
{
  memory_mark(this->nextInstruction);  /* must be first one marked          */
  memory_mark(this->initial);
  memory_mark(this->to);
  memory_mark(this->by);
  memory_mark(this->forcount);
  memory_mark(this->control);
  memory_mark(this->label);
  memory_mark(this->conditional);
  memory_mark(this->end);
}

void RexxInstructionDo::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/******************************************************************************/
{
                                       /* must be first one marked          */
  memory_mark_general(this->nextInstruction);
  memory_mark_general(this->initial);
  memory_mark_general(this->to);
  memory_mark_general(this->by);
  memory_mark_general(this->forcount);
  memory_mark_general(this->control);
  memory_mark_general(this->label);
  memory_mark_general(this->conditional);
  memory_mark_general(this->end);
}

void RexxInstructionDo::flatten(RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten an object                                               */
/******************************************************************************/
{
  setUpFlatten(RexxInstructionDo)

  flatten_reference(newThis->nextInstruction, envelope);
  flatten_reference(newThis->initial, envelope);
  flatten_reference(newThis->to, envelope);
  flatten_reference(newThis->by, envelope);
  flatten_reference(newThis->forcount, envelope);
  flatten_reference(newThis->control, envelope);
  flatten_reference(newThis->label, envelope);
  flatten_reference(newThis->conditional, envelope);
  flatten_reference(newThis->end, envelope);

  cleanUpFlatten
}

void RexxInstructionDo::terminate(
     RexxActivation *context,          /* current execution context         */
     RexxDoBlock    *doblock )         /* active do block                   */
/******************************************************************************/
/* Function:  Terminate an active do loop                                     */
/******************************************************************************/
{
                                       /* perform cleanup                   */
  context->terminateBlock(doblock->getIndent());
                                       /* jump to the loop end              */
  context->setNext(this->end->nextInstruction);
}

void RexxInstructionDo::execute(
    RexxActivation      *context,      /* current activation context        */
    RexxExpressionStack *stack)        /* evaluation stack                  */
/******************************************************************************/
/* Function:  Execute a REXX DO instruction                                   */
/******************************************************************************/
{
    RexxDoBlock  *doblock = OREF_NULL;   /* active DO block                   */
    RexxObject   *result;                /* expression evaluation result      */
    RexxArray    *array;                 /* converted collection object       */
    wholenumber_t count;                 /* count for repetitive or FOR loops */
    RexxObject   *object;                /* result object (for error)*/

    context->traceInstruction(this);     /* trace if necessary                */
    if (this->type != SIMPLE_DO)       /* a real loop instruction?          */
    {
        /* create an active DO block         */
        doblock = new RexxDoBlock (this, context->getIndent());
        context->newDo(doblock);           /* set the new block                 */

        switch (this->type)              /* process each DO seperately        */
        {

            case DO_FOREVER:                 /* DO FOREVER loop                   */
            case DO_UNTIL:                   /* DO UNTIL - no checks first time   */
                break;                         /* just quit                         */

            case DO_OVER:                    /* DO name OVER collection           */
            case DO_OVER_UNTIL:              /* same as DO_OVER on first pass     */
                /* get the collection object         */
                result = this->initial->evaluate(context, stack);
                doblock->setTo(result);        /* Anchor result in doBlock to keep  */
                                               /*  from GC.                         */
                context->traceResult(result);  /* trace if necessary                */
                if (isOfClass(Array, result))      /* already an array item?            */
                {
                    /* get the non-sparse version        */
                    array = ((RexxArray *)result)->makeArray();
                }
                else                           /* some other type of collection     */
                {
                    /* get the array version of this     */
                    array = REQUEST_ARRAY(result);
                    /* didn't convert ok?                */
                    if (array == TheNilObject || !isOfClass(Array, array) )
                    {
                        /* raise an error                    */
                        reportException(Error_Execution_noarray, result);
                    }
                }
                doblock->setTo(array);         /* save this as the "TO" value       */
                doblock->setFor(1);            /* set the initial position          */
                                               /* go process the loop               */
                if (!this->checkOver(context, stack, doblock))
                {
                    /* cause termination cleanup         */
                    this->terminate(context, doblock);
                }
                break;

            case DO_OVER_WHILE:              /* DO name OVER collection WHILE cond*/
                /* get the collection object         */
                result = this->initial->evaluate(context, stack);
                /* Anchor result in doBlock to keep  */
                doblock->setTo(result);        /*  from GC.                         */
                context->traceResult(result);  /* trace if necessary                */
                if (isOfClass(Array, result))      /* already an array item?            */
                {
                    /* get the non-sparse version        */
                    array = ((RexxArray *)result)->makeArray();
                }
                else                         /* some other type of collection     */
                {
                    /* get the array version of this     */
                    array = REQUEST_ARRAY(result);
                    /* didn't convert ok?                */
                    if (array == TheNilObject || !isOfClass(Array, array) )
                    {
                        /* raise an error                    */
                        reportException(Error_Execution_noarray, result);
                    }
                }
                doblock->setTo(array);         /* save this as the "TO" value       */
                doblock->setFor(1);            /* set the initial position          */
                                               /* go process the loop               */
                if (!this->checkOver(context, stack, doblock) || !this->whileCondition(context, stack))
                {
                    /* cause termination cleanup         */
                    this->terminate(context, doblock);
                }
                break;

            case DO_COUNT:                   /* DO expr                           */
            case DO_COUNT_UNTIL:             /* DO expr UNTIL foo                 */
                /* get the expression value          */
                result = this->forcount->evaluate(context, stack);
                object = result;               /* save for error reporting          */
                                               /* an integer value already, and     */
                                               /* we're dealing with a "normal      */
                                               /* NUMERIC DIGITS setting            */
                if (isOfClass(Integer, result) && context->digits() >= Numerics::DEFAULT_DIGITS)
                {
                    /* get the value directly            */
                    count = ((RexxInteger *)result)->getValue();
                    context->traceResult(result);/* trace if necessary                */
                }
                else
                {
                    /* get this as a number string,      */
                    /* which should force string         */
                    /* conversion also                   */
                    result = REQUEST_STRING(result);
                    /* force rounding                    */
                    result = callOperatorMethod(result, OPERATOR_PLUS, OREF_NULL);
                    context->traceResult(result);/* trace if necessary                */
                                                 /* convert the value                 */
                    if (!result->requestNumber(count, number_digits()))
                    {
                        /* report an exception               */
                        reportException(Error_Invalid_whole_number_repeat, object);
                    }
                }
                /* bad value, too small or too big?  */
                if (count < 0)
                {
                    /* report an exception               */
                    reportException(Error_Invalid_whole_number_repeat, object);
                }
                doblock->setFor(count);        /* save the new value                */
                if (doblock->testFor())        /* is this DO 0?                     */
                {
                                               /* cause termination cleanup         */
                    this->terminate(context, doblock);
                }
                break;

            case DO_COUNT_WHILE:             /* DO expr WHILE foo                 */
                /* get the expression value          */
                result = this->forcount->evaluate(context, stack);
                object = result;               /* save for error reporting          */
                                               /* an integer value already, and     */
                                               /* we're dealing with a "normal      */
                                               /* NUMERIC DIGITS setting            */
                if (isOfClass(Integer, result) && context->digits() >= Numerics::DEFAULT_DIGITS)
                {
                    /* get the value directly            */
                    count = ((RexxInteger *)result)->getValue();
                    context->traceResult(result);/* trace if necessary                */
                }
                else
                {
                    /* get this as a number string,      */
                    /* which should force string         */
                    /* conversion also                   */
                    result = REQUEST_STRING(result);
                    /* force rounding                    */
                    result = callOperatorMethod(result, OPERATOR_PLUS, OREF_NULL);
                    context->traceResult(result);/* trace if necessary                */
                                                 /* convert the value                 */
                    if (!result->requestNumber(count, number_digits()))
                    {
                        /* report an exception               */
                        reportException(Error_Invalid_whole_number_repeat, object);
                    }
                }
                /* bad value, too small or too big?  */
                if (count < 0)
                {
                    /* report an exception               */
                    reportException(Error_Invalid_whole_number_repeat, object);
                }
                doblock->setFor(count);        /* save the new value                */
                                               /* is this DO 0?                     */
                if (doblock->testFor() || !this->whileCondition(context, stack))
                {
                    /* cause termination cleanup         */
                    this->terminate(context, doblock);
                }
                break;

            case DO_WHILE:                   /* DO WHILE condition                */
                /* evaluate the condition            */
                if (!this->whileCondition(context, stack))
                {
                    /* cause termination cleanup         */
                    this->terminate(context, doblock);
                }
                break;

            case CONTROLLED_DO:              /* DO i=expr TO expr BY expr FOR expr*/
            case CONTROLLED_UNTIL:           /* DO i=expr ... UNTIL condition     */
                /* do initial controlled loop setup  */
                this->controlSetup(context, stack, doblock);
                /* fail the initial check?           */
                if (!this->checkControl(context, stack, doblock, false))
                {
                    /* cause termination cleanup         */
                    this->terminate(context, doblock);
                }
                break;

            case CONTROLLED_WHILE:           /* DO i=expr ... WHILE condition     */
                /* do initial controlled loop setup  */
                this->controlSetup(context, stack, doblock);
                /* fail the initial check or         */
                /* the WHILE check?                  */
                if (!this->checkControl(context, stack, doblock, false) || !this->whileCondition(context, stack))
                {
                    /* cause termination cleanup         */
                    this->terminate(context, doblock);
                }
                break;
        }
    }
    else                                 /* just a simple do                  */
    {
        if (getLabel() != OREF_NULL)
        {
            /* create an active DO block         */
            doblock = new RexxDoBlock (this, context->getIndent());
            context->newDo(doblock);           /* set the new block                 */
        }
        else
        {
            context->addBlock();               /* step the nesting level            */
        }
    }
    /* do debug pause if necessary       */
    /* have to re-execute?               */
    if (context->conditionalPauseInstruction())
    {
        if (doblock != OREF_NULL)
        {
            this->terminate(context, doblock); /* cause termination cleanup         */
        }
        else
        {
            context->removeBlock();        /* cause termination cleanup         */
        }
        context->setNext(this);            /* make this the new next instruction*/
    }
}


void RexxInstructionDo::controlSetup(
     RexxActivation      *context,     /* current activation context        */
     RexxExpressionStack *stack,       /* evaluation stack                  */
     RexxDoBlock         *doblock )    /* stacked DO execution block        */
/******************************************************************************/
/* Function:  Setup for use of a control variable                             */
/******************************************************************************/
{
    size_t      i;                       /* loop control variable             */
    RexxObject *result;                  /* expression result                 */
    RexxObject *_initial;                /* initial variable value            */
    RexxObject *object;                  /* original result object (for error)*/
    wholenumber_t count;                 /* for count                         */

                                         /* evaluate the initial expression   */
    _initial = this->initial->evaluate(context, stack);
    /* force rounding                    */
    _initial = callOperatorMethod(_initial, OPERATOR_PLUS, OREF_NULL);
    /* process each of the expressions   */
    for (i = 0; i < 3 && this->expressions[i] != 0; i++)
    {
        switch (this->expressions[i])    /* process various keywords          */
        {

            case EXP_TO:                     /* TO expression                     */
                {                                /* get the expression value          */
                    result = this->to->evaluate(context, stack);
                    /* force rounding                    */
                    result = callOperatorMethod(result, OPERATOR_PLUS, OREF_NULL);
                    /* if the result is a string, see if we can convert this to */
                    /* an integer value.  This is very common in loops, and can */
                    /* save us a lot of processing on each loop iteration. */
                    RexxObject *temp = result->integerValue(number_digits());
                    if (temp != TheNilObject)
                    {
                        result = temp;
                    }
                    doblock->setTo(result);        /* save the new value                */
                    break;
                }

            case EXP_BY:                     /* BY expression                     */
                /* get the expression value          */
                result = this->by->evaluate(context, stack);
                /* force rounding                    */
                result = callOperatorMethod(result, OPERATOR_PLUS, OREF_NULL);
                doblock->setBy(result);        /* save the new value                */
                                               /* if the BY is negative             */
                if (callOperatorMethod(result, OPERATOR_LESSTHAN, IntegerZero) == TheTrueObject)
                {
                    /* comparison is for less than       */
                    /* the termination value             */
                    doblock->setCompare(OPERATOR_LESSTHAN);
                }
                else
                {
                    /* comparison is for greater than    */
                    /* the termination value             */
                    doblock->setCompare(OPERATOR_GREATERTHAN);
                }
                break;

            case EXP_FOR:                    /* FOR expression                    */
                /* get the expression value          */
                result = this->forcount->evaluate(context, stack);
                object = result;               /* save for error reporting          */
                                               /* an integer value already, and     */
                                               /* we're dealing with a "normal      */
                                               /* NUMERIC DIGITS setting            */
                if (isOfClass(Integer, result) && context->digits() >= Numerics::DEFAULT_DIGITS)
                {
                    /* get the value directly            */
                    count = ((RexxInteger *)result)->getValue();
                    context->traceResult(result);/* trace if necessary                */
                }
                else
                {
                    /* get this as a number string,      */
                    /* which should force string         */
                    /* conversion also                   */
                    result = REQUEST_STRING(result);
                    /* force rounding                    */
                    result = callOperatorMethod(result, OPERATOR_PLUS, OREF_NULL);
                    context->traceResult(result);/* trace if necessary                */
                                                 /* convert the value                 */
                    if (!result->requestNumber(count, number_digits()))
                    {
                        /* report an exception               */
                        reportException(Error_Invalid_whole_number_for, object);
                    }
                }
                /* bad value, too small or too big?  */
                if (count < 0)
                {
                    /* report an exception               */
                    reportException(Error_Invalid_whole_number_for, object);
                }
                doblock->setFor(count);        /* save the new value                */
                break;
        }
    }
    if (this->by == OREF_NULL)         /* no BY expression?                 */
    {
        doblock->setBy(IntegerOne);        /* use an increment of 1             */
                                           /* comparison is for greater than    */
                                           /* the termination value             */
        doblock->setCompare(OPERATOR_GREATERTHAN);
    }
    /* do the initial assignment         */
    this->control->assign(context, stack, _initial);
}

bool RexxInstructionDo::checkOver(
     RexxActivation      *context,     /* current activation context        */
     RexxExpressionStack *stack,       /* evaluation stack                  */
     RexxDoBlock         *doblock )    /* stacked DO execution block        */
/******************************************************************************/
/* Function:  Process an iterationn of an OVER loop                           */
/******************************************************************************/
{
    size_t      over_position;           /* position of DO_OVER iteration     */
    RexxArray  *over_array;              /* DO OVER value array               */
    RexxObject *result;                  /* process the result                */
    over_position = doblock->getFor();   /* get the current position          */
                                         /* get the value array               */
    over_array = (RexxArray *)doblock->getTo();
    /* reached the end?                  */
    if (over_array->size() < over_position)
    {
        return false;                    // time to get out of here.
    }
    /* get the next element              */
    result = over_array->get(over_position);
    if (result == OREF_NULL)           /* empty for some reason?            */
    {
        result = TheNilObject;           /* use .nil instead                  */
    }
                                         /* do the initial assignment         */
    this->control->assign(context, stack, result);
    context->traceResult(result);      /* trace if necessary                */
    doblock->setFor(over_position + 1);/* set position for next time        */
    return true;
}


bool RexxInstructionDo::checkControl(
     RexxActivation      *context,     /* current activation context        */
     RexxExpressionStack *stack,       /* evaluation stack                  */
     RexxDoBlock         *doblock,     /* stacked DO execution block        */
     bool                 increment )  /* increment control variable test   */
/******************************************************************************/
/* Function:  Step and check the value of a control variable against the      */
/*            terminating value                                               */
/******************************************************************************/
{
    RexxObject *result;                  /* increment result                  */
                                         /* get the control variable value    */
    result = this->control->getValue(context);
    context->traceResult(result);        /* trace if necessary                */
    if (increment)
    {                     /* not the first time through?       */
                          /* perform the addition              */
        result = callOperatorMethod(result, OPERATOR_PLUS, doblock->getBy());
        /* increment the control variable    */
        /* value and assign new value        */
        this->control->set(context, result);
        context->traceResult(result);      /* trace if necessary                */
    }
    if (this->to != OREF_NULL)
    {         /* have a termination condition?     */
              /* do the comparison                 */
        if (callOperatorMethod(result, doblock->getCompare(), doblock->getTo()) == TheTrueObject)
        {
            return false;                  // time to stop if this is true
        }
    }
    if (this->forcount != OREF_NULL)     /* have a for count to check?        */
    {
        if (doblock->testFor())          /* hit the end condition?            */
        {
            return false;                // done looping
        }
    }
    return true;                         // still looping
}


void RexxInstructionDo::reExecute(
     RexxActivation      *context,     /* current activation context        */
     RexxExpressionStack *stack,       /* evaluation stack                  */
     RexxDoBlock         *doblock )    /* stacked DO execution block        */
/******************************************************************************/
/* Function:  Handle a re-execution of a DO loop (every iteration by the      */
/*            first.                                                          */
/******************************************************************************/
{
    /* set for the top of the loop       */
    context->setNext(this->nextInstruction);
    context->traceInstruction(this);     /* trace if necessary                */
    context->indent();                   /* now indent again                  */

    switch (this->type)                /* process each DO seperately        */
    {

        case DO_FOREVER:                   /* DO FOREVER loop                   */
            return;                          /* nothing to do at all              */

        case DO_OVER:                      /* DO name OVER collection loop      */
            /* go process the loop               */
            if (this->checkOver(context, stack, doblock))
            {
                return;                        /* finish quickly                    */
            }
            break;

        case DO_OVER_UNTIL:                /* DO name OVER coll. UNTIL cond.    */
            /* go process the loop               */
            /* fail the control check or         */
            /* the UNTIL check?                  */
            if (!this->untilCondition(context, stack) && this->checkOver(context, stack, doblock))
            {
                return;                        /* finish quickly                    */
            }
            break;

        case DO_OVER_WHILE:                /* DO name OVER coll. WHILE cond.    */
            /* go process the loop               */
            /* fail the control check or         */
            /* the WHILE check?                  */
            if (this->checkOver(context, stack, doblock) && this->whileCondition(context, stack))
            {
                return;                        /* finish quickly                    */
            }
            break;

        case DO_UNTIL:                     /* DO UNTIL condition                */
            /* evaluate the condition            */
            if (!this->untilCondition(context, stack))
                return;                        /* finish quickly                    */
            break;

        case DO_COUNT:                     /* DO expr                           */
            if (!doblock->testFor())         /* have we reached 0?                */
            {
                return;                        /* finish quickly                    */
            }
            break;

        case DO_COUNT_WHILE:               /* DO expr WHILE expr                */
            /* have we reached 0?                */
            if (!doblock->testFor() && this->whileCondition(context, stack))
            {
                return;                        /* finish quickly                    */
            }
            break;

        case DO_COUNT_UNTIL:               /* DO expr UNTIL expr                */
            /* have we reached 0?                */
            if (!this->untilCondition(context, stack) && !doblock->testFor())
            {
                return;                        /* finish quickly                    */
            }
            break;

        case DO_WHILE:                     /* DO WHILE condition                */
            /* evaluate the condition            */
            if (this->whileCondition(context, stack))
            {
                return;                        /* finish quickly                    */
            }
            break;

        case CONTROLLED_DO:                /* DO i=expr TO expr BY expr FOR expr*/
            /* fail the termination check?       */
            if (this->checkControl(context, stack, doblock, true))
            {
                return;                        /* finish quickly                    */
            }
            break;

        case CONTROLLED_UNTIL:             /* DO i=expr ... UNTIL condition     */
            /* fail the control check or         */
            /* the UNTIL check?                  */
            if (!this->untilCondition(context, stack) && this->checkControl(context, stack, doblock, true))
            {
                return;                        /* finish quickly                    */
            }
            break;

        case CONTROLLED_WHILE:             /* DO i=expr ... WHILE condition     */
            /* fail the control check or         */
            /* the WHILE check?                  */
            if (this->checkControl(context, stack, doblock, true) && this->whileCondition(context, stack))
            {
                return;                        /* finish quickly                    */
            }
            break;
    }
    context->popBlock();                 /* cause termination cleanup         */
    context->removeBlock();              /* remove the execution nest         */
                                         /* jump to the loop end              */
    context->setNext(this->end->nextInstruction);
    context->unindent();                 /* step back trace indentation       */
}

bool RexxInstructionDo::untilCondition(
     RexxActivation      *context,     /* current activation context        */
     RexxExpressionStack *stack )      /* evaluation stack                  */
/******************************************************************************/
/* Function:  Evaluate the result of a WHILE or UNTIL condition               */
/******************************************************************************/
{
    /* get the expression value          */
    RexxObject *result = this->conditional->evaluate(context, stack);
    context->traceResult(result);        /* trace if necessary                */

    /* most comparisons return either true or false directly, so we */
    /* can optimize this check.  UNTIL conditions are more likely to */
    /* evaluate to false, so we'll check that first */
    if (result == TheFalseObject)
    {
        return false;
    }
    else if (result == TheTrueObject)
    {
        return true;
    }
    /* This is some sort of computed boolean, so we need to do a real */
    /* validation on this */
    return result->truthValue(Error_Logical_value_until);
}

bool RexxInstructionDo::whileCondition(
     RexxActivation      *context,     /* current activation context        */
     RexxExpressionStack *stack )      /* evaluation stack                  */
/******************************************************************************/
/* Function:  Evaluate the result of a WHILE or UNTIL condition               */
/******************************************************************************/
{
    /* get the expression value          */
    RexxObject *result = this->conditional->evaluate(context, stack);
    context->traceResult(result);        /* trace if necessary                */

    /* most comparisons return either true or false directly, so we */
    /* can optimize this check.  WHILE conditions are more likely to */
    /* evaluate to true, so we'll check that first */
    if (result == TheTrueObject)
    {
        return true;
    }
    else if (result == TheFalseObject)
    {
        return false;
    }
    /* This is some sort of computed boolean, so we need to do a real */
    /* validation on this */
    return result->truthValue(Error_Logical_value_while);
}
