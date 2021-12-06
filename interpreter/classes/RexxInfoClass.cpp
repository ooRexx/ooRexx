/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                                */
/*                                                                            */
/* Class for retrieving information about the Rexx interpreter                */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxInfoClass.hpp"
#include "Numerics.hpp"
#include "SysFileSystem.hpp"
#include "MethodArguments.hpp"
#include "Interpreter.hpp"
#include "SystemInterpreter.hpp"
#include "SysProcess.hpp"
#include "ArrayClass.hpp"
#include "PackageClass.hpp"

RexxClass *RexxInfo::classInstance = OREF_NULL;   // singleton class instance

/**
 * Create initial bootstrap objects
 */
void RexxInfo::createInstance()
{
    CLASS_CREATE(RexxInfo);
}


/**
 * Allocate a new RexxContext object
 *
 * @param size   The size of the object.
 *
 * @return The newly allocated object.
 */
void *RexxInfo::operator new(size_t size)
{
    return new_object(size, T_RexxInfo);
}


/**
 * The Rexx accessible class NEW method.  This raises an
 * error because RexxContext objects can only be created
 * by the internal interpreter.
 *
 * @param args   The NEW args
 * @param argc   The count of arguments
 *
 * @return Never returns.
 */
RexxObject *RexxInfo::newRexx(RexxObject **args, size_t argc)
{
    // we do not allow these to be allocated from Rexx code...
    reportException(Error_Unsupported_new_method, ((RexxClass *)this)->getId());
    return TheNilObject;
}


/**
 * An override for the copy method to keep RexxContext
 * objects from being copied.
 *
 * @return Never returns.
 */
RexxObject *RexxInfo::copyRexx()
{
    // we do not allow these to be allocated from Rexx code...
    reportException(Error_Unsupported_copy_method, this);
    return TheNilObject;
}


/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void RexxInfo::live(size_t liveMark)
{
    memory_mark(objectVariables);
    memory_mark(endOfLine);
    memory_mark(directorySeparator);
    memory_mark(pathSeparator);
    memory_mark(interpreterName);
    memory_mark(interpreterDate);
    memory_mark(interpreterVersion);
    memory_mark(languageLevel);
    memory_mark(platformName);
}


/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void RexxInfo::liveGeneral(MarkReason reason)
{
    memory_mark_general(objectVariables);
    memory_mark_general(endOfLine);
    memory_mark_general(directorySeparator);
    memory_mark_general(pathSeparator);
    memory_mark_general(interpreterName);
    memory_mark_general(interpreterDate);
    memory_mark_general(interpreterVersion);
    memory_mark_general(languageLevel);
    memory_mark_general(platformName);
}


/**
 * Flatten a source object.
 *
 * @param envelope The envelope that will hold the flattened object.
 */
void RexxInfo::flatten(Envelope *envelope)
{
    setUpFlatten(RexxInfo)

    flattenRef(objectVariables);
    flattenRef(endOfLine);
    flattenRef(directorySeparator);
    flattenRef(pathSeparator);
    flattenRef(interpreterName);
    flattenRef(interpreterDate);
    flattenRef(interpreterVersion);
    flattenRef(languageLevel);
    flattenRef(platformName);

    cleanUpFlatten
}


/**
 * Initialize the static values of a RexxInfo object.
 */
void RexxInfo::initialize()
{
    char     buffer[100];                // buffer for building the string
    char     work[20];                   // working buffer for the date

    strcpy(work, __DATE__);              // copy the build date
    char *month = strtok(work, " ");     // now we need to tokenize the different bits
    char *day = strtok(NULL, " ");
    char *year = strtok(NULL, " ");

    // make sure we don't have a leading zero on the day
    if (*day == '0')
    {
        day++;
    }
    // now format into a usable date and make into a string object.
    sprintf(buffer, "%s %s %s", day, month, year);
    interpreterDate = new_string(buffer);

    // now build the version string
    sprintf(buffer, "%d.%d.%d", ORX_VER, ORX_REL, ORX_MOD);
    interpreterVersion = new_string(buffer);

    languageLevel = new_string(Interpreter::languageLevel);

    // now handle the platform-specific values
    endOfLine = new_string(SysFileSystem::getLineEnd());
    directorySeparator = new_string(SysFileSystem::getSeparator());
    pathSeparator = new_string(SysFileSystem::getPathSeparator());
    platformName = new_string(SystemInterpreter::getPlatformName());
    interpreterName = Interpreter::getVersionString();
}


/**
 * Return the REXX package object
 *
 * @return The single REXX package instance.
 */
PackageClass *RexxInfo::getPackage()
{
    return TheRexxPackage;
}


/**
 * Return the default digits setting for the interpreter
 *
 * @return The default digits value
 */
RexxObject *RexxInfo::getDigits()
{
    return new_integer(Numerics::DEFAULT_DIGITS);
}


/**
 * Return the default digits setting that the interpreter uses
 * internally for things like argument values, for counts, etc.
 *
 * @return The default digits value
 */
RexxObject *RexxInfo::getInternalDigits()
{
    return new_integer(Numerics::ARGUMENT_DIGITS);
}


/**
 * Return the default fuzz setting for the interpreter
 *
 * @return The current fuzz value
 */
RexxObject *RexxInfo::getFuzz()
{
    return new_integer(Numerics::DEFAULT_FUZZ);
}


/**
 * Return the default FORM setting for the interpreter
 *
 * @return The current form value
 */
RexxObject *RexxInfo::getForm()
{
    // always scientific
    return GlobalNames::SCIENTIFIC;
}


/**
 * Retrieve the implemented language level, as a string.
 *
 * @return The current language level.
 */
RexxString *RexxInfo::getLanguageLevel()
{
    return languageLevel;
}


/**
 * Retrieve the simple interpreter version level.
 *
 * @return The n.n.n interpreter version level.
 */
RexxString *RexxInfo::getInterpreterVersion()
{
    return interpreterVersion;
}


/**
 * Retrieve the full interpreter version string
 *
 * @return The fully encoded interpreter identifier string from
 *         PARSE VERSION.
 */
RexxString *RexxInfo::getInterpreterName()
{
    return interpreterName;
}


/**
 * Get the Major version level for this interpreter.
 *
 * @return The major version level as an Integer object.
 */
RexxObject *RexxInfo::getMajorVersion()
{
    return new_integer(ORX_VER);
}


/**
 * Get the release level for this interpreter.
 *
 * @return The release level as an Integer object.
 */
RexxObject *RexxInfo::getRelease()
{
    return new_integer(ORX_REL);
}


/**
 * Get the modification level for this interpreter.
 *
 * @return The modification level as an Integer object.
 */
RexxObject *RexxInfo::getModification()
{
    return new_integer(ORX_MOD);
}


/**
 * Get the source-control revision for this interpreter.
 *
 * @return The revision as an Integer object.
 */
RexxObject *RexxInfo::getRevision()
{
    return new_integer(ORX_BLD);
}


/**
 * Retrieve the interpreter build date.
 *
 * @return The build date, as a string.
 */
RexxString *RexxInfo::getInterpreterDate()
{
    return interpreterDate;
}


/**
 * Retrieve the Rexx platform name string.
 *
 * @return The string name of the platform version.
 */
RexxString *RexxInfo::getPlatform()
{
    return platformName;
}


/**
 * Return the size of the memory in use.
 *
 * @return An integer object containing the memory size.
 */
RexxObject *RexxInfo::getArchitecture()
{
   return new_integer(sizeof(void *) * 8);
}


/**
 * Get the value used for the file system end-of-line separator.
 *
 * @return The end-of-line separator, as a string.
 */
RexxString *RexxInfo::getFileEndOfLine()
{
    return endOfLine;
}


/**
 * Get the value used for the file system path separator
 *
 * @return The end-of-line separator, as a string.
 */
RexxString *RexxInfo::getPathSeparator()
{
    return pathSeparator;
}


/**
 * Get the value used for the file system path separator
 *
 * @return The end-of-line separator, as a string.
 */
RexxString *RexxInfo::getDirectorySeparator()
{
    return directorySeparator;
}


/**
 * Return an indicator of file system case sensitivity.
 *
 * @return True if the file system is case sensitive, false otherwise.
 */
RexxObject *RexxInfo::getCaseSensitiveFiles()
{
    return booleanObject(SysFileSystem::isCaseSensitive());
}


/**
 * Return the largest platform whole number allowed
 *
 * @return MAX_WHOLENUMBER, as an Integer object.
 */
RexxObject *RexxInfo::getInternalMaxNumber()
{
    return new_integer(Numerics::MAX_WHOLENUMBER);
}


/**
 * Return the smallest platform whole number allowed
 *
 * @return MIN_WHOLENUMBER, as an Integer object.
 */
RexxObject *RexxInfo::getInternalMinNumber()
{
    return new_integer(Numerics::MIN_WHOLENUMBER);
}


/**
 * Return the largest exponent allowed
 *
 * @return MAX_EXPONENT, as an Integer object.
 */
RexxObject *RexxInfo::getMaxExponent()
{
    return new_integer(Numerics::MAX_EXPONENT);
}


/**
 * Return the smallest exponent allowed
 *
 * @return MIN_EXPONENT, as an Integer object.
 */
RexxObject *RexxInfo::getMinExponent()
{
    return new_integer(Numerics::MIN_EXPONENT);
}


/**
 * Return the maximum path length allowed by the file system
 *
 * @return (Windows) MAX_PATH - 1 or (Unix) PATH_MAX, as an Integer object.
 */
RexxObject *RexxInfo::getMaxPathLength()
{
    // usable length is one less, as one char is reserved for the terminating NUL
    return new_integer(SysFileSystem::MaximumPathLength - 1);
}


/**
 * Return the maximum Array size allowed
 *
 * @return MaxFixedArraySize, as an Integer object.
 */
RexxObject *RexxInfo::getMaxArraySize()
{
    return new_integer(ArrayClass::MaxFixedArraySize);
}


/**
 * Return the full path of the currently running executable.
 *
 * @return A full path name as a string or .nil if this cannot be determined.
 */
RexxObject *RexxInfo::getRexxExecutable()
{
    const char *path = SysProcess::getExecutableFullPath();
    if (path == NULL)
    {
        return TheNilObject;
    }
    RexxClass *fileClass = TheRexxPackage->findClass(GlobalNames::FILE);
    Protected<RexxObject> pathString = new_string(path);
    ProtectedObject result;
    return fileClass->sendMessage(GlobalNames::NEW, pathString, result);
}


/**
 * Return the Rexx library directory.
 *
 * @return The directory portion of the path of the Rexx shared libraries,
 *         including a trailing (back)slash.
 */
RexxObject *RexxInfo::getRexxLibrary()
{
    const char *path = SysProcess::getLibraryLocation();
    if (path == NULL)
    {
        return TheNilObject;
    }
    RexxClass *fileClass = TheRexxPackage->findClass(GlobalNames::FILE);
    Protected<RexxObject> pathString = new_string(path);
    ProtectedObject result;
    return fileClass->sendMessage(GlobalNames::NEW, pathString, result);
}


/**
 * If compiled for debugging, then returns .true, .false else
 *
 * @return The end-of-line separator, as a string.
 */
RexxObject *RexxInfo::getDebug()
{
#ifdef _DEBUG
   return TheTrueObject;
#else
   return TheFalseObject;
#endif
}
