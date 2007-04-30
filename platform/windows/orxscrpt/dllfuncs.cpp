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
#define  DLLFUNCS_CPP
#include "dllfuncs.hpp"
#include "eng2rexx.hpp"


CRITICAL_SECTION EngineSection = { 0 };
HANDLE           semaphore;
HANDLE           mutex;
Index  *thread2EngineList = NULL;  // needed to find the engine that is using the
                                            // thread that the REXX code is running in

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved) {

  APIRET   rc;                        /* return code from REXX     */
  ListItem    *Current;
  OrxScript   *Content;


#if defined(DEBUGC)+defined(DEBUGZ)
  if(DLLlogfile) FPRINTF(DLLlogfile,"DllMain - reason %d\n",ul_reason_for_call);
#endif
  if ( ul_reason_for_call == DLL_PROCESS_ATTACH) {
    if(!EngineChain) EngineChain = new LooseLinkedList;
    InitializeCriticalSection(&EngineSection);
#if defined(DEBUGC)+defined(DEBUGZ)
    DLLlogfile = fopen("c:\\temp\\engine-dll.log","w");
    if (!DLLlogfile) DLLlogfile = stderr;
    FPRINTF(DLLlogfile,"DllMain ATTACH\n");
#endif
    // list to associate engines with the threads it owns
    thread2EngineList = new Index;
    // init REXX for this process, to keep directory entries
    // alive as long as this process is up
    RexxInitialize();
    // enable calling back to our code if REXX interpret "hits"
    // a unknown object name
    SetNovalueCallback(engineDispatch);
    // enable calling back for VALUE function with WSHPROPERTY selector
    SetWSHPropertyChange(propertyChange);
    // turn off dispatch messages, otherwise the WSH will interfere
    // with the execution of the engine code and will cause everything
    // to hang...
    RexxSetProcessMessages(FALSE);

    mutex = CreateMutex(NULL,false,NULL); // we're in deep trouble if this is zero after the call!

    // because the exit handler registration is cross-process,
    // a counting semaphore is used to create it, keep it alive
    // and remove it once all orxscrpt.dll's have shut down (last one
    // turns out the light...)
    if ( (semaphore = OpenSemaphore(SEMAPHORE_MODIFY_STATE|SYNCHRONIZE,false,"RexxEngineSemaphore")) == NULL) {
      #if defined(DEBUGZ)
      FPRINTF2(DLLlogfile,"registering exit handlers\n");
      #endif
      // semaphore does not exist, creating it (set usage count to 0)
      semaphore = CreateSemaphore(NULL,0,100000,"RexxEngineSemaphore");
      if (semaphore == NULL) {} // we're toast

      rc = RexxRegisterExitDll("RexxCatchExit","orxscrpt.dll","RexxCatchExit",NULL,RXEXIT_NONDROP);
      rc += RexxRegisterExitDll("RexxCatchExternalFunc","orxscrpt.dll","RexxCatchExternalFunc",NULL,RXEXIT_NONDROP);
      rc += RexxRegisterExitDll("RexxRetrieveVariables","orxscrpt.dll","RexxRetrieveVariables",NULL,RXEXIT_NONDROP);
      if (rc != RXEXIT_OK) {
        #if defined(DEBUGC)+defined(DEBUGZ)
        FPRINTF2(DLLlogfile,"registration failed!\n");
        #endif
      } // we're toast again...
    } else {
      #if defined(DEBUGZ)
      FPRINTF2(DLLlogfile,"incrementing usage count on exit handler\n");
      #endif
      // semaphore exists, increase usage count
      ReleaseSemaphore(semaphore,1,NULL);
    }
#if defined(DEBUGC)+defined(DEBUGZ)
    FPRINTF2(DLLlogfile,"DllMain ATTACH - complete\n");
#endif
  } else if ( ul_reason_for_call == DLL_PROCESS_DETACH) {
#if defined(DEBUGC)+defined(DEBUGZ)
    FPRINTF2(DLLlogfile,"DllMain DETACH\n");
#endif

    //  If we are going away before all of the engines are gone,
    // then close them, so they will release their resources.
    if(EngineChain) {
      Current = EngineChain->FindItem(0);
      while(Current) {
        Content = (OrxScript *)Current->GetContent();
        delete Content;
        Current = EngineChain->FindItem();
        }
      delete EngineChain;
      }

    // shut down REXX
    RexxTerminate();

    if (mutex) CloseHandle(mutex);

    if (semaphore) {
      // decrease exit handler usage count, if zero, deregister it
      if (WaitForSingleObject(semaphore,0) == WAIT_TIMEOUT) {
        #if defined(DEBUGC)+defined(DEBUGZ)
        FPRINTF2(DLLlogfile,"deregistering exit handlers\n");
        #endif
        RexxDeregisterExit("RexxRetrieveVariables","orxscrpt.dll");
        RexxDeregisterExit("RexxCatchExternalFunc","orxscrpt.dll");
        RexxDeregisterExit("RexxCatchExit","orxscrpt.dll");
        #if defined(DEBUGC)+defined(DEBUGZ)
        FPRINTF2(DLLlogfile,"We are done with RexxDeregisterExit\n");
        #endif
      } else {
        #if defined(DEBUGC)+defined(DEBUGZ)
        FPRINTF2(DLLlogfile,"decremented usage counter for exit handler\n");
        #endif
      }
      CloseHandle(semaphore);   // this has to be done in any case...
    }
    // remove this list
    if (thread2EngineList) delete thread2EngineList;
    // remove critical section
    DeleteCriticalSection(&EngineSection);

#if defined(DEBUGC)+defined(DEBUGZ)
    FPRINTF2(DLLlogfile,"DllMain DETACH - complete\n");
    if (DLLlogfile != stderr) fclose(DLLlogfile);
#endif
    }

  return TRUE;
  }

STDAPI DllCanUnloadNow(void) {
  HRESULT RetCode;

  /* The DLL can be unloaded when no OLE object is in existance and no */
  /* lock has been issued against this DLL (for performance reasons).  */
#if defined(DEBUGC)+defined(DEBUGZ)
  FPRINTF(DLLlogfile,"DllCanUnloadNow: DllReferences=%d DllLocks=%d\n", ulDllReferences, ulDllLocks);
#endif
  if ( (ulDllReferences == 0) && (ulDllLocks == 0) )
    RetCode = S_OK;
  else
    RetCode = S_FALSE;
#if defined(DEBUGC)+defined(DEBUGZ)
  FPRINTF2(DLLlogfile,"DllCanUnloadNow: HRESULT %08x\n", RetCode);
#endif
  return RetCode;
}


STDAPI DllGetClassObject(REFCLSID rclsid,
                         REFIID riid,
                         LPVOID *ppv)
{
  /* This call returns a pointer to the IUnknown interface of the OLE */
  /* class implemented by this DLL.                                   */
  OrxClassFactory *pClassFac = NULL;
  HRESULT         hr;

#if defined(DEBUGC)+defined(DEBUGZ)
  FPRINTF(DLLlogfile,"DllGetClassObject\n");
#endif
  /* check input arguments */
  if (ppv == NULL)
    return ResultFromScode(E_INVALIDARG);

  if ( (riid != IID_IUnknown) && (riid != IID_IClassFactory) )
    return ResultFromScode(E_INVALIDARG);

  /* create class factory object for this DLL */
  /*  This is what creates the engine object. */
  pClassFac = CreateClassFactory();
  if (pClassFac == NULL)
    return ResultFromScode(E_OUTOFMEMORY);

  /* check that correct classid is requested */
  if (rclsid != pClassFac->GetClassID())
  {
    pClassFac->Release();
    return ResultFromScode(E_INVALIDARG);
  }

  /* query requested interface from class object */
  /*  If that interface is supported, then the object
   * RefCount is incremented, and the Release() does
   * nothing, otherwise the object removes itself.
   */
  hr = pClassFac->QueryInterface(riid, ppv);
  pClassFac->Release();
  return hr;
}


STDAPI DllRegisterServer(void)
{
  /* This call registers the OLE class in the Windows registry. */
  OrxClassFactory *pClassFac = NULL;
  HRESULT         hr;

#if defined(DEBUGC)+defined(DEBUGZ)
  FPRINTF(DLLlogfile,"DllRegisterServer\n");
#endif
  /* create class factory object */
  pClassFac = CreateClassFactory();
  if (pClassFac == NULL)
    return ResultFromScode(E_UNEXPECTED);

  /* register server and return result to caller */
  hr = pClassFac->RegisterServer();
  pClassFac->Release();
  return hr;
}


STDAPI DllUnregisterServer(void)
{
  /* This call deregisters the OLE class from the Windows registry. */
  OrxClassFactory *pClassFac = NULL;
  HRESULT         hr;

#if defined(DEBUGC)+defined(DEBUGZ)
  FPRINTF(DLLlogfile,"DllUnregisterServer\n");
#endif
  /* create class factory object */
  pClassFac = CreateClassFactory();
  if (pClassFac == NULL)
    return ResultFromScode(E_UNEXPECTED);

  /* unregister server and return result to caller */
  hr = pClassFac->UnregisterServer();
  pClassFac->Release();
  return hr;
}
