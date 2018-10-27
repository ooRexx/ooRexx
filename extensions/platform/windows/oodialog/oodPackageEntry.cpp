/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
 * oodPackageEntry.cpp
 *
 * Contains the package entry point, routine and method declarations, and
 * routine and method tables for the native API.  Also contains all global
 * variables and DLLMain().
 */

#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h
#include <shlwapi.h>
#include <stdio.h>
#include <delayimp.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodShared.hpp"


HINSTANCE            MyInstance = NULL;
pCPlainBaseDialog    DialogTable[MAXDIALOGS] = {NULL};
pCPlainBaseDialog    TopDlg = NULL;
size_t               CountDialogs = 0;
CRITICAL_SECTION     crit_sec = {0};
CRITICAL_SECTION     ps_crit_sec = {0};
DWORD                ComCtl32Version = 0;
char                 ComCtl32VersionStr[COMCTL32_VERSION_STRING_LEN + 1] = "";

// Initialized in the DlgUtil class init method (dlgutil_init_cls.)
RexxObjectPtr       TheApplicationObj = NULLOBJECT;
RexxDirectoryObject TheConstDir = NULLOBJECT;

// Initialized in ApplicationManager::init() (app_init)
extern HICON        TheDefaultSmallIcon = NULL;
extern HICON        TheDefaultBigIcon   = NULL;
extern bool         DefaultIconIsShared = true;

// Initialized here, can be changed by ApplicationManager::useGlobalConstDir()
oodConstDir_t       TheConstDirUsage = globalNever;

// Initialized in the PlainBaseDialog class init method (pbdlg_init_cls.)
RexxClassObject     ThePlainBaseDialogClass = NULLOBJECT;

// Initialized in the DynamicDialog class init method (dyndlg_init_cls.)
RexxClassObject     TheDynamicDialogClass = NULLOBJECT;

// Initialized in the DialogControl class init method (dlgctrl_init_cls.)
RexxClassObject     TheDialogControlClass = NULLOBJECT;

// Initialized in the PropertySheetPage class init method (psp_init_cls.)
RexxClassObject     ThePropertySheetPageClass = NULLOBJECT;

// Initialized in the ControlDialog class init method (cd_init_cls.)
RexxClassObject     TheControlDialogClass = NULLOBJECT;

// Initialized in the LvCustomDrawSimple class init method (lvcds_init_cls.)
RexxClassObject     TheLvCustomDrawSimpleClass = NULLOBJECT;

// Initialized in the TvCustomDrawSimple class init method (tvcds_init_cls.)
RexxClassObject     TheTvCustomDrawSimpleClass = NULLOBJECT;

// Initialized in the LvFullRow class init method (lvfr_init_cls.)
RexxClassObject     TheLvFullRowClass = NULLOBJECT;

// Initialized in the LvItem class init method (lvi_init_cls.)
RexxClassObject     TheLvItemClass = NULLOBJECT;

// Initialized in the LvSubItem class init method (lvsi_init_cls.)
RexxClassObject     TheLvSubItemClass = NULLOBJECT;

// Initialized in the TheReBarBandInfo class init method (rbbi_init_cls.)
RexxClassObject     TheReBarBandInfoClass = NULLOBJECT;

// Initialized in the TbButton class init method (tbb_init_cls.)
RexxClassObject     TheTbButtonClass = NULLOBJECT;

/* GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR           gdiplusToken; */


#ifdef __cplusplus
extern "C" {
#endif

BOOL REXXENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if ( fdwReason == DLL_PROCESS_ATTACH )
    {
        MyInstance = hinstDLL;
        InitializeCriticalSection(&crit_sec);
        InitializeCriticalSection(&ps_crit_sec);
    }
    else if ( fdwReason == DLL_PROCESS_DETACH )
    {
        MyInstance = NULL;
        DeleteCriticalSection(&crit_sec);
        DeleteCriticalSection(&ps_crit_sec);
    }
    return(TRUE);
}

#ifdef __cplusplus
}
#endif


/**
 * Do not include ICC_STANDARD_CLASSES.  Some versions of Windows XP have a bug
 * that causes InitCommonControlsEx() to fail when that flag is used.  The flag
 * itself is not needed under any version of Windows.
 *
 * Note: These flags are valid under any supported CommCtrl32 version.  But, as
 * ooDialog adds support for more dialog controls, it may become necessary to
 * check the CommCtrl32 version and have a different set of flags for certain
 * versions.
 */
#define INITCOMMONCONTROLS_CLASS_FLAGS    ICC_WIN95_CLASSES | ICC_DATE_CLASSES | ICC_COOL_CLASSES

/**
 * Determines the version of comctl32.dll and compares it against a minimum
 * required version.
 *
 * @param  context      The ooRexx method context.
 * @param  pDllVersion  The loaded version of comctl32.dll is returned here as a
 *                      packed unsigned long. This number is created using
 *                      Microsoft's suggested process and can be used for
 *                      numeric comparisons.
 * @param  minVersion   The minimum acceptable version.
 * @param  packageName  The name of the package initiating this check.
 * @param  errTitle     The title for the error dialog if it is displayed.
 *
 * @note  If this function fails, an exception is raised.
 */
bool getComCtl32Version(RexxThreadContext *context, DWORD *pDllVersion, DWORD minVersion,
                         CSTRING packageName, CSTRING errTitle)
{
    bool success = false;
    *pDllVersion = 0;

    HINSTANCE hinst = LoadLibrary(TEXT(COMMON_CONTROL_DLL));
    if ( hinst )
    {
        DLLGETVERSIONPROC pDllGetVersion;

        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinst, DLLGETVERSION_FUNCTION);
        if ( pDllGetVersion )
        {
            HRESULT hr;
            DLLVERSIONINFO info;

            ZeroMemory(&info, sizeof(info));
            info.cbSize = sizeof(info);

            hr = (*pDllGetVersion)(&info);
            if ( SUCCEEDED(hr) )
            {
                *pDllVersion = MAKEVERSION(info.dwMajorVersion, info.dwMinorVersion);
                _snprintf(ComCtl32VersionStr, COMCTL32_VERSION_STRING_LEN, "ComCtl32 v%d.%d",
                          info.dwMajorVersion, info.dwMinorVersion);
                success = true;
            }
            else
            {
                systemServiceExceptionComCode(context, COM_API_FAILED_MSG, DLLGETVERSION_FUNCTION, hr);
            }
        }
        else
        {
            systemServiceExceptionCode(context, NO_PROC_MSG, DLLGETVERSION_FUNCTION);
        }
        FreeLibrary(hinst);
    }
    else
    {
        systemServiceExceptionCode(context, NO_HMODULE_MSG, COMMON_CONTROL_DLL);
    }

    if ( *pDllVersion == 0 )
    {
        CHAR msg[256];
        _snprintf(msg, sizeof(msg),
                  "The version of the Windows Common Controls library (%s)\n"
                  "could not be determined.  %s can not continue",
                  COMMON_CONTROL_DLL, packageName);

        internalErrorMsgBox(msg, errTitle);
        success = false;
    }
    else if ( *pDllVersion < minVersion )
    {
        CHAR msg[256];
        _snprintf(msg, sizeof(msg),
                  "%s can not continue with this version of the Windows\n"
                  "Common Controls library(%s.)  The minimum\n"
                  "version required is: %s.\n\n"
                  "This system has: %s\n",
                  packageName, COMMON_CONTROL_DLL, comctl32VersionName(minVersion),
                  comctl32VersionName(*pDllVersion));

        internalErrorMsgBox(msg, errTitle);
        *pDllVersion = 0;
        success = false;
    }
    return success;
}

/**
 * Initializes the common control library for the specified classes.
 *
 * @param classes       Flag specifing the classes to be initialized.
 * @param  packageName  The name of the package initializing the classes.
 * @param  errTitle     The title for the error dialog if it is displayed.
 *
 * @return True on success, otherwise false.
 *
 * @note   An exception has been raised when false is returned.
 */
bool initCommonControls(RexxThreadContext *context, DWORD classes, CSTRING packageName, CSTRING errTitle)
{
    INITCOMMONCONTROLSEX ctrlex;

    ctrlex.dwSize = sizeof(ctrlex);
    ctrlex.dwICC = classes;

    if ( ! InitCommonControlsEx(&ctrlex) )
    {
        systemServiceExceptionCode(context, NO_COMMCTRL_MSG, "Common Control Library");

        CHAR msg[128];
        _snprintf(msg, sizeof(msg),
                  "Initializing the Windows Common Controls\n"
                  "library failed.  %s can not continue.\n\n"
                  "Windows System Error Code: %d\n", packageName, GetLastError());

        internalErrorMsgBox(msg, errTitle);
        return false;
    }
    return true;
}

/**
 * RexxPackageLoader function.
 *
 * The package loader function is called when the library package is first
 * loaded.  This makes it the ideal place for any initialization that must be
 * done prior to ooDialog starting.
 *
 * Note that an exception raised here effectively terminates ooDialog before any
 * user code is executed.
 *
 * This function:
 *
 * 1.) Determines the version of comctl32.dll and initializes the common
 * controls.  The minimum acceptable version of 4.71 is supported on Windows 95
 * with Internet Explorer 4.0, Windows NT 4.0 with Internet Explorer 4.0,
 * Windows 98, and Windows 2000.
 *
 * 2.) Initializes some useful global variables, such as the TheTrueObj,
 * TheFalseObj, etc..
 *
 * 3.) Initializes a null pointer Pointer object and places it in the .local
 * directory. (.NullHandle)  This allows ooRexx code to use a null handle for an
 * argument where appropriate.
 *
 * 4.) Places the SystemErrorCode (.SystemErrorCode) variable in the .local
 * directory.
 *
 * 5.) In a future release it will be used for the GDI+ startup initialization.
 *
 * @param c  Thread context pointer passed from the intepreter when this package
 *           is loaded.
 *
 * @return Nothing is returned
 */
void RexxEntry ooDialogLoad(RexxThreadContext *c)
{
    if ( ! packageLoadHelper(c) )
    {
        return;
    }

    if ( ! getComCtl32Version(c, &ComCtl32Version, COMCTL32_4_71, "ooDialog", COMCTL_ERR_TITLE) )
    {
        return;
    }

    if ( ! initCommonControls(c, INITCOMMONCONTROLS_CLASS_FLAGS, "ooDialog", COMCTL_ERR_TITLE) )
    {
        ComCtl32Version = 0;
        return;
    }

    c->DirectoryPut(TheDotLocalObj, TheZeroPointerObj, "NULLHANDLE");
    c->DirectoryPut(TheDotLocalObj, c->WholeNumberToObject(0), "SYSTEMERRORCODE");

    /* Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);*/
}

/**
 * RexxPackageUnloader function.
 *
 * The package unloader function is called when the library package is unloaded
 * by the interpreter. The unloading process happens when the last interpreter
 * instance is destroyed during the last cleanup stages.
 *
 * At this time, nothing is done, the function is just a place holder.  A
 * furture version of ooDialog will use it to close down GDI+.
 *
 * @param c  Thread context pointer passed from the intepreter when this package
 *           is unloaded.
 *
 * @return Nothing is returned
 */
void RexxEntry ooDialogUnload(RexxThreadContext *c)
{
    /* GdiplusShutdown(gdiplusToken); */
}


/**
 * As ooDialog is enhanced to support more of Windows 7 features, it is linked
 * to Windows library procedures that are not available in the XP systen DLLs.
 * This causes oodialog.dll to fail to load on XP.  In an attempt to extend the
 * life of ooDialog on XP, we are going to use delayed loading of one or more
 * DLLs.
 *
 * This hook function is called by the delayed DLL library loading mechanism
 * when the delayed load fails.  Typically this is because the DLL can not be
 * found.  But, in our case it is more likely that the procedure entrance is not
 * found
 *
 * There is nothing we can do about this, so a message is printed and the
 * application is terminated.
 *
 * @param dliNotify   Reason for the failure.
 * @param pdli        Information about the failure.
 *
 * @return Return a proper HMODULE or valid proc address if this function can
 *         recover from the error, otherwise NULL.  We can not recover from this
 *         so we return NULL.
 */
FARPROC WINAPI delayFailHook(unsigned int dliNotify, PDelayLoadInfo pdli)
{
    switch (dliNotify)
    {
        case dliFailLoadLib :
            printf("Failed to load %s, error code: %d.  Ensure %s is available.\n",
                   pdli->szDll, pdli->dwLastError, pdli->szDll);
            break;

        case dliFailGetProc :
            if (pdli->dlp.fImportByName)
            {
                printf("Failed to get procedure: %s in %s.  Last error: %d\n",
                       pdli->dlp.szProcName, pdli->szDll, pdli->dwLastError);
            }
            else
            {
                printf("Failed to get procedure, ordinal: %s in %s.  Last error: %d\n",
                       pdli->dlp.szProcName, pdli->szDll, pdli->dwLastError);
            }
            break;

        default :
            printf("Unknown error trying to delay load %s.\n", pdli->szDll);
            break;
    }

    printf("The error is unrecoverable, have to abort.\n");

    return NULL;
}

#if 0
PfnDliHook __pfnDliFailureHook2 = delayFailHook;

/**
 * Temp for testing, will be removed.
 *
 * @param dliNotify
 * @param pdli
 *
 * @return FARPROC
 */
FARPROC WINAPI delayNoteHook(unsigned int dliNotify, PDelayLoadInfo pdli)
{
    switch (dliNotify)
    {
        case dliStartProcessing :
            printf("Check to see if library %s is already loaded.\n", pdli->szDll);
            break;

        case dliNotePreLoadLibrary :
            printf("Going to call LoadLibrary() for library %s.\n", pdli->szDll);
            break;

        case dliNotePreGetProcAddress :
            if (pdli->dlp.fImportByName)
            {
                printf("Going to call GetProcAddress() to get procedure: %s in %s.\n", pdli->dlp.szProcName, pdli->szDll);
            }
            else
            {
                printf("Failed to get procedure, ordinal: %d in %s.  Last error: %d\n", pdli->dlp.dwOrdinal, pdli->szDll);
            }
            break;

        case dliNoteEndProcessing :
            printf("Done processing for library %s.\n", pdli->szDll);
            break;

        default :
            printf("Unknown notification for delayed load library %s.\n", pdli->szDll);
            break;
    }

    return NULL;
}
PfnDliHook __pfnDliNotifyHook2 = delayNoteHook;
#endif




REXX_TYPED_ROUTINE_PROTOTYPE(fileNameDlg_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(findWindow_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(locate_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(messageDialog_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(msSleep_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(play_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(simpleFolderBrowse_rtn);
REXX_TYPED_ROUTINE_PROTOTYPE(winTimer_rtn);

REXX_TYPED_ROUTINE_PROTOTYPE(routineTest_rtn);

RexxRoutineEntry oodialog_functions[] =
{
    REXX_TYPED_ROUTINE(fileNameDlg_rtn,        fileNameDlg_rtn),
    REXX_TYPED_ROUTINE(findWindow_rtn,         findWindow_rtn),
    REXX_TYPED_ROUTINE(locate_rtn,             locate_rtn),
    REXX_TYPED_ROUTINE(messageDialog_rtn,      messageDialog_rtn),
    REXX_TYPED_ROUTINE(msSleep_rtn,            msSleep_rtn),
    REXX_TYPED_ROUTINE(play_rtn,               play_rtn),
    REXX_TYPED_ROUTINE(simpleFolderBrowse_rtn, simpleFolderBrowse_rtn),
    REXX_TYPED_ROUTINE(winTimer_rtn,           winTimer_rtn),

    REXX_TYPED_ROUTINE(routineTest_rtn,        routineTest_rtn),

    REXX_LAST_ROUTINE()
};

// DlgUtil
REXX_METHOD_PROTOTYPE(dlgutil_init_cls);
REXX_METHOD_PROTOTYPE(dlgutil_comctl32Version_cls);
REXX_METHOD_PROTOTYPE(dlgutil_getGuid_cls);
REXX_METHOD_PROTOTYPE(dlgutil_version_cls);
REXX_METHOD_PROTOTYPE(dlgutil_errMsg_cls);
REXX_METHOD_PROTOTYPE(dlgutil_hiWord_cls);
REXX_METHOD_PROTOTYPE(dlgutil_loWord_cls);
REXX_METHOD_PROTOTYPE(dlgutil_shiWord_cls);
REXX_METHOD_PROTOTYPE(dlgutil_sloWord_cls);
REXX_METHOD_PROTOTYPE(dlgutil_makeLPARAM_cls);
REXX_METHOD_PROTOTYPE(dlgutil_makeWPARAM_cls);
REXX_METHOD_PROTOTYPE(dlgutil_unsigned_cls);
REXX_METHOD_PROTOTYPE(dlgutil_unsigned32_cls);
REXX_METHOD_PROTOTYPE(dlgutil_signed_cls);
REXX_METHOD_PROTOTYPE(dlgutil_signed32_cls);
REXX_METHOD_PROTOTYPE(dlgutil_and_cls);
REXX_METHOD_PROTOTYPE(dlgutil_or_cls);
REXX_METHOD_PROTOTYPE(dlgutil_shiftLeft_cls);
REXX_METHOD_PROTOTYPE(dlgutil_shiftRight_cls);
REXX_METHOD_PROTOTYPE(dlgutil_sShiftLeft_cls);
REXX_METHOD_PROTOTYPE(dlgutil_sShiftRight_cls);
REXX_METHOD_PROTOTYPE(dlgutil_getSystemMetrics_cls);
REXX_METHOD_PROTOTYPE(dlgutil_screenSize_cls);
REXX_METHOD_PROTOTYPE(dlgutil_screenArea_cls);
REXX_METHOD_PROTOTYPE(dlgutil_halt_cls);
REXX_METHOD_PROTOTYPE(dlgutil_handleToPointer_cls);
REXX_METHOD_PROTOTYPE(dlgutil_terminate_cls);
REXX_METHOD_PROTOTYPE(dlgutil_threadID_cls);
REXX_METHOD_PROTOTYPE(dlgutil_windowFromPoint_cls);
REXX_METHOD_PROTOTYPE(dlgutil_test_cls);

// ApplicationManager
REXX_METHOD_PROTOTYPE(app_init);
REXX_METHOD_PROTOTYPE(app_srcDir_atr);
REXX_METHOD_PROTOTYPE(app_addToConstDir);
REXX_METHOD_PROTOTYPE(app_autoDetection);
REXX_METHOD_PROTOTYPE(app_defaultFont);
REXX_METHOD_PROTOTYPE(app_defaultIcon);
REXX_METHOD_PROTOTYPE(app_initAutoDetection);
REXX_METHOD_PROTOTYPE(app_requiredOS);
REXX_METHOD_PROTOTYPE(app_setDefaults);
REXX_METHOD_PROTOTYPE(app_useGlobalConstDir);

// OS
REXX_METHOD_PROTOTYPE(os_is64bit);
REXX_METHOD_PROTOTYPE(os_is32on64bit);
REXX_METHOD_PROTOTYPE(os_isVersion);
REXX_METHOD_PROTOTYPE(os_settingChanged);
REXX_METHOD_PROTOTYPE(os_shellChangeNotify);

// SPI
REXX_METHOD_PROTOTYPE(spi_init_cls);
REXX_METHOD_PROTOTYPE(spi_getDragHeight_cls);
REXX_METHOD_PROTOTYPE(spi_setDragHeight_cls);
REXX_METHOD_PROTOTYPE(spi_getDragWidth_cls);
REXX_METHOD_PROTOTYPE(spi_setDragWidth_cls);
REXX_METHOD_PROTOTYPE(spi_getMenuAnimation_cls);
REXX_METHOD_PROTOTYPE(spi_setMenuAnimation_cls);
REXX_METHOD_PROTOTYPE(spi_getMenuFade_cls);
REXX_METHOD_PROTOTYPE(spi_setMenuFade_cls);
REXX_METHOD_PROTOTYPE(spi_getMouseHoverHeight_cls);
REXX_METHOD_PROTOTYPE(spi_setMouseHoverHeight_cls);
REXX_METHOD_PROTOTYPE(spi_getMouseHoverTime_cls);
REXX_METHOD_PROTOTYPE(spi_setMouseHoverTime_cls);
REXX_METHOD_PROTOTYPE(spi_getMouseHoverWidth_cls);
REXX_METHOD_PROTOTYPE(spi_setMouseHoverWidth_cls);
REXX_METHOD_PROTOTYPE(spi_getMouseHoverHeight_cls);
REXX_METHOD_PROTOTYPE(spi_setMouseHoverHeight_cls);
REXX_METHOD_PROTOTYPE(spi_getNonClientMetrics_cls);
REXX_METHOD_PROTOTYPE(spi_setNonClientMetrics_cls);
REXX_METHOD_PROTOTYPE(spi_getUpdateFlag_cls);
REXX_METHOD_PROTOTYPE(spi_setUpdateFlag_cls);
REXX_METHOD_PROTOTYPE(spi_getWorkArea_cls);
REXX_METHOD_PROTOTYPE(spi_setWorkArea_cls);
REXX_METHOD_PROTOTYPE(spi_getWheelScrollLines_cls);
REXX_METHOD_PROTOTYPE(spi_setWheelScrollLines_cls);

// SM
REXX_METHOD_PROTOTYPE(sm_cMouseButtons_cls);
REXX_METHOD_PROTOTYPE(sm_cxCursor_cls);
REXX_METHOD_PROTOTYPE(sm_cxDrag_cls);
REXX_METHOD_PROTOTYPE(sm_cxFixedFrame_cls);
REXX_METHOD_PROTOTYPE(sm_cxIcon_cls);
REXX_METHOD_PROTOTYPE(sm_cxScreen_cls);
REXX_METHOD_PROTOTYPE(sm_cxSize_cls);
REXX_METHOD_PROTOTYPE(sm_cxSmIcon_cls);
REXX_METHOD_PROTOTYPE(sm_cxVScroll_cls);
REXX_METHOD_PROTOTYPE(sm_cyCaption_cls);
REXX_METHOD_PROTOTYPE(sm_cyCursor_cls);
REXX_METHOD_PROTOTYPE(sm_cyDrag_cls);
REXX_METHOD_PROTOTYPE(sm_cyFixedFrame_cls);
REXX_METHOD_PROTOTYPE(sm_cyHScroll_cls);
REXX_METHOD_PROTOTYPE(sm_cyIcon_cls);
REXX_METHOD_PROTOTYPE(sm_cyMenu_cls);
REXX_METHOD_PROTOTYPE(sm_cyScreen_cls);
REXX_METHOD_PROTOTYPE(sm_cySize_cls);
REXX_METHOD_PROTOTYPE(sm_cySmIcon_cls);
REXX_METHOD_PROTOTYPE(sm_menuDropAlignment_cls);

// ResourceUtils
REXX_METHOD_PROTOTYPE(rsrcUtils_resolveIconID_pvt);
REXX_METHOD_PROTOTYPE(rsrcUtils_resolveResourceID);
REXX_METHOD_PROTOTYPE(rsrcUtils_idError);
REXX_METHOD_PROTOTYPE(rsrcUtils_checkID);

// WindowBase
REXX_METHOD_PROTOTYPE(wb_getHwnd);
REXX_METHOD_PROTOTYPE(wb_getInitCode);
REXX_METHOD_PROTOTYPE(wb_setInitCode);
REXX_METHOD_PROTOTYPE(wb_getFactorX);
REXX_METHOD_PROTOTYPE(wb_setFactorX);
REXX_METHOD_PROTOTYPE(wb_getFactorY);
REXX_METHOD_PROTOTYPE(wb_setFactorY);
REXX_METHOD_PROTOTYPE(wb_getSizeX);
REXX_METHOD_PROTOTYPE(wb_setSizeX);
REXX_METHOD_PROTOTYPE(wb_getSizeY);
REXX_METHOD_PROTOTYPE(wb_setSizeY);
REXX_METHOD_PROTOTYPE(wb_getPixelCX);
REXX_METHOD_PROTOTYPE(wb_getPixelCY);
REXX_METHOD_PROTOTYPE(wb_init_windowBase);
REXX_METHOD_PROTOTYPE(wb_sendMessage);
REXX_METHOD_PROTOTYPE(wb_sendWinIntMsg);
REXX_METHOD_PROTOTYPE(wb_sendWinUintMsg);
REXX_METHOD_PROTOTYPE(wb_sendWinHandleMsg);
REXX_METHOD_PROTOTYPE(wb_sendWinHandle2Msg);
REXX_METHOD_PROTOTYPE(wb_enable);
REXX_METHOD_PROTOTYPE(wb_isEnabled);
REXX_METHOD_PROTOTYPE(wb_isVisible);
REXX_METHOD_PROTOTYPE(wb_show);
REXX_METHOD_PROTOTYPE(wb_showFast);
REXX_METHOD_PROTOTYPE(wb_display);
REXX_METHOD_PROTOTYPE(wb_redrawClient);
REXX_METHOD_PROTOTYPE(wb_redraw);
REXX_METHOD_PROTOTYPE(wb_getText);
REXX_METHOD_PROTOTYPE(wb_setText);
REXX_METHOD_PROTOTYPE(wb_getTextSizePx);
REXX_METHOD_PROTOTYPE(wb_getTextSizeScreen);
REXX_METHOD_PROTOTYPE(wb_setRect);
REXX_METHOD_PROTOTYPE(wb_resizeMove);
REXX_METHOD_PROTOTYPE(wb_setWindowPos);
REXX_METHOD_PROTOTYPE(wb_moveSizeWindow);
REXX_METHOD_PROTOTYPE(wb_getSizePos);
REXX_METHOD_PROTOTYPE(wb_windowRect);
REXX_METHOD_PROTOTYPE(wb_childWindowFromPoint);
REXX_METHOD_PROTOTYPE(wb_clientRect);
REXX_METHOD_PROTOTYPE(wb_clear);
REXX_METHOD_PROTOTYPE(wb_foreGroundWindow);
REXX_METHOD_PROTOTYPE(wb_screenClient);
REXX_METHOD_PROTOTYPE(wb_mapWindowPoints);
REXX_METHOD_PROTOTYPE(wb_updateWindow);
REXX_METHOD_PROTOTYPE(wb_getWindowLong_pvt);
REXX_METHOD_PROTOTYPE(wb_setWindowLong_pvt);

// EventNotification
REXX_METHOD_PROTOTYPE(en_init_eventNotification);
REXX_METHOD_PROTOTYPE(en_addUserMessage);
REXX_METHOD_PROTOTYPE(en_connectAllSBEvents);
REXX_METHOD_PROTOTYPE(en_connectButtonEvent);
REXX_METHOD_PROTOTYPE(en_connectComboBoxEvent);
REXX_METHOD_PROTOTYPE(en_connectCommandEvents);
REXX_METHOD_PROTOTYPE(en_connectDateTimePickerEvent);
REXX_METHOD_PROTOTYPE(en_connectDraw);
REXX_METHOD_PROTOTYPE(en_connectEachSBEvent);
REXX_METHOD_PROTOTYPE(en_connectEditEvent);
REXX_METHOD_PROTOTYPE(en_connectFKeyPress);
REXX_METHOD_PROTOTYPE(en_connectKeyPress);
REXX_METHOD_PROTOTYPE(en_connectListBoxEvent);
REXX_METHOD_PROTOTYPE(en_connectListViewEvent);
REXX_METHOD_PROTOTYPE(en_connectMonthCalendarEvent);
REXX_METHOD_PROTOTYPE(en_connectNotifyEvent);
REXX_METHOD_PROTOTYPE(en_connectReBarEvent);
REXX_METHOD_PROTOTYPE(en_connectScrollBarEvent);
REXX_METHOD_PROTOTYPE(en_connectStaticEvent);
REXX_METHOD_PROTOTYPE(en_connectStatusBarEvent);
REXX_METHOD_PROTOTYPE(en_connectTabEvent);
REXX_METHOD_PROTOTYPE(en_connectToolBarEvent);
REXX_METHOD_PROTOTYPE(en_connectToolTipEvent);
REXX_METHOD_PROTOTYPE(en_connectTrackBarEvent);
REXX_METHOD_PROTOTYPE(en_connectTreeViewEvent);
REXX_METHOD_PROTOTYPE(en_connectUpDownEvent);
REXX_METHOD_PROTOTYPE(en_connectWmEvent);
REXX_METHOD_PROTOTYPE(en_disconnectKeyPress);
REXX_METHOD_PROTOTYPE(en_hasKeyPressConnection);

// Window
REXX_METHOD_PROTOTYPE(window_init);
REXX_METHOD_PROTOTYPE(window_unInit);

// PlainBaseDialog
REXX_METHOD_PROTOTYPE(pbdlg_init_cls);
REXX_METHOD_PROTOTYPE(pbdlg_setDefaultFont_cls);
REXX_METHOD_PROTOTYPE(pbdlg_getFontName_cls);
REXX_METHOD_PROTOTYPE(pbdlg_getFontSize_cls);
REXX_METHOD_PROTOTYPE(pbdlg_new_cls);
REXX_METHOD_PROTOTYPE(pbdlg_init);
REXX_METHOD_PROTOTYPE(pbdlg_setDlgFont);
REXX_METHOD_PROTOTYPE(pbdlg_getFontNameSize);
REXX_METHOD_PROTOTYPE(pbdlg_setFontName_pvt);
REXX_METHOD_PROTOTYPE(pbdlg_setFontSize_pvt);
REXX_METHOD_PROTOTYPE(pbdlg_getAutoDetect);
REXX_METHOD_PROTOTYPE(pbdlg_setAutoDetect);
REXX_METHOD_PROTOTYPE(pbdlg_getDlgID);
REXX_METHOD_PROTOTYPE(pbdlg_setDlgID);
REXX_METHOD_PROTOTYPE(pbdlg_getParentDlg_pvt);
REXX_METHOD_PROTOTYPE(pbdlg_setParentDlg_pvt);
REXX_METHOD_PROTOTYPE(pbdlg_getOwnerDialog);
REXX_METHOD_PROTOTYPE(pbdlg_setOwnerDialog);
REXX_METHOD_PROTOTYPE(pbdlg_getFinished);
REXX_METHOD_PROTOTYPE(pbdlg_setFinished);
REXX_METHOD_PROTOTYPE(pbdlg_sendMessageToControl);
REXX_METHOD_PROTOTYPE(pbdlg_sendMessageToWindow);
REXX_METHOD_PROTOTYPE(pbdlg_getLibrary);
REXX_METHOD_PROTOTYPE(pbdlg_getResourceID);
REXX_METHOD_PROTOTYPE(pbdlg_get);
REXX_METHOD_PROTOTYPE(pbdlg_getDlgHandle);
REXX_METHOD_PROTOTYPE(pbdlg_isDialogActive);
REXX_METHOD_PROTOTYPE(pbdlg_stopIt);
REXX_METHOD_PROTOTYPE(pbdlg_show);
REXX_METHOD_PROTOTYPE(pbdlg_toTheTop);
REXX_METHOD_PROTOTYPE(pbdlg_getFocus);
REXX_METHOD_PROTOTYPE(pbdlg_setFocus);
REXX_METHOD_PROTOTYPE(pbdlg_tabTo);
REXX_METHOD_PROTOTYPE(pbdlg_pixel2dlgUnit);
REXX_METHOD_PROTOTYPE(pbdlg_dlgUnit2pixel);
REXX_METHOD_PROTOTYPE(pbdlg_backgroundBitmap);
REXX_METHOD_PROTOTYPE(pbdlg_tiledBackgroundBitmap);
REXX_METHOD_PROTOTYPE(pbdlg_backgroundColor);
REXX_METHOD_PROTOTYPE(pbdlg_focusControl);
REXX_METHOD_PROTOTYPE(pbdlg_showControl);
REXX_METHOD_PROTOTYPE(pbdlg_showWindow);
REXX_METHOD_PROTOTYPE(pbdlg_getControlHandle);
REXX_METHOD_PROTOTYPE(pbdlg_getWindowText);
REXX_METHOD_PROTOTYPE(pbdlg_setWindowText);
REXX_METHOD_PROTOTYPE(pbdlg_getTextSizeDu);
REXX_METHOD_PROTOTYPE(pbdlg_getTextSizeTitleBar);
REXX_METHOD_PROTOTYPE(pbdlg_getControlText);
REXX_METHOD_PROTOTYPE(pbdlg_setControlText);
REXX_METHOD_PROTOTYPE(pbdlg_enableDisableControl);
REXX_METHOD_PROTOTYPE(pbdlg_getControlID);
REXX_METHOD_PROTOTYPE(pbdlg_center);
REXX_METHOD_PROTOTYPE(pbdlg_doMinMax);
REXX_METHOD_PROTOTYPE(pbdlg_setTabGroup);
REXX_METHOD_PROTOTYPE(pbdlg_connect_ControName);
REXX_METHOD_PROTOTYPE(pbdlg_setDlgDataFromStem_pvt);
REXX_METHOD_PROTOTYPE(pbdlg_putDlgDataInStem_pvt);
REXX_METHOD_PROTOTYPE(pbdlg_getControlData);
REXX_METHOD_PROTOTYPE(pbdlg_setControlData);
REXX_METHOD_PROTOTYPE(pbdlg_getTextSizeDlg);
REXX_METHOD_PROTOTYPE(pbdlg_createToolTip);
REXX_METHOD_PROTOTYPE(pbdlg_newToolTip);
REXX_METHOD_PROTOTYPE(pbdlg_newControl);
REXX_METHOD_PROTOTYPE(pbdlg_newControlEx);
REXX_METHOD_PROTOTYPE(pbdlg_getNewControl);
REXX_METHOD_PROTOTYPE(pbdlg_putControl);
REXX_METHOD_PROTOTYPE(pbdlg_dumpMessageTable);
REXX_METHOD_PROTOTYPE(pbdlg_unInit);
REXX_METHOD_PROTOTYPE(pbdlg_test);

REXX_METHOD_PROTOTYPE(global_resolveSymbolicID);

// DialogExtensions
REXX_METHOD_PROTOTYPE(dlgext_setWindowRect);
REXX_METHOD_PROTOTYPE(dlgext_getControlRect);
REXX_METHOD_PROTOTYPE(dlgext_clearWindowRect);
REXX_METHOD_PROTOTYPE(dlgext_clearControlRect);
REXX_METHOD_PROTOTYPE(dlgext_clearRect);
REXX_METHOD_PROTOTYPE(dlgext_redrawRect);
REXX_METHOD_PROTOTYPE(dlgext_redrawWindowRect);
REXX_METHOD_PROTOTYPE(dlgext_redrawControl);
REXX_METHOD_PROTOTYPE(dlgext_resizeMoveControl);
REXX_METHOD_PROTOTYPE(dlgext_setForgroundWindow);
REXX_METHOD_PROTOTYPE(dlgext_setControlColor);
REXX_METHOD_PROTOTYPE(dlgext_installBitmapButton);
REXX_METHOD_PROTOTYPE(dlgext_changeBitmapButton);
REXX_METHOD_PROTOTYPE(dlgext_drawBitmap);
REXX_METHOD_PROTOTYPE(dlgext_dimBitmap);
REXX_METHOD_PROTOTYPE(dlgext_scrollButton);
REXX_METHOD_PROTOTYPE(dlgext_drawButton);
REXX_METHOD_PROTOTYPE(dlgext_getBitmapPosition);
REXX_METHOD_PROTOTYPE(dlgext_setBitmapPosition);
REXX_METHOD_PROTOTYPE(dlgext_getBitmapSize);
REXX_METHOD_PROTOTYPE(dlgext_getWindowDC);
REXX_METHOD_PROTOTYPE(dlgext_freeWindowDC);
REXX_METHOD_PROTOTYPE(dlgext_scrollText);
REXX_METHOD_PROTOTYPE(dlgext_writeToWindow);
REXX_METHOD_PROTOTYPE(dlgext_createBrush);

// UserDialog
REXX_METHOD_PROTOTYPE(userdlg_init);
REXX_METHOD_PROTOTYPE(userdlg_test);

// CategoryDialog
REXX_METHOD_PROTOTYPE(catdlg_createCategoryDialog);
REXX_METHOD_PROTOTYPE(catdlg_getControlDataPage);
REXX_METHOD_PROTOTYPE(catdlg_setControlDataPage);
REXX_METHOD_PROTOTYPE(catdlg_sendMessageToCategoryControl);

// DynamicDialog
REXX_METHOD_PROTOTYPE(dyndlg_init_cls);
REXX_METHOD_PROTOTYPE(dyndlg_getBasePtr);
REXX_METHOD_PROTOTYPE(dyndlg_setBasePtr);
REXX_METHOD_PROTOTYPE(dyndlg_getActivePtr);
REXX_METHOD_PROTOTYPE(dyndlg_setActivePtr);
REXX_METHOD_PROTOTYPE(dyndlg_getDialogItemCount);
REXX_METHOD_PROTOTYPE(dyndlg_setDialogItemCount);
REXX_METHOD_PROTOTYPE(dyndlg_dynamicInit);
REXX_METHOD_PROTOTYPE(dyndlg_create);
REXX_METHOD_PROTOTYPE(dyndlg_startParentDialog);
REXX_METHOD_PROTOTYPE(dyndlg_startChildDialog);
REXX_METHOD_PROTOTYPE(dyndlg_createStatic);
REXX_METHOD_PROTOTYPE(dyndlg_createStaticText);
REXX_METHOD_PROTOTYPE(dyndlg_createStaticImage);
REXX_METHOD_PROTOTYPE(dyndlg_createStaticFrame);
REXX_METHOD_PROTOTYPE(dyndlg_createPushButton);
REXX_METHOD_PROTOTYPE(dyndlg_createPushButtonStem);
REXX_METHOD_PROTOTYPE(dyndlg_createRadioButton);
REXX_METHOD_PROTOTYPE(dyndlg_createGroupBox);
REXX_METHOD_PROTOTYPE(dyndlg_createEdit);
REXX_METHOD_PROTOTYPE(dyndlg_createScrollBar);
REXX_METHOD_PROTOTYPE(dyndlg_createListBox);
REXX_METHOD_PROTOTYPE(dyndlg_createComboBox);
REXX_METHOD_PROTOTYPE(dyndlg_createProgressBar);
REXX_METHOD_PROTOTYPE(dyndlg_createNamedControl);
REXX_METHOD_PROTOTYPE(dyndlg_addButton);
REXX_METHOD_PROTOTYPE(dyndlg_addRadioButton);
REXX_METHOD_PROTOTYPE(dyndlg_addGroupBox);
REXX_METHOD_PROTOTYPE(dyndlg_addEntryLine);
REXX_METHOD_PROTOTYPE(dyndlg_addMethod);
REXX_METHOD_PROTOTYPE(dyndlg_addIconResource);
REXX_METHOD_PROTOTYPE(dyndlg_stop);

// ResizingAdmin
REXX_METHOD_PROTOTYPE(ra_maxSize);
REXX_METHOD_PROTOTYPE(ra_setMaxSize);
REXX_METHOD_PROTOTYPE(ra_minSize);
REXX_METHOD_PROTOTYPE(ra_setMinSize);
REXX_METHOD_PROTOTYPE(ra_controlSide);
REXX_METHOD_PROTOTYPE(ra_controlSizing);
REXX_METHOD_PROTOTYPE(ra_defaultSide);
REXX_METHOD_PROTOTYPE(ra_defaultSizing);
REXX_METHOD_PROTOTYPE(ra_initResizing);
REXX_METHOD_PROTOTYPE(ra_noMaxSize);
REXX_METHOD_PROTOTYPE(ra_noMinSize);
REXX_METHOD_PROTOTYPE(ra_pagedTab);
REXX_METHOD_PROTOTYPE(ra_useDefaultSizing);
REXX_METHOD_PROTOTYPE(ra_wantSizeEnded);

// ResDialog
REXX_METHOD_PROTOTYPE(resdlg_init);
REXX_METHOD_PROTOTYPE(resdlg_getDataTableIDs_pvt);
REXX_METHOD_PROTOTYPE(resdlg_startDialog_pvt);

// TabOwnerDialog
REXX_METHOD_PROTOTYPE(tod_tabOwnerDlgInit);
REXX_METHOD_PROTOTYPE(tod_getTabPage);

// TabOwnerDlgInfo
REXX_METHOD_PROTOTYPE(todi_init);
REXX_METHOD_PROTOTYPE(todi_add);
REXX_METHOD_PROTOTYPE(tod_tabOwnerOk);
REXX_METHOD_PROTOTYPE(tod_tabOwnerCancel);

// ManagedTab
REXX_METHOD_PROTOTYPE(mt_init);

// ControlDialog
REXX_METHOD_PROTOTYPE(cd_init_cls);
REXX_METHOD_PROTOTYPE(cd_controlDlgInit);
REXX_METHOD_PROTOTYPE(cd_get_isManaged);
REXX_METHOD_PROTOTYPE(cd_get_wasActivated);
REXX_METHOD_PROTOTYPE(cd_get_extraOptions);
REXX_METHOD_PROTOTYPE(cd_set_extraOptions);
REXX_METHOD_PROTOTYPE(cd_get_initializing);
REXX_METHOD_PROTOTYPE(cd_set_initializing);
REXX_METHOD_PROTOTYPE(cd_get_pageTitle);
REXX_METHOD_PROTOTYPE(cd_set_pageTitle);

// ControlDlgInfo
REXX_METHOD_PROTOTYPE(cdi_set_managed);
REXX_METHOD_PROTOTYPE(cdi_set_title);
REXX_METHOD_PROTOTYPE(cdi_set_size);
REXX_METHOD_PROTOTYPE(cdi_init);

// ResControlDialog
REXX_METHOD_PROTOTYPE(resCtrlDlg_startDialog_pvt);

// RcControlDialog
REXX_METHOD_PROTOTYPE(rcCtrlDlg_startTemplate);

// PropertySheetDialog
REXX_METHOD_PROTOTYPE(psdlg_getPages_atr);
REXX_METHOD_PROTOTYPE(psdlg_setCaption_atr);
REXX_METHOD_PROTOTYPE(psdlg_setResources_atr);
REXX_METHOD_PROTOTYPE(psdlg_setAppIcon_atr);
REXX_METHOD_PROTOTYPE(psdlg_setHeader_atr);
REXX_METHOD_PROTOTYPE(psdlg_setWatermark_atr);
REXX_METHOD_PROTOTYPE(psdlg_setStartPage_atr);
REXX_METHOD_PROTOTYPE(psdlg_setImageList_atr);
REXX_METHOD_PROTOTYPE(psdlg_init);
REXX_METHOD_PROTOTYPE(psdlg_execute);
REXX_METHOD_PROTOTYPE(psdlg_getPage);
REXX_METHOD_PROTOTYPE(psdlg_addPage);
REXX_METHOD_PROTOTYPE(psdlg_insertPage);
REXX_METHOD_PROTOTYPE(psdlg_removePage);
REXX_METHOD_PROTOTYPE(psdlg_apply);
REXX_METHOD_PROTOTYPE(psdlg_cancelToClose);
REXX_METHOD_PROTOTYPE(psdlg_changed);
REXX_METHOD_PROTOTYPE(psdlg_getCurrentPageHwnd);
REXX_METHOD_PROTOTYPE(psdlg_getTabControl);
REXX_METHOD_PROTOTYPE(psdlg_hwndToIndex);
REXX_METHOD_PROTOTYPE(psdlg_idToIndex);
REXX_METHOD_PROTOTYPE(psdlg_getResult);
REXX_METHOD_PROTOTYPE(psdlg_indexToID);
REXX_METHOD_PROTOTYPE(psdlg_indexToHandle);
REXX_METHOD_PROTOTYPE(psdlg_pageToIndex);
REXX_METHOD_PROTOTYPE(psdlg_popup);
REXX_METHOD_PROTOTYPE(psdlg_popupAsChild);
REXX_METHOD_PROTOTYPE(psdlg_pressButton);
REXX_METHOD_PROTOTYPE(psdlg_setCurSel);
REXX_METHOD_PROTOTYPE(psdlg_setCurSelByID);
REXX_METHOD_PROTOTYPE(psdlg_resetPageText);
REXX_METHOD_PROTOTYPE(psdlg_setTitle);
REXX_METHOD_PROTOTYPE(psdlg_setWizButtons);
REXX_METHOD_PROTOTYPE(psdlg_enableWizButtons);
REXX_METHOD_PROTOTYPE(psdlg_showWizButtons);
REXX_METHOD_PROTOTYPE(psdlg_querySiblings);
REXX_METHOD_PROTOTYPE(psdlg_setButtonText);
REXX_METHOD_PROTOTYPE(psdlg_setFinishText);
REXX_METHOD_PROTOTYPE(psdlg_setNextText);
REXX_METHOD_PROTOTYPE(psdlg_unchanged);
REXX_METHOD_PROTOTYPE(psdlg_test);

// PropertySheetPage
REXX_METHOD_PROTOTYPE(psp_init_cls);
REXX_METHOD_PROTOTYPE(psp_propSheet_atr);
REXX_METHOD_PROTOTYPE(psp_wasActivated_atr);
REXX_METHOD_PROTOTYPE(psp_pageID_atr);
REXX_METHOD_PROTOTYPE(psp_pageNumber_atr);
REXX_METHOD_PROTOTYPE(psp_setResources_atr);
REXX_METHOD_PROTOTYPE(psp_setTabIcon_atr);
REXX_METHOD_PROTOTYPE(psp_getcx);
REXX_METHOD_PROTOTYPE(psp_setcx);
REXX_METHOD_PROTOTYPE(psp_getPageTitle);
REXX_METHOD_PROTOTYPE(psp_setPageTitle);
REXX_METHOD_PROTOTYPE(psp_getWantNotification);
REXX_METHOD_PROTOTYPE(psp_setWantNotification);
REXX_METHOD_PROTOTYPE(psp_init_propertySheetPage);
REXX_METHOD_PROTOTYPE(psp_initTemplate);
REXX_METHOD_PROTOTYPE(psp_setSize);

// ResPSPDialog
REXX_METHOD_PROTOTYPE(respspdlg_init);

// RcPSPDialog
REXX_METHOD_PROTOTYPE(rcpspdlg_init);
REXX_METHOD_PROTOTYPE(rcpspdlg_startTemplate);

// UserPSPDialog
REXX_METHOD_PROTOTYPE(userpspdlg_init);

// ProgressDialog
REXX_METHOD_PROTOTYPE(pd_setInterruptible);

// TimedMessage
REXX_METHOD_PROTOTYPE(timedmsg_init);

// WindowExtensions
REXX_METHOD_PROTOTYPE(winex_initWindowExtensions);
REXX_METHOD_PROTOTYPE(winex_getFont);
REXX_METHOD_PROTOTYPE(winex_setFont);
REXX_METHOD_PROTOTYPE(winex_createFontEx);
REXX_METHOD_PROTOTYPE(winex_createFont);
REXX_METHOD_PROTOTYPE(winex_getScrollPos);
REXX_METHOD_PROTOTYPE(winex_setScrollPos);
REXX_METHOD_PROTOTYPE(winex_scroll);
REXX_METHOD_PROTOTYPE(winex_writeDirect);
REXX_METHOD_PROTOTYPE(winex_loadBitmap);
REXX_METHOD_PROTOTYPE(winex_removeBitmap);
REXX_METHOD_PROTOTYPE(winex_write);
REXX_METHOD_PROTOTYPE(winex_createBrush);
REXX_METHOD_PROTOTYPE(winex_createPen);
REXX_METHOD_PROTOTYPE(winex_deleteObject);
REXX_METHOD_PROTOTYPE(winex_getSysBrush);
REXX_METHOD_PROTOTYPE(winex_getTextAlign);
REXX_METHOD_PROTOTYPE(winex_setTextAlign);
REXX_METHOD_PROTOTYPE(winex_getTextExtent);
REXX_METHOD_PROTOTYPE(winex_getDC);
REXX_METHOD_PROTOTYPE(winex_freeDC);
REXX_METHOD_PROTOTYPE(winex_rectangle);
REXX_METHOD_PROTOTYPE(winex_objectToDC);
REXX_METHOD_PROTOTYPE(winex_drawLine);
REXX_METHOD_PROTOTYPE(winex_drawPixel);
REXX_METHOD_PROTOTYPE(winex_getPixel);
REXX_METHOD_PROTOTYPE(winex_fillDrawing);
REXX_METHOD_PROTOTYPE(winex_fillRect);
REXX_METHOD_PROTOTYPE(winex_drawArcOrPie);
REXX_METHOD_PROTOTYPE(winex_drawAngleArc);
REXX_METHOD_PROTOTYPE(winex_fontColor);
REXX_METHOD_PROTOTYPE(winex_textBkMode);
REXX_METHOD_PROTOTYPE(winex_getSetArcDirection);

// ResourceImage
REXX_METHOD_PROTOTYPE(ri_init);
REXX_METHOD_PROTOTYPE(ri_release);
REXX_METHOD_PROTOTYPE(ri_handle);
REXX_METHOD_PROTOTYPE(ri_isNull);
REXX_METHOD_PROTOTYPE(ri_systemErrorCode);
REXX_METHOD_PROTOTYPE(ri_getImage);
REXX_METHOD_PROTOTYPE(ri_getImages);

// Image
REXX_METHOD_PROTOTYPE(image_toID_cls);
REXX_METHOD_PROTOTYPE(image_getImage_cls);
REXX_METHOD_PROTOTYPE(image_fromFiles_cls);
REXX_METHOD_PROTOTYPE(image_fromIDs_cls);
REXX_METHOD_PROTOTYPE(image_userIcon_cls);
REXX_METHOD_PROTOTYPE(image_colorRef_cls);
REXX_METHOD_PROTOTYPE(image_getRValue_cls);
REXX_METHOD_PROTOTYPE(image_getGValue_cls);
REXX_METHOD_PROTOTYPE(image_getBValue_cls);
REXX_METHOD_PROTOTYPE(image_init);
REXX_METHOD_PROTOTYPE(image_release);
REXX_METHOD_PROTOTYPE(image_isNull);
REXX_METHOD_PROTOTYPE(image_handle);
REXX_METHOD_PROTOTYPE(image_systemErrorCode);

// ImageList
REXX_METHOD_PROTOTYPE(il_create_cls);
REXX_METHOD_PROTOTYPE(il_init);
REXX_METHOD_PROTOTYPE(il_add);
REXX_METHOD_PROTOTYPE(il_addIcon);
REXX_METHOD_PROTOTYPE(il_addImages);
REXX_METHOD_PROTOTYPE(il_addMasked);
REXX_METHOD_PROTOTYPE(il_duplicate);
REXX_METHOD_PROTOTYPE(il_getCount);
REXX_METHOD_PROTOTYPE(il_getIcon);
REXX_METHOD_PROTOTYPE(il_getImageSize);
REXX_METHOD_PROTOTYPE(il_handle);
REXX_METHOD_PROTOTYPE(il_isNull);
REXX_METHOD_PROTOTYPE(il_release);
REXX_METHOD_PROTOTYPE(il_remove);
REXX_METHOD_PROTOTYPE(il_removeAll);

// DialogControl
REXX_METHOD_PROTOTYPE(dlgctrl_new_cls);
REXX_METHOD_PROTOTYPE(dlgctrl_init_cls);
REXX_METHOD_PROTOTYPE(dlgctrl_init);
REXX_METHOD_PROTOTYPE(dlgctrl_unInit);
REXX_METHOD_PROTOTYPE(dlgctrl_addUserSubclass);
REXX_METHOD_PROTOTYPE(dlgctrl_assignFocus);
REXX_METHOD_PROTOTYPE(dlgctrl_clearRect);
REXX_METHOD_PROTOTYPE(dlgctrl_connectEvent);
REXX_METHOD_PROTOTYPE(dlgctrl_connectFKeyPress);
REXX_METHOD_PROTOTYPE(dlgctrl_connectKeyPress);
REXX_METHOD_PROTOTYPE(dlgctrl_data);
REXX_METHOD_PROTOTYPE(dlgctrl_dataEquals);
REXX_METHOD_PROTOTYPE(dlgctrl_disconnectKeyPress);
REXX_METHOD_PROTOTYPE(dlgctrl_getTextSizeDlg);
REXX_METHOD_PROTOTYPE(dlgctrl_hasKeyPressConnection);
REXX_METHOD_PROTOTYPE(dlgctrl_redrawRect);
REXX_METHOD_PROTOTYPE(dlgctrl_setColor);
REXX_METHOD_PROTOTYPE(dlgctrl_setParent);
REXX_METHOD_PROTOTYPE(dlgctrl_setWindowTheme);
REXX_METHOD_PROTOTYPE(dlgctrl_tabGroup);
REXX_METHOD_PROTOTYPE(dlgctrl_textSize);
REXX_METHOD_PROTOTYPE(dlgctrl_putInBag);

// Generic methods for dialog controls
REXX_METHOD_PROTOTYPE(generic_getToolTips);
REXX_METHOD_PROTOTYPE(generic_setListTabulators);
REXX_METHOD_PROTOTYPE(generic_subclassEdit);
REXX_METHOD_PROTOTYPE(generic_setToolTips);

// Static
REXX_METHOD_PROTOTYPE(stc_getIcon);
REXX_METHOD_PROTOTYPE(stc_setIcon);
REXX_METHOD_PROTOTYPE(stc_getImage);
REXX_METHOD_PROTOTYPE(stc_setImage);

// Button
REXX_METHOD_PROTOTYPE(gb_setStyle);
REXX_METHOD_PROTOTYPE(bc_getState);
REXX_METHOD_PROTOTYPE(bc_setState);
REXX_METHOD_PROTOTYPE(bc_setStyle);
REXX_METHOD_PROTOTYPE(bc_getIdealSize);
REXX_METHOD_PROTOTYPE(bc_getTextMargin);
REXX_METHOD_PROTOTYPE(bc_setTextMargin);
REXX_METHOD_PROTOTYPE(bc_getImage);
REXX_METHOD_PROTOTYPE(bc_setImage);
REXX_METHOD_PROTOTYPE(bc_setImageList);
REXX_METHOD_PROTOTYPE(bc_getImageList);
REXX_METHOD_PROTOTYPE(rb_checkInGroup_cls);
REXX_METHOD_PROTOTYPE(rb_getCheckState);
REXX_METHOD_PROTOTYPE(rb_checked);
REXX_METHOD_PROTOTYPE(rb_check);
REXX_METHOD_PROTOTYPE(rb_uncheck);
REXX_METHOD_PROTOTYPE(rb_isChecked);
REXX_METHOD_PROTOTYPE(rb_indeterminate);
REXX_METHOD_PROTOTYPE(ckbx_isIndeterminate);
REXX_METHOD_PROTOTYPE(ckbx_setIndeterminate);
REXX_METHOD_PROTOTYPE(bc_test);
REXX_METHOD_PROTOTYPE(bc_test_cls);

// Edit
REXX_METHOD_PROTOTYPE(e_isSingleLine);
REXX_METHOD_PROTOTYPE(e_selection);
REXX_METHOD_PROTOTYPE(e_replaceSelText);
REXX_METHOD_PROTOTYPE(e_getLine);
REXX_METHOD_PROTOTYPE(e_isGrandChild);
REXX_METHOD_PROTOTYPE(e_lineIndex);
REXX_METHOD_PROTOTYPE(e_lineFromIndex);
REXX_METHOD_PROTOTYPE(e_lineLength);
REXX_METHOD_PROTOTYPE(e_setTabStops);
REXX_METHOD_PROTOTYPE(e_showBallon);
REXX_METHOD_PROTOTYPE(e_hideBallon);
REXX_METHOD_PROTOTYPE(e_getCue);
REXX_METHOD_PROTOTYPE(e_setCue);
REXX_METHOD_PROTOTYPE(e_getRect);
REXX_METHOD_PROTOTYPE(e_setRect);
REXX_METHOD_PROTOTYPE(e_style);

// ComboBox
REXX_METHOD_PROTOTYPE(cb_add);
REXX_METHOD_PROTOTYPE(cb_addDirectory);
REXX_METHOD_PROTOTYPE(cb_find);
REXX_METHOD_PROTOTYPE(cb_getCue);
REXX_METHOD_PROTOTYPE(cb_getComboBoxInfo);
REXX_METHOD_PROTOTYPE(cb_getEditControl);
REXX_METHOD_PROTOTYPE(cb_getItemData);
REXX_METHOD_PROTOTYPE(cb_setItemData);
REXX_METHOD_PROTOTYPE(cb_getItemHeight);
REXX_METHOD_PROTOTYPE(cb_getMinVisible);
REXX_METHOD_PROTOTYPE(cb_getText);
REXX_METHOD_PROTOTYPE(cb_insert);
REXX_METHOD_PROTOTYPE(cb_isDropDown);
REXX_METHOD_PROTOTYPE(cb_isGrandchild);
REXX_METHOD_PROTOTYPE(cb_removeFullColor);
REXX_METHOD_PROTOTYPE(cb_removeItemData);
REXX_METHOD_PROTOTYPE(cb_select);
REXX_METHOD_PROTOTYPE(cb_setCue);
REXX_METHOD_PROTOTYPE(cb_setFullColor);
REXX_METHOD_PROTOTYPE(cb_setItemData);
REXX_METHOD_PROTOTYPE(cb_setItemHeight);
REXX_METHOD_PROTOTYPE(cb_setMinVisible);

// ScrollBar
REXX_METHOD_PROTOTYPE(sb_getRange);
REXX_METHOD_PROTOTYPE(sb_setRange);
REXX_METHOD_PROTOTYPE(sb_getPosition);
REXX_METHOD_PROTOTYPE(sb_setPosition);

// StatusBar
REXX_METHOD_PROTOTYPE(stb_getBorders);
REXX_METHOD_PROTOTYPE(stb_getIcon);
REXX_METHOD_PROTOTYPE(stb_getParts);
REXX_METHOD_PROTOTYPE(stb_getRect);
REXX_METHOD_PROTOTYPE(stb_getText);
REXX_METHOD_PROTOTYPE(stb_getTextLength);
REXX_METHOD_PROTOTYPE(stb_getTipText);
REXX_METHOD_PROTOTYPE(stb_isSimple);
REXX_METHOD_PROTOTYPE(stb_setBkColor);
REXX_METHOD_PROTOTYPE(stb_setIcon);
REXX_METHOD_PROTOTYPE(stb_setMinHeight);
REXX_METHOD_PROTOTYPE(stb_setParts);
REXX_METHOD_PROTOTYPE(stb_setText);
REXX_METHOD_PROTOTYPE(stb_setTipText);
REXX_METHOD_PROTOTYPE(stb_simple);

// ListBox
REXX_METHOD_PROTOTYPE(lb_add);
REXX_METHOD_PROTOTYPE(lb_addDirectory);
REXX_METHOD_PROTOTYPE(lb_deselectIndex);
REXX_METHOD_PROTOTYPE(lb_find);
REXX_METHOD_PROTOTYPE(lb_getText);
REXX_METHOD_PROTOTYPE(lb_hitTestInfo);
REXX_METHOD_PROTOTYPE(lb_insert);
REXX_METHOD_PROTOTYPE(lb_isSingleSelection);
REXX_METHOD_PROTOTYPE(lb_select);
REXX_METHOD_PROTOTYPE(lb_selectedIndex);
REXX_METHOD_PROTOTYPE(lb_selectIndex);

// ProgressBar
REXX_METHOD_PROTOTYPE(pbc_getFullRange);
REXX_METHOD_PROTOTYPE(pbc_setFullRange);
REXX_METHOD_PROTOTYPE(pbc_setMarquee);

// ReBar
REXX_METHOD_PROTOTYPE(rebar_deleteBand);
REXX_METHOD_PROTOTYPE(rebar_getBandBorders);
REXX_METHOD_PROTOTYPE(rebar_getBandCount);
REXX_METHOD_PROTOTYPE(rebar_getBandInfo);
REXX_METHOD_PROTOTYPE(rebar_getBandMargins);
REXX_METHOD_PROTOTYPE(rebar_getBarHeight);
REXX_METHOD_PROTOTYPE(rebar_getBarInfo);
REXX_METHOD_PROTOTYPE(rebar_getBkColor);
REXX_METHOD_PROTOTYPE(rebar_getColorScheme);
REXX_METHOD_PROTOTYPE(rebar_getExtendedStyle);
REXX_METHOD_PROTOTYPE(rebar_getPalette);
REXX_METHOD_PROTOTYPE(rebar_getRect);
REXX_METHOD_PROTOTYPE(rebar_getRowCount);
REXX_METHOD_PROTOTYPE(rebar_getRowHeight);
REXX_METHOD_PROTOTYPE(rebar_getTextColor);
REXX_METHOD_PROTOTYPE(rebar_hitTestInfo);
REXX_METHOD_PROTOTYPE(rebar_idToIndex);
REXX_METHOD_PROTOTYPE(rebar_insertBand);
REXX_METHOD_PROTOTYPE(rebar_maximizeBand);
REXX_METHOD_PROTOTYPE(rebar_minimizeBand);
REXX_METHOD_PROTOTYPE(rebar_moveBand);
REXX_METHOD_PROTOTYPE(rebar_pushChevron);
REXX_METHOD_PROTOTYPE(rebar_setBandInfo);
REXX_METHOD_PROTOTYPE(rebar_setBandWidth);
REXX_METHOD_PROTOTYPE(rebar_setBarInfo);
REXX_METHOD_PROTOTYPE(rebar_setBkColor);
REXX_METHOD_PROTOTYPE(rebar_setColorScheme);
REXX_METHOD_PROTOTYPE(rebar_setExtendedStyle);
REXX_METHOD_PROTOTYPE(rebar_setPalette);
REXX_METHOD_PROTOTYPE(rebar_setParent);
REXX_METHOD_PROTOTYPE(rebar_setTextColor);
REXX_METHOD_PROTOTYPE(rebar_setWindowTheme);
REXX_METHOD_PROTOTYPE(rebar_showBand);
REXX_METHOD_PROTOTYPE(rebar_sizeToRect);

// ReBarBandInfo
REXX_METHOD_PROTOTYPE(rbbi_init_cls          );
REXX_METHOD_PROTOTYPE(rbbi_unInit            );
REXX_METHOD_PROTOTYPE(rbbi_init              );
REXX_METHOD_PROTOTYPE(rbbi_bitmapBack        );
REXX_METHOD_PROTOTYPE(rbbi_setBitmapBack     );
REXX_METHOD_PROTOTYPE(rbbi_chevronLocation   );
REXX_METHOD_PROTOTYPE(rbbi_setChevronLocation);
REXX_METHOD_PROTOTYPE(rbbi_chevronState      );
REXX_METHOD_PROTOTYPE(rbbi_setChevronState   );
REXX_METHOD_PROTOTYPE(rbbi_child             );
REXX_METHOD_PROTOTYPE(rbbi_setChild          );
REXX_METHOD_PROTOTYPE(rbbi_clrBack           );
REXX_METHOD_PROTOTYPE(rbbi_setClrBack        );
REXX_METHOD_PROTOTYPE(rbbi_clrFore           );
REXX_METHOD_PROTOTYPE(rbbi_setClrFore        );
REXX_METHOD_PROTOTYPE(rbbi_cx                );
REXX_METHOD_PROTOTYPE(rbbi_setCx             );
REXX_METHOD_PROTOTYPE(rbbi_cxHeader          );
REXX_METHOD_PROTOTYPE(rbbi_setCxHeader       );
REXX_METHOD_PROTOTYPE(rbbi_cxIdeal           );
REXX_METHOD_PROTOTYPE(rbbi_setCxIdeal        );
REXX_METHOD_PROTOTYPE(rbbi_cxMinChild        );
REXX_METHOD_PROTOTYPE(rbbi_setCxMinChild     );
REXX_METHOD_PROTOTYPE(rbbi_cyChild           );
REXX_METHOD_PROTOTYPE(rbbi_setCyChild        );
REXX_METHOD_PROTOTYPE(rbbi_cyIntegral        );
REXX_METHOD_PROTOTYPE(rbbi_setCyIntegral     );
REXX_METHOD_PROTOTYPE(rbbi_cyMaxChild        );
REXX_METHOD_PROTOTYPE(rbbi_setCyMaxChild     );
REXX_METHOD_PROTOTYPE(rbbi_cyMinChild        );
REXX_METHOD_PROTOTYPE(rbbi_setCyMinChild     );
REXX_METHOD_PROTOTYPE(rbbi_id                );
REXX_METHOD_PROTOTYPE(rbbi_setId             );
REXX_METHOD_PROTOTYPE(rbbi_imageIndex        );
REXX_METHOD_PROTOTYPE(rbbi_setImageIndex     );
REXX_METHOD_PROTOTYPE(rbbi_itemData          );
REXX_METHOD_PROTOTYPE(rbbi_setItemData       );
REXX_METHOD_PROTOTYPE(rbbi_mask              );
REXX_METHOD_PROTOTYPE(rbbi_setMask           );
REXX_METHOD_PROTOTYPE(rbbi_style             );
REXX_METHOD_PROTOTYPE(rbbi_setStyle          );
REXX_METHOD_PROTOTYPE(rbbi_text              );
REXX_METHOD_PROTOTYPE(rbbi_setText           );

// ToolBar
REXX_METHOD_PROTOTYPE(tb_addBitmap);
REXX_METHOD_PROTOTYPE(tb_addButtons);
REXX_METHOD_PROTOTYPE(tb_addString);
REXX_METHOD_PROTOTYPE(tb_autoSize);
REXX_METHOD_PROTOTYPE(tb_buttonCount);
REXX_METHOD_PROTOTYPE(tb_changeBitmap);
REXX_METHOD_PROTOTYPE(tb_checkButton);
REXX_METHOD_PROTOTYPE(tb_commandToIndex);
REXX_METHOD_PROTOTYPE(tb_customize);
REXX_METHOD_PROTOTYPE(tb_enableButton);
REXX_METHOD_PROTOTYPE(tb_getButton);
REXX_METHOD_PROTOTYPE(tb_getButtonText);
REXX_METHOD_PROTOTYPE(tb_getButtonTextEx);
REXX_METHOD_PROTOTYPE(tb_getDisabledImageList);
REXX_METHOD_PROTOTYPE(tb_getExtendedStyle);
REXX_METHOD_PROTOTYPE(tb_getHotImageList);
REXX_METHOD_PROTOTYPE(tb_getImageList);
REXX_METHOD_PROTOTYPE(tb_getPressedImageList);
REXX_METHOD_PROTOTYPE(tb_indexTocommand);
REXX_METHOD_PROTOTYPE(tb_insertButton);
REXX_METHOD_PROTOTYPE(tb_isButtonChecked);
REXX_METHOD_PROTOTYPE(tb_isButtonEnabled);
REXX_METHOD_PROTOTYPE(tb_isButtonHidden);
REXX_METHOD_PROTOTYPE(tb_isButtonHighlighted);
REXX_METHOD_PROTOTYPE(tb_isButtonIndeterminate);
REXX_METHOD_PROTOTYPE(tb_isButtonPressed);
REXX_METHOD_PROTOTYPE(tb_loadImages);
REXX_METHOD_PROTOTYPE(tb_setBitmapSize);
REXX_METHOD_PROTOTYPE(tb_setButtonText);
REXX_METHOD_PROTOTYPE(tb_setDisabledImageList);
REXX_METHOD_PROTOTYPE(tb_setExtendedStyle);
REXX_METHOD_PROTOTYPE(tb_setHotImageList);
REXX_METHOD_PROTOTYPE(tb_setImageList);
REXX_METHOD_PROTOTYPE(tb_setPressedImageList);

// TbButton
REXX_METHOD_PROTOTYPE(tbb_init_cls    );
REXX_METHOD_PROTOTYPE(tbb_init        );
REXX_METHOD_PROTOTYPE(tbb_bitmapID    );
REXX_METHOD_PROTOTYPE(tbb_setBitmapID );
REXX_METHOD_PROTOTYPE(tbb_cmdID       );
REXX_METHOD_PROTOTYPE(tbb_setCmdID    );
REXX_METHOD_PROTOTYPE(tbb_itemData    );
REXX_METHOD_PROTOTYPE(tbb_setItemData );
REXX_METHOD_PROTOTYPE(tbb_state       );
REXX_METHOD_PROTOTYPE(tbb_setState    );
REXX_METHOD_PROTOTYPE(tbb_style       );
REXX_METHOD_PROTOTYPE(tbb_setStyle    );
REXX_METHOD_PROTOTYPE(tbb_text        );
REXX_METHOD_PROTOTYPE(tbb_setText     );
REXX_METHOD_PROTOTYPE(tbb_assignBitmapID);

// TrackBar
REXX_METHOD_PROTOTYPE(trckbar_getRange);
REXX_METHOD_PROTOTYPE(trckbar_getSelRange);

// UpDown
REXX_METHOD_PROTOTYPE(ud_deltaPosReply_cls);
REXX_METHOD_PROTOTYPE(ud_getRange);
REXX_METHOD_PROTOTYPE(ud_setRange);
REXX_METHOD_PROTOTYPE(ud_getPosition);
REXX_METHOD_PROTOTYPE(ud_getBuddy);
REXX_METHOD_PROTOTYPE(ud_setBuddy);
REXX_METHOD_PROTOTYPE(ud_getAcceleration);
REXX_METHOD_PROTOTYPE(ud_setAcceleration);

// ListView
REXX_METHOD_PROTOTYPE(lv_add);
REXX_METHOD_PROTOTYPE(lv_addClearExtendStyle);
REXX_METHOD_PROTOTYPE(lv_addFullRow);
REXX_METHOD_PROTOTYPE(lv_addRemoveStyle);
REXX_METHOD_PROTOTYPE(lv_addRow);
REXX_METHOD_PROTOTYPE(lv_addRowFromArray);
REXX_METHOD_PROTOTYPE(lv_arrange);
REXX_METHOD_PROTOTYPE(lv_checkUncheck);
REXX_METHOD_PROTOTYPE(lv_delete);
REXX_METHOD_PROTOTYPE(lv_deleteAll);
REXX_METHOD_PROTOTYPE(lv_deleteColumn);
REXX_METHOD_PROTOTYPE(lv_deselectAll);
REXX_METHOD_PROTOTYPE(lv_find);
REXX_METHOD_PROTOTYPE(lv_findNearestXY);
REXX_METHOD_PROTOTYPE(lv_fixFullRowColumns);
REXX_METHOD_PROTOTYPE(lv_getCheck);
REXX_METHOD_PROTOTYPE(lv_getColor);
REXX_METHOD_PROTOTYPE(lv_getColumnCount);
REXX_METHOD_PROTOTYPE(lv_getColumnInfo);
REXX_METHOD_PROTOTYPE(lv_getColumnOrder);
REXX_METHOD_PROTOTYPE(lv_getColumnText);
REXX_METHOD_PROTOTYPE(lv_getExtendedStyle);
REXX_METHOD_PROTOTYPE(lv_getFullRow);
REXX_METHOD_PROTOTYPE(lv_getImageList);
REXX_METHOD_PROTOTYPE(lv_getItem);
REXX_METHOD_PROTOTYPE(lv_getItemData);
REXX_METHOD_PROTOTYPE(lv_getItemInfo);
REXX_METHOD_PROTOTYPE(lv_getItemPos);
REXX_METHOD_PROTOTYPE(lv_getItemRect);
REXX_METHOD_PROTOTYPE(lv_getNextItem);
REXX_METHOD_PROTOTYPE(lv_getNextItemWithState);
REXX_METHOD_PROTOTYPE(lv_getSubitem);
REXX_METHOD_PROTOTYPE(lv_getSubitemRect);
REXX_METHOD_PROTOTYPE(lv_getView);
REXX_METHOD_PROTOTYPE(lv_hasCheckBoxes);
REXX_METHOD_PROTOTYPE(lv_hitTestInfo);
REXX_METHOD_PROTOTYPE(lv_insert);
REXX_METHOD_PROTOTYPE(lv_insertColumnPx);
REXX_METHOD_PROTOTYPE(lv_insertFullRow);
REXX_METHOD_PROTOTYPE(lv_isChecked);
REXX_METHOD_PROTOTYPE(lv_itemState);
REXX_METHOD_PROTOTYPE(lv_itemText);
REXX_METHOD_PROTOTYPE(lv_modify);
REXX_METHOD_PROTOTYPE(lv_modifyColumnPx);
REXX_METHOD_PROTOTYPE(lv_modifyFullRow);
REXX_METHOD_PROTOTYPE(lv_modifyItem);
REXX_METHOD_PROTOTYPE(lv_modifySubitem);
REXX_METHOD_PROTOTYPE(lv_prependFullRow);
REXX_METHOD_PROTOTYPE(lv_removeItemData);
REXX_METHOD_PROTOTYPE(lv_replaceExtendStyle);
REXX_METHOD_PROTOTYPE(lv_replaceStyle);
REXX_METHOD_PROTOTYPE(lv_setColor);
REXX_METHOD_PROTOTYPE(lv_setColumnOrder);
REXX_METHOD_PROTOTYPE(lv_setColumnWidthPx);
REXX_METHOD_PROTOTYPE(lv_setFullRowText);
REXX_METHOD_PROTOTYPE(lv_setImageList);
REXX_METHOD_PROTOTYPE(lv_setItemData);
REXX_METHOD_PROTOTYPE(lv_setItemPos);
REXX_METHOD_PROTOTYPE(lv_setItemState);
REXX_METHOD_PROTOTYPE(lv_setItemText);
REXX_METHOD_PROTOTYPE(lv_setSpecificState);
REXX_METHOD_PROTOTYPE(lv_setView);
REXX_METHOD_PROTOTYPE(lv_sortItems);
REXX_METHOD_PROTOTYPE(lv_stringWidthPx);
REXX_METHOD_PROTOTYPE(lv_zTest);

// LvItem
REXX_METHOD_PROTOTYPE(lvi_init_cls            );
REXX_METHOD_PROTOTYPE(lvi_init                );
REXX_METHOD_PROTOTYPE(lvi_unInit              );
REXX_METHOD_PROTOTYPE(lvi_columnFormats       );
REXX_METHOD_PROTOTYPE(lvi_setColumnFormats    );
REXX_METHOD_PROTOTYPE(lvi_columns             );
REXX_METHOD_PROTOTYPE(lvi_setColumns          );
REXX_METHOD_PROTOTYPE(lvi_groupID             );
REXX_METHOD_PROTOTYPE(lvi_setGroupID          );
REXX_METHOD_PROTOTYPE(lvi_imageIndex          );
REXX_METHOD_PROTOTYPE(lvi_setImageIndex       );
REXX_METHOD_PROTOTYPE(lvi_indent              );
REXX_METHOD_PROTOTYPE(lvi_setIndent           );
REXX_METHOD_PROTOTYPE(lvi_index               );
REXX_METHOD_PROTOTYPE(lvi_setIndex            );
REXX_METHOD_PROTOTYPE(lvi_itemState           );
REXX_METHOD_PROTOTYPE(lvi_setItemState        );
REXX_METHOD_PROTOTYPE(lvi_itemStateMask       );
REXX_METHOD_PROTOTYPE(lvi_setItemStateMask    );
REXX_METHOD_PROTOTYPE(lvi_mask                );
REXX_METHOD_PROTOTYPE(lvi_setMask             );
REXX_METHOD_PROTOTYPE(lvi_overlayImageIndex   );
REXX_METHOD_PROTOTYPE(lvi_setOverlayImageIndex);
REXX_METHOD_PROTOTYPE(lvi_stateImageIndex     );
REXX_METHOD_PROTOTYPE(lvi_itemData            );
REXX_METHOD_PROTOTYPE(lvi_setItemData         );
REXX_METHOD_PROTOTYPE(lvi_setStateImageIndex  );
REXX_METHOD_PROTOTYPE(lvi_text                );
REXX_METHOD_PROTOTYPE(lvi_setText             );

// LvSubItem
REXX_METHOD_PROTOTYPE(lvsi_init_cls       );
REXX_METHOD_PROTOTYPE(lvsi_init           );
REXX_METHOD_PROTOTYPE(lvsi_unInit         );
REXX_METHOD_PROTOTYPE(lvsi_item           );
REXX_METHOD_PROTOTYPE(lvsi_setItem        );
REXX_METHOD_PROTOTYPE(lvsi_subItem        );
REXX_METHOD_PROTOTYPE(lvsi_setSubItem     );
REXX_METHOD_PROTOTYPE(lvsi_mask           );
REXX_METHOD_PROTOTYPE(lvsi_setMask        );
REXX_METHOD_PROTOTYPE(lvsi_text           );
REXX_METHOD_PROTOTYPE(lvsi_setText        );
REXX_METHOD_PROTOTYPE(lvsi_imageIndex     );
REXX_METHOD_PROTOTYPE(lvsi_setImageIndex  );

// LvFullRow
REXX_METHOD_PROTOTYPE(lvfr_init_cls);
REXX_METHOD_PROTOTYPE(lvfr_fromArray_cls);
REXX_METHOD_PROTOTYPE(lvfr_init);
REXX_METHOD_PROTOTYPE(lvfr_unInit);
REXX_METHOD_PROTOTYPE(lvfr_userData);
REXX_METHOD_PROTOTYPE(lvfr_setUserData);
REXX_METHOD_PROTOTYPE(lvfr_userData);
REXX_METHOD_PROTOTYPE(lvfr_setUserData);
REXX_METHOD_PROTOTYPE(lvfr_addSubitem);
REXX_METHOD_PROTOTYPE(lvfr_insertSubitem);
REXX_METHOD_PROTOTYPE(lvfr_item);
REXX_METHOD_PROTOTYPE(lvfr_removeSubitem);
REXX_METHOD_PROTOTYPE(lvfr_subitem);
REXX_METHOD_PROTOTYPE(lvfr_subitems);

// ToolTip
REXX_METHOD_PROTOTYPE(tt_activate);
REXX_METHOD_PROTOTYPE(tt_addTool);
REXX_METHOD_PROTOTYPE(tt_addToolEx);
REXX_METHOD_PROTOTYPE(tt_addToolRect);
REXX_METHOD_PROTOTYPE(tt_adjustRect);
REXX_METHOD_PROTOTYPE(tt_delTool);
REXX_METHOD_PROTOTYPE(tt_enumTools);
REXX_METHOD_PROTOTYPE(tt_getBubbleSize);
REXX_METHOD_PROTOTYPE(tt_getCurrentTool);
REXX_METHOD_PROTOTYPE(tt_getDelayTime);
REXX_METHOD_PROTOTYPE(tt_getMargin);
REXX_METHOD_PROTOTYPE(tt_getMaxTipWidth);
REXX_METHOD_PROTOTYPE(tt_getText);
REXX_METHOD_PROTOTYPE(tt_getTipBkColor);
REXX_METHOD_PROTOTYPE(tt_getTipTextColor);
REXX_METHOD_PROTOTYPE(tt_getTitle);
REXX_METHOD_PROTOTYPE(tt_getToolCount);
REXX_METHOD_PROTOTYPE(tt_getToolInfo);
REXX_METHOD_PROTOTYPE(tt_hasCurrentTool);
REXX_METHOD_PROTOTYPE(tt_hitTestInfo);
REXX_METHOD_PROTOTYPE(tt_manageAtypicalTool);
REXX_METHOD_PROTOTYPE(tt_newToolRect);
REXX_METHOD_PROTOTYPE(tt_pop);
REXX_METHOD_PROTOTYPE(tt_popUp);
REXX_METHOD_PROTOTYPE(tt_setDelayTime);
REXX_METHOD_PROTOTYPE(tt_setMargin);
REXX_METHOD_PROTOTYPE(tt_setMaxTipWidth);
REXX_METHOD_PROTOTYPE(tt_setTipBkColor);
REXX_METHOD_PROTOTYPE(tt_setTipTextColor);
REXX_METHOD_PROTOTYPE(tt_setTitle);
REXX_METHOD_PROTOTYPE(tt_setToolInfo);
REXX_METHOD_PROTOTYPE(tt_setWindowTheme);
REXX_METHOD_PROTOTYPE(tt_trackActivate);
REXX_METHOD_PROTOTYPE(tt_trackPosition);
REXX_METHOD_PROTOTYPE(tt_update);
REXX_METHOD_PROTOTYPE(tt_updateTipText);

// ToolInfo
REXX_METHOD_PROTOTYPE(ti_forHitTest_cls);
REXX_METHOD_PROTOTYPE(ti_forID_cls);
REXX_METHOD_PROTOTYPE(ti_init);
REXX_METHOD_PROTOTYPE(ti_unInit);
REXX_METHOD_PROTOTYPE(ti_flags);
REXX_METHOD_PROTOTYPE(ti_setFlags);
REXX_METHOD_PROTOTYPE(ti_rexxHwnd);
REXX_METHOD_PROTOTYPE(ti_rexxID);
REXX_METHOD_PROTOTYPE(ti_rect);
REXX_METHOD_PROTOTYPE(ti_setRect);
REXX_METHOD_PROTOTYPE(ti_resource);
REXX_METHOD_PROTOTYPE(ti_text);
REXX_METHOD_PROTOTYPE(ti_setText);
REXX_METHOD_PROTOTYPE(ti_userData);
REXX_METHOD_PROTOTYPE(ti_setUserData);

REXX_METHOD_PROTOTYPE(ti_setTextMemoryIsAllocated);
REXX_METHOD_PROTOTYPE(ti_hitTestHelper);

// TreeView
REXX_METHOD_PROTOTYPE(tv_delete);
REXX_METHOD_PROTOTYPE(tv_deleteAll);
REXX_METHOD_PROTOTYPE(tv_expand);
REXX_METHOD_PROTOTYPE(tv_find);
REXX_METHOD_PROTOTYPE(tv_getImageList);
REXX_METHOD_PROTOTYPE(tv_getItemData);
REXX_METHOD_PROTOTYPE(tv_getItemHeight);
REXX_METHOD_PROTOTYPE(tv_getItemRect);
REXX_METHOD_PROTOTYPE(tv_getNextItem);
REXX_METHOD_PROTOTYPE(tv_getSpecificItem);
REXX_METHOD_PROTOTYPE(tv_getStateImage);
REXX_METHOD_PROTOTYPE(tv_hitTestInfo);
REXX_METHOD_PROTOTYPE(tv_insert);
REXX_METHOD_PROTOTYPE(tv_itemInfo);
REXX_METHOD_PROTOTYPE(tv_itemText);
REXX_METHOD_PROTOTYPE(tv_modify);
REXX_METHOD_PROTOTYPE(tv_removeItemData);
REXX_METHOD_PROTOTYPE(tv_selectItem);
REXX_METHOD_PROTOTYPE(tv_setImageList);
REXX_METHOD_PROTOTYPE(tv_setItemData);
REXX_METHOD_PROTOTYPE(tv_setItemHeight);
REXX_METHOD_PROTOTYPE(tv_setStateImage);
REXX_METHOD_PROTOTYPE(tv_sortChildrenCB);
REXX_METHOD_PROTOTYPE(tv_test);

// Tab
REXX_METHOD_PROTOTYPE(tab_select);
REXX_METHOD_PROTOTYPE(tab_selected);
REXX_METHOD_PROTOTYPE(tab_insert);
REXX_METHOD_PROTOTYPE(tv_modify);
REXX_METHOD_PROTOTYPE(tab_addSequence);
REXX_METHOD_PROTOTYPE(tab_addFullSeq);
REXX_METHOD_PROTOTYPE(tab_modify);
REXX_METHOD_PROTOTYPE(tab_itemInfo);
REXX_METHOD_PROTOTYPE(tab_setItemSize);
REXX_METHOD_PROTOTYPE(tab_setPadding);
REXX_METHOD_PROTOTYPE(tab_getItemRect);
REXX_METHOD_PROTOTYPE(tab_calcRect);
REXX_METHOD_PROTOTYPE(tab_setImageList);
REXX_METHOD_PROTOTYPE(tab_getImageList);

// DateTimePicker
REXX_METHOD_PROTOTYPE(dtp_getDateTime);
REXX_METHOD_PROTOTYPE(dtp_closeMonthCal);
REXX_METHOD_PROTOTYPE(dtp_getInfo);
REXX_METHOD_PROTOTYPE(dtp_getIdealSize);
REXX_METHOD_PROTOTYPE(dtp_getMonthCal);
REXX_METHOD_PROTOTYPE(dtp_getMonthCalColor);
REXX_METHOD_PROTOTYPE(dtp_getMonthCalStyle);
REXX_METHOD_PROTOTYPE(dtp_getRange);
REXX_METHOD_PROTOTYPE(dtp_setDateTime);
REXX_METHOD_PROTOTYPE(dtp_setFormat);
REXX_METHOD_PROTOTYPE(dtp_setMonthCalStyle);
REXX_METHOD_PROTOTYPE(dtp_setMonthCalColor);
REXX_METHOD_PROTOTYPE(dtp_setRange);

// MonthCalendar
REXX_METHOD_PROTOTYPE(get_mc_date);
REXX_METHOD_PROTOTYPE(set_mc_date);
REXX_METHOD_PROTOTYPE(mc_addRemoveStyle);
REXX_METHOD_PROTOTYPE(mc_replaceStyle);
REXX_METHOD_PROTOTYPE(mc_getStyle);
REXX_METHOD_PROTOTYPE(mc_getCalendarBorder);
REXX_METHOD_PROTOTYPE(mc_getCalendarCount);
REXX_METHOD_PROTOTYPE(mc_getCALID);
REXX_METHOD_PROTOTYPE(mc_getColor);
REXX_METHOD_PROTOTYPE(mc_getCurrentView);
REXX_METHOD_PROTOTYPE(mc_getFirstDayOfWeek);
REXX_METHOD_PROTOTYPE(mc_getGridInfo);
REXX_METHOD_PROTOTYPE(mc_getMinRect);
REXX_METHOD_PROTOTYPE(mc_getMonthRange);
REXX_METHOD_PROTOTYPE(mc_getRange);
REXX_METHOD_PROTOTYPE(mc_getSelectionRange);
REXX_METHOD_PROTOTYPE(mc_getToday);
REXX_METHOD_PROTOTYPE(mc_hitTestInfo);
REXX_METHOD_PROTOTYPE(mc_setCalendarBorder);
REXX_METHOD_PROTOTYPE(mc_setCALID);
REXX_METHOD_PROTOTYPE(mc_setColor);
REXX_METHOD_PROTOTYPE(mc_setCurrentView);
REXX_METHOD_PROTOTYPE(mc_setDayState);
REXX_METHOD_PROTOTYPE(mc_setDayStateQuick);
REXX_METHOD_PROTOTYPE(mc_setFirstDayOfWeek);
REXX_METHOD_PROTOTYPE(mc_setRange);
REXX_METHOD_PROTOTYPE(mc_setSelectionRange);
REXX_METHOD_PROTOTYPE(mc_setToday);
REXX_METHOD_PROTOTYPE(mc_sizeRectToMin);

// .DayStates
REXX_METHOD_PROTOTYPE(dss_makeDayStateBuffer);
REXX_METHOD_PROTOTYPE(dss_quickDayStateBuffer);

// .DayState
REXX_METHOD_PROTOTYPE(ds_init);
REXX_METHOD_PROTOTYPE(ds_value);


// .VK
REXX_METHOD_PROTOTYPE(vk_key2name);

// Menu classes methods
REXX_METHOD_PROTOTYPE(menu_menuInit_pvt);
REXX_METHOD_PROTOTYPE(menu_uninit);
REXX_METHOD_PROTOTYPE(menu_connectCommandEvent_cls);
REXX_METHOD_PROTOTYPE(menu_getHMenu);
REXX_METHOD_PROTOTYPE(menu_wID);
REXX_METHOD_PROTOTYPE(menu_isValidItemID);
REXX_METHOD_PROTOTYPE(menu_isValidMenu);
REXX_METHOD_PROTOTYPE(menu_isValidMenuHandle);
REXX_METHOD_PROTOTYPE(menu_isSeparator);
REXX_METHOD_PROTOTYPE(menu_isCommandItem);
REXX_METHOD_PROTOTYPE(menu_isPopup);
REXX_METHOD_PROTOTYPE(menu_getMenuHandle);
REXX_METHOD_PROTOTYPE(menu_releaseMenuHandle);
REXX_METHOD_PROTOTYPE(menu_destroy);
REXX_METHOD_PROTOTYPE(menu_insertSeparator);
REXX_METHOD_PROTOTYPE(menu_removeSeparator);
REXX_METHOD_PROTOTYPE(menu_insertItem);
REXX_METHOD_PROTOTYPE(menu_removeItem);
REXX_METHOD_PROTOTYPE(menu_insertPopup);
REXX_METHOD_PROTOTYPE(menu_removePopup);
REXX_METHOD_PROTOTYPE(menu_deletePopup);
REXX_METHOD_PROTOTYPE(menu_getPopup);
REXX_METHOD_PROTOTYPE(menu_isEnabled);
REXX_METHOD_PROTOTYPE(menu_isDisabled);
REXX_METHOD_PROTOTYPE(menu_isChecked);
REXX_METHOD_PROTOTYPE(menu_check);
REXX_METHOD_PROTOTYPE(menu_unCheck);
REXX_METHOD_PROTOTYPE(menu_checkRadio);
REXX_METHOD_PROTOTYPE(menu_hilite);
REXX_METHOD_PROTOTYPE(menu_unHilite);
REXX_METHOD_PROTOTYPE(menu_getCount);
REXX_METHOD_PROTOTYPE(menu_enable);
REXX_METHOD_PROTOTYPE(menu_disable);
REXX_METHOD_PROTOTYPE(menu_getItemState);
REXX_METHOD_PROTOTYPE(menu_getItemType);
REXX_METHOD_PROTOTYPE(menu_setID);
REXX_METHOD_PROTOTYPE(menu_getID);
REXX_METHOD_PROTOTYPE(menu_getHelpID);
REXX_METHOD_PROTOTYPE(menu_setHelpID);
REXX_METHOD_PROTOTYPE(menu_getMaxHeight);
REXX_METHOD_PROTOTYPE(menu_setMaxHeight);
REXX_METHOD_PROTOTYPE(menu_setText);
REXX_METHOD_PROTOTYPE(menu_getText);
REXX_METHOD_PROTOTYPE(menu_setAutoConnection);
REXX_METHOD_PROTOTYPE(menu_getAutoConnectStatus);
REXX_METHOD_PROTOTYPE(menu_connectMenuEvent);
REXX_METHOD_PROTOTYPE(menu_connectCommandEvent);
REXX_METHOD_PROTOTYPE(menu_connectAllCommandEvents);
REXX_METHOD_PROTOTYPE(menu_connectSomeCommandEvents);
REXX_METHOD_PROTOTYPE(menu_itemTextToMethodName);
REXX_METHOD_PROTOTYPE(menu_test);

REXX_METHOD_PROTOTYPE(menuBar_attachTo);
REXX_METHOD_PROTOTYPE(menuBar_detach);
REXX_METHOD_PROTOTYPE(menuBar_isAttached);
REXX_METHOD_PROTOTYPE(menuBar_redraw);
REXX_METHOD_PROTOTYPE(menuBar_replace);

REXX_METHOD_PROTOTYPE(binMenu_init);

REXX_METHOD_PROTOTYPE(sysMenu_init);
REXX_METHOD_PROTOTYPE(sysMenu_revert);

REXX_METHOD_PROTOTYPE(popMenu_connectContextMenu_cls);
REXX_METHOD_PROTOTYPE(popMenu_init);
REXX_METHOD_PROTOTYPE(popMenu_connectContextMenu);
REXX_METHOD_PROTOTYPE(popMenu_isAssigned);
REXX_METHOD_PROTOTYPE(popMenu_assignTo);
REXX_METHOD_PROTOTYPE(popMenu_track);
REXX_METHOD_PROTOTYPE(popMenu_show);

REXX_METHOD_PROTOTYPE(scriptMenu_init);

REXX_METHOD_PROTOTYPE(userMenu_init);
REXX_METHOD_PROTOTYPE(userMenu_complete);

REXX_METHOD_PROTOTYPE(menuTemplate_isComplete);
REXX_METHOD_PROTOTYPE(menuTemplate_addSeparator);
REXX_METHOD_PROTOTYPE(menuTemplate_addItem);
REXX_METHOD_PROTOTYPE(menuTemplate_addPopup);

// Mouse
REXX_METHOD_PROTOTYPE(mouse_new_cls);
REXX_METHOD_PROTOTYPE(mouse_doubleClickTime_cls);
REXX_METHOD_PROTOTYPE(mouse_loadCursor_cls);
REXX_METHOD_PROTOTYPE(mouse_loadCursorFromFile_cls);
REXX_METHOD_PROTOTYPE(mouse_setDoubleClickTime_cls);
REXX_METHOD_PROTOTYPE(mouse_swapButton_cls);
REXX_METHOD_PROTOTYPE(mouse_init);
REXX_METHOD_PROTOTYPE(mouse_capture);
REXX_METHOD_PROTOTYPE(mouse_clipCursor);
REXX_METHOD_PROTOTYPE(mouse_connectEvent);
REXX_METHOD_PROTOTYPE(mouse_dragDetect);
REXX_METHOD_PROTOTYPE(mouse_getClipCursor);
REXX_METHOD_PROTOTYPE(mouse_getCursorPos);
REXX_METHOD_PROTOTYPE(mouse_get_release_capture);
REXX_METHOD_PROTOTYPE(mouse_isButtonDown);
REXX_METHOD_PROTOTYPE(mouse_releaseClipCursor);
REXX_METHOD_PROTOTYPE(mouse_restoreCursor);
REXX_METHOD_PROTOTYPE(mouse_setCursor);
REXX_METHOD_PROTOTYPE(mouse_setCursorPos);
REXX_METHOD_PROTOTYPE(mouse_showCursor);
REXX_METHOD_PROTOTYPE(mouse_trackEvent);
REXX_METHOD_PROTOTYPE(mouse_test);

//Keyboard
REXX_METHOD_PROTOTYPE(kb_getAsyncKeyState_cls);


// CreateWindows
REXX_METHOD_PROTOTYPE(cw_initCreateWindows);
REXX_METHOD_PROTOTYPE(cw_createReBarWindow);
REXX_METHOD_PROTOTYPE(cw_createStatusBarWindow);
REXX_METHOD_PROTOTYPE(cw_createToolBarWindow);

// CustomDraw
REXX_METHOD_PROTOTYPE(cd_init);
REXX_METHOD_PROTOTYPE(cd_customDrawControl);

// LvCustomDrawSimple
REXX_METHOD_PROTOTYPE(lvcds_init_cls    );
REXX_METHOD_PROTOTYPE(lvcds_init        );
REXX_METHOD_PROTOTYPE(lvcds_setClrText  );
REXX_METHOD_PROTOTYPE(lvcds_setClrTextBk);
REXX_METHOD_PROTOTYPE(lvcds_getDrawStage);
REXX_METHOD_PROTOTYPE(lvcds_setFont     );
REXX_METHOD_PROTOTYPE(lvcds_getID       );
REXX_METHOD_PROTOTYPE(lvcds_getItem     );
REXX_METHOD_PROTOTYPE(lvcds_getItemData );
REXX_METHOD_PROTOTYPE(lvcds_setReply    );
REXX_METHOD_PROTOTYPE(lvcds_getSubItem  );

// TvCustomDrawSimple
REXX_METHOD_PROTOTYPE(tvcds_init_cls    );
REXX_METHOD_PROTOTYPE(tvcds_init        );
REXX_METHOD_PROTOTYPE(tvcds_setClrText  );
REXX_METHOD_PROTOTYPE(tvcds_setClrTextBk);
REXX_METHOD_PROTOTYPE(tvcds_getDrawStage);
REXX_METHOD_PROTOTYPE(tvcds_setFont     );
REXX_METHOD_PROTOTYPE(tvcds_getID       );
REXX_METHOD_PROTOTYPE(tvcds_getItem     );
REXX_METHOD_PROTOTYPE(tvcds_getItemData );
REXX_METHOD_PROTOTYPE(tvcds_setReply    );
REXX_METHOD_PROTOTYPE(tvcds_getLevel    );

// BrowseForFolder
REXX_METHOD_PROTOTYPE(bff_banner);
REXX_METHOD_PROTOTYPE(bff_setBanner);
REXX_METHOD_PROTOTYPE(bff_dlgTitle);
REXX_METHOD_PROTOTYPE(bff_setDlgTitle);
REXX_METHOD_PROTOTYPE(bff_hint);
REXX_METHOD_PROTOTYPE(bff_setHint);
REXX_METHOD_PROTOTYPE(bff_initialThread);
REXX_METHOD_PROTOTYPE(bff_options);
REXX_METHOD_PROTOTYPE(bff_setOptions);
REXX_METHOD_PROTOTYPE(bff_owner);
REXX_METHOD_PROTOTYPE(bff_setOwner);
REXX_METHOD_PROTOTYPE(bff_root);
REXX_METHOD_PROTOTYPE(bff_setRoot);
REXX_METHOD_PROTOTYPE(bff_startDir);
REXX_METHOD_PROTOTYPE(bff_setStartDir);
REXX_METHOD_PROTOTYPE(bff_usePathForHint);
REXX_METHOD_PROTOTYPE(bff_setUsePathForHint);
REXX_METHOD_PROTOTYPE(bff_init);
REXX_METHOD_PROTOTYPE(bff_uninit);
REXX_METHOD_PROTOTYPE(bff_getDisplayName);
REXX_METHOD_PROTOTYPE(bff_getFolder);
REXX_METHOD_PROTOTYPE(bff_getItemIDList);
REXX_METHOD_PROTOTYPE(bff_initCOM);
REXX_METHOD_PROTOTYPE(bff_releaseCOM);
REXX_METHOD_PROTOTYPE(bff_releaseItemIDList);
REXX_METHOD_PROTOTYPE(bff_test);

REXX_METHOD_PROTOTYPE(sfb_getFolder);

// CommonItemDialog
REXX_METHOD_PROTOTYPE(cid_options);
REXX_METHOD_PROTOTYPE(cid_setOptions);

REXX_METHOD_PROTOTYPE(cid_uninit);
REXX_METHOD_PROTOTYPE(cid_init);

REXX_METHOD_PROTOTYPE(cid_addPlace);
REXX_METHOD_PROTOTYPE(cid_advise);
REXX_METHOD_PROTOTYPE(cid_clearClientData);
REXX_METHOD_PROTOTYPE(cid_close);
REXX_METHOD_PROTOTYPE(cid_getCurrentSelection);
REXX_METHOD_PROTOTYPE(cid_getFileName);
REXX_METHOD_PROTOTYPE(cid_getFileTypeIndex);
REXX_METHOD_PROTOTYPE(cid_getFolder);
REXX_METHOD_PROTOTYPE(cid_getResult);
REXX_METHOD_PROTOTYPE(cid_initCOM);
REXX_METHOD_PROTOTYPE(cid_isReleased);
REXX_METHOD_PROTOTYPE(cid_release);
REXX_METHOD_PROTOTYPE(cid_releaseCOM);
REXX_METHOD_PROTOTYPE(cid_setClientGuid);
REXX_METHOD_PROTOTYPE(cid_setDefaultExtension);
REXX_METHOD_PROTOTYPE(cid_setDefaultFolder);
REXX_METHOD_PROTOTYPE(cid_setFileName);
REXX_METHOD_PROTOTYPE(cid_setFileNameLabel);
REXX_METHOD_PROTOTYPE(cid_setFileTypeIndex);
REXX_METHOD_PROTOTYPE(cid_setFileTypes);
REXX_METHOD_PROTOTYPE(cid_setFilter);
REXX_METHOD_PROTOTYPE(cid_setFolder);
REXX_METHOD_PROTOTYPE(cid_setCancelButtonLabel);
REXX_METHOD_PROTOTYPE(cid_setOkButtonLabel);
REXX_METHOD_PROTOTYPE(cid_setTitle);
REXX_METHOD_PROTOTYPE(cid_show);
REXX_METHOD_PROTOTYPE(cid_unadvise);

// OpenFileDialog
REXX_METHOD_PROTOTYPE(ofd_init);
REXX_METHOD_PROTOTYPE(ofd_getResults);

// SaveFileDialog
REXX_METHOD_PROTOTYPE(sfd_init);
REXX_METHOD_PROTOTYPE(sfd_setSaveAsItem);

// CommonDialogCustomizations
REXX_METHOD_PROTOTYPE(cid_initCustomizations);
REXX_METHOD_PROTOTYPE(cid_addCheckButton);
REXX_METHOD_PROTOTYPE(cid_addComboBox);
REXX_METHOD_PROTOTYPE(cid_addControlItem);
REXX_METHOD_PROTOTYPE(cid_addEditBox);
REXX_METHOD_PROTOTYPE(cid_addMenu);
REXX_METHOD_PROTOTYPE(cid_addPushButton);
REXX_METHOD_PROTOTYPE(cid_addRadioButtonList);
REXX_METHOD_PROTOTYPE(cid_addSeparator);
REXX_METHOD_PROTOTYPE(cid_addText);
REXX_METHOD_PROTOTYPE(cid_enableOpenDropDown);
REXX_METHOD_PROTOTYPE(cid_endVisualGroup);
REXX_METHOD_PROTOTYPE(cid_getCheckButtonState);
REXX_METHOD_PROTOTYPE(cid_getControlItemState);
REXX_METHOD_PROTOTYPE(cid_getControlState);
REXX_METHOD_PROTOTYPE(cid_getEditBoxText);
REXX_METHOD_PROTOTYPE(cid_getSelectedControlItem);
REXX_METHOD_PROTOTYPE(cid_makeProminent);
REXX_METHOD_PROTOTYPE(cid_removeAllControlItems);
REXX_METHOD_PROTOTYPE(cid_removeControlItem);
REXX_METHOD_PROTOTYPE(cid_setCheckButtonState);
REXX_METHOD_PROTOTYPE(cid_setControlItemState);
REXX_METHOD_PROTOTYPE(cid_setControlItemText);
REXX_METHOD_PROTOTYPE(cid_setControlLabel);
REXX_METHOD_PROTOTYPE(cid_setControlState);
REXX_METHOD_PROTOTYPE(cid_setEditBoxText);
REXX_METHOD_PROTOTYPE(cid_setSelectedControlItem);
REXX_METHOD_PROTOTYPE(cid_startVisualGroup);

// CommonDialgoEvents
REXX_METHOD_PROTOTYPE(cde_init);

// ShellItemFilter
REXX_METHOD_PROTOTYPE(sif_init);


RexxMethodEntry oodialog_methods[] = {
    REXX_METHOD(dlgutil_init_cls,               dlgutil_init_cls),
    REXX_METHOD(dlgutil_comctl32Version_cls,    dlgutil_comctl32Version_cls),
    REXX_METHOD(dlgutil_getGuid_cls,            dlgutil_getGuid_cls),
    REXX_METHOD(dlgutil_version_cls,            dlgutil_version_cls),
    REXX_METHOD(dlgutil_errMsg_cls,             dlgutil_errMsg_cls),
    REXX_METHOD(dlgutil_hiWord_cls,             dlgutil_hiWord_cls),
    REXX_METHOD(dlgutil_loWord_cls,             dlgutil_loWord_cls),
    REXX_METHOD(dlgutil_sloWord_cls,            dlgutil_sloWord_cls),
    REXX_METHOD(dlgutil_shiWord_cls,            dlgutil_shiWord_cls),
    REXX_METHOD(dlgutil_makeLPARAM_cls,         dlgutil_makeLPARAM_cls),
    REXX_METHOD(dlgutil_makeWPARAM_cls,         dlgutil_makeWPARAM_cls),
    REXX_METHOD(dlgutil_unsigned_cls,           dlgutil_unsigned_cls),
    REXX_METHOD(dlgutil_unsigned32_cls,         dlgutil_unsigned32_cls),
    REXX_METHOD(dlgutil_signed_cls,             dlgutil_signed_cls),
    REXX_METHOD(dlgutil_signed32_cls,           dlgutil_signed32_cls),
    REXX_METHOD(dlgutil_and_cls,                dlgutil_and_cls),
    REXX_METHOD(dlgutil_or_cls,                 dlgutil_or_cls),
    REXX_METHOD(dlgutil_shiftLeft_cls,          dlgutil_shiftLeft_cls),
    REXX_METHOD(dlgutil_shiftRight_cls,         dlgutil_shiftRight_cls),
    REXX_METHOD(dlgutil_sShiftLeft_cls,         dlgutil_sShiftLeft_cls),
    REXX_METHOD(dlgutil_sShiftRight_cls,        dlgutil_sShiftRight_cls),
    REXX_METHOD(dlgutil_screenSize_cls,         dlgutil_screenSize_cls),
    REXX_METHOD(dlgutil_screenArea_cls,         dlgutil_screenArea_cls),
    REXX_METHOD(dlgutil_getSystemMetrics_cls,   dlgutil_getSystemMetrics_cls),
    REXX_METHOD(dlgutil_halt_cls,               dlgutil_halt_cls),
    REXX_METHOD(dlgutil_handleToPointer_cls,    dlgutil_handleToPointer_cls),
    REXX_METHOD(dlgutil_terminate_cls,          dlgutil_terminate_cls),
    REXX_METHOD(dlgutil_threadID_cls,           dlgutil_threadID_cls),
    REXX_METHOD(dlgutil_windowFromPoint_cls,    dlgutil_windowFromPoint_cls),
    REXX_METHOD(dlgutil_test_cls,               dlgutil_test_cls),

    REXX_METHOD(app_init,                       app_init),
    REXX_METHOD(app_srcDir_atr,                 app_srcDir_atr),
    REXX_METHOD(app_addToConstDir,              app_addToConstDir),
    REXX_METHOD(app_autoDetection,              app_autoDetection),
    REXX_METHOD(app_defaultFont,                app_defaultFont),
    REXX_METHOD(app_defaultIcon,                app_defaultIcon),
    REXX_METHOD(app_requiredOS,                 app_requiredOS),
    REXX_METHOD(app_setDefaults,                app_setDefaults),
    REXX_METHOD(app_initAutoDetection,          app_initAutoDetection),
    REXX_METHOD(app_useGlobalConstDir,          app_useGlobalConstDir),

    // OS
    REXX_METHOD(os_is64bit,                     os_is64bit),
    REXX_METHOD(os_is32on64bit,                 os_is32on64bit),
    REXX_METHOD(os_isVersion,                   os_isVersion),
    REXX_METHOD(os_settingChanged,              os_settingChanged),
    REXX_METHOD(os_shellChangeNotify,           os_shellChangeNotify),

    // SPI
    REXX_METHOD(spi_init_cls,                   spi_init_cls),
    REXX_METHOD(spi_getDragHeight_cls,          spi_getDragHeight_cls),
    REXX_METHOD(spi_setDragHeight_cls,          spi_setDragHeight_cls),
    REXX_METHOD(spi_getDragWidth_cls,           spi_getDragWidth_cls),
    REXX_METHOD(spi_setDragWidth_cls,           spi_setDragWidth_cls),
    REXX_METHOD(spi_getMenuAnimation_cls,       spi_getMenuAnimation_cls),
    REXX_METHOD(spi_setMenuAnimation_cls,       spi_setMenuAnimation_cls),
    REXX_METHOD(spi_getMenuFade_cls,            spi_getMenuFade_cls),
    REXX_METHOD(spi_setMenuFade_cls,            spi_setMenuFade_cls),
    REXX_METHOD(spi_getMouseHoverHeight_cls,    spi_getMouseHoverHeight_cls),
    REXX_METHOD(spi_setMouseHoverHeight_cls,    spi_setMouseHoverHeight_cls),
    REXX_METHOD(spi_getMouseHoverTime_cls,      spi_getMouseHoverTime_cls),
    REXX_METHOD(spi_setMouseHoverTime_cls,      spi_setMouseHoverTime_cls),
    REXX_METHOD(spi_getMouseHoverWidth_cls,     spi_getMouseHoverWidth_cls),
    REXX_METHOD(spi_setMouseHoverWidth_cls,     spi_setMouseHoverWidth_cls),
    REXX_METHOD(spi_getNonClientMetrics_cls,    spi_getNonClientMetrics_cls),
    REXX_METHOD(spi_setNonClientMetrics_cls,    spi_setNonClientMetrics_cls),
    REXX_METHOD(spi_getUpdateFlag_cls,          spi_getUpdateFlag_cls),
    REXX_METHOD(spi_setUpdateFlag_cls,          spi_setUpdateFlag_cls),
    REXX_METHOD(spi_getWorkArea_cls,            spi_getWorkArea_cls),
    REXX_METHOD(spi_setWorkArea_cls,            spi_setWorkArea_cls),
    REXX_METHOD(spi_getWheelScrollLines_cls,    spi_getWheelScrollLines_cls),
    REXX_METHOD(spi_setWheelScrollLines_cls,    spi_setWheelScrollLines_cls),

    // SM
    REXX_METHOD(sm_cMouseButtons_cls,           sm_cMouseButtons_cls),
    REXX_METHOD(sm_cxCursor_cls,                sm_cxCursor_cls),
    REXX_METHOD(sm_cxDrag_cls,                  sm_cxDrag_cls),
    REXX_METHOD(sm_cxFixedFrame_cls,            sm_cxFixedFrame_cls),
    REXX_METHOD(sm_cxIcon_cls,                  sm_cxIcon_cls),
    REXX_METHOD(sm_cxScreen_cls,                sm_cxScreen_cls),
    REXX_METHOD(sm_cxSize_cls,                  sm_cxSize_cls),
    REXX_METHOD(sm_cxSmIcon_cls,                sm_cxSmIcon_cls),
    REXX_METHOD(sm_cxVScroll_cls,               sm_cxVScroll_cls),
    REXX_METHOD(sm_cyCaption_cls,               sm_cyCaption_cls),
    REXX_METHOD(sm_cyCursor_cls,                sm_cyCursor_cls),
    REXX_METHOD(sm_cyDrag_cls,                  sm_cyDrag_cls),
    REXX_METHOD(sm_cyFixedFrame_cls,            sm_cyFixedFrame_cls),
    REXX_METHOD(sm_cyHScroll_cls,               sm_cyHScroll_cls),
    REXX_METHOD(sm_cyIcon_cls,                  sm_cyIcon_cls),
    REXX_METHOD(sm_cyMenu_cls,                  sm_cyMenu_cls),
    REXX_METHOD(sm_cyScreen_cls,                sm_cyScreen_cls),
    REXX_METHOD(sm_cySize_cls,                  sm_cySize_cls),
    REXX_METHOD(sm_cySmIcon_cls,                sm_cySmIcon_cls),
    REXX_METHOD(sm_menuDropAlignment_cls,       sm_menuDropAlignment_cls),

    REXX_METHOD(rsrcUtils_resolveIconID_pvt,    rsrcUtils_resolveIconID_pvt),
    REXX_METHOD(rsrcUtils_resolveResourceID,    rsrcUtils_resolveResourceID),
    REXX_METHOD(rsrcUtils_idError,              rsrcUtils_idError),
    REXX_METHOD(rsrcUtils_checkID,              rsrcUtils_checkID),

    REXX_METHOD(wb_init_windowBase,             wb_init_windowBase),
    REXX_METHOD(wb_getHwnd,                     wb_getHwnd),
    REXX_METHOD(wb_getInitCode,                 wb_getInitCode),
    REXX_METHOD(wb_setInitCode,                 wb_setInitCode),
    REXX_METHOD(wb_getFactorX,                  wb_getFactorX),
    REXX_METHOD(wb_setFactorX,                  wb_setFactorX),
    REXX_METHOD(wb_getFactorY,                  wb_getFactorY),
    REXX_METHOD(wb_setFactorY,                  wb_setFactorY),
    REXX_METHOD(wb_getSizeX,                    wb_getSizeX),
    REXX_METHOD(wb_setSizeX,                    wb_setSizeX),
    REXX_METHOD(wb_getSizeY,                    wb_getSizeY),
    REXX_METHOD(wb_setSizeY,                    wb_setSizeY),
    REXX_METHOD(wb_getPixelCX,                  wb_getPixelCX),
    REXX_METHOD(wb_getPixelCY,                  wb_getPixelCY),
    REXX_METHOD(wb_childWindowFromPoint,        wb_childWindowFromPoint),
    REXX_METHOD(wb_clientRect,                  wb_clientRect),
    REXX_METHOD(wb_clear,                       wb_clear),
    REXX_METHOD(wb_display,                     wb_display),
    REXX_METHOD(wb_enable,                      wb_enable),
    REXX_METHOD(wb_foreGroundWindow,            wb_foreGroundWindow),
    REXX_METHOD(wb_getSizePos,                  wb_getSizePos),
    REXX_METHOD(wb_getText,                     wb_getText),
    REXX_METHOD(wb_getTextSizePx,               wb_getTextSizePx),
    REXX_METHOD(wb_getTextSizeScreen,           wb_getTextSizeScreen),
    REXX_METHOD(wb_getWindowLong_pvt,           wb_getWindowLong_pvt),
    REXX_METHOD(wb_isEnabled,                   wb_isEnabled),
    REXX_METHOD(wb_isVisible,                   wb_isVisible),
    REXX_METHOD(wb_mapWindowPoints,             wb_mapWindowPoints),
    REXX_METHOD(wb_moveSizeWindow,              wb_moveSizeWindow),
    REXX_METHOD(wb_redraw,                      wb_redraw),
    REXX_METHOD(wb_redrawClient,                wb_redrawClient),
    REXX_METHOD(wb_resizeMove,                  wb_resizeMove),
    REXX_METHOD(wb_screenClient,                wb_screenClient),
    REXX_METHOD(wb_setRect,                     wb_setRect),
    REXX_METHOD(wb_setText,                     wb_setText),
    REXX_METHOD(wb_sendMessage,                 wb_sendMessage),
    REXX_METHOD(wb_sendWinIntMsg,               wb_sendWinIntMsg),
    REXX_METHOD(wb_sendWinHandleMsg,            wb_sendWinHandleMsg),
    REXX_METHOD(wb_sendWinHandle2Msg,           wb_sendWinHandle2Msg),
    REXX_METHOD(wb_sendWinUintMsg,              wb_sendWinUintMsg),
    REXX_METHOD(wb_setWindowPos,                wb_setWindowPos),
    REXX_METHOD(wb_setWindowLong_pvt,           wb_setWindowLong_pvt),
    REXX_METHOD(wb_show,                        wb_show),
    REXX_METHOD(wb_showFast,                    wb_showFast),
    REXX_METHOD(wb_updateWindow,                wb_updateWindow),
    REXX_METHOD(wb_windowRect,                  wb_windowRect),

    REXX_METHOD(en_init_eventNotification,      en_init_eventNotification),
    REXX_METHOD(en_addUserMessage,              en_addUserMessage),
    REXX_METHOD(en_connectAllSBEvents,          en_connectAllSBEvents),
    REXX_METHOD(en_connectButtonEvent,          en_connectButtonEvent),
    REXX_METHOD(en_connectComboBoxEvent,        en_connectComboBoxEvent),
    REXX_METHOD(en_connectCommandEvents,        en_connectCommandEvents),
    REXX_METHOD(en_connectDateTimePickerEvent,  en_connectDateTimePickerEvent),
    REXX_METHOD(en_connectDraw,                 en_connectDraw),
    REXX_METHOD(en_connectEachSBEvent,          en_connectEachSBEvent),
    REXX_METHOD(en_connectEditEvent,            en_connectEditEvent),
    REXX_METHOD(en_connectFKeyPress,            en_connectFKeyPress),
    REXX_METHOD(en_connectKeyPress,             en_connectKeyPress),
    REXX_METHOD(en_connectListBoxEvent,         en_connectListBoxEvent),
    REXX_METHOD(en_connectListViewEvent,        en_connectListViewEvent),
    REXX_METHOD(en_connectMonthCalendarEvent,   en_connectMonthCalendarEvent),
    REXX_METHOD(en_connectNotifyEvent,          en_connectNotifyEvent),
    REXX_METHOD(en_connectReBarEvent,           en_connectReBarEvent),
    REXX_METHOD(en_connectScrollBarEvent,       en_connectScrollBarEvent),
    REXX_METHOD(en_connectStaticEvent,          en_connectStaticEvent),
    REXX_METHOD(en_connectStatusBarEvent,       en_connectStatusBarEvent),
    REXX_METHOD(en_connectTabEvent,             en_connectTabEvent),
    REXX_METHOD(en_connectToolBarEvent,         en_connectToolBarEvent),
    REXX_METHOD(en_connectToolTipEvent,         en_connectToolTipEvent),
    REXX_METHOD(en_connectTrackBarEvent,        en_connectTrackBarEvent),
    REXX_METHOD(en_connectTreeViewEvent,        en_connectTreeViewEvent),
    REXX_METHOD(en_connectUpDownEvent,          en_connectUpDownEvent),
    REXX_METHOD(en_connectWmEvent,              en_connectWmEvent),
    REXX_METHOD(en_disconnectKeyPress,          en_disconnectKeyPress),
    REXX_METHOD(en_hasKeyPressConnection,       en_hasKeyPressConnection),

    // PlainBaseDialog
    REXX_METHOD(pbdlg_init_cls,                 pbdlg_init_cls),
    REXX_METHOD(pbdlg_setDefaultFont_cls,       pbdlg_setDefaultFont_cls),
    REXX_METHOD(pbdlg_getFontName_cls,          pbdlg_getFontName_cls),
    REXX_METHOD(pbdlg_getFontSize_cls,          pbdlg_getFontSize_cls),
    REXX_METHOD(pbdlg_new_cls,                  pbdlg_new_cls),
    REXX_METHOD(pbdlg_init,                     pbdlg_init),
    REXX_METHOD(pbdlg_getFontNameSize,          pbdlg_getFontNameSize),
    REXX_METHOD(pbdlg_setFontName_pvt,          pbdlg_setFontName_pvt),
    REXX_METHOD(pbdlg_setFontSize_pvt,          pbdlg_setFontSize_pvt),
    REXX_METHOD(pbdlg_setDlgFont,               pbdlg_setDlgFont),
    REXX_METHOD(pbdlg_getAutoDetect,            pbdlg_getAutoDetect),
    REXX_METHOD(pbdlg_setAutoDetect,            pbdlg_setAutoDetect),
    REXX_METHOD(pbdlg_getDlgID,                 pbdlg_getDlgID),
    REXX_METHOD(pbdlg_setDlgID,                 pbdlg_setDlgID),
    REXX_METHOD(pbdlg_getParentDlg_pvt,         pbdlg_getParentDlg_pvt),
    REXX_METHOD(pbdlg_setParentDlg_pvt,         pbdlg_setParentDlg_pvt),
    REXX_METHOD(pbdlg_getOwnerDialog,           pbdlg_getOwnerDialog),
    REXX_METHOD(pbdlg_setOwnerDialog,           pbdlg_setOwnerDialog),
    REXX_METHOD(pbdlg_getFinished,              pbdlg_getFinished),
    REXX_METHOD(pbdlg_setFinished,              pbdlg_setFinished),
    REXX_METHOD(pbdlg_sendMessageToControl,     pbdlg_sendMessageToControl),
    REXX_METHOD(pbdlg_sendMessageToWindow,      pbdlg_sendMessageToWindow),
    REXX_METHOD(pbdlg_get,                      pbdlg_get),
    REXX_METHOD(pbdlg_getDlgHandle,             pbdlg_getDlgHandle),
    REXX_METHOD(pbdlg_getLibrary,               pbdlg_getLibrary),
    REXX_METHOD(pbdlg_getResourceID,            pbdlg_getResourceID),
    REXX_METHOD(pbdlg_isDialogActive,           pbdlg_isDialogActive),
    REXX_METHOD(pbdlg_show,                     pbdlg_show),
    REXX_METHOD(pbdlg_showWindow,               pbdlg_showWindow),
    REXX_METHOD(pbdlg_toTheTop,                 pbdlg_toTheTop),
    REXX_METHOD(pbdlg_getFocus,                 pbdlg_getFocus),
    REXX_METHOD(pbdlg_setFocus,                 pbdlg_setFocus),
    REXX_METHOD(pbdlg_tabTo,                    pbdlg_tabTo),
    REXX_METHOD(pbdlg_backgroundBitmap,         pbdlg_backgroundBitmap),
    REXX_METHOD(pbdlg_tiledBackgroundBitmap,    pbdlg_tiledBackgroundBitmap),
    REXX_METHOD(pbdlg_backgroundColor,          pbdlg_backgroundColor),
    REXX_METHOD(pbdlg_pixel2dlgUnit,            pbdlg_pixel2dlgUnit),
    REXX_METHOD(pbdlg_dlgUnit2pixel,            pbdlg_dlgUnit2pixel),
    REXX_METHOD(pbdlg_focusControl,             pbdlg_focusControl),
    REXX_METHOD(pbdlg_showControl,              pbdlg_showControl),
    REXX_METHOD(pbdlg_connect_ControName,       pbdlg_connect_ControName),
    REXX_METHOD(pbdlg_setDlgDataFromStem_pvt,   pbdlg_setDlgDataFromStem_pvt),
    REXX_METHOD(pbdlg_putDlgDataInStem_pvt,     pbdlg_putDlgDataInStem_pvt),
    REXX_METHOD(pbdlg_getControlData,           pbdlg_getControlData),
    REXX_METHOD(pbdlg_setControlData,           pbdlg_setControlData),
    REXX_METHOD(pbdlg_getControlHandle,         pbdlg_getControlHandle),
    REXX_METHOD(pbdlg_getWindowText,            pbdlg_getWindowText),
    REXX_METHOD(pbdlg_setWindowText,            pbdlg_setWindowText),
    REXX_METHOD(pbdlg_getControlText,           pbdlg_getControlText),
    REXX_METHOD(pbdlg_setControlText,           pbdlg_setControlText),
    REXX_METHOD(pbdlg_getTextSizeDu,            pbdlg_getTextSizeDu),
    REXX_METHOD(pbdlg_getTextSizeDlg,           pbdlg_getTextSizeDlg),
    REXX_METHOD(pbdlg_getTextSizeTitleBar,      pbdlg_getTextSizeTitleBar),
    REXX_METHOD(pbdlg_enableDisableControl,     pbdlg_enableDisableControl),
    REXX_METHOD(pbdlg_getControlID,             pbdlg_getControlID),
    REXX_METHOD(pbdlg_center,                   pbdlg_center),
    REXX_METHOD(pbdlg_doMinMax,                 pbdlg_doMinMax),
    REXX_METHOD(pbdlg_setTabGroup,              pbdlg_setTabGroup),
    REXX_METHOD(pbdlg_stopIt,                   pbdlg_stopIt),
    REXX_METHOD(pbdlg_createToolTip,            pbdlg_createToolTip),
    REXX_METHOD(pbdlg_newToolTip,               pbdlg_newToolTip),
    REXX_METHOD(pbdlg_newControl,               pbdlg_newControl),
    REXX_METHOD(pbdlg_newControlEx,             pbdlg_newControlEx),
    REXX_METHOD(pbdlg_getNewControl,            pbdlg_getNewControl),
    REXX_METHOD(pbdlg_putControl,               pbdlg_putControl),
    REXX_METHOD(pbdlg_dumpMessageTable,         pbdlg_dumpMessageTable),
    REXX_METHOD(pbdlg_unInit,                   pbdlg_unInit),
    REXX_METHOD(pbdlg_test,                     pbdlg_test),

    REXX_METHOD(global_resolveSymbolicID,       global_resolveSymbolicID),

    REXX_METHOD(dlgext_setWindowRect,           dlgext_setWindowRect),
    REXX_METHOD(dlgext_clearWindowRect,         dlgext_clearWindowRect),
    REXX_METHOD(dlgext_clearControlRect,        dlgext_clearControlRect),
    REXX_METHOD(dlgext_clearRect,               dlgext_clearRect),
    REXX_METHOD(dlgext_getControlRect,          dlgext_getControlRect),
    REXX_METHOD(dlgext_redrawWindowRect,        dlgext_redrawWindowRect),
    REXX_METHOD(dlgext_redrawControl,           dlgext_redrawControl),
    REXX_METHOD(dlgext_redrawRect,              dlgext_redrawRect),
    REXX_METHOD(dlgext_resizeMoveControl,       dlgext_resizeMoveControl),
    REXX_METHOD(dlgext_setForgroundWindow,      dlgext_setForgroundWindow),
    REXX_METHOD(dlgext_setControlColor,         dlgext_setControlColor),
    REXX_METHOD(dlgext_installBitmapButton,     dlgext_installBitmapButton),
    REXX_METHOD(dlgext_changeBitmapButton,      dlgext_changeBitmapButton),
    REXX_METHOD(dlgext_dimBitmap,               dlgext_dimBitmap),
    REXX_METHOD(dlgext_scrollButton,            dlgext_scrollButton),
    REXX_METHOD(dlgext_drawBitmap,              dlgext_drawBitmap),
    REXX_METHOD(dlgext_drawButton,              dlgext_drawButton),
    REXX_METHOD(dlgext_getBitmapPosition,       dlgext_getBitmapPosition),
    REXX_METHOD(dlgext_setBitmapPosition,       dlgext_setBitmapPosition),
    REXX_METHOD(dlgext_getBitmapSize,           dlgext_getBitmapSize),
    REXX_METHOD(dlgext_getWindowDC,             dlgext_getWindowDC),
    REXX_METHOD(dlgext_freeWindowDC,            dlgext_freeWindowDC),
    REXX_METHOD(dlgext_writeToWindow,           dlgext_writeToWindow),
    REXX_METHOD(dlgext_scrollText,              dlgext_scrollText),
    REXX_METHOD(dlgext_createBrush,             dlgext_createBrush),

    REXX_METHOD(userdlg_init,                   userdlg_init),
    REXX_METHOD(userdlg_test,                   userdlg_test),

    REXX_METHOD(catdlg_createCategoryDialog,           catdlg_createCategoryDialog),
    REXX_METHOD(catdlg_getControlDataPage,             catdlg_getControlDataPage),
    REXX_METHOD(catdlg_setControlDataPage,             catdlg_setControlDataPage),
    REXX_METHOD(catdlg_sendMessageToCategoryControl,   catdlg_sendMessageToCategoryControl),

    REXX_METHOD(dyndlg_init_cls,                dyndlg_init_cls),
    REXX_METHOD(dyndlg_getBasePtr,              dyndlg_getBasePtr),
    REXX_METHOD(dyndlg_setBasePtr,              dyndlg_setBasePtr),
    REXX_METHOD(dyndlg_getActivePtr,            dyndlg_getActivePtr),
    REXX_METHOD(dyndlg_setActivePtr,            dyndlg_setActivePtr),
    REXX_METHOD(dyndlg_getDialogItemCount,      dyndlg_getDialogItemCount),
    REXX_METHOD(dyndlg_setDialogItemCount,      dyndlg_setDialogItemCount),
    REXX_METHOD(dyndlg_dynamicInit,             dyndlg_dynamicInit),
    REXX_METHOD(dyndlg_create,                  dyndlg_create),
    REXX_METHOD(dyndlg_createStatic,            dyndlg_createStatic),
    REXX_METHOD(dyndlg_createStaticText,        dyndlg_createStaticText),
    REXX_METHOD(dyndlg_createStaticImage,       dyndlg_createStaticImage),
    REXX_METHOD(dyndlg_createStaticFrame,       dyndlg_createStaticFrame),
    REXX_METHOD(dyndlg_createPushButton,        dyndlg_createPushButton),
    REXX_METHOD(dyndlg_createPushButtonStem,    dyndlg_createPushButtonStem),
    REXX_METHOD(dyndlg_createRadioButton,       dyndlg_createRadioButton),
    REXX_METHOD(dyndlg_createGroupBox,          dyndlg_createGroupBox),
    REXX_METHOD(dyndlg_createEdit,              dyndlg_createEdit),
    REXX_METHOD(dyndlg_createScrollBar,         dyndlg_createScrollBar),
    REXX_METHOD(dyndlg_createListBox,           dyndlg_createListBox),
    REXX_METHOD(dyndlg_createComboBox,          dyndlg_createComboBox),
    REXX_METHOD(dyndlg_createProgressBar,       dyndlg_createProgressBar),
    REXX_METHOD(dyndlg_createNamedControl,      dyndlg_createNamedControl),
    REXX_METHOD(dyndlg_addButton,               dyndlg_addButton),
    REXX_METHOD(dyndlg_addRadioButton,          dyndlg_addRadioButton),
    REXX_METHOD(dyndlg_addGroupBox,             dyndlg_addGroupBox),
    REXX_METHOD(dyndlg_addEntryLine,            dyndlg_addEntryLine),
    REXX_METHOD(dyndlg_addMethod,               dyndlg_addMethod),
    REXX_METHOD(dyndlg_addIconResource,         dyndlg_addIconResource),
    REXX_METHOD(dyndlg_startParentDialog,       dyndlg_startParentDialog),
    REXX_METHOD(dyndlg_startChildDialog,        dyndlg_startChildDialog),
    REXX_METHOD(dyndlg_stop,                    dyndlg_stop),

    REXX_METHOD(window_init,                    window_init),
    REXX_METHOD(window_unInit,                  window_unInit),

    // ResizingAdmin
    REXX_METHOD(ra_maxSize,                     ra_maxSize),
    REXX_METHOD(ra_setMaxSize,                  ra_setMaxSize),
    REXX_METHOD(ra_minSize,                     ra_minSize),
    REXX_METHOD(ra_setMinSize,                  ra_setMinSize),
    REXX_METHOD(ra_controlSide,                 ra_controlSide),
    REXX_METHOD(ra_controlSizing,               ra_controlSizing),
    REXX_METHOD(ra_defaultSide,                 ra_defaultSide),
    REXX_METHOD(ra_defaultSizing,               ra_defaultSizing),
    REXX_METHOD(ra_initResizing,                ra_initResizing),
    REXX_METHOD(ra_noMaxSize,                   ra_noMaxSize),
    REXX_METHOD(ra_noMinSize,                   ra_noMinSize),
    REXX_METHOD(ra_pagedTab,                    ra_pagedTab),
    REXX_METHOD(ra_useDefaultSizing,            ra_useDefaultSizing),
    REXX_METHOD(ra_wantSizeEnded,               ra_wantSizeEnded),

    // ResDialog
    REXX_METHOD(resdlg_init,                    resdlg_init),
    REXX_METHOD(resdlg_getDataTableIDs_pvt,     resdlg_getDataTableIDs_pvt),
    REXX_METHOD(resdlg_startDialog_pvt,         resdlg_startDialog_pvt),

    // TabOwnerDialog
    REXX_METHOD(tod_tabOwnerDlgInit,            tod_tabOwnerDlgInit),
    REXX_METHOD(tod_getTabPage,                 tod_getTabPage),
    REXX_METHOD(tod_tabOwnerOk,                 tod_tabOwnerOk),
    REXX_METHOD(tod_tabOwnerCancel,             tod_tabOwnerCancel),

    // TabOwnerDlgInfo
    REXX_METHOD(todi_init,                      todi_init),
    REXX_METHOD(todi_add,                       todi_add),

    // ManagedTab
    REXX_METHOD(mt_init,                        mt_init),

    // ControlDialog
    REXX_METHOD(cd_init_cls,                    cd_init_cls),
    REXX_METHOD(cd_controlDlgInit,              cd_controlDlgInit),
    REXX_METHOD(cd_get_isManaged,               cd_get_isManaged),
    REXX_METHOD(cd_get_wasActivated,            cd_get_wasActivated),
    REXX_METHOD(cd_get_extraOptions,            cd_get_extraOptions),
    REXX_METHOD(cd_set_extraOptions,            cd_set_extraOptions),
    REXX_METHOD(cd_get_initializing,            cd_get_initializing),
    REXX_METHOD(cd_set_initializing,            cd_set_initializing),
    REXX_METHOD(cd_get_pageTitle,               cd_get_pageTitle),
    REXX_METHOD(cd_set_pageTitle,               cd_set_pageTitle),

    // ControlDlgInfo
    REXX_METHOD(cdi_set_title,                  cdi_set_title),
    REXX_METHOD(cdi_set_size,                   cdi_set_size),
    REXX_METHOD(cdi_set_managed,                cdi_set_managed),
    REXX_METHOD(cdi_init,                       cdi_init),

    // ResControlDialog
    REXX_METHOD(resCtrlDlg_startDialog_pvt,     resCtrlDlg_startDialog_pvt),

    // RcControlDialog
    REXX_METHOD(rcCtrlDlg_startTemplate,       rcCtrlDlg_startTemplate),

    // PropertySheetDialog
    REXX_METHOD(psdlg_getPages_atr,             psdlg_getPages_atr),
    REXX_METHOD(psdlg_setCaption_atr,           psdlg_setCaption_atr),
    REXX_METHOD(psdlg_setResources_atr,         psdlg_setResources_atr),
    REXX_METHOD(psdlg_setAppIcon_atr,           psdlg_setAppIcon_atr),
    REXX_METHOD(psdlg_setHeader_atr,            psdlg_setHeader_atr),
    REXX_METHOD(psdlg_setWatermark_atr,         psdlg_setWatermark_atr),
    REXX_METHOD(psdlg_setStartPage_atr,         psdlg_setStartPage_atr),
    REXX_METHOD(psdlg_setImageList_atr,         psdlg_setImageList_atr),
    REXX_METHOD(psdlg_init,                     psdlg_init),
    REXX_METHOD(psdlg_execute,                  psdlg_execute),
    REXX_METHOD(psdlg_popup,                    psdlg_popup),
    REXX_METHOD(psdlg_popupAsChild,             psdlg_popupAsChild),
    REXX_METHOD(psdlg_getPage,                  psdlg_getPage),
    REXX_METHOD(psdlg_addPage,                  psdlg_addPage),
    REXX_METHOD(psdlg_insertPage,               psdlg_insertPage),
    REXX_METHOD(psdlg_removePage,               psdlg_removePage),
    REXX_METHOD(psdlg_apply,                    psdlg_apply),
    REXX_METHOD(psdlg_cancelToClose,            psdlg_cancelToClose),
    REXX_METHOD(psdlg_changed,                  psdlg_changed),
    REXX_METHOD(psdlg_getCurrentPageHwnd,       psdlg_getCurrentPageHwnd),
    REXX_METHOD(psdlg_getTabControl,            psdlg_getTabControl),
    REXX_METHOD(psdlg_getResult,                psdlg_getResult),
    REXX_METHOD(psdlg_hwndToIndex,              psdlg_hwndToIndex),
    REXX_METHOD(psdlg_idToIndex,                psdlg_idToIndex),
    REXX_METHOD(psdlg_indexToID,                psdlg_indexToID),
    REXX_METHOD(psdlg_indexToHandle,            psdlg_indexToHandle),
    REXX_METHOD(psdlg_pageToIndex,              psdlg_pageToIndex),
    REXX_METHOD(psdlg_pressButton,              psdlg_pressButton),
    REXX_METHOD(psdlg_setCurSel,                psdlg_setCurSel),
    REXX_METHOD(psdlg_setCurSelByID,            psdlg_setCurSelByID),
    REXX_METHOD(psdlg_setWizButtons,            psdlg_setWizButtons),
    REXX_METHOD(psdlg_enableWizButtons,         psdlg_enableWizButtons),
    REXX_METHOD(psdlg_showWizButtons,           psdlg_showWizButtons),
    REXX_METHOD(psdlg_querySiblings,            psdlg_querySiblings),
    REXX_METHOD(psdlg_resetPageText,            psdlg_resetPageText),
    REXX_METHOD(psdlg_setTitle,                 psdlg_setTitle),
    REXX_METHOD(psdlg_setButtonText,            psdlg_setButtonText),
    REXX_METHOD(psdlg_setFinishText,            psdlg_setFinishText),
    REXX_METHOD(psdlg_setNextText,              psdlg_setNextText),
    REXX_METHOD(psdlg_unchanged,                psdlg_unchanged),
    REXX_METHOD(psdlg_test,                     psdlg_test),

    // PropertySheetPage
    REXX_METHOD(psp_init_cls,                   psp_init_cls),
    REXX_METHOD(psp_propSheet_atr,              psp_propSheet_atr),
    REXX_METHOD(psp_wasActivated_atr,           psp_wasActivated_atr),
    REXX_METHOD(psp_pageID_atr,                 psp_pageID_atr),
    REXX_METHOD(psp_pageNumber_atr,             psp_pageNumber_atr),
    REXX_METHOD(psp_setResources_atr,           psp_setResources_atr),
    REXX_METHOD(psp_setTabIcon_atr,             psp_setTabIcon_atr),
    REXX_METHOD(psp_getcx,                      psp_getcx),
    REXX_METHOD(psp_setcx,                      psp_setcx),
    REXX_METHOD(psp_getPageTitle,               psp_getPageTitle),
    REXX_METHOD(psp_setPageTitle,               psp_setPageTitle),
    REXX_METHOD(psp_getWantNotification,        psp_getWantNotification),
    REXX_METHOD(psp_setWantNotification,        psp_setWantNotification),
    REXX_METHOD(psp_init_propertySheetPage,     psp_init_propertySheetPage),
    REXX_METHOD(psp_initTemplate,               psp_initTemplate),
    REXX_METHOD(psp_setSize,                    psp_setSize),

    // UserPSPDialog
    REXX_METHOD(userpspdlg_init,                userpspdlg_init),

    // RcPSPDialog
    REXX_METHOD(rcpspdlg_init,                  rcpspdlg_init),
    REXX_METHOD(rcpspdlg_startTemplate,         rcpspdlg_startTemplate),

    // ResPSPDialog
    REXX_METHOD(respspdlg_init,                 respspdlg_init),

    // ProgressDialog
    REXX_METHOD(pd_setInterruptible,            pd_setInterruptible),

    //TimedMessage
    REXX_METHOD(timedmsg_init,                  timedmsg_init),

    // WindowExtensions
    REXX_METHOD(winex_initWindowExtensions,     winex_initWindowExtensions),
    REXX_METHOD(winex_getFont,                  winex_getFont),
    REXX_METHOD(winex_setFont,                  winex_setFont),
    REXX_METHOD(winex_createFontEx,             winex_createFontEx),
    REXX_METHOD(winex_createFont,               winex_createFont),
    REXX_METHOD(winex_getScrollPos,             winex_getScrollPos),
    REXX_METHOD(winex_setScrollPos,        	    winex_setScrollPos),
    REXX_METHOD(winex_scroll,        	        winex_scroll),
    REXX_METHOD(winex_writeDirect,              winex_writeDirect),
    REXX_METHOD(winex_loadBitmap,               winex_loadBitmap),
    REXX_METHOD(winex_removeBitmap,             winex_removeBitmap),
    REXX_METHOD(winex_write,                    winex_write),
    REXX_METHOD(winex_createBrush,              winex_createBrush),
    REXX_METHOD(winex_createPen,                winex_createPen),
    REXX_METHOD(winex_deleteObject,             winex_deleteObject),
    REXX_METHOD(winex_objectToDC,               winex_objectToDC),
    REXX_METHOD(winex_getTextExtent,            winex_getTextExtent),
    REXX_METHOD(winex_getTextAlign,             winex_getTextAlign),
    REXX_METHOD(winex_setTextAlign,             winex_setTextAlign),
    REXX_METHOD(winex_getDC,                    winex_getDC),
    REXX_METHOD(winex_getSysBrush,              winex_getSysBrush),
    REXX_METHOD(winex_freeDC,                   winex_freeDC),
    REXX_METHOD(winex_rectangle,                winex_rectangle),
    REXX_METHOD(winex_drawLine,                 winex_drawLine),
    REXX_METHOD(winex_drawPixel,                winex_drawPixel),
    REXX_METHOD(winex_getPixel,                 winex_getPixel),
    REXX_METHOD(winex_fillDrawing,              winex_fillDrawing),
    REXX_METHOD(winex_fillRect,                 winex_fillRect),
    REXX_METHOD(winex_drawArcOrPie,             winex_drawArcOrPie),
    REXX_METHOD(winex_drawAngleArc,             winex_drawAngleArc),
    REXX_METHOD(winex_fontColor,                winex_fontColor),
    REXX_METHOD(winex_textBkMode,               winex_textBkMode),
    REXX_METHOD(winex_getSetArcDirection,       winex_getSetArcDirection),

    // ResourceImage
    REXX_METHOD(ri_init,                        ri_init),
    REXX_METHOD(ri_release,                     ri_release),
    REXX_METHOD(ri_handle,                      ri_handle),
    REXX_METHOD(ri_isNull,                      ri_isNull),
    REXX_METHOD(ri_systemErrorCode,             ri_systemErrorCode),
    REXX_METHOD(ri_getImage,                    ri_getImage),
    REXX_METHOD(ri_getImages,                   ri_getImages),

    // Image
    REXX_METHOD(image_toID_cls,                 image_toID_cls),
    REXX_METHOD(image_getImage_cls,             image_getImage_cls),
    REXX_METHOD(image_fromFiles_cls,            image_fromFiles_cls),
    REXX_METHOD(image_fromIDs_cls,              image_fromIDs_cls),
    REXX_METHOD(image_userIcon_cls,             image_userIcon_cls),
    REXX_METHOD(image_colorRef_cls,             image_colorRef_cls),
    REXX_METHOD(image_getRValue_cls,            image_getRValue_cls),
    REXX_METHOD(image_getGValue_cls,            image_getGValue_cls),
    REXX_METHOD(image_getBValue_cls,            image_getBValue_cls),
    REXX_METHOD(image_init,                     image_init),
    REXX_METHOD(image_release,                  image_release),
    REXX_METHOD(image_isNull,                   image_isNull),
    REXX_METHOD(image_systemErrorCode,          image_systemErrorCode),
    REXX_METHOD(image_handle,                   image_handle),

    // ImageList
    REXX_METHOD(il_create_cls,                  il_create_cls),
    REXX_METHOD(il_init,                        il_init),
    REXX_METHOD(il_add,                         il_add),
    REXX_METHOD(il_addMasked,                   il_addMasked),
    REXX_METHOD(il_addIcon,                     il_addIcon),
    REXX_METHOD(il_addImages,                   il_addImages),
    REXX_METHOD(il_duplicate,                   il_duplicate),
    REXX_METHOD(il_getCount,                    il_getCount),
    REXX_METHOD(il_getIcon,                     il_getIcon),
    REXX_METHOD(il_getImageSize,                il_getImageSize),
    REXX_METHOD(il_handle,                      il_handle),
    REXX_METHOD(il_isNull,                      il_isNull),
    REXX_METHOD(il_release,                     il_release),
    REXX_METHOD(il_remove,                      il_remove),
    REXX_METHOD(il_removeAll,                   il_removeAll),

    // DialogControl
    REXX_METHOD(dlgctrl_new_cls,                dlgctrl_new_cls),
    REXX_METHOD(dlgctrl_init_cls,               dlgctrl_init_cls),
    REXX_METHOD(dlgctrl_init,                   dlgctrl_init),
    REXX_METHOD(dlgctrl_unInit,                 dlgctrl_unInit),
    REXX_METHOD(dlgctrl_addUserSubclass,        dlgctrl_addUserSubclass),
    REXX_METHOD(dlgctrl_assignFocus,            dlgctrl_assignFocus),
    REXX_METHOD(dlgctrl_clearRect,              dlgctrl_clearRect),
    REXX_METHOD(dlgctrl_connectEvent,           dlgctrl_connectEvent),
    REXX_METHOD(dlgctrl_connectFKeyPress,       dlgctrl_connectFKeyPress),
    REXX_METHOD(dlgctrl_connectKeyPress,        dlgctrl_connectKeyPress),
    REXX_METHOD(dlgctrl_data,                   dlgctrl_data),
    REXX_METHOD(dlgctrl_dataEquals,             dlgctrl_dataEquals),
    REXX_METHOD(dlgctrl_disconnectKeyPress,     dlgctrl_disconnectKeyPress),
    REXX_METHOD(dlgctrl_getTextSizeDlg,         dlgctrl_getTextSizeDlg),
    REXX_METHOD(dlgctrl_hasKeyPressConnection,  dlgctrl_hasKeyPressConnection),
    REXX_METHOD(dlgctrl_redrawRect,             dlgctrl_redrawRect),
    REXX_METHOD(dlgctrl_setColor,               dlgctrl_setColor),
    REXX_METHOD(dlgctrl_setParent,              dlgctrl_setParent),
    REXX_METHOD(dlgctrl_setWindowTheme,         dlgctrl_setWindowTheme),
    REXX_METHOD(dlgctrl_tabGroup,               dlgctrl_tabGroup),
    REXX_METHOD(dlgctrl_textSize,               dlgctrl_textSize),
    REXX_METHOD(dlgctrl_putInBag,               dlgctrl_putInBag),

    // Generic methods for dialog control
    REXX_METHOD(generic_getToolTips,            generic_getToolTips),
    REXX_METHOD(generic_setListTabulators,      generic_setListTabulators),
    REXX_METHOD(generic_setToolTips,            generic_setToolTips),

    // Static
    REXX_METHOD(stc_getIcon,                    stc_getIcon),
    REXX_METHOD(stc_setIcon,                    stc_setIcon),
    REXX_METHOD(stc_getImage,                   stc_getImage),
    REXX_METHOD(stc_setImage,                   stc_setImage),

    // Buttons
    REXX_METHOD(gb_setStyle,                    gb_setStyle),
    REXX_METHOD(bc_getState,                    bc_getState),
    REXX_METHOD(bc_setState,                    bc_setState),
    REXX_METHOD(bc_setStyle,                    bc_setStyle),
    REXX_METHOD(bc_getIdealSize,                bc_getIdealSize),
    REXX_METHOD(bc_getTextMargin,               bc_getTextMargin),
    REXX_METHOD(bc_setTextMargin,               bc_setTextMargin),
    REXX_METHOD(bc_getImage,                    bc_getImage),
    REXX_METHOD(bc_setImage,                    bc_setImage),
    REXX_METHOD(bc_setImageList,                bc_setImageList),
    REXX_METHOD(bc_getImageList,                bc_getImageList),
    REXX_METHOD(rb_checkInGroup_cls,            rb_checkInGroup_cls),
    REXX_METHOD(rb_checked,                     rb_checked),
    REXX_METHOD(rb_check,                       rb_check),
    REXX_METHOD(rb_uncheck,                     rb_uncheck),
    REXX_METHOD(rb_getCheckState,               rb_getCheckState),
    REXX_METHOD(rb_isChecked,                   rb_isChecked),
    REXX_METHOD(rb_indeterminate,               rb_indeterminate),
    REXX_METHOD(ckbx_isIndeterminate,           ckbx_isIndeterminate),
    REXX_METHOD(ckbx_setIndeterminate,          ckbx_setIndeterminate),
    REXX_METHOD(bc_test,                        bc_test),
    REXX_METHOD(bc_test_cls,                    bc_test_cls),

    // Edit
    REXX_METHOD(e_isSingleLine,                 e_isSingleLine),
    REXX_METHOD(e_selection,                    e_selection),
    REXX_METHOD(e_replaceSelText,               e_replaceSelText),
    REXX_METHOD(e_getLine,                      e_getLine),
    REXX_METHOD(e_isGrandChild,                 e_isGrandChild),
    REXX_METHOD(e_lineIndex,                    e_lineIndex),
    REXX_METHOD(e_lineFromIndex,                e_lineFromIndex),
    REXX_METHOD(e_lineLength,                   e_lineLength),
    REXX_METHOD(e_setTabStops,                  e_setTabStops),
    REXX_METHOD(e_style,                        e_style),
    REXX_METHOD(e_showBallon,                   e_showBallon),
    REXX_METHOD(e_hideBallon,                   e_hideBallon),
    REXX_METHOD(e_getCue,                       e_getCue),
    REXX_METHOD(e_setCue,                       e_setCue),
    REXX_METHOD(e_getRect,                      e_getRect),
    REXX_METHOD(e_setRect,                      e_setRect),

    // ComboBox
    REXX_METHOD(cb_add,                         cb_add),
    REXX_METHOD(cb_addDirectory,                cb_addDirectory),
    REXX_METHOD(cb_find,                        cb_find),
    REXX_METHOD(cb_getCue,                      cb_getCue),
    REXX_METHOD(cb_getComboBoxInfo,             cb_getComboBoxInfo),
    REXX_METHOD(cb_getEditControl,              cb_getEditControl),
    REXX_METHOD(cb_getItemData,                 cb_getItemData),
    REXX_METHOD(cb_getItemHeight,               cb_getItemHeight),
    REXX_METHOD(cb_getMinVisible,               cb_getMinVisible),
    REXX_METHOD(cb_getText,                     cb_getText),
    REXX_METHOD(cb_insert,                      cb_insert),
    REXX_METHOD(cb_isDropDown,                  cb_isDropDown),
    REXX_METHOD(cb_isGrandchild,                cb_isGrandchild),
    REXX_METHOD(cb_removeFullColor,             cb_removeFullColor),
    REXX_METHOD(cb_removeItemData,              cb_removeItemData),
    REXX_METHOD(cb_select,                      cb_select),
    REXX_METHOD(cb_setCue,                      cb_setCue),
    REXX_METHOD(cb_setFullColor,                cb_setFullColor),
    REXX_METHOD(cb_setItemData,                 cb_setItemData),
    REXX_METHOD(cb_setItemHeight,               cb_setItemHeight),
    REXX_METHOD(cb_setMinVisible,               cb_setMinVisible),

    // ListBox
    REXX_METHOD(lb_add,                         lb_add),
    REXX_METHOD(lb_addDirectory,                lb_addDirectory),
    REXX_METHOD(lb_deselectIndex,               lb_deselectIndex),
    REXX_METHOD(lb_find,                        lb_find),
    REXX_METHOD(lb_getText,                     lb_getText),
    REXX_METHOD(lb_hitTestInfo,                 lb_hitTestInfo),
    REXX_METHOD(lb_insert,                      lb_insert),
    REXX_METHOD(lb_isSingleSelection,           lb_isSingleSelection),
    REXX_METHOD(lb_select,                      lb_select),
    REXX_METHOD(lb_selectedIndex,               lb_selectedIndex),
    REXX_METHOD(lb_selectIndex,                 lb_selectIndex),

    // ListView
    REXX_METHOD(lv_add,                         lv_add),
    REXX_METHOD(lv_addClearExtendStyle,         lv_addClearExtendStyle),
    REXX_METHOD(lv_addFullRow,         	        lv_addFullRow),
    REXX_METHOD(lv_addRemoveStyle,              lv_addRemoveStyle),
    REXX_METHOD(lv_addRow,                      lv_addRow),
    REXX_METHOD(lv_addRowFromArray,             lv_addRowFromArray),
    REXX_METHOD(lv_arrange,                     lv_arrange),
    REXX_METHOD(lv_checkUncheck,                lv_checkUncheck),
    REXX_METHOD(lv_delete,                      lv_delete),
    REXX_METHOD(lv_deleteAll,                   lv_deleteAll),
    REXX_METHOD(lv_deleteColumn,                lv_deleteColumn),
    REXX_METHOD(lv_deselectAll,                 lv_deselectAll),
    REXX_METHOD(lv_find,                        lv_find),
    REXX_METHOD(lv_findNearestXY,               lv_findNearestXY),
    REXX_METHOD(lv_fixFullRowColumns,           lv_fixFullRowColumns),
    REXX_METHOD(lv_getCheck,                    lv_getCheck),
    REXX_METHOD(lv_getColor,                    lv_getColor),
    REXX_METHOD(lv_getColumnCount,              lv_getColumnCount),
    REXX_METHOD(lv_getColumnInfo,               lv_getColumnInfo),
    REXX_METHOD(lv_getColumnOrder,              lv_getColumnOrder),
    REXX_METHOD(lv_getColumnText,               lv_getColumnText),
    REXX_METHOD(lv_getExtendedStyle,            lv_getExtendedStyle),
    REXX_METHOD(lv_getFullRow,                  lv_getFullRow),
    REXX_METHOD(lv_getImageList,                lv_getImageList),
    REXX_METHOD(lv_getItem,                     lv_getItem),
    REXX_METHOD(lv_getItemData,                 lv_getItemData),
    REXX_METHOD(lv_getItemInfo,                 lv_getItemInfo),
    REXX_METHOD(lv_getItemPos,                  lv_getItemPos),
    REXX_METHOD(lv_getItemRect,                 lv_getItemRect),
    REXX_METHOD(lv_getNextItem,                 lv_getNextItem),
    REXX_METHOD(lv_getNextItemWithState,        lv_getNextItemWithState),
    REXX_METHOD(lv_getSubitem,                  lv_getSubitem),
    REXX_METHOD(lv_getSubitemRect,              lv_getSubitemRect),
    REXX_METHOD(lv_getView,                     lv_getView),
    REXX_METHOD(lv_hasCheckBoxes,               lv_hasCheckBoxes),
    REXX_METHOD(lv_hitTestInfo,                 lv_hitTestInfo),
    REXX_METHOD(lv_insert,                      lv_insert),
    REXX_METHOD(lv_insertColumnPx,              lv_insertColumnPx),
    REXX_METHOD(lv_insertFullRow,               lv_insertFullRow),
    REXX_METHOD(lv_isChecked,                   lv_isChecked),
    REXX_METHOD(lv_itemText,                    lv_itemText),
    REXX_METHOD(lv_itemState,                   lv_itemState),
    REXX_METHOD(lv_modify,                      lv_modify),
    REXX_METHOD(lv_modifyColumnPx,              lv_modifyColumnPx),
    REXX_METHOD(lv_modifyFullRow,               lv_modifyFullRow),
    REXX_METHOD(lv_modifyItem,                  lv_modifyItem),
    REXX_METHOD(lv_modifySubitem,               lv_modifySubitem),
    REXX_METHOD(lv_prependFullRow,         	    lv_prependFullRow),
    REXX_METHOD(lv_removeItemData,              lv_removeItemData),
    REXX_METHOD(lv_replaceExtendStyle,          lv_replaceExtendStyle),
    REXX_METHOD(lv_replaceStyle,                lv_replaceStyle),
    REXX_METHOD(lv_setColor,                    lv_setColor),
    REXX_METHOD(lv_setColumnOrder,              lv_setColumnOrder),
    REXX_METHOD(lv_setColumnWidthPx,            lv_setColumnWidthPx),
    REXX_METHOD(lv_setFullRowText,              lv_setFullRowText),
    REXX_METHOD(lv_setImageList,                lv_setImageList),
    REXX_METHOD(lv_setItemData,                 lv_setItemData),
    REXX_METHOD(lv_setItemPos,                  lv_setItemPos),
    REXX_METHOD(lv_setItemState,                lv_setItemState),
    REXX_METHOD(lv_setItemText,                 lv_setItemText),
    REXX_METHOD(lv_setSpecificState,            lv_setSpecificState),
    REXX_METHOD(lv_sortItems,                   lv_sortItems),
    REXX_METHOD(lv_stringWidthPx,               lv_stringWidthPx),
    REXX_METHOD(lv_setView,                     lv_setView),
    REXX_METHOD(lv_zTest,                       lv_zTest),

    // LvItem
    REXX_METHOD(lvi_init_cls,                   lvi_init_cls),
    REXX_METHOD(lvi_init,                       lvi_init),
    REXX_METHOD(lvi_unInit,                     lvi_unInit),
    REXX_METHOD(lvi_columnFormats,              lvi_columnFormats),
    REXX_METHOD(lvi_setColumnFormats,           lvi_setColumnFormats),
    REXX_METHOD(lvi_columns,                    lvi_columns),
    REXX_METHOD(lvi_setColumns,                 lvi_setColumns),
    REXX_METHOD(lvi_groupID,                    lvi_groupID),
    REXX_METHOD(lvi_setGroupID,                 lvi_setGroupID),
    REXX_METHOD(lvi_imageIndex,                 lvi_imageIndex),
    REXX_METHOD(lvi_setImageIndex,              lvi_setImageIndex),
    REXX_METHOD(lvi_indent,                     lvi_indent),
    REXX_METHOD(lvi_setIndent,                  lvi_setIndent),
    REXX_METHOD(lvi_index,                      lvi_index),
    REXX_METHOD(lvi_setIndex,                   lvi_setIndex),
    REXX_METHOD(lvi_itemData,                   lvi_itemData),
    REXX_METHOD(lvi_setItemData,                lvi_setItemData),
    REXX_METHOD(lvi_itemState,                  lvi_itemState),
    REXX_METHOD(lvi_setItemState,               lvi_setItemState),
    REXX_METHOD(lvi_itemStateMask,              lvi_itemStateMask),
    REXX_METHOD(lvi_setItemStateMask,           lvi_setItemStateMask),
    REXX_METHOD(lvi_mask,                       lvi_mask),
    REXX_METHOD(lvi_setMask,                    lvi_setMask),
    REXX_METHOD(lvi_overlayImageIndex,          lvi_overlayImageIndex),
    REXX_METHOD(lvi_setOverlayImageIndex,       lvi_setOverlayImageIndex),
    REXX_METHOD(lvi_stateImageIndex,            lvi_stateImageIndex),
    REXX_METHOD(lvi_setStateImageIndex,         lvi_setStateImageIndex),
    REXX_METHOD(lvi_text,                       lvi_text),
    REXX_METHOD(lvi_setText,                    lvi_setText),

    // LvSubItem
    REXX_METHOD(lvsi_init_cls,                  lvsi_init_cls),
    REXX_METHOD(lvsi_init,                      lvsi_init),
    REXX_METHOD(lvsi_unInit,                    lvsi_unInit),
    REXX_METHOD(lvsi_item,                      lvsi_item),
    REXX_METHOD(lvsi_setItem,                   lvsi_setItem),
    REXX_METHOD(lvsi_subItem,                   lvsi_subItem),
    REXX_METHOD(lvsi_setSubItem,                lvsi_setSubItem),
    REXX_METHOD(lvsi_mask,           	        lvsi_mask),
    REXX_METHOD(lvsi_setMask,                   lvsi_setMask),
    REXX_METHOD(lvsi_text,                      lvsi_text),
    REXX_METHOD(lvsi_setText,                   lvsi_setText),
    REXX_METHOD(lvsi_imageIndex,                lvsi_imageIndex),
    REXX_METHOD(lvsi_setImageIndex,             lvsi_setImageIndex),

    // LvFullRow
    REXX_METHOD(lvfr_init_cls,                  lvfr_init_cls),
    REXX_METHOD(lvfr_fromArray_cls,             lvfr_fromArray_cls),
    REXX_METHOD(lvfr_init,                      lvfr_init),
    REXX_METHOD(lvfr_unInit,                    lvfr_unInit),
    REXX_METHOD(lvfr_userData,                  lvfr_userData),
    REXX_METHOD(lvfr_setUserData,               lvfr_setUserData),
    REXX_METHOD(lvfr_addSubitem,                lvfr_addSubitem),
    REXX_METHOD(lvfr_insertSubitem,             lvfr_insertSubitem),
    REXX_METHOD(lvfr_item,                      lvfr_item),
    REXX_METHOD(lvfr_removeSubitem,             lvfr_removeSubitem),
    REXX_METHOD(lvfr_subitem,                   lvfr_subitem),
    REXX_METHOD(lvfr_subitems,                  lvfr_subitems),

    // ToolTip
    REXX_METHOD(tt_activate,                    tt_activate),
    REXX_METHOD(tt_addTool,                     tt_addTool),
    REXX_METHOD(tt_addToolEx,                   tt_addToolEx),
    REXX_METHOD(tt_addToolRect,                 tt_addToolRect),
    REXX_METHOD(tt_adjustRect,                  tt_adjustRect),
    REXX_METHOD(tt_delTool,                     tt_delTool),
    REXX_METHOD(tt_enumTools,                   tt_enumTools),
    REXX_METHOD(tt_getBubbleSize,               tt_getBubbleSize),
    REXX_METHOD(tt_getDelayTime,                tt_getDelayTime),
    REXX_METHOD(tt_getCurrentTool,              tt_getCurrentTool),
    REXX_METHOD(tt_getMargin,                   tt_getMargin),
    REXX_METHOD(tt_getMaxTipWidth,              tt_getMaxTipWidth),
    REXX_METHOD(tt_getText,                     tt_getText),
    REXX_METHOD(tt_getTipBkColor,               tt_getTipBkColor),
    REXX_METHOD(tt_getTipTextColor,             tt_getTipTextColor),
    REXX_METHOD(tt_getTitle,                    tt_getTitle),
    REXX_METHOD(tt_getToolCount,                tt_getToolCount),
    REXX_METHOD(tt_getToolInfo,                 tt_getToolInfo),
    REXX_METHOD(tt_hasCurrentTool,              tt_hasCurrentTool),
    REXX_METHOD(tt_hitTestInfo,                     tt_hitTestInfo),
    REXX_METHOD(tt_manageAtypicalTool,          tt_manageAtypicalTool),
    REXX_METHOD(tt_newToolRect,                 tt_newToolRect),
    REXX_METHOD(tt_pop,                         tt_pop),
    REXX_METHOD(tt_popUp,                       tt_popUp),
    REXX_METHOD(tt_setDelayTime,                tt_setDelayTime),
    REXX_METHOD(tt_setMargin,                   tt_setMargin),
    REXX_METHOD(tt_setMaxTipWidth,              tt_setMaxTipWidth),
    REXX_METHOD(tt_setTipBkColor,               tt_setTipBkColor),
    REXX_METHOD(tt_setTipTextColor,             tt_setTipTextColor),
    REXX_METHOD(tt_setTitle,                    tt_setTitle),
    REXX_METHOD(tt_setToolInfo,                 tt_setToolInfo),
    REXX_METHOD(tt_setWindowTheme,              tt_setWindowTheme),
    REXX_METHOD(tt_trackActivate,               tt_trackActivate),
    REXX_METHOD(tt_trackPosition,               tt_trackPosition),
    REXX_METHOD(tt_update,                      tt_update),
    REXX_METHOD(tt_updateTipText,               tt_updateTipText),

    // ToolInfo
    REXX_METHOD(ti_forHitTest_cls,              ti_forHitTest_cls),
    REXX_METHOD(ti_forID_cls,                   ti_forID_cls),
    REXX_METHOD(ti_init,                        ti_init),
    REXX_METHOD(ti_unInit,                      ti_unInit),
    REXX_METHOD(ti_flags,                       ti_flags),
    REXX_METHOD(ti_setFlags,                    ti_setFlags),
    REXX_METHOD(ti_rect,                        ti_rect),
    REXX_METHOD(ti_setRect,                     ti_setRect),
    REXX_METHOD(ti_resource,                    ti_resource),
    REXX_METHOD(ti_rexxHwnd,                    ti_rexxHwnd),
    REXX_METHOD(ti_rexxID,                      ti_rexxID),
    REXX_METHOD(ti_text,                        ti_text),
    REXX_METHOD(ti_setText,                     ti_setText),
    REXX_METHOD(ti_userData,                    ti_userData),
    REXX_METHOD(ti_setUserData,                 ti_setUserData),

    REXX_METHOD(ti_setTextMemoryIsAllocated,    ti_setTextMemoryIsAllocated),
    REXX_METHOD(ti_hitTestHelper,               ti_hitTestHelper),

    // TreeView
    REXX_METHOD(tv_delete,                      tv_delete),
    REXX_METHOD(tv_deleteAll,                   tv_deleteAll),
    REXX_METHOD(tv_expand,                      tv_expand),
    REXX_METHOD(tv_find,                        tv_find),
    REXX_METHOD(tv_getImageList,                tv_getImageList),
    REXX_METHOD(tv_getItemData,                 tv_getItemData),
    REXX_METHOD(tv_getItemHeight,               tv_getItemHeight),
    REXX_METHOD(tv_getItemRect,                 tv_getItemRect),
    REXX_METHOD(tv_getNextItem,                 tv_getNextItem),
    REXX_METHOD(tv_getSpecificItem,             tv_getSpecificItem),
    REXX_METHOD(tv_getStateImage,               tv_getStateImage),
    REXX_METHOD(tv_hitTestInfo,                 tv_hitTestInfo),
    REXX_METHOD(tv_insert,                      tv_insert),
    REXX_METHOD(tv_itemText,                    tv_itemText),
    REXX_METHOD(tv_itemInfo,                    tv_itemInfo),
    REXX_METHOD(tv_modify,                      tv_modify),
    REXX_METHOD(tv_removeItemData,              tv_removeItemData),
    REXX_METHOD(tv_selectItem,                  tv_selectItem),
    REXX_METHOD(tv_setImageList,                tv_setImageList),
    REXX_METHOD(tv_setItemData,                 tv_setItemData),
    REXX_METHOD(tv_setItemHeight,               tv_setItemHeight),
    REXX_METHOD(tv_setStateImage,               tv_setStateImage),
    REXX_METHOD(tv_sortChildrenCB,              tv_sortChildrenCB),
    REXX_METHOD(tv_test,                        tv_test),

    // Tab
    REXX_METHOD(tab_select,                     tab_select),
    REXX_METHOD(tab_selected,                   tab_selected),
    REXX_METHOD(tab_insert,                     tab_insert),
    REXX_METHOD(tab_addSequence,                tab_addSequence),
    REXX_METHOD(tab_addFullSeq,                 tab_addFullSeq),
    REXX_METHOD(tab_itemInfo,                   tab_itemInfo),
    REXX_METHOD(tab_modify,                     tab_modify),
    REXX_METHOD(tab_setItemSize,                tab_setItemSize),
    REXX_METHOD(tab_setPadding,                 tab_setPadding),
    REXX_METHOD(tab_getItemRect,                tab_getItemRect),
    REXX_METHOD(tab_calcRect,                   tab_calcRect),
    REXX_METHOD(tab_setImageList,               tab_setImageList),
    REXX_METHOD(tab_getImageList,               tab_getImageList),

    // DateTimePicker
    REXX_METHOD(dtp_getDateTime,                dtp_getDateTime),
    REXX_METHOD(dtp_setDateTime,                dtp_setDateTime),
    REXX_METHOD(dtp_closeMonthCal,              dtp_closeMonthCal),
    REXX_METHOD(dtp_getInfo,                    dtp_getInfo),
    REXX_METHOD(dtp_getIdealSize,               dtp_getIdealSize),
    REXX_METHOD(dtp_getMonthCal,                dtp_getMonthCal),
    REXX_METHOD(dtp_getMonthCalColor,           dtp_getMonthCalColor),
    REXX_METHOD(dtp_getMonthCalStyle,           dtp_getMonthCalStyle),
    REXX_METHOD(dtp_getRange,                   dtp_getRange),
    REXX_METHOD(dtp_setMonthCalColor,           dtp_setMonthCalColor),
    REXX_METHOD(dtp_setMonthCalStyle,           dtp_setMonthCalStyle),
    REXX_METHOD(dtp_setFormat,                  dtp_setFormat),
    REXX_METHOD(dtp_setRange,                   dtp_setRange),

    // MonthCalendar
    REXX_METHOD(get_mc_date,                    get_mc_date),
    REXX_METHOD(set_mc_date,                    set_mc_date),
    REXX_METHOD(mc_addRemoveStyle,              mc_addRemoveStyle),
    REXX_METHOD(mc_replaceStyle,                mc_replaceStyle),
    REXX_METHOD(mc_getStyle,                    mc_getStyle),
    REXX_METHOD(mc_getCalendarBorder,           mc_getCalendarBorder),
    REXX_METHOD(mc_getCalendarCount,            mc_getCalendarCount),
    REXX_METHOD(mc_getCALID,                    mc_getCALID),
    REXX_METHOD(mc_getColor,                    mc_getColor),
    REXX_METHOD(mc_getCurrentView,              mc_getCurrentView),
    REXX_METHOD(mc_getFirstDayOfWeek,           mc_getFirstDayOfWeek),
    REXX_METHOD(mc_getGridInfo,                 mc_getGridInfo),
    REXX_METHOD(mc_getMinRect,                  mc_getMinRect),
    REXX_METHOD(mc_getMonthRange,               mc_getMonthRange),
    REXX_METHOD(mc_getRange,                    mc_getRange),
    REXX_METHOD(mc_getSelectionRange,           mc_getSelectionRange),
    REXX_METHOD(mc_getToday,                    mc_getToday),
    REXX_METHOD(mc_hitTestInfo,                 mc_hitTestInfo),
    REXX_METHOD(mc_setCalendarBorder,           mc_setCalendarBorder),
    REXX_METHOD(mc_setCALID,                    mc_setCALID),
    REXX_METHOD(mc_setColor,                    mc_setColor),
    REXX_METHOD(mc_setCurrentView,              mc_setCurrentView),
    REXX_METHOD(mc_setDayState,                 mc_setDayState),
    REXX_METHOD(mc_setDayStateQuick,            mc_setDayStateQuick),
    REXX_METHOD(mc_setFirstDayOfWeek,           mc_setFirstDayOfWeek),
    REXX_METHOD(mc_setRange,                    mc_setRange),
    REXX_METHOD(mc_setSelectionRange,           mc_setSelectionRange),
    REXX_METHOD(mc_setToday,                    mc_setToday),
    REXX_METHOD(mc_sizeRectToMin,               mc_sizeRectToMin),

    // ProgressBar
    REXX_METHOD(pbc_getFullRange,               pbc_getFullRange),
    REXX_METHOD(pbc_setFullRange,               pbc_setFullRange),
    REXX_METHOD(pbc_setMarquee,                 pbc_setMarquee),

    // ReBar
    REXX_METHOD(rebar_deleteBand,               rebar_deleteBand),
    REXX_METHOD(rebar_getBandBorders,           rebar_getBandBorders),
    REXX_METHOD(rebar_getBandCount,             rebar_getBandCount),
    REXX_METHOD(rebar_getBandInfo,              rebar_getBandInfo),
    REXX_METHOD(rebar_getBandMargins,           rebar_getBandMargins),
    REXX_METHOD(rebar_getBarHeight,             rebar_getBarHeight),
    REXX_METHOD(rebar_getBarInfo,               rebar_getBarInfo),
    REXX_METHOD(rebar_getBkColor,               rebar_getBkColor),
    REXX_METHOD(rebar_getColorScheme,           rebar_getColorScheme),
    REXX_METHOD(rebar_getExtendedStyle,         rebar_getExtendedStyle),
    REXX_METHOD(rebar_getPalette,               rebar_getPalette),
    REXX_METHOD(rebar_getRect,                  rebar_getRect),
    REXX_METHOD(rebar_getRowCount,              rebar_getRowCount),
    REXX_METHOD(rebar_getRowHeight,             rebar_getRowHeight),
    REXX_METHOD(rebar_getTextColor,             rebar_getTextColor),
    REXX_METHOD(rebar_hitTestInfo,              rebar_hitTestInfo),
    REXX_METHOD(rebar_idToIndex,                rebar_idToIndex),
    REXX_METHOD(rebar_insertBand,               rebar_insertBand),
    REXX_METHOD(rebar_maximizeBand,             rebar_maximizeBand),
    REXX_METHOD(rebar_minimizeBand,             rebar_minimizeBand),
    REXX_METHOD(rebar_moveBand,                 rebar_moveBand),
    REXX_METHOD(rebar_pushChevron,              rebar_pushChevron),
    REXX_METHOD(rebar_setBandWidth,             rebar_setBandWidth),
    REXX_METHOD(rebar_setBandInfo,              rebar_setBandInfo),
    REXX_METHOD(rebar_setBarInfo,               rebar_setBarInfo),
    REXX_METHOD(rebar_setBkColor,               rebar_setBkColor),
    REXX_METHOD(rebar_setColorScheme,           rebar_setColorScheme),
    REXX_METHOD(rebar_setExtendedStyle,         rebar_setExtendedStyle),
    REXX_METHOD(rebar_setPalette,               rebar_setPalette),
    REXX_METHOD(rebar_setParent,                rebar_setParent),
    REXX_METHOD(rebar_setTextColor,             rebar_setTextColor),
    REXX_METHOD(rebar_setWindowTheme,           rebar_setWindowTheme),
    REXX_METHOD(rebar_showBand,                 rebar_showBand),
    REXX_METHOD(rebar_sizeToRect,               rebar_sizeToRect),

    // ReBarBandInfo
    REXX_METHOD(rbbi_init_cls          ,       rbbi_init_cls           ),
    REXX_METHOD(rbbi_unInit            ,       rbbi_unInit             ),
    REXX_METHOD(rbbi_init              ,       rbbi_init               ),
    REXX_METHOD(rbbi_bitmapBack        ,       rbbi_bitmapBack         ),
    REXX_METHOD(rbbi_setBitmapBack     ,       rbbi_setBitmapBack      ),
    REXX_METHOD(rbbi_chevronLocation   ,       rbbi_chevronLocation    ),
    REXX_METHOD(rbbi_setChevronLocation,       rbbi_setChevronLocation ),
    REXX_METHOD(rbbi_chevronState      ,       rbbi_chevronState       ),
    REXX_METHOD(rbbi_setChevronState   ,       rbbi_setChevronState    ),
    REXX_METHOD(rbbi_child             ,       rbbi_child              ),
    REXX_METHOD(rbbi_setChild          ,       rbbi_setChild           ),
    REXX_METHOD(rbbi_clrBack           ,       rbbi_clrBack            ),
    REXX_METHOD(rbbi_setClrBack        ,       rbbi_setClrBack         ),
    REXX_METHOD(rbbi_clrFore           ,       rbbi_clrFore            ),
    REXX_METHOD(rbbi_setClrFore        ,       rbbi_setClrFore         ),
    REXX_METHOD(rbbi_cx                ,       rbbi_cx                 ),
    REXX_METHOD(rbbi_setCx             ,       rbbi_setCx              ),
    REXX_METHOD(rbbi_cxHeader          ,       rbbi_cxHeader           ),
    REXX_METHOD(rbbi_setCxHeader       ,       rbbi_setCxHeader        ),
    REXX_METHOD(rbbi_cxIdeal           ,       rbbi_cxIdeal            ),
    REXX_METHOD(rbbi_setCxIdeal        ,       rbbi_setCxIdeal         ),
    REXX_METHOD(rbbi_cxMinChild        ,       rbbi_cxMinChild         ),
    REXX_METHOD(rbbi_setCxMinChild     ,       rbbi_setCxMinChild      ),
    REXX_METHOD(rbbi_cyChild           ,       rbbi_cyChild            ),
    REXX_METHOD(rbbi_setCyChild        ,       rbbi_setCyChild         ),
    REXX_METHOD(rbbi_cyIntegral        ,       rbbi_cyIntegral         ),
    REXX_METHOD(rbbi_setCyIntegral     ,       rbbi_setCyIntegral      ),
    REXX_METHOD(rbbi_cyMaxChild        ,       rbbi_cyMaxChild         ),
    REXX_METHOD(rbbi_setCyMaxChild     ,       rbbi_setCyMaxChild      ),
    REXX_METHOD(rbbi_cyMinChild        ,       rbbi_cyMinChild         ),
    REXX_METHOD(rbbi_setCyMinChild     ,       rbbi_setCyMinChild      ),
    REXX_METHOD(rbbi_id                ,       rbbi_id                 ),
    REXX_METHOD(rbbi_setId             ,       rbbi_setId              ),
    REXX_METHOD(rbbi_imageIndex        ,       rbbi_imageIndex         ),
    REXX_METHOD(rbbi_setImageIndex     ,       rbbi_setImageIndex      ),
    REXX_METHOD(rbbi_itemData          ,       rbbi_itemData           ),
    REXX_METHOD(rbbi_setItemData       ,       rbbi_setItemData        ),
    REXX_METHOD(rbbi_mask              ,       rbbi_mask               ),
    REXX_METHOD(rbbi_setMask           ,       rbbi_setMask            ),
    REXX_METHOD(rbbi_style             ,       rbbi_style              ),
    REXX_METHOD(rbbi_setStyle          ,       rbbi_setStyle           ),
    REXX_METHOD(rbbi_text              ,       rbbi_text               ),
    REXX_METHOD(rbbi_setText           ,       rbbi_setText            ),

    // ToolBar
    REXX_METHOD(tb_addBitmap,                  tb_addBitmap),
    REXX_METHOD(tb_addButtons,                 tb_addButtons),
    REXX_METHOD(tb_addString,                  tb_addString),
    REXX_METHOD(tb_autoSize,                   tb_autoSize),
    REXX_METHOD(tb_buttonCount,                tb_buttonCount),
    REXX_METHOD(tb_changeBitmap,               tb_changeBitmap),
    REXX_METHOD(tb_checkButton,                tb_checkButton),
    REXX_METHOD(tb_commandToIndex,             tb_commandToIndex),
    REXX_METHOD(tb_customize,                  tb_customize),
    REXX_METHOD(tb_enableButton,               tb_enableButton),
    REXX_METHOD(tb_getButton,                  tb_getButton),
    REXX_METHOD(tb_getButtonText,              tb_getButtonText),
    REXX_METHOD(tb_getButtonTextEx,            tb_getButtonTextEx),
    REXX_METHOD(tb_getDisabledImageList,       tb_getDisabledImageList),
    REXX_METHOD(tb_getExtendedStyle,           tb_getExtendedStyle),
    REXX_METHOD(tb_getHotImageList,            tb_getHotImageList),
    REXX_METHOD(tb_getImageList,               tb_getImageList),
    REXX_METHOD(tb_getPressedImageList,        tb_getPressedImageList),
    REXX_METHOD(tb_indexTocommand,             tb_indexTocommand),
    REXX_METHOD(tb_insertButton,               tb_insertButton),
    REXX_METHOD(tb_isButtonChecked,            tb_isButtonChecked),
    REXX_METHOD(tb_isButtonEnabled,            tb_isButtonEnabled),
    REXX_METHOD(tb_isButtonHidden,             tb_isButtonHidden),
    REXX_METHOD(tb_isButtonHighlighted,        tb_isButtonHighlighted),
    REXX_METHOD(tb_isButtonIndeterminate,      tb_isButtonIndeterminate),
    REXX_METHOD(tb_isButtonPressed,            tb_isButtonPressed),
    REXX_METHOD(tb_loadImages,                 tb_loadImages),
    REXX_METHOD(tb_setBitmapSize,              tb_setBitmapSize),
    REXX_METHOD(tb_setButtonText,              tb_setButtonText),
    REXX_METHOD(tb_setDisabledImageList,       tb_setDisabledImageList),
    REXX_METHOD(tb_setExtendedStyle,           tb_setExtendedStyle),
    REXX_METHOD(tb_setImageList,               tb_setImageList),
    REXX_METHOD(tb_setHotImageList,            tb_setHotImageList),
    REXX_METHOD(tb_setPressedImageList,        tb_setPressedImageList),

    // TbButton
    REXX_METHOD(tbb_init_cls           ,       tbb_init_cls    ),
    REXX_METHOD(tbb_init               ,       tbb_init        ),
    REXX_METHOD(tbb_bitmapID           ,       tbb_bitmapID    ),
    REXX_METHOD(tbb_setBitmapID        ,       tbb_setBitmapID ),
    REXX_METHOD(tbb_cmdID              ,       tbb_cmdID       ),
    REXX_METHOD(tbb_setCmdID           ,       tbb_setCmdID    ),
    REXX_METHOD(tbb_itemData           ,       tbb_itemData    ),
    REXX_METHOD(tbb_setItemData        ,       tbb_setItemData ),
    REXX_METHOD(tbb_state              ,       tbb_state       ),
    REXX_METHOD(tbb_setState           ,       tbb_setState    ),
    REXX_METHOD(tbb_style              ,       tbb_style       ),
    REXX_METHOD(tbb_setStyle           ,       tbb_setStyle    ),
    REXX_METHOD(tbb_text               ,       tbb_text        ),
    REXX_METHOD(tbb_setText            ,       tbb_setText     ),
    REXX_METHOD(tbb_assignBitmapID     ,       tbb_assignBitmapID),

    // TrackBar
    REXX_METHOD(trckbar_getRange,              trckbar_getRange),
    REXX_METHOD(trckbar_getSelRange,           trckbar_getSelRange),

    // UpDown
    REXX_METHOD(ud_deltaPosReply_cls,           ud_deltaPosReply_cls),
    REXX_METHOD(ud_getRange,                    ud_getRange),
    REXX_METHOD(ud_setRange,                    ud_setRange),
    REXX_METHOD(ud_getPosition,                 ud_getPosition),
    REXX_METHOD(ud_getBuddy,                    ud_getBuddy),
    REXX_METHOD(ud_setBuddy,                    ud_setBuddy),
    REXX_METHOD(ud_getAcceleration,             ud_getAcceleration),
    REXX_METHOD(ud_setAcceleration,             ud_setAcceleration),

    // ScrollBar
    REXX_METHOD(sb_getRange,                    sb_getRange),
    REXX_METHOD(sb_setRange,                    sb_setRange),
    REXX_METHOD(sb_getPosition,                 sb_getPosition),
    REXX_METHOD(sb_setPosition,                 sb_setPosition),

    // StatusBar
    REXX_METHOD(stb_getBorders,                 stb_getBorders),
    REXX_METHOD(stb_getIcon,                    stb_getIcon),
    REXX_METHOD(stb_getParts,                   stb_getParts),
    REXX_METHOD(stb_getRect,                    stb_getRect),
    REXX_METHOD(stb_getText,                    stb_getText),
    REXX_METHOD(stb_getTextLength,              stb_getTextLength),
    REXX_METHOD(stb_getTipText,                 stb_getTipText),
    REXX_METHOD(stb_isSimple,                   stb_isSimple),
    REXX_METHOD(stb_setBkColor,                 stb_setBkColor),
    REXX_METHOD(stb_setIcon,                    stb_setIcon),
    REXX_METHOD(stb_setMinHeight,               stb_setMinHeight),
    REXX_METHOD(stb_setParts,                   stb_setParts),
    REXX_METHOD(stb_setText,                    stb_setText),
    REXX_METHOD(stb_setTipText,                 stb_setTipText),
    REXX_METHOD(stb_simple,                     stb_simple),

    REXX_METHOD(dss_makeDayStateBuffer,         dss_makeDayStateBuffer),
    REXX_METHOD(dss_quickDayStateBuffer,        dss_quickDayStateBuffer),
    REXX_METHOD(ds_init,                        ds_init),
    REXX_METHOD(ds_value,                       ds_value),
    REXX_METHOD(vk_key2name,                    vk_key2name),

    // Menu classes methods
    REXX_METHOD(menu_menuInit_pvt,              menu_menuInit_pvt),
    REXX_METHOD(menu_uninit,                    menu_uninit),
    REXX_METHOD(menu_connectCommandEvent_cls,   menu_connectCommandEvent_cls),
    REXX_METHOD(menu_getHMenu,                  menu_getHMenu),
    REXX_METHOD(menu_wID,                       menu_wID),
    REXX_METHOD(menu_isValidItemID,             menu_isValidItemID),
    REXX_METHOD(menu_isValidMenu,               menu_isValidMenu),
    REXX_METHOD(menu_isValidMenuHandle,         menu_isValidMenuHandle),
    REXX_METHOD(menu_isSeparator,               menu_isSeparator),
    REXX_METHOD(menu_isCommandItem,             menu_isCommandItem),
    REXX_METHOD(menu_isPopup,                   menu_isPopup),
    REXX_METHOD(menu_isEnabled,                 menu_isEnabled),
    REXX_METHOD(menu_isDisabled,                menu_isDisabled),
    REXX_METHOD(menu_isChecked,                 menu_isChecked),
    REXX_METHOD(menu_getMenuHandle,             menu_getMenuHandle),
    REXX_METHOD(menu_releaseMenuHandle,         menu_releaseMenuHandle),
    REXX_METHOD(menu_destroy,                   menu_destroy),
    REXX_METHOD(menu_enable,                    menu_enable),
    REXX_METHOD(menu_disable,                   menu_disable),
    REXX_METHOD(menu_check,                     menu_check),
    REXX_METHOD(menu_unCheck,                   menu_unCheck),
    REXX_METHOD(menu_checkRadio,                menu_checkRadio),
    REXX_METHOD(menu_hilite,                    menu_hilite),
    REXX_METHOD(menu_unHilite,                  menu_unHilite),
    REXX_METHOD(menu_insertSeparator,           menu_insertSeparator),
    REXX_METHOD(menu_removeSeparator,           menu_removeSeparator),
    REXX_METHOD(menu_insertItem,                menu_insertItem),
    REXX_METHOD(menu_removeItem,                menu_removeItem),
    REXX_METHOD(menu_insertPopup,               menu_insertPopup),
    REXX_METHOD(menu_getPopup,                  menu_getPopup),
    REXX_METHOD(menu_removePopup,               menu_removePopup),
    REXX_METHOD(menu_deletePopup,               menu_deletePopup),
    REXX_METHOD(menu_getCount,                  menu_getCount),
    REXX_METHOD(menu_getItemState,              menu_getItemState),
    REXX_METHOD(menu_getItemType,               menu_getItemType),
    REXX_METHOD(menu_getID,                     menu_getID),
    REXX_METHOD(menu_setID,                     menu_setID),
    REXX_METHOD(menu_getHelpID,                 menu_getHelpID),
    REXX_METHOD(menu_setHelpID,                 menu_setHelpID),
    REXX_METHOD(menu_getMaxHeight,              menu_getMaxHeight),
    REXX_METHOD(menu_setMaxHeight,              menu_setMaxHeight),
    REXX_METHOD(menu_getText,                   menu_getText),
    REXX_METHOD(menu_setText,                   menu_setText),
    REXX_METHOD(menu_getAutoConnectStatus,      menu_getAutoConnectStatus),
    REXX_METHOD(menu_setAutoConnection,         menu_setAutoConnection),
    REXX_METHOD(menu_connectMenuEvent,          menu_connectMenuEvent),
    REXX_METHOD(menu_connectCommandEvent,       menu_connectCommandEvent),
    REXX_METHOD(menu_connectSomeCommandEvents,  menu_connectSomeCommandEvents),
    REXX_METHOD(menu_connectAllCommandEvents,   menu_connectAllCommandEvents),
    REXX_METHOD(menu_itemTextToMethodName,      menu_itemTextToMethodName),
    REXX_METHOD(menu_test,                      menu_test),

    REXX_METHOD(menuBar_attachTo,               menuBar_attachTo),
    REXX_METHOD(menuBar_detach,                 menuBar_detach),
    REXX_METHOD(menuBar_isAttached,             menuBar_isAttached),
    REXX_METHOD(menuBar_redraw,                 menuBar_redraw),
    REXX_METHOD(menuBar_replace,                menuBar_replace),

    REXX_METHOD(binMenu_init,                   binMenu_init),

    REXX_METHOD(sysMenu_init,                   sysMenu_init),
    REXX_METHOD(sysMenu_revert,                 sysMenu_revert),

    REXX_METHOD(popMenu_connectContextMenu_cls, popMenu_connectContextMenu_cls),
    REXX_METHOD(popMenu_init,                   popMenu_init),
    REXX_METHOD(popMenu_isAssigned,             popMenu_isAssigned),
    REXX_METHOD(popMenu_connectContextMenu,     popMenu_connectContextMenu),
    REXX_METHOD(popMenu_assignTo,               popMenu_assignTo),
    REXX_METHOD(popMenu_track,                  popMenu_track),
    REXX_METHOD(popMenu_show,                   popMenu_show),

    REXX_METHOD(scriptMenu_init,                scriptMenu_init),

    REXX_METHOD(userMenu_init,                  userMenu_init),
    REXX_METHOD(userMenu_complete,              userMenu_complete),

    REXX_METHOD(menuTemplate_isComplete,        menuTemplate_isComplete),
    REXX_METHOD(menuTemplate_addSeparator,      menuTemplate_addSeparator),
    REXX_METHOD(menuTemplate_addItem,           menuTemplate_addItem),
    REXX_METHOD(menuTemplate_addPopup,          menuTemplate_addPopup),

    // Mouse
    REXX_METHOD(mouse_new_cls,                  mouse_new_cls),
    REXX_METHOD(mouse_doubleClickTime_cls,      mouse_doubleClickTime_cls),
    REXX_METHOD(mouse_loadCursor_cls,           mouse_loadCursor_cls),
    REXX_METHOD(mouse_loadCursorFromFile_cls,   mouse_loadCursorFromFile_cls),
    REXX_METHOD(mouse_setDoubleClickTime_cls,   mouse_setDoubleClickTime_cls),
    REXX_METHOD(mouse_swapButton_cls,           mouse_swapButton_cls),
    REXX_METHOD(mouse_init,                     mouse_init),
    REXX_METHOD(mouse_capture,                  mouse_capture),
    REXX_METHOD(mouse_clipCursor,               mouse_clipCursor),
    REXX_METHOD(mouse_connectEvent,             mouse_connectEvent),
    REXX_METHOD(mouse_dragDetect,               mouse_dragDetect),
    REXX_METHOD(mouse_getClipCursor,            mouse_getClipCursor),
    REXX_METHOD(mouse_getCursorPos,             mouse_getCursorPos),
    REXX_METHOD(mouse_get_release_capture,      mouse_get_release_capture),
    REXX_METHOD(mouse_isButtonDown,             mouse_isButtonDown),
    REXX_METHOD(mouse_releaseClipCursor,        mouse_releaseClipCursor),
    REXX_METHOD(mouse_restoreCursor,            mouse_restoreCursor),
    REXX_METHOD(mouse_setCursor,                mouse_setCursor),
    REXX_METHOD(mouse_setCursorPos,             mouse_setCursorPos),
    REXX_METHOD(mouse_showCursor,               mouse_showCursor),
    REXX_METHOD(mouse_trackEvent,               mouse_trackEvent),
    REXX_METHOD(mouse_test,                     mouse_test),


    // Keyboard
    REXX_METHOD(kb_getAsyncKeyState_cls,        kb_getAsyncKeyState_cls),

// CreateWindows
    REXX_METHOD(cw_initCreateWindows,           cw_initCreateWindows),
    REXX_METHOD(cw_createReBarWindow,           cw_createReBarWindow),
    REXX_METHOD(cw_createStatusBarWindow,       cw_createStatusBarWindow),
    REXX_METHOD(cw_createToolBarWindow,         cw_createToolBarWindow),

// CustomDraw
    REXX_METHOD(cd_init,                        cd_init),
    REXX_METHOD(cd_customDrawControl,           cd_customDrawControl),

// LvCustomDrawSimple
    REXX_METHOD(lvcds_init_cls,                 lvcds_init_cls),
    REXX_METHOD(lvcds_init,                     lvcds_init),
    REXX_METHOD(lvcds_setClrText,               lvcds_setClrText),
    REXX_METHOD(lvcds_setClrTextBk,             lvcds_setClrTextBk),
    REXX_METHOD(lvcds_getDrawStage,             lvcds_getDrawStage),
    REXX_METHOD(lvcds_setFont,                  lvcds_setFont),
    REXX_METHOD(lvcds_getItem,                  lvcds_getItem),
    REXX_METHOD(lvcds_getItemData,              lvcds_getItemData),
    REXX_METHOD(lvcds_getID,                    lvcds_getID),
    REXX_METHOD(lvcds_setReply,                 lvcds_setReply),
    REXX_METHOD(lvcds_getSubItem,               lvcds_getSubItem),

// TvCustomDrawSimple
    REXX_METHOD(tvcds_init_cls,                 tvcds_init_cls),
    REXX_METHOD(tvcds_init,                     tvcds_init),
    REXX_METHOD(tvcds_setClrText,               tvcds_setClrText),
    REXX_METHOD(tvcds_setClrTextBk,             tvcds_setClrTextBk),
    REXX_METHOD(tvcds_getDrawStage,             tvcds_getDrawStage),
    REXX_METHOD(tvcds_setFont,                  tvcds_setFont),
    REXX_METHOD(tvcds_getID,                    tvcds_getID),
    REXX_METHOD(tvcds_getItem,                  tvcds_getItem),
    REXX_METHOD(tvcds_getItemData,              tvcds_getItemData),
    REXX_METHOD(tvcds_setReply,                 tvcds_setReply),
    REXX_METHOD(tvcds_getLevel,                 tvcds_getLevel),

// BrowseForFolder
    REXX_METHOD(bff_banner,                     bff_banner),
    REXX_METHOD(bff_setBanner,                  bff_setBanner),
    REXX_METHOD(bff_dlgTitle,                   bff_dlgTitle),
    REXX_METHOD(bff_setDlgTitle,                bff_setDlgTitle),
    REXX_METHOD(bff_hint,                       bff_hint),
    REXX_METHOD(bff_setHint,                    bff_setHint),
    REXX_METHOD(bff_initialThread,              bff_initialThread),
    REXX_METHOD(bff_owner,                      bff_owner),
    REXX_METHOD(bff_setOwner,                   bff_setOwner),
    REXX_METHOD(bff_options,                    bff_options),
    REXX_METHOD(bff_setOptions,                 bff_setOptions),
    REXX_METHOD(bff_root,                       bff_root),
    REXX_METHOD(bff_setRoot,                    bff_setRoot),
    REXX_METHOD(bff_startDir,                   bff_startDir),
    REXX_METHOD(bff_setStartDir,                bff_setStartDir),
    REXX_METHOD(bff_usePathForHint,             bff_usePathForHint),
    REXX_METHOD(bff_setUsePathForHint,          bff_setUsePathForHint),
    REXX_METHOD(bff_init,                       bff_init),
    REXX_METHOD(bff_uninit,                     bff_uninit),
    REXX_METHOD(bff_getDisplayName,             bff_getDisplayName),
    REXX_METHOD(bff_getFolder,                  bff_getFolder),
    REXX_METHOD(bff_getItemIDList,              bff_getItemIDList),
    REXX_METHOD(bff_initCOM,                    bff_initCOM),
    REXX_METHOD(bff_releaseCOM,                 bff_releaseCOM),
    REXX_METHOD(bff_releaseItemIDList,          bff_releaseItemIDList),
    REXX_METHOD(bff_test,                       bff_test),

    REXX_METHOD(sfb_getFolder,                  sfb_getFolder),

// CommonItemDialog
    REXX_METHOD(cid_options,                    cid_options),
    REXX_METHOD(cid_setOptions,                 cid_setOptions),

    REXX_METHOD(cid_uninit,                     cid_uninit),
    REXX_METHOD(cid_init,                    	cid_init),

    REXX_METHOD(cid_addPlace,                   cid_addPlace),
    REXX_METHOD(cid_advise,                     cid_advise),
    REXX_METHOD(cid_clearClientData,            cid_clearClientData),
    REXX_METHOD(cid_close,                      cid_close),
    REXX_METHOD(cid_getCurrentSelection,        cid_getCurrentSelection),
    REXX_METHOD(cid_getFileName,                cid_getFileName),
    REXX_METHOD(cid_getFileTypeIndex,           cid_getFileTypeIndex),
    REXX_METHOD(cid_getFolder,                  cid_getFolder),
    REXX_METHOD(cid_getResult,                  cid_getResult),
    REXX_METHOD(cid_initCOM,                    cid_initCOM),
    REXX_METHOD(cid_isReleased,                 cid_isReleased),
    REXX_METHOD(cid_release,                    cid_release),
    REXX_METHOD(cid_releaseCOM,                 cid_releaseCOM),
    REXX_METHOD(cid_setClientGuid,              cid_setClientGuid),
    REXX_METHOD(cid_setDefaultExtension,        cid_setDefaultExtension),
    REXX_METHOD(cid_setDefaultFolder,           cid_setDefaultFolder),
    REXX_METHOD(cid_setFileName,                cid_setFileName),
    REXX_METHOD(cid_setFileNameLabel,           cid_setFileNameLabel),
    REXX_METHOD(cid_setFileTypeIndex,           cid_setFileTypeIndex),
    REXX_METHOD(cid_setFileTypes,               cid_setFileTypes),
    REXX_METHOD(cid_setFilter,                  cid_setFilter),
    REXX_METHOD(cid_setFolder,                  cid_setFolder),
    REXX_METHOD(cid_setCancelButtonLabel,       cid_setCancelButtonLabel),
    REXX_METHOD(cid_setOkButtonLabel,           cid_setOkButtonLabel),
    REXX_METHOD(cid_setTitle,                   cid_setTitle),
    REXX_METHOD(cid_show,                       cid_show),
    REXX_METHOD(cid_unadvise,                   cid_unadvise),

    REXX_METHOD(ofd_init,                    	ofd_init),
    REXX_METHOD(ofd_getResults,                 ofd_getResults),

    REXX_METHOD(sfd_init,                    	sfd_init),
    REXX_METHOD(sfd_setSaveAsItem,              sfd_setSaveAsItem),

    REXX_METHOD(cid_initCustomizations,         cid_initCustomizations),
    REXX_METHOD(cid_addCheckButton,             cid_addCheckButton),
    REXX_METHOD(cid_addComboBox,                cid_addComboBox),
    REXX_METHOD(cid_addControlItem,             cid_addControlItem),
    REXX_METHOD(cid_addEditBox,                 cid_addEditBox),
    REXX_METHOD(cid_addMenu,                    cid_addMenu),
    REXX_METHOD(cid_addPushButton,              cid_addPushButton),
    REXX_METHOD(cid_addRadioButtonList,         cid_addRadioButtonList),
    REXX_METHOD(cid_addSeparator,               cid_addSeparator),
    REXX_METHOD(cid_addText,                    cid_addText),
    REXX_METHOD(cid_enableOpenDropDown,         cid_enableOpenDropDown),
    REXX_METHOD(cid_endVisualGroup,             cid_endVisualGroup),
    REXX_METHOD(cid_getCheckButtonState,        cid_getCheckButtonState),
    REXX_METHOD(cid_getControlItemState,        cid_getControlItemState),
    REXX_METHOD(cid_getControlState,            cid_getControlState),
    REXX_METHOD(cid_getEditBoxText,             cid_getEditBoxText),
    REXX_METHOD(cid_getSelectedControlItem,     cid_getSelectedControlItem),
    REXX_METHOD(cid_makeProminent,              cid_makeProminent),
    REXX_METHOD(cid_removeAllControlItems,      cid_removeAllControlItems),
    REXX_METHOD(cid_removeControlItem,          cid_removeControlItem),
    REXX_METHOD(cid_setCheckButtonState,        cid_setCheckButtonState),
    REXX_METHOD(cid_setControlItemState,        cid_setControlItemState),
    REXX_METHOD(cid_setControlItemText,         cid_setControlItemText),
    REXX_METHOD(cid_setControlLabel,            cid_setControlLabel),
    REXX_METHOD(cid_setControlState,            cid_setControlState),
    REXX_METHOD(cid_setEditBoxText,             cid_setEditBoxText),
    REXX_METHOD(cid_setSelectedControlItem,     cid_setSelectedControlItem),
    REXX_METHOD(cid_startVisualGroup,           cid_startVisualGroup),

    REXX_METHOD(cde_init,                       cde_init),

    REXX_METHOD(sif_init,                       sif_init),

    REXX_LAST_METHOD()
};

RexxPackageEntry oodialog_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_1_0,              // needs at least the 4.1.0 interpreter
    "ooDialog",                          // name of the package
    "4.2.0",                             // package information
    ooDialogLoad,                        // package load function
    ooDialogUnload,                      // package unload function
    oodialog_functions,                  // the exported functions
    oodialog_methods                     // the exported methods
};

// package loading stub.
OOREXX_GET_PACKAGE(oodialog);
