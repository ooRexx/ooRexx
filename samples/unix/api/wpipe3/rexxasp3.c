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
/***************************************************************************/
/*                                                                         */
/*  rexxasp3.c          Open Object Rexx samples                           */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  Description:       functions used by Open Object Rexx script           */
/*                                                                         */
/***************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <rexx.h>

/*********************************************************************/
/* Numeric Return calls                                              */
/*********************************************************************/

#define  INVALID_ROUTINE 40            /* Raise Rexx error           */
#define  VALID_ROUTINE    0            /* Successful completion      */

#ifdef __cplusplus
extern "C" {
#endif


/*********************************************************************/
/* AspiFncTable                                                      */
/*   Array of names of the REXXASPI functions.                       */
/*   This list is used for registration and deregistration.          */
/*********************************************************************/
static const char *AspiFncTable[] =
   {
      "Aspi_Exchange_Data",
      "AspiDeregFunc3",
      "Aspi_Read_All_Variables_From_REXX_VP",
      "Aspi_Read_All_Elements_Of_A_Specific_Stem_From_REXX_VP"
   };


/*************************************************************************
* Function:  AspiLoadFuncs                                               *
*                                                                        *
* Syntax:    call AspiLoadFuncs                                          *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

RexxReturnCode REXXENTRY AspiLoadFuncs3(
    const char *name,                    /* Function name              */
    size_t numargs,                      /* Number of arguments        */
    CONSTRXSTRING args[],                /* Argument array             */
    const char * queuename,              /* Current queue              */
    PRXSTRING retstr )                   /* Return RXSTRING            */
{
  int    entries;                      /* Num of entries             */
  int    j;                            /* Counter                    */


  entries = sizeof(AspiFncTable)/sizeof(const char *);

  for (j = 0; j < entries; j++)
  {
    RexxRegisterFunctionDll(AspiFncTable[j], "rexxasp3", AspiFncTable[j]);
  }
  return VALID_ROUTINE;
}


/*************************************************************************
* Function:  AspiDeregFunc                                               *
*                                                                        *
* Syntax:    call AspiDeregFuncs                                         *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

RexxReturnCode REXXENTRY AspiDeregFunc3(
    const char *name,                    /* Function name              */
    size_t numargs,                      /* Number of arguments        */
    CONSTRXSTRING args[],                /* Argument array             */
    const char * queuename,              /* Current queue              */
    PRXSTRING retstr )                   /* Return RXSTRING            */
{
  int    entries;                      /* Num of entries             */
  int    j;                            /* Counter                    */

  retstr->strlength = 0;               /* set return value           */

  if (numargs > 0)
    return INVALID_ROUTINE;


  entries = sizeof(AspiFncTable)/sizeof(const char *);

  for (j = 0; j < entries; j++)
  {
    RexxDeregisterFunction(AspiFncTable[j]);
  }
  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  Aspi_Read_All_Variables_From_REXX_VP                        *
*                                                                        *
* Syntax:    call Aspi_Read_All_Variables_From_REXX_VP                   *
*                                                                        *
* Params:    No parameter required. The function is called by ASPITEST.  *
*            It uses a while loop to read all the variables in the active*
*            REXX-variable pool. The shared variable block request code  *
*            is RXSHV_NEXTV. Be aware that with this request code REXX   *
*            treads every Stem variable as a variable itself (not the    *
*            whole stem). This gives the calling C-routine a chance      *
*            to clear up memory which was previously allocated by REXX   *
*            for every returned variable. If you don't free memory the   *
*            system will get out of storage.                             *
*            Be aware that the returned variables are NOT in any spe-    *
*            cified order.                                               *
* Return:    0 - success, 1 - failure                                    *
*************************************************************************/

RexxReturnCode REXXENTRY Aspi_Read_All_Variables_From_REXX_VP(
    const char *name,                    /* Function name              */
    size_t numargs,                      /* Number of arguments        */
    CONSTRXSTRING args[],                /* Argument array             */
    const char * queuename,              /* Current queue              */
    PRXSTRING retstr )                   /* Return RXSTRING            */
{
   SHVBLOCK *prxshv;
   RexxReturnCode rc;
   int i = 1;

   strcpy(retstr->strptr, "0");
   retstr->strlength = strlen(retstr->strptr);

   prxshv = (PSHVBLOCK)malloc(sizeof(SHVBLOCK));
   if (!prxshv)
   {
      strcpy(retstr->strptr, "Allocation error occured");
      retstr->strlength = strlen(retstr->strptr);
      return VALID_ROUTINE;
   }
   prxshv->shvnext = NULL;
   prxshv->shvname.strlength = 0;
   prxshv->shvname.strptr = NULL;
   prxshv->shvvalue.strptr = NULL; /*** let Rexx allocate for me ***/
   prxshv->shvcode = RXSHV_NEXTV;

/* Now reading all variables from the Rexx-variable pool ***********/

   rc = RexxVariablePool(prxshv);
   if (rc)
   {
      if (rc != RXSHV_LVAR)
      {
      printf("ERROR: shvret is %x hex after var nr. %d \n",rc,i);
      }
   }

   printf("Name of the variable from the Variable Pool: %s, Value: %s \n", prxshv->shvname.strptr, prxshv->shvvalue.strptr);
   i++;

   while (!prxshv->shvret)
   {

      prxshv->shvnext = (PSHVBLOCK)malloc(sizeof(SHVBLOCK));
      prxshv = prxshv->shvnext;
      if (!prxshv)
      {
         strcpy(retstr->strptr, "Allocation error occured");
         retstr->strlength = strlen(retstr->strptr);
         return VALID_ROUTINE;
      }
      prxshv->shvnext = NULL;
      prxshv->shvname.strlength = 0;
      prxshv->shvname.strptr = NULL;
      prxshv->shvvalue.strptr = NULL; /*** let Rexx allocate for me ***/
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
      printf("Name of the variable from the Variable Pool: %s, Value: %s \n", prxshv->shvname.strptr, prxshv->shvvalue.strptr);
      RexxFreeMemory((void *)prxshv->shvname.strptr);
      RexxFreeMemory((void *)prxshv->shvvalue.strptr);
   }
  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  Aspi_Read_All_Elements_Of_A_Specific_Stem_From_REXX_VP      *
*                                                                        *
* Syntax:    call Aspi_Read_All_Elements_Of_A_Specific_Stem_From_REXX_VP *
*            with the stem variable the values should be returned        *
*                                                                        *
* Params:    A stem where all values of the stem variables should be     *
             returned.                                                   *
*            The shared variable block request code  is RXSHV_SYFET.     *
*            Only ONE call is necessary. If the stem contains to many    *
*            variables the memory resources are exhausted and Rexx       *
*            should return RXSHV_MEMFL. There is no change for the       *
*            calling routine to handle the problem with DosFreeMem       *
*            because it doesn't get any control before Rexx terminates.  *
*            The problem is during allocating memory for the values of   *
*            the stem-variables by DosAllocMem which allocates on 64k    *
*            boundary.                                                   *
* Return:    0 - success, 1 - failure                                    *
*************************************************************************/

RexxReturnCode REXXENTRY Aspi_Read_All_Elements_Of_A_Specific_Stem_From_REXX_VP(
    const char *name,                    /* Function name              */
    size_t numargs,                      /* Number of arguments        */
    CONSTRXSTRING args[],                /* Argument array             */
    const char * queuename,              /* Current queue              */
    PRXSTRING retstr )                   /* Return RXSTRING            */
{
   SHVBLOCK *prxshv, *temp, *interim, rxshv;
   RexxReturnCode rc;
   char array[20], value[10];
   char *pch, *result;
   int chars;
   int j, k = 0;
   prxshv = &rxshv;

   if (numargs != 1 )                    /* validate arg count         */
    return INVALID_ROUTINE;
   strcpy(retstr->strptr, "0");
   retstr->strlength = strlen(retstr->strptr);

   pch = (char *) malloc(strlen(args[0].strptr) +1);
   strcpy(pch, args[0].strptr);

   prxshv->shvnext = NULL;
   prxshv->shvname.strlength = strlen(pch);
   prxshv->shvname.strptr = pch;
   prxshv->shvvalue.strptr = NULL; /*** let Rexx allocate for me ***/
   prxshv->shvcode = RXSHV_SYFET;

   rc = RexxVariablePool(prxshv);
   if (rc)
   {
      strcpy(retstr->strptr, "ASPIFETCH failed \n");
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
      temp->shvnext = (PSHVBLOCK)malloc(sizeof(SHVBLOCK));
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
      temp->shvname.strptr = (char *) malloc(strlen(array)+1);
      strcpy(temp->shvname.strptr, array);
      temp->shvvalue.strptr = NULL; /*** let Rexx allocate for me ***/
      temp->shvcode = RXSHV_SYFET;
   }
   rc = RexxVariablePool(prxshv->shvnext);
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
         printf("ERROR: shvret is %x hex \n",rc);
         return INVALID_ROUTINE;
      }
   }
   temp = prxshv->shvnext;

   for (k = 1;k <= j; k++)
   {
      printf("Name of the Stem-variable from the Rexx Variable Pool: %s, Value: %s \n", temp->shvname.strptr, temp->shvvalue.strptr);
      interim = temp;
      RexxFreeMemory((void *)temp->shvname.strptr);
      RexxFreeMemory((void *)temp->shvvalue.strptr);
      temp = temp->shvnext;
      free(interim);
   }

   RexxFreeMemory((void *)prxshv->shvvalue.strptr);
   free(pch);

   return VALID_ROUTINE;
}

#ifdef __cplusplus
}
#endif

