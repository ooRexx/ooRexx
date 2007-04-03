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
/* REXX Kernel                                                  otpinstr.c    */
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
#include "UseInstruction.hpp"

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

RexxInstruction *RexxSource::addressNew()
/****************************************************************************/
/* Function:  Create a new ADDRESS translator object                        */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  RexxObject *expression;              /* expression to evaluate            */
  RexxString *environment;             /* environment name                  */
  RexxObject *command;                 /* command to issue                  */
  RexxToken  *token;                   /* current working token             */

  expression = OREF_NULL;              /* initialize import state variables */
  environment = OREF_NULL;
  command = OREF_NULL;
  token = nextReal();                  /* get the next token                */
  if (token->classId != TOKEN_EOC) {   /* something other than a toggle?    */
                                       /* something other than a symbol or  */
                                       /* string?...implicit value form     */
    if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL) {
      previousToken();                 /* back up one token                 */
                                       /* process the expression            */
      expression = this->expression(TERM_EOC);
    }
    else {                             /* have a constant address target    */
                                       /* may be value keyword, however...  */
      if (token->classId == TOKEN_SYMBOL && this->subKeyword(token) == SUBKEY_VALUE) {
                                       /* process the expression            */
        expression = this->expression(TERM_EOC);
        if (expression == OREF_NULL)   /* no expression?                    */
                                       /* expression is required after value*/
          report_error(Error_Invalid_expression_address);
      }
      else {                           /* have a real constant target       */
        environment = token->value;    /* get the actual name value         */
        token = nextReal();            /* get the next token                */
                                       /* have a command following name?    */
        if (token->classId != TOKEN_EOC) {
          previousToken();             /* step back a token                 */
                                       /* process the expression            */
          command = this->expression(TERM_EOC);
        }
      }
    }                                  /* resolve the subkeyword            */
  }
                                       /* create a new translator object    */
  newObject = new_instruction(ADDRESS, Address);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionAddress(expression, environment, command);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::assignmentNew(
     RexxToken  *target )              /* target variable instruction       */
/****************************************************************************/
/* Function:  Create a new ASSIGNMENT translator object                     */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  RexxObject *expression;              /* expression to evaluate            */

  this->needVariable(target);          /* must be a variable                */
                                       /* process the expression            */
  expression = this->expression(TERM_EOC);
  if (expression == OREF_NULL)         /* no expression here?               */
                                       /* this is invalid                   */
    report_error(Error_Invalid_expression_assign);
                                       /* create a new translator object    */
  newObject = new_instruction(ASSIGNMENT, Assignment);
  new ((void *)newObject) RexxInstructionAssignment((RexxVariableBase *)(this->addText(target)), expression);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::callNew()
/****************************************************************************/
/* Function:  Create a new CALL translator object                           */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  LONG        argCount;                /* call arguments                    */
  RexxToken  *token;                   /* current working token             */
  RexxString *name;                    /* call name                         */
  INT         keyword;                 /* call subkeyword                   */
  RexxString *condition;               /* created USER condition            */
  CHAR        flags;                   /* final CALL flags                  */
  CHAR        builtin_index;           /* builtin function call index       */

  flags = 0;                           /* clear the flags                   */
  builtin_index = 0;                   /* clear the builtin index           */
  condition = OREF_NULL;               /* clear the condition               */
  name = OREF_NULL;                    /* no name yet                       */
  argCount = 0;                        /* no arguments yet                  */

  token = nextReal();                  /* get the next token                */
  if (token->classId == TOKEN_EOC)     /* no target specified?              */
                                       /* this is an error                  */
    report_error(Error_Symbol_or_string_call);
                                       /* may have to process ON/OFF forms  */
  else if (token->classId == TOKEN_SYMBOL) {
    keyword = this->subKeyword(token); /* check for the subkeywords         */
    if (keyword == SUBKEY_ON) {        /* CALL ON instruction?              */
      flags |= call_on_off;            /* this is a CALL ON                 */
      token = nextReal();              /* get the next token                */
                                       /* no condition specified or not a   */
                                       /* symbol?                           */
      if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
        report_error(Error_Symbol_expected_on);
      keyword = this->condition(token);/* get the condition involved        */
      if (keyword == 0 ||              /* invalid condition specified?      */
          keyword == CONDITION_SYNTAX ||
          keyword == CONDITION_NOVALUE ||
          keyword == CONDITION_PROPAGATE ||
          keyword == CONDITION_LOSTDIGITS ||
          keyword == CONDITION_NOMETHOD ||
          keyword == CONDITION_NOSTRING)
                                       /* got an error here                 */
        report_error_token(Error_Invalid_subkeyword_callon, token);

                                       /* actually a USER condition request?*/
      else if (keyword == CONDITION_USER) {
        token = nextReal();            /* get the next token                */
                                       /* no condition specified or not a   */
                                       /* symbol?                           */
        if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
          report_error(Error_Symbol_expected_user);
                                       /* set the builtin index for later   */
                                       /* resolution step                   */
        builtin_index = this->builtin(token);
        condition = token->value;      /* get the token string value        */
        name = condition;              /* set the default target            */
                                       /* condition name is "USER condition"*/
        condition = condition->concatToCstring(CHAR_USER_BLANK);
                                       /* save the condition name           */
        condition = this->commonString(condition);
      }
      else {                           /* language defined condition        */
        name = token->value;           /* set the default target            */
                                       /* set the builtin index for later   */
                                       /* resolution step                   */
        builtin_index = this->builtin(token);
        condition = name;              /* condition is the same as target   */
      }
      token = nextReal();              /* get the next token                */
                                       /* anything real here?               */
      if (token->classId != TOKEN_EOC) {
                                       /* not a symbol?                     */
        if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
          report_error_token(Error_Invalid_subkeyword_callonname, token);
                                       /* not the name token?               */
        if (this->subKeyword(token) != SUBKEY_NAME)
                                       /* got an error here                 */
          report_error_token(Error_Invalid_subkeyword_callonname, token);
        token = nextReal();            /* get the next token                */
        if (token->classId != TOKEN_SYMBOL &&
            token->classId != TOKEN_LITERAL)
                                       /* this is an error                  */
          report_error(Error_Symbol_or_string_name);
        name = token->value;           /* set the default target            */
                                       /* set the builtin index for later   */
                                       /* resolution step                   */
        builtin_index = this->builtin(token);
        token = nextReal();            /* get the next token                */
                                       /* must have the clause end here     */
        if (token->classId != TOKEN_EOC)
                                       /* this is an error                  */
          report_error_token(Error_Invalid_data_name, token);
      }
    }
    else if (keyword == SUBKEY_OFF) {  /* CALL OFF instruction?             */
      token = nextReal();              /* get the next token                */
                                       /* no condition specified or not a   */
                                       /* symbol?                           */
      if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
        report_error(Error_Symbol_expected_off);
                                       /* get the condition involved        */
      keyword = this->condition(token);
      if (keyword == 0 ||              /* invalid condition specified?      */
          keyword == CONDITION_SYNTAX ||
          keyword == CONDITION_NOVALUE ||
          keyword == CONDITION_PROPAGATE ||
          keyword == CONDITION_LOSTDIGITS ||
          keyword == CONDITION_NOMETHOD ||
          keyword == CONDITION_NOSTRING)
                                       /* got an error here                 */
        report_error_token(Error_Invalid_subkeyword_calloff, token);
                                       /* actually a USER condition request?*/
      else if (keyword == CONDITION_USER) {
        token = nextReal();            /* get the next token                */
                                       /* no condition specified or not a   */
                                       /* symbol?                           */
        if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
          report_error(Error_Symbol_expected_user);
        condition = token->value;      /* get the token string value        */
                                       /* condition name is "USER condition"*/
        condition = condition->concatToCstring(CHAR_USER_BLANK);
                                       /* save the condition name           */
        condition = this->commonString(condition);
      }
      else                             /* language defined condition        */
        condition = token->value;      /* set the condition name            */
      token = nextReal();              /* get the next token                */
      if (token->classId != TOKEN_EOC) /* must have the clause end here     */
                                       /* this is an error                  */
        report_error_token(Error_Invalid_data_condition, token);
    }
    else {                             /* normal CALL instruction           */
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
  else if (token->classId == TOKEN_LITERAL) {
    name = token->value;               /* set the default target            */
                                       /* set the builtin index for later   */
                                       /* resolution step                   */
    builtin_index = this->builtin(token);
                                       /* process the argument list         */
    argCount = this->argList(OREF_NULL, TERM_EOC);
    flags |= call_nointernal;          /* do not check for internal routines*/
  }
                                       /* indirect call case?               */
  else if (token->classId == TOKEN_LEFT) {
    flags |= call_dynamic;             /* going to be indirect              */
    token = nextReal();                /* step to the next token            */
    if (token->classId != TOKEN_SYMBOL)/* not a symbol?                     */
                                       /* error                             */
      report_error(Error_Symbol_expected_varref);
    this->needVariable(token);         /* need a variable token here        */
                                       /* get a variable retriever          */
    name = (RexxString *)this->addText(token);
    token = nextReal();                /* step to next real token           */
                                       /* must be a right paren here        */
    if (token->classId != TOKEN_RIGHT)
                                       /* this is an error                  */
      report_error_token(Error_Variable_reference_extra, token);
                                       /* process the argument list         */
    argCount = this->argList(OREF_NULL, TERM_EOC);
                                       /* NOTE:  this call is not added to  */
                                       /* the resolution list because it    */
                                       /* cannot be resolved until run time */
  }
  else
                                       /* this is an error                  */
    report_error(Error_Symbol_or_string_call);
                                       /* create a new translator object    */
  newObject = new_variable_instruction(CALL, Call, sizeof(RexxInstructionCallBase) + argCount * sizeof(OREF));
                                       /* Initialize this new object        */
  new ((void *)newObject) RexxInstructionCall(name, condition, argCount, this->subTerms, flags, builtin_index);
  if (!(flags&call_dynamic))           /* static name resolution?           */
    this->addReference(newObject);     /* add to table of references        */
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::commandNew()
/****************************************************************************/
/* Function:  Create a new COMMAND instruction object                       */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  RexxObject *expression;              /* expression to evaluate            */

                                       /* process the expression            */
  expression = this->expression(TERM_EOC);
                                       /* create a new translator object    */
  newObject = new_instruction(COMMAND, Command);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionCommand((RexxObject *) expression);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::doNew()
/****************************************************************************/
/* Function:  Create a new DO translator object                             */
/****************************************************************************/
{
  RexxObject   *newObject;             /* newly created object              */

  newObject = new_instruction(DO, Do); /* create a new translator object    */
  new((void *)newObject) RexxInstructionDo;
                                       /* now complete the parsing          */
  this->RexxInstructionDoCreate((RexxInstructionDo *)newObject);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

void RexxSource::RexxInstructionDoCreate(RexxInstructionDo *newDo)
/******************************************************************************/
/* Function:  Create a new DO translator object                               */
/******************************************************************************/
{
  RexxInstructionDo *newObject;        /* newly created object              */
  RexxToken         *token;            /* current working token             */
  INT                keyslot;          /* keyword ordering counter          */
  RexxToken         *second;           /* second token                      */
  INT                terminators;      /* set of expression terminators     */
  INT                conditional;      /* conditional type                  */

  token = nextReal();                  /* get the next token                */
  newObject = newDo;                   /* get addr of new object            */
  if (token->classId == TOKEN_EOC)     /* this just a simple DO?            */
    newDo->type = SIMPLE_DO;           /* this is just a simple DO          */
  else {                               /* have some kind of DO loop         */
                                       /* have a symbol?                    */
    if (token->classId == TOKEN_SYMBOL) {
      second = nextReal();             /* need the second token             */
                                       /* have the '==' form?               */
      if (second->subclass == OPERATOR_STRICT_EQUAL)
                                       /* this is an invalid expression     */
        report_error_token(Error_Invalid_expression_general, token);
                                       /* have a control variable form?     */
      else if (second->subclass == OPERATOR_EQUAL) {
        keyslot = 0;                   /* starting with the first slot      */
        newDo->type = CONTROLLED_DO;   /* this is a controlled DO loop      */
        this->needVariable(token);     /* enforce variable form             */
                                       /* save the control variable name    */
        OrefSet(newObject, newDo->name, token->value);
                                       /* add to variable symbol table      */
                                       /* and save the control information  */
        OrefSet(newObject, newDo->control, (RexxVariableBase *)this->addText(token));
        terminators = TERM_CONTROL;    /* set the appropriate terminator set*/
                                       /* get initial value expression      */
        OrefSet(newObject, newDo->initial, this->expression(terminators));
                                       /* no expression given?              */
        if (newDo->initial == OREF_NULL)
                                       /* this is a syntax error            */
          report_error(Error_Invalid_expression_control);
                                       /* while we don't have a clause      */
                                       /* terminator                        */
        token = nextReal();            /* get the terminator token          */
        while (token->classId != TOKEN_EOC) {
                                       /* process the keyword type          */
          switch (this->subKeyword(token) ) {

            case SUBKEY_BY:            /* BY exprb                          */
                                       /* already had one?                  */
              if (newDo->by != OREF_NULL)
                                       /* another invalid DO                */
                report_error_token(Error_Invalid_do_duplicate, token);
                                       /* get next subexpression            */
              OrefSet(newObject, newDo->by, this->expression(terminators));
                                       /* nothing really there?             */
              if (newDo->by == OREF_NULL)
                                       /* another invalid DO                */
                report_error(Error_Invalid_expression_by);
                                       /* remember the evaluation order     */
              newDo->expressions[keyslot] = EXP_BY;
              keyslot++;               /* step the key slot                 */
              break;

            case SUBKEY_TO:            /* TO exprt                          */
                                       /* already had one?                  */
              if (newDo->to != OREF_NULL)
                                       /* another invalid DO                */
                report_error_token(Error_Invalid_do_duplicate, token);
                                       /* get next subexpression            */
              OrefSet(newObject, newDo->to, this->expression(terminators));
                                       /* nothing really there?             */
              if (newDo->to == OREF_NULL)
                                       /* another invalid DO                */
                report_error(Error_Invalid_expression_to);
                                       /* remember the evaluation order     */
              newDo->expressions[keyslot] = EXP_TO;
              keyslot++;               /* step the key slot                 */
              break;

            case SUBKEY_FOR:           /* FOR exprf                         */
                                       /* already had one?                  */
              if (newDo->forcount != OREF_NULL)
                                       /* another invalid DO                */
                report_error_token(Error_Invalid_do_duplicate, token);
                                       /* get next subexpression            */
              OrefSet(newObject, newDo->forcount, this->expression(terminators));
                                       /* nothing really there?             */
              if (newDo->forcount == OREF_NULL)
                                       /* another invalid DO                */
                report_error(Error_Invalid_expression_for);
                                       /* remember the evaluation order     */
              newDo->expressions[keyslot] = EXP_FOR;
              keyslot++;               /* step the key slot                 */
              break;

            case SUBKEY_WHILE:         /* WHILE exprw                       */
              previousToken();         /* step back one token               */
                                       /* go process the conditional        */
              OrefSet(newObject, newDo->conditional, this->parseConditional(NULL, 0));
              previousToken();         /* place the terminator back         */
                                       /* this changes the loop type        */
              newDo->type = CONTROLLED_WHILE;
              break;

            case SUBKEY_UNTIL:         /* UNTIL expru                       */
              previousToken();         /* step back one token               */
                                       /* go process the conditional        */
              OrefSet(newObject, newDo->conditional, this->parseConditional(NULL, 0));
              previousToken();         /* place the terminator back         */
                                       /* this changes the loop type        */
              newDo->type = CONTROLLED_UNTIL;
              break;
          }
          token = nextReal();          /* step to the next token            */
        }
      }
                                       /* DO name OVER collection form?     */
      else if (this->subKeyword(second) == SUBKEY_OVER) {
        newDo->type = DO_OVER;         /* this is a DO OVER loop            */
        this->needVariable(token);     /* must have a control variable      */
                                       /* save the control variable name    */
        OrefSet(newObject, newDo->name, token->value);
                                       /* add to variable symbol table      */
                                       /* and save the control information  */
        OrefSet(newObject, newDo->control, (RexxVariableBase *)this->addText(token));
                                       /* and save the control information  */
        OrefSet(newObject, newDo->initial, this->expression(TERM_COND));
                                       /* no expression?                    */
        if (newDo->initial == OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_expression_over);
                                       /* go process the conditional        */
        OrefSet(newObject, newDo->conditional, this->parseConditional(&conditional, 0));
                                       /* have a DO name OVER expr WHILE?   */
        if (conditional == SUBKEY_WHILE)
          newDo->type = DO_OVER_WHILE;  /* this is the WHILE form            */
                                       /* have a DO name OVER expr UNTIL?   */
        else if (conditional == SUBKEY_UNTIL)
          newDo->type = DO_OVER_UNTIL;  /* this is the UNTIL form            */
      }
      else {                           /* not a controlled form             */
        firstToken();                  /* restart the process               */
        token = nextToken();           /* get the first token (the DO)      */
        token = nextReal();            /* now the keyword token             */
                                       /* process other keyword forms       */
        switch (this->subKeyword(token)) {

          case SUBKEY_FOREVER:         /* DO FOREVER                        */

            newDo->type = DO_FOREVER;  /* got a DO FOREVER                  */
                                       /* go process the conditional        */
            OrefSet(newObject, newDo->conditional, this->parseConditional(&conditional, Error_Invalid_do_forever));
                                       /* have a DO name OVER expr WHILE?   */
            if (conditional == SUBKEY_WHILE)
              newDo->type = DO_WHILE;  /* this is the WHILE form            */
                                       /* have a DO name OVER expr UNTIL?   */
            else if (conditional == SUBKEY_UNTIL)
              newDo->type = DO_UNTIL;  /* this is the UNTIL form            */
            break;


          case SUBKEY_WHILE:           /* WHILE exprw                       */
            previousToken();           /* step back one token               */
                                       /* go process the conditional        */
            OrefSet(newObject, newDo->conditional, this->parseConditional(&conditional, 0));
                                       /* this changes the loop type        */
            newDo->type = DO_WHILE;    /* this is the WHILE form            */
            break;

          case SUBKEY_UNTIL:           /* UNTIL expru                       */
            previousToken();           /* step back one token               */
                                       /* go process the conditional        */
            OrefSet(newObject, newDo->conditional, this->parseConditional(&conditional, 0));
                                       /* this changes the loop type        */
            newDo->type = DO_UNTIL;    /* this is the UNTIL form            */
            break;

          default:                     /* not a real DO keyword...expression*/
            previousToken();           /* step back one token               */
            newDo->type = DO_COUNT;    /* just a DO count form              */
                                       /* get the repetitor expression      */
            OrefSet(newObject, newDo->forcount, this->expression(TERM_COND));
                                       /* go process the conditional        */
            OrefSet(newObject, newDo->conditional, this->parseConditional(&conditional, 0));
                                       /* have a DO count WHILE expr?       */
            if (conditional == SUBKEY_WHILE)
                                       /* this is the WHILE form            */
              newDo->type = DO_COUNT_WHILE;
                                       /* have a DO count UNTIL expr?       */
            else if (conditional == SUBKEY_UNTIL)
                                       /* this is the UNTIL form            */
              newDo->type = DO_COUNT_UNTIL;
            break;
        }
      }
    }
    else {                             /* just DO expr form                 */
                                       /* just a DO count form              */
      newDo->type = DO_COUNT;
      previousToken();                 /* step back one token               */
                                       /* get the repetitor expression      */
      OrefSet(newObject, newDo->forcount, this->expression(TERM_COND));
                                       /* go process the conditional        */
      OrefSet(newObject, newDo->conditional, this->parseConditional(&conditional, 0));
      if (conditional == SUBKEY_WHILE) /* have a DO count WHILE expr?       */
        newDo->type = DO_COUNT_WHILE;  /* this is the WHILE form            */
                                       /* have a DO count UNTIL expr?       */
      else if (conditional == SUBKEY_UNTIL)
        newDo->type = DO_COUNT_UNTIL;  /* this is the UNTIL form            */
    }
  }
}

RexxInstruction *RexxSource::dropNew()
/****************************************************************************/
/* Function:  Create a new DROP translator object                           */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  LONG        variableCount;           /* count of variables                */

                                       /* go process the list               */
  variableCount = this->processVariableList(KEYWORD_DROP);
                                       /* create a new translator object    */
  newObject =(RexxObject *)new_variable_instruction(DROP, Drop, sizeof(RexxInstructionDrop)
              + (variableCount - 1) * sizeof(RexxObject *));
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionDrop(variableCount, this->subTerms);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::elseNew(
     RexxToken  *token)                /* ELSE keyword token                */
/****************************************************************************/
/* Function:  Create a new ELSE translator object                           */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */

                                       /* create a new translator object    */
  newObject = (RexxObject *)new_instruction(ELSE, Else);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionElse(token);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::endNew()
/****************************************************************************/
/* Function:  Create a new END translator object                            */
/****************************************************************************/
{
  RexxObject   *newObject;             /* newly created object              */
  RexxToken    *token;                 /* current working token             */
  RexxString   *name;                  /* the end control name              */

  name = OREF_NULL;                    /* no name yet                       */
  token = nextReal();                  /* get the next token                */
  if (token->classId != TOKEN_EOC) {   /* have a name specified?            */
    if (token->classId != TOKEN_SYMBOL)/* must have a symbol here           */
                                       /* this is an error                  */
      report_error(Error_Symbol_expected_end);
    name = token->value;               /* get the name pointer              */
    token = nextReal();                /* get the next token                */
    if (token->classId != TOKEN_EOC)   /* end of the instruction?           */
                                       /* this is an error                  */
      report_error_token(Error_Invalid_data_end, token);
  }
                                       /* create a new translator object    */
  newObject = (RexxObject *)new_instruction(END, End);
                                       /* now complete this                 */
  new((void *)newObject) RexxInstructionEnd(name);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::endIfNew(
     RexxInstructionIf *parent )       /* target parent IF or WHEN clause   */
/****************************************************************************/
/* Function:  Create a new DUMMY END IF translator object                   */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */

                                       /* create a new translator object    */
  newObject = (RexxObject *)new_instruction(ENDIF, EndIf);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionEndIf(parent);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::exitNew()
/****************************************************************************/
/* Function:  Create a EXIT instruction object                              */
/****************************************************************************/
{
  RexxObject   *newObject;             /* newly created object              */
  RexxObject   *expression;            /* expression to evaluate            */

                                       /* process the expression            */
  expression = (RexxObject *)this->expression(TERM_EOC);
                                       /* create a new translator object    */
  newObject = (RexxObject *)new_instruction(EXIT, Exit);
                                       /* now complete this                 */
  new((void *)newObject) RexxInstructionExit((RexxObject *)expression);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::exposeNew()
/****************************************************************************/
/* Function:  Create a new EXPOSE translator object                         */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  LONG        variableCount;           /* count of variables                */

  this->isExposeValid();               /* validate the placement            */
                                       /* go process the list               */
  variableCount = this->processVariableList(KEYWORD_EXPOSE);
                                       /* Get new object                    */
  newObject = (RexxObject *)new_variable_instruction(EXPOSE, Expose, sizeof(RexxInstructionExpose) + (variableCount - 1) * sizeof(RexxObject *));
                                       /* Initialize this new method        */
  new ((void *)newObject) RexxInstructionExpose(variableCount, this->subTerms);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

void RexxSource::RexxInstructionForwardCreate(
    RexxInstructionForward *newObject) /* target FORWARD instruction        */
/****************************************************************************/
/* Function:  Create a FORWARD instruction object                           */
/****************************************************************************/
{
  RexxToken *token;                    /* current working token             */
  BOOL       returnContinue;           /* return or continue found          */

  returnContinue = FALSE;              /* no return or continue yet         */
  token = nextReal();                  /* get the next token                */

  while (token->classId != TOKEN_EOC) {/* while still more to process       */
    if (token->classId != TOKEN_SYMBOL)/* not a symbol token?               */
                                       /* this is an error                  */
      report_error_token(Error_Invalid_subkeyword_forward_option, token);
    switch (this->subKeyword(token)) { /* get the keyword value             */

      case SUBKEY_TO:                  /* FORWARD TO expr                   */
                                       /* have a description already?       */
        if (newObject->target != OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_subkeyword_to);
                                       /* get the keyword value             */
        OrefSet(newObject, newObject->target, this->constantExpression());
                                       /* no expression here?               */
        if (newObject->target == OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_expression_forward_to);
        break;

      case SUBKEY_CLASS:               /* FORWARD CLASS expr                */
                                       /* have a class over ride already?   */
        if (newObject->superClass != OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_subkeyword_forward_class);
                                       /* get the keyword value             */
        OrefSet(newObject, newObject->superClass, this->constantExpression());
                                       /* no expression here?               */
        if (newObject->superClass == OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_expression_forward_class);
        break;

      case SUBKEY_MESSAGE:             /* FORWARD MESSAGE expr              */
                                       /* have a message over ride already? */
        if (newObject->message != OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_subkeyword_message);
                                       /* get the keyword value             */
        OrefSet(newObject, newObject->message, this->constantExpression());
                                       /* no expression here?               */
        if (newObject->message == OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_expression_forward_message);
        break;

      case SUBKEY_ARGUMENTS:           /* FORWARD ARGUMENTS expr            */
                                       /* have a additional already?        */
        if (newObject->arguments != OREF_NULL || newObject->array != OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_subkeyword_arguments);
                                       /* get the keyword value             */
        OrefSet(newObject, newObject->arguments, this->constantExpression());
                                       /* no expression here?               */
        if (newObject->arguments == OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_expression_forward_arguments);
        break;

      case SUBKEY_ARRAY:               /* FORWARD ARRAY (expr, expr)        */
                                       /* have arguments already?           */
        if (newObject->arguments != OREF_NULL || newObject->array != OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_subkeyword_arguments);
        token = nextReal();            /* get the next token                */
                                       /* not an expression list starter?   */
        if (token->classId != TOKEN_LEFT) {
                                       /* this is invalid                   */
          report_error(Error_Invalid_expression_raise_list);
        }
                                       /* process the array list            */
        OrefSet(newObject, newObject->array, this->argArray(token, TERM_RIGHT));
        break;

      case SUBKEY_CONTINUE:            /* FORWARD CONTINUE                  */
        if (returnContinue)            /* already had the return action?    */
                                       /* this is invalid                   */
          report_error(Error_Invalid_subkeyword_continue);
        returnContinue = TRUE;         /* not valid again                   */
                                       /* remember this                     */
        newObject->instructionInfo.flags |= forward_continue;
        break;

      default:                         /* invalid subkeyword                */
                                       /* this is a sub keyword error       */
        report_error_token(Error_Invalid_subkeyword_forward_option, token);
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
  RexxObject   *newObject;             /* newly created object              */

                                       /* create a new translator object    */
  newObject = new_instruction(FORWARD, Forward);
  new((void *)newObject) RexxInstructionForward;
  this->RexxInstructionForwardCreate((RexxInstructionForward *)newObject);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::guardNew()
/******************************************************************************/
/* Function:  Create a new GUARD translator object                            */
/******************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  RexxArray  *variable_list;           /* list of variables                 */
  RexxToken  *token;                   /* current working token             */
  BOOL        on_off;                  /* ON or OFF version                 */
  RexxObject *expression;              /* WHEN expression                   */
  INT         variable_count;          /* count of variables                */

  expression = OREF_NULL;              /* default no expression             */
  variable_list = OREF_NULL;           /* no variable either                */
  variable_count = 0;                  /* no variables yet                  */
  token = nextReal();                  /* get the next token                */
  if (token->classId != TOKEN_SYMBOL)  /* must have a symbol here           */
                                       /* raise the error                   */
    report_error_token(Error_Symbol_expected_numeric, token);

                                       /* resolve the subkeyword and        */
                                       /* process the subkeyword            */
  switch (this->subKeyword(token)) {
    case SUBKEY_OFF:                   /* GUARD OFF instruction             */
      on_off = FALSE;                  /* this is the guard off form        */
      break;

    case SUBKEY_ON:                    /* GUARD ON instruction              */
      on_off = TRUE;                   /* remember it is the on form        */
      break;

    default:
                                       /* raise an error                    */
      report_error_token(Error_Invalid_subkeyword_guard, token);
      break;
  }
  token = nextReal();                  /* get the next token                */
  if (token->classId == TOKEN_SYMBOL) {/* have the keyword form?            */
                                       /* resolve the subkeyword            */
    switch (this->subKeyword(token)) { /* and process                       */

      case SUBKEY_WHEN:                /* GUARD ON WHEN expr                */
        this->setGuard();              /* turn on guard variable collection */
                                       /* process the expression            */
        expression = this->expression(TERM_EOC);
        if (expression == OREF_NULL)   /* no expression?                    */
                                       /* expression is required after value*/
          report_error(Error_Invalid_expression_guard);
                                       /* retrieve the guard variable list  */
        variable_list = this->getGuard();
                                       /* get the size                      */
        variable_count = variable_list->size();
        break;

      default:                         /* invalid subkeyword                */
                                       /* raise an error                    */
        report_error_token(Error_Invalid_subkeyword_guard_on, token);
        break;
    }
  }
  else if (token->classId != TOKEN_EOC)/* not the end?                      */
                                       /* raise an error                    */
    report_error_token(Error_Invalid_subkeyword_guard_on, token);

                                       /* Get new object                    */
  newObject = (RexxObject *)new_variable_instruction(GUARD, Guard, sizeof(RexxInstructionGuard)
               + (variable_count - 1) * sizeof(RexxObject *));
                                       /* Initialize this new method        */
  new ((void *)newObject) RexxInstructionGuard(expression, variable_list, on_off);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::ifNew(
    INT   type )                       /* type of instruction (IF or WHEN)  */
/****************************************************************************/
/* Function:  Create a new INTERPRET instruction object                     */
/****************************************************************************/
{
  RexxInstruction *newObject;          /* newly created object              */
  RexxObject *condition;               /* expression to evaluate            */
  RexxToken  *token;                   /* working token                     */

                                       /* process the expression            */
  condition = this->expression(TERM_EOC | TERM_THEN | TERM_KEYWORD);
  if (condition == OREF_NULL) {        /* no expression here?               */
    if (type == KEYWORD_IF)            /* IF form?                          */
                                       /* issue the IF message              */
      report_error(Error_Invalid_expression_if);
    else
                                       /* issue the WHEN message            */
      report_error(Error_Invalid_expression_when);
  }
  token = nextReal();                  /* get terminator token              */
  previousToken();                     /* push the terminator back          */
                                       /* create a new translator object    */
  newObject = (RexxInstruction *)new_instruction(IF, If);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionIf(condition, token);
  newObject->setType(type);            /* set the IF/WHEN type              */
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::instructionNew(
     int type )                        /* instruction type                  */
/****************************************************************************/
/* Function:  Create a new INSTRUCTION translator object                    */
/****************************************************************************/
{
  RexxInstruction *newObject;          /* newly created object              */

                                       /* create a new translator object    */
  newObject = (RexxInstruction *)this->sourceNewObject(sizeof(RexxInstruction), TheInstructionBehaviour, KEYWORD_INSTRUCTION);
  newObject->setType(type);            /* set the given type                */
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::interpretNew()
/****************************************************************************/
/* Function:  Create a new INTERPRET instruction object                     */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  RexxObject *expression;              /* expression to evaluate            */

                                       /* process the expression            */
  expression = this->expression(TERM_EOC);
  if (expression == OREF_NULL)         /* no expression here?               */
                                       /* this is invalid                   */
    report_error(Error_Invalid_expression_interpret);
                                       /* create a new translator object    */
  newObject = new_instruction(INTERPRET, Interpret);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionInterpret(expression);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::labelNew()
/******************************************************************************/
/* Function:  Create a new LABEL instruction translator object                */
/******************************************************************************/
{
  RexxObject *newObject;               /* newly create object               */
  RexxToken  *token;                   /* current working token             */
  RexxString *name;                    /* label name                        */
  LOCATIONINFO  location;              /* location information              */

  token = nextToken();                 /* get the next token                */
  name = token->value;                 /* get the label name                */
                                       /* allocate a new object             */
  newObject = new_instruction(LABEL, Label);
                                       /* add to the label list             */
  this->addLabel((RexxInstruction *)newObject, name);
  token = nextReal();                  /* get the colon token               */
  token->getLocation(&location);       /* get the token location            */
                                       /* the clause ends with the colon    */
  ((RexxInstruction *)newObject)->setEnd(location.endline, location.endoffset);
  new ((void *)newObject) RexxInstructionLabel();
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::leaveNew(
     INT type )                        /* type of queueing operation        */
/****************************************************************************/
/* Function:  Create a new LEAVE/ITERATE instruction translator object      */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly create object               */
  RexxToken  *token;                   /* current working token             */
  RexxString *name;                    /* LEAVE/ITERATE control name        */

  name = OREF_NULL;                    /* no name yet                       */
  token = nextReal();                  /* get the next token                */
  if (token->classId != TOKEN_EOC) {   /* have a name specified?            */
                                       /* must have a symbol here           */
    if (token->classId != TOKEN_SYMBOL) {
      if (type == KEYWORD_LEAVE)       /* is it a LEAVE?                    */
                                       /* this is an error                  */
        report_error(Error_Symbol_expected_leave);
      else
                                       /* this is an iterate error          */
        report_error(Error_Symbol_expected_iterate);
    }
    name = token->value;               /* get the name pointer              */
    token = nextReal();                /* get the next token                */
    if (token->classId != TOKEN_EOC) { /* end of the instruction?           */
      if (type == KEYWORD_LEAVE)       /* is it a LEAVE?                    */
                                       /* this is an error                  */
        report_error_token(Error_Invalid_data_leave, token);
      else
                                       /* this is an iterate error          */
        report_error_token(Error_Invalid_data_iterate, token);
    }
  }
                                       /* allocate a new object             */
  newObject = new_instruction(LEAVE, Leave);
                                       /* Initialize this new method        */
  new ((void *)newObject) RexxInstructionLeave(type, name);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::messageNew(
  RexxExpressionMessage *message)      /* message to process                */
/****************************************************************************/
/* Function:  Create a new MESSAGE instruction translator object            */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly create object               */
  INT         argument_count;          /* number of arguments               */

  hold(message);                       /* lock this temporarily             */
                                       /* get the argument count            */
  argument_count = message->argumentCount;
                                       /* allocate a new object             */
  newObject = new_variable_instruction(MESSAGE, Message, sizeof(RexxInstructionMessage) + (argument_count - 1) * sizeof(OREF));
                                       /* Initialize this new method        */
  new ((void *)newObject) RexxInstructionMessage(message);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::messageAssignmentNew(
  RexxExpressionMessage *message,      /* message to process                */
  RexxObject            *expression )  /* assignment value                  */
/****************************************************************************/
/* Function:  Create a new MESSAGE assignment translator object             */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly create object               */
  INT         argument_count;          /* number of arguments               */
  RexxString *name;                    /* message name used                 */

  hold(message);                       /* lock this temporarily             */
  name = (RexxString *)message->u_name; /* get the name                     */
                                       /* need to add an equal sign to name */
  name = this->commonString(name->concat(OREF_EQUAL));
                                       /* get the argument count            */
  argument_count = message->argumentCount + 1;
                                       /* allocate a new object             */
  newObject = new_variable_instruction(MESSAGE, Message, sizeof(RexxInstructionMessage) + (argument_count - 1) * sizeof(OREF));
                                       /* Initialize this new method        */
  new ((void *)newObject) RexxInstructionMessage(message, name, expression);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::nopNew()
/****************************************************************************/
/* Function:  Create a NOP instruction object                               */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  RexxToken  *token;                   /* current working token             */

  token = nextReal();                  /* get the next token                */
  if (token->classId != TOKEN_EOC)     /* not at the end-of-clause?         */
                                       /* bad NOP instruction               */
    report_error_token(Error_Invalid_data_nop, token);
                                       /* create a new translator object    */
  newObject = new_instruction(NOP, Nop);
                                       /* dummy new to consruct VFT         */
  new ((void *)newObject) RexxInstructionNop;
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::numericNew()
/****************************************************************************/
/* Function:  Create a NUMERIC instruction object                           */
/****************************************************************************/
{
  RexxObject  *newObject;              /* newly created object              */
  RexxObject  *expression;             /* expression to evaluate            */
  RexxToken   *token;                  /* current working token             */
  USHORT       type;                   /* type of instruction               */
  UCHAR        flags;                  /* numeric flags                     */

  expression = OREF_NULL;              /* clear the expression              */
  flags = 0;                           /* and the flags                     */
  token = nextReal();                  /* get the next token                */
  if (token->classId != TOKEN_SYMBOL)  /* must have a symbol here           */
                                       /* raise the error                   */
    report_error_token(Error_Symbol_expected_numeric, token);

  type = this->subKeyword(token);      /* resolve the subkeyword            */
  switch (type) {                      /* process the subkeyword            */
    case SUBKEY_DIGITS:                /* NUMERIC DIGITS instruction        */
                                       /* process the expression            */
      expression = this->expression(TERM_EOC);
      break;

    case SUBKEY_FORM:                  /* NUMERIC FORM instruction          */
      token = nextReal();              /* get the next token                */
      if (token->classId == TOKEN_EOC) /* just NUMERIC FORM?                */
        break;                         /* we're all finished                */
                                       /* have the keyword form?            */
      else if (token->classId == TOKEN_SYMBOL) {
                                       /* resolve the subkeyword            */
                                       /* and process                       */
        switch (this->subKeyword(token)) {

          case SUBKEY_SCIENTIFIC:      /* NUMERIC FORM SCIENTIFIC           */
            token = nextReal();        /* get the next token                */
                                       /* end of the instruction?           */
            if (token->classId != TOKEN_EOC)
                                       /* this is an error                  */
              report_error_token(Error_Invalid_data_form, token);
            break;

          case SUBKEY_ENGINEERING:     /* NUMERIC FORM ENGINEERING          */
                                       /* switch to engineering             */
            flags |= numeric_engineering;
            token = nextReal();        /* get the next token                */
                                       /* end of the instruction?           */
            if (token->classId != TOKEN_EOC)
                                       /* this is an error                  */
              report_error_token(Error_Invalid_data_form, token);
            break;

          case SUBKEY_VALUE:           /* NUMERIC FORM VALUE expr           */
                                       /* process the expression            */
            expression = this->expression(TERM_EOC);
                                       /* no expression?                    */
            if (expression == OREF_NULL)
                                       /* expression is required after value*/
              report_error(Error_Invalid_expression_form);
            break;

          default:                     /* invalid subkeyword                */
                                       /* raise an error                    */
            report_error_token(Error_Invalid_subkeyword_form, token);
            break;

        }
      }
      else {                           /* NUMERIC FORM expr                 */
        previousToken();               /* step back a token                 */
                                       /* process the expression            */
        expression = this->expression(TERM_EOC);
      }
      break;

    case SUBKEY_FUZZ:                  /* NUMERIC FUZZ instruction          */
                                       /* process the expression            */
      expression = this->expression(TERM_EOC);
      break;

    default:                           /* invalid subkeyword                */
      report_error_token(Error_Invalid_subkeyword_numeric, token);
      break;
  }
                                       /* create a new translator object    */
  newObject = new_instruction(NUMERIC, Numeric);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionNumeric(expression,type,flags);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::optionsNew()
/****************************************************************************/
/* Function:  Create an OPTIONS instruction object                          */
/****************************************************************************/
{
  RexxObject   *newObject;             /* newly created object              */
  RexxObject   *expression;            /* expression to evaluate            */
  RexxToken    *first;                 /* first token of the clause         */
  RexxToken    *second;                /* second token of the clause        */
  RexxString   *value;                 /* literal token value               */

                                       /* process the expression            */
  expression = (RexxObject *)this->expression(TERM_EOC);
  if (expression == OREF_NULL)         /* no expression here?               */
                                       /* this is invalid                   */
    report_error(Error_Invalid_expression_options);
  firstToken();                        /* reset the clause to beginning     */
  nextToken();                         /* step past the first token         */
  first = nextReal();                  /* get the first token               */
                                       /* first token a string?             */
  if (first->classId == TOKEN_LITERAL) {
    second = nextReal();               /* get the next token                */
    if (second->classId == TOKEN_EOC) {/* literal the only token?           */
                                       /* could have "options 'ETMODE'" or  */
                                       /* "options 'NOETMODE'"              */
      value = first->value;            /* get the literal value             */
      if (value->strICompare("ETMODE")) /* have ETMODE?                     */
                                       /* turn on DBCS processing           */
        this->flags |= DBCS_scanning;  /* set the flag indicator            */
                                       /* have NOETMODE?                    */
      else if (value->strICompare("NOETMODE"))
                                       /* turn off DBCS processing          */
        this->flags &= ~DBCS_scanning; /* set the flag indicator            */
    }
  }
                                       /* create a new translator object    */
  newObject = new_instruction(OPTIONS, Options);
                                       /* now complete this                 */
  new((void *)newObject) RexxInstructionOptions(expression);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::otherwiseNew(
  RexxToken  *token)                   /* OTHERWISE token                   */
/****************************************************************************/
/* Function:  Create an OTHERWISE instruction object                        */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */

                                       /* create a new translator object    */
  newObject = new_instruction(OTHERWISE, OtherWise);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionOtherWise(token);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::parseNew(
  INT argpull)                         /* type of parsing operation         */
/****************************************************************************/
/* Function:  Create a PARSE instruction object                             */
/****************************************************************************/
{
  RexxObject       *newObject;         /* newly created object              */
  RexxObject       *expression;        /* expression for parse value        */
  RexxQueue        *parse_template;    /* parsing template                  */
  RexxToken        *token;             /* current working token             */
  RexxQueue        *variables;         /* template variables                */
  INT               keyword;           /* located keyword                   */
  RexxTrigger      *trigger;           /* current working trigger           */
  INT               trigger_type;      /* type of the current trigger       */
  USHORT            string_source;     /* source of string data             */
  UCHAR             flags;             /* parsing flags                     */
  LONG              templateCount;     /* number of template items          */
  LONG              variableCount;     /* number of variable items          */

  flags = 0;                           /* new flag values                   */
  expression = OREF_NULL;              /* zero out the expression           */
  if (argpull != KEYWORD_PARSE) {      /* have one of the short forms?      */
    string_source = argpull;           /* set the proper source location    */
    flags |= parse_upper;              /* we're uppercasing                 */
  }
  else {                               /* need to parse source keywords     */
    for (;;) {                         /* loop through for multiple keywords*/
      token = nextReal();              /* get a token                       */
                                       /* not a symbol here?                */
      if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
        report_error(Error_Symbol_expected_parse);
                                       /* resolve the keyword               */
      keyword = this->parseOption(token);

      switch (keyword) {               /* process potential modifiers       */

        case SUBKEY_UPPER:             /* doing uppercasing?                */
          if (flags&parse_translate)   /* already had a translation option? */
            break;                     /* this is an unrecognized keyword   */
          flags |= parse_upper;        /* set the translate option          */
          continue;                    /* go around for another one         */

        case SUBKEY_LOWER:             /* doing lowercasing?                */
          if (flags&parse_translate)   /* already had a translation option? */
            break;                     /* this is an unrecognized keyword   */
                                       /* set the translate option          */
          flags |= parse_lower;        /* set the translate option          */
          continue;                    /* go around for another one         */

        case SUBKEY_CASELESS:          /* caseless comparisons?             */
          if (flags&parse_caseless)    /* already had this options?         */
            break;                     /* this is an unrecognized keyword   */
                                       /* set the translate option          */
          flags |= parse_caseless;     /* set the scan option               */
          continue;                    /* go around for another one         */
      }
      break;                           /* fall out into source processing   */
    }

    string_source = keyword;           /* set the source                    */
    switch (keyword) {                 /* now process the parse option      */

      case SUBKEY_PULL:                /* PARSE PULL instruction            */
      case SUBKEY_LINEIN:              /* PARSE LINEIN instruction          */
      case SUBKEY_ARG:                 /* PARSE ARG instruction             */
      case SUBKEY_SOURCE:              /* PARSE SOURCE instruction          */
      case SUBKEY_VERSION:             /* PARSE VERSION instruction         */
        break;                         /* this is already done              */

      case SUBKEY_VAR:                 /* PARSE VAR name instruction        */
        token = nextReal();            /* get the next token                */
                                       /* not a symbol token                */
        if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
          report_error(Error_Symbol_expected_var);
        this->needVariable(token);     /* force a variable form             */
                                       /* get the variable retriever        */
        expression = (RexxObject *)this->addText(token);
        this->saveObject(expression);  /* protect it in the saveTable (required for large PARSE statements) */
        break;

      case SUBKEY_VALUE:               /* need to process an expression     */
        expression = (RexxObject *)this->expression(TERM_WITH | TERM_KEYWORD);
		if ( expression == OREF_NULL )       /* If script not complete, report error RI0001 */
          //report_error(Error_Invalid_template_with);
		  expression = OREF_NULLSTRING ;             // Set to NULLSTRING if not coded in script
        this->saveObject(expression);  /* protecting in the saveTable is better for large PARSE statements */

        token = nextToken();           /* get the terminator                */
        if (token->classId != TOKEN_SYMBOL || this->subKeyword(token) != SUBKEY_WITH)
                                       /* got another error                 */
          report_error(Error_Invalid_template_with);
        break;

      default:                         /* unknown (or duplicate) keyword    */
                                       /* report an error                   */
        report_error_token(Error_Invalid_subkeyword_parse, token);
        break;
    }
  }

  parse_template = this->subTerms;     /* use the sub terms list template   */
  templateCount = 0;                   /* no template items                 */
  variables = this->terms;             /* and the terms list for variables  */
  variableCount = 0;                   /* no variable items                 */
  token = nextReal();                  /* get the next token                */

  for (;;) {                           /* while still in the template       */
                                       /* found the end?                    */
    if (token->classId == TOKEN_EOC) {
                                       /* string trigger, null target       */
      trigger = new (variableCount) RexxTrigger(TRIGGER_END, (RexxObject *)OREF_NULL, variableCount, variables);
      variableCount = 0;               /* have a new set of variables       */
                                       /* add this to the trigger list      */
      parse_template->push((RexxObject *)trigger);
      templateCount++;                 /* and step the count                */
      break;                           /* done processing                   */
    }
                                       /* comma in the template?            */
    else if (token->classId == TOKEN_COMMA) {
      trigger = new (variableCount) RexxTrigger(TRIGGER_END, (RexxObject *)OREF_NULL, variableCount, variables);
      variableCount = 0;               /* have a new set of variables       */
                                       /* add this to the trigger list      */
      parse_template->push((RexxObject *)trigger);
      parse_template->push(OREF_NULL); /* add an empty entry in the list    */
      templateCount += 2;              /* count both of these               */
    }
                                       /* some variety of special trigger?  */
    else if (token->classId == TOKEN_OPERATOR) {
      switch (token->subclass) {       /* process the operator character    */

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
          report_error_token(Error_Invalid_template_trigger, token);
          break;
      }
      token = nextReal();              /* get the next token                */
                                       /* have a variable trigger?          */
      if (token->classId == TOKEN_LEFT) {
        // parse off an expression in the parens.
        RexxObject *subExpr = this->parenExpression(token);
                                       /* create the appropriate trigger    */
        trigger = new (variableCount) RexxTrigger(trigger_type, subExpr, variableCount, variables);
        variableCount = 0;             /* have a new set of variables       */
                                       /* add this to the trigger list      */
        parse_template->push((RexxObject *)trigger);
        templateCount++;               /* add this one in                   */
      }
                                       /* have a symbol?                    */
      else if (token->classId == TOKEN_SYMBOL) {
                                       /* non-variable symbol?              */
        if (token->subclass != SYMBOL_CONSTANT)
                                       /* this is invalid                   */
          report_error_token(Error_Invalid_template_position, token);
                                       /* set the trigger type              */
                                       /* create the appropriate trigger    */
        trigger = new (variableCount) RexxTrigger(trigger_type, this->addText(token), variableCount, variables);
        variableCount = 0;             /* have a new set of variables       */
                                       /* add this to the trigger list      */
        parse_template->push((RexxObject *)trigger);
        templateCount++;               /* step the counter                  */
      }
                                       /* at the end?                       */
      else if (token->classId == TOKEN_EOC)
                                       /* report the missing piece          */
        report_error(Error_Invalid_template_missing);
      else
                                       /* general position error            */
        report_error_token(Error_Invalid_template_position, token);
    }
                                       /* variable string trigger?          */
    else if (token->classId == TOKEN_LEFT) {
      // parse off an expression in the parens.
      RexxObject *subExpr = this->parenExpression(token);
                                       /* create the appropriate trigger    */
      trigger = new (variableCount) RexxTrigger(flags&parse_caseless ? TRIGGER_MIXED : TRIGGER_STRING, subExpr, variableCount, variables);
      variableCount = 0;               /* have a new set of variables       */
                                       /* add this to the trigger list      */
      parse_template->push((RexxObject *)trigger);
      templateCount++;                 /* step the counter                  */
    }
    else if (token->isLiteral()) {     /* non-variable string trigger?      */
                                       /* create the appropriate trigger    */
      trigger = new (variableCount) RexxTrigger(flags&parse_caseless ? TRIGGER_MIXED : TRIGGER_STRING,
          this->addText(token), variableCount, variables);
      variableCount = 0;               /* have a new set of variables       */
                                       /* add this to the trigger list      */
      parse_template->push((RexxObject *)trigger);
      templateCount++;                 /* step the counter                  */
    }
    else if (token->isSymbol()) {      /* symbol in the template?           */
                                       /* non-variable symbol?              */
      if (token->subclass == SYMBOL_CONSTANT) {
                                       /* create the appropriate trigger    */
        trigger = new (variableCount) RexxTrigger(TRIGGER_ABSOLUTE, this->addText(token), variableCount, variables);
        variableCount = 0;             /* have a new set of variables       */
                                       /* add this to the trigger list      */
        parse_template->push((RexxObject *)trigger);
        templateCount++;               /* step the counter                  */
      }
                                       /* place holder period?              */
      else {
        if (token->subclass == SYMBOL_DUMMY)
          variables->push(OREF_NULL);  /* just add an empty item            */
        else                           /* have a variable, add to list      */
          variables->push(this->addText(token));
        variableCount++;               /* step the variable counter         */
      }
    }
    else
                                       /* this is invalid                   */
      report_error_token(Error_Invalid_template_trigger, token);
    token = nextReal();                /* get the next token                */
  }
                                       /* create a new translator object    */
  newObject = new_variable_instruction(PARSE, Parse, sizeof(RexxInstructionParse) + (templateCount - 1) * sizeof(RexxObject *));
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionParse(expression, string_source, flags, templateCount, parse_template);
  this->toss(expression);              /* release the expression, it is saved by newObject */
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::procedureNew()
/****************************************************************************/
/* Function:  Create a new PROCEDURE translator object                      */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  LONG        variableCount;           /* list of variables                 */
  RexxToken  *token;                   /* current working token             */

  token = nextReal();                  /* get the next token                */
  variableCount = 0;                   /* no variables allocated yet        */
  if (token->classId != TOKEN_EOC) {   /* potentially PROCEDURE EXPOSE?     */
    if (token->classId != TOKEN_SYMBOL)/* not a symbol?                     */
                                       /* must be a symbol here             */
      report_error_token(Error_Invalid_subkeyword_procedure, token);
                                       /* not the EXPOSE keyword?           */
    if (this->subKeyword(token) != SUBKEY_EXPOSE)
      report_error_token(Error_Invalid_subkeyword_procedure, token);
                                       /* go process the list               */
    variableCount = this->processVariableList(KEYWORD_PROCEDURE);
  }
                                       /* create a new translator object    */
  newObject = new_variable_instruction(PROCEDURE, Procedure, sizeof(RexxInstructionProcedure) + (variableCount - 1) * sizeof(OREF));
                                       /* Initialize this new method        */
  new ((void *)newObject) RexxInstructionProcedure(variableCount, this->subTerms);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::queueNew(
  INT type)                            /* type of queueing operation        */
/****************************************************************************/
/* Function:  Create a QUEUE instruction object                             */
/****************************************************************************/
{
  RexxObject   *newObject;             /* newly created object              */
  RexxObject   *expression;            /* expression to evaluate            */

                                       /* process the expression            */
  expression = this->expression(TERM_EOC);
                                       /* create a new translator object    */
  newObject = new_instruction(QUEUE, Queue);
                                       /* now complete this                 */
  new((void *)newObject) RexxInstructionQueue(expression, type);
  return (RexxInstruction *)newObject; /* done, return this                 */
}
RexxInstruction *RexxSource::raiseNew()
/****************************************************************************/
/* Function:  Create a new RAISE translator object                             */
/****************************************************************************/
{
  RexxObject            *newObject;    /* newly created object              */
  RexxToken             *token;        /* current working token             */
  RexxString            *condition;    /* constructed user condition name   */
  RexxObject            *expression;   /* condition extra expression        */
  RexxObject            *description;  /* description expression            */
  RexxObject            *additional;   /* additional expression             */
  LONG                   arrayCount;   /* array expressions                 */
  RexxObject            *result;       /* result expression                 */
  INT                    keyword;      /* type of condition found           */
  RexxQueue             *saveQueue;    /* temporary save queue              */
  BOOL                   raiseReturn;  /* return form                       */


  arrayCount = -1;                     /* clear out the temporaries         */
  expression = OREF_NULL;
  description = OREF_NULL;
  additional = OREF_NULL;
  result = OREF_NULL;
  raiseReturn = FALSE;                 /* default is exit form              */

  saveQueue = new_queue();             /* get a new queue item              */
  this->saveObject(saveQueue);         /* protect it                        */
  token = nextReal();                  /* get the next token                */
                                       /* no target specified or have a     */
  if (token->classId != TOKEN_SYMBOL)  /* bad token type?                   */
                                       /* this is an error                  */
    report_error(Error_Symbol_expected_raise);
  condition = token->value;            /* use the condition string value    */
  saveQueue->push(condition);          /* save the condition name           */
  keyword = this->condition(token);    /* check for the subkeywords         */
  switch (keyword) {                   /* process the different conditions  */

    case CONDITION_FAILURE:            /* RAISE FAILURE returncode          */
    case CONDITION_ERROR:              /* RAISE ERROR   returncode          */
    case CONDITION_SYNTAX:             /* RAISE SYNTAX  number              */
                                       /* go get the keyword value expr.    */
      expression = this->constantExpression();
      if (expression == OREF_NULL) {   /* no expression given?              */
        token = nextToken();           /* get the terminator token          */
                                       /* this is an invalid expression     */
        report_error_token(Error_Invalid_expression_general, token);
      }
      saveQueue->queue(expression);    /* protect it                        */
      break;

    case CONDITION_USER:               /* CONDITION USER usercondition      */
      token = nextReal();              /* get the next token                */
                                       /* bad token type?                   */
      if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
        report_error(Error_Symbol_expected_user);
      condition = token->value;        /* get the token string value        */
                                       /* condition name is "USER condition"*/
      condition = condition->concatToCstring(CHAR_USER_BLANK);
                                       /* get the common version            */
      condition = this->commonString(condition);
      saveQueue->queue(condition);     /* save the condition                */
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
      report_error_token(Error_Invalid_subkeyword_raise, token);
      break;
  }
  token = nextReal();                  /* get the next token                */
  while (token->classId != TOKEN_EOC) {/* while still more to process       */
    if (token->classId != TOKEN_SYMBOL)/* not a symbol token?               */
                                       /* this is an error                  */
      report_error_token(Error_Invalid_subkeyword_raiseoption, token);
    keyword = this->subKeyword(token); /* get the keyword value             */
    switch (keyword) {                 /* process the subkeywords           */

      case SUBKEY_DESCRIPTION:         /* RAISE ... DESCRIPTION expr        */
        if (description != OREF_NULL)  /* have a description already?       */
                                       /* this is invalid                   */
          report_error(Error_Invalid_subkeyword_description);
                                       /* get the keyword value             */
        description = this->constantExpression();
        if (description == OREF_NULL)  /* no expression here?               */
                                       /* this is invalid                   */
          report_error(Error_Invalid_expression_raise_description);
        saveQueue->queue(description); /* protect this                      */
        break;

      case SUBKEY_ADDITIONAL:          /* RAISE ... ADDITIONAL expr         */
                                       /* have a additional already?        */
        if (additional != OREF_NULL || arrayCount != -1)
                                       /* this is invalid                   */
          report_error(Error_Invalid_subkeyword_additional);
                                       /* get the keyword value             */
        additional = this->constantExpression();
                                       /* no expression here?               */
        if (additional == OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_expression_raise_additional);
        saveQueue->queue(additional);  /* protect this                      */
        break;

      case SUBKEY_ARRAY:               /* RAISE ... ARRAY expr              */
                                       /* have a additional already?        */
        if (additional != OREF_NULL || arrayCount != -1)
                                       /* this is invalid                   */
          report_error(Error_Invalid_subkeyword_additional);
        token = nextReal();            /* get the next token                */
                                       /* not an expression list starter?   */
        if (token->classId != TOKEN_LEFT) {
                                       /* this is invalid                   */
          report_error(Error_Invalid_expression_raise_list);
        }
                                       /* process the array list            */
        arrayCount = this->argList(token, TERM_RIGHT);
        break;

      case SUBKEY_RETURN:              /* RAISE ... RETURN expr             */
                                       /* have a result already?            */
        if (result != OREF_NULL)
                                       /* this is invalid                   */
          report_error(Error_Invalid_subkeyword_result);
        raiseReturn = TRUE;            /* remember this                     */
                                       /* get the keyword value             */
        result = this->constantExpression();
        if (result != OREF_NULL)       /* actually have one?                */
          saveQueue->queue(result);    /* protect this                      */
        break;

      case SUBKEY_EXIT:                /* RAISE ... EXIT expr               */
        if (result != OREF_NULL)       /* have a result already?            */
                                       /* this is invalid                   */
          report_error(Error_Invalid_subkeyword_result);
                                       /* get the keyword value             */
        result = this->constantExpression();
        if (result != OREF_NULL)       /* actually have one?                */
          saveQueue->queue(result);    /* protect this                      */
        break;

      default:                         /* invalid subkeyword                */
                                       /* this is a sub keyword error       */
        report_error_token(Error_Invalid_subkeyword_raiseoption, token);
        break;
    }
    token = nextReal();                /* step to the next keyword          */
  }
  if (arrayCount != -1)                /* have the array version?           */
                                       /* create a new translator object    */
    newObject = new_variable_instruction(RAISE, Raise, sizeof(RexxInstructionRaise) + (arrayCount - 1) * sizeof(OREF));
  else                                 /* static instruction size           */
    newObject = new_instruction(RAISE, Raise);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionRaise(condition, expression, description, additional, result, arrayCount, this->subTerms, raiseReturn);
  this->toss(saveQueue);               /* release the GC lock               */
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::replyNew()
/****************************************************************************/
/* Function:  Create a REPLY instruction object                             */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  RexxObject *expression;              /* expression to evaluate            */
                                       /* process the expression            */
  expression = this->expression(TERM_EOC);
                                       /* create a new translator object    */
  newObject = new_instruction(REPLY, Reply);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionReply(expression);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::returnNew()
/****************************************************************************/
/* Function:  Create a RETURN instruction object                            */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  RexxObject *expression;              /* expression to evaluate            */

                                       /* process the expression            */
  expression = this->expression(TERM_EOC);
                                       /* create a new translator object    */
  newObject = new_instruction(RETURN, Return);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionReturn(expression);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::sayNew()
/****************************************************************************/
/* Function:  Create a SAY instruction object                               */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  RexxObject *expression;              /* expression to evaluate            */
                                       /* process the expression            */
  expression = this->expression(TERM_EOC);
                                       /* create a new translator object    */
  newObject = new_instruction(SAY, Say);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionSay(expression);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::selectNew()
/****************************************************************************/
/* Function:  Create a SELECT instruction object                            */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  RexxToken  *token;                   /* current working token             */

  token = nextToken();                 /* get the next token                */
  if (token->classId != TOKEN_EOC)     /* not at the end-of-clause?         */
                                       /* bad NOP instruction               */
    report_error_token(Error_Invalid_data_select, token);
                                       /* create a new translator object    */
  newObject = new_instruction(SELECT, Select);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionSelect();
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::signalNew()
/****************************************************************************/
/* Function:  Create a SIGNAL instruction object                            */
/****************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */
  RexxToken  *token;                   /* current working token             */
  RexxString *condition;               /* constructed USER condition name   */
  INT         keyword;                 /* signal subkeyword                 */
  RexxObject *expression;              /* signal expression                 */
  UCHAR       flags;                   /* option flags                      */
  RexxString *name;                    /* signal name                       */
  BOOL        signalOff;               /* signal off form                   */

  signalOff = FALSE;                   /* not a SIGNAL OFF instruction      */
  expression = OREF_NULL;              /* no expression yet                 */
  name = OREF_NULL;                    /* no name                           */
  condition = OREF_NULL;               /* and no condition                  */
  flags = 0;                           /* no flags                          */
  token = nextReal();                  /* get the next token                */
  if (token->classId == TOKEN_EOC)     /* no target specified?              */
                                       /* this is an error                  */
    report_error(Error_Symbol_or_string_signal);
                                       /* implicit value form?              */
  else if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL) {
    previousToken();                   /* step back a token                 */
                                       /* go process the expression         */
    expression = (RexxObject *)this->expression(TERM_EOC);
  }
  else {                               /* have a real target                */
                                       /* may have to process ON/OFF forms  */
    if (token->classId == TOKEN_SYMBOL) {
                                       /* check for the subkeywords         */
      keyword = this->subKeyword(token);
      if (keyword == SUBKEY_ON) {      /* SIGNAL ON instruction?            */
        flags |= signal_on;            /* this is a SIGNAL ON               */
        token = nextReal();            /* get the next token                */
                                       /* no condition specified or not a   */
                                       /* symbol?                           */
        if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
          report_error(Error_Symbol_expected_on);
                                       /* get the condition involved        */
        keyword = this->condition(token);
                                       /* invalid condition specified?      */
        if (keyword == 0 || keyword == CONDITION_PROPAGATE)
                                       /* got an error here                 */
          report_error_token(Error_Invalid_subkeyword_signalon, token);
                                       /* actually a USER condition request?*/
        else if (keyword == CONDITION_USER) {
          token = nextReal();          /* get the next token                */
                                       /* no condition specified or not a   */
                                       /* symbol?                           */
          if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
            report_error(Error_Symbol_expected_user);
          name = token->value;         /* set the default target            */
                                       /* condition name is "USER condition"*/
          condition = name->concatToCstring(CHAR_USER_BLANK);
                                       /* save the condition name           */
          condition = this->commonString(condition);
        }
        else {                         /* language defined condition        */
          name = token->value;         /* set the default target            */
          condition = name;            /* condition is the same as target   */
        }
        token = nextReal();            /* get the next token                */
                                       /* anything real here?               */
        if (token->classId != TOKEN_EOC) {
                                       /* not a symbol?                     */
          if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
            report_error_token(Error_Invalid_subkeyword_signalonname, token);
                                       /* not the name token?               */
          if (this->subKeyword(token) != SUBKEY_NAME)
                                       /* got an error here                 */
            report_error_token(Error_Invalid_subkeyword_signalonname, token);
          token = nextReal();          /* get the next token                */
                                       /* not got a symbol here?            */
          if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
                                       /* this is an error                  */
            report_error(Error_Symbol_or_string_name);
          name = token->value;         /* set the new target location       */
          token = nextReal();          /* get the next token                */
                                       /* must have the clause end here     */
          if (token->classId != TOKEN_EOC)
                                       /* this is an error                  */
            report_error_token(Error_Invalid_data_name, token);
        }
      }
      else if (keyword == SUBKEY_OFF) {/* SIGNAL OFF instruction?           */
        signalOff = TRUE;              /* doing a SIGNAL OFF                */
        token = nextReal();            /* get the next token                */
                                       /* no condition specified or not a   */
                                       /* symbol?                           */
        if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
          report_error(Error_Symbol_expected_off);
                                       /* get the condition involved        */
        keyword = this->condition(token);
                                       /* invalid condition specified?      */
        if (keyword == 0 || keyword == CONDITION_PROPAGATE)
                                       /* got an error here                 */
          report_error_token(Error_Invalid_subkeyword_signaloff, token);
                                       /* actually a USER condition request?*/
        else if (keyword == CONDITION_USER) {
          token = nextReal();          /* get the next token                */
                                       /* no condition specified or not a   */
                                       /* symbol?                           */
          if (token->classId != TOKEN_SYMBOL)
                                       /* this is an error                  */
            report_error(Error_Symbol_expected_user);
          condition = token->value;    /* get the token string value        */
                                       /* condition name is "USER condition"*/
          condition = condition->concatToCstring(CHAR_USER_BLANK);
                                       /* save the condition name           */
          condition = this->commonString(condition);
        }
        else                           /* language defined condition        */
          condition = token->value;    /* set the condition name            */
        token = nextReal();            /* get the next token                */
                                       /* must have the clause end here     */
        if (token->classId != TOKEN_EOC)
                                       /* this is an error                  */
          report_error_token(Error_Invalid_data_condition, token);
      }
                                       /* is this the value keyword form?   */
      else if (keyword == SUBKEY_VALUE) {
                                       /* get the expression                */
        expression = this->expression(TERM_EOC);
        if (expression == OREF_NULL)   /* no expression here?               */
                                       /* this is invalid                   */
          report_error(Error_Invalid_expression_signal);
      }
      else {                           /* normal SIGNAL instruction         */
        name = token->value;           /* set the signal target             */
        token = nextReal();            /* step to the next token            */
                                       /* not the end?                      */
        if (token->classId != TOKEN_EOC)
                                       /* have an error                     */
          report_error_token(Error_Invalid_data_signal, token);
      }
    }
    else {                             /* signal with a string target       */
      name = token->value;             /* set the signal target             */
      token = nextReal();              /* step to the next token            */
      if (token->classId != TOKEN_EOC) /* not the end?                      */
                                       /* have an error                     */
        report_error_token(Error_Invalid_data_signal, token);
    }
  }
                                       /* create a new translator object    */
  newObject = new_instruction(SIGNAL, Signal);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionSignal(expression, condition, name, flags);
  if (!signalOff)                      /* need to resolve later?            */
    this->addReference(newObject);     /* add to table of references        */
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::thenNew(
     RexxToken         *token,         /* THEN keyword token                */
     RexxInstructionIf *parent )       /* target parent IF or WHEN clause   */
/****************************************************************************/
/* Function:  Create a THEN instruction object                              */
/****************************************************************************/
{
  RexxObject   *newObject;             /* newly created object              */

                                       /* create a new translator object    */
  newObject = new_instruction(THEN, Then);
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionThen(token, parent);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::traceNew()
/****************************************************************************/
/* Function:  Create a TRACE instruction object                             */
/****************************************************************************/
{
  RexxToken  *token;                   /* current working token             */
  RexxString *value;                   /* string value of symbol            */
  RexxObject *expression;              /* trace expression                  */
  RexxObject *newObject;               /* newly created object              */
  INT         setting;                 /* new trace setting                 */
  INT         debug;                   /* new debug setting                 */
  LONG        debug_skip;              /* amount to skip                    */
  CHAR        debug_flags;             /* current debug flags               */

  setting = TRACE_NORMAL;              /* set default trace mode            */
  debug_skip = 0;                      /* no skipping                       */
  debug_flags = 0;                     /* no debug flags                    */
  expression = OREF_NULL;              /* not expression form               */
  token = nextReal();                  /* get the next token                */
  if (token->classId != TOKEN_EOC) {   /* more than just TRACE?             */
                                       /* is this a symbol?                 */
    if (token->classId == TOKEN_SYMBOL) {
                                       /* TRACE VALUE expr?                 */
      if (this->subKeyword(token) == SUBKEY_VALUE) {
                                       /* process the expression            */
        expression = this->expression(TERM_EOC);
        if (expression == OREF_NULL)   /* no expression?                    */
                                       /* expression is required after value*/
          report_error(Error_Invalid_expression_trace);
      }
      else {                           /* must have a symbol here           */
        value = token->value;          /* get the string value              */
        token = nextReal();            /* get the next token                */
                                       /* end of the instruction?           */
        if (token->classId != TOKEN_EOC)
                                       /* this is an error                  */
          report_error_token(Error_Invalid_data_trace, token);
                                       /* convert the value                 */
        debug_skip = REQUEST_LONG(value, NO_LONG);

        if (debug_skip == NO_LONG) {   /* bad value?                        */
          debug_skip = 0;              /* belt and braces                   */
                                       /* process the setting               */
          this->parseTraceSetting(value, &setting, &debug);
          debug_flags |= debug;        /* for execution time                */
        }
        else
          setting = 0;                 /* not a normal setting situation    */
      }
    }
    else if (token->isLiteral()) {     /* is this a string?                 */
      value = token->value;            /* get the string value              */
      token = nextReal();              /* get the next token                */
      if (token->classId != TOKEN_EOC) /* end of the instruction?           */
                                       /* this is an error                  */
        report_error_token(Error_Invalid_data_trace, token);
                                       /* convert the value                 */
      debug_skip = REQUEST_LONG(value, NO_LONG);
      if (debug_skip == NO_LONG) {     /* bad value?                        */
        debug_skip = 0;                /* belt and braces                   */
                                       /* process the setting               */
        this->parseTraceSetting(value, &setting, &debug);
        debug_flags |= debug;          /* for execution time                */
      }
      else
        setting = 0;                   /* not a normal setting situation    */
    }
                                       /* is this a minus sign?             */
    else if (token->subclass == OPERATOR_SUBTRACT || token->subclass == OPERATOR_PLUS) {
                                       /* minus form?                       */
      if (token->subclass == OPERATOR_SUBTRACT)
        debug_flags |= trace_notrace;  /* turn on the notrace flag          */
      setting = 0;                     /* indicate a debug version          */
      token = nextReal();              /* get the next token                */
      if (token->classId == TOKEN_EOC) /* end of the instruction?           */
                                       /* this is an error                  */
        report_error_token(Error_Invalid_expression_general, token);
      value = token->value;            /* get the string value              */
      token = nextReal();              /* get the next token                */
      if (token->classId != TOKEN_EOC) /* end of the instruction?           */
                                       /* this is an error                  */
        report_error(Error_Invalid_data_trace);
                                       /* convert the value                 */
      debug_skip = REQUEST_LONG(value, NO_LONG);
      if (debug_skip == NO_LONG)       /* bad value?                        */
                                       /* have an error                     */
        report_error1(Error_Invalid_whole_number_trace, value);
    }
    else {                             /* implicit value form               */
      previousToken();                 /* step back a token                 */
                                       /* process the expression            */
      expression = this->expression(TERM_EOC);
    }
  }
                                       /* create a new translator object    */
  newObject = new_instruction(TRACE, Trace);
                                       /* now complete this                 */
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionTrace(expression, setting, debug_flags, debug_skip);
  return (RexxInstruction *)newObject; /* done, return this                 */
}

RexxInstruction *RexxSource::useNew()
/****************************************************************************/
/* Function:  Create a USE instruction object                               */
/****************************************************************************/
{
  RexxObject       *newObject;         /* newly created object              */
  RexxToken        *token;             /* current working token             */
  RexxObject       *retriever;         /* retriever for a variable          */
  LONG              variableCount;     /* count of variables                */
  RexxQueue        *variable_list;     /* list of variables                 */

  token = nextReal();                  /* get the next token                */
                                       /* next token ARG?                   */
  if (this->subKeyword(token) != SUBKEY_ARG)
                                       /* invalid keyword                   */
    report_error_token(Error_Invalid_subkeyword_use, token);
  variableCount = 0;                   /* no variables yet                  */
  variable_list = this->subTerms;      /* use the sub terms queue           */
  token = nextReal();                  /* get the next token                */
  while (token->classId != TOKEN_EOC) {/* while more tokens                 */
                                       /* not a symbol token?               */
    if (token->classId == TOKEN_COMMA) {
      variable_list->push(OREF_NULL);  /* add a NIL entry to the list       */
      variableCount++;                 /* and step the count                */
    }
    else {
                                       /* not a symbol character?           */
      if (token->classId != TOKEN_SYMBOL)
                                       /* we have an error                  */
        report_exception(Error_Symbol_expected_use);
      this->needVariable(token);       /* non-variable symbol?              */
      retriever = this->addText(token);/* get a retriever for this          */
      variable_list->push(retriever);  /* add to the variable list          */
      variableCount++;                 /* and step the count                */
      token = nextReal();              /* get the next token                */
      if (token->classId == TOKEN_EOC) /* end of clause?                    */
        break;                         /* finished                          */
                                       /* not a comma?                      */
      else if (token->classId != TOKEN_COMMA)
                                       /* this is an error                  */
        report_error_token(Error_Translation_use_comma, token);
    }
    token = nextReal();                /* get the next token                */
  }
                                       /* create a new translator object    */
  newObject = new_variable_instruction(USE, Use, sizeof(RexxInstructionUse) + (variableCount - 1) * sizeof(OREF));
                                       /* now complete this                 */
  new ((void *)newObject) RexxInstructionUse(variableCount, variable_list);
  return (RexxInstruction *)newObject; /* done, return this                 */
}
