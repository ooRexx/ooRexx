/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2021 Rexx Language Association. All rights reserved.         */
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
/* THIS SOFTWARE IS PROVIDED BY THE COPYright HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYright   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include <oorexxapi.h>

// -----------------------------------------------------
RexxRoutine0(int,                   // return type
             NoArgRoutineReturn123) // native routine name
{
    return 123;                     // return value
}

RexxRoutineEntry orxtest_funcs[] = {
    REXX_TYPED_ROUTINE(NoArgRoutineReturn123 , NoArgRoutineReturn123 ),
    REXX_LAST_ROUTINE()                 // end marker
};

// -----------------------------------------------------
RexxMethod0(int,                    // return type
             NoArgMethodReturn123)  // native method name
{
    return 123;                     // return value
}

RexxMethodEntry orxtest_meths[] = {
    REXX_METHOD(NoArgMethodReturn123 , NoArgMethodReturn123 ),
    REXX_LAST_METHOD()                 // end marker
};

RexxPackageEntry DemoExternalLibrary_package_entry = {
    STANDARD_PACKAGE_HEADER
    REXX_CURRENT_INTERPRETER_VERSION,  // ooRexx version at compilation time or higher
    "ExternalMethodsRoutinesDemo",  // name of the package
    "1.0.0",                        // package information
    NULL,                           // no load function
    NULL,                           // no unload function
    orxtest_funcs,                  // the exported routines
    orxtest_meths                   // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(DemoExternalLibrary);

