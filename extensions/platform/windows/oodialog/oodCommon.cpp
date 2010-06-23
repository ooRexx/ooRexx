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
 * oodCommon.cpp
 *
 * Contains convenience / helper functions used throughout the ooDialog modules.
 */

#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include <CommCtrl.h>
#include <shlwapi.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"

/**
 * 49.900
 * 49 -> A severe error was detected in the language processor or execution
 *       process during internal self-consistency checks.
 *
 * 900 -> User message
 *
 * Call like this:
 *
 * ooDialogInternalException(c, __FUNCTION__, __LINE__, __DATE__, __FILE__);
 *
 * @param c
 * @param function
 * @param line
 * @param date
 * @param file
 */
void ooDialogInternalException(RexxMethodContext *c, char *function, int line, char *date, char *file)
{
    TCHAR buf[512];
    _snprintf(buf, sizeof(buf), "Interpretation error: ooDialog's internal state is inconsistent  "
                                "Function: %s line: %d compiled date: %s  File: %s", function, line, date, file);

    c->RaiseException1(Rexx_Error_Interpretation_user_defined, c->String(buf));
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
 *                return baseClassIntializationException(c);
 *            }
 *
 * @remarks  This error should be used when the CSelf pointer is null.  It can
 *           only happen (I believe) when the user inovkes a method on self in
 *           init() before the super class init() has run.  For example:
 *
 *           ::method init
 *             self~create(30, 30, 257, 123, "Simple Dialog", "CENTER")
 *             forward class (super) continue
 *
 *           Unfortunately, I have sample ooDialog programs from users that do
 *           just this sort of thing.  Prior to the conversion to the C++ APIs,
 *           the programs probably did not work as the user thought they were
 *           working, but it was not fatal.  However, now a null CSelf pointer
 *           causes a crash if not checked for.
 */
void *baseClassIntializationException(RexxMethodContext *c)
{
    return executionErrorException(c->threadContext, "The base class has not been initialized correctly");
}

/**
 *  93.900
 *  Error 93 - Incorrect call to method
 *        The specified method, built-in function, or external routine exists,
 *        but you used it incorrectly.
 *
 *  The "methName" method can not be invoked on "objectName" when the "msg"
 *
 *  The connectEdit method can not be invoked on a StyleDlg when the Windows
 *  dialog does not exist.
 *
 * @param c
 * @param rxDlg
 * @param msg
 */
RexxObjectPtr methodCanNotBeInvokedException(RexxMethodContext *c, RexxObjectPtr rxDlg, CSTRING msg)
{
    TCHAR buf[512];
    _snprintf(buf, sizeof(buf), "The %s method can not be invoked on %s when the %s.",
              c->GetMessageName(), c->ObjectToStringValue(rxDlg), msg);
    c->RaiseException1(Rexx_Error_Incorrect_method_user_defined, c->String(buf));
    return NULLOBJECT;
}

/**
 *  93.900
 *  Error 93 - Incorrect call to method
 *        The specified method, built-in function, or external routine exists,
 *        but you used it incorrectly.
 *
 *  Argument "position" is not a valid category page number; found "num"
 *
 *  Arguemnt 2 is not a valid category page number; found 0
 *
 * @param c
 * @param rxDlg
 */
RexxObjectPtr invalidCategoryPageException(RexxMethodContext *c, int pageNum, int pos)
{
    TCHAR buf[256];
    _snprintf(buf, sizeof(buf), "Argument %d is not a valid category page number; found %d", pos, pageNum);
    c->RaiseException1(Rexx_Error_Incorrect_method_user_defined, c->String(buf));
    return NULLOBJECT;
}


/**
 *  Error 98.900
 *
 *  98 The language processor detected a specific error during execution. The
 *  associated error gives the reason for the error.
 *
 *  900 User message.
 *
 *  The windows message reply must be of the 'class' class
 *
 *  The windows message reply must be of the DateTime class
 *
 * @param c    The thread context we are operating under.
 * @param n    The name of the class expected.
 *
 * @return Pointer to void, could be used in the return statement of a method
 *         to return NULLOBJECT after the exeception is raised.
 */
void *wrongClassReplyException(RexxThreadContext *c, const char *n)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), "The windows message reply must be of the %s class", n);
    return executionErrorException(c, buffer);
}

void controlFailedException(RexxThreadContext *c, const char *msg, const char *func, const char *control)
{
    TCHAR buffer[256];
    _snprintf(buffer, sizeof(buffer), msg, func, control);
    systemServiceException(c, buffer);
}


void wrongWindowStyleException(RexxMethodContext *c, const char *obj, const char *style)
{
    char msg[128];
    _snprintf(msg, sizeof(msg), "This %s does not have the %s style", obj, style);
    userDefinedMsgException(c->threadContext, msg);
}

RexxObjectPtr wrongWindowsVersionException(RexxMethodContext *context, const char *methodName, const char *windows)
{
    char msg[256];
    _snprintf(msg, sizeof(msg), "The %s() method requires Windows %s or later", methodName, windows);
    context->RaiseException1(Rexx_Error_Incorrect_method_user_defined, context->String(msg));
    return NULLOBJECT;
}


bool requiredComCtl32Version(RexxMethodContext *context, const char *methodName, DWORD minimum)
{
    if ( ComCtl32Version < minimum )
    {
        char msg[256];
        _snprintf(msg, sizeof(msg), "The %s() method requires %s or later", methodName, comctl32VersionName(minimum));
        context->RaiseException1(Rexx_Error_System_service_user_defined, context->String(msg));
        return false;
    }
    return true;
}


/**
 * Given an unknown Rexx object and a list of possible ooDialog classes,
 * determines the ooDialog class of the object.
 *
 * @param c      The method context we are operating under.
 * @param obj    The Rexx object whose class needs to be determined.
 * @param types  An array of ooDialog class types to test for.
 * @param count  The count of types in the array.
 *
 * @return The oodClass_t of the Rexx object.  If no match is made oodUnknown is
 *         returned.
 *
 * @note  The caller needs to be careful with the order of class types in the
 *        array.  A subclass type needs to come before the parent class type in
 *        the array.  For instance, to distinguish between a UserDialog object
 *        and a PlainBaseDialog object, oodUserDialog must precede
 *        oodPlainBaseDialog.  If oodPlainBaseDialog comes first, then the
 *        object will test true as a PlainBaseDialog object and the test for a
 *        UserDialog object will never be executed.
 */
oodClass_t oodClass(RexxMethodContext *c, RexxObjectPtr obj, oodClass_t *types, size_t count)
{
    if ( obj != NULLOBJECT )
    {
        for ( size_t i = 0; i < count; count++, types++)
        {
            switch ( *types )
            {
                case oodCategoryDialog :
                    if ( c->IsOfType(obj, "CATEGORYDIALOG") )
                    {
                        return oodCategoryDialog;
                    }
                    break;

                case oodPlainBaseDialog :
                    if ( c->IsOfType(obj, "PLAINBASEDIALOG") )
                    {
                        return oodPlainBaseDialog;
                    }
                    break;

                case oodListBox :
                    if ( c->IsOfType(obj, "LISTBOX") )
                    {
                        return oodListBox;
                    }
                    break;

                default :
                    // Really shouldn't happen.
                    break;

            }
        }
    }

    return oodUnknown;
}

/**
 * Checks if a Rexx object is either -1 or IDC_STATIC.
 *
 * @param c   The method context we are operating under.
 * @param id  The object to check
 *
 * @return True if the object is -1 or IDC_STATIC, otherwise false.
 */
static bool isStaticID(RexxMethodContext *c, RexxObjectPtr id)
{
    int tmp;
    if ( c->ObjectToInt32(id, &tmp) )
    {
        if ( tmp == -1 )
        {
            return true;
        }
    }
    else
    {
        if ( stricmp(c->ObjectToStringValue(id), "IDC_STATIC") == 0 )
        {
            return true;
        }
    }
    return false;
}

/**
 * Resolves a resource ID used in a native API method call to its numeric value.
 * The resource ID may be numeric or symbolic.  An exception is raised if the ID
 * can not be resolved.
 *
 * @param context    Method context for the method call.
 * @param oodObj     ooDialog object that has inherited .ResourceUtils.
 *                   <Assumed>
 * @param id         Resource ID.
 * @param argPosObj  Arg position of the assumed ooDialog object.  Used for
 *                   raised exceptions.
 * @param argPosID   Arg position of the ID, used for raised exceptions.
 *
 * @return The resolved numeric ID or -1 cast as an uint32_t on success,
 *         OOD_ID_EXCEPTION on error.
 *
 * @remarks  When the oodObj argument is known to be a .ResourceUtils, then use
 *           -1 for the argPosObj argument.  In this case the required class
 *           check can be / is skipped.
 *
 *           This function special cases -1 or IDC_STATIC.  When id is -1 or
 *           IDC_STATIC then the return will be: (uint32_t)-1.  Otherwise, id
 *           has to be a non-zero number or a symbol that resolves to a non-zero
 *           number.  This is slightly different than how things work on the
 *           Rexx side where the user could put anything into constDir.
 *
 *           New methods / functions added to ooDialog will raise an exception
 *           if the resource ID can not be resolved.  But, older existing
 *           ooDialog methods always returned -1, and that behavior is currently
 *           being preserved.  Use oodSafeResolveID() for those cases.
 */
uint32_t oodResolveSymbolicID(RexxMethodContext *context, RexxObjectPtr oodObj, RexxObjectPtr id,
                              int argPosObj, size_t argPosID)
{
    uint32_t result = OOD_ID_EXCEPTION;

    if ( argPosObj != -1 && ! requiredClass(context->threadContext, oodObj, "ResourceUtils", argPosObj) )
    {
        goto done_out;
    }
    if ( isStaticID(context, id) )
    {
        result = (uint32_t)-1;
        goto done_out;
    }

    char *symbol = NULL;

    if ( ! context->ObjectToUnsignedInt32(id, &result) )
    {
        RexxDirectoryObject constDir = (RexxDirectoryObject)context->SendMessage0(oodObj, "CONSTDIR");
        if ( constDir != NULLOBJECT )
        {
            /* The original ooDialog code uses:
             *   self~ConstDir[id~space(0)~translate]
             * Why they allowed a space in a symbolic ID, I don't understand.
             * But, I guess we need to preserve that.
             */

            symbol = strdupupr_nospace(context->ObjectToStringValue(id));
            if ( symbol == NULL )
            {
                outOfMemoryException(context->threadContext);
                goto done_out;
            }

            RexxObjectPtr item = context->DirectoryAt(constDir, symbol);
            if ( item != NULLOBJECT )
            {
                 context->ObjectToUnsignedInt32(item, &result);
            }
        }
    }

    safeFree(symbol);

    if ( result == OOD_ID_EXCEPTION )
    {
        wrongArgValueException(context->threadContext, argPosID, "a valid numeric ID or a valid symbolic ID" , id);
    }

done_out:
    return result;
}

/**
 * Resolves a resource ID used in a native API method call to its numeric value,
 * without raising an exception. The resource ID may be numeric or symbolic.
 *
 * @param pID        The numeric resource ID is returned here.
 * @param context    Method context for the method call.
 * @param oodObj     ooDialog object that has inherited .ResourceUtils.
 *                   <Assumed>
 * @param id         Resource ID.
 * @param argPosObj  Arg position of the assumed ooDialog object.  Used for
 *                   raised exceptions.
 * @param argPosID   Arg position of the ID, used for raised exceptions.
 *
 * @return True if the ID was resolved, otherwise false.
 *
 * @see oodResolveSymbolicID() for complete details.
 *
 * @note  This function merely calls oodResolveSymbolicID() to do the work.  If
 *        an exception is raised, it is cleared and false returned.
 */
bool oodSafeResolveID(uint32_t *pID, RexxMethodContext *context, RexxObjectPtr oodObj, RexxObjectPtr id,
                   int argPosObj, size_t argPosID)
{
    uint32_t tmp = oodResolveSymbolicID(context, oodObj, id, argPosObj, argPosID);
    if ( tmp == OOD_ID_EXCEPTION )
    {
        context->ClearCondition();
        return false;
    }
    *pID = tmp;
    return true;
}


DWORD oodGetSysErrCode(RexxThreadContext *c)
{
    uint32_t code = 0;
    RexxObjectPtr rxCode = c->DirectoryAt(TheDotLocalObj, "SYSTEMERRORCODE");
    c->UnsignedInt32(rxCode, &code);
    return (DWORD)code;
}

void oodSetSysErrCode(RexxThreadContext *context, DWORD code)
{
    context->DirectoryPut(TheDotLocalObj, context->UnsignedInt32(code), "SYSTEMERRORCODE");
}


void oodResetSysErrCode(RexxThreadContext *context)
{
    context->DirectoryPut(TheDotLocalObj, TheZeroObj, "SYSTEMERRORCODE");
}


bool oodGetWParam(RexxMethodContext *c, RexxObjectPtr wp, WPARAM *wParam, size_t argPos, bool required)
{
    WPARAM result = 0;
    bool success = true;

    if ( wp == NULLOBJECT )
    {
        if ( required )
        {
            success = false;
            missingArgException(c->threadContext, argPos);
        }
        goto done_out;
    }

    if ( c->Uintptr(wp, (uintptr_t *)&result) )
    {
        goto done_out;
    }

    if ( c->IsPointer(wp) )
    {
        result = (WPARAM)c->PointerValue((RexxPointerObject)wp);
        goto done_out;
    }

    uint64_t number;
    if ( rxStr2Number(c, c->ObjectToStringValue(wp), &number, argPos) )
    {
        result = (WPARAM)number;
        goto done_out;
    }
    else
    {
        c->ClearCondition();
    }

    // Really, a WPARAM should be unsigned.  But, we'll try this and if it
    // works, cast it.
    intptr_t iptr;
    if ( c->Intptr(wp, &iptr) )
    {
        result = (WPARAM)iptr;
        goto done_out;
    }

    wrongArgValueException(c->threadContext, argPos, "whole number, pointer object, hex string", wp);
    success = false;

done_out:
    *wParam = result;
    return success;
}


bool oodGetLParam(RexxMethodContext *c, RexxObjectPtr lp, LPARAM *lParam, size_t argPos, bool required)
{
    LPARAM result = 0;
    bool success = true;

    if ( lp == NULLOBJECT )
    {
        if ( required )
        {
            success = false;
            missingArgException(c->threadContext, argPos);
        }
        goto done_out;
    }

    if ( c->Intptr(lp, (intptr_t *)&result) )
    {
        goto done_out;
    }

    if ( c->IsPointer(lp) )
    {
        result = (LPARAM)c->PointerValue((RexxPointerObject)lp);
        goto done_out;
    }

    // Really a LPARAM should be signed.  But with casting, who knows how the
    // Rexx number may have ended up being represented.  So, we go ahead and try
    // these forms
    uintptr_t u;
    if ( c->Uintptr(lp, &u) )
    {
        result = (LPARAM)u;
        goto done_out;
    }

    uint64_t number;
    if ( rxStr2Number(c, c->ObjectToStringValue(lp), &number, argPos) )
    {
        result = (LPARAM)number;
        goto done_out;
    }
    else
    {
        c->ClearCondition();
    }

    wrongArgValueException(c->threadContext, argPos, "whole number, pointer object, hex string", lp);
    success = false;

done_out:
    *lParam = result;
    return success;
}

/**
 * Converts a Rexx object to a pointer.  Anything other than a .Pointer or a
 * pointer string will result in a null pointer.
 *
 * @param c    Rexx method context we are operating in.
 * @param obj  The object to convert.
 *
 * @return  The object converted to a pointer
 *
 * @remarks  Nothing is considered an error here.
 */
void *oodObj2pointer(RexxMethodContext *c, RexxObjectPtr obj)
{
    void *p = NULL;

    if ( obj != NULLOBJECT )
    {
        if ( c->IsPointer(obj) )
        {
            p = c->PointerValue((RexxPointerObject)obj);
        }
        else
        {
            p = string2pointer(c->ObjectToStringValue(obj));
        }
    }
    return p;
}

/**
 * Converts a Rexx object into a pointer, only if it is in a valid format for a
 * handle (really a pointer.)  Raises an exception if the object is not in a
 * valid format.
 *
 * For ooDialog, currently, a valid format is: a .Pointer object, a pointer
 * string, or exactly 0.
 *
 * @param c       Rexx method context we are operating in.
 * @param obj     The object to convert.
 * @param result  The converted pointer is returned here.
 * @param argPos  Argument postion.
 *
 * @return True on success, false if an exception has been raised.
 *
 * @remarks  Not implemented but, it could be that using -1 for argPos could
 *           generate a different condition message.
 */
bool oodObj2handle(RexxMethodContext *c, RexxObjectPtr obj, void **result, size_t argPos)
{
    if ( obj == NULLOBJECT )
    {
        goto raise_condition;
    }

    if ( c->IsPointer(obj) )
    {
        *result = c->PointerValue((RexxPointerObject)obj);
    }
    else
    {
        CSTRING str = c->ObjectToStringValue(obj);
        size_t len = strlen(str);

        if ( (len == 0 || len == 2) || (len == 1 && *str != '0') || toupper(str[1]) != 'X' )
        {
            goto raise_condition;
        }

        *result = string2pointer(str);
    }

    return true;

raise_condition:
    userDefinedMsgException(c, argPos, "is not a handle");
    return false;
}

int32_t idError(RexxMethodContext *c, RexxObjectPtr rxID)
{
    char buf[256];
    _snprintf(buf, sizeof(buf),
              "Error trying to add a dialog resource:\n\n%s is an undefined, non-numeric,\nidentification number.",
              c->ObjectToStringValue(rxID));

    internalErrorMsgBox(buf, OOD_RESOURCE_ERR_TITLE);
    return -1;
}

/**
 * Checks that a resource ID, which may be a symbolic ID, can be resolved
 * successfully, and returns the numeric ID.  If, it can not be resolved, an
 * error message box is put up.
 *
 * This is the implementation for ResourceUtils::checkID() and *must* resolve
 * IDC_STATIC correctly.  Which it does, by returning -1 and not generating an
 * error.
 *
 * @param c     Method context we are operating in.
 * @param rxID  Rexx object to be resolved, may be, and often is, a symbolic
 *              resource ID.
 * @param self  The Rexx object that has inherited ResourceUtils.
 *
 * @return The numeric resource ID value.  -1 is a valid return for IDC_STATIC,
 *         less than -1 means an error dialog was put up.
 */
int32_t checkID(RexxMethodContext *c, RexxObjectPtr rxID, RexxObjectPtr self)
{
    uint32_t id;
    if ( ! oodSafeResolveID(&id, c, self, rxID, -1, 1) )
    {
        return idError(c, rxID);
    }
    return (int)id;
}

int32_t resolveResourceID(RexxMethodContext *c, RexxObjectPtr rxID, RexxObjectPtr self)
{
    uint32_t id = (uint32_t)-1;
    oodSafeResolveID(&id, c, self, rxID, -1, 1);
    return (int)id;
}

/**
 *  Resolves a resource ID used for the application icon to its numeric value.
 *
 *  This is the implementation for ResourceUtils::resolveIconID() which is
 *  invoked to resolve the application icon IDs.  These IDs are special cased:
 *
 *  1.) -1, (a symbolic ID did not resolve) is changed to 0.  A 0 for the ID
 *  tells the underlying code to use the default application icon.
 *
 *  2.) IDs from 1 to 4 have 10 added to them.  When the ability to use an
 *  application icon was first added to ooDialog, the internal ooDialog icon
 *  resources, which are available for the programmer to use, were documented as
 *  having IDs of 1 through 4.  Only the symbolic ID should have been
 *  documented, so that actual numeric ID can be changed without breaking any
 *  existing code. The actual resource IDs have been changed to 11 through 14
 *  and we need to adjust for that.
 *
 * @param c         Method context we are operating in.
 * @param rxIconID  Resource ID, which may be symbolic
 * @param self      The self object of the method context.
 *
 * @return int32_t
 */
int32_t resolveIconID(RexxMethodContext *c, RexxObjectPtr rxIconID, RexxObjectPtr self)
{
    uint32_t id = (uint32_t)-1;
    oodSafeResolveID(&id, c, self, rxIconID, -1, 1);

    if ( (int)id == -1 )
    {
        id = 0;
    }
    else if ( id >= 1 && id <= 4 )
    {
        id += 10;
    }

    return (int)id;
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
 *
 *           This function should go away when ooDialog is converted to use
 *           .Pointer for all pointer-sized data types.
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

/**
 * Formats an unsigned 32 bit number in 'hex' format and returns it as a Rexx
 * string object.
 */
RexxStringObject dword2string(RexxMethodContext *c, uint32_t num)
{
    char buf[32];
    sprintf(buf, "0x%08x", num);
    return c->String(buf);
}

/**
 * Returns an upper-cased copy of the string.
 *
 * @param str   The string to copy and upper case.
 *
 * @return      A pointer to a new string, or null on a memory allocation
 *              failure.
 *
 * The caller is responsible for freeing the returned string.
 */
char *strdupupr(const char *str)
{
    char *retStr = NULL;
    if ( str )
    {
        size_t l = strlen(str);
        retStr = (char *)malloc(l + 1);
        if ( retStr )
        {
            char *p;
            for ( p = retStr; *str; ++str )
            {
                if ( ('a' <= *str) && (*str <= 'z') )
                {
                    *p++ = *str - ('a' - 'A');
                }
                else
                {
                    *p++ = *str;
                }
            }
            *p = '\0';
        }
    }
    return retStr;
}

/**
 * Returns a copy of the string with all space removed.
 *
 * @param str   The string to copy and remove spaces.
 *
 * @return      A pointer to a new string, or null on a memory allocation
 *              failure.
 *
 * @note        The caller is responsible for freeing the returned string.
 */
char *strdup_nospace(const char *str)
{
    char *retStr = NULL;
    if ( str )
    {
        size_t l = strlen(str);
        retStr = (char *)malloc(l + 1);
        if ( retStr )
        {
            char *p;
            for ( p = retStr; *str; ++str )
            {
                if ( *str == ' ' || *str == '\t' )
                {
                    continue;
                }
                *p++ = *str;
            }
            *p = '\0';
        }
    }
    return retStr;
}

/**
 * Returns an upper-cased copy of the string with all space removed.
 *
 * @param str   The string to copy and upper case.
 *
 * @return      A pointer to a new string, or null on a memory allocation
 *              failure.
 *
 * @note        The caller is responsible for freeing the returned string.
 */
char *strdupupr_nospace(const char *str)
{
    char *retStr = NULL;
    if ( str )
    {
        size_t l = strlen(str);
        retStr = (char *)malloc(l + 1);
        if ( retStr )
        {
            char *p;
            for ( p = retStr; *str; ++str )
            {
                if ( *str == ' ' )
                {
                    continue;
                }
                if ( ('a' <= *str) && (*str <= 'z') )
                {
                    *p++ = *str - ('a' - 'A');
                }
                else
                {
                    *p++ = *str;
                }
            }
            *p = '\0';
        }
    }
    return retStr;
}

/**
 * Returns a copy of the string that is suitable for an ooRexx method name. All
 * space, tab, ampersand, ':', and '+' characters are removed.  In addition it
 * removes any trailing ... from the string.  Upper-casing the characters is
 * skipped, because this should not matter.
 *
 * @param str   The string to copy and make into a method name.
 *
 * @return      A pointer to a new string, or null on a memory allocation
 *              failure.
 *
 * @note        The caller is responsible for freeing the returned string.
 */
char *strdup_2methodName(const char *str)
{
    char *retStr = NULL;
    if ( str )
    {
        size_t l = strlen(str);
        retStr = (char *)malloc(l + 1);
        if ( retStr )
        {
            char *p;
            for ( p = retStr; *str; ++str )
            {
                if ( *str == ' '|| *str == '\t' || *str == '&' || *str == '+' || *str == ':' )
                {
                    continue;
                }
                else
                {
                    *p++ = *str;
                }
            }
            *(p - 3) == '.' ? *(p - 3) = '\0' : *p = '\0';
        }
    }
    return retStr;
}


/**
 * Helper function to check for a modal dialog creation.  Only called when a
 * dialog is first created.  If the dialog is to be created as a modal dialog,
 * then the 'previous' dialog, if there is one, is disabled.
 *
 * @param previous  CSelf pointer of the previously created dialog.  This could
 *                  be null.
 * @param modeless  Is the dialog to be modeless, or modal.
 *
 * @remarks.  In the original ooDialog, all dialogs were created modeless, and a
 *            modal dialog was faked by disabling the previously created dialog.
 *            And this is still done.
 *
 *            However, this involves a rather tenous chain of enabling and
 *            disabling dialogs as they are created and destroyed (deleted.)  In
 *            the process of refactoring ooDialog and updating it to the C++
 *            APIs, it may be that this tenous chain is broken.
 */
void checkModal(pCPlainBaseDialog previous, logical_t modeless)
{
    if ( ! modeless && previous != NULL )
    {
        if ( IsWindowEnabled(previous->hDlg) )
        {
            EnableWindow(previous->hDlg, FALSE);
        }
    }

}


/**
 *  Helper function to re-enable the previous dialog when dialog creation has
 *  failed.
 *
 * @param previous  CSelf pointer of the previously created dialog.  This could
 *                  be null.
 *
 * @remarks  See the remarks for checkModal().
 */
void enablePrevious(pCPlainBaseDialog previous)
{
    if ( previous )
    {
        previous->onTheTop = true;

        if ( ! IsWindowEnabled(previous->hDlg) )
        {
            EnableWindow(previous->hDlg, TRUE);
        }
    }
}


/**
 * Convenience function to retrieve the dialog CSelf struct from a generic
 * ooDialog Rexx object.
 *
 * This function is safe to call for any object, including NULLOBJECT.  It will
 * fail for any object that is not a dialog or a dialog control object.
 *
 * @param c         Method context we are operating in.
 * @param self      The Rexx object.
 * @param selfType  The class self is required to be.  Can be oodUnknown to
 *                  indicate it doesn't matter if self is a PlainBaseDialog or a
 *                  DialogControl.
 * @param argPos    The argument position of self.  If this is not 0, a wrong
 *                  argument exception is raised.  If it is 0, then a base class
 *                  initialization is raised.
 *
 * @return A pointer to the dialog CSelf on success, or NULL on failure.
 *
 * @note  An exception is always raised on failure.
 */
pCPlainBaseDialog requiredDlgCSelf(RexxMethodContext *c, RexxObjectPtr self, oodClass_t type, size_t argPos)
{
    pCPlainBaseDialog pcpbd = NULL;

    if ( self != NULLOBJECT )
    {
        if ( type == oodPlainBaseDialog )
        {
            if ( c->IsOfType(self, "PLAINBASEDIALOG") )
            {
                pcpbd = dlgToCSelf(c, self);
            }
            else if ( argPos > 0)
            {
                wrongClassException(c->threadContext, argPos, "PlainBaseDialog");
                return NULLOBJECT;
            }
        }
        else if ( type == oodDialogControl )
        {
            if ( c->IsOfType(self, "DIALOGCONTROL") )
            {
                pCDialogControl pcdc = controlToCSelf(c, self);
                if ( pcdc != NULLOBJECT )
                {
                    pcpbd = dlgToCSelf(c, pcdc->oDlg);
                }
            }
            else if ( argPos > 0)
            {
                wrongClassException(c->threadContext, argPos, "DialogControl");
                return NULLOBJECT;
            }
        }
        else
        {
            if ( c->IsOfType(self, "PLAINBASEDIALOG") )
            {
                pcpbd = dlgToCSelf(c, self);
            }
            else if ( c->IsOfType(self, "DIALOGCONTROL") )
            {
                pCDialogControl pcdc = controlToCSelf(c, self);
                if ( pcdc != NULLOBJECT )
                {
                    pcpbd = dlgToCSelf(c, pcdc->oDlg);
                }
            }
            else if ( argPos > 0 )
            {
                // If an arg position is specified we raise a wrong arg
                // exception, otherwise we will drop through and raise a base
                // class intialization exception.
                wrongArgValueException(c->threadContext, argPos, "PlainBaseDialog or DialogControl", self);
                return NULLOBJECT;
            }
        }
    }

    if ( pcpbd == NULLOBJECT )
    {
        baseClassIntializationException(c);
    }
    return pcpbd;
}


PPOINT rxGetPoint(RexxMethodContext *context, RexxObjectPtr p, int argPos)
{
    if ( requiredClass(context->threadContext, p, "Point", argPos) )
    {
        return (PPOINT)context->ObjectToCSelf(p);
    }
    return NULL;
}


RexxObjectPtr rxNewPoint(RexxMethodContext *c, long x, long y)
{
    RexxObjectPtr point = NULL;
    RexxClassObject PointClass = rxGetContextClass(c, "POINT");
    if ( PointClass != NULL )
    {
        point = c->SendMessage2(PointClass, "NEW", c->WholeNumber(x), c->WholeNumber(y));
    }
    return point;
}


PRECT rxGetRect(RexxMethodContext *context, RexxObjectPtr r, int argPos)
{
    if ( requiredClass(context->threadContext, r, "Rect", argPos) )
    {
        return (PRECT)context->ObjectToCSelf(r);
    }
    return NULL;
}


RexxObjectPtr rxNewRect(RexxMethodContext *context, long l, long t, long r, long b)
{
    RexxObjectPtr rect = NULL;

    RexxClassObject RectClass = rxGetContextClass(context, "RECT");
    if ( RectClass != NULL )
    {
        RexxArrayObject args = context->NewArray(4);
        context->ArrayAppend(args, context->WholeNumber(l));
        context->ArrayAppend(args, context->WholeNumber(t));
        context->ArrayAppend(args, context->WholeNumber(r));
        context->ArrayAppend(args, context->WholeNumber(b));

        rect = context->SendMessage(RectClass, "NEW", args);
    }
    return rect;
}


RexxObjectPtr rxNewRect(RexxMethodContext *context, PRECT r)
{
    RexxObjectPtr rect = NULL;

    RexxClassObject RectClass = rxGetContextClass(context, "RECT");
    if ( RectClass != NULL )
    {
        RexxArrayObject args = context->NewArray(4);
        context->ArrayAppend(args, context->WholeNumber(r->left));
        context->ArrayAppend(args, context->WholeNumber(r->top));
        context->ArrayAppend(args, context->WholeNumber(r->right));
        context->ArrayAppend(args, context->WholeNumber(r->bottom));

        rect = context->SendMessage(RectClass, "NEW", args);
    }
    return rect;
}


PSIZE rxGetSize(RexxMethodContext *context, RexxObjectPtr s, int argPos)
{
    if ( requiredClass(context->threadContext, s, "Size", argPos) )
    {
        return (PSIZE)context->ObjectToCSelf(s);
    }
    return NULL;
}


RexxObjectPtr rxNewSize(RexxThreadContext *c, long cx, long cy)
{
    return c->SendMessage2(TheSizeClass, "NEW", c->WholeNumber(cx), c->WholeNumber(cy));
}

RexxObjectPtr rxNewSize(RexxMethodContext *c, long cx, long cy)
{
    return rxNewSize(c->threadContext, cx, cy);
}

bool rxGetWindowText(RexxMethodContext *c, HWND hwnd, RexxStringObject *pStringObj)
{
    oodResetSysErrCode(c->threadContext);

    uint32_t count = (uint32_t)GetWindowTextLength(hwnd);
    if ( count == 0 )
    {
        oodSetSysErrCode(c->threadContext);
        *pStringObj = c->NullString();
        return true;
    }

    // TODO For all windows except a multiline edit control this is fine.  We
    // should check the count and if bigger than a certain size, see if it could
    // be optimized by using a string buffer.

    LPTSTR pBuf = (LPTSTR)malloc(++count);
    if ( pBuf == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    count = GetWindowText(hwnd, pBuf, count);
    if ( count != 0 )
    {
        *pStringObj = c->String(pBuf);
    }
    else
    {
        oodSetSysErrCode(c->threadContext);
        *pStringObj = c->NullString();
    }
    free(pBuf);

    return true;
}


bool rxDirectoryFromArray(RexxMethodContext *c, RexxArrayObject a, size_t index, RexxDirectoryObject *d, size_t argPos)
{
    RexxObjectPtr _d = c->ArrayAt(a, index);

    if ( _d == NULLOBJECT )
    {
        sparseArrayException(c->threadContext, argPos, index);
        goto err_out;
    }
    if ( ! c->IsOfType(_d, "DIRECTORY") )
    {
        wrongObjInArrayException(c->threadContext, argPos, index, "Directory");
        goto err_out;
    }

    *d = (RexxDirectoryObject)_d;
    return true;

err_out:
    return false;
}


bool rxLogicalFromDirectory(RexxMethodContext *context, RexxDirectoryObject d, CSTRING index,
                            BOOL *logical, int argPos, bool required)
{
    logical_t value;
    RexxObjectPtr obj = context->DirectoryAt(d, index);

    if ( required && obj == NULLOBJECT )
    {
        missingIndexInDirectoryException(context->threadContext, argPos, index);
        return false;
    }

    if ( obj != NULLOBJECT )
    {
        if ( ! context->Logical(obj, &value) )
        {
            wrongObjInDirectoryException(context->threadContext, argPos, index, "a logical", obj);
            return false;
        }
        *logical = (BOOL)value;
    }
    return true;
}

bool rxNumberFromDirectory(RexxMethodContext *context, RexxDirectoryObject d, CSTRING index,
                           uint32_t *number, int argPos, bool required)
{
    uint32_t value;
    RexxObjectPtr obj = context->DirectoryAt(d, index);

    if ( required && obj == NULLOBJECT )
    {
        missingIndexInDirectoryException(context->threadContext, argPos, index);
        return false;
    }

    if ( obj != NULLOBJECT )
    {
        if ( ! context->UnsignedInt32(obj, &value) )
        {
            wrongObjInDirectoryException(context->threadContext, argPos, index, "a non-negative whole number", obj);
            return false;
        }
        *number = value;
    }
    return true;
}

bool rxIntFromDirectory(RexxMethodContext *context, RexxDirectoryObject d, CSTRING index,
                        int *number, int argPos, bool required)
{
    int value;
    RexxObjectPtr obj = context->DirectoryAt(d, index);

    if ( required && obj == NULLOBJECT )
    {
        missingIndexInDirectoryException(context->threadContext, argPos, index);
        return false;
    }

    if ( obj != NULLOBJECT )
    {
        if ( ! context->Int32(obj, &value) )
        {
            wrongObjInDirectoryException(context->threadContext, argPos, index, "an integer", obj);
            return false;
        }
        *number = value;
    }
    return true;
}


/**
 * Look up the int value of a keyword.
 *
 *
 * @param cMap  The string to int map to use for the look up.
 * @param str   The keyword to map.
 *
 * @return Return the value for the keyword, or -1 for not found.
 */
int getKeywordValue(String2Int *cMap, const char * str)
{
    String2Int::iterator itr;
    itr = cMap->find(str);
    if ( itr != cMap->end() )
    {
        return itr->second;
    }
    return -1;
}


/**
 * Sets a window style and returns the old window style, with checks for error.
 *
 * @param c       The method context we are operating in.
 * @param hwnd    Handle of window having its style changed / set.
 * @param style   The style to be set.
 *
 * @return The old style on success, otherwise a number less than 0.
 *
 * @remarks  Prior to the introduction of the C++ native API, it was difficult
 *           to convey system errors back to the Rexx programmer on failure.
 *           One convention was to return the negated system error code.  With
 *           the C++ API, it is much better to return negative one and set
 *           .SystemErrorCode.
 *
 *           However, some of the methods using this function were already
 *           documented as returning the negated system error code.  So, this
 *           function does both.
 */
RexxObjectPtr setWindowStyle(RexxMethodContext *c, HWND hwnd, uint32_t style)
{
    oodResetSysErrCode(c->threadContext);
    SetLastError(0);

    RexxObjectPtr result;
    style = SetWindowLong(hwnd, GWL_STYLE, style);

    /* SetWindowLong returns 0 on error, or the value of the previous long at
     * the specified index.  Very unlikely that the last style was 0, but assume
     * it is possible.  In that case, 0 is only an error if GetLastError does
     * not return 0.
     */
    if ( style == 0 )
    {
        result = TheZeroObj;
        uint32_t rc = GetLastError();
        if ( rc != 0 )
        {
            oodSetSysErrCode(c->threadContext, rc);
            result = c->WholeNumber(-(wholenumber_t)rc);
        }
    }
    else
    {
        result = c->UnsignedInt32(style);
    }
    return result;
}


/**
 *  Converts an ANSI character string to a wide (Unicode) character string and
 *  puts it in the specified buffer.
 *
 *  This is a convenience function that assumes the caller has passed a buffer
 *  known to be big enough.
 *
 *  It works correctly for the empty string "" and is designed to treat a null
 *  pointer for text as the empty string.  For both cases, the wide character
 *  null is copied to the destination buffer and a count of 1 is returned.
 *
 * @param dest  Buffer in which to place the converted string.  Must be big
 *              enough.
 * @param text  The text to convert.  As explained above, this can be a null
 *              pointer in which case it is treated as though it is the empty
 *              string.
 *
 * @return The number of wide character values copied to the buffer.  This will
 *         always be at least one, if an error occurs, the wide character null
 *         is copied to the destination and 1 is returned.
 */
int putUnicodeText(LPWORD dest, const char *text)
{
    int count = 1;
    if ( text == NULL )
    {
        *dest = 0;
    }
    else
    {
        int cchWideChar = (int)strlen(text) + 1;

        count = MultiByteToWideChar(CP_ACP, 0, text, -1, (LPWSTR)dest, cchWideChar);
        if ( count == 0 )
        {
            // Unlikely that this failed, but if it did, treat it as an empty
            // string.
            *dest = 0;
            count++;
        }
    }
    return count;
}


/**
 *
 *
 *
 * @param wstr
 * @param len     The length, including the terminating null, of the wide string
 *                to convert.  If this length does not include the terminating
 *                null, the returned string will not include a terminating
 *                string.
 *
 *                If -1 is passed for this parameter, the length will be
 *                calculated and assumed the terminating null is desired.
 *
 * @return The converted string, or null on error.
 *
 * @note  The caller is responsible for freeing the returned string.  Memory is
 *        allocated using malloc.
 */
char *unicode2Ansi(PWSTR wstr, int32_t len)
{
    if (wstr == NULL)
    {
        return NULL;
    }

    if ( len == -1 )
    {
        len = (int)wcslen(wstr) + 1;
    }

    char *ansiStr = NULL;
    int32_t neededLen = WideCharToMultiByte(CP_ACP, 0, wstr, len, NULL, 0, NULL, NULL);

    if ( neededLen != 0 )
    {
        ansiStr = (char *)malloc(neededLen);
        if ( ansiStr != NULL )
        {
            if ( WideCharToMultiByte(CP_ACP, 0, wstr, len, ansiStr, neededLen, NULL, NULL) == 0 )
            {
                /* conversion failed */
                free(ansiStr);
                ansiStr = NULL;
            }
        }
    }

    return ansiStr;
}

/**
 * Converts a wide character (Unicode) string to a Rexx string object.
 *
 *
 * @param c
 * @param wstr
 * @param len
 *
 * @return RexxStringObject
 *
 * @remarks  TODO  should an out of memeory exception be raised for a null
 *           return from unicode2Ansi?
 */
RexxStringObject unicode2String(RexxMethodContext *c, PWSTR wstr, int32_t len)
{
    RexxStringObject result = c->NullString();
    if ( wstr == NULL )
    {
        goto done_out;
    }

    char *str = unicode2Ansi(wstr, len);
    if ( str == NULL )
    {
        goto done_out;
    }

    result = c->String(str);
    free(str);

done_out:
    return result;
}


/**
 * Checks that an argument array contains the minimum, and not more than the
 * maximum, number of arguments.  Raises the appropriate exception if the check
 * fails.
 *
 * In addition the actual size of the argument array is returned.
 *
 * @param c         Method context we are operating in.
 * @param args      Argument array.
 * @param min       Minimum expected number of arguments.
 * @param max       Maximum number of arguments allowed.
 * @param arraySize [out] Size of argument array.
 *
 * @return True if the check succeeds, otherwise false.  If false, an exception
 *         has been raised.
 */
bool goodMinMaxArgs(RexxMethodContext *c, RexxArrayObject args, size_t min, size_t max, size_t *arraySize)
{
    *arraySize = c->ArraySize(args);
    if ( *arraySize > max )
    {
        tooManyArgsException(c->threadContext, max);
        return false;
    }
    if ( *arraySize < min )
    {
        missingArgException(c->threadContext, min);
        return false;
    }
    return true;
}

/**
 * Fills in a RECT structure using an argument array passed to a Rexx object
 * method.
 *
 * The purpose is to give the Rexx programmer some flexibility in how they pass
 * in "rectangle-like" coordinates to a method.
 *
 * The coordinates can be expressed as a .Rect, as a .Point and a .Size, as a
 * .Point and a .Point, or as 4 individual intergers.
 *
 * There are also two basic scenarios where this is used:
 *
 * 1.) A bounding rectangle: x1, y1, x2, y2  With a bounding rectangle x1 and y1
 * specify the upper-left corner of the rectangle. x2 and y2 specify the
 * lower-bottom corner of the rectangle.  In this scenario, the Rexx programmer
 * can use .Point .Point, but not .Point .Size.
 *
 * 2.) A point / size rectangle.  In this case the first two args specify the
 * upper-left corner of the rectangle, and the third and forth args specify the
 * width and height of the rectangle.  In this scenario, the Rexx programmer can
 * use .Point and .Size, but not .Point and .Point
 *
 * In either case, .Rect and 4 indvidual integers are taken at face value.
 *
 * @param c            Method context we are operating in.
 * @param args         The arg list array (ARGLIST) passed to the native API
 * @param rect         [IN/OUT] Pointer to a rect struct, this is filled in on
 *                     success.
 * @param boundingRect True if rect should be interpreted as a bounding
 *                     rectangle, false if rect should be interpreted as a
 *                     point/size rectangle.
 * @param startArg     The argument number in the arg array where the rectangle
 *                     specifications start.
 * @param maxArgs      The maximum number of args allowed.
 * @param arraySize    [IN/OUT] The size of the argument array, returned.
 * @param usedArgs     [IN/OUT] The number of arguments used in specifying the
 *                     rectangle. I.e., if startArg is a .Rect, then usedArgs
 *                     will be 1 on return.  If at startArg we have x, y, cx, cy
 *                     then useArgs will be 4 on return.
 *
 * @return True on success, false otherwise.  If the return is false, an
 *         exception has been raised.
 */
bool getRectFromArglist(RexxMethodContext *c, RexxArrayObject args, PRECT rect, bool boundingRect,
                        int startArg, int maxArgs, size_t *arraySize, size_t *usedArgs)
{
    if ( ! goodMinMaxArgs(c, args, startArg, maxArgs, arraySize) )
    {
        goto err_out;
    }

    RexxObjectPtr obj1 = c->ArrayAt(args, startArg);
    if ( obj1 == NULLOBJECT )
    {
        missingArgException(c->threadContext, startArg);
        goto err_out;
    }

    if ( c->IsOfType(obj1, "RECT") )
    {
        PRECT r = rxGetRect(c, obj1, startArg);
        if ( r == NULL )
        {
            goto err_out;
        }
        CopyRect(rect, r);
        *usedArgs = 1;
    }
    else if ( c->IsOfType(obj1, "POINT") )
    {
        PPOINT p = rxGetPoint(c, obj1, startArg);
        if ( p == NULL )
        {
            goto err_out;
        }

        RexxObjectPtr obj2 = c->ArrayAt(args, startArg + 1);
        if ( obj2 == NULLOBJECT )
        {
            missingArgException(c->threadContext, startArg + 1);
            goto err_out;
        }

        // If it is a bounding rectangle, the second object has to be a .Point
        // object. Otherwise, the second object has to be a .Size object
        if ( boundingRect )
        {
            PPOINT p2 = rxGetPoint(c, obj2, startArg + 1);
            if ( p2 == NULL )
            {
                goto err_out;
            }
            SetRect(rect, p->x, p->y, p2->x, p2->y);
        }
        else
        {
            PSIZE s = rxGetSize(c, obj2, startArg + 1);
            if ( s == NULL )
            {
                goto err_out;
            }
            SetRect(rect, p->x, p->y, s->cx, s->cy);
        }
        *usedArgs = 2;
    }
    else
    {
        int x, y, cx, cy;

        if ( ! c->Int32(obj1, &x) )
        {
            wrongRangeException(c->threadContext, startArg, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }

        obj1 = c->ArrayAt(args, startArg + 1);
        if ( obj1 == NULLOBJECT )
        {
            missingArgException(c->threadContext, startArg + 1);
            goto err_out;
        }
        if ( ! c->Int32(obj1, &y) )
        {
            wrongRangeException(c->threadContext, startArg + 1, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }

        obj1 = c->ArrayAt(args, startArg + 2);
        if ( obj1 == NULLOBJECT )
        {
            missingArgException(c->threadContext, startArg + 2);
            goto err_out;
        }
        if ( ! c->Int32(obj1, &cx) )
        {
            wrongRangeException(c->threadContext, startArg + 2, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }

        obj1 = c->ArrayAt(args, startArg + 3);
        if ( obj1 == NULLOBJECT )
        {
            missingArgException(c->threadContext, startArg + 3);
            goto err_out;
        }
        if ( ! c->Int32(obj1, &cy) )
        {
            wrongRangeException(c->threadContext, startArg + 3, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }
        SetRect(rect, x, y, cx, cy);
        *usedArgs = 4;
    }
    return true;

err_out:
    return false;
}

bool getPointFromArglist(RexxMethodContext *c, RexxArrayObject args, PPOINT point, int startArg, int maxArgs,
                         size_t *arraySize, size_t *usedArgs)
{
    if ( ! goodMinMaxArgs(c, args, startArg, maxArgs, arraySize) )
    {
        goto err_out;
    }

    RexxObjectPtr obj1 = c->ArrayAt(args, startArg);
    if ( obj1 == NULLOBJECT )
    {
        missingArgException(c->threadContext, startArg);
        goto err_out;
    }

    if ( c->IsOfType(obj1, "POINT") )
    {
        PPOINT p = rxGetPoint(c, obj1, startArg);
        if ( p == NULL )
        {
            goto err_out;
        }
        point->x = p->x;
        point->y = p->y;
        *usedArgs = 1;
    }
    else if ( c->IsOfType(obj1, "SIZE") )
    {
        PSIZE s = rxGetSize(c, obj1, startArg);
        if ( s == NULL )
        {
            goto err_out;
        }
        point->x = s->cx;
        point->y = s->cy;
        *usedArgs = 1;
    }
    else
    {
        int x, y;
        if ( ! c->Int32(obj1, &x) )
        {
            wrongRangeException(c->threadContext, startArg, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }

        obj1 = c->ArrayAt(args, startArg + 1);
        if ( obj1 == NULLOBJECT )
        {
            missingArgException(c->threadContext, startArg + 1);
            goto err_out;
        }
        if ( ! c->Int32(obj1, &y) )
        {
            wrongRangeException(c->threadContext, startArg + 1, INT32_MIN, INT32_MAX, obj1);
            goto err_out;
        }
        point->x = x;
        point->y = y;
        *usedArgs = 2;
    }
    return true;

err_out:
    return false;
}

