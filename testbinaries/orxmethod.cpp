/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2024 Rexx Language Association. All rights reserved.    */
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

#include <string.h>
#include <oorexxapi.h>


RexxMethod0(int,                       // Return type
            TestZeroIntArgs)            // Object_method name
{
    return 0;
}

RexxMethod1(int,                       // Return type
            TestOneIntArg,              // Object_method name
            int, arg1)                 // Argument
{
    return arg1;
}

RexxMethod2(int,                       // Return type
            TestTwoIntArgs,             // Object_method name
            int, arg1,                 // Argument
            int, arg2)                 // Argument
{
    return arg1 + arg2;
}

RexxMethod3(int,                       // Return type
            TestThreeIntArgs,           // Object_method name
            int, arg1,                 // Argument
            int, arg2,                 // Argument
            int, arg3)                 // Argument
{
    return arg1 + arg2 + arg3;
}

RexxMethod4(int,                       // Return type
            TestFourIntArgs,            // Object_method name
            int, arg1,                 // Argument
            int, arg2,                 // Argument
            int, arg3,                 // Argument
            int, arg4)                 // Argument
{
    return arg1 + arg2 + arg3 + arg4;
}

RexxMethod5(int,                       // Return type
            TestFiveIntArgs,            // Object_method name
            int, arg1,                 // Argument
            int, arg2,                 // Argument
            int, arg3,                 // Argument
            int, arg4,                 // Argument
            int, arg5)                 // Argument
{
    return arg1 + arg2 + arg3 + arg4 + arg5;
}

RexxMethod6(int,                       // Return type
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

RexxMethod7(int,                       // Return type
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

RexxMethod8(int,                       // Return type
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

RexxMethod9(int,                       // Return type
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

RexxMethod10(int,                       // Return type
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

RexxMethod1(int,                       // Return type
            TestIntArg,                 // Object_method name
            int, arg1)                 // Argument
{
    return arg1;
}

RexxMethod1(int32_t,                   // Return type
            TestInt32Arg,               // Function routine name
            int32_t, arg1)             // Argument
{
    return arg1;
}

RexxMethod1(uint32_t,                  // Return type
            TestUint32Arg,              // Function routine name
            uint32_t, arg1)            // Argument
{
    return arg1;
}

RexxMethod1(int8_t,                    // Return type
            TestInt8Arg,                // Function routine name
            int8_t, arg1)              // Argument
{
    return arg1;
}

RexxMethod1(uint8_t,                   // Return type
            TestUint8Arg,               // Function routine name
            uint8_t, arg1)             // Argument
{
    return arg1;
}

RexxMethod1(int16_t,                   // Return type
            TestInt16Arg,               // Function routine name
            int16_t, arg1)             // Argument
{
    return arg1;
}

RexxMethod1(uint16_t,                  // Return type
            TestUint16Arg,              // Function routine name
            uint16_t, arg1)            // Argument
{
    return arg1;
}

RexxMethod1(int64_t,                   // Return type
            TestInt64Arg,               // Function routine name
            int64_t, arg1)             // Argument
{
    return arg1;
}

RexxMethod1(uint64_t,                  // Return type
            TestUint64Arg,              // Function routine name
            uint64_t, arg1)            // Argument
{
    return arg1;
}

RexxMethod1(intptr_t,                  // Return type
            TestIntPtrArg,               // Function routine name
            intptr_t, arg1)            // Argument
{
    return arg1;
}

RexxMethod1(uintptr_t,                 // Return type
            TestUintPtrArg,              // Function routine name
            uintptr_t, arg1)           // Argument
{
    return arg1;
}

RexxMethod1(wholenumber_t,             // Return type
            TestWholeNumberArg,         // Function routine name
            wholenumber_t, arg1)       // Argument
{
    return arg1;
}

RexxMethod1(wholenumber_t,                // Return type
            TestNonnegativeWholeNumberArg,   // Function routine name
            nonnegative_wholenumber_t, arg1) // Argument
{
    return arg1;
}

RexxMethod1(stringsize_t,              // Return type
            TestStringSizeArg,          // Function routine name
            stringsize_t, arg1)        // Argument
{
    return arg1;
}

RexxMethod1(size_t,                     // Return type
            TestSizeArg,                // Function routine name
            size_t, arg1)               // Argument
{
    return arg1;
}

RexxMethod1(ssize_t,                     // Return type
            TestSSizeArg,                // Function routine name
            ssize_t, arg1)               // Argument
{
    return arg1;
}

RexxMethod1(logical_t,                 // Return type
            TestLogicalArg,             // Function routine name
            logical_t, arg1)           // Argument
{
    return arg1;
}

RexxMethod1(float,                     // Return type
            TestFloatArg,               // Function routine name
            float, arg1)               // Argument
{
    return arg1;
}

RexxMethod1(double,                    // Return type
            TestDoubleArg,              // Function routine name
            double, arg1)              // Argument
{
    return arg1;
}

RexxMethod1(CSTRING,                   // Return type
            TestCstringArg,             // Function routine name
            CSTRING, arg1)             // Argument
{
    return arg1;
}

RexxMethod0(POINTER,                   // Return type
           TestPointerValue)            // Function routine name
{
    return (void *)TestPointerValue;
}

RexxMethod1(logical_t,                 // Return type
           TestPointerArg,              // Function routine name
           POINTER, arg1)              // Argument
{
    if (arg1 == TestPointerValue) {
        return 1;
    }
    return 0;
}

RexxMethod0(POINTER,                   // Return type
           TestNullPointerValue)        // Function routine name
{
    return NULL;
}

RexxMethod0(POINTERSTRING,             // Return type
           TestPointerStringValue)      // Function routine name
{
    return (void *)TestPointerStringValue;
}

RexxMethod1(logical_t,                 // Return type
           TestPointerStringArg,        // Function routine name
           POINTERSTRING, arg1)         // Argument
{
    if (arg1 == TestPointerStringValue) {
        return 1;
    }
    return 0;
}

RexxMethod0(POINTERSTRING,                   // Return type
           TestNullPointerStringValue)        // Function routine name
{
    return NULL;
}

RexxMethod1(RexxStemObject,              // Return type
           TestStemArg,                  // Function routine name
           RexxStemObject, arg1)         // Argument
{
    return arg1;
}

RexxMethod1(RexxStringObject,            // Return type
           TestStringArg,                // Function routine name
           RexxStringObject, arg1)       // Argument
{
    return arg1;
}

RexxMethod1(RexxObjectPtr,               // Return type
           TestObjectArg,                // Function routine name
           RexxObjectPtr, arg1)          // Argument
{
    return arg1;
}

RexxMethod1(RexxArrayObject,             // Return type
           TestArrayArg,                 // Function routine name
           RexxArrayObject, arg1)        // Argument
{
    return arg1;
}

RexxMethod1(RexxClassObject,             // Return type
           TestClassArg,                 // Function routine name
           RexxClassObject, arg1)        // Argument
{
    return arg1;
}

RexxMethod1(RexxClassObject,             // Return type
           TestSuperArg,                 // Function routine name
           SUPER, arg1)                  // Argument
{
    return (RexxClassObject)arg1;
}

RexxMethod1(RexxObjectPtr,               // Return type
           TestScopeArg,                 // Function routine name
           SCOPE, arg1)                  // Argument
{
    return arg1;
}

RexxMethod1(RexxObjectPtr,               // Return type
           TestOSelfArg,                 // Function routine name
           OSELF, arg1)                  // Argument
{
    return arg1;
}

RexxMethod1(RexxArrayObject,             // Return type
           TestArglistArg,               // Function routine name
           ARGLIST, arg1)                // Argument
{
    return arg1;
}

RexxMethod1(CSTRING,                     // Return type
           TestNameArg,                  // Function routine name
           NAME, arg1)                   // Argument
{
    return arg1;
}

RexxMethod1(wholenumber_t,             // Return type
            TestObjectToWholeNumber,    // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    wholenumber_t result;
    if (!context->ObjectToWholeNumber(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(wholenumber_t,             // Return type
            TestObjectToWholeNumberAlt,  // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    wholenumber_t result;
    if (!context->WholeNumber(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(stringsize_t,              // Return type
            TestObjectToStringSize,     // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    stringsize_t result;
    if (!context->ObjectToStringSize(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(stringsize_t,              // Return type
            TestObjectToStringSizeAlt, // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    stringsize_t result;
    if (!context->StringSize(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(int64_t,                   // Return type
            TestObjectToInt64,          // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    int64_t result;
    if (!context->ObjectToInt64(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(int64_t,                   // Return type
            TestObjectToInt64Alt,          // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    int64_t result;
    if (!context->Int64(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(uint64_t,                   // Return type
            TestObjectToUnsignedInt64,  // Function routine name
            RexxObjectPtr, arg1)        // Argument
{
    uint64_t result;
    if (!context->ObjectToUnsignedInt64(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(uint64_t,                   // Return type
            TestObjectToUnsignedInt64Alt,  // Function routine name
            RexxObjectPtr, arg1)        // Argument
{
    uint64_t result;
    if (!context->UnsignedInt64(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(int32_t,                   // Return type
            TestObjectToInt32,          // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    int32_t result;
    if (!context->ObjectToInt32(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(int32_t,                   // Return type
            TestObjectToInt32Alt,      // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    int32_t result;
    if (!context->Int32(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(uint32_t,                   // Return type
            TestObjectToUnsignedInt32,  // Function routine name
            RexxObjectPtr, arg1)        // Argument
{
    uint32_t result;
    if (!context->ObjectToUnsignedInt32(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(uint32_t,                   // Return type
            TestObjectToUnsignedInt32Alt,  // Function routine name
            RexxObjectPtr, arg1)        // Argument
{
    uint32_t result;
    if (!context->UnsignedInt32(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(intptr_t,                  // Return type
            TestObjectToIntptr,         // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    intptr_t result;
    if (!context->ObjectToIntptr(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(intptr_t,                  // Return type
            TestObjectToIntptrAlt,     // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    intptr_t result;
    if (!context->Intptr(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(uintptr_t,                  // Return type
            TestObjectToUintptr,        // Function routine name
            RexxObjectPtr, arg1)        // Argument
{
    uintptr_t result;
    if (!context->ObjectToUintptr(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(uintptr_t,                  // Return type
            TestObjectToUintptrAlt,     // Function routine name
            RexxObjectPtr, arg1)        // Argument
{
    uintptr_t result;
    if (!context->Uintptr(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(logical_t,                 // Return type
            TestObjectToLogical,        // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    logical_t result;
    if (!context->ObjectToLogical(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(logical_t,                 // Return type
            TestObjectToLogicalAlt,        // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    logical_t result;
    if (!context->Logical(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0;
    }

    return result;
}

RexxMethod1(double,                    // Return type
            TestObjectToDouble,         // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    double result;
    if (!context->ObjectToDouble(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0.0;
    }

    return result;
}

RexxMethod1(double,                    // Return type
            TestObjectToDoubleAlt,     // Function routine name
            RexxObjectPtr, arg1)       // Argument
{
    double result;
    if (!context->Double(arg1, &result))
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return 0.0;
    }

    return result;
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestWholeNumberToObject,    // Function routine name
            wholenumber_t, arg1)       // Argument
{
    return context->WholeNumberToObject(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestWholeNumberToObjectAlt, // Function routine name
            wholenumber_t, arg1)       // Argument
{
    return context->WholeNumber(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestStringSizeToObject,    // Function routine name
            stringsize_t, arg1)        // Argument
{
    return context->StringSizeToObject(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestStringSizeToObjectAlt, // Function routine name
            stringsize_t, arg1)        // Argument
{
    return context->StringSize(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestInt64ToObject,         // Function routine name
            int64_t,      arg1)        // Argument
{
    return context->Int64ToObject(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestInt64ToObjectAlt,      // Function routine name
            int64_t,      arg1)        // Argument
{
    return context->Int64(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestUnsignedInt64ToObject, // Function routine name
            uint64_t,     arg1)        // Argument
{
    return context->UnsignedInt64ToObject(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestUnsignedInt64ToObjectAlt, // Function routine name
            uint64_t,     arg1)        // Argument
{
    return context->UnsignedInt64(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestInt32ToObject,         // Function routine name
            int32_t,      arg1)        // Argument
{
    return context->Int32ToObject(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestInt32ToObjectAlt,      // Function routine name
            int32_t,      arg1)        // Argument
{
    return context->Int32(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestUnsignedInt32ToObject, // Function routine name
            uint32_t,     arg1)        // Argument
{
    return context->UnsignedInt32ToObject(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestUnsignedInt32ToObjectAlt, // Function routine name
            uint32_t,     arg1)        // Argument
{
    return context->UnsignedInt32(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestIntptrToObject,        // Function routine name
            intptr_t,      arg1)       // Argument
{
    return context->IntptrToObject(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestIntptrToObjectAlt,     // Function routine name
            intptr_t,      arg1)       // Argument
{
    return context->Intptr(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestUintptrToObject,       // Function routine name
            uintptr_t,      arg1)      // Argument
{
    return context->UintptrToObject(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestUintptrToObjectAlt,    // Function routine name
            uintptr_t,      arg1)      // Argument
{
    return context->Uintptr(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestLogicalToObject,       // Function routine name
            logical_t,      arg1)      // Argument
{
    return context->LogicalToObject(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestLogicalToObjectAlt,    // Function routine name
            logical_t,      arg1)      // Argument
{
    return context->Logical(arg1);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestDoubleToObject,       // Function routine name
            double,         arg1)      // Argument
{
    return context->DoubleToObject(arg1);
}

RexxMethod2(RexxObjectPtr,
            TestDoubleToObjectWithPrecision,
            double,         number,
            size_t,         precision)
{
    return context->DoubleToObjectWithPrecision(number, precision);
}

RexxMethod1(RexxObjectPtr,             // Return type
            TestDoubleToObjectAlt,     // Function routine name
            double,         arg1)      // Argument
{
    return context->Double(arg1);
}

RexxMethod2(RexxObjectPtr,
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

RexxMethod1(RexxArrayObject, TestGetArguments,
            ARGLIST, arg1)          // unused dummy argument that allows this to be invoked with variable args.
{
    return context->GetArguments();
}

RexxMethod2(RexxObjectPtr,
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

RexxMethod0(CSTRING,
            TestGetMessageName)
{
    return context->GetMessageName();
}

RexxMethod0(RexxObjectPtr,
            TestGetMethod)
{
    return (RexxObjectPtr)context->GetMethod();
}

RexxMethod0(RexxObjectPtr,
            TestGetSelf)
{
    return (RexxObjectPtr)context->GetSelf();
}

RexxMethod0(RexxClassObject,
            TestGetSuper)
{
    return context->GetSuper();
}

RexxMethod0(RexxObjectPtr,
            TestGetScope)
{
    return context->GetScope();
}

RexxMethod2(int,
            TestSetObjectVariable,
            CSTRING, name,
            RexxObjectPtr, value)
{
    context->SetObjectVariable(name, value);
    return 0;
}

RexxMethod1(RexxObjectPtr,
            TestGetObjectVariable,
            CSTRING, name)
{
    RexxObjectPtr value = context->GetObjectVariable(name);
    if (value == NULLOBJECT) {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return value;
}

RexxMethod1(RexxObjectPtr,
            TestGetObjectVariableOrNil,
            CSTRING, name)
{
    RexxObjectPtr value = context->GetObjectVariable(name);
    if (value == NULLOBJECT)
    {
        return context->Nil();
    }
    return value;
}

RexxMethod1(int,
            TestDropObjectVariable,
            CSTRING, name)
{
    context->DropObjectVariable(name);
    return 0;
}


RexxMethod4(RexxObjectPtr,
            TestForwardMessage,
            RexxObjectPtr, to,
            CSTRING, name,
            RexxClassObject, super,
            RexxArrayObject, args)
{
    return context->ForwardMessage(to, name, super, args);
}


RexxMethod1(RexxClassObject,
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


RexxMethod1(RexxStemObject,
            TestNewStem,
            CSTRING, name)
{
    return context->NewStem(name);
}

RexxMethod3(int,
            TestSetStemElement,
            RexxStemObject, stem,
            CSTRING, tail,
            RexxObjectPtr, value)
{
    context->SetStemElement(stem, tail, value);
    return 0;
}

RexxMethod2(RexxObjectPtr,
            TestGetStemElement,
            RexxStemObject, stem,
            CSTRING, tail)
{
    RexxObjectPtr value = context->GetStemElement(stem, tail);
    if (value == NULLOBJECT) {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return value;
}

RexxMethod2(int,
            TestDropStemElement,
            RexxStemObject, stem,
            CSTRING, tail)
{
    context->DropStemElement(stem, tail);
    return 0;
}

RexxMethod3(int,
            TestSetStemArrayElement,
            RexxStemObject, stem,
            size_t, tail,
            RexxObjectPtr, value)
{
    context->SetStemArrayElement(stem, tail, value);
    return 0;
}

RexxMethod2(RexxObjectPtr,
            TestGetStemArrayElement,
            RexxStemObject, stem,
            size_t, tail)
{
    RexxObjectPtr value = context->GetStemArrayElement(stem, tail);
    if (value == NULLOBJECT) {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return value;
}

RexxMethod2(int,
            TestDropStemArrayElement,
            RexxStemObject, stem,
            size_t, tail)
{
    context->DropStemArrayElement(stem, tail);
    return 0;
}

RexxMethod1(RexxObjectPtr,
            TestGetAllStemElements,
            RexxStemObject, stem)
{
    return (RexxObjectPtr)context->GetAllStemElements(stem);
}

RexxMethod1(RexxObjectPtr,
            TestGetStemValue,
            RexxStemObject, stem)
{
    return context->GetStemValue(stem);
}

RexxMethod1(logical_t,
            TestIsStem,
            RexxObjectPtr, stem)
{
    return context->IsStem(stem);
}

RexxMethod1(RexxObjectPtr,
            TestSupplierItem,
            RexxObjectPtr, supplier)
{
    return context->SupplierItem((RexxSupplierObject)supplier);
}

RexxMethod1(RexxObjectPtr,
            TestSupplierIndex,
            RexxObjectPtr, supplier)
{
    return context->SupplierIndex((RexxSupplierObject)supplier);
}

RexxMethod1(logical_t,
            TestSupplierAvailable,
            RexxObjectPtr, supplier)
{
    return context->SupplierAvailable((RexxSupplierObject)supplier);
}

RexxMethod1(int,
            TestSupplierNext,
            RexxObjectPtr, supplier)
{
    context->SupplierNext((RexxSupplierObject)supplier);
    return 0;
}


RexxMethod2(RexxObjectPtr,
            TestNewSupplier,
            RexxArrayObject, values,
            RexxArrayObject, indexes)
{
    return (RexxObjectPtr)context->NewSupplier(values, indexes);
}


RexxMethod2(RexxObjectPtr,
            TestArrayAt,
            RexxArrayObject, array,
            size_t, index)
{
    RexxObjectPtr value = context->ArrayAt(array, index);
    if (value == NULLOBJECT) {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return value;
}


RexxMethod3(int,
            TestArrayPut,
            RexxArrayObject, array,
            RexxObjectPtr, value,
            size_t, index)
{
    context->ArrayPut(array, value, index);
    return 0;
}


RexxMethod2(size_t,
            TestArrayAppend,
            RexxArrayObject, array,
            RexxObjectPtr, value)
{
    return context->ArrayAppend(array, value);
}


RexxMethod2(size_t,
            TestArrayAppendString,
            RexxArrayObject, array,
            CSTRING, value)
{
    return context->ArrayAppendString(array, value, strlen(value));
}


RexxMethod1(size_t,
            TestArraySize,
            RexxArrayObject, array)
{
    return context->ArraySize(array);
}


RexxMethod1(size_t,
            TestArrayItems,
            RexxArrayObject, array)
{
    return context->ArrayItems(array);
}


RexxMethod1(size_t,
            TestArrayDimension,
            RexxObjectPtr, array)
{
    return context->ArrayDimension((RexxArrayObject)array);
}


RexxMethod1(RexxArrayObject,
            TestNewArray,
            size_t, size)
{
    return context->NewArray(size);
}


RexxMethod1(RexxArrayObject,
            TestArrayOfOne,
            RexxObjectPtr, arg1)
{
    return context->ArrayOfOne(arg1);
}


RexxMethod2(RexxArrayObject,
            TestArrayOfTwo,
            RexxObjectPtr, arg1,
            RexxObjectPtr, arg2)
{
    return context->ArrayOfTwo(arg1, arg2);
}


RexxMethod3(RexxArrayObject,
            TestArrayOfThree,
            RexxObjectPtr, arg1,
            RexxObjectPtr, arg2,
            RexxObjectPtr, arg3)
{
    return context->ArrayOfThree(arg1, arg2, arg3);
}


RexxMethod4(RexxArrayObject,
            TestArrayOfFour,
            RexxObjectPtr, arg1,
            RexxObjectPtr, arg2,
            RexxObjectPtr, arg3,
            RexxObjectPtr, arg4)
{
    return context->ArrayOfFour(arg1, arg2, arg3, arg4);
}


RexxMethod1(RexxArrayObject,
            TestArrayOfOneAlt,
            RexxObjectPtr, arg1)
{
    return context->Array(arg1);
}


RexxMethod2(RexxArrayObject,
            TestArrayOfTwoAlt,
            RexxObjectPtr, arg1,
            RexxObjectPtr, arg2)
{
    return context->Array(arg1, arg2);
}


RexxMethod3(RexxArrayObject,
            TestArrayOfThreeAlt,
            RexxObjectPtr, arg1,
            RexxObjectPtr, arg2,
            RexxObjectPtr, arg3)
{
    return context->Array(arg1, arg2, arg3);
}


RexxMethod4(RexxArrayObject,
            TestArrayOfFourAlt,
            RexxObjectPtr, arg1,
            RexxObjectPtr, arg2,
            RexxObjectPtr, arg3,
            RexxObjectPtr, arg4)
{
    return context->Array(arg1, arg2, arg3, arg4);
}

RexxMethod1(logical_t,
            TestIsArray,
            RexxObjectPtr, o)
{
    return context->IsArray(o);
}

RexxMethod3(int,
            TestDirectoryPut,
            RexxObjectPtr, dir,
            RexxObjectPtr, value,
            CSTRING, index)
{
    context->DirectoryPut((RexxDirectoryObject)dir, value, index);
    return 0;
}


RexxMethod2(RexxObjectPtr,
            TestDirectoryAt,
            RexxObjectPtr, dir,
            CSTRING, index)
{
    RexxObjectPtr value = context->DirectoryAt((RexxDirectoryObject)dir, index);
    if (value == NULLOBJECT) {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return value;
}


RexxMethod2(RexxObjectPtr,
            TestDirectoryRemove,
            RexxObjectPtr, dir,
            CSTRING, index)
{
    RexxObjectPtr value = context->DirectoryRemove((RexxDirectoryObject)dir, index);
    if (value == NULLOBJECT) {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return value;
}


RexxMethod0(RexxObjectPtr,
            TestNewDirectory)
{
    return (RexxObjectPtr)context->NewDirectory();
}

RexxMethod1(logical_t,
            TestIsDirectory,
            RexxObjectPtr, o)
{
    return context->IsDirectory(o);
}

// StringTable tests, added with 5.0

RexxMethod3(int,
            TestStringTablePut,
            RexxObjectPtr, st,
            RexxObjectPtr, value,
            CSTRING, index)
{
    context->StringTablePut((RexxStringTableObject)st, value, index);
    return 0;
}


RexxMethod2(RexxObjectPtr,
            TestStringTableAt,
            RexxObjectPtr, st,
            CSTRING, index)
{
    RexxObjectPtr value = context->StringTableAt((RexxStringTableObject)st, index);
    if (value == NULLOBJECT) {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return value;
}


RexxMethod2(RexxObjectPtr,
            TestStringTableRemove,
            RexxObjectPtr, st,
            CSTRING, index)
{
    RexxObjectPtr value = context->StringTableRemove((RexxStringTableObject)st, index);
    if (value == NULLOBJECT) {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        return NULLOBJECT;
    }
    return value;
}


RexxMethod0(RexxObjectPtr,
            TestNewStringTable)
{
    return (RexxObjectPtr)context->NewStringTable();
}

RexxMethod1(logical_t,
            TestIsStringTable,
            RexxObjectPtr, o)
{
    return context->IsStringTable(o);
}

RexxMethod1(int,                       // Return type
            TestOptionalIntArg,        // Object_method name
            OPTIONAL_int, arg1)        // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(int32_t,                    // Return type
            TestOptionalInt32Arg,       // Function routine name
            OPTIONAL_int32_t, arg1)     // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(uint32_t,                   // Return type
            TestOptionalUint32Arg,      // Function routine name
            OPTIONAL_uint32_t, arg1)    // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(int8_t,                     // Return type
            TestOptionalInt8Arg,        // Function routine name
            OPTIONAL_int8_t, arg1)      // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(uint8_t,                   // Return type
            TestOptionalUint8Arg,      // Function routine name
            OPTIONAL_uint8_t, arg1)    // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(int16_t,                   // Return type
            TestOptionalInt16Arg,      // Function routine name
            OPTIONAL_int16_t, arg1)    // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(uint16_t,                  // Return type
            TestOptionalUint16Arg,     // Function routine name
            OPTIONAL_uint16_t, arg1)   // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(int64_t,                   // Return type
            TestOptionalInt64Arg,      // Function routine name
            OPTIONAL_int64_t, arg1)    // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(uint64_t,                  // Return type
            TestOptionalUint64Arg,     // Function routine name
            OPTIONAL_uint64_t, arg1)   // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(intptr_t,                  // Return type
            TestOptionalIntPtrArg,     // Function routine name
            OPTIONAL_intptr_t, arg1)   // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(uintptr_t,                 // Return type
            TestOptionalUintPtrArg,    // Function routine name
            OPTIONAL_uintptr_t, arg1)  // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(wholenumber_t,             // Return type
            TestOptionalWholeNumberArg, // Function routine name
            OPTIONAL_wholenumber_t, arg1)       // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(positive_wholenumber_t,             // Return type
            TestOptionalPositiveWholeNumberArg, // Function routine name
            OPTIONAL_positive_wholenumber_t, arg1)       // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(wholenumber_t,             // Return type
            TestOptionalNonnegativeWholeNumberArg, // Function routine name
            OPTIONAL_nonnegative_wholenumber_t, arg1)       // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}


RexxMethod1(stringsize_t,              // Return type
            TestOptionalStringSizeArg,          // Function routine name
            OPTIONAL_stringsize_t, arg1)        // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(size_t,                     // Return type
            TestOptionalSizeArg,                // Function routine name
            OPTIONAL_size_t, arg1)               // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(ssize_t,                     // Return type
            TestOptionalSSizeArg,        // Function routine name
            OPTIONAL_ssize_t, arg1)      // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(logical_t,                 // Return type
            TestOptionalLogicalArg,    // Function routine name
            OPTIONAL_logical_t, arg1)           // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(float,                     // Return type
            TestOptionalFloatArg,               // Function routine name
            OPTIONAL_float, arg1)               // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0.0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(double,                    // Return type
            TestOptionalDoubleArg,     // Function routine name
            OPTIONAL_double, arg1)     // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != 0.0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(CSTRING,                   // Return type
            TestOptionalCstringArg,    // Function routine name
            OPTIONAL_CSTRING, arg1)    // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != NULL)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(POINTER,                   // Return type
           TestOptionalPointerArg,     // Function routine name
           OPTIONAL_POINTER, arg1)     // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != NULL)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(POINTER,                             // Return type
           TestOptionalPointerStringArg,         // Function routine name
           OPTIONAL_POINTERSTRING, arg1)         // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != NULL)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(RexxStemObject,              // Return type
           TestOptionalStemArg,          // Function routine name
           OPTIONAL_RexxStemObject, arg1)         // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != NULLOBJECT)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(RexxObjectPtr,               // Return type
           TestOptionalObjectArg,        // Function routine name
           OPTIONAL_RexxObjectPtr, arg1) // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != NULLOBJECT)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(RexxStringObject,            // Return type
           TestOptionalStringArg,        // Function routine name
           OPTIONAL_RexxStringObject, arg1)       // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != NULLOBJECT)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(RexxArrayObject,             // Return type
           TestOptionalArrayArg,         // Function routine name
           OPTIONAL_RexxArrayObject, arg1)        // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != NULLOBJECT)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod1(RexxClassObject,             // Return type
            TestOptionalClassArg,         // Function routine name
            OPTIONAL_RexxClassObject, arg1)        // Argument
{
    if (argumentOmitted(1))
    {
        if (arg1 != NULLOBJECT)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Conversion error"));
        }
    }
    return arg1;
}

RexxMethod3(RexxObjectPtr,
            TestSendMessage,
            RexxObjectPtr, target,
            CSTRING, name,
            RexxArrayObject, args)
{
    return context->SendMessage(target, name, args);
}

RexxMethod4(RexxObjectPtr,
            TestSendMessageScoped,
            RexxObjectPtr, target,
            CSTRING, name,
            RexxClassObject, scope,
            RexxArrayObject, args)
{
    return context->SendMessageScoped(target, name, scope, args);
}


RexxMethod2(RexxObjectPtr,
            TestSendMessage0,
            RexxObjectPtr, target,
            CSTRING, name)
{
    return context->SendMessage0(target, name);
}

RexxMethod3(RexxObjectPtr,
            TestSendMessage1,
            RexxObjectPtr, target,
            CSTRING, name,
            RexxObjectPtr, arg1)
{
    return context->SendMessage1(target, name, arg1);
}

RexxMethod4(RexxObjectPtr,
            TestSendMessage2,
            RexxObjectPtr, target,
            CSTRING, name,
            RexxObjectPtr, arg1,
            RexxObjectPtr, arg2)
{
    return context->SendMessage2(target, name, arg1, arg2);
}


RexxMethod2(logical_t,
            TestIsInstanceOf,
            RexxObjectPtr, arg1,
            RexxClassObject, arg2)
{
    return context->IsInstanceOf(arg1, arg2);
}


RexxMethod2(logical_t,
            TestHasMethod,
            RexxObjectPtr, arg1,
            CSTRING, name)
{
    return context->HasMethod(arg1, name);
}


RexxMethod1(RexxObjectPtr,
            TestLoadPackage,
            CSTRING, name)
{
    return (RexxObjectPtr)context->LoadPackage(name);
}

RexxMethod2(RexxObjectPtr,
            TestLoadPackageFromData,
            CSTRING, name,
            RexxStringObject, data)
{
    return (RexxObjectPtr)context->LoadPackageFromData(name, context->StringData(data), context->StringLength(data));
}


RexxMethod1(RexxClassObject,
            TestFindClass,
            CSTRING, name)
{
    return context->FindClass(name);
}


RexxMethod2(RexxClassObject,
            TestFindPackageClass,
            RexxObjectPtr, package,
            CSTRING, name)
{
    return context->FindPackageClass((RexxPackageObject)package, name);
}


RexxMethod1(RexxObjectPtr,
            TestGetPackageRoutines,
            RexxObjectPtr, package)
{
    return (RexxObjectPtr)context->GetPackageRoutines((RexxPackageObject)package);
}


RexxMethod1(RexxObjectPtr,
            TestGetPackagePublicRoutines,
            RexxObjectPtr, package)
{
    return (RexxObjectPtr)context->GetPackagePublicRoutines((RexxPackageObject)package);
}


RexxMethod1(RexxObjectPtr,
            TestGetPackageClasses,
            RexxObjectPtr, package)
{
    return (RexxObjectPtr)context->GetPackageClasses((RexxPackageObject)package);
}


RexxMethod1(RexxObjectPtr,
            TestGetPackagePublicClasses,
            RexxObjectPtr, package)
{
    return (RexxObjectPtr)context->GetPackagePublicClasses((RexxPackageObject)package);
}


RexxMethod1(RexxObjectPtr,
            TestGetPackageMethods,
            RexxObjectPtr, package)
{
    return (RexxObjectPtr)context->GetPackageMethods((RexxPackageObject)package);
}

RexxMethod2(RexxObjectPtr,
            TestCallRoutine,
            RexxObjectPtr, routine,
            RexxArrayObject, args)
{
    return context->CallRoutine((RexxRoutineObject)routine, args);
}

RexxMethod2(RexxObjectPtr,
            TestCallProgram,
            CSTRING, name,
            RexxArrayObject, args)
{
    return context->CallProgram(name, args);
}

RexxMethod2(RexxObjectPtr,
            TestNewMethod,
            CSTRING, name,
            RexxStringObject, data)
{
    return (RexxObjectPtr)context->NewMethod(name, context->StringData(data), context->StringLength(data));
}

RexxMethod2(RexxObjectPtr,
            TestNewRoutine,
            CSTRING, name,
            RexxStringObject, data)
{
    return (RexxObjectPtr)context->NewRoutine(name, context->StringData(data), context->StringLength(data));
}

RexxMethod1(logical_t,
            TestIsMethod,
            RexxObjectPtr, target)
{
    return context->IsMethod(target);
}

RexxMethod1(logical_t,
            TestIsRoutine,
            RexxObjectPtr, target)
{
    return context->IsRoutine(target);
}

RexxMethod1(RexxObjectPtr,
            TestGetMethodPackage,
            RexxObjectPtr, target)
{
    return (RexxObjectPtr)context->GetMethodPackage((RexxMethodObject)target);
}

RexxMethod1(RexxObjectPtr,
            TestGetRoutinePackage,
            RexxObjectPtr, target)
{
    return (RexxObjectPtr)context->GetRoutinePackage((RexxRoutineObject)target);
}

RexxMethod1(RexxStringObject,
            TestObjectToString,
            RexxObjectPtr, target)
{
    return context->ObjectToString(target);
}

RexxMethod3(RexxStringObject,
            TestStringGet,
            RexxStringObject, source,
            stringsize_t, offset,
            stringsize_t, length)
{
    char *buffer = (char *)malloc(length + 16);
    context->StringGet(source, offset, buffer, length);
    RexxStringObject result = context->NewString(buffer, length);
    free(buffer);
    return result;
}

RexxMethod1(stringsize_t,
            TestStringLength,
            RexxStringObject, target)
{
    return context->StringLength(target);
}


RexxMethod1(CSTRING,
            TestStringData,
            RexxStringObject, target)
{
    return context->StringData(target);
}


RexxMethod1(RexxStringObject,
            TestStringUpper,
            RexxStringObject, target)
{
    return context->StringUpper(target);
}

RexxMethod1(RexxStringObject,
            TestStringLower,
            RexxStringObject, target)
{
    return context->StringLower(target);
}

RexxMethod1(logical_t,
            TestIsString,
            RexxObjectPtr, target)
{
    return context->IsString(target);
}

RexxMethod1(RexxStringObject,
            TestNewStringFromAsciiz,
            CSTRING, source)
{
    return context->NewStringFromAsciiz(source);
}

RexxMethod1(RexxStringObject,
            TestNewStringFromAsciizAlt,
            CSTRING, source)
{
    return context->String(source);
}

RexxMethod1(RexxStringObject,
            TestCStringToObject,
            CSTRING, source)
{
    return context->CString(source);
}

RexxMethod1(CSTRING,
            TestObjectToCString,
            RexxObjectPtr, source)
{
    return context->CString(source);
}

RexxMethod2(RexxStringObject,
            TestNewString,
            CSTRING, source,
            size_t, len)
{
    return context->NewString(source, len);
}

RexxMethod2(RexxStringObject,
            TestNewStringAlt,
            CSTRING, source,
            size_t, len)
{
    return context->String(source, len);
}

RexxMethod1(int,
            TestRaiseException0,
            size_t, errNo)
{
    context->RaiseException0(errNo);
    // this should still get executed
    context->SetObjectVariable("CONTINUE", context->True());
    return 0;
}

RexxMethod2(int,
            TestRaiseException1,
            size_t, errNo,
            RexxObjectPtr, sub1)
{
    context->RaiseException1(errNo, sub1);
    // this should still get executed
    context->SetObjectVariable("CONTINUE", context->True());
    return 0;
}

RexxMethod3(int,
            TestRaiseException2,
            size_t, errNo,
            RexxObjectPtr, sub1,
            RexxObjectPtr, sub2)
{
    context->RaiseException2(errNo, sub1, sub2);
    // this should still get executed
    context->SetObjectVariable("CONTINUE", context->True());
    return 0;
}

RexxMethod2(int,
            TestRaiseException,
            size_t, errNo,
            RexxArrayObject, subs)
{
    context->RaiseException(errNo, subs);
    // this should still get executed
    context->SetObjectVariable("CONTINUE", context->True());
    return 0;
}

RexxMethod4(int,
            TestRaiseCondition,
            CSTRING, name,
            OPTIONAL_CSTRING, desc,
            OPTIONAL_RexxArrayObject, add,
            OPTIONAL_RexxObjectPtr, result)
{
    context->RaiseCondition(name, (RexxStringObject)desc, add, result);
    // this should still get executed
    context->SetObjectVariable("CONTINUE", context->True());
    return 0;
}

RexxMethod1(int,
            TestThrowException0,
            size_t, errNo)
{
    context->ThrowException0(errNo);
    // this should never execute
    context->SetObjectVariable("CONTINUE", context->True());
    return 0;
}

RexxMethod2(int,
            TestThrowException1,
            size_t, errNo,
            RexxObjectPtr, sub1)
{
    context->ThrowException1(errNo, sub1);
    // this should never execute
    context->SetObjectVariable("CONTINUE", context->True());
    return 0;
}

RexxMethod3(int,
            TestThrowException2,
            size_t, errNo,
            RexxObjectPtr, sub1,
            RexxObjectPtr, sub2)
{
    context->ThrowException2(errNo, sub1, sub2);
    // this should never execute
    context->SetObjectVariable("CONTINUE", context->True());
    return 0;
}

RexxMethod2(int,
            TestThrowException,
            size_t, errNo,
            RexxArrayObject, subs)
{
    context->ThrowException(errNo, subs);
    // this should never execute
    context->SetObjectVariable("CONTINUE", context->True());
    return 0;
}

RexxMethod4(int,
            TestThrowCondition,
            CSTRING, name,
            OPTIONAL_CSTRING, desc,
            OPTIONAL_RexxArrayObject, add,
            OPTIONAL_RexxObjectPtr, result)
{
    context->ThrowCondition(name, (RexxStringObject)desc, add, result);
    // this should never execute
    context->SetObjectVariable("CONTINUE", context->True());
    return 0;
}

RexxMethod1(int,
            TestBufferInit,
            int, initValue)
{
    RexxBufferObject buffer = context->NewBuffer(sizeof(int));
    int *ptr = (int *)context->BufferData(buffer);
    *ptr = initValue;
    context->SetObjectVariable("CSELF", (RexxObjectPtr)buffer);
    return 0;
}

RexxMethod1(int,
            TestBufferCSelf,
            CSELF, self)
{
    return *((int *)self);
}

RexxMethod0(RexxObjectPtr,
            TestGetBuffer)
{
    return context->GetObjectVariable("CSELF");
}

RexxMethod1(int,
            TestBufferObjectToCSelf,
            OSELF, self)
{
    int *ptr = (int *)context->ObjectToCSelf(self);
    return *ptr;
}

RexxMethod1(logical_t,
            TestIsBuffer,
            RexxObjectPtr, target)
{
    return context->IsBuffer(target);
}

RexxMethod0(size_t,
            TestInterpreterVersion)
{
    size_t version = context->InterpreterVersion();
    // this validates that the headers are in sync with the interpreter build
    if (version != REXX_CURRENT_INTERPRETER_VERSION)
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Interpreter version mismatch"));
    }
    return version;
}

RexxMethod0(size_t,
            TestLanguageLevel)
{
    size_t version = context->LanguageLevel();
    // this validates that the headers are in sync with the interpreter build
    if (version != REXX_CURRENT_LANGUAGE_LEVEL)
    {
        context->RaiseException1(Rexx_Error_Invalid_argument_user_defined, context->NewStringFromAsciiz("Language level mismatch"));
    }
    return version;
}

RexxMethod1(RexxMutableBufferObject, TestNewMutableBuffer, size_t, l)
{
    return context->NewMutableBuffer(l);
}

RexxMethod1(size_t, TestMutableBufferLength, RexxMutableBufferObject, b)
{
    return context->MutableBufferLength(b);
}

RexxMethod2(size_t, TestSetMutableBufferLength, RexxMutableBufferObject, b, size_t, l)
{
    return context->SetMutableBufferLength(b, l);
}

RexxMethod1(size_t, TestMutableBufferCapacity, RexxMutableBufferObject, b)
{
    return context->MutableBufferCapacity(b);
}

RexxMethod2(size_t, TestSetMutableBufferCapacity, RexxMutableBufferObject, b, size_t, l)
{
    context->SetMutableBufferCapacity(b, l);
    return context->MutableBufferCapacity(b);
}

RexxMethod1(logical_t, TestIsMutableBuffer, RexxObjectPtr, b)
{
    return context->IsMutableBuffer(b);
}

RexxMethod2(RexxMutableBufferObject, TestSetMutableBufferValue, RexxMutableBufferObject, b, CSTRING, newValue)
{
    size_t l = strlen(newValue);
    char * buffer = (char *)context->SetMutableBufferCapacity(b, l);
    // must set the length first if we are extending, as the set
    // will pad with nulls if it goes longer.
    context->SetMutableBufferLength(b, l);
    memcpy(buffer, newValue, l);
    return b;
}

RexxMethod1(RexxStringObject, TestGetMutableBufferValue, RexxMutableBufferObject, b)
{
    size_t l = context->MutableBufferLength(b);
    char * buffer = (char *)context->MutableBufferData(b);
    return context->NewString(buffer, l);
}

RexxMethod1(positive_wholenumber_t,     // Return type
            TestPositiveWholeNumberArg, // Function routine name
            positive_wholenumber_t, arg1) // Argument
{
    return arg1;
}

// [feature-requests:#634] Add a per-object storage management API

// POINTER (RexxEntry *AllocateObjectMemory)(RexxMethodContext *, size_t);
RexxMethod1(POINTER,                   // Return type
            TestAllocateObjectMemory,  // Function name
            size_t, bytes)             // Argument
{
    return context->AllocateObjectMemory(bytes);
}

// void    (RexxEntry *FreeObjectMemory)(RexxMethodContext *, POINTER);
RexxMethod1(RexxObjectPtr,             // Return type
            TestFreeObjectMemory,      // Function name
            POINTER, ptr)              // Argument
{
    context->FreeObjectMemory(ptr);
    return NULLOBJECT;
}

// POINTER (RexxEntry *ReallocateObjectMemory)(RexxMethodContext *, POINTER, size_t);
RexxMethod2(POINTER,                   // Return type
            TestReallocateObjectMemory,// Function name
            POINTER, ptr,              // Argument
            size_t, bytes)             // Argument
{
    return context->ReallocateObjectMemory(ptr, bytes);
}

// helper: set object memory content; must be null-terminated
RexxMethod2(RexxObjectPtr,             // Return type
            TestSetObjectMemory,       // Function name
            POINTER, ptr,              // Argument
            CSTRING, data)             // Argument
{
    memcpy(ptr, data, strlen(data));
    return NULLOBJECT;
}

// helper: get object memory content; must be null-terminated
RexxMethod1(CSTRING,                   // Return type
            TestGetObjectMemory,       // Function name
            POINTER, ptr)              // Argument
{
    return (char *)ptr;
}

// RexxVariableReferenceObject (RexxEntry *GetObjectVariableReference)(RexxMethodContext *, CSTRING);
RexxMethod1(RexxVariableReferenceObject,    // Return type
            TestGetObjectVariableReference, // Function name
            CSTRING, name)                  // Argument
{
    return context->GetObjectVariableReference(name);
}

// logical_t (RexxEntry *IsVariableReference)(RexxThreadContext *, RexxObjectPtr);
RexxMethod1(logical_t,                      // Return type
            TestIsVariableReference,        // Function name
            RexxObjectPtr, o)               // Argument
{
    return context->IsVariableReference(o);
}

// void (RexxEntry *SetVariableReferenceValue)(RexxThreadContext *, RexxVariableReferenceObject, RexxObjectPtr);
RexxMethod2(RexxObjectPtr,                  // Return type
            TestSetVariableReferenceValue,  // Function name
            RexxVariableReferenceObject, o, // Argument
            RexxObjectPtr, val)             // Argument
{
    context->SetVariableReferenceValue((RexxVariableReferenceObject)o, val);
    return NULLOBJECT;
}

// RexxStringObject (RexxEntry *VariableReferenceName)(RexxThreadContext *, RexxVariableReferenceObject);
RexxMethod1(RexxStringObject,               // Return type
            TestVariableReferenceName,      // Function name
            RexxVariableReferenceObject, o) // Argument
{
    return context->VariableReferenceName((RexxVariableReferenceObject)o);
}

// RexxObjectPtr (RexxEntry *VariableReferenceValue)(RexxThreadContext *, RexxVariableReferenceObject);
RexxMethod1(RexxObjectPtr,                  // Return type
            TestVariableReferenceValue,     // Function name
            RexxVariableReferenceObject, o) // Argument
{
    return context->VariableReferenceValue((RexxVariableReferenceObject)o);
}


// test SetGuard APIs - preliminary stuff
RexxMethod1(RexxObjectPtr,
            TestSetGuard,
            ARGLIST, args)
{
//printf("TestSetGuard: items %zd\n", context->ArrayItems(args));
    for (size_t i = 1; i <= context->ArrayItems(args); i++)
    {
        // must be a String, no error checking
        RexxObjectPtr item = context->ArrayAt(args, i);
        if (item == NULLOBJECT)
            continue;
        RexxStringObject command = (RexxStringObject)context->SendMessage0(item, "string");
        command = context->StringUpper(command);
        char *cmd = (char *)context->StringData(command);
        if (strcmp(cmd, "GUARD ON") == 0)
        {
//printf("TestSetGuard: %zd. SetGuardOn()\n", i);
            context->SetGuardOn();
        }
        else if (strcmp(cmd, "GUARD OFF") == 0)
        {
//printf("TestSetGuard: %zd. SetGuardOff()\n", i);
            context->SetGuardOff();
        }
        else if (strchr(cmd, '=') != NULL)
        {
            char *value = strchr(cmd, '=');
            *value++ = '\0';
//printf("TestSetGuard: %zd. SetObjectVariable(%s, %s)\n", i, cmd, value);
            context->SetObjectVariable(cmd, context->NewStringFromAsciiz(value));
        }
//      else printf("TestSetGuard: invalid command %s\n", cmd);
    }
    return NULLOBJECT;
}

// void RexxEntry SetGuardOn(RexxMethodContext *c)
// to facilitate testing we need to also set an object variable
RexxMethod2(RexxObjectPtr,
            TestSetGuardOn,
            CSTRING, name,
            RexxObjectPtr, value)
{
    context->SetGuardOn();
    context->SetObjectVariable(name, value);
    return NULLOBJECT;
}

// void RexxEntry SetGuardOff(RexxMethodContext *c)
// to facilitate testing we need to also set an object variable
RexxMethod2(RexxObjectPtr,
            TestSetGuardOff,
            CSTRING, name,
            RexxObjectPtr, value)
{
    context->SetGuardOn();
    context->SetGuardOff();
    context->SetObjectVariable(name, value);
    return NULLOBJECT;
}

// RexxObjectPtr RexxEntry SetGuardOnWhenUpdated(RexxMethodContext *c, CSTRING n)
RexxMethod1(RexxObjectPtr,
            TestSetGuardOnWhenUpdated,
            CSTRING, n)
{
    return context->SetGuardOnWhenUpdated(n);
}

// RexxObjectPtr RexxEntry SetGuardOffWhenUpdated(RexxMethodContext *c, CSTRING n)
RexxMethod1(RexxObjectPtr,
            TestSetGuardOffWhenUpdated,
            CSTRING, n)
{
    return context->SetGuardOffWhenUpdated(n);
}




RexxMethodEntry orxtest_methods[] = {
    REXX_METHOD(TestIsBuffer,          TestIsBuffer),
    REXX_METHOD(TestBufferInit,        TestBufferInit),
    REXX_METHOD(TestBufferCSelf,       TestBufferCSelf),
    REXX_METHOD(TestBufferObjectToCSelf,  TestBufferObjectToCSelf),
    REXX_METHOD(TestGetBuffer,         TestGetBuffer),
    REXX_METHOD(TestZeroIntArgs,       TestZeroIntArgs),
    REXX_METHOD(TestOneIntArg,         TestOneIntArg),
    REXX_METHOD(TestTwoIntArgs,        TestTwoIntArgs),
    REXX_METHOD(TestThreeIntArgs,      TestThreeIntArgs),
    REXX_METHOD(TestFourIntArgs,       TestFourIntArgs),
    REXX_METHOD(TestFiveIntArgs,       TestFiveIntArgs),
    REXX_METHOD(TestSixIntArgs,        TestSixIntArgs),
    REXX_METHOD(TestSevenIntArgs,      TestSevenIntArgs),
    REXX_METHOD(TestEightIntArgs,      TestEightIntArgs),
    REXX_METHOD(TestNineIntArgs,       TestNineIntArgs),
    REXX_METHOD(TestTenIntArgs,        TestTenIntArgs),
    REXX_METHOD(TestIntArg,            TestIntArg),
    REXX_METHOD(TestInt32Arg,          TestInt32Arg),
    REXX_METHOD(TestUint32Arg,         TestUint32Arg),
    REXX_METHOD(TestInt8Arg,           TestInt8Arg),
    REXX_METHOD(TestUint8Arg,          TestUint8Arg),
    REXX_METHOD(TestInt16Arg,          TestInt16Arg),
    REXX_METHOD(TestUint16Arg,         TestUint16Arg),
    REXX_METHOD(TestInt64Arg,          TestInt64Arg),
    REXX_METHOD(TestUint64Arg,         TestUint64Arg),
    REXX_METHOD(TestIntPtrArg,         TestIntPtrArg),
    REXX_METHOD(TestUintPtrArg,        TestUintPtrArg),
    REXX_METHOD(TestWholeNumberArg,    TestWholeNumberArg),
    REXX_METHOD(TestStringSizeArg,     TestStringSizeArg),
    REXX_METHOD(TestPositiveWholeNumberArg,     TestPositiveWholeNumberArg),
    REXX_METHOD(TestNonnegativeWholeNumberArg,     TestNonnegativeWholeNumberArg),
    REXX_METHOD(TestSizeArg,           TestSizeArg),
    REXX_METHOD(TestSSizeArg,          TestSSizeArg),
    REXX_METHOD(TestLogicalArg,        TestLogicalArg),
    REXX_METHOD(TestFloatArg,          TestFloatArg),
    REXX_METHOD(TestDoubleArg,         TestDoubleArg),
    REXX_METHOD(TestCstringArg,        TestCstringArg),
    REXX_METHOD(TestPointerValue,      TestPointerValue),
    REXX_METHOD(TestPointerArg,        TestPointerArg),
    REXX_METHOD(TestNullPointerValue,  TestNullPointerValue),
    REXX_METHOD(TestPointerStringValue,      TestPointerStringValue),
    REXX_METHOD(TestPointerStringArg,        TestPointerStringArg),
    REXX_METHOD(TestNullPointerStringValue,  TestNullPointerStringValue),
    REXX_METHOD(TestStemArg,                 TestStemArg),
    REXX_METHOD(TestObjectArg,               TestObjectArg),
    REXX_METHOD(TestStringArg,               TestStringArg),
    REXX_METHOD(TestArrayArg,                TestArrayArg),
    REXX_METHOD(TestClassArg,                TestClassArg),
    REXX_METHOD(TestObjectToWholeNumber,     TestObjectToWholeNumber),
    REXX_METHOD(TestObjectToWholeNumberAlt,  TestObjectToWholeNumberAlt),
    REXX_METHOD(TestObjectToStringSize,      TestObjectToStringSize),
    REXX_METHOD(TestObjectToStringSizeAlt,   TestObjectToStringSizeAlt),
    REXX_METHOD(TestObjectToInt64,           TestObjectToInt64),
    REXX_METHOD(TestObjectToInt64Alt,        TestObjectToInt64Alt),
    REXX_METHOD(TestObjectToUnsignedInt64,   TestObjectToUnsignedInt64),
    REXX_METHOD(TestObjectToUnsignedInt64Alt, TestObjectToUnsignedInt64Alt),
    REXX_METHOD(TestObjectToInt32,           TestObjectToInt32),
    REXX_METHOD(TestObjectToInt32Alt,        TestObjectToInt32Alt),
    REXX_METHOD(TestObjectToUnsignedInt32,   TestObjectToUnsignedInt32),
    REXX_METHOD(TestObjectToUnsignedInt32Alt, TestObjectToUnsignedInt32Alt),
    REXX_METHOD(TestObjectToIntptr,          TestObjectToIntptr),
    REXX_METHOD(TestObjectToIntptrAlt,       TestObjectToIntptrAlt),
    REXX_METHOD(TestObjectToUintptr,         TestObjectToUintptr),
    REXX_METHOD(TestObjectToUintptrAlt,      TestObjectToUintptrAlt),
    REXX_METHOD(TestObjectToLogical,         TestObjectToLogical),
    REXX_METHOD(TestObjectToLogicalAlt,      TestObjectToLogicalAlt),
    REXX_METHOD(TestObjectToDouble,          TestObjectToDouble),
    REXX_METHOD(TestObjectToDoubleAlt,       TestObjectToDoubleAlt),
    REXX_METHOD(TestWholeNumberToObject,     TestWholeNumberToObject),
    REXX_METHOD(TestWholeNumberToObjectAlt,  TestWholeNumberToObjectAlt),
    REXX_METHOD(TestStringSizeToObject,      TestStringSizeToObject),
    REXX_METHOD(TestStringSizeToObjectAlt,   TestStringSizeToObjectAlt),
    REXX_METHOD(TestInt64ToObject,           TestInt64ToObject),
    REXX_METHOD(TestInt64ToObjectAlt,        TestInt64ToObjectAlt),
    REXX_METHOD(TestUnsignedInt64ToObject,   TestUnsignedInt64ToObject),
    REXX_METHOD(TestUnsignedInt64ToObjectAlt, TestUnsignedInt64ToObjectAlt),
    REXX_METHOD(TestInt32ToObject,           TestInt32ToObject),
    REXX_METHOD(TestInt32ToObjectAlt,        TestInt32ToObjectAlt),
    REXX_METHOD(TestUnsignedInt32ToObject,   TestUnsignedInt32ToObject),
    REXX_METHOD(TestUnsignedInt32ToObjectAlt, TestUnsignedInt32ToObjectAlt),
    REXX_METHOD(TestIntptrToObject,          TestIntptrToObject),
    REXX_METHOD(TestIntptrToObjectAlt,       TestIntptrToObjectAlt),
    REXX_METHOD(TestUintptrToObject,         TestUintptrToObject),
    REXX_METHOD(TestUintptrToObjectAlt,      TestUintptrToObjectAlt),
    REXX_METHOD(TestLogicalToObject,         TestLogicalToObject),
    REXX_METHOD(TestLogicalToObjectAlt,      TestLogicalToObjectAlt),
    REXX_METHOD(TestDoubleToObject,          TestDoubleToObject),
    REXX_METHOD(TestDoubleToObjectWithPrecision, TestDoubleToObjectWithPrecision),
    REXX_METHOD(TestDoubleToObjectAlt,       TestDoubleToObjectAlt),
    REXX_METHOD(TestObjectToValue,           TestObjectToValue),
    REXX_METHOD(TestOptionalIntArg,            TestOptionalIntArg),
    REXX_METHOD(TestOptionalInt32Arg,          TestOptionalInt32Arg),
    REXX_METHOD(TestOptionalUint32Arg,         TestOptionalUint32Arg),
    REXX_METHOD(TestOptionalInt8Arg,           TestOptionalInt8Arg),
    REXX_METHOD(TestOptionalUint8Arg,          TestOptionalUint8Arg),
    REXX_METHOD(TestOptionalInt16Arg,          TestOptionalInt16Arg),
    REXX_METHOD(TestOptionalUint16Arg,         TestOptionalUint16Arg),
    REXX_METHOD(TestOptionalInt64Arg,          TestOptionalInt64Arg),
    REXX_METHOD(TestOptionalUint64Arg,         TestOptionalUint64Arg),
    REXX_METHOD(TestOptionalIntPtrArg,         TestOptionalIntPtrArg),
    REXX_METHOD(TestOptionalUintPtrArg,        TestOptionalUintPtrArg),
    REXX_METHOD(TestOptionalWholeNumberArg,    TestOptionalWholeNumberArg),
    REXX_METHOD(TestOptionalPositiveWholeNumberArg,    TestOptionalPositiveWholeNumberArg),
    REXX_METHOD(TestOptionalNonnegativeWholeNumberArg,    TestOptionalNonnegativeWholeNumberArg),
    REXX_METHOD(TestOptionalStringSizeArg,     TestOptionalStringSizeArg),
    REXX_METHOD(TestOptionalSizeArg,           TestOptionalSizeArg),
    REXX_METHOD(TestOptionalSSizeArg,          TestOptionalSSizeArg),
    REXX_METHOD(TestOptionalLogicalArg,        TestOptionalLogicalArg),
    REXX_METHOD(TestOptionalFloatArg,          TestOptionalFloatArg),
    REXX_METHOD(TestOptionalDoubleArg,         TestOptionalDoubleArg),
    REXX_METHOD(TestOptionalCstringArg,        TestOptionalCstringArg),
    REXX_METHOD(TestOptionalPointerArg,        TestOptionalPointerArg),
    REXX_METHOD(TestOptionalPointerStringArg,        TestOptionalPointerStringArg),
    REXX_METHOD(TestOptionalStemArg,                 TestOptionalStemArg),
    REXX_METHOD(TestOptionalObjectArg,               TestOptionalObjectArg),
    REXX_METHOD(TestOptionalStringArg,               TestOptionalStringArg),
    REXX_METHOD(TestOptionalArrayArg,                TestOptionalArrayArg),
    REXX_METHOD(TestOptionalClassArg,                TestOptionalClassArg),
    REXX_METHOD(TestSuperArg,                TestSuperArg),
    REXX_METHOD(TestScopeArg,                TestScopeArg),
    REXX_METHOD(TestNameArg,                 TestNameArg),
    REXX_METHOD(TestArglistArg,              TestArglistArg),
    REXX_METHOD(TestOSelfArg,                TestOSelfArg),
    REXX_METHOD(TestGetArguments,            TestGetArguments),
    REXX_METHOD(TestGetArgument,             TestGetArgument),
    REXX_METHOD(TestGetMessageName,          TestGetMessageName),
    REXX_METHOD(TestGetMessageName,          TestGetMessageName),
    REXX_METHOD(TestGetMethod,               TestGetMethod),
    REXX_METHOD(TestGetSelf,                 TestGetSelf),
    REXX_METHOD(TestGetSuper,                TestGetSuper),
    REXX_METHOD(TestGetScope,                TestGetScope),
    REXX_METHOD(TestSetObjectVariable,       TestSetObjectVariable),
    REXX_METHOD(TestGetObjectVariable,       TestGetObjectVariable),
    REXX_METHOD(TestGetObjectVariableOrNil,  TestGetObjectVariableOrNil),
    REXX_METHOD(TestDropObjectVariable,      TestDropObjectVariable),
    REXX_METHOD(TestForwardMessage,          TestForwardMessage),
    REXX_METHOD(TestFindContextClass,        TestFindContextClass),
    REXX_METHOD(TestNewStem,                 TestNewStem),
    REXX_METHOD(TestSetStemElement,          TestSetStemElement),
    REXX_METHOD(TestGetStemElement,          TestGetStemElement),
    REXX_METHOD(TestDropStemElement,         TestDropStemElement),
    REXX_METHOD(TestSetStemArrayElement,     TestSetStemArrayElement),
    REXX_METHOD(TestGetStemArrayElement,     TestGetStemArrayElement),
    REXX_METHOD(TestDropStemArrayElement,    TestDropStemArrayElement),
    REXX_METHOD(TestGetAllStemElements,      TestGetAllStemElements),
    REXX_METHOD(TestGetStemValue,            TestGetStemValue),
    REXX_METHOD(TestIsStem,                  TestIsStem),
    REXX_METHOD(TestSupplierItem,            TestSupplierItem),
    REXX_METHOD(TestSupplierIndex,           TestSupplierIndex),
    REXX_METHOD(TestSupplierAvailable,       TestSupplierAvailable),
    REXX_METHOD(TestSupplierNext,            TestSupplierNext),
    REXX_METHOD(TestNewSupplier,             TestNewSupplier),
    REXX_METHOD(TestArrayAt,                 TestArrayAt),
    REXX_METHOD(TestArrayPut,                TestArrayPut),
    REXX_METHOD(TestArrayAppend,             TestArrayAppend),
    REXX_METHOD(TestArrayAppendString,       TestArrayAppendString),
    REXX_METHOD(TestArraySize,               TestArraySize),
    REXX_METHOD(TestArrayItems,              TestArrayItems),
    REXX_METHOD(TestArrayDimension,          TestArrayDimension),
    REXX_METHOD(TestNewArray,                TestNewArray),
    REXX_METHOD(TestArrayOfOne,              TestArrayOfOne),
    REXX_METHOD(TestArrayOfTwo,              TestArrayOfTwo),
    REXX_METHOD(TestArrayOfThree,            TestArrayOfThree),
    REXX_METHOD(TestArrayOfFour,             TestArrayOfFour),
    REXX_METHOD(TestArrayOfOneAlt,           TestArrayOfOneAlt),
    REXX_METHOD(TestArrayOfTwoAlt,           TestArrayOfTwoAlt),
    REXX_METHOD(TestArrayOfThreeAlt,         TestArrayOfThreeAlt),
    REXX_METHOD(TestArrayOfFourAlt,          TestArrayOfFourAlt),
    REXX_METHOD(TestIsArray,                 TestIsArray),
    REXX_METHOD(TestDirectoryPut,            TestDirectoryPut),
    REXX_METHOD(TestDirectoryAt,             TestDirectoryAt),
    REXX_METHOD(TestDirectoryRemove,         TestDirectoryRemove),
    REXX_METHOD(TestNewDirectory,            TestNewDirectory),
    REXX_METHOD(TestIsDirectory,             TestIsDirectory),
    REXX_METHOD(TestStringTablePut,          TestStringTablePut),
    REXX_METHOD(TestStringTableAt,           TestStringTableAt),
    REXX_METHOD(TestStringTableRemove,       TestStringTableRemove),
    REXX_METHOD(TestNewStringTable,          TestNewStringTable),
    REXX_METHOD(TestIsStringTable,           TestIsStringTable),
    REXX_METHOD(TestSendMessage,             TestSendMessage),
    REXX_METHOD(TestSendMessageScoped,       TestSendMessageScoped),
    REXX_METHOD(TestSendMessage0,            TestSendMessage0),
    REXX_METHOD(TestSendMessage1,            TestSendMessage1),
    REXX_METHOD(TestSendMessage2,            TestSendMessage2),
    REXX_METHOD(TestIsInstanceOf,            TestIsInstanceOf),
    REXX_METHOD(TestHasMethod,               TestHasMethod),
    REXX_METHOD(TestLoadPackage,             TestLoadPackage),
    REXX_METHOD(TestLoadPackageFromData,     TestLoadPackageFromData),
    REXX_METHOD(TestFindClass,               TestFindClass),
    REXX_METHOD(TestFindPackageClass,        TestFindPackageClass),
    REXX_METHOD(TestGetPackageRoutines,      TestGetPackageRoutines),
    REXX_METHOD(TestGetPackagePublicRoutines,TestGetPackagePublicRoutines),
    REXX_METHOD(TestGetPackageClasses,       TestGetPackageClasses),
    REXX_METHOD(TestGetPackagePublicClasses, TestGetPackagePublicClasses),
    REXX_METHOD(TestGetPackageMethods,       TestGetPackageMethods),
    REXX_METHOD(TestCallRoutine,             TestCallRoutine),
    REXX_METHOD(TestCallProgram,             TestCallProgram),
    REXX_METHOD(TestNewMethod,               TestNewMethod),
    REXX_METHOD(TestNewRoutine,              TestNewRoutine),
    REXX_METHOD(TestIsMethod,                TestIsMethod),
    REXX_METHOD(TestIsRoutine,               TestIsRoutine),
    REXX_METHOD(TestGetMethodPackage,        TestGetMethodPackage),
    REXX_METHOD(TestGetRoutinePackage,       TestGetRoutinePackage),
    REXX_METHOD(TestObjectToString,          TestObjectToString),
    REXX_METHOD(TestStringGet,               TestStringGet),
    REXX_METHOD(TestStringLength,            TestStringLength),
    REXX_METHOD(TestStringData,              TestStringData),
    REXX_METHOD(TestStringUpper,             TestStringUpper),
    REXX_METHOD(TestStringLower,             TestStringLower),
    REXX_METHOD(TestIsString,                TestIsString),
    REXX_METHOD(TestRaiseException0,         TestRaiseException0),
    REXX_METHOD(TestRaiseException1,         TestRaiseException1),
    REXX_METHOD(TestRaiseException2,         TestRaiseException2),
    REXX_METHOD(TestRaiseException,          TestRaiseException),
    REXX_METHOD(TestRaiseCondition,          TestRaiseCondition),
    REXX_METHOD(TestInterpreterVersion,      TestInterpreterVersion),
    REXX_METHOD(TestLanguageLevel,           TestLanguageLevel),
    REXX_METHOD(TestNewStringFromAsciiz,     TestNewStringFromAsciiz),
    REXX_METHOD(TestNewStringFromAsciizAlt,  TestNewStringFromAsciizAlt),
    REXX_METHOD(TestNewString,               TestNewString),
    REXX_METHOD(TestNewStringAlt,            TestNewStringAlt),
    REXX_METHOD(TestCStringToObject,         TestCStringToObject),
    REXX_METHOD(TestObjectToCString,         TestObjectToCString),
    REXX_METHOD(TestNewMutableBuffer,        TestNewMutableBuffer),
    REXX_METHOD(TestIsMutableBuffer,         TestIsMutableBuffer),
    REXX_METHOD(TestMutableBufferLength,     TestMutableBufferLength),
    REXX_METHOD(TestSetMutableBufferLength,  TestSetMutableBufferLength),
    REXX_METHOD(TestMutableBufferCapacity,   TestMutableBufferCapacity),
    REXX_METHOD(TestSetMutableBufferCapacity,TestSetMutableBufferCapacity),
    REXX_METHOD(TestSetMutableBufferValue,   TestSetMutableBufferValue),
    REXX_METHOD(TestGetMutableBufferValue,   TestGetMutableBufferValue),
    REXX_METHOD(TestAllocateObjectMemory,    TestAllocateObjectMemory),
    REXX_METHOD(TestFreeObjectMemory,        TestFreeObjectMemory),
    REXX_METHOD(TestReallocateObjectMemory,  TestReallocateObjectMemory),
    REXX_METHOD(TestSetObjectMemory,         TestSetObjectMemory),
    REXX_METHOD(TestGetObjectMemory,         TestGetObjectMemory),
    REXX_METHOD(TestGetObjectVariableReference, TestGetObjectVariableReference),
    REXX_METHOD(TestIsVariableReference,     TestIsVariableReference),
    REXX_METHOD(TestSetVariableReferenceValue, TestSetVariableReferenceValue),
    REXX_METHOD(TestVariableReferenceName,   TestVariableReferenceName),
    REXX_METHOD(TestVariableReferenceValue,  TestVariableReferenceValue),
    REXX_METHOD(TestThrowException0,         TestThrowException0),
    REXX_METHOD(TestThrowException1,         TestThrowException1),
    REXX_METHOD(TestThrowException2,         TestThrowException2),
    REXX_METHOD(TestThrowException,          TestThrowException),
    REXX_METHOD(TestThrowCondition,          TestThrowCondition),
    REXX_METHOD(TestSetGuard,                TestSetGuard),
    REXX_METHOD(TestSetGuardOn,              TestSetGuardOn),
    REXX_METHOD(TestSetGuardOff,             TestSetGuardOff),
    REXX_METHOD(TestSetGuardOnWhenUpdated,   TestSetGuardOnWhenUpdated),
    REXX_METHOD(TestSetGuardOffWhenUpdated,  TestSetGuardOffWhenUpdated),
    REXX_LAST_METHOD()
};


RexxPackageEntry UnitTest_package_entry = {
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "UnitTest",                          // name of the package
    "1.0.0",                             // package information
    NULL,                                // no load/unload functions
    NULL,
    NULL,                                // the exported routines
    orxtest_methods                      // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(UnitTest);

