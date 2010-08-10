/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2009-2010 Rexx Language Association. All rights reserved.    */
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
/* Authors;                                                                   */
/*       W. David Ashley <dashley@us.ibm.com>                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <windows.h>
#include <rexx.h>
#include <oorexxapi.h>

#include "../../hostemu.h"


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Global variables                                                   */
/*                                                                    */
/*--------------------------------------------------------------------*/

// #define HOSTEMU_DEBUG

PCONSTRXSTRING prxCmd = NULL;
EXECIO_OPTIONS ExecIO_Options;
long           lCmdPtr;
unsigned long  ulNumSym;
char *         pszSymbol[SYMTABLESIZE];
char           szInline[1000];
long           lStmtType;


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Local definitions                                                  */
/*                                                                    */
/*--------------------------------------------------------------------*/

typedef struct _LL
   {
   struct _LL *  prev;
   struct _LL *  next;
   char          FileName [1024];
   FILE *        pFile;
   } LL;
typedef LL * PLL;


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Local variables                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/

static HANDLE htmxExecIO = NULL;
static PLL  pHead      = NULL;
static PLL  pTail      = NULL;


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Local function prototypes                                          */
/*                                                                    */
/*--------------------------------------------------------------------*/

static unsigned long ExecIO_Write_From_Stem(
   PLL pll);                     /* Pointer to file linked list item  */
static unsigned long ExecIO_Write_From_Queue(
   PLL pll);                     /* Pointer to file linked list item  */
static unsigned long ExecIO_Read_To_Stem(
   PLL pll);                     /* Pointer to file linked list item  */
static unsigned long ExecIO_Read_To_Queue(
   PLL pll);                     /* Pointer to file linked list item  */
static PLL Search_LL(
   char * SFilename);               /* Source file name                  */
static void Insert_LL(
   PLL pll);                     /* Pointer to the new item           */
static void Delete_LL(
   PLL pll);                     /* Pointer to the item to be deleted */
static long queued(
   void);                        /* No arguments                      */
static void push(
   char * pushstr,                  /* String to be pushed onto queue    */
   long lOp);                    /* 0 = FIFO, 1 = LIFO                */
static char * pull(
   void);                        /* No arguments                      */

/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    FetchRexxVar()                                        */
/*                                                                    */
/* Description: Fetch contents of a REXX variable from the current    */
/*              variable pool. The caller is responsible for freeing  */
/*              the buffer pointed to by the return value via         */
/*              DosFreeMem().                                         */
/*                                                                    */
/* Input:       PSZ - name of the REXX variable to be fetched         */
/*              PRXSTRING - pointer to the return RXSTRING structure  */
/*                                                                    */
/* Returns:     ULONG - return code from RexxVariablePool()           */
/*                                                                    */
/* Notes:       None.                                                 */
/*                                                                    */
/*--------------------------------------------------------------------*/

unsigned long FetchRexxVar (
   char *    pszVar,             /* Variable name                     */
   PRXSTRING prxVar)             /* REXX variable contents            */
   {

   /* local function variables */
   SHVBLOCK      RxVarBlock;
   unsigned long ulRetc;
   char *        pszTemp;

   /* initialize the shared variable block */
   RxVarBlock.shvnext = NULL;
   RxVarBlock.shvname.strptr = pszVar;
   RxVarBlock.shvname.strlength = strlen(pszVar);
   RxVarBlock.shvnamelen = RxVarBlock.shvname.strlength;
   RxVarBlock.shvvalue.strptr = NULL;
   RxVarBlock.shvvalue.strlength = 0;
   RxVarBlock.shvvaluelen = 0;
   RxVarBlock.shvcode = RXSHV_SYFET;
   RxVarBlock.shvret = RXSHV_OK;

   /* fetch variable from pool */
   ulRetc = RexxVariablePool(&RxVarBlock);

   /* test return code */
   if (ulRetc != RXSHV_OK && ulRetc != RXSHV_NEWV) {
      prxVar -> strptr = NULL;
      prxVar -> strlength = 0;
      }
   else {
      /* allocate a new buffer for the Rexx variable pool value */
      pszTemp = (char *) RexxAllocateMemory(RxVarBlock.shvvalue.strlength + 1);
      if (pszTemp == NULL) {
         /* no buffer available so return a NULL Rexx value */
         prxVar -> strptr = NULL;
         prxVar -> strlength = 0;
         ulRetc = RXSHV_MEMFL;
         }
      else {
         /* copy to new buffer and zero-terminate */
         memmove(pszTemp, RxVarBlock.shvvalue.strptr,
                  RxVarBlock.shvvalue.strlength);
         *(pszTemp + RxVarBlock.shvvalue.strlength) = '\0';
         prxVar -> strptr = pszTemp;
         prxVar -> strlength = RxVarBlock.shvvalue.strlength;
         }
      // free memory returned from RexxVariablePool API
      RexxFreeMemory(RxVarBlock.shvvalue.strptr);
      }

   return ulRetc;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    SetRexxVar()                                          */
/*                                                                    */
/* Description: Sets the contents of a variable in the REXX variable  */
/*              pool.                                                 */
/*                                                                    */
/* Input:       PSZ - name of the REXX variable to be set             */
/*              PVOID - pointer to new contents for variable          */
/*              ULONG - buffer size of new contents                   */
/*                                                                    */
/* Returns:     ULONG - return code from RexxVariablePool()           */
/*                                                                    */
/* Notes:       None.                                                 */
/*                                                                    */
/*--------------------------------------------------------------------*/

unsigned long SetRexxVar (
   char *        pszVar,         /* Variable name to be set           */
   char *        pValue,         /* Ptr to new value                  */
   size_t        ulLen)          /* Value length                      */
   {

   /* local function data */
   SHVBLOCK      RxVarBlock;
   unsigned long ulRetc;

   /* initialize RxVarBlock */
   RxVarBlock.shvnext = NULL;
   RxVarBlock.shvname.strptr = pszVar;
   RxVarBlock.shvname.strlength = strlen(pszVar);
   RxVarBlock.shvnamelen = RxVarBlock.shvname.strlength;
   RxVarBlock.shvvalue.strptr = pValue;
   RxVarBlock.shvvalue.strlength = ulLen;
   RxVarBlock.shvvaluelen = ulLen;
   RxVarBlock.shvcode = RXSHV_SYSET;
   RxVarBlock.shvret = RXSHV_OK;

   /* set variable in pool */
   ulRetc = RexxVariablePool(&RxVarBlock);

   /* test return code */
   if (ulRetc == RXSHV_NEWV) {
      ulRetc = RXSHV_OK;
      }

   return ulRetc;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    GrxHost()                                             */
/*                                                                    */
/* Description: Emulates the IBM host environment commands.           */
/*                                                                    */
/* Input:       Command string                                        */
/*              Pointer to return flags                               */
/*              Pointer to return string                              */
/*                                                                    */
/* Returns:     Return indicating success or failure                  */
/*                                                                    */
/* References:  None.                                                 */
/*                                                                    */
/* Notes:                                                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

RexxReturnCode GrxHost(PCONSTRXSTRING command,
                       unsigned short int *flags,
                       PRXSTRING retc)
   {

   /* Local function variables */
   unsigned long i, rc = 0;
   PLL pll;

   #ifdef HOSTEMU_DEBUG
   printf("HOSTEMU: Subcom called.\n");
   #endif

   /* request the semaphore so we can get exclusive access to         */
   /* our variables                                                   */
   WaitForSingleObject(htmxExecIO, INFINITE);

   /* initialize the global variables */
   memset(&ExecIO_Options, '\0', sizeof(EXECIO_OPTIONS));
   ExecIO_Options.lStartRcd = 1;
   prxCmd = command;
   lCmdPtr = 0;
   ulNumSym = 0;
   *flags = RXSUBCOM_OK;

   /* parse the command */
   if (!yyparse ()) {
      #ifdef HOSTEMU_DEBUG
      printf("HOSTEMU: Parse complete.\n");
      #endif
      if (lStmtType == HI_STMT) {
         RexxSetHalt(GetCurrentProcessId(), GetCurrentThreadId());
         }
      else if (lStmtType == TE_STMT) {
         RexxResetTrace(GetCurrentProcessId(), GetCurrentThreadId());
         }
      else if (lStmtType == TS_STMT) {
         RexxSetTrace(GetCurrentProcessId(), GetCurrentThreadId());
         }
      else if (lStmtType == EXECIO_STMT) {
         #ifdef HOSTEMU_DEBUG
         printf("HOSTEMU: Executing execio statement.\n");
         #endif
         /* check to see if the file is already open */
         pll = Search_LL(ExecIO_Options.aFilename);
         if (pll == NULL) {
            /* it is a new file, so open it and add to the list */
            pll = (PLL)malloc(sizeof (LL));
            if (pll == NULL) {
               rc = 20;
               *flags = RXSUBCOM_FAILURE;
               goto return_point;
               }
            memset(pll, '\0', sizeof (LL));
            strcpy(pll -> FileName, ExecIO_Options.aFilename);

            /* try to open an existing file */
            pll -> pFile = fopen(pll -> FileName, "r+");

            /* read only files can not be opened in read / write mode (r+) */
            if ( pll -> pFile == NULL && ! ExecIO_Options.fRW ) {
               pll -> pFile = fopen(pll -> FileName, "r");
               }
            if (pll -> pFile == NULL) {
               /* no existing file, so open a new file */
               pll -> pFile = fopen(pll -> FileName, "w+");
               }
            if (pll -> pFile == NULL) {
               /* nothing could be opened so return an error */
               free(pll);
               rc = 20;
               *flags = RXSUBCOM_FAILURE;
               goto return_point;
               }
            Insert_LL(pll);
            }
         /* is this a read or write operation? */
         if (ExecIO_Options.fRW) {
            /* DISKW */
            /* is this a stem or queue operation? */
            if (strlen (ExecIO_Options.aStem)) {
               rc = ExecIO_Write_From_Stem(pll);
               }
            else {
               rc = ExecIO_Write_From_Queue(pll);
               }
            }
         else {
            /* DISKR */
            /* is this a stem or queue operation? */
            if (strlen(ExecIO_Options.aStem)) {
               rc = ExecIO_Read_To_Stem(pll);
               }
            else {
               rc = ExecIO_Read_To_Queue(pll);
               }
            }
         /* process the FINIS option */
         if (ExecIO_Options.fFinis) {
            fclose(pll -> pFile);
            Delete_LL(pll);
            }
         /* if the return code is 20 then set the failure flag */
         if (rc == 20) {
            *flags = RXSUBCOM_FAILURE;
            }
         }
      else { /* bad statement type */
         *flags = RXSUBCOM_ERROR;
         rc = 20;
         }
      }
   else { /* parse failed */
      *flags = RXSUBCOM_ERROR;
      rc = 20;
      }

   return_point:

   /* release our symbol table memory */
   if (ulNumSym != 0) {
      for (i = 0; i < ulNumSym; i++) {
         free(pszSymbol[i]);
         }
      }

   ReleaseMutex(htmxExecIO);

   sprintf(retc->strptr, "%u", rc);
   retc->strlength = strlen(retc->strptr);
   #ifdef HOSTEMU_DEBUG
   printf("HOSTEMU: Subcom return code = %u.\n", rc);
   #endif
   return rc;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    ExecIO_Write_From_Stem                                */
/*                                                                    */
/* Description: ExecIO write from a stem to a file.                   */
/*                                                                    */
/* Input:       Pointer to file linked list item                      */
/*                                                                    */
/* Returns:     Return indicating success or failure                  */
/*                                                                    */
/* References:  None.                                                 */
/*                                                                    */
/* Notes:                                                             */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/

static unsigned long ExecIO_Write_From_Stem (
   PLL pll)                      /* Pointer to file linked list item  */
   {

   /* Local function variables */
   char *      Stem;             /* Stem variable name                */
   char *      Index;            /* Stem index value (string)         */
   RXSTRING rxVal;               /* Rexx stem variable value          */
   int      elements;

   /* process request */
   if (ExecIO_Options.lRcdCnt == 0)
      return 0;
   Stem = (char *)malloc(strlen(ExecIO_Options.aStem) + 33);
   if (Stem == NULL) {
      return 20;
      }
   strcpy(Stem, ExecIO_Options.aStem);
   Index = Stem + strlen(Stem);
   if (ExecIO_Options.lRcdCnt == -1) {
      /* process an "*" record count */
      // get the number of elements
      sprintf(Index, "%u", 0);
      FetchRexxVar(Stem, &rxVal);
      elements = atoi(rxVal.strptr);
      RexxFreeMemory(rxVal.strptr);
      while (ExecIO_Options.lStartRcd <= elements) {
         sprintf(Index, "%d", ExecIO_Options.lStartRcd);
         FetchRexxVar(Stem, &rxVal);
         fputs(rxVal.strptr, pll -> pFile);
         fputs("\n", pll -> pFile);
         RexxFreeMemory(rxVal.strptr);
         ExecIO_Options.lStartRcd++;
         }
      }
   else {
      /* process a specific record count */
      while (ExecIO_Options.lStartRcd <= ExecIO_Options.lRcdCnt) {
         sprintf(Index, "%u", ExecIO_Options.lStartRcd);
         FetchRexxVar(Stem, &rxVal);
         fputs(rxVal.strptr, pll -> pFile);
         fputs("\n", pll -> pFile);
         RexxFreeMemory(rxVal.strptr);
         ExecIO_Options.lStartRcd++;
         }
      }
   fflush (pll -> pFile);

   /* return with successful return code */
   return 0;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    ExecIO_Write_From_Queue                               */
/*                                                                    */
/* Description: ExecIO write from the queue to a file.                */
/*                                                                    */
/* Input:       Pointer to file linked list item                      */
/*                                                                    */
/* Returns:     Return indicating success or failure                  */
/*                                                                    */
/* References:  None.                                                 */
/*                                                                    */
/* Notes:                                                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

static unsigned long ExecIO_Write_From_Queue (
   PLL pll)                      /* Pointer to file linked list item  */
   {

   /* Local function variables */
   char * Item;                  /* Item pulled from the queue        */
   long items;

   /* process request */
   if (ExecIO_Options.lRcdCnt == 0) {
      return 0;
      }
   /* start at the proper place in the queue */
   while (ExecIO_Options.lStartRcd > 1 && queued() > 0) {
      Item = pull();
      if (Item != NULL) {
         RexxFreeMemory(Item);
         }
      ExecIO_Options.lStartRcd--;
      }
   if (ExecIO_Options.lRcdCnt == -1) {
      /* process an "*" record count */
      items = queued();
      while (items > 0) {
         Item = pull();
         if (Item != NULL) {
            fputs(Item, pll -> pFile);
            fputs("\n", pll -> pFile);
            RexxFreeMemory(Item);
            }
         else {
            goto return_point;
            }
         items--;
         }
      }
   else {
      /* process a specific record count */
      while (ExecIO_Options.lRcdCnt > 0) {
         if (queued() == 0)
            break;
         Item = pull();
         if (Item != NULL) {
            fputs(Item, pll -> pFile);
            fputs("\n", pll -> pFile);
            RexxFreeMemory(Item);
            }
         else {
            goto return_point;
            }
         ExecIO_Options.lRcdCnt--;
         }
      }

   return_point:
   fflush (pll -> pFile);

   /* return with successful return code */
   return 0;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    ExecIO_Read_To_Stem                                   */
/*                                                                    */
/* Description: ExecIO read from a file to a stem.                    */
/*                                                                    */
/* Input:       Pointer to file linked list item                      */
/*                                                                    */
/* Returns:     Return indicating success or failure                  */
/*                                                                    */
/* References:  None.                                                 */
/*                                                                    */
/* Notes:                                                             */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/

static unsigned long ExecIO_Read_To_Stem (
   PLL pll)                      /* Pointer to file linked list item  */
   {

   /* Local function variables */
   char *   Stem;                /* Stem variable name                */
   char *   Index;               /* Stem index value (string)         */
   unsigned long ulRc = 0;       /* Return code                       */

   /* process request */
   if (ExecIO_Options.lRcdCnt == 0) {
      return 0;
      }
   Stem = (char *)malloc(strlen(ExecIO_Options.aStem) + 33);
   if (Stem == NULL) {
      return 20;
      }
   strcpy(Stem, ExecIO_Options.aStem);
   Index = Stem + strlen(Stem);
   if (ExecIO_Options.lRcdCnt == -1) {
      /* process an "*" record count */
      while (fgets(szInline, sizeof (szInline), pll -> pFile)) {
         if (*(szInline + strlen(szInline) - 1) == '\n')
            *(szInline + strlen(szInline) - 1) = '\0';
         sprintf(Index, "%u", ExecIO_Options.lStartRcd);
         SetRexxVar(Stem, szInline, strlen(szInline));
         ExecIO_Options.lStartRcd++;
         }
      }
   else {
      /* process a specific record count */
      while (ExecIO_Options.lRcdCnt > 0) {
         if (fgets(szInline, sizeof(szInline), pll -> pFile)) {
            if (*(szInline + strlen(szInline) - 1) == '\n') {
               *(szInline + strlen(szInline) - 1) = '\0';
               }
            sprintf(Index, "%u", ExecIO_Options.lStartRcd);
            SetRexxVar(Stem, szInline, strlen(szInline));
            ExecIO_Options.lStartRcd++;
            }
         else {
            ulRc = 2;
            break;
            }
         ExecIO_Options.lRcdCnt--;
         }
      }
   ExecIO_Options.lStartRcd--;
   sprintf(szInline, "%u", ExecIO_Options.lStartRcd);
   sprintf(Index, "%u", 0);
   SetRexxVar(Stem, szInline, strlen (szInline));
   free(Stem);

   /* return with successful return code */
   return ulRc;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    ExecIO_Read_To_Queue                                  */
/*                                                                    */
/* Description: ExecIO read file to the current queue.                */
/*                                                                    */
/* Input:       Pointer to file linked list item                      */
/*                                                                    */
/* Returns:     Return indicating success or failure                  */
/*                                                                    */
/* References:  None.                                                 */
/*                                                                    */
/* Notes:                                                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

static unsigned long ExecIO_Read_To_Queue (
   PLL pll)                      /* Pointer to file linked list item  */
   {

   /* Local function variables */

   /* process request */
   if (ExecIO_Options.lRcdCnt == 0) {
      return 0;
      }
   if (ExecIO_Options.lRcdCnt == -1) {
      /* process an "*" record count */
      while (fgets (szInline, sizeof (szInline), pll -> pFile)) {
         if (*(szInline + strlen (szInline) - 1) == '\n') {
            *(szInline + strlen (szInline) - 1) = '\0';
            }
         if (ExecIO_Options.lDirection != 2) {
            push (szInline, ExecIO_Options.lDirection);
            }
         }
      }
   else {
      /* process a specific record count */
      while (ExecIO_Options.lRcdCnt > 0) {
         if (fgets (szInline, sizeof (szInline), pll -> pFile)) {
            if (*(szInline + strlen (szInline) - 1) == '\n') {
               *(szInline + strlen (szInline) - 1) = '\0';
               }
            if (ExecIO_Options.lDirection != 2) {
               push (szInline, ExecIO_Options.lDirection);
               }
            }
         else {
            return 2;
            }
         ExecIO_Options.lRcdCnt--;
         }
      }

   /* return with successful return code */
   return 0;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    Search_LL                                             */
/*                                                                    */
/* Description: Search the linked list of files for a match.          */
/*                                                                    */
/* Input:       Filename                                              */
/*                                                                    */
/* Returns:     Pointer to found struct or NULL if not found          */
/*                                                                    */
/* References:  None.                                                 */
/*                                                                    */
/* Notes:                                                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

static PLL Search_LL (
   char * SFilename)             /* Source file name                  */
   {

   /* Local function variables */
   PLL pll = pHead;

   while (pll != NULL) {
      if (!strcmp (SFilename, pll -> FileName)) {
         return pll;
         }
      pll = pll -> next;
      }
   return pll;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    Insert_LL                                             */
/*                                                                    */
/* Description: Insert a new item at the end of the list.             */
/*                                                                    */
/* Input:       Pointer to new item struct.                           */
/*                                                                    */
/* Returns:     None.                                                 */
/*                                                                    */
/* References:  None.                                                 */
/*                                                                    */
/* Notes:                                                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

static void Insert_LL (
   PLL pll)                      /* Pointer to the new item           */
   {

   if (pHead == NULL) {
      pHead = pll;
      }
   else {
      pTail -> next = pll;
      }
   pll -> prev = pTail;
   pll -> next = NULL;
   pTail = pll;
   return;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    Delete_LL                                             */
/*                                                                    */
/* Description: Delete an item from the list.                         */
/*                                                                    */
/* Input:       Pointer to item to be deleted.                        */
/*                                                                    */
/* Returns:     None.                                                 */
/*                                                                    */
/* References:  None.                                                 */
/*                                                                    */
/* Notes:                                                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

static void Delete_LL (
   PLL pll)                      /* Pointer to the item to be deleted */
   {

   if (pHead == pll) {
      pHead = pll -> next;
      }
   if (pTail == pll) {
      pTail = pll -> prev;
      }
   if (pll -> next != NULL) {
      pll -> next -> prev = pll -> prev;
      }
   if (pll -> prev != NULL) {
      pll -> prev -> next = pll -> next;
      }
   free(pll);
   return;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    queued                                                */
/*                                                                    */
/* Description: Returns the number of items in the current Rexx queue */
/*                                                                    */
/* Input:       None.                                                 */
/*                                                                    */
/* Returns:     Number of queued items                                */
/*                                                                    */
/* References:  None.                                                 */
/*                                                                    */
/* Notes:                                                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

static long queued (
   void)                         /* No arguments                      */
   {

   /* local function variables */
   size_t elements;

   RexxQueryQueue("SESSION", &elements);
   return (long)elements;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    push                                                  */
/*                                                                    */
/* Description: Push an item onto the current Rexx queue.             */
/*                                                                    */
/* Input:       Pointer to the string to be pushed                    */
/*                                                                    */
/* Returns:     Number of queued items                                */
/*                                                                    */
/* References:  None.                                                 */
/*                                                                    */
/* Notes:                                                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

static void push (
   char * pushstr,               /* String to be pushed onto queue    */
   long lOp)                     /* 0 = FIFO, 1 = LIFO                */
   {

   CONSTRXSTRING rxstr;

   rxstr.strptr = pushstr;
   rxstr.strlength = strlen(pushstr);
   RexxAddQueue("SESSION", &rxstr, (size_t)lOp);
   return;
   }


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Function:    pull                                                  */
/*                                                                    */
/* Description: Pull an item off the current Rexx queue.              */
/*                                                                    */
/* Input:       None                                                  */
/*                                                                    */
/* Returns:     Pointer to the pulled string                          */
/*                                                                    */
/* References:  None.                                                 */
/*                                                                    */
/* Notes:                                                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

static char * pull (
   void)                         /* No arguments                      */
   {

   /* local function variables */
   RXSTRING       result = {0, NULL};
   RexxReturnCode rc;

   rc = RexxPullFromQueue("SESSION", &result, NULL, RXQUEUE_WAIT);
   return result.strptr;
   }


static void RexxEntry hostemu_loader(RexxThreadContext *context) {
   RexxReturnCode rc;

   rc = RexxRegisterSubcomExe("HostEmu", (REXXPFN)GrxHost, NULL);
   htmxExecIO = CreateMutex(NULL, false, NULL);
   #ifdef HOSTEMU_DEBUG
   printf("HOSTEMU: Library loaded.\n");
   printf("HOSTEMU: RexxRegisterSubcomExe retc = %d.\n", rc);
   printf("HOSTEMU: CreateMutex htmxExecIO = %d.\n", htmxExecIO);
   #endif
   }


static void RexxEntry hostemu_unloader(RexxThreadContext *context) {
   PLL pll;

   /* close all our open files */
   pll = pHead;
   while (pll != NULL) {
      fclose (pll -> pFile);
      pll = pll -> next;
      }
   }


RexxPackageEntry hostemu_package_entry = {
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "HostEmu",                           // name of the package
    "1.0.0",                             // package information
    hostemu_loader,                      // load function
    hostemu_unloader,                    // unload function
    NULL,                                // the exported routines
    NULL                                 // the exported methods
    };

// package loading stub.
OOREXX_GET_PACKAGE(hostemu);

