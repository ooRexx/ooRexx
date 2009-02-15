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


/* Exit handler to find out the values of variables of "immediate code" */
int RexxEntry RexxRetrieveVariables(RexxExitContext *, int ExitNumber, int Subfunction, PEXIT parmblock)
{
    OrxScript *engine = ScriptProcessEngine::findEngineForThread();

    // if we're capturing variable values on exit, grab the entire set now
    if (engine && enginethis->enableVariableCapture)
    {
        // request all of the context variables
        RexxSupplierObject supplier = context->GetAllContextVariables();

        // now iterate through the directory, saving the list of routines (and the names).
        while (context->SupplierAvailable(supplier))
        {
            RexxObjectPtr name = context->SupplierIndex(supplier);
            RexxObjectPtr value = context->SupplierItem(supplier);
            context->RequestGlobalReference(value);
            CurrentEngine->insertVariable(context->ObjectToStringValue(name), value);
        }
    }

    // always return with NOT_HANDLED...
    return RXEXIT_NOT_HANDLED;
}


/* Exit handler for external function calls */
int RexxEntry RexxCatchExternalFunc(RexxExitContext *context, int ExitNumber, int Subfunction, PEXIT parmblock)
{
    RXEXFCAL_PARM *parmblock = (RXEXFCAL_PARM*) pblock;
    RexxObjectPtr     result    = ooRexxNil;
    const char    *fncname   = parmblock->rxfnc_name.strptr;
    OrxScript *engine = ScriptProcessEngine::findEngineForThread();
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
    FILE *logfile = NULL;

    // have an engine to work with?
    if (engine)
    {
        // get the logfile for debugging purposes
        logfile = engine->getLogFile();

        FPRINTF(logfile,"using engine %s, looking for %s\n",engine->getEngineName(),fncname);
        // is the function we're looking for a REXX method that was defined for this
        // script engine earlier?
        RCB *pImage = engine->findRexxFunction(fncname);
        if (pImage)
        {
            // the function that needs to be called is a REXX method NOT defined
            // in the current REXX script scope

            // argument array for runMethod
            LPVOID    arguments[8];
            char      invString[256];
            char      buffer[8];
            RexxConditionData cd;

            // build argument array of REXX objects...
            RexxArrayObjectPtr args = context->NewArray(parmblock->rxfnc_argc);

            for (size_t i=0; i < parmblock->rxfnc_argc; i++)
            {
                RexxObjectPtr temp = parmblock->rxfnc_argv[i];
                context->ArrayPut(args, temp, i + 1);
            }

            runMethod(context->threadContext, engine, pImage, args, result, cd);
            state = RXEXIT_HANDLED;
            parmblock->rxfnc_retc = (RexxObjectPtr)result;
            FPRINTF2(logfile,"invocation result %p (rc = %d)\n",result,cd.rc);
            // if something went wrong, indicate an error to REXX
            if (cd.rc != 0)
            {
                // indicate error in parmblock...
                parmblock->rxfnc_flags.rxfferr = 1;
                // we don't need to notify the engine of the error
                // here, because this was already done by the part
                // of the code that invoked the REXX codeblock
            }
        }
        else
        {
            // ask the named items if one of them knows this function
            hResult = engine->getNamedItems()->WhoKnows(parmblock->rxfnc_name.strptr, engine->Lang,&dispID,&flags,&pDispatch,&pTypeInfo);
            if (hResult == S_OK)
            {
                // found something...
                // build DISPPARAMS structure for the invoke
                dp.cNamedArgs = 0;
                dp.cArgs = (UINT)parmblock->rxfnc_argc;
                VariantInit(&sResult);
                pResult = &sResult;

                // do we have to build an argument array?
                if (parmblock->rxfnc_argc > 0)
                {
                    VARIANTARG *pVarArgs = (VARIANTARG*) GlobalAlloc(GMEM_FIXED,(sizeof(VARIANTARG) * dp.cArgs));

                    dp.rgvarg = pVarArgs;
                    for (UINT i = 0; i < dp.cArgs; i++)
                    {
                        // get the REXX object
                        RexxObjectPtr =temp = parmblock->rxfnc_argv[i];

                        // arguments must be filled in from the end of the array...
                        VariantInit(&(pVarArgs[dp.cArgs - i - 1]));
                        Rexx2Variant(context->threadContext, temp, &(pVarArgs[dp.cArgs - i - 1]), VT_EMPTY /*(try your best)*/, 0 /*dummy argument?!*/);
                    }
                }
                else
                {
                    dp.rgvarg = NULL; // no argument array needed
                }

                if (dispID != -1)
                {
                    hResult = pDispatch->Invoke(dispID, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dp, pResult, &sExc, &uArgErr);
                }
                else
                {
                    // call the default method of a named item object
                    unsigned short wFlags = DISPATCH_METHOD;
                    DISPID         PropPutDispId = DISPID_PROPERTYPUT;

                    pTypeInfo = NULL;
                    hResult = pDispatch->GetTypeInfo(0, LOCALE_USER_DEFAULT, &pTypeInfo);

                    if (SUCCEEDED(hResult))
                    {
                        TYPEATTR *pTypeAttr = NULL;
                        FUNCDESC *pFuncDesc = NULL;
                        bool      found = false;

                        hResult = pTypeInfo->GetTypeAttr(&pTypeAttr);
                        if (SUCCEEDED(hResult))
                        {
                            for (int l = 0; (l < pTypeAttr->cFuncs) && !found; l++)
                            {
                                hResult = pTypeInfo->GetFuncDesc(l, &pFuncDesc);
                                if (SUCCEEDED(hResult))
                                {
                                    if ((pFuncDesc->cParams + pFuncDesc->cParamsOpt >= parmblock->rxfnc_argc) &&
                                        pFuncDesc->memid == 0)
                                    {
                                        wFlags = pFuncDesc->invkind;
                                        found = true;
                                    }
                                    pTypeInfo->ReleaseFuncDesc(pFuncDesc);
                                }
                            }

                            pTypeInfo->ReleaseTypeAttr(pTypeAttr);
                            pTypeInfo->Release();
                        }
                        if (wFlags == DISPATCH_PROPERTYPUT)
                        {
                            dp.cNamedArgs = 1;
                            dp.rgdispidNamedArgs = &PropPutDispId;
                        }
                    }

                    hResult = pDispatch->Invoke((DISPID) 0, IID_NULL, LOCALE_USER_DEFAULT, wFlags, &dp, pResult, &sExc, &uArgErr);
                }

                if (SUCCEEDED(hResult))
                {
                    // success, make REXX object from VARIANT

                    // TODO note that Variant2Rexx() can / will return NULLOBJECT if it raises an
                    // exception.
                    parmblock->rxfnc_retc = Variant2Rexx(context->threadContext, &sResult);
                    state = RXEXIT_HANDLED;
                    FPRINTF2(logfile,"COM invoke ok, got back rexx object %p\n", parmblock->rxfnc_retc);
                }
                else
                {
                    result = NULL; // this is an error. behave accordingly
                    parmblock->rxfnc_flags.rxfferr = 1;
                }

                // clear argument array, free memory:
                if (dp.rgvarg)
                {
                    for (UINT i = 0; i < dp.cArgs; i++)
                    {
                        VariantClear(&(dp.rgvarg[i]));
                    }
                    GlobalFree(dp.rgvarg);
                }

                // free result
                VariantClear(&sResult);
            }
        }
        FPRINTF(logfile,"I %s this function - exiting now.\n",state==RXEXIT_HANDLED?"knew":"didn't know");
        return state;
    }

}


int __stdcall scriptSecurity(CLSID clsid, IUnknown *pObject)
{
    int result = 1; // assume OK
    OrxScript *engine = ScriptProcessEngine::findEngineForThread();

    // have an engine?
    if (engine)
    {
        HRESULT hResult;
        DWORD dwPolicy;
        IInternetHostSecurityManager *pSecurityManager;

        // script host object should be considered safe at all time (acc. to Joel Alley, MS)
        // this is achieved by turning of the security check for objects that
        // have been added by the script host and will be instantiated by the
        // script at the time of need. a flag is used to find out if a check
        // has to be made
        if (engine->checkObjectCreation() == false)
        {
            return result;
        }

        pSecurityManager = engine->getIESecurityManager();

        // found a security manager?
        if (pSecurityManager)
        {
            // only clsid given? check if we're allowed to create this
            if (pObject == NULL)
            {
                hResult = pSecurityManager->ProcessUrlAction(URLACTION_ACTIVEX_RUN,
                                                             (BYTE*) &dwPolicy,
                                                             sizeof(dwPolicy),
                                                             (BYTE*) &clsid,
                                                             sizeof(clsid),
                                                             PUAF_DEFAULT,
                                                             0);
                if (SUCCEEDED(hResult))
                {
                    if (dwPolicy != URLPOLICY_ALLOW)
                    {
                        result = 0; // not allowed to create
                    }
                }
                else
                {
                    result = 0;   // not allowed to create
                }
            }
            else
            {
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
                if (SUCCEEDED(hResult))
                {
                    dwPolicy = URLPOLICY_DISALLOW;
                    if (pdwPolicy)
                    {
                        if (sizeof(DWORD) <= cbPolicy)
                        {
                            dwPolicy = *pdwPolicy;
                        }
                        CoTaskMemFree(pdwPolicy);
                    }
                    if (dwPolicy != URLPOLICY_ALLOW)
                    {
                        result = 0; // not allowed to run
                    }
                }
                else
                {
                    result = 0;   // not allowed to run
                }
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
int RexxEntry RexxValueExtension(RexxExitContext *context, int ExitNumber, int Subfunction, PEXIT parmblock)
{
    RXVALCALL_PARM *parmblock = (RXVALCALL_PARM*) pblock;
    int SelectorType = 0;
    int RetCode = -1;
    const char *selectorName = parmblock->selector.strptr;
    RexxObjectPtr  result = NULL;
    OrxScript *engine = ScriptProcessEngine::findEngineForThread();
    const char *PropName = parmblock->variable_name.strptr;
    RexxObjectPtr  newvalue = parmblock->value;
    HRESULT     hResult;
    DISPID      dispID;
    DWORD       flags;
    IDispatch  *pDispatch = NULL;
    ITypeInfo  *pTypeInfo = NULL;
    VARIANT    *Property;

    if (strcmp(selectorName, "WSHPROPERTY") == 0)
    {
        SelectorType = 1;
    }
    else if (strcmp(selectorName, "WSHTYPELIB") == 0)
    {
        SelectorType = 2;
    }
    else if (strcmp(selectorName, "WSHENGINE") == 0)
    {
        SelectorType = 3;
    }
    else
    {
        return RXEXIT_NOT_HANDLED;     // unknown selector type
    }

    // if we set a new value, we must return the old one...
    // Either way, first find the old one.

    // do we have an engine to work with?
    if (engine)
    {
        switch (SelectorType)
        {
            case 1:
                Property = engine->GetExternalProperty(PropName);
                if (Property)
                {
                    result = Variant2Rexx(Property);
                    // PUT
                    if (newvalue)
                    {
                        VariantClear(Property);
                        Rexx2Variant(newvalue, Property, VT_EMPTY /*(try your best)*/, 0 /*dummy argument?!*/);
                    }
                    RetCode = 0;
                }
                else
                {
                    RetCode = 0;
                    result = ooRexxString("");
                }
                break;
            case 2:
                hResult = engine->getNamedItems()->WhoKnows(PropName,engine->Lang,&dispID,&flags,&pDispatch,&pTypeInfo);
                if (SUCCEEDED(hResult))
                {
                    // do we have a real named item?
                    if (dispID == -1)
                    {
                        //  >>> ??? <<<   This is nasty.
                        //  >>> ??? <<<   error condition 1.
                        RetCode = 0;
                    }
                    else
                    {
                        // we have a property

                        if (pDispatch == NULL)
                        {
                            // This property is part of a Typelib.
                            TYPEATTR *pTypeAttr;
                            VARDESC  *pVarDesc;
                            bool      found = false;


                            if (newvalue)
                            {
                                RetCode = 4;        // For now, Typelib's are immutable.
                                return NULL;
                            }
                            hResult = pTypeInfo->GetTypeAttr(&pTypeAttr);
                            if (SUCCEEDED(hResult))
                            {
                                for (int i=0; i<pTypeAttr->cVars && !found; i++)
                                {
                                    hResult = pTypeInfo->GetVarDesc(i,&pVarDesc);
                                    if (SUCCEEDED(hResult))
                                    {
                                        // found the variable?
                                        if (pVarDesc->memid == dispID)
                                        {
                                            found = true;
                                            RetCode = 6;      // Got a variant, just not a supported one.
                                            switch (pVarDesc->varkind)
                                            {
                                                case VAR_STATIC:
                                                    break;   // can this be treated like VAR_CONST, too?
                                                case VAR_CONST:
                                                    result = Variant2Rexx(pVarDesc->lpvarValue);
                                                    RetCode = 0;
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

                if (result == NULL)
                {
                    result = ooRexxString("");
                }
                break;
            case 3:
                result = ooRexxNil;
                // get the named items of the engine
                // a newvalue must not exist since this is read-only!
                if (newvalue == NULL)
                {
                    // it must be "NAMEDITEMS"
                    if (!strcmp(PropName, "NAMEDITEMS"))
                    {
                        // retrieve the NamedItem list of the engine
                        OrxNamedItem* pItemList = engine->getNamedItems();
                        if (pItemList)
                        {
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
                            if (num > 0)
                            {
                                ArrayBound.cElements = num;
                                ArrayBound.lLbound = 0;   // zero-based
                                pSafeArray = SafeArrayCreate(VT_VARIANT,(UINT) 1, &ArrayBound);
                                VariantInit(pVarArray);
                                V_VT(pVarArray) = VT_ARRAY | VT_VARIANT;
                                V_ARRAY(pVarArray) = pSafeArray;

                                for (long i = 0; i < num; i++)
                                {
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
                            result = Variant2Rexx(pVarArray);
                            RetCode = 0;
                        }
                    }
                }
                else
                {
                    RetCode = 4; // can only read the info!
                }

                break;
            default:
                RetCode = -2;    // Bad SelectorType
                break;
        }   //  switch(SelectorType)
    }   //  if (engine)
    else ;  // >>> ??? <<< Good question, what do we do? Error, or return NULL string?
    // result = RexxString("");

    parmblock->value = (RexxObjectPtr)result;
    return RetCode == 0 ? RXEXIT_HANDLED : RXEXIT_RAISE_ERROR;
}

/* unknown callback                                                     */
/* this will deal with objects that REXX is unaware of, but the engine  */
/* is...                                                                */
int RexxEntry RexxNovalueHandler(RexxExitContext *, int ExitNumber, int Subfunction, PEXIT parmblock)
{
    RXVARNOVALUE_PARM *parmblock = (RXVARNOVALUE_PARM*) pblock;
    const char *objname = parmblock->variable_name.strptr;
    RexxObjectPtr  result = NULL; // NULL indicates error
    OrxScript *engine = ScriptProcessEngine::findEngineForThread();
    HRESULT    hResult;
    DISPID     dispID;
    DWORD      flags;
    IDispatch *pDispatch = NULL;
    ITypeInfo *pTypeInfo = NULL;
    VARIANT    temp;

    // do we have an engine to work with?
    if (engine)
    {
        engine->setObjectCreation(false); // turn off IE security manager checking
        engine->PreventCallBack();        // Don't pick up properties.  This stops our GetDispID() from returning DispID's for properties.
        hResult = engine->getNamedItems()->WhoKnows(objname,engine->Lang,&dispID,&flags,&pDispatch,&pTypeInfo);
        engine->ReleaseCallBack();
        if (SUCCEEDED(hResult))
        {
            VariantInit(&temp);
            // do we have a real named item?
            if (dispID == -1)
            {

                V_VT(&temp) = VT_DISPATCH;
                temp.pdispVal = pDispatch;
                // convert it to an OLEObject
                // because the top activation on the activity might not be
                // "native", we have to make sure that it is
                // (Win...Kernel() must never be executed concurrently!)
                result = Variant2Rexx(&temp);
                // this must be a variable / constant
            }
            else
            {
                if (pDispatch == engine)
                {
                    hResult = engine->LocalParseProcedureText(objname,0,&pDispatch);
                    V_VT(&temp) = VT_DISPATCH;
                    temp.pdispVal = pDispatch;
                }
                else
                {
                    hResult = GetProperty(NULL,pDispatch,engine->Lang,&temp,dispID);
                }
                if (SUCCEEDED(hResult))
                {
                    result = Variant2Rexx(&temp);
                }
                VariantClear(&temp);
            }
        }
        engine->setObjectCreation(true); // turn on possible IE security manager checking

    } /* endif engine to work with */
    parmblock->value = (RexxObjectPtr)result;
    return result != NULLOBJECT ? RXEXIT_HANDLED : RXEXIT_NOT_HANDLED;
}


void __stdcall createCode(void *arguments)
{
    CreateCodeData * args = (CreateCodeData *)arguments;

    args->engine->convertTextToCode(args->strCode, &args->routine, args->condData);
    // and explicitly terminate
    _endthreadex(0);
}


int OrxScript::createRoutine(LPCOLESTR strCode, ULONG startingLineNumber, RexxRoutineObject &routine)
{
    CreateCodeData createArgs;
    RexxConditionData cd;
    cd.rc = 0;

    // fill in the args for calling the interpreter to handle all of this
    createArgs.engine = this;
    createArgs.strCode = strCode;
    createArgs.routine = NULLOBJECT;
    createArgs.condData = &cd;

    EnterCriticalSection(&EngineSection);
    pActiveScriptSite->OnEnterScript();

    FPRINTF2(logfile,"create method\n");

        // now create the method (runs in a different thread)
    HANDLE execution = (HANDLE)_beginthreadex(NULL, 0, (unsigned int (__stdcall *) (void*) ) createCode, (LPVOID) createArgs, 0, &dummy);
    // could not start thread?
    if (execution == 0)
    {
        return -1;
    }
    else
    {
        WaitForSingleObject(execution, INFINITE);
        CloseHandle(execution);
    }
    cd.position += startingLineNumber;
    //    The following code HAS to be after the _endthreadex(), or
    //  bad things will happen.  None of the COM calls that result
    //  from the following function calls will work.
    if (cd.rc != 0)
    {
        // an error occured: init excep info
        ErrObj = new OrxScriptError(logfile, &cd, &ErrObj_Exists);
        hResult = pActiveScriptSite->OnScriptError((IActiveScriptError*) ErrObj);
        // init to empty again....
        if (ErrObj_Exists)
        {
            ErrObj->UDRelease();
        }
    }
    return cd.rc;
}


void OrxScript::rexxError(RexxConditionData *condData)
{
    BOOL errObj_Exists;
    // an error occured: init excep info
    OrxScriptError *errObj = new OrxScriptError(logfile, condData, &errObj_Exists);
    pActiveScriptSite->OnScriptError((IActiveScriptError*) errObj);
    // init to empty again....
    if (errObj_Exists)
    {
        errObj->UDRelease();
    }
}

/**
 * Convert a fragment of ooRexx script code into an
 * executable routine.
 *
 * @param context  The instance thread context.
 * @param strCode  The raw string code to convert.
 * @param locationOffset
 *                 The line offset for the script starting location.
 * @param routine  The created routine.
 * @param condData The condition data for any errors.
 */
void OrxScript::convertTextToCode(RexxThreadContext *context, LPCOLESTR strCode, int locationOffset, RexxRoutineObject &routine, RexxConditionData *condData)
{
    condData->rc = 0;            // clear the return code for return

    size_t scriptSize = scslen(pStrCode);

    // we need to convert this script into ascii characters before converting
    char *script = (char *) malloc(sizeof(char) * (1 + scriptSize));
    sprintf(script,"%S",pStrCode);

    routine = context->NewRoutine(getEngineName(), script, scriptSize);
    free(script);
    // convert this into a routine
    if (context->checkException())
    {
        // if we had an exception, then get the decoded exception information and
        RexxDirectoryObject cond = context->GetConditionInfo();
        context->DecodeConditionInfo(cond, condData);
        // adjust this for the position within the script file context.
        condData->position += locationOffset;
        context->ClearException();
    }
}


void OrxScript::processScriptFragment(RexxThreadContext *context, int locationOffset, RexxRoutineObject routine, PRBC &codeBlock, RexxCondition *condData)
{
    //store in global list that will be used
    //in DTOR when leaving engine
    hResult = BuildRCB(RCB::ParseScript, NULL, dwFlags, locationOffset, coutine, &codeBlock);
    if (FAILED(hResult))
    {
        break;
    }
    FPRINTF2(logfile,"The Rexx Routine %p is now in the CodeBlock %p \n", routine, codeBlock);
    // now retrieve all of the public routines defined in this code block and add this to our external
    // table of available routines.

    RexxPackageObject package = context->GetRoutinePackage(routine);
    RexxDirectoryObject routines = context->GetPackagePublicRoutines(package);
    RexxSupplierObject supplier = (RexxSupplierObject)context->SendMessage0(routines, "SUPPLIER");

    int i = 0;
    while (context->SupplierAvailable(supplier))
    {
        PRCB  functionBlock = NULL;
        RexxObjectPtr name = context->SupplierIndex(supplier);
        RexxRoutineObject routine = (RexxRoutineObject)context->SupplierItem(supplier);
        const char *functionName = context->ObjectToStringValue(name);

        hResult = BuildRCB(RCB::ParseScript, NULL, dwFlags, locationOffset, routine, &functionBlock);
        if (FAILED(hResult))
        {
            return;
        }

        RexxFunctions->AddItem(functionName, LinkedList::Beginning, (void*)functionBlock);

        C2W(lName, functionName, strlen(functionName) + 1);
        hResult = DispID.AddDispID(lName,dwFlags, DID::Function, functionBlock, &lDispID);
        FPRINTF2(logfile,"associating method %s with rexx block %p, DispID %d %08x\n", functionName, functionBlock, (int)lDispID, lDispID);

        if (FAILED(hResult))
        {
            break;
        }
    }

}


void OrxScript::queueOrExecuteFragment(RexxThreadContext *context, PRCB codeBlock, RexxConditionData *condData)

    // what to do with the script text we have:
    if (engineState == SCRIPTSTATE_INITIALIZED || engineState == SCRIPTSTATE_UNINITIALIZED)
    {
        //store with stack to exec when connecting...
        RexxExecStack->AddItem(NULL, LinkedList::Beginning, (void*)codeBlock);
        FPRINTF2(logfile,"storing method %p for later execution\n", codeBlock);
    }
    else
    {
        //we are STARTED or CONNECTED: run right away...

        RexxObjectPtr resultDummy;
        this->enableVariableCapture = true;
        runMethod(context, this, codeBlock, NULL, resultDummy, condData);
        this->enableVariableCapture = false;
        //  Do not need to check the ConditionData here.  That was done for us by runMethod().
        if (cd.rc)
        {
            FPRINTF2(logfile,"ParseScriptText - immediate code execution produced an error! (rc = %d)\n",cd.rc);
        }
    }
}


RexxObjectPtr OrxScript::createSecurityObject()
{
    RexxObjectPtr securityObject = NULL;
    FPRINTF2(logfile,"Create_securityObject - start Engine %p \n",pEngine);


    /* create an instance of a security manager object */
    if (getSafetyOptions())
    {
        RexxConditionData condData; // condition info
        bool        ErrObj_Exists;
        HRESULT     hResult=S_OK;

        RexxThreadContext *context = ScriptProcessEngine::getThreadContext();

        memset((void*) &condData,0,sizeof(RexxConditionData));

        RexxObjectPtr flags = context->WholeNumberToObject(getSafetyOptions());

        FPRINTF2(logfile,"Create_securityObject - About to use SecurityManager %p\n", getSecurityManager());

        securityObject = context->SendMessage1(ScriptProcessEngine::getSecurityManagerClass(), "NEW", flags);

        if (context->CheckCondition())
        {
            // if we had an exception, then get the decoded exception information and
            RexxDirectoryObject cond = context->GetConditionInfo();
            context->DecodeConditionInfo(cond, &condData);
            context->ClearException();
            // an error occured: init excep info
            ErrObj = new OrxScriptError(logfile,&condData,&ErrObj_Exists);

            hResult = pEngine->getScriptSitePtr()->OnScriptError((IActiveScriptError*) ErrObj);
            FPRINTF2(logfile,"Create_securityObject - Error encountered: OnScriptError returned %08x.\n",hResult);
            if (FAILED(hResult))
            {
                FPRINTF2(logfile,"Create_securityObject -  Something is unexpected.\n");
            }
            // init to empty again....
            if (ErrObj_Exists)
            {
                ErrObj->UDRelease();
            }
        }
        else
        {
            // protect from GC
            context->RequestGlobalReference(securityObject);
            FPRINTF2(logfile,"Create_securityObject - Success created secturityObject: %p.\n",securityObject);
        }
        context->DetachThread();
    }
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
void OrxScript::runMethod(RexxThreadContext *context, RCB *RexxCode, RexxArrayObject args, RexxObjectPtr &targetResult, RexxConditionData &condData)
{
    bool       fGetVariables = (((int*)arguments)[7] != 0); // get variables from immediate code?

    targetResult = NULLOBJECT;     // make sure no object is set

    memset((void*) condData,0,sizeof(RexxConditionData));

    // thread dependend registration of the engine
    OrxEngine *previous = ScriptProcessEngine::registerEngineForCallback(pEngine);

    FPRINTF2(CurrentObj_logfile,"runMethod about to execute %p from CodeBlock %p \n",RexxCode->Code,RexxCode);

    // call the routine, and check to see if there was an exception
    targetResult = context->CallRoutine(RexxCode->Code, args);
    if (context->CheckException())
    {
        // if we had an exception, then get the decoded exception information and
        RexxDirectoryObject cond = context->GetConditionInfo();
        context->DecodeConditionInfo(cond, &condData);
        context->ClearException();
        // an error occured: init excep info
        ErrObj = new OrxScriptError(logfile,condData,&ErrObj_Exists);
        ErrObj = new OrxScriptError(NULL,condData,&ErrObj_Exists);
        hResult = pEngine->getScriptSitePtr()->OnScriptError((IActiveScriptError*) ErrObj);
        FPRINTF2(logfile,"debug: OnScriptError returned %08x.\n",hResult);
        if (FAILED(hResult))
        {
            FPRINTF2(logfile,"debug:  Something is unexpected.\n");
        }
        // init to empty again....
        if (ErrObj_Exists)
        {
            ErrObj->UDRelease();
        }
    }
    else
    {
        // make sure this is protected for a bit
        context->RequestGlobalReference(targetResult);
        FPRINTF2(logfile,"Rexx call success. Returned object: %p.\n", targetResult);
    }

    // remove engine from thread list
    deregisterEngineForCallback(previous);
    FPRINTF2(logfile,"done (%d %d)\n",rc,condData->rc);

    FPRINTF(logfile,"runMethod ends\n");
}
