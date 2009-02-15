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
#include "scriptutil.hpp"



/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/
ListItem::ListItem()
{
    Name = NULL;
    Content = NULL;
    FwdPtr = this;
    BckPtr = this;

    return;
}


ListItem::~ListItem()
{
    ListItem *Previous,*Next;


    if (Name)
    {
        free(Name);
    }
    Name = NULL;
    Content = NULL;
    Next = FwdPtr;
    Previous = BckPtr;
    Previous->FwdPtr = Next;
    Next->BckPtr = Previous;
}


void *ListItem::GetContent(void)
{

    return Content;
}



const char *ListItem::GetName(void)
{

    return Name;
}


/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/
LinkedList::LinkedList() : FixedSize(0),
                           ListSize(0),
                           CurrIndex(-1)
{
    InitList();
}

LinkedList::~LinkedList()
{
    FixedSize=0;
    DeleteList();
}


ListItem *LinkedList::FindItem(const char *Name)
{
    return NameSearch(Name);
}




ListItem *LinkedList::FindItem(int Index)
{
    return IndexSearch(Index);
}


ListItem *LinkedList::FindItem(void)
{
    return IndexIncrement();
}


const char *LinkedList::GetName(ListItem *Item)
{
    return Item->Name;
}



void *LinkedList::FindContent(const char *Name)
{
    ListItem *Item;

    //  This has to be seperated out so that a check for the existence
    //  is made before the pointer for Content is obtained.  Otherwise,
    //  getting the Content of NULL can be very rude.
    if (Item = NameSearch(Name))
    {
        return Item->Content;
    }
    return NULL;
}



void *LinkedList::FindContent(
  /*  in   */ int Index)
{
    ListItem *Item;


    //  This has to be seperated out so that a check for the existence
    //  is made before the pointer for Content is obtained.  Otherwise,
    //  getting the Content of NULL can be very rude.
    if (Item = IndexSearch(Index))
    {
        return Item->Content;
    }
    return NULL;
}



void *LinkedList::FindContent(void)
{
    ListItem *Item;

    //  This has to be seperated out so that a check for the existence
    //  is made before the pointer for Content is obtained.  Otherwise,
    //  getting the Content of NULL can be very rude.
    if (Item = IndexIncrement())
    {
        return Item->Content;
    }
    return NULL;
}



void *LinkedList::GetContent(ListItem *Item)
{
    return Item->Content;
}



void LinkedList::DropContent(void *Content)
{

    if (Content)
    {
        GlobalFree((HGLOBAL)Content);
    }
}


/*  The non-trivial methods follow.   */
/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/

ListItem *LinkedList::AddItem(
  /*  in   */ char          *Name,
  /*  in   */ InsertionPoint Place,
  /*  in   */ void          *Content)
{
    ListItem *Current,*Next;
    int       Len;

    if (Name)
    {
        Len = strlen(Name);
    }
    else
    {
        Len = 0;
    }
    Current = new ListItem;
    if (!Current)
    {
        return Current;
    }
    Current->Name = (char *)malloc(Len+2);
    if (!Current->Name)
    {
        delete Current;
        Current = NULL;
        return Current;
    }
    if (Name)
    {
        strcpy(Current->Name,Name);
    }
    else
    {
        Current->Name[0] = '\0';
    }
    Current->Content = Content;

    if (Place == Beginning)
    {
        Next = MasterLink.FwdPtr;
        MasterLink.FwdPtr = Current;
        Current->FwdPtr = Next;
        Current->BckPtr = Next->BckPtr;
        Next->BckPtr = Current;
        //  Keep the index with the item it points to.
        if (CurrIndex != -1)
        {
            ++CurrIndex;
        }
    }
    else
    {
        Next = MasterLink.BckPtr;
        MasterLink.BckPtr = Current;
        Current->BckPtr = Next;
        Current->FwdPtr = Next->FwdPtr;
        Next->FwdPtr = Current;
    }

    ListSize++;
    return Current;
}



/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/
void LinkedList::DropItem(ListItem *Item)
{
    if (Item == &MasterLink || Item == NULL)
        return;

    if (Item->Content)
        this->DropContent(Item->Content);
    ListSize--;
/*
  //  This check insures that the Index item is not the one being deleted,
  if(IndexItem == Item) CurrIndex = -1;
  //  Or that the list is not shrunk passed the index.
  else if(CurrIndex <= ListSize) CurrIndex = ListSize - 1;
*/
    //  Actually, any time an item is deleted from the list, we are at risk.
    //  It cannot be detetected if the item being deleted is before the Index.
    //  If it is, then the index must be decremented.  Since we don't know
    //  when to do this, it is best to just invalidate the index.
    CurrIndex = -1;
    delete Item;
}


/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/
void LinkedList::DropItem(void *Content)
{
    ListItem *Current;
    int CIndex = 0;


    Current = MasterLink.FwdPtr;
    while (Current != &MasterLink)
    {
        if (Current->Content == Content)
        {
            break;
        }
        Current = Current->FwdPtr;
    }

    //  If we don't find what we are looking for, leave the index alone.
    if (Current == &MasterLink)
    {
        return;
    }

    // Else, update the Index info, and return the Item pointer.
    this->DropContent(Current->Content);
    ListSize--;
    //  Actually, any time an item is deleted from the list, we are at risk.
    //  It cannot be detetected if the item being deleted is before the Index.
    //  If it is, then the index must be decremented.  Since we don't know
    //  when to do this, it is best to just invalidate the index.
    CurrIndex = -1;
    delete Current;
    return;
}


void LinkedList::DeleteList(void)
{
    ListItem *Next;
    for (;;)
    {
        Next = MasterLink.FwdPtr;
        if (Next == &MasterLink)
        {
            return;
        }
        DropItem(Next);
    }

}


void LinkedList::InitList(void)
{

    MasterLink.Name = NULL;
    MasterLink.Content = NULL;
    MasterLink.FwdPtr = &MasterLink;
    MasterLink.BckPtr = &MasterLink;

    return;
}


/*******************************************************************************
 *
 *     Performs a sequential search of the list, starting from the Master Link.
 *  At each item in the list the name is compared to the parameter.  When
 *  there is a match, then the address of that item is returned.  If no match
 *  is found,
 *
 *
 *
 *
 *
 *******************************************************************************/
ListItem *LinkedList::NameSearch(const char *Name)
{
    ListItem *Current;
    int CIndex = 0;


    Current = MasterLink.FwdPtr;
    while (Current != &MasterLink)
    {
        if (stricmp(Current->Name,Name) == 0)
        {
            break;
        }
        CIndex++;
        Current = Current->FwdPtr;
    }

    //  If we don't find what we are looking for, leave the index alone.
    if (Current == &MasterLink)
    {
        return NULL;
    }

    // Else, update the Index info, and return the Item pointer.
    CurrIndex = CIndex;
    IndexItem = Current;
    return Current;
}



/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/
ListItem *LinkedList::IndexSearch(int Index)
{
    int       EndDifference,MidDifference;
    int       StartPoint,Counter;
    ListItem *StartItem;


    if (Index < 0 || Index >= ListSize)
    {
        CurrIndex = -1;
        return NULL;
    }

    if (CurrIndex < 0)
    {
        CurrIndex = 0;
        IndexItem = MasterLink.FwdPtr;
    }


    //  Determine if the index is closer to the beginning, or end.
    EndDifference = ListSize - Index;
    if (EndDifference > Index)
    {
        Counter = 1;
        StartItem = MasterLink.FwdPtr;
        StartPoint = 0;
    }
    else
    {
        Counter = -1;
        StartItem = MasterLink.BckPtr;
        StartPoint = ListSize-1;
    }

    MidDifference = Index-CurrIndex;

    if (abs(MidDifference) < EndDifference)
    {
        Counter = MidDifference < 0 ? -1 : 1;
    }
    else
    {
        IndexItem = StartItem;
        CurrIndex = StartPoint;
    }

    if (Counter > 0)
    {
        for (;CurrIndex < Index; CurrIndex+=Counter)
        {
            IndexItem = IndexItem->FwdPtr;
        }
    }
    else
    {
        for (;CurrIndex > Index; CurrIndex+=Counter)
        {
            IndexItem = IndexItem->BckPtr;
        }
    }

    return IndexItem;
}



/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/
ListItem *LinkedList::IndexIncrement(void)
{


    CurrIndex++;
    if (CurrIndex >= ListSize)
    {
        CurrIndex =  -1;
        IndexItem = NULL;
    }
    else if (CurrIndex)
    {
        IndexItem = IndexItem->FwdPtr;
    }
    else
    {
        IndexItem = MasterLink.FwdPtr;
    }

    return IndexItem;
}



/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/
void LinkedList::PrintList(void)
{
    ListItem *Current;

    Current = MasterLink.FwdPtr;
    printf("\nMasterLink --- ");
    PrintItem(&MasterLink);
    while (Current != &MasterLink)
    {
        PrintItem(Current);
        Current = Current->FwdPtr;
    }
}



/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/
void LinkedList::PrintItem(ListItem *Item)
{
    printf("Item   %p:\n",Item);
    printf("   FwdPtr %p\n",Item->FwdPtr);
    printf("   BckPtr %p\n",Item->BckPtr);
    printf("   Name %p\n",Item->Name);
    printf("   Content %p\n",Item->Content);
    fflush(stdout);
}


/*============================vvvvvvvvv Specialty DropContent()'s vvvvvvvvv=================*/



/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/
void BSTRLinkedList::DropContent(void *Content)
{

    if (Content)
    {
        if (((PGBSTR)Content)->String)
        {
            SysFreeString(((PGBSTR)Content)->String);
        }
        delete Content;
    }
}



/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/
void VariantLList::DropContent(void *Content)
{
    if (Content)
    {
        // if(((PGVARIANT)Content)->Mutant) VariantClear(&((PGVARIANT)Content)->Mutant);
        VariantClear(&((PGVARIANT)Content)->Mutant);
        delete Content;
    }
}



/*******************************************************************************
 *
 *
 *     As small as this is, it cannot be put in the class declaration.
 *  The ListRoot member is a pointer to the LooseLinkedList class, which
 *  is forward declared, and not fully defined at the time of LLLContent
 *  class definition, therefore, LooseLinkedList::DropItem() is unknown
 *  by the compiler.  Forward defining the destructor in the class
 *  definition, and defining the content here, after both classes have
 *  been defined resolves all name conflicts.
 *
 *******************************************************************************/
LLLContent::~LLLContent()
{
    if (ListState == Exists)
    {
        ListRoot->DropItem((void *)Ourselves);
    }
}



/*******************************************************************************
 *
 *
 *
 *
 *
 *
 *******************************************************************************/
void LLLContent::SetDestructor(LooseLinkedList *Root, void *Container)
{
    ListState = Exists;
    ListRoot = Root;
    Ourselves = Container;
}



/*******************************************************************************
 *******************************************************************************
 *******************************************************************************
 *****                                                                     *****
 *****                                                                     *****
 *****                                                                     *****
 *****                                                                     *****
 *****       E n d   o f   L i n k e d L i s t ' s                         *****
 *****                                                                     *****
 *****                                                                     *****
 *****                                                                     *****
 *****                                                                     *****
 *******************************************************************************
 *******************************************************************************
 *******************************************************************************/




/****************************************************************************
 *
 *
 *
 *****************************************************************************/
STDMETHODIMP GetProperty(FILE *Stream, IDispatch *Disp, LPCOLESTR Property, LCID Lang, VARIANT *RetInfo)
{
    DISPID     DispID;

    FPRINTF2(Stream,"GetProperty() by name NOT returning DispID\n");
    return GetProperty(Stream,Disp,Property,Lang,RetInfo,&DispID);
}


/****************************************************************************
 *
 *
 *
 *****************************************************************************/
STDMETHODIMP GetProperty(FILE *Stream, IDispatch *Disp, LPCOLESTR Property, LCID Lang, VARIANT *RetInfo,DISPID *DispID)
{
    HRESULT    RetCode;




    FPRINTF2(Stream,"GetProperty() by name returning DispID\n");
    VariantInit(RetInfo);
    *DispID = (DISPID) -1;
    //  Use an array on one name to find the DispID.
    RetCode = Disp->GetIDsOfNames(IID_NULL, (OLECHAR **)&Property, 1, Lang, DispID);
    FPRINTF2(Stream,"GetProperty()  GetIDsOfNames() for \"%S\" returned: DispId %d HRESULT %08x\n", Property,*DispID,RetCode);

    if (SUCCEEDED(RetCode))
    {
        RetCode = GetProperty(Stream,Disp,Lang,RetInfo,*DispID);
    }

    return RetCode;
}


/****************************************************************************
 *
 *
 *
 *****************************************************************************/
STDMETHODIMP GetProperty(FILE *Stream, IDispatch *Disp, LCID Lang, VARIANT *RetInfo,DISPID DispID)
{
    HRESULT    RetCode;
    DISPPARAMS DispParms;
    EXCEPINFO  ExcepInfo;
    UINT       ErrCode;


    FPRINTF2(Stream,"GetProperty() by DispID\n");
    VariantInit(RetInfo);
    ErrCode = 0;
    DispParms.rgvarg = NULL;
    DispParms.rgdispidNamedArgs = NULL;
    DispParms.cArgs = 0;
    DispParms.cNamedArgs = 0;

    RetCode = Disp->Invoke(DispID, IID_NULL, Lang, DISPATCH_PROPERTYGET,
                           &DispParms, RetInfo, &ExcepInfo, &ErrCode);

    FPRINTF2(Stream,"GetProperty() -- Back from ProperyGet. HRESULT %08x\n",RetCode);
    PrntMutant(Stream,RetInfo);
    FPRINTF2(Stream,"GetProperty() -- end of parms.\n");


    // VariantClear(RetInfo);
    FPRINTF2(Stream,"GetProperty() exiting HRESULT %08x \n",RetCode);

    return RetCode;
}




/****************************************************************************
 *
 *
 *
 *****************************************************************************/
STDMETHODIMP PutProperty(FILE *Stream, IDispatch *Disp, LPCOLESTR Property, LCID Lang, VARIANT *NewValue)
{
    DISPID     DispID;

    FPRINTF2(Stream,"PutProperty() by name NOT returning DispID\n");
    return PutProperty(Stream,Disp,Property,Lang,NewValue,&DispID);
}


/****************************************************************************
 *
 *
 *
 *****************************************************************************/
STDMETHODIMP PutProperty(FILE *Stream, IDispatch *Disp, LPCOLESTR Property, LCID Lang, VARIANT *NewValue,DISPID *DispID)
{
    HRESULT    RetCode;

    FPRINTF2(Stream,"PutProperty() by name returning DispID\n");
    VariantInit(NewValue);
    *DispID = (DISPID) -1;
    //  Use an array on one name to find the DispID.
    RetCode = Disp->GetIDsOfNames(IID_NULL, (OLECHAR **)&Property, 1, Lang, DispID);
    FPRINTF2(Stream,"PutProperty()  GetIDsOfNames() for \"%S\" returned: DispId %d HRESULT %08x\n",
             Property,*DispID,RetCode);

    if (SUCCEEDED(RetCode))
    {
        RetCode = PutProperty(Stream,Disp,Lang,NewValue,*DispID);
    }

    return RetCode;
}


/****************************************************************************
 *
 *
 *
 *****************************************************************************/
STDMETHODIMP PutProperty(FILE *Stream, IDispatch *Disp, LCID Lang, VARIANT *NewValue,DISPID DispID)
{
    HRESULT    RetCode;
    DISPPARAMS DispParms;
    EXCEPINFO  ExcepInfo;
    UINT       ErrCode;

    FPRINTF2(Stream,"PutProperty() by DispID\n");
    VariantInit(NewValue);
    ErrCode = 0;
    DispParms.rgvarg = NULL;
    DispParms.rgdispidNamedArgs = NULL;
    DispParms.cArgs = 0;
    DispParms.cNamedArgs = 0;

    RetCode = Disp->Invoke(DispID, IID_NULL, Lang, DISPATCH_PROPERTYPUT,
                           &DispParms, NewValue, &ExcepInfo, &ErrCode);
    FPRINTF2(Stream,"PutProperty() -- Back from ProperyGet. HRESULT %08x\n",RetCode);
    PrntMutant(Stream,NewValue);
    FPRINTF2(Stream,"PutProperty() -- end of parms.\n");

    FPRINTF2(Stream,"PutProperty() exiting HRESULT %08x \n",RetCode);
    return RetCode;
}



/****************************************************************************
 *
 *    This routine calculates the numbers of characters required to
 *  represent 1-Number inclusive.
 *
 *    Example 1: Number = 9
 *      Required = 9   "123456789"
 *
 *    Example 2: Number = 27
 *      Required = 45   "123456789101112131415161718192021222324252627"
 *
 *    Use this when trying to compute how long a generated string will be containing
 *  strings such as "Arg(1), Arg(2)...Arg(Number)"
 *
 *****************************************************************************/
int CharsRequired(int Number)
{
    int  i,Digits, Required=0,Base=0,Multiplier=9;
    char lNumber[124];


    sprintf(lNumber,"%d",Number);
    Digits = strlen(lNumber);
    for (i=1;i < Digits; i++)
    {
        Required += Multiplier * (i);
        Base += Multiplier;
        Multiplier *= 10;
    }

    Required += (Number - Base) * Digits;
    return Required;
}


/****************************************************************************
 *
 *
 *
 *****************************************************************************/
STDMETHODIMP InvokeNamedParms(DISPPARAMS *Params, char    **CallString, int *MaxStrLen,
                              char    **NameList, int *MaxListLen)
{
    int      i,lNACount;
    HRESULT  RetCode=S_OK;
    // Commands
    char    *Command;
    char     Set[]="%s = .OLEObject~new(\"IDISPATCH=%p\");";
    // Names
    char    *Name;
    char     lTHIS[]="this";
    char     lPUT[]="\"PUT\"";
    char    *OrigStr,*OldStr,*OrigList,*OldList;
    int      BaseLen=sizeof(Set)-4;  // -4 length of %s and %p.
    int      CurrLen,Forecast,ListLen;


    if (!CallString) RetCode = E_POINTER;
    if (Params->cNamedArgs > 0 && Params->rgdispidNamedArgs == NULL)
    {
        lNACount = 0;
        RetCode = E_UNEXPECTED;
    }
    if (SUCCEEDED(RetCode))
    {
        OrigStr = *CallString;
        OrigList = *NameList;
        for (i=0; (unsigned)i < Params->cNamedArgs; i++)
        {
            CurrLen = strlen(*CallString);
            ListLen = strlen(*NameList);
            switch (Params->rgdispidNamedArgs[i])
            {
                case DISPID_THIS:
                    Name = lTHIS;
                    BaseLen = sizeof(Set)-4;  // -4 length of %s and %p.
                    Command = Set;
                    break;
                case DISPID_PROPERTYPUT:
                    Name = lPUT;
                    Command = NULL;
                    break;
                default:
                    Name = "\"Unknown Named Parm\"";
                    Command = NULL;
            }
            if (Command)
            {
                Forecast = BaseLen + strlen(Name) + 8;     // 8 possible length of pointer.
                while (Forecast+CurrLen > *MaxStrLen)
                {
                    OldStr = *CallString;
                    RetCode = NewBuffer(CallString,MaxStrLen);
                    if (FAILED(RetCode))
                    {
                        return RetCode;
                    }
                    if (OldStr != OrigStr)
                    {
                        delete OldStr;
                    }
                }
                sprintf(&(*CallString[CurrLen]),Set,Name, (Params->rgvarg[i].pdispVal));
            }
            while (strlen(Name)+ListLen > *MaxListLen)
            {
                OldList = *NameList;
                RetCode = NewBuffer(NameList,MaxListLen);
                if (FAILED(RetCode))
                {
                    return RetCode;
                }
                if (OldList != OrigList)
                {
                    delete OldList;
                }
            }
            sprintf(&(*NameList[ListLen]),"%s%s",i?",":"",Name);
        }
    }
    return RetCode;
}


/****************************************************************************
 *
 *
 *
 *****************************************************************************/
STDMETHODIMP NewBuffer(char    **OldBuffer, int *MaxBufLen)
{
    int      NewLen;
    char    *NewBuffer;


    NewLen = *MaxBufLen + 4096;
    NewBuffer = new char[NewLen];
    if (!NewBuffer)
    {
        return E_OUTOFMEMORY;
    }
    strncpy(NewBuffer,*OldBuffer,*MaxBufLen);
    *MaxBufLen = NewLen;
    *OldBuffer = NewBuffer;
    return S_OK;
}


/******************************************************************************
*                 AddMutant
*
*   Inserts an unnamed OLECHAR* argument as the first parameter into an IDispatch
*  invocation Variant list (an array of VARIANT, not a variant VT_ARRAY.
******************************************************************************/
STDMETHODIMP AddMutant(
  /*  [in]  */ OLECHAR     *FirstArg,
  /*  [in]  */ DISPPARAMS  *OrigDP,
  /*[in/out]*/ DISPPARAMS  *DP)
{
    VARIANTARG    *FCmd,*OCmd;
    int           ArgCount;


    ArgCount = OrigDP->cArgs-OrigDP->cNamedArgs;
    FCmd = new VARIANTARG[ArgCount + 1];
    if (!FCmd)
    {
        return E_OUTOFMEMORY;
    }

    OCmd = OrigDP->rgvarg;               //  In case OrigDP and DP are the same.
    DP->cNamedArgs = OrigDP->cNamedArgs;
    DP->cArgs = OrigDP->cArgs + 1;
    DP->rgvarg = FCmd;
    DP->rgdispidNamedArgs = OrigDP->rgdispidNamedArgs;


    //  This is where things get a bit messy.  Variants are passed in reverse
    // order.  Therefore, the parm that we generated must be put on at the end.
    // First, copy the args that the Host gave us.
    if (ArgCount > 0)
    {
        memcpy(&FCmd[0],&(OCmd[0]),sizeof(VARIANTARG)*ArgCount);
    }
    //  Then tack on the OLECHAR* at the end.
    VariantInit(&FCmd[ArgCount]);
    V_VT(&FCmd[ArgCount]) = VT_BSTR;
    V_BSTR(&FCmd[ArgCount]) = SysAllocString(FirstArg);

    return S_OK;
}





/******************************************************************************
*                 DropNamedPut
*
*
*
******************************************************************************/
STDMETHODIMP DropNamedPut(
  /*  [in]  */ DISPPARAMS  *OrigDP,
  /*[in/out]*/ DISPPARAMS  *DP)
{
    DISPID       *FDispID;
    unsigned int  ArgCount,i;


    ArgCount = OrigDP->cNamedArgs;

    DP->cNamedArgs = OrigDP->cNamedArgs;
    DP->cArgs = OrigDP->cArgs;
    DP->rgvarg = OrigDP->rgvarg;
    DP->rgdispidNamedArgs = OrigDP->rgdispidNamedArgs;

    for (i=0; i<OrigDP->cNamedArgs; ++i)
    {
        if (OrigDP->rgdispidNamedArgs[i] == DISPID_PROPERTYPUT)
        {
            break;
        }
    }

    if (i == OrigDP->cNamedArgs)
    {
        return S_OK;
    }

    --ArgCount;
    DP->cNamedArgs = ArgCount;
    --DP->cArgs;
    if (!ArgCount)
    {
        DP->rgdispidNamedArgs = NULL;
        return S_OK;
    }
    FDispID = new DISPID[ArgCount];
    if (!FDispID)
    {
        return E_OUTOFMEMORY;
    }
    if (i > 0)
    {
        memcpy(&FDispID[0],&(OrigDP->rgdispidNamedArgs[0]),sizeof(DISPID)*i);
    }
    if (i < ArgCount)
    {
        memcpy(&FDispID[i],&(OrigDP->rgdispidNamedArgs[i+1]),sizeof(DISPID)*(ArgCount-i));
    }
    DP->rgdispidNamedArgs = FDispID;

    return S_OK;
}
