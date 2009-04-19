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
#include "oovutil.h"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>


BOOL DialogInAdminTable(DIALOGADMIN * Dlg)
{
    register INT i;
    for ( i = 0; i < StoredDialogs; i++ )
    {
        if ( DialogTab[i] == Dlg )
        {
           break;
        }
    }
    return(i < StoredDialogs);
}

void rxstrlcpy(CHAR * tar, CONSTRXSTRING &src)
{
   register UINT i;
   for (i=0; (i<src.strlength) && (i<STR_BUFFER-1);i++) tar[i] = src.strptr[i];
   tar[i] = '\0';
}

void rxdatacpy(CHAR * tar, RXSTRING &src)
{
   register UINT i;
   for (i=0; (i<src.strlength) && (i<DATA_BUFFER-1);i++) tar[i] = src.strptr[i];
   tar[i] = '\0';
}

bool IsYes(const char * s)
{
   if (!strlen(s)) return FALSE;

   return ((s[0]=='j') || (s[0]=='J') || (s[0]=='y') || (s[0]=='Y') || atoi(s));
}

/* Slightly stricter than IsYes and not currently exported. */
bool IsNo(const char * s)
{
   return ( s && (*s == 'N' || *s == 'n') );
}

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
      else if ( ! IsYes(argv[1].strptr) )
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

