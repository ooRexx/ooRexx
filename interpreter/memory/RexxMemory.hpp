/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/*                                                   RexxMemory.hpp           */
/*                                                                            */
/* Interpreter memory management subsystem                                    */
/*                                                                            */
/******************************************************************************/

#ifndef Included_MemoryObject
#define Included_MemoryObject

#include "Memory.hpp"
#include "MemoryStack.hpp"
#include "SysSemaphore.hpp"
#include "IdentityTableClass.hpp"
#include "QueueClass.hpp"

#include <vector>

// this can be enabled to switch on memory profiling info
//#define MEMPROFILE

// this can be enabled to have major GC events chronicled.
//#define VERBOSE_GC

class ActivationFrameBuffer;
class MemorySegment;
class MethodClass;
class RexxVariable;
class WeakReference;
class IdentityTable;
class GlobalProtectedObject;
class MapTable;
class BufferClass;
class StringTable;
class FileNameBuffer;

#ifdef _DEBUG
class MemoryObject;
#endif

#include "MemoryStats.hpp"
#include "MemorySegment.hpp"


/**
 * A base class for handling general mark operations.  The
 * called mark is forwarded to a current marking object that
 * then performs the needed marking operation.
 */
class MarkHandler
{
 public:
    // pure virtual method for handling the mark operation.
    virtual void mark(RexxInternalObject **field, RexxInternalObject *object);
};


class MemoryObject : public RexxInternalObject
{
 public:
    inline MemoryObject();
    inline MemoryObject(RESTORETYPE restoreType) { ; };

    inline operator RexxObject*() { return (RexxObject *)this; };
    inline RexxObject *operator=(DeadObject *d) { return (RexxObject *)this; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;

    void        initialize(bool restoringImage, const char *imageTarget);
    MemorySegment *newSegment(size_t requestLength);
    MemorySegment *newSegment(size_t requestLength, size_t minLength);
    MemorySegment *newLargeSegment(size_t requestLength, size_t minLength);
    void freeSegment(MemorySegment *segment);
    RexxInternalObject *oldObject(size_t size);
    inline RexxInternalObject *newObject(size_t size) { return newObject(size, T_Object); }
    RexxInternalObject *newObject(size_t size, size_t type);
    void *temporaryObject(size_t size);
    void  deleteTemporaryObject(void *obj);
    ArrayClass *newObjects(size_t size, size_t count, size_t objectType);
    void        reSize(RexxInternalObject *, size_t);
    void        checkUninit();
    void        runUninits();
    void        removeUninitObject(RexxInternalObject *obj);
    void        addUninitObject(RexxInternalObject *obj);
    inline void checkUninitQueue() { if (pendingUninits > 0) runUninits(); }
    RexxInternalObject *unflattenObjectBuffer(Envelope *envelope, BufferClass *sourceBuffer, char *startPointer, size_t dataLength);
    void        unflattenProxyObjects(Envelope *envelope, RexxInternalObject *firstObject, RexxInternalObject *endObject);

    void        markObjects();
    void        markObjectsMain(RexxInternalObject *);
    void        mark(RexxInternalObject *);
    void        markGeneral(void *);
    void        tracingMark(RexxInternalObject *root, MarkReason reason);
    void        collect();
    inline void removeHold(RexxInternalObject *obj) { saveStack->remove(obj); }
    RexxInternalObject *holdObject(RexxInternalObject *obj);
    void        saveImage(const char *imageTarget);
    void        setOref(RexxInternalObject *variable, RexxInternalObject *value);
    void        shutdown();
    void        liveStackFull();
    void        liveStackFull(size_t needed);
    char *      allocateImageBuffer(size_t size);
    void        logVerboseOutput(const char *message, void *sub1, void *sub2, void*sub3);

    inline void verboseMessage(const char *message)
    {
  #ifdef VERBOSE_GC
        logVerboseOutput(message, NULL, NULL, NULL);
  #endif
    }

    inline void verboseMessage(const char *message, size_t sub1) {
  #ifdef VERBOSE_GC
        logVerboseOutput(message, (void *)sub1, NULL, NULL);
  #endif
    }

    inline void verboseMessage(const char *message, size_t sub1, size_t sub2) {
  #ifdef VERBOSE_GC
        logVerboseOutput(message, (void *)sub1, (void *)sub2, NULL);
  #endif
    }

    inline void verboseMessage(const char *message, const char *sub1, size_t sub2) {
  #ifdef VERBOSE_GC
        logVerboseOutput(message, (void *)sub1, (void *)sub2, NULL);
  #endif
    }

    inline void verboseMessage(const char *message, size_t sub1, const char *sub2) {
  #ifdef VERBOSE_GC
        logVerboseOutput(message, (void *)sub1, (void *)sub2, NULL);
  #endif
    }

    inline void verboseMessage(const char *message, size_t sub1, size_t sub2, size_t sub3) {
  #ifdef VERBOSE_GC
        logVerboseOutput(message, (void *)sub1, (void *)sub2, (void *)sub3);
  #endif
    }

    inline void verboseMessage(const char *message, size_t sub1, const char *sub2, const char *sub3) {
  #ifdef VERBOSE_GC
        logVerboseOutput(message, (void *)sub1, (void *)sub2, (void *)sub3);
  #endif
    }

    inline void logObjectStats(RexxInternalObject *obj) { imageStats->logObject(obj); }
    inline void pushSaveStack(RexxInternalObject *obj) { saveStack->push(obj); }
    inline void removeSavedObject(RexxInternalObject *obj) { saveStack->remove(obj); }
    inline void clearSaveStack() { saveStack->clear(); }

    void        checkAllocs();
    void        dumpImageStats();
    void        scavengeSegmentSets(MemorySegmentSet *requester, size_t allocationLength);
    void        transferSegmentToNormalSet(MemorySegment *segment);
    void        setUpMemoryTables(MapTable *old2newTable);
    void        collectAndUninit(bool clearStack);
    void        lastChanceUninit();
    inline StringTable *getGlobalStrings() { return globalStrings; }
    void        addWeakReference(WeakReference *ref);
    void        checkWeakReferences();
    void        addGlobalReference(RexxInternalObject *obj);
    void        removeGlobalReference(RexxInternalObject *obj);

    void restore();
    void buildVirtualFunctionTable();
    void create();
    void createImage(const char *imageTarget);
    RexxString *getGlobalName(const char *value);
    RexxString *getUpperGlobalName(const char *value);
    void createStrings();
    ArrayClass *saveStrings();
    void restoreStrings(ArrayClass *stringArray);

    inline void checkLiveStack() { if (!liveStack->checkRoom()) liveStackFull(); }
    inline void checkLiveStack(size_t needed) { if (!liveStack->checkRoom(needed)) liveStackFull(needed); }
    inline void pushLiveStack(RexxInternalObject *obj) { checkLiveStack(); liveStack->push(obj); }
    inline RexxInternalObject * popLiveStack() { return liveStack->pop(); }
    inline void bumpMarkWord() { markWord ^= ObjectHeader::MarkMask; }

    // set the live mark in an object referenced by a void pointer
    static inline void setObjectLive(void *o, size_t mark)
    {
        ((RexxInternalObject *)o)->setObjectLive(mark);
    }

    static void *virtualFunctionTable[];     // table of virtual functions
    static PCPPM exportedMethods[];          // start of exported methods table

    size_t markWord;                         // current marking counter
    GlobalProtectedObject *protectedObjects; // specially protected objects

    DirectoryClass *environment;             // global environment
    StringTable    *commonRetrievers;        // statically defined requires

    StringTable    *system;                  // the system directory...anchors stuff we don't want to expose in environment
    PackageClass   *rexxPackage;             // the main rexx package

private:


    /******************************************************************************/
    /* Define location of objects saved in SaveArray during Saveimage processing  */
    /*  and used during restart processing.                                       */
    /* Currently only used in MemoryObject                                        */
    /******************************************************************************/
    enum
    {
        saveArray_ENV = 1,
        saveArray_SYSTEM,
        saveArray_NAME_STRINGS,
        saveArray_TRUE,
        saveArray_FALSE,
        saveArray_NIL,
        saveArray_GLOBAL_STRINGS,
        saveArray_CLASS,
        saveArray_PBEHAV,
        saveArray_PACKAGES,
        saveArray_NULLA,
        saveArray_NULLPOINTER,
        saveArray_REXX_PACKAGE,
        saveArray_COMMON_RETRIEVERS,
        saveArray_highest = saveArray_COMMON_RETRIEVERS
    };



    void restoreImage();
    void loadImage(char *&imageBuffer, size_t &imageSize);
    bool loadImage(char *&imageBuffer, size_t &imageSize, FileNameBuffer &imageFile);

    void setMarkHandler(MarkHandler *h) { currentMarkHandler = h; }
    void resetMarkHandler() { currentMarkHandler = &defaultMarkHandler; }

    void defineMethod(const char *name, RexxBehaviour * behaviour, PCPPM entryPoint, size_t arguments, const char *entryPointName);
    void defineProtectedMethod(const char *name, RexxBehaviour * behaviour, PCPPM entryPoint, size_t arguments, const char *entryPointName);
    void definePrivateMethod(const char *name, RexxBehaviour *behaviour, PCPPM entryPoint, size_t arguments, const char *entryPointName);
    void defineUnguardedMethod(const char *name, RexxBehaviour *behaviour, PCPPM entryPoint, size_t arguments, const char *entryPointName);
    void addToEnvironment(const char *name, RexxInternalObject *value);
    void addToSystem(const char *name, RexxInternalObject *value);
    void completeSystemClass(const char *name, RexxClass *classObj);
    void createRexxPackage();

    LiveStack  *liveStack;               // stack used for memory marking
    PushThroughStack *saveStack;         // our temporary protection stack
    bool              markingObjects;    // a flag to indicate we are marking objects.

    MapTable         *old2new;           // the table for tracking old2new references.
    IdentityTable    *uninitTable;       // the table of objects with uninit methods
    size_t            pendingUninits;    // count of objects in the uninit queue
    bool              processingUninits; // this is set when we are processing the uninit table
    WeakReference    *weakReferenceList; // list of active weak references
    std::vector<MemorySegment *> segments;  // A tracker for all allocated memory segments

    OldSpaceSegmentSet oldSpaceSegments;     // the image memory segments (not swept)
    NormalSegmentSet newSpaceNormalSegments; // the segment set for "normal" size allocations
    LargeSegmentSet  newSpaceLargeSegments;  // the segment set for "larger" memory allocations
    SingleObjectSegmentSet newSpaceSingleSegments; // the segment set for the very largest of allocations. Managed a little differently

    MarkHandler *currentMarkHandler;     // current handler for liveGeneral marking
    MarkHandler  defaultMarkHandler;     // the default mark handler

    MemoryStats *imageStats;             // current statistics collector

    size_t allocations;                  // number of allocations since last GC
    size_t collections;                  // number of garbage collections

    char *restoredImage;                 // our restored image.
    StringTable   *globalStrings;        // table of global strings
    MapTable      *globalReferences;     // table of global references tagged by API code.
};


/**
 * A marking object used during image restore to convert
 * buffer offsets back into pointer values.
 */
class ImageRestoreMarkHandler : public MarkHandler
{
public:

    ImageRestoreMarkHandler(char *r) : relocation(r) { }

    // pure virtual method for handling the mark operation.
    void mark(RexxInternalObject **field, RexxInternalObject *object) override
    {
        // the object reference is an offset.  Add in the address
        // of the buffer start
        *field = *(RexxInternalObject *)(relocation + (size_t)object);
    }

    char *relocation;       // the relative relocation amount
};


/**
 * A marking object used during object unflattening to convert
 * buffer offsets back into pointer values.
 */
class UnflatteningMarkHandler : public MarkHandler
{
public:

    UnflatteningMarkHandler(char *r, size_t m) : relocation(r), markWord(m) { }

    // pure virtual method for handling the mark operation.
    void mark(RexxInternalObject **field, RexxInternalObject *object) override
    {
        // At this point, the object pointer is actually an offset and
        // the base buffer pointer is our location value
        object = (RexxInternalObject *)(relocation + (size_t)object);
        *field = object;
        // make sure this object is set to the current mark word
        // so that all objects will get marked correctly on the next pass.
        object->setObjectLive(markWord);
    }

    char  *relocation;       // the buffer we're restoring into.
    size_t markWord;         // the current mark word
};


/**
 * A mark handler for the unflatten phase of envelope
 * restoral.
 */
class EnvelopeMarkHandler : public MarkHandler
{
public:
    EnvelopeMarkHandler(Envelope *e) : envelope(e) { }

    // pure virtual method for handling the mark operation.
    void mark(RexxInternalObject **field, RexxInternalObject *object) override
    {
        // do the unflatten operation
        *field = object->unflatten(envelope);
    }

    Envelope *envelope;
};


/**
 * A mark handler for handling image save flattening.
 */
class ImageSaveMarkHandler : public MarkHandler
{
public:
    ImageSaveMarkHandler(MemoryObject *m, size_t mw, char *b, size_t o) : memory(m), markWord(mw), imageBuffer(b), imageOffset(o) { }

    // pure virtual method for handling the mark operation.
    void mark(RexxInternalObject **pMarkObject, RexxInternalObject *markObject) override;

    MemoryObject *memory;    // the memory object
    size_t markWord;         // the current mark word
    char  *imageBuffer;      // the buffer used for image save/restore operations
    size_t imageOffset;      // the offset information for the image
};


/**
 * A mark handler for handling general live set tracing
 */
class TracingMarkHandler : public MarkHandler
{
public:
    TracingMarkHandler(MemoryObject *m, size_t mw) : memory(m), markWord(mw) { }

    // pure virtual method for handling the mark operation.
    void mark(RexxInternalObject **pMarkObject, RexxInternalObject *markObject) override
    {
        // Save image processing.  We only handle this if the object has not
        // already been marked.
        if (!markObject->isObjectLive(markWord))
        {
            // now immediately mark this
            markObject->setObjectLive(markWord);
            // push this object on to the live stack so it's references can be marked later.
            memory->pushLiveStack(markObject);
        }
    }


    MemoryObject *memory;    // the memory object
    size_t markWord;         // the current mark word
};


/******************************************************************************/
/* Memory management macros                                                   */
/******************************************************************************/

inline void holdObject(RexxInternalObject *o) { memoryObject.holdObject(o); }

inline RexxInternalObject *new_object(size_t s) { return memoryObject.newObject(s); }
inline RexxInternalObject *new_object(size_t s, size_t t) { return memoryObject.newObject(s, t); }
inline RexxInternalObject *new_object(size_t s, size_t t, size_t i) { memoryObject.checkLiveStack(i); return memoryObject.newObject(s, t); }

inline ArrayClass *new_arrayOfObject(size_t s, size_t c, size_t t)  { return memoryObject.newObjects(s, c, t); }


// memory marking macros.  These are macros because they use the assumed arguments
// passed to the live method
#define ObjectNeedsMarking(oref) ((oref) != OREF_NULL && !((oref)->isObjectMarked(liveMark)) )
#define memory_mark(oref)  if (ObjectNeedsMarking(oref)) memoryObject.mark(oref)
#define memory_mark_general(oref) (memoryObject.markGeneral((void *)&(oref)))

// some convenience macros for marking arrays of objects.
#define memory_mark_array(count, array) \
  for (size_t i = 0; i < count; i++)    \
  {                                     \
      memory_mark(array[i]);            \
  }

#define memory_mark_general_array(count, array) \
  for (size_t i = 0; i < count; i++)            \
  {                                             \
      memory_mark_general(array[i]);            \
  }

// Following macros are for Flattening and unflattening of objects
// Some notes on what is going on here.  The flatten() method gets called on an object
// after it has been moved into the envelope buffer, so the this pointer is
// to the copied object, not the original.  On a call to flattenReference(), it might
// be necessary to allocate a larger buffer.  When that happens, the copied object gets
// moved to a new location and the newThis pointer gets updated to the new object location.
// it is necessary to copy the this pointer and also declare the newThis pointer as volatile
// so that the change in pointer value doesn't get optimized out by the compiler.

// set up for flattening.  This sets up the newThis pointer and also gets some
// information from the envelope.  The type argument allows newThis to be declared with
// the correct type.
#define setUpFlatten(type)        \
  {                               \
  size_t newSelf = envelope->currentOffset; \
  type * volatile newThis = (type *)this;   // NB:  This is declared volatile to avoid optimizer problems.

// just a block closer for the block created by setUpFlatten.
#define cleanUpFlatten                    \
 }

// newer, simplified form.  Just give the name of the field.
#define flattenRef(oref)  if ((newThis->oref) != OREF_NULL) envelope->flattenReference((void *)&newThis, newSelf, (void *)&(newThis->oref))

// a version for flattening arrays of objects.  Give the count field and the name of the array.
#define flattenArrayRefs(count, array)          \
  for (size_t i = 0; i < count; i++)            \
  {                                             \
      flattenRef(array[i]);                     \
  }

// declare a class creation routine
// for classes with their own
// explicit class objects
#define CLASS_CREATE(name) The##name##Class = new RexxClass(#name, The##name##ClassBehaviour, The##name##Behaviour);

#define CLASS_CREATE_SPECIAL(name, id, className) The##name##Class = new className(id, The##name##ClassBehaviour, The##name##Behaviour);

#endif
