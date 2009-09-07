/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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

/**
 * APICommon.cpp
 *
 * This module contains generic convenience functions that might be useful in
 * any code that uses the native API.  Include APICommon.hpp to use them.
 */

#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <errno.h>
#include "APICommon.hpp"


/**
 * 49.900
 * 49 -> A severe error was detected in the language processor or execution
 *       process during internal self-consistency checks.
 *
 * 900 -> User message
 *
 * @param c
 * @param msg
 */
void severeErrorException(RexxThreadContext *c, char *msg)
{
    c->RaiseException1(Rexx_Error_Interpretation_user_defined, c->String(msg));
}

void systemServiceException(RexxThreadContext *c, char *msg)
{
    c->RaiseException1(Rexx_Error_System_service_user_defined, c->String(msg));
}

void systemServiceException(RexxThreadContext *context, char *msg, const char *sub)
{
    if ( sub != NULL )
    {
        TCHAR buffer[128];
        _snprintf(buffer, sizeof(buffer), msg, sub);
        systemServiceException(context, buffer);
    }
    else
    {
        systemServiceException(context, msg);
    }
}

void systemServiceExceptionCode(RexxThreadContext *context, const char *msg, const char *arg1, DWORD rc)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), msg, arg1, rc);
    systemServiceException(context, buffer);
}

void systemServiceExceptionCode(RexxThreadContext *context, const char *msg, const char *arg1)
{
    systemServiceExceptionCode(context, msg, arg1, GetLastError());
}

void systemServiceExceptionComCode(RexxThreadContext *context, const char *msg, const char *arg1, HRESULT hr)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), msg, arg1, hr);
    systemServiceException(context, buffer);
}

void outOfMemoryException(RexxThreadContext *c)
{
    systemServiceException(c, NO_MEMORY_MSG);
}

/**
 * Message
 *
 * Raises 88.900
 *
 * @param c    Thread context we are executing in.
 * @param msg  "Some message"
 */
void userDefinedMsgException(RexxThreadContext *c, CSTRING msg)
{
    c->RaiseException1(Rexx_Error_Invalid_argument_user_defined, c->String(msg));
}

/**
 * Argument 'argument' 'message'
 *
 * Argument 2 message
 *
 * Raises 88.900
 *
 * @param c    Thread context we are executing in.
 * @param msg  "Some message"
 */
void userDefinedMsgException(RexxThreadContext *c, int pos, CSTRING msg)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d %s", pos, msg);
    userDefinedMsgException(c, buffer);
}

/**
 *  Argument 'argument' must be of the 'class' class
 *
 *  Argument 4 must be of the ImageList class
 *
 *  Raises 88.914
 *
 * @param c    The thread context we are operating under.
 * @param pos  The 'argument' position.
 * @param n    The name of the class expected.
 *
 * @return Pointer to void, could be used in the return statement of a method
 *         to return NULLOBJECT after the exeception is raised.
 */
void *wrongClassException(RexxThreadContext *c, int pos, const char *n)
{
    c->RaiseException2(Rexx_Error_Invalid_argument_noclass, c->WholeNumber(pos), c->String(n));
    return NULL;
}

/**
 * Argument 'argument' is not a valid'msg'
 *
 * Argument 3 is not a valid menu handle
 *
 * Raises 88.900
 *
 * @param c    Thread context we are executing in.
 * @param pos  Argumet position
 * @param msg  "Some message"
 *
 * @note  There is no space after 'valid' the caller must provide it in msg if
 *        it is needed
 */
void invalidTypeException(RexxThreadContext *c, int pos, const char *type)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d is not a valid%s", pos, type);
    userDefinedMsgException(c, buffer);
}

void invalidImageException(RexxThreadContext *c, int pos, CSTRING type, CSTRING actual)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d must be a %s image; found %s", pos, type, actual);
    userDefinedMsgException(c, buffer);
}

void wrongObjInArrayException(RexxThreadContext *c, int argPos, size_t index, CSTRING obj)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d is an array and index %d is not a %s", argPos, index, obj);
    userDefinedMsgException(c, buffer);
}

void wrongObjInDirectoryException(RexxThreadContext *c, int argPos, CSTRING index, CSTRING needed, RexxObjectPtr actual)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer),
              "Index, %s, of argument %d, must be %s; found \"%s\"",
              index, argPos, needed, c->ObjectToStringValue(actual));
    userDefinedMsgException(c, buffer);
}

void missingIndexInDirectoryException(RexxThreadContext *c, int argPos, CSTRING index)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer),
              "Index, %s, of argument %d, is required",
              index, argPos);
    userDefinedMsgException(c, buffer);
}

void emptyArrayException(RexxThreadContext *c, int argPos)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d must be a non-empty array", argPos);
    userDefinedMsgException(c, buffer);
}

/**
 * Error 98.900
 *
 * 98 The language processor detected a specific error during execution. The
 * associated error gives the reason for the error.
 *
 * 900 User message.
 *
 * @param c
 * @param msg
 */
void executionErrorException(RexxThreadContext *c, CSTRING msg)
{
    c->RaiseException1(Rexx_Error_Execution_user_defined, c->CString(msg));
}

/**
 * Error 98.913
 *
 * 98 The language processor detected a specific error during execution. The
 * associated error gives the reason for the error.
 *
 * 913: Unable to convert object "a MyThings" to a single-dimensional array
 * value
 *
 * @param c    Method context we are executing in.
 * @param obj  The "MyThings" object.
 *
 * @Note  This would be the exception most often seen in a do n over c statement
 *        when c does not have a makeArray() method.
 */
void doOverException(RexxThreadContext *c, RexxObjectPtr obj)
{
    c->RaiseException1(Rexx_Error_Execution_noarray, obj);
}

/**
 * Error 98.900
 *
 * Produces a message:
 *
 * Could not retrieve the "value" information for "object"
 *
 * Could not retrive the window handle information for a BaseDialog object.
 *
 * similar to old 98.921
 *
 * @param c       Method context we are operating in.
 * @param item    What was to be retrieved
 * @param source  The object it was being retrieved from.
 */
void failedToRetrieveException(RexxThreadContext *c, CSTRING item, RexxObjectPtr source)
{
    TCHAR buf[128];

    RexxObjectPtr name = c->SendMessage0(source, "OBJECTNAME");
    _snprintf(buf, sizeof(buf), "Could not retrieve the %s information for %s",
              item, c->ObjectToStringValue(name));

    c->RaiseException1(Rexx_Error_Execution_user_defined, c->String(buf));
}

void nullObjectException(RexxThreadContext *c, CSTRING name, int pos)
{
    TCHAR buffer[256];
    if ( pos == 0 )
    {
        _snprintf(buffer, sizeof(buffer), "The %s object must not be null", name);
    }
    else
    {
        _snprintf(buffer, sizeof(buffer), "Argument %d, the %s object, must not be null", pos, name);
    }
    userDefinedMsgException(c, buffer);
}

void nullObjectException(RexxThreadContext *c, CSTRING name)
{
    nullObjectException(c, name, 0);
}

void nullPointerException(RexxThreadContext *c, int pos)
{
    c->RaiseException1(Rexx_Error_Invalid_argument_null, c->WholeNumber(pos));
}

void notNonNegativeException(RexxThreadContext *c, int pos, RexxObjectPtr actual)
{
    c->RaiseException2(Rexx_Error_Invalid_argument_nonnegative, c->Int32(pos), actual);
}

void wrongRangeException(RexxThreadContext *c, int pos, int min, int max, RexxObjectPtr actual)
{
    c->RaiseException(Rexx_Error_Invalid_argument_range,
                      c->ArrayOfFour(c->WholeNumber(pos), c->WholeNumber(min), c->WholeNumber(max), actual));
}

void wrongRangeException(RexxThreadContext *c, int pos, int min, int max, int actual)
{
    wrongRangeException(c, pos, min, max, c->WholeNumber(actual));
}

void wrongArgValueException(RexxThreadContext *c, int pos, const char *list, RexxObjectPtr actual)
{
    c->RaiseException(Rexx_Error_Invalid_argument_list,
                      c->ArrayOfThree(c->WholeNumber(pos), c->String(list), actual));
}

void wrongArgValueException(RexxThreadContext *c, int pos, const char *list, const char *actual)
{
    wrongArgValueException(c, pos, list, c->String(actual));
}

CSTRING rxGetStringAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name)
{
    CSTRING value = NULL;
    RexxObjectPtr rxString = context->SendMessage0(obj, name);
    if ( rxString != NULLOBJECT )
    {
        value = context->ObjectToStringValue(rxString);
    }
    return value;
}

bool rxGetNumberAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name, wholenumber_t *pNumber)
{
    bool result = false;
    RexxObjectPtr rxNumber = context->SendMessage0(obj, name);
    if ( rxNumber != NULLOBJECT )
    {
        wholenumber_t number;
        if ( context->WholeNumber(rxNumber, &number) )
        {
            *pNumber = number;
            result = true;
        }
    }
    return result;
}

bool rxGetUIntPtrAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name, uintptr_t *pNumber)
{
    bool result = false;
    RexxObjectPtr rxNumber = context->SendMessage0(obj, name);
    if ( rxNumber != NULLOBJECT )
    {
        uintptr_t number;
        if ( context->ObjectToUintptr(rxNumber, &number) )
        {
            *pNumber = number;
            result = true;
        }
    }
    return result;
}

bool rxGetUInt32Attribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name, uint32_t *pNumber)
{
    bool result = false;
    RexxObjectPtr rxNumber = context->SendMessage0(obj, name);
    if ( rxNumber != NULLOBJECT )
    {
        uint32_t number;
        if ( context->ObjectToUnsignedInt32(rxNumber, &number) )
        {
            *pNumber = number;
            result = true;
        }
    }
    return result;
}

bool requiredClass(RexxThreadContext *c, RexxObjectPtr obj, const char *name, int pos)
{
    if ( obj == NULLOBJECT || ! c->IsOfType(obj, name) )
    {
        wrongClassException(c, pos, name);
        return false;
    }
    return true;
}

/**
 * Return the number of existing arguments in an ooRexx method invocation.  In
 * others words, it is intended to count neither the omitted args in the ooRexx
 * method, nor the pseudo-arguments to the native API function, like OSELF,
 * CSELF, etc..
 *
 * @param context  The method context pointer.
 *
 * @return The count of existing arguments in an ooRexx method invocation.
 */
size_t rxArgCount(RexxMethodContext * context)
{
    RexxObjectPtr items = context->SendMessage0(context->GetArguments(), "ITEMS");

    wholenumber_t count;
    context->ObjectToWholeNumber(items, &count);
    return (size_t)count;
}

bool rxStr2Number(RexxMethodContext *c, CSTRING str, uint64_t *number, int pos)
{
    char *end;
    *number = _strtoui64(str, &end, 0);
    if ( (end - str != strlen(str)) || errno == EINVAL || *number == _UI64_MAX )
    {
        invalidTypeException(c->threadContext, pos, " number");
        return false;
    }
    return true;
}

/**
 * Gets a Class object.
 *
 * This is for use for classes visible within the scope of the context, like say
 * .BaseDialog, or .Rect.  Use c->GetClass() to directly get classes from the
 * environment like .Bag or .Directory.
 *
 * @param c     The thread context we are operating in.
 * @param name  The name of the class to try and find.
 *
 * @return The class object or null on failure.
 *
 * @remarks  When null is returned an error has been raised: 98.909
 *
 *   98: The language processor detected a specific error during execution. The
 *   associated error gives the reason for the error.
 *
 *   909: Class "class" not found
 */
RexxClassObject rxGetContextClass(RexxMethodContext *c, CSTRING name)
{
    RexxClassObject theClass = c->FindContextClass(name);
    if ( theClass == NULL )
    {
        c->RaiseException1(Rexx_Error_Execution_noclass, c->String(name));
    }
    return theClass;
}

/**
 * Sets an object variable value and returns the existing value.  With the
 * caveat that if the object variable did not have a value set, .nil is
 * returned.
 *
 * @param c        The method context we are operating in.
 * @param varName  The object variable's name.
 * @param val      The value to set.
 *
 * @return The previous value of the object variable, if it was set, otherwise
 *         .nil.
 */
RexxObjectPtr rxSetObjVar(RexxMethodContext *c, CSTRING varName, RexxObjectPtr val)
{
    RexxObjectPtr result = c->GetObjectVariable(varName);
    if ( result == NULLOBJECT )
    {
        result = TheNilObj;
    }
    c->SetObjectVariable(varName, val);

    return result;
}


/**
 * Test if a generic Rexx object is exactly some int.
 *
 * @param c        The method context we are executing under.
 * @param testFor  The int value being tested for.
 * @param val      The generic Rexx object, which could be null.
 *
 * @return True if val is the int number we are testing for, otherwise false.
 */
bool isInt(RexxMethodContext *c, int testFor, RexxObjectPtr val)
{
    if ( val != NULLOBJECT )
    {
        int n;
        if ( c->ObjectToInt32(val, &n) )
        {
            return n == testFor;
        }
    }
    return false;
}


/**
 * Test if a genric Rexx object is the type of specified class object.
 *
 * @param c        The method context we are executing under.
 * @param obj      The object to test.
 * @param classID  The ID string of the class we are looking for.
 *
 * @return True if obj is a class object of the type specified, otherwise false.
 */
bool isOfClassType(RexxMethodContext *c, RexxObjectPtr obj, CSTRING classID)
{
    if ( obj != NULLOBJECT && c->IsOfType(obj, "CLASS") )
    {
        RexxStringObject clsID = (RexxStringObject)c->SendMessage0(obj, "ID");
        if ( clsID != NULLOBJECT && stricmp(c->StringData(clsID), classID) == 0 )
        {
            return true;
        }
    }
    return false;
}

/**
 * Print out the class ID of a Rexx object.  Useful in debugging to identify
 * exactly what a Rexx object is.  Will work with class objects or instance
 * objects.
 *
 * @param c    The method context we are operating in.
 * @param obj  The object to identify.
 */
void dbgPrintClassID(RexxMethodContext *c, RexxObjectPtr obj)
{
    if ( ! c->IsOfType(obj, "CLASS") )
    {
        obj = c->SendMessage0(obj, "CLASS");
    }

    CSTRING name = "<null>";
    if ( obj != NULLOBJECT )
    {
        RexxStringObject id = (RexxStringObject)c->SendMessage0(obj, "ID");
        if ( id != NULLOBJECT )
        {
            name = c->CString(id);
        }
    }
    printf("Class: %s\n", name);
}

