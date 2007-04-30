/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX Kernel                                                  RexxSOMCode.hpp   */
/*                                                                            */
/* Primitive SOM Method Class Definitions                                     */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxSOMMethod
#define Included_RexxSOMMethod

#define GENERIC_FLAG      0x00000001   /* Generic method flag               */
#define SOMRESOLVED_FLAG  0x00000002   /* SOM method info is resolved and   */
                                       /*  accessable.                      */
#ifdef SOM
  #include "orxsminf.xh"
  #include "orxsargl.xh"
#endif

class RexxSOMCode : public RexxInternalObject {
 public:
  void *operator new(size_t);
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  RexxSOMCode(BOOL generic = TRUE);
  inline RexxSOMCode(RESTORETYPE restoreType) { ; };
  void init(BOOL);
  void uninit();
  RexxObject * run(RexxObject *, RexxString *, RexxClass *, int, RexxObject **);
  void resolve(RexxObject *, RexxString *, RexxClass *);
  inline BOOL isResolved()  { return (this->SOMflags & SOMRESOLVED_FLAG);};
  inline BOOL isGeneric()   { return (this->SOMflags & GENERIC_FLAG);};
#ifdef SOM
  void setResolvedInfo(OrxSOMMethodInformation *,
                       SOMClass *,
                       somId *);
#endif

  long SOMflags;                       /* flags for SOM method            */
#ifdef SOM
  OrxSOMMethodInformation *methodInfo;
  SOMClass                *methodClass;/* SOM class method introduced at    */
  OrxSOMArgumentList      *argList;    /* argument list for message send    */
  somId                   msgId;       /* Messgae Id for this method.     */
#endif
};
#endif
