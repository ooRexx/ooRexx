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
#ifndef ORXDISPID_HPP
#define ORXDISPID_HPP

#include "WObaseproto.h"
#include "orxscrpt_main.hpp"
#include "rexx.h"

/*
 *    The usual Rexx Convention is to use base type char for all data
 *  and parameters.  These functions depart from that norm.  They are
 *  called by the routines that directly interface with the Windows
 *  system, and by leaving the data in OLECHAR a lot of conversion
 *  overhead is avoided.
 *    Also be aware, this is the "Jim" naming convention. p??????
 *  means that ?????? is a parameter, and not that it is necessarily
 *  a pointer.  The prefix "pb" denoted a parameter that is "passed
 *  back" (or returned), and "l" is for local storage (stack) data.
 *
 *
 *
 *
 */

/*
 *    The following struct is the "content" of the DispID linked list.
 *  It is externalized, instead of the usual object practice of hiding
 *  properties, to facilitate OrxIDispatch performing the necessary
 *  functions.
 */
typedef struct DISPID_STRUCT {
  enum    DType{ASEvent,LPPEvent,PPEvent,Function,Property};  //  All Known executables currently handled.
  OLECHAR       *Name;                      //  Name the host knows us by.
  DWORD          Flags;                     //  Copy of the flags the host passed to us.
  DType          Type;                      //  The type of executable this describes.
  void          *RexxCode;                  //  Rexx code that is to be executed,
                                            //  ... OR pointer to a Property VARIANT.
  } DID, *PDID;


class OrxDispID {
public:
  OrxDispID() : Count(0) {;}
  ~OrxDispID();


  /******************************************************************************
  *
  *    Creates the DID, and a copy of the name (so that the callers may
  *  be transient without any bad repercussions), copies all of that parms
  *  to the DID and returns.
  *    The only possible problem that can be encountered is running out of memory.
  *  In that case all memory obtained for this call (if any) is released,
  *  and E_OUTOFMEMORY is returned.
  *
  ******************************************************************************/
  STDMETHODIMP AddDispID(OLECHAR *pName, DWORD Flags, DID::DType Type, void *RexxCode, DISPID *pbDispID);

  /******************************************************************************
  *
  *    Intended for use by GetIDsOfNames(), or GetDispID().  The DispID chain is
  *  searched for a given name.  If found, the DispID is returned.  Otherwise,
  *  DISP_E_UNKNOWNNAME is returned.
  *
  ******************************************************************************/
  STDMETHODIMP FindDispID(OLECHAR *pName, DISPID *pbDispID);

  /******************************************************************************
  *
  *    If a DispID is known, then the corresponding name is returned.  Otherwise,
  *  DISP_E_UNKNOWNNAME is returned.
  *    (Used by debug prints and IDispatchEx methods.)
  *
  ******************************************************************************/
  STDMETHODIMP FindName(OLECHAR **pbName, DISPID pDispID);

  /******************************************************************************
  *
  *    Used by Invoke[Ex]() to validate a DispID, and obtain all known information
  *  on that ID so that it may be executed.
  *
  ******************************************************************************/
  STDMETHODIMP FindDID(DISPID pDispID, PDID *pbDispIDData);

  /******************************************************************************
  *
  *    Currently, the pFlags is ignored.  If pDispid is -1, then the first
  *  DispID, if there is one, is returned.  Otherwise, pDispID is searched for
  *  in the current list, and the next DispID is returned.  If neither the
  *  pDispID, or the next exists then -1 is returned.
  *
  *
  *
  ******************************************************************************/
  STDMETHODIMP GetNextDispID(
    /* [in] */ DWORD pFlags,               // Derived from fdexEnum... defines.
    /* [in] */ DISPID pDispID,             // Previous DispID returned.
    /* [out] */ DISPID __RPC_FAR *pbDispID); // Next DispID, or -1.

private:
  int         Count;    // Remembers the last DispID that was dispensed.
  //  A plain LinkedList is used.  Global memory is used to allocate the content.
  LinkedList  Chain;
  };


#endif     //    ifndef OXRDISPID_HPP
