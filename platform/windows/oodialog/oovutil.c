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
#include <windows.h>
#include <mmsystem.h>
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


extern HINSTANCE MyInstance = NULL;

extern DIALOGADMIN * DialogTab[MAXDIALOGS] = {NULL};
extern DIALOGADMIN * topDlg = {NULL};
extern INT StoredDialogs = 0;
extern CRITICAL_SECTION crit_sec = {0};
extern WPARAM InterruptScroll;

extern BOOL SearchMessageTable(ULONG message, WPARAM param, LPARAM lparam, DIALOGADMIN * addressedTo);
extern BOOL DrawBitmapButton(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam, BOOL MsgEnabled);
extern BOOL DrawBackgroundBmp(DIALOGADMIN * addr, HWND hDlg, WPARAM wParam, LPARAM lParam);
extern BOOL DataAutodetection(DIALOGADMIN * aDlg);
extern LRESULT PaletteMessage(DIALOGADMIN * addr, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern BOOL AddDialogMessage(CHAR * msg, CHAR * Qptr);
extern BOOL IsNT = TRUE;
extern LONG HandleError(PRXSTRING r, CHAR * text);
extern LONG SetRexxStem(CHAR * name, INT id, char * secname, CHAR * data);

INT DelDialog(DIALOGADMIN * aDlg);

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

             case CREATECHILD:
                hW = CreateDialogIndirectParam(MyInstance, (DLGTEMPLATE *) lParam, (HWND) wParam, RexxDlgProc, addressedTo->Use3DControls); /* pass 3D flag to WM_INITDIALOG */
                ReplyMessage((LRESULT) hW);
                return (LRESULT) hW;
             case INTERRUPTSCROLL:
                addressedTo->StopScroll = wParam;
                return (TRUE);
             case GETSETFOCUS:
                if (!lParam) ReplyMessage((LRESULT)GetFocus());
                else ReplyMessage((LRESULT)SetFocus((HWND)lParam));
                return (TRUE);
             case GETSETCAPTURE:
                if (!wParam) ReplyMessage((LRESULT)GetCapture());
                else if (wParam == 2) ReplyMessage((LRESULT)ReleaseCapture());
                else ReplyMessage((LRESULT)SetCapture((HWND)lParam));
                return (TRUE);
             case GETKEYSTATE:
                ReplyMessage((LRESULT)GetAsyncKeyState(wParam));
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
         MessageBox(0,"Couldn't load DLL","Error",MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
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
   DeleteObject((HICON) SetClassLong(aDlg->TheDlg, GCL_HICON, (LONG)aDlg->SysMenuIcon));
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
   HINSTANCE hI;
   ULONG thID;
   BOOL Release = FALSE;
   DEF_ADM;

   CHECKARG(7);
   GET_ADM;
   if (!dlgAdm) RETERR;

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
          SetThreadPriority(dlgAdm->TheThread, THREAD_PRIORITY_ABOVE_NORMAL);   /* for a faster drawing */
          dlgAdm->OnTheTop = TRUE;
                                  /* modal flag = yes ? */
          if (dlgAdm->previous && !IsYes(argv[6].strptr) && IsWindowEnabled(((DIALOGADMIN *)dlgAdm->previous)->TheDlg)) EnableWindow(((DIALOGADMIN *)dlgAdm->previous)->TheDlg, FALSE);

          if (atoi(argv[5].strptr) > 0)
          {
              dlgAdm->SysMenuIcon = (HICON)SetClassLong(dlgAdm->TheDlg, GCL_HICON, (LONG)LoadIcon(dlgAdm->TheInstance, MAKEINTRESOURCE(atoi(argv[5].strptr))));
          }
          else
          {
              hI = LoadLibrary(VISDLL);
              dlgAdm->SysMenuIcon = (HICON)SetClassLong(dlgAdm->TheDlg, GCL_HICON, (LONG)LoadIcon(hI, MAKEINTRESOURCE(99)));
              FreeLibrary(hI);
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
           if (topDlg->TheDlg)
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

#define FTS 29
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
                     "GetDialogFactor", \
                     "SleepMS", \
                     "PlaySoundFile", \
                     "PlaySoundFileInLoop",\
                     "StopSoundFile",\
                     "HandleScrollBar" \
                     };

#define SFTS 17
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
                     "HandleOtherNewCtrls", \
                     "DialogMenu", \
                     "WinTimer", \
                     "HandleFont", \
                     "DumpAdmin" \
                     };


#define UFTS 5
CHAR * UserFuncTab[UFTS] = {\
                     "UsrAddControl", \
                     "UsrCreateDialog", \
                     "UsrDefineDialog", \
                     "UsrMenu", \
                     "UsrAddNewCtrl", \
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
      RETC(1)          /* not ok then return 1 so that old CLS files won't run  */
   else
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
