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
 * oodControl.cpp
 *
 * Contains the base classes used for an object that represents a Windows
 * Control.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <shlwapi.h>
#include <OleAcc.h>
#include <uxtheme.h>
#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodMessaging.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodData.hpp"
#include "oodMouse.hpp"
#include "oodShared.hpp"
#include "oodControl.hpp"

const char *controlType2winName(oodControl_t control)
{
    switch ( control )
    {
        case winCheckBox :             return WC_BUTTON;
        case winComboBox :             return WC_COMBOBOX;
        case winComboLBox :            return "ComboLBox";
        case winDateTimePicker :       return DATETIMEPICK_CLASS;
        case winEdit :                 return WC_EDIT;
        case winGroupBox :             return WC_BUTTON;
        case winListBox :              return WC_LISTBOX;
        case winListView :             return WC_LISTVIEW;
        case winMonthCalendar :        return MONTHCAL_CLASS;
        case winProgressBar :          return PROGRESS_CLASS;
        case winPushButton :           return WC_BUTTON;
        case winRadioButton :          return WC_BUTTON;
        case winReBar :                return REBARCLASSNAME;
        case winScrollBar :            return WC_SCROLLBAR;
        case winStatic :               return WC_STATIC;
        case winStatusBar :            return STATUSCLASSNAME ;
        case winTab :                  return WC_TABCONTROL;
        case winToolBar :              return TOOLBARCLASSNAME;
        case winTreeView :             return WC_TREEVIEW;
        case winTrackBar :             return TRACKBAR_CLASS;
        case winUpDown :               return UPDOWN_CLASS;
        default :                      return "";
    }
}


const char *controlType2className(oodControl_t control)
{
    switch ( control )
    {
        case winCheckBox :             return "CHECKBOX";
        case winComboBox :             return "COMBOBOX";
        case winComboLBox :            return "LISTBOX";
        case winDateTimePicker :       return "DATETIMEPICKER";
        case winEdit :                 return "EDIT";
        case winGroupBox :             return "GROUPBOX";
        case winListBox :              return "LISTBOX";
        case winListView :             return "LISTVIEW";
        case winMonthCalendar :        return "MONTHCALENDAR";
        case winProgressBar :          return "PROGRESSBAR";
        case winPushButton :           return "BUTTON";
        case winRadioButton :          return "RADIOBUTTON";
        case winReBar :                return "REBAR";
        case winScrollBar :            return "SCROLLBAR";
        case winStatic :               return "STATIC";
        case winStatusBar :            return "STATUSBAR";
        case winTab :                  return "TAB";
        case winToolBar :              return "TOOLBAR";
        case winToolTip :              return "TOOLTIP";
        case winTreeView :             return "TREEVIEW";
        case winTrackBar :             return "TRACKBAR";
        case winUpDown :               return "UPDOWN";
        default :                      return "";
    }
}


const char *controlType2controlName(oodControl_t control)
{
    switch ( control )
    {
        case winCheckBox :             return "CheckBox";
        case winComboBox :             return "ComboBox";
        case winComboLBox :            return "ComboLBox";
        case winDateTimePicker :       return "DateTimePicker";
        case winEdit :                 return "Edit";
        case winGroupBox :             return "GroupBox";
        case winListBox :              return "ListBox";
        case winListView :             return "ListView";
        case winMonthCalendar :        return "MonthCalendar";
        case winProgressBar :          return "ProgressBar";
        case winPushButton :           return "PushButton";
        case winRadioButton :          return "RadioButton";
        case winReBar :                return "ReBar";
        case winScrollBar :            return "ScrollBar";
        case winStatic :               return "Static";
        case winStatusBar :            return "StatusBar";
        case winTab :                  return "Tab";
        case winToolBar :              return "ToolBar";
        case winToolTip :              return "ToolTip";
        case winTrackBar :             return "TrackBar";
        case winTreeView :             return "TreeView";
        case winUpDown :               return "UpDown";
        default :                      return "";
    }
}


oodControl_t winName2controlType(const char *className)
{
    if      ( strcmp(className, WC_COMBOBOX       ) == 0 ) return winComboBox;
    else if ( strcmp(className, "ComboLBox"       ) == 0 ) return winComboLBox;
    else if ( strcmp(className, DATETIMEPICK_CLASS) == 0 ) return winDateTimePicker;
    else if ( strcmp(className, WC_EDIT           ) == 0 ) return winEdit;
    else if ( strcmp(className, WC_LISTBOX        ) == 0 ) return winListBox;
    else if ( strcmp(className, WC_LISTVIEW       ) == 0 ) return winListView;
    else if ( strcmp(className, MONTHCAL_CLASS    ) == 0 ) return winMonthCalendar;
    else if ( strcmp(className, PROGRESS_CLASS    ) == 0 ) return winProgressBar;
    else if ( strcmp(className, WC_BUTTON         ) == 0 ) return winPushButton;
    else if ( strcmp(className, REBARCLASSNAME    ) == 0 ) return winReBar;
    else if ( strcmp(className, WC_SCROLLBAR      ) == 0 ) return winScrollBar;
    else if ( strcmp(className, WC_STATIC         ) == 0 ) return winStatic;
    else if ( strcmp(className, STATUSCLASSNAME   ) == 0 ) return winStatusBar;
    else if ( strcmp(className, WC_TABCONTROL     ) == 0 ) return winTab;
    else if ( strcmp(className, TOOLBARCLASSNAME  ) == 0 ) return winToolBar;
    else if ( strcmp(className, TOOLTIPS_CLASS    ) == 0 ) return winToolTip;
    else if ( strcmp(className, TRACKBAR_CLASS    ) == 0 ) return winTrackBar;
    else if ( strcmp(className, WC_TREEVIEW       ) == 0 ) return winTreeView;
    else if ( strcmp(className, UPDOWN_CLASS      ) == 0 ) return winUpDown;
    else if ( strcmp(className, "#32770"          ) == 0 ) return winDialog;
    else
    {
        return winUnknown;
    }
}

oodControl_t controlHwnd2controlType(HWND hControl)
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

oodControl_t controlName2controlType(CSTRING name)
{
    if      ( StrCmpI(name, "CHECKBOX"      ) == 0 ) return winCheckBox;
    else if ( StrCmpI(name, "COMBOBOX"      ) == 0 ) return winComboBox;
    else if ( StrCmpI(name, "COMBOLBOX"     ) == 0 ) return winComboLBox;
    else if ( StrCmpI(name, "DATETIMEPICKER") == 0 ) return winDateTimePicker;
    else if ( StrCmpI(name, "EDIT"          ) == 0 ) return winEdit;
    else if ( StrCmpI(name, "GROUPBOX"      ) == 0 ) return winGroupBox;
    else if ( StrCmpI(name, "LISTBOX"       ) == 0 ) return winListBox;
    else if ( StrCmpI(name, "LISTVIEW"      ) == 0 ) return winListView;
    else if ( StrCmpI(name, "MONTHCALENDAR" ) == 0 ) return winMonthCalendar;
    else if ( StrCmpI(name, "PROGRESSBAR"   ) == 0 ) return winProgressBar;
    else if ( StrCmpI(name, "PUSHBUTTON"    ) == 0 ) return winPushButton;
    else if ( StrCmpI(name, "RADIOBUTTON"   ) == 0 ) return winRadioButton;
    else if ( StrCmpI(name, "REBAR"         ) == 0 ) return winReBar;
    else if ( StrCmpI(name, "SCROLLBAR"     ) == 0 ) return winScrollBar;
    else if ( StrCmpI(name, "STATIC"        ) == 0 ) return winStatic;
    else if ( StrCmpI(name, "STATUSBAR"     ) == 0 ) return winStatusBar;
    else if ( StrCmpI(name, "TAB"           ) == 0 ) return winTab;
    else if ( StrCmpI(name, "TOOLBAR"       ) == 0 ) return winToolBar;
    else if ( StrCmpI(name, "TOOLTIP"       ) == 0 ) return winToolTip;
    else if ( StrCmpI(name, "TRACKBAR"      ) == 0 ) return winTrackBar;
    else if ( StrCmpI(name, "TREEVIEW"      ) == 0 ) return winTreeView;
    else if ( StrCmpI(name, "UPDOWN"        ) == 0 ) return winUpDown;
    else return winUnknown;
}

RexxStringObject controlWindow2rexxString(RexxMethodContext *c, HWND hControl)
{
    RexxStringObject result = c->NullString();

    TCHAR buf[512];
    if ( RealGetWindowClass(hControl, buf, sizeof(buf)) == 0)
    {
        _snprintf(buf, sizeof(buf), "Unknown control System Error Code: %d\n", GetLastError());
    }
    return c->String(buf);
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
 * Determine if a dialog control belongs to the specified dialog control class.
 *
 * @param hDlg     Handle to the owner dialog of the control, the assumed owner.
 * @param   id     Resource ID of the dialog control/
 * @param control  One of the oodControl types specifying the class to check
 *                 for.
 *
 * @return True if the dialog control is the type specified, otherwise false.
 */
bool isControlMatch(HWND hDlg, uint32_t id, oodControl_t control)
{
    HWND hControl = GetDlgItem(hDlg, id);

    if ( hControl != NULL )
    {
        return isControlMatch(hControl, control);
    }
    return false;
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
 * @return The windows control type.  winUnknown is returned if there is no
 *         match.
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
    else if ( StrCmpN(name, "RADIOBUTTON", 2   ) == 0 ) return winRadioButton;
    else if ( StrCmpN(name, "REBAR", 2         ) == 0 ) return winReBar;
    else if ( StrCmpN(name, "SCROLLBAR", 2     ) == 0 ) return winScrollBar;
    else if ( StrCmpN(name, "STATIC", 5        ) == 0 ) return winStatic;
    else if ( StrCmpN(name, "STATUSBAR", 5     ) == 0 ) return winStatusBar;
    else if ( StrCmpN(name, "TAB", 3           ) == 0 ) return winTab;
    else if ( StrCmpN(name, "TOOLB", 5         ) == 0 ) return winToolBar;
    else if ( StrCmpN(name, "TOOLT", 5         ) == 0 ) return winToolTip;
    else if ( StrCmpN(name, "TRACKBAR", 3      ) == 0 ) return winTrackBar;
    else if ( StrCmpN(name, "TREEVIEW", 3      ) == 0 ) return winTreeView;
    else if ( StrCmpN(name, "UPDOWN", 1        ) == 0 ) return winUpDown;
    else return winUnknown;
}

/**
 * Returns the Rexx control class for the specified ooDialog control type and
 * does not raise an exception on error.
 *
 * @param c             Method context we are executing in.
 * @param controlType   ooDialog control type
 *
 * @return  The Rexx class object on success and NULLOBJECT on error.
 */
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
 * Returns the Rexx control class for the specified ooDialog control type and
 * raises an exception on error.
 *
 * @param controlType   ooDialog control type
 * @param c             Method context we are executing in.
 *
 * @return  The Rexx class object on success and NULLOBJECT on error.
 */
RexxClassObject oodClass4controlType(oodControl_t controlType, RexxMethodContext *c)
{
    const char *className = controlType2className(controlType);
    return rxGetContextClass(c, className);
}


/**
 * Returns the Rexx control class for the specified ooDialog control type and
 * does not raise an exception on error.
 *
 * This function is for use from a thread context because the FindContextClass()
 * API is not available.
 *
 * @param c             Thread context we are executing in.
 * @param controlType   ooDialog control type
 *
 * @return  The Rexx class object on success and NULLOBJECT on error.
 */
RexxClassObject oodClass4controlType(RexxThreadContext *c, oodControl_t controlType)
{
    RexxClassObject controlClass = NULLOBJECT;
    const char *className = controlType2className(controlType);

    controlClass = rxGetPackageClass(c, "ooDialog.cls", className);
    if ( controlClass == NULLOBJECT )
    {
        // An exception has been raised, which we don't want.  So, clear it.
        c->ClearCondition();
    }
    return controlClass;
}


/**
 * Produce a string representation of an "object state."
 *
 * Windows Accessibility uses "Object State Constants" to describe the states of
 * objects. An object is associated with one or more of these state values at
 * any time.
 *
 * This is used in a few places in ooDialog.  MSDN is not explicit in describing
 * what state constants are valid in these places.
 *
 * This function retrieves a localized string that describes an object's state,
 * exactly what that string will be for any given situation, I don't know.
 *
 * @remarks  In testing DateTimePicker::getInfo() I see the state is equal to 0
 *           quite often.  Passing in 0 to the GetStateText() function returns a
 *           string -> "normal".  So, we will consider that valid.
 */
RexxStringObject objectStateToString(RexxMethodContext *c, uint32_t state)
{
    char result[512];
    result[0] = '\0';

    char stateStr[256];
    stateStr[0] = '\0';

    if ( state == 0 )
    {
        GetStateText(state, result, sizeof(result));
    }
    else
    {
        // We need mask to be valid when we enter the for loop.
        uint32_t mask = 1;
        for ( uint32_t i = 1; i < 32 && (mask & STATE_SYSTEM_VALID); i++ )
        {
            if ( mask & state )
            {
                if ( GetStateText(mask & state, stateStr, sizeof(stateStr)) > 0 )
                {
                    if ( strlen(result) > 0 )
                    {
                        strcat(result, ", ");
                    }
                    strcat(result, stateStr);
                }
            }
            mask = 1 << i;
        }
    }

    return c->String(result);
}


/**
 * Creates the Rexx dialog control object that represents the underlying Windows
 * dialog control.
 *
 * The control object can, almost, be created entirely from within the C / C++
 * environment.  A method context and the Rexx parent dialog are needed.
 *
 * @param c               Thread context we are operating in.
 * @param hControl
 * @param hDlg
 * @param id
 * @param controlType
 * @param self            Rexx dialog object that contains the dialog control.
 * @param isCategoryDlg
 * @param putInBag
 *
 * @return RexxObjectPtr
 */
RexxObjectPtr createRexxControl(RexxThreadContext *c, HWND hControl, HWND hDlg, uint32_t id, oodControl_t controlType,
                                RexxObjectPtr self, RexxClassObject controlCls, bool isCategoryDlg, bool putInBag)
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
        outOfMemoryException(c);
        goto out;
    }

    pCPlainBaseDialog pcpbd = dlgToCSelf(c, self);

    pArgs->isCatDlg    = isCategoryDlg;
    pArgs->controlType = controlType;
    pArgs->hwnd        = hControl;
    pArgs->pcpbd       = pcpbd;
    pArgs->id          = id;

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
 * Creates a Rexx dialog control from within a dialog method, using the window
 * handle of the control.
 *
 * In the Windows API it is easy to get the window handle of a control within a
 * dialog.  Normally we create Rexx dialog control objects from within Rexx
 * code, but it is convenient to be able to create the Rexx dialog control from
 * within native code.
 *
 * For instance, the Windows PropertySheet API gives you access to the window
 * handle of the tab control within the property sheet. From that handle we want
 * to be able to create a Rexx dialog control object to pass back into the Rexx
 * code.  Actually, this is the only case so far, but the code is made generic
 * in the assumption that other uses will come up.
 *
 * @param c         The method context we are operating in.
 * @param pcpbd     The CSelf struct of the dialog the control resides in.
 * @param hCtrl     The window handle of the companion control.  This can be
 *                  null, in which case .nil is returned.
 * @param type      The type of the dialog control.
 * @param putInBag  Whether the new Rexx dialog control object should be
 *                  protected from garbage collection by being placed in the
 *                  Rexx dialog object's control bag.
 *
 * @return A Rexx dialog control object that represents the dialo control, or
 *         .nil if the object is not instantiated.
 *
 * @remarks  The second from the last argument to createRexxControl() is true if
 *           the parent dialog is a CategoryDialog, otherwise false.  Since the
 *           CategoryDialog is now deprecated, we just pass false.
 */
RexxObjectPtr createControlFromHwnd(RexxMethodContext *c, pCPlainBaseDialog pcpbd, HWND hCtrl,
                                    oodControl_t type, bool putInBag)
{
    RexxObjectPtr result = TheNilObj;

    if ( hCtrl == NULL )
    {
        goto done_out;
    }

    uint32_t id = (uint32_t)GetDlgCtrlID(hCtrl);

    RexxClassObject controlCls = oodClass4controlType(c, type);
    if ( controlCls == NULLOBJECT )
    {
        goto done_out;
    }

    result = createRexxControl(c->threadContext, hCtrl, pcpbd->hDlg, id, type, pcpbd->rexxSelf, controlCls, false, putInBag);

done_out:
    return result;
}


/**
 * Creates a Rexx dialog control from within a thread context, using the window
 * handle of the control.
 *
 * In the Windows API it is easy to get the window handle of a control within a
 * dialog.  Normally we create Rexx dialog control objects from within Rexx
 * code, but it is convenient to be able to create the Rexx dialog control from
 * within native code.
 *
 * This function is similar to the other createControlFromHwnd() functions, but
 * it uses a thread context rather than a method context.  This allows it to be
 * used from inside a Windows dialog procedure where there is no method context
 * available.
 *
 * @param c         The thread context we are operating in.
 * @param pcpbd     The CSelf struct of the dialog the control resides in.
 * @param hCtrl     The window handle of the dialog control.  This can be
 *                  null, in which case .nil is returned, making it safe to use
 *                  without first checking the handle.
 * @param type      The type of the dialog control.
 * @param putInBag  Whether the new Rexx dialog control object should be
 *                  protected from garbage collection by being placed in the
 *                  Rexx dialog object's control bag.
 *
 * @return A Rexx dialog control object that represents the dialo control, or
 *         .nil if the object is not instantiated.
 *
 * @remarks  The second from the last argument to createRexxControl() is true if
 *           the parent dialog is a CategoryDialog, otherwise false.  Since the
 *           CategoryDialog is now deprecated, we just pass false.
 */
RexxObjectPtr createControlFromHwnd(RexxThreadContext *c, pCPlainBaseDialog pcpbd, HWND hCtrl,
                                    oodControl_t type, bool putInBag)
{
    RexxObjectPtr result = TheNilObj;

    if ( hCtrl == NULL )
    {
        goto done_out;
    }

    uint32_t id = (uint32_t)GetDlgCtrlID(hCtrl);

    RexxClassObject controlCls = oodClass4controlType(c, type);
    if ( controlCls == NULLOBJECT )
    {
        goto done_out;
    }

    result = createRexxControl(c, hCtrl, pcpbd->hDlg, id, type, pcpbd->rexxSelf, controlCls, false, putInBag);

done_out:
    return result;
}


/**
 * Creates a Rexx dialog control from within a dialog control method, using a
 * window handle of another control.
 *
 * In the Windows API for dialog controls, it is often possible to obtain the
 * handle of a "buddy" or "companion" control of a control.  For instance, with
 * the date time picker control, it is possible to obtain the window handle of
 * the drop down month calendar control. The 'originating' control is the date
 * time picker and the companion control is the month calendar.
 *
 * In these cases, this function will convert the window handle to a Rexx dialog
 * control object.
 *
 * @param c         The method context we are operating in.
 * @param pcdc      The CSelf struct of the originating control.
 * @param hCtrl     The window handle of the dialog control.  This can be null,
 *                  in which case .nil is returned.
 * @param type      The type of the dialog control.
 * @param putInBag  Whether the new Rexx dialog control object should be
 *                  protected from garbage collection by being placed in the
 *                  Rexx dialog object's control bag.
 *
 * @return A Rexx dialog control object that represents the underlying Windows
 *         dialog control, or .nil if the object is not instantiated.
 */
RexxObjectPtr createControlFromHwnd(RexxMethodContext *c, pCDialogControl pcdc, HWND hCtrl,
                                    oodControl_t type, bool putInBag)
{
    RexxObjectPtr result = TheNilObj;

    if ( hCtrl == NULL )
    {
        goto done_out;
    }

    RexxClassObject controlCls = oodClass4controlType(c, type);
    if ( controlCls == NULLOBJECT )
    {
        goto done_out;
    }

    bool     isCategoryDlg = (c->IsOfType(pcdc->oDlg, "CATEGORYDIALOG") ? true : false);
    uint32_t id = (uint32_t)GetDlgCtrlID(hCtrl);

    result = createRexxControl(c->threadContext, hCtrl, pcdc->hDlg, id, type, pcdc->oDlg, controlCls, isCategoryDlg, putInBag);

done_out:
    return result;
}


/**
 *  If the user stores a Rexx object in the user data storage of a dialog
 *  control, the Rexx object could be garbage collected because no Rexx object
 *  has a reference to it.  To prevent that we put the Rexx object in a bag that
 *  is an attribute of the dialog control object.
 *
 * @param c
 * @param pcdc
 * @param data
 *
 * @notes  This function could have been called maybeProtectControlUserData()
 *         because it only stores a Rexx object if the data is not .nil and not
 *         null.
 */
void protectControlUserData(RexxMethodContext *c, pCDialogControl pcdc, RexxObjectPtr data)
{
    if ( data != TheNilObj && data != NULLOBJECT )
    {
        if ( pcdc->rexxBag == NULL )
        {
            c->SendMessage1(pcdc->rexxSelf, "PUTINBAG", data);
        }
        else
        {
            c->SendMessage1(pcdc->rexxBag, "PUT", data);
        }
    }
}


/**
 * Removes a Rexx object from the dialog control's Rexx bag.
 *
 * @param c
 * @param pcdc
 * @param oldUserData
 */
void unProtectControlUserData(RexxMethodContext *c, pCDialogControl pcdc, RexxObjectPtr oldUserData)
{
    if ( oldUserData != TheNilObj && oldUserData != NULLOBJECT && pcdc->rexxBag != NULLOBJECT )
    {
        c->SendMessage1(pcdc->rexxBag, "REMOVE", oldUserData);
    }
}


/**
 *  Protects some Rexx object related to a dialog control from garbage
 *  collection by putting it in a Rexx bag.
 *
 *  This is similar to protectControlUserData(), but more generic.  For
 *  instance, it is used for ToolTip objects of a list-view or tree-view.
 *
 * @param c
 * @param pcdc
 * @param obj
 *
 * @notes
 */
void protectControlObject(RexxMethodContext *c, pCDialogControl pcdc, RexxObjectPtr obj)
{
    if ( obj != TheNilObj && obj != NULLOBJECT )
    {
        if ( pcdc->rexxBag == NULL )
        {
            c->SendMessage1(pcdc->rexxSelf, "PUTINBAG", obj);
        }
        else
        {
            c->SendMessage1(pcdc->rexxBag, "PUT", obj);
        }
    }
}


/**
 * Removes all instances of a single Rexx object from this dialog control's Rexx
 * bag.
 *
 * @param c
 * @param pcdc
 * @param oldUserData
 *
 * @note  We use remove all here to remove all of the specified items in the
 *        bag.
 */
void unProtectControlObject(RexxMethodContext *c, pCDialogControl pcdc, RexxObjectPtr obj)
{
    if ( obj != TheNilObj && obj != NULLOBJECT && pcdc->rexxBag != NULLOBJECT )
    {
        c->SendMessage1(pcdc->rexxBag, "REMOVEALL", obj);
    }
}


/**
 *  Methods for the .DialogControl class.
 */
#define DIALOGCONTROL_CLASS        "DialogControl"

/**
 * Tests if the key code is considered an extended key for the purposes of
 * connectCharEvent().
 *
 * Note that PageUp is VK_PRIOR and PageDown is VK_NEXT.
 *
 * @param wParam
 *
 * @return bool
 */
static inline bool isExtendedKeyEvent(WPARAM wParam)
{
    return (wParam >= VK_PRIOR && wParam <= VK_DOWN) || wParam == VK_INSERT || wParam == VK_DELETE;
}

/**
 * Free the subclass data used for the ControlSubclassProc().
 *
 * @param pData  Struct to free.
 */
void freeSubclassData(pSubClassData p)
{
    EnterCriticalSection(&crit_sec);

    if ( p != NULL )
    {
        if ( p->pData != NULL && p->pfn != NULL )
        {
            pfnFreeSubclassData extraFree = (pfnFreeSubclassData)p->pfn;
            extraFree(p);
        }

        size_t i;

        for ( i = 0; i < p->mNextIndex; i++ )
        {
            safeLocalFree(p->msgs[i].rexxMethod);
        }

        safeLocalFree(p->msgs);
        p->msgs = NULL;
        p->mSize = 0;
        p->mNextIndex = 0;
        if ( p->pcdc != NULL )
        {
            p->pcdc->pscd = NULL;
        }

        LocalFree(p);
    }

    LeaveCriticalSection(&crit_sec);
}

/**
 * Parses options used for subclassing a dialog control and connecting an event
 * notification to a method in the Rexx dialog.  These options allow some,
 * small, control over how the subclass window procedure handles the a window
 * message.
 *
 * An exception is raised if the keyword string does not contain any recognized
 * keyword.
 *
 * @param c
 * @param opts
 * @param tag
 * @param argPos
 *
 * @return True on succes, false if an exception has been raised.
 */
bool parseTagOpts(RexxThreadContext *c, CSTRING opts, uint32_t *pTag, size_t argPos)
{
    bool     foundKeyWord = true;
    uint32_t tag      = *pTag;

    if ( StrCmpI(opts, "SENDTODLG" ) == 0 )
    {
        tag |= CTRLTAG_SENDTODLG;
        foundKeyWord = true;
    }
    else if ( StrCmpI(opts, "SENDTOCONTROL" ) == 0 )
    {
        tag |= CTRLTAG_SENDTOCONTROL;
        foundKeyWord = true;
    }
    else if ( StrCmpI(opts, "REPLYZERO" ) == 0 )
    {
        tag |= CTRLTAG_REPLYZERO;
        foundKeyWord = true;
    }
    else if ( StrCmpI(opts, "REPLYTRUE" ) == 0 )
    {
        tag |= CTRLTAG_REPLYTRUE;
        foundKeyWord = true;
    }
    else if ( StrCmpI(opts, "REPLYFALSE" ) == 0 )
    {
        tag |= CTRLTAG_REPLYFALSE;
        foundKeyWord = true;
    }
    else if ( StrCmpI(opts, "DEFWINPROC" ) == 0 )
    {
        tag |= CTRLTAG_SENDTODEFWINDOWPROC;
        foundKeyWord = true;
    }

    if ( StrCmpI(opts, "NOWAIT" ) == 0 )
    {
        tag &= ~CTRLTAG_REPLYFROMREXX;
        foundKeyWord = true;
    }

    // That's all we have for now, more possibilities may be added in the
    // future.
    if ( foundKeyWord )
    {
        *pTag = tag;
    }
    else
    {
        wrongArgValueException(c, argPos, SUBCLASS_TAG_KEYWORDS, opts);
    }

    return foundKeyWord;
}

/**
 *
 *
 * @param pData
 * @param method
 * @param args
 * @param hwnd
 * @param msg
 * @param wParam
 * @param lParam
 *
 * @return LRESULT
 *
 * @note  We are responsible for releasing the local references in args.
 */
static LRESULT charReply(pSubClassData pData, char *method, RexxArrayObject args,
                         HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam)
{
    RexxThreadContext *c = pData->pcpbd->dlgProcContext;

    LRESULT ret;

    RexxObjectPtr reply = c->SendMessage(pData->pcpbd->rexxSelf, method, args);
    if ( ! checkForCondition(c, false) && reply != NULLOBJECT )
    {
        if ( reply == TheFalseObj || isInt(0, reply, c) )
        {
            // Swallow the message.
            ret = 0;
        }
        else if ( reply == TheTrueObj || isInt(1, reply, c) )
        {
            ret = DefSubclassProc(hwnd, msg, wParam, lParam);
        }
        else
        {
            // When it is neither true or false, the reply should be a charcter
            // that replaces the char with the char sent back to us.
            uint32_t chr;
            if ( c->UnsignedInt32(reply, &chr) )
            {
                return DefSubclassProc(hwnd, msg, (WPARAM)chr, lParam);
            }
            else
            {
                ret = DefSubclassProc(hwnd, msg, wParam, lParam);
            }
        }
    }
    else
    {
        // On errors:
        ret = DefSubclassProc(hwnd, msg, wParam, lParam);
    }

    if ( reply != NULLOBJECT )
    {
        c->ReleaseLocalReference(reply);
    }
    releaseKeyEventRexxArgs(c, args);

    return ret;
}

/**
 * Process a window message to a subclassed dialog control.  The message passed
 * the filters used to connect a message to a Rexx method in the dialog.
 *
 * @param msg
 * @param wParam
 * @param lParam
 * @param pData
 * @param method
 * @param tag
 *
 * @return LRESULT
 */
static LRESULT processControlMsg(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam,
                                 pSubClassData pData, char *method, uint32_t tag)
{
    RexxThreadContext *c = pData->pcpbd->dlgProcContext;

    switch ( tag & CTRLTAG_MASK )
    {
        case CTRLTAG_NOTHING :
            break;

        case CTRLTAG_COMBOBOX :
        {
            switch ( tag & CTRLTAG_FLAGMASK )
            {
                case CTRLTAG_ISGRANDCHILD :
                    return grandchildEvent(pData, method, hwnd, msg, wParam, lParam, tag);
                    break;

                case CTRLTAG_COLORS :
                    return comboBoxColor(pData, hwnd, msg, wParam, lParam, tag);
                    break;

                default :
                    break;
            }
            break;
        }

        case CTRLTAG_EDIT :
        {
            if ( tag & CTRLTAG_WANTRETURN )
            {
                if ( msg == WM_GETDLGCODE )
                {
                    return (DLGC_WANTALLKEYS | DefSubclassProc(hwnd, msg, wParam, lParam));
                }
                else if ( msg == WM_KEYDOWN )
                {
                    switch ( wParam )
                    {
                        case VK_RETURN :
                        {
                            RexxObjectPtr   ctrlID = c->UnsignedInt32(pData->id);
                            RexxArrayObject args   = c->ArrayOfTwo(ctrlID, pData->pcdc->rexxSelf);
                            RexxObjectPtr   reply  = c->SendMessage(pData->pcpbd->rexxSelf, method, args);

                            if ( ! checkForCondition(c, false) && reply != NULLOBJECT )
                            {
                                c->ReleaseLocalReference(ctrlID);
                                c->ReleaseLocalReference(args);
                                c->ReleaseLocalReference(reply);

                                return 0;
                            }
                            else
                            {
                                // On error return DefSubclassProc()
                                return DefSubclassProc(hwnd, msg, wParam, lParam);
                            }
                        }

                        case VK_ESCAPE :
                        {
                            SendMessage(pData->pcpbd->hDlg, WM_COMMAND, IDCANCEL, 0);
                            return 0;
                        }

                        case VK_TAB :
                        {
                            BOOL previous = (GetAsyncKeyState(VK_SHIFT) & ISDOWN) ? 1 : 0;
                            SendMessage(pData->pcpbd->hDlg, WM_NEXTDLGCTL, previous, FALSE);

                            return 0;
                        }
                    }
                }
            }
            else if ( tag & CTRLTAG_ISGRANDCHILD )
            {
                return grandchildEvent(pData, method, hwnd, msg, wParam, lParam, tag);
            }
            break;
        }

        case CTRLTAG_CONTROL :
        {
            if ( msg == WM_KEYDOWN )
            {
                if ( tag & CTRLTAG_ISOLATE )
                {
                    if ( isExtendedKeyEvent(wParam) )
                    {
                        RexxArrayObject args  = getKeyEventRexxArgs(c, wParam, true, pData->pcdc->rexxSelf);
                        return charReply(pData, method, args, hwnd, msg, wParam, lParam);
                    }
                    return DefSubclassProc(hwnd, msg, wParam, lParam);
                }
            }
            else if ( msg == WM_CHAR )
            {
                RexxArrayObject args  = getKeyEventRexxArgs(c, wParam, false, pData->pcdc->rexxSelf);
                return charReply(pData, method, args, hwnd, msg, wParam, lParam);
            }

            break;
        }

        case CTRLTAG_MOUSE :
        {
            return processMouseMsg(c, method, tag, msg, hwnd, wParam, lParam, pData->pcdc);
        }

        case CTRLTAG_DIALOG :
        {
            if ( msg == WM_CONTEXTMENU )
            {
                return DefWindowProc(hwnd, msg, wParam, lParam);
            }
            else if ( msg == WM_SIZE )
            {
                return 0;
            }

            break;
        }

        default :
            break;
    }

    // We dropped through without processing the message, but this message did
    // match an entry in the message table.
    if ( tag & CTRLTAG_REPLYTRUE )
    {
        // Note that I'm not sure it is correct to reply true ??
        return TRUE;
    }
    else if ( tag & CTRLTAG_REPLYZERO )
    {
        return 0;
    }
    else if ( tag & CTRLTAG_SENDTODEFWINDOWPROC )
    {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    else
    {
        // Note that here, if we use invokeDispatch(), we drop through and
        // DefSubclassProc() is invoked.  If we invoke the method directly and
        // wait for the reply, if there is no condition and if the reply is 0,
        // we return 0, which means the message was processed for most messages.
        // Oherwise we again drop through and DefSubclassProc() is invoked.

        RexxObjectPtr   _wP  = c->Uintptr(wParam);
        RexxObjectPtr   _lP  = c->Intptr(lParam);
        RexxArrayObject args = c->ArrayOfThree(_wP, _lP, pData->pcdc->rexxSelf);

        if ( tag & CTRLTAG_REPLYFROMREXX )
        {
            RexxObjectPtr reply = c->SendMessage(pData->pcpbd->rexxSelf, method, args);
            if ( ! checkForCondition(c, false) && reply != NULLOBJECT )
            {
                if ( isInt(0, reply, c) )
                {
                    c->ReleaseLocalReference(_wP);
                    c->ReleaseLocalReference(_lP);
                    c->ReleaseLocalReference(args);
                    c->ReleaseLocalReference(reply);

                    // Swallow the message.
                    return 0;
                }
            }

            if ( reply != NULLOBJECT )
            {
                c->ReleaseLocalReference(reply);
            }
        }
        else
        {
            invokeDispatch(c, pData->pcpbd, method, args);
        }

        c->ReleaseLocalReference(_wP);
        c->ReleaseLocalReference(_lP);
        c->ReleaseLocalReference(args);
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

/**
 * Generic subclass window procedure for all dialog controls.
 *
 * This is based, loosely, on the existing dialog mechanism for connecting
 * window messages to a method in the Rexx dialog.
 *
 * Normally, a multi-line edit control processes the WM_MOUSEWHEEL and scrolls
 * the text appropriately.  This prevents the Rexx programmer from doing any
 * custom handling of the message.  The message never makes it to the dialog
 * procedure.
 *
 * Here, we prevent the edit window from receiving the message by either passing
 * the message on to DefWindowProc() or returning TRUE.
 *
 * When the message is passed on to DefWindowProc(), it gets sent on to the
 * dialog window procedure.  However, if the Rexx programmer specified an event
 * handler, (by specifying the event handler method,) we inovke the Rexx dialog
 * method directly using mouseWheelNotify()
 *
 * Note that mouseWheelNotify() returns true for success and false if a
 * condition was raised in the Rexx event handler.  If a condition was raised,
 * the condition message is printed to the screen, but not, at this time,
 * cleared.  For now we just ignore the return from mouseWheelNotify().
 */
LRESULT CALLBACK ControlSubclassProc(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR dwData)
{
    pSubClassData pData = (pSubClassData)dwData;

    MESSAGETABLEENTRY *m = pData->msgs;
    if ( m == NULL )
    {
        return DefSubclassProc(hwnd, msg, wParam, lParam);
    }

    size_t tableSize = pData->mNextIndex;
    register size_t i = 0;

    for ( i = 0; i < tableSize; i++ )
    {
        if ( (msg    & m[i].msgFilter) == m[i].msg    &&
             (wParam & m[i].wpFilter)  == m[i].wParam &&
             (lParam & m[i].lpFilter)  == m[i].lParam )
        {
            return processControlMsg(hwnd, msg, wParam, lParam, pData, m[i].rexxMethod, m[i].tag);
        }
    }

    if ( msg == WM_NCDESTROY )
    {
        /* The window is being destroyed, remove the subclass, clean up memory.
         * Note that with the current ooDialog architecture, this message
         * *usually* never gets here.  Freeing the subclass data struct has to
         * be done in the dialog control uninit().
         */
        RemoveWindowSubclass(hwnd, ControlSubclassProc, pData->id);
        freeSubclassData(pData);
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}


/**
 * Sets the generic control subclass procedure for the specified control.
 *
 * @param c
 * @param pcdc
 *
 * @return True on success, false on failure.
 *
 * @assumes  The generic control subclass has not be set for this control.
 */
bool setControlSubclass(RexxMethodContext *c, pCDialogControl pcdc)
{
    pSubClassData pSCData = (pSubClassData)LocalAlloc(LPTR, sizeof(SubClassData));
    if ( pSCData == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }

    pSCData->pcpbd          = pcdc->pcpbd;
    pSCData->pcdc           = pcdc;
    pSCData->hCtrl          = pcdc->hCtrl;
    pSCData->id             = pcdc->id;

    BOOL success;
    if ( isDlgThread(pcdc->pcpbd) )
    {
        success = SetWindowSubclass(pcdc->hCtrl, ControlSubclassProc, pcdc->id, (DWORD_PTR)pSCData);
    }
    else
    {
        success = (BOOL)SendMessage(pcdc->pcpbd->hDlg, WM_USER_SUBCLASS, (WPARAM)ControlSubclassProc, (LPARAM)pSCData);
    }

    if ( ! success )
    {
        LocalFree(pSCData);
        systemServiceExceptionCode(c->threadContext, API_FAILED_MSG, "SetWindowSubclass");
        return false;
    }

    pcdc->pscd = pSCData;

    return true;
}

/**
 * Adds an event connection to the dialog control's subclass message table,
 * setting the controls subclass procedure if needed..
 *
 * @param c          Rexx method context we are operating in.
 * @param pcdc       The dialog control's C Self.
 * @param pwmf
 *
 * @return True on success, false on error.  On error an exception has been
 *         raised.
 *
 * @remarks  Caller must ensure that 'method' is not an empty string and that
 *           winMsg, wParam, lParam are not all 0.
 *
 *           See remarks in addCommandMessages() for some relevant information.
 */
bool addSubclassMessage(RexxMethodContext *c, pCDialogControl pcdc, pWinMessageFilter pwmf)
{
    pSubClassData pscd = (pSubClassData)pcdc->pscd;

    if ( pscd == NULL )
    {
        if ( ! setControlSubclass(c, pcdc) )
        {
            return false;
        }

        pscd = (pSubClassData)pcdc->pscd;

        pscd->pData = pwmf->pData;
        pscd->pfn   = pwmf->pfn;
    }

    if ( pscd->msgs == NULL )
    {
        pscd->msgs = (MESSAGETABLEENTRY *)LocalAlloc(LPTR, sizeof(MESSAGETABLEENTRY) * DEF_CONTROL_MSGS);
        if ( pscd->msgs == NULL )
        {
            outOfMemoryException(c->threadContext);
            return false;
        }
        pscd->mNextIndex = 0;
        pscd->mSize      = DEF_CONTROL_MSGS;
    }

    size_t index = pscd->mNextIndex;

    if ( index >= pscd->mSize )
    {
        HLOCAL temp = LocalReAlloc(pscd->msgs, sizeof(MESSAGETABLEENTRY) * pscd->mSize * 2, LMEM_ZEROINIT | LMEM_MOVEABLE);
        if ( temp == NULL )
        {
            MessageBox(0, "Dialog control message connections have exceeded the maximum\n"
                          "number of allocated table entries, and the table could not\n"
                          "be expanded.\n\n"
                          "No more dialog control message connections can be added.\n",
                       "Error", MB_OK | MB_ICONHAND);
            return false;
        }

        pscd->mSize *= 2;
        pscd->msgs = (MESSAGETABLEENTRY *)temp;
    }

    pscd->msgs[index].rexxMethod = (char *)LocalAlloc(LPTR, strlen(pwmf->method) + 1);
    if ( pscd->msgs[index].rexxMethod == NULL )
    {
        outOfMemoryException(c->threadContext);
        return false;
    }
    strcpy(pscd->msgs[index].rexxMethod, pwmf->method);

    pscd->msgs[index].msg       = pwmf->wm;
    pscd->msgs[index].msgFilter = pwmf->wmFilter;
    pscd->msgs[index].wParam    = pwmf->wp;
    pscd->msgs[index].wpFilter  = pwmf->wpFilter;
    pscd->msgs[index].lParam    = pwmf->lp;
    pscd->msgs[index].lpFilter  = pwmf->lpFilter;
    pscd->msgs[index].tag       = pwmf->tag;

    pscd->mNextIndex++;
    return true;
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
 *
 * @remarks  The connect key press functions were introduced, and documented,
 *           well before the generic ControlSubclassProc() was implemented.  The
 *           connect key press implementation is sufficiently different from the
 *           generic control subclass implementation to dictate that it remain
 *           distinct.  It does however use the same SubClassData structure.
 */
LRESULT CALLBACK KeyPressSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
  LPARAM lParam, UINT_PTR id, DWORD_PTR dwData)
{
    pSubClassData pSCData = (pSubClassData)dwData;
    if ( ! pSCData )
    {
        return DefSubclassProc(hwnd, msg, wParam, lParam);
    }

    KEYPRESSDATA *pKeyData = (KEYPRESSDATA *)pSCData->pData;

    switch ( msg )
    {
        case WM_SYSKEYDOWN:
            /* Sent when the alt key is down.  We need both WM_SYSKEYDOWN and
             * WM_KEYDOWN to catch everything that a keyboard hook catches.
             */
            if (  pKeyData->key[wParam] && !(lParam & KEY_RELEASED) && !(lParam & KEY_WASDOWN) )
            {
                processKeyPress(pSCData, wParam, lParam);
            }
            break;

        case WM_KEYDOWN:
            /* WM_KEYDOWN will never have KEY_RELEASED set. */
            if (  pKeyData->key[wParam] && !(lParam & KEY_WASDOWN) )
            {
                processKeyPress(pSCData, wParam, lParam);
            }
            break;

        case WM_NCDESTROY:
            /* The window is being destroyed, remove the subclass, clean up memory.
             * Note that with the current ooDialog architecture, this message never
             * gets here.  Freeing the subclass data struct has to be done in the
             * dialog control uninit().
             */
            RemoveWindowSubclass(hwnd, KeyPressSubclassProc, id);
            freeKeyPressData(pSCData);
            break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

/**
 * Convenience function to remove the key press subclass procedure and free the
 * associated memory.
 *
 * If for some reason remvoing the subclass fails, we can not free the memory
 * because the subclass procedure may (will) still acess it.
 *
 * @remarks  Note that the return from SendMessage is invalid if we are running
 *           in the same thread as the dialog's message processing loop.
 *           Currently, that is unlikely to happen, but it is a possible source
 *           of an error here.
 */
static BOOL removeKeyPressSubclass(pSubClassData pData, HWND hDlg, INT id)
{
    BOOL success = SendMessage(hDlg, WM_USER_SUBCLASS_REMOVE, (WPARAM)&KeyPressSubclassProc, (LPARAM)id) != 0;
    if ( success )
    {
        freeKeyPressData(pData);
    }
    return success;
}


/**
 *
 *
 * @param c
 * @param methodName
 * @param keys
 * @param filter
 * @param pcdc
 *
 * @return keyPressErr_t
 *
 * @remarks  Note that the return from SendMessage is invalid if we are running
 *           in the same thread as the dialog's message processing loop.
 *           Currently, that is unlikely to happen, but it is a possible source
 *           of an error here.
 */
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

    pSubClassData pSCData = (pSubClassData)pcdc->pKeyPress;

    // If pSCData is null, the subclass is not installed.  The data block needs
    // to be allocated and then install the subclass.  Otherwise, just update
    // the data block.
    if ( pSCData == NULL )
    {
        pSCData = (pSubClassData)LocalAlloc(LPTR, sizeof(SubClassData));
        if ( pSCData == NULL )
        {
            result = memoryErr;
            goto done_out;
        }

        KEYPRESSDATA *pKeyPressData = (KEYPRESSDATA *)LocalAlloc(LPTR, sizeof(KEYPRESSDATA));
        if ( pKeyPressData == NULL )
        {
            LocalFree(pSCData);
            result = memoryErr;
            goto done_out;
        }

        pSCData->hCtrl = pcdc->hCtrl;
        pSCData->id    = pcdc->id;
        pSCData->pData = pKeyPressData;
        pSCData->pcdc  = pcdc;
        pSCData->pcpbd = pcdc->pcpbd;

        // The subclass is not installed, abort and clean up if there is any
        // error in setKeyPressData()
        result = setKeyPressData(pKeyPressData, methodName, keys, filter);
        if ( result != noErr )
        {
            freeKeyPressData(pSCData);
            goto done_out;
        }

        BOOL success;

        if ( isDlgThread(pcdc->pcpbd) )
        {
            success = SetWindowSubclass(pSCData->hCtrl, KeyPressSubclassProc, pSCData->id, (DWORD_PTR)pSCData);
        }
        else
        {
            success = (BOOL)SendMessage(pcdc->hDlg, WM_USER_SUBCLASS, (WPARAM)KeyPressSubclassProc, (LPARAM)pSCData);
        }

        if ( success )
        {
            pcdc->pKeyPress = pSCData;
        }
        else
        {
            // Subclassing failed, clean up memory and report the error.
            freeKeyPressData(pSCData);
            result = winAPIErr;
        }
    }
    else
    {
        // The subclass is already installed, it has a valid key press data
        // table. If there are any errors in setKeyPressData(), the error is
        // reported, but the existing data table is left alone.
        result = setKeyPressData((KEYPRESSDATA *)pSCData->pData, methodName, keys, filter);
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

    cdcCSelf->controlType     = params->controlType;
    cdcCSelf->hCtrl           = params->hwnd;
    cdcCSelf->hDlg            = params->pcpbd->hDlg;
    cdcCSelf->id              = params->id;
    cdcCSelf->isInCategoryDlg = params->isCatDlg;
    cdcCSelf->lastItem        = -1;
    cdcCSelf->oDlg            = params->pcpbd->rexxSelf;
    cdcCSelf->pcpbd           = params->pcpbd;
    cdcCSelf->rexxSelf        = self;
    cdcCSelf->wndBase         = wbCSelf;

    context->SetObjectVariable("CSELF", cdcBuf);

    context->SetObjectVariable("ID", c->UnsignedInt32(cdcCSelf->id));
    context->SetObjectVariable("HDLG", pointer2string(context, cdcCSelf->pcpbd->hDlg));
    context->SetObjectVariable("ODLG", cdcCSelf->oDlg);
    result = 0;

done_out:
    return result;
}

/** DialogControl::unInit()
 *
 *  Release the global reference for CWindowBase::rexxHwnd and free any subclass
 *  data structures.
 *
 *  @remarks  Because the current architecture of ooDialog closes the window
 *            message processing loop before it does a DestroyWindow(), no
 *            WM_DESTROY or WM_NCDESTROY messages make it to any dialog or
 *            dialog control windows.  This prevents freeing the subclass data
 *            structures in any subclass window procedure.  Instead the pointer
 *            to the struct is placed in the dialog control CSelf and freed here
 *            in uninit().
 *
 *            However, when the dialog is terminated abruptly due to conditions
 *            being raised, we often do get the WM_NCDESTROY message in the
 *            subclass.  Because of this, we can have the control uninit running
 *            while the subclass data structure(s) are being freed in the
 *            subclass procedure.  We need to prevent this uninit() and the
 *            various subclass data frees in the subclass window procedures from
 *            running at the same time.  Note that the keypress subclassing does
 *            not use
 */
RexxMethod1(RexxObjectPtr, dlgctrl_unInit, CSELF, pCSelf)
{
#if 0
    printf("In dlgctrl_unInit() pCSelf=%p\n", pCSelf);
#endif

    if ( pCSelf != NULLOBJECT )
    {
        pCDialogControl pcdc = (pCDialogControl)pCSelf;

#if 0
    printf("In dlgctrl_unInit() hCtrl=%p pscd=%p pRelayEvent=%p rexxSelf=%p\n",
           pcdc->hCtrl, pcdc->pscd, pcdc->pRelayEvent, pcdc->rexxSelf);
#endif

        if ( pcdc->pKeyPress != NULL )
        {
            freeKeyPressData((pSubClassData)pcdc->pKeyPress);
        }

        EnterCriticalSection(&crit_sec);
        if ( pcdc->pscd != NULL )
        {
            freeSubclassData((pSubClassData)pcdc->pscd);
        }

        if ( pcdc->pRelayEvent != NULL )
        {
            freeRelayData((pSubClassData)pcdc->pRelayEvent);
        }
        LeaveCriticalSection(&crit_sec);

        if ( pcdc->pcrs != NULL )
        {
            safeLocalFree(pcdc->pcrs->method);
            LocalFree(pcdc->pcrs);
            pcdc->pcrs = NULL;
        }

        pCWindowBase pcwb = ((pCDialogControl)pCSelf)->wndBase;
        if ( pcwb->rexxHwnd != TheZeroObj )
        {
            context->ReleaseGlobalReference(pcwb->rexxHwnd);
            pcwb->rexxHwnd = TheZeroObj;
        }
    }
    return NULLOBJECT;
}

RexxMethod1(RexxObjectPtr, dlgctrl_assignFocus, CSELF, pCSelf)
{
    SendMessage(((pCDialogControl)pCSelf)->hDlg, WM_NEXTDLGCTL, (WPARAM)((pCDialogControl)pCSelf)->hCtrl, TRUE);
    return TheZeroObj;
}

/** DialogControl::connectEvent()
 *
 *  Subclasses the dialog control and adds default subclass procedure message
 *  processing for the event specified.
 *
 *  @param  event       [required ] Event keyword.
 *  @param  methodName  [optional]  Method name to use.
 *
 *  @return True for success, false for error
 *
 *  @remarks  Note that we have to use a method name or addSubclassMessage()
 *            will crash.  We use 'NOOP'.  Originally, it was intended for NOOP
 *            to signal some special processing, but that was never followed
 *            through on.
 *
 *  @remarks  For WANTRETURN, we need to also connect the VK_TAB and VK_ESCAPE
 *            keys.  The reason is that we need to use DLGC_WANTALLKEYS for
 *            WM_GETDLGCODE, which prevents the dialog manager from handling TAB
 *            and ESCAPE.  I don't see any way of asking for RETURN but not
 *            ESCAPE and TAB.  In the message processing loop, handle TAB and
 *            ESCAPE ourselves.
 */
RexxMethod3(RexxObjectPtr, dlgctrl_connectEvent, CSTRING, event, OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    RexxObjectPtr result = TheFalseObj;

    WinMessageFilter wmf = {0};
    wmf.method = "NOOP";

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    if ( ! requiredComCtl32Version(context, "connectEvent", COMCTL32_6_0) )
    {
        goto done_out;
    }

    if ( StrCmpI(event, "CONTEXTMENU") == 0 )
    {
        wmf.wm       = WM_CONTEXTMENU;
        wmf.wmFilter = 0xFFFFFFFF;
        wmf.tag      = CTRLTAG_DIALOG;

        if ( addSubclassMessage(context, pcdc, &wmf) )
        {
            result = TheTrueObj;
        }
        goto done_out;
    }
    else if ( StrCmpI(event, "CHAR") == 0 )
    {
        if ( argumentOmitted(2) )
        {
            methodName = "onChar";
        }

        wmf.wm       = WM_KEYDOWN;
        wmf.wmFilter = 0xFFFFFFFF;
        wmf.lp       = KEY_ISEXTENDED;
        wmf.lpFilter = KEY_ISEXTENDED;
        wmf.method   = methodName;
        wmf.tag      = CTRLTAG_CONTROL | CTRLTAG_ISOLATE;

        if ( ! addSubclassMessage(context, pcdc, &wmf) )
        {
            goto done_out;
        }

        wmf.wm       = WM_CHAR;
        wmf.wmFilter = 0xFFFFFFFF;
        wmf.lp       = 0;
        wmf.lpFilter = 0;
        wmf.method   = methodName;
        wmf.tag      = CTRLTAG_CONTROL;

        if ( addSubclassMessage(context, pcdc, &wmf) )
        {
            result = TheTrueObj;
        }
        goto done_out;
    }
    else if ( StrCmpI(event, "WM_SIZE") == 0 )
    {
        wmf.wm       = WM_SIZE;
        wmf.wmFilter = 0xFFFFFFFF;
        wmf.tag      = CTRLTAG_DIALOG | CTRLTAG_REPLYZERO;

        if ( addSubclassMessage(context, pcdc, &wmf) )
        {
            result = TheTrueObj;
        }
        goto done_out;
    }
    else if ( StrCmpI(event, "WANTRETURN") == 0 )
    {
        if ( pcdc->controlType != winEdit || ! isSingleLineEdit(pcdc->hCtrl) )
        {
            result = TheFalseObj;
            goto done_out;
        }

        if ( argumentOmitted(2) )
        {
            methodName = "onReturn";
        }

        wmf.method = methodName;
        wmf.tag    = CTRLTAG_EDIT | CTRLTAG_WANTRETURN;

        wmf.wm       = WM_KEYDOWN;
        wmf.wmFilter = 0xFFFFFFFF;
        wmf.wp       = VK_RETURN;
        wmf.wpFilter = 0xFFFFFFFF;
        wmf.lp       = 0;
        wmf.lpFilter = KEY_WASDOWN;

        if ( ! addSubclassMessage(context, pcdc, &wmf) )
        {
            goto done_out;
        }

        wmf.wp = VK_ESCAPE;
        if ( ! addSubclassMessage(context, pcdc, &wmf) )
        {
            goto done_out;
        }

        wmf.wp = VK_TAB;
        if ( ! addSubclassMessage(context, pcdc, &wmf) )
        {
            goto done_out;
        }

        wmf.wm       = WM_GETDLGCODE;
        wmf.wmFilter = 0xFFFFFFFF;
        wmf.wp       = 0;
        wmf.wpFilter = 0;
        wmf.lp       = 0;
        wmf.lpFilter = 0;

        if ( addSubclassMessage(context, pcdc, &wmf) )
        {
            result = TheTrueObj;
        }
        goto done_out;
    }

done_out:
    return result;
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

/**
 *
 * @remarks  Note that the return from SendMessage is invalid if we are running
 *           in the same thread as the dialog's message processing loop.
 *           Currently, that is unlikely to happen, but it is a possible source
 *           of an error here.
 */
RexxMethod2(int32_t, dlgctrl_disconnectKeyPress, OPTIONAL_CSTRING, methodName, CSELF, pCSelf)
{
    char *tmpName = NULL;
    keyPressErr_t result = winAPIErr;

    if ( ! requiredComCtl32Version(context, context->GetMessageName(), COMCTL32_6_0) )
    {
        goto done_out;
    }

    pCDialogControl pcdc = (pCDialogControl)pCSelf;

    pSubClassData pSCData = NULL;
    GetWindowSubclass(pcdc->hCtrl, KeyPressSubclassProc, pcdc->id, (DWORD_PTR *)&pSCData);

    // If pSCData is null, the subclass has already been removed, (or
    // never existed.)
    if ( pSCData == NULL )
    {
        result = nameErr;
        goto done_out;
    }

    // If no method name, remove the whole thing.
    if ( argumentOmitted(1) )
    {
        result = (removeKeyPressSubclass(pSCData, pcdc->hDlg, pcdc->id) ? noErr : winAPIErr);
        goto done_out;
    }

    // Have a method name, just remove that method from the mapping.
    tmpName = strdupupr(methodName);
    if ( tmpName == NULL )
    {
        result = memoryErr;
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    KEYPRESSDATA *pKeyPressData = (KEYPRESSDATA *)pSCData->pData;

    uint32_t index = seekKeyPressMethod(pKeyPressData, tmpName);
    if ( index == 0 )
    {
        result = nameErr;
        goto done_out;
    }

    // If only 1 method left, remove the subclass entirely.  Otherwise,
    // remove the subclass, fix up the subclass data block, then reinstall
    // the subclass.
    BOOL success = FALSE;
    if ( pKeyPressData->usedMethods == 1 )
    {
        success = removeKeyPressSubclass(pSCData, pcdc->hDlg, pcdc->id);
    }
    else
    {
        if ( SendMessage(pcdc->hDlg, WM_USER_SUBCLASS_REMOVE, (WPARAM)KeyPressSubclassProc, (LPARAM)pcdc->id) )
        {
            removeKeyPressMethod(pKeyPressData, index);
            success = (BOOL)SendMessage(pcdc->hDlg, WM_USER_SUBCLASS, (WPARAM)KeyPressSubclassProc, (LPARAM)pSCData);

            // If not success, then the subclass procedure is no longer
            // installed, (even though it was originally,) and the memory
            // will never be cleaned up, so clean it up now.
            if ( ! success )
            {
                freeKeyPressData(pSCData);
            }
        }
    }
    result = (success ? noErr : winAPIErr);

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

    pSubClassData pData = NULL;
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

    BOOL exists = (seekKeyPressMethod((KEYPRESSDATA *)pData->pData, tmpName) > 0);
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

    if ( ! getRectFromArglist(context, args, (PORXRECT)&r, true, 1, 4, &arraySize, &argsUsed) )
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

    if ( ! getRectFromArglist(context, args, (PORXRECT)&r, true, 1, 5, &arraySize, &argsUsed) )
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

    return redrawRect(context, getDChCtrl(pCSelf), &r, doErase, true);
}


/** DialogControl::setParent()
 *
 *  Sets a new parent for this dialog control.
 *
 *  @param  The new parent
 *
 *  @return  True on success, false on error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, dlgctrl_setParent, RexxObjectPtr, parent, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCDialogControl pcdcParent = requiredDlgControlCSelf(context, parent, 1);
    pCDialogControl pcdc       = validateDCCSelf(context, pCSelf);

    if ( pcdc == NULL || pcdcParent == NULL )
    {
        return TheFalseObj;
    }

    HWND old = SetParent(pcdc->hCtrl, pcdcParent->hCtrl);
    if ( old == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return TheFalseObj;
    }
    return TheTrueObj;
}


/** DialogControl::setWindowTheme()
 *
 *  Causes a window to use a different set of visual style information than its
 *  class normally uses.
 *
 *  @param  name [required]  The application name to use in place of the calling
 *               application's name.
 *
 *  @return  True on success, false on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  The only theme name I know to work is 'explorer.'  Not at all sure
 *            what other names might be valid.
 */
RexxMethod2(RexxObjectPtr, dlgctrl_setWindowTheme, CSTRING, name, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return TheFalseObj;
    }

    size_t len = strlen(name);
    if ( len >= MAX_PATH )
    {
        stringTooLongException(context->threadContext, 1, MAX_PATH - 1, len);
        return TheFalseObj;
    }

    WCHAR themeName[MAX_PATH];

    putUnicodeText((LPWORD)themeName, name);

    HRESULT hr = SetWindowTheme(pcdc->hCtrl, themeName, NULL);
    if ( ! SUCCEEDED(hr) )
    {
        oodSetSysErrCode(context->threadContext, hr);
        return TheFalseObj;
    }

    return TheTrueObj;
}


/** DialogControl::textSize()
 *
 *  Computes the width and height in pixels of the specified string of text when
 *  displayed by this control.
 *
 *  @param text  The text whose size is needed.
 *  @param size  [IN/OUT]  A .Size object, the calculated size is returned here.
 *
 *  @return  True on success, otherwise false.  It is unlikely that this
 *           function would fail.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, dlgctrl_textSize, CSTRING, text, RexxObjectPtr, _size, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    RexxObjectPtr result = TheFalseObj;

    PSIZE size = (PSIZE)rxGetSize(context, _size, 2);
    if ( size == NULL )
    {
        return result;
    }

    HWND hCtrl = getDChCtrl(pCSelf);
    HDC  hdc = GetDC(hCtrl);
    if ( hdc == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return result;
    }

    HFONT hFont = (HFONT)SendMessage(hCtrl, WM_GETFONT, 0, 0);
    if ( hFont == NULL )
    {
        // Font has not been set.
        hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    }

    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    if ( GetTextExtentPoint32(hdc, text, (int)strlen(text), size) != 0 )
    {
        result = TheTrueObj;
    }

    // Clean up.
    SelectObject(hdc, hOldFont);
    ReleaseDC(hCtrl, hdc);

    return result;
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


/** DialogControl::setColor()
 *  DialogControl::setSysColor
 *
 *  @remarks  Both setColor() and setSysColor() are enhanced in 4.2.1 to make
 *            the first argument optional.  If not specified, the background
 *            brush used is the default dialog color, or the brush used for the
 *            dialog background color.  This is the 'transparent' text effect
 *            that Martin Berg has always wanted.
 *
 *            setColor() is also enhanced by adding a third argument, isClrRef,
 *            that allows the user to specify the colors as COLORREFs, not
 *            pallete indexes.
 */
RexxMethod5(int32_t, dlgctrl_setColor, OPTIONAL_RexxObjectPtr, rxBG, OPTIONAL_RexxObjectPtr, rxFG,
            OPTIONAL_logical_t, isClrRef, NAME, method, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }

    bool    useSysColor = (method[3] == 'S');
    uint32_t bkColor = CLR_DEFAULT;
    uint32_t fgColor = CLR_DEFAULT;

    if ( useSysColor )
    {
        if ( argumentExists(1) && ! getSystemColor(context, rxBG, &bkColor, 1) )
        {
            return 1;
        }
        if ( argumentExists(2) && ! getSystemColor(context, rxFG, &fgColor, 2) )
        {
            return 1;
        }
    }
    else
    {
        if ( argumentExists(1) && ! context->UnsignedInt32(rxBG, &bkColor) )
        {
            return 1;
        }
        if ( argumentExists(2) && ! context->UnsignedInt32(rxFG, &fgColor) )
        {
            return 1;
        }
    }

    if ( ! (useSysColor || isClrRef) )
    {
        if ( bkColor != CLR_DEFAULT )
        {
            bkColor = PALETTEINDEX(bkColor);
        }
        if ( fgColor != CLR_DEFAULT )
        {
            fgColor = PALETTEINDEX(fgColor);
        }
    }

    return (int32_t)oodColorTable(context, dlgToCSelf(context, pcdc->oDlg), pcdc->id, bkColor, fgColor, useSysColor);
}

/** DialogControl::data()
 *
 *  Gets the "data" of the dialog control.
 *
 *  @return  The 'data' of the control.
 *
 *  @remarks  The original ooDialog code used the abstraction that there were
 *            only two objects involved.  The ooDialog object and the underlying
 *            Windows dialog.  The dialog controls were considered to be the
 *            'data' of the underlying Windows dialog.  In this abstraction, an
 *            edit control was part of the 'data' of the dialog and its 'data'
 *            was the edit control's text.  For a check box the 'data' is
 *            checked or not, etc..
 *
 */
RexxMethod1(RexxObjectPtr, dlgctrl_data, CSELF, pCSelf)
{
    pCDialogControl pcdc = (pCDialogControl)pCSelf;

    return getControlData(context, dlgToCSelf(context, pcdc->oDlg), pcdc->id, pcdc->hDlg, pcdc->controlType);
}

/** DialogControl::"data="
 *
 *  Sets the "data" of the dialog control.
 *
 *  @param  data  What to set the 'data' of the dialog control to.  Its meaning
 *                and format are dependent on the type of control.
 *
 *  @return  No return for "=" methods.
 *
 *  @remarks  See the remarks in dlgctrl_data above.
 */
RexxMethod2(RexxObjectPtr, dlgctrl_dataEquals, CSTRING, data, CSELF, pCSelf)
{
    pCDialogControl pcdc = (pCDialogControl)pCSelf;

    setControlData(context, dlgToCSelf(context, pcdc->oDlg), pcdc->id, data, pcdc->hDlg, pcdc->controlType);
    return NULLOBJECT;
}

/** DialogControl::addUserSubclass()
 *
 *  Subclasses the dialog control, if needed, and sdds a message to the message
 *  table for the subclass.
 *
 *  This performs a very similar function as does the addUserMessage() method of
 *  the dialog ojbect.
 *
 *  In general, each entry in the subclass message table connects a dialog
 *  control window message to a method in a Rexx dialog.  The fields for the
 *  entry consist of the window message, the WPARAM and LPARAM for the message,
 *  a filter for the message and its parameters, and the method name. Using the
 *  proper filters for the window message and its parameters allows the mapping
 *  of a very specific window message to the named method.
 *
 *  However, unlike the addUserMessage, the addUserSubclass allows a few more
 *  possiblitilies than just invoking a method.  The programmer can also specify
 *  that the window message delievery to the dialog control be surpressed, or
 *  that the window message be sent straight to the dialog message queue
 *  instead.  More possibilities may be added in the future.
 *
 *  @param  methodName   [required]  The method name to be connected.
 *  @param  wm           [required]  The Windows event message
 *  @param  _wmFilter    [optional]  Filter applied to the Windows message.  If
 *                       omitted the filter is 0xFFFFFFFF.
 *  @param  wp           [optional]  WPARAM for the message
 *  @param  _wpFilter    [optional]  Filter applied to the WPARAM.  If omitted a
 *                       filter of all hex Fs is applied
 *  @param  lp           [optional]  LPARAM for the message.
 *  @param  _lpFilter    [optional]  Filter applied to LPARAM.  If omitted the
 *                       filter is all hex Fs.
 *  @param  _tag         [optional]  A tag that allows a further differentiation
 *                       between messages.  Some, but perhaps not all of the
 *                       possible values will be documented publicly.
 *
 *  @return  True on success, false on failure.
 *
 *  @note     Method name can not be the empty string, but it can be 'NOOP' when
 *            the subclass will not invoke a method. The Window message, WPARAM,
 *            and LPARAM arguments can not all be 0.
 *
 *            If incorrect arguments are detected a syntax condition is raised.
 */
RexxMethod9(logical_t, dlgctrl_addUserSubclass, CSTRING, methodName, CSTRING, wm, OPTIONAL_CSTRING, _wmFilter,
            OPTIONAL_RexxObjectPtr, wp, OPTIONAL_CSTRING, _wpFilter, OPTIONAL_RexxObjectPtr, lp,
            OPTIONAL_CSTRING, _lpFilter, OPTIONAL_CSTRING, _tag, CSELF, pCSelf)
{
    WinMessageFilter wmf    = {0};
    logical_t        result = FALSE;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }
    if ( ! requiredComCtl32Version(context, "addUserSubclass", COMCTL32_6_0) )
    {
        goto done_out;
    }

    wmf.method    = methodName;
    wmf._wm       = wm;
    wmf._wmFilter = _wmFilter;
    wmf._wp       = wp;
    wmf._wpFilter = _wpFilter;
    wmf._lp       = lp;
    wmf._lpFilter = _lpFilter;

    if ( ! parseWinMessageFilter(context, &wmf) )
    {
        goto done_out;
    }

    uint32_t tag = CTRLTAG_REPLYFROMREXX;
    if ( argumentExists(8) )
    {
        uint64_t number;
        if ( rxStr2Number(context, _tag, &number, 8) )
        {
            tag = (uint32_t)number;
        }
        else
        {
            // _tag was not in 0xffff format, so we'll check for one of the
            // modifying keywords. But if there are none, then we will raise a
            // syntax error.
            context->ClearCondition();

            if ( ! parseTagOpts(context->threadContext, _tag, &tag, 8) )
            {
                context->ClearCondition();
                wrongArgValueException(context->threadContext, 8, USERSUBCLASS_TAG_KEYWORDS, _tag);
            }
        }
    }
    wmf.tag = tag;

    if ( addSubclassMessage(context, pcdc, &wmf) )
    {
        result = TRUE;
    }

done_out:
    return result;
}


/**
 * Put a Rexx object in this dialog control's 'bag.'  This saves a reference to
 * the object in a context variable of this dialog control.  Which should
 * prevent the object from being garbage collected.
 *
 * Although we call it a 'bag' it is really a set so that only one reference to
 * any single object is added.
 */
RexxMethod2(RexxObjectPtr, dlgctrl_putInBag, RexxObjectPtr, object, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);

    if ( pcdc != NULL )
    {
        if ( pcdc->rexxBag == NULL )
        {
            RexxObjectPtr bag = rxNewBag(context);
            context->SetObjectVariable(DIALOGCONTROL_BAG_ATTRIBUTE, bag);
            pcdc->rexxBag = bag;
        }
        context->SendMessage1(pcdc->rexxBag, "PUT", object);
    }

    return TheNilObj;
}



/**
 *  Generic methods for the dialog control classes.  These are methods that are
 *  very similar in two or more controls, enough similar that it doesn't make
 *  sense to have separate method implmentations.
 */
#define GENERIC_DIALOGCONTROL_METHODS        "Generic Methods"


/** ListView::getToolTips()
 *  ReBar::getToolTips()
 *  TreeView::getToolTips()
 *
 *  Retrieves the child ToolTip control used by this list-view, tree-view,
 *  rebar, or...
 *
 *  @param  None.
 *
 *  @return  Returns the tool tip Rexx object, or .nil if there is no tool tip.
 *
 *  @remarks  We create a Rexx tool tip object from the returned handle and then
 *  protect that object.  Rather than store the tool tip object in the dialog's
 *  bag, we put it in the control's bag. We don't check the return from create
 *  control for TheNilObj because protectControlObect() does that for us.
 */
RexxMethod1(RexxObjectPtr, generic_getToolTips, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    oodControl_t ctrlType = pcdc->controlType;
    HWND         hTT      = NULL;

    if ( ctrlType == winListView )
    {
        hTT = ListView_GetToolTips(pcdc->hCtrl);
    }
    else if ( ctrlType == winTreeView )
    {
        hTT = TreeView_GetToolTips(pcdc->hCtrl);
    }
    else if ( ctrlType == winReBar )
    {
        hTT = (HWND)SendMessage(pcdc->hCtrl, RB_GETTOOLTIPS, 0, 0);
    }

    if ( hTT == NULL )
    {
        goto done_out;
    }

    result = createControlFromHwnd(context, pcdc, hTT, winToolTip, false);
    protectControlObject(context, pcdc, result);

done_out:
    return result;
}


/** ListView::setToolTips()
 *  ReBar::setToolTips()
 *  TreeView::setToolTips()
 *
 *  Sets the child ToolTip control used by this list-view, tree-view, rebar,
 *  or...
 *
 *  @param  None.
 *
 *  @return  Returns the previous tool tip, as a Rexx ToolTip object, or .nil if
 *           there is no previous tool tip.
 */
RexxMethod2(RexxObjectPtr, generic_setToolTips, RexxObjectPtr, toolTip, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNilObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }
    if ( ! requiredClass(context->threadContext, toolTip, "ToolTip", 1) )
    {
        goto done_out;
    }

    oodControl_t ctrlType = pcdc->controlType;
    HWND         hOldTT   = NULL;

    // Rather than put the tool tip object in the dialog bag, we put it in this
    // control's bag, list-view, tree-view, rebar, or...
    pCDialogControl pcdcTT = controlToCSelf(context, toolTip);
    protectControlObject(context, pcdc, toolTip);

    if ( ctrlType == winListView )
    {
        hOldTT = ListView_SetToolTips(pcdc->hCtrl, pcdcTT->hCtrl);
    }
    else if ( ctrlType == winTreeView )
    {
        hOldTT = TreeView_SetToolTips(pcdc->hCtrl, pcdcTT->hCtrl);
    }
    else if ( ctrlType == winReBar )
    {
        hOldTT = (HWND)SendMessage(pcdc->hCtrl, RB_SETTOOLTIPS, (WPARAM)pcdcTT->hCtrl, 0);
    }
    if ( hOldTT == NULL )
    {
        goto done_out;
    }

    // We don't care if .nil is returned because unprotectControlObject() will
    // check for TheNilObj and not try to remove it from the bag.
    result = createControlFromHwnd(context, pcdc, hOldTT, winToolTip, false);
    unProtectControlObject(context, pcdc, result);

done_out:
    return result;
}


