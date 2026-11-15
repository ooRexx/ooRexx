/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2026 Rexx Language Association. All rights reserved.         */
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
/******************************************************************************/
/* REXX Translator                                                            */
/*                                                                            */
/* CATCH clause of a DO, LOOP or SELECT block (RFE #31)                       */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxActivation.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "CatchInstruction.hpp"
#include "DoInstruction.hpp"
#include "DoBlock.hpp"
#include "EndInstruction.hpp"
#include "ExpressionBaseVariable.hpp"
#include "ExpressionVariable.hpp"


/**
 * Construct a CATCH clause.
 *
 * @param c      The array of condition names caught by this clause.
 */
RexxInstructionCatch::RexxInstructionCatch(ArrayClass *c, RexxVariableBase *v, ArrayClass *q)
{
    conditions = c;
    variable = v;
    codes = q;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInstructionCatch::live(size_t liveMark)
{
    // must be first object marked
    memory_mark(nextInstruction);
    memory_mark(conditions);
    memory_mark(owner);
    memory_mark(nextCatch);
    memory_mark(firstCatch);
    memory_mark(variable);
    memory_mark(codes);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInstructionCatch::liveGeneral(MarkReason reason)
{
    // must be first object marked
    memory_mark_general(nextInstruction);
    memory_mark_general(conditions);
    memory_mark_general(owner);
    memory_mark_general(nextCatch);
    memory_mark_general(firstCatch);
    memory_mark_general(variable);
    memory_mark_general(codes);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInstructionCatch::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInstructionCatch)

    flattenRef(nextInstruction);
    flattenRef(conditions);
    flattenRef(owner);
    flattenRef(nextCatch);
    flattenRef(firstCatch);
    flattenRef(variable);
    flattenRef(codes);

    cleanUpFlatten
}


/**
 * Test if this clause catches a condition.  ANY catches every
 * condition, as SIGNAL ON ANY does.  SYNTAX, ERROR and FAILURE can
 * have a code, as on RAISE: SYNTAX 42 catches every error 42,
 * SYNTAX 42.3 that error only; ERROR and FAILURE compare their code
 * with the return code of the condition, as "==" does (RFE #31).
 *
 * @param condition The condition name.
 * @param conditionObj
 *                  The condition object, or OREF_NULL when only the
 *                  name is known (the condition object has not
 *                  been created yet): the codes are not looked at
 *                  then.
 * @param context   The activation that owns the block, where a
 *                  variable used as a code is read.
 *
 * @return true if the clause catches the condition.
 */
bool RexxInstructionCatch::handles(RexxString *condition, DirectoryClass *conditionObj, RexxActivation *context)
{
    size_t count = conditions->items();
    for (size_t i = 1; i <= count; i++)
    {
        RexxString *name = (RexxString *)conditions->get(i);
        if (name->strCompare(GlobalNames::ANY))
        {
            return true;
        }
        if (!name->strCompare(condition))
        {
            continue;
        }
        RexxObject *code = codes == OREF_NULL ? TheNilObject : (RexxObject *)codes->get(i);
        if (code == TheNilObject || conditionObj == OREF_NULL)
        {
            return true;
        }
        // SYNTAX: the code is major * 1000 + minor, as a message number;
        // with no minor part, every error with that major number
        if (name->strCompare(GlobalNames::SYNTAX))
        {
            wholenumber_t wanted = 0;
            // a simple variable is read now, in the activation of the
            // block (its name if it has no value, without NOVALUE); a value
            // that is not an error number does not match
            if (isOfClass(VariableTerm, code))
            {
                RexxObject *value = readCode(code, context);
                if (!(isString(value) || isInteger(value) || isNumberString(value)))
                {
                    continue;
                }
                wanted = errorNumber(value->stringValue());
                if (wanted < 0)
                {
                    continue;
                }
            }
            else
            {
                code->numberValue(wanted);
            }
            wholenumber_t raised = 0;
            RexxObject *raisedCode = (RexxObject *)conditionObj->get(GlobalNames::CODE);
            if (raisedCode != OREF_NULL)
            {
                // "42.3", or "42" for an error with no minor code
                RexxString *text = raisedCode->requestString();
                const char *data = text->getStringData();
                const char *point = strchr(data, '.');
                raised = strtol(data, NULL, 10) * 1000 + (point == NULL ? 0 : strtol(point + 1, NULL, 10));
            }
            if (wanted % 1000 == 0 ? raised / 1000 == wanted / 1000 : raised == wanted)
            {
                return true;
            }
        }
        // ERROR and FAILURE: the return code, compared as "=" does
        else
        {
            RexxObject *rc = (RexxObject *)conditionObj->get(GlobalNames::RC);
            if (rc != OREF_NULL && sameCode(code, rc, context))
            {
                return true;
            }
        }
    }
    return false;
}


/**
 * Compare the code of an ERROR or FAILURE clause with the return code
 * of a condition, as "==" does, without running any method when one
 * of them is a string: a command handler can return any object as
 * its return code, and asking it for a string value could raise
 * NOSTRING in the middle of the search for a clause (RFE #31).
 *
 * @param code    The code: a string, or a simple variable.
 * @param rc      The return code of the condition.
 * @param context The activation that owns the block.
 *
 * @return true if they match.
 */
bool RexxInstructionCatch::sameCode(RexxObject *code, RexxObject *rc, RexxActivation *context)
{
    // a simple variable is read now, in the activation of the block; with
    // no value it is its own name, and NOVALUE is not raised
    if (isOfClass(VariableTerm, code))
    {
        code = readCode(code, context);
    }
    bool codeIsString = isString(code) || isInteger(code) || isNumberString(code);
    bool rcIsString = isString(rc) || isInteger(rc) || isNumberString(rc);
    // two strings (or numbers): compared as strings, as "==" does
    if (codeIsString && rcIsString)
    {
        RexxString *a = code->stringValue();
        RexxString *b = rc->stringValue();
        return a->getLength() == b->getLength() && memcmp(a->getStringData(), b->getStringData(), a->getLength()) == 0;
    }
    // a string and another object never match
    if (codeIsString || rcIsString)
    {
        return false;
    }
    // two objects: the "==" message, which is identity unless the class
    // of the code redefines it
    ProtectedObject result;
    code->sendMessage(GlobalNames::STRICT_EQUAL, rc, result);
    RexxObject *answer = result;
    return answer == TheTrueObject;
}


/**
 * Read a simple variable used as the code of a CATCH clause, in the
 * activation that owns the block.  A variable with no value has its
 * own name (in uppercase), as everywhere in Rexx, but NOVALUE is not
 * raised: the clause is being tested while a condition is raised.
 *
 * @param code    The variable.
 * @param context The activation that owns the block.
 *
 * @return The value of the variable, or its name.
 */
RexxObject *RexxInstructionCatch::readCode(RexxObject *code, RexxActivation *context)
{
    RexxObject *value = ((RexxVariableBase *)code)->getRealValue(context);
    if (value == OREF_NULL)
    {
        value = ((RexxSimpleVariable *)code)->getName();
    }
    return value;
}


/**
 * Convert the code of a SYNTAX clause, an error number written as
 * "major" or "major.minor" (1 to 99, and 1 to 999), to
 * major * 1000 + minor, as message numbers are kept.
 *
 * @param text   The error number.
 *
 * @return The number, or -1 if the text is not a valid error number.
 */
wholenumber_t RexxInstructionCatch::errorNumber(RexxString *text)
{
    const char *data = text->getStringData();
    size_t length = text->getLength();
    wholenumber_t major = 0;
    wholenumber_t minor = 0;
    size_t i = 0;
    size_t digits = 0;
    for (; i < length && data[i] >= '0' && data[i] <= '9' && digits < 3; i++, digits++)
    {
        major = major * 10 + (data[i] - '0');
    }
    bool valid = digits > 0 && major >= 1 && major <= 99;
    if (valid && i < length)
    {
        valid = data[i++] == '.';
        digits = 0;
        for (; i < length && data[i] >= '0' && data[i] <= '9' && digits < 4; i++, digits++)
        {
            minor = minor * 10 + (data[i] - '0');
        }
        valid = valid && digits > 0 && i == length && minor >= 1 && minor <= 999;
    }
    return valid ? major * 1000 + minor : -1;
}


/**
 * Find the CATCH clause of an active block that catches a
 * condition.  The clauses are tried in order, the first one that
 * catches the condition wins.
 *
 * @param doblock   The block.
 * @param condition The condition name.
 * @param conditionObj The condition object, or OREF_NULL.
 * @param context   The activation that owns the block.
 *
 * @return The clause, or OREF_NULL if the block has no clause for
 *         the condition (or no CATCH clauses at all).
 */
RexxInstructionCatch *RexxInstructionCatch::findClause(DoBlock *doblock, RexxString *condition, DirectoryClass *conditionObj, RexxActivation *context)
{
    RexxInstructionCatch *clause = doblock->getCatches();
    while (clause != OREF_NULL)
    {
        if (clause->handles(condition, conditionObj, context))
        {
            return clause;
        }
        clause = clause->nextCatch;
    }
    return OREF_NULL;
}


/**
 * Test if a block instruction has CATCH clauses.
 *
 * @param block  The block instruction.
 *
 * @return true if it has.
 */
bool RexxInstructionCatch::hasCatches(RexxBlockInstruction *block)
{
    return block->end != OREF_NULL && block->end->getStyle() == CATCH_BLOCK;
}


/**
 * Find the first CATCH clause of a block instruction.  The clauses
 * are between the body and the END; the block does not keep them
 * (adding a field to it would change the layout of compiled
 * programs), so look for the first one that belongs to it.
 *
 * @param block  The block instruction.
 *
 * @return The first CATCH clause, or OREF_NULL if the block has none.
 */
RexxInstructionCatch *RexxInstructionCatch::firstClause(RexxBlockInstruction *block)
{
    if (!hasCatches(block))
    {
        return OREF_NULL;
    }
    RexxInstruction *end = block->end;
    for (RexxInstruction *i = block->nextInstruction; i != OREF_NULL && i != end; i = i->nextInstruction)
    {
        if (i->isType(KEYWORD_CATCH) && ((RexxInstructionCatch *)i)->owner == block)
        {
            return (RexxInstructionCatch *)i;
        }
    }
    return OREF_NULL;
}


/**
 * Give a copy of the condition object to the variable of a CATCH
 * clause written as CATCH variable = condition (RFE #31), when the
 * clause starts to run.  It is the same copy that CONDITION('O')
 * returns, so that CATCH v = c is CATCH c followed by
 * v = CONDITION('O').
 *
 * @param context      The activation that runs the clause.
 * @param conditionObj The condition object.
 */
void RexxInstructionCatch::assignCondition(RexxActivation *context, DirectoryClass *conditionObj)
{
    if (variable != OREF_NULL)
    {
        variable->assign(context, (RexxObject *)conditionObj->copy());
    }
}


/**
 * Execute a CATCH clause.  Control reaches a CATCH clause in two
 * ways: the body of the block runs into the first one, and then
 * the clause does what the END of the block would have done; or
 * the code of a CATCH clause runs into the next one, and then the
 * block ends.  Control does not reach the code of a clause this
 * way: the context jumps there when a condition is caught.
 *
 * Either way, what is traced is the END of the block, as it would be
 * without CATCH clauses: the clause itself does not run.
 *
 * @param context The current execution context.
 * @param stack   The current evaluation stack.
 */
void RexxInstructionCatch::execute(RexxActivation *context, ExpressionStack *stack)
{
    // no active block: a SIGNAL into the middle of a block
    if (!context->hasActiveBlockInstructions())
    {
        context->traceInstruction(this);
        reportException(Error_Unexpected_end_catch);
    }

    DoBlock *doblock = context->topBlockInstruction();

    // the code of a CATCH clause has finished: the block ends
    if (doblock->isInCatch())
    {
        owner->terminate(context, doblock);
        context->traceInstruction(owner->end);
        return;
    }

    // the body has finished: do what the END would have done
    switch (ownerStyle)
    {
        case LOOP_BLOCK:
        {
            context->setIndent(doblock->getIndent());
            context->traceInstruction(owner->end);
            ((RexxInstructionBaseLoop *)owner)->reExecute(context, stack, doblock);
            break;
        }
        // a SELECT without OTHERWISE where no WHEN was true
        case SELECT_BLOCK:
        {
            context->unindent();
            context->traceInstruction(owner->end);
            reportException(Error_When_expected_nootherwise);
            break;
        }
        default:
        {
            owner->terminate(context, doblock);
            context->traceInstruction(owner->end);
            break;
        }
    }
}
