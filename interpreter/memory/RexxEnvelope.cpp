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
/* REXX Kernel                                             RexxEnvelope.cpp   */
/*                                                                            */
/* Primitive Envelope Class                                                   */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "MemoryStack.hpp"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "RexxSmartBuffer.hpp"
#include "ArrayClass.hpp"
#include "RexxEnvelope.hpp"
#include "MethodClass.hpp"
#include "ActivityManager.hpp"
#include "MapTable.hpp"


/**
 * Allocate an envelope object.
 *
 * @param size   The size of the object.
 *
 * @return Storage for an object.
 */
void *RexxEnvelope::operator new(size_t size)
{
    return new_object(sizeof(RexxEnvelope), T_Envelope);
}


/**
 * Normal garbage collection live marking
 *
 *  NOTE: Do not mark flattenStack
 *
 * @param liveMark The current live mark.
 */
void RexxEnvelope::live(size_t liveMark)
{
    memory_mark(home);
    memory_mark(receiver);
    memory_mark(duptable);
    memory_mark(savetable);
    memory_mark(buffer);
    memory_mark(rehashtable);

}


/**
 * Generalized object marking
 *
 * NOTE: Do not mark flattenStack
 *
 * @param reason The reason for the marking call.
 */
void RexxEnvelope::liveGeneral(MarkReason reason)
{
    memory_mark_general(home);
    memory_mark_general(receiver);
    memory_mark_general(duptable);
    memory_mark_general(savetable);
    memory_mark_general(buffer);
    memory_mark_general(rehashtable);
}


/**
 * Potentially copy an object into a buffer and resolve
 * references to the object to it's buffer location.
 *
 * @param newThisVoid
 *                   The pointer to the reference to the copied object
 *                   owning this reference.  If it is necessary to reallocate
 *                   the buffer during this mark operation, this pointer
 *                   value gets updated.
 * @param newSelf    The current buffer offset for this object.
 * @param objRefVoid The pointer to the reference getting marked
 *                   (will also get updated)
 */
void RexxEnvelope::flattenReference(void *newThisVoid, size_t newSelf, void *objRefVoid)
{
    RexxObject **newThis = (RexxObject **)newThisVoid;
    RexxObject **objRef  = (RexxObject **)objRefVoid;

    RexxObject *obj = *objRef;

                                          /* See if object has already been    */
    size_t objOffset = this->queryObj(obj); /* flattened.                        */
    // if this object has not been previously flattened, we need to
    // copy the object into the buffer and add the offset to the table.
    if (objOffset == 0)
    {
        // this is the sart of the buffer
        char *flattenBuffer = this->bufferStart();
        // this is the offset to the reference we're working with.  If the
        // buffer needs to reallocate after a copy, we need to be able to
        // locate this later
        size_t referenceOffset = (char *)objRef - flattenBuffer;

        // if this is a proxied object, we need to convert it to a proxy and
        // copy that object into the buffer and the reference table
        if (obj->isProxyObject())
        {
            // get a proxy and make sure it's in our protection table
            RexxObject *proxyObj = obj->makeProxy(this);
            savetable->put(proxyObj, proxyObj);

            // copy the proxy to the buffer and add it to the dup table
            // using the original object as the index.
            objOffset = this->copyBuffer(proxyObj);
            // it's not likely, but we might get a dup of the
            // proxy object too.  Add it to our resolution table.
            this->associateObject(proxyObj, objOffset);
        }
        else
        {

            // non-proxied object.  This gets copied to the buffer
            // directly and added to the dup table
            objOffset = this->copyBuffer(obj);
        }
        // regardless of how we handle this, add an association for the object to the offset
        this->associateObject(obj, objOffset);
        // We're pushing an object offset on to our live stack, so we want to make sure our debug traps
        // don't try to process this.
        memoryObject.disableOrefChecks();
        this->flattenStack->push((RexxObject *)objOffset);
        memoryObject.enableOrefChecks();
        // if the buffer reallocated, we need to update the updating object pointer too.
        char *newBuffer = this->bufferStart();
        if (newBuffer != flattenBuffer)
        {
            *newThis = (RexxObject *) (newBuffer + newSelf);
        }
        // and update the reference with the offset
        *(RexxObject **)(newBuffer + referenceOffset) = (RexxObject *)objOffset;
    }
    else
    {
        // no copying means no reallocation...we just replace the
        // original object reference with the offset value
        *objRef = (RexxObject *)objOffset;
    }
}


/**
 * Pack an object into an envelope
 *
 * @param _receiver The object to be packed.
 *
 * @return The buffer containing the flattened object.
 */
RexxBuffer *RexxEnvelope::pack(RexxObject *_receiver)
{
    RexxObject *flattenObj;              /* flattened object                  */
    RexxObject *newSelf;                 /* the flattened envelope            */
    RexxObject *firstObject;             /* first object to flatten           */

    // NOTE:  Envelopes are transient objects and will never appear in the OldSpace
    // image.  We do not need to use OrefSet or setField here.

    receiver = _receiver;
    // create a save table to protect any objects (such as proxy
    // objects) we create during flattening.
    savetable = new_identity_table();
    duptable = new MapTable(DefaultDupTableSize);
    buffer = new RexxSmartBuffer(DefaultEnvelopeBuffer);
    // get a flatten stack from the memory object
    flattenStack = memoryObject.getFlattenStack();
    // push unique terminator onto stack
    flattenStack->push(OREF_NULL);

    // First, put a header into the buffer.  This is necessary because without
    // it, the envelope object would be at 0 offset into the buffer, which is not
    // distinguishable from OREF_NULL when the objects are unpacked from buffer.


    // the header is just a dummy minimal object instance.  We don't bother adding
    // this to the dup table, as it won't ever be duped.
    copyBuffer(new RexxObject);
    // we start the flattening process with the received object
    firstObject = receiver;

    currentOffset = copyBuffer(firstObject);
    // make sure we add this to the dup table in case it's self-referential at any point
    associateObject(firstObject, currentOffset);
    // point to the copied one
    newSelf = (RexxObject *)(bufferStart() + currentOffset);

    // ok, keep flattening until will find our marker object on the stack
    newSelf->flatten(this);

    for (flattenObj = flattenStack->pop();
        flattenObj != OREF_NULL;
        flattenObj = flattenStack->pop())
    {
        // the popped object is actually an object offset.  We need to store them
        // that way because the object location can change if the buffer needs to
        // reallocate.  We need to convert this into a real object pointer
        currentOffset = (size_t)flattenObj;
        flattenObj = (RexxObject *)(bufferStart() + currentOffset);
        // and flatten the next object
        flattenObj->flatten(this);
    }
    // finished with the flatten stack
    memoryObject.returnFlattenStack();
    // now unwrap the smart buffer and fix the length of the real buffer
    // behind it to the size we've written to it.
    RexxBuffer *letter = buffer->getBuffer();
    letter->setDataLength(buffer->getDataLength());
    return letter;
}


/**
 * Perform an in-place unflatten operation on an object
 * in a buffer.
 *
 * @param sourceBuffer
 *                   The buffer containing the flattened object.
 * @param startPointer
 *                   The starting data location in the buffer.
 * @param dataLength The length of the data to unflatten
 */
void RexxEnvelope::puff(RexxBuffer *sourceBuffer, char *startPointer, size_t dataLength)
{
    size_t primitiveTypeNum = 0;         /* primitive behaviour type number   */

    char *bufferPointer = startPointer;  /* copy the starting point           */
                                         /* point to end of buffer            */
    char *endPointer = (char *)startPointer + dataLength;
    RexxObject *puffObject = OREF_NULL;

    /* Set objoffset to the real address of the new objects.  This tells              */
    /* mark_general to fix the object's refs and set their live flags.                */
    memoryObject.setObjectOffset((size_t)bufferPointer);
    /* Now traverse the buffer fixing all of the behaviour pointers of the objects.   */
    while (bufferPointer < endPointer)
    {
        puffObject = (RexxObject *)bufferPointer;

        /* a non-primitive behaviour         */
        /* These are actually in flattened   */
        /* storgage.                         */
        if (puffObject->isNonPrimitive())
        {
            /* Yes, lets get the behaviour Object*/
            RexxBehaviour *objBehav = (RexxBehaviour *)(((uintptr_t)puffObject->behaviour) + sourceBuffer->getData());
            /* Resolve the static behaviour info */
            objBehav->resolveNonPrimitiveBehaviour();
            /* Set this objects behaviour.       */
            puffObject->behaviour = objBehav;
            /* get the behaviour's type number   */
            primitiveTypeNum = objBehav->getClassType();

        }
        else
        {
            // convert this from a type number to the actuall class.  This will unnormalize the
            // type number to the different object classes.
            puffObject->behaviour = RexxBehaviour::restoreSavedPrimitiveBehaviour(puffObject->behaviour);
            primitiveTypeNum = puffObject->behaviour->getClassType();
        }

        // Force fix-up of VirtualFunctionTable,
        ((RexxObject *)bufferPointer)->setVirtualFunctions(MemoryObject::virtualFunctionTable[primitiveTypeNum]);
        // mark this object as live
        puffObject->setObjectLive(memoryObject.markWord);
        // Mark other referenced objs
        // Note that this flavor of
        // mark_general should update the
        // mark fields in the objects.
        puffObject->liveGeneral(UNFLATTENINGOBJECT);
        // Point to next object in image.
        bufferPointer += puffObject->getObjectSize();
    }

    memoryObject.setObjectOffset(0);     /* all done with the fixups!         */

    // Prepare to reveal the objects in  the buffer.
    // the first object in the buffer is a dummy added
    // for padding.  We need to step past that one to the
    // beginning of the real unflattened objects
    receiver = (RexxObject *)(startPointer + ((RexxObject *)startPointer)->getObjectSize());

    // this is the location of the next object after the buffer
    char *nextObject = ((char *)sourceBuffer) + sourceBuffer->getObjectSize();
    // this is the size of any tailing buffer portion after the last unflattened object.
    size_t tailSize = nextObject - endPointer;

    // puffObject is the last object we processed.  Add any tail data size on to that object
    // so we don't create an invalid gap in the heap.
    puffObject->setObjectSize(puffObject->getObjectSize() + tailSize);
    // now adjust the front portion of the buffer object to reveal all of the
    // unflattened data.
    sourceBuffer->setObjectSize((char *)startPointer - (char *)sourceBuffer + ((RexxObject *)startPointer)->getObjectSize());

    // move past the header to the real unflattened data
    bufferPointer = (char *)receiver;
    // Set envelope to the real address of the new objects.  This tells
    // mark_general to send unflatten to run any proxies.

    // tell the memory object to unflatten...this sends a second message to the
    // objects in the buffer, which are real live objects now.
    memoryObject.setEnvelope(this);

    // Now traverse the buffer running any proxies.
    while (bufferPointer < endPointer)
    {
        puffObject = (RexxObject *)bufferPointer;
        // Since a GC could happen at anytime we need to check to make sure the object
        //  we are going now unflatten is still alive, since all who reference it may have already
        //  run and gotten the info from it and no longer reference it.

        // In theory, this should not be an issue because all of the objects were
        // in the protected set, but unflatten might have cast off some references.
        if (puffObject->isObjectLive(memoryObject.markWord))
        {
            // Note that this flavor of  liveGeneral will run any proxies
            // created by unflatten and fixup  the refs to them.
            puffObject->liveGeneral(UNFLATTENINGOBJECT);
        }

        // Point to next object in image.
        bufferPointer += puffObject->getObjectSize();
    }

    // Tell memory we're done unflattening
    memoryObject.setEnvelope(OREF_NULL);
    rehash();
}

/**
 * Check if an object has already been flattened.
 *
 * @param obj    The object to check.
 *
 * @return The offset to the object if it is in the table.
 */
size_t RexxEnvelope::queryObj(RexxObject *obj)
{
    return duptable->get(obj);
}


/**
 * Copy an object into the flattening buffer.
 *
 * @param obj    The object to copy.
 *
 * @return The offset of the object within the buffer.
 */
size_t RexxEnvelope::copyBuffer(RexxObject *obj)
{
    // copy the object into the buffer, which might cause the buffer to
    // resize itself.
    size_t objOffset = buffer->copyData((void *)obj, obj->getObjectSize());
    // get a reference to the copied object
    RexxObject *newObj = (RexxObject *) (buffer->getBuffer()->getData() + objOffset);
    // if this is a non-primative behaviour, we need to flatten it as well.  The
    // offset is tagged as being a non-primitive behaviour that needs later inflating.
    if (newObj->behaviour->isNonPrimitive())
    {
        void *behavPtr = &newObj->behaviour;
        flattenReference(&newObj, objOffset, (RexxObject **)behavPtr);
    }
    else
    {
        // transient classes should never be flattened.  This is a problem if we encounter one
        if (newObj->behaviour->isTransientClass())
        {
            reportException(Error_Interpretation);
        }

        // just replace the behaviour with its normalized type number.  This will be used
        // to restore it later.
        newObj->behaviour = newObj->behaviour->getSavedPrimitiveBehaviour();
    }
    // if we flattened an object from oldspace, we just copied everything.  Make sure
    // this is no longer marked as oldspace
    newObj->setNewSpace();
    // the offset is what we need
    return objOffset;
}


/**
 * Rehash flattened tables
 */
void  RexxEnvelope::rehash()
{
    // do we actually have anything here?
    if (rehashtable != OREF_NULL)
    {
        RexxTable *index;
        for (HashLink i = rehashtable->first(); (index = (RexxTable *)rehashtable->index(i)) != OREF_NULL; i = rehashtable->next(i))
        {
            // tell the table to rehash
            index->reHash();
        }
    }
}


/**
 * Return the start of the envelope buffer
 *
 * @return The pointer to the start of the buffer data.
 */
char *RexxEnvelope::bufferStart()
{
    return buffer->getBuffer()->getData();
}


/**
 * Associate an object with an object offset.
 *
 * @param o      The object reference.
 * @param flattenOffset
 *               The associated offset.
 */
void  RexxEnvelope::associateObject(RexxObject *o, size_t flattenOffset)
{
    // we just add this to the duptable under the original object
    // reference value.
    duptable->put(flattenOffset, o);
}


/**
 * Add an object to the rehash table for later processing
 *
 * @param obj    The object requiring a rehash.
 */
void RexxEnvelope::addTable(RexxObject *obj)
{
    // The following table will be used by the table_unflatten method.
    //
    // Every table that gets unflattened place itself in this table.  Once
    // every object has been unflattened we traverse this table and allow
    // the hashtables to re-hash their since some hash values may have changed
    // changed

    // create the table on first addition
    if (rehashtable == OREF_NULL)
    {
        rehashtable = new_identity_table();
    }

    // use put to make sure we only get
    // a single version of each table
    rehashtable->put(TheNilObject, obj);
}

