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
#ifndef EVENTS_H
#define EVENTS_H

/******************************************************************************/
/* Header file for event handling of OLE objects                              */
/******************************************************************************/



//******************************************************************************
// type definitions
//******************************************************************************
typedef struct _OLEFUNCINFO
{
  PSZ                   pszFuncName;
  MEMBERID              memId;
  INVOKEKIND            invkind;
  VARTYPE               FuncVt;
  int                   iParmCount;
  int                   iOptParms;
  VARTYPE               *pOptVt;
  PUSHORT               pusOptFlags;
  struct _OLEFUNCINFO * pNext;
} OLEFUNCINFO, * POLEFUNCINFO, * * PPOLEFUNCINFO;

// extended structure for events
typedef struct _OLEFUNCINFO2
{
  PSZ                   pszFuncName;
  PSZ                   pszDocString;
  MEMBERID              memId;
  INVOKEKIND            invkind;
  VARTYPE               FuncVt;
  int                   iParmCount;
  int                   iOptParms;
  VARTYPE               *pOptVt;
  PUSHORT               pusOptFlags;
  char                **pszName;
  struct _OLEFUNCINFO2 * pNext;
} OLEFUNCINFO2, * POLEFUNCINFO2, * * PPOLEFUNCINFO2;

typedef struct _OLECONSTINFO
{
  PSZ                   pszConstName;
  MEMBERID              memId;
  VARIANT               sValue;
  struct _OLECONSTINFO *pNext;
} OLECONSTINFO, * POLECONSTINFO, * * PPOLECONSTINFO;

typedef struct _OLECLASSINFO
{
  PSZ           pszProgId;
  PSZ           pszCLSId;

  ITypeInfo     *pTypeInfo;

  POLEFUNCINFO  pFuncInfo;
  POLECONSTINFO pConstInfo;

  int           iInstances;
  BOOL          fUsed;
} OLECLASSINFO, * POLECLASSINFO, * * PPOLECLASSINFO;

typedef struct _TYPELIBLIST
{
  GUID          guid;
  POLECONSTINFO info;
  struct _TYPELIBLIST *next;
} TYPELIBLIST, * PTYPELIBLIST;

/* event handler class */

class OLEObjectEvent : public IDispatch {
  public:
    OLEObjectEvent(POLEFUNCINFO2, RexxObjectPtr, RexxInstance *, GUID);
    virtual ~OLEObjectEvent();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID, void**);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    /* IDispatch methods */
    STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR *, unsigned int, LCID, DISPID FAR*);
    STDMETHOD(GetTypeInfo)(unsigned int, LCID, ITypeInfo **);
    STDMETHOD(GetTypeInfoCount)(unsigned int *);
    STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, unsigned int *);

    inline POLEFUNCINFO2 getEventList() { return pEventList; }  // return event list
    inline GUID getIntefaceID() { return interfaceID; }

  private:
    ULONG         ulRefCounter;         // reference counter
    POLEFUNCINFO2 pEventList;           // event list
    RexxObjectPtr    self;              // associated REXX OLEObject
    RexxInstance  *interpreter;         // the Rexx interpreter instance we're running under.
    GUID          interfaceID;          // event interface this object supports
};

#endif EVENTS_H
