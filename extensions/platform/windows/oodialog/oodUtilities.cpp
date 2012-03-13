/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2012 Rexx Language Association. All rights reserved.    */
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
 * oodUtilities.cpp
 *
 * Contains utility classes, including the .DlgUtil class.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>          // For printf()
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodResourceIDs.hpp"

/**
 * Generic method implementations that may be used by different classes
 */
#define GENERIC_METHODS        "GenericMethods"

/**
 * Resolve a resource ID using the .ConstDir.
 */
RexxMethod1(int32_t, global_resolveSymbolicID, RexxObjectPtr, id)
{
    return oodGlobalID(context, id, 1, true);
}


/** ListBox::setTabulators()
 *  PlainBaseDialog::setListTabulators()
 *  CategoryDialog::setCategoryListTabulators()
 *
 *  Sets the tab stop positions in a list-box.
 *
 *  This is generic implementation used by several different classes.  The
 *  resourceID and categoryId arguments are not always present.
 *
 *  @param resourceID  The resource ID (may be symbolic) of the list-box.
 *
 *  @param tabstop     The tab stop position.  This argument may repeat any
 *                     number of times.  Each argument is the next succesive tab
 *                     stop.  See the notes below for a fuller explanation.
 *
 *  @param categoryID  For a CategoryDialog, the catalog page that contains the
 *                     ListBox.
 *
 *  @return 0 on success, -1 for an invalid resource ID, and 1 for an API
 *          failure.
 *
 *  @note  The tab stop units are dialog template units. The tab stops must be
 *         listed in ascending order. You can't place a tab stop behind a
 *         previous tab stop.
 *
 *         If no tab stop is specified, than that signals the list-box to place
 *         tab stops equidistant at the default of 2 dialog units.  If 1 tab
 *         stop is specified, then equidistant tab stops are placed at the
 *         distance specified.  Othewise, a tab stop is placed at each position
 *         specified.
 */
RexxMethod2(int32_t, generic_setListTabulators, ARGLIST, args, OSELF, self)
{
    HWND hControl = NULL;
    int32_t  rc = -1;
    int32_t id;
    uint32_t *tabs = NULL;
    oodClass_t objects[] = {oodCategoryDialog, oodPlainBaseDialog, oodListBox};

    size_t count = context->ArrayItems((RexxArrayObject) args);
    size_t tabStart = 1;

    // Determine which object has invoked this method and parse the argument
    // list.  The object class determines how to get the handle to the listbox,
    // the count of tab stops, and at which arg position the tab stops start.
    switch ( oodClass(context, self, objects, sizeof(objects) / sizeof(oodClass_t))  )
    {
        case oodListBox :
        {
            hControl = controlToHCtrl(context, self);
        } break;

        case oodPlainBaseDialog :
        {
            if ( count < 1 )
            {
                missingArgException(context->threadContext, 1);
                goto done_out;
            }

            RexxObjectPtr resourceID = context->ArrayAt(args, 1);
            if ( ! oodSafeResolveID(&id, context, self, resourceID, -1, 1, true) )
            {
                goto done_out;
            }

            pCPlainBaseDialog pcpbd = dlgToCSelf(context, self);
            hControl = GetDlgItem(pcpbd->hDlg, (int)id);
            tabStart = 2;
            count--;

        } break;

        case oodCategoryDialog :
        {
            if ( count < 2 )
            {
                missingArgException(context->threadContext, (count == 1 ? 2 : 1));
                goto done_out;
            }

            RexxObjectPtr resourceID = context->ArrayAt(args, 1);
            if ( ! oodSafeResolveID(&id, context, self, resourceID, -1, 1, true) )
            {
                goto done_out;
            }

            // CatagoryDialogs have this basic construct to hold the dialog
            // handles for each page:
            //  catalogDialog~catalog['handles'][categoryID] == hwndDialog

            RexxDirectoryObject catalog = (RexxDirectoryObject)context->SendMessage0(self, "CATALOG");
            if ( catalog == NULLOBJECT )
            {
                ooDialogInternalException(context, __FUNCTION__, __LINE__, __DATE__, __FILE__);
                goto done_out;
            }

            RexxArrayObject handles = (RexxArrayObject)context->DirectoryAt(catalog, "handles");
            if ( handles == NULLOBJECT || ! context->IsArray(handles) )
            {
                ooDialogInternalException(context, __FUNCTION__, __LINE__, __DATE__, __FILE__);
                goto done_out;
            }
            RexxObjectPtr categoryID = context->ArrayAt(args, count);
            RexxObjectPtr rxHwnd = context->SendMessage1(handles, "AT", categoryID);
            if ( context->CheckCondition() )
            {
                goto done_out;
            }

            // From here on out, we might get NULL for window handles.  We just
            // ignore that and let LB_SETTABSTOPS fail;
            HWND hwnd = (HWND)string2pointer(context->ObjectToStringValue(rxHwnd));

            hControl = GetDlgItem(hwnd, (int)id);
            tabStart = 2;
            count -= 2;

        } break;

        default :
            ooDialogInternalException(context, __FUNCTION__, __LINE__, __DATE__, __FILE__);
            goto done_out;
            break;
    }

    if ( count > 0 )
    {
        tabs = (uint32_t *)malloc(sizeof(uint32_t *) * count);
        if ( tabs == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }

        uint32_t *p = tabs;
        for ( size_t i = 0; i < count; i++, p++, tabStart++ )
        {
            RexxObjectPtr tab = context->ArrayAt(args, tabStart);
            if ( tab == NULLOBJECT )
            {
                missingArgException(context->threadContext, tabStart);
                goto done_out;
            }
            if ( ! context->ObjectToUnsignedInt32(tab, p) )
            {
                notPositiveArgException(context->threadContext, tabStart, tab);
                goto done_out;
            }
        }
    }

    // LB_SETTABSTOPS returns true on success, otherwise false.  Reverse the
    // return so that 0 is returned for success and 1 for failure.
    rc = (SendMessage(hControl, LB_SETTABSTOPS, (WPARAM)count, (LPARAM)tabs) == 0);

done_out:
    safeFree(tabs);
    return rc;
}


/**
 *  Methods for the .ApplicationManager class.
 */
#define APPLICATIONMANAGER_CLASS        "ApplicationManager"

#define CONST_DIR_USAGE_OPTS            "[O]nly, [F]irst, [L]ast, or [N]ever"
#define CONST_DIR_SYMBOL_SRC_OPTS       "a collection class object, or a file name"
#define SETDEFAULTS_INDEXES             "constDirUsage, symbolSrc, autoDetction, fontName, or fontSize"


inline bool checkApplicationMagic(RexxMethodContext *c, RexxObjectPtr magic)
{
    if ( c->IsBuffer(magic) )
    {
        uint32_t *d = (uint32_t *)c->BufferData((RexxBufferObject)magic);
        if ( d != NULL && *d == APPLICATION_MANAGER_MAGIC )
        {
            return true;
        }
    }
    return false;
}

void putDefaultSymbols(RexxMethodContext *c, RexxDirectoryObject constDir)
{
    c->DirectoryPut(constDir, c->Int32(IDC_STATIC),       "IDC_STATIC");       // -1
    c->DirectoryPut(constDir, c->Int32(IDOK      ),       "IDOK");             // 1
    c->DirectoryPut(constDir, c->Int32(IDCANCEL  ),       "IDCANCEL");         // 2
    c->DirectoryPut(constDir, c->Int32(IDABORT   ),       "IDABORT");          //  ...
    c->DirectoryPut(constDir, c->Int32(IDRETRY   ),       "IDRETRY");
    c->DirectoryPut(constDir, c->Int32(IDIGNORE  ),       "IDIGNORE");
    c->DirectoryPut(constDir, c->Int32(IDYES     ),       "IDYES");
    c->DirectoryPut(constDir, c->Int32(IDNO      ),       "IDNO");
    c->DirectoryPut(constDir, c->Int32(IDCLOSE   ),       "IDCLOSE");
    c->DirectoryPut(constDir, c->Int32(IDHELP    ),       "IDHELP");           // 9
    c->DirectoryPut(constDir, c->Int32(IDTRYAGAIN),       "IDTRYAGAIN");       // 10
    c->DirectoryPut(constDir, c->Int32(IDCONTINUE),       "IDCONTINUE");       // 11
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_OODIALOG), "IDI_DLG_OODIALOG"); // This is 12
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_APPICON),  "IDI_DLG_APPICON");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_APPICON2), "IDI_DLG_APPICON2");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_OOREXX),   "IDI_DLG_OOREXX");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_DEFAULT),  "IDI_DLG_DEFAULT");
}

static RexxObjectPtr setConstDirUsage(RexxMethodContext *c, CSTRING _mode, size_t argPos, CSTRING index)
{
    CSTRING cMode;
    oodConstDir_t mode;

    switch ( toupper(*_mode) )
    {
        case 'O' :
            mode = globalOnly;
            cMode = "use only";
            break;
        case 'F' :
            mode = globalFirst;
            cMode = "use first";
            break;
        case 'L' :
            mode = globalLast;
            cMode = "use last";
            break;
        case 'N' :
            mode = globalNever;
            cMode = "use never";
            break;
        default :
            if ( index == NULL )
            {
                wrongArgOptionException(c->threadContext, argPos, CONST_DIR_USAGE_OPTS, _mode);
            }
            else
            {
                directoryIndexExceptionList(c->threadContext, argPos, index, CONST_DIR_USAGE_OPTS, _mode);
            }
            return TheFalseObj;
    }

    TheConstDirUsage = mode;
    c->DirectoryPut(TheDotLocalObj, c->String(cMode), "CONSTDIRUSAGE");

    return TheTrueObj;
}

static RexxObjectPtr addToConstDir(RexxMethodContext *c, pCApplicationManager pcam, RexxObjectPtr src,
                                   size_t argPos, CSTRING index)
{
    RexxObjectPtr result = TheTrueObj;

    if ( c->HasMethod(src, "SUPPLIER") )
    {
        c->SendMessage1(TheConstDir, "PUTALL", src);
    }
    else if( c->IsString(src) )
    {
        result = c->SendMessage1(pcam->rexxSelf, "PARSEINCLUDEFILE", src);
    }
    else
    {
        if ( index == NULL )
        {
            wrongArgValueException(c->threadContext, 1, CONST_DIR_SYMBOL_SRC_OPTS, src);
        }
        else
        {
            directoryIndexExceptionList(c->threadContext, argPos, index, CONST_DIR_SYMBOL_SRC_OPTS, src);
        }
        result = TheFalseObj;
    }
    return result;
}


static RexxObjectPtr setGlobalFont(RexxMethodContext *c, CSTRING name, uint32_t size, size_t argPos)
{
    RexxObjectPtr result = TheTrueObj;

    if ( strlen(name) > (MAX_DEFAULT_FONTNAME - 1) )
    {
        stringTooLongException(c->threadContext, argPos, MAX_DEFAULT_FONTNAME, strlen(name));
        result = TheFalseObj;
    }
    else
    {
        pCPlainBaseDialogClass pcpbdc = (pCPlainBaseDialogClass)c->ObjectToCSelf(ThePlainBaseDialogClass);

        if ( strlen(name) != 0 )
        {
            strcpy(pcpbdc->fontName, name);
        }

        if ( size != 0 )
        {
            pcpbdc->fontSize = size;
        }
    }
    return result;
}


static RexxObjectPtr setDefaults(RexxMethodContext *c, pCApplicationManager pcam, RexxDirectoryObject prefs)
{
    RexxObjectPtr result = TheTrueObj;
    size_t        count = 0;

    RexxObjectPtr val = c->DirectoryAt(prefs, "CONSTDIRUSAGE");
    if ( val != NULLOBJECT )
    {
        result = setConstDirUsage(c, c->ObjectToStringValue(val), 1, "CONSTDIRUSAGE");
        if ( result == TheFalseObj )
        {
            goto done_out;
        }
        count++;
    }

    val = c->DirectoryAt(prefs, "SYMBOLSRC");
    if ( val != NULLOBJECT )
    {
        result = addToConstDir(c, pcam, val, 1, "SYMBOLSRC");
        if ( result == TheFalseObj )
        {
            goto done_out;
        }
        count++;
    }

    val = c->DirectoryAt(prefs, "AUTODETECTION");
    if ( val != NULLOBJECT )
    {
        logical_t on;
        if ( ! c->Logical(val, &on) )
        {
            directoryIndexExceptionList(c->threadContext, 1, "AUTODETECTION", ".true or .false", val);
            goto err_out;
        }
        pcam->autoDetect = on ? true : false;
        count++;
    }

    RexxObjectPtr fontName = c->DirectoryAt(prefs, "FONTNAME");
    RexxObjectPtr fontSize = c->DirectoryAt(prefs, "FONTSIZE");

    if ( ! (fontName == NULLOBJECT && fontSize == NULLOBJECT) )
    {
        CSTRING  name;
        uint32_t size;

        if ( fontName == NULLOBJECT )
        {
            name = "";
        }
        else
        {
            name = c->ObjectToStringValue(fontName);
            if ( strlen(name) == 0 )
            {
                directoryIndexExceptionMsg(c->threadContext, 1, "FONTNAME", "can not be the empty string", fontName);
                goto err_out;
            }
            if ( strlen(name) > (MAX_DEFAULT_FONTNAME - 1))
            {
                // We do this check here rather than letting setGlobalFont()
                // handle it to give a better message for the condition.
                directoryIndexExceptionMsg(c->threadContext, 1, "FONTNAME", "can not be the empty string", fontName);
                goto err_out;
            }
        }

        if ( fontSize == NULLOBJECT )
        {
            size = 0;
        }
        else if ( ! c->UnsignedInt32(fontSize, &size) )
        {
            directoryIndexExceptionMsg(c->threadContext, 1, "FONTSIZE", "must be a positive whole number", fontSize);
            goto err_out;
        }

        result = setGlobalFont(c, name, size, 1);
        count++;
    }

    if ( count == 0 )
    {
        missingIndexesInDirectoryException(c->threadContext, 1, SETDEFAULTS_INDEXES);
        goto err_out;
    }

done_out:
    return result;

err_out:
    return TheFalseObj;
}

/** ApplicationManager::init()
 *
 *  Initializes a new ApplicationManager object.  This is done internally by the
 *  framework and is designed to prevent the Rexx programmer from instantiating
 *  an application manager object.
 *
 *  A single object is instantiated by the framework and placed in the .local
 *  environment as the .application object.
 */
RexxMethod2(RexxObjectPtr, app_init, RexxObjectPtr, magic, OSELF, self)
{
    if ( ! checkApplicationMagic(context, magic) )
    {
        severeErrorException(context->threadContext, BAD_APPLICATION_MSG);
        return NULLOBJECT;
    }

    RexxBufferObject obj = context->NewBuffer(sizeof(CApplicationManager));
    if ( obj == NULLOBJECT )
    {
        goto done_out;
    }

    pCApplicationManager pcam = (pCApplicationManager)context->BufferData(obj);
    memset(pcam, 0, sizeof(CApplicationManager));

    pcam->autoDetect = true;
    pcam->rexxSelf = self;

    context->SetObjectVariable("CSELF", obj);

    TheConstDir = context->NewDirectory();
    context->DirectoryPut(TheDotLocalObj, TheConstDir, "CONSTDIR");

    context->SendMessage1(self, "CONSTDIR=", TheConstDir);
    putDefaultSymbols(context, TheConstDir);

    // The default for the the const dir usage is never.
    setConstDirUsage(context, "N", 0, NULL);

done_out:
    return NULLOBJECT;
}


/** ApplicationManger::useGlobalConstDir()
 *
 *
 */
RexxMethod3(RexxObjectPtr, app_useGlobalConstDir, CSTRING, _mode, OPTIONAL_RexxObjectPtr, symbolSrc, CSELF, pCSelf)
{
    pCApplicationManager pcam = (pCApplicationManager)pCSelf;

    RexxObjectPtr result = setConstDirUsage(context, _mode, 1, NULL);

    if ( result == TheTrueObj && argumentExists(2) )
    {
        result = addToConstDir(context, pcam, symbolSrc, 2, NULL);
    }

    return result;
}


/** ApplicationManager::addToConstDir()
 *
 *
 */
RexxMethod2(RexxObjectPtr, app_addToConstDir, RexxObjectPtr, src, CSELF, pCSelf)
{
    return addToConstDir(context, (pCApplicationManager)pCSelf, src, 1, NULL);
}


/** ApplicationManager::autoDetection()
 *
 *  Sets the global auto detection to that specified.
 *
 *  @param on  If true sets the default auto detection to on, if false the
 *             default is set to off.  If omitted, auto detection is turned off.
 *
 *  @return    Returns 0 always.
 *
 *  @notes  By default auto detection is on in all dialogs.  The application
 *          manager can change that default for all dialogs instantiated in the
 *          current application.
 *
 */
RexxMethod2(uint32_t, app_autoDetection, OPTIONAL_logical_t, on, CSELF, pCSelf)
{
    pCApplicationManager pcam = (pCApplicationManager)pCSelf;
    pcam->autoDetect = on ? true : false;
    return 0;
}


/** ApplicationManager::initAutoDetection()
 *
 *  Called internally by the framework to set the auto detection for the dialog
 *  specified.  Not meant to be documented for the user.
 *
 *  This works by setting the auto detect attribute of the dialog to the same
 *  value as the application manager has.  The application manager's auto detect
 *  value is by default true, the same as the default for the dialog.
 *
 *  @params  dlg  The dialog whose auto detect attribute is to be set.
 *
 *  @return  Zero, always.
 *
 *  @remarks  We could probably skip the requiredDlgCSelf(), but the method is
 *            public so theoretically a user could invoke it even if it is not
 *            documented.
 */
RexxMethod2(uint32_t, app_initAutoDetection, RexxObjectPtr, dlg, CSELF, pCSelf)
{
    pCApplicationManager pcam  = (pCApplicationManager)pCSelf;

    pCPlainBaseDialog pcpbd = requiredDlgCSelf(context, dlg, oodPlainBaseDialog, 1, NULL);
    if ( pcpbd != NULL )
    {
        pcpbd->autoDetect = pcam->autoDetect;
    }
    return 0;
}


/** ApplicationManager::defaultFont()
 *
 *  Sets the global font.
 *
 *  @param  name  [optional]  The new font, the name of the font.  If omitted
 *                the global font name is not changed.  However, if both name
 *                and size are omitted the font is set back to the default font
 *                and size. (See remarks.)
 *
 *  @param  size  [optional]  The new size of the global font.  If omitted the
 *                global font size is not changed.  However, if both name and
 *                szie are omitted the font is set back to the default font and
 *                size. (See remarks.)
 *
 *  @return True on success, otherwise false.
 *
 *  @notes  The programmer can change the global font back to the default font
 *          and font size by omitting both arguments.
 *
 *          The length of the font name must be less than 256 characters,
 *          otherwise a syntax condition is raised.  This is the only possible
 *          cause of failure.
 *
 *          Since a syntax condition has been raised on error, false will never
 *          actually be returned.
 */
RexxMethod3(RexxObjectPtr, app_defaultFont, OPTIONAL_CSTRING, name, OPTIONAL_uint32_t, size, CSELF, pCSelf)
{
    pCApplicationManager pcam = (pCApplicationManager)pCSelf;

    if ( argumentOmitted(1) && argumentOmitted(2) )
    {
        name = DEFAULT_FONTNAME;
        size = DEFAULT_FONTSIZE;
    }
    else if ( argumentOmitted(1) )
    {
        name = "";
    }
    else if ( argumentOmitted(2) )
    {
        size = 0;
    }

    return setGlobalFont(context, name, size, 1);
}

/** ApplicationManager::setDefaults()
 *
 *  Sets a number of default values for the application in a single method call.
 *  Each default being set could also be set in a single-purpose method.  If the
 *  first argument is a .directory object, then the defaults values are
 *  specified by indexes of the .directory.  Otherwise, the opitonal arguments
 *  specify individual settings.
 *
 */
RexxMethod6(RexxObjectPtr, app_setDefaults, OPTIONAL_RexxObjectPtr, usage, OPTIONAL_RexxObjectPtr, symbolSrc,
            OPTIONAL_logical_t, autoDetect, OPTIONAL_CSTRING, fontName, OPTIONAL_uint32_t, fontSize, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    pCApplicationManager pcam  = (pCApplicationManager)pCSelf;
    RexxObjectPtr        result = TheTrueObj;

    if ( argumentExists(1) && c->IsDirectory(usage) )
    {
        return setDefaults(context, pcam, (RexxDirectoryObject)usage);
    }

    if ( argumentExists(1) )
    {
        result = setConstDirUsage(context, context->ObjectToStringValue(usage), 1, NULL);
        if ( result == TheFalseObj )
        {
            goto done_out;
        }
    }

    if ( argumentExists(2) )
    {
        result = addToConstDir(context, pcam, symbolSrc, 2, NULL);
        if ( result == TheFalseObj )
        {
            goto done_out;
        }
    }

    if ( argumentExists(3) )
    {
        pcam->autoDetect = autoDetect ? true : false;
    }

    if ( argumentExists(4) || argumentExists(5) )
    {
        if ( argumentOmitted(4) )
        {
            fontName = "";
        }
        else if (strlen(fontName) == 0)
        {
            context->RaiseException1(Rexx_Error_Invalid_argument_null, context->WholeNumber(4));
        }

        // If the fontSize argument is omitted, its value is already 0.

        result = setGlobalFont(c, fontName, fontSize, 1);
    }

done_out:
    return result;
}


/**
 * Defines, structs, and methods for the DlgUtil class.
 */
#define DLGUTIL_CLASS      "DlgUtil"


RexxObjectPtr SPI_getWorkArea(RexxMethodContext *c)
{
    oodResetSysErrCode(c->threadContext);

    RECT r = {0};
    if ( ! SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0) )
    {
        oodSetSysErrCode(c->threadContext);
    }

    return rxNewRect(c, &r);
}


/** DlgUtil::init() [class method]
 *
 * The .DlgUtil class init() method.  It executes when the .DlgUtil class is
 * constructed, which is done during the processing of the ::requires directive
 * for ooDialog.cls.
 *
 * We use this to create the an instance of the ApplicationManager and place it
 * in the .local directory.  To do this, at this point, the ApplicationManager
 * object must have already been constructed.  This in turn relies on the order
 * of the classes in ooDialog.cls, being: .ResourceUtils, .ApplicationManager,
 * and then .DlgUtils.
 *
 * @return No return.
 */
RexxMethod0(RexxObjectPtr, dlgutil_init_cls)
{
    RexxClassObject appClass = context->FindContextClass(APPLICATIONMANAGER_CLASS);
    if ( appClass == NULLOBJECT )
    {
        context->RaiseException1(Rexx_Error_Execution_noclass, context->String(APPLICATIONMANAGER_CLASS));
    }
    else
    {
        RexxMethodContext *c = context;

        RexxBufferObject m = c->NewBuffer(sizeof(uint32_t *));
        *((uint32_t *)c->BufferData(m)) = APPLICATION_MANAGER_MAGIC;

        TheApplicationObj = context->SendMessage1(appClass, "NEW", m);
        if ( TheApplicationObj != NULLOBJECT )
        {
            context->DirectoryPut(TheDotLocalObj, TheApplicationObj, "APPLICATION");
        }
    }
    return NULLOBJECT;
}

/** DlgUtil::comCtl32Version()  [class method]
 *
 * Returns the comctl32.dll version that ooDialog is currently using in one of
 * the 4 formats listed below.
 *
 * @param format  [optional] Keyword indicating the format, only the first
 *                letter is needed, case not significant.  The default is short.
 *                Incorrect strings are ignored and the default is used.
 *
 * The formats are:
 *   Full:  A complete string including the DLL name, version number and the
 *          minimum Windows OS name that the DLL is found on.
 *
 *   Number:  The version number part of the full string.
 *   Short:   Same as number.
 *   OS:      The OS name part of the full string.
 */
RexxMethod1(RexxStringObject, dlgutil_comctl32Version_cls, OPTIONAL_CSTRING, format)
{
    const char *ver;
    char f = argumentOmitted(1) ? 'S' : *format;

    switch ( f )
    {
        case 'f' :
        case 'F' :
            ver = comctl32VersionName(ComCtl32Version);
            break;

        case 'o' :
        case 'O' :
            ver = comctl32VersionPart(ComCtl32Version, COMCTL32_OS_PART);
            break;

        case 's' :
        case 'S' :
        case 'n' :
        case 'N' :
        default :
            ver = comctl32VersionPart(ComCtl32Version, COMCTL32_NUMBER_PART);
            break;
    }
    return context->String(ver);
}

/** DlgUtil::version()  [class method]
 *
 *  Returns the ooDialog version string, either the full string, or just the
 *  number part of the string.
 *
 * @param  format  [optional]  Keyword indicating which format the returned
 *                 string should be in.  Keywords are:
 *
 *         Short   4.1.0.5814
 *
 *         Full    ooDialog Version 4.1.0.5814 (an ooRexx Windows Extension)
 *
 *         Level   4.2.0
 *
 *                 Only the first letter is required and case is not
 *                 significant.  If the argument is omitted the Full format is
 *                 the default.
 */
RexxMethod1(RexxStringObject, dlgutil_version_cls, OPTIONAL_CSTRING, format)
{
    char buf[64];

    if ( argumentOmitted(1) )
    {
        format = "F";
    }

    switch ( toupper(*format) )
    {
        case 'L' :
            _snprintf(buf, sizeof(buf), "%u.%u.%u", OOD_LVL_MAJOR, OOD_LVL_MINOR, OOD_LVL_BIT);
            break;

        case 'S' :
            _snprintf(buf, sizeof(buf), "%u.%u.%u.%u", ORX_VER, ORX_REL, ORX_MOD, OOREXX_BLD);
            break;

        case 'F' :
        default :
            _snprintf(buf, sizeof(buf), "ooDialog Version %u.%u.%u.%u (an ooRexx Windows Extension)",
                      ORX_VER, ORX_REL, ORX_MOD, OOREXX_BLD);
            break;

    }
    return context->String(buf);
}

RexxMethod1(int16_t, dlgutil_shiWord_cls, int32_t, dw) { return HIWORD(dw); }
RexxMethod1(int16_t, dlgutil_sloWord_cls, int32_t, dw) { return LOWORD(dw); }

RexxMethod1(uint16_t, dlgutil_hiWord_cls, uint32_t, dw) { return HIWORD(dw); }
RexxMethod1(uint16_t, dlgutil_loWord_cls, uint32_t, dw) { return LOWORD(dw); }

RexxMethod2(intptr_t, dlgutil_makeLPARAM_cls, int16_t, loWord, int16_t, hiWord) { return MAKELPARAM(loWord, hiWord); }
RexxMethod2(uintptr_t, dlgutil_makeWPARAM_cls, int16_t, loWord, int16_t, hiWord) { return MAKEWPARAM(loWord, hiWord); }

RexxMethod1(uintptr_t, dlgutil_unsigned_cls, intptr_t, n1)
{
    return (uintptr_t)n1;
}

RexxMethod1(intptr_t, dlgutil_signed_cls, uintptr_t, n1)
{
    return (intptr_t)n1;
}

inline bool inBounds(RexxThreadContext *c, size_t pos, uint32_t n, uint32_t upperLimit)
{
    if ( n >= 0 && n <= upperLimit )
    {
        return true;
    }
    wrongRangeException(c, pos, 0, upperLimit, n);
    return false;
}

RexxMethod2(uint64_t, dlgutil_shiftLeft_cls, uint64_t, n1, OPTIONAL_uint8_t, amount)
{
    if ( ! inBounds(context->threadContext, 2, amount, 63) )
    {
        return 0;
    }
    return n1 << amount;
}

RexxMethod2(uint64_t, dlgutil_shiftRight_cls, uint64_t, n1, OPTIONAL_uint8_t, amount)
{
    if ( ! inBounds(context->threadContext, 2, amount, 63) )
    {
        return 0;
    }
    return n1 >> amount;
}

RexxMethod2(wholenumber_t, dlgutil_sShiftLeft_cls, int64_t, n1, OPTIONAL_uint8_t, amount)
{
    if ( ! inBounds(context->threadContext, 2, amount, 63) )
    {
        return 0;
    }

    uint64_t un1 = (uint64_t)n1;
    return (wholenumber_t)(un1 << amount);
}

RexxMethod2(wholenumber_t, dlgutil_sShiftRight_cls, int64_t, n1, OPTIONAL_uint8_t, amount)
{
    if ( ! inBounds(context->threadContext, 2, amount, 63) )
    {
        return 0;
    }

    uint64_t un1 = (uint64_t)n1;
    return (wholenumber_t)(un1 >> amount);
}

RexxMethod2(uint64_t, dlgutil_and_cls, CSTRING, s1, CSTRING, s2)
{
    uint64_t n1, n2;
    if ( ! rxStr2Number(context, s1, &n1, 1) || ! rxStr2Number(context, s2, &n2, 2) )
    {
        return 0;
    }
    return (n1 & n2);
}

RexxMethod1(uint64_t, dlgutil_or_cls, ARGLIST, args)
{
    RexxMethodContext *c = context;
    uint64_t result, n1;

    size_t count = c->ArraySize(args);
    if ( count == 0 )
    {
        return 0;
    }

    CSTRING s1 = c->ObjectToStringValue(c->ArrayAt(args, 1));
    if ( ! rxStr2Number(c, s1, &result, 1) )
    {
        return 0;
    }

    for ( size_t i = 2; i <= count; i++)
    {
        s1 = c->ObjectToStringValue(c->ArrayAt(args, i));
        if ( ! rxStr2Number(c, s1, &n1, (int)i) )
        {
            return 0;
        }
        result |= n1;
    }

    return result;
}

/** DlgUtil::getSystemMetrics()  [class method]
 *
 *  Returns the system metric for the give index.
 *
 *  @param index  The index of the system metric to look up.
 *
 *  @note There was a classic Rexx external function documented prior to 4.0.0
 *        with the function name of GetSysMetrics.  That function is now marked
 *        deprecated in the docs and the places where it was used are mapped to
 *        this .DlgUtil method.
 *
 *        The intent was to extend this function in the future to get multiple,
 *        and perhaps all, values at once.  This method could be enhanced to do
 *        that.
 *
 *        MSDN documents that GetLastError does not provide extended error
 *        information.
 */
RexxMethod1(uint32_t, dlgutil_getSystemMetrics_cls, int32_t, index)
{
    return GetSystemMetrics(index);
}

/** DlgUtil::screenSize()  [class method]
 *
 *  Retrieves the screen size in either pixels, dialog units, or both.
 *
 *  @param  _flag  [optional]  Keyword signaling whether to return the size in
 *                 pixels, dialog units, or both.  The default is both.  The
 *                 keywords are Pixels, DialogUnits, and Both.  Only the first
 *                 letter is checked and case is insignificant.
 *
 *  @param  dlgObj  [optional]  A Rexx dialog object.  When this argument is
 *                  used, dialog units are calculated correctly for the dialog.
 *                  If it is omitted, the dialog units are calculated using 8 pt
 *                  System font, which will be incorrect for any dialog that
 *                  uses a different fault.  However, this will give the same
 *                  result as was returned prior to ooRexx 4.0.0.
 *
 *  @return  The return is dependent on the flag in use:
 *           Pixels:       A .Size object with the screen size in pixels.
 *           DialogUnits:  A .Size object with the screen size in dialog units.
 *           Both:         An array:
 *                         a[1] = duX, a[2] = duY, a[3] = pixelX, a[4] = pixelY
 *
 */
RexxMethod2(RexxObjectPtr, dlgutil_screenSize_cls, OPTIONAL_CSTRING, _flag, OPTIONAL_RexxObjectPtr, dlgObj)
{
    RexxObjectPtr result = NULLOBJECT;

    uint32_t pixelX = GetSystemMetrics(SM_CXSCREEN);
    uint32_t pixelY = GetSystemMetrics(SM_CYSCREEN);

    char flag = 'B';
    if ( argumentExists(1) )
    {
        flag = toupper(*_flag);
        if ( ! (flag == 'B' || flag == 'D' || flag == 'P') )
        {
            wrongArgValueException(context->threadContext, 1, "DialogUnit, Pixel, Both", _flag);
            goto done_out;
        }
    }

    uint32_t duX, duY;
    if ( flag == 'B' || flag == 'D' )
    {
        // We need to calculate the dialog units.  Iff we have a dlgObj, we'll
        // calculate them correctly, otherwise we use the broken method.
        if ( argumentExists(2) )
        {
            if ( ! requiredClass(context->threadContext, dlgObj, "PlainBaseDialog", 2) )
            {
                goto done_out;
            }

            POINT point = {pixelX, pixelY};
            mapPixelToDu(context, dlgObj, &point, 1);
            duX = point.x;
            duY = point.y;
        }
        else
        {
            long bu = GetDialogBaseUnits();
            duX = (pixelX * 4) / LOWORD(bu);
            duY = (pixelY * 8) / HIWORD(bu);
        }
    }

    if ( flag == 'B')
    {
        result = context->ArrayOfFour(context->UnsignedInt32(duX),
                                      context->UnsignedInt32(duY),
                                      context->UnsignedInt32(pixelX),
                                      context->UnsignedInt32(pixelY));
    }
    else if ( flag == 'D' )
    {
        result = rxNewSize(context, duX, duY);
    }
    else
    {
        result = rxNewSize(context, pixelX, pixelY);
    }

done_out:
    return result;
}

/** DlgUtil::screenArea()  [class method]
 *
 *  Gets the usable screen area (work area.) on the primary display monitor. The
 *  work area is the portion of the screen not obscured by the system taskbar or
 *  by application desktop toolbars.
 *
 *  @return  The work area as a .Rect object.
 *
 *  @note Sets the .SystemErrorCode.
 *
 *        This method is the same as the .SPI~workArea get attribute.
 */
RexxMethod0(RexxObjectPtr, dlgutil_screenArea_cls)
{
    return SPI_getWorkArea(context);
}

/**
 * A temporary utility to convert from a handle that is still being stored in
 * ooDialog in string form ("0xFFFFAAAA") to its actual pointer value.  The
 * interface is needed to facilitate testing Windows extensions that have been
 * converted to only use pointer valules.
 */
RexxMethod1(POINTER, dlgutil_handleToPointer_cls, POINTERSTRING, handle)
{
    return handle;
}

/** DlgUtil::threadID()  [class method]
 *
 *  Simple method to use for testing.
 */
RexxMethod0(uint32_t, dlgutil_threadID_cls)
{
    return GetCurrentThreadId();
}


/** DlgUtil::windowFromPoint()  [class method]
 *
 *
 */
RexxMethod1(RexxStringObject, dlgutil_windowFromPoint_cls, RexxObjectPtr, pt)
{
    PPOINT p = rxGetPoint(context, pt, 1);
    if ( p != NULL )
    {
        return pointer2string(context, WindowFromPoint(*p));
    }
    return NULLOBJECT;
}

/** DlgUtil::test()  [class method]
 *
 *  Simple method to use for testing.
 */
RexxMethod1(uint64_t, dlgutil_test_cls, int64_t, n)
{
    return (uint64_t)n;
}

/**
 *  Methods for the .SPI (SystemParametersInfo) class.
 */
#define SPI_CLASS        "SPI"


/** SPI::new()
 *
 *  Sets up the CSelf for the .SPI class object.
 *
 */
RexxMethod1(RexxObjectPtr, spi_init_cls, OSELF, self)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(CSpi));
    context->SetObjectVariable("CSELF", obj);

    pCSpi spi = (pCSpi)context->BufferData(obj);
    spi->fWinIni = 0;

    return NULLOBJECT;
}

/** SPI::dragHeight  [class attribute get]
 *
 *  @remarks  SystemParametersInfo() can not be used to get the drag height or
 *            width.  So originally the attribute was going to be a set only
 *            attribute.  But, on reflection this seems silly, there is no
 *            reason the SPI class has to *only* use SystemParametersInfo().  We
 *            can just use GetSystemMetrics().
 */
RexxMethod0(uint32_t, spi_getDragHeight_cls)
{
    oodResetSysErrCode(context->threadContext);
    return (uint32_t)GetSystemMetrics(SM_CYDRAG);
}

/** SPI::dragHeight  [class attribute set]
 */
RexxMethod2(RexxObjectPtr, spi_setDragHeight_cls, uint32_t, pixels, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    if ( ! SystemParametersInfo(SPI_SETDRAGHEIGHT, pixels, NULL, ((pCSpi)pCSelf)->fWinIni) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return NULLOBJECT;
}

/** SPI::dragWidth  [class attribute get]
 *
 *  @remarks  SystemParametersInfo() can not be used to get the drag height or
 *            width.  So originally the attribute was going to be a set only
 *            attribute.  But, on reflection this seems silly, there is no
 *            reason the SPI class has to *only* use SystemParametersInfo().  We
 *            can just use GetSystemMetrics().
 */
RexxMethod0(uint32_t, spi_getDragWidth_cls)
{
    oodResetSysErrCode(context->threadContext);
    return (uint32_t)GetSystemMetrics(SM_CXDRAG);
}

/** SPI::dragWidth  [class attribute set]
 */
RexxMethod2(RexxObjectPtr, spi_setDragWidth_cls, uint32_t, pixels, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    if ( ! SystemParametersInfo(SPI_SETDRAGWIDTH, pixels, NULL, ((pCSpi)pCSelf)->fWinIni) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return NULLOBJECT;
}


/** SPI::menuAnimation  [class attribute get]
 */
RexxMethod0(logical_t, spi_getMenuAnimation_cls)
{
    oodResetSysErrCode(context->threadContext);

    logical_t on = FALSE;
    if ( ! SystemParametersInfo(SPI_GETMENUANIMATION, 0, &on, 0) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return on;
}

/** SPI::menuAnimation  [class attribute set]
 */
RexxMethod2(RexxObjectPtr, spi_setMenuAnimation_cls, logical_t, on, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    if ( ! SystemParametersInfo(SPI_SETMENUANIMATION, (uint32_t)on, NULL, ((pCSpi)pCSelf)->fWinIni) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return NULLOBJECT;
}


/** SPI::menuFade  [class attribute get]
 */
RexxMethod0(logical_t, spi_getMenuFade_cls)
{
    oodResetSysErrCode(context->threadContext);

    logical_t on = FALSE;
    if ( ! SystemParametersInfo(SPI_GETMENUFADE, 0, &on, 0) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return on;
}

/** SPI::menuFade  [class attribute set]
 */
RexxMethod2(RexxObjectPtr, spi_setMenuFade_cls, logical_t, on, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    if ( ! SystemParametersInfo(SPI_SETMENUFADE, (uint32_t)on, NULL, ((pCSpi)pCSelf)->fWinIni) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return NULLOBJECT;
}


/** SPI::mouseHoverHeight  [class attribute get]
 */
RexxMethod0(uint32_t, spi_getMouseHoverHeight_cls)
{
    oodResetSysErrCode(context->threadContext);

    uint32_t height = 0;
    if ( ! SystemParametersInfo(SPI_GETMOUSEHOVERHEIGHT, 0, &height, 0) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return height;
}

/** SPI::mouseHoverHeight  [class attribute set]
 */
RexxMethod2(RexxObjectPtr, spi_setMouseHoverHeight_cls, uint32_t, pixels, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    if ( ! SystemParametersInfo(SPI_SETMOUSEHOVERHEIGHT, pixels, NULL, ((pCSpi)pCSelf)->fWinIni) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return NULLOBJECT;
}


/** SPI::mouseHoverTime  [class attribute get]
 */
RexxMethod0(uint32_t, spi_getMouseHoverTime_cls)
{
    oodResetSysErrCode(context->threadContext);

    uint32_t ms = 0;
    if ( ! SystemParametersInfo(SPI_GETMOUSEHOVERTIME, 0, &ms, 0) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return ms;
}

/** SPI::mouseHoverTime  [class attribute set]
 */
RexxMethod2(RexxObjectPtr, spi_setMouseHoverTime_cls, uint32_t, ms, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    if ( ms < USER_TIMER_MINIMUM || ms > USER_TIMER_MAXIMUM )
    {
        wrongRangeException(context->threadContext, 1, USER_TIMER_MINIMUM, USER_TIMER_MAXIMUM, ms);
    }
    else if ( ! SystemParametersInfo(SPI_SETMOUSEHOVERTIME, ms, NULL, ((pCSpi)pCSelf)->fWinIni) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return NULLOBJECT;
}


/** SPI::mouseHoverWidth  [class attribute get]
 */
RexxMethod0(uint32_t, spi_getMouseHoverWidth_cls)
{
    oodResetSysErrCode(context->threadContext);

    uint32_t width = 0;
    if ( ! SystemParametersInfo(SPI_GETMOUSEHOVERWIDTH, 0, &width, 0) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return width;
}

/** SPI::mouseHoverWidth  [class attribute set]
 */
RexxMethod2(RexxObjectPtr, spi_setMouseHoverWidth_cls, uint32_t, pixels, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    if ( ! SystemParametersInfo(SPI_SETMOUSEHOVERWIDTH, pixels, NULL, ((pCSpi)pCSelf)->fWinIni) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return NULLOBJECT;
}


/** SPI::updateFlag  [class attribute get]
 *
 */
RexxMethod1(RexxStringObject, spi_getUpdateFlag_cls, CSELF, pCSelf)
{
    char *flag;
    switch ( ((pCSpi)pCSelf)->fWinIni )
    {
        case SPIF_UPDATEINIFILE :
            flag = "UpdateProfile";
            break;
        case SPIF_SENDCHANGE :
            flag = "SendChange";
            break;
        default :
            flag = "None";
            break;
    }
    return context->String(flag);
}


/** SPI::updateFlag  [class attribute set]
 *
 */
RexxMethod2(RexxObjectPtr, spi_setUpdateFlag_cls, CSTRING, flag, CSELF, pCSelf)
{
    pCSpi spi = (pCSpi)pCSelf;

    switch ( toupper(*flag) )
    {
        case 'U' :
            spi->fWinIni = SPIF_UPDATEINIFILE;
            break;
        case 'S' :
            spi->fWinIni = SPIF_SENDCHANGE;
            break;
        case 'N' :
            spi->fWinIni = 0;
            break;
        default :
            spi->fWinIni = 0;
            wrongArgOptionException (context->threadContext, 1, "[U]pdateProfile, [S]endChange, or [N]one", flag);
            break;

    }
    return NULLOBJECT;
}


/** SPI::workArea()  [class attribute get]
 *
 *  Gets the usable screen area (work area.) on the primary display monitor. The
 *  work area is the portion of the screen not obscured by the system taskbar or
 *  by application desktop toolbars.
 *
 *  @return  The work area as a .Rect object.  On error the rectangle will be
 *           all zeros.
 *
 *  @note Sets the .SystemErrorCode.
 *
 *        The .DlgUtil screenArea() method produces the same results.
 */
RexxMethod0(RexxObjectPtr, spi_getWorkArea_cls)
{
    return SPI_getWorkArea(context);
}


RexxMethod2(RexxObjectPtr, spi_setWorkArea_cls, RexxObjectPtr, rect, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    PRECT r = rxGetRect(context, rect, 1);
    if ( r != NULL )
    {
        if ( ! SystemParametersInfo(SPI_SETWORKAREA, 0, &r, ((pCSpi)pCSelf)->fWinIni) )
        {
            oodSetSysErrCode(context->threadContext);
        }
    }

    return NULLOBJECT;
}


RexxMethod0(uint32_t, spi_getWheelScrollLines_cls)
{
    oodResetSysErrCode(context->threadContext);

    uint32_t lines = 0;
    if ( ! SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines, 0) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return lines;
}

RexxMethod2(RexxObjectPtr, spi_setWheelScrollLines_cls, uint32_t, lines, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    if ( ! SystemParametersInfo(SPI_SETWHEELSCROLLLINES, lines, NULL, ((pCSpi)pCSelf)->fWinIni) )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return NULLOBJECT;
}


/**
 *  Methods for the .SM class.
 *
 *  Notes: GetLastError() does NOT provide extended error information.
 *         All heights and widths are in pixels
 *         All true / false metrics are 0 for false, non-zero for true.
 */
#define SM_CLASS        "SM"

RexxMethod0(int32_t, sm_cMouseButtons_cls)
{
    return GetSystemMetrics(SM_CMOUSEBUTTONS);
}
RexxMethod0(int32_t, sm_cxCursor_cls)
{
    return GetSystemMetrics(SM_CXCURSOR);
}
RexxMethod0(int32_t, sm_cxDrag_cls)
{
    return GetSystemMetrics(SM_CXDRAG);
}
RexxMethod0(int32_t, sm_cxFixedFrame_cls)
{
    return GetSystemMetrics(SM_CXFIXEDFRAME);
}
RexxMethod0(int32_t, sm_cxScreen_cls)
{
    return GetSystemMetrics(SM_CXSCREEN);
}
RexxMethod0(int32_t, sm_cxVScroll_cls)
{
    return GetSystemMetrics(SM_CXVSCROLL);
}
RexxMethod0(int32_t, sm_cyCaption_cls)
{
    return GetSystemMetrics(SM_CYCAPTION);
}
RexxMethod0(int32_t, sm_cyCursor_cls)
{
    return GetSystemMetrics(SM_CYCURSOR);
}
RexxMethod0(int32_t, sm_cyDrag_cls)
{
    return GetSystemMetrics(SM_CYDRAG);
}
RexxMethod0(int32_t, sm_cyFixedFrame_cls)
{
    return GetSystemMetrics(SM_CYFIXEDFRAME);
}
RexxMethod0(int32_t, sm_cyHScroll_cls)
{
    return GetSystemMetrics(SM_CYHSCROLL);
}
RexxMethod0(int32_t, sm_cyScreen_cls)
{
    return GetSystemMetrics(SM_CYSCREEN);
}
RexxMethod0(int32_t, sm_menuDropAlignment_cls)
{
    return GetSystemMetrics(SM_MENUDROPALIGNMENT);
}

/**
 *  Methods for the .OS class.
 */
#define OS_CLASS        "OS"

bool _is32on64Bit(void)
{
    if ( _isAtLeastXP() )
    {
        BOOL isWow64 = FALSE;
        typedef BOOL (WINAPI *PFNISWOW)(HANDLE, PBOOL);

        PFNISWOW fnIsWow64Process = (PFNISWOW)GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");

        if ( fnIsWow64Process != NULL)
        {
            if ( fnIsWow64Process(GetCurrentProcess(), &isWow64) && isWow64 )
            {
                return true;
            }
        }
    }
    return false;
}

bool _isVersion(DWORD major, DWORD minor, unsigned int sp, unsigned int type, unsigned int condition)
{
    OSVERSIONINFOEX ver;
    DWORDLONG       mask = 0;
    DWORD           testForMask = VER_MAJORVERSION | VER_MINORVERSION;

    ZeroMemory(&ver, sizeof(OSVERSIONINFOEX));

    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    ver.dwMajorVersion = major;
    ver.dwMinorVersion = minor;

    VER_SET_CONDITION(mask, VER_MAJORVERSION, condition);
    VER_SET_CONDITION(mask, VER_MINORVERSION, condition);

    if ( condition != VER_EQUAL )
    {
        ver.wServicePackMajor = sp;
        testForMask |= VER_SERVICEPACKMAJOR;
        VER_SET_CONDITION(mask, VER_SERVICEPACKMAJOR, condition);
    }

    if ( type != 0 )
    {
        ver.wProductType = type;
        testForMask |= VER_PRODUCT_TYPE;
        VER_SET_CONDITION(mask, VER_PRODUCT_TYPE, condition);
    }

    if ( VerifyVersionInfo(&ver, testForMask, mask) )
        return true;
    else
        return false;
}


RexxMethod0(RexxObjectPtr, os_is64bit)
{
    return (_is64Bit() ? TheTrueObj : TheFalseObj);
}

RexxMethod0(RexxObjectPtr, os_is32on64bit)
{
    return (_is32on64Bit() ? TheTrueObj : TheFalseObj);
}

RexxMethod1(RexxObjectPtr, os_isVersion, NAME, method)
{
    bool isVersion = false;
    CSTRING p;

    if ( method[2] == 'A' )
    {
        p = method + 9;
        if (      strcmp(p, "W2K") == 0      ) isVersion = _isAtLeastW2K();
        else if ( strcmp(p, "XP") == 0       ) isVersion = _isAtLeastXP();
        else if ( strcmp(p, "W2K3") == 0     ) isVersion = _isAtLeastW2K3();
        else if ( strcmp(p, "VISTA") == 0    ) isVersion = _isAtLeastVista();
        else if ( strcmp(p, "WINDOWS7") == 0 ) isVersion = _isAtLeastWindows7();
    }
    else
    {
        p = method + 2;
        if (      strcmp(p, "W2K") == 0          ) isVersion = _isW2K();
        else if ( strcmp(p, "XP") == 0           ) isVersion = _isXP();
        else if ( strcmp(p, "XP32") == 0         ) isVersion = _isXP32();
        else if ( strcmp(p, "XP64") == 0         ) isVersion = _isXP64();
        else if ( strcmp(p, "W2K3") == 0         ) isVersion = _isW2K3();
        else if ( strcmp(p, "VISTA") == 0        ) isVersion = _isVista();
        else if ( strcmp(p, "SERVER2008") == 0   ) isVersion = _isServer2008();
        else if ( strcmp(p, "WINDOWS7") == 0     ) isVersion = _isWindows7();
        else if ( strcmp(p, "SERVER2008R2") == 0 ) isVersion = _isServer2008R2();
    }

    return (isVersion ? TheTrueObj : TheFalseObj);
}


/**
 *  Methods for the .ResourceUtils mixin class.
 */
#define RESOURCEUTILS_CLASS        "ResourceUtils"

RexxMethod2(int32_t, rsrcUtils_checkID, RexxObjectPtr, rxID, OSELF, self)
{
    return checkID(context, rxID, self);
}

RexxMethod1(int32_t, rsrcUtils_idError, RexxObjectPtr, rxID)
{
    return idError(context, rxID);
}

/** ResourceUtils::resolveSymbolicID
 *  ResourceUtils::getResourceID
 *
 *  Returns the numeric value of an, assumed, resource ID.
 *
 *  The primary purpose is to resolve a symbolic resource ID to its integer
 *  value.  If the object is already an integer value, then that value is
 *  simpley returned.
 *
 *  @param rxID  The resource ID object to resolve.
 *
 *  @return  On success, the resolved interger value of the resource ID.
 *
 *  @remarks  resolveSymbolicID() expects a positive ID returned on success and
 *            -1 returned on error.  No exceptions can be raised, except an out
 *             of memory exception.  This is the implementation for the original
 *             ooDialog resolveSymbolicID().
 *
 *            getResourceID() raises an execeptions if a symbolic ID can not
 *            be resolved, or resolves to a number less than 1. A return of
 *            greater than 0 is expected for success and less than 1 for an
 *            error.
 */
RexxMethod3(int32_t, rsrcUtils_resolveResourceID, RexxObjectPtr, rxID, NAME, method, OSELF, self)
{
    if ( *method == 'R' )
    {
        return resolveResourceID(context, rxID, self);
    }
    else
    {
        return oodResolveSymbolicID(context, self, rxID, -1, 1, true);
    }
}

RexxMethod2(int32_t, rsrcUtils_resolveIconID_pvt, RexxObjectPtr, rxID, OSELF, self)
{
    return resolveIconID(context, rxID, self);
}

/**
 *  Methods for the .Window class.
 */
#define WINDOW_CLASS  "Window"

RexxMethod2(RexxObjectPtr, window_init, POINTERSTRING, hwnd, OSELF, self)
{
    if ( !IsWindow((HWND)hwnd) )
    {
        invalidTypeException(context->threadContext, 1, "window handle");
    }
    else
    {
        initWindowBase(context, (HWND)hwnd, self, NULL);
    }
    return NULLOBJECT;
}

/** Window::unInit()
 *
 *  Release the global reference for CWindowBase::rexxHwnd.
 *
 */
RexxMethod1(RexxObjectPtr, window_unInit, CSELF, pCSelf)
{
    if ( pCSelf != NULLOBJECT )
    {
        pCWindowBase pcwb = (pCWindowBase)pCSelf;
        if ( pcwb->rexxHwnd != TheZeroObj )
        {
            context->ReleaseGlobalReference(pcwb->rexxHwnd);
            pcwb->rexxHwnd = TheZeroObj;
        }
    }
    return NULLOBJECT;
}


/**
 * Methods for the ooDialog .Point class.
 */
#define POINT_CLASS  "Point"


RexxMethod1(RexxObjectPtr, point_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, POINT_CLASS) )
    {
        ThePointClass = (RexxClassObject)self;
        context->RequestGlobalReference(ThePointClass);
    }
    return NULLOBJECT;
}

RexxMethod2(RexxObjectPtr, point_init, OPTIONAL_int32_t,  x, OPTIONAL_int32_t, y)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(POINT));
    context->SetObjectVariable("CSELF", obj);

    POINT *p = (POINT *)context->BufferData(obj);
    p->x = argumentExists(1) ? x : 0;
    p->y = argumentExists(2) ? y : p->x;

    return NULLOBJECT;
}

RexxMethod1(int32_t, point_x, CSELF, p) { return ((POINT *)p)->x; }
RexxMethod1(int32_t, point_y, CSELF, p) { return ((POINT *)p)->y; }
RexxMethod2(RexxObjectPtr, point_setX, CSELF, p, int32_t, x) { ((POINT *)p)->x = x; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, point_setY, CSELF, p, int32_t, y) { ((POINT *)p)->y = y; return NULLOBJECT; }

/** Point::copy()
 *
 *  Returns a new point object that is a copy of this point.
 *
 */
RexxMethod1(RexxObjectPtr, point_copy, CSELF, p)
{
    POINT *_p = (POINT *)p;
    return rxNewPoint(context, _p->x, _p->y);
}

/** Point::+
 *
 *  Returns a new point object that is the result of "adding" two points.
 *
 */
RexxMethod2(RexxObjectPtr, point_add, RexxObjectPtr, other, CSELF, p)
{
    if ( ! context->IsOfType(other, "POINT") )
    {
        wrongClassException(context->threadContext, 1, "Point");
        return NULLOBJECT;
    }

    POINT *p1 = (POINT *)p;
    POINT *p2 = (POINT *)context->ObjectToCSelf(other);

    return rxNewPoint(context, p1->x + p2->x, p1->y + p2->y);
}

/** Point::-
 *
 *  Returns a new point object that is the result of "subtracting" two points.
 *
 */
RexxMethod2(RexxObjectPtr, point_subtract, RexxObjectPtr, other, CSELF, p)
{
    if ( ! context->IsOfType(other, "POINT") )
    {
        wrongClassException(context->threadContext, 1, "Point");
        return NULLOBJECT;
    }

    POINT *p1 = (POINT *)p;
    POINT *p2 = (POINT *)context->ObjectToCSelf(other);

    return rxNewPoint(context, p1->x - p2->x, p1->y - p2->y);
}

/** Point::incr
 *
 *  Increments this point's x and y attributes by the specified amount. If both
 *  optional args are ommitted, the x and y are incremented by 1.
 *
 *  @param  x  The amount to increment, (to add to,) this point's x attribute.
 *             If y is specified and this arg is omitted than 0 is used.
 *
 *  @param  y  The amount to increment, (to add to,) this point's y attribute.
 *             If x is specified and this arg is omitted than 0 is used.
 *
 *  @return  No return.
 *
 *  @remarks  If either x or y are omitted, then their value will be 0.  Since
 *            that is the default, once we check that both are not ommitted, we
 *            are safe to just add x and y to the current x and y.
 */
RexxMethod3(RexxObjectPtr, point_incr, OPTIONAL_int32_t, x, OPTIONAL_int32_t, y, CSELF, p)
{
    if ( argumentOmitted(1) && argumentOmitted(2) )
    {
        ((POINT *)p)->x++;
        ((POINT *)p)->y++;
    }
    else
    {
        ((POINT *)p)->x += x;
        ((POINT *)p)->y += y;
    }

    return NULLOBJECT;
}

/** Point::decr
 *
 *  Decrements this point's x and y attributes by the specified amount. See the
 *  comments above for Point::incr() for details.
 */
RexxMethod3(RexxObjectPtr, point_decr, OPTIONAL_int32_t, x, OPTIONAL_int32_t, y, CSELF, p)
{
    if ( argumentOmitted(1) && argumentOmitted(2) )
    {
        ((POINT *)p)->x--;
        ((POINT *)p)->y--;
    }
    else
    {
        ((POINT *)p)->x -= x;
        ((POINT *)p)->y -= y;
    }

    return NULLOBJECT;
}

/** Point::inRect
 *
 *  Determines if this point is in the specified rectangle.  The rectangle must
 *  be normalized, that is, rect.right must be greater than rect.left and
 *  rect.bottom must be greater than rect.top. If the rectangle is not
 *  normalized, a point is never considered inside of the rectangle.
 */
RexxMethod2(logical_t, point_inRect, RexxObjectPtr, rect, CSELF, p)
{
    PRECT r = rxGetRect(context, rect, 1);
    if ( r != NULL )
    {
        POINT pt = {((POINT *)p)->x, ((POINT *)p)->y};
        return PtInRect(r, pt);
    }

    return FALSE;
}

RexxMethod1(RexxStringObject, point_string, CSELF, p)
{
    PPOINT pt = (PPOINT)p;

    TCHAR buf[128];
    _snprintf(buf, sizeof(buf), "a Point (%d, %d)", pt->x, pt->y);

    return context->String(buf);
}



/**
 * Methods for the ooDialog .Size class.
 */
#define SIZE_CLASS  "Size"

RexxMethod1(RexxObjectPtr, size_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, SIZE_CLASS) )
    {
        TheSizeClass = (RexxClassObject)self;
        context->RequestGlobalReference(TheSizeClass);
    }
    return NULLOBJECT;
}

RexxMethod2(RexxObjectPtr, size_init, OPTIONAL_int32_t, cx, OPTIONAL_int32_t, cy)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(SIZE));
    context->SetObjectVariable("CSELF", obj);

    SIZE *s = (SIZE *)context->BufferData(obj);

    s->cx = argumentExists(1) ? cx : 0;
    s->cy = argumentExists(2) ? cy : s->cx;

    return NULLOBJECT;
}

RexxMethod1(int32_t, size_cx, CSELF, s) { return ((SIZE *)s)->cx; }
RexxMethod1(int32_t, size_cy, CSELF, s) { return ((SIZE *)s)->cy; }
RexxMethod2(RexxObjectPtr, size_setCX, CSELF, s, int32_t, cx) { ((SIZE *)s)->cx = cx; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, size_setCY, CSELF, s, int32_t, cy) { ((SIZE *)s)->cy = cy; return NULLOBJECT; }

RexxMethod1(RexxStringObject, size_string, CSELF, s)
{
    PSIZE size = (PSIZE)s;

    TCHAR buf[128];
    _snprintf(buf, sizeof(buf), "a Size (%d, %d)", size->cx, size->cy);

    return context->String(buf);
}

RexxMethod2(logical_t, size_equateTo, RexxObjectPtr, other, CSELF, s)
{
    PSIZE size = (PSIZE)s;
    PSIZE pOther = rxGetSize(context, other, 1);
    if ( pOther == NULL )
    {
        return FALSE;
    }

    size->cx = pOther->cx;
    size->cy = pOther->cy;

    return TRUE;
}

RexxMethod3(logical_t, size_compare, RexxObjectPtr, other, NAME, method, CSELF, s)
{
    PSIZE size = (PSIZE)s;
    PSIZE pOther = rxGetSize(context, other, 1);
    if ( pOther == NULL )
    {
        return 0;
    }

    if ( strcmp(method, "=") == 0 )
    {
        return (size->cx * size->cy) == (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, "==") == 0 )
    {
        return (size->cx == pOther->cx) && (size->cy == pOther->cy);
    }
    else if ( strcmp(method, "\\=") == 0 )
    {
        return (size->cx * size->cy) != (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, "\\==") == 0 )
    {
        return (size->cx != pOther->cx) && (size->cy != pOther->cy);
    }
    else if ( strcmp(method, "<") == 0 )
    {
        return (size->cx * size->cy) < (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, "<<") == 0 )
    {
        return (size->cx < pOther->cx) && (size->cy < pOther->cy);
    }
    else if ( strcmp(method, "<=") == 0 )
    {
        return (size->cx * size->cy) <= (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, "<<=") == 0 )
    {
        return (size->cx <= pOther->cx) && (size->cy <= pOther->cy);
    }
    else if ( strcmp(method, ">") == 0 )
    {
        return (size->cx * size->cy) > (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, ">>") == 0 )
    {
        return (size->cx > pOther->cx) && (size->cy > pOther->cy);
    }
    else if ( strcmp(method, ">=") == 0 )
    {
        return (size->cx * size->cy) >= (pOther->cx * pOther->cy);
    }
    else if ( strcmp(method, ">>=") == 0 )
    {
        return (size->cx >= pOther->cx) && (size->cy >= pOther->cy);
    }

    return FALSE;
}

/**
 * Methods for the ooDialog .Rect class.
 */
#define RECT_CLASS  "Rect"

RexxMethod1(RexxObjectPtr, rect_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, RECT_CLASS) )
    {
        TheRectClass = (RexxClassObject)self;
        context->RequestGlobalReference(TheRectClass);
    }
    return NULLOBJECT;
}

RexxMethod4(RexxObjectPtr, rect_init, OPTIONAL_int32_t, left, OPTIONAL_int32_t, top,
            OPTIONAL_int32_t, right, OPTIONAL_int32_t, bottom)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(RECT));
    context->SetObjectVariable("CSELF", obj);

    RECT *r = (RECT *)context->BufferData(obj);

    r->left = argumentExists(1) ? left : 0;
    r->top = argumentExists(2) ? top : r->left;
    r->right = argumentExists(3) ? right : r->left;
    r->bottom = argumentExists(4) ? bottom : r->left;

    return NULLOBJECT;
}

RexxMethod1(int32_t, rect_left, CSELF, pRect) { return ((RECT *)pRect)->left; }
RexxMethod1(int32_t, rect_top, CSELF, pRect) { return ((RECT *)pRect)->top; }
RexxMethod1(int32_t, rect_right, CSELF, pRect) { return ((RECT *)pRect)->right; }
RexxMethod1(int32_t, rect_bottom, CSELF, pRect) { return ((RECT *)pRect)->bottom; }
RexxMethod2(RexxObjectPtr, rect_setLeft, CSELF, pRect, int32_t, left) { ((RECT *)pRect)->left = left; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, rect_setTop, CSELF, pRect, int32_t, top) { ((RECT *)pRect)->top = top; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, rect_setRight, CSELF, pRect, int32_t, right) { ((RECT *)pRect)->right = right; return NULLOBJECT; }
RexxMethod2(RexxObjectPtr, rect_setBottom, CSELF, pRect, int32_t, bottom) { ((RECT *)pRect)->bottom = bottom; return NULLOBJECT; }

RexxMethod1(RexxStringObject, rect_string, CSELF, pRect)
{
    PRECT pR = (PRECT)pRect;

    TCHAR buf[128];
    _snprintf(buf, sizeof(buf), "a Rect (%d, %d, %d, %d)", pR->left, pR->top, pR->right, pR->bottom);

    return context->String(buf);
}

/**
 * Methods for the ooDialog .VK class.
 */
#define VK_CLASS  "VK"

/** VK::key2name()
 *
 * Translates a virtual key code into a string version of its symbolic name.
 * I.e., 32 becomes "VK_SPACE".
 *
 * @param  key  The virtual key code.  Must be between 0 and 255.
 *
 * @return The virtual key name.
 *
 * @notes  Not all the numbers between 0 and 255 map to a virtual key.  The
 *         empty string is returned for those numbers.
 *
 * @remarks  The initializers for names, and nothing else, are kept in
 *           oodKeyNames.hpp.
 *
 */
RexxMethod1(CSTRING, vk_key2name, uint8_t, key)
{
    static char *names[256] =
    {
#include "oodKeyNames.hpp"
    };

    return names[key];
}



/**
 * Methods for the ooDialog .DayState class.
 */
#define DAYSTATE_CLASS  "DayState"

typedef struct _dayState
{
    uint32_t  val;
} DAYSTATE, *PDAYSTATE;


RexxMethod1(RexxObjectPtr, ds_init, ARGLIST, args)
{
    RexxBufferObject obj = context->NewBuffer(sizeof(DAYSTATE));
    context->SetObjectVariable("CSELF", obj);

    PDAYSTATE dayState = (PDAYSTATE)context->BufferData(obj);
    uint32_t val = 0;
    uint32_t day;

    size_t count = context->ArraySize(args);
    for ( size_t i = 1; i <= count; i++ )
    {
        RexxObjectPtr _day = context->ArrayAt(args, i);
        if ( ! context->UnsignedInt32(_day, &day) )
        {
            notNonNegativeException(context->threadContext, i, _day);
        }
        if ( day < 1 || day > 31 )
        {
            wrongRangeException(context->threadContext, i, 1, 31, _day);
        }
        val |= (0x00000001 << (day - 1));
    }
    dayState->val = val;

    return NULLOBJECT;
}

RexxMethod1(uint32_t, ds_value, CSELF, ds) { return ((PDAYSTATE)ds)->val; }


/**
 * Methods for the ooDialog .DayStates class.
 */
#define DAYSTATES_CLASE  "DayStates"


RexxObjectPtr makeDayStateBuffer(RexxMethodContext *c, RexxArrayObject list, size_t count, LPMONTHDAYSTATE *ppmds)
{
    RexxBufferObject mdsBuf = c->NewBuffer(count * sizeof(MONTHDAYSTATE));
    if ( mdsBuf != NULLOBJECT )
    {
        MONTHDAYSTATE *pmds = (MONTHDAYSTATE *)c->BufferData(mdsBuf);
        RexxObjectPtr  rxMDSVal;
        PDAYSTATE      pDayState;

        MONTHDAYSTATE *p = pmds;
        for ( size_t i = 1; i <= count; i++, p++ )
        {
            rxMDSVal = c->ArrayAt(list, i);
            if ( rxMDSVal == NULLOBJECT || ! c->IsOfType(rxMDSVal, "DAYSTATE") )
            {
                wrongObjInArrayException(c->threadContext, 1, i, "a DayState object");
                mdsBuf = NULLOBJECT;
                break;
            }

            pDayState = (PDAYSTATE)c->ObjectToCSelf(rxMDSVal);
            *p = pDayState->val;
        }

        if ( mdsBuf != NULLOBJECT && ppmds != NULL )
        {
            *ppmds = pmds;
        }
    }

    return mdsBuf;
}

RexxObjectPtr quickDayStateBuffer(RexxMethodContext *c, uint32_t ds1, uint32_t ds2, uint32_t ds3, LPMONTHDAYSTATE *ppmds)
{
    RexxBufferObject mdsBuf = c->NewBuffer(3 * sizeof(MONTHDAYSTATE));
    if ( mdsBuf != NULLOBJECT )
    {
        MONTHDAYSTATE *pmds = (MONTHDAYSTATE *)c->BufferData(mdsBuf);

        *pmds = ds1;
        *(pmds + 1) = ds2;
        *(pmds + 2) = ds3;

        if ( ppmds != NULL )
        {
            *ppmds = pmds;
        }
    }

    return mdsBuf;
}

RexxObjectPtr makeQuickDayStateBuffer(RexxMethodContext *c, RexxObjectPtr _ds1, RexxObjectPtr _ds2,
                                      RexxObjectPtr _ds3, LPMONTHDAYSTATE *ppmds)
{
    RexxObjectPtr result = NULLOBJECT;

    if ( requiredClass(c->threadContext, _ds1, "DAYSTATE", 1) &&
         requiredClass(c->threadContext, _ds2, "DAYSTATE", 2) &&
         requiredClass(c->threadContext, _ds3, "DAYSTATE", 3) )
    {
        PDAYSTATE ds1 = (PDAYSTATE)c->ObjectToCSelf(_ds1);
        PDAYSTATE ds2 = (PDAYSTATE)c->ObjectToCSelf(_ds2);
        PDAYSTATE ds3 = (PDAYSTATE)c->ObjectToCSelf(_ds3);

        result = quickDayStateBuffer(c, ds1->val, ds2->val, ds3->val, ppmds);
    }
    return result;
}

RexxMethod1(RexxObjectPtr, dss_makeDayStateBuffer, RexxArrayObject, list)
{
    size_t count = context->ArrayItems(list);
    return makeDayStateBuffer(context, list, count, NULL);
}

RexxMethod3(RexxObjectPtr, dss_quickDayStateBuffer, RexxObjectPtr, _ds1, RexxObjectPtr, _ds2, RexxObjectPtr, _ds3)
{
    return makeQuickDayStateBuffer(context, _ds1, _ds2, _ds3, NULL);
}


/**
 * Methods for the ooDialog .TimedMessage class.
 */
#define TIMEDMESSAGE_CLASS  "TimedMessage"


RexxMethod7(RexxObjectPtr, timedmsg_init, RexxStringObject, msg, RexxStringObject, title, int32_t, duration,
            OPTIONAL_logical_t, earlyReply, OPTIONAL_RexxObjectPtr, pos, SUPER, super, OSELF, self)
{
    if ( argumentExists(5) )
    {
        if ( pos != TheNilObj && ! context->IsOfType(pos, "POINT"))
        {
            wrongArgValueException(context->threadContext, 5, "a Point object or the Nil object", pos);
            return NULLOBJECT;
        }
        context->SetObjectVariable("POS", pos);
    }
    else
    {
        context->SetObjectVariable("POS", TheNilObj);
    }

    RexxArrayObject args = context->NewArray(0);
    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, args);

    context->SetObjectVariable("MESSAGE", msg);
    context->SetObjectVariable("TITLE", title);
    context->SetObjectVariable("SLEEPING", context->Int32(duration));
    context->SetObjectVariable("EARLYREPLY", context->Logical(earlyReply));

    return TheZeroObj;
}

