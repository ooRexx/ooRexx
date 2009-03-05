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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include <rexx.h>

/*********************************************************************/
/* Numeric Return calls                                              */
/*********************************************************************/

#define  INVALID_ROUTINE 40            /* Raise Rexx error           */
#define  VALID_ROUTINE    0            /* Successful completion      */


/*********************************************************************/
/* ApiFncTable                                                       */
/*   Array of names of the REXXASP1 functions.                       */
/*   This list is used for registration and deregistration.          */
/*********************************************************************/
static PSZ  ApiFncTable[] =
   {
      "Api_Output_From_C",
      "Api_Output_From_REXX",
      "Api_Exchange_Data",
      "ApiDeregFunc"
   };


/*************************************************************************
* Function:  ApiLoadFuncs                                               *
*                                                                        *
* Syntax:    call ApiLoadFuncs                                          *
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
          "REXXAPI1", ApiFncTable[j]);
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
  RexxDeregisterFunction("ApiLoadFuncs");
  return VALID_ROUTINE;
}



/*************************************************************************
* Function:  Api_Output_From_C                                           *
*                                                                        *
* Syntax:    call Api_Output_From_C                                      *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    Version of this API support DLL                             *
*************************************************************************/

LONG REXXENTRY Api_Output_From_C(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{

  if (numargs > 0)
  {
    strcpy(retstr->strptr, "Api_OutPut_From_C does not support any arguments");
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;
  }

  printf("This output was generated and displayed by the C-function Api_Output_From_C\n");
  fflush(NULL);
  return VALID_ROUTINE;
}


/*************************************************************************
* Function:  Api_Output_From_REXX                                        *
*                                                                        *
* Syntax:    call Api_Output_From_REXX                                   *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    Version of this API support DLL                             *
*************************************************************************/

LONG REXXENTRY Api_Output_From_REXX(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  if (numargs > 0)
  {
    strcpy(retstr->strptr, "Api_OutPut_From_REXX does not support any arguments");
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;
  }

  strcpy(retstr->strptr, "This output was returned from the C-function Api_Output_From_REXX and is displayed by REXX");
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  Api_Exchange_Data                                           *
*                                                                        *
* Syntax:    call Api_Exchange_Data startsect, numsects, outbuf          *
*                                                                        *
* Params:    parm1 - numeric value given by REXX                         *
*            parm2 - numeric value given by REXX                         *
*            outbuf - variable given by REXX                             *
*                                                                        *
* Return:    0 - success, 1 - failure                                    *
*************************************************************************/

LONG REXXENTRY Api_Exchange_Data(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
  int    parm1 = 0;
  int    parm2 = 0;
  char   outbuf[255];

  /* we expect 3 arguments */

  if (numargs != 3 )
  {
    strcpy(retstr->strptr, "Api_Exchange_Data function expects 3 arguments");
    retstr->strlength = strlen(retstr->strptr);
    return VALID_ROUTINE;
  }

  strcpy(retstr->strptr, "\0");
  retstr->strlength = strlen(retstr->strptr);

  /* read the values from the parameters */

  parm1 = atoi(args[0].strptr);
  parm2 = atoi(args[1].strptr);
  strcpy(outbuf, args[2].strptr);

  printf("Api_Exchange_Data function has received the following arguments: Argument 1: %d, Argument 2: %d, Argument 3 %s\n",
         parm1, parm2, outbuf);
  fflush(NULL);

  return VALID_ROUTINE;

}
