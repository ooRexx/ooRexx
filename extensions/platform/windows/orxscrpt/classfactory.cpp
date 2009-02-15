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
#include "classfactory.hpp"


/* constants used for class registration */
#define szSERVERKEY     "InprocServer32"
#define szTHREADMODEL   "Both"

#ifdef _NO_SCRIPT_GUIDS
#error "no, this is not allowed"
#endif

/*---------------------------------------------------------------------------*/

/* Helper function to create a component category and associated description */
HRESULT CreateComponentCategory(CATID catid, OLECHAR* catDescription)
{

    ICatRegister *pcr = NULL;
    HRESULT      hr = S_OK;
    CATEGORYINFO catinfo;
    size_t       iLen;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
                          NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister,
                          (void**)&pcr);
    if (FAILED(hr))
    {
        return hr;
    }

    /* Make sure the HKCR\Component Categories\{..catid...} key is registered */
    catinfo.catid = catid;
    catinfo.lcid = 0x0409 ; // english

    /* Make sure the provided description is not too long (max 127 chars) */
    iLen = wcslen(catDescription);
    if (iLen>127)
    {
        iLen = 127;
    }

    wcsncpy(catinfo.szDescription, catDescription, iLen);

    /* Make sure the description is null terminated */
    catinfo.szDescription[iLen] = '\0';

    hr = pcr->RegisterCategories(1, &catinfo);
    pcr->Release();

    return hr;
}


/* Helper function to register a CLSID as belonging to a component category */
HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
    /* Register your component categories information. */
    ICatRegister *pcr = NULL ;
    HRESULT      hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
                          NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister,
                          (void**)&pcr);
    if (SUCCEEDED(hr))
    {
        /* Register this category as being "implemented" by the class. */
        CATID   rgcatid[1];

        rgcatid[0] = catid;
        hr = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
    {
        pcr->Release();
    }

    return hr;
}


/* Helper function to unregister a CLSID as belonging to a component category */
HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
    ICatRegister *pcr = NULL ;
    HRESULT      hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
                          NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister,
                          (void**)&pcr);
    if (SUCCEEDED(hr))
    {
        /* Unregister this category as being "implemented" by the class. */
        CATID   rgcatid[1];

        rgcatid[0] = catid;
        hr = pcr->UnRegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
    {
        pcr->Release();
    }

    return hr;
}


/* StringFromGuidA returns an ANSI string from a CLSID or GUID */
int StringFromGuidA(REFIID riid, LPSTR pszBuf)
{
    return sprintf((char *)pszBuf,
                 "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                 riid.Data1, riid.Data2, riid.Data3, riid.Data4[0],
                 riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4],
                 riid.Data4[5], riid.Data4[6], riid.Data4[7]);
}

/*---------------------------------------------------------------------------*/

STDMETHODIMP OrxClassFactory::QueryInterface(REFIID riid, void** ppvObj)
{

    char    cIID[100],TrulyUnknown[]="??????";

    StringFromGUID2(riid,(LPOLESTR)cIID,sizeof(cIID)/2);
    FPRINTF(StreamProcessEngine::logfile,"OrxClassFactory::QueryInterface %S\r\n",cIID);
    if (ppvObj == 0)
    {
        FPRINTF2(StreamProcessEngine::logfile,"OrxClassFactory::QueryInterface Bad Return Pointer\n");
        return E_POINTER;
    }
    *ppvObj = NULL;

    if (riid == IID_IUnknown)
    {
        FPRINTF2(StreamProcessEngine::logfile,"OrxClassFactory::QueryInterface IID_IUnknown\n");
        *ppvObj = (IUnknown *) this;
    }
    else if (riid == IID_IClassFactory)
    {
        FPRINTF2(StreamProcessEngine::logfile,"OrxClassFactory::QueryInterface IID_IClassFactory\n");
        *ppvObj = (IClassFactory *) this;
    }

    if (*ppvObj == NULL)
    {
        char *IIDName;
        if (!(IIDName = NameThatInterface((OLECHAR *)&cIID[0])))
        {
            IIDName = &TrulyUnknown[0];
        }
        FPRINTF2(StreamProcessEngine::logfile,"OrxClassFactory::QueryInterface Unsupported Interface (%s)\n",IIDName);
        if (IIDName != &TrulyUnknown[0])
        {
            free(IIDName);
        }
        return ResultFromScode(E_NOINTERFACE);
    }
    else
    {
        ((IUnknown*) *ppvObj)->AddRef();
        return NOERROR;
    }
}


STDMETHODIMP_(ULONG) OrxClassFactory::AddRef(void)
{
    FPRINTF(StreamProcessEngine::logfile,"OrxClassFactory::AddRef: ulRefCounter=%d (before incr.)\r\n", ulRefCounter);
    return ++ulRefCounter;
}


STDMETHODIMP_(ULONG) OrxClassFactory::Release(void)
{
    ULONG ulTempRefCounter;

    FPRINTF(StreamProcessEngine::logfile,"OrxClassFactory::Release\r\n");
    ulTempRefCounter = --ulRefCounter;

    if (ulRefCounter == 0)
    {
        delete this;
    }

    FPRINTF2(StreamProcessEngine::logfile,"ulRefCounter=%d\r\n", ulTempRefCounter);
    return ulTempRefCounter;
}


STDMETHODIMP OrxClassFactory::LockServer(BOOL fLock)
{
    FPRINTF(StreamProcessEngine::logfile,"OrxClassFactory::LockServer: fLock=%d\n", fLock);
    if (fLock)
    {
        InterlockedIncrement((long *)&ulDllLocks); //++ulDllLocks;
    }
    else
    {
        InterlockedDecrement((long *)&ulDllLocks);  //--ulDllLocks;
    }

    FPRINTF2(StreamProcessEngine::logfile,"ulDllLocks=%d\r\n", ulDllLocks);
    return NOERROR;
}


STDMETHODIMP OrxClassFactory::RegisterServer()
{
    HKEY      hk;
    HKEY      hkSub;
    HKEY      hkSub2;
    HKEY      hkthread;
    HRESULT   hr = NOERROR;
    const     int REGPATHMAX = 512;
    char      szDllPath[REGPATHMAX];
    ULONG     ulDllPathLen;

    FPRINTF(StreamProcessEngine::logfile,"OrxClassFactory::RegisterServer\r\n");

    /* Make a clean start */
    UnregisterServer();

    /* Create scripting categories if they do not exist yet */
    CreateComponentCategory(CATID_ActiveScript,
                            OLESTR("Active Scripting Engine"));
    CreateComponentCategory(CATID_ActiveScriptParse,
                            OLESTR("Active Scripting Engine with Parsing"));

    /* Register ourselves in the scripting categories */
    RegisterCLSIDInCategory(guidCLASSID, CATID_ActiveScript);
    RegisterCLSIDInCategory(guidCLASSID, CATID_ActiveScriptParse);

    /* Register server info */
    if (RegCreateKey(HKEY_CLASSES_ROOT, szLANGNAME, &hk) == ERROR_SUCCESS)
    {
        if (RegCreateKey(hk, "OLEScript", &hkSub) != ERROR_SUCCESS)
        {
            RegCloseKey(hk);
            return ResultFromScode(E_FAIL);
        }

        RegCloseKey(hkSub);

        RegSetValue(HKEY_CLASSES_ROOT, szLANGNAME, REG_SZ, szDESCRIPTION,
                    strlen(szDESCRIPTION));
        RegSetValue(hk, "CLSID", REG_SZ, szCLASSID, strlen(szCLASSID));
        RegCloseKey(hk);
    }
    else
    {
        return ResultFromScode(E_FAIL);
    }

    /* Register "alternate name" info */
    if (szALTERNATELANGNAME)
    {
        if (RegCreateKey(HKEY_CLASSES_ROOT, szALTERNATELANGNAME, &hk) == ERROR_SUCCESS)
        {
            if (RegCreateKey(hk, "OLEScript", &hkSub) != ERROR_SUCCESS)
            {
                RegCloseKey(hk);
                return ResultFromScode(E_FAIL);
            }

            RegCloseKey(hkSub);
            RegSetValue(HKEY_CLASSES_ROOT, szALTERNATELANGNAME, REG_SZ,
                        szDESCRIPTION, strlen(szDESCRIPTION));
            RegSetValue (hk, "CLSID", REG_SZ, szCLASSID, strlen(szCLASSID));
            RegCloseKey (hk);
        }
        else
        {
            return ResultFromScode(E_FAIL);
        }
    }

    /* Register createable class */
    if (RegCreateKey(HKEY_CLASSES_ROOT, "CLSID", &hk) == ERROR_SUCCESS)
    {
        if (RegCreateKey(hk, szCLASSID, &hkSub) != ERROR_SUCCESS)
        {
            RegCloseKey(hk);
            return ResultFromScode (E_FAIL);
        }

        if (RegCreateKey(hkSub, "OLEScript", &hkSub2) != ERROR_SUCCESS)
        {
            RegCloseKey(hk);
            RegCloseKey(hkSub);
            return ResultFromScode(E_FAIL);
        }

        ulDllPathLen = (ULONG) GetModuleFileName(GetModuleHandle(szDLLNAME),
                                                 szDllPath, REGPATHMAX);

        if (!ulDllPathLen)
        {
            RegCloseKey(hkSub2);
            RegCloseKey(hkSub);
            RegCloseKey(hk);
            return ResultFromScode(E_FAIL);
        }

        RegSetValue(hk, szCLASSID, REG_SZ, szDESCRIPTION, strlen(szDESCRIPTION));
        RegSetValue(hkSub, "ProgID", REG_SZ, szLANGNAME, strlen(szLANGNAME));
        RegSetValue(hkSub, szSERVERKEY, REG_SZ, szDllPath, ulDllPathLen);

        /* To set the threading model reg entry use the RegOpenKeyEx & SetValueEx */
        if (RegOpenKeyEx(hkSub, szSERVERKEY, 0, KEY_SET_VALUE, &hkthread) == ERROR_SUCCESS)
        {
            RegSetValueEx(hkthread, "ThreadingModel", 0, REG_SZ,
                          (BYTE *) szTHREADMODEL, strlen(szTHREADMODEL));
            RegCloseKey(hkthread);
        }
        else
        {
            RegCloseKey(hkthread);
            RegCloseKey(hkSub2);
            RegCloseKey(hkSub);
            RegCloseKey (hk);
            return ResultFromScode(E_FAIL);
        }

        RegCloseKey(hkSub2);
        RegCloseKey(hkSub);
        RegCloseKey(hk);
    }
    else
    {
        return ResultFromScode(E_FAIL);
    }

    /* Register the extension */
    if (szEXTENSION)
    {
        if (RegSetValue(HKEY_CLASSES_ROOT, szEXTENSION, REG_SZ, szLANGFILE, strlen(szLANGFILE)) != ERROR_SUCCESS)
        {
            return ResultFromScode(E_FAIL);
        }
    }

    /* Register the language file */
    if (szLANGFILE)
    {
        if (RegCreateKey(HKEY_CLASSES_ROOT, szLANGFILE, &hk) == ERROR_SUCCESS)
        {
            if (RegCreateKey(hk, "ScriptEngine", &hkSub) != ERROR_SUCCESS)
            {
                RegCloseKey(hk);
                return ResultFromScode(E_FAIL);
            }

            RegCloseKey(hkSub);

            if (RegCreateKey(hk, "DefaultIcon", &hkSub) != ERROR_SUCCESS)
            {
                RegCloseKey(hk);
                return ResultFromScode(E_FAIL);
            }

            RegSetValueEx(hkSub, NULL, 0, REG_EXPAND_SZ, (const unsigned char *) szDEFAULTICON, strlen(szDEFAULTICON));
            RegCloseKey(hkSub);

            RegSetValue(HKEY_CLASSES_ROOT, szLANGFILE, REG_SZ, szFILEDESCRIPTION,
                        strlen(szFILEDESCRIPTION));
            RegSetValue(hk, "ScriptEngine", REG_SZ, szLANGNAME, strlen(szLANGNAME));
            createSubKeys(hk);
            RegCloseKey(hk);
        }
        else
        {
            return ResultFromScode(E_FAIL);
        }
    }

    return S_OK;
}


STDMETHODIMP OrxClassFactory::UnregisterServer()
{
    HKEY     hk;
    HKEY     hkSub;
    HRESULT  hr = S_OK;

    FPRINTF(StreamProcessEngine::logfile,"OrxClassFactory::UnregisterServer\r\n");

    /* UnRegister ourselves from the category */
    UnRegisterCLSIDInCategory(guidCLASSID, CATID_ActiveScript);
    UnRegisterCLSIDInCategory(guidCLASSID, CATID_ActiveScriptParse);

    /* delete server info */
    if (RegOpenKey(HKEY_CLASSES_ROOT, szLANGNAME, &hk) != ERROR_SUCCESS)
    {
        FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegOpenKey HKCR\\%s\n",szLANGNAME);
        hr = ResultFromScode(E_FAIL);
    }
    else
    {
        if (RegDeleteKey(hk, "CLSID") != ERROR_SUCCESS)
        {
            FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\\CLSID\n",szLANGNAME);
            hr = ResultFromScode(E_FAIL);
        }

        if (RegDeleteKey(hk, "OLEScript") != ERROR_SUCCESS)
        {
            FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\\OLEScript\n",szLANGNAME);
            hr = ResultFromScode (E_FAIL);
        }

        RegCloseKey(hk);

        if (RegDeleteKey(HKEY_CLASSES_ROOT, szLANGNAME) != ERROR_SUCCESS)
        {
            FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey HKCR\\%s\n",szLANGNAME);
            hr = ResultFromScode(E_FAIL);
        }
    }

    /* delete alternate name entry */
    if (szALTERNATELANGNAME)
    {
        if (RegOpenKey(HKEY_CLASSES_ROOT, szALTERNATELANGNAME, &hk) != ERROR_SUCCESS)
        {
            FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegOpenKey HKCR\\%s\n", szALTERNATELANGNAME);
            hr = ResultFromScode(E_FAIL);
        }
        else
        {
            if (RegDeleteKey(hk, "CLSID") != ERROR_SUCCESS)
            {
                FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\\CLSID\n",szALTERNATELANGNAME);
                hr = ResultFromScode(E_FAIL);
            }

            if (RegDeleteKey(hk, "OLEScript") != ERROR_SUCCESS)
            {
                FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\\OLEScript\n",szALTERNATELANGNAME);
                hr = ResultFromScode(E_FAIL);
            }

            RegCloseKey(hk);

            if (RegDeleteKey(HKEY_CLASSES_ROOT, szALTERNATELANGNAME) != ERROR_SUCCESS)
            {
                FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\n",szALTERNATELANGNAME);
                hr = ResultFromScode(E_FAIL);
            }
        }
    }

    /* delete entries for CLSID */
    if (RegCreateKey(HKEY_CLASSES_ROOT, "CLSID", &hk) != ERROR_SUCCESS)
    {
        FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegCreateKey HKCR\\CLSID\n");
        hr = ResultFromScode(E_FAIL);
    }
    else
    {
        if (RegOpenKey(hk, szCLASSID, &hkSub) != ERROR_SUCCESS)
        {
            FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegOpenKey %s\n",szCLASSID);
            hr = ResultFromScode(E_FAIL);
        }
        else
        {
            if (RegDeleteKey(hkSub, "ProgID") != ERROR_SUCCESS)
            {
                FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\\ProgID\n",szCLASSID);
                hr = ResultFromScode(E_FAIL);
            }

            if (RegDeleteKey(hkSub, "OLEScript") != ERROR_SUCCESS)
            {
                FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\\OLEScript\n",szCLASSID);
                hr = ResultFromScode(E_FAIL);
            }

            if (RegDeleteKey(hkSub, "Implemented Categories") != ERROR_SUCCESS)
            {
                FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\\Implemented Categories\n",szCLASSID);
                hr = ResultFromScode(E_FAIL);
            }

            if (RegDeleteKey(hkSub, szSERVERKEY) != ERROR_SUCCESS)
            {
                FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\n",szSERVERKEY);
                hr = ResultFromScode(E_FAIL);
            }

            RegCloseKey(hkSub);
        }

        if (RegDeleteKey(hk, szCLASSID) != ERROR_SUCCESS)
        {
            FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\n",szCLASSID);
            hr = ResultFromScode(E_FAIL);
        }

        RegCloseKey(hk);
    }

    if (szEXTENSION)
    {
        /* delete extension */
        if (RegDeleteKey(HKEY_CLASSES_ROOT, szEXTENSION) != ERROR_SUCCESS)
        {
            FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\n",szEXTENSION);
            hr = ResultFromScode(E_FAIL);
        }
    }

    if (szLANGFILE)
    {
        if (RegOpenKey(HKEY_CLASSES_ROOT, szLANGFILE, &hk) != ERROR_SUCCESS)
        {
            FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegOpenKey HKCR\\%s\n",szLANGFILE);
            hr = ResultFromScode(E_FAIL);
        }
        else
        {
            deleteSubKeys(hk);
            if (RegDeleteKey(hk, "ScriptEngine") != ERROR_SUCCESS)
            {
                FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\\ScriptEngine\n",szLANGFILE);
                hr = ResultFromScode(E_FAIL);
            }
            if (RegDeleteKey(hk, "DefaultIcon") != ERROR_SUCCESS)
            {
                FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey %s\\DefaultIcon\n",szLANGFILE);
                hr = ResultFromScode(E_FAIL);
            }
            RegCloseKey(hk);
        }
        if (RegDeleteKey(HKEY_CLASSES_ROOT, szLANGFILE) != ERROR_SUCCESS)
        {
            FPRINTF(StreamProcessEngine::logfile,"  FAIL: RegDeleteKey HKCR\\%s\n",szLANGFILE);
            hr = ResultFromScode(E_FAIL);
        }
    }
    return hr;
}


HRESULT OrxClassFactory::deleteSubKeys(HKEY parent)
{
    HKEY hk1, hk2;
    HRESULT hResult = E_FAIL;

    if (RegOpenKey(parent, "Shell", &hk1) == ERROR_SUCCESS)
    {
        // delete EDIT entry
        if (RegOpenKey(hk1, "Edit", &hk2) == ERROR_SUCCESS)
        {
            RegDeleteKey(hk2, "Command");
            RegCloseKey(hk2);
            RegDeleteKey(hk1, "Edit");
        }
        // delete OPEN entry
        if (RegOpenKey(hk1, "Open", &hk2) == ERROR_SUCCESS)
        {
            RegDeleteKey(hk2, "Command");
            RegCloseKey(hk2);
            RegDeleteKey(hk1, "Open");
        }
        // delete OPEN2 entry
        if (RegOpenKey(hk1, "Open2", &hk2) == ERROR_SUCCESS)
        {
            RegDeleteKey(hk2, "Command");
            RegCloseKey(hk2);
            RegDeleteKey(hk1, "Open2");
        }
        // delete PRINT entry
        if (RegOpenKey(hk1, "Print", &hk2) == ERROR_SUCCESS)
        {
            RegDeleteKey(hk2, "Command");
            RegCloseKey(hk2);
            RegDeleteKey(hk1, "Print");
        }
        // delete SHELL entry
        RegCloseKey(hk1);
        if (RegDeleteKey(parent, "Shell") == ERROR_SUCCESS)
        {
            hResult = S_OK;
        }
    }
    return hResult;
}

HRESULT OrxClassFactory::createSubKeys(HKEY parent)
{
    HKEY hk1, hk2, hk3;
    HRESULT hResult = E_FAIL;

    if (RegCreateKey(parent, "Shell", &hk1) == ERROR_SUCCESS)
    {
        hResult = S_OK;
        // create EDIT entry
        if (RegCreateKey(hk1, "Edit", &hk2) == ERROR_SUCCESS)
        {
            if (RegCreateKey(hk2, "Command", &hk3) == ERROR_SUCCESS)
            {
                RegSetValueEx(hk3, NULL, 0, REG_EXPAND_SZ, (const unsigned char *) szSHELLEDIT, strlen(szSHELLEDIT));
                RegCloseKey(hk3);
            }
            RegCloseKey(hk2);
        }
        // create OPEN entry
        if (RegCreateKey(hk1, "Open", &hk2) == ERROR_SUCCESS)
        {
            if (RegCreateKey(hk2, "Command", &hk3) == ERROR_SUCCESS)
            {
                RegSetValueEx(hk3, NULL, 0, REG_EXPAND_SZ, (const unsigned char *) szSHELLOPEN, strlen(szSHELLOPEN));
                RegCloseKey(hk3);
            }
        }
        // create OPEN2 entry
        if (RegCreateKey(hk1, "Open2", &hk2) == ERROR_SUCCESS)
        {
            if (RegCreateKey(hk2, "Command", &hk3) == ERROR_SUCCESS)
            {
                RegSetValueEx(hk3, NULL, 0, REG_EXPAND_SZ, (const unsigned char *) szSHELLOPEN2, strlen(szSHELLOPEN2));
                RegCloseKey(hk3);
            }
        }
        // create PRINT entry
        if (RegCreateKey(hk1, "Print", &hk2) == ERROR_SUCCESS)
        {
            if (RegCreateKey(hk2, "Command", &hk3) == ERROR_SUCCESS)
            {
                RegSetValueEx(hk3, NULL, 0, REG_EXPAND_SZ, (const unsigned char *) szSHELLPRINT, strlen(szSHELLPRINT));
                RegCloseKey(hk3);
            }
        }
        RegCloseKey(hk1);
    }
    return hResult;
}
