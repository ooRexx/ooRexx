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
/* REXX Kernel                                                  ClassClass.hpp   */
/*                                                                            */
/* Primitive Class Class Definitions                                          */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxClass
#define Included_RexxClass

#define REXX_DEFINED      0x00000001    /* this class is a native rexx class */
#define IMPORTED          0x00000002    /* this class was imported to som    */
#define MIXIN             0x00000004    /* this is a mixin class             */
#define HAS_UNINIT        0x00000008    /* this class has an uninit method   */
#define META_CLASS        0x00000010    /* this class is a meta class        */
#define PRIMITIVE_CLASS   0x00000020    /* this is a primitive class         */
#define PARENT_HAS_UNINIT 0x00000040

void class_create (void);

 class RexxClass : public RexxObject {
  public:
   inline RexxClass(){;};
   inline RexxClass(RESTORETYPE restoreType) { ; };
   void *operator new(size_t, long, RexxBehaviour *, RexxBehaviour *);
   inline void *operator new(size_t size, void *ptr) {return ptr;};
   void live();
   void liveGeneral();
   void flatten(RexxEnvelope*);
   RexxObject *unflatten(RexxEnvelope*);
   RexxObject *makeProxy(RexxEnvelope*);
   BOOL        isEqual(RexxObject *);

   ULONG        hash();
   RexxObject * equal(RexxObject *);
   RexxObject * strictEqual(RexxObject *);
   RexxObject * notEqual(RexxObject *);
   RexxObject * setRexxDefined();
   RexxInteger *queryMixinClass();
   RexxString  *getId();
   RexxClass   *getBaseClass();
   RexxClass   *getMetaClass();
   RexxInteger *getSomClass();
   void         setSomClass(RexxInteger *);
   RexxClass   *getSuperClass();
   RexxArray   *getSuperClasses();
   RexxArray   *getSubClasses();
   void         defmeths(RexxTable *);
   void         setInstanceBehaviour(RexxBehaviour *);
   RexxTable  *getInstanceBehaviourDictionary();
   RexxTable  *getBehaviourDictionary();
   RexxString *defaultName();
   void        subClassable(PCHAR, bool);
   RexxObject *defineMethod(RexxString *, RexxMethod *);
   RexxObject *defineMethods(RexxTable *);
   RexxObject *deleteMethod(RexxString *);
   RexxMethod *method(RexxString *);
   RexxSupplier *methods(RexxClass *);
   void        updateSubClasses();
   void        updateInstanceSubClasses();
   void        createClassBehaviour(RexxBehaviour *);
   void        createInstanceBehaviour(RexxBehaviour *);
   void        methodDictionaryMerge(RexxTable *, RexxTable *);
   RexxTable  *methodDictionaryCreate(RexxTable *, RexxClass *);
   RexxObject *somSuperClass(RexxClass *);
   RexxObject *inherit(RexxClass *, RexxClass *);
   RexxObject *uninherit(RexxClass *);
   RexxObject *enhanced(RexxObject **, size_t);
   RexxClass  *mixinclass(RexxString *, RexxClass *, RexxTable *);
   RexxClass  *subclass(RexxString *, RexxClass *, RexxTable *);
   RexxInteger *importedRexx();
   RexxObject *importMethod();
   RexxObject *exportMethod(RexxString *, RexxString *, long, RexxClass *);
   RexxObject *somDefine(RexxString *, RexxInteger *);
   long        somInterfaces();
   RexxSOMProxy *newOpart(RexxInteger *);
   RexxClass  *newRexx(RexxObject **args, size_t argCount);
   void        setMetaClass(RexxClass *);
   RexxClass  *external(RexxString *, RexxClass *, RexxTable *);
   bool        isCompatibleWith(RexxClass *other);
   RexxObject *isSubclassOf(RexxClass *other);
   RexxString  *defaultNameRexx();


   inline BOOL         rexxDefined() { return this->class_info & REXX_DEFINED; };
   inline BOOL         imported()    { return this->class_info & IMPORTED; }
   inline void         setImported() { this->class_info |= IMPORTED; }
   inline BOOL         queryMixin()  { return this->class_info & MIXIN; };
   inline BOOL         queryMeta()   { return this->class_info & META_CLASS; };
   inline BOOL         uninitDefined()   { return this->class_info & HAS_UNINIT; };
   inline BOOL         parentUninitDefined()   { return this->class_info & PARENT_HAS_UNINIT; };
   inline void         parentHasUninit()   { this->class_info |= PARENT_HAS_UNINIT; };
   inline BOOL         isPrimitive() { return this->class_info & PRIMITIVE_CLASS; }
   inline void         setMixinClass() { this->class_info |= MIXIN; }
   inline void         setNotPrimitive(){this->class_info &= ~PRIMITIVE_CLASS; return;};
   inline RexxBehaviour *getInstanceBehaviour() {return this->instanceBehaviour;};
   inline void         setMeta() { this->class_info |= META_CLASS; }
   inline void         addSubClass(RexxClass *);

                                       /* Subclassable and subclassed       */
    RexxString    *id;                 /* classes will have a name string   */
                                       /* class methods specific to this    */
                                       /* class                             */
    RexxTable     *classMethodDictionary;
                                       /* instances of this class inherit   */
    RexxBehaviour *instanceBehaviour;  /* this behaviour                    */
                                       /* methods added to this class       */
    RexxTable     *instanceMethodDictionary;
    RexxClass     *baseClass;          /* Baseclass of this class           */
    RexxArray     *metaClass;          /* Metaclass of this class           */
                                       /* Metaclass mdict                   */
    RexxArray     *metaClassMethodDictionary;
    RexxObjectTable *metaClassScopes;  /* Metaclass scopes                  */
    RexxInteger   *somClass;           /* Root SomClass if there is one     */
                                       /* The superclass and any inherited  */
    RexxArray     *classSuperClasses;  /* mixins for class behaviour        */
                                       /* The superclass and any inherited  */
                                       /* mixins for instance behaviour     */
    RexxArray     *instanceSuperClasses;
                                       /* class specific information        */
                                       /* defines for this field are at the */
    ULONG class_info;                  /* top of this header file           */
 };
 #endif
