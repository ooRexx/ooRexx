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
 * oodControls.cpp
 *
 * Contains the base classes used for objects representing Windows Controls.
 */
#include "ooDialog.h"     // Must be first, includes windows.h and oorexxapi.h

//#include <stdio.h>
#include "APICommon.h"
#include "oodCommon.h"


/**
 *  Methods for the .DialogControl class.
 */
#define DIALOGCONTROL_CLASS        "DialogControl"


/** DialogControl::getTextSizeDlg()
 *
 *  Gets the size (width and height) in dialog units for any given string for
 *  the font specified.
 *
 *  @param  text         The string whose size is needed.
 *
 *  @param  fontName     Optional. If specified, use this font to calculate the
 *                       size.
 *
 *  @param  fontSize     Optional. If specified, use this font size with
 *                       fontName to calculate the size.  The default if omitted
 *                       is 8.  This arg is ignored if fontName is omitted.
 *
 *  @param  hwndFontSrc  Optional. Use this window's font to calculate the size.
 *                       This arg is always ignored if fontName is specified.
 *
 */
RexxMethod5(RexxObjectPtr, dlgctrl_getTextSizeDlg, CSTRING, text, OPTIONAL_CSTRING, fontName,
            OPTIONAL_uint32_t, fontSize, OPTIONAL_POINTERSTRING, hwndFontSrc, OSELF, self)
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
        if ( hwndFontSrc == NULL )
        {
            nullObjectException(context, "window handle", 4);
            goto error_out;
        }
        hwndSrc = (HWND)hwndFontSrc;
    }

    RexxObjectPtr dlgObj = context->SendMessage0(self, "ODLG");
    if ( dlgObj == NULLOBJECT )
    {
        // The interpreter kernel will have raised a syntax exception in this
        // case.  But, the ooDialog framework traps the exception and puts up a
        // message box saying ODLG is not a method of xx control.  I think that
        // will be confusing to the users, since they have no idea about this
        // call to oDlg. So, raise a more specific exception.
        context->RaiseException1(Rexx_Error_Interpretation_user_defined,
                                 context->String("Inconsistency: this .DialogControl object does not have "
                                                 "the oDlg (owner dialog) attribute"));
        goto error_out;
    }

    return getTextSize(context, text, fontName, fontSize, hwndSrc, dlgObj);

error_out:
    return NULLOBJECT;
}


/**
 *  Methods for the .AdvancedControls class.
 */
#define ADVANCEDCONTROLS_CLASS        "AdvancedControls"
#define ADVCTRLCONTROLBAG_ATTRIBUTE   "!advCtrlDlgControlBag"


/**
 * Given a CategoryDialog, retrieves the dialog handle corresponding to the
 * given page ID.
 *
 * Note that it is expected that this function will be called from native
 * methods where the pageID argument is possibly omitted in the Rexx method.
 * The original implementation of ooDialog had a convention, if the category
 * number is 0 it is meant to be a control in the main dialog.  If it is omitted
 * and the dialog is a category dialog, it is meant to be a control in the
 * current category page.  Because of this, we need to know if the pageID
 * argument was omitted in order to evalute things correctly.
 *
 * @param c            Method context we are operating under.
 *
 * @param categoryDlg  The CategoryDialog object.
 *
 * @param pageID       [in / out]  The page ID of the category dialog.  This
 *                     argument is unchanged on return if a lookup of the active
 *                     page is not done.  When the active page is looked, the
 *                     looked up page ID is returned here.
 *
 * @param hDlg         [in / out]  The handle of the page dialog is returned
 *                     here, on success.  Its value on entry is ignored.
 *
 * @return  True if no error, otherwise false.
 *
 * @assumes the caller has already checked that categoryDlg is in fact a
 *          category dialog object.
 *
 * @remarks  The 'CATALOG' attribute is set to a directory, the 'handles', and
 *           'category' entries are added in the init() method of a
 *           CategoryDialog. They must be there or ooRexx is broken. So, no
 *           check for NULLOBJECT is done for them.  Note that the indexes of
 *           the CATALOG directory are all lower case.
 */
bool getCategoryHDlg(RexxMethodContext *c, RexxObjectPtr categoryDlg, uint32_t *pageID, HWND *hDlg, bool idArgExists)
{
    bool result = false;

    if ( idArgExists && *pageID == 0 )
    {
        *hDlg = rxGetWindowHandle(c, categoryDlg);
        if ( *hDlg != NULL )
        {
            result = true;
        }
        goto done_out;
    }

    RexxDirectoryObject catalog = (RexxDirectoryObject)c->SendMessage0(categoryDlg, "CATALOG");
    RexxArrayObject handles = (RexxArrayObject)c->DirectoryAt(catalog, "handles");

    if ( *pageID == 0 )
    {
        // Look up the active page number.
        RexxObjectPtr rxPageID = c->DirectoryAt(catalog, "category");

        if ( ! c->UnsignedInt32(rxPageID, pageID) )
        {
            // TODO an exception is probably needed.
            goto done_out;
        }
    }
    RexxObjectPtr rxHwnd = c->ArrayAt(handles, *pageID);
    if ( rxHwnd != NULLOBJECT )
    {
        *hDlg = (HWND)string2pointer(c->ObjectToStringValue(rxHwnd));
        result = true;
    }

done_out:
    return result;
}

RexxClassObject getControlClass(RexxMethodContext *c, CSTRING className, oodControl_t *controlType)
{
    oodControl_t ctrl = winUnknown;
    RexxClassObject controlClass = NULLOBJECT;

    if ( strcmp(className, "STATICCONTROL" ) == 0 ) { ctrl = winStatic; }
    else if ( strcmp(className, "BUTTONCONTROL" ) == 0 ) { ctrl = winButton; }
    else if ( strcmp(className, "TREECONTROL"   ) == 0 ) { ctrl = winTreeView; }
    else if ( strcmp(className, "LISTCONTROL"   ) == 0 ) { ctrl = winListView; }
    else if ( strcmp(className, "TABCONTROL"    ) == 0 ) { ctrl = winTab; }
    else if ( strcmp(className, "EDITCONTROL"   ) == 0 ) { ctrl = winEdit; }
    else if ( strcmp(className, "RADIOBUTTON"   ) == 0 ) { ctrl = winRadioButton; }
    else if ( strcmp(className, "CHECKBOX"      ) == 0 ) { ctrl = winCheckBox; }
    else if ( strcmp(className, "GROUPBOX"      ) == 0 ) { ctrl = winGroupBox; }
    else if ( strcmp(className, "LISTBOX"       ) == 0 ) { ctrl = winListBox; }
    else if ( strcmp(className, "COMBOBOX"      ) == 0 ) { ctrl = winComboBox; }
    else if ( strcmp(className, "SCROLLBAR"     ) == 0 ) { ctrl = winScrollBar; }
    else if ( strcmp(className, "PROGRESSBAR"   ) == 0 ) { ctrl = winProgressBar; }
    else if ( strcmp(className, "SLIDERCONTROL" ) == 0 ) { ctrl = winTrackBar; }
    else if ( strcmp(className, "MONTHCALENDAR" ) == 0 ) { ctrl = winMonthCalendar; }
    else if ( strcmp(className, "DATETIMEPICKER") == 0 ) { ctrl = winDateTimePicker; }

    if ( ctrl == winUnknown )
    {
        goto done_out;
    }

    controlClass = rxGetContextClass(c, className);
    if ( controlClass == NULLOBJECT )
    {
        // An exception has been raised, which we don't want.  So, clear it.
        c->ClearCondition();
    }

done_out:
    *controlType = ctrl;
    return controlClass;
}

/** AdvancedControls::getXXXControl()
 *
 *  Instantiates a DialogControl object for the specified Windows control.  All
 *  of the AdvanceControls get methods filter through this function.
 *  getEditControl(), getButtonControl(), getListControl(), etc..
 *
 * @param  rxID  The resource ID of the control to get.
 *
 * @param  categoryPageID  [optional] If the dialog is a category dialog, this
 *                         indicates which page of the dialog the control is on.
 *
 * @returns  The properly instantiated dialog control object on success, or the
 *           nil object on failure.
 *
 * @remarks Replaces / combines the individualy getXXXControl() and the
 *          getControl() methods of the AdvancedControl class.  Either returns
 *          the control object asked for, or .nil.
 *
 *          The first time a Rexx object is instantiated for a specific Windows
 *          control, the Rexx object is stored in the window words of the
 *          control.  Before a Rexx object is instantiated, the window words are
 *          checked to see if there is already an instantiated object and if so
 *          returns that object.
 */
RexxMethod3(RexxObjectPtr, advCtrl_getControl, RexxObjectPtr, rxID, OPTIONAL_uint32_t, categoryPageID, OSELF, self)
{
    RexxMethodContext *c = context;
    RexxObjectPtr result = TheNilObj;

    bool isCategoryDlg = false;
    HWND hDlg = NULL;

    if ( c->IsOfType(self, "CATEGORYDIALOG") )
    {
        if ( ! getCategoryHDlg(context, self, &categoryPageID, &hDlg, argumentExists(2)) )
        {
            goto out;
        }
        isCategoryDlg = true;
    }
    else
    {
        hDlg = rxGetWindowHandle(c, self);
        if ( hDlg == NULL )
        {
            goto out;
        }
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, c, self, rxID, -1, 1) )
    {
        goto out;
    }

    HWND hControl = NULL;
    RexxObjectPtr rxControl = NULLOBJECT;

    hControl = GetDlgItem(hDlg, (int)id);
    if ( hControl != NULL )
    {
        rxControl = (RexxObjectPtr)getWindowPtr(hControl, GWLP_USERDATA);
        if ( rxControl != NULLOBJECT )
        {
            // Okay, this specific control has already had a control object
            // instantiated to represent it.  We return this object.
            result = rxControl;
            goto out;
        }
    }

    // No pointer is stored in the user data area, so no control object has been
    // instantiated for this specific control, yet.  We instantiate one now and
    // then store the object in the user data area of the control window.
    oodControl_t controlType;
    RexxClassObject controlCls = getControlClass(context, c->GetMessageName() + 3, &controlType);
    if ( controlCls == NULLOBJECT )
    {
        goto out;
    }

    // Check that the underlying Windows control is the control type requested
    // by the programmer.  Return .nil if this is not true.
    if ( ! checkControlClass(hControl, controlType) )
    {
        goto out;
    }

    // TODO Much of the information we just determined, is re-determined in the
    // new method of the dialog control.  It would be nice to change the new
    // method to take a .Pointer object and rewrite the new method as a native
    // API method and pass this information directly.
    RexxArrayObject args;
    if ( isCategoryDlg )
    {
         args = c->ArrayOfThree(self, c->UnsignedInt32(id), c->UnsignedInt32(categoryPageID));
    }
    else
    {
        args = c->ArrayOfTwo(self, c->UnsignedInt32(id));
    }

    rxControl = c->SendMessage(controlCls, "NEW", args);
    if ( rxControl != NULLOBJECT && rxControl != TheNilObj )
    {
        // In the old Rexx implementing code, there was the possibility here
        // that we would have a control object that would not be valid and have
        // its hwnd set to 0.  That is no longer possible.
        result = rxControl;

        setWindowPtr(hControl, GWLP_USERDATA, (LONG_PTR)result);
        c->SendMessage1(self, "putControl", result);
    }

out:
    return result;
}

RexxMethod2(RexxObjectPtr, advCtrl_putControl_pvt, RexxObjectPtr, control, OSELF, self)
{
    // This should never fail, do we need an exception if it does?

    RexxObjectPtr bag = context->GetObjectVariable(ADVCTRLCONTROLBAG_ATTRIBUTE);
    if ( bag == NULLOBJECT )
    {
        RexxObjectPtr theBagClass = context->FindClass("BAG");
        if ( theBagClass != NULLOBJECT )
        {
            bag = context->SendMessage0(theBagClass, "NEW");
            context->SetObjectVariable(ADVCTRLCONTROLBAG_ATTRIBUTE, bag);
        }
    }

    if ( bag != NULLOBJECT )
    {
        context->SendMessage2(bag, "PUT", control, control);
    }

    return TheNilObj;
}
