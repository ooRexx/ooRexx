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
#include "oodText.hpp"
#include "oodData.hpp"
#include "oodResourceIDs.hpp"

extern MsgReplyType SearchMessageTable(ULONG message, WPARAM param, LPARAM lparam, DIALOGADMIN * addressedTo);
extern BOOL DrawBitmapButton(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam, BOOL MsgEnabled);
extern BOOL DrawBackgroundBmp(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam);
extern LRESULT PaletteMessage(DIALOGADMIN * addr, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL AddDialogMessage(CHAR * msg, CHAR * Qptr);
extern LONG SetRexxStem(const char * name, INT id, const char * secname, const char * data);

/* Shared functions for keyboard hooks and key press subclassing */
extern LONG setKeyPressData(KEYPRESSDATA *, CONSTRXSTRING, CONSTRXSTRING, const char *);
extern void processKeyPress(KEYPRESSDATA *, WPARAM, LPARAM, PCHAR);
extern void freeKeyPressData(KEYPRESSDATA *);
extern UINT seekKeyPressMethod(KEYPRESSDATA *, const char *);
extern void removeKeyPressMethod(KEYPRESSDATA *, UINT);

/* Local functions */
static LONG installKBHook(DIALOGADMIN *, HWND, CONSTRXSTRING, CONSTRXSTRING, const char *);
static LONG setKBHook(DIALOGADMIN *, HWND);
static void removeKBHook(DIALOGADMIN *);
static BOOL parseKeyToken(PCHAR, PUINT, PUINT);
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
    AddDialogMessage((char *) MSG_TERMINATE, aDlg->pMessageQueue);

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

#ifdef __CTL3D
    if ((!StoredDialogs) && (aDlg->Use3DControls)) Ctl3dUnregister(aDlg->TheInstance);
#endif

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


size_t RexxEntry GetDialogFactor(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   ULONG u;
   double x,y;
   u = GetDialogBaseUnits();


   x = LOWORD(u);
   y = HIWORD(u);

   x = x / 4;
   y = y / 8;

   sprintf(retstr->strptr, "%4.1f %4.1f", x, y);
   retstr->strlength = strlen(retstr->strptr);
   return 0;
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

/**
 * Invalidates a rectangle in a window and has the window update.  This should
 * cause the window to immediately repaint the rectangle.
 *
 * @param hwnd              Handle to the window to be redrawn.
 * @param pr                Pointer to a rect structure specifying the area to
 *                          be redrawn.  If this arg is null, the entire client
 *                          area is redrawn.
 * @param eraseBackground   Should the background of the window be redrawn
 *                          during the repaint.
 *
 * @return True on success, otherwise false.
 *
 * @remarks  This is common code for several API methods.
 */
bool redrawRect(HWND hwnd, PRECT pr, bool eraseBackground)
{
    if ( InvalidateRect(hwnd, pr, eraseBackground) )
    {
       UpdateWindow(hwnd);
       return true;
    }
    return false;
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
 *
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
 *
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

RexxMethod1(uint32_t, wb_update, CSELF, pCSelf)
{
    RECT r;
    HWND hwnd = getWBWindow(pCSelf);
    GetClientRect(hwnd, &r);
    InvalidateRect(hwnd, &r, TRUE);
    return 0;
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

/** WindowsBase::draw() / WindowsBase::redraw() / WindowsBase::update()
 *
 *  Causes the entire client area of the the window to be redrawn.
 *
 *  This method maps to the draw(), update() and the redrawClient() methods.
 *  The implementation preserves existing behavior prior to ooRexx 4.0.0.  That
 *  is: the draw() method takes no argument and always uses false for the erase
 *  background arg. The update() method takes no argument and always uses true
 *  for the erase background arg, The redrawClient() method takes an argument to
 *  set the erase background arg. The argument can be either .true / .false (1
 *  or 0), or yes / no, or ja / nein.
 *
 *  @param  erase  [optional]  Whether the redraw operation should erase the
 *                 window background.  Can be true / false or yes / no.  The
 *                 default is false.
 *
 *  @return  0 for success, 1 for error.
 */
RexxMethod2(uint32_t, wb_redrawClient, OPTIONAL_CSTRING, erase, CSELF, pCSelf)
{
    RECT r;
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

    GetClientRect(hwnd, &r);
    return redrawRect(hwnd, &r, doErase) ? 0 : 1;
}

/** WindowsBase::redraw()
 *
 *  Causes the entire window, including frame, to be redrawn.
 *
 *  @return  0 for success, 1 for error.
 */
RexxMethod1(logical_t, wb_redraw, CSELF, pCSelf)
{
    return RedrawWindow(getWBWindow(pCSelf), NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN) != 0;
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
 *
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

RexxMethod2(uint32_t, wb_getWindowLong_pvt, int32_t, flag, CSELF, pCSelf)
{
    return GetWindowLong(getWBWindow(pCSelf), flag);
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


/** PlainBaseDialog::getItem()
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
 */
RexxMethod3(RexxObjectPtr, pbdlg_getItem, RexxObjectPtr, rxID, OPTIONAL_RexxStringObject, _hDlg, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

    uint32_t id;
    if ( ! oodSafeResolveID(&id, context, pcpbd->rexxSelf, rxID, -1, 1) || (int)id < 0 )
    {
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
    return pointer2string(context, GetDlgItem(hDlg, id));
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
        noWindowsDialogException(context, pcpbd);
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
        noWindowsDialogException(context, pcpbd);
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

RexxMethod2(RexxObjectPtr, baseDlg_test, OSELF, self, CSELF, pCSelf)
{
    RexxMethodContext *c = context;

    void *dlgCSelf = c->ObjectToCSelf(self);
    pCPlainBaseDialog realDlgCSelf = dlgToCSelf(c, self);

    dbgPrintClassID(context, self);
    printf("baseDlg_test() self=%p pCSelf=%p ObjectToCSelf(self)=%p ObjectToCSelf(self, self~class)=%p\n",
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


/**
 * Used to access Win32 API functions not involved with sending window messages
 * to dialogs or dialog controls.  General purpose functions.
 *
 * The parameters sent from ooRexx as an array of RXString:
 *
 * argv[0]  Selects general category.
 *
 * argv[1]  Picks specific function.
 *
 * argv[2] ... argv[n]  Varies depending on the function.
 *
 * Return to ooRexx, in general:
 *  Nothing to generalize as of yet ...
 *
 */
size_t RexxEntry WinAPI32Func(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    /* There has to be at least 2 args. */
    CHECKARGL(2);

    if ( argv[0].strptr[0] == 'G' )         /* Get something                  */
    {
        if ( !strcmp(argv[1].strptr, "WNDSTATE") )      /* get Window state      */
        {
            HWND hWnd;

            CHECKARGL(4);

            hWnd = GET_HWND(argv[3]);
            if ( hWnd == 0 || ! IsWindow(hWnd) ) RETERR

            else if ( argv[2].strptr[0] == 'Z' )     /* Zoomed is Maximized */
            {
                RETVAL((BOOL)IsZoomed(hWnd));
            }
            else if ( argv[2].strptr[0] == 'I' )     /* Iconic is Minimized */
            {
                RETVAL((BOOL)IsIconic(hWnd));
            }
        }
        else if ( !strcmp(argv[1].strptr, "ID") )       /* get dialog control ID */
        {
            HWND hWnd;
            INT  id;

            CHECKARGL(3);

            hWnd = GET_HWND(argv[2]);
            if ( hWnd == 0 || ! IsWindow(hWnd) ) RETVAL(-1)

            SetLastError(0);
            id = GetDlgCtrlID(hWnd);
            if ( ! id )
                id = -(INT)GetLastError();

            RETVAL(id);
        }
    }
    else if ( argv[0].strptr[0] == 'K' )    /* work with Keyboard hook */
    {
        DIALOGADMIN *dlgAdm = NULL;
        HWND         hDlg;

        CHECKARGL(3)

        dlgAdm = (DIALOGADMIN *)GET_POINTER(argv[2]);
        if ( !dlgAdm ) RETVAL(-4)

        if ( argv[1].strptr[0] == 'Q' )          /* Query if keyboard hook is installed */
        {
            if ( ! dlgAdm->hHook ) RETVAL(0)
            if ( argc == 3 ) RETVAL(1)

            CHECKARGL(4)

            RETVAL(seekKeyPressMethod(dlgAdm->pKeyPressData, argv[3].strptr) ? 1 : 0)
        }
        else if ( argv[1].strptr[0] == 'C' )     /* Connect method to keyboard hook for this dialog */
        {
            LONG ret;

            CHECKARGL(6)

            hDlg = GET_HWND(argv[3]);
            if ( hDlg == 0 || ! IsWindow(hDlg) ) RETVAL(-4)

            if ( argv[4].strlength == 0 || argv[5].strlength == 0 ) RETVAL(-1)

            /* If there is no existing hook, install one and connect the method
             * to it, otherwise connect the method to the existing hook.
             */
            if ( ! dlgAdm->hHook )
            {
                if ( argc > 6 )
                    ret = installKBHook(dlgAdm, hDlg, argv[4], argv[5], argv[6].strptr);
                else
                    ret = installKBHook(dlgAdm, hDlg, argv[4], argv[5], NULL);
            }
            else
            {
                if ( argc > 6 )
                    ret = setKeyPressData(dlgAdm->pKeyPressData, argv[4], argv[5], argv[6].strptr);
                else
                    ret = setKeyPressData(dlgAdm->pKeyPressData, argv[4], argv[5], NULL);
            }

            RETVAL(ret)
        }
        else if ( argv[1].strptr[0] == 'R' )     /* Remove method from keyboard hook for this dialog */
        {
            LONG ret = -1;

            if ( dlgAdm->hHook )
            {
                /* If there is no method arg, remove the entire hook, otherwise
                 * disconnect the method from the hook.
                 */
                if ( argc == 3 )
                {
                    removeKBHook(dlgAdm);
                    ret = 0;
                }
                else
                {
                    UINT index;

                    CHECKARGL(5)

                    hDlg = GET_HWND(argv[3]);
                    if ( hDlg == 0 || ! IsWindow(hDlg) ) RETVAL(-4)

                    index = seekKeyPressMethod(dlgAdm->pKeyPressData, argv[4].strptr);
                    if ( index == 0 ) RETVAL(-1)

                    /* If there is only 1 method connected to the hook, remove
                     * the hook completely.  Otherwise, unhook the hook, fix up
                     * the key press data, and reset the hook.
                     */
                    if ( dlgAdm->pKeyPressData->usedMethods == 1 )
                    {
                        removeKBHook(dlgAdm);
                        ret = 0;
                    }
                    else
                    {
                        UnhookWindowsHookEx(dlgAdm->hHook);
                        removeKeyPressMethod(dlgAdm->pKeyPressData, index);
                        ret = setKBHook(dlgAdm, hDlg);
                    }
                }
            }
            RETVAL(ret)
        }
    }

    RETERR
}


/**
 * The keyboard hook procedure.
 *
 * This is a thread specific hook, not a global hook. This function executes in
 * the same thread as the dialog's window procedure.  The dialog admin structure
 * stores the key press data, the thread ID is used to locate the correct dialog
 * admin.
 *
 * The key is examined to see if the user has set an ooDialog method for it and
 * if it is a key down event.  If so, the key data is sent on to
 * processKeyData() where the actual ooDialog method invocation is set up.  If
 * the user has also set a filter, there may be no method invocation after all.
 *
 */
LRESULT CALLBACK KeyboardHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    register int i;
    DWORD id = GetCurrentThreadId();
    DIALOGADMIN * dlgAdm;

    /* No matter what, we need to find the dialog admin struct, it is the only
     * way to get the hook handle.
     */
    for ( i = 0; i < StoredDialogs; i++ )
        if ( DialogTab[i]->threadID == id ) break;

    /* If we can't find it, there is nothing to do about it.  We can't call the
     * next hook, so just return 0.
     */
    if ( i >= StoredDialogs ) return 0;

    dlgAdm = DialogTab[i];

    if ( (code == HC_ACTION) && dlgAdm->pKeyPressData->key[wParam] )
    {
        if ( !(lParam & KEY_REALEASE) && !(lParam & KEY_WASDOWN) )
        {
            processKeyPress(dlgAdm->pKeyPressData, wParam, lParam, dlgAdm->pMessageQueue);
        }
    }
	return CallNextHookEx(dlgAdm->hHook, code, wParam, lParam);
}


/**
 * Takes a key event that has an ooDialog method connected to it, sets up the
 * method invocation message, and places it in the ooDialog message queue.
 *
 * It is possible for the key event to be filtered out and no ooDialog is then
 * invoked.
 *
 * The ooDialog event method gets 5 arguments:
 *   key:      decimal value of the key code.
 *   shift:    true / false, true if the shift key was depressed for this event.
 *   control:  true / false, true if control key was depressed.
 *   alt:      true / false, ditto.
 *   info:     Keyword string that specifies if right or left shift / control /
 *             alt were down and the state of the num lock, caps lock, and
 *             scroll lock keys.  The string contains some combination of:
 *
 *             rShift, lShift, rControl lControl, rAlt, lAlt, numOn, numOff,
 *             capsOn, capsOff, scrollOn, scrollOf
 */
void processKeyPress(KEYPRESSDATA *pKeyData, WPARAM wParam, LPARAM lParam, PCHAR pMessageQueue)
{
    /* Method name can not be longer than 197 chars.  This is checked for in
     * setKeyPressData()
     */
    CHAR oodMsg[256];
    BOOL passed = TRUE;
    INT i = pKeyData->key[wParam];
    PCHAR pMethod = pKeyData->pMethods[i];
    KEYFILTER *pFilter = pKeyData->pFilters[i];

    BOOL bShift = (GetAsyncKeyState(VK_SHIFT) & ISDOWN) ? 1 : 0;
    BOOL bControl = (GetAsyncKeyState(VK_CONTROL) & ISDOWN) ? 1 : 0;
    BOOL bAlt = (GetAsyncKeyState(VK_MENU) & ISDOWN) ? 1 : 0;

    if ( pFilter )
    {
        if ( pFilter->none )
        {
            passed = !bShift && !bControl && !bAlt;
        }
        else if ( pFilter->and )
        {
            passed = (pFilter->shift ? bShift : !bShift) &&
                     (pFilter->control ? bControl : !bControl) &&
                     (pFilter->alt ? bAlt : !bAlt);
        }
        else
        {
            passed = (pFilter->shift && bShift) ||
                     (pFilter->control && bControl) ||
                     (pFilter->alt && bAlt);
        }
    }

    if ( passed )
    {
        CHAR info[64] = {'\0'};

        if ( GetKeyState(VK_NUMLOCK) & KEY_TOGGLED )
            strcpy(info, "numOn");
        else
            strcpy(info, "numOff");

        if ( GetKeyState(VK_CAPITAL) & KEY_TOGGLED )
            strcat(info, " capsOn");
        else
            strcat(info, " capsOff");

        if ( bShift )
        {
            if ( GetAsyncKeyState(VK_RSHIFT) & ISDOWN )
                strcat(info, " rShift");
            else
                strcat(info, " lShift");
        }
        if ( bControl )
        {
            if ( GetAsyncKeyState(VK_RCONTROL) & ISDOWN )
                strcat(info, " rControl");
            else
                strcat(info, " lControl");
        }
        if ( bAlt )
        {
            if ( GetAsyncKeyState(VK_RMENU) & ISDOWN )
                strcat(info, " rAlt");
            else
                strcat(info, " lAlt");
        }

        if ( GetKeyState(VK_SCROLL) & KEY_TOGGLED )
            strcat(info, " scrollOn");
        else
            strcat(info, " scrollOff");

        sprintf(oodMsg, "%s(%u,%u,%u,%u,\"%s\")", pMethod, wParam, bShift, bControl, bAlt, info);
        AddDialogMessage((char *)oodMsg, pMessageQueue);
    }
}


/**
 * Allocates memory for the key press structure and sets up all the data used by
 * the keyboard hook procedure.  Once everything is good the hook is set.
 *
 */
static LONG installKBHook(
    DIALOGADMIN *dlgAdm,
    HWND hDlg,
    CONSTRXSTRING method,
    CONSTRXSTRING keys,
    const char *filter )
{
    KEYPRESSDATA *pData;
    LONG        ret = 0;

    if ( method.strlength == 0 || keys.strlength == 0 ) return -1;

    pData = (KEYPRESSDATA *)LocalAlloc(LPTR, sizeof(KEYPRESSDATA));
    if ( ! pData ) return -5;

    ret = setKeyPressData(pData, method, keys, filter);
    if ( ret == -5 )
    {
        LocalFree(pData);
        return -5;
    }
    else
    {
        dlgAdm->pKeyPressData = pData;
    }

    return setKBHook(dlgAdm, hDlg);
}


/**
 * Sets the Windows keyboard hook (WH_KEYBOARD.)  SetWindowsHookEx() has to run
 * in the same thread as the dialog, so a user message is sent to the dialog
 * window procedure to do the actual work.
 *
 * If the hook is not set, all the memory allocation is cleaned up.
 */
static LONG setKBHook(DIALOGADMIN *dlgAdm, HWND hDlg)
{
    LONG ret = 0;

    dlgAdm->hHook = (HHOOK)SendMessage(hDlg, WM_USER_HOOK, (WPARAM)&KeyboardHookProc, (LPARAM)0);
    if ( ! dlgAdm->hHook )
    {
        freeKeyPressData(dlgAdm->pKeyPressData);
        dlgAdm->pKeyPressData = NULL;
        ret = -2;
    }
    return ret;
}


/**
 * If the hook exists, unhook.  If the key press data exists, free it.
 */
static void removeKBHook(DIALOGADMIN *dlgAdm)
{
    if ( dlgAdm->hHook )
        UnhookWindowsHookEx(dlgAdm->hHook);

    freeKeyPressData(dlgAdm->pKeyPressData);
    dlgAdm->hHook = 0;
    dlgAdm->pKeyPressData = NULL;
}


/**
 * Removes the method at index from the key press data structure.  Assumes that
 * the keyboad hook is unhooked, or that the window subclass is removed.
 *
 */
void removeKeyPressMethod(KEYPRESSDATA *pData, UINT index)
{
    int i;

    if ( index >= 1 && index <= MAX_KEYPRESS_METHODS  )
    {
        for ( i = 0; i < COUNT_KEYPRESS_KEYS; i++)
        {
            if ( pData->key[i] == index ) pData->key[i] = 0;
        }

        if ( pData->pMethods[index] ) LocalFree(pData->pMethods[index]);
        if ( pData->pFilters[index] ) LocalFree(pData->pFilters[index]);
        pData->pMethods[index] = NULL;
        pData->pFilters[index] = NULL;
        pData->nextFreeQ[pData->topOfQ] = index;

        /* Theoretically we can't walk off the end of the array, but make sure
         * we don't.
         */
        if ( pData->topOfQ <= MAX_KEYPRESS_METHODS ) pData->topOfQ++;
        pData->usedMethods--;
    }
}

/**
 * Frees the key press data structure.  Note that methods can be removed leaving
 * holes in the array.
 */
void freeKeyPressData(KEYPRESSDATA *pData)
{
    UINT i;
    if ( pData )
    {
        for ( i = 1; i <= MAX_KEYPRESS_METHODS; i++ )
        {
            safeLocalFree((void *)pData->pMethods[i]);
            safeLocalFree((void *)pData->pFilters[i]);
        }
        LocalFree((void *)pData);
    }
}

/**
 * Searches the event method table for a matching method.  Returns 0 if no match
 * is found, otherwise the index into the table for the method.
 *
 */
UINT seekKeyPressMethod(KEYPRESSDATA *pData, const char *method)
{
    UINT i;
    UINT index = 0;

    if ( pData )
    {
        for ( i = 1; i <= MAX_KEYPRESS_METHODS; i++)
        {
            if ( pData->pMethods[i] && !strcmp(pData->pMethods[i], method) )
            {
                index = i;
                break;
            }
        }
    }
    return index;
}

/**
 * Set up the key press data for the specified method.
 *
 * It is possible that a key press comes in to the UI thread while this function
 * is executing:
 *
 * The key will have an already existing index or 0 while the method and filter
 * are being set up.  When a key slot is to be updated with the index of the new
 * method, it is protected by a critical section.
 *
 */
LONG setKeyPressData(KEYPRESSDATA *pData, CONSTRXSTRING method, CONSTRXSTRING keys, const char *filter)
{

    PSZ        token, str;
    PCHAR      pMethod;
    KEYFILTER *pFilter = NULL;
    UINT       firstKey, lastKey, index;
    LONG       ret = 0;

    /* If we are out room, or a duplicate method name, return */
    if ( pData->usedMethods >= (MAX_KEYPRESS_METHODS) ) return -6;
    if ( seekKeyPressMethod(pData, method.strptr) ) return -7;

    /* There has to be a limit on the length of a method name.  The size of the
     * message being sent to AddDialogMessage() is set at 256 (for the key press
     * event.)  Because of the arg string being sent to the method, this leaves
     * less than that for the method name.
     */
    if ( method.strlength > CCH_METHOD_NAME ) return -1;

    pMethod = (PCHAR)LocalAlloc(LPTR, method.strlength + 1);
    if ( ! pMethod ) return -5;

    if ( filter )
    {
        pFilter = (KEYFILTER *)LocalAlloc(LPTR, sizeof(KEYFILTER));
        if ( ! pFilter )
        {
            LocalFree(pMethod);
            return -5;
        }
    }
    rxstrlcpy(pMethod, method);

    if ( pFilter )
    {
        if ( strstr(filter, "NONE" ) )
        {
            pFilter->none = TRUE;
        }
        else
        {
            if ( strstr(filter, "AND") ) pFilter->and = TRUE;
            if ( strstr(filter, "SHIFT") ) pFilter->shift = TRUE;
            if ( strstr(filter, "CONTROL") ) pFilter->control = TRUE;
            if ( strstr(filter, "ALT") ) pFilter->alt = TRUE;
        }

        /* Some combinations are not filters.  Do not add more to the hook or
         * subclass procedure than needed.
         */
        if ( ((! pFilter->and) && pFilter->shift && pFilter->control && pFilter->alt) ||
             (pFilter->and && ! pFilter->shift && ! pFilter->control && ! pFilter->alt) )
        {
            LocalFree(pFilter);
            pFilter = NULL;
            ret = -1;
        }
    }

    /* Get the index of where to put the method.  If the next free queue is not
     * empty, pull the index from the queue, otherwise we are still adding
     * methods sequentially.
     */
    if ( pData->topOfQ )
    {
        index = pData->nextFreeQ[0];
        memmove(pData->nextFreeQ, pData->nextFreeQ + 1, (pData->topOfQ - 1) * sizeof(UINT));
        pData->topOfQ--;
    }
    else
    {
        index = pData->usedMethods + 1;
    }
    pData->pMethods[index] = pMethod;
    pData->usedMethods++;

    if ( pFilter )
    {
        pData->pFilters[index] = pFilter;
    }

    str = _strdup(keys.strptr);

    /* If there is an error parsing a token, the return is set to -1 to
     * indicate there was some error.  However, the good tokens are still used.
     * Thus, there may be some methods that are connected and some that are not.
     */
    token = strtok(str, ",");
    while( token != NULL )
    {
        if ( parseKeyToken(token, &firstKey, &lastKey) )
        {
            EnterCriticalSection(&crit_sec);
            if ( lastKey )
            {
                UINT i;
                for ( i = firstKey; i <= lastKey; i++)
                    pData->key[i] = index;
            }
            else
            {
                pData->key[firstKey] = index;
            }
            LeaveCriticalSection(&crit_sec);
        }
        else
        {
            ret = -1;
        }
        token = strtok(NULL, ",");
    }
    free(str);

    return ret;
}


/**
 * Parses a key token which could be: a keyword, a single number, or a number
 * range (n-m)  The results are checked to guarentee they are within bounds.
 *
 * Returns true on success, false on error.
 *
 * On true: *pFirst is guarenteed to be non-zero.  If the token represents a
 * single digit, *pLast is guarenteed to be zero.
 *
 * On false: *pFirst and *pLast are undefined.
 */
static BOOL parseKeyToken(PCHAR token, PUINT pFirst, PUINT pLast)
{
    PCHAR  p;
    BOOL   ret = TRUE;

    *pFirst = 0;
    *pLast = 0;

    if ( !strcmp(token, "ALL") )
    {
        *pFirst = VK_LBUTTON;
        *pLast =  VK_OEM_CLEAR;
    }
    else if ( !strcmp(token, "FKEYS") )
    {
        *pFirst = VK_F2;    /* F1 is handled by connectHelp in ooDialog */
        *pLast =  VK_F24;
    }
    else if ( !strcmp(token, "ALPHA") )
    {
        *pFirst = VK_A;
        *pLast =  VK_Z;
    }
    else if ( !strcmp(token, "NUMERIC") )
    {
        *pFirst = VK_0;
        *pLast =  VK_9;
    }
    else if ( !strcmp(token, "ALPHANUMERIC") )
    {
        *pFirst = VK_0;
        *pLast =  VK_Z;
    }
    else if ( (p = strchr(token, '-')) != NULL )
    {
        *p++ = '\0';
        *pFirst = atol(token);
        *pLast = atol(p);
        if ( (! *pFirst || ! *pLast) || (*pFirst > *pLast)       ||
             (*pFirst < VK_LBUTTON)  || (*pFirst > VK_OEM_CLEAR) ||
             (*pLast < VK_LBUTTON)   || (*pLast > VK_OEM_CLEAR)) ret = FALSE;
    }
    else
    {
        *pFirst = atol(token);
        *pLast = 0;
        if ( ! *pFirst || (*pFirst < VK_LBUTTON)   || (*pFirst > VK_OEM_CLEAR) ) ret = FALSE;
    }
    return ret;
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

