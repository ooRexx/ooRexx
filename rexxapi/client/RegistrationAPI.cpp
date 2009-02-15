/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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

#include "rexx.h"
#include "LocalAPIManager.hpp"
#include "LocalRegistrationManager.hpp"
#include "RexxAPI.h"
#include "LocalAPIContext.hpp"
#include "RexxInternalApis.h"
#include "ClientMessage.hpp"
#include "SysLocalAPIManager.hpp"


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
/*  Parameter(s):   EnvName   - name of the registered subcommand    */
/*                              handler                              */
/*                                                                   */
/*                  ModuleName - name of the module containing       */
/*                               the subcommand handler              */
/*                                                                   */
/*                  EntryPoint - name of the routine for the         */
/*                               handler                             */
/*                                                                   */
/*                  UserArea   - Area for any user data              */
/*                                                                   */
/*                  DropAuth   - Drop authority flag                 */
/*                                                                   */
/*  Return Value:   Valid RXSUBCOM return codes                      */
/*                                                                   */
/*********************************************************************/

RexxReturnCode RexxEntry RexxRegisterSubcomDll(
  const char *       envName,                    // Subcom name
  const char *       moduleName,                 // Name of DLL
  const char *       procedureName,              // DLL routine name
  const char *       userArea,                   // User data
  size_t             dropAuthority)              // Drop Authority
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.registerCallback(SubcomAPI,
            envName, moduleName, procedureName, userArea, dropAuthority == RXSUBCOM_NONDROP);
    }
    EXIT_REXX_API();
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

RexxReturnCode RexxEntry RexxRegisterSubcomExe(
    const char *    envName,                  // Subcom name
    REXXPFN    entryPoint,               // callback address
    const char *    userArea)                 // User data
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.registerCallback(SubcomAPI, envName, entryPoint, userArea);
    }
    EXIT_REXX_API();
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

RexxReturnCode RexxEntry RexxDeregisterSubcom(
    const char *    name,                            /* Environment Name           */
    const char *    moduleName )                     /* Associated library name    */
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.dropCallback(SubcomAPI, name, moduleName);
    }
    EXIT_REXX_API();
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxQuerySubcom                                 */
/*                                                                   */
/*  Description:     Queries the subcommand registration table.      */
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

RexxReturnCode RexxEntry RexxQuerySubcom(
  const char         *name,            /* Environment Name           */
  const char         *module,          /* Associated library name    */
  unsigned short     *flags,           /* existence information      */
  char               *userWord)        /* data from registration     */
{
    *flags = 0;
    ENTER_REXX_API(RegistrationManager)
    {
        RexxReturnCode ret = lam->registrationManager.queryCallback(SubcomAPI, name, module, userWord);
        *flags = (ret == RXSUBCOM_OK);
        return ret;
    }
    EXIT_REXX_API();
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxResolveSubcom                               */
/*                                                                   */
/*  Description:     Query and return information about a resolved   */
/*                   Subcom handler                                  */
/*                                                                   */
/*  Entry Point:     ooRexxResolveSubcom                             */
/*                                                                   */
/*  Parameter(s):    name       -  Name of the desired system exit   */
/*                   entryPoint -  Pointer to the resolved handler   */
/*                   style      -  The subcom call style info        */
/*                                                                   */
/*  Return Value:    Return code from subcommand handler processing  */
/*                                                                   */
/*********************************************************************/

RexxReturnCode RexxEntry RexxResolveSubcom(
  const char       *name,              // Exit name.
  REXXPFN     *entryPoint)        // the entry point of the exit
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.resolveCallback(SubcomAPI, name, NULL, *entryPoint);
    }
    EXIT_REXX_API();
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxLoadSubcom                                  */
/*                                                                   */
/*  Description:     Force a subcommand handler to the resolved and  */
/*                   loaded.                                         */
/*                                                                   */
/*  Entry Point:     RexxLoadSubcom                                  */
/*                                                                   */
/*  Parameter(s):    name       -  Name of the desired system exit   */
/*                   entryPoint -  Pointer to the resolved handler   */
/*                   style      -  The subcom call style info        */
/*                                                                   */
/*  Return Value:    Return code from subcommand handler processing  */
/*                                                                   */
/*********************************************************************/

RexxReturnCode RexxEntry RexxLoadSubcom(
  const char       *name,         // handler name
  const char       *lib)          // handler library
{
    ENTER_REXX_API(RegistrationManager)
    {
        REXXPFN entryPoint;

        return lam->registrationManager.resolveCallback(SubcomAPI, name, lib, entryPoint);
    }
    EXIT_REXX_API();
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
/*  Entry Point:    RexxRegisterLibraryExit                          */
/*                                                                   */
/*  Return Value:   Valid RXSUBCOM return codes                      */
/*                                                                   */
/*********************************************************************/

RexxReturnCode RexxEntry RexxRegisterExitDll(
    const char *     envName,                  // Exit name
    const char *     moduleName,               // Name of DLL
    const char *     procedureName,            // DLL routine name
    const char *     userArea,                 // User data
    size_t           dropAuthority)            // Drop Authority
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.registerCallback(ExitAPI, envName, moduleName, procedureName, userArea, dropAuthority == RXSUBCOM_NONDROP);
    }
    EXIT_REXX_API();
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
/*  Return Value:   Valid RXSUBCOM return codes                      */
/*                                                                   */
/*********************************************************************/

RexxReturnCode RexxEntry   RexxRegisterExitExe(
  const char *      envName,           /* exit name                  */
  REXXPFN      entryPoint,        /* Entry point address        */
  const char *      userArea)          /* User data                  */
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.registerCallback(ExitAPI, envName, entryPoint, userArea);
    }
    EXIT_REXX_API();
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

RexxReturnCode RexxEntry RexxDeregisterExit(
    const char *    name,                          /* Environment Name           */
    const char *    moduleName)                    /* Associated library name    */
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.dropCallback(ExitAPI, name, moduleName);
    }
    EXIT_REXX_API();
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

RexxReturnCode RexxEntry RexxQueryExit(
  const char *      name,              /* Environment Name           */
  const char *      module,            /* Associated Name (of DLL)   */
  unsigned short     *exist,           /* existence information      */
  char               *userWord)        /* data from registration     */
{
    *exist = 0;
    ENTER_REXX_API(RegistrationManager)
    {
        RexxReturnCode ret = lam->registrationManager.queryCallback(ExitAPI, name, module, userWord);
        *exist = (ret == RXQUEUE_OK);
        return ret;
    }
    EXIT_REXX_API();
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxResolveExit                                 */
/*                                                                   */
/*  Description:     Resolves a system exit entrypoint address       */
/*                                                                   */
/*  Entry Point:     ooRexxResolveExit                               */
/*                                                                   */
/*  Parameter(s):    name       -  Name of the desired system exit   */
/*                   entrypoint -  returned entry point address      */
/*                   legacyStyle - style of the call                 */
/*                                                                   */
/*  Return Value:    Return code from exit if the exit ran           */
/*                   -1 otherwise                                    */
/*                                                                   */
/*********************************************************************/

RexxReturnCode RexxEntry RexxResolveExit(
  const char *      name,              // Exit name.
  REXXPFN     *entryPoint)        // the entry point of the exit
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.resolveCallback(ExitAPI, name, NULL, *entryPoint);
    }
    EXIT_REXX_API();
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
/*  Return Value:   Valid RXSUBCOM return codes                      */
/*                                                                   */
/*********************************************************************/

RexxReturnCode RexxEntry RexxRegisterFunctionDll(
  const char *      name,                 // Subcom name
  const char *      moduleName,           // Name of library
  const char *      procedureName)        // library routine name
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.registerCallback(FunctionAPI, name, moduleName, procedureName, NULL, true);
    }
    EXIT_REXX_API();
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
/*  Return Value:   Valid RXSUBCOM return codes                      */
/*                                                                   */
/*********************************************************************/

RexxReturnCode RexxEntry RexxRegisterFunctionExe(
  const char *      name,                 // Function name
  REXXPFN  entryPoint)               // Entry point address
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.registerCallback(FunctionAPI, name, entryPoint, NULL);
    }
    EXIT_REXX_API();
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

RexxReturnCode RexxEntry RexxDeregisterFunction(
    const char *    name)                   /* Function Name           */
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.dropCallback(FunctionAPI, name, NULL);
    }
    EXIT_REXX_API();
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

RexxReturnCode RexxEntry RexxQueryFunction(
    const char *    name)                 /* Function Name           */
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.queryCallback(FunctionAPI, name, NULL, NULL);
    }
    EXIT_REXX_API();
}


RexxReturnCode RexxEntry RexxResolveRoutine(const char *name, REXXPFN *entryPoint)
{
    ENTER_REXX_API(RegistrationManager)
    {
        return lam->registrationManager.resolveCallback(FunctionAPI, name, NULL, *entryPoint);
    }
    EXIT_REXX_API();
}


/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxAllocateMemory                              */
/*                                                                   */
/*  Description:     Operating system independant method to          */
/*                   allocate memory. The function is a wrapper      */
/*                   for appropriate compiler or operating system    */
/*                   memory function.                                */
/*                                                                   */
/*                                                                   */
/*  Entry Point:     RexxAllocateMemory                              */
/*                                                                   */
/*  Parameter(s):    size of memory to allocate (ULONG)              */
/*                                                                   */
/*  Return Value:    The allocated Block of memory (PVOID)           */
/*                                                                   */
/*********************************************************************/
void *REXXENTRY RexxAllocateMemory(size_t size)
{
   return SysAPIManager::allocateMemory(size);
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxFreeMemory                                  */
/*                                                                   */
/*  Description:     Operating system independant method to          */
/*                   free memory. The function is a wrapper          */
/*                   for appropriate compiler or operating system    */
/*                   memory function.                                */
/*                                                                   */
/*                                                                   */
/*  Entry Point:     RexxFreeMemory                                  */
/*                                                                   */
/*  Parameter(s):    size of memory to allocate (ULONG)              */
/*                                                                   */
/*  Return Value:    The allocated Block of memory (PVOID)           */
/*                                                                   */
/*********************************************************************/
RexxReturnCode REXXENTRY RexxFreeMemory(void *ptr)
{
    SysAPIManager::releaseMemory(ptr);
    return 0;
}


