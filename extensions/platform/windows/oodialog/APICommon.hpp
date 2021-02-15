/*----------------------------------------------------------------------------*/;
/*                                                                            */;
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */;
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */;
/*                                                                            */;
/* This program and the accompanying materials are made available under       */;
/* the terms of the Common Public License v1.0 which accompanies this         */;
/* distribution. A copy is also available at the following address:           */;
/* https://www.oorexx.org/license.html                                        */;
/*                                                                            */;
/* Redistribution and use in source and binary forms, with or                 */;
/* without modification, are permitted provided that the following            */;
/* conditions are met:                                                        */;
/*                                                                            */;
/* Redistributions of source code must retain the above copyright             */;
/* notice, this list of conditions and the following disclaimer.              */;
/* Redistributions in binary form must reproduce the above copyright          */;
/* notice, this list of conditions and the following disclaimer in            */;
/* the documentation and/or other materials provided with the distribution.   */;
/*                                                                            */;
/* Neither the name of Rexx Language Association nor the names                */;
/* of its contributors may be used to endorse or promote products             */;
/* derived from this software without specific prior written permission.      */;
/*                                                                            */;
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */;
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */;
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */;
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */;
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */;
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */;
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */;
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */;
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */;
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */;
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */;
/*                                                                            */;
/*----------------------------------------------------------------------------*/;

#ifndef APICommon_Included
#define APICommon_Included


extern RexxObjectPtr       TheTrueObj;
extern RexxObjectPtr       TheFalseObj;
extern RexxObjectPtr       TheNilObj;
extern RexxObjectPtr       TheZeroObj;
extern RexxObjectPtr       TheOneObj;
extern RexxObjectPtr       TheTwoObj;
extern RexxObjectPtr       TheNegativeOneObj;
extern RexxObjectPtr       TheZeroPointerObj;
extern RexxDirectoryObject TheDotLocalObj;

#define NO_MEMORY_MSG             "failed to allocate memory"
#define INVALID_CONSTANT_MSG      "the valid %s_XXX constants"
#define NO_LOCAL_ENVIRONMENT_MSG  "the .local environment was not found"

extern bool RexxEntry packageLoadHelper(RexxThreadContext *c);

extern void  severeErrorException(RexxThreadContext *c, const char *msg);
extern void  systemServiceException(RexxThreadContext *context, const char *msg);
extern void  systemServiceException(RexxThreadContext *context, const char *msg, const char *sub);
extern void  systemServiceExceptionCode(RexxThreadContext *context, const char *msg, const char *arg1, uint32_t rc);
extern void  systemServiceExceptionCode(RexxThreadContext *context, const char *msg, const char *arg1);
extern void  outOfMemoryException(RexxThreadContext *c);
extern void *baseClassInitializationException(RexxThreadContext *c);
extern void *baseClassInitializationException(RexxThreadContext *c, CSTRING clsName);
extern void *baseClassInitializationException(RexxMethodContext *c);
extern void *baseClassInitializationException(RexxMethodContext *c, CSTRING clsName);
extern void *baseClassInitializationException(RexxThreadContext *c, CSTRING clsName, CSTRING msg);
extern void *baseClassInitializationException(RexxMethodContext *c, CSTRING clsName, CSTRING msg);
extern void  userDefinedMsgException(RexxThreadContext *c, CSTRING msg);
extern void  userDefinedMsgException(RexxThreadContext *c, CSTRING formatStr, int number);
extern void  userDefinedMsgException(RexxThreadContext *c, int pos, CSTRING msg);
extern void  userDefinedMsgException(RexxMethodContext *c, CSTRING msg);
extern void  userDefinedMsgException(RexxMethodContext *c, size_t pos, CSTRING msg);
extern void  invalidImageException(RexxThreadContext *c, size_t pos, CSTRING type, CSTRING actual);
extern void  stringTooLongException(RexxThreadContext *c, size_t pos, size_t len, size_t realLen);
extern void  stringTooLongException(RexxThreadContext *c, CSTRING name, bool isMethod, size_t max);
extern void  numberTooSmallException(RexxThreadContext *c, int pos, int min, RexxObjectPtr actual);
extern void  notNonNegativeException(RexxThreadContext *c, size_t pos, RexxObjectPtr actual);
extern void  notPositiveException(RexxThreadContext *c, size_t pos, RexxObjectPtr actual);
extern void  wrongObjInArrayException(RexxThreadContext *c, size_t argPos, size_t index, CSTRING msg, CSTRING actual);
extern void  wrongObjInArrayException(RexxThreadContext *c, size_t argPos, size_t index, CSTRING obj, RexxObjectPtr actual);
extern void  wrongObjInArrayException(RexxThreadContext *c, size_t argPos, size_t index, CSTRING obj);
extern void  wrongObjInDirectoryException(RexxThreadContext *c, int argPos, CSTRING index, CSTRING needed, RexxObjectPtr actual);
extern void *executionErrorException(RexxThreadContext *c, CSTRING msg);
extern void  doOverException(RexxThreadContext *c, RexxObjectPtr obj);
extern void  failedToRetrieveException(RexxThreadContext *c, CSTRING item, RexxObjectPtr source);
extern void  missingIndexInDirectoryException(RexxThreadContext *c, int argPos, CSTRING index);
extern void  directoryIndexExceptionMsg(RexxThreadContext *c, size_t pos, CSTRING index, CSTRING msg, CSTRING actual);
extern void  directoryIndexExceptionList(RexxThreadContext *, size_t pos, CSTRING index, CSTRING list, CSTRING actual);
extern void  missingIndexesInDirectoryException(RexxThreadContext *c, int argPos, CSTRING indexes);
extern void  missingIndexInStemException(RexxThreadContext *c, int argPos, CSTRING index);
extern void  stemIndexZeroException(RexxMethodContext *c, size_t pos);
extern void  emptyArrayException(RexxThreadContext *c, int argPos);
extern void  arrayWrongSizeException(RexxThreadContext *c, size_t found, size_t need, int argPos);
extern void  arrayToLargeException(RexxThreadContext *c, uint32_t found, uint32_t max, int argPos);
extern void  nullObjectException(RexxThreadContext *c, CSTRING name, size_t pos);
extern void  nullObjectException(RexxThreadContext *c, CSTRING name);
extern void  nullPointerException(RexxThreadContext *c, int pos);
extern void  nullStringMethodException(RexxMethodContext *c, size_t pos);

extern RexxObjectPtr sparseArrayException(RexxThreadContext *c, size_t argPos, size_t index);
extern RexxObjectPtr wrongClassException(RexxThreadContext *c, size_t pos, const char *n);
extern RexxObjectPtr wrongClassException(RexxThreadContext *c, size_t pos, const char *n, RexxObjectPtr actual);
extern RexxObjectPtr wrongClassListException(RexxThreadContext *c, size_t pos, const char *n, RexxObjectPtr _actual);
extern RexxObjectPtr wrongArgValueException(RexxThreadContext *c, size_t pos, const char *list, RexxObjectPtr actual);
extern RexxObjectPtr wrongArgValueException(RexxThreadContext *c, size_t pos, const char *list, const char *actual);
extern RexxObjectPtr wrongArgKeywordsException(RexxThreadContext *c, size_t pos, CSTRING list, CSTRING actual);
extern RexxObjectPtr wrongArgKeywordsException(RexxThreadContext *c, size_t pos, CSTRING list, RexxObjectPtr actual);
extern RexxObjectPtr wrongArgKeywordException(RexxThreadContext *c, size_t pos, CSTRING list, CSTRING actual);
extern RexxObjectPtr wrongArgKeywordException(RexxMethodContext *c, size_t pos, CSTRING list, CSTRING actual);
extern RexxObjectPtr invalidConstantException(RexxMethodContext *c, size_t argNumber, char *msg, const char *sub, RexxObjectPtr actual);
extern RexxObjectPtr invalidConstantException(RexxMethodContext *c, size_t argNumber, char *msg, const char *sub, const char *actual);
extern RexxObjectPtr wrongRangeException(RexxThreadContext *c, size_t pos, int min, int max, RexxObjectPtr actual);
extern RexxObjectPtr wrongRangeException(RexxThreadContext *c, size_t pos, int min, int max, int actual);
extern RexxObjectPtr wrongRangeException(RexxThreadContext *c, size_t pos, uint32_t min, uint32_t max, RexxObjectPtr actual);
extern RexxObjectPtr wrongRangeException(RexxMethodContext *c, size_t pos, uint32_t min, uint32_t max, uint32_t actual);
extern RexxObjectPtr notBooleanException(RexxThreadContext *c, size_t pos, RexxObjectPtr actual);
extern RexxObjectPtr wrongArgOptionException(RexxThreadContext *c, size_t pos, CSTRING list, RexxObjectPtr actual);
extern RexxObjectPtr wrongArgOptionException(RexxThreadContext *c, size_t pos, CSTRING list, CSTRING actual);
extern RexxObjectPtr invalidTypeException(RexxThreadContext *c, size_t pos, const char *type);
extern RexxObjectPtr invalidTypeException(RexxThreadContext *c, size_t pos, const char *type, RexxObjectPtr actual);
extern RexxObjectPtr noSuchRoutineException(RexxThreadContext *c, CSTRING rtnName, size_t pos);
extern RexxObjectPtr unsupportedRoutineException(RexxCallContext *c, CSTRING rtnName);
extern RexxObjectPtr invalidReturnWholeNumberException(RexxThreadContext *c, CSTRING name, RexxObjectPtr actual, bool isMethod);
extern void          notBooleanReplyException(RexxThreadContext *c, CSTRING method, RexxObjectPtr actual);
extern void          notUnsignedInt32Exception(RexxMethodContext *c, size_t pos, RexxObjectPtr actual);

extern bool              isPointerString(const char *string);
extern void *            string2pointer(const char *string);
extern void *            string2pointer(RexxMethodContext *c, RexxStringObject string);
extern void *            string2pointer(RexxThreadContext *c, RexxObjectPtr ptr);
extern void              pointer2string(char *, void *pointer);
extern RexxStringObject  pointer2string(RexxMethodContext *, void *);
extern RexxStringObject  pointer2string(RexxThreadContext *c, void *pointer);

extern CSTRING rxGetStringAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name);
extern bool    rxGetNumberAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name, wholenumber_t *pNumber);
extern bool    rxGetUIntPtrAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name, uintptr_t *pNumber);
extern bool    rxGetUInt32Attribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name, uint32_t *pNumber);
extern
RexxDirectoryObject rxGetDirectory(RexxMethodContext *context, RexxObjectPtr d, size_t argPos);

extern bool            requiredClass(RexxThreadContext *c, RexxObjectPtr obj, const char *name, size_t pos);
extern int32_t         getLogical(RexxThreadContext *c, RexxObjectPtr obj);
extern size_t          rxArgCount(RexxMethodContext * context);
extern bool            rxStr2Number(RexxMethodContext *c, CSTRING str, uint64_t *number, size_t pos);
extern bool            rxStr2Number32(RexxMethodContext *c, CSTRING str, uint32_t *number, size_t pos);
extern RexxClassObject rxGetContextClass(RexxMethodContext *c, CSTRING name);
extern RexxClassObject rxGetPackageClass(RexxThreadContext *c, CSTRING pkgName, CSTRING clsName);
extern RexxObjectPtr   rxSetObjVar(RexxMethodContext *c, CSTRING varName, RexxObjectPtr val);
extern RexxObjectPtr   rxNewBuiltinObject(RexxMethodContext *c, CSTRING className);
extern RexxObjectPtr   rxNewBuiltinObject(RexxThreadContext *c, CSTRING className);

extern bool    isOutOfMemoryException(RexxThreadContext *c);
extern bool    checkForCondition(RexxThreadContext *c, bool clear);
extern bool    isOutOfMemoryException(RexxThreadContext *c);
extern bool    isInt(int, RexxObjectPtr, RexxThreadContext *);
extern bool    isOfClassType(RexxMethodContext *, RexxObjectPtr, CSTRING);
extern void    dbgPrintClassID(RexxThreadContext *c, RexxObjectPtr obj);
extern void    dbgPrintClassID(RexxMethodContext *c, RexxObjectPtr obj);
extern CSTRING strPrintClassID(RexxThreadContext *c, RexxObjectPtr obj);
extern CSTRING strPrintClassID(RexxMethodContext *c, RexxObjectPtr obj);

/**
 * Return true if the Rexx object is equivalent to true, otherwise return false.
 *
 * Note that a return of false does not imply that the Rexx object is equivalent
 * to false.
 *
 * @param c
 * @param obj
 *
 * @return bool
 */
inline bool isTrue(RexxThreadContext *c, RexxObjectPtr obj)
{
    return getLogical(c, obj) == 1;
}
inline bool isTrue(RexxMethodContext *c, RexxObjectPtr obj)
{
    return getLogical(c->threadContext, obj) == 1;
}

/**
 *  Message "msg" did not return a result
 *
 *  Message "onFormat" did not return a result
 *
 *  Raises 91.999
 *
 * @param c    The thread context we are operating under.
 * @param msg  The message (method) name that did not return a value
 *
 * @return NULLOBJECT
 */
inline RexxObjectPtr noMsgReturnException(RexxThreadContext *c, RexxStringObject msg)
{
    c->RaiseException1(Rexx_Error_No_result_object_message, msg);
    return NULLOBJECT;
}
inline RexxObjectPtr noMsgReturnException(RexxThreadContext *c, CSTRING msg)
{
    return noMsgReturnException(c, c->String(msg));
}

/**
 *  No data returned from function "function"
 *
 *  No data returned from function "commitHookCallback"
 *
 *  Raises 44.001
 *
 * @param c    The thread context we are operating under.
 * @param msg  The function name that did not return a value
 *
 * @return NULLOBJECT
 */
inline RexxObjectPtr noRoutineReturnException(RexxThreadContext *c, RexxStringObject rtnName)
{
    c->RaiseException1(Rexx_Error_Function_no_data_function, rtnName);
    return NULLOBJECT;
}
inline RexxObjectPtr noRoutineReturnException(RexxThreadContext *c, CSTRING rtnName)
{
    return noRoutineReturnException(c, c->String(rtnName));
}

/**
 *  Missing argument; argument 'argument' is required
 *
 *  Missing argument; argument 2 is required
 *
 *  Raises 88.901
 *
 * @param c    The thread context we are operating under.
 * @param pos  The 'argument' position.
 *
 * @return NULLOBJECT
 */
inline RexxObjectPtr missingArgException(RexxThreadContext *c, size_t argPos)
{
    c->RaiseException1(Rexx_Error_Invalid_argument_noarg, c->WholeNumber(argPos));
    return NULLOBJECT;
}

/**
 *  Missing argument in method; argument 'argument' is required
 *
 *  Missing argument in method; argument 2 is required
 *
 *  Raises 93.90
 *
 * @param c    The method context we are operating under.
 * @param pos  The 'argument' position.
 *
 * @return NULLOBJECT
 */
inline RexxObjectPtr missingArgException(RexxMethodContext *c, size_t argPos)
{
    c->RaiseException1(Rexx_Error_Incorrect_method_noarg, c->WholeNumber(argPos));
    return NULLOBJECT;
}

/**
 *  Too many arguments in invocation; 'number' expected
 *
 *  Too many arguments in invocation; 5 expected
 *
 *  Raises 93.903
 *
 * @param c    The thread context we are operating under.
 * @param max  The maximum arguments expected.
 *
 * @return NULLOBJECT
 */
inline RexxObjectPtr tooManyArgsException(RexxThreadContext *c, size_t max)
{
    c->RaiseException1(Rexx_Error_Invalid_argument_maxarg, c->WholeNumber(max));
    return NULLOBJECT;
}

/**
 *  Method argument argument must be zero or a positive whole number; found
 *  "value"
 *
 *  Method argument 3 must be zero or a positive whole number; found "an Array"
 *
 *  Raises 93.906
 *
 * @param c    The thread context we are operating under.
 * @param max  The maximum arguments expected.
 *
 * @return NULLOBJECT
 */
inline RexxObjectPtr notPositiveArgException(RexxThreadContext *c, size_t argPos, RexxObjectPtr actual)
{
    c->RaiseException2(Rexx_Error_Incorrect_method_nonnegative, c->WholeNumber(argPos), actual);
    return NULLOBJECT;
}


/**
 * Index <index> of the array, argument <argPos>, must be <msg>; found
 * "<actual>"
 *
 * Index 2 of the array, argument 2, must be exactly one of keywords POP or
 * SHOW; found "POINT"
 *
 * Raises 88.900
 *
 * @param c        Thread context we are executing in.
 * @param argPos   Array argument position.
 * @param index    Index in array
 * @param msg      Some string message, or object name
 * @param actual   Actual Rexx object,
 */
inline void wrongObjInArrayException(RexxThreadContext *c, size_t argPos, size_t index, CSTRING msg, RexxObjectPtr actual)
{
    wrongObjInArrayException(c, argPos, index, msg, c->ObjectToStringValue(actual));
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
 * @param actual  Rexx object, actual object
 *
 * @return RexxObjectPtr
 */
inline RexxObjectPtr wrongArgKeywordException(RexxMethodContext *c, size_t pos, CSTRING list, RexxObjectPtr actual)
{
    return wrongArgKeywordException(c, pos, list, c->ObjectToStringValue(actual));
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
inline void directoryIndexExceptionList(RexxThreadContext *c, size_t pos, CSTRING index, CSTRING list, RexxObjectPtr actual)
{
    directoryIndexExceptionList(c, pos, index, list, c->ObjectToStringValue(actual));
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
inline void directoryIndexExceptionMsg(RexxThreadContext *c, size_t pos, CSTRING index, CSTRING msg, RexxObjectPtr actual)
{
    directoryIndexExceptionMsg(c, pos, index, msg, c->ObjectToStringValue(actual));
}

/**
 * Aargument <pos> must contain at least one of the indexes: <indexes>"
 *
 * Argument 1 must contain at least one of the indexes: constDirUsage,
 * symbolSrc, autoDetction, fontName, or fontSize
 *
 *
 * @param c
 * @param argPos
 * @param indexes
 */
inline void missingIndexesInDirectoryException(RexxMethodContext *c, int argPos, CSTRING indexes)
{
    missingIndexesInDirectoryException(c->threadContext, argPos, indexes);
}

inline RexxObjectPtr rxNewBag(RexxMethodContext *c)
{
    return rxNewBuiltinObject(c, "BAG");
}

inline RexxObjectPtr rxNewList(RexxMethodContext *c)
{
    return rxNewBuiltinObject(c, "LIST");
}

inline RexxObjectPtr rxNewQueue(RexxMethodContext *c)
{
    return rxNewBuiltinObject(c, "QUEUE");
}

inline RexxObjectPtr rxNewSet(RexxMethodContext *c)
{
    return rxNewBuiltinObject(c, "SET");
}


#endif
