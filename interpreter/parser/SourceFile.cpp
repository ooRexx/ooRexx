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
/* Primitive Translator Source File Class                                     */
/*                                                                            */
/******************************************************************************/
#include <ctype.h>
#include <string.h>
#include "RexxCore.h"
#include "StringClass.hpp"
#include "ArrayClass.hpp"
#include "DirectoryClass.hpp"
#include "BufferClass.hpp"
#include "RexxActivity.hpp"
#include "RexxActivation.hpp"
#include "MethodClass.hpp"
#include "RexxNativeCode.hpp"
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
#include "ProtectedObject.hpp"
#include "CPPCode.hpp"
#include "SystemInterpreter.hpp"
#include "PackageClass.hpp"
#include "InterpreterInstance.hpp"
#include "ClassDirective.hpp"
#include "LibraryDirective.hpp"
#include "RequiresDirective.hpp"
#include "PackageManager.hpp"
#include "SysFileSystem.hpp"
#include "RoutineClass.hpp"
#include "ActivationFrame.hpp"
#include "StackFrameClass.hpp"

#define HOLDSIZE         60            /* room for 60 temporaries           */

typedef struct _LINE_DESCRIPTOR {
  size_t position;                     /* position within the buffer        */
  size_t length;                       /* length of the line                */
} LINE_DESCRIPTOR;                     /* line within a source buffer       */

#define line_delimiters "\r\n"         /* stream file line end characters   */
#define ctrl_z 0x1a                    // the end of file marker

/**
 * Create a source object with source provided from an array.
 *
 * @param programname
 *               The name of the program.
 * @param source_array
 *               The array of the source lines.
 */
RexxSource::RexxSource(RexxString *programname, RexxArray  *source_array)
{
                                         /* fill in the name                  */
    setProgramName(programname);
    /* fill in the source array          */
    OrefSet(this, this->sourceArray, source_array);
    /* fill in the source size           */
    this->line_count = sourceArray->size();
    this->position(1, 0);              /* set position at the first line    */
}


/**
 * Create a source object with source provided from a buffer.
 *
 * @param programname
 *               The name of the program.
 * @param source_buffer
 *               The source buffer holding the source data.
 */
RexxSource::RexxSource(RexxString *programname, RexxBuffer *source_buffer)
{
                                         /* fill in the name                  */
    setProgramName(programname);
    // we require a bit of protection while doing this
    ProtectedObject p(this);
    // initialize from the buffer data
    initBuffered(source_buffer);
}


/**
 * Create a source object with source provided from a a data buffer
 * (not a buffer object).
 *
 * @param programname
 *               The name of the program.
 * @param data   The data buffer pointer.
 * @param length the size of the source buffer.
 */
RexxSource::RexxSource(RexxString *programname, const char *data, size_t length)
{
                                         /* fill in the name                  */
    setProgramName(programname);
    // we require a bit of protection while doing this
    ProtectedObject p(this);
    // initialize from the buffer data
    initBuffered(new_buffer(data, length));
}


/**
 * Create a source object with source provided from a filo.
 *
 * @param programname
 *               The name of the program (also the file name)
 */
RexxSource::RexxSource(RexxString *programname)
{
                                         /* fill in the name                  */
    setProgramName(programname);
    // we require a bit of protection while doing this
    ProtectedObject p(this);
    // read the file data and initialize.
    initFile();
}


void RexxSource::initBuffered(
    RexxBuffer *source_buffer)         /* containing source buffer          */
/******************************************************************************/
/* Function:  Initialize a source object using the entire source as a         */
/*            stream buffer                                                   */
/******************************************************************************/
{
    LINE_DESCRIPTOR     descriptor;      /* line description                  */
    const char *scan;                    /* line scanning pointer             */
    const char *_current;                /* current scan location             */
    char *start;                   /* start of the buffer               */
    size_t length;                       /* length of the buffer              */

    extractNameInformation();            // make sure we have name information to work with
                                         /* set the source buffer             */
    OrefSet(this, this->sourceBuffer, source_buffer);
    RexxSmartBuffer *indices = new RexxSmartBuffer(1024);
    ProtectedObject p(indices);
    /* point to the data part            */
    start = this->sourceBuffer->getData();
    /* get the buffer length             */
    length = this->sourceBuffer->getDataLength();

    // neutralize shell '#!...'
    if (start[0] == '#' && start[1] == '!')
    {
        memcpy(start, "--", 2);
    }

    descriptor.position = 0;             /* fill in the "zeroth" position     */
    descriptor.length = 0;               /* and the length                    */
                                         /* add to the line list              */
    indices->copyData(&descriptor, sizeof(descriptor));
    this->line_count = 0;                /* start with zero lines             */
                                         /* look for an EOF mark              */
    scan = (const char *)memchr(start, ctrl_z, length);
    if (scan != NULL)                    /* found one?                        */
    {
        length = scan - start;             /* reduce the length                 */
    }
    _current = start;                    /* start at the beginning            */
    while (length != 0)
    {                /* loop until all done               */
        this->line_count++;                /* add in another line               */
                                           /* set the start position            */
        descriptor.position = _current - start;
        /* scan for a important character    */
        scan = Utilities::locateCharacter(_current, line_delimiters, length);
        /* need to skip over null chars      */
        while (scan != OREF_NULL && *scan == '\0')
        {
            /* scan for a linend                 */
            scan = Utilities::locateCharacter(scan + 1, line_delimiters, length - (scan - _current - 1));
        }
        if (scan == NULL)
        {                /* not found, go to the end          */
            _current = _current + length;    /* step to the end                   */
            descriptor.length = length;      /* use the entire line               */
            length = 0;                      /* nothing left to process           */
        }
        else
        {
            /* calculate this line length        */
            descriptor.length = scan - _current;
            /* adjust scan at line end           */
            if (*scan == line_delimiters[0])
            {/* CR encountered                   */
                scan++;                        /* step the scan position            */
                /* now check for LF */
                if (length > (size_t)(scan - _current))
                {
                    if (*scan != '\0' && *scan == line_delimiters[1])     /*          */
                    {
                        scan++;                    /* step again, if required           */
                    }
                }
            }
            else                             /* just a LF                         */
            {
                scan++;                        /* step the scan position            */
            }

            length -= scan - _current;       /* reduce the length                 */
            _current = scan;                 /* copy the scan pointer             */
        }
        /* add to the line list              */
        indices->copyData(&descriptor, sizeof(descriptor));
    }
    /* throw away the buffer "wrapper"   */
    OrefSet(this, this->sourceIndices, indices->getBuffer());
    this->position(1, 0);                /* set position at the first line    */
}


void RexxSource::initFile()
/******************************************************************************/
/* Function:  Initialize a source object, reading the source from a file      */
/******************************************************************************/
{
                                         /* load the program file             */
    RexxBuffer *program_source = (RexxBuffer *)SystemInterpreter::readProgram(programName->getStringData());
    if (program_source == OREF_NULL)     /* Program not found or read error?  */
    {
        /* report this                       */
        reportException(Error_Program_unreadable_name, this->programName);
    }

#ifdef SCRIPTING
    if (program_source->getDataLength() > 9)
    {
        char begin[10];
        char end[4];
        // check, if XML comments have to be removed from the script... (engine situation)
        memcpy(begin,program_source->getData(), 9);
        // hashvalue is the length of the buffer
        memcpy(end, program_source->getData()+ (program_source->getDataLength()-3), 3);
        begin[9]=end[3]=0x00;
        if (!Utilities::strCaselessCompare("<![CDATA[",begin) && !Utilities::strCaselessCompare("]]>",end))
        {
            memcpy(program_source->getData(), "         ", 9);
            memcpy(program_source->getData() + (program_source->getDataLength() - 3), "   ", 3);
        }
    }
#endif

    /* save the returned buffer          */
    OrefSet(this, this->sourceBuffer, program_source);
    /* go process the buffer now         */
    this->initBuffered(this->sourceBuffer);
}


/**
 * Extract various bits of the source name to give us directory,
 * extension and file portions to be used for searches for additional
 * files.
 */
void RexxSource::extractNameInformation()
{
    if (programName == OREF_NULL)
    {
        return;
    }

    OrefSet(this, this->programDirectory, SysFileSystem::extractDirectory(programName));
    OrefSet(this, this->programExtension, SysFileSystem::extractExtension(programName));
    OrefSet(this, this->programFile, SysFileSystem::extractFile(programName));
}


/**
 * Set a program name for this source object.  Usually used after
 * a program restore to update the restored routine object.  This
 * will also update the extension and directory information.
 *
 * @param name   The new program name.
 */
void RexxSource::setProgramName(RexxString *name)
{
    OrefSet(this, this->programName, name);
    extractNameInformation();
}

bool RexxSource::reconnect()
/******************************************************************************/
/* Function:  Attempt to reconnect to the original source code file           */
/******************************************************************************/
{
    if (!(this->flags&reclaim_possible)) /* no chance of getting this?        */
    {
        return false;                      /* just get out of here              */
    }
    this->initFile();                    /* go reinit this                    */
    return true;                         /* give back the success return      */
}

void RexxSource::setReconnect()
/******************************************************************************/
/* Function:  Allow a source reconnect to occur                               */
/******************************************************************************/
{
  this->flags |= reclaim_possible;     /* we have a shot at this!           */
}

void RexxSource::interpretLine(size_t _line_number)
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
  this->line_count = _line_number;     /* size is now the line number       */
  this->line_number = _line_number;    /* we are now on line "nn of nn"     */
                                       /* remember for positioning          */
  this->interpret_adjust = _line_number - 1;
}

void RexxSource::needVariable(
    RexxToken  *token)                 /* current token                     */
/******************************************************************************/
/* Function:  validate that the current token is a variable token             */
/******************************************************************************/
{
    /* not a variable token?             */
    if (!token->isVariable())
    {
        /* begin with a dot?                 */
        if (token->value->getChar(0) == '.')
        {
            syntaxError(Error_Invalid_variable_period, token);
        }
        else
        {
            syntaxError(Error_Invalid_variable_number, token);
        }
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
      syntaxError(Error_Invalid_variable_number, token);
  }
}

bool RexxSource::terminator(
    int         terminators,           /* set of possible terminators       */
    RexxToken  *token)                 /* token being processed             */
/******************************************************************************/
/* Function:  test for a terminator token in the given context                */
/******************************************************************************/
{
    bool    endtoken;                    /* found the end flag                */

    endtoken = false;                    /* not found the end yet             */

    /* process based on terminator class */
    switch (token->classId)
    {

        case  TOKEN_EOC:                   /* found the end-of-clause           */
            endtoken = true;                 /* this is always an end marker      */
            break;

        case  TOKEN_RIGHT:                 /* found a right paren               */
            if (terminators&TERM_RIGHT)      /* terminate on this?                */
                endtoken = true;               /* set the flag                      */
            break;

        case  TOKEN_SQRIGHT:               /* found a right square bracket      */
            if (terminators&TERM_SQRIGHT)    /* terminate on this?                */
                endtoken = true;               /* set the flag                      */
            break;

        case  TOKEN_COMMA:                 /* found a comma                     */
            if (terminators&TERM_COMMA)      /* terminate on this?                */
                endtoken = true;               /* set the flag                      */
            break;

        case  TOKEN_SYMBOL:                /* have a symbol, need to resolve    */
            if (terminators&TERM_KEYWORD)
            {  /* need to do keyword checks?        */
               /* process based on the keyword      */
                switch (this->subKeyword(token))
                {

                    case SUBKEY_TO:              /* TO subkeyword                     */
                        if (terminators&TERM_TO)   /* terminate on this?                */
                            endtoken = true;         /* set the flag                      */
                        break;

                    case SUBKEY_BY:              /* BY subkeyword                     */
                        if (terminators&TERM_BY)   /* terminate on this?                */
                            endtoken = true;         /* set the flag                      */
                        break;

                    case SUBKEY_FOR:             /* FOR subkeyword                    */
                        if (terminators&TERM_FOR)  /* terminate on this?                */
                        {
                            endtoken = true;         /* set the flag                      */
                        }
                        break;

                    case SUBKEY_WHILE:           /* WHILE subkeyword                  */
                    case SUBKEY_UNTIL:           /* UNTIL subkeyword                  */
                        if (terminators&TERM_WHILE)/* terminate on this?                */
                            endtoken = true;         /* set the flag                      */
                        break;

                    case SUBKEY_WITH:            /* WITH subkeyword                   */
                        if (terminators&TERM_WITH) /* terminate on this?                */
                            endtoken = true;         /* set the flag                      */
                        break;

                    case SUBKEY_THEN:            /* THEN subkeyword                   */
                        if (terminators&TERM_THEN) /* terminate on this?                */
                            endtoken = true;         /* set the flag                      */
                        break;

                    default:                     /* not a terminator for others       */
                        break;
                }
            }
        default:                           /* not a terminator for others       */
            break;
    }
    if (endtoken)                        /* found the end one?                */
    {
        previousToken();                   /* push it back on the clause        */
    }
    return endtoken;                     /* return the true/false flag        */
}

void RexxSource::nextLine()
/******************************************************************************/
/* Function:  Advance the current position to the next source line            */
/******************************************************************************/
{
    if (this->clause)                    /* have a clause object?             */
    {
        /* record current position in clause */
        this->clause->setEnd(this->line_number, this->line_offset);
    }
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
    const char      *buffer_start;      /* start of source buffer            */
    RexxString       *new_line;         /* new line to scan                  */

    this->line_number = line;           /* set the line number               */
    this->line_offset = offset;         /* and the offset                    */
    /* past the end?                     */
    if (line > this->line_count)
    {
        this->current = OREF_NULL;        /* null out the current line         */
        this->current_length = 0;         /* tag this as a null line           */
    }
    else
    {
        /* working from an array?            */
        if (this->sourceArray != OREF_NULL)
        {
            /* get the next line                 */
            new_line = (RexxString *)(this->sourceArray->get(line - this->interpret_adjust));
            if (new_line == OREF_NULL)      /* missing line?                     */
            {
                /* this is an error                  */
                reportException(Error_Translation_invalid_line);
            }
            /* not working with a string?        */
            if (!isOfClass(String, new_line))
            {
                /* get this as a string              */
                new_line = (RexxString *)new_line->stringValue();
                if (new_line == TheNilObject) /* got back .nil?                    */
                {
                    /* this is an error                  */
                    reportException(Error_Translation_invalid_line);
                }
            }
            /* set the program pointer           */
            this->current = new_line->getStringData();
            /* get the string length             */
            this->current_length = new_line->getLength();
        }
        /* single buffer source              */
        else
        {
            /* get the descriptors pointer       */
            descriptors = (LINE_DESCRIPTOR *)(this->sourceIndices->getData());
            /* source buffered in a string?      */
            if (isOfClass(String, this->sourceBuffer))
            {
                /* point to the data part            */
                buffer_start = ((RexxString *)(this->sourceBuffer))->getStringData();
            }
            else
            {
                /* point to the data part            */
                buffer_start = this->sourceBuffer->getData();
            }
            /* calculate the line start          */
            this->current = buffer_start + descriptors[line - this->interpret_adjust].position;
            /* and get the length                */
            this->current_length = descriptors[line - this->interpret_adjust].length;
        }
    }
}

void RexxSource::live(size_t liveMark)
/******************************************************************************/
/* Perform garbage collection marking of a source object                      */
/******************************************************************************/
{
  memory_mark(this->parentSource);
  memory_mark(this->sourceArray);
  memory_mark(this->programName);
  memory_mark(this->programDirectory);
  memory_mark(this->programExtension);
  memory_mark(this->programFile);
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
  memory_mark(this->libraries);
  memory_mark(this->loadedPackages);
  memory_mark(this->package);
  memory_mark(this->classes);
  memory_mark(this->installed_public_classes);
  memory_mark(this->installed_classes);
  memory_mark(this->merged_public_classes);
  memory_mark(this->merged_public_routines);
  memory_mark(this->methods);
  memory_mark(this->active_class);
  memory_mark(this->initCode);
}

void RexxSource::liveGeneral(int reason)
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
    OrefSet(this, this->libraries, OREF_NULL);
    OrefSet(this, this->installed_classes, OREF_NULL);
    OrefSet(this, this->installed_public_classes, OREF_NULL);
    OrefSet(this, this->merged_public_classes, OREF_NULL);
    OrefSet(this, this->merged_public_routines, OREF_NULL);
    this->flags &= ~reclaim_possible;  /* can't recover source immediately  */
  }
#endif
  memory_mark_general(this->sourceArray);
  memory_mark_general(this->parentSource);
  memory_mark_general(this->programName);
  memory_mark_general(this->programDirectory);
  memory_mark_general(this->programExtension);
  memory_mark_general(this->programFile);
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
  memory_mark_general(this->libraries);
  memory_mark_general(this->loadedPackages);
  memory_mark_general(this->package);
  memory_mark_general(this->classes);
  memory_mark_general(this->installed_public_classes);
  memory_mark_general(this->installed_classes);
  memory_mark_general(this->merged_public_classes);
  memory_mark_general(this->merged_public_routines);
  memory_mark_general(this->methods);
  memory_mark_general(this->active_class);
  memory_mark_general(this->initCode);
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
    this->sourceArray = OREF_NULL;
    this->sourceBuffer = OREF_NULL;
    this->sourceIndices = OREF_NULL;
    this->securityManager = OREF_NULL;
    flatten_reference(newThis->sourceArray, envelope);
    flatten_reference(newThis->parentSource, envelope);
    flatten_reference(newThis->programName, envelope);
    flatten_reference(newThis->programDirectory, envelope);
    flatten_reference(newThis->programExtension, envelope);
    flatten_reference(newThis->programFile, envelope);
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
    flatten_reference(newThis->libraries, envelope);
    flatten_reference(newThis->loadedPackages, envelope);
    flatten_reference(newThis->package, envelope);
    flatten_reference(newThis->classes, envelope);
    flatten_reference(newThis->installed_public_classes, envelope);
    flatten_reference(newThis->installed_classes, envelope);
    flatten_reference(newThis->merged_public_classes, envelope);
    flatten_reference(newThis->merged_public_routines, envelope);
    flatten_reference(newThis->methods, envelope);
    flatten_reference(newThis->active_class, envelope);
    flatten_reference(newThis->initCode, envelope);

  cleanUpFlatten
}


size_t RexxSource::sourceSize()
/******************************************************************************/
/* Function:  Return count of lines in the source.  If the source is not      */
/*            available, return 0                                             */
/******************************************************************************/
{
    /* currently no source?              */
    if ((this->sourceArray == OREF_NULL && this->sourceBuffer == OREF_NULL))
    {
        if (!this->reconnect())            /* unable to recover the source?     */
        {
            return 0;                        /* we have no source lines           */
        }
    }
    return this->line_count;             /* return the line count             */
}


bool RexxSource::isTraceable()
/******************************************************************************/
/* Function:  Determine if a program is traceable (i.e., the program source   */
/*            is available)                                                   */
/******************************************************************************/
{
    /* currently no source?              */
    if ((this->sourceArray == OREF_NULL && this->sourceBuffer == OREF_NULL))
    {
        return this->reconnect();          /* unable to recover the source?     */
    }
    return true;                         /* return the line count             */
}

RexxString *RexxSource::get(
     size_t _position)                  /* requested source line             */
/******************************************************************************/
/* Function:  Extract a give source line from the source program              */
/******************************************************************************/
{
    LINE_DESCRIPTOR *descriptors;        /* line descriptors                  */
    const char *buffer_start;            /* start of source buffer            */

    if (_position > this->line_count)     /* beyond last line?                 */
    {
        return OREF_NULLSTRING;            /* just return a null string         */
    }
                                           /* working from an array?            */
    if (this->sourceArray != OREF_NULL)
    {
        /* return the array line             */
        return(RexxString *)(this->sourceArray->get(_position));
    }
    /* buffered version?                 */
    else if (this->sourceBuffer != OREF_NULL)
    {
        /* get the descriptors pointer       */
        descriptors = (LINE_DESCRIPTOR *)(this->sourceIndices->getData());
        /* source buffered in a string?      */
        if (isOfClass(String, this->sourceBuffer))
        {
            /* point to the data part            */
            buffer_start = ((RexxString *)(this->sourceBuffer))->getStringData();
        }
        else
        {
            /* point to the data part            */
            buffer_start = this->sourceBuffer->getData();
        }
        /* create a new string version       */
        return new_string(buffer_start + descriptors[_position].position, descriptors[_position].length);
    }
    else
    {
        return OREF_NULLSTRING;            /* we have no line                   */
    }
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
    SourceLocation location;             /* location of the clause            */
    SourceLocation token_location;       /* location of each token            */

    /* need to scan off a clause?        */
    if (!(this->flags&reclaimed))
    {
        this->clause->newClause();         /* reset the clause object           */
        /* loop until we get an non-null     */
        for (;;)
        {
            /* record the start position         */
            this->clause->setStart(this->line_number, this->line_offset);
            /* get the next source token         */
            /* (blanks are not significant here) */
            token = this->sourceNextToken(OREF_NULL);
            /* hit the end of the file?          */
            if (token == OREF_NULL)
            {
                this->flags |= no_clause;      /* flag this as a no clause          */
                return;                        /* we're finished                    */
            }
            /* is this the end of the clause?    */
            if (!token->isEndOfClause())
            {
                break;                         /* we've got what we need            */
            }
            this->clause->newClause();       /* reset the clause object           */
        }
        /* get the start position            */
        token_location = token->getLocation();
        location = token_location;         /* copy the location info            */
                                           /* record in clause for errors       */
        this->clause->setLocation(location);
        /* loop until physical end of clause */
        for (;;)
        {
            /* get the next token of real clause */
            /* (blanks can be significant)       */
            token = this->sourceNextToken(token);
            /* get this tokens location          */
            token_location = token->getLocation();
            if (token->isEndOfClause()) /* end of the clause now?            */
            {
                break;                         /* hit the physical end of clause    */
            }
        }
        location.setEnd(token_location);
        /* record the clause position        */
        this->clause->setLocation(location);
    }
    this->flags &= ~reclaimed;           /* no reclaimed clause               */
    // always set the error information
    clauseLocation = clause->getLocation();
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


/**
 * Create a stack frame for this parsing context.
 *
 * @return a stack frame instance for error reporting
 */
StackFrameClass *RexxSource::createStackFrame()
{
    return new StackFrameClass(FRAME_PARSE, programName, OREF_NULL, OREF_NULL, OREF_NULL, traceBack(clauseLocation, 0, true), clauseLocation.getLineNumber());
}


RexxString *RexxSource::traceBack(
     SourceLocation &location,         /* value to trace                    */
     size_t         indent,            /* blank indentation                 */
     bool           trace )            /* traced instruction (vs. error)    */
/******************************************************************************/
/* Function:  Format a source line for traceback or tracing                   */
/******************************************************************************/
{
    RexxString  *buffer;                 /* buffer for building result        */
    RexxString  *line;                   /* actual line data                  */
    size_t       outlength;              /* output length                     */
    char        *linepointer;            /* pointer to the line number        */
    char         linenumber[11];         /* formatted line number             */

                                           /* format the value                  */
    sprintf(linenumber,"%u", location.getLineNumber());

    line = this->extract(location);      /* extract the source string         */
                                         /* doesn't exist and this isn't a    */
                                         /* trace instruction format?         */
    if (line == OREF_NULLSTRING)
    {
        line = new_string(NO_SOURCE_MARKER);
    }

    if (indent < 0)                      /* possible negative indentation?    */
    {
        indent = 0;                        /* just reset it                     */
    }
                                           /* get an output string              */
    buffer = raw_string(line->getLength() + INSTRUCTION_OVERHEAD + indent * INDENT_SPACING);
    /* blank out the first part          */
    buffer->set(0, ' ', INSTRUCTION_OVERHEAD + indent * INDENT_SPACING);
    /* copy in the line                  */
    buffer->put(INSTRUCTION_OVERHEAD + indent * INDENT_SPACING, line->getStringData(), line->getLength());
    outlength = strlen(linenumber);      /* get the line number length        */
    linepointer = linenumber;            /* point to number start             */
    /* too long for defined field?       */
    if (outlength > LINENUMBER)
    {
        /* step over extra numbers           */
        linepointer += outlength - LINENUMBER;
        *linepointer = '?';                /* overlay a question mark           */
        outlength = LINENUMBER;            /* shorten the length                */
    }
    /* copy in the line number           */
    buffer->put(LINENUMBER - outlength, linepointer, outlength);
    buffer->put(PREFIX_OFFSET, "*-*", PREFIX_LENGTH);
    return buffer;                       /* return formatted buffer           */
}

RexxString *RexxSource::extract(
    SourceLocation &location )         /* target retrieval structure        */
/******************************************************************************/
/* Extrace a line from the source using the given location information        */
/******************************************************************************/
{
    RexxString *line;                    /* returned source line              */
    RexxString *source_line;             /* current extracting line           */
    size_t  counter;                     /* line counter                      */

                                         /* currently no source?              */
    if ((this->sourceArray == OREF_NULL && this->sourceBuffer == OREF_NULL))
    {
        if (!this->reconnect())            /* unable to recover the source?     */
            return OREF_NULLSTRING;          /* return a null array               */
    }
    /* is the location out of bounds?    */
    if (location.getLineNumber() == 0 || location.getLineNumber() > this->line_count)
        line = OREF_NULLSTRING;            /* just give back a null string      */
                                           /* all on one line?                  */
    else if (location.getLineNumber() >= location.getEndLine())
        /* just extract the string           */
        line = this->get(location.getLineNumber() - this->interpret_adjust)->extract(location.getOffset(),
                                                                                     location.getEndOffset() - location.getOffset());
    /* multiple line clause              */
    else
    {
        /* get the source line               */
        source_line = this->get(location.getLineNumber());
        /* extract the first part            */
        line = source_line->extract(location.getOffset(), source_line->getLength() - location.getOffset());
        /* loop down to end line             */
        for (counter = location.getLineNumber() + 1 - this->interpret_adjust; counter < location.getEndLine(); counter++)
        {
            /* concatenate the next line on      */
            line = line->concat(this->get(counter));
        }
        /* now add on the last part          */
        line = line->concat(this->get(counter)->extract(0, location.getEndOffset()));
    }
    return line;                         /* return the extracted line         */
}


/**
 * Extract all of the source from the package.
 *
 * @return An array of the source lines.
 */
RexxArray *RexxSource::extractSource()
{
    SourceLocation location;

    location.setLineNumber(1);
    location.setEndLine(0);
    location.setOffset(0);

    return extractSource(location);
}



RexxArray *RexxSource::extractSource(
    SourceLocation &location )         /* target retrieval structure        */
/******************************************************************************/
/* Function:  Extract a section of source from a method source object, using  */
/*            the created bounds for the method.                              */
/******************************************************************************/
{
                                         /* currently no source?              */
    if ((this->sourceArray == OREF_NULL && this->sourceBuffer == OREF_NULL))
    {
        if (!this->reconnect())            /* unable to recover the source?     */
        {
            /* return a null array               */
            return(RexxArray *)TheNullArray->copy();
        }
    }
    /* is the location out of bounds?    */
    if (location.getLineNumber() == 0 || location.getLineNumber() - this->interpret_adjust > this->line_count)
    {
        /* just give back a null array       */
        return (RexxArray *)TheNullArray->copy();
    }
    else
    {
        if (location.getEndLine() == 0)
        {  /* no ending line?                   */
           /* use the last line                 */
            location.setEnd(this->line_count, this->get(line_count)->getLength());
        }
        /* end at the line start?            */
        else if (location.getEndOffset() == 0)
        {
            // step back a line
            location.setEndLine(location.getEndLine() - 1); /* step back a line                  */
            /* end at the line end               */
            location.setEndOffset(this->get(location.getEndLine())->getLength());
        }
        /* get the result array              */
        RexxArray *source = new_array(location.getEndLine() - location.getLineNumber() + 1);
        /* all on one line?                  */
        if (location.getLineNumber() == location.getEndLine())
        {
            /* get the line                      */
            RexxString *source_line = this->get(location.getLineNumber());
            /* extract the line segment          */
            source_line = source_line->extract(location.getOffset(), location.getEndOffset() - location.getOffset());
            source->put(source_line, 1);     /* insert the trailing piece         */
            return source;                   /* all done                          */
        }
        if (location.getOffset() == 0)     /* start on the first location?      */
        {
            /* copy over the entire line         */
            source->put(this->get(location.getLineNumber()), 1);
        }
        else
        {
            /* get the line                      */
            RexxString *source_line = this->get(location.getLineNumber());
            /* extract the end portion           */
            source_line = source_line->extract(location.getOffset(), source_line->getLength() - location.getOffset());
            source->put(source_line, 1);     /* insert the trailing piece         */
        }

        size_t i = 2;
        /* loop until the last line          */
        for (size_t counter = location.getLineNumber() + 1; counter < location.getEndLine(); counter++, i++)
        {
            /* copy over the entire line         */
            source->put(this->get(counter), i);
        }
        /* get the last line                 */
        RexxString *source_line = this->get(location.getEndLine());
        /* more than one line?               */
        if (location.getEndLine() > location.getLineNumber())
        {
            /* need the entire line?             */
            if (location.getEndOffset() >= source_line->getLength())
            {
                source->put(source_line, i);   /* just use it                       */
            }
            else
            {
                /* extract the tail part             */
                source->put(source_line->extract(0, location.getEndOffset() - 1), i);
            }
        }
        return source;
    }
}

void RexxSource::globalSetup()
/******************************************************************************/
/* Function:  Perform global parsing initialization                           */
/******************************************************************************/
{
                                       /* holding pen for temporaries       */
  OrefSet(this, this->holdstack, new (HOLDSIZE, false) RexxStack(HOLDSIZE));
                                       /* create a save table               */
  OrefSet(this, this->savelist, new_identity_table());
                                       /* allocate global control tables    */
  OrefSet(this, this->control, new_queue());
  OrefSet(this, this->terms, new_queue());
  OrefSet(this, this->subTerms, new_queue());
  OrefSet(this, this->operators, new_queue());
  OrefSet(this, this->literals, new_directory());
  // during an image build, we have a global string table.  If this is
  // available now, use it.
  OrefSet(this, this->strings, memoryObject.getGlobalStrings());
  if (this->strings == OREF_NULL)
  {
      // no global string table, use a local copy
      OrefSet(this, this->strings, new_directory());
  }
                                       /* get the clause object             */
  OrefSet(this, this->clause, new RexxClause());
}


RexxCode *RexxSource::generateCode(bool isMethod)
/******************************************************************************/
/* Function:  Convert a source object into an executable method               */
/******************************************************************************/
{
  this->globalSetup();                 /* do the global setup part          */
                                       /* translate the source program      */
  RexxCode *newCode = this->translate(OREF_NULL);
  ProtectedObject p(newCode);
  this->cleanup();                     /* release temporary tables          */
  // if generating a method object, then process the directive installation now
  if (isMethod)
  {
      // force this to install now
      install();
  }
  return newCode;                      /* return the method                 */
}

RexxCode *RexxSource::interpretMethod(
    RexxDirectory *_labels )            /* parent label set                  */
/******************************************************************************/
/* Function:  Convert a source object into an executable interpret method     */
/******************************************************************************/
{
  this->globalSetup();                 /* do the global setup part          */
  this->flags |= _interpret;           /* this is an interpret              */
  RexxCode *newCode = this->translate(_labels); /* translate the source program      */
  ProtectedObject p(newCode);
  this->cleanup();                     /* release temporary tables          */
  return newCode;                      /* return the method                 */
}

RexxCode *RexxSource::interpret(
    RexxString    *string,             /* interpret string value            */
    RexxDirectory *_labels,             /* parent labels                     */
    size_t         _line_number )       /* line number of interpret          */
/******************************************************************************/
/* Function:   Interpret a string in the current activation context           */
/******************************************************************************/
{
                                       /* create a source object            */
  RexxSource *source = new RexxSource (this->programName, new_array(string));
  ProtectedObject p(source);
  source->interpretLine(_line_number);  /* fudge the line numbering          */
                                       /* convert to executable form        */
  return source->interpretMethod(_labels);
}

void RexxSource::checkDirective()
/******************************************************************************/
/* Function:  Verify that no code follows a directive except for more         */
/*            directive instructions.                                         */
/******************************************************************************/
{
    // save the clause location so we can reset for errors
    SourceLocation location = clauseLocation;

    this->nextClause();                  /* get the next clause               */
    /* have a next clause?               */
    if (!(this->flags&no_clause))
    {
        RexxToken *token = nextReal();     /* get the first token               */
                                           /* not a directive start?            */
        if (token->classId != TOKEN_DCOLON)
        {
            /* this is an error                  */
            syntaxError(Error_Translation_bad_directive);
        }
        firstToken();                      /* reset to the first token          */
        this->reclaimClause();             /* give back to the source object    */
    }
    // this resets the current clause location so that any errors on the current
    // clause detected after the clause check reports this on the correct line
    // number
    clauseLocation = location;
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
    /* have a real object                */
    if (object != OREF_NULL)
    {
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


/**
 * Merge a parent source context into our context so all of the
 * bits that are visible in the parent are also resolvable in our
 * context.  This is mostly used for dynamically created methods.
 *
 * @param parent The parent source context.
 */
void RexxSource::inheritSourceContext(RexxSource *source)
{
    // set this as a parent
    OrefSet(this, this->parentSource, source);
}


void RexxSource::mergeRequired(RexxSource *source)
/******************************************************************************/
/* Function:  Merge all public class and routine information from a called    */
/*            program into the full public information of this program.       */
/******************************************************************************/
{
    // has the source already merged in some public routines?  pull those in first,
    // so that the direct set will override
    if (source->merged_public_routines != OREF_NULL)
    {
        /* first merged attempt?             */
        if (this->merged_public_routines == OREF_NULL)
        {
            /* get the directory                 */
            OrefSet(this, this->merged_public_routines, new_directory());
        }
        /* loop through the list of routines */
        for (HashLink i = source->merged_public_routines->first(); source->merged_public_routines->available(i); i = source->merged_public_routines->next(i))
        {
            /* copy the routine over             */
            this->merged_public_routines->setEntry((RexxString *)source->merged_public_routines->index(i), source->merged_public_routines->value(i));
        }

    }

    // now process the direct set
    if (source->public_routines != OREF_NULL)
    {
        /* first merged attempt?             */
        if (this->merged_public_routines == OREF_NULL)
        {
            /* get the directory                 */
            OrefSet(this, this->merged_public_routines, new_directory());
        }
        /* loop through the list of routines */
        for (HashLink i = source->public_routines->first(); source->public_routines->available(i); i = source->public_routines->next(i))
        {
            /* copy the routine over             */
            this->merged_public_routines->setEntry((RexxString *)source->public_routines->index(i), source->public_routines->value(i));
        }
    }


    // now do the same process for any of the class contexts
    if (source->merged_public_classes != OREF_NULL)
    {
        if (this->merged_public_classes == OREF_NULL)
        {
            /* get the directory                 */
            OrefSet(this, this->merged_public_classes, new_directory());
        }
        /* loop through the list of classes, */
        for (HashLink i = source->merged_public_classes->first(); source->merged_public_classes->available(i); i = source->merged_public_classes->next(i))
        {
            /* copy the routine over             */
            this->merged_public_classes->setEntry((RexxString *)source->merged_public_classes->index(i), source->merged_public_classes->value(i));
        }
    }

    // the installed ones are processed second as they will overwrite the imported one, which
    // is the behaviour we want
    if (source->installed_public_classes != OREF_NULL)
    {
        if (this->merged_public_classes == OREF_NULL)
        {
            /* get the directory                 */
            OrefSet(this, this->merged_public_classes, new_directory());
        }
        /* loop through the list of classes, */
        for (HashLink i = source->installed_public_classes->first(); source->installed_public_classes->available(i); i = source->installed_public_classes->next(i))
        {
            /* copy the routine over             */
            this->merged_public_classes->setEntry((RexxString *)source->installed_public_classes->index(i), source->installed_public_classes->value(i));
        }
    }
}


/**
 * Resolve a directly defined class object in this or a parent
 * context.
 *
 * @param name   The name we're searching for (all uppercase).
 *
 * @return A resolved class object, if found.
 */
RoutineClass *RexxSource::findLocalRoutine(RexxString *name)
{
    // if we have one locally, then return it.
    if (this->routines != OREF_NULL)
    {
        /* try for a local one first         */
        RoutineClass *result = (RoutineClass *)(this->routines->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context, so check it also
    if (parentSource != OREF_NULL)
    {
        return parentSource->findLocalRoutine(name);
    }
    // nope, no got one
    return OREF_NULL;
}


/**
 * Resolve a public routine in this source context
 *
 * @param name   The target name.
 *
 * @return A resolved Routine object, if found.
 */
RoutineClass *RexxSource::findPublicRoutine(RexxString *name)
{
    // if we have one locally, then return it.
    if (this->merged_public_routines != OREF_NULL)
    {
        /* try for a local one first         */
        RoutineClass *result = (RoutineClass *)(this->merged_public_routines->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context, so check it also
    if (parentSource != OREF_NULL)
    {
        return parentSource->findPublicRoutine(name);
    }
    // nope, no got one
    return OREF_NULL;
}


/**
 * Resolve a routine from this source files base context.
 *
 * @param routineName
 *               The routine name of interest.
 *
 * @return A RoutineClass instance if located.  Returns OREF_NULL if this
 *         is not known at this level.
 */
RoutineClass *RexxSource::findRoutine(RexxString *routineName)
{
    // These lookups are case insensive, so the table are all created using the opper
    // case names.  Use it once and reuse it.
    RexxString *upperName = routineName->upper();
    ProtectedObject p1(upperName);
    RoutineClass *routineObject = findLocalRoutine(upperName);
    if (routineObject != OREF_NULL)
    {
        return routineObject;
    }

    // now try for one pulled in from ::REQUIRES objects
    return findPublicRoutine(upperName);
}


/**
 * Resolve an external call in the context of the program making the
 * call.  This will use the directory and extension of the context
 * program to modify the search order.
 *
 * @param activity The current activity
 * @param name     The target name
 *
 * @return The fully resolved string name of the target program, if one is
 *         located.
 */
RexxString *RexxSource::resolveProgramName(RexxActivity *activity, RexxString *name)
{
    return activity->getInstance()->resolveProgramName(name, programDirectory, programExtension);
}


/**
 * Resolve a directly defined class object in this or a parent
 * context.
 *
 * @param name   The name we're searching for (all uppercase).
 *
 * @return A resolved class object, if found.
 */
RexxClass *RexxSource::findInstalledClass(RexxString *name)
{
    // if we have one locally, then return it.
    if (this->installed_classes != OREF_NULL)
    {
        /* try for a local one first         */
        RexxClass *result = (RexxClass *)(this->installed_classes->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context, so check it also
    if (parentSource != OREF_NULL)
    {
        return parentSource->findInstalledClass(name);
    }
    // nope, no got one
    return OREF_NULL;
}


RexxClass *RexxSource::findPublicClass(RexxString *name)
{
    // if we have one locally, then return it.
    if (this->merged_public_classes != OREF_NULL)
    {
        /* try for a local one first         */
        RexxClass *result = (RexxClass *)(this->merged_public_classes->fastAt(name));
        if (result != OREF_NULL)
        {
            return result;
        }
    }

    // we might have a chained context, so check it also
    if (parentSource != OREF_NULL)
    {
        return parentSource->findPublicClass(name);
    }
    // nope, no got one
    return OREF_NULL;
}


/**
 * Resolve a class from this source file context (including any
 * chained parent contexts).
 *
 * @param className The target name of the class.
 *
 * @return The resolved class object, if any.
 */
RexxClass *RexxSource::findClass(RexxString *className)
{
    RexxString *internalName = className->upper();   /* upper case it                     */
    // check for a directly defined one in the source context chain
    RexxClass *classObject = findInstalledClass(internalName);
    // return if we got one
    if (classObject != OREF_NULL)
    {
        return classObject;
    }
    // now try for public classes we pulled in from other contexts
    classObject = findPublicClass(internalName);
    // return if we got one
    if (classObject != OREF_NULL)
    {
        return classObject;
    }

    // give the security manager a go
    if (this->securityManager != OREF_NULL)
    {
        classObject = (RexxClass *)securityManager->checkLocalAccess(internalName);
        if (classObject != OREF_NULL)
        {
            return classObject;
        }
    }

    /* send message to .local            */
    classObject = (RexxClass *)(ActivityManager::getLocalEnvironment(internalName));
    if (classObject != OREF_NULL)
    {
        return classObject;
    }

    /* normal execution?                 */
    if (this->securityManager != OREF_NULL)
    {
        classObject = (RexxClass *)securityManager->checkEnvironmentAccess(internalName);
        if (classObject != OREF_NULL)
        {
            return classObject;
        }
    }

    /* last chance, try the environment  */
    return(RexxClass *)(TheEnvironment->at(internalName));
}


/**
 * Perform a non-contextual install of a package.
 */
void RexxSource::install()
{
    if (needsInstallation())
    {
        // In order to install, we need to call something.  We manage this by
        // creating a dummy stub routine that we can call to force things to install
        RexxCode *stub = new RexxCode(this, OREF_NULL, OREF_NULL, 10, FIRST_VARIABLE_INDEX);
        ProtectedObject p2(stub);
        RoutineClass *code = new RoutineClass(programName, stub);
        p2 = code;
        ProtectedObject dummy;
        code->call(ActivityManager::currentActivity, programName, NULL, 0, dummy);
    }
}


void RexxSource::processInstall(
    RexxActivation *activation)        /* invoking activation               */
/******************************************************************************/
/* Function:  Process directive information contained within a method, calling*/
/*            all ::requires routines, creating all ::class methods, and      */
/*            processing all ::routines.                                      */
/******************************************************************************/
{
    /* turn the install flag off         */
    /* immediately, otherwise we may     */
    /* run into a recursion problem      */
    /* when class init methods are       */
    /* processed                         */
    this->flags &= ~_install;            /* we are now installed              */

    // native packages are processed first.  The requires might actually need
    // functons loaded by the packages
    if (this->libraries != OREF_NULL)
    {
        /* classes and routines              */
        // now loop through the requires items
        for (size_t i = libraries->firstIndex(); i != LIST_END; i = libraries->nextIndex(i))
        {
            // and have it do the installs processing
            LibraryDirective *library = (LibraryDirective *)this->libraries->getValue(i);
            library->install(activation);
        }
    }

    // native methods and routines are lazy resolved on first use, so we don't
    // need to process them here.

    if (this->requires != OREF_NULL)     /* need to process ::requires?       */
    {
        /* classes and routines              */
        // now loop through the requires items
        for (size_t i = requires->firstIndex(); i != LIST_END; i = requires->nextIndex(i))
        {
            // and have it do the installs processing.  This is a little roundabout, but
            // we end up back in our own context while processing this, and the merge
            // of the information happens then.
            RequiresDirective *_requires = (RequiresDirective *)this->requires->getValue(i);
            _requires->install(activation);
        }
    }

    // and finally process classes
    if (this->classes != OREF_NULL)
    {
        /* get an installed classes directory*/
        OrefSet(this, this->installed_classes, new_directory());
        /* and the public classes            */
        OrefSet(this, this->installed_public_classes, new_directory());
        for (size_t i = classes->firstIndex(); i != LIST_END; i = classes->nextIndex(i))
        {
            /* get the class info                */
            ClassDirective *current_class = (ClassDirective *)this->classes->getValue(i);
            current_class->install(this, activation);
        }
    }
}

RexxCode *RexxSource::translate(
    RexxDirectory *_labels)             /* interpret labels                  */
/******************************************************************************/
/* Function:  Translate a source object into a method object                  */
/******************************************************************************/
{
    ParseActivationFrame frame(ActivityManager::currentActivity, this);

    // set up the package global defaults
    digits = Numerics::DEFAULT_DIGITS;
    form = Numerics::DEFAULT_FORM;
    fuzz = Numerics::DEFAULT_FUZZ;
    traceSetting = DEFAULT_TRACE_SETTING;
    traceFlags = RexxActivation::default_trace_flags;

    /* go translate the lead block       */
    RexxCode *newMethod = this->translateBlock(_labels);
    // we save this in case we need to explicitly run this at install time
    OrefSet(this, this->initCode, newMethod);
    if (!this->atEnd())                  /* have directives to process?       */
    {
        /* create the routines directory     */
        OrefSet(this, this->routines, new_directory());
        /* create the routines directory     */
        OrefSet(this, this->public_routines, new_directory());
        /* and a directory of dependencies   */
        OrefSet(this, this->class_dependencies, new_directory());
        /* create the requires directory     */
        OrefSet(this, this->requires, new_list());
        // and a list of load libraries requiring loading.
        OrefSet(this, this->libraries, new_list());
        /* create the classes list           */
        OrefSet(this, this->classes, new_list());
        /* no active class definition        */
        OrefSet(this, this->active_class, OREF_NULL);
                                           /* translation stopped by a directive*/
        if (this->flags&_interpret)        /* is this an interpret?             */
        {
            this->nextClause();              /* get the directive clause          */
                                             /* raise an error                    */
            syntaxError(Error_Translation_directive_interpret);
        }
        /* create a directory for ..methods  */
        OrefSet(this, this->methods, new_directory());

        while (!this->atEnd())             /* loop until end of source          */
        {
            this->directive();               /* process the directive             */
        }
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
    // get our class list
    if (classes->items() == 0)           /* nothing to process?               */
    {
        /* clear out the classes list        */
        OrefSet(this, this->classes, OREF_NULL);
    }
    else                                 /* have classes to process           */
    {
        size_t i;
        // run through the class list having each directive set up its
        // dependencies
        for (i = classes->firstIndex(); i != LIST_END; i = classes->nextIndex(i))
        {
            /* get the next class                */
            ClassDirective *current_class = (ClassDirective *)(classes->getValue(i));
            // have the class figure out it's in-package dependencies
            current_class->addDependencies(class_dependencies);
        }

        RexxList *class_order = new_list();  // get a list for doing the order
        ProtectedObject p(class_order);

/* now we repeatedly scan the pending directory looking for a class         */
/* with no in-program dependencies - it's an error if there isn't one       */
/* as we build the classes we have to remove them (their names) from        */
/* pending list and from the remaining dependencies                         */
        while (classes->items() > 0)
        {
            // this is the next one we process
            ClassDirective *next_install = OREF_NULL;
            for (i = classes->firstIndex(); i != LIST_END; i = classes->nextIndex(i))
            {
                 /* get the next class                */
                ClassDirective *current_class = (ClassDirective *)(classes->getValue(i));
                // if this class doesn't have any additional dependencies, pick it next.
                if (current_class->dependenciesResolved())
                {
                    next_install = current_class;
                    // add this to the class ordering
                    class_order->append((RexxObject *)next_install);
                    // remove this from the processing list
                    classes->removeIndex(i);
                }
            }
            if (next_install == OREF_NULL)   /* nothing located?                  */
            {
                /* raise an error                    */
                syntaxError(Error_Execution_cyclic, this->programName);
            }
            RexxString *class_name = next_install->getName();

            // now go through the pending list telling each of the remaining classes that
            // they can remove this dependency from their list
            for (i = classes->firstIndex(); i != LIST_END; i = classes->nextIndex(i))
            {    /* go remove the dependencies        */
                 /* get a class                       */
                ClassDirective *current_class = (ClassDirective *)classes->getValue(i);
                current_class->removeDependency(class_name);
            }
        }

        /* replace the original class list   */
        OrefSet(this, this->classes, class_order);
        /* don't need the dependencies now   */
        OrefSet(this, this->class_dependencies, OREF_NULL);
    }

    if (this->requires->items() == 0)     /* nothing there?                    */
    {
        /* just clear it out                 */
        OrefSet(this, this->requires, OREF_NULL);
    }
    if (this->libraries->items() == 0)     /* nothing there?                    */
    {
        /* just clear it out                 */
        OrefSet(this, this->libraries, OREF_NULL);
    }
    if (this->routines->items() == 0)    /* no routines to process?           */
    {
        /* just clear it out also            */
        OrefSet(this, this->routines, OREF_NULL);
    }
    /* now finally the public routines   */
    if (this->public_routines->items() == 0)
    {
        /* just clear it out also            */
        OrefSet(this, this->public_routines, OREF_NULL);
    }
    if (this->methods->items() == 0)     /* and also the methods directory    */
    {
        /* just clear it out also            */
        OrefSet(this, this->methods, OREF_NULL);
    }
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
    RexxToken *token = nextReal();       /* get the next token                */
    /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_class);
    }
    RexxString *name = token->value;             /* get the routine name              */
                                     /* get the exposed name version      */
    RexxString *public_name = this->commonString(name->upper());
    /* does this already exist?          */
    if (this->class_dependencies->entry(public_name) != OREF_NULL)
    {
        /* have an error here                */
        syntaxError(Error_Translation_duplicate_class);
    }
    /* create a dependencies list        */
    this->flags |= _install;         /* have information to install       */

    // create a class directive and add this to the dependency list
    OrefSet(this, this->active_class, new ClassDirective(name, public_name, this->clause));
    this->class_dependencies->put((RexxObject *)active_class, public_name);
    // and also add to the classes list
    this->classes->append((RexxObject *)this->active_class);

    int  Public = DEFAULT_ACCESS_SCOPE;   /* haven't seen the keyword yet      */
    for (;;)
    {                       /* now loop on the option keywords   */
        token = nextReal();            /* get the next token                */
                                       /* reached the end?                  */
        if (token->isEndOfClause())
        {
            break;                       /* get out of here                   */
        }
                                         /* not a symbol token?               */
        else if (!token->isSymbol())
        {
            /* report an error                   */
            syntaxError(Error_Invalid_subkeyword_class, token);
        }
        else
        {                         /* have some sort of option keyword  */
                                  /* get the keyword type              */
            int type = this->subDirective(token);
            switch (type)
            {              /* process each sub keyword          */
                    /* ::CLASS name METACLASS metaclass  */
                case SUBDIRECTIVE_METACLASS:
                    /* already had a METACLASS?          */
                    if (active_class->getMetaClass() != OREF_NULL)
                    {
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    token = nextReal();      /* get the next token                */
                                             /* not a symbol or a string          */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_metaclass, token);
                    }
                                             /* tag the active class              */
                    this->active_class->setMetaClass(token->value);
                    break;


                case SUBDIRECTIVE_PUBLIC:  /* ::CLASS name PUBLIC               */
                    if (Public != DEFAULT_ACCESS_SCOPE)  /* already had one of these?         */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    Public = PUBLIC_SCOPE;   /* turn on the seen flag             */
                                             /* just set this as a public object  */
                    this->active_class->setPublic();
                    break;

                case SUBDIRECTIVE_PRIVATE: /* ::CLASS name PUBLIC               */
                    if (Public != DEFAULT_ACCESS_SCOPE)  /* already had one of these?         */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    Public = PRIVATE_SCOPE;  /* turn on the seen flag             */
                    break;
                    /* ::CLASS name SUBCLASS sclass      */
                case SUBDIRECTIVE_SUBCLASS:
                    // If we have a subclass set already, this is an error
                    if (active_class->getSubClass() != OREF_NULL)
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    token = nextReal();      /* get the next token                */
                                             /* not a symbol or a string          */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_subclass);
                    }
                    /* set the subclass information      */
                    this->active_class->setSubClass(token->value);
                    break;
                    /* ::CLASS name MIXINCLASS mclass    */
                case SUBDIRECTIVE_MIXINCLASS:
                    // If we have a subclass set already, this is an error
                    if (active_class->getSubClass() != OREF_NULL)
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_class, token);
                    }
                    token = nextReal();      /* get the next token                */
                                             /* not a symbol or a string          */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_mixinclass);
                    }
                    /* set the subclass information      */
                    this->active_class->setMixinClass(token->value);
                    break;
                    /* ::CLASS name INHERIT iclasses     */
                case SUBDIRECTIVE_INHERIT:
                    token = nextReal();      /* get the next token                */
                                             /* nothing after the keyword?        */
                    if (token->isEndOfClause())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_inherit, token);
                    }
                    while (!token->isEndOfClause())
                    {
                        /* not a symbol or a string          */
                        if (!token->isSymbolOrLiteral())
                        {
                            /* report an error                   */
                            syntaxError(Error_Symbol_or_string_inherit, token);
                        }
                        /* add to the inherit list           */
                        this->active_class->addInherits(token->value);
                        token = nextReal();    /* step to the next token            */
                    }
                    previousToken();         /* step back a token                 */
                    break;

                default:                   /* invalid keyword                   */
                    /* this is an error                  */
                    syntaxError(Error_Invalid_subkeyword_class, token);
                    break;
            }
        }
    }
}


/**
 * check for a duplicate method.
 *
 * @param name   The name to check.
 * @param classMethod
 *               Indicates whether this is a check for a CLASS or INSTANCE method.
 * @param errorMsg
 *               The error code to use if there is a duplicate.
 */
void RexxSource::checkDuplicateMethod(RexxString *name, bool classMethod, int errorMsg)
{
    /* no previous ::CLASS directive?    */
    if (this->active_class == OREF_NULL)
    {
        if (classMethod)             /* supposed to be a class method?    */
        {
                                     /* this is an error                  */
            syntaxError(Error_Translation_missing_class);
        }
        /* duplicate method name?            */
        if (this->methods->entry(name) != OREF_NULL)
        {
            /* this is an error                  */
            syntaxError(errorMsg);
        }
    }
    else
    {                                /* add the method to the active class*/
        if (active_class->checkDuplicateMethod(name, classMethod))
        {
            /* this is an error                  */
            syntaxError(errorMsg);
        }
    }
}


/**
 * Add a new method to this compilation.
 *
 * @param name   The directory name of the method.
 * @param method The method object.
 * @param classMethod
 *               The class/instance method indicator.
 */
void RexxSource::addMethod(RexxString *name, RexxMethod *method, bool classMethod)
{
    if (this->active_class == OREF_NULL)
    {
        this->methods->setEntry(name, method);
    }
    else
    {
        active_class->addMethod(name, method, classMethod);
    }
}



/**
 * Process a ::METHOD directive in a source file.
 */
void RexxSource::methodDirective()
{
    int  Private = DEFAULT_ACCESS_SCOPE;    /* this is a public method           */
    int  Protected = DEFAULT_PROTECTION;  /* and is not protected yet          */
    int guard = DEFAULT_GUARD;       /* default is guarding               */
    bool Class = false;              /* default is an instance method     */
    bool Attribute = false;          /* init Attribute flag               */
    bool abstractMethod = false;     // this is an abstract method
    RexxToken *token = nextReal();   /* get the next token                */
    RexxString *externalname = OREF_NULL;       /* not an external method yet        */

                                     /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_method, token);
    }
    RexxString *name = token->value; /* get the string name               */
                                     /* and the name form also            */
    RexxString *internalname = this->commonString(name->upper());
    for (;;)
    {                       /* now loop on the option keywords   */
        token = nextReal();            /* get the next token                */
                                       /* reached the end?                  */
        if (token->isEndOfClause())
        {
            break;                       /* get out of here                   */
        }
                                         /* not a symbol token?               */
        else if (!token->isSymbol())
        {
            /* report an error                   */
            syntaxError(Error_Invalid_subkeyword_method, token);
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
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Class = true;            /* flag this for later processing    */
                    break;
                    /* ::METHOD name EXTERNAL extname    */
                case SUBDIRECTIVE_EXTERNAL:
                    /* already had an external?          */
                    if (externalname != OREF_NULL || abstractMethod)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_external, token);
                    }
                    externalname = token->value;
                    break;
                    /* ::METHOD name PRIVATE             */
                case SUBDIRECTIVE_PRIVATE:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Private = PRIVATE_SCOPE;           /* flag for later processing         */
                    break;
                    /* ::METHOD name PUBLIC             */
                case SUBDIRECTIVE_PUBLIC:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Private = PUBLIC_SCOPE;        /* flag for later processing         */
                    break;
                    /* ::METHOD name PROTECTED           */
                case SUBDIRECTIVE_PROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Protected = PROTECTED_METHOD;        /* flag for later processing         */
                    break;
                    /* ::METHOD name UNPROTECTED           */
                case SUBDIRECTIVE_UNPROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Protected = UNPROTECTED_METHOD;      /* flag for later processing         */
                    break;
                    /* ::METHOD name UNGUARDED           */
                case SUBDIRECTIVE_UNGUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    guard = UNGUARDED_METHOD;/* flag for later processing         */
                    break;
                    /* ::METHOD name GUARDED             */
                case SUBDIRECTIVE_GUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    guard = GUARDED_METHOD;  /* flag for later processing         */
                    break;
                    /* ::METHOD name ATTRIBUTE           */
                case SUBDIRECTIVE_ATTRIBUTE:

                    if (Attribute)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    // cannot have an abstract attribute
                    if (abstractMethod)
                    {
                        /* EXTERNAL and ATTRIBUTE are        */
                        /* mutually exclusive                */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Attribute = true;        /* flag for later processing         */
                    break;

                                           /* ::METHOD name ABSTRACT            */
                case SUBDIRECTIVE_ABSTRACT:

                    if (abstractMethod || externalname != OREF_NULL)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    // not compatible with ATTRIBUTE or EXTERNAL
                    if (externalname != OREF_NULL || Attribute)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    abstractMethod = true;   /* flag for later processing         */
                    break;


                default:                   /* invalid keyword                   */
                    /* this is an error                  */
                    syntaxError(Error_Invalid_subkeyword_method, token);
                    break;
            }
        }
    }

    // go check for a duplicate and validate the use of the CLASS modifier
    checkDuplicateMethod(internalname, Class, Error_Translation_duplicate_method);


    RexxMethod *_method = OREF_NULL;
    // is this an attribute method?
    if (Attribute)
    {
        // now get this as the setter method.
        RexxString *setterName = commonString(internalname->concatWithCstring("="));
        // need to check for duplicates on that too
        checkDuplicateMethod(setterName, Class, Error_Translation_duplicate_method);

                                       /* Go check the next clause to make  */
        this->checkDirective();        /* sure that no code follows         */
        // this might be externally defined setters and getters.
        if (externalname != OREF_NULL)
        {
            RexxString *library = OREF_NULL;
            RexxString *procedure = OREF_NULL;
            decodeExternalMethod(internalname, externalname, library, procedure);
            // now create both getter and setting methods from the information.
            _method = createNativeMethod(internalname, library, procedure->concatToCstring("GET"));
            _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
            // add to the compilation
            addMethod(internalname, _method, Class);

            _method = createNativeMethod(setterName, library, procedure->concatToCstring("SET"));
            _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
            // add to the compilation
            addMethod(setterName, _method, Class);
        }
        else
        {
            // now get a variable retriever to get the property
            RexxVariableBase *retriever = this->getRetriever(name);

            // create the method pair and quit.
            createAttributeGetterMethod(internalname, retriever, Class, Private == PRIVATE_SCOPE,
                Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
            createAttributeSetterMethod(setterName, retriever, Class, Private == PRIVATE_SCOPE,
                Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
        }
        return;
    }
    // abstract method?
    else if (abstractMethod)
    {
                                       /* Go check the next clause to make  */
        this->checkDirective();        /* sure that no code follows         */
        // this uses a special code block
        BaseCode *code = new AbstractCode();
        _method = new RexxMethod(name, code);
    }
    /* not an external method?           */
    else if (externalname == OREF_NULL)
    {
        // NOTE:  It is necessary to translate the block and protect the code
        // before allocating the RexxMethod object.  The new operator allocates the
        // the object first, then evaluates the constructor arguments after the allocation.
        // Since the translateBlock() call will allocate a lot of new objects before returning,
        // there's a high probability that the method object can get garbage collected before
        // there is any opportunity to protect the object.
        RexxCode *code = this->translateBlock(OREF_NULL);
        this->saveObject((RexxObject *)code);

        /* go do the next block of code      */
        _method = new RexxMethod(name, code);
    }
    else
    {
        RexxString *library = OREF_NULL;
        RexxString *procedure = OREF_NULL;
        decodeExternalMethod(internalname, externalname, library, procedure);

        /* go check the next clause to make  */
        this->checkDirective();
        // and make this into a method object.
        _method = createNativeMethod(name, library, procedure);
    }
    _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
    // add to the compilation
    addMethod(internalname, _method, Class);
}



/**
 * Process a ::OPTIONS directive in a source file.
 */
void RexxSource::optionsDirective()
{
    // all options are of a keyword/value pattern
    for (;;)
    {
        RexxToken *token = nextReal(); /* get the next token                */
                                       /* reached the end?                  */
        if (token->isEndOfClause())
        {
            break;                       /* get out of here                   */
        }
                                         /* not a symbol token?               */
        else if (!token->isSymbol())
        {
            /* report an error                   */
            syntaxError(Error_Invalid_subkeyword_options, token);
        }
        else
        {                         /* have some sort of option keyword  */
                                  /* process each sub keyword          */
            switch (this->subDirective(token))
            {
                // ::OPTIONS DIGITS nnnn
                case SUBDIRECTIVE_DIGITS:
                {
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_digits_value, token);
                    }
                    RexxString *value = token->value;          /* get the string value              */

                    if (!value->requestUnsignedNumber(digits, number_digits()) || digits < 1)
                    {
                        /* report an exception               */
                        syntaxError(Error_Invalid_whole_number_digits, value);
                    }
                    /* problem with the fuzz setting?    */
                    if (digits <= fuzz)
                    {
                        /* this is an error                  */
                        reportException(Error_Expression_result_digits, digits, fuzz);
                    }
                    break;
                }
                // ::OPTIONS FORM ENGINEERING/SCIENTIFIC
                case SUBDIRECTIVE_FORM:
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isSymbol())
                    {
                        /* report an error                   */
                        syntaxError(Error_Invalid_subkeyword_form, token);
                    }
                    /* resolve the subkeyword            */
                    /* and process                       */
                    switch (this->subKeyword(token))
                    {

                        case SUBKEY_SCIENTIFIC:        /* NUMERIC FORM SCIENTIFIC           */
                            form = Numerics::FORM_SCIENTIFIC;
                            break;

                        case SUBKEY_ENGINEERING:     /* NUMERIC FORM ENGINEERING          */
                            form = Numerics::FORM_ENGINEERING;
                            break;

                        default:                     /* invalid subkeyword                */
                            /* raise an error                    */
                            syntaxError(Error_Invalid_subkeyword_form, token);
                            break;

                    }
                    break;
                // ::OPTIONS FUZZ nnnn
                case SUBDIRECTIVE_FUZZ:
                {
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_fuzz_value, token);
                    }
                    RexxString *value = token->value;          /* get the string value              */

                    if (!value->requestUnsignedNumber(fuzz, number_digits()))
                    {
                        /* report an exception               */
                        syntaxError(Error_Invalid_whole_number_fuzz, value);
                    }
                    /* problem with the digits setting?  */
                    if (fuzz >= digits)
                    {
                        /* and issue the error               */
                        reportException(Error_Expression_result_digits, digits, fuzz);
                    }
                    break;
                }
                // ::OPTIONS TRACE setting
                case SUBDIRECTIVE_TRACE:
                {
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_trace_value, token);
                    }
                    RexxString *value = token->value;          /* get the string value              */
                    char badOption = 0;
                                                 /* process the setting               */
                    if (!parseTraceSetting(value, traceSetting, traceFlags, badOption))
                    {
                        syntaxError(Error_Invalid_trace_trace, new_string(&badOption, 1));
                    }
                    break;
                }

                default:                   /* invalid keyword                   */
                    /* this is an error                  */
                    syntaxError(Error_Invalid_subkeyword_options, token);
                    break;
            }
        }
    }
}

/**
 * Create a native method from a specification.
 *
 * @param name      The method name.
 * @param library   The library containing the method.
 * @param procedure The name of the procedure within the package.
 *
 * @return A method object representing this method.
 */
RexxMethod *RexxSource::createNativeMethod(RexxString *name, RexxString *library, RexxString *procedure)
{
                                 /* create a new native method        */
    RexxNativeCode *nmethod = PackageManager::resolveMethod(library, procedure);
    // raise an exception if this entry point is not found.
    if (nmethod == OREF_NULL)
    {
         syntaxError(Error_External_name_not_found_method, procedure);
    }
    // this might return a different object if this has been used already
    nmethod = (RexxNativeCode *)nmethod->setSourceObject(this);
    /* turn into a real method object    */
    return new RexxMethod(name, nmethod);
}

/**
 * Decode an external library method specification.
 *
 * @param methodName The internal name of the method (uppercased).
 * @param externalSpec
 *                   The external specification string.
 * @param library    The returned library name.
 * @param procedure  The returned package name.
 */
void RexxSource::decodeExternalMethod(RexxString *methodName, RexxString *externalSpec, RexxString *&library, RexxString *&procedure)
{
    // this is the default
    procedure = methodName;
    library = OREF_NULL;

                            /* convert external into words       */
    RexxArray *_words = this->words(externalSpec);
    /* not 'LIBRARY library [entry]' form? */
    if (((RexxString *)(_words->get(1)))->strCompare(CHAR_LIBRARY))
    {
        // full library with entry name version?
        if (_words->size() == 3)
        {
            library = (RexxString *)_words->get(2);
            procedure = (RexxString *)_words->get(3);
        }
        else if (_words->size() == 2)
        {
            library = (RexxString *)_words->get(2);
        }
        else  // wrong number of tokens
        {
                                     /* this is an error                  */
            syntaxError(Error_Translation_bad_external, externalSpec);
        }
    }
    else
    {
        /* unknown external type             */
        syntaxError(Error_Translation_bad_external, externalSpec);
    }
}

#define ATTRIBUTE_BOTH 0
#define ATTRIBUTE_GET  1
#define ATTRIBUTE_SET  2


/**
 * Process a ::ATTRIBUTE directive in a source file.
 */
void RexxSource::attributeDirective()
{
    int  Private = DEFAULT_ACCESS_SCOPE;    /* this is a public method           */
    int  Protected = DEFAULT_PROTECTION;  /* and is not protected yet          */
    int  guard = DEFAULT_GUARD;       /* default is guarding               */
    int  style = ATTRIBUTE_BOTH;      // by default, we create both methods for the attribute.
    bool Class = false;              /* default is an instance method     */
    RexxToken *token = nextReal();   /* get the next token                */

                                     /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_attribute, token);
    }
    RexxString *name = token->value; /* get the string name               */
                                     /* and the name form also            */
    RexxString *internalname = this->commonString(name->upper());
    RexxString *externalname = OREF_NULL;

    for (;;)
    {                       /* now loop on the option keywords   */
        token = nextReal();            /* get the next token                */
                                       /* reached the end?                  */
        if (token->isEndOfClause())
        {
            break;                       /* get out of here                   */
        }
                                         /* not a symbol token?               */
        else if (!token->isSymbol())
        {
            /* report an error                   */
            syntaxError(Error_Invalid_subkeyword_method, token);
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
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    style = ATTRIBUTE_GET;
                    break;

                case SUBDIRECTIVE_SET:
                    // only one of GET/SET allowed
                    if (style != ATTRIBUTE_BOTH)
                    {
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    style = ATTRIBUTE_SET;
                    break;


                /* ::ATTRIBUTE name CLASS               */
                case SUBDIRECTIVE_CLASS:
                    if (Class)               /* had one of these already?         */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Class = true;            /* flag this for later processing    */
                    break;
                case SUBDIRECTIVE_PRIVATE:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Private = PRIVATE_SCOPE;           /* flag for later processing         */
                    break;
                    /* ::METHOD name PUBLIC             */
                case SUBDIRECTIVE_PUBLIC:
                    if (Private != DEFAULT_ACCESS_SCOPE)   /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Private = PUBLIC_SCOPE;        /* flag for later processing         */
                    break;
                    /* ::METHOD name PROTECTED           */
                case SUBDIRECTIVE_PROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Protected = PROTECTED_METHOD;        /* flag for later processing         */
                    break;
                case SUBDIRECTIVE_UNPROTECTED:
                    if (Protected != DEFAULT_PROTECTION)           /* already seen one of these?        */
                    {
                                             /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    Protected = UNPROTECTED_METHOD;      /* flag for later processing         */
                    break;
                    /* ::METHOD name UNGUARDED           */
                case SUBDIRECTIVE_UNGUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    guard = UNGUARDED_METHOD;/* flag for later processing         */
                    break;
                    /* ::METHOD name GUARDED             */
                case SUBDIRECTIVE_GUARDED:
                    /* already seen one of these?        */
                    if (guard != DEFAULT_GUARD)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    guard = GUARDED_METHOD;  /* flag for later processing         */
                    break;
                    /* ::METHOD name ATTRIBUTE           */
                case SUBDIRECTIVE_EXTERNAL:
                    /* already had an external?          */
                    if (externalname != OREF_NULL)
                    {
                        /* duplicates are invalid            */
                        syntaxError(Error_Invalid_subkeyword_method, token);
                    }
                    token = nextReal();      /* get the next token                */
                                             /* not a string?                     */
                    if (!token->isSymbolOrLiteral())
                    {
                        /* report an error                   */
                        syntaxError(Error_Symbol_or_string_external, token);
                    }
                    externalname = token->value;
                    break;


                default:                   /* invalid keyword                   */
                    /* this is an error                  */
                    syntaxError(Error_Invalid_subkeyword_method, token);
                    break;
            }
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
            checkDuplicateMethod(internalname, Class, Error_Translation_duplicate_attribute);
            // now get this as the setter method.
            RexxString *setterName = commonString(internalname->concatWithCstring("="));
            checkDuplicateMethod(setterName, Class, Error_Translation_duplicate_attribute);

            // no code can follow the automatically generated methods
            this->checkDirective();
            if (externalname != OREF_NULL)
            {
                RexxString *library = OREF_NULL;
                RexxString *procedure = OREF_NULL;
                decodeExternalMethod(internalname, externalname, library, procedure);
                // now create both getter and setting methods from the information.
                RexxMethod *_method = createNativeMethod(internalname, library, procedure->concatToCstring("GET"));
                _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                // add to the compilation
                addMethod(internalname, _method, Class);

                _method = createNativeMethod(setterName, library, procedure->concatToCstring("SET"));
                _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                // add to the compilation
                addMethod(setterName, _method, Class);
            }
            else
            {
                // create the method pair and quit.
                createAttributeGetterMethod(internalname, retriever, Class, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                createAttributeSetterMethod(setterName, retriever, Class, Private == PRIVATE_SCOPE,
                    Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
            }
            break;

        }

        case ATTRIBUTE_GET:       // just the getter method
        {
            checkDuplicateMethod(internalname, Class, Error_Translation_duplicate_attribute);
            // external?  resolve the method
            if (externalname != OREF_NULL)
            {
                // no code can follow external methods
                this->checkDirective();
                RexxString *library = OREF_NULL;
                RexxString *procedure = OREF_NULL;
                decodeExternalMethod(internalname, externalname, library, procedure);
                // if there was no procedure explicitly given, create one using the GET/SET convention
                if (internalname == procedure)
                {
                    procedure = procedure->concatToCstring("GET");
                }
                // now create both getter and setting methods from the information.
                RexxMethod *_method = createNativeMethod(internalname, library, procedure);
                _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                // add to the compilation
                addMethod(internalname, _method, Class);
            }
            // either written in ooRexx or is automatically generated.
            else {
                if (hasBody())
                {
                    createMethod(internalname, Class, Private == PRIVATE_SCOPE,
                        Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                }
                else
                {
                    createAttributeGetterMethod(internalname, retriever, Class, Private == PRIVATE_SCOPE,
                        Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                }
            }
            break;
        }

        case ATTRIBUTE_SET:
        {
            // now get this as the setter method.
            RexxString *setterName = commonString(internalname->concatWithCstring("="));
            checkDuplicateMethod(setterName, Class, Error_Translation_duplicate_attribute);
            // external?  resolve the method
            if (externalname != OREF_NULL)
            {
                // no code can follow external methods
                this->checkDirective();
                RexxString *library = OREF_NULL;
                RexxString *procedure = OREF_NULL;
                decodeExternalMethod(internalname, externalname, library, procedure);
                // if there was no procedure explicitly given, create one using the GET/SET convention
                if (internalname == procedure)
                {
                    procedure = procedure->concatToCstring("SET");
                }
                // now create both getter and setting methods from the information.
                RexxMethod *_method = createNativeMethod(setterName, library, procedure);
                _method->setAttributes(Private == PRIVATE_SCOPE, Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                // add to the compilation
                addMethod(setterName, _method, Class);
            }
            else
            {
                if (hasBody())        // just the getter method
                {
                    createMethod(setterName, Class, Private == PRIVATE_SCOPE,
                        Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                }
                else
                {
                    createAttributeSetterMethod(setterName, retriever, Class, Private == PRIVATE_SCOPE,
                        Protected == PROTECTED_METHOD, guard != UNGUARDED_METHOD);
                }
            }
            break;
        }
    }
}


/**
 * Process a ::CONSTANT directive in a source file.
 */
void RexxSource::constantDirective()
{
    RexxToken *token = nextReal();   /* get the next token                */
                                     /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_constant, token);
    }
    RexxString *name = token->value; /* get the string name               */
                                     /* and the name form also            */
    RexxString *internalname = this->commonString(name->upper());

    // we only expect just a single value token here
    token = nextReal();                /* get the next token                */
    RexxObject *value;
                                       /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        // if not a "+" or "-" operator, this is an error
        if (!token->isOperator() || (token->subclass != OPERATOR_SUBTRACT && token->subclass != OPERATOR_PLUS))
        {
            /* report an error                   */
            syntaxError(Error_Symbol_or_string_constant_value, token);
        }
        RexxToken *second = nextReal();
        // this needs to be a constant symbol...we check for
        // numeric below
        if (!second->isSymbol() || second->subclass != SYMBOL_CONSTANT)
        {
            /* report an error                   */
            syntaxError(Error_Symbol_or_string_constant_value, token);
        }
        // concat with the sign operator
        value = token->value->concat(second->value);
        // and validate that this a valid number
        if (value->numberString() == OREF_NULL)
        {
            /* report an error                   */
            syntaxError(Error_Symbol_or_string_constant_value, token);
        }
    }
    else
    {
        // this will be some sort of literal value
        value = this->commonString(token->value);
    }

    token = nextReal();                /* get the next token                */
    // No other options on this instruction
    if (!token->isEndOfClause())
    {
        /* report an error                   */
        syntaxError(Error_Invalid_data_constant_dir, token);
    }
    // this directive does not allow a body
    this->checkDirective();

    // check for duplicates.  We only do the class duplicate check if there
    // is an active class, otherwise we'll get a syntax error
    checkDuplicateMethod(internalname, false, Error_Translation_duplicate_constant);
    if (this->active_class != OREF_NULL)
    {
        checkDuplicateMethod(internalname, true, Error_Translation_duplicate_constant);
    }

    // create the method pair and quit.
    createConstantGetterMethod(internalname, value);
}


/**
 * Create a Rexx method body.
 *
 * @param name   The name of the attribute.
 * @param classMethod
 *               Indicates whether we are creating a class or instance method.
 * @param privateMethod
 *               The method's private attribute.
 * @param protectedMethod
 *               The method's protected attribute.
 * @param guardedMethod
 *               The method's guarded attribute.
 */
void RexxSource::createMethod(RexxString *name, bool classMethod,
    bool privateMethod, bool protectedMethod, bool guardedMethod)
{
    // NOTE:  It is necessary to translate the block and protect the code
    // before allocating the RexxMethod object.  The new operator allocates the
    // the object first, then evaluates the constructor arguments after the allocation.
    // Since the translateBlock() call will allocate a lot of new objects before returning,
    // there's a high probability that the method object can get garbage collected before
    // there is any opportunity to protect the object.
    RexxCode *code = this->translateBlock(OREF_NULL);
    this->saveObject((RexxObject *)code);

    /* go do the next block of code      */
    RexxMethod *_method = new RexxMethod(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // go add the method to the accumulator
    addMethod(name, _method, classMethod);
}


/**
 * Create an ATTRIBUTE "get" method.
 *
 * @param name      The name of the attribute.
 * @param retriever
 * @param classMethod
 *                  Indicates we're adding a class or instance method.
 * @param privateMethod
 *                  The method's private attribute.
 * @param protectedMethod
 *                  The method's protected attribute.
 * @param guardedMethod
 *                  The method's guarded attribute.
 */
void RexxSource::createAttributeGetterMethod(RexxString *name, RexxVariableBase *retriever,
    bool classMethod, bool privateMethod, bool protectedMethod, bool guardedMethod)
{
    // create the kernel method for the accessor
    BaseCode *code = new AttributeGetterCode(retriever);
    RexxMethod *_method = new RexxMethod(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // add this to the target
    addMethod(name, _method, classMethod);
}


/**
 * Create an ATTRIBUTE "set" method.
 *
 * @param name   The name of the attribute.
 * @param classMethod
 *                  Indicates we're adding a class or instance method.
 * @param privateMethod
 *               The method's private attribute.
 * @param protectedMethod
 *               The method's protected attribute.
 * @param guardedMethod
 *               The method's guarded attribute.
 */
void RexxSource::createAttributeSetterMethod(RexxString *name, RexxVariableBase *retriever,
    bool classMethod, bool privateMethod, bool protectedMethod, bool guardedMethod)
{
    // create the kernel method for the accessor
    BaseCode *code = new AttributeSetterCode(retriever);
    RexxMethod *_method = new RexxMethod(name, code);
    _method->setAttributes(privateMethod, protectedMethod, guardedMethod);
    // add this to the target
    addMethod(name, _method, classMethod);
}


/**
 * Create a CONSTANT "get" method.
 *
 * @param target The target method directory.
 * @param name   The name of the attribute.
 */
void RexxSource::createConstantGetterMethod(RexxString *name, RexxObject *value)
{
    ConstantGetterCode *code = new ConstantGetterCode(value);
    // add this as an unguarded method
    RexxMethod *method = new RexxMethod(name, code);
    method->setUnguarded();
    if (active_class == OREF_NULL)
    {
        addMethod(name, method, false);
    }
    else
    {
        active_class->addConstantMethod(name, method);
    }
}


/**
 * Process a ::routine directive in a source file.
 */
void RexxSource::routineDirective()
{
    RexxToken *token = nextReal();   /* get the next token                */
                                     /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_routine, token);
    }
    RexxString *name = token->value; /* get the routine name              */
                                     /* does this already exist?          */
    if (this->routines->entry(name) != OREF_NULL)
    {
        /* have an error here                */
        syntaxError(Error_Translation_duplicate_routine);
    }
    this->flags |= _install;         /* have information to install       */
    RexxString *externalname = OREF_NULL;        /* no external name yet              */
    int Public = DEFAULT_ACCESS_SCOPE;      /* not a public routine yet          */
    for (;;)                         /* now loop on the option keywords   */
    {
        token = nextReal();            /* get the next token                */
                                       /* reached the end?                  */
        if (token->isEndOfClause())
        {
            break;                       /* get out of here                   */
        }
        /* not a symbol token?               */
        else if (!token->isSymbol())
        {
            /* report an error                   */
            syntaxError(Error_Invalid_subkeyword_routine, token);
        }
        /* process each sub keyword          */
        switch (this->subDirective(token))
        {
            /* ::ROUTINE name EXTERNAL []*/
            case SUBDIRECTIVE_EXTERNAL:
                /* already had an external?          */
                if (externalname != OREF_NULL)
                {
                    /* duplicates are invalid            */
                    syntaxError(Error_Invalid_subkeyword_class, token);
                }
                token = nextReal();        /* get the next token                */
                /* not a string?                     */
                if (!token->isSymbolOrLiteral())
                {
                    /* report an error                   */
                    syntaxError(Error_Symbol_or_string_requires, token);
                }
                /* external name is token value      */
                externalname = token->value;
                break;
                /* ::ROUTINE name PUBLIC             */
            case SUBDIRECTIVE_PUBLIC:
                if (Public != DEFAULT_ACCESS_SCOPE)   /* already had one of these?         */
                {
                    /* duplicates are invalid            */
                    syntaxError(Error_Invalid_subkeyword_routine, token);

                }
                Public = PUBLIC_SCOPE;     /* turn on the seen flag             */
                break;
                /* ::ROUTINE name PUBLIC             */
            case SUBDIRECTIVE_PRIVATE:
                if (Public != DEFAULT_ACCESS_SCOPE)   /* already had one of these?         */
                {
                    /* duplicates are invalid            */
                    syntaxError(Error_Invalid_subkeyword_routine, token);

                }
                Public = PRIVATE_SCOPE;    /* turn on the seen flag             */
                break;

            default:                     /* invalid keyword                   */
                /* this is an error                  */
                syntaxError(Error_Invalid_subkeyword_routine, token);
                break;
        }
    }
    {
        this->saveObject(name);          /* protect the name                  */

        if (externalname != OREF_NULL)   /* have an external routine?         */
        {
            /* convert external into words       */
            RexxArray *_words = this->words(externalname);
            // ::ROUTINE foo EXTERNAL "LIBRARY libbar [foo]"
            if (((RexxString *)(_words->get(1)))->strCompare(CHAR_LIBRARY))
            {
                RexxString *library = OREF_NULL;
                // the default entry point name is the internal name
                RexxString *entry = name;

                // full library with entry name version?
                if (_words->size() == 3)
                {
                    library = (RexxString *)_words->get(2);
                    entry = (RexxString *)_words->get(3);
                }
                else if (_words->size() == 2)
                {
                    library = (RexxString *)_words->get(2);
                }
                else  // wrong number of tokens
                {
                    /* this is an error                  */
                    syntaxError(Error_Translation_bad_external, externalname);
                }

                /* go check the next clause to make  */
                this->checkDirective();      /* sure no code follows              */
                                             /* create a new native method        */
                RoutineClass *routine = PackageManager::resolveRoutine(library, entry);
                // raise an exception if this entry point is not found.
                if (routine == OREF_NULL)
                {
                     syntaxError(Error_External_name_not_found_routine, entry);
                }
                // make sure this is attached to the source object for context information
                routine->setSourceObject(this);
                /* add to the routine directory      */
                this->routines->setEntry(name, routine);
                if (Public == PUBLIC_SCOPE)    /* a public routine?                 */
                {
                    /* add to the public directory too   */
                    this->public_routines->setEntry(name, routine);
                }
            }

            // ::ROUTINE foo EXTERNAL "REGISTERED libbar [foo]"
            else if (((RexxString *)(_words->get(1)))->strCompare(CHAR_REGISTERED))
            {
                RexxString *library = OREF_NULL;
                // the default entry point name is the internal name
                RexxString *entry = name;

                // full library with entry name version?
                if (_words->size() == 3)
                {
                    library = (RexxString *)_words->get(2);
                    entry = (RexxString *)_words->get(3);
                }
                else if (_words->size() == 2)
                {
                    library = (RexxString *)_words->get(2);
                }
                else  // wrong number of tokens
                {
                    /* this is an error                  */
                    syntaxError(Error_Translation_bad_external, externalname);
                }

                /* go check the next clause to make  */
                this->checkDirective();      /* sure no code follows              */
                                             /* create a new native method        */
                RoutineClass *routine = PackageManager::resolveRoutine(name, library, entry);
                // raise an exception if this entry point is not found.
                if (routine == OREF_NULL)
                {
                     syntaxError(Error_External_name_not_found_routine, entry);
                }
                // make sure this is attached to the source object for context information
                routine->setSourceObject(this);
                /* add to the routine directory      */
                this->routines->setEntry(name, routine);
                if (Public == PUBLIC_SCOPE)    /* a public routine?                 */
                {
                    /* add to the public directory too   */
                    this->public_routines->setEntry(name, routine);
                }
            }
            else
            {
                /* unknown external type             */
                syntaxError(Error_Translation_bad_external, externalname);
            }
        }
        else
        {
            // NOTE:  It is necessary to translate the block and protect the code
            // before allocating the RexxMethod object.  The new operator allocates the
            // the object first, then evaluates the constructor arguments after the allocation.
            // Since the translateBlock() call will allocate a lot of new objects before returning,
            // there's a high probability that the method object can get garbage collected before
            // there is any opportunity to protect the object.
            RexxCode *code = this->translateBlock(OREF_NULL);
            this->saveObject((RexxObject *)code);
            RoutineClass *routine = new RoutineClass(name, code);
            /* add to the routine directory      */
            this->routines->setEntry(name, routine);
            if (Public == PUBLIC_SCOPE)    /* a public routine?                 */
            {
                /* add to the public directory too   */
                this->public_routines->setEntry(name, routine);

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
    RexxToken *token = nextReal();   /* get the next token                */
                                     /* not a symbol or a string          */
    if (!token->isSymbolOrLiteral())
    {
        /* report an error                   */
        syntaxError(Error_Symbol_or_string_requires, token);
    }
    RexxString *name = token->value; /* get the requires name             */
    token = nextReal();              /* get the next token                */
    if (!token->isEndOfClause()) /* something appear after this?      */
    {
        // this is potentially a library directive
        libraryDirective(name, token);
        return;
    }
    this->flags |= _install;         /* have information to install       */
    /* save the ::requires location      */
    this->requires->append((RexxObject *)new RequiresDirective(name, this->clause));
}


/**
 * Process a ::REQUIRES name LIBRARY directive.
 */
void RexxSource::libraryDirective(RexxString *name, RexxToken *token)
{
    // we have an extra token on a ::REQUIRES directive.  The only thing accepted here
    // is the token LIBRARY.
    if (!token->isSymbol())
    {
        syntaxError(Error_Invalid_subkeyword_requires, token);
    }
                                   /* process each sub keyword          */
    if (subDirective(token) != SUBDIRECTIVE_LIBRARY)
    {
        syntaxError(Error_Invalid_subkeyword_requires, token);
    }
    token = nextReal();              /* get the next token                */
    if (!token->isEndOfClause()) /* something appear after this?      */
    {
        // nothing else allowed after this
        syntaxError(Error_Invalid_subkeyword_requires, token);
    }
    this->flags |= _install;         /* have information to install       */
    // add this to the library list
    this->libraries->append((RexxObject *)new LibraryDirective(name, this->clause));
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
        syntaxError(Error_Translation_bad_directive);
    token = nextReal();                  /* get the keyword token             */
    if (!token->isSymbol())  /* not a symbol?                     */
                                         /* have an error here                */
        syntaxError(Error_Symbol_expected_directive);

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

        case DIRECTIVE_CONSTANT:           /* ::CONSTANT directive              */
            constantDirective();
            break;

        case DIRECTIVE_OPTIONS:            /* ::OPTIONS directive               */
            optionsDirective();
            break;

        default:                           /* unknown directive                 */
            syntaxError(Error_Translation_bad_directive);
            break;
    }
}


void RexxSource::flushControl(
    RexxInstruction *_instruction)      /* next instruction                  */
/******************************************************************************/
/* Function:  Flush any pending compound instructions from the control stack  */
/*            for new added instructions                                      */
/******************************************************************************/
{
    size_t           type;               /* instruction type                  */
    RexxInstruction *second;             /* additional created instructions   */

    /* loop through the control stack    */
    for (;;)
    {
        type = this->topDo()->getType();   /* get the instruction type          */
        /* pending ELSE close?               */
        if (type == KEYWORD_ELSE)
        {
            second = this->popDo();          /* pop the item off of the control   */
                                             /* create a new end marker           */
            second = this->endIfNew((RexxInstructionIf *)second);
            /* have an instruction?              */
            if (_instruction != OREF_NULL)
            {
                this->addClause(_instruction);  /* add this here                     */
                _instruction = OREF_NULL;       /* don't process more instructions   */
            }
            this->addClause(second);         /* add the clause to the list        */
        }
        /* nested IF-THEN situation?         */
        else if (type == KEYWORD_IFTHEN || type == KEYWORD_WHENTHEN)
        {
            second = this->popDo();          /* pop the item off of the control   */
            /* have an instruction?              */
            if (_instruction != OREF_NULL)
            {
                this->addClause(_instruction); /* add this here                     */
                _instruction = OREF_NULL;      /* don't process more instructions   */
                                               /* create a new end marker           */
                second = this->endIfNew((RexxInstructionIf *)second);
                this->addClause(second);       /* add the clause to the list        */
                this->pushDo(second);          /* add to the control stack too      */
            }
            else
            {
                /* create a new end marker           */
                second = this->endIfNew((RexxInstructionIf *)second);
                this->addClause(second);       /* add the clause to the list        */
                this->pushDo(second);          /* add to the control stack too      */
            }
            break;                           /* finish up here                    */
        }
        /* some other type of construct      */
        else
        {
            if (_instruction != OREF_NULL)    /* have an instruction?              */
            {
                this->addClause(_instruction);  /* add this here                     */
            }
            break;                           /* finished flushing                 */
        }
    }
}

RexxCode *RexxSource::translateBlock(
    RexxDirectory *_labels )            /* labels (for interpret)            */
/******************************************************************************/
/* Function:  Translate a block of REXX code (delimited by possible           */
/*            directive instructions                                          */
/******************************************************************************/
{
    RexxInstruction *_instruction;        /* created instruction item          */
    RexxInstruction *second;             /* secondary clause for IF/WHEN      */
    RexxToken       *token;              /* current working token             */
    size_t           type;               /* instruction type information      */
    size_t           controltype;        /* type on the control stack         */

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
    {
        /* just use the existing label set   */
        OrefSet(this, this->labels, _labels);
    }
    else
    {
        /* create a new labels directory     */
        OrefSet(this, this->labels, new_directory());
    }
    /* not collecting guard variables yet*/
    OrefSet(this, this->guard_variables, OREF_NULL);
    this->maxstack = 0;                  /* clear all of the stack accounting */
    this->currentstack = 0;              /* fields                            */
    this->flags &= ~no_clause;           /* not reached the end yet           */

                                         /* add the first dummy instruction   */
    _instruction = new RexxInstruction(OREF_NULL, KEYWORD_FIRST);
    this->pushDo(_instruction);           /* set bottom of control stack       */
    this->addClause(_instruction);        /* add to the instruction list       */
    this->nextClause();                  /* get the next physical clause      */
    for (;;)                             /* process all clauses               */
    {
        _instruction = OREF_NULL;           /* zero the instruction pointer      */
        while (!(this->flags&no_clause))   /* scan through all labels           */
        {
            /* resolve the instruction type      */
            _instruction = this->instruction();
            if (_instruction == OREF_NULL)    /* found a directive clause?         */
            {
                break;                         /* return to higher level            */
            }
            /* is this a label?                  */
            if (_instruction->getType() != KEYWORD_LABEL)
            {
                break;                         /* have a non-label clause           */
            }
            this->addClause(_instruction);    /* add this to clause list           */
            this->nextClause();              /* get the next physical clause      */
            _instruction = OREF_NULL;         /* no instruction any more           */
        }
        /* get an end-of-clause?             */
        if (this->flags&no_clause || _instruction == OREF_NULL)
        {
            /* get the control stack type        */
            controltype = this->topDo()->getType();
            /* while end of an IF or WHEN        */
            while (controltype == KEYWORD_ENDTHEN || controltype == KEYWORD_ENDWHEN)
            {
                this->popDo();                 /* pop pending closing IFs           */
                this->flushControl(OREF_NULL); /* flush any IFs or ELSEs            */
                                               /* get the control stack type        */
                controltype = this->topDo()->getType();
            }
            /* any unclosed composite clauses?   */
            if (this->topDo()->getType() != KEYWORD_FIRST)
            {
                /* report the block error            */
                blockSyntaxError(this->topDo());
            }
            this->popDo();                   /* remove the top one                */
            break;                           /* done parsing this section         */
        }
        type = _instruction->getType();     /* get the top instruction type      */
        if (type != KEYWORD_ELSE)          /* have a pending THEN to finish     */
        {
            /* get the control stack type        */
            controltype = this->topDo()->getType();
            /* while end of an IF or WHEN        */
            while (controltype == KEYWORD_ENDTHEN || controltype == KEYWORD_ENDWHEN)
            {
                this->popDo();                 /* pop pending closing IFs           */
                this->flushControl(OREF_NULL); /* flush any IFs or ELSEs            */
                                               /* get the control stack type        */
                controltype = this->topDo()->getType();
            }
        }
        if (type == KEYWORD_IF || type == KEYWORD_SELECT || type == KEYWORD_DO)
        {
            this->addClause(_instruction);   /* add to instruction heap           */
        }
        else if (type != KEYWORD_ELSE)     /* not a new control level           */
        {
            this->flushControl(_instruction); /* flush any IFs or ELSEs            */
        }
        /* have a bad instruction within a   */
        /* SELECT instruction?               */
        if (this->topDo()->getType() == KEYWORD_SELECT &&
            (type != KEYWORD_WHEN && type != KEYWORD_OTHERWISE && type != KEYWORD_END ))
        {
            syntaxError(Error_When_expected_whenotherwise, this->topDo());
        }

        switch (type)                      /* post process the instructions     */
        {
            case KEYWORD_WHEN:               /* WHEN clause of SELECT             */
                second = this->topDo();        /* get the top of the queue          */
                                               /* not working on a SELECT?          */
                if (second->getType() != KEYWORD_SELECT)
                {
                    syntaxError(Error_Unexpected_when_when);
                }
                /* add this to the select list       */
                ((RexxInstructionSelect *)second)->addWhen((RexxInstructionIf *)_instruction);
                /* just fall into IF logic           */

            case  KEYWORD_IF:                /* start of an IF instruction        */
                token = nextReal();            /* get the terminator token          */
                                               /* have a terminator before the THEN?*/
                if (token->isEndOfClause())
                {
                    this->nextClause();          /* get the next physical clause      */
                    if (this->flags&no_clause)   /* get an end-of-file?               */
                    {
                        /* raise an error                    */
                        syntaxError(Error_Then_expected_if, _instruction);
                    }
                    token = nextReal();          /* get the first token               */
                                                 /* not a THEN keyword?               */
                    if (!token->isSymbol() || this->keyword(token) != KEYWORD_THEN)
                    {
                        /* have an error                     */
                        syntaxError(Error_Then_expected_if, _instruction);
                    }
                    /* create a new then clause          */
                    second = this->thenNew(token, (RexxInstructionIf *)_instruction);
                    token = nextReal();          /* get token after THEN keyword      */
                                                 /* terminator here?                  */
                    if (token->isEndOfClause())
                    {
                        this->nextClause();        /* get the next physical clause      */
                        if (this->flags&no_clause) /* get an end-of-file?               */
                        {
                            /* raise an error                    */
                            syntaxError(Error_Incomplete_do_then, _instruction);
                        }
                    }
                    else
                    {
                        previousToken();           /* step back a token                 */
                        trimClause();              /* make this start of the clause     */
                    }
                }
                else                           /* if expr THEN form                 */
                {
                    /* create a new then clause          */
                    second = this->thenNew(token, (RexxInstructionIf *)_instruction);
                    token = nextReal();          /* get token after THEN keyword      */
                                                 /* terminator here?                  */
                    if (token->isEndOfClause())
                    {
                        this->nextClause();        /* get the next physical clause      */
                        if (this->flags&no_clause) /* get an end-of-file?               */
                        {
                            /* raise an error                    */
                            syntaxError(Error_Incomplete_do_then, _instruction);
                        }
                    }
                    else
                    {
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
                {
                    /* have an error                     */
                    syntaxError(Error_Unexpected_then_else);
                }
                this->addClause(_instruction); /* add to instruction heap           */
                second = this->popDo();        /* pop the ENDTHEN item              */
                this->pushDo(_instruction);     /* add to the control list           */
                /* join the THEN and ELSE together   */
                ((RexxInstructionElse *)_instruction)->setParent((RexxInstructionEndIf *)second);
                ((RexxInstructionEndIf *)second)->setEndInstruction((RexxInstructionEndIf *)_instruction);
                token = nextReal();            /* get the next token                */
                                               /* have an ELSE keyword alone?       */
                if (token->isEndOfClause())
                {
                    this->nextClause();          /* get the next physical clause      */
                    if (this->flags&no_clause)   /* get an end-of-file?               */
                    {
                        /* raise an error                    */
                        syntaxError(Error_Incomplete_do_else, _instruction);
                    }
                }
                else                           /* ELSE instruction form             */
                {
                    previousToken();             /* step back a token                 */
                    trimClause();                /* make this start of the clause     */
                }
                continue;                      /* straight around to process clause */
                                               /* remainder                         */

            case  KEYWORD_OTHERWISE:         /* start of an OTHERWISE group       */
                second = this->topDo();        /* get the top of the queue          */
                                               /* not working on a SELECT?          */
                if (second->getType() != KEYWORD_SELECT)
                {
                    syntaxError(Error_Unexpected_when_otherwise);
                }
                /* hook up the OTHERWISE instruction */
                ((RexxInstructionSelect *)second)->setOtherwise((RexxInstructionOtherwise *)_instruction);
                this->pushDo(_instruction);     /* add this to the control queue     */
                token = nextReal();            /* get the next token                */
                                               /* OTHERWISE instr form?             */
                if (!token->isEndOfClause())
                {
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
                if (type != KEYWORD_SELECT && type != KEYWORD_OTHERWISE && type != KEYWORD_DO)
                {
                    if (type == KEYWORD_ELSE)    /* on an else?                       */
                    {
                        /* give the specific error           */
                        syntaxError(Error_Unexpected_end_else);
                    }
                    else if (type == KEYWORD_IFTHEN || type == KEYWORD_WHENTHEN)
                    {
                        /* this is a different error         */
                        syntaxError(Error_Unexpected_end_then);
                    }
                    else
                    {
                        /* have a misplaced END              */
                        syntaxError(Error_Unexpected_end_nodo);
                    }
                }
                if (type == KEYWORD_OTHERWISE) /* OTHERWISE part of a SELECT?       */
                {
                    second = this->popDo();      /* need to pop one more item off     */
                }
                /* matching a select?                */
                if (second->getType() == KEYWORD_SELECT)
                {
                    /* match up the instruction          */
                    ((RexxInstructionSelect *)second)->matchEnd((RexxInstructionEnd *)_instruction, this);
                }
                else                           /* must be a DO block                */
                {
                    /* match up the instruction          */
                    ((RexxInstructionDo *)second)->matchEnd((RexxInstructionEnd *)_instruction, this);
                }
                this->flushControl(OREF_NULL); /* finish pending IFs or ELSEs       */
                break;

            case  KEYWORD_DO:                // start of new DO group (also picks up LOOP instruction)
                this->pushDo(_instruction);    /* add this to the control queue     */
                break;

            case  KEYWORD_SELECT:            /* start of new SELECT group         */
                this->pushDo(_instruction);    /* and also to the control queue     */
                break;

            default:                         /* other types of instruction        */
                break;
        }
        this->nextClause();                /* get the next physical clause      */
    }
    /* now go resolve any label targets  */
    _instruction = (RexxInstruction *)(this->calls->removeFirst());
    /* while still more references       */
    while (_instruction != (RexxInstruction *)TheNilObject)
    {
        /* actually a function call?         */
        if (isOfClass(FunctionCallTerm, _instruction))
        {
            /* resolve the function call         */
            ((RexxExpressionFunction *)_instruction)->resolve(this->labels);
        }
        else
        {
            /* resolve the CALL/SIGNAL/FUNCTION  */
            /* label targets                     */
            ((RexxInstructionCallBase *)_instruction)->resolve(this->labels);
        }
        /* now get the next instruction      */
        _instruction = (RexxInstruction *)(this->calls->removeFirst());
    }
    /* remove the first instruction      */
    OrefSet(this, this->first, this->first->nextInstruction);
    /* no labels needed?                 */
    if (this->labels != OREF_NULL && this->labels->items() == 0)
    {
        /* release that directory also       */
        OrefSet(this, this->labels, OREF_NULL);
    }
    /* create a rexx code object         */
    return new RexxCode(this, this->first, this->labels, (this->maxstack+ 10), this->variableindex);
}

RexxInstruction *RexxSource::instruction()
/******************************************************************************/
/* Function:  Process an individual REXX clause                               */
/******************************************************************************/
{
    RexxToken       *_first;              /* first token of clause             */
    RexxToken       *second;             /* second token of clause            */
    RexxInstruction *_instruction;        /* current working instruction       */
    RexxObject      *term;               /* term for a message send           */
    RexxObject      *subexpression;      /* subexpression of a clause         */
    int              _keyword;            /* resolved instruction keyword      */

    _instruction = OREF_NULL;             /* default to no instruction found   */
    _first = nextReal();                  /* get the first token               */

    if (_first->classId == TOKEN_DCOLON)
    {/* reached the end of a block?       */
        firstToken();                      /* reset the location                */
        this->reclaimClause();             /* give back the clause              */
    }
    else
    {                               /* have a real instruction to process*/
        second = nextToken();              /* now get the second token          */
                                           /* is this a label?  (can be either  */
                                           /* a symbol or a literal)            */
        if ((_first->classId == TOKEN_SYMBOL || _first->classId == TOKEN_LITERAL) && second->classId == TOKEN_COLON)
        {
            if (this->flags&_interpret)      /* is this an interpret?             */
            {
                                             /* this is an error                  */
                syntaxError(Error_Unexpected_label_interpret, _first);
            }
            firstToken();                    /* reset to the beginning            */
            _instruction = this->labelNew(); /* create a label instruction        */
            second = nextToken();            /* get the next token                */
                                             /* not the end of the clause?        */
            if (!second->isEndOfClause())
            {
                previousToken();               /* give this token back              */
                trimClause();                  /* make this start of the clause     */
                this->reclaimClause();         /* give the remaining clause back    */
            }
            return _instruction;
        }

        // this is potentially an assignment of the form "symbol = expr"
        if (_first->isSymbol())
        {
            // "symbol == expr" is considered an error
            if (second->subclass == OPERATOR_STRICT_EQUAL)
            {
                syntaxError(Error_Invalid_expression_general, second);
            }
            // true assignment instruction?
            if (second->subclass == OPERATOR_EQUAL)
            {
                return this->assignmentNew(_first);
            }
            // this could be a special assignment operator such as "symbol += expr"
            else if (second->classId == TOKEN_ASSIGNMENT)
            {
                return this->assignmentOpNew(_first, second);
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
            if (second->isEndOfClause())
            {
                return this->messageNew((RexxExpressionMessage *)term);
            }
            else if (second->subclass == OPERATOR_STRICT_EQUAL)
            {
                // messageterm == something is an invalid assignment
                syntaxError(Error_Invalid_expression_general, second);
            }
            // messageterm = something is a pseudo assignment
            else if (second->subclass == OPERATOR_EQUAL)
            {
                this->saveObject(term);      /* protect this                      */
                // we need an expression following the op token
                subexpression = this->subExpression(TERM_EOC);
                if (subexpression == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_general, second);
                }
                // this is a message assignment
                _instruction = this->messageAssignmentNew((RexxExpressionMessage *)term, subexpression);
                this->toss(term);              /* release the term                  */
                return _instruction;
            }
            // one of the special operator forms?
            else if (second->classId == TOKEN_ASSIGNMENT)
            {
                this->saveObject(term);      /* protect this                      */
                // we need an expression following the op token
                subexpression = this->subExpression(TERM_EOC);
                if (subexpression == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_general, second);
                }
                // this is a message assignment
                _instruction = this->messageAssignmentOpNew((RexxExpressionMessage *)term, second, subexpression);
                this->toss(term);              /* release the term                  */
                return _instruction;
            }
        }

        // ok, none of the special cases passed....not start the keyword processing

        firstToken();                  /* reset to the first token          */
        _first = nextToken();          /* get the first token again         */
                                       /* is first a symbol that matches a  */
                                       /* defined REXX keyword?             */
        if (_first->isSymbol() && (_keyword = this->keyword(_first)))
        {

            switch (_keyword)
            {           /* process each instruction type     */

                case KEYWORD_NOP:          /* NOP instruction                   */
                    /* add the instruction to the parse  */
                    _instruction = this->nopNew();
                    break;

                case KEYWORD_DROP:         /* DROP instruction                  */
                    /* add the instruction to the parse  */
                    _instruction = this->dropNew();
                    break;

                case KEYWORD_SIGNAL:       /* various forms of SIGNAL           */
                    /* add the instruction to the parse  */
                    _instruction = this->signalNew();
                    break;

                case KEYWORD_CALL:         /* various forms of CALL             */
                    /* add the instruction to the parse  */
                    _instruction = this->callNew();
                    break;

                case KEYWORD_RAISE:        /* RAISE instruction                 */
                    /* add the instruction to the parse  */
                    _instruction = this->raiseNew();
                    break;

                case KEYWORD_ADDRESS:      /* ADDRESS instruction               */
                    /* add the instruction to the parse  */
                    _instruction = this->addressNew();
                    break;

                case KEYWORD_NUMERIC:      /* NUMERIC instruction               */
                    /* add the instruction to the parse  */
                    _instruction = this->numericNew();
                    break;

                case KEYWORD_TRACE:        /* TRACE instruction                 */
                    /* add the instruction to the parse  */
                    _instruction = this->traceNew();
                    break;

                case KEYWORD_DO:           /* all variations of DO instruction  */
                    /* add the instruction to the parse  */
                    _instruction = this->doNew();
                    break;

                case KEYWORD_LOOP:         /* all variations of LOOP instruction  */
                    /* add the instruction to the parse  */
                    _instruction = this->loopNew();
                    break;

                case KEYWORD_EXIT:         /* EXIT instruction                  */
                    /* add the instruction to the parse  */
                    _instruction = this->exitNew();
                    break;

                case KEYWORD_INTERPRET:    /* INTERPRET instruction             */
                    /* add the instruction to the parse  */
                    _instruction = this->interpretNew();
                    break;

                case KEYWORD_PUSH:         /* PUSH instruction                  */
                    /* add the instruction to the parse  */
                    _instruction = this->queueNew(QUEUE_LIFO);
                    break;

                case KEYWORD_QUEUE:        /* QUEUE instruction                 */
                    /* add the instruction to the parse  */
                    _instruction = this->queueNew(QUEUE_FIFO);
                    break;

                case KEYWORD_REPLY:        /* REPLY instruction                 */
                    /* interpreted?                      */
                    if (this->flags&_interpret)
                        syntaxError(Error_Translation_reply_interpret);
                    /* add the instruction to the parse  */
                    _instruction = this->replyNew();
                    break;

                case KEYWORD_RETURN:       /* RETURN instruction                */
                    /* add the instruction to the parse  */
                    _instruction = this->returnNew();
                    break;

                case KEYWORD_IF:           /* IF instruction                    */
                    /* add the instruction to the parse  */
                    _instruction = this->ifNew(KEYWORD_IF);
                    break;

                case KEYWORD_ITERATE:      /* ITERATE instruction               */
                    /* add the instruction to the parse  */
                    _instruction = this->leaveNew(KEYWORD_ITERATE);
                    break;

                case KEYWORD_LEAVE:        /* LEAVE instruction                 */
                    /* add the instruction to the parse  */
                    _instruction = this->leaveNew(KEYWORD_LEAVE);
                    break;

                case KEYWORD_EXPOSE:       /* EXPOSE instruction                */
                    /* interpreted?                      */
                    if (this->flags&_interpret)
                        syntaxError(Error_Translation_expose_interpret);
                    /* add the instruction to the parse  */
                    _instruction = this->exposeNew();
                    break;

                case KEYWORD_FORWARD:      /* FORWARD instruction               */
                    /* interpreted?                      */
                    if (this->flags&_interpret)
                        syntaxError(Error_Translation_forward_interpret);
                    /* add the instruction to the parse  */
                    _instruction = this->forwardNew();
                    break;

                case KEYWORD_PROCEDURE:    /* PROCEDURE instruction             */
                    /* add the instruction to the parse  */
                    _instruction = this->procedureNew();
                    break;

                case KEYWORD_GUARD:        /* GUARD instruction                 */
                    /* interpreted?                      */
                    if (this->flags&_interpret)
                        syntaxError(Error_Translation_guard_interpret);
                    /* add the instruction to the parse  */
                    _instruction = this->guardNew();
                    break;

                case KEYWORD_USE:          /* USE instruction                   */
                    /* interpreted?                      */
                    if (this->flags&_interpret)
                        syntaxError(Error_Translation_use_interpret);
                    /* add the instruction to the parse  */
                    _instruction = this->useNew();
                    break;

                case KEYWORD_ARG:          /* ARG instruction                   */
                    /* add the instruction to the parse  */
                    _instruction = this->parseNew(SUBKEY_ARG);
                    break;

                case KEYWORD_PULL:         /* PULL instruction                  */
                    /* add the instruction to the parse  */
                    _instruction = this->parseNew(SUBKEY_PULL);
                    break;

                case KEYWORD_PARSE:        /* PARSE instruction                 */
                    /* add the instruction to the parse  */
                    _instruction = this->parseNew(KEYWORD_PARSE);
                    break;

                case KEYWORD_SAY:          /* SAY instruction                   */
                    /* add the instruction to the parse  */
                    _instruction = this->sayNew();
                    break;

                case KEYWORD_OPTIONS:      /* OPTIONS instruction               */
                    /* add the instruction to the parse  */
                    _instruction = this->optionsNew();
                    break;

                case KEYWORD_SELECT:       /* SELECT instruction                */
                    /* add the instruction to the parse  */
                    _instruction = this->selectNew();
                    break;

                case KEYWORD_WHEN:         /* WHEN in an SELECT instruction     */
                    /* add the instruction to the parse  */
                    _instruction = this->ifNew(KEYWORD_WHEN);
                    break;

                case KEYWORD_OTHERWISE:    /* OTHERWISE in a SELECT             */
                    /* add the instruction to the parse  */
                    _instruction = this->otherwiseNew(_first);
                    break;

                case KEYWORD_ELSE:         /* unexpected ELSE                   */
                    /* add the instruction to the parse  */
                    _instruction = this->elseNew(_first);
                    break;

                case KEYWORD_END:          /* END for a block construct         */
                    /* add the instruction to the parse  */
                    _instruction = this->endNew();
                    break;

                case KEYWORD_THEN:         /* unexpected THEN                   */
                    /* raise an error                    */
                    syntaxError(Error_Unexpected_then_then);
                    break;

            }
        }
        else
        {                         /* this is a "command" instruction   */
            firstToken();                /* reset to the first token          */
                                         /* process this instruction          */
            _instruction = this->commandNew();
        }
    }
    return _instruction;                 /* return the created instruction    */
}

RexxVariableBase *RexxSource::addVariable(
    RexxString *varname)               /* variable to add                   */
/******************************************************************************/
/* Function:  Resolve a variable name to a single common retriever object     */
/*            per method                                                      */
/******************************************************************************/
{
                                         /* check the directory for an entry  */
    RexxVariableBase *retriever = (RexxVariableBase *)this->variables->fastAt(varname);
    if (retriever == OREF_NULL)          /* not in the table yet?             */
    {
        if (!(this->flags&_interpret))     /* not in an interpret?              */
        {
            this->variableindex++;           /* step the counter                  */
                                             /* create a new variable retriever   */
            retriever = new RexxParseVariable(varname, this->variableindex);
        }
        else                               /* force dynamic lookup each time    */
        {
            retriever = new RexxParseVariable(varname, 0);
        }
        /* add to the variable table         */
        this->variables->put((RexxObject *)retriever, varname);
    }
    /* collecting guard variables?       */
    if (this->guard_variables != OREF_NULL)
    {
        /* in the list of exposed variables? */
        if (this->exposed_variables != OREF_NULL && this->exposed_variables->fastAt(varname) != OREF_NULL)
        {
            /* add this to the guard list        */
            this->guard_variables->put((RexxObject *)retriever, (RexxObject *)retriever);
        }
    }
    return retriever;                    /* return variable accesser          */
}

RexxStemVariable *RexxSource::addStem(
    RexxString *stemName)              /* stem to add                       */
/******************************************************************************/
/* Function:  Process creation of stem variables                              */
/******************************************************************************/
{
    /* check the table for an entry      */
    RexxStemVariable *retriever = (RexxStemVariable *)(this->variables->fastAt(stemName));
    if (retriever == OREF_NULL)          /* not in the table yet?             */
    {
        if (!(this->flags&_interpret))     /* not in an interpret?              */
        {
            this->variableindex++;           /* step the counter                  */
                                             /* create a new variable retriever   */
            retriever = new RexxStemVariable(stemName, this->variableindex);
        }
        else                               /* force dynamic lookup each time    */
        {
            retriever = new RexxStemVariable(stemName, 0);
        }
        /* add to the variable table         */
        this->variables->put((RexxObject *)retriever, stemName);
    }
    /* collecting guard variables?       */
    if (this->guard_variables != OREF_NULL)
    {
        /* in the list of exposed variables? */
        if (this->exposed_variables != OREF_NULL && this->exposed_variables->fastAt(stemName) != OREF_NULL)
        {
            /* add this to the guard list        */
            this->guard_variables->put((RexxObject *)retriever, (RexxObject *)retriever);
        }
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
    RexxString           *stemName;      /* stem part of compound variable    */
    RexxString           *tail;          /* tail section string value         */
    const char *          start;         /* starting scan position            */
    size_t                length;        /* length of tail section            */
    const char *          _position;     /* current position                  */
    const char *          end;           // the end scanning position
    size_t                tailCount;     /* count of tails in compound        */

    length = name->getLength();          /* get the string length             */
    _position = name->getStringData();   /* start scanning at first character */
    start = _position;                   /* save the starting point           */
    end = _position + length;            // save our end marker

    // we know this is a compound, so there must be at least one period.
    /* scan to the first period          */
    while (*_position != '.')
    {
        _position++;                       /* step to the next character        */
    }
    /* get the stem string               */
    stemName = new_string(start, _position - start + 1);
    stemRetriever = this->addStem(stemName); /* get a retriever item for this     */

    tailCount = 0;                       /* no tails yet                      */
    do                                   /* process rest of the variable      */
    {
        // we're here because we just saw a previous period.  that's either the
        // stem variable period or the last tail element we processed.
        // either way, we step past it.  If this period is a trailing one,
        // we'll add a null tail element, which is exactly what we want.
        _position++;                       /* step past previous period         */
        start = _position;                 /* save the start position           */
                                           /* scan for the next period          */
        while (_position < end)
        {
            if (*_position == '.')         // found the next one?
            {
                break;                      // stop scanning now
            }
            _position++;                   // continue looking
        }
        /* extract the tail piece            */
        tail = new_string(start, _position - start);
        /* have a null tail piece or         */
        /* section begin with a digit?       */
        if (!(tail->getLength() == 0 || (*start >= '0' && *start <= '9')))
        {
            /* push onto the term stack          */
            this->subTerms->push((RexxObject *)(this->addVariable(tail)));
        }
        else
        {
            /* just use the string value directly*/
            this->subTerms->push(this->commonString(tail));
        }
        tailCount++;                       /* up the tail count                 */
    } while (_position < end);
    /* finally, create the compound var  */
    return new (tailCount) RexxCompoundVariable(stemName, stemRetriever->index, this->subTerms, tailCount);
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
    /* check the global table first      */
    RexxString *result = (RexxString *)this->strings->fastAt(string);
    /* not in the table                  */
    if (result == OREF_NULL)
    {
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

    RexxString *name = token->value;     /* get the string value for this     */
    switch (token->classId)
    {

        case TOKEN_SYMBOL:                 /* various types of symbols          */
            /* each symbol subtype requires a    */
            /* different retrieval method        */
            switch (token->subclass)
            {

                case SYMBOL_DUMMY:             /* just a dot symbol                 */
                case SYMBOL_CONSTANT:          /* a literal symbol                  */

                    /* see if we've had this before      */
                    retriever = this->literals->fastAt(name);
                    /* first time literal?               */
                    if (retriever == OREF_NULL)
                    {
                        /* can we create an integer object?  */
                        if (token->numeric == INTEGER_CONSTANT)
                        {
                            /* create this as an integer         */
                            value = name->requestInteger(Numerics::DEFAULT_DIGITS);
                            /* conversion error?                 */
                            if (value == TheNilObject)
                            {
                                value = name;          /* just go with the string value     */
                            }
                            else
                                /* snip off the string number string */
                                /* value that was created when the   */
                                /* integer value was created.  This  */
                                /* is rarely used, but contributes   */
                                /* to the saved program size         */
                                name->setNumberString(OREF_NULL);
                        }
                        else
                        {
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
                    /* first time dot variable?          */
                    if (retriever == OREF_NULL)
                    {
                        /* create the shorter name           */
                        value = name->extract(1, name->getLength() - 1);
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
            /* first time literal?               */
            if (retriever == OREF_NULL)
            {
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
    RexxVariableBase *retriever = OREF_NULL; /* created retriever                 */

    /* go validate the symbol            */
    switch (name->isSymbol())
    {

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
            retriever = (RexxVariableBase *)RexxVariableDictionary::buildCompoundVariable(name, true);
            break;

        default:                           /* all other invalid cases           */
            /* have an invalid attribute         */
            syntaxError(Error_Translation_invalid_attribute, name);
    }
    return retriever;                    /* return created retriever          */
}


void RexxSource::addClause(
    RexxInstruction *_instruction)      /* new label to add                  */
/******************************************************************************/
/* Add an instruction to the tree code execution stream                       */
/******************************************************************************/
{
    /* is this the first one?            */
    if (this->first == OREF_NULL)
    {
        /* make this the first one           */
        OrefSet(this, this->first, _instruction);
        /* and the last one                  */
        OrefSet(this, this->last, _instruction);
    }
    /* non-root instruction              */
    else
    {
        this->last->setNext(_instruction);  /* add on to the last instruction    */
        /* this is the new last instruction  */
        OrefSet(this, this->last, _instruction);
    }
    /* now safe from garbage collection  */
    this->toss((RexxObject *)_instruction);
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
    {
        /* add this item                     */
        this->labels->put((RexxObject *)label, labelname);
    }
}


RexxInstruction *RexxSource::findLabel(
    RexxString *labelname)             /* target label                      */
/******************************************************************************/
/* Search the label table for a label name match                              */
/******************************************************************************/
{
    if (this->labels != OREF_NULL)       /* have labels?                      */
    {
        /* just return entry from the table  */
        return(RexxInstruction *)this->labels->fastAt(labelname);
    }
    else
    {
        return OREF_NULL;                  /* don't return anything             */
    }
}

void RexxSource::setGuard()
/******************************************************************************/
/* Function:  Set on guard expression variable "gathering"                    */
/******************************************************************************/
{
    /* just starting to trap?            */
    if (this->guard_variables == OREF_NULL)
    {
        /* create the guard table            */
        OrefSet(this, this->guard_variables, new_identity_table());
    }
}

RexxArray *RexxSource::getGuard()
/******************************************************************************/
/* Function:  Complete guard expression variable collection and return the    */
/*            table of variables.                                             */
/******************************************************************************/
{
    /* convert into an array             */
    RexxArray *guards = this->guard_variables->makeArray();
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
    RexxObject *_expression = OREF_NULL; /* parse expression                  */

    token = nextReal();                  /* get the first token               */
    if (token->isLiteral())              /* literal string expression?        */
    {
        _expression = this->addText(token); /* get the literal retriever         */
    }
    else if (token->isConstant())        /* how about a constant symbol?      */
    {
        _expression = this->addText(token); /* get the literal retriever         */
    }
    /* got an end of expression?         */
    else if (token->isEndOfClause())
    {
        previousToken();                   /* push the token back               */
        return OREF_NULL;                  /* nothing here (may be optional)    */
    }
    /* not a left paren here?            */
    else if (token->classId != TOKEN_LEFT)
    {
        /* this is an invalid expression     */
        syntaxError(Error_Invalid_expression_general, token);
    }
    else
    {
        /* get the subexpression             */
        _expression = this->subExpression(TERM_EOC | TERM_RIGHT);
        second = nextToken();              /* get the terminator token          */
                                           /* not terminated by a right paren?  */
        if (second->classId != TOKEN_RIGHT)
        {
            /* this is an error                  */
            syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
        }
    }
    this->holdObject(_expression);        /* protect the expression            */
    return _expression;                   /* and return it                     */
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
    RexxObject *_expression = OREF_NULL; /* parse expression                  */

    token = nextReal();                  /* get the first token               */
    if (token->isLiteral())              /* literal string expression?        */
    {

        _expression = this->addText(token); /* get the literal retriever         */
    }
    else if (token->isConstant())        /* how about a constant symbol?      */
    {
        _expression = this->addText(token); /* get the literal retriever         */
    }
    /* got an end of expression?         */
    else if (token->isEndOfClause())
    {
        previousToken();                   /* push the token back               */
        return OREF_NULL;                  /* nothing here (may be optional)    */
    }
    /* not a left paren here?            */
    else if (token->classId != TOKEN_LEFT)
    {
        /* this is an invalid expression     */
        syntaxError(Error_Invalid_expression_general, token);
    }
    else
    {
        /* get the subexpression             */
        _expression = this->parseLogical(token, TERM_EOC | TERM_RIGHT);
        second = nextToken();              /* get the terminator token          */
                                           /* not terminated by a right paren?  */
        if (second->classId != TOKEN_RIGHT)
        {
            /* this is an error                  */
            syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
        }
    }
    this->holdObject(_expression);        /* protect the expression            */
    return _expression;                   /* and return it                     */
}

RexxObject *RexxSource::parenExpression(RexxToken *start)
/******************************************************************************/
/* Function:  Evaluate a "parenthetical" expression for REXX instruction      */
/*            values.  A parenthetical expression is an expression enclosed   */
/*            in parentheses.                                                 */
/******************************************************************************/
{
  // NB, the opening paren has already been parsed off

  RexxObject *_expression = this->subExpression(TERM_EOC | TERM_RIGHT);
  RexxToken *second = nextToken();   /* get the terminator token          */
                                     /* not terminated by a right paren?  */
  if (second->classId != TOKEN_RIGHT)
  {
      syntaxErrorAt(Error_Unmatched_parenthesis_paren, start);
  }
                                     /* this is an error                  */
  this->holdObject(_expression);        /* protect the expression            */
  return _expression;                   /* and return it                     */
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
    SourceLocation location;             /* token location info               */

                                         /* get the left term                 */
    left = this->messageSubterm(terminators);
    if (left == OREF_NULL)               /* end of the expression?            */
    {
        return OREF_NULL;                  /* done processing here              */
    }
    this->pushTerm(left);                /* add the term to the term stack    */
                                         /* add a fence item to operator stack*/
    this->pushOperator((RexxToken *)TheNilObject);
    token = nextToken();                 /* get the next token                */
                                         /* loop until end of expression      */
    while (!this->terminator(terminators, token))
    {
        switch (token->classId)
        {

            case  TOKEN_TILDE:               /* have a message send operation     */
            case  TOKEN_DTILDE:              /* have a double twiddle operation   */
                left = this->popTerm();        /* get the left term from the stack  */
                if (left == OREF_NULL)         /* not there?                        */
                {
                    /* this is an invalid expression     */
                    syntaxError(Error_Invalid_expression_general, token);
                }
                /* process a message term            */
                subexpression = this->message(left, token->classId == TOKEN_DTILDE, terminators);
                this->pushTerm(subexpression); /* push this back on the term stack  */
                break;

            case  TOKEN_SQLEFT:              /* collection syntax message         */
                left = this->popTerm();        /* get the left term from the stack  */
                if (left == OREF_NULL)         /* not there?                        */
                {
                    /* this is an invalid expression     */
                    syntaxError(Error_Invalid_expression_general, token);
                }
                /* process a message term            */
                subexpression = this->collectionMessage(token, left, terminators);
                this->pushTerm(subexpression); /* push this back on the term stack  */
                break;

            case  TOKEN_SYMBOL:              /* Symbol in the expression          */
            case  TOKEN_LITERAL:             /* Literal in the expression         */
            case  TOKEN_LEFT:                /* start of subexpression            */

                location = token->getLocation(); /* get the token start position      */
                                                 /* abuttal ends on the same line     */
                location.setEnd(location.getLineNumber(), location.getOffset());
                /* This is actually an abuttal       */
                token = new RexxToken (TOKEN_OPERATOR, OPERATOR_ABUTTAL, OREF_NULLSTRING, location);
                previousToken();               /* step back on the token list       */

            case  TOKEN_BLANK:               /* possible blank concatenate        */
                second = nextReal();           /* get the next token                */
                                               /* blank prior to a terminator?      */
                if (this->terminator(terminators, second))
                {
                    break;                       /* not a real operator               */
                }
                else                           /* have a blank operator             */
                {
                    previousToken();             /* push this back                    */
                }
                                                 /* fall through to operator logic    */

            case  TOKEN_OPERATOR:            /* have a dyadic operator            */
                /* actually a prefix only one?       */
                if (token->subclass == OPERATOR_BACKSLASH)
                {
                    /* this is an invalid expression     */
                    syntaxError(Error_Invalid_expression_general, token);
                }
                /* handle operator precedence        */
                for (;;)
                {
                    second = this->topOperator();/* get the top term                  */
                                                 /* hit the fence term?               */
                    if (second == (RexxToken *)TheNilObject)
                    {
                        break;                     /* out of here                       */
                    }
                                                   /* current have higher precedence?   */
                    if (this->precedence(token) > this->precedence(second))
                    {
                        break;                     /* finished also                     */
                    }
                    right = this->popTerm();     /* get the right term                */
                    left = this->popTerm();      /* and the left term                 */
                                                 /* not enough terms?                 */
                    if (right == OREF_NULL || left == OREF_NULL)
                    {
                        /* this is an invalid expression     */
                        syntaxError(Error_Invalid_expression_general, token);
                    }
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
                {
                    /* have a bad expression             */
                    syntaxError(Error_Invalid_expression_general, token);
                }
                this->pushTerm(right);         /* add the term to the term stack    */
                break;

            case TOKEN_ASSIGNMENT:
                // special assignment token in a bad context.  We report this as an error.
                /* this is an invalid expression     */
                syntaxError(Error_Invalid_expression_general, token);
                break;

            case TOKEN_COMMA:                /* found a comma in the expression   */
                /* should have been trapped as an    */
                /* expression terminator, so this is */
                /* not a valid expression            */
                syntaxError(Error_Unexpected_comma_comma);
                break;

            case TOKEN_RIGHT:                /* found a paren in the expression   */
                syntaxError(Error_Unexpected_comma_paren);
                break;

            case TOKEN_SQRIGHT:              /* found a bracket in the expression */
                syntaxError(Error_Unexpected_comma_bracket);
                break;

            default:                         /* something unexpected              */
                /* not a valid expression            */
                syntaxError(Error_Invalid_expression_general, token);
                break;
        }
        token = nextToken();               /* get the next token                */
    }
    token= this->popOperator();          /* get top operator token            */
                                         /* process pending operations        */
    while (token != (RexxToken *)TheNilObject)
    {
        right = this->popTerm();           /* get the right term                */
        left = this->popTerm();            /* now get the left term             */
                                           /* missing any terms?                */
        if (left == OREF_NULL || right == OREF_NULL)
        {
            /* this is an invalid expression     */
            syntaxError(Error_Invalid_expression_general, token);
        }
        /* create a new operation            */
        subexpression = (RexxObject *)new RexxBinaryOperator(token->subclass, left, right);
        this->pushTerm(subexpression);     /* push this back on the term stack  */
        token = this->popOperator();       /* get top operator token            */
    }
    return this->popTerm();              /* expression is top of term stack   */
}

RexxArray *RexxSource::argArray(
  RexxToken   *_first,                 /* token starting arglist            */
  int          terminators )           /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off an array of argument expressions                      */
/******************************************************************************/
{
    size_t     argCount;                 /* count of arguments                */
    RexxArray *_argArray;                 /* returned array                    */

    /* scan off the argument list        */
    argCount = this->argList(_first, terminators);
    _argArray = new_array(argCount);      /* get a new argument list           */
    /* now copy the argument pointers    */
    while (argCount > 0)
    {
        /* in reverse order                  */
        _argArray->put(this->subTerms->pop(), argCount--);
    }
    return _argArray;                     /* return the argument array         */
}

size_t RexxSource::argList(
  RexxToken   *_first,                  /* token starting arglist            */
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
    /* loop until get a full terminator  */
    for (;;)
    {
        /* parse off next argument expression*/
        subexpr = this->subExpression(terminators | TERM_COMMA);
        arglist->push(subexpr);            /* add next argument to list         */
        this->pushTerm(subexpr);           /* add the term to the term stack    */
        total++;                           /* increment the total               */
        if (subexpr != OREF_NULL)          /* real expression?                  */
        {
            realcount = total;               /* update the real count             */
        }
        token = nextToken();               /* get the next token                */
        if (token->classId != TOKEN_COMMA) /* start of next argument?           */
        {
            break;                           /* no, all finished                  */
        }
    }
    /* not closed with expected ')'?     */
    if (terminators & TERM_RIGHT && token->classId != TOKEN_RIGHT)
    {
        /* raise an error                    */
        syntaxErrorAt(Error_Unmatched_parenthesis_paren, _first);
    }
    /* not closed with expected ']'?     */
    if (terminators&TERM_SQRIGHT && token->classId != TOKEN_SQRIGHT)
    {
        /* have an unmatched bracket         */
        syntaxErrorAt(Error_Unmatched_parenthesis_square, _first);
    }
    this->popNTerms(total);              /* pop all items off the term stack  */
    /* pop off any trailing omitteds     */
    while (total > realcount)
    {
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
  RexxExpressionFunction *_function;    /* newly created function argument   */

  saveObject((RexxObject *)name);      // protect while parsing the argument list

                                       /* process the argument list         */
  argCount = this->argList(token, ((terminators | TERM_RIGHT) & ~TERM_SQRIGHT));

                                       /* create a new function item        */
  _function = new (argCount) RexxExpressionFunction(name->value, argCount, this->subTerms, this->resolveBuiltin(name->value), name->isLiteral());
                                       /* add to table of references        */
  this->addReference((RexxObject *)_function);
  removeObj((RexxObject *)name);       // end of protected windoww.
  return (RexxObject *)_function;      /* and return this to the caller     */
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
  RexxObject *_message;                /* new message term                  */

  this->saveObject((RexxObject *)target);   /* save target until it gets connected to message */
                                       /* process the argument list         */
  argCount = this->argList(token, ((terminators | TERM_SQRIGHT) & ~TERM_RIGHT));
                                       /* create a new function item        */
  _message = (RexxObject *)new (argCount) RexxExpressionMessage(target, (RexxString *)OREF_BRACKETS, (RexxObject *)OREF_NULL, argCount, this->subTerms, false);
  this->holdObject(_message);          /* hold this here for a while        */
  this->removeObj((RexxObject *)target);   /* target is now connected to message, remove from savelist without hold */
  return _message;                     /* return the message item           */
}

RexxToken  *RexxSource::getToken(
    int   terminators,                 /* expression termination context    */
    int   errorcode)                   /* expected error code               */
/******************************************************************************/
/* Function:  Get a token, checking to see if this is a terminatore token     */
/******************************************************************************/
{
    RexxToken *token = nextToken();                 /* get the next token                */
    /* this a terminator token?          */
    if (this->terminator(terminators, token))
    {
        if (errorcode != 0)                /* want an error raised?             */
        {
            syntaxError(errorcode);         /* report this                       */
        }
        return OREF_NULL;                  /* just return a null                */
    }
    return token;                        /* return the token                  */
}

RexxObject *RexxSource::message(
  RexxObject  *target,                 /* message send target               */
  bool         doubleTilde,            /* class of message send             */
  int          terminators )           /* expression termination context    */
/******************************************************************************/
/* Function:  Parse a full message send expression term                       */
/******************************************************************************/
{
    size_t        argCount;              /* list of function arguments        */
    RexxString   *messagename = OREF_NULL;  /* message name                      */
    RexxObject   *super;                 /* super class target                */
    RexxToken    *token;                 /* current working token             */
    RexxExpressionMessage *_message;     /* new message term                  */

    super = OREF_NULL;                   /* default no super class            */
    argCount = 0;                        /* and no arguments                  */
    this->saveObject(target);   /* save target until it gets connected to message */

    /* add the term to the term stack so that the calculations */
    /* include this in the processing. */
    this->pushTerm(target);
    /* get the next token                */
    token = this->getToken(terminators, Error_Symbol_or_string_tilde);
    /* unexpected type?                  */
    if (token->isSymbolOrLiteral())
    {
        messagename = token->value;        /* get the message name              */
    }
    else
    {
        /* error!                            */
        syntaxError(Error_Symbol_or_string_tilde);
    }
    /* get the next token                */
    token = this->getToken(terminators, 0);
    if (token != OREF_NULL)
    {            /* not reached the clause end?       */
                 /* have a super class?               */
        if (token->classId == TOKEN_COLON)
        {
            /* get the next token                */
            token = this->getToken(terminators, Error_Symbol_expected_colon);
            /* not a variable symbol?            */
            if (!token->isVariable() && token->subclass != SYMBOL_DOTSYMBOL)
            {
                /* have an error                     */
                syntaxError(Error_Symbol_expected_colon);
            }
            super = this->addText(token);    /* get the variable retriever        */
                                             /* get the next token                */
            token = this->getToken(terminators, 0);
        }
    }
    if (token != OREF_NULL)
    {            /* not reached the clause end?       */
        if (token->classId == TOKEN_LEFT)  /* have an argument list?            */
        {
            /* process the argument list         */
            argCount = this->argList(token, ((terminators | TERM_RIGHT) & ~TERM_SQRIGHT));
        }
        else
        {
            previousToken();                 /* something else, step back         */
        }
    }

    this->popTerm();                     /* it is now safe to pop the message target */
                                         /* create a message send node        */
    _message =  new (argCount) RexxExpressionMessage(target, messagename, super, argCount, this->subTerms, doubleTilde);
    /* protect for a bit                 */
    this->holdObject((RexxObject *)_message);
    this->removeObj(target);   /* target is now connected to message, remove from savelist without hold */
    return(RexxObject *)_message;        /* return the message item           */
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
        RexxToken *_first = nextReal();
        if (_first->isSymbol())
        {
            // ok, add the variable to the processing list
            this->needVariable(_first);
            result = this->addText(_first);
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
    int          classId;                /* token class                       */

    size_t mark = markPosition();       // save the current position so we can reset cleanly

    start = this->subTerm(TERM_EOC);     /* get the first term of instruction */
    this->holdObject(start);             /* save the starting term            */
    term = OREF_NULL;                    /* default to no term                */
    token = nextToken();                 /* get the next token                */
    classId = token->classId;            /* get the token class               */
                                         /* while cascading message sends     */
    while (classId == TOKEN_TILDE || classId == TOKEN_DTILDE || classId == TOKEN_SQLEFT )
    {
        if (classId == TOKEN_SQLEFT)       /* left bracket form?                */
        {
            term = this->collectionMessage(token, start, TERM_EOC);
        }
        else
        {
            /* process a message term            */
            term = this->message(start, classId == TOKEN_DTILDE, TERM_EOC);
        }
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
    RexxObject  *term = OREF_NULL;       /* working term                      */
    int          classId;                /* token class                       */

    token = nextToken();                 /* get the next token                */
                                         /* this the expression end?          */
    if (this->terminator(terminators, token))
    {
        return OREF_NULL;                  /* nothing to do here                */
    }
                                           /* have potential prefix operator?   */
    if (token->classId == TOKEN_OPERATOR)
    {

        /* handle prefix operators as terms  */
        switch (token->subclass)
        {

            case OPERATOR_PLUS:              /* prefix plus                       */
            case OPERATOR_SUBTRACT:          /* prefix minus                      */
            case OPERATOR_BACKSLASH:         /* prefix backslash                  */
                /* handle following term             */
                term = this->messageSubterm(terminators);
                if (term == OREF_NULL)         /* nothing found?                    */
                {
                    /* this is an error                  */
                    syntaxError(Error_Invalid_expression_prefix, token);
                }
                /* create the new operator term      */
                term = (RexxObject *)new RexxUnaryOperator(token->subclass, term);
                break;

            default:                         /* other operators not allowed here  */
                /* this is an error                  */
                syntaxError(Error_Invalid_expression_general, token);
        }
    }
    /* non-prefix operator code          */
    else
    {
        previousToken();                   /* put back the first token          */
        term = this->subTerm(TERM_EOC);    /* get the first term of instruction */
        this->holdObject(term);            /* save the starting term            */
        token = nextToken();               /* get the next token                */
        classId = token->classId;          /* get the token class               */
                                           /* while cascading message sends     */
        while (classId == TOKEN_TILDE || classId == TOKEN_DTILDE || classId == TOKEN_SQLEFT )
        {
            if (classId == TOKEN_SQLEFT)     /* left bracket form?                */
            {
                term = this->collectionMessage(token, term, TERM_EOC);
            }
            else
            {
                /* process a message term            */
                term = this->message(term, classId == TOKEN_DTILDE, TERM_EOC);
            }
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
    RexxObject   *term = OREF_NULL;      /* parsed out term                   */
    RexxToken    *second;                /* second token of term              */

    token = nextToken();                 /* get the next token                */
                                         /* this the expression end?          */
    if (this->terminator(terminators, token))
    {
        return OREF_NULL;                  /* nothing to do here                */
    }

    switch (token->classId)
    {

        case  TOKEN_LEFT:                  /* have a left parentheses           */
            /* get the subexpression             */
            term = this->subExpression(((terminators | TERM_RIGHT) & ~TERM_SQRIGHT));
            if (term == OREF_NULL)           /* nothing found?                    */
            {
                /* this is an error                  */
                syntaxError(Error_Invalid_expression_general, token);
            }
            second = nextToken();            /* get the terminator token          */
                                             /* not terminated by a right paren?  */
            if (second->classId != TOKEN_RIGHT)
            {
                /* this is an error                  */
                syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
            }
            break;

        case  TOKEN_SYMBOL:                /* Symbol in the expression          */
        case  TOKEN_LITERAL:               /* Literal in the expression         */
            second = nextToken();            /* get the next token                */
                                             /* have a function call?             */
            if (second->classId == TOKEN_LEFT)
            {
                /* process the function call         */
                term = this->function(second, token, terminators);
            }
            else
            {
                previousToken();               /* push the token back               */
                term = this->addText(token);   /* variable or literal access        */
            }
            break;

        case  TOKEN_RIGHT:                 /* have a right parentheses          */
            /* this is an error here             */
            syntaxError(Error_Unexpected_comma_paren);
            break;

        case  TOKEN_COMMA:                 /* have a comma                      */
            /* this is an error here             */
            syntaxError(Error_Unexpected_comma_comma);
            break;

        case  TOKEN_SQRIGHT:               /* have a right square bracket       */
            /* this is an error here             */
            syntaxError(Error_Unexpected_comma_bracket);
            break;

        case  TOKEN_OPERATOR:              /* operator token                    */
            switch (token->subclass)
            {       /* handle prefix operators as terms  */

                case OPERATOR_PLUS:            /* prefix plus                       */
                case OPERATOR_SUBTRACT:        /* prefix minus                      */
                case OPERATOR_BACKSLASH:       /* prefix backslash                  */
                    previousToken();             /* put the token back                */
                    return OREF_NULL;            /* just return null (processed later)*/

                default:                       /* other operators not allowed here  */
                    /* this is an error                  */
                    syntaxError(Error_Invalid_expression_general, token);
            }
            break;

        default:                           /* unknown thing in expression       */
            /* this is an error                  */
            syntaxError(Error_Invalid_expression_general, token);
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
    {
        /* make it the highest point         */
        this->maxstack = this->currentstack;
    }
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
    RexxObject *result = OREF_NULL;                  /* final popped element              */

    this->currentstack -= count;         /* reduce the size count             */
    while (count--)                      /* while more to remove              */
    {
        result = this->terms->pop();       /* pop the next item               */
    }
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
    {
        /* give the interpret error          */
        syntaxError(Error_Translation_expose_interpret);
    }
    /* not the first instruction?        */
    if (this->last->getType() != KEYWORD_FIRST)
    {
        /* general placement error           */
        syntaxError(Error_Translation_expose);
    }
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
    for (i = 3, word = (RexxString *)(string->word(IntegerTwo)); word->getLength() != 0; i++)
    {
        count++;                           /* have another word                 */
        word = this->commonString(word);   /* get the common version of this    */
        wordlist->push(word);              /* add this word to the list         */
                                           /* get the next word                 */
        word = (RexxString *)string->word(new_integer(i));
    }
    wordarray = new_array(count);        /* get an array return value         */
    while (count > 0)                    /* while more words                  */
    {
        /* copy into the array               */
        wordarray->put(wordlist->pop(), count--);
    }
    return wordarray;                    /* return as an array                */
}

void RexxSource::errorCleanup()
/******************************************************************************/
/* Function:  Free up all of the parsing elements because of an error         */
/******************************************************************************/
{
  this->cleanup();                     /* do needed cleanup                 */
}

void RexxSource::error(int errorcode)
/******************************************************************************/
/* Function:  Raise an error caused by source translation problems.           */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, OREF_NULL, OREF_NULL);
}

void RexxSource::error(int errorcode, SourceLocation &location, RexxArray *subs)
/******************************************************************************/
/* Function:  Raise an error caused by source translation problems.           */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
  clauseLocation = location;           // set the error location
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, subs, OREF_NULL);
}

void RexxSource::errorLine(
     int   errorcode,                  /* error to raise                    */
     RexxInstruction *_instruction)    /* instruction for the line number   */
/******************************************************************************/
/* Function:  Raise an error where one of the error message substitutions is  */
/*            the line number of another instruction object                   */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(new_integer(_instruction->getLineNumber())), OREF_NULL);
}

void RexxSource::errorPosition(
     int        errorcode,             /* error to raise                    */
     RexxToken *token )                /* token value for description       */
/******************************************************************************/
/* Function:  Raise an error, displaying the location of a token associated   */
/*            with the error.                                                 */
/******************************************************************************/
{
  SourceLocation token_location = token->getLocation(); /* get the token location            */
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(new_integer(token_location.getOffset()), new_integer(token_location.getLineNumber())), OREF_NULL);
}

void RexxSource::errorToken(
     int        errorcode,             /* error to raise                    */
     RexxToken *token )                /* token value for description       */
/******************************************************************************/
/* Function:  Raise an error, displaying the value of a token in the error    */
/*            message.                                                        */
/******************************************************************************/
{
    RexxString *value = token->value;                /* get the token value               */
    if (value == OREF_NULL)
    {
        switch (token->classId)
        {

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
    ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(value), OREF_NULL);
}

void RexxSource::error(
     int         errorcode,            /* error to raise                    */
     RexxObject *value )               /* value for description             */
/******************************************************************************/
/* Function:  Issue an error message with a single substitution parameter.    */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(value), OREF_NULL);
}

void RexxSource::error(
     int         errorcode,            /* error to raise                    */
     RexxObject *value1,               /* first value for description       */
     RexxObject *value2 )              /* second value for description      */
/******************************************************************************/
/* Function:  Issue an error message with two substitution parameters.        */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(value1, value2), OREF_NULL);
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
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(value1, value2, value3), OREF_NULL);
}

void RexxSource::blockError(
    RexxInstruction *_instruction )     /* unclosed control instruction      */
/******************************************************************************/
/* Function:  Raise an error for an unclosed block instruction.               */
/******************************************************************************/
{
    // get the instruction location and set as the current error location
    clauseLocation = this->last->getLocation();

    switch (_instruction->getType())
    {   /* issue proper message type         */
        case KEYWORD_DO:                   /* incomplete DO                     */
            /* raise an error                    */
            syntaxError(Error_Incomplete_do_do, _instruction);
            break;

        case KEYWORD_SELECT:               /* incomplete SELECT                 */
            syntaxError(Error_Incomplete_do_select, _instruction);
            break;

        case KEYWORD_OTHERWISE:            /* incomplete SELECT                 */
            syntaxError(Error_Incomplete_do_otherwise, _instruction);
            break;

        case KEYWORD_IF:                   /* incomplete IF                     */
        case KEYWORD_IFTHEN:               /* incomplete IF                     */
        case KEYWORD_WHENTHEN:             /* incomplete IF                     */
            syntaxError(Error_Incomplete_do_then, _instruction);
            break;

        case KEYWORD_ELSE:                 /* incomplete ELSE                   */
            syntaxError(Error_Incomplete_do_else, _instruction);
            break;
    }
}

void *RexxSource::operator new (size_t size)
/******************************************************************************/
/* Function:  Create a new translator object from an array                    */
/******************************************************************************/
{
    /* Get new object                    */
    return new_object(sizeof(RexxSource), T_RexxSource);
}


RexxInstruction *RexxSource::sourceNewObject(
    size_t        size,                /* Object size                       */
    RexxBehaviour *_behaviour,         /* Object's behaviour                */
    int            type )              /* Type of instruction               */
/******************************************************************************/
/* Function:  Create a "raw" translator instruction object                    */
/******************************************************************************/
{
  RexxObject *newObject = new_object(size);        /* Get new object                    */
  newObject->setBehaviour(_behaviour); /* Give new object its behaviour     */
                                       /* do common initialization          */
  new ((void *)newObject) RexxInstruction (this->clause, type);
                                       /* now protect this                  */
  OrefSet(this, this->currentInstruction, (RexxInstruction *)newObject);
  return (RexxInstruction *)newObject; /* return the new object             */
}

/**
 * Parse a trace setting value into a decoded setting
 * and the RexxActivation debug flag set to allow
 * new trace settings to be processed more quickly.
 *
 * @param value      The string source of the trace setting.
 * @param newSetting The returned setting in binary form.
 * @param debugFlags The debug flag representation of the trace setting.
 */
bool RexxSource::parseTraceSetting(RexxString *value, size_t &newSetting, size_t &debugFlags, char &badOption)
{
    size_t setting = TRACE_IGNORE;       /* don't change trace setting yet    */
    size_t debug = DEBUG_IGNORE;         /* and the default debug change      */

    size_t length = value->getLength();  /* get the string length             */
    /* null string?                      */
    if (length == 0)
    {
        setting = TRACE_NORMAL;           /* use default trace setting         */
        debug = DEBUG_OFF;                /* turn off debug mode               */
    }
    else
    {
        /* start at the beginning            */
        /* while more length to process      */
        /* step one each character           */
        for (size_t _position = 0; _position < length; _position++)
        {

            /* process the next character        */
            switch (value->getChar(_position))
            {

                case '?':                      /* debug toggle character            */
                    /* already toggling?                 */
                    if (debug == DEBUG_TOGGLE)
                    {
                        debug = DEBUG_IGNORE;     /* this is back to no change at all  */
                    }
                    else
                    {
                        debug = DEBUG_TOGGLE;     /* need to toggle the debug mode     */
                    }
                    continue;                    /* go loop again                     */

                case 'a':                      /* TRACE ALL                         */
                case 'A':
                    setting = TRACE_ALL;
                    break;

                case 'c':                      /* TRACE COMMANDS                    */
                case 'C':
                    setting = TRACE_COMMANDS;
                    break;

                case 'l':                      /* TRACE LABELS                      */
                case 'L':
                    setting = TRACE_LABELS;
                    break;

                case 'e':                      /* TRACE ERRORS                      */
                case 'E':
                    setting = TRACE_ERRORS;
                    break;

                case 'f':                      /* TRACE FAILURES                    */
                case 'F':
                    setting = TRACE_FAILURES;
                    break;

                case 'n':                      /* TRACE NORMAL                      */
                case 'N':
                    setting = TRACE_NORMAL;
                    break;

                case 'o':                      /* TRACE OFF                         */
                case 'O':
                    setting = TRACE_OFF;
                    break;

                case 'r':                      /* TRACE RESULTS                     */
                case 'R':
                    setting = TRACE_RESULTS;
                    break;

                case 'i':                      /* TRACE INTERMEDIATES               */
                case 'I':
                    setting = TRACE_INTERMEDIATES;
                    break;

                default:                       /* unknown trace setting             */
                    // each context handles it's own error reporting, so give back the
                    // information needed for the message.
                    badOption = value->getChar(_position);
                    return false;
                    break;
            }
            break;                           /* non-prefix char found             */
        }
    }
    // return the merged setting
    newSetting = setting | debug;
    // create the activation-specific flags
    debugFlags = RexxActivation::processTraceSetting(newSetting);
    return true;
}


/**
 * Format an encoded trace setting back into human readable form.
 *
 * @param setting The source setting.
 *
 * @return The string representation of the trace setting.
 */
RexxString * RexxSource::formatTraceSetting(size_t source)
{
    char         setting[3];             /* returned trace setting            */
    setting[0] = '\0';                   /* start with a null string          */
                                         /* debug mode?                       */
    if (source & DEBUG_ON)
    {
        setting[0] = '?';                  /* add the question mark             */
                                           /* add current trace option          */
        setting[1] = (char)source&TRACE_SETTING_MASK;
        /* create a string form              */
        return new_string(setting, 2);
    }
    else                                 /* no debug prefix                   */
    {
        /* add current trace option          */
        setting[0] = (char)source&TRACE_SETTING_MASK;
        /* create a string form              */
        return new_string(setting, 1);
    }
}

size_t RexxSource::processVariableList(
  int        type )                    /* type of instruction               */
/****************************************************************************/
/* Function:  Process a variable list for PROCEDURE, DROP, and USE          */
/****************************************************************************/
{
    RexxToken   *token;                  /* current working token             */
    int          list_count;             /* count of variables in list        */
    RexxObject  *retriever;              /* variable retriever object         */

    list_count = 0;                      /* no variables yet                  */
    token = nextReal();                  /* get the first variable            */

    /* while not at the end of the clause*/
    while (!token->isEndOfClause())
    {
        /* have a variable name?             */
        if (token->isSymbol())
        {
            /* non-variable symbol?              */
            if (token->subclass == SYMBOL_CONSTANT)
            {
                /* report the error                  */
                syntaxError(Error_Invalid_variable_number, token);
            }
            else if (token->subclass == SYMBOL_DUMMY)
            {
                /* report the error                  */
                syntaxError(Error_Invalid_variable_period, token);
            }
            retriever = this->addText(token);/* get a retriever for this          */
            this->subTerms->push(retriever); /* add to the variable list          */
            if (type == KEYWORD_EXPOSE)      /* this an expose operation?         */
            {
                this->expose(token->value);    /* add to the expose list too        */
            }
            list_count++;                    /* record the variable               */
        }
        /* have a variable reference         */
        else if (token->classId == TOKEN_LEFT)
        {
            list_count++;                    /* record the variable               */
            token = nextReal();              /* get the next token                */
                                             /* not a symbol?                     */
            if (!token->isSymbol())
            {
                /* must be a symbol here             */
                syntaxError(Error_Symbol_expected_varref);
            }
            /* non-variable symbol?              */
            if (token->subclass == SYMBOL_CONSTANT)
            {
                /* report the error                  */
                syntaxError(Error_Invalid_variable_number, token);
            }
            else if (token->subclass == SYMBOL_DUMMY)
            {
                /* report the error                  */
                syntaxError(Error_Invalid_variable_period, token);
            }

            retriever = this->addText(token);/* get a retriever for this          */
                                             /* make this an indirect reference   */
            retriever = (RexxObject *)new RexxVariableReference((RexxVariableBase *)retriever);
            this->subTerms->queue(retriever);/* add to the variable list          */
            this->currentstack++;            /* account for the varlists          */

            token = nextReal();              /* get the next token                */
            if (token->isEndOfClause()) /* nothing following?                */
            {
                /* report the missing paren          */
                syntaxError(Error_Variable_reference_missing);
            }
            /* must be a right paren here        */
            else if (token->classId != TOKEN_RIGHT)
            {
                /* this is an error                  */
                syntaxError(Error_Variable_reference_extra, token);
            }
        }
        /* something bad....                 */
        else
        {                             /* this is invalid                   */
            if (type == KEYWORD_DROP)        /* DROP form?                        */
            {
                /* give appropriate message          */
                syntaxError(Error_Symbol_expected_drop);
            }
            else                             /* else give message for EXPOSEs     */
            {
                syntaxError(Error_Symbol_expected_expose);
            }
        }
        token = nextReal();                /* get the next variable             */
    }
    if (list_count == 0)
    {               /* no variables?                     */
        if (type == KEYWORD_DROP)          /* DROP form?                        */
        {
            /* give appropriate message          */
            syntaxError(Error_Symbol_expected_drop);
        }
        else                               /* else give message for EXPOSEs     */
        {
            syntaxError(Error_Symbol_expected_expose);
        }
    }
    return list_count;                   /* return the count                  */
}

RexxObject *RexxSource::parseConditional(
     int   *condition_type,            /* type of condition                 */
     int    error_message )            /* extra "stuff" error message       */
/******************************************************************************/
/* Function:  Allow for WHILE or UNTIL keywords following some other looping  */
/*            construct.  This returns SUBKEY_WHILE or SUBKEY_UNTIL to flag   */
/*            the caller that a conditional has been used.                    */
/******************************************************************************/
{
    RexxToken  *token;                   /* current working token             */
    int         _keyword;                /* keyword of parsed conditional     */
    RexxObject *_condition;              /* parsed out condition              */

    _condition = OREF_NULL;               /* default to no condition           */
    _keyword = 0;                         /* no conditional yet                */
    token = nextReal();                  /* get the terminator token          */

    /* real end of instruction?          */
    if (!token->isEndOfClause())
    {
        /* may have WHILE/UNTIL              */
        if (token->isSymbol())
        {
            /* process the symbol                */
            switch (this->subKeyword(token) )
            {

                case SUBKEY_WHILE:              /* DO WHILE exprw                    */
                    /* get next subexpression            */
                    _condition = this->parseLogical(OREF_NULL, TERM_COND);
                    if (_condition == OREF_NULL) /* nothing really there?             */
                    {
                        /* another invalid DO                */
                        syntaxError(Error_Invalid_expression_while);
                    }
                    token = nextToken();          /* get the terminator token          */
                                                  /* must be end of instruction        */
                    if (!token->isEndOfClause())
                    {
                        syntaxError(Error_Invalid_do_whileuntil);
                    }
                    _keyword = SUBKEY_WHILE;       /* this is the WHILE form            */
                    break;

                case SUBKEY_UNTIL:              /* DO UNTIL expru                    */
                    /* get next subexpression            */
                    /* get next subexpression            */
                    _condition = this->parseLogical(OREF_NULL, TERM_COND);

                    if (_condition == OREF_NULL)   /* nothing really there?             */
                    {
                        /* another invalid DO                */
                        syntaxError(Error_Invalid_expression_until);
                    }
                    token = nextToken();          /* get the terminator token          */
                                                  /* must be end of instruction        */
                    if (!token->isEndOfClause())
                    {
                        syntaxError(Error_Invalid_do_whileuntil);
                    }
                    _keyword = SUBKEY_UNTIL;       /* this is the UNTIL form            */
                    break;

                default:                        /* nothing else is valid here!       */
                    /* raise an error                    */
                    syntaxError(error_message, token);
                    break;
            }
        }
    }
    if (condition_type != NULL)          /* need the condition type?          */
    {
        *condition_type = _keyword;        /* set the keyword                   */
    }
    return _condition;                   /* return the condition expression   */
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
RexxObject *RexxSource::parseLogical(RexxToken *_first, int terminators)
{
    size_t count = argList(_first, terminators);
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


/**
 * Load a ::REQUIRES directive when the source file is first
 * invoked.
 *
 * @param target The name of the ::REQUIRES
 * @param instruction
 *               The directive instruction being processed.
 */
PackageClass *RexxSource::loadRequires(RexxActivity *activity, RexxString *target)
{
    // we need the instance this is associated with
    InterpreterInstance *instance = activity->getInstance();

    // get a fully resolved name for this....we might locate this under either name, but the
    // fully resolved name is generated from this source file context.
    RexxString *fullName = resolveProgramName(activity, target);
    ProtectedObject p(fullName);

    // if we've already loaded this in this instance, just return it.
    PackageClass *packageInstance = instance->loadRequires(activity, target, fullName);

    if (packageInstance == OREF_NULL)       /* couldn't create this?             */
    {
        /* report an error                   */
        reportException(Error_Routine_not_found_requires, target);
    }
    // add this to the source context
    addPackage(packageInstance);
    return packageInstance;
}


/**
 * Load a ::REQUIRES directive from an provided source target
 *
 * @param target The name of the ::REQUIRES
 */
PackageClass *RexxSource::loadRequires(RexxActivity *activity, RexxString *target, RexxArray *s)
{
    // we need the instance this is associated with
    InterpreterInstance *instance = activity->getInstance();

    // if we've already loaded this in this instance, just return it.
    PackageClass *packageInstance = instance->loadRequires(activity, target, s);

    if (packageInstance == OREF_NULL)             /* couldn't create this?             */
    {
        /* report an error                   */
        reportException(Error_Routine_not_found_requires, target);
    }
    // add this to the source context
    addPackage(packageInstance);
    return packageInstance;
}


/**
 * Add a package to a source file context.  This allows new
 * packages to be imported into a source.
 *
 * @param p
 */
void RexxSource::addPackage(PackageClass *p)
{
    // force the directives to be processed first
    install();
    // we only create this on the first use
    if (loadedPackages == OREF_NULL)
    {
        loadedPackages = new_list();
    }
    else
    {
        // we only add a given package item once.
        if (loadedPackages->hasItem(p) == TheTrueObject)
        {
            return;
        }
    }

    // add this to the list and merge the information
    loadedPackages->append(p);
    // not merge all of the info from the imported package
    mergeRequired(p->getSourceObject());
}


/**
 * Retrieve the package wrapper associated with this source.
 *
 * @return The package instance that fronts for this source in Rexx code.
 */
PackageClass *RexxSource::getPackage()
{
    if (package == OREF_NULL)
    {
        OrefSet(this, this->package, new PackageClass(this));
    }
    return package;
}


/**
 * Add an installed class to this source package
 *
 * @param name   The class name
 * @param classObject
 *               The class object
 * @param publicClass
 *               Indicates whether this needs to be added to the public list as well.
 */
void RexxSource::addInstalledClass(RexxString *name, RexxClass *classObject, bool publicClass)
{
    // force the directives to be processed first
    install();
    // make sure we have this created
    if (installed_classes == OREF_NULL)
    {
        OrefSet(this, installed_classes, new_directory());
    }
    installed_classes->setEntry(name, classObject);
    if (publicClass)
    {
        // make sure we have this created also
        if (installed_public_classes == OREF_NULL)
        {
            OrefSet(this, installed_public_classes, new_directory());
        }
        installed_public_classes->setEntry(name, classObject);
    }
}


/**
 * Add an installed routine to this source package
 *
 * @param name   The routine name
 * @param classObject
 *               The routine object
 * @param publicClass
 *               Indicates whether this needs to be added to the public list as well.
 */
void RexxSource::addInstalledRoutine(RexxString *name, RoutineClass *routineObject, bool publicRoutine)
{
    // force the directives to be processed first
    install();
    // make sure we have this created
    if (routines == OREF_NULL)
    {
        OrefSet(this, routines, new_directory());
    }
    routines->setEntry(name, routineObject);
    if (publicRoutine)
    {
        // make sure we have this created
        if (public_routines == OREF_NULL)
        {
            OrefSet(this, public_routines, new_directory());
        }
        public_routines->setEntry(name, routineObject);
    }
}



