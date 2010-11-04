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
/* REXX Kernel                                             RexxEnvelope.cpp   */
/*                                                                            */
/* Primitive Envelope Class                                                   */
/*                                                                            */
/******************************************************************************/

#include <stdlib.h>
#include "RexxCore.h"
#include "StackClass.hpp"
#include "StringClass.hpp"
#include "BufferClass.hpp"
#include "RexxSmartBuffer.hpp"
#include "ArrayClass.hpp"
#include "RexxEnvelope.hpp"
#include "MethodClass.hpp"
#include "ActivityManager.hpp"

RexxEnvelope::RexxEnvelope()
/******************************************************************************/
/* Function:  Initialize a REXX envelope object                               */
/******************************************************************************/
{
}

void RexxEnvelope::live(size_t liveMark)
/******************************************************************************/
/* Function:  Normal garbage collection live marking                          */
/*                                                                            */
/*  NOTE: Do not mark flattenStack                                            */
/******************************************************************************/
{
  memory_mark(this->home);
  memory_mark(this->receiver);
  memory_mark(this->duptable);
  memory_mark(this->savetable);
  memory_mark(this->buffer);
  memory_mark(this->rehashtable);

}

void RexxEnvelope::liveGeneral(int reason)
/******************************************************************************/
/* Function:  Generalized object marking                                      */
/*                                                                            */
/*  NOTE: Do not mark flattenStack                                            */
/******************************************************************************/
{
  memory_mark_general(this->home);
  memory_mark_general(this->receiver);
  memory_mark_general(this->duptable);
  memory_mark_general(this->savetable);
  memory_mark_general(this->buffer);
  memory_mark_general(this->rehashtable);
}


void RexxEnvelope::flattenReference(
    void        *newThisVoid,          /* current pointer to flattening obj */
    size_t       newSelf,              /* offset of the flattening object   */
    void        *objRefVoid)           /* object to process                 */
/******************************************************************************/
/* Function: This method does the copy buffer,                                */
/******************************************************************************/
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
        this->flattenStack->fastPush((RexxObject *)objOffset);
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


RexxBuffer *RexxEnvelope::pack(
    RexxObject *_receiver)              /* the receiver object               */
/******************************************************************************/
/* Function:  Pack an envelope item                                           */
/******************************************************************************/
{
    RexxObject *flattenObj;              /* flattened object                  */
    RexxObject *newSelf;                 /* the flattened envelope            */
    RexxObject *firstObject;             /* first object to flatten           */

    OrefSet(this, this->receiver, _receiver);
    // create a save table to protect any objects (such as proxy
    // objects) we create during flattening.
    OrefSet(this, this->savetable, new_identity_table());
    OrefSet(this, this->duptable, new_identity_table());
    // this is a bit of a hack, but necessary.  This allows us to store
    // object offsets into a hashtable without having the hashtable
    // attempt to mark the references.
    duptable->contents->setHasNoReferences();
    OrefSet(this, this->buffer, new RexxSmartBuffer(DEFAULT_ENVELOPE_BUFFER));
    // get a flatten stack from the memory object
    this->flattenStack = memoryObject.getFlattenStack();
    // push unique terminator onto stack
    this->flattenStack->fastPush(OREF_NULL);

    // First, put a header into the buffer.  This is necessary because without
    // it, the envelope object would be at 0 offset into the buffer, which is not
    // distinguishable from OREF_NULL when the objects are unpacked from buffer.


    // the header is just a dummy minimal object instance.  We don't bother adding
    // this to the dup table, as it won't ever be duped.
    this->copyBuffer(TheObjectClass->newObject());
    // we start the flattening process with the received object
    firstObject = this->receiver;

    this->currentOffset = this->copyBuffer(firstObject);
    // make sure we add this to the dup table in case it's self-referential at any point
    associateObject(firstObject, this->currentOffset);
    /* point to the copied one           */
    newSelf = (RexxObject *)(this->bufferStart() + this->currentOffset);

    // ok, keep flattening until will find our marker object on the stack
    newSelf->flatten(this);              /* start the flatten process.        */

    for (flattenObj = this->flattenStack->fastPop();
        flattenObj != OREF_NULL;
        flattenObj = this->flattenStack->fastPop())
    {
        // the popped object is actuall an object offset.  We need to convert this into a
        // real object pointer
        this->currentOffset = (size_t)flattenObj;
        flattenObj = (RexxObject *)(this->bufferStart() + this->currentOffset);
        // and flatten the next object
        flattenObj->flatten(this);         /* let this obj flatten its refs     */
    }
    memoryObject.returnFlattenStack();   /* done with the flatten stack       */
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
        /* Force fix-up of                   */
        /*VirtualFunctionTable,              */
        ((RexxObject *)bufferPointer)->setVirtualFunctions(RexxMemory::virtualFunctionTable[primitiveTypeNum]);
        puffObject->setObjectLive(memoryObject.markWord);  /* Then Mark this object as live.    */
                                           /* Mark other referenced objs        */
                                           /* Note that this flavor of          */
                                           /* mark_general should update the    */
                                           /* mark fields in the objects.       */
        puffObject->liveGeneral(UNFLATTENINGOBJECT);
        /* Point to next object in image.    */
        bufferPointer += puffObject->getObjectSize();
    }
    memoryObject.setObjectOffset(0);     /* all done with the fixups!         */

    // Prepare to reveal the objects in  the buffer.
    // the first object in the buffer is a dummy added
    // for padding.  We need to step past that one to the
    // beginning of the real unflattened objects
    OrefSet(this, this->receiver, (RexxObject *)(startPointer + ((RexxObject *)startPointer)->getObjectSize()));
    /* chop off end of buffer to reveal  */
    /* its contents to memory obj        */

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
    bufferPointer = (char *)this->receiver;
    /* Set envelope to the real address of the new objects.  This tells               */
    /* mark_general to send unflatten to run any proxies.                             */
    memoryObject.setEnvelope(this);      /* tell memory to send unflatten     */

    /* Now traverse the buffer running any proxies.                                   */
    while (bufferPointer < endPointer)
    {
        puffObject = (RexxObject *)bufferPointer;
        // Since a GC could happen at anytime we need to check to make sure the object
        //  we are going now unflatten is still alive, since all who reference it may have already
        //  run and gotten the info from it and no longer reference it.
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
    // Before we run the method we need to give any tables a chance to rehash...
    this->rehash();
}

size_t RexxEnvelope::queryObj(
    RexxObject *obj)                   /* object to check                   */
/******************************************************************************/
/* Function:  Check to see if we've already flattened an object               */
/******************************************************************************/
{
   return (size_t)this->duptable->get(obj);
}

size_t RexxEnvelope::copyBuffer(
    RexxObject *obj)                   /* object to copy                    */
/******************************************************************************/
/* Function:  Copy an object into our flatten buffer                          */
/******************************************************************************/
{
    // copy the object into the buffer, which might cause the buffer to
    // resize itself.
    size_t objOffset = this->buffer->copyData((void *)obj, obj->getObjectSize());
    // get a reference to the copied object
    RexxObject *newObj = (RexxObject *) (this->buffer->getBuffer()->getData() + objOffset);
    // if this is a non-primative behaviour, we need to flatten it as well.  The
    // offset is tagged as being a non-primitive behaviour that needs later inflating.
    if (newObj->behaviour->isNonPrimitive())
    {
        void *behavPtr = &newObj->behaviour;

        this->flattenReference(&newObj, objOffset, (RexxObject **)behavPtr);
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


void  RexxEnvelope::rehash()
/******************************************************************************/
/* Function:  Rehash flattened tables                                         */
/******************************************************************************/
{
    HashLink     i;                      /* loop index                        */
    RexxTable    * index;                /* table to flatten                  */

    if (this->rehashtable != OREF_NULL)
    {/* tables to rehash here?            */
     /* Before we run the method we need  */
     /* to give the tables a chance to    */
     /* rehash...                         */
        for (i = this->rehashtable->first(); (index = (RexxTable *)this->rehashtable->index(i)) != OREF_NULL; i = this->rehashtable->next(i))
        {
            index->reHash();                 /* rehash the table                  */
        }
    }
}

char *RexxEnvelope::bufferStart()
/******************************************************************************/
/* Return the start of the envelope buffer                                    */
/******************************************************************************/
{
    return this->buffer->getBuffer()->getData();
}

void  RexxEnvelope::associateObject(
    RexxObject *o,                     /* original object                   */
    size_t      flattenOffset)         /* new proxy object                  */
/******************************************************************************/
/* Function:  Map an object to a flattened proxy object                       */
/******************************************************************************/
{
    // we just add this to the duptable under the original object
    // reference value.
    this->duptable->addOffset(flattenOffset, o);
}

void RexxEnvelope::addTable(
    RexxObject *obj)                   /* table object to rehash            */
/******************************************************************************/
/* Function:  Add an object to the rehash table for later processing          */
/******************************************************************************/
{
    /*  the following table will be used */
    /* by the table_unflatten method.    */
    /*                                   */
    /* Every table that gets unflattened */
    /* place itself in this table.  Once */
    /* every object has been unflattened */
    /* we traverse this table and allow  */
    /* the hashtables to re-hash their   */
    /* since some hash value may have    */
    /* change                            */
    if (this->rehashtable == OREF_NULL)  /* first table added?                */
    {
        /* create the table now              */
        OrefSet(this, this->rehashtable, new_identity_table());
    }
    /* use put to make sure we only get  */
    /* a single version of each table    */
    this->rehashtable->put(TheNilObject, obj);
}


void *RexxEnvelope::operator new(size_t size)
/******************************************************************************/
/* Function:  Create a new translator object                                  */
/******************************************************************************/
{
    /* Get new object                    */
    return new_object(sizeof(RexxEnvelope), T_Envelope);
}

