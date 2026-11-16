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
#include "BuiltinFunctionSupport.hpp"
#include "Utilities.hpp"

#include <ctype.h>
#include <math.h>

// BEEP, DIRECTORY and FILESPEC are built-in functions since 5.3.0.  These
// routines are kept only for the programs compiled by older releases, which
// call them as external routines of this package; they are hidden from
// RxFuncQuery, RxFuncAdd and RxFuncDrop (PackageManager::isRetiredRoutine()).
// The work is shared with the built-in functions (BuiltinFunctionSupport);
// here, only the arguments are converted, with the native API, and the errors
// are those these routines always gave.  They can go when MinimumLanguageLevel
// passes 6.06 (see BuiltinFunctionSupport.hpp).

/****************************************************************************/
/* sysDirectory                                                             */
/****************************************************************************/
RexxRoutine1(RexxStringObject, sysDirectory, OPTIONAL_CSTRING, dir)
{
    RoutineFileNameBuffer qualified(context);
    RoutineFileNameBuffer current(context);
    if (!BuiltinFunctionSupport::directory(dir, qualified, current))
    {
        // a null string if we can't change the directory
        return context->NullString();
    }
    return context->NewStringFromAsciiz(current);
}


/********************************************************************************************/
/* sysFilespec                                                                              */
/********************************************************************************************/
RexxRoutine2(RexxStringObject, sysFilespec, CSTRING, option, CSTRING, name)
{
    const char *part;
    size_t length;
    if (!BuiltinFunctionSupport::filespec(Utilities::toUpper(*option), name, part, length))
    {
        char optionChar[2];
        optionChar[0] = *option;
        optionChar[1] = '\0';

        RexxArrayObject subs = context->Array(context->String("FILESPEC"), context->WholeNumberToObject(1),
                                              context->String(BuiltinFunctionSupport::FilespecOptions), context->String(optionChar));
        context->RaiseException(Rexx_Error_Incorrect_call_list, subs);
        return NULLOBJECT;
    }
    return context->String(part, length);
}


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
    if (!beepArgument(context, frequencyArg, "frequency", BuiltinFunctionSupport::MinBeepFrequency,
        BuiltinFunctionSupport::MaxBeepFrequency, frequency))
    {
        return NULL;
    }
    if (durationArg == NULLOBJECT)
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_noarg, context->WholeNumberToObject(2));
        return NULL;
    }
    if (!beepArgument(context, durationArg, "duration", BuiltinFunctionSupport::MinBeepDuration,
        BuiltinFunctionSupport::MaxBeepDuration, duration))
    {
        return NULL;
    }

    BuiltinFunctionSupport::beep(frequency, duration);
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
