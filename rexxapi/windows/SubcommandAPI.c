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
/*      Module Name:   WINSEAPI.C                                    */
/*                                                                   */
/* Descriptive Name:   Subcommand Environment Control Functions.     */
/*                                                                   */
/*  Entry Points:  RexxRegisterSubcomExe() Register an environment   */
/*                 RexxRegisterSubcomDll() Register an environment   */
/*                 RexxLoadSubcom()        Load an environment       */
/*                 RexxCallSubcom()        Execute environment entry */
/*                 RexxQuerySubcom()       Query environment entry   */
/*                 RexxDeregisterSubcom()  Drop an environment       */
/*                 RexxRegisterExitExe()   Register a system exit    */
/*                 RexxRegisterExitDll()   Register a system exit    */
/*                 RexxDeregisterExit()    Drop a system exit        */
/*                 RexxQueryExit()         Query a system exit       */
/*                 RexxRegisterFunctionExe() Register a REXX function*/
/*                 RexxRegisterFunctionDll() Register a REXX function*/
/*                 RexxDeregisterFunction()  Drop a REXX function    */
/*                 RexxQueryFunction()     Query a REXX function     */
/*                 RexxCallFunction()      Execute a REXX function   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
/* Please note the following:                                        */
/*                                                                   */
/* Functions in this module manipulate data that must be available   */
/* to all processes that call REXX API functions.  These processes   */
/* may invoke the REXX interpreter, or make direct calls to the      */
/* API routines.                                                     */
/*                                                                   */
/* In addition, functions in this module may establish data that     */
/* must persist after the calling process terminates.                */
/*                                                                   */
/* To satisfy these requirements, the system must maintain a process */
/* that serves as a data repository.  Functions in this module then  */
/* give critical data to the repository, and the data persists as    */
/* long as the repository process continues running.                 */
/*                                                                   */
/*********************************************************************/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>                    /* Included for string funcs  */
#include <malloc.h>                    /* for memory allocation      */
#include "rexx.h"
#include "APIServiceTables.h"          /* common api definitions     */
#include "RexxAPIManager.h"

#include "APIUtil.h"
#include "Characters.h"

#include "RexxAPIService.h"
#include "APIServiceMessages.h"
#include "APIServiceSystem.h"
#include "RexxInternalApis.h"          /* Get private REXXAPI API's         */

/*********************************************************************/
/*                                                                   */
/*  Registration types.                                              */
/*                                                                   */
/*********************************************************************/

#define  REGSUBCOMM    0               /* Register a subcommand.     */
#define  REGSYSEXIT    1               /* Register a system exit.    */
#define  REGFUNCTION   2               /* Register a function.       */

#define  stdout_handle 1               /* Standard Output.           */
#define MAXARGS       20               /* same as define in IXXREXX.H*/
#define YES           1
#define NO            0
#define isstr(x)     ( x?x:"" )
#define PT_32BIT      1

int int_var = 1;

extern process_id_t queue_get_pid(size_t* envchars);

/*********************************************************************/
/* Errors returned by the module loader and address finder.          */
/*********************************************************************/

typedef struct _loaderr {
   int       errors[2];                /* two entries                */
}  loaderr;

/*********************************************************************/
/* Errors returned by the module loader and address finder.          */
/*********************************************************************/

static loaderr load_errors[3] = {
               { RXSUBCOM_LOADERR,     /* Module not found.          */
                 RXSUBCOM_LOADERR },   /* ENtry point not found.     */
               { RXEXIT_LOADERR,       /* Module not found.          */
                 RXEXIT_LOADERR },     /* ENtry point not found.     */
               { RXFUNC_MODNOTFND,     /* Module not found.          */
                 RXFUNC_ENTNOTFND }    /* ENtry point not found.     */
              } ;

extern REXXAPIDATA * RexxinitExports;   /* Global state data  */

/*********************************************************************/
/*                                                                   */
/* Function prototypes for local support routines.                   */
/*                                                                   */
/*********************************************************************/

int     RegQuery(const char *, const char *, unsigned short *, char *, int);
int     RegLoad(const char *, const char *, int, REXXPFN *);
int     RegDrop(const char *, const char *, int);
int     RegRegisterExe(const char *, REXXPFN, const char *, int);
int     RegRegisterDll(const char *, const char *, const char *, const char *, size_t, int);
APIBLOCK *memmgrsearch(const char *, const char *, int);
int memmgrcheckexe(const char *, int);
int memmgrcheckdll(const char *, const char *, int);

extern _declspec(dllimport) CRITICAL_SECTION nest;

/* Now need LRX for local init because RX is process global */
extern LOCALREXXAPIDATA RexxinitLocal;

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

RexxReturnCode
REXXENTRY
RexxRegisterSubcomDll(
  const char *   EnvName,                       /* Subcom name                */
  const char *   ModuleName,                    /* Name of DLL                */
  const char *   EntryPoint,                    /* DLL routine name           */
  const char *   UserArea,                     /* User data                  */
  size_t DropAuth )                    /* Drop Authority             */
{
                                       /* Register the subcommand.   */
  return RegRegisterDll(EnvName, ModuleName, EntryPoint, UserArea,
                      DropAuth, REGSUBCOMM);
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

RexxReturnCode
REXXENTRY
RexxRegisterSubcomExe(
  const char *  EnvName,               /* Subcom name                */
  REXXPFN   EntryPoint,                /* DLL routine name           */
  const char *UserArea )               /* User data                  */
{
                                       /* Register the subcommand    */
                                       /* (as a 32-bit callout       */
  return RegRegisterExe(EnvName, EntryPoint, UserArea, REGSUBCOMM);
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

RexxReturnCode
REXXENTRY
RexxDeregisterSubcom(
  const char * name,                            /* Environment Name           */
  const char * dllname )                        /* Associated Name (of DLL)   */
{
  ULONG  rc;                           /* Function return code.      */
  rc = RegDrop(name, dllname, REGSUBCOMM);/* Drop the subcommand.    */
  return (rc);                         /* and exit with return code  */
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

RexxReturnCode
REXXENTRY
RexxQuerySubcom(
  const char *     name,               /* Environment Name           */
  const char *     dll,                /* Associated Name (of DLL)   */
  unsigned short * exist,              /* existence information      */
  char   *userword )                   /* data from registration     */
{
  return RegQuery(name, dll, exist, userword, /* Perform the query.    */
      REGSUBCOMM);
}

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

RexxReturnCode REXXENTRY RexxLoadSubcom(
  const char * name,                   /* Name of Subcommand Environ */
  const char * dll )                   /* Module name of its' DLL    */
{
  REXXPFN a;                           /* Address of Subcom.         */

                                       /* Load routine into memory   */
  return RegLoad(name, dll, REGSUBCOMM, &a);
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

RexxReturnCode REXXENTRY RexxCallSubcom(
    const char *name,             /* Name of Subcommand Environ */
    const char *dll,              /* Module name of its DLL     */
    PCONSTRXSTRING cmd,           /* Command string to be passed*/
    unsigned short *flags,        /* Stor for error flag notice */
    wholenumber_t *sbrc,          /* Stor for rc from handler   */
    PRXSTRING rv)                 /* Stor for returned string   */
{
  RexxSubcomHandler *subcom_addr;
  RexxReturnCode  rc;                          /* Function return code.      */

                                       /* Load the handler           */
  if (!(rc=RegLoad(name, dll, REGSUBCOMM, (REXXPFN *)&subcom_addr)))
  {
      *sbrc = (                        /* Call subcom environment w/ */
          (* subcom_addr ) (           /* The ptr to environmnt entry*/
          cmd,                         /* The command to perform     */
          flags,                       /* Pointer to the error flags */
          rv                           /* returned string after call */
          ));                          /* return value is set to SBRC*/
        rc = RXSUBCOM_OK;              /* Set RxSubComExecute return */
  }
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

RexxReturnCode
REXXENTRY
RexxRegisterExitDll(
  const char *   EnvName,              /* Exit name                  */
  const char *   ModuleName,           /* Name of DLL                */
  const char *   EntryPoint,           /* DLL routine name           */
  const char *   UserArea,             /* User data                  */
  size_t DropAuth )                    /* Drop Authority             */
{
                                       /* Register the subcommand.   */
  return RegRegisterDll(EnvName, ModuleName, EntryPoint, UserArea,
                      DropAuth, REGSYSEXIT);
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

RexxReturnCode
REXXENTRY
RexxRegisterExitExe(
  const char *EnvName,               /* exit name                  */
  REXXPFN     EntryPoint,            /* DLL routine name           */
  const char *UserArea )             /* User data                  */
{
                                       /* Register the exit          */
  return RegRegisterExe(EnvName, EntryPoint, UserArea, REGSYSEXIT);
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

RexxReturnCode
REXXENTRY
RexxDeregisterExit(
  const char * name,                            /* Environment Name           */
  const char * dllname )                        /* Associated Name (of DLL)   */
{
  return RegDrop(name, dllname, REGSYSEXIT);/* Drop the subcommand.    */
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

RexxReturnCode
REXXENTRY
RexxQueryExit(
  const char * name,                   /* Environment Name           */
  const char * dll,                    /* Associated Name (of DLL)   */
  unsigned short *exist,               /* existence information      */
  char   *userword )                   /* data from registration     */
{
  return RegQuery(name, dll, exist, userword, /* Perform the query.    */
      REGSYSEXIT);
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxResolveExit                                 */
/*                                                                   */
/*  Description:     Resolves a system exit.                         */
/*                                                                   */
/*  Entry Point:     RexxResolveExit                                 */
/*                                                                   */
/*  Parameter(s):    name       -  Name of the desired system exit   */
/*                   handler    -  returned handler                  */
/*                                                                   */
/*  Return Value:    0 if the exit was resolved or rc from the       */
/*                   resolution                                      */
/*                                                                   */
/*********************************************************************/
int REXXENTRY RexxResolveExit(
  const char * name,                   /* Exit name.                 */
  REXXPFN *handler)
{
  return RegLoad(name, NULL, REGSYSEXIT, handler);
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

RexxReturnCode
REXXENTRY
RexxRegisterFunctionDll(
  const char *   EnvName,                       /* Subcom name                */
  const char *   ModuleName,                    /* Name of DLL                */
  const char *   EntryPoint )                   /* DLL routine name           */
{
                                       /* Register the subcommand.   */

  return RegRegisterDll(EnvName, ModuleName, EntryPoint, NULL,
                      RXSUBCOM_DROPPABLE, REGFUNCTION);
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

RexxReturnCode
REXXENTRY
RexxRegisterFunctionExe(
  const char *   EnvName,              /* Subcom name                */
  REXXPFN   EntryPoint )               /* DLL routine name           */
{
                                       /* Register the subcommand.   */
  return RegRegisterExe(EnvName, EntryPoint, NULL, REGFUNCTION);
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

RexxReturnCode
REXXENTRY
RexxDeregisterFunction(
  const char * name )                  /* Environment Name           */
{

  return RegDrop(name, NULL, REGFUNCTION); /* Drop the function      */
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

RexxReturnCode
REXXENTRY
RexxQueryFunction(
  const char * name )                  /* Environment Name           */
{
  RexxReturnCode  rc;                          /* General Return code holder */
  USHORT exist;                        /* existance flage            */

  rc = RegQuery(name, NULL, &exist, NULL,  /* Perform the query.   */
                  REGFUNCTION);
                                       /* set the proper return code */
  rc = (exist)?RXFUNC_OK:RXFUNC_NOTREG;

  return (rc);                         /* and exit with return code  */
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:      RexxResolveRoutine                           */
/*                                                                   */
/*  Description:        find and call an external function           */
/*                                                                   */
/*  Entry Points:       sys_external(dname,argc,argv,result,type)    */
/*                                                                   */
/*  Inputs:             dname  - the name of the function to call    */
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
int REXXENTRY RexxResolveRoutine(const char *name, REXXPFN *handler)
{
    return RegLoad(name, NULL, REGFUNCTION, handler);
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RegRegisterDll                                  */
/*                                                                   */
/*  Description:     Registration engine for the subcommand and exit */
/*                   interface.  All programs wishing to act as      */
/*                   subcommand environments or exits for REXX must  */
/*                   first register through this function.           */
/*                                                                   */
/*                   The function adds new registration blocks to    */
/*                   the subcommand registration table. Uses         */
/*                   functions in REXXAPI for memory allocation      */
/*                   functions.                                      */
/*                                                                   */
/*  Entry Point:     RegRegisterDll                                  */
/*                                                                   */
/*  Parameter(s):    EnvName    - The name of the registered handler.*/
/*                   ModuleName - The name of a DLL library          */
/*                   EntryPoint - The name of a DLL procedure        */
/*                   UserArea   - The saved user information         */
/*                   DropAuth   - Handler drop authority             */
/*                   type       - The type of registration           */
/*                                                                   */
/*  Return Value:    Valid RXSUBCOM return codes                     */
/*                                                                   */
/*********************************************************************/

int   RegRegisterDll(
  const char *   EnvName,              /* Subcom name                */
  const char *   ModuleName,           /* Name of DLL                */
  const char *   EntryPoint,           /* DLL routine name           */
  const char *   UserArea,             /* User data                  */
  size_t DropAuth,                     /* Drop Authority             */
  int   type )                         /* Registration type.         */
{
  PAPIBLOCK cblock ;                   /* Working ptr, current block */
  HAPIBLOCK PtempH;
  int  rc, rc2;

  APISTARTUP_API();                        /* do common entry code       */

  rc = memmgrcheckdll(EnvName, ModuleName, type);

                                       /* if we can still register...*/
  if (rc == RXSUBCOM_OK || rc == RXSUBCOM_DUP) {

      /***************************************************************/
      /* Allocate the APIBLOCK, filling in the registration data     */
      /* Otherwise, return RXSUBCOM_NOEMEM.                           */
      /***************************************************************/

    if( ! FillAPIComBlock(&PtempH,
                     EnvName, ModuleName, EntryPoint)) {

        /*************************************************************/
        /* Complete the registration.                                */
        /*************************************************************/
       cblock = (PAPIBLOCK)PtempH;

       cblock->apiaddr = NULL;        /* no entry point address     */
       if (UserArea)                  /* copy the user data         */
         memcpy(cblock->apiuser, UserArea, USERLENGTH);
       cblock->apidrop_auth=DropAuth; /* and drop authority         */

       rc2 = (ULONG)MySendMessage(RXAPI_REGREGISTER,
                                (WPARAM)0,
                              (WPARAM)type);
        if (rc2) rc = rc2;// by IH: keep RXSUBCOM_DUP
    }
    else rc = RXSUBCOM_NOEMEM;
  }
  APICLEANUP_API();
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

int  RegRegisterExe(
  const char *   EnvName,              /* Subcom name                */
  REXXPFN   EntryPoint,                /* DLL routine name           */
  const char *UserArea,                /* User data                  */
  int   type)                          /* Registration type.         */
{
  PAPIBLOCK cblock ;                   /* Working ptr, current block */
  HAPIBLOCK PtempH ;                   /*                            */
  int  rc, rc2;

  APISTARTUP_API();

  rc = memmgrcheckexe(EnvName, type);
                                       /* if we can still register...*/
  if (rc == RXSUBCOM_OK || rc == RXSUBCOM_DUP) {

      /***************************************************************/
      /* Allocate the APIBLOCK.  If we succeed, fill in the registra-*/
      /* tion data.  Otherwise, return RXSUBCOM_NOEMEM.               */
      /***************************************************************/

    if( ! FillAPIComBlock( &PtempH,
                     EnvName, NULL, NULL)) {

        /*************************************************************/
        /* Complete the registration.                                */
        /*************************************************************/
        cblock = (PAPIBLOCK)PtempH;

        cblock->apiaddr = EntryPoint;  /* fill in the entry point    */
        if (UserArea)                  /* copy the user data         */
          memcpy(cblock->apiuser, UserArea, USERLENGTH);
                                       /* EXEs are always NONDROP    */
        cblock->apidrop_auth=RXSUBCOM_NONDROP;
        rc2 = (ULONG)MySendMessage(RXAPI_REGREGISTER,
                                  (WPARAM)0,
                                  (WPARAM)type);

        if (rc2 != RXSUBCOM_OK) rc = rc2;
    }
    else rc = RXSUBCOM_NOEMEM;
  }
  APICLEANUP_API();
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

int   RegDrop(
  const char * name,                   /* Environment Name           */
  const char * dll,                    /* Associated Name (of DLL)   */
  int   type )                         /* Registration type.         */
{
  APIBLOCK *cblock = NULL;             /* Working ptr, current block */
  int  rc = RXSUBCOM_NOTREG;
  HAPIBLOCK PtempH;

  if (!API_RUNNING())
      return(RXSUBCOM_NOEMEM);

  if (!RX.baseblock[type]) return RXSUBCOM_NOTREG; /* No such se/fnc registered */

  APISTARTUP_API();

                                       /* all pointers NULL? this is */
                                       /* a call from DllMain to drop*/
                                       /* all registrations of funcs */
                                       /* for this process id        */
  if (name == NULL &&
      dll == NULL) {
    rc = (LONG) MySendMessage(RXAPI_PROCESSGONE, (WPARAM) type, (WPARAM) GetCurrentProcessId());
  } else {
  if (!FillAPIComBlock(&PtempH,
                   name, dll, NULL))
      rc = (LONG)MySendMessage(RXAPI_REGDEREGISTER,
                            (WPARAM)0,
                            (WPARAM)type);
  else rc = RXSUBCOM_NOEMEM;
  }

  APICLEANUP_API();
  return (rc);                         /* Return the return code     */
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

int   RegQuery(
  const char * name,                   /* Environment Name           */
  const char * dll,                    /* Associated Name (of DLL)   */
  unsigned short *exist,               /* existence information      */
  char * usrwrd,                       /* data from registration     */
  int   type )                         /* Registration type.         */
{
  PAPIBLOCK cblock;                    /* Working ptr, current block */
  LONG      rc;

  if (!API_RUNNING()) return(RXSUBCOM_NOEMEM);

  if (!RX.baseblock[type])
  {
      *exist = NO;
      return RXSUBCOM_NOTREG; /* No such se/fnc registered */
  }

  APISTARTUP_API();

  cblock = memmgrsearch(name, dll, type); /* Find the block or not.*/

  if (cblock) {                        /* If block found then it must*/
    *exist = YES;

    if (usrwrd)                        /* And get user word as well  */
       memcpy(usrwrd,cblock->apiuser, USERLENGTH);
    rc = RXSUBCOM_OK;                  /* Now we're done. And its OK.*/
  }                                    /* Otherwise                  */
  else {
    rc = RXSUBCOM_NOTREG;              /* It ain't thar, so say it.  */
    *exist = NO;
  }
  APICLEANUP_API();
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

int   RegLoad(
  const char * name,                   /* the Subcommand Environment */
  const char * dll,                    /* Module name of its' DLL    */
  int   type,                          /* Load type.                 */
  REXXPFN *paddress)                   /* Function addr (returned if */
                                       /* function properly loaded). */
{                                      /*                            */
  APIBLOCK *cblock;                    /* Working ptr, current block */
  REXXPFN load_address;                /* Function's addr (from load)*/
  int  rc;

  APISTARTUP_API();

  cblock = memmgrsearch(name,          /* Find data for the routine. */
                        dll, type);

  if (!cblock)
    rc = RXSUBCOM_NOTREG;              /* If not there, set the error*/
  else {                               /* OTHERWISE                  */
    if (!(APIBLOCKDLLNAME(cblock))     /* If dllname is NULL and  */
        && (cblock->apiaddr)) {        /* the func addr is non-Zero  */
      rc = RXSUBCOM_OK;                /* then function loaded as EXE*/
      *paddress =                      /* Return its address.        */
          cblock->apiaddr;
    }
    else {                             /* Otherwise, get the address */
      rc = RxGetModAddress(            /* of the entry point in the  */
              APIBLOCKDLLNAME(cblock), /* DLL.                       */
              APIBLOCKDLLPROC(cblock),
              &load_errors[type].errors[0],
              &load_address);

      if( ! rc ) {
           *paddress = load_address;
#ifdef UPDATE_ADDRESS
           cblock->apiaddr = load_address;
           rc = (HANDLE)MySendMessage(RXAPI_REGUPDATE,
                                   (WPARAM)0,
                                   (WPARAM)type);
#endif
      }
    }                                  /* If exe ... else is dll.... */
  }                                    /* If registered.             */
  APICLEANUP_API();
  return (rc);
}


/*********************************************************************/
/*                                                                   */
/*  Function Name: memmgrsearch                                      */
/*                                                                   */
/*  Description:   asks the mem mgr to search its registration       */
/*                 chain.                                            */
/*                                                                   */
/*                 If the routine is found, it returns a pointer     */
/*                 to the control block.                             */
/*                                                                   */
/*  Entry Point:   memmgrsearch( name, dllname,type )                */
/*                                                                   */
/*  Parameter(s):    name     -  The handler name to locate.         */
/*                   dllname  -  dll name                            */
/*                   type     -  The Registration Type.              */
/*                                                                   */
/*  Return Value:    NULL or the address of the block if found.      */
/*                                                                   */
/*  Note:          The caller is responsible for performing an       */
/*                 UnmapViewOfFile on the returned pointer when      */
/*                 finished.                                         */
/*                                                                   */
/*********************************************************************/

APIBLOCK *memmgrsearch(
  const char * name,                   /* Name to find               */
  const char * dll,                    /* Dynalink Name to find.     */
  int   type )                         /* Type of name to find.      */
{

  APIBLOCK *cblock = NULL;             /* Working ptr, current block */
  HAPIBLOCK PtempH;
  BOOL found;

  /* no API_RUNNING check required because called after APISTARTUP */

  if (!RX.baseblock[type])
      return NULL; /* No such se/fnc registered */

  if(FillAPIComBlock(&PtempH,
                     name, dll, NULL))
        return(NULL);

  found = (BOOL)MySendMessage(RXAPI_REGQUERY,
                                    (WPARAM)0,
                                    (WPARAM)type);

  if (found && (found != RXSUBCOM_NOEMEM)) cblock = LRX.comblock[API_API];
  return (cblock);
}


/*********************************************************************/
/*                                                                   */
/*  Function Name: memmgrcheckdll                                    */
/*                                                                   */
/*  Description:   asks the mem mgr to search its registration       */
/*                 chain.                                            */
/*                                                                   */
/*                 If the routine is found, it returns a pointer     */
/*                 to the control block.                             */
/*                                                                   */
/*  Entry Point:   memmgrcheckdll( name, dllname,type )              */
/*                                                                   */
/*  Parameter(s):    name     -  The handler name to locate.         */
/*                   dllname  -  dll name                            */
/*                   type     -  The Registration Type.              */
/*                                                                   */
/*  Return Value:    NULL or the address of the block if found.      */
/*                                                                   */
/*  Note:          The caller is responsible for performing an       */
/*                 UnmapViewOfFile on the returned pointer when      */
/*                 finished.                                         */
/*                                                                   */
/*********************************************************************/

int  memmgrcheckdll(
  const char * name,                   /* Name to find               */
  const char * dll,                    /* Dynalink Name to find.     */
  int   type )                         /* Type of name to find.      */
{

  HAPIBLOCK PtempH;
  LONG rc = RXSUBCOM_NOEMEM;

  /* no API_RUNNING check required because called after APISTARTUP */

  if (!RX.baseblock[type]) return RXSUBCOM_OK; /* No such se/fnc registered --> return OK */

  if (FillAPIComBlock(&PtempH,
                     name, dll, NULL))
        return(rc);

  return (LONG)MySendMessage(RXAPI_REGCHECKDLL,
                         (WPARAM)0,
                         (WPARAM)type);
}

/*********************************************************************/
/*                                                                   */
/*  Function Name: memmgrcheckexe                                    */
/*                                                                   */
/*  Description:   asks the mem mgr to search its registration       */
/*                 chain.                                            */
/*                                                                   */
/*                 If the routine is found, it returns a pointer     */
/*                 to the control block.                             */
/*                                                                   */
/*  Entry Point:   memmgrcheckexe( name, dllname,type )              */
/*                                                                   */
/*  Parameter(s):    name     -  The handler name to locate.         */
/*                   dllname  -  dll name                            */
/*                   type     -  The Registration Type.              */
/*                                                                   */
/*  Return Value:    NULL or the address of the block if found.      */
/*                                                                   */
/*  Note:          The caller is responsible for performing an       */
/*                 UnmapViewOfFile on the returned pointer when      */
/*                 finished.                                         */
/*                                                                   */
/*********************************************************************/

int memmgrcheckexe(
  const char * name,                   /* Name to find               */
  int   type )                         /* Type of name to find.      */
{

  HAPIBLOCK PtempH;
  int  rc = RXSUBCOM_NOEMEM;

  /* no API_RUNNING check required because called after APISTARTUP */

  if (!RX.baseblock[type]) return RXSUBCOM_OK; /* No such se/fnc registered --> return OK */

  if(FillAPIComBlock(&PtempH,
                     name, NULL, NULL))
        return(rc);

  return (LONG)MySendMessage(RXAPI_REGCHECKEXE,
                             (WPARAM)0,
                             (WPARAM)type);
}



/* try to shutdown the RXAPI.EXE */
/* request from toronto */

RexxReturnCode REXXENTRY RexxShutDownAPI(void)
{
  size_t dummy;

  if (!API_RUNNING()) return(0);

  //** Ask the Memory Manager to deinstall itself*/

  return (DWORD)MySendMessage(RXAPI_SHUTDOWN,
                             (WPARAM)queue_get_pid(&dummy),
                             (WPARAM)0);
}



