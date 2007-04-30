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
/*                                                                   */
/*  Module Name:  aixseapi.c                                         */
/*                                                                   */
/*  Description:  External Function Adapter interface routines       */
/*                External Macro Space function interface routines   */
/*                                                                   */
/*  Function:     Locates and calls external adapter interface       */
/*                  manager and macro space functions                */
/*                                                                   */
/*  Entry Points: sys_external()  - external function search routine */
/*                sys_extension() - AFT function access              */
/*                sys_macrofn()   - macro space function access      */
/*                                                                   */
/*********************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define  INCL_RXFUNC
#define  INCL_RXSUBCOM
#define  INCL_RXSYSEXIT

#include <stdlib.h>
#include "PlatformDefinitions.h"
#include "RexxCore.h"                         /* global REXX declarations       */
//#include "okactiva.hpp"
//#include "okstring.hpp"
//#include "okactivi.hpp"
#include "RexxLibrary.h"
//#include "oknativa.hpp"
#include "RexxNativeAPI.h"
#include SYSREXXSAA
#include "SharedMemorySupport.h"
#include "SystemSemaphores.h"
#include "SubcommandAPI.h"
#include "RexxAPIManager.h"
#include "APIUtilities.h"
#include <unistd.h>
#include <sys/shm.h>

#if defined( HAVE_SYS_LDR_H )
# include <sys/ldr.h>
#endif

#if defined( HAVE_FILEHDR_H )
# include <filehdr.h>
#endif

#include <dlfcn.h>
#include <sys/stat.h>

#if defined( HAVE_USERSEC_H )
# include <usersec.h>
#endif

#include <errno.h>

#define CCHMAXPATH PATH_MAX+1

extern REXXAPIDATA  *apidata;                   /* Global state data          */

//#define PIDCMP(x)    ((x->apipid==apidata->ProcessId))
#define PT_32BIT       1                       /* only for Windows            */

PVOID  pLibSave = NULL;                        /* External lib handle         */
CHAR   szLibName[MAXNAME +1];                  /* External lib name           */

/*********************************************************************/
/*                                                                   */
/* Function prototypes for local support routines.                   */
/*                                                                   */
/*********************************************************************/

LONG  RegQuery(PSZ, PSZ, PUSHORT, PUCHAR, LONG );
LONG dllcheck(PSZ, PSZ, LONG);        // Check against duplicating
LONG execheck(PSZ, LONG );
LONG  RegLoad(PSZ, PSZ, LONG , PFN *, PULONG, PVOID*);
LONG  RegDrop(PSZ, PSZ, LONG );       // Drop an api block from the chain
LONG  RegRegisterExe(PSZ, PFN, PUCHAR, LONG, LONG);
LONG  RegRegisterDll(PSZ, PSZ, PSZ, PUCHAR, ULONG, LONG);
//PAPIBLOCK search(PSZ, PSZ, LONG );
APIBLOCK *RegSearch(PSZ, LONG, CHAR);
//APIBLOCK *dllsearch(PSZ, PSZ, LONG );


/********************************************************************/
/* Function name:      RxSubcomExitList()                           */
/*                                                                  */
/* Description:        Remove all process-specific system exits and */
/*                     subcommands when a process exits.            */
/*                                                                  */
/* Function:           Find all subcommands and system exits that   */
/*                       1.  Belong to the current process and      */
/*                       2.  Are registered by address.             */
/*                     Deregister them.                             */
/*                                                                  */
/* Inputs:             None.                                        */
/*                                                                  */
/* Outputs:            None.  Called for effects only.              */
/*                                                                  */
/* Effects:            All system exits and subcommands that the    */
/*                     current process registered by address are    */
/*                     deregistered.                                */
/*                                                                  */
/* Notes:                                                           */
/*                                                                  */
/*   Called by the OS/2 Exit List Handler.  This routine runs after */
/*   the semaphore cleanup exit routine.                            */
/*                                                                  */
/********************************************************************/

VOID   APIENTRY RxSubcomExitList(VOID)
{
  ULONG           block;
  ULONG           prev, c;
  LONG            i ;
  INT             begin = 0;
  pid_t           pidOwnID;             /* keep process id          */

if(apidata == NULL)                     /* nothing happend at all   */
  return;

if(apidata->sebase != NULL)             /* if there is API memory   */
{
                                       /* now try to get access     */
  if (RxAPIStartUp(SECHAIN))           /* if we couldn't get all of */
                                       /* the shared resources,     */
    return;

    /****************************************************************/
    /* The system maintains subcommand, exit and function           */
    /* registrations in seperate changes.  Loop through all chains  */
    /* lookinf for routines the current process has registered      */
    /* by address.  Remove any found.                               */
    /****************************************************************/
    pidOwnID = getpid();              /* Save time in loop          */
    for( i = 0 ; i < REGNOOFTYPES ; ++ i ) {
      c =apidata->baseblock[i];       /* get the anchor of the chain*/
      prev = 0;

      while( c ) {
        if(begin == 1){
          begin = 0;
          c = apidata->baseblock[i]; /* begin at front of chain     */
        }
        if( (SEDATA(c)->apiownpid == pidOwnID)  &&
            (SEDATA(c)->apidll_name == NULL ) )
        {
          /**********************************************************/
          /* If the block is at the beginning of the chain (the     */
          /* baseblock variable will contain its address),we need to*/
          /* reset the beginning of the chain to point to its       */
          /* follower.  Otherwise, fix up the next pointer in the   */
          /* previous block.                                        */
          /**********************************************************/

          if( prev ) {
             SEDATA(prev)->next = SEDATA(c)->next ;
          }
          else {
             apidata->baseblock[ i ] = SEDATA(c)->next ;
          }
          begin = 1; /* now begin at front(perhaps the memory is rearanged)*/
          c = SEDATA(c)->next;
          RxFreeAPIBlock(c,sizeof(APIBLOCK));    /* release the block */
//        if ( apidata->sememtop > 1 )
//           apidata->baseblock[ i ] = apidata->sememtop -  sizeof(APIBLOCK);
//        else
//           apidata->baseblock[ i ] = NULL;
        }
        else {
          prev = c ;                         /* Keeping this block.   */
          c = SEDATA(c)->next;               /* Next, please!         */
        }
      }
    }
  RxAPICleanUp(SECHAIN, SIGCNTL_RELEASE); /* release shared resources */
}
  return;                              /* finished                  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:  RexxRegisterSubcomDll                            */
/*                                                                   */
/*  Description:    Registration function for the subcommand         */
/*                  interface.  All programs wishing to act as       */
/*                  subcommand environments for REXX must first      */
/*                  register through this function.                  */
/*                                                                   */
/*                  The function adds new registration blocks to the */
/*                  subcommand registration table. Uses functions in */
/*                  REXXAPI for memory allocation functions.         */
/*                                                                   */
/*  Entry Point:    RexxRegisterSubcomDll                            */
/*                                                                   */
/*  Parameter(s):   EnvName   - name of the registered subcommand    */
/*                              handler                              */
/*                                                                   */
/*                  ModuleName - name of the DLL module containing   */
/*                               the subcommand handler              */
/*                                                                   */
/*                  EntryPoint - name of the DLL routine for the     */
/*                               handler                             */
/*                                                                   */
/*                  UserArea   - Area for any user data              */
/*                                                                   */
/*                  DropAuth   - Drop authority flag                 */
/*                                                                   */
/*  Return Value:   Valid RXSUBCOM return codes                      */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxRegisterSubcomDll(
  PSZ   EnvName,                       /* Subcom name                */
  PSZ   ModuleName,                    /* Name of DLL                */
  PSZ   EntryPoint,                    /* DLL routine name           */
  PUCHAR UserArea,                     /* User data                  */
  ULONG DropAuth )                     /* Drop Authority             */
{
  ULONG  rc;                           /* Function return code.      */
                                       /* Register the subcommand.   */
  rc = RegRegisterDll(EnvName, ModuleName, EntryPoint, UserArea,
                      DropAuth, REGSUBCOMM);
  return (rc);                         /* and exit with return code  */
}



/*********************************************************************/
/*                                                                   */
/*  Function Name:  RexxRegisterSubcomExe                            */
/*                                                                   */
/*  Description:    Registration function for the subcommand         */
/*                  interface.  All programs wishing to act as       */
/*                  subcommand environments for REXX must first      */
/*                  register through this function.                  */
/*                                                                   */
/*                  The function adds new registration blocks to the */
/*                  subcommand registration table. Uses functions in */
/*                  REXXAPI for memory allocation functions.         */
/*                                                                   */
/*  Entry Point:    RexxRegisterSubcomDll                            */
/*                                                                   */
/*  Parameter(s):   EnvName   - name of the registered subcommand    */
/*                              handler                              */
/*                                                                   */
/*                  EntryPoint - address of the subcommand handler   */
/*                                                                   */
/*                  UserArea   - Area for any user data              */
/*                                                                   */
/*  Return Value:   Valid RXSUBCOM return codes                      */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxRegisterSubcomExe(
  PSZ   EnvName,                       /* Subcom name                */
  PFN   EntryPoint,                    /* DLL routine name           */
  PUCHAR UserArea )                    /* User data                  */
{
  ULONG  rc;                           /* Function return code.      */
                                       /* Register the subcommand    */
                                       /* (as a 32-bit callout       */
  rc = RegRegisterExe(EnvName, EntryPoint, UserArea,
                      REGSUBCOMM,PT_32BIT);
  return (rc);                         /* and exit with return code  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      RexxCallFunction                             */
/*                                                                   */
/*  Description:        find and call an external function           */
/*                                                                   */
/*  Entry Points:       sys_external(dname,argc,argv,result,type)    */
/*                                                                   */
/*  Inputs:             dname  - the name of the function to call    */
/*                      argc   - count of arguments                  */
/*                      argv   - array of argument strings           */
/*                      result - storage for result string from call */
/*                      act_q  - name of active data queue           */
/*                                                                   */
/*  Notes:              External Function Search Order:              */
/*                         - System Exit functions                   */
/*                         - Macro Space Pre-Order functions         */
/*                         - Available Function Table functions      */
/*                         - External Same-Extension functions       */
/*                         - External Default-Extension functions    */
/*                         - Macro Space Post-Order functions        */
/*                                                                   */
/*  Outputs:            YES if function executed, NO if not          */
/*                                                                   */
/*********************************************************************/
APIRET APIENTRY RexxCallFunction (
        PSZ dname,                     /* Name of function to call   */
        ULONG argc,                    /* Number of arguments        */
        PRXSTRING argv,                /* Array of argument strings  */
        PUSHORT  funcrc,                /* RC from function called    */
        PRXSTRING result,              /* Storage for returned data  */
        PSZ act_q)                     /* Name of active data queue  */

{
  ULONG rc = 1;
  ULONG calltype;
  PRXFUNC    func_address;             /* addr for transfer to call  */
  PVOID plib = NULL;                   /* Dll handle                 */

  if (!(rc=RegLoad(dname, NULL, REGFUNCTION, (PFN *)&func_address,
       &calltype, &plib))) {

                                       /* call                       */
    *funcrc = (*func_address)(         /* unsigned char added        */
               (PUCHAR)dname,          /* send function name to call */
                  argc,                /*   and argument count       */
                  argv,                /*   and argument array       */
                  act_q,               /*   and the name of the queue*/
                  result);             /*   and place for ret data   */

//  if(plib)
//    dlclose(plib);                   /* free the DLL               */
  }
  return (rc);                         /* if ok, return success      */
}                                      /* end of RexxCallFunction()  */


/*********************************************************************/
/*                                                                   */
/*  Function Name: RexxLoadSubcom                                    */
/*                                                                   */
/*  Description:   If a subcommand resides in a dll and the dll is   */
/*                 not in memory, RxSubcomLoad loads it into memory  */
/*                                                                   */
/*  Entry Point:   RexxLoadSubcom                                    */
/*                                                                   */
/*  Parameter(s):  name     -  The environment name to be dropped.   */
/*                 dll      -  The associated dllname to be dropped, */
/*                             if appropiate.                        */
/*                                                                   */
/*  Return Value:  Valid RXSUBCOM return codes                       */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxLoadSubcom(
  PSZ name,                            /* Name of Subcommand Environ */
  PSZ dll )                            /* Module name of its' DLL    */
{
  ULONG  calltype;                     /* Routine calltype (not used)*/
  ULONG  rc;                           /* Function return code.      */
  PFN a;                               /* Address of Subcom.         */
  PVOID plib = NULL;                   /* DLL handle                 */

                                       /* Load routine into memory   */
  rc = RegLoad(name, dll, REGSUBCOMM, &a, &calltype, &plib);
  return (rc);                         /* and exit with return code  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxCallSubcom                                  */
/*                                                                   */
/*  Description:     Executes a subcommand handler                   */
/*                                                                   */
/*  Entry Point:     RexxCallSubcom                                  */
/*                                                                   */
/*  Parameter(s):    name       -  Name of the desired system exit   */
/*                   dll        -  Name of the desired dll           */
/*                   cmd        -  command to execute                */
/*                   flags      -  error and failure flags           */
/*                   sbrc       -  handler return code               */
/*                   rv         -  handler return value              */
/*                                                                   */
/*  Return Value:    Return code from subcommand handler processing  */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxCallSubcom(
  PSZ name,                            /* the Subcommand Environment */
  PSZ dll,                             /* Module name of its' DLL    */
  PRXSTRING cmd,                       /* Command string to be passed*/
  PUSHORT flags,                       /* Used to notify errors      */
  PUSHORT sbrc,                        /* return code from handler   */
  PRXSTRING rv )                       /* rxstring to pass back info */
{
  PRXSUBCOM    subcom_addr;            /* addr for transfer to call  */
  LONG rc;
  ULONG calltype;
  PVOID plib = NULL;

 if (!(rc=RegLoad(name, dll, REGSUBCOMM, (PFN *)&subcom_addr,
      &calltype, &plib))) {

    *sbrc = (USHORT)(                /* Call subcom environment w/ */
        (* subcom_addr ) (           /* The ptr to environmnt entry*/
        cmd,                         /* The command to perform     */
        flags,                       /* Pointer to the error flags */
        rv                           /* returned string after call */
        ));                          /* return value is set to SBRC*/
    rc = RXSUBCOM_OK;                /* Set RxSubComExecute return */
//  if(plib)
//    dlclose(plib);                   /* free the DLL              */
 }
  return (rc);                     /* and exit with return code  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxQuerySubcom                                 */
/*                                                                   */
/*  Description:     Querys the subcommand registration table.       */
/*                   Allows user to tell if a subcommand is          */
/*                   registered.                                     */
/*                                                                   */
/*  Entry Point:     RexxQuerySubcom                                 */
/*                                                                   */
/*  Parameter(s):    name     -  The environment name to be dropped. */
/*                   dll      -  The associated dllname to be        */
/*                               dropped, if appropiate.             */
/*                   exist    -  Existence flag.                     */
/*                   userword -  8 bytes of user data.               */
/*                                                                   */
/*  Return Value:    RXSUBCOM_OK - Subcommand is registered          */
/*                   RXSUBCOM_NOTREG - Subcommand not registered     */
/*                   RXSUBCOM_BADTYPE - Internal error.  Should not  */
/*                                      occur.                       */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxQuerySubcom(
  PSZ     name,                        /* Environment Name           */
  PSZ     dll,                         /* Associated Name (of DLL)   */
  PUSHORT exist,                       /* existence information      */
  PUCHAR  userword )                   /* data from registration     */
{
  ULONG  rc;                           /* General Return code holder */
  rc = RegQuery(name, dll, exist, userword, /* Perform the query.    */
      REGSUBCOMM);
  return (rc);                         /* and exit with return code  */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxDeregisterSubcom                            */
/*                                                                   */
/*  Description:     drops a block from the subcommand registration  */
/*                   table. Uses functions in rexxapi to free memory.*/
/*                                                                   */
/*  Entry Point:     RexxDeregisterSubcom                            */
/*                                                                   */
/*  Parameter(s):    name     -  The environment name to be dropped. */
/*                   dllname  -  The associated dllname to be        */
/*                               dropped, if appropiate.             */
/*                                                                   */
/*  Return Value:    Valid RXSUBCOM return codes                     */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxDeregisterSubcom(
  PSZ name,                            /* Environment Name           */
  PSZ dllname )                        /* Associated Name (of DLL)   */
{
  ULONG  rc;                           /* Function return code.      */
  rc = RegDrop(name, dllname, REGSUBCOMM);/* Drop the subcommand.    */
  return (rc);                         /* and exit with return code  */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:  RexxRegisterExitExe                              */
/*                                                                   */
/*  Description:    Registration function for the exit               */
/*                  interface.  All programs wishing to act as       */
/*                  exit handlers for REXX must first                */
/*                  register through this function.                  */
/*                                                                   */
/*                  The function adds new registration blocks to the */
/*                  subcommand registration table. Uses functions in */
/*                  REXXAPI for memory allocation functions.         */
/*                                                                   */
/*  Entry Point:    RexxRegisterExitDll                              */
/*                                                                   */
/*  Parameter(s):   EnvName   - name of the registered exit          */
/*                              handler                              */
/*                                                                   */
/*                  EntryPoint - address of the exit handler         */
/*                                                                   */
/*                  UserArea   - Area for any user data              */
/*                                                                   */
/*  Return Value:   Valid RXSUBCOM return codes                      */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxRegisterExitExe(
  PSZ   EnvName,                       /* exit name                  */
  PFN   EntryPoint,                    /* DLL routine name           */
  PUCHAR UserArea )                    /* User data                  */
{
  ULONG  rc;                           /* Function return code.      */
                                       /* Register the exit          */
  rc = RegRegisterExe(EnvName, EntryPoint, UserArea,
                      REGSYSEXIT, PT_32BIT);
  return (rc);                         /* and exit with return code  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:  RexxRegisterExitDll                              */
/*                                                                   */
/*  Description:    Registration function for the exit               */
/*                  interface.  All programs wishing to act as       */
/*                  exit handlers for REXX must first                */
/*                  register through this function.                  */
/*                                                                   */
/*                  The function adds new registration blocks to the */
/*                  exit registration table. Uses functions in       */
/*                  REXXAPI for memory allocation functions.         */
/*                                                                   */
/*  Entry Point:    RexxRegisterExitDll                              */
/*                                                                   */
/*  Parameter(s):   EnvName   - name of the registered exit          */
/*                              handler                              */
/*                                                                   */
/*                  ModuleName - name of the DLL module containing   */
/*                               the exit handler                    */
/*                                                                   */
/*                  EntryPoint - name of the DLL routine for the     */
/*                               handler                             */
/*                                                                   */
/*                  UserArea   - Area for any user data              */
/*                                                                   */
/*                  DropAuth   - Drop authority flag                 */
/*                                                                   */
/*  Return Value:   Valid RXSUBCOM return codes                      */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxRegisterExitDll(
  PSZ   EnvName,                       /* Exit name                  */
  PSZ   ModuleName,                    /* Name of DLL                */
  PSZ   EntryPoint,                    /* DLL routine name           */
  PUCHAR UserArea,                     /* User data                  */
  ULONG DropAuth )                     /* Drop Authority             */
{
  ULONG  rc;                           /* Function return code.      */
                                       /* Register the subcommand.   */
  rc = RegRegisterDll(EnvName, ModuleName, EntryPoint, UserArea,
                      DropAuth, REGSYSEXIT);
  return (rc);                         /* and exit with return code  */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxDeregisterExit                              */
/*                                                                   */
/*  Description:     drops a block from the exit registration        */
/*                   table. Uses functions in rexxapi to free memory.*/
/*                                                                   */
/*  Entry Point:     RexxDeregisterExit                              */
/*                                                                   */
/*  Parameter(s):    name     -  The environment name to be dropped. */
/*                   dllname  -  The associated dllname to be        */
/*                               dropped, if appropiate.             */
/*                                                                   */
/*  Return Value:    Valid RXSUBCOM return codes                     */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxDeregisterExit(
  PSZ name,                            /* Environment Name           */
  PSZ dllname )                        /* Associated Name (of DLL)   */
{
  ULONG  rc;                           /* Function return code.      */
  rc = RegDrop(name, dllname, REGSYSEXIT);/* Drop the subcommand.    */
  return (rc);                         /* and exit with return code  */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxQueryExit                                   */
/*                                                                   */
/*  Description:     Querys the exit registration table.             */
/*                   Allows user to tell if an exit is               */
/*                   registered.                                     */
/*                                                                   */
/*  Entry Point:     RexxQueryExit                                   */
/*                                                                   */
/*  Parameter(s):    name     -  The environment name to be dropped. */
/*                   dll      -  The associated dllname to be        */
/*                               dropped, if appropiate.             */
/*                   exist    -  Existence flag.                     */
/*                   userword -  8 bytes of user data.               */
/*                                                                   */
/*  Return Value:    RXSUBCOM_OK - Exit is registered                */
/*                   RXSUBCOM_NOTREG - Exit not registered           */
/*                   RXSUBCOM_BADTYPE - Internal error.  Should not  */
/*                                      occur.                       */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxQueryExit(
  PSZ name,                            /* Environment Name           */
  PSZ dll,                             /* Associated Name (of DLL)   */
  PUSHORT exist,                       /* existence information      */
  PUCHAR  userword )                   /* data from registration     */
{
  ULONG  rc;                           /* General Return code holder */
  rc = RegQuery(name, dll, exist, userword, /* Perform the query.    */
      REGSYSEXIT);
  return (rc);                         /* and exit with return code  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxCallExit                                    */
/*                                                                   */
/*  Description:     Executes a system exit.                         */
/*                                                                   */
/*  Entry Point:     RexxCallExit                                    */
/*                                                                   */
/*  Parameter(s):    name       -  Name of the desired system exit   */
/*                   dll        -  Name of the desired dll           */
/*                   fnc        -  Exit function number              */
/*                   subfnc      - Exit subfunction number           */
/*                   param_block - Exit parameter block              */
/*                                                                   */
/*  Return Value:    Return code from exit if the exit ran           */
/*                   -1 otherwise                                    */
/*                                                                   */
/*********************************************************************/

LONG APIENTRY RexxCallExit(
  PSZ name,                            /* Exit name.                 */
  PSZ dll,                             /* dll name.                  */
  LONG  fnc,                           /* Exit function.             */
  LONG  subfnc,                        /* Exit subfunction.          */
  PEXIT param_block )                  /* Exit parameter block.      */
{
  PRXEXIT exit_address;                /* Exit's calling address.    */
  LONG     rc;                         /* Function return code.      */
  ULONG calltype;
  PVOID plib = NULL;

  if (!(rc=RegLoad(name, dll, REGSYSEXIT, (PFN *)&exit_address,
       &calltype, &plib))) {
                                       /* call                       */
    rc = (LONG )(*exit_address)(fnc, subfnc, param_block);
    if(plib)
      dlclose(plib);                   /* free the DLL               */
  }
  else
    rc = -1;
  return (rc);                         /* and exit with return code  */
}



/*********************************************************************/
/*                                                                   */
/*  Function Name:  RexxRegisterFunctionDll                          */
/*                                                                  */
/*  Description:    Registration function for the external function  */
/*                  interface.  All programs wishing to act as       */
/*                  external functions for REXX must first           */
/*                  register through this function.                  */
/*                                                                   */
/*                  The function adds new registration blocks to the */
/*                  function registration table. Uses functions in   */
/*                  REXXAPI for memory allocation functions.         */
/*                                                                   */
/*  Entry Point:    RexxRegisterFunctionDll                          */
/*                                                                   */
/*  Parameter(s):   EnvName   - name of the registered function      */
/*                              handler                              */
/*                                                                   */
/*                  ModuleName - name of the DLL module containing   */
/*                               the function                        */
/*                                                                   */
/*                  EntryPoint - name of the DLL routine for the     */
/*                               function                            */
/*                                                                   */
/*  Return Value:   Valid RXSUBCOM return codes                      */
/*                                                                   */
/*********************************************************************/
APIRET APIENTRY RexxRegisterFunctionDll(
           PSZ EnvName,                /* Subcom name                */
           PSZ ModuleName,             /* Name of DLL                */
           PSZ EntryPoint)             /* Dll routine name           */
{
 return (RegRegisterDll(EnvName, ModuleName, EntryPoint, NULL,RXSUBCOM_DROPPABLE, REGFUNCTION));
                                            /* to new entrypoint name  */
}



/*********************************************************************/
/*                                                                   */
/*  Function Name:  RexxRegisterFunctionExe                          */
/*                                                                   */
/*  Description:    Registration function for the external function  */
/*                  interface.  All programs wishing to act as       */
/*                  external functions for REXX must first           */
/*                  register through this function.                  */
/*                                                                   */
/*                  The function adds new registration blocks to the */
/*                  function registration table.  Uses functions in  */
/*                  REXXAPI for memory allocation functions.         */
/*                                                                   */
/*  Entry Point:    RexxRegisterFunctionDll                          */
/*                                                                   */
/*  Parameter(s):   EnvName   - name of the registered function      */
/*                                                                   */
/*                  EntryPoint - address of the function             */
/*                                                                   */
/*  Return Value:   Valid RXSUBCOM return codes                      */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxRegisterFunctionExe(
  PSZ   EnvName,                       /* Subcom name                */
  PFN   EntryPoint )                   /* DLL routine name           */
{
  ULONG  rc;                           /* Function return code.      */
                                       /* Register the subcommand.   */
  rc = RegRegisterExe(EnvName, EntryPoint, NULL,
                      REGFUNCTION,PT_32BIT);
  return (rc);                         /* and exit with return code  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxDeregisterFunction                          */
/*                                                                   */
/*  Description:     drops a block from the function registration    */
/*                   table. Uses functions in rexxapi to free memory.*/
/*                                                                   */
/*  Entry Point:     RexxDeregisterFunction                          */
/*                                                                   */
/*  Parameter(s):    name     -  The function name to be dropped.    */
/*                                                                   */
/*  Return Value:    Valid RXSUBCOM return codes                     */
/*                                                                   */
/*********************************************************************/
APIRET APIENTRY RexxDeregisterFunction(
           PSZ name)               /* ASCIIZ subcom env name     */
{
  ULONG  rc;                           /* Function return code.      */

  rc = RegDrop(name, NULL, REGFUNCTION); /* Drop the function        */

  return (rc);                         /* and exit with return code  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxQueryFunction                               */
/*                                                                   */
/*  Description:     Querys the function registration table.         */
/*                   Allows user to tell if a function is            */
/*                   registered.                                     */
/*                                                                   */
/*  Entry Point:     RexxQueryFunction                               */
/*                                                                   */
/*  Parameter(s):    name     -  The function name to be queried.    */
/*                                                                   */
/*  Return Value:    RXSUBCOM_OK - Function is registered            */
/*                   RXSUBCOM_NOTREG - Function not registered       */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxQueryFunction(
  PSZ name )                           /* Environment Name           */
{
  ULONG  rc;                           /* General Return code holder */
  USHORT exist;                        /* existance flage            */
  rc = RegQuery(name, NULL, &exist, NULL,/* Perform the query.       */
      REGFUNCTION);
                                       /* set the proper return code */
  rc = (exist)?RXFUNC_OK:RXFUNC_NOTREG;
  return (rc);                         /* and exit with return code  */
}



/*********************************************************************/
/*                                                                   */
/*  Function Name:   RegRegisterDll                                  */
/*                                                                   */
/*  Description:     Registration engine for all external programs.  */
/*                   It loads the package(via the dlopen() function),*/
/*                   loads the functions address (via dlsym()),      */
/*                   then adds the new registration blocks to        */
/*                   the appropriate registration table. Uses        */
/*                   functions in REXXAPI for memory allocation      */
/*                   functions.                                      */
/*                                                                   */
/*  Entry Point:     RegRegisterDll                                  */
/*                                                                   */
/*  Parameter(s):    EnvName - Subcomand name                        */
/*                   ModuleName - The name of the DLL                */
/*                   EntryPoint - DLL routine name                   */
/*                   UserArea - ptr to 8 byte user data area         */
/*                   DropAuth - indicates who can drop this func     */
/*                   type - Registation type                         */
/*                                                                   */
/*  Return Value:    Valid RXSUBCOM return codes                     */
/*                                                                   */
/*********************************************************************/

LONG RegRegisterDll(
  PSZ       EnvName,                   /* Subcom name                */
  PSZ       ModuleName,                /* Name of DLL                */
  PSZ       EntryPoint,                /* Dll routine name           */
  PUCHAR    UserArea,                  /* User Data                  */
  ULONG     DropAuth,                  /* Drop Authoritiy            */
  LONG      type)                      /* Registration Type          */
{
  pid_t pidProcID;
  pid_t pidOwnID;
  PAPIBLOCK cblock = NULL;             /* Working ptr, current block */
  LONG rc = RXSUBCOM_OK;

//EnterMustComplete();                    /* start of critical section*/
  APISTARTUP(SECHAIN);                    /* do common entry code     */
//rc = dllcheck(EnvName, ModuleName, type); /* check against duplicating */
  cblock = RegSearch(EnvName, type, 'M');   /* check against duplicating */
                                          /* if we can still register...*/
//if (rc == RXSUBCOM_OK || rc == RXSUBCOM_DUP)
  if ( cblock == NULL )
  {
    pidOwnID = getpid();                 /* Save time in loop          */
#if defined( HAVE_GETPGRP )
    pidProcID = getpgrp();               /* Save time in loop          */
#else
    pidProcID = pidOwnID;                 /* Save time in loop          */
#endif
//  EnterMustComplete();                  /* start of critical section */
    if( ! RxAllocAPIBlock(&cblock, EnvName, ModuleName, EntryPoint)) {

      /*************************************************************/
      /* Complete the registration.                                */
      /*************************************************************/

      cblock->apiaddr = NULL;        /* no entry point, load later */
                                     /* check for errors           */
      if (UserArea)                  /* Copy the user data         */
         memcpy(cblock->apiuser,UserArea, USERLENGTH);
      cblock->apidrop_auth = DropAuth;/* and drop authority        */
      cblock->apitype = 0;           /* call type determined later */
      cblock->apipid = pidProcID;    /* copy the process Group ID  */
      cblock->apiownpid = pidOwnID;  /* copy the process ID        */
//    if ( apidata->baseblock[type] != 1 )
                                    /* Next pointer set to old top*/
      cblock->next =  apidata->baseblock[type];
      if ( (!rxstricmp(cblock->apidll_name, szLibName)) &&
           ( pLibSave != NULL ) )   /* If lib handle exists get it */
           cblock->apimod_handle = pLibSave;
      else
           cblock->apimod_handle = NULL;
                                    /* Make this block the top     */
        apidata->baseblock[type] = ((ULONG)(((char*)cblock)-(apidata->sebase)));
    }
    else rc = RXSUBCOM_NOEMEM;       /* Couldn't alloc a APIBLOCK  */
//  ExitMustComplete();              /* end of critical section    */
  }
  APICLEANUP(SECHAIN);               /* release shared resources   */
//ExitMustComplete();                /* end of critical section    */
  return (rc);
}




/*********************************************************************/
/*                                                                   */
/*  Function Name:   RegRegisterExe                                  */
/*                                                                   */
/*  Description:     Registration engine for the REXX API            */
/*                   interface.  All programs wishing to act as      */
/*                   subcommand, exits or functions for REXX must    */
/*                   first register through this function.           */
/*                                                                   */
/*                   The function adds new registration blocks to    */
/*                   the correct  registration table. Uses           */
/*                   functions in REXXAPI for memory allocation      */
/*                   functions.                                      */
/*                                                                   */
/*  Entry Point:     RegRegisterExe                                  */
/*                                                                   */
/*  Parameter(s):    EnvName    - The name of the registered handler.*/
/*                   EntryPoint - The address of the handler         */
/*                   UserArea   - The saved user information         */
/*                   type       - The type of registration           */
/*                   CallType   - The type of the callout (16-bit    */
/*                                or 32-bit)                         */
/*                                                                   */
/*  Return Value:    Valid RXSUBCOM return codes                     */
/*                                                                   */
/*********************************************************************/

LONG  RegRegisterExe(
  PSZ   EnvName,                       /* Subcom name                */
  PFN   EntryPoint,                    /* DLL routine name           */
  PUCHAR UserArea,                     /* User data                  */
  LONG  type,                          /* Registration type.         */
  LONG  CallType )                     /* 32- or 16-bit call outs    */
{
  PAPIBLOCK cblock ;                   /* Working ptr, current block */
  LONG rc;
  pid_t pidProcID;
  pid_t pidOwnID;

//EnterMustComplete();                 /* enter critical section     */
  APISTARTUP(SECHAIN);                 /* do common entry code       */
                                       /* check against duplicating  */
  rc = execheck(EnvName, type);
                                       /* if we can still register...*/
//if (rc == RXSUBCOM_NOTREG || rc == RXSUBCOM_DUP) {
  if (rc == RXSUBCOM_NOTREG )
  {
      /***************************************************************/
      /* Allocate the APIBLOCK.  If we succeed, fill in the registra-*/
      /* tion data.  Otherwise, return RXSUBCOM_NOMEM.               */
      /***************************************************************/

//  EnterMustComplete();               /* enter critical section     */
    if( ! RxAllocAPIBlock( &cblock,
                     EnvName, NULL, NULL))
    {
      pidOwnID = getpid();                 /* Save time in loop      */
#if defined( HAVE_GETPGRP )
      pidProcID = getpgrp();               /* Save time in loop      */
#else
      pidProcID = pidOwnID;                /* Save time in loop      */
#endif

        /*************************************************************/
        /* Complete the registration.                                */
        /*************************************************************/

        cblock->apiaddr = EntryPoint;  /* fill in the entry point    */
        if (UserArea)                  /* copy the user data         */
          memcpy(cblock->apiuser, UserArea, USERLENGTH);
                                       /* EXEs are always NONDROP    */
        cblock->apidrop_auth=RXSUBCOM_NONDROP;
        cblock->apiownpid = pidOwnID;  /* copy the process ID        */
        cblock->apipid = pidProcID;    /* fill in process and        */
        cblock->apitype = CallType;    /* remember type of call      */
        cblock->next =                 /* Next pointer set to old top*/
            apidata->baseblock[type];
        cblock->apiFunRegFlag = 1;     /* Not the main registration  */
                                       /* Make this block the top    */
        apidata->baseblock[type] = ((ULONG)(((char*)cblock)-(apidata->sebase)));
        rc = RXSUBCOM_OK;
    }
    else rc = RXSUBCOM_NOEMEM;         /* Couldn't alloc an APIBLOCK */
//  ExitMustComplete();                /* end of critical section    */
  }
  APICLEANUP(SECHAIN);                 /* release shared resources   */
//ExitMustComplete();                  /* end of critical section    */
  return (rc);
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RegQuery                                        */
/*                                                                   */
/*  Description:     query a named handler and return the user data  */
/*                   for a handler.                                  */
/*                                                                   */
/*  Entry Point:     RegDrop()                                       */
/*                                                                   */
/*  Parameter(s):    name     -  The environment name to be dropped. */
/*                   dll  -  The associated dllname to be queried,   */
/*                               if appropiate.                      */
/*                   type     -  Registration type.                  */
/*                                                                   */
/*  Output:          usrwrd   -  User specified data                 */
/*                   exist    -  Existance flag                      */
/*                                                                   */
/*  Return Value:    Valid RXSUBCOM return codes                     */
/*                                                                   */
/*********************************************************************/

LONG  RegQuery(
  PSZ name,                            /* Environment Name           */
  PSZ dll,                             /* Associated Name (of DLL)   */
  PUSHORT exist,                       /* existence information      */
  PUCHAR  usrwrd,                      /* data from registration     */
  LONG  type )                         /* Registration type.         */
{
  PAPIBLOCK cblock;                    /* Working ptr, current block */
  LONG      rc;

//EnterMustComplete();                 /* start of critical section  */
  APISTARTUP(SECHAIN);                 /* do common entry code       */

//cblock = search(name, dll, type);    /* Find the block or not.     */
  if ( dll != NULL )
  {
    cblock = RegSearch(name, type, 'M'); /* Search for MASTER only   */
  }
  else
  {
    cblock = RegSearch(name, type, 'A'); /* Search all blocks        */
  }

  if (cblock) {                        /* If block found then it must*/
    *exist = YES;

    if (usrwrd)                        /* And get user word as well  */
       memcpy(usrwrd,cblock->apiuser,USERLENGTH);
    rc = RXSUBCOM_OK;                  /* Now we're done. And its OK.*/
  }                                    /* Otherwise                  */
  else {
    rc = RXSUBCOM_NOTREG;              /* It ain't thar, so say it.  */
    *exist = NO;
  }
  APICLEANUP(SECHAIN);                    /* release shared resources   */
//ExitMustComplete();                     /* start of critical section  */
  return (rc);
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RegDrop                                         */
/*                                                                   */
/*  Description:     drops a block from the subcommand registration  */
/*                   table. Uses functions in rexxapi to free memory.*/
/*                                                                   */
/*  Entry Point:     RegDrop( name, dll, type )                      */
/*                                                                   */
/*  Parameter(s):    name     -  The environment name to be dropped. */
/*                   dll  -  The associated dllname to be dropped,   */
/*                               if appropiate.                      */
/*                   type     -  Registration type.                  */
/*                                                                   */
/*  Return Value:    Valid RXSUBCOM return codes                     */
/*                                                                   */
/*********************************************************************/

LONG  RegDrop(
  PSZ name,                            /* Environment Name           */
  PSZ dll,                             /* Associated Name (of DLL)   */
  LONG  type )                         /* Registration type.         */
{
  ULONG cblock;                        /* Ptr to the current block   */
  ULONG pblock;                        /* Ptr to the previous block  */
  LONG  rc = RXSUBCOM_NOTREG;          /* init return code to NOTREG */
                                       /* Exception handler record   */

//EnterMustComplete();                 /* start of critical section  */
  APISTARTUP(SECHAIN);                 /* do common entry code       */

  cblock = apidata->sememtop;          /* get to the top  'pointer'  */
//cblock = apidata->baseblock[type];   /* Get head of the chain      */
  pblock = 0;                          /* No previous block yet      */
  if (dll && !strlen(dll))             /* if dll is a null string    */
    dll = NULL;                        /* make it really null        */
//pidProcID = getpid();                /* Save time in loop          */

  while (cblock > SHM_OFFSET )
  {
   cblock -= sizeof(APIBLOCK);         /* Get last block  'pointer'  */
   if ((!rxstricmp(SEDATA(cblock)->apiname, name) &&
       (!dll)) ||
       (SEDATA(cblock)->apidll_name && dll &&
        !rxstricmp(SEDATA(cblock)->apidll_name, dll))) {
//   if (!SEDATA(cblock)->apidrop_auth ||                            */
//       (SEDATA(cblock)->apidrop_auth == 1 &&                       */
//       (SEDATA(cblock)->apipid == pidProcID ||
//       ((getpgid(SEDATA(cblock)->apipid) == -1) &&     /* if not running */
//       (errno == ESRCH)))) )
//   if (SEDATA(cblock)->apiFunRegFlag == 0) /* Only the main registration */
//   {
       SEDATA(cblock)->apiFunRegFlag = 1; /* Change to no main registration */
//   }
//   else {
//     rc = RXSUBCOM_NOCANDROP;        /* Set the error return code  */
       rc = RXSUBCOM_OK;               /* Set appropiate return code */
//     cblock = 0;                     /* Set the finished condition */
//   }
   }
   if (cblock) {                       /* if we have not reached end */
     pblock = cblock;                  /* of chain or still haven't  */
//   cblock -=  sizeof(APIBLOCK);      /* found it then continue     */
//   cblock = SEDATA(cblock)->next;    /* found it then continue     */
   }
  }
//if((apidata->baseblock[REGSUBCOMM] == 0 ) /* if all chains empty   */
//        && (apidata->baseblock[REGSYSEXIT] == 0 )
//        && (apidata->baseblock[REGFUNCTION] == 0 )){
//removeshmem(apidata->sebasememId);   /* remove the se memory       */
//detachshmem(apidata->sebase);   /* detach it to force the deletion */
//apidata->sebase = NULL;              /* reset the memory anchor    */
//}
  APICLEANUP(SECHAIN);                 /* release shared resources   */
//ExitMustComplete();                  /* end of critical section    */
//if ( rc == 0 )
//   rc = RXSUBCOM_NOCANDROP;        /* Set the error return code    */
  return (rc);
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   RegDeregFunc                                    */
/*                                                                   */
/*  Description:     drops a block from the subcommand registration  */
/*                   table. Uses functions in rexxapi to free memory.*/
/*                   A main registration block will be prepared for  */
/*                   reuse.                                          */
/*                                                                   */
/*  Entry Point:     RegDeregFunc(  dll, type )                      */
/*                                                                   */
/*  Parameter(s):    name     -  The environment name to be dropped. */
/*                   dll  -  The associated dllname to be dropped,   */
/*                               if appropiate.                      */
/*                   type     -  Registration type.                  */
/*                                                                   */
/*  Return Value:    Valid RXSUBCOM return codes                     */
/*                                                                   */
/*********************************************************************/

ULONG  RegDeregFunc(
  PSZ dll,                             /* Associated Name (of DLL)   */
  LONG  type )                         /* Registration type.         */
{
  LONG temp_cblock;
  LONG temp_nblock;
  ULONG ulBlockSize;

  ULONG     rc;                        /* init return code to NOTREG */
                                       /* Exception handler record   */
  pid_t     pidOwnID;                  /* Keep process id            */
  pid_t     pidProcID;                 /* Keep process id            */
  PVOID     pLibH[3];                  /* Save two lib handles       */

  pLibH[0] = NULL;                     /* Reset the lib handle 1     */
  pLibH[1] = NULL;                     /* Reset the lib handle 2     */
  pLibH[2] = NULL;                     /* Reset the lib handle 2     */

//EnterMustComplete();                 /* start of critical section  */
  APISTARTUP(SECHAIN);                 /* do common entry code       */

  rc = RXSUBCOM_NOTREG;                /* init return code to NOTREG */
  if (dll && !strlen(dll))             /* if dll is a null string    */
    dll = NULL;                        /* make it really null        */

  pidOwnID = getpid();                 /* Save time in loop          */
#if defined( HAVE_GETPGRP )
  pidProcID = getpgrp();               /* Save time in loop          */
#else
  pidProcID = pidOwnID;                /* Save time in loop          */
#endif

  ulBlockSize = sizeof(APIBLOCK);
  temp_cblock = apidata->sememtop;          /* update the top  'pointer'   */
  temp_nblock = temp_cblock - ulBlockSize;  /* update the next 'pointer'   */
  while ( temp_cblock > SHM_OFFSET )
  {
    temp_cblock  -= ulBlockSize;            /* Set the current block offset*/
    if ( temp_cblock > ulBlockSize )
      temp_nblock  -= ulBlockSize;          /* Set the next    block offset*/
    else
      temp_nblock  = 0;                     /* Set the next at beginning   */
                                            /* Only the Owner can free lib */
    if ( (SEDATA(temp_cblock)->apiownpid == pidOwnID)  &&
         (SEDATA(temp_cblock)->apimod_handle) &&
         (SEDATA(temp_cblock)->apimod_handle != pLibH[0]) &&
         (SEDATA(temp_cblock)->apimod_handle != pLibH[1]) &&
         (SEDATA(temp_cblock)->apimod_handle != pLibH[2]) )
    {
      dlclose(SEDATA(temp_cblock)->apimod_handle);     /* free the library     */
//    if (dlclose(SEDATA(temp_cblock)->apimod_handle)) /* for debug only   */
//      fprintf(stderr, " *** Info message: %s\n", dlerror());
      pLibH[2] = pLibH[1];                 /* Save lib handles closed */
      pLibH[1] = pLibH[0];                 /* Save lib handles closed */
      pLibH[0] = SEDATA(temp_cblock)->apimod_handle;
    }

    if ( (SEDATA(temp_cblock)->apipid == pidProcID) ||  /* Own entry or    */
         (SEDATA(temp_cblock)->apipid == 0)  ||         /* cleanup entries */
         ((SEDATA(temp_cblock)->apipid != 0)  &&        /* cleanup entries */
         (kill(SEDATA(temp_cblock)->apipid, 0) == -1 )) ) /* if not running*/
    {
//    EnterMustComplete();                   /* start of critical section  */
      if  (SEDATA(temp_cblock)->apiFunRegFlag)  /* Not the main registration */
      {
        RxFreeAPIBlock(temp_cblock, ulBlockSize);
        if ( apidata->sememtop > SHM_OFFSET )
           apidata->baseblock[type] = apidata->sememtop -  ulBlockSize;
        else
           apidata->baseblock[type] = 0;
      }
      else                               /* Reset main cblock for reuse */
      {
         SEDATA(temp_cblock)->apipid  = 0;
         SEDATA(temp_cblock)->apiownpid  = 0;
         SEDATA(temp_cblock)->apiaddr = NULL;
         SEDATA(temp_cblock)->apimod_handle = NULL;
      }
//    ExitMustComplete();             /* end of critical section    */

      rc = RXSUBCOM_OK;               /* Set appropiate return code */
    }
  }
  pLibSave = NULL;                       /* Reset lib handle         */
  szLibName[0] = '\0';                   /* Reset lib name           */
  if((apidata->baseblock[REGSUBCOMM] == 0 ) /* if all chains empty   */
          && (apidata->baseblock[REGSYSEXIT] == 0 )
          && (apidata->baseblock[REGFUNCTION] == 0 )){
  removeshmem(apidata->sebasememId);   /* remove the se memory       */
  detachshmem(apidata->sebase);   /* detach it to force the deletion */
  apidata->sebase = NULL;              /* reset the memory anchor    */
  }
  APICLEANUP(SECHAIN);                 /* release shared resources   */
//ExitMustComplete();                  /* end of critical section    */
  return (rc);
}



/*********************************************************************/
/*                                                                   */
/*  Function Name:   RegLoad                                         */
/*                                                                   */
/*  Description:     Load a DLL routine into storage.                */
/*                                                                   */
/*  Entry Point:     RegLoad()                                       */
/*                                                                   */
/*  Parameter(s):    name     -  The environment name to be dropped. */
/*                   dll  -  The associated dllname to be queried,   */
/*                               if appropiate.                      */
/*                   type     -  Registration type.                  */
/*                                                                   */
/*  Output:          paddress - Call address of a routine.           */
/*                                                                   */
/*  Return Value:    Valid RXSUBCOM return codes                     */
/*                                                                   */
/*********************************************************************/

LONG  RegLoad(
  PSZ name,                            /* the Subcommand Environment */
  PSZ dll,                             /* Module name of its' DLL    */
  LONG  type,                          /* Load type.                 */
  PFN *paddress,                       /* Function addr (returned if */
                                       /* function properly loaded). */
  PULONG calltype,                     /* type of DLL procedure      */
  PVOID *plib)
{                                      /*                            */
  APIBLOCK *cblock;                    /* Working ptr, current block */
  PFN load_address;                    /* Function's addr (from load)*/
  LONG rc = 0;
  char ModuleNameBuffer[MAXNAME + 40]={0};  /* full library name     */
  char *p;                             /* array pointer              */
  int len;                             /* length of the module name  */
  int i;                               /* loop counter               */
  ULONG pblock = 0;                    /* Init 0 for entry block     */
#ifdef LINUX
  const char *error;                   /* error message              */
#else
  RXFUNCBLOCK *funcblock;              /* Base for function blocks   */
  PRXINITFUNCPKG InitFunc;             /* Pointer returned from load */
#endif

//EnterMustComplete();                 /*start of critical section   */
  APISTARTUP(SECHAIN);                 /* do common entry code       */
#ifdef ORXLD_DEBUG
  fprintf(stderr,"*I* %s: SEARCH for Lib fnc name %s.\n", __FILE__ ,name);
#endif

//cblock = search(name, dll, type);    /* Find data for the routine. */
  cblock = RegSearch(name, type, 'A'); /* Find data for the routine. */

  if (!cblock)
    rc = RXSUBCOM_NOTREG;              /* If not there, set the error*/
  else {                               /* OTHERWISE                  */
 /* if ((cblock->apipid ==  getpid() )  * If new PID, load lib again */
    if     (cblock->apiaddr)           /* the func addr is non-Zero  */
    {
      rc = RXSUBCOM_OK;                /* then function loaded as EXE*/
      *paddress =                      /* Return its address.        */
          cblock->apiaddr;
    }
    else                               /* Otherwise, get the address */
    {                                  /* of the entry point in the  */
                                       /* DLL.                       */
  /**** generate the full library name *******************************/

      len = (int)strlen(cblock->apidll_name);
      if (len < 1)
        fprintf(stderr, " *E* There is no library name defined!");

     if ( cblock->apimod_handle == NULL )
     {
#ifdef __xlC__
       if (!strchr( cblock->apidll_name, '/' ))
       {
#endif
         if (len > (MAXNAME - 7))      /* lib3+4.so) prevent longer then buffer */
         {
           fprintf(stderr, " *E* The name of the library %s  is to long !",
                                                         cblock->apidll_name);
           len = (MAXNAME - 7);
         }
         p = ModuleNameBuffer;
         strcpy(p, "lib");                /* copy 'lib' into the buffer */
         p += 3;
         /* check for REXXUTIL to provide compatibility to OS/2 sources */
         if(!strcmp(cblock->apidll_name,"REXXUTIL"))
           strcpy(p,"rexxutil");          /* copy the lowercase version */
         else
           strncpy(p, cblock->apidll_name, len); /* add the module name */
         p += len;
         strcpy(p, ORX_SHARED_LIBRARY_EXT);              /* add the shared library extension    */
                                          /* to make the library name   */
                                          /* complete                   */
                  /* Load the specified library of CREXX (start)        */
#ifdef __xlC__
       }
       else                               /* add the module name        */
       {
          if (len > (MAXNAME - 1))        /* prevent longer then buffer */
          {
            fprintf(stderr, " *E* The name of the library %s is to long!",
                                                         cblock->apidll_name);
            len = (MAXNAME - 1);
          }
          strncpy( ModuleNameBuffer, cblock->apidll_name, len );

#ifdef ORXLD_DEBUG
          fprintf(stderr," *I* Load or check Library %s.\n", ModuleNameBuffer);
#endif
                  /* Load the specified library of CREXX (start)        */
          if (!(InitFunc = (PRXINITFUNCPKG)load(ModuleNameBuffer,0,NULL)))
          {
            if ( InitFunc == NULL ) {
              fprintf(stderr, " *E* Unable to load library %s !\nError message: errno = %d;",\
                       ModuleNameBuffer, errno);
              perror(" REXXAPI");
              rc = 1;
//            EnterMustComplete();                 /*start of critical section */
              pblock = cblock->next;               /* Save entry of chain      */
                                                   /* free the block itself    */
              RxFreeAPIBlock( apidata->baseblock[type] , sizeof(APIBLOCK));
//            if ( apidata->sememtop > 1 )
//              apidata->baseblock[type] = apidata->sememtop -  sizeof(APIBLOCK);
//            else
//              apidata->baseblock[type] = NULL;
              if((apidata->baseblock[REGSUBCOMM] == 0 ) /* if all chains empty */
                  && (apidata->baseblock[REGSYSEXIT] == 0 )
                  && (apidata->baseblock[REGFUNCTION] == 0 ))
              {
                 removeshmem(apidata->sebasememId);   /* remove the se memory       */
                 detachshmem(apidata->sebase);   /* detach it to force the deletion */
                 apidata->sebase = NULL;              /* reset the memory anchor    */
              }
//            ExitMustComplete();                   /* end of critical section   */
            }
            else
              *plib = *InitFunc;
          }
          if ( rc == NULL ) {
            /* Call the initialization routine for the library (which should */
            /* be the function pointer returned to us by load).              */
            rc = (*InitFunc)(&funcblock);
            if (rc) {                          /* If routine indicates error,*/
                                               /* tell the user.             */
              fprintf(stderr," *E* Library load routine gave error %d.\n",rc);
              APICLEANUP(SECHAIN);             /*  release shared resources  */
              return(rc);                      /* don't load anything        */
            } /* endif */


           /* Now run through the array of func blocks, adding them to the  */
           /* list of external functions or subcommand handlers.  Note that */
           /* we use the external function types in all cases, but since    */
           /* the only thing affected is the function pointers, there's no  */
           /* problem - the RXFUNCBLOCK and RXSUBCOMBLOCK types are         */
           /* otherwise identical.                                          */
#ifdef ORXLD_DEBUG
            fprintf(stderr,"*I* Lookup Function %s \n", cblock->apidll_proc);
#endif
            for (i=0; funcblock[i].name!=NULL; i++) {
                /*fprintf(stderr,"Function %s \n", funcblock[i].name);      */
                                               /* If this name is found,     */
                if ( strcmp(funcblock[i].name,cblock->apidll_proc) == 0 )
                  break;
            } /* endfor */

            if ( funcblock[i].function && (rc == NULL) ) {
#ifdef ORXLD_DEBUG
              fprintf(stderr,"*I* Function %s in Library found.\n",
                                                        funcblock[i].name);
#endif
              cblock->apimod_handle = NULL;
              cblock->apiaddr = funcblock[i].function;
              *paddress = funcblock[i].function;
              rc = RXSUBCOM_OK;
            }
            else
              rc = 1;                         /* could not resolve          */
         }
       }
#endif
     }
                  /* Load the specified library of CREXX (end)              */
  /*********** load the specific library *****************************/
#ifdef __xlC__
      if (!strchr( ModuleNameBuffer, '/' ))
      {
#endif
        if ( cblock->apimod_handle != NULL )
        {
           *plib = cblock->apimod_handle;        /* Get lib handle          */
        }
        else
        {
          if (!(*plib = dlopen(ModuleNameBuffer, RTLD_NOW )))
          {
             strcpy( ModuleNameBuffer, "lib" ); // was /usr/lib/lib
             strcat( ModuleNameBuffer, cblock->apidll_name );
             strcat( ModuleNameBuffer, ORX_SHARED_LIBRARY_EXT );
             if (!(*plib = dlopen(ModuleNameBuffer, RTLD_NOW )))
             {
                                                                 /* immediately        */
                fprintf(stderr, " *E* Unable to load library: %s !\nError message: %s\n",
                        ModuleNameBuffer, dlerror());
                rc = 1;                                     /* next 8 lines   */
             }
          }
        }
#ifdef ORXLD_DEBUG
        fprintf(stderr,"*I* %s: Lookup Function %s by dlsym \n", __FILE__
                ,cblock->apidll_proc);
#endif
        if ( rc == 0 ) {                         /* get the address           */
          load_address = dlsym(*plib, cblock->apidll_proc);
          if (load_address) {
             cblock->apimod_handle = *plib;
             pLibSave = *plib;
             strcpy( szLibName, cblock->apidll_name);
             cblock->apiaddr = load_address;
             *paddress = load_address;
             rc = RXSUBCOM_OK;
#ifdef ORXLD_DEBUG
          fprintf(stderr,"*i* %s Function loaded %s by dlsym \n", __FILE__
                  ,cblock->apidll_proc);
#endif
          }
          else {
            rc = 1;                         /* could not resolve          */
            fprintf(stderr, " *E* Function: %s not found in library: %s!\nError message: %s\n",
                    cblock->apidll_proc, ModuleNameBuffer, dlerror());
          }
        }
        if ( rc == 1 )
        {
//         EnterMustComplete();                 /*start of critical section */
           pblock = cblock->next;               /* Save entry of chain      */
                                                /* free the block itself    */
           RxFreeAPIBlock( apidata->baseblock[type] , sizeof(APIBLOCK));
//         if ( apidata->sememtop > 1 )
//            apidata->baseblock[type] = apidata->sememtop -  sizeof(APIBLOCK);
//         else
//            apidata->baseblock[type] = NULL;
           if((apidata->baseblock[REGSUBCOMM] == 0 ) /* if all chains empty */
               && (apidata->baseblock[REGSYSEXIT] == 0 )
               && (apidata->baseblock[REGFUNCTION] == 0 ))
           {
              removeshmem(apidata->sebasememId);   /* remove the se memory       */
              detachshmem(apidata->sebase);   /* detach it to force the deletion */
              apidata->sebase = NULL;              /* reset the memory anchor    */
           }
//         ExitMustComplete();                   /* end of critical section      */
        }
#ifdef __xlC__
      }
#endif
/*    if ( rc == NULL )                 * Change owner of cblock */
/*       cblock->apipid = getpid();     * copy the process ID    */
    }                                  /* If exe ... else is dll.... */
  }                                    /* If registered.             */
  APICLEANUP(SECHAIN);                 /*  release shared resources  */
//ExitMustComplete();                   /* end of critical section   */
  return (rc);
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
//PAPIBLOCK search(
//  PSZ name,
//  PSZ dll,
//  LONG  type )
//{
//  PAPIBLOCK cblock = NULL;             /* return pointer for function*/
//  if ((!dll) ||                        /* No DLLNAME?, search exe 1st*/
//      (!strlen(dll)))
//    cblock = exesearch(name, type);    /* set cblock with the result.*/
//  if (!cblock)                         /* If couldn't find it search */
//    cblock = dllsearch(name,           /* set cblock with result.    */
//        dll, type);
//  return (cblock);                     /* Return block found or NULL */
//}



/*********************************************************************/
/*                                                                   */
/*  Function Name: RegSearch                                         */
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
/*  Entry Point:   RegSearch( name, type )                           */
/*                                                                   */
/*  Parameter(s):    name     -  The handler name to locate.         */
/*                   type     -  The Registration Type.              */
/*                                                                   */
/*  Return Value:    NULL or the address of the block if found.      */
/*                                                                   */
/*********************************************************************/

PAPIBLOCK RegSearch(                   /* Function Declaration.      */
  PSZ name,                            /* Name of function to find   */
  LONG  type,                          /* Type of name.              */
  CHAR  chMstBlk )                     /* Search All or Muster only  */
{
  ULONG     cblock;                /* Working ptr, current block     */
  ULONG     previous;              /* prior block for movement       */
  ULONG     sblock;                /* Save main block offset         */
  PAPIBLOCK nblock;                /* Working ptr, new block              */
  pid_t     pidProcID = 0;                  /* keep process id       */
  pid_t     pidOwnID = 0;                   /* keep owner   id       */
//PAPIBLOCK dbgblock;                       /* Working ptr, debug   block */
  cblock   = 0;                    /* Working ptr, current block     */
  previous = 0;                    /* prior block for movement       */
  sblock = 0;                      /* Save main block offset         */
  nblock = NULL;                   /* Working ptr, new     block          */
  cblock = apidata->baseblock[type];        /* Working ptr, current block */


    pidOwnID  = getpid();
#if defined( HAVE_GETPGRP )
    pidProcID = getpgrp();
#else
    pidProcID = pidOwnID;
#endif

    if (chMstBlk == 'A' )                  /* Search all c_blocks             */
    {
      while (cblock)                       /* Run through the list       */
      {                                    /* Comp name with passed name */
//      dbgblock = SEDATA(cblock);         /* For debug only             */
        /* 1. Search for own block or a free matching block of the main reg  */
        if ( ((SEDATA(cblock)->apiownpid == pidOwnID) ||   /* PID or free    */
              (SEDATA(cblock)->apiownpid == 0)) &&         /* main flock and */
              (!rxstricmp(SEDATA(cblock)->apiname,name)) && /* Matching name */
              (strlen(SEDATA(cblock)->apidll_name)) )      /* and A DLL      */
        {                                  /* if found a matching name &     */
//        EnterMustComplete();             /* start of critical section      */
          SEDATA(cblock)->apiownpid = pidOwnID; /* for reuse of cblock       */
          if (previous)                    /* if not at front                */
          {
            SEDATA(previous)->next =       /* rearrange the chain to move    */
                   SEDATA(cblock)->next;   /* this block to the front        */
            SEDATA(cblock)->next=          /* of the list.  We are likely    */
              apidata->baseblock[type];    /* to need it again soon          */
            apidata->baseblock[type] = cblock;
            if ( ( pLibSave != NULL ) &&    /* If lib handle exists get it   */
                 (!rxstricmp(SEDATA(cblock)->apidll_name, szLibName)) )
               SEDATA(cblock)->apimod_handle = pLibSave;
            else
               SEDATA(cblock)->apimod_handle = NULL;
          }
//        ExitMustComplete();              /* end of critical section        */
          return ((PAPIBLOCK)SEDATA(cblock)); /* matching pid, return block  */
        }

        /* 2. Search for own block of an exec    registration                */
        if ( (SEDATA(cblock)->apiownpid == pidOwnID)  &&   /* PID and       */
             (!rxstricmp(SEDATA(cblock)->apiname,name)) && /* Matching name */
             (!strlen(SEDATA(cblock)->apidll_name)) )      /* and NOT A DLL */
        {
          if (previous)                    /* if not at front            */
          {
//          EnterMustComplete();           /* start of critical section      */
            SEDATA(previous)->next =       /* rearrange the chain to move*/
                   SEDATA(cblock)->next;   /* this block to the front    */
            SEDATA(cblock)->next=          /* of the list.  We are likely*/
              apidata->baseblock[type];    /* to need it again soon      */
            apidata->baseblock[type] = cblock;
            SEDATA(cblock)->apimod_handle = NULL;
//          ExitMustComplete();            /* end of critical section        */
          }
          return ((PAPIBLOCK)SEDATA(cblock)); /* matching pid, return block  */
        }
        previous = cblock;                 /* remember this one,             */
        if ( cblock > 0 )
           cblock = SEDATA(cblock)->next;  /* and continue the search        */
      }
    }                                      /* END of Search in while loop    */

    cblock = apidata->baseblock[type];   /* Working ptr, current block */
    while (cblock)                       /* Run through the list       */
    {                                    /* Comp name with passed name */
      /* 3. Search for main registration block and save offset             */
      if ( (SEDATA(cblock)->apiFunRegFlag == 0 ) &&  /* The main reg block */
           (!rxstricmp(SEDATA(cblock)->apiname,name)) &&  /* Matching name */
           (strlen(SEDATA(cblock)->apidll_name)) )   /* and a dll          */
      {
         sblock = cblock;                /* Save offset of main reg block  */
         if (SEDATA(cblock)->apiownpid == pidOwnID)
            cblock = 0;                  /* End the while loop             */
      }
      /* Get loop going if nothing found yet                               */
      previous = cblock;                 /* remember this one,             */
      if ( cblock > 0 )
         cblock = SEDATA(cblock)->next;  /* and continue the search        */
    }                                    /* END of Search                  */

    /* Check whether a not free main block has been found for a copy       */
    if ( (cblock == 0 ) && (sblock > 0 ) )
    {
      if ( (SEDATA(sblock)->apiFunRegFlag == 0 )  && /* The main reg block */
           (SEDATA(sblock)->apiownpid != pidOwnID) && /* Not the PID       */
           (SEDATA(sblock)->apiownpid > 0 ) )         /* and not free      */
      {
        /***************************************************************/
        /* Allocate the APIBLOCK.  If we succeed, fill in the registra-*/
        /* tion data.  Otherwise, return RXSUBCOM_NOMEM.               */
        /***************************************************************/

//      EnterMustComplete();                 /* enter critical section     */
        if ( ! RxAllocAPIBlock( &nblock, name, NULL, NULL))
        {
          /*************************************************************/
          /* Complete the registration.                                */
          /*************************************************************/
          memcpy((PVOID)nblock, (PVOID)SEDATA(sblock), APISIZE);
          nblock->apipid = pidProcID;         /* fill in process ID and    */
          nblock->apiownpid = pidOwnID;       /* fill in owner   ID and    */
          nblock->apiFunRegFlag = 1;          /* mark no main registration */
          nblock->apiaddr = NULL;
          if ( ( pLibSave != NULL ) &&    /* If lib handle exists get it   */
               (!rxstricmp(nblock->apidll_name, szLibName)) )
               nblock->apimod_handle = pLibSave;
          else
               nblock->apimod_handle = NULL;
          nblock->next =                    /* Next pointer set to old top */
              apidata->baseblock[type];
                                            /* Make this block the top     */
          apidata->baseblock[type] = ((ULONG)(((char*)nblock)-(apidata->sebase)));
        }
//      ExitMustComplete();                /* end of critical section      */
        return(nblock);                    /* matching pid, return block   */
      }
      if ( (sblock > 0) && ((SEDATA(sblock)->apiownpid == pidOwnID) ||
           (SEDATA(sblock)->apiownpid == 0)) )
      {
         cblock = sblock;
      }
    }

   if(cblock == 0)
     return((PAPIBLOCK)NULL);
   else
     return ((PAPIBLOCK)SEDATA(cblock));   /* matching pid, return block   */
}


/*$PE*/
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
/*********************************************************************/
//APIBLOCK *dllsearch(
//  PSZ name,                            /* Name to find               */
//  PSZ dll,                             /* Dynalink Name to find.     */
//  LONG  type )                         /* Type of name to find.      */
//{
//  ULONG     cblock;                    /* Working ptr, current block */
//  ULONG     previous;                  /* prior block for movement   */
//
//  previous = NULL;                     /* no prior block yet         */
//  cblock = apidata->baseblock[type];   /* Working ptr, current block */
//
//  if (!dll)                            /* Treat NULL dlls as 0 length*/
//    dll = "";                          /* names.                     */
//  while (cblock) {                     /* while not at end of chain  */
//    if( (strlen(SEDATA(cblock)->apidll_proc))&&/* If no proc name, is .exe   */
//                                       /* which we don't want.       */
//    /*  (strlen(SEDATA(cblock)->apidll_proc)))&&                     */
//                                       /* If environment names match */
//       (!rxstricmp(SEDATA(cblock)->apiname,name))
//        && (!dll[0]                    /* And passed dll name is NULL*/
//        || (strlen(SEDATA(cblock)->apidll_name)/* Or exists a stored dll name*/
//        && (!rxstricmp(SEDATA(cblock)->apidll_name, /* And           */
//        dll))                          /* it matches the passed one  */
//        ))) {                          /* Then                       */
//      if (previous) {                  /* if not at front            */
//        EnterMustComplete();           /* start of critical section  */
//        SEDATA(previous)->next =       /* rearrange the chain to move*/
//               SEDATA(cblock)->next;   /* this block to the front    */
//        SEDATA(cblock)->next=          /* of the list.  We are likely*/
//          apidata->baseblock[type];    /* to need it again soon      */
//        apidata->baseblock[type] = cblock;
//        ExitMustComplete();            /* end of critical section    */
//      }
//      if(cblock == NULL)
//        return((PAPIBLOCK)0);
//      else
//        return (SEDATA(cblock));       /* matching pid, return block */
//    }
//    previous = cblock;                 /* remember this one,         */
//    cblock = SEDATA(cblock)->next;     /* and continue the search    */
//  }                                    /* END of Search              */
//  if(cblock == NULL)
//    return((PAPIBLOCK)0);
//  else
//    return (SEDATA(cblock));           /* matching pid, return block */
//}




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

LONG dllcheck(
  PSZ name,
  PSZ dllname,
  LONG  type )
{
  ULONG  rc = RXSUBCOM_OK;             /* Function return code.      */
  pid_t  pidOwnID;                     /* keep process id            */
  pidOwnID = getpid();                 /* Save time in loop          */
                                       /* Working ptr, current block */
  ULONG cblock = apidata->baseblock[type];
  while (cblock) {                     /* While another block        */
                                       /* Com name with passed name  */
    if (!rxstricmp(SEDATA(cblock)->apiname,
        name)                          /* if found a matching name   */
        && SEDATA(cblock)->apiownpid == pidOwnID) {
      rc = RXSUBCOM_DUP;               /* then set the duplicate rc  */
                                       /* if a registered dll name & */
      if (strlen(SEDATA(cblock)->apidll_name) &&
                                       /* it matches the         */
          (!rxstricmp(SEDATA(cblock)->apidll_name,
          dllname))) {                 /* dll name passed in         */
        rc = RXSUBCOM_NOTREG;          /* then set appropiate rc     */
        cblock = 0;                    /* force a quick end.         */
      }                                /* END of DLL comparison      */
    }                                  /* END of NAME comparison     */
                                       /* If more blocks incrmnt ptr */
    if (cblock) cblock = SEDATA(cblock)->next;
  }                                    /* END of Search              */
  return (rc);
}


/*$PE*/
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

LONG execheck(
  PSZ   name,
  LONG  type )
{
  ULONG  rc = RXSUBCOM_NOTREG;         /* Function return code.NOT OK*/
  pid_t       pidOwnID;                /* keep process id            */
  pidOwnID = getpid();                 /* Save time in loop          */
                                       /* Working ptr, current block*/
  ULONG cblock = apidata->baseblock[type];
  while (cblock) {                     /* Run through the list       */
                                       /* Comp name with passed name */
    if (!rxstricmp(SEDATA(cblock)->apiname,
        name)) {                       /* if found a matching name   */
      if (! strlen(SEDATA(cblock)->apidll_name)){/* if not a dll type and*/
        if (SEDATA(cblock)->apiownpid == pidOwnID) {
/*      if (PIDCMP(SEDATA(cblock))) {   * matching process info      */
/*        rc = RXSUBCOM_NOTREG;         * then must be registered &  */
          rc = RXSUBCOM_DUP;           /* then must be registered &  */
                                       /* cannot be re-registered    */
          cblock = 0;                  /* so force a quick end.      */
        }
      }
      else                             /* this is a DLL match        */
        rc = RXSUBCOM_NOTREG;          /* then set the duplicate rc  */
/*      rc = RXSUBCOM_DUP;              * then set the duplicate rc  */
    }                                  /* END of NAME comparison     */
                                       /* If more blocks incrmnt ptr */
    if (cblock) cblock = SEDATA(cblock)->next;
  }                                    /* END of Search              */
  return (rc);                         /* Return the return code     */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxShutDownAPI moved to aixrxapi.c             */
/*                                                                   */
/*********************************************************************/

#ifdef AIX
#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************/
/*                                                                   */
/*  Function Name:  RexxRegisterFunction   ( CREXX function )        */
/*                                                                   */
/*  Description:    Registration function for the external function  */
/*                  interface.  ( see RexxRegisterFunctionExe )      */
/*                                                                   */
/*********************************************************************/

APIRET APIENTRY RexxRegisterFunction(
  PSZ   EnvName,                       /* Subcom name                */
  PFN   EntryPoint )                   /* DLL routine name           */
{
  ULONG  rc;                           /* Function return code.      */
                                       /* Register the subcommand.   */
  rc = RegRegisterExe(EnvName, EntryPoint, NULL,
                      REGFUNCTION,PT_32BIT);
  return (rc);                         /* and exit with return code  */
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:  RexxRegisterSubcom     ( CREXX function )        */
/*                                                                   */
/*  Description:    Registration function for the subcommand         */
/*                  interface.  (see RexxRegisterSubcomExe)          */
/*********************************************************************/

APIRET APIENTRY RexxRegisterSubcom(
  PSZ   EnvName,                       /* Subcom name                */
  PFN   EntryPoint,                    /* DLL routine name           */
  PUCHAR UserArea )                    /* User data                  */
{
  ULONG  rc;                           /* Function return code.      */
                                       /* Register the subcommand    */
                                       /* (as a 32-bit callout       */
  rc = RegRegisterExe(EnvName, EntryPoint, UserArea,
                      REGSUBCOMM,PT_32BIT);
  return (rc);                         /* and exit with return code  */
}

#ifdef __cplusplus
}
#endif
#endif
