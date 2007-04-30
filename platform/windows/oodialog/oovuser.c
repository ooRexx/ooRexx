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
#define INCL_REXXSAA
#include <rexx.h>
#include <stdio.h>
#include <dlgs.h>
#ifdef __CTL3D
#include <ctl3d.h>
#endif
#include "oovutil.h"
#include <commctrl.h>

extern LRESULT CALLBACK RexxDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
extern BOOL InstallNecessaryStuff(DIALOGADMIN * dlgAdm, RXSTRING ar[], INT argc);
extern BOOL DataAutodetection(DIALOGADMIN * aDlg);
extern INT DelDialog(DIALOGADMIN * aDlg);
extern CRITICAL_SECTION crit_sec;
extern BOOL DialogInAdminTable(DIALOGADMIN * Dlg);


static BOOL CommCtrlLoaded = FALSE;
//#define USE_DS_CONTROL


#ifndef USE_DS_CONTROL
BOOL IsNestedDialogMessage(
    DIALOGADMIN * dlgAdm,
    LPMSG  lpmsg    // address of structure with message
   );
#endif

/****************************************************************************************************

           Part for user defined Dialogs

****************************************************************************************************/


LPWORD lpwAlign ( LPWORD lpIn)
{
  ULONG ul;

  ul = (ULONG) lpIn;
  ul +=3;
  ul >>=2;
  ul <<=2;
  return (LPWORD) ul;
}


int nCopyAnsiToWideChar (LPWORD lpWCStr, LPSTR lpAnsiIn)
{
  int nChar = 0;

  do {
    *lpWCStr++ = (WORD) (UCHAR) *lpAnsiIn;  /* first convert to UCHAR, otherwise Ü,Ä,... are >65000 */
    nChar++;
  } while (*lpAnsiIn++);

  return nChar;
}




ULONG APIENTRY GetStdTextSize(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   HDC hDC;
   SIZE s;
   ULONG bux;
   ULONG buy;
   PCHAR fn;
   INT fsb;
   HFONT hFont=NULL, hSystemFont, oldF;
   HWND hW = NULL;

   CHECKARGL(1);

   if ((argc == 1) && topDlg) hW = topDlg->TheDlg;
   if (argc >1) fn = argv[1].strptr; else fn = "System";
   if (argc >2) fsb = atoi(argv[2].strptr); else fsb = 8;
   if (argc >3) hW = (HWND) atol(argv[3].strptr);

   if (hW)
      hDC = GetDC(hW);
   else
      hDC = CreateDC("Display", NULL, NULL, NULL);
   if (hDC)
   {
      hSystemFont = GetStockObject(SYSTEM_FONT);

      if (hW)
         hFont = (HFONT)SendMessage(hW, WM_GETFONT, 0, 0);

      if (hW && !hFont)
         hFont = hSystemFont;
      else
      {
         /* we have no dialog, this is the cas if standard dialogs are called  */
         /* directly from a REXX script */
         /* so if we have a system font, use it ! If not, use an estimated one */
         if (hSystemFont)
            hFont = hSystemFont;
         else
            hFont = CreateFont(fsb, fsb, 0, 0, FW_NORMAL,FALSE, FALSE, FALSE, ANSI_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, fn);
      }

      if (hFont && (hFont != hSystemFont)) oldF = SelectObject(hDC, hFont);

      GetTextExtentPoint32(hDC, argv[0].strptr, argv[0].strlength, &s);

      buy = GetDialogBaseUnits();
      bux = LOWORD(buy);
      buy = HIWORD(buy);
      sprintf(retstr->strptr, "%d %d", (s.cx * 4) / bux, (s.cy * 8) / buy);
      retstr->strlength = strlen(retstr->strptr);

      if (hFont && (hFont != hSystemFont))
      {
         SelectObject(hDC, oldF);
         DeleteObject(hFont);
      }

      if (hW)
         ReleaseDC(hW, hDC);
      else
         DeleteDC(hDC);
      return 0;
   }
   RETC(0)
}


ULONG APIENTRY GetScreenSize(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   ULONG sx, sy;
   ULONG bux, buy;

   buy = GetDialogBaseUnits();
   bux = LOWORD(buy);
   buy = HIWORD(buy);
   sx = GetSystemMetrics(SM_CXSCREEN);
   sy = GetSystemMetrics(SM_CYSCREEN);

   sprintf(retstr->strptr, "%d %d %d %d", (sx * 4) / bux, (sy * 8) / buy, sx, sy);
   retstr->strlength = strlen(retstr->strptr);
   return 0;
}


void UCreateDlg(WORD ** template, WORD **p, INT NrItems, INT x, INT y, INT cx, INT cy,
                CHAR * class, CHAR * title, CHAR * fontname, INT fontsize, ULONG lStyle)
{
   int   nchar;

   *template = *p = (PWORD) LocalAlloc(LPTR, (NrItems+3)*256);

     /* start to fill in the dlgtemplate information.  addressing by WORDs */
   **p = LOWORD (lStyle);
   (*p)++;
   **p = HIWORD (lStyle);
   (*p)++;
   **p = 0;          // LOWORD (lExtendedStyle)
   (*p)++;
   **p = 0;          // HIWORD (lExtendedStyle)
   (*p)++;
   **p = NrItems;          // NumberOfItems
   (*p)++;
   **p = x;         // x
   (*p)++;
   **p = y;         // y
   (*p)++;
   **p = cx;        // cx
   (*p)++;
   **p = cy;         // cy
   (*p)++;
   /* copy the menu of the dialog */

   /* no menu */
   **p = 0;
   (*p)++;

   /* copy the class of the dialog */
   if ( !(lStyle & WS_CHILD) && (class))
   {
      nchar = nCopyAnsiToWideChar (*p, TEXT(class));
      (*p) += nchar;
   }
   else
   {
     **p = 0;
    (*p)++;
   }
   /* copy the title of the dialog */
   if (title)
   {
      nchar = nCopyAnsiToWideChar (*p, TEXT(title));
      (*p) += nchar;
   }
   else
   {
     **p = 0;
    (*p)++;
   }

   /* add in the wPointSize and szFontName here iff the DS_SETFONT bit on */
   **p = fontsize;         // fontsize
   (*p)++;
   nchar = nCopyAnsiToWideChar (*p, TEXT(fontname));
   (*p) += nchar;

   /* make sure the first item starts on a DWORD boundary */
   (*p) = lpwAlign (*p);
}



ULONG APIENTRY UsrDefineDialog(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{

   INT buffer[5];
   PCHAR opts;

   BOOL child;
   ULONG lStyle;
   WORD *p;
   WORD *pbase;
   int i;

   CHECKARG(10);

   for (i=0;i<4;i++) buffer[i] = atoi(argv[i].strptr);
   opts = argv[8].strptr;

   if (strstr(opts, "CHILD")) child = TRUE; else child = FALSE;
   buffer[4] = atoi(argv[9].strptr);

   if (child) lStyle = DS_SETFONT | WS_CHILD;
   else lStyle = WS_CAPTION | DS_SETFONT;

#ifdef USE_DS_CONTROL
   if (child) lStyle |= DS_CONTROL;
#endif

   if (strstr(opts, "VISIBLE")) lStyle |= WS_VISIBLE;
   if (!strstr(opts, "NOMENU") && !child) lStyle |= WS_SYSMENU;
   if (!strstr(opts, "NOTMODAL") && !child) lStyle |= DS_MODALFRAME;
   if (strstr(opts, "SYSTEMMODAL")) lStyle |= DS_SYSMODAL;
   if (strstr(opts, "THICKFRAME")) lStyle |= WS_THICKFRAME;
   if (strstr(opts, "MINIMIZEBOX")) lStyle |= WS_MINIMIZEBOX;
   if (strstr(opts, "MAXIMIZEBOX")) lStyle |= WS_MAXIMIZEBOX;
   if (strstr(opts, "VSCROLL")) lStyle |= WS_VSCROLL;
   if (strstr(opts, "HSCROLL")) lStyle |= WS_HSCROLL;

   if (strstr(opts, "OVERLAPPED")) lStyle |= WS_OVERLAPPED;

   /*                     expected        x          y        cx        cy  */
   UCreateDlg(&pbase, &p, buffer[4], buffer[0], buffer[1], buffer[2], buffer[3],
   /*            class         title            fontname         fontsize */
              argv[4].strptr, argv[5].strptr, argv[6].strptr, atoi(argv[7].strptr), lStyle);
   sprintf(retstr->strptr, "%ld %ld", pbase, p);
   retstr->strlength = strlen(retstr->strptr);
   return 0;
}

     /* Loop getting messages and dispatching them. */
DWORD WINAPI WindowUsrLoopThread(LONG * arg)
{
   MSG msg;
   CHAR buffer[NR_BUFFER];
   DIALOGADMIN * Dlg;
   ULONG ret=0;
   BOOL * release;

   Dlg = (DIALOGADMIN*)arg[1];  /*  thread local admin pointer from StartDialog */
   Dlg->TheDlg = CreateDialogIndirectParam(MyInstance, (DLGTEMPLATE *) arg[0], NULL, RexxDlgProc, Dlg->Use3DControls);  /* pass 3D flag to WM_INITDIALOG */
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

      *release = TRUE;
      while (GetMessage(&msg,NULL, 0,0) && DialogInAdminTable(Dlg) && (!Dlg->LeaveDialog)) {
#ifdef USE_DS_CONTROL
           if (Dlg && !IsDialogMessage(Dlg->TheDlg, &msg)
               && !IsDialogMessage(Dlg->AktChild, &msg))
#else
           if (Dlg && (!IsNestedDialogMessage(Dlg, &msg)))
#endif
                   DispatchMessage(&msg);
      }
   }
   else *release = TRUE;
   EnterCriticalSection(&crit_sec);  /* otherwise Dlg might be still in table but DelDialog is already running */
   if (DialogInAdminTable(Dlg))
   {
       DelDialog(Dlg);
       Dlg->TheThread = NULL;
   }
   LeaveCriticalSection(&crit_sec);
   return ret;
}



ULONG APIENTRY UsrCreateDialog(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   LONG argList[4];
   DLGTEMPLATE * p;
   HINSTANCE hI;
   ULONG thID;
   BOOL Release = FALSE;
   HANDLE hThread;
   HWND hW;
   DEF_ADM;

   CHECKARGL(6);
   GET_ADM;
   if (!dlgAdm) RETERR;

   if (argv[1].strptr[0] == 'C')   /* do we have a child dialog to be created? */
   {
       LONG l;
       /* set number of items to dialogtemplate */
       p = (DLGTEMPLATE *) atol(argv[3].strptr);
       p->cdit = (WORD) atoi(argv[2].strptr);

       l = atol(argv[4].strptr);
       /* send a create message. This is out of history so the child dialog has been created in a faster thread */
       hW = (HWND) SendMessage((HWND)l, CREATECHILD, (WPARAM) l, (LPARAM) p);

       dlgAdm->ChildDlg[atoi(argv[5].strptr)] = hW;

       /* free the memory allocated for template */
       if (p) LocalFree(p);

       RETVAL((ULONG)hW)
   }
   else
   {
       CHECKARGL(8);
       /* set number of items to dialogtemplate */
       p = (DLGTEMPLATE *) atol(argv[4].strptr);
       if (!p)
       {
           MessageBox(0,"Illegal resource buffer","Error",MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
           RETC(0)
       }

       p->cdit = (WORD) atoi(argv[2].strptr);

       EnterCriticalSection(&crit_sec);
       if (!InstallNecessaryStuff(dlgAdm, &argv[3], argc-3))
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

       argList[0] = (LONG) p;         /* dialog template */
       argList[1] = (LONG) dlgAdm;
       argList[2] = 0;                /* no auto detection */
       argList[3] = (LONG) &Release;  /* pass along pointer so that variable can be modified */
       Release = FALSE;
       dlgAdm->TheThread = hThread = CreateThread(NULL, 2000, WindowUsrLoopThread, argList, 0, &thID);

       while ((!Release) && dlgAdm && (dlgAdm->TheThread)) {Sleep(1);};  /* wait for dialog start */
       LeaveCriticalSection(&crit_sec);

       /* free the memory allocated for template */
       if (p) LocalFree(p);

       if (dlgAdm)
       {
           if (dlgAdm->TheDlg)
           {
              SetThreadPriority(dlgAdm->TheThread, THREAD_PRIORITY_ABOVE_NORMAL);   /* for a faster drawing */
              dlgAdm->OnTheTop = TRUE;

              if ((argc < 9) || !IsYes(argv[8].strptr))  /* do we have a modal dialog? */
              {
                if (dlgAdm->previous && IsWindowEnabled(((DIALOGADMIN *)dlgAdm->previous)->TheDlg))
                    EnableWindow(((DIALOGADMIN *)dlgAdm->previous)->TheDlg, FALSE);
              }

              if (GetWindowLong(dlgAdm->TheDlg, GWL_STYLE) & WS_SYSMENU)
              {
                 if (atoi(argv[7].strptr) > 0)
                 {
                    dlgAdm->SysMenuIcon = (HICON)SetClassLong(dlgAdm->TheDlg, GCL_HICON, (LONG)LoadIcon(dlgAdm->TheInstance, MAKEINTRESOURCE(atoi(argv[7].strptr))));
                 }
                 else
                 {
                    hI = LoadLibrary(VISDLL);
                    dlgAdm->SysMenuIcon = (HICON)SetClassLong(dlgAdm->TheDlg, GCL_HICON, (LONG)LoadIcon(hI, MAKEINTRESOURCE(99)));
                    FreeLibrary(hI);
                 }
              }
              RETVAL((ULONG)dlgAdm->TheDlg);
           }
           else  /* the dialog creation failed, so do a clean up */
           {
              dlgAdm->OnTheTop = FALSE;
              if (dlgAdm->previous) ((DIALOGADMIN *)(dlgAdm->previous))->OnTheTop = TRUE;
              if (dlgAdm->previous && !IsWindowEnabled(((DIALOGADMIN *)dlgAdm->previous)->TheDlg))
                   EnableWindow(((DIALOGADMIN *)dlgAdm->previous)->TheDlg, TRUE);
              /* memory cleanup is done in HandleDialogAdmin */
           }
       }
   }
   RETC(0)
}


void UAddControl(WORD **p, SHORT kind, INT id, INT x, INT y, INT cx, INT cy, CHAR * txt, ULONG lStyle)
{
   int   nchar;

   **p = LOWORD (lStyle);
   (*p)++;
   **p = HIWORD (lStyle);
   (*p)++;
   **p = 0;          // LOWORD (lExtendedStyle)
   (*p)++;
   **p = 0;          // HIWORD (lExtendedStyle)
   (*p)++;
   **p = x;         // x
   (*p)++;
   **p = y;         // y
   (*p)++;
   **p = cx;         // cx
   (*p)++;
   **p = cy;         // cy
   (*p)++;
   **p = id;         // ID
   (*p)++;

   **p = (WORD)0xffff;
   (*p)++;
   **p = (WORD)kind;
   (*p)++;

   if (txt)
   {
      nchar = nCopyAnsiToWideChar (*p, TEXT(txt));
      (*p) += nchar;
   }
   else
   {
     **p = 0;
    (*p)++;
   }

   **p = 0;  // advance pointer over nExtraStuff WORD
   (*p)++;

   /* make sure the next item starts on a DWORD boundary */
   (*p) = lpwAlign (*p);
}


void UAddNamedControl(WORD **p, CHAR * className, INT id, INT x, INT y, INT cx, INT cy, CHAR * txt, ULONG lStyle)
{
   int   nchar;

   **p = LOWORD (lStyle);
   (*p)++;
   **p = HIWORD (lStyle);
   (*p)++;
   **p = 0;          // LOWORD (lExtendedStyle)
   (*p)++;
   **p = 0;          // HIWORD (lExtendedStyle)
   (*p)++;
   **p = x;         // x
   (*p)++;
   **p = y;         // y
   (*p)++;
   **p = cx;         // cx
   (*p)++;
   **p = cy;         // cy
   (*p)++;
   **p = id;         // ID
   (*p)++;

   nchar = nCopyAnsiToWideChar (*p, TEXT(className));
   (*p) += nchar;

   if (txt)
   {
      nchar = nCopyAnsiToWideChar (*p, TEXT(txt));
      (*p) += nchar;
   }
   else
   {
     **p = 0;
    (*p)++;
   }

   **p = 0;  // advance pointer over nExtraStuff WORD
   (*p)++;

   /* make sure the next item starts on a DWORD boundary */
   (*p) = lpwAlign (*p);
}





ULONG APIENTRY UsrAddControl(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   INT buffer[6];
   ULONG lStyle;
   WORD *p = NULL;
   int i;

   CHECKARGL(1);

   if (!strcmp(argv[0].strptr,"BUT") || !strcmp(argv[0].strptr,"CH") || !strcmp(argv[0].strptr,"RB"))
   {
       CHECKARG(9);

       for (i=0;i<6;i++) buffer[i] = atoi(argv[i+1].strptr);

       p = (WORD *)buffer[0];

       lStyle = WS_CHILD;
       if (strstr(argv[8].strptr,"3STATE")) lStyle |= BS_AUTO3STATE; else
       if (!strcmp(argv[0].strptr,"CH")) lStyle |= BS_AUTOCHECKBOX; else
       if (!strcmp(argv[0].strptr,"RB")) lStyle |= BS_AUTORADIOBUTTON; else
       if (strstr(argv[8].strptr,"DEFAULT")) lStyle |= BS_DEFPUSHBUTTON; else lStyle |= BS_PUSHBUTTON;

       if (strstr(argv[8].strptr,"OWNER")) lStyle |= BS_OWNERDRAW;
       if (strstr(argv[8].strptr,"NOTIFY")) lStyle |= BS_NOTIFY;
       if (strstr(argv[8].strptr,"LEFTTEXT")) lStyle |= BS_LEFTTEXT;
       if (strstr(argv[8].strptr,"BITMAP")) lStyle |= BS_BITMAP;
       if (strstr(argv[8].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (strstr(argv[8].strptr,"DISABLED")) lStyle |= WS_DISABLED;
       if (!strstr(argv[8].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (!strstr(argv[8].strptr,"NOTAB")) lStyle |= WS_TABSTOP;
       if (strstr(argv[8].strptr,"GROUP")) lStyle |= WS_GROUP;

       /*                       id         x           y         cx          cy  */
       UAddControl(&p, 0x0080, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], argv[7].strptr, lStyle);
   }
   else
   if (!strcmp(argv[0].strptr,"EL"))
   {
       CHECKARG(8);

       for (i=0;i<6;i++) buffer[i] = atoi(argv[i+1].strptr);

       p = (WORD *)buffer[0];

       lStyle = WS_CHILD;
       if (strstr(argv[7].strptr,"PASSWORD")) lStyle |= ES_PASSWORD;
       if (strstr(argv[7].strptr,"MULTILINE")) lStyle |= ES_MULTILINE | ES_WANTRETURN | ES_NOHIDESEL;
       if (strstr(argv[7].strptr,"AUTOSCROLLH")) lStyle |= ES_AUTOHSCROLL;
       if (strstr(argv[7].strptr,"AUTOSCROLLV")) lStyle |= ES_AUTOVSCROLL;
       if (strstr(argv[7].strptr,"HSCROLL")) lStyle |= WS_HSCROLL;
       if (strstr(argv[7].strptr,"VSCROLL")) lStyle |= WS_VSCROLL;
       if (strstr(argv[7].strptr,"READONLY")) lStyle |= ES_READONLY;
       if (strstr(argv[7].strptr,"WANTRETURN")) lStyle |= ES_WANTRETURN;
       if (strstr(argv[7].strptr,"KEEPSELECTION")) lStyle |= ES_NOHIDESEL;
       if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;
       if (strstr(argv[7].strptr,"CENTER")) lStyle |= ES_CENTER;
       else if (strstr(argv[7].strptr,"RIGHT")) lStyle |= ES_RIGHT;
       else lStyle |= ES_LEFT;
       if (strstr(argv[7].strptr,"UPPER")) lStyle |= ES_UPPERCASE;
       if (strstr(argv[7].strptr,"LOWER")) lStyle |= ES_LOWERCASE;
       if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (!strstr(argv[7].strptr,"NOBORDER")) lStyle |= WS_BORDER;
       if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;
       if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;

       /*                         id          x       y          cx           cy  */
       UAddControl(&p, 0x0081, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], NULL, lStyle);
   }
   else
   if (!strcmp(argv[0].strptr,"TXT"))
   {
       CHECKARGL(8);

       for (i=0;i<5;i++) buffer[i] = atoi(argv[i+1].strptr);

       p = (WORD *)buffer[0];

       if (argc > 8)
          i = atoi(argv[8].strptr);
       else i = -1;

       lStyle = WS_CHILD;
       if (!strstr(argv[6].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[6].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (strstr(argv[6].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[6].strptr,"DISABLED")) lStyle |= WS_DISABLED;

       if (strstr(argv[6].strptr,"CENTER")) lStyle |= SS_CENTER;
       else if (strstr(argv[6].strptr,"RIGHT")) lStyle |= SS_RIGHT;
       else if (strstr(argv[6].strptr,"SIMPLE")) lStyle |= SS_SIMPLE;
       else if (strstr(argv[6].strptr,"LEFTNOWRAP")) lStyle |= SS_LEFTNOWORDWRAP;
       else lStyle |= SS_LEFT;

       /*                      id      x         y         cx       cy  */
       UAddControl(&p, 0x0082, i, buffer[1], buffer[2], buffer[3], buffer[4], argv[7].strptr, lStyle);
   }
   else
   if (!strcmp(argv[0].strptr,"LB"))
   {
       CHECKARG(8);

       for (i=0;i<6;i++) buffer[i] = atoi(argv[i+1].strptr);

       p = (WORD *)buffer[0];

       lStyle = WS_CHILD;
       if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[7].strptr,"COLUMNS")) lStyle |= LBS_USETABSTOPS;
       if (strstr(argv[7].strptr,"VSCROLL")) lStyle |= WS_VSCROLL;
       if (strstr(argv[7].strptr,"HSCROLL")) lStyle |= WS_HSCROLL;
       if (strstr(argv[7].strptr,"SORT")) lStyle |= LBS_STANDARD;
       if (strstr(argv[7].strptr,"NOTIFY")) lStyle |= LBS_NOTIFY;
       if (strstr(argv[7].strptr,"MULTI")) lStyle |= LBS_MULTIPLESEL;
       if (strstr(argv[7].strptr,"MCOLUMN")) lStyle |= LBS_MULTICOLUMN;
       if (strstr(argv[7].strptr,"PARTIAL")) lStyle |= LBS_NOINTEGRALHEIGHT;
       if (strstr(argv[7].strptr,"SBALWAYS")) lStyle |= LBS_DISABLENOSCROLL;
       if (strstr(argv[7].strptr,"KEYINPUT")) lStyle |= LBS_WANTKEYBOARDINPUT;
       if (strstr(argv[7].strptr,"EXTSEL")) lStyle |= LBS_EXTENDEDSEL;
       if (!strstr(argv[7].strptr,"NOBORDER")) lStyle |= WS_BORDER;
       if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;
       if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;

       /*                         id       x          y            cx        cy  */
       UAddControl(&p, 0x0083, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], NULL, lStyle);
   }
   else
   if (!strcmp(argv[0].strptr,"CB"))
   {
       CHECKARG(8);

       for (i=0;i<6;i++) buffer[i] = atoi(argv[i+1].strptr);

       p = (WORD *)buffer[0];

       lStyle = WS_CHILD;

       if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (!strstr(argv[7].strptr,"NOHSCROLL")) lStyle |= CBS_AUTOHSCROLL;
       if (strstr(argv[7].strptr,"VSCROLL")) lStyle |= WS_VSCROLL;
       if (strstr(argv[7].strptr,"SORT")) lStyle |= CBS_SORT;
       if (strstr(argv[7].strptr,"SIMPLE")) lStyle |= CBS_SIMPLE;
       else if (strstr(argv[7].strptr,"LIST")) lStyle |= CBS_DROPDOWNLIST;
       else lStyle |= CBS_DROPDOWN;
       if (!strstr(argv[7].strptr,"NOBORDER")) lStyle |= WS_BORDER;
       if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;
       if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[7].strptr,"PARTIAL")) lStyle |= CBS_NOINTEGRALHEIGHT;
       if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;

       /*                         id       x          y            cx        cy  */
       UAddControl(&p, 0x0085, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], NULL, lStyle);
   }
   else
   if (!strcmp(argv[0].strptr,"GB"))
   {
       CHECKARGL(8);

       for (i=0;i<5;i++) buffer[i] = atoi(argv[i+1].strptr);

       if (argc > 8)
          i = atoi(argv[8].strptr);
       else i = -1;

       p = (WORD *)buffer[0];

       lStyle = WS_CHILD | BS_GROUPBOX;
       if (!strstr(argv[6].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[6].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (strstr(argv[6].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[6].strptr,"DISABLED")) lStyle |= WS_DISABLED;

       /*                      id      x         y        cx        cy  */
       UAddControl(&p, 0x0080, i, buffer[1], buffer[2], buffer[3], buffer[4], argv[7].strptr, lStyle);
   }
   else
   if (!strcmp(argv[0].strptr,"FRM"))
   {
       CHECKARGL(8);

       for (i=0;i<6;i++) buffer[i] = atoi(argv[i+1].strptr);

       p = (WORD *)buffer[0];

       if (argc > 8)
          i = atoi(argv[8].strptr);
       else i = -1;

       lStyle = WS_CHILD;

       if (buffer[6] == 0) lStyle |= SS_WHITERECT; else
       if (buffer[6] == 1) lStyle |= SS_GRAYRECT; else
       if (buffer[6] == 2) lStyle |= SS_BLACKRECT; else
       if (buffer[6] == 3) lStyle |= SS_WHITEFRAME; else
       if (buffer[6] == 4) lStyle |= SS_GRAYFRAME; else
       lStyle |= SS_BLACKFRAME;
       if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[7].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;

       /*                     id    x           y          cx         cy  */
       UAddControl(&p, 0x0082, i, buffer[1], buffer[2], buffer[3], buffer[4], NULL, lStyle);
   }
   else
   if (!strcmp(argv[0].strptr,"SB"))
   {
       CHECKARG(8);

       for (i=0;i<6;i++) buffer[i] = atoi(argv[i+1].strptr);

       p = (WORD *)buffer[0];

       lStyle = WS_CHILD;
       if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
       if (strstr(argv[7].strptr,"HORIZONTAL")) lStyle |= SBS_HORZ; else lStyle |= SBS_VERT;
       if (strstr(argv[7].strptr,"TOPLEFT")) lStyle |= SBS_TOPALIGN;
       if (strstr(argv[7].strptr,"BOTTOMRIGHT")) lStyle |= SBS_BOTTOMALIGN;
       if (strstr(argv[7].strptr,"TAB")) lStyle |= WS_TABSTOP;
       if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
       if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;

       /*                         id       x          y            cx        cy  */
       UAddControl(&p, 0x0084, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], NULL, lStyle);
   }

   RETVAL((LONG)p)
}



/******************* New 32 Controls ***********************************/


LONG EvaluateListStyle(CHAR * styledesc)
{
    LONG lStyle = 0;

    if (!strstr(styledesc,"NOBORDER")) lStyle |= WS_BORDER;
    if (strstr(styledesc,"VSCROLL")) lStyle |= WS_VSCROLL;
    if (strstr(styledesc,"HSCROLL")) lStyle |= WS_HSCROLL;
    if (!strstr(styledesc,"NOTAB")) lStyle |= WS_TABSTOP;
    if (strstr(styledesc,"EDIT")) lStyle |= LVS_EDITLABELS;
    if (strstr(styledesc,"SHOWSELALWAYS")) lStyle |= LVS_SHOWSELALWAYS;
    if (strstr(styledesc,"ALIGNLEFT")) lStyle |= LVS_ALIGNLEFT;
    if (strstr(styledesc,"ALIGNTOP")) lStyle |= LVS_ALIGNTOP;
    if (strstr(styledesc,"AUTOARRANGE")) lStyle |= LVS_AUTOARRANGE;
    if (strstr(styledesc,"ICON")) lStyle |= LVS_ICON;
    if (strstr(styledesc,"SMALLICON")) lStyle |= LVS_SMALLICON;
    if (strstr(styledesc,"LIST")) lStyle |= LVS_LIST;
    if (strstr(styledesc,"REPORT")) lStyle |= LVS_REPORT;
    if (strstr(styledesc,"NOHEADER")) lStyle |= LVS_NOCOLUMNHEADER;
    if (strstr(styledesc,"NOWRAP")) lStyle |= LVS_NOLABELWRAP;
    if (strstr(styledesc,"NOSCROLL")) lStyle |= LVS_NOSCROLL;
    if (strstr(styledesc,"NOSORTHEADER")) lStyle |= LVS_NOSORTHEADER;
    if (strstr(styledesc,"SHAREIMAGES")) lStyle |= LVS_SHAREIMAGELISTS;
    if (strstr(styledesc,"SINGLESEL")) lStyle |= LVS_SINGLESEL;
    if (strstr(styledesc,"ASCENDING")) lStyle |= LVS_SORTASCENDING;
    if (strstr(styledesc,"DESCENDING")) lStyle |= LVS_SORTDESCENDING;
    return lStyle;
}



ULONG APIENTRY UsrAddNewCtrl(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   INT buffer[6];
   ULONG lStyle;
   WORD *p;
   int i;

   CHECKARG(8);

   for (i=0;i<6;i++) buffer[i] = atoi(argv[i+1].strptr);

   p = (WORD *)buffer[0];

   lStyle = WS_CHILD;
   if (!strstr(argv[7].strptr,"HIDDEN")) lStyle |= WS_VISIBLE;
   if (strstr(argv[7].strptr,"GROUP")) lStyle |= WS_GROUP;
   if (strstr(argv[7].strptr,"DISABLED")) lStyle |= WS_DISABLED;
   if (!CommCtrlLoaded) {
       InitCommonControls();
       CommCtrlLoaded = TRUE;
   }

   if (!strcmp(argv[0].strptr,"TREE"))
   {
       if (!strstr(argv[7].strptr,"NOBORDER")) lStyle |= WS_BORDER;
       if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;
       if (strstr(argv[7].strptr,"VSCROLL")) lStyle |= WS_VSCROLL;
       if (strstr(argv[7].strptr,"HSCROLL")) lStyle |= WS_HSCROLL;
       if (strstr(argv[7].strptr,"NODRAG")) lStyle |= TVS_DISABLEDRAGDROP;
       if (strstr(argv[7].strptr,"EDIT")) lStyle |= TVS_EDITLABELS;
       if (strstr(argv[7].strptr,"BUTTONS")) lStyle |= TVS_HASBUTTONS;
       if (strstr(argv[7].strptr,"LINES")) lStyle |= TVS_HASLINES;
       if (strstr(argv[7].strptr,"ATROOT")) lStyle |= TVS_LINESATROOT;
       if (strstr(argv[7].strptr,"SHOWSELALWAYS")) lStyle |= TVS_SHOWSELALWAYS;
        /*                                   id       x          y            cx        cy  */
       UAddNamedControl(&p, WC_TREEVIEW, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], NULL, lStyle);
       RETVAL((LONG)p)
   }
   else
   if (!strcmp(argv[0].strptr,"LIST"))
   {
       lStyle |= EvaluateListStyle(argv[7].strptr);
        /*                                   id       x          y            cx        cy  */
       UAddNamedControl(&p, WC_LISTVIEW, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], NULL, lStyle);
       RETVAL((LONG)p)
   }
   else
   if (!strcmp(argv[0].strptr,"PROGRESS"))
   {
       if (strstr(argv[7].strptr,"VERTICAL")) lStyle |= PBS_VERTICAL;
       if (strstr(argv[7].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (strstr(argv[7].strptr,"SMOOTH")) lStyle |= PBS_SMOOTH;
        /*                                     id       x          y            cx        cy  */
       UAddNamedControl(&p, PROGRESS_CLASS, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], NULL, lStyle);
       RETVAL((LONG)p)
   }
   else
   if (!strcmp(argv[0].strptr,"SLIDER"))
   {
       if (strstr(argv[7].strptr,"BORDER")) lStyle |= WS_BORDER;
       if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;
       if (strstr(argv[7].strptr,"AUTOTICKS")) lStyle |= TBS_AUTOTICKS;
       if (strstr(argv[7].strptr,"NOTICKS")) lStyle |= TBS_NOTICKS;
       if (strstr(argv[7].strptr,"VERTICAL")) lStyle |= TBS_VERT;
       if (strstr(argv[7].strptr,"HORIZONTAL")) lStyle |= TBS_HORZ;
       if (strstr(argv[7].strptr,"TOP")) lStyle |= TBS_TOP;
       if (strstr(argv[7].strptr,"BOTTOM")) lStyle |= TBS_BOTTOM;
       if (strstr(argv[7].strptr,"LEFT")) lStyle |= TBS_LEFT;
       if (strstr(argv[7].strptr,"RIGHT")) lStyle |= TBS_RIGHT;
       if (strstr(argv[7].strptr,"BOTH")) lStyle |= TBS_BOTH;
       if (strstr(argv[7].strptr,"ENABLESELRANGE")) lStyle |= TBS_ENABLESELRANGE;
        /*                                   id       x          y            cx        cy  */
       UAddNamedControl(&p, TRACKBAR_CLASS, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], NULL, lStyle);
       RETVAL((LONG)p)
   }
   else
   if (!strcmp(argv[0].strptr,"TAB"))
   {
        if (strstr(argv[7].strptr,"BORDER")) lStyle |= WS_BORDER;
        if (!strstr(argv[7].strptr,"NOTAB")) lStyle |= WS_TABSTOP;
        if (strstr(argv[7].strptr,"BUTTONS")) lStyle |= TCS_BUTTONS;
        else lStyle |= TCS_TABS;
        if (strstr(argv[7].strptr,"FIXED")) lStyle |= TCS_FIXEDWIDTH;
        if (strstr(argv[7].strptr,"FOCUSNEVER")) lStyle |= TCS_FOCUSNEVER;
        if (strstr(argv[7].strptr,"FOCUSONDOWN")) lStyle |= TCS_FOCUSONBUTTONDOWN;
        if (strstr(argv[7].strptr,"ICONLEFT")) lStyle |= TCS_FORCEICONLEFT;
        if (strstr(argv[7].strptr,"LABELLEFT")) lStyle |= TCS_FORCELABELLEFT;
        if (strstr(argv[7].strptr,"MULTILINE")) lStyle |= TCS_MULTILINE;
        else lStyle |= TCS_SINGLELINE;
        if (strstr(argv[7].strptr,"ALIGNRIGHT")) lStyle |= TCS_RIGHTJUSTIFY;
        if (strstr(argv[7].strptr,"CLIPSIBLINGS")) lStyle |= WS_CLIPSIBLINGS;  /* used for property sheet to prevent wrong display */

        /*                                   id       x          y            cx        cy  */
        UAddNamedControl(&p, WC_TABCONTROL, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], NULL, lStyle);
        RETVAL((LONG)p)
   }

   RETC(0)
}




extern BOOL SHIFTkey = FALSE;

#ifndef USE_DS_CONTROL
BOOL IsNestedDialogMessage(
    DIALOGADMIN * dlgAdm,
    LPMSG  lpmsg    // address of structure with message
   )
{
   HWND hW, hParent, hW2;
   BOOL prev = FALSE;

   if ((!dlgAdm->ChildDlg) || (!dlgAdm->AktChild))
      return IsDialogMessage(dlgAdm->TheDlg, lpmsg);

   switch (lpmsg->message)
   {
      case WM_KEYDOWN:
         switch (lpmsg->wParam)
            {
            case VK_SHIFT: SHIFTkey = TRUE;
               break;

            case VK_TAB:

               if (IsChild(dlgAdm->AktChild, lpmsg->hwnd)) hParent = dlgAdm->AktChild; else hParent = dlgAdm->TheDlg;

               hW = GetNextDlgTabItem(hParent, lpmsg->hwnd, SHIFTkey);
               hW2 = GetNextDlgTabItem(hParent, NULL, SHIFTkey);

               /* see if we have to switch to the other dialog */
               if (hW == hW2)
               {
                  if (hParent == dlgAdm->TheDlg)
                     hParent = dlgAdm->AktChild;
                  else
                     hParent = dlgAdm->TheDlg;

                  hW = GetNextDlgTabItem(hParent, NULL, SHIFTkey);
                  return SendMessage(hParent, WM_NEXTDLGCTL, (WPARAM)hW, (LPARAM)TRUE);

               } else return SendMessage(hParent, WM_NEXTDLGCTL, (WPARAM)hW, (LPARAM)TRUE);

                return TRUE;

            case VK_LEFT:
            case VK_UP:
               prev = TRUE;
            case VK_RIGHT:
            case VK_DOWN:

               if (IsChild(dlgAdm->AktChild, lpmsg->hwnd)) hParent = dlgAdm->AktChild; else hParent = dlgAdm->TheDlg;

               hW = GetNextDlgGroupItem(hParent, lpmsg->hwnd, prev);
               hW2 = GetNextDlgGroupItem(hParent, NULL, prev);

               /* see if we have to switch to the other dialog */
               if (hW == hW2)
               {
                  if (hParent == dlgAdm->TheDlg)
                     hParent = dlgAdm->AktChild;
                  else
                     hParent = dlgAdm->TheDlg;

                   return IsDialogMessage(hParent, lpmsg);

               } else
                return IsDialogMessage(hParent, lpmsg);

                return TRUE;

            case VK_CANCEL:
            case VK_RETURN:
               return IsDialogMessage(dlgAdm->TheDlg, lpmsg);

            default:
               hParent = (HWND) GetWindowLong(lpmsg->hwnd, GWL_HWNDPARENT);
               if (!hParent) return FALSE;
               return IsDialogMessage(hParent, lpmsg);
           }
         break;

      case WM_KEYUP:
         if (lpmsg->wParam == VK_SHIFT) SHIFTkey = FALSE;
         break;
   }
   hParent = (HWND) GetWindowLong(lpmsg->hwnd, GWL_HWNDPARENT);
   if (hParent)
      return IsDialogMessage(hParent, lpmsg);
   else return IsDialogMessage(dlgAdm->TheDlg, lpmsg);
}
#endif



ULONG APIENTRY UsrMenu(
  PUCHAR funcname,
  ULONG argc,
  RXSTRING argv[],
  PUCHAR qname,
  PRXSTRING retstr )
{
   INT i;
   WORD *p, *template;
   HANDLE hMem;

   CHECKARGL(1);

   if (!strcmp(argv[0].strptr,"INIT"))
   {
       if (argc == 2)
       {
          i = atoi(argv[1].strptr);
       } else i = 100;

       hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (i+1)*128);

       template = p = (PWORD) GlobalLock(hMem);

       if (!p) RETC(1)
       /* writing menu header */
    #if EXTENDED_MENU
       *p = 1;    /* for extended menu */
       p++;
       *p = 0;
       offsetptr = p;
       p++;
       *p = 0;   /* DWORD helpid */
       p++;
       *p = 0;
       p++;
       p = lpwAlign (p);
       *offsetptr = (ULONG)p - (ULONG)offsetptr - sizeof(WORD);
    #else
       *p = 0;    /* for normal menu */
       p++;
       *p = 0;
       p++;
    #endif

       sprintf(retstr->strptr, "%ld %ld %ld", hMem, template, p);
       retstr->strlength = strlen(retstr->strptr);
       return 0;
   }
   else
   if (!strcmp(argv[0].strptr,"ADD"))
   {
       ULONG lStyle = MFS_ENABLED | MFS_UNCHECKED;
       ULONG lType = MFT_STRING;
       WORD lResInfo = 0;
       int nchar;

       CHECKARG(5);

       p = (WORD *) atol(argv[1].strptr);
       if (!p) RETC(1)

#if EXTENDED_MENU
       if (strstr(argv[4].strptr, "CHECK")) lStyle |= MFS_CHECKED;
       if (strstr(argv[4].strptr, "GRAY")) lStyle |= MFS_GRAYED;
       if (strstr(argv[4].strptr, "DISABLE")) lStyle |= MFS_DISABLED;
       if (strstr(argv[4].strptr, "HILITE")) lStyle |= MFS_HILITE;

       if (strstr(argv[4].strptr, "POPUP")) lResInfo |= 0x01;
       if (strstr(argv[4].strptr, "END")) lResInfo |= 0x80;

       if (strstr(argv[4].strptr, "SEPARATOR")) lType = MFT_SEPARATOR;
       if (strstr(argv[4].strptr, "RIGHT")) lType |= MFT_RIGHTJUSTIFY;
       if (strstr(argv[4].strptr, "RADIO")) lType |= MFT_RADIOCHECK;

       This is not finsihed!

       dp = p;
       *dp = lType;
       dp++;
       *dp = lStyle;
       dp++;
       p = dp;
       *p = atoi(argv[2].strptr);     /* menu id */
       p++;
       *p = lResInfo;
       p++;

       /* copy the name of the item */
       if (strlen(argv[3].strptr) > 96)
       {
           *p = 0;
           p++;
       }
       else
       {
           nchar = nCopyAnsiToWideChar (p, TEXT(argv[3].strptr));
           p += nchar;
       }
       p = lpwAlign (p);
       if (lResInfo == 0x01)
       {
           *p = 0;
           p++;
           *p = 0;
           p++;
           p = lpwAlign (p);
       }
#else
       if (strstr(argv[4].strptr, "CHECKED")) lStyle |= MF_CHECKED;
       if (strstr(argv[4].strptr, "GRAYED")) lStyle |= MF_GRAYED;
       if (strstr(argv[4].strptr, "DISABLED")) lStyle |= MF_DISABLED;
       if (strstr(argv[4].strptr, "POPUP")) lStyle |= MF_POPUP;
       if (strstr(argv[4].strptr, "END")) lStyle |= MF_END;
       if (strstr(argv[4].strptr, "SEPARATOR")) lStyle |= MF_SEPARATOR;


       *p = (WORD)lStyle;
       p++;

       if (!(lStyle & MF_POPUP))
       {
           *p = atoi(argv[2].strptr);     /* menu id */
           p++;
       }

       /* copy the name of the item */
       if (strlen(argv[3].strptr) > 96)
       {
           *p = 0;
           p++;
       }
       else
       {
           nchar = nCopyAnsiToWideChar (p, TEXT(argv[3].strptr));
           p += nchar;
       }
#endif
       RETVAL((LONG)p)
   }
   else
   if (!strcmp(argv[0].strptr,"SET"))
   {
       HWND hWnd;
       PVOID *p;
       DEF_ADM;

       CHECKARG(4);

       hWnd = (HWND)atol(argv[1].strptr);

       if (hWnd)
       {
          SEEK_DLGADM_TABLE(hWnd, dlgAdm);
          if (!dlgAdm) RETC(1)

          hMem = (HANDLE) atol(argv[2].strptr);
          p = (PVOID *) atol(argv[3].strptr);

          if (p)
          {
              INT rc;
              dlgAdm->menu = LoadMenuIndirect(p);
              rc = GetLastError();

              if (dlgAdm->menu)
                  SetMenu(hWnd, dlgAdm->menu);

              /* free the memory allocated for template */
              GlobalUnlock(hMem);
              GlobalFree(hMem);
              if (!dlgAdm->menu)
                  RETVAL(rc)
              RETC(0)
          }
       }
       RETC(1)
   }
   RETC(0)
}

