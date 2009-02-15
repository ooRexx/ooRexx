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
 #ifndef REXX_CLASSFACTORY
#define REXX_CLASSFACTORY


#include "orxscrpt_main.hpp"

// some misc registry information
#define szDEFAULTICON         "%SystemRoot%\\System32\\WScript.exe,3"
#define szSHELLEDIT           "%SystemRoot%\\System32\\Notepad.exe \"%1\""
#define szSHELLOPEN           "%SystemRoot%\\System32\\WScript.exe \"%1\" %*"
#define szSHELLOPEN2          "%SystemRoot%\\System32\\CScript.exe \"%1\" %*"
#define szSHELLPRINT          "%SystemRoot%\\System32\\Notepad.exe /p \"%1\""

/* functions to increment and decrement DLL reference count */
inline void DLLAddReference(void)
{
  InterlockedIncrement((LPLONG)&ulDllReferences);
#if defined(DEBUGZ)
  FPRINTF(StreamProcessEngine::logfile,"DLLAddReference: DLLReferences=%d\r\n", ulDllReferences);
#endif
}

inline void DLLRelease(void)
{
  InterlockedDecrement((LPLONG)&ulDllReferences);
#if defined(DEBUGZ)
  FPRINTF(StreamProcessEngine::logfile,"DLLRelease: DLLReferences=%d\r\n", ulDllReferences);
#endif
}


class OrxClassFactory : public IClassFactory
{
  public:

  /* constructor saves class specific information */
  OrxClassFactory(char        *szServerName,
                  REFCLSID    guidClassID,
                  char        *szClassID,
                  char        *szDescription,
                  char        *szExtension,
                  char        *szLangFile,
                  char        *szFileDescription,
                  char        *szDllName,
                  char        *szLangName,
                  char        *szAlternateLangName) :
    szSERVERNAME(szServerName),
    guidCLASSID(guidClassID),
    szCLASSID(szClassID),
    szDESCRIPTION(szDescription),
    szEXTENSION(szExtension),
    szLANGFILE(szLangFile),
    szFILEDESCRIPTION(szFileDescription),
    szDLLNAME(szDllName),
    szLANGNAME(szLangName),
    szALTERNATELANGNAME(szAlternateLangName),
    ulRefCounter(1L)
  {
    DLLAddReference();
  } /* end of constructor */

  /* constructor decreases reference count */
  ~OrxClassFactory()
  {
    DLLRelease();
  } /* end of destructor */

  /* methods from IUnknown and IClassFactory */
  STDMETHODIMP QueryInterface(REFIID, void **);
  STDMETHODIMP_(ULONG) AddRef(void);
  STDMETHODIMP_(ULONG) Release(void);
  STDMETHODIMP LockServer(BOOL);

  /* methods that need to be implemented by subclasses */
  virtual REFIID GetClassID() = 0;  // erh? this class will implement it = 0 makes no sense!
  STDMETHODIMP CreateInstance(IUnknown *, REFIID, void **) = 0;

  /* new methods implemented by this class */
  STDMETHODIMP RegisterServer();
  STDMETHODIMP UnregisterServer();

  /* public member variables */
  char   *szSERVERNAME;
  char   *szCLASSID;
  char   *szDESCRIPTION;
  char   *szEXTENSION;
  char   *szLANGFILE;
  char   *szFILEDESCRIPTION;
  char   *szDLLNAME;
  char   *szLANGNAME;
  char   *szALTERNATELANGNAME;
  CLSID  guidCLASSID;

  private:
  /* private methods */
  HRESULT createSubKeys(HKEY);  // create...
  HRESULT deleteSubKeys(HKEY);  // ...and delete a bunch of misc. registry entries
  /* private member variables */
  ULONG  ulRefCounter;
};

#endif
