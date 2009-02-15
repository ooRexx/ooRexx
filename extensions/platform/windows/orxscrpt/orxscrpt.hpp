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

#ifndef ORXSCRPT
#define ORXSCRPT

#include "orxscrpt_main.hpp"
#include <objsafe.h>
#include <dispex.h>        // IDispatchEx

#include "nameditem.hpp"
#include "eng2rexx.hpp"
#include "OrxDispID.hpp"
#include "OrxEvents.hpp"
#include "OrxScrptError.hpp"

// defines & macros
#define STD_THREADID 1    // script engine will only have one thread - and this is it (number)!

// any method that changes the engine may only be called from within the
// same thread as the one in which the engine was created
#define CHECK_BASE_THREAD if (dwBaseThread != GetCurrentThreadId()) return RPC_E_WRONG_THREAD;

typedef struct REXX_CODE_BLOCK_Struct {
  enum        EntryType{ParseProcedure,AddScriptlet,ParseScript,Engine};
  EntryType   EntrySource;   //  Need this to decode the Flags properly.
  OLECHAR    *Name;          //  Item Name (or NULL) this is associated with.  (WhoKnows() may eventually need this, and the flags.)
  DWORD       Flags;         //  Received at the same time as the source.
  ULONG       StartingLN;    //  Line number the code started on.
  RexxRoutineObject  Code;   //  Pointer to the executable object.
  } RCB, *PRCB;


class CreateCodeData
{
public:
    OrxScript *engine;      // the hosting script engine
    LPCOLESTR  strCode;     // the code string
    RexxRoutineObject routine;  // the create routine
    RexxConditionData *condData;  // returned condition data
};


// class definition

class OrxScript : public LLLContent,
                  public IActiveScript,
                  public IActiveScriptParse,
                  public IPersistStreamInit,
                  public IDispatchEx,
                  public IActiveScriptParseProcedure,
                  public IObjectSafety
{
  public:
    OrxScript();                         // CTOR
    virtual ~OrxScript();                // DTOR

    LCID               Lang;             //  Needed for calls into the engine

    /***** LLLContent Methods *****/
    void SetListState(LLLState Status) {LLLContent::SetListState(Status);}

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    // IActiveScript methods
    STDMETHOD(SetScriptSite)(IActiveScriptSite *);
    STDMETHOD(GetScriptSite)(REFIID, void **);
    STDMETHOD(SetScriptState)(SCRIPTSTATE);
    STDMETHOD(GetScriptState)(SCRIPTSTATE *);
    STDMETHOD(Close)(void);
    STDMETHOD(AddNamedItem)(LPCOLESTR, DWORD);
    STDMETHOD(AddTypeLib)(REFGUID, DWORD, DWORD, DWORD);
    STDMETHOD(GetScriptDispatch)(LPCOLESTR, IDispatch **);
    STDMETHOD(GetCurrentScriptThreadID)(SCRIPTTHREADID *);
    STDMETHOD(GetScriptThreadID)(DWORD, SCRIPTTHREADID *);
    STDMETHOD(GetScriptThreadState)(SCRIPTTHREADID, SCRIPTTHREADSTATE *);
    STDMETHOD(InterruptScriptThread)(SCRIPTTHREADID, const EXCEPINFO *, DWORD);
    STDMETHOD(Clone)(IActiveScript **);

    // IActiveScriptParse methods
    STDMETHOD(InitNew)(void);
    STDMETHOD(AddScriptlet)(LPCOLESTR, LPCOLESTR, LPCOLESTR, LPCOLESTR, LPCOLESTR, LPCOLESTR, DWORD, ULONG, DWORD, BSTR *, EXCEPINFO *);
    STDMETHOD(ParseScriptText)(LPCOLESTR, LPCOLESTR, IUnknown __RPC_FAR *, LPCOLESTR, DWORD, ULONG, DWORD, VARIANT *, EXCEPINFO *);

    // IPersist
    STDMETHOD(GetClassID)(LPCLSID lpClassID);

    // IPersistStreamInit methods
    STDMETHOD(IsDirty)(void);
    STDMETHOD(Load)(LPSTREAM);
    STDMETHOD(Save)(LPSTREAM, BOOL);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER FAR*);

    // IObjectSafety methods
    STDMETHOD(GetInterfaceSafetyOptions)(REFIID, DWORD *, DWORD *);
    STDMETHOD(SetInterfaceSafetyOptions)(REFIID, DWORD, DWORD);

  /***** IActiveScriptParseProcedure Methods *****/
  /*  In the code, this is just after IActiveScriptParse's AddScriptlet().  It performs the  */
  /* logical function, without as much mess (supposedly).                                    */
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
    /* out */ IDispatch **pbOurIDispatch);


  /***** IDispatch Methods *****/
  /*    Rather than naming variables by their type ala the Hungarian
   *  method,  I try to name variables by their purpose.  Their type
   *  can easily be looked up, and many times knowing the type does
   *  not help in understanding what the variable does.
   *    Naming by what it does increases understanding and reduces
   *  name collision.  For example, pName is the name passed in as
   *  a parameter, and lName is the local variable name.  Now lName = pName;
   *  is easy to see and understand what is happening.
   *
   *     Prefix    Meaning
   *       p    -  Parameter
   *       pb   -  Pass Back parameter
   *       l    -  Local
   *
   */
  STDMETHODIMP GetTypeInfoCount(UINT* pTInfo);
  STDMETHODIMP GetTypeInfo(UINT pTInfo, LCID plcid, ITypeInfo** pTypeInfo);
  STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** pNames,
    UINT pNamesCount, LCID plcid, DISPID* pbDispID);
  STDMETHODIMP Invoke(DISPID pDispID, REFIID riid, LCID plcid,
    WORD pFlags, DISPPARAMS* pDispParams,  VARIANT* pVarResult,
    EXCEPINFO* pExcepInfo,  UINT* pArgErr);



  /***** IDispatchEx Methods *****/
  /*
      OK, I cheated.  I ripped this right out of <DISPEX.H>.
    Then I changed all of the parameter names to my convention.

      The documentation on the functionality I found by doing a
    search in the MSDN on "dispatch_propertyputref".  One title
    returned was  "-- text figures".  Clicking on the
    'Back to Article' button produces no meaningful results.

      It turns out the full article is 'Dynamic Object Composition
    Using IDispatchEx' by Joe Graf.
  */
  /******************************************************************************
  *  GetDispID -- returns the DispID for a member, without having to construct
  *  an array first.  It also allows for the support of Dynamic Objects
  *  by adding them to our name space.
  *    GetDispID's support of 'Dynamic Objects', which we are not supporting
  *  right now, is triggerred by fdexNameEnsure.  If this flag is set,
  *  then the routine returns E_NOTIMPL.
  ******************************************************************************/
  virtual HRESULT STDMETHODCALLTYPE GetDispID(
    /* [in] */ BSTR pName,
    /* [in] */ DWORD pFlags,               // Derived from fdexName... defines.
    /* [out] */ DISPID __RPC_FAR *pbDispID);

  /******************************************************************************
  *  InvokeEx -- is for support of 'Dynamic Objects', which
  *  we are not supporting right now.  It always returns E_NOTIMPL.
  ******************************************************************************/
  virtual /* [local] */ HRESULT STDMETHODCALLTYPE InvokeEx(
    /* [in] */ DISPID pDispID,
    /* [in] */ LCID lcid,
    /* [in] */ WORD pFlags,                // Derived from the regular DISPATCH...
                                           // defines + DISPATCH_CONSTRUCT.
    /* [in] */ DISPPARAMS __RPC_FAR *pArgs,
    /* [out] */ VARIANT __RPC_FAR *pbResults,
    /* [out] */ EXCEPINFO __RPC_FAR *pbErrInfo,
    /* [unique][in] */ IServiceProvider __RPC_FAR *pCaller);

  /******************************************************************************
  *  DeleteMemberByName -- is for support of 'Dynamic Objects', which
  *  we are not supporting right now.  It always returns E_NOTIMPL.
  ******************************************************************************/
  virtual HRESULT STDMETHODCALLTYPE DeleteMemberByName(
    /* [in] */ BSTR pName,
    /* [in] */ DWORD pFlags);              // Derived from fdexName... defines.

  /******************************************************************************
  *  DeleteMemberByDispID -- is for support of 'Dynamic Objects', which
  *  we are not supporting right now.  It always returns E_NOTIMPL.
  ******************************************************************************/
  virtual HRESULT STDMETHODCALLTYPE DeleteMemberByDispID(
    /* [in] */ DISPID pDispID);

  virtual HRESULT STDMETHODCALLTYPE GetMemberProperties(
    /* [in] */ DISPID pDispID,
    /* [in] */ DWORD pFetchFlag,           // Derived from ???... defines.
    /* [out] */ DWORD __RPC_FAR *pbProperties); // Derived from fdexProp... defines.

  /******************************************************************************
  *  GetMemberName --  Takes a DispID as an index into our table of names.
  *  If the index is valid, then it returns S_OK, and the name is copied
  *  into a BSTR whose address is placed in pbName.  Otherwise, the error
  *  DISP_E_MEMBERNOTFOUND is returned, and pbName points to a ZLS (Zero
  *  Length String L"").
  ******************************************************************************/
  virtual HRESULT STDMETHODCALLTYPE GetMemberName(
    /* [in] */ DISPID pDispID,
    /* [out] */ BSTR __RPC_FAR *pbName);

  virtual HRESULT STDMETHODCALLTYPE GetNextDispID(
    /* [in] */ DWORD pFlags,               // Derived from fdexEnum... defines.
    /* [in] */ DISPID pDispID,             // Previous DispID returned.
    /* [out] */ DISPID __RPC_FAR *pbDispID);

  /******************************************************************************
  *  GetNameSpaceParent -- is for support of 'Dynamic Objects', which
  *  we are not supporting right now.  It always returns E_NOTIMPL.
  ******************************************************************************/
  virtual HRESULT STDMETHODCALLTYPE GetNameSpaceParent(
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *pbIUnknown);




  /****** NON-INTERFACE PUBLIC METHODS ******/
  void insertVariable(RexxThreadContext *, const char *name, RexxObjectPtr value);
  RexxObjectPtr  getSecurityManager() { return securityManager.Code; }
  RexxObjectPtr  getSecurityObject() { return securityObject; }
  DWORD getSafetyOptions() { return dwSafetyOptions; }
  OrxNamedItem* getNamedItems() { return NamedItemList; }
  FILE *getLogFile() { return logfile; }

  IActiveScriptSite* getScriptSitePtr() { return pActiveScriptSite; }
  VARIANT *GetExternalProperty(const char *PropName) {
    PGVARIANT Property;
    Property =  (PGVARIANT)PropertyList.FindContent(PropName);
    if(Property) return &(Property->Mutant);
    else return NULL;
  }

  void* findRexxFunction(const char* name) {
    ListItem *result = RexxFunctions->FindItem(name);
    if (result) return result->GetContent();
    return NULL;
  }

  void runMethod(RexxThreadContext *context, RCB *RexxCode, RexxArrayObject args, RexxObjectPtr &targetResult, RexxConditionData &condData);
  int createRoutine(LPCOLESTR strCode, ULONG startingLineNumber, RexxRoutineObject &routine);

  inline IInternetHostSecurityManager* getIESecurityManager() { return pIESecurityManager; }
  inline bool checkObjectCreation() { return checkObjectCreation; }
  inline void setObjectCreation(bool f) { checkObjectCreation = f; }

  STDMETHODIMP GetSourceIDispatch(LPCOLESTR   ItemName,
                                  LPCOLESTR   SubItemName,
                                  IDispatch **Source);
  //  Things will be bad if the state is not abstain when we start....
  void PreventCallBack(void) { EventState = NoProperties; } // Deny all knowledge
  void ReleaseCallBack(void) { if(EventState == NoProperties) EventState = Abstain; }

STDMETHODIMP LocalParseProcedureText(
  /* in  */ const char *Name,
  /* in  */ DWORD       Flags,
  /* out */ IDispatch **pbOurIDispatch);



  private:
/******************************************************************************
*                 CommonInvoke
*
*   Called by both Invoke() and InvokeEx so that behavior will be identical,
*  and support is centrally located (on place to maintain).
******************************************************************************/
  STDMETHODIMP OrxScript::CommonInvoke(
    /* [in] */ DISPID pDispID,
    /* [in] */ LCID lcid,
    /* [in] */ WORD pFlags,                // Derived from ... defines.
    /* [in] */ DISPPARAMS __RPC_FAR *pArgs,
    /* [out] */ VARIANT __RPC_FAR *pbResults,
    /* [out] */ EXCEPINFO __RPC_FAR *pbErrInfo);

  STDMETHODIMP InvokeMethod(
    /* [in]  */ PDID pDIDData,
    /* [in]  */ DISPPARAMS __RPC_FAR *pArgs,
    /* [out] */ VARIANT __RPC_FAR *pbResults,
    /* [out] */ EXCEPINFO __RPC_FAR *pbErrInfo);

  STDMETHODIMP BuildRCB(
    /* [in]  */ RCB::EntryType EntrySource,
    /* [in]  */ OLECHAR    *Name,
    /* [in]  */ DWORD       Flags,
    /* [in]  */ ULONG       StartingLN,
    /* [in]  */ RexxObjectPtr  Code,
    /* [out] */ PRCB       *CodeBlock);


    ULONG              ulRefCount;                 // reference count
    bool               checkObjectCreation;       // turn off IE security manager checking on special occasions
    bool               enableVariableCapture;      // enables variable capture in termination exit
    bool               initNew;                   // true if IPersistInitStream::Load, IPersistInitStream::InitNew or IActiveScriptParse::InitNew have been called
    bool               isConnected;               // to change between CONNECTED/DISCONNECTED without having to really
                                                   // unhook from event source...
    SCRIPTSTATE        engineState;                // state of script engine
    SCRIPTTHREADSTATE  threadState;                // state of thread (just one supported)
    IActiveScriptSite *pActiveScriptSite;          // the active script site
    IInternetHostSecurityManager *pIESecurityManager; // Internet Explorer security manager (*not* REXX's!)
    DWORD              dwBaseThread;               // the base thread of the engine
    FILE              *logfile;                    // Our private log file
    int                engineId;                   // Our Creation number.
    DWORD              dwSafetyOptions;            // Flags for Internet Explorer to allow scripts to execute.
    RCB                securityManager;            // REXX code for security manager
    RexxObjectPtr      securityObject;             // Object produced by running the Security Manager Code.
    OrxDispID          DispID;                     // The head of the chain of DispIDs that we support.
    OrxNamedItem      *NamedItemList;              // The head of the chain of NamedItems.
    VariantLList       PropertyList;               // The head of the chain of potential Properties.
    //     The next set of variables are all for setting up events.
    OrxEvent          *Events;                     // Head of the Event chain (linked list).
    enum EventConstrctn{Abstain,Searching,Binding,Bound,IMF,NoProperties};
    EventConstrctn     EventState;                 // Shorts GetIDsOfNames() durring event hooking.
    OLECHAR           *EventSourceName;            // The name we cannot know duing hooking.
    int                EventCount;                 // Generates unique names for ParseProcedure events

    // list section: named items, available rexx methods, rexx code
    // attention: we will have to use "special" linked lists here to
    // take care of memory mgmt.
    // stores the flattened rexx method
    LinkedList        *RexxCodeList;               // will clean up content ptr
    // stores the names of the available rexx functions
    Index             *RexxFunctions;              // will just clean up list item entries, not content ptr
    // stores all code for execution when transiting into connected
    Index             *RexxExecStack;              // will just clean up list item entries, not content ptr

  };



#endif
