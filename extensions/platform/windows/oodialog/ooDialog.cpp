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
#include "ooDialog.h"     // Must be first, includes windows.h and oorexxapi.h

#include <mmsystem.h>
#include <commctrl.h>
#include <stdio.h>
#include <dlgs.h>
#include <shlwapi.h>
#include "APICommon.h"
#include "oodCommon.h"
#include "oodSymbols.h"

extern INT DelDialog(DIALOGADMIN * aDlg);
extern MsgReplyType SearchMessageTable(ULONG message, WPARAM param, LPARAM lparam, DIALOGADMIN * addressedTo);
extern BOOL DrawBitmapButton(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam, BOOL MsgEnabled);
extern BOOL DrawBackgroundBmp(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam);
extern BOOL DataAutodetection(DIALOGADMIN * aDlg);
extern LRESULT PaletteMessage(DIALOGADMIN * addr, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL AddDialogMessage(CHAR * msg, CHAR * Qptr);
extern LONG SetRexxStem(const char * name, INT id, const char * secname, const char * data);
extern BOOL GetDialogIcons(DIALOGADMIN *, INT, UINT, PHANDLE, PHANDLE);

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
    const char *resourceId;
    DIALOGADMIN *dlgAdmin;
    const char *autoDetect;
    BOOL *release;           // used for a return value
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
                 * LPARAM arg from UsrCreateDialog().
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
 * @param dlgAdm
 * @param ar
 * @param argc
 *
 * @return True on success, false only if this is for a ResDialog and the
 *         loading of the resource DLL failed.
 */
bool InstallNecessaryStuff(DIALOGADMIN* dlgAdm, CONSTRXSTRING ar[], size_t argc)
{
    const char *Library;

    if ( dlgAdm->previous )
    {
        ((DIALOGADMIN*)dlgAdm->previous)->OnTheTop = FALSE;
    }
    topDlg = dlgAdm;
    Library = ar[0].strptr;

    if (Library[0] != '0')
    {
        dlgAdm->TheInstance = LoadLibrary(Library);
        if (!dlgAdm->TheInstance)
        {
            CHAR msg[256];
            sprintf(msg, "Failed to load Dynamic Link Library (resource DLL.)\n"
                    "  File name:\t\t\t%s\n"
                    "  Windows System Error Code:\t%d\n", Library, GetLastError());
            MessageBox(0, msg, "ooDialog DLL Load Error", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
            return false;
        }
    }
    else
    {
        dlgAdm->TheInstance = MyInstance;
    }

    if (argc >= 4)
    {
        dlgAdm->Use3DControls = FALSE;
    }
    return true;
}



/* end a dialog and remove installed components */
INT DelDialog(DIALOGADMIN * aDlg)
{
    DIALOGADMIN * current;
    INT ret, i;
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

    // Post the WM_QUIT message to exit the message loop.
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
    CHAR buffer[NR_BUFFER];
    DIALOGADMIN * Dlg;
    BOOL * release;
    ULONG ret;

    LoopThreadArgs *args = (LoopThreadArgs *)arg;


    Dlg = args->dlgAdmin;        /*  thread local admin pointer from StartDialog */
    Dlg->TheDlg = CreateDialogParam( Dlg->TheInstance, MAKEINTRESOURCE(atoi(args->resourceId)), 0, (DLGPROC) RexxDlgProc, Dlg->Use3DControls);  /* pass 3D flag to WM_INITDIALOG */
    Dlg->ChildDlg[0] = Dlg->TheDlg;

    release = args->release;   /* the Release flag is stored as the 4th argument */
    if (Dlg->TheDlg)
    {
        if (args->autoDetect)
        {
            strcpy(buffer, args->autoDetect);
        }
        else
        {
            strcpy(buffer, "0");
        }

        if (isYes(buffer))
        {
            if (!DataAutodetection(Dlg))
            {
                Dlg->TheThread = NULL;
                return 0;
            };
        }

        *release = TRUE;  /* Release wait in StartDialog  */
        do
        {
            if (GetMessage(&msg,NULL, 0,0))
            {
                if (!IsDialogMessage (Dlg->TheDlg, &msg))
                {
                    DispatchMessage(&msg);
                }
            }
        } while (Dlg && dialogInAdminTable(Dlg) && !Dlg->LeaveDialog);
    }
    else
    {
        *release = TRUE;
    }
    EnterCriticalSection(&crit_sec);
    if (dialogInAdminTable(Dlg))
    {
        ret = DelDialog(Dlg);
        Dlg->TheThread = NULL;
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
 * Create a dialog stored in a DLL.  Currently this is only used by ResDialog.
 *
 * @param funcname
 * @param argc
 * @param argv
 * @param qname
 * @param retstr
 *
 * @return size_t RexxEntry
 */
size_t RexxEntry StartDialog(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    ULONG thID;
    BOOL Release = FALSE;
    DEF_ADM;

    CHECKARG(7);
    GET_ADM;
    if (!dlgAdm)
    {
        RETERR;
    }

    EnterCriticalSection(&crit_sec);
    if ( ! InstallNecessaryStuff(dlgAdm, &argv[1], argc-1) )
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
        RETC(0)
    }

    LoopThreadArgs threadArgs;
    threadArgs.resourceId = argv[2].strptr;
    threadArgs.dlgAdmin = dlgAdm;
    threadArgs.autoDetect = argv[3].strptr;
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
            if ( dlgAdm->previous && !isYes(argv[6].strptr) && IsWindowEnabled(((DIALOGADMIN *)dlgAdm->previous)->TheDlg) )
            {
                EnableWindow(((DIALOGADMIN *)dlgAdm->previous)->TheDlg, FALSE);
            }

            if ( GetDialogIcons(dlgAdm, atoi(argv[5].strptr), ICON_DLL, (PHANDLE)&hBig, (PHANDLE)&hSmall) )
            {
                dlgAdm->SysMenuIcon = (HICON)setClassPtr(dlgAdm->TheDlg, GCLP_HICON, (LONG_PTR)hBig);
                dlgAdm->TitleBarIcon = (HICON)setClassPtr(dlgAdm->TheDlg, GCLP_HICONSM, (LONG_PTR)hSmall);
                dlgAdm->DidChangeIcon = TRUE;

                SendMessage(dlgAdm->TheDlg, WM_SETICON, ICON_SMALL, (LPARAM)hSmall);
            }

            RETPTR(dlgAdm->TheDlg);
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
    RETC(0);
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
            systemServiceExceptionCode(c, API_FAILED_MSG, "GetWindowRect");
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
        wrongClassException(context, 1, "Buffer");
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
            userDefinedMsgException(context, 1, "The keyword FAST can not be used with INACTIVE");
            goto done_out;
        }
        type = 'I';
    }
    else
    {
        userDefinedMsgException(context, 1, "The keyword option string must contain one of NORMAL, DEFAULT, HIDE, INACTIVE");
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
    oodResetSysErrCode(context);
    RexxStringObject result = context->NullString();

    HWND hwnd = getWBWindow(pCSelf);
    uint32_t count = (uint32_t)GetWindowTextLength(hwnd);

    if ( count == 0 )
    {
        oodSetSysErrCode(context);
        return result;
    }

    // TODO For all windows except an edit control this is fine.  For an edit
    // control with a very large amount of text, see if it could be optimized by
    // using a string buffer.

    LPTSTR pBuf = (LPTSTR)malloc(++count);
    if ( pBuf == NULL )
    {
        outOfMemoryException(context);
        return result;
    }

    count = GetWindowText(hwnd, pBuf, count);
    if ( count != 0 )
    {
        result = context->String(pBuf);
    }
    else
    {
        oodSetSysErrCode(context);
    }
    free(pBuf);

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
    oodResetSysErrCode(context);
    if ( SetWindowText(getWBWindow(pCSelf), text) == 0 )
    {
        oodSetSysErrCode(context);
        return 1;
    }
    return 0;
}

RexxMethod2(uint32_t, wb_getWindowLong_pvt, int32_t, flag, CSELF, pCSelf)
{
    return GetWindowLong(getWBWindow(pCSelf), flag);
}

/**
 *  Methods for the .Window class.
 */
#define WINDOW_CLASS  "Window"

RexxMethod2(RexxObjectPtr, window_init, POINTERSTRING, hwnd, OSELF, self)
{
    if ( !IsWindow((HWND)hwnd) )
    {
        invalidTypeException(context, 1, " window handle");
    }
    else
    {
        initWindowBase(context, (HWND)hwnd, self, NULL);
    }
    return NULLOBJECT;
}

/** Window::unInit()
 *
 *  Release the global reference for CWindowBase::rexxHwnd.
 *
 */
RexxMethod1(RexxObjectPtr, window_unInit, CSELF, pCSelf)
{
    if ( pCSelf != NULLOBJECT )
    {
        pCWindowBase pcwb = (pCWindowBase)pCSelf;
        if ( pcwb->rexxHwnd != TheZeroObj )
        {
            context->ReleaseGlobalReference(pcwb->rexxHwnd);
            pcwb->rexxHwnd = TheZeroObj;
        }
    }
    return NULLOBJECT;
}


/**
 *  Methods for the .PlainBaseDialog class.
 */
#define PLAINBASEDIALOG_CLASS  "PlainBaseDialog"

bool convert2PointerSize(RexxMethodContext *c, RexxObjectPtr obj, uint64_t *number, int argPos)
{
    if ( obj == NULLOBJECT )
    {
        *number = 0;
        return true;
    }

    if ( c->IsPointer(obj) )
    {
        *number = (uint64_t)c->PointerValue((RexxPointerObject)obj);
        return true;
    }

    return rxStr2Number(c, c->ObjectToStringValue(obj), number, argPos);
}

RexxMethod0(RexxObjectPtr, pbdlg_init_cls)
{
    context->SetObjectVariable("FONTNAME", context->String(DEFAULT_FONTNAME));
    context->SetObjectVariable("FONTSIZE", context->WholeNumber(DEFAULT_FONTSIZE));
    return NULLOBJECT;
}

RexxMethod2(RexxObjectPtr, pbdlg_setDefaultFont_cls, CSTRING, fontName, uint32_t, fontSize)
{
    context->SetObjectVariable("FONTNAME", context->String(fontName));
    context->SetObjectVariable("FONTSIZE", context->WholeNumber(fontSize));
    return NULLOBJECT;
}

RexxMethod0(RexxObjectPtr, pbdlg_getFontName_cls)
{
    return context->GetObjectVariable("FONTNAME");
}
RexxMethod0(RexxObjectPtr, pbdlg_getFontSize_cls)
{
    return context->GetObjectVariable("FONTSIZE");
}

RexxMethod5(RexxObjectPtr, pbdlg_init, RexxObjectPtr, library, RexxObjectPtr, resource,
            OPTIONAL_RexxObjectPtr, dlgDataStem, OPTIONAL_RexxObjectPtr, hFile, OSELF, self)
{
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
    RexxBufferObject cpbdBuf = context->NewBuffer(sizeof(CPlainBaseDialog));
    if ( cpbdBuf == NULLOBJECT )
    {
        goto err_out;
    }

    // Initialize the window base.
    pCWindowBase pWB;
    if ( ! initWindowBase(context, NULL, self, &pWB) )
    {
        goto done_out;
    }

    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)context->BufferData(cpbdBuf);

    pcpbd->wndBase = pWB;
    pcpbd->rexxSelf = self;
    pcpbd->hDlg = NULL;

    context->SetObjectVariable("CSELF", cpbdBuf);

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

    context->SetObjectVariable("PARENTDLG", TheNilObj);
    context->SetObjectVariable("FINISHED", TheZeroObj);
    context->SetObjectVariable("PROCESSINGLOAD", TheFalseObj);

    // We don't check the return of AddTheMessage() because the message table
    // can not be full at this point, we are just starting out.  A memory
    // allocation failure, which is highly unlikely, will just be ignored.  If
    // this ooRexx process is out of memory, that will quickly show up.
    AddTheMessage(dlgAdm, WM_COMMAND, 0xFFFFFFFF, IDOK,     (ULONG_PTR)SIZE_MAX, 0, 0, "OK", 0);
    AddTheMessage(dlgAdm, WM_COMMAND, 0xFFFFFFFF, IDCANCEL, (ULONG_PTR)SIZE_MAX, 0, 0, "Cancel", 0);
    AddTheMessage(dlgAdm, WM_COMMAND, 0xFFFFFFFF, IDHELP,   (ULONG_PTR)SIZE_MAX, 0, 0, "Help", 0);

    if ( argumentOmitted(4) )
    {
        result = context->SendMessage0(self, "FINALINIT");
    }
    else
    {
        result = context->SendMessage1(self, "FINALINIT", hFile);
    }

done_out:
    context->SendMessage1(self, "INITCODE=", result);
err_out:
    return result;
}

/** PlainBaseDialog::dlgHandle  [attribute set private]
 *
 *  Sets the handle of the underlying Windows dialog.  When a PlainBaseDialog is
 *  first initialized, the dialog handle is of course unknown.  The underlying
 *  dialog has not been created.
 *
 *  While this ooDialog is still a mix of old and new native APIs has to remain
 *  an actual method implementation.  See for example:
 *
 *  self~dlgHandle = UsrCreateDialog(self~adm, "PARENT", self~DialogItemCount,
 *  0, self~BasePtr, self~autoDetect, 1, 0, 0)
 *
 *  When / if the functions that actually create the underlying Windows dialog
 *  are converted to the new APIs, then the code for this method can be done
 *  entirely in the native API context and the dlgHandle= method can be removed.
 *
 */
RexxMethod2(RexxObjectPtr, pbdlg_setDlgHandle, RexxStringObject, hDlg, CSELF, pCSelf)
{
    pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;
    pCWindowBase pcwb = pcpbd->wndBase;

    pcpbd->hDlg = (HWND)string2pointer(context, hDlg);

    if ( pcpbd->hDlg != NULL )
    {
        pcwb->hwnd = pcpbd->hDlg;
        pcwb->rexxHwnd = context->RequestGlobalReference(hDlg);
    }
    else
    {
        pcwb->rexxHwnd = TheZeroObj;
    }

    return NULLOBJECT;
}

/** PlainBaseDialog::dlgHandle  [attribute set private]
 */
RexxMethod1(RexxObjectPtr, pbdlg_getDlgHandle, CSELF, pCSelf)
{
    return ( ((pCPlainBaseDialog)pCSelf)->wndBase->rexxHwnd );
}

RexxMethod1(RexxObjectPtr, pbdlg_deInstall, CSELF, pCSelf)
{
    if ( pCSelf != NULLOBJECT )
    {
        pCPlainBaseDialog pcpbd = (pCPlainBaseDialog)pCSelf;

        DIALOGADMIN *adm = pcpbd->dlgAdm;
        if ( adm != NULL )
        {
            EnterCriticalSection(&crit_sec);

            if (dialogInAdminTable(adm))
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

/** PlainBaseDialog::addUserMessage()
 *
 *  Adds a message to the message table.
 *
 *  Each entry in the message table connects a Windows event message to a method
 *  in a Rexx dialog.  The fields for the entry consist of the Windows message,
 *  the WPARAM and LPARAM for the message, a filter for the message and its
 *  parameters, and the method name. Using the proper filters for the Windows
 *  message and its parameters allows the mapping of a very specific Windows
 *  event to the named method.
 *
 *  @param  methodName   [required]  The method name to be connected.
 *  @param  wm  [required]  The Windows event message
 *  @param  _wmFilter    [optional]  Filter applied to the Windows message.  If
 *                       omitted the filter is 0xFFFFFFFF.
 *  @param  wp           [optional]  WPARAM for the message
 *  @param  _wpFilter    [optional]  Filter applied to the WPARAM.  If omitted a
 *                       filter of all hex Fs is applied
 *  @param  lp           [optional]  LPARAM for the message.
 *  @param  _lpFilter    [optional]  Filter applied to LPARAM.  If omitted the
 *                       filter is all hex Fs.
 *  @param  -tag         [optional]  A tag that allows a further differentiation
 *                       between messages.  This is an internal mechanism not to
 *                       be documented publicly.
 *
 *  @return  0 on success, 1 on failure.  One possible source of error is the
 *           message table being full.
 *
 *  @remarks  Although it would make more sense to return true on succes and
 *            false on failure, there is too much old code that relies on 0 for
 *            success and 1 for error.
 */
RexxMethod9(logical_t, pbdlg_addUserMessage, CSTRING, methodName, CSTRING, wm, OPTIONAL_CSTRING, _wmFilter,
            OPTIONAL_RexxObjectPtr, wp, OPTIONAL_CSTRING, _wpFilter, OPTIONAL_RexxObjectPtr, lp, OPTIONAL_CSTRING, _lpFilter,
            OPTIONAL_CSTRING, _tag, OSELF, self)
{
    RexxMethodContext *c = context;
    logical_t result = 1;

    DIALOGADMIN *dlgAdm = rxGetDlgAdm(context, self);
    if ( dlgAdm == NULL )
    {
        goto done_out;
    }
    if ( *methodName == '\0' )
    {
        c->RaiseException1(Rexx_Error_Invalid_argument_null, TheOneObj);
        goto done_out;
    }

    uint64_t number;

    UINT winMessage;
    UINT wmFilter;
    if ( ! rxStr2Number(context, wm, &number, 2) )
    {
        goto done_out;
    }
    winMessage = (UINT)number;

    if ( argumentOmitted(3) )
    {
        wmFilter = 0xFFFFFFFF;
    }
    else
    {
        if ( ! rxStr2Number(context, _wmFilter, &number, 3) )
        {
            goto done_out;
        }
        wmFilter = (UINT)number;
    }

    WPARAM    wParam;
    ULONG_PTR wpFilter;

    if ( ! convert2PointerSize(context, wp, &number, 4) )
    {
        goto done_out;
    }
    wParam = (WPARAM)number;

    if ( argumentOmitted(5) )
    {
        wpFilter = 0;
    }
    else
    {
        if ( ! rxStr2Number(context, _wpFilter, &number, 5) )
        {
            goto done_out;
        }
        wpFilter = (number == 0xFFFFFFFF ? (ULONG_PTR)SIZE_MAX : (ULONG_PTR)number);
    }

    LPARAM    lParam;
    ULONG_PTR lpFilter;

    if ( ! convert2PointerSize(context, lp, &number, 6) )
    {
        goto done_out;
    }
    lParam = (WPARAM)number;

    if ( argumentOmitted(7) )
    {
        lpFilter = 0;
    }
    else
    {
        if ( ! rxStr2Number(context, _lpFilter, &number, 7) )
        {
            goto done_out;
        }
        lpFilter = (number == 0xFFFFFFFF ? (ULONG_PTR)SIZE_MAX : (ULONG_PTR)number);
    }

    ULONG tag = 0;
    if ( argumentExists(8) )
    {
        if ( ! rxStr2Number(context, _tag, &number, 8) )
        {
            goto done_out;
        }
        tag = (ULONG)number;
    }

    if ( (winMessage | wParam | lParam) == 0 )
    {
        userDefinedMsgException(context, "The wm, wp, and lp arguements can not all be 0" );
    }
    else
    {
        if ( AddTheMessage(dlgAdm, winMessage, wmFilter, wParam, wpFilter, lParam, lpFilter, methodName, tag) )
        {
            result = 0;
        }
    }

done_out:
    return result;
}

/** PlainBaseDialog::getTextSizeDlg()
 *
 *  Gets the size (width and height) in dialog units for any given string, for
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
            nullObjectException(context, "window handle", 4);
            goto error_out;
        }
        hwndSrc = (HWND)hwndFontSrc;
    }
    return getTextSize(context, text, fontName, fontSize, hwndSrc, self);

error_out:
    return NULLOBJECT;
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


size_t RexxEntry HandleDlg(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    DEF_ADM;

    CHECKARGL(1);

    if (!strcmp(argv[0].strptr, "ACTIVE"))   /* see if the dialog is still in the dialog management table */
    {
        CHECKARG(2);
        SEEK_DLGADM_TABLE(GET_HWND(argv[1]), dlgAdm);

        if (dlgAdm)    /* it's alive */
        {
            RETC(1);
        }
        else
        {
            RETC(0);
        }
    }
    else if (!strcmp(argv[0].strptr, "HNDL"))   /* Get the dialog handle */
    {
        if (argc==2)
        {
            dlgAdm = (DIALOGADMIN*)GET_POINTER(argv[1]);
            if (!dlgAdm)
            {
                RETERR;
            }
            RETHANDLE(dlgAdm->TheDlg);
        }
        else
        {
            if (topDlg && topDlg->TheDlg)
            {
                RETHANDLE(topDlg->TheDlg);
            }
            else
            {
                RETC(0);
            }
        }
    }
    else if (!strcmp(argv[0].strptr, "ITEM"))   /* Get the handle to a dialog item */
    {
        HWND hW, hD;

        CHECKARGL(2);

        if (argc > 2)
        {
            hD = GET_HWND(argv[2]);
        }
        else
        {
            hD = topDlg->TheDlg;
        }

        hW = GetDlgItem(hD, atoi(argv[1].strptr));

        if (hW == NULL)
        {
            RETC(0);
        }
        else
        {
            RETHANDLE(hW);
        }
    }
    else if (!strcmp(argv[0].strptr, "STOP"))   /* Stop a dialog */
    {
        HWND h = NULL;

        if (argc>1)
        {
            h= GET_HWND(argv[1]);
        }

        if (h)
        {
            SEEK_DLGADM_TABLE(h, dlgAdm);
            if (dlgAdm)
            {
                RETVAL(DelDialog(dlgAdm));
            }
            else
            {
                RETVAL(-1);
            }
        }
        else if (!h && topDlg)
        {
            RETVAL(DelDialog(topDlg));       /* remove the top most */
        }
        else
        {
            RETVAL(-1);
        }
    }
    RETC(0);
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
           itoa(dlgAdm->DataTab[i].typ, data, 10);
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

