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
/******************************************************************************/
/* REXX Kernel                                             MethodClass.hpp    */
/*                                                                            */
/* Primitive Kernel Method Class Definitions                                  */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxMethod
#define Included_RexxMethod

#include "RexxCore.h"

class RexxSource;
class RexxActivity;
class RexxMethod;
class ProtectedObject;
class RexxArray;
class RexxClass;
class PackageClass;


/**
 * Base class for a code object.  Code objects can be invoked as
 * methods, or called.
 */
class BaseCode : public RexxInternalObject
{
public:
    virtual void run(RexxActivity *, RexxMethod *, RexxObject *, RexxString *,  RexxObject **, size_t, ProtectedObject &);
    virtual void call(RexxActivity *, RoutineClass *, RexxString *,  RexxObject **, size_t, RexxString *, RexxString *, int, ProtectedObject &);
    virtual void call(RexxActivity *, RoutineClass *, RexxString *,  RexxObject **, size_t, ProtectedObject &);
    virtual RexxArray *getSource();
    virtual RexxObject *setSecurityManager(RexxObject *manager);
    virtual RexxSource *getSourceObject();
    virtual RexxClass *findClass(RexxString *className);
    virtual BaseCode  *setSourceObject(RexxSource *s);
    virtual PackageClass *getPackage();
};
                                       /* pointer to native method function */
typedef uint16_t *(RexxEntry *PNATIVEMETHOD)(RexxMethodContext *, ValueDescriptor *);
                                       /* pointer to native function function*/
typedef uint16_t *(RexxEntry *PNATIVEROUTINE)(RexxCallContext *, ValueDescriptor *);

typedef size_t (RexxEntry *PREGISTEREDROUTINE)(const char *, size_t, PCONSTRXSTRING, const char *, PRXSTRING);

class BaseExecutable : public RexxObject
{
public:
    inline RexxSource *getSourceObject() { return code->getSourceObject(); };
    inline BaseCode   *getCode() { return code; }
    RexxArray  *getSource() { return code->getSource(); }
    PackageClass *getPackage();

    RexxArray *source();
    RexxClass *findClass(RexxString *className);
    BaseExecutable *setSourceObject(RexxSource *s);

protected:
    RexxString *executableName;     // the created name of this routine
    BaseCode   *code;                   // the backing code object
};


 class RexxMethod : public BaseExecutable
 {
  public:
  void *operator new(size_t);
  inline void *operator new(size_t size, void *ptr) { return ptr; };
  RexxMethod(RexxString *name, BaseCode *_code);
  RexxMethod(RexxString *name, RexxSource *source);
  RexxMethod(RexxString *name);
  RexxMethod(RexxString *name, RexxBuffer *source);
  RexxMethod(RexxString *name, const char *data, size_t length);
  RexxMethod(RexxString *name, RexxArray *source);
  inline RexxMethod(RESTORETYPE restoreType) { ; };

  void execute(RexxObject *, RexxObject *);
  void live(size_t);
  void liveGeneral(int reason);
  void flatten(RexxEnvelope*);

  void         run(RexxActivity *,  RexxObject *, RexxString *,  RexxObject **, size_t, ProtectedObject &);
  RexxMethod  *newScope(RexxClass  *);
  void         setScope(RexxClass  *);
  RexxSmartBuffer  *saveMethod();
  RexxObject  *setUnguardedRexx();
  RexxObject  *setGuardedRexx();
  RexxObject  *setPrivateRexx();
  RexxObject  *setProtectedRexx();
  RexxObject  *setSecurityManager(RexxObject *);

  RexxObject  *isGuardedRexx();
  RexxObject  *isPrivateRexx();
  RexxObject  *isProtectedRexx();

   inline bool   isGuarded()      {return (this->methodFlags & UNGUARDED_FLAG) == 0; };
   inline bool   isPrivate()      {return (this->methodFlags & PRIVATE_FLAG) != 0;}
   inline bool   isProtected()    {return (this->methodFlags & PROTECTED_FLAG) != 0;}
   inline bool   isSpecial()      {return (this->methodFlags & (PROTECTED_FLAG | PRIVATE_FLAG)) != 0;}

   inline void   setUnguarded()    {this->methodFlags |= UNGUARDED_FLAG;};
   inline void   setGuarded()      {this->methodFlags &= ~UNGUARDED_FLAG;};
   inline void   setPrivate()      {this->methodFlags |= (PRIVATE_FLAG | PROTECTED_FLAG);};
   inline void   setProtected()    {this->methodFlags |= PROTECTED_FLAG;};
          void   setAttributes(bool _private, bool _protected, bool _guarded);
   inline RexxClass *getScope() {return this->scope;}

   inline BaseCode  *getCode()     { return this->code; }
   RexxMethod  *newRexx(RexxObject **, size_t);
   RexxMethod  *newFileRexx(RexxString *);
   RexxMethod  *loadExternalMethod(RexxString *name, RexxString *descriptor);

   static RexxMethod  *newMethodObject(RexxString *, RexxObject *, RexxObject *, RexxSource *a);
   static RexxMethod  *restore(RexxBuffer *, char *, size_t length);

   static void createInstance();
   static RexxClass *classInstance;

 protected:
   enum
   {
       PRIVATE_FLAG      = 0x01,        // private method
       UNGUARDED_FLAG    = 0x02,        // Method can run with GUARD OFF
       PROTECTED_FLAG    = 0x40,        // method is protected
   };

   size_t    methodFlags;              // method status flags
   RexxClass  *scope;                  /* pointer to the method scope       */
};

#endif
