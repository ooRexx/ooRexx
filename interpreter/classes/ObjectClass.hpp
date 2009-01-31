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
/* REXX Kernel                                               ObjectClass.hpp  */
/*                                                                            */
/* Primitive Object Class Definitions                                         */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
#ifndef Included_RexxObject
#define Included_RexxObject

#include "Numerics.hpp"

#include <stddef.h>

  class RexxObject;
  class RexxInteger;
  class RexxBehaviour;
  class RexxCompoundTail;
  class RexxCompoundElement;
  class RexxInternalStack;
  class RexxSupplier;
  class RexxEnvelope;
  class RexxVariableDictionary;
  class RexxNumberString;
  class RexxMethod;
  class RexxMessage;
  class ProtectedObject;
  class SecurityManager;
  class BaseExecutable;


  enum
  {
      LiveMask         =  0xFFFC,    // mask for the checking the mark bits
      MarkMask         =  0x0003,    // mask use for checking the mark bits
      OldSpaceBit      =  0x0010,    // location of the OldSpace bit
  };

typedef size_t HashCode;            // a hash code value

                                       /* used ofor special constructor   */
typedef enum {RESTOREIMAGE, MOBILEUNFLATTEN, METHODUNFLATTEN} RESTORETYPE;


class ObjectHeader
{
public:
    inline ObjectHeader & operator= (ObjectHeader &h)
    {
        // copy the relevant state
        objectSize = h.objectSize;
        flags = h.flags;
        return *this;
    }

    inline size_t getObjectSize() { return (size_t)objectSize; }
    inline void setObjectSize(size_t l) { objectSize = l; }

    inline void makeProxiedObject() { flags |= ProxiedObject; }
    inline bool requiresProxyObject() { return (flags & ProxiedObject) != 0; }
    inline void makeProxy() { flags |= ProxyObject; }
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
        ProxiedObject    =  0x0004,    // This requires a proxy
        ProxyObject      =  0x0008,    // This object is a PROXY(String) Obj
        IsNonPrimitive   =  0x0010,    // use for flattened objects to indicated behaviour status
        NoRefBit         =  0x0020     // location of No References Bit.

    };

    size_t    objectSize;              // allocated size of the object
    union
    {
        uint16_t   flags;              // the object flag/type information
        size_t     sizePadding;        // padding to make sure this is a full pointer size
    };

};


  class RexxVirtualBase {              /* create first base level class     */
                                       /* dummy virtual function to force   */
                                       /* the virtual function table to a   */
                                       /* specific location.  Different     */
                                       /* compilers place the virtual       */
                                       /* function table pointer at         */
                                       /* different locations.  This forces */
                                       /* to the front location             */
  protected:
     virtual ~RexxVirtualBase() { ; }
     virtual void      baseVirtual() {;}
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
 typedef RexxObject *  (RexxObject::*PCPPMA1)(RexxArray *);
 typedef RexxObject *  (RexxObject::*PCPPMC1)(RexxObject **, size_t);

                                       /* pointer to method function        */
 typedef RexxObject *  (RexxObject::*PCPPM)();
 #define CPPM(n) ((PCPPM)&n)


#define OREFSHIFT 3
                                       /* generate hash value from OREF     */
inline uintptr_t HASHOREF(RexxVirtualBase *r) { return ((uintptr_t)r) >> OREFSHIFT; }
                                       /* Base Object REXX class            */
  class RexxInternalObject : public RexxVirtualBase{
    public:

     void * operator new(size_t, RexxClass *);
     void * operator new(size_t, RexxClass *, RexxObject **, size_t);
     inline void *operator new(size_t size, void *ptr) {return ptr;}
     inline void   operator delete(void *) { ; }
     inline void operator delete(void *p, void *ptr) { }
     inline RexxInternalObject() {;};
                                       /* Following constructor used to     */
                                       /*  reconstruct the Virtual          */
                                       /*  Functiosn table.                 */
                                       /* So it doesn't need to do anything */
     inline RexxInternalObject(RESTORETYPE restoreType) { ; };
     virtual ~RexxInternalObject() {;};

     inline operator RexxObject*() { return (RexxObject *)this; };

     inline size_t getObjectSize() { return header.getObjectSize(); }
     inline void   setObjectSize(size_t s) { header.setObjectSize(s); }
     // NB:  I hope this doesn't add any padding
     static inline size_t getObjectHeaderSize() { return sizeof(RexxInternalObject); }
     inline size_t getObjectDataSize() { return getObjectSize() - getObjectHeaderSize(); }
     inline void  *getObjectDataSpace() { return ((char *)this) + getObjectHeaderSize(); }
     // these clear everything after the hash value.
     inline void   clearObject() { memset(getObjectDataSpace(), '\0', getObjectDataSize()); }
     inline void   clearObject(size_t l) { memset(getObjectDataSpace(), '\0', l - getObjectHeaderSize()); }
     inline void   setVirtualFunctions(void *t) { *((void **)this) = t; }

     inline void   setInitHeader(size_t s, size_t markword)  { header.initHeader(s, markword); }
     inline void   setInitHeader(size_t markword)  { header.initHeader(markword); }

     inline void   setObjectLive(size_t markword)  { header.setObjectMark(markword); }
     inline void   setHasReferences() { header.setHasReferences(); }
     inline void   setHasNoReferences() { header.setHasNoReferences(); }
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
     inline bool   isProxyObject() { return header.isProxyObject(); }
            bool   isSubClassOrEnhanced();
            bool   isBaseClass();
            size_t getObjectTypeNumber();
     inline RexxBehaviour *getObjectType() { return behaviour; }
     inline bool   isObjectType(RexxBehaviour *b) { return b == behaviour; }
     inline bool   isObjectType(size_t t) { return getObjectTypeNumber() == t; }
     inline bool   isSameType(RexxInternalObject *o) { return behaviour == o->getObjectType(); }
     inline void   setBehaviour(RexxBehaviour *b) { behaviour = b; }

                                       /* the following are virtual         */
                                       /* functions required for every      */
                                       /* class                             */
     virtual void         live(size_t) {;}
     virtual void         liveGeneral(int reason) {;}
     virtual void         flatten(RexxEnvelope *) {;}
     virtual RexxObject  *unflatten(RexxEnvelope *) { return (RexxObject *)this; };
     virtual RexxObject  *makeProxy(RexxEnvelope *);
     virtual RexxObject  *copy();
     virtual RexxObject  *evaluate(RexxActivation *, RexxExpressionStack *) { return OREF_NULL; }
     virtual RexxObject  *getValue(RexxActivation *) { return OREF_NULL; }
     virtual RexxObject  *getValue(RexxVariableDictionary *) { return OREF_NULL; }
     virtual RexxObject  *getRealValue(RexxActivation *) { return OREF_NULL; }
     virtual RexxObject  *getRealValue(RexxVariableDictionary *) { return OREF_NULL; }
     virtual void         uninit() {;}
     virtual HashCode     hash()  { return getHashValue(); }
     virtual HashCode     getHashValue()  { return identityHash(); }

     inline  HashCode     identityHash() { return HASHOREF(this); }

     virtual bool         truthValue(int);
     virtual bool         logicalValue(logical_t &);
     virtual RexxString  *makeString();
     virtual void         copyIntoTail(RexxCompoundTail *buffer);
     virtual RexxString  *primitiveMakeString();
     virtual RexxArray   *makeArray();
     virtual RexxString  *stringValue();
     virtual RexxInteger *integerValue(size_t);
     virtual bool         numberValue(wholenumber_t &result, size_t precision);
     virtual bool         numberValue(wholenumber_t &result);
     virtual bool         unsignedNumberValue(stringsize_t &result, size_t precision);
     virtual bool         unsignedNumberValue(stringsize_t &result);
     virtual bool         doubleValue(double &result);
     virtual RexxNumberString *numberString();

     virtual bool         isEqual(RexxObject *);
     virtual bool         isInstanceOf(RexxClass *);
     virtual RexxMethod   *instanceMethod(RexxString *);
     virtual RexxSupplier *instanceMethods(RexxClass *);

             void         hasUninit();
             void         removedUninit();
             void         printObject();
             RexxObject  *clone();

     ObjectHeader header;              /* memory management header          */
     RexxBehaviour *behaviour;         /* the object's behaviour            */
  };



class RexxObject : public RexxInternalObject {
  public:
     void * operator new(size_t, RexxClass *);
     void * operator new(size_t, RexxClass *, RexxObject **, size_t);
     void * operator new(size_t size, void *objectPtr) { return objectPtr; };
     inline void  operator delete(void *, void *) {;}
     inline void  operator delete(void *) {;}
     inline void operator delete(void *, RexxClass *) {;}
     inline void operator delete(void *, RexxClass *, RexxObject **, size_t) {;}
                                       // Followin are used to create new objects.
                                       // Assumed that the message is sent to a class Object
                                       // These may move to RexxClass in the future......
     RexxObject *newRexx(RexxObject **arguments, size_t argCount);
     RexxObject *newObject() {return new ((RexxClass *)this) RexxObject; };

     operator RexxInternalObject*() { return (RexxInternalObject *)this; };
     inline RexxObject(){;};
                                       /* Following constructor used to   */
                                       /*  reconstruct the Virtual        */
                                       /*  Functiosn table.               */
                                       /* So it doesn't need to do anythin*/
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

     virtual RexxObject  *defMethod(RexxString *, RexxMethod *, RexxString *a = OREF_NULL);
     virtual RexxString  *defaultName();
     virtual RexxObject  *unknown(RexxString *msg, RexxArray *args){return OREF_NULL;};
     virtual RexxInteger *hasMethod(RexxString *msg);
             bool         hasUninitMethod();

     RexxObject *init();
     void        uninit();
     void live(size_t);
     void liveGeneral(int reason);
     void flatten(RexxEnvelope *);
     RexxObject  *copy();
     HashCode     hash();
     bool         truthValue(int);
     virtual bool logicalValue(logical_t &);
     virtual bool numberValue(wholenumber_t &result, size_t precision);
     virtual bool numberValue(wholenumber_t &result);
     virtual bool unsignedNumberValue(stringsize_t &result, size_t precision);
     virtual bool unsignedNumberValue(stringsize_t &result);
     virtual bool doubleValue(double &result);
     RexxNumberString *numberString();
     RexxInteger *integerValue(size_t);
     RexxString  *makeString();
     RexxString  *primitiveMakeString();
     void         copyIntoTail(RexxCompoundTail *buffer);
     RexxArray   *makeArray();
     RexxString  *stringValue();
     RexxString  *requestString();
     RexxString  *requestStringNoNOSTRING();
     RexxInteger *requestInteger(size_t);
     bool         requestNumber(wholenumber_t &, size_t);
     bool         requestUnsignedNumber(stringsize_t &, size_t);
     RexxArray   *requestArray();
     RexxString  *requiredString(size_t);
     RexxString  *requiredString(const char *);
     RexxString  *requiredString();
     RexxInteger *requiredInteger(size_t, size_t);
     wholenumber_t requiredNumber(size_t position, size_t precision = Numerics::DEFAULT_DIGITS);
     stringsize_t requiredPositive(size_t position, size_t precision = Numerics::DEFAULT_DIGITS);
     stringsize_t requiredNonNegative(size_t position, size_t precision = Numerics::DEFAULT_DIGITS);

     bool         isEqual(RexxObject *);
     bool         isInstanceOf(RexxClass *);
     RexxObject  *isInstanceOfRexx(RexxClass *);
     RexxMethod   *instanceMethod(RexxString *);
     RexxSupplier *instanceMethods(RexxClass *);
     RexxMethod   *instanceMethodRexx(RexxString *);
     RexxSupplier *instanceMethodsRexx(RexxClass *);
     RexxString  *objectName();
     RexxObject  *objectNameEquals(RexxObject *);
     RexxClass   *classObject();
     RexxObject  *setMethod(RexxString *, RexxMethod *, RexxString *a = OREF_NULL);
     RexxObject  *unsetMethod(RexxString *);
     RexxObject  *requestRexx(RexxString *);
     RexxMessage *start(RexxObject **, size_t);
     RexxMessage *startWith(RexxObject *, RexxArray *);
     RexxObject  *send(RexxObject **, size_t);
     RexxObject  *sendWith(RexxObject *, RexxArray *);
     RexxMessage *startCommon(RexxObject *message, RexxObject **arguments, size_t argCount);
     static void decodeMessageName(RexxObject *target, RexxObject *message, RexxString *&messageName, RexxObject *&startScope);
     RexxString  *oref();
     RexxObject  *pmdict();
     RexxObject  *run(RexxObject **, size_t);

     void         messageSend(RexxString *, RexxObject **, size_t, ProtectedObject &);
     void         messageSend(RexxString *, RexxObject **, size_t, RexxObject *, ProtectedObject &);
     RexxMethod  *checkPrivate(RexxMethod *);
     void         processUnknown(RexxString *, RexxObject **, size_t, ProtectedObject &);
     void         processProtectedMethod(RexxString *, RexxMethod *, RexxObject **, size_t, ProtectedObject &);
     void         sendMessage(RexxString *, RexxArray *, ProtectedObject &);
     inline void  sendMessage(RexxString *message, ProtectedObject &result) { this->messageSend(message, OREF_NULL, 0, result); };
     inline void  sendMessage(RexxString *message, RexxObject **args, size_t argCount, ProtectedObject &result) { this->messageSend(message, args, argCount, result); };
     inline void  sendMessage(RexxString *message, RexxObject *argument1, ProtectedObject &result)
         { this->messageSend(message, &argument1, 1, result); }
     void         sendMessage(RexxString *, RexxObject *, RexxObject *, ProtectedObject &);
     void         sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *, ProtectedObject &);
     void         sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, ProtectedObject &);
     void         sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, ProtectedObject&);

     RexxObject  *sendMessage(RexxString *, RexxArray *);
     RexxObject  *sendMessage(RexxString *message);
     RexxObject  *sendMessage(RexxString *message, RexxObject **args, size_t argCount);
     RexxObject  *sendMessage(RexxString *message, RexxObject *argument1);
     RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *);
     RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *);
     RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);
     RexxObject  *sendMessage(RexxString *, RexxObject *, RexxObject *, RexxObject *, RexxObject *, RexxObject *);

                                       // Following are internal OREXX methods
     RexxObject  *defMethods(RexxDirectory *);
     void         setObjectVariable(RexxString *, RexxObject *, RexxObject *);
     RexxObject  *getObjectVariable(RexxString *);
     RexxObject  *getObjectVariable(RexxString *, RexxObject *);
     void         addObjectVariables(RexxVariableDictionary *);
     void         copyObjectVariables(RexxObject *newObject);
     RexxObject  *superScope(RexxObject *);
     RexxMethod  *superMethod(RexxString *, RexxObject *);
     RexxObject  *mdict();
     RexxObject  *setMdict(RexxObject *);
     inline RexxBehaviour *behaviourObject() { return this->behaviour; }

     const char  *idString();
     RexxString  *id();
     RexxMethod  *methodLookup(RexxString *name );
     RexxVariableDictionary *getObjectVariables(RexxObject *);
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
     RexxObject  *unknownRexx(RexxString *, RexxArray *);
     RexxObject  *hasMethodRexx(RexxString *);
     void *getCSelf();
     // compare 2 values for equality, potentially falling back on the
     // "==" method for the test.
     bool inline equalValue(RexxObject *other)
     {
         // test first for direct equality, followed by value equality.
         return (this == other) || this->isEqual(other);
     }
     virtual wholenumber_t compareTo(RexxObject *);

 // Define operator methods here.

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

   RexxVariableDictionary *objectVariables;   /* set of object variables           */
   static PCPPM operatorMethods[];

   static void createInstance();
   static RexxClass *classInstance;
};




class RexxNilObject : public RexxObject {
public:
    void * operator new(size_t);
    void * operator new(size_t size, void *objectPtr) { return objectPtr; };
    inline void   operator delete(void *) { ; }
    inline void   operator delete(void *, void *) { ; }
    RexxNilObject();
    inline RexxNilObject(RESTORETYPE restoreType) { ; };
    virtual ~RexxNilObject() {;};

    virtual HashCode getHashValue();

    static RexxObject *nilObject;

protected:
    // we want .NIL to have a static hash value after the image restore, so
    // this needs to be included in the object state
    HashCode hashValue;
};

class RexxList;


class RexxActivationBase : public RexxInternalObject{
public:
    inline RexxActivationBase() {;};
    inline RexxActivationBase(RESTORETYPE restoreType) { ; };
    virtual RexxObject  *dispatch() {return NULL;};
    virtual void traceBack(RexxList *) {;};
    virtual size_t digits() {return Numerics::DEFAULT_DIGITS;};
    virtual size_t fuzz() {return Numerics::DEFAULT_FUZZ;};
    virtual bool form() {return Numerics::DEFAULT_FORM;};
    virtual NumericSettings *getNumericSettings() { return Numerics::getDefaultSettings(); }
    virtual RexxActivation *getRexxContext() { return OREF_NULL; }
    virtual RexxActivation *findRexxContext() { return OREF_NULL; }
    virtual void setDigits(size_t) {;};
    virtual void setFuzz(size_t) {;};
    virtual void setForm(bool) {;}
    virtual bool trap(RexxString *, RexxDirectory *) {return false;};
    virtual void setObjNotify(RexxMessage *) {;};
    virtual void termination(){;};
    virtual SecurityManager *getSecurityManager() = 0;
    virtual bool isForwarded() { return false; }
    virtual bool isStackBase() { return false; }
    virtual bool isRexxContext() { return false; }
    virtual RexxObject *getReceiver() { return OREF_NULL; }
    inline void setPreviousStackFrame(RexxActivationBase *p) { previous = p; }
    inline RexxActivationBase *getPreviousStackFrame() { return previous; }
    inline BaseExecutable *getExecutable() { return executable; }

protected:
    RexxActivationBase *previous;
    BaseExecutable     *executable;

};
#endif
