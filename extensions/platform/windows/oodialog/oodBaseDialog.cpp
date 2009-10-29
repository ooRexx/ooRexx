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
 * oodBaseDialog.cpp
 *
 * The base classes for all non-trival dialogs in the ooDialog package.
 * Contains the method implmentations for the BaseDialog, ResDialog, and
 * WindowExtensions classes.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <shlwapi.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodData.hpp"
#include "oodText.hpp"


class LoopThreadArgs
{
public:
    RexxMethodContext *context;  // Used for data autodetection only.
    DIALOGADMIN *dlgAdmin;
    uint32_t resourceId;
    bool autoDetect;
    bool *release;               // Used for a return value
};

/**
 *  Methods for the .BaseDialog class.
 */
#define BASEDIALOG_CLASS              "BaseDialog"

/** BaseDialog::init()
 */
RexxMethod3(RexxObjectPtr, baseDlg_init, ARGLIST, args, SUPER, super, OSELF, self)
{
    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, NULL);

    if ( isInt(0, result, context) )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();
        if ( ! initWindowExtensions(context, self, NULL, pcpbd->wndBase, pcpbd) )
        {
            return TheOneObj;
        }

        context->SendMessage1(self, "SCROLLNOW=", TheZeroObj);
        context->SendMessage1(self, "BKGBITMAP=", TheZeroObj);
        context->SendMessage1(self, "BKGBRUSHBMP=", TheZeroObj);
        context->SendMessage1(self, "MENUBAR=", context->Nil());
        context->SendMessage1(self, "ISLINKED=", TheFalseObj);
    }

    return result;
}


RexxMethod1(RexxObjectPtr, baseDlg_test, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    DIALOGADMIN *dlgAdm = pcpbd->dlgAdm;

    MESSAGETABLEENTRY *m = dlgAdm->MsgTab;

    for ( int i = 0; i < dlgAdm->MT_size; i++ )
    {
        printf("method: %s msg=0x%08x msgF=0x%08x\n", m[i].rexxProgram, m[i].msg, m[i].filterM);
        printf("wp=0x%016I64x wpF=0x%016I64x lp=0x%016I64x lpF=0x%016I64x\n",
               m[i].wParam, m[i].filterP, m[i].lParam, m[i].filterL);

    }

    return TheTrueObj;
}


/**
 *  Methods for the .ResDialog class.
 */
#define RESDIALOG_CLASS        "ResDialog"


/**
 *  The thread function for the windows message processing loop used by a
 *  ResDialog dialog.  This function creates the dialog and, on success, enters
 *  the message loop.
 *
 * @param arg  Structure sent by the caller containing arguments used to create
 *             the dialog and to notify the caller of the result of creating the
 *             dialog
 *
 * @return The exit code for the thread.
 */
DWORD WINAPI WindowLoopThread(void *arg)
{
    MSG msg;
    DIALOGADMIN * dlgAdm;
    bool * release;
    ULONG ret;

    LoopThreadArgs *args = (LoopThreadArgs *)arg;

    dlgAdm = args->dlgAdmin;
    dlgAdm->TheDlg = CreateDialogParam(dlgAdm->TheInstance, MAKEINTRESOURCE(args->resourceId), 0, (DLGPROC)RexxDlgProc, dlgAdm->Use3DControls);  /* pass 3D flag to WM_INITDIALOG */
    dlgAdm->ChildDlg[0] = dlgAdm->TheDlg;

    release = args->release;
    if ( dlgAdm->TheDlg )
    {
        if ( args->autoDetect )
        {
            if ( ! doDataAutoDetection(args->context, dlgAdm) )
            {
                dlgAdm->TheThread = NULL;
                return 0;
            }
        }

        *release = true;  /* Release wait in startDialog()  */
        do
        {
            if ( GetMessage(&msg,NULL, 0,0) )
            {
                if ( ! IsDialogMessage(dlgAdm->TheDlg, &msg) )
                {
                    DispatchMessage(&msg);
                }
            }
        } while ( dlgAdm && dialogInAdminTable(dlgAdm) && ! dlgAdm->LeaveDialog );
    }
    else
    {
        *release = true;
    }
    EnterCriticalSection(&crit_sec);
    if ( dialogInAdminTable(dlgAdm) )
    {
        ret = DelDialog(dlgAdm);
        dlgAdm->TheThread = NULL;
    }
    LeaveCriticalSection(&crit_sec);
    return ret;
}


/**
 *  Used to set the fontName and fontSize attributes of the resource dialog.
 */
void setFontAttrib(RexxMethodContext *c, pCPlainBaseDialog pcpbd)
{
    HFONT font = (HFONT)SendMessage(pcpbd->hDlg, WM_GETFONT, 0, 0);
    if ( font == NULL )
    {
        font = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    HDC hdc = GetDC(pcpbd->hDlg);
    if ( hdc )
    {
        HFONT oldFont = (HFONT)SelectObject(hdc, font);

        char fontName[64];
        TEXTMETRIC tm;

        GetTextMetrics(hdc, &tm);
        GetTextFace(hdc, sizeof(fontName), fontName);

        long fontSize = MulDiv((tm.tmHeight - tm.tmInternalLeading), 72, GetDeviceCaps(hdc, LOGPIXELSY));

        strcpy(pcpbd->fontName, fontName);
        pcpbd->fontSize = fontSize;

        SelectObject(hdc, oldFont);
        ReleaseDC(pcpbd->hDlg, hdc);
    }
    return;
}


/**
 * Creates the underlying Windows dialog using a dialog resource stored in a
 * DLL.  Currently this is only used for ResDialog dialogs.  All other ooDialog
 * dialogs use DynamicDialog::startParentDialog() to create the underlying
 * Windows dialog.
 *
 * @param libray      The name of the DLL.
 * @param dlgID       The resource ID for the dialog in the DLL
 * @param autoDetect  True if auto detect is on, otherwise false.
 * @param iconID      Ther resource ID to use for the application icon.
 * @param modeless    Whether to create a modeless or a modal dialog.
 *
 * @return True on succes, otherwise false.
 */
RexxMethod6(logical_t, resdlg_startDialog_pvt, CSTRING, library, uint32_t, dlgID, logical_t, autoDetect, uint32_t, iconID,
            logical_t, modeless, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    DIALOGADMIN *dlgAdm = pcpbd->dlgAdm;

    ULONG thID;
    bool Release = false;

    EnterCriticalSection(&crit_sec);
    if ( ! InstallNecessaryStuff(dlgAdm, library) )
    {
        if ( dlgAdm )
        {
            // TODO why is DelDialog() used here, but not below ??
            DelDialog(dlgAdm);

            // Note: The message queue and dialog administration block are /
            // must be freed from PlainBaseDialog::deInstall() or
            // PlainBaseDialog::unInit().
        }
        LeaveCriticalSection(&crit_sec);
        return FALSE;
    }

    LoopThreadArgs threadArgs;
    threadArgs.context = context;
    threadArgs.dlgAdmin = dlgAdm;
    threadArgs.resourceId = dlgID;
    threadArgs.autoDetect = autoDetect ? true : false;
    threadArgs.release = &Release;

    dlgAdm->TheThread = CreateThread(NULL, 2000, WindowLoopThread, &threadArgs, 0, &thID);

    // Wait for dialog start.
    while ( ! Release && (dlgAdm->TheThread) )
    {
        Sleep(1);
    }
    LeaveCriticalSection(&crit_sec);

    if (dlgAdm)
    {
        if (dlgAdm->TheDlg)
        {
            HICON hBig = NULL;
            HICON hSmall = NULL;

            // Set the thread priority higher for faster drawing.
            SetThreadPriority(dlgAdm->TheThread, THREAD_PRIORITY_ABOVE_NORMAL);
            dlgAdm->OnTheTop = TRUE;
            dlgAdm->threadID = thID;

            // Is this to be a modal dialog?
            if ( dlgAdm->previous && ! modeless && IsWindowEnabled(((DIALOGADMIN *)dlgAdm->previous)->TheDlg) )
            {
                EnableWindow(((DIALOGADMIN *)dlgAdm->previous)->TheDlg, FALSE);
            }

            if ( GetDialogIcons(dlgAdm, iconID, ICON_DLL, (PHANDLE)&hBig, (PHANDLE)&hSmall) )
            {
                dlgAdm->SysMenuIcon = (HICON)setClassPtr(dlgAdm->TheDlg, GCLP_HICON, (LONG_PTR)hBig);
                dlgAdm->TitleBarIcon = (HICON)setClassPtr(dlgAdm->TheDlg, GCLP_HICONSM, (LONG_PTR)hSmall);
                dlgAdm->DidChangeIcon = TRUE;

                SendMessage(dlgAdm->TheDlg, WM_SETICON, ICON_SMALL, (LPARAM)hSmall);
            }

                RexxMethodContext *c = context;

            setDlgHandle(context, pcpbd, dlgAdm->TheDlg);
            setFontAttrib(context, pcpbd);
            return TRUE;
        }

        // The dialog creation failed, so clean up.  For now, with the
        // mixture of old and new native APIs, the freeing of the dialog
        // administration block must be done in the deInstall() or
        // unInit() methods.

        // TODO this seems very wrong.  Why isn't a DelDialog() done here???
        dlgAdm->OnTheTop = FALSE;
        if (dlgAdm->previous)
        {
            ((DIALOGADMIN *)(dlgAdm->previous))->OnTheTop = TRUE;
        }
        if ((dlgAdm->previous) && !IsWindowEnabled(((DIALOGADMIN *)dlgAdm->previous)->TheDlg))
        {
            EnableWindow(((DIALOGADMIN *)dlgAdm->previous)->TheDlg, TRUE);
        }
    }
    return FALSE;
}


RexxMethod2(RexxArrayObject, resdlg_getDataTableIDs_pvt, CSELF, pCSelf, OSELF, self)
{
    return getDataTableIDs(context, (pCPlainBaseDialog)pCSelf, self);
}


/**
 *  Methods for the .WindowExtensions class.
 */
#define WINDOWEXTENSIONS_CLASS        "WindowExtensions"


static inline HWND getWEWindow(void *pCSelf)
{
    return ((pCWindowExtensions)pCSelf)->hwnd;
}

static HWND winExtSetup(RexxMethodContext *c, void *pCSelf)
{
    oodResetSysErrCode(c->threadContext);
    HWND hwnd = getWEWindow(pCSelf);

    if ( hwnd == NULL )
    {
        noWindowsDialogException(c, ((pCWindowExtensions)pCSelf)->rexxSelf);
    }
    return hwnd;
}


static int scrollBarType(HWND hwnd, CSTRING method)
{
    int type;
    if ( isControlMatch(hwnd, winScrollBar) )
    {
        type = SB_CTL;
    }
    else if ( *method == 'S' )
    {
        type = (method[3] == 'H' ? SB_HORZ : SB_VERT);
    }
    else
    {
        type = (*method == 'H' ? SB_HORZ : SB_VERT);
    }
    return type;
}


bool initWindowExtensions(RexxMethodContext *c, RexxObjectPtr self, HWND hwnd, pCWindowBase pcwb, pCPlainBaseDialog pcpbd)
{
    RexxBufferObject obj = c->NewBuffer(sizeof(CWindowExtensions));
    if ( obj == NULLOBJECT )
    {
        return false;
    }

    pCWindowExtensions pcwe = (pCWindowExtensions)c->BufferData(obj);
    pcwe->hwnd = hwnd;
    pcwe->rexxSelf = self;
    pcwe->wndBase = pcwb;

    if ( pcpbd != NULL )
    {
        pcpbd->weCSelf = pcwe;
    }
    c->SendMessage1(self, "INITWINDOWEXTENSIONS", obj);

    return true;
}


/** WindowExtensions::initWindowExtensions()
 *
 */
RexxMethod1(logical_t, winex_initWindowExtensions, RexxObjectPtr, cSelf)
{
    RexxMethodContext *c = context;
    if ( ! context->IsBuffer(cSelf) )
    {
        wrongClassException(context->threadContext, 1, "Buffer");
        return FALSE;
    }

    context->SetObjectVariable("CSELF", cSelf);
    return TRUE;
}


/** WindowExtensions::getTextSizeScreen()
 *
 *  Gets the size, width and height, in pixels, needed to display a string in a
 *  specific font.
 *
 *  @param text      The text to calculate the size of.  If this is the only
 *                   argument then the font of this object is used for the
 *                   calculation.
 *
 *  @param type      Optional.  If the text arg is not the only argument, then
 *                   type is required.  It signals what fontSrc is.  The allowed
 *                   types are:
 *
 *                   Indirect -> fontSrc is a font name and fontSize is the size
 *                   of the font.  The calculation is done indirectly by
 *                   temporarily obtaining a logical font.
 *
 *                   DC -> fontSrc is a handle to a device context.  The correct
 *                   font for the calculation must already be selected into this
 *                   device context.  fontSize is ignored.
 *
 *                   Font -> fontSrc is a handle to a font.  fontSize is
 *                   ignored.
 *
 *                   Only the first letter of type is needed and case is not
 *                   significant.
 *
 *  @param fontSrc   Optional.  An object to use for calculating the size of
 *                   text.  The type argument determines how this object is
 *                   interpreted.
 *
 *  @param fontSize  Optional.  The size of the font.  This argument is always
 *                   ignored unless the type argument is Indirect.  If type is
 *                   Indirect and this argument is omitted then the defualt font
 *                   size is used.  (Currently the default size is 8.)
 *
 *  @return  A .Size object containg the width and height for the text in
 *           pixels.
 */
RexxMethod5(RexxObjectPtr, winex_getTextSizeScreen, CSTRING, text, OPTIONAL_CSTRING, type,
            OPTIONAL_CSTRING, fontSrc, OPTIONAL_uint32_t, fontSize, CSELF, pCSelf)
{
    SIZE size = {0};

    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd == NULL )
    {
        goto error_out;
    }

    if ( rxArgCount(context) == 1 )
    {
        if ( ! textSizeFromWindow(context, text, &size, hwnd) )
        {
            goto error_out;
        }
    }
    else if ( argumentOmitted(2) )
    {
        missingArgException(context->threadContext, 2);
        goto error_out;
    }
    else
    {
        if ( argumentOmitted(3) )
        {
            missingArgException(context->threadContext, 3);
            goto error_out;
        }

        char m = toupper(*type);
        if ( m == 'I' )
        {
            if ( argumentOmitted(4) )
            {
                fontSize = DEFAULT_FONTSIZE;
            }
            if ( ! textSizeIndirect(context, text, fontSrc, fontSize, &size, hwnd) )
            {
                goto error_out;
            }
        }
        else if ( m == 'D' )
        {
            HDC hdc = (HDC)string2pointer(fontSrc);
            if ( hdc == NULL )
            {
                invalidTypeException(context->threadContext, 3, " handle to a device context");
            }
            GetTextExtentPoint32(hdc, text, (int)strlen(text), &size);
        }
        else if ( m == 'F' )
        {
            HFONT hFont = (HFONT)string2pointer(fontSrc);
            if ( hFont == NULL )
            {
                invalidTypeException(context->threadContext, 3, " handle to a font");
            }

            HDC hdc = GetDC(hwnd);
            if ( hdc == NULL )
            {
                systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "GetDC");
                goto error_out;
            }

            bool success = true;
            if ( ! getTextExtent(hFont, hdc, text, &size) )
            {
                systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "GetTextExtentPoint32");
                success = false;
            }

            ReleaseDC(hwnd, hdc);
            if ( ! success )
            {
                goto error_out;
            }
        }
        else
        {
            context->RaiseException2(Rexx_Error_Incorrect_method_option, context->String("I, D, F"),
                                     context->String(type));
            goto error_out;
        }
    }

    return rxNewSize(context, size.cx, size.cy);

error_out:
    return NULLOBJECT;
}

/** WindowExtensions::getFont()
 *
 *  Returns the font in use for the dialog or dialog control.
 *
 *  @note  If the window returns NULL for the font, then it has not been set
 *         through a WM_SETFONT message.  In this case it is using the stock
 *         system font. Rather than return 0, we return the stock system font to
 *         the ooDialog programmer.
 *
 */
RexxMethod1(POINTERSTRING, winex_getFont, CSELF, pCSelf)
{
    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd != NULL )
    {
        HFONT hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
        if ( hFont == NULL )
        {
            hFont = (HFONT)GetStockObject(SYSTEM_FONT);
        }
        return hFont;
    }
    return NULLOBJECT;
}

/** WindowExtensions::setFont()
 *
 *  Sets the font used for text in a dialog or dialog control.
 *
 *  @param font  Handle to the new font.
 *
 *  @param redraw  Optional. If true, the window will redraw itself. (According
 *                 to MSDN.) The defualt if this argument is omitted is true.
 *
 *  @return 0, always. The WM_SETFONT message does not return a value.
 */
RexxMethod3(int, winex_setFont, POINTERSTRING, font, OPTIONAL_logical_t, redraw, CSELF, pCSelf)
{
    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd != NULL )
    {
        if ( argumentOmitted(2) )
        {
            redraw = TRUE;
        }
        SendMessage(hwnd, WM_SETFONT, (WPARAM)font, redraw);
    }
    return 0;
}


/** WindowExtensions::createFont()
 *
 *  Creates a logical font with the specified characteristics.
 *
 *  This implementation is broken.  It is the original ooDialog implementation.
 *  It incorrectly maps the point size to the font height and it defaults the
 *  average character width to the point size.
 *
 *  It is maintained "as is" for program compatibility.
 *
 *  @param fontName  Optional.  The typeface name.  The default is System.
 *
 *  @param fSize     Optional.  The point size of the font.  The default is 10.
 *
 *  @param fontStyle Optional.  A string containing 0 or more of the style
 *                              keywords separated by blanks. The default is a
 *                              normal font style.
 *
 *  @param fWidth    Optional.  The average character width.  The default is the
 *                              point size.
 *
 *  @note  The most broken thing with this implementation is defaulting the
 *         average character width to the point size.  Using a 0 for fWidth
 *         rather than omitting the argument will fix this.  0 causes the font
 *         mapper to pick the best font that matches the height.
 *
 */
RexxMethod4(POINTERSTRING, winex_createFont, OPTIONAL_CSTRING, fontName, OPTIONAL_CSTRING, fSize,
            OPTIONAL_CSTRING, fontStyle, OPTIONAL_CSTRING, fWidth)
{
    if ( argumentOmitted(1) )
    {
        fontName = "System";
    }

    int fontSize = 10;
    if ( argumentExists(2) )
    {
        fontSize = atoi(fSize);
    }

    int fontWidth = fontSize;
    if ( argumentExists(4) )
    {
        fontWidth = atoi(fWidth);
    }

    int weight = FW_NORMAL;
    BOOL italic = FALSE;
    BOOL underline = FALSE;
    BOOL strikeout = FALSE;

    if ( argumentExists(3) )
    {
        italic = StrStrI(fontStyle, "ITALIC") != NULL;
        underline = StrStrI(fontStyle, "UNDERLINE") != NULL;
        strikeout = StrStrI(fontStyle, "STRIKEOUT") != NULL;
        weight = getWeight(fontStyle);
    }

    HFONT hFont = CreateFont(fontSize, fontWidth, 0, 0, weight, italic, underline, strikeout,
                             DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, FF_DONTCARE, fontName);
    return hFont;
}

/** WindowExtensions::createFontEx()
 *
 *  Creates a logical font with the specified characteristics.
 *
 *  This is a correct implementation of createFont() and should be used as a
 *  replacement for that method.  In addition it extends createFont() by giving
 *  the ooRexx progammer access to all of the options of the CreateFont API.
 *
 *  @param fontName  Required.  The typeface name.
 *
 *  @param fontSize  Optional.  The point size of the font, the default is 8.
 *
 *  @param args      Optional.  A .Directory object whose indexes can contain
 *                              over-rides for the default values of all other
 *                              arguments to CreateFont.
 *
 *  @return  Handle to the logical font.  On error, a null handle is returned
 *           and the ooDialog System error code (.SystemErrorCode) is set.
 *
 *  @note    All the 'other' arguments to CreateFont() have a default value. If
 *           the args Directory object has no index for a value, the default is
 *           used.  If the Directory object does have the index, then the value
 *           of the index is used for that arg.
 */
RexxMethod4(POINTERSTRING, winex_createFontEx, CSTRING, fontName, OPTIONAL_int, fontSize,
            OPTIONAL_RexxObjectPtr, args, OSELF, self)
{
    int   width = 0;                              // average character width
    int   escapement = 0;                         // angle of escapement
    int   orientation = 0;                        // base-line orientation angle
    int   weight = FW_NORMAL;                     // font weight
    BOOL  italic = FALSE;                         // italic attribute option
    BOOL  underline = FALSE;                      // underline attribute option
    BOOL  strikeOut = FALSE;                      // strikeout attribute option
    DWORD charSet = DEFAULT_CHARSET;              // character set identifier
    DWORD outputPrecision = OUT_TT_PRECIS;        // output precision
    DWORD clipPrecision = CLIP_DEFAULT_PRECIS;    // clipping precision
    DWORD quality = DEFAULT_QUALITY;              // output quality
    DWORD pitchAndFamily = FF_DONTCARE;           // pitch and family

    oodResetSysErrCode(context->threadContext);

    if ( argumentOmitted(2) )
    {
        fontSize = 8;
    }

    HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
    int height = -MulDiv(fontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    DeleteDC(hdc);

    if ( argumentExists(3) )
    {
        if ( ! context->IsDirectory(args) )
        {
            wrongClassException(context->threadContext, 3, "Directory");
            goto error_out;
        }
        RexxDirectoryObject d = (RexxDirectoryObject)args;

        if ( ! rxNumberFromDirectory(context, d, "WIDTH", (DWORD *)&width, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "ESCAPEMENT", (DWORD *)&escapement, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "ORIENTATION", (DWORD *)&orientation, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "WEIGHT", (DWORD *)&weight, 3) )
        {
            goto error_out;
        }
        if ( ! rxLogicalFromDirectory(context, d, "ITALIC", &italic, 3) )
        {
            goto error_out;
        }
        if ( ! rxLogicalFromDirectory(context, d, "UNDERLINE", &underline, 3) )
        {
            goto error_out;
        }
        if ( ! rxLogicalFromDirectory(context, d, "STRIKEOUT", &strikeOut, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "CHARSET", &charSet, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "OUTPUTPRECISION", &outputPrecision, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "CLIPPRECISION", &clipPrecision, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "QUALITY", &quality, 3) )
        {
            goto error_out;
        }
        if ( ! rxNumberFromDirectory(context, d, "PITCHANDFAMILY", &pitchAndFamily, 3) )
        {
            goto error_out;
        }
    }

    HFONT font = CreateFont(height, width, escapement, orientation, weight, italic, underline, strikeOut,
                            charSet, outputPrecision, clipPrecision, quality, pitchAndFamily, fontName);

    if ( font == NULL )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return font;

error_out:
  return NULLOBJECT;
}

/** WindowExtension::hScrollPos()
 *  WindowExtension::vScrollPos()
 *
 *  Retrieves the scroll box position for the appropriate scroll bar.
 *
 *  If this window is a ScrollBar window, then the position of the scroll box in
 *  the scroll bar is retrieved.
 *
 *  Otherwise, hScrollPos gets the position of the scroll box in a window's
 *  standard horizontal scroll bar and vScrollPos sets the position of the
 *  scroll box in a window's standard horizontal scroll bar.
 *
 * @return The position of the scroll box in the scroll bar.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(int, winex_getScrollPos, NAME, method, CSELF, pCSelf)
{
    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd != NULL )
    {
        SCROLLINFO si = {0};
        int type = scrollBarType(hwnd, method);

        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_POS;

        if ( GetScrollInfo(hwnd, type, &si) != 0 )
        {
            return si.nPos;
        }
        oodSetSysErrCode(context->threadContext);
    }
    return 0;
}


/** WindowExtension::setHScrollPos()
 *  WindowExtension::setVScrollPos()
 *
 *  Sets the appropriate scroll bar position.
 *
 *  If this window is a ScrollBar window, then the position of the scroll box in
 *  the scroll bar is set.
 *
 *  Otherwise, setHScrollPos sets the position of the scroll box in a window's
 *  standard horizontal scroll bar and setVScrollPos sets the position of the
 *  scroll box in a window's standard horizontal scroll bar.
 *
 *  @param newPos  The new position of the scroll box.
 *  @param redraw  Whether the scroll bar is immediately redrawn.  The default
 *                 is true.
 *
 *  @return The previous position of the scroll box in the scroll bar.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  MSDN docs are not clear if SetScrollInfo() will set last error.
 *            But, the deprecated SetScrollPos() did set last error.  So, assume
 *            that SetScrollInfo might also set last error.  After the call to
 *            SetScrollInfo(), just set the system error code.  If it is 0, no
 *            harm done.
 */
RexxMethod4(int, winex_setScrollPos, int32_t, newPos, OPTIONAL_logical_t, redraw, NAME, method, CSELF, pCSelf)
{
    int result = 0;
    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd != NULL )
    {
        oodResetSysErrCode(context->threadContext);

        SCROLLINFO si = {0};
        int type = scrollBarType(hwnd, method);

        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_POS;
        si.nPage = newPos;
        if ( argumentOmitted(2) )
        {
            redraw = TRUE;
        }

        result = SetScrollInfo(hwnd, type, &si, (BOOL)redraw);
        oodSetSysErrCode(context->threadContext);
    }
    return result;
}


/** WindowExtensions::scroll()
 *
 *  Scrolls the contents of this window's client area.
 *
 *  @param  amount  The amount to scroll, (cx, cy), in pixels.  A negative cx
 *                  must be used to scroll left and a negative cy to scroll up.
 *
 *                  The amount can be specified in these formats:
 *
 *      Form 1:  A .Size object.
 *      Form 2:  cx, cy
 *
 *  @return  0 for success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  No effort is made to ensure that a .Size object, not a .Point
 *            object is used.
 */
RexxMethod2(RexxObjectPtr, winex_scroll, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd == NULL )
    {
        return TheOneObj;
    }

    RexxMethodContext *c = context;

    // POINT and SIZE structs are binary compatible.  A POINT is used to return
    // the values, even though the semantics are not quite correct for scroll().
    size_t sizeArray;
    int    argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, &point, 1, 3, &sizeArray, &argsUsed) )
    {
        return TheOneObj;
    }

    if ( argsUsed == 1 && sizeArray == 2)
    {
        return tooManyArgsException(context->threadContext, 1);
    }

    if ( ScrollWindowEx(hwnd, point.x, point.y, NULL, NULL, NULL, NULL,  SW_INVALIDATE | SW_ERASE) == ERROR )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
}


/** WindowExtensions::Cursor_Arrow()
 *  WindowExtensions::Cursor_AppStarting()
 *  WindowExtensions::Cursor_Cross()
 *  WindowExtensions::Cursor_No()
 *  WindowExtensions::Cursor_Wait()
 *
 *
 *
 *
 */
RexxMethod2(RexxObjectPtr, winex_setCursorShape, NAME, method, CSELF, pCSelf)
{
    HCURSOR oldCursor = NULL;

    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd == NULL )
    {
        goto done_out;
    }

    LPTSTR cursor = IDC_ARROW;
    switch ( method[7] )
    {
        case 'A' :
            cursor = (method[8] == 'R' ? IDC_ARROW : IDC_APPSTARTING);
            break;
        case 'C' :
            cursor = IDC_CROSS;
            break;
        case 'N' :
            cursor = IDC_NO;
            break;
        case 'W' :
            cursor = IDC_WAIT;
            break;
    }

    HCURSOR hCursor = LoadCursor(NULL, cursor);
    if ( hCursor == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        goto done_out;
    }

    oldCursor = (HCURSOR)setClassPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)hCursor);
    SetCursor(hCursor);

done_out:
    return pointer2string(context, oldCursor);
}


/** WindowExtensions::setCursorPos()
 *
 *  Moves the cursor to the specified position.
 *
 *  @param  newPos  The new position (x, y), in pixels. The amount can be
 *                  specified in these formats:
 *
 *      Form 1:  A .Point object.
 *      Form 2:  x, y
 *
 *  @return  0 for success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  No effort is made to ensure that a .Size object, not a .Point
 *            object is used.
 */
RexxMethod2(RexxObjectPtr, winex_setCursorPos, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd == NULL )
    {
        return NULLOBJECT;
    }

    size_t sizeArray;
    int    argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, &point, 1, 3, &sizeArray, &argsUsed) )
    {
        return NULLOBJECT;
    }

    if ( argsUsed == 1 && sizeArray == 2)
    {
        return tooManyArgsException(context->threadContext, 1);
    }

    if ( SetCursorPos(point.x, point.y) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
}


/** WindowExtensions::getCursorPos()
 *
 *  Retrieves the current cursor position in pixels.
 *
 *  @return The cursor position as a .Point object.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod1(RexxObjectPtr, winex_getCursorPos, CSELF, pCSelf)
{
    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd == NULL )
    {
        return NULLOBJECT;
    }

    POINT p = {0};
    if ( GetCursorPos(&p) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return rxNewPoint(context, p.x, p.y);
}


/** WindowExtension::restoreCursorShape()
 *
 *  Sets the cursor to the specified cursor.
 *
 *  @param  newCursor  [OPTIONAL]  A handle to the new cursor.  If this argument
 *                     is omitted, the cursor is set to the arrow cursor.
 *
 *  @return  A handle to the previous cursor, or a null handle if there was no
 *           previous cursor.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, winex_restoreCursorShape, OPTIONAL_POINTERSTRING, newCursor, CSELF, pCSelf)
{
    HCURSOR oldCursor = NULL;

    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd == NULL )
    {
        goto done_out;
    }

    HCURSOR hCursor;
    if ( argumentExists(1) )
    {
        hCursor = (HCURSOR)newCursor;
    }
    else
    {
        hCursor = LoadCursor(NULL, IDC_ARROW);
        if ( hCursor == NULL )
        {
            oodSetSysErrCode(context->threadContext);
            goto done_out;
        }
    }
    oldCursor = (HCURSOR)setClassPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)hCursor);
    SetCursor(hCursor);

done_out:
    return pointer2string(context, oldCursor);
}


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *  The following functions are used to dump out the dialog admin table(s).
 *  There are several problems with this currently.
 *
 *  One, the code has not been maintained, and the values are probably no longer
 *  complete, and maybe inaccurate.
 *
 *  Two, the setting of the stem is done using RexxVarialbePool which is not
 *  available in the C++ API.
 *
 *  The whole thing needs to be redone (or maybe abandoned.)
 *
\*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

LONG SetRexxStem(const char * name, INT id, const char * secname, const char * data)
{
   SHVBLOCK shvb;
   CHAR buffer[72];

   if (id == -1)
   {
       sprintf(buffer,"%s.%s",name,secname);
   }
   else
   {
       if (secname) sprintf(buffer,"%s.%d.%s",name,id, secname);
       else sprintf(buffer,"%s.%d",name,id);
   }
   shvb.shvnext = NULL;
   shvb.shvname.strptr = buffer;
   shvb.shvname.strlength = strlen(buffer);
   shvb.shvnamelen = shvb.shvname.strlength;
   shvb.shvvalue.strptr = const_cast<char *>(data);
   shvb.shvvalue.strlength = strlen(data);
   shvb.shvvaluelen = strlen(data);
   shvb.shvcode = RXSHV_SYSET;
   shvb.shvret = 0;
   if (RexxVariablePool(&shvb) == RXSHV_BADN) {
       char messageBuffer[265];
       sprintf(messageBuffer, "Variable %s could not be declared", buffer);
       MessageBox(0,messageBuffer,"Error",MB_OK | MB_ICONHAND);
       return FALSE;
   }
   return TRUE;
}


size_t RexxEntry DumpAdmin(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   CHAR data[256];
   /* SHVBLOCK shvb; */
   CHAR name[64];
   CHAR buffer[128];
   DEF_ADM;
   INT i, cnt = 0;

   CHECKARGL(1);

   strcpy(name, argv[0].strptr); /* stem name */
   if (argc == 2)
   {
       dlgAdm = (DIALOGADMIN *)GET_POINTER(argv[1]);
       if (!dlgAdm) RETVAL(-1)

       strcpy(name, argv[0].strptr); /* stem name */
       itoa(dlgAdm->TableEntry, data, 10);
       if (!SetRexxStem(name, -1, "Slot", data)) { RETERR; }
       pointer2string(data, dlgAdm->TheThread);
       if (!SetRexxStem(name, -1, "hThread", data))  { RETERR; }
       pointer2string(data, dlgAdm->TheDlg);
       if (!SetRexxStem(name, -1, "hDialog", data))  { RETERR; }
       pointer2string(data, dlgAdm->BkgBrush);
       if (!SetRexxStem(name, -1, "BkgBrush", data))  { RETERR; }
       pointer2string(data, dlgAdm->BkgBitmap);
       if (!SetRexxStem(name, -1, "BkgBitmap", data))  { RETERR; }
       itoa(dlgAdm->OnTheTop, data, 10);
       if (!SetRexxStem(name, -1, "TopMost", data))  { RETERR; }
       pointer2string(data, dlgAdm->AktChild);
       if (!SetRexxStem(name, -1, "CurrentChild", data))  { RETERR; }
       pointer2string(data, dlgAdm->TheInstance);
       if (!SetRexxStem(name, -1, "DLL", data))  { RETERR; }
       if (!SetRexxStem(name, -1, "Queue", dlgAdm->pMessageQueue))  { RETERR; }
       itoa(dlgAdm->BT_size, data, 10);
       if (!SetRexxStem(name, -1, "BmpButtons", data))  { RETERR; }
       sprintf(buffer, "%s.%s", argv[0].strptr, "BmpTab");
       for (i=0; i<dlgAdm->BT_size; i++)
       {
           itoa(dlgAdm->BmpTab[i].buttonID, data, (dlgAdm->BmpTab[i].Loaded ? 16: 10));
           if (!SetRexxStem(buffer, i+1, "ID", data))  { RETERR; }
           pointer2string(data, (void *)dlgAdm->BmpTab[i].bitmapID);
           if (!SetRexxStem(buffer, i+1, "Normal", data))  { RETERR; }
           pointer2string(data, (void *)dlgAdm->BmpTab[i].bmpFocusID);
           if (!SetRexxStem(buffer, i+1, "Focused", data))  { RETERR; }
           pointer2string(data, (void *)dlgAdm->BmpTab[i].bmpSelectID);
           if (!SetRexxStem(buffer, i+1, "Selected", data))  { RETERR; }
           pointer2string(data, (void *)dlgAdm->BmpTab[i].bmpDisableID);
           if (!SetRexxStem(buffer, i+1, "Disabled", data))  { RETERR; }
       }
       itoa(dlgAdm->MT_size, data, 10);
       if (!SetRexxStem(name, -1, "Messages", data))  { RETERR; }
       sprintf(buffer, "%s.%s", argv[0].strptr, "MsgTab");
       for (i=0; i<dlgAdm->MT_size; i++)
       {
           ultoa(dlgAdm->MsgTab[i].msg, data, 16);
           if (!SetRexxStem(buffer, i+1, "msg", data))  { RETERR; }
           pointer2string(data, (void *)dlgAdm->MsgTab[i].wParam);
           if (!SetRexxStem(buffer, i+1, "param1", data))  { RETERR; }
           pointer2string(data, (void *)dlgAdm->MsgTab[i].lParam);
           if (!SetRexxStem(buffer, i+1, "param2", data))  { RETERR; }
           strcpy(data, dlgAdm->MsgTab[i].rexxProgram);
           if (!SetRexxStem(buffer, i+1, "method", data))  { RETERR; }
       }
       itoa(dlgAdm->DT_size, data, 10);
       if (!SetRexxStem(name, -1, "DataItems", data))  { RETERR; }
       sprintf(buffer, "%s.%s", argv[0].strptr, "DataTab");
       for (i=0; i<dlgAdm->DT_size; i++)
       {
           itoa(dlgAdm->DataTab[i].id, data, 10);
           if (!SetRexxStem(buffer, i+1, "ID", data))  { RETERR; }
           itoa(dlgAdm->DataTab[i].type, data, 10);
           if (!SetRexxStem(buffer, i+1, "type", data))  { RETERR; }
           itoa(dlgAdm->DataTab[i].category, data, 10);
           if (!SetRexxStem(buffer, i+1, "category", data))  { RETERR; }
       }
       itoa(dlgAdm->CT_size, data, 10);
       if (!SetRexxStem(name, -1, "ColorItems", data))  { RETERR; }
       sprintf(buffer, "%s.%s", argv[0].strptr, "ColorTab");
       for (i=0; i<dlgAdm->CT_size; i++)
       {
           itoa(dlgAdm->ColorTab[i].itemID, data, 10);
           if (!SetRexxStem(buffer, i+1, "ID", data))  { RETERR; }
           itoa(dlgAdm->ColorTab[i].ColorBk, data, 10);
           if (!SetRexxStem(buffer, i+1, "Background", data))  { RETERR; }
           itoa(dlgAdm->ColorTab[i].ColorFG, data, 10);
           if (!SetRexxStem(buffer, i+1, "Foreground", data)) { RETERR; }
       }
   }

   if (argc == 1)
   {
       for (i=0; i<MAXDIALOGS; i++)
       {
           if (DialogTab[i] != NULL)
           {
               cnt++;
               pointer2string(data, DialogTab[i]);
               if (!SetRexxStem(name, cnt, "AdmBlock", data)) { RETERR; }
               itoa(DialogTab[i]->TableEntry, data, 10);
               if (!SetRexxStem(name, cnt, "Slot", data)) { RETERR; }
               pointer2string(data, DialogTab[i]->TheThread);
               if (!SetRexxStem(name, cnt, "hThread", data)) { RETERR; }
               pointer2string(data, DialogTab[i]->TheDlg);
               if (!SetRexxStem(name, cnt, "hDialog", data)) { RETERR; }
               pointer2string(data, DialogTab[i]->BkgBrush);
               if (!SetRexxStem(name, cnt, "BkgBrush", data)) { RETERR; }
               pointer2string(data, DialogTab[i]->BkgBitmap);
               if (!SetRexxStem(name, cnt, "BkgBitmap", data)) { RETERR; }
               itoa(DialogTab[i]->OnTheTop, data, 10);
               if (!SetRexxStem(name, cnt, "TopMost", data)) { RETERR; }
               pointer2string(data, DialogTab[i]->AktChild);
               if (!SetRexxStem(name, cnt, "CurrentChild", data)) { RETERR; }
               pointer2string(data, DialogTab[i]->TheInstance);
               if (!SetRexxStem(name, cnt, "DLL", data)) { RETERR; }
               if (!SetRexxStem(name, cnt, "Queue", DialogTab[i]->pMessageQueue)) { RETERR; }
               itoa(DialogTab[i]->BT_size, data, 10);
               if (!SetRexxStem(name, cnt, "BmpButtons", data)) { RETERR; }
               itoa(DialogTab[i]->MT_size, data, 10);
               if (!SetRexxStem(name, cnt, "Messages", data)) { RETERR; }
               itoa(DialogTab[i]->DT_size, data, 10);
               if (!SetRexxStem(name, cnt, "DataItems", data)) { RETERR; }
               itoa(DialogTab[i]->CT_size, data, 10);
               if (!SetRexxStem(name, cnt, "ColorItems", data)) { RETERR; }
           }
       }
       itoa(cnt, data, 10);
       if (!SetRexxStem(name, 0, NULL, data)) { RETERR; }  /* Set number of dialog tables */
   }
   RETC(0);
}

