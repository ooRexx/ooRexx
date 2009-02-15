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
/******************************************************************************
*
*
*
*
*
******************************************************************************/
#include "orxEvents.hpp"

#define EVENTNAMECONVERT(Item,SubItem,OurName) \
  sprintf(OurName,"%S<-Item (AddScriptlet Event) SubItem->%S",Item,SubItem);

//  The values for these FL (Flag List) arrays are defined in OrxIDispatch.cpp
extern FL DispatchInvoke[];


extern FL DispExGetDispID[];

FL ImplementedInterface[] = {           // H - orizontal flags
  {IMPLTYPEFLAG_FDEFAULT,"Default"},
  {IMPLTYPEFLAG_FSOURCE,"Source"},
  {IMPLTYPEFLAG_FRESTRICTED,"Restricted"},
  {IMPLTYPEFLAG_FDEFAULTVTABLE,"Default VTable"},
  {(DWORD)0,(char *)NULL}
  };






/*
 *    This creates an empty event chain, and binds it to the engine that created it.
 *  The pointer to the logfile of the engine is also passed.  In this manner the
 *  events can add to the log without requiring access to private properties of the
 *  engine.
 */
OrxEvent::OrxEvent(OrxScript *Script, FILE *pLogFile) : logfile(pLogFile),
                                                        Engine(Script)
{


    EventSourceChain = new LooseLinkedList;
}


OrxEvent::~OrxEvent()
{
    ListItem    *Current;
    ESource     *Content;

    FPRINTF2(logfile,"OrxEvent::~OrxEvent()   --   i.e., the destructor. \n");
    DisconnectEvents();

    Current = EventSourceChain->FindItem(0);
    while (Current)
    {
        Content = (ESource *)Current->GetContent();
        if (Content->EventType == ESource::AddScriptlet)
        {
            Content->Release();
        }
        Current = EventSourceChain->FindItem();
    }
    delete EventSourceChain;
    FPRINTF2(logfile,"OrxEvent::~OrxEvent()   completed. \n");

}



/*
 */
STDMETHODIMP OrxEvent::CheckEvent(LPCOLESTR Code, IDispatch **pbIDispatch)
{
    ListItem    *Current;
    ESource     *Content;
    HRESULT      RetCode=DISP_E_UNKNOWNNAME;
    int          i=0;
    size_t       CodeLen;
    RetCode = DISP_E_UNKNOWNNAME;


    if (!Code)
    {
        RetCode = E_POINTER;
    }
    else
    {
        CodeLen = wcslen(Code);
    }
    if (!pbIDispatch)
    {
        RetCode = E_POINTER;
    }
    else
    {
        *pbIDispatch = (IDispatch *)NULL;
    }
    if (RetCode == DISP_E_UNKNOWNNAME)
    {
        while (Current = EventSourceChain->FindItem(i++))
        {
            Content = (ESource *)Current->GetContent();
            if (Content->EventType == ESource::ParseProcedure)
            {
                if (CodeLen == Content->CodeLen)
                {
                    if (Content->Code)
                    {
                        if (wcsicmp(Code,Content->Code) == 0)
                        {
                            *pbIDispatch = (IDispatch *)Content;
                            RetCode = S_OK;
                            ((IDispatch*) Content)->AddRef();
                            break;
                        }
                    }
                }
            }
        }
    }
    FPRINTF2(logfile,"OrxEvent::CheckEvent() HRESULT %08x. \n",RetCode);
    return RetCode;
}



/*
 *    This is the AddEvent() for ParseProcedure().  ParseProcedure() events always get called
 *  with a DispID of 0, therefore, these events are always treated as an individual container.
 */
STDMETHODIMP OrxEvent::AddEvent(OLECHAR *pName, LPCOLESTR Code, DISPID SinkDispID,DWORD Flags, IDispatch **pbIDispatch)
{
    ESource  *Current;
    ListItem *Next;
    HRESULT  RetCode=S_OK;
    char     Name[MAX_PATH];

    FPRINTF2(logfile,"OrxEvent::AddEvent() SinkDispID %d. \n",(int)SinkDispID);

    *pbIDispatch = (IDispatch *)NULL;
    Current = new ESource;
    if (Current == NULL)
    {
        RetCode = E_OUTOFMEMORY;
    }
    else
    {
        RetCode = Current->InitEvent(pName,Code,SinkDispID,Engine,logfile);
        if (SUCCEEDED(RetCode))
        {
            W2C(&Name[0], pName, (int)wcslen(pName)+1);
            Next = EventSourceChain->AddItem(Name,LinkedList::End,(void *)Current);
            //  Make sure the Event can tell people if it goes away prematurely.
            Current->SetDestructor(EventSourceChain, (void *)Current);
        }
        else
        {
            Next = NULL;
        }
        if (Next)
        {
            *pbIDispatch = (IDispatch *)Current;
        }
        else
        {
            delete Current;
            RetCode = SUCCEEDED(RetCode) ? E_OUTOFMEMORY : RetCode;
        }
    }
    FPRINTF2(logfile,"OrxEvent::AddEvent() HRESULT %08x. \n",RetCode);
    return RetCode;
}




STDMETHODIMP ESource::InitEvent(OLECHAR *Name,
                 LPCOLESTR  pCode,
                 DISPID     SinkDispID,
                 OrxScript *ORexxScript,
                 FILE      *LogFile)
{
    PEMAP      NewMap;
    HRESULT    RetCode;

    EventType = ParseProcedure;
    EventName = SysAllocString(Name);
    Engine = ORexxScript;
    Connected = true;
    ConnectionPoint = NULL;
    Container = NULL;
    logfile = LogFile;
    CodeLen = (int)wcslen(pCode);
    Code = (OLECHAR *) GlobalAlloc(GMEM_FIXED,(CodeLen+1)*sizeof(OLECHAR));
    if (!Code)
    {
        FPRINTF2(logfile," ParseProcedure Event ERROR Failed to initialize.  Out of memory.\n");
        return E_OUTOFMEMORY;
    }
    memcpy(Code,pCode,(CodeLen+1)*sizeof(OLECHAR));

    RetCode = AddMap("0",&NewMap);
    if (FAILED(RetCode))
    {
        GlobalFree(Code);
        Code = NULL;
        FPRINTF2(logfile," ParseProcedure Event ERROR Failed to initialize.  AddMap Failed.\n");
        return RetCode;
    }
    NewMap->SourceEventName = SysAllocString(Name);
    NewMap->Sink = SinkDispID;

    FPRINTF2(logfile,"created a new ParseProcedure Event Source. %p\n",this);
    FPRINTF2(DLLlogfile,"created a new Event Source.%p\n",this);
    return S_OK;
}


STDMETHODIMP OrxEvent::AddEvent(LPCOLESTR  ItemName,
                                LPCOLESTR  SubItemName,
                                LPCOLESTR  EventName,
                                DISPID     SinkDispID)
{
    ESource   *Current;
    IDispatch *Source;
    char       Name[2*MAX_PATH];
    HRESULT    RetCode=S_OK;


    FPRINTF2(logfile,"OrxEvent::AddEvent() SinkDispID %d. \n",(int)SinkDispID);

    sprintf(Name,"%S<-Item (AddScriptlet Event) SubItem->%S",ItemName,SubItemName);
    Current = (ESource *)EventSourceChain->FindContent(Name);
    if (!Current)
    {
        RetCode = Engine->GetSourceIDispatch(ItemName,SubItemName,&Source);
        if (RetCode == S_OK)
        {
            Current = new ESource;
            if (Current == NULL)
            {
                RetCode = E_OUTOFMEMORY;
            }
            else
            {
                RetCode = Current->InitEvent(Source,Engine,logfile);
                EventSourceChain->AddItem(Name,LinkedList::End,(void *)Current);
                //  Make sure the Event can tell people if it goes away prematurely.
                Current->SetDestructor(EventSourceChain, (void *)Current);
            }
        }
        else
        {
            FPRINTF2(logfile,"OrxEvent::AddEvent() An IDispatch for Source \"%s\" could not be created.  HRESULT = %08x\n",Name,RetCode);
        }
    }
    else
    {
        FPRINTF2(logfile,"OrxEvent::AddEvent() the event, \"%s\", has been defined. Current is %p. \n",Name,Current);
    }
    // The InitEvent() uses the MS terminology "function" for event name.
    //  Find the function that is identified by EventFunction, and map it to SinkDispID
    if (Current)
    {
        RetCode = Current->SetMap(EventName, SinkDispID);
    }
    FPRINTF2(logfile,"OrxEvent::AddEvent() Current %p HRESULT %08x \n",Current,RetCode);
    return RetCode;
}



STDMETHODIMP ESource::InitEvent(IDispatch *SourceDispatch,
                 OrxScript *ORexxScript,
                 FILE *LogFile)
{
    ITypeInfo *SourceType;
    TYPEATTR  *TypeAttributes;
    BSTR       SourceName;
    unsigned int NameCount;
    int        i;
    FUNCDESC  *FuncDesc;
    char       DispIDName[29];
    PEMAP      NewMap;
    HRESULT    RetCode=S_OK;
    int        EMCount;

    FPRINTF2(LogFile,"created a new Event Source. %p\n",this);
    FPRINTF2(DLLlogfile,"created a new Event Source.%p\n",this);
    EventType = AddScriptlet;
    Source = SourceDispatch;   // Mimick the ParseProcedures "THIS" parameter by returning this pointer.
    Engine = ORexxScript;
    Connected = false;
    ConnectionPoint = NULL;
    Container = NULL;
    logfile = LogFile;

    RetCode = GetTypeInfo(&SourceType);
    if (SUCCEEDED(RetCode))
    {
        RetCode = SourceType->GetTypeAttr(&TypeAttributes);
        memcpy(&SourceGUID,&TypeAttributes->guid,sizeof(GUID));
        EMCount = TypeAttributes->cFuncs;
        SourceType->ReleaseTypeAttr(TypeAttributes);
        OLECHAR    lGUID[50];
        StringFromGUID2(SourceGUID,lGUID,sizeof(lGUID));
        FPRINTF2(logfile,"The GUID is %S and there are %d functions.\n",lGUID,EMCount);

        /*    For each entry in the type library, create an entry on the Event Map chain.
         *  This is a many to one relation.  Each of the different Source Disp ID's
         *  will translate to the same Sink Disp ID.  There is only one chunk of code
         *  being bound to this Event that the Type Library is describing.  So every
         *  Source call must map to the same Sink.
         */
        for (i=0; i<EMCount; i++)
        {
            SourceType->GetFuncDesc(i, &FuncDesc);
            //  Despite what the documentation says, this returns Max Names, not Max Names - 1.
            // The first name is the function name, the remainder if they exist are parameters.
            SourceType->GetNames(FuncDesc->memid, &SourceName, 1, &NameCount);
            sprintf(DispIDName,"%d",FuncDesc->memid);
            //  This creates the entry for the function with an invalid DispID to call.
            RetCode = AddMap(DispIDName,&NewMap);
            if (FAILED(RetCode)) return RetCode;
            FPRINTF2(logfile,"ESource::InitEvent - AddScriptlet \"%S\" \n",SourceName);
            NewMap->SourceEventName = SourceName;
            SourceType->ReleaseFuncDesc(FuncDesc);
        }

        SourceType->Release();
    }
    else
    {
        FPRINTF2(logfile,"Could not obtain TypInfo for this event! HRESULT %08x\n",RetCode);
    }
    return RetCode;
}


/******************************************************************************
*
* returns
*             RetCode           SinkDispID     Meaning
*           ---------           ----------
*  DISP_E_UNKNOWNNAME                   -1     Events for this object have not been
*                                              defined before.
*                S_OK                   -1     This object has events, just not this one.
*                S_OK                   >0     This object has this event.  The Container
*                                              will call us using the value in SinkDispID
*                                              as the identifier.
*
******************************************************************************/
STDMETHODIMP OrxEvent::FindEvent(LPCOLESTR  ItemName,
                                 LPCOLESTR  SubItemName,
                                 LPCOLESTR  EventName,
                                 DISPID     *SinkDispID)
{
    ESource   *Current;
    char       Name[2*MAX_PATH];
    HRESULT    RetCode=DISP_E_UNKNOWNNAME;


    FPRINTF2(logfile,"OrxEvent::QueryEvent() Item \"%S\"  SubItem \"%S\" Event \"%S\". \n",ItemName,SubItemName,EventName);
    *SinkDispID = (DISPID) -1;
    EVENTNAMECONVERT(ItemName,SubItemName,Name)
    // sprintf(Name,"%S<-Item (AddScriptlet Event) SubItem->%S",ItemName,SubItemName);
    Current = (ESource *)EventSourceChain->FindContent(Name);
    if (Current)
    {
        RetCode = Current->FindMap(EventName, SinkDispID);
    }

    return RetCode;
}



STDMETHODIMP OrxEvent::ConnectEvents()
{
    ESource  *Current;
    HRESULT  RetCode=S_OK;
    bool     ConnSomething=false;


    FPRINTF2(logfile,"OrxEvent::ConnectEvents() - Connecting EVENTS. \n");
    Current = (ESource *)EventSourceChain->FindContent(0);
    while (Current != NULL)
    {
        /*  Check the connection flag.  DON'T try to connect something that is already connected.*/
        if (Current->EventType == ESource::AddScriptlet && Current->ConnectionPoint == NULL)
        {
            FPRINTF2(logfile,"OrxEvent::ConnectEvents() - Current %p Current-Source %p. \n",Current,Current->Source);
            RetCode = Current->Source->QueryInterface( IID_IConnectionPointContainer, (void**)&(Current->Container) );


            if (SUCCEEDED(RetCode))
            {
                //Use the interface to get the connection point we want
                RetCode = Current->Container->FindConnectionPoint( Current->SourceGUID, &Current->ConnectionPoint );
                Current->Container->Release();
            }
        }

        if (Current->EventType == ESource::AddScriptlet && SUCCEEDED(RetCode) && !Current->Connected)
        {
            //    Use the connection point to hook up to the event source.
            //  And just to be dogmatic about things, AddRef() the ESource.
            //  We are handing out the ptr to a COM object, to be used as such.
            //  The Unadvise() will Release() it.
            Current->AddRef();
            FPRINTF2(logfile,"OrxEvent::ConnectEvents() - Advise, the connection point is %p. \n",Current->ConnectionPoint);
            Current->ConnectionPoint->Advise( (IUnknown*)Current, &Current->Cookie );

            //  The Event is FINALLY connected!
            Current->Connected = true;
            FPRINTF2(logfile,"OrxEvent::ConnectEvents() - Connected, this event is powered on. \n");
            ConnSomething = true;
        }
        Current = (ESource *)EventSourceChain->FindContent();
    }
    if (RetCode == S_OK && ConnSomething)
    {
        FPRINTF2(logfile,"OrxEvent::ConnectEvents() - This is the real thing kids, we are plugged in! \n");
    }

    return RetCode;
}



STDMETHODIMP OrxEvent::DisconnectEvents()
{
    ESource  *Current;
    HRESULT   RetCode;

    FPRINTF2(logfile,"OrxEvent() Disconnecting Events\n");
    Current = (ESource *)EventSourceChain->FindContent(0);
    while (Current != NULL)
    {
        if (Current->Connected && Current->ConnectionPoint != NULL && Current->EventType == ESource::AddScriptlet)
        {
            RetCode = Current->ConnectionPoint->Unadvise(Current->Cookie);
            Current->Release();
        }
        Current->Connected = false;
        Current = (ESource *)EventSourceChain->FindContent();
    }

    return S_OK;
}


ESource::ESource()
                 : ulRefCount(1),
                   Source(NULL),
                   Engine(NULL),
                   Connected(false),
                   ConnectionPoint(NULL),
                   Container(NULL),
                   InvokeCount(0),
                   EventName(NULL),
                   Code(NULL),
                   CodeLen(0),
                   logfile(NULL)
{
    ;
}




// DTOR
ESource::~ESource()
{
    PEMAP      EventMap;

    FPRINTF2(logfile,"DTOR called for an Event Source %p\n",this);
    FPRINTF2(DLLlogfile,"DTOR called for an Event Source %p\n",this);

    if (EventName)
    {
        SysFreeString(EventName);
    }
    //  Disconnect the old-style of interrupts.
    if (Connected && ConnectionPoint != NULL && EventType == AddScriptlet)
    {
        ConnectionPoint->Unadvise(Cookie);
        ConnectionPoint->Release();
        Release();                         // Decrement our reference to match the unadvise.
    }
    if (EventType == ParseProcedure)
    {
        if (Code)
        {
            GlobalFree(Code);
        }
    }
    Connected = false;
    // Release the IDispatch pointer used to find the ConnectionPoint.
    if (Source)
    {
        Source->Release();
    }
    // Release all of the Event Maps for this Source.
    EventFunctionMap.DeleteList();

    EventMap = (PEMAP)EventFunctionMap.FindContent(0);

    if (EventMap != NULL)
    {
        FPRINTF2(logfile,"~ESource: The EventMap is not empty after DeleteList()\n");
    }

    FPRINTF2(logfile,"Leaving Event Source DTOR \n");
    FPRINTF2(DLLlogfile,"Leaving Event Source DTOR \n");
}


/****************************************/
/* implementation of IUnknown interface */
/*                           complete + */
/****************************************/

/*************************************************************/
/* ESource::QueryInterface                                   */
/*                                                           */
/* return an interface pointer of the requested IID.         */
/* this means simply casting "this" to the desired interface */
/* pointer, if the interface is supported.                   */
/*************************************************************/
STDMETHODIMP ESource::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hResult = E_NOINTERFACE;
    char    cIID[100],*IIDName,TrulyUnknown[]="??????";


    StringFromGUID2(riid,(LPOLESTR)cIID,sizeof(cIID)/2);
    FPRINTF(logfile,"ESource::QueryInterface (ppvObj = %p,\n    riid = %S \n",ppvObj,cIID);

    // a pointer to result storage must be supplied
    if (!ppvObj)
    {
        return E_INVALIDARG;
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
    else if (riid == IID_IDispatchEx)
    {
        FPRINTF3(logfile,"IDispatchEx");
#ifdef PARSEPROCEDURE
// Joel Alley = uncomment this next line to see what ParseProcedure does with IDispatch
// instead of IDispatchEx.
        // if(InvokeCount++ > 2)
#endif
        *ppvObj = static_cast<IDispatch *>(this);
    }
    else if (riid == IID_IDispatch)
    {
        FPRINTF3(logfile,"IDispatch");
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
    FPRINTF3(logfile,") interface.\n");

    // on success, call AddRef()
    if (*ppvObj != NULL)
    {
        AddRef();
        hResult = NOERROR;
    }

    return hResult;
}

/**********************************/
/* ESource::AddRef                */
/*                                */
/* increment the reference count. */
/**********************************/
STDMETHODIMP_(ULONG) ESource::AddRef()
{
    //        InterlockedIncrement()
    //  Returns > 0 if ulRefCount > 0, but is not guaranteed
    // to return the actual value of ulRefCount.
    InterlockedIncrement((long *)&ulRefCount);
    FPRINTF(logfile,"ESource::AddRef The count is now %u\n",ulRefCount);
    return ulRefCount;
}

/***********************************************************/
/* ESource::Release                                        */
/*                                                         */
/* decrement the reference count, if zero, destroy object. */
/***********************************************************/
STDMETHODIMP_(ULONG) ESource::Release()
{
    InterlockedDecrement((long *)&ulRefCount);
    FPRINTF(logfile,"ESource::Release The count is now %u\n",ulRefCount);

    if (ulRefCount)
    {
        return ulRefCount;
    }

    delete this;
    return 0;
}




/******************************************************************************
*   IDispatch Interface -- This interface allows this class to be used as an
*   automation server, allowing its functions to be called by other COM
*   objects
******************************************************************************/

/******************************************************************************
*   GetTypeInfoCount -- This function determines if the class supports type
*   information interfaces or not.  It places 1 in iTInfo if the class supports
*   type information and 0 if it doesn't.
******************************************************************************/
STDMETHODIMP ESource::GetTypeInfoCount(UINT *pTInfo)
{
    FPRINTF(logfile,"ESource::GetTypeInfoCount\n");

    //This object doesn't support type information
    *pTInfo = 0;
    return E_NOTIMPL;
}

/******************************************************************************
*   GetTypeInfo -- Returns the type information for the class.  For classes
*   that don't support type information, this function returns DISP_E_BADINDEX;
*   It would be preferable to return E_NOTIMPL, but that is not one of our options.
******************************************************************************/
STDMETHODIMP ESource::GetTypeInfo(UINT pTInfo, LCID plcid,
                                         ITypeInfo **pTypeInfo)
{
    FPRINTF(logfile,"ESource::GetTypeInfo\n");

    //This object doesn't support type information
    *pTypeInfo = NULL;
    return DISP_E_BADINDEX;
}


/******************************************************************************
*   GetIDsOfNames -- Takes an array of strings and returns an array of DISPID's
*   which correspond to the methods or properties indicated.  In real life,
*   If the name is not recognized, then DISP_E_UNKNOWNNAME is returned.
*   This should never be called.  Because of the IAdvise, our caller should know
*   all of the DispID's that we will support.
******************************************************************************/
STDMETHODIMP ESource::GetIDsOfNames(REFIID riid,
                                          OLECHAR **pNames,
                                          UINT pNamesCount,  LCID plcid,
                                          DISPID *pbDispID)
{
    HRESULT RetCode = S_OK;
    char    lIID[100];


    StringFromGUID2(riid,(LPOLESTR)lIID,sizeof(lIID)/2);

    FPRINTF(logfile,"ESource::GetIDsOfNames\n");
    FPRINTF2(logfile,"pNamesCount %d   riid %S \n",pNamesCount,lIID);

    //check parameters
    if (riid != IID_NULL)
    {
        RetCode = E_INVALIDARG;
    }
    else
    {
        RetCode = E_NOTIMPL;

    }

    return RetCode;
}

/******************************************************************************
*  Invoke -- Takes a dispid and uses it to call a method or property defined
*  in the script code in the context of this ESource.
******************************************************************************/
STDMETHODIMP ESource::Invoke(DISPID pDispID, REFIID riid, LCID plcid,
                                    WORD pFlags, DISPPARAMS* pDispParams,
                                    VARIANT* pVarResult, EXCEPINFO* pExcepInfo,
                                    UINT* pArgErr)
{
    FPRINTF(logfile,"ESource::Invoke\n");

    OLECHAR    lIID[100];
    StringFromGUID2(riid, lIID, sizeof(lIID));
    FPRINTF2(logfile,"riid %S \n",lIID);
    FPRINTF2(logfile,"pArgErr %p\n",pArgErr);

    //  check parameters
    //  According to the Doc, this should always be IID_NULL.
    if (riid != IID_NULL)
    {
        return E_INVALIDARG;
    }

    return CommonInvoke(pDispID, plcid, pFlags, pDispParams, pVarResult, pExcepInfo);
}




  /***** IDispatchEx Methods *****/
/******************************************************************************
*                 GetDispID
******************************************************************************/
  STDMETHODIMP ESource::GetDispID(
    /* [in] */ BSTR pName,
    /* [in] */ DWORD pFlags,               // Derived from fdexName... defines.
    /* [out] */ DISPID __RPC_FAR *pbDispID)
{
    HRESULT RetCode= E_NOTIMPL;

    FPRINTF(logfile,"ESource::GetDispID\n");
    FPRINTF2(logfile,"Name \"%S\" Flags 0x%08x\n",pName,pFlags);
    FPRINTF2(logfile,"In english the pFlags signifies:\n");
    FPRINTF2(logfile,"%s\n",FlagMeaning('H',pFlags,DispExGetDispID));
    return RetCode;
}


/******************************************************************************
*                 InvokeEx
******************************************************************************/
  STDMETHODIMP ESource::InvokeEx(
    /* [in] */ DISPID pDispID,
    /* [in] */ LCID lcid,
    /* [in] */ WORD pFlags,                // Derived from ... defines.
    /* [in] */ DISPPARAMS __RPC_FAR *pArgs,
    /* [out] */ VARIANT __RPC_FAR *pbResults,
    /* [out] */ EXCEPINFO __RPC_FAR *pbErrInfo,
    /* [unique][in] */ IServiceProvider __RPC_FAR *pCaller)
{
    FPRINTF(logfile,"ESource::InvokeEx\n");
    FPRINTF2(logfile,"IServiceProvider %p\n",pCaller);

    return CommonInvoke(pDispID, lcid, pFlags, pArgs, pbResults, pbErrInfo);
}



/******************************************************************************
*                 DeleteMemberByName
******************************************************************************/
  STDMETHODIMP ESource::DeleteMemberByName(
    /* [in] */ BSTR pName,
    /* [in] */ DWORD pFlags)                // Derived from fdexName... defines.
{
    FPRINTF(logfile,"ESource::DeleteMemberByName\n");
    FPRINTF2(logfile,"Name \"%S\" Flags 0x%08x\n",pName,pFlags);
    FPRINTF2(logfile,"In english the pFlags signifies:\n");
    FPRINTF2(logfile,"%s\n",FlagMeaning('H',pFlags,DispExGetDispID));
    return E_NOTIMPL;
}



/******************************************************************************
*                 DeleteMemberByDispID
******************************************************************************/
  STDMETHODIMP ESource::DeleteMemberByDispID(
    /* [in] */ DISPID pDispID)
{
    FPRINTF(logfile,"ESource::DeleteMemberByDispID\n");
    FPRINTF2(logfile,"DispID %ld \n",pDispID);
    return E_NOTIMPL;
}



/******************************************************************************
*                 GetMemberProperties
******************************************************************************/
  STDMETHODIMP ESource::GetMemberProperties(
    /* [in] */ DISPID pDispID,
    /* [in] */ DWORD pFetchFlag,           // Derived from ???... defines.
    /* [out] */ DWORD __RPC_FAR *pbProperties)  // Derived from fdexProp... defines.
{
    FPRINTF(logfile,"ESource::GetMemberProperties\n");
    FPRINTF2(logfile,"DispID %ld   Flags %08x\n ",pDispID,pFetchFlag);
    return E_NOTIMPL;
}



/******************************************************************************
*                 GetMemberName
******************************************************************************/
  STDMETHODIMP ESource::GetMemberName(
    /* [in] */ DISPID pDispID,
    /* [out] */ BSTR __RPC_FAR *pbName)
{
    HRESULT RetCode=E_NOTIMPL;

    FPRINTF(logfile,"ESource::GetMemberName\n");
    FPRINTF2(logfile,"DispID %ld ",pDispID);

    *pbName = SysAllocString(L"");
    return RetCode;
}



/******************************************************************************
*                 GetNextDispID
******************************************************************************/
  STDMETHODIMP ESource::GetNextDispID(
    /* [in] */ DWORD pFlags,               // Derived from fdexEnum... defines.
    /* [in] */ DISPID pDispID,             // Previous DispID returned.
    /* [out] */ DISPID __RPC_FAR *pbDispID)
{
    FPRINTF(logfile,"ESource::GetNextDispID\n");
    FPRINTF2(logfile,"DispID %ld   Flags %08x\n ",pDispID,pFlags);
    return E_NOTIMPL;
}



/******************************************************************************
*                 GetNameSpaceParent
******************************************************************************/
  STDMETHODIMP ESource::GetNameSpaceParent(
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *pbIUnknown)
{
    FPRINTF(logfile,"ESource::GetNameSpaceParent\n");
    return E_NOTIMPL;
}


STDMETHODIMP ESource::AddMap(char *Name, PEMAP *pbNewMap)
{
    PEMAP     Next;
    ListItem *Map;
    HRESULT   RetCode=S_OK;


    *pbNewMap = NULL;
    Next = (PEMAP)GlobalAlloc(GMEM_FIXED,sizeof(EMAP));
    if (Next == NULL)
    {
        RetCode = E_OUTOFMEMORY;
    }
    else
    {
        memset(Next,0,sizeof(EMAP));
        Map = EventFunctionMap.AddItem(Name,LinkedList::End,(void *)Next);
        if (Next)
        {
            *pbNewMap = Next;
            Next->Sink = -1;
        }
        else
        {
            RetCode = E_OUTOFMEMORY;
            GlobalFree((HGLOBAL)Next);
        }
    }
    return RetCode;
}



STDMETHODIMP ESource::SetMap(LPCOLESTR pName, DISPID SinkDispID)
{
    PEMAP     Map;
    HRESULT   RetCode=S_OK;

    FPRINTF2(logfile,"ESource::SetMap\n");

    Map = (PEMAP)EventFunctionMap.FindContent(0);
    /*    For each entry in the type library, create an entry on the Event Map chain.
     *  This is a many to one relation.  Each of the different Source Disp ID's
     *  will translate to the same Sink Disp ID.  There is only one chunk of code
     *  being bound to this Event that the Type Library is describing.  So every
     *  Source call must map to the same Sink.
     */
    while (Map)
    {
        size_t tab = 20 - wcslen(pName);
        tab = tab < 1 ? 1 : tab;
        FPRINTF2(logfile,"ESource::SetMap \"%S\" %*s \"%S\"\n",pName,tab," ",Map->SourceEventName);

        if (wcsicmp(pName,Map->SourceEventName) == 0)
        {
            break;
        }
        Map = (PEMAP)EventFunctionMap.FindContent();
    }

    if (!Map)
    {
        RetCode = DISP_E_UNKNOWNNAME;
    }
    else
    {
        Map->Sink = SinkDispID;
    }
    FPRINTF2(logfile,"ESource::SetMap()  An HRESULT of 0 means we found it in the function list. HRESULT %08x.\n",RetCode);
    return RetCode;
}


STDMETHODIMP ESource::FindMap(LPCOLESTR Event, DISPID *SinkDispID)
{
    PEMAP     Map;
    HRESULT   RetCode=S_OK;


    FPRINTF2(logfile,"ESource::QueryMap\n");

    *SinkDispID = (DISPID) -1;
    Map = (PEMAP)EventFunctionMap.FindContent(0);
    /*    For each entry in the type library, create an entry on the Event Map chain.
     *  This is a many to one relation.  Each of the different Source Disp ID's
     *  will translate to the same Sink Disp ID.  There is only one chunk of code
     *  being bound to this Event that the Type Library is describing.  So every
     *  Source call must map to the same Sink.
     */
    while (Map)
    {
        if (wcsicmp(Event,Map->SourceEventName) == 0)
        {
            break;
        }
        Map = (PEMAP)EventFunctionMap.FindContent();
    }

    if (!Map)
    {
        RetCode = DISP_E_UNKNOWNNAME;
    }
    else
    {
        *SinkDispID = Map->Sink;
    }

    FPRINTF2(logfile,"ESource::QueryMap()  An HRESULT of 0 means we found it.  DispID %d HRESULT %08x.\n",*SinkDispID,RetCode);
    return RetCode;
}




STDMETHODIMP ESource::GetTypeInfo(ITypeInfo **pbTypeInfo)
{
    HRESULT    RetCode;
    IProvideClassInfo* ClassInfo = NULL;
    TYPEATTR  *TypeAttributes = NULL;
    ITypeInfo *CoClassTypeInfo = NULL,*TypeInfo;
    int        NumInterfaces;
    int        InterfaceFlags;
    int        i;
    HREFTYPE   refType;


    OLECHAR    lGUID[50];
    FUNCDESC  *FuncDesc;
    unsigned int j,k,NameCount,FCount;
    BSTR       SourceEventName[20];
    char       *IIDName,TrulyUnknown[]="??????";


    FPRINTF2(logfile,"ESource::GetTypeInfo() \n");

    //make sure we have an source IDispatch to work with.
    if (Source)
    {
        //Get the TypeInfo from the source IDispatch
        RetCode = Source->QueryInterface( IID_IProvideClassInfo,
                                          (void**)&ClassInfo);
        if (FAILED(RetCode))
        {
            return RetCode;
        }

        RetCode = ClassInfo->GetClassInfo( &CoClassTypeInfo );
        ClassInfo->Release();

        if (FAILED(RetCode))
        {
            return RetCode;
        }

        //Check the type attributes of the ITypeInfo to make sure it's a coclass
        RetCode = CoClassTypeInfo->GetTypeAttr( &TypeAttributes );
        if (SUCCEEDED(RetCode))
        {
            if (TypeAttributes->typekind != TKIND_COCLASS) return E_NOTIMPL;
        }
        else
        {
            return RetCode;
        }

        //How many interfaces does this coclass implement?
        NumInterfaces = TypeAttributes->cImplTypes;
        FPRINTF2(logfile,"There are %d TypeInterfaces.\n",NumInterfaces);

        //Release the type attributes
        CoClassTypeInfo->ReleaseTypeAttr( TypeAttributes );

        //Now, search through the set of interfaces looking for one that is marked
        //default, source, and not restricted.
        for (i=0; i<NumInterfaces; i++)
        {
            RetCode = CoClassTypeInfo->GetImplTypeFlags( i, &InterfaceFlags );
            FPRINTF2(logfile,"\n\n   The flags for TypeInterface %d are %08x; GetImplTypeFlags() HRESULT is %08x.\n",
                     (i+1),InterfaceFlags,RetCode);
            FPRINTF2(logfile,"In english the pFlags signifies: %s\n",
                     FlagMeaning('H',InterfaceFlags,ImplementedInterface));
            if (FAILED(RetCode))
            {
                return RetCode;
            }

            RetCode = CoClassTypeInfo->GetRefTypeOfImplType( i, &refType );
            RetCode = CoClassTypeInfo->GetRefTypeInfo( refType, &TypeInfo );
            RetCode = TypeInfo->GetTypeAttr(&TypeAttributes);
            FCount = TypeAttributes->cFuncs;
            StringFromGUID2(TypeAttributes->guid,lGUID,sizeof(lGUID)/sizeof(lGUID[0]));
            if (!(IIDName = NameThatInterface((OLECHAR *)&lGUID[0])))
            {
                IIDName = &TrulyUnknown[0];
            }

            FPRINTF2(logfile,"The GUID is %S (%s) and there are %d functions.\n",lGUID,IIDName,FCount);
            if (IIDName != &TrulyUnknown[0])
            {
                free(IIDName);
            }
            TypeInfo->ReleaseTypeAttr(TypeAttributes);


            /*    For each entry in the type library, create an entry on the Event Map chain.
             *  This is a many to one relation.  Each of the different Source Disp ID's
             *  will translate to the same Sink Disp ID.  There is only one chunk of code
             *  being bound to this Event that the Type Library is describing.  So every
             *  Source call must map to the same Sink.
             */
            for (k=0; k<FCount; k++)
            {
                TypeInfo->GetFuncDesc(k, &FuncDesc);
                TypeInfo->GetNames(FuncDesc->memid, &(SourceEventName[0]), 20,(unsigned int *)&NameCount);
                FPRINTF2(logfile,"Event %d has %d names: ",(k+1),NameCount);
                for (j=0; j<NameCount; j++)
                {
                    FPRINTF2(logfile,"\"%S\"",SourceEventName[j]);
                    SysFreeString(SourceEventName[j]);
                }
                FPRINTF2(logfile,"\n");
                TypeInfo->ReleaseFuncDesc(FuncDesc);
            }

            if ((InterfaceFlags & (IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE |
                                   IMPLTYPEFLAG_FRESTRICTED)) == (IMPLTYPEFLAG_FDEFAULT |
                                                                  IMPLTYPEFLAG_FSOURCE))
            {
                *pbTypeInfo = TypeInfo;
                FPRINTF2(logfile,"Found the default. The previous list shows the functions.\n");
            }
            else
            {
                TypeInfo->Release();
            }


        }
    }
    return S_OK;
    // return E_INVALIDARG;
}




/******************************************************************************
*                 CommonInvoke
*
*   Called by both Invoke() and InvokeEx so that behavior will be identical,
*  and support is centrally located (on place to maintain).
******************************************************************************/
  STDMETHODIMP ESource::CommonInvoke(
    /* [in] */ DISPID pDispID,
    /* [in] */ LCID lcid,
    /* [in] */ WORD pFlags,                // Derived from ... defines.
    /* [in] */ DISPPARAMS __RPC_FAR *pArgs,
    /* [out] */ VARIANT __RPC_FAR *pbResults,
    /* [out] */ EXCEPINFO __RPC_FAR *pbErrInfo)
{
    HRESULT  RetCode=S_OK;
    PEMAP   Map;
    char    DispIDName[MAX_PATH];
    DISPPARAMS ASDispParms,*EDispParms;
    VARIANT    Mutant;
    DISPID     NamedArg;

    if (this->EventName)
    {
        FPRINTF2(logfile,"We are %S\n",EventName);
    }
    FPRINTF2(logfile,"They are looking for %d\n",pDispID);
    //  There may be more than one function using the same DispID.
    // Make sure it is one of the functions we are implementing.
    sprintf(DispIDName,"%d",(int)pDispID);
    Map = (PEMAP)EventFunctionMap.FindContent(DispIDName);
    if (Map)
    {
        if ((int)Map->Sink == -1)
        {
            RetCode = DISP_E_UNKNOWNNAME;
        }
    }
    else
    {
        RetCode = DISP_E_UNKNOWNNAME;
    }

    //  Print the saved DispIDName.  This is more difficult than the Std Dispatch.
    if (Map)
    {
        FPRINTF2(logfile,"Event \"%S\" DispID %d\n",Map->SourceEventName,(int)Map->Sink);
        if ((int)Map->Sink == -1)
        {
            FPRINTF2(logfile,"This particular event is not defined.\n");
        }
    }
    else
    {
        FPRINTF2(logfile,">>>> This DispID is not defined!\n");
    }
    FPRINTF2(logfile,"In english the pFlags signifies: %s\n",
             FlagMeaning('H',pFlags,DispatchInvoke));
    FPRINTF2(logfile,"pArgs %p\n",pArgs);
    FPRINTF2(logfile,"pbResults %p\n",pbResults);
    FPRINTF2(logfile,"pbErrInfo %p\n",pbErrInfo);



    /*   >>>>???<<<<
      Need to add code to examine the pExcepInfo and pArgErr parameters.
    */


    //    Override the parameters to pass the named THIS, if this is AddScriptlet.
    //  Then AddScriplet will emulate the behavior of ParseProcedure.
    if (EventType == AddScriptlet)
    {
        VariantInit(&Mutant);
        V_VT(&Mutant) = VT_DISPATCH;
        V_DISPATCH(&Mutant) = Source;

        NamedArg = DISPID_THIS;

        ASDispParms.rgvarg = &Mutant;
        ASDispParms.rgdispidNamedArgs = &NamedArg;
        ASDispParms.cArgs = 1;
        ASDispParms.cNamedArgs = 1;
        EDispParms = &ASDispParms;
    }
    else
    {
        EDispParms = pArgs;        // Set the Engine DispParms.
    }




    if (SUCCEEDED(RetCode))
    {
        switch (pFlags)
        {
            case DISPATCH_METHOD:
                RetCode = Engine->InvokeEx(Map->Sink, lcid, pFlags,  EDispParms,
                                           pbResults, pbErrInfo, NULL);
                break;
            case DISPATCH_PROPERTYGET:
                if (pbResults)
                {
                    V_VT(pbResults) = VT_DISPATCH;
                    pbResults->pdispVal = this;
                    this->AddRef();
                }
                else
                {
                    RetCode = E_POINTER;   // Let the caller know we did not do anything.
                }
                break;
            case DISPATCH_PROPERTYPUT:
            case DISPATCH_PROPERTYPUTREF:
            case DISPATCH_CONSTRUCT:
            default:
                RetCode = E_NOTIMPL;   // Let the caller know we did not do anything.
                FPRINTF2(logfile,"This type of Invoke is not currently supported.\n");
        }

    }

    FPRINTF2(logfile,"ESource::CommonInvoke()  Exit  HRESULT = %08x\n",RetCode);

    return RetCode;
}



