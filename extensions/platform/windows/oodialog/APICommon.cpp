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
RexxObjectPtr wrongClassException(RexxThreadContext *c, int pos, const char *n)
{
    c->RaiseException2(Rexx_Error_Invalid_argument_noclass, c->WholeNumber(pos), c->String(n));
    return NULLOBJECT;
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
RexxObjectPtr invalidTypeException(RexxThreadContext *c, size_t pos, const char *type)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d is not a valid%s", pos, type);
    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

void invalidImageException(RexxThreadContext *c, int pos, CSTRING type, CSTRING actual)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d must be a %s image; found %s", pos, type, actual);
    userDefinedMsgException(c, buffer);
}

/**
 * Argument <argPos> must be less than <len> characters in length; length is
 * <realLen>
 *
 * Argument 2 must be less than 255 characters in length; length is 260
 *
 * Raises 88.900
 *
 * @param c        Thread context we are executing in.
 * @param pos      Argumet position
 * @param len      Fixed length
 * @param realLen  Actual length
 */
void stringTooLongException(RexxThreadContext *c, int pos, size_t len, size_t realLen)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d must be less than %d characters in length; length is %d",
              pos, len, realLen);
    userDefinedMsgException(c, buffer);
}

/**
 * Argument <argPos> must be a whole number greater than <min>; found <actual>
 *
 * Argument 10 must be a whole number greater than 5; found 0
 *
 * Raises 88.900
 *
 * @param c        Thread context we are executing in.
 * @param pos      Argumet position
 * @param min      Minimum - 1
 * @param actual   Actual Rexx object
 */
void numberTooSmallException(RexxThreadContext *c, int pos, int min, RexxObjectPtr actual)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d must be a whole number greater than %d; actual %s",
              pos, min, c->ObjectToStringValue(actual));
    userDefinedMsgException(c, buffer);
}

/**
 * Argument <argPos> must be true or false; found "<actual>"
 *
 * Argument 5 must be true or fals; found "an Array"
 *
 * Similar to:
 *
 * 88.904
 * Argument argument must be zero or a positive whole number; found "value"
 *
 * Raises 88.900
 *
 * @param c        Thread context we are executing in.
 * @param pos      Argumet position
 * @param actual   Actual Rexx object
 */
RexxObjectPtr notBooleanException(RexxThreadContext *c, size_t pos, RexxObjectPtr actual)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d must be true or false; found \"%s\"",
              pos, c->ObjectToStringValue(actual));
    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

void wrongObjInArrayException(RexxThreadContext *c, size_t argPos, size_t index, CSTRING obj)
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

/**
 * Index, <index>, of argument <pos> must be one of <list>; found "<actual>"
 *
 * Index, PART, of argument 1 must be one of calendar, next, prev, or none;
 * found "today"
 *
 * @param c
 * @param pos
 * @param index
 * @param list
 * @param actual
 *
 * @return RexxObjectPtr
 */
void wrongValueAtDirectoryIndexException(RexxThreadContext *c, size_t pos, CSTRING index, CSTRING list, RexxObjectPtr actual)
{
    TCHAR buffer[512];
    _snprintf(buffer, sizeof(buffer),
              "Index, %s, of argument %d, must be one of %s; found \"%s\"",
              index, pos, list, c->ObjectToStringValue(actual));
    userDefinedMsgException(c, buffer);
}

/**
 * Index, <index>, of argument <pos> <msg>; found "<actual>"
 *
 * Index, PART, of argument 1 must contain one or more of the keywords calendar,
 * next, prev, or none; found "today"
 *
 * @param c
 * @param pos
 * @param index
 * @param list
 * @param actual
 *
 * @return RexxObjectPtr
 */
void directoryIndexException(RexxThreadContext *c, size_t pos, CSTRING index, CSTRING msg, RexxObjectPtr actual)
{
    TCHAR buffer[512];
    _snprintf(buffer, sizeof(buffer),
              "Index, %s, of argument %d, must be one of %s; found \"%s\"",
              index, pos, msg, c->ObjectToStringValue(actual));
    userDefinedMsgException(c, buffer);
}

void emptyArrayException(RexxThreadContext *c, int argPos)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d must be a non-empty array", argPos);
    userDefinedMsgException(c, buffer);
}

void sparseArrayException(RexxThreadContext *c, size_t argPos, size_t index)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Argument %d must be a non-sparse array, index %d is missing", argPos, index);
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

void notNonNegativeException(RexxThreadContext *c, size_t pos, RexxObjectPtr actual)
{
    c->RaiseException2(Rexx_Error_Invalid_argument_nonnegative, c->StringSize(pos), actual);
}

RexxObjectPtr wrongRangeException(RexxThreadContext *c, size_t pos, int min, int max, RexxObjectPtr actual)
{
    c->RaiseException(Rexx_Error_Invalid_argument_range,
                      c->ArrayOfFour(c->WholeNumber(pos), c->WholeNumber(min), c->WholeNumber(max), actual));
    return NULLOBJECT;
}

RexxObjectPtr wrongRangeException(RexxThreadContext *c, size_t pos, int min, int max, int actual)
{
    return wrongRangeException(c, pos, min, max, c->WholeNumber(actual));
}

RexxObjectPtr wrongArgValueException(RexxThreadContext *c, size_t pos, const char *list, RexxObjectPtr actual)
{
    c->RaiseException(Rexx_Error_Invalid_argument_list,
                      c->ArrayOfThree(c->WholeNumber(pos), c->String(list), actual));
    return NULLOBJECT;
}

RexxObjectPtr wrongArgValueException(RexxThreadContext *c, size_t pos, const char *list, const char *actual)
{
    return wrongArgValueException(c, pos, list, c->String(actual));
}

/**
 * Similar to 93.915 and 93.914  (actually a combination of the two.)
 *
 * Method argument <pos>, option must be one of <list>; found "<actual>"
 *
 * Method argument 2 must be one of [P]artially, or [E]ntirely; found "G"
 *
 * @param c
 * @param pos
 * @param list
 * @param actual
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr wrongArgOptionException(RexxThreadContext *c, size_t pos, CSTRING list, CSTRING actual)
{

    TCHAR buffer[512];
    _snprintf(buffer, sizeof(buffer), "Method argument %d, option must be one of %s; found \"%s\"", pos, list, actual);
    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

RexxObjectPtr wrongArgOptionException(RexxThreadContext *c, size_t pos, CSTRING list, RexxObjectPtr actual)
{
    return wrongArgOptionException(c, pos, list, c->ObjectToStringValue(actual));
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
 * Converts a Rexx object to a logical value, 0 or 1.  Returns -1 if the object
 * can not be converted.
 *
 * @param c    Thread context we are operating in.
 * @param obj  The object to convert.
 *
 * @return On success return 0 or 1 depending on what obj is.  Otherwise return
 *         -1 to signal failure.
 */
int32_t getLogical(RexxThreadContext *c, RexxObjectPtr obj)
{
    if ( obj != NULLOBJECT )
    {
        if ( obj == TheTrueObj )
        {
            return 1;
        }
        if ( obj == TheFalseObj )
        {
            return 0;
        }

        logical_t val;
        if ( c->Logical(obj, &val) )
        {
            return (val == 0 ? 0 : 1);
        }
    }
    return -1;
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

/**
 * Converts a string representing a number into an unsigned 64 bit number and
 * raises an exception if the conversion fails.
 *
 * The string must have a format of 123456789 or 0xFFAB. A leading 0 without the
 * following X will cause the string to be interpreted as an octal number, which
 * may or may not trigger a failure.  It is not the intent that this function be
 * used for octal numbers.
 *
 * Note that it is the use of 0 as the third argument that allows _strtoui64()
 * to interpret the string as decimal or hexadecimal based.
 *
 * @param c       Method context we are operating in.
 * @param str     String to convert.
 * @param number  [OUT]  Converted number is returned here.
 * @param pos     Argument position.  Used for exception.
 *
 * @return True if the number was converted, false if an exceptions is raised.
 *
 * @note  There is no way to tell the difference between a valid _UI64_MAX
 *        number and an error.  The function simply assumes a return of
 *        _UI64_MAX is an error signal.
 */
bool rxStr2Number(RexxMethodContext *c, CSTRING str, uint64_t *number, size_t pos)
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

/*
 * This function behaves exactly like rxStr2Number(), except it is for 32-bit
 * numbers, and ERANGE can differentiate between a valid ULONG_MAX and an error
 * return.
 */
bool rxStr2Number32(RexxMethodContext *c, CSTRING str, uint32_t *number, size_t pos)
{
    char *end;
    *number = strtoul(str, &end, 0);
    if ( (end - str != strlen(str)) || errno == ERANGE )
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
 * .BaseDialog, or .Rect.  Use c->FindClass() to directly get classes from the
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


RexxObjectPtr rxNewBuiltinObject(RexxThreadContext *c, CSTRING className)
{
    // This should never fail, provided the caller sends the right class name,
    // do we need an exception if it does?
    RexxClassObject classObj = c->FindClass(className);
    return c->SendMessage0(classObj, "NEW");
}

RexxObjectPtr rxNewBuiltinObject(RexxMethodContext *c, CSTRING className)
{
    return rxNewBuiltinObject(c->threadContext, className);
}


/**
 * Outputs the typical condition message.  For example:
 *
 *      4 *-* say dt~number
 * Error 97 running C:\work\qTest.rex line 4:  Object method not found
 * Error 97.1:  Object "a DateTime" does not understand message "NUMBER"
 *
 * @param c          The thread context we are operating in.
 * @param condObj    The condition information object.  The object returned from
 *                   the C++ API GetConditionInfo()
 * @param condition  The RexxCondition struct.  The filled in struct from the
 *                   C++ API DecodeConditionInfo().
 *
 * @assumes  There is a condition and that condObje and condition are valid.
 */
void standardConditionMsg(RexxThreadContext *c, RexxDirectoryObject condObj, RexxCondition *condition)
{
    RexxObjectPtr list = c->SendMessage0(condObj, "TRACEBACK");
    if ( list != NULLOBJECT )
    {
        RexxArrayObject a = (RexxArrayObject)c->SendMessage0(list, "ALLITEMS");
        if ( a != NULLOBJECT )
        {
            size_t count = c->ArrayItems(a);
            for ( size_t i = 1; i <= count; i++ )
            {
                RexxObjectPtr o = c->ArrayAt(a, i);
                if ( o != NULLOBJECT )
                {
                    printf("%s\n", c->ObjectToStringValue(o));
                }
            }
        }
    }
    printf("Error %d running %s line %d: %s\n", condition->rc, c->CString(condition->program),
           condition->position, c->CString(condition->errortext));

    printf("Error %d.%03d:  %s\n", condition->rc, conditionSubCode(condition), c->CString(condition->message));
}


/**
 * Given a thread context, checks for a raised condition, and prints out the
 * standard condition message if there is a condition.
 *
 * @param c            Thread context we are operating in.
 *
 * @return True if there was a condition, otherwsie false.
 *
 * @remarks.  This function could maybe take a second argument, true / false,
 *            whether to clear or not clear the condition.
 */
bool checkForCondition(RexxThreadContext *c)
{
    if ( c->CheckCondition() )
    {
        RexxCondition condition;
        RexxDirectoryObject condObj = c->GetConditionInfo();

        if ( condObj != NULLOBJECT )
        {
            c->DecodeConditionInfo(condObj, &condition);
            standardConditionMsg(c, condObj, &condition);
            return true;
        }
    }
    return false;
}


/**
 * Test if a generic Rexx object is exactly some int.
 *
 * @param testFor  The int value being tested for.
 * @param val      The generic Rexx object, which could be null.
 * @param c        The method context we are executing under.
 *
 * @return True if val is the int number we are testing for, otherwise false.
 */
bool isInt(int testFor, RexxObjectPtr val, RexxMethodContext *c)
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
void dbgPrintClassID(RexxThreadContext *c, RexxObjectPtr obj)
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


void dbgPrintClassID(RexxMethodContext *c, RexxObjectPtr obj)
{
    dbgPrintClassID(c->threadContext, obj);
}
