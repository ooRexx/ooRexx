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
 * oodRoutines.cpp
 *
 * Contains the implementation for the ooDialog public routines.
 */

#include "ooDialog.h"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include "oodCommon.h"


/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry InfoMessage(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND hW;

   CHECKARG(1);

   if ((topDlg) && (topDlg->OnTheTop)) hW = topDlg->TheDlg; else hW = NULL;
   MessageBox(hW,argv[0].strptr,"Information", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TASKMODAL);
   RETC(0)
}


/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry ErrorMessage(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND hW;

   CHECKARG(1);

   if ((topDlg) && (topDlg->OnTheTop)) hW = topDlg->TheDlg; else hW = NULL;
   MessageBox(hW,argv[0].strptr,"Error", MB_OK | MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL);
   RETC(0)
}

/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry YesNoMessage(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   HWND hW;
   UINT uType = MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TASKMODAL;

   CHECKARGLH(1, 2);

   if ( argc == 2 )
   {
      if ( IsNo(argv[1].strptr) )
         uType |= MB_DEFBUTTON2;
      else if ( ! isYes(argv[1].strptr) )
      {
         PSZ  pszMsg;
         CHAR szText[] = "YesNoMessage argument 2 must be one of [Yes, No]; "
                         "found \"%s\"";

         pszMsg = (PSZ)LocalAlloc(LPTR, sizeof(szText) + 1 + argv[1].strlength);
         if ( ! pszMsg )
            RETERR;
         sprintf(pszMsg, szText, argv[1].strptr);
         HandleError(retstr, pszMsg);
         LocalFree(pszMsg);
         return 40;
      }
   }

   retstr->strlength = 1;
   if ((topDlg) && (topDlg->OnTheTop)) hW = topDlg->TheDlg; else hW = NULL;

   if (MessageBox(hW,argv[0].strptr,"Question", uType) == IDYES)
      retstr->strptr[0] = '1';
   else
      retstr->strptr[0] = '0';
   return 0;
}

/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry SleepMS(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
   CHECKARG(1);

   Sleep(atoi(argv[0].strptr));
   RETC(0)
}

/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry WinTimer(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    UINT_PTR timerID;
    MSG msg;

    CHECKARG(2);
    if ( !stricmp(argv[0].strptr, "START") )
    {
        timerID = SetTimer(NULL, 1001, atoi(argv[1].strptr), NULL);
        RETPTR(timerID)
    }
    else if ( !stricmp(argv[0].strptr, "STOP") )
    {
        timerID = (UINT_PTR)GET_POINTER(argv[1]);
        if ( KillTimer(NULL, timerID) == 0 )
        {
            RETVAL(GetLastError())
        }
        RETC(0)
    }
    else if ( !stricmp(argv[0].strptr, "WAIT") )
    {
        timerID = (UINT_PTR)GET_POINTER(argv[1]);
        while ( !PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE) || (msg.wParam != timerID) )
        {
            ; // do nothing
        }
        RETC(0)
    }
    RETC(1)
}


#define FILENAME_BUFFER_LEN 65535

UINT_PTR CALLBACK  OFNSetForegroundHookProc(
    HWND hdlg,    // handle to child dialog window
    UINT uiMsg,    // message identifier
    WPARAM wParam,    // message parameter
    LPARAM lParam)    // message parameter
{
    if (uiMsg == WM_INITDIALOG)
    {
        HWND h = GetParent(hdlg);
        if (!h) h = hdlg;
        SetForegroundWindow(h);
    }
    return 0;   /* 0 means default routine handles message */
}


BOOL OpenFileDlg( BOOL load, PCHAR szFile, const char *szInitialDir, const char *szFilter, HWND hw, const char *title, const char *DefExt, BOOL multi, CHAR chSepChar) /* @DOE005M */
{
   OPENFILENAME OpenFileName;
   BOOL         fRc;

   OpenFileName.lStructSize       = sizeof(OPENFILENAME);
   OpenFileName.hwndOwner         = hw;
   OpenFileName.hInstance         = 0;
   OpenFileName.lpstrFilter       = szFilter;
   OpenFileName.lpstrCustomFilter = (LPSTR) NULL;
   OpenFileName.nMaxCustFilter    = 0L;
   OpenFileName.nFilterIndex      = 1L;
   OpenFileName.lpstrFile         = szFile;
   OpenFileName.nMaxFile          = FILENAME_BUFFER_LEN;
   OpenFileName.lpstrFileTitle    = NULL; /* we don't need the selected file */
   OpenFileName.nMaxFileTitle     = 0;    /* we don't need the selected file */
   OpenFileName.lpstrInitialDir   = szInitialDir;
   OpenFileName.lpstrTitle        = title;
   OpenFileName.nFileOffset       = 0;
   OpenFileName.nFileExtension    = 0;
   OpenFileName.lpstrDefExt       = DefExt;
   OpenFileName.lCustData         = 0;
   OpenFileName.lpfnHook          = OFNSetForegroundHookProc;   /* hook to set dialog to foreground */

   /* The OFN_EXPLORER flag is required when using OFN_ENABLEHOOK, otherwise the dialog is old style and does not change directory */
   OpenFileName.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_EXPLORER;   /* enable hook */

   if (load && multi) OpenFileName.Flags |= OFN_ALLOWMULTISELECT;

   if (load)
   {
       OpenFileName.Flags |= OFN_FILEMUSTEXIST;
       fRc = GetOpenFileName(&OpenFileName);

       if (fRc && multi)
       {
         /* OFN_EXPLORER returns the selected name separated with ASCII 0 instead of spaces */
         PCHAR pChr = szFile;

         while( (*pChr != 0) || (*(pChr+1) != 0))
         {
           if (*pChr == 0)
             *pChr =  chSepChar;
           pChr++;
         }
       }

       return fRc;
   }
   else
   {
       OpenFileName.Flags |= OFN_OVERWRITEPROMPT;
       return GetSaveFileName(&OpenFileName);
   }
}



#define VALIDARG(argn) (argc >= argn) && argv[argn-1].strptr && argv[argn-1].strptr[0]

/**
 * This classic Rexx external function was documented prior to 4.0.0.
 */
size_t RexxEntry GetFileNameWindow(const char *funcname, size_t argc, CONSTRXSTRING *argv, const char *qname, RXSTRING *retstr)
{
    BOOL    fSuccess;
    const char *  title;
    const char *defext = "TXT";
    BOOL    load = TRUE;
    BOOL    multi = FALSE;
    HWND    hWnd;
    const char *szFilter = "Text Files (*.TXT)\0*.TXT\0All Files (*.*)\0*.*\0";
    PCHAR   pszFiles = NULL;
    PCHAR   pszInitialDir = NULL;
    CHAR    chSepChar = ' ';  /* default separation character  /              */
                              /* allow to change separation character to      */
                              /* handle filenames with blank character        */

    pszFiles = (char *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, FILENAME_BUFFER_LEN);
    if (!pszFiles)
        RETERR

    if (VALIDARG(1))
    {
        if ( argv[0].strptr[argv[0].strlength - 1] == '\\' )
        {
            pszInitialDir = (char *)LocalAlloc(LPTR, _MAX_PATH);
            if ( !pszInitialDir )
              RETERR
            rxstrlcpy(pszInitialDir, argv[0]);
        }
        else
        {
          rxstrlcpy(pszFiles, argv[0]);
        }
    }
    if (VALIDARG(2)) hWnd = GET_HWND(argv[1]); else hWnd = NULL;
    if (VALIDARG(3)) szFilter= argv[2].strptr;
    if (VALIDARG(4)) load = (argv[3].strptr[0] != '0');
    if (VALIDARG(5)) title = argv[4].strptr;
    else {
        if (load) title = "Open a File";
        else title = "Save File As";
    }
    if ((argc >= 6) && (argv[5].strptr)) defext = argv[5].strptr;
    if (VALIDARG(7)) multi = isYes(argv[6].strptr);

    if (VALIDARG(8)) chSepChar =  argv[7].strptr[0];

    retstr->strlength = 0;
    fSuccess = OpenFileDlg(load, pszFiles, pszInitialDir, szFilter, hWnd, title,
                           defext, multi, chSepChar);

    if ( pszInitialDir )
        LocalFree(pszInitialDir);
    if ( fSuccess )
    {
        /* we simply use the allocated string as return code and let REXX free it */
        retstr->strptr = pszFiles;
        retstr->strlength = strlen(pszFiles);
        return 0;
    }

    if (CommDlgExtendedError())
        RETERR
    else
        RETC(0);
}

