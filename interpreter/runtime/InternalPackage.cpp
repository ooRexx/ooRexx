/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2026 Rexx Language Association. All rights reserved.    */
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
/* Package definition for exported native methods in the "REXX" package       */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "PackageManager.hpp"
#include "SysFileSystem.hpp"
#include "ExternalFileBuffer.hpp"
#include "SysProcess.hpp"

#include <ctype.h>
#include <math.h>

// FILESPEC function options
#define FILESPEC_PATH         'P'
#define FILESPEC_NAME         'N'
#define FILESPEC_LOCATION     'L'
#define FILESPEC_EXTENSION    'E'
#define FILESPEC_DRIVE        'D'


/****************************************************************************/
/* sysDirectory                                                             */
/****************************************************************************/
RexxRoutine1(RexxStringObject, sysDirectory, OPTIONAL_CSTRING, dir)
{
    if (dir != NO_CSTRING)
    {
        RoutineQualifiedName qualifiedName(context, dir);

        if (!SysFileSystem::setCurrentDirectory(qualifiedName))
        {
            // return a NULL string if we can't change the directory
            // to indicate an error
            return context->NullString();
        }
    }

    RoutineFileNameBuffer newDir(context);

    // obtain the current directory
    SysFileSystem::getCurrentDirectory(newDir);
    return context->NewStringFromAsciiz(newDir);
}


/********************************************************************************************/
/* sysFilespec                                                                              */
/********************************************************************************************/
RexxRoutine2(RexxStringObject, sysFilespec, CSTRING, option, CSTRING, name)
{
    const char *endPtr = name + strlen(name);        // point to last character

    const char *pathStart = SysFileSystem::getPathStart(name);
    const char *pathEnd = SysFileSystem::getPathEnd(name);

    // get the end of the path portion (if any)
    // note that pathend is one character past the end of the path.
    // this means the length is easily calculated as pathEnd - pathStart,
    // even in the cases where there is no patch portion
    pathEnd = pathEnd == NULL ? pathStart : pathEnd + 1;
    // this one needs a little adjustment for the case where this is all name
    const char *nameStart = pathEnd == name ? name : pathEnd;

    switch (Utilities::toUpper(*option)) // process each option
    {
        case FILESPEC_PATH:              // extract the path (without drive)
        {
            return context->String(pathStart, pathEnd - pathStart);
        }

        case FILESPEC_NAME:              // extract the file name
        {
            return context->String(nameStart, endPtr - nameStart);
        }

        case FILESPEC_LOCATION:          // extract the path (including drive)
        {
            return context->String(name, pathEnd - name);
        }

        case FILESPEC_DRIVE:             // extract the drive
        {
            // this will return a null string if nothing is before the pathStart
            return context->String(name, pathStart - name);
        }

        case FILESPEC_EXTENSION:           // extract the file extension
        {
            // find the position of the last dot
            const char *lastDot = strrchr(name, '.');

            if (lastDot >= nameStart)
            {
                // we don't extract the period
                lastDot++;
                return context->String(lastDot, endPtr - lastDot);
            }
            else
            {
                return context->NullString();        // nothing found, return the empty string
            }

        }
        default:                           /* unknown option                    */
        {
            char optionChar[2];
            optionChar[0] = *option;
            optionChar[1] = '\0';

            RexxArrayObject subs = context->Array(context->String("FILESPEC"), context->WholeNumberToObject(1),
                                                  context->String("DELNP"), context->String(optionChar));
            /* raise an error                    */
            context->RaiseException(Rexx_Error_Incorrect_call_list, subs);
            return NULLOBJECT;
        }
    }
}


#define  MAX_FREQUENCY 32767
#define  MIN_FREQUENCY    37
#define  MAX_DURATION  60000
#define  MIN_DURATION      0

/**
 * Get one of BEEP's arguments: a whole number in a range.  A value that is
 * not a whole number gets "must be a whole number"; a whole number outside
 * the range (however large) gets the range error.
 *
 * @return false if an error has been raised.
 */
static bool beepArgument(RexxCallContext *context, RexxObjectPtr argument, const char *name,
    wholenumber_t minimum, wholenumber_t maximum, wholenumber_t &value)
{
    double number;
    if (!context->ObjectToWholeNumber(argument, &value) &&
        (!context->ObjectToDouble(argument, &number) || number != floor(number)))
    {
        context->RaiseException2(Rexx_Error_Invalid_argument_whole, context->NewStringFromAsciiz(name), argument);
        return false;
    }
    if (!context->ObjectToWholeNumber(argument, &value) || value > maximum || value < minimum)
    {
        RexxArrayObject subs = context->NewArray(4);
        context->ArrayAppend(subs, context->NewStringFromAsciiz(name));
        context->ArrayAppend(subs, context->WholeNumberToObject(minimum));
        context->ArrayAppend(subs, context->WholeNumberToObject(maximum));
        context->ArrayAppend(subs, argument);
        context->RaiseException(Rexx_Error_Invalid_argument_range, subs);
        return false;
    }
    return true;
}


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   sysBeep                                      */
/*                                                                   */
/*   Descriptive Name:  BEEP function                                */
/*                                                                   */
/*   Function:          sounds the speaker at frequency Hertz for    */
/*                      specified duration (in milliseconds)         */
/*********************************************************************/
RexxRoutine2(CSTRING, sysBeep, RexxObjectPtr, frequencyArg, OPTIONAL_RexxObjectPtr, durationArg)
{
    // the arguments are checked here, in order and by name, and not by the
    // conversion of the native API, whose message for a non-number gives the
    // range of a wholenumber_t (bug #2009); duration is required, but it is
    // declared optional so that a bad frequency is reported first
    wholenumber_t frequency, duration;
    if (!beepArgument(context, frequencyArg, "frequency", MIN_FREQUENCY, MAX_FREQUENCY, frequency))
    {
        return NULL;
    }
    if (durationArg == NULLOBJECT)
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_noarg, context->WholeNumberToObject(2));
        return NULL;
    }
    if (!beepArgument(context, durationArg, "duration", MIN_DURATION, MAX_DURATION, duration))
    {
        return NULL;
    }

    SysProcess::beep((int)frequency, (int)duration);
    return "";                           /* always returns a null      */
}


#define INTERNAL_METHOD(name) REXX_METHOD_PROTOTYPE(name)

#include "NativeMethods.h"             // bring in the standard list,
#include "SysNativeMethods.h"          // plus any system extensions

#undef  INTERNAL_METHOD
#define INTERNAL_METHOD(name) REXX_METHOD(name, name),

// now build the actual entry list
RexxMethodEntry rexx_methods[] =
{
#include "NativeMethods.h"             // bring in the standard list,
#include "SysNativeMethods.h"          // plus any system extensions
    REXX_LAST_METHOD()
};


#define INTERNAL_ROUTINE(name, entry) REXX_TYPED_ROUTINE_PROTOTYPE(entry)

#include "NativeFunctions.h"             // bring in the standard list,
#include "SysNativeFunctions.h"          // plus any system extensions

#undef  INTERNAL_ROUTINE
#define INTERNAL_ROUTINE(name, entry) REXX_TYPED_ROUTINE(name, entry),

// now build the actual entry list
RexxRoutineEntry rexx_routines[] =
{
#include "NativeFunctions.h"             // bring in the standard list,
#include "SysNativeFunctions.h"          // plus any system extensions
    REXX_LAST_ROUTINE()
};

RexxPackageEntry rexx_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_CURRENT_INTERPRETER_VERSION,    // always make this the current version
    "REXX",                              // name of the package
    "4.0",                               // package information
    NULL,                                // no load/unload functions
    NULL,
    rexx_routines,                       // the exported routines
    rexx_methods                         // the exported methods
};

// and finally plug this in to the package manager.
RexxPackageEntry *PackageManager::rexxPackage = &rexx_package_entry;
