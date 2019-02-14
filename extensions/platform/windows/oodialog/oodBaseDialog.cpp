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
 * oodBaseDialog.cpp
 *
 * Some of the base classes for all non-trival dialogs in the ooDialog package.
 * Contains the method implmentations for the ResDialog and WindowExtensions
 * classes.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h, commctrl.h, and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <shlwapi.h>
#include "APICommon.hpp"
#include "ooShapes.hpp"
#include "oodCommon.hpp"
#include "oodShared.hpp"
#include "oodControl.hpp"
#include "oodData.hpp"
#include "oodMessaging.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodResizableDialog.hpp"
#include "oodUser.hpp"


class LoopThreadArgs
{
public:
    DLGTEMPLATEEX     *dlgTemplate;
    pCPlainBaseDialog  pcpbd;
    uint32_t           resourceId;
    uint32_t           autoDetectResult;
    bool              *release;       // Used to signal thread initialization complete
};


/**
 * Checks that a ControlDialog has a proper, active, running, owner dialog.
 * Sets that owner dialog information in the CSelf of the control dialog, the
 * owned dialog.
 *
 * Sets the pCPlainBaseDialog pointer of the control dialog as one of the child
 * dialogs in the owner's childDlg[] array.  This is important, and confusing.
 * The childDlg array was originally for Category dialogs and is defined as a
 * HWND array.  Category dialogs are deprecated and have no way to become the
 * owner dialog of a ControlDialog.
 *
 * So, when starting the implementation for ControlDialogs, to save space in the
 * plain base dialog struct, the existing childDlg array was used to track the
 * owner dialog's owned control dialogs. The cPlainBaseDialog pointer is cast to
 * HWND and placed in the array.
 *
 * Since *every* ControlDialog passes through this function before the
 * underlying dialog is created, we are guaranteed that any owner dialog that
 * has an activated child control dialog, has the pCPlainBaseDialog pointer for
 * every child control dialog in its childDlg array, and no other pointer types
 * in that array.
 *
 * PropertySheetDialogs track their child dialogs through a cppPage array and
 * for PropertySheetDialogs, the childDlg array is left alone.
 *
 * The window message processing function of a control dialog executes in the
 * same thread as the owner dialog, so we copy the thread context and thread ID
 * from the owner dialog.  This ensures the thread context and thread ID are set
 * for all ControlDialog dialogs, except for PropertySheetPage dialogs and
 * ControlDialogs in a ManagedTab tab.
 *
 * For PropertySheetPage dialogs, the PropertySheetDialog code handles setting
 * the thread context and ID in assignPSDThreadContext() and updatePageCSelf().
 *
 * For ControlDialog dialogs in a ManagedTab tab, the TabOwnerDialog code sets
 * the thread context and ID in createMTPageDlg()
 *
 *
 * @param c      Method context we are operating under.
 * @param pcpbd  Pointer to the CSelf struct of the ControlDialog
 *
 * @return True if all okay, otherwise false.
 *
 * @note  An exception has been raised if false is returned.
 *
 * @remarks  If AttachThread() failed for the parent dialog, that dialog is
 *           ended prematurely and we couldn't be here.  The control dialog will
 *           be, has to be, created on the the owner dialog's message
 *           proccessing thread.  So we can set the dlgProcContext here to the
 *           thread context of the owner dialog.
 *
 */
bool validControlDlg(RexxMethodContext *c, pCPlainBaseDialog pcpbd)
{
    if ( pcpbd->rexxOwner == NULL )
    {
        noOwnerRexxDialogException(c, pcpbd->rexxSelf);
        goto err_out;
    }

    pCPlainBaseDialog ownerPcpbd = requiredDlgCSelf(c, pcpbd->rexxOwner, oodPlainBaseDialog, 0, NULL);
    if ( ownerPcpbd == NULL )
    {
        goto err_out;
    }
    if ( ownerPcpbd->hDlg == NULL || ! ownerPcpbd->isActive )
    {
        noParentWindowsDialogException(c, pcpbd->rexxSelf);
        goto err_out;
    }
    if ( ownerPcpbd->countChilds >= MAXCHILDDIALOGS )
    {
        char buf[128];
        _snprintf(buf, sizeof(buf),
                  "The number of owned dialogs has\n"
                  "reached the maximum (%d) allowed\n\n"
                  "No more owned dialogs can be instantiated", MAXCHILDDIALOGS);
        MessageBox(NULL, buf, "ooDialog Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);

        goto err_out;
    }

    pcpbd->hOwnerDlg      = ownerPcpbd->hDlg;
    pcpbd->ownerCSelf     = ownerPcpbd;
    pcpbd->dlgProcContext = ownerPcpbd->dlgProcContext;

    ownerPcpbd->countChilds++;
    ownerPcpbd->isOwnerDlg = true;
    ownerPcpbd->childDlg[ownerPcpbd->countChilds] = (HWND)pcpbd;
    return true;

err_out:
    return false;
}


/**
 * Checks that an owned dialog has an underlying owner window and sets up the
 * popup as child data.
 *
 * All owned dialogs are treated as if they were started with popupAsChild()
 * even if they weren't. If rexxParent is set, then we actually were started
 * with popupAsChild(). If not set then we do that here.
 *
 * @param c
 * @param pcpbd
 *
 * @remarks
 */
bool processOwnedDialog(RexxMethodContext *c, pCPlainBaseDialog pcpbd)
{
    pCPlainBaseDialog ownerPcpbd = requiredDlgCSelf(c, pcpbd->rexxOwner, oodPlainBaseDialog, 0, NULL);
    if ( ownerPcpbd == NULL )
    {
        goto err_out;
    }

    if ( ownerPcpbd->hDlg == NULL || ! ownerPcpbd->isActive )
    {
        methodCanNotBeInvokedException(c, pcpbd->rexxSelf, "owner Windows dialog does not exist");
        goto err_out;
    }
    pcpbd->hOwnerDlg = ownerPcpbd->hDlg;

    if ( pcpbd->rexxParent == NULLOBJECT )
    {
        pcpbd->rexxParent = pcpbd->rexxOwner;
        RexxObjectPtr childList = c->SendMessage0(pcpbd->rexxParent, "CHILDDIALOGS");
        if ( childList == NULLOBJECT )
        {
            baseClassInitializationException(c);
            goto err_out;
        }
        c->SendMessage1(childList, "INSERT", pcpbd->rexxSelf);
    }

    return true;

err_out:
    return false;
}


/**
 *  Methods for the .ResDialog class.
 */
#define RESDIALOG_CLASS        "ResDialog"

/**
 * Forces a binary compiled dialog template to have the WS_THICKFRAME style.
 *
 * For resizable dialogs, (dialogs inheriting ResizeAdmin,) we add the thick
 * frame style to the dialog automatically.  This allows users to simply inherit
 * ResizeAdmin and have a resizable dialog with no additional code changes.
 *
 * This is easy to do for RcDialog and UserDialog dialogs.  For ResDialogs we
 * need to update the dialog template in the resource DLL.  We do that here.
 * The process is relatively simple.  Find the template in the DLL, load that
 * resource, lock the resource to get a regular pointer to the template, and
 * then modify the style in the template.
 *
 * @param pcpbd
 * @param dlgID
 *
 * @return DLGTEMPLATEEX*
 *
 * @note  If this doesn't work and we return null, the calling code just ignores
 *        it.  The dialog is created as usual.  If the template already has the
 *        thick frame style, the user will never notice anything.  Otherwise,
 *        the user will not have a resizable dialog.  This never failed during
 *        testing, so it is unlikely that it will not work.
 */
static DLGTEMPLATEEX *loadAndFixResizableDlgTemplate(pCPlainBaseDialog pcpbd, int32_t dlgID)
{
    DLGTEMPLATEEX *dlgTemplate = NULL;

    HRSRC hRes = FindResource(pcpbd->hInstance, MAKEINTRESOURCE(dlgID), RT_DIALOG);
    if ( hRes == NULL )
    {
        goto err_out;
    }

    HGLOBAL hResLoad = LoadResource(pcpbd->hInstance, hRes);
    if (hResLoad == NULL)
    {
        goto err_out;
    }

    dlgTemplate = (DLGTEMPLATEEX *)LockResource(hResLoad);
    if ( dlgTemplate == NULL )
    {
        goto err_out;
    }

    if ( dlgTemplate->signature == 0xFFFF )
    {
        dlgTemplate->style |= WS_THICKFRAME;
    }
    else
    {
        ((DLGTEMPLATE *)dlgTemplate)->style |= WS_THICKFRAME;
    }

err_out:
    return dlgTemplate;
}

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
 *
 * @remarks  If auto detect is on, we call doDataAutoDetction().  This only
 *           fails on a malloc error, or if the data table fills up.  If the
 *           data table is full, that's okay we can continue.  If there is a
 *           malloc error,
 */
DWORD WINAPI WindowLoopThread(void *arg)
{
    ULONG ret;
    LoopThreadArgs *args = (LoopThreadArgs *)arg;

    pCPlainBaseDialog pcpbd = args->pcpbd;
    bool *release = args->release;

    DLGPROC dlgProc = (DLGPROC)RexxDlgProc;
    if ( pcpbd->isTabOwnerDlg )
    {
        dlgProc = (DLGPROC)RexxTabOwnerDlgProc;
    }
    else if ( pcpbd->isResizableDlg )
    {
        dlgProc = (DLGPROC)RexxResizableDlgProc;
    }

    if ( args->dlgTemplate != NULL )
    {
        pcpbd->hDlg = CreateDialogIndirectParam(MyInstance, (LPCDLGTEMPLATE)args->dlgTemplate, pcpbd->hOwnerDlg,
                                                dlgProc, (LPARAM)pcpbd);
    }
    else
    {
        pcpbd->hDlg = CreateDialogParam(pcpbd->hInstance, MAKEINTRESOURCE(args->resourceId), pcpbd->hOwnerDlg,
                                        dlgProc, (LPARAM)pcpbd);
    }

    if ( pcpbd->hDlg == NULL )
    {
        *release = true;
        goto done_out;
    }

    pcpbd->childDlg[0] = pcpbd->hDlg;

    if ( pcpbd->autoDetect )
    {
        args->autoDetectResult = doDataAutoDetection(NULL, pcpbd);
        if ( args->autoDetectResult == OOD_MEMORY_ERR )
        {
            pcpbd->hDlgProcThread = NULL;
            *release = true;
            goto done_out;
        }
    }

    // Release wait in startDialog() and mark the dialog as active.
    *release = true;
    pcpbd->isActive = true;

    MSG msg;
    BOOL result;

    while ( (result = GetMessage(&msg, NULL, 0, 0)) != 0 && pcpbd->dlgAllocated )
    {
        if ( result == -1 )
        {
            break;
        }
        if ( ! IsDialogMessage(pcpbd->hDlg, &msg) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

done_out:

    // Need to synchronize here, otherwise dlgAllocated may still be true, but
    // delDialog() is already running.
    EnterCriticalSection(&crit_sec);
    if ( pcpbd->dlgAllocated )
    {
        ret = delDialog(pcpbd, pcpbd->dlgProcContext);
        pcpbd->hDlgProcThread = NULL;
    }
    LeaveCriticalSection(&crit_sec);

    if ( pcpbd->dlgProcContext != NULL )
    {
        pcpbd->dlgProcContext->DetachThread();
        pcpbd->dlgProcContext = NULL;
    }
    return ret;
}


/**
 *  Used to set the fontName and fontSize attributes of the resource dialog.
 */
void setFontAttrib(RexxThreadContext *c, pCPlainBaseDialog pcpbd)
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


RexxMethod7(RexxObjectPtr, resdlg_init, RexxObjectPtr, library, RexxObjectPtr, resourceID, OPTIONAL_RexxObjectPtr, dlgData,
            OPTIONAL_RexxObjectPtr, includeFile, OPTIONAL_RexxObjectPtr, ownerData,
            SUPER, super, OSELF, self)
{
    RexxArrayObject newArgs = context->NewArray(5);

    context->ArrayPut(newArgs, library, 1);
    context->ArrayPut(newArgs, resourceID, 2);
    if ( argumentExists(3) )
    {
        context->ArrayPut(newArgs, dlgData, 3);
    }
    if ( argumentExists(4) )
    {
        context->ArrayPut(newArgs, includeFile, 4);
    }
    if ( argumentExists(5) )
    {
        context->ArrayPut(newArgs, ownerData, 5);
    }

    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, newArgs);

    if ( result == TheZeroObj )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->GetCSelf();

        if ( pcpbd->isControlDlg || pcpbd->isTabOwnerDlg)
        {
            RexxPointerObject p = context->NewPointer(pcpbd);

            if ( pcpbd->isControlDlg )
            {
                result = context->SendMessage1(self, "CONTROLDLGINIT", p);
            }

            if ( pcpbd->isTabOwnerDlg && result == TheZeroObj )
            {
                result = context->SendMessage1(self, "TABOWNERDLGINIT", p);
            }
        }
    }

    return result;
}

/**
 * Creates the underlying Windows dialog using a dialog resource stored in a
 * DLL.  Currently this is only used for ResDialog dialogs.  All other ooDialog
 * dialogs use DynamicDialog::startParentDialog() to create the underlying
 * Windows dialog.
 *
 * @param libray      The name of the DLL.
 * @param dlgID       The resource ID for the dialog in the DLL
 * @param iconID      Ther resource ID to use for the application icon.
 * @param modeless    Whether to create a modeless or a modal dialog.
 *
 * @return True on succes, otherwise false.
 */
RexxMethod5(logical_t, resdlg_startDialog_pvt, CSTRING, library, RexxObjectPtr, _dlgID, uint32_t, iconID,
            logical_t, modeless, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    int32_t dlgID = oodResolveSymbolicID(context->threadContext, pcpbd->rexxSelf, _dlgID, -1, 2, true);
    if ( dlgID == OOD_ID_EXCEPTION )
    {
        return FALSE;
    }

    if ( pcpbd->isOwnedDlg )
    {
        modeless = TRUE;
        if ( ! processOwnedDialog(context, pcpbd) )
        {
            return false;
        }
    }

    ULONG thID;
    bool Release = false;

    EnterCriticalSection(&crit_sec);
    if ( ! installNecessaryStuff(pcpbd, library) )
    {
        delDialog(pcpbd, context->threadContext);

        LeaveCriticalSection(&crit_sec);
        return FALSE;
    }

    DLGTEMPLATEEX *dlgTemplate = NULL;
    if ( pcpbd->isResizableDlg )
    {
        // If this fails we just ignore it.
        dlgTemplate = loadAndFixResizableDlgTemplate(pcpbd, dlgID);
    }

    LoopThreadArgs threadArgs;
    threadArgs.dlgTemplate      = dlgTemplate;
    threadArgs.pcpbd            = pcpbd;
    threadArgs.resourceId       = dlgID;
    threadArgs.autoDetectResult = OOD_NO_ERROR;
    threadArgs.release          = &Release;

    pcpbd->hDlgProcThread = CreateThread(NULL, 2000, WindowLoopThread, &threadArgs, 0, &thID);

    // Wait for thread to signal us to continue, don't wait if the thread was not created.
    while ( ! Release && pcpbd->hDlgProcThread != NULL )
    {
        Sleep(1);
    }
    LeaveCriticalSection(&crit_sec);

    // If auto detection was on, but failed on a memory allocation, a dialog was
    // created.  delDialog() will be invoked in the window loop thread, to end
    // that dialog.  Here we just need to raise the out of memory exception and
    // return false.
    if ( threadArgs.autoDetectResult == OOD_MEMORY_ERR )
    {
        outOfMemoryException(context->threadContext);
        return FALSE;
    }

    if ( pcpbd->hDlg )
    {
        setDlgHandle(pcpbd);

        // Set the thread priority higher for faster drawing.
        SetThreadPriority(pcpbd->hDlgProcThread, THREAD_PRIORITY_ABOVE_NORMAL);
        pcpbd->onTheTop = true;
        pcpbd->dlgProcThreadID = thID;

        // Is this to be a modal dialog?
        checkModal((pCPlainBaseDialog)pcpbd->previous, modeless);

        HICON hBig = NULL;
        HICON hSmall = NULL;
        if ( getDialogIcons(pcpbd, iconID, ICON_DLL, (PHANDLE)&hBig, (PHANDLE)&hSmall) )
        {
            pcpbd->sysMenuIcon = (HICON)setClassPtr(pcpbd->hDlg, GCLP_HICON, (LONG_PTR)hBig);
            pcpbd->titleBarIcon = (HICON)setClassPtr(pcpbd->hDlg, GCLP_HICONSM, (LONG_PTR)hSmall);
            pcpbd->didChangeIcon = true;

            SendMessage(pcpbd->hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hSmall);
        }

        setFontAttrib(context->threadContext, pcpbd);
        return TRUE;
    }

    // The dialog creation failed, return false.  When the dialog creation fails
    // in the WindowLoop thread, delDialog() is done immediately, as it skips
    // entering the message processing loop.

    return FALSE;
}


RexxMethod2(RexxArrayObject, resdlg_getDataTableIDs_pvt, CSELF, pCSelf, OSELF, self)
{
    return getDataTableIDs(context, (pCPlainBaseDialog)pCSelf, self);
}


/**
 *  Methods for the .WindowExtensions class.
 */
#define WINDOWEXTENSIONS_CLASS    "WindowExtensions"


#define CREATE_FONT_EX_ARG_LIST   "a Directory object or a keyword string containing at lease one of the font style keywords"

static inline HWND getWEWindow(void *pCSelf)
{
    return ((pCWindowExtensions)pCSelf)->hwnd;
}

static HWND winExtSetup(RexxMethodContext *c, void *pCSelf)
{
    if ( pCSelf == NULL )
    {
        return (HWND)baseClassInitializationException(c);
    }

    oodResetSysErrCode(c->threadContext);
    HWND hwnd = getWEWindow(pCSelf);

    if ( hwnd == NULL )
    {
        noWindowsDialogException(c, ((pCWindowExtensions)pCSelf)->rexxSelf);
    }
    return hwnd;
}


static RexxStringObject textAlign2string(RexxMethodContext *c, uint32_t align)
{
    char buf[256] = {'\0'};

    uint32_t flags = TA_LEFT | TA_RIGHT | TA_CENTER;
    switch ( flags & align )
    {
        case TA_LEFT :
            strcpy(buf, "LEFT ");
            break;
        case TA_RIGHT :
            strcpy(buf, "RIGHT ");
            break;
        case TA_CENTER :
            strcpy(buf, "CENTER ");
            break;
    }

    flags = TA_BOTTOM | TA_TOP | TA_BASELINE;
    switch ( flags & align )
    {
        case TA_TOP :
            strcat(buf, "TOP ");
            break;
        case TA_BOTTOM :
            strcat(buf, "BOTTOM ");
            break;
        case TA_BASELINE :
            strcat(buf, "BASELINE ");
            break;
    }

    flags = TA_NOUPDATECP | TA_UPDATECP;
    switch ( flags & align )
    {
        case TA_NOUPDATECP :
            strcat(buf, "NOUPDATECP");
            break;
        case TA_UPDATECP :
            strcat(buf, "UPDATECP");
            break;
    }

    return c->String(buf);
}


static uint32_t string2textAlign(CSTRING flags)
{
    // Note that all 3 of these flags are 0.
    uint32_t align = TA_LEFT | TA_TOP | TA_NOUPDATECP;

    // flags can be null.
    if ( flags != NULL && strlen(flags) > 0 )
    {
        if ( StrStrI(     flags, "RIGHT")      != NULL ) align |= TA_RIGHT;
        else if ( StrStrI(flags, "CENTER")     != NULL ) align |= TA_CENTER;

        if ( StrStrI(     flags, "BOTTOM")     != NULL ) align |= TA_BOTTOM;
        else if ( StrStrI(flags, "BASELINE")   != NULL ) align |= TA_BASELINE;

        if ( StrStrI(     flags, "NOUPDATECP") != NULL ) align |= TA_NOUPDATECP;
        else if ( StrStrI(flags, "UPDATECP")   != NULL ) align |= TA_UPDATECP;
    }

    return align;
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
    if ( ! context->IsBuffer(cSelf) )
    {
        wrongClassException(context->threadContext, 1, "Buffer");
        return FALSE;
    }

    context->SetObjectVariable("CSELF", cSelf);
    return TRUE;
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


/** WindowExtensions::hScrollPos()
 *  WindowExtensions::vScrollPos()
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


/** WindowExtensions::setHScrollPos()
 *  WindowExtensions::setVScrollPos()
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

    // POINT and SIZE structs are binary compatible.  A POINT is used to return
    // the values, even though the semantics are not quite correct for scroll().
    size_t sizeArray;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, (PORXPOINT)&point, 1, 2, &sizeArray, &argsUsed) )
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


/** WindowExtensions::getDC()
 *
 *  Retrieves the device context (DC) for the entire window.  For a dialog, this
 *  includes title bar, menus, and scroll bars.  A window device context permits
 *  painting anywhere in a window, because the origin of the device context is
 *  the upper-left corner of the window instead of the client area.
 *
 *  @note  It is possible to retrieve a DC for the entire window, or for the
 *         client area of the window.  The MSDN docs say this about retrieving
 *         the DC for the entire window:
 *
 *         Getting the DC for the entire window "is intended for special
 *         painting effects within a window's nonclient area. Painting in
 *         nonclient areas of any window is not recommended."
 *
 *  @param  client  [OPTIONAL]  If this argument is .true, the DC returned is
 *                  for the client area of the window.
 *
 *  @return  A handle to the device context if successful, otherwise a null
 *           handle.
 *
 *  @note  Sets .SystemErrorCode.
 *
 *  @remarks  This method was documented as returning 0 on failure.  The
 *            optional client parameter was added after 4.0.0 and needs to be
 *            documented.
 */
RexxMethod2(RexxObjectPtr, winex_getDC, OPTIONAL_logical_t, client, CSELF, pCSelf)
{
    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd == NULL )
    {
        return 0;
    }

    HDC hDC;
    if ( client )
    {
        hDC = GetDC(hwnd);
    }
    else
    {
        hDC = GetWindowDC(hwnd);
    }

    if ( hDC == NULL )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return pointer2string(context, hDC);
}


/** WindowExtensions::freeDC()
 *
 *  Releases the device context (DC) for this object that was obtained through
 *  the getDC() method invoked on this object.
 *
 *  It is important to invoke this method using the correct DC.  The DC being
 *  freed must have been the DC obtained using the getDC() from the same object.
 *  E.g, this is not correct:
 *
 *  listView = self~newListView(IDC_LV_NAMES)
 *  hDC = listView~getDC
 *  ...
 *  self~freeDC(hDC)
 *
 *  This is correct:
 *
 *  listView = self~newListView(IDC_LV_NAMES)
 *  hDC = listView~getDC
 *  ...
 *  listView~freeDC(hDC)
 *
 *  @param  hDC  The device context to free.
 *
 *  @return  0 for success, 1 for failure.
 */
RexxMethod2(RexxObjectPtr, winex_freeDC, POINTERSTRING, hDC, CSELF, pCSelf)
{
    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd == NULL )
    {
        return TheFalseObj;
    }

    if ( ReleaseDC(hwnd, (HDC)hDC) == 0 )
    {
        return TheOneObj;
    }
    return TheZeroObj;
}


/** WindowsExtensions::getTextExtent()
 *
 *  Gets the size bounding rectangle for the specified text, if it were to be
 *  drawn in the specified device context.
 *
 *  @param  hDC   The device context the text is going to be written to.
 *  @param  text  The text to be written.
 *
 *  @return  A size object that reflects the size of the bounding rectangle of
 *           the text.
 *
 *  @note  Sets the .systemErrorCode
 *
 *         The size is calculated using the current selected font in the DC.
 *
 */
RexxMethod3(RexxObjectPtr, winex_getTextExtent, POINTERSTRING, hDC, CSTRING, text, CSELF, pCSelf)
{
    RexxObjectPtr result = rxNewSize(context, 0, 0);
    SIZE s = {0};

    if ( winExtSetup(context, pCSelf) == NULL )
    {
        goto done_out;
    }
    if ( hDC == NULL )
    {
        nullObjectException(context->threadContext, "device context", 1);
        goto done_out;
    }

    if ( GetTextExtentPoint32((HDC)hDC, text, (int32_t)strlen(text), &s) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        goto done_out;
    }
    result = rxNewSize(context, s.cx, s.cy);

done_out:
    return result;
}


/** WindowsExtensions::getTextAlign()
 *
 *  Gets the text alignment setting for the specified device context.
 *
 *  @param  hDC   The device context whose text alignment is required.
 *
 *  @return  A string of keywords specifying the text alignment.  The returned
 *           string will contain exactly one keyword from the following 3 groups
 *           of keywords, in the same order as the groups
 *
 *           LEFT RIGHT CENTER
 *
 *           TOP BOTTOM BASELINE
 *
 *           NOUPDATECP UPDATECP
 *
 *  @note  Sets the .systemErrorCode
 *
 *         The text alignment flags determine how the text writing methods
 *         align a string of text in relation to the reference point provided to
 *         the method
 */
RexxMethod2(RexxStringObject, winex_getTextAlign, POINTERSTRING, hDC, CSELF, pCSelf)
{
    RexxStringObject result = context->NullString();

    if ( winExtSetup(context, pCSelf) == NULL )
    {
        goto done_out;
    }
    if ( hDC == NULL )
    {
        nullObjectException(context->threadContext, "device context", 1);
        goto done_out;
    }

    uint32_t align = GetTextAlign((HDC)hDC);
    if ( align == GDI_ERROR )
    {
        oodSetSysErrCode(context->threadContext);
        goto done_out;
    }

    result = textAlign2string(context, align);

done_out:
    return result;
}


/** WindowsExtensions::setTextAlign()
 *
 *  Sets the text alignment setting for the specified device context.
 *
 *  @param  hDC   The device context whose text alignment is bein set.
 *
 *  @param  align  A string of keywords specifying the text alignment.  The
 *                 following keywords are recognized.  Specify exactly one
 *                 keyword from the following 3 groups of keywords, case and
 *                 order are insignificant.
 *
 *           LEFT RIGHT CENTER
 *
 *           TOP BOTTOM BASELINE
 *
 *           NOUPDATECP UPDATECP
 *
 *                The default if this argument is omitted is:
 *
 *                LEFT TOP NOUPDATECP.
 *
 *                For any group that has no keyword specified, the default for
 *                that group is used.  I.e., if neither LEFT RIGHT nor CENTER is
 *                specified, LEFT is used.
 *
 *  @note  Sets the .systemErrorCode
 *
 *         The text alignment flags determine how the text writing methods
 *         align a string of text in relation to the reference point provided to
 *         the method.
 */
RexxMethod3(RexxStringObject, winex_setTextAlign, POINTERSTRING, hDC, OPTIONAL_CSTRING, _align, CSELF, pCSelf)
{
    RexxStringObject result = context->NullString();

    if ( winExtSetup(context, pCSelf) == NULL )
    {
        goto done_out;
    }
    if ( hDC == NULL )
    {
        nullObjectException(context->threadContext, "device context", 1);
        goto done_out;
    }

    uint32_t align = string2textAlign(_align);
    uint32_t prev  = SetTextAlign((HDC)hDC, align);
    if ( prev == GDI_ERROR )
    {
        oodSetSysErrCode(context->threadContext);
        goto done_out;
    }

    result = textAlign2string(context, prev);

done_out:
    return result;
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
 *  @remarks  Note that this method does not make sense as a windows extension,
 *            it does not require a valid window handle.
 *
 *            It would be better to use a common function for the implementation
 *            and put a createFontEx() method in both PlainBaseDialog and
 *            DialogControl that delegates to the common function.
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
    parseFontStyleArg(fontStyle, &weight, &italic, &underline, &strikeout);

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
 *
 *  @remarks  Note that this method does not make sense as a windows extension,
 *            it does not require a valid window handle.
 *
 *            It would be better to use a common function for the implementation
 *            and put a createFontEx() method in both PlainBaseDialog and
 *            DialogControl that delegates to the common function.
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
    uint32_t charSet = DEFAULT_CHARSET;           // character set identifier
    uint32_t outputPrecision = OUT_TT_PRECIS;     // output precision
    uint32_t clipPrecision = CLIP_DEFAULT_PRECIS; // clipping precision
    uint32_t quality = DEFAULT_QUALITY;           // output quality
    uint32_t pitchAndFamily = FF_DONTCARE;        // pitch and family

    oodResetSysErrCode(context->threadContext);

    if ( argumentOmitted(2) )
    {
        fontSize = 8;
    }
    int height = getHeightFromFontSize(fontSize);

    if ( argumentExists(3) )
    {
        if ( context->IsDirectory(args) )
        {
            RexxDirectoryObject d = (RexxDirectoryObject)args;

            if ( ! rxNumberFromDirectory(context, d, "WIDTH", (uint32_t *)&width, 3, false) )
            {
                goto error_out;
            }
            if ( ! rxNumberFromDirectory(context, d, "ESCAPEMENT", (uint32_t *)&escapement, 3, false) )
            {
                goto error_out;
            }
            if ( ! rxNumberFromDirectory(context, d, "ORIENTATION", (uint32_t *)&orientation, 3, false) )
            {
                goto error_out;
            }
            if ( ! rxNumberFromDirectory(context, d, "WEIGHT", (uint32_t *)&weight, 3, false) )
            {
                goto error_out;
            }
            if ( ! rxLogicalFromDirectory(context, d, "ITALIC", &italic, 3, false) )
            {
                goto error_out;
            }
            if ( ! rxLogicalFromDirectory(context, d, "UNDERLINE", &underline, 3, false) )
            {
                goto error_out;
            }
            if ( ! rxLogicalFromDirectory(context, d, "STRIKEOUT", &strikeOut, 3, false) )
            {
                goto error_out;
            }
            if ( ! rxNumberFromDirectory(context, d, "CHARSET", &charSet, 3, false) )
            {
                goto error_out;
            }
            if ( ! rxNumberFromDirectory(context, d, "OUTPUTPRECISION", &outputPrecision, 3, false) )
            {
                goto error_out;
            }
            if ( ! rxNumberFromDirectory(context, d, "CLIPPRECISION", &clipPrecision, 3, false) )
            {
                goto error_out;
            }
            if ( ! rxNumberFromDirectory(context, d, "QUALITY", &quality, 3, false) )
            {
                goto error_out;
            }
            if ( ! rxNumberFromDirectory(context, d, "PITCHANDFAMILY", &pitchAndFamily, 3, false) )
            {
                goto error_out;
            }
        }
        else
        {
            CSTRING fontStyle = context->ObjectToStringValue(args);
            if ( ! parseFontStyleArg(fontStyle, &weight, &italic, &underline, &strikeOut) )
            {
                wrongArgValueException(context->threadContext, 3, CREATE_FONT_EX_ARG_LIST, args);
                goto error_out;
            }
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

/** WindowExtensions::writeDirect()
 *
 *
 *  @remarks  Note that this method does not make sense as a windows extension,
 *            it does not require a valid window handle.
 */
RexxMethod4(logical_t, winex_writeDirect, POINTERSTRING, hDC, int32_t, xPos, int32_t, yPos, CSTRING, text)
{
    if ( hDC != NULL )
    {
        TextOut((HDC)hDC, xPos, yPos, text, (int)strlen(text));
        return 0;
    }
    return 1;
}

/** WindowExtensions::loadBitmap()
 *
 *  Loads a bitmap into memory.
 *
 *  @param  opts  [OPTIONAL]  The only valid option is USEPAL.  If so, sets the
 *                color palette of the bitmap as the system color palette.
 *
 *  @return  The handle of the bitmap, or 0 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  Note that this method does not make sense as a windows extension,
 *            it does not require a valid window handle.
 *
 *  @remarks  For maybeSetColorPalette(): self could be either a dialog or a
 *            dialog control. If self is a dialog control, then we get the CSelf
 *            of its owner dialog. If we don't get the CSelf, an exception has
 *            been raised.
 *
 *            The only reason for this would be the user invoking the method
 *            in an init() method before invoking the superclass init(). IMHO,
 *            that should always raise a syntax error.
 */
RexxMethod3(RexxStringObject, winex_loadBitmap, CSTRING, bitmapFile, OPTIONAL_CSTRING, opts, OSELF, self)
{
    oodResetSysErrCode(context->threadContext);
    uint32_t errCode = 0;

    HBITMAP hBmp = (HBITMAP)loadDIB(bitmapFile, &errCode);
    if ( hBmp == NULL )
    {
        oodSetSysErrCode(context->threadContext, errCode);
    }
    else
    {
        pCPlainBaseDialog pcpbd = requiredDlgCSelf(context, self, oodUnknown, 0, NULL);
        if ( pcpbd == NULL )
        {
            return NULLOBJECT;
        }

        maybeSetColorPalette(context, hBmp, opts, pcpbd);
    }
    return pointer2string(context, hBmp);
}


/** WindowsExtensions::removeBitmap()
 *
 *
 *  @remarks  Note that this method does not make sense as a windows extension,
 *            it does not require a valid window handle.
 */
RexxMethod1(logical_t, winex_removeBitmap, POINTERSTRING, hBitmap)
{
    if ( hBitmap != NULL )
    {
        LocalFree(hBitmap);
        return 0;
    }
    return 1;
}


/** WindowExtensions::fontToDc()
 *  WindowExtensions::objectToDc()
 *
 *  Selects a graphics object into the specified device context (DC). The new
 *  object replaces the previous object of the same type.
 *
 *  This method should only be used with pen, brush, or font objects.  Although
 *  the operating system can select some other types of graphic objects into a
 *  device context, the restrictions placed on using those objects make it
 *  difficult to provide a correct means of using them through ooDialog.
 *
 *  This method returns the previously selected object of the specified type.
 *  An application should always replace a new object with the original, default
 *  object after it has finished drawing with the new object.
 *
 *  This is done by saving the handle of the returned object and then using
 *  objectToDC() to select the old object back into the device context.
 *
 *  @param  hDC   Handle to the device context receiving the object.
 *  @param  hObj  Handle to the graphics object.
 *
 *  @return  On success a handle to the existing object of the type specified is
 *           returned.  On failure a null pointer is returned.
 *
 *  @note  The operating system does not set the last error code during the
 *         execution of this method, so the .SystemErrorCode has no information
 *         if this method fails.
 *
 *  @remarks  Note that this method does not make sense as a windows extension,
 *            it does not require a valid window handle.
 *
 *            Previously, fontTodc() just called this method.  Now we simply map
 *
 *            ::method fontToDc directly to this method.
 */
RexxMethod2(POINTERSTRING, winex_objectToDC, POINTERSTRING, hDC, POINTERSTRING, hObj)
{
    oodResetSysErrCode(context->threadContext);

    HGDIOBJ hOldObj = SelectObject((HDC)hDC, (HGDIOBJ)hObj);
    if ( hOldObj == NULL )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return hOldObj;
}


/** WindowExtensions::createPen()
 *
 *  Creates a logical pen that has the specified style, width, and color. The
 *  pen can subsequently be selected into a device context and used to draw
 *  lines and curves.
 *
 *  @param  width  [OPTIONAL]  Specifies the width of the pen, in pixels. If
 *                 width is zero, the pen is set to be a single pixel wide.  If
 *                 omitted the width is set to 1.  If you specify a width
 *                 greater than one for the following styles: DASH, DOT,
 *                 DASHDOT, or DASHDOTDOT, the system returns a pen with the
 *                 specified width, but changes the pen style to SOLID.
 *
 *  @param  style  [OPTIONAL]  A keyword that specifies the pen style.  The
 *                 following keywords are valid: SOLID, NULL, DASH, DOT,
 *                 DASHDOT, DASHDOTDOT INSIDEFRAME.  Case is not significant. If
 *                 omitted a solid pen is returned.  Likewise, if the keyword is
 *                 not recognized, a solid pen is returned.
 *
 *  @param  color  [OPTIONAL] Specifies the color for the pen.
 *
 *  @return  The pen specified by the arguments on success, a null pointer on
 *           failure.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(POINTERSTRING, winex_createPen, OPTIONAL_uint32_t, width, OPTIONAL_CSTRING, _style, OPTIONAL_uint32_t, color)
{
    oodResetSysErrCode(context->threadContext);

    width = (argumentOmitted(1) ? 1 : width);
    color = (argumentOmitted(3) ? 0 : color);

    uint32_t style = PS_SOLID;
    if ( argumentExists(2) )
    {
        if (      stricmp(_style, "DASH")        == 0 ) style = PS_DASH;
        else if ( stricmp(_style, "DOT")         == 0 ) style = PS_DOT;
        else if ( stricmp(_style, "DASHDOT")     == 0 ) style = PS_DASHDOT;
        else if ( stricmp(_style, "DASHDOTDOT")  == 0 ) style = PS_DASHDOTDOT;
        else if ( stricmp(_style, "INSIDEFRAME") == 0 ) style = PS_NULL;
        else if ( stricmp(_style, "NULL")        == 0 ) style = PS_NULL;
        else style = PS_SOLID;
    }

    HPEN hPen = CreatePen(style, width, PALETTEINDEX(color));
    if ( hPen == NULL )
    {
        oodSetSysErrCode(context->threadContext);
    }
    return hPen;
}

/** WindowExtensions::createBrush()
 *
 * Retrieves a handle to a graphics brush.  The type of brush is dependent on
 * the supplied arguments.
 *
 * If both args were omitted,then a stock hollow brush is returned.  When only
 * the color arg is specified, then a solid color brush of the color specified
 * is returned.
 *
 * The second argument can either be a keyword to specify a brush pattern, or
 * the file name of a bitmap to use as the brush.
 *
 * @param color           [OPTIONAL]  The color of the brush.  If omitted, the
 *                        default is 1.
 * @param brushSpecifier  [OPTIONAL]  If specified, can be either a keyword for
 *                        the hatch pattern of a brush, or the name of a bitmap
 *                        file to use for the brush.
 *
 * @return The handle to the brush on success, or a null handle on failure.
 *
 * @note  Sets the .SystemErrorCode.
 */
RexxMethod2(POINTERSTRING, winex_createBrush, OPTIONAL_uint32_t, color, OPTIONAL_CSTRING, brushSpecifier)
{
    return oodCreateBrush(context, color, brushSpecifier);
}


/** WindowExtensions::getSysBrush()
 *
 * Retrieves a handle to a logical brush that corresponds to the specified
 * system color index.
 *
 * @param color           [REQUIRED]  The color index of the system brush.  Thsi
 *                        can be the raw numeric value, or one of the system
 *                        color keywords
 *
 * @return The handle to the brush on success, or .nil on failure.
 *
 * @note  System brushes do not need to be, and should NOT be deleted like a
 *        brush returned from the createBrush() method.  But, they can be used
 *        anywhere a brush is needed.
 */
RexxMethod1(RexxObjectPtr, winex_getSysBrush, RexxObjectPtr, color)
{
    RexxObjectPtr result = TheNilObj;
    uint32_t      index;

    if ( getSystemColor(context, color, &index, 1) )
    {
        HBRUSH hBrush = GetSysColorBrush(index);
        if ( hBrush != NULL )
        {
            result = pointer2string(context, hBrush);
        }
    }

    return result;
}


/** WindowExtensions::deleteObject()
 *
 *  Deletes a logical pen, brush, font, bitmap, region, or palette, freeing all
 *  system resources associated with the object.   After the object is deleted,
 *  the specified handle is no longer valid.
 *
 *  @param  hObj  Handle to the graphical object to be deleted.
 *
 *  @return  True on success, false on error.
 *
 *  @note Sets .SystemErrorCode.  Do not delete a drawing object (pen or brush)
 *        while it is still selected into a DC.
 *
 *        When a pattern brush is deleted, the bitmap associated with the brush
 *        is not deleted. The bitmap must be deleted independently.
 */
RexxMethod1(RexxObjectPtr, winex_deleteObject, POINTERSTRING, hObj)
{
    oodResetSysErrCode(context->threadContext);
    if ( DeleteObject((HGDIOBJ)hObj) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheFalseObj;
    }
    return TheTrueObj;
}


/** WindowExtensions::rectangle()
 *
 *  Draws a rectangle, either solid or hollow as specified, within the device
 *  context.
 *
 *  @param  hDC     Handle to the device context.
 *  @param  left    Left x co-ordinate of the rectangle.
 *  @param  top     Top y co-ordinate of the point.
 *  @param  right   Right x co-ordinate of the rectangle.
 *  @param  bottom  Bottom y co-ordinate of the point.
 *  @param  fill    [OPTIONAL]  If specified, the rectangle is drawn filled in,
 *                  or solid, using the current background brush of the device
 *                  context. If omitted, the outline of the rectangle is drawn.
 *
 *  @return  0 on success, 1 for error.
 *
 *  @note  Sets .SystemErrorCode.  If hDC is null, the error code is set to 1
 *         ERROR_INVALID_FUNCTION "Incorrect function."  The system may set
 *         other error codes.
 *
 *  @remarks  TODO allow a .Rectange object for the args.
 */
RexxMethod6(logical_t, winex_rectangle, POINTERSTRING, _hDC, int32_t, left, int32_t, top, int32_t, right, int32_t, bottom,
            OPTIONAL_CSTRING, fill)
{
    oodResetSysErrCode(context->threadContext);

    HDC hDC = (HDC)_hDC;

    if ( hDC != NULL )
    {
        if ( argumentExists(6) )
        {
            if ( Rectangle(hDC, left, top, right, bottom) == 0 )
            {
                goto syserr_out;
            }
        }
        else
        {
            if ( MoveToEx(hDC, left, top, NULL) == 0 )
            {
                goto syserr_out;
            }
            if ( LineTo(hDC, right, top) == 0 )
            {
                goto syserr_out;
            }
            if ( LineTo(hDC, right, bottom) == 0 )
            {
                goto syserr_out;
            }
            if ( LineTo(hDC, left, bottom) == 0 )
            {
                goto syserr_out;
            }
            if ( LineTo(hDC, left, top) == 0 )
            {
                goto syserr_out;
            }
        }
    }
    else
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        goto err_out;
    }
    return 0;

syserr_out:
  oodSetSysErrCode(context->threadContext);

err_out:
    return 1;
}


/** WindowExtensions::fillRect()
 *
 *  Fills a rectangle using the specified brush. within the specified device
 *  context.
 *
 *  @param  hDC     Handle to the device context.
 *  @param  rect    A .Rectangle object that specifies the area to be filled.
 *  @param  hBrush  The handle to the brush to be used, or one of the system
 *                  colors.
 *
 *  @return  True on success, false on error.
 *
 *  @note  Sets .SystemErrorCode.  If hDC is null, the error code is set to 1
 *         ERROR_INVALID_FUNCTION "Incorrect function."  The system may set
 *         other error codes.
 *
 *  @note  Unlike the rectangle(), the fillRect() method does not outline the
 *         rectangle with the pen of the device context.
 *
 */
RexxMethod3(RexxObjectPtr, winex_fillRect, POINTERSTRING, _hDC, RexxObjectPtr, _rect,
            RexxObjectPtr, _hBrush)
{
    oodResetSysErrCode(context->threadContext);
    RexxMethodContext *c = context;
    HBRUSH hBrush = NULL;
    HDC    hDC    = (HDC)_hDC;

    if ( hDC == NULL )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        goto err_out;
    }

    CSTRING tmpBrush = c->ObjectToStringValue(_hBrush);
    if ( isPointerString(tmpBrush) )
    {
        hBrush = (HBRUSH)string2pointer(tmpBrush);
    }
    else
    {
        uint32_t index;

        if ( ! getSystemColor(context, _hBrush, &index, 3) )
        {
            goto err_out;
        }
        hBrush = (HBRUSH)(UINT_PTR)(index + 1); // double cast avoids C4312
    }

    PORXRECT pRect = rxGetRect(context, _rect, 2);
    if ( pRect == NULL )
    {
        goto err_out;
    }

    if ( FillRect(hDC, (const RECT *)pRect, hBrush) )
    {
        return TheTrueObj;
    }

    oodSetSysErrCode(context->threadContext);

err_out:
    return TheFalseObj;
}


/** WindowExtensions::write()
 *
 *
 *  @notes  Sets the .SystemErrorCode.
 *
 *          Note that in one of the lower called functions, drawFontToDC(), the
 *          Windows SelectObject() function does not set the system error code
 *          (according to the docs.)  It is unlikely that it fails, but just for
 *          completedness, we set it to ERROR_SIGNAL_REFUSED if it does fail.
 *
 *  @remarks  This method uses the correct process to create the font.
 *
 *            Note:  In order for the argument positions to match other methods
 *            that call the common oodWriteToWindow() function, xPos must be the
 *            *second* arg.  So, in contrast to most method functions, CSELF is
 *             placed at the front.
 *
 *            The return from write() was never documented in ooRexx 3.2.0, but
 *            it returned 0 for success and 1 for failure.  Even though we use
 *            logical_t for the return, we maintain the 0 for success.
 */
RexxMethod9(logical_t, winex_write, CSELF, pCSelf, int32_t, xPos, int32_t, yPos, CSTRING, text,
            OPTIONAL_CSTRING, fontName, OPTIONAL_uint32_t, fontSize, OPTIONAL_CSTRING, fontStyle,
            OPTIONAL_int32_t, fgColor, OPTIONAL_int32_t, bkColor)
{
    HWND hwnd = winExtSetup(context, pCSelf);
    if ( hwnd == NULL )
    {
        return 1;
    }
    return oodWriteToWindow(context, (HWND)hwnd, xPos, yPos, text, fontName, fontSize, fontStyle, fgColor, bkColor);
}


/** WindowExtensions::drawLine()
 *
 *  Draws a line from (x, y) to (x2, y2) within the device context.
 *
 *  @param  hDC  Handle to the device context.
 *  @param  x    [OPTIONAL] x co-ordinate of start point.  If x or y are omitted
 *                          then the current position in the device context is
 *                          the start point.
 *
 *  @param  y    [OPTIONAL] y co-ordinate of start point.  If x or y
 *                          are omitted then the current position in the device
 *                          context is the start point.
 *  @param  x2              The x co-ordinate of the end point of the line.
 *  @param  y2              The y co-ordinate of the end point of the line.
 *
 *  @return  0 on success, 1 for error.
 *
 *  @note  Sets .SystemErrorCode.  If hDC is null, the error code is set to 1
 *         ERROR_INVALID_FUNCTION "Incorrect function"
 *
 *  @remarks  TODO allow .Point .Point objects for the args.
 */
RexxMethod5(logical_t, winex_drawLine, POINTERSTRING, _hDC, OPTIONAL_int32_t, x, OPTIONAL_int32_t, y, int32_t, x2, int32_t, y2)
{
    oodResetSysErrCode(context->threadContext);

    HDC hDC = (HDC)_hDC;

    if ( hDC != NULL )
    {
        if ( ! (argumentOmitted(1) || argumentOmitted(2)) )
        {
            if ( MoveToEx(hDC, x, y, NULL) == 0 )
            {
                goto syserr_out;
            }
        }

        if ( LineTo(hDC, x2, y2) == 0 )
        {
            goto syserr_out;
        }
    }
    else
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        goto err_out;
    }
    return 0;

syserr_out:
  oodSetSysErrCode(context->threadContext);

err_out:
    return 1;
}


/** WindowExtensions::getPixel()
 *
 *  Gets the color value of the pixel at the specified point in the device
 *  context..
 *
 *  The returned value is a COLORREF, which contains the RGB (red, green, blue)
 *  color.  See the .Image class for a discussion of COLORREFs and some methods
 *  for working with COLORREFs and RGB values.
 *
 *  Note that Microsoft says: A bitmap must be selected within the device
 *  context, otherwise, CLR_INVALID is returned on all pixels.
 *
 *  @param  hDC  Handle to the device context.
 *  @param  x    x co-ordinate of the point.
 *  @param  y    y co-ordinate of the point.
 *
 *  @return  The return value is a COLORREF. If the pixel is outside of the
 *           current clipping region, the return value is CLR_INVALID.
 *
 *  @note  Sets .SystemErrorCode.  The only error code set, is 1
 *         ERROR_INVALID_FUNCTION "Incorrect function" which is set if the
 *         device context handle is invalid.  For other errors check if the
 *         return is CLR_INVALID. You can use the .Image class to do this.  Code
 *         might be something like:
 *
 *         color = self~getPixe(hdc, 10, 10)
 *         if color == .Image~colorRef(CLR_INVALID) then do
 *           -- some error recovery
 *         end
 *
 *  @remarks  TODO allow .Point object for the args.
 */
RexxMethod3(uint32_t, winex_getPixel, POINTERSTRING, _hDC, int32_t, x, int32_t, y)
{
    oodResetSysErrCode(context->threadContext);

    HDC hDC = (HDC)_hDC;
    COLORREF color = CLR_INVALID;

    if ( hDC != NULL )
    {
        color = GetPixel(hDC, x, y);
    }
    else
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
    }
    return color;
}


/** WindowExtensions::drawPixel()
 *
 *  Draws a pixel at the specified point using the specified color.
 *
 *  @param  x     x co-ordinate of the point.
 *  @param  y     y co-ordinate of the point.
 *  @param color  The color of the pixel being drawn.
 *
 *  @return  On success, the RGB color (a COLORREF) the system used to
 *           draw the pixel.  This might not be the color specified, if the
 *           system could not find an exact match for the color in the device.
 *           On failure 1 is returned.
 *
 *  @note  Sets .SystemErrorCode.  If hDC is null, the error code is set to 1
 *         ERROR_INVALID_FUNCTION "Incorrect function."  Other error codes may
 *         be set by the system.
 *
 *  @remarks  TODO allow .Point object for the args.
 */
RexxMethod4(uint32_t, winex_drawPixel, POINTERSTRING, _hDC, int32_t, x, int32_t, y, int32_t, color)
{
    oodResetSysErrCode(context->threadContext);

    HDC hDC = (HDC)_hDC;
    COLORREF newColor = 1;

    if ( hDC != NULL )
    {
        newColor = SetPixel(hDC, x, y, PALETTEINDEX(color));
        if ( newColor == 1 )
        {
            goto syserr_out;
        }
    }
    else
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
    }
    goto done_out;

syserr_out:
  oodSetSysErrCode(context->threadContext);

done_out:
    return newColor;
}


/** WindowExtensions::fillDrawing()
 *
 *  Fills an area of the display surface with the current brush. The area is
 *  assumed to be bounded as specified by the fillTo parameter.
 *
 *  The area to be filled must be totally bounded by the fillTo color.  The
 *  system starts filling at the point specified and continues filling outwards
 *  in all directions until it reaches the boundry.
 *
 *  @param  x       x co-ordinate of the starting point.
 *  @param  y       y co-ordinate of the starting point.
 *  @param  fillTo  The color of the border of the filled area.
 *
 *  @note  Sets .SystemErrorCode.  If hDC is null, the error code is set to 1
 *         ERROR_INVALID_FUNCTION "Incorrect function."  Other error codes may
 *         be set by the system.
 *
 *  @remarks  TODO allow .Point object for the args.   TODO use ExtFloodFill()
 *            instead.
 */
RexxMethod4(logical_t, winex_fillDrawing, POINTERSTRING, _hDC, int32_t, x, int32_t, y, uint32_t, color)
{
    oodResetSysErrCode(context->threadContext);

    HDC hDC = (HDC)_hDC;
    COLORREF clr = color;

    if ( color >= 0 && color <= 18 )
    {
        clr = PALETTEINDEX(color);
    }

    if ( hDC != NULL )
    {
        if ( FloodFill(hDC, x, y, clr) == 0 )
        {
            goto syserr_out;
        }
    }
    else
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        goto err_out;
    }
    return TRUE;

syserr_out:
  oodSetSysErrCode(context->threadContext);

err_out:
    return FALSE;
}


/** WindowExtensions::drawArc()
 *  WindowExtensions::drawPie()
 *
 *  drawArc() method:
 *
 *    Draws an elliptical arc.  The elliptical arc is defined the same way the
 *    pie is, but only the arc is drawn, the radials are not drawn.
 *
 *  drawPie() method:
 *
 *    Draws a pie-shaped wedge bounded by the intersection of an ellipse and two
 *    radials. The pie is outlined by using the current pen and filled by using
 *    the current brush.
 *
 *  Both methods take the same arguements:
 *
 *  @param  left    x co-ordinate of the top corner of the bounding rectangle.
 *  @param  top     y co-ordinate of the top corner of the bounding rectangle.
 *  @param  right   x co-ordinate of the bottom corner of the bounding
 *                  rectangle.
 *  @param  bottom  y co-ordinate of the bottom corner of the bounding
 *                  rectangle.
 *  @param  startX  [OPTIONAL] x co-ordinate of the first radial ending point.
 *  @param  startY  [OPTIONAL] y co-ordinate of the first radial ending point.
 *  @param  endX    [OPTIONAL] x co-ordinate of second radial ending point.
 *  @param  endY    [OPTIONAL] y co-ordinate of second radial ending point.
 *
 *  @note  The optional arguements above all default to 0
 *
 *  @return  If the method succeeds, the return is 0.  On failure the return is
 *           1.
 *
 *  @note  Sets .SystemErrorCode.  If hDC is null, the error code is set to 1
 *         ERROR_INVALID_FUNCTION "Incorrect function."  Other error codes may
 *         be set by the system.
 *
 *  @remarks  TODO allow for .Rect and .Point objects for the args.
 */
RexxMethod10(logical_t, winex_drawArcOrPie, POINTERSTRING, _hDC, int32_t, left, int32_t, top, int32_t, right, int32_t, bottom,
            OPTIONAL_int32_t, startX, OPTIONAL_int32_t, startY, OPTIONAL_int32_t, endX, OPTIONAL_int32_t, endY, NAME, method)
{
    oodResetSysErrCode(context->threadContext);

    HDC hDC = (HDC)_hDC;

    if ( hDC != NULL )
    {
        logical_t result;
        if ( method[4] == 'A' )
        {
            result = Arc(hDC, left, top, right, bottom, startX, startY, endX, endY);
        }
        else
        {
            result = Pie(hDC, left, top, right, bottom, startX, startY, endX, endY);
        }

        if ( result == 0 )
        {
            goto syserr_out;
        }
    }
    else
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        goto err_out;
    }
    return 0;

syserr_out:
  oodSetSysErrCode(context->threadContext);

err_out:
    return 1;
}


/** WindowExtensions::drawAngleArc()
 *
 *  Draws a line segment and an arc. The line segment is drawn from the current
 *  position to the beginning of the arc. The arc is drawn along the perimeter
 *  of a circle with the given radius and center. The length of the arc is
 *  defined by the given start and sweep angles.
 *
 *  If the optional moveToX and moveToY are both specified, then the current
 *  positions if first set to the point specified by those values.
 *
 *  @param  hDC         Handle to the device context.
 *  @param  moveToX     [OPTIONAL] x co-ordinate of start point.  If moveToX or
 *                      moveToY are omitted then the current position in the
 *                      device context is the start point.
 *
 *  @param  moveToY     [OPTIONAL] y co-ordinate of start point.  If moveToX or
 *                      moveToY  are omitted then the current position in the
 *                      device context is the start point.
 *  @param  x           The x co-ordinate of the center point of the circle.
 *  @param  y           The y co-ordinate of the center point of the circle.
 *  @param  radius      The radius of the circle.  This must be positive.
 *  @param  startAngle  Specifies the start angle, in degrees, relative to the
 *                      x-axis.
 *  @param  sweepAngle  Specifies the sweep angle, in degrees, relative to the
 *                      starting angle.
 *
 *  @return  0 on success, 1 for error.
 *
 *  @note  Sets .SystemErrorCode.  If hDC is null, the error code is set to 1
 *         ERROR_INVALID_FUNCTION "Incorrect function"
 *
 *  @remarks  TODO allow .Point .Point objects for the args.
 */
RexxMethod8(logical_t, winex_drawAngleArc, POINTERSTRING, _hDC, OPTIONAL_int32_t, moveToX, OPTIONAL_int32_t, moveToY, int32_t, x, int32_t, y,
            uint32_t, radius, float, startAngle, float, sweepAngle)
{
    oodResetSysErrCode(context->threadContext);

    HDC hDC = (HDC)_hDC;

    if ( hDC != NULL )
    {
        if ( argumentExists(1) && argumentExists(2) )
        {
            if ( MoveToEx(hDC, moveToX, moveToY, NULL) == 0 )
            {
                goto syserr_out;
            }
        }

        if ( AngleArc(hDC, x, y, radius, startAngle, sweepAngle) == 0 )
        {
            goto syserr_out;
        }
    }
    else
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        goto err_out;
    }
    return 0;

syserr_out:
  oodSetSysErrCode(context->threadContext);

err_out:
    return 1;
}


/** WindowExtensions::fontColor()
 *
 *  Sets the font color in a device context.
 *
 *  @param  color  [OPTIONAL] The color index for the font color.  If this is
 *                 omitted, the index is set to 1.
 *  @param  hDC    Handle to the device context.
 *
 *  @return  0 on success, 1 on failure.
 *
 *  @note  Sets .SystemErrorCode.  If hDC is null, the error code is set to 1
 *         ERROR_INVALID_FUNCTION "Incorrect function."  Other error codes may
 *         be set by the system.
 */
RexxMethod2(logical_t, winex_fontColor, OPTIONAL_int32_t, color, POINTERSTRING, _hDC)
{
    oodResetSysErrCode(context->threadContext);

    HDC hDC = (HDC)_hDC;
    if ( hDC == NULL )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        goto err_out;
    }

    if ( SetTextColor(hDC, PALETTEINDEX(color)) == CLR_INVALID )
    {
        goto syserr_out;
    }
    return 0;

syserr_out:
  oodSetSysErrCode(context->threadContext);

err_out:
    return 1;
}


/** WindowExtensions::transparentText()
 *  WindowExtensions::opaqueText()ext
 *
 *  Sets the background mix mode of the specified device context. The background
 *  mix mode is used with text, hatched brushes, and pen styles that are not
 *  solid lines.
 *
 *  Note that traditionally ooDialog had not documented that the background mix
 *  mode also affects hatched brushes and some pen styles.  This is the reason
 *  for the method names.  Nevertheless, this has always been the case.
 *
 *  @param  hDC    Handle to the device context.
 *
 *  @return  0 on success, 1 on failure.
 *
 *  @note  Sets .SystemErrorCode.  If hDC is null, the error code is set to 1
 *         ERROR_INVALID_FUNCTION "Incorrect function."  Other error codes may
 *         be set by the system.
 */
RexxMethod2(logical_t, winex_textBkMode, POINTERSTRING, _hDC, NAME, method)
{
    oodResetSysErrCode(context->threadContext);

    HDC hDC = (HDC)_hDC;
    if ( hDC == NULL )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        goto err_out;
    }

    COLORREF clr;
    if ( *method == 'T' )
    {
        clr = SetBkMode(hDC, TRANSPARENT);
    }
    else
    {
        clr = SetBkMode(hDC, OPAQUE);
    }

    if ( clr == CLR_INVALID )
    {
        goto syserr_out;
    }
    return 0;

syserr_out:
  oodSetSysErrCode(context->threadContext);

err_out:
    return 1;
}


/** WindowExtensions::getArcDirection()
 *
 *  Gets the drawing direction be used for arc and rectangle functions.
 *
 *  @param  hDC    Handle to the device context.
 *
 *  @return  A string, CLOCKWISE or COUNTERCLOCKWISE, or the empty string on
 *           error.
 *
 *  @note  Sets .SystemErrorCode.  If hDC is null, the error code is set to 1
 *         ERROR_INVALID_FUNCTION "Incorrect function."  Other error codes may
 *         be set by the system.
 *
 *  -------------------------------------------------------------------
 *
 *  WindowExtensions::setArcDirection()
 *
 *  Sets the drawing direction to be used for arc and rectangle functions.
 *
 *  @param  hDC    Handle to the device context.
 *
 *  @return  A string, the old direction, on success, or the empty string on
 *           error.  On success, the string will be CLOCKWISE or
 *           COUNTERCLOCKWISE, depending on what the old direction was.
 *
 *  @note  Sets .SystemErrorCode.  If hDC is null, the error code is set to 1
 *         ERROR_INVALID_FUNCTION "Incorrect function."  Other error codes may
 *         be set by the system.
 *
 *  @remarks  The default arc direction in Windows is counterclockwise.
 */
RexxMethod3(CSTRING, winex_getSetArcDirection, POINTERSTRING, _hDC, OPTIONAL_CSTRING, _direction, NAME, method)
{
    oodResetSysErrCode(context->threadContext);

    HDC hDC = (HDC)_hDC;
    if ( hDC == NULL )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_FUNCTION);
        goto err_out;
    }

    int direction = 0;
    if ( *method == 'G' )
    {
        direction = GetArcDirection(hDC);
    }
    else
    {
        direction = AD_COUNTERCLOCKWISE;
        if ( _direction != NULLOBJECT && StrStrI(_direction, "COUNTER") == NULL )
        {
            direction = AD_CLOCKWISE;
        }
        direction = SetArcDirection(hDC, direction);
    }

    if ( direction == AD_CLOCKWISE )
    {
        return "CLOCKWISE";
    }
    else if ( direction == AD_COUNTERCLOCKWISE )
    {
        return "COUNTERCLOCKWISE";
    }

    oodSetSysErrCode(context->threadContext);

err_out:
    return "";
}


/**
 *  Methods for the .CreateWindow class.
 */
#define CREATEWINDOW_CLASS      "CreateWindow"


static pCCreateWindows validateCwCSelf(RexxMethodContext *c, void *pCSelf)
{
    pCCreateWindows pccw = (pCCreateWindows)pCSelf;
    if ( pccw == NULL )
    {
        baseClassInitializationException(c, "CreateWindow");
    }
    else
    {
        if ( pccw->hDlg == NULL )
        {
            if ( pccw->wndBase->hwnd == NULL )
            {
                noWindowsDialogException(c, pccw->rexxDlg);
            }
            else
            {
                pccw->hDlg = pccw->wndBase->hwnd;
            }
        }
    }

    return pccw;
}


bool initCreateWindows(RexxMethodContext *c, RexxObjectPtr self, pCPlainBaseDialog pcpbd)
{
    RexxBufferObject obj = c->NewBuffer(sizeof(CCreateWindows));
    if ( obj == NULLOBJECT )
    {
        return false;
    }

    pCCreateWindows pccw = (pCCreateWindows)c->BufferData(obj);
    pccw->rexxDlg  = self;
    pccw->wndBase  = pcpbd->wndBase;;
    pccw->hDlg     = NULL;
    pccw->hinst    = MyInstance;

    c->SendMessage1(self, "INITCREATEWINDOWS", obj);

    return true;
}


/** CreateWindows::initCreateWindows()
 *
 */
RexxMethod1(logical_t, cw_initCreateWindows, RexxObjectPtr, cSelf)
{
    if ( ! context->IsBuffer(cSelf) )
    {
        wrongClassException(context->threadContext, 1, "Buffer");
        return FALSE;
    }

    context->SetObjectVariable("CSELF", cSelf);
    return TRUE;
}


/** CreateWindows::createReBarWindow()
 *
 *  Creates a Windows rebar and returns the Rexx ReBar object.
 *
 *  @param id     [required] The resource ID of the rebar.
 *
 *  @param sytle  [optional] Style keywords.  If omitted the rebar uses the
 *                default rebar style of visible, tabstop, variable height, band
 *                borders, and common control no divider style.
 *
 *  @notes Sets the .SystemErrorCode
 *
 *         rebars ignore the position and size co-ordinates, so there is no
 *         arguments for them.
 *
 *  @remarks  The rebar control seems to remove the WS_BORDER style when it is
 *            created.  The WS_TABSTOP style seems to have no effect, behavior
 *            is the same with or without it.  However, tabbing does *not* work
 *            if the control does not have the WS_EX_CONTROLPARENT styles. The
 *            WS_EX_TOOLWINDOW style is used here because several of the MSDN
 *            samples use it.  But, not sure if it is really needed or
 *            appropriate.
 */
RexxMethod3(RexxObjectPtr, cw_createReBarWindow, RexxObjectPtr, rxID, OPTIONAL_CSTRING, _style, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr result = TheNilObj;
    pCCreateWindows pccw =  validateCwCSelf(context, pCSelf);
    if ( pccw == NULL )
    {
        goto done_out;
    }

    uint32_t id = oodResolveSymbolicID(context, pccw->rexxDlg, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto done_out;
    }

    uint32_t style = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;
    if ( argumentExists(2) )
    {
        style |= getControlStyle(winReBar, _style);
    }
    else
    {
        style |= WS_VISIBLE | WS_TABSTOP | RBS_VARHEIGHT | RBS_BANDBORDERS | CCS_NODIVIDER;
    }

    HWND hRebar = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_CONTROLPARENT, REBARCLASSNAME, NULL, style,
                                 0, 0, 0, 0, pccw->hDlg, (HMENU)(UINT_PTR)id, pccw->hinst, NULL); // double cast avoids C4312
    if ( hRebar == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        goto done_out;
    }

    RexxClassObject rxClass = oodClass4controlType(winReBar, context);
    if ( rxClass == NULLOBJECT )
    {
        goto done_out;
    }

    result = createRexxControl(context->threadContext, hRebar, pccw->hDlg, id, winReBar, pccw->rexxDlg,
                               rxClass, false, true);

done_out:
    return result;
}


/** CreateWindows::createStatusBarWindow()
 *
 *  Creates a Windows status bar and returns the Rexx StatusBar object.
 *
 *  @param id     [required] The resource ID of the status bar.
 *
 *  @param sytle  [optional] Style keywords.  If omitted the status bar uses
 *                the default status bar style of visible, sizing grep, tool
 *                tips enabled.
 *
 *  @notes Sets the .SystemErrorCode
 *
 *         Status bars ignore the position and size co-ordinates, so there is
 *         no arguments for them.
 */
RexxMethod3(RexxObjectPtr, cw_createStatusBarWindow, RexxObjectPtr, rxID, OPTIONAL_CSTRING, _style, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr result = TheNilObj;
    pCCreateWindows pccw =  validateCwCSelf(context, pCSelf);
    if ( pccw == NULL )
    {
        goto done_out;
    }

    uint32_t id = oodResolveSymbolicID(context, pccw->rexxDlg, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto done_out;
    }

    uint32_t style = WS_CHILD;
    if ( argumentExists(2) )
    {
        style |= getControlStyle(winStatusBar, _style);
    }
    else
    {
        style |= SBARS_SIZEGRIP | SBARS_TOOLTIPS | WS_VISIBLE;
    }

    HWND hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, style, 0, 0, 0, 0, pccw->hDlg,
                                  (HMENU)(UINT_PTR)id, pccw->hinst, NULL); // double cast avoids C4312
    if ( hStatus == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        goto done_out;
    }

    RexxClassObject rxClass = oodClass4controlType(winStatusBar, context);
    if ( rxClass == NULLOBJECT )
    {
        goto done_out;
    }

    result = createRexxControl(context->threadContext, hStatus, pccw->hDlg, id, winStatusBar, pccw->rexxDlg,
                               rxClass, false, true);

done_out:
    return result;
}


/** CreateWindows::createToolBarWindow()
 *
 *  Creates a Windows toolbar and returns the Rexx ToolBar object.
 *
 *  @param id     [required] The resource ID of the toolbar.
 *
 *  @param sytle  [optional] Style keywords.  If omitted the toolbar uses the
 *                default toolbar style of visible, border, tabstop, and
 *                wrappable.
 *
 *  @notes Sets the .SystemErrorCode
 *
 *  @remarks  Originally we were going to have the size and position arguments.
 *            However, after testing that, the size and position arguments are
 *            ignored just like they are are in status bars and rebars.
 */
RexxMethod3(RexxObjectPtr, cw_createToolBarWindow, RexxObjectPtr, rxID, OPTIONAL_CSTRING, _style, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    RexxObjectPtr result = TheNilObj;
    pCCreateWindows pccw =  validateCwCSelf(context, pCSelf);
    if ( pccw == NULL )
    {
        goto done_out;
    }

    uint32_t id = oodResolveSymbolicID(context, pccw->rexxDlg, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto done_out;
    }

    uint32_t style = WS_CHILD;
    if ( argumentExists(2) )
    {
        style |= getControlStyle(winToolBar, _style);
    }
    else
    {
        style |= WS_VISIBLE | WS_BORDER | WS_TABSTOP | TBSTYLE_WRAPABLE;
    }

    HWND hToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, style, 0, 0, 0, 0, pccw->hDlg,
                                  (HMENU)(UINT_PTR)id, pccw->hinst, NULL); // double cast avoids C4312
    if ( hToolbar == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        goto done_out;
    }

    RexxClassObject rxClass = oodClass4controlType(winToolBar, context);
    if ( rxClass == NULLOBJECT )
    {
        goto done_out;
    }

    result = createRexxControl(context->threadContext, hToolbar, pccw->hDlg, id, winToolBar, pccw->rexxDlg,
                               rxClass, false, true);

done_out:
    return result;
}



/**
 *  Methods for the .CustomDraw class.
 */
#define CUSTOMDRAW_CLASS        "CustomDraw"

/**
 * Ensures that the passed pointer is not null and is a pointer to a
 * CEventNotification struct.
 *
 * @param pCSelf  Pointer to check
 *
 * @return pCEventNotification
 */
static pCEventNotification ensureEventNotification(RexxMethodContext *c, void *pCSelf)
{
    if ( pCSelf != NULL && *(((uint32_t *)pCSelf)) == EVENTNOTIFICATION_MAGIC )
    {
        return (pCEventNotification)pCSelf;
    }

    baseClassInitializationException(c, "CustomDraw");
    return NULL;
}

/**
 * Ensures that the passed pointer is not null and is a pointer to a
 * CCustomDraw struct.
 *
 * @param pCSelf  Pointer to check
 *
 * @return pCCustomDraw
 */
static pCCustomDraw ensureCustomDraw(RexxMethodContext *c, void *pCSelf)
{
    if ( pCSelf != NULL && *(((uint32_t *)pCSelf)) == CUSTOMDRAW_MAGIC )
    {
        return (pCCustomDraw)pCSelf;
    }

    baseClassInitializationException(c, "CustomDraw");
    return NULL;
}

/**
 *
 *
 *
 * @param pcpbd
 *
 * @note  This function must be called from the Window procedure of the dialog
 *        represented by pcpbd.  The needed thread context is taken from pcpbd
 *        as the dlgProcContext.  Calling this function from a different thread
 *        context will cause the interpreter to throw a
 *        NativeActivationException, which, in ooDialog, appears to cause a
 *        hang.
 */
void customDrawCheckIDs(pCPlainBaseDialog pcpbd)
{
    pcpbd->idsNotChecked = false;

    pCEventNotification pcen = pcpbd->enCSelf;
    if ( pcen->pCustomDraw == NULL )
    {
        pcpbd->badIDs = true;

        baseClassInitializationException(pcpbd->dlgProcContext, "CustomDraw");
        checkForCondition(pcpbd->dlgProcContext, false);
        endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);

        return;
    }

    pCCustomDraw pccd = (pCCustomDraw)pcen->pCustomDraw;
    for ( uint32_t i = 0; i < pccd->count; i++ )
    {
        if ( ! isControlMatch(pcpbd->hDlg, pccd->ids[i], pccd->types[i]) )
        {
            pcpbd->badIDs = true;

            customDrawMismatchException(pcpbd->dlgProcContext, pccd->ids[i], pccd->types[i]);
            checkForCondition(pcpbd->dlgProcContext, false);
            endDialogPremature(pcpbd, pcpbd->hDlg, RexxConditionRaised);

            return;
        }
    }

    return;
}

/** CustomDraw::customDraw
 *
 *  Initializes the custom draw interface.  The user must call this method
 *  before envoking any other method of the CustomDraw interface.  Failure to do
 *  so results in error conditions being raised.
 *
 *  @return  True on success, false on error.
 *
 *  @note  class 'CustomDraw' public mixinclass EventNotification  So, when we
 *         get here, CSELF is the event notification class C self.  We check
 *         that it is, and end on error if it isn't.  We then replace the event
 *         notification with our own C self.
 */
RexxMethod2(RexxObjectPtr, cd_init, OSELF, self, CSELF, pCSelf)
{
    pCEventNotification pcen = ensureEventNotification(context, pCSelf);
    if ( pcen == NULL )
    {
        return TheFalseObj;
    }

    RexxBufferObject obj = context->NewBuffer(sizeof(CCustomDraw));
    if ( obj == NULLOBJECT )
    {
        outOfMemoryException(context->threadContext);
        return TheFalseObj;
    }

    pCCustomDraw pccd = (pCCustomDraw)context->BufferData(obj);
    memset(pccd, 0, sizeof(pCCustomDraw));

    pccd->magic    = CUSTOMDRAW_MAGIC;
    pccd->enCSelf  = pcen;
    pccd->rexxSelf = self;

    pcen->pCustomDraw = pccd;

    context->SetObjectVariable("CSELF", obj);

    return TheTrueObj;
}

/** CustomDraw::customDrawControl()
 *
 * Specifies that the programmer will custom draw the named control.
 *
 * @param  id   Resource ID of the control, may be numeric or symbolic.  A
 *              syntax condition is raised if the ID can not be resolved.
 *
 * @param controlName  [optional] The name of the type of control.  I.e.,
 *                     treeView, TrackBar, LISTVIEW.  By default the control is
 *                     assumed to be a ListView.
 *
 * @param methName     [optional]  The name of the method to be invoked for the
 *                     custom draw event.  If omitted it defaults to
 *                     onCustomDraw().
 *
 * @param flags        [optional]  Reserverd for future use.  One use will be to
 *                     specify 'simple' custom draw.  Right now that is all that
 *                     is implemented, so we don't check this argument.
 *
 * @notes  We are only supporting ListView right now, other controls to be added
 *         later.
 *
 *         We can not, always, tell at this point if the control specified by
 *         the user is really the type of control she says it is.  During the
 *         processing of the custom draw events, if it is not, the interpreter
 *         will likely crash.
 *
 *         What we do is keep track of the control IDs matched to the type the
 *         user says they are.  Then once the underlying dialog is created, we
 *         will check all the IDs at once before we process any NM_CUSTOMDRAW
 *         message. If there is a mismatch we abort the dialog.  Flags in the
 *         PlainBase dialog CSelf, keep track of whether the IDs have been
 *         checked, so that we only do it once.
 *
 *         However, although it makes the most sense for the user to specify all
 *         controls which will be custom drawn before the dialog starts, there
 *         is probably a use case for starting to custom draw a control at any
 *         point in the life cycle of the dialog.
 *
 *         Therefore we allow the user to add custom drawing for a control at
 *         any time. Because of this, when customDrawControl() is invoked we
 *         need to check whether the IDs have been checked already.  If they
 *         have, the underlying dialog must exist, and for this invocation we
 *         can do the check immediately.
 */
RexxMethod5(RexxObjectPtr, cd_customDrawControl, RexxObjectPtr, rxID, OPTIONAL_CSTRING, controlName,
            OPTIONAL_CSTRING, methName, OPTIONAL_CSTRING, flags, CSELF, pCSelf)
{
    pCCustomDraw pccd = ensureCustomDraw(context, pCSelf);
    if ( pccd == NULL )
    {
        goto err_out;
    }

    pCEventNotification pcen = pccd->enCSelf;

    int32_t id = oodResolveSymbolicID(context, pcen->rexxSelf, rxID, -1, 1, true);
    if ( id == OOD_ID_EXCEPTION )
    {
        goto err_out;
    }

    if ( pccd->count >= MAXCUSTOMDRAWCONTROLS )
    {
        char buffer[256];
        _snprintf(buffer, sizeof(buffer),
                  "the number of custom draw controls has reached the maximum (%d) allowed",
                  MAXCUSTOMDRAWCONTROLS);

        userDefinedMsgException(context, buffer);
        goto err_out;
    }


    // Simple will be an option in the future, but for now it is the only thing
    // implemented.
    uint32_t     tag         = TAG_CUSTOMDRAW | TAG_CD_SIMPLE;
    oodControl_t controlType = controlName2controlType(controlName);

    if ( argumentExists(2) )
    {
        switch ( controlType )
        {
            case winListView :
                tag |= TAG_CD_LISTVIEW;
                break;

            case winTreeView :
                tag |= TAG_CD_TREEVIEW;
                break;

            default :
                wrongArgValueException(context->threadContext, 2, "ListView, TreeView", controlName);
                goto err_out;
        }
    }
    else
    {
        tag |= TAG_CD_LISTVIEW;
    }

    pccd->types[pccd->count] = controlType;
    pccd->ids[pccd->count++] = id;

    pCPlainBaseDialog pcpbd = dlgToCSelf(context, pcen->rexxSelf);
    if ( ! pcpbd->idsNotChecked )
    {
        if ( ! isControlMatch(pcpbd->hDlg, id, controlType) )
        {
            pcpbd->badIDs = true;
            customDrawMismatchException(context->threadContext, id, controlType);
            goto err_out;
        }
    }

    if ( argumentOmitted(3) )
    {
        methName = "onCustomDraw";
    }

    if ( addNotifyMessage(pcen, context, id, 0xFFFFFFFF, NM_CUSTOMDRAW, 0xFFFFFFFF, methName, tag) )
    {
        return TheZeroObj;
    }

err_out:
    return TheOneObj;
}


