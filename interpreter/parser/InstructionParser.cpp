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
/* REXX Kernel                                                                */
/*                                                                            */
/* Primitive Translator Object Constructors                                   */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* NOTE:      Start of new methods for translator class objects.  These new   */
/*            methods are segregated here to allow the actual translator to   */
/*            be easily decoupled from the rest of the interpreter.           */
/*            All of these methods are actually methods of the SOURCE         */
/*            class.  They are in a seperate module for locational            */
/*            convenience.                                                    */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "RexxActivation.hpp"
#include "SourceFile.hpp"                /* this is part of source            */

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


RexxInstruction *RexxSource::addressNew()
/****************************************************************************/
/* Function:  Create a new ADDRESS translator object                        */
/****************************************************************************/
{
    RexxObject *_expression = OREF_NULL;             /* initialize import state variables */
    RexxString *environment = OREF_NULL;
    RexxObject *command = OREF_NULL;
    RexxToken *token = nextReal();                  /* get the next token                */
    if (!token->isEndOfClause())
    {        /* something other than a toggle?    */
        /* something other than a symbol or  */
        /* string?...implicit value form     */
        if (!token->isSymbolOrLiteral())
        {
            previousToken();                 /* back up one token                 */
                                             /* process the expression            */
            _expression = this->expression(TERM_EOC);
        }
        else
        {                             /* have a constant address target    */
                                      /* may be value keyword, however...  */
            if (this->subKeyword(token) == SUBKEY_VALUE)
            {
                /* process the expression            */
                _expression = this->expression(TERM_EOC);
                if (_expression == OREF_NULL)   /* no expression?                    */
                {
                    /* expression is required after value*/
                    syntaxError(Error_Invalid_expression_address);
                }
            }
            else
            {                           /* have a real constant target       */
                environment = token->value;    /* get the actual name value         */
                token = nextReal();            /* get the next token                */
                                               /* have a command following name?    */
                if (!token->isEndOfClause())
                {
                    previousToken();             /* step back a token                 */
                                                 /* process the expression            */
                    command = this->expression(TERM_EOC);
                }
            }
        }                                  /* resolve the subkeyword            */
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(ADDRESS, Address);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionAddress(_expression, environment, command);
    return newObject; /* done, return this                 */
}


RexxInstruction *RexxSource::assignmentNew(
     RexxToken  *target )              /* target variable instruction       */
/****************************************************************************/
/* Function:  Create a new ASSIGNMENT translator object                     */
/****************************************************************************/
{
    this->needVariable(target);          /* must be a variable                */
                                         /* process the expression            */
    RexxObject *_expression = this->expression(TERM_EOC);
    if (_expression == OREF_NULL)        /* no expression here?               */
    {
        /* this is invalid                   */
        syntaxError(Error_Invalid_expression_assign);
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(ASSIGNMENT, Assignment);
    new ((void *)newObject) RexxInstructionAssignment((RexxVariableBase *)(this->addText(target)), _expression);
    return newObject; /* done, return this                 */
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
RexxInstruction *RexxSource::assignmentOpNew(RexxToken *target, RexxToken *operation)
{
    this->needVariable(target);     // make sure this is a variable
    // we require an expression for the additional part, which is required
    RexxObject *_expression = this->expression(TERM_EOC);
    if (_expression == OREF_NULL)
    {
        syntaxError(Error_Invalid_expression_assign);
    }

    // we need an evaluator for both the expression and the assignment
    RexxObject *variable = addText(target);
    // now add a binary operator to this expression tree
    _expression = (RexxObject *)new RexxBinaryOperator(operation->subclass, variable, _expression);

    // now everything is the same as an assignment operator
    RexxInstruction *newObject = new_instruction(ASSIGNMENT, Assignment);
    new ((void *)newObject) RexxInstructionAssignment((RexxVariableBase *)variable, _expression);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::callNew()
/****************************************************************************/
/* Function:  Create a new CALL translator object                           */
/****************************************************************************/
{
    size_t _flags = 0;                          /* clear the flags                   */
    size_t builtin_index = 0;                   /* clear the builtin index           */
    RexxString *_condition = OREF_NULL;              /* clear the condition               */
    RexxObject *name = OREF_NULL;                    /* no name yet                       */
    size_t argCount = 0;                        /* no arguments yet                  */

    RexxToken *token = nextReal();                  /* get the next token                */
    if (token->isEndOfClause())     /* no target specified?              */
    {
        /* this is an error                  */
        syntaxError(Error_Symbol_or_string_call);
    }
    /* may have to process ON/OFF forms  */
    else if (token->isSymbol())
    {
        int _keyword = this->subKeyword(token); /* check for the subkeywords         */
        if (_keyword == SUBKEY_ON)
        {        /* CALL ON instruction?              */
            _flags |= RexxInstructionCall::call_on_off;  /* this is a CALL ON                 */
            token = nextReal();              /* get the next token                */
                                             /* no condition specified or not a   */
                                             /* symbol?                           */
            if (!token->isSymbol())
            {
                /* this is an error                  */
                syntaxError(Error_Symbol_expected_on);
            }
            _keyword = this->condition(token);/* get the condition involved        */
            if (_keyword == 0 ||              /* invalid condition specified?      */
                _keyword == CONDITION_SYNTAX ||
                _keyword == CONDITION_NOVALUE ||
                _keyword == CONDITION_PROPAGATE ||
                _keyword == CONDITION_LOSTDIGITS ||
                _keyword == CONDITION_NOMETHOD ||
                _keyword == CONDITION_NOSTRING)
            {
                /* got an error here                 */
                syntaxError(Error_Invalid_subkeyword_callon, token);
            }

            /* actually a USER condition request?*/
            else if (_keyword == CONDITION_USER)
            {
                token = nextReal();            /* get the next token                */
                                               /* no condition specified or not a   */
                                               /* symbol?                           */
                if (!token->isSymbol())
                {
                    /* this is an error                  */
                    syntaxError(Error_Symbol_expected_user);
                }
                /* set the builtin index for later   */
                /* resolution step                   */
                builtin_index = this->builtin(token);
                _condition = token->value;     /* get the token string value        */
                name = _condition;             /* set the default target            */
                                               /* condition name is "USER condition"*/
                _condition = _condition->concatToCstring(CHAR_USER_BLANK);
                /* save the condition name           */
                _condition = this->commonString(_condition);
            }
            else
            {                           /* language defined condition        */
                name = token->value;           /* set the default target            */
                _condition = token->value;     /* condition is the same as target   */
                                               /* set the builtin index for later   */
                                               /* resolution step                   */
                builtin_index = this->builtin(token);
            }
            token = nextReal();              /* get the next token                */
                                             /* anything real here?               */
            if (!token->isEndOfClause())
            {
                /* not a symbol?                     */
                if (!token->isSymbol())
                {
                    /* this is an error                  */
                    syntaxError(Error_Invalid_subkeyword_callonname, token);
                }
                /* not the name token?               */
                if (this->subKeyword(token) != SUBKEY_NAME)
                {
                    /* got an error here                 */
                    syntaxError(Error_Invalid_subkeyword_callonname, token);
                }
                token = nextReal();            /* get the next token                */
                if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
                {
                    /* this is an error                  */
                    syntaxError(Error_Symbol_or_string_name);
                }
                name = token->value;           /* set the default target            */
                                               /* set the builtin index for later   */
                                               /* resolution step                   */
                builtin_index = this->builtin(token);
                token = nextReal();            /* get the next token                */
                                               /* must have the clause end here     */
                if (!token->isEndOfClause())
                {
                    /* this is an error                  */
                    syntaxError(Error_Invalid_data_name, token);
                }
            }
        }
        else if (_keyword == SUBKEY_OFF)
        { /* CALL OFF instruction?             */
            token = nextReal();              /* get the next token                */
                                             /* no condition specified or not a   */
                                             /* symbol?                           */
            if (!token->isSymbol())
            {
                /* this is an error                  */
                syntaxError(Error_Symbol_expected_off);
            }
            /* get the condition involved        */
            _keyword = this->condition(token);
            if (_keyword == 0 ||              /* invalid condition specified?      */
                _keyword == CONDITION_SYNTAX ||
                _keyword == CONDITION_NOVALUE ||
                _keyword == CONDITION_PROPAGATE ||
                _keyword == CONDITION_LOSTDIGITS ||
                _keyword == CONDITION_NOMETHOD ||
                _keyword == CONDITION_NOSTRING)
            {
                /* got an error here                 */
                syntaxError(Error_Invalid_subkeyword_calloff, token);
            }
            /* actually a USER condition request?*/
            else if (_keyword == CONDITION_USER)
            {
                token = nextReal();            /* get the next token                */
                                               /* no condition specified or not a   */
                                               /* symbol?                           */
                if (!token->isSymbol())
                {
                    /* this is an error                  */
                    syntaxError(Error_Symbol_expected_user);
                }
                _condition = token->value;      /* get the token string value        */
                /* condition name is "USER condition"*/
                _condition = _condition->concatToCstring(CHAR_USER_BLANK);
                /* save the condition name           */
                _condition = this->commonString(_condition);
            }
            else                             /* language defined condition        */
            {
                _condition = token->value;      /* set the condition name            */
            }
            token = nextReal();              /* get the next token                */
            if (!token->isEndOfClause()) /* must have the clause end here     */
            {
                /* this is an error                  */
                syntaxError(Error_Invalid_data_condition, token);
            }
        }
        else
        {                             /* normal CALL instruction           */
                                      /* set the default target            */
            name = (RexxString *)token->value;
            /* set the builtin index for later   */
            /* resolution step                   */
            builtin_index = this->builtin(token);
            /* get the argument list             */
            argCount = this->argList(OREF_NULL, TERM_EOC);
        }
    }
    /* call with a string target         */
    else if (token->isLiteral())
    {
        name = token->value;               /* set the default target            */
                                           /* set the builtin index for later   */
                                           /* resolution step                   */
        builtin_index = this->builtin(token);
        /* process the argument list         */
        argCount = this->argList(OREF_NULL, TERM_EOC);
        _flags |= RexxInstructionCall::call_nointernal;          /* do not check for internal routines*/
    }
    /* indirect call case?               */
    else if (token->classId == TOKEN_LEFT)
    {
        _flags |= RexxInstructionCall::call_dynamic;             /* going to be indirect              */
        name = this->parenExpression(token); // this is a full expression
        /* process the argument list         */
        argCount = this->argList(OREF_NULL, TERM_EOC);
        /* NOTE:  this call is not added to  */
        /* the resolution list because it    */
        /* cannot be resolved until run time */
    }
    else
    {
        /* this is an error                  */
        syntaxError(Error_Symbol_or_string_call);
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_variable_instruction(CALL, Call, sizeof(RexxInstructionCallBase) + argCount * sizeof(RexxObject *));
    /* Initialize this new object        */
    new ((void *)newObject) RexxInstructionCall(name, _condition, argCount, this->subTerms, _flags, builtin_index);

    if (!(flags&RexxInstructionCall::call_dynamic))           /* static name resolution?           */
    {
        this->addReference((RexxObject *)newObject);     /* add to table of references        */
    }
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::commandNew()
/****************************************************************************/
/* Function:  Create a new COMMAND instruction object                       */
/****************************************************************************/
{
    /* process the expression            */
    RexxObject *_expression = this->expression(TERM_EOC);
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(COMMAND, Command);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionCommand(_expression);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::doNew()
/****************************************************************************/
/* Function:  Create a new DO translator object                             */
/****************************************************************************/
{
    RexxInstruction *newObject = createLoop();
                                       /* now complete the parsing          */
    return this->createDoLoop((RexxInstructionDo *)newObject, false);
}

RexxInstruction *RexxSource::loopNew()
/****************************************************************************/
/* Function:  Create a new LOOP translator object                           */
/****************************************************************************/
{
    RexxInstruction *newObject = createLoop();
                                       /* now complete the parsing          */
    return this->createDoLoop((RexxInstructionDo *)newObject, true);
}


/**
 * Create a LOOP/DO instruction instance.
 *
 * @return
 */
RexxInstruction *RexxSource::createLoop()
{
    // NOTE:  we create a DO instruction for both DO and LOOP
    RexxInstruction *newObject = new_instruction(DO, Do); /* create a new translator object    */
    new((void *)newObject) RexxInstructionDo;
    return newObject;
}


RexxInstruction *RexxSource::createDoLoop(RexxInstructionDo *newDo, bool isLoop)
/******************************************************************************/
/* Function:  Create a new DO translator object                               */
/******************************************************************************/
{
    size_t _position = markPosition();   // get a reset position before getting next token
    // We've already figured out this is either a LOOP or a DO, now we need to
    // decode what else is going on.
    RexxToken *token = nextReal();
    int conditional;


    // before doing anything, we need to test for a "LABEL name" before other options.  We
    // need to make certain we check that the LABEL keyword is not being
    // used as control variable.
    if (token->isSymbol())
    {
        // potentially a label.  Check the keyword value
        if (this->subKeyword(token) == SUBKEY_LABEL)
        {
            // if the next token is a symbol, this is a label.
            RexxToken *name = nextReal();
            if (name->isSymbol())
            {
                OrefSet(newDo, newDo->label, name->value);
                // update the reset position before stepping to the next position
                _position = markPosition();
                // step to the next token for processing the following parts
                token = nextReal();
            }
            else if (name->subclass == OPERATOR_EQUAL)
            {
                // back up to the LABEL token and process as normal
                previousToken();
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
        if (isLoop)
        {
            newDo->type = DO_FOREVER;    // a LOOP with nothing else is a LOOP FOREVER
        }
        else
        {
            newDo->type = SIMPLE_DO;     // this is a simple DO block
        }
        return newDo;
    }


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
            newDo->type = CONTROLLED_DO;   // this is a controlled DO loop
            // the control expressions need to evaluated in the coded order,
            // so we keep a little table of the order used.
            int keyslot = 0;
            // if this DO/LOOP didn't have a label clause, use the control
            // variable name as the loop name.
            if (newDo->label == OREF_NULL)
            {
                OrefSet(newDo, newDo->label, token->value);
            }

            // get the variable retriever
            OrefSet(newDo, newDo->control, (RexxVariableBase *)this->addVariable(token));
            // go get the initial expression, using the control variable terminators
            OrefSet(newDo, newDo->initial, this->expression(TERM_CONTROL));
            // this is a required expression.
            if (newDo->initial == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_control);
            }
            // ok, keep looping while we don't have a clause terminator
            // because the parsing of the initial expression is terminated by either
            // the end-of-clause or DO/LOOP keyword, we know the next token will by
            // a symbol
            token = nextReal();
            while (!token->isEndOfClause())
            {
                // this must be a keyword, so resolve it.
                switch (this->subKeyword(token) )
                {

                    case SUBKEY_BY:
                        // only one per customer
                        if (newDo->by != OREF_NULL)
                        {
                            syntaxError(Error_Invalid_do_duplicate, token);
                        }
                        // get the keyword expression, which is required also
                        OrefSet(newDo, newDo->by, this->expression(TERM_CONTROL));
                        if (newDo->by == OREF_NULL)
                        {
                            syntaxError(Error_Invalid_expression_by);

                        }
                        // record the processing order
                        newDo->expressions[keyslot++] = EXP_BY;
                        break;

                    case SUBKEY_TO:
                        // only one per customer
                        if (newDo->to != OREF_NULL)
                        {
                            syntaxError(Error_Invalid_do_duplicate, token);
                        }
                        // get the keyword expression, which is required also
                        OrefSet(newDo, newDo->to, this->expression(TERM_CONTROL));
                        if (newDo->to == OREF_NULL)
                        {
                            syntaxError(Error_Invalid_expression_to);

                        }
                        // record the processing order
                        newDo->expressions[keyslot++] = EXP_TO;
                        break;

                    case SUBKEY_FOR:
                        // only one per customer
                        if (newDo->forcount != OREF_NULL)
                        {
                            syntaxError(Error_Invalid_do_duplicate, token);
                        }
                        // get the keyword expression, which is required also
                        OrefSet(newDo, newDo->forcount, this->expression(TERM_CONTROL));
                        if (newDo->forcount == OREF_NULL)
                        {
                            syntaxError(Error_Invalid_expression_for);

                        }
                        // record the processing order
                        newDo->expressions[keyslot++] = EXP_FOR;
                        break;

                    case SUBKEY_WHILE:
                        // step back a token and process the conditional
                        previousToken();
                        OrefSet(newDo, newDo->conditional, this->parseConditional(NULL, 0));
                        previousToken();         // place the terminator back
                        // this changes the loop type
                        newDo->type = CONTROLLED_WHILE;
                        break;

                    case SUBKEY_UNTIL:
                        // step back a token and process the conditional
                        previousToken();
                        OrefSet(newDo, newDo->conditional, this->parseConditional(NULL, 0));
                        previousToken();         // place the terminator back
                        // this changes the loop type
                        newDo->type = CONTROLLED_UNTIL;
                        break;
                }
                token = nextReal();          /* step to the next token            */
            }
        }
        // DO name OVER collection form?
        else if (this->subKeyword(second) == SUBKEY_OVER)
        {
            // this is a DO OVER type
            newDo->type = DO_OVER;
            // if this DO/LOOP didn't have a label clause, use the control
            // variable name as the loop name.
            if (newDo->label == OREF_NULL)
            {
                OrefSet(newDo, newDo->label, token->value);
            }
            // save the control variable retriever
            OrefSet(newDo, newDo->control, (RexxVariableBase *)this->addVariable(token));
            // and get the OVER expression, which is required
            OrefSet(newDo, newDo->initial, this->expression(TERM_COND));
            /* no expression?                    */
            if (newDo->initial == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_over);
            }
            // process an additional conditional
            OrefSet(newDo, newDo->conditional, this->parseConditional(&conditional, 0));
            // if we have a conditional, we need to change type DO type.
            if (conditional == SUBKEY_WHILE)
            {
                newDo->type = DO_OVER_WHILE;  // this is the DO var OVER expr WHILE cond form
            }
            else if (conditional == SUBKEY_UNTIL)
            {
                newDo->type = DO_OVER_UNTIL;  // this is the DO var OVER expr UNTIL cond form
            }
        }
        else // not a controlled form, but this could be a conditional form.
        {
            // start the parsing process over from the beginning with the first of the
            // loop-type tokens
            resetPosition(_position);
            token = nextReal();
            // now check the other keyword varieties.
            switch (this->subKeyword(token))
            {
                case SUBKEY_FOREVER:         // DO FOREVER

                    newDo->type = DO_FOREVER;
                    // this has a potential conditional
                    OrefSet(newDo, newDo->conditional, this->parseConditional(&conditional, Error_Invalid_do_forever));
                    // if we have a conditional, then we need to adjust the loop type
                    if (conditional == SUBKEY_WHILE)
                    {
                        newDo->type = DO_WHILE;  // DO FOREVER WHILE cond
                    }
                    else if (conditional == SUBKEY_UNTIL)
                    {
                        newDo->type = DO_UNTIL;  // DO FOREVER UNTIL cond
                    }
                    break;

                case SUBKEY_WHILE:           // DO WHILE cond                     */
                    // step back one token and process the conditional
                    previousToken();
                    OrefSet(newDo, newDo->conditional, this->parseConditional(&conditional, 0));
                    newDo->type = DO_WHILE;    // set the proper loop type          */
                    break;

                case SUBKEY_UNTIL:           // DO WHILE cond                     */
                    // step back one token and process the conditional
                    previousToken();
                    OrefSet(newDo, newDo->conditional, this->parseConditional(&conditional, 0));
                    newDo->type = DO_UNTIL;    // set the proper loop type          */
                    break;

                default:                       /* not a real DO keyword...expression*/
                    previousToken();           /* step back one token               */
                    newDo->type = DO_COUNT;    /* just a DO count form              */
                                               /* get the repetitor expression      */
                    OrefSet(newDo, newDo->forcount, this->expression(TERM_COND));
                    // this has a potential conditional
                    OrefSet(newDo, newDo->conditional, this->parseConditional(&conditional, 0));
                    // if we have a conditional, then we need to adjust the loop type
                    if (conditional == SUBKEY_WHILE)
                    {
                        newDo->type = DO_COUNT_WHILE;  // DO expr WHILE cond
                    }
                    else if (conditional == SUBKEY_UNTIL)
                    {
                        newDo->type = DO_COUNT_UNTIL;  // DO expr UNTIL cond
                    }
                    break;
            }
        }
    }
    else // just a DO expr form
    {
        previousToken();           /* step back one token               */
        newDo->type = DO_COUNT;    /* just a DO count form              */
                                   /* get the repetitor expression      */
        OrefSet(newDo, newDo->forcount, this->expression(TERM_COND));
        // this has a potential conditional
        OrefSet(newDo, newDo->conditional, this->parseConditional(&conditional, 0));
        // if we have a conditional, then we need to adjust the loop type
        if (conditional == SUBKEY_WHILE)
        {
            newDo->type = DO_COUNT_WHILE;  // DO expr WHILE cond
        }
        else if (conditional == SUBKEY_UNTIL)
        {
            newDo->type = DO_COUNT_UNTIL;  // DO expr UNTIL cond
        }
    }
    return newDo;
}

RexxInstruction *RexxSource::dropNew()
/****************************************************************************/
/* Function:  Create a new DROP translator object                           */
/****************************************************************************/
{
    /* go process the list               */
    size_t variableCount = this->processVariableList(KEYWORD_DROP);
    /* create a new translator object    */
    RexxInstruction *newObject = new_variable_instruction(DROP, Drop, sizeof(RexxInstructionDrop) + (variableCount - 1) * sizeof(RexxObject *));
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionDrop(variableCount, this->subTerms);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::elseNew(
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

RexxInstruction *RexxSource::endNew()
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
        token = nextReal();                /* get the next token                */
        if (!token->isEndOfClause())   /* end of the instruction?           */
        {
            /* this is an error                  */
            syntaxError(Error_Invalid_data_end, token);
        }
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(END, End);
    /* now complete this                 */
    new((void *)newObject) RexxInstructionEnd(name);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::endIfNew(
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

RexxInstruction *RexxSource::exitNew()
/****************************************************************************/
/* Function:  Create a EXIT instruction object                              */
/****************************************************************************/
{
    /* process the expression            */
    RexxObject *_expression = this->expression(TERM_EOC);
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(EXIT, Exit);
    /* now complete this                 */
    new((void *)newObject) RexxInstructionExit(_expression);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::exposeNew()
/****************************************************************************/
/* Function:  Create a new EXPOSE translator object                         */
/****************************************************************************/
{
    this->isExposeValid();               /* validate the placement            */
                                         /* go process the list               */
    size_t variableCount = this->processVariableList(KEYWORD_EXPOSE);
    /* Get new object                    */
    RexxInstruction *newObject = new_variable_instruction(EXPOSE, Expose, sizeof(RexxInstructionExpose) + (variableCount - 1) * sizeof(RexxObject *));
    /* Initialize this new method        */
    new ((void *)newObject) RexxInstructionExpose(variableCount, this->subTerms);
    return newObject; /* done, return this                 */
}

void RexxSource::RexxInstructionForwardCreate(
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

RexxInstruction *RexxSource::forwardNew()
/****************************************************************************/
/* Function:  Create a new RAISE translator object                             */
/****************************************************************************/
{
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(FORWARD, Forward);
    new((void *)newObject) RexxInstructionForward;
    this->RexxInstructionForwardCreate((RexxInstructionForward *)newObject);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::guardNew()
/******************************************************************************/
/* Function:  Create a new GUARD translator object                            */
/******************************************************************************/
{
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

RexxInstruction *RexxSource::ifNew(
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


RexxInstruction *RexxSource::instructionNew(
     int type )                        /* instruction type                  */
/****************************************************************************/
/* Function:  Create a new INSTRUCTION translator object                    */
/****************************************************************************/
{
    /* create a new translator object    */
    RexxInstruction *newObject =  this->sourceNewObject(sizeof(RexxInstruction), TheInstructionBehaviour, KEYWORD_INSTRUCTION);
    newObject->setType(type);            /* set the given type                */
    return newObject;                    /* done, return this                 */
}

RexxInstruction *RexxSource::interpretNew()
/****************************************************************************/
/* Function:  Create a new INTERPRET instruction object                     */
/****************************************************************************/
{
    /* process the expression            */
    RexxObject *_expression = this->expression(TERM_EOC);
    if (_expression == OREF_NULL)         /* no expression here?               */
    {
        /* this is invalid                   */
        syntaxError(Error_Invalid_expression_interpret);
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(INTERPRET, Interpret);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionInterpret(_expression);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::labelNew()
/******************************************************************************/
/* Function:  Create a new LABEL instruction translator object                */
/******************************************************************************/
{
    RexxToken *token = nextToken();                 /* get the next token                */
    RexxString *name = token->value;                 /* get the label name                */
    /* allocate a new object             */
    RexxInstruction *newObject = new_instruction(LABEL, Label);
    /* add to the label list             */
    this->addLabel( newObject, name);
    token = nextReal();                  /* get the colon token               */

    SourceLocation location = token->getLocation();     /* get the token location            */
    /* the clause ends with the colon    */
    newObject->setEnd(location.getEndLine(), location.getEndOffset());
    new ((void *)newObject) RexxInstructionLabel();
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::leaveNew(
     int type )                        /* type of queueing operation        */
/****************************************************************************/
/* Function:  Create a new LEAVE/ITERATE instruction translator object      */
/****************************************************************************/
{
    RexxString *name = OREF_NULL;                    /* no name yet                       */
    RexxToken *token = nextReal();                  /* get the next token                */
    if (!token->isEndOfClause())
    {   /* have a name specified?            */
        /* must have a symbol here           */
        if (!token->isSymbol())
        {
            if (type == KEYWORD_LEAVE)       /* is it a LEAVE?                    */
            {
                /* this is an error                  */
                syntaxError(Error_Symbol_expected_leave);
            }
            else
            {
                /* this is an iterate error          */
                syntaxError(Error_Symbol_expected_iterate);
            }
        }
        name = token->value;               /* get the name pointer              */
        token = nextReal();                /* get the next token                */
        if (!token->isEndOfClause())
        { /* end of the instruction?           */
            if (type == KEYWORD_LEAVE)       /* is it a LEAVE?                    */
            {
                /* this is an error                  */
                syntaxError(Error_Invalid_data_leave, token);
            }
            else
            {
                /* this is an iterate error          */
                syntaxError(Error_Invalid_data_iterate, token);
            }
        }
    }
    /* allocate a new object             */
    RexxInstruction *newObject = new_instruction(LEAVE, Leave);
    /* Initialize this new method        */
    new ((void *)newObject) RexxInstructionLeave(type, name);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::messageNew(
  RexxExpressionMessage *_message)      /* message to process                */
/****************************************************************************/
/* Function:  Create a new MESSAGE instruction translator object            */
/****************************************************************************/
{
    ProtectedObject p(_message);
    /* allocate a new object             */
    RexxInstruction *newObject = new_variable_instruction(MESSAGE, Message, sizeof(RexxInstructionMessage) + (_message->argumentCount - 1) * sizeof(RexxObject *));
    /* Initialize this new method        */
    new ((void *)newObject) RexxInstructionMessage(_message);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::messageAssignmentNew(
  RexxExpressionMessage *_message,      /* message to process                */
  RexxObject            *_expression )  /* assignment value                  */
/****************************************************************************/
/* Function:  Create a new MESSAGE assignment translator object             */
/****************************************************************************/
{
    ProtectedObject p(_message);        // protect this
    _message->makeAssignment(this);       // convert into an assignment message
    // allocate a new object.  NB:  a message instruction gets an extra argument, so we don't subtract one.
    RexxInstruction *newObject = new_variable_instruction(MESSAGE, Message, sizeof(RexxInstructionMessage) + (_message->argumentCount) * sizeof(RexxObject *));
    /* Initialize this new method        */
    new ((void *)newObject) RexxInstructionMessage(_message, _expression);
    return newObject; /* done, return this                 */
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
RexxInstruction *RexxSource::messageAssignmentOpNew(RexxExpressionMessage *_message, RexxToken *operation, RexxObject *_expression)
{
    ProtectedObject p(_message);        // protect this
    ProtectedObject p2(_expression);    // also need to protect this portion
    // make a copy of the message term for use in the expression
    RexxObject *retriever = _message->copy();

    _message->makeAssignment(this);       // convert into an assignment message (the original message term)


    // now add a binary operator to this expression tree
    _expression = (RexxObject *)new RexxBinaryOperator(operation->subclass, retriever, _expression);

    // allocate a new object.  NB:  a message instruction gets an extra argument, so we don't subtract one.
    RexxInstruction *newObject = new_variable_instruction(MESSAGE, Message, sizeof(RexxInstructionMessage) + (_message->argumentCount) * sizeof(RexxObject *));
    /* Initialize this new method        */
    new ((void *)newObject) RexxInstructionMessage(_message, _expression);
    return newObject; /* done, return this                 */
}


RexxInstruction *RexxSource::nopNew()
/****************************************************************************/
/* Function:  Create a NOP instruction object                               */
/****************************************************************************/
{
    RexxToken *token = nextReal();                  /* get the next token                */
    if (!token->isEndOfClause())     /* not at the end-of-clause?         */
    {
        /* bad NOP instruction               */
        syntaxError(Error_Invalid_data_nop, token);
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(NOP, Nop);
    /* dummy new to consruct VFT         */
    new ((void *)newObject) RexxInstructionNop;
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::numericNew()
/****************************************************************************/
/* Function:  Create a NUMERIC instruction object                           */
/****************************************************************************/
{
    RexxObject *_expression = OREF_NULL;              /* clear the expression              */
    size_t _flags = 0;                           /* and the flags                     */
    RexxToken *token = nextReal();                  /* get the next token                */
    if (!token->isSymbol())  /* must have a symbol here           */
    {
        /* raise the error                   */
        syntaxError(Error_Symbol_expected_numeric, token);
    }

    unsigned short type = this->subKeyword(token);      /* resolve the subkeyword            */
    switch (type)
    {                      /* process the subkeyword            */
        case SUBKEY_DIGITS:                /* NUMERIC DIGITS instruction        */
            /* process the expression            */
            _expression = this->expression(TERM_EOC);
            break;

        case SUBKEY_FORM:                  /* NUMERIC FORM instruction          */
            token = nextReal();              /* get the next token                */
            if (token->isEndOfClause()) /* just NUMERIC FORM?                */
            {
                // reset to the default for this package
                _flags |= numeric_form_default;
                break;                         /* we're all finished                */
            }
                                               /* have the keyword form?            */
            else if (token->isSymbol())
            {
                /* resolve the subkeyword            */
                /* and process                       */
                switch (this->subKeyword(token))
                {

                    case SUBKEY_SCIENTIFIC:      /* NUMERIC FORM SCIENTIFIC           */
                        token = nextReal();        /* get the next token                */
                                                   /* end of the instruction?           */
                        if (!token->isEndOfClause())
                        {
                            /* this is an error                  */
                            syntaxError(Error_Invalid_data_form, token);
                        }
                        break;

                    case SUBKEY_ENGINEERING:     /* NUMERIC FORM ENGINEERING          */
                        /* switch to engineering             */
                        _flags |= numeric_engineering;
                        token = nextReal();        /* get the next token                */
                                                   /* end of the instruction?           */
                        if (!token->isEndOfClause())
                        {
                            /* this is an error                  */
                            syntaxError(Error_Invalid_data_form, token);
                        }
                        break;

                    case SUBKEY_VALUE:           /* NUMERIC FORM VALUE expr           */
                        /* process the expression            */
                        _expression = this->expression(TERM_EOC);
                        /* no expression?                    */
                        if (_expression == OREF_NULL)
                        {
                            /* expression is required after value*/
                            syntaxError(Error_Invalid_expression_form);
                        }
                        break;

                    default:                     /* invalid subkeyword                */
                        /* raise an error                    */
                        syntaxError(Error_Invalid_subkeyword_form, token);
                        break;

                }
            }
            else
            {                           /* NUMERIC FORM expr                 */
                previousToken();               /* step back a token                 */
                                               /* process the expression            */
                _expression = this->expression(TERM_EOC);
            }
            break;

        case SUBKEY_FUZZ:                  /* NUMERIC FUZZ instruction          */
            /* process the expression            */
            _expression = this->expression(TERM_EOC);
            break;

        default:                           /* invalid subkeyword                */
            syntaxError(Error_Invalid_subkeyword_numeric, token);
            break;
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(NUMERIC, Numeric);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionNumeric(_expression, type, _flags);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::optionsNew()
/****************************************************************************/
/* Function:  Create an OPTIONS instruction object                          */
/****************************************************************************/
{
    /* process the expression            */
    RexxObject *_expression = this->expression(TERM_EOC);
    if (_expression == OREF_NULL)         /* no expression here?               */
        /* this is invalid                   */
        syntaxError(Error_Invalid_expression_options);
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(OPTIONS, Options);
    /* now complete this                 */
    new((void *)newObject) RexxInstructionOptions(_expression);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::otherwiseNew(
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

RexxInstruction *RexxSource::parseNew(
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

RexxInstruction *RexxSource::procedureNew()
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

RexxInstruction *RexxSource::queueNew(
  int type)                            /* type of queueing operation        */
/****************************************************************************/
/* Function:  Create a QUEUE instruction object                             */
/****************************************************************************/
{
    /* process the expression            */
    RexxObject *_expression = this->expression(TERM_EOC);
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(QUEUE, Queue);
    /* now complete this                 */
    new((void *)newObject) RexxInstructionQueue(_expression, type);
    return newObject;  /* done, return this                 */
}
RexxInstruction *RexxSource::raiseNew()
/****************************************************************************/
/* Function:  Create a new RAISE translator object                             */
/****************************************************************************/
{
    size_t arrayCount = SIZE_MAX;               /* clear out the temporaries         */
    RexxObject *_expression = OREF_NULL;
    RexxObject *description = OREF_NULL;
    RexxObject *additional = OREF_NULL;
    RexxObject *result = OREF_NULL;
    bool raiseReturn = false;                 /* default is exit form              */

    RexxQueue *saveQueue = new_queue();             /* get a new queue item              */
    this->saveObject(saveQueue);         /* protect it                        */
    RexxToken *token = nextReal();                  /* get the next token                */
    /* no target specified or have a     */
    if (!token->isSymbol())  /* bad token type?                   */
    {
        /* this is an error                  */
        syntaxError(Error_Symbol_expected_raise);
    }
    RexxString *_condition = token->value;           /* use the condition string value    */
    saveQueue->push(_condition);         /* save the condition name           */
    int _keyword = this->condition(token);   /* check for the subkeywords         */
    switch (_keyword)
    {                  /* process the different conditions  */

        case CONDITION_FAILURE:            /* RAISE FAILURE returncode          */
        case CONDITION_ERROR:              /* RAISE ERROR   returncode          */
        case CONDITION_SYNTAX:             /* RAISE SYNTAX  number              */
            /* go get the keyword value expr.    */
            _expression = this->constantExpression();
            if (_expression == OREF_NULL)
            {   /* no expression given?              */
                token = nextToken();           /* get the terminator token          */
                                               /* this is an invalid expression     */
                syntaxError(Error_Invalid_expression_general, token);
            }
            saveQueue->queue(_expression);   /* protect it                        */
            break;

        case CONDITION_USER:               /* CONDITION USER usercondition      */
            token = nextReal();              /* get the next token                */
                                             /* bad token type?                   */
            if (!token->isSymbol())
            {
                /* this is an error                  */
                syntaxError(Error_Symbol_expected_user);
            }
            _condition = token->value;        /* get the token string value        */
            /* condition name is "USER condition"*/
            _condition = _condition->concatToCstring(CHAR_USER_BLANK);
            /* get the common version            */
            _condition = this->commonString(_condition);
            saveQueue->queue(_condition);    /* save the condition                */
            break;

        case CONDITION_HALT:               /* CONDITION HALT                    */
        case CONDITION_NOMETHOD:           /* CONDITION NOMETHOD                */
        case CONDITION_NOSTRING:           /* CONDITION NOSTRING                */
        case CONDITION_NOTREADY:           /* CONDITION NOTREADY                */
        case CONDITION_NOVALUE:            /* CONDITION NOVALUE                 */
        case CONDITION_LOSTDIGITS:         /* CONDITION NUMERIC                 */
        case CONDITION_PROPAGATE:          /* CONDITION PROPAGATE               */
            break;                           /* this is already processed above   */

        default:                           /* invalid condition specified       */
            /* this is a sub keyword error       */
            syntaxError(Error_Invalid_subkeyword_raise, token);
            break;
    }
    token = nextReal();                  /* get the next token                */
    while (!token->isEndOfClause())
    {/* while still more to process       */
        if (!token->isSymbol())/* not a symbol token?               */
        {
            /* this is an error                  */
            syntaxError(Error_Invalid_subkeyword_raiseoption, token);
        }
        _keyword = this->subKeyword(token); /* get the keyword value             */
        switch (_keyword)
        {                /* process the subkeywords           */

            case SUBKEY_DESCRIPTION:         /* RAISE ... DESCRIPTION expr        */
                if (description != OREF_NULL)  /* have a description already?       */
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_subkeyword_description);
                }
                /* get the keyword value             */
                description = this->constantExpression();
                if (description == OREF_NULL)  /* no expression here?               */
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_expression_raise_description);
                }
                saveQueue->queue(description); /* protect this                      */
                break;

            case SUBKEY_ADDITIONAL:          /* RAISE ... ADDITIONAL expr         */
                /* have a additional already?        */
                if (additional != OREF_NULL || arrayCount != SIZE_MAX)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_subkeyword_additional);
                }
                /* get the keyword value             */
                additional = this->constantExpression();
                /* no expression here?               */
                if (additional == OREF_NULL)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_expression_raise_additional);
                }
                saveQueue->queue(additional);  /* protect this                      */
                break;

            case SUBKEY_ARRAY:               /* RAISE ... ARRAY expr              */
                /* have a additional already?        */
                if (additional != OREF_NULL || arrayCount != SIZE_MAX)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_subkeyword_additional);
                }
                token = nextReal();            /* get the next token                */
                                               /* not an expression list starter?   */
                if (token->classId != TOKEN_LEFT)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_expression_raise_list);
                }
                /* process the array list            */
                arrayCount = this->argList(token, TERM_RIGHT);
                break;

            case SUBKEY_RETURN:              /* RAISE ... RETURN expr             */
                /* have a result already?            */
                if (result != OREF_NULL)
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_subkeyword_result);
                }
                raiseReturn = true;            /* remember this                     */
                                               /* get the keyword value             */
                result = this->constantExpression();
                if (result != OREF_NULL)       /* actually have one?                */
                {
                    saveQueue->queue(result);    /* protect this                      */
                }
                break;

            case SUBKEY_EXIT:                /* RAISE ... EXIT expr               */
                if (result != OREF_NULL)       /* have a result already?            */
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_subkeyword_result);
                }
                /* get the keyword value             */
                result = this->constantExpression();
                if (result != OREF_NULL)       /* actually have one?                */
                {
                    saveQueue->queue(result);    /* protect this                      */
                }
                break;

            default:                         /* invalid subkeyword                */
                /* this is a sub keyword error       */
                syntaxError(Error_Invalid_subkeyword_raiseoption, token);
                break;
        }
        token = nextReal();                /* step to the next keyword          */
    }

    RexxInstruction *newObject;

    if (arrayCount != SIZE_MAX)          /* have the array version?           */
    {
        /* create a new translator object    */
        newObject = new_variable_instruction(RAISE, Raise, sizeof(RexxInstructionRaise) + (arrayCount - 1) * sizeof(RexxObject *));
    }
    else                                 /* static instruction size           */
    {
        newObject = new_instruction(RAISE, Raise);
    }
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionRaise(_condition, _expression, description, additional, result, arrayCount, this->subTerms, raiseReturn);
    this->toss(saveQueue);               /* release the GC lock               */
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::replyNew()
/****************************************************************************/
/* Function:  Create a REPLY instruction object                             */
/****************************************************************************/
{
    RexxObject *_expression = this->expression(TERM_EOC);
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(REPLY, Reply);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionReply(_expression);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::returnNew()
/****************************************************************************/
/* Function:  Create a RETURN instruction object                            */
/****************************************************************************/
{
    /* process the expression            */
    RexxObject *_expression = this->expression(TERM_EOC);
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(RETURN, Return);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionReturn(_expression);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::sayNew()
/****************************************************************************/
/* Function:  Create a SAY instruction object                               */
/****************************************************************************/
{
    RexxObject *_expression = this->expression(TERM_EOC);
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(SAY, Say);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionSay(_expression);
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::selectNew()
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
    token = nextReal();
    // this must be the end of the clause
    if (!token->isEndOfClause())
    {
        syntaxError(Error_Invalid_data_select, token);
    }

    // ok, finally allocate this and return
    RexxInstruction *newObject = new_instruction(SELECT, Select);
    new ((void *)newObject) RexxInstructionSelect(label);
    return  newObject;
}

RexxInstruction *RexxSource::signalNew()
/****************************************************************************/
/* Function:  Create a SIGNAL instruction object                            */
/****************************************************************************/
{
    bool signalOff = false;                   /* not a SIGNAL OFF instruction      */
    RexxObject *_expression = OREF_NULL;              /* no expression yet                 */
    RexxString *name = OREF_NULL;                    /* no name                           */
    RexxString *_condition = OREF_NULL;               /* and no condition                  */
    size_t _flags = 0;                           /* no flags                          */
    RexxToken *token = nextReal();                  /* get the next token                */

    if (token->isEndOfClause())     /* no target specified?              */
    {
        /* this is an error                  */
        syntaxError(Error_Symbol_or_string_signal);
    }
    /* implicit value form?              */
    else if (!token->isSymbolOrLiteral())
    {
        previousToken();                   /* step back a token                 */
                                           /* go process the expression         */
        _expression = this->expression(TERM_EOC);
    }
    else
    {                               /* have a real target                */
                                    /* may have to process ON/OFF forms  */
        if (token->isSymbol())
        {
            /* check for the subkeywords         */
            int _keyword = this->subKeyword(token);
            if (_keyword == SUBKEY_ON)
            {      /* SIGNAL ON instruction?            */
                _flags |= signal_on;            /* this is a SIGNAL ON               */
                token = nextReal();            /* get the next token                */
                                               /* no condition specified or not a   */
                                               /* symbol?                           */
                if (!token->isSymbol())
                {
                    /* this is an error                  */
                    syntaxError(Error_Symbol_expected_on);
                }
                /* get the condition involved        */
                _keyword = this->condition(token);
                /* invalid condition specified?      */
                if (_keyword == 0 || _keyword == CONDITION_PROPAGATE)
                {
                    /* got an error here                 */
                    syntaxError(Error_Invalid_subkeyword_signalon, token);
                }
                /* actually a USER condition request?*/
                else if (_keyword == CONDITION_USER)
                {
                    token = nextReal();          /* get the next token                */
                                                 /* no condition specified or not a   */
                                                 /* symbol?                           */
                    if (!token->isSymbol())
                    {
                        /* this is an error                  */
                        syntaxError(Error_Symbol_expected_user);
                    }
                    name = token->value;         /* set the default target            */
                                                 /* condition name is "USER condition"*/
                    _condition = name->concatToCstring(CHAR_USER_BLANK);
                    /* save the condition name           */
                    _condition = this->commonString(_condition);
                }
                else
                {                         /* language defined condition        */
                    name = token->value;         /* set the default target            */
                    _condition = name;           /* condition is the same as target   */
                }
                token = nextReal();            /* get the next token                */
                                               /* anything real here?               */
                if (!token->isEndOfClause())
                {
                    /* not a symbol?                     */
                    if (!token->isSymbol())
                    {
                        /* this is an error                  */
                        syntaxError(Error_Invalid_subkeyword_signalonname, token);
                    }
                    /* not the name token?               */
                    if (this->subKeyword(token) != SUBKEY_NAME)
                    {
                        /* got an error here                 */
                        syntaxError(Error_Invalid_subkeyword_signalonname, token);
                    }
                    token = nextReal();          /* get the next token                */
                                                 /* not got a symbol here?            */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* this is an error                  */
                        syntaxError(Error_Symbol_or_string_name);
                    }
                    name = token->value;         /* set the new target location       */
                    token = nextReal();          /* get the next token                */
                                                 /* must have the clause end here     */
                    if (!token->isEndOfClause())
                    {
                        /* this is an error                  */
                        syntaxError(Error_Invalid_data_name, token);
                    }
                }
            }
            else if (_keyword == SUBKEY_OFF)
            {/* SIGNAL OFF instruction?           */
                signalOff = true;              /* doing a SIGNAL OFF                */
                token = nextReal();            /* get the next token                */
                                               /* no condition specified or not a   */
                                               /* symbol?                           */
                if (!token->isSymbol())
                {
                    /* this is an error                  */
                    syntaxError(Error_Symbol_expected_off);
                }
                /* get the condition involved        */
                _keyword = this->condition(token);
                /* invalid condition specified?      */
                if (_keyword == 0 || _keyword == CONDITION_PROPAGATE)
                {
                    /* got an error here                 */
                    syntaxError(Error_Invalid_subkeyword_signaloff, token);
                }
                /* actually a USER condition request?*/
                else if (_keyword == CONDITION_USER)
                {
                    token = nextReal();          /* get the next token                */
                                                 /* no condition specified or not a   */
                                                 /* symbol?                           */
                    if (!token->isSymbol())
                    {
                        /* this is an error                  */
                        syntaxError(Error_Symbol_expected_user);
                    }
                    _condition = token->value;    /* get the token string value        */
                    /* condition name is "USER condition"*/
                    _condition = _condition->concatToCstring(CHAR_USER_BLANK);
                    /* save the condition name           */
                    _condition = this->commonString(_condition);
                }
                else                           /* language defined condition        */
                {
                    _condition = token->value;    /* set the condition name            */
                }
                token = nextReal();            /* get the next token                */
                                               /* must have the clause end here     */
                if (!token->isEndOfClause())
                {
                    /* this is an error                  */
                    syntaxError(Error_Invalid_data_condition, token);
                }
            }
            /* is this the value keyword form?   */
            else if (_keyword == SUBKEY_VALUE)
            {
                /* get the expression                */
                _expression = this->expression(TERM_EOC);
                if (_expression == OREF_NULL)   /* no expression here?               */
                {
                    /* this is invalid                   */
                    syntaxError(Error_Invalid_expression_signal);
                }
            }
            else
            {                           /* normal SIGNAL instruction         */
                name = token->value;           /* set the signal target             */
                token = nextReal();            /* step to the next token            */
                                               /* not the end?                      */
                if (!token->isEndOfClause())
                {
                    /* have an error                     */
                    syntaxError(Error_Invalid_data_signal, token);
                }
            }
        }
        else
        {                             /* signal with a string target       */
            name = token->value;             /* set the signal target             */
            token = nextReal();              /* step to the next token            */
            if (!token->isEndOfClause()) /* not the end?                      */
            {
                /* have an error                     */
                syntaxError(Error_Invalid_data_signal, token);
            }
        }
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(SIGNAL, Signal);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionSignal(_expression, _condition, name, _flags);
    if (!signalOff)                      /* need to resolve later?            */
    {
        this->addReference((RexxObject *)newObject);     /* add to table of references        */
    }
    return newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::thenNew(
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

RexxInstruction *RexxSource::traceNew()
/****************************************************************************/
/* Function:  Create a TRACE instruction object                             */
/****************************************************************************/
{
    size_t setting = TRACE_NORMAL;              /* set default trace mode            */
    wholenumber_t debug_skip = 0;               /* no skipping                       */
    size_t traceFlags = 0;                      /* no translated flags               */
    RexxObject *_expression = OREF_NULL;        /* not expression form               */
    RexxToken *token = nextReal();              /* get the next token                */

    if (!token->isEndOfClause())
    {   /* more than just TRACE?             */
        /* is this a symbol?                 */
        if (token->isSymbol())
        {
            /* TRACE VALUE expr?                 */
            if (this->subKeyword(token) == SUBKEY_VALUE)
            {
                /* process the expression            */
                _expression = this->expression(TERM_EOC);
                if (_expression == OREF_NULL)   /* no expression?                    */
                {
                    /* expression is required after value*/
                    syntaxError(Error_Invalid_expression_trace);
                }
            }
            else
            {                           /* must have a symbol here           */
                RexxString *value = token->value;          /* get the string value              */
                token = nextReal();            /* get the next token                */
                                               /* end of the instruction?           */
                if (!token->isEndOfClause())
                {
                    /* this is an error                  */
                    syntaxError(Error_Invalid_data_trace, token);
                }
                if (!value->requestNumber(debug_skip, number_digits()))
                {
                    debug_skip = 0;              /* belt and braces                   */
                    char badOption = 0;
                                                 /* process the setting               */
                    if (!parseTraceSetting(value, setting, traceFlags, badOption))
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
        else if (token->isLiteral())
        {     /* is this a string?                 */
            RexxString *value = token->value;            /* get the string value              */
            token = nextReal();              /* get the next token                */
            if (!token->isEndOfClause()) /* end of the instruction?           */
            {
                /* this is an error                  */
                syntaxError(Error_Invalid_data_trace, token);
            }
            if (!value->requestNumber(debug_skip, number_digits()))
            {
                debug_skip = 0;                /* belt and braces                   */
                char badOption = 0;
                                             /* process the setting               */
                if (!parseTraceSetting(value, setting, traceFlags, badOption))
                {
                    syntaxError(Error_Invalid_trace_trace, new_string(&badOption, 1));
                }
            }
            else
            {
                setting = 0;                   /* not a normal setting situation    */
            }
        }
        /* is this a minus sign?             */
        else if (token->subclass == OPERATOR_SUBTRACT || token->subclass == OPERATOR_PLUS)
        {
            /* minus form?                       */
            if (token->subclass == OPERATOR_SUBTRACT)
            {
                setting |= DEBUG_NOTRACE;      // turn on the no tracing flag
            }
            setting = 0;                     /* indicate a debug version          */
            token = nextReal();              /* get the next token                */
            if (token->isEndOfClause()) /* end of the instruction?           */
            {
                /* this is an error                  */
                syntaxError(Error_Invalid_expression_general, token);
            }
            RexxString *value = token->value;            /* get the string value              */
            token = nextReal();              /* get the next token                */

            if (!token->isEndOfClause())     /* end of the instruction?           */
            {
                /* this is an error                  */
                syntaxError(Error_Invalid_data_trace);
            }
            if (!value->requestNumber(debug_skip, number_digits()))
            {
                /* have an error                     */
                syntaxError(Error_Invalid_whole_number_trace, value);
            }
        }
        else
        {                             /* implicit value form               */
            previousToken();                 /* step back a token                 */
                                             /* process the expression            */
            _expression = this->expression(TERM_EOC);
        }
    }
    /* create a new translator object    */
    RexxInstruction *newObject = new_instruction(TRACE, Trace);
    /* now complete this                 */
    new ((void *)newObject) RexxInstructionTrace(_expression, setting, traceFlags, debug_skip);
    return newObject; /* done, return this                 */
}

/**
 * Parse a USE STRICT ARG instruction.
 *
 * @return The executable instruction object.
 */
RexxInstruction *RexxSource::useNew()
{
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
    saveObject(variable_list);
    RexxQueue *defaults_list = new_queue();
    saveObject(defaults_list);
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
                // we need a more defaults marker
                defaults_list->push(OREF_NULL);
            }
        }
    }

    RexxInstruction *newObject = new_variable_instruction(USE, Use, sizeof(RexxInstructionUseStrict) + (variableCount == 0 ? 0 : (variableCount - 1)) * sizeof(UseVariable));
    new ((void *)newObject) RexxInstructionUseStrict(variableCount, strictChecking, allowOptionals, variable_list, defaults_list);

    // release the object locks and return;
    removeObj(variable_list);
    removeObj(defaults_list);
    return newObject;
}

