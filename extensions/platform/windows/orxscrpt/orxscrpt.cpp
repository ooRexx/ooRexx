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

#include "orxscrpt.hpp"
#include "security.inc"  // REXX source of security manager (OLECHAR *szSecurityCode)
#include "scriptProcessEngine.hpp"

extern CRITICAL_SECTION EngineSection;

//  The following symbol     >>>>???<<<<     indicates that there is
//  work to be done at that point.

/*
 *    The following lists are global to this file.  They can be referenced
 *  from anywhere in this file, and are created only once, no matter how
 *  many times this is invoked.
 *    The flags were taken from the <activscp.h> include file.
 */
FL ScriptItem[] = {       //  H - orizontal flags.
  {SCRIPTITEM_ISVISIBLE,"Visible"},
  {SCRIPTITEM_ISSOURCE,"Event"},            //
  {SCRIPTITEM_GLOBALMEMBERS,"Global"},
  {SCRIPTITEM_ISPERSISTENT,"Persistent"},
  {SCRIPTITEM_CODEONLY,"Code Only"},
  {SCRIPTITEM_NOCODE,"No Code"},
  {(DWORD)0,(char *)NULL}
  };

FL ScriptTypeLib[] = {    //  H - orizontal flags.
  {SCRIPTTYPELIB_ISCONTROL,"Control"},
  {SCRIPTTYPELIB_ISPERSISTENT,"Persistent"},
  {(DWORD)0,NULL}
  };

FL ScriptText[] = {       //  H - orizontal flags.
  {SCRIPTTEXT_DELAYEXECUTION,"Delay Execution"},
  {SCRIPTTEXT_ISVISIBLE,"Visible"},
  {SCRIPTTEXT_ISEXPRESSION,"Expression"},
  {SCRIPTTEXT_ISPERSISTENT,"Persistent"},
  {SCRIPTTEXT_HOSTMANAGESSOURCE,"Host manages source"},
  {(DWORD)0,NULL}
  };

FL ScriptProc[] = {       //  H - orizontal flags.
  {SCRIPTPROC_HOSTMANAGESSOURCE,"Host manages source"},
  {SCRIPTPROC_IMPLICIT_THIS,"THIS"},
  {SCRIPTPROC_IMPLICIT_PARENTS,"Parents"},
  {(DWORD)0,NULL}
  };

//    We send these.  I really doubt if we will need to print these.
//  See the note in ???? on their exact use.
FL ScriptInfo[] = {       // V - ertical
  {SCRIPTINFO_IUNKNOWN,"IUnkown"},
  {SCRIPTINFO_ITYPEINFO,"ITypeInfo"},
  {(DWORD)0,NULL}
  };

FL ScriptInterrupt[] = {  //  H - orizontal flags.
  {SCRIPTINTERRUPT_DEBUG,"Debug"},
  {SCRIPTINTERRUPT_RAISEEXCEPTION,"Raise Exception"},
  {(DWORD)0,NULL}
  };

FL ScriptStat[] = {       //  V - ertical
  {SCRIPTSTAT_STATEMENT_COUNT,"Statement Count"},
  {SCRIPTSTAT_INSTRUCTION_COUNT,"Instruction Count"},
  {SCRIPTSTAT_INTSTRUCTION_TIME,"Instruction Time"},
  {SCRIPTSTAT_TOTAL_TIME,"Total Execution Time"},
  {(DWORD)0,NULL}
  };

FL ScriptEngineState[] = {  //  V - ertical
  {SCRIPTSTATE_UNINITIALIZED,"Uninitialized"},
  {SCRIPTSTATE_INITIALIZED,"Initialized"},
  {SCRIPTSTATE_STARTED,"Started"},
  {SCRIPTSTATE_CONNECTED,"Connected"},
  {SCRIPTSTATE_DISCONNECTED,"Disconnected"},
  {SCRIPTSTATE_CLOSED,"Closed"},
  {(DWORD)0,NULL}
  };

FL ScriptThreadID[] = {   //  H - orizontal flags.
  {SCRIPTTHREADID_CURRENT,"Current"},
  {SCRIPTTHREADID_BASE,"Base"},
  {(DWORD)0,NULL}
  };

FL TypeKind[] = { // V - ertical
  {TKIND_ENUM,"Enum"},
  {TKIND_RECORD,"Record"},
  {TKIND_MODULE,"Module"},
  {TKIND_INTERFACE,"Interface"},
  {TKIND_DISPATCH,"Dispatch"},
  {TKIND_COCLASS,"CoClass"},
  {TKIND_ALIAS,"Alias"},
  {TKIND_UNION,"Union"},
  {TKIND_MAX,"Max(??)"},
  {(DWORD)0,NULL}
};

FL InterfaceSafetyOptions[] = {       //  H - orizontal flags.
  {INTERFACESAFE_FOR_UNTRUSTED_DATA,"'Safe for Untrusted data'"},
  {INTERFACESAFE_FOR_UNTRUSTED_CALLER,"'Safe for Untrusted Caller'"},
  {INTERFACE_USES_DISPEX,"Supports IDispatchEx"},
  {INTERFACE_USES_SECURITY_MANAGER,"Uses Security Manager"},
  {(DWORD)0,(char *)NULL}
  };

extern FL VariantFlags[];                // H - orizontal flags
extern FL VariantTypes[];                // V - ertical flags



// CTOR
// set reference count to one
// set engine state to uninitialized
// set engine flag to disconnected
// set thread state to not running
// set active script site to NULL
// remember current thread id
OrxScript::OrxScript() : ulRefCount(1),
                         checkObjectCreation(true),
                         initNew(false),
                         isConnected(false),
                         engineState(SCRIPTSTATE_UNINITIALIZED),
                         threadState(SCRIPTTHREADSTATE_NOTINSCRIPT),
                         pActiveScriptSite(NULL),
                         pIESecurityManager(NULL),
                         dwSafetyOptions(0),
                         securityObject(NULL),
                         EventSourceName(NULL),
                         EventState(Abstain),
                         EventCount(0),
                         dwBaseThread(GetCurrentThreadId())
{
    HANDLE execution;
    EnterCriticalSection(&EngineSection);

    engineId = ScriptProcessEngine::getEngineId();
    logfile= NULL;
#if SCRIPTDEBUG
    char   filename[MAX_PATH];
    sprintf(filename,"c:\\temp\\engine%d.log",engineId);
    logfile = fopen(filename,"w");
    if (!logfile)
    {
        logfile = stderr;
    }
    CurrentObj_logfile = logfile;
#endif
    FPRINTF(logfile,"created script engine (%d) %p\n", engineId, this);
    FPRINTF2(ScriptProcessEngine::logfile, "created script engine(%d) %p\n", engineId, this);
    Events = new OrxEvent(this,logfile);
    NamedItemList = new OrxNamedItem(this,logfile);
    RexxCodeList = new LinkedList();
    RexxFunctions = new Index();
    RexxExecStack = new Index();

    InterlockedIncrement((long *)&ulDllLocks);     //  Make sure the DLL does not go away before we do.

    // create code block that will be used to obtain a security manager
    memset((void*)&this->securityManager, 0, sizeof(RCB));


    // now create the method (runs in a different thread)
    execution = (HANDLE) _beginthreadex(NULL, 0, (unsigned int (__stdcall *) (void*) )createSecurityManager, (LPVOID) this, 0, &dummy);
    if (execution)
    {
        WaitForSingleObject(execution, INFINITE);
        CloseHandle(execution);
    }

    FPRINTF2(logfile,"RCB generation of security manager: CodeBlock %p rc = %d\n",this->securityManager.Code, cd.rc);
    FPRINTF2(logfile,"done with CTOR for %d\n", engineId);
    LeaveCriticalSection(&EngineSection);
}


// DTOR
OrxScript::~OrxScript()
{
    EnterCriticalSection(&EngineSection);

    FPRINTF(logfile,"~OrxScript()  called for engine(%d) %p\n", engineId, this);
    FPRINTF2(DLLlogfile,"DTOR called for engine(%d) %p\n", engineId, this);
    //  This chain is a loose linked list.  This means that the delete that follows
    //  will only remove the chain, and not the event objects.  This is so the
    //  Internet Explorer can call them with a Release() after the engine is gone.
    //  Note, however, if the Event does anything but destroy itself through a
    //  Release(), disaster will happen.  There will be NO ENGINE.  This is not cool,
    //  but it fits the definition of COM.
    delete Events;

    if (engineState != SCRIPTSTATE_CLOSED)
    {
        SetScriptState(SCRIPTSTATE_CLOSED); // shut down the engine if it is not already
    }
    // closed (meaning all pointers it held etc. have been released)
    if (pActiveScriptSite)
    {
        pActiveScriptSite->Release();
        pActiveScriptSite = NULL;
    }
    if (pIESecurityManager)
    {
        pIESecurityManager->Release();
        pIESecurityManager = NULL;
    }

    //  Clear lists whose heads were not dynamically allocated.
    delete NamedItemList;
    // DispID.DeleteList();           <---  Ugggh, this method is not there, yet. >>>>???<<<<
    PropertyList.DeleteList();

    // release list of all rexx code blocks
    delete RexxExecStack;
    delete RexxFunctions;
    delete RexxCodeList;

    InterlockedDecrement((long *)&ulDllLocks);

    FPRINTF2(logfile,"~OrxScript()  for engine(%d) complete\n", engineId);
    if (logfile != stderr) {
        fclose(logfile);  // for debugging
        logfile = NULL;
    }
    LeaveCriticalSection(&EngineSection);
}



/****************************************/
/* implementation of IUnknown interface */
/*                           complete + */
/****************************************/

/*************************************************************/
/* OrxScript::QueryInterface                                 */
/*                                                           */
/* return an interface pointer of the requested IID.         */
/* this means simply casting "this" to the desired interface */
/* pointer, if the interface is supported.                   */
/*************************************************************/
STDMETHODIMP OrxScript::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hResult = E_NOINTERFACE;
    OLECHAR  cIID[100];
    char    *IIDName,TrulyUnknown[]="??????";


    FPRINTF2(logfile,"\n");
    StringFromGUID2(riid, cIID, sizeof(cIID));

    FPRINTF(logfile, "OrxScript::QueryInterface (ppvObj = %p \n",ppvObj);
    FPRINTF2(logfile,"riid = %S \n",cIID);
    //  We should look this riid up in HKClass_Root\Interface\.... to print what it represents.  <----

    // a pointer to result storage must be supplied
    if (!ppvObj)
    {
        return ResultFromScode(E_INVALIDARG);
    }
    // set to NULL initiallly
    *ppvObj = NULL;


    FPRINTF2(logfile,"It is the (");
    // need to supply an IUnknown pointer?
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (LPVOID)(IUnknown *)(IActiveScript *) this;
        FPRINTF3(logfile,"IUnknown");
    }
    // need to supply an IActiveScript pointer?
    else if (IsEqualIID(riid, IID_IActiveScript))
    {
        *ppvObj = (LPVOID)(IActiveScript *) this;
        FPRINTF3(logfile,"IActiveScript");
    }
    // need to supply an IActiveScriptParse pointer?
    else if (IsEqualIID(riid, IID_IActiveScriptParse))
    {
        *ppvObj = (LPVOID)(IActiveScriptParse *) this;
        FPRINTF3(logfile,"IActiveScriptParse");
    }
    // need to supply an IActiveScriptParseProcedure pointer?
    else if (IsEqualIID(riid, IID_IActiveScriptParseProcedure))
    {
        *ppvObj = (LPVOID)(IActiveScriptParseProcedure *) this;
        FPRINTF3(logfile,"IActiveScriptParseProcedure");
    }
    // need to supply an IObjectSafety pointer?
    else if (IsEqualIID(riid, IID_IObjectSafety))
    {
        *ppvObj = (LPVOID)(IObjectSafety *) this;
        FPRINTF3(logfile,"IObjectSafety");
    }
    else if (riid == IID_IDispatch)
    {
        FPRINTF3(logfile,"IDispatch");
        *ppvObj = static_cast<IDispatch*>(this);
    }

    else if (riid == IID_IDispatchEx)
    {
        FPRINTF3(logfile,"IDispatchEx");
        *ppvObj = static_cast<IDispatch*>(this);
    }
    else
    {
        if (!(IIDName = NameThatInterface((OLECHAR *)&cIID[0])))
        {
            IIDName = &TrulyUnknown[0];
        }
        FPRINTF3(logfile,"unsupported  %s",IIDName);
        if (IIDName != &TrulyUnknown[0])
        {
            free(IIDName);
        }
    }

    FPRINTF3(logfile,") interface.\n\n");

    // on success, call AddRef()
    if (*ppvObj != NULL)
    {
        AddRef();
        hResult = NOERROR;
    }

    return hResult;
}

/**********************************/
/* OrxScript::AddRef              */
/*                                */
/* increment the reference count. */
/**********************************/
STDMETHODIMP_(ULONG) OrxScript::AddRef()
{
    //  Returns > 0 if ulRefCount > 0, but is not guaranteed
    // to return the actual value of ulRefCount.
    InterlockedIncrement((long *)&ulRefCount);
    FPRINTF(logfile,"OrxScript::AddRef The count is now %u\n",ulRefCount);
    return ulRefCount;
}

/***********************************************************/
/* OrxScript::Release                                      */
/*                                                         */
/* decrement the reference count, if zero, destroy object. */
/***********************************************************/
STDMETHODIMP_(ULONG) OrxScript::Release()
{
    InterlockedDecrement((long *)&ulRefCount);
    FPRINTF(logfile,"OrxScript::Release The count is now %u\n",ulRefCount);
    if (ulRefCount)
    {
        return ulRefCount;
    }

    delete this;
    return 0;
}



/*********************************************/
/* implementation of IActiveScript interface */
/*                                complete - */
/*********************************************/

/**************************************/
/* OrxScript::SetScriptSite           */
/*                                    */
/* set the script site of the engine. */
/**************************************/
STDMETHODIMP OrxScript::SetScriptSite(IActiveScriptSite *pActiveScriptSite)
{
    HRESULT hResult = E_FAIL;

    FPRINTF(logfile,"OrxScript::SetScriptSite (pActiveScriptSite = %p)\n",pActiveScriptSite);
    //CHECK_BASE_THREAD;    // can only be called within the same thread

    // is the pointer valid?
    if (!pActiveScriptSite)
    {
        hResult = E_POINTER;
    }
    // site already set?
    else if (this->pActiveScriptSite)
    {
        hResult = E_UNEXPECTED;
    }
    else
    {
        // set the site
        this->pActiveScriptSite = pActiveScriptSite;
        // add ref the site.  It is released when in the engine gets closed (possibly from the DTOR)
        this->pActiveScriptSite->AddRef();
        hResult = this->pActiveScriptSite->GetLCID(&Lang);
        if (FAILED(hResult))
        {
            FPRINTF2(logfile,"OrxScript::SetScriptSite   Failed to get a language code.\n");
            Lang = 0;
        }
        hResult = S_OK;
        // using security manager?
        if ( dwSafetyOptions & INTERFACE_USES_SECURITY_MANAGER)
        {
            IServiceProvider             *pProvider = NULL;

            hResult = this->pActiveScriptSite->QueryInterface(IID_IServiceProvider,(void**) &pProvider);
            if (SUCCEEDED(hResult))
            {
                hResult = pProvider->QueryService(SID_SInternetHostSecurityManager,IID_IInternetHostSecurityManager,(void**) &pIESecurityManager);
                if (FAILED(hResult))
                {
                    pIESecurityManager = NULL;
                }
                else
                {
                    // set the callback in the OLE code
                    // this will try to get back to the engine with it's registration
                    // of id and thread in the table used in eng2rexx
                    ::setCreationCallback(scriptSecurity);
                }
                pProvider->Release();
            }
        }

        // according to MSDN "Windows Script Engines" this causes
        // the engine to transit from UNINITIALIZED to INITIALIZED, if
        // IPersistInitStream::Load, IPersistInitStream::InitNew or
        // IActiveScriptParse::InitNew have been called
        if (initNew)
        {
            SetScriptState(SCRIPTSTATE_INITIALIZED);
        }
    }
    return hResult;
}

/******************************************************/
/* OrxScript::GetScriptSite                           */
/*                                                    */
/* get an interface of the script site of the engine. */
/* the invocation will be passed on to QueryInterface */
/* of the script site.                                */
/******************************************************/
STDMETHODIMP OrxScript::GetScriptSite(REFIID riid, void **povSiteObject)
{

// NOT IMPLEMENTED YET...!!!
// It's no wonder, the documentation is VERY unclear as to
// what this is supposed to do.
    OLECHAR    cIID[100];
    StringFromGUID2(riid, cIID, sizeof(cIID));
    FPRINTF(logfile,"OrxScript::GetScriptSite (riid = %S, povSiteObject = %p)\n",cIID,povSiteObject);
    FPRINTF2(logfile,"OrxScript::GetScriptSite() Not yet implemented!!!!!\n");
    if (!*povSiteObject)
    {
        return E_POINTER;
    }
    return E_NOINTERFACE;
}

/************************************************************/
/* OrxScript::SetScriptState                                */
/*                                                          */
/* change the state of the script engine. causes the engine */
/* to pass all necessary states between current state and   */
/* wanted state (if possible!)                              */
/************************************************************/
STDMETHODIMP OrxScript::SetScriptState(SCRIPTSTATE state)
{
    char     *s[] = { "UNINITIALIZED", "STARTED", "CONNECTED", "DISCONNECTED", "CLOSED", "INITIALIZED"};
    HRESULT   hResult = S_OK;
    ListItem *item;
    // UINT      dummy;
    // HANDLE    execution;
    LPVOID    arguments[8];
    RexxObjectPtr resultDummy;
    RexxConditionData cd;

    FPRINTF(logfile,"OrxScript::SetScriptState (state = %s)\n",s[(int) state]);
    FPRINTF2(logfile,"OrxScript::SetScriptState() going from %s to %s\n",FlagMeaning('V',engineState,ScriptEngineState),
             FlagMeaning('V',state,ScriptEngineState));

    if (engineState == SCRIPTSTATE_CLOSED)
        hResult = E_UNEXPECTED;
    else if (engineState == state)
        hResult = S_FALSE;
    else switch (state)
        {
            // going to CLOSED should not be allowed, i think, or Close() must
            // be called here, but i favour disallowing setting the state to closed,
            // because the host might call Close() just as well!
            case SCRIPTSTATE_CLOSED:
                if (engineState == SCRIPTSTATE_CONNECTED)
                    hResult = SetScriptState(SCRIPTSTATE_DISCONNECTED);
                if (hResult == S_OK)
                    engineState = SCRIPTSTATE_CLOSED;
                break;
                // ...means: IPersist*::Load, (IPersist*,IActiveScriptParse)::InitNew has
                // has been called.
                // we do nothing, they will set us to this state
            case SCRIPTSTATE_INITIALIZED:
                engineState = SCRIPTSTATE_INITIALIZED;
                break;
            case SCRIPTSTATE_UNINITIALIZED:
                // IIS (ASP server) sets the engine from STARTED to UNINITIALIZED
                // from what I remember from the documentation, this is ILLEGAL
                // it may be a shortcut way of rerunning a script, but only Microsoft
                // knows exactly what is intended to happen in this scenario
                // returning this error causes the webserver to shut down the engine
                // and create a new instance again...
                hResult = E_UNEXPECTED;
                break;
                // execute queued code, not yet connected to events
            case SCRIPTSTATE_STARTED:
                {
                    RexxThreadContext *context = ScriptProcessEngine::getThreadContext();
                    this->enableVariableCapture = true;

                    EnterCriticalSection(&EngineSection);
                    pActiveScriptSite->OnEnterScript();
                    // run all rexx code blocks ("immediate code")
                    while ( item = RexxExecStack->FindItem(0) )
                    {
                        arguments[1] = (LPVOID) item->GetContent();  // get code block

                        FPRINTF2(logfile,"OrxScript::SetScriptState() Executing Rexx Codeblock %p\n",arguments[1]);

                        runMethod(context, item->GetContent(), NULL, resultDummy, cd);
                        // remove from exec stack
                        RexxExecStack->DropItem(item);
                    }
                    enableVariableCapture = false;   // turn off the capture operations
                    context->DetachThread();
                    pActiveScriptSite->OnLeaveScript();
                    LeaveCriticalSection(&EngineSection);

                    engineState = SCRIPTSTATE_STARTED;
                    break;
                }
                // connect to events, ready to be called for execution
            case SCRIPTSTATE_CONNECTED:
                // if we come from being INITIALIZED, we must go through STARTED...
                if (engineState == SCRIPTSTATE_INITIALIZED)
                    hResult = SetScriptState(SCRIPTSTATE_STARTED);
                // connect here...
                if (hResult == S_OK)
                {
                    hResult = Events->ConnectEvents();
                    engineState = SCRIPTSTATE_CONNECTED;
                }
                break;
            case SCRIPTSTATE_DISCONNECTED:
                // "disconnect" here from the events (ignore them)
                hResult = Events->DisconnectEvents();
                engineState = SCRIPTSTATE_DISCONNECTED;
                break;
            default:
                FPRINTF2(logfile,"OrxScript::SetScriptState() unknown state!!!\n");
                break;
        }

    //  Must tell the ScriptSite that our state has changed.
    if (pActiveScriptSite)
    {
        pActiveScriptSite->OnStateChange(state);
    }

    FPRINTF2(logfile,"OrxScript::SetScriptState() engine state is now %s\n",s[(int) engineState]);
    return hResult;
}

/****************************************************************/
/* OrxScript::GetScriptState                                    */
/*                                                              */
/* provide state of engine; can be called from non-base threads */
/****************************************************************/
STDMETHODIMP OrxScript::GetScriptState(SCRIPTSTATE *pState)
{
    HRESULT hResult = E_POINTER;

    FPRINTF(logfile,"OrxScript::GetScriptState ");
    // if a pointer was supplied, write state into that address
    if (pState)
    {
        *pState = engineState;
        hResult = S_OK;
    }
    FPRINTF2(logfile,"(pState = %p, Content = %d)\n",pState,*pState);
    return hResult;
}

/****************************************************************/
/* OrxScript::Close                                             */
/*                                                              */
/* abandon any currently loaded script, lose state, release all */
/* currently held interface pointers.                           */
/* gets called from host as a shortcut way to kill the engine.  */
/****************************************************************/
STDMETHODIMP OrxScript::Close()
{
    HRESULT hResult = E_FAIL;
    FPRINTF(logfile,"OrxScript::Close() the current state is %s\n",FlagMeaning('V',engineState,ScriptEngineState));

//  CHECK_BASE_THREAD;    // can only be called within the same thread

    if (engineState == SCRIPTSTATE_CLOSED)
    {
        hResult = E_UNEXPECTED;
    }
    else
    {
        hResult = SetScriptState(SCRIPTSTATE_CLOSED);
    }
    FPRINTF2(logfile,"OrxScript::Close() we are now %s\n",FlagMeaning('V',engineState,ScriptEngineState));
    return hResult;
}

/**************************************************************/
/* OrxScript::AddNamedItem                                    */
/*                                                            */
/* this method is used to add new items and namespaces to the */
/* script engine.  Unlike the sample engine, this does not    */
/* attempt to interpret the flags.  It saves all named items  */
/* and leaves it up to the NamedItem::()'s to make the        */
/* correct interpretation.                                    */
/**************************************************************/
STDMETHODIMP OrxScript::AddNamedItem(LPCOLESTR pStrName, DWORD dwFlags)
{
    int PPreviously=0;    // No, we have not printed previously.
    ITypeInfo *ANITypeInfo=NULL;
    IUnknown  *ANIUnknown=NULL;
    IDispatch *ANIDispatch=NULL;
    HRESULT    RetCode;


    FPRINTF3(logfile,"\n");
    FPRINTF(logfile,"OrxScript::AddNamedItem (StrName = \"%S\", dwFlags = %08x)\n",pStrName,dwFlags);
    FPRINTF2(logfile,"In english the dwFlags signifies:\n");
    FPRINTF2(logfile,"%s\n",FlagMeaning('H',dwFlags,ScriptItem));

    if (pStrName == NULL)
    {
        FPRINTF2(logfile,"No ItemInfo to get.  This is the default-global code.\n");
    }
    else
    {

        /*
         *    Two calls must be made to GetItemInfo(); one for each type of return.  The
         *  second parameter, the Mask, cannot be combined when makin the call -
         *  >>>Mask = SCRIPTINFO_IUNKNOWN+SCRIPTINFO_ITYPEINFO;      Neither wi
         *  >>>Mask = SCRIPTINFO_IUNKNOWN|SCRIPTINFO_ITYPEINFO;                ll work!
         *  If either IUNKNOWN or ITYPEINFO is not available, then the whole call fails,
         *  even though the expected reaction is to set the pointer to whatever is
         *  available, and NULL the other.
         *    Instead, we start with all pointers NULLed, and make successive calls.
         *  The hRestult is ignored.  It is superfluous, since a non-NULL pointer is returned
         *  by a successful call.
         */
        if (FAILED(pActiveScriptSite->GetItemInfo(pStrName,SCRIPTINFO_IUNKNOWN,&ANIUnknown,NULL)))
        {
            ANIUnknown = NULL;
        }
        if (FAILED(pActiveScriptSite->GetItemInfo(pStrName,SCRIPTINFO_ITYPEINFO,NULL,&ANITypeInfo)))
        {
            ANITypeInfo = NULL;
        }
        if (ANIUnknown != NULL)
        {
            FPRINTF2(logfile,"There is an IUnknown:\n");
            /*
              The sample code calls the AddRef() for this object, presumably because we have received
              the pointer to the IUnknown without calling its interface.
                However, the sample code is wrong.  The reference count is incremented for each
              GetItemInfo() call that returns an IUnknown.
            */
            ANIUnknown->AddRef();
        }
        else
        {
            FPRINTF2(logfile,"There is NO IUnknown.\n");
        }

        if (ANITypeInfo != NULL)
        {
            FPRINTF2(logfile,"There is an Type Info:\n");
        }
        else
        {
            FPRINTF2(logfile,"There is NO ITypeInfo.\n");
        }
        // Save everything that we have learned to date.
        RetCode = NamedItemList->AddItem(pStrName,dwFlags,ANIUnknown,ANITypeInfo);
        FPRINTF2(logfile,"The return code from AddItem() %08x.\n",RetCode);
    }
    FPRINTF2(logfile,"OrxScript::AddNamedItem  - leaving\n");

    return S_OK;

    /* SCRIPTITEM_CODEONLY - This flag indicates that the name is only used to     */
    /*                       create a new namespace within the script engine.      */
    /*                       It informs the engine that there is no external COM   */
    /*                       object that will provide methods for scripts to call. */

    /* SCRIPTITEM_NOCODE - This flag indicates that the name only refers to an     */
    /*                     external COM object that will provide methods for       */
    /*                     scripts to call. It informs the engine that it doesn't  */
    /*                     need to create a namespace for this name, because the   */
    /*                     Host will not add any scripts for this name.            */

    /* SCRIPTITEM_GLOBALMEMBERS - This flag informs the engine that the Host wants  */
    /*                            this name to be used to extend the run-time       */
    /*                            libraries of the engine. Methods provided by this */
    /*                            object can be called from any script context      */
    /*                            without specifying the named item they belong to. */

    // globalmembers flag:
    // in conjunction with the nocode flag, i suggest the following:
    // - map methods to functions (REXX_SETFUNC)
    // - map attributes (those with a invkind 2 and no invkind 4!) to global variables (REXX_SETVAR)


    /* SCRIPTITEM_ISPERSISTENT - This flag indicates that this named item should be */
    /*                           persisted.  It informs script engines that support */
    /*                           any of the IPersist* interfaces or the             */
    /*                           IActiveScript::Clone method that this named item   */
    /*                           should be saved as part of the engine's state.     */

    /* SCRIPTITEM_ISSOURCE - This flag indicates that this named item supports    */
    /*                       events that the Host may wish to sink in script. The */
    /*                       engine uses this hint to determine which objects it  */
    /*                       needs to connect as event sources.                   */

    /* SCRIPTITEM_ISVISIBLE - This flag informs the script engine that the named    */
    /*                        item can be referenced in scripts. Without this flag, */
    /*                        references to the named item will be unresolved.      */
    // i honestly do not see why the host would ever add a named item WITHOUT this
    // flag?!

}

STDMETHODIMP OrxScript::AddTypeLib(REFGUID rguidTypeLib, DWORD dwMaj, DWORD dwMin, DWORD dwFlags)
{
    HRESULT    hResult;
    ITypeLib  *pTypeLib = NULL;
    ITypeInfo *pTypeInfo;
    TYPEATTR  *pTypeAttr = NULL;
    UINT       iTypeInfoCount;
    UINT       j;
    bool       added;

    OLECHAR  cIID[100];
    char     space[]="                 ";
    FPRINTF3(logfile,"\n");
    StringFromGUID2(rguidTypeLib, cIID, sizeof(cIID));
    FPRINTF(logfile,"OrxScript::AddTypeLib (rguidTypeLib = %S,\n\
   dwMaj = %08x, dwMin = %08x, dwFlags = %08x)\n",cIID,dwMaj,dwMin,dwFlags);
    FPRINTF2(logfile,"In english the dwFlags signifies:\n");
    FPRINTF2(logfile,"%s\n",FlagMeaning('H',dwFlags,ScriptTypeLib));
    hResult = LoadRegTypeLib(rguidTypeLib,(WORD)dwMaj,(WORD)dwMin,/*LOCALE_USER_DEFAULT*/this->Lang,&pTypeLib);
    if (SUCCEEDED(hResult))
    {
        iTypeInfoCount = pTypeLib->GetTypeInfoCount();
        FPRINTF2(logfile,"%sThis type library contains %d type info blocks.\n",space,iTypeInfoCount);
        for (j=0; j<iTypeInfoCount; j++)
        {
            hResult = pTypeLib->GetTypeInfo(j,&pTypeInfo);
            if (SUCCEEDED(hResult))
            {
                hResult = pTypeInfo->GetTypeAttr(&pTypeAttr);
                added = false;
                if (SUCCEEDED(hResult))
                {
                    FPRINTF2(logfile,"%s  %2d %s (%d functions, %d variables) flags: %04x\n",space,j,FlagMeaning('V',pTypeAttr->typekind,TypeKind),pTypeAttr->cFuncs,pTypeAttr->cVars,pTypeAttr->wTypeFlags);
                    if (pTypeAttr->cVars > 0)
                    {
                        NamedItemList->AddItem(NULL,dwFlags,NULL,pTypeInfo);
                        added = true;
                    }

                    pTypeInfo->ReleaseTypeAttr(pTypeAttr);
                }
                if (!added)
                {
                    pTypeInfo->Release();
                }
            }
        }
        pTypeLib->Release();
    }
    return hResult;
}



STDMETHODIMP OrxScript::GetScriptDispatch(LPCOLESTR pStrItemName, IDispatch **ppDisp)
{
    HRESULT    RetCode = E_POINTER;
    IDispatch *Disp;

    FPRINTF3(logfile,"\n");
    FPRINTF(logfile,"OrxScript:GetScriptDispatch (StrItemName = \"%S\", ppDisp = %p)\n",pStrItemName,ppDisp);
    if (ppDisp)
    {
        Disp = (IDispatch *)this;
        Disp->AddRef();
        RetCode = S_OK;
        *ppDisp = Disp;
    }
    return RetCode;
}

/****************************************************************************/
/* OrxScript::GetCurrentScriptThreadID                                      */
/*                                                                          */
/* retrieve an engine-defined identifier of the currently executing thread. */
/* only one REXX script will ever be run by the engine, so we can return a  */
/* constant here.                                                           */
/****************************************************************************/
STDMETHODIMP OrxScript::GetCurrentScriptThreadID(SCRIPTTHREADID *pStidThread)
{
    FPRINTF(logfile,"OrxScript::GetCurrentScriptThreadID (pStidThread = %p)\n",pStidThread);
    *pStidThread = (SCRIPTTHREADID)1;
    return S_OK;
}

/*****************************************************************/
/* OrxScript::GetScriptThreadID                                  */
/*                                                               */
/* retrieve an engine-defined identifier of a specified thread.  */
/* this function is primarily a no-doer since there will only be */
/* one executing thread for the engine...                        */
/*****************************************************************/
STDMETHODIMP OrxScript::GetScriptThreadID(DWORD dwWin32ThreadID, SCRIPTTHREADID *pStidThread)
{
    FPRINTF(logfile,"OrxScript::GetScriptTheadID (dwWin32ThreadID = %08x, pStidThread = %p)\n",dwWin32ThreadID,pStidThread);
    //just pass on the call to GetCurrentScriptThreadID
    return GetCurrentScriptThreadID( pStidThread );
}




/******************************************************************************
*  GetScriptThreadState -- This method returns the state of the given script
*  thread.
*
*  Parameters: stidThread -- SCRIPTTHREADID_BASE
*                            SCRIPTTHREADID_CURRENT
*              pstsState -- address of the variable which receives the state
*                           of the given script thread.
*  Returns: S_OK
*           E_POINTER
*           E_UNEXPECTED
******************************************************************************/
STDMETHODIMP OrxScript::GetScriptThreadState(SCRIPTTHREADID stidThread, SCRIPTTHREADSTATE *pStsState)
{
    HRESULT hResult = E_POINTER;

    FPRINTF(logfile,"OrxScript::GetScriptThreadState (stidThread = %08x, pStsState = %p)\n",stidThread,pStsState);
    if (engineState == SCRIPTSTATE_CLOSED)
        hResult = E_UNEXPECTED;
    else if (pStsState)
    {
        *pStsState = SCRIPTTHREADSTATE_NOTINSCRIPT;
        hResult = S_OK;
    }
    return hResult;
}




/******************************************************************************
*  InterruptScriptThread -- Interrupts the execution of a running script thread
*  (an event sink, an immediate execution, or a macro invocation). This method
*  can be used to terminate a script that is stuck (in an infinite loop,
*  for example). It can be called from non-base threads without resulting in a
*  non-base callout to host objects or to the IActiveScriptSite method.
*
*  Parameter:  stidThread -- SCRIPTTHREADID_ALL
*                            SCRIPTTHREADID_BASE
*                            SCRIPTTHREADID_CURRENT
*              pexcepinfo -- Address of an EXCEPINFO structure that receives
*                            error info.
*              dwFlags -- SCRIPTINTERRUPT_DEBUG
*                         SCRIPTINTERRUPT_RAISEEXCEPTION
*
*  Returns: S_OK
*           E_INVALIDARG
*           E_POINTER
*           E_UNEXPECTED
******************************************************************************/
STDMETHODIMP OrxScript::InterruptScriptThread(SCRIPTTHREADID stidThread, const EXCEPINFO *pexcepinfo, DWORD dwFlags)
{
    FPRINTF(logfile,"OrxScript::InterruptScriptThread (stidThread = %08x, pexcepinfo = %p, dwFlags = %08x)\n",stidThread,pexcepinfo,dwFlags);

    /*    As can be seen from the ...GetThreadID() above, we are only supporting one thread.
     *  Therefore, it doesn't really make any sense to implement this.  This may change if
     *  we have to spawn threads for each scriptlet.
     */
    return E_FAIL;
}




STDMETHODIMP OrxScript::Clone(IActiveScript **ppActiveScript)
{
    FPRINTF(logfile,"OrxScript::Clone (ppActiveScript = %p)\n",ppActiveScript);
    return E_NOTIMPL;
}



/**************************************************/
/* implementation of IActiveScriptParse interface */
/*                                     complete - */
/**************************************************/

// !: the question, though: is this InitNew of IActiveScriptParse
//    or of IPersistStreamInit? are they allowed to behave the same?!
STDMETHODIMP OrxScript::InitNew(void)
{

    HRESULT hResult = S_OK;

    FPRINTF(logfile,"OrxScript::InitNew\n");
//  CHECK_BASE_THREAD;    // can only be called within the same thread

    // !: init new
    // on success:
    initNew = true;

    // according to MSDN "Windows Script Engines" this causes
    // the engine to transit from UNINITIALIZED to INITIALIZED if
    // the script site is set
    if (pActiveScriptSite)
    {
        SetScriptState(SCRIPTSTATE_INITIALIZED);
    }

    return hResult;
}

STDMETHODIMP OrxScript::AddScriptlet(LPCOLESTR  pStrDefaultName,
                                     LPCOLESTR  pStrCode,
                                     LPCOLESTR  pStrItemName,
                                     LPCOLESTR  pStrSubItemName,
                                     LPCOLESTR  pStrEventName,
                                     LPCOLESTR  pStrDelimiter,
                                     DWORD      dwSourceContextCookie,
                                     ULONG      ulStartingLineNumber,
                                     DWORD      dwFlags,
                                     BSTR      *pBstrName,
                                     EXCEPINFO *pExcepInfo)
{
    HRESULT    RetCode;
    OLECHAR    NewName[MAX_PATH];
    DISPID     EventSinkDispID;            // The numeric value this automates under.
    HANDLE     execution;
    UINT       dummy;
    PRCB       CodeBlock;
    RexxConditionData cd;
    OrxScriptError *ErrObj;
    bool        ErrObj_Exists;
    RexxObjectPtr  pResult = NULL;


    FPRINTF3(logfile,"\n\n\n\n");
    FPRINTF(logfile,"OrxScript::AddScriptlet (StrDefaultName = \"%S\", StrCode = \n\"%S\"\n",pStrDefaultName, pStrCode);
    FPRINTF2(logfile,"StrItemName = \"%S\", StrSubItemName = \"%S\", StrEventName = \"%S\",\n", pStrItemName, pStrSubItemName, pStrEventName);
    FPRINTF2(logfile,"StrDelimiter = \"%S\", dwSourceContextCookie = %08x, ulStartingLineNumber = %lu,\n", pStrDelimiter, dwSourceContextCookie, ulStartingLineNumber);
    FPRINTF2(logfile,"dwFlags = %08x, BstrName = \"%S\", pExcepInfo = %p)\n"  , dwFlags, pBstrName, pExcepInfo);
    FPRINTF2(logfile,"In english the dwFlags signifies: %s\n",FlagMeaning('H',dwFlags,ScriptText));

    //  If this event has already been added, then don't do it again.
    RetCode = Events->FindEvent(pStrItemName,pStrSubItemName,pStrEventName,&EventSinkDispID);
    if (SUCCEEDED(RetCode) && EventSinkDispID > -1)
    {
        return RetCode;
    }

    //     >>>>???<<<<  May want to change the name's of the EventState, EventSourceName
    // pair to something more general so that it prevents all call backs from being
    // answered, and not just Events.
    EventState = Searching;
    EventSourceName = NULL;

    // Generate a name for this event.
    wcscpy(NewName,pStrItemName);
    wcscat(NewName,L"~");
    wcscat(NewName,pStrSubItemName);
    wcscat(NewName,L"~");
    wcscat(NewName,pStrEventName);

    //Tell the system what we are calling ourself.
    //This matches the name that we store with the DispID,
    //however, we must remember to insure the global flag is
    //set before returning this the DispID in GetIDsOfNames().
    *pBstrName = SysAllocString(NewName);

    RexxRoutineObject routine;

    // convert to an executable entity.  The creatRoutine() method also handles any error conditions.
    if (createRoutine(pStrCode, ulStartingLineNumber, routine) == 0)
    {
        RetCode = BuildRCB(RCB::AddScriptlet, NewName, dwFlags, ulStartingLineNumber, routine, &CodeBlock);
        if (SUCCEEDED(RetCode))
        {
            FPRINTF2(logfile,"successfully created codeblock %p for AddScriptlet\n",CodeBlock);
        }
    }

    if (FAILED(RetCode))
    {
        return RetCode;
    }

    RetCode = DispID.AddDispID(NewName,dwFlags,DID::ASEvent,CodeBlock,&EventSinkDispID);
    if (FAILED(RetCode))
    {
        FPRINTF2(logfile,"A DispID for \"%S\" could not be created.  HRESULT = %08x\n",NewName,RetCode);
    }

    if (SUCCEEDED(RetCode))
    {
        RetCode = Events->AddEvent(pStrItemName,pStrSubItemName,pStrEventName,EventSinkDispID);
    }
    FPRINTF2(logfile,"The Name is \"%S\".  HRESULT = %08x\n\n\n",NewName,RetCode);
    if (SUCCEEDED(RetCode) && engineState == SCRIPTSTATE_CONNECTED)
    {
        RetCode = Events->ConnectEvents();
    }
    FPRINTF2(logfile,"AddScriptlet end.  The final exit code is HRESULT = %08x\n\n\n",RetCode);
    EventState = Abstain;
    EventSourceName = NULL;
    return RetCode;
}




/******************************************************************************
*  IActiveScriptParseProcedure interface -- This interface allows an Active
*  Script Host to use IDispatch-style function pointers to fire methods instead
*  of using the more difficult method of Connection Points.
******************************************************************************/

/******************************************************************************
*  ParseProcedureText -- This method allows an Active Script Host to use
*  IDispatch-style function pointers to fire methods instead of using the more
*  difficult method of Connection Points.  It parses a scriplet and wraps it in
*  an anonymous IDispatch interface, which the host can use in lieu of
*  Connection Points to handle events.
*
*  Parameters: Code                -- Address of the script code to evaluate
*              FormalParams        -- Address of any formal parameters to the
*                                     scriptlet. (ignored)
*              ProcedureName       -- Name of the event
*              ItemName            -- Address of the named item that gives this
*                                     scriptlet its context.
*              DbgIUnknown         -- Address of the context object.  This item is
*                                     reserved for the debugger.
*              Delimiter           -- Address of the delimiter the host used to detect
*                                     the end of the scriptlet.
*              SourceContextCookie -- Application defined value for debugging
*              StartingLineNumber  -- zero-based number defining where parsing
*                                     began.
*              Flags               -- SCRIPTPROC_HOSTMANAGESSOURCE
*                                     SCRIPTPROC_IMPLICIT_THIS
*                                     SCRIPTPROC_IMPLICIT_PARENTS
*                                     SCRIPTPROC_ALL_FLAGS
*              pbOurIDispatch      -- Address of the pointer that receives the IDispatch
*                                     pointer the host uses to call this event.
*  Returns: S_OK
*           DISP_E_EXCEPTION
*           E_INVALIDARG
*           E_POINTER
*           E_NOTIMPL
*           E_UNEXPECTED
*           OLESCRIPT_E_SYNTAX
******************************************************************************/
STDMETHODIMP OrxScript::ParseProcedureText(
  /* in  */ LPCOLESTR Code,
  /* in  */ LPCOLESTR FormalParams,     /* What little doc we have says this is not used.  */
  /* in  */ LPCOLESTR ProcedureName,
  /* in  */ LPCOLESTR ItemName,
  /* in  */ IUnknown *DbgIUnknown,
  /* in  */ LPCOLESTR Delimiter,
  /* in  */ DWORD SourceContextCookie,
  /* in  */ ULONG StartingLineNumber,
  /* in  */ DWORD Flags,
  /* out */ IDispatch **pbOurIDispatch)
{
    HRESULT    RetCode=S_OK;
    OLECHAR    NewName[MAX_PATH];
    DISPID     EventSinkDispID;            // The numeric value this automates under.

    RexxObjectPtr method=NULL;
    HANDLE     execution;
    UINT       dummy;
    PRCB       CodeBlock;
    RexxConditionData cd;
    OrxScriptError *ErrObj;
    bool        ErrObj_Exists;



    FPRINTF3(logfile,"\n->\n");
    FPRINTF(logfile,"OrxScript::ParseProcedureText (ItemName = \"%S\", SourceContextCookie = %08x,\n",ItemName,SourceContextCookie);
    FPRINTF2(logfile,"Delimiter = \"%S\", DbgIUnknown = %p, StartingLineNumber = %u,",DbgIUnknown,Delimiter,StartingLineNumber);
    FPRINTF2(logfile,"Flags = %08x, FormalParams \"%S\", ProcedureName \"%S\" )\n",Flags,FormalParams,ProcedureName);
    FPRINTF2(logfile,"In english the dwFlags signifies:\n");
    FPRINTF2(logfile,"%s\n",FlagMeaning('H',Flags,ScriptProc));
    FPRINTF3(logfile,">>>>>>> START OF CODE next line:\n%S\n<<<<<<<END OF CODE\n",Code);

    *pbOurIDispatch = NULL;
    RetCode = Events->CheckEvent(Code, pbOurIDispatch);
    if (RetCode != DISP_E_UNKNOWNNAME)
    {
        FPRINTF2(logfile,"ParseProcedure ended.  Possible event re-use.  The final exit code is HRESULT = %08x\n",RetCode);
        return RetCode;
    }

    // Generate a name for this event.
    _snwprintf(NewName, sizeof(NewName), L"#Event-E%03d", ++EventCount);

    // convert to an executable entity.  The creatRoutine() method also handles any error conditions.
    if (createRoutine(Code, StartingLineNumber, routine) == 0)
    {
        RetCode = BuildRCB(RCB::ParseProcedure, NewName, Flags, StartingLineNumber, routine, &CodeBlock);
        if (SUCCEEDED(RetCode))
        {
            FPRINTF2(logfile,"successfully created codeblock %p for ParseProcedureText\n",CodeBlock);
        }
    }
    if (FAILED(RetCode))
    {
        return RetCode;
    }

    RetCode = DispID.AddDispID(NewName,Flags,DID::PPEvent,CodeBlock,&EventSinkDispID);
    if (SUCCEEDED(RetCode))
    {
        RetCode = Events->AddEvent(NewName,Code,EventSinkDispID,Flags,pbOurIDispatch);
        if (FAILED(RetCode))
        {
            FPRINTF2(logfile,"An IDispatch for Source \"%S\" could not be created.  HRESULT = %08x\n",NewName,RetCode);
        }
        else
        {
            FPRINTF2(logfile,"The IDispatch, %p, for DispID %d has been created\n",*pbOurIDispatch,(int)EventSinkDispID);
        }
    }
    else
    {
        FPRINTF2(logfile,"A DispID for \"%S\" could not be created.  HRESULT = %08x\n",NewName,RetCode);
    }
    FPRINTF2(logfile,"ParseProcedure end.  The Name is \"%S\".  The final exit code is HRESULT = %08x\n",NewName,RetCode);
    return RetCode;
}


STDMETHODIMP OrxScript::LocalParseProcedureText(
  /* in  */ const char *Name,
  /* in  */ DWORD       Flags,
  /* out */ IDispatch **pbOurIDispatch)
{
    HRESULT    RetCode=S_OK;
    OLECHAR    NewName[MAX_PATH];
    DISPID     EventSinkDispID;            // The numeric value this automates under.
    PRCB       CodeBlock;
    ULONG      StartingLineNumber=1;
    OLECHAR    Code[2048];
    OLECHAR    wName[2048];


    if (pbOurIDispatch == NULL)
    {
        return E_POINTER;
    }
    _snwprintf(&Code[0], sizeof(Code), L"Call %S", Name);
    *pbOurIDispatch = NULL;
    RetCode = Events->CheckEvent(Code, pbOurIDispatch);
    if (RetCode != DISP_E_UNKNOWNNAME)
    {
        FPRINTF2(logfile,"LocalParseProcedure ended.  Possible event re-use.  The final exit code is HRESULT = %08x\n",RetCode);
        return RetCode;
    }

    // Generate a name for this event.
    _snwprintf(NewName, sizeof(NewName), L"#Event-E%03d", ++EventCount);
    CodeBlock = (PRCB) RexxFunctions->FindContent(Name);

    _snwprintf(&wName[0], sizeof(wName), L"%S", Name);
    RetCode = DispID.AddDispID(wName,Flags,DID::LPPEvent,CodeBlock,&EventSinkDispID);
    if (SUCCEEDED(RetCode))
    {
        RetCode = Events->AddEvent(NewName,Code,EventSinkDispID,Flags,pbOurIDispatch);
        if (FAILED(RetCode))
        {
            FPRINTF2(logfile,"An IDispatch for Source \"%S\" could not be created.  HRESULT = %08x\n",NewName,RetCode);
        }
        else
        {
            FPRINTF2(logfile,"The IDispatch, %p, for DispID %d has been created\n",*pbOurIDispatch,(int)EventSinkDispID);
        }
    }
    else
    {
        FPRINTF2(logfile,"A DispID for \"%S\" could not be created.  HRESULT = %08x\n",NewName,RetCode);
    }
    FPRINTF2(logfile,"LocalParseProcedure end.  The Name is \"%S\".  The final exit code is HRESULT = %08x\n",NewName,RetCode);
    return RetCode;
}





STDMETHODIMP OrxScript::ParseScriptText(LPCOLESTR  pStrCode,
                                        LPCOLESTR  pStrItemName,
                                        IUnknown  *pUnkContext,
                                        LPCOLESTR  pStrDelimiter,
                                        DWORD      dwSourceContextCookie,
                                        ULONG      ulStartingLineNumber,
                                        DWORD      dwFlags,
                                        VARIANT   *pVarResult,
                                        EXCEPINFO *pExcepInfo)
{
    HRESULT hResult = S_OK;
    HANDLE     execution;
    DISPID     lDispID;
    UINT       dummy;
    RexxObjectPtr resultDummy;
    int        result;
    int        i;
    LinkedList tempNames;
    ListItem  *item;
    OLECHAR    lName[MAX_PATH];
    PRCB       CodeBlock;
    OrxScriptError *ErrObj;
    bool        ErrObj_Exists;


    FPRINTF3(logfile,"\n");
    FPRINTF(logfile,"OrxScript::ParseScriptText (StrItemName = \"%S\", dwSourceContextCookie = %08x,\n",pStrItemName,dwSourceContextCookie);
    FPRINTF2(logfile,"StrDelimiter = \"%S\", pUnkContext = %p, ulStartingLineNumber = %u,\n",pUnkContext,pStrDelimiter,ulStartingLineNumber);
    FPRINTF2(logfile,"dwFlags = %08x, pVarResult = %p, pExcepInfo = %p)\n",dwFlags,pVarResult,pExcepInfo);
    FPRINTF2(logfile,"In english the dwFlags signifies:\n");
    FPRINTF2(logfile,"%s\n",FlagMeaning('H',dwFlags,ScriptText));
    FPRINTF3(logfile,">>>>>>> START OF CODE next line:\n%S\n<<<<<<<END OF CODE\n",pStrCode);

    ParseProcedureTextDispatcher dispatcher(this, pStrCode, ulStartingLineNumber);

    // go process this on a separate thread
    if (dispatcher.invoke() != 0) {
        hResult = S_ERR;
    }

    pActiveScriptSite->OnLeaveScript();
    LeaveCriticalSection(&EngineSection);
    FPRINTF2(logfile,"done with ParseScriptText\n");
    return hResult;
}



/****************************************/
/* implementation of IPersist interface */
/*                           complete - */
/****************************************/

STDMETHODIMP OrxScript::GetClassID(LPCLSID pClassID)
{
    char    cIID[40];
    strcpy(cIID,"{?}  It wants OUR CLSID!!!!");
    FPRINTF(logfile,"OrxScript::GetClassID (ClassID = %s \n",cIID);

    // i think we do not want just say okay if we don't provide anything
    // in pClassID!!!

    return S_OK;
}



/**************************************************/
/* implementation if IPersistStreamInit interface */
/*                                     complete - */
/**************************************************/

STDMETHODIMP OrxScript::IsDirty()
{
    HRESULT hResult = S_OK;
    FPRINTF(logfile,"OrxScript::IsDirty\n");
    return hResult;
}

STDMETHODIMP OrxScript::Load(LPSTREAM pStream)
{
    HRESULT hResult = S_OK;

    FPRINTF(logfile,"OrxScript::Load (pStream = %p)\n",pStream);
//  CHECK_BASE_THREAD;    // can only be called within the same thread

    // !: "load"
    // if success:
    initNew = true;

    // according to MSDN "Windows Script Engines" this causes
    // the engine to transit from UNINITIALIZED to INITIALIZED if
    // the script site is set
    if (pActiveScriptSite && engineState == SCRIPTSTATE_UNINITIALIZED)
        SetScriptState(SCRIPTSTATE_INITIALIZED);

    return hResult;
}

STDMETHODIMP OrxScript::Save(LPSTREAM pStream, BOOL fClearDirty)
{
    FPRINTF(logfile,"OrxScript::Save (pStream = %p, fClearDirty = %s)\n",pStream,fClearDirty?"true":"false");
    return S_OK;
}

STDMETHODIMP OrxScript::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    HRESULT hResult = S_OK;
    FPRINTF(logfile,"OrxScript::GetSizeMax (pcbSize = %p)\n",pcbSize);
    return hResult;
}



/*********************************************/
/* implementation of IObjectSafety interface */
/*                                complete - */
/*********************************************/
STDMETHODIMP OrxScript::GetInterfaceSafetyOptions(REFIID iid,
                                                  DWORD *pdwSupportedOptions,
                                                  DWORD *pdwEnabledOptions)
{
    HRESULT hResult = S_OK;


    OLECHAR   cIID[100];
    char     *IIDName,TrulyUnknown[]="??????";

    StringFromGUID2(iid, cIID, sizeof(cIID));
    if (!(IIDName = NameThatInterface((OLECHAR *)&cIID[0]))) IIDName = &TrulyUnknown[0];
    FPRINTF(logfile,"OrxScript::GetInterfaceSafetyOptions() - Start\n");
    FPRINTF2(logfile,"riid = %S (%s)\n",cIID,IIDName);
    FPRINTF2(logfile,"pdwSupportedOptions = %p \n",pdwSupportedOptions);
    FPRINTF2(logfile,"pdwEnabledOptions   = %p\n", pdwEnabledOptions);
    if (IIDName != &TrulyUnknown[0])
    {
        free(IIDName);
    }

    if (IsEqualIID(iid, IID_IActiveScript) ||
        IsEqualIID(iid, IID_IActiveScriptParse) ||
        //  >>>>> ??? <<<<<  Should the IDispatchEx be uncommented now?
        // IsEqualIID(iid, IID_IDispatchEx) ||
        IsEqualIID(iid, IID_IPersist) ||
        IsEqualIID(iid, IID_IPersistStreamInit) ||
        IsEqualIID(iid, IID_IActiveScriptParse))
    {
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA |
                               INTERFACESAFE_FOR_UNTRUSTED_CALLER |
                               INTERFACE_USES_DISPEX |
                               INTERFACE_USES_SECURITY_MANAGER;
        *pdwEnabledOptions = dwSafetyOptions;
    }
    else
    {
        FPRINTF2(logfile,"OrxScript::GetInterfaceSafetyOptions >>> unsupported Interface\n");
        hResult = E_NOINTERFACE;
    } /* endif IsEqualIID(...  */

    FPRINTF2(logfile,"OrxScript::GetInterfaceSafetyOptions() - Leaving HRESULT %08x\n",hResult);
    return hResult;
}



STDMETHODIMP OrxScript::SetInterfaceSafetyOptions(REFIID iid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{
    HRESULT hResult = S_OK;


    OLECHAR  cIID[100];
    char    *IIDName,TrulyUnknown[]="??????";

    // _asm int 3
    StringFromGUID2(iid, cIID, sizeof(cIID));
    if (!(IIDName = NameThatInterface((OLECHAR *)&cIID[0])))
    {
        IIDName = &TrulyUnknown[0];
    }
    FPRINTF(logfile,"OrxScript::SetInterfaceSafetyOptions() - Start\n");
    FPRINTF2(logfile,"riid = %S (%s)\n",cIID,IIDName);
    FPRINTF2(logfile,"dwOptionSetMask %d \n",dwOptionSetMask);
    FPRINTF2(logfile,"dwOptionSetMask (%08x) to support: %s\n",dwOptionSetMask,
             FlagMeaning('H',dwOptionSetMask,InterfaceSafetyOptions));
    FPRINTF2(logfile,"dwEnabledOptions (%08x) to support: %s\n",dwEnabledOptions,
             FlagMeaning('H',dwEnabledOptions,InterfaceSafetyOptions));
    if (IIDName != &TrulyUnknown[0])
    {
        free(IIDName);
    }
    //  >>>>> ??? <<<<<  Find out what "Uses Security Manager" means to microsoft

    if (IsEqualIID(iid, IID_IActiveScript) ||
        IsEqualIID(iid, IID_IActiveScriptParse) ||
        //  >>>>> ??? <<<<<  Should the IDispatchEx be uncommented now?
        // IsEqualIID(iid, IID_IDispatchEx) ||
        IsEqualIID(iid, IID_IPersist) ||
        IsEqualIID(iid, IID_IPersistStreamInit) ||
        IsEqualIID(iid, IID_IActiveScriptParse))
    {
        // just for fun: we say we don't know DISPEX /*| SECURITY_MANAGER*/
        // (even though we said in GetInterfaceSafetyOptions() we support it...)
//    if ( (dwOptionSetMask & INTERFACE_USES_DISPEX) |
//         (dwOptionSetMask & INTERFACE_USES_SECURITY_MANAGER) ) hResult=E_FAIL;
        // we ignore INTERFACE_USES_DISPEX...
        // deal with INTERFACE_USES_SECURITY_MANAGER on setSite...
        dwSafetyOptions &= ~dwOptionSetMask; // clear out old bits
        dwSafetyOptions |= dwEnabledOptions; // set new bits...
    }
    else
    {
        FPRINTF2(logfile,"OrxScript::SetInterfaceSafetyOptions >>> unsupported Interface\n");
        hResult = E_NOINTERFACE;
    }

    securityObject = createSecurityObject();

    FPRINTF2(logfile,"OrxScript::SetInterfaceSafetyOptions() - Leaving HRESULT %08x\n",hResult);
    return hResult;
}




/********************************************************************/
/*                                                                  */
/*   BuildRCB - Build Rexx Code Block                               */
/*                                                                  */
/*   Once a scriptlet has been tokenized, this builds the block     */
/*  to store it on the DispID chain in.  This contains a pointer    */
/*  to the tokenized code, the flags the Host passed with the       */
/*  code, the starting line number, and the "Item Name" the         */
/*  Host thinks the code belongs to.                                */
/*                                                                  */
/********************************************************************/
STDMETHODIMP OrxScript::BuildRCB(
    /* [in]  */ RCB::EntryType EntrySource,
    /* [in]  */ OLECHAR    *Name,
    /* [in]  */ DWORD       Flags,
    /* [in]  */ ULONG       StartingLN,
    /* [in]  */ RexxObjectPtr  Code,
    /* [out] */ PRCB       *CodeBlock)
{
    size_t     NameLen,BlockLen;
    PRCB       Block;
    OLECHAR   *lName;
    char       SB_NewName[MAX_PATH],*SBName=NULL;       // single byte
    ListItem  *NewItem;


    *CodeBlock = NULL;
    if (Name)
    {
        NameLen = wcslen(Name);
    }
    else
    {
        NameLen = 0;
    }
    BlockLen = sizeof(RCB) + (sizeof(OLECHAR)*(NameLen+1));
    Block = (PRCB) GlobalAlloc(GMEM_FIXED,BlockLen);
    if (!Block)
    {
        return E_FAIL; // serious error, could not generate code block
    }
    lName = (OLECHAR *)((char *)Block + sizeof(RCB));
    if (Name)
    {
        wcscpy(lName,Name);
    }
    else
    {
        lName[0] = L'\0';
    }
    Block->EntrySource = EntrySource;
    Block->Name = lName;
    Block->Flags = Flags;
    Block->StartingLN = StartingLN;
    Block->Code = Code;
    if (Name)
    {
        sprintf(SB_NewName,"%S",Name);
        SBName = &SB_NewName[0];
    }
    NewItem = RexxCodeList->AddItem(SBName,LinkedList::Beginning,(void*) Block);
    if (!NewItem)
    {
        GlobalFree(Block);
        return E_FAIL; // serious error, could not generate code block
    }

    *CodeBlock = Block;
    return S_OK;
}




/********************************************************************/
/*                                                                  */
/*   This supports the Event handling interface by obtaining the    */
/* IDispatch of ItemName.SubItemName, unless SubItemName is NULL,   */
/* then the IDispatch of ItemName is returned.                      */
/*                                                                  */
/*   The IDispatch that is obtained is used to obtain the TypeInfo  */
/* and DispIDs that the Event will use when calling us.  See        */
/* OrxASEvent.hpp for a brief description of the Event system.      */
/*                                                                  */
/********************************************************************/
STDMETHODIMP OrxScript::GetSourceIDispatch(LPCOLESTR   ItemName,
                                           LPCOLESTR   SubItemName,
                                           IDispatch **Source)
{
    HRESULT    RetCode;
    IUnknown  *Unk;
    IDispatch *Disp,*Items;
    VARIANT    RetInfo;

    FPRINTF2(logfile,"OrxScript::GetSourceIDispatch()  \n");
    *Source = NULL;

    if (ItemName == NULL)
    {
        return E_INVALIDARG; // That would ruin our day.
    }

    //  The cast is to remove the const attribute, otherwise the assignment cannot be made, on the
    // possible chance that we might change a string that this routine is promising
    // will be constant, never touched.
    EventSourceName = (OLECHAR *)ItemName;
    if (FAILED(RetCode = pActiveScriptSite->GetItemInfo(ItemName, SCRIPTINFO_IUNKNOWN,&Unk,NULL)))
    {
        Unk = NULL;
    }
    if (Unk)
    {
        //  Get the IDispatch for the main name.
        FPRINTF2(logfile,"Debug: OrxScript::GetSourceIDispatch()  -- The IUnknown %p.\n",Unk);
        RetCode = Unk->QueryInterface(IID_IDispatch, (void **)&Disp);
        FPRINTF2(logfile,"Debug: OrxScript::GetSourceIDispatch()  back from QI IUnknown %p IDispatch %p HRESULT %08x\n",Unk,Disp,RetCode);
        Unk->Release();
        Unk = NULL;
        Items = Disp;

        if (SUCCEEDED(RetCode))
        {
            //  If the SubItemName is NULL, we are done.
            //  If it is not, then get its IDispatch via Property Get from the main
            // name's IDispatch.
            if (SubItemName != NULL)
            {
                // See if we are known about.
                EventSourceName = (OLECHAR *)SubItemName;
                FPRINTF2(logfile,"OrxScript::GetSourceIDispatch()  -- GetIDsOfNames for the SubItem using IDispatch %p.\n", Disp);

                RetCode = GetProperty(logfile, Disp, SubItemName, Lang, &RetInfo);
                if (SUCCEEDED(RetCode) && (RetInfo.vt == VT_DISPATCH))
                {
                    if (RetInfo.pdispVal != this)
                    {
                        Disp = RetInfo.pdispVal;

                        // This is what happens when two Microsoft "technologies" collide.  The GetProperty
                        // causes an AddRef(), however, when the VariantClear() is called it will notice
                        // that this is an IDispatch, and do a Release() on our behalf.  So, we must
                        // make sure that this pointer stays around.
                        Disp->AddRef();
                    }
                    else
                    {
                        FPRINTF2(logfile,"Debug: Whoa!! Do NOT set this to ourselves!\n");
                    }
                }
                VariantClear(&RetInfo);
            }  // SubItemName != NULL
            *Source = Disp;
            Items->Release();           // We are done with the ItemName's IDispatch.
        }  // Attempt to get main name's IDispatch
    }  // Attempt to get the main name's IUnknown
    if (Items == Disp)
    {
        FPRINTF2(logfile,"Debug:  -- The net effect is that the IDispatch is \"%S\", not \"%S\" .\n",ItemName,SubItemName);
    }
    FPRINTF2(logfile,"OrxScript::GetSourceIDispatch()  -- Exiting Source: %p HRESULT %08x.\n",*Source,RetCode);
    return RetCode;
}


void OrxScript::insertVariable(RexxThreadContext *context, const char *varName, RexxObjectPtr varValue)
{
    PGVARIANT   temp;
    HRESULT     RetCode;
    DISPID      DispID;
    DWORD       Flags;
    IDispatch  *Dispatch;
    ITypeInfo  *TypeInfo;
    ListItem   *LI;


    //   Stop built-in Rexx names from being added.
    if (stricmp(varName,"RESULT") == 0) return;


    // Once on a list is enough.
    LI = PropertyList.FindItem(varName);
    if (LI)
    {
        return;
    }

    //  Unfortunately, this cannot be done in RexxRetrieveVariable(), since it is in a different thread.
    RetCode = NamedItemList->WhoKnows(varName,Lang,&DispID,&Flags,&Dispatch,&TypeInfo);
    if (SUCCEEDED(RetCode))
    {
        return;       // Doesn't matter who knows this, don't save it.  (Or do we want this if it is an ITypeInfo?)
    }
    FPRINTF(logfile,"Make known the variable %s (RexxObject %p)\n",varName,varValue);
    // convert to variant, store it (WinEnterKernel(false) might be needed)
    temp = new GVARIANT;
    VariantInit(&(temp->Mutant));
    Rexx2Variant(context, varValue, &(temp->Mutant), VT_EMPTY, 0);
    PropertyList.AddItem(const_cast<char *>(varName),LinkedList::End,(void *)temp);
}
