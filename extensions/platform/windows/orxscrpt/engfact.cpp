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
#define INITGUID        /*  The invocation of <initguid.h> have been removed.  That
                           header always defined the macro DEFINE_GUID() to generate
                           an initial value, so if the macro was in a header that
                           was included by all of the programs, then the linker
                           freaked out due to redefinitions.  Using the default
                           macro that comes with <basetyps.h> only a reference to
                           the GUID will be generated, no initial value.
                           This will cause the DEFINE_GUID() macro to generate the text
                           of the macro instead of a reference.
                        */
#include "engfact.hpp"
#include <stdio.h>


STDMETHODIMP OrxEngineClassFactory::CreateInstance(IUnknown *punkOuter,
                                                   REFIID riid,
                                                   LPVOID *ppvObj)
{
    HRESULT      hr;
    OrxScript   *pmyobject;
    ListItem    *Next;


    if (ppvObj == NULL)
    {
        return ResultFromScode(E_POINTER);
    }

    *ppvObj = NULL;

    if (punkOuter != NULL)
    {
        return ResultFromScode(E_INVALIDARG);    // Aggregation not supported
    }

    pmyobject = new OrxScript;                 // create instance of engine
                                               // and its associated COM Dispatcher.

    if (pmyobject == NULL)
    {
        if (pmyobject != NULL)
        {
            delete pmyobject;
        }
        return E_OUTOFMEMORY;
    }

    // add this to the global engine chain
    ScriptProcessEngine::addScriptEngine(pmyojbect);

    hr = pmyobject->QueryInterface(riid, ppvObj);
    pmyobject->Release();   // ppvObj now is only reference
                            // (if QI failed, object is destroyed)
    return hr;
}


REFIID OrxEngineClassFactory::GetClassID()
{
    return CLSID_ObjectREXX;
}


OrxClassFactory *CreateClassFactory()
{
    return new OrxEngineClassFactory();
}
