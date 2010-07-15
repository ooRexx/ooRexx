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
 * ooDialog.cpp
 *
 * The base module for the ooDialog package.  Contains the method implmentations
 * for the WindowBase and PlainBaseDialog classes.
 */
#include "ooDialog.hpp"     // Must be first, includes windows.h and oorexxapi.h

#include <mmsystem.h>
#include <commctrl.h>
#include <stdio.h>
#include <dlgs.h>
#include <shlwapi.h>
#include "APICommon.hpp"
#include "oodCommon.hpp"
#include "oodControl.hpp"
#include "oodMessaging.hpp"
#include "oodData.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodResourceIDs.hpp"


/**
 * Loads and returns the handle to an icon for the specified ID, of the
 * specified size.
 *
 * The icons can come from the user resource DLL, a user defined dialog, the
 * OODialog DLL, the System.  IDs for the icons bound to the OODialog.dll are
 * reserved.
 *
 * @param pcpbd     Pointer to the CSelf struct for the dialog
 * @param id        Numerical resource ID.
 * @param iconSrc   Flag indicating the source of the icon.
 * @param cx        The desired width of the icon.
 * @param cy        The desired height of the icon.
 *
 * @return The handle to the loaded icon on success, or null on failure.
 */
static HICON getIconForID(pCPlainBaseDialog pcpbd, UINT id, UINT iconSrc, int cx, int cy)
{
    HINSTANCE hInst = NULL;
    LPCTSTR   pName = NULL;
    UINT      loadFlags = 0;

    if ( iconSrc & ICON_FILE )
    {
        // Load the icon from a file, file name should be in the icon table.
        size_t i;

        for ( i = 0; i < pcpbd->IT_size; i++ )
        {
            if ( pcpbd->IconTab[i].iconID == id )
            {
                pName = pcpbd->IconTab[i].fileName;
                break;
            }
        }

        if ( ! pName )
        {
            return NULL;
        }
        loadFlags = LR_LOADFROMFILE;
    }
    else
    {
        // Load the icon from the resources in oodialog.dll or the user's resource DLL.
        hInst = (iconSrc & ICON_OODIALOG) ? MyInstance : pcpbd->hInstance;

        pName = MAKEINTRESOURCE(id);
        loadFlags = LR_SHARED;
    }

    return (HICON)LoadImage(hInst, pName, IMAGE_ICON, cx, cy, loadFlags);
}

static void deleteMessageTables(pCEventNotification pcen)
{
    size_t i;

    for ( i = 0; i < pcen->cmSize; i++ )
    {
        safeLocalFree(pcen->commandMsgs[i].rexxMethod);
    }
    LocalFree(pcen->commandMsgs);
    pcen->commandMsgs = NULL;
    pcen->cmSize = 0;

    if ( pcen->notifyMsgs != NULL )
    {
        for ( i = 0; i < pcen->nmSize; i++ )
        {
            safeLocalFree(pcen->notifyMsgs[i].rexxMethod);
        }
        LocalFree(pcen->notifyMsgs);
        pcen->nmSize = 0;
        pcen->notifyMsgs = NULL;
    }

    if ( pcen->miscMsgs != NULL )
    {
        for ( i = 0; i < pcen->mmSize; i++ )
        {
            safeLocalFree(pcen->miscMsgs[i].rexxMethod);
        }
        LocalFree(pcen->miscMsgs);
        pcen->mmSize = 0;
        pcen->miscMsgs = NULL;
    }
}


HBRUSH searchForBrush(pCPlainBaseDialog pcpbd, size_t *index, uint32_t id)
{
    HBRUSH hBrush = NULL;
    size_t i = 0;

    if ( pcpbd != NULL && pcpbd->ColorTab != NULL )
    {
        while ( i < pcpbd->CT_size && pcpbd->ColorTab[i].itemID != id )
        {
           i++;
        }
        if ( i < pcpbd->CT_size )
        {
            hBrush = pcpbd->ColorTab[i].ColorBrush;
            *index = i;
        }
    }
    return hBrush;
}

bool loadResourceDLL(pCPlainBaseDialog pcpbd, CSTRING library)
{
    pcpbd->hInstance = LoadLibrary(library);
    if ( ! pcpbd->hInstance )
    {
        CHAR msg[256];
        sprintf(msg,
                "Failed to load Dynamic Link Library (resource DLL.)\n"
                "  File name:\t\t\t%s\n"
                "  Windows System Error Code:\t%d\n", library, GetLastError());
        MessageBox(0, msg, "ooDialog DLL Load Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
        return false;
    }
    return true;
}

/**
 * Do some common set up when creating the underlying Windows dialog for any
 * ooDialog dialog.  This involves setting the 'TopDlg' and the hInstance
 * fields of the CSelf struct for the dialog.
 *
 * If this is a ResDialog then the resource DLL is loaded, otherwise the
 * hInstance field is the ooDialog.dll instance.
 *
 * @param pcpbd    Pointer to the CSelf struct for the dialog
 * @param library  The library to load the dialog from, if a ResDialog, othewise
 *                 null.
 *
 * @return True on success, false only if this is for a ResDialog and the
 *         loading of the resource DLL failed.
 */
bool installNecessaryStuff(pCPlainBaseDialog pcpbd, CSTRING library)
{
    if ( pcpbd->previous )
    {
        ((pCPlainBaseDialog)pcpbd->previous)->onTheTop = false;
    }
    TopDlg = pcpbd;

    if ( library != NULL )
    {
        return loadResourceDLL(pcpbd, library);
    }
    else
    {
        pcpbd->hInstance = MyInstance;
    }

    return true;
}


inline bool dlgInDlgTable(pCPlainBaseDialog dlg)
{
    register size_t i;
    for ( i = 0; i < CountDialogs; i++ )
    {
        if ( DialogTable[i] == dlg )
        {
           return true;
        }
    }
    return false;
}


/**
 * Makes sure the finished attribute in the Rexx dialog is set to true.  This
 * causes the Rexx dialog's run method to complete and things to unwind.
 *
 * @param pcpbd   CSelf struct for the dialog.
 * @param c       Rexx thread context we are operating in.  It is possible that
 *                this is null.
 *
 * @remarks
 */
void ensureFinished(pCPlainBaseDialog pcpbd, RexxThreadContext *c, RexxObjectPtr abnormal)
{
    if ( c != NULL )
    {
        c->SendMessage1(pcpbd->rexxSelf, "ENSUREFINISHED", abnormal);
    }
    else
    {
        RexxThreadContext *context;

        if ( pcpbd->interpreter->AttachThread(&context) )
        {
            context->SendMessage1(pcpbd->rexxSelf, "ENSUREFINISHED", abnormal);

            context->DetachThread();
        }
    }
}

void delPageDialog(pCPropertySheetPage pcpsp)
{
    if ( pcpsp->pageTitle != NULL )
    {
        LocalFree(pcpsp->pageTitle);
        pcpsp->pageTitle = NULL;
    }
    if ( pcpsp->headerTitle != NULL )
    {
        LocalFree(pcpsp->headerTitle);
        pcpsp->headerTitle = NULL;
    }
    if ( pcpsp->headerSubtitle != NULL )
    {
        LocalFree(pcpsp->headerSubtitle);
        pcpsp->headerSubtitle = NULL;
    }
}

void delPropSheetDialog(pCPropertySheetDialog pcpsd)
{
    if ( pcpsd->cppPages != NULL )
    {
        LocalFree(pcpsd->cppPages);
        pcpsd->cppPages = NULL;
    }
    if ( pcpsd->rexxPages != NULL )
    {
        LocalFree(pcpsd->rexxPages);
        pcpsd->rexxPages = NULL;
    }
    if ( pcpsd->caption != NULL )
    {
        LocalFree(pcpsd->caption);
        pcpsd->caption = NULL;
    }
    pcpsd->pageCount = 0;
}

/**
 * Ends a running dialog and cleans up some (most) of the CSelf struct.
 *
 * Note that this function is at times called when the underlying Windows dialog
 * was not created, and therefore there is no dialog handle.
 *
 * @param pcpbd  The CSelf pointer for the PlainBaseDialog whose underlying
 *               dialog is being ended, (or whose CSelf is being cleaned up
 *               if the dialog never got started.)
 *
 * @param c      A valid thread context, or null if the thread context is not
 *               known.  This is only used when it is thought to be an abnormal
 *               termination, in which case it is use to invoke ensureFinished()
 *               int the Rexx dialog object.
 *
 * @return  It doesn't appeat that the return code was ever used anywhere in
 *          ooDialog.  So, it may be better to just eliminate the return. For
 *          now, 1 is returned if the admin block has already been through
 *          delDialog(), which should nver happen.  2 is returned if
 *          abnormalHal() is still set.  Otherwise 1 is returned.
 *
 * @remarks  We should never enter this function with dlgAllocated set to
 *           false.  The flag is only set to false in this function, and it is
 *           protected by the critical section.  Nevertheless, if we do get
 *           here, we just leave.
 *
 * @remarks  If abnormalHalt set to true, then we entered delDialog() under some
 *           very unusual error path.
 *
 *           In which case, it is likely that the PlainBaseDialog::run() method
 *           is still waiting (guard on) on the finished attribute and we will
 *           hang.
 *
 *           In all normal cases, the finished attribute is set through the OK
 *           or Cancel mechanism and delDialog() is running *after* finished is
 *           already set.  So, if abnormalHalt is set, we take an extra step to
 *           try and be sure waitForDialog() ends.  So far, this cleans up all
 *           the hangs I was able to produce, but this could still be a problem
 *           area.
 *
 * @remarks  There was an old ooDialog comment right before the admin block was
 *           removed from the DialogTable, which read: "The dialog adminstration
 *           block entry must be removed before the WM_QUIT message is posted."
 *
 *           I believe that comment was due to worries about acessing the
 *           pointer after things had started to be freed. The
 *           'dialogInAdminTable' check was used a lot.  Now the dlgAllocated
 *           flag replaces that check, and the flag is set ahead of the WM_QUIT
 *           being posted.
 *
 * @remarks  The small and big icons:  If not shared, the icon(s) were loaded
 *           from a file and need to be freed, otherwise the OS handles the
 *           free. The title bar icon is tricky.  At this point the dialog may
 *           still be visible.  If the small icon in the class is set to 0, the
 *           application will hang.  Same thing happens if the icon is freed.
 *           So, don't set a zero into the class bytes, and, if the icon is to
 *           be freed, do so after leaving the critical section.
 */
int32_t delDialog(pCPlainBaseDialog pcpbd, RexxThreadContext *c)
{
    size_t i;
    HICON hIconBig = NULL;
    HICON hIconSmall = NULL;

    EnterCriticalSection(&crit_sec);
    printf("In delDialog() hDlg=%p tabIdx=%d allocate=%d isActive=%d onTop=%d prev=%p\n",
           pcpbd->hDlg, pcpbd->tableIndex, pcpbd->dlgAllocated, pcpbd->isActive, pcpbd->onTheTop, pcpbd->previous);
    if ( ! pcpbd->dlgAllocated )
    {
        printf("delDialog() already ran for this dialog! pcpbd=%p\n", pcpbd);
        return -1;
    }
    pcpbd->dlgAllocated = false;

    int32_t ret = 1;
    bool wasFGW = (pcpbd->hDlg == GetForegroundWindow());

    if ( pcpbd->abnormalHalt )
    {
        ensureFinished(pcpbd, c, TheTrueObj);
        ret = 2;
    }
    pcpbd->abnormalHalt = false;

    if ( !( pcpbd->isControlDlg || pcpbd->isPageDlg) )
    {
        // Remove the dialog from the dialog table
        if ( pcpbd->tableIndex == CountDialogs - 1 )
        {
            // The dialog being ended is the last entry in the table, just set it to
            // null.
            DialogTable[pcpbd->tableIndex] = NULL;
        }
        else
        {
            // The dialog being ended is not the last entry.  Move the last entry to
            // the one being deleted and then delete the last entry.
            DialogTable[pcpbd->tableIndex] = DialogTable[CountDialogs-1];
            DialogTable[pcpbd->tableIndex]->tableIndex = pcpbd->tableIndex;
            DialogTable[CountDialogs-1] = NULL;
        }
        CountDialogs--;
    }

    if ( pcpbd->hDlg && ! pcpbd->isPageDlg )
    {
        if ( ! pcpbd->isControlDlg )
        {
            PostMessage(pcpbd->hDlg, WM_QUIT, 0, 0);
        }
        DestroyWindow(pcpbd->hDlg);
    }
    pcpbd->isActive = false;

    // Swap back the saved icons, if needed.  See the remarks in the function
    // header.
    if ( pcpbd->hDlg && pcpbd->didChangeIcon )
    {
        hIconBig = (HICON)setClassPtr(pcpbd->hDlg, GCLP_HICON, (LONG_PTR)pcpbd->sysMenuIcon);
        if ( pcpbd->titleBarIcon )
        {
            hIconSmall = (HICON)setClassPtr(pcpbd->hDlg, GCLP_HICONSM, (LONG_PTR)pcpbd->titleBarIcon);
        }

        if ( ! pcpbd->sharedIcon )
        {
            DestroyIcon(hIconBig);
            if ( ! hIconSmall )
            {
                hIconSmall = (HICON)getClassPtr(pcpbd->hDlg, GCLP_HICONSM);
            }
        }
        else
        {
            hIconSmall = NULL;
        }
    }

    if ( pcpbd->hInstance != NULL && pcpbd->hInstance != MyInstance )
    {
        FreeLibrary(pcpbd->hInstance);
    }

    pcpbd->hDlg = NULL;
    if ( pcpbd->wndBase->rexxHwnd != TheZeroObj && c != NULL )  // TODO fix it so that c can not be null.
    {
        c->ReleaseGlobalReference(pcpbd->wndBase->rexxHwnd);
        pcpbd->wndBase->rexxHwnd = TheZeroObj;
    }

    safeLocalFree(pcpbd->bkgBitmap);
    safeDeleteObject(pcpbd->bkgBrush);

    pcpbd->bkgBitmap = NULL;
    pcpbd->bkgBrush = NULL;

    // Delete the message tables of the dialog.
    deleteMessageTables(pcpbd->enCSelf);

    // Delete the data table.
    safeLocalFree(pcpbd->DataTab);
    pcpbd->DataTab = NULL;
    pcpbd->DT_size = 0;

    // Delete the color brushes.
    if (pcpbd->ColorTab)
    {
        for ( i = 0; i < pcpbd->CT_size; i++ )
        {
            safeDeleteObject(pcpbd->ColorTab[i].ColorBrush);
        }
        LocalFree(pcpbd->ColorTab);
        pcpbd->ColorTab = NULL;
        pcpbd->CT_size = 0;
    }

    // Delete the bitmaps and bitmap table.
    if (pcpbd->BmpTab)
    {
        for ( i = 0; i < pcpbd->BT_size; i++ )
        {
            if ( (pcpbd->BmpTab[i].loaded & 0x1011) == 1 )
            {
                /* otherwise stretched bitmap files are not freed */
                safeLocalFree((void *)pcpbd->BmpTab[i].bitmapID);
                safeLocalFree((void *)pcpbd->BmpTab[i].bmpFocusID);
                safeLocalFree((void *)pcpbd->BmpTab[i].bmpSelectID);
                safeLocalFree((void *)pcpbd->BmpTab[i].bmpDisableID);
            }
            else if ( pcpbd->BmpTab[i].loaded == 0 )
            {
                safeDeleteObject((HBITMAP)pcpbd->BmpTab[i].bitmapID);
                safeDeleteObject((HBITMAP)pcpbd->BmpTab[i].bmpFocusID);
                safeDeleteObject((HBITMAP)pcpbd->BmpTab[i].bmpSelectID);
                safeDeleteObject((HBITMAP)pcpbd->BmpTab[i].bmpDisableID);
            }
        }

        LocalFree(pcpbd->BmpTab);
        safeDeleteObject(pcpbd->colorPalette);
        pcpbd->BT_size = 0;
    }

    // Delete the icon resource table.
    if (pcpbd->IconTab)
    {
        for ( i = 0; i < pcpbd->IT_size; i++ )
        {
            safeLocalFree(pcpbd->IconTab[i].fileName);
        }
        LocalFree(pcpbd->IconTab);
        pcpbd->IconTab = NULL;
        pcpbd->IT_size = 0;
    }

    // Unhook a hook if it is installed.
    if ( pcpbd->enCSelf->hHook )
    {
        removeKBHook(pcpbd->enCSelf);
    }

    // For property sheet and property sheet page dialogs, it is unlikely, but
    // posible that dlgPrivate is not yet set.  (Happens if there is an error in
    // init().
    if ( pcpbd->isPageDlg && pcpbd->dlgPrivate != NULL )
    {
        delPageDialog((pCPropertySheetPage)pcpbd->dlgPrivate);
        pcpbd->dlgPrivate = NULL;
    }

    if ( pcpbd->isPropSheetDlg && pcpbd->dlgPrivate != NULL  )
    {
        delPropSheetDialog((pCPropertySheetDialog)pcpbd->dlgPrivate);
        pcpbd->dlgPrivate = NULL;
    }

    if ( ! (pcpbd->isControlDlg || pcpbd->isPageDlg) )
    {
        // Not sure this whole TopDlg thing is correctly coded.
        if ( CountDialogs == 0 )
        {
            TopDlg = NULL;
        }
        else
        {
            // It seems possible that pcpbd->previous may have been deleted already.
            pCPlainBaseDialog prev = (pCPlainBaseDialog)pcpbd->previous;

            if ( prev != NULL && dlgInDlgTable(prev) )
            {
                TopDlg = prev;

                if ( ! IsWindowEnabled(TopDlg->hDlg) )
                {
                    EnableWindow(TopDlg->hDlg, TRUE);
                }
                if ( wasFGW )
                {
                    SetForegroundWindow(TopDlg->hDlg);
                    TopDlg->onTheTop = true;
                }
            }
        }
    }

    LeaveCriticalSection(&crit_sec);

    if ( hIconSmall )
    {
        DestroyIcon(hIconSmall);
    }

    return ret;
}


/**
 * Loads and returns the handles to the regular size and small size icons for
 * the dialog. These icons are used in the title bar of the dialog, on the task
 * bar, and for the alt-tab display.
 *
 * The icons can come from the user resource DLL, a user defined dialog, or the
 * OODialog DLL.  IDs for the icons bound to the OODialog.dll are reserved.
 *
 * This function attempts to always succeed.  If an icon is not attained, the
 * default icon from the resources in the OODialog DLL is used.  This icon
 * should always be present, it is bound to the DLL when ooRexx is built.
 *
 * @param pcpbd     Pointer to the CSelf struct for the dialog
 * @param id        Numerical resource ID.
 * @param iconSrc   Flag indicating whether the icon is located in a DLL or to
 *                  be loaded from a file.
 * @param phBig     In/Out Pointer to an icon handle.  If the function succeeds,
 *                  on return will contain the handle to a regular size icon.
 * @param phSmall   In/Out Pointer to an icon handle.  On success will contain
 *                  a handle to a small size icon.
 *
 * @return True if the icons were loaded and the returned handles are valid,
 *         otherwise false.
 */
BOOL getDialogIcons(pCPlainBaseDialog pcpbd, INT id, UINT iconSrc, PHANDLE phBig, PHANDLE phSmall)
{
    int cx, cy;

    if ( phBig == NULL || phSmall == NULL )
    {
        return FALSE;
    }

    if ( id < 1 )
    {
        id = IDI_DLG_DEFAULT;
    }

    /* If one of the reserved IDs, iconSrc has to be ooDialog. */
    if ( id >= IDI_DLG_MIN_ID && id <= IDI_DLG_MAX_ID )
    {
        iconSrc = ICON_OODIALOG;
    }

    cx = GetSystemMetrics(SM_CXICON);
    cy = GetSystemMetrics(SM_CYICON);

    *phBig = getIconForID(pcpbd, id, iconSrc, cx, cy);

    /* If that didn't get the big icon, try to get the default icon. */
    if ( ! *phBig && id != IDI_DLG_DEFAULT )
    {
        id = IDI_DLG_DEFAULT;
        iconSrc = ICON_OODIALOG;
        *phBig = getIconForID(pcpbd, id, iconSrc, cx, cy);
    }

    /* If still no big icon, don't bother trying for the small icon. */
    if ( *phBig )
    {
        cx = GetSystemMetrics(SM_CXSMICON);
        cy = GetSystemMetrics(SM_CYSMICON);
        *phSmall = getIconForID(pcpbd, id, iconSrc, cx, cy);

        /* Very unlikely that the big icon was obtained and failed to get the
         * small icon.  But, if so, fail completely.  If the big icon came from
         * a DLL it was loaded as shared and the system handles freeing it.  If
         * it was loaded from a file, free it here.
         */
        if ( ! *phSmall )
        {
            if ( iconSrc & ICON_FILE )
            {
                DestroyIcon((HICON)*phBig);
            }
            *phBig = NULL;
        }
    }

    if ( ! *phBig )
    {
        return FALSE;
    }

    pcpbd->sharedIcon = iconSrc != ICON_FILE;
    return TRUE;
}


/**
 *  Methods for the .WindowBase mixin class.
 */
#define WINDOWBASE_CLASS       "WindowBase"

#define DISPLAY_METHOD_OPTIONS "'NORMAL', 'NORMAL FAST', 'DEFAULT', 'DEFAULT FAST', 'HIDE', 'HIDE FAST', or 'INACTIVE'"

static inline pCWindowBase validateWbCSelf(RexxMethodContext *c, void *pCSelf)
{
    pCWindowBase pcwb = (pCWindowBase)pCSelf;
    if ( pcwb == NULL )
    {
        baseClassIntializationException(c);
    }
    return pcwb;
}

static inline HWND getWBWindow(void *pCSelf)
{
    return ((pCWindowBase)pCSelf)->hwnd;
}

static HWND wbSetUp(RexxMethodContext *c, void *pCSelf)
{
    HWND hwnd = NULL;

    if ( pCSelf == NULL )
    {
        baseClassIntializationException(c);
    }
    else
    {
        oodResetSysErrCode(c->threadContext);

        hwnd = getWBWindow(pCSelf);
        if ( hwnd == NULL )
        {
            noWindowsDialogException(c, ((pCWindowBase)pCSelf)->rexxSelf);
        }
    }
    return hwnd;
}


/**
 * Interface to the Windows API: SendMessage().
 *
 * @param context
 * @param wm_msg
 * @param wParam
 * @param lParam
 * @param _hwnd
 * @param pcwb
 *
 * @return LRESULT
 *
 * @remarks   This function is used for most of the internally generated send
 *            message functions.  The assumption is that internally the code is
 *            correct in calling the function, but the programmer may be calling
 *            a method incorrectly.
 *
 *            Easier to describe by example:  Many of the dialog methods are
 *            only valid after the underlying dialog has been created, because a
 *            valid window handle is needed.  But, the programmer may invoke the
 *            method at the wrong time, say in the defineDialog().  Therefore,
 *            this function will raise a syntax condition if the window handle,
 *            for any reason is not valid.  Contrast this with
 *            sendWinMsgGeneric().
 */
static LRESULT sendWinMsg(RexxMethodContext *context, CSTRING wm_msg, WPARAM wParam, LPARAM lParam,
                          HWND _hwnd, pCWindowBase pcwb)
{
    oodResetSysErrCode(context->threadContext);

    uint32_t msgID;
    if ( ! rxStr2Number32(context, wm_msg, &msgID, 2) )
    {
        return 0;
    }

    HWND hwnd = (argumentOmitted(4) ? pcwb->hwnd : (HWND)_hwnd);
    if ( ! IsWindow(hwnd) )
    {
        invalidWindowException(context, pcwb->rexxSelf);
        return 0;
    }

    LRESULT result = SendMessage(hwnd, msgID, wParam, lParam);
    oodSetSysErrCode(context->threadContext);
    return result;
}

/**
 * Generic interface to the Windows API: SendMessage().
 *
 * @param c
 * @param hwnd
 * @param wm_msg
 * @param _wParam
 * @param _lParam
 * @param argPos
 * @param doIntReturn
 *
 * @return RexxObjectPtr
 *
 * @remarks   This function is used mostly for the implementation of previously
 *            documented ooDialog methods, such as
 *            DialogControl::processMessage(), or
 *            CategoryDialog::sendMessageToCategoryItem().  These functions did
 *            not raise syntax conditions, so, in an effort to retain backwards
 *            compatibility, an exception is not raised for an invalid window
 *            handle.  Rather, the .SystemErrorCode is set.  Contrast this to
 *            sendWinMessage().
 */
RexxObjectPtr sendWinMsgGeneric(RexxMethodContext *c, HWND hwnd, CSTRING wm_msg, RexxObjectPtr _wParam,
                                RexxObjectPtr _lParam, size_t argPos, bool doIntReturn)
{
    oodResetSysErrCode(c->threadContext);

    uint32_t msgID;
    if ( ! rxStr2Number32(c, wm_msg, &msgID, argPos) )
    {
        return TheZeroObj;
    }
    argPos++;

    WPARAM wParam;
    if ( ! oodGetWParam(c, _wParam, &wParam, argPos, true) )
    {
        return TheZeroObj;
    }
    argPos++;

    LPARAM lParam;
    if ( ! oodGetLParam(c, _lParam, &lParam, argPos, true) )
    {
        return TheZeroObj;
    }

    LRESULT lr = SendMessage(hwnd, msgID, wParam, lParam);
    oodSetSysErrCode(c->threadContext);

    if ( doIntReturn )
    {
        return c->Intptr((intptr_t)lr);
    }
    else
    {
        return pointer2string(c, (void *)lr);
    }
}


/**
 * Common code to call ShowWindow() from a native C++ API method.
 *
 * @param hwnd  Window handle of window to show.
 * @param type  Single character indicating which SW_ flag to use.
 *
 * @return  True if the window was previously visible.  Return false if the
 *          window was previously hidden.
 */
logical_t showWindow(HWND hwnd, char type)
{
    int flag;
    switch ( type )
    {
        case 'D' :
        case 'N' :
            flag = SW_NORMAL;
            break;

        case 'H' :
            flag = SW_HIDE;
            break;

        case 'I' :
            flag = SW_SHOWNA;
            break;

        case 'M' :
            flag = SW_SHOWMINIMIZED;
            break;

        case 'R' :
            flag = SW_RESTORE;
            break;

        case 'X' :
            flag = SW_SHOWMAXIMIZED;
            break;

        case 'S' :
        default :
            flag = SW_SHOW;
            break;

    }
    return ShowWindow(hwnd, flag);
}

uint32_t showFast(HWND hwnd, char type)
{
    uint32_t style = GetWindowLong(hwnd, GWL_STYLE);
    if ( style )
    {
        if ( type == 'H' )
        {
            style ^= WS_VISIBLE;
        }
        else
        {
            style |= WS_VISIBLE;
        }
        SetWindowLong(hwnd, GWL_STYLE, style);
        return 0;
    }
    return 1;
}


/**
 * Parses a string for all the show options possible in the setWindowPos() API.
 *
 * @param options  The keyword string.  This is a case-insensitive check. More
 *                 than one keyword, or no keyword, is acceptable.
 *
 * @return The show window postion flag corresponding to the options string.
 */
static uint32_t parseAllShowOptions(CSTRING options)
{
    uint32_t opts = 0;

    if ( options != NULL )
    {
        if ( StrStrI(options, "ASYNCWINDOWPOS") ) opts |= SWP_ASYNCWINDOWPOS;
        if ( StrStrI(options, "DEFERERASE"    ) ) opts |= SWP_DEFERERASE;
        if ( StrStrI(options, "DRAWFRAME"     ) ) opts |= SWP_DRAWFRAME;
        if ( StrStrI(options, "FRAMECHANGED"  ) ) opts |= SWP_FRAMECHANGED;
        if ( StrStrI(options, "HIDEWINDOW"    ) ) opts |= SWP_HIDEWINDOW;
        if ( StrStrI(options, "NOACTIVATE"    ) ) opts |= SWP_NOACTIVATE;
        if ( StrStrI(options, "NOCOPYBITS"    ) ) opts |= SWP_NOCOPYBITS;
        if ( StrStrI(options, "NOMOVE"        ) ) opts |= SWP_NOMOVE;
        if ( StrStrI(options, "NOOWNERZORDER" ) ) opts |= SWP_NOOWNERZORDER;
        if ( StrStrI(options, "NOREDRAW"      ) ) opts |= SWP_NOREDRAW;
        if ( StrStrI(options, "NOREPOSITION"  ) ) opts |= SWP_NOREPOSITION;
        if ( StrStrI(options, "NOSENDCHANGING") ) opts |= SWP_NOSENDCHANGING;
        if ( StrStrI(options, "NOSIZE"        ) ) opts |= SWP_NOSIZE;
        if ( StrStrI(options, "NOZORDER"      ) ) opts |= SWP_NOZORDER;
        if ( StrStrI(options, "SHOWWINDOW"    ) ) opts |= SWP_SHOWWINDOW;
    }
    return opts;
}


static bool getHwndBehind(RexxMethodContext *c, RexxObjectPtr _hwndBehind, HWND *hwnd)
{
    if ( c->IsPointer(_hwndBehind) )
    {
        *hwnd = (HWND)c->PointerValue((RexxPointerObject)_hwndBehind);
    }
    else
    {
        CSTRING str = c->ObjectToStringValue(_hwndBehind);

        if      ( stricmp(str, "BOTTOM"    ) == 0 ) *hwnd = HWND_BOTTOM;
        else if ( stricmp(str, "NOTTOPMOST") == 0 ) *hwnd = HWND_NOTOPMOST;
        else if ( stricmp(str, "TOP"       ) == 0 ) *hwnd = HWND_TOP;
        else if ( stricmp(str, "TOPMOST"   ) == 0 ) *hwnd = HWND_TOPMOST;
        else
        {
            size_t len = strlen(str);

            if ( (len == 0 || len == 2) || (len == 1 && *str != '0') || toupper(str[1]) != 'X' )
            {
                wrongArgValueException(c->threadContext, 1, "BOTTOM, NOTTOPMOST, TOP, TOPMOST, or a valid window handle", str);
                return false;
            }

            *hwnd = (HWND)string2pointer(str);
        }
    }

    return true;
}


/**
 * Performs the initialization of the WindowBase mixin class.
 *
 * This is done by creating the cself struct for that class and then sending
 * that struct to the init_windowBase() method.  That method will raise an
 * exception if its arg is not a RexxBufferObject.  This implies that the
 * WindowBase mixin class can only be initialized through the native API.
 *
 * The plain base dialog, dialog control, and window classes in ooDialog inherit
 * WindowBase.
 *
 * @param c        Method context we are operating in.
 *
 * @param hwndObj  Window handle of the underlying object.  This can be null and
 *                 for a dialog object, it is always null.  (Because for a
 *                 dialog object the underlying dialog has not been created at
 *                 this point.)
 *
 * @param self     The Rexx object that inherited WindowBase.
 *
 * @return True on success, otherwise false.  If false an exception has been
 *         raised.
 *
 * @remarks  This method calculates the factor X and Y values using the old,
 *           incorrect, ooDialog method.  When the hwnd is unknown, that is the
 *           best that can be done.  But, when the hwnd is known, it would be
 *           better to calculate it correctly.  Even when the hwnd is unknown,
 *           we could calculate it correctly using the font of the dialog.
 *
 *           Note that in the original ooDialog code, factorX and factorY were
 *           formatted to only 1 decimal place.
 *
 *           Note that in the original ooDialog, if the object was a dialog,
 *           (i.e. the hwnd is unknown,) then sizeX and sizeY simply remain at
 *           0.
 */
bool initWindowBase(RexxMethodContext *c, HWND hwndObj, RexxObjectPtr self, pCWindowBase *ppCWB)
{
    RexxBufferObject obj = c->NewBuffer(sizeof(CWindowBase));
    if ( obj == NULLOBJECT )
    {
        return false;
    }

    pCWindowBase pcwb = (pCWindowBase)c->BufferData(obj);
    memset(pcwb, 0, sizeof(CWindowBase));

    ULONG bu = GetDialogBaseUnits();
    pcwb->factorX = LOWORD(bu) / 4;
    pcwb->factorY = HIWORD(bu) / 8;

    pcwb->rexxSelf = self;

    if ( hwndObj != NULL )
    {
        RECT r = {0};
        if ( GetWindowRect(hwndObj, &r) == 0 )
        {
            systemServiceExceptionCode(c->threadContext, API_FAILED_MSG, "GetWindowRect");
            return false;
        }

        RexxStringObject h = pointer2string(c, hwndObj);
        pcwb->hwnd = hwndObj;
        pcwb->rexxHwnd = c->RequestGlobalReference(h);

        pcwb->sizeX =  (uint32_t)((r.right - r.left) / pcwb->factorX);
        pcwb->sizeY =  (uint32_t)((r.bottom - r.top) / pcwb->factorY);
    }
    else
    {
        pcwb->rexxHwnd = TheZeroObj;
    }

    RexxObjectPtr result = c->SendMessage1(self, "INIT_WINDOWBASE", obj);
    if ( result == TheFalseObj )
    {
        pcwb->initCode = 1;
    }

    if ( ppCWB != NULL )
    {
        *ppCWB = pcwb;
    }
    return true;
}

/** WindowBase::init_windowBase()
 *
 *
 */
RexxMethod1(RexxObjectPtr, wb_init_windowBase, RexxObjectPtr, cSelf)
{
    RexxMethodContext *c = context;
    if ( ! context->IsBuffer(cSelf) )
    {
        wrongClassException(context->threadContext, 1, "Buffer");
        return TheFalseObj;
    }

    context->SetObjectVariable("CSELF", cSelf);
    return TheTrueObj;
}

/** WindowBase::initCode  [attribute]
 */
RexxMethod1(wholenumber_t, wb_getInitCode, CSELF, pCSelf)
{
    pCWindowBase pcwb = validateWbCSelf(context, pCSelf);
    if ( pcwb != NULL )
    {
        return pcwb->initCode;
    }
    return 0;
}
RexxMethod2(RexxObjectPtr, wb_setInitCode, wholenumber_t, code, CSELF, pCSelf)
{
    pCWindowBase pcwb = validateWbCSelf(context, pCSelf);
    if ( pcwb != NULL )
    {
        pcwb->initCode = code;
    }
    return NULLOBJECT;
}

/** WindowBase::hwnd  [attribute get]
 */
RexxMethod1(RexxObjectPtr, wb_getHwnd, CSELF, pCSelf)
{
    return ((pCWindowBase)pCSelf)->rexxHwnd;
}

/** WindowBase::factorX  [attribute]
 */
RexxMethod1(double, wb_getFactorX, CSELF, pCSelf) { return ((pCWindowBase)pCSelf)->factorX; }
RexxMethod2(RexxObjectPtr, wb_setFactorX, float, xFactor, CSELF, pCSelf) { ((pCWindowBase)pCSelf)->factorX = xFactor; return NULLOBJECT; }

/** WindowBase::factorY  [attribute]
 */
RexxMethod1(double, wb_getFactorY, CSELF, pCSelf) { return ((pCWindowBase)pCSelf)->factorY; }
RexxMethod2(RexxObjectPtr, wb_setFactorY, float, yFactor, CSELF, pCSelf) { ((pCWindowBase)pCSelf)->factorY = yFactor; return NULLOBJECT; }

/** WindowBase::sizeX  [attribute]
 */
RexxMethod1(uint32_t, wb_getSizeX, CSELF, pCSelf) { return ((pCWindowBase)pCSelf)->sizeX; }
RexxMethod2(RexxObjectPtr, wb_setSizeX, uint32_t, xSize, CSELF, pCSelf) { ((pCWindowBase)pCSelf)->sizeX = xSize; return NULLOBJECT; }

/** WindowBase::sizeY  [attribute]
 */
RexxMethod1(uint32_t, wb_getSizeY, CSELF, pCSelf) { return ((pCWindowBase)pCSelf)->sizeY; }
RexxMethod2(RexxObjectPtr, wb_setSizeY, uint32_t, ySize, CSELF, pCSelf) { ((pCWindowBase)pCSelf)->sizeY = ySize; return NULLOBJECT; }

/** WindowBase::pixelCX  [attribute]
 *
 *  Returns the width of the window in pixels.  This is a 'get' only attribute.
 */
RexxMethod1(uint32_t, wb_getPixelCX, CSELF, pCSelf)
{
    pCWindowBase pcs = (pCWindowBase)pCSelf;
    if ( pcs->hwnd == NULL )
    {
        return 0;
    }

    RECT r = {0};
    GetWindowRect(pcs->hwnd, &r);
    return r.right - r.left;
}

/** WindowBase::pixelY  [attribute]
 *
 *  Returns the height of the window in pixels.  This is a 'get' only attribute.
 */
RexxMethod1(uint32_t, wb_getPixelCY, CSELF, pCSelf)
{
    pCWindowBase pcs = (pCWindowBase)pCSelf;
    if ( pcs->hwnd == NULL )
    {
        return 0;
    }

    RECT r = {0};
    GetWindowRect(pcs->hwnd, &r);
    return r.bottom - r.top;
}

/** WindowBase::sendMessage()
 *  WindowBase::sendMessageHandle()
 *
 *  Sends a window message to the underlying window of this object.
 *
 *  @param  wm_msg  The Windows window message ID.  This can be specified in
 *                  either "0xFFFF" or numeric format.
 *
 *  @param  wParam  The WPARAM value for the message.
 *  @param  lParam  The LPARAM value for the message.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.  For sendMessage() the result is returned as a whole number.
 *          For sendMessageHandle() the result is returned as an operating
 *          system handle.
 *
 *  @note Sets the .SystemErrorCode.
 *
 *        The wParam and lParam arguments can be in a number of formats.  An
 *        attempt is made to convert the Rexx object from a .Pointer, pointer
 *        string, uintptr_t number, or intptr_t number.  If they all fail, an
 *        exception is raised.
 *
 *        These methods will not work for window messages that take a string as
 *        an argument or return a string as a result.
 *
 *  @remarks  This function is used for the documented DialogControl
 *            processMessage() method, and therefore needs to remain generic.
 *            Internally, most of the time, it would make more sense to use one
 *            of the argument type specific methods like sendWinIntMsg().
 */
RexxMethod5(RexxObjectPtr, wb_sendMessage, CSTRING, wm_msg, RexxObjectPtr, _wParam, RexxObjectPtr, _lParam,
            NAME, method, CSELF, pCSelf)
{
    HWND hwnd = getWBWindow(pCSelf);
    bool doIntReturn = (strlen(method) == 11 ? true : false);

    return sendWinMsgGeneric(context, hwnd, wm_msg, _wParam, _lParam, 1, doIntReturn);
}

/** WindowBase::sendWinIntMsg()
 *  WindowBase::sendWinIntMsgH()
 *
 *  Sends a message to a Windows window where WPARAM and LPARAM are both numbers
 *  and the return is a number or a handle.  I.e., neither param is a handle and
 *  the return is not a string.
 *
 *  @param  wm_msg  The Windows window message ID.  This can be specified in
 *                  "0xFFFF" format or numeric format.
 *
 *  @param  wParam  The WPARAM value for the message.
 *  @param  lParam  The LPARAM value for the message.
 *
 *  @param  _hwnd   [OPTIONAL]  The handle of the window the message is sent to.
 *                  If omitted, the window handle of this object is used.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.
 *
 *  @remarks  Sets the .SystemErrorCode.
 *
 *            This method is not meant to be documented for the user, it is
 *            intended to be used internally only.
 *
 *            In addition, wParam should really be uintptr_t.  However, some,
 *            many?, control messages use / accept negative nubmers for wParam.
 *            If we were just casting the number here, that would work.  But,
 *            the interpreter checks the range before invoking and negative
 *            numbers cause a condition to be raised.  So, we use intptr_t for
 *            wParam here.  sendWinUintMsg() has been added for the case where
 *            WPARAM and LPARAM need an unsigned range.
 */
RexxMethod6(RexxObjectPtr, wb_sendWinIntMsg, CSTRING, wm_msg, intptr_t, wParam, intptr_t, lParam,
            OPTIONAL_POINTERSTRING, _hwnd, NAME, method, CSELF, pCSelf)
{
    LRESULT lr = sendWinMsg(context, wm_msg, (WPARAM)wParam, (LPARAM)lParam, (HWND)_hwnd, (pCWindowBase)pCSelf);

    if ( method[13] == '\0' )
    {
        return context->Intptr((intptr_t)lr);
    }
    else
    {
        return pointer2string(context, (void *)lr);
    }
}


/** WindowBase::sendWinUintMsg()
 *
 *  Sends a message to a Windows window where WPARAM and LPARAM are both
 *  unsigned numbers and the return is also unsigned.  I.e., neither param is a
 *  handle and the return is not a string or a handle.
 *
 *  @param  wm_msg  The Windows window message ID.  This can be specified in
 *                  "0xFFFF" format or numeric format.
 *
 *  @param  wParam  The WPARAM value for the message.
 *  @param  lParam  The LPARAM value for the message.
 *
 *  @param  _hwnd   [OPTIONAL]  The handle of the window the message is sent to.
 *                  If omitted, the window handle of this object is used.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.
 *
 *  @remarks  Sets the .SystemErrorCode.
 *
 *            This method is not meant to be documented for the user, it is
 *            intended to be used internally only.  Currently, all uses of this
 *            function have a return of a number.  If a need comes up to return
 *            a handle, then add sendWinUintMsgH().
 */
RexxMethod5(uintptr_t, wb_sendWinUintMsg, CSTRING, wm_msg, uintptr_t, wParam, uintptr_t, lParam,
            OPTIONAL_POINTERSTRING, _hwnd, CSELF, pCSelf)
{
    return (uintptr_t)sendWinMsg(context, wm_msg, (WPARAM)wParam, (LPARAM)lParam, (HWND)_hwnd, (pCWindowBase)pCSelf);
}


/** WindowBase::sendWinHandleMsg()
 *  WindowBase::sendWinHandleMsgH()
 *
 *  Sends a message to a Windows window where WPARAM is a handle and LPARAM is a
 *  number.  The result is returned as a number or as a handle, depending on the
 *  invoking method.
 *
 *  @param  wm_msg  The Windows window message ID.  This can be specified as a
 *                  decimal number, or in "0xFFFF" format.
 *
 *  @param  wParam  The WPARAM value for the message.  The argument must be in
 *                  pointer or handle format.
 *  @param  lParam  The LPARAM value for the message.
 *
 *  @param  _hwnd   [OPTIONAL]  The handle of the window the message is sent to.
 *                  If omitted, the window handle of this object is used.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.
 *
 *  @note     Sets the .SystemErrorCode.
 *
 *  @remarks  This method is not meant to be documented to the user, it is
 *            intended to be used internally only.
 */
RexxMethod6(RexxObjectPtr, wb_sendWinHandleMsg, CSTRING, wm_msg, POINTERSTRING, wParam, intptr_t, lParam,
            OPTIONAL_POINTERSTRING, _hwnd, NAME, method, CSELF, pCSelf)
{
    LRESULT lr = sendWinMsg(context, wm_msg, (WPARAM)wParam, (LPARAM)lParam, (HWND)_hwnd, (pCWindowBase)pCSelf);

    if ( method[16] == '\0' )
    {
        return context->Intptr((intptr_t)lr);
    }
    else
    {
        return pointer2string(context, (void *)lr);
    }
}


/** WindowBase::sendWinHandle2Msg()
 *  WindowBase::sendWinHandle2MsgH()
 *
 *  Sends a message to a Windows window where WPARAM is a number and LPARAM is a
 *  handle.  The result is returned as a number or as a handle, depending on the
 *  invoking method.
 *
 *  @param  wm_msg  The Windows window message ID.  This can be specified as a
 *                  decimal number, or in "0xFFFF" format.
 *
 *  @param  wParam  The WPARAM value for the message.
 *  @param  lParam  The LPARAM value for the message.  The argument must be in
 *                  pointer or handle format.
 *
 *  @param  _hwnd   [OPTIONAL]  The handle of the window the message is sent to.
 *                  If omitted, the window handle of this object is used.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.
 *
 *  @note     Sets the .SystemErrorCode.
 *
 *  @remarks  This method is not meant to be documented to the user, it is
 *            intended to be used internally only.
 */
RexxMethod6(RexxObjectPtr, wb_sendWinHandle2Msg, CSTRING, wm_msg, intptr_t, wParam, POINTERSTRING, lParam,
            OPTIONAL_POINTERSTRING, _hwnd, NAME, method, CSELF, pCSelf)
{
    LRESULT lr = sendWinMsg(context, wm_msg, (WPARAM)wParam, (LPARAM)lParam, (HWND)_hwnd, (pCWindowBase)pCSelf);

    if ( strlen(method) == 17 )
    {
        return context->Intptr((intptr_t)lr);
    }
    else
    {
        return pointer2string(context, (void *)lr);
    }
}


/** WindowBase::enable() / WindowBase::disable()
 *
 *  Enables or disables the window.  This function is mapped to both methods of
 *  WindowBase.
 *
 *  @return  True if the window was previously disabled, returns false if the
 *           window was not previously disabled.  Note that this is not succes
 *           or failure.  It always succeeds.
 *
 *  @remarks  The return was not previously documented.
 */
RexxMethod1(logical_t, wb_enable, CSELF, pCSelf)
{
    BOOL enable = TRUE;
    if ( msgAbbrev(context) == 'D' )
    {
        enable = FALSE;
    }
    return EnableWindow(getWBWindow(pCSelf), enable);
}

RexxMethod1(logical_t, wb_isEnabled, CSELF, pCSelf)
{
    return IsWindowEnabled(getWBWindow(pCSelf));
}

RexxMethod1(logical_t, wb_isVisible, CSELF, pCSelf)
{
    return IsWindowVisible(getWBWindow(pCSelf));
}

/** WindowBase::show() / WindowBase::hide()
 *
 *  Hides or shows the window.  This function is mapped to both methods of
 *  WindowBase.  The return for these methods was not previously documented.
 *
 *  @return  True if the window was previously visible.  Return false if the
 *           window was previously hidden.
 *
 *  @remarks Note that early versions of ooDialog use SW_SHOWNORMAL for the
 *           flag, not SW_SHOW. We need to preserve that because of the
 *           differences in SW_SHOW and SW_SHOWNORMAL when the window is
 *           maximized or minimized.
 */
RexxMethod2(logical_t, wb_show, NAME, method, CSELF, pCSelf)
{
    return showWindow(getWBWindow(pCSelf), (*method == 'S' ? 'N' : 'H'));
}

/** WindowBase::showFast() / WindowBase::hideFast()
 *
 *  Hides or shows the window 'fast'.  What this means is the visible flag is
 *  set, but the window is not forced to update.
 *
 *  This function is mapped to both methods of WindowBase. The return for these
 *  methods was not previously documented.
 *
 *  @return  0 for no error, 1 for error.  An error is highly unlikely.
 */
RexxMethod1(uint32_t, wb_showFast, CSELF, pCSelf)
{
    return showFast(getWBWindow(pCSelf), msgAbbrev(context));
}

/** WindowBase::display()
 *
 *
 *  @return  0 for success, 1 for error.  An error is highly unlikely.
 *
 *  @remarks display() is a method that was originally in WindowExtentions,
 *           making it a method of both a dialog and a dialog object. It is one
 *           of those methods that makes little sense for a dialog control
 *           because the SW_SHOWxxx flags used have no effect on dialog
 *           controls, other than to make them visible or invisible.
 *
 *           If the options keyword was omitted or wrong, then SW_SHOW was used
 *           as the flag, (really by accident.)  This is changed in 4.1.0 to
 *           cause a syntax condition.
 */
RexxMethod2(uint32_t, wb_display, OPTIONAL_CSTRING, opts,  CSELF, pCSelf)
{
    char type;
    uint32_t ret = 0;
    bool doFast = false;
    HWND hwnd = getWBWindow(pCSelf);

    if ( opts != NULL && StrStrI(opts, "FAST") != NULL )
    {
        doFast = true;
    }

    if ( opts == NULL ) {type = 'N';}
    else if ( StrStrI(opts, "NORMAL"  ) != NULL ) {type = 'N';}
    else if ( StrStrI(opts, "DEFAULT" ) != NULL ) {type = 'N';}
    else if ( StrStrI(opts, "HIDE"    ) != NULL ) {type = 'H';}
    else if ( StrStrI(opts, "INACTIVE") != NULL )
    {
        if ( doFast )
        {
            userDefinedMsgException(context->threadContext, 1, "The keyword FAST can not be used with INACTIVE");
            goto done_out;
        }
        type = 'I';
    }
    else
    {
        wrongArgValueException(context->threadContext, 1, DISPLAY_METHOD_OPTIONS, opts);
        return 1;
    }

    if ( doFast )
    {
        // showFast() needs S or H
        type = (type == 'N' ? 'S' : type);
        ret = showFast(hwnd, type);
    }
    else
    {
        showWindow(hwnd, type);
    }

done_out:
    return ret;
}

/** WindowsBase::draw()
 *  WindowsBase::redrawClient()
 *  WindowsBase::update()
 *
 *  Invalidates the entire client area of the the window. draw() and
 *  redrawClient() then forces the window to repaint.
 *
 *  This method maps to the draw(), update() and the redrawClient() methods.
 *  The implementation preserves existing behavior prior to ooRexx 4.0.1.  That
 *  is:
 *
 *  The draw() method takes no argument and always uses false for the erase
 *  background arg.
 *
 *  The update() method takes no argument and always uses true for the erase
 *  background arg. This method invalidates the client area only, does not call
 *  UpdateWindow().  The Rexx method name of 'update' was poorly choosen,
 *  because it does not do an update, only an invalidate.
 *
 *  The redrawClient() method takes an argument to set the erase background arg.
 *  The argument can be either .true / .false (1 or 0), or yes / no, or ja /
 *  nein. Note however, the redrawClient() has always been documented as taking
 *  1 or Y to erase the background and implemented so that if the arg was
 *  omitted it would be .false.
 *
 *  @param  erase  [optional]  Whether the redrawClient operation should erase
 *                 the window background.  Can be true / false or yes / no.  The
 *                 default is false.
 *
 *  @return  0 for success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, wb_redrawClient, OPTIONAL_CSTRING, erase, CSELF, pCSelf)
{
    HWND hwnd = getWBWindow(pCSelf);
    char flag = msgAbbrev(context);
    bool doErase;
    bool doUpdate = true;

    if ( flag == 'D' )
    {
        doErase = false;
    }
    else if ( flag == 'U' )
    {
        doErase = true;
        doUpdate = false;
    }
    else
    {
        doErase = isYes(erase);
    }
    return redrawRect(context, hwnd, NULL, doErase, doUpdate);
}

/** WindowsBase::redraw()
 *
 *  Causes the entire window, including frame, to be redrawn.
 *
 *  @return  0 for success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod1(RexxObjectPtr, wb_redraw, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    if ( RedrawWindow(getWBWindow(pCSelf), NULL, NULL,
                      RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
}

/** WindowBase::title / WindowBase::getText()
 *
 *  Gets the window text.
 *
 *  For a window with a frame, this is the window title.  But for a dialog
 *  control, this is the text for the control.  This of course varies depending
 *  on the type of control.  For a button, it is the button label, for an edit
 *  control it is the edit text, etc..
 *
 *  @return  On success, the window text, which could be the empty string.  On
 *           failure, the empty string.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod1(RexxStringObject, wb_getText, CSELF, pCSelf)
{
    RexxStringObject result = context->NullString();
    HWND hwnd = getWBWindow(pCSelf);

    // Whether this fails or succeeds doesn't matter, we just return result.
    rxGetWindowText(context, hwnd, &result);
    return result;
}

/** WindowBase::setText() / WindowBase::setTitle() / WindowBase::title=
 *
 *  Sets the window text to the value specified.
 *
 *  @param  text  The text to be set as the window text
 *
 *  @return  0 for success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  Unfortunately, in 3.2.0, setText for an edit control was
 *            documented as returning the negated system error code and in 4.0.0
 *            setText for a static control was documented as returning the
 *            system error code (which is positive of course.)  The mapping here
 *            to the various versions of 'getText' is cleaner, but we may need
 *            to special case the return to match the previous documentation.
 */
RexxMethod2(wholenumber_t, wb_setText, CSTRING, text, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    if ( SetWindowText(getWBWindow(pCSelf), text) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return 1;
    }
    return 0;
}


/** WindowBase::getTextSizePx()
 *
 *  Gets the size, (width and height,) in pixels, needed to display a string.
 *
 *  @param text      The string to calculate the size of.
 *
 *  @return  A .Size object containg the width and height for the text in
 *           pixels.
 */
RexxMethod2(RexxObjectPtr, wb_getTextSizePx, CSTRING, text, CSELF, pCSelf)
{
    SIZE size = {0};

    HWND hwnd = wbSetUp(context, pCSelf);
    if ( hwnd == NULL )
    {
        goto error_out;
    }

    if ( ! textSizeFromWindow(context, text, &size, hwnd) )
    {
        goto error_out;
    }

    return rxNewSize(context, size.cx, size.cy);

error_out:
    return NULLOBJECT;
}


/** WindowBase::getTextSizeScreen()
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
 *
 *  @remarks  This method is needed to provide backward compatibility for the
 *            deprecated getTextSize().  That is why the arguments are so
 *            convoluted.  Users are advised to use getTextSizePx()
 */
RexxMethod5(RexxObjectPtr, wb_getTextSizeScreen, CSTRING, text, OPTIONAL_CSTRING, type,
            OPTIONAL_CSTRING, fontSrc, OPTIONAL_uint32_t, fontSize, CSELF, pCSelf)
{
    SIZE size = {0};

    HWND hwnd = wbSetUp(context, pCSelf);
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
                goto error_out;
            }
            GetTextExtentPoint32(hdc, text, (int)strlen(text), &size);
        }
        else if ( m == 'F' )
        {
            HFONT hFont = (HFONT)string2pointer(fontSrc);
            if ( hFont == NULL )
            {
                invalidTypeException(context->threadContext, 3, " handle to a font");
                goto error_out;
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


/** WindowBase::windowRect()
 *
 *  Retrieves the dimensions of the bounding rectangle of this window.  The
 *  dimensions are in pixels and the coordinates are relative to the upper-left
 *  corner of the screen.
 *
 *  @param  hwnd  [OPTIONAL]  By default, the coordinates are for this window.
 *                However, the optional hwnd argument can be used to specify
 *                getting the coordinates for some other window.  See remarks.
 *
 *  @return  The bounding rectangle of the window as .Rect object.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  The windowRect() method supplies an alternative to both the
 *            getRect() and getWindowRect(hwnd) methods, to allow returning a
 *            .Rect object rather than a string of blank separated values.
 *
 *            Pre 4.0.1, getRect() was in WindowExtensions and getWindowRect()
 *            was in DialogExtensions.  The need to provide backward
 *            compatibility, use meaningful method names, maintain symmetry with
 *            the getClientRect(hwnd) method is what drove the decision to allow
 *            the optional hwnd argument to this method.
 */
RexxMethod2(RexxObjectPtr, wb_windowRect, OPTIONAL_POINTERSTRING, _hwnd, CSELF, pCSelf)
{
    HWND hwnd = (argumentOmitted(1) ? getWBWindow(pCSelf) : (HWND)_hwnd);
    return oodGetWindowRect(context, hwnd);
}


/** WindowBase::clientRect()
 *
 *  Retrieves the coordinates of a window's client area.  The coordinates are in
 *  pixels.
 *
 *  The client coordinates specify the upper-left and lower-right corners of the
 *  client area. Because client coordinates are relative to the upper-left
 *  corner of a window's client area, the coordinates of the upper-left corner
 *  are always (0,0).
 *
 *  @param  hwnd  [OPTIONAL]  By default, the coordinates are for this window.
 *                However, the optional hwnd argument can be used to specify
 *                getting the coordinates for some other window.  See remarks.
 *
 *  @return  The coordinates of the client area of the window as a .Rect object.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  The clientRect() method supplies an alternative to the
 *            getClinetRect(hwnd) method, to allow returning a .Rect object
 *            rather than a string of blank separated values.
 *
 *            Pre 4.0.1, getClientRect() was in WindowExtensions.  The need to
 *            provide backward compatibility, use meaningful method names, and
 *            maintain symmetry with the getRect() and getWindowRect(hwnd)
 *            methods is what drove the decision to move this method to
 *            WidnowBase and allow the optional hwnd argument to this method.
 */
RexxMethod2(RexxObjectPtr, wb_clientRect, OPTIONAL_POINTERSTRING, _hwnd, CSELF, pCSelf)
{
    HWND hwnd = (argumentOmitted(1) ? getWBWindow(pCSelf) : (HWND)_hwnd);

    RECT r = {0};
    oodGetClientRect(context, (HWND)hwnd, &r);
    return rxNewRect(context, &r);
}

/** WindowBase::setRect()
 *
 *  Changes the size, and position of a child, pop-up, or top-level window.
 *
 *  Provides a simplified interface to the Windows API, SetWindowPos().  There
 *  is no provision for the hwndInsertAfter arg, it does not recognize all the
 *  allowable flags, and it always uses SWP_NOZORDER.
 *
 *  By specifying either NOSIZE or NOMOVE options the programmer can only move
 *  or only resize the window.
 *
 *  @param coordinates  The coordinates of a point / size rectangle, given in
 *                      pixels
 *
 *    Form 1:  A .Rect object.
 *    Form 2:  A .Point object and a .Size object.
 *    Form 3:  x1, y1, cx, cy
 *
 *  @param  flags   [OPTIONAL] Keywords specifying the behavior of the method.
 *
 *  @return  True for success, false on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  Microsoft says: If the SWP_SHOWWINDOW or SWP_HIDEWINDOW flag is
 *            set, the window cannot be moved or sized.  But, that does not
 *            appear to be true.
 *
 *            The processing of the flags argument is exactly the same as what
 *            the original ooDialog code did. I.e., the flags start with
 *            SWP_NOZORDER and then the string is checked for each possible
 *            keyword, if found, that flag is added.  If the user had both
 *            NOMOVE and NOSIZE, both flags ared added. If the user had both
 *            SHOW and HIDE, then both flags are added, etc..
 */
RexxMethod2(RexxObjectPtr, wb_setRect, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = wbSetUp(context, pCSelf);
    if ( hwnd == NULL )
    {
        return NULLOBJECT;
    }

    // A RECT is used to return the values, even though the semantics are not
    // quite correct.  (right will really be cx and bottom will be cy.)
    size_t countArgs;
    size_t argsUsed;
    RECT   rect;
    if ( ! getRectFromArglist(context, args, &rect, false, 1, 5, &countArgs, &argsUsed) )
    {
        return NULLOBJECT;
    }

    RexxObjectPtr obj;
    CSTRING options = "";
    if ( argsUsed == 1 )
    {
        if ( countArgs > 2 )
        {
            return tooManyArgsException(context->threadContext, 2);
        }
        if ( countArgs == 2 )
        {
            obj = context->ArrayAt(args, 2);
            options = context->ObjectToStringValue(obj);
        }
    }
    else if ( argsUsed == 2 )
    {
        if ( countArgs > 3 )
        {
            return tooManyArgsException(context->threadContext, 3);
        }
        if ( countArgs == 3 )
        {
            obj = context->ArrayAt(args, 3);
            options = context->ObjectToStringValue(obj);
        }
    }
    else
    {
        if ( countArgs == 5 )
        {
            obj = context->ArrayAt(args, 5);
            options = context->ObjectToStringValue(obj);
        }
    }

    uint32_t opts = parseShowOptions(options);
    if ( SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.right, rect.bottom, opts) != 0 )
    {
        return TheTrueObj;
    }

    oodSetSysErrCode(context->threadContext);
    return TheFalseObj;
}

/** WindowBase::resizeTo()
 *  WindowBase::moveTo()
 *
 *  Resize to, changes the size of this window.  The new size is specified in
 *  pixels.
 *
 *  Move to, changes the position of this window.  The new position is specified
 *  as the coordinates of the upper left corner of the window, in pixels.
 *
 *  @param  coordinates  The new position (x, y) or new size (cx, cy) in pixels.
 *
 *    resizeTo()
 *      Form 1:  A .Size object.
 *      Form 2:  cx, cy
 *
 *    moveTo()
 *      Form 1:  A .Point object.
 *      Form 2:  x, y
 *
 *  @param  flags   [OPTIONAL] Keywords that control the behavior of the method.
 *
 *  @return  0 for success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  No effort is made to ensure that only a .Size object is used for
 *            resizeTo() and only a .Point object for moveTo().
 *
 *            The processing of the flags argument is exactly the same as what
 *            the original ooDialog code did. I.e., the flags start with
 *            SWP_NOZORDER and then the string is checked for each possible
 *            keyword, if found, that flag is added.  For resize and move then,
 *            if the user did have NOMOVE or NOSIZE, the flag is added. If the
 *            user had both SHOW and HIDE, then both flags are added, etc..
 */
RexxMethod3(RexxObjectPtr, wb_resizeMove, ARGLIST, args, NAME, method, CSELF, pCSelf)
{
    HWND hwnd = wbSetUp(context, pCSelf);
    if ( hwnd == NULL )
    {
        return NULLOBJECT;
    }

    // POINT and SIZE structs are binary compatible.  A POINT is used to return
    // the values, even though the semantics are not quite correct for
    // resizeTo(). (x will really be cx and y will be cy.)
    size_t countArgs;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, &point, 1, 3, &countArgs, &argsUsed) )
    {
        return NULLOBJECT;
    }

    RexxObjectPtr obj;
    CSTRING options = "";
    if ( argsUsed == 1 )
    {
        if ( countArgs > 2 )
        {
            return tooManyArgsException(context->threadContext, 2);
        }
        if ( countArgs == 2 )
        {
            // The object at index 2 has to exist, otherwise countArgs would
            // equal 1.
            obj = context->ArrayAt(args, 2);
            options = context->ObjectToStringValue(obj);
        }
    }
    else if ( argsUsed == 2 )
    {
        if ( countArgs == 3 )
        {
            // The object at index 3 has to exist, otherwise countArgs would
            // equal 2.
            obj = context->ArrayAt(args, 3);
            options = context->ObjectToStringValue(obj);
        }
    }

    uint32_t opts = parseShowOptions(options);
    RECT r = {0};

    if ( *method == 'R' )
    {
        opts = (opts & ~SWP_NOSIZE) | SWP_NOMOVE;
        r.right = point.x;
        r.bottom = point.y;
    }
    else
    {
        opts = (opts & ~SWP_NOMOVE) | SWP_NOSIZE;
        r.left = point.x;
        r.top = point.y;
    }

    if ( SetWindowPos(hwnd, NULL, r.left, r.top, r.right, r.bottom, opts) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
}


/** WindowBase::getRealSize()
 *  WindowBase::getRealPos()
 *
 *  Retrieves the size, or the position, in pixels, of this window
 *
 *  @return  The size as a .Size object, or the position as a .Point object.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, wb_getSizePos, NAME, method, CSELF, pCSelf)
{
    HWND hwnd = wbSetUp(context, pCSelf);
    if ( hwnd == NULL )
    {
        return NULLOBJECT;
    }

    RECT r = {0};
    if ( GetWindowRect(getWBWindow(pCSelf), &r) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
    }
    if ( method[7] == 'S' )
    {
        return rxNewSize(context, (r.right - r.left), (r.bottom - r.top));
    }

    return rxNewPoint(context, r.left, r.top);
}


/** WindowBase::setWindowPos()
 *
 *  Changes the size and position of the specified window.  Provides a complete
 *  interface to the SetWindowPos() Windows API, as contrasted to the setRect()
 *  method.
 *
 *  By specifying either NOSIZE or NOMOVE options the programmer can only move
 *  or only resize the window.  However, the programmer can also use the
 *  convenience methods moveWindow() and sizeWindow().
 *
 *  @param hwndBehind   This window is inserted after hwndBehind.  This is the
 *                      hwndInsertAfter arg in SetWindowPos().
 *  @param coordinates  The coordinates of a point / size rectangle, given in
 *                      pixels
 *
 *    Form 1:  A .Rect object.
 *    Form 2:  A .Point object and a .Size object.
 *    Form 3:  x1, y1, cx, cy
 *
 *  @param  flags   [OPTIONAL] Keywords specifying the behavior of the method.
 *
 *  @return  True for success, false on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  Microsoft says: If the SWP_SHOWWINDOW or SWP_HIDEWINDOW flag is
 *            set, the window cannot be moved or sized.  But, that does not
 *            appear to be true.
 *
 *            Th WindowBase::setRect() should be considered a convenience method
 *            for setWindowPos().  It leaves out the hwndBehind arg and only a
 *            recognizes a subset of the show options.
 */
RexxMethod3(RexxObjectPtr, wb_setWindowPos, RexxObjectPtr, _hwndBehind, ARGLIST, args, CSELF, pCSelf)
{
    HWND hwnd = wbSetUp(context, pCSelf);
    if ( hwnd == NULL )
    {
        goto done_out;
    }

    HWND hwndBehind;
    if ( ! getHwndBehind(context, _hwndBehind, &hwndBehind) )
    {
        goto done_out;
    }

    size_t countArgs;
    size_t    argsUsed;
    RECT   rect;
    if ( ! getRectFromArglist(context, args, &rect, false, 2, 6, &countArgs, &argsUsed) )
    {
        goto done_out;
    }

    RexxObjectPtr obj;
    CSTRING options = "";
    if ( argsUsed == 1 )
    {
        if ( countArgs > 3 )
        {
            return tooManyArgsException(context->threadContext, 3);
        }
        if ( countArgs == 3 )
        {
            obj = context->ArrayAt(args, 3);
            options = context->ObjectToStringValue(obj);
        }
    }
    else if ( argsUsed == 2 )
    {
        if ( countArgs > 4 )
        {
            return tooManyArgsException(context->threadContext, 4);
        }
        if ( countArgs == 4 )
        {
            obj = context->ArrayAt(args, 4);
            options = context->ObjectToStringValue(obj);
        }
    }
    else
    {
        if ( countArgs == 6 )
        {
            obj = context->ArrayAt(args, 6);
            options = context->ObjectToStringValue(obj);
        }
    }

    uint32_t opts = parseAllShowOptions(options);
    if ( SetWindowPos(hwnd, hwndBehind, rect.left, rect.top, rect.right, rect.bottom, opts) != 0 )
    {
        return TheTrueObj;
    }

    oodSetSysErrCode(context->threadContext);

done_out:
    return TheFalseObj;
}

/** WindowBase::moveWindow()
 *  WindowBase::sizeWindow()
 *
 *  Size window, changes the size of this window.  The new size is specified in
 *  pixels.
 *
 *  Move window, changes the position of this window.  The new position is
 *  specified as the coordinates of the upper left corner of the window, in
 *  pixels.
 *
 *  @param  hwndBehind   If the z-order is changed, then this window is inserted
 *                       into the z-order after hwndBehind.  TOP BOTTOM TOPMOST
 *                       and NOTTOPMOST keywords are also accepted.
 *
 *  @param  coordinates  The new position (x, y) or new size (cx, cy) in pixels.
 *
 *    sizeWindow()
 *      Form 1:  A .Size object.
 *      Form 2:  cx, cy
 *
 *    moveWindow()
 *      Form 1:  A .Point object.
 *      Form 2:  x, y
 *
 *  @param  flags   [OPTIONAL] Keywords that control the behavior of the method.
 *
 *  @return  True for success, false on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *         moveTo() and resizeTo() are convenience methods that are somewhat
 *         less complicated to use.  Those methods eliminate the hwndBehind
 *         argument and use a simple subset of the allowable flags.
 *
 *  @remarks  No effort is made to ensure that only a .Size object is used for
 *            sizeWindow() and only a .Point object for moveWindow().
 */
RexxMethod4(RexxObjectPtr, wb_moveSizeWindow, RexxObjectPtr, _hwndBehind, ARGLIST, args, NAME, method, CSELF, pCSelf)
{
    RECT r = {0};

    HWND hwnd = wbSetUp(context, pCSelf);
    if ( hwnd == NULL )
    {
        goto done_out;
    }

    HWND hwndBehind;
    if ( ! getHwndBehind(context, _hwndBehind, &hwndBehind) )
    {
        goto done_out;
    }

    // POINT and SIZE structs are binary compatible.  A POINT is used to return
    // the values, even though the semantics are not quite correct for
    // resizeTo(). (x will really be cx and y will be cy.)
    size_t countArgs;
    size_t argsUsed;
    POINT  point;
    if ( ! getPointFromArglist(context, args, &point, 2, 4, &countArgs, &argsUsed) )
    {
        goto done_out;
    }

    RexxObjectPtr obj;
    CSTRING options = "";
    if ( argsUsed == 1 )
    {
        if ( countArgs > 3 )
        {
            return tooManyArgsException(context->threadContext, 3);
        }
        if ( countArgs == 3 )
        {
            // The object at index 3 has to exist, otherwise countArgs would
            // equal 2.
            obj = context->ArrayAt(args, 3);
            options = context->ObjectToStringValue(obj);
        }
    }
    else if ( argsUsed == 2 )
    {
        if ( countArgs == 4 )
        {
            // The object at index 4 has to exist, otherwise countArgs would
            // equal 3.
            obj = context->ArrayAt(args, 4);
            options = context->ObjectToStringValue(obj);
        }
    }

    uint32_t opts = parseAllShowOptions(options);

    if ( *method == 'S' )
    {
        opts = (opts &= ~SWP_NOSIZE) | SWP_NOMOVE;
        ;
        r.right = point.x;
        r.bottom = point.y;
    }
    else
    {
        opts = (opts & ~SWP_NOMOVE) | SWP_NOSIZE;
        r.left = point.x;
        r.top = point.y;
    }
    if ( SetWindowPos(hwnd, hwndBehind, r.left, r.top, r.right, r.bottom, opts) != 0 )
    {
        return TheTrueObj;
    }

    oodSetSysErrCode(context->threadContext);

done_out:
    return TheFalseObj;
}


/** WindowBase::clear()
 *
 *  'Clears' the dialog's client area by painting the entire area with the
 *  system color COLOR_BTNFACE brush.  This is the same color as the default
 *  dialog background color so it has the effect of erasing everything on the
 *  client area, or 'clearing' it.
 *
 *  @return 0 on success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  Prior to 4.0.1, this method was in both the DialogExtensions and
 *            Dialog class. However, each method simply uses the object's window
 *            handle so it makes more sense to be a WindowBase method.
 *
 *            Note this also: both methods were broken.  They used the window
 *            rect of the window, not the client rect.  In addition,
 *            GetWindowDC() was used instead of GetDC().  This has the effect of
 *            painting in the nonclient area of the window, which normally is
 *            not done.  Since the methods were broken and the documentation was
 *            vague, it is not possible to tell what the original intent was.
 *
 *            Since clearing the window rect of the dialog erases the title bar,
 *            and there is no way for the ooDialog programmer to redraw the
 *            title bar or title bar buttons, the assumption is that the methods
 *            were intended to clear the client area.
 */
RexxMethod1(RexxObjectPtr, wb_clear, CSELF, pCSelf)
{
    HWND hwnd = getWBWindow(pCSelf);

    RECT r = {0};
    if ( oodGetClientRect(context, hwnd, &r) == TheOneObj )
    {
        return TheOneObj;
    }
    return clearRect(context, hwnd, &r);
}


/** WindowBase::foreGroundWindow()
 *
 *  Returns the handle of the window in the foreground.  The foreground window
 *  can be NULL in certain circumstances, such as when a window is losing
 *  activation.
 *
 *  @note  Sets the .SystemErrorCode, but it is unlikely that the operating
 *         system sets last error during the execution of this method.
 */
RexxMethod0(POINTERSTRING, wb_foreGroundWindow)
{
    oodResetSysErrCode(context->threadContext);
    HWND hwnd = GetForegroundWindow();
    oodSetSysErrCode(context->threadContext);
    return hwnd;
}


/** WindowBase::screen2client()
 *  WindowBase::client2screen()
 *
 *  screen2client() converts the screen coordinates of the specified point on
 *  the screen to client-area coordinates of this window.
 *
 *  This method uses the screen coordinates given in the .Point object to
 *  compute client-area coordinates. It then replaces the screen coordinates
 *  with the client coordinates. The new coordinates are relative to the
 *  upper-left corner of this window's client area.
 *
 *
 *  client2screen() converts the client-area coordinates of the specified point
 *  to screen coordinates.
 *
 *  This method uses the client-area coordinates given in the .Point object to
 *  compute screen coordinates. It then replaces the client-area coordinates
 *  with the screen coordinates. The new coordinates are relative to the
 *  upper-left corner of the screen.
 *
 *  @param  pt  [in/out] On entry, the coordinates to be converted, on exit the
 *              converted coordinates.
 *
 *  @return True on success, false on failure.  The .SystemErrorCode should
 *          contain the reason for failure.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 */
RexxMethod4(logical_t, wb_screenClient, RexxObjectPtr, pt, NAME, method, OSELF, self, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    oodResetSysErrCode(context->threadContext);
    BOOL success = FALSE;

    HWND hwnd = getWBWindow(pCSelf);
    if ( hwnd == NULL )
    {
        noWindowsDialogException(context, self);
        goto done_out;
    }

    if ( c->IsOfType(pt, "POINT") )
    {
        POINT *p = rxGetPoint(context, pt, 1);
        if ( p != NULL )
        {
            if ( *method == 'S' )
            {
                success = ScreenToClient(hwnd, p);
            }
            else
            {
                success = ClientToScreen(hwnd, p);
            }

            if ( ! success )
            {
                oodSetSysErrCode(context->threadContext);
            }
        }
    }
    else if ( c->IsOfType(pt, "RECT") )
    {
        RECT *r = rxGetRect(context, pt, 1);
        if ( r != NULL )
        {
            if ( *method == 'S' )
            {
                if ( ScreenToClient(hwnd, (POINT *)r) )
                {
                    success = ScreenToClient(hwnd, ((POINT *)r) + 1);
                }
            }
            else
            {
                if ( ClientToScreen(hwnd, (POINT *)r) )
                {
                    success = ClientToScreen(hwnd, ((POINT *)r) + 1);
                }
            }

            if ( ! success )
            {
                oodSetSysErrCode(context->threadContext);
            }
        }
    }
    else
    {
        wrongArgValueException(context->threadContext, 1, "Point or Rect", pt);
    }

done_out:
    return success;
}

/** WindowBase::getWindowLong()  [private]
 *
 *  Retrieves information about this window.  Specifically, the information
 *  available through the GetWindowLong().  Internally this used to get the
 *  GWL_ID, GWL_STYLE, and GWL_EXSTYLE values.
 *
 *  @param  flag  The index for the information to be retrieved.
 *
 *  @return  The unsigned 32-bit information for the specified index.
 *
 *  @remarks  The other indexes, besides the 3 mentioned above are all pointer
 *            or handle values.  Because of this, this method should not be
 *            documented.  The implementation may change.
 */
RexxMethod2(uint32_t, wb_getWindowLong_pvt, int32_t, flag, CSELF, pCSelf)
{
    return GetWindowLong(getWBWindow(pCSelf), flag);
}


/**
 *  Methods for the .PlainBaseDialog class.
 */
#define PLAINBASEDIALOG_CLASS    "PlainBaseDialog"
#define CONTROLBAG_ATTRIBUTE     "PlainBaseDialogControlBag"

static inline HWND getPBDWindow(RexxMethodContext *c, void *pCSelf)
{
    if ( pCSelf == NULL )
    {
        return (HWND)baseClassIntializationException(c);
    }

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(c, pcpbd->rexxSelf);
    }
    return pcpbd->hDlg;
}

HWND getPBDControlWindow(RexxMethodContext *c, pCPlainBaseDialog pcpbd, RexxObjectPtr rxID)
{
    HWND hCtrl = NULL;
    oodResetSysErrCode(c->threadContext);

    uint32_t id;
    if ( ! oodSafeResolveID(&id, c, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        oodSetSysErrCode(c->threadContext, ERROR_INVALID_WINDOW_HANDLE);
    }
    else
    {
        hCtrl = GetDlgItem(pcpbd->hDlg, id);
        if ( hCtrl == NULL )
        {
            oodSetSysErrCode(c->threadContext);
        }
    }
    return hCtrl;
}

/**
 * Used to end the Windows dialog and clean up the CSelf struct for the Rexx
 * dialog.
 *
 * Can also be used to clean up the CSelf struct when an error happens during
 * the creation of the underlying Windows dialog.
 *
 * @param pcpbd  Pointer to the PlainBaseDialog CSelf struct.
 * @param c      Thread context we are operating in.
 *
 * @return -1 if the CSelf struct has already been cleaned up.  Otherwise, 1 for
 *         a normal termination, or 2 for an abnormal termination.  The return
 *         is not really used anywhere.
 *
 * @remarks  Prior to the conversion to the C++ APIs, this function was passed
 *           the dialog window handle and did a seekDlgAdm using that handle.
 *           It was also possible that it would be called when the creation of
 *           the underlying dialog failed, passing in a null handle.  In that
 *           case, it erroneously deleted the 'TopDlg.'
 *
 * @remarks  It turns out it is relatively easy for this function to be called
 *           twice for the same dialog (hence the old seekDlgAdm.)  We first
 *           test dlgAllocated to bypass calling EnterCriticalSection() when
 *           it is not needed.  But, then we need to test the flag again after
 *           gaining the critical section, because it is not unusual for
 *           delDialog() to be running at the time this function is entered.
 */
int32_t stopDialog(pCPlainBaseDialog pcpbd, RexxThreadContext *c)
{
    int32_t result = -1;

    if ( pcpbd->dlgAllocated )
    {
        EnterCriticalSection(&crit_sec);
        if ( pcpbd->dlgAllocated )
        {
            result = delDialog(pcpbd, c);
        }
        LeaveCriticalSection(&crit_sec);
    }
    return result;
}

/**
 *  This function is only called after the underlying dialog has been
 *  successfully created.  It then sets the dialog handle into the various CSelf
 *  structs that need it.
 *
 * @param c      Method context we are operating in.
 * @param pcpbd  CSelf pointer of the dialog
 */
void setDlgHandle(RexxThreadContext *c, pCPlainBaseDialog pcpbd)
{
    HWND hDlg = pcpbd->hDlg;

    pcpbd->enCSelf->hDlg = hDlg;
    pcpbd->weCSelf->hwnd = hDlg;

    pcpbd->wndBase->hwnd = hDlg;
    pcpbd->wndBase->rexxHwnd = c->RequestGlobalReference(pointer2string(c, hDlg));
}


/**
 * Gets the window handle of the dialog control that has the focus.  The call to
 * GetFocus() needs to run in the window thread of the dialog to ensure that the
 * correct handle is obtained.
 *
 * @param c       Method context we are operating in.
 * @param hDlg    Handle to the dialog of interest.
 *
 * @return  The window handle of the dialog control with the focus, or 0 on
 *          failure.
 */
RexxObjectPtr oodGetFocus(RexxMethodContext *c, HWND hDlg)
{
   HWND hwndFocus = (HWND)SendMessage(hDlg, WM_USER_GETFOCUS, 0,0);
   return pointer2string(c, hwndFocus);
}


/**
 * Brings the specified window to the foreground and returns the previous
 * foreground window.
 *
 * @param c
 * @param hwnd
 *
 * @return The previous window handle as a Rexx object, or 0 on error.
 *
 * @remarks  SetForegroundWindow() is not documented as setting last error.  So,
 *           if it fails, last error is checked.  If it is set, it is used.  If
 *           it is not set, we arbitrarily use ERROR_NOTSUPPORTED.  On XP at
 *           least, last error is set to 5, access denied.
 */
RexxObjectPtr oodSetForegroundWindow(RexxMethodContext *c, HWND hwnd)
{
    oodResetSysErrCode(c->threadContext);

    if ( hwnd == NULL || ! IsWindow(hwnd) )
    {
        oodSetSysErrCode(c->threadContext, ERROR_INVALID_WINDOW_HANDLE);
        return TheZeroObj;
    }

    HWND hwndPrevious = GetForegroundWindow();
    if ( hwndPrevious != hwnd )
    {
        SetLastError(0);
        if ( SetForegroundWindow(hwnd) == 0 )
        {
            uint32_t rc = GetLastError();
            if ( rc == 0 )
            {
                rc = ERROR_NOT_SUPPORTED;
            }
            oodSetSysErrCode(c->threadContext, rc);
            return TheZeroObj;
        }
    }
    return pointer2string(c, hwndPrevious);
}

RexxMethod1(RexxObjectPtr, pbdlg_init_cls, OSELF, self)
{
    if ( isOfClassType(context, self, PLAINBASEDIALOG_CLASS) )
    {
        ThePlainBaseDialogClass = (RexxClassObject)self;
        context->RequestGlobalReference(ThePlainBaseDialogClass);

        RexxBufferObject buf = context->NewBuffer(sizeof(CPlainBaseDialogClass));
        if ( buf != NULLOBJECT )
        {
            pCPlainBaseDialogClass pcpbdc = (pCPlainBaseDialogClass)context->BufferData(buf);

            strcpy(pcpbdc->fontName, DEFAULT_FONTNAME);
            pcpbdc->fontSize = DEFAULT_FONTSIZE;
            context->SetObjectVariable("CSELF", buf);
        }
    }
    return NULLOBJECT;
}

RexxMethod2(RexxObjectPtr, pbdlg_setDefaultFont_cls, CSTRING, fontName, uint32_t, fontSize)
{
    pCPlainBaseDialogClass pcpbdc = dlgToClassCSelf(context);

    if ( strlen(fontName) > (MAX_DEFAULT_FONTNAME - 1) )
    {
        stringTooLongException(context->threadContext, 1, MAX_DEFAULT_FONTNAME, strlen(fontName));
    }
    else
    {
        strcpy(pcpbdc->fontName, fontName);
        pcpbdc->fontSize = fontSize;
    }
    return NULLOBJECT;
}

RexxMethod1(CSTRING, pbdlg_getFontName_cls, CSELF, pCSelf)
{
    pCPlainBaseDialogClass pcpbdc = dlgToClassCSelf(context);
    return pcpbdc->fontName;
}
RexxMethod1(uint32_t, pbdlg_getFontSize_cls, CSELF, pCSelf)
{
    pCPlainBaseDialogClass pcpbdc = dlgToClassCSelf(context);
    return pcpbdc->fontSize;
}


/** PlainBaseDialog::new()
 *
 *  The new() method is implemented to provide a clean way to prevent more than
 *  MAXDIALOGS dialog objects from being instantiated.
 *
 *  If we try to enforce this in PlaingBaseDialog::init() we run into the
 *  problem of having to either:
 *
 *  Raise an error condition and have the other dialog threads pretty much hang.
 *
 *  Raise an error condition and issue an interpreter halt, pretty much shutting
 *  everything down.
 *
 *  Here we intercept the too many dialogs condition and return a proxy object
 *  that has only 1 method, the initCode.  For the proxy object, initCode always
 *  returns 1.  If the programmer checks the init code, they will know the
 *  dialog is not vaild.  If they try to use it, it won't work.
 *
 */
RexxMethod2(RexxObjectPtr, pbdlg_new_cls, ARGLIST, args, SUPER, superClass)
{
    RexxObjectPtr dlgObj = TheNilObj;

    if ( CountDialogs >= MAXDIALOGS )
    {
        char buf[128];
        _snprintf(buf, sizeof(buf),
                  "The number of active dialogs has\n"
                  "reached the maximum (%d) allowed\n\n"
                  "No more dialogs can be instantiated", MAXDIALOGS);
        MessageBox(NULL, buf, "ooDialog Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);

        RexxClassObject proxyClass = rxGetContextClass(context, "DIALOGPROXY");
        if ( proxyClass != NULLOBJECT )
        {
            dlgObj = context->SendMessage0(proxyClass, "NEW");
        }
        goto done_out;
    }

    dlgObj = context->ForwardMessage(NULLOBJECT, NULL, superClass, NULL);

done_out:
    return dlgObj;
}


/** PlainBaseDialog::init()
 *
 *  The initialization of the base dialog.
 *
 *
 *  @remarks  Prior to 4.0.1, if something failed here, the 'init code' was set
 *            to non-zero.  One problem with this is that it relies on the user
 *            checking the init code and *not* using the dialog object if it is
 *            an error code.  Since we can not rely on that, in order to prevent
 *            a crash, the internal code would need to constantly check that the
 *            CSELF pointer was not null.
 *
 *            Rather than do that, the code now raises an error condition if
 *            something fails.  (This seems to be the right thing to do anyway.)
 *            That works fine if there is only one dialog running.  But, if more
 *            than one dialog is running, the other threads just hang.  Not real
 *            clean for the user.  So, in addition, we use the interpreter
 *            instance to do a Halt(), which pretty much shuts everything down.
 *
 *            As ooDialog was being converted to the C++ APIs, the initCode
 *            attribute came to have less and less meaning.  With this change,
 *            it is mostly not needed.  The one exception is when the maximum
 *            number of active dialogs has been reached.  (See
 *            PlainBaseDialog::new().)
 */
RexxMethod5(wholenumber_t, pbdlg_init, CSTRING, library, RexxObjectPtr, resource,
            OPTIONAL_RexxObjectPtr, dlgDataStem, OPTIONAL_RexxObjectPtr, hFile, OSELF, self)
{
    // This is an error return, but see remarks.
    wholenumber_t result = 1;

    // Before processing the arguments, do everything that might raise an error
    // condition.

    // Get a buffer for the PlainBaseDialog CSelf.
    RexxBufferObject cselfBuffer = context->NewBuffer(sizeof(CPlainBaseDialog));
    if ( cselfBuffer == NULLOBJECT )
    {
        goto terminate_out;
    }

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->BufferData(cselfBuffer);
    memset(pcpbd, 0, sizeof(CPlainBaseDialog));

    // Initialize the window base.
    pCWindowBase pWB;
    if ( ! initWindowBase(context, NULL, self, &pWB) )
    {
        goto terminate_out;
    }
    pcpbd->wndBase = pWB;

    if ( ! initWindowExtensions(context, self, NULL, pcpbd->wndBase, pcpbd) )
    {
        goto terminate_out;
    }

    // Initialize the event notification mixin class.  The only thing that could
    // fail is getting a buffer from the interpreter kernel.
    pCEventNotification pEN = NULL;
    if ( ! initEventNotification(context, pcpbd, self, &pEN) )
    {
        goto terminate_out;
    }
    pcpbd->enCSelf = pEN;

    size_t len = strlen(library);
    if ( len >= MAX_LIBRARYNAME )
    {
        stringTooLongException(context->threadContext, 1, MAX_LIBRARYNAME, len);
        goto terminate_out;
    }
    strcpy(pcpbd->library, library);
    pcpbd->resourceID = resource;

    if ( context->IsOfType(self, "CONTROLDIALOG") )
    {
        pcpbd->isControlDlg = true;
    }
    else if ( context->IsOfType(self, "CATEGORYDIALOG") )
    {
        pcpbd->isCategoryDlg = true;
    }
    else if ( context->IsOfType(self, "PROPERTYSHEETDIALOG") )
    {
        pcpbd->isPropSheetDlg = true;
    }
    else if ( context->IsOfType(self, "PROPERTYSHEETPAGE") )
    {
        pcpbd->isPageDlg = true;
    }

    pcpbd->interpreter  = context->threadContext->instance;
    pcpbd->dlgAllocated = true;
    pcpbd->autoDetect   = (pcpbd->isPropSheetDlg ? FALSE : TRUE);
    pcpbd->rexxSelf     = self;

    context->SetObjectVariable("CSELF", cselfBuffer);

    // Set our default font to the PlainBaseDialog class default font.
    pCPlainBaseDialogClass pcpbdc = dlgToClassCSelf(context);
    strcpy(pcpbd->fontName, pcpbdc->fontName);
    pcpbd->fontSize = pcpbdc->fontSize;

    // TODO at this point calculate the true dialog base units and set them into CPlainBaseDialog.

    if ( ! (pcpbd->isControlDlg || pcpbd->isPageDlg) )
    {
        pcpbd->previous = TopDlg;
        pcpbd->tableIndex = CountDialogs;
        CountDialogs++;
        DialogTable[pcpbd->tableIndex] = pcpbd;
    }

    // Now process the arguments and do the rest of the initialization.
    result = 0;

    if ( argumentExists(3) )
    {
        context->SetObjectVariable("DLGDATA.", dlgDataStem);
        context->SetObjectVariable("USESTEM", TheTrueObj);
    }
    else
    {
        context->SetObjectVariable("DLGDATA.", TheNilObj);
        context->SetObjectVariable("USESTEM", TheFalseObj);
    }

    context->SetObjectVariable("PARENTDLG", TheNilObj);
    context->SetObjectVariable("FINISHED", TheFalseObj);
    context->SetObjectVariable("PROCESSINGLOAD", TheFalseObj);

    context->SendMessage1(self, "CHILDDIALOGS=", rxNewList(context));       // self~childDialogs = .list~new
    context->SendMessage0(self, "INITAUTODETECTION");                       // self~initAutoDetection
    context->SendMessage1(self, "DATACONNECTION=", context->NewArray(50));  // self~dataConnection = .array~new(50)
    context->SendMessage1(self, "AUTOMATICMETHODS=", rxNewQueue(context));  // self~autoMaticMethods = .queue~new

    context->SendMessage1(self, "MENUBAR=", TheNilObj);
    context->SendMessage1(self, "ISLINKED=", TheFalseObj);

    RexxDirectoryObject constDir = context->NewDirectory();
    context->SendMessage1(self, "CONSTDIR=", constDir);                     // self~constDir = .directory~new

    context->DirectoryPut(constDir, context->Int32(IDC_STATIC),       "IDC_STATIC");       // -1
    context->DirectoryPut(constDir, context->Int32(IDOK      ),       "IDOK");             // 1
    context->DirectoryPut(constDir, context->Int32(IDCANCEL  ),       "IDCANCEL");         // 2
    context->DirectoryPut(constDir, context->Int32(IDABORT   ),       "IDABORT");          //  ...
    context->DirectoryPut(constDir, context->Int32(IDRETRY   ),       "IDRETRY");
    context->DirectoryPut(constDir, context->Int32(IDIGNORE  ),       "IDIGNORE");
    context->DirectoryPut(constDir, context->Int32(IDYES     ),       "IDYES");
    context->DirectoryPut(constDir, context->Int32(IDNO      ),       "IDNO");
    context->DirectoryPut(constDir, context->Int32(IDCLOSE   ),       "IDCLOSE");
    context->DirectoryPut(constDir, context->Int32(IDHELP    ),       "IDHELP");           // 9
    context->DirectoryPut(constDir, context->Int32(IDTRYAGAIN),       "IDTRYAGAIN");       // 10
    context->DirectoryPut(constDir, context->Int32(IDCONTINUE),       "IDCONTINUE");       // 11
    context->DirectoryPut(constDir, context->Int32(IDI_DLG_OODIALOG), "IDI_DLG_OODIALOG"); // This is 12
    context->DirectoryPut(constDir, context->Int32(IDI_DLG_APPICON),  "IDI_DLG_APPICON");
    context->DirectoryPut(constDir, context->Int32(IDI_DLG_APPICON2), "IDI_DLG_APPICON2");
    context->DirectoryPut(constDir, context->Int32(IDI_DLG_OOREXX),   "IDI_DLG_OOREXX");
    context->DirectoryPut(constDir, context->Int32(IDI_DLG_DEFAULT),  "IDI_DLG_DEFAULT");

    if ( argumentExists(4) )
    {
        context->SendMessage1(self, "PARSEINCLUDEFILE", hFile);
    }

    if ( context->IsOfType(self, "OwnerDialog") )
    {
        RexxClassObject ownerClass = rxGetContextClass(context, "OWNERDIALOG");
        RexxArrayObject args = context->ArrayOfOne(cselfBuffer);
        context->ForwardMessage(NULL, NULL, ownerClass, args);
    }

    goto done_out;

terminate_out:
    context->threadContext->instance->Halt();
    return result;

done_out:
    pWB->initCode = result;
    return result;
}

RexxMethod1(RexxObjectPtr, pbdlg_unInit, CSELF, pCSelf)
{
    if ( pCSelf != NULLOBJECT )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
        printf("PlainBaseDialog::uninit() hDlg=%p isAllocated=%d  Dialog is a ", pcpbd->hDlg, pcpbd->dlgAllocated);
        dbgPrintClassID(context, pcpbd->rexxSelf);

        EnterCriticalSection(&crit_sec);

        if ( pcpbd->dlgAllocated )
        {
            delDialog(pcpbd, context->threadContext);
        }

        LeaveCriticalSection(&crit_sec);

        pCWindowBase pcwb = pcpbd->wndBase;
        if ( pcwb->rexxHwnd != TheZeroObj )
        {
            context->ReleaseGlobalReference(pcwb->rexxHwnd);
            pcwb->rexxHwnd = TheZeroObj;
        }

        // It is tempting to think dlgProcContext should be checked here, and,
        // if not null do a DetachThread().  However, the DetachThread() is, and
        // should be, done from the message loop thread, not here.  The only
        // time dlgProcContext is not null is if the interpreter is being
        // abruptly terminated (Ctrl-C, syntax condition, etc..)  And in this
        // case the DetachThread() is not needed.
    }

    return TheZeroObj;
}

/** PlainBaseDialog::library  [attribute get]
 */
RexxMethod1(CSTRING, pbdlg_getLibrary, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->library );
}

/** PlainBaseDialog::resourceID  [attribute get]
 */
RexxMethod1(RexxObjectPtr, pbdlg_getResourceID, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->resourceID );
}

/** PlainBaseDialog::dlgHandle  [attribute get] / PlainBaseDialog::getSelf()
 */
RexxMethod1(RexxObjectPtr, pbdlg_getDlgHandle, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->wndBase->rexxHwnd );
}

/** PlainBaseDialog::autoDetect  [attribute get]
 */
RexxMethod1(RexxObjectPtr, pbdlg_getAutoDetect, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->autoDetect ? TheTrueObj : TheFalseObj );
}
/** PlainBaseDialog::autoDetect  [attribute set]
 */
RexxMethod2(CSTRING, pbdlg_setAutoDetect, logical_t, on, CSELF, pCSelf)
{
    ((pCPlainBaseDialog)pCSelf)->autoDetect = on;
    return NULLOBJECT;
}

/** PlainBaseDialog::fontName  [attribute get]
 *  PlainBaseDialog::fontSize  [attribute get]
 */
RexxMethod2(RexxObjectPtr, pbdlg_getFontNameSize, NAME, method, CSELF, pCSelf)
{
    RexxObjectPtr result;
    if ( *(method + 4) == 'N' )
    {
        result =  context->String(((pCPlainBaseDialog)pCSelf)->fontName);
    }
    else
    {
        result = context->UnsignedInt32(((pCPlainBaseDialog)pCSelf)->fontSize);
    }
    return result;
}

/** PlainBaseDialog::fontName  [attribute set private]
 */
RexxMethod2(RexxObjectPtr, pbdlg_setFontName_pvt, CSTRING, name, CSELF, pCSelf)
{
    if ( strlen(name) > (MAX_DEFAULT_FONTNAME - 1) )
    {
        stringTooLongException(context->threadContext, 1, MAX_DEFAULT_FONTNAME, strlen(name));
    }
    else
    {
        strcpy(((pCPlainBaseDialog)pCSelf)->fontName, name);
    }
    return NULLOBJECT;
}

/** PlainBaseDialog::fontSize  [attribute set private]
 */
RexxMethod2(RexxObjectPtr, pbdlg_setFontSize_pvt, uint32_t, size, CSELF, pCSelf)
{
    ((pCPlainBaseDialog)pCSelf)->fontSize = size;
    return NULLOBJECT;
}

/** PlaingBaseDialog::setDlgFont()
 *
 *  Sets the font that will be used for the font of the underlying Windows
 *  dialog, when it is created.  This is primarily of use in a UserDialog or a
 *  subclasses of a UserDialog.
 *
 *  In a ResDialog, the font of the compiled binary resource will be used and
 *  the font set by this method has no bearing.  In a RcDialog, if the resource
 *  script file specifies the font, that font will be used.
 *
 *  Likewise, in a UserDialog, if the programmer specifies a font in the create
 *  method call (or createCenter() method call) the specified font over-rides
 *  what is set by this method.
 *
 *  However, setting the font through the setDlgFont() method allows the true
 *  dialog unit values to be correctly calculated.  That is the primary use for
 *  this method.  A typical sequence might be:
 *
 *  dlg = .MyDialog~new
 *  dlg~setFont("Tahoma", 12)
 *  ...
 *  ::method defineDialog
 *  ...
 *
 *  @param  fontName  The font name, such as Tahoma.  The name must be less than
 *                    256 characters in length.
 *  @param  fontSize  [optional]  The point size of the font, for instance 10.
 *                    The default if this argument is omitted is 8.
 *
 *  @return  0, always.
 */
RexxMethod3(RexxObjectPtr, pbdlg_setDlgFont, CSTRING, fontName, OPTIONAL_uint32_t, fontSize, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    if ( strlen(fontName) > (MAX_DEFAULT_FONTNAME - 1) )
    {
        stringTooLongException(context->threadContext, 1, MAX_DEFAULT_FONTNAME, strlen(fontName));
    }
    else
    {
        if ( argumentOmitted(2) )
        {
            fontSize = DEFAULT_FONTSIZE;
        }
        strcpy(pcpbd->fontName, fontName);
        pcpbd->fontSize = fontSize;

        // TODO at this point calculate the true dialog base units from the font
        // and set them into CPlainBaseDialog.
    }
    return TheZeroObj;
}


/** PlainBaseDialog::sendMessageToControl()
 *  PlainBaseDialog::sendMessageToControlH()
 *
 *  Sends a window message to the specified dialog control.
 *
 *  @param  rxID    The resource ID of the control, may be numeric or symbolic.
 *
 *  @param  wm_msg  The Windows window message ID.  This can be specified in
 *                  either "0xFFFF" or numeric format.
 *
 *  @param  _wParam  The WPARAM value for the message.
 *  @param  _lParam  The LPARAM value for the message.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.  sendMessageToControl() returns the result as a number.
 *          sendMessageToControlH() returns the result as an operating system
 *          handle.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  sendMessageToItem(), which forwards to this method, a method of
 *            DialogExtensions, but was moved back into PlainBaseDialog after
 *            the 4.0.0 release.  Therefore, sendMessageToControl() can not
 *            raise an exception for a bad resource ID, sendMessageToControlH()
 *            can.
 */
RexxMethod6(RexxObjectPtr, pbdlg_sendMessageToControl, RexxObjectPtr, rxID, CSTRING, wm_msg,
            RexxObjectPtr, _wParam, RexxObjectPtr, _lParam, NAME, method, CSELF, pCSelf)
{
    bool doIntReturn = (strlen(method) == 20 ? true : false);

    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl == NULL )
    {
        if ( doIntReturn )
        {
            return TheNegativeOneObj;
        }
        else
        {
            return invalidTypeException(context->threadContext, 1, " resource ID, there is no matching dialog control");
        }
    }

    return sendWinMsgGeneric(context, hCtrl, wm_msg, _wParam, _lParam, 2, doIntReturn);
}


/** PlainBaseDialog::sendMessageToWindow()
 *  PlainBaseDialog::sendMessageToWindowH()
 *
 *  Sends a window message to the specified window.
 *
 *  @param  _hwnd   The handle of the window the message is sent to.
 *  @param  wm_msg  The Windows window message ID.  This can be specified in
 *                  either "0xFFFF" or numeric format.
 *
 *  @param  _wParam  The WPARAM value for the message.
 *  @param  _lParam  The LPARAM value for the message.
 *
 *  @return The result of sending the message, as returned by the operating
 *          system.  sendMessageToWindow() returns the result as a number and
 *          sendMessageToWindowH() returns the the result as an operating systme
 *          handle.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod6(RexxObjectPtr, pbdlg_sendMessageToWindow, CSTRING, _hwnd, CSTRING, wm_msg,
            RexxObjectPtr, _wParam, RexxObjectPtr, _lParam, NAME, method, CSELF, pCSelf)
{
    HWND hwnd = (HWND)string2pointer(_hwnd);

    bool doIntReturn = (strlen(method) == 19 ? true : false);

    return sendWinMsgGeneric(context, hwnd, wm_msg, _wParam, _lParam, 2, doIntReturn);
}

/** PlainBaseDialog::get()
 *
 *  Returns the handle of the "top" dialog.
 *
 *  @return  The handle of the top dialog, or the null handle if there is no
 *           top dialog.
 *
 *  @remarks  This is a documented method from the original ooDialog
 *            implementation.  The original documentaion said: "The Get method
 *            returns the handle of the current Windows dialog."  The
 *            implementation has always been to get the handle of the TopDlg.  I
 *            have never understood what the point of this method is, since the
 *            TopDlg, usually, just reflects the last dialog created.
 */
RexxMethod0(RexxObjectPtr, pbdlg_get)
{
    if (TopDlg && TopDlg->hDlg)
    {
        return pointer2string(context, TopDlg->hDlg);
    }
    else
    {
        return TheZeroObj;
    }
}

/** PlainBaseDialog::getWindowText()
 *
 *  Gets the text of the specified window.
 *
 *  For a window with a frame, this is the window title.  But for a dialog
 *  control, this is the text for the control.  This of course varies depending
 *  on the type of control.  For a button, it is the button label, for an edit
 *  control it is the edit text, etc..
 *
 *  @param  hwnd  The handle of the window.
 *
 *  @return  On success, the window text, which could be the empty string.  On
 *           failure, the empty string.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod1(RexxStringObject, pbdlg_getWindowText, POINTERSTRING, hwnd)
{
    RexxStringObject result = context->NullString();
    rxGetWindowText(context, (HWND)hwnd, &result);
    return result;
}

/** PlainBaseDialog::getTextSizeDu()
 *
 *  Gets the size (width and height) in dialog units for any given string.
 *
 *  @param  text  The string whose size is needed.
 *
 *  @return  The size needed for the string in a .Size object.
 */
RexxMethod2(RexxObjectPtr, pbdlg_getTextSizeDu, CSTRING, text, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    HWND hDlg     = pcpbd->hDlg;
    SIZE textSize = {0};

    // If hwnd is null, this will get a DC for the entire screen and that should be ok.
    HDC hdc = GetDC(hDlg);
    if ( hdc == NULL )
    {
        systemServiceExceptionCode(context->threadContext, API_FAILED_MSG, "GetDC");
        goto error_out;
    }

    HFONT dlgFont = NULL;
    bool createdFont = false;

    if ( hDlg == NULL )
    {
        dlgFont = createFontFromName(hdc, pcpbd->fontName, pcpbd->fontSize);
        if ( dlgFont != NULL )
        {
            createdFont = true;
        }
    }
    else
    {
        dlgFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
    }

    // If the font is still null we use the stock system font.  Even if we have
    // the dialog handle, the font could be null if WM_SETFONT was not used.  In
    // this case the stock system font will be correct.  If the
    // createFontFromName() failed, well that is unlikely.  Just use the stock
    // system font in that case.
    if ( dlgFont == NULL )
    {
        dlgFont = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    HFONT hOldFont = (HFONT)SelectObject(hdc, dlgFont);

    GetTextExtentPoint32(hdc, text, (int)strlen(text), &textSize);
    screenToDlgUnit(hdc, (POINT *)&textSize, 1);

    SelectObject(hdc, hOldFont);
    ReleaseDC(hDlg, hdc);

    if ( createdFont )
    {
        DeleteObject(dlgFont);
    }
    return rxNewSize(context, textSize.cx, textSize.cy);

error_out:
    return NULLOBJECT;
}


/** PlainBaseDialog::getTextSizeDlg()
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
 *                       size.
 *
 *  @param  fontSize     Optional. If specified, use this font size with
 *                       fontName to calculate the size.  The default if omitted
 *                       is 8.  This arg is ignored if fontName is omitted.
 *
 *  @param  hwndFontSrc  Optional. Use this window's font to calculate the size.
 *                       This arg is always ignored if fontName is specified.
 *
 *  @note The normal useage for this method would be, before the underlying
 *        dialog is created:
 *
 *          dlg~setDlgFont("fontName", fontSize)
 *          dlg~getTextSizeDlg("some text")
 *
 *        or, after the underlying dialog is created, just:
 *
 *          dlg~getTextSizeDlg("some text")
 *
 *        The convoluted use of the optional arguments are needed to maintain
 *        backwards compatibility with the pre 4.0.0 ooDialog, the Rexx
 *        programmer should be strongly discouraged from using them.
 *
 *        In addition, a version of this method is mapped to the DialogControl
 *        class.  This also is done only for backwards compatibility.  There is
 *        no logical reason for this to be a method of a dialog control.
 */
RexxMethod5(RexxObjectPtr, pbdlg_getTextSizeDlg, CSTRING, text, OPTIONAL_CSTRING, fontName,
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
            nullObjectException(context->threadContext, "window handle", 4);
            return NULLOBJECT;
        }
        hwndSrc = (HWND)hwndFontSrc;
    }

    SIZE textSize = {0};
    if ( getTextSize(context, text, fontName, fontSize, hwndSrc, self, &textSize) )
    {
        return rxNewSize(context, textSize.cx, textSize.cy);
    }
    return NULLOBJECT;
}


/** PlainBaseDialog::show()
 *
 *  Shows the dialog in the manner specified by the keyword option.  The dialog
 *  can be hidden, minimized, made visible, brought to the foreground, etc.
 *
 *  @param  keyword  A single keyword that controls how the dialog is shown.
 *                   Case is not significant.
 *
 *  @return  True if the dialog was previously visible.  Return false if the
 *           window was previously hidden.  Note that this is not a success or
 *           failure return, rather it is what the Windows API returns.
 *
 *  @note  The DEFAULT and NORMAL keywords are both equivalent to SW_SHOWNORMAL.
 *
 *         Key words are NORMAL DEFAULT SHOWTOP RESTORE MIN MAX HIDE INACTIVE
 *
 *  @remarks  Note!  This method over-rides the WindowBase::show() method.  The
 *            WindowBase::show() method, does a show normal only.  This method
 *            takes a number of keyword options, whereas WindowBase::show()
 *            method takes no options.
 *
 *            Therefore, all dialog objects have this show method.  All dialog
 *            control objects have the no-option show() method.
 *
 *            The WindowBase::display() method is similar to this show(), but
 *            with fewer, and somewhat different, keywords.
 *
 *            PlainBaseDialog::restore() also maps to this function.  restore()
 *            takes no options.
 *
 *            Older ooDialog did this: SHOWTOP used the SW_SHOW flag after
 *            bringing the dialog to the foreground.  If there was a keyword,
 *            but no keyword match, then SW_SHOW was used.  If the keyword was
 *            omitted, then SW_SHOWNORMAL was used.  This bevavior is preserved.
 */
RexxMethod3(logical_t, pbdlg_show, OPTIONAL_CSTRING, options, NAME, method, CSELF, pCSelf)
{
    HWND hwnd = getPBDWindow(context, pCSelf);
    if ( hwnd == NULL )
    {
        return FALSE;
    }

    char flag = (*method == 'R' ? 'R' : 'N')  ;

    if ( options != NULL && *options != '\0' && flag != 'R' )
    {
        switch ( toupper(*options) )
        {
            case 'S' :
                SetForegroundWindow(hwnd);
                flag = 'S';
                break;

            case 'M' :
                flag = (StrCmpNI("MIN", options, 3) == 0 ? 'M' : 'X');
                break;

            case 'H' :
                flag = 'H';
                break;

            case 'I' :
                flag = 'I';
                break;

            case 'R' :
                flag = 'R';
                break;

            case 'D' :
                flag = 'D';
                break;

            case 'N' :
                flag = 'N';
                break;

            default  :
                flag = 'S';
                break;

        }
    }
    return showWindow(hwnd, flag);
}


/** PlainBaseDialog::showWindow() / PlainBaseDialog::hideWindow()
 *  PlainBaseDialog::showWindowFast() / PlainBaseDialog::hideWindowFast()
 *
 *  Hides or shows the specified window, normally, or 'fast'.  "Fast" means
 *  the visible flag is set, but the window is not forced to update.
 *
 *  @param   hwnd  The handle of the window to be shown.
 *
 *  @return  1 if the window was previously visible.  Return 0 if the
 *           window was previously hidden.  The return for these methods was not
 *           previously documented, but this matches what was returned.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, pbdlg_showWindow, POINTERSTRING, hwnd, NAME, method, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    if ( *method == 'S' )
    {
        ((pCPlainBaseDialog)pCSelf)->activeChild = (HWND)hwnd;
    }

    logical_t rc;
    if ( strstr("FAST", method) != NULL )
    {
        rc = showFast((HWND)hwnd, *method);
    }
    else
    {
        rc = showWindow((HWND)hwnd, *method);
    }

    return (rc ? TheOneObj : TheZeroObj);
}


/** PlainBaseDialog::showControl() / PlainBaseDialog::hideControl()
 *  PlainBaseDialog::showControlFast() / PlainBaseDialog::hideControlFast()
 *
 *  Hides or shows the dialog control window, normally, or 'fast'.  "Fast" means
 *  the visible flag is set, but the window is not forced to update.
 *
 *  @param   rxID  The resource ID of the dialog control, may be numeric or
 *                 symbolic.
 *
 *  @return  1 if the window was previously visible.  Return 0 if the
 *           window was previously hidden.  Return -1 if the resource ID was no
 *           good. The return for these methods was not previously documented,
 *           but this matches what was returned.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, pbdlg_showControl, RexxObjectPtr, rxID, NAME, method, CSELF, pCSelf)
{
    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl == NULL )
    {
        return TheNegativeOneObj;
    }

    logical_t rc;
    if ( strstr("FAST", method) != NULL )
    {
        rc = showFast(hCtrl, *method);
    }
    else
    {
        rc = showWindow(hCtrl, *method);
    }

    return (rc ? TheOneObj : TheZeroObj);
}

/** PlainBaseDialog::center()
 *
 *  Moves the dialog to the center of screen.
 *
 *  The dialog can be centered in the physical screen, or optionally centered in
 *  the screen work area. The work area is the portion of the screen not
 *  obscured by the system taskbar or by application desktop toolbars.
 *
 *  @param options  [OPTIONAL]  A string of zero or more blank separated
 *                  keywords. The key words control the behavior when the dialog
 *                  is moved.
 *  @param workArea [OPTIONAL]  If true, the dialog is centered in the work area
 *                  of the screen.  The default, false, centers the dialog in
 *                  the physical screen area.
 *
 *  @return  True on success, otherwise false.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, pbdlg_center, OPTIONAL_CSTRING, options, OPTIONAL_logical_t, workArea, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    HWND hwnd = getPBDWindow(context, pCSelf);
    if ( hwnd = NULL )
    {
        return TheFalseObj;
    }

    RECT r;
    if ( GetWindowRect(hwnd, &r) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheFalseObj;
    }

    if ( workArea )
    {
        RECT wa;
        if ( SystemParametersInfo(SPI_GETWORKAREA, 0, &wa, 0) == 0 )
        {
            oodSetSysErrCode(context->threadContext);
            return TheFalseObj;
        }
        r.left = wa.left + (((wa.right - wa.left) - (r.right - r.left)) / 2);
        r.top = wa.top + (((wa.bottom - wa.top) - (r.bottom - r.top)) / 2);
    }
    else
    {
        uint32_t cxScr = GetSystemMetrics(SM_CXSCREEN);
        uint32_t cyScr = GetSystemMetrics(SM_CYSCREEN);

        r.left = (cxScr - (r.right - r.left)) / 2;
        r.top = (cyScr - (r.bottom - r.top)) / 2;
    }

    uint32_t opts = parseShowOptions(options);
    opts = (opts | SWP_NOSIZE) & ~SWP_NOMOVE;

    if ( SetWindowPos(hwnd, NULL, r.left, r.top, r.right, r.bottom, opts) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheFalseObj;
    }
    return TheTrueObj;
}

/** PlainBaseDialog::setWindowText()
 *
 *  Sets the text for the specified window.
 *
 *  @param  hwnd  The handle of the window.
 *  @param  text  The text to be set.
 *
 *  @return  0 for success, 1 for error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(wholenumber_t, pbdlg_setWindowText, POINTERSTRING, hwnd, CSTRING, text)
{
    oodResetSysErrCode(context->threadContext);
    if ( SetWindowText((HWND)hwnd, text) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return 1;
    }
    return 0;
}

/** PlainBaseDialog::toTheTop()
 *
 *  Brings this dialog to the foreground.
 *
 *  @return  The handle of the window that previously was the foreground on
 *           success, 0 on failure.
 *
 *  @note  Sets the .SystemErrorCode.  In very rare cases, there might not be a
 *         previous foreground window and 0 would be returned.  In this case,
 *         the .SystemErrorCode will be 0, otherwise the .SystemErrorCode will
 *         not be 0.
 *
 *  @note  Windows no longer allows a program to arbitrarily change the
 *         foreground window.
 *
 *         The system restricts which processes can set the foreground window. A
 *         process can set the foreground window only if one of the following
 *         conditions is true:
 *
 *         The process is the foreground process.
 *         The process was started by the foreground process.
 *         The process received the last input event.
 *         There is no foreground process.
 *         No menus are active.
 *
 *         With this change, an application cannot force a window to the
 *         foreground while the user is working with another window.
 */
RexxMethod1(RexxObjectPtr, pbdlg_toTheTop, CSELF, pCSelf)
{
    HWND hwnd = getPBDWindow(context, pCSelf);
    if ( hwnd = NULL )
    {
        return TheZeroObj;
    }
    return oodSetForegroundWindow(context, hwnd);
}

/** PlainBaseDialog::getFocus()
 *
 *  Retrieves the window handle of the dialog control with the focus.
 *
 *  @return  The window handle on success, 0 on failure.
 */
RexxMethod1(RexxObjectPtr, pbdlg_getFocus, CSELF, pCSelf)
{
    HWND hwnd = getPBDWindow(context, pCSelf);
    if ( hwnd = NULL )
    {
        return TheZeroObj;
    }
    return oodGetFocus(context, hwnd);
}

/** PlainBaseDialog::setFocus()
 *
 *  Sets the focus to the dialog control specified.
 *
 *  @param  hwnd  The window handle of the dialog control to recieve the focus.
 *
 *  PlainBaseDialog::setFocusToWindow()
 *
 *  Sets the focus to the dialog or other top-level window specified.
 *
 *  @param  hwnd  The window handle of the dialog or other top-level window that
 *                will be brought to the foreground.
 *
 *  @return  On success, the window handle of the dialog control that previously
 *           had the focus.  Otherwise, 0, if the focus was changed, but the
 *           previous focus could not be determined, and -1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, pbdlg_setFocus, RexxStringObject, hwnd, NAME, method, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    HWND hDlg = getPBDWindow(context, pCSelf);
    if ( hDlg == NULL )
    {
        return TheNegativeOneObj;
    }

    HWND focusNext = (HWND)string2pointer(context, hwnd);
    if ( ! IsWindow(focusNext) )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_WINDOW_HANDLE);
        return TheNegativeOneObj;
    }

    RexxObjectPtr previousFocus = oodGetFocus(context, hDlg);
    if ( strlen(method) > 7 )
    {
        if ( oodSetForegroundWindow(context, focusNext) == TheZeroObj )
        {
            return TheNegativeOneObj;
        }
    }
    else
    {
        SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)focusNext, TRUE);
    }
    return previousFocus;
}

/** PlainBaseDialog::TabToNext()
 *
 *  Sets the focus to the next tab stop dialog control in the dialog and returns
 *  the window handle of the dialog control that currently has the focus.
 *
 *  PlainBaseDialog::TabToPrevious()
 *
 *  Sets the focus to the previous tab stop dialog control in the dialog and
 *  returns the window handle of the dialog control that currently has the
 *  focus.
 *
 *  @return  On success, the window handle of the dialog control that previously
 *           had the focus.  Otherwise, 0, if the focus was changed, but the
 *           previous focus could not be determined, and -1 on error.
 *
 *  @note  Sets the .SystemErrorCode, but there is nothing that would change it
 *         from 0.
 */
RexxMethod2(RexxObjectPtr, pbdlg_tabTo, NAME, method, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    HWND hDlg = getPBDWindow(context, pCSelf);
    if ( hDlg == NULL )
    {
        return TheNegativeOneObj;
    }

    RexxObjectPtr previousFocus = oodGetFocus(context, hDlg);
    if ( method[5] == 'N' )
    {
        SendMessage(hDlg, WM_NEXTDLGCTL, 0, FALSE);
    }
    else
    {
        SendMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
    }
    return previousFocus;
}

/** PlainBaseDialog::backgroundBitmap()
 *
 *  Uses the specified bitmap for the background of the dialog.
 *
 *  The bitmap is stretched or shrunk to fit the size of the dialog and the
 *  bitmap is drawn internally by ooDialog.  Contrast this with
 *  tiledBackgroundBitmap().
 *
 *  @param  bitmapFileName  The name of a bitmap file to use as the dilaog
 *                          background.
 *
 *  @return  True on success, false for some error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *         The .SystemErrorCode is set to: 2 ERROR_FILE_NOT_FOUND The system
 *         cannot find the file specified, if there is a problem with the bitmap
 *         file.  This might be because the file could not be found.  However,
 *         this code is used for other problems, such as the file was not a
 *         valid bitmap file.
 *
 *         If the dialog already had a custom background set, it is changed to
 *         the bitmap.  However, if an error ocurrs, then the existing custom
 *         background is not changed.
 *
 *  @remarks  In 4.0.1 and prior, this method was a DialogExtension method and
 *            it did not return a value.  When it was moved to PlainBaseDialog
 *            after 4.0.1, the return was added.
 */
RexxMethod3(RexxObjectPtr, pbdlg_backgroundBitmap, CSTRING, bitmapFileName, OPTIONAL_CSTRING, opts, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    uint32_t errCode = 0;
    HBITMAP hBitmap = (HBITMAP)loadDIB(bitmapFileName, &errCode);
    if ( hBitmap == NULL )
    {
        oodSetSysErrCode(context->threadContext, errCode);
        return TheFalseObj;
    }
    maybeSetColorPalette(context, hBitmap, opts, pcpbd);

    if ( pcpbd->bkgBitmap != NULL )
    {
        DeleteObject(pcpbd->bkgBitmap);
    }
    pcpbd->bkgBitmap = hBitmap;
    return TheTrueObj;
}

/** PlainBaseDialog::tiledBackgroundBitmap()
 *
 *  Sets a bitmap to be used as a custom dialog background.
 *
 *  An operating system 'brush' is created from the bitmap and used to paint the
 *  dialog background.  If the bitmap size is less than the size of the dialog,
 *  this results in the background having a 'tiled' effect.  The painting is
 *  done entirely by the operating system.  Contrast this to the
 *  backgroundBitmap() method.
 *
 *  @param  bitmapFileName  The name of a bitmap file to use as the dilaog
 *                          background.
 *
 *  @return  True on success, false for some error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *         The .SystemErrorCode is set to: 2 ERROR_FILE_NOT_FOUND The system
 *         cannot find the file specified, if there is a problem with the bitmap
 *         file.  This might be because the file could not be found.  However,
 *         this code is used for other problems, such as the file was not a
 *         valid bitmap file.
 *
 *         If the dialog already had a custom background set, it is changed to
 *         the bitmap.  However, if an error ocurrs, then the existing custom
 *         background is not changed.
 *
 *  @remarks  In 4.0.1 and prior, this method was a DialogExtension method and
 *            it did not return a value.  When it was moved to PlainBaseDialog
 *            after 4.0.1, the return was added.
 */
RexxMethod2(RexxObjectPtr, pbdlg_tiledBackgroundBitmap, CSTRING, bitmapFileName, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    uint32_t errCode = 0;
    HBITMAP hBitmap = (HBITMAP)loadDIB(bitmapFileName, &errCode);
    if ( hBitmap == NULL )
    {
        oodSetSysErrCode(context->threadContext, errCode);
        return TheFalseObj;
    }

    LOGBRUSH logicalBrush;
    logicalBrush.lbStyle = BS_DIBPATTERNPT;
    logicalBrush.lbColor = DIB_RGB_COLORS;
    logicalBrush.lbHatch = (ULONG_PTR)hBitmap;

    HBRUSH hBrush = CreateBrushIndirect(&logicalBrush);
    errCode = GetLastError();
    LocalFree((void *)hBitmap);

    if ( hBrush == NULL )
    {
        oodSetSysErrCode(context->threadContext, errCode);
        return TheFalseObj;
    }

    if ( pcpbd->bkgBrush != NULL )
    {
        DeleteObject(pcpbd->bkgBrush);
    }
    pcpbd->bkgBrush = hBrush;
    return TheTrueObj;
}

/** PlainBaseDialog::backgroundColor()
 *
 *  Sets a custom background color for the dialog.
 *
 *  In Windows, background colors are painted using a "brush."  This method
 *  creates a new brush for the specified color and this brush is then used
 *  whenever the background of the dialog needs to be repainted.
 *
 *  @param  color  The color index.
 *
 *  @return  True on success, false for some error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *         If the dialog already had a custom background color set, it is
 *         changed to the new color.  However, if an error ocurrs creating the
 *         new color brush then the custom color is not changed.
 *
 *  @remarks  In 4.0.1 and prior, this method was a DialogExtension method and
 *            it did not return a value.  When it was moved to PlainBaseDialog
 *            after 4.0.1, the return was added.
 */
RexxMethod2(RexxObjectPtr, pbdlg_backgroundColor, uint32_t, colorIndex, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    HBRUSH hBrush = CreateSolidBrush(PALETTEINDEX(colorIndex));
    if ( hBrush == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return TheFalseObj;
    }

    if ( pcpbd->bkgBrush != NULL )
    {
        DeleteObject(pcpbd->bkgBrush);
    }
    pcpbd->bkgBrush = hBrush;
    return TheTrueObj;
}

/** PlainBaseDialog::pixel2dlgUnit()
 *
 *  Takes a dimension expressed in pixels and tranforms it to a dimension
 *  expressed in dialog units of this dialog.
 *
 *  @param  pixels  [IN / OUT ]  The object to transform, can be either a
 *                  .Point, .Size, or .Rect.  On input, the unit of measurement
 *                  is assumed to be pixels and on return the pixels will have
 *                  been converted to dialog units.
 *
 *  @return  True on success, false on error.
 *
 *  @remarks  A point and a size are binary compatible, so they can be treated
 *            the same by using a cast for a size.
 */
RexxMethod2(logical_t, pbdlg_pixel2dlgUnit, RexxObjectPtr, du, OSELF, self)
{
    if ( context->IsOfType(du, "POINT") || context->IsOfType(du, "SIZE") )
    {
        POINT *p = (PPOINT)context->ObjectToCSelf(du);
        return mapPixelToDu(context, self, p, 1) ? TRUE : FALSE;
    }
    else if ( context->IsOfType(du, "RECT") )
    {
        RECT *r = (PRECT)context->ObjectToCSelf(du);
        return mapPixelToDu(context, self, (PPOINT)r, 2) ? TRUE : FALSE;
    }

    wrongArgValueException(context->threadContext, 1, "Point, Size, or Rect", du);
    return FALSE;
}

/** PlainBaseDialog::dlgUnit2pixel()
 *
 *  Takes a dimension expressed in dialog units of this dialog and tranforms it
 *  to a dimension expressed in pixels.
 *
 *  @param  du  [IN / OUT ]  The object to transform, can be either a .Rect,
 *              .Point, or .Size.  On input, the unit of measurement is assumed
 *              to be dialog units of this dialog and on return the dialog units
 *              will have been converted to pixels.
 *
 *  @return  True on success, false on error.
 *
 *  @remarks  A point and a size are binary compatible, so they can be treated
 *            the same by using a cast for a size.
 */
RexxMethod2(logical_t, pbdlg_dlgUnit2pixel, RexxObjectPtr, pixels, OSELF, self)
{
    RECT *r = {0};

    if ( context->IsOfType(pixels, "RECT") )
    {
        RECT *r = (PRECT)context->ObjectToCSelf(pixels);
        return mapDuToPixel(context, self, r) ? TRUE : FALSE;
    }
    else if ( context->IsOfType(pixels, "POINT") || context->IsOfType(pixels, "SIZE") )
    {
        POINT *p = (PPOINT)context->ObjectToCSelf(pixels);
        r->right  = p->x;
        r->bottom = p->y;

        if ( mapDuToPixel(context, self, r) )
        {
            p->x = r->right;
            p->y = r->bottom;
            return TRUE;
        }
        return FALSE;
    }

    wrongArgValueException(context->threadContext, 1, "Point, Size, or Rect", pixels);
    return FALSE;
}

/** PlainBaseDialog::getControlText()
 *
 *  Gets the text of the specified control.
 *
 *  @param  rxID  The resource ID of the control, may be numeric or symbolic.
 *
 *  @return  On success, the window text, which could be the empty string.  On
 *           failure, the empty string.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxStringObject, pbdlg_getControlText, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    RexxStringObject result = context->NullString();

    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl != NULL )
    {
        rxGetWindowText(context, hCtrl, &result);
    }
    else
    {
        result = context->String("-1");
    }
    return result;
}

/** PlainBaseDialog::setControlText()
 *
 *  Sets the text for the specified control.
 *
 *  @param  rxID  The resource ID of the control, may be numeric or symbolic.
 *  @param  text  The text to be set.
 *
 *  @return  0 for success, -1 for an incorrect resource ID, 1 for other errors.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, pbdlg_setControlText, RexxObjectPtr, rxID, CSTRING, text, CSELF, pCSelf)
{
    RexxObjectPtr result = TheOneObj;

    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl != NULL )
    {
        if ( SetWindowText(hCtrl, text) == 0 )
        {
            oodSetSysErrCode(context->threadContext);
        }
        else
        {
            result = TheZeroObj;
        }
    }
    else
    {
        result = TheNegativeOneObj;
    }
    return result;
}


/** PlainBaseDialog::focusControl
 *
 *  Sets the focus to the specified control in this dialog.
 *
 *  @param  rxID  The resource ID of the control, may be numeric or symbolic.
 *
 *  @return  0 on success, -1 if there is a problem identifying the dialog
 *           control.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, pbdlg_focusControl, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl != NULL )
    {
        SendMessage(((pCPlainBaseDialog)pCSelf)->hDlg, WM_NEXTDLGCTL, (WPARAM)hCtrl, TRUE);
        return TheZeroObj;
    }
    else
    {
        return TheNegativeOneObj;
    }
}

/** PlainBaseDialog::enableControl()
 *  PlainBaseDialog::disableControl()
 *
 *  Disables or enables the specified control
 *
 *  @param  rxID  The resource ID of the control, may be numeric or symbolic.
 *
 *  @return  1 (true) if the window was previously disabled, returns 0 (false)
 *           if the window was not previously disabled.  Note that this is not
 *           succes or failure.  -1 on error
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  The return was not previously documented.
 */
RexxMethod2(RexxObjectPtr, pbdlg_enableDisableControl, RexxObjectPtr, rxID, CSELF, pCSelf)
{
    RexxObjectPtr result = TheNegativeOneObj;

    HWND hCtrl = getPBDControlWindow(context, (pCPlainBaseDialog)pCSelf, rxID);
    if ( hCtrl != NULL )
    {
        BOOL enable = TRUE;
        if ( msgAbbrev(context) == 'D' )
        {
            enable = FALSE;
        }
        result = EnableWindow(hCtrl, enable) == 0 ? TheZeroObj : TheOneObj;
    }
    return result;
}

/** PlainBaseDialog::getControlHandle()
 *
 *  Gets the window handle of a dialog control.
 *
 *  @param  rxID  The resource ID of the control, which may be numeric or
 *                symbolic.
 *  @param  _hDlg [optional]  The window handle of the dialog the control
 *                belongs to.  If omitted, the handle of this dialog is used.
 *
 *  @return The window handle of the specified dialog control on success,
 *          otherwise 0.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  For pre 4.0.1 compatibility, this method needs to return 0 if the
 *            symbolic ID does not resolve, rather than the normal -1.
 */
RexxMethod3(RexxObjectPtr, pbdlg_getControlHandle, RexxObjectPtr, rxID, OPTIONAL_RexxStringObject, _hDlg, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_WINDOW_HANDLE);
        return TheZeroObj;
    }

    HWND hDlg;
    if ( argumentOmitted(2) )
    {
        hDlg = pcpbd->hDlg;
    }
    else
    {
        hDlg = (HWND)string2pointer(context, _hDlg);
    }

    HWND hCtrl = GetDlgItem(hDlg, id);
    if ( hCtrl == NULL )
    {
        oodSetSysErrCode(context->threadContext);
        return TheZeroObj;
    }
    return pointer2string(context, hCtrl);
}

RexxMethod1(int32_t, pbdlg_getControlID, CSTRING, hwnd)
{
    HWND hCtrl = (HWND)string2pointer(hwnd);
    int32_t id = -1;

    if ( IsWindow(hCtrl) )
    {
        SetLastError(0);
        id = GetDlgCtrlID(hCtrl);
        if ( id == 0 )
        {
            id = -(int32_t)GetLastError();
        }
    }
    return id == 0 ? -1 : id;
}

/** PlainBaseDialog::maximize()
 *  PlainBaseDialog::minimize()
 *  PlainBaseDialog::isMaximized()
 *  PlainBaseDialog::isMinimized()
 *
 */
RexxMethod2(RexxObjectPtr, pbdlg_doMinMax, NAME, method, CSELF, pCSelf)
{
    HWND hDlg = getPBDWindow(context, pCSelf);
    if ( hDlg == NULL )
    {
        return TheFalseObj;
    }

    if ( *method == 'M' )
    {
        uint32_t flag = (method[1] == 'I' ? SW_SHOWMINIMIZED : SW_SHOWMAXIMIZED);
        return (ShowWindow(hDlg, flag) ? TheZeroObj : TheOneObj);
    }

    if ( method[3] == 'A' )
    {
        // Zoomed is Maximized.
        return (IsZoomed(hDlg) ? TheTrueObj : TheFalseObj);
    }

    // Iconic is Minimized.
    return (IsIconic(hDlg) ? TheTrueObj : TheFalseObj);
}


/** PlainBaseDialog::setTabstop()
 *  PlainBaseDialog::setGroup()
 */
RexxMethod4(RexxObjectPtr, pbdlg_setTabGroup, RexxObjectPtr, rxID, OPTIONAL_logical_t, addStyle, NAME, method, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(context, pcpbd->rexxSelf);
        return TheFalseObj;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return TheNegativeOneObj;
    }

    if ( argumentOmitted(2) )
    {
        addStyle = TRUE;
    }

    HWND hCtrl = GetDlgItem(pcpbd->hDlg, id);
    uint32_t style = GetWindowLong(hCtrl, GWL_STYLE);

    if ( method[3] == 'T' )
    {
        style = (addStyle ? (style | WS_TABSTOP) : (style & ~WS_TABSTOP));
    }
    else
    {
        style = (addStyle ? (style | WS_GROUP) : (style & ~WS_GROUP));
    }
    return setWindowStyle(context, hCtrl, style);
}


/** PlainBaseDialog::isDialogActive()
 *
 *  Tests if the Windows dialog is still active.
 *
 *  @return  True if the underlying Windows dialog is active, otherwise false.
 */
RexxMethod1(RexxObjectPtr, pbdlg_isDialogActive, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = getPBDCSelf(context, pCSelf);
    if ( pcpbd != NULL )
    {
        if ( pcpbd->hDlg != NULL && pcpbd->isActive && IsWindow(pcpbd->hDlg) )
        {
            return TheTrueObj;
        }
    }
    return TheFalseObj;
}


/** PlainBaseDialog::connect<ControlName>()
 *
 *  Connects a windows dialog control with a 'data' attribute of the Rexx dialog
 *  object.  The attribute is added to the Rexx object and an entry is made in
 *  the data table using the control's resource ID.
 *
 *  @param  rxID           The resource ID of the control, can be numeric or
 *                         symbolic.
 *  @param  attributeName  [optional]  The name of the attribute to be added.
 *                         If omitted, the addAttribute() method will design a
 *                         name from the information available.
 *  @param  opts           [optional]  Not used, but must be present for
 *                         backwards compatibility.  Was only used in
 *                         connectComboBox() to distinguish between types of
 *                         combo boxes.  That functionality has been moved to
 *                         the native code.
 *
 *  @remarks  The control type is determined by the invoking Rexx method name.
 */
RexxMethod6(RexxObjectPtr, pbdlg_connect_ControName, RexxObjectPtr, rxID, OPTIONAL_RexxObjectPtr, attributeName,
            OPTIONAL_CSTRING, opts, NAME, msgName, OSELF, self, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = getPBDCSelf(context, pCSelf);
    if ( pcpbd == NULL )
    {
        return NULLOBJECT;
    }

    oodControl_t type = oodName2controlType(msgName + 7);
    if ( type == winNotAControl )
    {
        return TheNegativeOneObj;
    }

    // result will be the resolved resource ID, which may be -1 on error.
    RexxObjectPtr result = context->ForwardMessage(NULLOBJECT, "ADDATTRIBUTE", NULLOBJECT, NULLOBJECT);

    int id;
    if ( ! context->Int32(result, &id) || id == -1 )
    {
        return TheNegativeOneObj;
    }

    uint32_t category = getCategoryNumber(context, self);
    return ( addToDataTable(context, pcpbd, id, type, category) == OOD_NO_ERROR ? TheZeroObj : TheOneObj );
}


RexxMethod2(uint32_t, pbdlg_setDlgDataFromStem_pvt, RexxStemObject, internDlgData, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = getPBDCSelf(context, pCSelf);
    if ( pcpbd == NULL )
    {
        return 0;
    }
    return setDlgDataFromStem(context, pcpbd, internDlgData);
}


RexxMethod2(uint32_t, pbdlg_putDlgDataInStem_pvt, RexxStemObject, internDlgData, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = getPBDCSelf(context, pCSelf);
    if ( pcpbd == NULL )
    {
        return 0;
    }
    return putDlgDataInStem(context, pcpbd, internDlgData);
}


/** PlainBaseDialog::getControlData()
 *
 *  Gets the 'data' from a single dialog control.
 *
 *  The original ooDialog implementation seemed to use the abstraction that the
 *  state of a dialog control was its 'data' and this abstraction influenced the
 *  naming of many of the instance methods.  I.e., getData() setData().
 *
 *  The method getControlData() is, in the Rexx code, a general purpose method
 *  that replaces getValue() after 4.0.0.  getValue() forwards to
 *  getControlData().  The old doc:
 *
 *  "The getValue method gets the value of a dialog item, regardless of its
 *  type. The item must have been connected before."
 *
 *  @param  rxID  The resource ID of control.
 *
 *  @return  The 'data' value of the dialog control.  This of course varies
 *           depending on the type of the dialog control.
 *
 *  @remarks  The control type is determined by the invoking method name.  When
 *            the general purpose GETCONTROLDATA + 3 name is passed to
 *            oodName2controlType() it won't resolve and winUnknown will be
 *            returned.  This is the value that signals getControlData() to do a
 *            data table look up by resource ID.
 */
RexxMethod3(RexxObjectPtr, pbdlg_getControlData, RexxObjectPtr, rxID, NAME, msgName, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(context, pcpbd->rexxSelf);
        return TheNegativeOneObj;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return TheNegativeOneObj;
    }

    oodControl_t ctrlType = oodName2controlType(msgName + 3);

    return getControlData(context, pcpbd, id, pcpbd->hDlg, ctrlType);
}


/** PlainBaseDialog::setControlData()
 *
 *  Sets the 'data' for a single dialog control.
 *
 *  The original ooDialog implementation seemed to use the abstraction that the
 *  state of a dialog control was its 'data' and this abstraction influenced the
 *  naming of many of the instance methods.  I.e., getData() setData().
 *
 *  The method setControlData() is, in the Rexx code, a general purpose method
 *  that replaces setValue() after 4.0.0.  setValue() forwards to
 *  setControlData().  The old doc:
 *
 *  "The setValue() method sets the value of a dialog item. You do not have to
 *  know what kind of item it is. The dialog item must have been connected
 *  before."
 *
 *  @param  rxID  The resource ID of control.
 *  @param  data  The 'data' to set the control with.
 *
 *  @return  0 for success, 1 for error, -1 for bad resource ID.
 *
 *  @remarks  The control type is determined by the invoking method name.  When
 *            the general purpose SETCONTROLDATA + 3 name is passed to
 *            oodName2controlType() it won't resolve and winUnknown will be
 *            returned.  This is the value that signals setControlData() to do a
 *            data table look up by resource ID.
 */
RexxMethod4(int32_t, pbdlg_setControlData, RexxObjectPtr, rxID, CSTRING, data, NAME, msgName, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(context, pcpbd->rexxSelf);
        return -1;
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        return -1;
    }

    oodControl_t ctrlType = oodName2controlType(msgName + 3);

    return setControlData(context, pcpbd, id, data, pcpbd->hDlg, ctrlType);
}


/** PlainBaseDialog::stopIt()
 *
 *
 *  @remarks  Normally, this method is inovked to stop a running dialog, and
 *            internally clean up the CSelf struct.  However, it is sometimes
 *            invoked when a dialog fails to be created.  The old ooDialog code
 *            checked if hDlg was null and if so didn't call stopDialog().  That
 *            was okay, sort of, the dialog admin block got cleaned up in the
 *            uninit() method.  The check for hDlg == null is now skipped so
 *            that stopDialog is called to clean up the CSelf struct here
 *            instead of in uninit().
 *
 *  @remarks  PlainBaseDialog::leaving() does nothing.  It is intended to be
 *            over-ridden by the Rexx programer to do whatever she would want.
 *            It is invoked here, right before stopDialog().  However, it is
 *            only invoked if the underlying dialog was created.
 *
 *
 *
 *
 */
RexxMethod2(int32_t, pbdlg_stopIt, OPTIONAL_RexxObjectPtr, caller, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    RexxObjectPtr finished = context->GetObjectVariable("FINISHED");
    pcpbd->abnormalHalt = (finished == TheFalseObj ? true : false);

    if ( pcpbd->hDlg != NULL && ! pcpbd->abnormalHalt && ! pcpbd->isPageDlg )
    {
        context->SendMessage0(pcpbd->rexxSelf, "LEAVING");
    }

    int32_t result = stopDialog(pcpbd, context->threadContext);

    if ( argumentOmitted(1) )
    {
        context->SendMessage0(pcpbd->rexxSelf, "FINALSTOPIT");
    }
    else
    {
        context->SendMessage1(pcpbd->rexxSelf, "FINALSTOPIT", caller);
    }
    return result;
}

/** PlainBaseDialog::new<DialogControl>()
 *
 *  Instantiates a dialog control object for the specified Windows control.  All
 *  dialog control objects are instantiated through one of the PlainBaseDialog
 *  new<DialogControl>() methods. In turn each of those methods filter through
 *  this function. newEdit(), newPushButton(), newListView(), etc..
 *
 * @param  rxID  The resource ID of the control.
 *
 * @param  categoryPageID  [optional] If the dialog is a category dialog, this
 *                         indicates which page of the dialog the control is on.
 *
 * @returns  The properly instantiated dialog control object on success, or the
 *           nil object on failure.
 *
 * @remarks Either returns the control object asked for, or .nil.
 *
 *          The first time a Rexx object is instantiated for a specific Windows
 *          control, the Rexx object is stored in the window words of the
 *          control.  Before a Rexx object is instantiated, the window words are
 *          checked to see if there is already an instantiated object. If so,
 *          that object is returned rather than instantiating a new object.
 *
 *          It goes without saying that the intent here is to exactly preserve
 *          pre 4.0.0 behavior.  With category dialogs, it is somewhat hard to
 *          be sure the behavior is preserved. Prior to 4.0.0, getControl(id,
 *          catPage) would assign the parent dialog as the oDlg (owner dialog)
 *          of dialog controls on any of the category pages.  This seems
 *          technically wrong, but the same thing is done here. Need to
 *          investigate whether this ever makes a difference?
 */
RexxMethod5(RexxObjectPtr, pbdlg_newControl, RexxObjectPtr, rxID, OPTIONAL_uint32_t, categoryPageID,
            NAME, msgName, OSELF, self, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    RexxObjectPtr result = TheNilObj;

    bool isCategoryDlg = false;
    HWND hDlg = ((pCPlainBaseDialog)pCSelf)->hDlg;

    if ( c->IsOfType(self, "CATEGORYDIALOG") )
    {
        isCategoryDlg = true;

        // If the category page is not omitted and equal to 0, then we already
        // have the dialog handle.  It is the handle of the parent Rexx dialog.
        // Otherwise, we need to resolve the handle, which is the handle of one
        // of the child dialogs used for the pages of the parent dialog.
        //
        // In addition, the original Rexx code would check if the category page
        // ID was greater than 9000 and if so treat things as though the control
        // were part of the parent dialog, not one of the child dialog's
        // controls.  That is done here.  TODO - need to test that this part is
        // working correctly.

        if ( ! (argumentExists(2) && (categoryPageID == 0 || categoryPageID > 9000)) )
        {
            if ( ! getCategoryHDlg(context, self, &categoryPageID, &hDlg, 2) )
            {
                context->ClearCondition();
                goto out;
            }
        }
    }

    uint32_t id;
    if ( ! oodSafeResolveID(&id, c, self, rxID, -1, 1) || (int)id < 0 )
    {
        goto out;
    }

    HWND hControl = GetDlgItem(hDlg, (int)id);
    if ( isCategoryDlg && hControl == NULL )
    {
        // It could be that this is a control in the parent dialog of the
        // category dialog.  So, try once more.  If this still fails, then we
        // give up.
        hDlg = ((pCPlainBaseDialog)pCSelf)->hDlg;
        hControl = GetDlgItem(((pCPlainBaseDialog)pCSelf)->hDlg, (int)id);
    }

    if ( hControl == NULL )
    {
        goto out;
    }

    // Check that the underlying Windows control is the control type requested
    // by the programmer.  Return .nil if this is not true.
    oodControl_t controlType = oodName2controlType(msgName + 3);
    if ( ! isControlMatch(hControl, controlType) )
    {
        goto out;
    }

    result = createRexxControl(context, hControl, hDlg, id, controlType, self, isCategoryDlg, true);

out:
    return result;
}

RexxMethod2(RexxObjectPtr, pbdlg_putControl_pvt, RexxObjectPtr, control, OSELF, self)
{
    RexxObjectPtr bag = context->GetObjectVariable(CONTROLBAG_ATTRIBUTE);
    if ( bag == NULLOBJECT )
    {
        bag = rxNewBag(context);
        context->SetObjectVariable(CONTROLBAG_ATTRIBUTE, bag);
    }
    if ( bag != NULLOBJECT )
    {
        context->SendMessage2(bag, "PUT", control, control);
    }

    return TheNilObj;
}

