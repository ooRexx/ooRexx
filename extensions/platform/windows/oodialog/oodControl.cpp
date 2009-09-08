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

#include <stdio.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodText.hpp"
#include "oodData.hpp"

typedef struct newControlParams {
   HWND           hwnd;
   uint32_t       id;
   HWND           hwndDlg;
   RexxObjectPtr  parentDlg;
} NEWCONTROLPARAMS;
typedef NEWCONTROLPARAMS *PNEWCONTROLPARAMS;


/**
 *  Methods for the .DialogControl class.
 */
#define DIALOGCONTROL_CLASS        "DialogControl"

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
 *  These attributes are publicly 'get' only attributes and can not be changed.
 *
 *
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

    pCDialogControl cdcCSelf = (pCDialogControl)c->BufferData(cdcBuf);
    memset(cdcCSelf, 0, sizeof(CDialogControl));

    cdcCSelf->wndBase = wbCSelf;
    cdcCSelf->rexxSelf = self;
    cdcCSelf->hCtrl = params->hwnd;
    cdcCSelf->id = params->id;
    cdcCSelf->hDlg = params->hwndDlg;
    cdcCSelf->oDlg = params->parentDlg;

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

    return getTextSize(context, text, fontName, fontSize, hwndSrc, ((pCDialogControl)pCSelf)->oDlg);
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
    else if ( strcmp(className, "RADIOCONTROL"  ) == 0 ) { ctrl = winRadioButton; className = "RADIOBUTTON"; }
    else if ( strcmp(className, "CHECKCONTROL"  ) == 0 ) { ctrl = winCheckBox; className = "CHECKBOX"; }
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
 * @remarks Replaces / combines the individual getXXXControl() and the
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
    if ( ! oodSafeResolveID(&id, c, self, rxID, -1, 1) || (int)id < 0 )
    {
        goto out;
    }

    HWND hControl = GetDlgItem(hDlg, (int)id);
    if ( hControl == NULL )
    {
        goto out;
    }

    // Check that the underlying Windows control is the control type requested
    // by the programmer.  Return .nil if this is not true.
    oodControl_t controlType;
    RexxClassObject controlCls = getControlClass(context, c->GetMessageName() + 3, &controlType);
    if ( controlCls == NULLOBJECT )
    {
        goto out;
    }
    if ( ! checkControlClass(hControl, controlType) )
    {
        goto out;
    }

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
        outOfMemoryException(context->threadContext);
        goto out;
    }

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


RexxMethod2(RexxObjectPtr, advCtrl_test, OSELF, self, CSELF, pDlgCSelf)
{
    RexxMethodContext *c = context;

    void *dlgCSelf = c->ObjectToCSelf(self);
    printf("advCtrl_test() self=%p self.get.CSelf=%p pDlgCSelf=%p\n", self, dlgCSelf, pDlgCSelf);
    dbgPrintClassID(context, self);

    DIALOGADMIN *dlgAdm = rxGetDlgAdm(context, self);
    printf("dlgAdm=%p\n", dlgAdm);
    if ( dlgCSelf != NULL )
    {
        pCWindowBase pcwb = (pCWindowBase)dlgCSelf;
        printf("pcwb hwnd=%p factorX=%f\n", pcwb->hwnd, pcwb->factorX);
    }
    return TheNilObj;
}
