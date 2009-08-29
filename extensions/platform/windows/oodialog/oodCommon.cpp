/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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

#include "ooDialog.h"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include <CommCtrl.h>
#include "APICommon.h"
#include "oodCommon.h"

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

    if ( argPosObj != -1 && ! requiredClass(context, oodObj, "ResourceUtils", argPosObj) )
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
                outOfMemoryException(context);
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
        wrongArgValueException(context, argPosID, "a valid numeric ID or a valid symbolic ID" , id);
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


DWORD oodGetSysErrCode(RexxMethodContext *c)
{
    uint32_t code = 0;
    RexxObjectPtr rxCode = c->DirectoryAt(TheDotLocalObj, "SYSTEMERRORCODE");
    c->UnsignedInt32(rxCode, &code);
    return (DWORD)code;
}

void oodSetSysErrCode(RexxMethodContext *context, DWORD code)
{
    context->DirectoryPut(TheDotLocalObj, context->UnsignedInt32(code), "SYSTEMERRORCODE");
}


void oodResetSysErrCode(RexxMethodContext *context)
{
    context->DirectoryPut(TheDotLocalObj, TheZeroObj, "SYSTEMERRORCODE");
}

BOOL DialogInAdminTable(DIALOGADMIN * Dlg)
{
    register INT i;
    for ( i = 0; i < StoredDialogs; i++ )
    {
        if ( DialogTab[i] == Dlg )
        {
           break;
        }
    }
    return(i < StoredDialogs);
}

/**
 * Converts a string in hexadecimal format (starts with 0x) to its pointer-sized
 * value.
 *
 * Note that this converts "0" to null, which is what we want.
 *
 * @param string  The string to convert.
 *
 * @return The converted value, which could be null to begin with, or null if it
 *         is not converted.
 */
void *string2pointer(const char *string)
{
    void *pointer = NULL;
    if ( strlen(string) > 1 )
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

/* Slightly stricter than isYes. TODO remove this when YesNoMessage() is
   fixed. */
bool IsNo(const char * s)
{
   return ( s && (*s == 'N' || *s == 'n') );
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
 * space, tab, ampersand, and '+' characters are removed.  In addition it
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
                if ( *str == ' '|| *str == '\t' || *str == '&' || *str == '+' )
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
        failedToRetrieveException(context, "dialog administration block", dlg);
    }
    return adm;
}


PPOINT rxGetPoint(RexxMethodContext *context, RexxObjectPtr p, int argPos)
{
    if ( requiredClass(context, p, "Point", argPos) )
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
    if ( requiredClass(context, r, "Rect", argPos) )
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
    if ( requiredClass(context, s, "Size", argPos) )
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


static HFONT createFontFromName(HDC hdc, CSTRING name, uint32_t size)
{
    LOGFONT lf={0};

    strcpy(lf.lfFaceName, name);
    lf.lfHeight = -MulDiv(size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    return CreateFontIndirect(&lf);
}

/**
 * Correctly converts from a device coordinate (pixel) to a dialog unit
 * coordinate, for any dialog.
 *
 * MapDialogRect() correctly converts from dialog units to pixels for any
 * dialog.  But, there is no conversion the other way, from pixels to dialog
 * units.
 *
 * MSDN gives these formulas to convert from pixel to dialog unit:
 *
 * templateunitX = MulDiv(pixelX, 4, baseUnitX);
 * templateunitY = MulDiv(pixelY, 8, baseUnitY);
 *
 * Now, you just need to get the correct dialog base unit.
 *
 * GetDialogBaseUnits() always assumes the font is the system font.  If the
 * dialog uses any other font, the base units returned will be incorrect.
 *
 * MSDN, again, has two methods for calculating the correct base units for any
 * font.  This way is the simplest, but it requires the window handle to the
 * dialog.
 *
 * Rect rect( 0, 0, 4, 8 );
 * MapDialogRect( &rc );
 * int baseUnitY = rc.bottom;
 * int baseUnitX = rc.right;
 *
 * @param hwnd   Window handle of the dialog.  If this is not a dialog window
 *               handle, this method will fail.
 *
 * @param point  Pointer to a POINT struct.  Not that a SIZE struct and a POINT
 *               struct are binary equivalents.  They both have two fields, each
 *               of which is a long.  Only the field names differ, cx and cy for
 *               a SIZE and x and y for a POINT.  Therefore you can cast a
 *               SIZE pointer to a POINT pointer.
 *
 * @return true on success, false otherwise.
 *
 * Dialog class: #32770
 */
static bool screenToDlgUnit(HWND hwnd, POINT *point)
{
    RECT r = {0, 0, 4, 8};

    if ( MapDialogRect(hwnd, &r) )
    {
        point->x = MulDiv(point->x, 4, r.right);
        point->y = MulDiv(point->y, 8, r.bottom);
        return true;
    }
    return false;
}

/**
 * Given a device context with the correct font already selected into it,
 * correctly converts from a device coordinate (pixel) to a dialog unit
 * coordinate.  The correct font means, the font actually used by the dialog.
 *
 * See screenToDlgUnit(HWND, POINT *) for a discussion of this
 * conversion.
 *
 * @param hdc    Handle to a device context with the dialog's font selected into
 *               it.
 *
 * @param point  Pointer to a POINT struct.  Not that a SIZE struct and a POINT
 *               struct are binary equivalents.  They both have two fields, each
 *               of which is a long.  Only the field names differ, cx and cy for
 *               a SIZE and x and y for a POINT.  Therefore you can cast a
 *               SIZE pointer to a POINT pointer.
 *
 * @return true on success, false otherwise.
 *
 */
static void screenToDlgUnit(HDC hdc, POINT *point)
{
    TEXTMETRIC tm;
    SIZE size;
    GetTextMetrics(hdc, &tm);
    int baseUnitY = tm.tmHeight;

    GetTextExtentPoint32(hdc, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size);
    int baseUnitX = (size.cx / 26 + 1) / 2;

    point->x = MulDiv(point->x, 4, baseUnitX);
    point->y = MulDiv(point->y, 8, baseUnitY);
}

/**
 * Uses GetTextExtentPoint32() to get the size needed for a string using the
 * specified font and device context.
 *
 * @param font   The font being used for the string.
 * @param hdc    The device context to use.
 * @param text   The string.
 * @param size   Pointer to a SIZE struct used to return the size.
 *
 * @return True if  GetTextExtentPoint32() succeeds, otherwise false.
 *
 * @note   GetTextExtentPoint32() sets last error and SelectObject() does not.
 *         Therefore if this function fails, GetLastError() will return the
 *         correct error code for the failed GetTextExtentPoint32().
 */
bool getTextExtent(HFONT font, HDC hdc, CSTRING text, SIZE *size)
{
    bool success = true;
    HFONT hOldFont = (HFONT)SelectObject(hdc, font);

    if ( GetTextExtentPoint32(hdc, text, (int)strlen(text), size) == 0 )
    {
        success = false;
    }
    SelectObject(hdc, hOldFont);
    return success;
}

bool textSizeIndirect(RexxMethodContext *context, CSTRING text, CSTRING fontName, uint32_t fontSize,
                      SIZE *size, HWND hwnd)
{
    bool success = true;

    // If hwnd is null, GetDC() returns a device context for the whole screen,
    // and that suites our purpose here.
    HDC hdc = GetDC(hwnd);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "GetDC");
        return false;
    }

    HFONT font = createFontFromName(hdc, fontName, fontSize);
    if ( font == NULL )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "CreateFontIndirect");
        ReleaseDC(hwnd, hdc);
        return false;
    }

    if ( ! getTextExtent(font, hdc, text, size) )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "GetTextExtentPoint32");
        success = false;
    }

    DeleteObject(font);
    ReleaseDC(hwnd, hdc);

    return success;
}

bool textSizeFromWindow(RexxMethodContext *context, CSTRING text, SIZE *size, HWND hwnd)
{
    HDC hdc = GetDC(hwnd);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "GetDC");
        return false;
    }

    // Dialogs and controls need to have been issued a WM_SETFONT or else they
    // return null here.  If null, they are using the stock system font.
    HFONT font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    if ( font == NULL )
    {
        font = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    bool success = true;
    if ( ! getTextExtent(font, hdc, text, size) )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "GetTextExtentPoint32");
        success = false;
    }

    ReleaseDC(hwnd, hdc);
    return success;
}


RexxObjectPtr getTextSize(RexxMethodContext *context, CSTRING text, CSTRING fontName, uint32_t fontSize,
                          HWND hwndFontSrc, RexxObjectPtr dlgObj)
{
    // hwndDlg can be null if this is happening before the real dialog is created.
    HWND hwndDlg = rxGetWindowHandle(context, dlgObj);

    // We may not have a window handle, but using null is okay.
    HWND hwndForDC = (hwndFontSrc != NULL ? hwndFontSrc : hwndDlg);

    SIZE textSize = {0};

    if ( fontName != NULL )
    {
        if ( ! textSizeIndirect(context, text, fontName, fontSize, &textSize, hwndForDC) )
        {
            goto error_out;
        }
    }
    else if ( hwndFontSrc != NULL )
    {
        if ( ! textSizeFromWindow(context, text, &textSize, hwndFontSrc) )
        {
            goto error_out;
        }
    }

    // Even if we use a font other than the dialog font to calculate the text
    // size, we always have to get the dialog font and select it into a HDC to
    // correctly calculate the dialog units.
    HDC hdc = GetDC(hwndForDC);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(context, API_FAILED_MSG, "GetDC");
        goto error_out;
    }

    HFONT dlgFont = NULL;
    bool createdFont = false;

    if ( hwndDlg == NULL )
    {
        fontSize = 0;
        fontName = rxGetStringAttribute(context, dlgObj, "FONTNAME");

        RexxObjectPtr rxSize = context->SendMessage0(dlgObj, "FONTSIZE");
        if ( rxSize != NULLOBJECT )
        {
            context->ObjectToUnsignedInt32(rxSize, &fontSize);
        }

        if ( fontName != NULL && fontSize != 0 )
        {
            dlgFont = createFontFromName(hdc, fontName, fontSize);
            if ( dlgFont != NULL )
            {
                createdFont = true;
            }
        }
    }
    else
    {
        dlgFont = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0);
    }

    // If dlgFont is null, then, (almost for sure,) the dialog will be using the
    // default system font.  The exception to this is if the user calls the
    // getTextSizeDlg() method before the create() method, and then defines a
    // custom font in create().  The docs tell the user not to do that, but
    // there is nothing to do about it if they do.
    if ( dlgFont == NULL )
    {
        dlgFont = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    HFONT hOldFont = (HFONT)SelectObject(hdc, dlgFont);
    if ( textSize.cx == 0 )
    {
        GetTextExtentPoint32(hdc, text, (int)strlen(text), &textSize);
    }

    screenToDlgUnit(hdc, (POINT *)&textSize);

    SelectObject(hdc, hOldFont);
    ReleaseDC(hwndForDC, hdc);

    if ( createdFont )
    {
        DeleteObject(dlgFont);
    }

    return rxNewSize(context, textSize.cx, textSize.cy);

error_out:
    return NULLOBJECT;
}


/**
 * Determine if a dialog control belongs to the specified dialog control class.
 *
 * @param hControl   Handle to the control.
 * @param control    One of the oodControl types specifying the class to check
 *                   for.
 *
 * @return True if the dialog control is the type specified, otherwise false.
 */
bool checkControlClass(HWND hControl, oodControl_t control)
{
    TCHAR buf[64];
    TCHAR *pClass = NULL;

    switch ( control )
    {
        case winStatic :
            pClass = WC_STATIC;
            break;
        case winButton :
            pClass = WC_BUTTON;
            break;
        case winTreeView :
            pClass = WC_TREEVIEW;
            break;
        case winListView :
            pClass = WC_LISTVIEW;
            break;
        case winTab :
            pClass = WC_TABCONTROL;
            break;
        case winEdit :
            pClass = WC_EDIT;
            break;
        case winRadioButton :
            pClass = WC_BUTTON;
            break;
        case winCheckBox :
            pClass = WC_BUTTON;
            break;
        case winGroupBox :
            pClass = WC_BUTTON;
            break;
        case winListBox :
            pClass = WC_LISTBOX;
            break;
        case winComboBox :
            pClass = WC_COMBOBOX;
            break;
        case winScrollBar :
            pClass = WC_SCROLLBAR;
            break;
        case winProgressBar :
            pClass = PROGRESS_CLASS;
            break;
        case winTrackBar :
            pClass = TRACKBAR_CLASS;
            break;
        case winMonthCalendar :
            pClass = MONTHCAL_CLASS;
            break;
        case winDateTimePicker :
            pClass = DATETIMEPICK_CLASS;
            break;
        default :
            return false;
    }

    if ( ! RealGetWindowClass(hControl, buf, sizeof(buf)) || strcmp(buf, pClass) )
    {
        return false;
    }

    if ( control == winCheckBox || control == winRadioButton || control == winGroupBox )
    {
        BUTTONTYPE type = getButtonInfo(hControl, NULL, NULL);
        switch ( control )
        {
            case winCheckBox :
                if ( type != check )
                {
                    return false;
                }
                break;
            case winRadioButton :
                if ( type != radio )
                {
                    return false;
                }
                break;
            case winGroupBox :
                if ( type != group )
                {
                    return false;
                }
                break;
        }
    }
    return true;
}

