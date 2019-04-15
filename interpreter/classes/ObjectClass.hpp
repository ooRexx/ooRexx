/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                               ObjectClass.hpp  */
/*                                                                            */
/* Primitive Object Class Definitions                                         */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxObject
#define Included_RexxObject

#include <new>
#include <string.h>
#include "Numerics.hpp"
#include "RexxErrorCodes.h"

class RexxInternalObject;
class RexxObject;
class RexxInteger;
class RexxBehaviour;
class CompoundVariableTail;
class CompoundTableElement;
class SupplierClass;
class Envelope;
class VariableDictionary;
class NumberString;
class MethodClass;
class MessageClass;
class ProtectedObject;
class SecurityManager;
class BaseExecutable;
class Activity;
class PointerTable;


typedef size_t HashCode;               // a hash code value

// Special type used for building the virtual function table. //
typedef enum {RESTOREIMAGE} RESTORETYPE;


/**
 * Typedef for the marking reasons passed to all liveGeneral methods.
 */
typedef enum
{
    NOREASON,               // Nothing configured currently.
    LIVEMARK,               // Performing debug live marking
    RESTORINGIMAGE,         // marking during image restore
    PREPARINGIMAGE,         // marking to allow and image save preparation.
    SAVINGIMAGE,            // saving the Rexx image
    FLATTENINGOBJECT,       // marking to flatten an object
    UNFLATTENINGOBJECT,     // marking to unflatten an object
} MarkReason;


/**
 * The header that is at the beginning of every object instanct.
 */
class ObjectHeader
{
friend class MemoryObject;
public:
    inline ObjectHeader & operator= (ObjectHeader &h)
    {
        // copy the relevant state
        objectSize = h.objectSize;
        flags = h.flags;
        return *this;
    }

    inline size_t getObjectSize() { return objectSize; }
    inline void setObjectSize(size_t l)
    {
        objectSize = l;
    }

    inline void makeProxiedObject() { flags |= ProxiedObject; }
    inline bool requiresProxyObject() { return (flags & ProxiedObject) != 0; }
    inline void makeProxyObject() { flags |= ProxyObject; }
    inline bool isProxyObject() { return (flags & ProxyObject) != 0; }
    inline void clearObjectMark() { flags &= LiveMask; }
    inline void setObjectMark(size_t mark) { clearObjectMark(); flags |= mark; }
    inline bool isObjectMarked(size_t mark) { return (flags & mark) != 0; }
    inline bool isObjectLive(size_t mark) { return ((size_t)(flags & MarkMask)) == mark; }
    inline bool isObjectDead(size_t mark) { return ((size_t)(flags & MarkMask)) != mark; }
    inline void clear() { objectSize = 0; flags = 0; }
    inline void setOldSpace() { flags |= OldSpaceBit; }
    inline void clearOldSpace() { flags &= ~OldSpaceBit; }
    inline void setNewSpace() { clearOldSpace(); }
    inline bool isOldSpace() { return (flags & OldSpaceBit) != 0; }
    inline bool isNewSpace() { return (flags & OldSpaceBit) == 0; }
    inline void setHasNoReferences() { flags |= NoRefBit; }
    inline void setHasReferences() { flags &= ~NoRefBit; }
    inline bool hasReferences() { return (flags & NoRefBit) == 0; }
    inline bool hasNoReferences() { return (flags & NoRefBit) != 0; }
    inline void setNonPrimitive() { flags |= IsNonPrimitive; }
    inline void setPrimitive() { flags &= ~IsNonPrimitive; }
    inline bool isNonPrimitive() { return (flags & IsNonPrimitive) != 0; }
    inline bool isPrimitive() { return (flags & IsNonPrimitive) == 0; }
    inline void setReadyForUninit() { flags |= UninitPending; }
    inline bool isReadyForUninit() { return (flags & UninitPending) != 0; }
    inline bool hasUninit() { return (flags & HasUninit) != 0; }
    inline void setHasUninit() { flags |= HasUninit; }
    inline void clearHasUninit() { flags &= ~HasUninit; }
    inline bool isDeadObject() { return (flags & DeadObject) != 0; }
    inline void setDeadObject() { flags |= DeadObject; }
    inline void clearDeadObject() { flags &= ~DeadObject; }
    inline void initHeader(size_t l, size_t mark)
    {
        objectSize = l;
        flags = (uint16_t)mark;    // the flags are cleared except for the mark.
    }
    inline void initHeader(size_t mark)
    {
        flags = (uint16_t)mark;    // the flags are cleared except for the mark.
    }

protected:
    enum
    {
        MarkBit1         =  0x0001,    // location of the first mark bit.  Note:  shared with IsNonPrimitive
        MarkBit2         =  0x0002,    // Second of the mark bits
        LiveMask         =  0xFFFC,    // mask for the checking the mark bits
        MarkMask         =  0x0003,    // mask use for checking the mark bits
        ProxiedObject    =  0x0004,    // This requires a proxy
        ProxyObject      =  0x0008,    // This object is a PROXY(String) Obj
        IsNonPrimitive   =  0x0010,    // use for flattened objects to indicated behaviour status
        NoRefBit         =  0x0020,    // location of No References Bit.
        OldSpaceBit      =  0x0040,    // location of the OldSpace bit
        UninitPending    =  0x0080,    // we have an uninit operation pending
        HasUninit        =  0x0100,    // this object has an uninit method
        DeadObject       =  0x0200,    // this is a dead object
    };

    size_t    objectSize;              // allocated size of the object
    union
    {
        uint16_t      flags;           // the object flag/type information
        uintptr_t     sizePadding;     // padding to make sure this is a full pointer size
    };

};


/**
 * create first base level class dummy virtual function to force
 * the virtual function table to a specific location.  Different
 * compilers place the virtual function table pointer at
 * different locations.  By having virtual methods defined on
 * the very first class level, we can get things where we expect
 * to find it.
 */
class RexxVirtualBase
{
 protected:
    virtual ~RexxVirtualBase() { ; }
    virtual void      baseVirtual() {;}

 public:
    // the following need to be defined at the base virtual level.  When
    // an exception is thrown from within an object constructor, the destructors
    // unwind and the constructed object just ends up with a virtual base
    // vft.  If the garbage collector sees this, it will crash unless these
    // are defined at this level.
    virtual void live(size_t) {;}
    virtual void liveGeneral(MarkReason reason) {;}
    virtual void flatten(Envelope *) {;}
    virtual RexxInternalObject *unflatten(Envelope *) { return (RexxInternalObject *)this; };
};

class RexxObject;

/******************************************************************************/
/* Method pointer special types                                               */
/******************************************************************************/

typedef RexxObject *  (RexxObject::*PCPPM0)();
typedef RexxObject *  (RexxObject::*PCPPM1)(RexxObject *);
typedef RexxObject *  (RexxObject::*PCPPM2)(RexxObject *, RexxObject *);
typedef RexxObject *  (RexxObject::*PCPPM3)(RexxObject *, RexxObject *, RexxObject *);
typedef RexxObject *  (RexxObject::*PCPPM4)(RexxObject *, RexxObject *, RexxObject *, RexxObject *);
typedef RexxObject *  (RexxObject::*PCPPM5)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
typedef RexxObject *  (RexxObject::*PCPPM6)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
typedef RexxObject *  (RexxObject::*PCPPM7)(RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
typedef RexxObject *  (RexxObject::*PCPPMA1)(ArrayClass *);
typedef RexxObject *  (RexxObject::*PCPPMC1)(RexxObject **, size_t);

// pointer to a method function
typedef RexxObject *  (RexxObject::*PCPPM)();
#define CPPM(n) ((PCPPM)&n)

/**
 * Base class for an object Rexx object.  This class
 * is used for internal objects only.  An internal object
 * is one that is not exposed to the Rexx programmer and
 * are only used internal to the interpreter.  Objects
 * of this type are allocated from the ooRexx object heap
 * and participate in garbage collection.
 */
class RexxInternalObject : public RexxVirtualBase
{
 public:
    inline RexxInternalObject() {;};
    /**
     * Following constructor used to reconstruct the Virtual
     * Functions table, o it doesn't need to do anything.
     * Every class defined in PrimitiveClasses.xml will need
     * to provide one of these.
     *
     * @param restoreType
     *               Dummy argument used for contstructor signature matches.
     */
    inline RexxInternalObject(RESTORETYPE restoreType) { ; };
    virtual ~RexxInternalObject() {;};

    inline operator RexxObject*() { return (RexxObject *)this; };

    inline size_t getObjectSize() { return header.getObjectSize(); }
    inline void   setObjectSize(size_t s) { header.setObjectSize(s); }
    // NB:  I hope this doesn't add any padding
    static inline size_t getObjectHeaderSize() { return sizeof(RexxInternalObject); }
    inline size_t getObjectDataSize() { return getObjectSize() - getObjectHeaderSize(); }
    inline void  *getObjectDataSpace() { return ((char *)this) + getObjectHeaderSize(); }

    inline RexxInternalObject *nextObject() { return (RexxInternalObject *)(((char *)this) + getObjectSize()); }
    inline RexxInternalObject *nextObject(size_t l) { return (RexxInternalObject *)(((char *)this) + l); }
    // these clear everything after the hash value.
    inline void   clearObject() { memset(getObjectDataSpace(), '\0', getObjectDataSize()); }
    inline void   clearObject(size_t l) { memset(getObjectDataSpace(), '\0', l - getObjectHeaderSize()); }
    inline void   setVirtualFunctions(void *t) { *((void **)this) = t; }
    inline bool   checkVirtualFunctions() { return *((void **)this) != NULL; }

    inline void   setInitHeader(size_t s, size_t markword)  { header.initHeader(s, markword); }
    inline void   setInitHeader(size_t markword)  { header.initHeader(markword); }

           void   setObjectType(size_t type);

    inline void   setObjectLive(size_t markword)  { header.setObjectMark(markword); }
    inline void   setHasReferences() { header.setHasReferences(); }
    inline void   setHasNoReferences() { header.setHasNoReferences(); }
    inline void   setReadyForUninit() { header.setReadyForUninit(); }
    inline bool   isReadyForUninit() { return header.isReadyForUninit(); }
    inline bool   hasUninit() { return header.hasUninit(); }
    inline void   setHasUninit() { header.setHasUninit(); }
    inline void   clearHasUninit() { header.clearHasUninit(); }
    inline bool   hasReferences() { return header.hasReferences(); }
    inline bool   hasNoReferences() { return header.hasNoReferences(); }
    inline void   setPrimitive() { header.setPrimitive(); }
    inline void   setNonPrimitive() { header.setNonPrimitive(); }
    inline bool   isPrimitive() { return header.isPrimitive(); }
    inline bool   isNonPrimitive() { return header.isNonPrimitive(); }
    inline bool   isObjectMarked(size_t markword) { return header.isObjectMarked(markword); }
    inline void   setObjectMark(size_t markword) { header.setObjectMark(markword); }
    inline void   clearObjectMark() { header.clearObjectMark(); }
    inline bool   isObjectLive(size_t mark) { return header.isObjectLive(mark); }
    inline bool   isObjectDead(size_t mark) { return header.isObjectDead(mark); }
    inline bool   isOldSpace() { return header.isOldSpace(); }
    inline bool   isNewSpace() { return header.isNewSpace(); }
    inline void   setNewSpace() { header.setNewSpace(); }
    inline void   setOldSpace() { header.setOldSpace(); }
    inline void   makeProxiedObject() { header.makeProxiedObject(); }
    inline bool   requiresProxyObject() { return header.requiresProxyObject(); }
    inline void   makeProxyObject() { header.makeProxyObject(); }
    inline bool   isProxyObject() { return header.isProxyObject(); }
           bool   isSubClassOrEnhanced();
           bool   isBaseClass();
           size_t getObjectTypeNumber();
    inline RexxBehaviour *getObjectType() { return behaviour; }
    inline bool   isObjectType(RexxBehaviour *b) { return b == behaviour; }
    inline bool   isObjectType(size_t t) { return getObjectTypeNumber() == t; }
    inline bool   isSameType(RexxInternalObject *o) { return behaviour == o->getObjectType(); }
    inline void   setBehaviour(RexxBehaviour *b) { behaviour = b; }

    virtual RexxObject  *makeProxy(Envelope *);
    virtual RexxInternalObject *copy();
    virtual RexxObject  *evaluate(RexxActivation *, ExpressionStack *) { return OREF_NULL; }
    virtual RexxObject  *getValue(RexxActivation *) { return OREF_NULL; }
    virtual RexxObject  *getValue(VariableDictionary *) { return OREF_NULL; }
    virtual RexxObject  *getRealValue(RexxActivation *) { return OREF_NULL; }
    virtual RexxObject  *getRealValue(VariableDictionary *) { return OREF_NULL; }
    virtual void         uninit() {;}
    virtual HashCode     hash()  { return getHashValue(); }
    virtual HashCode     getHashValue()  { return identityHash(); }

    // the pointer will be a multiple of our object grain size and have a lot of zero
    // bits at the top end.  Take the complement of all of the bits to get an odd
    // number and more bits to get fewer collisions.
    inline  HashCode     identityHash() { return ((uintptr_t)this) ^ UINTPTR_MAX; }

    virtual bool         truthValue(RexxErrorCodes);
    virtual bool         logicalValue(logical_t &);
    virtual RexxString  *makeString();
    virtual RexxString  *defaultName();
    virtual void         copyIntoTail(CompoundVariableTail *buffer);
    virtual RexxString  *primitiveMakeString();
    virtual ArrayClass  *makeArray();
    virtual RexxString  *stringValue();
    virtual RexxInteger *integerValue(wholenumber_t);
    virtual bool         numberValue(wholenumber_t &result, wholenumber_t precision);
    virtual bool         numberValue(wholenumber_t &result);
    virtual bool         unsignedNumberValue(size_t &result, wholenumber_t precision);
    virtual bool         unsignedNumberValue(size_t &result);
    virtual bool         doubleValue(double &result);
    virtual NumberString *numberString();

    virtual bool         isEqual(RexxInternalObject *);
    virtual bool         isInstanceOf(RexxClass *);
    virtual MethodClass *instanceMethod(RexxString *);
    virtual SupplierClass *instanceMethods(RexxClass *);
    virtual wholenumber_t  compareTo(RexxInternalObject *);

    // compare 2 values for equality, potentially falling back on the
    // "==" method for the test.
    bool inline equalValue(RexxInternalObject *other)
    {
        // test first for direct equality, followed by value equality.
        return (this == other) || this->isEqual(other);
    }

    void         requiresUninit();
    void         removedUninit();
    RexxInternalObject  *clone();

    RexxString  *requiredString(size_t);
    RexxString  *requiredString(const char *);
    RexxString  *requiredString();
    RexxInteger *requiredInteger(size_t, wholenumber_t);
    RexxInteger *requiredInteger(const char *, wholenumber_t);
    wholenumber_t requiredNumber(size_t position, wholenumber_t precision = Numerics::ARGUMENT_DIGITS);
    wholenumber_t requiredNumber(const char *position, wholenumber_t precision = Numerics::ARGUMENT_DIGITS);
    size_t requiredPositive(size_t position, wholenumber_t precision = Numerics::ARGUMENT_DIGITS);
    size_t requiredPositive(const char *position, wholenumber_t precision = Numerics::ARGUMENT_DIGITS);
    size_t requiredNonNegative(size_t position, wholenumber_t precision = Numerics::ARGUMENT_DIGITS);
    size_t requiredNonNegative(const char *position, wholenumber_t precision = Numerics::ARGUMENT_DIGITS);

    RexxString  *requestString();
    RexxString  *requestStringNoNOSTRING();
    RexxInteger *requestInteger(wholenumber_t digits = Numerics::ARGUMENT_DIGITS);
    bool         requestNumber(wholenumber_t &, wholenumber_t);
    bool         requestUnsignedNumber(size_t &, wholenumber_t);
    virtual ArrayClass  *requestArray();

    ObjectHeader header;              // memory management header
    RexxBehaviour *behaviour;         // the object's behaviour
};


/**
 * This is the base class for all objects visible to
 * the Rexx programmer (e.g., String, Array, etc.).  These
 * classes add an objectVariables field to the base layout
 * that will anchor any object variables created by methods.
 */
class RexxObject : public RexxInternalObject
{
 public:
    void * operator new(size_t);
    inline void  operator delete(void *) { ; }

    // Following are used to create new objects.
    // Assumed that the message is sent to a class Object
    // These may move to RexxClass in the future......
    RexxObject *newRexx(RexxObject **arguments, size_t argCount);

    operator RexxInternalObject*() { return (RexxInternalObject *)this; };

    inline RexxObject(){;};
    inline RexxObject(RESTORETYPE restoreType) { ; };


    // The following two methods probably should be on RexxInternalObject, but they
    // need to reference the objectVariables field.  That field could be moved to
    // RexxInternalObject, but it would increase the size of all internal objects
    // by 4 bytes.  Since the minimum object size is large enough to always have
    // that field, it's safe to clear this here.
    inline void initializeNewObject(size_t size, size_t mark, void *vft, RexxBehaviour *b)
    {
        // we need to make this a function object of some type in case
        // a GC cycle gets triggered before this is complete.  By default,
        // we make this a generic object
        setVirtualFunctions(vft);
        setBehaviour(b);
        // this has a clean set of flags, except for the live mark
        header.initHeader(size, mark);
        // make sure the object is cleared in case this gets marked out of any of
        // the constructors.
        clearObject();
    }

    inline void initializeNewObject(size_t mark, void *vft, RexxBehaviour *b)
    {
        // we need to make this a function object of some type in case
        // a GC cycle gets triggered before this is complete.  By default,
        // we make this a generic object
        setVirtualFunctions(vft);
        setBehaviour(b);
        // this has a clean set of flags, except for the live mark
        header.initHeader(mark);
        // make sure the object is cleared in case this gets marked out of any of
        // the constructors.
        clearObject();
    }

    virtual ~RexxObject(){;};

            RexxObject  *defineInstanceMethod(RexxString *, MethodClass *, RexxClass *);
            RexxObject  *deleteInstanceMethod(RexxString *msgname);
            RexxString  *defaultName() override;
    virtual bool         hasMethod(RexxString *msg);
            RexxObject  *hasMethodRexx(RexxString *);
            bool         hasUninitMethod();

    RexxObject *initRexx();

    void uninit() override;
    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    RexxInternalObject *copy() override;
    HashCode     hash() override;
    RexxString  *stringValue() override;

    virtual void processUnknown(RexxErrorCodes, RexxString *, RexxObject **, size_t, ProtectedObject &);

    bool isInstanceOf(RexxClass *) override;
    MethodClass   *instanceMethod(RexxString *) override;
    SupplierClass *instanceMethods(RexxClass *) override;
    RexxObject  *isInstanceOfRexx(RexxClass *);
    RexxObject  *isNilRexx();
    MethodClass   *instanceMethodRexx(RexxString *);
    SupplierClass *instanceMethodsRexx(RexxClass *);
    RexxString  *objectName();
    RexxObject  *objectNameEquals(RexxObject *);
    RexxClass   *classObject();
    RexxObject  *setMethod(RexxString *, MethodClass *, RexxString *a = OREF_NULL);
    RexxObject  *unsetMethod(RexxString *);
    RexxObject  *requestRexx(RexxString *);
    MessageClass *start(RexxObject **, size_t);
    MessageClass *startWith(RexxObject *, ArrayClass *);
    RexxObject  *send(RexxObject **, size_t);
    RexxObject  *sendWith(RexxObject *, ArrayClass *);
    MessageClass *startCommon(RexxObject *message, RexxObject **arguments, size_t argCount);
    static void decodeMessageName(RexxObject *target, RexxObject *message, ProtectedObject &messageName, ProtectedObject &startScope);
    RexxObject  *run(RexxObject **, size_t);
    void         checkUninit();

    RexxObject  *messageSend(RexxString *, RexxObject **, size_t, ProtectedObject &);
    RexxObject  *messageSend(RexxString *, RexxObject **, size_t, RexxClass *, ProtectedObject &);
    MethodClass *checkPrivate(MethodClass *, RexxErrorCodes &);
    MethodClass *checkPackage(MethodClass *, RexxErrorCodes &);
    void         checkRestrictedMethod(const char *methodName);
    void         processProtectedMethod(RexxString *, MethodClass *, RexxObject **, size_t, ProtectedObject &);
    RexxObject* sendMessage(RexxString *, ArrayClass *, ProtectedObject &);
    RexxObject* sendMessage(RexxString *, RexxClass *, ArrayClass *, ProtectedObject &);
    inline RexxObject *sendMessage(RexxString *message, ProtectedObject &result) { return messageSend(message, OREF_NULL, 0, result); };
    inline RexxObject *sendMessage(RexxString *message, RexxObject **args, size_t argCount, ProtectedObject &result) { return messageSend(message, args, argCount, result); };
    inline RexxObject *sendMessage(RexxString *message, RexxObject *argument1, ProtectedObject &result)
        { return messageSend(message, &argument1, 1, result); }
    RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *, ProtectedObject &);
    RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *, ProtectedObject &);
    RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, ProtectedObject &);
    RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, ProtectedObject&);

    void         validateScopeOverride(RexxClass *scope);
    void         validateOverrideContext(RexxObject *target, RexxClass *scope);

                                      // Following are internal OREXX methods
    RexxObject  *defineInstanceMethods(DirectoryClass *);
    void         setObjectVariable(RexxString *, RexxObject *, RexxClass *);
    RexxObject  *getObjectVariable(RexxString *);
    RexxObject  *getObjectVariable(RexxString *, RexxClass *);
    void         addObjectVariables(VariableDictionary *);
    void         copyObjectVariables(RexxObject *newObject);
    RexxClass   *superScope(RexxClass *);
    MethodClass *superMethod(RexxString *, RexxClass *);
    RexxObject  *mdict();
    RexxObject  *setMdict(RexxObject *);
    inline RexxBehaviour *behaviourObject() { return this->behaviour; }

    MethodClass  *methodLookup(RexxString *name );
    VariableDictionary *getObjectVariables(RexxClass *);
    void guardOn(Activity *activity, RexxClass *scope);
    void guardOff(Activity *activity, RexxClass *scope);
    RexxObject  *equal(RexxObject *);
    RexxObject  *notEqual(RexxObject *other);
    RexxObject  *strictEqual(RexxObject *);
    RexxObject  *strictNotEqual(RexxObject *other);

    RexxInteger *identityHashRexx();

    RexxObject  *hashCode();

    RexxString  *stringRexx();
    RexxString  *concatRexx(RexxObject *);
    RexxString  *concatBlank(RexxObject *);
    RexxObject  *makeStringRexx();
    RexxObject  *makeArrayRexx();
    RexxString  *defaultNameRexx();
    RexxObject  *copyRexx();
    void *getCSelf();
    void *getCSelf(RexxClass *scope);
    void *allocateObjectMemory(size_t size);
    void  freeObjectMemory(void *data);
    void *reallocateObjectMemory(void *data, size_t newSize);
    PointerTable *getMemoryTable();

    RexxObject *callOperatorMethod(size_t methodOffset, RexxObject *argument);

 // Define operator methods here.

// used for building short hand operator method definitions.
#define koper(name) RexxObject *name(RexxObject *);

    koper  (operator_plus)
    koper  (operator_minus)
    koper  (operator_multiply)
    koper  (operator_divide)
    koper  (operator_integerDivide)
    koper  (operator_remainder)
    koper  (operator_power)
    koper  (operator_abuttal)
    koper  (operator_concat)
    koper  (operator_concatBlank)
    koper  (operator_equal)
    koper  (operator_notEqual)
    koper  (operator_isGreaterThan)
    koper  (operator_isBackslashGreaterThan)
    koper  (operator_isLessThan)
    koper  (operator_isBackslashLessThan)
    koper  (operator_isGreaterOrEqual)
    koper  (operator_isLessOrEqual)
    koper  (operator_strictEqual)
    koper  (operator_strictNotEqual)
    koper  (operator_strictGreaterThan)
    koper  (operator_strictBackslashGreaterThan)
    koper  (operator_strictLessThan)
    koper  (operator_strictBackslashLessThan)
    koper  (operator_strictGreaterOrEqual)
    koper  (operator_strictLessOrEqual)
    koper  (operator_lessThanGreaterThan)
    koper  (operator_greaterThanLessThan)
    koper  (operator_and)
    koper  (operator_or)
    koper  (operator_xor)
    koper  (operator_not)

    VariableDictionary *objectVariables;   // set of object variables

    static PCPPM operatorMethods[];

    static void createInstance();
    static RexxClass *classInstance;
};


/**
 * Hidden internal class used to create the NIL object.
 */
class RexxNilObject : public RexxObject
{
public:
    void * operator new(size_t);
    inline void   operator delete(void *) { ; }

    RexxNilObject();
    inline RexxNilObject(RESTORETYPE restoreType) { ; };
    virtual ~RexxNilObject() {;};

    HashCode getHashValue() override;

    static RexxObject *nilObject;

protected:

    // we want .NIL to have a static hash value after the image restore, so
    // this needs to be included in the object state
    HashCode hashValue;
};

#endif
