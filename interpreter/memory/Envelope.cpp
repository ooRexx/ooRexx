/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                 Envelope.cpp   */
/*                                                                            */
/* Primitive Envelope Class                                                   */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "MemoryStack.hpp"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "SmartBuffer.hpp"
#include "ArrayClass.hpp"
#include "Envelope.hpp"
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
void *Envelope::operator new(size_t size)
{
    return new_object(sizeof(Envelope), T_Envelope);
}


/**
 * Normal garbage collection live marking
 *
 * @param liveMark The current live mark.
 */
void Envelope::live(size_t liveMark)
{
    memory_mark(home);
    memory_mark(receiver);
    memory_mark(dupTable);
    memory_mark(saveTable);
    memory_mark(buffer);
    memory_mark(rehashTable);
}


/**
 * Generalized object marking
 *
 * @param reason The reason for the marking call.
 */
void Envelope::liveGeneral(MarkReason reason)
{
    memory_mark_general(home);
    memory_mark_general(receiver);
    memory_mark_general(dupTable);
    memory_mark_general(saveTable);
    memory_mark_general(buffer);
    memory_mark_general(rehashTable);
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
void Envelope::flattenReference(void *newThisVoid, size_t newSelf, void *objRefVoid)
{
    RexxInternalObject **newThis = (RexxInternalObject **)newThisVoid;
    RexxInternalObject **objRef  = (RexxInternalObject **)objRefVoid;

    RexxInternalObject *obj = *objRef;

    // see if the object has already been flattened.
    size_t objOffset = queryObj(obj);
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
        if (obj->requiresProxyObject())
        {
            // get a proxy and make sure it's in our protection table
            RexxInternalObject *proxyObj = obj->makeProxy(this);
            saveTable->put(proxyObj, proxyObj);

            // copy the proxy to the buffer and add it to the dup table
            // using the original object as the index.
            objOffset = copyBuffer(proxyObj);
            // it's not likely, but we might get a dup of the
            // proxy object too.  Add it to our resolution table.
            associateObject(proxyObj, objOffset);
        }
        else
        {

            // non-proxied object.  This gets copied to the buffer
            // directly and added to the dup table
            objOffset = copyBuffer(obj);
        }
        // regardless of how we handle this, add an association for the object to the offset
        associateObject(obj, objOffset);
        flattenStack->push((RexxInternalObject *)objOffset);
        // if the buffer reallocated, we need to update the updating object pointer too.
        char *newBuffer = bufferStart();
        if (newBuffer != flattenBuffer)
        {
            *newThis = (RexxInternalObject *) (newBuffer + newSelf);
        }
        // and update the reference with the offset
        *(RexxInternalObject **)(newBuffer + referenceOffset) = (RexxInternalObject *)objOffset;
    }
    else
    {
        // no copying means no reallocation...we just replace the
        // original object reference with the offset value
        *objRef = (RexxInternalObject *)objOffset;
    }
}


/**
 * Pack an object into an envelope
 *
 * @param _receiver The object to be packed.
 *
 * @return The buffer containing the flattened object.
 */
BufferClass *Envelope::pack(RexxInternalObject *_receiver)
{
    RexxInternalObject *flattenObj;           // flattened object
    RexxInternalObject *newSelf;              // the flattened envelope
    RexxInternalObject *firstObject;          // first object to flatten

    // NOTE:  Envelopes are transient objects and will never appear in the OldSpace
    // image.  We do not need to use OrefSet or setField here.

    receiver = _receiver;
    // create a save table to protect any objects (such as proxy
    // objects) we create during flattening.
    saveTable = new_identity_table();
    dupTable = new MapTable(DefaultDupTableSize);
    buffer = new SmartBuffer(DefaultEnvelopeBuffer);
    // Allocate a flatten stack
    flattenStack = new (Memory::LiveStackSize) LiveStack (Memory::LiveStackSize);
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
    newSelf = (RexxInternalObject *)(bufferStart() + currentOffset);

    // ok, keep flattening until will find our marker object on the stack
    newSelf->flatten(this);

    for (flattenObj = flattenStack->pop(); flattenObj != OREF_NULL; flattenObj = flattenStack->pop())
    {
        // the popped object is actually an object offset.  We need to store them
        // that way because the object location can change if the buffer needs to
        // reallocate.  We need to convert this into a real object pointer
        currentOffset = (size_t)flattenObj;
        flattenObj = (RexxInternalObject *)(bufferStart() + currentOffset);
        // and flatten the next object
        flattenObj->flatten(this);
    }

    // now unwrap the smart buffer and fix the length of the real buffer
    // behind it to the size we've written to it.
    BufferClass *letter = buffer->getBuffer();
    letter->setDataLength(buffer->getDataLength());
    // delete the flatten stack, since that is not allocated from the object heap.
    delete flattenStack;
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
void Envelope::puff(BufferClass *sourceBuffer, char *startPointer, size_t dataLength)
{
    // fix up the objects contained in the data buffer.
    receiver = memoryObject.unflattenObjectBuffer(this, sourceBuffer, startPointer, dataLength);

    // now rehash any unflattened collection objects that require it.
    rehash();
}


/**
 * Check if an object has already been flattened.
 *
 * @param obj    The object to check.
 *
 * @return The offset to the object if it is in the table.
 */
size_t Envelope::queryObj(RexxInternalObject *obj)
{
    return dupTable->get(obj);
}


/**
 * Copy an object into the flattening buffer.
 *
 * @param obj    The object to copy.
 *
 * @return The offset of the object within the buffer.
 */
size_t Envelope::copyBuffer(RexxInternalObject *obj)
{
    // copy the object into the buffer, which might cause the buffer to
    // resize itself.
    size_t objOffset = buffer->copyData((void *)obj, obj->getObjectSize());
    // get a reference to the copied object
    RexxInternalObject *newObj = (RexxInternalObject *) (buffer->getBuffer()->getData() + objOffset);
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
void  Envelope::rehash()
{
    // do we actually have anything here?
    if (rehashTable != OREF_NULL)
    {
        HashContents::TableIterator iterator = rehashTable->iterator();

        for (; iterator.isAvailable(); iterator.next())
        {
            ((HashCollection *)iterator.value())->reHash();
        }
    }
}


/**
 * Return the start of the envelope buffer
 *
 * @return The pointer to the start of the buffer data.
 */
char *Envelope::bufferStart()
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
void  Envelope::associateObject(RexxInternalObject *o, size_t flattenOffset)
{
    // we just add this to the duptable under the original object
    // reference value.
    dupTable->put(flattenOffset, o);
}


/**
 * Add an object to the rehash table for later processing
 *
 * @param obj    The object requiring a rehash.
 */
void Envelope::addTable(RexxInternalObject *obj)
{
    // The following table will be used by the table_unflatten method.
    //
    // Every table that gets unflattened place itself in this table.  Once
    // every object has been unflattened we traverse this table and allow
    // the hashtables to re-hash their since some hash values may have changed
    // changed

    // create the table on first addition
    if (rehashTable == OREF_NULL)
    {
        rehashTable = new_identity_table();
    }

    // use put to make sure we only get
    // a single version of each table
    rehashTable->put(obj, obj);
}

