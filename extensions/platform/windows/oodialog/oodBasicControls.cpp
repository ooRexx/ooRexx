/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2010 Rexx Language Association. All rights reserved.    */
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
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodResources.hpp"


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
 *  Methods for the ooDialog class: .ButtonControl and its subclasses
 *  .RadioButton and .CheckBox.
 */
#define BUTTON_CLASS                 "ButtonControl"

#define BUTTONIMAGELIST_ATTRIBUTE    "!BUTTONIMAGELIST"
#define BUTTONIMAGE_ATTRIBUTE        "!BUTTONIMAGE"

#define BUTTONCONTROLCLASS   ".ButtonControl"
#define RADIOBUTTONCLASS     ".RadioButton"
#define CHECKBOXCLASS        ".CheckBox"
#define GROUPBOXCLASS        ".GroupBox"
#define ANIMATEDBUTTONCLASS  ".AnimatedButton"

#define BC_SETSTYLE_OPTS     "PUSHBOX, DEFPUSHBUTTON, CHECKBOX, AUTOCHECKBOX, 3STATE, AUTO3STATE, "        \
                             "RADIO, AUTORADIO, GROUPBOX, OWNERDRAW, LEFTTEXT, RIGHTBUTTON, NOTLEFTTEXT, " \
                             "TEXT, ICON, BITMAP, LEFT, RIGHT, HCENTER, TOP, BOTTOM, VCENTER, PUSHLIKE, "  \
                             "NOTPUSHLIKE, MULTILINE, NOTMULTILINE, NOTIFY, NOTNOTIFY, FLAT, NOTFLAT"

#define BC_SETSTATE_OPTS     "CHECKED, UNCHECKED, INDETERMINATE, FOCUS, PUSH, NOTPUSHED"
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
 * Gets the button image list information for this button, which includes the
 * image list itself, the image alignment, and the margin around the image.
 *
 * @param c     The method context we are executing in.
 * @param hwnd  The ButtonControl window handle.
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
 * the right or left alignment of the text.  Other changes either have no
 * effect, or cause the group box / dialog to paint in a weird way.
 */
RexxMethod2(int, gb_setStyle, CSTRING, opts, CSELF, pCSelf)
{
    HWND hwnd = ((pCDialogControl)pCSelf)->hCtrl;
    HWND hDlg = ((pCDialogControl)pCSelf)->hDlg;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);

    if ( stricmp(opts, "RIGHT") == 0 )
    {
        style = (style & ~BS_CENTER) | BS_RIGHT;
    }
    else if ( stricmp(opts, "LEFT") == 0 )
    {
        style = (style & ~BS_CENTER) | BS_LEFT;
    }
    else
    {
        wrongArgValueException(context->threadContext, 1, "RIGHT, LEFT", opts);
        return 0;
    }

    /**
     * When the alignment changes, we need to force the dialog to redraw the
     * area occupied by the group box.  Otherwise the old text remains on the
     * screen.  But, it is only the top part of the group box that needs to be
     * redrawn, so we only invalidate the top half of the group box.
     */
    RECT r;

    // Get the screen area of the group box and map it to the client area of the
    // dialog.
    GetWindowRect(hwnd, &r);
    MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);

    LONG halfHeight = ((r.bottom - r.top) / 2);
    r.bottom = (halfHeight >= MIN_HALFHEIGHT_GB ? r.top + halfHeight : r.bottom);

    // Change the group box style, force the dialog to repaint.
    SetWindowLong(hwnd, GWL_STYLE, style);
    SendMessage(hwnd, BM_SETSTYLE, (WPARAM)style, (LPARAM)TRUE);

    InvalidateRect(hDlg, &r, TRUE);
    UpdateWindow(hDlg);

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

    RECT r;
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

    PRECT pRect = rxGetRect(context, r, 1);
    if ( pRect != NULL )
    {
        if ( Button_SetTextMargin(hwnd, pRect) )
        {
            return 1;
        }
    }
    return 0;
}

RexxMethod1(RexxObjectPtr, bc_getIdealSize, CSELF, pCSelf)
{
    if ( ! requiredComCtl32Version(context, "getIdealSize", COMCTL32_6_0) )
    {
        return NULLOBJECT;
    }

    HWND hwnd = getDChCtrl(pCSelf);
    RexxObjectPtr result = NULLOBJECT;

    SIZE size;
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

/** ButtonControl::setImage()
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

/** ButtonControl::getImageList()
 *
 * Gets the image list for the button, if there is one.
 *
 * @return  .nil if this the button control does not have an image list.
 *          Otherwise, a .Directory object with the following indexes.  The
 *          indexes contain the image list related information:
 *
 *          d~imageList -> The .ImageList object set by setImageList().
 *          d~rect      -> A .Rect object containing the margins.
 *          d~alignment -> The image alignment in the button.
 *
 * @requires  Common Controls version 6.0 or later.
 *
 * @exception  A syntax error is raised for wrong comctl version.
 *
 * @note  The only way to have an image list is for it have been put there by
 *        setImageList().  That method stores the .ImageList object as an
 *        attribute of the ButtonControl ojbect.  That stored object is the
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

/** ButtonControl::setImageList()
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
            OPTIONAL_uint8_t, align, CSELF, pCSelf)
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
        PRECT pRect = rxGetRect(context, margin, 2);
        if ( pRect == NULL )
        {
            goto err_out;
        }
        biml.margin.top = pRect->top;
        biml.margin.left = pRect->left;
        biml.margin.right = pRect->right;
        biml.margin.bottom = pRect->bottom;
    }

    if ( argumentExists(3) )
    {
        if ( align > BUTTON_IMAGELIST_ALIGN_CENTER )
        {
            wrongRangeException(context->threadContext, 3, BUTTON_IMAGELIST_ALIGN_LEFT,
                                BUTTON_IMAGELIST_ALIGN_CENTER, align);
            goto err_out;
        }
        biml.uAlign =  align;
    }
    else
    {
        biml.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
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

/* TODO convert to using optional .Rect arg */

/** ButtonControl::scroll()
 *
 *  Moves the specified rectangle within the button and redraws the uncovered
 *  area with the button background color.  This method is used to move bitmaps
 *  within bitmap buttons.
 *
 *  @note  Sets .SystemErrorCode.
 *
 *  @remarks  TODO convert to using an options .Rect arg.
 *
 *            The original ooDialog external function had an option whether or
 *            not to redraw the uncovered portion of the button.  The option was
 *            not documented and internally the function was always called with
 *            true.  That option was therefore eliminated
 */
RexxMethod7(logical_t, bc_scroll, int32_t, xPos, int32_t, yPos, int32_t, left, int32_t, top, int32_t, right, int32_t, bottom, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCDialogControl pcdc = (pCDialogControl)pCSelf;
    pCPlainBaseDialog pcpbd = dlgToCSelf(context, pcdc->oDlg);

    HWND hwnd = pcdc->hCtrl;
    RECT r;
    if ( GetWindowRect(hwnd, &r) )
    {
        RECT rs;
        HDC hDC = GetDC(hwnd);

        rs.left = left;
        rs.top = top;
        rs.right = right;
        rs.bottom = bottom;

        r.right = r.right - r.left;
        r.bottom = r.bottom - r.top;
        r.left = 0;
        r.top = 0;

        if ( ScrollDC(hDC, xPos, yPos, &rs, &r, NULL, NULL) == 0 )
        {
            oodSetSysErrCode(context->threadContext);
            goto err_out;
        }

        // Draw uncovered rectangle with background color.
        HBRUSH hBrush, hOldBrush;
        HPEN hOldPen, hPen;

        if ( pcpbd->bkgBrush )
        {
            hBrush = pcpbd->bkgBrush;
        }
        else
        {
            hBrush = GetSysColorBrush(COLOR_BTNFACE);
        }

        hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
        hOldPen = (HPEN)SelectObject(hDC, hPen);
        hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

        if ( xPos > 0 )
        {
            Rectangle(hDC, rs.left, rs.top, rs.left + xPos, rs.bottom);
        }
        else if ( xPos < 0 )
        {
            Rectangle(hDC, rs.right + xPos, rs.top, rs.right, rs.bottom);
        }

        if ( yPos > 0 )
        {
            Rectangle(hDC, rs.left, rs.top, rs.right, rs.top + yPos);
        }
        else if ( yPos < 0 )
        {
            Rectangle(hDC, rs.left, rs.bottom + yPos, rs.right, rs.bottom);
        }

        SelectObject(hDC, hOldBrush);
        SelectObject(hDC, hOldPen);
        DeleteObject(hPen);

        ReleaseDC(hwnd, hDC);
        return 0;
    }

err_out:
    return 1;
}


/** ButtonControl::dimBitmap()
 *
 *
 */
RexxMethod7(RexxObjectPtr, bc_dimBitmap, POINTERSTRING, hBmp, uint32_t, width, uint32_t, height,
            OPTIONAL_uint32_t, stepX, OPTIONAL_uint32_t, stepY, OPTIONAL_uint32_t, steps, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);

    stepX = (argumentOmitted(4) ? 2  : stepX);
    stepY = (argumentOmitted(5) ? 2  : stepY);
    steps = (argumentOmitted(6) ? 10 : steps);

    HDC hDC = GetWindowDC(hwnd);

    LOGBRUSH logicalBrush;
    logicalBrush.lbStyle = BS_DIBPATTERNPT;
    logicalBrush.lbColor = DIB_RGB_COLORS;
    logicalBrush.lbHatch = (ULONG_PTR)hBmp;

    HBRUSH hBrush = CreateBrushIndirect(&logicalBrush);
    HPEN hPen = CreatePen(PS_NULL, 0, PALETTEINDEX(0));

    HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
    HPEN oldPen = (HPEN)SelectObject(hDC, hPen);

    uint32_t diffY = steps * stepY;
    uint32_t diffX = steps * stepX;

    uint32_t a, i, j, x, y;

    for ( a = 0; a < steps; a++ )
    {
       for ( y = a * stepY, i = 0; i < height / steps; y += diffY, i++ )
       {
          for ( x = a * stepX, j = 0; j < width / steps; x += diffX, j++ )
          {
              Rectangle(hDC, x - a * stepX, y - a * stepY, x + stepX + 1, y + stepY + 1);
          }
       }
    }

    SelectObject(hDC, oldBrush);
    SelectObject(hDC, oldPen);
    DeleteObject(oldBrush);
    DeleteObject(oldPen);
    ReleaseDC(hwnd, hDC);

    return TheZeroObj;
}


RexxMethod4(int, rb_checkInGroup_cls, RexxObjectPtr, dlg, RexxObjectPtr, idFirst,
            RexxObjectPtr, idLast, RexxObjectPtr, idCheck)
{
    int result = 0;
    if ( requiredClass(context->threadContext, dlg, "PlainBaseDialog", 1) )
    {
        HWND hwnd = dlgToHDlg(context, dlg);

        int first = oodResolveSymbolicID(context, dlg, idFirst, -1, 2);
        int last = oodResolveSymbolicID(context, dlg, idLast, -1, 3);
        int check = oodResolveSymbolicID(context, dlg, idCheck, -1, 4);

        if ( first != OOD_ID_EXCEPTION && last != OOD_ID_EXCEPTION && check != OOD_ID_EXCEPTION )
        {
            if ( CheckRadioButton(hwnd, first, last, check) == 0 )
            {
                result = (int)GetLastError();
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


/** ButtonControl::test()
 *
 *  This method is used as a convenient way to test code.
 */
RexxMethod1(int, bc_test, ARGLIST, args)
{
    return 0;
}

/** ButtonControl::test()  [class method]
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
 * Subclass procedure for an edit control that intercepts the WM_CONTEXTMENU
 * message and passes it on to the owner dialog rather than the edit control.
 */
LRESULT CALLBACK NoEditContextMenu(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR dwData)
{
    SUBCLASSDATA *pData = (SUBCLASSDATA *)dwData;

    switch ( msg )
    {
        case WM_CONTEXTMENU:
            if ( pData )
            {
                return SendMessage((HWND)pData->pData, msg, wParam, lParam);
            }
            break;

        case WM_NCDESTROY:
            /* The window is being destroyed, remove the subclass, clean up
             * memory.
             */
            RemoveWindowSubclass(hwnd, NoEditContextMenu, id);
            if ( pData )
            {
                LocalFree(pData);
            }
            break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

/** Edit::noContextMenu()
 *
 * Suppresses, or removes the suppression of, the default edit control's context
 * menu.
 *
 * The primary purpose of this would be to allow the Rexx programmer to put up a
 * substitute context menu using the PopupMenu class.  If the programer does not
 * connect a context menu to the edit control, then no context menu is
 * displayed.
 *
 * If the Rexx programmer previously suppressed the context menu, then that
 * suppression can be removed.
 *
 * @arg  undo  [OPTIONAL] If true, the suppression of the context menu is
 *             removed.
 *
 * @return  True on success, false on error.
 *
 * @notes  Sets the .SystemErrorCode.  These codes may be set, by us, the OS
 *         does not set any:
 *
 * ERROR_NOT_SUPPORTED   The request is not supported.
 * ERROR_SIGNAL_REFUSED  The recipient process has refused the signal.
 *
 */
RexxMethod2(RexxObjectPtr, e_noContextMenu, OPTIONAL_logical_t, undo, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr result = TheFalseObj;
    if ( ! requiredComCtl32Version(context, "noContextMenu", COMCTL32_6_0) )
    {
        goto done_out;
    }

    pCDialogControl pcdc = (pCDialogControl)pCSelf;

    SUBCLASSDATA *pData = NULL;
    BOOL success = GetWindowSubclass(pcdc->hCtrl, NoEditContextMenu, pcdc->id, (DWORD_PTR *)&pData);

    if ( undo )
    {
        if ( pData == NULL )
        {
            // The subclass is not installed, we call this an error.
            oodSetSysErrCode(context->threadContext, ERROR_NOT_SUPPORTED);
            goto done_out;
        }

        if ( SendMessage(pcdc->hDlg, WM_USER_SUBCLASS_REMOVE, (WPARAM)NoEditContextMenu, (LPARAM)pcdc->id) == 0 )
        {
            // The subclass is not removed, we can't free pData because the
            // subclass procedure may (will) still access it.
            oodSetSysErrCode(context->threadContext, ERROR_SIGNAL_REFUSED);
        }
        else
        {
            LocalFree(pData);
            result = TheTrueObj;
        }
        goto done_out;
    }

    if ( pData != NULL )
    {
        // The subclass is already installed, we call this an error.
        oodSetSysErrCode(context->threadContext, ERROR_NOT_SUPPORTED);
        goto done_out;
    }

    pData = (SUBCLASSDATA *)LocalAlloc(LPTR, sizeof(SUBCLASSDATA));
    if ( pData == NULL )
    {
        outOfMemoryException(context->threadContext);
        goto done_out;
    }

    pData->hCtrl = pcdc->hCtrl;
    pData->uID = pcdc->id;
    pData->pData = (void *)pcdc->hDlg;

    if ( SendMessage(pcdc->hDlg, WM_USER_SUBCLASS, (WPARAM)NoEditContextMenu, (LPARAM)pData) == 0 )
    {
        // The subclass was not installed, free memeory, set error code.
        LocalFree(pData);
        oodSetSysErrCode(context->threadContext, ERROR_SIGNAL_REFUSED);
        goto done_out;
    }

    result = TheTrueObj;

done_out:
    return result;
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
 *  multiline edit control. A character index is the zero-based index of the
 *  character from the beginning of the edit control.
 *
 *  @param  lineNumber  The one-based index of the line whose character index is
 *                      desired.  A value of –1 specifies the current line
 *                      number (the line that contains the caret).
 *
 *  @return The character index. -1 is returned if the specified line index is
 *          not within bounds of the edit control lines.  (More than the curent
 *          number of lines or 0.)
 *
 *  @note  The lineIndex() method is intended for multi-line edit controls,
 *         however, it will behave as documented for single-line edit controls.
 *         The return is always 1 for a single-line if the lineNumber argument
 *         is 1 or -1 and always -1 for any other value.
 *
 *  @remarks  The EM_LINEINDEX message is documented as returning -1 if the line
 *            number specified as being greater than the current number of lines
 *            in the edit control.  However, under 64-bit Windows, the return is
 *            0x00000000FFFFFFFF (4294967295) rather than -1.
 */
RexxMethod2(RexxObjectPtr, e_lineIndex, int32_t, lineNumber, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNegativeOneObj;
    if ( lineNumber != 0 )
    {
        HWND hCtrl = getDChCtrl(pCSelf);

        if ( isSingleLineEdit(hCtrl) )
        {
            if ( lineNumber == 1 || lineNumber == -1 )
            {
                result = TheOneObj;
            }
        }
        else
        {
            if ( lineNumber != -1 )
            {
                lineNumber--;
            }

            uint32_t charIndex = (uint32_t)SendMessage(hCtrl, EM_LINEINDEX, lineNumber, 0);
            if ( charIndex != 0xFFFFFFFF )
            {
                result = context->UnsignedInt32(++charIndex);
            }
        }
    }
    return result;
}


/** Edit::getLine()
 *
 *  Retrieves the text of the specified line.
 *
 *  @param  lineNumber  The one-base index of the line whose text is desired.
 *                      A value of –1 specifies the current line number (the
 *                      line that contains the caret).
 *  @param  ignored     Prior to 4.0.1, ooDialog required the user to specify
 *                      how long the line was (or how much text to retrieve) if
 *                      the line was over 255 characters.  This restriction is
 *                      removed and this argument is simply ignored to provide
 *                      backward compatibility.
 *
 *  @return  The text of the specified line, or the empty string if an error
 *           occurs.  Recall that it is possible that the line specified
 *           actually contains no text.
 */
RexxMethod3(RexxStringObject, e_getLine, int32_t, lineNumber, OPTIONAL_RexxObjectPtr, ignored, CSELF, pCSelf)
{
    HWND hwnd = getDChCtrl(pCSelf);
    char *buf = NULL;
    RexxStringObject result = context->NullString();

    if ( lineNumber == 0 )
    {
        goto done_out;
    }

    if ( isSingleLineEdit(hwnd) )
    {
        if ( lineNumber != 1 )
        {
            goto done_out;
        }
        rxGetWindowText(context, hwnd, &result);
    }
    else
    {
        if ( lineNumber != -1 )
        {
            lineNumber--;
        }

        uint32_t charIndex = (uint32_t)SendMessage(hwnd, EM_LINEINDEX, lineNumber, 0);
        if ( charIndex == 0xFFFFFFFF )
        {
            goto done_out;
        }

        WORD count = (WORD)SendMessage(hwnd, EM_LINELENGTH, charIndex, 0);
        if ( count == 0 )
        {
            goto done_out;
        }

        buf = (char *)LocalAlloc(LPTR, ++count);
        if ( buf == NULL )
        {
            outOfMemoryException(context->threadContext);
            goto done_out;
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
        stringTooLongException(context->threadContext, 2, BALLON_MAX_TEXT, strlen(title));
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

/* Note that the EM_GETCUEBANNER simply does not work.  At least on XP.  So
 * the code is removed.  But, it might be worth trying on Vista.
 */


RexxMethod2(RexxObjectPtr, e_setCue, CSTRING, text, CSELF, pCSelf)
{
    if ( ! requiredComCtl32Version(context, context->GetMessageName(), COMCTL32_6_0)  )
    {
        return TheOneObj;
    }

    // The text is limited to 255.
    WCHAR wszCue[QUE_MAX_TEXT + 1];
    if ( strlen(text) > QUE_MAX_TEXT )
    {
        stringTooLongException(context->threadContext, 1, QUE_MAX_TEXT, strlen(text));
        return TheOneObj;
    }

    putUnicodeText((LPWORD)wszCue, text);
    return (Edit_SetCueBannerText(getDChCtrl(pCSelf), wszCue) ? TheZeroObj : TheOneObj);
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
    if ( exactly != NULL && (*exactly == '1' || toupper(*exactly) || 'E') )
    {
        exact = true;
    }
    if ( startIndex > 0 )
    {
        startIndex--;
    }

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

    found = (int32_t)SendMessage(hCtrl, LB_FINDSTRING, startIndex, (LPARAM)text);

    return (found > 0 ? 0 : --found);
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


RexxMethod1(RexxObjectPtr, lb_isSingleSelection, CSELF, pCSelf)
{
    return (isSingleSelectionListBox(getDChCtrl(pCSelf)) ? TheTrueObj : TheFalseObj);
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


RexxMethod3(int32_t, lb_addDirectory, CSTRING, drivePath, OPTIONAL_CSTRING, fileAttributes, CSELF, pCSelf)
{
    return cbLbAddDirectory(((pCDialogControl)pCSelf)->hCtrl, drivePath, fileAttributes, winListBox);
}

/**
 * Methods for the ComboBox class.
 */
#define COMBOBOX_CLASS   "ComboBox"


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


/** ComboBox::insert()
 *
 *  Inserts a string entry into the cobo box at the index specified.
 *
 *  @param  index  [OPTIONAL]  The one-based index of where the entry is to be
 *                 inerted.  If index is less than -1, the entry is inserted at
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

RexxMethod3(int32_t, cb_addDirectory, CSTRING, drivePath, OPTIONAL_CSTRING, fileAttributes, CSELF, pCSelf)
{
    return cbLbAddDirectory(((pCDialogControl)pCSelf)->hCtrl, drivePath, fileAttributes, winComboBox);
}
