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
 * oodRoutines.cpp
 *
 * Contains the implementation for the ooDialog public routines.
 */

#include "ooDialog.h"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <malloc.h>
#include <dlgs.h>
#include <shlwapi.h>
#include "APICommon.h"
#include "oodCommon.h"
#include "oodText.hpp"

/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry InfoMessage(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND hW;

   CHECKARG(1);

   if ((topDlg) && (topDlg->OnTheTop)) hW = topDlg->TheDlg; else hW = NULL;
   MessageBox(hW,argv[0].strptr,"Information", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL);
   RETC(0)
}


/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry ErrorMessage(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND hW;

   CHECKARG(1);

   if ((topDlg) && (topDlg->OnTheTop)) hW = topDlg->TheDlg; else hW = NULL;
   MessageBox(hW,argv[0].strptr,"Error", MB_OK | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);
   RETC(0)
}

/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry YesNoMessage(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND hW;
   UINT uType = MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TASKMODAL;

   CHECKARGLH(1, 2);

   if ( argc == 2 )
   {
      if ( IsNo(argv[1].strptr) )
         uType |= MB_DEFBUTTON2;
      else if ( ! isYes(argv[1].strptr) )
      {
         PSZ  pszMsg;
         CHAR szText[] = "YesNoMessage argument 2 must be one of [Yes, No]; "
                         "found \"%s\"";

         pszMsg = (PSZ)LocalAlloc(LPTR, sizeof(szText) + 1 + argv[1].strlength);
         if ( ! pszMsg )
            RETERR;
         sprintf(pszMsg, szText, argv[1].strptr);
         HandleError(retstr, pszMsg);
         LocalFree(pszMsg);
         return 40;
      }
   }

   retstr->strlength = 1;
   if ((topDlg) && (topDlg->OnTheTop)) hW = topDlg->TheDlg; else hW = NULL;

   if (MessageBox(hW,argv[0].strptr,"Question", uType) == IDYES)
      retstr->strptr[0] = '1';
   else
      retstr->strptr[0] = '0';
   return 0;
}

/** findWindow()
 *
 *  Retrieves a window handle to the top-level window whose class name and
 *  window name match the specified strings. This function does not search child
 *  windows. This function does not perform a case-sensitive search.
 *
 *  @param caption     The title of the window to search for.  Although this
 *                     argument is required, the empty string can be used to
 *                     indicate a null should be used for the caption.
 *
 *  @param className   [optional]  Specifies the window class name of the window
 *                     to search for.  The class name can be any name registered
 *                     with RegisterClass() or RegisterClassEx(), or any of the
 *                     predefined control-class names.
 *
 *                     If className is omitted, it finds any window whose title
 *                     matches the caption argument.
 *
 *  @return  The window handle if the window is found, otherwise 0.
 *
 *  @note  Sets the system error code.
 */
RexxRoutine2(RexxObjectPtr, findWindow_rtn, CSTRING, caption, OPTIONAL_CSTRING, className)
{
    oodResetSysErrCode(context->threadContext);

    if ( strlen(caption) == 0 )
    {
        caption = NULL;
    }
    HWND hwnd = FindWindow(className, caption);
    if ( hwnd == NULL )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return pointer2string(context->threadContext, hwnd);
}


/** msSleep()
 *
 *  Sleeps the specified number of miliseconds.
 *
 *  Prior to ooRexx 4.0.0, both the public routine msSleep() and the external
 *  function sleepMS() were both documented.  For backward compatibility, both
 *  need to be maintained.  sleepMS() is marked as deprecated and mapped to this
 *  function.
 *
 *  @param ms   The number of miliseconds to sleep.
 *
 *  @return     Always returns 0.
 *
 *  @note  This function can block the Windows message loop.  It should really
 *         only be used for durations less than 1 second.  Certainly no more
 *         than a few seconds.  Use SysSleep() instead.
 */
RexxRoutine1(RexxObjectPtr, msSleep_rtn, uint32_t, ms)
{
    Sleep(ms);
    return TheZeroObj;
}

/** winTimer()
 *
 *  The classic Rexx external function, WinTimer() was documented prior to 4.0.0
 *  and therefore needs to be retained for backward compatibility.  This
 *  implementation is poor.  PeekMessage() returns immediately if there are no
 *  messages.  Since there are probably no windows on this thread, the WAIT
 *  function spins in a busy loop consuming 100% of the CPU.
 *
 *  @param  mode    Keyword for what is to be done.  START creates the timer and
 *                  starts it.  WAIT waits on the timer.  STOP destroys the
 *                  timer.
 *
 *  @param  msOrId  Either the period of the timer, in miliseconds, if mode is
 *                  START, or the timer ID for the other modes.
 *
 *  @return The timer ID for the START mode, or success / error return code for
 *          the  other modes.
 */
RexxRoutine2(uintptr_t, winTimer_rtn, CSTRING, mode, uintptr_t, msOrId)
{
    MSG msg;

    if ( stricmp("START", mode) == 0 )
    {
        return SetTimer(NULL, 1001, (unsigned int)msOrId, NULL);
    }
    else if ( stricmp("STOP", mode) == 0 )
    {
        if ( KillTimer(NULL, msOrId) == 0 )
        {
            return GetLastError();
        }
        return 0;
    }
    else if ( stricmp("WAIT", mode) == 0 )
    {
        while ( !PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE) || (msg.wParam != msOrId) )
        {
            ; // do nothing
        }
        return 0;
    }

    wrongArgValueException(context->threadContext, 1, "START, STOP, WAIT", mode);
    return 0;
}


/**
 * A call back function for the Open / Save file name dialog.  This simply
 * ensures that the dialog is brought to the foreground.
 */
UINT_PTR CALLBACK  OFNSetForegroundHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uiMsg == WM_INITDIALOG )
    {
        HWND h = GetParent(hdlg);
        if ( h == NULL )
        {
            h = hdlg;
        }
        SetForegroundWindow(h);
    }

    // Return 0 for all messages so that the default dialog box procedure
    // processess everything.
    return 0;
}

#define FILENAME_BUFFER_LEN 65535

/** fileNameDialog()
 *
 *  Displays either the Open File or Save File dialog to the user and returns
 *  their selection to the Rexx programmer.
 *
 *  This serves as the implementation for both the fileNameDialog() and the
 *  getFileNameWindow() public routines.  getFileNameWindow() was documented
 *  prior to 4.0.0 and therefore must be retained for backward compatibility.
 *
 *  All arguments are optional.
 *
 *  @param preselected
 *  @param _hwndOwner
 *  @param fileFilter
 *  @param loadOrSave
 *  @param _title
 *  @param _defExt
 *  @param multi
 *  @param _sep
 *
 *  @return  The selected file name(s) on success, 0 if the user cancelled or on
 *           error.
 */
RexxRoutine8(RexxObjectPtr, fileNameDlg_rtn,
            OPTIONAL_CSTRING, preselected, OPTIONAL_CSTRING, _hwndOwner, OPTIONAL_RexxStringObject, fileFilter,
            OPTIONAL_CSTRING, loadOrSave,  OPTIONAL_CSTRING, _title,     OPTIONAL_CSTRING, _defExt,
            OPTIONAL_CSTRING, multi,       OPTIONAL_CSTRING, _sep)
{
    // The bulk of the work here is setting up the open file name struct based
    // on the arguements passed by the user.

    OPENFILENAME OpenFileName = {0};
    OpenFileName.lStructSize  = sizeof(OPENFILENAME);
    OpenFileName.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_ENABLEHOOK |
                         OFN_EXPLORER | OFN_ENABLESIZING;

    // Allocate a large buffer for the returned file name(s).
    char * pszFiles = (char *)LocalAlloc(GPTR, FILENAME_BUFFER_LEN);
    if ( pszFiles == NULL )
    {
        outOfMemoryException(context->threadContext);
        return NULLOBJECT;
    }
    OpenFileName.lpstrFile = pszFiles;
    OpenFileName.nMaxFile = FILENAME_BUFFER_LEN;

    // Preselected file name and / or the directory to start in.
    if ( argumentExists(1) && *preselected != '\0' )
    {
        if ( preselected[strlen(preselected) - 1] == '\\' )
        {
            OpenFileName.lpstrInitialDir = preselected;
        }
        else
        {
          StrNCpy(pszFiles, preselected, _MAX_PATH);
        }
    }

    // Possible owner window.
    if ( argumentExists(2) && !_hwndOwner != '\0' )
    {
        OpenFileName.hwndOwner = (HWND)string2pointer(_hwndOwner);
    }

    // File filter string.  Note that the string needs to end with a double
    // null, which has never been documented well to the Rexx user.  This makes
    // it a little tricky if the user sends a string.  There is little chance
    // the user added the extra null.  The need for embedded nulls to separate
    // strings has been documented, so we can expect embedded nulls in the
    // string.  Therefore we have to get the real length of the string data.

    char *filterBuf = NULL;
    OpenFileName.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";

    if ( argumentExists(3) )
    {
        size_t len = context->StringLength(fileFilter);
        if ( len > 0 )
        {
            filterBuf = (char *)LocalAlloc(GMEM_FIXED, len + 2);
            if ( filterBuf == NULL )
            {
                outOfMemoryException(context->threadContext);
                LocalFree(pszFiles);
                return NULLOBJECT;
            }

            memcpy(filterBuf, context->StringData(fileFilter), len);
            filterBuf[len] = '\0';
            filterBuf[len + 1] = '\0';

            OpenFileName.lpstrFilter = filterBuf;
        }
    }
    OpenFileName.nFilterIndex = 1L;

    // Open or save file dialog.  Allow mutiple file selection on open, or not.
    bool open = true;
    bool multiSelect = false;
    if ( argumentExists(4) && *loadOrSave != '\0' )
    {
        char f = toupper(*loadOrSave);
        if ( f == 'S' || f == '0' )
        {
            open = false;
        }
    }
    if ( open && argumentExists(7) && *multi != '\0' )
    {
        char f = toupper(*multi);
        if ( f == 'M' || f == '1' )
        {
            multiSelect = true;
        }
    }

    // Dialog title.
    if ( argumentExists(5) && *_title != '\0' )
    {
        OpenFileName.lpstrTitle = _title;
    }
    else
    {
        OpenFileName.lpstrTitle = (open ? "Open a File" : "Save File As");
    }

    // Default file extension.
    OpenFileName.lpstrDefExt = (argumentExists(6) && *_defExt != '\0') ? _defExt : "txt";

    // Hook procedure to bring dialog to the foreground.
    OpenFileName.lpfnHook = OFNSetForegroundHookProc;

    // Default separator character, can be changed by the user to handle file
    // names with spaces.
    char sepChar = ' ';
    if ( argumentExists(8) && *_sep != '\0' )
    {
        sepChar = *_sep;
    }

    // Okay, show the dialog.
    BOOL success;
    if ( open )
    {
        OpenFileName.Flags |= OFN_FILEMUSTEXIST;
        if ( multiSelect )
        {
            OpenFileName.Flags |= OFN_ALLOWMULTISELECT;
        }

        success = GetOpenFileName(&OpenFileName);

        if ( success && multiSelect )
        {
            // If more than one name selected, names are separated with ASCII 0
            // instead of spaces.
            char *p = pszFiles;

            while ( (*p != '\0') || (*(p+1) != '\0') )
            {
                if ( *p == 0 )
                {
                    *p = sepChar;
                }
                p++;
            }
        }
    }
    else
    {
        OpenFileName.Flags |= OFN_OVERWRITEPROMPT;
        success = GetSaveFileName(&OpenFileName);
    }

    // Return 0 if the user cancelled, or on error.  Otherwise, the return is in
    // the allocated file buffer.
    RexxObjectPtr result = TheZeroObj;
    if ( success )
    {
        result = context->String(pszFiles);
    }

    LocalFree(pszFiles);
    safeLocalFree(filterBuf);

    if ( ! success )
    {
        // Raise an exception if this was a Windows API failure.  Prior to
        // 4.0.0, this was a generic 'routine failed' exception.  We'll try to
        // give the user a little more information.
        DWORD rc = CommDlgExtendedError();
        if ( rc != 0 )
        {
            systemServiceExceptionCode(context->threadContext, API_FAILED_MSG,
                                       open ? "GetOpenFileName" : "GetSaveFileName", rc);
        }
    }

    return result;
}


RexxRoutine1(RexxObjectPtr, routineTest_rtn, RexxObjectPtr, obj)
{
    RexxCallContext *cc = context;
    RexxThreadContext *c = context->threadContext;
    printf("FindClass routine thread context .Size=%p\n", c->FindClass("RECT"));
    printf("FindClass routine context .Size=%p\n", context->FindClass("RECT"));
    printf("FindContextClass call context .Size=%p\n", cc->FindContextClass("REXT"));
    return TheZeroObj;
}

