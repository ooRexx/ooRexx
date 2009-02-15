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
#include "OrxDispID.hpp"

OrxDispID::~OrxDispID(){


  Chain.DeleteList();
  }



/******************************************************************************
*
*     AddDispID
*
******************************************************************************/
STDMETHODIMP OrxDispID::AddDispID(OLECHAR *pName, DWORD Flags, DID::DType Type, void *RexxCode, DISPID *pbDispID)
{
    size_t    Len;
    PDID      Next;
    ListItem *Item;
    OLECHAR  *lName;
    HRESULT   RetCode;
    char      Name[MAX_PATH];
    int       lCount;


    InterlockedIncrement((long *)&Count);
    lCount = Count;

    //  Save the DispIDName to print when this DispID is Invoke(Ex)d.
    //  Leave it in wide characters for IDispatchEx and other functions.
    Len = wcslen(pName);
    Next = (PDID)GlobalAlloc(GMEM_FIXED,(sizeof(wchar_t)*(Len+2))+sizeof(DID));
    sprintf(Name,"%d",lCount);
    Item = Chain.AddItem(Name,LinkedList::End,(void *)Next);
    if (Next && Item)
    {
        lName = (OLECHAR *)((char *)Next+sizeof(DID));
        wcscpy(lName,pName);
        Next->Name   = lName;
        Next->Flags  = Flags;
        Next->Type   = Type;
        Next->RexxCode = RexxCode;  // warning, might be NULL because not all functions (e.g. AS) are implemented yet
        *pbDispID = (DISPID)lCount;
        RetCode = S_OK;
    }
    else
    {
        if (Next)
        {
            GlobalFree((HGLOBAL)Next);
        }
        if (Item)
        {
            Chain.DropItem(Item);
        }
        RetCode = E_OUTOFMEMORY;
        *pbDispID = (DISPID)0;
    }

    FPRINTF2(CurrentObj_logfile,"AddDispID() Leaving with a HRESULT of %08x\n",RetCode);
    return RetCode;
}

/******************************************************************************
*
*     FindDispID
*
******************************************************************************/
STDMETHODIMP OrxDispID::FindDispID(OLECHAR *pName, DISPID *pbDispID)
{
    ListItem *CurrItem;
    PDID      CurrDID;
    int       DispID;


    *pbDispID = (DISPID)0;
    CurrItem = Chain.FindItem(0);
    while (CurrItem)
    {
        CurrDID = (PDID)CurrItem->GetContent();
        if (CurrDID->Name != NULL) if (wcsicmp(pName,CurrDID->Name) == 0)
            {
                sscanf(CurrItem->GetName(),"%d",&DispID);
                *pbDispID = (DISPID)DispID;
                FPRINTF2(CurrentObj_logfile,"OrxDisp::FindDispID() Found %S, DispID %d\n",pName,DispID);
                return S_OK;
            }
        CurrItem = Chain.FindItem();
    }
    return DISP_E_UNKNOWNNAME;
}

/******************************************************************************
*
*     FindName
*
******************************************************************************/
STDMETHODIMP OrxDispID::FindName(OLECHAR **pbName, DISPID pDispID)
{
    HRESULT   RetCode;
    PDID      Current;
    char      Name[MAX_PATH];


    *pbName = NULL;
    sprintf(Name,"%d",(int)pDispID);
    Current = (PDID)Chain.FindContent(Name);
    if (Current)
    {
        *pbName = Current->Name;
        RetCode = S_OK;
    }
    else RetCode = DISP_E_UNKNOWNNAME;
    FPRINTF2(CurrentObj_logfile,"OrxDisp::FindName() Leaving with a HRESULT of %08x\n",RetCode);
    return RetCode;
}

/******************************************************************************
*
*     FindDID
*
******************************************************************************/
STDMETHODIMP OrxDispID::FindDID(DISPID pDispID, PDID *pbDispIDData)
{
    HRESULT   RetCode;
    PDID      Current;
    char      Name[MAX_PATH];


    *pbDispIDData = NULL;
    sprintf(Name,"%d",(int)pDispID);
    Current = (PDID)Chain.FindContent(Name);
    if (Current)
    {
        *pbDispIDData = Current;
        RetCode = S_OK;
    }
    else RetCode = DISP_E_UNKNOWNNAME;
    FPRINTF2(CurrentObj_logfile,"OrxDisp::FindDID() Leaving with a HRESULT of %08x\n",RetCode);
    return RetCode;
}



/******************************************************************************
*                 GetNextDispID
******************************************************************************/
STDMETHODIMP OrxDispID::GetNextDispID(
  /* [in]  */ DWORD pFlags,               // Derived from fdexEnum... defines.
  /* [in]  */ DISPID pDispID,             // Previous DispID returned.
  /* [out] */ DISPID __RPC_FAR *pbDispID)   // Next DispID or -1.
{
    ListItem *Current;
    char      Name[MAX_PATH];


    FPRINTF(CurrentObj_logfile,"OrxDispID::GetNextDispID\n");
    FPRINTF2(CurrentObj_logfile,"DispID %ld   Flags %08x\n ",pDispID,pFlags);
    if (!pbDispID)
    {
        return E_POINTER;
    }
    *pbDispID = -1;
    sprintf(Name,"%d",(int)pDispID);
    Current = (ListItem *)Chain.FindItem(Name);
    if (Current)
    {
        if (pDispID != -1)
        {
            Current = (ListItem *)Chain.FindItem(); // Get the Next DispID.
        }
        // *pbDispID = Current->DispID;
        sscanf(Current->GetName(),"%d",pbDispID);
    }

    FPRINTF2(CurrentObj_logfile,"OrxDispID::GetNextDispID - returning %d\n",*pbDispID);
    return S_OK;
}

