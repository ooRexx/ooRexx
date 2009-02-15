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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>
#include <rexx.h>

/*********************************************************************/
/* Numeric Return calls                                              */
/*********************************************************************/

#define  INVALID_ROUTINE 40            /* Raise Rexx error           */
#define  VALID_ROUTINE    0            /* Successful completion      */


/*********************************************************************/
/* ApiFncTable                                                      */
/*   Array of names of the REXXApi functions.                       */
/*   This list is used for registration and deregistration.          */
/*********************************************************************/
static PSZ  ApiFncTable[] =
   {
      "Api_Exchange_Data",
      "ApiDeregFunc",
      "Api_Fill_REXX_Variable_Pool"
   };


/*************************************************************************
* Function:  ApiLoadFuncs                                                *
*                                                                        *
* Syntax:    call ApiLoadFuncs                                           *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

LONG REXXENTRY ApiLoadFuncs(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  INT    entries;                      /* Num of entries             */
  INT    j;                            /* Counter                    */


  entries = sizeof(ApiFncTable)/sizeof(PSZ);

  for (j = 0; j < entries; j++)
  {
    RexxRegisterFunctionDll(ApiFncTable[j],
          "REXXAPI2", ApiFncTable[j]);
  }
  return VALID_ROUTINE;
}


/*************************************************************************
* Function:  ApiDeregFunc                                                *
*                                                                        *
* Syntax:    call ApiDeregFuncs                                          *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

LONG REXXENTRY ApiDeregFunc(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  INT    entries;                      /* Num of entries             */
  INT    j;                            /* Counter                    */

  retstr->strlength = 0;               /* set return value           */

  if (numargs > 0)
    return INVALID_ROUTINE;


  entries = sizeof(ApiFncTable)/sizeof(PSZ);

  for (j = 0; j < entries; j++)
  {
    RexxDeregisterFunction(ApiFncTable[j]);
  }
  return VALID_ROUTINE;
  RexxDeregisterFunction("ApiLoadFuncs");
}



/*************************************************************************
* Function:  ApiRead                                                     *
*                                                                        *
* Syntax:    call Api_Fill_REXX_Variable_Pool                            *
*                                                                        *
* Params:    outbuf - variable that will be filled with data             *
* Return:    0 - success, 1 - failure                                    *
*************************************************************************/

LONG REXXENTRY Api_Fill_REXX_Variable_Pool(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  char   *outbuf = NULL;
  char   *ptr;
  int    i;
  SHVBLOCK shvb;

  /* we expect 1 argument */

  if (numargs != 1 )                    /* validate arg count         */
  {
    strcpy(retstr->strptr, "Api_Fill_REXX_Variable_Pool expects 1 arguments");
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;
  }

  /* preset the return value */

  strcpy(retstr->strptr, "0");
  retstr->strlength = 1;

  outbuf = malloc(300);
  if (outbuf == NULL)
  {
    strcpy(retstr->strptr, "Memory allocation error in Api_Fill_REXX_Variable_Pool!");
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;
  }

  ptr = outbuf;
  for (i=0; i<300; i++)
  {
     *ptr = i % 256;
     ptr++;
  }

/* Create a new variable in the REXX Variable Pool ****************/

  shvb.shvnext = NULL;
  shvb.shvname.strptr = args[0].strptr;
  shvb.shvname.strlength = strlen(args[0].strptr);
  shvb.shvnamelen = shvb.shvname.strlength;
  shvb.shvvalue.strptr = outbuf;
  shvb.shvvalue.strlength = 300;
  shvb.shvvaluelen = 300;
  shvb.shvcode = RXSHV_SYSET;
  shvb.shvret = 0;
  if (RexxVariablePool(&shvb) == RXSHV_BADN)
  {
    free(outbuf);
    strcpy(retstr->strptr, "1");
    return INVALID_ROUTINE;
  }

  free(outbuf);
  return VALID_ROUTINE;
}
