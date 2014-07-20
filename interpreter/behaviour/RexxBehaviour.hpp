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
/* REXX Kernel                                            RexxBehaviour.hpp   */
/*                                                                            */
/* Primitive Behaviour Class Definitions                                      */
/*                                                                            */
/*******************************************************************************/
#ifndef Included_RexxBehaviour
#define Included_RexxBehaviour

#include "FlagSet.hpp"

class MethodDictionary;


/**
 * The class that defines base object behaviour (i.e., what methods can be invoked)
 */
class RexxBehaviour : public RexxInternalObject
{
 public:
    void *operator new(size_t, size_t);
    inline void *operator new(size_t size, void *ptr) {return ptr;};
    inline void  operator delete(void *, size_t) { }
    inline void  operator delete(void *, void *) { ; }


    static const uintptr_t INTERNALCLASS = (((uintptr_t)1) << ((sizeof(uintptr_t) * 8) - 1));

    RexxBehaviour(size_t, PCPPM *);
    inline RexxBehaviour() {;};
    inline RexxBehaviour(RESTORETYPE restoreType) { ; };

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope*);

    virtual RexxObject *copy();
    void         copyBehaviour(RexxBehaviour *source);
    void         defineMethod(RexxString *, MethodClass *);
    void         defineMethod(const char *, PCPPM, size_t);
    void         addInstanceMethod(RexxString *, MethodClass *);
    void         removeInstanceMethod(RexxString *);
    MethodClass *methodObject(RexxString *);
    MethodClass *methodLookup( RexxString *);
    MethodClass *getMethod( RexxString *);
    RexxObject  *deleteMethod(RexxString *);
    void         restore(RexxBehaviour *);
    RexxClass   *restoreClass();
    RexxObject  *superScope( RexxObject *);
    MethodClass *superMethod(RexxString *, RexxObject *);
    void         subclass(RexxBehaviour *);
    SupplierClass *getMethods(RexxObject *scope);

    void        resolveNonPrimitiveBehaviour();

    void        merge(RexxBehaviour *);
    void        methodDictionaryMerge(MethodDictionary *);
    MethodDictionary *copyMethodDictionary();

    inline MethodDictionary *getMethodDictionary()   { return methodDictionary; };
           void        setMethodDictionary(MethodDictionary *m);
    inline void        clearMethodDictionary() { setMethodDictionary(OREF_NULL); }
    inline RexxClass  *getOwningClass()        { return owningClass;};
           void        setOwningClass(RexxClass *c);
           ArrayClass  *allScopes();

    inline void  setClassType(size_t n) { classType = (uint16_t)n; }
    inline size_t getClassType()  { return (size_t)classType; }
    inline bool  isPrimitive()    {  return !behaviourFlags[NON_PRIMITIVE_BEHAVIOUR]; };
    inline bool  isNonPrimitive() {  return behaviourFlags[NON_PRIMITIVE_BEHAVIOUR]; };
    inline bool  isNotResolved()  {  return behaviourFlags[BEHAVIOUR_NOT_RESOLVED]; };
    inline bool  isResolved()     {  return !behaviourFlags[BEHAVIOUR_NOT_RESOLVED]; };
    inline bool  isEnhanced()     {  return behaviourFlags[ENHANCED_OBJECT]; };
    inline bool  isInternalClass()  {  return behaviourFlags[INTERNAL_CLASS]; };
    inline bool  isTransientClass()  {  return behaviourFlags[TRANSIENT_CLASS]; };
    inline void  setResolved()    {  behaviourFlags.reset(BEHAVIOUR_NOT_RESOLVED); };
    inline void  setNotResolved() {  behaviourFlags.set(BEHAVIOUR_NOT_RESOLVED); };
    inline void  setEnhanced()    {  behaviourFlags.set(ENHANCED_OBJECT); };
    inline void  setNonPrimitive() {  behaviourFlags.set(NON_PRIMITIVE_BEHAVIOUR); };
    inline void  setInternalClass() { behaviourFlags.set(INTERNAL_CLASS); };
    inline void  setTransientClass() { behaviourFlags.set(TRANSIENT_CLASS); };

    inline bool  hasScope(RexxClass *scope) { return instanceMethodDictionary->hasScope(scope); }

    inline RexxBehaviour *getSavedPrimitiveBehaviour()
    {
        uintptr_t behaviourID = getClassType();
        // if this is an internal class, normalize this so we can
        // restore this to the correct value if we add additional internal classes.
        if (isInternalClass())
        {
            behaviourID -= T_Last_Exported_Class;
            behaviourID |= INTERNALCLASS;
        }
        return (RexxBehaviour *)behaviourID;
    }

    static inline RexxBehaviour *restoreSavedPrimitiveBehaviour(RexxBehaviour *b)
    {
        uintptr_t behaviourID = (uintptr_t)b;
        // if this is an internal class, we need to convert back to
        // the relative internal class id
        if ((behaviourID & INTERNALCLASS) != 0)
        {
            behaviourID &= ~INTERNALCLASS;    // turn off the internal marker
            behaviourID += T_Last_Exported_Class; // turn back into an internal class
        }
        return &primitiveBehaviours[behaviourID];          // translate back into proper behaviour
    }

    inline PCPPM getOperatorMethod(size_t index) { return operatorMethods[index]; }
    static inline RexxBehaviour *getPrimitiveBehaviour(size_t index) { return &primitiveBehaviours[index]; }
    static inline PCPPM *getOperatorMethods(size_t index) { return getPrimitiveBehaviour(index)->operatorMethods; }
    // table of primitive behaviour objects
    static RexxBehaviour primitiveBehaviours[];

 protected:


    typedef enum
    {
        NON_PRIMITIVE_BEHAVIOUR,
        ENHANCED_OBJECT,
        INTERNAL_CLASS,
        TRANSIENT_CLASS,
        BEHAVIOUR_NOT_RESOLVED,
    } BehaviourFlag;


    size_t   classType;                   // primitive class identifier
    FlagSet<BehaviourFlag, 32> behaviourFlags; // various behaviour flag types
    MethodDictionary *methodDictionary;   // method dictionary obtained from our class.
    PCPPM      *operatorMethods;          // operator look-a-side table
    RexxClass  *owningClass;              // class that created this object
};


/******************************************************************************/
/* Global Objects - Primitive Behaviour                                       */
/******************************************************************************/

#include "PrimitiveBehaviourNames.h"

#endif
