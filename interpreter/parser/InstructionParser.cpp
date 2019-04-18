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
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "LanguageParser.hpp"

#include "ExpressionMessage.hpp"
#include "ExpressionOperator.hpp"
#include "IndirectVariableReference.hpp"

#include "RexxInstruction.hpp"                /* base REXX instruction class       */

#include "AssignmentInstruction.hpp"          /* keyword <expression> instructions */
#include "CommandInstruction.hpp"
#include "ExitInstruction.hpp"
#include "InterpretInstruction.hpp"
#include "OptionsInstruction.hpp"
#include "ReplyInstruction.hpp"
#include "ReturnInstruction.hpp"
#include "SayInstruction.hpp"

#include "AddressInstruction.hpp"             /* other instructions                */
#include "AddressWithInstruction.hpp"
#include "CommandIOConfiguration.hpp"
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
#include "UseInstruction.hpp"
#include "UseLocalInstruction.hpp"

#include "CallInstruction.hpp"                 /* call/signal instructions          */
#include "SignalInstruction.hpp"

#include "ParseInstruction.hpp"                /* parse instruction and associates  */
#include "ParseTarget.hpp"
#include "ParseTrigger.hpp"

#include "ElseInstruction.hpp"                 /* control type instructions         */
#include "EndIf.hpp"
#include "IfInstruction.hpp"
#include "ThenInstruction.hpp"
#include "WhenCaseInstruction.hpp"

#include "DoBlock.hpp"                         /* block type instructions           */
#include "DoInstruction.hpp"
#include "EndInstruction.hpp"
#include "OtherwiseInstruction.hpp"
#include "SelectInstruction.hpp"
#include "ProtectedObject.hpp"
#include "UseArgVariableRef.hpp"


/**
 * Process an individual Rexx clause and decide what
 * type of instruction it represents.
 *
 * @return An executable instruction object, or OREF_NULL if
 *         we've reached the end of a code block.
 */
RexxInstruction* LanguageParser::nextInstruction()
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
    // empty the term stack also
    terms->empty();
    // also clear the stack count for each new instruction
    currentStack = 0;


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
        second = nextToken();

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
            return assignmentOpNew(first, second);
        }
    }

    // some other type of instruction we need to skip over the first
    // term of the instruction to determine the type of clause,
    // including recognition of keyword instructions

    // reset to the beginning and parse off the first term (which could be a
    // message send)
    firstToken();
    RexxInternalObject *term = parseMessageTerm();
    // a lot depends on the nature of the second token
    second = nextToken();

    // some sort of recognizable message term?  Need to check for the
    // special cases.
    if (term != OREF_NULL)
    {
        // if parsing the message term consumed everything, this is a message instruction
        if (second->isEndOfClause())
        {
            RexxExpressionMessage *msgTerm = (RexxExpressionMessage *)term;

            return msgTerm->isDoubleTilde() ? doubleMessageNew(msgTerm) : messageNew(msgTerm);
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
            RexxInternalObject *subexpression = parseSubExpression(TERM_EOC);
            if (subexpression == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_general, second);
            }
            // this is a message assignment
            return messageAssignmentNew((RexxExpressionMessage *)term, subexpression);
        }
        // one of the special operator forms?
        else if (second->isType(TOKEN_ASSIGNMENT))
        {
            ProtectedObject p(term);
            // we need an expression following the op token
            RexxInternalObject *subexpression = parseSubExpression(TERM_EOC);
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

    if (keyword != KEYWORD_NONE)
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
                return pushNew();
                break;

                // QUEUE instruction
            case KEYWORD_QUEUE:
                return queueNew();
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
                return ifNew();
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
                return parseNew(SUBKEY_NONE);
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
                // a different target instruction.
                return whenNew();
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

                // invalid KEYWORD (should really never happen)
            default:
                reportException(Error_Interpretation_switch, "keyword", keyword);
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
RexxInstruction* LanguageParser::sourceNewObject(size_t size, RexxBehaviour *_behaviour,
                                                 InstructionKeyword type)
{
    RexxInternalObject *newObject = new_object(size);
    newObject->setBehaviour(_behaviour);
    ::new((void *)newObject)RexxInstruction(clause, type);
    currentInstruction = (RexxInstruction *)newObject;
    return (RexxInstruction *)newObject;
}


/**
 * Create a "raw" executable instruction object of variable
 * size.  Allocation/creation of instruction objects are handled
 * a little differently, and goes in stages: 1) Have RexxMemory
 * allocate a Rexx object of the appropriate size.  2) Set the
 * object behaviour to be the table for the target instruction.
 * 3) Call the in-memory new() method on this object using the
 * RexxInstruction() constructor.  This does base initialization
 * of the object as an instruction object.  This version is
 * returned to the caller, with this instruction anchored in the
 * parser object to protect it from garbage collection, which is
 * handy if its constructor needs to allocate any additional
 * objects.<p>
 * Step 4) happens once the caller receives this object
 * instance.  It agains calls the new() in-memory allocator for
 * the final instruction class.  The constructor fills in any
 * instruction specific information, as well as changing the
 * object virtual function table to the final version.  The
 * information we set up here will remain untouched.
 *
 * @param size       The base size of the object type.
 * @param count      The count of extra items to add to the object instance.
 * @param itemSize   The size of the extra items to add.
 * @param _behaviour The Rexx object behaviour.
 * @param type       The type identifiers for the instruction
 *                   (see RexxToken InstructionKeyword enum).
 *
 * @return A newly created instruction object with basic instruction initialization.
 */
RexxInstruction* LanguageParser::sourceNewObject(size_t size, size_t count, size_t itemSize,
                                                 RexxBehaviour *_behaviour, InstructionKeyword type)
{
    // for variable size instructions, there is always an extra field at the end of the object
    // for the first slot allocation.  If the count is zero, then we subtract the item size from
    // the base size.  Otherwise, we add count - 1 extra items to the allocation.
    if (count == 0)
    {
        size -= itemSize;
    }
    else
    {
        size += (count - 1) * itemSize;
    }

    // go allocate the actual object.
    return sourceNewObject(size, _behaviour, type);
}


/**
 * Parse an ADDRESS instruction and create an executable instruction object.
 *
 * @return An instruction object that can perform this function.
 */
RexxInstruction* LanguageParser::addressNew()
{
    RexxInternalObject *dynamicAddress = OREF_NULL;
    RexxString *environment = OREF_NULL;
    RexxInternalObject *command = OREF_NULL;
    RexxToken *token = nextReal();
    Protected<CommandIOConfiguration> ioConfig;

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
            dynamicAddress = parseExpression(TERM_EOC | TERM_WITH | TERM_KEYWORD);
            // Now check to see if we ended with the WITH keyword, if so, .
            // we need to create an AddressWithInstruction rather than the
            // normal AddressInstruction object.
            token = nextToken();
            if (token->subKeyword() == SUBKEY_WITH)
            {
                ioConfig = parseAddressWith();
            }
        }
        else
        {
            // could be ADDRESS VALUE (NOTE:  Subkeyword also
            // checks that this is a SYMBOL token.
            if (token->subKeyword() == SUBKEY_VALUE)
            {
                // get the value expression
                dynamicAddress = requiredExpression(TERM_EOC | TERM_WITH | TERM_KEYWORD, Error_Invalid_expression_address);
                // Now check to see if we ended with the WITH keyword, if so, .
                // we need to create an AddressWithInstruction rather than the
                // normal AddressInstruction object.
                token = nextToken();
                if (token->subKeyword() == SUBKEY_WITH)
                {
                    ioConfig = parseAddressWith();
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
                    // the command expression could be terminated by a WITH keyword,
                    // in which case, we need to worry about command redirection,
                    // which is a much more complicated parsing process.
                    command = parseExpression(TERM_EOC | TERM_WITH | TERM_KEYWORD);
                    // This could have been ADDRESS env WITH (i.e., no expresson)
                    // which is set a configuration set, not a command .

                    // the bit above will have terminated with either the WITH keyword
                    // or the end of the clause. We know this will return something
                    // for an expression if it started with anything other than WITH,
                    // so the next token is either the symbol WITH or the end of cloase
                    // Now check to see if we ended with the WITH keyword, if so, .
                    // we end up attaching a configuration ruction rather than the
                    // normal AddressInstruction object.
                    token = nextToken();
                    if (token->subKeyword() == SUBKEY_WITH)
                    {
                        ioConfig = parseAddressWith();
                    }
                }
            }
        }
    }

    // if we don't have an io config, then use the older instruction version for
    // program save/restore compatibility
    if (ioConfig == OREF_NULL)
    {
        RexxInstruction *newObject = new_instruction(ADDRESS, Address);
        ::new((void *)newObject)RexxInstructionAddress(dynamicAddress, environment, command);
        return newObject;
    }
    else
    {
        RexxInstruction *newObject = new_instruction(ADDRESS, AddressWith);
        ::new((void *)newObject)RexxInstructionAddressWith(dynamicAddress, environment, command, ioConfig);
        return newObject;
    }
}


/**
 * Complete processing of an ADDRESS instruction that
 * has specified WITH I/O redirection.
 *
 * @param environment
 *                The already parsed environment string
 * @param command The expression for the command to issue
 *
 * @return A new instruction object.
 */
CommandIOConfiguration* LanguageParser::parseAddressWith()
{
    Protected<CommandIOConfiguration> ioConfig = new CommandIOConfiguration();

    // step to the next token and start processing
    RexxToken *token = nextReal();

    // we must have something after the WITH keyword, otherwise this is an error
    if (token->isEndOfClause())
    {
        syntaxError(Error_Symbol_expected_address_with);
    }

    // and check options until we reach the end of the clause
    while (!token->isEndOfClause())
    {
        // all options are symbol names,
        if (!token->isSymbol())
        {
            syntaxError(Error_Invalid_subkeyword_address_with_option, token);
        }

        // map the keyword name to a symbolic identifier.
        InstructionSubKeyword option = token->subKeyword();
        switch (option)
        {
            // Address env cmd with INPUT
            case SUBKEY_INPUT:
            {
                // not valid if we've had this before
                if (ioConfig->inputType != RedirectionType::DEFAULT)
                {
                    syntaxError(Error_Invalid_subkeyword_address_input);
                }
                token = nextReal();
                // is this specified as NORMAL? we don't parse any further.
                if (checkRedirectNormal(token))
                {
                    ioConfig->inputType = RedirectionType::NORMAL;
                }
                // need to parse further
                else
                {
                    previousToken();
                    parseRedirectOptions(ioConfig->inputSource, ioConfig->inputType);
                }
                break;
            }
            case SUBKEY_OUTPUT:
            {
                // not valid if we've had this before
                if (ioConfig->outputType != RedirectionType::DEFAULT)
                {
                    syntaxError(Error_Invalid_subkeyword_address_output);
                }
                token = nextReal();
                // is this specified as NORMAL? we don't parse any further.
                if (checkRedirectNormal(token))
                {
                    ioConfig->outputType = RedirectionType::NORMAL;
                }
                else
                {
                    // backup and parse off the output options and targets
                    previousToken();
                    ioConfig->outputOption = parseRedirectOutputOptions();
                    parseRedirectOptions(ioConfig->outputTarget, ioConfig->outputType);
                }
                break;
            }
            case SUBKEY_ERROR:
            {
                // not valid if we've had this before
                if (ioConfig->errorType != RedirectionType::DEFAULT)
                {
                    syntaxError(Error_Invalid_subkeyword_address_error);
                }
                token = nextReal();
                // is this specified as NORMAL? we don't parse any further.
                if (checkRedirectNormal(token))
                {
                    ioConfig->errorType = RedirectionType::NORMAL;
                }
                else
                {
                    // backup and parse off the output options and targets
                    previousToken();
                    ioConfig->errorOption = parseRedirectOutputOptions();
                    parseRedirectOptions(ioConfig->errorTarget, ioConfig->errorType);
                }
                break;
            }
            default:
            {
                syntaxError(Error_Invalid_subkeyword_address_with_option, token);
            }
        }
        // step to the next token position
        token = nextReal();
    }
    // and return the constructed configuration
    return ioConfig;
}


/**
 * Check if this source or target is specified as NORMAL,
 * which resets to default.
 *
 * @return true if the next keyword is NORMAL, false for anything else.
 */
bool LanguageParser::checkRedirectNormal(RexxToken *token)
{
    return token->isSymbol() && token->subKeyword() == SUBKEY_NORMAL;
}


/**
 * Check for potential APPEND/REPLACE options on an
 * ADDRESS WITH OUTPUT or ERROR keyword.
 *
 * @return The output option (including DEFAULT_REPLACE if
 *         nothing was specified)
 */
OutputOption::Enum LanguageParser::parseRedirectOutputOptions()
{
    // any option must be a symbol. The next token after the
    // option must also be a symbol, but we'll handle that elsewhere.
    RexxToken *token = nextReal();
    if (!token->isSymbol())
    {
        // back up and return the default
        previousToken();
        return OutputOption::DEFAULT;
    }

    // now see if this is one of our keyword values
    switch (token->subKeyword())
    {
        case SUBKEY_REPLACE:
            return OutputOption::REPLACE;

        case SUBKEY_APPEND:
            return OutputOption::APPEND;

            // probably one of the target type keywords
        default:
            // back up and return the default
            previousToken();
            return OutputOption::DEFAULT;
    }
    return OutputOption::DEFAULT;
}


/**
 * Parse the different redirection options on ADDRESS WITH
 *
 * @param ioType The type of I/O stream we're redirecting
 * @param source A reference to the field for the evaluated target
 *
 * @param type   The type of redirection to apply
 */
void LanguageParser::parseRedirectOptions(RexxInternalObject *&source, RedirectionType::Enum &type)
{
    RexxToken *token = nextReal();

    // all options are symbol names,
    if (!token->isSymbol())
    {
        syntaxError(Error_Invalid_subkeyword_address_with_io_option, token);
    }

    // now handle the different I/O types.
    switch (token->subKeyword())
    {
        // using a stem variable from the context
        case SUBKEY_STEM:
        {
            type = RedirectionType::STEM_VARIABLE;
            // this must be followed by a stem variable
            token = nextReal();
            if (!token->isStem())
            {
                syntaxError(Error_Symbol_expected_after_stem_keyword);
            }
            // resolve this to a variable expression type.
            source = addText(token);
            break;
        }

            // a string stream name. Can be a literal or an expression
        case SUBKEY_STREAM:
        {
            type = RedirectionType::STREAM_NAME;

            // we use the constant expression syntax here like the
            // RAISE instruction
            source = parseConstantExpression();
            if (source == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_missing_general, GlobalNames::STREAM, GlobalNames::ADDRESS);
            }
            break;
        }

            // determined by evaluating an expression to an object and figuring
            // out what to do at run time.
        case SUBKEY_USING:
        {
            type = RedirectionType::USING_OBJECT;

            // we use the constant expression syntax here like the
            // RAISE instruction
            source = parseConstantExpression();
            if (source == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_missing_general, GlobalNames::USING, GlobalNames::ADDRESS);
            }
            break;
        }

        default:
        {
            syntaxError(Error_Invalid_subkeyword_address_with_io_option, token);
        }
    }
}


/**
 * Create a new variable assignment instruction.
 *
 * @param target This is the token holding the name of the variable name
 *               for the assignment.
 *
 * @return An assignment instruction object.
 */
RexxInstruction* LanguageParser::assignmentNew(RexxToken  *target)
{
    // so far, we only know that the target is a symbol.  Verify that this
    // really is a variable symbol.  This handles raising an error if not valid.
    needVariable(target);
    // everthing after the "=" is the expression that is assigned to the variable.
    // The expression is required.
    RexxInternalObject *expr = requiredExpression(TERM_EOC, Error_Invalid_expression_assign);

    // build an instruction object and return it.
    RexxInstruction *newObject = new_instruction(ASSIGNMENT, Assignment);
    ::new((void *)newObject)RexxInstructionAssignment(addVariable(target), expr);
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
RexxInstruction* LanguageParser::assignmentOpNew(RexxToken *target, RexxToken *operation)
{
    // make sure this is a variable
    needVariable(target);
    // we require an expression for the additional part, which is required
    RexxInternalObject *expr = requiredExpression(TERM_EOC, Error_Invalid_expression_assign);

    // get a retriever for this, of the appropriate type
    RexxVariableBase *variable = addVariable(target);

    // now add a binary operator to this expression tree
    expr = new RexxBinaryOperator(operation->subtype(), variable, expr);

    // now everything is the same as an assignment operator
    RexxInstruction *newObject = new_instruction(ASSIGNMENT, Assignment);
    ::new((void *)newObject)RexxInstructionAssignment(variable, expr);
    return newObject;
}


/**
 * Parse a CALL ON or CALL OFF instruction.
 *
 * @param type   The target instruction type (CALL_ON or CALL_OFF).
 *
 * @return An executable instruction object for te target instruction.
 */
RexxInstruction* LanguageParser::callOnNew(InstructionSubKeyword type)
{
    // The processing of the CONDITION name is the same for both CALL ON
    // and CALL OFF

    RexxString *labelName;
    RexxString *conditionName;
    BuiltinCode builtinIndex = NO_BUILTIN;


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
        condType == CONDITION_PROPAGATE ||
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
        conditionName = commonString(labelName->concatToCstring("USER "));
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
            labelName = token->value();
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
    ::new((void *)newObject)RexxInstructionCallOn(conditionName, labelName, builtinIndex);

    // if this is the ON form, we have some end parsing resolution to perform.
    if (type == SUBKEY_ON)
    {
        addReference(newObject);
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
RexxInstruction* LanguageParser::dynamicCallNew(RexxToken *token)
{
    // this is a full expression in parens
    RexxInternalObject *targetName = parenExpression(token);
    // an expression is required
    if (name == OREF_NULL)
    {
        syntaxError(Error_Invalid_expression_call);
    }
    // process the argument list
    size_t argCount = parseArgList(OREF_NULL, TERM_EOC);

    // create a new instruction object
    RexxInstruction *newObject = new_variable_instruction(CALL_VALUE, DynamicCall, argCount, RexxObject * );
    ::new((void *)newObject)RexxInstructionDynamicCall(targetName, argCount, subTerms);

    // NOTE:  The name of the call cannot be determined until run time, so we don't
    // add this to the reference list for later processing
    return newObject;
}


/**
 * Process a call of the form CALL namespace:name, which is
 * restricted to public routines in the named namespace.
 *
 * @param token  The first token of the expression (with the opening paren of the name expression.)
 *
 * @return A new object for executing this call.
 */
RexxInstruction* LanguageParser::qualifiedCallNew(RexxToken *token)
{
    RexxString *namespaceName = token->value();

    // the colon has already been pulled off...so the next token
    // needs to be a symbol

    token = nextToken();
    if (!token->isSymbol())
    {
        syntaxError(Error_Symbol_expected_qualified_call, token);
    }

    RexxString *routineName = token->value();

    // process the argument list
    size_t argCount = parseArgList(OREF_NULL, TERM_EOC);

    // create a new instruction object
    RexxInstruction *newObject = new_variable_instruction(CALL_QUALIFIED, QualifiedCall, argCount, RexxObject * );
    ::new((void *)newObject)RexxInstructionQualifiedCall(namespaceName, routineName, argCount, subTerms);

    // NOTE:  The call target has no reliance on the labels because it is always an external
    // call.  This does not need a resolve step.
    return newObject;
}


/**
 * Finish parsing of a CALL instruction.  There are 3 distinct
 * forms of the Call instruction, with a different execution
 * object for each form.
 *
 * @return An executable call instruction object.
 */
RexxInstruction* LanguageParser::callNew()
{
    BuiltinCode builtin_index;
    RexxString *targetName = OREF_NULL;
    RexxString *namespaceName = OREF_NULL;
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
        // we could have a namespace-qualified name here.
        RexxToken *next = nextToken();
        if (next->isType(TOKEN_COLON))
        {
            return qualifiedCallNew(token);
        }

        // back up one token and try the others
        previousToken();


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
            argCount = parseArgList(OREF_NULL, TERM_EOC);
        }
    }
    // call with a string target
    else if (token->isLiteral())
    {
        targetName = token->value();
        // set the builtin index for later resolution steps
        builtin_index = token->builtin();
        // parse off an argument list
        argCount = parseArgList(OREF_NULL, TERM_EOC);
        // because this uses a string name, the internal label
        // search is bypassed.
        noInternal = false;
    }
    // is this call (expr) form?
    else if (token->isLeftParen())
    {
        // this has its own custom instruction object.
        return dynamicCallNew(token);
    }
    // Something unknown...
    else
    {
        syntaxError(Error_Symbol_or_string_call);
    }

    // create a new Call instruction.  This only handles the simple calls.
    RexxInstruction *newObject = new_variable_instruction(CALL, Call, argCount, RexxObject * );
    ::new((void *)newObject)RexxInstructionCall(targetName, argCount, subTerms, builtin_index);

    // add to our references list, but only if this is a form where
    // internal calls are allowed.
    if (!noInternal)
    {
        addReference(newObject);
    }
    return newObject;
}


/**
 * Parse and create a COMMAND instruction.
 *
 * @return A COMMAND instruction instance.
 */
RexxInstruction* LanguageParser::commandNew()
{
    // this is pretty simple...parse the optional expresson and create the
    // instruction instance.  NOTE:  we already know we have something to
    // parse, so there's no need to check for a missing expression.
    RexxInternalObject *expression = parseExpression(TERM_EOC);

    RexxInstruction *newObject = new_instruction(COMMAND, Command);
    ::new((void *)newObject)RexxInstructionCommand(expression);
    return newObject;
}


/**
 * Parse and create the variants of a controlled DO/LOOP
 * instruction (Loop i = 1 to 5 by 2 for 10).  This
 * can also include a WHILE or UNTIL modifier (but not both).
 *
 * @param label     Any label obtained from the LABEL keyword.
 * @param c      A variable name for setting a counter (optional)
 * @param nameToken The control variable name token.
 *
 * @return A contructed instruction object of the appropriate type.
 */
RexxInstruction* LanguageParser::newControlledLoop(RexxString *label, RexxVariableBase *countVariable, RexxToken *nameToken)
{
    // a contruct to fill in for the instruction.
    ControlledLoop control;
    WhileUntilLoop conditional;
    // track while/until forms
    InstructionSubKeyword conditionalType = SUBKEY_NONE;


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
    control.initial = requiredExpression(TERM_CONTROL, Error_Invalid_expression_control);

    // protect on the term stack
    pushSubTerm(control.initial);

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
                control.by = requiredExpression(TERM_CONTROL, Error_Invalid_expression_by);

                // protect on the term stack
                pushSubTerm(control.by);

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
                control.to = requiredExpression(TERM_CONTROL, Error_Invalid_expression_to);
                // protect on the term stack
                pushSubTerm(control.to);

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
                control.forCount = requiredExpression(TERM_CONTROL, Error_Invalid_expression_for);

                // protect on the term stack
                pushSubTerm(control.forCount);

                // record the processing order
                control.expressions[keyslot++] = EXP_FOR;
                break;
            }

            case SUBKEY_UNTIL:
            case SUBKEY_WHILE:
            {
                // step back a token and process the conditional
                previousToken();
                // this also does not allow anything after the loop conditional
                conditional.conditional = parseLoopConditional(conditionalType, Error_None);
                break;
            }

                // invalid loop subkey (should really never happen)
            default:
            {
                reportException(Error_Interpretation_switch, "loop subkey", token->subKeyword());
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
            ::new((void *)newObject)RexxInstructionControlledDo(label, countVariable, control);
            return newObject;
        }
            // Controlled loop with a WHILE conditional
        case SUBKEY_WHILE:
        {
            RexxInstruction *newObject = new_instruction(LOOP_CONTROLLED_WHILE, ControlledDoWhile);
            ::new((void *)newObject)RexxInstructionControlledDoWhile(label, countVariable, control, conditional);
            return newObject;
        }
            // Controlled loop with an UNTIL conditional.
        case SUBKEY_UNTIL:
        {
            RexxInstruction *newObject = new_instruction(LOOP_CONTROLLED_UNTIL, ControlledDoUntil);
            ::new((void *)newObject)RexxInstructionControlledDoUntil(label, countVariable, control, conditional);
            return newObject;
        }
            // invalid controlled loop subkey (should really never happen)
        default:
        {
            reportException(Error_Interpretation_switch, "controlled loop subkey", conditionalType);
            break;
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
RexxInstruction* LanguageParser::newDoOverLoop(RexxString *label, RexxVariableBase *countVariable, RexxToken *nameToken)
{
    // a construct to fill in for the instruction.
    OverLoop control;
    ForLoop forLoop;
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
    control.control = addVariable(nameToken);
    // and get the OVER expression, which is required
    control.target = requiredExpression(TERM_OVER, Error_Invalid_expression_over);
    // protect this expression from GC
    pushSubTerm(control.target);

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
            case SUBKEY_FOR:
            {
                // only one per customer
                if (forLoop.forCount != OREF_NULL)
                {
                    syntaxError(Error_Invalid_do_duplicate, token);
                }
                // get the keyword expression, which is required also
                forLoop.forCount = requiredExpression(TERM_CONTROL, Error_Invalid_expression_for);
                // protect from GC
                pushSubTerm(forLoop.forCount);
                break;
            }

            case SUBKEY_UNTIL:
            case SUBKEY_WHILE:
            {
                // step back a token and process the conditional
                previousToken();
                // this also does not allow anything after the loop conditional
                conditional.conditional = parseLoopConditional(conditionalType, Error_None);
                break;
            }

                // invalid DO OVER loop subkey (should really never happen)
            default:
            {
                reportException(Error_Interpretation_switch, "DO OVER loop subkey", token->subKeyword());
                break;
            }
        }
        token = nextReal();
    }

    // NOTE:  We parse until we hit the end of clause or found an error,
    // so once we get here, there's no need for any end-of-clause checks.

    // we've parsed everything correctly and we have six potential types of
    // loop now.  1)  A DO OVER loop with no conditional, 2) a DO OVER loop
    // with a WHILE condition and 3) a DO OVER loop with a UNTIL condition.
    // Each of those forms can also have a FOR modifier.
    // The conditionalType variable tells us which form it is, so we can create
    // the correct type instruction object, the forControl will tell us if we have a FOR
    // expression.

    switch (conditionalType)
    {
        // DO OVER with no extra conditional.
        case SUBKEY_NONE:
        {
            if (forLoop.forCount == OREF_NULL)
            {
                RexxInstruction *newObject = new_instruction(LOOP_OVER, DoOver);
                ::new((void *)newObject)RexxInstructionDoOver(label, countVariable, control);
                return newObject;
            }
            else
            {
                RexxInstruction *newObject = new_instruction(LOOP_OVER_FOR, DoOverFor);
                ::new((void *)newObject)RexxInstructionDoOverFor(label, countVariable, control, forLoop);
                return newObject;
            }
        }
            // DO OVER with a WHILE conditional
        case SUBKEY_WHILE:
        {
            if (forLoop.forCount == OREF_NULL)
            {
                RexxInstruction *newObject = new_instruction(LOOP_OVER_WHILE, DoOverWhile);
                ::new((void *)newObject)RexxInstructionDoOverWhile(label, countVariable, control, conditional);
                return newObject;
            }
            else
            {
                RexxInstruction *newObject = new_instruction(LOOP_OVER_FOR_WHILE, DoOverForWhile);
                ::new((void *)newObject)RexxInstructionDoOverForWhile(label, countVariable, control, forLoop, conditional);
                return newObject;
            }
        }
            // DO OVER with an UNTIL conditional.
        case SUBKEY_UNTIL:
        {
            if (forLoop.forCount == OREF_NULL)
            {
                RexxInstruction *newObject = new_instruction(LOOP_OVER_UNTIL, DoOverUntil);
                ::new((void *)newObject)RexxInstructionDoOverUntil(label, countVariable, control, conditional);
                return newObject;
            }
            else
            {
                RexxInstruction *newObject = new_instruction(LOOP_OVER_FOR_UNTIL, DoOverForUntil);
                ::new((void *)newObject)RexxInstructionDoOverForUntil(label, countVariable, control, forLoop, conditional);
                return newObject;
            }
        }
            // invalid DO OVER conditional (should really never happen)
        default:
        {
            reportException(Error_Interpretation_switch, "DO OVER conditional", conditionalType);
            break;
        }
    }
    return OREF_NULL;    // should never get here.
}


/**
 * Parse and create the variants of a DO WITH loop instruction.
 * This can also include a WHILE or UNTIL modifier (but not
 * both).
 *
 * @param label     Any label obtained from the LABEL keyword.
 *
 * @return A contructed instruction object of the appropriate type.
 */
RexxInstruction* LanguageParser::newDoWithLoop(RexxString *label, RexxVariableBase *countVariable)
{
    // a construct to fill in for the instruction.
    WithLoop withLoop;
    ForLoop forLoop;
    WhileUntilLoop conditional;
    // track while/until forms
    InstructionSubKeyword conditionalType = SUBKEY_NONE;

    // we have already parsed over the WITH keyword, so we're looing
    // for the INDEX, ITEM, and OVER keywords now.  OVER must be after
    // INDEX and ITEM, which can be in any order (and only one is needed).

    // get the next real token.
    RexxToken *token = nextReal();

    // these all options are marked by symbols, so keep looping while we have one
    while (token->isSymbol())
    {
        // this must be a keyword, so resolve it.
        switch (token->subKeyword())
        {
            // an index variable
            case SUBKEY_INDEX:
            {
                if (withLoop.indexVar != OREF_NULL)
                {
                    syntaxError(Error_Invalid_do_duplicate, token);
                }

                // this must be a variable token
                token = nextReal();

                // save the loopWith variable retriever
                withLoop.indexVar = requiredVariable(token, "INDEX");
                // get the next token and continue
                token = nextReal();
                continue;
            }

                // an item variable
            case SUBKEY_ITEM:
            {
                if (withLoop.itemVar != OREF_NULL)
                {
                    syntaxError(Error_Invalid_do_duplicate, token);
                }

                // this must be a variable token
                token = nextReal();

                // save the loopWith variable retriever
                withLoop.itemVar = requiredVariable(token, "ITEM");
                // get the next token and continue
                token = nextReal();
                continue;
            }

            default:
            {
                break;
            }
        }

        // found a keyword that is not INDEX or ITEM, break out of the loop
        break;
    }

    // we must have at least one of these loop loopWith variables
    if (withLoop.itemVar == OREF_NULL && withLoop.indexVar == OREF_NULL)
    {
        syntaxError(Error_Invalid_do_with_no_control);
    }

    // the next token needs to be an OVER keyword.
    if (!token->isSymbol() || token->subKeyword() != SUBKEY_OVER)
    {
        syntaxError(Error_Invalid_do_with_no_over);
    }

    // and get the OVER expression, which is required
    withLoop.supplierSource = requiredExpression(TERM_OVER, Error_Invalid_expression_over);

    // protect this expression from GC
    pushSubTerm(withLoop.supplierSource);

    // ok, keep looping while we don't have a clause terminator
    // because the parsing of the initial expression is terminated by either
    // the end-of-clause or DO/LOOP keyword, we know the next token will by
    // a symbol if it is not the end.
    token = nextReal();

    while (!token->isEndOfClause())
    {
        // this must be a keyword, so resolve it.
        switch (token->subKeyword())
        {
            case SUBKEY_FOR:
            {
                // only one per customer
                if (forLoop.forCount != OREF_NULL)
                {
                    syntaxError(Error_Invalid_do_duplicate, token);
                }
                // get the keyword expression, which is required also
                forLoop.forCount = requiredExpression(TERM_CONTROL, Error_Invalid_expression_for);
                // protect from GC
                pushSubTerm(forLoop.forCount);
                break;
            }

            case SUBKEY_UNTIL:
            case SUBKEY_WHILE:
            {
                // step back a token and process the conditional
                previousToken();
                // this also does not allow anything after the loop conditional
                conditional.conditional = parseLoopConditional(conditionalType, Error_None);
                break;
            }

                // invalid DO WITH conditional (should really never happen)
            default:
            {
                reportException(Error_Interpretation_switch, "DO WITH conditional", conditionalType);
                break;
            }
        }
        token = nextReal();
    }

    // NOTE:  We parse until we hit the end of clause or found an error,
    // so once we get here, there's no need for any end-of-clause checks.

    // we've parsed everything correctly and we have six potential types of
    // loop now.  1)  A DO WITH loop with no conditional, 2) a DO WITH loop
    // with a WHILE condition and 3) a DO WITH loop with a UNTIL condition.
    // Each of those forms can also have a FOR modifier.
    // The conditionalType variable tells us which form it is, so we can create
    // the correct type instruction object, the forControl will tell us if we have a FOR
    // expression.

    switch (conditionalType)
    {
        // DO WITH with no extra conditional.
        case SUBKEY_NONE:
        {
            if (forLoop.forCount == OREF_NULL)
            {
                RexxInstruction *newObject = new_instruction(LOOP_WITH, DoWith);
                ::new((void *)newObject)RexxInstructionDoWith(label, countVariable, withLoop);
                return newObject;
            }
            else
            {
                RexxInstruction *newObject = new_instruction(LOOP_WITH_FOR, DoWithFor);
                ::new((void *)newObject)RexxInstructionDoWithFor(label, countVariable, withLoop, forLoop);
                return newObject;
            }
        }
            // DO WITH with a WHILE conditional
        case SUBKEY_WHILE:
        {
            if (forLoop.forCount == OREF_NULL)
            {
                RexxInstruction *newObject = new_instruction(LOOP_WITH_WHILE, DoWithWhile);
                ::new((void *)newObject)RexxInstructionDoWithWhile(label, countVariable, withLoop, conditional);
                return newObject;
            }
            else
            {
                RexxInstruction *newObject = new_instruction(LOOP_WITH_FOR_WHILE, DoWithForWhile);
                ::new((void *)newObject)RexxInstructionDoWithForWhile(label, countVariable, withLoop, forLoop, conditional);
                return newObject;
            }
        }
            // DO WITH with an UNTIL conditional.
        case SUBKEY_UNTIL:
        {
            if (forLoop.forCount == OREF_NULL)
            {
                RexxInstruction *newObject = new_instruction(LOOP_WITH_UNTIL, DoWithUntil);
                ::new((void *)newObject)RexxInstructionDoWithUntil(label, countVariable, withLoop, conditional);
                return newObject;
            }
            else
            {
                RexxInstruction *newObject = new_instruction(LOOP_WITH_FOR_UNTIL, DoWithForUntil);
                ::new((void *)newObject)RexxInstructionDoWithForUntil(label, countVariable, withLoop, forLoop, conditional);
                return newObject;
            }
        }
            // invalid DO WITH conditional (should really never happen)
        default:
        {
            reportException(Error_Interpretation_switch, "DO WITH conditional", conditionalType);
            break;
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
RexxInstruction* LanguageParser::newSimpleDo(RexxString *label)
{
    RexxInstruction *newObject = new_instruction(SIMPLE_BLOCK, SimpleDo);
    ::new((void *)newObject)RexxInstructionSimpleDo(label);
    return newObject;
}


/**
 * Create an instance of a DO FOREVER loop.
 *
 * @param label  The optional block label.
 * @param countVariable
 *               The optional counter variable.
 *
 * @return An instruction object for a do block.
 */
RexxInstruction* LanguageParser::newLoopForever(RexxString *label, RexxVariableBase *countVariable)
{
    RexxInstruction *newObject = new_instruction(LOOP_FOREVER, DoForever);
    ::new((void *)newObject)RexxInstructionDoForever(label, countVariable);
    return newObject;
}


/**
 * Create an instance of a DO WHILE loop.
 *
 * @param label  The optional block label.
 * @param countVariable
 *               The optional counter variable.
 *
 * @return An instruction object for a do block.
 */
RexxInstruction* LanguageParser::newLoopWhile(RexxString *label, RexxVariableBase *countVariable, WhileUntilLoop &conditional)
{
    RexxInstruction *newObject = new_instruction(LOOP_WHILE, DoWhile);
    ::new((void *)newObject)RexxInstructionDoWhile(label, countVariable, conditional);
    return newObject;
}


/**
 * Create an instance of a DO WHILE loop.
 *
 * @param label  The optional block label.
 * @param countVariable
 *               The optional counter variable.
 *
 * @return An instruction object for a do block.
 */
RexxInstruction* LanguageParser::newLoopUntil(RexxString *label, RexxVariableBase *countVariable, WhileUntilLoop &conditional)
{
    RexxInstruction *newObject = new_instruction(LOOP_UNTIL, DoUntil);
    ::new((void *)newObject)RexxInstructionDoUntil(label, countVariable, conditional);
    return newObject;
}


/**
 * Parse a DO FOREVER loop.  This will be either just
 * DO FOREVER (most common), but can also have WHILE
 * or UNTIL modifiers, which makes them identical to DO
 * WHILE and DO UNTIL.
 *
 * @param label  The instruction label.
 * @param countVariable
 *               The optional counter variable.
 *
 * @return An instruction of the appropriate type.
 */
RexxInstruction* LanguageParser::parseForeverLoop(RexxString *label, RexxVariableBase *countVariable)
{
    // a contruct to fill in for the instruction.
    WhileUntilLoop conditional;

    // track while/until forms
    InstructionSubKeyword conditionalType = SUBKEY_NONE;

    // this also does not allow anything after the loop conditional
    conditional.conditional = parseLoopConditional(conditionalType, Error_Invalid_do_forever);

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
            return newLoopForever(label, countVariable);
        }
            // DO FOREVER with a WHILE conditional...identical to a DO WHILE
        case SUBKEY_WHILE:
        {
            return newLoopWhile(label, countVariable, conditional);
        }
            // DO FOREVER with an UNTIL conditional...identical to a DO UNTIL
        case SUBKEY_UNTIL:
        {
            return newLoopUntil(label, countVariable, conditional);
        }
            // invalid DO FOREVER conditional (should really never happen)
        default:
        {
            reportException(Error_Interpretation_switch, "DO FOREVER conditional", conditionalType);
            break;
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
RexxInstruction* LanguageParser::parseCountLoop(RexxString *label, RexxVariableBase *countVariable)
{
    // the For count controller
    ForLoop forCount;

    // a contruct to fill in for the instruction.
    WhileUntilLoop conditional;

    // track while/until forms
    InstructionSubKeyword conditionalType = SUBKEY_NONE;

    // we know there is something there, so we will either get an
    // expression, or get an error for an invalid expression

    forCount.forCount = parseExpression(TERM_COND);

    // protect on the term stack
    pushSubTerm(forCount.forCount);

    // process an additional conditional (NOTE:  Because of the
    // terminators used for the target expression, the only possibilities
    // here are end of clause, a WHILE keyword, or an UNTIL keyword)
    conditional.conditional = parseLoopConditional(conditionalType, Error_None);

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
            ::new((void *)newObject)RexxInstructionDoCount(label, countVariable, forCount);
            return newObject;
        }
            // DO count with a WHILE conditional
        case SUBKEY_WHILE:
        {
            RexxInstruction *newObject = new_instruction(LOOP_COUNT_WHILE, DoCountWhile);
            ::new((void *)newObject)RexxInstructionDoCountWhile(label, countVariable, forCount, conditional);
            return newObject;
        }
            // DO count with an UNTIL conditional.
        case SUBKEY_UNTIL:
        {
            RexxInstruction *newObject = new_instruction(LOOP_COUNT_UNTIL, DoCountUntil);
            ::new((void *)newObject)RexxInstructionDoCountUntil(label, countVariable, forCount, conditional);
            return newObject;
        }
            // invalid DO count conditional (should really never happen)
        default:
        {
            reportException(Error_Interpretation_switch, "DO count conditional", conditionalType);
            break;
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
RexxInstruction* LanguageParser::createLoop(bool isLoop)
{
    // we may need to parse ahead a bit to determine the type of loop, so
    // get a reset position before getting next token
    size_t currentPosition = markPosition();
    // We've already figured out this is either a LOOP or a DO, now we need to
    // decode what else is going on.
    RexxToken *token = nextReal();

    // label parsing is handled here.
    RexxString *label = OREF_NULL;
    // the counter parsing is also
    RexxVariableBase *countVariable = OREF_NULL;

    // before doing anything, we need to test for a "LABEL name" and/or "COUNTER name" before other options.  We
    // need to make certain we check that the keyword is not being
    // used as control variable.
    while (token->isSymbol())
    {
        // potentially a label or a counter.  Check the keyword value
        if (token->subKeyword() == SUBKEY_LABEL || token->subKeyword() == SUBKEY_COUNTER)
        {
            // if we already had COUNTER, this is part of the loop
            if (countVariable != OREF_NULL && token->subKeyword() == SUBKEY_COUNTER)
            {
                break;
            }
            // if we already had LABEL, this is part of the loop
            if (label != OREF_NULL && token->subKeyword() == SUBKEY_LABEL)
            {
                break;
            }
            // if the next token is a symbol, this is a label or counter
            RexxToken *name = nextReal();
            if (name->isSymbol())
            {
                if (token->subKeyword() == SUBKEY_LABEL)
                {
                    // save the label name.
                    label = name->value();
                }
                else
                {
                    // get a retriever to be able to access this token.
                    countVariable = addVariable(name);
                }

                // update the reset position before stepping to the next token
                currentPosition = markPosition();
                // step to the next token for processing the following parts
                token = nextReal();
                // if we've had both global options specified, everything else after this
                // is part of the loop type.
                if (countVariable != OREF_NULL && label != OREF_NULL)
                {
                    break;
                }
            }
            // is this symbol "="?  Handle as a controlled loop
            else if (name->isSubtype(OPERATOR_EQUAL))
            {
                // This is a controlled loop, we're all positioned to process this.
                return newControlledLoop(label, countVariable, token);
            }
            // similar to assignment instructions, any equal sign triggers this to
            // be a controlled form, even if the the "=" is part of a larger instruction
            // such as "==".  We give this as an expression error.
            else if (name->isSubtype(OPERATOR_STRICT_EQUAL))
            {
                syntaxError(Error_Invalid_expression_general, name);
            }
            else
            {
                // this is either an end-of-clause or something not a symbol.  This is an error.
                syntaxError(token->subKeyword() == SUBKEY_LABEL ? Error_Symbol_expected_LABEL : Error_Symbol_expected_counter);
            }
        }
        else
        {
            break;
        }
    }


    // Just the single token means this is either a simple DO block or
    // a LOOP forever, depending on the type of instruction we're parsing.
    if (token->isEndOfClause())
    {
        // A LOOP instruction with nothing is a LOOP FOREVER.
        if (isLoop)
        {
            return newLoopForever(label, countVariable);
        }
        else
        {
            // IF this is just a simple DO, the count variable is not allowed.
            if (countVariable != OREF_NULL)
            {
                syntaxError(Error_Invalid_do_simple_do_counter);
            }
            return newSimpleDo(label);
        }
    }

    // we've already handled the control variable version where the variable happens to
    // be named LABEL or COUNTER above.  We still need to check for this with any other name.
    if (token->isSymbol())
    {
        // this can be a control variable or keyword.  We need to look ahead
        // to the next token.
        RexxToken *second = nextReal();
        // similar to assignment instructions, any equal sign triggers this to
        // be a controlled form, even if the the "=" is part of a larger instruction
        // such as "==".  We give this as an expression error.
        if (second->isSubtype(OPERATOR_STRICT_EQUAL))
        {
            syntaxError(Error_Invalid_expression_general, second);
        }
        // ok, now check for the control variable.
        else if (second->isSubtype(OPERATOR_EQUAL))
        {
            // go parse a controlled do loop.
            return newControlledLoop(label, countVariable, token);
        }
        // DO name OVER collection form?
        else if (second->subKeyword() == SUBKEY_OVER)
        {
            // parse and return the DO OVER options
            return newDoOverLoop(label, countVariable, token);
        }
        // not a controlled form, but this could be a conditional form.
        else
        {
            // start the parsing process over from the beginning with the first of the
            // loop-type tokens
            resetPosition(currentPosition);

            token = nextReal();
            // now check the other keyword varieties.
            switch (token->subKeyword())
            {
                // WITH INDEX var ITEM var OVER expr....with a potention WHILE or UNTIL modifier
                case SUBKEY_WITH:
                {
                    return newDoWithLoop(label, countVariable);
                }
                // FOREVER...this can have either a WHILE or UNTIL modifier.
                case SUBKEY_FOREVER:         // DO FOREVER
                {
                    return parseForeverLoop(label, countVariable);
                }

                // WHILE cond
                case SUBKEY_WHILE:
                {
                    // a contruct to fill in for the instruction.
                    WhileUntilLoop conditional;

                    InstructionSubKeyword conditionalType = SUBKEY_NONE;

                    // step back one token and process the conditional
                    previousToken();
                    conditional.conditional = parseLoopConditional(conditionalType, Error_None);
                    // We know this is WHILE already, so we can create this directly
                    return newLoopWhile(label, countVariable, conditional);
                }

                // DO UNTIL cond
                case SUBKEY_UNTIL:
                {
                    // a contruct to fill in for the instruction.
                    WhileUntilLoop conditional;

                    InstructionSubKeyword conditionalType = SUBKEY_NONE;

                    // step back one token and process the conditional
                    previousToken();
                    conditional.conditional = parseLoopConditional(conditionalType, Error_None);
                    // We know this is UNTIL already, so we can create this directly
                    return newLoopUntil(label, countVariable, conditional);
                }

                // not a real DO keyword, probably DO expr form, which can also have
                // WHILE or UNTIL modifiers.
                default:
                    // push the first token back
                    previousToken();
                    // and go parse this out.
                    return parseCountLoop(label, countVariable);
            }
        }
    }
    // just a DO expr form that doesn't start with a symbol
    else
    {
        // push the first token back
        previousToken();
        // and go parse this out.
        return parseCountLoop(label, countVariable);
    }
    // should never get here
    return OREF_NULL;
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

    RexxInstruction *newObject = new_variable_instruction(DROP, Drop, variableCount, RexxObject *);
    // this initializes from the sub term stack.
    ::new ((void *)newObject) RexxInstructionDrop(variableCount, subTerms);
    return newObject;
}


/**
 * We've encountered an ELSE in the source.  Just create
 * an ELSE instruction object.  Wiring this up with
 * the corresponding IF/THEN takes place when this
 * is added to the instruction chain, as well as handling
 * errors for misplaced instructions.
 *
 * @param token  The token for the ELSE keyword.  Used for setting
 *               the instruction location.
 *
 * @return A new ELSE instruction object.
 */
RexxInstruction *LanguageParser::elseNew(RexxToken  *token)
{
    RexxInstruction *newObject = new_instruction(ELSE, Else);
    ::new ((void *)newObject) RexxInstructionElse(token);
    return newObject;
}


/**
 * Create a new END instruction.  This only parses the
 * instruction syntax and creates the new instruction
 * object.  Matching up with its matching block instruction
 * occurs when it is added to the instruction chain.
 *
 * @return An END instruction object.
 */
RexxInstruction *LanguageParser::endNew()
{
    // The block name is optional
    RexxString *name = OREF_NULL;

    // see if we have a label or control variable name specified
    RexxToken *token = nextReal();
    if (!token->isEndOfClause())
    {
        if (!token->isSymbol())
        {
            syntaxError(Error_Symbol_expected_end);
        }
        // get the name and verify that's all there is.
        name = token->value();
        requiredEndOfClause(Error_Invalid_data_end);
    }

    RexxInstruction *newObject = new_instruction(END, End);
    ::new ((void *)newObject) RexxInstructionEnd(name);
    return newObject;
}


/**
 * Create an END IF instruction.  The EndIf is a dummy
 * instruction that is attached to the end of the
 * instruction following the THEN of an IF or WHEN.
 * This branches to the appropriate location after
 * that instruction completes.  For example, branching
 * around an ELSE or to the end of a SELECT block.
 *
 * This is created when the pending control stack is processed
 * when the previous instruction completes.
 *
 * @param parent The parent instruction.
 *
 * @return The new instruction object.
 */
RexxInstruction *LanguageParser::endIfNew(RexxInstructionIf *parent)
{
    RexxInstruction *newObject = new_instruction(ENDIF, EndIf);
    ::new ((void *)newObject) RexxInstructionEndIf(parent);
    return newObject;
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
    RexxInternalObject *expression = parseExpression(TERM_EOC);
    RexxInstruction *newObject = new_instruction(EXIT, Exit);
    ::new ((void *)newObject) RexxInstructionExit(expression);
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

    // The EXPOSE must be the first instruction.
    // NOTE:  labels are not allowed preceding, as that will give a target
    // for SIGNAL or CALL that will result in an invalid EXPOSE execution.

    // the last instruction in the chain must be our dummy
    // first instruction
    if (!lastInstruction->isType(KEYWORD_FIRST))
    {
        syntaxError(Error_Translation_expose);
    }

    // process the variable list and create an instruction from this.
    // The variables are placed in the subTerms stack
    size_t variableCount = processVariableList(KEYWORD_EXPOSE);

    RexxInstruction *newObject = new_variable_instruction(EXPOSE, Expose, variableCount, RexxObject *);
    ::new ((void *)newObject) RexxInstructionExpose(variableCount, subTerms);
    return newObject;
}


/**
 * Parse and create a new USE LOCAL instruction
 *
 * @return The configured instruction instance.
 */
RexxInstruction *LanguageParser::useLocalNew()
{
    // not valid in an interpret
    if (isInterpret())
    {
        syntaxError(Error_Translation_use_local_interpret);
    }

    // validate the placement at the beginning of the code block...
    // the rules are the same as with EXPOSE.
    if (!lastInstruction->isType(KEYWORD_FIRST))
    {
        syntaxError(Error_Translation_use_local);
    }

    // switch on auto expose tracking
    autoExpose();

    // process the variable list and create an instruction from this.
    // There are enough differences from EXPOSE/DROP/PROCEDURE that it is
    // easier doing this here.
    size_t variableCount = 0;

    // the next real token is the start of the list (after the
    // space following the keyword instruction.
    RexxToken *token = nextReal();

    // while not at the end of the clause, process a list of variables.  Note
    // that the variable list is optional.
    while (!token->isEndOfClause())
    {
        // generally, these are symbols, but not all symbols are variables.
        if (token->isSymbol())
        {
            // non-variable symbol?
            if (token->isSubtype(SYMBOL_CONSTANT))
            {
                syntaxError(Error_Invalid_variable_number, token);
            }
            // the dummy period and other dot symbols
            else if (token->isSubtype(SYMBOL_DUMMY, SYMBOL_DOTSYMBOL))
            {
                syntaxError(Error_Invalid_variable_period, token);
            }
            // we only allow simple variables or stems to be declared local
            else if (token->isSubtype(SYMBOL_COMPOUND))
            {
                syntaxError(Error_Translation_use_local_compound, token);
            }

            // ok, get a retriever for the variable and push it on the stack.
            pushSubTerm(addVariable(token));
            // the GUARD instruction also needs to understand about locals,
            // so record this as an explicit local
            localVariable(token->value());
            // update our return value.
            variableCount++;
        }
        // something unrecognized...we need to issue the message for the instruction.
        else
        {
            syntaxError(Error_Symbol_expected_use_local);
        }
        // and see if we have more variables
        token = nextReal();
    }

    RexxInstruction *newObject = new_variable_instruction(USE_LOCAL, UseLocal, variableCount, RexxObject *);
    ::new ((void *)newObject) RexxInstructionUseLocal(variableCount, subTerms);
    return newObject;
}


/**
 * Parse and create a new FORWARD instruction object.
 *
 * @return The new instruction object.
 */
RexxInstruction *LanguageParser::forwardNew()
{
    // not permitted in interpret
    if (isInterpret())
    {
        syntaxError(Error_Translation_forward_interpret);
    }

    // variables for all of the instruction pieces.
    bool returnContinue = false;
    RexxInternalObject *target = OREF_NULL;
    RexxInternalObject *message = OREF_NULL;
    RexxInternalObject *superClass = OREF_NULL;
    RexxInternalObject *arguments = OREF_NULL;
    ArrayClass  *array = OREF_NULL;

    // and start parsing
    RexxToken *token = nextReal();

    // keep processing keywords until we reach the end of the clause
    while (!token->isEndOfClause())
    {
        // all keywords are symbols, so not finding a symbol is an error
        if (!token->isSymbol())
        {
            syntaxError(Error_Invalid_subkeyword_forward_option, token);
        }

        // process each of the options
        switch (token->subKeyword())
        {
            // FORWARD TO expr
            case SUBKEY_TO:
            {
                // can only be specified once
                if (target != OREF_NULL)
                {
                    syntaxError(Error_Invalid_subkeyword_to);
                }
                // get the expression, which is required
                target = parseConstantExpression();
                if (target == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_forward_to);
                }
                pushSubTerm(target);
                break;
            }

            // FORWARD CLASS expr
            case SUBKEY_CLASS:
            {
                // have a class over ride already?
                if (superClass != OREF_NULL)
                {
                    syntaxError(Error_Invalid_subkeyword_forward_class);
                }
                // get the value, which is a required expression
                superClass = parseConstantExpression();
                if (superClass == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_forward_class);
                }
                pushSubTerm(superClass);
                break;
            }

            // FORWARD MESSAGE expr
            case SUBKEY_MESSAGE:
            {
                // have a message over ride already?
                if (message != OREF_NULL)
                {
                    syntaxError(Error_Invalid_subkeyword_message);
                }
                // this is a required expression     */
                message = parseConstantExpression();
                if (message == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_forward_message);
                }
                pushSubTerm(message);
                break;
            }

            // FORWARD ARGUMENTS expr
            case SUBKEY_ARGUMENTS:
            {
                // only one of ARGUMENS or ARRAY allowed
                if (arguments != OREF_NULL || array != OREF_NULL)
                {
                    syntaxError(Error_Invalid_subkeyword_arguments);
                }
                // single, required expression
                arguments = parseConstantExpression();
                if (arguments == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_forward_arguments);
                }
                break;
            }

            // FORWARD ARRAY (expr, expr, ...)
            case SUBKEY_ARRAY:
            {
                // only one of ARRAY ARGUMENTS allowed
                if (arguments != OREF_NULL || array != OREF_NULL)
                {
                    syntaxError(Error_Invalid_subkeyword_arguments);
                }
                // we need a list of expression here...process as an argument array
                token = nextReal();
                if (!token->isLeftParen())
                {
                    syntaxError(Error_Invalid_expression_raise_list);
                }
                array = parseArgArray(token, TERM_RIGHT);
                break;
            }

            // FORWARD CONTINUE
            case SUBKEY_CONTINUE:
            {
                // can only be specified once
                if (returnContinue)
                {
                    syntaxError(Error_Invalid_subkeyword_continue);
                }
                returnContinue = true;
                break;
            }

            // something invalie
            default:
                syntaxError(Error_Invalid_subkeyword_forward_option, token);
                break;
        }
        // go process another keyword
        token = nextReal();
    }
    // and create the forward object
    RexxInstruction *newObject = new_instruction(FORWARD, Forward);
    ::new ((void *)newObject) RexxInstructionForward(target, message, superClass, arguments, array, returnContinue);
    return newObject;
}

/**
 * Parse and create a guard expression.
 *
 * @return A constructed executable GUARD instruction.
 */
RexxInstruction *LanguageParser::guardNew()
{
    // not permitted in interpret
    if (isInterpret())
    {
        syntaxError(Error_Translation_guard_interpret);
    }

    RexxInternalObject *expression = OREF_NULL;
    Protected<ArrayClass> variable_list;
    size_t variable_count = 0;


    RexxToken *token = nextReal();

    // first token must be either GUARD ON or GUARD OFF.
    if (!token->isSymbol())
    {
        syntaxError(Error_Symbol_expected_numeric, token);
    }

    // this tells us which way to set
    bool guardOn = false;

    // and figure out which case we have
    switch (token->subKeyword())
    {
        // GUARD OFF
        case SUBKEY_OFF:
            guardOn = false;
            break;

        // GUARD OFF
        case SUBKEY_ON:
            guardOn = true;
            break;

        // anything else is an error
        default:
            syntaxError(Error_Invalid_subkeyword_guard, token);
            break;
    }

    // we can have a WHEN expression after this.
    token = nextReal();

    // we could have a WHEN expression
    if (token->isSymbol())
    {
        switch (token->subKeyword())
        {
            // GUARD XXX WHEN expr
            case SUBKEY_WHEN:
            {
                // turn on variable tracking during expression evaluation
                setGuard();
                // evaluate the WHEN expression, which is required
                expression = requiredExpression(TERM_EOC, Error_Invalid_expression_guard);

                // get the guard expression variable list
                variable_list = getGuard();
                variable_count = variable_list->items();

                // if using GUARD WHEN, we will never wake up if there are
                // not at least one object variable accessed.
                if (variable_count == 0)
                {
                    syntaxError(Error_Translation_guard_expose);
                }
                break;
            }

            // any other keyword is an error
            default:
                syntaxError(Error_Invalid_subkeyword_guard_on, token);
                break;
        }
    }
    // anything other than the end is an error
    else if (!token->isEndOfClause())
    {
        syntaxError(Error_Invalid_subkeyword_guard_on, token);
    }

    RexxInstruction *newObject = new_variable_instruction(GUARD, Guard, variable_count, RexxVariableBase *);
    ::new ((void *)newObject) RexxInstructionGuard(expression, variable_list, guardOn);
    return newObject;
}


/**
 * Create a new IF or WHEN instruction object.  These
 * share the same implementation class, but get wired
 * together differently.
 *
 * @param type   The type of instruction to create.
 *
 * @return A an executable IF instruction object.
 */
RexxInstruction *LanguageParser::ifNew()
{
    // ok, get a conditional expression
    RexxInternalObject *_condition = requiredLogicalExpression(TERM_IF, Error_Invalid_expression_if);
    // protect on the term stack
    pushSubTerm(_condition);

    // get to the terminator token for this (likely a THEN, but it could
    // be an EOC.  We use this to update the end location for the instruction since
    // a THEN on the same line would include the THEN (and potentially the following)
    // instruction in the instruction location.  This ensures we truncate at
    // the end of the IF portion.
    RexxToken *token = nextReal();
    previousToken();

    RexxInstruction *newObject =  new_instruction(IF, If);
    ::new ((void *)newObject) RexxInstructionIf(_condition, token);
    return newObject;
}


/**
 * Create a new IF or WHEN instruction object.  These
 * share the same implementation class, but get wired
 * together differently.
 *
 * @param type   The type of instruction to create.
 *
 * @return A an executable IF instruction object.
 */
RexxInstruction *LanguageParser::whenNew()
{
    // the top of the queue must be a SELECT instruction, but
    // we have TWO varieties of this.  The two varieties have different
    // expression parsing rules.
    RexxInstruction *context = topBlockInstruction();
    // no block, this is an error
    if (context == OREF_NULL)
    {
        syntaxError(Error_Unexpected_when_when);
    }

    // parsing of the WHEN depends on the type of SELECT we're currently in.
    if (context->isType(KEYWORD_SELECT))
    {
        // ok, get a conditional expression
        RexxInternalObject *_condition = requiredLogicalExpression(TERM_IF, Error_Invalid_expression_when);
        // protect on the term stack
        pushSubTerm(_condition);

        // get to the terminator token for this (likely a THEN, but it could
        // be an EOC.  We use this to update the end location for the instruction since
        // a THEN on the same line would include the THEN (and potentially the following)
        // instruction in the instruction location.  This ensures we truncate at
        // the end of the IF portion.
        RexxToken *token = nextReal();
        previousToken();
        // this is really an IF instruction
        RexxInstruction *newObject =  new_instruction(WHEN, If);
        ::new ((void *)newObject) RexxInstructionIf(_condition, token);
        return newObject;
    }
    else if (context->isType(KEYWORD_SELECT_CASE))
    {
        // ok, get a conditional expression
        size_t argCount = parseCaseWhenList(TERM_IF);

        // get to the terminator token for this (likely a THEN, but it could
        // be an EOC.  We use this to update the end location for the instruction since
        // a THEN on the same line would include the THEN (and potentially the following)
        // instruction in the instruction location.  This ensures we truncate at
        // the end of the IF portion.
        RexxToken *token = nextReal();
        previousToken();
        // this is really an IF instruction
        RexxInstruction *newObject = new_variable_instruction(WHEN_CASE, CaseWhen, argCount, RexxObject *);
        ::new ((void *)newObject) RexxInstructionCaseWhen(argCount, subTerms, token);
        return newObject;
    }
    // mis-placed WHEN instruction
    else
    {
        syntaxError(Error_Unexpected_when_when);
    }

    return OREF_NULL;
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
    RexxInternalObject *expression = requiredExpression(TERM_EOC, Error_Invalid_expression_interpret);

    RexxInstruction *newObject = new_instruction(INTERPRET, Interpret);
    ::new ((void *)newObject) RexxInstructionInterpret(expression);
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
    ::new ((void *)newObject) RexxInstructionLabel();
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
    ::new ((void *)newObject) RexxInstructionLeave(name);
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
    RexxInstruction *newObject = new_variable_instruction(MESSAGE, Message, msg->argumentCount, RexxObject *);
    ::new ((void *)newObject) RexxInstructionMessage(msg);
    return newObject;
}


/**
 * Create a new Message instruction object for a double tilde
 * message type.
 *
 * @param _message the message expression term...used as a standalone instruction.
 *
 * @return A message object.
 */
RexxInstruction *LanguageParser::doubleMessageNew(RexxExpressionMessage *msg)
{
    ProtectedObject p(msg);
    // just allocate and initialize the object.
    RexxInstruction *newObject = new_variable_instruction(MESSAGE_DOUBLE, Message, msg->argumentCount, RexxObject *);
    ::new ((void *)newObject) RexxInstructionMessage(msg);
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
RexxInstruction *LanguageParser::messageAssignmentNew(RexxExpressionMessage *msg, RexxInternalObject *expr)
{
    ProtectedObject p(msg);               // protect this
    msg->makeAssignment(this);            // convert into an assignment message

    // allocate a new object.  NB:  a message instruction gets an extra argument, so we add one
    RexxInstruction *newObject = new_variable_instruction(MESSAGE, Message, msg->argumentCount + 1, RexxObject *);
    ::new ((void *)newObject) RexxInstructionMessage(msg, expr);
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
RexxInstruction *LanguageParser::messageAssignmentOpNew(RexxExpressionMessage *msg, RexxToken *operation, RexxInternalObject *expr)
{
    ProtectedObject p(expr);        // the message term is already protected, also need to protect this portion
    // There are two things that go on now with the message term,
    // 1) used in the expression evaluation and 2) perform the assignment
    // part.  We copy the original message object to use as a retriever, then
    // convert the original to an assignment message by changing the message name.

    RexxInternalObject *retriever = msg->copy();

    msg->makeAssignment(this);       // convert into an assignment message (the original message term)

    // now add a binary operator to this expression tree using the message copy.
    expr = new RexxBinaryOperator(operation->subtype(), retriever, expr);

    // allocate a new object.  NB:  a message instruction gets an extra argument, so we add one
    RexxInstruction *newObject = new_variable_instruction(MESSAGE, Message, msg->argumentCount + 1, RexxObject *);
    ::new ((void *)newObject) RexxInstructionMessage(msg, expr);
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
    ::new ((void *)newObject) RexxInstructionNop;
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
    RexxInternalObject *expression = OREF_NULL;
    FlagSet<NumericInstructionFlags, 32> _flags;

    // get to the first real token of the instruction
    RexxToken *token = nextReal();

    // all forms of this start with an instruction sub keyword
    if (!token->isSymbol())
    {
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
            expression = parseExpression(TERM_EOC);
            break;

        // NUMERIC FUZZ
        case SUBKEY_FUZZ:
            // set the function and parse the optional expression
            _flags[numeric_fuzz] = true;
            expression = parseExpression(TERM_EOC);
            break;

        // NUMERIC FORM
        // The most complicated of these because it has
        // quite a flavors
        case SUBKEY_FORM:
        {
            // setting the numeric form
            _flags[numeric_form] = true;
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
                switch (token->subKeyword())
                {
                    // NUMERIC FORM SCIENTIFIC
                    case SUBKEY_SCIENTIFIC:
                        _flags[numeric_scientific] = true;
                        // check there's nothing extra
                        requiredEndOfClause(Error_Invalid_data_form);
                        break;

                    // NUMERIC FORM ENGINEERING
                    case SUBKEY_ENGINEERING:
                        // set the engineering flag
                        _flags[numeric_engineering] = true;
                        // check for extra stuff
                        requiredEndOfClause(Error_Invalid_data_form);
                        break;

                    // NUMERIC FORM VALUE expr
                    case SUBKEY_VALUE:
                        expression = requiredExpression(TERM_EOC, Error_Invalid_expression_form);
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
                expression = parseExpression(TERM_EOC);
            }
            break;
        }


        default:
            syntaxError(Error_Invalid_subkeyword_numeric, token);
            break;
    }

    // and create the instruction object.
    RexxInstruction *newObject = new_instruction(NUMERIC, Numeric);
    ::new ((void *)newObject) RexxInstructionNumeric(expression, _flags);
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
    RexxInternalObject *expression = requiredExpression(TERM_EOC, Error_Invalid_expression_options);

    RexxInstruction *newObject = new_instruction(OPTIONS, Options);
    ::new ((void *)newObject) RexxInstructionOptions(expression);
    return newObject;
}


/**
 * Create a new Otherwise instruction object.
 *
 * @param token  The token for the OTHERWISE keyword.
 *
 * @return An executable OTHERWISE instruction object.
 */
RexxInstruction *LanguageParser::otherwiseNew(RexxToken  *token)
{
    RexxInstruction *newObject = new_instruction(OTHERWISE, Otherwise);
    ::new ((void *)newObject) RexxInstructionOtherwise(token);
    return newObject;
}


/**
 * Parse a PARSE instruction.
 *
 * @param argPull Indicates if this is an ARG or PULL operation.
 *
 * @return An executable instruction object.
 */
RexxInstruction *LanguageParser::parseNew(InstructionSubKeyword argPull)
{
    // initialize the source using our initial call value.
    InstructionSubKeyword stringSource = argPull;
    FlagSet<ParseFlags, 32> parseFlags;
    ProtectedObject sourceExpression;

    // if we're working on one of the short forms, there are no source
    // options or other keyword modifiers to worry about.  Both of these
    // use an uppercase version of the string.
    if (argPull != SUBKEY_NONE)
    {
        parseFlags[parse_upper] = true;
    }
    // long form, need to figure out the casing and search options + a source
    // for where the string comes from.
    else
    {
        InstructionSubKeyword keyword = SUBKEY_NONE;
        RexxToken *token;

        // we first need to parse out any of the option modifiers
        for (;;)
        {
            token = nextReal();
            // anything not a symbol is an error
            if (!token->isSymbol())
            {
                syntaxError(Error_Symbol_expected_parse);
            }

            // resolve this to the indexed form
            keyword = token->parseOption();

            switch (keyword)
            {
                // we only handle the control options here

                // PARSE UPPER ... uppercase the input string(s)
                case SUBKEY_UPPER:
                    // if we've already had an UPPER or LOWER option specified,
                    // we drop through and handle this like an unknown keyword.
                    if (parseFlags[parse_upper] || parseFlags[parse_lower])
                    {
                        break;
                    }
                    // remember the option...passed on when we create the instruction,
                    // but also helps trap duplicates.
                    parseFlags[parse_upper] = true;
                    // go directly to the start of the loop again.
                    continue;

                // PARSE LOWER ... lowercase the input string(s)
                case SUBKEY_LOWER:
                    // if we've already had an UPPER or LOWER option specified,
                    // we drop through and handle this like an unknown keyword.
                    if (parseFlags[parse_upper] || parseFlags[parse_lower])
                    {
                        break;
                    }
                    // remember the option...passed on when we create the instruction,
                    // but also helps trap duplicates.
                    parseFlags[parse_lower] = true;
                    // go directly to the start of the loop again.
                    continue;

                // PARSE CASELESS ... all string searches are caseless searches.
                case SUBKEY_CASELESS:
                    // if we've already had a CASELESS option specified,
                    // we drop through and handle this like an unknown keyword.
                    if (parseFlags[parse_caseless])
                    {
                        break;
                    }
                    // remember the option...passed on when we create the instruction,
                    // but also helps trap duplicates.
                    parseFlags[parse_caseless] = true;
                    // go directly to the start of the loop again.
                    continue;

                // any other option?  just fall trough
                default:
                    break;
            }

            // if we've reached here, we've found a keyword that is not an option (or rejected
            // a keyword because it was a duplicate.  Break out of the loop and start handling
            // the string source options.
            break;
        }

        // the last keyword processed above should be our string source,
        // now we need to verify.
        stringSource = keyword;
        switch (keyword)
        {
            // all of these sources are handled internall by the PARSE instruction.  We
            // remember what to do, then it is on to parsing the template.
            case SUBKEY_PULL:
            case SUBKEY_LINEIN:
            case SUBKEY_ARG:
            case SUBKEY_SOURCE:
            case SUBKEY_VERSION:
                break;

            // PARSE VAR must be followed by a variable name.
            case SUBKEY_VAR:
                // must be a symbol, or that is an error.
                token = nextReal();
                if (!token->isSymbol())
                {
                    syntaxError(Error_Symbol_expected_var);
                }

                // get the variable retriever for this.
                sourceExpression = addVariable(token);
                break;

            // PARSE VALUE expr WITH ... The expression is optional...we'll replace with
            // a NULL string if not there.
            case SUBKEY_VALUE:
                sourceExpression = parseExpression(TERM_WITH | TERM_KEYWORD);
                if (sourceExpression.isNull())
                {
                    sourceExpression = GlobalNames::NULLSTRING;
                }
                // the terminator needs to be a WITH keyword.
                token = nextToken();
                if (token->subKeyword() != SUBKEY_WITH)
                {
                    syntaxError(Error_Invalid_template_with);
                }
                break;

            // UNKNOWN (or duplicate) keyword.
            default:
                syntaxError(Error_Invalid_subkeyword_parse, token);
                break;
        }
    }

    // now we're processing the template(s

    // we create multiple lists here.  The different parsing templates are
    // pushed on the subTerms stack, while lists of variables are pushed on to the terms
    // stack.  Referencing them via more descriptive variable names will make this clearer.
    QueueClass *parse_template = subTerms;
    int templateCount = 0;
    QueueClass *_variables = terms;
    int variableCount = 0;

    RexxToken *token = nextReal();
    for (;;)
    {
        if (token->isEndOfClause())
        {
            // we've hit the end of the template, add an END trigger to handle assigning any remaining variables.
            // HOWEVER, if there are no variables between the end and the previous trigger, we can
            // skip this because there are no variables to assign.
            if (variableCount > 0)
            {
                parse_template->push(new (variableCount) ParseTrigger(TRIGGER_END, OREF_NULL, variableCount, _variables));
                templateCount++;
            }
            variableCount = 0;               // have a new set of variables (well, we're done, really)
            break;
        }
        // comma in the template?  Really the same as the end-of-clause case,
        // but we need to push a NULL on to the trigger stack to cause a switch to the
        // next parse string
        else if (token->isComma())
        {
            if (variableCount > 0)
            {
                parse_template->push(new (variableCount) ParseTrigger(TRIGGER_END, OREF_NULL, variableCount, _variables));
                templateCount++;
            }
            // now add an empty fence item to the list...make sure the count is incremented
            parse_template->push(OREF_NULL);
            templateCount++;
            variableCount = 0;               // have a new set of variables
        }
        // some variety of special trigger?
        else if (token->isOperator())
        {
            ParseTriggerType trigger_type = TRIGGER_NONE;
            switch (token->subtype())
            {
                // +num or +(expr) positive relative movement
                case  OPERATOR_PLUS:
                    trigger_type = TRIGGER_PLUS;
                    break;

                // -num or -(expr) form negative relative movement
                case  OPERATOR_SUBTRACT:
                    trigger_type = TRIGGER_MINUS;
                    break;

                // =num or =(expr) absolute column positioning
                case  OPERATOR_EQUAL:
                    trigger_type = TRIGGER_ABSOLUTE;
                    break;

                // <num or <(expr) relative backward movement
                case OPERATOR_LESSTHAN:
                    trigger_type = TRIGGER_MINUS_LENGTH;
                    break;

                // >num or >(expr)
                case OPERATOR_GREATERTHAN:
                    trigger_type = TRIGGER_PLUS_LENGTH;
                    break;

                // something unrecognized
                default:
                    syntaxError(Error_Invalid_template_trigger, token);
                    break;
            }

            // we have an operation.  These will be either a numeric symbol
            // or an expression in parentheses.
            token = nextReal();
            if (token->isLeftParen())
            {
                // parse off an expression in the parens.
                ProtectedObject subExpr = parenExpression(token);
                // an expression is required
                if (subExpr.isNull())
                {
                    syntaxError(Error_Invalid_expression_parse);
                }

                parse_template->push(new (variableCount) ParseTrigger(trigger_type,
                    subExpr, variableCount, _variables));
                variableCount = 0;
                templateCount++;
            }
            // not an expression, we need to find a symbol here.
            else if (token->isSymbol())
            {
                // non variable symbol?
                if (token->isVariable())
                {
                    syntaxError(Error_Invalid_template_position, token);
                }

                // add a trigger of the given type.
                parse_template->push(new (variableCount) ParseTrigger(trigger_type,
                    addText(token), variableCount, _variables));
                variableCount = 0;
                templateCount++;
            }
            // something missing following the operator character?
            else if (token->isEndOfClause())
            {
                syntaxError(Error_Invalid_template_missing);
            }
            // all other errors go here.
            else
            {
                syntaxError(Error_Invalid_template_position, token);
            }
        }
        // could be a variable string search using (expr)
        else if (token->isLeftParen())
        {
            // parse off an expression in the parens.
            ProtectedObject subExpr = parenExpression(token);
            // an expression is required
            if (subExpr.isNull())
            {
                syntaxError(Error_Invalid_expression_parse);
            }

            parse_template->push(new (variableCount) ParseTrigger(parseFlags[parse_caseless] ? TRIGGER_MIXED : TRIGGER_STRING,
                subExpr, variableCount, _variables));
            variableCount = 0;
            templateCount++;
        }
        // a literal string is a string search.
        else if (token->isLiteral())
        {

            parse_template->push(new (variableCount) ParseTrigger(parseFlags[parse_caseless] ? TRIGGER_MIXED : TRIGGER_STRING,
                addText(token), variableCount, _variables));
            variableCount = 0;
            templateCount++;
        }
        // a symbol in the template?  Several possibilities here:  1) a numeric trigger 2)
        else if (token->isSymbol())
        {
            // absolute positioning
            if (token->isNumericSymbol())
            {
                parse_template->push(new (variableCount) ParseTrigger(TRIGGER_ABSOLUTE,
                    addText(token), variableCount, _variables));
                variableCount = 0;

                templateCount++;
            }
            // probably some sort of variable
            else
            {
                // if this is the placeholder period, push a null item on to the stack
                // to mark this variable position as special
                if (token->isDot())
                {
                    _variables->push(OREF_NULL);
                    variableCount++;
                }
                // we have a variable or a message term to process
                else
                {
                    // this is potentially a message term...step back
                    // and parse as one. This also catches simple variables
                    previousToken();
                    RexxInternalObject *term = parseVariableOrMessageTerm();
                    if (term == OREF_NULL)
                    {
                        syntaxError(Error_Variable_expected_PARSE, token);
                    }
                    _variables->push(term);
                    variableCount++;
                }
            }
        }
        // something entirely unknown
        else
        {
            syntaxError(Error_Invalid_template_trigger, token);
        }
        token = nextReal();
    }

    // and finally create the instruction from the accumulated information.
    RexxInstruction *newObject = new_variable_instruction(PARSE, Parse, templateCount, RexxObject *);
    ::new ((void *)newObject) RexxInstructionParse(sourceExpression, stringSource, parseFlags, templateCount, parse_template);
    return newObject;
}

/**
 * Parse a PROCEDURE instruction.
 *
 * @return An executable instruction object.
 */
RexxInstruction *LanguageParser::procedureNew()
{
    RexxToken *token = nextReal();
    size_t variableCount = 0;

    // if we're not at the END, then this might be PROCEDURE expose.
    if (!token->isEndOfClause())
    {
        // not the EXPOSE keyword?  NOTE:  this also takes care of the
        // non-symbol case since we issue the same error message for both
        if (token->subKeyword() != SUBKEY_EXPOSE)
        {
            syntaxError(Error_Invalid_subkeyword_procedure, token);
        }
        // process the list                  */
        variableCount = processVariableList(KEYWORD_PROCEDURE);
    }

    RexxInstruction *newObject = new_variable_instruction(PROCEDURE, Procedure, variableCount, RexxObject *);
    ::new ((void *)newObject) RexxInstructionProcedure(variableCount, subTerms);
    return newObject;
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
    RexxInternalObject *expression = parseExpression(TERM_EOC);

    RexxInstruction *newObject = new_instruction(QUEUE, Queue);
    ::new ((void *)newObject) RexxInstructionQueue(expression);
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
    RexxInternalObject *expression = parseExpression(TERM_EOC);

    // NOTE:  PUSH and QUEUE share an implementation class, but the
    // instruction type identifies which operation is performed.
    RexxInstruction *newObject = new_instruction(PUSH, Queue);
    ::new ((void *)newObject) RexxInstructionQueue(expression);
    return newObject;
}


/**
 * Parse a RAISE instruction and return a new executable object.
 *
 * @return An instructon object that can execute this RAISE instruction.
 */
RexxInstruction *LanguageParser::raiseNew()
{
    ArrayClass *arrayItems = OREF_NULL;
    RexxInternalObject *rcValue = OREF_NULL;
    RexxInternalObject *description = OREF_NULL;
    RexxInternalObject *additional = OREF_NULL;
    RexxInternalObject *result = OREF_NULL;
    FlagSet<RaiseInstructionFlags, 32> flags;

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
            rcValue = parseConstantExpression();
            // this is required.  If not found, use the terminator
            // token to report the error location
            if (rcValue == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_general, nextToken());
            }
            // add this to the sub terms queue
            pushSubTerm(rcValue);
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
            _condition = _condition->concatToCstring("USER ");
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
            break;

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
        InstructionSubKeyword option = token->subKeyword();
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
                description = parseConstantExpression();
                if (description == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_raise_description);
                }
                // protect from GC.
                pushSubTerm(description);
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
                additional = parseConstantExpression();
                if (additional == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_raise_additional);
                }
                pushSubTerm(additional);
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
                if (!token->isLeftParen())
                {
                    syntaxError(Error_Invalid_expression_raise_list);
                }
                // process this like an argument list.  Usually, we'd
                // leave this on the subTerms stack, but we're pushing
                // other items on there that will mess things up.  So,
                // we grab this in an array.
                arrayItems = parseArgArray(token, TERM_RIGHT);
                pushSubTerm(arrayItems);
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
                result = parseConstantExpression();
                // this is actually optional
                if (result != OREF_NULL)
                {
                    pushSubTerm(result);
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
                result = parseConstantExpression();
                if (result != OREF_NULL)
                {
                    pushSubTerm(result);
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
        newObject = new_variable_instruction(RAISE, Raise, arrayCount, RexxObject *);
    }
    // fixed instruction size
    else
    {
        newObject = new_instruction(RAISE, Raise);
    }
    ::new ((void *)newObject) RexxInstructionRaise(_condition, rcValue, description, additional, result, flags);
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
    RexxInternalObject *expression = parseExpression(TERM_EOC);

    RexxInstruction *newObject = new_instruction(REPLY, Reply);
    ::new ((void *)newObject) RexxInstructionReply(expression);
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
    RexxInternalObject *expression = parseExpression(TERM_EOC);

    RexxInstruction *newObject = new_instruction(RETURN, Return);
    ::new ((void *)newObject) RexxInstructionReturn(expression);
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
    RexxInternalObject *expression = parseExpression(TERM_EOC);

    RexxInstruction *newObject = new_instruction(SAY, Say);
    ::new ((void *)newObject) RexxInstructionSay(expression);
    return newObject;
}


/**
 * Parse and create a suitable SELECT instruction object.
 * This has different objects for standard SELECT and
 * SELECT CASE instructions.
 *
 * @return An executable SELECT instruction object.
 */
RexxInstruction *LanguageParser::selectNew()
{
    // SELECT can be either SELECT; or SELECT CASE.  We have different
    // classes for the two different functions.
    RexxToken *token = nextReal();
    RexxString *label = OREF_NULL;
    RexxInternalObject *caseExpr = OREF_NULL;

    // ok, if this is more than just SELECT, we need to handle both a LABEL option
    // (which must be first) and a potential CASE option, which creates a completely different
    // instruction type.
    if (!token->isEndOfClause())
    {
        if (!token->isSymbol())
        {
            // not a LABEL keyword, this is bad
            syntaxError(Error_Invalid_subkeyword_select, token); // SELECT must be followed by the keyword LABEL or CASE
        }

        // potentially a label.  At this point, not being the label keyword is
        // not an error...it could be CASE.  We just handle LABEL here.
        if (token->subKeyword() == SUBKEY_LABEL)
        {
            // ok, get the label now
            token = nextReal();
            // this is required, and must be a symbol
            if (!token->isSymbol())
            {
                syntaxError(Error_Symbol_expected_LABEL);
            }

            label = token->value();
            // step to the next token and handle a potential
            // CASE instruction
            token = nextReal();
        }
        // if we have anything left, this must be a CASE option
        if (token->isSymbol())
        {
            // potentially a CASE option.  Anything else is an error here
            if (token->subKeyword() != SUBKEY_CASE)
            {
                syntaxError(Error_Invalid_subkeyword_select, token); // SELECT must be followed by the keyword LABEL or CASE
            }

            // eat the rest of the expression for the CASE, which is
            // required
            caseExpr = requiredExpression(TERM_EOC, Error_Invalid_expression_select_case);

            // get the terminator token
            token = nextReal();
        }
        // this must be the end token here.
        if (!token->isEndOfClause())
        {
            syntaxError(Error_Invalid_subkeyword_select, token); // SELECT must be followed by the keyword LABEL or CASE
        }
    }

    // normal SELECT instruction?
    if (caseExpr == OREF_NULL)
    {
        // ok, finally allocate this and return
        RexxInstruction *newObject = new_instruction(SELECT, Select);
        ::new ((void *)newObject) RexxInstructionSelect(label);
        return  newObject;
    }
    // this the SELECT CASE version.  Fundamentally a different instruction.
    else
    {
        // ok, finally allocate this and return
        RexxInstruction *newObject = new_instruction(SELECT_CASE, SelectCase);
        ::new ((void *)newObject) RexxInstructionSelectCase(label, caseExpr);
        return  newObject;
    }

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
    RexxInternalObject *labelExpression = requiredExpression(TERM_EOC, Error_Invalid_expression_signal);

    // create a new instruction object
    RexxInstruction *newObject = new_instruction(SIGNAL_VALUE, DynamicSignal);
    ::new ((void *)newObject) RexxInstructionDynamicSignal(labelExpression);

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
        conditionName = commonString(labelName->concatToCstring("USER "));
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
            labelName = token->value();
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
    ::new ((void *)newObject) RexxInstructionSignalOn(conditionName, labelName);

    // if this is the ON form, we have some end parsing resolution to perform.
    if (type == SUBKEY_ON)
    {
        addReference(newObject);
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
        return dynamicSignalNew();
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
                return dynamicSignalNew();
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
    ::new ((void *)newObject) RexxInstructionSignal(labelName);

    // this requires a resolve call back once the labels are determined.
    addReference(newObject);
    return newObject;
}


/**
 * Create a new THEN instruction object.
 *
 * @param token  The THEN token (used for location information).
 * @param parent The parent IF or WHEN instruction we're part of.
 *
 * @return An executable instruction object.
 */
RexxInstruction *LanguageParser::thenNew(RexxToken *token, RexxInstructionIf *parent )
{
    // no additional parsing needed here, we just create the instruction object.
    RexxInstruction *newObject = new_instruction(THEN, Then);
    ::new ((void *)newObject) RexxInstructionThen(token, parent);
    return newObject;
}


/**
 * Parse and create a new Trace instruction object.
 *
 * @return A Trace instruction object.
 */
RexxInstruction *LanguageParser::traceNew()
{
    TraceSetting settings;                      // the parsed trace setting flags
    bool skipForm = false;                      // set default trace mode
    wholenumber_t debug_skip = 0;               // no skipping
    RexxInternalObject *expression = OREF_NULL; // not expression form

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
                expression = requiredExpression(TERM_EOC, Error_Invalid_expression_trace);
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
                    if (!settings.parseTraceSetting(value, badOption))
                    {
                        syntaxError(Error_Invalid_trace_trace, new_string(&badOption, 1));
                    }
                }
                // a valid number, this is the skip form
                else
                {
                    skipForm = true;
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
                // parse into
                if (!settings.parseTraceSetting(value, badOption))
                {
                    syntaxError(Error_Invalid_trace_trace, new_string(&badOption, 1));
                }
            }
            else
            {
                // remember that this is the skip form
                skipForm = true;
            }
        }
        // potential numeric value with a sign?
        else if (token->isSubtype(OPERATOR_SUBTRACT, OPERATOR_PLUS))
        {
            // remember that this is the skip form
            skipForm = true;

            // we expect to find a number
            RexxToken *second = nextReal();
            if (second->isEndOfClause())
            {
                syntaxError(Error_Invalid_expression_general, second);
            }
            // this must be a symbol or a literal value
            if (!second->isSymbolOrLiteral())
            {
                syntaxError(Error_Invalid_expression_general, second);
            }

            // this needs to be the end of the clause
            RexxString *value = second->value();
            requiredEndOfClause(Error_Invalid_data_trace);

            // convert to a binary number
            if (!value->requestNumber(debug_skip, number_digits()))
            {
                syntaxError(Error_Invalid_whole_number_trace, value);
            }

            // minus form?  then negate the value
            if (token->isSubtype(OPERATOR_SUBTRACT))
            {
                debug_skip = -debug_skip;
                // turns off tracing entirely
            }
        }
        // implicit TRACE VALUE form
        else
        {
            // take a step back and parse the expression
            previousToken();
            expression = parseExpression(TERM_EOC);
        }
    }

    RexxInstruction *newObject = new_instruction(TRACE, Trace);
    // this is one of three forms
    if (skipForm)
    {
        ::new ((void *)newObject) RexxInstructionTrace(debug_skip);
    }
    else if (expression != OREF_NULL)
    {
        ::new ((void *)newObject) RexxInstructionTrace(expression);
    }
    else
    {
        ::new ((void *)newObject) RexxInstructionTrace(settings);
    }

    return newObject;
}


/**
 * Parse a USE ARG or USE STRICT ARG instruction.
 *
 * @return The executable instruction object.
 */
RexxInstruction *LanguageParser::useNew()
{
    bool strictChecking = false;  // no strict checking enabled yet
    bool referencesUsed = false;  // no aliasing required yet

    // The STRICT keyword turns this into a different instruction with different
    // syntax rules
    RexxToken *token = nextReal();
    InstructionSubKeyword subkeyword = token->subKeyword();

    // check for USE LOCAL first. we really just handle the USE ARG in here
    if (subkeyword == SUBKEY_LOCAL)
    {
        return useLocalNew();
    }

    if (subkeyword == SUBKEY_STRICT)
    {
        token = nextReal();        // skip over the token
        strictChecking = true;     // apply the strict checks.
    }

    // the only subkeyword supported is ARG
    if (token->subKeyword() != SUBKEY_ARG)
    {
        syntaxError(strictChecking == true ? Error_Invalid_subkeyword_use_strict : Error_Invalid_subkeyword_use, token);
    }

    // we accumulate 2 sets of data here, so we need 2 queues to push them in
    // if this is the SIMPLE version, the second queue will be empty.
    size_t variableCount = 0;
    Protected<QueueClass> variable_list = new_queue();         // we might be parsing message terms, so we can't use the subterms list.
    Protected<QueueClass> defaults_list = new_queue();
    token = nextReal();                  /* get the next token                */

    bool allowOptionals = false;  // we don't allow trailing optionals unless the list ends with "..."
    // keep processing tokens to the end
    while (!token->isEndOfClause())
    {
        // this could be a token to skip a variable
        if (token->isComma())
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
                if (token->value()->strCompare("..."))
                {
                    // ok, this is the end of everything.  Tell the instructions to not enforce the max rules
                    allowOptionals = true;
                    // but we still need to make sure it's at the end
                    token = nextReal();
                    if (!token->isEndOfClause())
                    {
                        syntaxError(Error_Translation_use_arg_ellipsis);
                    }
                    break;  // done parsing
                }
            }
            // this could be a '>' or ',' prefix used to indicate variable aliasing
            else if (token->isOperator(OPERATOR_GREATERTHAN) || token->isOperator(OPERATOR_LESSTHAN))
            {
                // this must be a simple variable symbol or stem symbol
                token = nextReal();
                if (!token->isSymbol() || !token->isNonCompoundVariable())
                {
                    syntaxError(Error_Symbol_expected_after_use_arg_reference, token);
                }

                // get the variable retriever for this variable.
                RexxInternalObject *retriever = addText(token);

                // step to the next token, which should be a comma or a end of clause
                token = nextReal();
                // if this is not a comma, then we need to do some additional checking
                if (!token->isComma())
                {
                    // Check to see if a default is specified. This is not
                    // allowed for references. We could give a generic error, but
                    // it is better to be explicit about the problem,
                    if (token->isSubtype(OPERATOR_EQUAL))
                    {
                        syntaxError(Error_Translation_use_arg_reference_no_default);
                    }
                    // anything other than a end of clause is a generic error
                    else if (!token->isEndOfClause())
                    {
                        // if not an assignment, this needs to be a comma.
                        syntaxError(Error_Variable_reference_use, token);
                    }
                }
                // we had a terminating comma, so step to the next token
                else
                {
                    token = nextReal();
                }

                // we wrap this in a special retriever to handle the aliasing
                Protected<UseArgVariableRef> ref = new UseArgVariableRef((RexxVariableBase *)retriever);

                // add the reference proxy as the variable, and add a NULL to the defaults
                // to keep them in sync.
                variable_list->push(ref);
                defaults_list->push(OREF_NULL);
                variableCount++;
                // continue the loop. Note that token is already positioned for the
                // next section
                continue;
            }

            previousToken();       // push the current token back for term processing
            // see if we can get a variable or a message term from this
            RexxInternalObject *retriever = parseVariableOrMessageTerm();
            if (retriever == OREF_NULL)
            {
                syntaxError(Error_Variable_expected_USE, token);
            }
            variable_list->push(retriever);
            variableCount++;
            token = nextReal();
            // a terminator takes us out.  We need to keep our lists in sync with dummy entries.
            if (token->isEndOfClause())
            {
                defaults_list->push(OREF_NULL);
                break;
            }
            // if we've hit a comma here, step to the next token and continue with the next variable
            else if (token->isComma())
            {
                defaults_list->push(OREF_NULL);
                token = nextReal();
                continue;
            }
            // if this is NOT a comma, we potentially have a
            // default value
            if (token->isSubtype(OPERATOR_EQUAL))
            {
                // this is a constant expression value.  Single token forms
                // are fine without parens, more complex forms require parens as
                // delimiters.
                RexxInternalObject *defaultValue = parseConstantExpression();
                // not a constant expression is an error
                if (defaultValue == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_use_arg_default);
                }

                // add this to the defaults
                defaults_list->push(defaultValue);
                // step to the next token
                token = nextReal();
                // a terminator takes us out.
                if (token->isEndOfClause())
                {
                    break;
                }
                // if we've hit a comma here, step to the next token and continue with the next variable
                else if (token->isComma())
                {
                    token = nextReal();
                    continue;
                }
                // not a comma is an error
                else
                {
                    syntaxError(Error_Invalid_expression_use_arg_default);
                }
            }
            else
            {
                // if not an assignment, this needs to be a comma.
                syntaxError(Error_Variable_reference_use, token);
            }
        }
    }

    RexxInstruction *newObject = new_variable_instruction(USE, Use, variableCount, UseVariable);
    ::new ((void *)newObject) RexxInstructionUse(variableCount, strictChecking, allowOptionals, variable_list, defaults_list);

    return newObject;
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
            else if (token->isSubtype(SYMBOL_DUMMY, SYMBOL_DOTSYMBOL))
            {
                syntaxError(Error_Invalid_variable_period, token);
            }

            // ok, get a retriever for the variable and push it on the stack.
            pushSubTerm(addText(token));

            // are we processing an expose instruction?  keep track of this variable
            // in case we use GUARD WHEN
            if (type == KEYWORD_EXPOSE)
            {
                expose(token->value());
            }
            // update our return value.
            listCount++;
        }
        // this could be an indirect reference through a list of form "(var)"
        else if (token->isLeftParen())
        {
            listCount++;
            // get the next token...which should be a valid variable
            token = nextReal();

            if (!token->isSymbol())
            {
                syntaxError(Error_Symbol_expected_varref);
            }

            // get a retriever for the variable in the parens
            RexxVariableBase *retriever = addVariable(token);
            // and wrap this in a variable reference, which handles the indirection.
            retriever = new RexxVariableReference(retriever);
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
            else if (!token->isRightParen())
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
    return listCount;
}


/**
 * Allow for WHILE or UNTIL keywords following some other
 * looping construct.  This returns SUBKEY_WHILE or SUBKEY_UNTIL
 * to flag the caller that a conditional has been used.
 *
 * @param condition_type
 *               An option pointer for a place to return the condition type (WHILE or UNTIL)
 * @param error_message
 *
 * @return A parsed out expression object.
 */
RexxInternalObject *LanguageParser::parseLoopConditional(InstructionSubKeyword &conditionType, RexxErrorCodes error_message)
{
    RexxInternalObject *conditional = OREF_NULL;

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
                conditional = requiredLogicalExpression(TERM_COND, Error_Invalid_expression_while);
                // protect on the term stack
                pushSubTerm(conditional);

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
                conditional = requiredLogicalExpression(TERM_COND, Error_Invalid_expression_until);
                // protect on the term stack
                pushSubTerm(conditional);

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
