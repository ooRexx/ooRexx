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
#include <stdio.h>

// -----------------------------------------------------
RexxRoutine0(int,                   // return type
             NoArgRoutineReturn123) // native routine name
{
    return 123;                     // return value
}

// -----------------------------------------------------
RexxRoutine0(RexxObjectPtr,         // return type a Rexx Object Pointer
             NoArgRoutineVoid)      // native routine name
{
    fprintf(stdout, "(from native code) \"NoArgRoutineVoid\"\n");
    return NULLOBJECT;              // indicate no return value !
}

// -----------------------------------------------------
RexxRoutine1(int,                   // return type
             OneArgRoutineReturnArg,// native routine name
             OPTIONAL_int, arg1)    // optional argument
{
    fprintf(stdout, "(from native code) \"OneArgRoutineReturnArg\": arg1=[%d]", arg1);
    fprintf(stdout, " argumentExists(1)=[%d], argumentOmitted(1)=[%d]\n",
                      argumentExists(1),      argumentOmitted(1));
    return arg1;                    // return value
}

// -----------------------------------------------------
RexxRoutine1(RexxObjectPtr,         // return type a Rexx Object Pointer
             OneArgRoutineVoid,     // native routine name
             int, arg1)             // argument
{
    fprintf(stdout, "(from native code) \"OneArgRoutineVoid\": arg1=[%d]\n", arg1);
    return NULLOBJECT;              // indicate no return value !
}

// -----------------------------------------------------
RexxRoutine2(int,                   // return type
            TwoIntArgAdder,         // native routine name
            int, arg1,              // argument 1
            int, arg2)              // argument 2
{
    return arg1 + arg2;             // return value
}

// -----------------------------------------------------
RexxRoutine2(double,                // return type
            TwoDoubleArgAdder,      // native routine name
            double, arg1,           // argument 1
            double, arg2)           // argument 2
{
    return arg1 + arg2;             // return value
}


RexxRoutineEntry orxtest_funcs[] = {

    REXX_TYPED_ROUTINE(NoArgRoutineReturn123 , NoArgRoutineReturn123 ),
    REXX_TYPED_ROUTINE(NoArgRoutineVoid      , NoArgRoutineVoid      ),
    REXX_TYPED_ROUTINE(OneArgRoutineReturnArg, OneArgRoutineReturnArg),
    REXX_TYPED_ROUTINE(OneArgRoutineVoid     , OneArgRoutineVoid     ),
    REXX_TYPED_ROUTINE(TwoIntArgAdder        , TwoIntArgAdder        ),
    REXX_TYPED_ROUTINE(TwoDoubleArgAdder     , TwoDoubleArgAdder     ),
    REXX_LAST_ROUTINE()                 // end marker
};


RexxPackageEntry DemoExternalRoutines_package_entry = {
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,         // ooRexx version 4.0.0 or higher required
    "ExternalRoutinesDemo",         // name of the package
    "1.0.0",                        // package information
    NULL,                           // no load function
    NULL,                           // no unload function
    orxtest_funcs,                  // the exported routines
    NULL                            // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(DemoExternalRoutines);

