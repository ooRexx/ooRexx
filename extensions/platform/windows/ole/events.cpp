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
#include <agtctl_i.c> /* include to get the ID of ControlAgent events */

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
            int         i;
            int         j;
            //fprintf(stderr,"%08x %s\n",dispIdMember,pList->pszFuncName);

            // does argument count match? does object have method?
            if ( (pList->iParmCount == (int) pDispParams->cArgs) && context->HasMethod(self, pList->pszFuncName))
            {
                char upperBuffer[128];

                rxArray = context->NewArray(pDispParams->cArgs);
                // convert VARIANTs to Rexx Objects...
                for ( i = 0; i < (int)pDispParams->cArgs; i++ )
                {
                    rxResult = Variant2Rexx(context, &pDispParams->rgvarg[i]);

                    if ( rxResult == NULLOBJECT )
                    {
                        // Variant2Rexx() raised an exception, we could not
                        // convert this arg.  Be sure to detach and then
                        // indicate the error to whoever invoked us.
                        context->DetachThread();
                        *puArgErr = i;
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
                    for (i=j=0;i<(int) pDispParams->cArgs;i++)
                    {
                        if (pList->pusOptFlags[i] & PARAMFLAG_FOUT) j++;
                    }

                        // out parameters exist
                    if (j>0)
                    {
                        // if there is just a single out parameter, the return
                        // value will go into it directly
                        if (j==1)
                        {
                            i=0;
                            while ( (i<(int) pDispParams->cArgs) && !(pList->pusOptFlags[i] & PARAMFLAG_FOUT))
                            {
                                i++;
                            }
                            // put value into out parameter TODO what if there is an exception?
                            Rexx2Variant(context, rxResult,&pDispParams->rgvarg[i],pDispParams->rgvarg[i].vt,-1);
                        }
                        else if (context->IsArray(rxResult))
                        {
                            RexxArrayObject rxArray = (RexxArrayObject)rxResult;
                            wholenumber_t k;

                            context->ObjectToWholeNumber(context->SendMessage0(rxArray,"DIMENSION"), &k);
                            if (k == 1)
                            {
                                k = (int)context->ArraySize(rxArray);
                                for (i=1,j=0; i <= k && j < (int)pDispParams->cArgs; j++)
                                {
                                    if (pList->pusOptFlags[j] & PARAMFLAG_FOUT)
                                    {
                                        // TODO what if there is an exception?
                                        Rexx2Variant(context, context->ArrayAt(rxArray, i),&pDispParams->rgvarg[j],pDispParams->rgvarg[j].vt,-1);
                                        i++;
                                    }
                                }
                            }
                        }
                    }
                }
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
