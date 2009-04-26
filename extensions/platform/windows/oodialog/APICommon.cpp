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
 * This modules contains generic convenience functions that might be useful in
 * any code that uses the native API.  Include APICommon.h to use them.
 */

#include "oovutil.h"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <errno.h>
#include "APICommon.h"

void systemServiceException(RexxMethodContext *context, char *msg)
{
    context->RaiseException1(Rexx_Error_System_service_user_defined, context->String(msg));
}

void systemServiceException(RexxMethodContext *context, char *msg, const char *sub)
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

void systemServiceExceptionCode(RexxMethodContext *context, const char *msg, const char *arg1)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), msg, arg1, GetLastError());
    systemServiceException(context, buffer);
}

void systemServiceExceptionComCode(RexxMethodContext *context, const char *msg, const char *arg1, HRESULT hr)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), msg, arg1, hr);
    systemServiceException(context, buffer);
}

void outOfMemoryException(RexxMethodContext *c)
{
    systemServiceException(c, NO_MEMORY_MSG);
}

void userDefinedMsgException(RexxMethodContext *c, CSTRING msg)
{
    c->RaiseException1(Rexx_Error_Incorrect_method_user_defined, c->String(msg));
}

void userDefinedMsgException(RexxMethodContext *c, int pos, CSTRING msg)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Method argument %d %s", pos, msg);
    userDefinedMsgException(c, buffer);
}

void *wrongClassException(RexxMethodContext *c, int pos, const char *n)
{
    c->RaiseException2(Rexx_Error_Incorrect_method_noclass, c->WholeNumber(pos), c->String(n));
    return NULL;
}

void invalidTypeException(RexxMethodContext *c, int pos, const char *type)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Method argument %d is not a valid %s", pos, type);
    userDefinedMsgException(c, buffer);
}

void invalidImageException(RexxMethodContext *c, int pos, CSTRING type, CSTRING actual)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Method argument %d must be a %s image; found %s", pos, type, actual);
    userDefinedMsgException(c, buffer);
}

void wrongObjInArrayException(RexxMethodContext *c, int argPos, size_t index, CSTRING obj)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Method argument %d is an array and index %d is not a %s", argPos, index, obj);
    userDefinedMsgException(c, buffer);
}

void wrongObjInDirectoryException(RexxMethodContext *c, int argPos, CSTRING index, CSTRING needed, RexxObjectPtr actual)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer),
              "Index, %s, of method argument %d must be %s; found \"%s\"",
              index, argPos, needed, c->ObjectToStringValue(actual));
    userDefinedMsgException(c, buffer);
}

void missingIndexInDirectoryException(RexxMethodContext *c, int argPos, CSTRING index)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer),
              "Index, %s, of method argument %d is required",
              index, argPos);
    userDefinedMsgException(c, buffer);
}

void emptyArrayException(RexxMethodContext *c, int argPos)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "Method argument %d must be a non-empty array", argPos);
    userDefinedMsgException(c, buffer);
}

void nullObjectException(RexxMethodContext *c, CSTRING name, int pos)
{
    TCHAR buffer[256];
    if ( pos == 0 )
    {
        _snprintf(buffer, sizeof(buffer), "The %s object must not be null", name);
    }
    else
    {
        _snprintf(buffer, sizeof(buffer), "Method argument %d, the %s object, must not be null", pos, name);
    }
    userDefinedMsgException(c, buffer);
}

void nullObjectException(RexxMethodContext *c, CSTRING name)
{
    nullObjectException(c, name, 0);
}

void nullPointerException(RexxMethodContext *c, int pos)
{
    c->RaiseException1(Rexx_Error_Incorrect_method_null, c->WholeNumber(pos));
}

void wrongRangeException(RexxMethodContext *c, int pos, int min, int max, RexxObjectPtr actual)
{
    c->RaiseException(Rexx_Error_Invalid_argument_range,
                      c->ArrayOfFour(c->WholeNumber(pos), c->WholeNumber(min), c->WholeNumber(max), actual));
}

void wrongRangeException(RexxMethodContext *c, int pos, int min, int max, int actual)
{
    wrongRangeException(c, pos, min, max, c->WholeNumber(actual));
}

void wrongArgValueException(RexxMethodContext *c, int pos, const char *list, RexxObjectPtr actual)
{
    c->RaiseException(Rexx_Error_Incorrect_method_list,
                      c->ArrayOfThree(c->WholeNumber(pos), c->String(list), actual));
}

void wrongArgValueException(RexxMethodContext *c, int pos, const char *list, const char *actual)
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

bool requiredClass(RexxMethodContext *c, RexxObjectPtr obj, const char *name, int pos)
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
        invalidTypeException(c, pos, "number");
        return false;
    }
    return true;

}

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
        result = c->Nil();
    }
    c->SetObjectVariable(varName, val);

    return result;
}

