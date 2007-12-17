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
/* REXX Kernel                                             MethodClass.hpp    */
/*                                                                            */
/* Primitive Kernel Method Class Definitions                                  */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxMethod
#define Included_RexxMethod

#include "RexxCode.hpp"
#include "RexxNativeCode.hpp"

class RexxMethodClass;

 class RexxMethod : public RexxObject {
  public:
  void *operator new(size_t);
  inline void *operator new(size_t size, void *ptr) { return ptr; };
  RexxMethod(size_t, PCPPM, size_t, RexxInternalObject *);
  inline RexxMethod(RESTORETYPE restoreType) { ; };
  void execute(RexxObject *, RexxObject *);
  void live();
  void liveGeneral();
  void flatten(RexxEnvelope*);
  RexxObject   *unflatten(RexxEnvelope *envelope);
  RexxObject   *run(RexxActivity *,  RexxObject *, RexxString *,  size_t, RexxObject **);
  RexxObject   *call(RexxActivity *,  RexxObject *,  RexxString *,  RexxObject **, size_t, RexxString *, RexxString *, int);
  RexxMethod  *newScope(RexxClass  *);
  RexxArray   *source();
  void              setAttribute(RexxVariableBase *variable) {OrefSet(this, this->attribute, variable); }
  RexxVariableBase *getAttribute() { return this->attribute; };
  void         setScope(RexxClass  *);
  RexxSmartBuffer  *saveMethod();
  RexxObject  *setUnGuardedRexx();
  RexxObject  *setGuardedRexx();
  RexxObject  *setPrivateRexx();
  RexxObject  *setProtectedRexx();
  RexxObject  *setSecurityManager(RexxObject *);

  RexxObject  *isGuardedRexx();
  RexxObject  *isPrivateRexx();
  RexxObject  *isProtectedRexx();

   inline size_t getMethodIndex() {return this->methodIndex; };
   inline size_t getArgumentCount() {return this->argumentCount; };
   inline void   setMethodIndex(size_t num) { methodIndex = num; };
   inline void   setArgumentCount(size_t args) { this->argumentCount = args; };

   inline bool   isGuarded()      {return (this->methodFlags & UNGUARDED_FLAG) == 0; };
   inline bool   isInternal()     {return (this->methodFlags & INTERNAL_FLAG) != 0; };
   inline bool   isPrivate()      {return (this->methodFlags & PRIVATE_FLAG) != 0;}
   inline bool   isProtected()    {return (this->methodFlags & PROTECTED_FLAG) != 0;}
   inline bool   isSpecial()      {return (this->methodFlags & (PROTECTED_FLAG | PRIVATE_FLAG)) != 0;}

   inline bool   isRexxMethod()   {return (this->methodFlags & REXX_METHOD) != 0; };
   inline bool   isNativeMethod() {return (this->methodFlags & NATIVE_METHOD) != 0; };
   inline bool   isCPPMethod()    {return (this->methodFlags & KERNEL_CPP_METHOD) != 0; };

   inline void   setUnGuarded()    {this->methodFlags |= UNGUARDED_FLAG;};
   inline void   setGuarded()      {this->methodFlags &= ~UNGUARDED_FLAG;};
   inline void   setInternal()     {this->methodFlags |= INTERNAL_FLAG;};
   inline void   setPrivate()      {this->methodFlags |= (PRIVATE_FLAG | PROTECTED_FLAG);};
   inline void   setProtected()    {this->methodFlags |= PROTECTED_FLAG;};
   inline void   setRexxMethod()   {this->methodFlags |= REXX_METHOD; };
   inline void   setNativeMethod() {this->methodFlags |= NATIVE_METHOD; };
   inline void   setCPPMethod()    {this->methodFlags |= KERNEL_CPP_METHOD; };

   RexxInteger *Private() { return  (this->isPrivate() ? TheTrueObject : TheFalseObject); };
   inline RexxObject *getCode() {return (RexxObject *)this->code;}
   inline RexxCode   *getRexxCode() {return this->rexxCode;}
   inline RexxNativeCode *getNativeCode() {return this->nativeCode;}
   inline RexxSource *getSource() {return this->rexxCode->getSource();}
   inline RexxClass  *getScope() {return this->scope;}
   inline void setLocalRoutines(RexxDirectory *r) { getSource()->setLocalRoutines(r); }
   inline void setPublicRoutines(RexxDirectory *r) { getSource()->setPublicRoutines(r); }
   inline RexxDirectory *getLocalRoutines() { return getSource()->getLocalRoutines(); }
   inline RexxDirectory *getPublicRoutines() { return getSource()->getPublicRoutines(); }

   static RexxMethodClass *classInstance;

 protected:
   enum
   {
       PRIVATE_FLAG      = 0x01,        // private method
       UNGUARDED_FLAG    = 0x02,        // Method can run with GUARD OFF
       INTERNAL_FLAG     = 0x04,        // method is part of saved image
       REXX_METHOD       = 0x08,        // method is a REXX method
       NATIVE_METHOD     = 0x10,        // method is a native C method
       PROTECTED_FLAG    = 0x40,        // method is protected
       KERNEL_CPP_METHOD = 0x80,        // method is a kernel C++ meth
   };



   // TODO:  the method number and arguments should be moved into a native code object rather than
   // being in the method object.
   uint16_t   methodIndex;           // kernel method number
   uint8_t    argumentCount;         // argument count
   uint8_t    methodFlags;           // method status flags

   RexxClass  *scope;                  /* pointer to the method scope       */
   RexxVariableBase *attribute;      /* method attribute info             */
   union {
      RexxInternalObject *code;        /* associated "code" object          */
      RexxNativeCode     *nativeCode;  /* associated "code" object          */
      RexxCode           *rexxCode;    /* associated "code" object          */
   };
   PCPPM  cppEntry;                    /* C++ Method entry point.           */
 };

class RexxMethodClass : public RexxClass {
 public:
  RexxMethod  *newRexxMethod(RexxSource *, RexxClass  *);
  RexxMethod  *newRexxCode(RexxString *, RexxObject *, RexxObject *, RexxObject *a = OREF_NULL);
  RexxMethod  *newRexx(RexxObject **, size_t);
  RexxMethod  *newRexxBuffer(RexxString *, RexxBuffer *, RexxClass  *);
  RexxMethod  *newEntry(PNMF);
  RexxMethod  *restore(RexxBuffer *, char *);
  RexxMethod  *newFile(RexxString *);
  RexxMethod  *newFileRexx(RexxString *);
};

#endif
