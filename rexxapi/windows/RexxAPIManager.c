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


/**********************************************************************
**
** Module:      WINRXAPI.C
**
** Description: Utility functions used by the REXXAPI
**              Dynalink Module
**
**********************************************************************/

#define INTERNALNAME_EXISTENCE            "RXAPI_EXISTENCE_MUTEX"
#define INTERNALNAME_START                "RXAPI_START_MUTEX"
#define INTERNALNAME_API                  "RXAPI_API_MUTEX"
#define INTERNALNAME_QUEUE                "RXAPI_QUEUE_MUTEX"
#define INTERNALNAME_MACRO                "RXAPI_MACRO_MUTEX"
#define INTERNALNAME_MESSAGE              "RXAPI_MESSAGE_MUTEX"
#define INTERNALNAME_MSGEVENT             "RXAPI_MESSAGE_EVENT"
#define INTERNALNAME_RESULTEVENT          "RXAPI_MESSAGE_RESEVENT"
#define INTERNALNAME_FMAPNAME_INITEXPORTS "RXAPI_GLOBALS_MAPFILE"
#define INTERNALNAME_FMAPNAME_API_API     "RXAPI_API_API_MAPFILE"
#define INTERNALNAME_FMAPNAME_API_MACRO   "RXAPI_API_MACRO_MAPFILE"
#define INTERNALNAME_FMAPNAME_API_QUEUE   "RXAPI_API_QUEUE_MAPFILE"

char *NormalNamedObjects[] = {
  INTERNALNAME_EXISTENCE,
  INTERNALNAME_START,
  INTERNALNAME_API,
  INTERNALNAME_QUEUE,
  INTERNALNAME_MACRO,
  INTERNALNAME_MESSAGE,
  INTERNALNAME_MSGEVENT,
  INTERNALNAME_RESULTEVENT,
  INTERNALNAME_FMAPNAME_INITEXPORTS,
  INTERNALNAME_FMAPNAME_API_API,
  INTERNALNAME_FMAPNAME_API_MACRO,
  INTERNALNAME_FMAPNAME_API_QUEUE
};

char *GlobalNamedObjects[] = {
  "Global\\"INTERNALNAME_EXISTENCE,
  "Global\\"INTERNALNAME_START,
  "Global\\"INTERNALNAME_API,
  "Global\\"INTERNALNAME_QUEUE,
  "Global\\"INTERNALNAME_MACRO,
  "Global\\"INTERNALNAME_MESSAGE,
  "Global\\"INTERNALNAME_MSGEVENT,
  "Global\\"INTERNALNAME_RESULTEVENT,
  "Global\\"INTERNALNAME_FMAPNAME_INITEXPORTS,
  "Global\\"INTERNALNAME_FMAPNAME_API_API,
  "Global\\"INTERNALNAME_FMAPNAME_API_MACRO,
  "Global\\"INTERNALNAME_FMAPNAME_API_QUEUE
};

char **APInamedObjects = NormalNamedObjects;

#define INCL_RXSUBCOM
#include "rexx.h"
#include "process.h"
#include "malloc.h"
#include "RexxAPIManager.h"
#include "string.h"                    /* Include string Manipulation*/
#include "stdlib.h"                    /* Include "_MAX_" defines    */

#include "APIUtil.h"
#include "Characters.h"
#include "RexxAPIService.h"
#include "APIServiceMessages.h"
#include "APIServiceSystem.h"
#include "SystemVersion.h"

extern UCHAR first_char[];             /* character type table       */
extern UCHAR lower_case_table[];       /* lower case table for Rexx  */
extern UCHAR upper_case_table[];       /* upper case table for Rexx  */

#define SZUPPER_BUF       256          /* uppercase buf size         */
#define ALREADY_INIT      1            /* queue manager status       */
#define NO              0
#define YES             1



//#pragma data_seg(".sdata")
extern REXXAPIDATA * RexxinitExports = NULL;
//#pragma data_seg()

/* Now needed for local init because RX is process global */
extern LOCALREXXAPIDATA RexxinitLocal={0};

extern _declspec(dllexport) SECURITY_DESCRIPTOR SD_NullAcl={0};
extern _declspec(dllexport) LONG APIENTRY RxIsAPIActive(HWND hwnd,LPLONG lpIValue, LPSTR lpszValue);
extern _declspec(dllexport) LONG APIENTRY RxAPIShutdown(HWND hwnd,LPLONG lpIValue, LPSTR lpszValue);

extern LONG StartRXAPIExe(VOID);
extern CRITICAL_SECTION nest;
extern ULONG search_session_in_API(PULONG cnt, BOOL newprocess);
extern BOOL MapComBlock(int chain);
extern void UnmapComBlock(int chain);

#define CLOSEHANDLE(hndl) if (hndl) {CloseHandle(hndl); hndl = NULL;}


/*********************************************************************/
/* Function:           Serialize REXX API function execution and     */
/*                     perform memory initialization.                */
/*                                                                   */
/* Description:        Obtain ownership of the FSRam semaphore that  */
/*                     serializes REXX API Function execution.       */
/*                     Establish an exit list routine to clear the   */
/*                     semaphore if we abort before we clear it.     */
/*                                                                   */
/*                     Also obtains access to all of the memory      */
/*                     blocks currently allocated for API control    */
/*                     blocks.                                       */
/*                                                                   */
/* Input:              Pointer to exception registration record      */
/*                                                                   */
/* Output:             Zero if everything went well, error code      */
/*                     otherwise.                                    */
/*                                                                   */
/* Side effects:       Semaphore set.  Process, Thread and Session   */
/*                     ID variables initialized.  All memory segments*/
/*                     obtained and reconnected.                     */
/*                                                                   */
/*********************************************************************/



ULONG RxAPIStartUp(int chain)
{
  LONG rc;
  PCHAR mutexname;

  /* this is necessary so that programs using REXXAPI.DLL without REXX.DLL
     are initialized as well because RxInterProcessInit cannot be located in DllMain
     of REXXAPI.DLL, otherwise multiple RXAPE.EXEs are started */
  if (!LRX.local_init || LRX.UID != RX.UID) RxInterProcessInit(FALSE);
  if (!LRX.local_init) return (RXSUBCOM_NOEMEM);

  if (chain == API_API) mutexname = MUTEXNAME_API; else
  if (chain == API_QUEUE) mutexname = MUTEXNAME_QUEUE; else
  if (chain == API_MACRO) mutexname = MUTEXNAME_MACRO;
  else return (RXSUBCOM_NOEMEM);

  if (!LRX.MutexSem[chain])
  {
      LRX.MutexSem[chain] = OpenMutex(MUTEX_ALL_ACCESS, TRUE, mutexname);
      if (!LRX.MutexSem[chain]) return (RXSUBCOM_NOEMEM);          /* call this a memory error   */
  }

  if ((rc = WaitForSingleObject(LRX.MutexSem[chain], INFINITE)) != WAIT_OBJECT_0) {
      if (rc != WAIT_ABANDONED)  /* this is just a work around*/
      {
#if _DEBUG
          MessageBox(NULL, "Mutex wait failed. RXAPI running in unguarded mode!","Error",MB_OK | MB_ICONHAND);
#endif
          return (RXSUBCOM_NOEMEM);          /* call this a memory error   */
      }
      /* if rc is WAIT_ABANDONED, the synchronization doesn't work anymore */
      /* The RXAPI then runs in an unguarded mode */
  }
  return (RXSUBCOM_OK);
}



/********************************************************************/
/* Function name:      StartRXAPIExe                                */
/*                                                                  */
/* Description:        Startup the REXX API system.  Establish      */
/*                     communication with the data management       */
/*                     thread.                                      */
/*                                                                  */
/* Function:                                                        */
/*                                                                  */
/* Inputs:             None.                                        */
/*                                                                  */
/* Outputs:                                                         */
/*                                                                  */
/* Effects:            Initialization message sent to data          */
/*                                                                  */
/*                                                                  */
/* Notes:                                                           */
/*                                                                  */
/********************************************************************/

LONG StartRXAPIExe(VOID)
{
    char msgbuf[80];                   // holds error msg if required
    char apiExeName[] = "RXAPI.EXE";

    LPCTSTR lpszImageName = NULL;       /* address of module name  */
    LPTSTR lpszCommandLine = NULL;     /* address of command line */
    LPSECURITY_ATTRIBUTES
    lpsaProcess = NULL;         /* address of process security attrs */
    LPSECURITY_ATTRIBUTES
    lpsaThread = NULL;          /* address of thread security attrs */
    /* don't inherit handles, because otherwise files are inherited
       and inaccessible until RXAPI.EXE is stopped again */
    BOOL fInheritHandles = FALSE;        /* new process doesn't inherit handles */
    DWORD fdwCreate = DETACHED_PROCESS; /* creation flags */
    LPVOID lpvEnvironment = NULL;       /* address of new environment block */
    char szSysDir[256];                 /* retrieve system directory */
    LPCTSTR lpszCurDir = szSysDir;      /* address of command line */
    LPSTARTUPINFO lpsiStartInfo;        /* address of STARTUPINFO  */
    STARTUPINFO siStartInfo;
    LPPROCESS_INFORMATION lppiProcInfo; /* address of PROCESS_INFORMATION   */
    PROCESS_INFORMATION MemMgrProcessInfo;
    lppiProcInfo = &MemMgrProcessInfo;

    siStartInfo.cb = sizeof(siStartInfo);
    siStartInfo.lpReserved = NULL;
    siStartInfo.lpDesktop = NULL;
    siStartInfo.lpTitle = NULL;
    siStartInfo.dwX = 0;
    siStartInfo.dwY = 0;
    siStartInfo.dwXSize = 0;
    siStartInfo.dwYSize = 0;
    siStartInfo.dwXCountChars = 0;
    siStartInfo.dwYCountChars = 0;
    siStartInfo.dwFillAttribute = 0;
    siStartInfo.dwFlags = 0;
    siStartInfo.wShowWindow = 0;
    siStartInfo.cbReserved2 = 0;
    siStartInfo.lpReserved2 = NULL;
    siStartInfo.hStdInput = NULL;
    siStartInfo.hStdOutput = NULL;
    siStartInfo.hStdError = NULL;
    lpsiStartInfo = &siStartInfo;
    lpszCommandLine = apiExeName;

    /* start RXAPI process out of system directory */
    if (!GetSystemDirectory(szSysDir, 255)) lpszCurDir = NULL;

    if(!CreateProcess(lpszImageName, lpszCommandLine, lpsaProcess,
                      lpsaThread, fInheritHandles, fdwCreate, lpvEnvironment,
                      lpszCurDir, lpsiStartInfo, lppiProcInfo)) {
          wsprintf(msgbuf,"RXAPI execution failed. Error Code %d",
                           GetLastError());
          MessageBox(NULL, msgbuf,
                  "WinQapi", MB_ICONSTOP|MB_OK);
          return(1);
    }
    else return 0;
}



SECURITY_ATTRIBUTES * SetSecurityDesc(SECURITY_ATTRIBUTES * sa)
{
    if (!RUNNING_95 && !IsValidSecurityDescriptor(&SD_NullAcl))
    {
        InitializeSecurityDescriptor(&SD_NullAcl, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&SD_NullAcl, TRUE, (PACL) NULL, FALSE);
    }

    sa->nLength = sizeof(SECURITY_ATTRIBUTES);
    sa->bInheritHandle = TRUE;
    if (RUNNING_95)
        sa->lpSecurityDescriptor = NULL;
    else
        sa->lpSecurityDescriptor = &SD_NullAcl;
    return sa;
}


BOOL MapComBlock(int chain)
{
    // The com blocks used for the QUEUE and MACROSPACE apis need to
    // be reallocatable.  Because we're using named memory, this requires
    // that the server process AND all of the client processes close
    // the named memory segment before we can allocate a new segment
    // with the same name.  This is generally difficult (if not impossible)
    // to implement, so we get around the problem by using named memory
    // segments that incorporate the extension size in the name so that
    // we avoid conflicts between the new and old segments.
   char mapName[256];

   if (!RX.comhandle[chain]) return FALSE;


   if (chain == API_QUEUE)
   {
       sprintf(mapName, "%s%d", FMAPNAME_COMBLOCK(chain), RX.comblockQueue_ExtensionLevel);
   }
   else if (chain == API_MACRO)
   {
       sprintf(mapName, "%s%d", FMAPNAME_COMBLOCK(chain), RX.comblockMacro_ExtensionLevel);
   }
   else
   {
       strcpy(mapName, FMAPNAME_COMBLOCK(chain));
   }

   /* Obtain handle to named memory mapped file */
   LRX.comhandle[chain] = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, mapName);

   if (LRX.comhandle[chain])
   {
      LRX.comblock[chain]= MapViewOfFile(LRX.comhandle[chain],FILE_MAP_WRITE,0,0,0);
      if (chain == API_QUEUE)
          LRX.comblockQueue_ExtensionLevel = RX.comblockQueue_ExtensionLevel;
      else if (chain == API_MACRO)
          LRX.comblockMacro_ExtensionLevel = RX.comblockMacro_ExtensionLevel;
      return (LRX.comblock[chain] != NULL);
   }
   return FALSE;
}


void UnmapComBlock(int chain)
{
    if (LRX.comblock[chain]) UnmapViewOfFile(LRX.comblock[chain]);
    CLOSEHANDLE(LRX.comhandle[chain]);
}


LONG Connect2RxAPI()
{
    /* Open memory mapped file to share RexxinitExports */
    LRX.hFMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, FMAPNAME_INITEXPORTS);
    if (!LRX.hFMap) return -1;

    RexxinitExports = (REXXAPIDATA *)MapViewOfFile(LRX.hFMap,FILE_MAP_WRITE,0,0,0);
    if (!RexxinitExports) {
        CloseHandle(LRX.hFMap);
        LRX.hFMap = NULL;
        return -2;
    }
    return 0;
}




void DetachLocalInit()
{
    int i;
    if (LRX.local_init)
    {
        for (i=0; i<NUMBEROFCOMBLOCKS; i++) {
            ReleaseMutex(LRX.MutexSem[i]);   /* release, otherwise APISTARTUP might hang */
            CLOSEHANDLE(LRX.MutexSem[i]);
            UnmapComBlock(i);
        }
        CLOSEHANDLE(LRX.MsgMutex);
        CLOSEHANDLE(LRX.MsgEvent);
        CLOSEHANDLE(LRX.ResultEvent);
        if (LRX.hFMap)
        {
            UnmapViewOfFile(RexxinitExports);
            RexxinitExports = NULL;
            CLOSEHANDLE(LRX.hFMap);
        }
        LRX.local_init = 0;
     }
}

/* This is a special check whether or not RXAPI.EXE has been killed
manually. If it has been killed, the memory mapped file might still
be mapped to another REXX process and therefore API_RUNNING still
is true. The API existence mutex is only owned by RXAPI.EXE and therefore
is closed when RXAPI has been killed */

BOOL APIHasBeenKilled()
{
    HANDLE hAPI;

    if ((hAPI = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEXNAME_EXISTENCE)) != NULL)
    {
        CloseHandle(hAPI);
        return FALSE;
    }
    else return TRUE;
}



ULONG RxInterProcessInit(BOOL sessionqueue)
{
  HANDLE hAPI_Init, hAPI_S;
  SECURITY_ATTRIBUTES sa;
  LONG  rc = 0;                       /* Function return value.    */

  if (!RexxinitExports || (RX.init != ALREADY_INIT))
  {
      int count = 10000;
      SetSecurityDesc(&sa);                /* this is also to init the SD_NullACl */

      if ((hAPI_S = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEXNAME_START)) == NULL)
      {
          if ((hAPI_S = CreateMutex(&sa, FALSE, MUTEXNAME_START)) == NULL) {
              MessageBox(NULL, "Could not start Open Object Rexx API Manager (Start Mutex)!","ERROR", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
              return (RXSUBCOM_NOEMEM);
          }
      }
      if (WaitForSingleObject(hAPI_S, 15000) != WAIT_OBJECT_0)
      {
          MessageBox(NULL, "Could not start Open Object Rexx API Manager (Timeout)!","ERROR", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
          CloseHandle(hAPI_S);
          return (RXSUBCOM_NOEMEM);
      }

      if ((hAPI_Init = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEXNAME_EXISTENCE)) == NULL)     /* has RXAPI.EXE not yet been started */
      {
          if (StartRXAPIExe() != 0) {
              MessageBox(NULL, "Could not start Open Object Rexx API Manager (RXAPI.EXE)!","ERROR", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
              ReleaseMutex(hAPI_S);
              CloseHandle(hAPI_S);
              return (RXSUBCOM_NOEMEM);
          }

          /* wait max. 10 sec. to open the existence mutex */
          while (!hAPI_Init && (count-- > 0))
          {
             Sleep(1);
             hAPI_Init = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEXNAME_EXISTENCE);
          }
          if (!hAPI_Init)
          {
              MessageBox(NULL, "Open Object Rexx API Manager (RXAPI.EXE) could not be started."\
                  "This could be due to a version conflict!","ERROR", MB_OK | MB_ICONHAND| MB_SYSTEMMODAL);
              ReleaseMutex(hAPI_S);
              CloseHandle(hAPI_S);
              return (RXSUBCOM_NOEMEM);
          }
      }


      /* wait until RXAPI completed initialization  */
      if (WaitForSingleObject(hAPI_Init, 5000) != WAIT_OBJECT_0)
      {
          MessageBox(NULL, "Initialization of Open Object Rexx API Manager timed out!","ERROR", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
          ReleaseMutex(hAPI_S);
          CloseHandle(hAPI_S);
          CloseHandle(hAPI_Init);
          return (RXSUBCOM_NOEMEM);
      }

      if (!RexxinitExports) rc = Connect2RxAPI();

      ReleaseMutex(hAPI_Init);
      CloseHandle(hAPI_Init);
      ReleaseMutex(hAPI_S);
      CloseHandle(hAPI_S);
      if (!RexxinitExports || (RX.init == -1)) {
          MessageBox(NULL, "Open Object Rexx API Manager (RXAPI.EXE) could not be started or conflicts!","ERROR", MB_OK | MB_ICONHAND| MB_SYSTEMMODAL);
          return (RXSUBCOM_NOEMEM);
      }
  }

  if (RX.MemMgrVersion != RXAPI_VERSION)
  {
      char tmp[130];
      sprintf(tmp, "Open Object Rexx version conflict: this executable=%ld, running API manager=%ld!", RXAPI_VERSION, RX.MemMgrVersion);
      MessageBox(NULL, tmp,"ERROR", MB_OK | MB_ICONHAND| MB_SYSTEMMODAL);
      return(RXSUBCOM_NOEMEM);
  }

  if (!LRX.local_init || LRX.UID != RX.UID) /* do local initialization */
  {
      /* check whether or not RXAPI has been restarted (UIDs are different and local_init is true) */
      if (LRX.local_init)
      {
          int i;
          for (i=0; i<NUMBEROFCOMBLOCKS; i++) UnmapComBlock(i);  /* unmap sysobjects of previous rxapi.exe */
          /* close message events, otherwise MySendMessage might hang after restart of RXAPI */
          CLOSEHANDLE(LRX.MsgMutex);
          CLOSEHANDLE(LRX.MsgEvent);
          CLOSEHANDLE(LRX.ResultEvent);
      }
      else
          if (!nest.DebugInfo) InitializeCriticalSection(&nest);

      /* map the 3 com ports created in RXAPI.EXE */
      if (!MapComBlock(API_QUEUE)) return(RXSUBCOM_NOEMEM);
      if (!MapComBlock(API_API))
      {
          UnmapComBlock(API_QUEUE);
          return(RXSUBCOM_NOEMEM);
      }
      if (!MapComBlock(API_MACRO))
      {
          UnmapComBlock(API_QUEUE);
          UnmapComBlock(API_API);
          return(RXSUBCOM_NOEMEM);
      }

      if (!LRX.local_init)
      {
          if (sessionqueue)
              search_session_in_API((PULONG)&rc, TRUE);
          LRX.local_init = TRUE;
      }
      LRX.UID = RX.UID;    /* connect this DLL to a particular RXAPI.EXE */
  }
  return (0);                          /* it all worked              */
}



/*********************************************************************/
/* Function:           Allocate and fill in an API block.            */
/*                                                                   */
/* Inputs:             Pointers the 3 possible ASCII-Z strings that  */
/*                     may be included in the block and the storage  */
/*                     base.                                         */
/*                     name is reguired.                             */
/*                     dll_name & dll_proc can be NULL               */
/*                                                                   */
/*********************************************************************/
LONG  FillAPIComBlock(
  HAPIBLOCK *block,                    /* allocated block            */
  PSZ        name,                     /* api name                   */
  PSZ        dll_name,                 /* name of dll                */
  PSZ        dll_proc)                 /* dll procedure name         */
{
  LONG    size;                          /* total allocation size      */
  PSZ     temp;                          /* used to fill in APIBLOCK   */
  PAPIBLOCK tmpPtr;                      /* temp work pointer          */

  size = APISIZE + SZSTR(name);        /* get minimum size           */
  if (dll_name)                        /* if we have a dll name      */
    size += SZSTR(dll_name);           /* add in that name           */
  if (dll_proc)                        /* if we have a dll proc      */
    size += SZSTR(dll_proc);           /* add in that string         */

  if (size > sizeof(RXREG_TALK) - sizeof(ULONG)) return 5; /* Names too long >3k shouldn't happen  */

  tmpPtr = *block = LRX.comblock[API_API];
  if (!tmpPtr) return(1);
  memset(tmpPtr, 0, APISIZE);          /* clear out the block        */
  tmpPtr->apipid=(long)GetCurrentProcessId(); /* set process id      */
  temp = (PSZ)tmpPtr;                  /* copy the pointer           */
  temp += APISIZE;                     /* step past header           */
  (tmpPtr)->apiname = (PSZ)(temp-(PSZ)tmpPtr);   /* point to name              */
  strcpy(temp,name);                   /* copy the name into block   */
  temp += SZSTR(name);                 /* step past the name         */
  if (dll_name) {                      /* if we have a dll_name      */
    (tmpPtr)->apidll_name = (PSZ)(temp-(PSZ)tmpPtr); /* fill in the pointer    */
    strcpy(temp,dll_name);             /* copy the dll_name too      */
    temp += SZSTR(dll_name);           /* step past the string       */
  }
  else
    (tmpPtr)->apidll_name = NULL;      /* otherwise say no dll used  */

  if (dll_proc) {                      /* if we have a dll_proc      */
    (tmpPtr)->apidll_proc = (PSZ)(temp-(PSZ)tmpPtr); /* fill in the pointer    */
    strcpy(temp,dll_proc);             /* copy the dll_proc too      */
  }
  else
    (tmpPtr)->apidll_proc = NULL;      /* otherwise say no dll used  */
  (tmpPtr)->apisize = (ULONG)size;

  return (0);                          /* no errors, return nicely   */
}


/*********************************************************************/
/* Function:           Get the address of a function in a DLL.       */
/*                                                                   */
/* Description:        Given a DLL name and the name of an entry     */
/*                     point in the DLL, make sure the DLL is loaded */
/*                     into memory.  Then get the address of the     */
/*                     entry point.                                  */
/*                                                                   */
/* Input:              DLL name, Entry point name, error codes.      */
/*                                                                   */
/* Output:             Entry point address.                          */
/*                                                                   */
/* Effects:            DLL loaded into memory.                       */
/*                                                                   */
/* Notes:                                                            */
/*                                                                   */
/*   The error code argument points to an array of two ULONG         */
/*   values.  The first is returned if we cannot load the module.    */
/*   The second is returned if we cannot find the entry point.       */
/*   We return 0 if we succeed.                                      */
/*                                                                   */
/*   This function does the following:                               */
/*                                                                   */
/*   1.  Call GetModuleHandle().  This retrieves the module handle   */
/*       if the DLL has been loaded into memory.                     */
/*                                                                   */
/*   2.  If the module has not been loaded into memory, call         */
/*       LoadLibrary() to load the library into memory.  This        */
/*       call will return the module handle.  If we cannot load the  */
/*       library, set the return code to the first error code.       */
/*                                                                   */
/*   3.  If we have successfully found the module handle, call       */
/*       GetProcAddress() to retrieve the procedure address.  If     */
/*       there is an error, try using an uppercase copy of the       */
/*       module name. On any error, set the return code              */
/*       to the second error code.                                   */
/*                                                                   */
/*   4.  If we have successfully retrieved the module handle and     */
/*       the procedure address, return NO (0).  Otherwise, return    */
/*       an error code.                                              */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

ULONG  RxGetModAddress( PSZ       dll_name,
                        PSZ       function_name,
                        PULONG    error_codes,
                        PFN *     function_address,
                        PULONG    call_type)
{
   ULONG    rc = 0 ;                   /* Function result.           */
   HMODULE  dll_handle ;               /* DLL handle.                */
                                       /* used for uppercasing name  */
   UCHAR     upper_dll[SZUPPER_BUF];

    // see if dll is already loaded
    if ( dll_handle = GetModuleHandle( (LPCTSTR) dll_name) ) {

       // dll is loaded, get the function address
       if ( !((FARPROC)*function_address = GetProcAddress( dll_handle, (LPCSTR)function_name)) ) {

          // couldn't find the function, try uppercasing the name
          strcpy(upper_dll,function_name);        /* copy the name             */
          memupper(upper_dll, strlen(upper_dll)); /* uppercase the name        */

          // if the uppercase version is found, uppercase the name for next time
          if ( (FARPROC)*function_address = GetProcAddress( dll_handle, (LPCSTR)upper_dll) )
             memupper(function_name,       /* uppercase the table name for the    */
                 strlen(function_name));   /* next time                           */
          else
             rc = *(error_codes+1) ;       /* Error loading functions        */
       }
    }
    else {   // Library not already loaded, load it now

       if (dll_handle = LoadLibrary((LPCTSTR)dll_name)) {

          // dll loaded ok, get the function address
          if ( !((FARPROC)*function_address = GetProcAddress( dll_handle, (LPCSTR)function_name)) ) {

             // couldn't find the function, try uppercasing the name
             strcpy(upper_dll,function_name);        /* copy the name             */
             memupper(upper_dll, strlen(upper_dll)); /* uppercase the name        */

             // if the uppercase version is found, uppercase the name for next time
             if ( (FARPROC)*function_address = GetProcAddress( dll_handle, (LPCSTR)upper_dll) )
                memupper(function_name,       /* uppercase the table name for the    */
                    strlen(function_name));   /* next time                           */
             else
                rc = *(error_codes+1) ;             /* Error loading DLL.        */
          }
       }
       else { // load of library failed
          rc = * error_codes ;             /* Error loading DLL.        */
       } /* else Load Failed */
    }   /* else not already Loaded */

   return( rc ) ;
}


/* This function was written for defect 136 */
/* It will remove all the subcommand entries for a specific process */

VOID RxFreeProcessSubcomList(ULONG pid)
{
    APIBLOCK        *block;              /* block to free             */
    APIBLOCK        *c;
    APIBLOCK        *prev;
    LONG            i ;
    /****************************************************************/
    /* The system maintains subcommand, exit and function           */
    /* registrations in seperate changes.  Loop through all chains  */
    /* lookinf for routines the current process has registered      */
    /* by address.  Remove any found.                               */
    /****************************************************************/

    for( i = 0 ; i < REGNOOFTYPES ; ++ i ) {
       c = RX.baseblock[ i ] ;
       prev = NULL ;

       while( c ) {
          if( ( c->apipid == pid)  &&
             ( c->apidll_name == NULL ) ) {

          /**********************************************************/
          /* If the block is at the beginning of the chain (the     */
          /* baseblock variable will contain its address),we need to*/
          /* reset the beginning of the chain to point to its       */
          /* follower.  Otherwise, fix up the next pointer in the   */
          /* previous block.                                        */
          /**********************************************************/

             if( prev ) {
                prev->next = c->next ;
             }
             else {
                RX.baseblock[ i ] = c->next ;
             }

             block = c;                   /* save block address        */
             c = c->next;                 /* step to next block        */
             GlobalFree(block);           /* release the block         */
          }
          else {
             prev = c ;                     /* Keeping this block.     */
             c = c->next;                   /* Next, please!           */
          }
       }
    }
}



/********************************************************************/
/*                                                                  */
/* Function name:      MySendMessage                                */
/*                                                                  */
/* Description:        Replaces normal SendMessage because          */
/*                     SendMessage requires "interact with desktop" */
/*                     to be enabled                                */
/*                                                                  */
/********************************************************************/

LRESULT MySendMessage(UINT msg, WPARAM wP, LPARAM lP)
{
    LRESULT r;
    BOOL killed;

    if (!LRX.MsgMutex)
    {
        LRX.MsgMutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEXNAME_MESSAGE);
        if (!LRX.MsgMutex) return (RXSUBCOM_NOEMEM);      /* call this a memory error   */
    }

    if (!LRX.MsgEvent)
    {
        LRX.MsgEvent = OpenEvent(EVENT_ALL_ACCESS, TRUE, MUTEXNAME_MSGEVENT);
        if (!LRX.MsgEvent) return (RXSUBCOM_NOEMEM);      /* call this a memory error   */
    }

    if (!LRX.ResultEvent)
    {
        LRX.ResultEvent = OpenEvent(EVENT_ALL_ACCESS, TRUE, MUTEXNAME_RESULTEVENT);
        if (!LRX.ResultEvent) return (RXSUBCOM_NOEMEM);      /* call this a memory error   */
    }

    /* wait for exclusive access to the message structure */
    /* do wait in intervals of 5 seconds and check if RXAPI is still up */
    do
    {
        r = WaitForSingleObject(LRX.MsgMutex, 5000);
        if (r == WAIT_FAILED)
            return (RXSUBCOM_NOEMEM);          /* call this a memory error   */
    } while ((r == WAIT_TIMEOUT) && API_RUNNING() && !(killed=APIHasBeenKilled()));
    if (r == WAIT_TIMEOUT)
    {
        if (killed) DetachLocalInit();
        return (RXSUBCOM_NOEMEM);          /* call this a memory error   */
    }

    RX.msg.message = msg;
    RX.msg.wParam = wP;
    RX.msg.lParam = lP;
    RX.msg.result = 0;
    RX.msg.done = FALSE;

    ResetEvent(LRX.ResultEvent);
    SetEvent(LRX.MsgEvent);  /* signal RXAPI.EXE that there is a message to process */

    /* wait for the message result */
    /* do wait in intervals of 5 seconds and check if RXAPI is still up */
    do
    {
        r = WaitForSingleObject(LRX.ResultEvent, 5000);
        if (r == WAIT_FAILED)
        {
            ReleaseMutex(LRX.MsgMutex);
            return (RXSUBCOM_NOEMEM);          /* call this a memory error   */
        }
    } while ((r == WAIT_TIMEOUT) && API_RUNNING() && !(killed=APIHasBeenKilled()));
    if (r == WAIT_TIMEOUT)
    {
        ReleaseMutex(LRX.MsgMutex);
        if (killed) DetachLocalInit();
        return (RXSUBCOM_NOEMEM);          /* call this a memory error   */
    }

    r = RX.msg.result;         /* save the result and */
    /* release exculsive access to the message structure */
    ReleaseMutex(LRX.MsgMutex);

    return r;
}



/********************************************************************/
/*                                                                  */
/* Function name:      RxIsAPIActive                                */
/*                                                                  */
/* Description:        Can be called from InstallShield to check    */
/*                     whether RXAPI is running                     */
/*                     returns 1 if API is running                  */
/*                                                                  */
/********************************************************************/

LONG APIENTRY RxIsAPIActive(HWND hwnd,LPLONG lpIValue, LPSTR lpszValue)
{
    BOOL ret;
    if (!RexxinitExports) Connect2RxAPI();
    ret = API_RUNNING();
    if (LRX.hFMap)
    {
         UnmapViewOfFile(RexxinitExports);
         RexxinitExports = NULL;
         CloseHandle(LRX.hFMap);
    }
    return ret;
}

/********************************************************************/
/*                                                                  */
/* Function name:      RxAPIShutdown                                */
/*                                                                  */
/* Description:        Can be called from Installer to shut down    */
/*                     the service.                                 */
/*                                                                  */
/********************************************************************/

LONG APIENTRY RxAPIShutdown(HWND hwnd,LPLONG lpIValue, LPSTR lpszValue)
{
    if (API_RUNNING())
      if (!APIHasBeenKilled())
        MySendMessage(RXAPI_SHUTDOWN, 0, (LONG) GetCurrentProcessId());
    return 0;
}

/********************************************************************/
/*                                                                  */
/* Function name:      DllMain                                      */
/*                                                                  */
/* Description:        EntryPoint for REXXAPI.DLL                   */
/*                                                                  */
/* Function:           Called by O/S whenever a process attaches    */
/*                     or detaches from the DLL.                    */
/*                     This entry point is used to initialize the   */
/*                     memory manager, and check to make sure       */
/*                     it is functioning.                           */
/*                                                                  */
/* Inputs:             Defined by DllEntryPoint function of Windows */
/*                                                                  */
/* Outputs:            TRUE if successful, FALSE if failure.        */
/*                                                                  */
/* Notes:                                                           */
/*                                                                  */
/*                                                                  */
/********************************************************************/
BOOL WINAPI DllMain(
  HINSTANCE hinstDll,
  DWORD fdwReason,
  LPVOID lpvReserved)
{
  HANDLE hTest;

  if (fdwReason == DLL_PROCESS_ATTACH) {
    /* check if a named object with "Global\..." can be created      */
    /* if yes, use "Global\..." for ALL RXAPI communication objects. */
    /* this test is needed for W2K & Terminal Services               */
    hTest = CreateMutex(NULL, FALSE, "Global\\uniqueREXXtestMutex123HUGO");
    if (hTest != NULL) {
      CloseHandle(hTest);
      APInamedObjects = GlobalNamedObjects;
    } else {
      APInamedObjects = NormalNamedObjects;
    }
    /* if only REXXAPI.DLL is attached without REXX.DLL,
       nest must be initialized, so that APISTARTUP can be called */
    if (!nest.DebugInfo) InitializeCriticalSection(&nest);
  } else if (fdwReason == DLL_PROCESS_DETACH) {
    /* on Win2000 and sometimes even on NT API_RUNNING and/or APIHasBeenKilled*/
    /* returns wrong results and it takes a long time until rexx returns      */
    /* so an additional check is added (APIHasBeenKilled). the sleep was to   */
    /* allow the semaphores to be cleared                                     */
    Sleep(1);
    if (API_RUNNING())
      if (!APIHasBeenKilled())
        MySendMessage(RXAPI_PROCESSCLEANUP, 0, (LONG) GetCurrentProcessId());
    DetachLocalInit();
  }
  return(TRUE);
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxAllocateMemory                              */
/*                                                                   */
/*  Description:     Operating system independant function to        */
/*                   allocate memory. The function is a wrapper      */
/*                   for appropriate operating system memory         */
/*                   allocation function.                            */
/*                                                                   */
/*                                                                   */
/*  Entry Point:     RexxAllocateMemory                              */
/*                                                                   */
/*  Parameter(s):    Specifies the number of bytes to allocate(ULONG)*/
/*                                                                   */
/*  Return Value:    Pointer to the allocated memory (PVOID)         */
/*                   NULL if the function fails                      */
/*                                                                   */
/*********************************************************************/
PVOID APIENTRY RexxAllocateMemory(ULONG size)
{
   PVOID pMem;

   pMem = GlobalAlloc(GMEM_FIXED, size);
   return pMem;
}

/*********************************************************************/
/*                                                                   */
/*  Function Name:   RexxFreeMemory                                  */
/*                                                                   */
/*  Description:     Operating system independant function to        */
/*                   free memory. The function is a wrapper          */
/*                   for appropriate operating system memory         */
/*                   allocation function.                            */
/*                                                                   */
/*                                                                   */
/*  Entry Point:     RexxFreeMemory                                  */
/*                                                                   */
/*  Parameter(s):    Pointer to the memory allocated with            */
/*                   RexxAllocateMemory (PVOID)                      */
/*                                                                   */
/*  Return Value:    always returns 0 (ULONG)                        */
/*                                                                   */
/*********************************************************************/
ULONG APIENTRY RexxFreeMemory(PVOID pMem)
{
    GlobalFree(pMem);
return 0;
}
