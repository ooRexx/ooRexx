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
/* REXX Kernel                                                  SourceFile.c    */
/*                                                                            */
/* Primitive Translator Source File Class                                     */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#define INCL_REXX_STREAM               /* bring in all stream defines       */
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxBuffer.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "MethodClass.hpp"
#include "RexxNativeMethod.hpp"
#include "RexxCode.hpp"
#include "RexxVariableDictionary.hpp"
#include "RexxSmartBuffer.hpp"
#include "SourceFile.hpp"

#include "ExpressionFunction.hpp"                 /* expression terms                  */
#include "ExpressionMessage.hpp"
#include "ExpressionOperator.hpp"
#include "ExpressionLogical.hpp"

#include "ExpressionBaseVariable.hpp"                   /* base variable management class    */
#include "ExpressionCompoundVariable.hpp"
#include "ExpressionDotVariable.hpp"
#include "ExpressionVariable.hpp"
#include "IndirectVariableReference.hpp"
#include "ExpressionStem.hpp"

#include "RexxInstruction.hpp"                /* base instruction definition       */
#include "SelectInstruction.hpp"
#include "ElseInstruction.hpp"
#include "EndIf.hpp"
#include "DoInstruction.hpp"
#include "CallInstruction.hpp"
#include "StreamNative.h"

#include "ASCIIDBCSStrings.hpp"

#define HOLDSIZE         60            /* room for 60 temporaries           */

/* globals for block count based yielding support  */
#ifdef  NOTIMER
extern UINT iTransClauseCounter;       /* defined in WinYield.c            */
                                       /* cnt of blocks translated         */
#define CLAUSESPERYIELD 100            /* yield every n blocks             */
#endif

extern RexxActivity *CurrentActivity;  /* expose current activity object    */
                                       /* table of internal REXX methods    */
extern "C" internalMethodEntry internalMethodTable[];
                                       /* current global settings           */
extern ACTIVATION_SETTINGS *current_settings;

typedef struct _LINE_DESCRIPTOR {
  size_t position;                     /* position within the buffer        */
  size_t length;                       /* length of the line                */
} LINE_DESCRIPTOR;                     /* line within a source buffer       */

#define CLASS_NAME          1          /* location of class name information*/
#define CLASS_PUBLIC_NAME   2          /* class public name (uppercase)     */
#define CLASS_SUBCLASS_NAME 3          /* location of class subclass        */
#define CLASS_METACLASS     4          /* location of class metaclass       */
#define CLASS_INHERIT       5          /* class inheritance info            */
#define CLASS_PUBLIC        6          /* class public info                 */
#define CLASS_METHODS       7          /* classes instance methods          */
#define CLASS_CLASS_METHODS 8          /* classes class methods             */
#define CLASS_EXTERNAL_NAME 9          /* class external name               */
#define CLASS_MIXINCLASS    10         /* class mixinclass flag             */
#define CLASS_DIRECTIVE     11         /* class directive instruction       */
                                       /* size of the class info array      */
#define CLASS_INFO_SIZE CLASS_DIRECTIVE

#define line_delimiters "\r\n"         /* stream file line end characters   */

void RexxSource::initBuffered(
    RexxObject *source_buffer)         /* containing source buffer          */
/******************************************************************************/
/* Function:  Initialize a source object using the entire source as a         */
/*            stream buffer                                                   */
/******************************************************************************/
{
  LINE_DESCRIPTOR     descriptor;      /* line description                  */
  PCHAR  scan;                         /* line scanning pointer             */
  PCHAR  current;                      /* current scan location             */
  PCHAR  start;                        /* start of the buffer               */
  size_t length;                       /* length of the buffer              */

                                       /* set the source buffer             */
  OrefSet(this, this->sourceBuffer, (RexxBuffer *)source_buffer);
  OrefSet(this, this->sourceIndices, (RexxBuffer *)new_smartbuffer());
                                       /* point to the data part            */
  start = ((RexxBuffer *)this->sourceBuffer)->data;
                                       /* get the buffer length             */
  length = ((RexxBuffer *)this->sourceBuffer)->length();

  if (start[0] == '#' &&
      start[1] == '!') {               // neutralize shell '#!...'
    memcpy(start, "--", 2);
  }

  descriptor.position = 0;             /* fill in the "zeroth" position     */
  descriptor.length = 0;               /* and the length                    */
                                       /* add to the line list              */
  (((RexxSmartBuffer *)(this->sourceIndices)))->copyData((PVOID)&descriptor, sizeof(descriptor));
  this->line_count = 0;                /* start with zero lines             */
                                       /* look for an EOF mark              */
  scan = (PCHAR)memchr(start, ctrl_z, length);
  if (scan != NULL)                    /* found one?                        */
    length = scan - start;             /* reduce the length                 */
  current = start;                     /* start at the beginning            */
  while (length != 0) {                /* loop until all done               */
    this->line_count++;                /* add in another line               */
                                       /* set the start position            */
    descriptor.position = current - start;
                                       /* scan for a important character    */
    scan = mempbrk(current, line_delimiters, length);
                                       /* need to skip over null chars      */
    while (scan != OREF_NULL && *scan == '\0') {
                                       /* scan for a linend                 */
      scan = mempbrk(scan + 1, line_delimiters, length - (scan - current - 1));
    }
    if (scan == NULL) {                /* not found, go to the end          */
      current = current + length;      /* step to the end                   */
      descriptor.length = length;      /* use the entire line               */
      length = 0;                      /* nothing left to process           */
    }
    else {
                                       /* calculate this line length        */
      descriptor.length = scan - current;
                                       /* adjust scan at line end           */
      if (*scan == line_delimiters[0]) {/* CR encountered                   */
        scan++;                        /* step the scan position            */
        /* now check for LF */
        if (length > (size_t)(scan - current))
          if (*scan != '\0' && *scan == line_delimiters[1])     /*          */
            scan++;                    /* step again, if required           */
      }
      else                             /* just a LF                         */
        scan++;                        /* step the scan position            */

      length -= scan - current;        /* reduce the length                 */
      current = scan;                  /* copy the scan pointer             */
    }
                                       /* add to the line list              */
    (((RexxSmartBuffer *)(this->sourceIndices)))->copyData((PVOID)&descriptor, sizeof(descriptor));
  }
                                       /* throw away the buffer "wrapper"   */
  OrefSet(this, this->sourceIndices, (((RexxSmartBuffer *)(this->sourceIndices)))->buffer);
  this->position(1, 0);                /* set position at the first line    */
}

void RexxSource::initFile()
/******************************************************************************/
/* Function:  Initialize a source object, reading the source from a file      */
/******************************************************************************/
{
  PCHAR       file_name;               /* ASCII-Z version of the file name  */
  RexxBuffer *program_source;          /* read in program source object     */

                                       /* get the file name pointer         */
  file_name = this->programName->stringData;
                                       /* load the program file             */
  program_source = (RexxBuffer *)SysReadProgram(file_name);
  if (program_source == OREF_NULL) {   /* Program not found or read error?  */
                                       /* report this                       */
    report_exception1(Error_Program_unreadable_name, this->programName);
  }

#ifdef SCRIPTING
  else if (program_source->hashvalue > 9) {
    char begin[10];
    char end[4];
    // check, if XML comments have to be removed from the script... (engine situation)
    memcpy(begin,program_source->data,9);
    // hashvalue is the length of the buffer
    memcpy(end,program_source->data+(program_source->hashvalue-3),3);
    begin[9]=end[3]=0x00;
    if (!stricmp("<![CDATA[",begin) && !stricmp("]]>",end)) {
      memcpy(program_source->data,"         ",9);
      memcpy(program_source->data+(program_source->hashvalue-3),"   ",3);
    }
  }
#endif

                                       /* save the returned buffer          */
  OrefSet(this, this->sourceBuffer, program_source);
  memoryObject.removeSavedObject((RexxObject *)program_source);

                                       /* go process the buffer now         */
  this->initBuffered((RexxObject *)this->sourceBuffer);
}

BOOL RexxSource::reconnect()
/******************************************************************************/
/* Function:  Attempt to reconnect to the original source code file           */
/******************************************************************************/
{
  if (!(this->flags&reclaim_possible)) /* no chance of getting this?        */
    return FALSE;                      /* just get out of here              */
  this->initFile();                    /* go reinit this                    */
  return TRUE;                         /* give back the success return      */
}

void RexxSource::setReconnect()
/******************************************************************************/
/* Function:  Allow a source reconnect to occur                               */
/******************************************************************************/
{
  this->flags |= reclaim_possible;     /* we have a shot at this!           */
}

void RexxSource::interpretLine(size_t line_number)
/******************************************************************************/
/* Arguments:  interpret line location                                        */
/*                                                                            */
/* Function:  Adjust the source object so that it thinks it is scanning a     */
/*            1-line source file with a line number other than 1 so that      */
/*            errors and trace of an interpreted instruction will display     */
/*            the interpret instructions line number.                         */
/******************************************************************************/
{
                                       /* fill in the source size           */
  this->line_count = line_number;      /* size is now the line number       */
  this->line_number = line_number;     /* we are now on line "nn of nn"     */
                                       /* remember for positioning          */
  this->interpret_adjust = line_number - 1;
}

void RexxSource::needVariable(
    RexxToken  *token)                 /* current token                     */
/******************************************************************************/
/* Function:  validate that the current token is a variable token             */
/******************************************************************************/
{
  if (!token->isVariable()) {          /* not a variable token?             */
                                       /* begin with a dot?                 */
    if (token->value->getChar(0) == '.')
      report_error_token(Error_Invalid_variable_period, token);
    else
      report_error_token(Error_Invalid_variable_number, token);
  }
}

void RexxSource::needVariableOrDotSymbol(
    RexxToken  *token)                 /* current token                     */
/******************************************************************************/
/* Function:  validate that the current token is a variable token             */
/******************************************************************************/
{
                                       /* not a variable token or dot symbol*/
  if (!token->isVariable() && (token->subclass != SYMBOL_DOTSYMBOL)) {
      report_error_token(Error_Invalid_variable_number, token);
  }
}

BOOL RexxSource::terminator(
    int         terminators,           /* set of possible terminators       */
    RexxToken  *token)                 /* token being processed             */
/******************************************************************************/
/* Function:  test for a terminator token in the given context                */
/******************************************************************************/
{
  BOOL    endtoken;                    /* found the end flag                */

  endtoken = FALSE;                    /* not found the end yet             */

  switch (token->classId) {            /* process based on terminator class */

    case  TOKEN_EOC:                   /* found the end-of-clause           */
      endtoken = TRUE;                 /* this is always an end marker      */
      break;

    case  TOKEN_RIGHT:                 /* found a right paren               */
      if (terminators&TERM_RIGHT)      /* terminate on this?                */
        endtoken = TRUE;               /* set the flag                      */
      break;

    case  TOKEN_SQRIGHT:               /* found a right square bracket      */
      if (terminators&TERM_SQRIGHT)    /* terminate on this?                */
        endtoken = TRUE;               /* set the flag                      */
      break;

    case  TOKEN_COMMA:                 /* found a comma                     */
      if (terminators&TERM_COMMA)      /* terminate on this?                */
        endtoken = TRUE;               /* set the flag                      */
      break;

    case  TOKEN_SYMBOL:                /* have a symbol, need to resolve    */
      if (terminators&TERM_KEYWORD) {  /* need to do keyword checks?        */
                                       /* process based on the keyword      */
        switch (this->subKeyword(token)) {

          case SUBKEY_TO:              /* TO subkeyword                     */
            if (terminators&TERM_TO)   /* terminate on this?                */
              endtoken = TRUE;         /* set the flag                      */
            break;

          case SUBKEY_BY:              /* BY subkeyword                     */
            if (terminators&TERM_BY)   /* terminate on this?                */
              endtoken = TRUE;         /* set the flag                      */
            break;

          case SUBKEY_FOR:             /* FOR subkeyword                    */
            if (terminators&TERM_FOR)  /* terminate on this?                */
              endtoken = TRUE;         /* set the flag                      */
            break;

          case SUBKEY_WHILE:           /* WHILE subkeyword                  */
          case SUBKEY_UNTIL:           /* UNTIL subkeyword                  */
            if (terminators&TERM_WHILE)/* terminate on this?                */
              endtoken = TRUE;         /* set the flag                      */
            break;

          case SUBKEY_WITH:            /* WITH subkeyword                   */
            if (terminators&TERM_WITH) /* terminate on this?                */
              endtoken = TRUE;         /* set the flag                      */
            break;

          case SUBKEY_THEN:            /* THEN subkeyword                   */
            if (terminators&TERM_THEN) /* terminate on this?                */
              endtoken = TRUE;         /* set the flag                      */
            break;

          default:                     /* not a terminator for others       */
            break;
        }
      }
      default:                           /* not a terminator for others       */
        break;
  }
  if (endtoken)                        /* found the end one?                */
    previousToken();                   /* push it back on the clause        */
  return endtoken;                     /* return the TRUE/FALSE flag        */
}

void RexxSource::nextLine()
/******************************************************************************/
/* Function:  Advance the current position to the next source line            */
/******************************************************************************/
{
  if (this->clause)                    /* have a clause object?             */
                                       /* record current position in clause */
    this->clause->setEnd(this->line_number, this->line_offset);
                                       /* move to the start of the next line*/
  this->position(this->line_number + 1, 0);
}

void RexxSource::position(
    size_t line,                       /* target line number                */
    size_t offset)                     /* target line offset                */
/******************************************************************************/
/* Function:  Move the current scan position to a new spot                    */
/******************************************************************************/
{
   LINE_DESCRIPTOR *descriptors;       /* line descriptors                  */
   PCHAR            buffer_start;      /* start of source buffer            */
   RexxString       *new_line;         /* new line to scan                  */

   this->line_number = line;           /* set the line number               */
   this->line_offset = offset;         /* and the offset                    */
   if (line > this->line_count) {      /* past the end?                     */
     this->current = OREF_NULL;        /* null out the current line         */
     this->current_length = 0;         /* tag this as a null line           */
   }
   else {
                                       /* working from an array?            */
     if (this->sourceArray != OREF_NULL) {
                                       /* get the next line                 */
       new_line = (RexxString *)(this->sourceArray->get(line - this->interpret_adjust));
       if (new_line == OREF_NULL)      /* missing line?                     */
                                       /* this is an error                  */
         report_exception(Error_Translation_invalid_line);
       if (!OTYPE(String, new_line)) { /* not working with a string?        */
                                       /* get this as a string              */
         new_line = (RexxString *)new_line->stringValue();
         if (new_line == TheNilObject) /* got back .nil?                    */
                                       /* this is an error                  */
           report_exception(Error_Translation_invalid_line);
       }
                                       /* set the program pointer           */
       this->current = new_line->stringData;
                                       /* get the string length             */
       this->current_length = new_line->length;
     }
     else {                            /* single buffer source              */
                                       /* get the descriptors pointer       */
       descriptors = (LINE_DESCRIPTOR *)(this->sourceIndices->data);
                                       /* source buffered in a string?      */
       if (OTYPE(String, this->sourceBuffer))
                                       /* point to the data part            */
         buffer_start = ((RexxString *)(this->sourceBuffer))->stringData;
       else
                                       /* point to the data part            */
         buffer_start = this->sourceBuffer->data;
                                       /* calculate the line start          */
       this->current = buffer_start + descriptors[line - this->interpret_adjust].position;
                                       /* and get the length                */
       this->current_length = descriptors[line - this->interpret_adjust].length;
     }
   }
}

void RexxSource::live()
/******************************************************************************/
/* Perform garbage collection marking of a source object                      */
/******************************************************************************/
{
  setUpMemoryMark
  memory_mark(this->sourceArray);
  memory_mark(this->programName);
  memory_mark(this->clause);
  memory_mark(this->securityManager);
  memory_mark(this->sourceBuffer);
  memory_mark(this->sourceIndices);
  memory_mark(this->first);
  memory_mark(this->last);
  memory_mark(this->currentInstruction);
  memory_mark(this->savelist);
  memory_mark(this->holdstack);
  memory_mark(this->variables);
  memory_mark(this->literals);
  memory_mark(this->labels);
  memory_mark(this->strings);
  memory_mark(this->guard_variables);
  memory_mark(this->exposed_variables);
  memory_mark(this->control);
  memory_mark(this->terms);
  memory_mark(this->subTerms);
  memory_mark(this->operators);
  memory_mark(this->calls);
  memory_mark(this->routines);
  memory_mark(this->public_routines);
  memory_mark(this->class_dependencies);
  memory_mark(this->requires);
  memory_mark(this->classes);
  memory_mark(this->installed_public_classes);
  memory_mark(this->installed_classes);
  memory_mark(this->merged_public_classes);
  memory_mark(this->merged_public_routines);
  memory_mark(this->methods);
  memory_mark(this->active_class);
  cleanUpMemoryMark
}

void RexxSource::liveGeneral()
/******************************************************************************/
/* Function:  Perform generalized marking of a source object                  */
/******************************************************************************/
{
#ifndef KEEPSOURCE
  if (memoryObject.savingImage()) {    /* save image time?                  */
                                       /* don't save the source image       */
    OrefSet(this, this->sourceArray, OREF_NULL);
    OrefSet(this, this->sourceBuffer, OREF_NULL);
    OrefSet(this, this->sourceIndices, OREF_NULL);
    OrefSet(this, this->clause, OREF_NULL);
                                       /* don't save the install information*/
    OrefSet(this, this->methods, OREF_NULL);
    OrefSet(this, this->requires, OREF_NULL);
    OrefSet(this, this->classes, OREF_NULL);
    OrefSet(this, this->routines, OREF_NULL);
    OrefSet(this, this->installed_classes, OREF_NULL);
    OrefSet(this, this->installed_public_classes, OREF_NULL);
    OrefSet(this, this->merged_public_classes, OREF_NULL);
    OrefSet(this, this->merged_public_routines, OREF_NULL);
    this->flags &= ~reclaim_possible;  /* can't recover source immediately  */
  }
#endif
  setUpMemoryMarkGeneral
  memory_mark_general(this->sourceArray);
  memory_mark_general(this->programName);
  memory_mark_general(this->clause);
  memory_mark_general(this->securityManager);
  memory_mark_general(this->sourceBuffer);
  memory_mark_general(this->sourceIndices);
  memory_mark_general(this->first);
  memory_mark_general(this->currentInstruction);
  memory_mark_general(this->last);
  memory_mark_general(this->savelist);
  memory_mark_general(this->holdstack);
  memory_mark_general(this->variables);
  memory_mark_general(this->literals);
  memory_mark_general(this->labels);
  memory_mark_general(this->strings);
  memory_mark_general(this->guard_variables);
  memory_mark_general(this->exposed_variables);
  memory_mark_general(this->control);
  memory_mark_general(this->terms);
  memory_mark_general(this->subTerms);
  memory_mark_general(this->operators);
  memory_mark_general(this->calls);
  memory_mark_general(this->routines);
  memory_mark_general(this->public_routines);
  memory_mark_general(this->class_dependencies);
  memory_mark_general(this->requires);
  memory_mark_general(this->classes);
  memory_mark_general(this->installed_public_classes);
  memory_mark_general(this->installed_classes);
  memory_mark_general(this->merged_public_classes);
  memory_mark_general(this->merged_public_routines);
  memory_mark_general(this->methods);
  memory_mark_general(this->active_class);
  cleanUpMemoryMarkGeneral
}

void RexxSource::flatten (RexxEnvelope *envelope)
/******************************************************************************/
/* Function:  Flatten a source object                                         */
/******************************************************************************/
{

  setUpFlatten(RexxSource)
                                       /* if we are flattening for EA's, we   */
                                       /* don't need to to keep source info   */
                                       /* so ask the envelope if this is a    */
                                       /*  flatten to save the method image   */
    if (METHOD_ENVELOPE == envelope->queryType()) {
                                       /* Yes it is, so don't flatten the     */
                                       /*  source image.                      */
      this->sourceArray = OREF_NULL;
      this->sourceBuffer = OREF_NULL;
      this->sourceIndices = OREF_NULL;
      this->securityManager = OREF_NULL;
      this->flags &= ~reclaim_possible;/* can't recover the source immediately*/
    }
    flatten_reference(newThis->sourceArray, envelope);
    flatten_reference(newThis->programName, envelope);
    flatten_reference(newThis->clause, envelope);
    flatten_reference(newThis->securityManager, envelope);
    flatten_reference(newThis->sourceBuffer, envelope);
    flatten_reference(newThis->sourceIndices, envelope);
    flatten_reference(newThis->first, envelope);
    flatten_reference(newThis->last, envelope);
    flatten_reference(newThis->currentInstruction, envelope);
    flatten_reference(newThis->savelist, envelope);
    flatten_reference(newThis->holdstack, envelope);
    flatten_reference(newThis->variables, envelope);
    flatten_reference(newThis->literals, envelope);
    flatten_reference(newThis->labels, envelope);
    flatten_reference(newThis->strings, envelope);
    flatten_reference(newThis->guard_variables, envelope);
    flatten_reference(newThis->exposed_variables, envelope);
    flatten_reference(newThis->control, envelope);
    flatten_reference(newThis->terms, envelope);
    flatten_reference(newThis->subTerms, envelope);
    flatten_reference(newThis->operators, envelope);
    flatten_reference(newThis->calls, envelope);
    flatten_reference(newThis->routines, envelope);
    flatten_reference(newThis->public_routines, envelope);
    flatten_reference(newThis->class_dependencies, envelope);
    flatten_reference(newThis->requires, envelope);
    flatten_reference(newThis->classes, envelope);
    flatten_reference(newThis->installed_public_classes, envelope);
    flatten_reference(newThis->installed_classes, envelope);
    flatten_reference(newThis->merged_public_classes, envelope);
    flatten_reference(newThis->merged_public_routines, envelope);
    flatten_reference(newThis->methods, envelope);
    flatten_reference(newThis->active_class, envelope);

  cleanUpFlatten
}


size_t RexxSource::sourceSize()
/******************************************************************************/
/* Function:  Return count of lines in the source.  If the source is not      */
/*            available, return 0                                             */
/******************************************************************************/
{
                                       /* currently no source?              */
  if ((this->sourceArray == OREF_NULL && this->sourceBuffer == OREF_NULL)) {
    if (!this->reconnect())            /* unable to recover the source?     */
      return 0;                        /* we have no source lines           */
  }
  return this->line_count;             /* return the line count             */
}


BOOL RexxSource::traceable()
/******************************************************************************/
/* Function:  Determine if a program is traceable (i.e., the program source   */
/*            is available)                                                   */
/******************************************************************************/
{
                                       /* currently no source?              */
  if ((this->sourceArray == OREF_NULL && this->sourceBuffer == OREF_NULL)) {
    return this->reconnect();          /* unable to recover the source?     */
  }
  return TRUE;                         /* return the line count             */
}

RexxString *RexxSource::get(
     size_t position)                  /* requested source line             */
/******************************************************************************/
/* Function:  Extract a give source line from the source program              */
/******************************************************************************/
{
  LINE_DESCRIPTOR *descriptors;        /* line descriptors                  */
  PCHAR   buffer_start;                /* start of source buffer            */

  if (position > this->line_count)     /* beyond last line?                 */
    return OREF_NULLSTRING;            /* just return a null string         */
                                       /* working from an array?            */
  if (this->sourceArray != OREF_NULL) {
                                       /* return the array line             */
    return (RexxString *)(this->sourceArray->get(position));
  }
                                       /* buffered version?                 */
  else if (this->sourceBuffer != OREF_NULL) {
                                       /* get the descriptors pointer       */
    descriptors = (LINE_DESCRIPTOR *)(this->sourceIndices->data);
                                       /* source buffered in a string?      */
    if (OTYPE(String, this->sourceBuffer))
                                       /* point to the data part            */
      buffer_start = ((RexxString *)(this->sourceBuffer))->stringData;
    else
                                       /* point to the data part            */
      buffer_start = this->sourceBuffer->data;
                                       /* create a new string version       */
    return new_string(buffer_start + descriptors[position].position, descriptors[position].length);
  }
  else
    return OREF_NULLSTRING;            /* we have no line                   */
}

void RexxSource::nextClause()
/*********************************************************************/
/* Extract a clause from the source and return as a clause object.   */
/* The clause object contains a list of all of the tokens contained  */
/* within the clause and is used by the parser to determine the      */
/* type of instruction and create the instruction parse tree.        */
/*********************************************************************/
{
  RexxToken   *token;                  /* current token being processed     */
  LOCATIONINFO location;               /* location of the clause            */
  LOCATIONINFO token_location;         /* location of each token            */

  if (!(this->flags&reclaimed)) {      /* need to scan off a clause?        */
    this->clause->newClause();         /* reset the clause object           */
    for (;;) {                         /* loop until we get an non-null     */
                                       /* record the start position         */
      this->clause->setStart(this->line_number, this->line_offset);
                                       /* get the next source token         */
                                       /* (blanks are not significant here) */
      token = this->sourceNextToken(OREF_NULL);
      if (token == OREF_NULL) {        /* hit the end of the file?          */
        this->flags |= no_clause;      /* flag this as a no clause          */
        return;                        /* we're finished                    */
      }
                                       /* is this the end of the clause?    */
      if (token->classId != TOKEN_EOC)
        break;                         /* we've got what we need            */
      this->clause->newClause();       /* reset the clause object           */
    }
                                       /* get the start position            */
    token->getLocation(&token_location);
    location = token_location;         /* copy the location info            */
                                       /* record in clause for errors       */
    this->clause->setLocation(&location);
    for (;;) {                         /* loop until physical end of clause */
                                       /* get the next token of real clause */
                                       /* (blanks can be significant)       */
      token = this->sourceNextToken(token);
                                       /* get this tokens location          */
      token->getLocation(&token_location);
      if (token->classId == TOKEN_EOC) /* end of the clause now?            */
        break;                         /* hit the physical end of clause    */
    }
                                       /* copy over the ending information  */
    location.endline = token_location.endline;
    location.endoffset = token_location.endoffset;
                                       /* record the clause position        */
    this->clause->setLocation(&location);
  }
  this->flags &= ~reclaimed;           /* no reclaimed clause               */
}
                                       /* extra space required to format a  */
                                       /* result line.  This overhead is    */
                                       /* 8 leading spaces for the line     */
                                       /* number, + 1 space + length of the */
                                       /* message prefix (3) + 1 space +    */
                                       /* 2 for an indent + 2 for the       */
                                       /* quotes surrounding the value      */
#define TRACE_OVERHEAD 16
                                       /* overhead for a traced instruction */
                                       /* (8 digit line number, blank,      */
                                       /* 3 character prefix, and a blank   */
#define INSTRUCTION_OVERHEAD 11
#define LINENUMBER 6                   /* size of a line number             */
#define PREFIX_OFFSET (LINENUMBER + 1) /* location of the prefix field      */
#define PREFIX_LENGTH 3                /* length of the prefix flag         */
#define INDENT_SPACING 2               /* spaces per indentation amount     */

RexxString *RexxSource::traceBack(
     PLOCATIONINFO  location,          /* value to trace                    */
     size_t         indent,            /* blank indentation                 */
     BOOL           trace )            /* traced instruction (vs. error)    */
/******************************************************************************/
/* Function:  Format a source line for traceback or tracing                   */
/******************************************************************************/
{
  RexxString  *buffer;                 /* buffer for building result        */
  RexxString  *line;                   /* actual line data                  */
  size_t       outlength;              /* output length                     */
  PCHAR        linepointer;            /* pointer to the line number        */
  CHAR         linenumber[11];         /* formatted line number             */

  line = this->extract(location);      /* extract the source string         */
                                       /* doesn't exist and this isn't a    */
                                       /* trace instruction format?         */
  if (line == OREF_NULLSTRING && !trace)
    return OREF_NULL;                  /* don't trace this either           */
                                       /* format the value                  */
  sprintf(linenumber,"%li",location->line);
  if (indent < 0)                      /* possible negative indentation?    */
    indent = 0;                        /* just reset it                     */
                                       /* get an output string              */
  buffer = raw_string(line->length + INSTRUCTION_OVERHEAD + indent * INDENT_SPACING);
                                       /* blank out the first part          */
  buffer->set(0, ' ', INSTRUCTION_OVERHEAD + indent * INDENT_SPACING);
                                       /* copy in the line                  */
  buffer->put(INSTRUCTION_OVERHEAD + indent * INDENT_SPACING, line->stringData, line->length);
  outlength = strlen(linenumber);      /* get the line number length        */
  linepointer = linenumber;            /* point to number start             */
  if (outlength > LINENUMBER) {        /* too long for defined field?       */
                                       /* step over extra numbers           */
    linepointer += outlength - LINENUMBER;
    *linepointer = '?';                /* overlay a question mark           */
    outlength = LINENUMBER;            /* shorten the length                */
  }
                                       /* copy in the line number           */
  buffer->put(LINENUMBER - outlength, linepointer, outlength);
  buffer->put(PREFIX_OFFSET, "*-*", PREFIX_LENGTH);
  buffer->generateHash();              /* rebuild the hash value            */
  return buffer;                       /* return formatted buffer           */
}

RexxString *RexxSource::extract(
     PLOCATIONINFO  location )        /* target retrieval structure        */
/******************************************************************************/
/* Extrace a line from the source using the given location information        */
/******************************************************************************/
{
  RexxString *line;                    /* returned source line              */
  RexxString *source_line;             /* current extracting line           */
  size_t  counter;                     /* line counter                      */

                                       /* currently no source?              */
  if ((this->sourceArray == OREF_NULL && this->sourceBuffer == OREF_NULL)) {
    if (!this->reconnect())            /* unable to recover the source?     */
      return OREF_NULLSTRING;          /* return a null array               */
  }
                                       /* is the location out of bounds?    */
  if (location->line == 0 || location->line > this->line_count)
    line = OREF_NULLSTRING;            /* just give back a null string      */
                                       /* all on one line?                  */
  else if (location->line >= location->endline)
                                       /* just extract the string           */
    line = this->get(location->line - this->interpret_adjust)->extract(location->offset,
        location->endoffset - location->offset);
                                       /* multiple line clause              */
  else {
                                       /* get the source line               */
    source_line = this->get(location->line);
                                       /* extract the first part            */
    line = source_line->extract(location->offset, source_line->length - location->offset);
                                       /* loop down to end line             */
    for (counter = location->line + 1 - this->interpret_adjust; counter < location->endline; counter++) {
                                       /* concatenate the next line on      */
      line = line->concat(this->get(counter));
    }
                                       /* now add on the last part          */
    line = line->concat(this->get(counter)->extract(0, location->endoffset));
  }
  return line;                         /* return the extracted line         */
}

RexxArray *RexxSource::extractSource(
     PLOCATIONINFO  location )         /* target retrieval structure        */
/******************************************************************************/
/* Function:  Extract a section of source from a method source object, using  */
/*            the created bounds for the method.                              */
/******************************************************************************/
{
  RexxArray  *source;                  /* returned source array             */
  RexxString *source_line;             /* current extracting line           */
  size_t      counter;                 /* line counter                      */
  size_t      i;                       /* loop counter                      */

                                       /* currently no source?              */
  if ((this->sourceArray == OREF_NULL && this->sourceBuffer == OREF_NULL)) {
    if (!this->reconnect())            /* unable to recover the source?     */
                                       /* return a null array               */
      return (RexxArray *)TheNullArray->copy();
  }
                                       /* is the location out of bounds?    */
  if (location->line == 0 || location->line - this->interpret_adjust > this->line_count)
                                       /* just give back a null array       */
    source = (RexxArray *)TheNullArray->copy();
  else {
    if (location->endline == 0) {      /* no ending line?                   */
                                       /* use the last line                 */
      location->endline = this->line_count;
                                       /* end at the line end               */
      location->endoffset = this->get(location->endline)->length;
    }
                                       /* end at the line start?            */
    else if (location->endoffset == 0) {
      location->endline--;             /* step back a line                  */
                                       /* end at the line end               */
      location->endoffset = this->get(location->endline)->length;
    }
                                       /* get the result array              */
    source = new_array(location->endline - location->line + 1);
                                       /* all on one line?                  */
    if (location->line == location->endline) {
                                       /* get the line                      */
      source_line = this->get(location->line);
                                       /* extract the line segment          */
      source_line = source_line->extract(location->offset, location->endoffset - location->offset);
      source->put(source_line, 1);     /* insert the trailing piece         */
      return source;                   /* all done                          */
    }
    if (location->offset == 0)         /* start on the first location?      */
                                       /* copy over the entire line         */
      source->put(this->get(location->line), 1);
    else {
                                       /* get the line                      */
      source_line = this->get(location->line);
                                       /* extract the end portion           */
      source_line = source_line->extract(location->offset, source_line->length - location->offset);
      source->put(source_line, 1);     /* insert the trailing piece         */
    }
                                       /* loop until the last line          */
    for (counter = location->line + 1, i = 2; counter < location->endline; counter++, i++)
                                       /* copy over the entire line         */
      source->put(this->get(counter), i);
                                       /* get the last line                 */
    source_line = this->get(location->endline);
                                       /* more than one line?               */
    if (location->endline > location->line) {
                                       /* need the entire line?             */
      if (location->endoffset >= source_line->length)
        source->put(source_line, i);   /* just use it                       */
      else
                                       /* extract the tail part             */
        source->put(source_line->extract(0, location->endoffset - 1), i);
    }
  }
  return source;                       /* return the extracted lines        */
}

void RexxSource::globalSetup()
/******************************************************************************/
/* Function:  Perform global parsing initialization                           */
/******************************************************************************/
{
                                       /* holding pen for temporaries       */
  OrefSet(this, this->holdstack, new_stack(HOLDSIZE));
                                       /* create a save table               */
  OrefSet(this, this->savelist, new_object_table());
                                       /* allocate global control tables    */
  OrefSet(this, this->control, new_queue());
  OrefSet(this, this->terms, new_queue());
  OrefSet(this, this->subTerms, new_queue());
  OrefSet(this, this->operators, new_queue());
  OrefSet(this, this->literals, new_directory());
  if (TheGlobalStrings != OREF_NULL)   /* doing an image build?             */
                                       /* use this for the string table     */
    OrefSet(this, this->strings, TheGlobalStrings);
  else
    OrefSet(this, this->strings, new_directory());
                                       /* get the clause object             */
  OrefSet(this, this->clause, new RexxClause());
}

RexxMethod *RexxSource::method()
/******************************************************************************/
/* Function:  Convert a source object into an executable method               */
/******************************************************************************/
{
  RexxMethod *newMethod;               /* returned method                   */

  this->globalSetup();                 /* do the global setup part          */
                                       /* translate the source program      */
  newMethod = this->translate(OREF_NULL);
  save(newMethod);                     /* protect this during cleanup       */
  this->cleanup();                     /* release temporary tables          */
  discard_hold(newMethod);             /* and on the new method             */
  return newMethod;                    /* return the method                 */
}

RexxMethod *RexxSource::interpretMethod(
    RexxDirectory *labels )            /* parent label set                  */
/******************************************************************************/
/* Function:  Convert a source object into an executable interpret method     */
/******************************************************************************/
{
  RexxMethod  *newMethod;              /* returned method                   */

  this->globalSetup();                 /* do the global setup part          */
  this->flags |= _interpret;           /* this is an interpret              */
  newMethod = this->translate(labels); /* translate the source program      */
  save(newMethod);                     /* protect this during cleanup       */
  this->cleanup();                     /* release temporary tables          */
  discard_hold(newMethod);             /* and on the new method             */
  return newMethod;                    /* return the method                 */
}

RexxMethod *RexxSource::interpret(
    RexxString    *string,             /* interpret string value            */
    RexxDirectory *labels,             /* parent labels                     */
    size_t         line_number )       /* line number of interpret          */
/******************************************************************************/
/* Function:   Interpret a string in the current activation context           */
/******************************************************************************/
{
  RexxSource *source;                  /* created source object             */
  RexxMethod *method;                  /* new method for interpret          */

                                       /* create a source object            */
  source = (RexxSource *)save(new RexxSource (this->programName, new_array1(string)));
  source->interpretLine(line_number);  /* fudge the line numbering          */
                                       /* convert to executable form        */
  method = source->interpretMethod(labels);
  discard_hold(source);                /* release lock on the source        */
  return method;                       /* return this method                */
}

void RexxSource::checkDirective()
/******************************************************************************/
/* Function:  Verify that no code follows a directive except for more         */
/*            directive instructions.                                         */
/******************************************************************************/
{
  RexxToken *token;                    /* first token of the clause         */

  this->nextClause();                  /* get the next clause               */
  if (!(this->flags&no_clause)) {      /* have a next clause?               */
    token = nextReal();                /* get the first token               */
                                       /* not a directive start?            */
    if (token->classId != TOKEN_DCOLON)
                                       /* this is an error                  */
      report_error(Error_Translation_bad_directive);
    firstToken();                      /* reset to the first token          */
    this->reclaimClause();             /* give back to the source object    */
  }
}


/**
 * Test if a directive is followed by a body of Rexx code
 * instead of another directive or the end of the source.
 *
 * @return True if there is a non-directive clause following the current
 *         clause.
 */
bool RexxSource::hasBody()
{
    // assume there's no body here
    bool result = false;

    // if we have anything to look at, see if it is a directive or not.
    this->nextClause();
    if (!(this->flags&no_clause))
    {
        // we have a clause, now check if this is a directive or not
        RexxToken *token = nextReal();
        // not a "::", not a directive, which means we have real code to deal with
        result = token->classId != TOKEN_DCOLON;
        // reset this clause entirely so we can start parsing for real.
        firstToken();
        this->reclaimClause();
    }
    return result;
}


RexxObject *RexxSource::toss(
    RexxObject *object)                /* object to "release"               */
/******************************************************************************/
/* Function:  Remove an object from the save list                             */
/******************************************************************************/
{
  if (object != OREF_NULL) {           /* have a real object                */
    this->savelist->remove(object);    /* remove from the save table        */
    this->holdObject(object);          /* return this object as held        */
  }
  return object;                       /* return the object                 */
}

void RexxSource::cleanup()
/******************************************************************************/
/* Function:  Final cleanup after parsing                                     */
/******************************************************************************/
{
                                       /* global area cleanup               */
                                       /* release the holding pen           */
  OrefSet(this, this->holdstack, OREF_NULL);
                                       /* release any saved objects         */
  OrefSet(this, this->savelist, OREF_NULL);
  OrefSet(this, this->literals, OREF_NULL);
  OrefSet(this, this->strings, OREF_NULL);
  OrefSet(this, this->clause, OREF_NULL);
  OrefSet(this, this->control, OREF_NULL);
  OrefSet(this, this->terms, OREF_NULL);
  OrefSet(this, this->subTerms, OREF_NULL);
  OrefSet(this, this->operators, OREF_NULL);
  OrefSet(this, this->class_dependencies, OREF_NULL);
  OrefSet(this, this->active_class, OREF_NULL);
                                       /* now method parsing areas          */
  OrefSet(this, this->calls, OREF_NULL);
  OrefSet(this, this->variables, OREF_NULL);
  OrefSet(this, this->guard_variables, OREF_NULL);
  OrefSet(this, this->exposed_variables, OREF_NULL);
  OrefSet(this, this->labels, OREF_NULL);
  OrefSet(this, this->first, OREF_NULL);
  OrefSet(this, this->last, OREF_NULL);
  OrefSet(this, this->currentInstruction, OREF_NULL);
}

void RexxSource::mergeRequired(
    RexxSource *source)                /* source to merge into              */
/******************************************************************************/
/* Function:  Merge all public class and routine information from a called    */
/*            program into the full public information of this program.       */
/******************************************************************************/
{
  size_t i;                            /* loop index                        */

                                       /* have routines?                    */
  if (source->public_routines != OREF_NULL || source->merged_public_routines) {
                                       /* first merged attempt?             */
    if (this->merged_public_routines == OREF_NULL)
                                       /* get the directory                 */
      OrefSet(this, this->merged_public_routines, new_directory());
                                       /* do the merged set first           */
    if (source->merged_public_routines != OREF_NULL) {
                                       /* loop through the list of routines */
      for (i = source->merged_public_routines->first(); source->merged_public_routines->available(i); i = source->merged_public_routines->next(i)) {
                                       /* copy the routine over             */
        this->merged_public_routines->setEntry((RexxString *)source->merged_public_routines->index(i), source->merged_public_routines->value(i));
      }
    }
                                       /* have public routines              */
    if (source->public_routines != OREF_NULL) {
                                       /* loop through the list of routines */
      for (i = source->public_routines->first(); source->public_routines->available(i); i = source->public_routines->next(i)) {
                                       /* copy the routine over             */
        this->merged_public_routines->setEntry((RexxString *)source->public_routines->index(i), source->public_routines->value(i));
      }
    }
  }

                                       /* have classes also?                */
  if (source->installed_public_classes != OREF_NULL || source->merged_public_classes != OREF_NULL) {
                                       /* first merged attempt?             */
    if (this->merged_public_classes == OREF_NULL)
                                       /* get the directory                 */
      OrefSet(this, this->merged_public_classes, new_directory());
                                       /* do the merged classes first       */
    if (source->merged_public_classes != OREF_NULL) {
                                       /* loop through the list of classes, */
      for (i = source->merged_public_classes->first(); source->merged_public_classes->available(i); i = source->merged_public_classes->next(i)) {
                                       /* copy the routine over             */
        this->merged_public_classes->setEntry((RexxString *)source->merged_public_classes->index(i), source->merged_public_classes->value(i));
      }
    }
                                       /* have public classes?              */
    if (source->installed_public_classes != OREF_NULL) {
                                       /* loop through the list of classes, */
      for (i = source->installed_public_classes->first(); source->installed_public_classes->available(i); i = source->installed_public_classes->next(i)) {
                                       /* copy the routine over             */
        this->merged_public_classes->setEntry((RexxString *)source->installed_public_classes->index(i), source->installed_public_classes->value(i));
      }
    }
  }
}

RexxMethod *RexxSource::resolveRoutine(
    RexxString *routineName )          /* target routine name               */
/******************************************************************************/
/* Function:  Try to resolve a routine name from the information held by this */
/*            source object.                                                  */
/******************************************************************************/
{
  RexxMethod *routineObject;           /* resolved class object             */

  routineObject = OREF_NULL;           /* no routine found yet              */
  if (this->routines != OREF_NULL) {   /* have locally defined routines?    */
                                       /* try for a local one first         */
    routineObject = (RexxMethod *)(this->routines->entry(routineName));
    if (routineObject != OREF_NULL)    /* did this work?                    */
      return routineObject;            /* return now                        */
  }
                                       /* have publically defined routines? */
  if (this->merged_public_routines != OREF_NULL)
                                       /* try for a public one now          */
    routineObject = (RexxMethod *)(this->merged_public_routines->entry(routineName));
  return routineObject;                /* return the object                 */
}


RexxClass *RexxSource::resolveClass(
    RexxString     *className,         /* name of the target class          */
    RexxActivation *context )          /* the execution context             */
/******************************************************************************/
/* Function:  Try to resolve a class name from the information held by this   */
/*            source object.                                                  */
/******************************************************************************/
{
  RexxClass *classObject;              /* resolved class object             */
  RexxDirectory * securityArgs;        /* security check arguments          */
  RexxString *internalName;            /* upper cased class name            */

  internalName = className->upper();   /* upper case it                     */
  classObject = OREF_NULL;             /* default to not found              */
                                       /* have locally defined classes?     */
  if (this->installed_classes != OREF_NULL) {
                                       /* try for a local one first         */
    classObject = (RexxClass *)(this->installed_classes->fastAt(internalName));
    if (classObject != OREF_NULL)      /* did this work?                    */
      return classObject;              /* return now                        */
  }
                                       /* have publically defined classes?  */
  if (this->merged_public_classes != OREF_NULL) {
                                       /* try for a local one first         */
    classObject = (RexxClass *)(this->merged_public_classes->fastAt(internalName));
    if (classObject != OREF_NULL)      /* did this work?                    */
      return classObject;              /* return now                        */
  }

                                       /* normal execution?                 */
  if (this->securityManager == OREF_NULL) {
                                       /* send message to .local            */
    classObject = (RexxClass *)(((RexxDirectory *)(CurrentActivity->local))->at(internalName));
    if (classObject == OREF_NULL)      /* still not found?                  */
                                       /* last chance, try the environment  */
      classObject = (RexxClass *)(TheEnvironment->at(internalName));
    return classObject;                /* return the object                 */
  }
                                       /* protected execution               */
  securityArgs = new_directory();      /* get the information directory     */
                                       /* put the name in the directory     */
  securityArgs->put(internalName, OREF_NAME);
                                       /* did manager handle this?          */
  if (context->callSecurityManager(OREF_LOCAL, securityArgs))
                                       /* get the resolved name             */
    classObject = (RexxClass *)securityArgs->fastAt(OREF_RESULT);
  else
                                       /* send message to .local            */
    classObject = (RexxClass *)(((RexxDirectory *)(CurrentActivity->local))->at(internalName));
  if (classObject == OREF_NULL) {      /* still not found?                  */
                                       /* put the name in the directory     */
    securityArgs->put(internalName, OREF_NAME);
                                       /* did manager handle this?          */
    if (context->callSecurityManager(OREF_ENVIRONMENT, securityArgs))
                                       /* get the resolved name             */
      classObject = (RexxClass *)securityArgs->fastAt(OREF_RESULT);
    else
                                       /* last chance, try the environment  */
      classObject = (RexxClass *)(TheEnvironment->at(internalName));
  }
  return classObject;                  /* return the object                 */
}


void RexxSource::processInstall(
    RexxActivation *activation)        /* invoking activation               */
/******************************************************************************/
/* Function:  Process directive information contained within a method, calling*/
/*            all ::requires routines, creating all ::class methods, and      */
/*            processing all ::routines.                                      */
/******************************************************************************/
{
  RexxString    *this_required;        /* working ::requires item           */
  RexxString    *name;                 /* working name                      */
  RexxString    *metaclass_name;       /* class meta class                  */
  RexxString    *subclass_name;        /* class subclass                    */
  RexxArray     *inherits;             /* additional inheritance            */
  RexxDirectory *instanceMethods;      /* instance methods for a class      */
  RexxObject    *Public;               /* public flag                       */
  RexxString    *external;             /* external flag                     */
  RexxClass     *metaclass;            /* class metaclass                   */
  RexxClass     *subclass;             /* class subclass                    */
  RexxDirectory *class_methods;        /* class method list                 */
  RexxArray     *current_class;        /* current class information         */
  size_t         size;                 /* number of required objects        */
  size_t         i;                    /* loop counter                      */
  size_t         j;                    /* loop counter                      */
  RexxObject    *mixin;                /* MIXINCLASS flag                   */
  RexxString    *class_id;             /* class id name                     */
  RexxClass     *classObject;          /* created class                     */

                                       /* turn the install flag off         */
                                       /* immediately, otherwise we may     */
                                       /* run into a recursion problem      */
                                       /* when class init methods are       */
                                       /* processed                         */
  this->flags &= ~_install;            /* we are now installed              */

  if (this->requires != OREF_NULL) {   /* need to process ::requires?       */
                                       /* for each REQUIRES, call the       */
                                       /* program and merge in the public   */
                                       /* classes and routines              */
    size = this->requires->size();     /* get the number to install         */

    for (i = 1; i <= size; i += 2) {   /* loop through the requires names   */
                                       /* get the next required item        */
      this_required = (RexxString *)(this->requires->get(i));
                                       /* try to load this up               */
      activation->loadRequired(this_required, (RexxInstruction *)this->requires->get(i + 1));
    }
  }

  if (this->classes != OREF_NULL) {    /* have classes to process?          */
                                       /* get an installed classes directory*/
    OrefSet(this, this->installed_classes, new_directory());
                                       /* and the public classes            */
    OrefSet(this, this->installed_public_classes, new_directory());
    size = this->classes->size();      /* get the number of classes         */
    for (i = 1; i <= size; i++) {      /* process each class                */
                                       /* get the class info                */
      current_class = (RexxArray *)(this->classes->get(i));
                                       /* get the public (uppercase) name   */
      name = (RexxString *)(current_class->get(CLASS_PUBLIC_NAME));
                                       /* get the public flag               */
      Public = current_class->get(CLASS_PUBLIC);
                                       /* get the mixingclass flag          */
      mixin = current_class->get(CLASS_MIXINCLASS);
                                       /* and the external name             */
      external = (RexxString *)current_class->get(CLASS_EXTERNAL_NAME);
                                       /* the metaclass                     */
      metaclass_name = (RexxString *)(current_class->get(CLASS_METACLASS));
                                       /* and subclass                      */
      subclass_name = (RexxString *)(current_class->get(CLASS_SUBCLASS_NAME));
                                       /* set the current line for errors   */
      activation->setCurrent((RexxInstruction *)current_class->get(CLASS_DIRECTIVE));
                                       /* get the class id                  */
      class_id = (RexxString *)(current_class->get(CLASS_NAME));
      if (metaclass_name == OREF_NULL) /* no metaclass?                     */
        metaclass = OREF_NULL;         /* flag this                         */
      else {                           /* have a real metaclass             */
                                       /* resolve the class                 */
        metaclass = this->resolveClass(metaclass_name, activation);
        if (metaclass == OREF_NULL)    /* nothing found?                    */
                                       /* not found in environment, error!  */
          report_exception1(Error_Execution_nometaclass, metaclass_name);
      }

      if (subclass_name == OREF_NULL)  /* no subclass?                      */
                                       /* flag this                         */
        subclass = (RexxClass *)TheNilObject;
      else {                           /* have a real subclass              */
                                       /* resolve the class                 */
        subclass = this->resolveClass(subclass_name, activation);
        if (subclass == OREF_NULL)     /* nothing found?                    */
                                       /* not found in environment, error!  */
          report_exception1(Error_Execution_noclass, subclass_name);
      }
                                       /* get the inherits information      */
      inherits = (RexxArray *)(current_class->get(CLASS_INHERIT));
                                       /* instance methods                  */
      instanceMethods = (RexxDirectory *)(current_class->get(CLASS_METHODS));
                                       /* and class methods                 */
      class_methods = (RexxDirectory *)(current_class->get(CLASS_CLASS_METHODS));
      if (external != OREF_NULL) {     /* have an external name?            */
                                       /* import external class             */
        classObject = TheClassClass->external(external, metaclass, (RexxTable *)class_methods);
      }
      else {                           /* not imported                      */
        if (subclass == TheNilObject)  /* no subclass?                      */
                                       /* use .object for the subclass      */
          subclass = (RexxClass *)TheEnvironment->fastAt(OREF_OBJECTSYM);
        if (metaclass == TheNilObject) /* no metaclass?                     */
          metaclass = OREF_NULL;       /* just null out                     */
        if (mixin != OREF_NULL)        /* this a mixin class?               */
          classObject = (RexxClass *)(subclass->mixinclass(class_id, metaclass, (RexxTable *)class_methods));
        else
                                       /* doing a subclassing               */
          classObject = (RexxClass *)(subclass->subclass(class_id, metaclass, (RexxTable *)class_methods));
      }
                                       /* add the class to the directory    */
      this->installed_classes->put(classObject, name);
      if (inherits != OREF_NULL) {     /* have inherits to process?         */
                                       /* establish remainder of inheritance*/
        for (j = 1; j <= inherits->size(); j++) {
                                       /* get the next inherits name        */
          subclass_name = (RexxString *)(inherits->get(j));
                                       /* go resolve the entry              */
          subclass = this->resolveClass(subclass_name, activation);
          if (subclass == OREF_NULL)   /* not found?                        */
                                       /* not found in environment, error!  */
            report_exception1(Error_Execution_noclass, subclass_name);
                                       /* do the actual inheritance         */
          send_message1(classObject, OREF_INHERIT, subclass);
        }
      }
      if (instanceMethods != OREF_NULL)/* have instance methods to add?     */
                                       /* define them to the class object   */
        classObject->defineMethods((RexxTable *)instanceMethods);
                                       /* make public if required           */
      if (Public != OREF_NULL)         /* need to make this public?         */
                                       /* add to public directory           */
        this->installed_public_classes->setEntry(name, classObject);
    }
  }
}

RexxMethod *RexxSource::translate(
    RexxDirectory *labels)             /* interpret labels                  */
/******************************************************************************/
/* Function:  Translate a source object into a method object                  */
/******************************************************************************/
{
  RexxMethod *newMethod;               /* new parent method                 */

                                       /* go translate the lead block       */
  newMethod = this->translateBlock(labels);
  this->saveObject(newMethod);         /* make this safe                    */
  if (!this->atEnd()) {                /* have directives to process?       */
                                       /* create the routines directory     */
    OrefSet(this, this->routines, new_directory());
                                       /* create the routines directory     */
    OrefSet(this, this->public_routines, new_directory());
                                       /* and a directory of dependencies   */
    OrefSet(this, this->class_dependencies, new_directory());
                                       /* create the requires directory     */
    OrefSet(this, this->requires, (RexxArray *)new_list());
                                       /* create the classes list           */
    OrefSet(this, this->classes, (RexxArray *)new_list());
                                       /* no active class definition        */
    OrefSet(this, this->active_class, OREF_NULL);
    this->flags |= requires_allowed;   /* still allow ::REQUIRES directives */
                                       /* translation stopped by a directive*/
    if (this->flags&_interpret) {      /* is this an interpret?             */
      this->nextClause();              /* get the directive clause          */
                                       /* raise an error                    */
      report_error(Error_Translation_directive_interpret);
    }
                                       /* create a directory for ..methods  */
    OrefSet(this, this->methods, new_directory());

    while (!this->atEnd()) {           /* loop until end of source          */
      this->directive();               /* process the directive             */
    }
                                       /* have a class already active?      */
    if (this->active_class != OREF_NULL)
      this->completeClass();           /* finish off currently active one   */
    this->resolveDependencies();       /* go resolve class dependencies     */
  }
  return newMethod;                    /* return the method                 */
}

void RexxSource::resolveDependencies()
/*********************************************************************/
/* Function:  Resolve dependencies between ::CLASS directives,       */
/*            rearranging the order of the directives to preserve    */
/*            relative ordering wherever possible.  Classes with no  */
/*            dependencies in this source file will be done first,   */
/*            followed by those with dependencies in the appropriate */
/*            order                                                  */
/*********************************************************************/
{
  RexxArray     *class_order;          /* class ordering array              */
  RexxArray     *current_class;        /* current class definition          */
  RexxDirectory *dependencies;         /* dependencies list                 */
  RexxString    *metaclass_name;       /* name of the metaclass             */
  RexxString    *subclass_name;        /* name of the subclass              */
  RexxArray     *inherits;             /* set of inherits                   */
  RexxArray     *next_install;         /* next class to install             */
  RexxString    *class_name;           /* name of the class                 */
  RexxArray     *classes;              /* array of classes                  */
  size_t         size;                 /* count of classes                  */
  size_t         i;                    /* loop counter                      */
  size_t         j;                    /* loop counter                      */
  size_t         next_class;           /* next class position               */

                                       /* first convert class list to array */
  classes = ((RexxList *)(this->classes))->makeArray();
                                       /* and cast off the list             */
  OrefSet(this, this->classes, classes);
  size = classes->size();              /* get the array size                */
  if (size == 0)                       /* nothing to process?               */
                                       /* clear out the classes list        */
    OrefSet(this, this->classes, OREF_NULL);
  else {                               /* have classes to process           */
                                       /* now traverse the classes array,   */
    for (i = 1; i <= size; i++) {      /* building up a dependencies list   */
                                       /* get the next class                */
      current_class = (RexxArray *)(classes->get(i));
                                       /* get the dependencies              */
      dependencies = (RexxDirectory *)(this->class_dependencies->fastAt((RexxString *)current_class->get(CLASS_PUBLIC_NAME)));
                                       /* get the subclassing info          */
      metaclass_name = (RexxString *)(current_class->get(CLASS_METACLASS));
      subclass_name = (RexxString *)(current_class->get(CLASS_SUBCLASS_NAME));
      inherits = (RexxArray *)(current_class->get(CLASS_INHERIT));
                                       /* have a metaclass?                 */
      if (metaclass_name != OREF_NULL) {
                                       /* in our list to install?           */
        if (this->class_dependencies->entry(metaclass_name) != OREF_NULL)
                                       /* add to our pending list           */
          dependencies->setEntry(metaclass_name, metaclass_name);
      }
      if (subclass_name != OREF_NULL) {/* have a subclass?                  */
                                       /* in our list to install?           */
        if (this->class_dependencies->entry(subclass_name) != OREF_NULL)
                                       /* add to our pending list           */
          dependencies->setEntry(subclass_name, subclass_name);
      }
      if (inherits != OREF_NULL) {
                                       /* process each inherits             */
        for (j = 1; j <= inherits->size(); j++) {
                                       /* get the next class name           */
          subclass_name = (RexxString *)(inherits->get(j));
                                       /* in our list to install?           */
          if (this->class_dependencies->entry(subclass_name) != OREF_NULL)
                                       /* add to our pending list           */
            dependencies->setEntry(subclass_name, subclass_name);
        }
      }
    }
    class_order = new_array(size);     /* get the ordering array            */

/* now we repeatedly scan the pending directory looking for a class         */
/* with no in-program dependencies - it's an error if there isn't one       */
/* as we build the classes we have to remove them (their names) from        */
/* pending list and from the remaining dependencies                         */
    next_class = 1;                    /* inserting at the beginning        */
    while (next_class <= size) {       /* while still more to process       */
      next_install = OREF_NULL;        /* nothing located yet               */
      for (i = 1; i <= size; i++) {    /* loop through the list             */
                                       /* get the next class                */
        current_class = (RexxArray *)(classes->get(i));
        if (current_class == OREF_NULL)/* already processed?                */
          continue;                    /* go do the next one                */
                                       /* get the dependencies              */
        dependencies = (RexxDirectory *)(this->class_dependencies->fastAt((RexxString *)current_class->get(CLASS_PUBLIC_NAME)));
                                       /* no dependencies?                  */
        if (dependencies->items() == 0) {
          next_install = current_class;/* get the next one                  */
          break;                       /* go process this one               */
        }
      }
      if (next_install == OREF_NULL)   /* nothing located?                  */
                                       /* raise an error                    */
        report_error1(Error_Execution_cyclic, this->programName);
                                       /* get the class name                */
      class_name = (RexxString *)(current_class->get(CLASS_PUBLIC_NAME));
      for (j = 1; j <= size; j++) {    /* go remove the dependencies        */
                                       /* get a class                       */
        current_class = (RexxArray *)(classes->get(j));
                                       /* not installed yet?                */
        if (current_class != OREF_NULL) {
                                       /* get the dependencies list         */
          dependencies = (RexxDirectory *)(this->class_dependencies->fastAt((RexxString *)current_class->get(CLASS_PUBLIC_NAME)));
                                       /* remove from the dependencies      */
          dependencies->remove(class_name);
        }
      }
                                       /* insert into the install order     */
      class_order->put(next_install, next_class);
      next_class++;                    /* and step the position             */
      classes->put(OREF_NULL, i);      /* remove the installed one          */
    }
                                       /* replace the original class list   */
    OrefSet(this, this->classes, class_order);
                                       /* don't need the dependencies now   */
    OrefSet(this, this->class_dependencies, OREF_NULL);
  }
                                       /* convert requires list to an array */
  OrefSet(this, this->requires, this->requires->makeArray());
  if (this->requires->size() == 0)     /* nothing there?                    */
                                       /* just clear it out                 */
    OrefSet(this, this->requires, OREF_NULL);
  if (this->routines->items() == 0)    /* no routines to process?           */
                                       /* just clear it out also            */
    OrefSet(this, this->routines, OREF_NULL);
                                       /* now finally the public routines   */
  if (this->public_routines->items() == 0)
                                       /* just clear it out also            */
    OrefSet(this, this->public_routines, OREF_NULL);
  if (this->methods->items() == 0)     /* and also the methods directory    */
                                       /* just clear it out also            */
    OrefSet(this, this->methods, OREF_NULL);
}

void RexxSource::completeClass()
/*********************************************************************/
/* Function:  complete create of the active class definition         */
/*********************************************************************/
{
  RexxArray     *inherits;             /* the inherits information          */
  RexxDirectory *classMethods;         /* methods directory                 */

                                       /* get the inherits list             */
  inherits = (RexxArray *)(this->active_class->get(CLASS_INHERIT));
  if (inherits != OREF_NULL) {         /* have an inherits list?            */
                                       /* convert to an array               */
    inherits = ((RexxList *)inherits)->makeArray();
                                       /* replace the original list         */
    this->active_class->put(inherits, CLASS_INHERIT);
  }
                                       /* get the class methods             */
  classMethods = (RexxDirectory *)(this->active_class->get(CLASS_METHODS));
  if (classMethods->items() == 0)      /* have any methods?                 */
                                       /* through away the directory        */
    this->active_class->put(OREF_NULL, CLASS_METHODS);
                                       /* now repeat for class methods      */
  classMethods = (RexxDirectory *)(this->active_class->get(CLASS_CLASS_METHODS));

  if (classMethods->items() == 0)           /* have any methods?                 */
                                       /* throw away the directory          */
    this->active_class->put(OREF_NULL, CLASS_CLASS_METHODS);
}

#define DEFAULT_GUARD    0             /* using defualt guarding            */
#define GUARDED_METHOD   1             /* method is a guarded one           */
#define UNGUARDED_METHOD 2             /* method is unguarded               */

#define DEFAULT_PROTECTION 0           /* using defualt protection          */
#define PROTECTED_METHOD   1           /* security manager permission needed*/
#define UNPROTECTED_METHOD 2           /* no protection.                    */

#define DEFAULT_ACCESS_SCOPE      0    /* using defualt scope               */
#define PUBLIC_SCOPE       1           /* publicly accessible               */
#define PRIVATE_SCOPE      2           /* private scope                     */

/**
 * Process a ::CLASS directive for a source file.
 */
void RexxSource::classDirective()
{
    RexxToken    *token;                 /* current token under processing    */
    /* have a class already active?      */
    if (this->active_class != OREF_NULL)
    {
        this->completeClass();         /* go finish this up                 */
    }

    this->flags &= ~requires_allowed;/* ::REQUIRES no longer valid        */
    token = nextReal();              /* get the next token                */
                                     /* not a symbol or a string          */
    if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
    {
        /* report an error                   */
        report_error(Error_Symbol_or_string_class);
    }
    RexxString *name = token->value;             /* get the routine name              */
                                     /* get the exposed name version      */
    RexxString *public_name = this->commonString(name->upper());
    /* does this already exist?          */
    if (this->class_dependencies->entry(public_name) != OREF_NULL)
    {
        /* have an error here                */
        report_error(Error_Translation_duplicate_class);
    }
    /* create a dependencies list        */
    this->class_dependencies->put(new_directory(), public_name);
    this->flags |= _install;         /* have information to install       */
                                     /* create the class definition       */
    OrefSet(this, this->active_class, new_array(CLASS_INFO_SIZE));
    /* add this to the class table       */
    ((RexxList *)(this->classes))->addLast(this->active_class);
    /* add the name to the information   */
    this->active_class->put(name, CLASS_NAME);
    /* add the name to the information   */
    this->active_class->put(public_name, CLASS_PUBLIC_NAME);
    /* create the method table           */
    this->active_class->put(new_directory(), CLASS_METHODS);
    /* and the class method tabel        */
    this->active_class->put(new_directory(), CLASS_CLASS_METHODS);
    /* save the ::class location         */
    this->active_class->put((RexxObject *)new RexxInstruction(this->clause, KEYWORD_CLASS), CLASS_DIRECTIVE);
    int  Public = DEFAULT_ACCESS_SCOPE;          /* haven't seen the keyword yet      */
    bool subclass = false;                /* no subclass keyword yet           */
    RexxString *externalname = OREF_NULL; /* no external name yet              */
    RexxString *metaclass = OREF_NULL;    /* no metaclass yet                  */
    for (;;)
    {                       /* now loop on the option keywords   */
        token = nextReal();            /* get the next token                */
                                       /* reached the end?                  */
        if (token->classId == TOKEN_EOC)
            break;                       /* get out of here                   */
                                         /* not a symbol token?               */
        else if (token->classId != TOKEN_SYMBOL)
            /* report an error                   */
            report_error_token(Error_Invalid_subkeyword_class, token);
        else
        {                         /* have some sort of option keyword  */
                                  /* get the keyword type              */
            int type = this->subDirective(token);
            switch (type)
            {              /* process each sub keyword          */
                    /* ::CLASS name METACLASS metaclass  */
                case SUBDIRECTIVE_METACLASS:
                    /* already had a METACLASS?          */
                    if (metaclass != OREF_NULL)
                        report_error_token(Error_Invalid_subkeyword_class, token);
                    token = nextReal();      /* get the next token                */
                                             /* not a symbol or a string          */
                    if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
                        /* report an error                   */
                        report_error_token(Error_Symbol_or_string_metaclass, token);
                    metaclass = token->value;/* external name is token value      */
                                             /* tag the active class              */
                    this->active_class->put(metaclass, CLASS_METACLASS);
                    break;


                case SUBDIRECTIVE_PUBLIC:  /* ::CLASS name PUBLIC               */
                    if (Public != DEFAULT_ACCESS_SCOPE)  /* already had one of these?         */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_class, token);
                    }
                    Public = PUBLIC_SCOPE;   /* turn on the seen flag             */
                                             /* just set this as a public object  */
                    this->active_class->put((RexxObject *)TheTrueObject, CLASS_PUBLIC);
                    break;

                case SUBDIRECTIVE_PRIVATE: /* ::CLASS name PUBLIC               */
                    if (Public != DEFAULT_ACCESS_SCOPE)  /* already had one of these?         */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_class, token);
                    }
                    Public = PRIVATE_SCOPE;  /* turn on the seen flag             */
                    break;
                    /* ::CLASS name SUBCLASS sclass      */
                case SUBDIRECTIVE_SUBCLASS:
                    if (subclass)            /* already had one of these?         */
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_class, token);
                    subclass = true;         /* turn on the seen flag             */
                    token = nextReal();      /* get the next token                */
                                             /* not a symbol or a string          */
                    if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
                        /* report an error                   */
                        report_error(Error_Symbol_or_string_subclass);
                    /* set the subclass information      */
                    this->active_class->put(token->value, CLASS_SUBCLASS_NAME);
                    break;
                    /* ::CLASS name MIXINCLASS mclass    */
                case SUBDIRECTIVE_MIXINCLASS:
                    if (subclass)            /* already had one of these?         */
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_class, token);
                    subclass = true;         /* turn on the seen flag             */
                    token = nextReal();      /* get the next token                */
                                             /* not a symbol or a string          */
                    if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
                        /* report an error                   */
                        report_error(Error_Symbol_or_string_mixinclass);
                    /* set the subclass information      */
                    this->active_class->put(token->value, CLASS_SUBCLASS_NAME);
                    /* this a mixin?                     */
                    if (type == SUBDIRECTIVE_MIXINCLASS)
                        /* just set this as a public object  */
                        this->active_class->put((RexxObject *)TheTrueObject, CLASS_MIXINCLASS);
                    break;
                    /* ::CLASS name INHERIT iclasses     */
                case SUBDIRECTIVE_INHERIT:
                    token = nextReal();      /* get the next token                */
                                             /* nothing after the keyword?        */
                    if (token->classId == TOKEN_EOC)
                        /* report an error                   */
                        report_error_token(Error_Symbol_or_string_inherit, token);
                    /* add an inherits list              */
                    this->active_class->put(new_list(), CLASS_INHERIT);
                    while (token->classId != TOKEN_EOC)
                    {
                        /* not a symbol or a string          */
                        if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
                            /* report an error                   */
                            report_error_token(Error_Symbol_or_string_inherit, token);
                        /* add to the inherit list           */
                        ((RexxList *)(this->active_class->get(CLASS_INHERIT)))->addLast(token->value);
                        token = nextReal();    /* step to the next token            */
                    }
                    previousToken();         /* step back a token                 */
                    break;

                default:                   /* invalid keyword                   */
                    /* this is an error                  */
                    report_error_token(Error_Invalid_subkeyword_class, token);
                    break;
            }
        }
    }
}


/**
 * Process a ::METHOD directive in a source file.
 */
void RexxSource::methodDirective()
{
    this->flags &= ~requires_allowed;/* ::REQUIRES no longer valid        */
    int  Private = DEFAULT_ACCESS_SCOPE;    /* this is a public method           */
    int  Protected = DEFAULT_PROTECTION;  /* and is not protected yet          */
    int guard = DEFAULT_GUARD;       /* default is guarding               */
    bool Class = false;              /* default is an instance method     */
    bool Attribute = false;          /* init Attribute flag               */
    bool abstractMethod = false;     // this is an abstract method
    RexxToken *token = nextReal();   /* get the next token                */
    RexxString *externalname = OREF_NULL;       /* not an external method yet        */

                                     /* not a symbol or a string          */
    if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
    {
        /* report an error                   */
        report_error_token(Error_Symbol_or_string_method, token);
    }
    RexxString *name = token->value; /* get the string name               */
                                     /* and the name form also            */
    RexxString *internalname = this->commonString(name->upper());
    for (;;)
    {                       /* now loop on the option keywords   */
        token = nextReal();            /* get the next token                */
                                       /* reached the end?                  */
        if (token->classId == TOKEN_EOC)
        {
            break;                       /* get out of here                   */
        }
                                         /* not a symbol token?               */
        else if (token->classId != TOKEN_SYMBOL)
        {
            /* report an error                   */
            report_error_token(Error_Invalid_subkeyword_method, token);
        }
        else
        {                         /* have some sort of option keyword  */
                                  /* process each sub keyword          */
            switch (this->subDirective(token))
            {
                /* ::METHOD name CLASS               */
                case SUBDIRECTIVE_CLASS:
                    if (Class)               /* had one of these already?         */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    Class = true;            /* flag this for later processing    */
                    break;
                    /* ::METHOD name EXTERNAL extname    */
                case SUBDIRECTIVE_EXTERNAL:
                    /* already had an external?          */
                    if (externalname != OREF_NULL || abstractMethod || Attribute)
                    {
                        /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    if (Attribute)           /* ATTRIBUTE already specified ?     */
                                             /* EXTERNAL and ATTRIBUTE are        */
                                             /* mutually exclusive                */
                    {
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
                    {
                        /* report an error                   */
                        report_error_token(Error_Symbol_or_string_requires, token);
                    }
                    externalname = token->value;
                    break;
                    /* ::METHOD name PRIVATE             */
                case SUBDIRECTIVE_PRIVATE:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    Private = PRIVATE_SCOPE;           /* flag for later processing         */
                    break;
                    /* ::METHOD name PUBLIC             */
                case SUBDIRECTIVE_PUBLIC:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    Private = PUBLIC_SCOPE;        /* flag for later processing         */
                    break;
                    /* ::METHOD name PROTECTED           */
                case SUBDIRECTIVE_PROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    Protected = PROTECTED_METHOD;        /* flag for later processing         */
                    break;
                    /* ::METHOD name UNPROTECTED           */
                case SUBDIRECTIVE_UNPROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    Protected = UNPROTECTED_METHOD;      /* flag for later processing         */
                    break;
                    /* ::METHOD name UNGUARDED           */
                case SUBDIRECTIVE_UNGUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    guard = UNGUARDED_METHOD;/* flag for later processing         */
                    break;
                    /* ::METHOD name GUARDED             */
                case SUBDIRECTIVE_GUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    guard = GUARDED_METHOD;  /* flag for later processing         */
                    break;
                    /* ::METHOD name ATTRIBUTE           */
                case SUBDIRECTIVE_ATTRIBUTE:

                    if (Attribute)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    /* EXTERNAL already specified ?      */
                    if (externalname != OREF_NULL || abstractMethod)
                    {
                        /* EXTERNAL and ATTRIBUTE are        */
                        /* mutually exclusive                */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    Attribute = true;        /* flag for later processing         */
                    break;

                                           /* ::METHOD name ABSTRACT            */
                case SUBDIRECTIVE_ABSTRACT:

                    if (abstractMethod || externalname != OREF_NULL)
                    {
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    // not compatible with ATTRIBUTE or EXTERNAL
                    if (externalname != OREF_NULL || Attribute)
                    {
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    abstractMethod = true;   /* flag for later processing         */
                    break;


                default:                   /* invalid keyword                   */
                    /* this is an error                  */
                    report_error_token(Error_Invalid_subkeyword_method, token);
                    break;
            }
        }
    }

    RexxDirectory *methodsDir;
    /* no previous ::CLASS directive?    */
    if (this->active_class == OREF_NULL)
    {
        if (Class)                   /* supposed to be a class method?    */
        {
                                     /* this is an error                  */
            report_error(Error_Translation_missing_class);
        }
        methodsDir = this->methods;  /* adding to the global set          */
    }
    else
    {                                /* add the method to the active class*/
        if (Class)                   /* class method?                     */
        {
                                     /* add to the class method list      */
            methodsDir = ((RexxDirectory *)(this->active_class->get(CLASS_CLASS_METHODS)));
        }
        else
        {
            /* add to the method list            */
            methodsDir = ((RexxDirectory *)(this->active_class->get(CLASS_METHODS)));
        }
    }
    /* duplicate method name?            */
    if (methodsDir->entry(internalname) != OREF_NULL)
    {
        /* this is an error                  */
        report_error(Error_Translation_duplicate_method);
    }

    RexxMethod *method;
    // is this an attribute method?
    if (Attribute)
    {
        // now get a variable retriever to get the property
        RexxVariableBase *retriever = this->getRetriever(name);

        // create the method pair and quit.
        createAttributeGetterMethod(methodsDir, internalname, retriever, Private == PRIVATE_SCOPE,
            Protected == PROTECTED_METHOD, guard == GUARDED_METHOD);
        // now get this as the setter method.
        internalname = commonString(internalname->concatWithCstring("="));
        // make sure we don't have one of these define already.
        if (methodsDir->entry(internalname) != OREF_NULL)
        {
            /* this is an error                  */
            report_error(Error_Translation_duplicate_method);
        }
        createAttributeSetterMethod(methodsDir, internalname, retriever, Private == PRIVATE_SCOPE,
            Protected == PROTECTED_METHOD, guard == GUARDED_METHOD);
        return;
    }
    // abstract method?
    else if (abstractMethod)
    {
                                       /* Go check the next clause to make  */
        this->checkDirective();        /* sure that no code follows         */
        method = new_method(abstractIndex, CPPM(RexxObject::abstractMethod), A_COUNT, OREF_NULL);
    }
    /* not an external method?           */
    else if (externalname == OREF_NULL)
    {
        /* go do the next block of code      */
        method = this->translateBlock(OREF_NULL);
    }
    else
    {                           /* have an external method           */
        /* convert external into words       */
        RexxArray *words = this->words(externalname);
        /* not 'LIBRARY library entry' form? */
        if (((RexxString *)(words->get(1)))->strCompare(CHAR_LIBRARY))
        {
            if (words->size() != 3)      /* wrong number of tokens?           */
            {
                                         /* this is an error                  */
                report_error1(Error_Translation_bad_external, externalname);
            }

            /* go check the next clause to make  */
            this->checkDirective();      /* sure no code follows              */
                                         /* create a new native method        */
            RexxNativeCode *nmethod = new_nmethod((RexxString *)(words->get(3)), (RexxString *)(words->get(2)));
            /* turn into a real method object    */
            method = new_method(0, (PCPPM)NULL, 0, (RexxObject *)nmethod);
        }
        /* is it 'REXX entry' form?          */
        else if (((RexxString *)(words->get(1)))->strCompare(CHAR_REXX))
        {
            if (words->size() != 2)      /* wrong number of tokens?           */
            {
                                         /* this is an error                  */
                report_error1(Error_Translation_bad_external, externalname);
            }

            /* go check the next clause to make  */
            this->checkDirective();      /* sure no code follows              */
                                         /* point to the entry name           */
            char *entryName = ((RexxString *)(words->get(2)))->stringData;
            /* loop through the internal table   */
            size_t index = 0;
            for (; internalMethodTable[index].entryName != NULL; index++)
            {
                if (strcmp(entryName, internalMethodTable[index].entryName) == 0)
                {
                    break;                   /* get out                           */
                }
            }
            /* name not found?                   */
            if (internalMethodTable[index].entryName == NULL)
            {
                /* this is an error                  */
                report_error1(Error_Translation_bad_external, externalname);
            }
            /* create a new native method        */
            RexxNativeCode *nmethod = new RexxNativeCode(OREF_NULL, OREF_NULL, NULL, index);
            /* turn into a real method object    */
            method = new_method(0, (PCPPM)NULL, 0, (RexxObject *)nmethod);
        }
        else
        {
            /* unknown external type             */
            report_error1(Error_Translation_bad_external, externalname);
        }
    }
    if (Private == PRIVATE_SCOPE)                   /* is this a private method?         */
    {
        method->setPrivate();        /* turn on the private attribute     */
    }
    if (Protected == PROTECTED_METHOD)   /* is this a protected method?       */
    {
        method->setProtected();      /* turn on the protected attribute   */
    }
    if (guard == UNGUARDED_METHOD) /* is this unguarded?                */
    {
        method->setUnGuarded();      /* turn on the unguarded attribute   */
    }

    /* add the method to the table       */
    methodsDir->put(method, internalname);
}

#define ATTRIBUTE_BOTH 0
#define ATTRIBUTE_GET  1
#define ATTRIBUTE_SET  2


/**
 * Process a ::ATTRIBUTE directive in a source file.
 */
void RexxSource::attributeDirective()
{
    this->flags &= ~requires_allowed;/* ::REQUIRES no longer valid        */
    int  Private = DEFAULT_ACCESS_SCOPE;    /* this is a public method           */
    int  Protected = DEFAULT_PROTECTION;  /* and is not protected yet          */
    int  guard = DEFAULT_GUARD;       /* default is guarding               */
    int  style = ATTRIBUTE_BOTH;      // by default, we create both methods for the attribute.
    bool Class = false;              /* default is an instance method     */
    RexxToken *token = nextReal();   /* get the next token                */

                                     /* not a symbol or a string          */
    if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
    {
        /* report an error                   */
        report_error_token(Error_Symbol_or_string_method, token);
    }
    RexxString *name = token->value; /* get the string name               */
                                     /* and the name form also            */
    RexxString *internalname = this->commonString(name->upper());
    for (;;)
    {                       /* now loop on the option keywords   */
        token = nextReal();            /* get the next token                */
                                       /* reached the end?                  */
        if (token->classId == TOKEN_EOC)
        {
            break;                       /* get out of here                   */
        }
                                         /* not a symbol token?               */
        else if (token->classId != TOKEN_SYMBOL)
        {
            /* report an error                   */
            report_error_token(Error_Invalid_subkeyword_method, token);
        }
        else
        {                         /* have some sort of option keyword  */
                                  /* process each sub keyword          */
            switch (this->subDirective(token))
            {
                case SUBDIRECTIVE_GET:
                    // only one of GET/SET allowed
                    if (style != ATTRIBUTE_BOTH)
                    {
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    style = ATTRIBUTE_GET;
                    break;

                case SUBDIRECTIVE_SET:
                    // only one of GET/SET allowed
                    if (style != ATTRIBUTE_BOTH)
                    {
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    style = ATTRIBUTE_SET;
                    break;


                /* ::ATTRIBUTE name CLASS               */
                case SUBDIRECTIVE_CLASS:
                    if (Class)               /* had one of these already?         */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    Class = true;            /* flag this for later processing    */
                    break;
                case SUBDIRECTIVE_PRIVATE:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    Private = PRIVATE_SCOPE;           /* flag for later processing         */
                    break;
                    /* ::METHOD name PUBLIC             */
                case SUBDIRECTIVE_PUBLIC:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    Private = PUBLIC_SCOPE;        /* flag for later processing         */
                    break;
                    /* ::METHOD name PROTECTED           */
                case SUBDIRECTIVE_PROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    Protected = PROTECTED_METHOD;        /* flag for later processing         */
                    break;
                case SUBDIRECTIVE_UNPROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    Protected = UNPROTECTED_METHOD;      /* flag for later processing         */
                    break;
                    /* ::METHOD name UNGUARDED           */
                case SUBDIRECTIVE_UNGUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    guard = UNGUARDED_METHOD;/* flag for later processing         */
                    break;
                    /* ::METHOD name GUARDED             */
                case SUBDIRECTIVE_GUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        report_error_token(Error_Invalid_subkeyword_method, token);
                    }
                    guard = GUARDED_METHOD;  /* flag for later processing         */
                    break;
                    /* ::METHOD name ATTRIBUTE           */


                default:                   /* invalid keyword                   */
                    /* this is an error                  */
                    report_error_token(Error_Invalid_subkeyword_method, token);
                    break;
            }
        }
    }

    RexxDirectory *methodsDir;
    // now figure out which dictionary we need to add the methods to.
    if (this->active_class == OREF_NULL)
    {
        if (Class)                   /* supposed to be a class method?    */
        {
                                     /* this is an error                  */
            report_error(Error_Translation_missing_class);
        }
        methodsDir = this->methods;  /* adding to the global set          */
    }
    else
    {                                /* add the method to the active class*/
        if (Class)                   /* class method?                     */
        {
                                     /* add to the class method list      */
            methodsDir = ((RexxDirectory *)(this->active_class->get(CLASS_CLASS_METHODS)));
        }
        else
        {
            /* add to the method list            */
            methodsDir = ((RexxDirectory *)(this->active_class->get(CLASS_METHODS)));
        }
    }

    // both attributes same default properties?

    // now get a variable retriever to get the property (do this before checking the body
    // so errors get diagnosed on the correct line),
    RexxVariableBase *retriever = this->getRetriever(name);

    switch (style)
    {
        case ATTRIBUTE_BOTH:
        {
            // make sure we don't have one of these define already.
            if (methodsDir->entry(internalname) != OREF_NULL)
            {
                /* this is an error                  */
                report_error(Error_Translation_duplicate_attribute);
            }
            // create the method pair and quit.
            createAttributeGetterMethod(methodsDir, internalname, retriever, Private == PRIVATE_SCOPE,
                Protected == PROTECTED_METHOD, guard == GUARDED_METHOD);
            // now get this as the setter method.
            internalname = commonString(internalname->concatWithCstring("="));
            // make sure we don't have one of these define already.
            if (methodsDir->entry(internalname) != OREF_NULL)
            {
                /* this is an error                  */
                report_error(Error_Translation_duplicate_attribute);
            }
            createAttributeSetterMethod(methodsDir, internalname, retriever, Private == PRIVATE_SCOPE,
                Protected == PROTECTED_METHOD, guard == GUARDED_METHOD);
            break;

        }

        case ATTRIBUTE_GET:       // just the getter method
        {
            // make sure we don't have one of these define already.
            if (methodsDir->entry(internalname) != OREF_NULL)
            {
                /* this is an error                  */
                report_error(Error_Translation_duplicate_attribute);
            }

            if (hasBody())
            {
                createMethod(methodsDir, internalname, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard == GUARDED_METHOD);
            }
            else
            {
                createAttributeGetterMethod(methodsDir, internalname, retriever, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard == GUARDED_METHOD);
            }
            break;
        }

        case ATTRIBUTE_SET:
        {
            // now get this as the setter method.
            internalname = commonString(internalname->concatWithCstring("="));
            // make sure we don't have one of these define already.
            if (methodsDir->entry(internalname) != OREF_NULL)
            {
                /* this is an error                  */
                report_error(Error_Translation_duplicate_attribute);
            }
            if (hasBody())        // just the getter method
            {
                createMethod(methodsDir, internalname, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard == GUARDED_METHOD);
            }
            else
            {
                createAttributeSetterMethod(methodsDir, internalname, retriever, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard == GUARDED_METHOD);
            }
            break;
        }
    }
}


/**
 * Create a Rexx method body.
 *
 * @param target The target method directory.
 * @param name   The name of the attribute.
 * @param privateMethod
 *               The method's private attribute.
 * @param protectedMethod
 *               The method's protected attribute.
 * @param guardedMethod
 *               The method's guarded attribute.
 */
void RexxSource::createMethod(RexxDirectory *target, RexxString *name,
    bool privateMethod, bool protectedMethod, bool guardedMethod)
{
    // translate the method block
    RexxMethod *method = translateBlock(OREF_NULL);

    // set the method properties
    if (privateMethod)
    {
        method->setPrivate();
    }
    if (protectedMethod)
    {
        method->setProtected();
    }
    if (guardedMethod)
    {
        method->setUnGuarded();
    }
    // and finally add to the target method directory.
    target->put(method, name);
}


/**
 * Create an ATTRIBUTE "get" method.
 *
 * @param target The target method directory.
 * @param name   The name of the attribute.
 * @param privateMethod
 *               The method's private attribute.
 * @param protectedMethod
 *               The method's protected attribute.
 * @param guardedMethod
 *               The method's guarded attribute.
 */
void RexxSource::createAttributeGetterMethod(RexxDirectory *target, RexxString *name, RexxVariableBase *retriever,
    bool privateMethod, bool protectedMethod, bool guardedMethod)
{
    // no code can follow the automatically generated methods
    this->checkDirective();

    // create the kernel method for the accessor
    RexxMethod *method = new_method(getAttributeIndex, CPPM(RexxObject::getAttribute), 0, OREF_NULL);

    if (privateMethod)
    {
        method->setPrivate();
    }
    if (protectedMethod)
    {
        method->setProtected();
    }
    if (guardedMethod)
    {
        method->setUnGuarded();
    }
    // set the the retriever as the attribute
    method->setAttribute(retriever);

    // and finally add to the target method directory.
    target->put(method, name);
}


/**
 * Create an ATTRIBUTE "set" method.
 *
 * @param target The target method directory.
 * @param name   The name of the attribute.
 * @param privateMethod
 *               The method's private attribute.
 * @param protectedMethod
 *               The method's protected attribute.
 * @param guardedMethod
 *               The method's guarded attribute.
 */
void RexxSource::createAttributeSetterMethod(RexxDirectory *target, RexxString *name, RexxVariableBase *retriever,
    bool privateMethod, bool protectedMethod, bool guardedMethod)
{
    // no code can follow the automatically generated methods
    this->checkDirective();

    // create the kernel method for the accessor
    RexxMethod *method = new_method(setAttributeIndex, CPPM(RexxObject::setAttribute), 1, OREF_NULL);

    if (privateMethod)
    {
        method->setPrivate();
    }
    if (protectedMethod)
    {
        method->setProtected();
    }
    if (guardedMethod)
    {
        method->setUnGuarded();
    }
    // set the the retriever as the attribute
    method->setAttribute(retriever);

    // and finally add to the target method directory.
    target->put(method, name);
}


/**
 * Process a ::routine directive in a source file.
 */
void RexxSource::routineDirective()
{
    this->flags &= ~requires_allowed;/* ::REQUIRES no longer valid        */
    RexxToken *token = nextReal();   /* get the next token                */
                                     /* not a symbol or a string          */
    if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
                                     /* report an error                   */
      report_error_token(Error_Symbol_or_string_routine, token);
    RexxString *name = token->value; /* get the routine name              */
                                     /* does this already exist?          */
    if (this->routines->entry(name) != OREF_NULL)
                                     /* have an error here                */
      report_error(Error_Translation_duplicate_routine);
    this->flags |= _install;         /* have information to install       */
    RexxString *externalname = OREF_NULL;        /* no external name yet              */
    int Public = DEFAULT_ACCESS_SCOPE;      /* not a public routine yet          */
    for (;;) {                       /* now loop on the option keywords   */
      token = nextReal();            /* get the next token                */
                                     /* reached the end?                  */
      if (token->classId == TOKEN_EOC)
        break;                       /* get out of here                   */
                                     /* not a symbol token?               */
      else if (token->classId != TOKEN_SYMBOL)
                                     /* report an error                   */
        report_error_token(Error_Invalid_subkeyword_routine, token);
                                     /* process each sub keyword          */
      switch (this->subDirective(token)) {
#if 0
                                     /* ::ROUTINE name EXTERNAL []*/
        case SUBDIRECTIVE_EXTERNAL:
                                     /* already had an external?          */
          if (externalname != OREF_NULL)
                                     /* duplicates are invalid            */
            report_error_token(Error_Invalid_subkeyword_class, token);
          token = nextReal();        /* get the next token                */
                                   /* not a string?                     */
          if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
          {
              /* report an error                   */
              report_error_token(Error_Symbol_or_string_requires, token);
          }
                               /* external name is token value      */
          externalname = token->value;
          break;
#endif
                                     /* ::ROUTINE name PUBLIC             */
        case SUBDIRECTIVE_PUBLIC:
          if (Public != DEFAULT_ACCESS_SCOPE)   /* already had one of these?         */
          {
                                     /* duplicates are invalid            */
              report_error_token(Error_Invalid_subkeyword_routine, token);

          }
          Public = PUBLIC_SCOPE;     /* turn on the seen flag             */
          break;
                                     /* ::ROUTINE name PUBLIC             */
        case SUBDIRECTIVE_PRIVATE:
          if (Public != DEFAULT_ACCESS_SCOPE)   /* already had one of these?         */
          {
                                     /* duplicates are invalid            */
              report_error_token(Error_Invalid_subkeyword_routine, token);

          }
          Public = PRIVATE_SCOPE;    /* turn on the seen flag             */
          break;

        default:                     /* invalid keyword                   */
                                     /* this is an error                  */
          report_error_token(Error_Invalid_subkeyword_routine, token);
          break;
      }
    }
    {
        RexxMethod *method = OREF_NULL;

        this->saveObject(name);          /* protect the name                  */

        if (externalname != OREF_NULL)   /* have an external routine?         */
        {
#if 0
                                    /* convert external into words       */
            RexxArray *words = this->words(externalname);
            /* not 'PACKAGE library [entry]' form? */
            if (((RexxString *)(words->get(1)))->strCompare(CHAR_PACKAGE))
            {
                RexxString *package;
                // the default entry point name is the internal name
                RexxString *entry = name;

                // full library with entry name version?
                if (words->size() == 3)
                {
                    package = (RexxString *)words->get(2);
                    entry = (RexxString *)words->get(3);
                }
                else if (words->size() == 2)
                {
                    package = (RexxString *)words->get(2);
                }
                else  // wrong number of tokens
                {
                                             /* this is an error                  */
                    report_error(Error_Translation_bad_external, externalname);
                }

                /* go check the next clause to make  */
                this->checkDirective();      /* sure no code follows              */
                                             /* create a new native method        */
                RexxNativeCode *nmethod = ThePackageManager->resolveNativeFunction(name, package, entry);
                // NEED to figure this out....
                /* turn into a real method object    */
                method = new RexxMethod(nmethod);
                // attach this to the source
                method->setSource(this);
            }
            else
                /* unknown external type             */
                report_error(Error_Translation_bad_external, externalname);
#endif
        }
        else
        {
                                         /* go do the next block of code      */
          method = this->translateBlock(OREF_NULL);
                                         /* add to the routine directory      */
          this->routines->setEntry(name, method);
          if (Public == PUBLIC_SCOPE)    /* a public routine?                 */
          {
                                         /* add to the public directory too   */
              this->public_routines->setEntry(name, method);

          }
        }
        this->toss(name);                /* release the "Gary Cole" (GC) lock */
    }
}

/**
 * Process a ::REQUIRES directive.
 */
void RexxSource::requiresDirective()
{
    /* no longer valid?                  */
    if (!(this->flags&requires_allowed))
    {
        /* this is an error                  */
        report_error(Error_Translation_requires);
    }
    RexxToken *token = nextReal();   /* get the next token                */
                                     /* not a symbol or a string          */
    if (token->classId != TOKEN_SYMBOL && token->classId != TOKEN_LITERAL)
    {
        /* report an error                   */
        report_error_token(Error_Symbol_or_string_requires, token);
    }
    this->flags |= _install;         /* have information to install       */
    RexxString *name = token->value; /* get the requires name             */
    RexxString *internalname = name; /* get the name form                 */
    token = nextReal();              /* get the next token                */
    if (token->classId != TOKEN_EOC) /* something appear after this?      */
    {
                                     /* this is a syntax error            */
        report_error_token(Error_Invalid_subkeyword_requires, token);
    }
    /* add this name to the table        */
    ((RexxList *)(this->requires))->addLast(internalname);
    /* save the ::requires location      */
    ((RexxList *)(this->requires))->addLast((RexxObject *)new RexxInstruction(this->clause, KEYWORD_REQUIRES));
}


void RexxSource::directive()
/********************************************************************/
/* Function:  parse a directive statement                           */
/********************************************************************/
{
    RexxToken    *token;                 /* current token under processing    */

    this->nextClause();                  /* get the directive clause          */
    if (this->flags&no_clause)           /* reached the end?                  */
        return;                          /* all finished                      */
    token = nextReal();                  /* skip the leading ::               */
    if (token->classId != TOKEN_DCOLON)  /* reached the end of a block?       */
                                         /* have an error here                */
        report_error(Error_Translation_bad_directive);
    token = nextReal();                  /* get the keyword token             */
    if (token->classId != TOKEN_SYMBOL)  /* not a symbol?                     */
                                         /* have an error here                */
        report_error(Error_Symbol_expected_directive);

    switch (this->keyDirective(token))
    { /* match against the directive list  */

        case DIRECTIVE_CLASS:              /* ::CLASS directive                 */
            classDirective();
            break;

        case DIRECTIVE_METHOD:             /* ::METHOD directive                */
            methodDirective();
            break;

        case DIRECTIVE_ROUTINE:            /* ::ROUTINE directive               */
            routineDirective();
            break;

        case DIRECTIVE_REQUIRES:           /* ::REQUIRES directive              */
            requiresDirective();
            break;

        case DIRECTIVE_ATTRIBUTE:          /* ::ATTRIBUTE directive             */
            attributeDirective();
            break;

        default:                           /* unknown directive                 */
            report_error(Error_Translation_bad_directive);
            break;
    }
}


void RexxSource::flushControl(
    RexxInstruction *instruction)      /* next instruction                  */
/******************************************************************************/
/* Function:  Flush any pending compound instructions from the control stack  */
/*            for new added instructions                                      */
/******************************************************************************/
{
  INT              type;               /* instruction type                  */
  RexxInstruction *second;             /* additional created instructions   */

  for (;;) {                           /* loop through the control stack    */
    type = this->topDo()->getType();   /* get the instruction type          */
    if (type == KEYWORD_ELSE) {        /* pending ELSE close?               */
      second = this->popDo();          /* pop the item off of the control   */
                                       /* create a new end marker           */
      second = this->endIfNew((RexxInstructionIf *)second);
      if (instruction != OREF_NULL) {  /* have an instruction?              */
        this->addClause(instruction);  /* add this here                     */
        instruction = OREF_NULL;       /* don't process more instructions   */
      }
      this->addClause(second);         /* add the clause to the list        */
    }
                                       /* nested IF-THEN situation?         */
    else if (type == KEYWORD_IFTHEN || type == KEYWORD_WHENTHEN) {
      second = this->popDo();          /* pop the item off of the control   */
      if (instruction != OREF_NULL) {  /* have an instruction?              */
        this->addClause(instruction);  /* add this here                     */
        instruction = OREF_NULL;       /* don't process more instructions   */
                                       /* create a new end marker           */
        second = this->endIfNew((RexxInstructionIf *)second);
        this->addClause(second);       /* add the clause to the list        */
        this->pushDo(second);          /* add to the control stack too      */
      }
      else {
                                       /* create a new end marker           */
        second = this->endIfNew((RexxInstructionIf *)second);
        this->addClause(second);       /* add the clause to the list        */
        this->pushDo(second);          /* add to the control stack too      */
      }
      break;                           /* finish up here                    */
    }
    else {                             /* some other type of construct      */
      if (instruction != OREF_NULL)    /* have an instruction?              */
        this->addClause(instruction);  /* add this here                     */
      break;                           /* finished flushing                 */
    }
  }
}

RexxMethod *RexxSource::translateBlock(
    RexxDirectory *labels )            /* labels (for interpret)            */
/******************************************************************************/
/* Function:  Translate a block of REXX code (delimited by possible           */
/*            directive instructions                                          */
/******************************************************************************/
{
  RexxInstruction *instruction;        /* created instruction item          */
  RexxInstruction *second;             /* secondary clause for IF/WHEN      */
  RexxToken       *token;              /* current working token             */
  RexxCode        *newCode;            /* newly created method              */
  INT              type;               /* instruction type information      */
  INT              controltype;        /* type on the control stack         */

                                       /* no instructions yet               */
  OrefSet(this, this->first, OREF_NULL);
  OrefSet(this, this->last, OREF_NULL);
                                       /* allocate the call list            */
  OrefSet(this, this->calls, new_list());
                                       /* create variables and lit tables   */
  OrefSet(this, this->variables, (RexxDirectory *)TheCommonRetrievers->copy());
                                       /* restart the variable index        */
  this->variableindex = FIRST_VARIABLE_INDEX;
  OrefSet(this, this->exposed_variables, new_directory());
  if (this->flags&_interpret)          /* this an interpret?                */
                                       /* just use the existing label set   */
    OrefSet(this, this->labels, labels);
  else {
                                       /* create a new labels directory     */
    OrefSet(this, this->labels, new_directory());
  }
                                       /* not collecting guard variables yet*/
  OrefSet(this, this->guard_variables, OREF_NULL);
  this->maxstack = 0;                  /* clear all of the stack accounting */
  this->currentstack = 0;              /* fields                            */
  this->flags &= ~no_clause;           /* not reached the end yet           */

                                       /* add the first dummy instruction   */
  instruction = new RexxInstruction(OREF_NULL, KEYWORD_FIRST);
  this->pushDo(instruction);           /* set bottom of control stack       */
  this->addClause(instruction);        /* add to the instruction list       */
  this->nextClause();                  /* get the next physical clause      */
  for (;;) {                           /* process all clauses               */
    instruction = OREF_NULL;           /* zero the instruction pointer      */
    while (!(this->flags&no_clause)) { /* scan through all labels           */
                                       /* resolve the instruction type      */
      instruction = this->instruction();
      if (instruction == OREF_NULL)    /* found a directive clause?         */
        break;                         /* return to higher level            */
                                       /* is this a label?                  */
      if (instruction->getType() != KEYWORD_LABEL)
        break;                         /* have a non-label clause           */
      this->addClause(instruction);    /* add this to clause list           */
      this->nextClause();              /* get the next physical clause      */
      instruction = OREF_NULL;         /* no instruction any more           */
    }
                                       /* get an end-of-clause?             */
    if (this->flags&no_clause || instruction == OREF_NULL) {
                                       /* get the control stack type        */
      controltype = this->topDo()->getType();
                                       /* while end of an IF or WHEN        */
      while (controltype == KEYWORD_ENDTHEN || controltype == KEYWORD_ENDWHEN) {
        this->popDo();                 /* pop pending closing IFs           */
        this->flushControl(OREF_NULL); /* flush any IFs or ELSEs            */
                                       /* get the control stack type        */
        controltype = this->topDo()->getType();
      }
                                       /* any unclosed composite clauses?   */
      if (this->topDo()->getType() != KEYWORD_FIRST)
                                       /* report the block error            */
        report_error_block(this->topDo());
      this->popDo();                   /* remove the top one                */
      break;                           /* done parsing this section         */
    }
    type = instruction->getType();     /* get the top instruction type      */
    if (type != KEYWORD_ELSE) {        /* have a pending THEN to finish     */
                                       /* get the control stack type        */
      controltype = this->topDo()->getType();
                                       /* while end of an IF or WHEN        */
      while (controltype == KEYWORD_ENDTHEN || controltype == KEYWORD_ENDWHEN) {
        this->popDo();                 /* pop pending closing IFs           */
        this->flushControl(OREF_NULL); /* flush any IFs or ELSEs            */
                                       /* get the control stack type        */
        controltype = this->topDo()->getType();
      }
    }
    if (type == KEYWORD_IF || type == KEYWORD_SELECT || type == KEYWORD_DO)
      this->addClause(instruction);    /* add to instruction heap           */
    else if (type != KEYWORD_ELSE)     /* not a new control level           */
      this->flushControl(instruction); /* flush any IFs or ELSEs            */
                                       /* have a bad instruction within a   */
                                       /* SELECT instruction?               */
    if (this->topDo()->getType() == KEYWORD_SELECT &&
        (type != KEYWORD_WHEN && type != KEYWORD_OTHERWISE && type != KEYWORD_END ))
      report_error_line(Error_When_expected_whenotherwise, this->topDo());

    switch (type) {                    /* post process the instructions     */

      case KEYWORD_WHEN:               /* WHEN clause of SELECT             */
        second = this->topDo();        /* get the top of the queue          */
                                       /* not working on a SELECT?          */
        if (second->getType() != KEYWORD_SELECT)
          report_error(Error_Unexpected_when_when);
                                       /* add this to the select list       */
        ((RexxInstructionSelect *)second)->addWhen((RexxInstructionIf *)instruction);
                                       /* just fall into IF logic           */

      case  KEYWORD_IF:                /* start of an IF instruction        */
        token = nextReal();            /* get the terminator token          */
                                       /* have a terminator before the THEN?*/
        if (token->classId == TOKEN_EOC) {
          this->nextClause();          /* get the next physical clause      */
          if (this->flags&no_clause)   /* get an end-of-file?               */
                                       /* raise an error                    */
            report_error_line(Error_Then_expected_if, instruction);
          token = nextReal();          /* get the first token               */
                                       /* not a THEN keyword?               */
          if (token->classId != TOKEN_SYMBOL || this->keyword(token) != KEYWORD_THEN)
                                       /* have an error                     */
            report_error_line(Error_Then_expected_if, instruction);
                                       /* create a new then clause          */
          second = this->thenNew(token, (RexxInstructionIf *)instruction);
          token = nextReal();          /* get token after THEN keyword      */
                                       /* terminator here?                  */
          if (token->classId == TOKEN_EOC) {
            this->nextClause();        /* get the next physical clause      */
            if (this->flags&no_clause) /* get an end-of-file?               */
                                       /* raise an error                    */
              report_error_line(Error_Incomplete_do_then, instruction);
          }
          else {
            previousToken();           /* step back a token                 */
            trimClause();              /* make this start of the clause     */
          }
        }
        else {                         /* if expr THEN form                 */
                                       /* create a new then clause          */
          second = this->thenNew(token, (RexxInstructionIf *)instruction);
          token = nextReal();          /* get token after THEN keyword      */
                                       /* terminator here?                  */
          if (token->classId == TOKEN_EOC) {
            this->nextClause();        /* get the next physical clause      */
            if (this->flags&no_clause) /* get an end-of-file?               */
                                       /* raise an error                    */
              report_error_line(Error_Incomplete_do_then, instruction);
          }
          else {
            previousToken();           /* step back a token                 */
            trimClause();              /* make this start of the clause     */
          }
        }
        this->addClause(second);       /* add this to the instruction list  */
        this->pushDo(second);          /* add to top of control queue       */
        continue;                      /* straight around to process clause */
                                       /* remainder                         */
      case  KEYWORD_ELSE:              /* have an ELSE instruction          */
        second = this->topDo();        /* get the top block                 */
        if (this->topDo()->getType() != KEYWORD_ENDTHEN)
                                       /* have an error                     */
          report_error(Error_Unexpected_then_else);
        this->addClause(instruction);  /* add to instruction heap           */
        second = this->popDo();        /* pop the ENDTHEN item              */
        this->pushDo(instruction);     /* add to the control list           */
                                       /* join the THEN and ELSE together   */
        ((RexxInstructionElse *)instruction)->setParent((RexxInstructionEndIf *)second);
        ((RexxInstructionEndIf *)second)->setEndInstruction((RexxInstructionEndIf *)instruction);
        token = nextReal();            /* get the next token                */
                                       /* have an ELSE keyword alone?       */
        if (token->classId == TOKEN_EOC) {
          this->nextClause();          /* get the next physical clause      */
          if (this->flags&no_clause)   /* get an end-of-file?               */
                                       /* raise an error                    */
            report_error_line(Error_Incomplete_do_else, instruction);
        }
        else {                         /* ELSE instruction form             */
          previousToken();             /* step back a token                 */
          trimClause();                /* make this start of the clause     */
        }
        continue;                      /* straight around to process clause */
                                       /* remainder                         */

      case  KEYWORD_OTHERWISE:         /* start of an OTHERWISE group       */
        second = this->topDo();        /* get the top of the queue          */
                                       /* not working on a SELECT?          */
        if (second->getType() != KEYWORD_SELECT)
          report_error(Error_Unexpected_when_otherwise);
                                       /* hook up the OTHERWISE instruction */
        ((RexxInstructionSelect *)second)->setOtherwise((RexxInstructionOtherWise *)instruction);
        this->pushDo(instruction);     /* add this to the control queue     */
        token = nextReal();            /* get the next token                */
                                       /* OTHERWISE instr form?             */
        if (token->classId != TOKEN_EOC) {
          previousToken();             /* step back a token                 */
          trimClause();                /* make this start of the clause     */
          continue;                    /* straight around to process clause */
                                       /* remainder                         */
        }
        break;                         /* normal OTHERWISE processing       */


      case  KEYWORD_END:               /* END instruction for DO or SELECT  */
        second = this->popDo();        /* get the top of the queue          */
        type = second->getType();      /* get the instruction type          */
                                       /* not working on a block?           */
        if (type != KEYWORD_SELECT &&
            type != KEYWORD_OTHERWISE &&
            type != KEYWORD_DO) {
          if (type == KEYWORD_ELSE)    /* on an else?                       */
                                       /* give the specific error           */
            report_error(Error_Unexpected_end_else);
          else if (type == KEYWORD_IFTHEN || type == KEYWORD_WHENTHEN)
                                       /* this is a different error         */
            report_error(Error_Unexpected_end_then);
          else
                                       /* have a misplaced END              */
            report_error(Error_Unexpected_end_nodo);
        }
        if (type == KEYWORD_OTHERWISE) /* OTHERWISE part of a SELECT?       */
          second = this->popDo();      /* need to pop one more item off     */
                                       /* matching a select?                */
        if (second->getType() == KEYWORD_SELECT)
                                       /* match up the instruction          */
          ((RexxInstructionSelect *)second)->matchEnd((RexxInstructionEnd *)instruction, this);
        else                           /* must be a DO block                */
                                       /* match up the instruction          */
          ((RexxInstructionDo *)second)->matchEnd((RexxInstructionEnd *)instruction, this);
        this->flushControl(OREF_NULL); /* finish pending IFs or ELSEs       */
        break;

      case  KEYWORD_DO:                // start of new DO group (also picks up LOOP instruction)
        this->pushDo(instruction);     /* add this to the control queue     */
        break;

      case  KEYWORD_SELECT:            /* start of new SELECT group         */
        this->pushDo(instruction);     /* and also to the control queue     */
        break;

      default:                         /* other types of instruction        */
        break;
    }
    this->nextClause();                /* get the next physical clause      */
#ifdef NOTIMER
    if (!(iTransClauseCounter++ % CLAUSESPERYIELD))
      CurrentActivity->relinquish();   /* yield to other system processes   */
#endif
  }
                                       /* now go resolve any label targets  */
  instruction = (RexxInstruction *)(this->calls->removeFirst());
                                       /* while still more references       */
  while (instruction != (RexxInstruction *)TheNilObject) {
                                       /* actually a function call?         */
    if (OTYPE(Function, instruction))
                                       /* resolve the function call         */
      ((RexxExpressionFunction *)instruction)->resolve(this->labels);
    else
                                       /* resolve the CALL/SIGNAL/FUNCTION  */
                                       /* label targets                     */
      ((RexxInstructionCallBase *)instruction)->resolve(this->labels);
                                       /* now get the next instruction      */
    instruction = (RexxInstruction *)(this->calls->removeFirst());
  }
                                       /* remove the first instruction      */
  OrefSet(this, this->first, this->first->nextInstruction);
                                       /* no labels needed?                 */
  if (this->labels != OREF_NULL && this->labels->items() == 0)
                                       /* release that directory also       */
    OrefSet(this, this->labels, OREF_NULL);
                                       /* create a rexx code object         */
  newCode = new RexxCode(this, this->first, this->labels,
                               (this->maxstack+ 10),
                               this->variableindex);
                                       /* now return as a method object     */
  return new_method(0, (PCPPM)NULL, 0, (RexxObject *)newCode);
}

RexxInstruction *RexxSource::instruction()
/******************************************************************************/
/* Function:  Process an individual REXX clause                               */
/******************************************************************************/
{
    RexxToken       *first;              /* first token of clause             */
    RexxToken       *second;             /* second token of clause            */
    RexxInstruction *instruction;        /* current working instruction       */
    RexxObject      *term;               /* term for a message send           */
    RexxObject      *subexpression;      /* subexpression of a clause         */
    INT              keyword;            /* resolved instruction keyword      */

    instruction = OREF_NULL;             /* default to no instruction found   */
    first = nextReal();                  /* get the first token               */

    if (first->classId == TOKEN_DCOLON)
    {/* reached the end of a block?       */
        firstToken();                      /* reset the location                */
        this->reclaimClause();             /* give back the clause              */
    }
    else
    {                               /* have a real instruction to process*/
        second = nextToken();              /* now get the second token          */
                                           /* is this a label?  (can be either  */
                                           /* a symbol or a literal)            */
        if ((first->classId == TOKEN_SYMBOL || first->classId == TOKEN_LITERAL) && second->classId == TOKEN_COLON)
        {
            if (this->flags&_interpret)      /* is this an interpret?             */
                                             /* this is an error                  */
                report_error_token(Error_Unexpected_label_interpret, first);
            firstToken();                    /* reset to the beginning            */
            instruction = this->labelNew();  /* create a label instruction        */
            second = nextToken();            /* get the next token                */
                                             /* not the end of the clause?        */
            if (second->classId != TOKEN_EOC)
            {
                previousToken();               /* give this token back              */
                trimClause();                  /* make this start of the clause     */
                this->reclaimClause();         /* give the remaining clause back    */
            }
            return instruction;
        }

        // this is potentially an assignment of the form "symbol = expr"
        if (first->classId == TOKEN_SYMBOL)
        {
            // "symbol == expr" is considered an error
            if (second->subclass == OPERATOR_STRICT_EQUAL)
            {
                report_error_token(Error_Invalid_expression_general, second);
            }
            // true assignment instruction?
            if (second->subclass == OPERATOR_EQUAL)
            {
                return this->assignmentNew(first);
            }
            // this could be a special assignment operator such as "symbol += expr"
            else if (second->classId == TOKEN_ASSIGNMENT)
            {
                return this->assignmentOpNew(first, second);
            }
            // other

        }

        /* some other type of instruction    */
        /* we need to skip over the first    */
        /* term of the instruction to        */
        /* determine the type of clause,     */
        /* including recognition of keyword  */
        /* instructions                      */
        firstToken();                    /* reset to the first token          */
        term = this->messageTerm();      /* get the first term of instruction */
        second = nextToken();            /* get the next token                */


        // some sort of recognizable message term?  Need to check for the
        // special cases.
        if (term != OREF_NULL)
        {
            // if parsing the message term consumed everything, this is a message instruction
            if (second->classId == TOKEN_EOC)
            {
                return this->messageNew((RexxExpressionMessage *)term);
            }
            else if (second->subclass == OPERATOR_STRICT_EQUAL)
            {
                // messageterm == something is an invalid assignment
                report_error_token(Error_Invalid_expression_general, second);
            }
            // messageterm = something is a pseudo assignment
            else if (second->subclass == OPERATOR_EQUAL)
            {
                this->saveObject(term);      /* protect this                      */
                // we need an expression following the op token
                subexpression = this->subExpression(TERM_EOC);
                if (subexpression == OREF_NULL)
                {
                    report_error_token(Error_Invalid_expression_general, second);
                }
                // this is a message assignment
                instruction = this->messageAssignmentNew((RexxExpressionMessage *)term, subexpression);
                this->toss(term);              /* release the term                  */
                return instruction;
            }
            // one of the special operator forms?
            else if (second->classId == TOKEN_ASSIGNMENT)
            {
                this->saveObject(term);      /* protect this                      */
                // we need an expression following the op token
                subexpression = this->subExpression(TERM_EOC);
                if (subexpression == OREF_NULL)
                {
                    report_error_token(Error_Invalid_expression_general, second);
                }
                // this is a message assignment
                instruction = this->messageAssignmentOpNew((RexxExpressionMessage *)term, second, subexpression);
                this->toss(term);              /* release the term                  */
                return instruction;
            }
        }

        // ok, none of the special cases passed....not start the keyword processing

        firstToken();                  /* reset to the first token          */
        first = nextToken();           /* get the first token again         */
                                       /* is first a symbol that matches a  */
                                       /* defined REXX keyword?             */
        if (first->classId == TOKEN_SYMBOL && (keyword = this->keyword(first)))
        {

            switch (keyword)
            {           /* process each instruction type     */

                case KEYWORD_NOP:          /* NOP instruction                   */
                    /* add the instruction to the parse  */
                    instruction = this->nopNew();
                    break;

                case KEYWORD_DROP:         /* DROP instruction                  */
                    /* add the instruction to the parse  */
                    instruction = this->dropNew();
                    break;

                case KEYWORD_SIGNAL:       /* various forms of SIGNAL           */
                    /* add the instruction to the parse  */
                    instruction = this->signalNew();
                    break;

                case KEYWORD_CALL:         /* various forms of CALL             */
                    /* add the instruction to the parse  */
                    instruction = this->callNew();
                    break;

                case KEYWORD_RAISE:        /* RAISE instruction                 */
                    /* add the instruction to the parse  */
                    instruction = this->raiseNew();
                    break;

                case KEYWORD_ADDRESS:      /* ADDRESS instruction               */
                    /* add the instruction to the parse  */
                    instruction = this->addressNew();
                    break;

                case KEYWORD_NUMERIC:      /* NUMERIC instruction               */
                    /* add the instruction to the parse  */
                    instruction = this->numericNew();
                    break;

                case KEYWORD_TRACE:        /* TRACE instruction                 */
                    /* add the instruction to the parse  */
                    instruction = this->traceNew();
                    break;

                case KEYWORD_DO:           /* all variations of DO instruction  */
                    /* add the instruction to the parse  */
                    instruction = this->doNew();
                    break;

                case KEYWORD_LOOP:         /* all variations of LOOP instruction  */
                    /* add the instruction to the parse  */
                    instruction = this->loopNew();
                    break;

                case KEYWORD_EXIT:         /* EXIT instruction                  */
                    /* add the instruction to the parse  */
                    instruction = this->exitNew();
                    break;

                case KEYWORD_INTERPRET:    /* INTERPRET instruction             */
                    /* add the instruction to the parse  */
                    instruction = this->interpretNew();
                    break;

                case KEYWORD_PUSH:         /* PUSH instruction                  */
                    /* add the instruction to the parse  */
                    instruction = this->queueNew(QUEUE_LIFO);
                    break;

                case KEYWORD_QUEUE:        /* QUEUE instruction                 */
                    /* add the instruction to the parse  */
                    instruction = this->queueNew(QUEUE_FIFO);
                    break;

                case KEYWORD_REPLY:        /* REPLY instruction                 */
                    /* interpreted?                      */
                    if (this->flags&_interpret)
                        report_error(Error_Translation_reply_interpret);
                    /* add the instruction to the parse  */
                    instruction = this->replyNew();
                    break;

                case KEYWORD_RETURN:       /* RETURN instruction                */
                    /* add the instruction to the parse  */
                    instruction = this->returnNew();
                    break;

                case KEYWORD_IF:           /* IF instruction                    */
                    /* add the instruction to the parse  */
                    instruction = this->ifNew(KEYWORD_IF);
                    break;

                case KEYWORD_ITERATE:      /* ITERATE instruction               */
                    /* add the instruction to the parse  */
                    instruction = this->leaveNew(KEYWORD_ITERATE);
                    break;

                case KEYWORD_LEAVE:        /* LEAVE instruction                 */
                    /* add the instruction to the parse  */
                    instruction = this->leaveNew(KEYWORD_LEAVE);
                    break;

                case KEYWORD_EXPOSE:       /* EXPOSE instruction                */
                    /* interpreted?                      */
                    if (this->flags&_interpret)
                        report_error(Error_Translation_expose_interpret);
                    /* add the instruction to the parse  */
                    instruction = this->exposeNew();
                    break;

                case KEYWORD_FORWARD:      /* FORWARD instruction               */
                    /* interpreted?                      */
                    if (this->flags&_interpret)
                        report_error(Error_Translation_forward_interpret);
                    /* add the instruction to the parse  */
                    instruction = this->forwardNew();
                    break;

                case KEYWORD_PROCEDURE:    /* PROCEDURE instruction             */
                    /* add the instruction to the parse  */
                    instruction = this->procedureNew();
                    break;

                case KEYWORD_GUARD:        /* GUARD instruction                 */
                    /* interpreted?                      */
                    if (this->flags&_interpret)
                        report_error(Error_Translation_guard_interpret);
                    /* add the instruction to the parse  */
                    instruction = this->guardNew();
                    break;

                case KEYWORD_USE:          /* USE instruction                   */
                    /* interpreted?                      */
                    if (this->flags&_interpret)
                        report_error(Error_Translation_use_interpret);
                    /* add the instruction to the parse  */
                    instruction = this->useNew();
                    break;

                case KEYWORD_ARG:          /* ARG instruction                   */
                    /* add the instruction to the parse  */
                    instruction = this->parseNew(SUBKEY_ARG);
                    break;

                case KEYWORD_PULL:         /* PULL instruction                  */
                    /* add the instruction to the parse  */
                    instruction = this->parseNew(SUBKEY_PULL);
                    break;

                case KEYWORD_PARSE:        /* PARSE instruction                 */
                    /* add the instruction to the parse  */
                    instruction = this->parseNew(KEYWORD_PARSE);
                    break;

                case KEYWORD_SAY:          /* SAY instruction                   */
                    /* add the instruction to the parse  */
                    instruction = this->sayNew();
                    break;

                case KEYWORD_OPTIONS:      /* OPTIONS instruction               */
                    /* add the instruction to the parse  */
                    instruction = this->optionsNew();
                    break;

                case KEYWORD_SELECT:       /* SELECT instruction                */
                    /* add the instruction to the parse  */
                    instruction = this->selectNew();
                    break;

                case KEYWORD_WHEN:         /* WHEN in an SELECT instruction     */
                    /* add the instruction to the parse  */
                    instruction = this->ifNew(KEYWORD_WHEN);
                    break;

                case KEYWORD_OTHERWISE:    /* OTHERWISE in a SELECT             */
                    /* add the instruction to the parse  */
                    instruction = this->otherwiseNew(first);
                    break;

                case KEYWORD_ELSE:         /* unexpected ELSE                   */
                    /* add the instruction to the parse  */
                    instruction = this->elseNew(first);
                    break;

                case KEYWORD_END:          /* END for a block construct         */
                    /* add the instruction to the parse  */
                    instruction = this->endNew();
                    break;

                case KEYWORD_THEN:         /* unexpected THEN                   */
                    /* raise an error                    */
                    report_error(Error_Unexpected_then_then);
                    break;

            }
        }
        else
        {                         /* this is a "command" instruction   */
            firstToken();                /* reset to the first token          */
                                         /* process this instruction          */
            instruction = this->commandNew();
        }
    }
    return instruction;                  /* return the created instruction    */
}

RexxVariableBase *RexxSource::addVariable(
    RexxString *varname)               /* variable to add                   */
/******************************************************************************/
/* Function:  Resolve a variable name to a single common retriever object     */
/*            per method                                                      */
/******************************************************************************/
{
  RexxVariableBase *retriever;         /* variable retriever                */

                                       /* check the directory for an entry  */
  retriever = (RexxVariableBase *)this->variables->fastAt(varname);
  if (retriever == OREF_NULL) {        /* not in the table yet?             */
    if (!(this->flags&_interpret)) {   /* not in an interpret?              */
      this->variableindex++;           /* step the counter                  */
                                       /* create a new variable retriever   */
      retriever = new RexxParseVariable(varname, this->variableindex);
    }
    else                               /* force dynamic lookup each time    */
      retriever = new RexxParseVariable(varname, 0);
                                       /* add to the variable table         */
    this->variables->put((RexxObject *)retriever, varname);
  }
                                       /* collecting guard variables?       */
  if (this->guard_variables != OREF_NULL) {
                                       /* in the list of exposed variables? */
    if (this->exposed_variables != OREF_NULL && this->exposed_variables->fastAt(varname) != OREF_NULL)
                                       /* add this to the guard list        */
      this->guard_variables->put((RexxObject *)retriever, (RexxObject *)retriever);
  }
  return retriever;                    /* return variable accesser          */
}

RexxStemVariable *RexxSource::addStem(
    RexxString *stem)                  /* stem to add                       */
/******************************************************************************/
/* Function:  Process creation of stem variables                              */
/******************************************************************************/
{
  RexxStemVariable *retriever;         /* variable retriever                */

                                       /* check the table for an entry      */
  retriever = (RexxStemVariable *)(this->variables->fastAt(stem));
  if (retriever == OREF_NULL) {        /* not in the table yet?             */
    if (!(this->flags&_interpret)) {   /* not in an interpret?              */
      this->variableindex++;           /* step the counter                  */
                                       /* create a new variable retriever   */
      retriever = new RexxStemVariable(stem, this->variableindex);
    }
    else                               /* force dynamic lookup each time    */
      retriever = new RexxStemVariable(stem, 0);
                                       /* add to the variable table         */
    this->variables->put((RexxObject *)retriever, stem);
  }
                                       /* collecting guard variables?       */
  if (this->guard_variables != OREF_NULL) {
                                       /* in the list of exposed variables? */
    if (this->exposed_variables != OREF_NULL && this->exposed_variables->fastAt(stem) != OREF_NULL)
                                       /* add this to the guard list        */
      this->guard_variables->put((RexxObject *)retriever, (RexxObject *)retriever);
  }
  return retriever;                    /* return variable accesser          */
}


RexxCompoundVariable *RexxSource::addCompound(
    RexxString *name)                  /* name of the compound variable     */
/******************************************************************************/
/* Function:  Parse to completion a compound variable                         */
/******************************************************************************/
{
  RexxStemVariable     *stemRetriever; /* retriever for the stem value      */
  RexxString           *stem;          /* stem part of compound variable    */
  RexxString           *tail;          /* tail section string value         */
  PCHAR                 start;         /* starting scan position            */
  INT                   length;        /* length of tail section            */
  PCHAR                 position;      /* current position                  */
  size_t                tailCount;     /* count of tails in compound        */

  length = name->length;               /* get the string length             */
  position = name->stringData;         /* start scanning at first character */
  start = position;                    /* save the starting point           */

  while (*position != '.') {           /* scan to the first period          */
    position++;                        /* step to the next character        */
    length--;                          /* reduce the length also            */
  }
                                       /* get the stem string               */
  stem = new_string(start, position - start + 1);
  stemRetriever = this->addStem(stem); /* get a retriever item for this     */

  tailCount = 0;                       /* no tails yet                      */
  position++;                          /* step past previous period         */
  length--;                            /* adjust the length                 */
  while (length > 0) {                 /* process rest of the variable      */
    start = position;                  /* save the start position           */
                                       /* scan for the next period          */
    while (length > 0 && *position != '.') {
      position++;                      /* step to the next character        */
      length--;                        /* reduce the length also            */
    }
                                       /* extract the tail piece            */
    tail = new_string(start, position - start);
                                       /* have a null tail piece or         */
                                       /* section begin with a digit?       */
    if (!(tail->length == 0 || (*start >= '0' && *start <= '9')))
                                       /* push onto the term stack          */
      this->subTerms->push((RexxObject *)(this->addVariable(tail)));
    else
                                       /* just use the string value directly*/
      this->subTerms->push(this->commonString(tail));
    tailCount++;                       /* up the tail count                 */
    position++;                        /* step past previous period         */
    length--;                          /* adjust the length                 */
  }
  if (*(position - 1) == '.') {        /* have a trailing period?           */
                                       /* add to the tail piece list        */
    this->subTerms->push(OREF_NULLSTRING);
    tailCount++;                       /* up the tail count                 */
  }
                                       /* finally, create the compound var  */
  return new (tailCount) RexxCompoundVariable(stem, stemRetriever->index, this->subTerms, tailCount);
}


void RexxSource::expose(
    RexxString *name )                 /* variable name to add to list      */
/******************************************************************************/
/* Function:  Add a variable name to the list of exposed variables for the    */
/*            method.                                                         */
/******************************************************************************/
{
                                       /* add to the exposed variables list */
  this->exposed_variables->put(name, name);
}


RexxString *RexxSource::commonString(
    RexxString *string )               /* string token to "collapse"        */
/******************************************************************************/
/* Function:  Compress all string tokens needed by a group of programs into   */
/*            a single, common set of strings.                                */
/******************************************************************************/
{
  RexxString *result;                  /* resulting common string           */

                                       /* check the global table first      */
  result = (RexxString *)this->strings->fastAt(string);
  if (result == OREF_NULL) {           /* not in the table                  */
    this->strings->put(string, string);/* add this to the table             */
    result = string;                   /* also the final value              */
  }
  return result;                       /* return the string                 */
}


RexxObject *RexxSource::addVariable(RexxToken *token)
{
    needVariable(token);
    return addText(token);
}


RexxObject *RexxSource::addText(
    RexxToken *token)                  /* token to process                  */
/******************************************************************************/
/* Function:  Generalized text token addition                                 */
/******************************************************************************/
{
  RexxObject       *retriever;         /* created retriever                 */
  RexxObject       *value;             /* evaluated literal value           */
  RexxString       *name;              /* name of the added literal string  */

  name = token->value;                 /* get the string value for this     */
  switch (token->classId) {            /* process different token types     */

    case TOKEN_SYMBOL:                 /* various types of symbols          */
                                       /* each symbol subtype requires a    */
      switch (token->subclass) {       /* different retrieval method        */

        case SYMBOL_DUMMY:             /* just a dot symbol                 */
        case SYMBOL_CONSTANT:          /* a literal symbol                  */

                                       /* see if we've had this before      */
          retriever = this->literals->fastAt(name);
          if (retriever == OREF_NULL) {/* first time literal?               */
                                       /* can we create an integer object?  */
            if (token->numeric == INTEGER_CONSTANT) {
                                       /* create this as an integer         */
              value = name->requestInteger(DEFAULT_DIGITS);
                                       /* conversion error?                 */
              if (value == TheNilObject)
                value = name;          /* just go with the string value     */
              else
                                       /* snip off the string number string */
                                       /* value that was created when the   */
                                       /* integer value was created.  This  */
                                       /* is rarely used, but contributes   */
                                       /* to the saved program size         */
                name->setNumberString(OREF_NULL);
            }
            else {
              value = name;            /* just use the string value         */
                                       /* give it a number string value     */
              name->setNumberString((RexxObject *)value->numberString());
            }
                                       /* the constant is the retriever     */
            this->literals->put(value, name);
            retriever = value;         /* the retriever is the value itthis */
          }
          break;

        case SYMBOL_VARIABLE:          /* simple variable symbol            */
                                       /* add variable to proper dictionary */
          retriever = (RexxObject *)this->addVariable(name);
          break;

        case SYMBOL_STEM:              /* stem variable                     */
                                       /* add variable to proper dictionary */
          retriever = (RexxObject *)this->addStem(name);
          break;

        case SYMBOL_COMPOUND:          /* compound variable, need more      */
                                       /* add variable to proper dictionary */
          retriever = (RexxObject *)this->addCompound(name);
          break;

        case SYMBOL_DOTSYMBOL:         /* variable with a leading dot       */
                                       /* get a lookup object               */
                                       /* see if we've had this before      */
          retriever = this->variables->fastAt(name);
          if (retriever == OREF_NULL) {/* first time dot variable?          */
                                       /* create the shorter name           */
            value = name->extract(1, name->length - 1);
                                       /* add this to the common pile       */
            value = this->commonString((RexxString *)value);
                                       /* create a retriever for this       */
            retriever = (RexxObject *)new RexxDotVariable((RexxString *)value);
                                       /* add this to the common table      */
            this->variables->put(retriever, name);
          }
          break;

        default:                       /* all other types (shouldn't happen)*/
          retriever = OREF_NULL;       /* return nothing                    */
          break;
      }
      break;

    case TOKEN_LITERAL:                /* literal strings                   */
                                       /* get a lookup object               */
                                       /* see if we've had this before      */
      retriever = this->literals->fastAt(name);
      if (retriever == OREF_NULL) {    /* first time literal?               */
                                       /* the constant is the retriever     */
        this->literals->put(name,  name);
        retriever = name;              /* use the name directly             */
      }
      break;

    default:                           /* all other tokens                  */
      retriever = OREF_NULL;           /* don't return anything             */
      break;
  }
  return retriever;                    /* return created retriever          */
}

RexxVariableBase *RexxSource::getRetriever(
    RexxString *name)                  /* name of the variable to process   */
/******************************************************************************/
/* Function:  Generalized method attribute retriever                          */
/******************************************************************************/
{
  RexxVariableBase *retriever;         /* created retriever                 */

  switch (name->isSymbol()) {          /* go validate the symbol            */

    case STRING_NAME:                  /* valid simple name                 */
                                       /* get a simple dynamic retriever    */
      retriever = (RexxVariableBase *)new RexxParseVariable(name, 0);
      break;

    case STRING_STEM:                  /* this is a stem name               */
                                       /* force dynamic lookup each time    */
      retriever = (RexxVariableBase *)new RexxStemVariable(name, 0);
      break;

    case STRING_COMPOUND_NAME:         /* compound variable name            */
                                       /* get a direct retriever for this   */
      retriever = (RexxVariableBase *)buildCompoundVariable(name, TRUE);
      break;

    default:                           /* all other invalid cases           */
                                       /* have an invalid attribute         */
      report_error1(Error_Translation_invalid_attribute, name);
  }
  return retriever;                    /* return created retriever          */
}


void RexxSource::addClause(
    RexxInstruction *instruction)      /* new label to add                  */
/******************************************************************************/
/* Add an instruction to the tree code execution stream                       */
/******************************************************************************/
{
  if (this->first == OREF_NULL) {      /* is this the first one?            */
                                       /* make this the first one           */
    OrefSet(this, this->first, instruction);
                                       /* and the last one                  */
    OrefSet(this, this->last, instruction);
  }
  else {                               /* non-root instruction              */
    this->last->setNext(instruction);  /* add on to the last instruction    */
                                       /* this is the new last instruction  */
    OrefSet(this, this->last, instruction);
  }
                                       /* now safe from garbage collection  */
  this->toss((RexxObject *)instruction);
}


void RexxSource::addLabel(
    RexxInstruction      *label,       /* new label to add                  */
    RexxString           *labelname )  /* the label name                    */
/******************************************************************************/
/* Function:  add a label to the global label table.                          */
/******************************************************************************/
{
                                       /* not already in the table?         */
  if (this->labels->fastAt(labelname) == OREF_NULL)
                                       /* add this item                     */
    this->labels->put((RexxObject *)label, labelname);
}


RexxInstruction *RexxSource::findLabel(
    RexxString *labelname)             /* target label                      */
/******************************************************************************/
/* Search the label table for a label name match                              */
/******************************************************************************/
{
  if (this->labels != OREF_NULL)       /* have labels?                      */
                                       /* just return entry from the table  */
    return (RexxInstruction *)this->labels->fastAt(labelname);
  else
    return OREF_NULL;                  /* don't return anything             */
}

void RexxSource::setGuard()
/******************************************************************************/
/* Function:  Set on guard expression variable "gathering"                    */
/******************************************************************************/
{
                                       /* just starting to trap?            */
  if (this->guard_variables == OREF_NULL)
                                       /* create the guard table            */
    OrefSet(this, this->guard_variables, new_object_table());
}

RexxArray *RexxSource::getGuard()
/******************************************************************************/
/* Function:  Complete guard expression variable collection and return the    */
/*            table of variables.                                             */
/******************************************************************************/
{
  RexxArray *guards;                   /* returned guard variable list      */

                                       /* convert into an array             */
  guards = this->guard_variables->makeArray();
                                       /* discard the table                 */
  OrefSet(this, this->guard_variables, OREF_NULL);
                                       /* just starting to trap?            */
  return guards;                       /* return the guards array           */
}

RexxObject *RexxSource::constantExpression()
/******************************************************************************/
/* Function:  Evaluate a "constant" expression for REXX instruction keyword   */
/*            values.  A constant expression is a literal string, constant    */
/*            symbol, or an expression enclosed in parentheses.               */
/******************************************************************************/
{
  RexxToken  *token;                   /* current token                     */
  RexxToken  *second;                  /* second token                      */
  RexxObject *expression;              /* parse expression                  */

  token = nextReal();                  /* get the first token               */
  if (token->isLiteral())              /* literal string expression?        */

    expression = this->addText(token); /* get the literal retriever         */
  else if (token->isConstant())        /* how about a constant symbol?      */
    expression = this->addText(token); /* get the literal retriever         */
                                       /* got an end of expression?         */
  else if (token->classId == TOKEN_EOC) {
    previousToken();                   /* push the token back               */
    return OREF_NULL;                  /* nothing here (may be optional)    */
  }
                                       /* not a left paren here?            */
  else if (token->classId != TOKEN_LEFT)
                                       /* this is an invalid expression     */
    report_error_token(Error_Invalid_expression_general, token);
  else {
                                       /* get the subexpression             */
    expression = this->subExpression(TERM_EOC | TERM_RIGHT);
    second = nextToken();              /* get the terminator token          */
                                       /* not terminated by a right paren?  */
    if (second->classId != TOKEN_RIGHT)
                                       /* this is an error                  */
      report_error_position(Error_Unmatched_parenthesis_paren, token);
  }
  this->holdObject(expression);        /* protect the expression            */
  return expression;                   /* and return it                     */
}

RexxObject *RexxSource::constantLogicalExpression()
/******************************************************************************/
/* Function:  Evaluate a "constant" expression for REXX instruction keyword   */
/*            values.  A constant expression is a literal string, constant    */
/*            symbol, or an expression enclosed in parentheses.  The          */
/*            expression inside parens can be a complex logical expression.   */
/******************************************************************************/
{
  RexxToken  *token;                   /* current token                     */
  RexxToken  *second;                  /* second token                      */
  RexxObject *expression;              /* parse expression                  */

  token = nextReal();                  /* get the first token               */
  if (token->isLiteral())              /* literal string expression?        */

    expression = this->addText(token); /* get the literal retriever         */
  else if (token->isConstant())        /* how about a constant symbol?      */
    expression = this->addText(token); /* get the literal retriever         */
                                       /* got an end of expression?         */
  else if (token->classId == TOKEN_EOC) {
    previousToken();                   /* push the token back               */
    return OREF_NULL;                  /* nothing here (may be optional)    */
  }
                                       /* not a left paren here?            */
  else if (token->classId != TOKEN_LEFT)
                                       /* this is an invalid expression     */
    report_error_token(Error_Invalid_expression_general, token);
  else {
                                       /* get the subexpression             */
    expression = this->parseLogical(token, TERM_EOC | TERM_RIGHT);
    second = nextToken();              /* get the terminator token          */
                                       /* not terminated by a right paren?  */
    if (second->classId != TOKEN_RIGHT)
                                       /* this is an error                  */
      report_error_position(Error_Unmatched_parenthesis_paren, token);
  }
  this->holdObject(expression);        /* protect the expression            */
  return expression;                   /* and return it                     */
}

RexxObject *RexxSource::parenExpression(RexxToken *start)
/******************************************************************************/
/* Function:  Evaluate a "parenthetical" expression for REXX instruction      */
/*            values.  A parenthetical expression is an expression enclosed   */
/*            in parentheses.                                                 */
/******************************************************************************/
{
  // NB, the opening paren has already been parsed off

  RexxObject *expression = this->subExpression(TERM_EOC | TERM_RIGHT);
  RexxToken *second = nextToken();   /* get the terminator token          */
                                     /* not terminated by a right paren?  */
  if (second->classId != TOKEN_RIGHT)
  {
      report_error_position(Error_Unmatched_parenthesis_paren, start);
  }
                                     /* this is an error                  */
  this->holdObject(expression);        /* protect the expression            */
  return expression;                   /* and return it                     */
}

RexxObject *RexxSource::expression(
  int   terminators )                  /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off an expression, stopping when one of the possible set  */
/*            of terminator tokens is reached.  The terminator token is       */
/*            placed back on the token queue.                                 */
/******************************************************************************/
{
  nextReal();                          /* get the first real token          */
  previousToken();                     /* now put it back                   */
                                       /* parse off the subexpression       */
  return this->subExpression(terminators);
}

RexxObject *RexxSource::subExpression(
  int   terminators )                  /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off a sub- expression, stopping when one of the possible  */
/*            set of terminator tokens is reached.  The terminator token is   */
/*            placed back on the token queue.                                 */
/******************************************************************************/
{
  RexxObject    *left;                 /* left term of operation            */
  RexxObject    *right;                /* right term of operation           */
  RexxToken     *token;                /* current working token             */
  RexxToken     *second;               /* look ahead token                  */
  RexxObject    *subexpression;        /* final subexpression               */
  LOCATIONINFO  location;              /* token location info               */

                                       /* get the left term                 */
  left = this->messageSubterm(terminators);
  if (left == OREF_NULL)               /* end of the expression?            */
    return OREF_NULL;                  /* done processing here              */
  this->pushTerm(left);                /* add the term to the term stack    */
                                       /* add a fence item to operator stack*/
  this->pushOperator((RexxToken *)TheNilObject);
  token = nextToken();                 /* get the next token                */
                                       /* loop until end of expression      */
  while (!this->terminator(terminators, token)) {
    switch (token->classId) {          /* switch based on token type        */

      case  TOKEN_TILDE:               /* have a message send operation     */
      case  TOKEN_DTILDE:              /* have a double twiddle operation   */
        left = this->popTerm();        /* get the left term from the stack  */
        if (left == OREF_NULL)         /* not there?                        */
                                       /* this is an invalid expression     */
          report_error_token(Error_Invalid_expression_general, token);
                                       /* process a message term            */
        subexpression = this->message(left, token->classId, terminators);
        this->pushTerm(subexpression); /* push this back on the term stack  */
        break;

      case  TOKEN_SQLEFT:              /* collection syntax message         */
        left = this->popTerm();        /* get the left term from the stack  */
        if (left == OREF_NULL)         /* not there?                        */
                                       /* this is an invalid expression     */
          report_error_token(Error_Invalid_expression_general, token);
                                       /* process a message term            */
        subexpression = this->collectionMessage(token, left, terminators);
        this->pushTerm(subexpression); /* push this back on the term stack  */
        break;

      case  TOKEN_SYMBOL:              /* Symbol in the expression          */
      case  TOKEN_LITERAL:             /* Literal in the expression         */
      case  TOKEN_LEFT:                /* start of subexpression            */

        token->getLocation(&location); /* get the token start position      */
                                       /* abuttal ends on the same line     */
        location.endline = location.line;
                                       /* and is zero characters long       */
        location.endoffset = location.offset;
                                       /* This is actually an abuttal       */
        token = new RexxToken (TOKEN_OPERATOR, OPERATOR_ABUTTAL, OREF_NULLSTRING, &location);
        previousToken();               /* step back on the token list       */

      case  TOKEN_BLANK:               /* possible blank concatenate        */
        second = nextReal();           /* get the next token                */
                                       /* blank prior to a terminator?      */
        if (this->terminator(terminators, second))
          break;                       /* not a real operator               */
        else                           /* have a blank operator             */
          previousToken();             /* push this back                    */
                                       /* fall through to operator logic    */

      case  TOKEN_OPERATOR:            /* have a dyadic operator            */
                                       /* actually a prefix only one?       */
        if (token->subclass == OPERATOR_BACKSLASH)
                                       /* this is an invalid expression     */
            report_error_token(Error_Invalid_expression_general, token);
        for (;;) {                     /* handle operator precedence        */
          second = this->topOperator();/* get the top term                  */
                                       /* hit the fence term?               */
          if (second == (RexxToken *)TheNilObject)
            break;                     /* out of here                       */
                                       /* current have higher precedence?   */
          if (this->precedence(token) > this->precedence(second))
            break;                     /* finished also                     */
          right = this->popTerm();     /* get the right term                */
          left = this->popTerm();      /* and the left term                 */
                                       /* not enough terms?                 */
          if (right == OREF_NULL || left == OREF_NULL)
                                       /* this is an invalid expression     */
            report_error_token(Error_Invalid_expression_general, token);
                                       /* create a new operation            */
          RexxToken *op = popOperator();
          subexpression = (RexxObject *)new RexxBinaryOperator(op->subclass, left, right);
                                       /* push this back on the term stack  */
          this->pushTerm(subexpression);
        }
        this->pushOperator(token);     /* push this operator onto stack     */
        right = this->messageSubterm(terminators);
                                       /* end of the expression?            */
        if (right == OREF_NULL && token->subclass != OPERATOR_BLANK)
                                       /* have a bad expression             */
          report_error_token(Error_Invalid_expression_general, token);
        this->pushTerm(right);         /* add the term to the term stack    */
        break;

      case TOKEN_ASSIGNMENT:
      // special assignment token in a bad context.  We report this as an error.
                                       /* this is an invalid expression     */
        report_error_token(Error_Invalid_expression_general, token);
        break;

      case TOKEN_COMMA:                /* found a comma in the expression   */
                                       /* should have been trapped as an    */
                                       /* expression terminator, so this is */
                                       /* not a valid expression            */
        report_error(Error_Unexpected_comma_comma);
        break;

      case TOKEN_RIGHT:                /* found a paren in the expression   */
        report_error(Error_Unexpected_comma_paren);
        break;

      case TOKEN_SQRIGHT:              /* found a bracket in the expression */
        report_error(Error_Unexpected_comma_bracket);
        break;

      default:                         /* something unexpected              */
                                       /* not a valid expression            */
        report_error_token(Error_Invalid_expression_general, token);
        break;
    }
    token = nextToken();               /* get the next token                */
  }
  token= this->popOperator();          /* get top operator token            */
                                       /* process pending operations        */
  while (token != (RexxToken *)TheNilObject) {
    right = this->popTerm();           /* get the right term                */
    left = this->popTerm();            /* now get the left term             */
                                       /* missing any terms?                */
    if (left == OREF_NULL || right == OREF_NULL)
                                       /* this is an invalid expression     */
      report_error_token(Error_Invalid_expression_general, token);
                                       /* create a new operation            */
    subexpression = (RexxObject *)new RexxBinaryOperator(token->subclass, left, right);
    this->pushTerm(subexpression);     /* push this back on the term stack  */
    token = this->popOperator();       /* get top operator token            */
  }
  return this->popTerm();              /* expression is top of term stack   */
}

RexxArray *RexxSource::argArray(
  RexxToken   *first,                  /* token starting arglist            */
  int          terminators )           /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off an array of argument expressions                      */
/******************************************************************************/
{
  size_t     argCount;                 /* count of arguments                */
  RexxArray *argArray;                 /* returned array                    */

                                       /* scan off the argument list        */
  argCount = this->argList(first, terminators);
  argArray = new_array(argCount);      /* get a new argument list           */
  while (argCount > 0) {               /* now copy the argument pointers    */
                                       /* in reverse order                  */
    argArray->put(this->subTerms->pop(), argCount--);
  }
  return argArray;                     /* return the argument array         */
}

size_t RexxSource::argList(
  RexxToken   *first,                  /* token starting arglist            */
  int          terminators )           /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off a list of argument expressions                        */
/******************************************************************************/
{
  RexxQueue    *arglist;               /* argument list                     */
  RexxObject   *subexpr;               /* current subexpression             */
  RexxToken    *token;                 /* current working token             */
  size_t        realcount;             /* count of real arguments           */
  size_t        total;                 /* total arguments                   */

  arglist = this->subTerms;            /* use the subterms list             */
  realcount = 0;                       /* no arguments yet                  */
  total = 0;
                                       /* get the first real token, which   */
  nextReal();                          /* skips any leading blanks on CALL  */
  previousToken();                     /* now put it back                   */
  for (;;) {                           /* loop until get a full terminator  */
                                       /* parse off next argument expression*/
    subexpr = this->subExpression(terminators | TERM_COMMA);
    arglist->push(subexpr);            /* add next argument to list         */
    this->pushTerm(subexpr);           /* add the term to the term stack    */
    total++;                           /* increment the total               */
    if (subexpr != OREF_NULL)          /* real expression?                  */
      realcount = total;               /* update the real count             */
    token = nextToken();               /* get the next token                */
    if (token->classId != TOKEN_COMMA) /* start of next argument?           */
      break;                           /* no, all finished                  */
  }
                                       /* not closed with expected ')'?     */
  if (terminators & TERM_RIGHT && token->classId != TOKEN_RIGHT)
                                       /* raise an error                    */
    report_error_position(Error_Unmatched_parenthesis_paren, first);
                                       /* not closed with expected ']'?     */
  if (terminators&TERM_SQRIGHT && token->classId != TOKEN_SQRIGHT)
                                       /* have an unmatched bracket         */
    report_error_position(Error_Unmatched_parenthesis_square, first);
  this->popNTerms(total);              /* pop all items off the term stack  */
  while (total > realcount) {          /* pop off any trailing omitteds     */
    arglist->pop();                    /* just pop off the dummy            */
    total--;                           /* reduce the total                  */
  }
  return realcount;                    /* return the argument count         */
}

RexxObject *RexxSource::function(
    RexxToken     *token,              /* arglist start (for error reports) */
    RexxToken     *name,               /* function name                     */
    int            terminators )       /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off a REXX function call                                  */
/******************************************************************************/
{
  size_t        argCount;              /* count of function arguments       */
  RexxExpressionFunction *function;    /* newly created function argument   */

  saveObject((RexxObject *)name);      // protect while parsing the argument list

                                       /* process the argument list         */
  argCount = this->argList(token, ((terminators | TERM_RIGHT) & ~TERM_SQRIGHT));

                                       /* create a new function item        */
  function = new (argCount) RexxExpressionFunction(name->value, argCount, this->subTerms, this->resolveBuiltin(name->value), (BOOL)(name->classId == TOKEN_LITERAL));
                                       /* add to table of references        */
  this->addReference((RexxObject *)function);
  removeObj((RexxObject *)name);       // end of protected windoww.
  return (RexxObject *)function;       /* and return this to the caller     */
}

RexxObject *RexxSource::collectionMessage(
  RexxToken   *token,                  /* arglist start (for error reports) */
  RexxObject  *target,                 /* target term                       */
  int          terminators )           /* expression termination context    */
/******************************************************************************/
/* Function:  Process an expression term of the form "target[arg,arg]"        */
/******************************************************************************/
{
  size_t     argCount;                 /* count of function arguments       */
  RexxObject *message;                 /* new message term                  */

  this->saveObject((RexxObject *)target);   /* save target until it gets connected to message */
                                       /* process the argument list         */
  argCount = this->argList(token, ((terminators | TERM_SQRIGHT) & ~TERM_RIGHT));
                                       /* create a new function item        */
  message = (RexxObject *)new (argCount) RexxExpressionMessage(target, (RexxString *)OREF_BRACKETS, (RexxObject *)OREF_NULL, argCount, this->subTerms, TOKEN_TILDE);
  this->holdObject(message);           /* hold this here for a while        */
  this->removeObj((RexxObject *)target);   /* target is now connected to message, remove from savelist without hold */
  return message;                      /* return the message item           */
}

RexxToken  *RexxSource::getToken(
    int   terminators,                 /* expression termination context    */
    int   errorcode)                   /* expected error code               */
/******************************************************************************/
/* Function:  Get a token, checking to see if this is a terminatore token     */
/******************************************************************************/
{
  RexxToken   *token;                  /* retrieved token                   */

  token = nextToken();                 /* get the next token                */
                                       /* this a terminator token?          */
  if (this->terminator(terminators, token)) {
    if (errorcode != 0)                /* want an error raised?             */
      report_error(errorcode);         /* report this                       */
    return OREF_NULL;                  /* just return a null                */
  }
  return token;                        /* return the token                  */
}

RexxObject *RexxSource::message(
  RexxObject  *target,                 /* message send target               */
  int          classId,                /* class of message send             */
  int          terminators )           /* expression termination context    */
/******************************************************************************/
/* Function:  Parse a full message send expression term                       */
/******************************************************************************/
{
  size_t        argCount;              /* list of function arguments        */
  RexxString   *messagename;           /* message name                      */
  RexxObject   *super;                 /* super class target                */
  RexxToken    *token;                 /* current working token             */
  RexxExpressionMessage *message;      /* new message term                  */

  super = OREF_NULL;                   /* default no super class            */
  argCount = 0;                        /* and no arguments                  */
  this->saveObject(target);   /* save target until it gets connected to message */

  /* add the term to the term stack so that the calculations */
  /* include this in the processing. */
  this->pushTerm(target);
                                       /* get the next token                */
  token = this->getToken(terminators, Error_Symbol_or_string_tilde);
                                       /* unexpected type?                  */
  if (token->classId == TOKEN_SYMBOL || token->classId == TOKEN_LITERAL)
    messagename = token->value;        /* get the message name              */
  else
                                       /* error!                            */
    report_error(Error_Symbol_or_string_tilde);
                                       /* get the next token                */
  token = this->getToken(terminators, 0);
  if (token != OREF_NULL) {            /* not reached the clause end?       */
                                       /* have a super class?               */
    if (token->classId == TOKEN_COLON) {
                                       /* get the next token                */
      token = this->getToken(terminators, Error_Symbol_expected_colon);
                                       /* not a variable symbol?            */
      if (!token->isVariable() && token->subclass != SYMBOL_DOTSYMBOL)
                                       /* have an error                     */
        report_error(Error_Symbol_expected_colon);
      super = this->addText(token);    /* get the variable retriever        */
                                       /* get the next token                */
      token = this->getToken(terminators, 0);
    }
  }
  if (token != OREF_NULL) {            /* not reached the clause end?       */
    if (token->classId == TOKEN_LEFT)  /* have an argument list?            */
                                       /* process the argument list         */
      argCount = this->argList(token, ((terminators | TERM_RIGHT) & ~TERM_SQRIGHT));
    else
      previousToken();                 /* something else, step back         */
  }

  this->popTerm();                     /* it is now safe to pop the message target */
                                       /* create a message send node        */
  message =  new (argCount) RexxExpressionMessage(target, messagename, super, argCount, this->subTerms, classId);
                                       /* protect for a bit                 */
  this->holdObject((RexxObject *)message);
  this->removeObj(target);   /* target is now connected to message, remove from savelist without hold */
  return (RexxObject *)message;        /* return the message item           */
}


/**
 * Parse off a single variable symbol or a message term that
 * can be used for an assignment.
 *
 * NOTE:  If this is a message term, then the message term
 * will be configured as an assignment term.
 *
 * @return The object for an assignment target, or OREF_NULL if something
 *         other than a variable or a message term was found.  On return,
 *         the clause position pointer will either be unchanged or
 *         positioned at the next token of the clause.
 */
RexxObject *RexxSource::variableOrMessageTerm()
{
    // try for a message term first.  If not successful, see if the
    // next token is a variable symbol.
    RexxObject *result = messageTerm();
    if (result == OREF_NULL)
    {
        RexxToken *first = nextReal();
        if (first->classId == TOKEN_SYMBOL)
        {
            // ok, add the variable to the processing list
            this->needVariable(first);
            result = this->addText(first);
        }
        else
        {
            previousToken();     // just push back on for the caller to sort out
        }
    }
    else
    {
        // we need to convert this into an assignment message.
        ((RexxExpressionMessage *)result)->makeAssignment(this);
    }
    return result;
}



RexxObject *RexxSource::messageTerm()
/******************************************************************************/
/* Function:  Parse off an instruction leading message term element           */
/******************************************************************************/
{
  RexxToken   *token;                  /* current working token             */
  RexxObject  *term;                   /* working term                      */
  RexxObject  *start;                  /* starting term                     */
  INT          classId;                /* token class                       */

  size_t mark = markPosition();       // save the current position so we can reset cleanly

  start = this->subTerm(TERM_EOC);     /* get the first term of instruction */
  this->holdObject(start);             /* save the starting term            */
  term = OREF_NULL;                    /* default to no term                */
  token = nextToken();                 /* get the next token                */
  classId = token->classId;            /* get the token class               */
                                       /* while cascading message sends     */
  while (classId == TOKEN_TILDE || classId == TOKEN_DTILDE || classId == TOKEN_SQLEFT ) {
    if (classId == TOKEN_SQLEFT)       /* left bracket form?                */
      term = this->collectionMessage(token, start, TERM_EOC);
    else
                                       /* process a message term            */
      term = this->message(start, classId, TERM_EOC);
    start = term;                      /* set for the next pass             */
    token = nextToken();               /* get the next token                */
    classId = token->classId;          /* get the token class               */
  }
  previousToken();                     /* push this term back               */
  // if this was not a valid message term, reset the position to the beginning
  if (term == OREF_NULL)
  {
      resetPosition(mark);                 // reset back to the entry conditions
  }
                                       /* return the message term (returns  */
  return term;                         /* OREF_NULL if not a message term)  */
}

RexxObject *RexxSource::messageSubterm(
  int   terminators )                  /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off a message subterm within an expression                */
/******************************************************************************/
{
  RexxToken   *token;                  /* current working token             */
  RexxObject  *term;                   /* working term                      */
  INT          classId;                /* token class                       */

  token = nextToken();                 /* get the next token                */
                                       /* this the expression end?          */
  if (this->terminator(terminators, token))
    return OREF_NULL;                  /* nothing to do here                */
                                       /* have potential prefix operator?   */
  if (token->classId == TOKEN_OPERATOR) {

    switch (token->subclass) {         /* handle prefix operators as terms  */

      case OPERATOR_PLUS:              /* prefix plus                       */
      case OPERATOR_SUBTRACT:          /* prefix minus                      */
      case OPERATOR_BACKSLASH:         /* prefix backslash                  */
                                       /* handle following term             */
        term = this->messageSubterm(terminators);
        if (term == OREF_NULL)         /* nothing found?                    */
                                       /* this is an error                  */
          report_error_token(Error_Invalid_expression_prefix, token);
                                       /* create the new operator term      */
        term = (RexxObject *)new RexxUnaryOperator(token->subclass, term);
        break;

      default:                         /* other operators not allowed here  */
                                       /* this is an error                  */
        report_error_token(Error_Invalid_expression_general, token);
    }
  }

  else {                               /* non-prefix operator code          */
    previousToken();                   /* put back the first token          */
    term = this->subTerm(TERM_EOC);    /* get the first term of instruction */
    this->holdObject(term);            /* save the starting term            */
    token = nextToken();               /* get the next token                */
    classId = token->classId;          /* get the token class               */
                                       /* while cascading message sends     */
    while (classId == TOKEN_TILDE || classId == TOKEN_DTILDE || classId == TOKEN_SQLEFT ) {
      if (classId == TOKEN_SQLEFT)     /* left bracket form?                */
        term = this->collectionMessage(token, term, TERM_EOC);
      else
                                       /* process a message term            */
        term = this->message(term, classId, TERM_EOC);
//      this->holdObject(term);          /* lock the term                   */
      /* The above hold is obsolete (even bad) because both collectionMessage and message "hold" the new term */
      token = nextToken();             /* get the next token                */
      classId = token->classId;        /* get the token class               */
    }
    previousToken();                   /* push this term back               */
  }
                                       /* return the message term (returns  */
  return term;                         /* OREF_NULL if not a message term)  */
}

RexxObject *RexxSource::subTerm(
  int   terminators )                  /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off a subterm of an expression, from simple ones like     */
/*            variable names, to more complex such as message sends           */
/******************************************************************************/
{
  RexxToken    *token;                 /* current token being processed     */
  RexxObject   *term;                  /* parsed out term                   */
  RexxToken    *second;                /* second token of term              */

  token = nextToken();                 /* get the next token                */
                                       /* this the expression end?          */
  if (this->terminator(terminators, token))
    return OREF_NULL;                  /* nothing to do here                */

  switch (token->classId) {            /* process based on token type       */

    case  TOKEN_LEFT:                  /* have a left parentheses           */
                                       /* get the subexpression             */
      term = this->subExpression(((terminators | TERM_RIGHT) & ~TERM_SQRIGHT));
      if (term == OREF_NULL)           /* nothing found?                    */
                                       /* this is an error                  */
        report_error_token(Error_Invalid_expression_general, token);
      second = nextToken();            /* get the terminator token          */
                                       /* not terminated by a right paren?  */
      if (second->classId != TOKEN_RIGHT)
                                       /* this is an error                  */
        report_error_position(Error_Unmatched_parenthesis_paren, token);
      break;

    case  TOKEN_SYMBOL:                /* Symbol in the expression          */
    case  TOKEN_LITERAL:               /* Literal in the expression         */
      second = nextToken();            /* get the next token                */
                                       /* have a function call?             */
      if (second->classId == TOKEN_LEFT)
                                       /* process the function call         */
        term = this->function(second, token, terminators);
      else {
        previousToken();               /* push the token back               */
        term = this->addText(token);   /* variable or literal access        */
      }
      break;

    case  TOKEN_RIGHT:                 /* have a right parentheses          */
                                       /* this is an error here             */
      report_error(Error_Unexpected_comma_paren);
      break;

    case  TOKEN_COMMA:                 /* have a comma                      */
                                       /* this is an error here             */
      report_error(Error_Unexpected_comma_comma);
      break;

    case  TOKEN_SQRIGHT:               /* have a right square bracket       */
                                       /* this is an error here             */
      report_error(Error_Unexpected_comma_bracket);
      break;

    case  TOKEN_OPERATOR:              /* operator token                    */
      switch (token->subclass) {       /* handle prefix operators as terms  */

        case OPERATOR_PLUS:            /* prefix plus                       */
        case OPERATOR_SUBTRACT:        /* prefix minus                      */
        case OPERATOR_BACKSLASH:       /* prefix backslash                  */
          previousToken();             /* put the token back                */
          return OREF_NULL;            /* just return null (processed later)*/

        default:                       /* other operators not allowed here  */
                                       /* this is an error                  */
          report_error_token(Error_Invalid_expression_general, token);
      }
      break;

    default:                           /* unknown thing in expression       */
                                       /* this is an error                  */
      report_error_token(Error_Invalid_expression_general, token);
  }
  return term;                         /* return this term                  */
}

void RexxSource::pushTerm(
    RexxObject *term )                 /* term to push                      */
/******************************************************************************/
/* Function:  Push a term onto the expression term stack                      */
/******************************************************************************/
{
  this->terms->push(term);             /* push the term on the stack      */
  this->currentstack++;                /* step the stack depth              */
                                       /* new "high water" mark?            */
  if (this->currentstack > this->maxstack)
                                       /* make it the highest point         */
    this->maxstack = this->currentstack;
}

RexxObject *RexxSource::popTerm()
/******************************************************************************/
/* Function:  Pop a term off of the expression term stack                     */
/******************************************************************************/
{
  RexxObject *term;                    /* returned term                   */

  this->currentstack--;                /* reduce the size count           */
  term = this->terms->pop();           /* pop the term                    */
  this->holdObject(term);              /* give it a little protection     */
  return term;                         /* and return it                   */
}

RexxObject *RexxSource::popNTerms(
     size_t count )                    /* number of terms to pop            */
/******************************************************************************/
/* Function:  Pop multiple terms off of the operator stack                    */
/******************************************************************************/
{
  RexxObject *result;                  /* final popped element              */

  this->currentstack -= count;         /* reduce the size count             */
  while (count--)                      /* while more to remove              */
    result = this->terms->pop();       /* pop the next item               */
  this->holdObject(result);            /* protect this a little             */
  return result;                       /* and return it                     */
}

void RexxSource::isExposeValid()
/******************************************************************************/
/* Function:  Validate placement of an EXPOSE instruction.  The EXPOSE must   */
/*            be the first instruction and this must not be an interpret      */
/*            invocation.  NOTE:  labels are not allowed preceeding, as that  */
/*            will give a target for SIGNAL or CALL that will result in an    */
/*            invalid EXPOSE execution.                                       */
/******************************************************************************/
{
  if (this->flags&_interpret)          /* is this an interpret?             */
                                       /* give the interpret error          */
    report_error(Error_Translation_expose_interpret);
                                       /* not the first instruction?        */
  if (this->last->getType() != KEYWORD_FIRST)
                                       /* general placement error           */
    report_error(Error_Translation_expose);
}

RexxArray  *RexxSource::words(
    RexxString *string)                /* target string                     */
/******************************************************************************/
/* Function:  Break up a string into an array of words for parsing and        */
/*            interpretation.                                                 */
/******************************************************************************/
{
  RexxQueue  *wordlist;                /* created list of words             */
  RexxArray  *wordarray;               /* array version of the list         */
  RexxString *word;                    /* current word                      */
  size_t      count;                   /* count of words                    */
  size_t      i;                       /* loop counter                      */

  wordlist = this->subTerms;           /* use the subterms list             */
                                       /* get the first word                */
  word = ((RexxString *)(string->word(IntegerOne)))->upper();
  word = this->commonString(word);     /* get the common version of this    */
  wordlist->push(word);                /* add to the word list              */
  count = 1;                           /* one word so far                   */
                                       /* while still more words            */
  for (i = 3, word = (RexxString *)(string->word(IntegerTwo)); word->length != 0; i++) {
    count++;                           /* have another word                 */
    word = this->commonString(word);   /* get the common version of this    */
    wordlist->push(word);              /* add this word to the list         */
                                       /* get the next word                 */
    word = (RexxString *)string->word(new_integer(i));
  }
  wordarray = new_array(count);        /* get an array return value         */
  while (count > 0)                    /* while more words                  */
                                       /* copy into the array               */
    wordarray->put(wordlist->pop(), count--);
  return wordarray;                    /* return as an array                */
}

void RexxSource::errorCleanup()
/******************************************************************************/
/* Function:  Free up all of the parsing elements because of an error         */
/******************************************************************************/
{
  this->cleanup();                     /* do needed cleanup                 */
  discard_hold(this);                  /* release lock on the source        */
}

void RexxSource::error(
     int   errorcode)                  /* error to raise                    */
/******************************************************************************/
/* Function:  Raise an error caused by source translation problems.           */
/******************************************************************************/
{
  LOCATIONINFO    location;            /* error location                    */

                                       /* get the clause location           */
  this->clause->getLocation(&location);
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  CurrentActivity->raiseException(errorcode, &location, this, OREF_NULL, OREF_NULL, OREF_NULL);
}

void RexxSource::errorLine(
     int   errorcode,                  /* error to raise                    */
     RexxInstruction *instruction)     /* instruction for the line number   */
/******************************************************************************/
/* Function:  Raise an error where one of the error message substitutions is  */
/*            the line number of another instruction object                   */
/******************************************************************************/
{
  LOCATIONINFO instruction_location;   /* location of the token             */
  LOCATIONINFO    location;            /* error location                    */

                                       /* get the clause location           */
  this->clause->getLocation(&location);
                                       /* get the instruction location      */
  instruction->getLocation(&instruction_location);
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  CurrentActivity->raiseException(errorcode, &location, this, OREF_NULL, new_array1(new_integer(instruction_location.line)), OREF_NULL);
}

void RexxSource::errorPosition(
     int        errorcode,             /* error to raise                    */
     RexxToken *token )                /* token value for description       */
/******************************************************************************/
/* Function:  Raise an error, displaying the location of a token associated   */
/*            with the error.                                                 */
/******************************************************************************/
{
  LOCATIONINFO    token_location;      /* location of the token             */
  LOCATIONINFO    location;            /* error location                    */

  this->clause->getLocation(&location);/* get the clause location           */
  token->getLocation(&token_location); /* get the token location            */
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  CurrentActivity->raiseException(errorcode, &location, this, OREF_NULL, new_array2(new_integer(token_location.offset), new_integer(token_location.line)), OREF_NULL);
}

void RexxSource::errorToken(
     int        errorcode,             /* error to raise                    */
     RexxToken *token )                /* token value for description       */
/******************************************************************************/
/* Function:  Raise an error, displaying the value of a token in the error    */
/*            message.                                                        */
/******************************************************************************/
{
  RexxString     *value;               /* token value                       */
  LOCATIONINFO    location;            /* error location                    */

  this->clause->getLocation(&location);/* get the clause location           */

  value = token->value;                /* get the token value               */
  if (value == OREF_NULL) {            /* no value?                         */
    switch (token->classId) {

      case TOKEN_BLANK:                /* blank operator                    */
        value = new_string(" ", 1);    /* use a blank                       */
        break;

      case TOKEN_EOC:                  /* source terminator                 */
        value = new_string(";", 1);    /* use a semicolon                   */
        break;

      case TOKEN_COMMA:                /* comma                             */
        value = new_string(",", 1);    /* display a comma                   */
        break;

      case TOKEN_LEFT:                 /* left parentheses                  */
        value = new_string("(", 1);    /* display that                      */
        break;

      case TOKEN_RIGHT:                /* right parentheses                 */
        value = new_string(")", 1);    /* display that                      */
        break;

      case TOKEN_SQLEFT:               /* left square bracket               */
        value = new_string("[", 1);    /* display that                      */
        break;

      case TOKEN_SQRIGHT:              /* right square bracket              */
        value = new_string("]", 1);    /* display that                      */
        break;

      case TOKEN_COLON:                /* colon                             */
        value = new_string(":", 1);    /* display that                      */
        break;

      case TOKEN_TILDE:                /* twiddle operator                  */
        value = new_string("~", 1);    /* display that                      */
        break;

      case TOKEN_DTILDE:               /* double twiddle operator           */
        value = new_string("~~", 2);   /* display that                      */
        break;

      case TOKEN_DCOLON:               /* double colon operator             */
        value = new_string("::", 2);   /* display that                      */
        break;

      default:                         /* ????? token                       */
                                       /* just use a null string            */
        value = (RexxString *)OREF_NULLSTRING;
        break;
    }
  }
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  CurrentActivity->raiseException(errorcode, &location, this, OREF_NULL, new_array1(value), OREF_NULL);
}

void RexxSource::error(
     int         errorcode,            /* error to raise                    */
     RexxObject *value )               /* value for description             */
/******************************************************************************/
/* Function:  Issue an error message with a single substitution parameter.    */
/******************************************************************************/
{
  LOCATIONINFO    location;            /* error location                    */

  this->clause->getLocation(&location);/* get the clause location           */
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  CurrentActivity->raiseException(errorcode, &location, this, OREF_NULL, new_array1(value), OREF_NULL);
}

void RexxSource::error(
     int         errorcode,            /* error to raise                    */
     RexxObject *value1,               /* first value for description       */
     RexxObject *value2 )              /* second value for description      */
/******************************************************************************/
/* Function:  Issue an error message with two substitution parameters.        */
/******************************************************************************/
{
  LOCATIONINFO    location;            /* error location                    */

  this->clause->getLocation(&location);/* get the clause location           */
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  CurrentActivity->raiseException(errorcode, &location, this, OREF_NULL, new_array2(value1, value2), OREF_NULL);
}

void RexxSource::error(
     int         errorcode,            /* error to raise                    */
     RexxObject *value1,               /* first value for description       */
     RexxObject *value2,               /* second value for description      */
     RexxObject *value3 )              /* third value for description       */
/****************************************************************************/
/* Function:  Issue an error message with three substitution parameters.    */
/****************************************************************************/
{
  LOCATIONINFO    location;            /* error location                    */

  this->clause->getLocation(&location);/* get the clause location           */
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  CurrentActivity->raiseException(errorcode, &location, this, OREF_NULL, new_array3(value1, value2, value3), OREF_NULL);
}

void RexxSource::blockError(
    RexxInstruction *instruction )     /* unclosed control instruction      */
/******************************************************************************/
/* Function:  Raise an error for an unclosed block instruction.               */
/******************************************************************************/
{
  LOCATIONINFO  location;              /* location of last instruction      */

  this->last->getLocation(&location);  /* get the instruction location      */
  this->clause->setLocation(&location);/* report as the last instruction    */

  switch (instruction->getType()) {    /* issue proper message type         */
    case KEYWORD_DO:                   /* incomplete DO                     */
                                       /* raise an error                    */
      report_error_line(Error_Incomplete_do_do, instruction);
      break;

    case KEYWORD_SELECT:               /* incomplete SELECT                 */
      report_error_line(Error_Incomplete_do_select, instruction);
      break;

    case KEYWORD_OTHERWISE:            /* incomplete SELECT                 */
      report_error_line(Error_Incomplete_do_otherwise, instruction);
      break;

    case KEYWORD_IF:                   /* incomplete IF                     */
    case KEYWORD_IFTHEN:               /* incomplete IF                     */
    case KEYWORD_WHENTHEN:             /* incomplete IF                     */
      report_error_line(Error_Incomplete_do_then, instruction);
      break;

    case KEYWORD_ELSE:                 /* incomplete ELSE                   */
      report_error_line(Error_Incomplete_do_else, instruction);
      break;
  }
}

void *RexxSource::operator new (size_t size)
/******************************************************************************/
/* Function:  Create a new translator object from an array                    */
/******************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */

                                       /* Get new object                    */
  newObject = new_object(sizeof(RexxSource));
  ClearObjectLength(newObject,sizeof(RexxSource)); /* clear object          */
                                       /* Give new object its behaviour     */
  BehaviourSet(newObject, TheSourceBehaviour);
  return newObject;                    /* return the new object             */
}

RexxSource *RexxSource::classNewBuffered(
    RexxString *programname,           /* name of the program               */
    RexxBuffer *source_buffer )        /* buffer containing the source      */
/******************************************************************************/
/* Function:  Create a new source object, using buffered input                */
/******************************************************************************/
{
  RexxSource *newObject;               /* newly created source object       */

  save(source_buffer);                 /* protect the buffer                */
  newObject = new RexxSource (programname, OREF_NULL);
  save(newObject);                     // protect this while parsing
                                       /* process the buffering             */
  newObject->initBuffered((RexxObject *)source_buffer);
  discard(source_buffer);              /* release the buffer protect        */
  discard_hold(newObject);             // and also release the source object protection
  return newObject;                    /* return the new source object      */
}

RexxSource *RexxSource::classNewFile(
    RexxString *programname )          /* program file name                 */
/******************************************************************************/
/* Function:  Create a source object from a file.                             */
/******************************************************************************/
{
  RexxSource *newObject;               /* newly created source object       */

                                       /* create a new source object        */
  newObject = new RexxSource (programname, OREF_NULL);
  save(newObject);                     // protect this while parsing
  newObject->initFile();               /* go process the file               */
  discard_hold(newObject);             // and also release the source object protection
  return newObject;                    /* return the new object             */
}

RexxObject *RexxSource::sourceNewObject(
    size_t        size,                /* Object size                       */
    RexxBehaviour *behaviour,          /* Object's behaviour                */
    INT            type )              /* Type of instruction               */
/******************************************************************************/
/* Function:  Create a "raw" translator instruction object                    */
/******************************************************************************/
{
  RexxObject *newObject;               /* newly created object              */

  newObject = new_object(size);        /* Get new object                    */
  BehaviourSet(newObject, behaviour);  /* Give new object its behaviour     */
                                       /* do common initialization          */
  new ((void *)newObject) RexxInstruction (this->clause, type);
                                       /* now protect this                  */
  OrefSet(this, this->currentInstruction, (RexxInstruction *)newObject);
  return newObject;                    /* return the new object             */
}

void RexxSource::parseTraceSetting(
    RexxString *value,                 /* string with trace setting         */
    PINT        setting,               /* new trace setting                 */
    PINT        debug )                /* new debug mode setting            */
/******************************************************************************/
/* Function:  Process a trace setting                                         */
/******************************************************************************/
{
  size_t   length;                     /* length of value string            */
  size_t   position;                   /* position within the string        */

  *setting = TRACE_IGNORE;             /* don't change trace setting yet    */
  *debug = DEBUG_IGNORE;               /* and the default debug change      */

  length = value->length;              /* get the string length             */
  if (length == 0) {                   /* null string?                      */
    *setting = TRACE_NORMAL;           /* use default trace setting         */
    *debug = DEBUG_OFF;                /* turn off debug mode               */
  }
  else {
    for (position = 0;                 /* start at the beginning            */
         position < length;            /* while more length to process      */
         position++) {                 /* step one each character           */

                                       /* process the next character        */
      switch (value->getChar(position)) {

        case '?':                      /* debug toggle character            */
                                       /* already toggling?                 */
          if (*debug == DEBUG_TOGGLE)
            *debug = DEBUG_IGNORE;     /* this is back to no change at all  */
          else
            *debug = DEBUG_TOGGLE;     /* need to toggle the debug mode     */
          continue;                    /* go loop again                     */

        case 'a':                      /* TRACE ALL                         */
        case 'A':
          *setting = TRACE_ALL;
          break;

        case 'c':                      /* TRACE COMMANDS                    */
        case 'C':
          *setting = TRACE_COMMANDS;
          break;

        case 'l':                      /* TRACE LABELS                      */
        case 'L':
          *setting = TRACE_LABELS;
          break;

        case 'e':                      /* TRACE ERRORS                      */
        case 'E':
          *setting = TRACE_ERRORS;
          break;

        case 'f':                      /* TRACE FAILURES                    */
        case 'F':
          *setting = TRACE_FAILURES;
          break;

        case 'n':                      /* TRACE NORMAL                      */
        case 'N':
          *setting = TRACE_NORMAL;
          break;

        case 'o':                      /* TRACE OFF                         */
        case 'O':
          *setting = TRACE_OFF;
          break;

        case 'r':                      /* TRACE RESULTS                     */
        case 'R':
          *setting = TRACE_RESULTS;
          break;

        case 'i':                      /* TRACE INTERMEDIATES               */
        case 'I':
          *setting = TRACE_INTERMEDIATES;
          break;

        default:                       /* unknown trace setting             */
          /* call report_error1 instead of report_exception1 to  */
          /* include line number and source information          */
          if (this->clause)           /* call different error routines      */
            report_error1(Error_Invalid_trace_trace, value->extract(position, 1));
          else
            report_exception1(Error_Invalid_trace_trace, value->extract(position, 1));
          break;
      }
      break;                           /* non-prefix char found             */
    }
  }
}

size_t RexxSource::processVariableList(
  INT        type )                    /* type of instruction               */
/****************************************************************************/
/* Function:  Process a variable list for PROCEDURE, DROP, and USE          */
/****************************************************************************/
{
  RexxToken   *token;                  /* current working token             */
  INT          list_count;             /* count of variables in list        */
  RexxObject  *retriever;              /* variable retriever object         */

  list_count = 0;                      /* no variables yet                  */
  token = nextReal();                  /* get the first variable            */

  while (token->classId != TOKEN_EOC) {/* while not at the end of the clause*/
                                       /* have a variable name?             */
    if (token->classId == TOKEN_SYMBOL) {
                                       /* non-variable symbol?              */
      if (token->subclass == SYMBOL_CONSTANT)
                                       /* report the error                  */
        report_error_token(Error_Invalid_variable_number, token);
      else if (token->subclass == SYMBOL_DUMMY)
                                       /* report the error                  */
        report_error_token(Error_Invalid_variable_period, token);
      retriever = this->addText(token);/* get a retriever for this          */
      this->subTerms->push(retriever); /* add to the variable list          */
      if (type == KEYWORD_EXPOSE)      /* this an expose operation?         */
        this->expose(token->value);    /* add to the expose list too        */
      list_count++;                    /* record the variable               */
    }
                                       /* have a variable reference         */
    else if (token->classId == TOKEN_LEFT) {
      list_count++;                    /* record the variable               */
      token = nextReal();              /* get the next token                */
                                       /* not a symbol?                     */
      if (token->classId != TOKEN_SYMBOL)
                                       /* must be a symbol here             */
        report_error(Error_Symbol_expected_varref);
                                       /* non-variable symbol?              */
      if (token->subclass == SYMBOL_CONSTANT)
                                       /* report the error                  */
        report_error_token(Error_Invalid_variable_number, token);
      else if (token->subclass == SYMBOL_DUMMY)
                                       /* report the error                  */
        report_error_token(Error_Invalid_variable_period, token);

      retriever = this->addText(token);/* get a retriever for this          */
                                       /* make this an indirect reference   */
      retriever = (RexxObject *)new RexxVariableReference((RexxVariableBase *)retriever);
      this->subTerms->queue(retriever);/* add to the variable list          */
      this->currentstack++;            /* account for the varlists          */

      token = nextReal();              /* get the next token                */
      if (token->classId == TOKEN_EOC) /* nothing following?                */
                                       /* report the missing paren          */
        report_error(Error_Variable_reference_missing);
                                       /* must be a right paren here        */
      else if (token->classId != TOKEN_RIGHT)
                                       /* this is an error                  */
        report_error_token(Error_Variable_reference_extra, token);
    }
                                       /* something bad....                 */
    else {                             /* this is invalid                   */
      if (type == KEYWORD_DROP)        /* DROP form?                        */
                                       /* give appropriate message          */
        report_error(Error_Symbol_expected_drop);
      else                             /* else give message for EXPOSEs     */
        report_error(Error_Symbol_expected_expose);
    }
    token = nextReal();                /* get the next variable             */
  }
  if (list_count == 0) {               /* no variables?                     */
    if (type == KEYWORD_DROP)          /* DROP form?                        */
                                       /* give appropriate message          */
      report_error(Error_Symbol_expected_drop);
    else                               /* else give message for EXPOSEs     */
      report_error(Error_Symbol_expected_expose);
  }
  return list_count;                   /* return the count                  */
}

RexxObject *RexxSource::parseConditional(
     PINT  condition_type,             /* type of condition                 */
     INT   error_message )             /* extra "stuff" error message       */
/******************************************************************************/
/* Function:  Allow for WHILE or UNTIL keywords following some other looping  */
/*            construct.  This returns SUBKEY_WHILE or SUBKEY_UNTIL to flag   */
/*            the caller that a conditional has been used.                    */
/******************************************************************************/
{
  RexxToken  *token;                   /* current working token             */
  INT         keyword;                 /* keyword of parsed conditional     */
  RexxObject *condition;               /* parsed out condition              */

  condition = OREF_NULL;               /* default to no condition           */
  keyword = 0;                         /* no conditional yet                */
  token = nextReal();                  /* get the terminator token          */

  if (token->classId != TOKEN_EOC) {   /* real end of instruction?          */
                                       /* may have WHILE/UNTIL              */
    if (token->classId == TOKEN_SYMBOL) {
                                       /* process the symbol                */
       switch (this->subKeyword(token) ) {

       case SUBKEY_WHILE:              /* DO WHILE exprw                    */
                                       /* get next subexpression            */
         condition = this->parseLogical(OREF_NULL, TERM_COND);
         if (condition == OREF_NULL) /* nothing really there?             */
                                       /* another invalid DO                */
           report_error(Error_Invalid_expression_while);
         token = nextToken();          /* get the terminator token          */
                                       /* must be end of instruction        */
         if (token->classId != TOKEN_EOC)
           report_error(Error_Invalid_do_whileuntil);
         keyword = SUBKEY_WHILE;       /* this is the WHILE form            */
         break;

       case SUBKEY_UNTIL:              /* DO UNTIL expru                    */
                                       /* get next subexpression            */
                                       /* get next subexpression            */
         condition = this->parseLogical(OREF_NULL, TERM_COND);

         if (condition == OREF_NULL)   /* nothing really there?             */
                                       /* another invalid DO                */
           report_error(Error_Invalid_expression_until);
         token = nextToken();          /* get the terminator token          */
                                       /* must be end of instruction        */
         if (token->classId != TOKEN_EOC)
           report_error(Error_Invalid_do_whileuntil);
         keyword = SUBKEY_UNTIL;       /* this is the UNTIL form            */
         break;

       default:                        /* nothing else is valid here!       */
                                       /* raise an error                    */
         report_error_token(error_message, token);
         break;
       }
    }
  }
  if (condition_type != NULL)          /* need the condition type?          */
    *condition_type = keyword;         /* set the keyword                   */
  return condition;                    /* return the condition expression   */
}


/**
 * Parse off a "logical list expression", consisting of a
 * list of conditionals separated by commas.
 *
 * @param terminators
 *               The set of terminators for this logical context.
 *
 * @return OREF_NULL if no expressions is found, a single expression
 *         element if a single expression is located, and a complex
 *         logical expression operator for a list of expressions.
 */
RexxObject *RexxSource::parseLogical(RexxToken *first, int terminators)
{
    size_t count = argList(first, terminators);
    // arglist has swallowed the terminator token, so we need to back up one.
    previousToken();
    // let the caller deal with completely missing expressions
    if (count == 0)
    {
        return OREF_NULL;
    }

    // just a single item (common)?  Just pop the top item and return it.
    if (count == 1)
    {
        return subTerms->pop();
    }

                                       /* create a new function item        */
    return (RexxObject *)new (count) RexxExpressionLogical(this, count, this->subTerms);
}
