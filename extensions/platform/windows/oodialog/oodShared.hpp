/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2014 Rexx Language Association. All rights reserved.    */
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

#ifndef oodShared_Included
#define oodShared_Included

// Buffer sizes are doubled for each increment
#define SMALL_BUF_SIZE                256
#define MEDIUM_BUF_SIZE               512
#define LARGE_BUF_SIZE                1024
#define HUGE_BUF_SIZE                 2048

#define ERR_MB_FLAGS                  MB_OK | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL
#define INFO_MB_FLAGS                 MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL


inline LONG_PTR setWindowPtr(HWND hwnd, int index, LONG_PTR newPtr)
{
#ifndef __REXX64__
#pragma warning(disable:4244)
#endif
    return SetWindowLongPtr(hwnd, index, newPtr);
#ifndef __REXX64__
#pragma warning(default:4244)
#endif
}

inline LONG_PTR getWindowPtr(HWND hwnd, int index)
{
    return GetWindowLongPtr(hwnd, index);
}

inline LONG_PTR setClassPtr(HWND hwnd, int index, LONG_PTR newPtr)
{
#ifndef __REXX64__
#pragma warning(disable:4244)
#endif
    return SetClassLongPtr(hwnd, index, newPtr);
#ifndef __REXX64__
#pragma warning(default:4244)
#endif
}

inline LONG_PTR getClassPtr(HWND hwnd, int index)
{
    return GetClassLongPtr(hwnd, index);
}

extern int               putUnicodeText(LPWORD dest, const char *text);
extern int               putUnicodeText(LPWORD dest, const char *text, HRESULT *pHR);
extern LPWSTR            ansi2unicode(LPCSTR str);
extern RexxStringObject  unicode2string(RexxThreadContext *c, PWSTR wstr);
extern RexxObjectPtr     unicode2StringOrNil(RexxThreadContext *c, LPWSTR wstr);
extern char             *unicode2ansi(LPWSTR wstr);
extern char             *getCompleteVersion(RexxThreadContext *c);
extern char             *getStandardConditionMsg(RexxThreadContext *c, wholenumber_t *major, wholenumber_t *minor);

inline RexxStringObject unicode2string(RexxMethodContext *c, LPWSTR wstr)
{
    return unicode2string(c->threadContext, wstr);
}

/**
 * Given a condition object, extracts and returns as a whole number the subcode
 * of the condition.
 */
inline wholenumber_t conditionSubCode(RexxCondition *condition)
{
    return (condition->code - (condition->rc * 1000));
}

/**
 * Convenience function to put up an error message box.
 *
 * @param pszMsg    The message.
 * @param pszTitle  The title of for the message box.
 */
inline void internalErrorMsgBox(CSTRING pszMsg, CSTRING pszTitle)
{
    MessageBox(0, pszMsg, pszTitle, ERR_MB_FLAGS);
}

/**
 * Convenience function to put up an error message box.
 *
 * @param hwnd      Owner window handle
 * @param pszMsg    The message.
 * @param pszTitle  The title of for the message box.
 */
inline void internalErrorMsgBox(HWND owner, CSTRING pszMsg, CSTRING pszTitle)
{
    MessageBox(owner, pszMsg, pszTitle, ERR_MB_FLAGS);
}

/**
 * Convenience function to put up an information message box.
 *
 * @param pszMsg    The message.
 * @param pszTitle  The title of for the message box.
 */
inline void internalInfoMsgBox(CSTRING pszMsg, CSTRING pszTitle)
{
    MessageBox(0, pszMsg, pszTitle, INFO_MB_FLAGS);
}

/**
 * Convenience function to put up an information message box.
 *
 * @param hwnd      Owner window handle
 * @param pszMsg    The message.
 * @param pszTitle  The title of for the message box.
 */
inline void internalInfoMsgBox(HWND owner, CSTRING pszMsg, CSTRING pszTitle)
{
    MessageBox(owner, pszMsg, pszTitle, INFO_MB_FLAGS);
}

#endif
