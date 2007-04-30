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
/* REXX Kernel                                                MethodClass.hpp    */
/*                                                                            */
/* Primitive Kernel Method Class Definitions                                  */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxMethod
#define Included_RexxMethod

#define PRIVATE_FLAG      0x01         /* private method                    */
#define UNGUARDED_FLAG    0x02         /* Method can run with GUARD OFF     */
#define INTERNAL_FLAG     0x04         /* method is part of saved image     */
#define REXX_METHOD       0x08         /* method is a REXX method           */
#define NATIVE_METHOD     0x10         /* method is a native C method       */
#define SOM_METHOD        0x20         /* method is a SOM method            */
#define PROTECTED_FLAG    0x40         /* method is protected               */
#define KERNEL_CPP_METHOD 0x80         /* method is a kernel C++ meth       */

 class RexxMethod : public RexxObject {
  public:
  void *operator new(size_t);
  inline void *operator new(size_t size, void *ptr) { return ptr; };
  RexxMethod(ULONG, PCPPM, LONG, RexxInternalObject *);
  inline RexxMethod(RESTORETYPE restoreType) { ; };
  void execute(RexxObject *, RexxObject *);
  void live();
  void liveGeneral();
  void flatten(RexxEnvelope*);
  RexxObject   *unflatten(RexxEnvelope *envelope);
  RexxObject   *run(RexxActivity *,  RexxObject *, RexxString *,  size_t, RexxObject **);
  RexxObject   *call(RexxActivity *,  RexxObject *,  RexxString *,  RexxObject **, size_t, RexxString *, RexxString *, ULONG);
  RexxMethod  *newScope(RexxClass  *);
  RexxArray   *source();
  RexxObject  *setInterface( RexxDirectory *);
  void              setAttribute(RexxVariableBase *variable) {OrefSet(this, this->attribute, variable); }
  RexxVariableBase *getAttribute() { return this->attribute; };
  RexxDirectory *getInterface();
  void         setScope(RexxClass  *);
  RexxSmartBuffer  *saveMethod();
  RexxObject  *setUnGuardedRexx();
  RexxObject  *setGuardedRexx();
  RexxObject  *setPrivateRexx();
  RexxObject  *setProtectedRexx();
  RexxObject  *setSecurityManager(RexxObject *);

   inline USHORT methnum() {return this->methodInfo.methnum; };
   inline UCHAR  arguments() {return this->methodInfo.arguments; };
   inline UCHAR  flags() {return this->methodInfo.flags; };
   inline void   setMethnum(USHORT num) { this->methodInfo.methnum = num; };
   inline void   setFlags(UCHAR newFlags) { this->methodInfo.flags = newFlags; };
   inline void   setArguments(UCHAR args) { this->methodInfo.arguments = args; };

   inline BOOL   isGuarded()      {return !(this->methodInfo.flags & UNGUARDED_FLAG); };
   inline BOOL   isInternal()     {return this->methodInfo.flags & INTERNAL_FLAG; };
   inline BOOL   isPrivate()      {return this->methodInfo.flags & PRIVATE_FLAG;}
   inline BOOL   isProtected()    {return this->methodInfo.flags & PROTECTED_FLAG;}
   inline BOOL   isSpecial()      {return this->methodInfo.flags & (PROTECTED_FLAG | PRIVATE_FLAG);}

   inline BOOL   isRexxMethod()   {return this->methodInfo.flags & REXX_METHOD; };
   inline BOOL   isSOMMethod()    {return this->methodInfo.flags & SOM_METHOD; };
   inline BOOL   isNativeMethod() {return this->methodInfo.flags & NATIVE_METHOD; };
   inline BOOL   isCPPMethod()    {return this->methodInfo.flags & KERNEL_CPP_METHOD; };

   inline void   setUnGuarded()    {this->methodInfo.flags |= UNGUARDED_FLAG;};
   inline void   setGuarded()      {this->methodInfo.flags &= ~UNGUARDED_FLAG;};
   inline void   setInternal()     {this->methodInfo.flags |= INTERNAL_FLAG;};
   inline void   setPrivate()      {this->methodInfo.flags |= (PRIVATE_FLAG | PROTECTED_FLAG);};
   inline void   setProtected()    {this->methodInfo.flags |= PROTECTED_FLAG;};
   inline void   setRexxMethod()   {this->methodInfo.flags |= REXX_METHOD; };
   inline void   setSOMMethod()    {this->methodInfo.flags |= SOM_METHOD; };
   inline void   setNativeMethod() {this->methodInfo.flags |= NATIVE_METHOD; };
   inline void   setCPPMethod()    {this->methodInfo.flags |= KERNEL_CPP_METHOD; };

   RexxInteger *Private() { return  (this->isPrivate() ? TheTrueObject : TheFalseObject); };
   inline RexxObject *getCode() {return (RexxObject *)this->code;}
   inline RexxClass  *getScope() {return this->scope;}


   RexxClass  *scope;                  /* pointer to the method scope       */
   union {
     RexxDirectory *methodinterface;   /* SOM interface information         */
     RexxVariableBase *attribute;      /* method attribute info             */
   };
   union {
      RexxInternalObject *code;        /* associated "code" object          */
      RexxSOMCode        *somCode;     /* associated "code" object          */
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
  RexxMethod  *newNative(RexxString *, RexxString *, RexxClass  *);
  RexxMethod  *newEntry(PFN);
  RexxMethod  *newSom(RexxClass  *);
  RexxArray   *newArrayOfSOMMethods(RexxClass  *, long);
  RexxMethod  *restore(RexxBuffer *, PCHAR);
  RexxMethod  *newFile(RexxString *);
  RexxMethod  *newFileRexx(RexxString *);
};

#endif
