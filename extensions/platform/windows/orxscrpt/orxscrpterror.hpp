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
#ifndef ORXSCRPTERROR
#define ORXSCRPTERROR

#include "orxscrpt_main.hpp"
#include "eng2rexx.hpp"


/*
    This object interacts with its creator outside of the COM specification.  To handle
  this complication a pointer to a bool flag is passed on the constructor.  If the
  pointer is defined, this object interacts with its creator by writing to the FILE*
  passed as the first parameter of the constructor, and by setting the bool pointed to
  by the third parameter to true in the constructor, and false in the destructor.
  The bool is a signal to the creator about the existence of this object.  It is
  important to the creator in making the decision to Release(), or not.
    Not all of the users of this object act in the typical COM tradition.  The typical
  COM usage assumes that when a pointer is given to a caller, that the giver performs
  an AddRef(), and the receiver performs a Release() when done.  When the refcount
  goes to 0, the object is destroyed.  It has been noted in some cases that upon
  receiving a pointer to this object, the receiver did not all assume a refcount had
  been added, and the receiver performed an AddRef().  Thus when they were done, the
  Release() brought the count back to the point where the receiver obtained a pointer
  to the object, and the object did not go away.  In these cases, the creator must
  perform the additional Release().
    The bool flag that resides in the creators address space, informs the creator if
  the object still exists.  There is a special Release(), UDRelease(), that tells the
  object to cease the extra interactions.  The creator is gone.
    The creator calls UDRelease() if it is done with the object, and the object still
  exists. Example:

  HRESULT     hResult=S_OK;
  OrxScriptError *ErrObj;
  bool        ErrObj_Exists;

      ErrObj = new OrxScriptError(logfile,condData,&ErrObj_Exists);
      hResult = pEngine->getScriptSitePtr()->OnScriptError((IActiveScriptError*) ErrObj);
      if (hResult == E_UNEXPECTED) {
      }
      // init to empty again....
      if(ErrObj_Exists) ErrObj->UDRelease();


  NB:  There is a potential problem.  An assumption is being made based on passed
  behavior.  If this changes, so that the receiver acts as other COM objects, the
  code through the flag will adjust for this, unless the receiver expects the life
  of the object to persist passed what we assume it should be.
*/

// class definition

class OrxScriptError : public IActiveScriptError {

  public:
    OrxScriptError(FILE *Stream, RexxConditionData *info, bool *Exists);    // CTOR
    ~OrxScriptError();                // DTOR


    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);
    STDMETHOD_(ULONG,UDRelease)(void);         // UnDocumented Release(), sets ExternalWorld to false.


    /***** IActiveScriptError Methods *****/
    STDMETHOD(GetExceptionInfo)(EXCEPINFO *pInfo);

    STDMETHOD(GetSourcePosition)(DWORD *pdwSourceContext, ULONG *LineNumber, LONG *CharPos);

    STDMETHOD(GetSourceLineText)(BSTR *Text);

  private:

    FILE          *logfile;
    ULONG          ulRefCount;                 // reference count
    RexxConditionData  RexxErrorInfo;          // error info if a script returns with an error
    bool          *RunDestructor;              // True, destroy the object: false, we are not here any more.

    };



#endif     // ifndef ORXSCRPTERROR
