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

/**
 * oodCommon.cpp
 *
 * Contains convenience / helper functions used throughout the ooDialog modules.
 */

#include "oovutil.h"     // Must be first, includes windows.h and oorexxapi.h

#include <stdio.h>
#include <dlgs.h>
#include <malloc.h>
#include "APICommon.h"
#include "oodCommon.h"


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

/**
 * Validates and converts an ASCII-Z string from string form to a pointer value.
 *
 * @param string  String to convert.
 */
void *string2pointer(const char *string)
{
    void *pointer = 0;
    sscanf(string, "0x%p", &pointer);
    return pointer;
}

void pointer2string(char *result, void *pointer)
{
    if ( pointer == NULL )
    {
        sprintf(result, "0");
    }
    else
    {
        sprintf(result, "0x%p", pointer);
    }
}


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

/* Slightly stricter than IsYes. TODO remove this when YesNoMessage() is
   fixed. */
bool IsNo(const char * s)
{
   return ( s && (*s == 'N' || *s == 'n') );
}

DIALOGADMIN *rxGetDlgAdm(RexxMethodContext *context, RexxObjectPtr dlg)
{
    DIALOGADMIN *adm = (DIALOGADMIN *)rxGetPointerAttribute(context, dlg, "ADM");
    if ( adm == NULL )
    {
         // Want this message: Could not retrieve the "value" information for "object"
         // similar to old 98.921

        TCHAR buf[128];
        RexxObjectPtr name = context->SendMessage0(dlg, "OBJECTNAME");
        _snprintf(buf, sizeof(buf), "Could not retrieve the dialog administration block information for %s",
                  context->ObjectToStringValue(name));

        context->RaiseException1(Rexx_Error_Execution_user_defined, context->String(buf));
    }
    return adm;
}


// TODO move to APICommon when ooDialog is converted to use .Pointer instead of
// pointer strings.
POINTER rxGetPointerAttribute(RexxMethodContext *context, RexxObjectPtr obj, CSTRING name)
{
    CSTRING value = "";
    RexxObjectPtr rxString = context->SendMessage0(obj, name);
    if ( rxString != NULLOBJECT )
    {
        value = context->ObjectToStringValue(rxString);
    }
    return string2pointer(value);
}

