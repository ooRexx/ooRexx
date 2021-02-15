/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/* REXX Kernel                                               ClassClass.hpp   */
/*                                                                            */
/* Primitive Class Class Definitions                                          */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxClass
#define Included_RexxClass

#include "FlagSet.hpp"

class PackageClass;
class SourceTable;
class StringTable;
class TableClass;
class ArrayClass;
class MethodDictionary;
/**
 * The class defintion for the Rexx CLASS class.
 */
class ListClass;

class RexxClass : public RexxObject
{
 public:
    void *operator new(size_t);
    inline void operator delete(void *) { }

    inline RexxClass(){;};
    inline RexxClass(RESTORETYPE restoreType) { ; };
           RexxClass(const char *id , RexxBehaviour *classBehaviour, RexxBehaviour *instanceBehaviour);

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    RexxObject * makeProxy(Envelope*) override;

    HashCode     hash() override;
    HashCode     getHashValue() override;

    RexxObject * equal(RexxObject *);
    RexxObject * strictEqual(RexxObject *);
    RexxObject * notEqual(RexxObject *);

    // start of methods used only during image build
    RexxObject * setRexxDefined();
    void        buildFinalClassBehaviour();
    void        buildFinalClassBehaviour(RexxClass *superClass);
    void        mergeSuperClassScopes(RexxBehaviour *target_instance_behaviour);
    RexxObject *defineMethod(RexxString *, RexxObject *);
    RexxObject *defineMethodsRexx(RexxObject * methods);
    RexxObject *deleteMethod(RexxString *);
    RexxObject *defineClassMethod(RexxString *method_name, MethodClass *newMethod);
    void        removeClassMethod(RexxString *method_name);
    void        removeSetupMethods();
    RexxObject *defineMethods(StringTable *);
    void        inheritInstanceMethods(RexxClass *);
    RexxObject *inheritInstanceMethodsRexx(RexxClass *);

    MethodDictionary *getInstanceBehaviourDictionary();
    MethodDictionary *getBehaviourDictionary();

    // methods for building class behaviours
    void        updateSubClasses();
    void        updateInstanceSubClasses();
    void        createClassBehaviour(RexxBehaviour *);
    void        createInstanceBehaviour(RexxBehaviour *);
    void        mergeBehaviour(RexxBehaviour *target_instance_behaviour);
    MethodDictionary *createMethodDictionary(RexxObject *sourceCollection, RexxClass *scope);
    MethodDictionary *copyInstanceMethods();
    void mergeInstanceBehaviour(RexxBehaviour *targetBehaviour);
    void mergeClassMethodDictionary(RexxBehaviour *targetBehaviour);

    RexxObject  *queryMixinClass();
    RexxObject  *isMetaClassRexx();
    RexxObject  *isAbstractRexx();
    RexxString  *getId();
    RexxClass   *getBaseClass();
    RexxClass   *getMetaClass();
    RexxClass   *getSuperClass();
    ArrayClass  *getSuperClasses();
    ArrayClass  *getSubClasses();
    RexxObject  *copyRexx();

    void         setInstanceBehaviour(RexxBehaviour *);
    RexxString  *defaultName() override;

    MethodClass *method(RexxString *);

    SupplierClass *methods(RexxClass *);
    RexxObject *inherit(RexxClass *, RexxClass *);
    RexxObject *uninherit(RexxClass *);
    RexxObject *enhanced(RexxObject **, size_t);
    RexxClass  *mixinClass(PackageClass *, RexxString *, RexxClass *, RexxObject *);
    RexxClass  *subclass(PackageClass *, RexxString *, RexxClass *, RexxObject *);
    RexxClass  *mixinClassRexx(RexxString *, RexxClass *, RexxObject *);
    RexxClass  *subclassRexx(RexxString *, RexxClass *, RexxObject *);
    RexxClass  *newRexx(RexxObject **args, size_t argCount);
    void        setMetaClass(RexxClass *);
    bool        isCompatibleWith(RexxClass *other);
    RexxObject *isSubclassOf(RexxClass *other);
    RexxString *defaultNameRexx();
    RexxObject *getAnnotationRexx(RexxObject *name);
    void        setPackage(PackageClass *s);
    PackageClass *getPackage();
    void        completeNewObject(RexxObject *obj, RexxObject **initArgs = OREF_NULL, size_t argCount = 0);
    StringTable *getAnnotations();
    RexxString *getAnnotation(RexxString *name);
    void        setAnnotations(StringTable *annotations);

    inline bool         isRexxDefined() { return classFlags[REXX_DEFINED]; }
    inline bool         isMixinClass()  { return classFlags[MIXIN]; }
    inline bool         isMetaClass() { return classFlags[META_CLASS]; }
    inline bool         isAbstract() { return classFlags[ABSTRACT]; }
    inline bool         hasUninitDefined()   { return classFlags[HAS_UNINIT]; }
    inline void         setHasUninitDefined()   { classFlags.set(HAS_UNINIT); }
    inline void         clearHasUninitDefined()   { classFlags.reset(HAS_UNINIT); }
    // NB:  This clears every flag BUT the UNINIT flag
    inline void         setInitialFlagState()   { bool uninit = classFlags[HAS_UNINIT]; classFlags.reset(); classFlags.set(HAS_UNINIT, uninit); }
    inline bool         parentHasUninitDefined()   { return classFlags[PARENT_HAS_UNINIT]; }
    inline void         setParentHasUninitDefined()   { classFlags.set(PARENT_HAS_UNINIT); }
    inline bool         isPrimitiveClass() { return classFlags[PRIMITIVE_CLASS]; }
    inline void         setMixinClass() { classFlags.set(MIXIN); }
    inline void         setNonPrimitive() { classFlags.reset(PRIMITIVE_CLASS); }
    inline RexxBehaviour *getInstanceBehaviour() {return instanceBehaviour;}
    inline void         setMetaClass() { classFlags.set(META_CLASS); }
    inline void         setAbstract() { classFlags.set(ABSTRACT); }
    inline void         clearAbstract() { classFlags.reset(ABSTRACT); }
           void         addSubClass(RexxClass *);
           void         removeSubclass(RexxClass *c);
           void         checkUninit();
           void         checkAbstract();
           void         makeAbstract();

    static void processNewArgs(RexxObject **, size_t, RexxObject **&, size_t &, size_t, RexxObject *&, RexxObject **);

    static void createInstance();
    // singleton class instance;
    static RexxClass *classInstance;

 protected:

    typedef enum
    {
        REXX_DEFINED,                     // this class is a native rexx class
        MIXIN,                            // this is a mixin class
        HAS_UNINIT,                       // this class has an uninit method
        META_CLASS,                       // this class is a meta class
        PRIMITIVE_CLASS,                  // this is a primitive class
        PARENT_HAS_UNINIT,                // the class parent has an uninit method
        ABSTRACT,                         // the class is abstract
    } ClassFlag;

                                       // Subclassable and subclassed
    RexxString    *id;                 // classes will have a name string
    // class methods specific to this class.                                */
    MethodDictionary *classMethodDictionary;
    // methods defined at this class level.
    MethodDictionary *instanceMethodDictionary;
    // instances of this class will be given this behaviour (merged behaviour from
    // superclasses/mixins
    RexxBehaviour *instanceBehaviour;

    RexxClass     *baseClass;          // Baseclass of this class
    RexxClass     *metaClass;          // Metaclass of this class
    RexxClass     *superClass;         // immediate super class of this class.
    // the super class and any inherited mixins that contribute to instance behaviour.
    ArrayClass     *superClasses;
    FlagSet<ClassFlag, 32> classFlags; // class attributes

    ListClass     *subClasses;         // our list of weak referenced subclasses
    PackageClass  *package;            // source we're defined in (if any)
    StringTable   *annotations;        // annotations attached to the class (if any)
};
#endif
