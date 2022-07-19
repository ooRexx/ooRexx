/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2020 Rexx Language Association. All rights reserved.    */
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
/*----------------------------------------------------------------------------*/
/******************************************************************************/
/* Object REXX Kernel                                                         */
/*                                                                            */
/* Main language parser transient class.                                      */
/*                                                                            */
/******************************************************************************/

#include <algorithm>
#include "RexxCore.h"
#include "LanguageParser.hpp"
#include "ProgramSource.hpp"
#include "MethodClass.hpp"
#include "RoutineClass.hpp"
#include "PackageClass.hpp"
#include "RexxCode.hpp"
#include "StringTableClass.hpp"
#include "ClassDirective.hpp"
#include "StackFrameClass.hpp"
#include "ActivationFrame.hpp"
#include "RexxActivation.hpp"
#include "RexxLocalVariables.hpp"
#include "SelectInstruction.hpp"
#include "ElseInstruction.hpp"
#include "EndIf.hpp"
#include "EndInstruction.hpp"
#include "DoInstruction.hpp"
#include "OtherwiseInstruction.hpp"
#include "CallInstruction.hpp"
#include "ExpressionFunction.hpp"
#include "ExpressionVariable.hpp"
#include "ExpressionCompoundVariable.hpp"
#include "ExpressionStem.hpp"
#include "ExpressionDotVariable.hpp"
#include "SpecialDotVariable.hpp"
#include "ExpressionMessage.hpp"
#include "ExpressionOperator.hpp"
#include "ExpressionLogical.hpp"
#include "ExpressionList.hpp"
#include "RexxInternalApis.h"
#include "SystemInterpreter.hpp"
#include "TraceSetting.hpp"
#include "ExpressionQualifiedFunction.hpp"
#include "ExpressionClassResolver.hpp"
#include "VariableReferenceOp.hpp"


const char *ENCODED_NEEDLE = "/**/@REXX@";   // from ProgramMetaData.cpp


/**
 * Static method for creating a new MethodClass instance.
 *
 * @param name   The name given to the method and package.
 * @param source The code source as an array of strings.
 * @param sourceContext
 *               A parent source context that this method will inherit
 *               a package environment from.
 *
 * @return An executable method object.
 */
MethodClass *LanguageParser::createMethod(RexxString *name, ArrayClass *source, PackageClass *sourceContext)
{
    // check whether the data is representing an encoded compiled Rexx program (rexxc with the '/e' switch),
    // i.e. the second entry consists of the string "/**/@REXX@" only;
    if (source->items()>1 && source->get(2)->stringValue()->strCompare(ENCODED_NEEDLE))
    {
        Protected<RexxString> strSource=source->toString(GlobalNames::LINES, GlobalNames::LINEFEED);  // use single LF for concatenation
        Protected<BufferClass> program_buffer = new_buffer(strSource->getStringData(), strSource->getLength());

        // try to restore a compiled and encoded program first
        Protected<MethodClass> method = MethodClass::restore(name, program_buffer);
        if (method != (MethodClass *)OREF_NULL)
        {
            return method;
        }
    }

    // create the appropriate array source, then the parser, then generate the
    // code.
    Protected<ProgramSource> programSource = new ArrayProgramSource(source);
    Protected<LanguageParser> parser = new LanguageParser(name, programSource);
    return parser->generateMethod(sourceContext);
}


/**
 * Static method for creating a new MethodClass instance.
 *
 * @param name   The name given to the method and package.
 * @param source The code source as a buffer
 * @param sourceContext
 *               A parent source context that this method will inherit
 *               a package environment from.
 *
 * @return An executable method object.
 */
MethodClass *LanguageParser::createMethod(RexxString *name, BufferClass *source)
{
    // create the appropriate array source, then the parser, then generate the
    // code.
    Protected<ProgramSource> programSource = new BufferProgramSource(source);
    Protected<LanguageParser> parser = new LanguageParser(name, programSource);
    return parser->generateMethod();
}


/**
 * Static method for creating a new MethodClass instance from a
 * file.
 *
 * @param name   The filename to read the method source from.
 * @param sourceContext
 *               A parent source context that this method will inherit
 *               a package environment from.
 *
 * @return An executable method object.
 */
MethodClass *LanguageParser::createMethod(RexxString *name, PackageClass *sourceContext)
{
    // load the file into a buffer
    Protected<BufferClass> program_buffer = FileProgramSource::readProgram(name->getStringData());
    // if this failed, report an error now.
    if (program_buffer == (BufferClass *)OREF_NULL)
    {
        reportException(Error_Program_unreadable_name, name);
    }

    // try to restore a flattened program first
    Protected<MethodClass> method = MethodClass::restore(name, program_buffer);
    if (method != (MethodClass *)OREF_NULL)
    {
        // method->setPackageObject(sourceContext);
        // method->getPackage()->addPackage(sourceContext);
        // method->getPackage()->inheritPackageContext(sourceContext);
        // method->getPackage()->install();
        return method;
    }

    // create the appropriate program source, then the parser, then generate the
    // code.
    Protected<ProgramSource> programSource = new BufferProgramSource(program_buffer);
    Protected<LanguageParser> parser = new LanguageParser(name, programSource);
    return parser->generateMethod(sourceContext);
}


/**
 * Static method for creating a new RoutineClass instance.
 *
 * @param name   The name given to the routine and package.
 * @param source The code source as an array of strings.
 * @param sourceContext
 *               A parent source context that this method will inherit
 *               a package environment from.
 *
 * @return An executable method object.
 */
RoutineClass *LanguageParser::createRoutine(RexxString *name, ArrayClass *source, PackageClass *sourceContext)
{
    // check whether the data is representing an encoded compiled Rexx program (rexxc with the '/e' switch),
    // i.e. the second entry consists of the string "/**/@REXX@" only;
    if (source->items()>1 && source->get(2)->stringValue()->strCompare(ENCODED_NEEDLE))
    {
        Protected<RexxString> strSource=source->toString(GlobalNames::LINES, GlobalNames::LINEFEED);  // use single LF for concatenation
        Protected<BufferClass> program_buffer = new_buffer(strSource->getStringData(), strSource->getLength());

        // try to restore a compiled and encoded program first
        Protected<RoutineClass> routine = RoutineClass::restore(name, program_buffer);
        if (routine != (RoutineClass *)OREF_NULL)
        {
            return routine;
        }
    }

    // create the appropriate array source, then the parser, then generate the
    // code.
    Protected<ProgramSource> programSource = new ArrayProgramSource(source);
    Protected<LanguageParser> parser = new LanguageParser(name, programSource);
    return parser->generateRoutine(sourceContext);
}


/**
 * Static method for creating a new RoutineClass instance from a
 * file.
 *
 * @param name   The filename to read the routine source from.
 * @param sourceContext
 *               A parent source context that this routine will inherit
 *               a package environment from.
 *
 * @return An executable routine object.
 */
RoutineClass* LanguageParser::createRoutine(RexxString *name, PackageClass *sourceContext)
{
    // load the file into a buffer
    Protected<BufferClass> program_buffer = FileProgramSource::readProgram(name->getStringData());
    // if this failed, report an error now.
    if (program_buffer == (BufferClass *)OREF_NULL)
    {
        reportException(Error_Program_unreadable_name, name);
    }

    // try to restore a flattened program first
    Protected<RoutineClass> routine = RoutineClass::restore(name, program_buffer);
    if (routine != (RoutineClass *)OREF_NULL)
    {
        return routine;
    }

    // process this from the source
    return createRoutine(name, program_buffer, sourceContext);
}


/**
 * Static method for creating a new RoutineClass instance.
 *
 * @param name   The name given to the routine and package.
 * @param source The code source as a buffer
 * @param sourceContext
 *               A parent source context that this method will inherit
 *               a package environment from.
 *
 * @return An executable method object.
 */
RoutineClass* LanguageParser::createRoutine(RexxString *name, BufferClass *source, PackageClass *sourceContext)
{
    // try to restore a flattened program first
    Protected<RoutineClass> routine = RoutineClass::restore(name, source);
    if (routine != (RoutineClass *)OREF_NULL)
    {
        return routine;
    }
    // create the appropriate array source, then the parser, then generate the
    // code.
    Protected<ProgramSource> programSource = new BufferProgramSource(source);
    Protected<LanguageParser> parser = new LanguageParser(name, programSource);
    return parser->generateRoutine(sourceContext);
}


/**
 * Static method for creating a new RoutineClass instance as a
 * top-level program (no install step is run)
 *
 * @param name   The name given to the routine and package.
 * @param source The buffer containing the source.
 *
 * @return An executable method object.
 */
RoutineClass* LanguageParser::createProgram(RexxString *name, BufferClass *source)
{
    // try to restore a flattened program first
    Protected<RoutineClass> routine = RoutineClass::restore(name, source);
    if (routine != (RoutineClass *)OREF_NULL)
    {
        return routine;
    }
    // create the appropriate array source, then the parser, then generate the
    // code.
    Protected<ProgramSource> programSource = new BufferProgramSource(source);
    Protected<LanguageParser> parser = new LanguageParser(name, programSource);
    return parser->generateProgram();
}


/**
 * Static method for creating a new RoutineClass instance as a
 * top-level program (no install step is run)
 *
 * @param name   The name given to the routine and package.
 * @param source The buffer containing the source.
 *
 * @return An executable method object.
 */
RoutineClass* LanguageParser::createProgram(RexxString *name, ArrayClass *source, PackageClass *sourceContext)
{
    // check whether the data is representing an encoded compiled Rexx program (rexxc with the '/e' switch),
    // i.e. the second entry consists of the string "/**/@REXX@" only;
    if (source->items()>1 && source->get(2)->stringValue()->strCompare(ENCODED_NEEDLE))
    {
        Protected<RexxString> strSource=source->toString(GlobalNames::LINES, GlobalNames::LINEFEED);  // use single LF for concatenation
        Protected<BufferClass> program_buffer = new_buffer(strSource->getStringData(), strSource->getLength());

        // try to restore a compiled and encoded program first
        Protected<RoutineClass> routine = RoutineClass::restore(name, program_buffer);
        if (routine != (RoutineClass *)OREF_NULL)
        {
            return routine;
        }
    }

    // create the appropriate array source, then the parser, then generate the
    // code.
    Protected<ProgramSource> programSource = new ArrayProgramSource(source);
    Protected<LanguageParser> parser = new LanguageParser(name, programSource);
    return parser->generateProgram(sourceContext);
}


/**
 * Static method for creating a new RoutineClass instance as a
 * top-level program (no install step is run)
 *
 * @param name   The name given to the routine and package.
 * @param source The buffer containing the source.
 *
 * @return An executable method object.
 */
RoutineClass* LanguageParser::createProgram(RexxString *name)
{
    // create the appropriate program source, then the parser, then generate the
    // code.
    Protected<ProgramSource> programSource = new FileProgramSource(name);
    Protected<LanguageParser> parser = new LanguageParser(name, programSource);
    return parser->generateProgram();
}


/**
 * Retrieve a routine object from a file.  This will first attempt
 * to restore a previously translated image, then will try to
 * translate the source if that fails.
 *
 * @param filename The target file name.
 *
 * @return A resulting Routine object, if possible.
 */
RoutineClass* LanguageParser::createProgramFromFile(RexxString *filename)
{
    // load the file into a buffer
    Protected<BufferClass> program_buffer = FileProgramSource::readProgram(filename->getStringData());
    // if this failed, report an error now.
    if (program_buffer == (BufferClass *)OREF_NULL)
    {
        reportException(Error_Program_unreadable_name, filename);
    }

    // try to restore a flattened program first
    Protected<RoutineClass> routine = RoutineClass::restore(filename, program_buffer);
    if (routine != (RoutineClass *)OREF_NULL)
    {
        return routine;
    }

    // process this from the source
    return createProgram(filename, program_buffer);
}


/**
 * Retrieve a package object from a file.  This will first
 * attempt to restore a previously translated image, then will
 * try to translate the source if that fails.
 *
 * @param filename The target file name.
 *
 * @return A resulting Routine object, if possible.
 */
PackageClass *LanguageParser::createPackage(RexxString *name, ArrayClass *source, PackageClass *sourceContext)
{
    // we just load this as a program object and return the associated package
    return createProgram(name, source, sourceContext)->getPackage();
}


/**
 * Retrieve a package object from a file.  This will first
 * attempt to restore a previously translated image, then will
 * try to translate the source if that fails.
 *
 * @param filename The target file name.
 *
 * @return A resulting Routine object, if possible.
 */
PackageClass *LanguageParser::createPackage(RexxString *name, BufferClass *source)
{
    // we just load this as a program object and return the associated package
    return createProgram(name, source)->getPackage();
}


/**
 * Create a package object from an array source.
 *
 * @param filename The package name
 *
 * @return A resulting Package object, if possible.
 */
PackageClass *LanguageParser::createPackage(RexxString *filename)
{
    // we just load this as a program object and return the associated package
    return createProgramFromFile(filename)->getPackage();
}


/**
 * Translate a single string value for an interpret
 * instruction.
 *
 * @param interpretString
 *                   The string to interpret.
 * @param labels     The labels from the interpret context.
 * @param lineNumber The line number of the interpret location.
 *
 * @return The interpreted code.
 */
RexxCode *LanguageParser::translateInterpret(RexxString *interpretString, PackageClass *sourceContext, StringTable *labels, size_t lineNumber)
{
    // create the appropriate array source, then the parser, then generate the
    // code.
    ProgramSource *programSource = new ArrayProgramSource(new_array(interpretString), lineNumber);
    // the package for the interpret will inherit the program name from the parent context.
    Protected<LanguageParser> parser = new LanguageParser(sourceContext->getProgramName(), programSource);
    return parser->translateInterpret(sourceContext, labels);
}


/**
 * Allocate a new LanguageParser item
 *
 * @param size    The base object size.
 *
 * @return The storage for creating a LanguageParser.
 */
void *LanguageParser::operator new(size_t size)
{
   return new_object(size, T_LanguageParser);
}


/**
 * Construct a program source object.
 *
 * @param p      The source package we're parsing code for.
 * @param s      The provider for the actual program source.
 */
LanguageParser::LanguageParser(RexxString *n, ProgramSource *s)
{
    // at this point, we just save the link back to the
    // package and source objects.  We hold off creating
    // more objects until we start parsing.
    name = n;
    source = s;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void LanguageParser::live(size_t liveMark)
{
    // because of the way garbage collection works, it is a good
    // idea to mark these first because they are chained.
    memory_mark(mainSection);
    memory_mark(firstInstruction);
    memory_mark(lastInstruction);
    memory_mark(currentInstruction);

    memory_mark(package);
    memory_mark(source);
    memory_mark(name);
    memory_mark(clause);
    memory_mark(holdStack);
    memory_mark(variables);
    memory_mark(constantVariables);
    memory_mark(literals);
    memory_mark(dotVariables);
    memory_mark(labels);
    memory_mark(strings);
    memory_mark(guardVariables);
    memory_mark(exposedVariables);
    memory_mark(localVariables);
    memory_mark(control);
    memory_mark(terms);
    memory_mark(subTerms);
    memory_mark(operators);
    memory_mark(calls);
    memory_mark(activeClass);
    memory_mark(classes);
    memory_mark(unattachedMethods);
    memory_mark(classDependencies);
    memory_mark(routines);
    memory_mark(publicRoutines);
    memory_mark(requires);
    memory_mark(libraries);
    memory_mark(resources);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void LanguageParser::liveGeneral(MarkReason reason)
{
    // because of the way garbage collection works, it is a good
    // idea to mark these first because they are chained.
    memory_mark_general(mainSection);
    memory_mark_general(firstInstruction);
    memory_mark_general(lastInstruction);
    memory_mark_general(currentInstruction);

    memory_mark_general(package);
    memory_mark_general(source);
    memory_mark_general(name);
    memory_mark_general(clause);
    memory_mark_general(holdStack);
    memory_mark_general(variables);
    memory_mark_general(constantVariables);
    memory_mark_general(literals);
    memory_mark_general(dotVariables);
    memory_mark_general(labels);
    memory_mark_general(strings);
    memory_mark_general(guardVariables);
    memory_mark_general(exposedVariables);
    memory_mark_general(localVariables);
    memory_mark_general(control);
    memory_mark_general(terms);
    memory_mark_general(subTerms);
    memory_mark_general(operators);
    memory_mark_general(calls);
    memory_mark_general(activeClass);
    memory_mark_general(classes);
    memory_mark_general(unattachedMethods);
    memory_mark_general(classDependencies);
    memory_mark_general(routines);
    memory_mark_general(publicRoutines);
    memory_mark_general(requires);
    memory_mark_general(libraries);
    memory_mark_general(resources);
}


/**
 * Generate a method object from a source collection.
 *
 * @param sourceContext
 *               An optional source context used to add additional visibilty to
 *               dynamically generated methods.
 *
 * @return A method object represented by the leading code block
 *         of the source.  Ideally, this code should not contain directives,
 *         but since this was allowed in the past, we need to continue
 *         to allow this.
 */
MethodClass *LanguageParser::generateMethod(PackageClass *sourceContext)
{
    // initialize, and compile all of the source.
    compileSource();
    // get the main section of the source package and make a method
    // object from it.  This is the package "main" executable.
    package->mainExecutable = new MethodClass(name, mainSection);
    // since we've explicitly made this a method, there is no
    // longer an init code section to this package.
    package->initCode = OREF_NULL;
    // if we have a source context, then we need to inherit this
    // context before doing the install so that anything from the parent
    // context is visible during the install processing.
    package->inheritPackageContext(sourceContext);

    // force the package to resolve classes/libraries now.
    installPackage();
    // return the main executable.
    return (MethodClass *)package->mainExecutable;
}


/**
 * Generate a routine object from a source collection.
 *
 * @param sourceContext
 *               An optional source context used to add additional visibilty to
 *               dynamically generated methods.
 *
 * @return A routine object represented by the leading code
 *         block of the source.  Ideally, this code should not
 *         contain directives, but since this was allowed in the
 *         past, we need to continue to allow this.
 */
RoutineClass *LanguageParser::generateRoutine(PackageClass *sourceContext)
{
    // initialize, and compile all of the source.
    compileSource();
    // get the main section of the source package and make a method
    // object from it.  This is the package "main" executable.
    package->mainExecutable = new RoutineClass(name, mainSection);
    // since we've explicitly made this a method, there is no
    // longer an init code section to this package.
    package->initCode = OREF_NULL;
    // if we have a source context, then we need to inherit this
    // context before doing the install so that anything from the parent
    // context is visible during the install processing.
    package->inheritPackageContext(sourceContext);

    // force the package to resolve classes/libraries now.
    installPackage();
    // return the main executable.
    return (RoutineClass *)package->mainExecutable;
}


/**
 * Generate a routine object as a top-level program.
 *
 * @return A routine object represented by the leading code
 *         block of the source.  This will not have an installs
 *         performed at this time.
 */
RoutineClass *LanguageParser::generateProgram(PackageClass *sourceContext)
{
    // initialize, and compile all of the source.
    compileSource();
    // get the main section of the source package and make a method
    // object from it.  This is the package "main" executable.
    package->mainExecutable = new RoutineClass(name, mainSection);
    // this has code marked as the init code that will run the first
    // time something on this is called.  We do not do any installation
    // at this time.
    package->initCode = mainSection;
    // if we have a source context, then we need to inherit this
    // context before doing the install so that anything from the parent
    // context is visible during the install processing.
    package->inheritPackageContext(sourceContext);
    // return the main executable.
    return (RoutineClass *)package->mainExecutable;
}


/**
 * Translate an interpret string.
 *
 * @param contextlabels
 *               The labels for the interpreting source program.
 *
 * @return A RexxCode object resulting from the compilation of
 *         this interpret line.
 */
RexxCode *LanguageParser::translateInterpret(PackageClass *sourceContext, StringTable *contextLabels)
{
    // to translate this, we use the labels from the parent context.
    labels = contextLabels;
    // turn on the interpret restrictions
    setInterpret();
    // initialize, and compile all of the source.
    compileSource();

    // if we have a source context, then we need to inherit this
    // context before doing the install so that anything from the parent
    // context is visible during the install processing.
    package->inheritPackageContext(sourceContext);

    //. the package ends up with neither a init section or a main executable.
    // we just return the main code section
    // return the main executable.
    return mainSection;
}


/**
 * Load a package and return the source object for the package.
 * We create the main executable as a Routine object, but also
 * leave the initialization code in place for the package loader
 * to use.
 *
 * @return A package object representing the package.
 */
PackageClass *LanguageParser::generatePackage(PackageClass *sourceContext)
{
    // initialize, and compile all of the source.
    compileSource();
    // get the main section of the source package and make a method
    // object from it.  This is the package "main" executable.
    package->mainExecutable = new RoutineClass(name, mainSection);
    // The main section is also set into the package init code to be
    // run as part of package loading.
    package->initCode = mainSection;
    // if we have a source context, then we need to inherit this
    // context before doing the install so that anything from the parent
    // context is visible during the install processing.
    package->inheritPackageContext(sourceContext);
    // return the package.
    return package;
}


/**
 * Compile our configured source into executable code.
 */
void LanguageParser::compileSource()
{
    // initialize the global environment for parsing this source
    // into an executable.
    initializeForParsing();

    // now translate the code
    translate();
}


/**
 * Initialize the parser before starting the parse operation.
 * NOTE:  This is a transient object, which will never be stored
 * in the oldspace, so we don't need to use OrefSet.
 */
void LanguageParser::initializeForParsing()
{
    // create a package object that we'll be filling in.
    package = new PackageClass(name, source);
    // do setup, which also initializes the program source.
    package->setup();

    // get the count of lines
    lineCount = source->getLineCount();

    // and the lineNumber we should start reading from.  For interpret,
    // this will be one line before the interpret instruction.  This allows
    // all syntax errors to be reported on the correct line.
    lineNumber = source->getFirstLine();

    // position at the start of that line
    position(lineNumber, 0);

    // handy stack for temporary values...this is a push through
    holdStack = new (HOLDSIZE) PushThroughStack(HOLDSIZE);

    // general parsing control setups
    control = new_queue();        // our stack of control instructions
    terms = new_queue();          // expression term stack
    subTerms = new_queue();       // temporary stack for holding lists of terms
    operators = new_queue();      // the operator queue
    literals = new_string_table();   // table of literal values
    dotVariables = new_string_table();   // table of dot variables

    // we have three special dot variables that are given special treatment. Go ahead and
    // create the retriever objects now
    dotVariables->put(new SpecialDotVariable(GlobalNames::DOTNIL, TheNilObject), GlobalNames::DOTNIL);
    dotVariables->put(new SpecialDotVariable(GlobalNames::DOTTRUE, TheTrueObject), GlobalNames::DOTTRUE);
    dotVariables->put(new SpecialDotVariable(GlobalNames::DOTFALSE, TheFalseObject), GlobalNames::DOTFALSE);

    // during an image build, we have a global string table.  If this is
    // available now, use it.
    strings = memoryObject.getGlobalStrings();
    if (strings == OREF_NULL)
    {
        // no global string table, use a local copy
        strings = new_string_table();
    }

    // create the singleton clause object for parsing
    clause = new RexxClause();
}


/**
 * Mark the package that it requires at least the given
 * language level to execute.
 *
 * @param l      The new language level.
 */
void LanguageParser::requireLanguageLevel(LanguageLevel l)
{
    LanguageLevel oldLevel = package->getLanguageLevel();
    if (l > oldLevel)
    {
        package->setLanguageLevel(l);
    }
}


/**
 * Test if this version of the interpreter can handle
 * executing a program created with a given language level.
 *
 * @param l      The target language level.
 *
 * @return True if the given level is within the range supported
 *         by this interpreter version.
 */
bool LanguageParser::canExecute(LanguageLevel l)
{
    return l >= MinimumLanguageLevel && l <= MaximumLanguageLevel;
}


/**
 * Initialize the global tables used for keeping track of
 * directive information.
 */
void LanguageParser::initializeForDirectives()
{
    routines = new_string_table();
    publicRoutines = new_string_table();
    classDependencies = new_string_table();
    requires = new_array();
    libraries = new_array();
    classes = new_array();
    activeClass = OREF_NULL;
    unattachedMethods = new_string_table();
    resources = new_string_table();
}


/**
 * Have the generated package process the package
 * installation step at completion of parsing.
 */
void LanguageParser::installPackage()
{
    package->install();
}


/**
 * Create a stack frame for this parsing context.
 *
 * @return a stack frame instance for error reporting
 */
StackFrameClass *LanguageParser::createStackFrame()
{
    // construct the traceback line before we allocate the stack frame object.
    // calling this in the constructor argument list can cause the stack frame instance
    // to be inadvertently reclaimed if a GC is triggered while evaluating the constructor
    // arguments.
    RexxString *traceback = package->traceBack(OREF_NULL, clauseLocation, 0, true);
    ProtectedObject p(traceback);
    return new StackFrameClass(StackFrameClass::FRAME_COMPILE, package->programName, OREF_NULL,
        OREF_NULL, OREF_NULL, traceback, clauseLocation.getLineNumber());
}


/**
 * Validate that a given token is a variable (i.e., a symbol
 * that does not begin with a digit).
 *
 * @param token  The target token.
 */
void LanguageParser::needVariable(RexxToken  *token)
{
    // this must be a variable token.
    if (!token->isVariable())
    {
        // the error message depends on whether this begins with a dot
        // or is a numeric value
        if (token->value()->getChar(0) == '.')
        {
            syntaxError(Error_Invalid_variable_period, token);
        }
        else
        {
            syntaxError(Error_Invalid_variable_number, token);
        }
    }
}


/**
 * Validate that a token is a variable or the literal
 * dot (e.g. the tokens allowed in a parsing template).
 *
 * @param token  The target token.
 */
void LanguageParser::needVariableOrDotSymbol(RexxToken  *token)
{
    // check if the correct types
    if (!token->isVariable() && !token->isDotSymbol())
    {
        syntaxError(Error_Invalid_variable_number, token);
    }
}


/**
 * Advance the current position to the next line.
 */
void LanguageParser::nextLine()
{
    if (clause != OREF_NULL)       // do we have a clause object active?
    {
        // record the current position in the clause...this marks the location end.
        clause->setEnd(lineNumber, lineOffset);
    }
    // move to the start of the next line
    position(lineNumber + 1, 0);
}


/**
 * Advance the current position to the next line, but only if we
 * are not already positioned at the first character.
 */
void LanguageParser::conditionalNextLine()
{
    // if scanning of the previous clause terminated at the end of the
    // line, the position has already been advanced to the next line and
    // we'll be sitting at the first character
    if (lineOffset != 0)
    {
        nextLine();
    }
}


/**
 * Check the current line against the end marker to see
 * if this is a match.
 *
 * @param marker The marker string to check.
 *
 * @return true if this line begins with the marker, false otherwise.
 */
bool LanguageParser::checkMarker(RexxString *marker)
{
    size_t checkLength = marker->getLength();
    // quick return if this is too short
    if (checkLength > currentLength)
    {
        return false;
    }
    // do a strict compare with the beginning of the line.  we only trigger on
    // the line beginning with the string, anything else is ignored.
    return memcmp(marker->getStringData(), current, checkLength) == 0;
}


/**
 * Extract the current line as a string object.
 *
 * @return The current line as a string object.
 */
RexxString *LanguageParser::getStringLine()
{
    return new_string(current, currentLength);
}


/**
 * Move the current scan position to a new source location.
 *
 * @param line   The target line number.
 * @param offset The offset within the new line.
 */
void LanguageParser::position(size_t line, size_t offset)
{
    lineNumber = line;                 // set the new position information
    lineOffset = offset;

    // retrieve the line specifics from the source object.  If this
    // is out of bounds, current will be nulled out.
    source->getLine(line, current, currentLength);
}

/**
 * Extract a clause from the source and return as a clause
 * object.  The clause object contains a list of all of the
 * tokens contained within the clause and is used by
 * the parser to determine the type of instruction and
 * create the instruction parse tree.
 *
 * @return true if we managed to get a clause, false otherwise.
 */
bool LanguageParser::nextClause()
{
    SourceLocation location;             // location of the clause
    SourceLocation tokenLocation;        // location of each token
    RexxToken *token = OREF_NULL;

    // if reclamed is true, we have previously given up on processing a clause, so
    // the tokens are already available.  This is not typically the case.
    if (!flags.test(reclaimed))
    {
        // we keep one clause object, just reset the tokens in the clause.
        clause->newClause();
        // scan over all null clauses.  These could be blank lines, comments,
        // semi-colons, etc.  Those are not interesting.
        for (;;)
        {
            // record the start position
            clause->setStart(lineNumber, lineOffset);
            // get the next source token.  White space tokens are ignored in this context,
            // as we're looking for the beginning of a real clause.
            token = sourceNextToken(OREF_NULL);
            // OREF_NULL indicates we've hit the end of the source.  Mark us as
            // finished and return
            if (token->isEndOfFile())
            {
                flags.set(noClause);
                return false;
            }
            // we have a token, but is it an end of clause marker (explicit or implicit
            // semicolon).  If not, we've got a real clause to process.
            if (!token->isEndOfClause())
            {
                break;
            }
            // reset the clause and start again.
            clause->newClause();
        }
        // get the clause start position from the first token, and
        // make a copy for the start
        tokenLocation = token->getLocation();
        location = tokenLocation;
        // record this in the clause for potential error reporting.
        clause->setLocation(location);

        // now consume tokens until we find an end of clause marker.
        for (;;)
        {
            // get the next token.  Blanks can now be significant inside
            // a clause.
            token = sourceNextToken(token);
            // and save the position again.
            tokenLocation = token->getLocation();
            // and quit once we've hit an end-of-clause (note, we'll never
            // see a null return here because there will be an end of clause
            // implied by the end of line.  We only see the null case when
            // we're scanning for a new clause.
            if (token->isEndOfClause())
            {
                break;
            }
        }

        // the location of the last scanned token is the clause end position.
        location.setEnd(tokenLocation);
        // and set this in the class to give it the full bounds.
        clause->setLocation(location);
    }

    // no reclaimed clause now, regardless of how we got here.
    flags.reset(reclaimed);
    // always set the error information for the clause being processed.
    clauseLocation = clause->getLocation();
    // we have a clause
    return true;
}


/**
 * Translate a source object into executable code.  This translates
 * the main code section and stores it in the mainSection
 * field.  If permitted (i.e, not an interpret), this will
 * also process any additional directives and associated
 * code sections.
 */
void LanguageParser::translate()
{
    // create a stack frame so errors can display the parsing location.
    CompileActivationFrame frame(ActivityManager::currentActivity, this);

    // set up the package global defaults
    package->packageSettings.setDefault();

    // go translate the lead block.  We will figure out what type of
    // object this gets turned into later.
    mainSection = translateBlock();
    // we might have directives to process, which adds additional stuff
    // to the package.
    if (!atEnd())
    {
        // we store a lot of stuff that we only need if there are directives.
        // set up to handle this now.
        initializeForDirectives();
        // no active class definition
        activeClass = OREF_NULL;

        // We only allow directives when translating ... interpret need not apply
        if (flags.test(noDirectives))
        {
            // step to the next clause to report the error
            nextClause();
            syntaxError(Error_Translation_directive_interpret);
        }

        // now loop until we hit the end of the source processing directives.
        while (!atEnd())
        {
            nextDirective();
        }
        // resolve any class dependencies
        resolveDependencies();
    }
}


/**
 * Initialize all of the fields that need to start clean for a new
 * translation section.
 */
void LanguageParser::initializeForTranslation()
{
    // initialize the parsing environment.
    firstInstruction = OREF_NULL;
    lastInstruction = OREF_NULL;
    // get a list of all calls that might need resolution
    calls = new_array();
    // a table of variables...starting with the special variables we allocated space for.
    variables = (StringTable *)TheCommonRetrievers->copy();
    // restart the variable index
    variableIndex = RexxLocalVariables::FIRST_VARIABLE_INDEX;

    // do we have labels from an interpret?
    // only create a new set if we're not reusing.
    if (labels == OREF_NULL)
    {
        labels = new_string_table();
    }

    // until we need guard variables, we don't need the table
    guardVariables = OREF_NULL;
    // and we reset the exposed/local variables for each code section
    exposedVariables = OREF_NULL;
    localVariables = OREF_NULL;

    // clear the stack accounting fields
    maxStack = 0;
    currentStack = 0;
    // we're not at the end yet
    flags.reset(noClause);
}


/**
 * Translate a block of REXX code (delimited by possible
 * directive instructions
 *
 * @return A RexxCode object for this block.
 */
RexxCode *LanguageParser::translateBlock()
{
    // reset for a fresh translation
    initializeForTranslation();

    // add a dummy instruction at the front.  All other instructions get chained off of this.
    RexxInstruction *instruction = new RexxInstruction(OREF_NULL, KEYWORD_FIRST);
    // this is the bottom of the control stack, and also the
    // first clause of the code stream.
    firstInstruction = instruction;
    lastInstruction = instruction;

    pushDo(instruction);

    // get a location for this block of code from the first and last instructions
    SourceLocation blockLocation;

    // save the block start position
    blockLocation.setStart(lineNumber == 0 ? 1 : lineNumber, lineOffset);

    // time to start actual parsing.  Continue until we reach the end
    nextClause();

    for (;;)
    {
        // start with no instruction
        instruction = OREF_NULL;
        // At this point, we want to consume any label clauses, since they are
        // not real instructions.
        while (!noClauseAvailable())
        {
            // resolve this clause into an instruction
            instruction = nextInstruction();
            // if nothing is returned, this must be a directive, which terminates
            // parsing of this block.
            if (instruction == OREF_NULL)
            {
                break;
            }
            // not a label, break out of the loop
            if (!instruction->isType(KEYWORD_LABEL))
            {
                break;
            }
            // append the label and try again
            addClause(instruction);
            nextClause();
            // need to zero this out in case we break the loop for an end of file
            instruction = OREF_NULL;
        }
        // ok, have we hit the end of the file or the end of the block?
        if (noClauseAvailable() || instruction == OREF_NULL)
        {
            // see what we have at the top of the control stack, we probably
            // have some cleanup to do.
            InstructionKeyword controltype = topDoType();
            // handle any then or when terminators
            while (controltype == KEYWORD_ENDTHEN || controltype == KEYWORD_ENDWHEN)
            {
                // pop these from the stack and flush and pending control instructions
                popDo();
                flushControl(OREF_NULL);
                controltype = topDoType();
            }
            // In theory, at this point we should be at the first instruction.
            // if not, we have an unclosed block instruction, which is an error
            if (!topDoIsType(KEYWORD_FIRST))
            {
                blockSyntaxError(topDo());
            }
            // remove the top instruction from the stack, and we're done
            popDo();
            break;
        }

        // now check if we need to adjust the control stack for this new instruction.
        InstructionKeyword type = instruction->getType();
        // if this is not an ELSE, we might have a pending THEN to finish.
        if (type != KEYWORD_ELSE)
        {
            // get the type at the type of the stack.
            InstructionKeyword controltype = topDoType();
            // we might need to pop off multiple pending thens while end of an IF or WHEN
            while (controltype == KEYWORD_ENDTHEN || controltype == KEYWORD_ENDWHEN)
            {
                popDo();
                flushControl(OREF_NULL);
                controltype = topDoType();
            }
        }

        // now check the actual disposition of this instruction.  If it is a control type
        // add it immediately to the stream.  Control types are IF, SELECT, or any of the
        // DO/LOOP types.
        if (instruction->isControl())
        {
            addClause(instruction);
        }
        // if this is an ELSE, we don't add a new level, but rather flush
        // any pending control levels.
        else if (type != KEYWORD_ELSE)
        {
            flushControl(instruction);
        }
        // validate allowed instructions in a SELECT
        if (topDoIsType(KEYWORD_SELECT, KEYWORD_SELECT_CASE) &&
            (type != KEYWORD_WHEN && type != KEYWORD_WHEN_CASE && type != KEYWORD_OTHERWISE && type != KEYWORD_END))
        {
            syntaxError(Error_When_expected_whenotherwise, topDo());
        }

        // now handle the different instructions to ensure they
        // are valid in their particular contexts.
        switch (type)
        {
            // a WHEN clause.  The top of the Do stack must be a select.
            case KEYWORD_WHEN:
            case KEYWORD_WHEN_CASE:
            {
                // the top of the queue must be a SELECT instruction, but
                // we have TWO varieties of this.  The second requires the
                // WHEN to be converted to a different type.
                RexxInstruction *second = topDo();
                if (second->isType(KEYWORD_SELECT) || second->isType(KEYWORD_SELECT_CASE))
                {
                    // let the select know that another WHEN was added
                    ((RexxInstructionSelect *)second)->addWhen((RexxInstructionIf *)instruction);
                }
            }

                // processing of an IF instruction, and also a WHEN (from above)
            case  KEYWORD_IF:
            {
                // we need to finish the IF instruction.
                // get the next token
                RexxToken *token = nextReal();
                RexxInstruction *second;
                // Did the line end with no THEN?  It must be on the next
                // line,
                if (token->isEndOfClause())
                {
                    // get the next full clause, which should start with
                    // the THEN keyword.
                    // did we hit the end of file?, this is an error
                    if (!nextClause())
                    {
                        // select appropriate error message: either for the IF here,
                        // or for the WHEN / WHEN_CASE we fell through from above
                        syntaxError(type == KEYWORD_IF ? Error_Then_expected_if : Error_Then_expected_when, instruction);
                    }

                    // now check the next token and ensure it is a THEN keyword.
                    token = nextReal();
                    // Not a THEN keyword?  This is an error
                    if (token->keyword() != KEYWORD_THEN)
                    {
                        // select appropriate error message: either for the IF here,
                        // or for the WHEN / WHEN_CASE we fell through from above
                        syntaxError(type == KEYWORD_IF ? Error_Then_expected_if : Error_Then_expected_when, instruction);
                    }
                    // create a new then clause attached to the IF
                    second = thenNew(token, (RexxInstructionIf *)instruction);
                    // now get the next token.. and ensure is something after the THEN
                    token = nextReal();

                    // if this is a clause end (e.g, end of line), parse off a clause
                    // to make sure there is something there
                    if (token->isEndOfClause())
                    {
                        if (!nextClause())
                        {
                            syntaxError(Error_Incomplete_do_then, instruction);
                        }
                    }
                    else
                    {
                        // step back a token and trim the THEN off of the clause.
                        previousToken();
                        trimClause();
                    }
                }
                // we have THEN on the same line as the IF
                else
                {
                    // attach a THEN to the IF
                    second = thenNew(token, (RexxInstructionIf *)instruction);
                    // there might not be anything after the THEN, so we have to check

                    token = nextReal();
                    // if the THEN was the last thing, ensure we have something on the
                    // next line
                    if (token->isEndOfClause())
                    {
                        // we must have a clause, else there is an error
                        if (!nextClause())
                        {
                            syntaxError(Error_Incomplete_do_then, instruction);
                        }
                    }
                    else
                    {
                        // ok, back up one token and trim the clause to remove the THEN
                        previousToken();
                        trimClause();
                    }
                }

                // add this to the instruction list and also add this to the
                // DO stack in case there is an ELSE to process.
                addClause(second);
                pushDo(second);
                // back to the top to get a new instruction
                continue;
            }

                // we have an ELSE instruction.  Need to verify this is in a correct context.
            case  KEYWORD_ELSE:
            {
                // ok, the top instruction is the key.  It must be the
                // tail end of a THEN instruction that we can attach to.
                RexxInstruction *second = topDo();
                if (!second->isType(KEYWORD_ENDTHEN))
                {
                    syntaxError(Error_Unexpected_then_else);
                }

                // ok, add the ELSE to the instruction list, pop the
                // THEN off of the control stack, push the ELSE on to the stack,
                // and hook them together.  The ELSE will need to be completed
                // once the next instruction has been parsed off.
                addClause(instruction);
                second = popDo();
                // this is the ELSE
                pushDo(instruction);
                // join the THEN and ELSE together
                ((RexxInstructionElse *)instruction)->setParent((RexxInstructionEndIf *)second);
                ((RexxInstructionEndIf *)second)->setEndInstruction((RexxInstructionEndIf *)instruction);

                // check for a dangling ELSE now.
                RexxToken *token = nextReal();

                // if the next token is the end of the line (or potentially, a semicolon)
                // we're not at the end.
                if (token->isEndOfClause())
                {
                    // dangling ELSE if we can't get another clause.
                    if (!nextClause())
                    {
                        syntaxError(Error_Incomplete_do_else, instruction);
                    }
                }
                // ELSE followed by an instruction on the same line.  We need to
                // push the token we grabbed back on to the clause and trim off the
                // ELSE part we've already process.
                else
                {
                    previousToken();
                    trimClause();
                }
                // continue parsing instructions.
                continue;
            }


                // found the start of an OTHERWISE group.  We need to verify that
                // we're really working on a SELECT.
            case  KEYWORD_OTHERWISE:
            {
                // we must have a SELECT at the top of the control stack
                RexxInstruction *second = topDo();
                if (!second->isType(KEYWORD_SELECT) && !second->isType(KEYWORD_SELECT_CASE))
                {
                    syntaxError(Error_Unexpected_when_otherwise);
                }

                // hook the otherwise up to the SELECT and push the otherwise
                // on to the top of the stack until we find an END.
                ((RexxInstructionSelect *)second)->setOtherwise((RexxInstructionOtherwise *)instruction);
                pushDo(instruction);

                // we could have the OTHERWISE on the same line as its following instruction, so
                // we need to trim the instruction
                RexxToken *token = nextReal();
                if (!token->isEndOfClause())
                {
                    previousToken();
                    trimClause();
                    // we have a clause, go back around and process this
                    continue;
                }
                // on the next line, fall through to the bottom of the loop
                // to get the next clause.
                break;
            }

                // An END instruction.  This could the the closure for a DO, LOOP, or SELECT.
                // its matchup should be on the top of the control stack.
            case  KEYWORD_END:
            {
                // ok, pop the top instruction.  If this is the
                // correct type, we're finished with this.  If not the
                // correct type, we have an error.
                RexxInstruction *second = popDo();
                type = second->getType();
                // that the top of the stack is an acceptable block-type instruction.
                // This includes all of the DO/LOOP variants, both Select types, and Otherwise
                if (!second->isBlock())
                {
                    // we have a couple of specific errors based on what sort of instruction is
                    // on the top.
                    if (type == KEYWORD_ELSE)
                    {
                        syntaxError(Error_Unexpected_end_else);
                    }
                    else if (type == KEYWORD_IFTHEN || type == KEYWORD_WHENTHEN)
                    {
                        syntaxError(Error_Unexpected_end_then);
                    }
                    else
                    {
                        syntaxError(Error_Unexpected_end_nodo);
                    }
                }

                // ok, we have something good, now there is some additional
                // processing required for specific block instructions

                // if the top of the stack is an OTHERWISE, then the
                // END is really closing the SELECT.  Replace the Otherwise
                // with the Select.
                if (type == KEYWORD_OTHERWISE)
                {
                    second = popDo();
                }

                // Now do the apprpriate closure action based on the instruction type.
                if (type == KEYWORD_SELECT || type == KEYWORD_SELECT_CASE)
                {
                    ((RexxInstructionSelect *)second)->matchEnd((RexxInstructionEnd *)instruction, this);
                }
                else                           /* must be a DO block                */
                {
                    ((RexxBlockInstruction *)second)->matchEnd((RexxInstructionEnd *)instruction, this);
                }

                // We've just completed a large block instruction.  It is possible that
                // this was part of something like "if a then do;..end", in which case, the
                // conditional instruction is still in a pending state.  Flush out the control
                // stack to complete now that the block has been closed.
                flushControl(OREF_NULL);
                break;
            }

                // start of new DO group (also picks up LOOP instruction)
                // this gets pushed on to the top of the control stack until
                // we find an END instruction.
            case KEYWORD_DO:
            case KEYWORD_LOOP:
            case KEYWORD_SIMPLE_BLOCK:
            case KEYWORD_LOOP_FOREVER:
            case KEYWORD_LOOP_OVER:
            case KEYWORD_LOOP_OVER_UNTIL:
            case KEYWORD_LOOP_OVER_WHILE:
            case KEYWORD_LOOP_OVER_FOR:
            case KEYWORD_LOOP_OVER_FOR_UNTIL:
            case KEYWORD_LOOP_OVER_FOR_WHILE:
            case KEYWORD_LOOP_WITH:
            case KEYWORD_LOOP_WITH_UNTIL:
            case KEYWORD_LOOP_WITH_WHILE:
            case KEYWORD_LOOP_WITH_FOR:
            case KEYWORD_LOOP_WITH_FOR_UNTIL:
            case KEYWORD_LOOP_WITH_FOR_WHILE:
            case KEYWORD_LOOP_CONTROLLED:
            case KEYWORD_LOOP_CONTROLLED_UNTIL:
            case KEYWORD_LOOP_CONTROLLED_WHILE:
            case KEYWORD_LOOP_COUNT:
            case KEYWORD_LOOP_COUNT_UNTIL:
            case KEYWORD_LOOP_COUNT_WHILE:
            case KEYWORD_LOOP_WHILE:
            case KEYWORD_LOOP_UNTIL:
            {
                pushDo(instruction);
                break;
            }

                // new select group.  Again, we push this on the start of the stack
                // while it awaits its associated WHEN, OTHERWISE, and END bits.
            case  KEYWORD_SELECT:
            case  KEYWORD_SELECT_CASE:
            {
                pushDo(instruction);
                break;
            }

                // all other types of instructions don't require additional processing.
            default:
                break;
        }
        // grab another clause and go around.
        nextClause();
    }

    // we might have function calls or call instructions that had deferred resolution,
    // take care of those now
    resolveCalls();

    // the first instruction is just a dummy we use to anchor
    // everything while parsing.  We can unchain that now.
    firstInstruction = firstInstruction->nextInstruction;
    // if this code block does not contain labels (pretty common if
    // using an oo style), get rid of those too
    if (labels->isEmpty())
    {
        labels = OREF_NULL;
    }

    // a code block need not have any instructions, and it may have leading or
    // trailing blanks. The source block started at the very beginning, now we need
    // find out the location of whatever terminated the block, which will be either
    // a directive clause that has already been parsed or actual end of the source itself.
    // if this block was terminated by discovering another directive, then
    // we set the end of this block to just before
    if (clause != OREF_NULL)
    {
        // get the next clause. If this clause starts at the beginning of
        // a line (common), then we need to use the end of the previous line as the
        // end location.
        SourceLocation nextLocation = clause->getLocation();
        if (nextLocation.getOffset() == 0)
        {
            size_t previousLine = nextLocation.getLineNumber() - 1;
            // this might actually have stepped things back a tick if there truly
            // was nothing there. In that case, set the source location to zero to indicate
            // there was nothing there
            if (blockLocation.getLineNumber() > previousLine)
            {
                blockLocation.setLineNumber(0);
            }
            else
            {
                // we need this to be the end of the previous line
                const char *data = NULL;
                size_t lineLength = 0;

                source->getLine(previousLine, data, lineLength);
                blockLocation.setEnd(previousLine, lineLength);
            }
        }
        // we can just set the end to the beginning of the pending clause
        else
        {
            blockLocation.setEnd(nextLocation.getLineNumber(), nextLocation.getOffset());
        }
    }
    // no active clause, so we set this to the very end of the file
    else
    {
        // we need this to be the end of the previous line
        const char *data = NULL;
        size_t lineLength = 0;

        source->getLine(source->getLineCount(), data, lineLength);
        blockLocation.setEnd(source->getLineCount(), lineLength);
    }


    // now create a code object that is attached to the package.
    // this will have all of the information needed to execute this code.
    RexxCode *code = new RexxCode(package, blockLocation, firstInstruction, labels, maxStack, variableIndex);

    // we don't automatically create the labels when we translate the block because
    // they might have been provided by an interpret.  So always clear them out at the
    // end of a block.
    labels = OREF_NULL;
    // and return the code object.
    return code;
}


/**
 * Attempt to resolve and deferred function or call instructions.
 */
void LanguageParser::resolveCalls()
{
    // ok, we have a stack of pending call/function calls to handle.
    // now that we've got all of the labels scanned off, we can figure out
    // what sort of targets these calls will resolve to.

    size_t count = calls->items();
    for (size_t i = 1;  i <= count; i++)
    {
        RexxInstruction *instruction = (RexxInstruction *)calls->get(i);
        // function calls are expression objects, while CALLs
        // are instructions. Similar, but have different
        // processing methods
        if (isOfClass(FunctionCallTerm, instruction))
        {
            ((RexxExpressionFunction *)instruction)->resolve(labels);
        }
        else
        {
            // ok, technically, this could be either a CALL or a SIGNAL.
            ((RexxInstructionCallBase *)instruction)->resolve(labels);
        }
    }
}


/**
 * Translate a expression of REXX code on a directive (already
 * have an active clause, not not translating a block
 * yet)
 *
 * @param token  The left paren delimiter for the expression. Required for error reporting.
 *
 * @return A translated expression for this directive.
 */
RexxInternalObject *LanguageParser::translateConstantExpression(RexxToken *token, RexxErrorCodes error)
{
    // reset for a fresh translation
    initializeForTranslation();

    // since constants associated with a class are not translated in
    // strictly back-to-back fashion, we need to restore accumulated values
    maxStack = constantMaxStack;
    variableIndex = constantVariableIndex;
    if (constantVariables != OREF_NULL)
    {
        variables = constantVariables;
    }

    // parse out a subexpression, terminating on the end of clause or
    // a right paren (the right paren is actually the required terminator)
    // we get called here because we've already seen the left paren.
    RexxInternalObject *exp = requiredExpression(TERM_RIGHT, error);

    // now copy back the values we need to cache
    constantMaxStack = maxStack;
    constantVariableIndex = variableIndex;
    constantVariables = variables;

    // now verify that the terminator token was a right paren.  If not,
    // issue an error message using the original opening token so we know
    // which one is an issue.
    if (!nextToken()->isRightParen())
    {
        syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
    }
    // protect the expression from GC and return it.
    holdObject(exp);

    // resolve any function calls that might have been used on the constant
    resolveCalls();
    return exp;
}


/**
 * Locate the top block instruction in the control stack.  This
 * involves skipping over any THEN instructions waiting for
 * resolution.
 *
 * @return The top block instruction or OREF_NULL if none are active.
 */
RexxInstruction *LanguageParser::topBlockInstruction()
{
    // iterate through the control stack until we find a real
    // control instruction
    for (size_t i = 1; i <= control->lastIndex(); i++)
    {
        RexxInstruction *inst = (RexxInstruction *)control->get(i);

        // We can have various THEN or ELSE or terminators for those
        // instructions on the stack.  Keep drilling until we find a block
        // instruction type
        if (inst->isBlock())
        {
            return inst;
        }
    }
    // no control instruction found
    return OREF_NULL;
}


/**
 * Resolve dependencies between ::CLASS directives,
 * rearranging the order of the directives to preserve
 * relative ordering wherever possible.  Classes with no
 * dependencies in this source file will be done first,
 * followed by those with dependencies in the appropriate
 * order
 */
void LanguageParser::resolveDependencies()
{
    // if we have classes, here, we need to sort out the install order
    // and configure the source package with this.
    if (!classes->isEmpty())
    {
        // get the count of classes we need to process.
        size_t classCount = classes->items();

        // run through the class list having each directive set up its
        // dependencies
        for (size_t i = 1; i <= classCount; i++)
        {
            ClassDirective *currentClass = (ClassDirective *)classes->get(i);
            currentClass->addDependencies(classDependencies);
        }

        // get a array for handling the ordering
        ArrayClass *classOrder = new_array(classCount);
        ProtectedObject p2(classOrder);

        // now we repeatedly scan the pending directory looking for a class
        // with no in-program dependencies - it's an error if there isn't one
        // as we build the classes we have to remove them (their names) from
        // pending list and from the remaining dependencies
        while (!classes->isEmpty())
        {
            // this is the next one we process
            ClassDirective *nextInstall = OREF_NULL;

            // the count will update each time through.
            classCount = classes->items();
            for (size_t i = 1; i <= classCount; i++)
            {
                // get the next directive
                ClassDirective *currentClass = (ClassDirective *)classes->get(i);
                // if this class doesn't have any additional dependencies, pick it next.
                if (currentClass->dependenciesResolved())
                {
                    nextInstall = currentClass;
                    // add this to the class ordering
                    classOrder->append(nextInstall);
                    // remove this from the processing list
                    classes->deleteItem(i);
                    break;
                }
            }

            // if nothing was located during this pass, we must have circular dependencies
            // this is an error.
            if (nextInstall == OREF_NULL)
            {
                // directive line where we can give as the source of the error
                ClassDirective *errorClass = (ClassDirective *)classes->get(1);
                clauseLocation = errorClass->getLocation();
                syntaxError(Error_Execution_cyclic, name);
            }

            // ok, now go remove these from the dependencies
            RexxString *className = nextInstall->getName();

            // now go through the pending list telling each of the remaining classes that
            // they can remove this dependency from their list

            // the count will update each time through.
            classCount = classes->items();
            for (size_t i = 1; i <= classCount; i++)
            {
                ClassDirective *currentClass = (ClassDirective *)classes->get(i);
                currentClass->removeDependency(className);
            }
        }

        // now add this to the package
        package->classes = classOrder;
        // this requires an install step
        package->setNeedsInstallation();
    }

    // ok, now we need to fill in any additional bits needed by the package
    if (!requires->isEmpty())
    {
        package->requires = requires;
        // this requires an install step
        package->setNeedsInstallation();
    }
    if (!libraries->isEmpty())
    {
        package->libraries = libraries;
        // this requires an install step
        package->setNeedsInstallation();
    }
    if (!routines->isEmpty())
    {
        package->routines = routines;
    }
    if (!publicRoutines->isEmpty())
    {
        package->publicRoutines = publicRoutines;
    }
    if (!unattachedMethods->isEmpty())
    {
        package->unattachedMethods = unattachedMethods;
    }
    if (!resources->isEmpty())
    {
        package->resources = resources;
    }
}


/**
 * Flush an pending instructions from the control stack
 * for a new added instruction.
 *
 * @param instruction
 *               The newly added instruction.
 */
void LanguageParser::flushControl(RexxInstruction *instruction)
{
    // loop through the control stack
    for (;;)
    {
        // get the type of the instruction at the top of the control
        // stack.
        InstructionKeyword type = topDoType();   /* get the instruction type          */
        // is this a pending ELSE clause?    */
        if (type == KEYWORD_ELSE)
        {
            // pop the instruction off of the stack
            RexxInstruction *second = popDo();
            // and create a new end marker for that instruction.
            second = endIfNew((RexxInstructionIf *)second);
            // have an instruction to add?
            if (instruction != OREF_NULL)
            {
                // add to the current location and don't process any additional
                // instructions.
                addClause(instruction);
                instruction = OREF_NULL;
            }
            // now add the else terminator behind this.
            addClause(second);
            // we can go around again on this one.
        }
        // nested IF-THEN situation?
        else if (type == KEYWORD_IFTHEN || type == KEYWORD_WHENTHEN)
        {
            // get the top item
            RexxInstruction *second = popDo();
            // have an instruction to add?
            if (instruction != OREF_NULL)
            {
                // insert this here and null it out.
                addClause(instruction);
                instruction = OREF_NULL;
            }
            // we need a new end marker
            second = endIfNew((RexxInstructionIf *)second);
            // we add this clause behine the new one, and also add this
            // to the control stack as a pending instruction
            addClause(second);
            pushDo(second);

            // we're done with this
            break;
        }
        // some other type of construct.  We just add the instruction to the
        // execution stream
        else
        {
            if (instruction != OREF_NULL)
            {
                addClause(instruction);
            }
            // all done flushing
            break;
        }
    }
}


/**
 * Test if a named variable has been previously
 * exposed.
 *
 * @param varName The variable name.
 *
 * @return True if the variable has been exposed, false otherwise.
 */
bool LanguageParser::isExposed(RexxString *varName)
{
    // if we had an explicit EXPOSE instruction, check if this was listed
    if (exposedVariables != OREF_NULL)
    {
        return exposedVariables->hasIndex(varName);
    }
    // had a USE ARG instruction specified?  Variable is exposed

    else if (localVariables != OREF_NULL)
    {
        return !localVariables->hasIndex(varName);
    }
    // neither situation, not an exposed variable
    return false;
}


/**
 * Perform a variable capture operation if we're
 * evaluating a GUARD WHEN expression.
 *
 * @param varname   The name of the variable
 * @param retriever The associated variable retriever.
 */
void LanguageParser::captureGuardVariable(RexxString *varname, RexxVariableBase *retriever)
{
    // are we collecting guard variable references (usually the case when
    // parsing a GUARD WHEN expression.
    if (capturingGuardVariables())
    {
        // in the context of this method, we have a list of exposed variables.
        // if this is one of the exposed variables, then also add this to the guard
        // variables list so we know which variables will force a re-evaluation.
        if (isExposed(varname))
        {
            // add to the guard list
            guardVariables->put(retriever, retriever);
        }
    }
}


/**
 * Resolve a variable name to a single common retriever object
 * per code block.
 *
 * @param varname The name of the target variable.
 *
 * @return A retrieve object for accessing the variable.  This
 *         will identify a variable slot in the code stack frame.
 */
RexxVariableBase *LanguageParser::addSimpleVariable(RexxString *varname)
{
    // we might have this already (fairly common in most programs).  If
    // not we cache a new one for the next time.
    RexxVariableBase *retriever = (RexxVariableBase *)variables->get(varname);
    // if not in the table yet, we need to create a new one
    if (retriever == OREF_NULL)
    {
        // ok, have to create a new one.

        // if we're in normal operation, we allocate a slot in the stack frame
        // and tie a variable reference to that slot.  If this is an interpret,
        // then the variable value must be resolved dynamically.
        if (!isInterpret())
        {
            variableIndex++;
            retriever = new RexxSimpleVariable(varname, variableIndex);
        }
        else
        {
            // a slot index of zero tells the retriever to perform a dynamic
            // lookup
            retriever = new RexxSimpleVariable(varname, 0);
        }
        // and add this to the table.
        variables->put(retriever, varname);
    }
    // Small optimization here.  In order for a variable to be added to the
    // guard variable list, it must have been exposed already.  That means
    // the variable will already be in this table, so we don't need to
    // perform the capturing test then.
    else
    {
        captureGuardVariable(varname, retriever);
    }
    // return the variable accesser, either a new one or one pulled from the cache.
    return retriever;
}


/**
 * Add a stem variable instance to the execution instance.
 * This is like the addVariable, but the retriever
 * in question is for a Stem variable.
 *
 * @param stemName The name of the stem (including the period).
 *
 * @return A retriever for the specified variable.
 */
RexxStemVariable *LanguageParser::addStem(RexxString *stemName)
{
    // like with normal variables, we might have this already cached.
    RexxStemVariable *retriever = (RexxStemVariable *)(variables->get(stemName));
    if (retriever == OREF_NULL)
    {
        if (!isInterpret())
        {
            // non-interpret uses an allocated stack frame slot
            variableIndex++;
            retriever = new RexxStemVariable(stemName, variableIndex);
        }
        else
        {
            // interpret uses dynamic lookup
            retriever = new RexxStemVariable(stemName, 0);
        }
        variables->put(retriever, stemName);
    }
    // Small optimization here.  In order for a variable to be added to the
    // guard variable list, it must have been exposed already.  That means
    // the variable will already be in this table, so we don't need to
    // perform the capturing test then.
    else
    {
        captureGuardVariable(stemName, retriever);
    }
    return retriever;                    /* return variable accesser          */
}



/**
 * Add a compound variable reference to our variable tables.
 * This is a two part operation...first we make sure the
 * stem variable is added, which uses a specific slot.  Then
 * we use that information to create a retriever for
 * the specific compound variable.
 *
 * @param name   The name of the compound variable.
 *
 * @return A retriever for this variable.
 */
RexxCompoundVariable *LanguageParser::addCompound(RexxString *name)
{
    // we cache compound variables also...see if we've encountered
    // this exact name before.
    RexxCompoundVariable *retriever = (RexxCompoundVariable *)(variables->get(name));
    if (retriever != OREF_NULL)
    {
        return retriever;
    }

    // we need to start out by scanning the name to separate the
    // stem and tail portions of the variable name.
    size_t length = name->getLength();
    const char *_position = name->getStringData();
    const char *start = _position;
    const char *end = _position + length;

    // we know this is a compound, so there must be at least one period.
    while (*_position != '.')
    {
        _position++;
    }

    // create a Rexx string version of the stem name and retrieve this
    // from the
    RexxString *stemName = new_string(start, _position - start + 1);
    RexxStemVariable *stemRetriever = addStem(stemName);

    ProtectedObject p(stemRetriever);

    // now split the tail piece into its component parts so that
    // we can optimize construction of the final tail lookup.
    size_t tailCount = 0;
    do
    {
        // we're here because we just saw a previous period.  that's either the
        // stem variable period or the last tail element we processed.
        // either way, we step past it.  If this period is a trailing one,
        // we'll add a null tail element, which is exactly what we want.
        _position++;
        start = _position;

        // scan for the next period
        while (_position < end)
        {
            if (*_position == '.')         // found the next one?
            {
                break;                      // stop scanning now
            }
            _position++;                   // continue looking
        }

        // we found a tail piece.  Now figure out if this is
        // something that cannot be a variable.  Also make sure
        // this is part of the common string cache.
        RexxString *tail = commonString(new_string(start, _position - start));
        if (!(tail->getLength() == 0 || (*start >= '0' && *start <= '9')))
        {
            // this is a real variable piece.  Add this to the global
            // variable table and push its retriever on to the term stack
            // for safe keeping.
            subTerms->push((addSimpleVariable(tail)));
        }
        else
        {
            // this is a constant string value.  We can evaluate this
            // directly, so push it directly on to the stack.
            subTerms->push(tail);
        }
        // make sure we count the number of tails
        tailCount++;
    } while (_position < end);
    // This will pull the items off of the term stack and populate its internal
    // tail table for lookup processing.
    retriever = new (tailCount) RexxCompoundVariable(stemName, stemRetriever->getIndex(), subTerms, tailCount);
    // add this to the retriever table so that all references to a compound variable with the
    // same full name will resolved to the same retriever.  We're safe just using the
    // variables table for this.
    variables->put(retriever, name);

    // NOTE: compound variables do get get added to the guard list.
    return retriever;
}


/**
 * Add a variable name to the list of variables exposed
 * by a method.
 *
 * @param name   The name of the variable.
 */
void LanguageParser::expose(RexxString *name )
{
    // create the table on the first expose
    if (exposedVariables == OREF_NULL)
    {
        exposedVariables = new_string_table();
    }
    exposedVariables->put(name, name);
}


/**
 * We have a USE LOCAL specified, so turn on the auto expose.
 */
void LanguageParser::autoExpose()
{
    localVariables = new_string_table();
    // add the special local variables to the list
    localVariables->put(GlobalNames::SUPER, GlobalNames::SUPER);
    localVariables->put(GlobalNames::SELF, GlobalNames::SELF);
    localVariables->put(GlobalNames::RC, GlobalNames::RC);
    localVariables->put(GlobalNames::RESULT, GlobalNames::RESULT);
    localVariables->put(GlobalNames::SIGL, GlobalNames::SIGL);

}


/**
 * Add a variable name to the list of explicitly declared to be
 * local.
 *
 * @param name   The name of the variable.
 */
void LanguageParser::localVariable(RexxString *name )
{

    localVariables->put(name, name);
}


/**
 * Compress all string tokens needed by a group of programs into
 * a single, common set of strings.  It is fairly common
 * for a program to have duplicate versions of strings
 * in the form of variable names, etc.  This ensures there
 * is just a single version of these available.
 *
 * @param string The string to add to the set.
 *
 * @return The commonized string.  If this string is already
 *         in the common table, the previously added one will be
 *         returned.
 */
RexxString *LanguageParser::commonString(RexxString *string)
{
    // check the global table first for this value.
    RexxString *result = (RexxString *)strings->get(string);
    // if not in the table, we add this new one, otherwise we
    // return the table version
    if (result != OREF_NULL)
    {
        return result;
    }
    strings->put(string, string);
    return string;
}


/**
 * Handle adding a variable to a program.  This verifies that
 * the token is a valid variable, then creates a retriever that
 * can be used in an expression to obtain the variable value.
 *
 * @param token  The token holding the variable name.
 *
 * @return A retriever object that can be used as an expression
 *         term.
 */
RexxVariableBase *LanguageParser::addVariable(RexxToken *token)
{
    // we first validate that this token represents a valid variable,
    needVariable(token);
    // then create the variable retriever object.
    return (RexxVariableBase *)addText(token);
}


/**
 * Handle adding a variable to a program.  This verifies that
 * the token is a valid variable, then creates a retriever that
 * can be used in an expression to obtain the variable value.
 *
 * @param token  The token holding the variable name.
 *
 * @return A retriever object that can be used as an expression
 *         term.
 */
RexxVariableBase *LanguageParser::requiredVariable(RexxToken *token, const char *keyword)
{
    // first verify that this is a symbol
    if (!token->isSymbol())
    {
        syntaxError(Error_Symbol_expected_after_keyword, new_string(keyword));
    }

    // now process the variable
    return addVariable(token);
}


// generate a retriever for a specific token type.  Note that we
// keep two caches of retrievers here.  There are tokens that true
// literals (strings, dot-variables, etc) and variable text tokens.  The
// literals can be kept in a common set across the entire package and
// reused.  The variables, however, contain information that is unique
// to each code block within the package.  These need to be kept in a
// different list that is rebuilt in each code block.
RexxInternalObject *LanguageParser::addText(RexxToken *token)
{
    // these should be text type tokens that have a real value.
    RexxString *name = token->value();

    // NOTE:  We cannot check the literals table at the beginning without
    // knowing the type of token first.  It is possible for both a literal string
    // and a variable name or dotvariable to have the same value, which could result in the
    // string constant getting used where a real variable retriever is required.

    // now switch on the major token class id.
    switch (token->type())
    {
        // various categories of symbols.
        case TOKEN_SYMBOL:
        {
            // each symbol subtype requires a
            // different retrieval method
            switch (token->subtype())
            {
                // the dummy placeholder period and symbols
                // that start with a digit are just literal strings.
                case SYMBOL_DUMMY:
                case SYMBOL_CONSTANT:
                {
                    // we might already have processed this before.
                    // if not, we need to examine this and find the
                    // most appropriate form.
                    RexxInternalObject *retriever = literals->get(name);
                    if (retriever != OREF_NULL)
                    {
                        return retriever;
                    }

                    RexxInternalObject *value;

                    // if this is a pure integer value within the default
                    // digits, create an integer object
                    if (token->isIntegerConstant())
                    {
                        value = name->requestInteger(Numerics::REXXINTEGER_DIGITS);
                        // this should not happen, given we've already validated
                        // this, but belt and braces and all that...just
                        // stick with the string value if it does occur.
                        if (value == TheNilObject)
                        {
                            value = name;
                        }
                        else
                        {
                            // snip off the string number string
                            // value that was created when the
                            // integer value was created.  This
                            // is rarely used, but contributes
                            // to the saved program size. It will
                            // be rebuilt on demand if it is needed.
                            name->setNumberString(OREF_NULL);
                        }
                    }
                    else
                    {
                        // just use the string value, but also try to create and
                        // attach the string's numeric value.
                        value = name;
                        name->setNumberString(value->numberString());
                    }
                    // and stash the retriever value so we can resolve this if used again.
                    literals->put(value, name);
                    // strings and integers work directly as expression terms, so we can
                    // just return this directly.
                    return value;
                    break;
                }

                // simple variable.
                case SYMBOL_VARIABLE:
                {
                    // do the variable resolution
                    return addSimpleVariable(name);
                    break;
                }

                // stem variable, handled much like simple variables.
                case SYMBOL_STEM:
                {
                    return addStem(name);
                    break;
                }

                // compound variable...need to chop this up into its
                // component pieces.
                case SYMBOL_COMPOUND:
                {
                    return addCompound(name);
                    break;
                }

                // this is a non-numeric symbol that starts with a dot.  These
                // are treated as environment symbols.
                case SYMBOL_DOTSYMBOL:
                {
                    // we might already have processed this before.
                    // if not, we need to examine this and find the
                    // most appropriate form. Note that the constant, non-overridable
                    // variables .nil, .true, and .false were added to the common set
                    // up front, so we don't need to perform special checks here.
                    RexxInternalObject *retriever = dotVariables->get(name);
                    if (retriever != OREF_NULL)
                    {
                        return retriever;
                    }

                    // create the shorter name and add to the common set
                    RexxString *shortName = commonString(name->extract(1, name->getLength() - 1));
                    // create a retriever for this using the shorter name.
                    retriever = new RexxDotVariable(shortName);
                    // we can add this to the literals list, since they do not
                    // depend upon context.
                    dotVariables->put(retriever, name);
                    return retriever;
                    break;
                }

                // invalid symbol subtype (should really never happen)
                default:
                {
                    reportException(Error_Interpretation_switch, "symbol subtype", token->subtype());
                    break;
                }
            }
            break;
        }

        // just a straight literal string
        case TOKEN_LITERAL:
        {
            // we might already have processed this before.
            // if not, we need to examine this and find the
            // most appropriate form.
            RexxInternalObject *retriever = literals->get(name);
            if (retriever != OREF_NULL)
            {
                return retriever;
            }

            // strings are their own expression retrievers, so just add
            // this to the table and return it directly
            literals->put(name,  name);
            return name;
            break;
        }

        // not a token type that can have a retriever
        default:
        {
            break;
        }
    }

    // not a token type that can have a retriever
    return OREF_NULL;
}


/**
 * Build a retriever for a string name.  This will be
 * a version that uses dynamic lookup for variables rather
 * than using pre-assigned slots.  Generally used for
 * dynamic lookups such as from the variable pool interface.
 *
 * @param name   The variable name.
 *
 * @return A retriever object for looking up this value.
 */
RexxVariableBase *LanguageParser::getRetriever(RexxString *name)
{
    // first validate that this is a symbol and get the type of symbol.
    switch (name->isSymbol())
    {
        // simple variable name
        case STRING_NAME:
            return (RexxVariableBase *)new RexxSimpleVariable(name, 0);
            break;

        // stem name.
        case STRING_STEM:
            return (RexxVariableBase *)new RexxStemVariable(name, 0);
            break;

        // compound name...more complicated.
        case STRING_COMPOUND_NAME:
            return (RexxVariableBase *)VariableDictionary::buildCompoundVariable(name, true);
            break;

        default:
            // this is invalid and a syntax error
            syntaxError(Error_Translation_invalid_attribute, name);
    }
    return OREF_NULL;
}



/**
 * Add a new instruction to the code execution chain.  Instructions
 * are a linear chain of instruction objects, with all
 * branching handled by additional links within instructions.
 *
 * @param instruction
 *               The new instruction to add.
 */
void LanguageParser::addClause(RexxInstruction *instruction)
{
    // NOTE:  the first instruction is set manually, so we should ALWAYS
    // have non-null values here.
    lastInstruction->setNext(instruction);
    lastInstruction = instruction;
}


/**
 * Add a label to our global table.  Note, in Rexx it is
 * not an error to have duplicate labels, but ony the
 * first can be used as a target.  We do not overwrite
 * a given name if it is already in the table.
 *
 * @param label     The label instruction to add.
 * @param labelname The label name.
 */
void LanguageParser::addLabel(RexxInstruction *label, RexxString *labelname )
{
    if (!labels->hasIndex(labelname))
    {
        labels->put(label, labelname);
    }
}


/**
 * Locate a label for a name match.
 *
 * @param labelname The label name.
 *
 * @return The label instruction, or OREF_NULL if the label doesn't exist
 */
RexxInstruction *LanguageParser::findLabel(RexxString *labelname)
{
    // it's possible we don't have a label table.
    if (labels != OREF_NULL)
    {
        return(RexxInstruction *)labels->get(labelname);
    }
    return OREF_NULL;
}


/**
 * We're about to process a WHEN expression on a GUARD
 * instruction, so we need to turn on variable capture
 * so we know what exposed variables are referenced in the
 * expression.
 */
void LanguageParser::setGuard()
{
    // create the guard variable table if it is not already created...
    // it would be unusual for this to already exist.
    if (guardVariables == OREF_NULL)
    {
        guardVariables = new_identity_table();
    }
}


/**
 * Complete guard variable collection and return the table of
 * variable names.
 *
 * @return An array of the guard variable names.
 */
ArrayClass *LanguageParser::getGuard()
{
    // get the indices as an array of names.
    ArrayClass *guards = guardVariables->makeArray();
    // turn off collection by tossing the table.
    guardVariables = OREF_NULL;
    return guards;
}

/**
 * Parse out a "constant" expression for a Rexx instruction.
 * a constant expression is a literal string, a constant
 * symbol, or an expression enclosed in parentheses.  Most
 * new instructions use constant expressions, since they
 * are not terminated by instruction subkeywords, and thus
 * more immune to breakage when new options are added.
 *
 * @return An object that can be used to evaluate this option
 *         expression.
 */
RexxInternalObject *LanguageParser::parseConstantExpression()
{
    // everthing keys off of the first token.
    RexxToken *token = nextReal();
    // just a literal token?
    if (token->isLiteral())              /* literal string expression?        */
    {
        // process this as text and return.  This is automatically
        // protected from GC.
        return addText(token);
    }
    // could be a constant symbol like a number or an environment variable
    // like .nil or .true.
    else if (token->isConstant())
    {
        return addText(token);
    }
    // is this missing entirely?
    else if (token->isEndOfClause())
    {
        // we don't raise an error here because we don't know that instruction
        // or context where this is needed.  Just back up one token and return
        // a NULL value.
        previousToken();
        return OREF_NULL;
    }
    // only other option here is an expression in parens.  If this
    // is not there, raise an error (this is a generic error, so
    // we can do this here.
    else if (!token->isLeftParen())
    {
        syntaxError(Error_Invalid_expression_general, token);
    }
    else
    {
        // parse our a subexpression, terminating on the end of clause or
        // a right paren (the right paren is actually the required terminator)
        RexxInternalObject *exp = parseFullSubExpression(TERM_RIGHT);
        // now verify that the terminator token was a right paren.  If not,
        // issue an error message using the original opening token so we know
        // which one is an issue.
        if (!nextToken()->isRightParen())
        {
            syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
        }
        // protect the expression from GC and return it.
        holdObject(exp);
        return exp;
    }
    return OREF_NULL;
}


/**
 * Evaluate a "parenthetical" expression for REXX instruction
 * values.  A parenthetical expression is an expression enclosed
 * in parentheses.
 *
 * @param start  The starting token of the expression.  This will be
 *               the opening paren and is used for error reporting.
 *
 * @return The parsed expression.
 */
RexxInternalObject *LanguageParser::parenExpression(RexxToken *start)
{
    // NB, the opening paren has already been parsed off and is in
    // the start token, which we only need for error reporting.

    RexxInternalObject *expression = parseSubExpression(TERM_RIGHT);

    // this must be terminated by a right paren
    if (!nextToken()->isRightParen())
    {
        syntaxErrorAt(Error_Unmatched_parenthesis_paren, start);
    }

    // protect the expression and return it.
    holdObject(expression);
    return expression;
}


/**
 * Parse off an expression, stopping when one of the possible set
 * of terminator tokens is reached.  The terminator token is
 * placed back on the token queue.
 *
 * @param terminators
 *               An int containing the bit flag versions of the terminators.
 *
 * @return An executable expression object.
 */
RexxInternalObject *LanguageParser::parseExpression(int terminators)
{
    // get the first real token.  This will skip over all of the
    // white space, comments, etc. to get to the real meat of the expression.
    // we then back up so that the real expression processing will see that as
    // the first token.
    nextReal();
    previousToken();
    // and go parse this.  In this context, commas are allow to create
    // list format.
    return parseFullSubExpression(terminators);
}


/**
 * Perform the parsing of an expression where the expression
 * can be treated as a comma-separated list of subexpressions.
 * If we have just a simple single subexpression, the
 * return value is the parsed subexpression.  If a comma
 * is found as a terminator, then we turn this expression
 * into an operator that will create an array object from the
 * list of expressions.  Omitted expressions are allowed and
 * no effort is made to remove trailing null expressions.
 *
 * @param terminators
 *               The list of terminators for this expression type.
 *
 * @return Either a simple expression, or an expression object for
 *         creating an array item.
 */
RexxInternalObject *LanguageParser::parseFullSubExpression(int terminators)
{
    size_t total = 0;                // total is the full count of arguments we attempt to parse.
    RexxToken *terminatorToken;      // the terminator token that ended a sub expression

    // now loop until we get a terminator.  Note that COMMAs are always a terminator
    // token now that list expressions are possible.
    for (;;)
    {
        // parse off an argument expression
        RexxInternalObject *subExpr = parseSubExpression(terminators);
        // We have two term stacks.  The main term stack is used for expression evaluation.
        // the subTerm stack is used for processing expression lists like this.
        // NOTE that we need to use pushSubTerm here so that the required expression stack
        // calculation comes out right.
        pushSubTerm(subExpr);

        // now check the total.  Real count will be the last
        // expression that requires evaluation.
        total++;

        // the next token will be our terminator.  If this is not
        // a comma, we have more expressions to parse.
        terminatorToken = nextToken();
        if (!terminatorToken->isComma())
        {
            // push this token back and stop parsing
            previousToken();
            break;
        }
    }

    // if we only saw the single expression, then return that expression
    // as the result
    if (total == 1)
    {
        return popSubTerm();
    }

    // we have an array creation list, so create the operator type for
    // building the array.
    return new (total) RexxExpressionList(total, subTerms);
}


/**
 * Parse off a sub- expression, stopping when one of the possible
 * set of terminator tokens is reached.  The terminator token is
 * placed back on the token queue.  This is generally
 * called in the context of parsing different bits of
 * an expression and is frequently called recursively.  Note
 * that this subexpression parse will always terminate on a
 * comma...the calling context will then determine how that
 * comma is processed.
 *
 * @param terminators
 *               an int containing flags that indicate what terminators
 *               should be used to stop parsing this subexpression.
 *
 * @return
 */
RexxInternalObject *LanguageParser::parseSubExpression(int terminators )
{
    // generally, expressions proceed with term-operator-term, with various modifications.
    // start by processing of a term value.
    RexxInternalObject *left = parseMessageSubterm(terminators);
    RexxInternalObject *right = OREF_NULL;

    // hmmm, nothing found, so we're done.
    if (left == OREF_NULL)
    {
        return OREF_NULL;
    }

    // we keep both a stack of expression terms and one of operators and process
    // things based on precedence.

    // pus the term to the stack
    pushTerm(left);

    // add a fence value to the operator stack to wall us off from other
    // active subexpressions.
    pushOperator((RexxToken *)TheNilObject);
    // now see what is next.
    RexxToken *token = nextToken();
    // loop until we find a terminator in our current token
    while (!token->isTerminator(terminators))
    {
        switch (token->type())
        {
            // either a single or double message send operation.
            // this uses a term from the stack as a target, and can possibly
            // have an argument list attached.
            case  TOKEN_TILDE:
            case  TOKEN_DTILDE:
            {
                // there must be a left term here to act as the target
                left = requiredTerm(token);

                // create a message term from the target...this also figures out
                // the message name and any arguments.
                RexxInternalObject *subexpression = parseMessage(left, token->isType(TOKEN_DTILDE), terminators);
                // that goes back on the term stack
                pushTerm(subexpression);
                break;
            }

            // a left square bracket.  This will turn into a "[]" message.
            case  TOKEN_SQLEFT:
            {
                // the term is required
                left = requiredTerm(token);
                // this is a message to the left term, and may (ok, probably) have
                // arguments as well
                RexxInternalObject *subexpression = parseCollectionMessage(token, left);
                // and this goes back on to the term stack
                pushTerm(subexpression);
                break;
            }

            // we have a symbol, which may be a variable, a literal string, or
            // an open paren, which may be the start of a parenthetical sub-expression
            // because we're looking for an operator at this point, this is actually
            // an implied abuttal operation.
            case  TOKEN_SYMBOL:
            case  TOKEN_LITERAL:
            case  TOKEN_LEFT:
            {
                // get the token location.  We're going to turn this into a zero-length
                // token for the abuttal operation.
                SourceLocation location = token->getLocation();
                location.setEnd(location.getLineNumber(), location.getOffset());
                // create a dummy token, push the abuttal token back on the queue, and fall
                // through to the next section where the blank concatenate operator is handled.
                token = new RexxToken (TOKEN_OPERATOR, location, OPERATOR_ABUTTAL, GlobalNames::NULLSTRING);
                previousToken();
            }
            // NOTE:  The above section falls through to this piece

            // A blank within an expression.  This is a concatenate (although, if we've
            // fallen through from the section above, this is really an abuttal operation)
            case  TOKEN_BLANK:
            {
                // this potentially could be a blank before one of the terminators (for example,
                // a blank before a WHILE keyword on a LOOP.  This is the same with abuttal.
                // We need to check for the terminators, and if we get a hit, we're done here.
                 RexxToken *second = nextReal();

                 // back up to the operator token and fall through to the operator logic.
                 // this will then sort things out based on the sub type of the token.
                 // we need to go to the previous token even if the terminator test is true.
                 previousToken();
                 // not a real operator if adjacent to a terminator
                 if (second->isTerminator(terminators))
                 {
                     break;
                 }
            }

            // we have an operator of some sort (including the blank and abuttal concatenates)
            // At this point, these will be dyadic operators, so we need to screen out the
            // the not operator, which can only be a prefix one.
            case  TOKEN_OPERATOR:
            {
                // check for the backslash, which is a problem here.
                if (token->isSubtype(OPERATOR_BACKSLASH))
                {
                    syntaxError(Error_Invalid_expression_general, token);
                }

                // we now need to handle operator precedence with anything we have on the stack
                for (;;)
                {
                    // get the top of the operator stack.  If we hit the fence, we've
                    // unwound any pending operations to the beginning, and we can quit this
                    // loop.
                    RexxToken *second = topOperator();
                    if (second == (RexxToken *)TheNilObject)
                    {
                        break;
                    }

                    // check the token precedence,  If our new operator has higher
                    // precedenc, we're done popping.
                    if (token->precedence() > second->precedence())
                    {
                        break;
                    }

                    // these are both required terms here.
                    right = requiredTerm(token);
                    left = requiredTerm(token);

                    // pop off the top operator, and push a new term on the stack to
                    // replace this.
                    RexxToken *op = popOperator();
                    pushTerm(new RexxBinaryOperator(op->subtype(), left, right));
                }

                // finished popping lower precedence items.  Now push this operator on
                // to the stack.  We need to evaluate its right-hand side before this can be handled.
                pushOperator(token);
                // evaluate a right-hand side to this expression.
                right = parseMessageSubterm(terminators);
                // Ok, we need a right term here, unless this is actually a blank operator.
                // in theory, we've already handled that possibility, but it doesn't hurt
                // to check.
                if (right == OREF_NULL && !token->isSubtype(OPERATOR_BLANK))
                {
                    syntaxError(Error_Invalid_expression_general, token);
                }
                // add the term to the stack.  We can handle this once the
                // operator gets popped off of the stack.
                pushTerm(right);
                break;
            }

            // this one of the assignment operators, out of location.  This is just
            // reported as an error here.
            case TOKEN_ASSIGNMENT:
            {
                syntaxError(Error_Invalid_expression_general, token);
                break;
            }

            // We've encountered a right paren.  Like the comma, if this is
            // expected, it acts as a terminator.  This is a stray and must
            // be reported.
            case TOKEN_RIGHT:
            {
                syntaxError(Error_Unexpected_comma_paren);
                break;
            }

            // another good character in a bad location.
            case TOKEN_SQRIGHT:
            {
                syntaxError(Error_Unexpected_comma_bracket);
                break;
            }

            // something unexpected that we have no specific error for.
            default:
            {
                syntaxError(Error_Invalid_expression_general, token);
                break;
            }
        }

        // ok, grab the next token and keep processing.
        token = nextToken();
    }

    // step back so the terminator is the next token.
    previousToken();

    // we've hit a terminator for this subexpression.  We probably
    // have some pending operators on the operator stack that we need
    // to handle.  These will be in proper precedence order, so we just
    // need to pop of the operators and terms and handle them.
    token = popOperator();

    // keep popping until we hit our fence value
    while (token != (RexxToken *)TheNilObject)
    {
        // we need two terms
        right = requiredTerm(token);
        left = requiredTerm(token);

        // all of these operators are binaries, and get pushed back on the stack
        pushTerm(new RexxBinaryOperator(token->subtype(), left, right));
        // and pop another operator.
        token = popOperator();
    }

    // our final subexpression is at the top of the term stack, so pop it
    // off and return.
    return popTerm();
}


/**
 * Parse off multiple arguments for a some sort of
 * call, function, or method invocation, returning them
 * as an array.
 *
 * @param firstToken The first token of the expresssion.
 * @param terminators
 *                   The set of terminators that will end the list.
 *
 * @return An array object holding the argument expressions.
 */
ArrayClass *LanguageParser::parseArgArray(RexxToken *firstToken, int terminators )
{
    // scan a set of arguments until we hit our terminator (likely to be
    // a ')' or ']'.  Arguments are delimited by ','
    size_t argCount = parseArgList(firstToken, terminators);

    // The arguments are pushed on to the term stack.  We need to allocate
    // an array and copy them into the array
    ArrayClass *args = new_array(argCount);

    // arguments are pushed on to the term stack in reverse order, so
    // we fill from the end.  Note that missing arguments are
    // just pushed on to the stack as OREF_NULL.
    while (argCount > 0)
    {
        args->put(subTerms->pop(), argCount--);
    }

    return args;
}


/**
 * Perform the parsing of a list of arguments where each
 * argument is separated by commas.  Each argument expression
 * is pushed on to the term stack in last-to-first order,
 * with omitted arguments represented by OREF_NULL.
 *
 * @param firstToken The first token of the expression list, which is
 *                   really the delimiter that opens the list.
 * @param terminators
 *                   The list of terminators for this expression type.
 *
 * @return The count of argument expressions we found.  Note that
 *         an empty expression following a final "," does not
 *         count as a real argument.
 */
size_t LanguageParser::parseArgList(RexxToken *firstToken, int terminators )
{
    size_t realcount = 0;            // real count is arguments up to the last real one
    size_t total = 0;                // total is the full count of arguments we attempt to parse.
    RexxToken *terminatorToken;      // the terminator token that ended a sub expression

    // we need to skip ahead to the first real token, then backup one to be
    // properly positioned for the start.  If this is a CALL instruction, this
    // will skip the blank between the CALL keyword and the start of the expression,
    // which counts as a significant blank by the tokenizer.
    nextReal();
    previousToken();

    // now loop until we get a terminator.  COMMAs are always subexpression terminators,
    // so we don't need to add anything additional to our terminator lists.  We just
    // interpret them differently in this context.
    for (;;)
    {
        // parse off an argument expression
        RexxInternalObject *subExpr = parseSubExpression(terminators);
        // We have two term stacks.  The main term stack is used for expression evaluation.
        // the subTerm stack is used for processing expression lists like this.
        // NOTE that we need to use pushSubTerm here so that the required expression stack
        // calculation comes out right.
        pushSubTerm(subExpr);

        // now check the total.  Real count will be the last
        // expression that requires evaluation.
        total++;
        // if we got something back, this is our new real total.
        if (subExpr != OREF_NULL)
        {
            realcount = total;
        }

        // the next token will be our terminator.  If this is not
        // a comma, we have more expressions to parse.
        terminatorToken = nextToken();
        if (!terminatorToken->isComma())
        {
            break;
        }
    }


    // if this is a function or method invocation, we're expecting this list to be
    // ended by a right parent.  firstToken gives the position of the missing
    // left paren.
    if (terminators & TERM_RIGHT && !terminatorToken->isRightParen())
    {
        syntaxErrorAt(Error_Unmatched_parenthesis_paren, firstToken);
    }

    // same deal with square brackets, different error message.
    if (terminators&TERM_SQRIGHT && !terminatorToken->isRightBracket())
    {
        syntaxErrorAt(Error_Unmatched_parenthesis_square, firstToken);
    }

    // all arguments are on the subterm stack now.  If we had trailing
    // commas on the list, pop off any of the extras
    while (total > realcount)
    {
        subTerms->pop();
        total--;
    }
    // return the count of real argument expressions.
    return realcount;
}


/**
 * Perform the parsing of a list of expressions for a SELECT
 * CASE WHEN expression.  This is a list of one or more
 * expressions separated by commas.  Omitted expressions are not
 * allowed.  Each argument expression is pushed on to the term
 * stack in last-to-first order.
 *
 * @param firstToken The first token of the expression list, which is
 *                   really the delimiter that opens the list.
 * @param terminators
 *                   The list of terminators for this expression type.
 *
 * @return The count of argument expressions we found.
 */
size_t LanguageParser::parseCaseWhenList(int terminators )
{
    size_t total = 0;                // total is the full count of arguments we attempt to parse.

    // we need to skip ahead to the first real token, then backup one to be
    // properly positioned for the start.  If this is a CALL instruction, this
    // will skip the blank between the CALL keyword and the start of the expression,
    // which counts as a significant blank by the tokenizer.
    nextReal();
    previousToken();

    // now loop until we get a terminator.  COMMAs are always subexpression terminators,
    // so we don't need to add anything additional to our terminator lists.  We just
    // interpret them differently in this context.
    for (;;)
    {
        // parse off an argument expression
        RexxInternalObject *subExpr = parseSubExpression(terminators);
        // all sub expressions are required here.
        if (subExpr == OREF_NULL)
        {
            syntaxError(Error_Invalid_expression_case_when_list);
        }

        // We have two term stacks.  The main term stack is used for expression evaluation.
        // the subTerm stack is used for processing expression lists like this.
        // NOTE that we need to use pushSubTerm here so that the required expression stack
        // calculation comes out right.
        pushSubTerm(subExpr);

        // now check the total.
        total++;

        // the next token will be our terminator.  If this is not
        // a comma, we have more expressions to parse.
        RexxToken *terminatorToken = nextToken();
        if (!terminatorToken->isComma())
        {
            // put the terminator back on
            previousToken();
            break;
        }
    }

    // return the count of expressions.
    return total;
}


/**
 * Parse off a function call and create an expression
 * object that can process the call.
 *
 * @param token  The token that marks the start of the argument list.
 * @param name   The function name token.
 *
 * @return A function expression object that can invoke this function.
 */
RexxInternalObject *LanguageParser::parseFunction(RexxToken *token, RexxToken *name)
{
    // parse off the argument list, leaving the arguments in the subterm stack.
    // NOTE:  Because we have a closed () construct delimiting the function arguments,
    // we can ignore any terminators specified from the parent context.
    size_t argCount = parseArgList(token, (TERM_RIGHT));

    // create a function item.  This will also pull the argument items from the
    // subterm stack
    RexxInternalObject *func = new (argCount) RexxExpressionFunction(name->value(), argCount,
        subTerms, name->builtin());

    // at this point, we can't resolve the final target of this call.  It could be
    // a builtin, a call to an internal label, or an external call.  We'll resolve this
    // once we've finished this code block and have all of the labels.  But note that
    // if this was specified as a literal, we can skip the resolution step since
    // this can never resolve to an internal label.
    if (!name->isLiteral())
    {
        addReference(func);
    }
    return func;
}


/**
 * Parse off a qualified symbol.  This can either be a class
 * lookup or a qualified function call.
 * the call.
 *
 * @param n
 *
 * @return An expression object that can process this qualified lookup
 *         type.
 */
RexxInternalObject *LanguageParser::parseQualifiedSymbol(RexxString *namespaceName)
{
    RexxToken *token = nextToken();

    if (!token->isSymbol())
    {
        syntaxError(Error_Symbol_expected_qualified_symbol);
    }

    // get the qualified name
    RexxString *qualifiedName = token->value();

    // step to the next immediate token.  If this is left paren, then
    // this is a qualified function call
    token = nextToken();
    if (token->isLeftParen())
    {
        // parse off the argument list, leaving the arguments in the subterm stack.
        // NOTE:  Because we have a closed () construct delimiting the function arguments,
        // we can ignore any terminators specified from the parent context.
        size_t argCount = parseArgList(token, (TERM_RIGHT));

        // create a function item.  This will also pull the argument items from the
        // subterm stack
        return new (argCount) QualifiedFunction(namespaceName, qualifiedName, argCount, subTerms);
    }
    // this is a qualified class lookup
    else
    {
        // need to push the following token back...it is something else.
        previousToken();
        return new ClassResolver(namespaceName, qualifiedName);
    }
}


/**
 * Parse a collection message expression term.  This is
 * if the form term[args], and follows much the same
 * parsing rules as function calls.
 *
 * @param token  The token that starts the argument list (the square bracket)
 * @param target The target for the message term.
 * @param terminators
 *               Our terminators.
 *
 * @return A message expression object.
 */
RexxInternalObject *LanguageParser::parseCollectionMessage(RexxToken *token, RexxInternalObject *target)
{
    // this was popped from the term stack, so we need to give it a little protection
    // until we're done parsing.
    ProtectedObject p(target);

    // get the arguments.  Like with builtin function calls, we just ignore any
    // prior terminator context and rely on the fact that the brackes must match.
    size_t argCount = parseArgList(token, (TERM_SQRIGHT));

    // create the message item.
    RexxInternalObject *msg = new (argCount) RexxExpressionMessage(target, GlobalNames::BRACKETS,
        OREF_NULL, argCount, subTerms, false);
    // give this a little short-term GC protection.
    holdObject(msg);
    return msg;
}


/**
 * Get a token from the clause, combining terminator checks
 * and error reporting if we don't get a terminator.
 *
 * @param terminators
 *                  The terminators for the current context.
 * @param errorcode The error to issue if no token is found.  The error code
 *                  is optional, and if not specified, OREF_NULL is returned.
 *
 * @return The parsed off token.
 */
RexxToken  *LanguageParser::getToken(int terminators, RexxErrorCodes errorcode)
{
    // get the next token and perform the terminator checks.  If we were
    // terminated, issue an error message if requested.
    RexxToken *token = nextToken();
    if (token->isTerminator(terminators))
    {
        if (errorcode != 0)
        {
            syntaxError(errorcode);
        }
        // just return a null if not requested to issue an error.
        previousToken();
        return OREF_NULL;
    }
    return token;
}


/**
 * Parse a message send term within an expression.
 *
 * @param target The target object term for the message send.
 * @param doubleTilde
 *               Indicates whether this is the "~" or "~~" form of operatior.
 * @param terminators
 *               Expression terminators.
 *
 * @return An object to execute the message send.
 */
RexxInternalObject *LanguageParser::parseMessage(RexxInternalObject *target, bool doubleTilde, int terminators )
{
    // this message might have a superclass override...default is none.
    RexxInternalObject *super = OREF_NULL;
    // no arguments yet
    size_t argCount = 0;

    // add the term to the term stack so that stacksize calculations
    // include this in the processing.  This has the side effect of
    // protecting this object from GC while we're parsing.
    pushTerm(target);
    // ok, we're expecting a message name next, go get one.
    RexxToken *token = getToken(terminators, Error_Symbol_or_string_tilde);
    // this must be a string or symbol
    if (!token->isSymbolOrLiteral())
    {
        syntaxError(Error_Symbol_or_string_tilde);
    }

    // we have a message name
    RexxString *messagename = token->value();
    // upper case this here and add to the common pool.
    messagename = commonString(messagename->upper());

    // ok, what else do we have.  Nothing additional is required
    // here, but this could be an argument list, or a superclass
    // override.
    token = getToken(terminators);
    if (token != OREF_NULL)
    {
        // an override is marked with a colon.
        if (token->isType(TOKEN_COLON))
        {
            // ok, if we have the colon, then we need an override spec.
            token = getToken(terminators, Error_Symbol_expected_colon);
            // this must be a variable or a dot symbol.
            if (!token->isVariableOrDot())
            {
                syntaxError(Error_Symbol_expected_colon);
            }

            // the super class is either a variable lookup or a dot variable, so
            // we need a retriever.
            super = addText(token);
            // get the next token to check for an argument list.
            token = getToken(terminators);
            // NOTE:  We fall through from here to the code
            // that follows the checks for the superclass override to
            // the code that checks for the argument list.
        }
    }

    // ok, we might have an argument list here.
    if (token != OREF_NULL)
    {
        // this is only interesting if the text token is an open paren directly
        // abutted to the name.  Process that as an argument list for the final
        // created message object
        if (token->isLeftParen())
        {
            // NOTE:  because of the parens, we can ignore our parent terminators...
            // we only look for the right paren
            argCount = parseArgList(token, TERM_RIGHT);
        }
        else
        {
            previousToken();                 /* something else, step back         */
        }
    }

    // got all of the pieces, now create the message object and give it some short term
    // protection from GC.
    RexxInternalObject *msg =  new (argCount) RexxExpressionMessage(target, messagename, super, argCount, subTerms, doubleTilde);
    holdObject(msg);
    // now safe to pop the message target object we saved.
    popTerm();
    return msg;
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
RexxInternalObject *LanguageParser::parseVariableOrMessageTerm()
{
    // try for a message term first.  If not successful, see if the
    // next token is a variable symbol.
    RexxInternalObject *result = parseMessageTerm();
    if (result == OREF_NULL)
    {
        RexxToken *_first = nextReal();
        if (_first->isSymbol())
        {
            // ok, add the variable to the processing list
            needVariable(_first);
            result = addText(_first);
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



/**
 * Parse off an instruction leading message term element. This
 * is an important step, as it is used to determine what sort of
 * instruction we have.  If the instruction starts with a
 * message term, then this is a messaging instruction.  Part of
 * the instruction type determination at the beginning of each
 * clause.
 *
 * @return The message term execution element.
 */
RexxInternalObject *LanguageParser::parseMessageTerm()
{
    // save the current position so we can reset cleanly
    size_t mark = markPosition();

    // The straight forward approach is to parse off the first
    // sub term and then see if this is followed by a messaging
    // operator.  Unfortunately, there's a problem hidden within
    // that approach.  Parsing off the first term results in an
    // symbols that are located getting handled as variable references.
    // This means that for all keyword instructions, we're allocating a variable
    // slot and creating a variable retriever for all keyword instruction names
    // used within a code block.  And this happens with every code block (e.g.,
    // ::method or ::routine section) in the program.  So, we will attempt to
    // eliminate this particular situation up front just be examining the
    // tokens.

    // get the first token and make sure we really have something here.
    // The caller knows how to deal with a missing term.
    RexxToken *token = nextToken();
    if (token->isTerminator(TERM_EOC))
    {
        // push the terminator back
        previousToken();
        return OREF_NULL;
    }

    // this problem only occurs if the first token is a simple variable token.
    if (token->isSimpleVariable())
    {
        RexxToken *second = nextToken();
        // we can go ahead and reset...we need to do this regardless of the
        // path we take.
        resetPosition(mark);

        // if the first token is a symbol that is followed by a
        // message operator token (~, ~~, or [), this is a message term.
        // if the next token is a "(", this is "potentially" a function call
        // that is a message term.  But in that case, it will not be handled
        // as a variable.  For any other case, we reject this as a message term
        // and return NULL.
        if (!second->isMessageOperator() & !second->isLeftParen())
        {
            return OREF_NULL;
        }
        // fall through and do the generalized parsing process from here.
    }
    // if not a simple symbol, back up to the first token again
    else
    {
        resetPosition(mark);
    }

    // get the first message term
    RexxInternalObject *start = parseSubTerm(TERM_EOC);
    // save this on the term stack
    pushTerm(start);

    RexxInternalObject *term = OREF_NULL;         // an allocated message term
    token = nextToken();

    // the leading message term can be a cascade of messages, so
    // keep processing things until we hit some other type of operation.
    while (token->isMessageOperator())
    {
        // we need to perform the the message operation on this term to create
        // an new term which will be the target of the next one.
        // this could be a bracket lookup, which is a collection message.
        if (token->isLeftBracket())
        {
            term = parseCollectionMessage(token, start);
        }
        else
        {
            // normal message twiddle message
            term = parseMessage(start, token->isType(TOKEN_DTILDE), TERM_EOC);
        }
        // this is the target for the next one in the chain.
        // remove the previous one from the term stack and push a new
        // one on.
        start = term;
        popTerm();
        pushTerm(start);
        // and see if we have another.
        token = nextToken();
    }

    // found a token that was not a message send type, so push it back.
    previousToken();
    // if we never created a message term from this, we need to take another
    // path.  Roll the position back to where we started and return.
    if (term == OREF_NULL)
    {
        resetPosition(mark);
    }
    // we still have either the starting term or our last created message
    // term on the stack...we can remove this now.
    popTerm();

    // ok, this is either OREF_NULL to indicate we found nothing,
    // or the last message term we ended up creating.
    return term;
}


/**
 * Handle a potential message subterm in the context of an
 * expression.  This is similar to messageTerm(), but this
 * occurs after we've already determined we need an expression
 * and need an expression term.
 *
 * @param terminators
 *               Terminators for this subexpression context.
 *
 * @return An object to represent this message term.  Returns
 *         OREF_NULL if no suitable term is found.
 */
RexxInternalObject* LanguageParser::parseMessageSubterm(int terminators)
{
    // with very complex instructions, it is possible to recurse quite deeply here.
    // so give a look at the stack space on each call so we can terminate "nicely"
    ActivityManager::currentActivity->checkStackSpace();
    // get the first token.  If we've hit a terminator here, this could be
    // the real end of the expression.  The caller context will figure out
    // how to handle that.
    RexxToken *token = nextToken();

    if (token->isTerminator(terminators))
    {
        // need to push the terminator back
        previousToken();
        return OREF_NULL;
    }

    // is this an operator?  I this context, it will need to be a prefix operator.
    if (token->isOperator())
    {
        // prefix operators are handled as expression terms, although they themselves
        // require a term.
        switch (token->subtype())
        {
            // we only recognize 3 prefix operators, and they are handled in pretty
            // much the same way.
            case OPERATOR_PLUS:
            case OPERATOR_SUBTRACT:
            case OPERATOR_BACKSLASH:
            {
                // we need a term as a target. this is an error
                // if we don't find anything we can use.  We just recurse
                // on this, which might find a chain of prefix operators (except,
                // as I'm sure Walter Pachl will be sure to point out, --, which
                // will be treated as a line comment :-)
                RexxInternalObject *term = parseMessageSubterm(terminators);
                if (term == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_prefix, token);
                }
                // create a new unary operator using the subtype code.
                return new RexxUnaryOperator(token->subtype(), term);
                break;
            }
                // not a aperator in the normal sense, but > or as a prefix creates
                // a variable reference.
            case OPERATOR_LESSTHAN:
            case OPERATOR_GREATERTHAN:
            {
                // parse off the term
                return parseVariableReferenceTerm();
            }

                // other operators are invalid
            default:
                syntaxError(Error_Invalid_expression_general, token);
        }
    }

    // ok, not an operator character.  Now we need to figure out what sort of
    // term element is here.
    else
    {
        // put the token back and try parsing off a subTerm (smaller subset, no
        // prefix stuff allowed).
        previousToken();
        RexxInternalObject *term = parseSubTerm(terminators);
        // protect on the term stack
        pushTerm(term);
        // ok, now see if this is actually a message send target by checking
        // the next token
        token = nextToken();

        // we can have a long cascade of message sends.  For expression syntax,
        // this all one term (just like nested function calls would be).
        while (token->isMessageOperator())
        {
            // we have two possibilities here, a bracket message or a twiddle form.
            if (token->isLeftBracket())
            {
                term = parseCollectionMessage(token, term);
            }
            else
            {
                term = parseMessage(term, token->isType(TOKEN_DTILDE), terminators);
            }
            popTerm();
            pushTerm(term);
            // message cascades are considered part of the same expression term.
            token = nextToken();
        }
        // back up to the token that stopped the loop
        previousToken();
        // pop our term from the stack and return the final version.
        popTerm();
        return term;
    }
    return OREF_NULL;     // should never get here.
}


/**
 * Parse off a variable reference term.
 *
 * @return A variable reference term object
 */
RexxInternalObject* LanguageParser::parseVariableReferenceTerm()
{
    // this must be either a simple variable or a stem.
    RexxToken *token = nextReal();
    if (!token->isSymbol() || !token->isNonCompoundVariable())
    {
        syntaxError(Error_Symbol_expected_after_prefix_reference, token);
    }
    RexxVariableBase *retriever = OREF_NULL;

    if (token->isSimpleVariable())
    {
        retriever = addSimpleVariable(token->value());
    }
    else
    {
        retriever = addStem(token->value());
    }

    // create a new expression term to retrieve the variable
    return new VariableReferenceOp(retriever);
}


/**
 * Parse off a subterm of an expression, from simple ones like
 * variable names and literals, to more complex such as message
 * sends.
 *
 * @param terminators
 *               The expression terminator context.
 *
 * @return An executable object for the message term.
 */
RexxInternalObject* LanguageParser::parseSubTerm(int terminators)
{
    // with very complex instructions, it is possible to recurse quite deeply here.
    // so give a look at the stack space on each call so we can terminate "nicely"
    ActivityManager::currentActivity->checkStackSpace();

    // get the first token and make sure we really have something here.
    // The caller knows how to deal with a missing term.
    RexxToken *token = nextToken();
    if (token->isTerminator(terminators))
    {
        // push the terminator back
        previousToken();
        return OREF_NULL;
    }


    // ok, process based on the token category
    switch (token->type())
    {
        // parenthetical subexpression.  Note, we do not pass along our
        // terminators when requesting this, since the bracketing within the parens
        // will disambiguate keywords.  We only look for terminators and a right paren
        case  TOKEN_LEFT:
        {
            // parse off the parenthetical.  This might not return anything if there
            // is nothing in the parens.  This is an error.  Also, in this context,
            // we are back in a mode where the array-creation syntax is allowed.
            RexxInternalObject *term = parseFullSubExpression(TERM_RIGHT);
            if (term == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_general, token);
            }
            // this had better been terminated by a righ paren.
            if (!nextToken()->isRightParen())
            {
                syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
            }
            // we're done.
            return term;
        }

            // a symbol.  These are generally pretty simple, but
            // we also have to account for function calls or qualified lookups
        case  TOKEN_SYMBOL:
        {
            // need to check if the next token is an open paren.  That turns
            // the symbol or literal token into a function invocation.
            RexxToken *second = nextToken();
            if (second->isLeftParen())
            {
                return parseFunction(second, token);
            }
            // either a qualified symbol lookup or a potential
            // qualified function.
            else if (second->isType(TOKEN_COLON))
            {
                return parseQualifiedSymbol(token->value());
            }
            else
            {
                // simple text token type...push the next token back
                // and go resolve how the token is handled.
                previousToken();
                return addText(token);
            }
            break;
        }

            // a literal.  These are generally pretty simple, but
            // we also have to account for function calls.
        case  TOKEN_LITERAL:
        {
            // need to check if the next token is an open paren.  That turns
            // the symbol or literal token into a function invocation.
            RexxToken *second = nextToken();
            if (second->isLeftParen())
            {
                return parseFunction(second, token);
            }
            else
            {
                // simple text token type...push the next token back
                // and go resolve how the token is handled.
                previousToken();
                return addText(token);
            }
            break;
        }

            // an operator token.  We do allow prefix operators here, others are
            // an error
        case  TOKEN_OPERATOR:
        {
            switch (token->subtype())
            {
                // +, -, and logical NOT variants are permitted here...except
                // we don't actually process them here, so back up and say we got nothing.
                case OPERATOR_PLUS:
                case OPERATOR_SUBTRACT:
                case OPERATOR_BACKSLASH:
                    previousToken();
                    return OREF_NULL;

                // a prefix '>' or '<' is a variable reference
                case OPERATOR_LESSTHAN:
                case OPERATOR_GREATERTHAN:
                {
                    // parse off the term
                    return parseVariableReferenceTerm();
                }

                // other operators we can flag as an error now.
                default:
                    syntaxError(Error_Invalid_expression_general, token);
            }
            break;
        }

        // a few error situations with specific error messages.
        case  TOKEN_RIGHT:
            syntaxError(Error_Unexpected_comma_paren);
            break;

        case  TOKEN_COMMA:
            syntaxError(Error_Unexpected_comma_comma);
            break;

        case  TOKEN_SQRIGHT:
            syntaxError(Error_Unexpected_comma_bracket);
            break;

        // ok, this is bad
        default:
            syntaxError(Error_Invalid_expression_general, token);
    }
    // never reach here.
    return OREF_NULL;
}


/**
 * Push a term on to the expression term stack.
 *
 * @param term   The term object.
 */
void LanguageParser::pushTerm(RexxInternalObject *term )
{
    // push the term on to the stack.
    terms->push(term);

    // we keep track of how large the term stack gets during parsing.  This
    // tells us how much stack space we need to allocate at run time.
    currentStack++;
    maxStack = std::max(currentStack, maxStack);
}


/**
 * Pop a term off of the expression stack.
 *
 * @return The popped object.
 */
RexxInternalObject *LanguageParser::popTerm()
{
    // reduce the stack count
    currentStack--;
    // pop the object off of the stack and give it some short-term
    // GC protection.
    RexxInternalObject *term = terms->pop();
    holdObject(term);
    return term;
}


/**
 * Push a term on to the expression sub term stack.  The
 * subterms normally contribute to the total required stack
 * size, so make sure we account for these when calculating the
 * total required stack size.  Only use this method of pushing
 * the term when the max stack size is affected.
 *
 * @param term   The term object.
 */
void LanguageParser::pushSubTerm(RexxInternalObject *term )
{
    // push the term on to the stack.
    subTerms->push(term);

    // we keep track of how large the term stack gets during parsing.  This
    // tells us how much stack space we need to allocate at run time.
    currentStack++;
    maxStack = std::max(currentStack, maxStack);
}


/**
 * Pop a term off of the expression sub term stack.
 *
 * @return The popped object.
 */
RexxInternalObject *LanguageParser::popSubTerm()
{
    // reduce the stack count
    currentStack--;
    // pop the object off of the stack and give it some short-term
    // GC protection.
    RexxInternalObject *term = subTerms->pop();
    holdObject(term);
    return term;
}


/**
 * Pop a term item off of the execution stack.  Will
 * raise an error if this is not there.
 *
 * @param token     The token giving the context location.  Used to
 *                  identify the error position.
 * @param errorCode The error code.  The default is the generic syntax error.
 *
 * @return The object popped off of the stack.
 */
RexxInternalObject *LanguageParser::requiredTerm(RexxToken *token, RexxErrorCodes errorCode)
{
    // we track the size count when we push/pop
    currentStack--;
    // pop the term off of the stack
    RexxInternalObject *term = terms->pop();
    // we need a term, if this is not here, this is a syntax error
    if (term == OREF_NULL)
    {
        syntaxError(errorCode, token);
    }
    // give this term some short-term GC protection and return it.
    holdObject(term);
    return term;
}

/**
 * Pop multiple items off of the term stack.
 *
 * @param count  The count of items to pop.
 *
 * @return The last object popped.
 */
RexxInternalObject *LanguageParser::popNTerms(size_t count)
{
    RexxInternalObject *result = OREF_NULL;

    // reduce the stack size
    currentStack -= count;
    // and pop that many elements
    while (count--)
    {
        result = terms->pop();
    }
    // if we have a return item, protect it for a little while.
    if (result != OREF_NULL)
    {
        holdObject(result);
    }
    return result;                       /* and return it                     */
}


/**
 * Break up a string into an array of words for parsing and
 * interpretation.  The first word will be uppercased, and all
 * of the strings will be added to the common string pool.
 *
 * @param string The string we break up.
 *
 * @return An array of the words.
 */
ArrayClass  *LanguageParser::words(RexxString *string)
{
    // reduce this to an array of words... this is the easy part.
    ArrayClass *wordArray = string->subWords(OREF_NULL, OREF_NULL);
    size_t count = wordArray->items();

    // it's possible this could contain nothing...just return the
    // empty array
    if (count == 0)
    {
        return wordArray;
    }

    // we could be creating new objects here, better protect this
    ProtectedObject p(wordArray);

    // we know we have at least 1 word here.  Replace the first one
    // with the uppercase, commonstring version.

    wordArray->put(commonString(((RexxString *)wordArray->get(1))->upper()), 1);

    // now make commonstring versions of the rest of the words
    for (size_t i = 2; i <= count; i++)
    {
        wordArray->put(commonString(((RexxString *)wordArray->get(i))), i);
    }

    return wordArray;
}


/**
 * Raise a simple error message, using the current clause
 * location information.
 *
 * @param errorcode The error message to raise.
 */
void LanguageParser::error(RexxErrorCodes errorcode)
{
    ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, OREF_NULL, OREF_NULL);
}


/**
 * Raise an error message using specifically provided location
 * information.
 *
 * @param errorcode The error message to raise.
 * @param location  The location of the instruction in error.
 * @param subs      The message substitutions.
 */
void LanguageParser::error(RexxErrorCodes errorcode, const SourceLocation &location, ArrayClass *subs)
{
    // set the error location.  This location is picked up from the
    // parse context stack frame we set up before we started
    clauseLocation = location;
    ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, subs, OREF_NULL);
}


/**
 * Raise an error where one of the error message substitutions
 * is the line number location of another instruction.
 *
 * @param errorcode The error to issue
 * @param instruction
 *                  The instruction used to obtain the line number for the
 *                  message.
 */
void LanguageParser::errorLine(RexxErrorCodes errorcode, RexxInstruction *instruction)
{
    ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(new_integer(instruction->getLineNumber())), OREF_NULL);
}


/**
 * Raise an error using a Token location for the message
 * substitution.
 *
 * @param errorcode The error to issue.
 * @param token     The source token for the location.
 */
void LanguageParser::errorPosition(RexxErrorCodes errorcode, RexxToken *token )
{
    SourceLocation tokenLocation = token->getLocation();

    ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(new_integer(tokenLocation.getOffset() + 1), new_integer(tokenLocation.getLineNumber())), OREF_NULL);
}


/**
 * Raise an error using the value of the token as a
 * substitution parameter.
 *
 * @param errorcode The error code to issue.
 * @param token     The token used for the substitution information.
 */
void LanguageParser::errorToken(RexxErrorCodes errorcode, RexxToken *token )
{
    ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(token->displayValue()), OREF_NULL);
}


/**
 * Issue an error message with a single substitution value.
 *
 * @param errorcode The error to issue.
 * @param value     The substitution value.
 */
void LanguageParser::error(RexxErrorCodes errorcode, RexxObject *value )
{
    ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(value), OREF_NULL);
}


/**
 * Issue an error message with two substitution values.
 *
 * @param errorcode The error number to issue.
 * @param value1    The first substitution object.
 * @param value2    The second substitution objec.t
 */
void LanguageParser::error(RexxErrorCodes errorcode, RexxObject *value1, RexxObject *value2 )
{
    ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(value1, value2), OREF_NULL);
}


/**
 * Issue an error message with three substitution values.
 *
 * @param errorcode The error number to issue.
 * @param value1    The first substitution object.
 * @param value2    The second substitution objec.t
 * @param value3    The third substitution objec.t
 */
void LanguageParser::error(RexxErrorCodes errorcode, RexxObject *value1, RexxObject *value2, RexxObject *value3 )
{
    ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(value1, value2, value3), OREF_NULL);
}


/**
 * Raise an error for an unclosed block instruction.
 *
 * @param instruction
 *               The unclosed instruction.
 */
void LanguageParser::blockError(RexxInstruction *instruction)
{
    // get the last instruction location and set as the current error location
    clauseLocation = lastInstruction->getLocation();

    switch (instruction->getType())
    {
        // each type of block instruction has its own message

        // DO instruction
        case KEYWORD_SIMPLE_BLOCK:
        case KEYWORD_DO:
            syntaxError(Error_Incomplete_do_do, instruction);
            break;

        // LOOP instruction
        case KEYWORD_LOOP:
        case KEYWORD_LOOP_FOREVER:
        case KEYWORD_LOOP_OVER:
        case KEYWORD_LOOP_OVER_UNTIL:
        case KEYWORD_LOOP_OVER_WHILE:
        case KEYWORD_LOOP_OVER_FOR:
        case KEYWORD_LOOP_OVER_FOR_UNTIL:
        case KEYWORD_LOOP_OVER_FOR_WHILE:
        case KEYWORD_LOOP_CONTROLLED:
        case KEYWORD_LOOP_CONTROLLED_UNTIL:
        case KEYWORD_LOOP_CONTROLLED_WHILE:
        case KEYWORD_LOOP_COUNT:
        case KEYWORD_LOOP_COUNT_UNTIL:
        case KEYWORD_LOOP_COUNT_WHILE:
        case KEYWORD_LOOP_WHILE:
        case KEYWORD_LOOP_UNTIL:
        case KEYWORD_LOOP_WITH:
        case KEYWORD_LOOP_WITH_UNTIL:
        case KEYWORD_LOOP_WITH_WHILE:
        case KEYWORD_LOOP_WITH_FOR:
        case KEYWORD_LOOP_WITH_FOR_UNTIL:
        case KEYWORD_LOOP_WITH_FOR_WHILE:
            syntaxError(Error_Incomplete_do_loop, instruction);
            break;

        // SELECT instruction
        case KEYWORD_SELECT:
        case KEYWORD_SELECT_CASE:
            syntaxError(Error_Incomplete_do_select, instruction);
            break;

        // OTHERWISE section of a SELECT
        case KEYWORD_OTHERWISE:
            syntaxError(Error_Incomplete_do_otherwise, instruction);
            break;

        // different variants of an IF
        case KEYWORD_IF:
        case KEYWORD_WHEN:
        case KEYWORD_WHEN_CASE:
        case KEYWORD_IFTHEN:
        case KEYWORD_WHENTHEN:
            syntaxError(Error_Incomplete_do_then, instruction);
            break;

        // ELSE problem
        case KEYWORD_ELSE:
            syntaxError(Error_Incomplete_do_else, instruction);
            break;

        // invalid block instruction type (should really never happen)
        default:
        {
            reportException(Error_Interpretation_switch, "block instruction type", instruction->getType());
            break;
        }
    }
}


/**
 * Parse off a "logical list expression", consisting of a
 * list of conditionals separated by commas.
 *
 * @return OREF_NULL if no expressions is found, a single expression
 *         element if a single expression is located, and a complex
 *         logical expression operator for a list of expressions.
 */
RexxInternalObject *LanguageParser::parseLogical(int terminators)
{
    // These are not delimited lists, but are part of keyword contexts where
    // other keywords terminate the expression (e.g., IF, WHEN, WHILE), so we
    // need to pass along the terminators.  Unlike argument lists,
    // omitted expressions are not allowed:
    size_t total = 0;                // total is the full count of arguments we attempt to parse.
    RexxToken *terminatorToken;      // the terminator token that ended a sub expression

    // we need to skip ahead to the first real token, then backup one to be
    // properly positioned for the start.  If this is an IF instruction, this
    // will skip the blank between the IF keyword and the start of the expression,
    // which counts as a significant blank by the tokenizer.
    nextReal();
    previousToken();

    // now loop until we get a terminator.  Note that commas are always subexpression
    // terminators, but in this context, we interpret them differently.
    for (;;)
    {
        // parse off an argument expression
        RexxInternalObject *subExpr = parseSubExpression(terminators);
        // all sub expressions are required here.
        if (subExpr == OREF_NULL)
        {
            syntaxError(Error_Invalid_expression_logical_list);
        }

        // We have two term stacks.  The main term stack is used for expression evaluation.
        // the subTerm stack is used for processing expression lists like this.
        pushSubTerm(subExpr);

        // add this to our total count.
        total++;

        // the next token will be our terminator.  If this is not
        // a comma, we have more expressions to parse.
        terminatorToken = nextToken();
        if (!terminatorToken->isComma())
        {
            // push the terminator token back
            previousToken();
            break;
        }
    }

    // we have at least one item (caught in the loop) and if we have exactly
    // one (most common situation), just pop the top item and return it
    if (total == 1)
    {
        return subTerms->pop();
    }

    // composite tis expression into a single object that can evaluate the
    // multiple expressions.
    return new (total) RexxExpressionLogical(total, subTerms);
}


/**
 * Process handling of instore execution arguments.
 *
 * @param instore The instore descriptor.
 * @param name    The name of the program.
 *
 * @return An executable top-level program object (a routine, actually)
 */
RoutineClass *LanguageParser::processInstore(PRXSTRING instore, RexxString * name )
{
    // just a generic empty one indicating that we should check the macrospace?
    if (instore[0].strptr == NULL && instore[1].strptr == NULL)
    {
        unsigned short temp;

        // see if this exists
        if (!RexxQueryMacro(name->getStringData(), &temp))
        {
            return restoreFromMacroSpace(name);
        }
        return OREF_NULL;
    }
    // we have a precompiled image (likely a repeat call...restore and reuse this
    if (instore[1].strptr != NULL)
    {
        // we're saved as a program object
        Protected<RoutineClass> routine = RoutineClass::restore(&instore[1], name);
        if (!routine.isNull())
        {
            // did it unflatten successfully?   If we have source also,
            // reattach it to the routine for tracing/error reporting.
            if (instore[0].strptr != NULL)
            {
                Protected<BufferClass> source_buffer = new_buffer(instore[0]);
                routine->getPackageObject()->attachSource(source_buffer);
            }
            return routine;                  /* go return it                      */
        }
    }
    // we have instorage source, but no compiled image.  We'll compile, and optionally
    // flatten the program for reuse.
    if (instore[0].strptr != NULL)
    {
        Protected<BufferClass> source_buffer = new_buffer(instore[0]);

        // translate the source
        Protected<RoutineClass> routine = createProgram(name, source_buffer);
        // save this in the instore buffer
        routine->save(&instore[1]);
        return routine;
    }
    return OREF_NULL;                    /* processing failed                 */
}


/**
 * Create a routine from a macrospace source.
 *
 * @param name   The name of the macrospace item.
 *
 * @return The inflatted macrospace routine.
 */
RoutineClass *LanguageParser::restoreFromMacroSpace(RexxString *name)
{
    // the instorage buffer
    RXSTRING buffer;

    MAKERXSTRING(buffer, NULL, 0);
    // get the image of function
    RexxResolveMacroFunction(name->getStringData(), &buffer);
    // unflatten the method now
    Protected<RoutineClass> routine = RoutineClass::restore(&buffer, name);
    // release the buffer memory
    SystemInterpreter::releaseResultMemory(buffer.strptr);
    return routine;
}
