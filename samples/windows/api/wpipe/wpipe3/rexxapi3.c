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
/* ApiFncTable                                                       */
/*   Array of names of the REXXApi functions.                        */
/*   This list is used for registration and deregistration.          */
/*********************************************************************/
static PSZ  ApiFncTable[] =
   {
      "Api_Exchange_Data",
      "ApiDeregFunc",
      "Api_Read_All_Variables_From_REXX_VP",
      "Api_Read_All_Elements_Of_A_Specific_Stem_From_REXX_VP"
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
          "REXXAPI3", ApiFncTable[j]);
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
* Function:  Api_Read_All_Variables_From_REXX_VP                         *
*                                                                        *
* Syntax:    call Api_Read_All_Variables_From_REXX_VP                    *
*                                                                        *
* Params:    No parameter required. The function is called by Apitest3.  *
*            It uses a while loop to read all the variables in the active*
*            REXX-variable pool. The shared variable block request code  *
*            is RXSHV_NEXTV. Be aware that with this request code REXX   *
*            treads every Stem variable as a variable itself (not the    *
*            whole stem). This gives the calling C-routine a chance      *
*            to clear up memory which was previously allocated by REXX   *
*            for every returned variable. If you don't free memory, the  *
*            system will run out of storage.                             *
*            Be aware that the returned variables are NOT in any spe-    *
*            cified order.                                               *
*************************************************************************/

LONG REXXENTRY Api_Read_All_Variables_From_REXX_VP(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
   SHVBLOCK rxshv;
   SHVBLOCK *prxshv = &rxshv;
   RexxReturnCode rc;
   int i = 1;

   strcpy(retstr->strptr, "0");
   retstr->strlength = strlen(retstr->strptr);

   prxshv->shvnext = NULL;
   prxshv->shvname.strlength = 0;
   prxshv->shvname.strptr = NULL;  /* let rexx allocate it for me */
   prxshv->shvvalue.strptr = NULL; /* let rexx allocate it for me */
   /* Another way would be to assign an existing buffer and specify its max. length */
   prxshv->shvcode = RXSHV_NEXTV;

/* Now reading all variables from the REXX-variable pool ***********/

   rc = RexxVariablePool(prxshv);
   if (rc)
   {
      if (rc != RXSHV_LVAR)
      {
        printf("ERROR: shvret is %x hex after var nr. %d \n",rc,i);
        return INVALID_ROUTINE;
      }
   }

   if (prxshv->shvvalue.strlength)
       printf("Name of the variable from the Variable Pool: %s, Value: %s \n", prxshv->shvname.strptr, prxshv->shvvalue.strptr);
   else
       printf("Name of the variable from the Variable Pool: %s, Empty\n", prxshv->shvname.strptr);
   i++;
   GlobalFree(prxshv->shvname.strptr);  /* free pointers allocated by REXX */
   GlobalFree(prxshv->shvvalue.strptr); /* free pointers allocated by REXX */

   while (!prxshv->shvret)
   {
      prxshv->shvret = 0;
      prxshv->shvnext = NULL;
      prxshv->shvname.strlength = 0;
      prxshv->shvname.strptr = NULL;  /* let rexx allocate it for me */
      prxshv->shvvalue.strptr = NULL; /* let rexx allocate it for me */
      prxshv->shvcode = RXSHV_NEXTV;
      rc = RexxVariablePool(prxshv);
      if (rc)
      {
         if (rc== RXSHV_MEMFL)
         {
           strcpy(retstr->strptr, "Allocation error occured");
           retstr->strlength = strlen(retstr->strptr);
           return VALID_ROUTINE;
         }
         else if (rc != RXSHV_LVAR)
         {
           printf("ERROR: shvret is %x hex after var nr. %d\n",rc,i);
           return INVALID_ROUTINE;
         }
      }
      i++;
      if (!prxshv->shvret)
      {
          if (prxshv->shvvalue.strlength)
              printf("Name of the variable from the Variable Pool: %s, Value: %s \n", prxshv->shvname.strptr, prxshv->shvvalue.strptr);
          else
              printf("Name of the variable from the Variable Pool: %s, Empty\n", prxshv->shvname.strptr);
          GlobalFree(prxshv->shvname.strptr);  /* free pointers allocated by REXX */
          GlobalFree(prxshv->shvvalue.strptr); /* free pointers allocated by REXX */
      }
   }
  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  Api_Read_All_Elements_Of_A_Specific_Stem_From_REXX_VP       *
*                                                                        *
* Syntax:    call Api_Read_All_Elements_Of_A_Specific_Stem_From_REXX_VP  *
*            with the stem variable the values should be returned        *
*                                                                        *
* Params:    A stem where all values of the stem variables should be     *
*            returned.                                                   *
*            The shared variable block request code  is RXSHV_SYFET.     *
*            Only ONE call is necessary                                  *
*                                                                        *
*************************************************************************/

LONG REXXENTRY Api_Read_All_Elements_Of_A_Specific_Stem_From_REXX_VP(
  PSZ       name,                      /* Function name              */
  LONG      numargs,                   /* Number of arguments        */
  RXSTRING  args[],                    /* Argument array             */
  PSZ       queuename,                 /* Current queue              */
  PRXSTRING retstr )                   /* Return RXSTRING            */
{
   SHVBLOCK rxshv;
   SHVBLOCK *prxshv = &rxshv, *temp, *interim;
   RexxReturnCode rc;
   char array[20], value[10];
   char pch[64], *result;

   int chars;
   int i, j, k = 0;

   if (numargs != 1 )                    /* validate arg count         */
    return INVALID_ROUTINE;
   strcpy(retstr->strptr, "0");
   retstr->strlength = strlen(retstr->strptr);

   strncpy(pch, args[0].strptr, 64);

   prxshv->shvnext = NULL;
   prxshv->shvname.strptr = pch;   /* here we use our own buffer that is limited to 64 characters */
   prxshv->shvname.strlength = strlen(pch);
   prxshv->shvvalue.strptr = NULL; /* let rexx allocate for me */
   prxshv->shvcode = RXSHV_SYFET;

   rc = RexxVariablePool(prxshv);
   if (rc)
   {
      strcpy(retstr->strptr, "ApiFETCH failed \n");
      retstr->strlength = strlen(retstr->strptr);
      return VALID_ROUTINE;
   }

   j = atoi(prxshv->shvvalue.strptr);

   chars = '.';
   result = strrchr(pch, chars);
   result++;
   *result = 0x00;

   temp = prxshv;

   memset(array, 0x00, sizeof(array));
   memset(value, 0x00, sizeof(value));
   for (k = 1;k <= j; k++)
   {
      temp->shvnext = (PSHVBLOCK)malloc(sizeof(SHVBLOCK));  /* allocate a new node */
      temp = temp->shvnext;
      if (!temp)
      {
         strcpy(retstr->strptr, "Allocation error occured");
         retstr->strlength = strlen(retstr->strptr);
         return VALID_ROUTINE;
      }
      strcpy(array, pch);
      sprintf(value, "%d", k);
      strcat(array, value);
      temp->shvnext = NULL;
      temp->shvname.strlength = strlen(array);
      temp->shvname.strptr = malloc(strlen(array)+1);
      strcpy(temp->shvname.strptr, array);
      temp->shvvalue.strptr = NULL; /* let rexx allocate it for me */
      temp->shvcode = RXSHV_SYFET;
   }

   temp = prxshv->shvnext;   /* first allocated one */
   rc = RexxVariablePool(temp);
   if (rc)
   {
      if (rc== RXSHV_MEMFL)
      {
         strcpy(retstr->strptr, "Allocation error occured");
         retstr->strlength = strlen(retstr->strptr);
         return VALID_ROUTINE;
      }
      else if (rc != RXSHV_LVAR)
      {
         printf("ERROR: shvret is %x hex after var nr. %d\n",rc,i);
         return INVALID_ROUTINE;
      }
   }

   for (k = 1;k <= j; k++)
   {
      printf("Name of the Stem-variable from the Variable Pool: %s, Value: %s \n", temp->shvname.strptr,temp->shvvalue.strptr);
      free(temp->shvname.strptr);   /* allocated by us and therefore freed with free */
      GlobalFree(temp->shvvalue.strptr);  /* allocated by REXX and therefore freed by GlobalFree */
      interim = temp;
      temp = temp->shvnext;  /* process next in list */
      free(interim);         /* free current node */
   }
   GlobalFree(prxshv->shvvalue.strptr);  /* allocated by REXX and freed by GlobalFree */

   return VALID_ROUTINE;
}

