/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2013-2018 Rexx Language Association. All rights reserved.    */
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
 * oodMain.cpp
 *
 * ooDialog.exe, a stand alone interface to run ooDialog programs.  This is
 * essentially rexxhide, with some ooDialog specific modifications.
 *
 * At this time there is no ooDialog specific code that would prevent
 * ooDialog.exe from running any ooRexx program with no console.
 */

#include <windows.h>
#include <commctrl.h>
#include <oorexxapi.h>
#include <stdio.h>
#include <shlwapi.h>
#include <shlobj.h>
#include "oodShared.hpp"
#include "oodExecutable.hpp"
#include <versionhelpers.h>

#define WinMainDlg_BEGIN

inline CSTRING level2text(uint32_t level)
{

    switch ( level )
    {
        case SECURITY_MANDATORY_UNTRUSTED_RID: return "Untrusted";
        case SECURITY_MANDATORY_LOW_RID:       return "Low";
        case SECURITY_MANDATORY_MEDIUM_RID:    return "Medium";
        case SECURITY_MANDATORY_HIGH_RID:      return "High";
        case SECURITY_MANDATORY_SYSTEM_RID:    return "System";
        default:                               return "Unknown";
    }
}

inline bool requiresElevation(pConfigureArguments pca)
{
    if ( pca->isVistaOrLater && ! pca->isRunAsAdmin )
    {
        return true;
    }
    return false;
}

inline bool isSingleMode(HWND hDlg)
{
    return IsDlgButtonChecked(hDlg, IDC_RB_SINGLE_VIEW) == BST_CHECKED;
}

static void setDlgIcon(HWND hDlg, HINSTANCE hInstance)
{
    HANDLE hIcon = LoadImage(hInstance, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON,
                             GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
     if( hIcon )
     {
         SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
     }
}

static void setButtonIcons(HWND hDlg, HINSTANCE hInstance)
{
    int32_t cx   = GetSystemMetrics(SM_CXSMICON);
    int32_t cy   = GetSystemMetrics(SM_CYSMICON);

    HANDLE hIcon = LoadImage(hInstance, MAKEINTRESOURCE(IDI_UP_ARROW), IMAGE_ICON, cx, cy, 0);
    HWND hButton = GetDlgItem(hDlg, IDC_PB_UP);

    if ( hIcon )
    {
        SendMessage(hButton, BM_SETIMAGE , IMAGE_ICON, (LPARAM)hIcon);
    }

    hButton = GetDlgItem(hDlg, IDC_PB_DOWN);
    hIcon = LoadImage(hInstance, MAKEINTRESOURCE(IDI_DOWN_ARROW), IMAGE_ICON, cx, cy, 0);

    if ( hIcon )
    {
        SendMessage(hButton, BM_SETIMAGE , IMAGE_ICON, (LPARAM)hIcon);
    }

    hButton = GetDlgItem(hDlg, IDC_PB_DEL);
    hIcon = LoadImage(hInstance, MAKEINTRESOURCE(IDI_DELETE), IMAGE_ICON, cx, cy, 0);

    if ( hIcon )
    {
        SendMessage(hButton, BM_SETIMAGE , IMAGE_ICON, (LPARAM)hIcon);
    }

    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);
    hButton = GetDlgItem(hDlg, IDC_PB_DELXX);

    if ( paa->allUsers )
    {
        ShowWindow(hButton, SW_HIDE);
    }
    else
    {
        hIcon = LoadImage(hInstance, MAKEINTRESOURCE(IDI_DELETE_XX), IMAGE_ICON, cx, cy, 0);
        if ( hIcon )
        {
            SendMessage(hButton, BM_SETIMAGE , IMAGE_ICON, (LPARAM)hIcon);
        }
    }
}

/**
 * Formats an error message for a system error and shows it in a message box
 *
 * @param function  Name of system function that failed
 * @param title     Caption for message box
 * @param dw        System error code
 */
static void reportError(HWND hwnd, CSTRING function, CSTRING title, HRESULT dw)
{
    char  msgBuf[HUGE_BUF_SIZE];
    char *formatMsgBuf;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&formatMsgBuf, 0, NULL);

    _snprintf(msgBuf, HUGE_BUF_SIZE, "Error executing Windows API:\n\nFunction:\t\t%s\nError Code:\t%d\n\n%s",
              function, dw, formatMsgBuf);

    internalErrorMsgBox(hwnd, msgBuf, title);
    LocalFree(formatMsgBuf);
}

/**
 * Formats an error message for a system error, plus some follow up text, and
 * shows it in a message box
 *
 * @param function  Name of system function that failed
 * @param title     Caption for message box
 * @param dw        System error code
 */
static void reportErrorPlus(HWND hwnd, CSTRING function, CSTRING title, CSTRING extraMsg, HRESULT dw)
{
    char  msgBuf[HUGE_BUF_SIZE];
    char *formatMsgBuf;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&formatMsgBuf, 0, NULL);

    _snprintf(msgBuf, HUGE_BUF_SIZE, "Error executing Windows API:\n\nFunction:\t\t%s\n"
                                     "Error Code:\t%d\n\n%s\n\n%s",
              function, dw, formatMsgBuf, extraMsg);

    internalErrorMsgBox(hwnd, msgBuf, title);
    LocalFree(formatMsgBuf);
}

/**
 * Reports an error using the specified message in a message box.
 *
 * @param hwnd
 * @param msg
 * @param title
 */
static void reportError(HWND hwnd, CSTRING msg, CSTRING title)
{
    internalErrorMsgBox(hwnd, msg, title);
}

/**
 * Formats an error message using static memory and shows it in a message box.
 *
 * This is primarily for use in memory allocation errors.  The idea is to avoid
 * allocating memory to display an out of memory message.  If we failed to
 * allocate memory, I doubt this will work anyway.
 *
 * @param fmtStr
 * @param title
 * @param dw
 */
static void reportError(HWND hwnd, CSTRING fmtStr, CSTRING title, CSTRING function, uint32_t rc)
{
    char  msgBuf[HUGE_BUF_SIZE];

    _snprintf(msgBuf, HUGE_BUF_SIZE, fmtStr, function, rc);
    internalErrorMsgBox(hwnd, msgBuf, title);
}

/**
 * Formats an error message that uses a single, numerical, insert and shows it
 * in a message box.
 *
 * @param fmt
 * @param insert
 */
static void reportError(CSTRING fmt, uint32_t insert, CSTRING title)
{
    char buf[MEDIUM_BUF_SIZE];

    _snprintf(buf, MEDIUM_BUF_SIZE, fmt, insert);
    internalErrorMsgBox(buf, title);
}

inline void unicodeConversionErr(void)
{
    internalErrorMsgBox(UNICODE_CONVERSION_ERR, OS_ERR_TITLE);
}


/**
 *
 *
 *
 * @return uint32_t
 *
 * @notes  This function must be run on Vista or later, or it fails.  We assume
 *         the caller has already checked the OS.
 */
uint32_t getProcessIntegrityLevel(HWND hwnd, uint32_t *rc)
{
    PTOKEN_MANDATORY_LABEL pTokenIL       = NULL;
    uint32_t               integrityLevel = 0;
    uint32_t               error          = ERROR_SUCCESS;
    HANDLE                 hToken         = NULL;
    uint32_t               tokenSize      = 0;

    // Open the primary access token of the process with TOKEN_QUERY.
    if ( ! OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) )
    {
        error = GetLastError();
        reportError(hwnd, "OpenProcessToken", OS_ERR_TITLE, error);
        goto done_out;
    }

    // Query the size of the token integrity level so we can allocate a buffer
    // for it.
    if ( ! GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, (DWORD *)&tokenSize) )
    {
        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        {
            error = GetLastError();
            reportError(hwnd, "GetTokenInformation", OS_ERR_TITLE, error);
            goto done_out;
        }
    }

    // Now we allocate a buffer for the integrity level information.
    pTokenIL = (TOKEN_MANDATORY_LABEL *)LocalAlloc(LPTR, tokenSize);
    if (pTokenIL == NULL)
    {
        error = GetLastError();
        reportError(hwnd, "LocalAlloc", OS_ERR_TITLE, error);
        goto done_out;
    }

    // Retrieve token integrity level information.
    if ( ! GetTokenInformation(hToken, TokenIntegrityLevel, pTokenIL, tokenSize, (DWORD *)&tokenSize))
    {
        error = GetLastError();
        reportError(hwnd, "GetTokenInformation", OS_ERR_TITLE, error);
        goto done_out;
    }

    // Integrity Level SIDs are in the form of S-1-16-0xXXXX. (e.g.
    // S-1-16-0x1000 stands for low integrity level SID). There is one and
    // only one subauthority.
    integrityLevel = *GetSidSubAuthority(pTokenIL->Label.Sid, 0);

done_out:
    if ( hToken )
    {
        CloseHandle(hToken);
    }
    if ( pTokenIL)
    {
        LocalFree(pTokenIL);
    }

    *rc = error;

    return integrityLevel;
}


/**
 *
 *
 *
 * @param hwnd
 * @param isVistaOrLater
 * @param rc
 *
 * @return bool
 */
bool isUserInAdminGroup(HWND hwnd, bool isVistaOrLater, uint32_t *rc)
{
    HANDLE   hTokenToCheck = NULL;
    HANDLE   hToken        = NULL;
    bool     inGroup       = false;
    uint32_t error         = ERROR_SUCCESS;
    DWORD    cbSize        = 0;

    // Open the primary access token of the process for query and duplicate.
    if ( ! OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &hToken) )
    {
        error = GetLastError();
        reportError(hwnd, "OpenProcessToken", OS_ERR_TITLE, error);
        goto done_out;
    }

    if ( isVistaOrLater )
    {
        // Running Windows Vista or later.  Determine token type: limited,
        // elevated, or default.
        TOKEN_ELEVATION_TYPE elevType;
        if ( ! GetTokenInformation(hToken, TokenElevationType, &elevType, sizeof(elevType), &cbSize) )
        {
            error = GetLastError();
            reportError(hwnd, "GetTokenInformation", OS_ERR_TITLE, error);
            goto done_out;
        }

        // If limited, get the linked elevated token for further check.
        if ( TokenElevationTypeLimited == elevType )
        {
            if ( ! GetTokenInformation(hToken, TokenLinkedToken, &hTokenToCheck, sizeof(hTokenToCheck), &cbSize) )
            {
                error = GetLastError();
                reportError(hwnd, "GetTokenInformation", OS_ERR_TITLE, error);
                goto done_out;
            }
        }
    }

    // CheckTokenMembership requires an impersonation token. If we just got a
    // linked token, it already is an impersonation token.  If we did not get
    // a linked token, duplicate the original into an impersonation token for
    // CheckTokenMembership.
    if ( ! hTokenToCheck )
    {
        if ( ! DuplicateToken(hToken, SecurityIdentification, &hTokenToCheck) )
        {
            error = GetLastError();
            reportError(hwnd, "DuplicateToken", OS_ERR_TITLE, error);
            goto done_out;
        }
    }

    // Create the SID corresponding to the Administrators group.
    BYTE adminSID[SECURITY_MAX_SID_SIZE];

    cbSize = sizeof(adminSID);
    if ( ! CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID, &cbSize) )
    {
        error = GetLastError();
        reportError(hwnd, "CreateWellKnownSid", OS_ERR_TITLE, error);
        goto done_out;
    }

    // Check if the token to be checked contains admin SID.
    // http://msdn.microsoft.com/en-us/library/aa379596(VS.85).aspx:
    // To determine whether a SID is enabled in a token, that is, whether it
    // has the SE_GROUP_ENABLED attribute, call CheckTokenMembership.
    BOOL inAdminGroup;
    if ( ! CheckTokenMembership(hTokenToCheck, &adminSID, &inAdminGroup) )
    {
        error = GetLastError();
        reportError(hwnd, "CheckTokenMembership", OS_ERR_TITLE, error);
        goto done_out;
    }

    inGroup = inAdminGroup ? true : false;

done_out:
    if ( hToken )
    {
        CloseHandle(hToken);
    }
    if ( hTokenToCheck )
    {
        CloseHandle(hTokenToCheck);
    }

    *rc = error;

    return inGroup;
}


/**
 *
 *
 *
 * @return bool
 *
 * @notes  This function must be run on Vista or later, or it fails.  We assume
 *         the caller has already checked the OS.
 */
bool isProcessElevated(HWND hwnd, uint32_t *rc)
{
    uint32_t error    = ERROR_SUCCESS;
    HANDLE   hToken   = NULL;
    bool     elevated = false;

    // Open the primary access token of the process with TOKEN_QUERY.
    if ( ! OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        error = GetLastError();
        reportError(hwnd, "OpenProcessToken", OS_ERR_TITLE, error);
        goto done_out;
    }

    // Retrieve token elevation information.
    TOKEN_ELEVATION elevation;
    uint32_t        size;
    if ( ! GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), (DWORD *)&size) )
    {
        error = GetLastError();
        reportError(hwnd, "GetTokenInformation", OS_ERR_TITLE, error);
        goto done_out;
    }

    elevated = elevation.TokenIsElevated ? true : false;

done_out:
    if ( hToken )
    {
        CloseHandle(hToken);
    }
    *rc = error;

    return elevated;
}

/**
 *
 *
 * @param hwnd
 * @param isVistaOrLater
 * @param rc
 *
 * @return bool
 */
bool runningAsAdmin(HWND hwnd, uint32_t *rc)
{
    PSID     adminGroup = NULL;
    bool     result     = false;
    uint32_t error      = ERROR_SUCCESS;

    // get SID of the admin group.
    SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
    if ( ! AllocateAndInitializeSid(&authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0, &adminGroup) )
    {
        error = GetLastError();
        reportError(hwnd, "AllocateAndInitializeSid", OS_ERR_TITLE, error);
        goto done_out;
    }

    // Determine whether the SID of administrators group is enabled in
    // the primary access token of the process.
    BOOL isAdmin;
    if ( ! CheckTokenMembership(NULL, adminGroup, &isAdmin) )
    {
        error = GetLastError();
        reportError(hwnd, "CheckTokenMembership", OS_ERR_TITLE, error);
        goto done_out;
    }

done_out:
    if ( adminGroup )
    {
        FreeSid(adminGroup);
    }
    *rc = error;

    return isAdmin ? true : false;
}


uint32_t determineStatus(HWND hDlg, pConfigureArguments pca)
{
    OSVERSIONINFO osver = { sizeof(osver) };
    uint32_t      rc    = ERROR_SUCCESS;

    pca->requiresElevation = true;

    pca->isVistaOrLater = IsWindowsVistaOrGreater();

    pca->isInAdminGroup = isUserInAdminGroup(hDlg, pca->isVistaOrLater, &rc);
    if ( rc != ERROR_SUCCESS )
    {
        goto done_out;
    }

    pca->isRunAsAdmin = runningAsAdmin(hDlg, &rc);
    if ( rc != ERROR_SUCCESS )
    {
        goto done_out;
    }

    if ( pca->isVistaOrLater )
    {
        pca->isElevated = isProcessElevated(hDlg, &rc);
        if ( rc != ERROR_SUCCESS )
        {
            goto done_out;
        }

        pca->integrityLevel = getProcessIntegrityLevel(hDlg, &rc);
    }

    pca->requiresElevation = requiresElevation(pca);

done_out:
    return rc;
}

/**
 * Creates a mono spaced font for the size specified.
 *
 * @param fontSize
 *
 * @return HFONT
 */
HFONT createMonoSpacedFont(int32_t fontSize)
{
    int   width = 0;                              // average character width
    int   escapement = 0;                         // angle of escapement
    int   orientation = 0;                        // base-line orientation angle
    int   weight = FW_NORMAL;                     // font weight
    BOOL  italic = FALSE;                         // italic attribute option
    BOOL  underline = FALSE;                      // underline attribute option
    BOOL  strikeOut = FALSE;                      // strikeout attribute option
    uint32_t charSet = DEFAULT_CHARSET;           // character set identifier
    uint32_t outputPrecision = OUT_TT_PRECIS;     // output precision
    uint32_t clipPrecision = CLIP_DEFAULT_PRECIS; // clipping precision
    uint32_t quality = DEFAULT_QUALITY;           // output quality
    uint32_t pitchAndFamily = FF_DONTCARE;        // pitch and family

    HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
    int height = -MulDiv(fontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    DeleteDC(hdc);

    HFONT f = CreateFont(height, width, escapement, orientation, weight, italic, underline, strikeOut,
                         charSet, outputPrecision, clipPrecision, quality, pitchAndFamily, "Courier New");
    return f;
}


/**
 * The dialog procedure for the 'main' dialog.  This is the dialog that displays
 * if the ooDialog.exe executable is run with no arguments, or with the -H
 * option flag.
 *
 * @param hDlg
 * @param uMsg
 * @param wParam
 * @param lParam
 *
 * @return intptr_t CALLBACK
 */
intptr_t CALLBACK WinMainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HFONT font = NULL;

    if ( uMsg == WM_INITDIALOG )
    {
        pProgramArguments pa = (pProgramArguments)lParam;

        setDlgIcon(hDlg, pa->hInstance);

        // Initialize the edit control that displays the long text.
        font = createMonoSpacedFont(10);

        HWND hEdit  = GetDlgItem(hDlg, IDC_EDIT);

        SendMessage(hEdit, WM_SETFONT, (WPARAM)font, TRUE);
        SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)pa->mainMsg);

        // Initialize the short banner text and the dialog title.
        char tmp[MEDIUM_BUF_SIZE];
        HWND hMsg = GetDlgItem(hDlg, IDC_ST_SHORT_MSG);

        switch ( pa->dlgType )
        {
            case conditionDisplay :
                SetWindowText(hDlg, "ooDialog Execute Program: Condition Raised");
                _snprintf(tmp, MEDIUM_BUF_SIZE, "A Condition was raised while executing:\n\t%s", pa->oodProgram);
                break;

            case versionDisplay :
                SetWindowText(hDlg, "ooDialog Execute Program: Version Information");
                _snprintf(tmp, MEDIUM_BUF_SIZE, "ooDialog version %u.%u.%u.%u", OOD_VER, OOD_REL, OOD_MOD, OOD_BLD);
                break;

            case syntaxDisplay :
                SetWindowText(hDlg, "ooDialog Execute Program: Syntax Information");
                _snprintf(tmp, MEDIUM_BUF_SIZE, "ooDialog version %u.%u.%u.%u", OOD_VER, OOD_REL, OOD_MOD, OOD_BLD);
                break;

            default :
                SetWindowText(hDlg, "ooDialog Execute Program: Internal Error");
                _snprintf(tmp, MEDIUM_BUF_SIZE, "An unexpected internal error has happend:\n");
                break;
        }
        SendMessage(hMsg, WM_SETTEXT, 0, (LPARAM)tmp);

        // Set the proper icon image.
        LPCTSTR imageName = IDI_INFORMATION;
        if ( pa->dlgType == conditionDisplay )
        {
            imageName = IDI_ERROR;
        }
        HANDLE hNewIcon = LoadImage(NULL, imageName, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE);
        HWND   hSt      = GetDlgItem(hDlg, IDC_ST_DISPLAY_ICON);
        SendMessage(hSt, STM_SETICON, (WPARAM)hNewIcon, 0);

        if ( pa->allowServices )
        {
            ShowWindow(GetDlgItem(hDlg, IDC_GB_SERVICES), SW_SHOWNA);
            ShowWindow(GetDlgItem(hDlg, IDC_CK_FILEASSOC), SW_SHOWNA);
        }

        setWindowPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pa);

        return TRUE;
    }

    switch ( uMsg )
    {
        case WM_COMMAND:
            switch ( LOWORD(wParam) )
            {
                case IDC_CK_FILEASSOC :
                {
                    if ( HIWORD(wParam) == BN_CLICKED )
                    {
                        if ( IsDlgButtonChecked(hDlg, IDC_CK_FILEASSOC) == BST_CHECKED )
                        {
                            SetDlgItemText(hDlg, IDOK, "Continue");
                        }
                        else
                        {
                            SetDlgItemText(hDlg, IDOK, "Close");
                        }
                    }
                }
                    break;

                case IDOK:
                {
                    if ( IsDlgButtonChecked(hDlg, IDC_CK_FILEASSOC) == BST_CHECKED )
                    {
                        pProgramArguments pa = (pProgramArguments)getWindowPtr(hDlg, GWLP_USERDATA);

                        pa->doConfigure = true;
                    }

                    EndDialog(hDlg, wParam);

                    if ( font )
                    {
                        DeleteObject(font);
                    }
                }
                return TRUE;

                case IDCANCEL:
                {
                    EndDialog(hDlg, wParam);

                    if ( font )
                    {
                        DeleteObject(font);
                    }
                }
                    return TRUE;
            }

        default:
            break;
    }

    return FALSE;
}

#define FileAssocDlg_BEGIN

/**
 * Frees the alocated records in the specified extension info array and adjusts
 * the next pointer.  Does not free the array itself.
 *
 * @param extRecs
 */
static void resetExtRecords(pExtRecords extRecs, recArrayType_t ra)
{
    switch ( ra )
    {
        case auPeRecArray :
            if ( extRecs->allUsersRecs != NULL )
            {
                for ( size_t i = 0; i < extRecs->auNext; i++ )
                {
                    safeLocalFree(extRecs->allUsersRecs[i]);
                    extRecs->allUsersRecs[i] = NULL;
                }
                extRecs->auNext = 0;
            }
            break;

        case cuPeRecArray :
            if ( extRecs->curUserRecs != NULL )
            {
                for ( size_t i = 0; i < extRecs->cuNext; i++ )
                {
                    safeLocalFree(extRecs->curUserRecs[i]);
                    extRecs->curUserRecs[i] = NULL;
                }
                extRecs->cuNext = 0;
            }
            break;

        case suFaRecArray :
            if ( extRecs->faSuggestedRecs != NULL )
            {
                for ( size_t i = 0; i < extRecs->faSuggestedNext; i++ )
                {
                    safeLocalFree(extRecs->faSuggestedRecs[i]);
                    extRecs->faSuggestedRecs[i] = NULL;
                }
                extRecs->faSuggestedNext = 0;
            }
            break;

        case cuFaRecArray :
            if ( extRecs->faCurrentRecs != NULL )
            {
                for ( size_t i = 0; i < extRecs->faCurrentNext; i++ )
                {
                    safeLocalFree(extRecs->faCurrentRecs[i]);
                    extRecs->faCurrentRecs[i] = NULL;
                }
                extRecs->faCurrentNext = 0;
            }
            break;

        default :
            /* Can't happen all values of the enum are handled. */
            break;
    }
}

/**
 * Frees all allocated memory in extRecs and frees extRecs itself.
 *
 * @param extRecs
 */
static pExtRecords freeExtRecords(pExtRecords extRecs)
{
    if ( extRecs != NULL )
    {
        resetExtRecords(extRecs, auPeRecArray);
        resetExtRecords(extRecs, cuPeRecArray);
        resetExtRecords(extRecs, suFaRecArray);
        resetExtRecords(extRecs, cuFaRecArray);

        safeLocalFree(extRecs->allUsersRecs);
        safeLocalFree(extRecs->curUserRecs);
        safeLocalFree(extRecs->faCurrentRecs);
        safeLocalFree(extRecs->faSuggestedRecs);

        safeLocalFree(extRecs);
    }
    return NULL;
}

/**
 * Called to end the FileAssocDlg dialog.  Since this dialog could,
 * theoretically, be run numerous times within the same process, we free all
 * allocated memory when we close the dialog.
 *
 * @param hDlg
 * @param WPARAM
 */
static void endThisDialog(HWND hDlg, WPARAM wParam)
{
    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);

    freeExtRecords(paa->extensionRecords);
    EndDialog(hDlg, wParam);
}

/**
 * In place string upper.
 *
 * @param s
 */
static void strUpper(char *s)
{
     while(*s++=toupper(*s));
}

/**
 * Generates the 'display name' in the extension record to match what the
 * variables in the record indicate.  That is, if the variables in the record
 * indicate that the .ext is registered for all users, the display name is
 * marked accordingly.
 *
 * @param rec
 *
 * @note  We use this scheme:  The display name has 3 columns.  The first
 *        column indicates the registered status for all users, the second
 *        column indicates the registered status for the current user, and the
 *        third column is the .ext name.
 *
 *        A '-' character indicates the .ext is not registered.  A '*' character
 *        indicates the .ext is registered.  A '+' character indicates the .ext
 *        is registered, but not registered to the ooDialog prog ID.
 *
 *        This is also used to update a PATHEXT record, which does not use the
 *        'other' concept.
 */
static void updateExtDisplayName(pExtensionInfo rec)
{
    char *prefix = "- -  ";
    if ( rec->allUsers )
    {
        if ( rec->curUser )
        {
            prefix = "* *  ";
        }
        else if ( rec->curUserOther )
        {
            prefix = "* +  ";
        }
        else
        {
            prefix = "* -  ";
        }
    }
    else if ( rec->curUser )
    {
        if ( rec->allUsersOther )
        {
            prefix = "+ *  ";
        }
        else
        {
            prefix = "- *  ";
        }
    }
    else
    {
        // Not associated with our program ID, but may be in use by others.
        if ( rec->allUsersOther )
        {
            if ( rec->curUserOther )
            {
                prefix = "+ +  ";
            }
            else
            {
                prefix = "+ -  ";
            }
        }
        else if ( rec->curUser )
        {
            prefix = "- +  ";
        }
    }

    _snprintf(rec->displayName, MAX_EXT_DISPLAY, "%s%s", prefix, rec->extension);
    _snprintf(rec->displayName2, MAX_EXT_DISPLAY, "%s", rec->extension);
}

/**
 * Marks this extension record as one of the 'suggested' extensions.  We use
 * this function to set things correctly, even when we already know it is a
 * suggested function.
 *
 * @param info
 *
 * @return bool
 */
static bool suggestedExt(pExtensionInfo info)
{
    for ( size_t i = 0; i < OOD_SUGGESTED_EXT_COUNT; i++ )
    {
        if ( StrCmpI(oodSuggestedExts[i], info->extension) == 0 )
        {
            strcpy(info->extension, oodSuggestedExts[i]);
            info->suggested = true;
            return true;
        }
    }
    return false;
}

/**
 * Given an extension information record for the specified ext, determines where
 * (in which locations) the extension is associated with the specified progID,
 * and or if it is associated with a different progID.
 *
 * Essentially this fills out the fields of the record for an extension that
 * show where, or if, that extension is registered to a file type.
 *
 * @param keyName  The extension, .ood, for example
 * @param rec      An allocated extension information record
 * @param progID   The progID we are interested in.  Actually this should always
 *                 be OODIALOG_PROGID_VERSIONED, but for testing and flexibility
 *                 we use an input argument.
 *
 */
static void qualifyExtensionInfo(char *extKeyName, pExtensionInfo rec, char *progID)
{
    char     value[MAX_HKEY_VALUE] = {'\0'};
    uint32_t maxValue              = MAX_HKEY_VALUE;
    HKEY     hKey, hSubkey;

    // See if extKeyName is a subkey for current user
    if( RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Classes", 0, KEY_READ, &hKey) == ERROR_SUCCESS )
    {
        if ( RegOpenKeyEx(hKey, extKeyName, 0, KEY_QUERY_VALUE, &hSubkey) == ERROR_SUCCESS )
        {
            if ( RegQueryValueEx(hSubkey, "", NULL, NULL, (LPBYTE)value, (LPDWORD)&maxValue) == ERROR_SUCCESS )
            {
                if ( StrCmpI(value, progID) == 0 )
                {
                    rec->curUser = true;
                }
                else
                {
                    rec->curUserOther = true;
                }
            }

            RegCloseKey(hSubkey);
        }
        RegCloseKey(hKey);
    }

    // See if extKeyName is a subkey for local machine classes
    if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes", 0, KEY_READ, &hKey) == ERROR_SUCCESS )
    {
        if ( RegOpenKeyEx(hKey, extKeyName, 0, KEY_QUERY_VALUE, &hSubkey) == ERROR_SUCCESS )
        {
            value[0] = '\0';
            maxValue = MAX_HKEY_VALUE;

            if ( RegQueryValueEx(hSubkey, "", NULL, NULL, (LPBYTE)value, (LPDWORD)&maxValue) == ERROR_SUCCESS )
            {
                if ( StrCmpI(value, progID) == 0 )
                {
                    rec->allUsers = true;
                }
                else
                {
                    rec->allUsersOther = true;
                }
            }
            RegCloseKey(hSubkey);
        }
        RegCloseKey(hKey);
    }

    suggestedExt(rec);
    updateExtDisplayName(rec);
}

/**
 * Searches the specified list box for an extension record that matches the
 * extension specified.  Returns NULL if no recored is found.
 *
 * @param hLB
 * @param extension
 * @param index  [out]
 *
 * @return pExtensionInfo
 */
static pExtensionInfo getExtRec(HWND hLB, CSTRING extension, uint32_t *index)
{
    LRESULT count = SendMessage(hLB, LB_GETCOUNT, NULL, NULL);
    if ( count > 0 )
    {
        for (LRESULT i = 0; i < count; i++ )
        {
            pExtensionInfo info = (pExtensionInfo)SendMessage(hLB, LB_GETITEMDATA, i, 0);
            if ( StrCmpI(info->extension, extension) == 0 )
            {
                *index = (uint32_t)i;
                return info;
            }
        }
    }
    return NULL;
}

/**
 * Returns the extension record for the currently selected item in the specified
 * list box.
 *
 * We return NULL if there is no selected item.  An enhancement to this program
 * would be to disable / enable things when items are not selected.
 *
 * @param extension
 * @param index
 *
 * @return pExtensionInfo
 */
static pExtensionInfo getSelectedExtRec(HWND hLB, uint32_t *index)
{
    LRESULT selected = SendMessage(hLB, LB_GETCURSEL, NULL, NULL);
    if ( selected < 0 )
    {
        return NULL;
    }

    *index = (uint32_t)selected;
    return (pExtensionInfo)SendMessage(hLB, LB_GETITEMDATA, selected, 0);
}

/**
 * Checks an extension record to see if the extension is registered in the
 * current scope.  That is, if the current scope is all users, is the extension
 * registered for all users?
 *
 * @return bool
 */
inline bool extRegisteredInScope(pAssocArguments paa, pExtensionInfo existing)
{
    if ( paa->allUsers )
    {
        return existing->allUsers;
    }
    else
    {
        return existing->curUser;
    }
}

/**
 * Checks an extension record to see if the extension is registered in the
 * 'other' scope.  That is, if the current scope is all users, is the extension
 * registered for the current user?
 *
 * @return bool
 */
inline bool extRegisteredInOtherScope(pAssocArguments paa, pExtensionInfo existing)
{
    if ( paa->allUsers )
    {
        return existing->curUser;
    }
    else
    {
        return existing->allUsers;
    }
}

/**
 * Updates the extension record using the assumption that it is registered in
 * the current scope.
 *
 * @param paa
 * @param existing
 *
 * @note  If the record has the 'other' set to true, we change it to false
 *        because the extension was just updated to point to our prog ID.  We
 *        don't actuall need to check what it was, we just do it.
 */
static void nowRegisteredInScope(pAssocArguments paa, pExtensionInfo existing)
{
    if ( paa->allUsers )
    {
        existing->allUsers      = true;
        existing->allUsersOther = false;
    }
    else
    {
        existing->curUser       = true;
        existing->curUserOther  = false;
    }
    updateExtDisplayName(existing);
}

/**
 * Updates the extension record using the assumption that it is no longer
 * registered in the current scope.
 *
 * @param paa
 * @param existing
 *
 * @note
 */
static void notRegisteredInScope(pAssocArguments paa, pExtensionInfo existing)
{
    if ( paa->allUsers )
    {
        existing->allUsers = false;
    }
    else
    {
        existing->curUser = false;
    }
    updateExtDisplayName(existing);
}

/**
 * Creates a copy of the specified extension info record.
 *
 * @param hDlg
 * @param rec
 *
 * @return pExtensionInfo
 */
static pExtensionInfo dupExtRec(HWND hDlg, pExtensionInfo rec)
{
    pExtensionInfo r = (pExtensionInfo)LocalAlloc(LPTR, sizeof(extensionInfo));
    if ( r == NULL )
    {
        reportError(hDlg, OUT_OF_MEMORY_ERR_FMT, OS_ERR_TITLE, "LocalAlloc", GetLastError());
        return NULL;
    }
    memcpy(r->extension, rec->extension, sizeof(rec->extension));
    return r;
}

/**
 * Checks if currentRec is an extension that will be deleted from the PATHEXT
 * list box.  If the extension is not in the 'in scope' PATHEXT then deleting it
 * from the list box would not actually do anything, so we shouldn't do
 * anything.
 *
 * @param hDlg
 * @param currentRec
 *
 * @return True don't do any thing, false do do something.
 *
 * @note  We explain to the user why nothing is done.
 */
static bool isNotRelevantPathExt(HWND hDlg, pAssocArguments paa, pExtensionInfo currentRec)
{
    char msg[SMALL_BUF_SIZE];

    if ( paa->allUsers )
    {
        if ( ! currentRec->allUsers )
        {
            _snprintf(msg, SMALL_BUF_SIZE, NOT_IN_SCOPE_PATHEXT_ERR_FMT, currentRec->extension, "All Users");
            internalInfoMsgBox(hDlg, msg, USER_ERR_TITLE);
            return true;
        }
    }
    else
    {
        if ( ! currentRec->curUser )
        {
            _snprintf(msg, SMALL_BUF_SIZE, NOT_IN_SCOPE_PATHEXT_ERR_FMT, currentRec->extension, "Current User");
            internalInfoMsgBox(hDlg, msg, USER_ERR_TITLE);
            return true;
        }
    }
    return false;
}

/**
 * To prevent a catastrophe on the user's computer, we refuse to delete things
 * like .exe from the PATHEXT.
 *
 * @param ext
 * @param hDlg
 *
 * @return True, the extension is required, false it is not required.
 *
 * @note  We explain to the user why nothing is done.
 */
static bool isRequiredPathExt(char *ext, HWND hDlg)
{
    for ( size_t i = 0; i < REQUIRED_PATH_EXT_COUNT; i++ )
    {
        if ( StrCmpI(requiredPathExts[i], ext) == 0 )
        {
            char msg[SMALL_BUF_SIZE];

            _snprintf(msg, SMALL_BUF_SIZE, REQUIRED_PATHEXT_ERR_FMT, ext);
            internalInfoMsgBox(hDlg, msg, USER_ERR_TITLE);

            return true;
        }
    }
    return false;
}

/**
 * Determines and returns the predefined registry key and its descriptive string
 * that matches the scope we are using.  Local machine or  current user.
 *
 * @param paa
 * @param user  Descriptive string [out]
 *
 * @return HKEY
 */
static HKEY getScopeVars(pAssocArguments paa, char **user)
{
    HKEY  hPreDefKey = HKEY_CURRENT_USER;
    char *u          = "the Current User.";

    if ( paa->allUsers )
    {
        hPreDefKey = HKEY_LOCAL_MACHINE;
        u          = "All Users";
    }
    *user = u;
    return hPreDefKey;
}

/**
 * Allocates the specified number of extensionInfo structs.  This is used both
 * for the file association extension records and the PATHEXT extension records.
 *
 * We allocate an array of pointers to an extensionInfo block and then allocate
 * each record individually because we need (may need) to free individual
 * records.
 *
 * @param matches
 * @param hDlg
 *
 * @return pExtensionInfo*
 */
static pExtensionInfo *allocExtensionInfoRecs(size_t matches, HWND hDlg)
{
    pExtensionInfo *recs  = NULL;
    size_t          count = 0;

    recs = (pExtensionInfo *)LocalAlloc(LPTR, matches * sizeof(pExtensionInfo));
    if ( recs == NULL )
    {
        reportError(hDlg, OUT_OF_MEMORY_ERR_FMT, OS_ERR_TITLE, "LocalAlloc", GetLastError());
        goto err_out;
    }

    for ( size_t i = 0; i < matches; i++ )
    {
        pExtensionInfo r = (pExtensionInfo)LocalAlloc(LPTR, sizeof(extensionInfo));
        if ( r == NULL )
        {
            reportError(hDlg, OUT_OF_MEMORY_ERR_FMT, OS_ERR_TITLE, "LocalAlloc", GetLastError());
            goto err_out;
        }

        recs[i] = r;
        count++;
    }
    goto done_out;

err_out:
    for ( size_t i = 0; i < count; i++ )
    {
        safeLocalFree(recs[i]);
    }
    safeLocalFree(recs);
    recs = NULL;

done_out:
    return recs;
}

/**
 * Returns a pointer to a newly alocated extRecords struct.
 *
 * @param hDlg
 *
 * @return  A ponter to an extRecords strut, or NULL on error.
 *
 * @assumes paa->extensionRecords is NULL.
 */
static pExtRecords allocExtRecordsStruct(HWND hDlg)
{
    pExtensionInfo *faSuggestedRecs = NULL;
    pExtensionInfo *faCurrentRecs   = NULL;
    pExtensionInfo *allUsersRecs    = NULL;
    pExtensionInfo *curUserRecs     = NULL;
    pExtRecords     extRecs         = NULL;
    uint32_t        rc              = 0;

    extRecs = (pExtRecords)LocalAlloc(LPTR, sizeof(extRecords));
    if ( extRecs == NULL )
    {
        rc = GetLastError();
        goto err_out;
    }

    faSuggestedRecs = (pExtensionInfo *)LocalAlloc(LPTR, MIN_SUGGESTED_ARRAY_SIZE * sizeof(pExtensionInfo *));
    if ( faSuggestedRecs == NULL )
    {
        rc = GetLastError();
        goto err_out;
    }

    faCurrentRecs = (pExtensionInfo *)LocalAlloc(LPTR, MIN_CURRENT_ARRAY_SIZE * sizeof(pExtensionInfo *));
    if ( faCurrentRecs == NULL )
    {
        rc = GetLastError();
        goto err_out;
    }

    allUsersRecs = (pExtensionInfo *)LocalAlloc(LPTR, MIN_ALLUSERS_ARRAY_SIZE * sizeof(pExtensionInfo *));
    if ( allUsersRecs == NULL )
    {
        rc = GetLastError();
        goto err_out;
    }

    curUserRecs = (pExtensionInfo *)LocalAlloc(LPTR, MIN_CURRENTUSER_ARRAY_SIZE * sizeof(pExtensionInfo *));
    if ( curUserRecs == NULL )
    {
        rc = GetLastError();
        goto err_out;
    }

    extRecs->allUsersRecs    = allUsersRecs;
    extRecs->curUserRecs     = curUserRecs;
    extRecs->faCurrentRecs   = faCurrentRecs;
    extRecs->faSuggestedRecs = faSuggestedRecs;

    extRecs->auSize          = MIN_ALLUSERS_ARRAY_SIZE;
    extRecs->cuSize          = MIN_CURRENTUSER_ARRAY_SIZE;
    extRecs->faCurrentSize   = MIN_CURRENT_ARRAY_SIZE;
    extRecs->faSuggestedSize = MIN_SUGGESTED_ARRAY_SIZE;

    return extRecs;

err_out:
    safeLocalFree(faSuggestedRecs);
    safeLocalFree(faCurrentRecs);
    safeLocalFree(allUsersRecs);
    safeLocalFree(curUserRecs);
    safeLocalFree(extRecs);

    reportError(hDlg, OUT_OF_MEMORY_ERR_FMT, OS_ERR_TITLE, "LocalAlloc", rc);
    return NULL;
}

/**
 * Gets the extension information records for all extensions mapped to the
 * specified progID currently in the registry.
 *
 * @param progID  The progID we are looking for IN
 * @param count   The count of records found    OUT
 *
 * @return An allocated array of extension records.
 *
 * @note  The caller is responsible for freeing the array.
 */
static pExtensionInfo *getExtensionRecords(HWND hDlg, char *progID, size_t *count)
{
    uint32_t cSubKeys = 0;
    HKEY     hKCR     = HKEY_CLASSES_ROOT;

    // We only need the number of subkeys.
    uint32_t retCode = RegQueryInfoKey(HKEY_CLASSES_ROOT, NULL, NULL, NULL, (LPDWORD)&cSubKeys,
                                       NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    uint32_t maxName;                  // Size of name buffer
    char     keyName[MAX_HKEY_NAME];   // Buffer for subkey name

    uint32_t maxValue = MAX_HKEY_VALUE; // Size of value buffer
    char     value[MAX_HKEY_VALUE];     // Buffer for default value for subkey

    // Enumerate the subkeys, and see if the prog ID matches our ooDialog prog
    // ID.  We go through the loop twice, once to count the number of matches so
    // we can allocate the array of extension records.  Then a second time to
    // actually fill in the records.

    pExtensionInfo *recs    = NULL;
    size_t         matches  = 0;

    for ( uint32_t i = 0; i < cSubKeys; i++ )
    {
        maxName = MAX_HKEY_NAME;
        retCode = RegEnumKeyEx(hKCR, i, keyName, (LPDWORD)&maxName, NULL, NULL, NULL, NULL);
        if (retCode == ERROR_SUCCESS)
        {
            if ( *keyName == '.' )
            {
                HKEY hExtKey;

                retCode = RegOpenKeyEx(hKCR, keyName, 0, KEY_QUERY_VALUE, &hExtKey);
                if ( retCode == ERROR_SUCCESS )
                {
                    maxValue = MAX_HKEY_VALUE;
                    value[0] = '\0';

                    retCode = RegQueryValueEx(hExtKey, "", NULL, NULL, (LPBYTE)value, (LPDWORD)&maxValue);
                    if ( retCode == ERROR_SUCCESS )
                    {
                        if ( StrCmpI(value, progID) == 0 )
                        {
                            matches++;
                        }
                    }
                    RegCloseKey(hExtKey);
                }
            }
        }
    }

    if ( matches == 0 )
    {
        goto done_out;
    }

    recs = allocExtensionInfoRecs(matches, hDlg);
    if ( recs == NULL )
    {
        goto done_out;
    }

    size_t processed = 0;
    for ( uint32_t i = 0; i < cSubKeys; i++ )
    {
        maxName = MAX_HKEY_NAME;
        retCode = RegEnumKeyEx(hKCR, i, keyName, (LPDWORD)&maxName, NULL, NULL, NULL, NULL);
        if (retCode == ERROR_SUCCESS)
        {
            if ( *keyName == '.' )
            {
                HKEY hExtKey;

                retCode = RegOpenKeyEx(hKCR, keyName, 0, KEY_QUERY_VALUE, &hExtKey);
                if ( retCode == ERROR_SUCCESS )
                {
                    maxValue = MAX_HKEY_VALUE;
                    value[0] = '\0';

                    retCode = RegQueryValueEx(hExtKey, "", NULL, NULL, (LPBYTE)value, (LPDWORD)&maxValue);
                    if ( retCode == ERROR_SUCCESS )
                    {
                        if ( StrCmpI(value, progID) == 0 )
                        {
                            pExtensionInfo current = recs[processed++];

                            current->exists = true;
                            strcpy(current->extension, keyName);

                            qualifyExtensionInfo(keyName, current, progID);
                        }
                    }
                    RegCloseKey(hExtKey);
                }
            }
        }
    }

done_out:
    *count = matches;
    return recs;
}

/**
 * Get the names of all extensions associated with our ProgID in the current
 * scope.
 *
 * @param hDlg
 * @param paa
 * @param count
 *
 * @return An array of strings which are the extension names.
 *
 * @note  The caller is responsible for freeing the memory of the strings and
 *        the array itself.
 */
static char **getAllExtensionNames(HWND hDlg, pAssocArguments paa, size_t *count)
{
    char     *user;
    uint32_t  retCode;
    HKEY      hClassesKey;
    HKEY      hPreDefKey = getScopeVars(paa, &user);
    uint32_t  cSubKeys   = 0;

    retCode = RegOpenKeyEx(hPreDefKey, "SOFTWARE\\Classes", 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hClassesKey);
    if ( retCode != ERROR_SUCCESS )
    {
        *count = 0;
        return NULL;
    }

    // We only need the number of subkeys.
    retCode = RegQueryInfoKey(hClassesKey, NULL, NULL, NULL, (LPDWORD)&cSubKeys, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL);

    uint32_t maxName;                  // Size of name buffer
    char     keyName[MAX_HKEY_NAME];   // Buffer for subkey name

    uint32_t maxValue = MAX_HKEY_VALUE; // Size of value buffer
    char     value[MAX_HKEY_VALUE];     // Buffer for default value for subkey

    // Enumerate the subkeys, and see if the prog ID matches our ooDialog prog
    // ID.  We go through the loop twice, once to count the number of matches so
    // we can allocate an array of strings.  Then a second time to
    // actually fill in the records.

    char   **recs     = NULL;
    size_t   matches  = 0;
    char    *progID   = paa->progID;

    for ( uint32_t i = 0; i < cSubKeys; i++ )
    {
        maxName = MAX_HKEY_NAME;
        retCode = RegEnumKeyEx(hClassesKey, i, keyName, (LPDWORD)&maxName, NULL, NULL, NULL, NULL);
        if (retCode == ERROR_SUCCESS)
        {
            if ( *keyName == '.' )
            {
                HKEY hExtKey;

                retCode = RegOpenKeyEx(hClassesKey, keyName, 0, KEY_QUERY_VALUE, &hExtKey);
                if ( retCode == ERROR_SUCCESS )
                {
                    maxValue = MAX_HKEY_VALUE;
                    value[0] = '\0';

                    retCode = RegQueryValueEx(hExtKey, "", NULL, NULL, (LPBYTE)value, (LPDWORD)&maxValue);
                    if ( retCode == ERROR_SUCCESS )
                    {
                        if ( StrCmpI(value, progID) == 0 )
                        {
                            matches++;
                        }
                    }
                    RegCloseKey(hExtKey);
                }
            }
        }
    }

    if ( matches == 0 )
    {
        goto done_out;
    }

    recs = (char **)LocalAlloc(LPTR, matches * sizeof(char *));
    if ( recs == NULL )
    {
        reportError(hDlg, OUT_OF_MEMORY_ERR_FMT, OS_ERR_TITLE, "LocalAlloc", GetLastError());
        matches = 0;
        goto done_out;
    }

    size_t processed = 0;
    for ( uint32_t i = 0; i < cSubKeys; i++ )
    {
        maxName = MAX_HKEY_NAME;
        retCode = RegEnumKeyEx(hClassesKey, i, keyName, (LPDWORD)&maxName, NULL, NULL, NULL, NULL);
        if (retCode == ERROR_SUCCESS)
        {
            if ( *keyName == '.' )
            {
                HKEY hExtKey;

                retCode = RegOpenKeyEx(hClassesKey, keyName, 0, KEY_QUERY_VALUE, &hExtKey);
                if ( retCode == ERROR_SUCCESS )
                {
                    maxValue = MAX_HKEY_VALUE;
                    value[0] = '\0';

                    retCode = RegQueryValueEx(hExtKey, "", NULL, NULL, (LPBYTE)value, (LPDWORD)&maxValue);
                    if ( retCode == ERROR_SUCCESS )
                    {
                        if ( StrCmpI(value, progID) == 0 )
                        {
                            char *n = (char *)LocalAlloc(LPTR, strlen(keyName) + 1);
                            if ( n == NULL )
                            {
                                reportError(hDlg, OUT_OF_MEMORY_ERR_FMT, OS_ERR_TITLE, "LocalAlloc", GetLastError());
                                for ( size_t j = 0; j < processed; j++ )
                                {
                                    safeLocalFree(recs[j]);
                                }
                                safeLocalFree(recs);
                                recs = NULL;
                                matches = 0;
                                goto done_out;
                            }
                            strcpy(n, keyName);
                            recs[processed++] = n;
                        }
                    }
                    RegCloseKey(hExtKey);
                }
            }
        }
    }

done_out:
    *count = matches;
    return recs;
}

/**
 * Counts the individual extensions in a string of extensions where the
 * individual extenstions are separated by a ';' character.
 *
 * @param value
 *
 * @return size_t
 */
static size_t countPathExt(char *value)
{
    size_t cnt = 0;
    char *p;

    // Remove any possible trailing semi-colons.
    p = value + strlen(value) - 1;
    while( p > value && *p == ';' )
    {
        p--;
    }
    *(p + 1) = '\0';

    p = strchr(value, ';');
    while ( p != NULL )
    {
        cnt++;
        ++p = strchr(p, ';');
    }

    // Since there are no trailing semi-colons, count == count + 1, if we have
    // at least one.
    if ( cnt > 0 )
    {
        cnt++;
    }
    return cnt;
}

/**
 * Returns the index of the extension information record in an array of records
 * for the specified extension. Or -1 if there is no record.
 *
 * @param recs
 * @param count
 * @param extension
 *
 * @return int32_t
 */
static int32_t getExtRecordIndex(pExtensionInfo *recs, size_t count, char *extension)
{
    for ( size_t i = 0; i < count; i++ )
    {
        pExtensionInfo current = recs[i];

        if ( StrCmpI(current->extension, extension) == 0 )
        {
            return (int32_t)i;
        }
    }
    return -1;
}

/**
 * Returns the extension information record in an array of records for the
 * specified extension.  Or null if there is no record.
 *
 * @param recs
 * @param count
 * @param ext
 *
 * @return pExtensionInfo
 */
static pExtensionInfo getExtRecord(pExtensionInfo *recs, size_t count, char *ext)
{
    for (size_t i = 0; i < count; i++ )
    {
        pExtensionInfo curRec = recs[i];
        if ( StrCmpI(curRec->extension, ext) == 0 )
        {
            return curRec;
        }
    }
    return NULL;
}

/**
 * Fills in a single extension info record with its basic information.
 *
 * Note that this does not fill in or update the display name fields.  Use
 * updateExtDisplayName() for that.
 *
 * @param record
 * @param ext
 * @param isAllUsers
 */
static void fillPathExtRecord(pExtensionInfo record, char *ext, bool isAllUsers)
{
    record->exists = true;
    strcpy(record->extension, ext);
    if ( isAllUsers )
    {
        record->allUsers = true;
    }
    else
    {
        record->curUser = true;
    }
    strUpper(record->extension);
}

/**
 * Fills in all PATHEXT extension records in the specified array
 *
 * @param recs        An array of extensionInfo structs
 * @param pathExt     A PATHEXT string, i.e.: .rex;.com;.cmd; ...
 * @param count       The count of records in recs
 * @param isAllUsers  The PATHEXT string is the All Users PATHEXT
 *
 * @assumes  The count of records matches the number of extensions in pathExt
 */
static void fillPathExtRecords(pExtensionInfo *recs, char *pathExt, size_t count, bool isAllUsers)
{
    char *start = pathExt;
    char *p;

    for ( size_t i = 0; i < count; i++ )
    {
        p = strchr(start, ';');
        if ( p != NULL )
        {
            *p = '\0';
            p++;
        }

        fillPathExtRecord(recs[i], start, isAllUsers);
        start = p;
    }
}

/**
 * Fills the PATHEXT list box using the PATHEXT record arrays and the current
 * mode, single or dual.
 *
 * @param hDlg
 * @param paa
 *
 * @assumes the record arrays are currently correct and that the caller has
 *          emptied the list box.
 */
static void fillPathExtListBox(HWND hDlg, pAssocArguments paa)
{
    pExtRecords extRecs = paa->extensionRecords;
    HWND        hLB     = paa->lbPathExt;

    pExtensionInfo *recArray = NULL;
    size_t         count     = 0;
    pExtensionInfo *secArray = NULL;
    size_t         count2    = 0;

    if ( isSingleMode(hDlg) )
    {
        if ( paa->allUsers )
        {
            recArray = extRecs->allUsersRecs;
            count    = extRecs->auNext;
        }
        else
        {
            recArray = extRecs->curUserRecs;
            count    = extRecs->cuNext;
        }

        for ( size_t i = 0; i < count; i++ )
        {
            pExtensionInfo current = recArray[i];

            LRESULT index = SendMessage(hLB, LB_ADDSTRING, 0, (LPARAM)current->displayName2);
            SendMessage(hLB, LB_SETITEMDATA, index, (LPARAM)current);
        }
    }
    else
    {
        if ( paa->allUsers )
        {
            recArray = extRecs->allUsersRecs;
            count    = extRecs->auNext;
            secArray = extRecs->curUserRecs;
            count2   = extRecs->cuNext;
        }
        else
        {
            recArray = extRecs->curUserRecs;
            count    = extRecs->cuNext;
            secArray = extRecs->allUsersRecs;
            count2   = extRecs->auNext;
        }

        /* Add all records in the record array that matches the scope. */
        for ( size_t i = 0; i < count; i++ )
        {
            pExtensionInfo current = recArray[i];

            LRESULT index = SendMessage(hLB, LB_ADDSTRING, 0, (LPARAM)current->displayName);
            SendMessage(hLB, LB_SETITEMDATA, index, (LPARAM)current);
        }

        /* Now go through all the record in the array that doesn't match the
           scope, and add any record that has not already been added. */
        for ( size_t i = 0; i < count2; i++ )
        {
            pExtensionInfo current = secArray[i];

            if ( paa->allUsers )
            {
                if ( ! current->allUsers )
                {
                    LRESULT index = SendMessage(hLB, LB_ADDSTRING, 0, (LPARAM)current->displayName);
                    SendMessage(hLB, LB_SETITEMDATA, index, (LPARAM)current);
                }
            }
            else
            {
                if ( ! current->curUser )
                {
                    LRESULT index = SendMessage(hLB, LB_ADDSTRING, 0, (LPARAM)current->displayName);
                    SendMessage(hLB, LB_SETITEMDATA, index, (LPARAM)current);
                }
            }
        }
    }
}

/**
 * Fills the File Association Suggested list box using the records in the
 * suggested record array.
 *
 * @param hDlg
 * @param paa
 *
 * @remarks  For the suggested list box, this is easy.  The only records in the
 *           suggested record array are ones that are not registered in the
 *           current scope.  All we are doing is updating the display name.
 */
static void fillSuggestedListBox(HWND hDlg, pAssocArguments paa)
{
    pExtRecords extRecs = paa->extensionRecords;
    HWND        hLB     = paa->lbSuggested;

    pExtensionInfo *recArray = extRecs->faSuggestedRecs;
    size_t          count    = extRecs->faSuggestedNext;

    for ( size_t i = 0; i < count; i++ )
    {
        pExtensionInfo  current = recArray[i];
        char           *display  = isSingleMode(hDlg) ? current->displayName2 : current->displayName;

        LRESULT index = SendMessage(hLB, LB_ADDSTRING, 0, (LPARAM)display);
        SendMessage(hLB, LB_SETITEMDATA, index, (LPARAM)current);
    }

}

static void fillCurrentListBox(HWND hDlg, pAssocArguments paa)
{
    pExtRecords extRecs = paa->extensionRecords;
    HWND        hLB     = paa->lbCurrent;

    pExtensionInfo *recArray = extRecs->faCurrentRecs;
    size_t          count    = extRecs->faCurrentNext;

    if ( isSingleMode(hDlg) )
    {
        for ( size_t i = 0; i < count; i++ )
        {
            pExtensionInfo current = recArray[i];

            if ( extRegisteredInScope(paa, current) )
            {
                LRESULT index = SendMessage(hLB, LB_ADDSTRING, 0, (LPARAM)current->displayName2);
                SendMessage(hLB, LB_SETITEMDATA, index, (LPARAM)current);
            }
        }
    }
    else
    {
        for ( size_t i = 0; i < count; i++ )
        {
            pExtensionInfo current = recArray[i];

            LRESULT index = SendMessage(hLB, LB_ADDSTRING, 0, (LPARAM)current->displayName);
            SendMessage(hLB, LB_SETITEMDATA, index, (LPARAM)current);
        }
    }
}

/**
 * Saves the order of the extension records for the PATHEXT.
 *
 * @param hDlg
 * @param paa
 */
static void savePathExtListBoxOrder(HWND hDlg, pAssocArguments paa)
{
    pExtRecords extRecs = paa->extensionRecords;
    HWND        hLB     = paa->lbPathExt;

    pExtensionInfo *recArray = NULL;
    size_t          count     = 0;
    size_t          next      = 0;

    if ( paa->allUsers )
    {
        recArray = extRecs->allUsersRecs;
        count    = extRecs->auSize;
        next     = extRecs->auNext;
    }
    else
    {
        recArray = extRecs->curUserRecs;
        count    = extRecs->cuSize;
        next     = extRecs->cuNext;
    }

    size_t itemCount = (size_t)SendMessage(hLB, LB_GETCOUNT, NULL, NULL);

    // The PATHEXT list box will never have 0 items.
    for (size_t i = 0, j = 0; i < itemCount; i++ )
    {
        pExtensionInfo info = (pExtensionInfo)SendMessage(hLB, LB_GETITEMDATA, i, 0);
        if ( paa->allUsers )
        {
            if ( info->allUsers )
            {
                recArray[j++] = info;
            }
        }
        else
        {
            if ( info->curUser )
            {
                recArray[j++] = info;
            }
        }

        if ( j >= count )
        {
            // Add error code.  This should never happen, if an extension is added
            // to the list box, its record should be added to the appropriate
            // records array.  But we haven't coded that yet.
            ;
        }
    }

    if ( next < count )
    {
        for ( size_t i = next; i < count; i++ )
        {
            recArray[i] = NULL;
        }
    }
}

/**
 * Checks that the record array specified by ra is big enough to add at least
 * one more record (default.) Or, (optional,) big enough to add min more
 * records.  If not, the array is doubled in size.
 *
 * @param hDlg
 * @param ra
 * @param min
 *
 * @return bool
 */
static bool ensureArraySize(HWND hDlg, recArrayType_t ra, size_t min)
{
    pAssocArguments paa     = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);
    pExtRecords     extRecs = paa->extensionRecords;

    pExtensionInfo *curArray;
    size_t          curSize;
    size_t          curNext;

    switch ( ra )
    {
        case auPeRecArray :
            curArray = extRecs->allUsersRecs;
            curSize  = extRecs->auSize;
            curNext  = extRecs->auNext;
            break;

        case cuPeRecArray :
            curArray = extRecs->curUserRecs;
            curSize  = extRecs->cuSize;
            curNext  = extRecs->cuNext;
            break;

        case suFaRecArray :
            curArray = extRecs->faSuggestedRecs;
            curSize  = extRecs->faSuggestedSize;
            curNext  = extRecs->faSuggestedNext;
            break;

        case cuFaRecArray :
            curArray = extRecs->faCurrentRecs;
            curSize  = extRecs->faCurrentSize;
            curNext  = extRecs->faCurrentNext;
            break;

        default :
            /* Can't really happen */
            return false;
    }

    if ( curNext + min < curSize )
    {
        return true;
    }

    size_t newSize = 2 * curSize;
    while ( curNext + min >= newSize )
    {
        newSize *= 2;
    }

    pExtensionInfo *newArray = (pExtensionInfo *)LocalAlloc(LPTR, newSize * sizeof(pExtensionInfo));
    if ( newArray == NULL )
    {
        reportError(hDlg, OUT_OF_MEMORY_ERR_FMT, OS_ERR_TITLE, "LocalAlloc", GetLastError());
        return false;
    }

    memcpy(newArray, curArray, curSize * sizeof(pExtensionInfo));
    LocalFree(curArray);

    switch ( ra )
    {
        case auPeRecArray :
            extRecs->allUsersRecs = newArray;
            extRecs->auSize       = newSize;
            break;

        case cuPeRecArray :
            extRecs->curUserRecs = newArray;
            extRecs->cuSize       = newSize;
            break;

        case suFaRecArray :
            extRecs->faSuggestedRecs = newArray;
            extRecs->faSuggestedSize = newSize;
            break;

        case cuFaRecArray :
            extRecs->faCurrentRecs = newArray;
            extRecs->faCurrentSize = newSize;
            break;
    }

    return true;
}
inline bool ensureArraySize(HWND hDlg, recArrayType_t ra)
{
    return ensureArraySize(hDlg, ra, 1);
}

/**
 * Ensures that if there is the possibility of the user updating the current
 * user PATHEXT, it has the required extensions in it.
 *
 * @param paa
 *
 * @remarks  When we fill in the fields of a new record, we know that a required
 *           PATHEXT extension exists in the all users PATHEXT, (unless the user
 *           is an idiot,) and that we are adding the record to the current user
 *           PATHEXT.
 */
static bool ensureSafePathExt(HWND hDlg, pAssocArguments paa)
{
    pExtRecords extRecs = paa->extensionRecords;

    if ( extRecs->cuNext == 0 )
    {
        pExtensionInfo *recs = allocExtensionInfoRecs(REQUIRED_PATH_EXT_COUNT, hDlg);
        if ( recs == NULL )
        {
            return false;
        }

        for ( size_t i = 0; i < REQUIRED_PATH_EXT_COUNT; i++ )
        {
            pExtensionInfo allUsersRec = getExtRecord(extRecs->allUsersRecs, extRecs->auNext, requiredPathExts[i]);
            pExtensionInfo current     = recs[i];

            if ( allUsersRec != NULL )
            {
                allUsersRec->curUser = true;
                current->allUsers    = true;
            }
            else
            {
                allUsersRec->curUser = false;
            }

            current->curUser  = true;
            current->exists   = true;
            strcpy(current->extension, requiredPathExts[i]);
            updateExtDisplayName(current);

            extRecs->curUserRecs[extRecs->cuNext++] = current;
        }
    }
    return true;
}

/**
 * Adds a extension info record to the specified array, increasing the array
 * size if needed.
 *
 * @param hDlg
 * @param rec
 * @param ra  Which record array we are concerned with.
 *
 * @remarks  For the PATHEXT, only the 'in scope' PATHEXT can be changed.  The
 *           'out of scope' array remains fixed.  We are going to assume the
 *           caller used the correct record array type.
 */
static bool addRecToArray(HWND hDlg, pExtensionInfo rec, recArrayType_t ra)
{
    pAssocArguments paa     = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);
    pExtRecords     extRecs = paa->extensionRecords;

    // Make sure there will be enough room in the array to begin with.
    if ( !ensureArraySize(hDlg, ra) )
    {
        return false;
    }

    switch ( ra )
    {
        case auPeRecArray :
            extRecs->allUsersRecs[extRecs->auNext++] = rec;
            break;

        case cuPeRecArray :
            extRecs->curUserRecs[extRecs->cuNext++] = rec;
            break;

        case suFaRecArray :
            extRecs->faSuggestedRecs[extRecs->faSuggestedNext++] = rec;
            break;

        case cuFaRecArray :
            extRecs->faCurrentRecs[extRecs->faCurrentNext++] = rec;
            break;

        default :
            return false;
    }
    return true;
}

/**
 * When a PATHEXT extension is deleted from or added to the PATHEXT list box, we
 * need to update the arrays that save the PATHEXT extension records.
 *
 * @param hDlg
 * @param ext
 * @param deleting
 *
 * @remarks  If we are deleting, the caller has already deleted the item from
 *           the PATHEXT list box.  If we are adding, we add the item to the
 *           list box, and it will go at the end of the record array.
 *
 *           The all users PATHEXT will never be empty.  But there might not be
 *           any PATHEXT for the current user.  We need to be sure the user does
 *           not add a current user PATHEXT that does not contain .exe, etc.. So
 *           we check to see if the current user PATHEXT is empty and add the
 *           required PATHEXT records if needed.
 */
static void updatePathExtArrays(HWND hDlg, pAssocArguments paa, pExtensionInfo rec, bool deleting)
{
    pExtRecords extRecs = paa->extensionRecords;
    HWND        hLB     = paa->lbPathExt;

    pExtensionInfo *inScope  = NULL;
    pExtensionInfo *outScope = NULL;
    size_t          count     = 0;
    size_t          next      = 0;

    pExtensionInfo info = NULL;

    if ( deleting )
    {
        if ( paa->allUsers )
        {
            extRecs->auNext--;
            savePathExtListBoxOrder(hDlg, paa);

            info = getExtRecord(extRecs->curUserRecs, extRecs->cuNext, rec->extension);
            if ( info != NULL && info->allUsers )
            {
                info->allUsers = false;
                updateExtDisplayName(info);
            }
        }
        else
        {
            extRecs->cuNext--;
            savePathExtListBoxOrder(hDlg, paa);

            info = getExtRecord(extRecs->allUsersRecs, extRecs->auNext, rec->extension);
            if ( info != NULL && info->curUser )
            {
                info->curUser = false;
                updateExtDisplayName(info);
            }
        }
    }
    else
    {
        // Be sure we don't lose the current order
        savePathExtListBoxOrder(hDlg, paa);

        // The record sent to us needs its fields updated.
        rec->allUsersOther = false;
        rec->curUserOther  = false;
        strUpper(rec->extension);

        recArrayType_t ra;
        if ( paa->allUsers )
        {
            rec->allUsers = true;
            ra            = auPeRecArray;

            info = getExtRecord(extRecs->curUserRecs, extRecs->cuNext, rec->extension);
            if ( info != NULL )
            {
                rec->curUser   = true;
                info->allUsers = true;
                updateExtDisplayName(info);
            }
        }
        else
        {
            rec->curUser = true;
            ra           = cuPeRecArray;

            ensureSafePathExt(hDlg, paa);

            info = getExtRecord(extRecs->allUsersRecs, extRecs->auNext, rec->extension);
            if ( info != NULL )
            {
                rec->allUsers = true;
                info->curUser = true;
                updateExtDisplayName(info);
            }
        }

        updateExtDisplayName(rec);
        addRecToArray(hDlg, rec, ra);

        SendMessage(hLB, LB_RESETCONTENT, 0, 0);
        fillPathExtListBox(hDlg, paa);
    }
}

/**
 * Determines if an extension already exists in the PATHEXT for the current
 * scope.
 *
 * @param paa
 * @param ext
 *
 * @return bool
 */
static bool pathExtExtExists(pAssocArguments paa, char *ext)
{
    pExtensionInfo *recArray = NULL;
    pExtRecords     extRecs  = paa->extensionRecords;
    size_t          next     = 0;

    if ( paa->allUsers )
    {
        recArray = extRecs->allUsersRecs;
        next     = extRecs->auNext;
    }
    else
    {
        recArray = extRecs->curUserRecs;
        next     = extRecs->cuNext;
    }

    return getExtRecord(recArray, next, ext) != NULL;
}

/**
 * Gets the individual extension records for the All Users and Current User
 * PATHEXT.  Unlike the file association records, these records do not change
 * during the life time of the dialog, so we only want to do this once.
 *
 * @param hDlg
 * @param paa
 */
static void getAddPathExtRecords(HWND hDlg, pAssocArguments paa)
{
    if ( paa->extensionRecords->auNext > 0 )
    {
        // A little insurance, we shouldn't be called in the first place.
        return;
    }

    char  *keyName = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
    HKEY   hKey    = HKEY_LOCAL_MACHINE;
    HKEY   hEnvKey;
    size_t allCnt = 0;
    size_t curCnt = 0;

    uint32_t maxValue = MAX_HKEY_VALUE;       // Size of value buffer
    char     value[MAX_HKEY_VALUE] = {'\0'};  // Buffer for value
    char     extraMsg[SMALL_BUF_SIZE];
    char     expanded[MAX_HKEY_VALUE];        // Buffer for expanded string.

    pExtensionInfo *recs     = NULL;
    pExtensionInfo *userRecs = NULL;
    pExtRecords     extRecs  = paa->extensionRecords;

    HWND hLB = paa->lbPathExt;

    uint32_t retCode = RegOpenKeyEx(hKey, keyName, 0, KEY_QUERY_VALUE, &hEnvKey);
    if ( retCode == ERROR_SUCCESS )
    {
        retCode = RegQueryValueEx(hEnvKey, "PATHEXT", NULL, NULL, (LPBYTE)value, (LPDWORD)&maxValue);
        if ( retCode == ERROR_SUCCESS )
        {
            uint32_t size = ExpandEnvironmentStrings(value, expanded, MAX_HKEY_VALUE);
            if ( size > MAX_HKEY_VALUE || size == 0 )
            {
                _snprintf(extraMsg, SMALL_BUF_SIZE, "PATHEXT listing may be incorrect");
                reportErrorPlus(hDlg, "ExpandEnvironmentStrings", REG_ERR_TITLE, extraMsg, GetLastError());

                StrNCpy(expanded, value, MAX_HKEY_VALUE - 1);
                expanded[MAX_HKEY_VALUE - 1] = '\0';
            }
            allCnt = countPathExt(expanded);
        }
        else
        {
            _snprintf(extraMsg, SMALL_BUF_SIZE, "Failed to query value of PATHEXT for all users.");
            reportErrorPlus(hDlg, "RegQueryValueEx", REG_ERR_TITLE, extraMsg, retCode);
        }

        RegCloseKey(hEnvKey);
    }
    else
    {
        _snprintf(extraMsg, SMALL_BUF_SIZE, "Failed to open the HKLM\\%s registry key.", keyName);
        reportErrorPlus(hDlg, "RegOpenKeyEx", REG_ERR_TITLE, extraMsg, retCode);
        goto done_out;
    }

    recs = allocExtensionInfoRecs(allCnt, hDlg);
    if ( recs == NULL )
    {
        goto done_out;
    }

    fillPathExtRecords(recs, expanded, allCnt, true);

    value[0] = '\0';
    maxValue = MAX_HKEY_VALUE;
    hKey     = HKEY_CURRENT_USER;

    retCode = RegOpenKeyEx(hKey, "Environment", 0, KEY_QUERY_VALUE, &hEnvKey);
    if ( retCode == ERROR_SUCCESS )
    {
        retCode = RegQueryValueEx(hEnvKey, "PATHEXT", NULL, NULL, (LPBYTE)value, (LPDWORD)&maxValue);
        if ( retCode == ERROR_SUCCESS )
        {
            uint32_t size = ExpandEnvironmentStrings(value, expanded, MAX_HKEY_VALUE);
            if ( size > MAX_HKEY_VALUE || size == 0 )
            {
                _snprintf(extraMsg, SMALL_BUF_SIZE, "PATHEXT listing for current user may be incorrect");
                reportErrorPlus(hDlg, "ExpandEnvironmentStrings", REG_ERR_TITLE, extraMsg, GetLastError());

                StrNCpy(expanded, value, MAX_HKEY_VALUE - 1);
                expanded[MAX_HKEY_VALUE - 1] = '\0';
            }
            curCnt = countPathExt(expanded);
        }
        else if ( retCode == ERROR_FILE_NOT_FOUND )
        {
            // This simply means the user didn't define a current user PATHEXT,
            // do nothing on purpose.
            ;
        }
        else
        {
            _snprintf(extraMsg, SMALL_BUF_SIZE, "Failed to query value of PathEXT.");
            reportErrorPlus(hDlg, "RegQueryValueEx", REG_ERR_TITLE, extraMsg, retCode);
        }
        RegCloseKey(hEnvKey);
    }
    else
    {
        _snprintf(extraMsg, SMALL_BUF_SIZE, "Failed to open the HKCU\\%s registry key.", "Environment");
        reportErrorPlus(hDlg, "RegOpenKeyEx", REG_ERR_TITLE, extraMsg, retCode);
        goto err_out;
    }

    if ( curCnt > 0 )
    {
        userRecs = allocExtensionInfoRecs(curCnt, hDlg);
        if ( userRecs == NULL )
        {
            goto err_out;
        }
        fillPathExtRecords(userRecs, expanded, curCnt, false);
    }

    // First we need to fix up the display name for all records in each array.
    for ( size_t i = 0; i < allCnt; i++)
    {
        pExtensionInfo current = recs[i];

        if ( curCnt > 0 )
        {
            int32_t idx = getExtRecordIndex(userRecs, curCnt, current->extension);
            if ( idx >= 0 )
            {
                current->curUser = true;
            }
        }
        updateExtDisplayName(current);
    }

    if ( curCnt > 0 )
    {
        for ( size_t i = 0; i < curCnt; i++)
        {
            pExtensionInfo current = userRecs[i];

            int32_t idx = getExtRecordIndex(recs, allCnt, current->extension);
            if ( idx >= 0 )
            {
                current->allUsers = true;
            }
            updateExtDisplayName(current);
        }
    }

    // Check that the  arrays are big enough.
    if ( (! ensureArraySize(hDlg, auPeRecArray, allCnt)) || (! ensureArraySize(hDlg, cuPeRecArray, curCnt)) )
    {
        endThisDialog(hDlg, NULL);
        return;
    }

    // Then save the two record arrays and associated info.
    for ( size_t i = 0; i < allCnt; i++ )
    {
        extRecs->allUsersRecs[i] = recs[i];
    }
    extRecs->auNext = allCnt;

    for ( size_t i = 0; i < curCnt; i++ )
    {
        extRecs->curUserRecs[i] = userRecs[i];
    }
    extRecs->cuNext = curCnt;

    safeLocalFree(recs);
    safeLocalFree(userRecs);

    // Finally fill the list box with the proper records according to mode
    fillPathExtListBox(hDlg, paa);

    goto done_out;

err_out:
    if ( recs != NULL )
    {
        for ( size_t i = 0; i < allCnt; i++ )
        {
            safeLocalFree(recs[i]);
        }
        safeLocalFree(recs);
    }

done_out:
    return;
}

/**
 * Determines if our ooDialog file type is registered, and if so if it is
 * registered for all users and if it is registered for the current user.
 *
 * @param paa
 */
static void checkRegistration(pAssocArguments paa)
{
    char     value[MAX_HKEY_VALUE] = {'\0'};
    uint32_t maxValue              = MAX_HKEY_VALUE;
    HKEY     hKey, hSubkey;

    paa->ftypeIsRegistered  = false;
    paa->registeredAllUsers = false;
    paa->registeredCurUsers = false;

    // See if our progID is a subkey of classes root
    if( RegOpenKeyEx(HKEY_CLASSES_ROOT, paa->progID, 0, KEY_QUERY_VALUE , &hKey) == ERROR_SUCCESS )
    {
        paa->ftypeIsRegistered = true;

        char buf[MEDIUM_BUF_SIZE];
        _snprintf(buf, MEDIUM_BUF_SIZE, "SOFTWARE\\Classes\\%s", paa->progID);

        // See if progID is a subkey for local machine
        if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, buf, 0, KEY_QUERY_VALUE, &hSubkey) == ERROR_SUCCESS )
        {
            paa->registeredAllUsers = true;
            RegCloseKey(hSubkey);
        }

        // See if progID is a subkey for current user
        if( RegOpenKeyEx(HKEY_CURRENT_USER, buf, 0, KEY_QUERY_VALUE, &hSubkey) == ERROR_SUCCESS )
        {
            paa->registeredCurUsers = true;
            RegCloseKey(hSubkey);
        }

        RegCloseKey(hKey);
    }
}

/**
 * In a list box you can't simply change the displayed name of an item.  The
 * item has to be deleted and a new string inserted.
 *
 * In much of this application we simply empty the list box and then refill it
 * from the record arrays.  This ensures that the item index in the list box
 * matches the index of the extension info record in its array.
 *
 * Sometimes though, all we want is to change the item string displayed.  This
 * function handles that.
 *
 * @param paa
 * @param rec
 * @param index
 * @param ra
 */
static void updateDisplayNameInPlace(pAssocArguments paa, pExtensionInfo rec, uint32_t index, recArrayType_t ra)
{
    HWND  hLB;

    switch ( ra )
    {
        case auPeRecArray :
        case cuPeRecArray :
            hLB = paa->lbPathExt;
            break;

        case suFaRecArray :
            hLB = paa->lbSuggested;
            break;

        case cuFaRecArray :
            hLB = paa->lbCurrent;
            break;
    }

    char *display = isSingleMode(hLB) ? rec->displayName2 : rec->displayName;

    SendMessage(hLB, LB_DELETESTRING, index, 0);
    SendMessage(hLB, LB_ADDSTRING, 0, (LPARAM)display);
    SendMessage(hLB, LB_SETITEMDATA, index, (LPARAM)rec);
}

/**
 * Retrieves the assumed extension from an edit control, allocates an extension
 * info record for it, and returns it.
 *
 * @param hDlg
 * @param edit
 *
 * @return A pExtensionInfo record on success and null on error.
 */
static pExtensionInfo getExtRecordFromEditBox(HWND hDlg, HWND edit)
{
    char           text[MAX_EXT_NAME];
    int32_t        count = MAX_EXT_NAME;
    pExtensionInfo info = NULL;

    count = GetWindowText(edit, text, count);
    if ( count == 0 )
    {
        uint32_t rc = GetLastError();
        if ( rc != 0 )
        {
            reportError(hDlg, "GetWindowText", OS_ERR_TITLE, rc);
        }
        goto done_out;
    }

    info = (pExtensionInfo)LocalAlloc(LPTR, sizeof(extensionInfo));
    if ( info == NULL )
    {
        reportError(hDlg, OUT_OF_MEMORY_ERR_FMT, OS_ERR_TITLE, "LocalAlloc", GetLastError());
        goto done_out;
    }
    if ( *text != '.' )
    {
        strcpy(info->extension, ".");
    }
    strcat(info->extension, text);

done_out:
    return info;
}

/**
 * Resets the extension info record array specified by ra and removes all items
 * from the associated list box.
 *
 * @param paa
 * @param ra
 *
 * @assumes We are only called for the Suggested File Association or the Current
 *          File Association list boxes.
 *
 * @note  We just invoke resetExtRecords() unconditionally, if there are no
 *        records the function will just return.
 */
static void maybeReset(pAssocArguments paa, recArrayType_t ra)
{
    resetExtRecords(paa->extensionRecords, ra);

    SendMessage(ra == suFaRecArray ? paa->lbSuggested : paa->lbCurrent, LB_RESETCONTENT, 0, 0);
}

/**
 * Removes the extension info record specified by its index from the suggested
 * list box and the suggested records array.
 *
 * @param hDlg
 * @param index
 *
 * @return The removed record.
 *
 * @note  The removed record must be freed by the caller.
 */
static pExtensionInfo removeFromSuggested(HWND hDlg, uint32_t index, int32_t arrayIndex)
{
    pAssocArguments paa     = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);
    pExtRecords     extRecs = paa->extensionRecords;

    pExtensionInfo tempRec = extRecs->faSuggestedRecs[arrayIndex];

    if ( arrayIndex + 1 == extRecs->faSuggestedNext )
    {
        // The record being remove is the last entry in the array, just set it
        // to null.
        extRecs->faSuggestedRecs[arrayIndex] = NULL;
    }
    else
    {
        // The record being remove is not the last entry.  Move the entries
        // following the removed entry to the one being removed.
        memmove(&extRecs->faSuggestedRecs[arrayIndex],
                &extRecs->faSuggestedRecs[arrayIndex + 1],
                (extRecs->faSuggestedNext - arrayIndex + 1) * sizeof(pExtensionInfo));

        extRecs->faSuggestedRecs[extRecs->faSuggestedNext - 1] = NULL;
    }
    extRecs->faSuggestedNext--;

    SendMessage(paa->lbSuggested, LB_DELETESTRING, index, 0);

    return tempRec;
}

/**
 * Removes the extension info record specified by its index from the current
 * list box and the current records array.
 *
 * @param hDlg
 * @param index
 *
 * @return The removed record.
 *
 * @note  The removed record must be freed by the caller.
 */
static pExtensionInfo removeFromCurrent(HWND hDlg, uint32_t index, int32_t arrayIndex)
{
    pAssocArguments paa     = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);
    pExtRecords     extRecs = paa->extensionRecords;

    pExtensionInfo tempRec = extRecs->faCurrentRecs[arrayIndex];

    if ( arrayIndex + 1 == extRecs->faCurrentNext )
    {
        // The record being remove is the last entry in the array, just set it
        // to null.
        extRecs->faCurrentRecs[arrayIndex] = NULL;
    }
    else
    {
        // The record being remove is not the last entry.  Move the entries
        // following the removed entry to the one being removed.
        memmove(&extRecs->faCurrentRecs[arrayIndex],
                &extRecs->faCurrentRecs[arrayIndex + 1],
                (extRecs->faCurrentNext - arrayIndex + 1) * sizeof(pExtensionInfo));

        extRecs->faCurrentRecs[extRecs->faCurrentNext - 1] = NULL;
    }
    extRecs->faCurrentNext--;

    SendMessage(paa->lbCurrent, LB_DELETESTRING, index, 0);

    return tempRec;
}

/**
 * Adds the suggested file extensions to the list box.
 *
 * The first time through, the list box will be empty.  But, we may do this
 * several times if the user adds and / or removes the registration for the file
 * type.  Rather than try to updated existing records on succesive invocations,
 * we just reset everything and start over.
 *
 * For the suggested extensions, we may end up with 2 records.  If the extension
 * is not registered in this scope, then we want it to be an item in the
 * suggested list box so the user can register it.  If it is registered in this
 * scope then we do not want the item in the suggested list box, but do want it
 * in the current list box.
 *
 * We handle not putting it in the suggested list box here, and handle putting
 * it in the current list box in addCurrentExt().
 *
 * @param hDlg
 * @param paa
 *
 * @return bool
 *
 * @remarks  The suggeste record array is always big enough to hold all the
 *           suggested extensions.  Since we reset that array to begin with, we
 *           do not need to worry about the size of the array here.
 */
static bool addSuggestedExt(HWND hDlg, pAssocArguments paa)
{

    maybeReset(paa, suFaRecArray);

    for ( size_t i = 0; i < OOD_SUGGESTED_EXT_COUNT; i++ )
    {
        pExtensionInfo info = (pExtensionInfo)LocalAlloc(LPTR, sizeof(extensionInfo));

        if ( info == NULL )
        {
            reportError(hDlg, OUT_OF_MEMORY_ERR_FMT, OS_ERR_TITLE, "LocalAlloc", GetLastError());
            return false;
        }

        strcpy(info->extension, oodSuggestedExts[i]);
        info->suggested = true;

        qualifyExtensionInfo(oodSuggestedExts[i], info, paa->progID);

        if ( extRegisteredInScope(paa, info) )
        {
            LocalFree(info);
        }
        else
        {
            pExtRecords extRecs = paa->extensionRecords;
            HWND        hLB     = paa->lbSuggested;
            char       *display = isSingleMode(hDlg) ? info->displayName2 : info->displayName;

            extRecs->faSuggestedRecs[extRecs->faSuggestedNext++] = info;

            LRESULT index = SendMessage(hLB, LB_ADDSTRING, 0, (LPARAM)display);
            SendMessage(hLB, LB_SETITEMDATA, index, (LPARAM)info);
        }
    }
    return true;
}

/**
 * Fills the current extensions list box.
 *
 * This should always be called after the suggested extensions list box is
 * filled, because if a currently registered extension matches one of the
 * suggested extensions we remove the suggested extension from its list box and
 * use its record.
 *
 * @param hDlg
 * @param paa            NOTE redo comments
 *
 * @note  When we get the extension records we get a record for every .ext
 *        registered to our prog ID.
 *
 *        As we go through the records, if the .ext is a suggested extension, we
 *        want to put it in the current list box only if it is registered in the
 *        current scope.  If it is registered in the other scope, a record in
 *        the suggested list box already exists.  (Provided we stick to filling
 *        the suggested list box first.)
 *
 *        If the .ext is not a suggested extension, if the .ext is registered in
 *        this scope we put it in the current list box.  If it is not registered
 *        in this scope, then it is registered in the other scope, which makes
 *        it a good candidate for a 'suggestion', so we put it in the suggested
 *        list box.
 */
static void addCurrentExt(HWND hDlg, pAssocArguments paa)
{
    HWND   hLB   = paa->lbCurrent;

    maybeReset(paa, cuFaRecArray);

    size_t          count = 0;
    pExtensionInfo *recs  = getExtensionRecords(hDlg, paa->progID, &count);

    if ( ! ensureArraySize(hDlg, cuFaRecArray, count) )
    {
        endThisDialog(hDlg, NULL);
        return;
    }

    for ( size_t i = 0; i < count; i++ )
    {
        pExtensionInfo current = recs[i];

        pExtRecords extRecs = paa->extensionRecords;
        HWND        hLB     = paa->lbCurrent;
        char       *display = isSingleMode(hDlg) ? current->displayName2 : current->displayName;

        extRecs->faCurrentRecs[extRecs->faCurrentNext++] = current;

        LRESULT index = SendMessage(hLB, LB_ADDSTRING, 0, (LPARAM)display);
        SendMessage(hLB, LB_SETITEMDATA, index, (LPARAM)current);
    }
}

/**
 * Fills the PATHEXT list box with its items.
 *
 * In the first version of this dialog we upated the PATHEXT list box each time
 * the registered / unregistered state changed, like we do for the file
 * association list boxes. However since any changes to PATHEXT are not done
 * until the dialog exits, that is not necessary.
 *
 * That makes this is an unneeded intermediate step, but it is being retained in
 * case we need to go back to the original logic.
 *
 * @param hDlg
 * @param paa
 */
static void addPathExt(HWND hDlg, pAssocArguments paa)
{
    if ( paa->extensionRecords->auNext == 0 )
    {
        getAddPathExtRecords(hDlg, paa);
    }
}

/**
 * Used to enable or disable the controls related to associating an .ext with
 * our ooDialg file type.
 *
 * If the file type is not registered in any scope, then no .ext can be
 * associated.  To make it easy for the user to understand this, we disable all
 * the controls related to associating an .ext.  Then, when the user registers
 * our file type we enable the controls.
 *
 * @param hDlg
 * @param paa
 * @param flag
 */
static void setAssocControls(HWND hDlg, pAssocArguments paa, bool flag)
{
    BOOL enable = flag ? TRUE : FALSE;

    EnableWindow(paa->lbCurrent, enable);
    EnableWindow(paa->lbSuggested, enable);
    EnableWindow(paa->lbPathExt, enable);

    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_FA_EXT), enable);
    EnableWindow(GetDlgItem(hDlg, IDC_PB_ADD_FA_EXT), enable);
    EnableWindow(GetDlgItem(hDlg, IDC_PB_ADD_CURRENT), enable);
    EnableWindow(GetDlgItem(hDlg, IDC_PB_REMOVE_CURRENT), enable);
    EnableWindow(GetDlgItem(hDlg, IDC_PB_ADD_PATHEXT), enable);
}


/**
 * Sets the state of the dialog controls.
 *
 * @param hDlg
 * @param paa
 *
 * @note This is a little complex.  At this time I think the Register push
 *       button should always be enabled.
 */
static void setRegisteredState(HWND hDlg, pAssocArguments paa)
{
    // The static text for these controls can vary during the life-time of the
    // dialog:
    SetDlgItemText(hDlg, IDC_ST_REGISTERED, paa->ftypeIsRegistered ? "True" : "False");
    SetDlgItemText(hDlg, IDC_ST_REGALL, paa->registeredAllUsers ? "Yes" : "No");
    SetDlgItemText(hDlg, IDC_ST_REGCURRENT, paa->registeredCurUsers ? "Yes" : "No");

    if ( ! paa->ftypeIsRegistered )
    {
        if ( paa->allUsers )
        {
            SetWindowText(paa->pbRegister, "Register ooDialog File Type For All Users");
        }
        else
        {
            SetWindowText(paa->pbRegister, "Register ooDialog File Type For the Current User");
        }

        setAssocControls(hDlg, paa, false);
        addSuggestedExt(hDlg, paa);
        addCurrentExt(hDlg, paa);
        addPathExt(hDlg, paa);
        return;
    }

    // Our progID is registered for either all users or current user.  Maybe
    // both, but not necessarily both.  How we set the state is dependent on the
    // scope and if progID is registered for that scope.
    if ( paa->registeredAllUsers && paa->registeredCurUsers )
    {
        if ( paa->allUsers )
        {
            SetWindowText(paa->pbRegister, "Un-register ooDialog File Type For All Users");
        }
        else
        {
            SetWindowText(paa->pbRegister, "Un-register ooDialog File Type For the Current User");
        }
    }
    else if ( paa->registeredAllUsers )
    {
        if ( paa->allUsers )
        {
            SetWindowText(paa->pbRegister, "Un-register ooDialog File Type For All Users");
        }
        else
        {
            SetWindowText(paa->pbRegister, "Register ooDialog File Type For the Current User");
        }
    }
    else
    {
        if ( paa->allUsers )
        {
            SetWindowText(paa->pbRegister, "Register ooDialog File Type For All Users");
        }
        else
        {
            SetWindowText(paa->pbRegister, "Un-register ooDialog File Type For the Current User");
        }
    }

    setAssocControls(hDlg, paa, true);
    addSuggestedExt(hDlg, paa);
    addCurrentExt(hDlg, paa);
    addPathExt(hDlg, paa);
}

/**
 * Sets the text for the static text controls that does not change for the life
 * time of the dialog.
 *
 * For these controls, we set the text on start up and then do not have to touch
 * them again.
 *
 * @param hDlg
 * @param paa
 */
static void setStaticText(HWND hDlg, pAssocArguments paa)
{
    SetDlgItemText(hDlg, IDC_GB_ASSOCIATE, paa->allUsers ?
                   "Configuring File Associations and the PATHEXT for:  All Users" :
                   "Configuring File Associations and the PATHEXT for:  the Current User");

    SetDlgItemText(hDlg, IDC_GB_PATHEXT, paa->allUsers ?
                   "Configure PATHEXT for:  All Users" :
                   "Configure PATHEXT for:  the Current User");

    SetDlgItemText(hDlg, IDC_ST_FTYPE, paa->friendlyName);
    SetDlgItemText(hDlg, IDC_ST_SCOPE, paa->allUsers ? "All Users" : "Current User");
    SetDlgItemText(hDlg, IDC_ST_RUNAS, paa->isRunAsAdmin ? "True" : "False");
    SetDlgItemText(hDlg, IDC_ST_ELEVATED, paa->isElevated ? "True" : "False");
    SetDlgItemText(hDlg, IDC_ST_PATHEXT_DETAILS, PATHEXT_DETAILS);
}

/**
 * Deletes our ooDialog ProgID from the registry.
 *
 * @param hDlg
 * @param paa
 *
 * @return bool
 */
static bool deleteRegProgID(HWND hDlg, pAssocArguments paa)
{
    char *user;
    HKEY hPreDefKey = getScopeVars(paa, & user);

    char valueBuf[MEDIUM_BUF_SIZE];
    _snprintf(valueBuf, MEDIUM_BUF_SIZE, "SOFTWARE\\Classes\\%s", paa->progID);

    uint32_t rc = SHDeleteKey(hPreDefKey, valueBuf);
    if ( rc != ERROR_SUCCESS )
    {
        char extraMsg[SMALL_BUF_SIZE];

        _snprintf(extraMsg, SMALL_BUF_SIZE, "Failed to delete the %s key for %s", valueBuf, user);

        reportErrorPlus(hDlg, "SHDeleteKey", REG_ERR_TITLE, extraMsg, rc);
        return false;
    }
    return true;
}

/**
 * Deletes all extension (.xxx) subkeys for our ooDialog file type (prog ID)
 * within the scope (all users / current user) we are under.
 *
 * @param hDlg
 * @param paa
 *
 * @return bool
 *
 * @note  When querying a subkey value, I have had a problem with file not found
 *        error.  This seems to be when the default value for the subkey is not
 *        set.  So, I ignore that specific error.  In getExtensionRecords() we
 *        just ignore all errors and that might be what we should do here.
 *        After all, if we can't query the default value for an extension
 *        subkey, it was probably not set by us.
 *
 *        If we enumerate through the .ext keys and check each one for our prog
 *        ID, and delete it on a match, the index gets screwed up and we fail to
 *        delete extensions if there are more than one.
 *
 *        Theoretically, all registered extension will be in our current list
 *        box.  So what we do, is enumerate the list box items and delete each
 *        extension that is is registered in our scope.  This seems to work
 *        well, but it is dependent on the list box items being accurate.
 *
 *        The other approach would be to enumerate all subkeys and keep track of
 *        our matches.  The current approach is simplier and we'll use it unless
 *        we run into problems.
 */
static bool deleteRegAllExt(HWND hDlg, pAssocArguments paa)
{
    char     *user;
    HKEY      hPreDefKey = getScopeVars(paa, &user);
    uint32_t  rc = 0;
    HKEY      hClassesKey;
    char      extraMsg[SMALL_BUF_SIZE];

    rc = RegOpenKeyEx(hPreDefKey, "SOFTWARE\\Classes\\", 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | DELETE,
                      &hClassesKey);
    if ( rc != ERROR_SUCCESS )
    {
        _snprintf(extraMsg, SMALL_BUF_SIZE, "Failed to open the %s registry key for %s", "SOFTWARE\\Classes\\", user);

        reportErrorPlus(hDlg, "RegOpenKeyEx", REG_ERR_TITLE, extraMsg, rc);
        return false;
    }

    bool   success = true;
    size_t count   = 0;

    char **names = getAllExtensionNames(hDlg, paa, &count);
    if ( names == NULL )
    {
        RegCloseKey(hClassesKey);
        return false;
    }

    for ( size_t i = 0; i < count; i++ )
    {
        rc = RegDeleteKey(hClassesKey, names[i]);
        if ( rc != ERROR_SUCCESS )
        {
            _snprintf(extraMsg, SMALL_BUF_SIZE,
                      "Failed to delete the SOFTWARE\\Classes\\%s registry subkey for %s",
                      names[i], user);

            reportErrorPlus(hDlg, "RegDeleteKey", REG_ERR_TITLE, extraMsg, rc);
            success = false;
        }
        safeLocalFree(names[i]);
    }

    safeLocalFree(names);
    RegCloseKey(hClassesKey);
    return success;
}

/**
 * Creates, or opens, a subkey and, optionally, writes its Default string value.
 *
 * @param openedKey  An already open registry key
 * @param subkey     A subkey of 'openedKey to create or open
 * @param defValue   The value for the Default entry for subkey.  If this is
 *                   null, then writing a value for Default is just skipped.
 *
 * @param hDlg       The window handle of the dialog, for error displary
 * @param user       A string for the error dispaly that specifies if we are
 *                   writing the subkey in current user or all users (local
 *                   machine.)
 *
 * @return The opened subkey on success, otherwise NULL.
 *
 * @note  There may be one problem here, if we create the key, but fail to set
 *        the default value, we don't clean up the created key.  TODO  We could
 *        check the disposition and if was created, we could delete the key if
 *        we fail to set the default value.
 */
static HKEY writeRegSubkeyValue(HKEY openedKey, char *subkey, char *defValue, HWND hDlg, char *user)
{
    uint32_t disp;
    uint32_t rc;
    char     extraMsg[SMALL_BUF_SIZE];
    HKEY     hKey = NULL;

    rc = RegCreateKeyEx(openedKey, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | DELETE, NULL, &hKey, (LPDWORD)&disp);
    if ( rc != ERROR_SUCCESS )
    {
        _snprintf(extraMsg, SMALL_BUF_SIZE, "Failed to create or open the %s registry key for %s", subkey, user);

        reportErrorPlus(hDlg, "RegCreateKeyEx", REG_ERR_TITLE, extraMsg, rc);
        return NULL;
    }

    // Set the default value, if requested ...
    if ( defValue != NULL )
    {
        size_t  len  = strlen(defValue) + 1;

        rc = RegSetValueEx(hKey, "", 0, REG_SZ, (LPBYTE)defValue, (uint32_t)len);
        if ( rc != ERROR_SUCCESS )
        {
            _snprintf(extraMsg, SMALL_BUF_SIZE,
                      "Failed to set the default value (%s) for the %s registry key for %s",
                      defValue, subkey, user);

            reportErrorPlus(hDlg, "RegSetValueEx", REG_ERR_TITLE, extraMsg, rc);

            RegCloseKey(hKey);
            hKey = NULL;
        }
    }
    return hKey;
}

/**
 * Writes a named string value for an opened registry key.
 *
 * @param openedKey  An already open registry key
 * @param valueName  The name of the value to be written
 * @param value      The value to be written
 * @param hDlg       The window handle of the dialog, for error displary
 * @param user       A string for the error dispaly that specifies if we are
 *                   writing the subkey in current user or all users (local
 *                   machine.)
 *
 * @return True on success, false on error.
 */
static bool writeRegKeyValue(HKEY openedKey, char *valueName, char *value, HWND hDlg, char *user)
{
    uint32_t rc;
    size_t  len = strlen(value) + 1;

    rc = RegSetValueEx(openedKey, valueName, 0, REG_SZ, (LPBYTE)value, (uint32_t)len);
    if ( rc != ERROR_SUCCESS )
    {
        char extraMsg[SMALL_BUF_SIZE];

        _snprintf(extraMsg, SMALL_BUF_SIZE,
                  "Failed to set the %s value (%s) for %s",
                  valueName, value, user);

        reportErrorPlus(hDlg, "RegSetValueEx", REG_ERR_TITLE, extraMsg, rc);

        return false;
    }
    return true;
}

/**
 * Deletes (un-registers) the specified extension in our current scope.
 *
 * @param hDlg
 * @param paa
 * @param extension
 *
 * @return bool
 *
 * @note  We do no checking - it is the caller's responsibility to ensure the
 *        .ext is actually registered in our scope.  If the extension is not
 *        registered, it probably would do no harm, but it should generate a
 *        warning that would be confusing to the user.
 */
static bool deleteRegExt(HWND hDlg, pAssocArguments paa, CSTRING extension)
{
    char *user;
    HKEY  hPreDefKey = getScopeVars(paa, & user);
    HKEY  hClassesKey;
    char  extraMsg[SMALL_BUF_SIZE];

    // Open the Software\Classes key.  It seems that we should be adding the
    // DELETE flag, but this seems to work okay.
    uint32_t rc = RegOpenKeyEx(hPreDefKey, "SOFTWARE\\Classes\\", 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hClassesKey);
    if ( rc != ERROR_SUCCESS )
    {
        _snprintf(extraMsg, SMALL_BUF_SIZE, "Failed to open the %s registry key for %s", "SOFTWARE\\Classes\\", user);

        reportErrorPlus(hDlg, "RegOpenKeyEx", REG_ERR_TITLE, extraMsg, rc);
        return false;
    }

    // Delete the key.
    rc = RegDeleteKey(hClassesKey, extension);
    if ( rc != ERROR_SUCCESS )
    {
        _snprintf(extraMsg, SMALL_BUF_SIZE,
                  "Failed to delete the SOFTWARE\\Classes\\%s registry subkey for %s", extension, user);

        reportErrorPlus(hDlg, "RegDeleteKey", REG_ERR_TITLE, extraMsg, rc);
        return false;
    }

    return true;
}

static bool registerRegExt(HWND hDlg, pAssocArguments paa, CSTRING extension)
{
    char *user;
    HKEY hPreDefKey = getScopeVars(paa, & user);
    HKEY hKey       = NULL;
    char subkeyName[MEDIUM_BUF_SIZE];

    // Create, or overwrite, the .ext subkey of software\classees, and set the
    // default value to our prog ID
    _snprintf(subkeyName, MEDIUM_BUF_SIZE, "SOFTWARE\\Classes\\%s", extension);

    hKey = writeRegSubkeyValue(hPreDefKey, subkeyName, paa->progID, hDlg, user);
    if ( hKey == NULL )
    {
        return false;
    }

    return true;
}

// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.ood

// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.rxd

/**
 * Removes the ooDialog ftype and (optionally ?) its associated extensions
 *
 * @param hDlg
 * @param paa
 *
 * @return bool
 *
 * @note  TODO add code to check if the user wants the extensions removed.  For
 *        now we just do it. Possibly add code, or another function, to
 *        completely clean up the registry stuff that was, or may have been
 *        added by us.
 *
 * @note  We need to be sure to remove the extensions (if we are going to)
 *        before we remove the ftype (progID).  If we remove the progID first
 *        and an error happens removing the extension, we end up with a problem
 *        with any extension not removed where the registry refuses to open that
 *        extension.
 */
static bool removeProgID(HWND hDlg, pAssocArguments paa)
{
    if ( ! deleteRegAllExt(hDlg, paa) )
    {
        return false;
    }

    if ( ! deleteRegProgID(hDlg, paa) )
    {
        return false;
    }

    return true;
}

/**
 * Writes a new PATHEXT value to the registry.  Which registry key is written is
 * based on the current scope.
 *
 * @param hDlg
 * @param paa
 * @param val
 */
static void writeNewPathExt(HWND hDlg, pAssocArguments paa, char *val)
{
    char  *keyName = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
    char  *user;
    HKEY   hKey = getScopeVars(paa, & user);
    HKEY   hEnvKey;
    char   extraMsg[SMALL_BUF_SIZE];

    if ( hKey == HKEY_CURRENT_USER )
    {
        keyName = "Environment";
    }

    uint32_t retCode = RegOpenKeyEx(hKey, keyName, 0, KEY_WRITE, &hEnvKey);
    if ( retCode == ERROR_SUCCESS )
    {
        retCode = RegSetValueEx(hEnvKey, "PATHEXT", 0, REG_EXPAND_SZ, (LPBYTE)val, (uint32_t)(strlen(val) + 1));
        if ( retCode != ERROR_SUCCESS )
        {
            _snprintf(extraMsg, SMALL_BUF_SIZE,
                      "Failed to set the %s value (%s) for %s",
                      "PATHEXT", val, user);

            reportErrorPlus(hDlg, "RegSetValueEx", REG_ERR_TITLE, extraMsg, retCode);
        }

        RegCloseKey(hEnvKey);
        if ( retCode == ERROR_SUCCESS )
        {
            DWORD_PTR result;
            SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_NORMAL, 10000, &result);
        }
    }
    else
    {
        _snprintf(extraMsg, SMALL_BUF_SIZE, "Failed to open the %s\\%s registry key for writing.",
                  hKey == HKEY_LOCAL_MACHINE ? "HKLM" : "HKCU", keyName);
        reportErrorPlus(hDlg, "RegOpenKeyEx", REG_ERR_TITLE, extraMsg, retCode);
    }

    return;
}

/**
 * Removes the PATHEXT value for the Current User.
 *
 * @param hDlg
 * @param paa
 * @param val
 *
 * @assumes The caller has ensured that the scope is correct.
 */
static bool removePathExt(HWND hDlg, pAssocArguments paa)
{
    char  *keyName = "Environment";
    char  *user    = "the Current User.";
    HKEY   hKey    = HKEY_CURRENT_USER;
    HKEY   hEnvKey;
    char   extraMsg[SMALL_BUF_SIZE];

    if ( hKey == HKEY_CURRENT_USER )
    {
        keyName = "Environment";
    }

    uint32_t retCode = RegOpenKeyEx(hKey, keyName, 0, KEY_SET_VALUE, &hEnvKey);
    if ( retCode == ERROR_SUCCESS )
    {
        retCode = RegDeleteValue(hEnvKey, "PATHEXT");
        if ( retCode != ERROR_SUCCESS )
        {
            _snprintf(extraMsg, SMALL_BUF_SIZE,
                      "Failed to remove the %s value for %s", "PATHEXT", user);

            reportErrorPlus(hDlg, "RegDeleteValue", REG_ERR_TITLE, extraMsg, retCode);
            return false;
        }

        RegCloseKey(hEnvKey);
        if ( retCode == ERROR_SUCCESS )
        {
            DWORD_PTR result;
            SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_NORMAL, 10000, &result);
        }
    }
    else
    {
        _snprintf(extraMsg, SMALL_BUF_SIZE, "Failed to open the %s\\%s registry key for setting a value.",
                  hKey == HKEY_LOCAL_MACHINE ? "HKLM" : "HKCU", keyName);
        reportErrorPlus(hDlg, "RegOpenKeyEx", REG_ERR_TITLE, extraMsg, retCode);
        return false;
    }

    return true;
}

/**
 * Handles removing the Current User PATHEXT variable.  Asks the user if it is
 * okay before doing it.
 *
 * @param hDlg
 * @param paa
 *
 * @return intptr_t
 *
 * @assumes the caller has insured the scope is the Current User.
 */
static intptr_t removingPathExtForCurrentUser(HWND hDlg, pAssocArguments paa)
{
    char msg[MEDIUM_BUF_SIZE];
    _snprintf(msg, SMALL_BUF_SIZE, "The PATHEXT variable for the Current User will be removed completely.\n\n"
                                   "Continue with this operations?");

    int32_t reply = MessageBox(hDlg, msg, "Removing PATHEXT for the Current User",
                               MB_YESNOCANCEL | MB_ICONINFORMATION | MB_DEFBUTTON3);
    if ( reply == IDYES )
    {
        removePathExt(hDlg, paa);
    }
    else if ( reply == IDCANCEL )
    {
        return FALSE;
    }
    return TRUE;
}

/**
 * Adds our ooDialog ProgID to the registry
 *
 * @author Administrator (11/12/2013)
 *
 * @param hDlg
 * @param paa
 *
 * @return bool
 */
static bool addProgID(HWND hDlg, pAssocArguments paa)
{
    char *user;
    HKEY hPreDefKey = getScopeVars(paa, & user);
    HKEY hProgIdKey = NULL;
    HKEY hKey       = NULL;
    char valueBuf[MEDIUM_BUF_SIZE];

    // Do the ooRexx.ooDialog.1 subkey of software\classees
    _snprintf(valueBuf, MEDIUM_BUF_SIZE, "SOFTWARE\\Classes\\%s", paa->progID);

    hProgIdKey = writeRegSubkeyValue(hPreDefKey, valueBuf, OODIALOG_FRIENDLY_NAME, hDlg, user);
    if ( hProgIdKey == NULL )
    {
        goto done_out;
    }

    // Do the DefaultIcon subkey of ooRexx.ooDialog.1
    _snprintf(valueBuf, MEDIUM_BUF_SIZE, "%s,-%d", paa->exeName, IDI_APP_BLUE_BEAKER_ICON);

    hKey = writeRegSubkeyValue(hProgIdKey, "DefaultIcon", valueBuf, hDlg, user);
    if ( hKey == NULL )
    {
        goto done_out;
    }
    RegCloseKey(hKey);

    // Do the CurVer subkey of ooRexx.ooDilaog.1
    hKey = writeRegSubkeyValue(hProgIdKey, "CurVer", paa->progID, hDlg, user);
    if ( hKey == NULL )
    {
        goto done_out;
    }
    RegCloseKey(hKey);

    // Do the FiendlyTypeName value of ooRexx.ooDialog.1
    _snprintf(valueBuf, MEDIUM_BUF_SIZE, "@%s,-%d", paa->exeName, IDS_FRIENDLY_NAME);

    if ( ! writeRegKeyValue(hProgIdKey, "FriendlyTypeName", valueBuf, hDlg, user) )
    {
        goto done_out;
    }

    // Do the InfoTip value of ooRexx.ooDailog.1
    _snprintf(valueBuf, MEDIUM_BUF_SIZE, "@%s,-%d", paa->exeName, IDS_INFOTIP);

    if ( ! writeRegKeyValue(hProgIdKey, "InfoTip", valueBuf, hDlg, user) )
    {
        goto done_out;
    }

    // Do the shell subkey of ooRexx.ooDiallog.1 and its subkeys.
    // This will create the shell\open sub subky and set its default to Run
    hKey = writeRegSubkeyValue(hProgIdKey, "shell\\open", "Run", hDlg, user);
    if ( hKey == NULL )
    {
        goto done_out;
    }
    RegCloseKey(hKey);

    // Now create the shell\open\command sub sub subkey
    _snprintf(valueBuf, MEDIUM_BUF_SIZE, "\"%s\" \"%%1\" %%*", paa->exeName);

    hKey = writeRegSubkeyValue(hProgIdKey, "shell\\open\\command", valueBuf, hDlg, user);
    if ( hKey == NULL )
    {
        goto done_out;
    }
    RegCloseKey(hKey);

    /* TODO add code to set an editor. (shell\edit) */

    /* TODO add code to use rexxc ? (shell\compile) */

    // Do the shellex\DropHandler subkey of ooRexx.ooDialog.1
    hKey = writeRegSubkeyValue(hProgIdKey, "ShellEx\\DropHandler", OODIALOG_DROP_HANDLER, hDlg, user);
    if ( hKey == NULL )
    {
        goto done_out;
    }

    RegCloseKey(hKey);
    RegCloseKey(hProgIdKey);

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return true;

done_out:
    if ( hProgIdKey != NULL )
    {
        RegCloseKey(hProgIdKey);

        // We delete the key because it was not completely written.  We don't
        // care if it fails or not because if it fails we can't do any thing
        // about it.
        deleteRegProgID(hDlg, paa);

    }
    return false;
}

/**
 * Handles the button click on the Done button.  If the user has checked the
 * Update when done check box, we update the PATHEXT, based on the current
 * contents of the PATHEXT list box.  Otherwise we just return.
 *
 * @param hDlg
 *
 * @return intptr_t
 */
static intptr_t pbDone(HWND hDlg)
{
    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);

    if ( IsDlgButtonChecked(hDlg, IDC_CK_UPDATE) != BST_CHECKED )
    {
        return TRUE;
    }

    char valBuf[MEDIUM_BUF_SIZE] = {'\0'};
    HWND hLB = paa->lbPathExt;

    LRESULT count = SendMessage(hLB, LB_GETCOUNT, NULL, NULL);
    if ( count == 0 )
    {
        // If we are configuring for the Current User, this means the user wants
        // to remove the Current User PATHEXT completely.
        if ( ! paa->allUsers )
        {
            return removingPathExtForCurrentUser(hDlg, paa);
        }
        else
        {
            reportError(hDlg, PATHEXT_UPDATE_FAILED, OOD_ERR_TITLE);
            return TRUE;
        }
    }
    else
    {
        for ( LRESULT i = 0; i < count; i++ )
        {
            pExtensionInfo info = (pExtensionInfo)SendMessage(hLB, LB_GETITEMDATA, i, 0);

            if ( paa->allUsers )
            {
                if ( info->allUsers )
                {
                    strcat(valBuf, info->extension);
                    strcat(valBuf, ";");
                }
            }
            else
            {
                if ( info->curUser )
                {
                    strcat(valBuf, info->extension);
                    strcat(valBuf, ";");
                }
            }
        }

        if ( strlen(valBuf) == 0 )
        {
            if ( ! paa->allUsers )
            {
                return removingPathExtForCurrentUser(hDlg, paa);
            }
            else
            {
                reportError(hDlg, PATHEXT_UPDATE_FAILED, OOD_ERR_TITLE);
                return TRUE;
            }
        }

        valBuf[strlen(valBuf) - 1] = '\0';

        char msg[MEDIUM_BUF_SIZE];
        _snprintf(msg, SMALL_BUF_SIZE, "The PATHEXT variable for %s will be changed to:\n\n"
                                       "%s.\n\nContinue and write this value?", paa->allUsers ? "All Users" : "the Current User", valBuf);

        int32_t reply = MessageBox(hDlg, msg, "Configure PATHEXT", MB_YESNOCANCEL | MB_ICONINFORMATION | MB_DEFBUTTON3);
        if ( reply == IDYES )
        {
            writeNewPathExt(hDlg, paa, valBuf);
        }
        else if ( reply == IDCANCEL )
        {
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * Handles the click on the arrow button that moves an exension from the
 * suggested list box to the current list box.
 *
 * This needs to get the record of the suggested item, register the extension,
 * adjust the list boxes and record arrays to the new status.
 *
 * @param hDlg
 *
 * @return intptr_t  We always return true.
 *
 * @remarks  The itemIndex of an item in a list box is often, but not always,
 *           the itemIndex of the extension record in its array.
 */
static intptr_t pbAddCurrent(HWND hDlg)
{
    pAssocArguments paa     = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);
    pExtRecords     extRecs = paa->extensionRecords;

    uint32_t       itemIndex;
    pExtensionInfo suggestedRec = getSelectedExtRec(paa->lbSuggested, &itemIndex);
    if ( suggestedRec == NULL )
    {
        goto done_out;
    }

    // It can't be registered in this scope, or it wouldn't have a record in the
    // suggested list box. We need to register the extension.
    if ( ! registerRegExt(hDlg, paa, suggestedRec->extension) )
    {
        goto done_out;
    }

    int32_t arrayIndex = getExtRecordIndex(extRecs->faSuggestedRecs, extRecs->faSuggestedNext, suggestedRec->extension);

    // The extension is now registered in the current scope, so the suggested
    // item must be removed no matter what we do in the current list box.
    // However we save the removed record because we may continue to use it,
    // then we free it, possibly, later.
    suggestedRec = removeFromSuggested(hDlg, itemIndex, arrayIndex);

    // An item for the extension may already be in the current list box, showing
    // as registered, but not in the current scope. We see if that record
    // exists.
    pExtensionInfo currentRec = getExtRecord(extRecs->faCurrentRecs, extRecs->faCurrentNext, suggestedRec->extension);

    if ( currentRec == NULL )
    {
        // Instead of freeing the suggestRec, we will add it to the current recs
        // array.
        currentRec = suggestedRec;
        if ( ! addRecToArray(hDlg, currentRec, cuFaRecArray) )
        {
            endThisDialog(hDlg, NULL);
            return TRUE;
        }
    }
    else
    {
        // We already have a rec, we just need to update its fields. In this
        // case we can free the suggested Rec.
        safeLocalFree(suggestedRec);
    }

    // The extension is now registered in the current scope, so we update the
    // record we are using to reflect that, which automatically updates the
    // display name.
    nowRegisteredInScope(paa, currentRec);

    // Now update the current list box
    SendMessage(paa->lbCurrent, LB_RESETCONTENT, 0, 0);
    fillCurrentListBox(hDlg, paa);

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

done_out:
    return TRUE;
}

/**
 * Handles the click on the arrow button that moves the extension from the
 * current list box to the suggested list box.
 *
 * This needs to get the record from the current list box, un-register the
 * extension, adjust the list boxes and record arrays to the new status.
 *
 * @param hDlg
 *
 * @return intptr_t
 */
static intptr_t pbRemoveCurrent(HWND hDlg)
{
    pAssocArguments paa     = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);
    pExtRecords     extRecs = paa->extensionRecords;

    uint32_t       index;
    pExtensionInfo currentRec = getSelectedExtRec(paa->lbCurrent, &index);
    if ( currentRec == NULL )
    {
        goto done_out;
    }

    // The record might not be registered in this scope.  If not we don't want
    // to do anything.  We should maybe put up a message box to let the user
    // know why the push button didn't do anything.
    if ( ! extRegisteredInScope(paa, currentRec) )
    {
        goto done_out;
    }

    // It is registered in this scope.  We want to un-register the extension.
    if ( ! deleteRegExt(hDlg, paa, currentRec->extension) )
    {
        goto done_out;
    }

    // Mark this record as not registered in the current scope, also updates the
    // display names.
    notRegisteredInScope(paa, currentRec);

    // The extension is no longer registered in the current scope, but it could
    // be registered in the 'other' scope.  The 'other' scope being the opposite
    // of what we are configuring for i.e., if are configuring for all users the
    // other scope is current user.  If this is the case, we need 2 records. One
    // is left in the current list box and we need a new record for the
    // suggested list box.
    pExtensionInfo suggestedRec = NULL;
    if ( extRegisteredInOtherScope(paa, currentRec) )
    {
        suggestedRec = dupExtRec(hDlg, currentRec);
        if ( suggestedRec == NULL )
        {
            endThisDialog(hDlg, NULL);
            return TRUE;
        }
        updateDisplayNameInPlace(paa, currentRec, index, cuFaRecArray);
    }
    else
    {
        // Not registered in the other scope, so the current list box item must
        // be removed from the current list box and record array.
        // removeFromCurrent() returns the record it is removing.  We rename it
        // to suggestedRec and will insert it into the suggested list box.
        int32_t arrayIndex = getExtRecordIndex(extRecs->faCurrentRecs, extRecs->faCurrentNext, currentRec->extension);
        suggestedRec = removeFromCurrent(hDlg, index, arrayIndex);
    }

    // An item for the extension can not already be in the suggested list box,
    // because it was  registered in this scope.  We add the record to the
    // suggested records and update the suggested list box.
    if ( ! addRecToArray(hDlg, suggestedRec, suFaRecArray) )
    {
        endThisDialog(hDlg, NULL);
        return TRUE;
    }

    SendMessage(paa->lbSuggested, LB_RESETCONTENT, 0, 0);
    fillSuggestedListBox(hDlg, paa);

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

done_out:
    return TRUE;
}

/**
 * Handles the click on the arrow button that adds an exension in the current
 * list box to the PATHEXT list box.  Note that this does not remove the
 * extension from the current list box.
 *
 * This needs to get the record of the selected item in the current list box,
 * alloc a new extension record, and add the new item to the PATHEXT list box.
 *
 * Note that we check to see if the extension is already in the PATHEXT list
 * box, and do nothing if that is the case.
 *
 * @param hDlg
 *
 * @return intptr_t  We always return true.
 */
static intptr_t pbArrowAddPathExt(HWND hDlg)
{
    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);

    uint32_t       index;
    pExtensionInfo currentRec = getSelectedExtRec(paa->lbCurrent, &index);
    if ( currentRec == NULL )
    {
        goto done_out;
    }

    // We want to see if the extension already exists in the PATHEXT for the
    // current scope
    if ( pathExtExtExists(paa, currentRec->extension) )
    {
        goto done_out;
    }

    pExtensionInfo rec = dupExtRec(hDlg, currentRec);
    if ( rec == NULL )
    {
        goto done_out;
    }

    updatePathExtArrays(hDlg, paa, rec, false);

done_out:
    return TRUE;
}

static intptr_t pbMovePathExt(HWND hDlg, bool up)
{
    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);

    uint32_t       index;
    pExtensionInfo currentRec = getSelectedExtRec(paa->lbPathExt, &index);
    if ( currentRec == NULL )
    {
        goto done_out;
    }

    /*
    char msg[SMALL_BUF_SIZE];
    _snprintf(msg, SMALL_BUF_SIZE, "Current rec %p", currnetRec);
    internalInfoMsgBox(msg, "Move item UP");
    */

    bool single = isSingleMode(hDlg);
    if ( up )
    {
        if ( index == 0 )
        {
            // Already at the top.
            SendMessage(paa->lbPathExt, LB_SETCURSEL, -1, 0);
            Sleep(0);
            SendMessage(paa->lbPathExt, LB_SETCURSEL, 0, 0);
            goto done_out;
        }

        // If we delete the item above, this should move our item up.
        pExtensionInfo rec = (pExtensionInfo)SendMessage(paa->lbPathExt, LB_GETITEMDATA, index - 1, 0);
        SendMessage(paa->lbPathExt, LB_DELETESTRING, index - 1, 0);

        // Now reinsert the deleted item at the position of our item
        SendMessage(paa->lbPathExt, LB_INSERTSTRING, index, (LPARAM)(single ? rec->displayName2 : rec->displayName));
        SendMessage(paa->lbPathExt, LB_SETITEMDATA, index, (LPARAM)rec);
        SendMessage(paa->lbPathExt, LB_SETTOPINDEX, index - 1, 0);
    }
    else
    {
        LRESULT count = SendMessage(paa->lbPathExt, LB_GETCOUNT, 0, 0);
        if ( count == index + 1 )
        {
            // Already at the bottom.
            goto done_out;
        }
        // If we delete this item, this should move the below items up.  We
        // don't need the extension record, we already have ti.
        SendMessage(paa->lbPathExt, LB_DELETESTRING, index, 0);

        // Now reinsert the item above its old index.
        index ++;
        SendMessage(paa->lbPathExt, LB_INSERTSTRING, index,
                    (LPARAM)(single ? currentRec->displayName2 : currentRec->displayName));
        SendMessage(paa->lbPathExt, LB_SETITEMDATA, index, (LPARAM)currentRec);
        SendMessage(paa->lbPathExt, LB_SETCURSEL, index, 0);
    }

done_out:
    return TRUE;
}

/**
 * Handles a click on the Delete PATHEXT extension button, the button with the X
 * icon.
 *
 * @param hDlg
 *
 * @return intptr_t
 *
 * @note  We can't just blindly delete an item.  We first check:  is an item
 *        selected, if not do nothing.  Is the selected item in the scoped
 *        PATHEXT, if not put up a message box telling the user that no change
 *        is possible.  Is the extension 'special', if yes put up message box
 *        telling the user it can't be deleted.
 *
 *        Then, if we do delete the extension we need to update the arrays.  For
 *        the in scope array, we decrement the next index by 1 and use save
 *        order to fix up the array.  For the not in scope array, we find the
 *        extension in the array, if it exists we mark it as not existing in the
 *        in scope PATHEXT
 */
static intptr_t pbDelPathExt(HWND hDlg)
{
    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);

    uint32_t       index;
    pExtensionInfo currentRec = getSelectedExtRec(paa->lbPathExt, &index);

    if ( currentRec == NULL )
    {
        goto done_out;
    }

    if ( isNotRelevantPathExt(hDlg, paa, currentRec) )
    {
        goto done_out;
    }

    if ( isRequiredPathExt(currentRec->extension, hDlg) )
    {
        goto done_out;
    }

    SendMessage(paa->lbPathExt, LB_DELETESTRING, index, 0);
    updatePathExtArrays(hDlg, paa, currentRec, true);
    safeLocalFree(currentRec);

done_out:
    return TRUE;
}

/**
 * Handles a click on the Double Delete PATHEXT extension button, the button
 * with the XXX icon.
 *
 * This allows the user to completely remove the Current User PATHEXT.  The X
 * delete button will not allow the user to remove .exe, .com, etc.. extensions.
 * This prevents the user from removing the Current User PATHEXT variable
 * completely.
 *
 * @param hDlg
 *
 * @return intptr_t
 *
 * @note  The DelXX button should be hidden unless we are configuring for the
 *        Current User, but we double check the scope for insurance.
 */
static intptr_t pbDelXXPathExt(HWND hDlg)
{
    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);

    if ( ! paa->allUsers )
    {
        pExtRecords extRecs = paa->extensionRecords;

        pExtensionInfo *auRecs = extRecs->allUsersRecs;
        pExtensionInfo *cuRecs = extRecs->curUserRecs;

        // First we need to fix up the display name in the all users records
        // array for each recored we are removing.
        for ( size_t i = 0; i < extRecs->cuNext; i++)
        {
            pExtensionInfo current = cuRecs[i];

            int32_t idx = getExtRecordIndex(auRecs, extRecs->auNext, current->extension);
            if ( idx >= 0 )
            {
                pExtensionInfo allRec = auRecs[idx];
                allRec->curUser = false;
                updateExtDisplayName(allRec);
            }
        }

        resetExtRecords(extRecs, cuPeRecArray);
        SendMessage(paa->lbPathExt, LB_RESETCONTENT, 0, 0);
        fillPathExtListBox(hDlg, paa);
    }

    return TRUE;
}


/**
 * Handles the click on the add PATHEXT extension button.  If the user has typed
 * in a valid extension in the edit box, the extension is added to the PATHEXT
 * list box for the in scope PATHEXT.
 *
 * @param hDlg
 *
 * @return intptr_t
 */
static intptr_t pbAddPEExtension(HWND hDlg)
{
    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);

    pExtensionInfo info = getExtRecordFromEditBox(hDlg, paa->peEdit);
    if ( info == NULL )
    {
        goto err_out;
    }

    // We want to see if the extension already exists in the PATHEXT for the
    // current scope
    if ( pathExtExtExists(paa, info->extension) )
    {
        goto err_out;
    }

    updatePathExtArrays(hDlg, paa, info, false);

    goto good_out;

err_out:
    safeLocalFree(info);

good_out:
    SetWindowText(paa->peEdit, "");
    return TRUE;
}

/**
 * Handles the click on the add file association extension button.  If the user
 * has typed in a valid extension in the edit box, the extension is registered
 * as a file association and the current list box is updated.
 *
 * @param hDlg
 *
 * @return intptr_t
 */
static intptr_t pbAddFAExtension(HWND hDlg)
{
    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);

    pExtensionInfo info = getExtRecordFromEditBox(hDlg, paa->faEdit);
    if ( info == NULL )
    {
        goto err_out;
    }

    qualifyExtensionInfo(info->extension, info, paa->progID);

    // If this extension is already registered in this scope, do nothing
    if ( extRegisteredInScope(paa, info) )
    {
        goto err_out;
    }

    // See if extension is already listed in the current list box
    uint32_t index;
    pExtensionInfo existing = getExtRec(paa->lbCurrent, info->extension, &index);
    if ( existing != NULL )
    {
        // It can't be registered in this scope, we already checked. We just
        // want to register and update the existing item.
        if ( ! registerRegExt(hDlg, paa, existing->extension) )
        {
            goto err_out;
        }
        nowRegisteredInScope(paa, existing);
        updateDisplayNameInPlace(paa, existing, index, cuFaRecArray);
        goto good_out;
    }

    pExtRecords extRecs = paa->extensionRecords;

    // See if the user entered an extenstion that is also in the suggested list
    // box.
    existing = getExtRec(paa->lbSuggested, info->extension, &index);
    if ( existing != NULL )
    {
        // It can't be registered in this scope, we already checked. We just
        // want to register, update the existing item, update the list boxes.
        if ( ! registerRegExt(hDlg, paa, existing->extension) )
        {
            goto err_out;
        }
        nowRegisteredInScope(paa, existing);

        int32_t arrayIndex = getExtRecordIndex(extRecs->faSuggestedRecs, extRecs->faSuggestedNext, existing->extension);

        existing = removeFromSuggested(hDlg, index, arrayIndex);
        if ( ! addRecToArray(hDlg, existing, cuFaRecArray) )
        {
            endThisDialog(hDlg, NULL);
            return TRUE;
        }

        SendMessage(paa->lbCurrent, LB_RESETCONTENT, 0, 0);
        fillCurrentListBox(hDlg, paa);
        goto good_out;
    }

    // Okay, a completely new extension, just register it and update the list
    // box and the records array.
    if ( ! registerRegExt(hDlg, paa, info->extension) )
    {
        goto err_out;
    }
    nowRegisteredInScope(paa, info);

    if ( ! addRecToArray(hDlg, info, cuFaRecArray) )
    {
        endThisDialog(hDlg, NULL);
        return TRUE;
    }

    SendMessage(paa->lbCurrent, LB_RESETCONTENT, 0, 0);
    fillCurrentListBox(hDlg, paa);

    goto good_out;

err_out:
    safeLocalFree(info);
    SetWindowText(paa->faEdit, "");
    return TRUE;

good_out:
    SetWindowText(paa->faEdit, "");
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    return TRUE;
}

/**
 * Handles a click on the Register / Un-register push button
 *
 * @param hDlg
 *
 * @return intptr_t
 *
 * @note  If scope is all users and all users is registered, then we must be
 *        removing the prog ID.  Otherwise we must be adding the prog ID.
 *
 *        Same logic if scope is current user.  If current user is registered,
 *        we must be removing the prog ID, otherwise we are adding.
 *
 *        If the operation is not a success, then we disable the push button so
 *        that the user does not try again.
 *
 *        When the operation is a success, then the state of the dialog must
 *        always be reset.
 */
static intptr_t pbRegister(HWND hDlg)
{
    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);

    bool removing;
    bool success;

    if ( paa->allUsers )
    {
        if ( paa->registeredAllUsers )
        {
            success  = removeProgID(hDlg, paa);
            removing = true;
        }
        else
        {
            success  = addProgID(hDlg, paa);
            removing = false;
        }
    }
    else
    {
        if ( paa->registeredCurUsers )
        {
            success  = removeProgID(hDlg, paa);
            removing = true;
        }
        else
        {
            success  = addProgID(hDlg, paa);
            removing = false;
        }
    }

    if ( success )
    {
        if ( removing )
        {
            if ( paa->allUsers )
            {
                paa->registeredAllUsers = false;
            }
            else
            {
                paa->registeredCurUsers = false;
            }
        }
        else
        {
            if ( paa->allUsers )
            {
                paa->registeredAllUsers = true;
            }
            else
            {
                paa->registeredCurUsers = true;
            }
        }

        paa->ftypeIsRegistered = (paa->registeredAllUsers || paa->registeredCurUsers) ? true : false;
        setRegisteredState(hDlg, paa);
    }
    else
    {
        EnableWindow(paa->pbRegister, FALSE);
    }
    return TRUE;
}

static intptr_t pbHelp(HWND hDlg)
{
    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);
    char buf[MEDIUM_BUF_SIZE];

    // Get the complete file name of ooDialog.pdf
    strcpy(buf, paa->exeName);
    char *p = StrRChr(buf, NULL, '\\');
    if ( p == NULL )
    {
        internalErrorMsgBox("An unexpected internal error is preventing Help from being shown",
                            "ooDialog Configure Services: Windows Error");
        return TRUE;
    }

    *p = '\0';
    strcat(buf, "\\doc\\oodialog.pdf");

    SHELLEXECUTEINFO ShExecInfo = {sizeof(SHELLEXECUTEINFO)};

    ShExecInfo.lpFile = buf;
    ShExecInfo.nShow  = SW_NORMAL;

    if ( ! ShellExecuteEx(&ShExecInfo) )
    {
        reportErrorPlus(hDlg, "ShellExecuteEx", OS_ERR_TITLE, "Failed to show the PDF help file", GetLastError());
    }

    /*  Works:
    ShellExecute(NULL, "open", "\"C:\\Program Files (x86)\\Adobe\\Reader 10.0\\Reader\\AcroRd32.exe\"",
                "/A page=97 \"C:\\Rexx\\ooRexx\\doc\\ooDialog.pdf\"", NULL, SW_SHOWNORMAL);
    */

    /* Also works:
    ShellExecute(NULL, "open", "\"AcroRd32.exe\"",
                "/A page=97 \"C:\\Rexx\\ooRexx\\doc\\ooDialog.pdf\"", NULL, SW_SHOWNORMAL);
    */

    /* Example from Web:
    ShellExecute(NULL, "open", "\"D:\\program files\\Adobe Reader 9.exe\"",
                "/A page=45 "
                "\"C:\\Documents and Settings\\Prabakar\\My Documents\\Downloads\\How to Do "
                "Everything _ Ubuntu (McGraw-Hill).pdf\"", NULL, SW_SHOWNORMAL);
    */

    return TRUE;
}

static intptr_t rbModeSwitch(HWND hDlg, bool isSingleNow)
{
    pAssocArguments paa = (pAssocArguments)getWindowPtr(hDlg, GWLP_USERDATA);

    savePathExtListBoxOrder(hDlg, paa);
    SendMessage(paa->lbPathExt, LB_RESETCONTENT, 0, 0);
    fillPathExtListBox(hDlg, paa);

    SendMessage(paa->lbSuggested, LB_RESETCONTENT, 0, 0);
    fillSuggestedListBox(hDlg, paa);

    SendMessage(paa->lbCurrent, LB_RESETCONTENT, 0, 0);
    fillCurrentListBox(hDlg, paa);

    return TRUE;
}

static intptr_t handleButtonClick(HWND hDlg, WPARAM wParam, LPARAM lParam)
{

    switch ( LOWORD(wParam) )
    {
        case IDOK:
            if ( pbDone(hDlg) )
            {
                endThisDialog(hDlg, wParam);
            }
            return TRUE;

        case IDCANCEL:
            endThisDialog(hDlg, wParam);
            return TRUE;

        case IDC_PB_REGISTER :
            return pbRegister(hDlg);

        case IDC_PB_ADD_PE_EXT :
            return pbAddPEExtension(hDlg);

        case IDC_PB_ADD_FA_EXT :
            return pbAddFAExtension(hDlg);

        case IDC_PB_ADD_CURRENT :
            return pbAddCurrent(hDlg);

        case IDC_PB_REMOVE_CURRENT :
            return pbRemoveCurrent(hDlg);

        case IDC_PB_ADD_PATHEXT :
            return pbArrowAddPathExt(hDlg);

        case IDC_PB_UP :
            return pbMovePathExt(hDlg, true);

        case IDC_PB_DOWN :
            return pbMovePathExt(hDlg, false);

        case IDC_PB_DEL :
            return pbDelPathExt(hDlg);

        case IDC_PB_DELXX :
            return pbDelXXPathExt(hDlg);

        case IDHELP :
            return pbHelp(hDlg);

        case IDC_RB_SINGLE_VIEW :
            return rbModeSwitch(hDlg, true);

        case IDC_RB_DOUBLE_VIEW :
            return rbModeSwitch(hDlg, false);

        default:
            break;
    }
    return FALSE;
}

/**
 * This function handles the initial set up for the dialog when it is first
 * opened.
 *
 * @param hDlg
 * @param paa
 */
static void setUp(HWND hDlg, pAssocArguments paa)
{
    setWindowPtr(hDlg, GWLP_USERDATA, (LONG_PTR)paa);

    paa->lbSuggested = GetDlgItem(hDlg, IDC_LB_SUGGESTED);
    paa->lbCurrent   = GetDlgItem(hDlg, IDC_LB_CURRENT);
    paa->lbPathExt   = GetDlgItem(hDlg, IDC_LB_PATHEXT);
    paa->pbRegister  = GetDlgItem(hDlg, IDC_PB_REGISTER);
    paa->faEdit      = GetDlgItem(hDlg, IDC_EDIT_FA_EXT);
    paa->peEdit      = GetDlgItem(hDlg, IDC_EDIT_PE_EXT);

    LoadString(paa->hInstance, IDS_FRIENDLY_NAME, paa->friendlyName, MAX_FRIENDLY_NAME);

    HFONT font = createMonoSpacedFont(8);

    SendMessage(paa->lbSuggested, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessage(paa->lbCurrent, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessage(paa->lbPathExt, WM_SETFONT, (WPARAM)font, FALSE);

    // Leave room for both the ending NULL and to add a '.'
    SendMessage(paa->faEdit, EM_SETLIMITTEXT, MAX_EXT_NAME - 2, 0);

    setDlgIcon(hDlg, paa->hInstance);
    setButtonIcons(hDlg, paa->hInstance);
    setStaticText(hDlg, paa);

    CheckDlgButton(hDlg, IDC_RB_DOUBLE_VIEW, BST_CHECKED);

    paa->extensionRecords = allocExtRecordsStruct(hDlg);
    if ( paa->extensionRecords == NULL )
    {
        endThisDialog(hDlg, NULL);
        return;
    }

    setRegisteredState(hDlg, paa);
}

/**
 *  The window procedure for the File Association / PATHEXT configuration
 *  dialog.
 *
 *  HKEY_CURRENT_USER \ Environment
 *
 *  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session
 *  Manager\Environment
 *
 * @param hDlg
 * @param uMsg
 * @param wParam
 * @param lParam
 *
 * @return intptr_t CALLBACK
 */
intptr_t CALLBACK FileAssocDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_INITDIALOG )
    {
        pAssocArguments paa = (pAssocArguments)lParam;

        // Query the registry to see if ooDialog ftype / progID is already
        // registered. This information is used to correctly set up the dialog.
        checkRegistration(paa);

        setUp(hDlg, paa);

        return TRUE;
    }

    switch ( uMsg )
    {
        case WM_COMMAND:
            // This is okay because we do not have any menus:
            if ( HIWORD(wParam) == BN_CLICKED )
            {
                return handleButtonClick(hDlg, wParam, lParam);
            }
            break;

        default:
            break;
    }

    return FALSE;
}


/**
 * The window procedure for the Configure Services dialog.
 *
 * @author Administrator (11/12/2013)
 *
 * @param hDlg
 * @param uMsg
 * @param wParam
 * @param lParam
 *
 * @return intptr_t CALLBACK
 */
intptr_t CALLBACK ConfigureDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if ( uMsg == WM_INITDIALOG )
    {
        pConfigureArguments pca = (pConfigureArguments)lParam;

        setDlgIcon(hDlg, pca->hInstance);

        if ( pca->wasElevated )
        {
            // We are only restarted as elevated when all users is checked.
            CheckDlgButton(hDlg, IDC_RB_ALL, BST_CHECKED);
        }
        else
        {
            CheckDlgButton(hDlg, IDC_RB_CURRENT, BST_CHECKED);
        }

        uint32_t rc = determineStatus(hDlg, pca);
        if ( rc != ERROR_SUCCESS )
        {
            internalErrorMsgBox("Operaring system errors prevent continuing.  Going to abort",
                                "ooDialog Configure Services: Windows Error");

            EndDialog(hDlg, wParam);
            return FALSE;
        }

        SetDlgItemText(hDlg, IDC_ST_INADMINGROUP, pca->isInAdminGroup ? "True" : "False");
        SetDlgItemText(hDlg, IDC_ST_ISRUNASADMIN, pca->isRunAsAdmin ? "True" : "False");

        if ( pca->isVistaOrLater )
        {
            SetDlgItemText(hDlg, IDC_ST_ISELEVATED, pca->isElevated ? "True" : "False");
            SetDlgItemText(hDlg, IDC_ST_IL, level2text(pca->integrityLevel));
        }
        else
        {
            SetDlgItemText(hDlg, IDC_ST_ISELEVATED, "N/A");
            SetDlgItemText(hDlg, IDC_ST_IL, "N/A");

            // If not running as admin on XP, can not select all users.
            if ( ! pca->isRunAsAdmin )
            {
                EnableWindow(GetDlgItem(hDlg, IDC_RB_ALL), FALSE);
            }
        }

        setWindowPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pca);

        if ( pca->wasElevated )
        {
            PostMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_PB_CONFIGURE, BN_CLICKED), 0);
        }

        return TRUE;
    }

    pConfigureArguments pca = (pConfigureArguments)getWindowPtr(hDlg, GWLP_USERDATA);

    switch ( uMsg )
    {
        case WM_COMMAND:
            switch ( LOWORD(wParam) )
            {
                case IDC_RB_ALL :
                {
                    if ( HIWORD(wParam) == BN_CLICKED && pca->requiresElevation )
                    {
                        Button_SetElevationRequiredState(GetDlgItem(hDlg, IDC_PB_CONFIGURE), TRUE);
                    }
                }
                break;

                case IDC_RB_CURRENT :
                {
                    if ( HIWORD(wParam) == BN_CLICKED && pca->requiresElevation )
                    {
                        Button_SetElevationRequiredState(GetDlgItem(hDlg, IDC_PB_CONFIGURE), FALSE);
                    }
                }
                break;

                case IDC_PB_CONFIGURE :
                {
                    if ( HIWORD(wParam) == BN_CLICKED )
                    {
                        uint32_t error;
                        bool     isAllUsers = IsDlgButtonChecked(hDlg, IDC_RB_ALL) ? true : false;

                        if ( isAllUsers && pca->requiresElevation )
                        {
                            char exeName[MAX_PATH];

                            if ( GetModuleFileName(NULL, exeName, ARRAYSIZE(exeName)) )
                            {
                                // Re-execute ourself as Administrator
                                SHELLEXECUTEINFO sei = { sizeof(sei) };

                                sei.lpVerb       = "runas";
                                sei.lpFile       = exeName;
                                sei.lpParameters = "-confElevatedAllUsers";
                                sei.hwnd         = hDlg;
                                sei.nShow        = SW_NORMAL;

                                if ( ! ShellExecuteEx(&sei))
                                {
                                    error = GetLastError();
                                    if ( error == ERROR_CANCELLED )
                                    {
                                        // The user refused the elevation.
                                        // Do nothing ...
                                    }
                                    else
                                    {
                                        reportError(hDlg, "ShellExecuteEx", OS_ERR_TITLE, error);
                                    }
                                }
                                else
                                {
                                    // Quit this process.
                                    ShowWindow(hDlg, SW_HIDE);
                                    EndDialog(hDlg, TRUE);
                                }
                            }
                            else
                            {
                                error = GetLastError();
                                reportError(hDlg, "GetModuleFileName", OS_ERR_TITLE, error);
                            }
                        }
                        else
                        {
                            // Start the file assoc dialog.
                            assocArguments aa = { 0 };

                            aa.isRunAsAdmin = pca->isRunAsAdmin;
                            aa.hInstance    = pca->hInstance;
                            aa.allUsers     = isAllUsers;
                            aa.isElevated   = pca->isElevated;
                            aa.exeName      = pca->exeName;

                            strcpy(aa.progID, OODIALOG_PROGID_VERSIONED);

                            DialogBoxParam(pca->hInstance, MAKEINTRESOURCE(IDD_FILEASSOC), hDlg, FileAssocDlgProc, (LPARAM)&aa);
                        }
                    }
                }
                    break;

                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, wParam);
                    return TRUE;

            }
            break;

        default:
            break;
    }

    return FALSE;
}


#define WinMain_FUNCTIONS_BEGIN

inline bool haveFlag(CSTRING a)
{
    return *a == '-' || *a == '/';
}

static int32_t displayCondition(RexxThreadContext *c, pProgramArguments pa)
{
    wholenumber_t code = 0;

    char *msg = getStandardConditionMsg(c, &code, NULL);
    if ( msg )
    {
        pa->mainMsg = msg;
        pa->dlgType = conditionDisplay;

        DialogBoxParam(pa->hInstance, MAKEINTRESOURCE(IDD_OODIALOG_DISPLAY), NULL, WinMainDlgProc, (LPARAM)pa);

        LocalFree(msg);
    }
    return (int32_t)code;
}

static int32_t ooDialogServices(pProgramArguments pa)
{
    configureArguments ca = { 0 };
    ca.hInstance = pa->hInstance;
    ca.exeName   = pa->exeName;

    DialogBoxParam(pa->hInstance, MAKEINTRESOURCE(IDD_CONFIGURE_SERVICES), NULL, ConfigureDlgProc, (LPARAM)&ca);

    return 0;
}

static int32_t displaySyntax(pProgramArguments pa)
{
    pa->dlgType = syntaxDisplay;
    pa->mainMsg = long_syntax_text;

    DialogBoxParam(pa->hInstance, MAKEINTRESOURCE(IDD_OODIALOG_DISPLAY), NULL, WinMainDlgProc, (LPARAM)pa);

    if ( pa->doConfigure )
    {
        ooDialogServices(pa);
    }
    return 0;
}

static int32_t displayVersion(RexxThreadContext *c, pProgramArguments pa)
{
    if ( pa->showShortVersion )
    {
        char buf[MEDIUM_BUF_SIZE];

        _snprintf(buf, MEDIUM_BUF_SIZE, "ooDialog version %u.%u.%u.%u", OOD_VER, OOD_REL, OOD_MOD, OOD_BLD);
        internalInfoMsgBox(buf, "ooDialog Execute Program: Version Information");
        return 0;
    }

    char *ver = getCompleteVersion(c);
    if ( ver )
    {
        pa->dlgType = versionDisplay;
        pa->mainMsg = ver;
        DialogBoxParam(pa->hInstance, MAKEINTRESOURCE(IDD_OODIALOG_DISPLAY), NULL, WinMainDlgProc, (LPARAM)pa);

        LocalFree(ver);
    }
    else
    {
        internalErrorMsgBox("ooDialog internal error, could not get version string.", "ooDialog Execute Program: Internal Error");
        return -1;
    }
    return 0;
}

/**
 * Gets the wide character string command line arguments in conventional argv /
 * argc format and converst the argument array to an array of ANSI strings.
 *
 * @param count
 *
 * @return An array of null-terminated ANSI strings, or null on error.
 *
 * @note  The returned array does not contain the executable name.
 *
 *        If there is no error, it is the responsibility of the caller to free
 *        both the returned array and the strings in the array.
 *
 *        On error, there is no memory to free.  We free the allocated argv and
 *        any strings it may point to, even though we do not need to free this
 *        memory.  On error, we are going to immediately exit the program and
 *        the OS will free the memory.  We just do it because it looks a little
 *        cleaner.
 */
static char **commandLineToArgv(int32_t *count, char *exeName)
{
    LPWSTR  *szArglist = NULL;
    char   **args      = NULL;
    int32_t nArgs      = 0;
    int32_t i;

    char buf[512] = {'\0'};  // Possible error message buffer

    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if ( NULL == szArglist )
    {
        reportError(OS_PARSING_ERR_FMT, GetLastError(), OS_ERR_TITLE);
        goto done_out;
    }

    // Need to save the fully qualified executable name.
    char *tmp = unicode2ansi(szArglist[0]);
    if ( tmp )
    {
        strcpy(exeName, tmp);
        LocalFree(tmp);

        if ( PathIsRelative(exeName) )
        {
            PathFindOnPath(exeName, NULL);
        }
    }
    else
    {
        unicodeConversionErr();
        goto done_out;
    }

    // For the returned argument array, we are going skip the first arg, the
    // executable name.
    nArgs--;

    args = (char **)LocalAlloc(LPTR, nArgs * sizeof(char **));
    if ( args == NULL )
    {
        unicodeConversionErr();
        goto done_out;
    }

    for ( i = 0; i < nArgs; i++)
    {
        LPWSTR a = szArglist[i + 1];

        if ( wcslen(a) == 0 )
        {
            reportError(EMPTY_STRING_ARG_ERR_FMT, i + 1, USER_ERR_TITLE);
            goto error_out;
        }

        tmp = unicode2ansi(a);
        if ( tmp )
        {
            args[i] = tmp;
        }
        else
        {
            unicodeConversionErr();
            goto error_out;
        }
    }

    goto done_out;

error_out:
    if ( args != NULL )
    {
        for ( int32_t j = 0; j < nArgs; j++ )
        {
            safeLocalFree(args[j]);
        }
        LocalFree(args);
    }
    args  = NULL;
    nArgs = 0;

done_out:
    LocalFree(szArglist);
    *count = nArgs;

    return args;
}

static bool getFlags(RexxThreadContext *c, pProgramArguments pa, char **argv, int32_t argc, int32_t *currentArg)
{
    int32_t i = *currentArg;

    while ( i < argc && haveFlag(argv[i]) )
    {
        switch ( argv[i][1] )
        {
            case '?' :
            case 'h' :
                pa->showShortHelp = true;
                c->ArrayPut(pa->rxOodArgs, c->String(argv[i]), i + 1);
                LocalFree(argv[i]);
                break;

            case 'H' :
                pa->showHelp = true;
                c->ArrayPut(pa->rxOodArgs, c->String(argv[i]), i + 1);
                LocalFree(argv[i]);
                break;

            case 's' :
                pa->doConfigure = true;
                c->ArrayPut(pa->rxOodArgs, c->String(argv[i]), i + 1);
                LocalFree(argv[i]);
                break;

            case 'v' :
                pa->showShortVersion = true;
                c->ArrayPut(pa->rxOodArgs, c->String(argv[i]), i + 1);
                LocalFree(argv[i]);
                break;

            case 'V' :
                pa->showVersion = true;
                c->ArrayPut(pa->rxOodArgs, c->String(argv[i]), i + 1);
                LocalFree(argv[i]);
                break;

            case '-' :
                if ( stricmp(argv[i], "--version") == 0 )
                {
                    pa->showVersion = true;
                    c->ArrayPut(pa->rxOodArgs, c->String(argv[i]), i + 1);
                    LocalFree(argv[i]);
                }
                else if ( stricmp(argv[i], "--help") == 0 )
                {
                    pa->showHelp = true;
                    c->ArrayPut(pa->rxOodArgs, c->String(argv[i]), i + 1);
                    LocalFree(argv[i]);
                }
                break;

            default :
            {
                char msg[256];

                sprintf(msg, "Invalid command line argument:\n\n"
                             "  Argument:\t%s\n"
                             "  Position:\t\t%d\n",
                             argv[i], i + 1);
                internalErrorMsgBox(msg, "ooDialog Execute Program: User Error");

                LocalFree(argv[i]);
                return false;
            }
        }
        i++;
    }

    *currentArg = i;
    return true;
}

static bool parseArgs(RexxThreadContext *c, pProgramArguments pa, char **argv, int32_t argc, LPSTR lpCmdLine)
{
    int32_t currentArg = 0;
    bool    result     = false;

    pa->rxOodArgs = c->NewArray(argc);

    if ( ! getFlags(c, pa, argv, argc, &currentArg) )
    {
        displaySyntax(pa);
        goto done_out;
    }

    if ( pa->showShortHelp || pa->showShortVersion || pa->showHelp || pa->showVersion || pa->doConfigure )
    {
        // If any of these are set, we're done;
        result = true;
        goto done_out;
    }

    if ( currentArg >= argc )
    {
        internalErrorMsgBox("The ooDialog program to execute was not specified.", "ooDialog Execute Program: User Error");
        displaySyntax(pa);
        goto done_out;
    }

    size_t prgLen = strlen(argv[currentArg]);

    pa->oodProgram = (char *)LocalAlloc(LPTR, prgLen + 1);
    if ( pa->oodProgram == NULL )
    {
        internalErrorMsgBox("The Operating System failed to allocate memory.", "ooDialog Execute Program: Windows Error");
        LocalFree(argv[currentArg]);
        goto done_out;
    }

    memcpy((void *)pa->oodProgram, argv[currentArg], prgLen);
    LocalFree(argv[currentArg++]);

    if ( currentArg >= argc )
    {
        pa->callArg = c->NewArray(0);
        result = true;
        goto done_out;
    }

    char *tmp = strstr(lpCmdLine, pa->oodProgram);
    tmp += prgLen;
    while( isspace(*tmp) )
    {
        tmp++;
    }

    pa->callArg = c->ArrayOfOne(c->String(tmp));
    pa->rxCArgs = c->NewArray(argc - currentArg);

    for ( int32_t j = 1; currentArg < argc; currentArg++, j++ )
    {
        c->ArrayPut(pa->rxCArgs,   c->String(argv[currentArg]), currentArg + 1);
        c->ArrayPut(pa->rxOodArgs, c->String(argv[currentArg]), j);
        LocalFree(argv[currentArg]);
    }
    result = true;

done_out:
    LocalFree(argv);
    return result;
}

/**
 * Standard Windows entry point.
 *
 * @param hInstance
 * @param hPrevInstance
 * @param lpCmdLine
 * @param nCmdShow
 *
 * @return int
 *
 * @notes  Testing shows that lpCmdLine is never null.
 *
 *         If the first argument is exactly: -confElevatedAllUsers then this a
 *         special case.  We were in the ConfigureDlgProc procedure and the user
 *         requested to change or set the All Users file associations, on Vista
 *         or later, and elevation was required.  So, we have been restarted
 *         through ShellExcuteEx().
 *
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    programArguments pa = { 0 };
    int32_t          rc = 0;

    pa.hInstance = hInstance;

    if ( strlen(lpCmdLine) == 0 )
    {
        pa.allowServices = true;
        return displaySyntax(&pa);
    }

    // Get the command line in a C style argument vector form.
    int32_t   argc;
    char    **argv = commandLineToArgv(&argc, pa.exeName);
    if ( argv == NULL )
    {
        return -1;
    }

    if ( strcmp(argv[0], "-confElevatedAllUsers") == 0 )
    {
        configureArguments ca = { 0 };

        ca.hInstance   = hInstance;
        ca.wasElevated = true;
        ca.exeName     = pa.exeName;

        DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CONFIGURE_SERVICES), NULL, ConfigureDlgProc, (LPARAM)&ca);
        return 0;
    }

    // Mostly, we will be executing a Rexx program.  parseArgs() needs an
    // interpreter thread context, so we create the interpreter here.  For the
    // few times it is not needed, the overhead is negligible.
    RexxThreadContext *c;
    RexxInstance      *interpreter;

    if ( ! RexxCreateInterpreter(&interpreter, &c, NULL) )
    {
        internalErrorMsgBox("The ooRexx interpreter instance could not be created",
                            "ooDialog Execute Program: ooRexx Error");
        return -1;
    }

    if ( ! parseArgs(c, &pa, argv, argc, lpCmdLine) )
    {
        rc = -1;
        goto done_out;
    }

    if ( pa.showVersion || pa.showShortVersion )
    {
        displayVersion(c, &pa);
        goto done_out;
    }

    if ( pa.showHelp || pa.showShortHelp )
    {
        pa.allowServices = pa.showHelp ? true : false;
        displaySyntax(&pa);
        goto done_out;
    }

    if ( pa.doConfigure )
    {
        ooDialogServices(&pa);
        goto done_out;
    }

    // We put 2 argument arrays into the .local environment
    RexxDirectoryObject localEnv = (RexxDirectoryObject)c->GetLocalEnvironment();

    c->DirectoryPut(localEnv, pa.rxCArgs, "SYSCARGS");
    c->DirectoryPut(localEnv, pa.rxOodArgs, "OODCARGS");

    // Have the interpreter run the program:
    RexxObjectPtr result = c->CallProgram(pa.oodProgram, pa.callArg);

    if ( c->CheckCondition() )
    {
        rc = displayCondition(c, &pa);
        rc = -rc;  // Traditionally, the return code is negated on a condition.
    }
    else
    {
        if ( result != NULLOBJECT )
        {
            c->ObjectToInt32(result, &rc);
        }
    }

done_out:
    interpreter->Terminate();

    return rc;
}


