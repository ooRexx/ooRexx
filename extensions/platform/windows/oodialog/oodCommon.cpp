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

//

/**
 *  93.900
 *  Error 93 - Incorrect call to method
 *        The specified method, built-in function, or external routine exists,
 *        but you used it incorrectly.
 *
 *  The connectEdit method can not be invoked on a StyleDlg when the Windows
 *  dialog does not exist.
 *
 * @param c
 * @param pcpbd
 */
void noWindowsDialogException(RexxMethodContext *c, pCPlainBaseDialog pcpbd)
{
    TCHAR buf[512];
    _snprintf(buf, sizeof(buf), "The %s method can not be invoked on %s when the Windows dialog does not exist.",
              c->GetMessageName(), c->ObjectToStringValue(pcpbd->rexxSelf));
    c->RaiseException1(Rexx_Error_Incorrect_method_user_defined, c->String(buf));
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


inline void failedToRetrieveDlgAdmException(RexxThreadContext *c, RexxObjectPtr source)
{
    failedToRetrieveException(c, "dialog administration block", source);
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
                              int argPosObj, int argPosID)
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
                   int argPosObj, int argPosID)
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

bool dialogInAdminTable(DIALOGADMIN * Dlg)
{
    register int i;
    for ( i = 0; i < StoredDialogs; i++ )
    {
        if ( DialogTab[i] == Dlg )
        {
           return true;
        }
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

LONG HandleError(PRXSTRING r, CHAR * text)
{
      HWND hW = NULL;
      if ((topDlg) && (topDlg->TheDlg)) hW = topDlg->TheDlg;
      MessageBox(hW,text,"Error",MB_OK | MB_ICONHAND);
      r->strlength = 2;
      r->strptr[0] = '4';
      r->strptr[1] = '0';
      r->strptr[2] = '\0';
      return 40;
}

void rxstrlcpy(CHAR * tar, CONSTRXSTRING &src)
{
   register UINT i;
   for (i=0; (i<src.strlength) && (i<STR_BUFFER-1);i++) tar[i] = src.strptr[i];
   tar[i] = '\0';
}

void rxdatacpy(CHAR * tar, RXSTRING &src)
{
   register UINT i;
   for (i=0; (i<src.strlength) && (i<DATA_BUFFER-1);i++) tar[i] = src.strptr[i];
   tar[i] = '\0';
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

DIALOGADMIN *rxGetDlgAdm(RexxMethodContext *context, RexxObjectPtr dlg)
{
    DIALOGADMIN *adm = (DIALOGADMIN *)rxGetPointerAttribute(context, dlg, "ADM");
    if ( adm == NULL )
    {
        failedToRetrieveDlgAdmException(context->threadContext, dlg);
    }
    return adm;
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


PSIZE rxGetSize(RexxMethodContext *context, RexxObjectPtr s, int argPos)
{
    if ( requiredClass(context->threadContext, s, "Size", argPos) )
    {
        return (PSIZE)context->ObjectToCSelf(s);
    }
    return NULL;
}


RexxObjectPtr rxNewSize(RexxMethodContext *c, long cx, long cy)
{
    RexxObjectPtr size = NULL;
    RexxClassObject SizeClass = rxGetContextClass(c, "SIZE");
    if ( SizeClass != NULL )
    {
        size = c->SendMessage2(SizeClass, "NEW", c->WholeNumber(cx), c->WholeNumber(cy));
    }
    return size;
}


// TODO move to APICommon when ooDialog is converted to use .Pointer instead of
// pointer strings.
//
// NOTE: this function won't crash, but it can easily return null.
//
// NOTE 2: be sure to return the raw pointer, not .Pointer, that way calling do
// not need to unwrap things.
POINTER rxGetPointerAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name)
{
    CSTRING value = "";
    if ( obj != NULLOBJECT )
    {
        RexxObjectPtr rxString = context->SendMessage0(obj, name);
        if ( rxString != NULLOBJECT )
        {
            value = context->ObjectToStringValue(rxString);
        }
    }
    return string2pointer(value);
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

    // TODO For all windows except an edit control this is fine.  We should
    // check the count and if bigger than a certain size, see if it could be
    // optimized by using a string buffer.

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
 *  Converts an ANSI character string to a wide (Unicode) character string.
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
int addUnicodeText(LPWORD dest, const char *text)
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

