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

#ifndef oodCommon_Included
#define oodCommon_Included


#define OOD_ID_EXCEPTION -9

// Enum for the type of an ooDialog class.  Types to be added as needed.
typedef enum
{
    oodPlainBaseDialog, oodCategoryDialog, oodStaticControl, oodButtonControl, oodEditControl,
    oodListBox,         oodProgressBar,    oodUnknown
} oodClass_t;

// Enum for the type of Windows dialog control.
typedef enum
{
    winStatic, winButton, winEdit, winProgressBar
} oodControl_t;

extern BOOL DialogInAdminTable(DIALOGADMIN * Dlg);
extern void rxstrlcpy(CHAR * tar, CONSTRXSTRING &src);
extern void rxdatacpy(CHAR * tar, RXSTRING &src);
extern bool IsYes(const char *s);
extern bool IsNo(const char * s);
extern void *string2pointer(const char *string);
extern void pointer2string(char *, void *pointer);
extern LONG HandleError(PRXSTRING r, CHAR * text);
extern char *strdupupr_nospace(const char *str);
extern DIALOGADMIN *rxGetDlgAdm(RexxMethodContext *, RexxObjectPtr);

extern void ooDialogInternalException(RexxMethodContext *);
extern oodClass_t oodClass(RexxMethodContext *, RexxObjectPtr, oodClass_t *, size_t);
extern int oodResolveSymbolicID(RexxMethodContext *, RexxObjectPtr, RexxObjectPtr, int, int);
extern bool oodSafeResolveID(int *, RexxMethodContext *, RexxObjectPtr, RexxObjectPtr, int, int);


// TODO move to APICommon when ooDialog is converted to use .Pointer instead of
// pointer strings.
extern POINTER rxGetPointerAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name);

#define GET_HANDLE(p) string2pointer(p)
#define GET_HWND(p)   ((HWND)string2pointer(p))
#define GET_POINTER(p) string2pointer(p)

inline void *string2pointer(CONSTRXSTRING *string) { return string2pointer(string->strptr); }
inline void *string2pointer(CONSTRXSTRING &string) { return string2pointer(string.strptr); }

inline void pointer2string(PRXSTRING result, void *pointer)
{
    pointer2string(result->strptr, pointer);
    result->strlength = strlen(result->strptr);
}

inline void safeLocalFree(void *p)
{
    if (p != NULL)
    {
        LocalFree(p);
    }
}

inline void safeFree(void *p)
{
    if (p != NULL)
    {
        free(p);
    }
}

inline void safeDeleteObject(HANDLE h)
{
    if (h != NULL)
    {
        DeleteObject(h);
    }
}

#endif
