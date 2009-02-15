/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
/*********************************************************************/
/*
/*  File Name:          CALLRXWN - Windows application
/*
/*  Description:        Provides a sample call to the REXX
/*                      interpreter, passing in an environment name,
/*                      a file name, and a single argument string.
/*
/*                      A dialog box is created for the output.
/*                      Note that you must define a REXX standard
/*                      input and output exit handler for Windows
/*                      applications. Console applications are
/*                      not required to do this.
/*
/*
/*  Entry Points:       main - main entry point
/*                      RexxIOExit - REXX input and output exit
/*
/*  Input:              None
/*
/*  Output:             returns 0 in all cases.
/*
\*********************************************************************/

#include <rexx.h>                      /* needed for RexxStart()     */
#include <stdio.h>                     /* needed for printf()        */
#include <string.h>                    /* needed for strlen()        */
#include "callrxwn.h"                  /* prototypes, globals, const */

/*********************************************************************\
*
*  function:  WinMain()
*
*  input parameters:  c.f. generic sample
*
\**********************************************************************/
int REXXENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)
{
int ret;

    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );
    UNREFERENCED_PARAMETER( nCmdShow);
                                       // create a dialog box for REXX STDIO
    ret = DialogBox (hInstance, "callrexxDlg", NULL, (DLGPROC)MainDlgProc);
    return ret;
}

/**********************************************************************\
*
*  function:  MainDlgProc()
*
*  input parameters:  standard window procedure parameters.
*
*  Description: at initialization time, call CallRexx routine to do
*  rexxstart
*
\**********************************************************************/
LRESULT CALLBACK MainDlgProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{

  UNREFERENCED_PARAMETER(lParam);


  switch (msg) {

    case WM_INITDIALOG: {
      gHwnd=hwnd;                      // Save handle for exits
      (VOID)CallRexx(hwnd);           // call RexxStart
    } break;


    /******************************************************************\
    * WM_SYSCOMMAND
    *
    * ignore all syscommand messages, except for SC_CLOSE.
    *  on this one, call EndDialog().
    \******************************************************************/
    case WM_SYSCOMMAND:
      if (wParam == SC_CLOSE) {
        EndDialog (hwnd, TRUE);
        return TRUE;
      } else
        return FALSE;
    break;


    default: return FALSE;
  } /* end switch(message) */
  return 0;
}


/*********************************************************************\
*  function:  CallRexx()
*
*  Description:        Provides a sample call to the REXX
*                      interpreter, passing in an environment name,
*                      a file name, and a single argument string.
*
*  input parameters:
*   hwnd  - parent of the list box with the info.
*
*  output:
*     returns 0
*
*
\*********************************************************************/
int CallRexx(HWND hwnd)
{

   RXSYSEXIT exitlist[9];              /* Exit list array            */
   RXSTRING arg;                       /* argument string for REXX   */
   RXSTRING rexxretval;                /* return value from REXX     */

   CHAR    *str = "These words will be swapped"; /* text to swap     */

   RexxReturnCode   rc;                        /* return code from REXX      */
   SHORT    rexxrc = 0;                /* return code from function  */
   CHAR     *chTextOut[]={
            "This program will call the REXX interpreter to reverse  ",
            "    the order of the words in a string. The interpreter ",
            "    is invoked with an initial environment name of 'FNC'",
            "    and a file name of 'BACKWARD.FNC'"
            };
   SHORT sIndex;                       /* index into output text     */

                                       /* put info text on dialog    */
   for (sIndex=0; sIndex < 4; sIndex++) {
         SendDlgItemMessage (hwnd, DID_LISTBOX, LB_ADDSTRING, 0,
                             (LONG)chTextOut[sIndex] );
   }

   /* By setting the strlength of the output RXSTRING to zero, we    */
   /* force the interpreter to allocate memory and return it to us.  */
   /* We could provide a buffer for the interpreter to use instead.  */
   rexxretval.strlength = 0L;          /* initialize return to empty */

   MAKERXSTRING(arg, str, strlen(str));/* create input argument      */

                                       /* register exit handler      */
   rc = RexxRegisterExitExe("RexxIOExit",
        &RexxIOExit,                   /* located at this address    */
        NULL);

                                       /* set up for RXSIO exit      */
   exitlist[0].sysexit_name = "RexxIOExit";
   exitlist[0].sysexit_code = RXSIO;
   exitlist[1].sysexit_code = RXENDLST;

   /* Here we call the interpreter.  We don't really need to use     */
   /* all the casts in this call; they just help illustrate          */
   /* the data types used.                                           */
   rc=RexxStart(1,               /* number of arguments        */
                &arg,            /* array of arguments         */
                "BACKWARD.FNC",  /* name of REXX file          */
                0,               /* No INSTORE used            */
                "FNC",           /* Command env. name          */
                RXSUBROUTINE,    /* Code for how invoked       */
                exitlist,        /* No EXITs on this call      */
                &rexxrc,         /* Rexx program output        */
                &rexxretval);    /* Rexx program output        */

                                       /* send rc info to dialog box */
   wsprintf (chTxtBuffer," %s    %d", "Interpreter Return Code: ", rc);
   SendDlgItemMessage (hwnd, DID_LISTBOX, LB_ADDSTRING, 0, (LONG)chTxtBuffer);

   wsprintf (chTxtBuffer," %s    %d", "Function Return Code: ", (int) rexxrc);
   SendDlgItemMessage (hwnd, DID_LISTBOX, LB_ADDSTRING, 0, (LONG)chTxtBuffer);

   wsprintf (chTxtBuffer," %s   '%s' ", "Original String: ", arg.strptr);
   SendDlgItemMessage (hwnd, DID_LISTBOX, LB_ADDSTRING, 0, (LONG)chTxtBuffer);

   wsprintf (chTxtBuffer," %s   '%s' ", "Backwards String: ", rexxretval.strptr);
   SendDlgItemMessage (hwnd, DID_LISTBOX, LB_ADDSTRING, 0, (LONG)chTxtBuffer);

   RexxFreeMemory(rexxretval.strptr);  /* Release storage            */
                                       /* given to us by REXX.       */
                                       /* remove the exit            */
   RexxDeregisterExit("RexxIOExit",NULL);

   return 0;
   }

/*********************************************************************\
*  function:  CallRexx()
*
*  Description:    This is our REXX Standard input and output handler
*
\*********************************************************************/
LONG REXXENTRY RexxIOExit(
     LONG ExitNumber,                  /* code defining exit function*/
     LONG Subfunction,                 /* code defining exit subfunc */
     PEXIT parmblock)                  /* func dependent control bloc*/
{
   RXSIOSAY_PARM *sparm ;
   RXSIOTRC_PARM *tparm ;

   switch (Subfunction) {
   case RXSIOSAY:                      /* write line to standard     */
                                       /* output stream for SAY instr*/
      sparm = ( RXSIOSAY_PARM * )parmblock ;
      SendDlgItemMessage (gHwnd, DID_LISTBOX, LB_ADDSTRING, 0,
                          (LONG)sparm->rxsio_string.strptr);
      break;
   case RXSIOTRC:                      /* write line to standard     */
                                       /* error stream for trace or  */
                                       /* error messages             */
      tparm = ( RXSIOTRC_PARM * )parmblock ;
      SendDlgItemMessage (gHwnd, DID_LISTBOX, LB_ADDSTRING, 0,
                          (LONG)tparm->rxsio_string.strptr);
      break;
   case RXSIOTRD:                      /* read line from standard    */
                                       /* input stream (PULL)        */
   case RXSIODTR:                      /* read line from standard    */
   default:                            /* input stream for           */
      break;                           /* interactive debug          */
   } /* endswitch */

   return RXEXIT_HANDLED;              /* successfully handled       */

}
