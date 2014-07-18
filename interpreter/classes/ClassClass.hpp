/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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

// required for method signatures
class RexxSource;
class PackageClass;
class SourceTable;

class RexxClass : public RexxObject
{
 public:
    void *operator new(size_t);
    inline void *operator new(size_t size, void *ptr) {return ptr;};
    inline void operator delete(void *) { }
    inline void operator delete(void *, void *) { }

    inline RexxClass(){;};
    inline RexxClass(RESTORETYPE restoreType) { ; };
           RexxClass(const char *id , RexxBehaviour *classBehaviour, RexxBehaviour *instanceBehaviour);

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);

    virtual RexxObject *makeProxy(RexxEnvelope*);
    virtual bool        isEqual(RexxObject *);

    HashCode     hash();
    HashCode     getHashValue();
    RexxObject * equal(RexxObject *);
    RexxObject * strictEqual(RexxObject *);
    RexxObject * notEqual(RexxObject *);
    RexxObject * setRexxDefined();
    RexxInteger *queryMixinClass();
    RexxString  *getId();
    RexxClass   *getBaseClass();
    RexxClass   *getMetaClass();
    RexxClass   *getSuperClass();
    ArrayClass   *getSuperClasses();
    ArrayClass   *getClassSuperClasses() { return classSuperClasses; }
    ArrayClass   *getSubClasses();
    void         defineMethods(TableClass *);
    void         setInstanceBehaviour(RexxBehaviour *);
    TableClass  *getInstanceBehaviourDictionary();
    TableClass  *getBehaviourDictionary();
    RexxString *defaultName();
    void        subClassable(bool);
    void        subClassable(RexxClass *superClass, bool restricted);
    void        mergeSuperClassScopes(RexxBehaviour *target_instance_behaviour);
    RexxObject *defineMethod(RexxString *, MethodClass *);
    RexxObject *defineMethods(TableClass *);
    RexxObject *deleteMethod(RexxString *);
    RexxObject *defineClassMethod(RexxString *method_name, MethodClass *newMethod);
    void        removeClassMethod(RexxString *method_name);
    MethodClass *method(RexxString *);
    SupplierClass *methods(RexxClass *);
    void        updateSubClasses();
    void        updateInstanceSubClasses();
    void        createClassBehaviour(RexxBehaviour *);
    void        createInstanceBehaviour(RexxBehaviour *);
    void        methodDictionaryMerge(TableClass *, TableClass *);
    TableClass  *methodDictionaryCreate(TableClass *, RexxClass *);
    RexxObject *inherit(RexxClass *, RexxClass *);
    RexxObject *uninherit(RexxClass *);
    RexxObject *enhanced(RexxObject **, size_t);
    RexxClass  *mixinclass(RexxSource *, RexxString *, RexxClass *, TableClass *);
    RexxClass  *subclass(RexxSource *, RexxString *, RexxClass *, TableClass *);
    RexxClass  *mixinclassRexx(RexxString *, RexxClass *, TableClass *);
    RexxClass  *subclassRexx(RexxString *, RexxClass *, TableClass *);
    RexxClass  *newRexx(RexxObject **args, size_t argCount);
    void        setMetaClass(RexxClass *);
    bool        isCompatibleWith(RexxClass *other);
    RexxObject *isSubclassOf(RexxClass *other);
    RexxString  *defaultNameRexx();
    void        setSource(RexxSource *s);
    RexxSource *getSource();
    RexxObject *getPackage();
    void        completeNewObject(RexxObject *obj, RexxObject **initArgs = OREF_NULL, size_t argCount = 0);

    inline bool         isRexxDefined() { return classFlags[REXX_DEFINED]; }
    inline bool         isMixinClass()  { return classFlags[MIXIN]; }
    inline bool         isMetaClass() { return classFlags[META_CLASS]; }
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
           void         addSubClass(RexxClass *);
           void         removeSubclass(RexxClass *c);
           ScopeTable  *copyScopes();
           ArrayClass   *allScopes();
           TableClass   *copyInstanceMethods();
           ScopeTable  *copyMetaclassScopes();
           RexxClass   *getSuperScope() { return scopeSuperClass; }
           ArrayClass   *getScopeOrder() { return scopeSearchOrder; }

    static void processNewArgs(RexxObject **, size_t, RexxObject ***, size_t *, size_t, RexxObject **, RexxObject **);

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
        PARENT_HAS_UNINIT,
    } ClassFlag;

                                       // Subclassable and subclassed
    RexxString    *id;                 // classes will have a name string
    // class methods specific to this class.                                */
    MethodDictionary *classMethodDictionary;
    // instances of this class will be given this behaviour.
    RexxBehaviour *instanceBehaviour;

    RexxClass     *baseClass;          // Baseclass of this class
    RexxClass     *metaClass;          // Metaclass of this class
    // the super class and any inherited mixins for class
    // behaviour
    ArrayClass     *classSuperClasses;
    // the super class and any inherited mixins that contribute to instance behaviour.
    ArrayClass     *instanceSuperClasses;
    FlagSet<ClassFlag, 32> classFlags; // class attributes

    ListClass      *subClasses;         // our list of weak referenced subclasses
    RexxSource    *source;             // source we're defined in (if any)
    RexxClass     *scopeSuperClass;    // the immediate superclass used for lookups starting from this point.
    RexxArrray    *scopeSearchOrder;   // the search order used for searches starting from this scope position.
};
#endif
