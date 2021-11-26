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
RexxMethod0(int,                    // return type
             NoArgMethodReturn123)  // native method name
{
    return 123;                     // return value
}

// -----------------------------------------------------
RexxMethod0(RexxObjectPtr,          // return type a Rexx Object Pointer
             NoArgMethodVoid)       // native method name
{
    fprintf(stdout, "(from native code) \"NoArgMethodVoid\"\n");
    return NULLOBJECT;              // indicate no return value !
}

// -----------------------------------------------------
RexxMethod1(int,                    // return type
             OneArgMethodReturnArg, // native method name
             OPTIONAL_int, arg1)             // argument
{
    fprintf(stdout, "(from native code) \"OneArgMethodReturnArg\": arg1=[%d]", arg1);
    fprintf(stdout, " argumentExists(1)=[%d], argumentOmitted(1)=[%d]\n",
                      argumentExists(1),      argumentOmitted(1));
    return arg1;                    // return value
}

// -----------------------------------------------------
RexxMethod1(RexxObjectPtr,          // return type a Rexx Object Pointer
             OneArgMethodVoid,      // native method name
             int, arg1)             // argument
{
    fprintf(stdout, "(from native code) \"OneArgMethodVoid\": arg1=[%d]\n", arg1);
    return NULLOBJECT;              // indicate no return value !
}

// -----------------------------------------------------
RexxMethod2(int,                    // return type
            TwoIntArgAdder,         // native method name
            int, arg1,              // argument 1
            int, arg2)              // argument 2
{
    return arg1 + arg2;             // return value
}

// -----------------------------------------------------
RexxMethod2(double,                 // return type
            TwoDoubleArgAdder,      // native method name
            double, arg1,           // argument 1
            double, arg2)           // argument 2
{
    return arg1 + arg2;             // return value
}

// -----------------------------------------------------
RexxMethod2(RexxObjectPtr,          // return type a Rexx Object Pointer
             OneArgInvokeOrxMethod, // native method name
             OSELF, self,           // pseudo-argument: allow access to object instance
             int, arg1)             // argument from Rexx program!
{
    fprintf(stdout, "(from native code) \"OneArgInvokeOrxMethod\": arg1=[%d]\n", arg1);
    fprintf(stdout, "(from native code) invoking method \"hello\" in this very same object:\n");
    context->SendMessage0(self,"HELLO");
    return NULLOBJECT;              // indicate no return value !
}


RexxMethodEntry orxtest_meths[] = {

    REXX_METHOD(NoArgMethodReturn123 , NoArgMethodReturn123 ),
    REXX_METHOD(NoArgMethodVoid      , NoArgMethodVoid      ),
    REXX_METHOD(OneArgMethodReturnArg, OneArgMethodReturnArg),
    REXX_METHOD(OneArgMethodVoid     , OneArgMethodVoid     ),
    REXX_METHOD(TwoIntArgAdder       , TwoIntArgAdder       ),
    REXX_METHOD(TwoDoubleArgAdder    , TwoDoubleArgAdder    ),
    REXX_METHOD(OneArgInvokeOrxMethod, OneArgInvokeOrxMethod),
    REXX_LAST_METHOD()                 // end marker
};


RexxPackageEntry DemoExternalMethods_package_entry = {
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_5_0_0,         // ooRexx version 5.0.0 or higher required
    "ExternalMethodsDemo",          // name of the package
    "1.0.0",                        // package information
    NULL,                           // no load function
    NULL,                           // no unload function
    NULL,                           // the exported routines
    orxtest_meths                   // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(DemoExternalMethods);

