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

/*********************************************************************/
/** Module Name: MISC C                                              */
/*********************************************************************/
/** Description: REXX SAA Miscellaneous Functions                    */
/*********************************************************************/
/*                                                                   */
/* Function(s):                                                      */
/*                                                                   */
/*   memicmp                 compare memory case insensitive         */
/*   stricmp                 compare string case insensitive         */
/*   memupper                uppercase a memory location             */
/*                                                                   */
/*********************************************************************/
#define INCL_RXSUBCOM
#include "rexx.h"
#include "Characters.h"
#include "CodePageTables.h"
#include "RexxAPIManager.h"


extern __declspec(dllexport) PAPIBLOCK APIsearch(PSZ, PSZ, LONG, DWORD);
APIBLOCK *exesearch(PSZ, LONG, DWORD );
APIBLOCK *dllsearch(PSZ, PSZ, LONG );
extern __declspec(dllexport) LONG APIexecheck(PSZ, LONG, DWORD );
extern __declspec(dllexport) LONG APIdllcheck(PAPIBLOCK, LONG);
extern __declspec(dllexport) LONG APIregdrop(PSZ, PSZ, LONG, DWORD );
extern __declspec(dllexport) LONG addPID(PAPIBLOCK, PID);
extern __declspec(dllexport) LONG removePID(PAPIBLOCK, PID);

#define PIDCMP(x)    (((x)->apipid==GetCurrentProcessId()))
#define  REGSUBCOMM    0

extern REXXAPIDATA * RexxinitExports;   /* Global state data  */
extern _declspec(dllexport) CRITICAL_SECTION nest; /* must complete nest count   */

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   memupper                                     */
/*                                                                   */
/*   Descriptive Name:  uppercase a memory location                  */
/*                                                                   */
/*   Entry Point:       memupper                                     */
/*                                                                   */
/*   Input:             memory to upper case                         */
/*                      length of memory location                    */
/*                                                                   */
/*********************************************************************/

VOID  memupper(location, length)
  PUCHAR   location;                   /* location to uppercase      */
  ULONG    length;                     /* length to uppercase        */
{
  for (; length--; location++)         /* loop for entire string     */
                                       /* uppercase in place         */
    *location = upper_case_table[*location];
}


/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   rxmemicmp                                    */
/*                                                                   */
/*   Descriptive Name:  case insensitive memory compare              */
/*                                                                   */
/*   Entry Point:       rxmemicmp                                    */
/*                                                                   */
/*   Input:             pointers to two memory locations             */
/*                      length of comparisons                        */
/*                                                                   */
/*********************************************************************/

INT rxmemicmp(
  PVOID     s1,                        /* first memory location      */
  PVOID     s2,                        /* second memory location     */
  UINT      n )                        /* length of comparison       */
{
  PUCHAR   s1_ptr;                     /* working pointer            */
  PUCHAR   s2_ptr;                     /* working pointer            */

  s1_ptr = (PUCHAR)s1;
  s2_ptr = (PUCHAR)s2;

  if ( n ) {
    while ( 1 ) {
      if ( ( tolower( *s1_ptr ) != tolower( *s2_ptr ) ) || !(--n) )
        break;

      ++s1_ptr;
      ++s2_ptr;
    }

    return ( (INT)tolower( *s1_ptr )  - (INT)tolower( *s2_ptr ) );
  }
  else
    return 0;
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   rxstricmp                                    */
/*                                                                   */
/*   Descriptive Name:  case insensitive string compare              */
/*                                                                   */
/*   Entry Point:       rxstricmp                                    */
/*                                                                   */
/*   Input:             pointers to two ASCII strings                */
/*                                                                   */
/*********************************************************************/
int rxstricmp(
  PSZ       s1,                        /* first string location      */
  PSZ       s2 )                       /* second string location     */
{
  while ( 1 ) {
    if ( ( tolower(*s1) != tolower(*s2) ) || !*s1 )
      break;

    ++s1;
    ++s2;
  }

  return ( (INT)tolower(*s1) - (INT)tolower(*s2) );
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   addPID                                       */
/*                                                                   */
/*   Descriptive Name:  add PID to APIBLOCK list of PIDs             */
/*                                                                   */
/*   Entry Point:       addPID                                       */
/*                                                                   */
/*   Input:             pointer to APIBLOCK and PID                  */
/*                                                                   */
/*********************************************************************/
LONG addPID(PAPIBLOCK cblock, PID processID) {
  ULONG index = 0;
  PID   *tempptr;

  if (cblock->pUserPIDs == NULL) {     /* first addition?            */
    cblock->uPIDBlockSize = 2;
    cblock->pUserPIDs = (PID*) GlobalAlloc(GPTR, cblock->uPIDBlockSize*sizeof(PID));
  }

  while (index < cblock->uPIDBlockSize && cblock->pUserPIDs[index] != (PID) 0) {
    if (cblock->pUserPIDs[index] == processID) {
                                      /* already on the list, return*/
      return RXSUBCOM_NOTREG;         /* immediately                */
    }
    index++;
  }
  if (index == cblock->uPIDBlockSize){/* array too small? enlarge   */
    cblock->uPIDBlockSize *= 2;
    tempptr = (PID*) GlobalAlloc(GPTR, cblock->uPIDBlockSize*sizeof(PID));
    memcpy(tempptr, cblock->pUserPIDs, (cblock->uPIDBlockSize/2)*sizeof(PID));
    GlobalFree(cblock->pUserPIDs);
    cblock->pUserPIDs = tempptr;
  }
                                      /* store the PID              */
  cblock->pUserPIDs[index] = processID;
  return RXSUBCOM_NOTREG;
}

/*********************************************************************/
/*                                                                   */
/*   Subroutine Name:   removePID                                    */
/*                                                                   */
/*   Descriptive Name:  remove PID from APIBLOCK list of PIDs        */
/*                                                                   */
/*   Entry Point:       removePID                                    */
/*                                                                   */
/*   Input:             pointer to APIBLOCK and PID                  */
/*                                                                   */
/*********************************************************************/
LONG removePID(PAPIBLOCK cblock, PID processID) {
  LONG  retval = 0;                    /* 2 if found                 */
                                       /* 1 if found and block can be*/
                                       /*                removed     */
                                       /* 0 if not found             */
  ULONG index = 0;

                                       /* not used by anyone anymore?*/
  if (cblock->pUserPIDs == NULL) return 1;

  while (index < cblock->uPIDBlockSize &&
         cblock->pUserPIDs[index] != processID ) {
    index++;
  }
                                       /* found it?                  */
  if (index < cblock->uPIDBlockSize) {
    retval = 2;
                                       /* remove in the middle?      */
    if (index < (cblock->uPIDBlockSize-1)) {
      memmove(cblock->pUserPIDs + index, cblock->pUserPIDs + index + 1, (cblock->uPIDBlockSize - index - 1)*sizeof(PID));
      if ((index+1) == (cblock->uPIDBlockSize-1)) {
                                       /* set final entry to zero    */
        cblock->pUserPIDs[index+1] = (PID) 0;
      }
    } else {
                                       /* set entry to zero (at end) */
      cblock->pUserPIDs[index] = (PID) 0;
    }
  }
  /* this code is currently disabled
   * if nothing was found, it checks wether all entries are still
   * valid, i.e. if a process with the PIDs from the list still
   * exist. the idea was to remove PIDs from processes that no
   * longer exist. as any REXX process deregisters itself when the
   * DLL is unloaded, this is not needed. it would only be beneficial
   * if a REXX script crashed and the unloading did not take place.
   * if a new process with the "old" PID is around (does not have to
   * be a REXX process!) this would make the APIBLOCK undroppable.
   * therefore, this is disabled unless we reconsider the behaviour.
  else {
    // nothing matching was found, see if there are entries
    // from processes that no longer exist. if so, remove those
    // entries
    index = 0;
    while (index < cblock->uPIDBlockSize &&
           cblock->pUserPIDs[index] != (PID) 0) {
      // try to retrieve info on a process identified by pid
      // if this fails, assume the process is no longer usable
      if (GetProcessVersion(cblock->pUserPIDs[index]) == 0) {
        retval = 2;
       // removing something in the middle?
        if (index < (cblock->uPIDBlockSize-1)) {
          memmove(cblock->pUserPIDs + index, cblock->pUserPIDs + index + 1, (cblock->uPIDBlockSize - index - 1)*sizeof(PID));
        } else {
          // at the end
          cblock->pUserPIDs[index] = (PID) 0; // clear it out
        }
      } else {
        index++;
      }
    }
  } */

  if (cblock->pUserPIDs[0] == (PID) 0) {
    retval = 1; // block can be freed completely
    GlobalFree(cblock->pUserPIDs);
    cblock->pUserPIDs = NULL;
    cblock->uPIDBlockSize = 0;
  }

  return retval;
}

/*********************************************************************/
/*                                                                   */
/*  Function Name: search                                            */
/*                                                                   */
/*  Description:   Given an Environment name and a DLL name search   */
/*                 returns a pointer to the block holding the        */
/*                 registration information with the following       */
/*                 resolution:                                       */
/*                                                                   */
/*                 if no DLL name is passed then first search for an */
/*                 EXE type registration, then search for the name   */
/*                 as a DLL type block                               */
/*                                                                   */
/*  Entry Point:   search( name, dll, type )                         */
/*                                                                   */
/*  Parameter(s):    name     -  The environment name to free        */
/*                   dll  -  The Associated DLL name if appropiate.  */
/*                   type     -  The Registration Type.              */
/*                                                                   */
/*  Return Value:    NULL or the address of the block if found.      */
/*                                                                   */
/*********************************************************************/

PAPIBLOCK APIsearch(
  PSZ name,
  PSZ dll,
  LONG  type,
  DWORD pid  )
{
  PAPIBLOCK cblock = NULL;             /* return pointer for function*/

  if ((!dll) ||                        /* No DLLNAME?, search exe 1st*/
      (!strlen(dll)))
    cblock = exesearch(name,type,pid); /* set cblock with the result.*/

  if (!cblock)                         /* If couldn't find it search */
    cblock = dllsearch(name,           /* set cblock with result.    */
        dll, type);

  if (cblock) memcpy(RX.comblock[API_API], cblock, cblock->apisize);
#ifdef UPDATE_ADDRESS
  /* write address of current pointer at the end of communication page */
  ((RXREG_TALK *)RX.comblock[API_API])->curAPI = (ULONG)cblock;
#endif
  return (cblock);                     /* Return block found or NULL */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name: exesearch                                         */
/*                                                                   */
/*  Description:   Seaches the registration chain for a given        */
/*                 handler.  Only handlers declared as entry points  */
/*                 with the same process id as the caller are        */
/*                 returned.                                         */
/*                                                                   */
/*                 If the routine is found, it returns a pointer     */
/*                 to the control block.  The block will be moved    */
/*                 to the front of the chain for to speed            */
/*                 subsequent references.                            */
/*                                                                   */
/*  Entry Point:   exesearch( name, type, pid )                      */
/*                                                                   */
/*  Parameter(s):    name     -  The handler name to locate.         */
/*                   type     -  registration type                   */
/*                   pid      -  processid of registering process    */
/*                                                                   */
/*  Return Value:    NULL or the address of the block if found.      */
/*                                                                   */
/*********************************************************************/

APIBLOCK *exesearch(                   /* Function Declaration.      */
  PSZ name,                            /* Name to find               */
  LONG  type,                          /* Type of name.              */
  DWORD pid)
{
  APIBLOCK *cblock;                    /* Working ptr, current block */
  APIBLOCK *previous;                  /* prior block for movement   */

  previous = NULL;                     /* no prior block yet         */
  cblock = RX.baseblock[type];         /* Working ptr, current block */

  while (cblock) {                     /* Run through the list       */
    if ((PID)pid == cblock->apipid ) {
                                       /* Comp name with passed name */
    if((!rxstricmp(APIBLOCKNAME(cblock),name)) &&
        !APIBLOCKDLLNAME(cblock)) {    /* and not a dll              */
      if (previous) {                  /* if not at front            */
        previous->next =               /* rearrange the chain to move*/
          cblock->next;                /* this block to the front    */
        cblock->next=                  /* of the list.  We are likely*/
          RX.baseblock[type];          /* to need it again soon      */
        RX.baseblock[type] = cblock;
      }
      return (cblock);                 /* matching pid, return block */
    }
    } else ;

    previous = cblock;                 /* remember this one,         */
    cblock = cblock->next;             /* and continue the search    */
  }                                    /* END of Search              */
  return (cblock);                     /* Return the return code     */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name: dllsearch                                         */
/*                                                                   */
/*  Description:   Seaches the registration chain for a given        */
/*                 handler.  Only handlers declared as dll           */
/*                 procedures.                                       */
/*                                                                   */
/*                 If the routine is found, it returns a pointer     */
/*                 to the control block.  The block will be moved    */
/*                 to the front of the chain for to speed            */
/*                 subsequent references.                            */
/*                                                                   */
/*  Entry Point:   dllsearch( name, type )                           */
/*                                                                   */
/*  Parameter(s):    name     -  The handler name to locate.         */
/*                   type     -  The Registration Type.              */
/*                                                                   */
/*  Return Value:    NULL or the address of the block if found.      */
/*                                                                   */
/*  Notes:         This routine returns a pointer which may have     */
/*                 returned from a MapViewOfFile. The caller is      */
/*                 responsible for calling UnmapViewOfFile.          */
/*                                                                   */
/*********************************************************************/

APIBLOCK *dllsearch(
  PSZ name,                            /* Name to find               */
  PSZ dll,                             /* Dynalink Name to find.     */
  LONG  type )                         /* Type of name to find.      */
{
  APIBLOCK *cblock;                    /* Working ptr, current block */
  APIBLOCK *previous;                  /* prior block for movement   */

  previous = NULL;                     /* no prior block yet         */
  cblock = RX.baseblock[type];         /* Working ptr, current block */

  if (!dll)                            /* Treat NULL dlls as 0 length*/
    dll = "";                          /* names.                     */
  while (cblock) {                     /* while not at end of chain  */
    if(((APIBLOCKDLLPROC(cblock))&&    /* If no proc name, is .exe   */
        (strlen(APIBLOCKDLLPROC(cblock))))&& /* which we don't want. */
                                       /* If environment names match */
       (!rxstricmp(APIBLOCKNAME(cblock),name))
        && (!dll[0]                    /* And passed dll name is NULL*/
        || (APIBLOCKDLLNAME(cblock)    /* Or exists a stored dll name*/
        && (!rxstricmp(APIBLOCKDLLNAME(cblock), /* And               */
        dll))                          /* it matches the passed one  */
        ))) {                          /* Then                       */
      if (previous) {                  /* if not at front            */
        previous->next =               /* rearrange the chain to move*/
          cblock->next;                /* this block to the front    */
        cblock->next=                  /* of the list.  We are likely*/
          RX.baseblock[type];          /* to need it again soon      */
        RX.baseblock[type] = cblock;
      }
      return (cblock);                 /* matching pid, return block */
    }
    previous = cblock;                 /* remember this one,         */
    cblock = cblock->next;             /* and continue the search    */
  }                                    /* END of Search              */

  return (cblock);                     /* Return the return code     */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   execheck                                        */
/*                                                                   */
/*  Description:     Searches the registration chain with respect to */
/*                   Environment Name and process information. If    */
/*                   registration of this type is possible then      */
/*                   the function returns the appropiate code        */
/*                   RXSUBCOM_OK otherwise the return value is set   */
/*                   to the appropiate error code.                   */
/*                                                                   */
/*  Entry Point:     execheck                                        */
/*                                                                   */
/*  Parameter(s):    name - name of subcom environment               */
/*                   type - the registration type.                   */
/*                                                                   */
/*  Return Value:    as stated above                                 */
/*                                                                   */
/*********************************************************************/

LONG APIexecheck(
  PSZ   name,
  LONG  type,
  DWORD pid  )
{
  ULONG  rc = RXSUBCOM_OK;             /* Function return code.      */
  PAPIBLOCK cblock;

  cblock = RX.baseblock[type];         /* Working ptr, current block*/
  while (cblock) {                     /* Run through the list       */
    if (!rxstricmp(APIBLOCKNAME(cblock), /* Comp name w/ passed name */
        name)) {                       /* if found a matching name   */
      if (!APIBLOCKDLLNAME(cblock)) {  /* if not a dll type and      */
        if ((PID)pid == cblock->apipid){ /* matching process info    */
          rc = RXSUBCOM_NOTREG;        /* then must be registered &  */
                                       /* cannot be re-registered    */
          cblock = NULL;               /* so force a quick end.      */
        }
      }
      else                             /* this is a DLL match        */
        rc = RXSUBCOM_DUP;             /* then set the duplicate rc  */
    }                                  /* END of NAME comparison     */
    if (cblock)                        /* If more blocks incrmnt ptr */
      cblock = cblock -> next ;
  }                                    /* END of Search              */
  return (rc);                         /* Return the return code     */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   dllcheck                                        */
/*                                                                   */
/*  Description:     dllcheck searches the registration chain with   */
/*                   respect to Environment Name and DLL name. If    */
/*                   registration of this type is possible then the  */
/*                   function returns the appropiate code            */
/*                   RXSUBCOM_OK otherwise the return value is       */
/*                   set to the appropiate error code.               */
/*                                                                   */
/*  Entry Point:     dllcheck                                        */
/*                                                                   */
/*  Parameter(s):    name    - name of the subcom routine            */
/*                   dllname - name of dll file                      */
/*                   type -    the registration type.                */
/*                                                                   */
/*  Return Value:    as stated above                                 */
/*                                                                   */
/*********************************************************************/

LONG APIdllcheck(
        PAPIBLOCK sblock,
  LONG  type )
{
  LONG  rc = RXSUBCOM_OK;              /* Function return code.      */
  PSZ   name = APIBLOCKNAME(sblock);   /* function name              */
  PSZ   dllname = APIBLOCKDLLNAME(sblock);/* @ENG002A - dll name     */
  PAPIBLOCK cblock = RX.baseblock[type];/* Working ptr, current block */

  while (cblock) {                     /* While another block        */
    if (!rxstricmp(APIBLOCKNAME(cblock), /* Com name with passed name*/
        name)) {                       /* if found a matching name   */
      rc = RXSUBCOM_DUP;               /* then set the duplicate rc  */
                                       /* if a registered dll name & */
      if ((APIBLOCKDLLNAME(cblock)) && (!rxstricmp((APIBLOCKDLLNAME(cblock)),dllname)))
      {                                /* it matches the dll name passed in */
        rc = RXSUBCOM_NOTREG;          /* then set appropiate rc     */
        if (type != REGSUBCOMM) {
          addPID(cblock, sblock->apipid);
        }
        cblock = NULL;
      }                                /* END of DLL comparison      */
    }                                  /* END of NAME comparison     */
    if (cblock)                        /* If more blocks incrmnt ptr */
      cblock = cblock->next;
  }                                    /* END of Search              */
  return (rc);
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   regdrop                                         */
/*                                                                   */
/*  Description:     drops a block from the registration             */
/*                   table.                                          */
/*                                                                   */
/*  Entry Point:     regdrop( name, dll, type )                      */
/*                                                                   */
/*  Parameter(s):    name     -  The environment name to be dropped. */
/*                   dll  -  The associated dllname to be dropped,   */
/*                               if appropiate.                      */
/*                   type     -  Registration type.                  */
/*                   pid      -  process id                          */
/*                                                                   */
/*  Return Value:    Valid RXSUBCOM return codes                     */
/*                                                                   */
/*********************************************************************/

LONG  APIregdrop(
  PSZ name,                            /* Environment Name           */
  PSZ dll,                             /* Associated Name (of DLL)   */
  LONG  type,                          /* Registration type.         */
  DWORD pid)
{
  PAPIBLOCK cblock;                    /* Ptr to the current block   */
  PAPIBLOCK pblock;                    /* Ptr to the previous block  */
  LONG      rc;                        /* init return code to NOTREG */

  rc = RXSUBCOM_NOTREG;                /* init return code to NOTREG */
  cblock = RX.baseblock[type];         /* Get head of the chain      */
  pblock = NULL;                       /* No previous block yet      */
  if (dll && !strlen(dll))             /* if dll is a null string    */
    dll = NULL;                        /* make it really null        */

  while (cblock != NULL) {
   if ((!rxstricmp(APIBLOCKNAME(cblock), name) &&
       (!dll)) ||
       (APIBLOCKDLLNAME(cblock) && dll &&
        !rxstricmp(APIBLOCKDLLNAME(cblock), dll))) {
     if (!cblock->apidrop_auth ||
         (cblock->apidrop_auth == 1 &&
          cblock->apipid == (PID)pid   )) {
       EnterCriticalSection(&nest);

       if (type != REGSUBCOMM) {
         rc = removePID(cblock, pid);
       } else {
         rc = 1;
       }
       if (rc == 0) {
         rc = RXSUBCOM_NOCANDROP;
       } else {
         // can this be freed?
         if (rc == 1) {
           if (pblock) pblock->next =      /* if previous block then     */
                 cblock->next;             /* linkup chain as next entry */
           else                            /* Otherwise, link to anchor  */
             RX.baseblock[type] = cblock->next;/* as the anchor.         */


           GlobalFree(cblock);               /* free the block itself      */
                                 }
         rc = RXSUBCOM_OK;               /* Set appropiate return code */
         cblock = NULL;                  /* set finished condition     */
                         }
       LeaveCriticalSection(&nest);
     }
     else {
       rc = RXSUBCOM_NOCANDROP;        /* Set the error return code  */
       /* don't end loop but seek for the one that matches the pid */
     }
   }
   if (cblock) {                       /* if we have not reached end */
     pblock = cblock;                  /* of chain or still haven't  */
     cblock = cblock->next;            /* found it then continue     */
   }

  }

  return (rc);
}
