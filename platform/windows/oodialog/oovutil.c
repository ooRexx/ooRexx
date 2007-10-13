/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
#define _WIN32_WINNT    0x0501
#define WINVER          0x0501

#include <windows.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <commctrl.h>
#define INCL_REXXSAA
#define INCL_RXMACRO
#include <rexx.h>
#include <stdio.h>
#include <dlgs.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif
/* set the noglob... so global variables wont be defined twice */
#define NOGLOBALVARIABLES 1
#include "oovutil.h"
#undef NOGLOBALVARIABLES
#include "oodResources.h"


extern HINSTANCE MyInstance = NULL;

extern DIALOGADMIN * DialogTab[MAXDIALOGS] = {NULL};
extern DIALOGADMIN * topDlg = {NULL};
extern INT StoredDialogs = 0;
extern CRITICAL_SECTION crit_sec = {0};
extern WPARAM InterruptScroll;
extern DWORD ComCtl32Version = 0;

extern BOOL SearchMessageTable(ULONG message, WPARAM param, LPARAM lparam, DIALOGADMIN * addressedTo);
extern BOOL DrawBitmapButton(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam, BOOL MsgEnabled);
extern BOOL DrawBackgroundBmp(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam);
extern BOOL DataAutodetection(DIALOGADMIN * aDlg);
extern LRESULT PaletteMessage(DIALOGADMIN * addr, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL AddDialogMessage(CHAR * msg, CHAR * Qptr);
extern BOOL IsNT = TRUE;
extern LONG HandleError(PRXSTRING r, CHAR * text);
extern LONG SetRexxStem(CHAR * name, INT id, char * secname, CHAR * data);
extern BOOL GetDialogIcons(DIALOGADMIN *, INT, BOOL, PHANDLE, PHANDLE);
extern HICON GetIconForID(DIALOGADMIN *, UINT, BOOL, int, int);
extern BOOL InitForCommonControls(void);

/* Shared functions for keyboard hooks and key press subclassing */
extern LONG setKeyPressData(KEYPRESSDATA *, RXSTRING, RXSTRING, PCHAR);
extern void processKeyPress(KEYPRESSDATA *, WPARAM, LPARAM, PCHAR);
extern void freeKeyPressData(KEYPRESSDATA *);
extern UINT seekKeyPressMethod(KEYPRESSDATA *, PCHAR);
extern void removeKeyPressMethod(KEYPRESSDATA *, UINT);

/* Local functions */
INT DelDialog(DIALOGADMIN * aDlg);
static LONG installKBHook(DIALOGADMIN *, HWND, RXSTRING, RXSTRING, PCHAR);
static LONG setKBHook(DIALOGADMIN *, HWND);
static void removeKBHook(DIALOGADMIN *);
static BOOL parseKeyToken(PCHAR, PUINT, PUINT);

#define COMCTL_ERR_TITLE    "ooDialog - Windows Common Controls Error"
#define GENERIC_ERR_TITLE   "ooDialog - Error"

LONG HandleError(PRXSTRING r, CHAR * text)
{
      HWND hW = NULL;
      if ((topDlg) && (topDlg->TheDlg)) hW = topDlg->TheDlg;
      MessageBox(hW,text,"Error",MB_OK | MB_ICONHAND);
      r->strlength = 2;
      r->strptr[0] = '4';
      r->strptr[1] = '0';
      r->strptr[2] = '\0';
      return 40;
}


BOOL DialogInAdminTable(DIALOGADMIN * Dlg)
{
    register INT i;
    for (i = 0; i < StoredDialogs; i++)
        if (DialogTab[i] == Dlg) break;
    return (i < StoredDialogs);
}

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
       if (!addressedTo && topDlg && DialogInAdminTable(topDlg) && topDlg->TheDlg) addressedTo = topDlg;

       if (addressedTo)
       {
          MsgEnabled = IsWindowEnabled(hDlg) && DialogInAdminTable(addressedTo);
          if (MsgEnabled && (uMsg != WM_PAINT) && (uMsg != WM_NCPAINT)                /* do not search message table for WM_PAINT to improve redraw */
              && SearchMessageTable(uMsg, wParam, lParam, addressedTo)) return FALSE;

          switch (uMsg) {
             case WM_PAINT:
                if (addressedTo->BkgBitmap) DrawBackgroundBmp(addressedTo, hDlg, wParam, lParam);
                break;

             case WM_DRAWITEM:
                if ((lParam != 0) && (addressedTo)) return DrawBitmapButton(addressedTo, hDlg, wParam, lParam, MsgEnabled);
                break;

             case WM_CTLCOLORDLG:
                if (addressedTo->BkgBrush) return (LRESULT) addressedTo->BkgBrush;
#ifdef __CTL3D
                else {
                   if (addressedTo->Use3DControls)  {
                       hbrush = Ctl3dCtlColorEx(uMsg, wParam, lParam);
                       addressedTo->BkgBrush = hbrush;
                       return (LRESULT) hbrush;
                   }
                }
#endif


             case WM_CTLCOLORSTATIC:
             case WM_CTLCOLORBTN:
             case WM_CTLCOLOREDIT:
             case WM_CTLCOLORLISTBOX:
             case WM_CTLCOLORMSGBOX:
             case WM_CTLCOLORSCROLLBAR:
                if (addressedTo->CT_size)   /* Has the dialog item its own user color ?*/
                {
                    LONG id = GetWindowLong((HWND)lParam, GWL_ID);
                    SEARCHBRUSH(addressedTo, i, id, hbrush);
                    if (hbrush)
                    {
                        SetBkColor((HDC)wParam, PALETTEINDEX(addressedTo->ColorTab[i].ColorBk));
                        if (addressedTo->ColorTab[i].ColorFG != -1) SetTextColor((HDC)wParam, PALETTEINDEX(addressedTo->ColorTab[i].ColorFG));
                    }
                }
#ifdef __CTL3D
                if (!hbrush && addressedTo->Use3DControls)
                    hbrush = Ctl3dCtlColorEx(uMsg, wParam, lParam);
#endif
                if (hbrush)
                   return (LRESULT) hbrush;
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
                hW = CreateDialogIndirectParam(MyInstance, (DLGTEMPLATE *) lParam, (HWND) wParam, RexxDlgProc, addressedTo->Use3DControls); /* pass 3D flag to WM_INITDIALOG */
                ReplyMessage((LRESULT) hW);
                return (LRESULT) hW;
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
                ReplyMessage((LRESULT)GetAsyncKeyState(wParam));
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



/* prepare dialog management table for a new dialog entry */
ULONG APIENTRY HandleDialogAdmin(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   DIALOGADMIN * current;
   DEF_ADM;

   EnterCriticalSection(&crit_sec);

   if (argc == 1)  /* we have to do a dialog admin cleanup */
   {
       GET_ADM;
       if (!dlgAdm)
       {
            LeaveCriticalSection(&crit_sec);
            RETVAL(-1)
       }

       if (DialogInAdminTable(dlgAdm)) DelDialog(dlgAdm);
       if (dlgAdm->pMessageQueue) LocalFree((void *)dlgAdm->pMessageQueue);
       LocalFree(dlgAdm);
   }
   else   /* we have to do a new dialog admin allocation */
   {
       if (StoredDialogs<MAXDIALOGS)
       {
          current = (DIALOGADMIN *) LocalAlloc(LPTR, sizeof(DIALOGADMIN));
          if (current) current->pMessageQueue = LocalAlloc(LPTR, MAXLENQUEUE);
          if (!current || !current->pMessageQueue)
          {
             MessageBox(0,"Out of system resources","Error",MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
             LeaveCriticalSection(&crit_sec);
             RETC(0)
          }
          current->previous = topDlg;
          current->TableEntry = StoredDialogs;
          StoredDialogs++;
          DialogTab[current->TableEntry] = current;
          LeaveCriticalSection(&crit_sec);
          RETVAL((ULONG)current)
       } else
       {
          MessageBox(0,"To many aktive Dialogs","Error",MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
       }
   }
   LeaveCriticalSection(&crit_sec);
   RETC(0)
}



/* install DLL, control program and 3D controls */
BOOL InstallNecessaryStuff(DIALOGADMIN* dlgAdm, RXSTRING ar[], INT argc)
{
   PCHAR Library;

   if (dlgAdm->previous) ((DIALOGADMIN*)dlgAdm->previous)->OnTheTop = FALSE;
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
         return FALSE;
      }
   } else
      dlgAdm->TheInstance = MyInstance;

   if (argc >= 4)
   {
      dlgAdm->Use3DControls = FALSE;
      if (IsYes(ar[3].strptr))
      {
#ifdef __CTL3D
        if (Ctl3dRegister(dlgAdm->TheInstance))
        {
           Ctl3dAutoSubclass(dlgAdm->TheInstance);
           dlgAdm->Use3DControls = TRUE;
        }
#endif
      }
   }
   return TRUE;
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

   /* add this message, so HandleMessages in DIALOG.CMD knows that finsihed shall be set */
   AddDialogMessage((char *) MSG_TERMINATE, aDlg->pMessageQueue);

   if (!aDlg->LeaveDialog) aDlg->LeaveDialog = 3;    /* signal to exit */

   /* the entry must be removed before the last message is sent so the GetMessage loop can quit */
   if (aDlg->TableEntry == StoredDialogs-1)
       DialogTab[aDlg->TableEntry] = NULL;
   else
   {
       DialogTab[aDlg->TableEntry] = DialogTab[StoredDialogs-1];  /* move last entry to deleted one */
       DialogTab[aDlg->TableEntry]->TableEntry = aDlg->TableEntry;
       DialogTab[StoredDialogs-1] = NULL;              /* and delete last entry */
   }
   StoredDialogs--;

   PostMessage(aDlg->TheDlg, WM_QUIT, 0, 0);      /* to exit GetMessage */

   if (aDlg->TheDlg) DestroyWindow(aDlg->TheDlg);      /* docu states "must not use EndDialog for non-modal dialogs" */
   if (aDlg->menu) DestroyMenu(aDlg->menu);

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
        hIconBig = (HICON)SetClassLong(aDlg->TheDlg, GCL_HICON, (LONG)aDlg->SysMenuIcon);
        if ( aDlg->TitleBarIcon )
            hIconSmall = (HICON)SetClassLong(aDlg->TheDlg, GCL_HICONSM, (LONG)aDlg->TitleBarIcon);

        if ( ! aDlg->SharedIcon )
        {
            DestroyIcon(hIconBig);
            if ( ! hIconSmall )
                hIconSmall = (HICON)GetClassLong(aDlg->TheDlg, GCL_HICONSM);
        }
        else
        {
            hIconSmall = NULL;
        }
    }

   if ((aDlg->TheInstance) && (aDlg->TheInstance != MyInstance)) FreeLibrary(aDlg->TheInstance);
   current = (DIALOGADMIN *)aDlg->previous;

   /* delete data, message and bitmap table of the dialog */
   if (aDlg->MsgTab)
   {
       for (i=0;i<aDlg->MT_size;i++)
       {
           if (aDlg->MsgTab[i].rexxProgram) LocalFree(aDlg->MsgTab[i].rexxProgram);
       }
       LocalFree(aDlg->MsgTab);
       aDlg->MT_size = 0;
   }
   if (aDlg->DataTab) LocalFree(aDlg->DataTab);
   aDlg->DT_size = 0;

   /* delete color brushs */
   if (aDlg->ColorTab)
   {
       for (i=0;i<aDlg->CT_size;i++)
           if (aDlg->ColorTab[i].ColorBrush) DeleteObject((HBRUSH)aDlg->ColorTab[i].ColorBrush);
       LocalFree(aDlg->ColorTab);
       aDlg->CT_size = 0;
   }

   /* delete bitmaps */
   if (aDlg->BmpTab)
   {
      for (i=0;i<aDlg->BT_size;i++)
         if ((aDlg->BmpTab[i].Loaded & 0x1011) == 1)           /* otherwise stretched bitmap files are not freed */
         {
            if (aDlg->BmpTab[i].bitmapID) LocalFree((void *)aDlg->BmpTab[i].bitmapID);
            if (aDlg->BmpTab[i].bmpFocusID) LocalFree((void *)aDlg->BmpTab[i].bmpFocusID);
            if (aDlg->BmpTab[i].bmpSelectID) LocalFree((void *)aDlg->BmpTab[i].bmpSelectID);
            if (aDlg->BmpTab[i].bmpDisableID) LocalFree((void *)aDlg->BmpTab[i].bmpDisableID);
         }
         else if (aDlg->BmpTab[i].Loaded == 0)
         {
            if (aDlg->BmpTab[i].bitmapID) DeleteObject((HBITMAP)aDlg->BmpTab[i].bitmapID);
            if (aDlg->BmpTab[i].bmpFocusID) DeleteObject((HBITMAP)aDlg->BmpTab[i].bmpFocusID);
            if (aDlg->BmpTab[i].bmpSelectID) DeleteObject((HBITMAP)aDlg->BmpTab[i].bmpSelectID);
            if (aDlg->BmpTab[i].bmpDisableID) DeleteObject((HBITMAP)aDlg->BmpTab[i].bmpDisableID);
         }

      LocalFree(aDlg->BmpTab);
      if (aDlg->ColorPalette) DeleteObject(aDlg->ColorPalette);
      aDlg->BT_size = 0;
   }

   /* delete the icon resource table */
   if (aDlg->IconTab)
   {
       for ( i = 0; i < aDlg->IT_size; i++ )
       {
           if ( aDlg->IconTab[i].fileName )
               LocalFree(aDlg->IconTab[i].fileName);
       }
       LocalFree(aDlg->IconTab);
       aDlg->IT_size = 0;
   }

   /* Unhook a hook if it is installed */
   if ( aDlg->hHook ) removeKBHook(aDlg);

       /* The message queue and the admin block are freed in HandleDialogAdmin called from HandleMessages */

   if (!StoredDialogs) topDlg = NULL; else topDlg = current;
   if (topDlg)
   {
       if (DialogInAdminTable(topDlg))
       {
           if (!IsWindowEnabled(topDlg->TheDlg))
               EnableWindow(topDlg->TheDlg, TRUE);
           if (wasFGW)
           {
               SetForegroundWindow(topDlg->TheDlg);
               topDlg->OnTheTop = TRUE;
           }
       }
       else topDlg = NULL;
   }
   LeaveCriticalSection(&crit_sec);
   if ( hIconSmall )
       DestroyIcon(hIconSmall);

   return ret;
}


/* create an asynchronous dialog and run asynchronous message loop */
DWORD WINAPI WindowLoopThread(LONG * arg)
{
   MSG msg;
   CHAR buffer[NR_BUFFER];
   DIALOGADMIN * Dlg;
   BOOL * release;
   ULONG ret;

   Dlg = (DIALOGADMIN*)arg[1];  /*  thread local admin pointer from StartDialog */
   Dlg->TheDlg = CreateDialogParam( Dlg->TheInstance, MAKEINTRESOURCE(atoi((CHAR *) arg[0])), 0, (DLGPROC) RexxDlgProc, Dlg->Use3DControls);  /* pass 3D flag to WM_INITDIALOG */
   Dlg->ChildDlg[0] = Dlg->TheDlg;

   release = (BOOL *)arg[3];  /* the Release flag is stored as the 4th argument */
   if (Dlg->TheDlg)
   {
      if (arg[2]) rxstrlcpy(buffer, * ((PRXSTRING) arg[2]));
      else strcpy(buffer, "0");

      if (IsYes(buffer))
         if (!DataAutodetection(Dlg))
         {
            Dlg->TheThread = NULL;
            return 0;
         };

      *release = TRUE;  /* Release wait in StartDialog  */
      do
      {
         if (GetMessage(&msg,NULL, 0,0)) {
           if (!IsDialogMessage (Dlg->TheDlg, &msg))
               DispatchMessage(&msg);
         }
      } while (Dlg && DialogInAdminTable(Dlg) && !Dlg->LeaveDialog);
   } else *release = TRUE;
   EnterCriticalSection(&crit_sec);
   if (DialogInAdminTable(Dlg))
   {
       ret = DelDialog(Dlg);
       Dlg->TheThread = NULL;
   }
   LeaveCriticalSection(&crit_sec);
   return ret;
}


ULONG APIENTRY GetDialogFactor(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
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




/* create an asynchronous dialog */
ULONG APIENTRY StartDialog(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   LONG argList[4];
   ULONG thID;
   BOOL Release = FALSE;
   DEF_ADM;

   CHECKARG(7);
   GET_ADM;
   if (!dlgAdm) RETERR;

   if ( ! ComCtl32Version && ! InitForCommonControls() )
       RETC(0)

   EnterCriticalSection(&crit_sec);
   if (!InstallNecessaryStuff(dlgAdm, &argv[1], argc-1))
   {
      if (dlgAdm)
      {
          DelDialog(dlgAdm);
          if (dlgAdm->pMessageQueue) LocalFree((void *)dlgAdm->pMessageQueue);
          LocalFree(dlgAdm);
      }
      LeaveCriticalSection(&crit_sec);
      RETC(0)
   }

   argList[0] = (LONG) argv[2].strptr;  /* dialog resource id */
   argList[1] = (LONG) dlgAdm;
   argList[2] = (LONG) &argv[3];  /* auto detection? */
   argList[3] = (LONG) &Release;  /* pass along pointer so that variable can be modified */

   dlgAdm->TheThread = CreateThread(NULL, 2000, WindowLoopThread, argList, 0, &thID);

   while ((!Release) && (dlgAdm->TheThread)) {Sleep(1);};  /* wait for dialog start */
   LeaveCriticalSection(&crit_sec);

   if (dlgAdm)
   {
       if (dlgAdm->TheDlg)
       {
          HICON hBig = NULL;
          HICON hSmall = NULL;

          SetThreadPriority(dlgAdm->TheThread, THREAD_PRIORITY_ABOVE_NORMAL);   /* for a faster drawing */
          dlgAdm->OnTheTop = TRUE;
          dlgAdm->threadID = thID;
                                  /* modal flag = yes ? */
          if (dlgAdm->previous && !IsYes(argv[6].strptr) && IsWindowEnabled(((DIALOGADMIN *)dlgAdm->previous)->TheDlg)) EnableWindow(((DIALOGADMIN *)dlgAdm->previous)->TheDlg, FALSE);

          if ( GetDialogIcons(dlgAdm, atoi(argv[5].strptr), FALSE, &hBig, &hSmall) )
          {
              dlgAdm->SysMenuIcon = (HICON)SetClassLong(dlgAdm->TheDlg, GCL_HICON, (LONG)hBig);
              dlgAdm->TitleBarIcon = (HICON)SetClassLong(dlgAdm->TheDlg, GCL_HICONSM, (LONG)hSmall);
              dlgAdm->DidChangeIcon = TRUE;

              SendMessage(dlgAdm->TheDlg, WM_SETICON, ICON_SMALL, (LPARAM)hSmall);
          }

          RETVAL((ULONG)dlgAdm->TheDlg)
       }
       dlgAdm->OnTheTop = FALSE;
       if (dlgAdm->previous) ((DIALOGADMIN *)(dlgAdm->previous))->OnTheTop = TRUE;
       if ((dlgAdm->previous) && !IsWindowEnabled(((DIALOGADMIN *)dlgAdm->previous)->TheDlg))
          EnableWindow(((DIALOGADMIN *)dlgAdm->previous)->TheDlg, TRUE);
   }
   RETC(0)
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
 * @param fromFile  Flag indicating whether the icon is located in a DLL or to
 *                  be loaded from a file.
 * @param phBig     In/Out Pointer to an icon handle.  If the function succeeds,
 *                  on return will contian the handle to a regular size icon.
 * @param phSmall   In/Out Pointer to an icon handle.  On success will contain
 *                  a handle to a small size icon.
 *
 * @return True if the icons were loaded and the returned handles are valid,
 *         otherwise false.
 */
BOOL GetDialogIcons(DIALOGADMIN *dlgAdm, INT id, BOOL fromFile, PHANDLE phBig, PHANDLE phSmall)
{
    int cx, cy;

    if ( phBig == NULL || phSmall == NULL )
        return FALSE;

    if ( id < 1 )
        id = IDI_DLG_DEFAULT;

    /* If one of the reserved IDs, fromFile has to be false. */
    if ( id <= IDI_DLG_MAX_ID )
        fromFile = FALSE;

    cx = GetSystemMetrics(SM_CXICON);
    cy = GetSystemMetrics(SM_CYICON);

    *phBig = GetIconForID(dlgAdm, id, fromFile, cx, cy);

    /* If that didn't get the big icon, try to get the default icon. */
    if ( ! *phBig && id != IDI_DLG_DEFAULT )
    {
        id = IDI_DLG_DEFAULT;
        fromFile = FALSE;
        *phBig = GetIconForID(dlgAdm, id, fromFile, cx, cy);
    }

    /* If still no big icon, don't bother trying for the small icon. */
    if ( *phBig )
    {
        cx = GetSystemMetrics(SM_CXSMICON);
        cy = GetSystemMetrics(SM_CYSMICON);
        *phSmall = GetIconForID(dlgAdm, id, fromFile, cx, cy);

        /* Very unlikely that the big icon was obtained and failed to get the
         * small icon.  But, if so, fail completely.  If the big icon came from
         * a DLL it was loaded as shared and the system handles freeing it.  If
         * it was loaded from a file, free it here.
         */
        if ( ! *phSmall )
        {
            if ( fromFile )
                DestroyIcon(*phBig);
            *phBig = NULL;
        }
    }

    if ( ! *phBig )
        return FALSE;

    dlgAdm->SharedIcon = !fromFile;
    return TRUE;
}


/**
 * Loads and returns the handle to an icon for the specified ID, of the
 * specified size.
 *
 * The icons can come from the user resource DLL, a user defined dialog, or the
 * OODialog DLL.  IDs for the icons bound to the OODialog.dll are reserved.
 *
 * @param dlgAdm    Pointer to the dialog administration block.
 * @param id        Numerical resource ID.
 * @param fromFile  Flag indicating whether the icon is located in a DLL or to
 *                  be loaded from a file.
 * @param cx        The desired width of the icon.
 * @param cy        The desired height of the icon.
 *
 * @return The handle to the loaded icon on success, or null on failure.
 */
HICON GetIconForID(DIALOGADMIN *dlgAdm, UINT id, BOOL fromFile, int cx, int cy)
{
    HINSTANCE hInst = NULL;
    LPCTSTR   pName = NULL;
    UINT      loadFlags = 0;

    if ( fromFile )
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
    else if ( id <= IDI_DLG_MAX_ID )
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

    return LoadImage(hInst, pName, IMAGE_ICON, cx, cy, loadFlags);
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
ULONG APIENTRY WinAPI32Func(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
    /* There has to be at least 2 args. */
    CHECKARGL(2);

    if ( argv[0].strptr[0] == 'G' )         /* Get something                  */
    {
        if ( !strcmp(argv[1].strptr, "WNDSTATE") )   /* get Window state      */
        {
            HWND hWnd;

            CHECKARGL(4);

            hWnd = (HWND)atol(argv[3].strptr);
            if ( hWnd == 0 || ! IsWindow(hWnd) ) RETERR

            if ( argv[2].strptr[0] == 'E' )          /* Enabled */
            {
                RETVAL((BOOL)IsWindowEnabled(hWnd));
            }
            else if ( argv[2].strptr[0] == 'V' )     /* Visible */
            {
                RETVAL((BOOL)IsWindowVisible(hWnd));
            }
            else if ( argv[2].strptr[0] == 'Z' )     /* Zoomed is Maximized */
            {
                RETVAL((BOOL)IsZoomed(hWnd));
            }
            else if ( argv[2].strptr[0] == 'I' )     /* Iconic is Minimized */
            {
                RETVAL((BOOL)IsIconic(hWnd));
            }
        }
        else if ( !strcmp(argv[1].strptr, "ID") )    /* get dialog control ID */
        {
            HWND hWnd;
            INT  id;

            CHECKARGL(3);

            hWnd = (HWND)atol(argv[2].strptr);
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

        dlgAdm = (DIALOGADMIN *)atol(argv[2].strptr);
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

            hDlg = (HWND)atol(argv[3].strptr);
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

                    hDlg = (HWND)atol(argv[3].strptr);
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
    RXSTRING method,
    RXSTRING keys,
    PCHAR filter )
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
            if ( pData->pMethods[i] ) LocalFree((void *)pData->pMethods[i]);
            if ( pData->pFilters[i] ) LocalFree((void *)pData->pFilters[i]);
        }
        LocalFree((void *)pData);
    }
}

/**
 * Searches the event method table for a matching method.  Returns 0 if no match
 * is found, otherwise the index into the table for the method.
 *
 */
UINT seekKeyPressMethod(KEYPRESSDATA *pData, PCHAR method)
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
LONG setKeyPressData(KEYPRESSDATA *pData, RXSTRING method, RXSTRING keys, PCHAR filter)
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

    pMethod = LocalAlloc(LPTR, method.strlength + 1);
    if ( ! pMethod ) return -5;

    if ( filter )
    {
        pFilter = LocalAlloc(LPTR, sizeof(KEYFILTER));
        if ( ! pFilter )
        {
            free(pMethod);
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
            free(pFilter);
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

LONG InternalStopDialog(HWND h)
{
   ULONG i, ret;
   DIALOGADMIN * dadm = NULL;
   MSG msg;
   HANDLE hTh;

   if (!h) dadm = topDlg;
   else SEEK_DLGADM_TABLE(h, dadm);

   if (dadm)
   {
       DWORD ec;
       hTh = dadm->TheThread;
       if (!dadm->LeaveDialog) dadm->LeaveDialog = 3;    /* signal to exit */
       PostMessage(dadm->TheDlg, WM_QUIT, 0, 0);      /* to exit GetMessage */
       DestroyWindow(dadm->TheDlg);      /* to remove system resources */
       if (dadm->TheThread) GetExitCodeThread(dadm->TheThread, &ec);
       i = 0; while (dadm && (ec == STILL_ACTIVE) && dadm->TheThread && (i < 1000))  /* wait for window thread of old dialog to terminate */
       {
           if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
               DispatchMessage(&msg);
           }
           Sleep(1); i++;
           GetExitCodeThread(dadm->TheThread, &ec);
       }

       if (dadm->TheThread && (ec == STILL_ACTIVE))
       {
           TerminateThread(dadm->TheThread, -1);
           ret = 1;
       } else ret = 0;
       if (hTh) CloseHandle(hTh);
       if (dadm == topDlg) topDlg = NULL;

       /* The message queue and the admin block are freed in HandleDialogAdmin called from HandleMessages */

       return ret;
   }
   return -1;
}

_declspec(dllexport) LONG __cdecl OODialogCleanup(BOOL Process)
{
    if (Process)
    {
        INT i;
        for (i = MAXDIALOGS; i>0; i--)
        {
            if (DialogTab[i-1]) InternalStopDialog(DialogTab[i-1]->TheDlg);
            DialogTab[i-1] = NULL;
        }
        StoredDialogs = 0;
    }
    return (StoredDialogs);
}


ULONG APIENTRY HandleDlg(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   DEF_ADM;

   CHECKARGL(1);

   if (!strcmp(argv[0].strptr, "ACTIVE"))   /* see if the dialog is still in the dialog management table */
   {
       CHECKARG(2);
       SEEK_DLGADM_TABLE((HWND)atol(argv[1].strptr), dlgAdm);

       if (dlgAdm)    /* it's alive */
           RETC(1)
       else
          RETC(0)
   }
   else
   if (!strcmp(argv[0].strptr, "HNDL"))   /* Get the dialog handle */
   {
       if (argc==2)
       {
           dlgAdm = (DIALOGADMIN*) atol(argv[1].strptr);
           if (!dlgAdm) RETERR
           RETVAL((ULONG)dlgAdm->TheDlg)
       }
       else {
           if (topDlg && topDlg->TheDlg)
               RETVAL((ULONG)topDlg->TheDlg)
           else
               RETC(0)
       }
   }
   else
   if (!strcmp(argv[0].strptr, "ITEM"))   /* Get the handle to a dialog item */
   {
       HWND hW, hD;

       CHECKARGL(2);

       if (argc > 2)
       {
          hD = (HWND) atol(argv[2].strptr);
       } else hD = topDlg->TheDlg;

       hW = GetDlgItem(hD, atoi(argv[1].strptr));

       if (hW)
          RETVAL((ULONG)hW)
       else
          RETC(0)
   }
   else
   if (!strcmp(argv[0].strptr, "STOP"))   /* Stop a dialog */
   {
       HWND h;

       if (argc>1)
          h=(HWND) atol(argv[1].strptr);
       else h=NULL;

       if (h)
       {
           SEEK_DLGADM_TABLE(h, dlgAdm);
           if (dlgAdm) RETVAL(DelDialog(dlgAdm))
           else RETVAL(-1)
       }
       else if (!h && topDlg)
           RETVAL(DelDialog(topDlg))        /* remove the top most */
       else
           RETVAL(-1)
   }
   RETC(0); /* DOE002A */

}






ULONG APIENTRY DialogMenu(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

{
   HWND hWnd;
   DEF_ADM;

   CHECKARGL(3);

   if (!strcmp(argv[0].strptr, "ASSOC"))   /* Associates a menu with a dialog */
   {
       hWnd = (HWND)atol(argv[1].strptr);

       if (hWnd)
       {
          SEEK_DLGADM_TABLE(hWnd, dlgAdm);
          if (dlgAdm)
          {
              dlgAdm->menu = LoadMenu(dlgAdm->TheInstance, MAKEINTRESOURCE(atoi(argv[2].strptr)));
              if (dlgAdm->menu)
              {
                  SetMenu(hWnd, dlgAdm->menu);
                  RETC(0)
              }
          }
       }
       RETC(1)
   }
   else
   {
       DWORD opt;
       DEF_ADM;
       dlgAdm = (DIALOGADMIN *)atol(argv[1].strptr);
       if (!dlgAdm) RETERR
       if (!dlgAdm->menu) RETC(1)

       if (!strcmp(argv[0].strptr, "SETMI"))   /* set state of menu item */
       {
           CHECKARGL(4);
           if (argc == 4)
           {
               if (!strcmp(argv[3].strptr, "ENABLE")) opt = MF_ENABLED; else
               if (!strcmp(argv[3].strptr, "DISABLE")) opt = MF_DISABLED; else
               if (!strcmp(argv[3].strptr, "GRAY")) opt = MF_GRAYED; else
               {
                   if (!strcmp(argv[3].strptr, "CHECK")) opt = MF_CHECKED; else
                   if (!strcmp(argv[3].strptr, "UNCHECK")) opt = MF_UNCHECKED; else RETC(1)
                   if (CheckMenuItem(dlgAdm->menu, atoi(argv[2].strptr), MF_BYCOMMAND | opt) == 0xFFFFFFFF)
                       RETC(1) else RETC(0)
               }
               if (EnableMenuItem(dlgAdm->menu, atoi(argv[2].strptr), MF_BYCOMMAND | opt) == 0xFFFFFFFF)
                  RETC(1) else RETC(0)
           }
           else
           if (argc == 5)
           {
                                                     /* start of group       end                selected item */
               if (!CheckMenuRadioItem(dlgAdm->menu, atoi(argv[2].strptr), atoi(argv[3].strptr), atoi(argv[4].strptr), MF_BYCOMMAND))
                  RETC(1) else RETC(0)

           } else RETC(1)
       }
       else
       if (!strcmp(argv[0].strptr, "GETMI"))    /* get state of menu item */
       {
           UINT state;
           retstr->strptr[0] = '\0';
           state = GetMenuState(dlgAdm->menu, atoi(argv[2].strptr), MF_BYCOMMAND);
           if (state == 0xFFFFFFFF) RETC(1);
           if (state & MF_CHECKED) strcat(retstr->strptr, "CHECKED ");
           if (state & MF_DISABLED) strcat(retstr->strptr, "DISABLED ");
           if (state & MF_GRAYED) strcat(retstr->strptr, "GRAYED ");
           if (state & MF_HILITE) strcat(retstr->strptr, "HIGHLIGHTED ");
           retstr->strlength = strlen(retstr->strptr);
           return 0;
       }
       RETERR
   }
}



/* dump out the dialog admin table(s) */

LONG SetRexxStem(CHAR * name, INT id, char * secname, CHAR * data)
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
   shvb.shvvalue.strptr = data;
   shvb.shvvalue.strlength = strlen(data);
   shvb.shvvaluelen = strlen(data);
   shvb.shvcode = RXSHV_SYSET;
   shvb.shvret = 0;
   if (RexxVariablePool(&shvb) == RXSHV_BADN) {
       sprintf(data, "Variable %s could not be declared", buffer);
       MessageBox(0,data,"Error",MB_OK | MB_ICONHAND);
       return FALSE;
   }
   return TRUE;
}




ULONG APIENTRY DumpAdmin(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

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
       dlgAdm = (DIALOGADMIN *)atol(argv[1].strptr);
       if (!dlgAdm) RETVAL(-1)

       strcpy(name, argv[0].strptr); /* stem name */
       itoa(dlgAdm->TableEntry, data, 10);
       if (!SetRexxStem(name, -1, "Slot", data)) RETERR
       itoa((LONG)dlgAdm->TheThread, data, 16);
       if (!SetRexxStem(name, -1, "hThread", data)) RETERR
       itoa((LONG)dlgAdm->TheDlg, data, 16);
       if (!SetRexxStem(name, -1, "hDialog", data)) RETERR
       itoa((LONG)dlgAdm->menu, data, 16);
       if (!SetRexxStem(name, -1, "hMenu", data)) RETERR
       itoa((LONG)dlgAdm->BkgBrush, data, 16);
       if (!SetRexxStem(name, -1, "BkgBrush", data)) RETERR
       itoa((LONG)dlgAdm->BkgBitmap, data, 16);
       if (!SetRexxStem(name, -1, "BkgBitmap", data)) RETERR
       itoa(dlgAdm->OnTheTop, data, 10);
       if (!SetRexxStem(name, -1, "TopMost", data)) RETERR
       itoa((LONG)dlgAdm->AktChild, data, 16);
       if (!SetRexxStem(name, -1, "CurrentChild", data)) RETERR
       itoa((LONG)dlgAdm->TheInstance, data, 16);
       if (!SetRexxStem(name, -1, "DLL", data)) RETERR
       if (!SetRexxStem(name, -1, "Queue", dlgAdm->pMessageQueue)) RETERR
       itoa(dlgAdm->BT_size, data, 10);
       if (!SetRexxStem(name, -1, "BmpButtons", data)) RETERR
       sprintf(buffer, "%s.%s", argv[0].strptr, "BmpTab");
       for (i=0; i<dlgAdm->BT_size; i++)
       {
           itoa(dlgAdm->BmpTab[i].buttonID, data, (dlgAdm->BmpTab[i].Loaded ? 16: 10));
           if (!SetRexxStem(buffer, i+1, "ID", data)) RETERR
           itoa((LONG)dlgAdm->BmpTab[i].bitmapID, data, (dlgAdm->BmpTab[i].Loaded ? 16: 10));
           if (!SetRexxStem(buffer, i+1, "Normal", data)) RETERR
           itoa((LONG)dlgAdm->BmpTab[i].bmpFocusID, data, (dlgAdm->BmpTab[i].Loaded ? 16: 10));
           if (!SetRexxStem(buffer, i+1, "Focused", data)) RETERR
           itoa((LONG)dlgAdm->BmpTab[i].bmpSelectID, data, (dlgAdm->BmpTab[i].Loaded ? 16: 10));
           if (!SetRexxStem(buffer, i+1, "Selected", data)) RETERR
           itoa((LONG)dlgAdm->BmpTab[i].bmpDisableID, data, (dlgAdm->BmpTab[i].Loaded ? 16: 10));
           if (!SetRexxStem(buffer, i+1, "Disabled", data)) RETERR
       }
       itoa(dlgAdm->MT_size, data, 10);
       if (!SetRexxStem(name, -1, "Messages", data)) RETERR
       sprintf(buffer, "%s.%s", argv[0].strptr, "MsgTab");
       for (i=0; i<dlgAdm->MT_size; i++)
       {
           itoa((LONG)dlgAdm->MsgTab[i].msg, data, 16);
           if (!SetRexxStem(buffer, i+1, "msg", data)) RETERR
           itoa((LONG)dlgAdm->MsgTab[i].wParam, data, 16);
           if (!SetRexxStem(buffer, i+1, "param1", data)) RETERR
           itoa((LONG)dlgAdm->MsgTab[i].lParam, data, 16);
           if (!SetRexxStem(buffer, i+1, "param2", data)) RETERR
           strcpy(data, dlgAdm->MsgTab[i].rexxProgram);
           if (!SetRexxStem(buffer, i+1, "method", data)) RETERR
       }
       itoa(dlgAdm->DT_size, data, 10);
       if (!SetRexxStem(name, -1, "DataItems", data)) RETERR
       sprintf(buffer, "%s.%s", argv[0].strptr, "DataTab");
       for (i=0; i<dlgAdm->DT_size; i++)
       {
           itoa(dlgAdm->DataTab[i].id, data, 10);
           if (!SetRexxStem(buffer, i+1, "ID", data)) RETERR
           itoa(dlgAdm->DataTab[i].typ, data, 10);
           if (!SetRexxStem(buffer, i+1, "type", data)) RETERR
           itoa(dlgAdm->DataTab[i].category, data, 10);
           if (!SetRexxStem(buffer, i+1, "category", data)) RETERR
       }
       itoa(dlgAdm->CT_size, data, 10);
       if (!SetRexxStem(name, -1, "ColorItems", data)) RETERR
       sprintf(buffer, "%s.%s", argv[0].strptr, "ColorTab");
       for (i=0; i<dlgAdm->CT_size; i++)
       {
           itoa(dlgAdm->ColorTab[i].itemID, data, 10);
           if (!SetRexxStem(buffer, i+1, "ID", data)) RETERR
           itoa(dlgAdm->ColorTab[i].ColorBk, data, 10);
           if (!SetRexxStem(buffer, i+1, "Background", data)) RETERR
           itoa(dlgAdm->ColorTab[i].ColorFG, data, 10);
           if (!SetRexxStem(buffer, i+1, "Foreground", data)) RETERR
       }
   }

   if (argc == 1)
   {
       for (i=0; i<MAXDIALOGS; i++)
       {
           if (DialogTab[i] != NULL)
           {
               cnt++;
               itoa((LONG)DialogTab[i], data, 10);
               if (!SetRexxStem(name, cnt, "AdmBlock", data)) RETERR
               itoa(DialogTab[i]->TableEntry, data, 10);
               if (!SetRexxStem(name, cnt, "Slot", data)) RETERR
               itoa((LONG)DialogTab[i]->TheThread, data, 16);
               if (!SetRexxStem(name, cnt, "hThread", data)) RETERR
               itoa((LONG)DialogTab[i]->TheDlg, data, 16);
               if (!SetRexxStem(name, cnt, "hDialog", data)) RETERR
               itoa((LONG)DialogTab[i]->menu, data, 16);
               if (!SetRexxStem(name, cnt, "hMenu", data)) RETERR
               itoa((LONG)DialogTab[i]->BkgBrush, data, 16);
               if (!SetRexxStem(name, cnt, "BkgBrush", data)) RETERR
               itoa((LONG)DialogTab[i]->BkgBitmap, data, 16);
               if (!SetRexxStem(name, cnt, "BkgBitmap", data)) RETERR
               itoa(DialogTab[i]->OnTheTop, data, 10);
               if (!SetRexxStem(name, cnt, "TopMost", data)) RETERR
               itoa((LONG)DialogTab[i]->AktChild, data, 16);
               if (!SetRexxStem(name, cnt, "CurrentChild", data)) RETERR
               itoa((LONG)DialogTab[i]->TheInstance, data, 16);
               if (!SetRexxStem(name, cnt, "DLL", data)) RETERR
               if (!SetRexxStem(name, cnt, "Queue", DialogTab[i]->pMessageQueue)) RETERR
               itoa(DialogTab[i]->BT_size, data, 10);
               if (!SetRexxStem(name, cnt, "BmpButtons", data)) RETERR
               itoa(DialogTab[i]->MT_size, data, 10);
               if (!SetRexxStem(name, cnt, "Messages", data)) RETERR
               itoa(DialogTab[i]->DT_size, data, 10);
               if (!SetRexxStem(name, cnt, "DataItems", data)) RETERR
               itoa(DialogTab[i]->CT_size, data, 10);
               if (!SetRexxStem(name, cnt, "ColorItems", data)) RETERR
           }
       }
       itoa(cnt, data, 10);
       if (!SetRexxStem(name, 0, NULL, data)) RETERR  /* Set number of dialog tables */
   }
   RETC(0)
}


/****************************************************************************************************

           Part for REXXAPI

****************************************************************************************************/

#define FTS 31
CHAR * FuncTab[FTS] = {\
                     "GetDlgMsg", \
                     "SendWinMsg", \
                     "HandleDlg",\
                     "AddUserMessage", \
                     "GetFileNameWindow",\
                     "DataTable", \
                     "HandleDialogAdmin", \
                     "SetItemData",\
                     "SetStemData",\
                     "GetItemData",    \
                     "GetStemData",    \
                     "Wnd_Desktop", \
                     "WndShow_Pos", \
                     "WinAPI32Func", \
                     "InfoMessage", \
                     "ErrorMessage", \
                     "YesNoMessage", \
                     "FindTheWindow", \
                     "StartDialog",\
                     "WindowRect", \
                     "GetStdTextSize", \
                     "SetLBTabStops", \
                     "BinaryAnd", \
                     "GetScreenSize", \
                     "GetSysMetrics", \
                     "GetDialogFactor", \
                     "SleepMS", \
                     "PlaySoundFile", \
                     "PlaySoundFileInLoop",\
                     "StopSoundFile",\
                     "HandleScrollBar" \
                     };

#define SFTS 19
CHAR * SpecialFuncTab[SFTS] = {\
                     "BmpButton", \
                     "DCDraw", \
                     "DrawGetSet", \
                     "DrawTheBitmap", \
                     "ScrollText", \
                     "ScrollTheWindow", \
                     "HandleDC_Obj", \
                     "SetBackground", \
                     "LoadRemoveBitmap", \
                     "WriteText", \
                     "HandleTreeCtrl", \
                     "HandleListCtrl", \
                     "HandleListCtrlEx", \
                     "HandleControlEx", \
                     "HandleOtherNewCtrls", \
                     "DialogMenu", \
                     "WinTimer", \
                     "HandleFont", \
                     "DumpAdmin" \
                     };


#define UFTS 6
CHAR * UserFuncTab[UFTS] = {\
                     "UsrAddControl", \
                     "UsrCreateDialog", \
                     "UsrDefineDialog", \
                     "UsrMenu", \
                     "UsrAddNewCtrl", \
                     "UsrAddResource", \
                     };


ULONG APIENTRY RemoveMMFuncs(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

{
   INT rc, i;
   BOOL err = FALSE;

   /* don't remove if there's still an dialog active */
   if (StoredDialogs) RETC(1)

   for (i=0;i<FTS;i++)
   {
      rc = RexxDeregisterFunction(FuncTab[i]);
      if (rc) err = TRUE;
   }
   rc = RexxDeregisterFunction("RemoveMMFuncs");
   if (rc) err = TRUE;
   rc = RexxDeregisterFunction("InstMMFuncs");
   if (rc) err = TRUE;

   if (!err)
      RETC(0)
   else
      RETC(1)

}

/**
 * Convenience function to put up an error message box.
 *
 * @param pszMsg    The message.
 * @param pszTitle  The title of for the message box.
 */
static void internalErrorMsg(PSZ pszMsg, PSZ pszTitle)
{
    MessageBox(0, pszMsg, pszTitle, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
}

/**
 * Determines the version of comctl32.dll and initializes the common controls.
 *
 * The minimum version of 4.71 is supported on Windows 95 with Internet Explorer
 * 4.0, Windows NT 4.0 with Internet Explorer 4.0, Windows 98, and Windows 2000.
 *
 * @return TRUE if comctl32.dll is at least version 4.71, otherwise FALSE.
 */
BOOL InitForCommonControls(void)
{
    HINSTANCE hinst;
    BOOL      success = FALSE;

    hinst = LoadLibrary(TEXT("comctl32.dll"));
    if ( hinst )
    {
        DLLGETVERSIONPROC pDllGetVersion;

        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinst, "DllGetVersion");
        if ( pDllGetVersion )
        {
            DLLVERSIONINFO info;

            ZeroMemory(&info, sizeof(info));
            info.cbSize = sizeof(info);
            if ( SUCCEEDED((*pDllGetVersion)(&info)) )
                ComCtl32Version = MAKEVERSION(info.dwMajorVersion, info.dwMinorVersion);
        }
        FreeLibrary(hinst);
    }

    if ( ComCtl32Version == 0 )
    {
        internalErrorMsg("The version of the Windows Common Controls library (comctl32.dll)\n"
                         "could not be determined.  ooDialog will not run", COMCTL_ERR_TITLE);
    }
    else if ( ComCtl32Version < COMCTL32_4_71 )
    {
        CHAR msg[256];
        sprintf(msg, "ooDialog can not run with this version of the Windows Common Controls library\n"
                "(comctl32.dll.)  The minimum version required is 4.71.\n\nThis system has version: %s\n",
                ComCtl32Version == COMCTL32_4_0 ? "4.0" : "4.7" );

        internalErrorMsg(msg, COMCTL_ERR_TITLE);
        ComCtl32Version = 0;
    }
    else
    {
        INITCOMMONCONTROLSEX ctrlex;

        ctrlex.dwSize = sizeof(ctrlex);
        ctrlex.dwICC = ICC_WIN95_CLASSES;
        if ( ! InitCommonControlsEx(&ctrlex) )
        {
            CHAR msg[128];
            sprintf(msg, "Initializing the Windows Common Controls library (InitCommonControlsEx)\n"
                    "failed.  Windows System Error Code: %d\n", GetLastError());
            internalErrorMsg(msg, COMCTL_ERR_TITLE);
            ComCtl32Version = 0;
        }
        else
        {
            success = TRUE;
        }
    }
    return success;
}


ULONG APIENTRY InstMMFuncs(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

{
   INT rc, i;
   BOOL err = FALSE;
   retstr->strlength = 1;

   /* If the common controls are not initialized, don't load. */
   if ( ! ComCtl32Version && ! InitForCommonControls() )
      RETC(1)

   rc = RexxRegisterFunctionDll(
     "RemoveMMFuncs",
     VISDLL,
     "RemoveMMFuncs");
   if ((rc != RXFUNC_OK) && (rc != RXFUNC_DEFINED)) err = TRUE;

   for (i=0;i<FTS;i++)
   {
      rc = RexxRegisterFunctionDll(FuncTab[i],VISDLL,FuncTab[i]);
      if ((rc != RXFUNC_OK) && (rc != RXFUNC_DEFINED)) err = TRUE;
   }

   if (err)
      RETC(1)       /* not ok then return 1 so that old CLS files won't run  */

   RETVAL(DLLVER)   /* ok, so we return the DLL version */
}



ULONG APIENTRY RemoveExtendedMMFuncs(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

{
   INT rc, i;
   BOOL err = FALSE;

   if (StoredDialogs) RETC(1)

   for (i=0;i<SFTS;i++)
   {
      rc = RexxDeregisterFunction(SpecialFuncTab[i]);
      if (rc) err = TRUE;
   }
   rc = RexxDeregisterFunction("RemoveExtendedMMFuncs");
   if (rc) err = TRUE;
   rc = RexxDeregisterFunction("InstExtendedMMFuncs");
   if (rc) err = TRUE;

   if (!err)
      RETC(0)    /* OK */
   else
      RETC(1)
}




ULONG APIENTRY InstExtendedMMFuncs(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

{
   INT rc, i;
   BOOL err = FALSE;
   retstr->strlength = 1;

   /* If the common controls are not initialized, don't load. */
   if ( ! ComCtl32Version && ! InitForCommonControls() )
      RETC(1)

   rc = RexxRegisterFunctionDll(
     "RemoveExtendedMMFuncs",
     VISDLL,
     "RemoveExtendedMMFuncs");

   if ((rc != RXFUNC_OK) && (rc != RXFUNC_DEFINED)) err = TRUE;

   for (i=0;i<SFTS;i++)
   {
      rc = RexxRegisterFunctionDll(SpecialFuncTab[i],VISDLL,SpecialFuncTab[i]);
      if ((rc != RXFUNC_OK) && (rc != RXFUNC_DEFINED)) err = TRUE;
   }

   if (err)
      RETC(1)
   else
      RETC(0)    /* OK */
}



ULONG APIENTRY RemoveUserMMFuncs(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

{
   INT rc, i;
   BOOL err = FALSE;

   /* don't remove if there's still an dialog active */
   if (StoredDialogs) RETC(1)

   for (i=0;i<UFTS;i++)
   {
      rc = RexxDeregisterFunction(UserFuncTab[i]);
      if (rc) err = TRUE;
   }
   rc = RexxDeregisterFunction("RemoveUserMMFuncs");
   if (rc) err = TRUE;
   rc = RexxDeregisterFunction("InstUserMMFuncs");
   if (rc) err = TRUE;

   if (!err)
      RETC(0)
   else
      RETC(1)

}



ULONG APIENTRY InstUserMMFuncs(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )

{
   INT rc, i;
   BOOL err = FALSE;
   retstr->strlength = 1;

   /* If the common controls are not initialized, don't load. */
   if ( ! ComCtl32Version && ! InitForCommonControls() )
      RETC(1)

   rc = RexxRegisterFunctionDll(
     "RemoveUserMMFuncs",
     VISDLL,
     "RemoveUserMMFuncs");

   if ((rc != RXFUNC_OK) && (rc != RXFUNC_DEFINED)) err = TRUE;

   for (i=0;i<UFTS;i++)
   {
      rc = RexxRegisterFunctionDll(UserFuncTab[i],VISDLL,UserFuncTab[i]);
      if ((rc != RXFUNC_OK) && (rc != RXFUNC_DEFINED)) err = TRUE;
   }

   if (err)
      RETC(1)
   else
      RETC(0)
}


#ifdef __cplusplus
extern "C" {
#endif

BOOL APIENTRY DllMain(
    HINSTANCE  hinstDLL,    // handle of DLL module
    DWORD  fdwReason,    // reason for calling function
    LPVOID  lpvReserved     // reserved
   )
{
   OSVERSIONINFO version_info={0}; /* for optimization so that GetVersionEx */

   if (fdwReason == DLL_PROCESS_ATTACH) {
      MyInstance = hinstDLL;
      version_info.dwOSVersionInfoSize = sizeof(version_info);  // if not set --> violation error
      GetVersionEx(&version_info);
      if (version_info.dwPlatformId == VER_PLATFORM_WIN32_NT) IsNT = TRUE; else IsNT = FALSE;
      InitializeCriticalSection(&crit_sec);
   }  else if (fdwReason == DLL_PROCESS_DETACH)
   {
       MyInstance = NULL;
       DeleteCriticalSection(&crit_sec);
   }
   return(TRUE);
}

#ifdef __cplusplus
}
#endif
