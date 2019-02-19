/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2019 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
#include <string.h>

RexxRoutine0(int,                       // Return type
            TestZeroIntArgs)            // Object_method name
{
    return 0;
}

RexxRoutine1(int,                       // Return type
            TestOneIntArg,              // Object_method name
            int, arg1)                 // Argument
{
    return arg1;
}

RexxRoutine2(int,                       // Return type
            TestTwoIntArgs,             // Object_method name
            int, arg1,                 // Argument
            int, arg2)                 // Argument
{
    return arg1 + arg2;
}

RexxRoutine3(int,                       // Return type
            TestThreeIntArgs,           // Object_method name
            int, arg1,                 // Argument
            int, arg2,                 // Argument
            int, arg3)                 // Argument
{
    return arg1 + arg2 + arg3;
}

RexxRoutine4(int,                       // Return type
            TestFourIntArgs,            // Object_method name
            int, arg1,                 // Argument
            int, arg2,                 // Argument
            int, arg3,                 // Argument
            int, arg4)                 // Argument
{
    return arg1 + arg2 + arg3 + arg4;
}

RexxRoutine5(int,                       // Return type
            TestFiveIntArgs,            // Object_method name
            int, arg1,                 // Argument
            int, arg2,                 // Argument
            int, arg3,                 // Argument
            int, arg4,                 // Argument
            int, arg5)                 // Argument
{
    return arg1 + arg2 + arg3 + arg4 + arg5;
}

RexxRoutine6(int,                       // Return type
            TestSixIntArgs,             // Object_method name
            int, arg1,                 // Argument
            int, arg2,                 // Argument
            int, arg3,                 // Argument
            int, arg4,                 // Argument
            int, arg5,                 // Argument
            int, arg6)                 // Argument
{
    return arg1 + arg2 + arg3 + arg4 + arg5 + arg6;
}

RexxRoutine7(int,                       // Return type
            TestSevenIntArgs,           // Object_method name
            int, arg1,                 // Argument
            int, arg2,                 // Argument
            int, arg3,                 // Argument
            int, arg4,                 // Argument
            int, arg5,                 // Argument
            int, arg6,                 // Argument
            int, arg7)                 // Argument
{
    return arg1 + arg2 + arg3 + arg4 + arg5 + arg6 + arg7;
}

RexxRoutine8(int,                       // Return type
            TestEightIntArgs,           // Object_method name
            int, arg1,                 // Argument
            int, arg2,                 // Argument
            int, arg3,                 // Argument
            int, arg4,                 // Argument
            int, arg5,                 // Argument
            int, arg6,                 // Argument
            int, arg7,                 // Argument
            int, arg8)                 // Argument
{
    return arg1 + arg2 + arg3 + arg4 + arg5 + arg6 + arg7 + arg8;
}

RexxRoutine9(int,                       // Return type
            TestNineIntArgs,            // Object_method name
            int, arg1,                 // Argument
            int, arg2,                 // Argument
            int, arg3,                 // Argument
            int, arg4,                 // Argument
            int, arg5,                 // Argument
            int, arg6,                 // Argument
            int, arg7,                 // Argument
            int, arg8,                 // Argument
            int, arg9)                 // Argument
{
    return arg1 + arg2 + arg3 + arg4 + arg5 + arg6 + arg7 + arg8 + arg9;
}

RexxRoutine10(int,                       // Return type
            TestTenIntArgs,             // Object_method name
            int, arg1,                 // Argument
            int, arg2,                 // Argument
            int, arg3,                 // Argument
            int, arg4,                 // Argument
            int, arg5,                 // Argument
            int, arg6,                 // Argument
            int, arg7,                 // Argument
            int, arg8,                 // Argument
            int, arg9,                 // Argument
            int, arg10)                // Argument
{
    return arg1 + arg2 + arg3 + arg4 + arg5 + arg6 + arg7 + arg8 + arg9 + arg10;
}

RexxRoutine1(int,                       // Return type
            TestIntArg,                 // Object_method name
            int, arg1)                 // Argument
{
    return arg1;
}

RexxRoutine1(int32_t,                   // Return type
            TestInt32Arg,               // Function routine name
            int32_t, arg1)             // Argument
{
    return arg1;
}

RexxRoutine1(uint32_t,                  // Return type
            TestUint32Arg,              // Function routine name
            uint32_t, arg1)            // Argument
{
    return arg1;
}

RexxRoutine1(int8_t,                    // Return type
            TestInt8Arg,                // Function routine name
            int8_t, arg1)              // Argument
{
    return arg1;
}

RexxRoutine1(uint8_t,                   // Return type
            TestUint8Arg,               // Function routine name
            uint8_t, arg1)             // Argument
{
    return arg1;
}

RexxRoutine1(int16_t,                   // Return type
            TestInt16Arg,               // Function routine name
            int16_t, arg1)             // Argument
{
    return arg1;
}

RexxRoutine1(uint16_t,                  // Return type
            TestUint16Arg,              // Function routine name
            uint16_t, arg1)            // Argument
{
    return arg1;
}

RexxRoutine1(int64_t,                   // Return type
            TestInt64Arg,               // Function routine name
            int64_t, arg1)             // Argument
{
    return arg1;
}

RexxRoutine1(uint64_t,                  // Return type
            TestUint64Arg,              // Function routine name
            uint64_t, arg1)            // Argument
{
    return arg1;
}

RexxRoutine1(intptr_t,                  // Return type
            TestIntPtrArg,               // Function routine name
            intptr_t, arg1)            // Argument
{
    return arg1;
}

RexxRoutine1(uintptr_t,                 // Return type
            TestUintPtrArg,              // Function routine name
            uintptr_t, arg1)           // Argument
{
    return arg1;
}

RexxRoutine1(wholenumber_t,             // Return type
             TestWholeNumberArg,         // Function routine name
             wholenumber_t, arg1)       // Argument
{
    return arg1;
}

RexxRoutine1(positive_wholenumber_t,             // Return type
             TestPositiveWholeNumberArg,         // Function routine name
             positive_wholenumber_t, arg1)       // Argument
{
    return arg1;
}

RexxRoutine1(wholenumber_t,                      // Return type
             TestNonnegativeWholeNumberArg,      // Function routine name
             nonnegative_wholenumber_t, arg1)    // Argument
{
    return arg1;
}


RexxRoutine1(stringsize_t,              // Return type
            TestStringSizeArg,          // Function routine name
            stringsize_t, arg1)         // Argument
{
    return arg1;
}

RexxRoutine1(size_t,                     // Return type
            TestSizeArg,                // Function routine name
            size_t, arg1)               // Argument
{
    return arg1;
}

RexxRoutine1(ssize_t,                     // Return type
            TestSSizeArg,                // Function routine name
            ssize_t, arg1)               // Argument
{
    return arg1;
}

RexxRoutine1(logical_t,                 // Return type
            TestLogicalArg,             // Function routine name
            logical_t, arg1)           // Argument
{
    return arg1;
}

RexxRoutine1(float,                     // Return type
            TestFloatArg,               // Function routine name
            float, arg1)               // Argument
{
    return arg1;
}

RexxRoutine1(double,                    // Return type
            TestDoubleArg,              // Function routine name
            double, arg1)              // Argument
{
    return arg1;
}

RexxRoutine1(CSTRING,                   // Return type
            TestCstringArg,             // Function routine name
            CSTRING, arg1)             // Argument
{
    return arg1;
}

RexxRoutine0(POINTER,                   // Return type
           TestPointerValue)            // Function routine name
{
    return (void *)TestPointerValue;
}

RexxRoutine1(logical_t,                 // Return type
           TestPointerArg,              // Function routine name
           POINTER, arg1)              // Argument
{
    if (arg1 == TestPointerValue) {
        return 1;
    }
    return 0;
}

RexxRoutine0(POINTER,                   // Return type
           TestNullPointerValue)        // Function routine name
{
    return NULL;
}

RexxRoutine0(POINTERSTRING,             // Return type
           TestPointerStringValue)      // Function routine name
{
    return (void *)TestPointerStringValue;
}

RexxRoutine1(logical_t,                 // Return type
           TestPointerStringArg,        // Function routine name
           POINTERSTRING, arg1)         // Argument
{
    if (arg1 == TestPointerStringValue) {
        return 1;
    }
    return 0;
}

RexxRoutine0(POINTERSTRING,                   // Return type
           TestNullPointerStringValue)        // Function routine name
{
    return NULL;
}

RexxRoutine1(RexxStemObject,              // Return type
           TestStemArg,                  // Function routine name
           RexxStemObject, arg1)         // Argument
{
    return arg1;
}

RexxRoutine2(RexxObjectPtr,
            TestObjectToValue,
            RexxObjectPtr, arg1,
            int, type)                 // the type of value to convert
{
    ValueDescriptor value;
    value.type = type;                 // this is the desired type

    if (!context->ObjectToValue(arg1, &value))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }

    return context->ValueToObject(&value);
}

RexxRoutine1(RexxArrayObject,             // Return type
            TestArglistArg,               // Function routine name
            ARGLIST, arg1)                // Argument
{
    return arg1;
}

RexxRoutine1(CSTRING,                     // Return type
            TestNameArg,                  // Function routine name
            NAME, arg1)                   // Argument
{
    return arg1;
}

RexxRoutine1(RexxArrayObject, TestGetArguments,
            ARGLIST, arg1)          // unused dummy argument that allows this to be invoked with variable args.
{
    return context->GetArguments();
}

RexxRoutine2(RexxObjectPtr,
            TestGetArgument,
            size_t, index,
            ARGLIST, arg1)          // unused dummy argument that allows this to be invoked with variable args.
{
    RexxObjectPtr result = context->GetArgument(index);
    if (result == NULLOBJECT)
    {
        // distinguishes between existing/non-existing argments
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return result;
}

RexxRoutine0(CSTRING,
            TestGetRoutineName)
{
    return context->GetRoutineName();
}

RexxRoutine0(RexxObjectPtr,
            TestGetRoutine)
{
    return (RexxObjectPtr)context->GetRoutine();
}

RexxRoutine2(int,
            TestSetContextVariable,
            CSTRING, name,
            RexxObjectPtr, value)
{
    context->SetContextVariable(name, value);
    return 0;
}

RexxRoutine1(RexxObjectPtr,
            TestGetContextVariable,
            CSTRING, name)
{
    RexxObjectPtr value = context->GetContextVariable(name);
    if (value == NULLOBJECT) {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return value;
}

RexxRoutine1(int,
            TestDropContextVariable,
            CSTRING, name)
{
    context->DropContextVariable(name);
    return 0;
}

RexxRoutine0(RexxObjectPtr,
            TestGetAllContextVariables)
{
    return (RexxObjectPtr)context->GetAllContextVariables();
}

RexxRoutine0(stringsize_t,
            TestGetContextDigits)
{
    return context->GetContextDigits();
}

RexxRoutine0(stringsize_t,
            TestGetContextFuzz)
{
    return context->GetContextFuzz();
}

RexxRoutine0(logical_t,
            TestGetContextForm)
{
    return context->GetContextForm();
}

RexxRoutine1(RexxStemObject,
            TestResolveStemVariable,
            RexxObjectPtr, arg1)
{
    RexxStemObject value = context->ResolveStemVariable(arg1);
    if (value == NULLOBJECT) {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return value;
}

RexxRoutine1(RexxClassObject,
            TestFindContextClass,
            CSTRING, name)
{
    RexxClassObject value = context->FindContextClass(name);
    if (value == NULLOBJECT) {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return value;
}

// test DIRECT command handler for TestAddCommandEnvironment
// always returns -1
RexxObjectPtr RexxEntry dHandler(RexxExitContext *context,
                                 RexxStringObject address,
                                 RexxStringObject command)
{
    return context->WholeNumberToObject(-1);
}


// test REDIRECTING command handler for TestAddCommandEnvironment
// just sets a return code identifying various I/O context functions
RexxObjectPtr RexxEntry rHandler(RexxExitContext *context,
                                 RexxStringObject address,
                                 RexxStringObject command,
                                 RexxIORedirectorContext *ioContext)
{
    size_t rc = 0;
    // we don't do actual redirection, but we want to return a five-digit
    // number with each digit in sequence representing the status of:
    // - IsRedirectionRequested()
    // - IsInputRedirected()
    // - IsOutputRedirected()
    // - IsErrorRedirected()
    // - AreOutputAndErrorSameTarget()
    rc = rc * 10 + ioContext->IsRedirectionRequested();
    rc = rc * 10 + ioContext->IsInputRedirected();
    rc = rc * 10 + ioContext->IsOutputRedirected();
    rc = rc * 10 + ioContext->IsErrorRedirected();
    rc = rc * 10 + ioContext->AreOutputAndErrorSameTarget();

    return context->WholeNumberToObject(rc);
}


// test command handler for the I/O redirection APIs
RexxObjectPtr RexxEntry ioHandler(RexxExitContext *context, RexxStringObject address, RexxStringObject command, RexxIORedirectorContext *ioContext)
{
    CSTRING commandString = context->CString(command);

    if (strcmp(commandString, "INPUTOUTPUT") == 0)
    {
        CSTRING data;
        size_t length;
        size_t count = 0;

        ioContext->ReadInput(&data, &length);
        while (data != NULL)
        {
            count++;
            ioContext->WriteOutput(data, length);
            ioContext->ReadInput(&data, &length);
        }

        return context->StringSizeToObject(count);
    }

    if (strcmp(commandString, "INPUTERROR") == 0)
    {
        CSTRING data;
        size_t length;
        size_t count = 0;

        ioContext->ReadInput(&data, &length);
        while (data != NULL)
        {
            count++;
            ioContext->WriteError(data, length);
            ioContext->ReadInput(&data, &length);
        }

        return context->StringSizeToObject(count);
    }

    if (strcmp(commandString, "INPUTBOTH") == 0)
    {
        CSTRING data;
        size_t length;
        bool useError = false;
        size_t count = 0;

        ioContext->ReadInput(&data, &length);
        while (data != NULL)
        {
            count++;
            if (useError)
            {
                ioContext->WriteError(data, length);
            }
            else
            {
                ioContext->WriteOutput(data, length);
            }
            useError = !useError;
            ioContext->ReadInput(&data, &length);
        }

        return context->StringSizeToObject(count);
    }


    if (strcmp(commandString, "NOBLANKOUTPUT") == 0)
    {
        CSTRING data;
        size_t length;
        size_t count = 0;

        ioContext->ReadInput(&data, &length);
        while (data != NULL)
        {
            count++;
            // only write non blank lines
            if (length > 0)
            {
                ioContext->WriteOutput(data, length);
            }
            ioContext->ReadInput(&data, &length);
        }

        return context->StringSizeToObject(count);
    }


    if (strcmp(commandString, "NOBLANKERROR") == 0)
    {
        CSTRING data;
        size_t length;
        size_t count = 0;

        ioContext->ReadInput(&data, &length);
        while (data != NULL)
        {
            count++;
            // only write non blank lines
            if (length > 0)
            {
                ioContext->WriteError(data, length);
            }
            ioContext->ReadInput(&data, &length);
        }

        return context->StringSizeToObject(count);
    }

    if (strcmp(commandString, "BUFFEROUTPUT") == 0)
    {
        CSTRING data;
        size_t length;
        size_t count = 0;

        ioContext->ReadInput(&data, &length);
        while (data != NULL)
        {
            count++;
            ioContext->WriteOutputBuffer(data, length);
            ioContext->ReadInput(&data, &length);
        }

        return context->StringSizeToObject(count);
    }

    if (strcmp(commandString, "BUFFERERROR") == 0)
    {
        CSTRING data;
        size_t length;
        size_t count = 0;

        ioContext->ReadInput(&data, &length);
        while (data != NULL)
        {
            count++;
            ioContext->WriteErrorBuffer(data, length);
            ioContext->ReadInput(&data, &length);
        }

        return context->StringSizeToObject(count);
    }

    if (strcmp(commandString, "BUFFERINPUT") == 0)
    {
        CSTRING data;
        size_t length;

        ioContext->ReadInputBuffer(&data, &length);
        if (data != NULL)
        {
            ioContext->WriteOutputBuffer(data, length);
        }

        return context->StringSizeToObject(length);
    }

    if (strcmp(commandString, "INPUTREDIRECTED") == 0)
    {
        return ioContext->IsInputRedirected() ? context->True() : context->False();
    }

    if (strcmp(commandString, "OUTPUTREDIRECTED") == 0)
    {
        return ioContext->IsOutputRedirected() ? context->True() : context->False();
    }

    if (strcmp(commandString, "ERRORREDIRECTED") == 0)
    {
        return ioContext->IsErrorRedirected() ? context->True() : context->False();
    }

    if (strcmp(commandString, "AREOUTPUTERRORTHESAME") == 0)
    {
        return ioContext->AreOutputAndErrorSameTarget() ? context->True() : context->False();
    }

    if (strcmp(commandString, "ISREDIRECTIONREQUESTED") == 0)
    {
        return ioContext->IsRedirectionRequested() ? context->True() : context->False();
    }

    return context->True();
}


// install 'ioHandler', 'rHandler', or 'dHandler'
RexxRoutine2(RexxObjectPtr,
             TestAddCommandEnvironment,
             CSTRING, name,
             CSTRING, type)
{
    if (type[0] == 'r' || type[0] == 'R')
    {   // redirecting command handler
        if (strcmp(name, "io") == 0)
        {
            context->AddCommandEnvironment(name, (REXXPFN)ioHandler, REDIRECTING_COMMAND_ENVIRONMENT);
        }
        else
        {
            context->AddCommandEnvironment(name, (REXXPFN)rHandler, REDIRECTING_COMMAND_ENVIRONMENT);
        }
    }
    else
    {   // direct command handler
        context->AddCommandEnvironment(name, (REXXPFN)dHandler, DIRECT_COMMAND_ENVIRONMENT);
    }
    return NULLOBJECT;
}



RexxRoutineEntry orxtest_funcs[] = {
    REXX_TYPED_ROUTINE(TestZeroIntArgs,       TestZeroIntArgs),
    REXX_TYPED_ROUTINE(TestOneIntArg,         TestOneIntArg),
    REXX_TYPED_ROUTINE(TestTwoIntArgs,        TestTwoIntArgs),
    REXX_TYPED_ROUTINE(TestThreeIntArgs,      TestThreeIntArgs),
    REXX_TYPED_ROUTINE(TestFourIntArgs,       TestFourIntArgs),
    REXX_TYPED_ROUTINE(TestFiveIntArgs,       TestFiveIntArgs),
    REXX_TYPED_ROUTINE(TestSixIntArgs,        TestSixIntArgs),
    REXX_TYPED_ROUTINE(TestSevenIntArgs,      TestSevenIntArgs),
    REXX_TYPED_ROUTINE(TestEightIntArgs,      TestEightIntArgs),
    REXX_TYPED_ROUTINE(TestNineIntArgs,       TestNineIntArgs),
    REXX_TYPED_ROUTINE(TestTenIntArgs,        TestTenIntArgs),
    REXX_TYPED_ROUTINE(TestIntArg,            TestIntArg),
    REXX_TYPED_ROUTINE(TestInt32Arg,          TestInt32Arg),
    REXX_TYPED_ROUTINE(TestUint32Arg,         TestUint32Arg),
    REXX_TYPED_ROUTINE(TestInt8Arg,           TestInt8Arg),
    REXX_TYPED_ROUTINE(TestUint8Arg,          TestUint8Arg),
    REXX_TYPED_ROUTINE(TestInt16Arg,          TestInt16Arg),
    REXX_TYPED_ROUTINE(TestUint16Arg,         TestUint16Arg),
    REXX_TYPED_ROUTINE(TestInt64Arg,          TestInt64Arg),
    REXX_TYPED_ROUTINE(TestUint64Arg,         TestUint64Arg),
    REXX_TYPED_ROUTINE(TestIntPtrArg,         TestIntPtrArg),
    REXX_TYPED_ROUTINE(TestUintPtrArg,        TestUintPtrArg),
    REXX_TYPED_ROUTINE(TestWholeNumberArg,    TestWholeNumberArg),
    REXX_TYPED_ROUTINE(TestPositiveWholeNumberArg,    TestPositiveWholeNumberArg),
    REXX_TYPED_ROUTINE(TestNonnegativeWholeNumberArg, TestNonnegativeWholeNumberArg),
    REXX_TYPED_ROUTINE(TestStringSizeArg,     TestStringSizeArg),
    REXX_TYPED_ROUTINE(TestSizeArg,           TestSizeArg),
    REXX_TYPED_ROUTINE(TestSSizeArg,          TestSSizeArg),
    REXX_TYPED_ROUTINE(TestLogicalArg,        TestLogicalArg),
    REXX_TYPED_ROUTINE(TestFloatArg,          TestFloatArg),
    REXX_TYPED_ROUTINE(TestDoubleArg,         TestDoubleArg),
    REXX_TYPED_ROUTINE(TestCstringArg,        TestCstringArg),
    REXX_TYPED_ROUTINE(TestPointerValue,      TestPointerValue),
    REXX_TYPED_ROUTINE(TestPointerArg,        TestPointerArg),
    REXX_TYPED_ROUTINE(TestNullPointerValue,  TestNullPointerValue),
    REXX_TYPED_ROUTINE(TestPointerStringValue,      TestPointerStringValue),
    REXX_TYPED_ROUTINE(TestPointerStringArg,        TestPointerStringArg),
    REXX_TYPED_ROUTINE(TestNullPointerStringValue,  TestNullPointerStringValue),
    REXX_TYPED_ROUTINE(TestStemArg,           TestStemArg),
    REXX_TYPED_ROUTINE(TestNameArg,           TestNameArg),
    REXX_TYPED_ROUTINE(TestArglistArg,        TestArglistArg),
    REXX_TYPED_ROUTINE(TestObjectToValue,     TestObjectToValue),
    REXX_TYPED_ROUTINE(TestGetArguments,      TestGetArguments),
    REXX_TYPED_ROUTINE(TestGetArgument,       TestGetArgument),
    REXX_TYPED_ROUTINE(TestGetRoutineName,    TestGetRoutineName),
    REXX_TYPED_ROUTINE(TestGetRoutine,        TestGetRoutine),
    REXX_TYPED_ROUTINE(TestSetContextVariable, TestSetContextVariable),
    REXX_TYPED_ROUTINE(TestGetContextVariable, TestGetContextVariable),
    REXX_TYPED_ROUTINE(TestDropContextVariable, TestDropContextVariable),
    REXX_TYPED_ROUTINE(TestGetAllContextVariables, TestGetAllContextVariables),
    REXX_TYPED_ROUTINE(TestGetContextFuzz,    TestGetContextFuzz),
    REXX_TYPED_ROUTINE(TestGetContextForm,    TestGetContextForm),
    REXX_TYPED_ROUTINE(TestGetContextDigits,  TestGetContextDigits),
    REXX_TYPED_ROUTINE(TestResolveStemVariable, TestResolveStemVariable),
    REXX_TYPED_ROUTINE(TestFindContextClass,  TestFindContextClass),
    REXX_TYPED_ROUTINE(TestAddCommandEnvironment, TestAddCommandEnvironment),
    REXX_LAST_ROUTINE()
};


RexxPackageEntry UnitTest_package_entry = {
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "UnitTest",                          // name of the package
    "1.0.0",                             // package information
    NULL,                                // no load/unload functions
    NULL,
    orxtest_funcs,                       // the exported routines
    NULL                                 // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(UnitTest);

