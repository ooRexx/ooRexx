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
/* REXX Kernel                                                DeadObject.c  */
/*                                                                            */
/* Primitive DeadObject management code                                       */
/*                                                                            */
/******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "RexxCore.h"
#include "RexxMemory.hpp"
#include "Interpreter.hpp"

void DeadObjectPool::dumpMemoryProfile(FILE *outFile)
/******************************************************************************/
/* Function:  Dump statistics for this dead object pool.                      */
/******************************************************************************/
{
#ifdef MEMPROFILE                      /* doing memory profiling            */
    fprintf(outFile, "Statistics for Dead Object Pool %s\n, id");
    fprintf(outFile, "    Total memory allocations:  %d\n", allocationCount);
    fprintf(outFile, "    Unsuccessful requests:  %d\n", allocationMisses);
    fprintf(outFile, "    Successful requests:  %d\n", allocationHits);
    fprintf(outFile, "    Objects added back to the pool\n", allocationReclaim);
#endif
}


DeadObject *DeadObjectPool::findBestFit(size_t length)
/******************************************************************************/
/* Function:  Find an object that is the best fit for a requested length.     */
/* To get the best fit, we either want an object that fits well enough that   */
/* we don't need to create a trailing dead object from the remainder, OR      */
/* we return the largest possible object so that we can create the largest    */
/* possible remainder object.                                                 */
/******************************************************************************/
{
    DeadObject *newObject = anchor.next;
    DeadObject *largest = NULL;
    size_t largestSize = 0;
    size_t deadLength = newObject->getObjectSize();

    for (; deadLength != 0; deadLength = newObject->getObjectSize()) {
        if (deadLength >= length) {
            /* if within an allocation unit of the request size (which should */
            /* be common, since we round to a higher boundary for */
            /* large allocations), use this block */
            if ((deadLength - length) < VeryLargeObjectGrain) {
                newObject->remove();
                logHit();
                return newObject;
            }
            /* keep track of the largest block.  We want to */
            /* suballocate that if we can't find a good fit. */
            else if (deadLength > largestSize) {
                largestSize = deadLength;
                largest = newObject;
            }
        }
        newObject = newObject->next;
    }
    /* we didn't find a close fit, so use the largest that will */
    /* work, if we found one. */
    if (largest != NULL) {
        logHit();
        largest->remove();
    }
    else {
        logMiss();
    }
    return largest;
}


DeadObject *DeadObjectPool::findSmallestFit(size_t minSize)
/******************************************************************************/
/* Function:  Find the smallest object in the pool that will hold the length. */
/******************************************************************************/
{
    DeadObject *newObject = anchor.next;
    DeadObject *smallest = NULL;
    size_t smallestSize = MaximumObjectSize;

    while (newObject->isReal()) {
        size_t deadLength = newObject->getObjectSize();
        /* does this fit the request size? */
        if (deadLength >= minSize && deadLength < smallestSize) {
            /* remember this for the end. */
            smallestSize = deadLength;
            smallest = newObject;
            /* did we get an exact fit?  No need to look at any */
            /* others then. */
            if (deadLength == minSize) {
                break;
            }
        }
        newObject = newObject->next;
    }
    /* if we found a fit, remove the block and return it. */
    if (smallest != NULL) {
        smallest->remove();
        logHit();
    }
    else {
        logMiss();
    }
    return smallest;
}


void DeadObjectPool::addSortedBySize(DeadObject *obj)
/******************************************************************************/
/* Function:  Add an object to the pool sorted by size (ascending order).     */
/******************************************************************************/
{
//  checkObjectOverlap(obj);
//  checkObjectGrain(obj);
    /* we start with the first element in the chain as the */
    /* insertion point.  If the chain is empty, we'll terminate the */
    /* loop immediately and use the anchor as the insertion point, */
    /* which gives us the result we want. */
    DeadObject *insertPoint = anchor.next;
    size_t size = obj->getObjectSize();

    while (insertPoint->isReal()) {
        /* if the current block is larger than the one we're */
        /* inserting, we've found our spot. */
        if (insertPoint->getObjectSize() >= size) {
            break;
        }
        insertPoint = insertPoint->next;
    }
    /* insert this at the given point */
    insertPoint->insertBefore(obj);
}


void DeadObjectPool::addSortedByLocation(DeadObject *obj)
/******************************************************************************/
/* Function:  Add an object to the pool sorted by address (ascending order).  */
/******************************************************************************/
{
//  checkObjectOverlap(obj);
//  checkObjectGrain(obj);
    /* we start with the first element in the chain as the */
    /* insertion point.  If the chain is empty, we'll terminate the */
    /* loop immediately and use the anchor as the insertion point, */
    /* which gives us the result we want. */
    DeadObject *insertPoint = anchor.next;

    while (insertPoint->isReal()) {
        /* if the current block's address is larger than the one we're */
        /* inserting, we've found our spot. */
        if (insertPoint > obj) {
            break;
        }
        insertPoint = insertPoint->next;
    }
    /* insert this at the given point */
    insertPoint->insertBefore(obj);
}


void DeadObjectPool::checkObjectGrain(DeadObject *obj)
/******************************************************************************/
/* Function:  Debug validity check of object added to a dead pool.            */
/******************************************************************************/
{
    if (!IsObjectGrained(obj))
    {
        Interpreter::logicError("Object aligned on improper boundary");
    }
}

void DeadObjectPool::checkObjectOverlap(DeadObject *obj)
/******************************************************************************/
/* Function:  Debug validity check of object added to a dead pool.            */
/******************************************************************************/
{
    DeadObject *check = anchor.next;

    while (check != NULL && check->isReal()) {
        if (check->overlaps(obj)) {
            printf("Object at %p for length %d overlaps object at %p for length %d\n", obj, obj->getObjectSize(), check, check->getObjectSize());
            Interpreter::logicError("Overlapping dead objects added to the cache.");
        }
        check = check->next;
    }
}
