/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2010 Rexx Language Association. All rights reserved.    */
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

#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>
#include <malloc.h>
#include <dlgs.h>
#include <shlwapi.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodDeviceGraphics.hpp"


#define MB_BUTTON_KEYWORDS "ABORTRETRYIGNORE CANCELTRYCONTINUE HELP OK OKCANCEL RETRYCANCEL YESNO YESNOCANCEL"

/**
 * Initializes the string to int map for common Windows constant flags.  Things
 * like MB_OK, etc..
 *
 * @return The string to int map.
 */
static String2Int *winCommonInitMap(void)
{
    String2Int *cMap = new String2Int;

    // MessagBox flags.  Buttons
    cMap->insert(String2Int::value_type("ABORTRETRYIGNORE",  MB_ABORTRETRYIGNORE ));
    cMap->insert(String2Int::value_type("CANCELTRYCONTINUE", MB_CANCELTRYCONTINUE));
    cMap->insert(String2Int::value_type("HELP",              MB_HELP             ));
    cMap->insert(String2Int::value_type("OK",                MB_OK               ));
    cMap->insert(String2Int::value_type("OKCANCEL",          MB_OKCANCEL         ));
    cMap->insert(String2Int::value_type("RETRYCANCEL",       MB_RETRYCANCEL      ));
    cMap->insert(String2Int::value_type("YESNO",             MB_YESNO            ));
    cMap->insert(String2Int::value_type("YESNOCANCEL",       MB_YESNOCANCEL      ));

    // MessageBox Icons
    cMap->insert(String2Int::value_type("EXCLAMATION", MB_ICONEXCLAMATION));
    cMap->insert(String2Int::value_type("WARNING",     MB_ICONWARNING    ));
    cMap->insert(String2Int::value_type("INFORMATION", MB_ICONINFORMATION));
    cMap->insert(String2Int::value_type("ASTERISK",    MB_ICONASTERISK   ));
    cMap->insert(String2Int::value_type("QUESTION",    MB_ICONQUESTION   ));
    cMap->insert(String2Int::value_type("STOP",        MB_ICONSTOP       ));
    cMap->insert(String2Int::value_type("ERROR",       MB_ICONERROR      ));
    cMap->insert(String2Int::value_type("HAND",        MB_ICONHAND       ));
    cMap->insert(String2Int::value_type("QUERY",       MB_ICONQUESTION   ));
    cMap->insert(String2Int::value_type("NONE",        0                 ));

    // MessageBox default button
    cMap->insert(String2Int::value_type("DEFBUTTON1",         MB_DEFBUTTON1));
    cMap->insert(String2Int::value_type("DEFBUTTON2",         MB_DEFBUTTON2));
    cMap->insert(String2Int::value_type("DEFBUTTON3",         MB_DEFBUTTON3));
    cMap->insert(String2Int::value_type("DEFBUTTON4",         MB_DEFBUTTON4));

    // MessageBox modal
    cMap->insert(String2Int::value_type("APPLMODAL",          MB_APPLMODAL  ));
    cMap->insert(String2Int::value_type("SYSTEMMODAL",        MB_SYSTEMMODAL));
    cMap->insert(String2Int::value_type("TASKMODAL",          MB_TASKMODAL  ));

    // MessageBox miscellaneous
    cMap->insert(String2Int::value_type("DEFAULTDESKTOP",      MB_DEFAULT_DESKTOP_ONLY));
    cMap->insert(String2Int::value_type("RIGHT",               MB_RIGHT               ));
    cMap->insert(String2Int::value_type("RTLREADING",          MB_RTLREADING          ));
    cMap->insert(String2Int::value_type("SETFOREGROUND",       MB_SETFOREGROUND       ));
    cMap->insert(String2Int::value_type("TOPMOST",             MB_TOPMOST             ));
    cMap->insert(String2Int::value_type("SERVICENOTIFICATION", MB_SERVICE_NOTIFICATION));

    //cMap->insert(String2Int::value_type("", ));

    return cMap;
}


/**
 * Translate a keyword to its equivalent numeric value.
 *
 * @param  symbol  The keyword to translate.
 *
 * @return  The numeric value for symbol on success, (int)value == -1 on
 *          failure.
 */
static uint32_t winKeyword2ID(CSTRING symbol, RexxThreadContext *c, int pos, CSTRING type)
{
    static String2Int *winConstantsMap = NULL;

    if ( winConstantsMap == NULL )
    {
        winConstantsMap = winCommonInitMap();
    }

    int id = getKeywordValue(winConstantsMap, symbol);
    if ( id == -1 )
    {
        invalidTypeException(c, pos, type);
    }
    return (uint32_t)getKeywordValue(winConstantsMap, symbol);
}


/**
 * Helper function for the MessageDialog() routine.  Parses the miscellaneous
 * message box style keywords.
 */
static uint32_t getMiscMBStyle(char *mbStyle, RexxCallContext *c, int pos, CSTRING msg)
{
    uint32_t flag, styles = 0;

    char *token = strtok(mbStyle, " ");
    while ( token != NULL )
    {
        flag = winKeyword2ID(token, c->threadContext, pos, msg);
        if ( flag == (int)-1 )
        {
            return flag;
        }
        styles |= flag;
        token = strtok(NULL, " ");
    }

    return styles;
}

RexxRoutine6(int, messageDialog_rtn, CSTRING, text, OPTIONAL_CSTRING, hwnd, OPTIONAL_CSTRING, _title,
             OPTIONAL_CSTRING, button, OPTIONAL_CSTRING, icon, OPTIONAL_CSTRING, miscStyles)
{
    int result = -1;

    char *uprButton = NULL;
    char *uprIcon = NULL;
    char *uprMiscStyles = NULL;

    HWND hwndOwner = (HWND)string2pointer(hwnd);
    if ( hwndOwner == NULL )
    {
        if ( TopDlg != NULL && TopDlg->onTheTop )
        {
            hwndOwner = TopDlg->hDlg;
        }
    }

    CSTRING title = "ooDialog Application Message";
    if ( argumentExists(3) )
    {
        title = _title;
    }

    // Defaults.  These values are all 0.
    uint32_t flags = MB_OK | MB_DEFBUTTON1 | MB_APPLMODAL;

    uint32_t flag;
    if ( argumentExists(4) )
    {
        uprButton = strdupupr(button);
        if ( uprButton == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }

        flag = winKeyword2ID(uprButton, context->threadContext, 4, "MessageDialog button keyword");
        if ( flag == (int)-1 )
        {
            goto done_out;
        }
        flags |= flag;
    }

    // There is no default for the icon, if omitted there is no icon.
    if ( argumentExists(5) )
    {
        uprIcon = strdupupr(icon);
        if ( uprIcon == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }

        flag = winKeyword2ID(uprIcon, context->threadContext, 5, "MessageDialog icon keyword");
        if ( flag == (int)-1 )
        {
            goto done_out;
        }
        flags |= flag;
    }

    if ( argumentExists(6) )
    {
        uprMiscStyles = strdupupr(miscStyles);
        if ( uprIcon == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }

        flag = getMiscMBStyle(uprMiscStyles, context, 6, "MessageDialog style keyword");
        if ( flag == (int)-1 )
        {
            goto done_out;
        }
        flags |= flag;
    }

    result = MessageBox(hwndOwner, text, title, flags);

done_out:
    safeFree(uprButton);
    safeFree(uprIcon);
    safeFree(uprMiscStyles);

    return result;
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


static char *searchSoundPath(CSTRING file, RexxCallContext *c)
{
    oodResetSysErrCode(c->threadContext);

    // We need a buffer for the path to search, a buffer for the returned full
    // file name, (if found,) and a pointer to char (an unused arg to
    // SearchPath().)
    char *buf = NULL;
    char *fullFileName = NULL;
    char *pFileName;

    // Calculate how much room we need for the search path buffer.
    uint32_t cchCWD = GetCurrentDirectory(0, NULL);

    // Many modern systems no longer have the SOUNDPATH set.
    SetLastError(0);
    uint32_t cchSoundPath = GetEnvironmentVariable("SOUNDPATH", NULL, 0);
    uint32_t rc = GetLastError();
    if ( cchSoundPath == 0 && rc != ERROR_ENVVAR_NOT_FOUND )
    {
        oodSetSysErrCode(c->threadContext, rc);
        goto err_out;
    }

    // Allocate our needed buffers.
    buf = (char *)malloc(cchCWD + cchSoundPath + 3);
    fullFileName = (char *)malloc(_MAX_PATH);
    if ( buf == NULL || fullFileName == NULL )
    {
        outOfMemoryException(c->threadContext);
        goto err_out;
    }

    // Now get the current directory and the sound path.
    cchCWD = GetCurrentDirectory(cchCWD + 1, buf);
    if ( cchCWD == 0 )
    {
        oodSetSysErrCode(c->threadContext);
        goto err_out;
    }

    if ( cchSoundPath != 0 )
    {
        buf[cchCWD++] = ';';
        cchSoundPath = GetEnvironmentVariable("SOUNDPATH", buf + cchCWD, cchSoundPath + 1);
        if ( cchSoundPath == 0 )
        {
            oodSetSysErrCode(c->threadContext);
            goto err_out;
        }
    }

    uint32_t errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    cchSoundPath = SearchPath(buf, file, NULL, _MAX_PATH, fullFileName, &pFileName);
    SetErrorMode(errorMode);

    if ( cchSoundPath == 0 || cchSoundPath >= _MAX_PATH )
    {
        oodSetSysErrCode(c->threadContext);
        goto err_out;
    }

    free(buf);
    return fullFileName;

err_out:
    safeFree(buf);
    safeFree(fullFileName);
    return NULL;
}

RexxRoutine3(RexxObjectPtr, playSound_rtn, OPTIONAL_CSTRING, fileName, OPTIONAL_CSTRING, modifier, NAME, routineName)
{
    bool isStopRoutine = strcmp("STOPSOUNDFILE", routineName) == 0;

    if ( (! isStopRoutine && argumentOmitted(1)) || isStopRoutine )
    {
        return (sndPlaySound(NULL, SND_SYNC | SND_NODEFAULT) ? TheZeroObj : TheOneObj);
    }

    char *fullFileName = searchSoundPath(fileName, context);
    if ( fullFileName == NULL )
    {
        return TheOneObj;
    }

    uint32_t opts = SND_NODEFAULT;
    if ( strcmp("PLAYSOUNDFILE", routineName) == 0 )
    {
        opts |= isYes(modifier) ? SND_ASYNC : SND_SYNC;
    }
    else if ( strcmp("PLAYSOUNDFILEINLOOP", routineName) == 0 )
    {
        opts |= SND_ASYNC | SND_LOOP;
    }
    else
    {
        // Must be Play()
        if ( argumentExists(2) )
        {
            if ( stricmp("LOOP", modifier) == 0 )
            {
                opts |= SND_ASYNC | SND_LOOP;
            }
            else
            {
                opts |= isYes(modifier) ? SND_ASYNC : SND_SYNC;
            }
        }
        else
        {
            opts |= SND_SYNC;
        }
    }

    RexxObjectPtr result = sndPlaySound(fullFileName, opts) ? TheZeroObj : TheOneObj;
    free(fullFileName);
    return result;
}

RexxRoutine1(RexxObjectPtr, routineTest_rtn, RexxObjectPtr, obj)
{
    return TheZeroObj;
}

