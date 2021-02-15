/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2017 Rexx Language Association. All rights reserved.    */
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
 * oodShared.cpp
 *
 * Contains object code shared between oodialog.dll and ooDialog.exe
 */

#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>
#include "oodShared.hpp"


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
    char *tmp = (char *)LocalAlloc(LPTR, *bytes);
    if ( tmp == NULL )
    {
        // We just bail.
        LocalFree(buffer);
        return NULL;
    }

    strcpy(tmp, buffer);
    LocalFree(buffer);
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
 * @returns  A buffer allocated through LocalAlloc containing the standard
 *           condition message.  The caller is responsible for freeing the
 *           buffer using LocalFree().
 *
 * @assumes  The the condition has already been preformed.
 *
 * @notes  Null is returned if LocalAlloc() fails.  If for some reason there is
 *         no condition object, the string: "No condition object" is returned.
 *         The caller must still free this string.
 *
 *         Either or both of major and minor can be null.  Both are only set on
 *         success.
 *
 *         At times, the condition message is used to fill in an edit control
 *         text.  For these cases, if \r\n is not used, the edit control does
 *         not honor the line breaks.  Using \r\n does not seem to change the
 *         way the text is displayed when printf() is used.
 */
char *getStandardConditionMsg(RexxThreadContext *c, wholenumber_t *major, wholenumber_t *minor)
{
    size_t bytes  = HUGE_BUF_SIZE;
    char *condMsg = (char *)LocalAlloc(LPTR, bytes);
    if ( condMsg == NULL )
    {
        return condMsg;
    }

    RexxDirectoryObject condObj = c->GetConditionInfo();
    RexxCondition       condition;
    if ( condObj == NULLOBJECT )
    {
        strcpy(condMsg, "No condition object");
        return condMsg;
    }

    size_t usedBytes = 0;
    size_t cBytes    = 0;
    char   buf[MEDIUM_BUF_SIZE];

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
                    cBytes = _snprintf(buf, MEDIUM_BUF_SIZE, "%s\r\n", c->ObjectToStringValue(o));

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

    cBytes = _snprintf(buf, MEDIUM_BUF_SIZE, "Error %zd running %s line %zd: %s\r\n", condition.rc,
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

    _snprintf(buf, MEDIUM_BUF_SIZE, "Error %zd.%03zd:  %s\r\n", condition.rc, conditionSubCode(&condition),
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
 *  Puts an ANSI character string converted to a wide (Unicode) character string
 *  in the specified buffer.
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
        int countWideChars = (int)strlen(text) + 1;

        count = MultiByteToWideChar(CP_ACP, 0, text, -1, (LPWSTR)dest, countWideChars);
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
 *  Puts an ANSI character string converted to a wide (Unicode) character string
 *  in the specified buffer.
 *
 *  This is a convenience function that assumes the caller has passed a buffer
 *  known to be big enough.
 *
 *  It works correctly for the empty string "" and is designed to treat a null
 *  pointer for text as an error.
 *
 * @param dest  Buffer in which to place the converted string.  Must be big
 *              enough.
 * @param text  The text to convert.
 * @param pHR   Pointer to a return code. This is set on error to the system
 *              error code, or to 0x8000ffff Catastrophic failure if text is
 *              null.
 *
 * @return The number of wide character values copied to the buffer.  On error
 *         this is zero.
 */
int putUnicodeText(LPWORD dest, const char *text, HRESULT *pHR)
{
    int count = 0;
    if ( text == NULL )
    {
        *pHR = 0x8000ffff; // Catastrophic failure
    }
    else
    {
        int countWideChars = (int)strlen(text) + 1;

        count = MultiByteToWideChar(CP_ACP, 0, text, -1, (LPWSTR)dest, countWideChars);
        if ( count == 0 )
        {
            *pHR = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return count;
}


/**
 * Alocates a buffer and converts an ANSI string into a wide (Unicode) character
 * string.
 *
 * @param str     The ANSI string to convert, can not be null, must be null
 *                terminated.
 *
 * @return The converted string, or null on failure.
 *
 * @note  The caller is responsible for freeing the returned string.  Memory is
 *        allocated using LocalAlloc.
 */
LPWSTR ansi2unicode(LPCSTR str)
{
    if ( str == NULL )
    {
        return NULL;
    }

    LPWSTR wstr = NULL;

    size_t len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);

    if ( len != 0 )
    {
        wstr = (LPWSTR)LocalAlloc(LPTR, len * 2);
        if ( wstr != NULL )
        {
            if ( MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, (int)len ) == 0)
            {
                // Conversion failed.
                LocalFree(wstr);
                wstr = NULL;
            }
        }
    }

    return wstr;
}


/**
 * Allocates a buffer and converts a wide character (Unicode) string to an Ansi
 * string.
 *
 * @param wstr    The string to convert.
 *
 * @return The converted string, or null on error.
 *
 * @note  The caller is responsible for freeing the returned string.  Memory is
 *        allocated using LocalAlloc.
 */
char *unicode2ansi(LPWSTR wstr)
{
    if (wstr == NULL)
    {
        return NULL;
    }

    char *ansiStr = NULL;
    int32_t neededLen = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);

    if ( neededLen != 0 )
    {
        ansiStr = (char *)LocalAlloc(LPTR, neededLen);
        if ( ansiStr != NULL )
        {
            if ( WideCharToMultiByte(CP_ACP, 0, wstr, -1, ansiStr, neededLen, NULL, NULL) == 0 )
            {
                /* conversion failed */
                LocalFree(ansiStr);
                ansiStr = NULL;
            }
        }
    }

    return ansiStr;
}

/**
 * Converts a wide character (Unicode) string to a Rexx string object.
 *
 * @param c    Thread context we are operating in.
 * @param wstr Wide character string to convert.
 *
 * @return The conveted string as a new Rexx string object on success.
 *
 * @remarks  The Rexx null string is returned if an error occurs.  Use
 *           unicode2StringOrNil() to return the .nil ojbect on errors.
 */
RexxStringObject unicode2string(RexxThreadContext *c, LPWSTR wstr)
{
    RexxStringObject result = c->NullString();
    if ( wstr == NULL )
    {
        goto done_out;
    }

    char *str = unicode2ansi(wstr);
    if ( str == NULL )
    {
        goto done_out;
    }

    result = c->String(str);
    LocalFree(str);

done_out:
    return result;
}


/**
 * Converts a wide character (Unicode) string to a Rexx string object, or .nil
 * on error.
 *
 * @param c    Thread context we are operating in.
 * @param wstr Wide character string to convert.
 *
 * @return The conveted string as a new Rexx string object on success, .nil on
 *         error.
 */
RexxObjectPtr unicode2StringOrNil(RexxThreadContext *c, LPWSTR wstr)
{
    RexxObjectPtr result = c->Nil();
    if ( wstr == NULL )
    {
        goto done_out;
    }

    char *str = unicode2ansi(wstr);
    if ( str == NULL )
    {
        goto done_out;
    }

    result = c->String(str);
    LocalFree(str);

done_out:
    return result;
}


/**
 * Allocates and returns a string buffer containing the "complete" ooDialog
 * versions string.
 *
 * @param c    Pointer to a Rexx thread context.
 *
 * @return The version string, or null on a memory allocation failures.
 *
 * @note  The caller is reponsible for freeing the returned string.
 *
 *        If the returned string is used in a multi-line edit control, it needs
 *        to use \r\n instead of a simple \n.  Since using \r\n has no ill
 *        effects if the string is used in printf(), we use \r\n always.
 */
char *getCompleteVersion(RexxThreadContext *c)
{
    char *buf = (char *)LocalAlloc(LPTR, MEDIUM_BUF_SIZE);
    if ( buf == NULL )
    {
        return NULL;
    }

    int  bits = 32;

#if defined(_WIN64)
    bits = 64;
#endif

#ifdef _DEBUG
    _snprintf(buf, MEDIUM_BUF_SIZE, "\r\nooDialog: ooDialog Version %d.%d.%d.%d (%d bit) - Internal Test Version\r\n",
             OOD_VER, OOD_REL, OOD_MOD, OOD_BLD, bits);
#else
    _snprintf(buf, MEDIUM_BUF_SIZE, "\r\nooDialog: ooDialog Version %d.%d.%d.%d (%d bit)\r\n",
             OOD_VER, OOD_REL, OOD_MOD, OOD_BLD, bits);
#endif

    char buf1[SMALL_BUF_SIZE];

#define QUOTE(arg) #arg
#define QUOTED(name) QUOTE(name)
    _snprintf(buf1, SMALL_BUF_SIZE, "          Built %s %s\r\n"
                                    "          Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.\r\n"
                                    "          Copyright (c) %s Rexx Language Association. All rights reserved.\r\n"
                                    "\r\n\r\n",
              __DATE__, __TIME__, QUOTED(OOD_COPY_YEAR));
    strcat(buf, buf1);

    size_t rx = c->InterpreterVersion();

    _snprintf(buf1, SMALL_BUF_SIZE, "Rexx:     Open Object Rexx Version %zd.%zd.%zd\r\n\r\n",
             (rx >> 16) & 0xff, (rx >> 8) & 0xff, rx & 0x0000ff);
    strcat(buf, buf1);

    return buf;
}
