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
#include "oodText.hpp"
#include "oodData.hpp"
#include "oodDeviceGraphics.hpp"
#include "oodResourceIDs.hpp"

extern MsgReplyType SearchMessageTable(ULONG message, WPARAM param, LPARAM lparam, DIALOGADMIN * addressedTo);
extern BOOL DrawBitmapButton(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam, BOOL MsgEnabled);
extern BOOL DrawBackgroundBmp(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam);
extern LRESULT PaletteMessage(DIALOGADMIN * addr, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LONG SetRexxStem(const char * name, INT id, const char * secname, const char * data);

static HICON GetIconForID(DIALOGADMIN *, UINT, UINT, int, int);


class LoopThreadArgs
{
public:
    RexxMethodContext *context;  // Used for data autodetection only.
    DIALOGADMIN *dlgAdmin;
    uint32_t resourceId;
    bool autoDetect;
    bool *release;               // Used for a return value
};


/* dialog procedure
   handles the search for user defined messages and bitmap buttons
   handles messages necessary for 3d controls
   seeks for the addressed dialog to handle the messages */


LRESULT CALLBACK RexxDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   HBRUSH hbrush = NULL;
   HWND hW;
   DIALOGADMIN * addressedTo = NULL;
   BOOL MsgEnabled = FALSE;
   register INT i=0;

   if (uMsg != WM_INITDIALOG)
   {
       SEEK_DLGADM_TABLE(hDlg, addressedTo);
       if (!addressedTo && topDlg && dialogInAdminTable(topDlg) && topDlg->TheDlg) addressedTo = topDlg;

       if (addressedTo)
       {
          MsgEnabled = IsWindowEnabled(hDlg) && dialogInAdminTable(addressedTo);

          // Do not search message table for WM_PAINT to improve redraw.
          if ( MsgEnabled && (uMsg != WM_PAINT) && (uMsg != WM_NCPAINT) )
          {
              MsgReplyType searchReply;
              if ( (searchReply = SearchMessageTable(uMsg, wParam, lParam, addressedTo)) != NotMatched )
              {
                  // Note pre 4.0.1, reply was always FALSE, pass on to the system to process.
                  return (searchReply == ReplyTrue ? TRUE : FALSE);
              }
          }

          switch (uMsg) {
             case WM_PAINT:
                if (addressedTo->BkgBitmap) DrawBackgroundBmp(addressedTo, hDlg, wParam, lParam);
                break;

             case WM_DRAWITEM:
                if ((lParam != 0) && (addressedTo)) return DrawBitmapButton(addressedTo, hDlg, wParam, lParam, MsgEnabled);
                break;

             case WM_CTLCOLORDLG:
                if (addressedTo->BkgBrush)
                {
                    return (LRESULT) addressedTo->BkgBrush;
                }

             case WM_CTLCOLORSTATIC:
             case WM_CTLCOLORBTN:
             case WM_CTLCOLOREDIT:
             case WM_CTLCOLORLISTBOX:
             case WM_CTLCOLORMSGBOX:
             case WM_CTLCOLORSCROLLBAR:
                if (addressedTo->CT_size)
                {
                    // See of the user has set the dialog item with a different
                    // color.
                    LONG id = GetWindowLong((HWND)lParam, GWL_ID);
                    SEARCHBRUSH(addressedTo, i, id, hbrush);
                    if (hbrush)
                    {
                        if ( addressedTo->ColorTab[i].isSysBrush )
                        {
                            SetBkColor((HDC)wParam, GetSysColor(addressedTo->ColorTab[i].ColorBk));
                            if ( addressedTo->ColorTab[i].ColorFG != -1 )
                            {
                                SetTextColor((HDC)wParam, GetSysColor(addressedTo->ColorTab[i].ColorFG));
                            }
                        }
                        else
                        {
                            SetBkColor((HDC)wParam, PALETTEINDEX(addressedTo->ColorTab[i].ColorBk));
                            if ( addressedTo->ColorTab[i].ColorFG != -1 )
                            {
                                SetTextColor((HDC)wParam, PALETTEINDEX(addressedTo->ColorTab[i].ColorFG));
                            }
                        }
                    }
                }
                if (hbrush)
                   return (LRESULT)hbrush;
                else
                   return DefWindowProc(hDlg, uMsg, wParam, lParam);


             case WM_COMMAND:
             switch( LOWORD(wParam) ) {
                   case IDOK:
                      if (!HIWORD(wParam)) addressedTo->LeaveDialog = 1; /* Notify code must be 0 */
                      return TRUE;
                      break;
                   case IDCANCEL:
                      if (!HIWORD(wParam)) addressedTo->LeaveDialog = 2; /* Notify code must be 0 */
                      return TRUE;
                      break;
             }
             break;

#ifdef __CTL3D
             case WM_SYSCOLORCHANGE:
                if (addressedTo->Use3DControls)
                    Ctl3dColorChange();
             break;
#endif

             case WM_QUERYNEWPALETTE:
             case WM_PALETTECHANGED:
                if (addressedTo) return PaletteMessage(addressedTo, hDlg, uMsg, wParam, lParam);
                break;

             case WM_SETTEXT:
             case WM_NCPAINT:
             case WM_NCACTIVATE:
#ifdef __CTL3D
                if (addressedTo->Use3DControls)
                {
                    SetWindowLong(hDlg, DWL_MSGRESULT,Ctl3dDlgFramePaint(hDlg, uMsg, wParam, lParam));
                    return TRUE;
                }
#endif
                return FALSE;

              case WM_USER_CREATECHILD:
                /* Create a child dialog of this dialog and return its window
                 * handle. The dialog template pointer is passed here as the
                 * LPARAM arg from DynamicDialog::startChildDialog().
                 */
                hW = CreateDialogIndirectParam(MyInstance, (DLGTEMPLATE *)lParam, hDlg, (DLGPROC)RexxDlgProc, addressedTo->Use3DControls);
                ReplyMessage((LRESULT)hW);
                return (LRESULT)hW;
             case WM_USER_INTERRUPTSCROLL:
                addressedTo->StopScroll = wParam;
                return (TRUE);
             case WM_USER_GETFOCUS:
                ReplyMessage((LRESULT)GetFocus());
                return (TRUE);
             case WM_USER_GETSETCAPTURE:
                if (!wParam) ReplyMessage((LRESULT)GetCapture());
                else if (wParam == 2) ReplyMessage((LRESULT)ReleaseCapture());
                else ReplyMessage((LRESULT)SetCapture((HWND)lParam));
                return (TRUE);
             case WM_USER_GETKEYSTATE:
                ReplyMessage((LRESULT)GetAsyncKeyState((int)wParam));
                return (TRUE);

             case WM_USER_SUBCLASS:
             {
                 SUBCLASSDATA * pData = (SUBCLASSDATA *)lParam;
                 BOOL success = FALSE;

                 if ( pData )
                 {
                     success = SetWindowSubclass(pData->hCtrl, (SUBCLASSPROC)wParam, pData->uID, (DWORD_PTR)pData);
                 }
                 ReplyMessage((LRESULT)success);
             } return (TRUE);

             case WM_USER_SUBCLASS_REMOVE:
                 ReplyMessage((LRESULT)RemoveWindowSubclass(GetDlgItem(hDlg, (int)lParam), (SUBCLASSPROC)wParam, (int)lParam));
                 return (TRUE);

             case WM_USER_HOOK:
                 ReplyMessage((LRESULT)SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)wParam, NULL, GetCurrentThreadId()));
                 return (TRUE);

             case WM_USER_CONTEXT_MENU:
             {
                 PTRACKPOP ptp = (PTRACKPOP)wParam;
                 uint32_t cmd;

                 SetLastError(0);
                 cmd = (uint32_t)TrackPopupMenuEx(ptp->hMenu, ptp->flags, ptp->point.x, ptp->point.y,
                                                  ptp->hWnd, ptp->lptpm);

                 // If TPM_RETURNCMD is specified, the return is the menu item
                 // selected.  Otherwise, the return is 0 for failure and
                 // non-zero for success.
                 if ( ! (ptp->flags & TPM_RETURNCMD) )
                 {
                     cmd = (cmd == 0 ? FALSE : TRUE);
                     if ( cmd == FALSE )
                     {
                         ptp->dwErr = GetLastError();
                     }
                 }
                 ReplyMessage((LRESULT)cmd);
                 return (TRUE);
             }
          }
       }
   }
   else
   /* the WM_INITDIALOG message is sent by CreateDialog(Indirect)Param before TheDlg is set */
   {
#ifdef __CTL3D
         if (lParam)    /* Use3DControls is the lparam that is specified for the Create API */
             Ctl3dSubclassDlgEx(hDlg, CTL3D_ALL);
#endif
         return TRUE;
   }
   return FALSE;
}

static DIALOGADMIN * allocDlgAdmin(RexxMethodContext *c)
{
    DIALOGADMIN *adm = NULL;

    EnterCriticalSection(&crit_sec);

    if ( StoredDialogs < MAXDIALOGS )
    {
        adm = (DIALOGADMIN *)LocalAlloc(LPTR, sizeof(DIALOGADMIN));
        if ( adm == NULL )
        {
            goto err_out;
        }

        adm->pMessageQueue = (char *)LocalAlloc(LPTR, MAXLENQUEUE);
        if ( adm->pMessageQueue == NULL )
        {
            goto err_out;
        }

        adm->previous = topDlg;
        adm->TableEntry = StoredDialogs;
        StoredDialogs++;
        DialogTab[adm->TableEntry] = adm;
        goto done_out;
    }
    else
    {
        MessageBox(0, "Too many active Dialogs","Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
        goto done_out;
    }

err_out:
    safeLocalFree(adm);
    MessageBox(0, "Out of system resources", "Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);

done_out:
    LeaveCriticalSection(&crit_sec);
    return adm;
}

/**
 * Do some common set up when creating the underlying Windows dialog for any
 * ooDialog dialog.  This involves setting the 'topDlg' and the TheInstance
 * field of the DIALOGADMIN struct.
 *
 * If this is a ResDialog then the resource DLL is loaded, otherwise the
 * TheInstance field is the ooDialog.dll instance.
 *
 * @param dlgAdm    Pointer to the dialog administration block
 * @param library   The library to load the dialog from, if a ResDialog,
 *                  othewise null.
 *
 * @return True on success, false only if this is for a ResDialog and the
 *         loading of the resource DLL failed.
 */
bool InstallNecessaryStuff(DIALOGADMIN* dlgAdm, CSTRING library)
{
    if ( dlgAdm->previous )
    {
        ((DIALOGADMIN*)dlgAdm->previous)->OnTheTop = FALSE;
    }
    topDlg = dlgAdm;

    if ( library != NULL )
    {
        dlgAdm->TheInstance = LoadLibrary(library);
        if ( ! dlgAdm->TheInstance )
        {
            CHAR msg[256];
            sprintf(msg,
                    "Failed to load Dynamic Link Library (resource DLL.)\n"
                    "  File name:\t\t\t%s\n"
                    "  Windows System Error Code:\t%d\n", library, GetLastError());
            MessageBox(0, msg, "ooDialog DLL Load Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
            return false;
        }
    }
    else
    {
        dlgAdm->TheInstance = MyInstance;
    }

    dlgAdm->Use3DControls = FALSE;
    return true;
}



/* end a dialog and remove installed components */
int32_t DelDialog(DIALOGADMIN * aDlg)
{
    DIALOGADMIN * current;
    int32_t ret;
    INT i;
    BOOL wasFGW;
    HICON hIconBig = NULL;
    HICON hIconSmall = NULL;

    EnterCriticalSection(&crit_sec);
    wasFGW = (aDlg->TheDlg == GetForegroundWindow());

    ret = aDlg->LeaveDialog;

    // Add this message, so PlainBaseDialog::handleMessages() knows that
    // PlainBaseDialog::finished() should be set.
    AddDialogMessage(MSG_TERMINATE, aDlg->pMessageQueue);

    if ( aDlg->LeaveDialog == 0 )
    {
        // Signal to exit.
        aDlg->LeaveDialog = 3;
    }

    // The dialog adminstration block entry must be removed before the WM_QUIT
    // message is posted.
    if ( aDlg->TableEntry == StoredDialogs - 1 )
    {
        // The dialog being ended is the last entry in the table, just set it to
        // null.
        DialogTab[aDlg->TableEntry] = NULL;
    }
    else
    {
        // The dialog being ended is not the last entry.  Move the last entry to
        // the one being deleted and then delete the last entry.
        DialogTab[aDlg->TableEntry] = DialogTab[StoredDialogs-1];
        DialogTab[aDlg->TableEntry]->TableEntry = aDlg->TableEntry;
        DialogTab[StoredDialogs-1] = NULL;
    }
    StoredDialogs--;

    // Post the WM_QUIT message to exit the Windows message loop.
    PostMessage(aDlg->TheDlg, WM_QUIT, 0, 0);

    if ( aDlg->TheDlg )
    {
        // The Windows documentation states: "must not use EndDialog for
        // non-modal dialogs"
        DestroyWindow(aDlg->TheDlg);
    }

    /* Swap back the saved icons. If not shared, the icon was loaded from a file
     * and needs to be freed, otherwise the OS handles the free.  The title bar
     * icon is tricky.  At this point the dialog is still visible.  If the small
     * icon in the class is set to 0, the application will hang.  Same thing
     * happens if the icon is freed.  So, don't set a zero into the class bytes,
     * and, if the icon is to be freed, do so after leaving the critical
     * section.
     */
    if ( aDlg->DidChangeIcon )
    {
        hIconBig = (HICON)setClassPtr(aDlg->TheDlg, GCLP_HICON, (LONG_PTR)aDlg->SysMenuIcon);
        if ( aDlg->TitleBarIcon )
        {
            hIconSmall = (HICON)setClassPtr(aDlg->TheDlg, GCLP_HICONSM, (LONG_PTR)aDlg->TitleBarIcon);
        }

        if ( ! aDlg->SharedIcon )
        {
            DestroyIcon(hIconBig);
            if ( ! hIconSmall )
            {
                hIconSmall = (HICON)getClassPtr(aDlg->TheDlg, GCLP_HICONSM);
            }
        }
        else
        {
            hIconSmall = NULL;
        }
    }

    if ( aDlg->TheInstance != NULL && aDlg->TheInstance != MyInstance )
    {
        FreeLibrary(aDlg->TheInstance);
    }
    current = (DIALOGADMIN *)aDlg->previous;

    // Delete the data and message tables of the dialog.
    if ( aDlg->MsgTab )
    {
        for ( i = 0; i < aDlg->MT_size; i++ )
        {
            safeLocalFree(aDlg->MsgTab[i].rexxProgram);
        }
        LocalFree(aDlg->MsgTab);
        aDlg->MT_size = 0;
    }
    safeLocalFree(aDlg->DataTab);
    aDlg->DT_size = 0;

    // Delete the color brushes.
    if (aDlg->ColorTab)
    {
        for (i=0;i<aDlg->CT_size;i++)
        {
            safeDeleteObject(aDlg->ColorTab[i].ColorBrush);
        }
        LocalFree(aDlg->ColorTab);
        aDlg->CT_size = 0;
    }

    // Delete the bitmaps and bitmap table.
    if (aDlg->BmpTab)
    {
        for (i=0;i<aDlg->BT_size;i++)
        {
            if ( (aDlg->BmpTab[i].Loaded & 0x1011) == 1 )
            {
                /* otherwise stretched bitmap files are not freed */
                safeLocalFree((void *)aDlg->BmpTab[i].bitmapID);
                safeLocalFree((void *)aDlg->BmpTab[i].bmpFocusID);
                safeLocalFree((void *)aDlg->BmpTab[i].bmpSelectID);
                safeLocalFree((void *)aDlg->BmpTab[i].bmpDisableID);
            }
            else if ( aDlg->BmpTab[i].Loaded == 0 )
            {
                safeDeleteObject((HBITMAP)aDlg->BmpTab[i].bitmapID);
                safeDeleteObject((HBITMAP)aDlg->BmpTab[i].bmpFocusID);
                safeDeleteObject((HBITMAP)aDlg->BmpTab[i].bmpSelectID);
                safeDeleteObject((HBITMAP)aDlg->BmpTab[i].bmpDisableID);
            }
        }

        LocalFree(aDlg->BmpTab);
        safeDeleteObject(aDlg->ColorPalette);
        aDlg->BT_size = 0;
    }

    // Delete the icon resource table.
    if (aDlg->IconTab)
    {
        for ( i = 0; i < aDlg->IT_size; i++ )
        {
            safeLocalFree(aDlg->IconTab[i].fileName);
        }
        LocalFree(aDlg->IconTab);
        aDlg->IT_size = 0;
    }

    // Unhook a hook if it is installed.
    if ( aDlg->hHook )
    {
        removeKBHook(aDlg);
    }

    // The message queue and the dialog administration block itself are freed
    // from the PlainBaseDialog::deInstall() or PlainBaseDialog::unInit()

    if ( StoredDialogs == NULL )
    {
        topDlg = NULL;
    }
    else
    {
        topDlg = current;
    }

    if ( topDlg != NULL )
    {
        if ( dialogInAdminTable(topDlg) )
        {
            if (!IsWindowEnabled(topDlg->TheDlg))
            {
                EnableWindow(topDlg->TheDlg, TRUE);
            }
            if ( wasFGW )
            {
                SetForegroundWindow(topDlg->TheDlg);
                topDlg->OnTheTop = TRUE;
            }
        }
        else
        {
            topDlg = NULL;
        }
    }
    LeaveCriticalSection(&crit_sec);
    if ( hIconSmall )
    {
        DestroyIcon(hIconSmall);
    }
    return ret;
}


/* create an asynchronous dialog and run asynchronous message loop */
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
 * @param dlgAdm    Pointer to the dialog administration block.
 * @param id        Numerical resource ID.
 * @param iconSrc   Flag indicating whether the icon is located in a DLL or to
 *                  be loaded from a file.
 * @param phBig     In/Out Pointer to an icon handle.  If the function succeeds,
 *                  on return will contian the handle to a regular size icon.
 * @param phSmall   In/Out Pointer to an icon handle.  On success will contain
 *                  a handle to a small size icon.
 *
 * @return True if the icons were loaded and the returned handles are valid,
 *         otherwise false.
 */
BOOL GetDialogIcons(DIALOGADMIN *dlgAdm, INT id, UINT iconSrc, PHANDLE phBig, PHANDLE phSmall)
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

    *phBig = GetIconForID(dlgAdm, id, iconSrc, cx, cy);

    /* If that didn't get the big icon, try to get the default icon. */
    if ( ! *phBig && id != IDI_DLG_DEFAULT )
    {
        id = IDI_DLG_DEFAULT;
        iconSrc = ICON_OODIALOG;
        *phBig = GetIconForID(dlgAdm, id, iconSrc, cx, cy);
    }

    /* If still no big icon, don't bother trying for the small icon. */
    if ( *phBig )
    {
        cx = GetSystemMetrics(SM_CXSMICON);
        cy = GetSystemMetrics(SM_CYSMICON);
        *phSmall = GetIconForID(dlgAdm, id, iconSrc, cx, cy);

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

    dlgAdm->SharedIcon = iconSrc != ICON_FILE;
    return TRUE;
}


/**
 * Loads and returns the handle to an icon for the specified ID, of the
 * specified size.
 *
 * The icons can come from the user resource DLL, a user defined dialog, the
 * OODialog DLL, the System.  IDs for the icons bound to the OODialog.dll are
 * reserved.
 *
 * @param dlgAdm    Pointer to the dialog administration block.
 * @param id        Numerical resource ID.
 * @param iconSrc   Flag indicating the source of the icon.
 * @param cx        The desired width of the icon.
 * @param cy        The desired height of the icon.
 *
 * @return The handle to the loaded icon on success, or null on failure.
 */
static HICON GetIconForID(DIALOGADMIN *dlgAdm, UINT id, UINT iconSrc, int cx, int cy)
{
    HINSTANCE hInst = NULL;
    LPCTSTR   pName = NULL;
    UINT      loadFlags = 0;

    if ( iconSrc & ICON_FILE )
    {
        /* Load the icon from a file, file name should be in the icon table. */
        INT i;

        for ( i = 0; i < dlgAdm->IT_size; i++ )
        {
            if ( dlgAdm->IconTab[i].iconID == id )
            {
                pName = dlgAdm->IconTab[i].fileName;
                break;
            }
        }

        if ( ! pName )
            return NULL;

        loadFlags = LR_LOADFROMFILE;
    }
    else if ( iconSrc & ICON_OODIALOG )
    {
        /* Load the icon from the resources in oodialog.dll. */
        hInst = MyInstance;
        pName = MAKEINTRESOURCE(id);
        loadFlags = LR_SHARED;
    }
    else
    {
        /* Load the icon from the user's resource DLL. */
        hInst = dlgAdm->TheInstance;
        pName = MAKEINTRESOURCE(id);
        loadFlags = LR_SHARED;
    }

    return (HICON)LoadImage(hInst, pName, IMAGE_ICON, cx, cy, loadFlags);
}


/**
 *  Methods for the .WindowBase mixin class.
 */
#define WINDOWBASE_CLASS       "WindowBase"

static inline HWND getWBWindow(void *pCSelf)
{
    return ((pCWindowBase)pCSelf)->hwnd;
}

// TODO, may be okay to use static ?
uint32_t parseShowOptions(CSTRING options)
{
    uint32_t opts = SWP_NOZORDER;

    if ( options != NULL )
    {
       if ( StrStrI(options, "NOMOVE"    ) ) opts |= SWP_NOMOVE;
       if ( StrStrI(options, "NOSIZE"    ) ) opts |= SWP_NOSIZE;
       if ( StrStrI(options, "HIDEWINDOW") ) opts |= SWP_HIDEWINDOW;
       if ( StrStrI(options, "SHOWWINDOW") ) opts |= SWP_SHOWWINDOW;
       if ( StrStrI(options, "NOREDRAW"  ) ) opts |= SWP_NOREDRAW;
    }
    return opts;
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

    c->SendMessage1(self, "INIT_WINDOWBASE", obj);

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
RexxMethod1(logical_t, wb_init_windowBase, RexxObjectPtr, cSelf)
{
    RexxMethodContext *c = context;
    if ( ! context->IsBuffer(cSelf) )
    {
        context->SetObjectVariable("INITCODE", TheOneObj);
        wrongClassException(context->threadContext, 1, "Buffer");
        return FALSE;
    }

    context->SetObjectVariable("CSELF", cSelf);

    context->SetObjectVariable("INITCODE", TheZeroObj);

    return TRUE;
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

/** WindowBase::pixelX  [attribute]
 *
 *  Returns the width of the window in pixels.  This is a 'get' only attribute.
 */
RexxMethod1(uint32_t, wb_getPixelX, CSELF, pCSelf)
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
RexxMethod1(uint32_t, wb_getPixelY, CSELF, pCSelf)
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

/** WindowBase::size()
 *
 *  Returns the size of the window in pixels.
 *
 *  @return  A .Size object with the width and height of the window in pixels.
 *           If there is now underlying window yet, the width and height will be
 *           0.
 */
RexxMethod1(RexxObjectPtr, wb_size, CSELF, pCSelf)
{
    pCWindowBase pcs = (pCWindowBase)pCSelf;
    if ( pcs->hwnd == NULL )
    {
        return rxNewSize(context, 0, 0);
    }

    RECT r = {0};
    GetWindowRect(pcs->hwnd, &r);
    return rxNewSize(context, r.right - r.left, r.bottom - r.top);
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

/**
 * Common code to call ShowWindow() for a native API method.
 *
 * @param pCSelf   Pointer to a struct with the window handle.  Must be a
 *                 CWindowBase struct.
 * @param type     Single character indicating which SW_ flag to use.
 *
 * @return  True if the window was previously visible.  Return false if the
 *          window was previously hidden.
 */
logical_t showWindow(void *pCSelf, char type)
{
    int flag;
    switch ( type )
    {
        case 'D' :
        case 'N' :
        case 'S' :
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

        default :
            flag = SW_SHOW;
            break;

    }
    return ShowWindow(getWBWindow(pCSelf), flag);
}

uint32_t showFast(void *pCSelf, char type)
{
    HWND hwnd = getWBWindow(pCSelf);
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

/** WindowBase::show() / WindowBase::hide()
 *
 *  Hides or shows the window.  This function is mapped to both methods of
 *  WindowBase.  The return for these methods was not previously documented.
 *
 *  @return  True if the window was previously visible.  Return false if the
 *           window was previously hidden.
 */
RexxMethod1(logical_t, wb_show, CSELF, pCSelf)
{
    return showWindow(pCSelf, msgAbbrev(context));
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
    return showFast(pCSelf, msgAbbrev(context));
}

/** WindowBase::display()
 *
 *
 *  @return  0 for success, 1 for error.  An error is highly unlikely.
 */
RexxMethod2(uint32_t, wb_display, OPTIONAL_CSTRING, opts,  CSELF, pCSelf)
{
    char type;
    uint32_t ret = 0;
    bool doFast = false;

    if ( opts != NULL && StrStrI(opts, "FAST") != NULL )
    {
        doFast = true;
    }

    if ( opts == NULL ) {type = 'S';}
    else if ( StrStrI(opts, "NORMAL") != NULL ) {type = 'S';}
    else if ( StrStrI(opts, "DEFAULT") != NULL ) {type = 'S';}
    else if ( StrStrI(opts, "HIDE") != NULL ) {type = 'H';}
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
        userDefinedMsgException(context->threadContext, 1, "The keyword option string must contain one of NORMAL, DEFAULT, HIDE, INACTIVE");
        goto done_out;
    }

    if ( doFast )
    {
        ret = showFast(pCSelf, type);
    }
    else
    {
        showWindow(pCSelf, type);
    }

done_out:
    return ret;
}

/** WindowsBase::draw() / WindowsBase::redrawClient() / WindowsBase::update()
 *
 *  Causes the entire client area of the the window to be redrawn.
 *
 *  This method maps to the draw(), update() and the redrawClient() methods.
 *  The implementation preserves existing behavior prior to ooRexx 4.0.1.  That
 *  is: the draw() method takes no argument and always uses false for the erase
 *  background arg. The update() method takes no argument and always uses true
 *  for the erase background arg, The redrawClient() method takes an argument to
 *  set the erase background arg. The argument can be either .true / .false (1
 *  or 0), or yes / no, or ja / nein.
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

    if ( flag == 'R' )
    {
        doErase = false;
    }
    else if ( flag == 'U' )
    {
        doErase = true;
    }
    else
    {
        doErase = isYes(erase);
    }
    return redrawRect(context, hwnd, NULL, doErase);
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

/** WindowBase::windowRect()
 *
 *  Retrieves the dimensions of the bounding rectangle of this window.  The
 *  dimensions are in screen coordinates that are relative to the upper-left
 *  corner of the screen.
 *
 *  @return  The bounding rectangle of this window.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod1(RexxObjectPtr, wb_windowRect, CSELF, pCSelf)
{
    return oodGetWindowRect(context, getWBWindow(pCSelf));
}


/** WindowBase::clientRect()
 *
 *  Retrieves the coordinates of a window's client area.  The coordinates are in
 *  pixels.
 *
 *  The client coordinates specify the upper-left and lower-right corners of the
 *  client area. Because client coordinates are relative to the upper-left
 *  corner of a window's client area, the coordinates of the upper-left corner
 *  are (0,0).
 *
 *  @param  hwnd  [OPTIONAL]  By default, the coordinates are for this window.
 *                However, the optional hwnd argument can be used to specify
 *                getting the coordinates for some other window.
 *
 *  @return  The coordinates of the client area of the specified window as a
 *           .Rect object.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod2(RexxObjectPtr, wb_clientRect, OPTIONAL_POINTERSTRING, hwnd, CSELF, pCSelf)
{
    if ( argumentOmitted(1) )
    {
        hwnd = getWBWindow(pCSelf);
    }

    RECT r = {0};
    oodGetClientRect(context, (HWND)hwnd, &r);
    return rxNewRect(context, &r);
}

/** WindowBase::setWindowPos()
 *
 *  Changes the size, and position of a child, pop-up, or top-level window.
 *
 *  Provides an interface to the Windows API, SetWindowPos().  The inteface is
 *  incomplete, at this time, as it does not recognize all the allowable flags
 *  to SetWindowPos.  In particular, it always uses SWP_NOZORDER.
 *
 *  By specifying either NOSIZE or NOMOVE options the programmer can only move
 *  or only resize the window.
 *
 *  The arguments needed are as follows:
 *
 *  However, the position and size can be specified in a flexiable manner. There
 *  are 3 forms accepted.
 *
 *  Form One:
 *  @param  rect    The new size and position specified as a .Rect object.  Note
 *                  that this is a point / size rectangle, not a bounding
 *                  rectangle. rect.right must specify the "width" of the window
 *                  and rect.bottom must specify the height of the window. They
 *                  must *not* specify the bottom left corner of the window.
 *  @param  flags   [OPTIONAL] Keywords specifying the behavior of the method.
 *
 *  Form Two:
 *  @param  pos     The new upper left corner position for the window, specified
 *                  as a .Point object.
 *  @param  size    The new size for the window, specified as a .Size object.
 *  @param  flags   [OPTIONAL] Keywords specifying the behavior of the method.
 *
 *  Form Three:
 *  @param  x       x coordinate of upper left corner of window, in pixels.
 *  @param  y       y coordinate of upper left corner of window, in pixels.
 *  @param  width   Width of window in pixels.
 *  @param  height  Height of window in pixels.
 *  @param  flags   [OPTIONAL] Keywords specifying the behavior of the method.
 *
 *  @return  0 for success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 *
 *  @remarks  Microsoft says: If the SWP_SHOWWINDOW or SWP_HIDEWINDOW flag is
 *            set, the window cannot be moved or sized.  But, that does not
 *            appear to be true.
 */
RexxMethod2(RexxObjectPtr, wb_setWindowPos, ARGLIST, args, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    oodResetSysErrCode(context->threadContext);

    // A RECT is used to return the values, even though the semantics are not
    // quite correct.  (right will really be cx and bottom will be cy.)
    size_t countArgs;
    int    argsUsed;
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
            obj = c->ArrayAt(args, 2);
            options = c->ObjectToStringValue(obj);
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
            obj = c->ArrayAt(args, 3);
            options = c->ObjectToStringValue(obj);
        }
    }
    else
    {
        if ( countArgs == 5 )
        {
            obj = c->ArrayAt(args, 5);
            options = c->ObjectToStringValue(obj);
        }
    }

    uint32_t opts = parseShowOptions(options);
    if ( SetWindowPos(getWBWindow(pCSelf), NULL, rect.left, rect.top, rect.right, rect.bottom, opts) == 0 )
    {
        oodSetSysErrCode(context->threadContext);
        return TheOneObj;
    }
    return TheZeroObj;
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
 *  Two forms for the argument list.  For resizeTo(), either the new size, is
 *  specified as .Size object, or as individual integers.  For moveTo(), the new
 *  position is specified as either a .Point object or as individual integers.
 *
 *  Form One:
 *  @param  size    The new size is specified using a .Size object.
 *  @param  flags   [OPTIONAL] Keywords that control the behavior of the method.
 *
 *  @param  point   The new position is specified using a .Point object.
 *  @param  flags   [OPTIONAL] Keywords that control the behavior of the method.
 *
 *  Form Two:
 *  @param  cx      The new width of the window.
 *  @param  cy      The new height of the window.
 *  @param  flags   [OPTIONAL] Keywords that control the behavior of the method.
 *
 *  @param  x       The x coordinate of the upper left corner of the window.
 *  @param  y       The y coordinate of the upper left corner of the window.
 *  @param  flags   [OPTIONAL] Keywords that control the behavior of the method.
 *
 *  @return  0 for success, 1 on error.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, wb_resizeMove, ARGLIST, args, NAME, method, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    oodResetSysErrCode(context->threadContext);

    // POINT and SIZE structs are binary compatible.  A POINT is used to return
    // the values, even though the semantics are not quite correct for
    // resizeTo(). (x will really be cx and y will be cy.)
    size_t countArgs;
    int    argsUsed;
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
            obj = c->ArrayAt(args, 2);
            options = c->ObjectToStringValue(obj);
        }
    }
    else if ( argsUsed == 2 )
    {
        if ( countArgs == 3 )
        {
            // The object at index 3 has to exist, otherwise countArgs would
            // equal 2.
            obj = c->ArrayAt(args, 3);
            options = c->ObjectToStringValue(obj);
        }
    }

    uint32_t opts = parseShowOptions(options);
    RECT r = {0};

    if ( *method == 'R' )
    {
        opts |= SWP_NOMOVE;
        r.right = point.x;
        r.bottom = point.y;
    }
    else
    {
        opts |= SWP_NOSIZE;
        r.left = point.x;
        r.top = point.y;
    }

    if ( SetWindowPos(getWBWindow(pCSelf), NULL, r.left, r.top, r.right, r.bottom, opts) == 0 )
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
    oodResetSysErrCode(context->threadContext);

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


RexxMethod2(uint32_t, wb_getWindowLong_pvt, int32_t, flag, CSELF, pCSelf)
{
    return GetWindowLong(getWBWindow(pCSelf), flag);
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

/**
 *  Methods for the .PlainBaseDialog class.
 */
#define PLAINBASEDIALOG_CLASS  "PlainBaseDialog"

int32_t stopDialog(HWND hDlg)
{
    if ( hDlg != NULL )
    {
        DIALOGADMIN * dlgAdm = seekDlgAdm(hDlg);
        if ( dlgAdm != NULL)
        {
            return DelDialog(dlgAdm);
        }
        else
        {
            return -1;
        }
    }
    else if ( hDlg == NULL && topDlg != NULL )
    {
        // Remove the top most.  This scenario is invoked from the DynamicDialog
        // class when create() or load() fail.
        return DelDialog(topDlg);
    }
    return -1;
}

RexxObjectPtr setDlgHandle(RexxMethodContext *c, pCPlainBaseDialog pcpbd, HWND hDlg)
{
    pCWindowBase pcwb = pcpbd->wndBase;
    pcpbd->hDlg = hDlg;
    pcpbd->enCSelf->hDlg = hDlg;

    if ( pcpbd->hDlg != NULL )
    {
        pcwb->hwnd = pcpbd->hDlg;
        pcwb->rexxHwnd = c->RequestGlobalReference(pointer2string(c, hDlg));
    }
    else
    {
        pcwb->hwnd = NULL;
        if ( pcwb->rexxHwnd != NULLOBJECT && pcwb->rexxHwnd != TheZeroObj )
        {
            c->ReleaseGlobalReference(pcwb->rexxHwnd);
        }
        pcwb->rexxHwnd = TheZeroObj;
    }

    return NULLOBJECT;
}

bool initEventNotification(RexxMethodContext *c, DIALOGADMIN *dlgAdm, RexxObjectPtr self, pCEventNotification *ppCEN)
{
    RexxBufferObject obj = c->NewBuffer(sizeof(CEventNotification));
    if ( obj == NULLOBJECT )
    {
        return false;
    }

    pCEventNotification pcen = (pCEventNotification)c->BufferData(obj);
    pcen->dlgAdm = dlgAdm;
    pcen->hDlg = NULL;
    pcen->rexxSelf = self;

    c->SendMessage1(self, "INIT_EVENTNOTIFICATION", obj);
    *ppCEN = pcen;
    return true;
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
    pCPlainBaseDialogClass pcpbdc = getPBDClass_CSelf(context);

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
    pCPlainBaseDialogClass pcpbdc = getPBDClass_CSelf(context);
    return pcpbdc->fontName;
}
RexxMethod1(uint32_t, pbdlg_getFontSize_cls, CSELF, pCSelf)
{
    pCPlainBaseDialogClass pcpbdc = getPBDClass_CSelf(context);
    return pcpbdc->fontSize;
}

RexxMethod5(RexxObjectPtr, pbdlg_init, RexxObjectPtr, library, RexxObjectPtr, resource,
            OPTIONAL_RexxObjectPtr, dlgDataStem, OPTIONAL_RexxObjectPtr, hFile, OSELF, self)
{
    RexxMethodContext *c = context;

    RexxObjectPtr result = TheOneObj;  // This is an error return.

    context->SetObjectVariable("LIBRARY", library);
    context->SetObjectVariable("RESOURCE", resource);

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

    // Get a buffer for the PlainBaseDialog CSelf.
    RexxBufferObject cselfBuffer = context->NewBuffer(sizeof(CPlainBaseDialog));
    if ( cselfBuffer == NULLOBJECT )
    {
        goto err_out;
    }

    // Initialize the window base.
    pCWindowBase pWB;
    if ( ! initWindowBase(context, NULL, self, &pWB) )
    {
        goto done_out;
    }

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->BufferData(cselfBuffer);

    pcpbd->autoDetect = TRUE;
    pcpbd->wndBase = pWB;
    pcpbd->rexxSelf = self;
    pcpbd->hDlg = NULL;
    context->SetObjectVariable("CSELF", cselfBuffer);

    // The adm attribute needs to be set to whatever the result of allocating
    // the dialog administration block is.  If it is null, then the attribute
    // will be set to 0, which is what most of the existing ooDialog code
    // expects.

    DIALOGADMIN *dlgAdm = allocDlgAdmin(context);

    pcpbd->dlgAdm = dlgAdm;
    context->SetObjectVariable("ADM", pointer2string(context, dlgAdm));
    if ( dlgAdm == NULL )
    {
        goto done_out;
    }

    // Initialize the event notification mixin class.  The only thing that could
    // fail is getting a buffer from the interpreter kernel.  If that happens an
    // exceptions is raised and we should not need to do any clean up.
    pCEventNotification pEN = NULL;
    if ( ! initEventNotification(context, dlgAdm, self, &pEN) )
    {
        goto done_out;
    }
    pcpbd->enCSelf = pEN;

    result = TheZeroObj;

    context->SetObjectVariable("PARENTDLG", TheNilObj);
    context->SetObjectVariable("FINISHED", TheZeroObj);
    context->SetObjectVariable("PROCESSINGLOAD", TheFalseObj);

    // We don't check the return of AddTheMessage() because the message table
    // can not be full at this point, we are just starting out.  A memory
    // allocation failure, which is highly unlikely, will just be ignored.  If
    // this ooRexx process is out of memory, that will quickly show up.
    AddTheMessage(dlgAdm, WM_COMMAND, 0xFFFFFFFF, IDOK,     UINTPTR_MAX, 0, 0, "OK", 0);
    AddTheMessage(dlgAdm, WM_COMMAND, 0xFFFFFFFF, IDCANCEL, UINTPTR_MAX, 0, 0, "Cancel", 0);
    AddTheMessage(dlgAdm, WM_COMMAND, 0xFFFFFFFF, IDHELP,   UINTPTR_MAX, 0, 0, "Help", 0);

    // Set our default font to the PlainBaseDialog class default font.
    pCPlainBaseDialogClass pcpbdc = getPBDClass_CSelf(c);
    strcpy(pcpbd->fontName, pcpbdc->fontName);
    pcpbd->fontSize = pcpbdc->fontSize;

    // TODO at this point calculate the true dialog base units and set them into CPlainBaseDialog.

    c->SendMessage1(self, "CHILDDIALOGS=", rxNewList(context));       // self~childDialogs = .list~new
    c->SendMessage0(self, "INITAUTODETECTION");                       // self~initAutoDetection
    c->SendMessage1(self, "DATACONNECTION=", c->NewArray(50));        // self~dataConnection = .array~new(50)
    c->SendMessage1(self, "AUTOMATICMETHODS=", rxNewQueue(context));  // self~autoMaticMethods = .queue~new

    RexxDirectoryObject constDir = c->NewDirectory();
    c->SendMessage1(self, "CONSTDIR=", constDir);                     // self~constDir = .directory~new

    c->DirectoryPut(constDir, c->Int32(IDOK),             "IDOK");
    c->DirectoryPut(constDir, c->Int32(IDCANCEL),         "IDCANCEL");
    c->DirectoryPut(constDir, c->Int32(IDHELP),           "IDHELP");
    c->DirectoryPut(constDir, c->Int32(IDC_STATIC),       "IDC_STATIC");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_OODIALOG), "IDI_DLG_OODIALOG");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_APPICON),  "IDI_DLG_APPICON");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_APPICON2), "IDI_DLG_APPICON2");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_OOREXX),   "IDI_DLG_OOREXX");
    c->DirectoryPut(constDir, c->Int32(IDI_DLG_DEFAULT),  "IDI_DLG_DEFAULT");

    if ( argumentExists(4) )
    {
        c->SendMessage1(self, "PARSEINCLUDEFILE", hFile);
    }

done_out:
    c->SendMessage1(self, "INITCODE=", result);
err_out:
    return result;
}

RexxMethod1(RexxObjectPtr, pbdlg_unInit, CSELF, pCSelf)
{
    if ( pCSelf != NULLOBJECT )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

        DIALOGADMIN *adm = pcpbd->dlgAdm;
        if ( adm != NULL )
        {
            EnterCriticalSection(&crit_sec);

            if ( dialogInAdminTable(adm) )
            {
                DelDialog(adm);
            }
            safeLocalFree(adm->pMessageQueue);
            LocalFree(adm);
            ((pCPlainBaseDialog)pCSelf)->dlgAdm = NULL;

            LeaveCriticalSection(&crit_sec);
        }

        pCWindowBase pcwb = pcpbd->wndBase;
        if ( pcwb->rexxHwnd != TheZeroObj )
        {
            context->ReleaseGlobalReference(pcwb->rexxHwnd);
            pcwb->rexxHwnd = TheZeroObj;
        }
    }

    // Set the adm attribute to null here, to be sure it reflects that, no
    // matter what, dlgAdm is null at this point.
    context->SetObjectVariable("ADM", TheZeroObj);

    return TheZeroObj;
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
 */
RexxMethod1(CSTRING, pbdlg_getFontName, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->fontName );
}
/** PlainBaseDialog::fontSize  [attribute get]
 */
RexxMethod1(uint32_t, pbdlg_getFontSize, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->fontSize );
}

/** PlainBaseDialog::dlgHandle  [attribute get] / PlainBaseDialog::getSelf()
 */
RexxMethod1(RexxObjectPtr, pbdlg_getDlgHandle, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->wndBase->rexxHwnd );
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
 *            implementation has always been to get the handle of the topDlg.  I
 *            have never understood what the point of this method is, since the
 *            topDlg, usually, just reflects the last dialog created.
 */
RexxMethod0(RexxObjectPtr, pbdlg_get)
{
    if (topDlg && topDlg->TheDlg)
    {
        return pointer2string(context, topDlg->TheDlg);
    }
    else
    {
        return TheZeroObj;  // TODO for now, 0. Should be null Pointer.
    }
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
 *  @return  This method does not return a value.
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
    return NULLOBJECT;
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
    oodResetSysErrCode(context->threadContext);

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    RexxStringObject result = context->NullString();

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_WINDOW_HANDLE);
    }
    else
    {
        HWND hCtrl = GetDlgItem(pcpbd->hDlg, id);
        if ( hCtrl == NULL )
        {
            oodSetSysErrCode(context->threadContext);
        }
        else
        {
            rxGetWindowText(context, hCtrl, &result);
        }
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
    oodResetSysErrCode(context->threadContext);

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    RexxObjectPtr result = TheOneObj;

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_WINDOW_HANDLE);
        result = TheNegativeOneObj;
    }
    else
    {
        HWND hCtrl = GetDlgItem(pcpbd->hDlg, id);
        if ( hCtrl == NULL )
        {
            oodSetSysErrCode(context->threadContext);
        }
        else
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
 *  @return The window handle of the specified dialog control on success. -1 if
 *          the ID can not be resolved.  A null handle there is no such control.
 *
 *  @note  Sets the .SystemErrorCode.
 */
RexxMethod3(RexxObjectPtr, pbdlg_getControlHandle, RexxObjectPtr, rxID, OPTIONAL_RexxStringObject, _hDlg, CSELF, pCSelf)
{
    oodResetSysErrCode(context->threadContext);
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
        oodSetSysErrCode(context->threadContext, ERROR_INVALID_WINDOW_HANDLE);
        return TheNegativeOneObj;
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
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->hDlg == NULL )
    {
        noWindowsDialogException(context, pcpbd->rexxSelf);
        return TheFalseObj;
    }

    if ( *method == 'M' )
    {
        uint32_t flag = (method[1] == 'I' ? SW_SHOWMINIMIZED : SW_SHOWMAXIMIZED);
        return (ShowWindow(pcpbd->hDlg, flag) ? TheZeroObj : TheOneObj);
    }

    if ( method[3] == 'A' )
    {
        // Zoomed is Maximized.
        return (IsZoomed(pcpbd->hDlg) ? TheTrueObj : TheFalseObj);
    }

    // Iconic is Minimized.
    return (IsIconic(pcpbd->hDlg) ? TheTrueObj : TheFalseObj);
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
 *
 *  @remarks  The original ooDialog code checked if the dlgAdm was still in the
 *            DialogAdmin table.  ??  This would be true for a dialog in a
 *            CategoryDialog, if the dialog was the active child, and false if
 *            it was a good dialog, but for one of the other pages.  That may
 *            have been the point of the method.
 *
 *            Since the DialogTable will be going away, this method will need to
 *            be re-thought.
 */
RexxMethod1(logical_t, pbdlg_isDialogActive, CSELF, pCSelf)
{
    return seekDlgAdm(((pCPlainBaseDialog)pCSelf)->hDlg) != NULL;
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
 *            oodName2control() special cases connectSeparator to
 *            winNotAControl.  winNotAControls is what is expected by the data
 *            table code.
 *
 */
RexxMethod6(RexxObjectPtr, pbdlg_connect_ControName, RexxObjectPtr, rxID, OPTIONAL_RexxObjectPtr, attributeName,
            OPTIONAL_CSTRING, opts, NAME, msgName, OSELF, self, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    if ( pcpbd->dlgAdm == NULL )
    {
        return TheOneObj;
    }
    // result will be the resolved resource ID, which may be -1 on error.
    RexxObjectPtr result = context->ForwardMessage(NULLOBJECT, "ADDATTRIBUTE", NULLOBJECT, NULLOBJECT);

    int id;
    if ( ! context->Int32(result, &id) || id == -1 )
    {
        return TheNegativeOneObj;
    }

    oodControl_t type = oodName2controlType(msgName + 7);

    uint32_t category = getCategoryNumber(context, self);
    return ( addToDataTable(context, pcpbd->dlgAdm, id, type, category) == 0 ? TheZeroObj : TheOneObj );
}


RexxMethod2(uint32_t, pbdlg_setDlgDataFromStem_pvt, RexxStemObject, internDlgData, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    return setDlgDataFromStem(context, pcpbd, internDlgData);
}


RexxMethod2(uint32_t, pbdlg_putDlgDataInStem_pvt, RexxStemObject, internDlgData, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
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
 *  @param  rxID  The reosource ID of control.
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
RexxMethod3(RexxObjectPtr, plbdlg_getControlData, RexxObjectPtr, rxID, NAME, msgName, CSELF, pCSelf)
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
 *  @param  rxID  The reosource ID of control.
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


RexxMethod2(int32_t, pbdlg_stopIt, OPTIONAL_RexxObjectPtr, caller, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    if ( pcpbd->hDlg == NULL || pcpbd->dlgAdm == NULL )
    {
        return -1;
    }

    // PlainBaseDialog::leaving() does nothing.  It is intended to be over-
    // ridden by the Rexx programmer to do whatever she would want.
    context->SendMessage0(pcpbd->rexxSelf, "LEAVING");

    int32_t result = stopDialog(pcpbd->hDlg);

    pcpbd->hDlg = NULL;
    pcpbd->wndBase->hwnd = NULL;
    pcpbd->wndBase->rexxHwnd = TheZeroObj;

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


/**
 *  Methods for the .BaseDialog class.
 */
#define BASEDIALOG_CLASS              "BaseDialog"
#define CONTROLBAG_ATTRIBUTE          "BaseDialogControlBag"

/** BaseDialog::init()
 */
RexxMethod3(RexxObjectPtr, baseDlg_init, ARGLIST, args, SUPER, super, OSELF, self)
{
    RexxObjectPtr result = context->ForwardMessage(NULL, NULL, super, NULL);

    if ( isInt(0, result, context) )
    {
        context->SendMessage1(self, "SCROLLNOW=", TheZeroObj);
        context->SendMessage1(self, "BKGBITMAP=", TheZeroObj);
        context->SendMessage1(self, "BKGBRUSHBMP=", TheZeroObj);
        context->SendMessage1(self, "MENUBAR=", context->Nil());
        context->SendMessage1(self, "ISLINKED=", TheFalseObj);
    }

    return result;
}


/** BaseDialog::newXXX()
 *
 *  Instantiates a dialog control object for the specified Windows control.  All
 *  dialog control objects are instantiated through one of the BaseDialog
 *  newXXX() methods. In turn each of those methods filter through this
 *  function. newEdit(), newPushButton(), newListView(), etc..
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
 */
RexxMethod5(RexxObjectPtr, baseDlg_newControl, RexxObjectPtr, rxID, OPTIONAL_uint32_t, categoryPageID,
            NAME, msgName, OSELF, self, CSELF, pCSelf)
{
    RexxMethodContext *c = context;
    RexxObjectPtr result = TheNilObj;

    bool isCategoryDlg = false;
    HWND hDlg = ((pCPlainBaseDialog)pCSelf)->hDlg;

    if ( c->IsOfType(self, "CATEGORYDIALOG") )
    {
        if ( ! getCategoryHDlg(context, self, &categoryPageID, &hDlg, argumentExists(2)) )
        {
            goto out;
        }
        isCategoryDlg = true;
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
    oodControl_t controlType = oodName2controlType(msgName + 3);
    if ( ! isControlMatch(hControl, controlType) )
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

    RexxClassObject controlCls = oodClass4controlType(context, controlType);
    if ( controlCls == NULLOBJECT )
    {
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

RexxMethod2(RexxObjectPtr, baseDlg_putControl_pvt, RexxObjectPtr, control, OSELF, self)
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

RexxMethod3(RexxObjectPtr, baseDlg_test, int, x, CSTRING, y, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    /*
    size_t count = c->ArrayItems(args);
    size_t size = c->ArraySize(args);
    printf("Arg count=%d size=%d\n", count, size);
    */
    return TheTrueObj;
}


/**
 *  Methods for the .ResDialog class.
 */
#define RESDIALOG_CLASS        "ResDialog"


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


/* dump out the dialog admin table(s) */

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

