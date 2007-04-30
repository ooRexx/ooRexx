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

#include <process.h>
#include "eng2rexx.hpp"
#include "scriptutil.hpp"

#undef DEBUGC
#undef DEBUGZ

// these are definitions from urlmon.h - needed for security manager of IE
// instead of having to use the extra package of IE they are defined here
// (see INETSDK and it's successors for details!)
// note: it may be that Microsoft decides to include these into the SDK
//       at some later point - it's mixed up already (IObjectSafety, for example) -
//       these lines may result in compiler errors and would have to be removed
// MHES 29122004 - commented out ENG001A
//#define CONFIRMSAFETYACTION_LOADOBJECT  0x00000001
//static const GUID GUID_CUSTOM_CONFIRMOBJECTSAFETY = { 0x10200490, 0xfa38, 0x11d0, { 0xac, 0xe, 0x0, 0xa0, 0xc9, 0xf, 0xff, 0xc0 } };
//#ifdef _OLDWINDOWS
//struct CONFIRMSAFETY
//    {
//    CLSID clsid;
//    IUnknown __RPC_FAR *pUnk;
//    DWORD dwFlags;
//    };
//#endif

// include struct definition if CONFIRMSAFETYACTION_LOADOBJECT not defined - more portable
#ifndef CONFIRMSAFETYACTION_LOADOBJECT
# define CONFIRMSAFETYACTION_LOADOBJECT  0x00000001
struct CONFIRMSAFETY
    {
    CLSID clsid;
    IUnknown __RPC_FAR *pUnk;
    DWORD dwFlags;
    };
#endif
static const GUID GUID_CUSTOM_CONFIRMOBJECTSAFETY = { 0x10200490, 0xfa38, 0x11d0, { 0xac, 0xe, 0x0, 0xa0, 0xc9, 0xf, 0xff, 0xc0 } };

OrxScript* findEngineForThread(DWORD);

OrxScript *CurrentEngine;

RexxArray* __stdcall DispParms2RexxArray(void *arguments)
{
  RexxArray *result = NULL;
  RexxObject *temp;
  DISPPARAMS *dp = (DISPPARAMS*) arguments;
  int j;

  //   Thanks to the wonderful way that Windows passes variants,
  // this routine must reverse the args....
  if (dp) {
    j = dp->cArgs;
    result = (RexxArray*) RexxArray(j);
    for (int i=0; i<j; i++) {
      temp = Variant2Rexx(&dp->rgvarg[j-i-1]);
      array_put(result,temp,i+1);
    }

  }
  // no arguments? set in default empty string
  else {
    result = (RexxArray*) RexxArray(1);
    array_put(result,RexxString(""),1);
  }

  return result;
}

/* Exit handler to find out the names of any functions in currently parsed script */
LONG APIENTRY RexxCatchExit(LONG ExitNumber, LONG Subfunction, PEXIT parmblock)
{
   char **names;
   int  iCount;
   LinkedList *myList = NULL;

   // get LISTOFNAMES, containing a pointer to the list that will
   // store all names we find out here...
   sscanf(((RexxString*) REXX_GETVAR("LISTOFNAMES"))->stringData,"%p",&myList);

   if (myList) {

     /* obtain all PUBLIC routines from the current activation */
     REXX_GETFUNCTIONNAMES(&names,&iCount);
     // if there were any, add to the list
     if (iCount) {
       while (iCount) {
         iCount--;
         myList->AddItem(names[iCount],LinkedList::Beginning,NULL);
         GlobalFree(names[iCount]);
       }
       GlobalFree(names);
     }
   }

   // always return with NOT_HANDLED...
   return RXEXIT_NOT_HANDLED;
}

void __stdcall fillVariables(void*a)
{
  //  Why can't we use this here?
  // OrxScript  *engine = findEngineForThread(GetCurrentThreadId());
  CurrentEngine->insertVariable(a);
}

/* Exit handler to find out the values of variables of "immediate code" */
LONG APIENTRY RexxRetrieveVariables(LONG ExitNumber, LONG Subfunction, PEXIT parmblock)
{
  OrxScript     *engine    = findEngineForThread(GetCurrentThreadId());

  if (engine) {
    // we need to pass a function, a member of an object doesn't work...
    // therefore a global variable that is holding the pointer to the
    // current engine. beware: this doesn't work in multi-threads...
    // >>> ??? <<< Can we put a mutex check on this?
    CurrentEngine = engine;
    WinGetVariables(fillVariables);
  }

  // always return with NOT_HANDLED...
  return RXEXIT_NOT_HANDLED;
}

/* Exit handler for external function calls */
LONG APIENTRY RexxCatchExternalFunc(LONG ExitNumber, LONG Subfunction, PEXIT pblock)
{
//   RXFNC_FLAGS       rxfnc_flags ;     /* function flags             */
//   PUCHAR            rxfnc_name;       /* Pointer to function name.  */
//   USHORT            rxfnc_namel;      /* Length of function name.   */
//   PUCHAR            rxfnc_que;        /* Current queue name.        */
//   USHORT            rxfnc_quel;       /* Length of queue name.      */
//   USHORT            rxfnc_argc;       /* Number of args in list.    */
//   PRXSTRING         rxfnc_argv;       /* Pointer to argument list.  */
//   RXSTRING          rxfnc_retc;       /* Return value.              */
  RXFNCCAL_PARM *parmblock = (RXFNCCAL_PARM*) pblock;
  RexxObject    *result    = RexxNil;
  char          *fncname   = (char*) parmblock->rxfnc_name;
  OrxScript     *engine    = findEngineForThread(GetCurrentThreadId());
  PRCB           pImage    = NULL;
  RXSTRING      *pCode     = NULL;
  LONG           state = RXEXIT_NOT_HANDLED;
  bool           function = !parmblock->rxfnc_flags.rxffsub;  // called as function?

  HRESULT        hResult;
  DISPID         dispID;
  DWORD          flags;
  IDispatch     *pDispatch = NULL;
  ITypeInfo     *pTypeInfo = NULL;
  DISPPARAMS     dp;
  VARIANT        sResult;
  VARIANT       *pResult;
  EXCEPINFO      sExc;
  unsigned int   uArgErr;
  bool           test = false;

#if defined(DEBUGC)+defined(DEBUGZ)
  static int ecount = 0;
  char fname[64];
  FILE *logfile;

  sprintf(fname,"c:\\temp\\extfunc%d.log",++ecount);
  logfile = fopen(fname,"w");
#endif

  // have an engine to work with?
  if (engine) {
#if defined(DEBUGZ)
    FPRINTF(logfile,"using engine %s, looking for %s\n",engine->getEngineName(),fncname);
#endif
    // is the function we're looking for a REXX method that was defined for this
    // script engine earlier?
    pImage = (RCB*) engine->findRexxFunction((char*) parmblock->rxfnc_name);
    if (pImage) {
      // the function that needs to be called is a REXX method NOT defined
      // in the current REXX script scope

      // argument array for runMethod
      LPVOID    arguments[8];
      char      invString[256];
      char      buffer[8];
      ConditionData cd;

      // build argument array of REXX objects...
      RexxArray *args = (RexxArray*) RexxArray(1+parmblock->rxfnc_argc);
      RexxObject *temp;

      // ...and invocation string
      if (function)
        sprintf(invString,"return %s(",parmblock->rxfnc_name);
      else
        sprintf(invString,"call %s ",parmblock->rxfnc_name);
      for (int i=0;i<parmblock->rxfnc_argc;i++) {
        sscanf(parmblock->rxfnc_argv[i].strptr,"%p",&temp);
        array_put(args,temp,i+2); // does this change current activity?? seems to be NULL when we run into okarray.c
        sprintf(buffer,"arg(%d)",i+2); // +2,because 1st argument is "CALL %s..." string
        strcat(invString,buffer);
        // not the last argument?
        if (i<parmblock->rxfnc_argc-1)
          strcat(invString,",");
        // if there are too many arguments, raise error (maybe we need a bigger
        // string, or a different way of passing the args?)
        if (strlen(invString) > 240) {
          sprintf(invString,"RAISE SYNTAX 5");
          break;
        }
      }
      if (function)
        strcat(invString,")");
      else
        strcat(invString,"; exit");

#if defined(DEBUGZ)
      FPRINTF2(logfile,"invoke string: %s\n",invString);
#endif
      // invocation statement
      array_put(args,RexxString(invString),1);

      arguments[0] = (void*) engine; // engine that is running this code
      arguments[1] = (void*) pImage; // method to run
      arguments[2] = NULL;           // no COM arguments
      arguments[3] = (void*) args;   // we have REXX arguments
      arguments[4] = (void*) &result;// result object
      arguments[5] = (void*) &cd;    // condition information
      arguments[6] = (void*) false;  // don't end this thread
      arguments[7] = (void*) false;  // don't care about variables at the end
      runMethod(arguments);
      state = RXEXIT_HANDLED;
      sprintf(parmblock->rxfnc_retc.strptr,"%p",result);
      parmblock->rxfnc_retc.strlength = 8;
#if defined(DEBUGZ)
      FPRINTF2(logfile,"invocation result %p (rc = %d)\n",result,cd.rc);
#endif
      // if something went wrong, indicate an error to REXX
      if (cd.rc != 0) {
        // indicate error in parmblock...
        parmblock->rxfnc_flags.rxfferr = 1;
        // we don't need to notify the engine of the error
        // here, because this was already done by the part
        // of the code that invoked the REXX codeblock
      }
    } else {
      // ask the named items if one of them knows this function
      hResult = engine->getNamedItems()->WhoKnows((char*) parmblock->rxfnc_name,engine->Lang,&dispID,&flags,&pDispatch,&pTypeInfo);
      if (hResult == S_OK) {
        // found something...
        // build DISPPARAMS structure for the invoke
        dp.cNamedArgs = 0;
        dp.cArgs = parmblock->rxfnc_argc;
        VariantInit(&sResult);
        pResult = &sResult;

        // do we have to build an argument array?
        if (parmblock->rxfnc_argc) {
          RexxObject *temp;
          VARIANTARG *pVarArgs = (VARIANTARG*) GlobalAlloc(GMEM_FIXED,(sizeof(VARIANTARG) * dp.cArgs));

          dp.rgvarg = pVarArgs;
          for (int i = 0; i < dp.cArgs; i++) {
            // get the REXX object
            sscanf(parmblock->rxfnc_argv[i].strptr,"%p",&temp);

            // arguments must be filled in from the end of the array...
            VariantInit(&(pVarArgs[dp.cArgs - i - 1]));
            Rexx2Variant(temp, &(pVarArgs[dp.cArgs - i - 1]), VT_EMPTY /*(try your best)*/, 0 /*dummy argument?!*/);
          }
        }
        else
          dp.rgvarg = NULL; // no argument array needed

        if (dispID != -1) {
          hResult = pDispatch->Invoke(dispID, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dp, pResult, &sExc, &uArgErr);
        } else {
          // call the default method of a named item object
          unsigned short wFlags = DISPATCH_METHOD;
          DISPID         PropPutDispId = DISPID_PROPERTYPUT;

          pTypeInfo = NULL;
          hResult = pDispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTypeInfo);

          if (SUCCEEDED(hResult)) {
            TYPEATTR *pTypeAttr = NULL;
            FUNCDESC *pFuncDesc = NULL;
            bool      found = false;

            hResult = pTypeInfo->GetTypeAttr(&pTypeAttr);
            if (SUCCEEDED(hResult)) {
              for (int l = 0; (l < pTypeAttr->cFuncs) && !found; l++) {
                hResult = pTypeInfo->GetFuncDesc(l, &pFuncDesc);
                if (SUCCEEDED(hResult)) {
                  if ((pFuncDesc->cParams + pFuncDesc->cParamsOpt >= parmblock->rxfnc_argc) &&
                    pFuncDesc->memid == 0) {
                    wFlags = pFuncDesc->invkind;
                    found = true;
                  }
                  pTypeInfo->ReleaseFuncDesc(pFuncDesc);
                }
              }

              pTypeInfo->ReleaseTypeAttr(pTypeAttr);
              pTypeInfo->Release();
            }
            if (wFlags == DISPATCH_PROPERTYPUT) {
              dp.cNamedArgs = 1;
              dp.rgdispidNamedArgs = &PropPutDispId;
            }
          }

          hResult = pDispatch->Invoke((DISPID) 0, IID_NULL, LOCALE_USER_DEFAULT, wFlags, &dp, pResult, &sExc, &uArgErr);
        }

        if (SUCCEEDED(hResult)) {
          // success, make REXX object from VARIANT
          result = Variant2Rexx(&sResult);
          state = RXEXIT_HANDLED;
          sprintf(parmblock->rxfnc_retc.strptr,"%p",result);
          parmblock->rxfnc_retc.strlength = 8;
#if defined(DEBUGZ)
          FPRINTF2(logfile,"COM invoke ok, got back rexx object %p\n",result);
#endif
        } else {
          result = NULL; // this is an error. behave accordingly
          parmblock->rxfnc_flags.rxfferr = 1;
        }

        // clear argument array, free memory:
        if (dp.rgvarg) {
          for (int i = 0; i < dp.cArgs; i++)
            VariantClear(&(dp.rgvarg[i]));
          GlobalFree(dp.rgvarg);
        }

        // free result
        VariantClear(&sResult);
      }
    }
  }

#if defined(DEBUGC)+defined(DEBUGZ)
  FPRINTF(logfile,"I %s this function - exiting now.\n",state==RXEXIT_HANDLED?"knew":"didn't know");
  fclose(logfile);
#endif

  return state;
}

// insert engine pointer into thread list so that
// the string unknown can find out which engine to use
// this may be additional work if REXX code is only run
// from one thread. if we have to go to "multithread"
// (again) this will be usefull, there we just leave it
// in here...
void registerEngineForCallback(OrxScript *engine)
{
  char       string[9];
#if defined(DEBUG_EXTRA)
  FILE *logfile;

  logfile = fopen("c:\\temp\\threadtable.log","a");
#endif

  sprintf(string,"%08x",GetCurrentThreadId());
  // exclusive access to this region
  WaitForSingleObject(mutex,INFINITE);
  thread2EngineList->AddItem(string,LinkedList::Beginning,engine);
#if defined(DEBUG_EXTRA)
  {
    int i = 0;
    ListItem *item;
    FPRINTF(logfile,"register\nthread<->engine association (we are %s):\n",string);
    while ( item = thread2EngineList->FindItem(i++) )
      FPRINTF2(logfile,"%2d %s %p\n",i,item->GetName(),item->GetContent());
  }
  fclose(logfile);
#endif
  ReleaseMutex(mutex);
}

// remove engine pointer from thread list
void deregisterEngineForCallback()
{
  char       string[9];
#if defined(DEBUG_EXTRA)
  FILE *logfile;

  logfile = fopen("c:\\temp\\threadtable.log","a");
#endif

  sprintf(string,"%08x",GetCurrentThreadId());
  // exclusive access to this region
  WaitForSingleObject(mutex,INFINITE);

  thread2EngineList->DropItem(thread2EngineList->FindItem(string));
#if defined(DEBUG_EXTRA)
  {
    int i = 0;
    ListItem *item;
    FPRINTF(logfile,"deregister\nthread<->engine association (we were %s):\n",string);
    while ( item = thread2EngineList->FindItem(i++) )
      FPRINTF2(logfile,"%2d %s %p\n",i,item->GetName(),item->GetContent());
    FPRINTF2(logfile,"list ends\n");
  }
  fclose(logfile);
#endif
  ReleaseMutex(mutex);
}


/* this function looks for the engine that is associated with the */
/* given thread id. on failure it returns NULL.                   */
/* the list used to find this information is process-global.      */
OrxScript* findEngineForThread(DWORD id)
{
  char       string[9];
  ListItem  *result = NULL;
  OrxScript *engine = NULL;

  sprintf(string,"%08x",id);

  // exclusive access to this region
  WaitForSingleObject(mutex,INFINITE);
  result = thread2EngineList->FindItem(string);

  if (result)
    engine = (OrxScript*) result->GetContent();

  ReleaseMutex(mutex);

  return engine;
}

int __stdcall scriptSecurity(CLSID clsid, IUnknown *pObject)
{
  int result = 1; // assume OK
  OrxScript  *engine = findEngineForThread(GetCurrentThreadId());

  // have an engine?
  if (engine) {
    HRESULT hResult;
    DWORD dwPolicy;
    IInternetHostSecurityManager *pSecurityManager;

    // script host object should be considered safe at all time (acc. to Joel Alley, MS)
    // this is achieved by turning of the security check for objects that
    // have been added by the script host and will be instantiated by the
    // script at the time of need. a flag is used to find out if a check
    // has to be made
    if (engine->checkObjectCreation() == false) return result;

    pSecurityManager = engine->getIESecurityManager();

    // found a security manager?
    if (pSecurityManager) {
      // only clsid given? check if we're allowed to create this
      if (pObject == NULL) {
        hResult = pSecurityManager->ProcessUrlAction(URLACTION_ACTIVEX_RUN,
                                                     (BYTE*) &dwPolicy,
                                                     sizeof(dwPolicy),
                                                     (BYTE*) &clsid,
                                                     sizeof(clsid),
                                                     PUAF_DEFAULT,
                                                     0);
        if (SUCCEEDED(hResult)) {
          if (dwPolicy != URLPOLICY_ALLOW)
            result = 0; // not allowed to create
        } else
          result = 0;   // not allowed to create
      } else {
        // an object was given, check if it is safe to run it
        DWORD        *pdwPolicy;
        DWORD         cbPolicy;
        CONFIRMSAFETY csafe;

        csafe.pUnk  = pObject;
        csafe.clsid = clsid;
        csafe.dwFlags = CONFIRMSAFETYACTION_LOADOBJECT;

        hResult = pSecurityManager->QueryCustomPolicy(GUID_CUSTOM_CONFIRMOBJECTSAFETY,
                                                      (BYTE**) &pdwPolicy,
                                                      &cbPolicy,
                                                      (BYTE*) &csafe,
                                                      sizeof(csafe),
                                                      0);
        if (SUCCEEDED(hResult)) {
          dwPolicy = URLPOLICY_DISALLOW;
          if (pdwPolicy) {
            if (sizeof(DWORD) <= cbPolicy)
              dwPolicy = *pdwPolicy;
            CoTaskMemFree(pdwPolicy);
          }
          if (dwPolicy != URLPOLICY_ALLOW)
            result = 0; // not allowed to run
        } else
          result = 0;   // not allowed to run
      }
    }
  }
  return result;
}

/******************************************************************************
*
*  callback for VALUE
*  this is used set and retrieve engine properties with the VALUE func.
*
*  Note that we go through WhoKnows() to find/set these, and do not
*  check the PropertyList ourselves.  By going through WhoKnows(),
*  we let the WSH name precedence resolve it.  If it is one of ours
*  it will be handled by OrxIDispatch, just as if JScript called.
*
******************************************************************************/
RexxObject* __stdcall propertyChange(RexxString* name,RexxObject* newvalue,int SelectorType,int *RetCode)
{
  RexxObject *result = NULL;
  OrxScript  *engine = findEngineForThread(GetCurrentThreadId());
  char       *PropName=NULL;
  char        tBuffer[1024];
  HRESULT     hResult;
  DISPID      dispID;
  DWORD       flags;
  IDispatch  *pDispatch = NULL;
  ITypeInfo  *pTypeInfo = NULL;
  VARIANT    *Property;

  *RetCode = -1;        // No name found.
  //  First, validate parameters.  Second, copy the name into a useable area.
  // _asm int 3
  if(!name) {                     // If there is no name, error out.
    *RetCode = 1;
    return result;
    }
  if(name->stringData == NULL  || name->length == 0) {
    *RetCode = 2;
    return result;
    }

  if((name->length + 1) <= sizeof(tBuffer)) PropName = &tBuffer[0];
  else if(!(PropName = (char *)malloc(name->length+1))) {
    *RetCode = 3;
    return result;
    }
  memcpy(PropName,name->getStringData(),name->getLength());
  PropName[name->length] = '\0';

  // if we set a new value, we must return the old one...
  // Either way, first find the old one.

  // do we have an engine to work with?
  if (engine) {
    switch (SelectorType) {
    case 1:
      Property = engine->GetExternalProperty(PropName);
      if(PropName != &tBuffer[0]) free(PropName);
      if(Property) {
        // GET
        WinEnterKernel(false);
        result = Variant2Rexx(Property);
        WinLeaveKernel(false);
        // PUT
        if(newvalue) {
          VariantClear(Property);
          WinEnterKernel(false);
          //  >>> ??? <<< Does this return something that I can check for failure?
          Rexx2Variant(newvalue, Property, VT_EMPTY /*(try your best)*/, 0 /*dummy argument?!*/);
          WinLeaveKernel(false);
          }
        *RetCode = 0;
        }
      else {
        *RetCode = 0;
        WinEnterKernel(false);
        result = RexxString("");
        WinLeaveKernel(false);
        }
      break;
    case 2:
      hResult = engine->getNamedItems()->WhoKnows(PropName,engine->Lang,&dispID,&flags,&pDispatch,&pTypeInfo);
      if(PropName != &tBuffer[0]) free(PropName);
      if (SUCCEEDED(hResult)) {
        // do we have a real named item?
        if (dispID == -1) {
          //  >>> ??? <<<   This is nasty.
          //  >>> ??? <<<   error condition 1.
          *RetCode = 0;
          }
        else {
          // we have a property

          if (pDispatch == NULL) {
            // This property is part of a Typelib.
            TYPEATTR *pTypeAttr;
            VARDESC  *pVarDesc;
            BOOL      found = false;


            if(newvalue) {
              *RetCode = 4;        // For now, Typelib's are immutable.
              return NULL;
            }
            hResult = pTypeInfo->GetTypeAttr(&pTypeAttr);
            if (SUCCEEDED(hResult)) {
              for (int i=0; i<pTypeAttr->cVars && !found; i++) {
                hResult = pTypeInfo->GetVarDesc(i,&pVarDesc);
                if (SUCCEEDED(hResult)) {
                  // found the variable?
                  if (pVarDesc->memid == dispID) {
                    found = true;
                    *RetCode = 6;      // Got a variant, just not a supported one.
                    switch (pVarDesc->varkind) {
                      case VAR_STATIC:
                        break;   // can this be treated like VAR_CONST, too?
                      case VAR_CONST:
                        WinEnterKernel(false);
                        result = Variant2Rexx(pVarDesc->lpvarValue);
                        WinLeaveKernel(false);
                        *RetCode = 0;
                        break;
                      // don't know what to do with these two:
                      case VAR_PERINSTANCE:
                      case VAR_DISPATCH:
                        break;
                    }
                  }

                  pTypeInfo->ReleaseVarDesc(pVarDesc);
                }
              }

              pTypeInfo->ReleaseTypeAttr(pTypeAttr);
            }
          }
        }  //  if (dispID == -1)
      }  //  if (SUCCEEDED(hResult))

      if (result == NULL) {
        WinEnterKernel(false);
        result = RexxString("");
        WinLeaveKernel(false);
      }
      break;
    case 3:
      result = RexxNil;
      // get the named items of the engine
      // a newvalue must not exist since this is read-only!
      if (newvalue == NULL) {
        // a name must be given
        if (name) {
          // it must be "NAMEDITEMS"
          if (!strcmp(name->stringData, "NAMEDITEMS")) {
            // retrieve the NamedItem list of the engine
            OrxNamedItem* pItemList = engine->getNamedItems();
            if (pItemList) {
              int num = 0;
              // return a char* array with the names
              // num contains the number of strings
              char **names = pItemList->getNamedItems(&num);
              SAFEARRAY      *pSafeArray = NULL;
              SAFEARRAYBOUND  ArrayBound;
              VARIANT        *pVarArray = (VARIANT*) malloc(sizeof(VARIANT));
              VARIANT         sVariant;
              OLECHAR         uniBuffer[128];

              // we cannot directly use REXX to create objects
              // so we have to first create a OLE Array Variant
              // and let this be converted by the OREXXOLE.C functions
              if (num > 0) {
                ArrayBound.cElements = num;
                ArrayBound.lLbound = 0;   // zero-based
                pSafeArray = SafeArrayCreate(VT_VARIANT,(UINT) 1, &ArrayBound);
                VariantInit(pVarArray);
                V_VT(pVarArray) = VT_ARRAY | VT_VARIANT;
                V_ARRAY(pVarArray) = pSafeArray;

                for (long i = 0; i < num; i++) {
                  // convert to unicode
                  C2W(uniBuffer, names[i], 128);
                  // create a BSTR variant
                  VariantInit(&sVariant);
                  V_VT(&sVariant) = VT_BSTR;
                  V_BSTR(&sVariant) = SysAllocString(uniBuffer);
                  SafeArrayPutElement(pSafeArray, &i, &sVariant);
                  // release the char* from the string array
                  free(names[i]);
                }
              }
              // release the string array
              free(names);
              // create a REXX array
              WinEnterKernel(false);
              result = Variant2Rexx(pVarArray);
              WinLeaveKernel(false);
              *RetCode = 0;
            }
          }
        }
      } else {
        *RetCode = 4; // can only read the info!
      }

      break;
    default:
      *RetCode = -2;    // Bad SelectorType
      break;
    }   //  switch(SelectorType)
  }   //  if (engine)
  else ;  // >>> ??? <<< Good question, what do we do? Error, or return NULL string?
    // result = RexxString("");

  return result;
}

/* unknown callback                                                     */
/* this will deal with objects that REXX is unaware of, but the engine  */
/* is...                                                                */
RexxObject* __stdcall engineDispatch(void *arguments)
{
  char       *objname = (char*) arguments;
  RexxObject *result = NULL; // NULL indicates error
  OrxScript  *engine = findEngineForThread(GetCurrentThreadId());
  HRESULT    hResult;
  DISPID     dispID;
  DWORD      flags;
  IDispatch *pDispatch = NULL;
  ITypeInfo *pTypeInfo = NULL;
  VARIANT    temp;

  // do we have an engine to work with?
  if (engine) {
    engine->setObjectCreation(false); // turn off IE security manager checking
    engine->PreventCallBack();        // Don't pick up properties.  This stops our GetDispID() from returning DispID's for properties.
    hResult = engine->getNamedItems()->WhoKnows(objname,engine->Lang,&dispID,&flags,&pDispatch,&pTypeInfo);
    engine->ReleaseCallBack();
    if (SUCCEEDED(hResult)) {
      VariantInit(&temp);
      // do we have a real named item?
      if (dispID == -1) {

        V_VT(&temp) = VT_DISPATCH;
        temp.pdispVal = pDispatch;
        // convert it to an OLEObject
        // because the top activation on the activity might not be
        // "native", we have to make sure that it is
        // (Win...Kernel() must never be executed concurrently!)
        WinEnterKernel(false);           // we only need an activation, no activity
        result = Variant2Rexx(&temp);
        WinLeaveKernel(false);
        // this must be a variable / constant
      } else {
        if(pDispatch == engine) {
          hResult = engine->LocalParseProcedureText(objname,0,&pDispatch);
          V_VT(&temp) = VT_DISPATCH;
          temp.pdispVal = pDispatch;
          }
        else hResult = GetProperty(NULL,pDispatch,engine->Lang,&temp,dispID);
        if (SUCCEEDED(hResult)) {
          WinEnterKernel(false);
          result = Variant2Rexx(&temp);
          WinLeaveKernel(false);
        }
        VariantClear(&temp);
      }
    }
    engine->setObjectCreation(true); // turn on possible IE security manager checking

  } /* endif engine to work with */
  return result;
}

/* here we "parse" the text and retrieve the names of all */
/* (public) routines mentioned in there.                  */
/* the name retrieval is done with the exit handler that  */
/* will call back into the engine to add the names...     */
/* parameters: 0 - script text in wide charachters (in)   */
/*             1 - pointer to script engine (in)          */
/*             2 - return code (out)                      */
// this is running in a thread of its own...
void __stdcall parseText(void *arguments)
{
  // get the info that was passed to us
  LPCOLESTR   pStrCode = ((LPCOLESTR*) arguments)[0]; // script text
  LinkedList *pList = ((LinkedList**) arguments)[1];  // pointer to list for names
  int        *result = ((int**) arguments)[2];

  // "normal" local variables
  RXSTRING instore[2];
  RXSYSEXIT exit_list[9];
  RXSTRING  rexxretval;
  APIRET    rc;
  SHORT     rexxrc = 0;

  char funcHandler[128];
  char *script = NULL;

#if defined(DEBUGC)+defined(DEBUGZ)
  static int parsecount = 0;
  char fname[64];
  FILE *logfile;

  sprintf(fname,"c:\\temp\\parsetext%d.log",++parsecount);
  logfile = fopen(fname,"wb");
#endif
  // build "our" version of the script text that does "nothing"...
  sprintf(funcHandler,"LISTOFNAMES='%p'; exit;",pList);
  script = (char*) malloc(sizeof(char)*(1+wcslen(pStrCode)+strlen(funcHandler)));
  sprintf(script,"%s%S",funcHandler,pStrCode);

  // by setting the strlength of the output RXSTRING to zero, we
  // force the interpreter to allocate memory and return it to us
  rexxretval.strptr = NULL;
  rexxretval.strlength = 0;

  // provide script in instore mode, we will simply discard the
  // resulting image...
  MAKERXSTRING(instore[0], script, strlen(script));
  instore[1].strptr = NULL;
  instore[1].strlength = 0;


  // initialize exit list
  exit_list[0].sysexit_name = "RexxCatchExit";
  exit_list[0].sysexit_code = RXTER;
  exit_list[1].sysexit_code = RXENDLST;
#if defined(DEBUGZ)
  FPRINTF(logfile,"RexxStart()\n");
#endif
  // run this
  rc = RexxStart( (LONG)       0,                     // no arguments
                  (PRXSTRING)  NULL,                  // the argument pointer
                  (PSZ)        "Rexx/ParseText",      // name of script
                  (PRXSTRING)  instore,               // instore parameters
                  (PSZ)        NULL,                  // command environment name
                  (LONG)       RXSUBROUTINE,          // invoke as COMMAND
                  (PRXSYSEXIT) exit_list,             // exit list (catch exit only)
                  (PSHORT)     &rexxrc,               // rexx result code
                  (PRXSTRING)  &rexxretval);          // rexx program result
#if defined(DEBUGZ)
  FPRINTF2(logfile,"done with RexxStart (RC = %d)\n",rexxrc);
#endif
  *result = (int) rexxrc;

  // during exits in REXX the exit handler will add elements to
  // an engine-specific list...
  //RexxWaitForTermination();   //hm... maybe this will not return until all paired rexxinitialize() rexxterminate()'s are thru (end of engine...!!)
  RexxDidRexxTerminate();
//  FPRINTF(logfile,"done with RexxWaitForTermination\n");

  // we do not need any return value...
  if (rexxretval.strptr) GlobalFree(rexxretval.strptr);

  // we throw away the image, we need a special one anyway...
  if (instore[1].strptr) GlobalFree(instore[1].strptr);

  free(script);

#if defined(DEBUGC)+defined(DEBUGZ)
  FPRINTF(logfile,"parseText ends\n");
  fclose(logfile);
#endif

  _endthreadex(0);
}

/* this creates a (flattened) method from the source    */
/* that is passed to us.                                */
/* a "handler" is prepended to allow for execution of   */
/* the immediate code (once) and calls for functions    */
/* inside this method (many times). this is done with   */
/* an interpret statement.                              */
/* parameters: 0 - script text in wide charachters (in) */
/*             1 - pointer to script engine (in)        */
/*             2 - pointer to RXSTRING for result image */
/*                 (out)                                */
void __stdcall createCode(void *arguments)
{
  // get the info that was passed to us
  LPCOLESTR  pStrCode = ((LPCOLESTR*) arguments)[0]; // script text
  OrxScript *pEngine = ((OrxScript**) arguments)[1]; // pointer to engine
  RexxObject  **pImage = ((RexxObject***) arguments)[2];   // result object (a string containing the image)
  ConditionData *condData = ((ConditionData**) arguments)[3]; // condition info

  // "normal" local variables
  char        funcHandler[128];
  char       *script = NULL;
  RXSTRING    source;
  APIRET      rc;


#if defined(DEBUGC)+defined(DEBUGZ)
  static int codecount = 0;
  char fname[64];
  FILE *logfile;

  sprintf(fname,"c:\\temp\\createcode%d.log",++codecount);
  logfile = fopen(fname,"wb");
  FPRINTF(logfile,"createCode() started\n");
  FPRINTF(logfile,"code:\n%S\n",pStrCode);
#endif

  // build "our" version of the script text that does allows us "dispatch" function
  // calls and to run "immediate" code
  sprintf(funcHandler,"if arg(1) \\='' then interpret arg(1)\n");
  script = (char*) malloc(sizeof(char)*(1+wcslen(pStrCode)+strlen(funcHandler)));
  sprintf(script,"%s%S",funcHandler,pStrCode);

  MAKERXSTRING(source,script,strlen(script));

  condData->rc = 0;  // clear to ok
  rc = RexxCreateMethod(pEngine->getEngineName(),&source,pImage,condData);
  if (condData->rc == 0) {
#if defined(DEBUGZ)
    FPRINTF2(logfile,"RexxCreateMethod success\n");
#endif
    // we created the method, now "flatten" it to store it in memory (NOT REXX memory, btw)
    // when this thread has finished, the caller can use the pointer we set here
    //rc = RexxStoreMethod(pMethod,pImage);
#if defined(DEBUGZ)
    if (rc)
      FPRINTF2(logfile,"RexxStoreMethod() failed\n");
#endif
  }
  else {
#if defined(DEBUGZ)
    FPRINTF2(logfile,"RexxCreateMethod with %s failed: %s\n",script,condData->errortext.strptr);
#endif
  }

  free(script);

#if defined(DEBUGC)+defined(DEBUGZ)
  FPRINTF(logfile,"stored in RXSTRING at %p\ncreateCode ends\n",pImage);
  fclose(logfile);
#endif

  _endthreadex(0);
}



RexxObject *Create_securityObject(OrxScript  *pEngine,
                                  FILE       *logfile)
{
  RexxObject *securityObject = NULL;
#define DEBUGC
#define DEBUGZ
    FPRINTF2(logfile,"Create_securityObject - start Engine %p \n",pEngine);


  /* create an instance of a security manager object */
  if (pEngine->getSafetyOptions()) {
    DISPPARAMS dp;
    VARIANT temp;
    OLECHAR invokeString[32];
    char args[32];
    ConditionData condData; // condition info
    OrxScriptError *ErrObj;
    bool        ErrObj_Exists;
    APIRET      rc;
    HRESULT     hResult=S_OK;


    memset((void*) &condData,0,sizeof(ConditionData));
    sprintf(args,"FLAGS=%d",pEngine->getSafetyOptions());
    C2W(invokeString,args,32);
    VariantInit(&temp);
    V_VT(&temp) = VT_BSTR;
    V_BSTR(&temp) = SysAllocString(invokeString);
    dp.cArgs = 1;
    dp.cNamedArgs = 0;
    dp.rgdispidNamedArgs = NULL;
    dp.rgvarg = &temp;
    FPRINTF2(logfile,"Create_securityObject - About to use SecurityManager %p\n",pEngine->getSecurityManager());
#if defined(DEBUGZ)
    FPRINTF2(logfile,"Create_securityObject - About to use SecurityManager %p\n",pEngine->getSecurityManager());
#endif
    rc = RexxRunMethod(pEngine->getEngineName(),pEngine->getSecurityManager(),&dp,DispParms2RexxArray,NULL,&securityObject,NULL,&condData);
    VariantClear(&temp);
    if (condData.rc) {
      // an error occured: init excep info
#if defined(DEBUGZ)
      ErrObj = new OrxScriptError(logfile,&condData,&ErrObj_Exists);
#else
      ErrObj = new OrxScriptError(NULL,&condData,&ErrObj_Exists);
#endif
      hResult = pEngine->getScriptSitePtr()->OnScriptError((IActiveScriptError*) ErrObj);
#if defined(DEBUGZ)
      FPRINTF2(logfile,"Create_securityObject - Error encountered: OnScriptError returned %08x.\n",hResult);
#endif
      if (FAILED(hResult)) {
#if defined(DEBUGZ)
        FPRINTF2(logfile,"Create_securityObject -  Something is unexpected.\n");
#endif
        }
      // init to empty again....
      if(ErrObj_Exists) ErrObj->UDRelease();
    }  // if(condData->rc)
    else {
#if defined(DEBUGZ)
      FPRINTF2(logfile,"Create_securityObject - Success created secturityObject: %p.\n",securityObject);
#endif
      }
    }
  else securityObject = NULL;
#undef DEBUGC
#undef DEBUGZ

  return securityObject;
  }



/* this runs a previously created method       */
/* parameters: 0 - pointer to engine           */
/*             1 - ptr to image of meth. block */
/*             2 - COM parameters, if needed   */
/*             2 - REXX parameters, if needed  */
/*             3 - REXX result object          */
/*             4 - VARIANT result              */
/*             5 - ConditionData pointer       */
/*             6 - end thread?                 */
/*             7 - get variables?              */
void __stdcall runMethod(void *arguments)
{
  // get the info that was passed to us
  OrxScript  *pEngine = ((OrxScript**) arguments)[0]; // pointer to engine
  RCB        *RexxCode = ((PRCB *) arguments)[1];   // method image
  DISPPARAMS *parms = ((DISPPARAMS**) arguments)[2];  // COM parameters   (...either...)
  RexxArray  *args = ((RexxArray**) arguments)[3];    // REXX parameters  (.....or.....)
  RexxObject **pTargetResult = ((RexxObject***) arguments)[4]; // result object
  VARIANT    **vResult = ((VARIANT***) arguments)[4]; // result variant
  ConditionData *condData = ((ConditionData**) arguments)[5]; // condition info
  bool       fEndThread = (bool) ((int*)arguments)[6];// end this thread?
  bool       fGetVariables = (bool) ((int*)arguments)[7]; // get variables from immediate code?

  RexxObject *pResult = NULL;
  RexxObject *pMethod = NULL;
  RexxObject *securitySource = NULL;
  APIRET      rc;
  RXSYSEXIT   exit_list[9];
  HRESULT     hResult=S_OK;
  OrxScriptError *ErrObj;
  bool        ErrObj_Exists;


#if defined(DEBUGC)+defined(DEBUGZ)
  static int runcount = 0;
  char fname[64];
  FILE *logfile;

  sprintf(fname,"c:\\temp\\runmethod%d.log",++runcount);
  logfile = fopen(fname,"wb");
  if (fEndThread) {
    // needed? hResult = CoInitializeEx(NULL,COINIT_MULTITHREADED);
    //if ( hResult != S_OK) _asm int 3
  }
  FPRINTF(logfile,"RexxLoadMethod %p\n",RexxCode);
#endif
    memset((void*) condData,0,sizeof(ConditionData));

    // thread dependend registration of the engine
    registerEngineForCallback(pEngine);

    // initialize exit list
    exit_list[0].sysexit_name = "RexxCatchExternalFunc";
    exit_list[0].sysexit_code = RXEXF;
    exit_list[1].sysexit_code = RXENDLST;

    if (fGetVariables) {
      //_asm int 3
      exit_list[1].sysexit_name = "RexxRetrieveVariables";
      exit_list[1].sysexit_code = RXTER;
      exit_list[2].sysexit_code = RXENDLST;
    }
#if defined(DEBUGZ)
    FPRINTF2(CurrentObj_logfile,"runMethod about to execute %p from CodeBlock %p \n",RexxCode->Code,RexxCode);
#endif

    // if this was invoked with DISPPARMs, we need to convert the arguments,
    // plus the return object must be converted to a VARIANT
    if (parms) {
      rc = RexxRunMethod(pEngine->getEngineName(),RexxCode->Code,parms,DispParms2RexxArray,exit_list,&pResult,pEngine->getSecurityObject(),condData);
      // successful execution?
      if (condData->rc == 0) {
        // _asm int 3
        WinEnterKernel(true);
        Rexx2Variant(pResult, *vResult, VT_EMPTY /*(try your best)*/, 0 /*dummy argument?!*/);
        WinLeaveKernel(true);
        if (V_VT(*vResult) == VT_ERROR)
          (*vResult)->vt = VT_EMPTY;
      }
    }
    else
      rc = RexxRunMethod(pEngine->getEngineName(),RexxCode->Code,args,NULL,exit_list,pTargetResult,pEngine->getSecurityObject(),condData);

    condData->position += RexxCode->StartingLN;

    // remove engine from thread list
    deregisterEngineForCallback();
#if defined(DEBUGZ)
    FPRINTF2(logfile,"done (%d %d)\n",rc,condData->rc);
#endif

    if (condData->rc) {
      // an error occured: init excep info
#if defined(DEBUGZ)
      ErrObj = new OrxScriptError(logfile,condData,&ErrObj_Exists);
#else
      ErrObj = new OrxScriptError(NULL,condData,&ErrObj_Exists);
#endif
      hResult = pEngine->getScriptSitePtr()->OnScriptError((IActiveScriptError*) ErrObj);
#if defined(DEBUGZ)
      FPRINTF2(logfile,"debug: OnScriptError returned %08x.\n",hResult);
#endif
      if (FAILED(hResult)) {
#if defined(DEBUGZ)
        FPRINTF2(logfile,"debug:  Something is unexpected.\n");
#endif
      }
      // init to empty again....
      if(ErrObj_Exists) ErrObj->UDRelease();
    }
    else {
#if defined(DEBUGZ)
      FPRINTF2(logfile,"RexxRunMethod success. Returned object: %p.\n",pResult);
#endif
    }

#if defined(DEBUGC)+defined(DEBUGZ)
  FPRINTF(logfile,"runMethod ends\n");
  fclose(logfile);
#endif

  if (fEndThread) {
    // needed? CoUninitialize();
    _endthreadex(0);
  }

}
