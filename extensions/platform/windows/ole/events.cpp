/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2026 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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
/***************************************************************************/
/*                                                                         */
/*  COM Object to handle events for an OLEObject                           */
/*                                                                         */
/*  This calls methods of the OLEObject                                    */
/*                                                                         */
/***************************************************************************/
/****************************************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <dispex.h>
// no more required for MSVC 2015/2017
// #include <agtctl_i.c> /* include to get the ID of ControlAgent events */

#include "oorexxapi.h"
#include "events.h"

extern VOID ORexxOleFree(PVOID ptr);
extern RexxObjectPtr Variant2Rexx(RexxThreadContext *, VARIANT *pVariant);
extern bool Rexx2Variant(RexxThreadContext *, RexxObjectPtr RxObject, VARIANT *pVariant, VARTYPE DestVt, size_t iArgPos);

// CTOR
// set reference count to one
OLEObjectEvent::OLEObjectEvent( POLEFUNCINFO2 pEL, RexxObjectPtr slf, RexxInstance *c, GUID guid) :
  ulRefCounter(1), pEventList(pEL), self(slf), interpreter(c), interfaceID(guid)
{
  //fprintf(stderr,"OLEObjectEvent::OLEObjectEvent() ");
}

// DTOR
OLEObjectEvent::~OLEObjectEvent()
{
    POLEFUNCINFO2 pTemp;
    int i;

//  fprintf(stderr,"OLEObjectEvent::~OLEObjectEvent() (%p)\n",this);

    // remove event list from memory
    while (pEventList)
    {
        pTemp = pEventList;
        pEventList = pEventList->pNext;
        //fprintf(stderr,"releasing event map entry: %s (%s)\n",pTemp->pszFuncName,pTemp->pszDocString);
        ORexxOleFree( pTemp->pszFuncName );
        ORexxOleFree( pTemp->pszDocString );
        ORexxOleFree( pTemp->pOptVt );
        ORexxOleFree( pTemp->pusOptFlags );
        for (i=0;i<pTemp->iParmCount;i++)
        {
            ORexxOleFree (pTemp->pszName[i]);
        }
        ORexxOleFree( pTemp->pszName );
        ORexxOleFree( pTemp );
    }

}

/* Implementation of IUnknown interface */

STDMETHODIMP OLEObjectEvent::QueryInterface(
    /* [in]  */ REFIID riid,
    /* [out] */ void **ppvObj)
{
    if (!ppvObj)
        return ResultFromScode(E_INVALIDARG);
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID) (IUnknown *) this;
    else if (IsEqualIID(riid, IID_IDispatch))
        *ppvObj = (LPVOID) (IDispatch *) this;
    else if ( IsEqualIID( riid, interfaceID) )
        *ppvObj = (LPVOID) (IDispatch *) this;


    if (*ppvObj != NULL)
    {
        /* we found an interface that can be returned, add reference */
        AddRef();
        return NOERROR;
    }
    else
    {
        return ResultFromScode(E_NOINTERFACE);
    }
}


STDMETHODIMP_(ULONG) OLEObjectEvent::AddRef(void)
{
  return ++ulRefCounter;
}


STDMETHODIMP_(ULONG) OLEObjectEvent::Release(void)
{
  /* if more than one reference return new count */
  if (--ulRefCounter)
    return ulRefCounter;

  /* last reference was released, free object */
  delete this;
  return 0;
}

/* Implementation of IDispatch interface */

STDMETHODIMP OLEObjectEvent::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
                                      unsigned int cNames, LCID lcid,
                                      DISPID *rgDispId)
{
  //fprintf(stderr,"OLEObjectEvent::GetIDsOfNames\n");
  return ResultFromScode(E_OUTOFMEMORY);
}


STDMETHODIMP OLEObjectEvent::GetTypeInfo(unsigned int iTinfo, LCID lcid,
                                    ITypeInfo **ppTInfo)
{
  //fprintf(stderr,"OLEObjectEvent::GetTypeInfo\n");
  return ResultFromScode(DISP_E_BADINDEX);
}


STDMETHODIMP OLEObjectEvent::GetTypeInfoCount(unsigned int *pctinfo)
{
  //fprintf(stderr,"OLEObjectEvent::GetTypeInfoCount\n");
  return ResultFromScode(E_NOTIMPL);
}


/**
 * Stores a Rexx value into the storage that a VT_BYREF argument points to
 * (an event's out parameter), converted to that storage's type. The
 * argument's own VARIANT is left as it is: it belongs to the event's source,
 * which reads the storage through it once Invoke returns.
 *
 * @return true if the value was stored.
 */
static bool storeByRef(RexxThreadContext *context, RexxObjectPtr value, VARIANT *pArg)
{
    VARTYPE vt = V_VT(pArg);
    if ( !(vt & VT_BYREF) || V_BYREF(pArg) == NULL )
    {
        return false;
    }
    VARTYPE baseVt = vt & ~VT_BYREF;

    VARIANT temp;
    VariantInit(&temp);
    if ( !Rexx2Variant(context, value, &temp, baseVt == VT_VARIANT ? VT_EMPTY : baseVt, -1) )
    {
        VariantClear(&temp);
        context->ClearCondition();      // the value could not be converted: leave the parameter
        return false;
    }
    if ( baseVt != VT_VARIANT && V_VT(&temp) != baseVt &&
         VariantChangeType(&temp, &temp, 0, baseVt) != S_OK )
    {
        VariantClear(&temp);
        return false;
    }

    void *p = V_BYREF(pArg);
    switch ( baseVt )
    {
        case VT_VARIANT:                // the new VARIANT replaces the old one
            VariantClear((VARIANT *)p);
            *(VARIANT *)p = temp;
            return true;
        case VT_BSTR:                   // the new string replaces the old one
            SysFreeString(*(BSTR *)p);
            *(BSTR *)p = V_BSTR(&temp);
            return true;
        case VT_DISPATCH:
        case VT_UNKNOWN:                // temp's reference goes to the parameter
            if ( *(IUnknown **)p != NULL )
            {
                (*(IUnknown **)p)->Release();
            }
            *(IUnknown **)p = V_UNKNOWN(&temp);
            return true;
        case VT_I1:
        case VT_UI1:
            *(BYTE *)p = V_UI1(&temp);
            break;
        case VT_I2:
        case VT_UI2:
        case VT_BOOL:
            *(SHORT *)p = V_I2(&temp);
            break;
        case VT_I4:
        case VT_UI4:
        case VT_INT:
        case VT_UINT:
        case VT_ERROR:
        case VT_R4:
            *(LONG *)p = V_I4(&temp);
            break;
        case VT_I8:
        case VT_UI8:
        case VT_R8:
        case VT_DATE:
        case VT_CY:
            *(LONGLONG *)p = V_I8(&temp);
            break;
        case VT_DECIMAL:
            *(DECIMAL *)p = V_DECIMAL(&temp);
            ((DECIMAL *)p)->wReserved = 0;      // (temp's vt, in a VARIANT)
            break;
        default:
            VariantClear(&temp);
            return false;
    }
    VariantClear(&temp);
    return true;
}


STDMETHODIMP OLEObjectEvent::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
                               WORD wFlags, DISPPARAMS *pDispParams,
                               VARIANT *pVarResult, EXCEPINFO *pExcepInfo,
                               unsigned int *puArgErr)
{
    HRESULT      hResult = S_OK; //DISP_E_MEMBERNOTFOUND;   // default return value
    POLEFUNCINFO2 pList = pEventList;
    BOOL         fFound = false;
    RexxThreadContext *context;

    if (!interpreter->AttachThread(&context))
    {
        return DISP_E_MEMBERNOTFOUND;
    }

//  fprintf(stderr,"OLEObjectEvent::Invoke %08x PID %08x TID %08x\n",dispIdMember,GetCurrentProcessId(),GetCurrentThreadId());

    // is this a method ?
    if (wFlags & DISPATCH_METHOD)
    {
        while (!fFound && pList)
        {
            if (pList->memId == dispIdMember) fFound = true;
            else pList = pList->pNext;
        }

        if (fFound)
        {
            RexxObjectPtr  rxResult;
            RexxArrayObject rxArray;
            int         cArgs = (int)pDispParams->cArgs;
            int         cNamed = (int)pDispParams->cNamedArgs;
            int         i;
            int         j;
            int         pos;
            //fprintf(stderr,"%08x %s\n",dispIdMember,pList->pszFuncName);

            // does argument count match? does object have method?
            if ( (pList->iParmCount == cArgs) && context->HasMethod(self, pList->pszFuncName))
            {
                char upperBuffer[128];

                // The VARIANT of each parameter, in the event's order. COM
                // gives the named arguments first (rgdispidNamedArgs: their
                // positions), then the positional ones, last first.
                VARIANT **ppArg = (VARIANT **)calloc(cArgs > 0 ? cArgs : 1, sizeof(VARIANT *));
                if (ppArg == NULL)
                {
                    context->DetachThread();
                    return E_OUTOFMEMORY;
                }
                for ( j = 0; j < cNamed && j < cArgs; j++ )
                {
                    pos = (int)pDispParams->rgdispidNamedArgs[j];
                    if ( pos >= 0 && pos < cArgs && ppArg[pos] == NULL )
                    {
                        ppArg[pos] = &pDispParams->rgvarg[j];
                    }
                }
                for ( j = cNamed, pos = 0; j < cArgs; j++ )
                {
                    while ( pos < cArgs && ppArg[pos] != NULL )
                    {
                        pos++;
                    }
                    if ( pos == cArgs )
                    {
                        break;
                    }
                    ppArg[pos++] = &pDispParams->rgvarg[cArgs - 1 - (j - cNamed)];
                }

                rxArray = context->NewArray(cArgs);
                // convert VARIANTs to Rexx Objects...
                for ( i = 0; i < cArgs; i++ )
                {
                    if ( ppArg[i] == NULL )     // (a named argument out of range)
                    {
                        continue;
                    }
                    rxResult = Variant2Rexx(context, ppArg[i]);

                    if ( rxResult == NULLOBJECT )
                    {
                        // Variant2Rexx() raised an exception, we could not
                        // convert this arg.  Be sure to detach and then
                        // indicate the error to whoever invoked us.
                        if ( puArgErr != NULL )
                        {
                            *puArgErr = (unsigned int)(ppArg[i] - pDispParams->rgvarg);
                        }
                        free(ppArg);
                        context->DetachThread();
                        return DISP_E_TYPEMISMATCH;
                    }

                    context->ArrayPut(rxArray, rxResult, i + 1);
                }
                strcpy(upperBuffer,pList->pszFuncName);
                strupr(upperBuffer);
                rxResult = context->SendMessage(self, upperBuffer, rxArray);

                // if method returned something, see if out paramters can be filled in
                // otherwise ignore them
                if ( !(rxResult == NULL) && !(rxResult == context->Nil()) )
                {
                    // the out parameters, in the event's order
                    for (i=j=0;i<cArgs;i++)
                    {
                        if ((pList->pusOptFlags[i] & PARAMFLAG_FOUT) && ppArg[i] != NULL) j++;
                    }

                    // if there is just a single out parameter, the return
                    // value goes into it directly
                    if (j==1)
                    {
                        for (i=0; i<cArgs; i++)
                        {
                            if ((pList->pusOptFlags[i] & PARAMFLAG_FOUT) && ppArg[i] != NULL)
                            {
                                storeByRef(context, rxResult, ppArg[i]);
                                break;
                            }
                        }
                    }
                    // several: an Array gives their values, in order
                    else if (j>1 && context->IsArray(rxResult))
                    {
                        RexxArrayObject rxValues = (RexxArrayObject)rxResult;
                        wholenumber_t k;

                        context->ObjectToWholeNumber(context->SendMessage0(rxValues,"DIMENSION"), &k);
                        if (k == 1)
                        {
                            k = (wholenumber_t)context->ArraySize(rxValues);
                            for (i=0, pos=1; i<cArgs && pos<=k; i++)
                            {
                                if ((pList->pusOptFlags[i] & PARAMFLAG_FOUT) && ppArg[i] != NULL)
                                {
                                    RexxObjectPtr item = context->ArrayAt(rxValues, pos++);
                                    if (item != NULLOBJECT && item != context->Nil())   // .nil: leave it
                                    {
                                        storeByRef(context, item, ppArg[i]);
                                    }
                                }
                            }
                        }
                    }
                }
                free(ppArg);
                hResult = S_OK;
            }
            else
            {
                hResult = DISP_E_MEMBERNOTFOUND;
            }
        }
    }

    // make sure we detach this thread, which will clean up our stack frame
    // entries.
    context->DetachThread();

    return hResult;
}
