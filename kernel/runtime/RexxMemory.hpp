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
/* REXX Kernel                                                  RexxMemory.hpp  */
/*                                                                            */
/* Primitive Memory Class Definitions                                         */
/*                                                                            */
/******************************************************************************/

#ifndef Included_RexxMemory
#define Included_RexxMemory

void memoryCreate();
void memoryRestore(void);
void memoryNewProcess (void);

/* turn this on to get verbose GC output */
//#define VERBOSE_GC


#define LiveMask            0xFFFFFFFC
#define MarkMask            0x00000003

/* behaviour (used on restoreimage).  Not that this is overloaded */
/* with the mark bits.  We don't need to use these at the same */
/* time.  The IsNonPrimitive bit on has meaning in the saved image. */
/* The mark bits only have meaning once the image has been */
/* restored. */
#define IsNonPrimitive  0x00000001
#define MarkBit1        0x00000001     /* Second of the mark bits           */
#define MarkBit2        0x00000002     /* Second of the mark bits           */
#define MakeProxyObject 0x00000004     /* This object is a PROXY(String) Obj*/
#define ProxyObject     0x00000008     /* This object is a PROXY(String) Obj*/
#define OldSpaceBit     0x00000010     /* location of the OldSpace bit      */
#define LargeObjectBit  0x00000020     /* This is a Large Object            */
#define NoRefBit        0x00000040     /* location of No References Bit.    */
#define Special         0x00000080     /* One bit available class specific usage */
#define SizeMask        0xFFFFFF00     /* mask for object size              */
#define TenureMask      0xFFFFFF00     /* mask for tenure....               */

/* The minimum allocation unit for an object.   */
#define ObjectGrain 8
/* The unit of granularity for large allocation */
#define LargeAllocationUnit 1024
/* The unit of granularity for extremely large objects */
#define VeryLargeAllocationUnit 4096
/* Minimum size of an object.  This is not the actual minimum size, */
/* but we allocate objects with an 8-byte granularity */
/* this is the granularity for objects greater than 16Mb. */
#define VeryLargeObjectGrain    256

/* This is the smallest object we'll allocate from storage.  Our */
/* smallest possible object is smaller than this, but we get better */
/* usage by allocating with a larger grain size. */
#define MinimumObjectSize 24l
/* The older minimum object size.  Tokenized images will contain */
/* objects of this size, so we need to accomadate them. */
#define OldMinimumObjectSize 20l
#define ObjectHeaderSize 16l
#define MaximumObjectSize ((size_t)0xfffffff0)

/* bits to shift out for object size */
#define ObjectSizeShift 8
/* Or lower 8 bits for large obj size*/
#define LargeObjectSizeMask 0xFFFFFF00
/* Minimum size of a large object    */
#define LargeObjectMinSize  0x01000000

#define ObjectIsLive(o)     (memoryObject.objectIsLive((RexxObject *)(o)))
#define ObjectIsNotLive(o)  (!ObjectIsLive(o))
#define SetObjectLive(o)    (ObjectHeader(o) &= LiveMask); \
                            (ObjectHeader(o) |= memoryObject.markWord)
#define ObjectIsMarked(o)   (ObjectHeader(o) & headerMarkedValue)
#define ClearObjectMark(o)  (ObjectHeader(o) &= LiveMask)

#ifdef FORCE_GRAINING
#define IsObjectGrained(o)  ((((size_t)o)%ObjectGrain) == 0)
#define IsValidSize(s) ((s) >= MinimumObjectSize && ((s) % ObjectGrain) == 0)
#else
#define IsObjectGrained(o)  ((((size_t)o)%sizeof(void *)) == 0)
#define IsValidSize(s) ((s) >= OldMinimumObjectSize && ((s) % sizeof(void *)) == 0)
#endif

/* Following Defines HEADINFO constants */

/* NOTE: The following are used in places other than OKMEMORY, such */
/* as OKTRACE/OKBEHAV/OKOBJECT use the ObjectSize/SetObjectSize */
/* macros, but OKGDATA uses ObjectSizeShift directly.  If we change */
/* defaults in object headers, be sure to check the above modules, */
/* for possible changes.... */

/* return the size of the object */
#define ObjectSize(o)  ObjectSizeFunc(((RexxObject *)(o))->header)
inline size_t ObjectSizeFunc(HEADINFO header) { return (header & LargeObjectBit) ? header & LargeObjectSizeMask : header>>ObjectSizeShift; }

/* largest size we'll keep in the save stack on a mark */
#define SaveStackThreshold   4096

#define IsLargeObject(o)  (ObjectSize(o) > SaveStackThreshold)
#define ObjectHeader(o)   (((RexxObject *)(o))->header)
#define ClearObjectHeader(o) (ObjectHeader(o) = (HEADINFO)0)
#define SetOldSpace(o)       (ObjectHeader(o) |= OldSpaceBit)

#define SetSpecial(o)       (ObjectHeader(o) |= Special)
#define ClearSpecial(o)     (ObjectHeader(o) &= (~Special))
#define IsSpecial(o)        (ObjectHeader(o) & Special)

#define ClearObjectSize(o) (ObjectHeader(o) &= (~(SizeMask | LargeObjectBit)))

#define SmallObjectSize(s) (((size_t)(s)) << ObjectSizeShift)
#define LargeObjectSize(s) (((size_t)(s) & LargeObjectSizeMask) | LargeObjectBit)

#define SetSmallObjectSize(o,s)     (ObjectHeader(o) |= SmallObjectSize(s))
#define SetLargeObjectSize(o,s)     (ObjectHeader(o) |= LargeObjectSize(s))
/* set the size of the object in header */
#define SetObjectSize(o, s) SetObjectSizeFunc((RexxObject *)(o), s)

inline void SetObjectSizeFunc(RexxObject *o, size_t s)
{
#ifdef CHECKOREFS
    if (!IsValidSize(s)) {
        logic_error("Invalid object size");
    }
#endif

    ClearObjectSize(o);
    if (s >= LargeObjectMinSize)
        SetLargeObjectSize(o, s);
    else
        SetSmallObjectSize(o, s);
}

#define SetNewSpace(o)               (ObjectHeader(o) &= (~OldSpaceBit))
#define SetOldSpace(o)               (ObjectHeader(o) |= OldSpaceBit)

#define SetObjectHasNoReferences(o)  (ObjectHeader(o) |= NoRefBit)
#define SetObjectHasReferences(o)    (ObjectHeader(o) &= (~NoRefBit))
#define ObjectHasNoReferences(o)     (ObjectHeader(o) & NoRefBit)
#define ObjectHasReferences(o)       (!ObjectHasNoReferences(o))
#define OldSpace(o)                  (ObjectHeader(o) & OldSpaceBit)

#define NullOrOld(o)        (((RexxObject *)(o) == OREF_NULL) || OldSpace(o))

#define ObjectIsNonPrimitive(o) (ObjectHeader(o) & IsNonPrimitive)
#define SetNonPrimitive(o)  (ObjectHeader(o) |= IsNonPrimitive)
#define ClearNonPrimitive(o)  (ObjectHeader(o) &= ~IsNonPrimitive)

#define SetUpNewObject(o,s) {                         \
    memoryObject.setObjectSize((RexxObject *)(o), s); \
    setObjectVirtualFunctions(o);                     \
    (o)->objectVariables = NULL;                      \
    BehaviourSet(o, TheObjectBehaviour);              \
}

#define SetUpNewAllocation(o) {                       \
    setObjectVirtualFunctions(o);                     \
    (o)->objectVariables = NULL;                      \
    BehaviourSet(o, TheObjectBehaviour);              \
    SetObjectLive(o);                                 \
}

inline size_t roundObjectBoundary(size_t n) { return RXROUNDUP(n,ObjectGrain); }
// inline size_t roundObjectBoundary(size_t n) { return (n + 7) & 0xFFFFFFF8; }
inline size_t roundLargeObjectAllocation(size_t n) { return n > LargeObjectMinSize ? RXROUNDUP(n, VeryLargeAllocationUnit) : RXROUNDUP(n, LargeAllocationUnit); }
inline size_t roundObjectResize(size_t n) { return n > LargeObjectMinSize ? RXROUNDUP(n, VeryLargeObjectGrain) : RXROUNDUP(n, ObjectGrain); }

#define ObjectDataSize(o) (ObjectSize(o) - ObjectHeaderSize)
#define ObjectDataSpace(o) (&((RexxObject *)(o))->objectVariables)

/* Zero out the state data of an object */
#define ClearObject(o)          memset((void *)ObjectDataSpace(o),'\0', ObjectDataSize(o));
#define ClearObjectLength(o,l)  memset((void *)ObjectDataSpace(o),'\0', l - ObjectHeaderSize);

/* restore/set the VirtualFunction table for the specified Object */
#define setVirtualFunctions(o,t)  (*((void **)o) = VFTArray[t])
#define setObjectVirtualFunctions(o)  (*((void **)o) = VFTArray[T_object])

class RexxActivationFrameBuffer;
class MemorySegment;
#ifdef _DEBUG
class RexxMemory;
#endif
                                       /* This class is implemented in      */
                                       /*OS2MEM.C, since the function is    */
                                       /*system dependant.                  */
typedef char MEMORY_POOL_STATE;

class MemorySegmentPoolHeader {
#ifdef _DEBUG
 friend class RexxMemory;
#endif

 protected:
   MemorySegmentPool *next;
   MemorySegment     *spareSegment;
   char  *nextAlloc;
   char  *nextLargeAlloc;
   size_t uncommitted;
   size_t reserved;            // force aligment of the state data....
};

class MemorySegmentPool : public MemorySegmentPoolHeader {
#ifdef _DEBUG
 friend class RexxMemory;
#endif
 friend BOOL SysAccessPool(MemorySegmentPool **);
 public:
   void          *operator new(size_t size, size_t minSize);
   void          *operator new(size_t size, void *pool) { return pool;}
   MemorySegmentPool();
   MemorySegment *newSegment(size_t minSize);
   MemorySegment *newLargeSegment(size_t minSize);
   BOOL           accessNextPool(void);
   MemorySegmentPool *freePool(void); /* CHM - defect 96: add return value */
   MemorySegmentPool *nextPool() {return this->next;}
   void               setNext( MemorySegmentPool *nextPool ); /* CHM - def.96: new function */

 private:
   char           state[8];    // must be at the end of the structure.
};

#include "MemoryStats.hpp"
#include "MemorySegment.hpp"

class RexxMemory : public RexxObject {
#ifdef _DEBUG
  friend class RexxInstructionOptions;
#endif
 public:
  inline RexxMemory();
  inline RexxMemory(RESTORETYPE restoreType) { ; };

  inline void *operator new(size_t size, void *ptr) {return ptr; };

  inline operator RexxObject*() { return (RexxObject *)this; };
  inline RexxObject *operator=(DeadObject *d) { return (RexxObject *)this; };

  void live();
  void liveGeneral();
  void flatten(RexxEnvelope *);
  RexxObject  *makeProxy(RexxEnvelope *);

  void        init(BOOL savingImage, BOOL restoringImage);
  MemorySegment *newSegment(size_t requestLength, size_t minLength);
  MemorySegment *newLargeSegment(size_t requestLength, size_t minLength);
  RexxObject *oldObject(size_t size);
  RexxObject *newObject(size_t size);
  RexxObject *temporaryObject(size_t size);
  RexxArray  *newObjects(size_t size, size_t count, RexxBehaviour *behaviour);
  RexxObject *clone(RexxObject *objr);
  void        reSize(RexxObject *, size_t);
  void        checkUninit(RexxTable *);
  void        checkSubClasses(RexxObjectTable *);
  void        markObjects(void);
  void        markObjectsMain(RexxObject *);
  void        killOrphans(RexxObject *);
  void        mark(RexxObject *);
  void        markGeneral(RexxObject **);
  void        collect();
  inline RexxObject *saveObject(RexxInternalObject *saveObj) {this->saveTable->add((RexxObject *)saveObj, (RexxObject *)saveObj); return (RexxObject *)saveObj;}
  inline void        discardObject(RexxInternalObject *obj) {this->saveTable->remove((RexxObject *)obj);};
  inline void        removeHold(RexxInternalObject *obj) { this->saveStack->remove((RexxObject *)obj); }
  void        discardHoldObject(RexxInternalObject *obj);
  RexxObject *holdObject(RexxInternalObject *obj);
  void        saveImage();
  BOOL        savingImage() { return saveimage; }
  BOOL        restoringImage() { return restoreimage; }
  RexxObject *setDump(RexxObject *);
  inline BOOL queryDump() {return this->dumpEnable;};
  RexxObject *dump();
  void        dumpObject(RexxObject *objectRef, FILE *outfile);
  void        setObjectOffset(size_t offset);
  void        setEnvelope(RexxEnvelope *);
  inline void        setMarkTable(RexxTable *marktable) {this->markTable = marktable;};
  inline void        setOrphanCheck(BOOL orphancheck) {this->orphanCheck = orphancheck; };
  RexxObject *checkSetOref(RexxObject *, RexxObject **, RexxObject *, char *, long);
  RexxObject *setOref(RexxObject **index, RexxObject *value);
  RexxStack  *getFlattenStack();
  void        returnFlattenStack();
  RexxObject *reclaim();
  RexxObject *setParms(RexxObject *, RexxObject *);
  RexxObject *gutCheck();
  void        accessPools();
  void        accessPools(MemorySegmentPool *);
  void        freePools();
  MemorySegmentPool *freePools(MemorySegmentPool *);
  void        liveStackFull();
  BOOL        extendSaveStack(size_t inc);
  void        dumpMemoryProfile();
  char *      allocateImageBuffer(size_t size);
  void        logVerboseOutput(char *message, void *sub1, void *sub2);
  inline void verboseMessage(char *message) {
#ifdef VERBOSE_GC
      logVerboseOutput(message, NULL, NULL);
#endif
  }

  inline void verboseMessage(char *message, size_t sub1) {
#ifdef VERBOSE_GC
      logVerboseOutput(message, (void *)sub1, NULL);
#endif
  }

  inline void verboseMessage(char *message, size_t sub1, size_t sub2) {
#ifdef VERBOSE_GC
      logVerboseOutput(message, (void *)sub1, (void *)sub2);
#endif
  }

  inline void logObjectStats(RexxObject *obj) { imageStats->logObject(obj); }
  inline void pushSaveStack(RexxObject *obj) { saveStack->push(obj); }
  inline void removeSavedObject(RexxObject *obj) { saveStack->remove(obj); }
  inline void setObjectSize(RexxObject *o, size_t s)
  {
      if (s <  LargeObjectMinSize) {
          ObjectHeader(o) = SmallObjectSize(s) | markWord;
      }
      else {                                                                                                  \
          ObjectHeader(o) = LargeObjectSize(s) | markWord;
      }
  }

  inline BOOL objectIsLive(RexxObject *obj) {return ((ObjectHeader(obj) & MarkMask) == markWord); }
  inline BOOL objectIsNotLive(RexxObject *obj) {return ((ObjectHeader(obj) & MarkMask) != markWord); }
  inline void disableOrefChecks() { checkSetOK = FALSE; }
  inline void enableOrefChecks() { checkSetOK = TRUE; }
  inline void clearSaveStack() {
                                       /* remove all objects from the save- */
                                       /* stack. to be really oo, this      */
                                       /* should be done in RexxSaveStack,  */
                                       /* but we do it here for speed...    */
    memset(saveStack->stack, 0, sizeof(RexxObject*) * saveStack->u_size);
  }
/* Start of methods to build some specific REXX objects */
  RexxTable       *newHashCollection(size_t, size_t);
  RexxHashTable   *newHashTable(size_t);
  RexxTable       *newTable();
  RexxObjectTable *newObjectTable(size_t size);
  RexxDirectory   *newDirectory();
  RexxRelation    *newRelation();
  RexxVariableDictionary *newVariableDictionary(size_t s);
  RexxVariableDictionary *newVariableDictionary(RexxObject *scope);
  RexxCompoundElement *newCompoundElement(RexxString *name);
  RexxVariable    *newVariable(RexxString *name);

  RexxInternalStack *newInternalStack(size_t);
  RexxActivationFrameBuffer *newActivationFrameBuffer(size_t);

  void        checkAllocs();
  RexxObject *dumpImageStats();
  void        createLocks();
  void        scavengeSegmentSets(MemorySegmentSet *requester, size_t allocationLength);
  void setUpMemoryTables(RexxObjectTable *old2newTable);

  ULONG markWord;                      /* current marking counter           */
  SMTX flattenMutex;                   /* locks for various memory processes */
  SMTX unflattenMutex;
  SMTX envelopeMutex;
  RexxVariable *variableCache;         /* our cache of variable objects     */

private:
  inline void checkLiveStack() { if (!liveStack->checkRoom()) liveStackFull(); }
  inline void pushLiveStack(RexxObject *obj) { checkLiveStack(); liveStack->fastPush(obj); }
  inline RexxObject * popLiveStack() { return (RexxObject *)liveStack->fastPop(); }
  inline void bumpMarkWord() { markWord ^= MarkMask; }
  inline void restoreMark(RexxObject *markObject, RexxObject **pMarkObject) {
                                       /* we update the object's location   */
      *pMarkObject = (RexxObject *)((size_t)markObject + relocation);
  }

  inline void unflattenMark(RexxObject *markObject, RexxObject **pMarkObject) {
                                       /* do the unflatten                  */
      *pMarkObject = markObject->unflatten(this->envelope);
  }

  inline void restoreObjectMark(RexxObject *markObject, RexxObject **pMarkObject) {
                                         /* update the object reference       */
      markObject = (RexxObject *)((ULONG)markObject + objOffset);
      SetObjectLive(markObject);         /* Then Mark this object as live.    */
      *pMarkObject = markObject;         /* now set this back again           */
  }


/* object validation method --used to find and diagnose broken object references       */
  void saveImageMark(RexxObject *markObject, RexxObject **pMarkObject);
  void orphanCheckMark(RexxObject *markObject, RexxObject **pMarkObject);

  BOOL inObjectStorage(RexxObject *obj);
  BOOL inSharedObjectStorage(RexxObject *obj);
  BOOL objectReferenceOK(RexxObject *o);
  void restoreImage(void);

  RexxStack  *liveStack;
  RexxStack  *flattenStack;
  RexxSaveStack      *saveStack;
  RexxObjectTable  *saveTable;
  RexxTable  *markTable;               /* tabobjects to start a memory mark */
                                       /*  if building/restoring image,     */
                                       /*OREF_ENV, else old2new             */
  RexxObjectTable  *old2new;           /* remd set                          */
  MemorySegmentPool *firstPool;        /* First segmentPool block.          */
  MemorySegmentPool *currentPool;      /* Curent segmentPool being carved   */
  OldSpaceSegmentSet oldSpaceSegments;
  NormalSegmentSet newSpaceNormalSegments;
  LargeSegmentSet  newSpaceLargeSegments;
  char *image_buffer;                  /* the buffer used for image save/restore operations */
  size_t image_offset;                 /* the offset information for the image */
  size_t relocation;                   /* image save/restore relocation factor */
  BOOL dumpEnable;                     /* enabled for dumps?                */
  BOOL saveimage;                      /* we're saving the image */
  BOOL restoreimage;                   /* we're restoring the image */
  BOOL checkSetOK;                     /* OREF checking is enabled          */
                                       /* enabled for checking for bad      */
                                       /*OREF's?                            */
  BOOL orphanCheck;
  size_t objOffset;                    /* offset of arriving mobile objects */
                                       /* envelope for arriving mobile      */
                                       /*objects                            */
  RexxEnvelope *envelope;
  RexxStack *originalLiveStack;        /* original live stack allocation    */
  MemoryStats *imageStats;             /* current statistics collector      */

  size_t allocations;                  /* number of allocations since last GC */
  size_t collections;                  /* number of garbage collections     */
};

#endif
