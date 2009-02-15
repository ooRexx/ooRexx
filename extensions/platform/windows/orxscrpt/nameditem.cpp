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


#include "nameditem.hpp"


OrxNamedItem::OrxNamedItem(OrxScript *pEngine, FILE *Stream) : Engine(pEngine), logfile(Stream)  {;}   // CTOR


OrxNamedItem::~OrxNamedItem(){


#if defined(DEBUGC)+defined(DEBUGZ)
FPRINTF2(logfile,"OrxNamedItem::~() \n");
#endif
  Items.DeleteList();
  }



STDMETHODIMP OrxNamedItem::AddItem(LPCOLESTR pName, DWORD pFlags, IUnknown *pIUnk, ITypeInfo *pITyIn)
{
    size_t    Len;
    PNID      Next;
    ListItem *Item;
    OLECHAR  *lName;
    HRESULT   RetCode;
    char      Name[MAX_PATH];
    long   CurCount=0;



    if (pName == NULL)
    {
        pName = L"#TypeLib";
    }

    FPRINTF2(logfile,"OrxNamedItem::AddItem() Saving \"%S\"\n",pName);
    Len = wcslen(pName);

    Next = (PNID)GlobalAlloc(GMEM_FIXED,(sizeof(wchar_t)*(Len+2))+sizeof(NID));
    sprintf(Name,"%S",pName);
    Item = Items.AddItem(Name,LinkedList::End,(void *)Next);
    if (Next && Item)
    {
        lName = (OLECHAR *)((char *)Next + sizeof(NID));
        wcscpy(lName,pName);
        Next->Name     = lName;
        Next->Flags    = pFlags;
        Next->Unknown  = pIUnk;
        Next->TypeInfo = pITyIn;
        Next->Dispatch = NULL;
        Next->DispID   = -1;
        // QueryInterface does an AddRef() on our behalf, so don't forget to Release() it.
        if (Next->Unknown)
        {
            RetCode = Next->Unknown->QueryInterface(IID_IDispatch, (void **)&(Next->Dispatch));
//      CurCount = Next->Dispatch->AddRef();
//FPRINTF2(logfile,"OrxNamedItem::AddItem() Dispatch refcount %d \n",CurCount);
        }

        if (Next->Unknown)
        {
            IDispatchEx *DispEx;
            RetCode = Next->Unknown->QueryInterface(IID_IDispatchEx, (void **)&DispEx);
            if (SUCCEEDED(RetCode))
            {
                EnumerateProperty(logfile,DispEx ,0);
                DispEx->Release();
            }
            else FPRINTF2(logfile,"Named Item does not support IDispatchEx::\n");
        }
        Next->Ex = false;
        RetCode = S_OK;
    }
    else
    {
        if (Next) GlobalFree((HGLOBAL)Next);
        if (Item) Items.DropItem(Item);
        RetCode = E_OUTOFMEMORY;
    }

    FPRINTF2(logfile,"OrxNamedItem::AddItem() Leaving with a HRESULT of %08x\n",RetCode);
    return RetCode;
}



STDMETHODIMP OrxNamedItem::AddItem(LPCOLESTR pName, DWORD pFlags, IDispatch *pIDisp, ITypeInfo *pTypeInfo, DISPID pDispID, PNID *pbNamedItem)
{
    size_t    Len;
    PNID      Next;
    ListItem *Item;
    OLECHAR  *lName;
    HRESULT   RetCode;
    char      Name[MAX_PATH];

    FPRINTF2(logfile,"OrxNamedItem::AddItem(DISPID) Saving \"%S\"\n",pName);
    Len = wcslen(pName);
    Next = (PNID)GlobalAlloc(GMEM_FIXED,(sizeof(wchar_t)*(Len+2))+sizeof(NID));
    sprintf(Name,"%S",pName);
    Item = Items.AddItem(Name,LinkedList::End,(void *)Next);
    *pbNamedItem = NULL;
    if (Next && Item)
    {
        lName = (OLECHAR *)((char *)Next+sizeof(NID));
        wcscpy(lName,pName);
        Next->Name     = lName;
        Next->Flags    = pFlags;
        Next->Dispatch = pIDisp;
        Next->DispID   = pDispID;
        Next->Ex = false;
        Next->Unknown  = NULL;
        Next->TypeInfo = pTypeInfo;
        *pbNamedItem = Next;
        RetCode = S_OK;
    }
    else
    {
        if (Next) GlobalFree((HGLOBAL)Next);
        if (Item) Items.DropItem(Item);
        RetCode = E_OUTOFMEMORY;
    }

    FPRINTF2(logfile,"OrxNamedItem::AddItem(DISPID) Leaving with a HRESULT of %08x\n",RetCode);
    return RetCode;
}



//  >>> ??? <<<  Why didn't I want flags passed in for comparison purposes?
STDMETHODIMP OrxNamedItem::WhoKnows(
  /* in  */ const char *pName,         // SubItem name
  /* in  */ LCID      Lang,            // User Language code (English, Deutsch, etc.)
  /* out */ DISPID    *pbDispID,       // DispID of the SubItem
  /* out */ DWORD     *pbFlags,        // Flags stored with the Item, or SubItem
  /* out */ IDispatch **pbDispatch,    // IDispatch ptr of the Item that knows the SubItem.
  /* out */ ITypeInfo **pbTypeInfo)    // ITypeInfo ptr that contains the found variable (constant)
{
    PNID      NamedItem;
    HRESULT   RetCode;
    OLECHAR   lName[MAX_PATH];
    OLECHAR   *aName=lName;
    DISPID    DispID;
    IDispatchEx *DispEx;


    FPRINTF2(logfile,"OrxNamedItem::WhoKnows() looking for \"%s\"\n",pName);
    *pbDispatch = NULL;
    *pbTypeInfo = NULL;
    *pbDispID = (DISPID) -1;
    NamedItem = (PNID)Items.FindContent(pName);
    if (!NamedItem)
    {
        C2W(lName,pName,strlen(pName)+1);
        RetCode = Engine->GetIDsOfNames(IID_NULL, (OLECHAR **)&aName, 1, LOCALE_USER_DEFAULT, &DispID);
        if (SUCCEEDED(RetCode))
        {
            *pbDispatch = Engine;
            *pbDispID = DispID;
            RetCode = S_OK;
            return RetCode;
        }
        NamedItem = (PNID)Items.FindContent(0);
        while (NamedItem)
        {
            // special name => item was added by AddTypeLib
            if (!wcscmp(NamedItem->Name,L"#TypeLib"))
            {
                RetCode = (NamedItem->TypeInfo)->GetIDsOfNames((OLECHAR **)&aName, 1, &DispID);
                FPRINTF2(logfile,"OrxNamedItem::WhoKnows()  [ITypeInfo::]GetIDsOfNames() for \"%s\" returned: DispId %d HRESULT %08x\n",
                         pName,DispID,RetCode);
                if (SUCCEEDED(RetCode))
                {
                    RetCode = AddItem(lName,0,NULL/*NamedItem->Dispatch*/,NamedItem->TypeInfo,DispID,&NamedItem);
                    FPRINTF2(logfile,"OrxNamedItem::WhoKnows()  Added \"%s\" to the NamedItemList DispId %d HRESULT %08x\n",
                             pName,DispID,RetCode);
                    break;
                }

            }
            //  Only ask Item's if they know a name.
            else if (NamedItem->DispID == -1)
            {
                DispID = (DISPID) -1;
                //  Use an array on one name to find the DispID.
                /* this should be checked in general, but IIS put a named item */
                /* on the named item list that does not support automation (an */
                /* IDispatch interface) - it looks like a type library object; */
                /* IIS should actually call us with AddTypeLibrary... sigh...  */
                if (NamedItem->Dispatch)
                    RetCode = (NamedItem->Dispatch)->GetIDsOfNames(IID_NULL, (OLECHAR **)&aName, 1, LOCALE_USER_DEFAULT, &DispID);
                else
                    RetCode = DISP_E_UNKNOWNNAME;
                FPRINTF2(logfile,"OrxNamedItem::WhoKnows()  GetIDsOfNames() for \"%s\" returned: DispId %d HRESULT %08x\n",
                         pName,DispID,RetCode);
                if (SUCCEEDED(RetCode))
                {
                    // RetCode = AddItem(lName,NamedItem->Flags,NamedItem->Dispatch,DispID);
                    RetCode = AddItem(lName,0,NamedItem->Dispatch,NULL,DispID,&NamedItem);
                    FPRINTF2(logfile,"OrxNamedItem::WhoKnows()  Added \"%s\" to the NamedItemList DispId %d HRESULT %08x\n",
                             pName,DispID,RetCode);
                    break;
                }
                else
                {
                    /* this should be checked in general */
                    if (NamedItem->Unknown)
                        RetCode = (NamedItem->Unknown)->QueryInterface(IID_IDispatchEx, (void **)&DispEx);
                    else
                        RetCode = DISP_E_UNKNOWNNAME;
                    if (SUCCEEDED(RetCode))
                    {

                        /* TODO This is just plain wrong here, including the comment.  One
                         * of the SysAllocXX functions should be used to create the BSTR and
                         * SysFreeString should be used to free it.  BSTR memory is owned
                         * by the OS and can be (and is) passed between process boundaries.
                         * Allocating local memory and then using it as a BSTR in a COM call
                         * is simply not correct.
                         */

                        // the MSDN Library defines BSTR in two(!) ways:
                        // BSTR <=> OLECHAR* and as a "VB-compatible" structure: <32-bits: length><array of ole chars>
                        //
                        // one object in JSCRIPT.DLL does not like the common way of using
                        // BSTR (OLECHAR*), so this code creates something that has the length
                        // of the OLECHAR array before the actual characters in memory...
                        size_t len = wcslen(lName);
                        OLECHAR *VB_BSTR = (OLECHAR*) malloc(4+sizeof(OLECHAR)*(len+1));
                        *((size_t *) VB_BSTR) = len;
                        memcpy(VB_BSTR+2,lName,sizeof(OLECHAR)*(len+1));
                        // this line changed: pass in pointer to OLECHARs, but have the length of the string before them...
                        RetCode = DispEx->GetDispID(VB_BSTR+2, fdexNameCaseInsensitive, &DispID);
                        free(VB_BSTR);
                        DispEx->Release();
                        if (SUCCEEDED(RetCode))
                        {
                            // RetCode = AddItem(lName,NamedItem->Flags,NamedItem->Dispatch,DispID);
                            RetCode = AddItem(lName,0,NamedItem->Dispatch,NULL,DispID,&NamedItem);
                            FPRINTF2(logfile,"OrxNamedItem::WhoKnows()  Added \"%s\" to the NamedItemList DispId %d HRESULT %08x\n",
                                     pName,DispID,RetCode);
                            break;
                        }
                    }
                }
            }     // if(NamedItem->DispID == -1)
            NamedItem = (PNID)Items.FindContent();
        }
    }
    else
    {
        DispID = NamedItem->DispID;
        FPRINTF2(logfile,"OrxNamedItem::WhoKnows()  \"%s\" was already on the  NamedItemList DispId %d \n",
                 pName,DispID);
    }

    if (NamedItem)
    {
        *pbDispatch = NamedItem->Dispatch;
        *pbTypeInfo = NamedItem->TypeInfo;
        *pbFlags = NamedItem->Flags;
        *pbDispID = DispID;
        RetCode = S_OK;
    }
    else
    {
        RetCode = DISP_E_UNKNOWNNAME;
    }

    FPRINTF2(logfile,"OrxNamedItem::WhoKnows() Leaving with a HRESULT of %08x\n",RetCode);
    return RetCode;
}


void NILList::DropContent(
  /*  in   */ void *Content)
{
    OrxNamedItem::PNID    NamedItem;
    long CurCount=0;


    if (Content)
    {
        //  First release interfaces that we had opened.
        NamedItem = (OrxNamedItem::PNID)Content;
        // Only release Item pointers, not SubItems.
        if (NamedItem->DispID == -1)
        {
            if (NamedItem->Unknown) CurCount = (NamedItem->Unknown)->Release();
            if (NamedItem->TypeInfo) CurCount = NamedItem->TypeInfo->Release();
            if (NamedItem->Dispatch) CurCount = NamedItem->Dispatch->Release();
        }
        GlobalFree((HGLOBAL)NamedItem);
    }
}


// retrieve the names of all named items of the engine
char** OrxNamedItem::getNamedItems(int *num)
{
    PNID pItem;
    int  i = 0;
    int  j = 0;
    int  size = 10;
    char **result = (char**) malloc(sizeof(char*)*size);

    do
    {
        pItem = (PNID) Items.FindContent(i);
        if (pItem)
        {
            // the item must have a) a IDispatch interface,
            //                    b) the ISVISIBLE flag turned on, and
            //                    c) a DispID of -1 indicating a "top level member" of the list
            if (pItem->Dispatch != NULL &&
                (pItem->Flags & SCRIPTITEM_ISVISIBLE) &&
                pItem->DispID == -1)
            {
                // get its name
                char *tmp = Items.GetName(Items.FindItem(i));
                result[j] = (char*) calloc((strlen(tmp)+1),sizeof(char));
                strcpy(result[j], tmp);
                j++;
                // if the array is too small, double it with realloc
                if (j == size)
                {
                    result = (char**) realloc(result, size*2*sizeof(char*));
                    size *= 2;
                }
            }
        }
        i++;
    } while (pItem);

    *num = j;
    return result;
}
