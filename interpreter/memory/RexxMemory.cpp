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
/* Kernel                                                     RexxMemory.cpp  */
/*                                                                            */
/* Memory Object                                                              */
/*                                                                            */
/******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "RexxCore.h"
#include "RexxMemory.hpp"
#include "StackClass.hpp"
#include "StringClass.hpp"
#include "MutableBufferClass.hpp"
#include "DirectoryClass.hpp"
#include "RexxActivity.hpp"
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
#include "ExceptionClass.hpp"
#include "Interpreter.hpp"
#include "SystemInterpreter.hpp"
#include "Interpreter.hpp"
#include "PackageManager.hpp"
#include "SysFileSystem.hpp"
#include "UninitDispatcher.hpp"

// restore a class from its
// associated primitive behaviour
// (already restored by memory_init)
#define RESTORE_CLASS(name, className) The##name##Class = (className *)RexxBehaviour::getPrimitiveBehaviour(T_##name)->restoreClass();


bool SysAccessPool(MemorySegmentPool **pool);
/* NOTE:  There is just a single memory object in global storage.  We'll define      */
/* memobj to be the direct address of this memory object.                            */
RexxMemory memoryObject;

#define LiveStackSize  16370         /* live stack size                   */

#define SaveStackSize 10             /* newly created objects to save */
#define SaveStackAllocSize 500       /* pre-allocation for save stack  */

#define MaxImageSize 1800000         /* maximum startup image size */

RexxDirectory *RexxMemory::globalStrings = OREF_NULL;
RexxDirectory *RexxMemory::environment = OREF_NULL;       // global environment
RexxDirectory *RexxMemory::functionsDir = OREF_NULL;      // statically defined requires
RexxDirectory *RexxMemory::commonRetrievers = OREF_NULL;
RexxDirectory *RexxMemory::kernel = OREF_NULL;
RexxDirectory *RexxMemory::system = OREF_NULL;

// locks for the memory process access
SysMutex RexxMemory::flattenMutex;
SysMutex RexxMemory::unflattenMutex;
SysMutex RexxMemory::envelopeMutex;

static void logMemoryCheck(FILE *outfile, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    if (outfile != NULL) {
        vfprintf(outfile, message, args);
    }
    va_end(args);
}


RexxMemory::RexxMemory()
/******************************************************************************/
/* Function: Main Constructor for Rexxmemory, called once during main         */
/*  initialization.  Will create the initial memory Pool(s), etc.             */
/******************************************************************************/
{
    /* we need to set a valid size for this object.  We round it up */
    /* to the minimum allocation boundary, even though that might be */
    /* a lie.  Since this never participates in a sweep operation, */
    /* this works ok in the end. */
    this->setObjectSize(roundObjectBoundary(sizeof(RexxMemory)));
    // our first pool is the current one
    currentPool = firstPool;

    /* make sure we hand craft memory's  */
    /*  header information.              */

    /* See if we want this debug info    */
    /*or not by default, its always      */
    /*setable from OREXX code.           */
#ifdef CHECKOREFS
    this->orphanCheck = true;            /* default value for OREF checking   */
#else
    this->orphanCheck = false;           /* default value for OREF checking   */
#endif
    /* OR'ed into object headers to      */
    /*mark                               */
    this->markWord = 1;
    /* objects as being alive in mark    */
    /*phase                              */
    this->saveStack = NULL;
    this->saveTable = NULL;
    this->dumpEnable = false;
    this->objOffset = 0;
    this->envelope = NULL;

    // we always start out with an empty list.  WeakReferences that are in the
    // saved image will (MUST) never be set to a new value, so it's not necessary
    // to hook those back up again.
    weakReferenceList = OREF_NULL;
}


void RexxMemory::initialize(bool _restoringImage)
/******************************************************************************/
/* Function:  Gain access to all Pools                                        */
/******************************************************************************/
{
    /* access 1st pool directly. SysCall */
    /* Did the pool exist?               */

    firstPool = MemorySegmentPool::createPool();
    currentPool = firstPool;

    disableOrefChecks();                 /* Make sure we don't try to validate*/
                                         /*  OrefSets just yet.               */
                                         /* Make sure memory is fully         */
                                         /*constructed, mainlyt a concern on  */
                                         /*2nd entry and DLL not unloaded.    */
    new (this) RexxMemory;
    new (&newSpaceNormalSegments) NormalSegmentSet(this);
    new (&newSpaceLargeSegments) LargeSegmentSet(this);

    /* and the new/old Space segments    */
    new (&oldSpaceSegments) OldSpaceSegmentSet(this);

    collections = 0;
    allocations = 0;
    variableCache = OREF_NULL;
    globalStrings = OREF_NULL;

    // get our table of virtual functions setup first thing.
    buildVirtualFunctionTable();

    /* NOTE: we don't set livestack      */
    /*via the  OrefSet macro, since we   */
    /*are so early on in the system,     */
    /*that we can't use it right now,    */
    /*we will fix this all               */
    liveStack = (RexxStack *)oldSpaceSegments.allocateObject(SegmentDeadSpace);
    /* remember the original one         */
    originalLiveStack = liveStack;

    if (_restoringImage)                 /* restoring the image?              */
    {
        restoreImage();                  /* do it now...                      */
    }

    /* set memories behaviour */
    memoryObject.setBehaviour(TheMemoryBehaviour);
    /* initial marktable value is        */
    /* TheKernel                         */
    this->markTable = OREF_NULL;         /* fix by CHM/Rick: set initial table*/
                                         /* to NULL since TheKernel could     */
                                         /* point to an invalid memory address*/
                                         /* if one OREXX session is started   */
                                         /* while another one is closed       */

    /* make sure we have an inital segment set to allocate from. */
    newSpaceNormalSegments.getInitialSet();

    // get the initial uninit table
    uninitTable = new_identity_table();

    // is this image creation?  This will build and save the image, then
    // terminate
    if (!_restoringImage)
    {
        createImage();
    }
    restore();                           // go restore the state of the memory object
}


void RexxMemory::logVerboseOutput(const char *message, void *sub1, void *sub2)
/******************************************************************************/
/* Function:  Log verbose output events                                       */
/******************************************************************************/
{
    logMemoryCheck(NULL, message, sub1, sub2);
}

void RexxMemory::dumpMemoryProfile()
{
    FILE *outFile;                       /* memory stat output file           */

    outFile = fopen("memory.prf","wb");  /* open the file                     */
    /* have each of the memory segments dump their own profile information */
    newSpaceNormalSegments.dumpMemoryProfile(outFile);
    newSpaceLargeSegments.dumpMemoryProfile(outFile);
    fclose(outFile);                     /* close the file                    */
}


void RexxMemory::dumpObject(RexxObject *objectRef, FILE *outfile)
/******************************************************************************/
/* Arguments:  none,                                                          */
/*                                                                            */
/*  Function: Dump out the contents of an object, to screen and optionally    */
/*       a file.                                                              */
/******************************************************************************/
{
    void    **dmpPtr;
    void    **ObjEnd;

    ObjEnd = (void    **)((char *)objectRef + objectRef->getObjectSize());
    for (dmpPtr = (void **)objectRef; dmpPtr <= ObjEnd ; dmpPtr++)
    {
        logMemoryCheck(outfile, "  >Parent Dump -->%p   %p   %p   %p \n", *dmpPtr, *(dmpPtr+1), *(dmpPtr+2), *(dmpPtr+3));
    }
}

bool RexxMemory::inSharedObjectStorage(RexxObject *object)
/******************************************************************************/
/* Arguments:  Any OREF                                                       */
/*                                                                            */
/*  Returned:  true if object is in object storage, else false                */
/******************************************************************************/
{
    /* 1st Check Old Space. */
    if (oldSpaceSegments.isInSegmentSet(object))
    {
        return true;
    }

    /* Now Check New Space.              */
    if (newSpaceNormalSegments.isInSegmentSet(object))
    {
        return true;
    }

    /* Now Check New Space large segments*/
    if (newSpaceLargeSegments.isInSegmentSet(object))
    {
        return true;
    }

    /* this is bad, definitely very bad... */
    return false;
}


bool RexxMemory::inObjectStorage(RexxObject *object)
/******************************************************************************/
/* Arguments:  Any OREF                                                       */
/*                                                                            */
/*  Returned:  true if object is in object storage, else false                */
/******************************************************************************/
{
    /* check for a few valid locations in 'C' storage                     */
    if ((object >= (RexxObject *)RexxBehaviour::getPrimitiveBehaviour(0) && object <= (RexxObject *)RexxBehaviour::getPrimitiveBehaviour(T_Last_Class_Type)) ||
        (object == (RexxObject *)this))
    {
        return true;
    }

    return inSharedObjectStorage(object);
}


/* object validation method --used to find and diagnose broken object references       */
bool RexxMemory::objectReferenceOK(RexxObject *o)
/******************************************************************************/
/* Function:  Object validation...used to find and diagnose broken object     */
/* references.                                                                */
/******************************************************************************/
{
    if (!inObjectStorage(o))
    {
        return false;
    }
    RexxBehaviour *type = o->getObjectType();
    if (inObjectStorage((RexxObject *)type) && type->getObjectType() == TheBehaviourBehaviour)
    {
        return true;
    }
    /* these last two checks are for very early checking...we can */
    /* possibly be testing this before TheBehaviourBehaviour is */
    /* set up, so we have two additional means of verifying the */
    /* behaviour object. */
    return type->isObjectType(T_Behaviour) || type == RexxBehaviour::getPrimitiveBehaviour(T_Behaviour);
}


void RexxMemory::markObjectsMain(RexxObject *rootObject)
/******************************************************************************/
/* Function:  Main memory_mark driving loop                                   */
/******************************************************************************/
{
    // for some of the root objects, we get called to mark them before they get allocated.
    // make sure we don't process any null references.
    if (rootObject == OREF_NULL)
    {
        return;
    }

    RexxObject *markObject;

    // set up the live marking word passed to the live() routines
    size_t liveMark = markWord | OldSpaceBit;

    allocations = 0;
    pushLiveStack(OREF_NULL);            /* push a unique terminator          */
    mark(rootObject);                    /* OREF_ENV or old2new               */
    for (markObject = popLiveStack();
        markObject != OREF_NULL;        /* test for unique terminator        */
        markObject = popLiveStack())
    {
        /* mark behaviour live               */
        memory_mark((RexxObject *)markObject->behaviour);
        /* Mark other referenced obj.  We can do this without checking */
        /* the references flag because we only push the object on to */
        /* the stack if it has references. */
        allocations++;
        markObject->live(liveMark);
    }
}


void  RexxMemory::killOrphans(RexxObject *rootObject)
/******************************************************************************/
/* Function:  Garbage collection validity check routine                       */
/******************************************************************************/
{
    // for some of the root objects, we get called to mark them before they get allocated.
    // make sure we don't process any null references.
    if (rootObject == OREF_NULL)
    {
        return;
    }

    RexxObject *mref;

    markReason = LIVEMARK;
    /* This is the debugging mark code that traverses the object tree from          */
    /* OREF_ENV, verifying that the objects and their OREFs look OK, specifically   */
    /* that each one is in object storage, and has a behaviour whose behaviour      */
    /* is OREF_BEHAV_B, behaviour's behaviour.                                      */

    /* push a unique terminator          */
    pushLiveStack(OREF_NULL);
    /*Push an extra marktable            */
    pushLiveStack(rootObject);
    memory_mark_general(rootObject);     /* start from the very tip-top       */
    memory_mark_general(TheNilObject);   /* use .nil to mark the stack        */
                                         /* mark .nil behaviour live          */
    memory_mark_general(TheNilObject->behaviour);
    /* mark .nil ovds      live          */
    memory_mark_general(TheNilObject->objectVariables);
    for (mref = popLiveStack();
        mref != OREF_NULL;              /* test for unique terminator        */
        /* get the next marked item          */
        mref = popLiveStack())
    {

        /* OREF_NIL is used as a marker on the stack.  These markers will be used to  */
        /* recreate the ancestry of any broken object found.  At this point though,   */
        /* finding one indicates that we've already marked all of the descendants of  */
        /* the object just below the OREF_NIL, so we'll leave the OREF_NIL popped,    */
        /* and pop off the parent object which is no longer of any interest.          */
        if (mref == TheNilObject)
        {        /* found a marker                    */
                 /* pop off the completed parent      */
            popLiveStack();
            continue;                       /* and back to top of loop           */
        }
        /* mark behaviour live               */
        memory_mark_general(mref->behaviour);
        /* object have any references?       */
        if (mref->hasReferences())
        {
            /* Though this guy is already marked, and as far as gc, we're done with him,    */
            /* we'll now push him back onto the livestack followed by an OREF_NIL marker.   */
            /* If we find an invalid object later, we'll be able to find all the ancestors  */
            /* by looking for the OREF_NIL objects on the livestack.                        */
            /* push the ancestor object          */
            pushLiveStack(mref);
            /* push an ancestor marker           */
            pushLiveStack(TheNilObject);
            mref->liveGeneral(LIVEMARK);    /* tell the ancestor to mark his kids*/
        }                                  /* object have any references?      */
    }
}

void RexxMemory::checkUninit()
/******************************************************************************/
/*                                                                            */
/******************************************************************************/
{
    /* we might not actually have a table yet, so make sure we check */
    /* before using it. */
    if (uninitTable == NULL)
    {
        return;
    }

    RexxObject *uninitObject;
    /* table and any object is isn't   */
    /* alive, we indicate it should be */
    /* sent unInit.  We indiacte this  */
    /* by setting the value to 1,      */
    /* instead of NIL (the default)    */
    for (HashLink i = uninitTable->first(); (uninitObject = uninitTable->index(i)) != OREF_NULL; i = uninitTable->next(i))
    {
        /* is this object now dead?        */
        if (uninitObject->isObjectDead(markWord))
        {
            /* yes, indicate object is to be   */
            /*  sent uninit.                   */
            uninitTable->replace(TheTrueObject, i);
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
void RexxMemory::collectAndUninit(bool clearStack)
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
void RexxMemory::lastChanceUninit()
{
    // collect and run any uninits still pending
    collectAndUninit(true);
    // we're about to start releasing libraries, so it is critical
    // we don't run any more uninits after this
    uninitTable->empty();
}


void  RexxMemory::runUninits()
/******************************************************************************/
/* Function:  Run any UNINIT methods for this activity                        */
/******************************************************************************/
/* NOTE: The routine to iterate across uninit Table isn't quite right, since  */
/*  the removal of zombieObj may move another zombieObj and then doing        */
/*  the next will skip this zombie, we should however catch it next time      */
/*  through.                                                                  */
/*                                                                            */
/******************************************************************************/
{
    RexxObject * zombieObj;              /* obj that needs uninit run.        */
    HashLink iterTable;                  /* iterator for table.               */

    /* if we're already processing this, don't try to do this */
    /* recursively. */
    if (processingUninits)
    {
        return;
    }

    /* turn on the recursion flag, and also zero out the count of */
    /* pending uninits to run */
    processingUninits = true;
    pendingUninits = 0;

    // get the current activity for running the uninits
    RexxActivity *activity = ActivityManager::currentActivity;

    /* uninitTabe exists, run UNINIT     */
    for (iterTable = uninitTable->first();
        (zombieObj = uninitTable->index(iterTable)) != OREF_NULL;)
    {
        // TODO:  Ther's a bug here.  Removing the object can cause the
        // iterator to skip over an entry....something should be done to
        // prevent this.

        /* is this object readyfor UNINIT?   */
        if (uninitTable->value(iterTable) == TheTrueObject)
        {
            /* make sure we don't recurse        */
            uninitTable->put(TheFalseObject, zombieObj);
            {
                // run this method with appropriate error trapping
                UninitDispatcher dispatcher(zombieObj);
                activity->run(dispatcher);
            }
                                           /* remove zombie from uninit table   */
            uninitTable->remove(zombieObj);


            // because we just did a remove operation, this will effect the iteration
            // process. There are two possibilities here.  Either A) we were at the end of the
            // chain and this is now an empty slot or B) the removal process moved an new item
            // into this slot.  If it is case A), then we need to search for the next item.  If
            // it is case B) we'll just leave the index alone and process this position again.
            if (uninitTable->index(iterTable) == OREF_NULL)
            {
                iterTable = uninitTable->next(iterTable);
            }
        }
        else
        {
            iterTable = uninitTable->next(iterTable);
        }
    }                                  /* now go check next object in table */
    /* make sure we remove the recursion protection */
    processingUninits = false;
}


void  RexxMemory::removeUninitObject(
    RexxObject *obj)                   /* object to remove                  */
/******************************************************************************/
/* Function:  Remove an object from the uninit tables                         */
/******************************************************************************/
{
    // just remove this object from the table
    uninitTable->remove(obj);
}


void RexxMemory::addUninitObject(
    RexxObject *obj)                   /* object to add                     */
/******************************************************************************/
/* Function:  Add an object with an uninit method to the uninit table for     */
/*            a process                                                       */
/******************************************************************************/
{
                                       /* is object already in table?       */
   if (uninitTable->get(obj) == OREF_NULL)
   {
                                       /* nope, add obj to uninitTable,     */
                                       /*  initial value is NIL             */
       uninitTable->put(TheNilObject, obj);
   }

}

bool RexxMemory::isPendingUninit(RexxObject *obj)
/******************************************************************************/
/* Function:  Test if an object is going to require its uninit method run.    */
/******************************************************************************/
{
    return uninitTable->get(obj) != OREF_NULL;
}


void RexxMemory::markObjects()
/******************************************************************************/
/* Function:   Main mark routine for garbage collection.  This reoutine       */
/*  Determines which mark routine to call and does all additional processing  */
/******************************************************************************/
{
    verboseMessage("Beginning mark operation\n");

    if (this->orphanCheck)
    {             /* debugging bad OREF's?             */
                  /* yup, call debugging mark          */
        this->killOrphans((RexxObject *)this);
        // now process the weak reference queue...We check this before the
        // uninit list is processed so that the uninit list doesn't mark any of the
        // weakly referenced items.  We don't want an object placed on the uninit queue
        // to end up strongly referenced later.
        checkWeakReferences();
        this->checkUninit();               /* flag all objects about to be dead */
        this->killOrphans(uninitTable);    /* the uninit table                  */
    }
    else
    {
        /* call normal,speedy,efficient mark */
        this->markObjectsMain((RexxObject *)this);
        // now process the weak reference queue...We check this before the
        // uninit list is processed so that the uninit list doesn't mark any of the
        // weakly referenced items.  We don't want an object placed on the uninit queue
        // to end up strongly referenced later.
        checkWeakReferences();
        this->checkUninit();               /* flag all objects about to be dead */
                                           /* now mark the unInit table and the */
        this->markObjectsMain(uninitTable);
    }
    /* have to expand the live stack?    */
    if (this->liveStack != this->originalLiveStack)
    {
        free((void *)this->liveStack);     /* release the old one               */
                                           /* and set back to the original      */
        this->liveStack = this->originalLiveStack;
    }
    verboseMessage("Mark operation completed\n");
}


/******************************************************************************/
/* Function:  Scan the weak reference queue looking for either dead weak      */
/* objects or weak references that refer to objects that have gone out of     */
/* scope.  Objects with registered notification objects (that are still in    */
/* scope) will be moved to a notification queue, which is processed after     */
/* everything is scanned.                                                     */
/******************************************************************************/
void RexxMemory::checkWeakReferences()
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
        // step to the new nest item
        current = next;
    }

    // update the list
    weakReferenceList = newList;
}


void RexxMemory::addWeakReference(WeakReference *ref)
/******************************************************************************/
/* Function:  Add a new weak reference to the tracking table                  */
/******************************************************************************/
{
    // just add this to the front of the list
    ref->nextReferenceList = weakReferenceList;
    weakReferenceList = ref;
}


MemorySegment *RexxMemory::newSegment(size_t requestedBytes, size_t minBytes)
/******************************************************************************/
/* Function:  Allocate a segment of the requested size.  The requested size   */
/* is the desired size, while the minimum is the absolute minimum we can      */
/* handle.  This takes care of the overhead accounting and additional         */
/* rounding.  The requested size is assumed to have been rounded up to the    */
/* next "appropriate" boundary already, and the segment overhead will be      */
/* allocated from that part, if possible.  Otherwise, and additional page is  */
/* added.                                                                     */
/******************************************************************************/
{
    MemorySegment *segment;

#ifdef MEMPROFILE
    printf("Allocating a new segment of %d bytes\n", requestedBytes);
#endif
    /* first make sure we've got enough space for the control */
    /* information, and round this to a proper boundary */
    requestedBytes = roundSegmentBoundary(requestedBytes + MemorySegmentOverhead);
#ifdef MEMPROFILE
    printf("Allocating boundary a new segment of %d bytes\n", requestedBytes);
#endif
    /*Get a new segment                  */
    segment = currentPool->newSegment(requestedBytes);
    /* Did we get a segment              */
    if (segment == NULL)
    {
        /* Segmentsize is the minimum size request we handle.  If */
        /* minbytes is small, then we're just adding a segment to the */
        /* small pool.  Reduce the request to SegmentSize and try again. */
        /* For all other requests, try once more with the minimum. */
        minBytes = roundSegmentBoundary(minBytes + MemorySegmentOverhead);
        /* try to allocate once more...if this fails, the caller will */
        /* have to handle it. */
        segment = currentPool->newSegment(minBytes);
    }
    return segment;                      /* return the allocated segment      */
}


MemorySegment *RexxMemory::newLargeSegment(size_t requestedBytes, size_t minBytes)
/******************************************************************************/
/* Function:  Allocate a segment of the requested size.  The requested size   */
/* is the desired size, while the minimum is the absolute minimum we can      */
/* handle.  This takes care of the overhead accounting and additional         */
/* rounding.  The requested size is assumed to have been rounded up to the    */
/* next "appropriate" boundary already, and the segment overhead will be      */
/* allocated from that part, if possible.  Otherwise, and additional page is  */
/* added.                                                                     */
/******************************************************************************/
{
    MemorySegment *segment;

    /* first make sure we've got enough space for the control */
    /* information, and round this to a proper boundary */
    size_t allocationBytes = roundSegmentBoundary(requestedBytes + MemorySegmentOverhead);
#ifdef MEMPROFILE
    printf("Allocating large boundary new segment of %d bytes for request of %d\n", allocationBytes, requestedBytes);
#endif
    /*Get a new segment                  */
    segment = currentPool->newLargeSegment(allocationBytes);
    /* Did we get a segment              */
    if (segment == NULL)
    {
        /* Segmentsize is the minimum size request we handle.  If */
        /* minbytes is small, then we're just adding a segment to the */
        /* small pool.  Reduce the request to SegmentSize and try again. */
        /* For all other requests, try once more with the minimum. */
        minBytes = roundSegmentBoundary(minBytes + MemorySegmentOverhead);
        /* try to allocate once more...if this fails, the caller will */
        /* have to handle it. */
        segment = currentPool->newLargeSegment(minBytes);
    }
    return segment;                      /* return the allocated segment      */
}


void RexxMemory::restoreImage()
/******************************************************************************/
/* Function:  Restore a saved image to usefulness.                            */
/******************************************************************************/
{
    // Nothing to restore if we have a buffer already
    if (image_buffer != NULL)
    {
        return;
    }

    markReason = RESTORINGIMAGE;         // we're doing an image restore

    size_t imagesize;                    /* size of the image                 */
    char *objectPointer, *endPointer;
    RexxBehaviour *imageBehav;           /*behaviour of OP object in image    */
    RexxArray  *primitiveBehaviours;     /* saved array of behaviours         */
    size_t primitiveTypeNum;
    int i;

    /* go load the image */
    SystemInterpreter::loadImage(&image_buffer, &imagesize);
    /* get the relocation factor         */
    relocation = (size_t)image_buffer - sizeof(size_t);
    objectPointer = image_buffer;      /* address the start of the image    */
                                       /* and the ending location           */
    endPointer = image_buffer + imagesize;
    /* the save Array is the 1st object  */
    RexxArray *saveArray = (RexxArray *)image_buffer;
    restoreimage = true;               /* we are now restoring              */
                                       /* loop through the image buffer     */
    while (objectPointer < endPointer)
    {

        /*Fixup Behaviour pointer for obj.   */
        /* We don't do an OrefSet here, since*/
        /* too early, and we don't need to be*/
        /* concerned about object in pbehav  */

        /* Retrieve the behaviour obj        */
        /* or a copy?                        */
        if (((RexxObject *)objectPointer)->isNonPrimitive())
        {
            /* Working with a copy, don't use    */
            /* PBEHAV version.                   */
            /* Make sure static behaviour inf    */
            /* is resolved before using the      */
            /* Behaviour.                        */
            imageBehav = (RexxBehaviour *)(relocation + (uintptr_t)((RexxObject *)objectPointer)->behaviour);
            /* give this object it behaviour.    */
            ((RexxObject *)objectPointer)->behaviour = (RexxBehaviour *)imageBehav;
            primitiveTypeNum = imageBehav->getClassType();
        }
        else
        {
            // the original behaviour pointer has been encoded as a type number and class
            // category to allow us to convert back to the appropriate type.
            ((RexxObject *)objectPointer)->behaviour = RexxBehaviour::restoreSavedPrimitiveBehaviour(((RexxObject *)objectPointer)->behaviour);
            primitiveTypeNum = ((RexxObject *)objectPointer)->behaviour->getClassType();
        }
        /* This will be an OldSpace object.  We delay setting this */
        /* until now, because the oldspace bit is overloaded with the */
        /* NonPrimitive bit.  Since the we are done with the */
        /* non-primitive bit now, we can use this for the oldspace */
        /* flag too. */
        ((RexxObject *)objectPointer)->setOldSpace();
        /* Force fix-up of                   */
        /*VirtualFunctionTable,              */
        ((RexxObject *)objectPointer)->setVirtualFunctions(virtualFunctionTable[primitiveTypeNum]);

        /* Do this object have any           */
        /*references?                        */
        if (((RexxObject *)objectPointer)->hasReferences())
            /*  Yes, mark other referenced objs  */
            ((RexxObject *)objectPointer)->liveGeneral(RESTORINGIMAGE);
        /* Point to next object in image..   */
        objectPointer += ((RexxObject *)objectPointer)->getObjectSize();

    }

    restoreimage = false;
    /* OREF_ENV is the first element of  */
    /*array                              */
    TheEnvironment = (RexxDirectory *)saveArray->get(saveArray_ENV);
    /* get the primitive behaviours      */
    primitiveBehaviours = (RexxArray *)saveArray->get(saveArray_PBEHAV);
    /* restore all of the saved primitive*/
    for (i = 0; i <= T_Last_Exported_Class; i++)
    {
        /* behaviours into this array        */
        RexxBehaviour::primitiveBehaviours[i].restore((RexxBehaviour *)primitiveBehaviours->get(i + 1));
    }

    TheKernel      = (RexxDirectory *)saveArray->get(saveArray_KERNEL);
    TheSystem      = (RexxDirectory *)saveArray->get(saveArray_SYSTEM);
    TheFunctionsDirectory = (RexxDirectory *)saveArray->get(saveArray_FUNCTIONS);
    TheTrueObject  = (RexxInteger *)saveArray->get(saveArray_TRUE);
    TheFalseObject = (RexxInteger *)saveArray->get(saveArray_FALSE);
    TheNilObject   = saveArray->get(saveArray_NIL);
    TheNullArray   = (RexxArray *)saveArray->get(saveArray_NULLA);
    TheNullPointer   = (RexxPointer *)saveArray->get(saveArray_NULLPOINTER);
    TheClassClass  = (RexxClass *)saveArray->get(saveArray_CLASS);
    TheCommonRetrievers = (RexxDirectory *)saveArray->get(saveArray_COMMON_RETRIEVERS);

    /* restore the global strings        */
    memoryObject.restoreStrings((RexxArray *)saveArray->get(saveArray_NAME_STRINGS));
    // make sure we have a working thread context
    RexxActivity::initializeThreadContext();
    PackageManager::restore((RexxArray *)saveArray->get(saveArray_PACKAGES));
}


void RexxMemory::live(size_t liveMark)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/*                                                                            */
/******************************************************************************/
{
  /* Mark the save stack first, since it will be pulled off of */
  /* the stack after everything else.  This will give other */
  /* objects a chance to be marked before we remove them from */
  /* the savestack. */
  memory_mark(this->saveStack);
  memory_mark(this->saveTable);
  memory_mark(this->old2new);
  memory_mark(this->envelope);
  memory_mark(this->variableCache);
  memory_mark(this->markTable);
  memory_mark(globalStrings);
  // now call the various subsystem managers to mark their references
  Interpreter::live(liveMark);
  SystemInterpreter::live(liveMark);
  ActivityManager::live(liveMark);
  PackageManager::live(liveMark);
}

void RexxMemory::liveGeneral(int reason)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/*                                                                            */
/******************************************************************************/
{
  memory_mark_general(this->saveStack);/* Mark the save stack last, to give it a chance to clear out entries */
  memory_mark_general(this->saveTable);
  memory_mark_general(this->old2new);
  memory_mark_general(this->envelope);
  memory_mark_general(this->variableCache);
  memory_mark_general(this->markTable);
  memory_mark_general(globalStrings);
  // now call the various subsystem managers to mark their references
  Interpreter::liveGeneral(reason);
  SystemInterpreter::liveGeneral(reason);
  ActivityManager::liveGeneral(reason);
  PackageManager::liveGeneral(reason);
}

void RexxMemory::flatten(RexxEnvelope *env)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  OREF NULL,                                                     */
/******************************************************************************/
{

}

RexxObject  *RexxMemory::makeProxy(RexxEnvelope *env)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  OREF NULL,                                                     */
/******************************************************************************/
{
                                       /* Memory Obj doesn't get moved,     */
                                       /*create a proxy object to           */
                                       /*re-establish on other system.      */


                                       /* Generate a new proxy object to    */
                                       /*represent this object, and then    */
                                       /*send flatten to this proxy.  We    */
                                       /*then return the result from the    */
                                       /*proxt flatten method.              */
  return (RexxObject *)new_proxy("MEMORY");
}


RexxObject* RexxMemory::reclaim()
/******************************************************************************/
/* Function:  Interface method for external control of garbage collection.    */
/* This forces a GC cycle.                                                    */
/******************************************************************************/
{
    /* just drive the GC operation */
    collect();
    return OREF_NULL;
}


void RexxMemory::collect()
/******************************************************************************/
/* Function:  Collect all dead memory in the Rexx object space.  The          */
/* collection process performs a mark operation to mark all of the live       */
/* objects, followed by sweep of each of the segment sets.                    */
/******************************************************************************/
{
    collections++;
    verboseMessage("Begin collecting memory, cycle #%d after %d allocations.\n", collections, allocations);
    allocations = 0;

    /* change our marker to the next value so we can distinguish */
    /* between objects marked on this cycle from the objects marked */
    /* in the pervious cycles. */
    bumpMarkWord();

    /* do the object marking now...followed by a sweep of all of the */
    /* segments. */
    markObjects();
    newSpaceNormalSegments.sweep();
    newSpaceLargeSegments.sweep();

    /* The space segments are now in a known, completely clean state. */
    /* Now based on the context that caused garbage collection to be */
    /* initiated, the segment sets may be expanded to add additional */
    /* free memory.  The decision to expand the object space requires */
    /* the usage statistics collected by the mark-and-sweep */
    /* operation. */

    verboseMessage("End collecting memory\n");
    verboseMessage("Object save table contains %d objects\n", this->saveTable->items());
}

RexxObject *RexxMemory::oldObject(size_t requestLength)
/******************************************************************************/
/* Arguments:  Requested length                                               */
/*                                                                            */
/*  Returned:  New object, or OREF_NULL if requested length was too large     */
/******************************************************************************/
{
    /* Compute size of new object and determine where we should */
    /* allocate from */
    requestLength = roundObjectBoundary(requestLength);
    RexxObject *newObj = oldSpaceSegments.allocateObject(requestLength);

    /* if we got a new object, then perform the final setup steps. */
    /* Since the oldspace objects are special, we don't push them on */
    /* to the save stack.  Also, we don't set the oldspace flag, as */
    /* those are a separate category of object. */
    if (newObj != OREF_NULL)
    {
        // initialize the hash table object
        newObj->initializeNewObject(requestLength, markWord, virtualFunctionTable[T_Object], TheObjectBehaviour);
    }

    /* return the newly allocated object to our caller */
    return newObj;
}

char *RexxMemory::allocateImageBuffer(size_t imageSize)
/******************************************************************************/
/* Function:  Allocate an image buffer for the system code.  The image buffer */
/* is allocated in the oldspace segment set.  We create an object from that   */
/* space, then return this object as a character pointer.  We eventually will */
/* get that pointer passed back to us as the image address.                   */
/******************************************************************************/
{
    return (char *)oldObject(imageSize);
}


RexxObject *RexxMemory::newObject(size_t requestLength, size_t type)
/******************************************************************************/
/* Arguments:  Requested length                                               */
/*                                                                            */
/*  Returned:  New object, or OREF_NULL if requested length was too large     */
/******************************************************************************/
{
    RexxObject *newObj;

    allocations++;

    /* Compute size of new object and determine where we should */
    /* allocate from */
    requestLength = roundObjectBoundary(requestLength);

    /* is this a typical small object? */
    if (requestLength <= LargeBlockThreshold)
    {
        /* make sure we don't go below our minimum size. */
        if (requestLength < MinimumObjectSize)
        {
            requestLength = MinimumObjectSize;
        }
        newObj = newSpaceNormalSegments.allocateObject(requestLength);
        /* feat. 1061 moves the handleAllocationFailure code into the initial */
        /* allocation parts; this way an "if" instruction can be saved.       */
        if (newObj == NULL)
        {
            newObj = newSpaceNormalSegments.handleAllocationFailure(requestLength);
        }
    }
    /* we keep the big objects in a separate cage */
    else
    {
        /* round this allocation up to the appropriate large boundary */
        requestLength = roundLargeObjectAllocation(requestLength);
        newObj = newSpaceLargeSegments.allocateObject(requestLength);
        if (newObj == NULL)
        {
            newObj = newSpaceLargeSegments.handleAllocationFailure(requestLength);
        }
    }

    newObj->initializeNewObject(markWord, virtualFunctionTable[type], RexxBehaviour::getPrimitiveBehaviour(type));

    if (this->saveStack != OREF_NULL)
    {
        /* saveobj doesn't get turned on     */
        /*until the system is initialized    */
        /*far enough but once its on, push   */
        /*this new obj on the save stack to  */
        /*keep him from being garbage        */
        /*collected, before it can be used   */
        /*and safely anchored by caller.     */
        pushSaveStack(newObj);
    }
    /* return the newly allocated object to our caller */
    return newObj;
}


RexxArray  *RexxMemory::newObjects(
                size_t         size,
                size_t         count,
                size_t         objectType)
/******************************************************************************/
/* Arguments:  size of individual objects,                                    */
/*             number of objects to get                                       */
/*             behaviour of the new objects.                                  */
/*                                                                            */
/* Function : Return an  Array of count objects of size size, with the given  */
/*             behaviour.  Each objects will be cleared                       */
/*  Returned: Array object                                                    */
/*                                                                            */
/******************************************************************************/
{
    size_t i;
    size_t objSize = roundObjectBoundary(size);
    size_t totalSize;                      /* total size allocated              */
    RexxObject *prototype;                 /* our first prototype object        */

    RexxArray  *arrayOfObjects;
    RexxObject *largeObject;

    /* Get array object to contain all the objects.. */
    arrayOfObjects = (RexxArray *)new_array(count);

    /* Get one LARGE object, that we will parcel up into the smaller */
    /* objects over allocate by the size of one minimum object so we */
    /* can handle any potential overallocations */
    totalSize = objSize * count;         /* first get the total object size   */
    /* We make the decision on which heap this should be allocated */
    /* from based on the size of the object request rather than the */
    /* total size of the aggregate object.  Since our normal usage of */
    /* this is for allocating large collections of small objects, we */
    /* don't want those objects coming from the large block heap, */
    /* even if the aggregate size would suggest this should happen. */
    if (objSize <= LargeBlockThreshold)
    {
        largeObject = newSpaceNormalSegments.allocateObject(totalSize);
        if (largeObject == OREF_NULL)
        {
            largeObject = newSpaceNormalSegments.handleAllocationFailure(totalSize);
        }
    }
    /* we keep the big objects in a separate cage */
    else
    {
        largeObject = newSpaceLargeSegments.allocateObject(totalSize);
        if (largeObject == OREF_NULL)
        {
            largeObject = newSpaceLargeSegments.handleAllocationFailure(totalSize);
        }
    }

    largeObject->initializeNewObject(markWord, virtualFunctionTable[T_Object], TheObjectBehaviour);

    if (this->saveStack != OREF_NULL)
    {
        /* saveobj doesn't get turned on     */
        /*until the system is initialized    */
        /*far enough but once its on, push   */
        /*this new obj on the save stack to  */
        /*keep him from being garbage        */
        /*collected, before it can be used   */
        /*and safely anchored by caller.     */
        pushSaveStack(largeObject);
    }

    /* Description of defect 318:  IH:

    The problem is caused by the constructor of RexxClause which is calling newObjects.
    NewObjects allocates one large Object. Immediately after this large Object is allocated,
    an array is allocated as well. It then can happen that while allocating the array
    the largeObject shall be marked. This causes a trap when objectVariables is != NULL.

    Solution: Set objectVariables to NULL before allocating the array. In order to make
    OrefOK (called if CHECKOREF is defined) work properly, the largeObject has to be
    set to a valid object state before calling new_array. Therefore the behaviour assignement
    and the virtual functions assignement has to be done in advance. */

    /* get the remainder object size...this is used to manage the */
    /* dangling piece on the end of the allocation. */
    totalSize = largeObject->getObjectSize() - totalSize;

    /* Clear out the entire object... */
    largeObject->clearObject();

    prototype = largeObject;

    /* IH: Object gets a valid state for the mark and sweep process. */
    /* Otherwise OrefOK (CHECKOREFS) will fail */

    // initialize the hash table object
    largeObject->initializeNewObject(objSize, markWord, virtualFunctionTable[objectType], RexxBehaviour::getPrimitiveBehaviour(objectType));

    for (i=1 ;i < count ; i++ )
    {
        /* IH: Loop one time less than before because first object is initialized
           outside of the loop. I had to move the following 2 statements
           in front of the object initialization */
        /* add this object to the array of objs */
        arrayOfObjects->put(largeObject, i);
        /* point to the next object space. */
        largeObject = (RexxObject *)((char *)largeObject + objSize);
        /* copy the information from the prototype */
        memcpy(largeObject, prototype, sizeof(RexxInternalObject));
    }
    arrayOfObjects->put(largeObject, i);  /* put the last Object */

    /* adjust the size of the last one to account for any remainder */
    largeObject->setObjectSize(totalSize + objSize);

    return arrayOfObjects;               /* Return our array of objects.      */
}


void RexxMemory::reSize(RexxObject *shrinkObj, size_t requestSize)
/******************************************************************************/
/* Function:  The object shrinkObj only needs to be the size of newSize       */
/*             If the left over space is big enough to become a dead object   */
/*             we will shrink the object to the specified size.               */
/*            NOTE: Since memory knows nothing about any objects that are     */
/*             in the extra space, it cannot do anything about them if this   */
/*             is an OldSpace objetc, therefore the caller must have already  */
/*             take care of this.                                             */
/*                                                                            */
/******************************************************************************/
{
    DeadObject *newDeadObj;

    size_t newSize = roundObjectResize(requestSize);

    /* is the rounded size smaller and is remainder at least the size */
    /* of the smallest OBJ MINOBJSIZE */
    if (newSize < requestSize && (shrinkObj->getObjectSize() - newSize) >= MinimumObjectSize)
    {
        size_t deadObjectSize = shrinkObj->getObjectSize() - newSize;
        /* Yes, then we can shrink the object.  Get starting point of */
        /* the extra, this will be the new Dead obj */
        newDeadObj = new ((void *)((char *)shrinkObj + newSize)) DeadObject (deadObjectSize);
        /* if an object is larger than 16 MB, the last 8 bits (256) are */
        /* truncated and therefore the object must have a size */
        /* dividable by 256 and the rest must be put to the dead chain. */
        /* If the resulting dead object is smaller than the size we */
        /* gave, then we've got a truncated remainder we need to turn */
        /* into a dead object. */
        deadObjectSize -= newDeadObj->getObjectSize();
        if (deadObjectSize != 0)
        {
            /* create difference object.  Note:  We don't bother */
            /* putting this back on the dead chain.  It will be */
            /* picked up during the next GC cycle. */
            new ((char *)newDeadObj + newDeadObj->getObjectSize()) DeadObject (deadObjectSize);
        }
        /* Adjust size of original object */
        shrinkObj->setObjectSize(newSize);
    }
}


void RexxMemory::scavengeSegmentSets(
    MemorySegmentSet *requestor,           /* the requesting segment set */
    size_t allocationLength)               /* the size required          */
/******************************************************************************/
/* Function:  Orchestrate sharing of sharing of storage between the segment   */
/* sets in low storage conditions.  We do this only as a last ditch, as we'll */
/* end up fragmenting the large heap with small blocks, messing up the        */
/* benefits of keeping the heaps separate.  We first look a set to donate a   */
/* segment to the requesting set.  If that doesn't work, we'll borrow a block */
/* of storage from the other set so that we can satisfy this request.         */
/******************************************************************************/
{
    MemorySegmentSet *donor;

    /* first determine the donor/requester relationships. */
    if (requestor->is(MemorySegmentSet::SET_NORMAL))
    {
        donor = &newSpaceLargeSegments;
    }
    else
    {
        donor = &newSpaceNormalSegments;
    }

    /* first look for an unused segment.  We might be able to steal */
    /* one and just move it over. */
    MemorySegment *newSeg = donor->donateSegment(allocationLength);
    if (newSeg != NULL)
    {
        requestor->addSegment(newSeg);
        return;
    }

    /* we can't just move a segment over.  Find the smallest block */
    /* we can find that will satisfy this allocation.  If found, we */
    /* can insert it into the normal deadchains. */
    DeadObject *largeObject = donor->donateObject(allocationLength);
    if (largeObject != NULL)
    {
        /* we need to insert this into the normal dead chain */
        /* locations. */
        requestor->addDeadObject(largeObject);
    }
}


void RexxMemory::liveStackFull()
/******************************************************************************/
/* Function:  Process a live-stack overflow situation                         */
/******************************************************************************/
{
                                         /* create a temporary stack          */
    RexxStack *newLiveStack = new (this->liveStack->size * 2, true) RexxStack (this->liveStack->size * 2);
    /* copy the live stack entries       */
    newLiveStack->copyEntries(this->liveStack);
    /* has this already been expanded?   */
    if (this->liveStack != this->originalLiveStack)
    {
        free((void *)this->liveStack);     /* release the old one               */
    }
    this->liveStack = newLiveStack;      /* and set the new stack             */
}

void RexxMemory::mark(RexxObject *markObject)
/******************************************************************************/
/* Function:  Perform a memory management mark operation                      */
/******************************************************************************/
{
    size_t liveMark = markWord | OldSpaceBit;

    markObject->setObjectLive(markWord); /* Then Mark this object as live.    */
                                         /* object have any references?       */
                                         /* if there are no references, we don't */
                                         /* need to push this on the stack, but */
                                         /* we might need to push the behavior */
                                         /* on the stack.  Since behaviors are */
                                         /* we can frequently skip that step as well */
    if (markObject->hasNoReferences())
    {
        if (ObjectNeedsMarking(markObject->behaviour))
        {
            /* mark the behaviour now to keep us from processing this */
            /* more than once. */
            markObject->behaviour->setObjectLive(markWord);
            /* push him to livestack, to mark    */
            /* later.                            */
            pushLiveStack((RexxObject *)markObject->behaviour);
        }
    }
    else
    {
        /* push him to livestack, to mark    */
        /* later.                            */
        pushLiveStack(markObject);
    }
}

RexxObject *RexxMemory::temporaryObject(size_t requestLength)
/******************************************************************************/
/* Function:  Allocate and setup a temporary object obtained via malloc       */
/*            storage.  This is used currently only by the mark routine to    */
/*            expand the size of the live stack during a garbage collection.  */
/******************************************************************************/
{
    /* get the rounded size of the object*/
    size_t allocationLength = roundObjectBoundary(requestLength);
    /* allocate a new object             */
    RexxObject *newObj = (RexxObject *)malloc(allocationLength);
    if (newObj == OREF_NULL)             /* unable to allocate a new one?     */
    {
        /* can't allocate, report resource   */
        /* error.                            */
        reportException(Error_System_resources);
    }
    /* setup the new object header for   */
    /*use                                */
    // initialize the hash table object
    newObj->initializeNewObject(allocationLength, markWord, virtualFunctionTable[T_Object], TheObjectBehaviour);
    return newObj;                       /* and return it                     */
}

void RexxMemory::markGeneral(void *obj)
/******************************************************************************/
/* Function:  Perform various general marking functions such as image save,   */
/*            image restore, object unflatten, and debug garbage collection   */
/******************************************************************************/
{
    RexxObject **pMarkObject = (RexxObject **)obj;
    RexxObject *markObject = *pMarkObject;


    if (markObject == OREF_NULL)         /* is this a null reference?         */
    {
        return;                            /* always return immediately         */
    }

    /* If its a restore image mark       */
    if (restoreimage)
    {
        /* we update the object's location   */
        restoreMark(markObject, pMarkObject);
        return;                            /* finished                          */
    }

    if (objOffset != 0)
    {                /* restoring an object?              */
        /* we update the object's location   */
        restoreObjectMark(markObject, pMarkObject);
        return;                            /* finished                          */
    }

    if (this->envelope)
    {                /* processing an envelope?           */
        /* do the unflatten */
        unflattenMark(markObject, pMarkObject);
        return;                          /* and out                           */
    }


    /* The OrphanCheck code is complete mark from OREF_ENV, which checks the           */
    /* validity of each object in the entire object tree.  Any invalid objects will be */
    /* cause the mark to terminate and output information about the invalid object and */
    /* its ancestry.                                                                   */
    if ((this->orphanCheck) && !saveimage)
    {
        /* do the special validity check mark. */
        orphanCheckMark(markObject, pMarkObject);
        /*  all done here, so return         */
        return;
    }


    if (!saveimage)
    {
        Interpreter::logicError("Wrong mark routine called");
    }

    /* we're doing the image construction work */
    saveImageMark(markObject, pMarkObject);
}


void RexxMemory::saveImageMark(RexxObject *markObject, RexxObject **pMarkObject)
/******************************************************************************/
/* Function:  Perform marking during a save image operation                   */
/******************************************************************************/
{
    RexxObject *bufferReference;
    size_t size;
    /* Regular GC or Saveimage processing. if a REAL object and not */
    /* already marked and not in Old Space. */
    if (markObject != OREF_NULL && !markObject->isObjectLive(markWord) && markObject->isNewSpace())
    {
        /* Then Mark this object as live.    */
        markObject->setObjectLive(markWord);
        /* push him to livestack, so we can mark (at a later time) his */
        /* references. */
        pushLiveStack(markObject);
        /* Get size of this object.          */
        size = markObject->getObjectSize();
        /* add this to our image statistics  */
        logObjectStats(markObject);

        /* address the copy in the image */
        bufferReference = (RexxObject *)(image_buffer + image_offset);
        // we allocated a hard coded buffer, so we need to make sure we don't blow
        // the buffer size.
        if (image_offset + size> MaxImageSize)
        {
            Interpreter::logicError("Rexx saved image exceeds expected maximum");
        }
        /* Copy object to image buffer. */
        memcpy(bufferReference, markObject, size);
        /* clear the mark in the copy        */
        bufferReference->clearObjectMark();
        /* Retrieve the behaviour obj */
        behaviour = bufferReference->behaviour;
        /* Working with a primitive behaviour or a copy? */
        if (behaviour->isNonPrimitive())
        {
            /* tag this as a non-primitive behav */
            bufferReference->setNonPrimitive();
        }
        else
        {
            if (behaviour->isTransientClass())
            {
                Interpreter::logicError("Transient class included in image buffer");
            }
            /* clear this out, as this is overloaded with the oldspace */
            /* flag. */
            bufferReference->setPrimitive();
            // replace behaviour with normalized type number
            bufferReference->behaviour = behaviour->getSavedPrimitiveBehaviour();
        }

        /* we are saving image at this point, no need to keep track of */
        /* Remembered set so don't use OrefSet here. */
        markObject->behaviour = (RexxBehaviour *)image_offset;
        /* update image_offset to point to where next object is to be */
        /* placed in image */
        image_offset += size;
    }

    /* If its a saveimage, modify object referencing this "moved" obj */
    /* with it location within the image Buffer */
    *pMarkObject = (RexxObject *)markObject->behaviour;
}


void RexxMemory::orphanCheckMark(RexxObject *markObject, RexxObject **pMarkObject)
/******************************************************************************/
/* Function:  Perform orphan check marking on an object                       */
/******************************************************************************/
{
    const char *outFileName;
    FILE *outfile;
    bool firstnode;
    RexxString *className;
    const char *objectClassName;

    /* check for invalid objects         */
    if (!objectReferenceOK(markObject))
    {
        /* Get a temporary file name for out */
        outFileName = SysFileSystem::getTempFileName();
        /* Open the temp file for the dump. */
        outfile = fopen(outFileName,"wb");
        logMemoryCheck(outfile, "Found non Object at %p, being marked from %p\n",markObject, pMarkObject);
        /* Let the traversal know we want    */
        /*  more info about first guy...     */
        firstnode = true;
        /* If the object is in object storage*/
        if (inObjectStorage(markObject))
        {
            /* DIsplay a few words of the object's storage. */
            logMemoryCheck(outfile, " non-Object dump -->%8.8X   %8.8X   %8.8X   %8.8X \n", *(int32_t *)markObject,      *((int32_t *)markObject+1) , *((int32_t *)markObject+2) , *((int32_t *)markObject+3));
            logMemoryCheck(outfile, " non-Object dump -->%8.8X   %8.8X   %8.8X   %8.8X \n", *((int32_t *)markObject+4) , *((int32_t *)markObject+5) , *((int32_t *)markObject+6) , *((int32_t *)markObject+7));
            logMemoryCheck(outfile, " non-Object dump -->%8.8X   %8.8X   %8.8X   %8.8X \n", *((int32_t *)markObject+8) , *((int32_t *)markObject+9) , *((int32_t *)markObject+10), *((int32_t *)markObject+11));
            logMemoryCheck(outfile, " non-Object dump -->%8.8X   %8.8X   %8.8X   %8.8X \n", *((int32_t *)markObject+12), *((int32_t *)markObject+13), *((int32_t *)markObject+14), *((int32_t *)markObject+15));

        }
        /* Time to traverse the livestack and get this guy's ancestry--very useful    */
        /* information for debugging this type of problem.                            */
        for (markObject = popLiveStack();
            markObject != OREF_NULL;     /* test for unique terminator        */
            markObject = popLiveStack())
        {

            /* OREF_NIL marks an ancestor        */
            if (markObject == TheNilObject)
            {
                /* pop the ancestor                  */
                markObject  = popLiveStack();
                className = markObject->id();
                if (className == OREF_NULL)
                    objectClassName = "";
                else
                    objectClassName = className->getStringData();
                if (firstnode)
                {             /* is this the first node??          */
                    printf("-->Parent node was marking offset '%u'x \n", (char *)pMarkObject - (char *)markObject);
                    dumpObject(markObject, outfile);
                    firstnode = false;
                    logMemoryCheck(outfile, "Parent node is at %p, of type %s(%d) \n",
                                   markObject, objectClassName, markObject->behaviour->getClassType());
                }
                else
                {
                    logMemoryCheck(outfile, "Next node is at %p, of type %s(%d) \n",
                                   markObject, objectClassName, markObject->behaviour->getClassType());
                }
            }
        }
        /* reached the OREF_NULL sentinel    */
        logMemoryCheck(outfile, "Finished popping stack !!\n");
        printf("All data has been captured in file %s \n", outFileName);
        fclose(outfile);                /* Close the file, all with it.      */
                                        /* Make sure we exit the GC critical */
                                        /*  otherwise the cleanup from logic */
                                        /*  error will hang !!!              */
        /* we would have crashed soon anyway!*/
        Interpreter::logicError("Bad Object found during GC !\n");
    }

    if (!markObject->isObjectLive(markWord) && markObject->isNewSpace())
    {
        markObject->setObjectLive(markWord); /* Mark this object as live          */
        /* push him to livestack, so we can  */
        /*mark (at a later time) his         */
        /*references.                        */
        pushLiveStack(markObject);
    }
}


void RexxMemory::discardHoldObject(RexxInternalObject *obj)
{
   /* Remove object form savetable */
   saveTable->remove((RexxObject *)obj);
   /* no memory section needed */
   saveStack->push((RexxObject *)obj);
}


RexxObject *RexxMemory::holdObject(RexxInternalObject *obj)
/******************************************************************************/
/* Function:  Place an object on the hold stack                               */
/******************************************************************************/
{
   /* Push object onto saveStack */
   saveStack->push((RexxObject *)obj);
   /* return held  object               */
   return (RexxObject *)obj;
}

RexxObject *RexxMemory::setParms(RexxObject *deadSegs, RexxObject *notUsed)
/******************************************************************************/
/* Arguments:  Maximum and minimum allocations between reclamations           */
/******************************************************************************/
{
    return OREF_NULL;
}

void RexxMemory::saveImage(void)
/******************************************************************************/
/* Function:  Save the memory image as part of interpreter build              */
/******************************************************************************/
{
    FILE *image;
    RexxObject *markObject;
    MemoryStats _imageStats;
    /* array of objects needed at front  */
    /*of image for faster restore.       */
    int   i;                             /* loop counter                      */
    RexxArray *primitive_behaviours;     /* saved array of behaviours         */
    RexxArray *saveArray;                /* array of objects needed at front  */

    this->imageStats = &_imageStats;     /* set the pointer to the current collector */
                                         /* of image for faster restore.      */
    _imageStats.clear();                 /* clear out image counters          */

    markReason = SAVINGIMAGE;            // this is an image save

    globalStrings = OREF_NULL;
    /* memory Object not saved           */
    TheKernel->remove(getGlobalName(CHAR_MEMORY));

    // this has been protecting every thing critical
    // from GC events thus far, but now we remove it because
    // it contains things we don't want to save in the image.
    TheEnvironment->remove(getGlobalName(CHAR_KERNEL));

    /* remove any programs left over in  */
    /* Get an array to hold all special  */
    /*objs                               */
    saveArray = new_array(saveArray_highest);
    // Note:  A this point, we don't have an activity we can use ProtectedObject to save
    // this with, so we need to use saveObject();
    saveObject(saveArray);
    /* Add all elements needed in        */
    /*saveArray                          */
    saveArray->put((RexxObject *)TheEnvironment,   saveArray_ENV);
    saveArray->put((RexxObject *)TheKernel,        saveArray_KERNEL);
    saveArray->put((RexxObject *)TheTrueObject,    saveArray_TRUE);
    saveArray->put((RexxObject *)TheFalseObject,   saveArray_FALSE);
    saveArray->put((RexxObject *)TheNilObject,     saveArray_NIL);
    saveArray->put((RexxObject *)TheNullArray,     saveArray_NULLA);
    saveArray->put((RexxObject *)TheNullPointer,   saveArray_NULLPOINTER);
    saveArray->put((RexxObject *)TheClassClass,    saveArray_CLASS);
    saveArray->put((RexxObject *)PackageManager::getImageData(), saveArray_PACKAGES);
    saveArray->put((RexxObject *)TheSystem,       saveArray_SYSTEM);
    saveArray->put((RexxObject *)TheFunctionsDirectory,  saveArray_FUNCTIONS);
    saveArray->put((RexxObject *)TheCommonRetrievers,    saveArray_COMMON_RETRIEVERS);
    saveArray->put((RexxObject *)saveStrings(), saveArray_NAME_STRINGS);

    /* create the behaviour array        */
    primitive_behaviours= (RexxArray *)new_array(T_Last_Exported_Class + 1);
    /* copy all of the primitive         */
    for (i = 0; i <= T_Last_Exported_Class; i++)
    {
        /* behaviours into this array        */
        primitive_behaviours->put((RexxObject *)RexxBehaviour::getPrimitiveBehaviour(i), i + 1);
    }
    /* add to the save array             */
    saveArray->put(primitive_behaviours, saveArray_PBEHAV);

    image_buffer = (char *)malloc(MaxImageSize);
    image_offset = sizeof(size_t);
    saveimage = true;
    disableOrefChecks();                 /* Don't try to check OrefSets now.  */
    bumpMarkWord();
    /* push a unique terminator          */
    pushLiveStack(OREF_NULL);
    /* don't want to save                */
    /*savestack/savetabl                 */
    saveStack = OREF_NULL;
    /* or any of there references in the */
    saveTable = OREF_NULL;
    /* image, which will become OldSpace */
    OrefSet(&memoryObject, old2new, OREF_NULL);

    pushLiveStack(OREF_NULL);            /* push a unique terminator          */
    memory_mark_general(saveArray);      /* push live root                    */

    for (markObject = popLiveStack();
        markObject != OREF_NULL;        /*   test for unique terminator      */
        markObject = popLiveStack())
    {
        /* The mark of this object moved it  */
        /*to the image, its behaviour now    */
        /*contains its offset in the image   */
        /* so point to the object in the     */
        /*image.                             */
        /* the buffer copy                   */
        RexxObject *copyObject = (RexxObject *)(image_buffer+(uintptr_t)markObject->behaviour);

        copyObject->liveGeneral(SAVINGIMAGE); /* mark other referenced objs        */
        /* non-primitive behaviour?          */
        if (copyObject->isNonPrimitive())
            /* mark/move behaviour live      */
            memory_mark_general(copyObject->behaviour);
    }

    image = fopen(BASEIMAGE,"wb");
    /* PLace actual size at beginning of buffer*/
    memcpy(image_buffer, &image_offset, sizeof(size_t));
    /* Write out buffer (image)      */
    fwrite(image_buffer, 1, image_offset, image);
    fclose(image);
    free(image_buffer);

    printf("Object stats for this image save are \n");
    _imageStats.printSavedImageStats();
    printf("\n\n Total bytes for this image %d bytes \n", image_offset);
}


RexxObject *RexxMemory::dump(void)
/******************************************************************************/
/* Function:  Dump the memory tables                                          */
/******************************************************************************/
{
#ifdef _DEBUG
    FILE *dumpfile;
    FILE *keyfile;
    MemorySegmentPool *currentPool = this->firstPool;
    int   i;

    if (dumpEnable)
    {
        /* don't allow another dump unless   */
        /*reset by user                      */
        memoryObject.dumpEnable = false;
        /* first generate the dump by writing*/
        /*  each segment to the dumpfile     */
        printf("Dumping object memory to orexdump.dmp\n");
        /* open for binary write             */
        dumpfile = fopen("orexdump.dmp","wb");
        /* now generate the key file which   */
        /*is a series of REXX statements     */
        printf("Creating dump key file in orexdump.key\n");
        /* open for text write               */
        keyfile = fopen("orexdump.key","w");
        fprintf(keyfile, "/* Object REXX dump key file */\n");
        fprintf(keyfile, "memoryaddr = %p\n", this);
        fprintf(keyfile, "marker = %d\n", markWord);
        /* dump the oldspace segments */
        oldSpaceSegments.dumpSegments(keyfile, dumpfile);
        /* followed by the newer allocations */
        newSpaceNormalSegments.dumpSegments(keyfile, dumpfile);
        newSpaceLargeSegments.dumpSegments(keyfile, dumpfile);

        i = 0;
        while (currentPool)
        {
            i++;
            fprintf(keyfile, "Pool addr.%d = %p\n", i, currentPool);
            fprintf(keyfile, "Pool size.%d = %d\n", i, currentPool->reserved);
            currentPool = currentPool->nextPool();
        }

        for (i = 0; i <= T_Last_Exported_Class; i++)
        {
            fprintf(keyfile, "Behaviour type %d = %p\n", i, RexxBehaviour::getPrimitiveBehaviour(i));
        }

        /* now close actual dump and key file*/
        fclose(dumpfile);
        fclose(keyfile);
    }
#endif
    return OREF_NULL;
}


RexxObject *RexxMemory::setDump(RexxObject *selection)
/******************************************************************************/
/* Arguments:  selection, 0 for no, anything else for yes                     */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
    if (selection != OREF_NULL)
    {
        dumpEnable = selection->truthValue(Error_Logical_value_method);
    }
    return selection;
}

RexxObject *RexxMemory::gutCheck(void)
/******************************************************************************/
/* Arguments:  None                                                           */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
    wholenumber_t  count, testcount;
    HashLink j;
    bool restoreimagesave;
    RexxInteger *value;
    RexxInteger *testValue;
    RexxObject *index;
    /* temp OREF used to hold original   */
    /*and test versions                  */
    RexxIdentityTable *tempold2new;

    printf("Comparing old2new with the current system.\n");

    /* build a test remembered set       */
    tempold2new = new_identity_table();
    restoreimagesave = restoreimage;
    restoreimage = true;                 /* setting both of these to true will*/
                                         /* traverse System OldSpace and live */
                                         /*mark all objects                   */
    oldSpaceSegments.markOldSpaceObjects();

    restoreimage = restoreimagesave;     /* all done building remembered set  */
                                         /* For all object in old2new table   */
    for (j = old2new->first();
        (index = (RexxObject *)old2new->index(j)) != OREF_NULL;
        j = old2new->next(j))
    {
        /* Get refCount for this obj old2new */
        value = (RexxInteger *)this->old2new->get(index);
        /* new refcount for obj in newly     */
        /*build old2new table                */
        testValue = (RexxInteger *)tempold2new->get(index);
        /* Was object found in new table?    */
        if (testValue == OREF_NULL)
        {
            /* nope, extra stuff in orig.        */
            printf("object:  %p,  type:  %d, is extra in old2new.\n\n",
                   index, index->behaviour->getClassType());
        }
        else
        {
            /* now make sure both refCounts are  */
            /* the same.                         */
            count = value->getValue();
            testcount = testValue->getValue();
            if (count != testcount)
            {
                printf("object:  %p,  type:  %d, has an incorrect refcount.\n",
                       index, index->behaviour->getClassType());
                printf("Refcount for object is %d, should be %d.\n\n", count, testcount);
            }
            /* now remove object from new table  */
            tempold2new->remove(index);
        }
    }
    /* Anything left in new table was    */
    /* missing from original old2new     */
    /* so iterate through it and report  */
    /* the missing objects               */
    for (j = tempold2new->first();
        (index = (RexxObject *)tempold2new->index(j)) != OREF_NULL;
        j = tempold2new->next(j))
    {
        printf("object:  %p,  type:  %d, is missing from old2new.\n\n",
               index,index->behaviour->getClassType());
    }

    /* now take a dump so that we can    */
    /*  diagnose any problems we just    */
    /*uncovered                          */
    printf("Dumping object memory.\n");  /* tell the user what's happening    */
    this->dumpEnable = true;             /* turn dumps on                     */
    this->dump();                        /* take the dump                     */

    return OREF_NULL;
}


void RexxMemory::setObjectOffset(size_t offset)
/******************************************************************************/
/* Arguments:  offset, the length to add to references within arriving objects*/
/*     since only one unflattem can happen at a time, we need to ensure       */
/*     serialization, so if already have an objOffset, wait unto done.        */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
    /* Starting or ending? */
    if (offset != 0)
    {

        /* have a value, starting unflatten.  See if we can get MUTEX */
        /* immed */
        if (!unflattenMutex.requestImmediate())
        {
            {
                UnsafeBlock releaser;
                /* wait for current unflatten to end */
                unflattenMutex.request();
            }
        }
    }
    else
    {
        /* no value, ending an unflatten */
        /* Release the MUTEX.                */
        unflattenMutex.release();
    }

    /* setup offSet value.               */
    this->objOffset = offset;
}

void      RexxMemory::setEnvelope(RexxEnvelope *_envelope)
/******************************************************************************/
/* Arguments:  envelope object,                                               */
/*     since only one unflattem can happen at a time, we need to ensure       */
/*     serialization, so if already have an envelope,  wait unto done.        */
/*                                                                            */
/*  Returned:  Nothing                                                        */
/******************************************************************************/
{
    /* Starting or ending?               */
    if (_envelope != OREF_NULL)
    {
        /* have a value, starting unflatt    */
        /* See if we can get MUTEX immed     */
        if (!envelopeMutex.requestImmediate())
        {
            /* Nope, have to wait for it.        */
            /* release kernel access.            */
            {
                UnsafeBlock releaser;
                envelopeMutex.request();   /* wait for current unflat to end    */
            }
        }
    }
    else
    {
        /* no value, ending an unflatten     */
        envelopeMutex.release();         /* Release the MUTEX.                */
    }

    this->envelope = _envelope;          /* set the envelope object           */
}


RexxObject *RexxMemory::setOref(void *oldValue, RexxObject *value)
/******************************************************************************/
/* Arguments:  index-- OREF to set;  value--OREF to which objr is set         */
/*                                                                            */
/*  Returned:  nothing                                                        */
/*                                                                            */
/******************************************************************************/
{
    RexxInteger *refcount;
    RexxObject **oldValueLoc = (RexxObject **)oldValue;
    RexxObject *index = *oldValueLoc;

    if (old2new != OREF_NULL)
    {
        if (index != OREF_NULL && index->isNewSpace())
        {
            /* decrement reference count for     */
            /**index                             */
            refcount = (RexxInteger *)this->old2new->get(index);
            if (refcount != OREF_NULL)
            {
                /* found a value for refcount, now   */
                /*decrement the count                */
                refcount->decrementValue();
                /* if the new value is 0, *index is  */
                /*no longer ref'ed from oldspace     */
                if (refcount->getValue() == 0)
                {
                    /* delete the entry for *index       */
                    this->old2new->remove(index);
                }
            }
            else
            {
                /* naughty, naughty, someone didn't use SetOref */
                printf("******** error in memory_setoref, unable to decrement refcount\n");
                printf("Naughty object reference is from:  %p\n", oldValueLoc);
                printf("Naughty object reference is at:  %p\n", index);
                printf("Naughty object reference type is:  %d\n", (index)->behaviour->getClassType());
            }
        }
        if (value != OREF_NULL && value->isNewSpace())
        {
            /* increment reference count for     */
            /*value                              */
            refcount = (RexxInteger *)this->old2new->get(value);
            /* was there a reference here        */
            /*already?                           */
            if (refcount)
            {
                /* yep, found one, so just increment */
                /*it                                 */
                refcount->incrementValue();
            }
            else
            {
                /* nope, this is the first, so set   */
                /*the refcount to 1                  */
                // NOTE:  We don't use new_integer here because we need a mutable
                // integer and can't use the one out of the cache.
                this->old2new->put(new RexxInteger(1), value);
            }
        }
    }
    /* now make the assignment, just     */
    /*like all this stuff never happened!*/
    return *oldValueLoc = value;
}


RexxObject *RexxMemory::checkSetOref(
                RexxObject  *setter,
                RexxObject **index,
                RexxObject  *value,
                const char  *fileName,
                int          lineNum)
/******************************************************************************/
/* Arguments:  index-- OREF to set;  value--OREF to which objr is set         */
/*                                                                            */
/*  Returned:  nothing                                                        */
/*                                                                            */
/******************************************************************************/
{
    bool allOK = true;
    const char *outFileName;
    FILE *outfile;
    /* Skip all checks during saveimage  */
    if (checkSetOK)
    {                     /* and initial part of restore Image */
        if (!inObjectStorage(setter))
        {      /* Is the setter object invalid      */
            allOK = false;                     /* No, just put out setters addr.    */
            outFileName = SysFileSystem::getTempFileName();/* Get a temporary file name for out */
            outfile = fopen(outFileName,"wb");
            logMemoryCheck(outfile, "The Setter object at %p is invalid...\n");

            /* Is the new value a real object?   */
        }
        else if (value && (RexxBehaviour *)value != TheBehaviourBehaviour && (RexxBehaviour *)value != RexxBehaviour::getPrimitiveBehaviour(T_Behaviour) && !objectReferenceOK(value))
        {
            allOK = false;                     /* No, put out the info              */
            outFileName = SysFileSystem::getTempFileName();/* Get a temporary file name for out */
            outfile = fopen(outFileName,"wb");
            logMemoryCheck(outfile, "The Setter object at %p attempted to put a non object %p, at offset %p\n",setter, value, (char *)index - (char *)setter);
            logMemoryCheck(outfile, " A dump of the Setting object follows: \n");
            dumpObject(setter, outfile);

        }
        else if (index >= (RexxObject **)((char *)setter + setter->getObjectSize()))
        {
            allOK = false;                     /* Yes, let them know                */
            outFileName = SysFileSystem::getTempFileName();/* Get a temporary file name for out */
            outfile = fopen(outFileName,"wb");
            logMemoryCheck(outfile, "The Setter object at %p has tried to store at offset, which is  outside his object range\n",setter, (char *)index - (char *)setter);
            logMemoryCheck(outfile, " A dump of the Setting object follows: \n");
            dumpObject(setter, outfile);
        }


    }

    if (!allOK)
    {
        logMemoryCheck(outfile, " The error occurred in line %u of file %s\n", lineNum, fileName);
        printf("The dump data has been written to file %s \n",outFileName);
        fclose(outfile);
        Interpreter::logicError("Something went really wrong in SetOref ...\n");
    }
    /* now do the normal SetOref() stuff */
    return(setter->isOldSpace() ? (this->setOref(index, value)) : (*index = value));

}

RexxStack *RexxMemory::getFlattenStack(void)
/******************************************************************************/
/* Function:  Allocate and lock the flatten stack capability.                 */
/******************************************************************************/
{
    if (!flattenMutex.requestImmediate())
    {
        /* Nope, have to wait for it.        */
        /* release kernel access.            */
        {
            UnsafeBlock releaser;
            flattenMutex.request();         /* wait for current flattento end    */
        }
    }
    /* create a temporary stack          */
    this->flattenStack = new (LiveStackSize, true) RexxStack (LiveStackSize);
    return this->flattenStack;           /* return flatten Stack              */
}

void RexxMemory::returnFlattenStack(void)
/******************************************************************************/
/* Function:  Release the flatten stack                                       */
/******************************************************************************/
{
   free((void *)this->flattenStack);   /* release the flatten stack         */
   this->flattenMutex.release();       /* and release the semaphore         */
}

RexxObject *RexxMemory::dumpImageStats(void)
/******************************************************************************/
/*                                                                            */
/******************************************************************************/
{
    MemoryStats _imageStats;

    /* clear all of the statistics */
    _imageStats.clear();
    /* gather a fresh set of stats for all of the segments */
    newSpaceNormalSegments.gatherStats(&_imageStats, &_imageStats.normalStats);
    newSpaceLargeSegments.gatherStats(&_imageStats, &_imageStats.largeStats);

    /* print out the memory statistics */
    _imageStats.printMemoryStats();
    return TheNilObject;
}


/**
 *
 * Add a new pool to the memory set.
 *
 * @param pool   The new pool.
 */
void RexxMemory::memoryPoolAdded(MemorySegmentPool *pool)
{
    currentPool = pool;
}


void RexxMemory::shutdown()
/******************************************************************************/
/* Function:  Free all the memory pools currently accessed by this process    */
/*    If not already released.  Then set process Pool to NULL, indicate       */
/*    pools have been released.                                               */
/******************************************************************************/
{
    MemorySegmentPool *pool = firstPool;
    while (pool != NULL)
    {
        // save the one we're about to release, and get the next one.
        MemorySegmentPool *releasedPool = pool;
        pool = pool->nextPool();
        // go free this pool up
        releasedPool->freePool();
    }
    // clear out the memory pool information
    firstPool = NULL;
    currentPool = NULL;
}


void RexxMemory::setUpMemoryTables(RexxIdentityTable *old2newTable)
/******************************************************************************/
/* Function:  Set up the initial memory table.                                */
/******************************************************************************/
{
    /* fix up the previously allocated live stack to have the correct */
    /* characteristics...we're almost ready to go on the air. */
    liveStack->setBehaviour(TheStackBehaviour);
    liveStack->init(LiveStackSize);
    /* set up the old 2 new table provided for us */
    old2new = old2newTable;
    /* if we have a table (this is NULL if we're not running from a */
    /* restored image), then add the first entry to the table. */
    if (old2new != NULL)
    {
        /* now add old2new itself to the old2new table! */
        // NOTE:  We don't use new_integer here because we need a mutable
        // integer and can't use the one out of the cache.
        old2new->put(new RexxInteger(1), old2new);
    }
    /* first official OrefSet!! */
    OrefSet(this, markTable, old2new);
    /* Now get our savestack and savetable */
    /* allocate savestack with usable and allocated size */
    /* NOTE:  We do not use OREF_SET here.  We want to control the */
    /* order in which these two are marked in the live(size_t) method of */
    /* RexxMemory.  If these are added to the mark table, they'll be */
    /* processed earlier than we'd like. */
    saveStack = new (SaveStackAllocSize) RexxSaveStack(SaveStackSize, SaveStackAllocSize);
    /* from this point on, we push things on to the save stack */
    saveTable = new_identity_table();
}

void RexxMemory::createLocks()
/******************************************************************************/
/* Function:  Do the initial lock creation for memory setup.                  */
/******************************************************************************/
{
                                       /* Create/Open Shared MUTEX      */
                                       /* Semophores used to serialize  */
                                       /* the flatten/unflatten process */
    flattenMutex.create();
    unflattenMutex.create();
    envelopeMutex.create();
}

void RexxMemory::closeLocks()
/******************************************************************************/
/* Function:  Do the initial lock creation for memory setup.                  */
/******************************************************************************/
{
                                       /* Create/Open Shared MUTEX      */
                                       /* Semophores used to serialize  */
                                       /* the flatten/unflatten process */
    flattenMutex.close();
    unflattenMutex.close();
    envelopeMutex.close();
}


/**
 * Add a string to the global name table.
 *
 * @param value  The new value to add.
 *
 * @return The single instance of this string.
 */
RexxString *RexxMemory::getGlobalName(const char *value)
{
    // see if we have a global table.  If not collecting currently,
    // just return the non-unique value

    RexxString *stringValue = new_string(value);
    if (globalStrings == OREF_NULL)
    {
        return stringValue;                /* just return the string            */
    }

    // now see if we have this string in the table already
    RexxString *result = (RexxString *)globalStrings->at(stringValue);
    if (result != OREF_NULL)
    {
        return result;                       // return the previously created one
    }
    /* add this to the table             */
    globalStrings->put((RexxObject *)stringValue, stringValue);
    return stringValue;              // return the newly created one
}


void RexxMemory::create()
/******************************************************************************/
/* Function:  Initial memory setup during image build                         */
/******************************************************************************/
{
    RexxClass::createInstance();         /* get the CLASS class created       */
    RexxInteger::createInstance();
    /* Now get our savestack and         */
    /*savetable                          */
    memoryObject.setUpMemoryTables(OREF_NULL);
                                         /* Create/Open Shared MUTEX          */
                                         /* Semophores used to serialize      */
                                         /* the flatten/unflatten process     */
    memoryObject.createLocks();
}


void RexxMemory::restore()
/******************************************************************************/
/* Function:  Memory management image restore functions                       */
/******************************************************************************/
{
    /* Retrieve special saved objects    */
    /* OREF_ENV and primitive behaviours */
    /* are already restored              */
                                         /* start restoring class OREF_s      */
    RESTORE_CLASS(Object, RexxClass);
    RESTORE_CLASS(Class, RexxClass);
                                         /* (CLASS is already restored)       */
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
    RESTORE_CLASS(IdentityTable, RexxClass);
    RESTORE_CLASS(Relation, RexxClass);
    RESTORE_CLASS(MutableBuffer, RexxMutableBufferClass);
    RESTORE_CLASS(Pointer, RexxClass);
    RESTORE_CLASS(Buffer, RexxClass);
    RESTORE_CLASS(WeakReference, RexxClass);
    RESTORE_CLASS(StackFrame, RexxClass);
    RESTORE_CLASS(Exception, RexxClass);

    memoryObject.setOldSpace();          /* Mark Memory Object as OldSpace    */
    /* initialize the tables used for garbage collection. */
    memoryObject.setUpMemoryTables(new_identity_table());
                                         /* If first one through, generate all*/
    IntegerZero   = new_integer(0);      /*  static integers we want to use...*/
    IntegerOne    = new_integer(1);      /* This will allow us to use static  */
    IntegerTwo    = new_integer(2);      /* integers instead of having to do a*/
    IntegerThree  = new_integer(3);      /* new_integer evrytime....          */
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
    ActivityManager::init();             /* do activity restores              */
    PackageManager::restore();           // finish restoration of the packages.
    memoryObject.enableOrefChecks();     /* enable setCheckOrefs...           */
}

