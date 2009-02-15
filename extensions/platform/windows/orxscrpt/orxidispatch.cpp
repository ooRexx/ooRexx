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
*     This file is a continuation of orxscrpt.cpp.  The class was too large
*  and too complicated to define all of the methods in one source file.
*
*
******************************************************************************/

// #define OrxIDispatch_cpp
#include "orxscrpt.hpp"

FL DispatchInvoke[] = {           // H - orizontal flags
  {DISPATCH_METHOD,"Method"},
  {DISPATCH_PROPERTYGET,"Property Get"},
  {DISPATCH_PROPERTYPUT,"Property Put"},
  {DISPATCH_PROPERTYPUTREF,"Put by Reference"},
  {DISPATCH_CONSTRUCT,"Use Member as a Constructor"}, // IDispatchEx - Dynamic Objects
  {(DWORD)0,(char *)NULL}
  };


/*
 *     Variants are a nasty bunch.  Currently, they are defined as "short int",
 *  which means that they take up 2 bytes.  The high order nibble is defined
 *  as a horizontal flag, and the first 3 nibbles are defined as a vertical
 *  type.  These are separated by ANDing the correct compliment of VT_TYPEMASK
 *  onto the variant type field examine the desired part:
 *  Variant & ~VT_TYPEMASK     yields variant flags.
 *  Variant & VT_TYPEMASK      yields variant types.
 */
FL VariantFlags[] = {             // H - orizontal flags
  {VT_VECTOR,"Vector"},
  {VT_ARRAY,"Array"},
  {VT_BYREF,"By Reference"},
  {VT_RESERVED,"Reserved"},
  {(DWORD)0,(char *)NULL}
  };


FL VariantTypes[] = {             // V - ertical flags
  {VT_EMPTY,"Empty"},
  {VT_NULL,"SQL NULL"},
  {VT_I2,"Short Int"},
  {VT_I4,"Int"},
  {VT_R4,"Float"},
  {VT_R8,"Double"},
  {VT_CY,"Currency"},
  {VT_DATE,"Date"},
  {VT_BSTR,"VB String"},
  {VT_DISPATCH,"IDispatch"},
  {VT_ERROR,"S Codes"},
  {VT_BOOL,"Boolean"},
  {VT_VARIANT,"Variant *"},
  {VT_UNKNOWN,"IUnknown"},
  {VT_DECIMAL,"Decimal"},
  {VT_I1,"Char"},
  {VT_UI1,"Unsigned Char"},
  {VT_UI2,"Unsigned Short Int"},
  {VT_UI4,"Unsigned Int"},
  {VT_I8,"signed 64-bit int"},
  {VT_UI8,"unsigned 64-bit int"},
  {VT_INT,"signed machine int"},
  {VT_UINT,"unsigned machine int"},
  {VT_VOID,"C style void"},
  {VT_HRESULT,"Standard return type (HRESULT)"},
  {VT_PTR,"pointer type"},
  {VT_SAFEARRAY,"(use VT_ARRAY in VARIANT)"},
  {VT_CARRAY,"C style array"},
  {VT_USERDEFINED,"user defined type"},
  {VT_LPSTR,"null terminated string"},
  {VT_LPWSTR,"wide null terminated string"},
  {VT_RECORD,"Record"},
  {VT_FILETIME,"FILE TIME"},
  {VT_BLOB,"Length prefixed bytes"},
  {VT_STREAM,"Name of the stream follows"},
  {VT_STORAGE,"Name of the storage follows"},
  {VT_STREAMED_OBJECT,"Stream contains an object"},
  {VT_STORED_OBJECT,"Storage contains an object"},
  {VT_BLOB_OBJECT,"Blob contains an object"},
  {VT_CF,"Clipboard format"},
  {VT_CLSID,"A Class ID"},
  {(DWORD)0,(char *)NULL}
  };


FL DispExGetDispID[] = {          // H - orizontal flags
  {fdexNameCaseSensitive,"Case Sensitive"},
  {fdexNameEnsure,"Ensure"},      //  Add Name to you known pool.  (Ensure it is Persistent)
                                  // part of 'Dynamic Object'support.
  {fdexNameImplicit,"Implicit"},
  {fdexNameCaseInsensitive,"Case Insensitive"},
  {(DWORD)0,(char *)NULL}
  };


FL DispExMembProp[] = {           // H - orizontal flags
  {fdexPropCanGet,"Can Get"},
  {fdexPropCannotGet,"Cannot Get"},
  {fdexPropCanPut,"Can Put"},
  {fdexPropCannotPut,"Cannot Put"},
  {fdexPropCanPutRef,"Can Put by Ref"},
  {fdexPropCannotPutRef,"Cannot Put by Ref"},
  {fdexPropNoSideEffects,"No Side Effects"},
  {fdexPropDynamicType,"Dynamic Type"},
  {fdexPropCanCall,"Can Call"},
  {fdexPropCannotCall,"Cannot Call"},
  {fdexPropCanConstruct,"Can Construct"},
  {fdexPropCannotConstruct,"Cannot Construct"},
  {fdexPropCanSourceEvents,"Can Source Events"},
  {fdexPropCannotSourceEvents,"Cannot Source Events"},
  {(DWORD)0,(char *)NULL}
  };


FL DispExNextDispID[] = {         // V - ertical flags
  {fdexEnumDefault,"Default"},
  {fdexEnumAll,"All"},
  {(DWORD)0,(char *)NULL}
  };


FL DispExInvokDispID[] = {        // H - orizontal flags
  {DISPATCH_CONSTRUCT,"Use Member as a Constructor"},
  {(DWORD)0,(char *)NULL}
  };


FL MSDispIDs[] = {        // V - ertical flags
  {DISPID_UNKNOWN,"IUnknown pointer"},
  {DISPID_VALUE,"Activate default value or property"},
  {DISPID_PROPERTYPUT,"Property PUT"},
  {DISPID_NEWENUM,"Enum"},
  {DISPID_EVALUATE,"Excel style evaluate"},
  {DISPID_CONSTRUCTOR,"C++ constructor"},
  {DISPID_DESTRUCTOR,"C++ destructor"},
  {DISPID_COLLECT,"Invoke through an accessor"},
  {DISPID_THIS,"THIS from DispatchEx"},
  {(DWORD)0,(char *)NULL}
  };

extern FL ScriptText[];                // H - orizontal flags
extern FL ScriptProc[];                // H - orizontal flags






/******************************************************************************
*   IUnknown Interfaces -- All COM objects must implement, either directly or
*   indirectly, the IUnknown interface.
*   However, ours is performed in OrxScript.cpp, since this is just additional
*   implementation of that object.
******************************************************************************/


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
STDMETHODIMP OrxScript::GetTypeInfoCount(UINT *pTInfo)
{
    FPRINTF2(logfile,"\n");
    FPRINTF(logfile,"OrxScript::GetTypeInfoCount  -- NOT IMPLEMENTED!!!\n\n");

    //This object doesn't support type information
    *pTInfo = 0;
    return E_NOTIMPL;
}

/******************************************************************************
*   GetTypeInfo -- Returns the type information for the class.  For classes
*   that don't support type information, this function returns DISP_E_BADINDEX;
*   It would be preferable to return E_NOTIMPL, but that is not one of our options.
******************************************************************************/
STDMETHODIMP OrxScript::GetTypeInfo(UINT pTInfo, LCID plcid,
                                         ITypeInfo **pTypeInfo)
{
    FPRINTF2(logfile,"\n");
    FPRINTF(logfile,"OrxScript::GetTypeInfo  -- NOT IMPLEMENTED!!!\n\n");

    //This object doesn't support type information
    *pTypeInfo = NULL;
    return DISP_E_BADINDEX;
}


/******************************************************************************
*   GetIDsOfNames -- Takes an array of strings and returns an array of DISPID's
*   which correspond to the methods or properties indicated.  In real life,
*   If the name is not recognized, then DISP_E_UNKNOWNNAME is returned.
*   However, this is T-Rexx, we know every thing.  Log the name that is being
*   sought, and return a bogus DispID.
******************************************************************************/
STDMETHODIMP OrxScript::GetIDsOfNames(REFIID riid,
                                          OLECHAR **pNames,
                                          UINT pNamesCount,  LCID plcid,
                                          DISPID *pbDispID)
{
    HRESULT RetCode = S_OK,RC;
    UINT    i;
    char    lIID[100];


    StringFromGUID2(riid,(LPOLESTR)lIID,sizeof(lIID)/2);
    FPRINTF(logfile,"OrxScript::GetIDsOfNames\n");
    FPRINTF2(logfile,"pNamesCount %d   riid %S \n",pNamesCount,lIID);

    //check parameters
    if (riid != IID_NULL)
    {
        RetCode = E_INVALIDARG;
    }

    else
    {
        //loop through all the pNames that were passed in, and pass
        //them to the routine that deals with one name at a time.
        for (i = 0; i < pNamesCount; i++)
        {

            RC = GetDispID(pNames[i],fdexNameCaseInsensitive,&pbDispID[i]);
            if (RC != S_OK)
            {
                RetCode = RC;     //  The only returns the last bad error code.
            }

        }
    }

    //  RetCode = S_OK;
    return RetCode;
}

/******************************************************************************
*  Invoke -- Takes a dispid and uses it to call a method or property defined
*  in the script code in the context of this OrxScript.
******************************************************************************/
STDMETHODIMP OrxScript::Invoke(DISPID pDispID, REFIID riid, LCID plcid,
                                    WORD pFlags, DISPPARAMS* pDispParams,
                                    VARIANT* pVarResult, EXCEPINFO* pExcepInfo,
                                    UINT* pArgErr)
{
    FPRINTF(logfile,"OrxScript::Invoke\n");

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
  /*
      OK, I cheated.  I ripped this right out of <DISPEX.H>.
    Then I changed all of the parameter names to my convention.

    For a brief description of each of these routines, please see
    the header file.
  */
/******************************************************************************
*                 GetDispID
******************************************************************************/
  STDMETHODIMP OrxScript::GetDispID(
    /* [in] */ BSTR pName,
    /* [in] */ DWORD pFlags,               // Derived from fdexName... defines.
    /* [out] */ DISPID __RPC_FAR *pbDispID)
{
    HRESULT   RetCode= S_OK;
    void     *Property;
    DISPID    PropertyDispID;
    char      lName[MAX_PATH];
    PDID          DispIDData;


    //    N.B. (Nota Bene - Latin for read this, your life may depend on it.)
    //  The flags are ignored, and all comparisons are case sensative.
    FPRINTF(logfile,"OrxScript::GetDispID\n");
    FPRINTF2(logfile,"Name \"%S\" Flags 0x%08x\n",pName,pFlags);
    FPRINTF2(logfile,"In english the pFlags signifies:\n");
    FPRINTF2(logfile,"%s\n",FlagMeaning('H',pFlags,DispExGetDispID));

    do
    {
        if (pFlags & fdexNameEnsure)
        {      // We are not supporting the Dynamic ability to add
            RetCode = E_NOTIMPL;             // properties or methods.
            break;
        }
        if (pbDispID == NULL)
        {
            RetCode = E_POINTER;
            break;
        }

        *pbDispID = -1;
        //    Generalities are OK, it is the special cases that kill you.
        if (EventState == Searching && EventSourceName != NULL)
        {
            FPRINTF2(logfile,"Searching for an event name?  \n");
            if (wcsicmp(EventSourceName,pName) == 0)
            {
                //  Yes, we are being queried for an Event that we are looking for!
                //  We must deny knowing this, or we will be forced to provided information
                //  that we really don't have.
                RetCode = DISP_E_UNKNOWNNAME;
                break;
            }
            // For now, if it looks like we had an event call, but didn't
            // then do normal processing.  Later it may be decided that this
            // is an error.
        }
        //    This is to help enforce Rexx scoping rules.  This is set during a NoValue check.
        //  During this, Mr. Phelps, we deny all knowledge of ourserves.
        if (EventState == IMF)
        {
            RetCode = DISP_E_UNKNOWNNAME;
            break;
        }



        // Do we already have this name?
        FPRINTF2(logfile,"Searching through the list.  \n");
        RetCode = DispID.FindDispID(pName,pbDispID);
        if (EventState != NoProperties)
        {
            //  No, then see if it could be a Property.
            if (FAILED(RetCode))
            {
                //  Using W2C() instead of sprintf() since the max length specification is easier.
                W2C(lName,pName,sizeof(lName));
                Property = PropertyList.FindContent(lName);
                if (Property)
                {
                    RetCode = DispID.AddDispID(pName,0,DID::Property,Property,&PropertyDispID);
                }
                if (SUCCEEDED(RetCode))
                {
                    *pbDispID = PropertyDispID;
                }
            }
        }
        else
        {
            //  Yes, then ensure it is not a Property.
            if (SUCCEEDED(RetCode))
            {
                RetCode = DispID.FindDID(*pbDispID,&DispIDData);
                if (DispIDData->Type == DID::Property)
                {
                    RetCode = DISP_E_UNKNOWNNAME;
                    *pbDispID = -1;
                }
            }
        }

    } while (0==1);

    if (SUCCEEDED(RetCode))
    {
        FPRINTF2(logfile,"%03d - *%S*\n",(int)*pbDispID,pName);
    }
    if (FAILED(RetCode))
    {
        FPRINTF2(logfile,"A DispID for \"%S\" was not found, or created.  HRESULT = %08x\n",pName,RetCode);
    }

    return RetCode;

}


/******************************************************************************
*                 InvokeEx
******************************************************************************/
  STDMETHODIMP OrxScript::InvokeEx(
    /* [in] */ DISPID pDispID,
    /* [in] */ LCID lcid,
    /* [in] */ WORD pFlags,                // Derived from ... defines.
    /* [in] */ DISPPARAMS __RPC_FAR *pArgs,
    /* [out] */ VARIANT __RPC_FAR *pbResults,
    /* [out] */ EXCEPINFO __RPC_FAR *pbErrInfo,
    /* [unique][in] */ IServiceProvider __RPC_FAR *pCaller)
{
    FPRINTF(logfile,"OrxScript::InvokeEx\n");
    FPRINTF2(logfile,"IServiceProvider %p\n",pCaller);
    return CommonInvoke(pDispID, lcid, pFlags, pArgs, pbResults, pbErrInfo);
}





/******************************************************************************
*                 DeleteMemberByName
******************************************************************************/
  STDMETHODIMP OrxScript::DeleteMemberByName(
    /* [in] */ BSTR pName,
    /* [in] */ DWORD pFlags)                // Derived from fdexName... defines.
{
    FPRINTF(logfile,"OrxScript::DeleteMemberByName\n");
    FPRINTF2(logfile,"Name \"%S\" Flags 0x%08x\n",pName,pFlags);
    FPRINTF2(logfile,"In english the pFlags signifies:\n");
    FPRINTF2(logfile,"%s\n",FlagMeaning('H',pFlags,DispExGetDispID));
    return E_NOTIMPL;
}



/******************************************************************************
*                 DeleteMemberByDispID
******************************************************************************/
  STDMETHODIMP OrxScript::DeleteMemberByDispID(
    /* [in] */ DISPID pDispID)
{
    FPRINTF2(logfile,"\n");
    FPRINTF(logfile,"OrxScript::DeleteMemberByDispID  -- NOT IMPLEMENTED!!!\n\n");
    FPRINTF2(logfile,"DispID %ld \n",pDispID);
    return E_NOTIMPL;
}



/******************************************************************************
*                 GetMemberProperties
******************************************************************************/
  STDMETHODIMP OrxScript::GetMemberProperties(
    /* [in] */ DISPID pDispID,
    /* [in] */ DWORD pFetchFlag,           // Derived from ???... defines.
    /* [out] */ DWORD __RPC_FAR *pbProperties)  // Derived from fdexProp... defines.
{
    FPRINTF2(logfile,"\n");
    FPRINTF(logfile,"OrxScript::GetMemberProperties  -- NOT IMPLEMENTED!!!\n\n");
    FPRINTF2(logfile,"DispID %ld   Flags %08x\n ",pDispID,pFetchFlag);
    return E_NOTIMPL;
}


/******************************************************************************
*                 GetMemberName
******************************************************************************/
  STDMETHODIMP OrxScript::GetMemberName(
    /* [in] */ DISPID pDispID,
    /* [out] */ BSTR __RPC_FAR *pbName)
{
    HRESULT RetCode;
    OLECHAR *Name;


    FPRINTF(logfile,"OrxScript::GetMemberName\n");
    FPRINTF2(logfile,"DispID %ld ",pDispID);
    //  Print the saved DispIDName.
    RetCode = DispID.FindName(&Name,pDispID);
    if (SUCCEEDED(RetCode))
    {
        FPRINTF2(logfile,"\"%S\"\n",Name);
        *pbName = SysAllocString(Name);
        RetCode = S_OK;
    }
    else
    {
        *pbName = SysAllocString(L"");
        RetCode = DISP_E_MEMBERNOTFOUND;
    }
    FPRINTF(logfile,"OrxScript::GetMemberName - returning \"%S\"\n",*pbName);
    return RetCode;
}



/******************************************************************************
*                 GetNextDispID
******************************************************************************/
  STDMETHODIMP OrxScript::GetNextDispID(
    /* [in] */ DWORD pFlags,               // Derived from fdexEnum... defines.
    /* [in] */ DISPID pDispID,             // Previous DispID returned.
    /* [out] */ DISPID __RPC_FAR *pbDispID)
{
    FPRINTF(logfile,"OrxScript::GetNextDispID\n");
    FPRINTF2(logfile,"DispID %ld   Flags %08x\n ",pDispID,pFlags);

    return DispID.GetNextDispID(pFlags,pDispID,pbDispID);
}



/******************************************************************************
*                 GetNameSpaceParent
******************************************************************************/
  STDMETHODIMP OrxScript::GetNameSpaceParent(
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *pbIUnknown)
{
    FPRINTF2(logfile,"\n");
    FPRINTF(logfile,"OrxScript::GetNameSpaceParent  -- NOT IMPLEMENTED!!!\n\n");

    return E_NOTIMPL;
}




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
    /* [out] */ EXCEPINFO __RPC_FAR *pbErrInfo)
{
    HRESULT       RetCode;
    int           State,ArgCount;
    DISPPARAMS    DP;
    VARIANT      *FCmd;
    DISPID       *FDispID;
    PDID          DispIDData;
    DWORD         lFlags;


    RetCode = DispID.FindDID(pDispID,&DispIDData);

    FPRINTF2(logfile,"DispID %ld ",pDispID);
    //  Print the saved DispIDName.
    if (SUCCEEDED(RetCode))
    {
        FPRINTF2(logfile,"\"%S\"\n",DispIDData->Name);
    }
    else
    {
        FPRINTF2(logfile,">>>> This DispID is not defined!\n");
    }
    FPRINTF2(logfile,"In english the pFlags signifies: %s\n", FlagMeaning('H',pFlags,DispatchInvoke));
    FPRINTF2(logfile,"pArgs %p\n",pArgs);
    FPRINTF2(logfile,"pbResults %p\n",pbResults);
    FPRINTF2(logfile,"pbErrInfo %p\n",pbErrInfo);



    if (pArgs != NULL)
    {
        PrntDispParams(logfile, pArgs);

    }  //  if(pArgs != NULL)

    /*   >>>>???<<<<
      Need to add code to examine the pExcepInfo and pArgErr parameters.
    */




    if (SUCCEEDED(RetCode))
    {
        // Put the flags in local storage so we can whack them.
        //  See if this is a property opertation.  If it is, ignore all other flags.
        //  The code below knows if this is really a simple variable, or a method.
        // If somebody calls us with both of these on, then this will fail.
        if (pFlags & (DISPATCH_PROPERTYGET+DISPATCH_PROPERTYPUT))
        {
            lFlags = pFlags &(DISPATCH_PROPERTYGET+DISPATCH_PROPERTYPUT);
        }
        else
        {
            lFlags = pFlags & DISPATCH_METHOD;      //  Otherwise, ignore all flags, but method.
        }

        // BUT wait!!!  If this is a Method, And its really a property, we have some thinking to do.
        // This happens when someone codes: Object~Property("New Value")
        if (lFlags == DISPATCH_METHOD && DispIDData->Type == DID::Property)
        {
            ArgCount = pArgs->cArgs - pArgs->cNamedArgs;
            /*   >>>>???<<<<
              Currently, there is a BIG BUG in the DISPPARAMS the WSH passes us.  cArgs is
            the number of arguments only, not the total of arguments and named arguments.
            DETECT and correct for this bug!
            */
            if (pArgs->cNamedArgs > 0)
                if (pArgs->rgdispidNamedArgs[0] == DISPID_PROPERTYPUT)
                {
                    ++ArgCount;
                }
                /*   >>>>???<<<<
                   End BIG BUG fix.....
                */
            if (ArgCount > 0)
            {
                lFlags = DISPATCH_PROPERTYPUT;
            }
            else
            {
                lFlags = DISPATCH_PROPERTYGET;
            }
        }

        switch (lFlags)
        {
            case DISPATCH_METHOD:
                //  IActiveScriptSite OnEnterScript()/OnLeaveScript() called by InvokeMethod().
                RetCode = InvokeMethod(DispIDData, pArgs, pbResults, pbErrInfo);
                break;
                //  This satisfies someone asking us for properties.  See Eng2Rexx for when
                // Rexx wants a a property.
            case DISPATCH_PROPERTYGET:
                RetCode = E_FAIL;
                if (pbResults)
                {
                    V_VT(pbResults) = VT_ERROR;
                }
                else
                {
                    break;
                }
                // If this is a function, let the user take care of it.
                if (DispIDData->Type == DID::Function)
                {
                    RetCode = AddMutant(L"GET",pArgs,&DP);
                    if (SUCCEEDED(RetCode))
                    {
                        RetCode = InvokeMethod(DispIDData, &DP, pbResults, pbErrInfo);
                    }
                    FCmd = DP.rgvarg;
                    VariantClear(&FCmd[pArgs->cArgs]);  // freestring
                    delete FCmd;                        // Remove the created command list
                }
                if (DispIDData->Type == DID::Property)
                {
                    VariantInit(pbResults);
                    // Copy the Variant pointed to by RexxCode to pbResults.
                    RetCode = VariantCopy(pbResults,(VARIANT *)&(((PGVARIANT)(DispIDData->RexxCode))->Mutant));
                }
                break;
                //  This satisfies someone sets one of our properties.  See Eng2Rexx for when
                // Rexx sets a a property.
            case DISPATCH_PROPERTYPUT:
                RetCode = E_FAIL;
                if (pbResults)
                {
                    V_VT(pbResults) = VT_ERROR;
                }
                // If this is a function, let the user take care of it.
                if (DispIDData->Type == DID::Function)
                {
                    /*   >>>>???<<<<
                      Currently, there is a BIG BUG in the DISPPARAMS the WSH passes us.  cArgs is
                    the number of arguments only, not the total of arguments and named arguments.
                    DETECT and correct for this bug!
                    */
                    if (DISPATCH_PROPERTYPUT == pFlags && pArgs->cNamedArgs > 0)
                    {
                        if (pArgs->rgdispidNamedArgs[0] == DISPID_PROPERTYPUT)
                        {
                            ++(pArgs->cArgs);
                        }
                    }
                        /*   >>>>???<<<<
                           End BIG BUG fix.....
                        */
                    State = 0;
                    RetCode = DropNamedPut(pArgs,&DP);
                    if (SUCCEEDED(RetCode))
                    {
                        State = 1;
                        RetCode = AddMutant(L"PUT",&DP,&DP);
                    }
                    if (SUCCEEDED(RetCode))
                    {
                        State = 2;
                        RetCode = InvokeMethod(DispIDData, &DP, pbResults, pbErrInfo);
                    }
                    FDispID = DP.rgdispidNamedArgs;
                    FCmd = DP.rgvarg;
                    if (State == 1 && FDispID && DP.cNamedArgs != pArgs->cNamedArgs)
                    {
                        delete FDispID;
                    }
                    if (State == 2)
                    {
                        VariantClear(&FCmd[DP.cArgs-DP.cNamedArgs]);  // freestring
                        delete FCmd;                                  // Remove the created command list
                    }
                }
                if (DispIDData->Type == DID::Property)
                {
                    if (pbResults)
                    {     //  If they are looking for something back, then give them the prePUT version.
                        VariantInit(pbResults);
                        VariantCopy(pbResults,(VARIANT *)&(((PGVARIANT)(DispIDData->RexxCode))->Mutant));
                    }
                    VariantClear((VARIANT *) &(((PGVARIANT)(DispIDData->RexxCode))->Mutant));
                    // Copy the Variant pointed to by RexxCode to pbResults.
                    FCmd = pArgs->rgvarg;
                    RetCode = VariantCopy((VARIANT *) &(((PGVARIANT)(DispIDData->RexxCode))->Mutant),&FCmd[pArgs->cArgs - 1]);
                }
                break;
            case DISPATCH_PROPERTYPUTREF:
            case DISPATCH_CONSTRUCT:
            default:
                RetCode = E_NOTIMPL;   // Let the caller know we did not do anything.
                FPRINTF2(logfile,"This type of Invoke is not currently supported.\n");
        }

    }

    FPRINTF2(logfile,"CommonInvoke()  Exit  HRESULT = %08x\n",RetCode);

    return RetCode;
}


/******************************************************************************
*                 InvokeMethod
*
*   When called by the host treat the DispID as a METHOD (as opposed to a
*  PROPERTY) this code takes care of all the details needed to load and
*  execute the function, or event.
******************************************************************************/
  STDMETHODIMP OrxScript::InvokeMethod(
    /* [in]  */ PDID pDIDData,
    /* [in]  */ DISPPARAMS __RPC_FAR *pArgs,
    /* [out] */ VARIANT __RPC_FAR *pbResults,
    /* [out] */ EXCEPINFO __RPC_FAR *pbErrInfo)
{
    HRESULT       RetCode=S_OK;
    OLECHAR       invokeString[4096],*Invocation = invokeString;
    char          lName[251],tInvokeString[4096],*FInvokeString,*Temp=NULL,NameList[MAX_PATH],*tNL;
    LPVOID        arguments[8];
    RexxConditionData cd;
    DISPPARAMS    dp;
    VARIANT       sResult,*mResult;
    VARIANTARG    *FCmd=NULL;
    int           ArgCount,ISMaxLen,NameListLen;

    RexxThreadContext *context = ScriptProcessEngine::getThreadContext();

    // There had better be some RexxCode to execute.
    if (NULL == pDIDData->RexxCode)
    {
        if (pbResults)
        {
            V_VT(pbResults) = VT_ERROR;
        }
        return E_UNEXPECTED;
    }

    pActiveScriptSite->OnEnterScript();

    //  If things work right, the invocation string will be generated onto
    // the local storage.  If not, such as when a genetic mutant tries to
    // pass 900 arguments, then there are provisions to build it in dynamic
    // memory.
    //   When the routines that are building the parms encounter a situation
    // where dynamic memory must be used, they do not delete the pointer to
    // the string that was passed to them.  They leave that for the caller.
    // This was local area's can be passed.  As the caller, we only need to
    // delete the string if its pointer is not that of the local area.
    FInvokeString = tInvokeString;
    FInvokeString[0] = '\0';
    tNL = NameList;
    tNL[0] = '\0';
    ISMaxLen = sizeof(tInvokeString);
    NameListLen = sizeof(NameList);
    RetCode = InvokeNamedParms(pArgs,&FInvokeString,&ISMaxLen,&tNL,&NameListLen);
    FPRINTF2(logfile,"InvokeString \"%s\" \n",FInvokeString);

    Temp = FInvokeString;
    //  ArgCount is the number of args that we are passing to the routine.
    //  This is the number of args passed to us, minus any named args.
    ArgCount = pArgs->cArgs - pArgs->cNamedArgs;

    RexxThreadContext *context = ScriptProcessEngine::getThreadContext();

    /*
     *   The following executes code that is specific to each type of entrypoint that received code.
     */
    if (SUCCEEDED(RetCode)) switch (pDIDData->Type)
        {
            case DID::Function:
                FPRINTF2(logfile,"InvokeMethod()  Processing a function (METHOD in MS parlance), ParseScriptText flags were %s. \n",
                         FlagMeaning('H',pDIDData->Flags,ScriptText));
                FPRINTF2(logfile,"RexxCodeBlock is %p. Running it:\n",pDIDData->RexxCode);
                break;

                //  At the moment the two types of events are seperated out because the interpretation
                // of the flags are different.  (Even though the flag is not currently used for anything)
            case DID::ASEvent :
                FPRINTF2(logfile,"InvokeMethod()  Processing an AddScriptlet EVENT, Event flags were %s. \n",
                         FlagMeaning('H',pDIDData->Flags,ScriptText));
                if (pArgs->cArgs != 1 && pArgs->cNamedArgs != 1)
                {
                    RetCode = E_FAIL; break;
                }
                if (pArgs->rgdispidNamedArgs[0] != DISPID_THIS)
                {
                    RetCode = E_FAIL; break;
                }
                break;

            case DID::LPPEvent:
                FPRINTF2(logfile,"InvokeMethod()  Processing a LocalParseProcedure EVENT\n");
                FPRINTF2(logfile,"RexxCodeBlock is %p. Running it:\n",pDIDData->RexxCode);
                break;

            case DID::PPEvent :
                FPRINTF2(logfile,"InvokeMethod()  Processing an ParseProcedure EVENT, Event flags were %s. \n",
                         FlagMeaning('H',pDIDData->Flags,ScriptProc));
                FPRINTF2(logfile,"RexxCodeBlock is %p. Running it:\n",pDIDData->RexxCode);
                break;
            default:                   // Theoretically we shouldn't get here.
                RetCode = E_FAIL;        //  So, if we do, make theory match reality.
        }

    /*
     *   Continuation of the joint code.
     */
    if (SUCCEEDED(RetCode))
    {
        if (pbResults != NULL)
        {
            mResult = pbResults;
        }
        else
        {
            mResult = &sResult;
        }
        VariantInit(mResult);

        // convert the arguments to Rexx objects for the call
        RexxArrayObject args = dispParms2RexxArray(&dp);

        runMethod(context, this, pDIDData->RexxCode, args, mResult, cd);
        FPRINTF2(logfile,"Rexx returned the following:\n");
        PrntMutant(logfile, mResult);

        if (FCmd)
        {
            if (pArgs->cNamedArgs > 0)
            {
                // only release THIS if the execution was caused by an event -
                // for a call as a method, THIS is probably not AddRef'd by us.
                if (pArgs->rgdispidNamedArgs[0] == DISPID_THIS && pDIDData->Type != DID::Function)
                {
                    // >>> ??? <<<  If we keep this code, in the AddScriptlet Events where it build THIS, AddRef() it.
                    pArgs->rgvarg[0].pdispVal->Release();
                    FPRINTF2(logfile,"I have released THIS!!! \n");
                }
            }
            VariantClear(&FCmd[ArgCount]);  // freestring
            delete FCmd;
        }
        if (tNL != NameList)
        {
            delete tNL;
        }

        if (cd.rc == 0)
        {
            FPRINTF2(logfile,"done running RCB %p\n",pDIDData->RexxCode);
            //  If the call was made as a Return ???(), and it did not actually return
            // anything, then Rexx sets the return to VT_ERROR with the condition code 0.
            // We do not know how the caller invoked us, so we turn this back into VT_EMPTY.
            if (pbResults)
            {
                if (V_VT(pbResults) == VT_ERROR)
                {
                    pbResults->vt = VT_EMPTY;
                }
            }
        }
    }
    else if (pbResults)
    {
        V_VT(pbResults) = VT_ERROR;
    }

    // we're done with this thread context
    context->DetachThread();

    pActiveScriptSite->OnLeaveScript();

    return RetCode;
}
