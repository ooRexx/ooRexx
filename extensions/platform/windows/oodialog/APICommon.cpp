/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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

/**
 * APICommon.cpp
 *
 * This module contains generic convenience functions that might be useful in
 * any code that uses the ooRexx native API.  Include APICommon.hpp to use them.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "oorexxapi.h"
#include "APICommon.hpp"

RexxObjectPtr       TheTrueObj        = NULLOBJECT;
RexxObjectPtr       TheFalseObj       = NULLOBJECT;
RexxObjectPtr       TheNilObj         = NULLOBJECT;
RexxObjectPtr       TheZeroObj        = NULLOBJECT;
RexxObjectPtr       TheOneObj         = NULLOBJECT;
RexxObjectPtr       TheTwoObj         = NULLOBJECT;
RexxObjectPtr       TheNegativeOneObj = NULLOBJECT;
RexxObjectPtr       TheZeroPointerObj = NULLOBJECT;
RexxDirectoryObject TheDotLocalObj    = NULLOBJECT;

bool RexxEntry packageLoadHelper(RexxThreadContext *c)
{
    TheTrueObj    = c->True();
    TheFalseObj   = c->False();
    TheNilObj     = c->Nil();
    TheZeroObj    = TheFalseObj;
    TheOneObj     = TheTrueObj;

    TheNegativeOneObj = c->WholeNumber(-1);
    c->RequestGlobalReference(TheNegativeOneObj);

    TheTwoObj = c->WholeNumber(2);
    c->RequestGlobalReference(TheTwoObj);

    TheZeroPointerObj = c->NewPointer(NULL);
    c->RequestGlobalReference(TheZeroPointerObj);

    RexxDirectoryObject local = c->GetLocalEnvironment();
    if ( local != NULLOBJECT )
    {
        TheDotLocalObj = local;
    }
    else
    {
        severeErrorException(c, NO_LOCAL_ENVIRONMENT_MSG);
        return false;
    }
    return true;
}


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
void severeErrorException(RexxThreadContext *c, const char *msg)
{
    c->RaiseException1(Rexx_Error_Interpretation_user_defined, c->String(msg));
}

void systemServiceException(RexxThreadContext *c, const char *msg)
{
    c->RaiseException1(Rexx_Error_System_service_user_defined, c->String(msg));
}

void systemServiceException(RexxThreadContext *context, const char *msg, const char *sub)
{
    if ( sub != NULL )
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), msg, sub);
        systemServiceException(context, buffer);
    }
    else
    {
        systemServiceException(context, msg);
    }
}

void systemServiceExceptionCode(RexxThreadContext *context, const char *msg, const char *arg1, uint32_t rc)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), msg, arg1, rc);
    systemServiceException(context, buffer);
}

void outOfMemoryException(RexxThreadContext *c)
{
    systemServiceException(c, NO_MEMORY_MSG);
}

/**
 * Error 98.900
 *
 * 98 The language processor detected a specific error during execution.
 *
 * 900 User message.
 *
 * The number of active dialogs has reached the maximum (20) allowed
 *
 * @param c
 * @param msg
 */
void *executionErrorException(RexxThreadContext *c, CSTRING msg)
{
    c->RaiseException1(Rexx_Error_Execution_user_defined, c->CString(msg));
    return NULL;
}

/**
 *  Error 98.900
 *
 *  98 The language processor detected a specific error during execution. The
 *  associated error gives the reason for the error.
 *
 *  900 User message.
 *
 *  The base class has not been initialized correctly
 *
 * @param c    The method context we are operating under.
 *
 * @return  Returns a null pointer.  This allows this type of code:
 *
 *            if ( pCSelf == NULL )
 *            {
 *                return baseClassInitializationException(c);
 *            }
 *
 * @remarks  This error is intended to be used when the CSelf pointer is null.
 *           It can only happen (I believe) when the user inovkes a method on
 *           self in init() before the super class init() has run.
 */
void *baseClassInitializationException(RexxThreadContext *c)
{
    return executionErrorException(c, "The base class has not been initialized correctly");
}
void *baseClassInitializationException(RexxMethodContext *c)
{
    return baseClassInitializationException(c->threadContext);
}

/**
 *  Error 98.900
 *
 *  98 The language processor detected a specific error during execution. The
 *  associated error gives the reason for the error.
 *
 *  900 User message.
 *
 *  The ooSQLiteDB base class has not been initialized correctly
 *
 * @param c         The method context we are operating under.
 * @param clsName   The name of the base class.
 *
 * @return  A null pointer to void
 *
 * @remarks  This error is intended to be used when a CSelf pointer, is null. It
 *           can only happen (I believe) when the user inovkes a method on self
 *           in init() before the super class init() has run.
 *
 *           Identifying the actual base class may make it easier for the user to
 *           understand what the problem is.
 */
void *baseClassInitializationException(RexxThreadContext *c, CSTRING clsName)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "The %s base class has not been initialized correctly", clsName);
    return executionErrorException(c, buffer);
}
void *baseClassInitializationException(RexxMethodContext *c, CSTRING clsName)
{
    return baseClassInitializationException(c->threadContext, clsName);
}

/**
 *  Error 98.900
 *
 *  98 The language processor detected a specific error during execution. The
 *  associated error gives the reason for the error.
 *
 *  900 User message.
 *
 *  The PropertySheetDialog base class has not been initialized correctly; the
 *  defineSizing method failed
 *
 * @param c         The method context we are operating under.
 * @param clsName   The name of the base class.
 * @param msg       Some *short* message to follow the ';'
 *
 * @return  A null pointer to void
 *
 * @remarks  This error is intended to be used when a some type of fatal error
 *           happens during the native API processing of the init methods for a
 *           class.  It should end the dialog.  At this point there is no dialog
 *           window handle to do a endDialogPremature().
 */
void *baseClassInitializationException(RexxThreadContext *c, CSTRING clsName, CSTRING msg)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "The %s base class has not been initialized correctly; %s", clsName, msg);
    return executionErrorException(c, buffer);
}
void *baseClassInitializationException(RexxMethodContext *c, CSTRING clsName, CSTRING msg)
{
    return baseClassInitializationException(c->threadContext, clsName, msg);
}

/**
 * Message
 *
 * Argument 1, the database connection object, can not be null
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
 * Some kind of %d message
 *
 * The number of property sheet dialogs being concurrently created has reached
 * the maximum (5) allowed.
 *
 * Raises 88.900
 *
 * @param *c          Thread context we are executing in.
 * @param formatStr   Format string with 1 %d contained in it.
 * @param number      Replacement arg.
 */
void userDefinedMsgException(RexxThreadContext *c, CSTRING formatStr, int number)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), formatStr, number);
    userDefinedMsgException(c, buffer);
}

/**
 * Argument 'argument' 'message'
 *
 * Argument 2 must be a whole number greater than 100; actual 100.5
 *
 * Raises 88.900
 *
 * @param *c    Thread context we are executing in.
 * @param pos   Argument position.
 * @param msg   "Some message"
 */
void userDefinedMsgException(RexxThreadContext *c, int pos, CSTRING msg)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %d %s", pos, msg);
    userDefinedMsgException(c, buffer);
}

/**
 * Message
 *
 * The 'methodName' argument must be less than 256 characters
 *
 * Raises 93.900
 *
 * @param c    Method context we are executing in.
 * @param msg  "Some message"
 */
void userDefinedMsgException(RexxMethodContext *c, CSTRING msg)
{
    c->RaiseException1(Rexx_Error_Incorrect_method_user_defined, c->String(msg));
}

/**
 * Method argument 'argument' 'message'
 *
 * Method argument 2 is not a handle
 *
 * Raises 93.900
 *
 * @param c    Method context we are executing in.
 * @param msg  "Some message"
 */
void userDefinedMsgException(RexxMethodContext *c, size_t pos, CSTRING msg)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Method argument %zd %s", pos, msg);
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
RexxObjectPtr wrongClassException(RexxThreadContext *c, size_t pos, const char *n)
{
    c->RaiseException2(Rexx_Error_Invalid_argument_noclass, c->WholeNumber(pos), c->String(n));
    return NULLOBJECT;
}

/**
 *  Argument 'argument' must be of the 'class' class; found 'actual'
 *
 *  Argument 4 must be of the Directory class; found a Stem
 *
 *  Similar to 88.914
 *  Raises 88.900
 *
 * @param c       The thread context we are operating under.
 * @param pos     The 'argument' position.
 * @param n       The name of the class expected.
 * @param actual  Some Rexx object.
 *
 * @return Pointer to void, could be used in the return statement of a method
 *         to return NULLOBJECT after the exeception is raised.
 *
 * @remarks  If _actual is a stem object without an assigned name, then
 *           ObjectToStringValue() will return the empty string, which is
 *           confusing in the error message.  Hence the work around.  What would
 *           be better is to use the real class name for _actual, but currently
 *           I get Stem, rather than 'a Stem' or Array rather than 'an Array'.
 *           Need to figure out how to get an Array.
 */
RexxObjectPtr wrongClassException(RexxThreadContext *c, size_t pos, const char *n, RexxObjectPtr _actual)
{
    char    buffer[256];

    CSTRING actual = c->ObjectToStringValue(_actual);
    if ( strlen(actual) == 0 )
    {
        actual = strPrintClassID(c, _actual);
    }

    snprintf(buffer, sizeof(buffer), "Argument %zd must be of the %s class; found %s",
             pos, n, actual);

    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

/**
 *  Argument 'argument' must be of the 'list' class; found 'actual'
 *
 *  Argument 1 must be of the ToolInfo, PlainBaseDialog, or DialogControl class;
 *  found a Stem
 *
 *  Similar to 88.914
 *  Raises 88.900
 *
 * @param c       The thread context we are operating under.
 * @param pos     The 'argument' position.
 * @param n       The list of the classes expected.
 * @param actual  Some Rexx object.
 *
 * @return Pointer to void, could be used in the return statement of a method
 *         to return NULLOBJECT after the exeception is raised.
 *
 * @remarks  If _actual is a stem object without an assigned name, then
 *           ObjectToStringValue() will return the empty string, which is
 *           confusing in the error message.  Hence the work around.  What would
 *           be better is to use the real class name for _actual, but currently
 *           I get Stem, rather than 'a Stem' or Array rather than 'an Array'.
 *           Need to figure out how to get an Array.
 */
RexxObjectPtr wrongClassListException(RexxThreadContext *c, size_t pos, const char *n, RexxObjectPtr _actual)
{
    char    buffer[256];

    CSTRING actual = c->ObjectToStringValue(_actual);
    if ( strlen(actual) == 0 )
    {
        actual = strPrintClassID(c, _actual);
    }

    snprintf(buffer, sizeof(buffer), "Argument %zd must be of the %s class; found %s",
             pos, n, actual);

    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

/**
 * Argument 'argument' is not a valid 'msg'
 *
 * Argument 3 is not a valid menu handle
 *
 * Raises 88.900
 *
 * @param c    Thread context we are executing in.
 * @param pos  Argumet position
 * @param msg  "Some message"
 */
RexxObjectPtr invalidTypeException(RexxThreadContext *c, size_t pos, const char *type)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %zd is not a valid %s", pos, type);
    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

/**
 * Argument 'argument' is not a valid 'type'; found 'actual'
 *
 * Argument 1 is not a valid COLORREF; found a Directoy
 *
 * Raises 88.900
 *
 * @param c    Thread context we are executing in.
 * @param pos  Argumet position
 * @param type  "Some thing"
 * @param actual
 */
RexxObjectPtr invalidTypeException(RexxThreadContext *c, size_t pos, const char *type, RexxObjectPtr actual)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %zd is not a valid %s; found %s", pos, type, c->ObjectToStringValue(actual));
    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

void invalidImageException(RexxThreadContext *c, size_t pos, CSTRING type, CSTRING actual)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %zd must be a %s image; found %s", pos, type, actual);
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
void stringTooLongException(RexxThreadContext *c, size_t pos, size_t len, size_t realLen)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %zd must be less than %zd characters in length; length is %zd",
              pos, len, realLen);
    userDefinedMsgException(c, buffer);
}

/**
 * String produced by the <name> <type> is longer than allowed; (max == <max>)
 *
 * String produced by the enquote method is longer than allowed; (max == 2048)
 *
 * Raises 88.900
 *
 * @param c        Thread context we are executing in.
 * @param pos      Argumet position
 * @param len      Fixed length
 * @param realLen  Actual length
 */
void stringTooLongException(RexxThreadContext *c, CSTRING name, bool isMethod, size_t max)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "String produced by the %s %s is longer than allowed; (max == %zd)",
             name, isMethod ? "method" : "function", max);
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
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %d must be a whole number greater than %d; actual %s",
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
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %zd must be true or false; found \"%s\"",
              pos, c->ObjectToStringValue(actual));
    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

/**
 * Index <index> of the array, argument <argPos>, must be <msg>; found
 * "<actual>"
 *
 * Index 2 of the array, argument 2, must be exactly one of keywords POP or
 * SHOW; found "POINT"
 *
 *
 * Raises 88.900
 *
 * @param c        Thread context we are executing in.
 * @param argPos   Array argument position.
 * @param index    Index in array
 * @param msg      Some string message, or object
 * @param actual   Actual Rexx object, in string format.
 */
void wrongObjInArrayException(RexxThreadContext *c, size_t argPos, size_t index, CSTRING msg, CSTRING actual)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Index %zd of the array, argument %zd, must be %s; found \"%s\"",
              index, argPos, msg, actual);
    userDefinedMsgException(c, buffer);
}

/**
 * Index <index> of the array, argument <argPos>, must be <obj>
 *
 * Index 1 of the array, argument 2, must be "a Directory"
 *
 *
 * Raises 88.900
 *
 * @param c        Thread context we are executing in.
 * @param argPos   Array argument position.
 * @param index    Index in array
 * @param msg      Some string message, or object
 * @param actual   Actual Rexx object, in string format.
 */
void wrongObjInArrayException(RexxThreadContext *c, size_t argPos, size_t index, CSTRING obj)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Index %zd of the array, argument %zd, must be \"%s\"", index, argPos, obj);
    userDefinedMsgException(c, buffer);
}

void wrongObjInDirectoryException(RexxThreadContext *c, int argPos, CSTRING index, CSTRING needed, RexxObjectPtr actual)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
              "Index, %s, of argument %d, must be %s; found \"%s\"",
              index, argPos, needed, c->ObjectToStringValue(actual));
    userDefinedMsgException(c, buffer);
}

void missingIndexInDirectoryException(RexxThreadContext *c, int argPos, CSTRING index)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
              "Index, %s, of argument %d, is required",
              index, argPos);
    userDefinedMsgException(c, buffer);
}

/**
 * Argument <pos> must contain at least one of the indexes: <indexes>"
 *
 * Argument 1 must contain at least one of the indexes: constDirUsage,
 * symbolSrc, autoDetction, fontName, or fontSize
 *
 *
 * @param c
 * @param argPos
 * @param indexes
 */
void missingIndexesInDirectoryException(RexxThreadContext *c, int argPos, CSTRING indexes)
{
    char buffer[512];
    snprintf(buffer, sizeof(buffer),
              "The directory object, argument %d, most contain at least one of the indexes, %s",
              argPos, indexes);
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
void directoryIndexExceptionList(RexxThreadContext *c, size_t pos, CSTRING index, CSTRING list, CSTRING actual)
{
    char buffer[512];
    snprintf(buffer, sizeof(buffer),
              "Index, %s, of argument %zd must be one of %s; found \"%s\"", index, pos, list, actual);
    userDefinedMsgException(c, buffer);
}

/**
 * Index, <index>, of argument <pos> <msg>; found "<actual>"
 *
 * Index, PART, of argument 1 must contain at least one of the keywords: date,
 * rect, or name; found "today"
 *
 * @param c
 * @param pos
 * @param index
 * @param list
 * @param actual
 *
 * @return RexxObjectPtr
 */
void directoryIndexExceptionMsg(RexxThreadContext *c, size_t pos, CSTRING index, CSTRING msg, CSTRING actual)
{
    char buffer[512];
    snprintf(buffer, sizeof(buffer),
              "Index, %s, of argument %zd %s; found \"%s\"", index, pos, msg, actual);
    userDefinedMsgException(c, buffer);
}

/**
 * The wording of this exception is exactly the same for a Directory or a Stem
 * object.
 *
 * @param c
 * @param argPos
 * @param index
 */
void missingIndexInStemException(RexxThreadContext *c, int argPos, CSTRING index)
{
    return missingIndexInDirectoryException(c, argPos, index);
}

/**
 *  Raises 93.900
 *
 * Method argument <pos>, the Stem object, must have an index "0" containing a
 * non-negative whole number value.
 *
 * Method argument 5, the Stem object, must have an index "0' containing a
 * positive whold number value.
 *
 * @param c
 * @param pos
 */
void stemIndexZeroException(RexxMethodContext *c, size_t pos)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "Method argument %zd, the Stem object, must have an index \"0\" containing a non-negative whole number value", pos);
    userDefinedMsgException(c, buffer);
}


void emptyArrayException(RexxThreadContext *c, int argPos)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %d must be a non-empty array", argPos);
    userDefinedMsgException(c, buffer);
}

void arrayToLargeException(RexxThreadContext *c, uint32_t found, uint32_t max, int argPos)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %d, array items (%d) exceeds maximum (%d) allowed", argPos, found, max);
    userDefinedMsgException(c, buffer);
}
void arrayWrongSizeException(RexxThreadContext *c, size_t found, size_t need, int argPos)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %d, array items must equal (%zd), found (%zd)", argPos, need, found);
    userDefinedMsgException(c, buffer);
}


RexxObjectPtr sparseArrayException(RexxThreadContext *c, size_t argPos, size_t index)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %zd must be a non-sparse array, index %zd is missing", argPos, index);
    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

/**
 * Error 98.913
 *
 * 98 The language processor detected a specific error during execution.
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
 * Could not retrive the window handle information for a PlainBaseDialog object.
 *
 * similar to old 98.921
 *
 * @param c       Method context we are operating in.
 * @param item    What was to be retrieved
 * @param source  The object it was being retrieved from.
 */
void failedToRetrieveException(RexxThreadContext *c, CSTRING item, RexxObjectPtr source)
{
    char buf[128];

    RexxObjectPtr name = c->SendMessage0(source, "OBJECTNAME");
    snprintf(buf, sizeof(buf), "Could not retrieve the %s information for %s",
              item, c->ObjectToStringValue(name));

    c->RaiseException1(Rexx_Error_Execution_user_defined, c->String(buf));
}

void nullObjectException(RexxThreadContext *c, CSTRING name, size_t pos)
{
    char buffer[256];
    if ( pos == 0 )
    {
        snprintf(buffer, sizeof(buffer), "The %s object must not be null", name);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "Argument %zd, the %s object, must not be null", pos, name);
    }
    userDefinedMsgException(c, buffer);
}

void nullStringMethodException(RexxMethodContext *c, size_t pos)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Argument %zd, must not be the empty string", pos);
    c->RaiseException1(Rexx_Error_Incorrect_method_user_defined, c->String(buffer));
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

void notPositiveException(RexxThreadContext *c, size_t pos, RexxObjectPtr actual)
{
    c->RaiseException2(Rexx_Error_Invalid_argument_positive, c->StringSize(pos), actual);
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

RexxObjectPtr wrongRangeException(RexxThreadContext *c, size_t pos, uint32_t min, uint32_t max, RexxObjectPtr actual)
{
    c->RaiseException(Rexx_Error_Invalid_argument_range,
                      c->ArrayOfFour(c->StringSize(pos),
                                     c->UnsignedInt32(min),
                                     c->UnsignedInt32(max),
                                     actual));
    return NULLOBJECT;
}

RexxObjectPtr wrongRangeException(RexxMethodContext *c, size_t pos, uint32_t min, uint32_t max, uint32_t actual)
{
    return wrongRangeException(c->threadContext, pos, min, max, c->UnsignedInt32(actual));
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
 * Similar to 93.914
 *
 * Method argument <pos>, must contain one or more of <list>; found "<actual>"
 *
 * Method argument 2 must contain one or more of SKIPINVISIBLE, SKIPDISABLED,
 * SKIPTRANSPARENT, or ALL; found "BOGUS NOTVISIBLE"
 *
 * @param c       Thread context
 * @param pos
 * @param list
 * @param actual
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr wrongArgKeywordsException(RexxThreadContext *c, size_t pos, CSTRING list, CSTRING actual)
{

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Method argument %zd, must contain one or more of %s; found \"%s\"",
              pos, list, actual);
    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

RexxObjectPtr wrongArgKeywordsException(RexxThreadContext *c, size_t pos, CSTRING list, RexxObjectPtr actual)
{
    return wrongArgKeywordsException(c, pos, list, c->ObjectToStringValue(actual));
}

/**
 * Similar to 93.915 and 93.914  (actually a combination of the two.)
 *
 * Method argument <pos>, keyword must be exactly one of <list>; found
 * "<actual>"
 *
 * Method argument 2 must be exactly one of left, right, top, or bottom found
 * "Side"
 *
 * @param c
 * @param pos
 * @param list
 * @param actual  String, actual keyword
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr wrongArgKeywordException(RexxMethodContext *c, size_t pos, CSTRING list, CSTRING actual)
{
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Method argument %zd, keyword must be exactly one of %s; found \"%s\"", pos, list, actual);
    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

/**
 * Similar to 93.915 and 93.914  (actually a combination of the two.)
 *
 * Argument <pos>, keyword must be exactly one of <list>; found
 * "<actual>"
 *
 * Method argument 2 must be exactly one of left, right, top, or bottom found
 * "Side"
 *
 * @param c
 * @param pos
 * @param list
 * @param actual  String, actual keyword
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr wrongArgKeywordException(RexxThreadContext *c, size_t pos, CSTRING list, CSTRING actual)
{
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Argument %zd, keyword must be exactly one of %s; found \"%s\"", pos, list, actual);
    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
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
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "Method argument %zd, option must be one of %s; found \"%s\"", pos, list, actual);
    userDefinedMsgException(c, buffer);
    return NULLOBJECT;
}

RexxObjectPtr wrongArgOptionException(RexxThreadContext *c, size_t pos, CSTRING list, RexxObjectPtr actual)
{
    return wrongArgOptionException(c, pos, list, c->ObjectToStringValue(actual));
}

/**
 * 93.914
 * Method argument <argument> must be one of <values>; found "<value>"
 *
 * Method argument 1 must be one of the valid CSIDL_XXX constants; found "dog"
 *
 * @param argNumber
 * @param acceptable
 * @param actual
 */
RexxObjectPtr invalidConstantException(RexxMethodContext *c, size_t argNumber, char *msg,
                                       const char *sub, RexxObjectPtr actual)
{
    char buffer[512];
    snprintf(buffer, sizeof(buffer), msg, sub);

    return wrongArgValueException(c->threadContext, argNumber, buffer, actual);
}
RexxObjectPtr invalidConstantException(RexxMethodContext *c, size_t argNumber, char *msg, const char *sub, const char *actual)
{
    return invalidConstantException(c, argNumber, msg, sub, c->String(actual));
}

/**
 * Similar to error 43.001 Could not find routine <routine>
 *
 * Argument <pos>, (the <rtnName> routine,) could not be found
 *
 * Argument 2, (the "myCallback" routine,) could not be found
 *
 * @param c
 * @param pos
 * @param list
 * @param actual
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr noSuchRoutineException(RexxThreadContext *c, CSTRING rtnName, size_t pos)
{
    char buf[512];
    snprintf(buf, sizeof(buf), "Argument %zd, (the \"%s\" routine,) could not be found", pos, rtnName);
    c->RaiseException1(Rexx_Error_Invalid_argument_user_defined, c->String(buf));
    return NULLOBJECT;
}

/**
 * Similar to error 93.963 Call to unsupported or unimplemented method
 *
 * Call to unsupported or unimplemented routine (<rtnName)
 *
 * Argument 2, (the "myCallback" routine,) could not be found
 *
 * @param c
 * @param pos
 * @param list
 * @param actual
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr unsupportedRoutineException(RexxCallContext *c, CSTRING rtnName)
{
    char buf[512];
    snprintf(buf, sizeof(buf), "Call to unsupported or unimplemented routine (\"%s\")", rtnName);
    c->RaiseException1(Rexx_Error_Incorrect_call_user_defined, c->String(buf));
    return NULLOBJECT;
}

/**
 *  98.900
 *  Error 98 - Execution error
 *        The language processor detected a specific error during execution.
 *
 *  The return from method "name"() must be a whole number; found "actual"
 *  or
 *  The return from routine "name"() must be a whole number; found "actual"
 *
 *  The return from method commitHookCallBack() must be a whole number; found an
 *  array
 *
 */
RexxObjectPtr invalidReturnWholeNumberException(RexxThreadContext *c, CSTRING name, RexxObjectPtr actual, bool isMethod)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "The return from %s %s() must be a whole number; found %s",
             isMethod ? "method" : "routine", name, c->ObjectToStringValue(actual));

    c->RaiseException1(Rexx_Error_Execution_user_defined, c->String(buf));
    return NULLOBJECT;
}

/**
 *  98.900
 *  Error 98 - Execution error
 *        The language processor detected a specific error during execution.
 *
 *  The return from method "name"() must a logical; found "value"
 *
 *  The return from method onCustomDraw() must be a logical; found an Array
 */
void notBooleanReplyException(RexxThreadContext *c, CSTRING method, RexxObjectPtr actual)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "The return from method %s() must be a logical; found %s",
             method, c->ObjectToStringValue(actual));

    c->RaiseException1(Rexx_Error_Execution_user_defined, c->String(buf));
}

/**
 *  93.900
 *
 *  Error 93 - Incorrect call to method
 *        The specified method, built-in function, or external routine exists,
 *        but you used it incorrectly.
 *
 *  Argument pos must be in the range 0 to 4294967295; found "actual"
 *
 *  Argument 1 must be in the range 0 to 4294967295; found "an Array"
 */
void notUnsignedInt32Exception(RexxMethodContext *c, size_t pos, RexxObjectPtr actual)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "Argument %zd must be in the range 0 to 4294967295; found \"%s\"",
             pos, c->ObjectToStringValue(actual));

    c->RaiseException1(Rexx_Error_Incorrect_method_user_defined, c->String(buf));
}


/**
 * Tests if a string is a pointer string.
 *
 * Pointer strings are strings representing a pointer, handle, etc..  I.e. in
 * "0xdd" format. But, this really just tests for hexidecimal format.
 *
 * @param string  The string to test.
 *
 * @return True or false
 */
bool isPointerString(const char *string)
{
    if ( string != NULL && strlen(string) > 2 )
    {
        return *string == '0' && toupper(string[1]) == 'X' && isxdigit(string[2]);
    }
    return false;
}

/**
 * Converts a string in hexadecimal format (starts with 0x) to its pointer-sized
 * value.
 *
 * Note that this converts "0" to null, which is what we want.  It also accepts
 * a NULL pointer for string.
 *
 * @param string  The string to convert.
 *
 * @return The converted value, which could be null to begin with, or null if it
 *         is not converted.
 */
void *string2pointer(const char *string)
{
    void *pointer = NULL;
    if ( string != NULL && strlen(string) > 1 )
    {
        if ( string[1] == 'x' )
        {
            sscanf(string, "0x%p", &pointer);
        }
        else if ( string[1] == 'X' )
        {
            sscanf(string, "0X%p", &pointer);
        }
    }
    return pointer;
}

void *string2pointer(RexxMethodContext *c, RexxStringObject string)
{
    if ( string == NULLOBJECT )
    {
        return NULL;
    }
    return string2pointer(c->CString(string));
}

/**
 * A sort of special case used in dialog procedure functions.  We don't really
 * know what the user returned.  It is supposedly a pointer (some type of
 * handle, a HWND, or ...).
 *
 * There is no error, if it is not a handle, then null is returned.  The caller
 * would need to implement any type checking.
 *
 * @param c
 * @param ptr
 *
 * @return A handle, which may be null
 */
void *string2pointer(RexxThreadContext *c, RexxObjectPtr ptr)
{
    if ( ptr == NULLOBJECT )
    {
        return NULL;
    }
    return string2pointer(c->ObjectToStringValue(ptr));
}

/**
 * Converts a pointer-sized type to a pointer-string, or 0 if the pointer is
 * null.
 *
 * @param result   [out] Pointer-string is returned here.  Ensure the storage
 *                 pointed to is big enough for a 64-bit pointer.
 *
 * @param pointer  [in] The pointer to convert.
 *
 * @remarks  Pointer-sized type is used to indicate that this will work for
 *           opaque types, like HANDLE, HMENU, HINST, UINT_PTR, DWORD_PTR, etc.,
 *           that are pointer size.
 *
 *           For now, 0 is returned for null rather than 0x00000000 because
 *           many, many places in ooDialog test for 0 to detect error.
 */
void pointer2string(char *result, void *pointer)
{
    if ( pointer == NULL )
    {
        sprintf(result, "0");
    }
    else
    {
        sprintf(result, "0x%p", pointer);
    }
}


/**
 * Variation of above.  Converts the pointer and returns it as a
 * RexxStringObject.
 *
 * @param c        Method context we are operating in.
 * @param pointer  Pointer to convert
 *
 * @return A string object representing the pointer as either 0xffff1111 if not
 *         null, or as 0 if null.
 */
RexxStringObject pointer2string(RexxMethodContext *c, void *pointer)
{
    char buf[32];
    pointer2string(buf, pointer);
    return c->String(buf);
}

/**
 * Variation of above, but takes a thread context pointer instead of a method
 * context pointer. Converts the pointer and returns it as a RexxStringObject.
 *
 * @param c        Thread context we are operating in.
 * @param pointer  Pointer to convert
 *
 * @return A string object representing the pointer as either 0xffff1111 if not
 *         null, or as 0 if null.
 */
RexxStringObject pointer2string(RexxThreadContext *c, void *pointer)
{
    char buf[32];
    pointer2string(buf, pointer);
    return c->String(buf);
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

/**
 * Checks that an argument value is truly a Directory object.
 *
 * @param context
 * @param d
 * @param argPos
 *
 * @return a RexxDirectoryObject object on sucess, null on error.
 */
RexxDirectoryObject rxGetDirectory(RexxMethodContext *context, RexxObjectPtr d, size_t argPos)
{
    if ( requiredClass(context->threadContext, d, "Directory", argPos) )
    {
        return (RexxDirectoryObject)d;
    }
    return NULL;
}

bool requiredClass(RexxThreadContext *c, RexxObjectPtr obj, const char *name, size_t pos)
{
    if ( obj == NULLOBJECT )
    {
        wrongClassException(c, pos, name);
        return false;
    }
    else if ( ! c->IsOfType(obj, name) )
    {
        wrongClassException(c, pos, name, obj);
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
        if ( obj == c->True() )
        {
            return 1;
        }
        if ( obj == c->False() )
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
 *
 * @note  This function is using some things that work on Windows, but cause
 *        errors on Linux.  Just comment out until it can be researched.
 *        TODO PLEASE fix this.
 */
#ifdef _WIN32
bool rxStr2Number(RexxMethodContext *c, CSTRING str, uint64_t *number, size_t pos)
{
    char *end;
    *number = _strtoui64(str, &end, 0);
    if ( (end - str != strlen(str)) || errno == EINVAL || *number == _UI64_MAX )
    {
        invalidTypeException(c->threadContext, pos, "number");
        return false;
    }
    return true;
}
#endif

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
        invalidTypeException(c->threadContext, pos, "number");
        return false;
    }
    return true;
}

/**
 * Gets a Class object.
 *
 * This is for classes visible within the scope of a method context, like say
 * .PlainBaseDialog, or .Rect.  Use c->FindClass() to directly get classes from
 * the environment like .Bag or .Directory.  Use rxGetPackageClass() to get a
 * class from a thread context when a method context is not available.
 *
 * @param c     The method context we are operating in.
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
 * Gets a Class object.
 *
 * This is for use when a method context is not available.  Use
 * rxGetContextClass() for classes visible within the scope of a method context,
 * like say .PlainBaseDialog, or .Rect.  Use c->FindClass() to directly get
 * classes from the environment like .Bag or .Directory.
 *
 * @param c        The thread context we are operating in.
 * @param pkgName  The name of the package the class is located in.
 * @param clsName  The name of the class to try and find.
 *
 * @return The class object or null on failure.
 *
 * @remarks  When null is returned an error has been raised: 98.909
 *
 *   98: The language processor detected a specific error during execution. The
 *   associated error gives the reason for the error.
 *
 *   909: Class "class" not found
 *
 *           This function will load the package named by pkgName.  If the
 *           pacakge is already loaded this should be relatively expensive.
 *           This also implies that this function could be used to load a
 *           package that has not been loaded.
 */
RexxClassObject rxGetPackageClass(RexxThreadContext *c, CSTRING pkgName, CSTRING clsName)
{
    RexxClassObject theClass = NULL;

    RexxPackageObject pkg = c->LoadPackage(pkgName);
    if ( pkg != NULL )
    {
        theClass = c->FindPackageClass(pkg, clsName);
    }
    if ( theClass == NULL )
    {
        c->RaiseException1(Rexx_Error_Execution_noclass, c->String(clsName));
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


/**
 * Return a new object of one of the builtin ooRexx classes.
 *
 * This should never fail, provided the caller sends the right class name,
 * but, raise an exception if it does.
 *
 * @param c
 * @param className
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr rxNewBuiltinObject(RexxThreadContext *c, CSTRING className)
{
    RexxObjectPtr o = NULLOBJECT;
    RexxClassObject classObj = c->FindClass(className);

    if ( classObj != NULL )
    {
        o = c->SendMessage0(classObj, "NEW");
        if ( o == NULLOBJECT )
        {
            c->RaiseException2(Rexx_Error_No_method_name, classObj, c->String("NEW"));
        }
    }
    else
    {
        c->RaiseException1(Rexx_Error_Execution_noclass, c->String(className));
    }
    return o;
}
RexxObjectPtr rxNewBuiltinObject(RexxMethodContext *c, CSTRING className)
{
    return rxNewBuiltinObject(c->threadContext, className);
}


bool isOutOfMemoryException(RexxThreadContext *c)
{
    RexxCondition condition;
    RexxDirectoryObject condObj = c->GetConditionInfo();

    if ( condObj != NULLOBJECT )
    {
        c->DecodeConditionInfo(condObj, &condition);
        if ( condition.code == 48900 )
        {
            return true;
        }
    }
    return false;
}


/**
 * Given a condition object, extracts and returns as a whole number the subcode
 * of the condition.
 */
static inline wholenumber_t conditionSubCode(RexxCondition *condition)
{
    return (condition->code - (condition->rc * 1000));
}


/**
 * Doubles a buffer of size *bytes and returns the new buffer and new size.
 *
 * @param buffer
 * @param bytes
 *
 * @return char*
 *
 * @notes  The existing buffer is assumed to contain null terminated text.  This
 *         text is copied into the new buffer on success.  The existing buffer
 *         is freed.
 *
 *         Null is returned if memory allocation fails.
 */
static char *doubleBuffer(char *buffer, size_t *bytes)
{
    *bytes *= 2;
    char *tmp = (char *)malloc(*bytes);
    if ( tmp == NULL )
    {
        // We just bail.
        free(buffer);
        return NULL;
    }

    strcpy(tmp, buffer);
    free(buffer);
    return tmp;
}

/**
 * Returns a buffer with the typical condition message.  For example:
 *
 *      4 *-* say dt~number
 * Error 97 running C:\work\qTest.rex line 4:  Object method not found
 * Error 97.1:  Object "a DateTime" does not understand message "NUMBER"
 *
 * @param c       The thread context we are operating in.
 * @param major   The major error code, i.e., Error 93, the 93
 * @param minor   The minor error subcode, i.e., Error 93.900, the 900
 *
 * @returns  A buffer allocated through malloc containing the standard
 *           condition message.  The caller is responsible for freeing the
 *           buffer using free().
 *
 * @assumes  The the condition has already been preformed.
 *
 * @notes  Null is returned if free() fails.  If for some reason there is
 *         no condition object, the string: "No condition object" is returned.
 *         The caller must still free this string.
 *
 *         Either or both of major and minor can be null.  Both are only set on
 *         success.
 *
 *         This is an OS neutral version of an ooDialog function.
 */
static char *getConditionMsg(RexxThreadContext *c, wholenumber_t *major, wholenumber_t *minor)
{
#define BIG_BUF 2048
#define MED_BUF  512

    size_t bytes  = BIG_BUF;
    char *condMsg = (char *)malloc(bytes);
    if ( condMsg == NULL )
    {
        return condMsg;
    }
    memset(condMsg, 0, BIG_BUF);

    RexxDirectoryObject condObj = c->GetConditionInfo();
    RexxCondition       condition;
    if ( condObj == NULLOBJECT )
    {
        strcpy(condMsg, "No condition object");
        return condMsg;
    }

    size_t usedBytes = 0;
    size_t cBytes    = 0;
    char   buf[MED_BUF] = {'\0'};

    c->DecodeConditionInfo(condObj, &condition);

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
                    cBytes = snprintf(buf, MED_BUF, "%s\n", c->ObjectToStringValue(o));

                    while ( cBytes + usedBytes >= bytes )
                    {
                        condMsg = doubleBuffer(condMsg, &bytes);
                        if ( condMsg == NULL )
                        {
                            return NULL;
                        }
                    }

                    strcat(condMsg, buf);
                    usedBytes += cBytes;
                }
            }
        }
    }

    cBytes = snprintf(buf, MED_BUF, "Error %zd running %s line %zd: %s\n", condition.rc,
                       c->CString(condition.program), condition.position, c->CString(condition.errortext));

    // The next, last string is short.  We add some padding to the needed size
    // to account for it.  If we come up short, doubling the current buffer is
    // always sufficient to finish.
    if ( cBytes + usedBytes + 256 >= bytes )
    {
        condMsg = doubleBuffer(condMsg, &bytes);
        if ( condMsg == NULL )
        {
            return NULL;
        }
    }
    strcat(condMsg, buf);

    snprintf(buf, MED_BUF, "Error %zd.%03zd:  %s\n", condition.rc, conditionSubCode(&condition),
              c->CString(condition.message));
    strcat(condMsg, buf);

    if ( major != NULL )
    {
        *major = condition.rc;
    }
    if ( minor != NULL )
    {
        *minor = conditionSubCode(&condition);
    }
    return condMsg;
}

/**
 * Given a thread context, checks for a raised condition, and prints out the
 * standard condition message if there is a condition.
 *
 * @param c      Thread context we are operating in.
 * @param clear  True if the condition should be cleared, false if it should not
 *               be cleared.
 *
 * @return True if there was a condition, otherwise false.
 */
bool checkForCondition(RexxThreadContext *c, bool clear)
{
    if ( c->CheckCondition() )
    {
        // Use the local function.
        char *msg = getConditionMsg(c, NULL, NULL);
        if ( msg )
        {
            printf(msg);
            free(msg);
        }
            if ( clear )
            {
                c->ClearCondition();
            }
            return true;
        }
    return false;
}


/**
 * Test if a generic Rexx object is exactly some int.
 *
 * @param testFor  The int value being tested for.
 * @param val      The generic Rexx object, which could be null.
 * @param c        The thread context we are executing under.
 *
 * @return True if val is the int number we are testing for, otherwise false.
 */
bool isInt(int testFor, RexxObjectPtr val, RexxThreadContext *c)
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
 *
 * @note  This is changed to be case sensitive because of stricmp problems.
 *        TODO PLEASE fix this.
 */
bool isOfClassType(RexxMethodContext *c, RexxObjectPtr obj, CSTRING classID)
{
    if ( obj != NULLOBJECT && c->IsOfType(obj, "CLASS") )
    {
        RexxStringObject clsID = (RexxStringObject)c->SendMessage0(obj, "ID");
        if ( clsID != NULLOBJECT && strcmp(c->StringData(clsID), classID) == 0 )
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


/**
 * Returns the class ID of a Rexx object as a string, rather than printing it.
 *
 * Not that good of a function name, but meant to go hand in hand with
 * dbgPrintClassID.
 *
 * Useful in exception messages to identify exactly what a Rexx object is. Will
 * work with class objects or instance objects.
 *
 * @param c    The thread context we are operating in.
 * @param obj  The object to identify.
 */
CSTRING strPrintClassID(RexxThreadContext *c, RexxObjectPtr obj)
{
    if ( ! c->IsOfType(obj, "CLASS") )
    {
        obj = c->SendMessage0(obj, "CLASS");
    }

    if ( obj != NULLOBJECT )
    {
        RexxStringObject id = (RexxStringObject)c->SendMessage0(obj, "ID");
        if ( id != NULLOBJECT )
        {
            return c->CString(id);
        }
        else
        {
            return "<not known>";
        }
    }
    else
    {
        return "<not known>";
    }
}


void dbgPrintClassID(RexxMethodContext *c, RexxObjectPtr obj)
{
    dbgPrintClassID(c->threadContext, obj);
}


CSTRING strPrintClassID(RexxMethodContext *c, RexxObjectPtr obj)
{
    return strPrintClassID(c->threadContext, obj);
}
