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
#include "orxgui.h"


/****************************************************************************
*
*    FUNCTION: EnterParms(HWND, UINT, UINT, LONG)
*
*    PURPOSE:  Processes messages for "EnterNew" dialog box
*
*    COMMENTS:
*
*        This function allows the user to enter new text in the current
*        window.  This text is stored in the global current buffer.
*
****************************************************************************/

BOOL CALLBACK EnterParms(
        HWND hDlg,                /* window handle of the dialog box */
        UINT message,             /* type of message                 */
        UINT wParam,            /* message-specific information    */
        LONG lParam)
{
    switch (message)
    {
       case WM_INITDIALOG:
          return (TRUE);

        case WM_COMMAND:                      /* message: received a command */
            if (LOWORD(wParam) == IDC_OK)
            {
                GetDlgItemText( hDlg, IDC_EDITORXPARMS, g_OrxParmBuf, 256-1);
                EndDialog( hDlg, IDC_OK );
                return (TRUE);
            }
            else if (LOWORD(wParam) == IDC_CANCEL)
            {
                EndDialog(hDlg, IDC_CANCEL);        /* Exits the dialog box        */
                return (TRUE);
            }
            break;
         case WM_CLOSE:                       /* message: received a command */
            // End the dialog if user closes from system menu
            EndDialog(hDlg, IDC_CANCEL);
            return(TRUE);
            break;
    } // switch
    return (FALSE);                           /* Didn't process a message    */


    // avoid compiler warnings at W3
    lParam;
}

/****************************************************************************
*
*    FUNCTION: EditorSelection(hWND, UINT, UINT, LONG)
*
*    PURPOSE:  Processes messages for "EditorSelection" dialog box
*
*    COMMENTS:
*
*        This function allows the user to enter new editor
*
****************************************************************************/

BOOL CALLBACK EditorSelection(
        HWND hDlg,                /* window handle of the dialog box */
        UINT message,             /* type of message                 */
        UINT wParam,            /* message-specific information    */
        LONG lParam)
{
    switch (message)
    {
       case WM_INITDIALOG:
          return (TRUE);

        case WM_COMMAND:                      /* message: received a command */
            if (LOWORD(wParam) == IDC_OPTEDITORFNSAVE)
            {
                GetDlgItemText( hDlg, IDC_OPTEDITORFN, g_szEditorBuf, MAX_PATH-1);
                EndDialog( hDlg, IDC_OK );
                return (TRUE);
            }
            else if (LOWORD(wParam) == IDC_OPTEDITORCANCEL)
            {
                EndDialog(hDlg, IDC_CANCEL);        /* Exits the dialog box        */
                return (TRUE);
            }
            break;
         case WM_CLOSE:                       /* message: received a command */
            // End the dialog if user closes from system menu
            EndDialog(hDlg, IDC_CANCEL);
            return(TRUE);
            break;
    } // switch
    return (FALSE);                           /* Didn't process a message    */


    // avoid compiler warnings at W3
    lParam;
}
/****************************************************************************
*
*    FUNCTION: OrxOpenFile(HWND)
*
*    PURPOSE:  Invokes common dialog function to open a file and opens it.
*
*    COMMENTS:
*
*        This function initializes the OPENFILENAME structure and calls
*        the GetOpenFileName() common dialog function.  This function will
*        work regardless of the mode: standard, using a hook or using a
*        customized template.
*
*    RETURN VALUES:
*        TRUE - The file was opened successfully and read into the buffer.
*        FALSE - No files were opened.
*
****************************************************************************/
BOOL OrxOpenFile ( char *pszFname, HWND hWnd)
{
  // let the user specify a filename using a common dialog
    OPENFILENAME    ofn;                    // open struct
    char            szFname[MAX_PATH];        // filename string

    strcpy ( szFname, "" );
    memset ( &ofn, 0, sizeof (OPENFILENAME) );
    ofn.lStructSize = sizeof ( OPENFILENAME );
    ofn.lpstrFile = szFname;
    ofn.nMaxFile = sizeof ( szFname );
    ofn.lpstrTitle = "Object Rexx program selection";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
        ofn.lpstrFilter = "Object Files (*.REX)\0*.REX\0All Files (*.*)\0*.*\0";
        ofn.lpstrDefExt = "REX";

  // display the common dialog, and retrieve the file
  // if the user OKs the dialog
    if ( GetOpenFileName ( &ofn ) )
    {
       strcpy(pszFname,szFname);             // set value in callers var
       return(TRUE);
    }
    else
    {
        DWORD   dwErr = CommDlgExtendedError ();
        if ( dwErr == 0 )
            return FALSE;        // cancel button pressed
        else
        {
            MessageBox ( hWnd,"Common Dialog Error","Open",MB_OK|MB_ICONEXCLAMATION );
            return FALSE;
        }
    }
}


