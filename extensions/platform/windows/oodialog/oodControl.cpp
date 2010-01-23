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
 * oodControl.cpp
 *
 * Contains the base classes used for an object that represents a Windows
 * Control.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <commctrl.h>
#include <shlwapi.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodMessaging.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodData.hpp"
#include "oodControl.hpp"

const char *controlType2winName(oodControl_t control)
{
    switch ( control )
    {
        case winStatic :               return WC_STATIC;
        case winPushButton :           return WC_BUTTON;
        case winRadioButton :          return WC_BUTTON;
        case winCheckBox :             return WC_BUTTON;
        case winGroupBox :             return WC_BUTTON;
        case winEdit :                 return WC_EDIT;
        case winListBox :              return WC_LISTBOX;
        case winComboBox :             return WC_COMBOBOX;
        case winScrollBar :            return WC_SCROLLBAR;
        case winTreeView :             return WC_TREEVIEW;
        case winListView :             return WC_LISTVIEW;
        case winTab :                  return WC_TABCONTROL;
        case winProgressBar :          return PROGRESS_CLASS;
        case winTrackBar :             return TRACKBAR_CLASS;
        case winMonthCalendar :        return MONTHCAL_CLASS;
        case winDateTimePicker :       return DATETIMEPICK_CLASS;
        case winUpDown :               return UPDOWN_CLASS;
        default :                      return "";
    }
}


const char *controlType2className(oodControl_t control)
{
    switch ( control )
    {
        case winStatic :               return "STATICCONTROL";
        case winPushButton :           return "BUTTONCONTROL";
        case winRadioButton :          return "RADIOBUTTON";
        case winCheckBox :             return "CHECKBOX";
        case winGroupBox :             return "GROUPBOX";
        case winEdit :                 return "EDITCONTROL";
        case winListBox :              return "LISTBOX";
        case winComboBox :             return "COMBOBOX";
        case winScrollBar :            return "SCROLLBAR";
        case winTreeView :             return "TREECONTROL";
        case winListView :             return "LISTCONTROL";
        case winTab :                  return "TABCONTROL";
        case winProgressBar :          return "PROGRESSBAR";
        case winTrackBar :             return "SLIDERCONTROL";
        case winMonthCalendar :        return "MONTHCALENDAR";
        case winDateTimePicker :       return "DATETIMEPICKER";
        case winUpDown :               return "UPDOWN";
        default :                      return "";
    }
}


oodControl_t winName2controlType(const char *className)
{
    if (      strcmp(className, WC_STATIC         ) == 0 ) return winStatic;
    else if ( strcmp(className, WC_BUTTON         ) == 0 ) return winPushButton;
    else if ( strcmp(className, WC_EDIT           ) == 0 ) return winEdit;
    else if ( strcmp(className, WC_LISTBOX        ) == 0 ) return winListBox;
    else if ( strcmp(className, WC_COMBOBOX       ) == 0 ) return winComboBox;
    else if ( strcmp(className, WC_SCROLLBAR      ) == 0 ) return winScrollBar;
    else if ( strcmp(className, WC_TREEVIEW       ) == 0 ) return winTreeView;
    else if ( strcmp(className, WC_LISTVIEW       ) == 0 ) return winListView;
    else if ( strcmp(className, WC_TABCONTROL     ) == 0 ) return winTab;
    else if ( strcmp(className, PROGRESS_CLASS    ) == 0 ) return winProgressBar;
    else if ( strcmp(className, TRACKBAR_CLASS    ) == 0 ) return winTrackBar;
    else if ( strcmp(className, MONTHCAL_CLASS    ) == 0 ) return winMonthCalendar;
    else if ( strcmp(className, DATETIMEPICK_CLASS) == 0 ) return winDateTimePicker;
    else if ( strcmp(className, UPDOWN_CLASS      ) == 0 ) return winUpDown;
    else
    {
        return winUnknown;
    }
}

oodControl_t control2controlType(HWND hControl)
{
    oodControl_t type = winUnknown;

    TCHAR buf[64];
    if ( RealGetWindowClass(hControl, buf, sizeof(buf)) )
    {
        type = winName2controlType(buf);
        if ( type == winPushButton )
        {
            BUTTONTYPE buttonType = getButtonInfo(hControl, NULL, NULL);
            if ( buttonType == check )
            {
                type = winCheckBox;
            }
            else if ( buttonType == radio )
            {
                type = winRadioButton;
            }
            else if ( buttonType == group )
            {
                type = winGroupBox;
            }
        }
    }

    return type;
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
bool isControlMatch(HWND hControl, oodControl_t control)
{
    char buf[64];
    const char *pClass = controlType2winName(control);

    if ( ! RealGetWindowClass(hControl, buf, sizeof(buf)) || strcmp(buf, pClass) != 0 )
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

/**
 * Resolves a string to the type of windows control it is.  The function only
 * compares enough letters to determine unequivocally if it matches one of the
 * supported dialog controls.
 *
 * Example:
 *
 * CSTRING msgName = "CONNECTEDITDATA";
 * oodControl_t ctrl = oodName2controlType(msgName + 7);
 *
 * @param name   The name to resolve.
 *
 * @return The windows control type.  winUnknown is returned for no match and
 *         the name "separator" is special cased to winNotAControl.  (Separator
 *         is used along with the data table stuff.)
 *
 * @remarks  There are some generic message names such as getControlDataPage
 *           that need to match to winUnknown.  CO is not sufficient to
 *           distinguish between comboBox and control.
 */
oodControl_t oodName2controlType(CSTRING name)
{
    if      ( StrCmpN(name, "CHECKBOX", 3      ) == 0 ) return winCheckBox;
    else if ( StrCmpN(name, "COMBOBOX", 3      ) == 0 ) return winComboBox;
    else if ( StrCmpN(name, "DATETIMEPICKER", 1) == 0 ) return winDateTimePicker;
    else if ( StrCmpN(name, "EDIT", 1          ) == 0 ) return winEdit;
    else if ( StrCmpN(name, "GROUPBOX", 1      ) == 0 ) return winGroupBox;
    else if ( StrCmpN(name, "LISTBOX", 5       ) == 0 ) return winListBox;
    else if ( StrCmpN(name, "LISTVIEW", 5      ) == 0 ) return winListView;
    else if ( StrCmpN(name, "MONTHCALENDAR", 1 ) == 0 ) return winMonthCalendar;
    else if ( StrCmpN(name, "PROGRESSBAR", 2   ) == 0 ) return winProgressBar;
    else if ( StrCmpN(name, "PUSHBUTTON", 2    ) == 0 ) return winPushButton;
    else if ( StrCmpN(name, "RADIOBUTTON", 1   ) == 0 ) return winRadioButton;
    else if ( StrCmpN(name, "SCROLLBAR", 2     ) == 0 ) return winScrollBar;
    else if ( StrCmpN(name, "SEPARATOR", 2     ) == 0 ) return winNotAControl;
    else if ( StrCmpN(name, "STATIC", 2        ) == 0 ) return winStatic;
    else if ( StrCmpN(name, "TAB", 3           ) == 0 ) return winTab;
    else if ( StrCmpN(name, "TRACKBAR", 3      ) == 0 ) return winTrackBar;
    else if ( StrCmpN(name, "TREEVIEW", 3      ) == 0 ) return winTreeView;
    else if ( StrCmpN(name, "UPDOWN", 1        ) == 0 ) return winUpDown;
    else return winUnknown;
}

RexxClassObject oodClass4controlType(RexxMethodContext *c, oodControl_t controlType)
{
    RexxClassObject controlClass = NULLOBJECT;
    const char *className = controlType2className(controlType);

    controlClass = rxGetContextClass(c, className);
    if ( controlClass == NULLOBJECT )
    {
        // An exception has been raised, which we don't want.  So, clear it.
        c->ClearCondition();
    }
    return controlClass;
}


/**
 * Creates the Rexx control object that represents the underlying Windows
 * control.
 *
 * The control object can, almost, be created entirely from within the C / C++
 * environment.  A method context and the Rexx parent dialog are needed.
 *
 * @param c
 * @param hControl
 * @param hDlg
 * @param id
 * @param controlType
 * @param self
 * @param isCategoryDlg
 * @param putInBag
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr createRexxControl(RexxMethodContext *c, HWND hControl, HWND hDlg, uint32_t id, oodControl_t controlType,
                                RexxObjectPtr self, bool isCategoryDlg, bool putInBag)
{
    RexxObjectPtr result = TheNilObj;

    // Check if the Rexx control object has already been instantiated.
    RexxObjectPtr rxControl = (RexxObjectPtr)getWindowPtr(hControl, GWLP_USERDATA);
    if ( rxControl != NULLOBJECT )
    {
        // Okay, this specific control has already had a control object
        // instantiated to represent it.  We return this object.
        result = rxControl;
        goto out;
    }

    // No pointer is stored in the user data area, so no control object has been
    // instantiated for this specific control, yet.  We instantiate one now and
    // then store the object in the user data area of the control window.

    PNEWCONTROLPARAMS pArgs = (PNEWCONTROLPARAMS)malloc(sizeof(NEWCONTROLPARAMS));
    if ( pArgs == NULL )
    {
        outOfMemoryException(c->threadContext);
        goto out;
    }

    RexxClassObject controlCls = oodClass4controlType(c, controlType);
    if ( controlCls == NULLOBJECT )
    {
        goto out;
    }

    pArgs->isCatDlg = isCategoryDlg;
    pArgs->controlType = controlType;
    pArgs->hwnd = hControl;
    pArgs->hwndDlg = hDlg;
    pArgs->id = id;
    pArgs->parentDlg = self;

    rxControl = c->SendMessage1(controlCls, "NEW", c->NewPointer(pArgs));
    free(pArgs);

    if ( rxControl != NULLOBJECT && rxControl != TheNilObj )
    {
        result = rxControl;
        setWindowPtr(hControl, GWLP_USERDATA, (LONG_PTR)result);

        if ( putInBag )
        {
            c->SendMessage1(self, "PUTCONTROL", result);
        }
    }

out:
    return result;
}


/**
 *  Methods for the .DialogControl class.
 */
#define DIALOGCONTROL_CLASS        "DialogControl"


DIALOGADMIN *getDCDlgAdm(RexxMethodContext *c, pCDialogControl pcdc)
{
    pCPlainBaseDialog pcpbd = dlgToCSelf(c, pcdc->oDlg);

    DIALOGADMIN *dlgAdm = NULL;
    if ( pcpbd == NULL || pcpbd->dlgAdm == NULL )
    {
        failedToRetrieveDlgAdmException(c->threadContext, pcdc->rexxSelf);
    }
    else
    {
        dlgAdm = pcpbd->dlgAdm;
    }
    return dlgAdm;
}


/**
 * If there is subclass data, free it.
 */
static void freeSubclassData(SUBCLASSDATA * pData)
{
    if ( pData )
    {
        freeKeyPressData(pData->pKeyPressData);
        LocalFree((void *)pData);
    }
}


/**
 * Subclass procedure for any dialog control.  Reports key press events to
 * ooDialog for those key presses connected to an ooDialog method by the user.
 *
 * All messages are passed on unchanged to the control.
 *
 * processKeyPress() is used to actually decipher the key press data and set
 * up the ooDialog method invocation.  That function documents what is sent on
 * to the ooDialog method.
 */
LRESULT CALLBACK KeyPressSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
  LPARAM lParam, UINT_PTR id, DWORD_PTR dwData)
{
    KEYPRESSDATA *pKeyData;
    SUBCLASSDATA *pData = (SUBCLASSDATA *)dwData;
    if ( ! pData ) return DefSubclassProc(hwnd, msg, wParam, lParam);

    pKeyData = pData->pKeyPressData;

    switch ( msg )
    {
        case WM_GETDLGCODE:
            /* Don't do anything for now. This message has some interesting
             * uses, perhaps a future enhancement.
             */
            break;

        case WM_SYSKEYDOWN :
            /* Sent when the alt key is down.  We need both WM_SYSKEYDOWN and
             * WM_KEYDOWN to catch everything that a keyboard hook catches.
             */
            if (  pKeyData->key[wParam] && !(lParam & KEY_REALEASE) && !(lParam & KEY_WASDOWN) )
            {
                processKeyPress(pKeyData, wParam, lParam, pData->pMessageQueue);
            }
            break;

        case WM_KEYDOWN:
            /* WM_KEYDOWN will never have KEY_RELEASE set. */
            if (  pKeyData->key[wParam] && !(lParam & KEY_WASDOWN) )
            {
                processKeyPress(pKeyData, wParam, lParam, pData->pMessageQueue);
            }
            break;

        case WM_NCDESTROY:
            /* The window is being destroyed, remove the subclass, clean up
             * memory.
             */
            RemoveWindowSubclass(hwnd, KeyPressSubclassProc, id);
            if ( pData )
            {
                freeSubclassData(pData);
            }
            break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

/**
 * Convenience function to remove the key press subclass procedure and free the
 * associated memory.
 *
 */
static BOOL removeKeyPressSubclass(SUBCLASSDATA *pData, HWND hDlg, INT id)
{
    BOOL success = SendMessage(hDlg, WM_USER_SUBCLASS_REMOVE, (WPARAM)&KeyPressSubclassProc, (LPARAM)id) != 0;
    if ( success )
    {
        freeSubclassData(pData);
    }
    return success;
}


static keyPressErr_t connectKeyPressSubclass(RexxMethodContext *c, CSTRING methodName, CSTRING keys, CSTRING filter,
                                             pCDialogControl pcdc)
{
    keyPressErr_t result = nameErr;
    if ( ! requiredComCtl32Version(c, c->GetMessageName(), COMCTL32_6_0) )
    {
        goto done_out;
    }
    if ( *methodName == '\0' )
    {
        c->RaiseException1(Rexx_Error_Invalid_argument_null, TheOneObj);
        goto done_out;
    }
    if ( *keys == '\0' )
    {
        c->RaiseException1(Rexx_Error_Invalid_argument_null, TheTwoObj);
        goto done_out;
    }

    SUBCLASSDATA *pData = NULL;
    BOOL success = GetWindowSubclass(pcdc->hCtrl, KeyPressSubclassProc, pcdc->id, (DWORD_PTR *)&pData);

    // If pData is null, the subclass is not installed.  The data block needs to
    // be allocated and then install the subclass.  Otherwise, just update the
    // data block.
    if ( pData == NULL )
    {
        pData = (SUBCLASSDATA *)LocalAlloc(LPTR, sizeof(SUBCLASSDATA));
        if ( pData == NULL )
        {
            result = memoryErr;
            goto done_out;
        }

        pData->pKeyPressData = (KEYPRESSDATA *)LocalAlloc(LPTR, sizeof(KEYPRESSDATA));
        if ( pData->pKeyPressData == NULL )
        {
            LocalFree(pData);
            result = memoryErr;
            goto done_out;
        }

        pCPlainBaseDialog pcpbd = dlgToCSelf(c, pcdc->oDlg);

        pData->hCtrl = pcdc->hCtrl;
        pData->uID = pcdc->id;
        pData->pMessageQueue = pcpbd->dlgAdm->pMessageQueue;

        result = setKeyPressData(pData->pKeyPressData, methodName, keys, filter);
        if ( result == noErr || result == badFilterErr || result == keyMapErr )
        {
            if ( SendMessage(pcdc->hDlg, WM_USER_SUBCLASS, (WPARAM)KeyPressSubclassProc, (LPARAM)pData) == 0 )
            {
                result = winAPIErr;
            }
        }
        else
        {
            LocalFree(pData);
        }
    }
    else
    {
        if ( success )
        {
            result = setKeyPressData(pData->pKeyPressData, methodName, keys, filter);
        }
        else
        {
            result = winAPIErr;
        }
    }

done_out:
    return result;
}


RexxMethod1(RexxObjectPtr, dlgctrl_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, DIALOGCONTROL_CLASS) )
    {
        TheDialogControlClass = (RexxClassObject)self;
        context->RequestGlobalReference(TheDialogControlClass);
    }
    return NULLOBJECT;
}


/** DialogControl::new()
 *
 *
 */
RexxMethod3(RexxObjectPtr, dlgctrl_new_cls, OPTIONAL_RexxObjectPtr, args, OSELF, self, SUPER, superClass)
{
    RexxMethodContext *c = context;
    RexxObjectPtr control = TheNilObj;

    if ( argumentOmitted(1) || ! c->IsPointer(args) )
    {
        goto done_out;
    }

    // Forwarding this message to the super class will also invoke the init()
    // method of the control instance object.
    control = c->ForwardMessage(NULLOBJECT, NULL, superClass, NULL);
    if ( control == NULLOBJECT )
    {
        control = TheNilObj;
    }

done_out:
    return control;
}

/** DialogControl::init()
 *
 *  The base init() for all dialog control objects.
 *
 *  Initializes the WindowBase and sets the 3 attributes: id, hDlg, and oDlg.
 *  These attributes are 'get' only attributes and can not be changed.
 */
RexxMethod2(uint32_t, dlgctrl_init, OPTIONAL_POINTER, args, OSELF, self)
{
    RexxMethodContext *c = context;
    uint32_t result = 1;

    if ( argumentOmitted(1) || args == NULL )
    {
        goto done_out;
    }

    // Set up for the DialogControl CSelf.
    RexxBufferObject cdcBuf = c->NewBuffer(sizeof(CDialogControl));
    if ( cdcBuf == NULLOBJECT )
    {
        goto done_out;
    }

    // Do the WindowBase initialization.
    pCWindowBase wbCSelf;
    PNEWCONTROLPARAMS params = (PNEWCONTROLPARAMS)args;
    if ( ! initWindowBase(context, params->hwnd, self, &wbCSelf) )
    {
        goto done_out;
    }

    if ( ! initWindowExtensions(context, self, params->hwnd, wbCSelf, NULL) )
    {
        goto done_out;
    }


    pCDialogControl cdcCSelf = (pCDialogControl)c->BufferData(cdcBuf);
    memset(cdcCSelf, 0, sizeof(CDialogControl));

    cdcCSelf->controlType = params->controlType;
    cdcCSelf->lastItem = -1;
    cdcCSelf->wndBase = wbCSelf;
    cdcCSelf->rexxSelf = self;
    cdcCSelf->hCtrl = params->hwnd;
    cdcCSelf->id = params->id;
    cdcCSelf->hDlg = params->hwndDlg;
    cdcCSelf->oDlg = params->parentDlg;
    cdcCSelf->isInCategoryDlg = params->isCatDlg;

    context->SetObjectVariable("CSELF", cdcBuf);

    context->SetObjectVariable("ID", c->UnsignedInt32(params->id));
    context->SetObjectVariable("HDLG", pointer2string(context, params->hwndDlg));
    context->SetObjectVariable("ODLG", params->parentDlg);
    result = 0;

done_out:
    return result;
}

/** DialogControl::unInit()
 *
 *  Release the global reference for CWindowBase::rexxHwnd.
 *
 */
RexxMethod1(RexxObjectPtr, dlgctrl_unInit, CSELF, pCSelf)
{
    if ( pCSelf != NULLOBJECT )
    {
        pCWindowBase pcwb = ((pCDialogControl)pCSelf)->wndBase;
        if ( pcwb->rexxHwnd != TheZeroObj )
        {
            context->ReleaseGlobalReference(pcwb->rexxHwnd);
            pcwb->rexxHwnd = TheZeroObj;
        }
    } return NULLOBJECT;
}

RexxMethod1(RexxObjectPtr, dlgctrl_assignFocus, CSELF, pCSelf)
{
    SendMessage(((pCDialogControl)pCSelf)->hDlg, WM_NEXTDLGCTL, (WPARAM)((pCDialogControl)pCSelf)->hCtrl, TRUE);
    return TheZeroObj;
}

RexxMethod4(int32_t, dlgctrl_connectKeyPress, CSTRING, methodName, CSTRING, keys, OPTIONAL_CSTRING, filter,
            CSELF, pCSelf)
{
    keyPressErr_t result = connectKeyPressSubclass(context, methodName, keys, filter, (pCDialogControl)pCSelf);
    if ( result == memoryErr )
    {
        outOfMemoryException(context->threadContext);
    }
    return -(int32_t)result;
}

RexxMethod2(int32_t, dlgctrl_connectFKeyPress, CSTRING, methodName, CSELF, pCSelf)
{
    keyPressErr_t result = connectKeyPressSubclass(context, methodName, "FKEYS", NULL, (pCDialogControl)pCSelf);
    if ( result == memoryErr )
    {
        outOfMemoryException(context->threadContext);
    }
    return -(int32_t)result;
}

RexxMethod2(int32_t, dlgctrl_disconnectKeyPress, OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    char *tmpName = NULL;
    keyPressErr_t result = winAPIErr;

    if ( ! requiredComCtl32Version(context, context->GetMessageName(), COMCTL32_6_0) )
    {
        goto done_out;
    }

    pCDialogControl pcdc = (pCDialogControl)pCSelf;

    SUBCLASSDATA *pData = NULL;
    BOOL success = GetWindowSubclass(pcdc->hCtrl, KeyPressSubclassProc, pcdc->id, (DWORD_PTR *)&pData);

    // If success, the subclass is still installed, otherwise the subclass has
    // already been removed, (or never existed.)
    if ( success )
    {
        // If no method name, remove the whole thing.
        if ( argumentOmitted(1) )
        {
            result = (removeKeyPressSubclass(pData, pcdc->hDlg, pcdc->id) ? noErr : winAPIErr);
            goto done_out;
        }

        // Have a method name, just remove that method from the mapping.
        tmpName = strdupupr(methodName);
        if ( tmpName == NULL )
        {
            result = memoryErr;
            goto done_out;
        }

        success = FALSE;  // Reuse the success variable.
        uint32_t index = seekKeyPressMethod(pData->pKeyPressData, tmpName);
        if ( index == 0 )
        {
            result = nameErr;
            goto done_out;
        }

        // If only 1 method left, remove the subclass entirely.  Otherwise,
        // remove the subclass, fix up the subclass data block, then reinstall
        // the subclass.
        if ( pData->pKeyPressData->usedMethods == 1 )
        {
            success = removeKeyPressSubclass(pData, pcdc->hDlg, pcdc->id);
        }
        else
        {
            if ( SendMessage(pcdc->hDlg, WM_USER_SUBCLASS_REMOVE, (WPARAM)KeyPressSubclassProc, (LPARAM)pcdc->id) )
            {
                removeKeyPressMethod(pData->pKeyPressData, index);
                success = (BOOL)SendMessage(pcdc->hDlg, WM_USER_SUBCLASS, (WPARAM)KeyPressSubclassProc, (LPARAM)pData);
            }
        }
        result = (success ? noErr : winAPIErr);
    }

done_out:
    return -(int32_t)result;
}

RexxMethod2(logical_t, dlgctrl_hasKeyPressConnection, OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    if ( ComCtl32Version <  COMCTL32_6_0 )
    {
        return FALSE;
    }

    pCDialogControl pcdc = (pCDialogControl)pCSelf;

    SUBCLASSDATA *pData = NULL;
    if ( ! GetWindowSubclass(pcdc->hCtrl, KeyPressSubclassProc, pcdc->id, (DWORD_PTR *)&pData) )
    {
        return FALSE;
    }
    if ( pData == NULL )
    {
        return FALSE;
    }
    if ( argumentOmitted(1) )
    {
        return TRUE;
    }

    char *tmpName = strdupupr(methodName);
    if ( tmpName == NULL )
    {
        outOfMemoryException(context->threadContext);
        return FALSE;
    }

    BOOL exists = (seekKeyPressMethod(pData->pKeyPressData, tmpName) > 0);
    free(tmpName);
    return exists;
}

/** DialogControl::tabstop()
 *  DialogControl::group()
 */
RexxMethod3(RexxObjectPtr, dlgctrl_tabGroup, OPTIONAL_logical_t, addStyle, NAME, method, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCDialogControl pcdc = (pCDialogControl)pCSelf;
    if ( argumentOmitted(1) )
    {
        addStyle = TRUE;
    }
    uint32_t style = GetWindowLong(pcdc->hCtrl, GWL_STYLE);

    if ( *method == 'T' )
    {
        style = (addStyle ? (style | WS_TABSTOP) : (style & ~WS_TABSTOP));
    }
    else
    {
        style = (addStyle ? (style | WS_GROUP) : (style & ~WS_GROUP));
    }
    return setWindowStyle(context, pcdc->hCtrl, style);
}

/** DialogControl::clearRect()
 *
 *  Clears the rectangle specified in this control's client area.
 *
 *  @param  The coordinates of the rectangle.
 *    Form 1:  A .Rect object.
 *    Form 2:  A .Point object and a .Point object.
 *    Form 3:  x1, y1, y1, y2
 *
 *  @return  0 on success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, dlgctrl_clearRect, ARGLIST, args, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RECT r = {0};
    size_t arraySize;
    size_t argsUsed;

    if ( ! getRectFromArglist(context, args, &r, true, 1, 4, &arraySize, &argsUsed) )
    {
        return TheOneObj;
    }
    if ( argsUsed < arraySize )
    {
        return tooManyArgsException(context->threadContext, argsUsed);
    }

    return clearRect(context, getDChCtrl(pCSelf), &r);
}

/** DialogControl::redrawRect()
 *
 *  Immediately redraws the specified rectangle in this control.
 *
 *  @param  The coordinates of the rectangle.
 *    Form 1:  A .Rect object.
 *    Form 2:  A .Point object and a .Point object.
 *    Form 3:  x1, y1, y1, y2
 *
 *  @param erase  [OPITONAL]  Whether the background should be erased first.
 *                The default is false.
 *
 *  @return  0 on success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, dlgctrl_redrawRect, ARGLIST, args, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    bool doErase = false;
    RECT r = {0};
    size_t arraySize;
    size_t argsUsed;

    if ( ! getRectFromArglist(context, args, &r, true, 1, 5, &arraySize, &argsUsed) )
    {
        return TheOneObj;
    }

    if ( arraySize > argsUsed + 1 )
    {
        return tooManyArgsException(context->threadContext, argsUsed + 1);
    }
    else if ( arraySize == (argsUsed + 1) )
    {
        // The object at argsUsed + 1 has to exist, otherwise arraySize would
        // equal argsUsed.
        RexxObjectPtr obj = context->ArrayAt(args, argsUsed + 1);

        logical_t erase;
        if ( ! context->Logical(obj, &erase) )
        {
            return notBooleanException(context->threadContext, argsUsed + 2, obj);
        }
        doErase = erase ? true : false;
    }

    return redrawRect(context, getDChCtrl(pCSelf), &r, doErase);
}

/** DialogControl::getTextSizeDlg()
 *
 *  Gets the size (width and height) in dialog units for any given string.
 *
 *  Since dialog units only have meaning for a specific dialog, normally the
 *  dialog units are calculated using the font of the dialog.  Optionally, this
 *  method will calculate the dialog units using a specified font.
 *
 *  @param  text         The string whose size is needed.
 *
 *  @param  fontName     Optional. If specified, use this font to calculate the
 *                       size.  The default is to use the font of the owner
 *                       dialog of the dialog control.  This would be the normal
 *                       usage.
 *
 *  @param  fontSize     Optional. If specified, use this font size with
 *                       fontName to calculate the size.  The default if omitted
 *                       is 8.  This arg is ignored if fontName is omitted.
 *
 *  @param  hwndFontSrc  Optional. Use this window's font to calculate the size.
 *                       This arg is always ignored if fontName is specified.
 *
 *  @return  A .Size object representing the area (width and height,) in dialog
 *           units, needed for the specified string.
 *
 *  @note  This method, mapped to a dialog control object, does not really make
 *         sense.  It, and its convoluted optional arguments, are maintained
 *         only for backward compatibility.  Its use should be strongly
 *         discouraged.
 */
RexxMethod5(RexxObjectPtr, dlgctrl_getTextSizeDlg, CSTRING, text, OPTIONAL_CSTRING, fontName,
            OPTIONAL_uint32_t, fontSize, OPTIONAL_POINTERSTRING, hwndFontSrc, CSELF, pCSelf)
{
    HWND hwndSrc = NULL;
    if ( argumentExists(2) )
    {
        if ( argumentOmitted(3) )
        {
            fontSize = DEFAULT_FONTSIZE;
        }
    }
    else if ( argumentExists(4) )
    {
        hwndSrc = (HWND)hwndFontSrc;
        if ( hwndFontSrc == NULL )
        {
            nullObjectException(context->threadContext, "window handle", 4);
            return NULLOBJECT;
        }
    }

    SIZE textSize = {0};
    if ( getTextSize(context, text, fontName, fontSize, hwndSrc, getDCownerDlg(pCSelf), &textSize) )
    {
        return rxNewSize(context, textSize.cx, textSize.cy);
    }
    return NULLOBJECT;
}


/** DialogControl::captureMouse
 *
 *  Sets the mouse capture to this dialog control.  captureMouse() captures
 *  mouse input either when the mouse is over the control, or when the mouse
 *  button was pressed while the mouse was over the control and the button is
 *  still down. Only one window at a time can capture the mouse.
 *
 *  If the mouse cursor is over a window created by another thread, the system
 *  will direct mouse input to the specified window only if a mouse button is
 *  down.
 *
 *  @return  The window handle of the window that previously had captured the
 *           mouse, or the null handle if there was no such window.
 */
RexxMethod1(RexxObjectPtr, dlgctrl_captureMouse, CSELF, pCSelf)
{
    HWND oldCapture = (HWND)SendMessage(getDChDlg(pCSelf), WM_USER_GETSETCAPTURE, 1, (LPARAM)getDChCtrl(pCSelf));
    return pointer2string(context, oldCapture);
}

/** DialogControl::setColor()
 *  DialogControl::setSysColor
 */
RexxMethod4(logical_t, dlgctrl_setColor, int32_t, bkColor, OPTIONAL_int32_t, fgColor, NAME, method, CSELF, pCSelf)
{
    DIALOGADMIN *dlgAdm = getDCDlgAdm(context, (pCDialogControl)pCSelf);
    if ( dlgAdm == NULL )
    {
        return 1;
    }
    return oodColorTable(context, dlgAdm, ((pCDialogControl)pCSelf)->id, bkColor,
                         (argumentOmitted(2) ? -1 : fgColor), (method[3] == 'S'));
}

/** DialogControl::value()
 *
 *  Gets the "value" of the dialog control.
 *
 *  @return  The 'value' or 'data' of the control.
 *
 *  @remarks  The original ooDialog code used the abstraction that there were
 *            only two objects involved.  The ooDialog object and the underlying
 *            Windows dialog.  The dialog controls were considered to be the
 *            'data' of the underlying Windows dialog.  In this abstraction, an
 *            edit control was part of the 'data' of the dialog and its 'value'
 *            was the edit control's text.  For a check box the 'value' is
 *            checked or not, etc..
 *
 */
RexxMethod1(RexxObjectPtr, dlgctrl_value, CSELF, pCSelf)
{
    pCDialogControl pcdc = (pCDialogControl)pCSelf;

    return getControlData(context, dlgToCSelf(context, pcdc->oDlg), pcdc->id, pcdc->hDlg, pcdc->controlType);
}

/** DialogControl::"value="
 *
 *  Sets the "value" of the dialog control.
 *
 *  @param  data  What to set the value of the dialog control to.  Its meaning
 *                and format are dependent on the type of control.
 *
 *  @return  0 on success, 1 on error.
 *
 *  @remarks  See the remarks in dlgctrl_value above.
 */
RexxMethod2(int32_t, dlgctrl_valueEquals, CSTRING, data, CSELF, pCSelf)
{
    pCDialogControl pcdc = (pCDialogControl)pCSelf;

    return setControlData(context, dlgToCSelf(context, pcdc->oDlg), pcdc->id, data, pcdc->hDlg, pcdc->controlType);
}


