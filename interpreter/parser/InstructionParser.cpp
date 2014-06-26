/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Translator Object Constructors                                   */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* NOTE:      Start of new methods for translator class objects.  These new   */
/*            methods are segregated here to allow the actual translator to   */
/*            be easily decoupled from the rest of the interpreter.           */
/*            All of these methods are actually methods of the LanguageParser */
/*            class.  They are in a seperate source file for locational       */
/*            convenience.                                                    */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "LanguageParser.cpp"

#include "ExpressionMessage.hpp"
#include "ExpressionOperator.hpp"
#include "RexxInstruction.hpp"                /* base REXX instruction class       */

#include "AssignmentInstruction.hpp"                /* keyword <expression> instructions */
#include "CommandInstruction.hpp"
#include "ExitInstruction.hpp"
#include "InterpretInstruction.hpp"
#include "OptionsInstruction.hpp"
#include "ReplyInstruction.hpp"
#include "ReturnInstruction.hpp"
#include "SayInstruction.hpp"

#include "AddressInstruction.hpp"                 /* other instructions                */
#include "DropInstruction.hpp"
#include "ExposeInstruction.hpp"
#include "ForwardInstruction.hpp"
#include "GuardInstruction.hpp"
#include "LabelInstruction.hpp"
#include "LeaveInstruction.hpp"
#include "MessageInstruction.hpp"
#include "NopInstruction.hpp"
#include "NumericInstruction.hpp"
#include "ProcedureInstruction.hpp"
#include "QueueInstruction.hpp"
#include "RaiseInstruction.hpp"
#include "TraceInstruction.hpp"
#include "UseStrictInstruction.hpp"

#include "CallInstruction.hpp"                 /* call/signal instructions          */
#include "SignalInstruction.hpp"

#include "ParseInstruction.hpp"                /* parse instruction and associates  */
#include "ParseTarget.hpp"
#include "ParseTrigger.hpp"

#include "ElseInstruction.hpp"                 /* control type instructions         */
#include "EndIf.hpp"
#include "IfInstruction.hpp"
#include "ThenInstruction.hpp"

#include "DoBlock.hpp"                /* block type instructions           */
#include "DoInstruction.hpp"
#include "EndInstruction.hpp"
#include "OtherwiseInstruction.hpp"
#include "SelectInstruction.hpp"
#include "ProtectedObject.hpp"


/**
 * Process an individual Rexx clause and decide what
 * type of instruction it represents.
 *
 * @return An executable instruction object, or OREF_NULL if
 *         we've reached the end of a code block.
 */
RexxInstruction *LanguageParser::instruction()
{
    RexxInstruction *workingInstruction = OREF_NULL;
    // get the first token of the clause
    RexxToken *first = nextReal();

    // a :: at the start of a clause is a directive, which ends this execution
    // block.  Back things up and return null to indicate we've reached the end.
    if (first->isType(TOKEN_DCOLON))
    {
        firstToken();
        reclaimClause();
        return OREF_NULL;
    }

    // the subTerms list is used for a number of temporary things during
    // parsing, as well as being a handy place for protecting sets of
    // objects from garbage collection.  Empty this list before parsing
    // any instruction.
    subTerms->empty();

    // ok, now go through the instruction type progression.  First check is for
    // a label.
    RexxToken *second = nextToken();

    // a label is a symbol or string immediately followed by a :
    if (first->isSymbolOrLiteral() && second->isType(TOKEN_COLON))
    {
        // not allowed in an interpret instruction
        if (isInterpret())
        {
            syntaxError(Error_Unexpected_label_interpret, first);
        }

        // create a new instruction using these two tokens.
        RexxInstruction *inst = labelNew(first, second);
        // The colon on a label acts as a clause terminator.  If there
        // are any tokens after the colon, we need to make that the start of
        // the next clause.
        second = nextToken()

        if (!second->isEndOfClause())
        {
            // push that token back and trim it to this position.  This becomes
            // the next clause.
            previousToken();
            trimClause();
            reclaimClause();
        }
        return inst;
    }

    // this is potentially an assignment of the form "symbol = expr"
    // we still have both the first and second tokens available for testing.
    if (first->isSymbol())
    {
        // "symbol == expr" is considered an error
        if (second->isSubtype(OPERATOR_STRICT_EQUAL))
        {
            syntaxError(Error_Invalid_expression_general, second);
        }
        // true assignment instruction?
        if (second->isSubtype(OPERATOR_EQUAL))
        {
            return assignmentNew(first);
        }
        // this could be a special assignment operator such as "symbol += expr"
        else if (second->isType(TOKEN_ASSIGNMENT))
        {
            return this->assignmentOpNew(_first, second);
        }
    }

    // some other type of instruction
    // we need to skip over the first
    // term of the instruction to
    // determine the type of clause,
    // including recognition of keyword
    // instructions

    // reset to the beginning and parse off the first term (which could be a
    // message send)
    firstToken();
    RexxObject *term = messageTerm();
    // a lot depends on the nature of the second token
    second = nextToken();

    // some sort of recognizable message term?  Need to check for the
    // special cases.
    if (term != OREF_NULL)
    {
        // if parsing the message term consumed everything, this is a message instruction
        if (second->isEndOfClause())
        {
            return messageNew((RexxExpressionMessage *)term);
        }
        else if (second->isSubtype(OPERATOR_STRICT_EQUAL))
        {
            // messageterm == something is an invalid assignment
            syntaxError(Error_Invalid_expression_general, second);
        }
        // messageterm = something is a pseudo assignment
        else if (second->isSubtype(OPERATOR_EQUAL))
        {
            ProtectedObject p(term);

            // we need an expression following the op token, again, using the rest of the
            // instruction.
            RexxObject *subexpression = subExpression(TERM_EOC);
            if (subexpression == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_general, second);
            }
            // this is a message assignment
            return messageAssignmentNew((RexxExpressionMessage *)term, subexpression);
        }
        // one of the special operator forms?
        else if (second->classId == TOKEN_ASSIGNMENT)
        {
            ProtectedObject p(term);
            // we need an expression following the op token
            RexxObject *subexpression = this->subExpression(TERM_EOC);
            if (subexpression == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_general, second);
            }
            // this is a message assignment
            return messageAssignmentOpNew((RexxExpressionMessage *)term, second, subexpression);
        }
    }

    // ok, none of the special cases passed....now start the keyword processing

    // reset again and get the first token
    firstToken();
    first = nextToken();

    // see if we can get an instruction keyword match from the first token
    InstructionKeyword keyword = first->keyword();

    if (keyword \= KEYWORD_NONE)
    {
        // we found something, switch to the appropriate instruction processor.
        switch (keyword)
        {
            // NOP instruction
            case KEYWORD_NOP:
                return nopNew();
                break;

            // DROP instruction
            case KEYWORD_DROP:
                return dropNew();
                break;

            // SIGNAL instruction
            case KEYWORD_SIGNAL:
                return signalNew();
                break;

            // CALL instruction, in all of its forms
            case KEYWORD_CALL:
                return callNew();
                break;

            // RAISE instruction
            case KEYWORD_RAISE:
                return raiseNew();
                break;

            // ADDRESS instruction
            case KEYWORD_ADDRESS:
                return addressNew();
                break;

            // NUMERIC instruction
            case KEYWORD_NUMERIC:
                return numericNew();
                break;

            // TRACE instruction
            case KEYWORD_TRACE:
                return traceNew();
                break;

            // DO instruction, all variants
            case KEYWORD_DO:
                // the option indicates this is a DO instruction.
                return createLoop(false);
                break;

            // all variants of the LOOP instruction
            case KEYWORD_LOOP:
                // the option indicates this is a LOOP instruction.
                return createLoop(true);
                break;

            // EXIT instruction
            case KEYWORD_EXIT:
                return exitNew();
                break;

            // INTERPRET instruction
            case KEYWORD_INTERPRET:
                return interpretNew();
                break;

            // PUSH instruction
            case KEYWORD_PUSH:
                return queueNew();
                break;

            // QUEUE instruction
            case KEYWORD_QUEUE:
                return pushNew();
                break;

            // REPLY instruction
            case KEYWORD_REPLY:
                return replyNew();
                break;

            // RETURN instruction
            case KEYWORD_RETURN:
                return returnNew();
                break;

            // IF instruction
            case KEYWORD_IF:
                return ifNew(KEYWORD_IF);
                break;

            // ITERATE instruction
            case KEYWORD_ITERATE:
                return leaveNew(KEYWORD_ITERATE);
                break;

            // LEAVE instruction
            case KEYWORD_LEAVE:
                return leaveNew(KEYWORD_LEAVE);
                break;

            // EXPOSE instruction
            case KEYWORD_EXPOSE:
                return exposeNew();
                break;

            // FORWARD instruction
            case KEYWORD_FORWARD:
                return forwardNew();
                break;

            // PROCEDURE instruction
            case KEYWORD_PROCEDURE:
                return procedureNew();
                break;

            // GUARD instruction
            case KEYWORD_GUARD:
                return guardNew();
                break;

            // USE instruction
            case KEYWORD_USE:
                return useNew();
                break;

            // ARG instruction
            case KEYWORD_ARG:
                return parseNew(SUBKEY_ARG);
                break;

            // PULL instruction
            case KEYWORD_PULL:
                return parseNew(SUBKEY_PULL);
                break;

            // PARSE instruction
            case KEYWORD_PARSE:
                return parseNew(KEYWORD_PARSE);
                break;

            // SAY instruction
            case KEYWORD_SAY:
                return sayNew();
                break;

            // OPTIONS instruction
            case KEYWORD_OPTIONS:
                return optionsNew();
                break;

            // select instruction
            case KEYWORD_SELECT:
                return selectNew();
                break;

            // WHEN instruction in the context of a SELECT
            case KEYWORD_WHEN:
                // this parses as if it was an IF, but creates
                // a differnt target instruction.
                return ifNew(KEYWORD_WHEN);
                break;

            // OTEHRWISE in a SELECT
            case KEYWORD_OTHERWISE:
                return otherwiseNew(first);
                break;

            // ELSE instruction...possibly unexpected.
            case KEYWORD_ELSE:
                return elseNew(first);
                break;

            // END instruction for some block instruction (DO, LOOP, SELECT, OTHERWISE).
            case KEYWORD_END:
                return endNew();
                break;

            // THEN instruction.  IF processing handles THEN directly.  Found
            // in a naked context like this, we have an invalid THEN.
            case KEYWORD_THEN:
                syntaxError(Error_Unexpected_then_then);
                break;

        }
    }
    // does not begin with a recognized keyword...this is a "command" instruction.
    else
    {
        // the first token is part of the command, put it back.
        firstToken();
        return commandNew();
    }
    // should never reach here.
    return OREF_NULL;
}


/**
 * Create a "raw" executable instruction object.
 * Allocation/creation of instruction objects are handled a
 * little differently, and goes in stages: 1) Have RexxMemory
 * allocate a Rexx object of the appropriate size.  2) Set the
 * object behaviour to be the table for the target instruction.
 * 3) Call the in-memory new() method on this object using the
 * RexxInstruction() constructor.  This does base initialization
 * of the object as an instruction object.  This version is
 * returned to the caller, with this instruction anchored in the
 * parser object to protect it from garbage collection, which is
 * handy if its constructor needs to allocate any additional
 * objects.
 *
 * Step 4) happens once the caller receives this object
 * instance.  It agains calls the new() in-memory allocator for
 * the final instruction class.  The constructor fills in any
 * instruction specific information, as well as changing the
 * object virtual function table to the final version.  The
 * information we set up here will remain untouched.
 *
 * @param size       The size of the object (can be variable for
 *                   some instructions).
 * @param _behaviour The Rexx object behaviour.
 * @param type       The type identifiers for the instruction
 *                   (see RexxToken InstructionKeyword enum).
 *
 * @return A newly created instruction object with basic instruction initialization.
 */
RexxInstruction *LanguageParser::sourceNewObject(size_t size, RexxBehaviour *_behaviour,
    InstructionKeyword type )
{
    RexxObject *newObject = new_object(size);
    newObject->setBehaviour(_behaviour);
    new ((void *)newObject) RexxInstruction (clause, type);
    currentInstruction = newObject;
    return (RexxInstruction *)newObject;
}


/**
 * Parse an ADDRESS instruction and create an executable instruction object.
 *
 * @return An instruction object that can perform this function.
 */
RexxInstruction *LanguageParser::addressNew()
{
    RexxObject *dynamicAddress = OREF_NULL;
    RexxString *environment = OREF_NULL;
    RexxObject *command = OREF_NULL;
    RexxToken *token = nextReal();

    // have something to process?  Having nothing is not an error, it
    // just toggles the environment
    if (!token->isEndOfClause())
    {
        // if this is a symbol or a string, this is the
        // ADDRESS env [command] form.  Otherwise, this is an
        // implicit ADDRESS VALUE
        if (!token->isSymbolOrLiteral())
        {
            // back up to include this token in the expression
            previousToken();
            // and get the expression
            dynamicAddress = expression(TERM_EOC);
        }
        else
        {
            // could be ADDRESS VALUE (NOTE:  Subkeyword also
            // checks that this is a SYMBOL token.
            if (token->subKeyword() == SUBKEY_VALUE)
            {
                // get the value expression
                dynamicAddress = expression(TERM_EOC);
                // this is a required expression
                if (dynamicAddress == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_address);
                }
            }
            else
            {
                // this is a constant target
                environment = token->value();
                // see if we have the command form
                token = nextReal();
                if (!token->isEndOfClause())
                {
                    // back up and create the expression
                    previousToken();
                    command = expression(TERM_EOC);
                }
            }
        }
    }

    RexxInstruction *newObject = new_instruction(ADDRESS, Address);
    new ((void *)newObject) RexxInstructionAddress(dynamicAddress, environment, command);
    return newObject;
}



/**
 * Create a new variable assignment instruction.
 *
 * @param target This is the token holding the name of the variable name
 *               for the assignment.
 *
 * @return An assignment instruction object.
 */
RexxInstruction *LanguageParser::assignmentNew(RexxToken  *target )
{
    // so far, we only know that the target is a symbol.  Verify that this
    // really is a variable symbol.  This handles raising an error if not valid.
    needVariable(target);
    // everthing after the "=" is the expression that is assigned to the variable.
    // The expression is required.
    RexxObject *expr = expression(TERM_EOC);
    if (expr == OREF_NULL)
    {
        syntaxError(Error_Invalid_expression_assign);
    }

    // build an instruction object and return it.
    RexxInstruction *newObject = new_instruction(ASSIGNMENT, Assignment);
    new ((void *)newObject) RexxInstructionAssignment((RexxVariableBase *)(addText(target)), expr);
    return newObject;
}


/**
 * Create a special assignment op of the form "variable (op)= expr".
 *
 * @param target    The assignment target variable.
 * @param operation The operator token.  classId is TOKEN_ASSIGNMENT, the subclass
 *                  is the type of the operation to perform.
 *
 * @return The constructed instruction object.
 */
RexxInstruction *LanguageParser::assignmentOpNew(RexxToken *target, RexxToken *operation)
{
    needVariable(target);     // make sure this is a variable
    // we require an expression for the additional part, which is required
    RexxObject *expr = expression(TERM_EOC);
    if (expr == OREF_NULL)
    {
        syntaxError(Error_Invalid_expression_assign);
    }

    // we need an evaluator for both the expression and the assignment
    RexxObject *variable = addText(target);
    // now add a binary operator to this expression tree
    expr = (RexxObject *)new RexxBinaryOperator(operation->subtype(), variable, expr);

    // now everything is the same as an assignment operator
    RexxInstruction *newObject = new_instruction(ASSIGNMENT, Assignment);
    new ((void *)newObject) RexxInstructionAssignment((RexxVariableBase *)variable, _expression);
    return newObject; /* done, return this                 */
}


/**
 * Parse a CALL ON or CALL OFF instruction.
 *
 * @param type   The target instruction type (CALL_ON or CALL_OFF).
 *
 * @return An executable instruction object for te target instruction.
 */
RexxInstruction *LanguageParser::callOnNew(InstructionSubKeyword type)
{
    // The processing of the CONDITION name is the same for both CALL ON
    // and CALL OFF

    RexxString *labelName;
    RexxString *conditionName;
    BuiltinCode builtinIndex = NO_BUILTIN; RexxToken::resolveBuiltin(targetName);


    // we must have a symbol following, otherwise this is an error.
    RexxToken *token = nextReal();
    if (!token->isSymbol())
    {
        syntaxError(type == SUBKEY_ON ? Error_Symbol_expected_on : Error_Symbol_expected_off);
    }

    // get the condition involved
    ConditionKeyword condType = token->condition();
    // invalid condition specified?  another error.
    // NOTE:  CALL ON only supports a subset of the conditions allowed for SIGNAL ON
    if (condType == CONDITION_NONE ||
        condType == CONDITION_PROPAGATE)
        condType == CONDITION_SYNTAX ||
        condType == CONDITION_NOVALUE ||
        condType == CONDITION_PROPAGATE ||
        condType == CONDITION_LOSTDIGITS ||
        condType == CONDITION_NOMETHOD ||
        condType == CONDITION_NOSTRING)
    {
        syntaxError(type == SUBKEY_ON ? Error_Invalid_subkeyword_callon : Error_Invalid_subkeyword_calloff, token);
    }
    // USER conditions need a little more work.
    else if (condType == CONDITION_USER)
    {
        // The condition name follows the USER keyword.
        // This must be a symbol and is required.
        token = nextReal();
        if (!token->isSymbol())
        {
            syntaxError(Error_Symbol_expected_user);
        }
        // get the User condition name.  That is the default target for the
        // signal trap
        labelName = token->value();
        // The condition name for this instruction is "USER condition", so
        // construct that now and make it a common string (likely used
        // other places in the program)
        conditionName = commonString(labelName->concatToCstring(CHAR_USER_BLANK));
    }
    else
    {
        // this is one of the language defined conditions, use this for both
        // the name and the condition...the name
        labelName = token->value();
        conditionName = labelName;
    }

    // IF This is CALL ON, we can have an optional signal target
    // specified with the NAME option.
    if (type == SUBKEY_ON)
    {
        // ok, we can have a NAME keyword after this
        token = nextReal();
        if (!token->isEndOfClause())
        {
            // keywords are always symbols    */
            if (!token->isSymbol())
            {
                syntaxError(Error_Invalid_subkeyword_callonname, token);
            }
            // only subkeyword allowed here is NAME
            if (token->subKeyword() != SUBKEY_NAME)
            {
                syntaxError(Error_Invalid_subkeyword_callonname, token);
            }
            // we need a label name after this as a string or symbol
            token = nextReal();
            if (!token->isSymbolOrLiteral())
            {
                syntaxError(Error_Symbol_or_string_name);
            }
            // this overrides the default label taken from the condition name.
            labelName = token->value;
            // nothing more permitted after this
            requiredEndOfClause(Error_Invalid_data_name);
        }
        // resolve any potential builtin match...not likely to be used, but it is
        // part of the defined search order.
        builtinIndex = RexxToken::resolveBuiltin(labelName);
    }
    else
    {
        // SIGNAL OFF doesn't have a label name, so make sure this is turned off.
        // We use this NULL value to determined which function to perform.
        labelName = OREF_NULL;

        // must have the end of clause here.
        requiredEndOfClause(Error_Invalid_data_condition);
    }

    // create a new instruction object
    RexxInstruction *newObject = new_instruction(CALL_ON, CallOn);
    new ((void *)newObject) RexxInstructionCallOn(conditionName, labelName, buildinIndex);

    // if this is the ON form, we have some end parsing resolution to perform.
    if (type == SUBKEY_ON)
    {
        addReference((RexxObject *)newObject);
    }
    return newObject;
}


/**
 * Process a call of the form CALL (expr), which
 * has a dynamically determined call target.
 *
 * @param token  The first token of the expression (with the opening paren of the name expression.)
 *
 * @return A new object for executing this call.
 */
RexxInstruction *LanguageParser::dynamicCallNew(RexxToken *token)
{
    // this is a full expression in parens
    RexxObject *targetName = parenExpression(token);
    // an expression is required
    if (name == OREF_NULL)
    {
        syntaxError(Error_Invalid_expression_call);
    }
    // process the argument list
    size_t argCount = argList(OREF_NULL, TERM_EOC);

    // create a new instruction object
    RexxInstruction *newObject = new_variable_instruction(CALL_VALUE, DynamicCall, sizeof(RexxInstructionDynamicCall) + (argCount - 1) * sizeof(RexxObject *));
    RexxInstruction *newObject = new_instruction(CALL_VALUE, DynamicCall);
    new ((void *)newObject) RexxInstructionDynamicCall(targetName, argCount, subTerms);

    // NOTE:  The name of the call cannot be determined until run time, so we don't
    // add this to the reference list for later processing
    return newObject;
}


/**
 * Finish parsing of a CALL instruction.  There are 3 distinct
 * forms of the Call instruction, with a different execution
 * object for each form.
 *
 * @return An executable call instruction object.
 */
RexxInstruction *LanguageParser::callNew()
{
    BuiltinCode builtin_index;
    RexxString *targetName;
    size_t argCount = 0;
    bool noInternal = false;

    // get the next token (skipping over any blank following the CALL keyword
    RexxToken *token = nextReal();

    // there must be something here.
    if (token->isEndOfClause())
    {
        /* this is an error                  */
        syntaxError(Error_Symbol_or_string_call);
    }
    // ok, if this is a symbol, we might have CALL ON or CALL OFF.  These get
    // processed into a separate instruction type.
    else if (token->isSymbol())
    {
        // check for a matching subkeyword.  On ON or OFF are of significance
        // here.
        InstructionSubKeyword keyword = token->subKeyword();
        // one of the special forms, this has it's own parsing code.
        if (keyword == SUBKEY_ON || keyword == SUBKEY_OFF)
        {
            return callOnNew(keyword);
        }
        // This is a normal call instruction.  Need to grab the target name and
        // parse off the arguments
        else
        {
            targetName = token->value();
            // set the builtin index for later resolution steps
            builtin_index = token->builtin();
            // parse off an argument list
            argCount = this->argList(OREF_NULL, TERM_EOC);
        }
    }
    // call with a string target
    else if (token->isLiteral())
    {
        targetName = token->value();
        // set the builtin index for later resolution steps
        builtin_index = token->builtin();
        // parse off an argument list
        argCount = this->argList(OREF_NULL, TERM_EOC);
        // because this uses a string name, the internal label
        // search is bypassed.
        noInternal = false;
    }
    // is this call (expr) form?
    else if (token->isType(TOKEN_LEFT))
    {
        // this has its own custom instruction object.
        return dynamicCallNew(token);
    }
    // Something unknown...
    else
    {
        syntaxError(Error_Symbol_or_string_call);
    }
    // create a new Call instruction.  This only handles the simple calles.
    RexxInstruction *newObject = new_variable_instruction(CALL, Call, sizeof(RexxInstructionCall) + (argCount - 1) * sizeof(RexxObject *));
    new ((void *)newObject) RexxInstructionCall(targetName, argCount, subTerms, noInternal, builtin_index);

    // add to our references list
    addReference((RexxObject *)newObject);
    return newObject;
}


/**
 * Parse and create a COMMAND instruction.
 *
 * @return A COMMAND instruction instance.
 */
RexxInstruction *LanguageParser::commandNew()
{
    // this is pretty simple...parse the optional expresson and create the
    // instruction instance.  NOTE:  we already know we have something to
    // parse, so there's no need to check for a missing expression.
    RexxObject *_expression = expression(TERM_EOC);

    RexxInstruction *newObject = new_instruction(COMMAND, Command);
    new ((void *)newObject) RexxInstructionCommand(_expression);
    return newObject;
}


/**
 * Parse and create the variants of a controlled DO/LOOP
 * instruction (Loop i = 1 to 5 by 2 for 10).  This
 * can also include a WHILE or UNTIL modifier (but not both).
 *
 * @param label     Any label obtained from the LABEL keyword.
 * @param nameToken The control variable name token.
 *
 * @return A contructed instruction object of the appropriate type.
 */
RexxInstruction *LanguageParser::newControlledLoop(RexxString *label, RexxToken *nameToken)
{
    // a contruct to fill in for the instruction.
    ControlledLoop control;
    WhileUntilLoop conditional;
    // track while/until forms
    InstructionSubKeyword conditionalType = SUBKEY_NONE;

    // set a default increment value
    control.by = IntegerOne;

    // the control expressions need to evaluated in the coded order,
    // so we keep a little table of the order used.
    int keyslot = 0;
    // if this DO/LOOP didn't have a label clause, use the control
    // variable name as the loop name.
    if (label == OREF_NULL)
    {
        label = nameToken->value();
    }

    // get a retriever to be able to access this token.
    control.control = addVariable(nameToken);
    // parse off the initial value using the subkeywords as expression terminators
    control.initial = expression(TERM_CONTROL);
    // the initial value is a required expression
    if (control.initial == OREF_NULL)
    {
        syntaxError(Error_Invalid_expression_control);
    }
    // protect on the term stack
    subTerms->push(control.initial);

    // ok, keep looping while we don't have a clause terminator
    // because the parsing of the initial expression is terminated by either
    // the end-of-clause or DO/LOOP keyword, we know the next token will by
    // a symbol if it is not the end.
    RexxToken *token = nextReal();

    while (!token->isEndOfClause())
    {
        // this must be a keyword, so resolve it.
        switch (token->subKeyword())
        {

            case SUBKEY_BY:
            {
                // only one per customer
                if (control.by != OREF_NULL)
                {
                    syntaxError(Error_Invalid_do_duplicate, token);
                }

                // get the keyword expression, which is required also
                control.by = expression(TERM_CONTROL);
                if (control.by == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_by);

                }
                // protect on the term stack
                subTerms->push(control.by);

                // record the processing order
                control.expressions[keyslot++] = EXP_BY;
                break;
            }

            case SUBKEY_TO:
            {
                // only one per customer
                if (control.to != OREF_NULL)
                {
                    syntaxError(Error_Invalid_do_duplicate, token);
                }

                // get the keyword expression, which is required also
                control.to = expression(TERM_CONTROL);
                if (control.to == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_to);

                }
                // protect on the term stack
                subTerms->push(control.to);

                // record the processing order
                control.expressions[keyslot++] = EXP_TO;
                break;
            }

            case SUBKEY_FOR:
            {
                // only one per customer
                if (control.forCount != OREF_NULL)
                {
                    syntaxError(Error_Invalid_do_duplicate, token);
                }
                // get the keyword expression, which is required also
                control.forCount, expression(TERM_CONTROL);
                if (control.forCount == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_for);

                }
                // record the processing order
                control.expressions[keyslot++] = EXP_FOR;
                break;
            }

            case SUBKEY_UNTIL:
            {
                // step back a token and process the conditional
                previousToken();
                // this also does not allow anything after the loop conditional
                conditional.conditional = parseLoopConditional(conditionalType, 0);
                break;
            }
        }
        token = nextReal();
    }

    // NOTE:  We parse until we hit the end of clause or found an error,
    // so once we get here, there's no need for any end-of-clause checks.

    // we've parsed everything correctly and we have three potential types of
    // loop now.  1)  A controlled loop with no conditional, 2) a controlled loop
    // with a WHILE condition and 3) a controlled loop with a UNTIL condition.
    // The conditionalType variable tells us which form it is, so we can create
    // the correct type instruction object.

    switch (conditionalType)
    {
        // Controlled loop with no extra conditional.
        case SUBKEY_NONE:
        {
            RexxInstruction *newObject = new_instruction(LOOP_CONTROLLED, ControlledDo);
            new ((void *)newObject) RexxInstructionControlledDo(label, control);
            return newObject;
        }
        // Controlled loop with a WHILE conditional
        case SUBKEY_WHILE:
        {
            RexxInstruction *newObject = new_instruction(LOOP_CONTROLLED_WHILE, ControlledDoWhile);
            new ((void *)newObject) RexxInstructionControlledDoWhile(label, control, conditional);
            return newObject;
        }
        // Controlled loop with an UNTIL conditional.
        case SUBKEY_UNTIL:
        {
            RexxInstruction *newObject = new_instruction(LOOP_CONTROLLED_UNTIL, ControlledDoUntil);
            new ((void *)newObject) RexxInstructionControlledDoUntil(label, control, conditional);
            return newObject;
        }
    }
    return OREF_NULL;    // should never get here.
}


/**
 * Parse and create the variants of a DO OVER loop instruction.
 * This can also include a WHILE or UNTIL modifier (but not
 * both).
 *
 * @param label     Any label obtained from the LABEL keyword.
 * @param nameToken The control variable name token.
 *
 * @return A contructed instruction object of the appropriate type.
 */
RexxInstruction *LanguageParser::newDoOverLoop(RexxString *label, RexxToken *nameToken)
{
    // a contruct to fill in for the instruction.
    OverLoop control;
    WhileUntilLoop conditional;
    // track while/until forms
    InstructionSubKeyword conditionalType = SUBKEY_NONE;

    // if this DO/LOOP didn't have a label clause, use the control
    // variable name as the loop name.
    if (label == OREF_NULL)
    {
        label = nameToken->value();
    }

    // save the control variable retriever
    control.control = addVariable(nameToken));
    // and get the OVER expression, which is required
    control.target = expression(TERM_COND);
    if (control.target == OREF_NULL)
    {
        syntaxError(Error_Invalid_expression_over);
    }

    // process an additional conditional (NOTE:  Because of the
    // terminators used for the target expression, the only possibilities
    // here are end of clause, a WHILE keyword, or an UNTIL keyword)
    conditional.conditional = parseLoopConditional(conditionalType, 0));

    // NOTE:  We parse until we hit the end of clause or found an error,
    // so once we get here, there's no need for any end-of-clause checks.

    // we've parsed everything correctly and we have three potential types of
    // loop now.  1)  A DO OVER loop with no conditional, 2) a DO OVER loop
    // with a WHILE condition and 3) a DO OVER loop with a UNTIL condition.
    // The conditionalType variable tells us which form it is, so we can create
    // the correct type instruction object.

    switch (conditionalType)
    {
        // DO OVER with no extra conditional.
        case SUBKEY_NONE:
        {
            RexxInstruction *newObject = new_instruction(LOOP_OVER, DoOver);
            new ((void *)newObject) RexxInstructionDoOver(label, control);
            return newObject;
        }
        // DO OVER with a WHILE conditional
        case SUBKEY_WHILE:
        {
            RexxInstruction *newObject = new_instruction(LOOP_OVER_WHILE, DoOverWhile);
            new ((void *)newObject) RexxInstructionDoOverWhile(label, control, conditional);
            return newObject;
        }
        // DO OVER with an UNTIL conditional.
        case SUBKEY_UNTIL:
        {
            RexxInstruction *newObject = new_instruction(LOOP_OVER_UNTIL, DoOverUntil);
            new ((void *)newObject) RexxInstructionDoOverUntil(label, control, conditional);
            return newObject;
        }
    }
    return OREF_NULL;    // should never get here.
}


/**
 * Create an instance of a simple DO block.
 *
 * @param label  The optional block label.
 *
 * @return An instruction object for a do block.
 */
RexxInstruction *LanguageParser::newSimpleDo(RexxString *label)
{
    RexxInstruction *newObject = new_instruction(SIMPLE_BLOCK, SimpleDo);
    new ((void *)newObject) RexxInstructionSimpleDo(label);
    return newObject;
}


/**
 * Create an instance of a DO FOREVER loop.
 *
 * @param label  The optional block label.
 *
 * @return An instruction object for a do block.
 */
RexxInstruction *LanguageParser::newLoopForever(RexxString *label)
{
    RexxInstruction *newObject = new_instruction(LOOP_FOREVER, DoForever);
    new ((void *)newObject) RexxInstructionDoForever(label);
    return newObject;
}


/**
 * Create an instance of a DO WHILE loop.
 *
 * @param label  The optional block label.
 *
 * @return An instruction object for a do block.
 */
RexxInstruction *LanguageParser::newLoopWhile(RexxString *label, WhileUntilLoop &conditional)
{
    RexxInstruction *newObject = new_instruction(LOOP_WHILE, DoWhile);
    new ((void *)newObject) RexxInstructionDoWhile(label, conditional);
    return newObject;
}


/**
 * Create an instance of a DO WHILE loop.
 *
 * @param label  The optional block label.
 *
 * @return An instruction object for a do block.
 */
RexxInstruction *LanguageParser::newLoopUntil(RexxString *label, WhileUntilLoop &conditional)
{
    RexxInstruction *newObject = new_instruction(LOOP_UNTIL, DoUntil);
    new ((void *)newObject) RexxInstructionDoUntil(label, conditional);
    return newObject;
}


/**
 * Parse a DO FOREVER loop.  This will be either just
 * DO FOREVER (most common), but can also have WHILE
 * or UNTIL modifiers, which makes them identical to DO
 * WHILE and DO UNTIL.
 *
 * @param label  The instruction label.
 *
 * @return An instruction of the appropriate type.
 */
RexxInstruction *LanguageParser::parseForeverLoop(RexxString *label)
{
    // a contruct to fill in for the instruction.
    WhileUntilLoop conditional;

    // track while/until forms
    InstructionSubKeyword conditionalType = SUBKEY_NONE;

    // process an additional conditional.
    conditional.conditional = parseLoopConditional(conditionalType, Error_Invalid_do_forever));

    // NOTE:  We parse until we hit the end of clause or found an error,
    // so once we get here, there's no need for any end-of-clause checks.

    // we've parsed everything correctly and we have three potential types of
    // loop now.  1)  FOREVER loop with no conditional, 2) a FOREVER loop
    // with a WHILE condition and 3) a FOREVER loop with a UNTIL condition.
    // The conditionalType variable tells us which form it is, so we can create
    // the correct type instruction object.

    switch (conditionalType)
    {
        // Straight DO FOREVER
        case SUBKEY_NONE:
        {
            return newLoopForever(label);
        }
        // DO FOREVER with a WHILE conditional...identical to a DO WHILE
        case SUBKEY_WHILE:
        {
            return newLoopWhile(label, conditional);
        }
        // DO FOREVER with an UNTIL conditional...identical to a DO UNTIL
        case SUBKEY_UNTIL:
        {
            return newLoopWhile(label, conditional);
        }
    }
    return OREF_NULL;    // should never get here.
}


/**
 * Parse a DO count loop.  This will be either just DO count
 * (most common), but can also have WHILE or UNTIL modifiers.
 *
 * @param label  The instruction label.
 *
 * @return An instruction of the appropriate type.
 */
RexxInstruction *LanguageParser::parseCountLoop(RexxString *label)
{
    // the For count controller
    ForLoop forCount;

    // a contruct to fill in for the instruction.
    WhileUntilLoop conditional;

    // track while/until forms
    InstructionSubKeyword conditionalType = SUBKEY_NONE;

    // we know there is something there, so we will either get an
    // expression, or get an error for an invalid expression

    forCount.forCount = expression(TERM_COND);

    // process an additional conditional (NOTE:  Because of the
    // terminators used for the target expression, the only possibilities
    // here are end of clause, a WHILE keyword, or an UNTIL keyword)
    conditional.conditional = parseLoopConditional(conditionalType, 0));

    // NOTE:  We parse until we hit the end of clause or found an error,
    // so once we get here, there's no need for any end-of-clause checks.

    // we've parsed everything correctly and we have three potential types of
    // loop now.  1)  A count loop with no conditional, 2) a count loop
    // with a WHILE condition and 3) a count loop with a UNTIL condition.
    // The conditionalType variable tells us which form it is, so we can create
    // the correct type instruction object.

    switch (conditionalType)
    {
        // DO count with no extra conditional.
        case SUBKEY_NONE:
        {
            RexxInstruction *newObject = new_instruction(LOOP_COUNT, DoCount);
            new ((void *)newObject) RexxInstructionDoCount(label, forCount);
            return newObject;
        }
        // DO count with a WHILE conditional
        case SUBKEY_WHILE:
        {
            RexxInstruction *newObject = new_instruction(LOOP_COUNT_WHILE, DoCountWhile);
            new ((void *)newObject) RexxInstructionDoCountWhile(label, control, conditional);
            return newObject;
        }
        // DO count with an UNTIL conditional.
        case SUBKEY_UNTIL:
        {
            RexxInstruction *newObject = new_instruction(LOOP_COUNT_UNTIL, DoCountUntil);
            new ((void *)newObject) RexxInstructionDoCountUntil(label, control, conditional);
            return newObject;
        }
    }
    return OREF_NULL;    // should never get here.
}


/**
 * Create a new instruction object for a DO or LOOP
 * instruction.
 *
 * @param newDo
 * @param isLoop indicates whether this is a DO or a LOOP
 *               instruction.
 *
 * @return An appropriate instruction object for the type of block
 *         construct.
 */
RexxInstruction *LanguageParser::createLoop(bool isLoop)
{
    // we may need to parse ahead a bit to determine the type of loop, so
    // get a reset position before getting next token
    size_t resetPosition = markPosition();
    // We've already figured out this is either a LOOP or a DO, now we need to
    // decode what else is going on.
    RexxToken *token = nextReal();

    // label parsing is handled here.
    RexxString *label = OREF_NULL;

    // before doing anything, we need to test for a "LABEL name" before other options.  We
    // need to make certain we check that the LABEL keyword is not being
    // used as control variable.
    if (token->isSymbol())
    {
        // potentially a label.  Check the keyword value
        if (token->subKeyword() == SUBKEY_LABEL)
        {
            // if the next token is a symbol, this is a label.
            RexxToken *name = nextReal();
            if (name->isSymbol())
            {
                // save the label name.
                label = name->value();
                // update the reset position before stepping to the next token
                resetPosition = markPosition();
                // step to the next token for processing the following parts
                token = nextReal();
            }
            // is this "symbol ="?  Handle as a controlled loop
            else if (name->subclass == OPERATOR_EQUAL)
            {
                // This is a controlled loop, we're all positioned to process this.
                return newControlledLoop(label, token);
            }
            else
            {
                // this is either an end-of-clause or something not a symbol.  This is an error.
                syntaxError(Error_Symbol_expected_LABEL);
            }
        }
    }


    // Just the single token means this is either a simple DO block or
    // a LOOP forever, depending on the type of instruction we're parsing.
    if (token->isEndOfClause())
    {
        // A LOOP instruction with nothing is a LOOP FOREVER.
        if (isLoop)
        {
            return newLoopForever(label);
        }
        else
        {
            return newSimpleDo(label);
        }
    }

    // we've already handled the control variable version where the variable happens to
    // be named LABEL above.  We still need to check for this with any other name.
    if (token->isSymbol())
    {
        // this can be a control variable or keyword.  We need to look ahead
        // to the next token.
        RexxToken *second = nextReal();
        // similar to assignment instructions, any equal sign triggers this to
        // be a controlled form, even if the the "=" is part of a larger instruction
        // such as "==".  We give this as an expression error.
        if (second->subclass == OPERATOR_STRICT_EQUAL)
        {
            syntaxError(Error_Invalid_expression_general, second);
        }
        // ok, now check for the control variable.
        else if (second->subclass == OPERATOR_EQUAL)
        {
            // go parse a controlled do loop.
            return newControlledLoop(label, token);
        }
        // DO name OVER collection form?
        else if (second->subKeyword() == SUBKEY_OVER)
        {
            // parse and return the DO OVER options
            return newDoOverLoop(label, token);
        }
        // not a controlled form, but this could be a conditional form.
        else
        {
            // start the parsing process over from the beginning with the first of the
            // loop-type tokens
            resetPosition(resetPosition);

            token = nextReal();
            // now check the other keyword varieties.
            switch (this->subKeyword(token))
            {
                // FOREVER...this can have either a WHILE or UNTIL modifier.
                case SUBKEY_FOREVER:         // DO FOREVER
                {
                    return parseForeverLoop(label);
                }

                // WHILE cond
                case SUBKEY_WHILE:
                {
                    // a contruct to fill in for the instruction.
                    WhileUntilLoop conditional;

                    // track while/until forms
                    InstructionSubKeyword conditionalType = SUBKEY_NONE;

                    // step back one token and process the conditional
                    previousToken();
                    conditional.conditional = parseLoopConditional(&conditional, 0));
                    // We know this is WHILE already, so we can create this directly
                    return newLoopWhile(label, conditional);
                }

                // DO UNTIL cond
                case SUBKEY_UNTIL:
                {
                    // a contruct to fill in for the instruction.
                    WhileUntilLoop conditional;

                    // track while/until forms
                    InstructionSubKeyword conditionalType = SUBKEY_NONE;

                    // step back one token and process the conditional
                    previousToken();
                    conditional.conditional = parseLoopConditional(&conditional, 0));
                    // We know this is UNTIL already, so we can create this directly
                    return newLoopUntil(label, conditional);
                }

                // not a real DO keyword, probably DO expr form, which can also have
                // WHILE or UNTIL modifiers.
                default:
                    // push the first token back
                    previousToken();
                    // and go parse this out.
                    return parseCountLoop(label);
            }
        }
    }
    // just a DO expr form that doesn't start with a symbol
    else
    {
        // push the first token back
        previousToken();
        // and go parse this out.
        return parseCountLoop(label);
    }
}


/**
 * Build a drop instruction object.
 *
 * @return An executable object.
 */
RexxInstruction *LanguageParser::dropNew()
{
    // process the variable list...variables will be left
    // in the subterms stack.
    size_t variableCount = processVariableList(KEYWORD_DROP);

    RexxInstruction *newObject = new_variable_instruction(DROP, Drop, sizeof(RexxInstructionDrop) + (variableCount - 1) * sizeof(RexxObject *));
    // this initializes from the sub term stack.
    new ((void *)newObject) RexxInstructionDrop(variableCount, subTerms);
    return newObject;
}

RexxInstruction *LanguageParser::elseNew(
     RexxToken  *token)                /* ELSE keyword token                */
/****************************************************************************/
/* Function:  Create a new ELSE translator object                           */
/****************************************************************************/
{
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(ELSE, Else);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionElse(token);
    return newObject; /* done, return this                 */
}

RexxInstruction *LanguageParser::endNew()
/****************************************************************************/
/* Function:  Create a new END translator object                            */
/****************************************************************************/
{
    RexxString *name = OREF_NULL;                    /* no name yet                       */
    RexxToken *token = nextReal();                  /* get the next token                */
    if (!token->isEndOfClause())
    {   /* have a name specified?            */
        if (!token->isSymbol())/* must have a symbol here           */
        {
            /* this is an error                  */
            syntaxError(Error_Symbol_expected_end);
        }
        name = token->value;               /* get the name pointer              */
        requiredEndOfClause(Error_Invalid_data_end);
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(END, End);
    /* now complete this                 */
    new((void *)newObject) RexxInstructionEnd(name);
    return newObject; /* done, return this                 */
}

RexxInstruction *LanguageParser::endIfNew(
     RexxInstructionIf *parent )       /* target parent IF or WHEN clause   */
/****************************************************************************/
/* Function:  Create a new DUMMY END IF translator object                   */
/****************************************************************************/
{
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(ENDIF, EndIf);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionEndIf(parent);
    return newObject; /* done, return this                 */
}


/**
 * Parse an EXIT instruction and return an executable
 * instruction object for this exit.
 *
 * @return A instance of the EXIT instruction.
 */
RexxInstruction *LanguageParser::exitNew()
{
    // this is pretty simple...parse the optional expresson and create the
    // instruction instance.
    RexxObject *_expression = expression(TERM_EOC);
    RexxInstruction *newObject = new_instruction(EXIT, Exit);
    new((void *)newObject) RexxInstructionExit(_expression);
    return newObject;
}


/**
 * Parse and create a new EXPOSE instruction
 *
 * @return The configured instruction instance.
 */
RexxInstruction *LanguageParser::exposeNew()
{
    // not valid in an interpret
    if (isInterpret())
    {
        syntaxError(Error_Translation_expose_interpret);
    }

    // validate the placement at the beginning of the code block
    isExposeValid();

    // process the variable list and create an instruction from this.
    // The variables are placed in the subTerms stack
    size_t variableCount = processVariableList(KEYWORD_EXPOSE);

    RexxInstruction *newObject = new_variable_instruction(EXPOSE, Expose, sizeof(RexxInstructionExpose) + (variableCount - 1) * sizeof(RexxObject *));
    new ((void *)newObject) RexxInstructionExpose(variableCount, subTerms);
    return newObject;
}

void LanguageParser::RexxInstructionForwardCreate(
    RexxInstructionForward *newObject) /* target FORWARD instruction        */
/****************************************************************************/
/* Function:  Create a FORWARD instruction object                           */
/****************************************************************************/
{
    bool returnContinue = false;              /* no return or continue yet         */
    RexxToken *token = nextReal();                  /* get the next token                */

    while (!token->isEndOfClause())
    {/* while still more to process       */
        if (!token->isSymbol())/* not a symbol token?               */
        {
            /* this is an error                  */
            syntaxError(Error_Invalid_subkeyword_forward_option, token);
        }
        switch (this->subKeyword(token))
        { /* get the keyword value             */

            case SUBKEY_TO:                  /* FORWARD TO expr                   */
                /* have a description already?       */
                if (newObject->target != OREF_NULL)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_subkeyword_to);
                }
                /* get the keyword value             */
                OrefSet(newObject, newObject->target, this->constantExpression());
                /* no expression here?               */
                if (newObject->target == OREF_NULL)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_expression_forward_to);
                }
                break;

            case SUBKEY_CLASS:               /* FORWARD CLASS expr                */
                /* have a class over ride already?   */
                if (newObject->superClass != OREF_NULL)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_subkeyword_forward_class);
                }
                /* get the keyword value             */
                OrefSet(newObject, newObject->superClass, this->constantExpression());
                /* no expression here?               */
                if (newObject->superClass == OREF_NULL)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_expression_forward_class);
                }
                break;

            case SUBKEY_MESSAGE:             /* FORWARD MESSAGE expr              */
                /* have a message over ride already? */
                if (newObject->message != OREF_NULL)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_subkeyword_message);
                }
                /* get the keyword value             */
                OrefSet(newObject, newObject->message, this->constantExpression());
                /* no expression here?               */
                if (newObject->message == OREF_NULL)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_expression_forward_message);
                }
                break;

            case SUBKEY_ARGUMENTS:           /* FORWARD ARGUMENTS expr            */
                /* have a additional already?        */
                if (newObject->arguments != OREF_NULL || newObject->array != OREF_NULL)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_subkeyword_arguments);
                }
                /* get the keyword value             */
                OrefSet(newObject, newObject->arguments, this->constantExpression());
                /* no expression here?               */
                if (newObject->arguments == OREF_NULL)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_expression_forward_arguments);
                }
                break;

            case SUBKEY_ARRAY:               /* FORWARD ARRAY (expr, expr)        */
                /* have arguments already?           */
                if (newObject->arguments != OREF_NULL || newObject->array != OREF_NULL)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_subkeyword_arguments);
                }
                token = nextReal();            /* get the next token                */
                                               /* not an expression list starter?   */
                if (token->classId != TOKEN_LEFT)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_expression_raise_list);
                }
                /* process the array list            */
                OrefSet(newObject, newObject->array, this->argArray(token, TERM_RIGHT));
                break;

            case SUBKEY_CONTINUE:            /* FORWARD CONTINUE                  */
                if (returnContinue)            /* already had the return action?    */
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_subkeyword_continue);
                }
                returnContinue = true;         /* not valid again                   */
                                               /* remember this                     */
                newObject->instructionFlags |= forward_continue;
                break;

            default:                         /* invalid subkeyword                */
                /* this is a sub keyword error       */
                syntaxError(Error_Invalid_subkeyword_forward_option, token);
                break;
        }
        token = nextReal();                /* step to the next keyword          */
    }
}

RexxInstruction *LanguageParser::forwardNew()
/****************************************************************************/
/* Function:  Create a new RAISE translator object                             */
/****************************************************************************/
{
    // not permitted in interpret
    if (isInterpret())
    {
        syntaxError(Error_Translation_forward_interpret);
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(FORWARD, Forward);
    new((void *)newObject) RexxInstructionForward;
    this->RexxInstructionForwardCreate((RexxInstructionForward *)newObject);
    return newObject; /* done, return this                 */
}

RexxInstruction *LanguageParser::guardNew()
/******************************************************************************/
/* Function:  Create a new GUARD translator object                            */
/******************************************************************************/
{
    // not permitted in interpret
    if (this->flags&_interpret)
    {
        syntaxError(Error_Translation_guard_interpret);
    }

    RexxObject *_expression = OREF_NULL;             /* default no expression             */
    RexxArray *variable_list = OREF_NULL;           /* no variable either                */
    size_t variable_count = 0;                  /* no variables yet                  */
    RexxToken *token = nextReal();                  /* get the next token                */
    if (!token->isSymbol())  /* must have a symbol here           */
    {
        /* raise the error                   */
        syntaxError(Error_Symbol_expected_numeric, token);
    }

    bool on_off = false;

    /* resolve the subkeyword and        */
    /* process the subkeyword            */
    switch (this->subKeyword(token))
    {
        case SUBKEY_OFF:                   /* GUARD OFF instruction             */
            on_off = false;                  /* this is the guard off form        */
            break;

        case SUBKEY_ON:                    /* GUARD ON instruction              */
            on_off = true;                   /* remember it is the on form        */
            break;

        default:
            /* raise an error                    */
            syntaxError(Error_Invalid_subkeyword_guard, token);
            break;
    }
    token = nextReal();                  /* get the next token                */
    if (token->isSymbol())
    {/* have the keyword form?            */
        /* resolve the subkeyword            */
        switch (this->subKeyword(token))
        { /* and process                       */

            case SUBKEY_WHEN:                /* GUARD ON WHEN expr                */
                this->setGuard();              /* turn on guard variable collection */
                                               /* process the expression            */
                _expression = this->expression(TERM_EOC);
                if (_expression == OREF_NULL)   /* no expression?                    */
                {
                    /* expression is required after value*/
                    syntaxError(Error_Invalid_expression_guard);
                }
                /* retrieve the guard variable list  */
                variable_list = this->getGuard();
                /* get the size                      */
                variable_count = variable_list->size();
                break;

            default:                         /* invalid subkeyword                */
                /* raise an error                    */
                syntaxError(Error_Invalid_subkeyword_guard_on, token);
                break;
        }
    }
    else if (!token->isEndOfClause())/* not the end?                      */
    {
        /* raise an error                    */
        syntaxError(Error_Invalid_subkeyword_guard_on, token);
    }

    /* Get new object                    */
    RexxInstruction *newObject = new_variable_instruction(GUARD, Guard, sizeof(RexxInstructionGuard)
                                                       + (variable_count - 1) * sizeof(RexxObject *));
    /* Initialize this new method        */
    new ((void *)newObject) RexxInstructionGuard(_expression, variable_list, on_off);
    return newObject; /* done, return this                 */
}

RexxInstruction *LanguageParser::ifNew(
    int   type )                       /* type of instruction (IF or WHEN)  */
/****************************************************************************/
/* Function:  Create a new INTERPRET instruction object                     */
/****************************************************************************/
{
    /* process the expression            */
    RexxObject *_condition = this->parseLogical(OREF_NULL, TERM_IF);
    if (_condition == OREF_NULL)
    {        /* no expression here?               */
        if (type == KEYWORD_IF)            /* IF form?                          */
        {
            /* issue the IF message              */
            syntaxError(Error_Invalid_expression_if);
        }
        else
        {
            /* issue the WHEN message            */
            syntaxError(Error_Invalid_expression_when);
        }
    }
    RexxToken *token = nextReal();                  /* get terminator token              */
    previousToken();                     /* push the terminator back          */
                                         /* create a new translator object    */
    RexxInstruction *newObject =  new_instruction(IF, If);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionIf(_condition, token);
    newObject->setType(type);            /* set the IF/WHEN type              */
    return newObject; /* done, return this                 */
}


/**
 * Create a new interpret instruction instance.
 *
 * @return An executable INTERPRET instruction.
 */
RexxInstruction *LanguageParser::interpretNew()
{
    // this is pretty simple...parse the optional expresson and create the
    // instruction instance.
    RexxObject *_expression = expression(TERM_EOC);
    // the expression is required.
    if (_expression == OREF_NULL)
    {
        syntaxError(Error_Invalid_expression_interpret);
    }

    RexxInstruction *newObject = new_instruction(INTERPRET, Interpret);
    new ((void *)newObject) RexxInstructionInterpret(_expression);
    return newObject;
}


/**
 * Create a new label object.
 *
 * @param nameToken  The token that identifies the label name.
 * @param colonToken The token marking the colon.
 *
 * @return A new label object.
 */
RexxInstruction *LanguageParser::labelNew(RexxToken *nameToken, RexxToken *colonToken)
{
    // create a label instruction with this name.
    RexxString *name = nameToken->value();                /* get the label name                */
    // get a new instruction and add it to the label list under this name.
    RexxInstruction *newObject = new_instruction(LABEL, Label);
    /* add to the label list             */
    addLabel(newObject, name);

    // use the name object for tracking the location.
    SourceLocation location = colonToken->getLocation();
    // the clause ends with the colon.
    newObject->setEnd(location.getEndLine(), location.getEndOffset());
    // complete construction of this.
    new ((void *)newObject) RexxInstructionLabel();
    return newObject;
}

/**
 * Create a new ITERATE or LEAVE instruction.  These
 * two instructions share an implementation class.
 *
 * @param type   The type of instruction we're creating
 *
 * @return The instruction instance.
 */
RexxInstruction *LanguageParser::leaveNew(InstructionKeyword type)
{
    // The name is optional...
    RexxString *name = OREF_NULL;

    // anthing else on the instruction?
    RexxToken *token = nextReal();
    if (!token->isEndOfClause())
    {
        // must be a symbol
        if (!token->isSymbol())
        {
            syntaxError(type == KEYWORD_LEAVE ? Error_Symbol_expected_leave : Error_Symbol_expected_iterate);
        }
        // get the name and verify there is nothing else on the clause
        name = token->value();
        requiredEndOfClause(type == KEYWORD_LEAVE ? Error_Invalid_data_leave : Error_Invalid_data_iterate);
    }

    // allocate the object and return
    RexxInstruction *newObject = type == KEYWORD_LEAVE ? new_instruction(LEAVE, Leave) : new_instruction(ITERATE, Leave);
    new ((void *)newObject) RexxInstructionLeave(name);
    return newObject;
}


/**
 * Create a new Message instruction object.
 *
 * @param _message the message expression term...used as a standalone instruction.
 *
 * @return A message object.
 */
RexxInstruction *LanguageParser::messageNew(RexxExpressionMessage *msg)
{
    ProtectedObject p(msg);
    // just allocate and initialize the object.
    RexxInstruction *newObject = new_variable_instruction(MESSAGE, Message, sizeof(RexxInstructionMessage) + (msg->argumentCount - 1) * sizeof(RexxObject *));
    new ((void *)newObject) RexxInstructionMessage(msg);
    return newObject;
}


/**
 * Create a new message assignment object.
 *
 * @param msg    The message term that is the expression target.
 * @param expr   The expression to be evaluated for the assignment value.
 *
 * @return A new instruction to process this assignment.
 */
RexxInstruction *LanguageParser::messageAssignmentNew(RexxExpressionMessage *msg, RexxObject *expr)
{
    ProtectedObject p(msg);               // protect this
    msg->makeAssignment(this);            // convert into an assignment message
    // allocate a new object.  NB:  a message instruction gets an extra argument, so we don't subtract one.
    RexxInstruction *newObject = new_variable_instruction(MESSAGE, Message, sizeof(RexxInstructionMessage) + (msg->argumentCount) * sizeof(RexxObject *));
    new ((void *)newObject) RexxInstructionMessage(msg, expr);
    return newObject;
}


/**
 * Create a message term pseudo assignment instruction for a
 * shortcut operator.  The instruction is of the form
 * "messageTerm (op)= expr".
 *
 * @param message    The target message term.
 * @param operation  The operation to perform in the expansion.
 * @param expression The expression (which is the right-hand term of the eventual
 *                   expression).
 *
 * @return The constructed message operator.
 */
RexxInstruction *LanguageParser::messageAssignmentOpNew(RexxExpressionMessage *msg, RexxToken *operation, RexxObject *expr)
{
    ProtectedObject p(expr);        // the message term is already protected, also need to protect this portion
    // There are two things that go on now with the message term,
    // 1) used in the expression evaluation and 2) perform the assignment
    // part.  We copy the original message object to use as a retriever, then
    // convert the original to an assignment message by changing the message name.

    RexxObject *retriever = msg->copy();

    msg->makeAssignment(this);       // convert into an assignment message (the original message term)

    // now add a binary operator to this expression tree using the message copy.
    expr = (RexxObject *)new RexxBinaryOperator(operation->subtype(), retriever, expr);

    // allocate a new object.  NB:  a message instruction gets an extra argument, so we don't subtract one.
    RexxInstruction *newObject = new_variable_instruction(MESSAGE, Message, sizeof(RexxInstructionMessage) + (msg->argumentCount) * sizeof(RexxObject *));
    new ((void *)newObject) RexxInstructionMessage(msg, expr);
    return newObject;
}


/**
 * Create a nop object.
 *
 * @return a NOP instruction object.
 */
RexxInstruction *LanguageParser::nopNew()
{
    // this should be at the end of the clause.
    requiredEndOfClause(Error_Invalid_data_nop);

    // allocate and initialize the object.
    RexxInstruction *newObject = new_instruction(NOP, Nop);
    new ((void *)newObject) RexxInstructionNop;
    return newObject;
}

/**
 * Parse a NUMERIC instruction and create a processing
 * instruction object.
 *
 * @return A NUMERIC instruction object.
 */
RexxInstruction *LanguageParser::numericNew()
{
    RexxObject *_expression = OREF_NULL;
    bitset<32> _flags = 0;

    // get to the first real token of the instruction
    RexxToken *token = nextReal();

    // all forms of this start with an instruction sub keyword
    if (!token->isSymbol())
    {
        /* raise the error                   */
        syntaxError(Error_Symbol_expected_numeric, token);
    }

    // resolve the subkeyword value
    InstructionSubKeyword type = token->subKeyword();

    switch (type)
    {
        // NUMERIC DIGITS
        case SUBKEY_DIGITS:
            // set the function and parse the optional expression
            _flags[numeric_digits] = true;
            _expression = expression(TERM_EOC);
            break;

        // NUMERIC FUZZ
        case SUBKEY_FUZZ:
            // set the function and parse the optional expression
            _flags[numeric_fuzz] = true;
            _expression = expression(TERM_EOC);
            break;

        // NUMERIC FORM
        // The most complicated of these because it has
        // quite a flavors
        case SUBKEY_FORM:
        {
            // get the next token, skipping any white space
            token = nextReal();
            // Just NUMERIC FORM resets to the default
            if (token->isEndOfClause())
            {
                // reset to the default for this package
                _flags[numeric_form_default] = true;
                break;
            }
            // could be a keyword form
            else if (token->isSymbol())
            {
                // these are constant subkeywords, so resolve them and handle
                switch (subKeyword(token))
                {
                    // NUMERIC FORM SCIENTIFIC
                    case SUBKEY_SCIENTIFIC:
                        // check there's nothing extra
                        requiredEndOfClause(Error_Invalid_data_form);
                        break;

                    // NUMERIC FORM ENGINEERING
                    case SUBKEY_ENGINEERING:
                        // set the engineering flag
                        _flags[numeric_engineering];
                        // check for extra stuff
                        requiredEndOfClause(Error_Invalid_data_form);
                        break;

                    // NUMERIC FORM VALUE expr
                    case SUBKEY_VALUE:
                        _expression = expression(TERM_EOC);
                        // the expression os required
                        if (_expression == OREF_NULL)
                        {
                            syntaxError(Error_Invalid_expression_form);
                        }
                        break;

                    // invalid sub keyword
                    default:
                        syntaxError(Error_Invalid_subkeyword_form, token);
                        break;

                }
            }
            // Implicit NUMERIC FORM VALUE
            else
            {
                previousToken();
                _expression = expression(TERM_EOC);
            }
            break;
        }


        default:
            syntaxError(Error_Invalid_subkeyword_numeric, token);
            break;
    }

    // and create the instruction object.
    RexxInstruction *newObject = new_instruction(NUMERIC, Numeric);
    new ((void *)newObject) RexxInstructionNumeric(_expression, _flags);
    return newObject;
}


/**
 * Parse and create an instance of the OPTIONS instruction.
 *
 * @return An executable OPTIONS instance.
 */
RexxInstruction *LanguageParser::optionsNew()
{
    // this is pretty simple...parse the optional expresson and create the
    // instruction instance.
    RexxObject *_expression = expression(TERM_EOC);

    // the expression is required on OPTIONS.
    if (_expression == OREF_NULL)
    {
        syntaxError(Error_Invalid_expression_options);
    }

    RexxInstruction *newObject = new_instruction(OPTIONS, Options);
    new((void *)newObject) RexxInstructionOptions(_expression);
    return newObject;
}


RexxInstruction *LanguageParser::otherwiseNew(
  RexxToken  *token)                   /* OTHERWISE token                   */
/****************************************************************************/
/* Function:  Create an OTHERWISE instruction object                        */
/****************************************************************************/
{
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(OTHERWISE, Otherwise);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionOtherwise(token);
    return newObject; /* done, return this                 */
}

RexxInstruction *LanguageParser::parseNew(
  int argpull)                         /* type of parsing operation         */
/****************************************************************************/
/* Function:  Create a PARSE instruction object                             */
/****************************************************************************/
{

    RexxToken        *token;             /* current working token             */
    unsigned short    string_source;     /* source of string data             */

    size_t _flags = 0;                           /* new flag values                   */
    RexxObject *_expression = OREF_NULL;              /* zero out the expression           */
    if (argpull != KEYWORD_PARSE)
    {       /* have one of the short forms?      */
        string_source = argpull;            /* set the proper source location    */
        _flags |= parse_upper;              /* we're uppercasing                 */
    }
    else
    {                               /* need to parse source keywords     */
        int _keyword;
        for (;;)
        {                         /* loop through for multiple keywords*/
            token = nextReal();              /* get a token                       */
                                             /* not a symbol here?                */
            if (!token->isSymbol())
            {
                /* this is an error                  */
                syntaxError(Error_Symbol_expected_parse);
            }
            /* resolve the keyword               */
            _keyword = this->parseOption(token);

            switch (_keyword)
            {               /* process potential modifiers       */

                case SUBKEY_UPPER:             /* doing uppercasing?                */
                    if (_flags&parse_translate)   /* already had a translation option? */
                    {
                        break;                     /* this is an unrecognized keyword   */
                    }
                    _flags |= parse_upper;        /* set the translate option          */
                    continue;                    /* go around for another one         */

                case SUBKEY_LOWER:             /* doing lowercasing?                */
                    if (_flags&parse_translate)   /* already had a translation option? */
                    {
                        break;                     /* this is an unrecognized keyword   */
                    }
                                                   /* set the translate option          */
                    _flags |= parse_lower;        /* set the translate option          */
                    continue;                    /* go around for another one         */

                case SUBKEY_CASELESS:          /* caseless comparisons?             */
                    if (_flags&parse_caseless)    /* already had this options?         */
                    {
                        break;                     /* this is an unrecognized keyword   */
                    }
                                                   /* set the translate option          */
                    _flags |= parse_caseless;     /* set the scan option               */
                    continue;                    /* go around for another one         */
            }
            break;                           /* fall out into source processing   */
        }

        string_source = _keyword;          /* set the source                    */
        switch (_keyword)
        {                 /* now process the parse option      */

            case SUBKEY_PULL:                /* PARSE PULL instruction            */
            case SUBKEY_LINEIN:              /* PARSE LINEIN instruction          */
            case SUBKEY_ARG:                 /* PARSE ARG instruction             */
            case SUBKEY_SOURCE:              /* PARSE SOURCE instruction          */
            case SUBKEY_VERSION:             /* PARSE VERSION instruction         */
                break;                         /* this is already done              */

            case SUBKEY_VAR:                 /* PARSE VAR name instruction        */
                token = nextReal();            /* get the next token                */
                                               /* not a symbol token                */
                if (!token->isSymbol())
                {
                    /* this is an error                  */
                    syntaxError(Error_Symbol_expected_var);
                }
                /* get the variable retriever        */
                _expression = (RexxObject *)this->addVariable(token);
                this->saveObject(_expression);  /* protect it in the saveTable (required for large PARSE statements) */
                break;

            case SUBKEY_VALUE:               /* need to process an expression     */
                _expression = this->expression(TERM_WITH | TERM_KEYWORD);
                if (_expression == OREF_NULL )       /* If script not complete, report error RI0001 */
                {
                    //syntaxError(Error_Invalid_template_with);
                    _expression = OREF_NULLSTRING ;             // Set to NULLSTRING if not coded in script
                }
                this->saveObject(_expression);  /* protecting in the saveTable is better for large PARSE statements */

                token = nextToken();           /* get the terminator                */
                if (!token->isSymbol() || this->subKeyword(token) != SUBKEY_WITH)
                {
                    /* got another error                 */
                    syntaxError(Error_Invalid_template_with);
                }
                break;

            default:                         /* unknown (or duplicate) keyword    */
                /* report an error                   */
                syntaxError(Error_Invalid_subkeyword_parse, token);
                break;
        }
    }

    RexxQueue *parse_template = this->subTerms;     /* use the sub terms list template   */
    int templateCount = 0;                   /* no template items                 */
    RexxQueue *_variables = this->terms;            /* and the terms list for variables  */
    int variableCount = 0;                   /* no variable items                 */
    token = nextReal();                  /* get the next token                */

    RexxTrigger *trigger;

    for (;;)
    {                           /* while still in the template       */
                                /* found the end?                    */
        if (token->isEndOfClause())
        {
            /* string trigger, null target       */
            trigger = new (variableCount) RexxTrigger(TRIGGER_END, (RexxObject *)OREF_NULL, variableCount, _variables);
            variableCount = 0;               /* have a new set of variables       */
                                             /* add this to the trigger list      */
            parse_template->push((RexxObject *)trigger);
            templateCount++;                 /* and step the count                */
            break;                           /* done processing                   */
        }
        /* comma in the template?            */
        else if (token->classId == TOKEN_COMMA)
        {
            trigger = new (variableCount) RexxTrigger(TRIGGER_END, (RexxObject *)OREF_NULL, variableCount, _variables);
            variableCount = 0;               /* have a new set of variables       */
                                             /* add this to the trigger list      */
            parse_template->push((RexxObject *)trigger);
            parse_template->push(OREF_NULL); /* add an empty entry in the list    */
            templateCount += 2;              /* count both of these               */
        }
        /* some variety of special trigger?  */
        else if (token->classId == TOKEN_OPERATOR)
        {
            int trigger_type = 0;
            switch (token->subclass)
            {       /* process the operator character    */

                case  OPERATOR_PLUS:           /* +num or +(var) form               */
                    trigger_type = TRIGGER_PLUS; /* positive relative trigger         */
                    break;

                case  OPERATOR_SUBTRACT:       /* -num or -(var) form               */
                    trigger_type = TRIGGER_MINUS;/* negative relative trigger         */
                    break;

                case  OPERATOR_EQUAL:          /* =num or =(var) form               */
                    /* absolute column trigger           */
                    trigger_type = TRIGGER_ABSOLUTE;
                    break;

                case OPERATOR_LESSTHAN:        // <num or <(var)
                    trigger_type = TRIGGER_MINUS_LENGTH;
                    break;

                case OPERATOR_GREATERTHAN:     // >num or >(var)
                    trigger_type = TRIGGER_PLUS_LENGTH;
                    break;

                default:                       /* something unrecognized            */
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_template_trigger, token);
                    break;
            }
            token = nextReal();              /* get the next token                */
                                             /* have a variable trigger?          */
            if (token->classId == TOKEN_LEFT)
            {
                // parse off an expression in the parens.
                RexxObject *subExpr = this->parenExpression(token);
                // an expression is required
                if (subExpr == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_parse);
                }
                /* create the appropriate trigger    */
                trigger = new (variableCount) RexxTrigger(trigger_type, subExpr, variableCount, _variables);
                variableCount = 0;             /* have a new set of variables       */
                                               /* add this to the trigger list      */
                parse_template->push((RexxObject *)trigger);
                templateCount++;               /* add this one in                   */
            }
            /* have a symbol?                    */
            else if (token->isSymbol())
            {
                /* non-variable symbol?              */
                if (token->subclass != SYMBOL_CONSTANT)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_template_position, token);
                }
                /* set the trigger type              */
                /* create the appropriate trigger    */
                trigger = new (variableCount) RexxTrigger(trigger_type, this->addText(token), variableCount, _variables);
                variableCount = 0;             /* have a new set of variables       */
                                               /* add this to the trigger list      */
                parse_template->push((RexxObject *)trigger);
                templateCount++;               /* step the counter                  */
            }
            /* at the end?                       */
            else if (token->isEndOfClause())
            {
                /* report the missing piece          */
                syntaxError(Error_Invalid_template_missing);
            }
            else
            {
                /* general position error            */
                syntaxError(Error_Invalid_template_position, token);
            }
        }
        /* variable string trigger?          */
        else if (token->classId == TOKEN_LEFT)
        {
            // parse off an expression in the parens.
            RexxObject *subExpr = this->parenExpression(token);
            // an expression is required
            if (subExpr == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_parse);
            }
            /* create the appropriate trigger    */
            trigger = new (variableCount) RexxTrigger(_flags&parse_caseless ? TRIGGER_MIXED : TRIGGER_STRING, subExpr, variableCount, _variables);
            variableCount = 0;               /* have a new set of variables       */
                                             /* add this to the trigger list      */
            parse_template->push((RexxObject *)trigger);
            templateCount++;                 /* step the counter                  */
        }
        else if (token->isLiteral())
        {     /* non-variable string trigger?      */
              /* create the appropriate trigger    */
            trigger = new (variableCount) RexxTrigger(_flags&parse_caseless ? TRIGGER_MIXED : TRIGGER_STRING,
                                                      this->addText(token), variableCount, _variables);
            variableCount = 0;               /* have a new set of variables       */
                                             /* add this to the trigger list      */
            parse_template->push((RexxObject *)trigger);
            templateCount++;                 /* step the counter                  */
        }
        else if (token->isSymbol())
        {      /* symbol in the template?           */
               /* non-variable symbol?              */
            if (token->subclass == SYMBOL_CONSTANT)
            {
                /* create the appropriate trigger    */
                trigger = new (variableCount) RexxTrigger(TRIGGER_ABSOLUTE, this->addText(token), variableCount, _variables);
                variableCount = 0;             /* have a new set of variables       */
                                               /* add this to the trigger list      */
                parse_template->push((RexxObject *)trigger);
                templateCount++;               /* step the counter                  */
            }
            /* place holder period?              */
            else
            {
                if (token->subclass == SYMBOL_DUMMY)
                {
                    _variables->push(OREF_NULL);   /* just add an empty item            */
                    variableCount++;               /* step the variable counter         */
                }
                else                           /* have a variable, add to list      */
                {
                    // this is potentially a message term
                    previousToken();
                    RexxObject *term = variableOrMessageTerm();
                    if (term == OREF_NULL)
                    {
                        syntaxError(Error_Variable_expected_PARSE, token);
                    }
                    _variables->push(term);
                    variableCount++;               /* step the variable counter         */
                }
            }
        }
        else
        {
            /* this is invalid                   */
            syntaxError(Error_Invalid_template_trigger, token);
        }
        token = nextReal();                /* get the next token                */
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_variable_instruction(PARSE, Parse, sizeof(RexxInstructionParse) + (templateCount - 1) * sizeof(RexxObject *));
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionParse(_expression, string_source, _flags, templateCount, parse_template);
    this->toss(_expression);             /* release the expression, it is saved by newObject */
    return newObject;                    /* done, return this                 */
}

RexxInstruction *LanguageParser::procedureNew()
/****************************************************************************/
/* Function:  Create a new PROCEDURE translator object                      */
/****************************************************************************/
{
    RexxToken *token = nextReal();                  /* get the next token                */
    size_t variableCount = 0;                   /* no variables allocated yet        */
    if (!token->isEndOfClause())
    {   /* potentially PROCEDURE EXPOSE?     */
        if (!token->isSymbol())/* not a symbol?                     */
        {
            /* must be a symbol here             */
            syntaxError(Error_Invalid_subkeyword_procedure, token);
        }
        /* not the EXPOSE keyword?           */
        if (this->subKeyword(token) != SUBKEY_EXPOSE)
        {
            syntaxError(Error_Invalid_subkeyword_procedure, token);
        }
        /* go process the list               */
        variableCount = this->processVariableList(KEYWORD_PROCEDURE);
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_variable_instruction(PROCEDURE, Procedure, sizeof(RexxInstructionProcedure) + (variableCount - 1) * sizeof(RexxObject *));
    /* Initialize this new method        */
    new ((void *)newObject) RexxInstructionProcedure(variableCount, this->subTerms);
    return newObject; /* done, return this                 */
}


/**
 * Create an instance of the QUEUE instruction.
 *
 * @return An executable instruction object.
 */
RexxInstruction *LanguageParser::queueNew()
{
    // this is pretty simple...parse the optional expresson and create the
    // instruction instance.
    RexxObject *_expression = expression(TERM_EOC);
    RexxInstruction *newObject = new_instruction(QUEUE, Queue);
    new((void *)newObject) RexxInstructionQueue(_expression, type);
    return newObject;
}


/**
 * Create an instance of the PUSH instruction.
 *
 * @return An executable instruction object.
 */
RexxInstruction *LanguageParser::pushNew()
{
    // this is pretty simple...parse the optional expresson and create the
    // instruction instance.
    RexxObject *_expression = expression(TERM_EOC);

    // NOTE:  PUSH and QUEUE share an implementation class, but the
    // instruction type identifies which operation is performed.
    RexxInstruction *newObject = new_instruction(PUSH, Queue);
    new((void *)newObject) RexxInstructionQueue(_expression, type);
    return newObject;
}


/**
 * Parse a RAISE instruction and return a new executable object.
 *
 * @return An instructon object that can execute this RAISE instruction.
 */
RexxInstruction *LanguageParser::raiseNew()
{
    RexxArray *arrayItems = OREF_NULL
    RexxObject *rcValue = OREF_NULL;
    RexxObject *description = OREF_NULL;
    RexxObject *additional = OREF_NULL;
    RexxObject *result = OREF_NULL;
    bitset<32> flags;

    // ok, get the first TOKEN
    RexxToken *token = nextReal();
    // the first token is the condition name, and must be a symbol
    if (!token->isSymbol())
    {
        syntaxError(Error_Symbol_expected_raise);
    }

    RexxString *_condition = token->value();
    // make sure this matches one of the allowed condition names
    ConditionKeyword conditionType = token->condition();
    switch (conditionType)
    {
        // FAILURE, ERROR, and SYNTAX are pretty similar.  They take an
        // argument after the condition keyword.
        case CONDITION_FAILURE:
        case CONDITION_ERROR:
        case CONDITION_SYNTAX:
        {
            // SYNTAX requires some extra runtime processing, so set a
            // flag for that.
            if (conditionType == CONDITION_SYNTAX)
            {
                flags[raise_syntax] = true;
            }

            // this is a constant expression form.
            rcValue = constantExpression();
            // this is required.  If not found, use the terminator
            // token to report the error location
            if (rcVaue == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_general, nextToken());
            }
            // add this to the sub terms queue
            subTerms->push(rcValue);
            break;
        }

        // Raising a USER condition...this is a two part name.
        case CONDITION_USER:
        {
            token = nextReal();
            // next part must by a symbol
            if (!token->isSymbol())
            {
                syntaxError(Error_Symbol_expected_user);
            }
            // the condition name is actuall "USER condition", so construct
            // the composite
            _condition = token->value();
            _condition = _condition->concatToCstring(CHAR_USER_BLANK);
            // NB:  Common string protects this from garbage collection, so we
            // don't need to give it extra protection.
            _condition = commonString(_condition);
            break;
        }

        // no special processing for any of these
        case CONDITION_HALT:
        case CONDITION_NOMETHOD:
        case CONDITION_NOSTRING:
        case CONDITION_NOTREADY:
        case CONDITION_NOVALUE:
        case CONDITION_LOSTDIGITS:
            break;

        // we need to remember if this is propagate
        case CONDITION_PROPAGATE:          /* CONDITION PROPAGATE               */
            flags[raise_propagate] = true;
            break

        // could be ALL, or could be something completely unknown.
        default:
            syntaxError(Error_Invalid_subkeyword_raise, token);
            break;
    }

    // ok, lots of options to process, and we also need to check for dups/mutual exclusions (sigh)

    // process tokens until we hit the end.
    token = nextReal();
    while (!token->isEndOfClause())
    {
        // all options are symbol names,
        if (!token->isSymbol())
        {
            syntaxError(Error_Invalid_subkeyword_raiseoption, token);
        }

        // map the keyword name to a symbolic identifier.
        InstructionSubKeyword option = token->subKeyword);
        switch (option)
        {
            // RAISE .... DESCRIPTION expr
            case SUBKEY_DESCRIPTION:
            {
                // not valid if we've had this before
                if (description != OREF_NULL)
                {
                    syntaxError(Error_Invalid_subkeyword_description);
                }
                // this is constant expression form, and is required
                description = constantExpression();
                if (description == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_raise_description);
                }
                // protect from GC.
                subTerms->push(description);
                break;
            }

            // RAISE .... ADDITIONAL expr
            case SUBKEY_ADDITIONAL:
            {
                // can't have dups, and this is mutually exclusive with the ARRAY option.
                if (additional != OREF_NULL || arrayItems != OREF_NULL)
                {
                    syntaxError(Error_Invalid_subkeyword_additional);
                }
                // another constant expression form
                additional = constantExpression();
                if (additional == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_raise_additional);
                }
                subTerms->push(additional);
                break;
            }

            // RAISE ... ARRAY expr
            case SUBKEY_ARRAY:
            {
                // can only specified once, and ARRAY and ADDITIONAL are mutually exclusive
                if (additional != OREF_NULL || arrayItems != OREF_NULL)
                {
                    syntaxError(Error_Invalid_subkeyword_additional);
                }

                // this is not a conditional expression, the items must be
                // surrounded by parens.
                token = nextReal();
                if (!token->isType(TOKEN_LEFT))
                {
                    syntaxError(Error_Invalid_expression_raise_list);
                }
                // process this like an argument list.  Usually, we'd
                // leave this on the subTerms stack, but we're pushing
                // other items on there that will mess things up.  So,
                // we grab this in an array.
                arrayItems = argArray(token, TERM_RIGHT);
                subTerms->push(arrayItems)
                // remember this is the raise form.
                flags[raise_array] = true;
                break;
            }

            // RAISE ... RETURN <expr>
            // Two purposes here, specifies EXIT vs RETURN semantics and also
            // gives a value for the return.
            case SUBKEY_RETURN:
            {
                // have we hit a RETURN or exit before?  This is bad.
                if (flags[raise_return] || flags[raise_exit])
                {
                    syntaxError(Error_Invalid_subkeyword_result);
                }
                // remember which return type we need to use
                flags[raise_return] = true;
                // and get the return value
                result = constantExpression();
                // this is actually optional
                if (result != OREF_NULL)
                {
                    subTerms->push(result);
                }
                break;
            }

            // RAISE ... EXIT <expr>
            case SUBKEY_EXIT:
            {
                // check for duplicates
                if (flags[raise_return] || flags[raise_exit])
                {
                    syntaxError(Error_Invalid_subkeyword_result);
                }
                flags[raise_exit] = true;
                // get the optional keyword value
                result = this->constantExpression();
                if (result != OREF_NULL)
                {
                    subTerms->push(result);
                }
                break;
            }

            // and invalid subkeyword
            default:
                syntaxError(Error_Invalid_subkeyword_raiseoption, token);
                break;
        }
        token = nextReal();                /* step to the next keyword          */
    }

    RexxInstruction *newObject;

    // is this the array version?  need to dynamically allocate
    if (flags[raise_array])
    {
        size_t arrayCount = arrayItems->size();
        // we pass this as the additional...the flag tells us which to use
        additional = arrayItems;
        newObject = new_variable_instruction(RAISE, Raise, sizeof(RexxInstructionRaise) + (arrayCount - 1) * sizeof(RexxObject *));
    }
    // fixed instruction size
    else
    {
        newObject = new_instruction(RAISE, Raise);
    }
    new ((void *)newObject) RexxInstructionRaise(_condition, rcValue, description, additional, result, flags);
    return newObject;
}


/**
 * Create an instance of the REPLY instruction.
 *
 * @return An executable instruction instance.
 */
RexxInstruction *LanguageParser::replyNew()
{
    // reply is not allowed in interpret
    if (isInterpret())
    {
        syntaxError(Error_Translation_reply_interpret);
    }

    // this is pretty simple...parse the optional expresson and create the
    // instruction instance.
    RexxObject *_expression = expression(TERM_EOC);
    RexxInstruction *newObject = new_instruction(REPLY, Reply);
    new ((void *)newObject) RexxInstructionReply(_expression);
    return newObject;
}


/**
 * Parse and create a RETURN instruction.
 *
 * @return An instance of the RETURN instruction.
 */
RexxInstruction *LanguageParser::returnNew()
{
    // this is pretty simple...parse the optional expresson and create the
    // instruction instance.
    RexxObject *_expression = expression(TERM_EOC);
    RexxInstruction *newObject = new_instruction(RETURN, Return);
    new ((void *)newObject) RexxInstructionReturn(_expression);
    return newObject;
}


/**
 * Parse and create a SAY instruction.
 *
 * @return An executable instance of the SAY instruction.
 */
RexxInstruction *LanguageParser::sayNew()
{
    // this is pretty simple...parse the optional expresson and create the
    // instruction instance.
    RexxObject *_expression = expression(TERM_EOC);
    RexxInstruction *newObject = new_instruction(SAY, Say);
    new ((void *)newObject) RexxInstructionSay(_expression);
    return newObject;
}

RexxInstruction *LanguageParser::selectNew()
/****************************************************************************/
/* Function:  Create a SELECT instruction object                            */
/****************************************************************************/
{
    // SELECT can be either SELECT; or SELECT LABEL name;
    // for saved image compatibility, we have different classes for this
    // depending on the form used.
    RexxToken *token = nextReal();
    // easy version, no LABEL.
    if (token->isEndOfClause())
    {
        // just a simple SELECT type
        RexxInstruction *newObject = new_instruction(SELECT, Select);
        new ((void *)newObject) RexxInstructionSelect(OREF_NULL);
        return newObject;
    }
    else if (!token->isSymbol())
    {
        // not a LABEL keyword, this is bad
        syntaxError(Error_Invalid_data_select, token);
    }

    // potentially a label.  Check the keyword value
    if (this->subKeyword(token) != SUBKEY_LABEL)
    {
                                       /* raise an error                    */
        syntaxError(Error_Invalid_subkeyword_select, token);
    }
    // ok, get the label now
    token = nextReal();
    // this is required, and must be a symbol
    if (!token->isSymbol())
    {
        syntaxError(Error_Symbol_expected_LABEL);
    }

    RexxString *label = token->value;
    requiredEndOfClause(Error_Invalid_data_select);

    // ok, finally allocate this and return
    RexxInstruction *newObject = new_instruction(SELECT, Select);
    new ((void *)newObject) RexxInstructionSelect(label);
    return  newObject;
}


/**
 * We've encountered a SIGNAL VALUE or an implicit SIGNAL value
 * version of a SIGNAL instruction.  This has a separate
 * instruction object tailored to that function, so complete
 * parsing of this instruction and return an instance of that
 * instruction object.
 *
 * @return An instruction object for processing a SIGNAL VALUE
 */
RexxInstruction *LanguageParser::dynamicSignalNew()
{
    // we are already positioned for processing the label expression, so
    // parse it off now.
    RexxObject *labelExpression = expression(TERM_EOC);
    // we must have something here.
    if (labelExpression == OREF_NULL)
    {
        syntaxError(Error_Invalid_expression_signal);
    }

    // create a new instruction object
    RexxInstruction *newObject = new_instruction(SIGNAL_VALUE, DynamicSignal);
    new ((void *)newObject) RexxInstructionDynamicSignal(labelExpression);

    // NOTE:  because this uses dynamic resolution, this does not get
    // added to the reference list for processing.  There is nothing that
    // can be resolved at this time.
    return newObject;
}


/**
 * Complete parsing of a SIGNAL ON or SIGNAL OFF instruction.
 * This also has a separate instruction object for executing
 * this function.
 *
 * @param type   The keyword type of the ON or OFF function.
 *
 * @return A new instruction object.
 */
RexxInstruction *LanguageParser::signalOnNew(InstructionSubKeyword type)
{
    // The processing of the CONDITION name is the same for both SIGNAL ON
    // and SIGNAL OFF

    RexxString *labelName;
    RexxString *conditionName;

    // we must have a symbol following, otherwise this is an error.
    RexxToken *token = nextReal();
    if (!token->isSymbol())
    {
        syntaxError(type == SUBKEY_ON ? Error_Symbol_expected_on : Error_Symbol_expected_off);
    }

    // get the condition involved
    ConditionKeyword condType = token->condition();
    // invalid condition specified?  another error
    if (condType == CONDITION_NONE || condType == CONDITION_PROPAGATE)
    {
        syntaxError(type == SUBKEY_ON ? Error_Invalid_subkeyword_signalon : Error_Invalid_subkeyword_signaloff, token);
    }
    // USER conditions need a little more work.
    else if (condType == CONDITION_USER)
    {
        // The condition name follows the USER keyword.
        // This must be a symbol and is required.
        token = nextReal();
        if (!token->isSymbol())
        {
            syntaxError(Error_Symbol_expected_user);
        }
        // get the User condition name.  That is the default target for the
        // signal trap
        labelName = token->value();
        // The condition name for this instruction is "USER condition", so
        // construct that now and make it a common string (likely used
        // other places in the program)
        conditionName = commonString(labelName->concatToCstring(CHAR_USER_BLANK));
    }
    else
    {
        // this is one of the language defined conditions, use this for both
        // the name and the condition...the name
        labelName = token->value();
        conditionName = labelName;
    }

    // IF This is signal ON, we can have an optional signal target
    // specified with the NAME option.
    if (type == SUBKEY_ON)
    {
        // ok, we can have a NAME keyword after this
        token = nextReal();
        if (!token->isEndOfClause())
        {
            // keywords are always symbols    */
            if (!token->isSymbol())
            {
                syntaxError(Error_Invalid_subkeyword_signalonname, token);
            }
            // only subkeyword allowed here is NAME
            if (token->subKeyword() != SUBKEY_NAME)
            {
                syntaxError(Error_Invalid_subkeyword_signalonname, token);
            }
            // we need a label name after this as a string or symbol
            token = nextReal();
            if (!token->isSymbolOrLiteral())
            {
                syntaxError(Error_Symbol_or_string_name);
            }
            // this overrides the default label taken from the condition name.
            labelName = token->value;
            // nothing more permitted after this
            requiredEndOfClause(Error_Invalid_data_name);
        }
    }
    else
    {
        // SIGNAL OFF doesn't have a label name, so make sure this is turned off.
        // We use this NULL value to determined which function to perform.
        labelName = OREF_NULL;

        // must have the end of clause here.
        requiredEndOfClause(Error_Invalid_data_condition);
    }

    // create a new instruction object
    RexxInstruction *newObject = new_instruction(SIGNAL_ON, SignalOn);
    new ((void *)newObject) RexxInstructionSignalOn(labelExpression);

    // if this is the ON form, we have some end parsing resolution to perform.
    if (type == SUBKEY_ON)
    {
        addReference((RexxObject *)newObject);
    }

    return newObject;
}


/**
 * Finish parsing a SIGNAL instruction and return
 * an appropriate instruction object.  The SIGNAL instruction
 * performs several distinct functions, so there are 3 separate
 * instruction objects tailored to thos functions.
 *
 * @return An instruction object for processing a SIGNAL.
 */
RexxInstruction *LanguageParser::signalNew()
{
    RexxString *labelName = OREF_NULL;       // this is our target label name (default is condition name)

    // now to work.
    RexxToken *token = nextReal();
    // no target, that's a paddling...
    if (token->isEndOfClause())
    {
        syntaxError(Error_Symbol_or_string_signal);
    }
    // might be some expression form, which is an implicit SIGNAL VALUE
    else if (!token->isSymbolOrLiteral())
    {
        // step back for the expression processor.
        previousToken();
        // process this as a dynamic signal instruction
        return dynamicSignalNew()
    }
    else
    {
        // A static target, although we need to check on the ON/OFF forms.
        if (token->isSymbol())
        {
            // check for a potential subkeyword
            InstructionSubKeyword keyword = token->subKeyword();
            // SIGNAL ON condition or SIGNAL OFF condition
            if (keyword == SUBKEY_ON || keyword == SUBKEY_OFF)
            {
                // this is a separate instruction form, go handle it.
                return signalOnNew(keyword);
            }
            // explicit VALUE form?
            else if (keyword == SUBKEY_VALUE)
            {
                // process this as a dynamic signal instruction too.  Again, we're
                // positioned at the correct location for evaluating this.
                return dynamicSignalNew()
            }
            // just an old boring SIGNAL to a label.
            else
            {
                // this is the symbol form
                labelName = token->value();
                // and nothing is allowed after this
                requiredEndOfClause(Error_Invalid_data_signal);
            }
        }
        // SIGNAL with a string target
        else
        {
            labelName = token->value();
            // and nothing is allowed after this
            requiredEndOfClause(Error_Invalid_data_signal);
        }
    }

    // create a new instruction object
    RexxInstruction *newObject = new_instruction(SIGNAL, Signal);
    new ((void *)newObject) RexxInstructionSignal(labelName);

    // this requires a resolve call back once the labels are determined.
    addReference((RexxObject *)newObject);
    return newObject;
}

RexxInstruction *LanguageParser::thenNew(
     RexxToken         *token,         /* THEN keyword token                */
     RexxInstructionIf *parent )       /* target parent IF or WHEN clause   */
/****************************************************************************/
/* Function:  Create a THEN instruction object                              */
/****************************************************************************/
{
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(THEN, Then);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionThen(token, parent);
    return newObject; /* done, return this                 */
}

/**
 * Parse and create a new Trace instruction object.
 *
 * @return A Trace instruction object.
 */
RexxInstruction *LanguageParser::traceNew()
{
    size_t setting = TRACE_NORMAL;              // set default trace mode
    wholenumber_t debug_skip = 0;               // no skipping
    size_t trcFlags = 0;                        // no translated flags
    RexxObject *_expression = OREF_NULL;        // not expression form

    // ok, start processing for real
    RexxToken *token = nextReal();              // get the next token

    // TRACE by itself is TRACE Normal.  Not really highly used
    if (!token->isEndOfClause())
    {
        // most trace settings are symbols, but VALUE is a special subkeyword
        if (token->isSymbol())
        {
            // TRACE VALUE expr?
            if (token->subKeyword() == SUBKEY_VALUE)
            {
                // This is an required expression
                _expression = expression(TERM_EOC);
                if (_expression == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_trace);
                }
            }
            else
            {
                // we have a symbol here, this should be the trace setting
                RexxString *value = token->value();
                // and nothing else after the symbol
                requiredEndOfClause(Error_Invalid_data_trace);

                // if this is not numeric, this is a TRACE option, with potential
                // ? prefix.
                if (!value->requestNumber(debug_skip, number_digits()))
                {
                    debug_skip = 0;
                    char badOption = 0;
                    // go parse the trace setting values
                    if (!parseTraceSetting(value, setting, trcFlags, badOption))
                    {
                        syntaxError(Error_Invalid_trace_trace, new_string(&badOption, 1));
                    }
                }
                else
                {
                    setting = 0;                 /* not a normal setting situation    */
                }
            }
        }
        // the trace setting can also be a string
        else if (token->isLiteral())
        {
            RexxString *value = token->value();
            // again, that can be the only thing
            requiredEndOfClause(Error_Invalid_data_trace);
            // same checks as above
            if (!value->requestNumber(debug_skip, number_digits()))
            {
                debug_skip = 0;
                char badOption = 0;

                if (!parseTraceSetting(value, setting, trcFlags, badOption))
                {
                    syntaxError(Error_Invalid_trace_trace, new_string(&badOption, 1));
                }
            }
            else
            {
                setting = 0;                   /* not a normal setting situation    */
            }
        }
        // potential numeric value with a sign?
        else if (token->isSubtype(OPERATOR_SUBTRACT, OPERATOR_PLUS))
        {
            setting = 0;
            // minus form?
            if (token->isSubtype(OPERATOR_SUBTRACT))
            {
                // turns off tracing entirely
                setting |= DEBUG_NOTRACE;
            }

            // we expect to find a number
            token = nextReal();
            if (token->isEndOfClause())
            {
                syntaxError(Error_Invalid_expression_general, token);
            }
            // this must be a symbol or a literal value
            if (!token->isSymbolOrLiteral())
            {
                syntaxError(Error_Invalid_expression_general, token);
            }

            // this needs to be the end of the clause
            RexxString *value = token->value();
            requiredEndOfClause(Error_Invalid_data_trace);

            // convert to a binary number
            if (!value->requestNumber(debug_skip, number_digits()))
            {
                syntaxError(Error_Invalid_whole_number_trace, value);
            }
        }
        // implicit TRACE VALUE form
        else
        {
            // take a step back and parse the expression
            previousToken();
            _expression = expression(TERM_EOC);
        }
    }

    RexxInstruction *newObject = new_instruction(TRACE, Trace);
    new ((void *)newObject) RexxInstructionTrace(_expression, setting, trcFlags, debug_skip);
    return newObject;
}

/**
 * Parse a USE STRICT ARG instruction.
 *
 * @return The executable instruction object.
 */
RexxInstruction *LanguageParser::useNew()
{
    // TODO:  I'm not sure why this restriction is here...need to check this out.

    if (isInterpret())
    {
        syntaxError(Error_Translation_use_interpret);
    }

    bool strictChecking = false;  // no strict checking enabled yet

    // The STRICT keyword turns this into a different instruction with different
    // syntax rules
    RexxToken *token = nextReal();
    int subkeyword = this->subKeyword(token);

    if (subkeyword == SUBKEY_STRICT)
    {
        token = nextReal();        // skip over the token
        strictChecking = true;     // apply the strict checks.
    }

    // the only subkeyword supported is ARG
    if (subKeyword(token) != SUBKEY_ARG)
    {
        syntaxError(Error_Invalid_subkeyword_use, token);
    }

    // we accumulate 2 sets of data here, so we need 2 queues to push them in
    // if this is the SIMPLE version, the second queue will be empty.
    size_t variableCount = 0;
    RexxQueue *variable_list = new_queue();         // we might be parsing message terms, so we can't use the subterms list.
    ProtectedObject p1(variable_list);
    RexxQueue *defaults_list = new_queue();
    ProtectedObject p2(defaults_list);
    token = nextReal();                  /* get the next token                */

    bool allowOptionals = false;  // we don't allow trailing optionals unless the list ends with "..."
    // keep processing tokens to the end
    while (!token->isEndOfClause())
    {
        // this could be a token to skip a variable
        if (token->classId == TOKEN_COMMA)
        {
            // this goes on as a variable, but an empty entry to process.
            // we also need to push empty entries on the other queues to keep everything in sync.
            variable_list->push(OREF_NULL);
            defaults_list->push(OREF_NULL);
            variableCount++;
            // step to the next token, and go process more
            token = nextReal();
            continue;
        }
        else   // something real.  This could be a single symbol or a message term
        {
            // we might have an ellipsis (...) on the end of the list meaning stop argument checking at that point
            if (token->isSymbol())
            {
                // is this an ellipsis symbol?
                if (token->value->strCompare(CHAR_ELLIPSIS))
                {
                    // ok, this is the end of everything.  Tell the instructions to not enforce the max rules
                    allowOptionals = true;
                    // but we still need to make sure it's at the end
                    token = nextReal();
                    if (!token->isEndOfClause())
                    {
                        syntaxError(Error_Translation_use_strict_ellipsis);
                    }
                    break;  // done parsing
                }
            }


            previousToken();       // push the current token back for term processing
            // see if we can get a variable or a message term from this
            RexxObject *retriever = variableOrMessageTerm();
            if (retriever == OREF_NULL)
            {
                syntaxError(Error_Variable_expected_USE, token);
            }
            variable_list->push(retriever);
            variableCount++;
            token = nextReal();
            // a terminator takes us out.  We need to keep all 3 lists in sync with dummy entries.
            if (token->isEndOfClause())
            {
                defaults_list->push(OREF_NULL);
                break;
            }
            // if we've hit a comma here, step to the next token and continue with the next variable
            else if (token->classId == TOKEN_COMMA)
            {
                defaults_list->push(OREF_NULL);
                token = nextReal();
                continue;
            }
            // if this is NOT a comma, we potentially have a
            // default value
            if (token->subclass == OPERATOR_EQUAL)
            {
                // this is a constant expression value.  Single token forms
                // are fine without parens, more complex forms require parens as
                // delimiters.
                RexxObject *defaultValue = constantExpression();
                // no expression is an error
                if (defaultValue == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_use_strict_default);
                }

                // add this to the defaults
                defaults_list->push(defaultValue);
                // step to the next token
                token = nextReal();
                // a terminator takes us out.  We need to keep all 3 lists in sync with dummy entries.
                if (token->isEndOfClause())
                {
                    break;
                }
                // if we've hit a comma here, step to the next token and continue with the next variable
                else if (token->classId == TOKEN_COMMA)
                {
                    token = nextReal();
                    continue;
                }
            }
            else
            {
                // if not an assignment, this needs to be a comma.
                syntaxError(Error_Variable_reference_use, token);
            }
        }
    }

    RexxInstruction *newObject = new_variable_instruction(USE, Use, sizeof(RexxInstructionUseStrict) + (variableCount == 0 ? 0 : (variableCount - 1)) * sizeof(UseVariable));
    new ((void *)newObject) RexxInstructionUseStrict(variableCount, strictChecking, allowOptionals, variable_list, defaults_list);

    return newObject;
}


/**
 * Validate placement of an EXPOSE instruction.  The EXPOSE must
 * be the first instruction and this must not be an interpret
 * invocation.  NOTE:  labels are not allowed preceeding, as that
 * will give a target for SIGNAL or CALL that will result in an
 * invalid EXPOSE execution.
 */
void LanguageParser::isExposeValid()
{
    // expose is never allowed in an interpret
    if (isInterpret())
    {
        syntaxError(Error_Translation_expose_interpret);
    }

    // the last instruction in the chain must be our dummy
    // first instruction
    if (!lastInstruction->isType(KEYWORD_FIRST))
    {
        syntaxError(Error_Translation_expose);
    }
}


/**
 * Process a list of variables for PROCEDURE, DROP, and EXPOSE.
 *
 * @param type   The type of instruction we're parsing for.
 *
 * @return The count of variables.  The variable retrievers are
 *         pushed onto the subTerm stack.
 */
size_t LanguageParser::processVariableList(InstructionKeyword type )
{
    size_t listCount = 0;

    // the next real token is the start of the list (after the
    // space following the keyword instruction.
    RexxToken *token = nextReal();

    // while not at the end of the clause
    while (!token->isEndOfClause())
    {
        // generally, these are symbols, but not all symbols are variables.
        if (token->isSymbol())
        {
            // non-variable symbol?
            if (token->isSubtype( SYMBOL_CONSTANT))
            {
                syntaxError(Error_Invalid_variable_number, token);
            }
            // the dummy period
            else if (token->isSubtype(SYMBOL_DUMMY))
            {
                syntaxError(Error_Invalid_variable_period, token);
            }

            // ok, get a retriever for the variable and push it on the stack.
            subTerms->push(addText(token));

            // are we processing an expose instruction?  keep track of this variable
            // in case we use GUARD WHEN
            if (type == KEYWORD_EXPOSE)
            {
                expose(token->value);
            }
            // update our return value.
            listCount++;
        }
        // this could be an indirect reference through a list of form "(var)"
        else if (token->isType(TOKEN_LEFT))
        {
            listCount++
            // get the next token...which should be a valid variable
            token = nextReal();

            if (!token->isSymbol())
            {
                syntaxError(Error_Symbol_expected_varref);
            }
            // non-variable symbol?
            if (token->isSubtype(SYMBOL_CONSTANT))
            {
                syntaxError(Error_Invalid_variable_number, token);
            }
            // the dummy dot?
            else if (token->isSubtype(SYMBOL_DUMMY))
            {
                syntaxError(Error_Invalid_variable_period, token);
            }

            // get a retriever for the variable in the parens
            RexxObject *retriever = addText(token);
            // and wrap this in a variable reference, which handles the indirection.
            retriever = (RexxObject *)new RexxVariableReference((RexxVariableBase *)retriever);
            // add this to the queue
            subTerms->queue(retriever);

            // make sure we have the closing paren
            token = nextReal();
            if (token->isEndOfClause())
            {
                /* report the missing paren          */
                syntaxError(Error_Variable_reference_missing);
            }
            // must be a right paren here
            else if (!token->isSubtype(TOKEN_RIGHT))
            {
                syntaxError(Error_Variable_reference_extra, token);
            }
        }
        // something unrecognized...we need to issue the message for the instruction.
        else
        {
            if (type == KEYWORD_DROP)
            {
                syntaxError(Error_Symbol_expected_drop);
            }
            else
            {
                syntaxError(Error_Symbol_expected_expose);
            }
        }
        // and see if we have more variables
        token = nextReal();
    }

    // we should have at least one variable...
    if (listCount == 0)
    {
        if (type == KEYWORD_DROP)
        {
            syntaxError(Error_Symbol_expected_drop);
        }
        else
        {
            syntaxError(Error_Symbol_expected_expose);
        }
    }
    // return our counter
    return listCount:
}


/**
 * Allow for WHILE or UNTIL keywords following some other looping
 * construct.  This returns SUBKEY_WHILE or SUBKEY_UNTIL to flag
 * the caller that a conditional has been used.
 *
 * @param condition_type
 *               An option pointer for a place to return the condition type (WHILE or UNTIL)
 * @param error_message
 *
 * @return A parsed out expression object.
 */
RexxObject *LanguageParser::parseLoopConditional(InstructionSubKeyword *conditionType, int error_message )
{
    RexxObject *conditional = OREF_NULL;

    // no type detected yet
    conditionType = SUBKEY_NONE;

    // get the next token...this should be either EOC or one of WHILE/UNTIL keywords.
    RexxToken *token = nextReal();

    // real end of instruction?  Nothing to return.  This is frequently called
    // in situations where we want to check for a trailing conditional
    if (token->isEndOfClause())
    {
        return OREF_NULL;
    }

    // if we have a symbol, this is potentially WHILE or UNTIL.
    if (token->isSymbol())
    {
        // get the subkeyword code
        switch (token->subKeyword() )
        {
            // WHILE exprw
            case SUBKEY_WHILE:              /* DO WHILE exprw                    */
            {
                // parse off the conditional.  We only recognize other conditional keywords
                // as terminators after this.  Since we had the keyword, the
                // epression is required.
                conditional = requiredLogicalExpression(OREF_NULL, TERM_COND, Error_Invalid_expression_while);
                // must not be anthing after this
                requiredEndOfClause(Error_Invalid_do_whileuntil);
                // and record the type.
                conditionType = SUBKEY_WHILE;
                break;
            }

            // UNTIL expru
            case SUBKEY_UNTIL:
            {
                // pretty much the same as the WHILE situation
                conditional = requiredLogicalExpression(OREF_NULL, TERM_COND, Error_Invalid_expression_until);
                // must not be anthing after this
                requiredEndOfClause(Error_Invalid_do_whileuntil);
                // and record the type.
                conditionType = SUBKEY_UNTIL;
                break;
            }

            // nothing else is valid here.
            default:
                syntaxError(error_message, token);
                break;
        }
    }
    return conditional;
}
