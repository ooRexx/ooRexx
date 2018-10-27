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

/**
 * oodShellObjects.cpp
 *
 * Contains ooDialog classes that are more Shell orientated.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <shlobj.h>
#include <shlwapi.h>
#include <Rpc.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodShared.hpp"
#include "oodShellObjects.hpp"

/* Allow compiling with the Windows SDK version 6.1.  To compile with a 6.1
 * Windows SDK set OODIALOG_WINSDK_6_1=1 in the environment and the make file
 * will do the proper thing.
 */
#ifdef OODIALOG_WINSDK_6_1
typedef DWORD FILEOPENDIALOGOPTIONS;
#define CDCS_ENABLEDVISIBLE 0x3
#endif

/**
 * General purpose stuff for working with the Shell.
 */
#define GENERAL_PUPOSE_SHELL

/**
 * Checks that a path is a full path name and not a relative path name.  We
 * define full path name here as a UNC path or a path that includes the drive
 * letter and colon.
 *
 * @param path
 *
 * @return bool
 */
static bool _PathIsFull(const char *path)
{
    if ( PathIsUNC(path) )
    {
        return true;
    }
    else if ( PathGetDriveNumber(path) != -1 )
    {
        return true;
    }
    return false;
}

/**
 * Converts a string of keywords to its CSIDL_xx value. Raises an exception if
 * keyword is not valid.
 *
 * @param c
 * @param keyword
 * @param argPos
 *
 * @return uint32_t
 */
static uint32_t keyword2csidl(RexxMethodContext *c, CSTRING keyword, size_t argPos)
{
    uint32_t csidl = OOD_ID_EXCEPTION;

         if ( StrCmpI(keyword, "CSIDL_DESKTOP"                ) == 0 ) csidl = CSIDL_DESKTOP                ;
    else if ( StrCmpI(keyword, "CSIDL_INTERNET"               ) == 0 ) csidl = CSIDL_INTERNET               ;
    else if ( StrCmpI(keyword, "CSIDL_PROGRAMS"               ) == 0 ) csidl = CSIDL_PROGRAMS               ;
    else if ( StrCmpI(keyword, "CSIDL_CONTROLS"               ) == 0 ) csidl = CSIDL_CONTROLS               ;
    else if ( StrCmpI(keyword, "CSIDL_PRINTERS"               ) == 0 ) csidl = CSIDL_PRINTERS               ;
    else if ( StrCmpI(keyword, "CSIDL_PERSONAL"               ) == 0 ) csidl = CSIDL_PERSONAL               ;
    else if ( StrCmpI(keyword, "CSIDL_FAVORITES"              ) == 0 ) csidl = CSIDL_FAVORITES              ;
    else if ( StrCmpI(keyword, "CSIDL_STARTUP"                ) == 0 ) csidl = CSIDL_STARTUP                ;
    else if ( StrCmpI(keyword, "CSIDL_RECENT"                 ) == 0 ) csidl = CSIDL_RECENT                 ;
    else if ( StrCmpI(keyword, "CSIDL_SENDTO"                 ) == 0 ) csidl = CSIDL_SENDTO                 ;
    else if ( StrCmpI(keyword, "CSIDL_BITBUCKET"              ) == 0 ) csidl = CSIDL_BITBUCKET              ;
    else if ( StrCmpI(keyword, "CSIDL_STARTMENU"              ) == 0 ) csidl = CSIDL_STARTMENU              ;
    else if ( StrCmpI(keyword, "CSIDL_MYDOCUMENTS"            ) == 0 ) csidl = CSIDL_MYDOCUMENTS            ;
    else if ( StrCmpI(keyword, "CSIDL_MYMUSIC"                ) == 0 ) csidl = CSIDL_MYMUSIC                ;
    else if ( StrCmpI(keyword, "CSIDL_MYVIDEO"                ) == 0 ) csidl = CSIDL_MYVIDEO                ;
    else if ( StrCmpI(keyword, "CSIDL_DESKTOPDIRECTORY"       ) == 0 ) csidl = CSIDL_DESKTOPDIRECTORY       ;
    else if ( StrCmpI(keyword, "CSIDL_DRIVES"                 ) == 0 ) csidl = CSIDL_DRIVES                 ;
    else if ( StrCmpI(keyword, "CSIDL_NETWORK"                ) == 0 ) csidl = CSIDL_NETWORK                ;
    else if ( StrCmpI(keyword, "CSIDL_NETHOOD"                ) == 0 ) csidl = CSIDL_NETHOOD                ;
    else if ( StrCmpI(keyword, "CSIDL_FONTS"                  ) == 0 ) csidl = CSIDL_FONTS                  ;
    else if ( StrCmpI(keyword, "CSIDL_TEMPLATES"              ) == 0 ) csidl = CSIDL_TEMPLATES              ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_STARTMENU"       ) == 0 ) csidl = CSIDL_COMMON_STARTMENU       ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_PROGRAMS"        ) == 0 ) csidl = CSIDL_COMMON_PROGRAMS        ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_STARTUP"         ) == 0 ) csidl = CSIDL_COMMON_STARTUP         ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_DESKTOPDIRECTORY") == 0 ) csidl = CSIDL_COMMON_DESKTOPDIRECTORY;
    else if ( StrCmpI(keyword, "CSIDL_APPDATA"                ) == 0 ) csidl = CSIDL_APPDATA                ;
    else if ( StrCmpI(keyword, "CSIDL_PRINTHOOD"              ) == 0 ) csidl = CSIDL_PRINTHOOD              ;
    else if ( StrCmpI(keyword, "CSIDL_LOCAL_APPDATA"          ) == 0 ) csidl = CSIDL_LOCAL_APPDATA          ;
    else if ( StrCmpI(keyword, "CSIDL_ALTSTARTUP"             ) == 0 ) csidl = CSIDL_ALTSTARTUP             ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_ALTSTARTUP"      ) == 0 ) csidl = CSIDL_COMMON_ALTSTARTUP      ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_FAVORITES"       ) == 0 ) csidl = CSIDL_COMMON_FAVORITES       ;
    else if ( StrCmpI(keyword, "CSIDL_INTERNET_CACHE"         ) == 0 ) csidl = CSIDL_INTERNET_CACHE         ;
    else if ( StrCmpI(keyword, "CSIDL_COOKIES"                ) == 0 ) csidl = CSIDL_COOKIES                ;
    else if ( StrCmpI(keyword, "CSIDL_HISTORY"                ) == 0 ) csidl = CSIDL_HISTORY                ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_APPDATA"         ) == 0 ) csidl = CSIDL_COMMON_APPDATA         ;
    else if ( StrCmpI(keyword, "CSIDL_WINDOWS"                ) == 0 ) csidl = CSIDL_WINDOWS                ;
    else if ( StrCmpI(keyword, "CSIDL_SYSTEM"                 ) == 0 ) csidl = CSIDL_SYSTEM                 ;
    else if ( StrCmpI(keyword, "CSIDL_PROGRAM_FILES"          ) == 0 ) csidl = CSIDL_PROGRAM_FILES          ;
    else if ( StrCmpI(keyword, "CSIDL_MYPICTURES"             ) == 0 ) csidl = CSIDL_MYPICTURES             ;
    else if ( StrCmpI(keyword, "CSIDL_PROFILE"                ) == 0 ) csidl = CSIDL_PROFILE                ;
    else if ( StrCmpI(keyword, "CSIDL_SYSTEMX86"              ) == 0 ) csidl = CSIDL_SYSTEMX86              ;
    else if ( StrCmpI(keyword, "CSIDL_PROGRAM_FILESX86"       ) == 0 ) csidl = CSIDL_PROGRAM_FILESX86       ;
    else if ( StrCmpI(keyword, "CSIDL_PROGRAM_FILES_COMMON"   ) == 0 ) csidl = CSIDL_PROGRAM_FILES_COMMON   ;
    else if ( StrCmpI(keyword, "CSIDL_PROGRAM_FILES_COMMONX86") == 0 ) csidl = CSIDL_PROGRAM_FILES_COMMONX86;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_TEMPLATES"       ) == 0 ) csidl = CSIDL_COMMON_TEMPLATES       ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_DOCUMENTS"       ) == 0 ) csidl = CSIDL_COMMON_DOCUMENTS       ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_ADMINTOOLS"      ) == 0 ) csidl = CSIDL_COMMON_ADMINTOOLS      ;
    else if ( StrCmpI(keyword, "CSIDL_ADMINTOOLS"             ) == 0 ) csidl = CSIDL_ADMINTOOLS             ;
    else if ( StrCmpI(keyword, "CSIDL_CONNECTIONS"            ) == 0 ) csidl = CSIDL_CONNECTIONS            ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_MUSIC"           ) == 0 ) csidl = CSIDL_COMMON_MUSIC           ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_PICTURES"        ) == 0 ) csidl = CSIDL_COMMON_PICTURES        ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_VIDEO"           ) == 0 ) csidl = CSIDL_COMMON_VIDEO           ;
    else if ( StrCmpI(keyword, "CSIDL_RESOURCES"              ) == 0 ) csidl = CSIDL_RESOURCES              ;
    else if ( StrCmpI(keyword, "CSIDL_RESOURCES_LOCALIZED"    ) == 0 ) csidl = CSIDL_RESOURCES_LOCALIZED    ;
    else if ( StrCmpI(keyword, "CSIDL_COMMON_OEM_LINKS"       ) == 0 ) csidl = CSIDL_COMMON_OEM_LINKS       ;
    else if ( StrCmpI(keyword, "CSIDL_CDBURN_AREA"            ) == 0 ) csidl = CSIDL_CDBURN_AREA            ;
    else if ( StrCmpI(keyword, "CSIDL_COMPUTERSNEARME"        ) == 0 ) csidl = CSIDL_COMPUTERSNEARME        ;
    else
    {
        invalidConstantException(c, argPos, INVALID_CONSTANT_MSG, "CSIDL", keyword);
    }

    return csidl;
}


/**
 * Converts a keyword to its SIGDN_xx value. Raises an exception if keyword is
 * not valid.
 *
 * @param c
 * @param keyword
 * @param argPos
 *
 * @return uint32_t
 */
static bool keyword2sigdn(RexxMethodContext *c, CSTRING keyword, SIGDN *pSigdn, size_t argPos)
{
    SIGDN sigdn;

         if ( StrCmpI(keyword, "DESKTOPABSOLUTEEDITING"      ) == 0 ) sigdn = SIGDN_DESKTOPABSOLUTEEDITING     ;
    else if ( StrCmpI(keyword, "DESKTOPABSOLUTEPARSING"      ) == 0 ) sigdn = SIGDN_DESKTOPABSOLUTEPARSING     ;
    else if ( StrCmpI(keyword, "FILESYSPATH"                 ) == 0 ) sigdn = SIGDN_FILESYSPATH                ;
    else if ( StrCmpI(keyword, "PARENTRELATIVE"              ) == 0 ) sigdn = SIGDN_PARENTRELATIVE             ;
    else if ( StrCmpI(keyword, "PARENTRELATIVEEDITING"       ) == 0 ) sigdn = SIGDN_PARENTRELATIVEEDITING      ;
    else if ( StrCmpI(keyword, "PARENTRELATIVEFORADDRESSBAR" ) == 0 ) sigdn = SIGDN_PARENTRELATIVEFORADDRESSBAR;
    else if ( StrCmpI(keyword, "PARENTRELATIVEPARSING"       ) == 0 ) sigdn = SIGDN_PARENTRELATIVEPARSING      ;
    else if ( StrCmpI(keyword, "URL"                         ) == 0 ) sigdn = SIGDN_URL                        ;
    else if ( StrCmpI(keyword, "NORMALDISPLAY"               ) == 0 ) sigdn = SIGDN_NORMALDISPLAY              ;
    else
    {
        invalidConstantException(c, argPos, INVALID_CONSTANT_MSG, "SIGDN", keyword);
        return false;
    }

    *pSigdn = sigdn;
    return true;
}


/**
 * Obtains a pointer to an item ID list from a path string.
 *
 * This function is used because it will work on W2K (and all the way back to
 * Windows 95.)
 *
 * @param path   Get the item ID list for this path.
 * @param ppidl  The item ID list is returned in this variable.
 *
 * @return True on success, otherwise false.
 *
 * @note   If this function fails, *ppidl will be null on return.  This function
 *         does not raise an exception, that is left to the caller.
 */
static bool pidlFromPath(RexxMethodContext *c, LPCSTR path, LPITEMIDLIST *ppidl, HRESULT *pHR)
{
    LPSHELLFOLDER pShellFolder = NULL;
    HRESULT       hr = 0;
    WCHAR         wPath[MAX_PATH];

    *ppidl = NULL;

    if ( MultiByteToWideChar(CP_ACP, 0, path, -1, wPath, MAX_PATH) == 0 )
    {
        hr = GetLastError();
        goto err_out;
    }

    // Need the desktop IShellFolder interface.
    hr = SHGetDesktopFolder(&pShellFolder);
    if ( FAILED(hr) )
    {
        goto err_out;
    }

    // convert the path to an ITEMIDLIST
    hr = pShellFolder->ParseDisplayName(NULL, NULL, wPath, NULL, ppidl, NULL);

    pShellFolder->Release();

    if ( FAILED(hr) )
    {
        goto err_out;
    }
    return true;

err_out:
    oodSetSysErrCode(c->threadContext, hr);
    *pHR = hr;
    *ppidl = NULL;  // Docs are unclear as to the value of this on failure.
    return false;

}

/**
 * Takes a CSIDL_xxx constant and returns its item ID list pointer.
 *
 * @param csidl
 * @param ppidl
 *
 * @return bool
 */
static bool pidlForSpecialFolder(uint32_t csidl, LPITEMIDLIST *ppidl, HRESULT *pHR)
{
    HRESULT  hr;
    bool     success = true;

    hr = SHGetFolderLocation(NULL, csidl, NULL, 0, ppidl);
    if ( FAILED(hr) )
    {
        *ppidl  = NULL;
        *pHR    = hr;
        success = false;
    }
    return success;
}


/**
 * Converts a string into a LPITEMIDLIST.  The string must either be one of the
 * CSIDL_xxx constants or a full path name.
 *
 * @param c
 * @param idl
 * @param argPos
 * @param canFail
 * @param pHR
 *
 * @return LPITEMIDLIST
 *
 * @remarks  If the string starts with CSIDL_, but is not a recognized, an
 *           exception is raised.
 */
LPITEMIDLIST getPidlFromString(RexxMethodContext *c, CSTRING idl, size_t argPos, bool canFail, HRESULT *pHR)
{
    LPITEMIDLIST pidl = NULL;

    // See if it looks like a CSIDL_ constant.
    if ( strlen(idl) > 6 && StrCmpNI("CSIDL_", idl, 6) == 0 )
    {
        uint32_t csidl = keyword2csidl(c, idl, argPos);

        if ( csidl == OOD_ID_EXCEPTION )
        {
            return NULL;
        }
        pidlForSpecialFolder(csidl, &pidl, pHR);
    }
    else if ( _PathIsFull(idl) )
    {
        pidlFromPath(c, idl, &pidl, pHR);
    }
    else
    {
        wrongArgValueException(c->threadContext, argPos, WRONG_IDL_TYPE_LIST_SHORT, idl);
        return NULL;
    }

    if ( pidl == NULL && ! canFail )
    {
        systemServiceException(c->threadContext, NO_ITEMID_MSG, idl);
    }
    return pidl;
}

/**
 * Extracts a pointer to an item ID list from a Rexx object sent as arg to a
 * method.  The object must resolve to a CSIDL_xxx constant, a full path, or a
 * Pointer object.
 *
 * @param c
 * @param obj
 * @param argPos
 *
 * @return LPITEMIDLIST
 */
LPITEMIDLIST getPidlFromObject(RexxMethodContext *c, RexxObjectPtr obj, size_t argPos, HRESULT *pHR)
{
    LPITEMIDLIST pidl = NULL;

    if ( c->IsString(obj) )
    {
        CSTRING idl = c->ObjectToStringValue(obj);
        pidl = getPidlFromString(c, idl, argPos, true, pHR);
    }
    else if ( c->IsPointer(obj) )
    {
        pidl = (LPITEMIDLIST)c->PointerValue((RexxPointerObject)obj);
        if ( pidl == NULL )
        {
            nullObjectException(c->threadContext, "pointer to an Item ID List", argPos);
        }
    }
    else
    {
        wrongArgValueException(c->threadContext, argPos, WRONG_IDL_TYPE_LIST, obj);
    }

    return pidl;
}


/**
 * Transform a Rexx object used as an argument to a method to a pointer to an
 * IShellItem object.
 *
 * @param c
 * @param folder
 * @param argPos
 *
 * @return IShellItem*
 *
 * @remarks  Although the function name does not indicate it, the Rexx object
 *           argument is assumed to be a folder in this case.
 *
 *           This uses a depracated method so that it will / should work on XP.
 *           When ooDialog becomes Vista or later this should be reworked.
 */
static IShellItem *getShellItemFromObject(RexxMethodContext *c, RexxObjectPtr folder, size_t argPos, HRESULT *pHR)
{
    LPITEMIDLIST pidl = getPidlFromObject(c, folder, argPos, pHR);
    if ( pidl == NULL )
    {
        return NULL;
    }

    IShellItem *psi = NULL;
    HRESULT hr = SHCreateShellItem(NULL, NULL, pidl, &psi);
    if ( FAILED(hr) )
    {
        oodSetSysErrCode(c->threadContext, hr);
        *pHR = hr;
    }

    CoTaskMemFree(pidl);
    return psi;
}


/**
 * Returns a shell item for the path name specified.
 *
 * @param c
 * @param path
 * @param argPos
 * @param pHR
 *
 * @return IShellItem*
 */
static IShellItem *getShellItemFromPath(RexxMethodContext *c, LPCSTR path, size_t argPos, HRESULT *pHR)
{
    LPITEMIDLIST  pidl = NULL;
    IShellItem   *psi  = NULL;

    if ( pidlFromPath(c, path, &pidl, pHR) )
    {
        HRESULT hr = SHCreateShellItem(NULL, NULL, pidl, &psi);
        if ( FAILED(hr) )
        {
            oodSetSysErrCode(c->threadContext, hr);
            psi  = NULL;  // Insurance.
            *pHR = hr;
        }
        CoTaskMemFree(pidl);
    }

    return psi;
}

/**
 * Converts the specified shell item to its display name.
 *
 * @param c
 * @param psi
 * @param sigdn   Specifies the display name format.  Use SIGDN_FILESYSPATH to
 *                get the normal full path.
 * @param pHR
 *
 * @return RexxObjectPtr
 *
 * @remarks  The caller is responsible for releasing psi
 */
RexxObjectPtr shellItem2name(RexxThreadContext *c, IShellItem *psi, SIGDN sigdn, HRESULT *pHR)
{
    RexxObjectPtr result = TheNilObj;
    HRESULT       hr;
    PWSTR         pszPath;

    hr = psi->GetDisplayName(sigdn, &pszPath);
    if( SUCCEEDED(hr) )
    {
        result = unicode2StringOrNil(c, pszPath);
        CoTaskMemFree(pszPath);
    }

    *pHR = hr;
    return result;
}

/**
 * Returns the display name for the specified item ID list as a Rexx object.
 *
 * @param c
 * @param pidl
 * @param sigdn
 *
 * @return RexxObjectPtr
 *
 * @note  Sets the .systemErrorCode for errors.  The calling function should
 *        have reset the .systemErrorCode to 0.
 */
RexxObjectPtr getDisplayNameRx(RexxThreadContext *c, PIDLIST_ABSOLUTE pidl, SIGDN sigdn)
{
    RexxObjectPtr result = TheNilObj;

    IShellFolder    *psfParent;
    PCUITEMID_CHILD  pidlRelative;

    HRESULT hr = SHBindToParent(pidl, IID_PPV_ARGS(&psfParent), &pidlRelative);
    if ( SUCCEEDED(hr) )
    {
        STRRET strDispName;

        hr = psfParent->GetDisplayNameOf(pidlRelative, sigdn, &strDispName);
        if ( SUCCEEDED(hr) )
        {
            char szDisplayName[MAX_PATH];

            hr = StrRetToBuf(&strDispName, pidlRelative, szDisplayName, ARRAYSIZE(szDisplayName));
            if ( SUCCEEDED(hr) )
            {
                result = c->String(szDisplayName);
            }
            else
            {
                oodSetSysErrCode(c, hr);
            }
        }
        else
        {
            oodSetSysErrCode(c);
        }

        psfParent->Release();
    }
    else
    {
        oodSetSysErrCode(c, hr);
    }

    return result;
}


/**
 * Methods for the ooDialog .BrowseForFolder class.
 */
#define BROWSEFORFOLDER_CLASS  "BrowseForFolder"


/**
 * Ensures the BrowseForFolder CSelf pointer is not null.
 *
 * @param c
 * @param pCSelf
 *
 * @return pCBrowseForFolder
 */
static pCBrowseForFolder getBffCSelf(RexxMethodContext *c, void * pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)pCSelf;
    if ( pcbff == NULL )
    {
        baseClassInitializationException(c, BROWSEFORFOLDER_CLASS);
    }
    return pcbff;
}

/**
 * This is the Browse for Folder dialog call back function used by the
 * BrowseForFolder and SimpleFolderBrowse classes to customize the appearance
 * and behavior of the standard dialog.
 */
int32_t CALLBACK BrowseCallbackProc(HWND hwnd, uint32_t uMsg, LPARAM lp, LPARAM pData)
{
    TCHAR szDir[MAX_PATH];
    pCBrowseForFolder pbff = NULL;

    switch ( uMsg )
    {
        case BFFM_INITIALIZED:
            pbff = (pCBrowseForFolder)pData;

            if ( pbff->dlgTitle != NULL )
            {
                SetWindowText(hwnd, pbff->dlgTitle);
            }
            if ( pbff->useHint )
            {
                if ( pbff->usePathForHint && pbff->startDir != NULL )
                {
                    SetDlgItemText(hwnd, HINT_ID, pbff->startDir);
                }
                else if ( pbff->hint != NULL )
                {
                    SetDlgItemText(hwnd, HINT_ID, pbff->hint);
                }
            }

            // WParam should be TRUE when passing a path and should be FALSE
            // when passing a pidl.
            if ( pbff->startDir != NULL )
            {
                SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)pbff->startDir);
            }
            break;

        case BFFM_SELCHANGED:
            pbff = (pCBrowseForFolder)pData;

            // Set the hint static control to the currently selected path if
            // wanted.
            if ( pbff->usePathForHint )
            {
                if ( SHGetPathFromIDList((LPITEMIDLIST)lp, szDir) )
                {
                    SetDlgItemText(hwnd, HINT_ID, szDir);
                }
            }
            break;
    }
    return 0;
}

/**
 * Uses CoUninitialize() to uninit COM, when executing on the same thread as the
 * BrowseForFolder object was instantiated on.
 *
 * @param pcbff
 */
static void uninitCom(pCBrowseForFolder pcbff)
{
    if ( pcbff->countCoInitialized > 0 && pcbff->coThreadID == GetCurrentThreadId() )
    {
        do
        {
#ifdef _DEBUG
    printf("uninitCom() thread ID=%d\n", GetCurrentThreadId());
#endif
            CoUninitialize();
            pcbff->countCoInitialized--;
        } while ( pcbff->countCoInitialized > 0 );
    }
}

/**
 * Converts a string of keywords to its BIF_xx value.
 *
 * @param c
 * @param keyword
 * @param argPos
 *
 * @return uint32_t
 */
static uint32_t keywords2bif(RexxMethodContext *c, CSTRING keyword, size_t argPos)
{
    uint32_t bif = 0;

    if ( StrStrI(keyword, "BROWSEFILEJUNCTIONS") != NULL ) bif |= BIF_BROWSEFILEJUNCTIONS;
    if ( StrStrI(keyword, "BROWSEFORCOMPUTER"  ) != NULL ) bif |= BIF_BROWSEFORCOMPUTER  ;
    if ( StrStrI(keyword, "BROWSEFORPRINTER"   ) != NULL ) bif |= BIF_BROWSEFORPRINTER   ;
    if ( StrStrI(keyword, "BROWSEINCLUDEFILES" ) != NULL ) bif |= BIF_BROWSEINCLUDEFILES ;
    if ( StrStrI(keyword, "BROWSEINCLUDEURLS"  ) != NULL ) bif |= BIF_BROWSEINCLUDEURLS  ;
    if ( StrStrI(keyword, "DONTGOBELOWDOMAIN"  ) != NULL ) bif |= BIF_DONTGOBELOWDOMAIN  ;
    if ( StrStrI(keyword, "NEWDIALOGSTYLE"     ) != NULL ) bif |= BIF_NEWDIALOGSTYLE     ;
    if ( StrStrI(keyword, "NONEWFOLDERBUTTON"  ) != NULL ) bif |= BIF_NONEWFOLDERBUTTON  ;
    if ( StrStrI(keyword, "NOTRANSLATETARGETS" ) != NULL ) bif |= BIF_NOTRANSLATETARGETS ;
    if ( StrStrI(keyword, "RETURNFSANCESTORS"  ) != NULL ) bif |= BIF_RETURNFSANCESTORS  ;
    if ( StrStrI(keyword, "RETURNONLYFSDIRS"   ) != NULL ) bif |= BIF_RETURNONLYFSDIRS   ;
    if ( StrStrI(keyword, "SHAREABLE"          ) != NULL ) bif |= BIF_SHAREABLE          ;
    if ( StrStrI(keyword, "STATUSTEXT"         ) != NULL ) bif |= BIF_STATUSTEXT         ;
    if ( StrStrI(keyword, "UAHINT"             ) != NULL ) bif |= BIF_UAHINT             ;
    if ( StrStrI(keyword, "USENEWUI"           ) != NULL ) bif |= BIF_USENEWUI           ;

    return bif;
}

/**
 * Parse the BIF_xx flags and returns the matching keyword string.
 *
 * @param c
 * @param bif
 *
 * @return RexxObjectPtr
 */
static RexxObjectPtr bif2keywords(RexxMethodContext *c, uint32_t bif)
{
    char buf[512] = { '\0' };

    if ( bif & BIF_BROWSEFILEJUNCTIONS) strcat(buf, "BROWSEFILEJUNCTIONS ");
    if ( bif & BIF_BROWSEFORCOMPUTER  ) strcat(buf, "BROWSEFORCOMPUTER "  );
    if ( bif & BIF_BROWSEFORPRINTER   ) strcat(buf, "BROWSEFORPRINTER "   );
    if ( bif & BIF_BROWSEINCLUDEFILES ) strcat(buf, "BROWSEINCLUDEFILES " );
    if ( bif & BIF_BROWSEINCLUDEURLS  ) strcat(buf, "BROWSEINCLUDEURLS "  );
    if ( bif & BIF_DONTGOBELOWDOMAIN  ) strcat(buf, "DONTGOBELOWDOMAIN "  );
    if ( bif & BIF_NEWDIALOGSTYLE     ) strcat(buf, "NEWDIALOGSTYLE "     );
    if ( bif & BIF_NONEWFOLDERBUTTON  ) strcat(buf, "NONEWFOLDERBUTTON "  );
    if ( bif & BIF_NOTRANSLATETARGETS ) strcat(buf, "NOTRANSLATETARGETS " );
    if ( bif & BIF_RETURNFSANCESTORS  ) strcat(buf, "RETURNFSANCESTORS "  );
    if ( bif & BIF_RETURNONLYFSDIRS   ) strcat(buf, "RETURNONLYFSDIRS "   );
    if ( bif & BIF_SHAREABLE          ) strcat(buf, "SHAREABLE "          );
    if ( bif & BIF_UAHINT             ) strcat(buf, "UAHINT "             );
    if ( (bif & BIF_USENEWUI) == BIF_USENEWUI ) strcat(buf, "USENEWUI "   );

    if ( buf[0] != '\0' )
    {
        buf[strlen(buf) - 1] = '\0';
    }
    return c->String(buf);
}

/**
 * Sets one of the text attributes of the BrowseForFolder object.
 *
 * If rxValue is the .nil object or the empty string, then the CSelf value of
 * the attribute is set to NULL, which basically causes the default value to be
 * used by the browse dialog.
 *
 * The attribute could already have a malloc'd string, in which case we need to
 * be sure and free it.
 *
 * @param c
 * @param pcbff
 * @param rxValue
 * @param type
 */
void setTextAttribute(RexxMethodContext *c, pCBrowseForFolder pcbff, RexxObjectPtr rxValue, BffAttributeType type)
{
    char    *newVal  = NULL;
    CSTRING  text    = NULL;
    size_t   len     = 0;

    if ( rxValue != TheNilObj )
    {
        text = c->ObjectToStringValue(rxValue);
        len  = strlen(text);
        if ( len == 0 )
        {
            text = NULL;
        }
    }

    if ( text != NULL )
    {
        newVal = (char *)LocalAlloc(LPTR, len + 1);
        if ( newVal == NULL )
        {
            outOfMemoryException(c->threadContext);
            return;
        }
        else
        {
            strcpy(newVal, text);
        }
    }

    switch ( type )
    {
        case DlgBanner :
            safeLocalFree(pcbff->banner);
            pcbff->banner = newVal;
            break;

        case DlgHint :
            safeLocalFree(pcbff->hint);
            pcbff->hint = newVal;
            if ( newVal != NULL )
            {
                pcbff->useHint = true;
            }
            else
            {
                pcbff->useHint = pcbff->usePathForHint ? true : false;
            }

            if ( pcbff->useHint )
            {
                pcbff->bifFlags |= BIF_UAHINT;
            }
            else
            {
                pcbff->bifFlags &= ~BIF_UAHINT;
            }
            break;

        case DlgStartDir :
            safeLocalFree(pcbff->startDir);
            pcbff->startDir = newVal;
            break;

        case DlgTitle :
            safeLocalFree(pcbff->dlgTitle);
            pcbff->dlgTitle = newVal;
            break;

        default :
            break;
    }
}

/**
 * Invokes CoUninitialize() once on the current thread. However, if the current
 * thread is the same thread as instantiation of the BrowseForFolder object,
 * then the inovocation will be dependent on the state flags.
 *
 * @param pcbff
 *
 * @return True if CoUnitialize() was invoked on the original thread of
 *         instantiation, otherwise false.
 */
static RexxObjectPtr releaseCOM(pCBrowseForFolder pcbff)
{
    if ( GetCurrentThreadId() == pcbff->coThreadID )
    {
        uninitCom(pcbff);
        return TheTrueObj;
    }
    else
    {
        CoUninitialize();
        return TheFalseObj;
    }
}


/**
 * Handles a CoInitializeEx() request on the same thread as the BrowseForFolder
 * object was created on.
 *
 * @param c
 * @param pcbff
 *
 * @return True for a successful CoInitializeEx() otherwise false.
 *
 * @assumes That the caller has already checked pcbff->coThreadID and we are in
 *          fact in the same thread.
 */
static RexxObjectPtr reInitCOM(RexxMethodContext *c, pCBrowseForFolder pcbff)
{
    if ( pcbff->countCoInitialized > 0 )
    {
        return TheFalseObj;
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    oodSetSysErrCode(c->threadContext, hr);
    if ( hr == S_OK )
    {
        pcbff->countCoInitialized++;
        return TheTrueObj;
    }
    else if ( hr == S_FALSE )
    {
        // This should be impossible.
        CoUninitialize();
    }
    return TheFalseObj;
}


/**
 * Puts up the Browse for Folder dialog using the supplied browse info
 * structure.
 *
 * @param pBI
 * @param returnPath
 *
 * @return RexxObjectPtr
 */
static RexxObjectPtr folderBrowse(RexxMethodContext *context, PBROWSEINFO pBI, bool returnPath)
{
	LPITEMIDLIST  pidl = NULL;
    TCHAR         path[MAX_PATH];
    RexxObjectPtr rxResult = context->NullString();    // User canceled.

    pidl = SHBrowseForFolder(pBI);
    if ( pidl != NULL )
    {
        if ( returnPath )
        {
            if ( SHGetPathFromIDList(pidl, path) )
            {
                rxResult = context->String(path);
            }
            else
            {
                rxResult = context->Nil();
            }
            CoTaskMemFree(pidl);
        }
        else
        {
            rxResult = context->NewPointer(pidl);
        }
    }

    if ( ! returnPath && rxResult == context->NullString() )
    {
        rxResult = TheNilObj;
    }
    return rxResult;
}

/**
 * Fills in the browse info struct using the attributes of this BrowseForFolder
 * object.
 *
 * @param context
 * @param pBI
 * @param pcbff
 *
 * @remarks  We treat a few things about the BIF_xx flags special here.  The
 *           user can change the flags to what they want, but we always use
 *           BIF_NEWDIALOGSTYLE.  So, we always add it here as a precaution in
 *           case it was dropped.  A similar thing for BIF_UAHINT, if useHint is
 *           true, we always add it.
 *
 *           In the code we try to keep bifFlags accurate so that if the user
 *           accesses the attribute, they get the right info.  But, the logic is
 *           a little convoluted ... so we take a little extra precaution.
 */
static void fillInBrowseInfo(RexxMethodContext *context, PBROWSEINFO pBI, pCBrowseForFolder pcbff)
{
    pBI->hwndOwner = pcbff->hOwner;
    pBI->ulFlags   = pcbff->bifFlags;

    pBI->ulFlags |= BIF_NEWDIALOGSTYLE;

    if ( pcbff->useHint )
    {
        pBI->ulFlags |= BIF_UAHINT;
    }

    if ( pcbff->banner != NULL )
    {
        pBI->lpszTitle = pcbff->banner;
    }

    pBI->pidlRoot = pcbff->root;

    pBI->lpfn   = BrowseCallbackProc;
    pBI->lParam = (LPARAM)pcbff;
}

/**
 * Common code for the getFolder() and getItemIDList() Rexx methods.  The only
 * difference between the 2 methods is that getFolder() returns the  selected
 * folder as a fils system path and getItemIDList() returns the raw pidl.
 *
 * @param c
 * @param pCSelf
 * @param reuse
 * @param getPath
 *
 * @return RexxObjectPtr
 *
 * @remarks  By default we do the CoUnitialize after the browse for folder
 *           dialog is closed.  The user should be advised in the docs that the
 *           simplest thing to do is instantiate, configure, and get the folder
 *           on one thread.  For other usage patterns, the user then becomes
 *           responsible for matching CoInitializeEx() / CoUnitialize().  The
 *           BrowseForFolder class has sufficient methods for that to be done in
 *           Rexx, but the user will need to take care.
 */
RexxObjectPtr bffGetFolderOrIDL(RexxMethodContext *c, void *pCSelf, logical_t reuse, bool getPath)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(c, pCSelf);
    if ( pcbff == NULL )
    {
        return NULLOBJECT;
    }

    BROWSEINFO   bi = { 0 };

    fillInBrowseInfo(c, &bi, pcbff);

    // If second arg is true - return path.
    RexxObjectPtr result = folderBrowse(c, &bi, getPath);

    if ( ! reuse )
    {
        releaseCOM(pcbff);
    }

    return result;
}

/** BrowseForFolder::banner                  [attribute]
 */
RexxMethod1(RexxObjectPtr, bff_banner, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff != NULL && pcbff->banner != NULL )
    {
        return context->String(pcbff->banner);
    }
    return context->NullString();
}
RexxMethod2(RexxObjectPtr, bff_setBanner, RexxObjectPtr, hint, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
        setTextAttribute(context, pcbff, hint, DlgBanner);
    }
    return NULLOBJECT;
}

/** BrowseForFolder::dlgTitle                  [attribute]
 */
RexxMethod1(RexxObjectPtr, bff_dlgTitle, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff != NULL && pcbff->dlgTitle != NULL )
    {
        return context->String(pcbff->dlgTitle);
    }
    return context->NullString();
}
RexxMethod2(RexxObjectPtr, bff_setDlgTitle, RexxObjectPtr, dlgTitle, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
        setTextAttribute(context, pcbff, dlgTitle, DlgTitle);
    }
    return NULLOBJECT;
}

/** BrowseForFolder::hint                  [attribute]
 */
RexxMethod1(RexxObjectPtr, bff_hint, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff != NULL && pcbff->hint != NULL )
    {
        return context->String(pcbff->hint);
    }
    return context->NullString();
}
RexxMethod2(RexxObjectPtr, bff_setHint, RexxObjectPtr, hint, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
        setTextAttribute(context, pcbff, hint, DlgHint);
    }
    return NULLOBJECT;
}

/** BrowseForFolder::initialThread               [attribute]
 *
 *  Returns the thread ID of the thread this BrowseForFolder object was
 *  instantiated on, the ID of the thread that COM was initialized on.
 */
RexxMethod1(uint32_t, bff_initialThread, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
        return pcbff->coThreadID;
    }
    return 0;
}

/** BrowseForFolder:options                  [attribute]
 */
RexxMethod1(RexxObjectPtr, bff_options, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
        return bif2keywords(context, pcbff->bifFlags);
    }
    return context->NullString();
}
RexxMethod2(RexxObjectPtr, bff_setOptions, CSTRING, opts, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
        pcbff->bifFlags = keywords2bif(context, opts, 1) | BIF_NEWDIALOGSTYLE;
        if ( pcbff->useHint )
        {
            pcbff->bifFlags |= BIF_UAHINT;
        }
        else
        {
            pcbff->bifFlags &= ~BIF_UAHINT;
        }
    }
    return NULLOBJECT;
}

/** BrowseForFolder:owner                  [attribute]
 */
RexxMethod1(RexxObjectPtr, bff_owner, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
        return pcbff->rexxOwner == NULLOBJECT ? TheNilObj : pcbff->rexxOwner;
    }
    return TheNilObj;
}
RexxMethod2(RexxObjectPtr, bff_setOwner, RexxObjectPtr, owner, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
        if ( owner == TheNilObj )
        {
            pcbff->rexxOwner = NULL;
            pcbff->hOwner    = NULL;
        }
        else
        {
            pCPlainBaseDialog pcpbd = requiredPlainBaseDlg(context, owner, 1);
            if ( pcpbd != NULL )
            {
                pcbff->rexxOwner = owner;
                pcbff->hOwner    = pcpbd->hDlg;
            }
        }
    }
    return NULLOBJECT;
}

/** BrowseForFolder::root                  [attribute]
 */
RexxMethod1(RexxObjectPtr, bff_root, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff != NULL && pcbff->root != NULL )
    {
        return context->NewPointer(pcbff->root);
    }
    return TheNilObj;
}
RexxMethod2(RexxObjectPtr, bff_setRoot, RexxObjectPtr, root, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff == NULL )
    {
        return NULLOBJECT;
    }

    // Setting root to the .nil ojbect, or the empty string is the way to remove
    // a root setting.
    LPITEMIDLIST pidl = NULL;

    if ( root == TheNilObj )
    {
        ;  // Do not need to do anything pidl is already NULL.
    }
    else if ( context->IsPointer(root) )
    {
        pidl = (LPITEMIDLIST)context->PointerValue((RexxPointerObject)root);
    }
    else
    {
        CSTRING idl = context->ObjectToStringValue(root);

        // If the empty string, we do nothing, pidl is already NULL
        if ( *idl != '\0' )
        {
            HRESULT hr = S_OK;

            pidl = getPidlFromString(context, idl, 1, false, &hr);
            if ( pidl == NULL )
            {
                return NULLOBJECT;
            }
        }
    }

    if ( pcbff->root != NULL )
    {
        CoTaskMemFree((LPVOID)pcbff->root);
    }
    pcbff->root = pidl;

    return NULLOBJECT;
}

/** BrowseForFolder::startDir                  [attribute]
 */
RexxMethod1(RexxObjectPtr, bff_startDir, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = getBffCSelf(context, pCSelf);
    if ( pcbff != NULL && pcbff->startDir != NULL )
    {
        return context->String(pcbff->startDir);
    }
    return context->NullString();
}
RexxMethod2(RexxObjectPtr, bff_setStartDir, RexxObjectPtr, startDir, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
        setTextAttribute(context, pcbff, startDir, DlgStartDir);
    }
    return NULLOBJECT;
}

/** BrowseForFolder::usePathForHint                  [attribute]
 */
RexxMethod1(RexxObjectPtr, bff_usePathForHint, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
        return context->Logical(pcbff->usePathForHint);
    }
    return context->NullString();
}
RexxMethod2(RexxObjectPtr, bff_setUsePathForHint, logical_t, usePath, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
        if ( usePath )
        {
            pcbff->usePathForHint = true;
            pcbff->useHint = true;
        }
        else
        {
            pcbff->usePathForHint = false;
            if ( pcbff->hint == NULL )
            {
                pcbff->useHint = false;
            }
        }
    }
    return NULLOBJECT;
}

/** BrowseForFolder::uninit()
 *
 * Does clean up for this BrowseForFolder.  Frees the root PIDL and calls the
 * common uninit routine.
 */
RexxMethod1(RexxObjectPtr, bff_uninit, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = getBffCSelf(context, pCSelf);
    if ( pcbff != NULL )
    {
#ifdef _DEBUG
    printf("bff_uninit()\n");
#endif
        if ( pcbff->root != NULL )
        {
            CoTaskMemFree((LPVOID)pcbff->root);
            pcbff->root = NULL;
        }

        safeLocalFree(pcbff->banner);
        safeLocalFree(pcbff->dlgTitle);
        safeLocalFree(pcbff->hint);
        safeLocalFree(pcbff->startDir);

        uninitCom(pcbff);
    }
    return NULLOBJECT;
}

/** BrowseForFolder::init()
 *
 * Initializes a BrowseForFolder object.  The title, banner, and hint strings
 * all have default values that are different from the Windows default values.
 * If the user wants to have the default Windows values, they specify that by
 * setting them to either .nil or the empty string.
 *
 * @remarks  MSDN says you have to use CoInitialize or SHBrowseForFolder will
 *           fail.  But it seems to work without it?
 *
 *           If a BrowseForFolder object has been previously instantiated and
 *           COM not released then CoInitializeEx will return S_FALSE, meaning
 *           COM is already initialized on this thread. We immediately do a
 *           CoUninitialize to decrement the reference counter for this case.
 */
RexxMethod4(RexxObjectPtr, bff_init, OPTIONAL_RexxObjectPtr, title, OPTIONAL_RexxObjectPtr, banner, OPTIONAL_RexxObjectPtr, hint,
            OPTIONAL_RexxObjectPtr, startDir)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(CBrowseForFolder));
    context->SetObjectVariable("CSELF", obj);

    pCBrowseForFolder pcbff = (pCBrowseForFolder)context->BufferData(obj);
    memset(pcbff, 0, sizeof(CBrowseForFolder));

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    #ifdef _DEBUG
        printf("CoInitializeEx returns: 0x%08x\n", hr);
    #endif
    if ( hr == S_FALSE )
    {
        CoUninitialize();
        hr = S_OK;
    }
    else if ( hr == S_OK )
    {
        pcbff->countCoInitialized = 1;
        pcbff->coThreadID = GetCurrentThreadId();
    }
    else
    {
        systemServiceExceptionComCode(context->threadContext, COM_API_FAILED_MSG, "CoInitializeEx", hr);
        return NULLOBJECT;
    }

    pcbff->bifFlags = DEFAULT_BIF_FLAGS;

    // Set the default attributes for the browser.
    if ( argumentOmitted(1) )
    {
        title = context->String(BFF_TITLE);
    }
    setTextAttribute(context, pcbff, title, DlgTitle);

    if ( argumentOmitted(2) )
    {
        banner = context->String(BFF_BANNER);
    }
    setTextAttribute(context, pcbff, banner, DlgBanner);

    if ( argumentOmitted(3) )
    {
        hint = context->String(BFF_HINT);
    }
    setTextAttribute(context, pcbff, hint, DlgHint);

    if ( argumentOmitted(4) )
    {
        startDir = context->String(BFF_STARTDIR);
    }
    setTextAttribute(context, pcbff, startDir, DlgStartDir);

	LPITEMIDLIST pidl = NULL;
    if ( pidlForSpecialFolder(CSIDL_DRIVES, &pidl, &hr) )
    {
        pcbff->root = pidl;
    }
    else
    {
        systemServiceException(context->threadContext, NO_ITEMID_MSG, "CSIDL_DRIVES");
    }

    return NULLOBJECT;
}

/** BrowseForFolder::getDisplayName()
 *
 * Returns the display name for an item ID list.
 *
 * @param   pidl  [required]  A handle, a pointer, to an item ID list
 *
 * @return  Returns the display name for the item ID list on success, otherwise
 *          returns the .nil object.
 *
 * @note  Sets the system error code.
 *
 *        If sigdn is omitted this method should return some name for any shell
 *        item, virtual folder or not.  In this case the method first tries to
 *        get a full path name.  But, if that fails the method tries again using
 *        the flag for destop absolute editing.
 *
 *        If sigdn is used, then the method uses the specified sigdn flag as is
 *        and returns whatever the Shell returns.
 */
RexxMethod3(RexxObjectPtr, bff_getDisplayName, POINTER, pidl, OPTIONAL_CSTRING, _sigdn, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr result = TheNilObj;

    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff == NULL )
    {
        return NULLOBJECT;
    }

    if ( argumentOmitted(2) )
    {
        result = getDisplayNameRx(context->threadContext, (PIDLIST_ABSOLUTE)pidl, SIGDN_FILESYSPATH);
        if ( result == TheNilObj )
        {
            result = getDisplayNameRx(context->threadContext, (PIDLIST_ABSOLUTE)pidl, SIGDN_DESKTOPABSOLUTEEDITING);
        }
    }
    else
    {
        SIGDN sigdn;

        if ( keyword2sigdn(context, _sigdn, &sigdn, 2) )
        {
            result = getDisplayNameRx(context->threadContext, (PIDLIST_ABSOLUTE)pidl, sigdn);
        }
    }

    return result;
}


/** BrowseForFolder::getFolder()
 *
 * Puts up the customized Browse for Folder dialog using the atributes set for
 * this instance.
 *
 * @param   reuse  [OPTIONAL]  By default COM is unitialized on the return from
 *          this methd.  If reuse is true, COM is not unitialized and it becomes
 *          the programmer's responsibility to invoke release when the
 *          BrowseForFolder object is no longer needed.
 *
 * @return  Returns the fully qualified path of the folder the user selects, or
 *          the empty string if the user cancels.  In addition, if the user
 *          selects a virtual folder that has no file system path, .nil is
 *          returned.
 */
RexxMethod2(RexxObjectPtr, bff_getFolder, OPTIONAL_logical_t, reuse, CSELF, pCSelf)
{
    return bffGetFolderOrIDL(context, pCSelf, reuse, true);
}


/** BrowseForFolder::getItemIDList()
 *
 * Puts up the customized Browse for Folder dialog using the atributes set for
 * this instance.
 *
 * @param   reuse  [OPTIONAL]  By default COM is unitialized on the return from
 *          this methd.  If reuse is true, COM is not unitialized and it becomes
 *          the programmer's responsibility to invoke release when the
 *          BrowseForFolder object is no longer needed.
 *
 * @return  Returns the pointer to item ID list the user selected, or the .nil
 *          object if the user cancels the dialog.
 */
RexxMethod2(RexxObjectPtr, bff_getItemIDList, OPTIONAL_logical_t, reuse, CSELF, pCSelf)
{
    return bffGetFolderOrIDL(context, pCSelf, reuse, false);
}


/** BrowseForFolder::initCOM
 *
 *  Invokes CoInitalizeEx() on the current thread, with some forethought.  If
 *  the current thread is the initial thread that this object was instantiated
 *  on, then the invocation is only done if needed.
 *
 *  If we are on the initial thread and the countCoInitialized is 0 the
 *  initialize is done and the count incremented.  If the count is not 0, the
 *  initialize is skipped.
 *
 *  If we are on another thread, the CoInitializeEx is always done, but the
 *  return is checked.  If it is S_FALSE, that indicates the CoInitializeEx has
 *  already been done on this thread and we immediately do an unitialize to keep
 *  things balanced.
 *
 *  If the user makes use of these methods, he assumes the responsibility of
 *  keeping CoInitializedEx / CoUnitialize() in balance *and* in invoking the
 *  releaseCOM method when he is done with this object.
 */
RexxMethod1(RexxObjectPtr, bff_initCOM, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff == NULL )
    {
        return NULLOBJECT;
    }

    if ( pcbff->coThreadID == GetCurrentThreadId() )
    {
        return reInitCOM(context, pcbff);
    }

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    oodSetSysErrCode(context->threadContext, hr);
    if ( hr == S_OK )
    {
        return TheTrueObj;
    }
    else if ( hr == S_FALSE )
    {
        CoUninitialize();
    }
    return TheFalseObj;
}


/** BrowseForFolder::releaseCom
 *
 *  Calls CoUninitialize() on the current thread.  This is to allow the user to
 *  reuse the BrowseForFolder object.
 *
 *  By default CoUninitialize() will be called from getFolder() and the object
 *  will be 'dead'.  The user can signal getFolder() to not call
 *  CoUninitialize(), in which case, it becomes the user's responsibility to
 *  call release.
 *
 *  @return True if the thread CoUnitialize() was called on was the same thread
 *          as this object was first created on.  Returns false if the thread
 *          CoUnitialize() was called on another thread.  CoUnitialize() is
 *          *always* called.  Unless we are on the same thread as this object
 *          was instantiated on and the count initialized indicates we have
 *          already closed out COM.
 */
RexxMethod1(RexxObjectPtr, bff_releaseCOM, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff == NULL )
    {
        return NULLOBJECT;
    }
    return releaseCOM(pcbff);
}

/** BrowseForFolder::releaseItemIDList()
 *
 *  Releases a handle, a pointer, to an item ID list.  This must be done for
 *  each handle returned from the getItemIDList() method and must *not* be done
 *  for the root item ID list.
 *
 *  @note CoTaskMemFree() is specifically documented as not needing a call to
 *        CoInitializeEx() before being used.
 *
 *  @remarks  A user may set the root attriubte to a pidl they obtained through
 *            getItemIDList(). If they did not read the docs closely, they may
 *            think they still need to release that pidl.  This can cause a
 *            crash when the root attriubte is freed again in uninit().  So, we
 *            protect against that.
 */
RexxMethod2(RexxObjectPtr, bff_releaseItemIDList, POINTER, pidl, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff == NULL )
    {
        return NULLOBJECT;
    }
    if ( pcbff->root == pidl )
    {
        pcbff->root = NULL;
    }

    CoTaskMemFree(pidl);

    return TheZeroObj;
}

/**
 * Example code for GetDisplayName() using SHCreateItemFromIDList to create a
 * IShellItem from a IDL.   Worked Windows 7 64 bit
 *
 * On XP, having SHCreateItemFromIDlist in the code prevents shell.dll from
 * being loaded, which prevents oodialog.dll from being loaded.  This can be
 * worked around using GetProcAddress().  But, that would be cumbersome if using
 * a number of the 'Vista or later' shell APIs.
 *
 * This was integrated into the BrowseForFolder::getDisplayName method.  It
 * worked fine with 64-bit ooRexx on 64-bit Windows.  But, it crashed under
 * 32-bit ooRexx on Windows 64-bit.  From debugging, it seems the typedef for
 * SHCreateItemFromIDListPROC was not correct.  The code worked fine in all
 * cases, if the direct SHCreateItemFromIDList() was used.
 *
 * Since the problem was not debugged, BrowseForFolder::getDisplayName() was
 * switched to use another algorithm that works on XP.
 */
#if 0
RexxMethod1(RexxObjectPtr, bff_test, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff == NULL )
    {
        return NULLOBJECT;
    }

    if ( ! requiredOS(context, "bff_test SHCreateItemFromIDList()", "Vista", Vista_OS) )
    {
        return NULLOBJECT;
    }

    PIDLIST_ABSOLUTE pidlSystem;

    HINSTANCE hinst = LoadLibrary(TEXT("Shell32.dll"));
    if ( hinst == NULL )
    {
        printf("LoadLibrary() failed\n");
        return NULLOBJECT;
    }
    SHCreateItemFromIDListPROC pSHCreateItemFromIDList;

    pSHCreateItemFromIDList = (SHCreateItemFromIDListPROC)GetProcAddress(hinst, "SHCreateItemFromIDList");
    if ( pSHCreateItemFromIDList == NULL )
    {
        printf("GetProcAddress() failed\n");
        return NULLOBJECT;
    }

    HRESULT hr = SHGetFolderLocation(NULL, CSIDL_PRINTERS, NULL, NULL, &pidlSystem);
    if ( SUCCEEDED(hr) )
    {
        IShellItem *psi;

        hr = pSHCreateItemFromIDList(pidlSystem, IID_PPV_ARGS(&psi));
        if ( SUCCEEDED(hr) )
        {
            PWSTR pszName;
            hr = psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEEDITING, &pszName);
            if (SUCCEEDED(hr))
            {
                wprintf(L"Desktop Absolute Editing Display - %s\n", pszName);
                CoTaskMemFree(pszName);
            }
            else
            {
                printHResultErr("GetDisplayName", hr);
            }
            psi->Release();
        }
        else
        {
            printHResultErr("SHCreateItemFromIDList", hr);
        }

        ILFree(pidlSystem);
    }
    else
    {
        printHResultErr("ShGetFolderLocation", hr);
    }

    return NULLOBJECT;
}


/**
 * Trial code IFileDialog used to browse for folder.  Works Win 7 64 bit.
 *
 * Fails on XP because CLSID_FileOpenDialog is not available.  A simple
 * requiredOS() fixes that problem.
 *
 */
RexxMethod1(RexxObjectPtr, bff_test, CSELF, pCSelf)
{
    pCBrowseForFolder pcbff = (pCBrowseForFolder)getBffCSelf(context, pCSelf);
    if ( pcbff == NULL )
    {
        return NULLOBJECT;
    }
    printf("bff_test() enter\n");

    if ( ! requiredOS(context, Vista_OS, "bff_test CLSID_FileOpenDialog", "Vista") )
    {
        return NULLOBJECT;
    }

    IFileDialog *pfd;

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if ( SUCCEEDED(hr) )
    {
        DWORD dwOptions;

        hr = pfd->GetOptions(&dwOptions);
        if ( SUCCEEDED(hr) )
        {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);

            hr = pfd->Show(NULL);
            if ( SUCCEEDED(hr) )
            {
                IShellItem *psi;

                hr = pfd->GetResult(&psi);
                if ( SUCCEEDED(hr) )
                {
                    PWSTR pszPath;

                    hr = psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszPath);
                    if( SUCCEEDED(hr) )
                    {
                        wprintf(L"Got folder - %s\n", pszPath);
                        CoTaskMemFree(pszPath);
                    }
                    else
                    {
                        printHResultErr("GetDisplayName", hr);
                    }
                    psi->Release();
                }
                else
                {
                    printHResultErr("GetResult", hr);
                }
            }
            else
            {
                printHResultErr("Show", hr);
            }
        }
        else
        {
            printHResultErr("GetOptions", hr);
        }
        pfd->Release();
    }
    else
    {
        printHResultErr("CoCreateInstance", hr);
    }

    return NULLOBJECT;
}
#endif


/**
 * Example code for GetDisplayName() that does not use SHCreateItemFromIDList.
 * This code has no problems on XP, so BrowseForFolder::getDisplayName() uses
 * this code.
 *
 */
RexxMethod1(RexxObjectPtr, bff_test, CSELF, pCSelf)
{
    PIDLIST_ABSOLUTE pidlSystem;

    HRESULT hr = SHGetFolderLocation(NULL, CSIDL_PRINTERS, NULL, NULL, &pidlSystem);
    if ( SUCCEEDED(hr) )
    {
        IShellFolder *psfParent;
        PCUITEMID_CHILD pidlRelative;
        hr = SHBindToParent(pidlSystem, IID_PPV_ARGS(&psfParent), &pidlRelative);
        if ( SUCCEEDED(hr) )
        {
            STRRET strDispName;
            hr = psfParent->GetDisplayNameOf(pidlRelative, SIGDN_DESKTOPABSOLUTEEDITING, &strDispName);
            if ( SUCCEEDED(hr) )
            {
                WCHAR szDisplayName[MAX_PATH];
                hr = StrRetToBufW(&strDispName, pidlRelative, szDisplayName, ARRAYSIZE(szDisplayName));
                if ( SUCCEEDED(hr) )
                {
                    wprintf(L"Got folder SHGDN_NORMAL - %s\n", szDisplayName);
                }
            }
            psfParent->Release();
        }
        CoTaskMemFree(pidlSystem);
    }

    return TheZeroObj;
}

/**
 * Methods for the ooDialog .SimpleFolderBrowse class.
 */
#define SIMPLEFOLDERBROWSE_CLASS  "SimpleFolderBrowse"


/** SimpleFolderBrowse::getFolder()  [class method]
 *
 * SimpleFolderBrowse has only 1 method, getFolder().  The method is a class
 * method and would be used like so:
 *
 * folder = .SimplerFolderBrowse(...)
 * say 'User picked:' folder
 *
 * The arguments to the method can set non-default values for the dialog title,
 * dialog banner, dialog hint, dialog start directory, and root of the tree.
 * With no arguments the default SHBrowseForFolder is shown.
 *
 */
RexxMethod6(RexxObjectPtr, sfb_getFolder, OPTIONAL_CSTRING, title, OPTIONAL_CSTRING, banner, OPTIONAL_CSTRING, hint,
            OPTIONAL_CSTRING, startDir, OPTIONAL_CSTRING, root, OPTIONAL_RexxObjectPtr, owner)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if ( FAILED(hr) )
    {
        systemServiceExceptionComCode(context->threadContext, COM_API_FAILED_MSG, "CoInitializeEx", hr);
        return NULLOBJECT;
    }

    pCBrowseForFolder pcbff  = NULL;
    RexxObjectPtr     result = TheNilObj;
    BROWSEINFO        bi     = { 0 };

    if ( argumentExists(1) || argumentExists(3) || argumentExists(4) )
    {
        pcbff = (pCBrowseForFolder)LocalAlloc(LPTR, sizeof(CBrowseForFolder));
        if ( pcbff == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }

        bi.lParam = (LPARAM)pcbff;
        bi.lpfn   = BrowseCallbackProc;
    }

    bi.ulFlags = DEFAULT_BIF_FLAGS;

    if ( argumentExists(1) )
    {
        if ( *title != '\0' )
        {
            pcbff->dlgTitle = (char *)title;
        }
    }

    if ( argumentExists(2) )
    {
        if ( *banner != '\0' )
        {
            bi.lpszTitle = banner;
        }
    }

    if ( argumentExists(3) )
    {
        bi.ulFlags |= BIF_UAHINT;

        pcbff->useHint = true;
        if ( StrStrI("PATH", hint) != NULL )
        {
            pcbff->usePathForHint = true;
            pcbff->hint           = "";
        }
        else
        {
            pcbff->hint = (char *)hint;
        }
    }

    if ( argumentExists(4) )
    {
        if ( *startDir != '\0' )
        {
            pcbff->startDir = (char *)startDir;
        }
    }

    if ( argumentExists(5) )
    {
        bi.pidlRoot = getPidlFromString(context, root, 5, false, &hr);
        if ( bi.pidlRoot == NULL )
        {
            goto done_out;
        }
    }

    if ( argumentExists(6) )
    {
        pCPlainBaseDialog pcpbd = requiredPlainBaseDlg(context, owner, 1);
        if ( pcpbd == NULL )
        {
            goto done_out;
        }

        bi.hwndOwner = pcpbd->hDlg;
    }

    // If the user only sent empty strings as arguments, we do not need the
    // call back function
    if ( pcbff && pcbff->dlgTitle == NULL && pcbff->hint == NULL && pcbff->startDir == NULL )
    {
        bi.lParam = NULL;
        bi.lpfn   = NULL;
    }

     result = folderBrowse(context, &bi, true);

done_out:
    if ( bi.pidlRoot != NULL )
    {
        CoTaskMemFree((LPVOID)bi.pidlRoot);
    }
    safeLocalFree(pcbff);
    CoUninitialize();

    return result;
}



/**
 * Methods for the ooDialog .CommonItemDialog class.
 */
#define COMMONITEMDIALOG_CLASS  "CommonItemDialog"


/**
 * Ensures the CommonItemDialog CSelf pointer is not null and sets the HRESULT
 * value.
 *
 * @param c
 * @param pCSelf
 * @param pHR
 *
 * @return pCCommonItemDialog
 */
static pCCommonItemDialog getCidCSelf(RexxMethodContext *c, void * pCSelf, HRESULT *pHR)
{
    oodResetSysErrCode(c->threadContext);
    *pHR = S_OK;

    pCCommonItemDialog pccid = (pCCommonItemDialog)pCSelf;
    if ( pccid == NULL )
    {
        *pHR = ERROR_INVALID_FUNCTION;
        oodSetSysErrCode(c->threadContext, ERROR_INVALID_FUNCTION);
        baseClassInitializationException(c, COMMONITEMDIALOG_CLASS);
    }
    else if ( pccid->pfd == NULL )
    {
        pccid = NULL;
        *pHR  = ERROR_INVALID_FUNCTION;
        oodSetSysErrCode(c->threadContext, ERROR_INVALID_FUNCTION);
        userDefinedMsgException(c, NO_IFILEDIALOG_POINTER);
    }
    return pccid;
}

/**
 * Parse the FOS_xx flags and returns the matching keyword string.
 *
 * @param c
 * @param fos
 *
 * @return RexxObjectPtr
 */
static RexxObjectPtr fos2keywords(RexxMethodContext *c, FILEOPENDIALOGOPTIONS fos)
{
    char buf[512] = { '\0' };

    if ( fos & FOS_ALLNONSTORAGEITEMS) strcat(buf, "ALLNONSTORAGEITEMS ");
    if ( fos & FOS_ALLOWMULTISELECT  ) strcat(buf, "ALLOWMULTISELECT "  );
    if ( fos & FOS_CREATEPROMPT      ) strcat(buf, "CREATEPROMPT "      );
    if ( fos & FOS_DEFAULTNOMINIMODE ) strcat(buf, "DEFAULTNOMINIMODE " );
    if ( fos & FOS_DONTADDTORECENT   ) strcat(buf, "DONTADDTORECENT "   );
    if ( fos & FOS_FILEMUSTEXIST     ) strcat(buf, "FILEMUSTEXIST "     );
    if ( fos & FOS_FORCEFILESYSTEM   ) strcat(buf, "FORCEFILESYSTEM "   );
    if ( fos & FOS_FORCEPREVIEWPANEON) strcat(buf, "FORCEPREVIEWPANEON ");
    if ( fos & FOS_FORCESHOWHIDDEN   ) strcat(buf, "FORCESHOWHIDDEN "   );
    if ( fos & FOS_HIDEMRUPLACES     ) strcat(buf, "HIDEMRUPLACES "     );
    if ( fos & FOS_HIDEPINNEDPLACES  ) strcat(buf, "HIDEPINNEDPLACES "  );
    if ( fos & FOS_NOCHANGEDIR       ) strcat(buf, "NOCHANGEDIR "       );
    if ( fos & FOS_NODEREFERENCELINKS) strcat(buf, "NODEREFERENCELINKS ");
    if ( fos & FOS_NOREADONLYRETURN  ) strcat(buf, "NOREADONLYRETURN "  );
    if ( fos & FOS_NOTESTFILECREATE  ) strcat(buf, "NOTESTFILECREATE "  );
    if ( fos & FOS_NOVALIDATE        ) strcat(buf, "NOVALIDATE "        );
    if ( fos & FOS_OVERWRITEPROMPT   ) strcat(buf, "OVERWRITEPROMPT "   );
    if ( fos & FOS_PATHMUSTEXIST     ) strcat(buf, "PATHMUSTEXIST "     );
    if ( fos & FOS_PICKFOLDERS       ) strcat(buf, "PICKFOLDERS "       );
    if ( fos & FOS_SHAREAWARE        ) strcat(buf, "SHAREAWARE "        );
    if ( fos & FOS_STRICTFILETYPES   ) strcat(buf, "STRICTFILETYPES "   );

    if ( buf[0] != '\0' )
    {
        buf[strlen(buf) - 1] = '\0';
    }
    return c->String(buf);
}

/**
 * Converts a string of keywords to its FOS_xx value.
 *
 * @param c
 * @param keywords
 * @param argPos
 *
 * @return uint32_t
 */
static FILEOPENDIALOGOPTIONS keywords2fos(RexxMethodContext *c, CSTRING keywords, size_t argPos)
{
    FILEOPENDIALOGOPTIONS fos = 0;

    if ( StrStrI(keywords, "ALLNONSTORAGEITEMS") != NULL ) fos |= FOS_ALLNONSTORAGEITEMS;
    if ( StrStrI(keywords, "ALLOWMULTISELECT"  ) != NULL ) fos |= FOS_ALLOWMULTISELECT  ;
    if ( StrStrI(keywords, "CREATEPROMPT"      ) != NULL ) fos |= FOS_CREATEPROMPT      ;
    if ( StrStrI(keywords, "DEFAULTNOMINIMODE" ) != NULL ) fos |= FOS_DEFAULTNOMINIMODE ;
    if ( StrStrI(keywords, "DONTADDTORECENT"   ) != NULL ) fos |= FOS_DONTADDTORECENT   ;
    if ( StrStrI(keywords, "FILEMUSTEXIST"     ) != NULL ) fos |= FOS_FILEMUSTEXIST     ;
    if ( StrStrI(keywords, "FORCEFILESYSTEM"   ) != NULL ) fos |= FOS_FORCEFILESYSTEM   ;
    if ( StrStrI(keywords, "FORCEPREVIEWPANEON") != NULL ) fos |= FOS_FORCEPREVIEWPANEON;
    if ( StrStrI(keywords, "FORCESHOWHIDDEN"   ) != NULL ) fos |= FOS_FORCESHOWHIDDEN   ;
    if ( StrStrI(keywords, "HIDEMRUPLACES"     ) != NULL ) fos |= FOS_HIDEMRUPLACES     ;
    if ( StrStrI(keywords, "HIDEPINNEDPLACES"  ) != NULL ) fos |= FOS_HIDEPINNEDPLACES  ;
    if ( StrStrI(keywords, "NOCHANGEDIR"       ) != NULL ) fos |= FOS_NOCHANGEDIR       ;
    if ( StrStrI(keywords, "NODEREFERENCELINKS") != NULL ) fos |= FOS_NODEREFERENCELINKS;
    if ( StrStrI(keywords, "NOREADONLYRETURN"  ) != NULL ) fos |= FOS_NOREADONLYRETURN  ;
    if ( StrStrI(keywords, "NOTESTFILECREATE"  ) != NULL ) fos |= FOS_NOTESTFILECREATE  ;
    if ( StrStrI(keywords, "NOVALIDATE"        ) != NULL ) fos |= FOS_NOVALIDATE        ;
    if ( StrStrI(keywords, "OVERWRITEPROMPT"   ) != NULL ) fos |= FOS_OVERWRITEPROMPT   ;
    if ( StrStrI(keywords, "PATHMUSTEXIST"     ) != NULL ) fos |= FOS_PATHMUSTEXIST     ;
    if ( StrStrI(keywords, "PICKFOLDERS"       ) != NULL ) fos |= FOS_PICKFOLDERS       ;
    if ( StrStrI(keywords, "SHAREAWARE"        ) != NULL ) fos |= FOS_SHAREAWARE        ;
    if ( StrStrI(keywords, "STRICTFILETYPES"   ) != NULL ) fos |= FOS_STRICTFILETYPES   ;

    return fos;
}

static HRESULT cidUnadvise(RexxMethodContext *c, pCCommonItemDialog pccid)
{
    HRESULT hr = ERROR_INVALID_FUNCTION;

    if ( pccid != NULL && pccid->pcde != NULL && pccid->cookie != 0 )
    {
        if ( ! pccid->errorUnadviseIsDone )
        {
            hr = pccid->pfd->Unadvise(pccid->cookie);
            pccid->errorUnadviseIsDone = false;
        }
        pccid->pcde->Release();

        c->DropObjectVariable(CID_EVENTHANDLER_VAR);
        pccid->pcde   = NULL;
        pccid->cookie = 0;
#ifdef _DEBUG
        printf("cidUnadvise(), pccid->pcde->Release() invoked\n");
#endif
    }

    return hr;
}


static HRESULT cidReleaseFilter(RexxMethodContext *c, pCCommonItemDialog pccid)
{
    HRESULT hr = ERROR_INVALID_FUNCTION;

    if ( pccid != NULL && pccid->psif != NULL )
    {
        pccid->psif->Release();

        c->DropObjectVariable(CID_FILTER_VAR);
        pccid->psif   = NULL;
#ifdef _DEBUG
        printf("cidReleaseFilter(), pccid->psif->Release() invoked\n");
#endif
    }

    return hr;
}


/**
 * Releases the IFileDialog pointer and releases the COM library if we are on
 * the proper thread.
 *
 * @param pccid
 */
static void cidDone(RexxMethodContext *c, pCCommonItemDialog pccid)
{
    if ( pccid->pfd != NULL )
    {
        cidUnadvise(c, pccid);
        cidReleaseFilter(c, pccid);

        pccid->pfd->Release();
        pccid->pfd  = NULL;

#ifdef _DEBUG
        printf("cidDone(), pccid->pfd-Release() invoked\n");
#endif
    }
    if ( pccid->comInitialized && pccid->comThreadID == GetCurrentThreadId() )
    {
        CoUninitialize();
        pccid->comInitialized = false;
#ifdef _DEBUG
        printf("cidDone(), CoUninitialize() invoked\n");
#endif
    }
}

/**
 * A helper function to get the window handle of the Common Item Dialog.
 *
 * @param c
 * @param pfd
 * @param pHr
 *
 * @return The window handle.
 */
HWND getCIDHwnd(IFileDialog *pfd, HRESULT *pHr)
{
    IOleWindow  *piow  = NULL;
    HWND         hwnd  = NULL;

    HRESULT hr = pfd->QueryInterface(IID_PPV_ARGS(&piow));
    if ( SUCCEEDED(hr) )
    {
        hr = piow->GetWindow(&hwnd);
        piow->Release();
    }
    *pHr = hr;

    return hwnd;
}

/**
 * A helper function to get the window handle of the Common Item Dialog as a
 * Rexx object.
 *
 * @param c
 * @param pfd
 *
 * @return The window handle as a Rexx object
 *
 * @remarks  We ignore any errors here and just return a NULL handle.  Seems to
 *           work every time, though.
 */
RexxObjectPtr getCommonDialogHwnd(RexxThreadContext *c, IFileDialog *pfd)
{
    HRESULT hr;
    HWND    hwnd = getCIDHwnd(pfd, &hr);

    return  pointer2string(c, hwnd);
}

HRESULT cidGetDlgHwnd(RexxMethodContext *c, pCCommonItemDialog pccid)
{
    HRESULT hr;
    pccid->hwndDlg = getCIDHwnd(pccid->pfd, &hr);

    return hr;
}

/**
 * Common code for:
 *
 * CommonItemDialog::setFolder() and CommonItemDialog::setDefaultFolder()
 *
 * @param c
 * @param pCSelf
 * @param folder
 * @param isDefaultFolder
 *
 * @return HRESULT
 */
static HRESULT cidSetFolder(RexxMethodContext *c, void *pCSelf, RexxObjectPtr folder, bool isDefaultFolder)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(c, pCSelf, &hr);
    if ( pccid == NULL )
    {
        goto done_out;
    }

    IShellItem *psi = getShellItemFromObject(c, folder, 1, &hr);
    if ( psi != NULL )
    {
        hr = isDefaultFolder ? pccid->pfd->SetDefaultFolder(psi) : pccid->pfd->SetFolder(psi);
        if ( FAILED(hr) )
        {
            oodSetSysErrCode(c->threadContext, hr);
        }
        psi->Release();
    }

done_out:
    return hr;
}

/**
 * Common code to set some type of text item in the Common Item Dialog.
 *
 * @param c
 * @param text
 * @param type
 * @param pCSelf
 *
 * @return HRESULT
 *
 * @remarks  All text items have the same max length, which is the max length
 *           for the file name text.
 */
static HRESULT cidSetText(RexxMethodContext *c, CSTRING text, CidTextType type, void *pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(c, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return hr;
    }

    size_t len = strlen(text);
    if (  len >= MAX_PATH )
    {
        stringTooLongException(c->threadContext, 1, MAX_PATH - 1, len);
        return hr;
    }

    WCHAR wName[MAX_PATH];

    if ( putUnicodeText((LPWORD)wName, text, &hr) != 0 )
    {
        switch ( type )
        {
            case CidDefaultExtension :
                hr = pccid->pfd->SetDefaultExtension(wName);
                break;

            case CidFileName :
                hr = pccid->pfd->SetFileName(wName);
                break;

            case CidFileNameLabel :
                hr = pccid->pfd->SetFileNameLabel(wName);
                break;

            case CidCancelButtonLabel :
            {
                if ( pccid->hwndDlg == NULL )
                {
                    hr = cidGetDlgHwnd(c, pccid);
                    if ( ! SUCCEEDED(hr) )
                    {
                        break;
                    }
                }
                if ( SetDlgItemTextW(pccid->hwndDlg, IDCANCEL, wName) == 0 )
                {
                    hr = GetLastError();
                }
                break;
            }

            case CidOkButtonLabel :
                hr = pccid->pfd->SetOkButtonLabel(wName);
                break;

            case CidTitle :
                hr = pccid->pfd->SetTitle(wName);
                break;
        }
    }

    oodSetSysErrCode(c->threadContext, hr);
    return hr;
}


/** CommonItemDialog::options                  [attribute]
 */
RexxMethod1(RexxObjectPtr, cid_options, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid != NULL )
    {
        FILEOPENDIALOGOPTIONS fos;

        hr = pccid->pfd->GetOptions(&fos);
        if ( SUCCEEDED(hr) )
        {
            return fos2keywords(context, fos);
        }
        else
        {
            oodSetSysErrCode(context->threadContext, hr);
        }
    }
    return context->NullString();
}
RexxMethod2(RexxObjectPtr, cid_setOptions, CSTRING, opts, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid != NULL )
    {
        uint32_t fos = keywords2fos(context, opts, 1);
        hr = pccid->pfd->SetOptions(fos);
        if ( FAILED(hr) )
        {
            oodSetSysErrCode(context->threadContext, hr);
        }
    }
    return NULLOBJECT;
}

/** CommonItemDialog::uninit()
 *
 * Does clean up for this CommonItemDialog.  May not be needed
 */
RexxMethod1(RexxObjectPtr, cid_uninit, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = getCidCSelf(context, pCSelf, &hr);
    if ( pccid != NULL )
    {
        cidDone(context, pccid);
    }
    return NULLOBJECT;
}

/** CommonItemDialog::init()
 *
 * Initializes
 *
 *
 */
RexxMethod1(RexxObjectPtr, cid_init, RexxObjectPtr, cselfBuf)
{
    if ( ! context->IsBuffer(cselfBuf) )
    {
        baseClassInitializationException(context, "CommonItemDialog");
        return TheNilObj;
    }

    context->SetObjectVariable("CSELF", cselfBuf);

    return TheZeroObj;
}


/** CommonItemDialog::addPlace()
 *
 *
 *
 *  @param folder  [required] The folder ...  This can be specified as a full
 *                 path, a CSIDL_XX name, or an item ID list.
 *
 *  @param where   [optional] Specifies where the folder is placed within the
 *                 list. The only allowable values are TOP or BOTTOM. If the
 *                 argumen is omitted, the default is TOP.
 *
 *  @param  Returns the system result code.
 *
 *  @notes
 */
RexxMethod3(uint32_t, cid_addPlace, RexxObjectPtr, folder, OPTIONAL_CSTRING, _where, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        goto done_out;
    }

    FDAP where = FDAP_TOP;
    if ( argumentExists(2) )
    {
        if ( StrCmpI(_where, "TOP") == 0 )
        {
            ; // puposefully do nothing
        }
        else if ( StrCmpI(_where, "BOTTOM") == 0 )
        {
            where = FDAP_BOTTOM;
        }
        else
        {
            wrongArgKeywordException(context, 2, "TOP or BOTTOM", _where);
            oodSetSysErrCode(context->threadContext, E_INVALIDARG);
            return E_INVALIDARG;
        }
    }

    IShellItem *psi = getShellItemFromObject(context, folder, 1, &hr);
    if ( psi != NULL )
    {
        hr = pccid->pfd->AddPlace(psi, FDAP_TOP);
        if ( FAILED(hr) )
        {
            oodSetSysErrCode(context->threadContext, hr);
        }
        psi->Release();
    }

done_out:
    return hr;
}


/** CommonItemDialog::advise()
 *
 *
 *
 *  @param RexxObjectPtr  [required] The CommonDialogEvents object that will
 *                        handle event notifications from this common item
 *                        dialog.
 *
 *  @param  Returns the system result code.
 *
 *  @notes
 */
RexxMethod2(uint32_t, cid_advise, RexxObjectPtr, eventHandler, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        goto done_out;
    }
    RexxMethodContext *c = context;
    if ( ! c->IsOfType(eventHandler, "CommonDialogEvents") )
    {
        cidDone(context, pccid);
        wrongClassException(c->threadContext, 1, "CommonDialogEvents", eventHandler);
        hr = ERROR_INVALID_FUNCTION;
        goto done_out;
    }

    if ( pccid->pcde != NULL )
    {
        cidDone(context, pccid);
        userDefinedMsgException(context, 1, "an event handler is already assigned");
        hr = ERROR_INVALID_FUNCTION;
        goto done_out;
    }

    pCCommonDialogEvents pccde = (pCCommonDialogEvents)c->ObjectToCSelf(eventHandler);
    if ( pccde->inUse )
    {
        cidDone(context, pccid);
        userDefinedMsgException(context, 1, "the event handler is invalid, it has been used previously and disposed of");
        hr = ERROR_INVALID_FUNCTION;
        goto done_out;
    }

    CommonDialogEvents *pcde = pccde->pcde;
    uint32_t            cookie;

    IFileDialogEvents *pfde = NULL;
    hr = pcde->QueryInterface(IID_PPV_ARGS(&pfde));
    if ( SUCCEEDED(hr) )
    {
        hr = pccid->pfd->Advise(pfde, (DWORD *)&cookie);
        if ( SUCCEEDED(hr) )
        {
            pcde->setRexxPFD(pccid->rexxSelf);
            pccde->inUse = true;

            pccid->cookie = cookie;
            pccid->pcde   = pcde;

            context->SetObjectVariable(CID_EVENTHANDLER_VAR, eventHandler);
        }
        pfde->Release();
    }

done_out:
    if ( hr != S_OK )
    {
        oodSetSysErrCode(context->threadContext, hr);
    }
    return hr;
}


/** CommonItemDialog::clearClientData()
 *
 * Instructs the dialog to clear all persisted state information.
 *
 *  @param  Returns the system result code.
 *
 *  @notes  Persisted information can be associated with an application or a
 *          GUID. If a GUID was set by using IFileDialog::SetClientGuid, that
 *          GUID is used to clear persisted information.
 *
 *          The ClearClientData() method always returns:
 *
 *            0x80004005 -> Unspecified error
 *
 *          This is true even in the MSDN samples.  We just pass it on to the
 *          Rexx programmer, but the method seems to work, sometimes ... at
 *          least when a GUID is assigned.
 */
RexxMethod1(uint32_t, cid_clearClientData, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return hr;
    }

    hr = pccid->pfd->ClearClientData();
    if ( FAILED(hr) )
    {
        oodSetSysErrCode(context->threadContext, hr);
    }

    return hr;
}


/** CommonItemDialog::close()
 *
 *  Instructs the dialog to clear all persisted state information.
 *
 *  @returns  Returns the system result code.
 *
 *  @notes
 */
RexxMethod2(uint32_t, cid_close, OPTIONAL_CSTRING, _hr, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return hr;
    }

    HRESULT setHr = S_OK;
    if ( argumentExists(1) && ! rxStr2Number32(context, _hr, (uint32_t *)&setHr, 1) )
    {
        return E_UNEXPECTED;
    }

    return pccid->pfd->Close(setHr);
}


/** CommonItemDialog::getCurrentSelection()
 *
 *
 *
 *
 */
RexxMethod2(RexxObjectPtr, cid_getCurrentSelection, OPTIONAL_CSTRING, _sigdn, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;
    HRESULT       hr;

    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        goto done_out;
    }

    SIGDN sigdn = SIGDN_FILESYSPATH;
    if ( argumentExists(1) )
    {
        if ( ! keyword2sigdn(context, _sigdn, &sigdn, 1) )
        {
            goto done_out;
        }
    }

    IShellItem *psi;

    hr = pccid->pfd->GetCurrentSelection(&psi);
    if ( SUCCEEDED(hr) )
    {
        result = shellItem2name(context->threadContext, psi, sigdn, &hr);
        psi->Release();
    }

done_out:
    oodSetSysErrCode(context->threadContext, hr);
    return result;
}


/** CommonItemDialog::getFileName()
 *
 *  Retrieves the text currently entered in the dialog's File name edit box.
 *
 *  @return  The file entered in the edit box on success, otherwise the .nil
 *           object.
 *
 *  @notes   This can be used to get the file name in the edit box after the
 *           show() method returns.
 *
 *  @notes   Applications that use the common item dialogs can receive event
 *           notifications from the dialog when it is open.  ooDialog does not
 *           support this feature.  The getFileName method is probably only of
 *           use in an event handler.  Future enhancements of ooDialog may
 *           include the ability to recieve envent notifications.
 */
RexxMethod1(RexxObjectPtr, cid_getFileName, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;
    HRESULT       hr;

    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return result;
    }

    PWSTR pszPath;

    hr = pccid->pfd->GetFileName(&pszPath);
    if( SUCCEEDED(hr) )
    {
        result = unicode2StringOrNil(context->threadContext, pszPath);
        CoTaskMemFree(pszPath);
    }

    oodSetSysErrCode(context->threadContext, hr);
    return result;
}


/** CommonItemDialog::getFileTypeIndex()
 *
 * Gets the currently selected file type.
 *
 * @return  Returns the one-based index of the selected file type in the file
 *          type array passed to setFileTypes().
 *
 *  @notes  Can be called either while the dialog is open or after it has
 *          closed.
 */
RexxMethod1(int32_t, cid_getFileTypeIndex, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return -1;
    }

    uint32_t index;
    hr = pccid->pfd->GetFileTypeIndex(&index);
    if ( FAILED(hr) )
    {
        oodSetSysErrCode(context->threadContext, hr);
        return -1;
    }

    return index;
}


/** CommonItemDialog::getFolder()
 *
 *
 *
 *
 */
RexxMethod2(RexxObjectPtr, cid_getFolder, OPTIONAL_CSTRING, _sigdn, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;
    HRESULT       hr;

    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return result;
    }

    SIGDN sigdn = SIGDN_FILESYSPATH;
    if ( argumentExists(1) )
    {
        if ( ! keyword2sigdn(context, _sigdn, &sigdn, 1) )
        {
            hr = ERROR_INVALID_FUNCTION;
            goto done_out;
        }
    }

    IShellItem *psi;

    hr = pccid->pfd->GetFolder(&psi);
    if ( SUCCEEDED(hr) )
    {
        result = shellItem2name(context->threadContext, psi, sigdn, &hr);
        psi->Release();
    }

done_out:
    if ( hr != S_OK )
    {
        oodSetSysErrCode(context->threadContext, hr);
    }
    return result;
}


/** CommonItemDialog::getResult()
 *
 *
 *
 *
 */
RexxMethod2(RexxObjectPtr, cid_getResult, OPTIONAL_CSTRING, _sigdn, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;
    HRESULT       hr;

    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return result;
    }

    SIGDN sigdn = SIGDN_FILESYSPATH;
    if ( argumentExists(1) )
    {
        if ( ! keyword2sigdn(context, _sigdn, &sigdn, 1) )
        {
            hr = ERROR_INVALID_FUNCTION;
            goto done_out;
        }
    }

    IShellItem *psi;

    hr = pccid->pfd->GetResult(&psi);
    if ( SUCCEEDED(hr) )
    {
        result = shellItem2name(context->threadContext, psi, sigdn, &hr);
        psi->Release();
    }

done_out:
    if ( hr != S_OK )
    {
        oodSetSysErrCode(context->threadContext, hr);
    }
    return result;
}


/** CommonItemDialog::initCOM
 *
 *  Invokes CoInitalizeEx() on the current thread, with some forethought.  If
 *  the current thread is the initial thread that this object was instantiated
 *  on, then the invocation is only done if needed.
 *
 *  If we are on the initial thread and comInitialized is false the initialize
 *  is done and comInitialized is set to true.  If comInitialized is true, the
 *  initialize is skipped.
 *
 *  If we are on another thread, the CoInitializeEx is always done, but the
 *  return is checked.  If it is S_FALSE, that indicates the CoInitializeEx has
 *  already been done on this thread and we immediately do an unitialize to keep
 *  things balanced.
 *
 *  If the user makes use of the initCom method, he assumes the responsibility
 *  of keeping CoInitializedEx / CoUnitialize() in balance *and* in invoking the
 *  releaseCOM method when he is done with this object.
 *
 *  @remarks  For now, initializing COM with COINIT_APARTMENTTHREADED.  But,
 *            things seem to work if COINIT_MULTITHREADED is used.  MSDN
 *            examples have one with apartment threaded and one with multi
 *            threaded.  Have to revisit which should be used.
 */
RexxMethod1(RexxObjectPtr, cid_initCOM, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return TheFalseObj;
    }

    if ( pccid->comThreadID == GetCurrentThreadId() && pccid->comInitialized )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        return TheFalseObj;
    }

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    oodSetSysErrCode(context->threadContext, hr);
    if ( hr == S_OK )
    {
        if ( pccid->comThreadID == GetCurrentThreadId() )
        {
            pccid->comInitialized = true;
        }
        return TheTrueObj;
    }
    else if ( hr == S_FALSE )
    {
        CoUninitialize();
    }
    return TheFalseObj;
}


/** CommonItemDialog::isReleased()
 *
 *  Determines if this common item dialog object has been released or not.
 *
 *  @return True if this object has been released, false if it has not.
 *
 *  @notes  The relese() method must be invoked once and only once on this
 *          object to release the system resources used by the common file
 *          dialog.
 *
 *          After release() has been invoked, no other methods on this object,
 *          except the isReleased() method, can be invoked. The isReleased()
 *          method can be used to determine if this object is still valid or
 *          not.
 */
RexxMethod1(RexxObjectPtr, cid_isReleased, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCCommonItemDialog pccid = (pCCommonItemDialog)pCSelf;
    if ( pccid == NULL )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        baseClassInitializationException(context, COMMONITEMDIALOG_CLASS);
        return TheFalseObj;
    }

    return pccid->pfd == NULL ? TheTrueObj : TheFalseObj;
}

/** CommonItemDialog::release()
 *
 *  Releases the operating system resources used by this Rexx object.
 *
 *  @return True if this is the first invocation of release() on this object.
 *
 *  @notes  The relese method must be invoked once and only once on this object
 *          to release the system resources used by the common file dialog.
 *          This also releases the COM library if the library was initialized on
 *          the current thread.
 *
 *          After release() has been invoked, no other methods on this object
 *          can be invoked.
 */
RexxMethod1(RexxObjectPtr, cid_release, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return TheFalseObj;
    }
    cidDone(context, pccid);
    return TheTrueObj;
}

/** CommonItemDialog::releaseCom()
 *
 *  Calls CoUninitialize() on the current thread.
 *
 *  The user is responsible for invoking release(), on the same thread as this
 *  object was instantiated on, *once* when she is done with the
 *  CommonItemDialog object.  release() also releases the COM library.
 *
 *  In addition to that invocation, the user is responsible for invoking
 *  releaseCom() once for each successful invocation of initCom().
 *
 *  @return True if CoUnitialize() was called on this thread, false if this is
 *          the same thread as the original thread CoUnitialize() was called on,
 *          and COM is not intialized at this time.
 *
 *  @remarks  CoUninitialize() has nothing to do with the IFileDialog inteface
 *            pointer.  So, it can, and at times should, be invoked after the
 *            interface pointer has been released. We purposively don't check
 *            that this object has already been released.
 */
RexxMethod1(RexxObjectPtr, cid_releaseCOM, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCCommonItemDialog pccid = (pCCommonItemDialog)pCSelf;
    if ( pccid == NULL )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        baseClassInitializationException(context, COMMONITEMDIALOG_CLASS);
        return TheFalseObj;
    }

    if ( pccid->comThreadID == GetCurrentThreadId() && ! pccid->comInitialized )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        return TheFalseObj;
    }

    CoUninitialize();
    if ( pccid->comThreadID == GetCurrentThreadId() )
    {
        pccid->comInitialized = false;
    }
    return TheTrueObj;
}


/** CommonItemDialog::setClientGuid()
 *
 *
 */
RexxMethod2(uint32_t, cid_setClientGuid, CSTRING, _guid, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        goto done_out;
    }

    UUID uuid;
    hr = UuidFromString((RPC_CSTR)_guid, &uuid);
    if ( hr != RPC_S_OK )
    {
        oodSetSysErrCode(context->threadContext, hr);
        goto done_out;
    }

    hr = pccid->pfd->SetClientGuid(uuid);
    if ( FAILED(hr) )
    {
        oodSetSysErrCode(context->threadContext, hr);
    }

done_out:
    return hr;
}

 /** CommonItemDialog::setDefaultExtension()
 *
 *  Sets the title for the dialog.
 *
 *  @param extension  [required] The defualt extension for the common item
 *                    dialog.  Do not include the period.  'jpg' is correct,
 *                    while '.jpg' is not.
 *
 *  @param  Returns the system result code.
 *
 *  @notes  The extension must be less than 260 characters in length.
 */
RexxMethod2(uint32_t, cid_setDefaultExtension, CSTRING, extension, CSELF, pCSelf)
{
    return cidSetText(context, extension, CidDefaultExtension, pCSelf);
}


/** CommonItemDialog::setDefaultFolder()
 *
 *  Sets the folder used as a default if there is not a recently used folder
 *  value available.
 *
 *  @param folder  [required] The folder that is set as the default folder when
 *                 the dialog opens.  This can be specified as a full path, a
 *                 CSIDL_XX name, or an item ID list.
 *
 *  @param  Returns the system result code.
 */
RexxMethod2(uint32_t, cid_setDefaultFolder, RexxObjectPtr, folder, CSELF, pCSelf)
{
    return cidSetFolder(context, pCSelf, folder, true);
}


/** CommonItemDialog::setFileName()
 *
 *  Sets the file name initially placed in the edit box of the dialog
 *
 *  @param name  [required] The initial file name displayed in the  edit box.
 *
 *  @param  Returns the system result code.
 *
 *  @notes  The length of the name must be less than 260 characters.
 */
RexxMethod2(uint32_t, cid_setFileName, CSTRING, fileName, CSELF, pCSelf)
{
    return cidSetText(context, fileName, CidFileName, pCSelf);
}


/** CommonItemDialog::setFileNameLabel()
 *
 *  Sets the text of the label next to the file name edit box.
 *
 *  @param label  [required] The label for the edit box.
 *
 *  @param  Returns the system result code.
 *
 *  @notes  The length of the label must be less than 260 characters.
 */
RexxMethod2(uint32_t, cid_setFileNameLabel, CSTRING, label, CSELF, pCSelf)
{
    return cidSetText(context, label, CidFileNameLabel, pCSelf);
}


/** CommonItemDialog::setFileTypeIndex()
 *
 *  Sets the file type that appears as selected in the dialog.
 *
 *  @param index  [required] The 1-based index in the file types filter array
 *                that should be the selected file type when the dialog is
 *                shown.
 *
 *  @param  Returns the system result code.
 *
 *  @notes  The OS seems to set the file type to that closet to the index
 *          number.  If index is 0, then file type 1 in the array is selected.
 *          In an array of 4 file types, if index is 566, the file type 4 is
 *          selected.
 */
RexxMethod2(uint32_t, cid_setFileTypeIndex, uint32_t, index, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return hr;
    }

    hr = pccid->pfd->SetFileTypeIndex(index);
    if ( FAILED(hr) )
    {
        oodSetSysErrCode(context->threadContext, hr);
    }

    return hr;
}


/** CommonItemDialog::setFileTypes()
 *
 *  Sets the file types filter.
 *
 *  @param types  [required] An array of types.  Index 1 is friendly name of the
 *                filter, index 2 is the filter pattern. Index 3 is friendly
 *                name, index 4 pattern ...
 *
 *  @param  Returns the system result code.
 *
 *  @notes  The types array must not be sparse and it must contain an even
 *          number of items.
 */
RexxMethod2(uint32_t, cid_setFileTypes, RexxArrayObject, types, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return hr;
    }

    COMDLG_FILTERSPEC *fSpec  = NULL;
    size_t             cFSpec = 0;

    hr = ERROR_INVALID_FUNCTION;
    RexxMethodContext *c = context;

    size_t count = c->ArrayItems(types);
    if ( count % 2 != 0 )
    {
        userDefinedMsgException(c, FILE_FILTER_ARRAY_MUST_BE_EVEN);
        goto done_out;
    }

    count /= 2;
    fSpec = (COMDLG_FILTERSPEC *)LocalAlloc(LPTR, count * sizeof(COMDLG_FILTERSPEC ));
    if ( fSpec == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }
    cFSpec = count;

    size_t j;
    size_t i;

    for ( i = 0; i < count; i++ )
    {
        j = (2 * i) + 1;
        RexxObjectPtr _name = c->ArrayAt(types, j);
        if ( _name == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 1, j);
            goto done_out;
        }

        j++;
        RexxObjectPtr _pattern = c->ArrayAt(types, j);
        if ( _name == NULLOBJECT || _pattern == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 1, j);
            goto done_out;
        }

        fSpec[i].pszName = ansi2unicode(c->ObjectToStringValue(_name));
        fSpec[i].pszSpec = ansi2unicode(c->ObjectToStringValue(_pattern));
        if ( fSpec[i].pszName == NULL || fSpec[i].pszSpec == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }
    }

    hr = pccid->pfd->SetFileTypes((uint32_t)count, fSpec);

done_out:
    if ( fSpec != NULL )
    {
        for ( i = 0; i < cFSpec; i++ )
        {
            safeLocalFree((void *)fSpec[i].pszName);
            safeLocalFree((void *)fSpec[i].pszSpec);
        }
        LocalFree(fSpec);
    }

    oodSetSysErrCode(context->threadContext, hr);
    return hr;
}


/** CommonItemDialog::setFilter()
 *
 *
 *
 *  @param filter  [required] The ShellItemFilter object that will be used to
 *                 filter items from the common item dialog view.
 *
 *  @param  Returns the system result code.
 *
 *  @notes  The filter can only be set one time and can not be removed.  The
 *          filter ojbect can not be reused.
 *
 *          MSDN says both:
 *
 *          To filter by file type, setFileTypes() should be used, because in
 *          folders with a large number of items it may offer better performance
 *          than applying a ShellItemFilter.
 *
 *          Deprecated. SetFilter is no longer available for use as of Windows 7
 */
RexxMethod2(uint32_t, cid_setFilter, RexxObjectPtr, filter, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        goto done_out;
    }
    RexxMethodContext *c = context;
    if ( ! c->IsOfType(filter, "ShellItemFilter") )
    {
        cidDone(context, pccid);
        wrongClassException(c->threadContext, 1, "ShellItemFilter", filter);
        hr = ERROR_INVALID_FUNCTION;
        goto done_out;
    }

    if ( pccid->psif != NULL )
    {
        cidDone(context, pccid);
        userDefinedMsgException(context, 1, "a shell item filter is already assigned");
        hr = ERROR_INVALID_FUNCTION;
        goto done_out;
    }

    pCShellItemFilter pcsif = (pCShellItemFilter)c->ObjectToCSelf(filter);

    if ( pcsif->inUse )
    {
        cidDone(context, pccid);
        userDefinedMsgException(context, 1, "the shell item filter is invalid, it has been used previously and disposed of");
        hr = ERROR_INVALID_FUNCTION;
        goto done_out;
    }


    IShellItemFilter *psif = NULL;
    hr = pcsif->psif->QueryInterface(IID_PPV_ARGS(&psif));
    if ( SUCCEEDED(hr) )
    {
        hr = pccid->pfd->SetFilter(psif);
        if ( SUCCEEDED(hr) )
        {
            pcsif->psif->setRexxPFD(pccid->rexxSelf);
            pcsif->inUse = true;

            pccid->psif = pcsif->psif;

            context->SetObjectVariable(CID_FILTER_VAR, filter);
        }
        psif->Release();
    }

done_out:
    if ( hr != S_OK )
    {
        oodSetSysErrCode(context->threadContext, hr);
    }
    return hr;
}


/** CommonItemDialog::setFolder()
 *
 *  Sets the folder that is selected when the dialog is opened
 *
 *  @param folder  [required] The folder that is set as the selected folder when
 *                 the dialog opens.  This can be specified as a full path, a
 *                 CSIDL_XX name, or an item ID list.
 *
 *  @param  Returns the system result code.
 *
 *  @notes  This folder overrides any "most recently used" folder. If this
 *          method is called while the dialog is displayed, it causes the dialog
 *          to navigate to the specified folder.
 */
RexxMethod2(uint32_t, cid_setFolder, RexxObjectPtr, folder, CSELF, pCSelf)
{
    return cidSetFolder(context, pCSelf, folder, false);
}


 /** CommonItemDialog::setCancelButtonLabel()
 *
 *  Sets the label of the Cancel button in the dialog.
 *
 *  @param label  [required] The text for the Cacel button in the dialog
 *
 *  @param  Returns the system result code.
 *
 *  @notes  The label must be less than 260 characters in length.
 *
 *          This method will fail if invoked before show(), and after show()
 *          returns, it is too late.  Therefore the method must be invoked from
 *          one of the CommonDialogEvents event handling methods.
 *          onFolderChanging() is a good choice if the programmer wants the
 *          label changed before the dialog is shown.
 */
RexxMethod2(uint32_t, cid_setCancelButtonLabel, CSTRING, title, CSELF, pCSelf)
{
    return cidSetText(context, title, CidCancelButtonLabel, pCSelf);
}


 /** CommonItemDialog::setOkButtonLabel()
 *
 *  Sets the label of the Ok button in the dialog.
 *
 *  @param label  [required] The text for the Ok button in the dialog
 *
 *  @param  Returns the system result code.
 *
 *  @notes  The label must be less than 260 characters in length.
 */
RexxMethod2(uint32_t, cid_setOkButtonLabel, CSTRING, title, CSELF, pCSelf)
{
    return cidSetText(context, title, CidOkButtonLabel, pCSelf);
}


 /** CommonItemDialog::setTitle()
 *
 *  Sets the title for the dialog.
 *
 *  @param title  [required] The title for the common item dialog
 *
 *  @param  Returns the system result code.
 *
 *  @notes  The title must be less than 260 characters in length.
 */
RexxMethod2(uint32_t, cid_setTitle, CSTRING, title, CSELF, pCSelf)
{
    return cidSetText(context, title, CidTitle, pCSelf);
}


/** CommonItemDialog::show()
 *
 *  Launches the modal dialog window.
 *
 *  @param  owner  [optional] The dialog object that is the owner of the modal
 *                 dialog window.
 *
 *  @param  Returns the system result code.
 */
RexxMethod2(uint32_t, cid_show, OPTIONAL_RexxObjectPtr, owner, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return hr;
    }

    HWND hOwner = NULL;
    if ( argumentExists(1) )
    {
        pCPlainBaseDialog pcpbd = requiredPlainBaseDlg(context, owner, 1);
        if ( pcpbd != NULL )
        {
            hOwner = pcpbd->hDlg;
        }
        else
        {
            hr = ERROR_WINDOW_NOT_DIALOG;
            goto done_out;
        }
    }

    hr = pccid->pfd->Show(hOwner);

done_out:
    oodSetSysErrCode(context->threadContext, hr);
    return hr;
}


RexxMethod1(uint32_t, cid_unadvise, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return hr;
    }

    return cidUnadvise(context, pccid);
}


/**
 * Methods for the ooDialog .OpenFileDialog class.
 */
#define OPENFILEDIALOG_CLASS  "OpenFileDialog"

/**
 * Common code for OpenFileDialog::init() and SaveFileDialog::init()
 *
 * @param c
 * @param super
 * @param rclsid
 * @param name
 * @param isOpen
 *
 * @return RexxObjectPtr
 *
 * @remarks  For now, initializing COM with COINIT_APARTMENTTHREADED.  But,
 *           things seem to work if COINIT_MULTITHREADED is used.  MSDN examples
 *           have one with apartment threaded and one with multi threaded.  Have
 *           to revisit which should be used.
 */
RexxObjectPtr commonFileDialogInit(RexxMethodContext *c, RexxClassObject super, RexxObjectPtr self,
                                   REFCLSID rclsid, CSTRING name, bool isOpen)
{
    oodResetSysErrCode(c->threadContext);
    RexxObjectPtr result = TheOneObj;

    if ( ! requiredOS(c, Vista_OS, name, "Vista") )
    {
        return result;
    }

    RexxBufferObject bufObj = c->NewBuffer(sizeof(CCommonItemDialog));

    pCCommonItemDialog pccid = (pCCommonItemDialog)c->BufferData(bufObj);
    memset(pccid, 0, sizeof(CCommonItemDialog));

    pccid->rexxSelf = self;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if ( hr == S_OK )
    {
        pccid->comInitialized = true;
        pccid->comThreadID    = GetCurrentThreadId();
    }
    else if ( hr == S_FALSE )
    {
        CoUninitialize();
    }
    else
    {
        oodSetSysErrCode(c->threadContext, hr);
        return result;
    }

    if ( isOpen )
    {
        IFileOpenDialog *pfd;

        hr = CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
        if ( SUCCEEDED(hr) )
        {
            pccid->pfd  = pfd;
        }
    }
    else
    {
        IFileSaveDialog *pfd;

        hr = CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
        if ( SUCCEEDED(hr) )
        {
            pccid->pfd  = pfd;
        }
    }

    if ( SUCCEEDED(hr) )
    {
        RexxArrayObject newArgs = c->NewArray(1);
        c->ArrayPut(newArgs, bufObj, 1);

        result = c->ForwardMessage(NULL, NULL, super, newArgs);
        if ( result == TheZeroObj )
        {
            if ( c->IsOfType(self, "COMMONDIALOGCUSTOMIZATIONS") )
            {
                result = c->SendMessage1(self, "INITCUSTOMIZATIONS", bufObj);
            }
        }
        else
        {
            CoUninitialize();
            oodSetSysErrCode(c->threadContext, OR_INVALID_OID);
            pccid->comInitialized = false;
        }
    }
    else
    {
        oodSetSysErrCode(c->threadContext, hr);
    }

    return result;
}

/** OpenFileDialog::init()
 *
 * Initializes
 *
 *
 */
RexxMethod2(RexxObjectPtr, ofd_init, SUPER, super, OSELF, self)
{
    return commonFileDialogInit(context, super, self, CLSID_FileOpenDialog, "OpenFileDialog", true);
}


/** OpenFileDialog::getResults()
 *
 *  Gets the files selected by the user when multi selection is enabled.
 *
 *  @param  sigdn  [optional]  One of the SIGDN_xxx constants to signal how the
 *                 returned file name is formatted.  By default, the file names
 *                 are returned as the full path names of the files selected.
 *
 *  @return  On success returns an array of the file names.  On error returns
 *           the .nil object.
 *
 *  @notes  This method fails when the open file dialog did not have multi
 *          selection enabled or if the user canceled the dialog.
 */
RexxMethod2(RexxObjectPtr, ofd_getResults, OPTIONAL_CSTRING, _sigdn, CSELF, pCSelf)
{
    RexxObjectPtr    result = TheNilObj;
    IShellItemArray *psia   = NULL;
    IShellItem      *psi    = NULL;
    HRESULT          hr;

    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        goto done_out;
    }

    SIGDN sigdn = SIGDN_FILESYSPATH;
    if ( argumentExists(1) )
    {
        if ( ! keyword2sigdn(context, _sigdn, &sigdn, 1) )
        {
            goto done_out;
        }
    }

    IFileOpenDialog *pfod = NULL;
    hr = pccid->pfd->QueryInterface(&pfod);
    if ( FAILED(hr) )
    {
        goto done_out;
    }

    hr = pfod->GetResults(&psia);
    if ( psia != NULL )
    {
        uint32_t cItems;

        hr = psia->GetCount((DWORD *)&cItems);
        if ( FAILED(hr) )
        {
            goto done_out;
        }

        RexxArrayObject files = context->NewArray(cItems);
        for ( uint32_t i = 0; i < cItems; i++ )
        {
            hr = psia->GetItemAt(i, &psi);
            if ( FAILED(hr) )
            {
                goto done_out;
            }

            // rxItem could be .nil if unicode2StringOrNil() fails.  We just
            // ignore that and put the object in the array.
            RexxObjectPtr rxItem = shellItem2name(context->threadContext, psi, sigdn, &hr);
            context->ArrayPut(files, rxItem, i + 1);

            psi->Release();
            psi = NULL;
        }

        psia->Release();
        psia = NULL;

        result = files;
    }

done_out:
    if ( pfod != NULL )
    {
        pfod->Release();
    }
    if ( psi != NULL )
    {
        psi->Release();
    }
    if ( psia != NULL )
    {
        psia->Release();
    }

    oodSetSysErrCode(context->threadContext, hr);
    return result;
}


/**
 * Methods for the ooDialog .SaveFileDialog class.
 */
#define SAVEFILEDIALOG_CLASS  "SaveFileDialog"



/** SaveFileDialog::init()
 *
 * Instantiates a new SaveFileDialog object.
 *
 *
 */
RexxMethod2(RexxObjectPtr, sfd_init, SUPER, super, OSELF, self)
{
    return commonFileDialogInit(context, super, self, CLSID_FileSaveDialog, "SaveFileDialog", false);
}


/** SaveFileDialog::setSaveAsItem()
 *
 *  Sets the item to be used as the initial entry in the Save File dialog.
 *
 *  @param filePath  [required]  The initial file name.  This must be the
 *                   complete path name of an existing file.
 *
 *  @returns  The operating system result code.  This is 0 on success.
 *
 *  @notes  The text of the file name edit box is set to the file name part of
 *          pathName, and the containing folder is opened in the view of the
 *          dialog.
 *
 *          setSaveAsItem would generally be used when the application is saving
 *          a file that already exists.  This method fails if the full path name
 *          is not used, or if the file does not exist.
 *
 *          For files that do not already exist, setFileName() and setFolder()
 *          can be used to place a file name in the edit box and open the view
 *          to a specific folder.
 */
RexxMethod2(uint32_t, sfd_setSaveAsItem, CSTRING, filePath, CSELF, pCSelf)
{
    HRESULT hr;

    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return hr;
    }

    IShellItem *psi = getShellItemFromPath(context, filePath, 1, &hr);
    if ( psi != NULL )
    {
        IFileSaveDialog *pfsd;

        hr = pccid->pfd->QueryInterface(&pfsd);
        if ( SUCCEEDED(hr) )
        {
            hr = pfsd->SetSaveAsItem(psi);
            psi->Release();
        }
    }

    if ( hr != S_OK )
    {
        oodSetSysErrCode(context->threadContext, hr);
    }
    return hr;
}


/**
 * Methods for the ooDialog .CommonDialogCustomizations mixinclass.
 *
 * Users add this mixin class to the CommonItemDialog class by inheriting it
 * before they instantiated a new common file dialog:
 *
 *   .CommonItemDialog~inherit(.CommonDialogCustomizations)
 *   fod = .OpenFileDialog~new
 *
 * Having the concrete subclass they are going to use also works:
 *
 *   .OpenFileDialog~inherit(.CommonDialogCustomizations)
 *   fod = .OpenFileDialog~new
 *
 * Intenally we check for the mixin class during the init() method of the
 * concrete subclass and set things up properly.  Note that we are using the
 * cid_ prefix for methods of the mixin class because things are pretty tied
 * together.
 */
#define COMMONDIALOGCUSTOMIZATIONS_MIXINCLASS  "CommonDialogCustomizations"


static bool cdcGetIdCheckText(RexxMethodContext *c, RexxObjectPtr rxID, CSTRING text, RexxObjectPtr rxID2, uint32_t *id,
                              uint32_t *id2, HRESULT *pHR)
{
    uint32_t ctrlID = oodGlobalID(c, rxID, 1, true);
    if ( ctrlID == OOD_ID_EXCEPTION )
    {
        goto err_out;
    }

    if ( text != NULL )
    {
        size_t len = strlen(text);
        if (  len >= MAX_PATH )
        {
            stringTooLongException(c->threadContext, 2, MAX_PATH - 1, len);
            goto err_out;
        }
    }

    if ( rxID2 != NULL )
    {
        uint32_t ctrlID2 = oodGlobalID(c, rxID2, 3, true);
        if ( ctrlID2 == OOD_ID_EXCEPTION )
        {
            goto err_out;
        }
        *id2 = ctrlID2;
    }

    *pHR = S_OK;
    *id  = ctrlID;
    return true;

err_out:
    *pHR = ERROR_INVALID_FUNCTION;
    oodSetSysErrCode(c->threadContext, ERROR_INVALID_FUNCTION);
    return false;
}


static bool cdcKeyword2state(RexxMethodContext *c, CSTRING keyword, CDCONTROLSTATEF *state, size_t argPos)
{
    *state = CDCS_INACTIVE;

    if (      StrCmpI(keyword, "INACTIVE")       == 0 ) *state = CDCS_INACTIVE;
    else if ( StrCmpI(keyword, "ENABLED")        == 0 ) *state = CDCS_ENABLED;
    else if ( StrCmpI(keyword, "VISIBLE")        == 0 ) *state = CDCS_VISIBLE;
    else if ( StrCmpI(keyword, "ENABLEDVISIBLE") == 0 ) *state = CDCS_ENABLEDVISIBLE;
    else
    {
        wrongArgValueException(c->threadContext, argPos, CDC_STATE_KEYWORDS, keyword);
        return false;
    }
    return true;
}

static RexxObjectPtr cdcState2keyword(RexxMethodContext *c, CDCONTROLSTATEF state)
{
    CSTRING str = "";
    switch ( state )
    {
        case 0 :
            str = "Inactive";
            break;
        case 1 :
            str = "Enabled";
            break;
        case 2 :
            str = "Visible";
            break;
        case 3 :
            str = "EnabledVisible";
            break;
    }
    return c->String(str);
}

static uint32_t cdcControlFunc(RexxMethodContext *c, RexxObjectPtr rxID1, CSTRING text, RexxObjectPtr rxID2,
                               size_t misc, void *pCSelf, CdcControlType ctl)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(c, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return hr;
    }

    uint32_t ctrlID1;
    uint32_t ctrlID2;
    if ( ! cdcGetIdCheckText(c, rxID1, text, rxID2, &ctrlID1, &ctrlID2, &hr) )
    {
        return hr;
    }

    WCHAR wName[MAX_PATH];
    if ( text != NULL )
    {
        if ( putUnicodeText((LPWORD)wName, text, &hr) == 0 )
        {
            oodSetSysErrCode(c->threadContext, hr);
            return hr;
        }
    }

    IFileDialogCustomize *pfdc;

    hr = pccid->pfd->QueryInterface(IID_PPV_ARGS(&pfdc));
    if ( SUCCEEDED(hr) )
    {
        switch ( ctl )
        {
            case CdcCheckButton :
                hr = pfdc->AddCheckButton(ctrlID1, wName, (BOOL)misc);
                break;

            case CdcComboBox :
                hr = pfdc->AddComboBox(ctrlID1);
                break;

            case CdcControlItem :
                hr = pfdc->AddControlItem(ctrlID1, ctrlID2, wName);
                break;

            case CdcEditBox :
                hr = pfdc->AddEditBox(ctrlID1, wName);
                break;

            case CdcEnableOpenDropDown :
                hr = pfdc->EnableOpenDropDown(ctrlID1);
                break;

            case CdcMakeProminent :
                hr = pfdc->MakeProminent(ctrlID1);
                break;

            case CdcMenu :
                hr = pfdc->AddMenu(ctrlID1, wName);
                break;

            case CdcPushButton :
                hr = pfdc->AddPushButton(ctrlID1, wName);
                break;

            case CdcRadioButtonList :
                hr = pfdc->AddRadioButtonList(ctrlID1);
                break;

            case CdcRemoveAll :
                hr = pfdc->RemoveAllControlItems(ctrlID1);
                break;

            case CdcRemoveItem :
                hr = pfdc->RemoveControlItem(ctrlID1, ctrlID2);
                break;

            case CdcSeparator :
                hr = pfdc->AddSeparator(ctrlID1);
                break;

            case CdcSetCheckButton :
                hr = pfdc->SetCheckButtonState(ctrlID1, (BOOL)misc);
                break;

            case CdcSetControlItemState :
                hr = pfdc->SetControlItemState(ctrlID1, ctrlID2, (CDCONTROLSTATEF)misc);
                break;

            case CdcSetControlItemText :
                hr = pfdc->SetControlItemText(ctrlID1, ctrlID2, wName);
                break;

            case CdcSetControlLabel :
                hr = pfdc->SetControlLabel(ctrlID1, wName);
                break;

            case CdcSetControlState :
                hr = pfdc->SetControlState(ctrlID1, (CDCONTROLSTATEF)misc);
                break;

            case CdcSetEditBoxText :
                hr = pfdc->SetEditBoxText(ctrlID1, wName);
                break;

            case CdcSetSelectedControlItem :
                hr = pfdc->SetSelectedControlItem(ctrlID1, ctrlID2);
                break;

            case CdcStartVisualGroup :
                hr = pfdc->StartVisualGroup(ctrlID1, wName);
                break;

            case CdcText :
                hr = pfdc->AddText(ctrlID1, wName);
                break;
        }
        pfdc->Release();
    }

    if ( hr != S_OK )
    {
        oodSetSysErrCode(c->threadContext, hr);
    }
    return hr;
}

static uint32_t cdcControlStateFunc(RexxMethodContext *c, RexxObjectPtr rxID1, CSTRING text, RexxObjectPtr rxID2,
                                    size_t argPos, void *pCSelf, CdcControlType ctl)
{
    CDCONTROLSTATEF state;
    if ( ! cdcKeyword2state(c, text, &state, ctl == CdcSetControlItemState ? 3 : 2) )
    {
        oodSetSysErrCode(c->threadContext, ERROR_INVALID_FUNCTION);
        return ERROR_INVALID_FUNCTION;
    }

    return cdcControlFunc(c, rxID1, NULL, rxID2, state, pCSelf, ctl);
}

static RexxObjectPtr cdcGetControlState(RexxMethodContext *c, RexxObjectPtr rxID1, RexxObjectPtr rxID2,
                                        void *pCSelf, CdcControlType ctl)
{
    RexxObjectPtr result = TheNilObj;
    HRESULT       hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(c, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return result;
    }

    uint32_t ctrlID1;
    uint32_t ctrlID2;
    if ( ! cdcGetIdCheckText(c, rxID1, NULL,  rxID2, &ctrlID1, &ctrlID2, &hr) )
    {
        return result;
    }

    IFileDialogCustomize *pfdc;

    hr = pccid->pfd->QueryInterface(IID_PPV_ARGS(&pfdc));
    if ( SUCCEEDED(hr) )
    {
        switch ( ctl )
        {
            case CdcCheckButtonState :
                BOOL chkd;

                hr = pfdc->GetCheckButtonState(ctrlID1, &chkd);
                if ( SUCCEEDED(hr) )
                {
                    result = c->Logical(chkd);
                }
                break;

            case CdcControlItemState :
            case CdcControlState :
                CDCONTROLSTATEF state;

                if ( ctl == CdcControlItemState )
                {
                    hr = pfdc->GetControlItemState(ctrlID1, ctrlID2, &state);
                }
                else
                {
                    hr = pfdc->GetControlState(ctrlID1, &state);
                }
                if ( SUCCEEDED(hr) )
                {
                    result = cdcState2keyword(c, state);
                }
                break;

            case CdcEditBoxState :
                WCHAR *text;

                hr = pfdc->GetEditBoxText(ctrlID1, &text);
                if ( SUCCEEDED(hr) )
                {
                    result = unicode2StringOrNil(c->threadContext, text);
                    CoTaskMemFree(text);
                }
                break;

            case CdcSelectedControlItem :
                DWORD id;

                hr = pfdc->GetSelectedControlItem(ctrlID1, &id);
                if ( SUCCEEDED(hr) )
                {
                    result = c->UnsignedInt32(id);
                }
                break;
        }

        pfdc->Release();
    }

    if ( hr != S_OK )
    {
        oodSetSysErrCode(c->threadContext, hr);
    }
    return result;
}


/** CommonDialogCustomizations::initCustomizations()
 *
 * Called internally to set the CSelf point for this class.
 */
RexxMethod1(RexxObjectPtr, cid_initCustomizations, RexxObjectPtr, cselfBuf)
{
    if ( ! context->IsBuffer(cselfBuf) )
    {
        baseClassInitializationException(context, "CommonDialogCustomizations");
        return TheNilObj;
    }

    context->SetObjectVariable("CSELF", cselfBuf);

    return TheZeroObj;
}

RexxMethod4(uint32_t, cid_addCheckButton, RexxObjectPtr, rxID, CSTRING, label, OPTIONAL_logical_t, checked, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, label, NULLOBJECT, checked, pCSelf, CdcCheckButton);
}

RexxMethod2(uint32_t, cid_addComboBox, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, NULL, NULLOBJECT, 0, pCSelf, CdcComboBox);
}


/** CommonDialogCustomizations::addControlItem
 *
 *  Adds a control to the specified control container
 *
 *  @param  containerID    [required] The control container to add the item to.
 *  @param  label          [required] The text for the item being added.
 *  @param  itemID         [required] The item to add.
 *
 *  return System result code, S_OK on success, otherwise the error code.
 *
 */
RexxMethod4(uint32_t, cid_addControlItem, RexxObjectPtr, rxContainerID, CSTRING, label, RexxObjectPtr, rxItemID, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxContainerID, label, rxItemID, 0, pCSelf, CdcControlItem);
}


/** CommonDialogCustomizations::addEditBox
 *
 *  @remarks  The MSDN doc says that the second arg is a label for the edit
 *            control, but it is actually text that gets set for the edit box.
 *            To actually add a label, you have to use startVisualGroup,
 *            addEditBox, endVisual group.
 *
 *            If we send a null to the COM method for text, we get a garbled
 *            string in the edit box.  Probably just lucky we don't crash, so we
 *            use an empty string.
 */
RexxMethod3(uint32_t, cid_addEditBox, RexxObjectPtr, rxID, OPTIONAL_CSTRING, text, CSELF, pCSelf)
{
    if ( argumentOmitted(2) )
    {
        text = "";
    }
    return cdcControlFunc(context, rxID, text, NULLOBJECT, 0, pCSelf, CdcEditBox);
}


RexxMethod3(uint32_t, cid_addMenu, RexxObjectPtr, rxID, CSTRING, text, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, text, NULLOBJECT, 0, pCSelf, CdcMenu);
}


RexxMethod3(uint32_t, cid_addPushButton, RexxObjectPtr, rxID, CSTRING, label, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, label, NULLOBJECT, 0, pCSelf, CdcPushButton);
}


RexxMethod2(uint32_t, cid_addRadioButtonList, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, NULL, NULLOBJECT, 0, pCSelf, CdcRadioButtonList);
}


RexxMethod2(uint32_t, cid_addSeparator, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, NULL, NULLOBJECT, 0, pCSelf, CdcSeparator);
}


RexxMethod3(uint32_t, cid_addText, RexxObjectPtr, rxID, CSTRING, text, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, text, NULLOBJECT, 0, pCSelf, CdcText);
}


RexxMethod2(uint32_t, cid_enableOpenDropDown, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, NULL, NULLOBJECT, 0, pCSelf, CdcEnableOpenDropDown);
}


/** CommonDialogCustomizations::endVisualGroup()
 *
 */
RexxMethod1(uint32_t, cid_endVisualGroup, CSELF, pCSelf)
{
    HRESULT hr;
    pCCommonItemDialog pccid = (pCCommonItemDialog)getCidCSelf(context, pCSelf, &hr);
    if ( pccid == NULL )
    {
        return hr;
    }

    IFileDialogCustomize *pfdc;

    hr = pccid->pfd->QueryInterface(IID_PPV_ARGS(&pfdc));
    if ( SUCCEEDED(hr) )
    {
        hr = pfdc->EndVisualGroup();
        pfdc->Release();
    }

    if ( hr != S_OK )
    {
        oodSetSysErrCode(context->threadContext, hr);
    }
    return hr;
}


/**  CommonDialogCustomizations::getCheckButtonState()
 *
 *   Gets the checked state of the specified check button.
 *
 *   @param id  [required]  The ID for the check button.
 *
 *   @return True if checked, otherwise false.
 *
 *   @notes  Also aliased to the isChecked() method
 */
RexxMethod2(RexxObjectPtr, cid_getCheckButtonState, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    return cdcGetControlState(context, rxID, NULLOBJECT, pCSelf, CdcCheckButtonState);
}


/**  CommonDialogCustomizations::getControlItemState()
 *
 *
 */
RexxMethod3(RexxObjectPtr, cid_getControlItemState, RexxObjectPtr, rxContainerID, RexxObjectPtr, rxItemID, CSELF, pCSelf)
{
    return cdcGetControlState(context, rxContainerID, rxItemID, pCSelf, CdcControlItemState);
}


/**  CommonDialogCustomizations::getControlState()
 *
 *
 */
RexxMethod2(RexxObjectPtr, cid_getControlState, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    return cdcGetControlState(context, rxID, NULLOBJECT, pCSelf, CdcControlState);
}


/**  CommonDialogCustomizations::getEditBoxText()
 *
 *
 */
RexxMethod2(RexxObjectPtr, cid_getEditBoxText, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    return cdcGetControlState(context, rxID, NULLOBJECT, pCSelf, CdcEditBoxState);
}


/**  CommonDialogCustomizations::getSelectedControlItem()
 *
 *   @param  rxID  [required] The *container* control id.
 *
 *   @return  The selected control item within the specified container control.
 */
RexxMethod2(RexxObjectPtr, cid_getSelectedControlItem, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    return cdcGetControlState(context, rxID, NULLOBJECT, pCSelf, CdcSelectedControlItem);
}


/**  CommonDialogCustomizations::makeProminent()
 *
 *
 */
RexxMethod2(uint32_t, cid_makeProminent, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, NULL, NULLOBJECT, 0, pCSelf, CdcMakeProminent);
}


/**  CommonDialogCustomizations::removeAllControlItems()
 *
 *
 *   @notes  IFileDialogCustomize::removeAllControlItems() returns (0x80004001):
 *           Not implemented.  Search on Google reveals this method is not
 *           implemented.
 */
RexxMethod2(uint32_t, cid_removeAllControlItems, RexxObjectPtr, rxContainerID, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxContainerID, NULL, NULLOBJECT, 0, pCSelf, CdcRemoveAll);
}


/**  CommonDialogCustomizations::removeControlItem()
 *
 *
 *   @notes  IFileDialogCustomize::removeControlItem() returns (0x80004001): Not
 *           implemented, for items in a radio button list.  But, I do not see
 *           any confirmation of that anywhere.
 *
 *           However, it does work for items in other types of containers.
 *           Menus, combo boxes, and the drop down list in the Open / Save
 *           button.
 */
RexxMethod3(uint32_t, cid_removeControlItem, RexxObjectPtr, rxContainerID, RexxObjectPtr, rxItemID, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxContainerID, NULL, rxItemID, 0, pCSelf, CdcRemoveItem);
}


/**  CommonDialogCustomizations::setCheckBoxState()
 *
 *   Also implements CommonDialogCustomizations::check() and
 *   CommonDialogCustomizations::uncheck()
 *
 */
RexxMethod4(uint32_t, cid_setCheckButtonState, RexxObjectPtr, rxID, OPTIONAL_logical_t, checked,
            NAME, methodName, CSELF, pCSelf)
{
    if ( StrCmpI(methodName, "CHECK") == 0 )
    {
        checked = TRUE;
    }
    else if ( StrCmpI(methodName, "UNCHECK") == 0 )
    {
        checked = FALSE;
    }
    else if ( argumentOmitted(2) )
    {
        missingArgException(context->threadContext, 2);
        return 0;
    }
    return cdcControlFunc(context, rxID, NULL, NULLOBJECT, checked, pCSelf, CdcSetCheckButton);
}


/**  CommonDialogCustomizations::setControlItemState()
 *
 *
 */
RexxMethod4(uint32_t, cid_setControlItemState, RexxObjectPtr, rxContainerID, RexxObjectPtr, rxItemID,
            CSTRING, _state, CSELF, pCSelf)
{
    return cdcControlStateFunc(context, rxContainerID, _state, rxItemID, 3, pCSelf, CdcSetControlItemState);
}


/**  CommonDialogCustomizations::setControlItemText()
 *
 *
 */
RexxMethod4(uint32_t, cid_setControlItemText, RexxObjectPtr, rxContainerID, RexxObjectPtr, rxItemID, CSTRING, text, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxContainerID, text, rxItemID, 0, pCSelf, CdcSetControlItemText);
}


/**  CommonDialogCustomizations::setControlLabel()
 *
 *
 */
RexxMethod3(uint32_t, cid_setControlLabel, RexxObjectPtr, rxID, CSTRING, text, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, text, NULLOBJECT, 0, pCSelf, CdcSetControlLabel);
}


/**  CommonDialogCustomizations::setControlState()
 *
 *
 */
RexxMethod3(uint32_t, cid_setControlState, RexxObjectPtr, rxID, CSTRING, _state, CSELF, pCSelf)
{
    return cdcControlStateFunc(context, rxID, _state, NULLOBJECT, 2, pCSelf, CdcSetControlState);
}


/**  CommonDialogCustomizations::setEditBoxText()
 *
 *
 */
RexxMethod3(uint32_t, cid_setEditBoxText, RexxObjectPtr, rxID, CSTRING, text, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, text, NULLOBJECT, 0, pCSelf, CdcSetEditBoxText);
}


/** CommonDialogCustomizations::setSelectedControlItem
 *
 *  Sets the selected state of the specified item in an option button group or a
 *  combo box found in the dialog.
 *
 *  @param  containerID    [required] The control container of the item.
 *  @param  itemID         [required] The item to set to selected.
 *
 *  return System result code, S_OK on success, otherwise the error code.
 *
 */
RexxMethod3(uint32_t, cid_setSelectedControlItem, RexxObjectPtr, rxContainerID, RexxObjectPtr, rxItemID, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxContainerID, NULL, rxItemID, 0, pCSelf, CdcSetSelectedControlItem);
}


/** CommonDialogCustomizations::startVisualGroup()
 *
 *
 */
RexxMethod3(uint32_t, cid_startVisualGroup, RexxObjectPtr, rxID, CSTRING, label, CSELF, pCSelf)
{
    return cdcControlFunc(context, rxID, label, NULLOBJECT, 0, pCSelf, CdcStartVisualGroup);
}


/**
 * Methods for the ooDialog .CommonDialogEvents class.
 */
#define COMMONDIALOGEVENTS_CLASS  "CommonDialogEvents"



/** CommonDialogEvents::init()
 *
 * Initializes
 *
 *
 */
RexxMethod1(RexxObjectPtr, cde_init, OSELF, self)
{
    if ( ! requiredOS(context, Vista_OS, "CommonDialogEvents", "Vista") )
    {
        return TheOneObj;
    }

    CommonDialogEvents *pcde = new (std::nothrow) CommonDialogEvents(self, context->threadContext->instance);

    RexxBufferObject bufObj = context->NewBuffer(sizeof(CCommonDialogEvents));

    pCCommonDialogEvents pccde = (pCCommonDialogEvents)context->BufferData(bufObj);
    memset(pccde, 0, sizeof(CCommonDialogEvents));

    pccde->rexxSelf = self;
    pccde->pcde     = pcde;

    context->SetObjectVariable("CSELF", bufObj);

    return TheZeroObj;
}


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - -*\
 COM class for the Rexx CommonDialogEvents class
\*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - -*/


/**
 * A private method to end things if one of the Rexx methods has a condition
 * pending, or if the user does not return a value.
 *
 * We do this by unadvising ourself from the Common Item File dialog and setting
 * a flag that tells the ooDialog framework that Unadvise() has been already
 * called.  Then we close the file dialog with a cancel.
 *
 * @param c
 *
 * @return HRESULT
 */
HRESULT CommonDialogEvents::abortCommonDialog(RexxThreadContext *c)
{
    HRESULT hr;

    pCCommonItemDialog pccid = (pCCommonItemDialog)c->ObjectToCSelf(rexxPFD);

    hr = pccid->pfd->Unadvise(pccid->cookie);
    pccid->errorUnadviseIsDone = true;

    hr = pccid->pfd->Close(HRESULT_FROM_WIN32(ERROR_CANCELLED));

    return E_NOTIMPL;
}

/**
 * A private method to check the Rexx reply from the event handler.
 *
 * @param c
 * @param reply
 *
 * @remarks  The idea here is to unadvise and close the dialog, if reply is
 *           NULLOBJECT, or if a syntax condition exists.  This works well.
 */
HRESULT CommonDialogEvents::checkEventReply(RexxThreadContext *c, RexxObjectPtr reply, CSTRING methodName)
{
    uint32_t hr = S_OK;

    if ( checkForCondition(c, true) )
    {
        return abortCommonDialog(c);
    }

    if ( reply == NULLOBJECT )
    {
        noMsgReturnException(c, methodName);
        checkForCondition(c, true);
        return abortCommonDialog(c);
    }

    if ( ! c->UnsignedInt32(reply, &hr) )
    {
        hr = E_NOTIMPL;
    }
    return hr;
}


/** CommonDialogEvents::CommonDialogEvents()
 *
 *  The constructor for the class.  We don't do anything at this time.
 *
 * @param rxSelf
 * @param c
 */
CommonDialogEvents::CommonDialogEvents(RexxObjectPtr rxSelf, RexxInstance *c) :
                    cRef(1), rexxSelf(rxSelf), interpreter(c), rexxPFD(TheNilObj)
{
    ;
}


HRESULT CommonDialogEvents::dialogEvent(IFileDialog *pfd, IShellItem *psi, CdeDialogEvent evt)
{
    RexxThreadContext *c;
    RexxObjectPtr      result = TheNilObj;
    HRESULT            hr = S_OK;

    if ( ! interpreter->AttachThread(&c))
    {
        return E_NOTIMPL;
    }

    RexxObjectPtr   rxHwnd = getCommonDialogHwnd(c, pfd);
    RexxArrayObject args   = c->ArrayOfTwo(rexxPFD, rxHwnd);

    CSTRING mthName;
    switch ( evt )
    {
        case FileOk :
            mthName = "onFileOk";
            break;

        case FolderChange :
            mthName = "onFolderChange";
            break;

        case FolderChanging :
            result = shellItem2name(c, psi, SIGDN_FILESYSPATH, &hr);
            c->ArrayPut(args, result, 3);

            mthName = "onFolderChanging";
            break;

        case Help :
            mthName = "onHelp";
            break;

        case SelectionChange :
            mthName = "onSelectionChange";
            break;

        case TypeChange :
            mthName = "onTypeChange";
            break;
    }

    RexxObjectPtr reply = c->SendMessage(rexxSelf, mthName, args);

    hr = checkEventReply(c, reply, mthName);

    c->DetachThread();
    return hr;
}


HRESULT CommonDialogEvents::dialogEventWithResp(IFileDialog *pfd, IShellItem *psi,
                                                uint32_t *resp, CdeDialogEvent evt)
{
    RexxThreadContext *c;
    HRESULT            hr = S_OK;

    if ( ! interpreter->AttachThread(&c))
    {
        return E_NOTIMPL;
    }

    RexxObjectPtr rxHwnd = getCommonDialogHwnd(c, pfd);
    RexxObjectPtr result = shellItem2name(c, psi, SIGDN_FILESYSPATH, &hr);

    if ( SUCCEEDED(hr) )
    {
        RexxArrayObject args = c->ArrayOfThree(rexxPFD, rxHwnd, result);

        CSTRING mthName;
        switch ( evt )
        {
            case Overwrite :
                mthName = "onOverwrite";
                break;

            case ShareViolation :
                mthName = "onShareViolation";
                break;
        }

        RexxObjectPtr reply = c->SendMessage(rexxSelf, mthName, args);

        hr = checkEventReply(c, reply, mthName);
        if ( hr != E_NOTIMPL )
        {
            if ( hr == FDESVR_ACCEPT || hr == FDESVR_DEFAULT || hr == FDESVR_REFUSE )
            {
                *resp = hr;
                hr    = S_OK;
            }
            else
            {
                wrongReplyListException(c, mthName, "one of the FDESVR_xx constants", reply);
                checkForCondition(c, true);
                hr = abortCommonDialog(c);
            }
        }
    }

    c->DetachThread();
    return hr;
}

/** CommonDialogEvents::OnFileOk()
 *
 *  Invoked just before the dialog is about to return with a result.
 *
 * @param pfd
 *
 * @return HRESULT
 *
 * @notes  When this method is called, the getResult() and getResults() methods
 *         can be called.
 *
 *         The application can use this callback method to perform additional
 *         validation before the dialog closes, or to prevent the dialog from
 *         closing. If the application prevents the dialog from closing, it
 *         should display a user interface (UI) to indicate a cause.
 *
 *         An application can also use this method to perform all of its work
 *         surrounding the opening or saving of files.
 */
HRESULT CommonDialogEvents::OnFileOk(IFileDialog *pfd)
{
    return dialogEvent(pfd, NULL, FileOk);
}


/** CommonDialogEvents::OnFolderChange()
 *
 *  Invoked when the user navigates to a new folder.  This method is also
 *  invoked when the dialog first opens
 *
 * @param pfd
 *
 * @return HRESULT
 */
HRESULT CommonDialogEvents::OnFolderChange(IFileDialog *pfd)
{
    return dialogEvent(pfd, NULL, FolderChange);
}


/** CommonDialogEvents::OnFolderChanging()
 *
 *  Invoked before OnFolderChange. This allows the application to stop
 *  navigation to a particular location.
 *
 * @param pfd
 * @param psiFolder
 *
 * @return HRESULT
 *
 * @remarks  The calling application can call IFileDialog::SetFolder during this
 *           callback to redirect navigation to an alternate folder. The actual
 *           navigation does not occur until OnFolderChanging has returned.
 *
 *           If the calling application simply prevents navigation to a
 *           particular folder, a user interface (UI) should be displayed with
 *           an explanation of the restriction.
 */
HRESULT CommonDialogEvents::OnFolderChanging(IFileDialog *pfd, IShellItem *psiFolder)
{
    return dialogEvent(pfd, psiFolder, FolderChanging);
}


/** CommonDialogEvents::OnHelp()
 *
 *  This method was in the MSDN example for common dialog events.  But, it is
 *  not documented and doesn't seem to get called.
 *
 *  Leaving the code in for now.
 *
 * @param pfd
 *
 * @return HRESULT
 */
HRESULT CommonDialogEvents::OnHelp(IFileDialog *pfd)
{
    return dialogEvent(pfd, NULL, Help);
}


/** CommonDialogEvents::OnOverwrite()
 *
 *  Invoked from the save dialog when the user chooses to overwrite a file.
 *
 *
 * @param pfd
 * @param psi
 * @param resp
 *
 * @return HRESULT
 */
HRESULT CommonDialogEvents::OnOverwrite(IFileDialog *pfd, IShellItem *psi, FDE_OVERWRITE_RESPONSE *resp)
{
    return dialogEventWithResp(pfd, psi, (uint32_t *)resp, Overwrite);
}


/** CommonDialogEvents::OnSelectionChange()
 *
 * Invokded when the user changes the selection in the dialog's view.
 *
 *
 * @param pfd
 *
 * @return HRESULT
 */
HRESULT CommonDialogEvents::OnSelectionChange(IFileDialog *pfd)
{
    return dialogEvent(pfd, NULL, SelectionChange);
}


/** CommonDialogEvents::OnShareViolation()
 *
 *  Enables an application to respond to sharing violations that arise from Open
 *  or Save operations.
 *
 * @param pfd
 * @param psi
 * @param resp
 *
 * @return HRESULT
 *
 * @notes  The FOS_SHAREAWARE flag must be set through IFileDialog::SetOptions
 *         before this method is called.
 *
 *         A sharing violation could possibly arise when the application
 *         attempts to open a file, because the file could have been locked
 *         between the time that the dialog tested it and the application opened
 *         it.
 */
HRESULT CommonDialogEvents::OnShareViolation(IFileDialog *pfd, IShellItem *psi, FDE_SHAREVIOLATION_RESPONSE *resp)
{
    return dialogEventWithResp(pfd, psi, (uint32_t *)resp, ShareViolation);
}

/** CommonDialogEvents::OnTypeChange()
 *
 *  Invoked when the dialog is opened to notify the application of the initial
 *  chosen filetype.
 *
 * @param pfd
 *
 * @return HRESULT
 */
HRESULT CommonDialogEvents::OnTypeChange(IFileDialog *pfd)
{
    return dialogEvent(pfd, NULL, TypeChange);
}

/** CommonDialogEvents::dialogControlEvent()
 *
 *  Generic code to Handle all the IFileDialogControlEvents method responses.
 *
 * @param pfdc
 * @param itemID
 * @param ctlID   Can be either the control ID parameter or the checked
 *                parameter dependin on the event
 * @param evt     Identifies which event we are handling.
 *
 * @return HRESULT
 */
HRESULT CommonDialogEvents::dialogControlEvent(IFileDialogCustomize *pfdc, DWORD itemID, DWORD ctlID, CdeDialogEvent evt)
{
    RexxThreadContext *c;
    HRESULT            hr = S_OK;

    if ( ! interpreter->AttachThread(&c))
    {
        return E_NOTIMPL;
    }

    IFileDialog *pfd = NULL;

    hr = pfdc->QueryInterface(IID_PPV_ARGS(&pfd));
    if ( SUCCEEDED(hr) )
    {
        RexxObjectPtr   rxHwnd = getCommonDialogHwnd(c, pfd);
        RexxArrayObject args   = c->ArrayOfThree(rexxPFD, rxHwnd, c->UnsignedInt32(itemID));

        CSTRING mthName;
        switch ( evt )
        {
            case ButtonClicked :
                mthName = "onButtonClicked";
                break;

            case CheckButtonToggled :
                mthName = "onCheckButtonToggled";
                c->ArrayPut(args, ctlID ? TheTrueObj : TheFalseObj, 4);
                break;

            case ControlActivating :
                mthName = "onControlActivating";
                break;

            case ItemSelected :
                mthName = "onItemSelected";
                c->ArrayPut(args, c->UnsignedInt32(ctlID), 4);
                break;
        }

        RexxObjectPtr reply = c->SendMessage(rexxSelf, mthName, args);

        hr = checkEventReply(c, reply, mthName);

        pfd->Release();
    }

    c->DetachThread();
    return hr;
}

/** CommonDialogEvents::OnButtonClicked()
 *
 *  Invoked when the user clicks a command button.
 *
 * @param pfdc
 * @param itemID
 *
 * @return HRESULT
 */
HRESULT CommonDialogEvents::OnButtonClicked(IFileDialogCustomize *pfdc, DWORD itemID)
{
    return dialogControlEvent(pfdc, itemID, 0, ButtonClicked);
}


/** CommonDialogEvents::OnCheckButtonToggled()
 *
 *  Invoked when the user changes the state of a check button.
 *
 * @param pfdc
 * @param itemID
 * @param checked
 *
 * @return HRESULT
 */
HRESULT CommonDialogEvents::OnCheckButtonToggled(IFileDialogCustomize *pfdc, DWORD itemID, BOOL checked)
{
    return dialogControlEvent(pfdc, itemID, checked, CheckButtonToggled);
}


/** CommonDialofEvents::OnControlActivating()
 *
 *  Invoked when an Open button drop-down list customized through
 *  EnableOpenDropDown or a Tools menu is about to display its contents.
 *
 * @param pfdc
 * @param itemID
 *
 * @return HRESULT
 */
HRESULT CommonDialogEvents::OnControlActivating(IFileDialogCustomize *pfdc, DWORD itemID)
{
    return dialogControlEvent(pfdc, itemID, 0, ControlActivating);
}


/** CommonDialogEvents::OnItemSelected()
 *
 * This method is invoked when a dialog control item is selected. (A radio
 * button is selected, etc..)
 *
 * @param pfdc
 * @param dwIDCtl
 * @param dwIDItem
 *
 * @return HRESULT
 */
HRESULT CommonDialogEvents::OnItemSelected(IFileDialogCustomize *pfdc, DWORD ctlID, DWORD itemID)
{
    return dialogControlEvent(pfdc, itemID, ctlID, ItemSelected);
}



/**
 * Methods for the ooDialog .ShellItemFilter class.
 */
#define SHELLITEMFILTER_CLASS  "ShellItemFilter"



/** ShellItemFilter::init()
 *
 * Initializes
 *
 *
 */
RexxMethod1(RexxObjectPtr, sif_init, OSELF, self)
{
    ShellItemFilter *psif = new (std::nothrow) ShellItemFilter(self, context->threadContext->instance);

    RexxBufferObject bufObj = context->NewBuffer(sizeof(CShellItemFilter));

    pCShellItemFilter pcsif = (pCShellItemFilter)context->BufferData(bufObj);
    memset(pcsif, 0, sizeof(CShellItemFilter));

    pcsif->rexxSelf = self;
    pcsif->psif     = psif;

    context->SetObjectVariable("CSELF", bufObj);

    return TheZeroObj;
}


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - -*\
 COM class for the Rexx ShellItemFilter class
\*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - -*/

/** ShellItemFilter::commonDialogHwnd()
 *
 *
 * @param c
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr ShellItemFilter::commonDialogHwnd(RexxThreadContext *c)
{
    RexxObjectPtr rxHwnd = TheZeroObj;
    HRESULT        hr;

    pCCommonItemDialog pccid = (pCCommonItemDialog)c->ObjectToCSelf(rexxPFD);

    IFileDialog *pfd = NULL;
    hr = pccid->pfd->QueryInterface(IID_PPV_ARGS(&pfd));
    if ( SUCCEEDED(hr) )
    {
        rxHwnd = getCommonDialogHwnd(c, pfd);
    }

    return rxHwnd;
}


/**
 * A private method to end things if one of the Rexx methods has a condition
 * pending, or if the user does not return a value.
 *
 * We do this by closing the file dialog with a cancel.
 *
 * @param c
 *
 * @return HRESULT
 */
HRESULT ShellItemFilter::abortCommonDialog(RexxThreadContext *c)
{
    HRESULT hr;

    pCCommonItemDialog pccid = (pCCommonItemDialog)c->ObjectToCSelf(rexxPFD);

    hr = pccid->pfd->Close(HRESULT_FROM_WIN32(ERROR_CANCELLED));

    return E_NOTIMPL;
}

/**
 * A private method to check the Rexx reply from the event handler.
 *
 * @param c
 * @param reply
 *
 * @remarks  The idea here is to close the dialog, if reply is NULLOBJECT, or if
 *           a syntax condition exists.  This works well.
 */
HRESULT ShellItemFilter::checkEventReply(RexxThreadContext *c, RexxObjectPtr reply, CSTRING methodName)
{
    uint32_t hr = S_OK;

    if ( checkForCondition(c, true) )
    {
        return abortCommonDialog(c);
    }

    if ( reply == NULLOBJECT )
    {
        noMsgReturnException(c, methodName);
        checkForCondition(c, true);
        return abortCommonDialog(c);
    }

    if ( ! c->UnsignedInt32(reply, &hr) )
    {
        hr = E_NOTIMPL;
    }
    return hr;
}


/** ShellItemFilter::ShellItemFilter()
 *
 *  The constructor for the class.  We don't do anything at this time.
 *
 * @param rxSelf
 * @param c
 */
ShellItemFilter::ShellItemFilter(RexxObjectPtr rxSelf, RexxInstance *c) :
    cRef(1), rexxSelf(rxSelf), interpreter(c), rexxPFD(TheNilObj)
{
    ;
}


/** ShellItemFilter::IncludeItem()
 *
 *  The host invokes this method for each item in the folder. Return S_OK to
 *  have the item enumerated for inclusion in the view. Return S_FALSE to
 *  prevent the item from being enumerated for inclusion in the view.
 *
 * @param pfd
 * @param psi
 * @param resp
 *
 * @return HRESULT
 */
HRESULT ShellItemFilter::IncludeItem(IShellItem *psi)
{
    RexxThreadContext *c;
    HRESULT            hr = S_OK;

    if ( ! interpreter->AttachThread(&c))
    {
        return E_NOTIMPL;
    }

    RexxObjectPtr rxHwnd = commonDialogHwnd(c);
    RexxObjectPtr result = shellItem2name(c, psi, SIGDN_FILESYSPATH, &hr);

    if ( SUCCEEDED(hr) )
    {
        RexxArrayObject args = c->ArrayOfThree(rexxPFD, rxHwnd, result);

        RexxObjectPtr reply = c->SendMessage(rexxSelf, "includeItem", args);

        hr = checkEventReply(c, reply, "includeItem");

    }

    c->DetachThread();
    return hr;
}


