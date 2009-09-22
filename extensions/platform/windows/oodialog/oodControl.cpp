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
#include <commctrl.h>
#include <shlwapi.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodText.hpp"
#include "oodData.hpp"
#include "oodControl.hpp"

typedef struct newControlParams {
   HWND           hwnd;
   uint32_t       id;
   HWND           hwndDlg;
   RexxObjectPtr  parentDlg;
} NEWCONTROLPARAMS;
typedef NEWCONTROLPARAMS *PNEWCONTROLPARAMS;


uint32_t listViewStyle(CSTRING opts, uint32_t style)
{
    if ( StrStrI(opts, "VSCROLL"      ) != NULL ) style |= WS_VSCROLL;
    if ( StrStrI(opts, "HSCROLL"      ) != NULL ) style |= WS_HSCROLL;
    if ( StrStrI(opts, "EDIT"         ) != NULL ) style |= LVS_EDITLABELS;
    if ( StrStrI(opts, "SHOWSELALWAYS") != NULL ) style |= LVS_SHOWSELALWAYS;
    if ( StrStrI(opts, "ALIGNLEFT"    ) != NULL ) style |= LVS_ALIGNLEFT;
    if ( StrStrI(opts, "ALIGNTOP"     ) != NULL ) style |= LVS_ALIGNTOP;
    if ( StrStrI(opts, "AUTOARRANGE"  ) != NULL ) style |= LVS_AUTOARRANGE;
    if ( StrStrI(opts, "ICON"         ) != NULL ) style |= LVS_ICON;
    if ( StrStrI(opts, "SMALLICON"    ) != NULL ) style |= LVS_SMALLICON;
    if ( StrStrI(opts, "LIST"         ) != NULL ) style |= LVS_LIST;
    if ( StrStrI(opts, "REPORT"       ) != NULL ) style |= LVS_REPORT;
    if ( StrStrI(opts, "NOHEADER"     ) != NULL ) style |= LVS_NOCOLUMNHEADER;
    if ( StrStrI(opts, "NOWRAP"       ) != NULL ) style |= LVS_NOLABELWRAP;
    if ( StrStrI(opts, "NOSCROLL"     ) != NULL ) style |= LVS_NOSCROLL;
    if ( StrStrI(opts, "NOSORTHEADER" ) != NULL ) style |= LVS_NOSORTHEADER;
    if ( StrStrI(opts, "SHAREIMAGES"  ) != NULL ) style |= LVS_SHAREIMAGELISTS;
    if ( StrStrI(opts, "SINGLESEL"    ) != NULL ) style |= LVS_SINGLESEL;
    if ( StrStrI(opts, "ASCENDING"    ) != NULL ) style |= LVS_SORTASCENDING;
    if ( StrStrI(opts, "DESCENDING"   ) != NULL ) style |= LVS_SORTDESCENDING;
    return style;
}


TCHAR *controlType2winName(oodControl_t control)
{
    switch ( control )
    {
        case winStatic :               return WC_STATIC;
        case winButton :               return WC_BUTTON;
        case winRadioButton :          return WC_BUTTON;
        case winCheckBox :             return WC_BUTTON;
        case winGroupBox :             return WC_BUTTON;
        case winEdit :                 return WC_EDIT;
        case winListBox :              return WC_LISTBOX;
        case winSingleSelectListBox :  return WC_LISTBOX;
        case winComboBox :             return WC_COMBOBOX;
        case winSimpleComboBox :       return WC_COMBOBOX;
        case winScrollBar :            return WC_SCROLLBAR;
        case winTreeView :             return WC_TREEVIEW;
        case winListView :             return WC_LISTVIEW;
        case winTab :                  return WC_TABCONTROL;
        case winProgressBar :          return PROGRESS_CLASS;
        case winTrackBar :             return TRACKBAR_CLASS;
        case winMonthCalendar :        return MONTHCAL_CLASS;
        case winDateTimePicker :       return DATETIMEPICK_CLASS;
        default :                      return "";
    }
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
bool checkControlClass(HWND hControl, oodControl_t control)
{
    TCHAR buf[64];
    TCHAR *pClass = controlType2winName(control);

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


oodControl_t oodName2controlType(CSTRING name)
{
    if (      strcmp(name, "STATICCONTROL" ) == 0 ) return winStatic;
    else if ( strcmp(name, "BUTTONCONTROL" ) == 0 ) return winButton;
    else if ( strcmp(name, "RADIOCONTROL"  ) == 0 ) return winRadioButton;
    else if ( strcmp(name, "CHECKCONTROL"  ) == 0 ) return winCheckBox;
    else if ( strcmp(name, "GROUPBOX"      ) == 0 ) return winGroupBox;
    else if ( strcmp(name, "EDITCONTROL"   ) == 0 ) return winEdit;
    else if ( strcmp(name, "EDIT"          ) == 0 ) return winEdit;
    else if ( strcmp(name, "LISTBOX"       ) == 0 ) return winListBox;
    else if ( strcmp(name, "COMBOBOX"      ) == 0 ) return winComboBox;
    else if ( strcmp(name, "SCROLLBAR"     ) == 0 ) return winScrollBar;
    else if ( strcmp(name, "PROGRESSBAR"   ) == 0 ) return winProgressBar;
    else if ( strcmp(name, "SLIDERCONTROL" ) == 0 ) return winTrackBar;
    else if ( strcmp(name, "TRACKBAR"      ) == 0 ) return winTrackBar;
    else if ( strcmp(name, "TREECONTROL"   ) == 0 ) return winTreeView;
    else if ( strcmp(name, "TREEVIEW"      ) == 0 ) return winTreeView;
    else if ( strcmp(name, "LISTCONTROL"   ) == 0 ) return winListView;
    else if ( strcmp(name, "LISTVIEW"      ) == 0 ) return winListView;
    else if ( strcmp(name, "TABCONTROL"    ) == 0 ) return winTab;
    else if ( strcmp(name, "TAB"           ) == 0 ) return winTab;
    else if ( strcmp(name, "MONTHCALENDAR" ) == 0 ) return winMonthCalendar;
    else if ( strcmp(name, "DATETIMEPICKER") == 0 ) return winDateTimePicker;
    else return winUnknown;


}

RexxClassObject getControlClass(RexxMethodContext *c, CSTRING className, oodControl_t *controlType)
{
    RexxClassObject controlClass = NULLOBJECT;
    oodControl_t ctrl = oodName2controlType(className);

    if ( ctrl == winUnknown )
    {
        goto done_out;
    }
    else if ( ctrl == winRadioButton )
    {
        className = "RADIOBUTTON";
    }
    else if ( ctrl == winCheckBox )
    {
        className = "CHECKBOX";
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

    SIZE textSize = {0};
    if ( getTextSize(context, text, fontName, fontSize, hwndSrc, ((pCDialogControl)pCSelf)->oDlg, &textSize) )
    {
        return rxNewSize(context, textSize.cx, textSize.cy);
    }
    return NULLOBJECT;
}


/**
 *  Methods for the .AdvancedControls class.
 */
#define ADVANCEDCONTROLS_CLASS        "AdvancedControls"
#define ADVCTRLCONTROLBAG_ATTRIBUTE   "!advCtrlDlgControlBag"


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

    RexxObjectPtr bag = context->GetObjectVariable(ADVCTRLCONTROLBAG_ATTRIBUTE);
    if ( bag == NULLOBJECT )
    {
        bag = rxNewBag(context);
        context->SetObjectVariable(ADVCTRLCONTROLBAG_ATTRIBUTE, bag);
    }
    if ( bag != NULLOBJECT )
    {
        context->SendMessage2(bag, "PUT", control, control);
    }

    return TheNilObj;
}

RexxMethod2(RexxObjectPtr, advCtrl_test, OSELF, self, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    void *dlgCSelf = c->ObjectToCSelf(self);
    pCPlainBaseDialog realDlgCSelf = dlgToCSelf(c, self);

    dbgPrintClassID(context, self);
    printf("advCtrl_test() self=%p pCSelf=%p ObjectToCSelf(self)=%p ObjectToCSelf(self, self~class)=%p\n",
           self, pCSelf, dlgCSelf, realDlgCSelf);

    DIALOGADMIN *dlgAdm = rxGetDlgAdm(context, self);
    printf("dlgAdm=%p\n", dlgAdm);
    if ( realDlgCSelf != NULL )
    {
        pCWindowBase pcwb = ((pCPlainBaseDialog)realDlgCSelf)->wndBase;
        printf("pcpbd->dlgAdm=%p pcwb hwnd=%p factorX=%f\n", realDlgCSelf->dlgAdm, pcwb->hwnd, pcwb->factorX);
    }
    return TheNilObj;
}
