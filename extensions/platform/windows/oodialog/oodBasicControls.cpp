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
 * oodBasicControls.cpp
 *
 * Contains the classes used for objects representing the original, basic,
 * Windows Controls.  These are: static, button, edit, list box, and combo box
 * controls.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <shlwapi.h>
#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodMessaging.hpp"
#include "oodMouse.hpp"
#include "oodResources.hpp"
#include "oodShared.hpp"
#include "oodDeviceGraphics.hpp"


/**
 * The genric implementation for the isGrandChild() method.  Currently only the
 * edit and combo box controls have this method, so being static in this module
 * is fine.
 *
 * This may need to change if the mehtod is used for more controls .
 *
 * @param context
 * @param mthName
 * @param wantTab
 * @param pCSelf
 *
 * @return RexxObjectPtr
 */
static RexxObjectPtr genericIsGrandchild(RexxMethodContext *context, CSTRING mthName, logical_t wantTab,
                                         CSELF pCSelf, oodControl_t type)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr    result = TheFalseObj;
    WinMessageFilter wmf    = {0};

    // For now there is only edit and combo box, may be others in the future.
    if ( type == winComboBox )
    {
        wmf.method = "onComboBoxGrandChildEvent";
        wmf.tag    = CTRLTAG_COMBOBOX | CTRLTAG_ISGRANDCHILD;
    }
    else
    {
        wmf.method = "onEditGrandChildEvent";
        wmf.tag    = CTRLTAG_EDIT | CTRLTAG_ISGRANDCHILD;
    }

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }
    if ( ! requiredComCtl32Version(context, "isGrandChild", COMCTL32_6_0) )
    {
        goto done_out;
    }
    if ( type == winEdit && ! isSingleLineEdit(pcdc->hCtrl) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_SUPPORTED);
        goto done_out;
    }

    if ( argumentExists(1) )
    {
        wmf.method = mthName;
    }

    wmf.wm       = WM_KILLFOCUS;
    wmf.wmFilter = 0xFFFFFFFF;
    wmf.wp       = 0;
    wmf.wpFilter = 0;
    wmf.lp       = 0;
    wmf.lpFilter = 0;

    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm       = WM_GETDLGCODE;
    wmf.wmFilter = 0xFFFFFFFF;
    wmf.wp       = 0;
    wmf.wpFilter = 0;
    wmf.lp       = 0;
    wmf.lpFilter = 0;

    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm       = WM_KEYDOWN;
    wmf.wmFilter = 0xFFFFFFFF;
    wmf.wp       = VK_RETURN;
    wmf.wpFilter = 0xFFFFFFFF;
    wmf.lp       = 0;
    wmf.lpFilter = KEY_WASDOWN;

    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm = WM_KEYUP;
    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm = WM_CHAR;
    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm       = WM_KEYDOWN;
    wmf.wmFilter = 0xFFFFFFFF;
    wmf.wp       = VK_ESCAPE;
    wmf.wpFilter = 0xFFFFFFFF;
    wmf.lp       = 0;
    wmf.lpFilter = KEY_WASDOWN;

    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm = WM_KEYUP;
    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm = WM_CHAR;
    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm       = WM_KEYDOWN;
    wmf.wmFilter = 0xFFFFFFFF;
    wmf.wp       = VK_TAB;
    wmf.wpFilter = 0xFFFFFFFF;
    wmf.lp       = 0;
    wmf.lpFilter = KEY_WASDOWN;

    if ( wantTab )
    {
        wmf.tag |= CTRLTAG_WANTTAB;
    }

    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm = WM_KEYUP;
    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm = WM_CHAR;
    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    result = TheTrueObj;

done_out:
    return result;
}


LRESULT grandchildEvent(pSubClassData pData, char *method, HWND hwnd, uint32_t msg,
                        WPARAM wParam, LPARAM lParam, uint32_t tag)
{
    RexxThreadContext *c = pData->pcpbd->dlgProcContext;

    if ( msg == WM_GETDLGCODE )
    {
        return (DLGC_WANTALLKEYS | DefSubclassProc(hwnd, msg, wParam, lParam));
    }
    else if ( msg == WM_KEYDOWN || msg == WM_KILLFOCUS )
    {
        CSTRING keyWord = "error";
        LRESULT ret     = 0;

        if ( msg == WM_KILLFOCUS )
        {
            keyWord = "killfocus";
            ret = DefSubclassProc(hwnd, msg, wParam, lParam);
        }
        else
        {
            switch ( wParam )
            {
                case VK_RETURN :
                    keyWord = "enter";
                    break;

                case VK_ESCAPE :
                    keyWord = "escape";
                    break;

                case VK_TAB :
                {
                    if ( tag & CTRLTAG_WANTTAB )
                    {
                        keyWord = "tab";
                        break;
                    }

                    BOOL previous = (GetAsyncKeyState(VK_SHIFT) & ISDOWN) ? 1 : 0;
                    SendMessage(pData->pcpbd->hDlg, WM_NEXTDLGCTL, previous, FALSE);

                    return 0;
                }
            }
        }

        RexxObjectPtr   ctrlID = c->UnsignedInt32(pData->id);
        RexxObjectPtr   flag   = c->String(keyWord);
        RexxArrayObject args   = c->ArrayOfThree(ctrlID, flag, pData->pcdc->rexxSelf);
        RexxObjectPtr   reply  = c->SendMessage(pData->pcpbd->rexxSelf, method, args);

        if ( ! checkForCondition(c, false) && reply != NULLOBJECT )
        {
            c->ReleaseLocalReference(ctrlID);
            c->ReleaseLocalReference(flag);
            c->ReleaseLocalReference(args);
            c->ReleaseLocalReference(reply);

            return ret;
        }
        else
        {
            // On error return DefSubclassProc()
            return (ret == 0 ? DefSubclassProc(hwnd, msg, wParam, lParam) : ret);
        }
    }
    else if ( msg == WM_KEYUP || msg == WM_CHAR )
    {
        return 0;
    }

    // Quiet compiler warnings, we can not really get here.
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}


/**
 *  Methods for the .StaticControl.
 */
#define STATIC_CLASS              "StaticControl"
#define STATICIMAGE_ATTRIBUTE     "!STATICIMAGE"

/** StaticControl::setIcon()
 *
 *  Sets or removes the icon image for this static control.
 *
 *  @param  icon  The new icon image for the the static control, or .nil to
 *                remove the existing icon.
 *
 *  @return  The existing icon, or .nil if there is no existing icon.
 */
RexxMethod2(RexxObjectPtr, stc_setIcon, RexxObjectPtr, icon, CSELF, pCSelf)
{
    RexxObjectPtr result = NULLOBJECT;

    HANDLE hNewIcon = NULL;
    if ( icon != TheNilObj )
    {
        POODIMAGE oi = rxGetImageIcon(context, icon, 1);
        if ( oi == NULL )
        {
            goto out;
        }
        hNewIcon = oi->hImage;
    }

    HWND hwnd = ((pCDialogControl)pCSelf)->hCtrl;
    HICON hIcon = (HICON)SendMessage(hwnd, STM_SETICON, (WPARAM)hNewIcon, 0);

    result = oodSetImageAttribute(context, STATICIMAGE_ATTRIBUTE, icon, hwnd, hIcon, IMAGE_ICON, winStatic);
out:
    return result;
}

/** StaticControl::getIcon()
 *
 *  Gets the icon image for this static control.
 *
 *  @return  The icon image, or .nil if there is no icon image.
 */
RexxMethod1(RexxObjectPtr, stc_getIcon, OSELF, self)
{
    return oodGetImageAttribute(context, self, STATICIMAGE_ATTRIBUTE, STM_GETICON, 0, IMAGE_ICON, winStatic);
}

/** StaticControl::setImage()
 *
 *  Sets or removes the image for this static control.
 *
 *  @param  rxNewImage  The new image for the the control, or .nil to remove the
 *                      existing image.
 *
 * @return  The old image, if there is one, otherwise .nil.
 */
RexxMethod2(RexxObjectPtr, stc_setImage, RexxObjectPtr, rxNewImage, CSELF, pCSelf)
{
    RexxObjectPtr result = NULLOBJECT;

    long type = 0;
    HANDLE hImage = NULL;

    if ( rxNewImage != TheNilObj )
    {
        POODIMAGE oi = rxGetOodImage(context, rxNewImage, 1);
        if ( oi == NULL )
        {
            goto out;
        }
        type = oi->type;
        hImage = oi->hImage;
    }

    HWND hwnd = getDChCtrl(pCSelf);
    HANDLE oldHandle = (HANDLE)SendMessage(hwnd, STM_SETIMAGE, (WPARAM)type, (LPARAM)hImage);

    result = oodSetImageAttribute(context, STATICIMAGE_ATTRIBUTE, rxNewImage, hwnd, oldHandle, -1, winStatic);
out:
    return result;
}

/** StaticControl::getImage()
 *
 *  Gets the image for this static control, if any.
 *
 *  @param  type  [optional]  Signals the type of image, one of the image type
 *                IDs.  The default is IMAGE_BITMAP.
 *
 * @return  The existing image, if there is one, otherwise .nil
 */
RexxMethod2(RexxObjectPtr, stc_getImage, OPTIONAL_uint8_t, type, OSELF, self)
{
    if ( argumentOmitted(1) )
    {
        type = IMAGE_BITMAP;
    }
    if ( type > IMAGE_ENHMETAFILE )
    {
        return wrongArgValueException(context->threadContext, 1, IMAGE_TYPE_LIST, getImageTypeName(type));
    }
    return oodGetImageAttribute(context, self, STATICIMAGE_ATTRIBUTE, STM_GETIMAGE, type, -1, winStatic);
}

/**
 *  Methods for the ooDialog class: .Button and its subclasses
 *  .RadioButton and .CheckBox.
 */
#define BUTTON_CLASS                 "Button"

#define BUTTONIMAGELIST_ATTRIBUTE    "!BUTTONIMAGELIST"
#define BUTTONIMAGE_ATTRIBUTE        "!BUTTONIMAGE"

#define BUTTONCONTROLCLASS   ".Button"
#define RADIOBUTTONCLASS     ".RadioButton"
#define CHECKBOXCLASS        ".CheckBox"
#define GROUPBOXCLASS        ".GroupBox"
#define ANIMATEDBUTTONCLASS  ".AnimatedButton"

#define BC_SETSTYLE_OPTS     "PUSHBOX, DEFPUSHBUTTON, CHECKBOX, AUTOCHECKBOX, 3STATE, AUTO3STATE, "        \
                             "RADIO, AUTORADIO, GROUPBOX, OWNERDRAW, LEFTTEXT, RIGHTBUTTON, NOTLEFTTEXT, " \
                             "TEXT, ICON, BITMAP, LEFT, RIGHT, HCENTER, TOP, BOTTOM, VCENTER, PUSHLIKE, "  \
                             "NOTPUSHLIKE, MULTILINE, NOTMULTILINE, NOTIFY, NOTNOTIFY, FLAT, NOTFLAT"

#define BC_SETSTATE_OPTS      "CHECKED, UNCHECKED, INDETERMINATE, FOCUS, PUSH, NOTPUSHED"
#define BUTTON_ALIGNMENT_LIST "LEFT, RIGHT, TOP, BOTTOM, or CENTER"
#define MIN_HALFHEIGHT_GB    12


/**
 * Changes the default push button in a dialog to that of the dialog control
 * specified.
 *
 * @param hCtrl  The push button that is to become the default push button.
 *
 * @return True on success, otherwise false.
 *
 * @assumes hCtrl is a push button control in a dialog.
 */
static HWND changeDefPushButton(HWND hCtrl)
{
    HWND hDlg = GetParent(hCtrl);
    int  id = GetDlgCtrlID(hCtrl);
    HWND hOldDef = NULL;

    if ( hDlg != NULL )
    {
        LRESULT result = SendMessage(hDlg, DM_GETDEFID, 0, 0);

        if ( HIWORD(result) == DC_HASDEFID )
        {
            if ( LOWORD(result) == id )
            {
                /* This control already is the default push button, just return.
                 */
                return hOldDef;
            }

            /* The DM_SETDEFID message does not remove the default push button
             * highlighting, we have to do that ourselves.
             */
            hOldDef = (HWND)GetDlgItem(hDlg, LOWORD(result));
        }

        SendMessage(hDlg, DM_SETDEFID, (WPARAM)id, 0);

        if ( hOldDef )
        {
            SendMessage(hOldDef, BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, (LPARAM)TRUE);
        }
    }
    return hOldDef;
}

/**
 * Gets the button image list alignment value from the specified argument
 * object, where the object could be the numeric value, a string keyword, or
 * omitted altogether.
 *
 * @param context
 * @param _type
 * @param defType   The value to use if the argument is omitted.
 * @param argPos
 *
 * @return The alignment value or OOD_NO_VALUE on error.  An exception has been
 *         raised on error.
 */
static uint32_t getButtonILAlignArg(RexxMethodContext *context, RexxObjectPtr _align, uint32_t defAlign, size_t argPos)
{
    uint32_t align = defAlign;

    if ( argumentExists(argPos) )
    {
        if ( ! context->UnsignedInt32(_align, &align) )
        {
            CSTRING al = context->ObjectToStringValue(_align);
            if (      StrCmpI("LEFT", al)   == 0 ) align = BUTTON_IMAGELIST_ALIGN_LEFT;
            else if ( StrCmpI("RIGH", al)   == 0 ) align = BUTTON_IMAGELIST_ALIGN_RIGHT;
            else if ( StrCmpI("TOP", al)    == 0 ) align = BUTTON_IMAGELIST_ALIGN_TOP;
            else if ( StrCmpI("BOTTOM", al) == 0 ) align = BUTTON_IMAGELIST_ALIGN_BOTTOM;
            else if ( StrCmpI("CENTER", al) == 0 ) align = BUTTON_IMAGELIST_ALIGN_CENTER;
            else
            {
                wrongArgValueException(context->threadContext, argPos, BUTTON_ALIGNMENT_LIST, _align);
                align = OOD_NO_VALUE;
            }
        }

        if ( align != OOD_NO_VALUE && align > BUTTON_IMAGELIST_ALIGN_CENTER )
        {
            wrongRangeException(context->threadContext, argPos,
                                BUTTON_IMAGELIST_ALIGN_LEFT, BUTTON_IMAGELIST_ALIGN_CENTER, align);
            align = OOD_NO_VALUE;
        }
    }
    return align;
}

static RexxStringObject align2keywords(RexxMethodContext *c, uint32_t align)
{
    CSTRING key = "Unknown";

    if ( align == BUTTON_IMAGELIST_ALIGN_LEFT   ) key = "Left";
    if ( align == BUTTON_IMAGELIST_ALIGN_RIGHT  ) key = "Right";
    if ( align == BUTTON_IMAGELIST_ALIGN_TOP    ) key = "Top";
    if ( align == BUTTON_IMAGELIST_ALIGN_BOTTOM ) key = "Bottom";
    if ( align == BUTTON_IMAGELIST_ALIGN_CENTER ) key = "Center";

    return c->String(key);
}

/**
 * Gets the button image list information for this button, which includes the
 * image list itself, the image alignment, and the margin around the image.
 *
 * @param c     The method context we are executing in.
 * @param hwnd  The Button window handle.
 *
 * @return  A directory object containing the image list information, if there
 *          is an image list.  Otherwise .nil.
 *
 * @note    Button image lists can not be set from a resource file, so if there
 *          is an image list for this button, it had to be set from code.
 *          Meaning, if there is not an image list in the object variable, then
 *          this button does not have an image list.
 */
static RexxObjectPtr bcGetImageList(RexxMethodContext *c, HWND hwnd)
{
    RexxObjectPtr result = TheNilObj;

    RexxObjectPtr imageList = c->GetObjectVariable(BUTTONIMAGELIST_ATTRIBUTE);
    if ( imageList != NULLOBJECT && imageList != TheNilObj )
    {
        BUTTON_IMAGELIST biml;

        Button_GetImageList(hwnd, &biml);
        RexxDirectoryObject table = c->NewDirectory();
        if ( table != NULLOBJECT )
        {
            c->DirectoryPut(table, imageList, "IMAGELIST");

            RexxObjectPtr rect = rxNewRect(c, biml.margin.left, biml.margin.top,
                                           biml.margin.right, biml.margin.bottom);
            if ( rect != NULL )
            {
                c->DirectoryPut(table, rect, "RECT");
            }

            RexxObjectPtr alignment = c->WholeNumber(biml.uAlign);
            if ( alignment != NULLOBJECT )
            {
                c->DirectoryPut(table, alignment, "ALIGNMENT");
                c->DirectoryPut(table, align2keywords(c, biml.uAlign), "ALIGNMENTKEYWORD");
            }
            result = table;
        }
    }
    return result;
}

static RexxObjectPtr bcRemoveImageList(RexxMethodContext *c, HWND hwnd)
{
    RexxObjectPtr result = bcGetImageList(c, hwnd);

    if ( result != TheNilObj )
    {
        BUTTON_IMAGELIST biml = {0};
        biml.himl = ImageList_Create(32, 32, ILC_COLOR8, 2, 0);

        Button_SetImageList(hwnd, &biml);
        ImageList_Destroy(biml.himl);
        RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
    }
    return result;
}

static CSTRING getIsChecked(HWND hwnd)
{
    char * state = "UNKNOWN";
    BUTTONTYPE type = getButtonInfo(hwnd, NULL, NULL);

    if ( type == check || type == radio )
    {
        switch ( SendMessage(hwnd, BM_GETCHECK, 0, 0) )
        {
            case BST_CHECKED :
                state = "CHECKED";
                break;
            case BST_UNCHECKED :
                state = "UNCHECKED";
                break;
            case BST_INDETERMINATE :
                state = getButtonInfo(hwnd, NULL, NULL) == check ? "INDETERMINATE" : "UNKNOWN";
                break;
            default :
                break;
        }
    }
    return state;

}


BUTTONTYPE getButtonInfo(HWND hwnd, PBUTTONSUBTYPE sub, DWORD *style)
{
    TCHAR buf[64];
    BUTTONTYPE type = notButton;

    if ( ! RealGetWindowClass(hwnd, buf, sizeof(buf)) || strcmp(buf, WC_BUTTON) )
    {
        if ( sub != NULL )
        {
            *sub = noSubtype;
        }
        if ( style != NULL )
        {
            *style = 0;
        }
        return type;
    }

    LONG _style = GetWindowLong(hwnd, GWL_STYLE);
    BUTTONSUBTYPE _sub;

    switch ( _style & BS_TYPEMASK )
    {
        case BS_PUSHBUTTON :
        case BS_PUSHBOX :
            type = push;
            _sub = noSubtype;
            break;

        case BS_DEFPUSHBUTTON :
            type = push;
            _sub = def;
            break;

        case BS_CHECKBOX :
            type = check;
            _sub = noSubtype;
            break;

        case BS_AUTOCHECKBOX :
            type = check;
            _sub = autoCheck;
            break;

        case BS_3STATE :
            type = check;
            _sub = threeState;
            break;

        case BS_AUTO3STATE :
            type = check;
            _sub = autoThreeState;
            break;

        case BS_RADIOBUTTON :
            type = radio;
            _sub = noSubtype;
            break;

        case BS_AUTORADIOBUTTON :
            type = radio;
            _sub = autoCheck;
            break;

        case BS_GROUPBOX :
            type = group;
            _sub = noSubtype;
            break;

        case BS_USERBUTTON :
        case BS_OWNERDRAW :
            type = owner;
            _sub = noSubtype;
            break;

        default :
            // Can not happen.
            type = notButton;
            _sub = noSubtype;
            break;
     }

    if ( style != NULL )
    {
        *style = _style & ~BS_TYPEMASK;
    }
    if ( sub != NULL )
    {
        *sub = _sub;
    }
    return type;
}


/** GroupBox::style=()
 *
 * A group box is a button, but the only style change that makes much sense is
 * the right, center or left alignment of the text.  Other changes either have
 * no effect, or cause the group box / dialog to paint in a weird way.
 */
RexxMethod2(int, gb_setStyle, CSTRING, opts, CSELF, pCSelf)
{
    HWND hwnd = ((pCDialogControl)pCSelf)->hCtrl;
    HWND hDlg = ((pCDialogControl)pCSelf)->hDlg;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);

    if ( stricmp(opts, "RIGHT") == 0 )
    {
        style = (style & ~(BS_CENTER | BS_LEFT)) | BS_RIGHT;
    }
    else if ( stricmp(opts, "LEFT") == 0 )
    {
        style = (style & ~(BS_CENTER | BS_RIGHT)) | BS_LEFT;
    }
    else if ( stricmp(opts, "CENTER") == 0 )
    {
        style = (style & ~(BS_RIGHT | BS_LEFT)) | BS_CENTER;
    }
    else
    {
        wrongArgValueException(context->threadContext, 1, "RIGHT, CENTER, or LEFT, ", opts);
        return 0;
    }

    /**
     * When the alignment changes, we need to force the dialog to redraw the
     * area occupied by the group box.  Otherwise the old text remains on the
     * screen. Likewise, when we force the dialog to redraw the background, we
     * need to force the group box to redraw.  But, it is only the top part of
     * the group box that needs to be redrawn, so we only invalidate the top
     * half of the group box.
     *
     * The sequence below works.  There may be a better sequence, but the ones
     * I've tried, don't work.
     */

    // Change the group box style, then force the repainting.
    SetWindowLong(hwnd, GWL_STYLE, style);
    SendMessage(hwnd, BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);

    RECT r;
    LONG halfHeight;

    // Get the screen area of the group box and map it to the client area of the
    // dialog.  This forces the dialog to repaint the background, which erases
    // the group box text.
    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    halfHeight = ((r.bottom - r.top) / 2);
    r.bottom = (halfHeight >= MIN_HALFHEIGHT_GB ? r.top + halfHeight : r.bottom);

    InvalidateRect(hDlg, &r, TRUE);
    UpdateWindow(hDlg);

    // Now force the group box to repaint the text.
    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hwnd, (LPPOINT)&r, 2);

    halfHeight = ((r.bottom - r.top) / 2);
    r.bottom = (halfHeight >= MIN_HALFHEIGHT_GB ? r.top + halfHeight : r.bottom);

    InvalidateRect(hwnd, &r, TRUE);
    UpdateWindow(hwnd);

    return 0;
}

RexxMethod2(RexxObjectPtr, bc_setState, CSTRING, opts, CSELF, pCSelf)
{
    HWND hwnd = ((pCDialogControl)pCSelf)->hCtrl;
    HWND hDlg = ((pCDialogControl)pCSelf)->hDlg;

    BUTTONTYPE type = getButtonInfo(hwnd, NULL, NULL);
    UINT msg = 0;
    WPARAM wp = 0;

    char *token;
    char *str = strdupupr(opts);
    if ( ! str )
    {
        outOfMemoryException(context->threadContext);
        return NULLOBJECT;
    }

    token = strtok(str, " ");
    while ( token != NULL )
    {
        if ( strcmp(token, "CHECKED") == 0 )
        {
            if ( (type == check || type == radio) )
            {
                msg = BM_SETCHECK;
                wp = (WPARAM)BST_CHECKED;
            }
        }
        else if ( strcmp(token, "UNCHECKED") == 0 )
        {
            if ( (type == check || type == radio) )
            {
                msg = BM_SETCHECK;
                wp = (WPARAM)BST_UNCHECKED;
            }
        }
        else if ( strcmp(token, "INDETERMINATE") == 0 )
        {
            if ( type == check )
            {
                msg = BM_SETCHECK;
                wp = (WPARAM)BST_INDETERMINATE;
            }
        }
        else if ( strcmp(token, "FOCUS") == 0 )
        {
            msg = 0;
            SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hwnd, TRUE);
        }
        else if ( strcmp(token, "PUSHED") == 0 )
        {
            msg = BM_SETSTATE;
            wp = (WPARAM)TRUE;
        }
        else if ( strcmp(token, "NOTPUSHED") == 0 )
        {
            msg = BM_SETSTATE;
            wp = (WPARAM)FALSE;
        }
        else
        {
            wrongArgValueException(context->threadContext, 1, BC_SETSTATE_OPTS, token);
            free(str);
            return NULLOBJECT;
        }

        if ( msg != 0 )
        {
            SendMessage(hwnd, msg, wp, 0);
            msg = 0;
        }
        token = strtok(NULL, " ");
    }

    safeFree(str);
    return NULLOBJECT;
}

RexxMethod1(RexxStringObject, bc_getState, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);
    BUTTONTYPE type = getButtonInfo(hwnd, NULL, NULL);

    TCHAR buf[64] = {'\0'};
    LRESULT l;

    if ( type == radio || type == check )
    {
        l = SendMessage(hwnd, BM_GETCHECK, 0, 0);
        if ( l == BST_CHECKED )
        {
            strcpy(buf, "CHECKED ");
        }
        else if ( l == BST_INDETERMINATE )
        {
            strcpy(buf,  "INDETERMINATE ");
        }
        else
        {
            strcpy(buf, "UNCHECKED ");
        }
    }

    l = SendMessage(hwnd, BM_GETSTATE, 0, 0);
    if ( l & BST_FOCUS )
        strcat(buf, "FOCUS ");
    {
    }
    if ( l & BST_PUSHED )
    {
        strcat(buf, "PUSHED");
    }

    return context->String(buf);
}


RexxMethod2(RexxObjectPtr, bc_setStyle, CSTRING, opts, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    BUTTONSUBTYPE sub;
    DWORD style, oldStyle;
    BUTTONTYPE type;
    DWORD typeStyle, oldTypeStyle;
    bool changeDefButton = false;

    if ( strlen(opts) == 0 )
    {
        // No change.
        return NULLOBJECT;
    }

    type = getButtonInfo(hwnd, &sub, &style);
    oldStyle = style;
    oldTypeStyle = ((DWORD)GetWindowLong(hwnd, GWL_STYLE) & BS_TYPEMASK);
    typeStyle = oldTypeStyle;

    char *token;
    char *str = strdupupr(opts);
    if ( ! str )
    {
        outOfMemoryException(context->threadContext);
        return NULLOBJECT;
    }

    token = strtok(str, " ");
    while ( token != NULL )
    {
        if ( strcmp(token, "PUSHBOX") == 0 )
        {
            if ( type == push )
            {
                typeStyle = BS_PUSHBOX;
            }
        }
        else if ( strcmp(token, "DEFPUSHBUTTON") == 0 )
        {
            if ( type == push  && sub != def )
            {
                typeStyle = BS_DEFPUSHBUTTON;
                changeDefButton = true;
            }
        }
        else if ( strcmp(token, "CHECKBOX") == 0 )
        {
            if ( type == check )
            {
                typeStyle = BS_CHECKBOX;
            }
        }
        else if ( strcmp(token, "AUTOCHECKBOX") == 0 )
        {
            if ( type == check )
            {
                typeStyle = BS_AUTOCHECKBOX;
            }
        }
        else if ( strcmp(token, "3STATE") == 0 )
        {
            if ( type == check )
            {
                typeStyle = BS_3STATE;
            }
        }
        else if ( strcmp(token, "AUTO3STATE") == 0 )
        {
            if ( type == check )
            {
                typeStyle = BS_AUTO3STATE;
            }
        }
        else if ( strcmp(token, "RADIO") == 0 )
        {
            if ( type == radio )
            {
                typeStyle = BS_RADIOBUTTON;
            }
        }
        else if ( strcmp(token, "AUTORADIO") == 0 )
        {
            if ( type == radio )
            {
                typeStyle = BS_AUTORADIOBUTTON;
            }
        }
        else if ( strcmp(token, "GROUPBOX") == 0 || strcmp(token, "OWNERDRAW") == 0 )
        {
            ; // Ignored.
        }
        else if ( strcmp(token, "LEFTTEXT") == 0 || strcmp(token, "RIGHTBUTTON") == 0 )
        {
            style |= BS_LEFTTEXT;
        }
        else if ( strcmp(token, "NOTLEFTTEXT") == 0 )
        {
            style &= ~BS_LEFTTEXT;
        }
        else if ( strcmp(token, "TEXT") == 0 )
        {
            style &= ~(BS_ICON | BS_BITMAP);
        }
        else if ( strcmp(token, "ICON") == 0 )
        {
            style = (style & ~BS_BITMAP) | BS_ICON;
        }
        else if ( strcmp(token, "BITMAP") == 0 )
        {
            style = (style & ~BS_ICON) | BS_BITMAP;
        }
        else if ( strcmp(token, "LEFT") == 0 )
        {
            style = (style & ~BS_CENTER) | BS_LEFT;
        }
        else if ( strcmp(token, "RIGHT") == 0 )
        {
            style = (style & ~BS_CENTER) | BS_RIGHT;
        }
        else if ( strcmp(token, "HCENTER") == 0 )
        {
            style |= BS_CENTER;
        }
        else if ( strcmp(token, "TOP") == 0 )
        {
            style = (style & ~BS_VCENTER) | BS_TOP;
        }
        else if ( strcmp(token, "BOTTOM") == 0 )
        {
            style = (style & ~BS_VCENTER) | BS_BOTTOM;
        }
        else if ( strcmp(token, "VCENTER") == 0 )
        {
            style |= BS_VCENTER;
        }
        else if ( strcmp(token, "PUSHLIKE") == 0 )
        {
            if ( type == check || type == radio )
            {
                style |= BS_PUSHLIKE;
            }
        }
        else if ( strcmp(token, "MULTILINE") == 0 )
        {
            style |= BS_MULTILINE;
        }
        else if ( strcmp(token, "NOTIFY") == 0 )
        {
            style |= BS_NOTIFY;
        }
        else if ( strcmp(token, "FLAT") == 0 )
        {
            style |= BS_FLAT;
        }
        else if ( strcmp(token, "NOTPUSHLIKE") == 0 )
        {
            if ( type == check || type == radio )
            {
                style &= ~BS_PUSHLIKE;
            }
        }
        else if ( strcmp(token, "NOTMULTILINE") == 0 )
        {
            style &= ~BS_MULTILINE;
        }
        else if ( strcmp(token, "NOTNOTIFY") == 0 )
        {
            style &= ~BS_NOTIFY;
        }
        else if ( strcmp(token, "NOTFLAT") == 0 )
        {
            style &= ~BS_FLAT;
        }
        else
        {
            wrongArgValueException(context->threadContext, 1, BC_SETSTYLE_OPTS, token);
            free(str);
            return NULLOBJECT;
        }

        token = strtok(NULL, " ");
    }

    style |= typeStyle;

    HWND oldDefButton = NULL;
    if ( changeDefButton )
    {
        oldDefButton = changeDefPushButton(hwnd);
    }

    if ( style != (oldStyle | oldTypeStyle) )
    {
        SetWindowLong(hwnd, GWL_STYLE, style);
        SendMessage(hwnd, BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);

        RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);

        if ( oldDefButton )
        {
            RedrawWindow(oldDefButton, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
        }
    }

    safeFree(str);
    return NULLOBJECT;
}

RexxMethod1(RexxObjectPtr, bc_getTextMargin, CSELF, pCSelf)
{
    if ( ! requiredComCtl32Version(context, "getTextMargin", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }

    HWND hwnd = getDChCtrl(pCSelf);
    RexxObjectPtr result = NULLOBJECT;

    ORXRECT r;
    if ( Button_GetTextMargin(hwnd, &r) )
    {
        result = rxNewRect(context, &r);
    }
    return (result == NULL) ? TheNilObj : result;
}

RexxMethod2(logical_t, bc_setTextMargin, RexxObjectPtr, r, CSELF, pCSelf)
{
    if ( ! requiredComCtl32Version(context, "setTextMargin", COMCTL32_6_0) )
    {
        return 0;
    }

    HWND hwnd = getDChCtrl(pCSelf);

    PORXRECT pRect = rxGetRect(context, r, 1);
    if ( pRect != NULL )
    {
        if ( Button_SetTextMargin(hwnd, pRect) )
        {
            return 1;
        }
    }
    return 0;
}

/** Button::getIdealSize()
 *
 *  Gets the size of the button that best fits the text and image, if an image
 *  list is present.
 *
 *  @param  wantWidth  [optional]  Specifies the desired width of the button.
 *                     If this argument is used, the operating system will
 *                     calculate the idea height for a button of the width
 *                     specified.  This functionality is not available on
 *                     Windows XP
 *
 *  @return  On success returns a .Size object containing the operating
 *           systems's calculation of the ideal size.  On error returns the .nil
 *           object.
 */
RexxMethod2(RexxObjectPtr, bc_getIdealSize, OPTIONAL_uint32_t, wantWidth, CSELF, pCSelf)
{
    if ( ! requiredComCtl32Version(context, "getIdealSize", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }

    HWND hwnd = getDChCtrl(pCSelf);
    RexxObjectPtr result = NULLOBJECT;

    SIZE size = { 0, 0 };

    if ( argumentExists(1) )
    {
        size.cx;
    }
    if ( Button_GetIdealSize(hwnd, &size) )
    {
        result = rxNewSize(context, size.cx, size.cy);
    }
    return (result == NULLOBJECT) ? TheNilObj : result;
}

RexxMethod2(RexxObjectPtr, bc_getImage, OPTIONAL_uint8_t, type, OSELF, self)
{
    if ( argumentOmitted(1) )
    {
        type = IMAGE_BITMAP;
    }
    if ( type > IMAGE_CURSOR )
    {
        return wrongArgValueException(context->threadContext, 1, "Bitmap, Icon, Cursor", getImageTypeName(type));
    }
    WPARAM wParam = (type == IMAGE_BITMAP) ? IMAGE_BITMAP : IMAGE_ICON;

    return oodGetImageAttribute(context, self, BUTTONIMAGE_ATTRIBUTE, BM_GETIMAGE, wParam, type, winPushButton);
}

/** Button::setImage()
 *
 *  Sets or removes the image for a button control.
 *
 *  @param  rxNewImage  The new image for the button, or .nil to remove the
 *                      existing image.
 *
 *  @return  The existing image, if there is one, or .nil if there is not.
 *
 *  @note  Only bitmap, icon, or cursor images are valid.
 */
RexxMethod2(RexxObjectPtr, bc_setImage, RexxObjectPtr, rxNewImage, CSELF, pCSelf)
{
    RexxObjectPtr result = NULLOBJECT;

    long type = IMAGE_BITMAP;
    HANDLE hImage = NULL;

    if ( rxNewImage != TheNilObj )
    {
        POODIMAGE oi = rxGetOodImage(context, rxNewImage, 1);
        if ( oi == NULL )
        {
            goto out;
        }

        if ( oi->type > IMAGE_CURSOR )
        {
            wrongArgValueException(context->threadContext, 1, "Bitmap, Icon, Cursor", oi->typeName);
            goto out;
        }
        hImage = oi->hImage;
        type = oi->type == IMAGE_BITMAP ? IMAGE_BITMAP : IMAGE_ICON;
    }

    HWND hwnd = getDChCtrl(pCSelf);
    HANDLE oldHandle = (HANDLE)SendMessage(hwnd, BM_SETIMAGE, (WPARAM)type, (LPARAM)hImage);

    result = oodSetImageAttribute(context, BUTTONIMAGE_ATTRIBUTE, rxNewImage, hwnd, oldHandle, -1, winPushButton);

out:
    return result;
}

/** Button::getImageList()
 *
 * Gets the image list for the button, if there is one.
 *
 * @return  .nil if this the button control does not have an image list.
 *          Otherwise, a .Directory object with the following indexes.  The
 *          indexes contain the image list related information:
 *
 *          d~imageList -> The .ImageList object set by setImageList().
 *          d~rect      -> A .Rect object containing the margins.
 *          d~rect      -> A .Rect object containing the margins.
 *          d~alignment -> The image alignment in the button, numeric.
 *          d~alignmentKeyword -> The image alignment in the button, string.
 *
 * @requires  Common Controls version 6.0 or later.
 *
 * @exception  A syntax error is raised for wrong comctl version.
 *
 * @note  The only way to have an image list is for it have been put there by
 *        setImageList().  That method stores the .ImageList object as an
 *        attribute of the Button ojbect.  That stored object is the
 *        object returned.
 */
RexxMethod1(RexxObjectPtr, bc_getImageList, CSELF, pCSelf)
{
    if ( ! requiredComCtl32Version(context, "getImageList", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }
    return bcGetImageList(context, getDChCtrl(pCSelf));

}

/** Button::setImageList()
 *
 * Sets or removes an image list for the button.
 *
 * @param   imageList  [required]  The new image list for the button, or .nil to
 *                     remove the current image list.
 *
 * @param   margin     [optional]  A .Rect object containing the margins around
 *                     the image.  The default is no margin on either side.
 *
 * @param   align      [optional]  One of the BUTTON_IMAGELIST_ALIGN_xxx
 *                     constant values.  The default is center.
 *
 * @return  The old image list information, if there was an existing image list.
 *          .nil is returned on error and if there was not an existing image
 *          list..
 *
 * @requires  Common Controls version 6.0 or later.
 *
 * @exception  Syntax errors are raised for incorrect arguments and wrong comctl
 *             version.
 *
 * @remarks This method sets the ooDialog System error code (.SystemErrorCode)
 *          if the args seem valid but one of the Windows APIs fails.
 *
 * @see bcGetImageList() for the format of the returned image list information.
 */
RexxMethod4(RexxObjectPtr, bc_setImageList, RexxObjectPtr, imageList, OPTIONAL_RexxObjectPtr, margin,
            OPTIONAL_RexxObjectPtr, align, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    RexxObjectPtr result = NULLOBJECT;

    BUTTON_IMAGELIST biml = {0};
    HWND hwnd = getDChCtrl(pCSelf);

    if ( ! requiredComCtl32Version(context, "setImageList", COMCTL32_6_0) )
    {
        goto err_out;
    }

    if ( imageList == TheNilObj )
    {
        // This is a request to remove the image list.
        result = bcRemoveImageList(context, hwnd);
        goto good_out;
    }

    biml.himl = rxGetImageList(context, imageList, 1);
    if ( biml.himl == NULL )
    {
        goto err_out;
    }

    // Default would be a 0 margin
    if ( argumentExists(2) )
    {
        PORXRECT pRect = rxGetRect(context, margin, 2);
        if ( pRect == NULL )
        {
            goto err_out;
        }
        biml.margin.top = pRect->top;
        biml.margin.left = pRect->left;
        biml.margin.right = pRect->right;
        biml.margin.bottom = pRect->bottom;
    }

    biml.uAlign = getButtonILAlignArg(context, align, BUTTON_IMAGELIST_ALIGN_CENTER, 3);
    if ( biml.uAlign == OOD_NO_VALUE )
    {
        goto err_out;
    }

    result = bcGetImageList(context, hwnd);

    if ( Button_SetImageList(hwnd, &biml) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        goto err_out;
    }
    else
    {
        RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
        context->SetObjectVariable(BUTTONIMAGELIST_ATTRIBUTE, imageList);
    }

good_out:
    return result;

err_out:
    return NULLOBJECT;
}

/** RadioButton::checkInGroup() [class method]
 *
 *  Checks the button specified in a group of buttons and unchecks all the rest.
 *
 *  @param dlg      [required]  Dialog that contains the radio buttons
 *  @param idFirst  [required]  Fist button in group.
 *  @param idLast   [required]  Second button in group.
 *  @param idCheck  [optional]  If omitted all radio buttons are unchecked.  If
 *                              0 or -1, all radio buttons are unchecked.
 *                              Otherwise, this is the button checked.
 *
 *  @return  O on success, the system error code on error.
 *
 *  @notes   Sets the .SystemErrorCode.  Experimentation has shown, that with
 *           many things that might be considered errors, the OS does not set
 *           LastError().
 *
 */
RexxMethod4(uint32_t, rb_checkInGroup_cls, RexxObjectPtr, dlg, RexxObjectPtr, idFirst,
            RexxObjectPtr, idLast, OPTIONAL_RexxObjectPtr, idCheck)
{
    oodResetSysErrCode(context->threadContext);

    uint32_t result = 0;
    if ( requiredClass(context->threadContext, dlg, "PlainBaseDialog", 1) )
    {
        HWND hwnd = dlgToHDlg(context, dlg);

        if ( argumentOmitted(4) )
        {
            idCheck = TheZeroObj;
        }

        int32_t first = oodResolveSymbolicID(context->threadContext, dlg, idFirst, -1, 2, true);
        int32_t last = oodResolveSymbolicID(context->threadContext, dlg, idLast, -1, 3, true);
        int32_t check = oodResolveSymbolicID(context->threadContext, dlg, idCheck, -1, 4, false);

        if ( first != OOD_ID_EXCEPTION && last != OOD_ID_EXCEPTION  && check != OOD_ID_EXCEPTION )
        {
            if ( CheckRadioButton(hwnd, first, last, check) == 0 )
            {
                uint32_t result = GetLastError();
                oodSetSysErrCode(context->threadContext, result);
            }
        }
    }
    return result;
}

RexxMethod1(logical_t, rb_checked, CSELF, pCSelf)
{
    return (SendMessage(getDChCtrl(pCSelf), BM_GETCHECK, 0, 0) == BST_CHECKED ? 1 : 0);
}

RexxMethod1(CSTRING, rb_getCheckState, CSELF, pCSelf)
{
    return getIsChecked(getDChCtrl(pCSelf));
}
RexxMethod1(int, rb_check, CSELF, pCSelf)
{
    SendMessage(getDChCtrl(pCSelf), BM_SETCHECK, BST_CHECKED, 0);
    return 0;
}

RexxMethod1(int, rb_uncheck, CSELF, pCSelf)
{
    SendMessage(getDChCtrl(pCSelf), BM_SETCHECK, BST_UNCHECKED, 0);
    return 0;
}

/* DEPRECATED */
RexxMethod1(CSTRING, rb_isChecked, CSELF, pCSelf)
{
    return getIsChecked(getDChCtrl(pCSelf));
}

/* DEPRECATED */
RexxMethod1(int, rb_indeterminate, CSELF, pCSelf)
{
    SendMessage(getDChCtrl(pCSelf), BM_SETCHECK, BST_INDETERMINATE, 0);
    return 0;
}

RexxMethod1(logical_t, ckbx_isIndeterminate, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);
    if ( getButtonInfo(hwnd, NULL, NULL) == check  )
    {
        return (SendMessage(hwnd, BM_GETCHECK, 0, 0) == BST_INDETERMINATE ? 1 : 0);
    }
    return 0;
}

RexxMethod1(int, ckbx_setIndeterminate, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);
    if ( getButtonInfo(hwnd, NULL, NULL) == check  )
    {
        SendMessage(hwnd, BM_SETCHECK, BST_INDETERMINATE, 0);
    }
    return 0;
}


/** Button::test()
 *
 *  This method is used as a convenient way to test code.
 */
RexxMethod1(int, bc_test, RexxStemObject, stm)
{
    RexxMethodContext *c = context;

    RexxStemObject s = (RexxStemObject)c->GetStemArrayElement(stm, 3);
    if ( s == NULLOBJECT )
    {
        printf("stm.3 is null\n");

        RexxObjectPtr _t = c->GetStemElement(stm, "3.TEXT");
        if ( _t == NULLOBJECT )
        {
            printf("Get stm ->3.text didn't work\n");
        }
        else
        {
            printf("Get stm ->3.text text=%s\n", c->ObjectToStringValue(_t));
        }
    }
    else
    {
        RexxObjectPtr _t = c->GetStemElement(s, "TEXT");
        if ( _t == NULLOBJECT )
        {
            printf("Get stm.3 ->text didn't work\n");
        }
        else
        {
            printf("Get stm.3.text text=%s\n", c->ObjectToStringValue(_t));
        }
    }
    return 0;
}

/** Button::test()              [class method]
 *
 *  This method is used as a convenient way to test code.
 */
RexxMethod1(int, bc_test_cls, RexxObjectPtr, obj)
{
    return 0;
}


/**
 * Methods for the Edit class.
 */
#define EDIT_CLASS   "Edit"

#define BALLON_MAX_TITLE      99
#define BALLON_MAX_TEXT     1023
#define QUE_MAX_TEXT         255



/**
 * Generic function to get the cue text.  Used by both the combobox and the edit
 * controls.
 *
 * @author Administrator (2/5/2013)
 *
 * @param c
 * @param isEdit
 * @param pCSelf
 *
 * @return RexxObjectPtr
 */
static RexxStringObject cbEditGetCue(RexxMethodContext *c, bool isEdit, void *pCSelf)
{
    RexxStringObject result = c->NullString();

    pCDialogControl pcdc = validateDCCSelf(c, pCSelf);
    if ( pcdc == NULL )
    {
        return result;
    }

    if ( ! requiredComCtl32Version(c, "getCue", COMCTL32_6_0)  )
    {
        return result;
    }

    if ( ! isEdit && ! requiredOS(c, "setCue", "Vista", Vista_OS) )
    {
        return result;
    }

    WCHAR wszCue[QUE_MAX_TEXT + 1];

    if ( isEdit )
    {
        if ( Edit_GetCueBannerText(pcdc->hCtrl, wszCue, QUE_MAX_TEXT) )
        {
            result = unicode2string(c, wszCue);
        }
    }
    else
    {
        if ( ComboBox_GetCueBannerText(pcdc->hCtrl, wszCue, QUE_MAX_TEXT) )
        {
            result = unicode2string(c, wszCue);
        }
    }

    return result;
}


/**
 * Generic function to set the cue text.  Used for both a combo box and an edit
 * control.
 *
 * @param c
 * @param text
 * @param show
 * @param isEdit
 * @param pCSelf
 *
 * @return RexxObjectPtr
 */
static RexxObjectPtr cbEditSetCue(RexxMethodContext *c, CSTRING text, logical_t show, bool isEdit, void *pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(c, pCSelf);
    if ( pcdc == NULL )
    {
        return TheOneObj;
    }

    if ( ! requiredComCtl32Version(c, "setCue", COMCTL32_6_0)  )
    {
        return TheOneObj;
    }

    if ( ! isEdit && ! requiredOS(c, "setCue", "Vista", Vista_OS) )
    {
        return TheOneObj;
    }

    // The text is limited to 255.
    if ( strlen(text) > QUE_MAX_TEXT )
    {
        stringTooLongException(c->threadContext, 1, QUE_MAX_TEXT, strlen(text));
        return TheOneObj;
    }

    WCHAR wszCue[QUE_MAX_TEXT + 1];
    putUnicodeText((LPWORD)wszCue, text);

    if ( isEdit )
    {
        return Edit_SetCueBannerTextFocused(pcdc->hCtrl, wszCue, show) ? TheZeroObj : TheOneObj;
    }
    else
    {
        return ComboBox_SetCueBannerText(pcdc->hCtrl, wszCue) ? TheZeroObj : TheOneObj;
    }
}


/**
 * Take an edit control's window flags and construct a Rexx string that
 * represents the control's style.
 */
RexxObjectPtr editStyleToString(RexxMethodContext *c, uint32_t style)
{
    char buf[512];

    if ( style & WS_VISIBLE ) strcpy(buf, "VISIBLE");
    else strcpy(buf, "HIDDEN");

    if ( style & WS_TABSTOP ) strcat(buf, " TAB");
    else strcat(buf, " NOTAB");

    if ( style & WS_DISABLED ) strcat(buf, " DISABLED");
    else strcat(buf, " ENABLED");

    if ( style & WS_GROUP )       strcat(buf, " GROUP");
    if ( style & WS_HSCROLL )     strcat(buf, " HSCROLL");
    if ( style & WS_VSCROLL )     strcat(buf, " VSCROLL");
    if ( style & ES_PASSWORD )    strcat(buf, " PASSWORD");
    if ( style & ES_MULTILINE )   strcat(buf, " MULTILINE");
    if ( style & ES_AUTOHSCROLL ) strcat(buf, " AUTOSCROLLH");
    if ( style & ES_AUTOVSCROLL ) strcat(buf, " AUTOSCROLLV");
    if ( style & ES_READONLY )    strcat(buf, " READONLY");
    if ( style & ES_WANTRETURN )  strcat(buf, " WANTRETURN");
    if ( style & ES_NOHIDESEL )   strcat(buf, " KEEPSELECTION");
    if ( style & ES_UPPERCASE )   strcat(buf, " UPPER");
    if ( style & ES_LOWERCASE )   strcat(buf, " LOWER");
    if ( style & ES_NUMBER )      strcat(buf, " NUMBER");
    if ( style & ES_OEMCONVERT )  strcat(buf, " OEM");

    if ( style & ES_RIGHT ) strcat(buf, " RIGHT");
    else if ( style & ES_CENTER ) strcat(buf, " CENTER");
    else strcat(buf, " LEFT");

    return c->String(buf);
}

/**
 * Parse an edit control style string sent from ooDialog into the corresponding
 * style flags.
 *
 * Note that this is meant to only deal with the styles that can be changed
 * after the control is created through SetWindowLong.
 */
uint32_t parseEditStyle(CSTRING keyWords)
{
    uint32_t style = 0;

    if ( StrStrI(keyWords, "UPPER"     ) ) style |= ES_UPPERCASE;
    if ( StrStrI(keyWords, "LOWER"     ) ) style |= ES_LOWERCASE;
    if ( StrStrI(keyWords, "NUMBER"    ) ) style |= ES_NUMBER;
    if ( StrStrI(keyWords, "WANTRETURN") ) style |= ES_WANTRETURN;
    if ( StrStrI(keyWords, "OEM"       ) ) style |= ES_OEMCONVERT;

    /* Although these styles can be changed by individual ooDialog methods, as
     * a convenience, allow the programmer to include them when changing
     * multiple styles at once.
     */
    if ( StrStrI(keyWords, "TAB"  ) ) style |= WS_TABSTOP;
    if ( StrStrI(keyWords, "GROUP") ) style |= WS_GROUP;

    return style;
}


/**
 * Subclass window procdure that prevents an edit control from automatically
 * resizing itself on the WM_SIZE message.  This is useful if the programmer is
 * going to handle the resizing.
 *
 */
LRESULT CALLBACK EditSizeProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR dwData)
{
    switch ( msg )
    {
        case WM_SIZE:
            // Do not let the edit control resize itself.
            return TRUE;

        case WM_NCDESTROY:
            /* The window is being destroyed, remove the subclass, clean up
             * memory.
             */
            RemoveWindowSubclass(hwnd, EditSizeProc, id);
            LocalFree((HLOCAL)dwData);
            break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}


RexxMethod1(logical_t, e_isSingleLine, CSELF, pCSelf)
{
    return isSingleLineEdit(getDChCtrl(pCSelf));
}


RexxMethod1(RexxObjectPtr, e_selection, CSELF, pCSelf)
{
    pCDialogControl pcdc = (pCDialogControl)pCSelf;

    uint32_t start, end;
    SendMessage(pcdc->hCtrl, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);

    RexxDirectoryObject result = context->NewDirectory();
    context->DirectoryPut(result, context->UnsignedInt32(++start), "STARTCHAR");
    context->DirectoryPut(result, context->UnsignedInt32(++end), "ENDCHAR");
    return result;
}


/** Edit::replaceSelText()
 *
 *  Replaces the selected text in an edit control with the specified text.
 *
 *  @param  replacement  The text that replaces the selected text.
 *
 *  @param  canUndo      [OPTIONAL]  If true, the replacement can be undone, if
 *                       false it can not be undone.  The default is true.
 *
 *  @return  0, always.  MSDN docs say: This message does not return a value.
 */
RexxMethod3(RexxObjectPtr, e_replaceSelText, CSTRING, replacement, OPTIONAL_logical_t, canUndo, CSELF, pCSelf)
{
    if ( argumentOmitted(2) )
    {
        canUndo = TRUE;
    }
    SendMessage(((pCDialogControl)pCSelf)->hCtrl, EM_REPLACESEL, canUndo, (LPARAM)replacement);
    return TheZeroObj;
}


/** Edit::lineIndex()
 *
 *  Gets the character index of the first character of a specified line in a
 *  multiline edit control. A character index is the one-based index of the
 *  character from the beginning of the edit control.
 *
 *  @param  lineNumber  The non-negative, one-based index, of the line whose
 *                      character index is desired.  A value of 0 specifies the
 *                      current line number (the line that contains the caret).
 *
 *  @return On success, the one-based character index.  On error, 0 is returned,
 *          for instance if the specified line index is not valid.  (Greater
 *          than the lines in the edit control.)
 *
 *  @note  The lineIndex() method is intended for multi-line edit controls,
 *         however, it will behave as documented for single-line edit controls.
 *         The return is always 1 for a single-line if the lineNumber argument
 *         is 1 or 0 and always 0 for any other value.
 */
RexxMethod2(uint32_t, e_lineIndex, OPTIONAL_uint32_t, lineNumber, CSELF, pCSelf)
{
    HWND hCtrl = getDChCtrl(pCSelf);
    uint32_t charIndex = 0;

    lineNumber--;

    if ( isSingleLineEdit(hCtrl) )
    {
        if ( lineNumber == 1 || lineNumber == 0 )
        {
            charIndex = 1;
        }
    }
    else
    {
        charIndex = (uint32_t)SendMessage(hCtrl, EM_LINEINDEX, lineNumber, 0);
        charIndex++;
    }

    return charIndex;
}


/** Edit::lineFromIndex()
 *
 *  Retrieves the line index containing the character at the specified character
 *  index.
 *
 *  @param charIndex  A non-negative number specifying the one-based character
 *                    index of the character whose line number is needed. If
 *                    charIndex is 0 then this method retrieves the current line
 *                    number (the line the caret is on,) or, if there is a
 *                    selection, then the line containging the current
 *                    selection.
 *
 *  @return  The one-based line index containing the specified character.
 *
 *  @notes  If the character index is beyond the end of the text in the edit
 *          control, the index of the last line is returned.
 *
 *  @remarks  If charIndex is omitted, its value will be 0.  0 is the default
 *            for the argument, so we do not need to check if it is omitted or
 *            not.
 *
 *            Experimentation shows that the index of the last line is returned
 *            when char index is not valid. MSDN is not specific on this, but,
 *            it does not say that -1 is returned
 */
RexxMethod2(uint32_t, e_lineFromIndex, OPTIONAL_uint32_t, charIndex, CSELF, pCSelf)
{
    HWND hCtrl = getDChCtrl(pCSelf);

    charIndex--;
    uint32_t lineIndex = (uint32_t)SendMessage(hCtrl, EM_LINEFROMCHAR, charIndex, 0);

    return ++lineIndex;
}


/** Edit::lineLength()
 *
 *  Returns the length, in characters, of the specified line in an edit control.
 *
 *  @param  lineIndex  [optional] The one-based index of the line whose length
 *                     is needed.
 *
 *                     This argument can be 0. The default if omitted is 0.  See
 *                     the @notes.
 *
 *  @return On success, the number of charcters in the line specified, not
 *          including the carriage return at the end of the line. On error -1.
 *          In particular, -1 is returned if lineIndex is greate than the number
 *          of lines in the edit control.
 *
 *
 *  @notes  If lineIndex is 0 the return changes in this fashion:
 *
 *          If there is no selection, the number of characters in the current
 *          line is returned.  The current line is the line with the caret in
 *          it.
 *
 *          If there is a selection, this method returns the number of
 *          unselected characters on lines containing selected characters.
 *
 *          For example, if the selection started at the fourth character on one
 *          line through the next line up through the fourth character from the
 *          end of the line, the return value would be 6 (three characters on
 *          the first line and three on the next).
 *
 *
 */
RexxMethod2(RexxObjectPtr, e_lineLength, OPTIONAL_uint32_t, lineIndex, CSELF, pCSelf)
{
    HWND hCtrl = getDChCtrl(pCSelf);

    lineIndex--;
    uint32_t charIndex = lineIndex;

    if ( lineIndex != (uint32_t)-1 )
    {
        charIndex = (uint32_t)SendMessage(hCtrl, EM_LINEINDEX, lineIndex, 0);
        if ( charIndex == (uint32_t)-1 )
        {
            return TheNegativeOneObj;
        }
    }

    // Reuse lineIndex here to get the result.
    lineIndex = (uint32_t)SendMessage(hCtrl, EM_LINELENGTH, charIndex, 0);
    if ( lineIndex == (uint32_t)-1 )
    {
        return TheNegativeOneObj;
    }

    return context->UnsignedInt32(lineIndex);
}


/** Edit::getLine()
 *
 *  Retrieves the text of the specified line.
 *
 *  @param  lineNumber  The one-base index of the line whose text is desired.
 *                      This value can be 0, in which case the text of the
 *                      current line is retrieved
 *
 *  @param  ignored     Prior to 4.0.1, ooDialog required the user to specify
 *                      how long the line was (or how much text to retrieve) if
 *                      the line was over 255 characters.  This restriction is
 *                      removed and this argument is simply ignored to provide
 *                      backward compatibility.
 *
 *  @return  The text of the specified line, or the empty string if an error
 *           occurs.  Recall that it is possible that the line specified
 *           actually contains no text. If there is an error see below:
 *
 *
 *  @notes  Sets the .SystemErrorCode.  This code is set on error, by us, the OS
 *          does not set any:
 *
 *          ERROR_NOT_SUPPORTED (50)   The request is not supported.
 *
 */
RexxMethod3(RexxStringObject, e_getLine, uint32_t, lineNumber, OPTIONAL_RexxObjectPtr, ignored, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    HWND hwnd = getDChCtrl(pCSelf);
    char *buf = NULL;
    RexxStringObject result = context->NullString();

    RexxMethodContext *c = context;

    lineNumber--;

    if ( isSingleLineEdit(hwnd) )
    {
        if ( lineNumber != 0 && lineNumber != (uint32_t)-1 )
        {
            oodSetSysErrCode(context->threadContext, ERROR_NOT_SUPPORTED);
            goto done_out;
        }
        rxGetWindowText(context, hwnd, &result);
    }
    else
    {
        uint32_t charIndex = (uint32_t)SendMessage(hwnd, EM_LINEINDEX, lineNumber, 0);
        if ( charIndex == (uint32_t)-1 )
        {
            oodSetSysErrCode(context->threadContext, ERROR_NOT_SUPPORTED);
            goto done_out;
        }

        WORD count = (WORD)SendMessage(hwnd, EM_LINELENGTH, charIndex, 0);
        if ( count == 0 )
        {
            // This could be an error, *if* charIndex is greater than the number
            // of characters in the edit control.  But, that should have been
            // caught above with EM_LINEINDEX, so it is not an error.  (Not
            // likely to be an error.
            goto done_out;
        }

        buf = (char *)LocalAlloc(LPTR, ++count);
        if ( buf == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }

        // If lineNumber == -1, then we need to get the line number of the
        // current line.  Count is the cout of the current line already.
        if ( lineNumber == (uint32_t)-1 )
        {
            lineNumber = (uint32_t)SendMessage(hwnd, EM_LINEFROMCHAR, -1, 0);
        }

        (*(WORD *)buf) = count;
        if ( SendMessage(hwnd, EM_GETLINE, lineNumber, (LPARAM)buf) != 0 )
        {
            result = context->String(buf);
        }
    }

done_out:
    safeLocalFree(buf);
    return result;
}


/** Edit::isGrandChild()
 *
 *  Notifies the framework that this edit control is a grandchild of the dialog
 *  and configures the underlying edit control to send some event notifications
 *  to the dialog, rather than its direcet parent.
 *
 *  @param  mthName  [optional] The method to be invoked in the Rexx dialog when
 *                   one of the 4 default events happens.  The default if
 *                   omitted is onEditGrandChildEvent()
 *
 *  @param  wantTab  [opitonal]  If the Rexx method should be invoked for the
 *                   TAB event.  The default is false, the method is not invoked
 *                   for the tab key event.
 *
 *  @return  True on success, otherwise false.
 *
 *  @notes   Requires common control library 6.2.0.
 *
 *           This method connects 4 event notifications from the grandchild edit
 *           control to the method in the Rexx dialog.  3 of the events are key
 *           down events, the RETURN, ESCAPE, and TAB key down events.  The
 *           other event is the KILLFOCUS event.  All events invoke the same
 *           method in the Rexx dialog.  One of the arguments sent to the event
 *           handler is a keyword that specifies which event happened.
 *
 *           By default, the method is not invoked for the TAB key down event,
 *           but that can be changed using the wantTab argument.
 *
 *           Sets the .SystemErrorCode.  This code is set on error, by us, the
 *           OS does not set any.
 *
 *          This edit control must be a singel line edit control, otherwise the
 *          system error code is set to:
 *
 *          ERROR_NOT_SUPPORTED (50)   The request is not supported.
 *
 *  @remarks  We need to always connect the VK_TAB key, even if the user does
 *            not want the TAB nofication.  The reason is that we need to use
 *            DLGC_WANTALLKEYS for WM_GETDLGCODE, which prevents the dialog
 *            manager from handling TAB.  I don't see any way of asking for
 *            RETURN and ESCAPE, but not TAB.  In the message processing loop,
 *            we simply do invoke the Rexx method unless CTRLTAG_WANTTAB is set
 *            in the tag.
 */
RexxMethod3(RexxObjectPtr, e_isGrandChild, OPTIONAL_CSTRING, mthName, OPTIONAL_logical_t, wantTab, CSELF, pCSelf)
{
    return genericIsGrandchild(context, mthName, wantTab, pCSelf, winEdit);
}


/** Edit::setTabStops()
 *
 *  Sets the tab stops for text copied into a multi-line edit control.
 *
 *  When text is copied to the control, any tab character in the text causes
 *  space to be generated up to the next tab stop.  This method is ignored if
 *  the edit control is a single-line edit control.
 *
 *  @param  tabStops  An array containing the tab stops.  Each tab stop is a
 *                    positive number expressed in dialog template units.  If
 *                    the array contains no elements, default tab stops are set
 *                    at every 32 dialog template units.
 *
 *                    If the array contains only 1 element at index 1, tab stops
 *                    are set at every n dialog template units, where n is the
 *                    distance at index 1.
 *
 *                    Otherwise, tab stops are set to the numbers contained in
 *                    the array.
 *
 *  @return  True if all tab stops were set, otherwise false.
 *
 *  @note  The array must contain all positive numbers (or be an empty array to
 *         set the default tab stops.)  Also, the array must not be sparse,
 *         i.e., it must not skip any array indexes.
 *
 *         The operating system will not do negative tab stops.  I.e. you can
 *         not do 15 35 20.  Also, note that when specifying an array of more
 *         than 1 tap stop, each tab stop is the absolute position of the tab
 *         stop, not the distance between the tab stops.
 *
 *         Under normal circumstances there is no way for a user to enter a tab
 *         character by typing in a multi-line edit control.  The edit control
 *         would need to be sub-classed and this is not provided by ooDialog.
 *         However, the user could use copy and paste to paste in text with
 *         tabs.  And, text placed in the edit control by the programmer can
 *         also contain tabs.
 */
RexxMethod2(logical_t, e_setTabStops, RexxArrayObject, tabStops, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    logical_t result = 0;
    HWND hwnd = getDChCtrl(pCSelf);
    uint32_t *buf = NULL;

    if ( isSingleLineEdit(hwnd) )
    {
        goto done_out;
    }

    size_t count = c->ArrayItems(tabStops);
    if ( count == 0 )
    {
        result = SendMessage(hwnd, EM_SETTABSTOPS, 0, 0);
        goto done_out;
    }

    buf = (uint32_t *)malloc(count * sizeof(uint32_t *));
    if ( buf == NULL )
    {
        outOfMemoryException(c->threadContext);
        goto done_out;
    }

    RexxObjectPtr item;
    uint32_t tabStop;
    for ( size_t i = 1; i <= count; i++ )
    {
        item = c->ArrayAt(tabStops, i);
        if ( item == NULLOBJECT )
        {
            sparseArrayException(context->threadContext, 1, i);
            goto done_out;
        }
        if ( ! c->UnsignedInt32(item, &tabStop) )
        {
            wrongObjInArrayException(c->threadContext, 1, i, "a positive number", item);
            goto done_out;
        }
        buf[i - 1] = tabStop;
    }

    result = SendMessage(hwnd, EM_SETTABSTOPS, count, (LPARAM)buf);

    // Redraw the text in the edit control.  This will resize the tabs if there
    // is already text in the edit control with tabs.
    InvalidateRect(hwnd, NULL, TRUE);

done_out:
    safeFree(buf);
    return result;
}


RexxMethod1(RexxObjectPtr, e_hideBallon, CSELF, pCSelf)
{
    if ( ! requiredComCtl32Version(context, context->GetMessageName(), COMCTL32_6_0)  )
    {
        return TheOneObj;
    }
    pCDialogControl pcdc = (pCDialogControl)pCSelf;
    return (Edit_HideBalloonTip(pcdc->hCtrl) ? TheZeroObj : TheOneObj);
}


RexxMethod4(RexxObjectPtr, e_showBallon, CSTRING, title, CSTRING, text, OPTIONAL_CSTRING, icon, CSELF, pCSelf)
{
    if ( ! requiredComCtl32Version(context, context->GetMessageName(), COMCTL32_6_0)  )
    {
        return TheOneObj;
    }
    pCDialogControl pcdc = (pCDialogControl)pCSelf;

    EDITBALLOONTIP tip;
    WCHAR wszTitle[128];
    WCHAR wszText[BALLON_MAX_TEXT + 1];

    // The title string has a limit of 99 characters / text is limited to 1023.
    if ( strlen(title) > BALLON_MAX_TITLE )
    {
        stringTooLongException(context->threadContext, 1, BALLON_MAX_TITLE, strlen(title));
        return TheOneObj;
    }
    if ( strlen(text) > BALLON_MAX_TEXT )
    {
        stringTooLongException(context->threadContext, 2, BALLON_MAX_TEXT, strlen(text));
        return TheOneObj;
    }

    putUnicodeText((LPWORD)wszTitle, title);
    putUnicodeText((LPWORD)wszText, text);

    tip.cbStruct = sizeof(tip);
    tip.pszText = wszText;
    tip.pszTitle = wszTitle;
    tip.ttiIcon = TTI_INFO;

    if ( argumentExists(3) )
    {
        switch( toupper(*icon) )
        {
            case 'E' :
                tip.ttiIcon = TTI_ERROR;
                break;
            case 'N' :
                tip.ttiIcon = TTI_NONE;
                break;
            case 'W' :
                tip.ttiIcon = TTI_WARNING;
                break;
        }
    }
    return (Edit_ShowBalloonTip(pcdc->hCtrl, &tip) ? TheZeroObj : TheOneObj);
}

/** Edit::setCue()
 *
 *  Sets the cue, or tip, text for the edit control.  This text prompts the user
 *  for what to enter in the edit control.
 *
 *  @param  text   The text for the tip.  The length of the text must be 255
 *                 characters or less.
 *  @param  show   [optional] Whether the cue should still display when the edit
 *                 control has the focus. The default is false, the cue text
 *                 disappears when the user sets focus to the edit control.
 *
 *  @return  0 on success, 1 on failure.
 *
 *  @notes  Requires ComCtl 6.0 or later. Note the restriction on the length of
 *          text.  You can not set a cue on a multi-line edit control.
 *
 *  @remarks  This method was originally written using the old API in 3.2.0.  At
 *            that time the method returned a bunch of negative numbers, -4 for
 *            wrong ComCtl version, -3 for this, -2 for that.  Unintentionally
 *            in the conversion to the C++ APIs, the negative return numbers got
 *            dropped and exceptions were raised.  We're going to just stick
 *            with this, some users might complain.
 *
 */
RexxMethod3(RexxObjectPtr, e_setCue, CSTRING, text, OPTIONAL_logical_t, show, CSELF, pCSelf)
{
    return cbEditSetCue(context, text, show, true, pCSelf);
}

/** Edit::getCue()
 *
 *  Retrieves the cue banner text, or the empty string if there is no cue set.
 *
 *  @return  The cue banner text on success, or the empty string on error and if
 *           no cue is set
 *
 *  @remarks  This simply does not seem to work under XP.  However, it does work
 *            in Vista and Windows 7.
 */
RexxMethod1(RexxStringObject, e_getCue, CSELF, pCSelf)
{
    return cbEditGetCue(context, true, pCSelf);
}


/** Edit::setRect()
 *
 *  Sets the formatting rectangle for the edit control.
 *
 *  @param rect  [required]  Either a .Rect object specifying the new formatting
 *               rectangle, or 0.  If rect is 0, the formatting rectangle is set
 *               back to its default.
 *
 *  @param redraw  [optional] Specifies whether or not to have the edit control
 *                 redraw itself.  If true, the edit controls will redraw
 *                 itself, if false it will not redraw.  The default is true.
 *
 *  @return  Zero, always.
 *
 */
RexxMethod3(RexxObjectPtr, e_setRect, RexxObjectPtr, rect, OPTIONAL_logical_t, redraw, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    if ( argumentOmitted(2) )
    {
        redraw = TRUE;
    }

    PORXRECT r = NULL;

    if ( rect != TheZeroObj )
    {
        r = rxGetRect(context, rect, 1);
        if ( r == NULL )
        {
            goto done_out;
        }
    }

    if ( redraw )
    {
        SendMessage(hwnd, EM_SETRECT, 0, (LPARAM)r);
    }
    else
    {
        SendMessage(hwnd, EM_SETRECTNP, 0, (LPARAM)r);
    }

done_out:
    return TheZeroObj;
}


/** Edit::getRect()
 *
 *  Retrieves the formatting rectangle for the edit control
 *
 *  @return  The the current formatting rectangle of the edit control as a .Rect
 *           object.
 */
RexxMethod1(RexxObjectPtr, e_getRect, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);
    ORXRECT r;

    SendMessage(hwnd, EM_GETRECT, 0, (LPARAM)&r);

    return rxNewRect(context, &r);
}


/** Edit::getStyle()
 *  Edit::replaceStyle()
 *  Edit::removeStyle()
 *  Edit::addStyle()
 *
 *  @param  _style1  Style to add for addStyle(), style to remove for
 *                   removeStyle() and replaceStyle().
 *  @param  _style2  Style to add for replaceStyle().
 */
RexxMethod4(RexxObjectPtr, e_style, OPTIONAL_CSTRING, _style1, OPTIONAL_CSTRING, _style2, NAME, method, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    pCDialogControl pcdc = (pCDialogControl)pCSelf;

    uint32_t style = GetWindowLong(pcdc->hCtrl, GWL_STYLE);
    if ( *method == 'G' )
    {
        return editStyleToString(context, style);
    }

    if ( argumentOmitted(1) )
    {
        return missingArgException(context->threadContext, 1);
    }

    uint32_t style1 = parseEditStyle(_style1);
    if ( *method == 'A' )
    {
        style |= style1;
    }
    else if ( method[2] == 'M' )
    {
        style &= ~style1;
    }
    else
    {
        if ( argumentOmitted(2) )
        {
            return missingArgException(context->threadContext, 2);
        }

        uint32_t style2 = parseEditStyle(_style2);
        style = (style & ~style1) | style2;
    }
    return setWindowStyle(context, pcdc->hCtrl, style);
}


/**
 * Methods for the ListBox class.
 */
#define LISTBOX_CLASS   "ListBox"


/*
 * Much of the functionality of the list box and combo box is basically the
 * same.  Some common functions follow that merely use different Window
 * messages.
 */

/**
 * Gets an array of the selected items indexes from a multiple selection list
 * box.
 *
 * @param hwnd   Handle of the list box.
 * @param count  The count of selected items / size of the array is returned
 *               here.  On a memory allocation error this is set to -1.  If
 *               there are no selected items this is set to 0.
 *
 * @return A pointer to a buffer containing the indexes.  Null is returned if
 *         there is a memory allocation, or if there are no selected items.  The
 *         caller is responsible for freeing the buffer if non-null is returned.
 *
 * @assumes The caller has already determined the list box is a multiple
 *          selection list box.
 */
static int32_t *getLBSelectedItems(HWND hwnd, int32_t *count)
{
    int32_t c = 0;
    int32_t *items = NULL;

    c = (int32_t)SendMessage(hwnd, LB_GETSELCOUNT, 0, 0);
    if ( c < 1 )
    {
        c = 0;
        goto done_out;
    }

    items = (int32_t *)malloc(c * sizeof(int32_t));
    if ( items == NULL )
    {
        c = -1;
        goto done_out;
    }

    if ( SendMessage(hwnd, LB_GETSELITEMS, c, (LPARAM)items) != c )
    {
        // Really should never happen.
        free(items);
        c = 0;
        items = NULL;
    }

done_out:
    *count = c;
    return items;
}


/*
 * Much of the functionality of the list box and combo box is basically the
 * same.  Some common functions follow that merely use different Window
 * messages.
 */

/**
 * Get the text of an item in a list box or a combo box.
 *
 * @return The Rexx string object that represents the text.
 */
RexxStringObject cbLbGetText(RexxMethodContext *c, HWND hCtrl, uint32_t index, oodControl_t ctrl)
{
    RexxStringObject result = c->NullString();

    if ( index-- > 0 )
    {
        uint32_t msg = (ctrl == winComboBox ? CB_GETLBTEXTLEN : LB_GETTEXTLEN);

        LRESULT l = SendMessage(hCtrl, msg, index, 0);
        if ( l > 0 )
        {
            char *buf = (char *)malloc(l + 1);
            if ( buf == NULL )
            {
                outOfMemoryException(c->threadContext);
                return result;
            }

            msg = (ctrl == winComboBox ? CB_GETLBTEXT : LB_GETTEXT);
            l = SendMessage(hCtrl, msg, index, (LPARAM)buf);
            if ( l > 0 )
            {
                result = c->String(buf);
            }
            free(buf);
        }
    }
    return result;
}


/**
 * Insert an item into a single selection list box or a combo box.  Note that
 * this will not work properly for a multiple selection list box.
 */
static int32_t cbLbInsert(RexxMethodContext *context, HWND hCtrl, int32_t index, CSTRING text, oodControl_t ctrl)
{
    uint32_t msg;

    if ( argumentOmitted(1) )
    {
        msg = ( ctrl == winComboBox ? CB_GETCURSEL : LB_GETCURSEL);
        index = (int32_t)SendMessage(hCtrl, msg, 0, 0);
    }
    else
    {
        // If index is 0 or -1, then that is the correct index already.
        if ( index > 0 )
        {
            index--;
        }
        else if ( index < -1 )
        {
            index = -1;
        }
    }

    msg = (ctrl == winComboBox ? CB_INSERTSTRING : LB_INSERTSTRING);

    int32_t ret = (int32_t)SendMessage(hCtrl, msg, (WPARAM)index, (LPARAM)text);
    if ( ret >= 0 )
    {
        ret++;
    }
    return ret;
}

static int32_t cbLbSelect(HWND hCtrl, CSTRING text, oodControl_t ctrl)
{
    uint32_t msg = (ctrl == winComboBox ? CB_FINDSTRING : LB_FINDSTRING);

    int32_t index = (int32_t)SendMessage(hCtrl, msg, 0, (LPARAM)text);
    if ( index < 0 )
    {
        return 0;
    }

    msg = (ctrl == winComboBox ? CB_SETCURSEL : LB_SETCURSEL);

    // LB_SETCURSEL is only for single selection list boxes and LB_SETSEL is
    // only for multiple selection list boxes

    if ( msg == LB_SETCURSEL && ! isSingleSelectionListBox(hCtrl) )
    {
        index = (SendMessage(hCtrl, LB_SETSEL, TRUE, index) == 0 ? index + 1 : 0);
    }
    else
    {
        index = (SendMessage(hCtrl, msg, index, 0) < 0 ? 0 : index + 1);
    }

    return index;
}

static int32_t cbLbFind(HWND hCtrl, CSTRING text, uint32_t startIndex, CSTRING exactly, oodControl_t ctrl)
{
    bool exact = false;
    if ( exactly != NULL && (*exactly == '1' || toupper(*exactly) == 'E') )
    {
        exact = true;
    }

    int32_t index = startIndex == 0 ? -1 : --startIndex;

    int32_t found;
    uint32_t msg;
    if ( exact )
    {
        msg = (ctrl == winComboBox ? CB_FINDSTRINGEXACT : LB_FINDSTRINGEXACT);
    }
    else
    {
        msg = (ctrl == winComboBox ? CB_FINDSTRING : LB_FINDSTRING);
    }

    found = (int32_t)SendMessage(hCtrl, msg, index, (LPARAM)text);

    return (found == CB_ERR ? 0 : ++found);
}


static int32_t cbLbAddDirectory(HWND hCtrl, CSTRING drivePath, CSTRING fileAttributes, oodControl_t ctrl)
{
    uint32_t attributes = DDL_READWRITE;
    if ( fileAttributes != NULL && *fileAttributes != '\0' )
    {
        if ( StrStrI(fileAttributes, "READWRITE") != 0 ) attributes |= DDL_READWRITE;
        if ( StrStrI(fileAttributes, "READONLY" ) != 0 ) attributes |= DDL_READONLY;
        if ( StrStrI(fileAttributes, "HIDDEN"   ) != 0 ) attributes |= DDL_HIDDEN;
        if ( StrStrI(fileAttributes, "SYSTEM"   ) != 0 ) attributes |= DDL_SYSTEM;
        if ( StrStrI(fileAttributes, "DIRECTORY") != 0 ) attributes |= DDL_DIRECTORY;
        if ( StrStrI(fileAttributes, "ARCHIVE"  ) != 0 ) attributes |= DDL_ARCHIVE;
        if ( StrStrI(fileAttributes, "EXCLUSIVE") != 0 ) attributes |= DDL_EXCLUSIVE;
        if ( StrStrI(fileAttributes, "DRIVES"   ) != 0 ) attributes |= DDL_DRIVES;
    }
    uint32_t msg = (ctrl == winComboBox ? CB_DIR : LB_DIR);

    return (int32_t)SendMessage(hCtrl, msg, attributes, (LPARAM)drivePath);
}


/** ListBox::add()
 *
 *  Adds a string item to the list box.
 *
 *  @param  The string to add.
 *
 *  @return  The 1-based index of the added item on success.  -1 (LB_ERR) on
 *           error and -2 (LB_ERRSPACE) if there is not enough room for the new
 *           string.
 */
RexxMethod2(int32_t, lb_add, CSTRING, text, CSELF, pCSelf)
{
    int32_t ret = (int32_t)SendMessage(((pCDialogControl)pCSelf)->hCtrl, LB_ADDSTRING, 0, (LPARAM)text);
    if ( ret >= 0 )
    {
        ret++;
    }
    return ret;
}


/** ListBox::addDirectory()
 *
 *
 */
RexxMethod3(int32_t, lb_addDirectory, CSTRING, drivePath, OPTIONAL_CSTRING, fileAttributes, CSELF, pCSelf)
{
    return cbLbAddDirectory(((pCDialogControl)pCSelf)->hCtrl, drivePath, fileAttributes, winListBox);
}


/** ListBox::deselectIndex()
 *
 *  Deselects the specified item, or all items, in the list box.
 *
 *  @param  index  [OPTIONAL]  The one-based index of the item to deselect.  If
 *                 this argument is omitted, 0 or -1, all items in the list box
 *                 are deselected.
 *
 *  @return  -1 on error, otherwise 0.
 *
 *  @note  If the list box is a single-selection list box, the index argument is
 *         simply ignored.  The return will always be 0.  For a
 *         multiple-selection list box, if index is greater than the last item
 *         in the listbox, -1 is returned.
 */
RexxMethod2(int32_t, lb_deselectIndex, OPTIONAL_int32_t, index, CSELF, pCSelf)
{
    HWND hCtrl = getDChCtrl(pCSelf);
    int32_t ret;

    if ( isSingleSelectionListBox(hCtrl) )
    {
        ret = ((int32_t)SendMessage(hCtrl, LB_SETCURSEL, -1, 0) != -1 ? -1 : 0);
    }
    else
    {
        index = (index <= 0 ? -1 : index - 1);
        ret = (int32_t)SendMessage(hCtrl, LB_SETSEL, FALSE, index);
    }
    return ret;
}


/** ListBox::find()
 *
 *  Finds the index of the list box item that matches 'text'.  In all cases
 *  the search is case insensitive.
 *
 *  @param  text        The text of the item to search for.  If not exact, this
 *                      can be just an abbreviation, or prefix, of the item.
 *                      Otherwise an exact match, disregarding case, is searched
 *                      for.
 *
 *  @param  startIndex  [OPTIONAL] The one-based index of the item to start the
 *                      search at.  If the search reaches the end of the items
 *                      without a match, the search continues with the first
 *                      item until all items have been examined.  When
 *                      omitted, or 0, the search starts with the first item.
 *
 *  @param  exactly     [OPTIONAL]  Whether to do an exact match.  When this
 *                      arugment is omitted, 'text' can just the abbreviation of
 *                      the item to find.  I.e., 'San' would match "San Diego."
 *                      If the argument is used and equals true or "Exact" then
 *                      the item must match text exactly.  When using the
 *                      "Exact" form, only the first letter is considered and
 *                      case is insignificant.
 *
 *  @return  The one-based index of the item, if found, otherwise 0.
 */
RexxMethod4(int32_t, lb_find, CSTRING, text, OPTIONAL_uint32_t, startIndex, OPTIONAL_CSTRING, exactly, CSELF, pCSelf)
{

    return cbLbFind(((pCDialogControl)pCSelf)->hCtrl, text, startIndex, exactly, winListBox);
}


/** ListBox::getText()
 *
 *  Return the text of the item at the specified index.
 *
 *  @param  index  The 1-based item index.  (The underlying list box uses
 *                 0-based indexes.)
 *
 *  @return  The item's text or the empty string on error.
 */
RexxMethod2(RexxObjectPtr, lb_getText, uint32_t, index, CSELF, pCSelf)
{
    return cbLbGetText(context, ((pCDialogControl)pCSelf)->hCtrl, index, winListBox);
}


/** ListBox::hitTestInfo()
 *
 *  Gets the one-based index of the item nearest the specified point in this
 *  list box.
 *
 *  @param  pt  [required]  The position, x and y co-ordinates, of the point to
 *              test. This can be specified in two forms.
 *
 *      Form 1:  arg 1 is a .Point object.
 *      Form 2:  arg 1 is the x co-ordinate and arg2 is the y co-ordinate.
 *
 *  @param  info  [optional in/out]  A directory object in which all hit info is
 *                returned.  If the directory is supplied, on return the
 *                directory will have these indexes:
 *
 *                inClientArea    True if the point is in the clien area of the
 *                                list box.  False if it is not in the client
 *                                area
 *
 *                itemIndex       Same value as the return.  The index of
 *                                the item nearest the specified point.
 *
 *  @return  The index of the item nearest the point.
 *
 *  @note    Any x, y coordinates will work.  I.e. -6000, -7000 will work. The
 *           item index will be 1 and inClientArea will be false.  The operating
 *           system always returns the item index that is *closest* to the point
 *           specified.  Even if it is not very close at all.
 */
RexxMethod2(int32_t, lb_hitTestInfo, ARGLIST, args, CSELF, pCSelf)
{
    int32_t result = -1;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }
    HWND hwnd = pcdc->hCtrl;

    size_t sizeArray;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 1, 3, &sizeArray, &argsUsed) )
    {
        goto done_out;
    }

    bool haveDirectory = (sizeArray > argsUsed) ? true : false;
    RexxDirectoryObject info;

    // Check arg count against expected.
    if ( sizeArray > (haveDirectory ? argsUsed + 1 : argsUsed) )
    {
        tooManyArgsException(context->threadContext, (haveDirectory ? argsUsed + 1 : argsUsed));
        goto done_out;
    }

    if ( haveDirectory )
    {
        RexxObjectPtr _info = context->ArrayAt(args, argsUsed + 1);
        if ( _info == NULLOBJECT || ! context->IsDirectory(_info) )
        {
            wrongClassException(context->threadContext, argsUsed + 1, "Directory");
            goto done_out;
        }

        info = (RexxDirectoryObject)_info;
    }

    LPARAM ret = SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(point.x, point.y));

    result = LOWORD(ret);
    result++;

    if ( haveDirectory )
    {
        context->DirectoryPut(info, context->Int32(result), "ITEMINDEX");
        context->DirectoryPut(info, context->Logical(! HIWORD(ret)), "INCLIENTAREA");
    }

done_out:
    return result;
}

/** ListBox::insert()
 *
 *  Inserts a string item into the list box at the index specified.
 *
 *  @param  index  [OPTIONAL] The one-based index of where the item is to be
 *                 inerted.  If index is -1, the item is inserted at the end of
 *                 the list.  If the index is 0, the item is insererted at the
 *                 beginning of the list.  (Numbers less than -1 are treated as
 *                 -1.)
 *
 *                 When this argument is omitted, the item is inserted after the
 *                 selected item (after the last selected item on multiple
 *                 selection list boxes.)  If there is no selected item, the new
 *                 item is inserted as the last item.
 *
 *  @param  text   The text of the item being inserted.
 *
 *  @return  The 1-based index of the inserted item on success.  -1 (LB_ERR) on
 *           error and -2 (LB_ERRSPACE) if there is not enough room for the new
 *           item.
 */
RexxMethod3(int32_t, lb_insert, OPTIONAL_int32_t, index, CSTRING, text, CSELF, pCSelf)
{
    HWND hwnd = ((pCDialogControl)pCSelf)->hCtrl;

    if ( isSingleSelectionListBox(hwnd) )
    {
        return cbLbInsert(context, hwnd, index, text, winListBox);
    }

    if ( argumentOmitted(1) )
    {
        int32_t count;
        int32_t *items = getLBSelectedItems(hwnd, &count);
        if ( count < 1 || items == NULL )
        {
            index = -1;
        }
        else
        {
            index = items[count - 1] + 1;
            free(items);
        }
    }
    else
    {
        if ( index > 0 )
        {
            index--;
        }
        else if ( index < -1 )
        {
            index = -1;
        }
    }

    int32_t ret = (int32_t)SendMessage(hwnd, LB_INSERTSTRING, (WPARAM)index, (LPARAM)text);
    if ( ret >= 0 )
    {
        ret++;
    }
    return ret;
}


/** ListBox::isSingleSelection()
 *
 *
 */
RexxMethod1(RexxObjectPtr, lb_isSingleSelection, CSELF, pCSelf)
{
    return (isSingleSelectionListBox(getDChCtrl(pCSelf)) ? TheTrueObj : TheFalseObj);
}


/** ListBox::select()
 *
 *  Selects the item in the list box that that begins with the letters specified
 *  by text.  The search is case insensitive.  I.e., if text is 'new' it would
 *  select "New York".
 *
 *  @param  text  The text (or prefix) of the item to select.
 *
 *  @return  The one-based index of the item selected. 0 if no matching entry
 *           was found, or some other error.
 */
RexxMethod2(int32_t, lb_select, CSTRING, text, CSELF, pCSelf)
{
    return cbLbSelect(((pCDialogControl)pCSelf)->hCtrl, text, winListBox);
}


/** ListBox::selectedIndex()
 *
 *  Returns the index of the currently selected item in the list box.
 *
 *  If the list box is a multiple selection list box, and more than one item is
 *  selected, the index of the selected item that has the focus rectangle is
 *  returned.  If none of the selected items has the focus rectangle, the index
 *  least in value of the selected items is returned.
 *
 *  @return  The one-based index of the selected item as explained in above.  If
 *           there is no item selected, or some other error, then 0 is returned.
 *
 *  @remarks  Pre 4.0.1, for multiple selection list boxes, this method returned
 *            the index of the item with the focus rectangle.  This has nothing
 *            to do with the selected item.  In addition, the MSDN docs say for
 *            the window message being used: "Do not send this message to a
 *            multiple-selection list box."  This resulted in non-deterministic
 *            behavior if the Rexx programmer used this method for a multiple
 *            selection list box.
 *
 *            Because of this, the implementation is slightly changed, to return
 *            a deterministic index.
 */
RexxMethod1(int32_t, lb_selectedIndex, CSELF, pCSelf)
{
    HWND hCtrl = getDChCtrl(pCSelf);

    int32_t *items = NULL;
    int32_t index = index = (int32_t)SendMessage(hCtrl, LB_GETCURSEL, 0, 0);

    if ( ! isSingleSelectionListBox(hCtrl) )
    {
        // For a multi-selection list box, index should be the item that has the
        // focus rectangle.  If index is LB_ERR, it is an error of course.
        if ( index == LB_ERR )
        {
            goto done_out;
        }

        // We get all the selected items.  If only 1 item, we use it.  Otherwise
        // we try to match a selected item with the focuse rectangle.  If no
        // match, we use the first selected item.  (First by index.)

        int32_t count;
        items = getLBSelectedItems(hCtrl, &count);
        if ( items == NULL || count < 1 )
        {
            // Doubt that this could happen, except for a malloc error.
            index = LB_ERR;
            goto done_out;
        }
        if ( count == 1 )
        {
            index = *items;
            goto done_out;
        }

        for ( int32_t i = 0; i < count; i++)
        {
            if ( index == items[i] )
            {
                goto done_out;
            }
        }
        index = items[0];
    }

done_out:
    safeFree(items);
    return (index >= 0 ? index + 1 : 0);
}


/** ListBox::selectIndex()
 *
 *  Selects the specified item in the list box.
 *
 *  @param  index  [OPTIONAL]  The one-based index of the item to select.  See
 *                 the notes for the behavior is this argument is omitted or 0.
 *                 For a multiple-selection list box only, if this argument is
 *                 -1, then all items in the list box are selected.
 *
 *  @return  False on error, true on no error.
 *
 *  @note    For backwards compatibility, if the index argument is omitted, or
 *           0, the selection is removed from all items in the list box.  But
 *           really, the deselectIndex() method should be used.
 */
RexxMethod2(int32_t, lb_selectIndex, OPTIONAL_int32_t, index, CSELF, pCSelf)
{
    HWND hCtrl = getDChCtrl(pCSelf);
    int32_t ret;

    bool backwardCompat = (argumentOmitted(1) || index == 0 ? true : false);

    if ( isSingleSelectionListBox(hCtrl) )
    {
        index = (backwardCompat ? -1 : index - 1);
        ret = (int32_t)SendMessage(hCtrl, LB_SETCURSEL, index, 0);
        ret = (ret != index ? 0 : 1);
    }
    else
    {
        if ( backwardCompat )
        {
            ret = (int32_t)SendMessage(hCtrl, LB_SETSEL, FALSE, -1);
        }
        else
        {
            index = (index < 0 ? -1 : index - 1);
            ret = (int32_t)SendMessage(hCtrl, LB_SETSEL, TRUE, index);
        }
        ret = (ret == -1 ? 0 : 1);
    }
    return ret;
}



/**
 * Methods for the ComboBox class.
 */
#define COMBOBOX_CLASS   "ComboBox"


/**
 * Returns the user data for the specified combo box item as a Rexx object
 *
 * @param hComboBox
 * @param index
 *
 * @return The Rexx object set as the user data, or the .nil object if no user
 *         data is set.  Returns NULLOBJECT on a combo box error.
 */
static RexxObjectPtr getCurrentComboBoxUserData(HWND hComboBox, uint32_t index)
{
    RexxObjectPtr result = TheNilObj;

    LRESULT iData = SendMessage(hComboBox, CB_GETITEMDATA, index, 0);
    if ( iData == CB_ERR )
    {
        result = NULLOBJECT;
    }
    else if ( iData != NULL )
    {
        result = (RexxObjectPtr)iData;
    }
    return result;
}


void freeComboBoxData(pSubClassData pSCData)
{
    if ( pSCData )
    {
        // This is just a color table entry at this time.
        if ( pSCData->pData )
        {
            COLORTABLEENTRY *cte = (COLORTABLEENTRY *)pSCData->pData;
            if ( ! cte->isSysBrush )
            {
                DeleteObject(cte->ColorBrush);
            }
        }
        safeLocalFree(pSCData->pData);
        pSCData->pData = NULL;
    }
}


/**
 * Handles the WM_CTLCOLORxxx messages for the combo box subclass, i.e. for the
 * setFullColor() and removeFullColor() methods.  Called from the subclass
 * procedure.
 *
 * @param pSCData
 * @param hwnd
 * @param msg
 * @param wParam
 * @param lParam
 * @param tag
 *
 * @return LRESULT
 *
 * @note  In the regular control color code, we need to determine if we are
 *        using system colors or not because ColorBk and ColorFG have not been
 *        converted. But for this subclassed combobox we have already converted
 *        those colors to system colors during setFullColor().
 *
 *        For removeFullColor() we can not actually remove the subclass because
 *        it is possible the user has also invoked some other method that
 *        involves subclassing the specific combobox.  So what we do is look for
 *        hBrush set back to null and FG / BK both set to CLR_DEFAULT.  When we
 *        see this we just pass the message on to the combo box without any
 *        intervention.
 */
LRESULT comboBoxColor(pSubClassData pSCData, HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam, uint32_t tag)
{
    COLORTABLEENTRY *cte = (COLORTABLEENTRY *)pSCData->pData;
    HBRUSH hBrush        = cte->ColorBrush;

    if ( hBrush == NULL && cte->ColorBk == CLR_DEFAULT && cte->ColorFG == CLR_DEFAULT )
    {
        return DefSubclassProc(hwnd, msg, wParam, lParam);
    }

    if ( cte->ColorBk != CLR_DEFAULT )
    {
        SetBkColor((HDC)wParam, cte->ColorBk);
        if ( cte->ColorFG != CLR_DEFAULT )
        {
            SetTextColor((HDC)wParam, cte->ColorFG);
        }
    }
    else if ( cte->ColorFG != CLR_DEFAULT )
    {
        // We are only setting the text color, so I guess the brush should
        // be what the combo box would use.  We get that brush by sending the
        // message on to the combo box and saving what is returned.
        hBrush = (HBRUSH)DefSubclassProc(hwnd, msg, wParam, lParam);

        SetBkMode((HDC)wParam, TRANSPARENT);
        SetTextColor((HDC)wParam, cte->ColorFG);
    }

    return (LRESULT)hBrush;
}


/** ComboBox::add()
 *
 *  Adds a string entry to the combo box.
 *
 *  @param  text  The string to add.
 *
 *  @return  The 1-based index of the added entry on success.  -1 (CB_ERR) on
 *           error and -2 (CB_ERRSPACE) if there is not enough room for the new
 *           entry.
 */
RexxMethod2(int32_t, cb_add, CSTRING, text, CSELF, pCSelf)
{
    int32_t ret = (int32_t)SendMessage(((pCDialogControl)pCSelf)->hCtrl, CB_ADDSTRING, 0, (LPARAM)text);
    if ( ret >= 0 )
    {
        ret++;
    }
    return ret;
}


/** ComboBox::addDirectory()
 *
 */
RexxMethod3(int32_t, cb_addDirectory, CSTRING, drivePath, OPTIONAL_CSTRING, fileAttributes, CSELF, pCSelf)
{
    return cbLbAddDirectory(((pCDialogControl)pCSelf)->hCtrl, drivePath, fileAttributes, winComboBox);
}


/** ComboBox::find()
 *
 *  Finds the index of the combo box entry that matches 'text'.  In all cases
 *  the search is case insensitive.
 *
 *  @param  text        The text of the entry to search for.  If not exact, this
 *                      can be just an abbreviation, or prefix, of the entry.
 *                      Otherwise an exact match, disregarding case, is searched
 *                      for.
 *
 *  @param  startIndex  [OPTIONAL] The one-based index of the entry to start the
 *                      search at.  If the search reaches the end of the entries
 *                      without a match, the search continues with the first
 *                      entry until all entries have been examined.  When
 *                      omitted, or 0, the search starts with the first entry.
 *
 *  @param  exactly     [OPTIONAL]  Whether to do an exact match.  When this
 *                      arugment is omitted, 'text' can just the abbreviation of
 *                      the entry to find.  I.e., 'San' would match "San Diego."
 *                      If the argument is used and equals true or "Exact" then
 *                      the entry must match text exactly.  When using the
 *                      "Exact" form, only the first letter is considered and
 *                      case is insignificant.
 *
 *  @return  The one-based index of the entry, if found, otherwise 0.
 */
RexxMethod4(int32_t, cb_find, CSTRING, text, OPTIONAL_uint32_t, startIndex, OPTIONAL_CSTRING, exactly, CSELF, pCSelf)
{
    return cbLbFind(((pCDialogControl)pCSelf)->hCtrl, text, startIndex, exactly, winComboBox);
}


/** ComboBox::getComboBoxInfo()
 *
 *  Returns a Directory object containing information for this combo box.
 *
 *  @remarks  The COMBOBOXINFO struct has a field, hwndItem, which is the edit
 *            control for simple and drop-down combo boxes.  For a drop-down
 *            list combo box I thought it would be a static control.  But,
 *            testing seems to show it is the combo box itself.  Rather than try
 *            to guess what type of control it is, we just use
 *            controlHwnd2controltype() to determine its type.
 */
RexxMethod1(RexxObjectPtr, cb_getComboBoxInfo, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return NULLOBJECT;
    }

    RexxObjectPtr result = TheNilObj;
    COMBOBOXINFO  cbi    = { sizeof(COMBOBOXINFO) };

    if ( GetComboBoxInfo(pcdc->hCtrl, &cbi) )
    {
        RexxDirectoryObject d    = context->NewDirectory();
        oodControl_t        type = controlHwnd2controlType(cbi.hwndItem);

        RexxObjectPtr temp = createControlFromHwnd(context, pcdc, cbi.hwndItem, type, true);
        context->DirectoryPut(d, temp, "TEXTOBJ");

        temp = createControlFromHwnd(context, pcdc, cbi.hwndList, winComboLBox, true);
        context->DirectoryPut(d, temp, "LISTBOXOBJ");

        temp = rxNewRect(context, (PORXRECT)&cbi.rcButton);
        context->DirectoryPut(d, temp, "BUTTONRECT");

        temp = rxNewRect(context, (PORXRECT)&cbi.rcItem);
        context->DirectoryPut(d, temp, "TEXTRECT");

        CSTRING state = "error";
        if ( cbi.stateButton == 0 )                          state = "notpressed";
        else if ( cbi.stateButton == STATE_SYSTEM_INVISIBLE ) state = "absent";
        else if ( cbi.stateButton == STATE_SYSTEM_PRESSED )   state = "pressed";

        temp = context->String(state);
        context->DirectoryPut(d, temp, "BUTTONSTATE");

        result = d;
    }
    else
    {
        oodSetSysErrCode(context->threadContext);
    }

    return result;
}


/** ComboBox::getCue()
 *
 *  Retrieves the cue banner text, or the empty string if there is no cue set.
 *
 *  @return  The cue banner text on success, or the empty string on error and if
 *           no cue is set
 *
 *  @remarks  For an edit control, this simply does not seem to work under XP.
 *            However, it does work in Vista and Windows 7.
 */
RexxMethod1(RexxStringObject, cb_getCue, CSELF, pCSelf)
{
    return cbEditGetCue(context, false, pCSelf);
}


/** ComboBox::getEditControl()
 *
 *  Returns a Rexx Edit object that reprsents the child edit control used by the
 *  combo box.
 *
 *  @notes  Sets the .SystemErrorCode.
 */
RexxMethod1(RexxObjectPtr, cb_getEditControl, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr result = TheNilObj;
    COMBOBOXINFO  cbi    = { sizeof(COMBOBOXINFO) };

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }

    if ( isDropDownListCB(pcdc->hCtrl) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_COMBOBOX_MESSAGE);
        goto done_out;
    }

    if ( GetComboBoxInfo(pcdc->hCtrl, &cbi) )
    {
        result = createControlFromHwnd(context, pcdc, cbi.hwndItem, winEdit, true);
    }
    else
    {
        oodSetSysErrCode(context->threadContext);
    }

done_out:
    return result;
}


/** ComboBox::getItemData()
 *
 *  Returns the user data associated with the specified combo box item, or .nil
 *  if there is no user data associated.
 *
 *  @param  index  [required]  The one-based index of the item whose user data
 *                 is to be retrieved.
 *
 *  @return  Returns the associated user data, or .nil if there is no associated
 *           data.
 */
RexxMethod2(RexxObjectPtr, cb_getItemData, uint32_t, index, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return TheNilObj;
    }

    index--;
    RexxObjectPtr result = getCurrentComboBoxUserData(pcdc->hCtrl, index);
    if ( result == NULLOBJECT )
    {
        oodSetSysErrCode(context->threadContext, ERROR_SIGNAL_REFUSED);
        return TheNilObj;
    }
    return result;
}


/** ComboBox::getItemHeight()
 *
 *  Determines the height of the list items or the height of the selection field
 *  in this combo box.
 *
 *  @param  getSelectionField  [optional]  If true get the selection field
 *                             height, if false get the item height.  The
 *                             default is false, get the item height.
 *
 *  @return  The height in pixels of the list box items or the selection field
 *           as specified.  On error returns -1
 */
RexxMethod2(int32_t, cb_getItemHeight, OPTIONAL_logical_t, getSelectionField, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }

    WPARAM which = getSelectionField ? -1 : 0;
    return (int32_t)SendMessage(pcdc->hCtrl, CB_GETITEMHEIGHT, which, 0);
}


/** ComboBox::getMinVisible()
 *
 * Retrieves the minimum visible number for this combo box.
 *
 * @notes  Requires Common Controls Library 6.0
 */
RexxMethod1(int32_t, cb_getMinVisible, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }
    if ( ! requiredComCtl32Version(context, "getMinVisible", COMCTL32_6_0)  )
    {
        return 0;
    }

    return ComboBox_GetMinVisible(pcdc->hCtrl);
}


/** ComboBox::getText()
 *
 *  Return the text of the item at the specified index.
 *
 *  @param  index  The 1-based item index.  (The underlying combo box uses
 *                 0-based indexes.)
 *
 *  @return  The item's text or the empty string on error.
 */
RexxMethod2(RexxStringObject, cb_getText, uint32_t, index, CSELF, pCSelf)
{
    return cbLbGetText(context, ((pCDialogControl)pCSelf)->hCtrl, index, winComboBox);
}


/** ComboBox::insert()
 *
 *  Inserts a string entry into the cobo box at the index specified.
 *
 *  @param  index  [OPTIONAL]  The one-based index of where the entry is to be
 *                 inserted.  If index is less than 0, the entry is inserted at
 *                 the end of the entries.  If index is 0, the new entry is
 *                 inserted as the first entry.
 *
 *                 When this argument is omitted, the entry is inserted after
 *                 the current selected entry.  If there is no selected entry,
 *                 the new entry is inserted as the last entry.
 *
 *  @param  text   The string to insert.
 *
 *  @return  The 1-based index of the added entry on success.  -1 (CB_ERR) on
 *           error and -2 (CB_ERRSPACE) if there is not enough room for the new
 *           entry.
 */
RexxMethod3(int32_t, cb_insert, OPTIONAL_int32_t, index, CSTRING, text, CSELF, pCSelf)
{
    return cbLbInsert(context, ((pCDialogControl)pCSelf)->hCtrl, index, text, winComboBox);
}

/** ComboBox::isDropDown()
 ** ComboBox::isDropDownList()
 ** ComboBox::isSimple()
 *
 *  Tests if this combo box is a drop-down combo box.
 *
 *  @remarks  We combine the 3 different Rexx methods into this one.
 */
RexxMethod2(logical_t, cb_isDropDown, NAME, method, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return FALSE;
    }

    if ( method[2] == 'S' )
    {
        return isSimpleCB(pcdc->hCtrl) ? TRUE : FALSE;
    }
    else if ( method[10] == '\0' )
    {
        return isDropDownListCB(pcdc->hCtrl) ? TRUE : FALSE;
    }

    return isDropDownCB(pcdc->hCtrl) ? TRUE : FALSE;
}

/** ComboBox::isGrandChild()
 *
 *  Notifies the framework that this combo box control is a grandchild of the
 *  dialog and configures the underlying combo box control to send some event
 *  notifications to the dialog, rather than its direcet parent.
 *
 *  @param  mthName  [optional] The method to be invoked in the Rexx dialog when
 *                   one of the 4 default events happens.  The default if
 *                   omitted is onComboBoxGrandChildEvent()
 *
 *  @param  wantTab  [opitonal]  If the Rexx method should be invoked for the
 *                   TAB event.  The default is false, the method is not invoked
 *                   for the tab key event.
 *
 *  @return  True on success, otherwise false.
 *
 *  @notes   Requires common control library 6.2.0.
 *
 *           This method connects 4 event notifications from the grandchild
 *           combo box control to the method in the Rexx dialog.  3 of the
 *           events are key down events, the RETURN, ESCAPE, and TAB key down
 *           events. The other event is the KILLFOCUS event.  All events invoke
 *           the same method in the Rexx dialog.  One of the arguments sent to
 *           the event handler is a keyword that specifies which event happened.
 *
 *           By default, the method is not invoked for the TAB key down event,
 *           but that can be changed using the wantTab argument.
 *
 *  @remarks  We need to always connect the VK_TAB key, even if the user does
 *            not want the TAB nofication.  The reason is that we need to use
 *            DLGC_WANTALLKEYS for WM_GETDLGCODE, which prevents the dialog
 *            manager from handling TAB.  I don't see any way of asking for
 *            RETURN and ESCAPE, but not TAB.  In the message processing loop,
 *            we simply do not invoke the Rexx method unless CTRLTAG_WANTTAB is
 *            set in the tag.
 */
RexxMethod3(RexxObjectPtr, cb_isGrandchild, OPTIONAL_CSTRING, mthName, OPTIONAL_logical_t, wantTab, CSELF, pCSelf)
{
    return genericIsGrandchild(context, mthName, wantTab, pCSelf, winComboBox);
}


/** ComboBox::removeFullColor()
 *
 *  Removes the combo box suclass, if it exists, that provides the
 *  implementation of changing the combo box colors.
 *
 * @return True on success, false otherwise
 *
 * @notes  We can not remove the subclass procedure because it is universal to
 *         the control and may be used for some other messages.  (This is
 *         unlikely, but possible.)  So, what we do is set a flag that disables
 *         the custom coloring.
 */
RexxMethod1(RexxObjectPtr, cb_removeFullColor, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr result = TheFalseObj;

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }
    if ( ! requiredComCtl32Version(context, "removeFullColor", COMCTL32_6_0) )
    {
        goto done_out;
    }

    pSubClassData pscd   = (pSubClassData)pcdc->pscd;
    if ( pscd == NULL )
    {
        goto done_out;
    }
    if ( pscd->pData == NULL )
    {
        goto done_out;
    }

    COLORTABLEENTRY *cte = (COLORTABLEENTRY *)pscd->pData;
    cte->ColorBrush = NULL;
    cte->isSysBrush = false;
    cte->ColorBk    = CLR_DEFAULT;
    cte->ColorFG    = CLR_DEFAULT;
    result          = TheTrueObj;

done_out:
    return result;
}


/** ComboBox::removeItemData()
 *
 */
RexxMethod2(RexxObjectPtr, cb_removeItemData, uint32_t, index, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return TheNilObj;
    }

    index--;
    RexxObjectPtr result = getCurrentComboBoxUserData(pcdc->hCtrl, index);
    if ( result == NULLOBJECT )
    {
        oodSetSysErrCode(context->threadContext, ERROR_SIGNAL_REFUSED);
        return TheNilObj;
    }

    if ( result != TheNilObj )
    {
        LRESULT ret = SendMessage(pcdc->hCtrl, CB_SETITEMDATA, index, (LPARAM)NULL);
        if ( ret != CB_ERR && ret != CB_ERRSPACE )
        {
            unProtectControlUserData(context, pcdc, result);
        }
        else
        {
            // Not removed, set result back to the .nil ojbect.
            result = TheNilObj;
            oodSetSysErrCode(context->threadContext, ERROR_SIGNAL_REFUSED);
        }
    }

    return result;
}

/** ComboBox::select()
 *
 *  Selects the entry in the combo box that that begins with the letters
 *  specified by text.  The search is case insensitive.  I.e., if text is 'new'
 *  it would select "New York".
 *
 *  @param  text  The text (or prefix) of the entry to select.
 *
 *  @return  The one-based index of the entry selected. 0 if no matching entry
 *           was found, or some other error.
 *
 *  @note  The first match found is selected.  For instance with two entries in
 *         the combo box of "San Diego" and "San Jose" and using for text 'san',
 *         the entry with the lowest index would be selected.
 */
RexxMethod2(int32_t, cb_select, CSTRING, text, CSELF, pCSelf)
{
    return cbLbSelect(((pCDialogControl)pCSelf)->hCtrl, text, winComboBox);
}


/** ComboBox::setCue()
 *
 *  Sets the cue, text for the edit control in a combo box.  This text prompts
 *  the user for what to enter in the edit control.
 *
 *  @param  text   The text for the tip.  The length of the text must be 255
 *                 characters or less.
 *
 *  @return  0 on success, 1 on failure.
 *
 *  @notes  Requires Vista  or later. Note the restriction on the length of
 *          text.
 *
 */
RexxMethod2(RexxObjectPtr, cb_setCue, CSTRING, text, CSELF, pCSelf)
{
    return cbEditSetCue(context, text, FALSE, false, pCSelf);
}


/** ComboBox::setFullColor()
 *
 *  Sets the color for this combobox.
 *
 *  Allow system colors but all colors must be the same, all system colors or
 *  all non-system colores
 *
 *  For system colors, the user specifies a keyword, or the system color number,
 *  we then get the COLORREF for that number.
 *
 * @return RexxObjectPtr
 *
 * @notes  If a subclass already exists and a custom color struct is present, we
 *         just change the values and return. If the subclass already exists,
 *         but no custom color struct exists we need to do everything as though
 *         no subclass existed.
 *
 *         Currently the only other subclassing method is isGrandchild().
 *         Although we haven't thought through something like the generic dialog
 *         control subclassings (?)  connectChar()
 */
RexxMethod4(RexxObjectPtr, cb_setFullColor, OPTIONAL_RexxObjectPtr, _bk, OPTIONAL_RexxObjectPtr, _fg,
            OPTIONAL_logical_t, _sysColor, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr    result = TheFalseObj;
    WinMessageFilter wmf    = {0};

    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        goto done_out;
    }
    if ( ! requiredComCtl32Version(context, "setFullColor", COMCTL32_6_0) )
    {
        goto done_out;
    }
    if ( argumentOmitted(1) && argumentOmitted(2) )
    {
        goto done_out;
    }

    bool    useSysColor = _sysColor ? true : false;
    uint32_t bkColor = CLR_DEFAULT;
    uint32_t fgColor = CLR_DEFAULT;

    if ( useSysColor )
    {
        if ( argumentExists(1) && ! getSystemColor(context, _bk, &bkColor, 1) )
        {
            goto done_out;
        }
        if ( argumentExists(2) && ! getSystemColor(context, _fg, &fgColor, 2) )
        {
            goto done_out;
        }

        if ( bkColor != CLR_DEFAULT )
        {
            bkColor = GetSysColor(bkColor);
        }
        if ( fgColor != CLR_DEFAULT )
        {
            fgColor = GetSysColor(fgColor);
        }
    }
    else
    {
        if ( argumentExists(1) && ! context->UnsignedInt32(_bk, &bkColor) )
        {
            goto done_out;
        }
        if ( argumentExists(2) && ! context->UnsignedInt32(_fg, &fgColor) )
        {
            goto done_out;
        }
    }

    COLORTABLEENTRY *cte    = NULL;
    pSubClassData    pscd   = (pSubClassData)pcdc->pscd;
    bool             exists = false;

    if ( pscd != NULL && pscd->pData != NULL )
    {
        // This specific combo box control has already been subclassed and has
        // had custom colors added.  The message filters are already set up, we
        // just need to change the colors.
        cte    = (COLORTABLEENTRY *)pscd->pData;
        exists = true;
    }
    else
    {
        cte = (COLORTABLEENTRY *)LocalAlloc(LPTR, sizeof(COLORTABLEENTRY));
        if ( cte == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
        }
    }

    if ( cte->ColorBrush != NULL && ! cte->isSysBrush )
    {
        DeleteObject(cte->ColorBrush);
    }

    cte->ColorBk = bkColor;
    cte->ColorFG = fgColor;

    if ( useSysColor )
    {
        cte->ColorBrush = GetSysColorBrush(bkColor);
        cte->isSysBrush = true;
    }
    else
    {
        cte->ColorBrush = CreateSolidBrush(bkColor);
        cte->isSysBrush = false;
    }

    if ( exists )
    {
        result = TheTrueObj;
        goto done_out;
    }

    // There is no method invocation needed
    wmf.method = "NOOP";
    wmf.tag    = CTRLTAG_COMBOBOX | CTRLTAG_COLORS;

    // If the subclass was already set, we need to add our private data here,
    // otherwise we can just send it along and the subclassing procedure will
    // add it.
    if ( pscd == NULL )
    {
        wmf.pData = cte;
        wmf.pfn   = freeComboBoxData;
    }
    else
    {
        pscd->pData = cte;
        pscd->pfn   = freeComboBoxData;
    }

    wmf.wm       = WM_CTLCOLORBTN;
    wmf.wmFilter = 0xFFFFFFFF;
    wmf.wp       = 0;
    wmf.wpFilter = 0;
    wmf.lp       = 0;
    wmf.lpFilter = 0;

    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm       = WM_CTLCOLOREDIT;
    wmf.wmFilter = 0xFFFFFFFF;
    wmf.wp       = 0;
    wmf.wpFilter = 0;
    wmf.lp       = 0;
    wmf.lpFilter = 0;

    wmf.pData = NULL;
    wmf.pfn   = NULL;

    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    wmf.wm       = WM_CTLCOLORLISTBOX;
    wmf.wmFilter = 0xFFFFFFFF;
    wmf.wp       = 0;
    wmf.wpFilter = 0;
    wmf.lp       = 0;
    wmf.lpFilter = 0;

    if ( ! addSubclassMessage(context, pcdc, &wmf) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_NOT_ENOUGH_MEMORY);
        goto done_out;
    }

    result = TheTrueObj;

done_out:
    return result;
}


/** ComboBox::setItemData()
 *
 *  Assigns a user data value to the specified combo box item.
 *
 *  @param  index  [required]  The one-based index of the item whose user data
 *                 is to be set.
 *
 *  @param  data   [optional]  The user data to be set. If this argument is
 *                 omitted, the current user data, if any, is removed.
 *
 *  @return  Returns the previous user data object for the specified combo box
 *           item, if there was a user data object, or .nil if there wasn't.
 *
 *           On error, .nil is returned.  An error is very unlikely.  An error
 *           can be checked for by examining the .systemErrorCode object.
 *
 *  @notes  Sets the .systemErrorCode.  On error set to:
 *
 *          156  ERROR_SIGNAL_REFUSED The recipient process has refused the
 *          signal.
 *
 *          This is not a system error, the code is just used here to indicate a
 *          combo box error when setting the user data.  The combo box provides
 *          no information on why it failed.
 */
RexxMethod3(RexxObjectPtr, cb_setItemData, uint32_t, index, OPTIONAL_RexxObjectPtr, data, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return TheNilObj;
    }

    index--;
    RexxObjectPtr oldUserData = getCurrentComboBoxUserData(pcdc->hCtrl, index);

    if ( oldUserData == NULLOBJECT )
    {
        oodSetSysErrCode(context->threadContext, ERROR_SIGNAL_REFUSED);
        return TheNilObj;
    }

    if ( argumentExists(2) )
    {
        LRESULT ret = SendMessage(pcdc->hCtrl, CB_SETITEMDATA, index, (LPARAM)data);
        if ( ret != CB_ERR && ret != CB_ERRSPACE )
        {
            unProtectControlUserData(context, pcdc, oldUserData);
            protectControlUserData(context, pcdc, data);
        }
        else
        {
            oldUserData = TheNilObj;
            oodSetSysErrCode(context->threadContext, ERROR_SIGNAL_REFUSED);
        }
    }
    else
    {
        LRESULT ret = SendMessage(pcdc->hCtrl, CB_SETITEMDATA, index, (LPARAM)NULL);
        if ( ret != CB_ERR && ret != CB_ERRSPACE )
        {
            unProtectControlUserData(context, pcdc, oldUserData);
        }
        else
        {
            oldUserData = TheNilObj;
            oodSetSysErrCode(context->threadContext, ERROR_SIGNAL_REFUSED);
        }
    }

    return oldUserData;
}


/** ComboBox::setItemHeight()
 *
 *  Sets the height of the list items or the selection field in a combo box.
 *
 *  @param  height             [required] The height in pixels to set the
 *                             specified component.
 *
 *  @param  setSelectionField  [optional]  If true get the selection field
 *                             height, if false get the item height.  The
 *                             default is false, get the item height.
 *
 *  @return  If the height is wrong returns -1.
 *
 *  @note  The selection field height in a combo box is set independently of the
 *         height of the list items. The programmer must ensure that the height
 *         of the selection field is not smaller than the height of a particular
 *         list item.
 */
RexxMethod3(int32_t, cb_setItemHeight, uint32_t, pixels, OPTIONAL_logical_t, setSelectionField, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }

    WPARAM which = setSelectionField ? -1 : 0;
    return (int32_t)SendMessage(pcdc->hCtrl, CB_SETITEMHEIGHT, which, pixels);
}


/** ComboBox::setMinVisible()
 *
 *  Sets the minimum number of visible items in the drop-down list of a combo
 *  box.
 *
 *  @param count  The minimum number of visible items.
 *
 *  @return True on success, otherwise false.
 *
 *  @notes  Requires Common Control library 6.0
 *
 *          When the number of items in the drop-down list is greater than the
 *          minimum, the combo box uses a scrollbar. By default, 30 is the
 *          minimum number of visible items.
 */
RexxMethod2(logical_t, cb_setMinVisible, int32_t, count, CSELF, pCSelf)
{
    pCDialogControl pcdc = validateDCCSelf(context, pCSelf);
    if ( pcdc == NULL )
    {
        return 0;
    }
    if ( ! requiredComCtl32Version(context, "setMinVisible", COMCTL32_6_0)  )
    {
        return 0;
    }

    return ComboBox_SetMinVisible(pcdc->hCtrl, count);
}
