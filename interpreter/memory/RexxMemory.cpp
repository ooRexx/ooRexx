/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/*                                                                            */
/* The main subsystem for dealing with all memory-related issues and          */
/* interpreter initialization and image build.                                */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"
#include "RexxMemory.hpp"
#include "MemoryStack.hpp"
#include "StringClass.hpp"
#include "MutableBufferClass.hpp"
#include "DirectoryClass.hpp"
#include "Activity.hpp"
#include "IntegerClass.hpp"
#include "ArrayClass.hpp"
#include "TableClass.hpp"
#include "RexxActivation.hpp"
#include "ActivityManager.hpp"
#include "MessageClass.hpp"
#include "MethodClass.hpp"
#include "RelationClass.hpp"
#include "SupplierClass.hpp"
#include "PointerClass.hpp"
#include "BufferClass.hpp"
#include "PackageClass.hpp"
#include "WeakReferenceClass.hpp"
#include "StackFrameClass.hpp"
#include "Interpreter.hpp"
#include "SystemInterpreter.hpp"
#include "Interpreter.hpp"
#include "PackageManager.hpp"
#include "SysFileSystem.hpp"
#include "UninitDispatcher.hpp"
#include "GlobalProtectedObject.hpp"
#include "MapTable.hpp"
#include "SetClass.hpp"
#include "BagClass.hpp"
#include "NumberStringClass.hpp"
#include "RexxInfoClass.hpp"
#include "VariableReference.hpp"
#include "EventSemaphore.hpp"
#include "MutexSemaphore.hpp"
#include "SysFile.hpp"
#include "SysProcess.hpp"
#include <stdio.h>
#include <stdarg.h>

// restore a class from its
// associated primitive behaviour
// (already restored by memory_init)
#define RESTORE_CLASS(name, className) The##name##Class = (className *)RexxBehaviour::getPrimitiveBehaviour(T_##name)->restoreClass();

// NOTE:  There is just a single memory object in global storage.  We'll define
// memobj to be the direct address of this memory object.
MemoryObject memoryObject;


/**
 * Local function to handle memory logging.
 *
 * @param outfile The output file handle.
 * @param message The message to write out
 */
static void logMemoryCheck(FILE *outfile, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    if (outfile != NULL)
    {
        vfprintf(outfile, message, args);
    }
    va_end(args);
}


/**
 * Main Constructor for Rexxmemory, called once during main
 * initialization.  Will create the initial memory Pool(s), etc.
 */
MemoryObject::MemoryObject()
{
    // we need to set a valid size for this object.  We round it up
    // to the minimum allocation boundary, even though that might be
    // a lie.  Since this never participates in a sweep operation,
    // this works ok in the end.
    setObjectSize(Memory::roundObjectBoundary(sizeof(MemoryObject)));

    // OR'ed into object headers to mark during gc
    markWord = 1;
    saveStack = OREF_NULL;
    globalReferences = OREF_NULL;

    // we always start out with an empty list.  WeakReferences that are in the
    // saved image will (MUST) never be set to a new value, so it's not necessary
    // to hook those back up again.
    weakReferenceList = OREF_NULL;
}


/**
 * Initialize the memory object, including getting the
 * initial memory pool allocations.
 *
 * @param restoringImage
 *               True if we are initializing during an image restore, false
 *               if we need to build the initial image environemnt (i.e., called
 *               via rexximage during a build).
 * @param imageTarget
 *               The location to store the image if this is a save operation.
 */
void MemoryObject::initialize(bool restoringImage, const char *imageTarget)
{
    // The constructor makes sure some crucial aspects of the Memory object are set up.
    new (this) MemoryObject;

    // create our various segment pools
    new (&newSpaceNormalSegments) NormalSegmentSet(this);
    new (&newSpaceLargeSegments) LargeSegmentSet(this);
    new (&newSpaceSingleSegments) SingleObjectSegmentSet(this);

    // and the new/old Space segments
    new (&oldSpaceSegments) OldSpaceSegmentSet(this);

    collections = 0;
    allocations = 0;
    globalStrings = OREF_NULL;

    // get our table of virtual functions setup first thing.  We need this
    // before we can allocate our first real object.
    buildVirtualFunctionTable();

    // This is the live stack used for
    // sweep marking (not allocated from the object heap)
    liveStack = new(Memory::LiveStackSize) LiveStack(Memory::LiveStackSize);

    // if we're restoring, load everything from the image file.  All of the
    // classes will exist then, as well as restoring all of the
    // behaviours
    if (restoringImage)
    {
        restoreImage();
    }

    // set the object behaviour
    memoryObject.setBehaviour(TheMemoryBehaviour);

    // make sure we have an inital segment set to allocate from.
    newSpaceNormalSegments.getInitialSet();

    // we also get an initial segment set for large object, since we
    // will need this for the globalReferences table.
    newSpaceLargeSegments.getInitialSet();

    // get the initial uninit table
    uninitTable = new_identity_table();

    // and our global references table
    globalReferences = new MapTable(Memory::DefaultGlobalReferenceSize);

    // this is our size of uninit objects awaiting processing.
    pendingUninits = 0;

    // is this image creation?  This will build and save the image, then
    // terminate
    if (!restoringImage)
    {
        createImage(imageTarget);
    }

    restore();                           // go restore the state of the memory object
}


/**
 * Log verbose output events
 *
 * @param message The main message text.
 * @param sub1    The first substitution value.
 * @param sub2    The second substitution value.
 * @param sub3    third substitution value
 */
void MemoryObject::logVerboseOutput(const char *message, void *sub1, void *sub2, void *sub3)
{
    logMemoryCheck(NULL, message, sub1, sub2, sub3);
}


/**
 * Main memory_mark driving loop
 *
 * @param rootObject The root object used for the marking (usually the
 *                   MemoryObject).
 */
void MemoryObject::markObjectsMain(RexxInternalObject *rootObject)
{
    // for some of the root objects, we get called to mark them before they get allocated.
    // make sure we don't process any null references.
    if (rootObject == OREF_NULL)
    {
        return;
    }

    // flag that we're in the middle of a mark operation
    markingObjects = true;

    // set up the live marking word passed to the live() routines
    // we include the OldSpaceBit here to allow both conditions to be tested
    // in one shot.
    size_t liveMark = markWord | ObjectHeader::OldSpaceBit;

    allocations = 0;
    // add a fence to the stack to act as a terminator.
    pushLiveStack(OREF_NULL);
    // mark the root object and start processing the stacked item.
    // we terminate once we hit the null fence item.
    mark(rootObject);
    for (RexxInternalObject *markObject = popLiveStack(); markObject != OREF_NULL; markObject = popLiveStack())
    {
        // mark the behaviour as live
        memory_mark(markObject->behaviour);
        // Mark other referenced obj.  We can do this without checking
        // the references flag because we only push the object on to
        // the stack if it has references.
        allocations++;
        markObject->live(liveMark);
    }

    // we are done marking, no longer in a critical section
    markingObjects = false;
}


/**
 * Check for objects that require an uninit method run.
 */
void MemoryObject::checkUninit()
{
    // we might not actually have a table yet, so make sure we check
    // before using it.
    if (uninitTable == NULL)
    {
        return;
    }

    // scan the uninit table looking for objects that are elegable for collection.
    for (HashContents::TableIterator iterator = uninitTable->iterator(); iterator.isAvailable(); iterator.next())
    {
        RexxInternalObject *uninitObject = iterator.value();

        // was this object not marked by the last sweep operation?
        if (uninitObject != OREF_NULL && uninitObject->isObjectDead(markWord))
        {
            // mark this as ready for uninit
            uninitObject->setReadyForUninit();
            // up the pending uninit count
            pendingUninits++;
        }
    }
}


/**
 * Force a last-gasp garbage collection and running of the
 * uninits during interpreter instance shutdown.  This is an
 * attempt to ensure that all objects with uninit methods get
 * a chance to clean up prior to termination.
 */
void MemoryObject::collectAndUninit(bool clearStack)
{
    // clear the save stack if we're working with a single instance
    if (clearStack)
    {
        clearSaveStack();
    }
    collect();
    runUninits();
}


/**
 * Force a last-gasp garbage collection and running of the
 * uninits during interpreter instance shutdown.  This is an
 * attempt to ensure that all objects with uninit methods get
 * a chance to clean up prior to termination.
 */
void MemoryObject::lastChanceUninit()
{
    // collect and run any uninits still pending
    collectAndUninit(true);
    // we're about to start releasing libraries, so it is critical
    // we don't run any more uninits after this
    uninitTable->empty();
}


/**
 * Run any pending uninit methods for this activity.
 */
void  MemoryObject::runUninits()
{
    // if we're already processing this, don't try to do this
    // recursively.
    if (processingUninits)
    {
        return;
    }

    // ok, turn on the interlock
    processingUninits = true;
    verboseMessage("Starting to process pending uninit methods\n");

    // get the current activity for running the uninits
    Activity *activity = ActivityManager::currentActivity;

    // scan the uninit table looking for objects that are elegable for collection.
    for (HashContents::TableIterator iterator = uninitTable->iterator(); iterator.isAvailable();)
    {
        RexxInternalObject *uninitObject = iterator.value();

        // was this object already marked for running the uninit?  run it now
        if (uninitObject != OREF_NULL && uninitObject->isReadyForUninit())
        {
            // we remove this item here and advance the iterator.
            iterator.removeAndAdvance();
            // remove the pending item count
            pendingUninits--;

            // because we've removed this from the uninit table, it is no longer
            // proctected.  We need to ensure it does not get GC'd until after the
            // uninit method runs
            ProtectedObject p(uninitObject);

            // run this method with appropriate error trapping
            UninitDispatcher dispatcher(uninitObject);
            activity->run(dispatcher);
        }
        // not processing that item, so just step the iterator
        else
        {
            iterator.next();
        }
    }

    // turn off the interlock
    processingUninits = false; ;
    verboseMessage("Done processing pending uninit methods\n");
}


/**
 * Remove an object from the uninit tables.  This can happen
 * if a setMethod() on an object removes the uninit method.
 *
 * @param obj    The object to remove.
 */
void  MemoryObject::removeUninitObject(RexxInternalObject *obj)
{
    // just remove this object from the table
    uninitTable->remove(obj);
}


/**
 * Add an object with an uninit method to the uninit table
 *
 * @param obj    The object to add.
 */
void MemoryObject::addUninitObject(RexxInternalObject *obj)
{
    // just add to the table unconditionally.
    uninitTable->put(obj, obj);
}


/**
 * Main mark routine for garbage collection.
 */
void MemoryObject::markObjects()
{
    verboseMessage("Beginning mark operation\n");

    // do the marking using the memory object as the root object.
    markObjectsMain(this);
    // now process the weak reference queue...We check this before the
    // uninit list is processed so that the uninit list doesn't mark any of the
    // weakly referenced items.  We don't want an object placed on the uninit queue
    // to end up strongly referenced later.
    checkWeakReferences();

    // check for any objects we're holding for uninits that have
    // gone out of scope
    checkUninit();
    // make the uninit table and the pending uninits queue to keep those
    // objects from getting reclaimed.
    markObjectsMain(uninitTable);

    // if we had to expand the live stack previously, we allocated a temporary
    // one from malloc() storage rather than the object heap.  We will hold on
    // to the expanded one because once we've hit the threshold where we expand,
    // it's likely we'll need to in the future.
    verboseMessage("Mark operation completed\n");
}


/**
 * Scan the weak reference queue looking for either dead weak
 * objects or weak references that refer to objects that have gone out of
 * scope.
 */
void MemoryObject::checkWeakReferences()
{
    WeakReference *current = weakReferenceList;
    // list of "live" weak references...built while scanning
    WeakReference *newList = OREF_NULL;

    // loop through the list
    while (current != OREF_NULL)
    {
        // we have to save the next one in the list
        WeakReference *next = current->nextReferenceList;
        // this reference still in scope?
        if (current->isObjectLive(markWord))
        {
            // keep this one in the list
            current->nextReferenceList = newList;
            newList = current;
            // have a reference?
            if (current->referentObject != OREF_NULL)
            {
                // if the object is not alive, null out the reference
                if (!current->referentObject->isObjectLive(markWord))
                {
                    current->referentObject = OREF_NULL;
                }
            }
        }
        // step to the new next item
        current = next;
    }

    // update the list
    weakReferenceList = newList;
}


/**
 * Add a new weak reference to the tracking table
 *
 * @param ref    The weak reference item to add.
 */
void MemoryObject::addWeakReference(WeakReference *ref)
{
    // just add this to the front of the list
    ref->nextReferenceList = weakReferenceList;
    weakReferenceList = ref;
}


/**
 * Allocate the memory for a managed heap segment.
 *
 * @param requestedBytes
 *               The size required for the segment
 *
 * @return A new segment or NULL in the case of an allocation failure.
 */
MemorySegment *MemoryObject::newSegment(size_t requestedBytes)
{
    // the platform determines how the memory is allocated
    void *segmentBlock = SystemInterpreter::allocateSegmentMemory(requestedBytes);
    // return nothing if this was not allocated
    if (segmentBlock == NULL)
    {
        return NULL;
    }

    // initialize this as a segment object.
    MemorySegment *segment = new (segmentBlock) MemorySegment (requestedBytes);
    // we keep track of the segments here for cleanup at shutdown time.
    segments.push_back(segment);
    return segment;
}


/**
 * Return a memory segment to the system.
 *
 * @param segment The segment to release.
 */
void MemoryObject::freeSegment(MemorySegment *segment)
{
    // free up all of the allocated memory segments
    for (std::vector<MemorySegment *>::iterator it = segments.begin(); it != segments.end(); ++it)
    {
        // is this the segment being released, remove it from the tracking list
        if (*it == segment)
        {
                                    // and release the memory before removing from
                                    // the tracking list and the iterator is no longer
                                    // valid.
            SystemInterpreter::releaseSegmentMemory (*it);
            segments.erase(it);     // remove from the tracking list
            break;                  // the iterator is no longer usable after the erase.
        }
    }
}


/**
 * Allocate a segment of the requested size.  The requested size
 * is the desired size, while the minimum is the absolute minimum we can
 * handle.  This takes care of the overhead accounting and additional
 * rounding.  The requested size is assumed to have been rounded up to the
 * next "appropriate" boundary already, and the segment overhead will be
 * allocated from that part, if possible.  Otherwise, an
 * additional page is added.
 *
 * @param requestedBytes
 *                 The suggested number of bytes to allocate.
 * @param minBytes The minimum number of bytes we can accept.  Failure to
 *                 allocate the minimum is an out-of-memory situation.
 *
 * @return The new memory segment.
 */
MemorySegment *MemoryObject::newSegment(size_t requestedBytes, size_t minBytes)
{
    // first make sure we've got enough space for the control
    // information. We don't do any rounding. The segment set has already
    // decided what the apropriate size should be. In some cases (i.e., the single object
    // segment set), rounding would just waste memory.
    requestedBytes = requestedBytes + MemorySegment::MemorySegmentOverhead;
    verboseMessage("Allocating a new segment of %d bytes\n", requestedBytes);
    // try to allocate a new segment
    MemorySegment *segment = newSegment(requestedBytes);
    if (segment == NULL)
    {
        verboseMessage("Allocating a boundary new segment of %d bytes\n", requestedBytes);
        // Segmentsize is the minimum size request we handle.  If
        // minbytes is small, then we're just adding a segment to the
        // small pool.  Reduce the request to SegmentSize and try again.
        // For all other requests, try once more with the minimum.
        minBytes = minBytes + MemorySegment::MemorySegmentOverhead;
        verboseMessage("Allocating a fallback new segment of %d bytes\n", minBytes);
        // try to allocate once more...if this fails, the caller will
        // have to handle it.
        segment = newSegment(minBytes);
    }
    return segment;                      // return the allocated segment
}


/**
 * Allocate a segment of the requested size.  The requested size
 * is the desired size, while the minimum is the absolute minimum we can
 * handle.
 *
 * @param requestedBytes
 *                 The requested bytes for the allocation.
 * @param minBytes The minimum acceptable allocation size.
 *
 * @return A new segment.
 */
MemorySegment *MemoryObject::newLargeSegment(size_t requestedBytes, size_t minBytes)
{
    // first make sure we've got enough space for the control
    // information, and round this to a proper boundary
    size_t allocationBytes = MemorySegment::roundSegmentBoundary(requestedBytes + MemorySegment::MemorySegmentOverhead);
#ifdef MEMPROFILE
    printf("Allocating large boundary new segment of %zu bytes for request of %zu\n", allocationBytes, requestedBytes);
#endif
    // try allocate a segment using the requested size
    MemorySegment *segment = newSegment(allocationBytes);
    if (segment == NULL)
    {
        // Segmentsize is the minimum size request we handle.  If
        // minbytes is small, then we're just adding a segment to the
        // small pool.  Reduce the request to SegmentSize and try again.
        // For all other requests, try once more with the minimum.
        minBytes = MemorySegment::roundSegmentBoundary(minBytes + MemorySegment::MemorySegmentOverhead);
        segment = newSegment(minBytes);
    }
    return segment;
}


/**
 * Restore a saved image to usefulness.
 */
void MemoryObject::restoreImage()
{
    // Nothing to restore if we have a buffer already
    if (restoredImage != NULL)
    {
        return;
    }

    size_t imageSize;

    // load the image file
    loadImage(restoredImage, imageSize);
    // we write a size to the start of the image when the image is created.
    // the restoredImage buffer does not include that image size, so we
    // need to pretend the buffer is slightly before the start.
    // image data is just past that information.
    char *relocation = restoredImage - sizeof(size_t);

    // create a handler for fixing up reference addresses.
    ImageRestoreMarkHandler markHandler(relocation);
    setMarkHandler(&markHandler);

    // address the start and end of the image.
    RexxInternalObject *objectPointer = (RexxInternalObject *)restoredImage;
    RexxInternalObject *endPointer = (RexxInternalObject *)(restoredImage + imageSize);

    // the save array is the 1st object in the buffer
    ArrayClass *saveArray = (ArrayClass *)objectPointer;

    // now loop through the image buffer fixing up the objects.
    while (objectPointer < endPointer)
    {
        size_t primitiveTypeNum;

        // Fixup the Behaviour pointer for the object.

        // If this is a primitive object, we can just pick up the hard coded
        // behaviour.
        if (objectPointer->isNonPrimitive())
        {
            // Working with a copy, so don't use the static table version.
            // the behaviour will have been packed into the image, so we need to
            // pick up the reference.
            RexxBehaviour *imageBehav = (RexxBehaviour *)(relocation + (uintptr_t)objectPointer->behaviour);
            // and rewrite the offset version with the resolved pointer
            objectPointer->behaviour = imageBehav;
            // get the type number from the behaviour.  This tells us what
            // virtual function pointer to use.
            primitiveTypeNum = imageBehav->getClassType();
        }
        else
        {
            // the original behaviour pointer has been encoded as a type number and class
            // category to allow us to convert back to the appropriate type.
            objectPointer->behaviour = RexxBehaviour::restoreSavedPrimitiveBehaviour(objectPointer->behaviour);
            primitiveTypeNum = objectPointer->behaviour->getClassType();
        }
        // This will be an OldSpace object.  We delay setting this
        // until now, because the oldspace bit is overloaded with the
        // NonPrimitive bit.  Since the we are done with the
        // non-primitive bit now, we can use this for the oldspace
        // flag too.
        objectPointer->setOldSpace();
        // now fix up the virtual function table for the object using the type number
        objectPointer->setVirtualFunctions(virtualFunctionTable[primitiveTypeNum]);

        // if the object has references, call liveGeneral() to cause these references to
        // be converted back into real pointers
        if (objectPointer->hasReferences())
        {
            objectPointer->liveGeneral(RESTORINGIMAGE);
        }
        // advance to the next object in the image
        objectPointer = objectPointer->nextObject();
    }

    // now start restoring the critical objects from the save array
    TheEnvironment = (DirectoryClass *)saveArray->get(saveArray_ENV);
    // restore all of the primitive behaviour data...right now, these are
    // all default values.
    ArrayClass *primitiveBehaviours = (ArrayClass *)saveArray->get(saveArray_PBEHAV);
    for (size_t i = 0; i <= T_Last_Exported_Class; i++)
    {
        RexxBehaviour::primitiveBehaviours[i].restore((RexxBehaviour *)primitiveBehaviours->get(i + 1));
    }

    TheSystem      = (StringTable *)saveArray->get(saveArray_SYSTEM);
    TheTrueObject  = (RexxInteger *)saveArray->get(saveArray_TRUE);
    TheFalseObject = (RexxInteger *)saveArray->get(saveArray_FALSE);
    TheNilObject   = (RexxObject *)saveArray->get(saveArray_NIL);
    TheNullArray   = (ArrayClass *)saveArray->get(saveArray_NULLA);
    TheNullPointer   = (PointerClass *)saveArray->get(saveArray_NULLPOINTER);
    TheClassClass  = (RexxClass *)saveArray->get(saveArray_CLASS);
    TheCommonRetrievers = (StringTable *)saveArray->get(saveArray_COMMON_RETRIEVERS);
    TheRexxPackage = (PackageClass *)saveArray->get(saveArray_REXX_PACKAGE);

    // restore the global strings
    memoryObject.restoreStrings((ArrayClass *)saveArray->get(saveArray_NAME_STRINGS));
    // make sure we have a working thread context
    Activity::initializeThreadContext();
    // now we can restore the packages
    PackageManager::restore((ArrayClass *)saveArray->get(saveArray_PACKAGES));
}


/**
 * Load the image file into storage
 *
 * @param imageBuffer
 *                  The returned image buffer
 * @param imageSize The returned image size.
 */
void MemoryObject::loadImage(char *&imageBuffer, size_t &imageSize)
{
    FileNameBuffer fullname;

    // BASEIMAGE should be located together with the Rexx shared libraries
    const char *installLocation =  SysProcess::getLibraryLocation();
    if (installLocation != NULL)
    {
        fullname = installLocation;
        fullname += BASEIMAGE;
        if (loadImage(imageBuffer, imageSize, fullname))
        {
            return;
        }
    }

    fullname = BASEIMAGE;

    // try the current directory next
    if (loadImage(imageBuffer, imageSize, fullname))
    {
        return;
    }

    FileNameBuffer path;

    SystemInterpreter::getEnvironmentVariable("PATH", path);

    // Now try to locate the file on the path if that fails
    if (SysFileSystem::primitiveSearchName(BASEIMAGE, path, NULL, fullname))
    {
        if (loadImage(imageBuffer, imageSize, fullname))
        {
            return;
        }
    }
    Interpreter::logicError("cannot locate startup image " BASEIMAGE);
}


/**
 * Load the image file into storage
 *
 * @param imageBuffer
 *                  The returned image buffer
 * @param imageSize The returned image size.
 */
bool MemoryObject::loadImage(char *&imageBuffer, size_t &imageSize, FileNameBuffer &imageFile)
{
    SysFile image;
    // if unable to open this, return false
    if (!image.open(imageFile, RX_O_RDONLY, RX_S_IREAD, RX_SH_DENYWR))
    {
        return false;
    }

    size_t bytesRead = 0;
    // read in for the size of the image
    if (!image.read((char *)&imageSize, sizeof(imageSize), bytesRead))
    {
        return false;
    }

    // Create new segment for image
    imageBuffer = (char *)memoryObject.allocateImageBuffer(imageSize);
    // Create an object the size of the
    // image. We will be overwriting the
    // object header.
    // read in the image, store the
    // the size read
    if (!image.read(imageBuffer, imageSize, imageSize))
    {
        Interpreter::logicError("could not read in the image");
    }
    return true;
}


/**
 * Main live marking routine for normal garbage collection.
 * This starts the process by marking the key root objects.
 *
 * @param liveMark The current live mark.
 */
void MemoryObject::live(size_t liveMark)
{
    // Mark the save stack first, since it will be pulled off of
    // the stack after everything else.  This will give other
    // objects a chance to be marked before we remove them from
    // the savestack.
    memory_mark(saveStack);
    memory_mark(old2new);
    memory_mark(globalStrings);
    memory_mark(environment);
    memory_mark(commonRetrievers);
    memory_mark(system);
    memory_mark(rexxPackage);
    memory_mark(globalReferences);

    // now call the various subsystem managers to mark their references
    Interpreter::live(liveMark);
    SystemInterpreter::live(liveMark);
    ActivityManager::live(liveMark);
    PackageManager::live(liveMark);
    // mark any protected objects we've been watching over

    GlobalProtectedObject *p = protectedObjects;
    while (p != NULL)
    {
        if (p->protectedObject != OREF_NULL)
        {
            memory_mark(p->protectedObject);
        }
        p = p->next;
    }
}


/**
 * General live marking routine for the memory object.
 *
 * @param reason The marking reason.
 */
void MemoryObject::liveGeneral(MarkReason reason)
{
    memory_mark_general(saveStack);
    memory_mark_general(old2new);
    memory_mark_general(globalStrings);
    memory_mark_general(environment);
    memory_mark_general(commonRetrievers);
    memory_mark_general(system);
    memory_mark_general(rexxPackage);
    memory_mark_general(globalReferences);

    // now call the various subsystem managers to mark their references
    Interpreter::liveGeneral(reason);
    SystemInterpreter::liveGeneral(reason);
    ActivityManager::liveGeneral(reason);
    PackageManager::liveGeneral(reason);
    // mark any protected objects we've been watching over

    GlobalProtectedObject *p = protectedObjects;
    while (p != NULL)
    {
        memory_mark_general(p->protectedObject);
        p = p->next;
    }
}


/**
 * Collect all dead memory in the Rexx object space.  The
 * collection process performs a mark operation to mark all of the live
 * objects, followed by sweep of each of the segment sets.
 */
void MemoryObject::collect()
{
    collections++;
    verboseMessage("Begin collecting memory, cycle #%zu after %zu allocations.\n", collections, allocations);
    allocations = 0;

    // change our marker to the next value so we can distinguish
    // between objects marked on this cycle from the objects marked
    // in the pervious cycles.
    bumpMarkWord();

    // do the object marking now...followed by a sweep of all of the
    // segments.
    markObjects();

    // have each of the segment spaces sweep up the dead objects from
    // all of their spaces.
    newSpaceNormalSegments.sweep();
    newSpaceLargeSegments.sweep();
    newSpaceSingleSegments.sweep();

    // The space segments are now in a known, completely clean state.
    // Now based on the context that caused garbage collection to be
    // initiated, the segment sets may be expanded to add additional
    // free memory.  The decision to expand the object space requires
    // the usage statistics collected by the mark-and-sweep
    // operation.

    verboseMessage("End collecting memory\n");
}


/**
 * Allocate an object in "old space".  This is generally
 * used just for special memory objects or for allocating
 * the space for the restored image.
 *
 * @param requestLength
 *               The size of the object.
 *
 * @return Storage for a new object.
 */
RexxInternalObject *MemoryObject::oldObject(size_t requestLength)
{
    // Compute size of new object and allocate from the old segment pool
    requestLength = Memory::roundObjectBoundary(requestLength);
    RexxInternalObject *newObj = oldSpaceSegments.allocateObject(requestLength);

    // if we got a new object, then perform the final setup steps.
    // Since the oldspace objects are special, we don't push them on
    // to the save stack.  Also, we don't set the oldspace flag, as
    // those are a separate category of object.
    if (newObj != OREF_NULL)
    {
        // initialize the new object
        ((RexxObject *)newObj)->initializeNewObject(requestLength, markWord, virtualFunctionTable[T_Object], TheObjectBehaviour);
    }

    return newObj;
}


/**
 * Allocate an image buffer for the system code.  The image buffer
 * is allocated in the oldspace segment set.  We create an object from that
 * space, then return this object as a character pointer.  We eventually will
 * get that pointer passed back to us as the image address.
 *
 * @param imageSize The size required for the image buffer.
 *
 * @return The storage for the image.
 */
char *MemoryObject::allocateImageBuffer(size_t imageSize)
{
    return (char *)oldObject(imageSize);
}


/**
 * allocate a new object of the requested size.
 *
 * @param requestLength
 *               The required object length.
 * @param type   The object type this should be initialized to.
 *
 * @return The storage for creating a new object.
 */
RexxInternalObject *MemoryObject::newObject(size_t requestLength, size_t type)
{
    allocations++;

    // Compute size of new object and determine where we should
    // allocate from
    requestLength = Memory::roundObjectBoundary(requestLength);

    RexxInternalObject *newObj;

    // is this a typical small object? These come from a segment set that optimizes
    // for small size objects.
    if (requestLength <= MemorySegment::LargeBlockThreshold)
    {
        // make sure we don't go below our minimum size.
        if (requestLength < Memory::MinimumObjectSize)
        {
            requestLength = Memory::MinimumObjectSize;
        }
        newObj = newSpaceNormalSegments.allocateObject(requestLength);
        // if we could not allocate, process an allocation failure.  This will
        // drive a garbage collection and potentially expand the heap size. This also
        // raises an error if all of the recovery steps result in an allocation failure.
        if (newObj == NULL)
        {
            newObj = newSpaceNormalSegments.handleAllocationFailure(requestLength);
        }
    }
    // between the small size threshold and "really big", we allocate from a segment set
    // designed to handle this range of allocation.
    else if (requestLength <= MemorySegment::SingleBlockThreshold)
    {
        // these are larger objects, but we still try to pack them tightly in the
        // segment sets.
        newObj = newSpaceLargeSegments.allocateObject(requestLength);

        // again, if there is a failure, we try various fall back strategies.
        if (newObj == NULL)
        {
            newObj = newSpaceLargeSegments.handleAllocationFailure(requestLength);
        }
    }
    // once we get into really large objects (segment size or larger), we manage those allocations separately
    // so that they get removed from the heap when they get garbage collected.
    else
    {
        // since we're only to have one object in the segment, there's no need to round
        // the size to any boundary.
        newObj = newSpaceSingleSegments.allocateObject(requestLength);
        if (newObj == NULL)
        {
            // Recovery here will always force a GC and try again. Because previously
            // allocated large objects may have been returned to the system, this might succeed
            // this time.
            newObj = newSpaceSingleSegments.handleAllocationFailure(requestLength);
        }
    }


    // initialize the object to the required type.  This also fills in the
    // object header so things work correctly on a garbage collection.
    // NOTE that this is a method on the RexxObject class because it uses access to
    // Object variables field.  Not terribly optimal to need to cast this way, but
    // it is pragmatic...
    ((RexxObject *)newObj)->initializeNewObject(markWord, virtualFunctionTable[type], RexxBehaviour::getPrimitiveBehaviour(type));

    if (saveStack != OREF_NULL)
    {
        // saveobj doesn't get turned on until the system is initialized
        //far enough but once it's on, push this new obj on the save stack to
        //keep it from being garbage collected before it can be used
        //and safely anchored by caller.
        pushSaveStack(newObj);
    }
    // return the newly allocated object to our caller
    return newObj;
}


/**
 * Resize an object to a smaller size.   Usually used for array
 * objects after a reallocation.
 *
 * The object shrinkObj only needs to be the size of newSize If
 * the left over space is big enough to become a dead object we
 * will shrink the object to the specified size.
 *
 * NOTE: Since memory knows nothing about any objects that are
 * in the extra space, it cannot do anything about them if this
 * is an OldSpace objetc, therefore the caller must have already
 * take care of this.
 *
 * @param shrinkObj The object to resize.
 * @param requestSize
 *                  The new object size.
 */
void MemoryObject::reSize(RexxInternalObject *shrinkObj, size_t requestSize)
{
    // any object following this object must be aligned on an object
    // grain boundary, so we round to that boundary.
    size_t newSize = Memory::roundObjectResize(requestSize);

    size_t objectSize = shrinkObj->getObjectSize();

    // The rounded size must still be smaller than the object we're shrinking
    // AND the trailing part of the object must be at least a minimum size object.
    if (newSize < objectSize && (objectSize - newSize) >= Memory::MinimumObjectSize)
    {
        size_t deadObjectSize = objectSize - newSize;
        // Yes, then we can shrink the object.  Get starting point of
        // the extra, this will be the new Dead obj. This will be swept up on the next GC.
        DeadObject *newDeadObj = new ((void *)((char *)shrinkObj + newSize)) DeadObject (deadObjectSize);
        // Adjust size of original object
        shrinkObj->setObjectSize(newSize);
    }
}


/**
 * Orchestrate sharing of sharing of storage between the segment
 * sets in low storage conditions.  We do this only as a last ditch, as we'll
 * end up fragmenting the large heap with small blocks, messing up the
 * benefits of keeping the heaps separate.  We first look a set to donate a
 * segment to the requesting set.  We try to find a block of
 * storage large enough for the request than we can add to our
 * dead cache until the next GC cycle.
 *
 * @param requestor The segment set needing to steal storage.
 * @param allocationLength
 *                  The allocation length needed.
 */
void MemoryObject::scavengeSegmentSets(MemorySegmentSet *requestor, size_t allocationLength)
{
    MemorySegmentSet *donor;

    // first determine the donor/requester relationships.
    if (requestor->is(MemorySegmentSet::SET_NORMAL))
    {
        donor = &newSpaceLargeSegments;
    }
    else
    {
        donor = &newSpaceNormalSegments;
    }


    // we can't just move a segment over.  Find the smallest block
    // we can find that will satisfy this allocation.  If found, we
    // can insert it into the normal deadchains.
    DeadObject *largeObject = donor->donateObject(allocationLength);
    if (largeObject != NULL)
    {
        verboseMessage("Donating an object of %zu bytes from %s to %s\n", largeObject->getObjectSize(), donor->name, requestor->name);
        // we need to insert this into the normal dead chain
        // locations.
        requestor->addDeadObject(largeObject);
    }

    // Note, by this point, the Single Object set has already released its empty
    // blocks or transferred segments to the Normal set, so there is nothing to
    // scavenge from there.
}


/**
 * Transfer a memory segment from the single object pool into the
 * normal segment space because of array object expansion.
 *
 * @param segment The segment to transfer.
 */
void MemoryObject::transferSegmentToNormalSet(MemorySegment *segment)
{
    newSpaceNormalSegments.transferSegment(segment);
}


/**
 * Process a live-stack overflow situation
 */
void MemoryObject::liveStackFull()
{
    // create a new stack that a stack increment larger
    LiveStack *newLiveStack = liveStack->reallocate(Memory::LiveStackSize);

    // delete the existing liveStack
    delete liveStack;
    // we can set the new stack
    liveStack = newLiveStack;
}


/**
 * Process a predictive live-stack overflow situation
 */
void MemoryObject::liveStackFull(size_t needed)
{
    // create a new stack that a stack increment larger
    LiveStack *newLiveStack = liveStack->ensureSpace(needed);

    // delete the existing liveStack
    delete liveStack;
    // we can set the new stack
    liveStack = newLiveStack;
}


/**
 * Perform a memory management mark operation.  This is only
 * used during a real garbage collection.
 *
 * @param markObject The object being marked.
 */
void MemoryObject::mark(RexxInternalObject *markObject)
{
    // get the current live mark to use for testing
    size_t liveMark = markWord | ObjectHeader::OldSpaceBit;

    // The following is useful for debugging some garbage collection problems where
    // an object with a NULL VFT is getting pushed on the to stack. This is a somewhat
    // critical performance pack, so only enable these lines when debugging problems.
#ifdef CHECKOREFS
    if (!markObject->checkVirtualFunctions())
    {
        Interpreter::logicError("Invalid object traced during garbage collection");
    }
#endif

    // mark this object as live
    markObject->setObjectLive(markWord);

    // if the object does not have any references, we don't need to push this on
    // the stack.  We might need to do this with the behaviour, but since they are
    // shared across object instances, we frequently can skip that as well.
    if (markObject->hasNoReferences())
    {
        if (ObjectNeedsMarking(markObject->behaviour))
        {
            // mark the behaviour now to keep us from processing this
            // more than once.
            markObject->behaviour->setObjectLive(markWord);
            // push the behaviour on the live stack to mark later
            pushLiveStack(markObject->behaviour);
        }
    }
    else
    {
        // add this to the live stack so we can call the live() method late.r
        pushLiveStack(markObject);
    }
}


/**
 * Allocate and setup a temporary object obtained via malloc
 * storage.  This is used currently only by the mark routine to
 * expand the size of the live stack during a garbage collection.
 *
 * @param requestLength
 *               The size of the object.
 *
 * @return Storage for creating a new object.
 */
void *MemoryObject::temporaryObject(size_t requestLength)
{
    size_t allocationLength = Memory::roundObjectBoundary(requestLength);
    // allocate just using malloc()
    void *newObj = malloc(allocationLength);
    // there are two times where we can get an allocation failure. When
    // collection objects are allocated, we try to predict if we will need a
    // larger live stack to handle large collections. If we get an allocation
    // failure at that point, we can raise this as a normal error. This is not
    // a perfect process, so we might need to expand the live stack during a
    // a marking operation. A failure at that time is fatal, so we can only
    // handle this as a fatal internal error.
    if (newObj == OREF_NULL)
    {
        // If we're marking, then we can't recover from this.
        if (markingObjects)
        {
            // This is an unrecoverable logic error
            Interpreter::logicError("Unrecoverable out of memory error");
        }
        // a failure during the predictive process, we can raise a real error
        else
        {
            reportException(Error_System_resources);
        }
    }
    return newObj;
}


/**
 * Delete a temporary object.
 *
 * @param storage The storage pointer to release,
 */
void MemoryObject::deleteTemporaryObject(void *storage)
{
    free(storage);
}


/**
 * When we are doing a general marking operation, all
 * objects call markGeneral for all of its object references.
 * We perform different functions based on what the
 * current marking reason is.
 *
 * @param obj    A pointer to the field being marked.  This is
 *               passed as a void * because there are lots of
 *               issues with passing this as a object type.  We
 *               just fake things out and avoid the compile
 *               problems this way.
 *
 */
void MemoryObject::markGeneral(void *obj)
{
    // OK, convert this to a pointer to an Object field, then
    // get the object reference stored there.
    RexxInternalObject **pMarkObject = (RexxInternalObject **)obj;
    RexxInternalObject *markObject = *pMarkObject;

    // NULL references require no processing for any operation.
    if (markObject == OREF_NULL)
    {
        return;
    }

    // have our current marking handler take care of this
    currentMarkHandler->mark(pMarkObject, markObject);
}


/**
 * Place an object on the hold stack to provide some
 * temporary GC protection.
 *
 * @param obj    The object to hold.
 *
 * @return The same object.
 */
RexxInternalObject *MemoryObject::holdObject(RexxInternalObject *obj)
{
   saveStack->push(obj);
   return obj;
}


/**
 * Save the memory image as part of the interpreter
 * build.
 */
void MemoryObject::saveImage(const char *imageTarget)
{
    MemoryStats _imageStats;

    imageStats = &_imageStats;     // set the pointer to the current collector
    _imageStats.clear();

    // get an array to hold all special objects.  This will be the first object
    // copied into the image buffer and allows us to recover all of the important
    // image objects at restore time.
    ArrayClass *saveArray = new_array(saveArray_highest);
    // Note:  A this point, we don't have an activity we can use ProtectedObject to save
    // this with, so we need to use GlobalProtectedObject();
    GlobalProtectedObject p(saveArray);

    // Add all elements needed in
    saveArray->put(TheEnvironment,   saveArray_ENV);
    saveArray->put(TheTrueObject,    saveArray_TRUE);
    saveArray->put(TheFalseObject,   saveArray_FALSE);
    saveArray->put(TheNilObject,     saveArray_NIL);
    saveArray->put(TheNullArray,     saveArray_NULLA);
    saveArray->put(TheNullPointer,   saveArray_NULLPOINTER);
    saveArray->put(TheClassClass,    saveArray_CLASS);
    saveArray->put(PackageManager::getImageData(), saveArray_PACKAGES);
    saveArray->put(TheSystem,       saveArray_SYSTEM);
    saveArray->put(TheCommonRetrievers,    saveArray_COMMON_RETRIEVERS);
    saveArray->put(saveStrings(), saveArray_NAME_STRINGS);
    saveArray->put(TheRexxPackage, saveArray_REXX_PACKAGE);

    // create an array for all of the primitive behaviours and fill it with
    // pointers to our primitive behaviours (only the exported ones need this).
    ArrayClass *primitive_behaviours= (ArrayClass *)new_array(T_Last_Exported_Class + 1);
    for (size_t i = 0; i <= T_Last_Exported_Class; i++)
    {
        primitive_behaviours->put(RexxBehaviour::getPrimitiveBehaviour(i), i + 1);
    }

    // add these to the save array
    saveArray->put(primitive_behaviours, saveArray_PBEHAV);

    // this is make sure we're getting the new set
    bumpMarkWord();

    // create a generic mark handler for this.  We really just
    // want to trace all of the live objects alerting them to the pending
    // image save.
    TracingMarkHandler markHandler(this, markWord);
    setMarkHandler(&markHandler);

    // go to any pre-save pruning/replacement needed by image objects first
    tracingMark(saveArray, PREPARINGIMAGE);

    // reset the marking handler
    resetMarkHandler();

    // now allocate an image buffer and flatten everything hung off of the
    // save array into it.
    char *imageBuffer = (char *)malloc(Memory::MaxImageSize);
    // we save the size of this image at the beginning, so we start
    // the flattening process after that size location.
    size_t imageOffset = sizeof(size_t);
    // bump the mark word to ensure we're going to hit everthing
    bumpMarkWord();

    ImageSaveMarkHandler saveHandler(this, markWord, imageBuffer, imageOffset);
    setMarkHandler(&saveHandler);

    // push a marker on the stack and start the process from the root
    // save object.  This will copy this object to the start of the image
    // buffer so we know where to find it at restore time.
    pushLiveStack(OREF_NULL);

    // this starts the process by marking everything in the save array.
    memory_mark_general(saveArray);

    // now keep popping objects from the stack until we hit the null terminator.
    for (RexxInternalObject *markObject = popLiveStack(); markObject != OREF_NULL; markObject = popLiveStack())
    {
        // The mark of this object moved it to the image buffer.  Its behaviour
        // now contains its offset in the image.  We don't want to mark the original
        // object, but rather the save image copy.
        RexxInternalObject *copyObject = (RexxInternalObject *)(imageBuffer + (uintptr_t)markObject->behaviour);

        // mark any other referenced objects in the copy.
        copyObject->liveGeneral(SAVINGIMAGE);
        // so that we don't store variable pointer values in the image, null out the
        // copy object virtual function pointer now that we're done with it
        copyObject->setVirtualFunctions(NULL);

        // if this is a non-primitive behaviour, we need to mark that also
        // so that it is copied into the buffer.  The primitive behaviours have already
        // been handled as part of the savearray.
        if (copyObject->isNonPrimitive())
        {
            memory_mark_general(copyObject->behaviour);
        }
    }

    resetMarkHandler();

    SysFile image;

    image.open(imageTarget == NULL ? BASEIMAGE : imageTarget, RX_O_CREAT | RX_O_TRUNC | RX_O_WRONLY, RX_S_IREAD | RX_S_IWRITE, RX_SH_DENYRW);

    // place the real size at the beginning of the buffer
    memcpy(imageBuffer, &saveHandler.imageOffset, sizeof(size_t));
    // and finally write this entire image out.
    size_t written = 0;

    image.write(imageBuffer, saveHandler.imageOffset, written);
    image.close();
    free(imageBuffer);

#ifdef MEMPROFILE
    printf("Object stats for this image save are \n");
    _imageStats.printSavedImageStats();
    printf("\n\n Total bytes for this image %zu bytes \n", saveHandler.imageOffset);
#endif
}


/**
 * Perform a general marking operation with a specified
 * reason.  This performs no real operation, but touches
 * all of the objects and traverses the live object tree.
 * Usually used during image save preparation.
 *
 * @param root   The root object to mark from.
 * @param reason The marking reason.
 */
void MemoryObject::tracingMark(RexxInternalObject *root, MarkReason reason)
{
    // push a unique terminator
    pushLiveStack(OREF_NULL);
    // push the live root, and process until we run out of stacked objects.
    memory_mark_general(root);

    for (RexxInternalObject *markObject = popLiveStack();
        markObject != OREF_NULL;        /*   test for unique terminator      */
        markObject = popLiveStack())
    {
        // mark the behaviour first
        memory_mark_general(markObject->behaviour);
        // just call the liveGeneral method on the popped object
        markObject->liveGeneral(reason);
    }
}


/**
 * Perform an in-place unflatten operation on an object
 * in a buffer.
 *
 * @param envelope  The envelope for the unflatten operation.
 * @param sourceBuffer
 *                   The source buffer that contains this data (will need fixups at the end)
 * @param startPointer
 *                   The starting data location in the buffer.
 * @param dataLength The length of the data to unflatten
 *
 * @return The first "real" object in the buffer.
 */
RexxInternalObject *MemoryObject::unflattenObjectBuffer(Envelope *envelope, BufferClass *sourceBuffer, char *startPointer, size_t dataLength)
{
    // get an end pointer
    RexxInternalObject *endPointer = (RexxInternalObject *)(startPointer + dataLength);

    // create the handler that will process the markGeneral calls.
    UnflatteningMarkHandler markHandler(startPointer, markWord);
    setMarkHandler(&markHandler);

    // pointer for addressing a location as an object.  This will also
    // give use the last object we've processed at the end.
    RexxInternalObject *puffObject = (RexxInternalObject *)startPointer;
    // this will be the last object we process in the buffer
    RexxInternalObject *lastObject = OREF_NULL;

    // now traverse the buffer fixing all of the behaviour pointers and having the object
    // mark and fix up their references.
    while (puffObject < endPointer)
    {
        size_t primitiveTypeNum = 0;

        // a non-primitive behaviour.  Then are flattened with the referencing objects.
        if (puffObject->isNonPrimitive())
        {
            // Yes, lets get the behaviour Object...this is stored as an offset at this
            // point that we can turn into a real reference.
            RexxBehaviour *objBehav = (RexxBehaviour *)(startPointer + ((uintptr_t)puffObject->behaviour));
            // Resolve the static behaviour info
            objBehav->resolveNonPrimitiveBehaviour();
            // Set this object's behaviour to a real object.
            puffObject->behaviour = objBehav;
            // get the behaviour's type number
            primitiveTypeNum = objBehav->getClassType();
        }
        else
        {
            // convert this from a type number to the actual class.  This will unnormalize the
            // type number to the different object classes.
            puffObject->behaviour = RexxBehaviour::restoreSavedPrimitiveBehaviour(puffObject->behaviour);
            primitiveTypeNum = puffObject->behaviour->getClassType();
        }

        // Force fix-up of VirtualFunctionTable,
        puffObject->setVirtualFunctions(MemoryObject::virtualFunctionTable[primitiveTypeNum]);
        // mark this object as live
        puffObject->setObjectLive(memoryObject.markWord);
        // Mark other referenced objs
        // Note that this flavor of mark_general should update the
        // mark fields in the objects.
        puffObject->liveGeneral(UNFLATTENINGOBJECT);
        // save the pointer before stepping to the next object so we know the last object.
        lastObject = puffObject;
        // Point to next object in image.
        puffObject = puffObject->nextObject();
    }

    // reset the mark handler to the default
    resetMarkHandler();

    // Prepare to reveal the objects in  the buffer.
    // the first object in the buffer is a dummy added
    // for padding.  We need to step past that one to the
    // beginning of the real unflattened objects
    RexxInternalObject *firstObject = ((RexxInternalObject *)startPointer)->nextObject();

    // this is the location of the next object after the buffer
    RexxInternalObject *nextObject = sourceBuffer->nextObject();
    // this is the size of any tailing buffer portion after the last unflattened object.
    size_t tailSize = (char *)nextObject - (char *)endPointer;

    // lastObject is the last object we processed.  Add any tail data size on to that object
    // so we don't create an invalid gap in the heap.
    lastObject->setObjectSize(lastObject->getObjectSize() + tailSize);

    // now have the memory object traverse this set of objects handling
    // the unflatten calls.
    // Set envelope to the real address of the new objects.  This tells
    // mark_general to send unflatten to run any proxies.
    memoryObject.unflattenProxyObjects(envelope, firstObject, nextObject);

    // now adjust the front portion of the buffer object to reveal all of the
    // unflattened data.  There is a dummy object at the front of the buffer...we want to
    // step to the the first real object
    sourceBuffer->setObjectSize((char *)firstObject - (char *)sourceBuffer);
    // and return the first object
    return firstObject;
}


/**
 * Run the list of objects in an unflattened buffer calling
 * the unflatten() method to perform any proxy/collection
 * table processing.
 *
 * @param envelope  The envelope for the unflatten operation.
 * @param firstObject
 *                  The first object of the buffer.
 * @param endObject The end location for the buffer (actually the first object past the end of the buffer).
 */
void MemoryObject::unflattenProxyObjects(Envelope *envelope, RexxInternalObject *firstObject, RexxInternalObject *endObject)
{
    // switch to an unflattening mark handler.
    EnvelopeMarkHandler markHandler(envelope);
    setMarkHandler(&markHandler);

    // Now traverse the buffer running any proxies.
    while (firstObject < endObject)
    {
        // Since a GC could happen at anytime we need to check to make sure the object
        //  we are going now unflatten is still alive, since all who reference it may have already
        //  run and gotten the info from it and no longer reference it.

        // In theory, this should not be an issue because all of the objects were
        // in the protected set, but unflatten might have cast off some references.
        if (firstObject->isObjectLive(memoryObject.markWord))
        {
            // Note that this flavor of  liveGeneral will run any proxies
            // created by unflatten and fixup  the refs to them.
            firstObject->liveGeneral(UNFLATTENINGOBJECT);
        }

        // Point to next object in image.
        firstObject = firstObject->nextObject();
    }

    // and switch back the mark handler
    resetMarkHandler();
}


/**
 * Update a reference to an object when the target
 * object is a member of the OldSpace.
 *
 * @param oldValue The old field needing updating.
 * @param value    The new value being assigned.
 *
 * @return The assigned object value.
 */
void MemoryObject::setOref(RexxInternalObject *oldValue, RexxInternalObject *value)
{
    // if there is no old2new table, we're doing an image build.  No tracking
    // required then.
    if (old2new != OREF_NULL)
    {
        // the index value is the one assigned there currently.  If this
        // is a newspace value, we should have a table entry with a reference
        // count for it in our table.
        if (oldValue != OREF_NULL && oldValue->isNewSpace())
        {
            // decrement the reference count for this
            old2new->decrement(oldValue);
        }
        // now we have to do this for the new value.
        if (value != OREF_NULL && value->isNewSpace())
        {
            old2new->increment(value);
        }
    }
}


/**
 * Add a global object reference to an object.  The references
 * are counted, so it takes a like number of remove calls
 * to remove the object from the table.
 *
 * @param obj    The object to create a global reference lock on.
 */
void MemoryObject::addGlobalReference(RexxInternalObject *obj)
{
    // increment the count, which will add to the table if this is
    // the first time.
    globalReferences->increment(obj);
}


/**
 * Remove a global object reference to an object.  The
 * references are counted, so it takes a like number of remove
 * calls to remove the object from the table.
 *
 * @param obj    The object to remove global reference lock.
 */
void MemoryObject::removeGlobalReference(RexxInternalObject *obj)
{
    // increment the count, which will remove this from the table if the count
    // goes to zero.
    globalReferences->decrement(obj);
}


/**
 * Dump the image statistics
 */
void MemoryObject::dumpImageStats()
{
    MemoryStats _imageStats;

    _imageStats.clear();
    // gather a fresh set of stats for all of the segments
    newSpaceNormalSegments.gatherStats(&_imageStats, &_imageStats.normalStats);
    newSpaceLargeSegments.gatherStats(&_imageStats, &_imageStats.largeStats);

    _imageStats.printMemoryStats();
}


/**
 * Free all the memory pools currently accessed by this process
 * If not already released.  Then set process Pool to NULL, indicate
 * pools have been released.
 */
void MemoryObject::shutdown()
{
    // free up all of the allocated memory segments
    for (std::vector<MemorySegment *>::iterator it = segments.begin(); it != segments.end(); ++it)
    {
        SystemInterpreter::releaseSegmentMemory(*it);
    }

    // release the livestack also, which is allocated separately.
    delete liveStack;
}


/**
 * Set up the initial memory table.
 *
 * @param old2newTable
 *               The old-to-new tracking table.
 */
void MemoryObject::setUpMemoryTables(MapTable *old2newTable)
{
    // set up the old 2 new table provided for us
    old2new = old2newTable;
    // Now get our savestack
    saveStack = new (Memory::SaveStackSize) PushThroughStack(Memory::SaveStackSize);
}


/**
 * Add a string to the global name table.
 *
 * @param value  The new value to add.
 *
 * @return The single instance of this string.
 */
RexxString *MemoryObject::getGlobalName(const char *value)
{
    // see if we have a global table.  If not collecting currently,
    // just return the non-unique value

    RexxString *stringValue = new_string(value);
    if (globalStrings == OREF_NULL)
    {
        return stringValue;
    }

    // now see if we have this string in the table already
    RexxString *result = (RexxString *)globalStrings->get(stringValue);
    if (result != OREF_NULL)
    {
        return result;                       // return the previously created one
    }
    // add this to the table
    globalStrings->put(stringValue, stringValue);
    return stringValue;              // return the newly created one
}


/**
 * Add a string to the global name table, in uppercase
 *
 * @param value  The new value to add.
 *
 * @return The single instance of this string.
 */
RexxString *MemoryObject::getUpperGlobalName(const char *value)
{
    // see if we have a global table.  If not collecting currently,
    // just return the non-unique value

    RexxString *stringValue = new_upper_string(value);
    if (globalStrings == OREF_NULL)
    {
        return stringValue;                /* just return the string            */
    }

    // now see if we have this string in the table already
    RexxString *result = (RexxString *)globalStrings->get(stringValue);
    if (result != OREF_NULL)
    {
        return result;                       // return the previously created one
    }
    // add this to the table
    globalStrings->put(stringValue, stringValue);
    return stringValue;              // return the newly created one
}


/**
 * Initial memory setup during image build
 */
void MemoryObject::create()
{
    // Now get our savestack
    memoryObject.setUpMemoryTables(OREF_NULL);
}


/**
 * Memory management image restore functions
 */
void MemoryObject::restore()
{
    RESTORE_CLASS(Object, RexxClass);
    RESTORE_CLASS(Class, RexxClass);
                                         // (CLASS is already restored)
    RESTORE_CLASS(String, RexxClass);
    RESTORE_CLASS(Array, RexxClass);
    RESTORE_CLASS(Directory, RexxClass);
    RESTORE_CLASS(Integer, RexxIntegerClass);
    RESTORE_CLASS(List, RexxClass);
    RESTORE_CLASS(Message, RexxClass);
    RESTORE_CLASS(Method, RexxClass);
    RESTORE_CLASS(Routine, RexxClass);
    RESTORE_CLASS(Package, RexxClass);
    RESTORE_CLASS(RexxContext, RexxClass);
    RESTORE_CLASS(NumberString, RexxClass);
    RESTORE_CLASS(Queue, RexxClass);
    RESTORE_CLASS(Stem, RexxClass);
    RESTORE_CLASS(Supplier, RexxClass);
    RESTORE_CLASS(Table, RexxClass);
    RESTORE_CLASS(StringTable, RexxClass);
    RESTORE_CLASS(Set, RexxClass);
    RESTORE_CLASS(Bag, RexxClass);
    RESTORE_CLASS(IdentityTable, RexxClass);
    RESTORE_CLASS(Relation, RexxClass);
    RESTORE_CLASS(MutableBuffer, RexxClass);
    RESTORE_CLASS(Pointer, RexxClass);
    RESTORE_CLASS(Buffer, RexxClass);
    RESTORE_CLASS(WeakReference, RexxClass);
    RESTORE_CLASS(StackFrame, RexxClass);
    RESTORE_CLASS(RexxInfo, RexxClass);
    RESTORE_CLASS(VariableReference, RexxClass);
    RESTORE_CLASS(EventSemaphore, RexxClass);
    RESTORE_CLASS(MutexSemaphore, RexxClass);

    // mark the memory object as old space.
    memoryObject.setOldSpace();
    // initialize the tables used for garbage collection.
    memoryObject.setUpMemoryTables(new MapTable(Memory::DefaultOld2NewSize));

    // initialize the static integer pointers.

    IntegerZero   = new_integer(0);
    IntegerOne    = new_integer(1);
    IntegerTwo    = new_integer(2);
    IntegerThree  = new_integer(3);
    IntegerFour   = new_integer(4);
    IntegerFive   = new_integer(5);
    IntegerSix    = new_integer(6);
    IntegerSeven  = new_integer(7);
    IntegerEight  = new_integer(8);
    IntegerNine   = new_integer(9);
    IntegerMinusOne = new_integer(-1);

    // the activity manager will create the local server, which will use the
    // stream classes.  We need to get the external libraries reloaded before
    // that happens.
    Interpreter::init();
    ActivityManager::init();
    PackageManager::restore();
}


/**
 * Default mark handler mark method.
 *
 * @param field  The field being marked.
 * @param object the object value in the field.
 */
void MarkHandler::mark(RexxInternalObject **field, RexxInternalObject *object)
{
    Interpreter::logicError("Wrong mark routine called");
}


/**
 * pure virtual method for handling the mark operation during an image save.
 *
 * @param pMarkObject
 *                   The pointer to the field being marked.
 * @param markObject The object being marked.
 */
void ImageSaveMarkHandler::mark(RexxInternalObject **pMarkObject, RexxInternalObject *markObject)
{
    // Save image processing.  We only handle this if the object has not
    // already been marked.
    if (!markObject->isObjectLive(markWord))
    {
        // now immediately mark this
        markObject->setObjectLive(markWord);
        // push this object on to the live stack so it's references can be marked later.
        memory->pushLiveStack(markObject);
        // get the size of this object.
        size_t size = markObject->getObjectSize();
        // add this to our image statistics
        memory->logObjectStats(markObject);

        // ok, this is our target copy address
        RexxInternalObject *bufferReference = (RexxInternalObject *)(imageBuffer + imageOffset);
        // we allocated a hard coded buffer, so we need to make sure we don't blow
        // the buffer size.
        if (imageOffset + size > Memory::MaxImageSize)
        {
            Interpreter::logicError("Rexx saved image exceeds expected maximum");
        }

        // copy the object to the image buffer
        memcpy((void *)bufferReference, (void *)markObject, size);
        // clear the mark in the copy so we're clean in restore
        bufferReference->clearObjectMark();
        // now get the behaviour object
        RexxBehaviour *behaviour = bufferReference->behaviour;
        // if this is a non primitive behaviour, we need to mark the
        // copy object apprpriately (this uses the live mark)
        if (behaviour->isNonPrimitive())
        {
            bufferReference->setNonPrimitive();
        }
        else
        {
            // double check that we're not accidentally saving a transient class.
            if (behaviour->isTransientClass())
            {
                Interpreter::logicError("Transient class included in image buffer");
            }

            // clear this out, as this is overloaded with the oldspace flag.
            bufferReference->setPrimitive();
            // replace behaviour with normalized type number
            bufferReference->behaviour = behaviour->getSavedPrimitiveBehaviour();
        }

        // replace the behaviour in the original object with the image offset.
        // this is a destructive operation, but we're not going to be running any
        // Rexx code at this point, so this is fine.
        markObject->behaviour = (RexxBehaviour *)imageOffset;
        // now update our image offset with the next object location.
        imageOffset += size;
    }

    // now update the field reference with the moved offset location.
    *pMarkObject = (RexxInternalObject *)markObject->behaviour;
}
