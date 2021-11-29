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
/* Constant definitions and static methods for manipulating various memory    */
/* objects.  Placed in a separate place to avoid circular include problems.   */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#ifndef Memory_Included
#define Memory_Included


/**
 * Holding place for Memory-related constants and functions.
 * Most things in here are protected, with classes that
 * need the information defined as friends.  Anything needed
 * on a global basis is public.
 */
class Memory
{
public:

    static inline bool isObjectGrained(RexxObject *o) { return ((((size_t)o) % ObjectGrain) == 0); }
    static inline bool isGrained(void *o) { return ((((size_t)o) % ObjectGrain) == 0); }
    static inline bool isValidSize(size_t s) { return ((s) >= MinimumObjectSize && ((s) % ObjectGrain) == 0); }

    static inline size_t roundUp(size_t n, size_t to) { return ((((n)+(to-1))/(to))*to); }
    static inline size_t roundDown(size_t n, size_t to) { return (((n)/(to))*to); }

    static inline size_t roundObjectBoundary(size_t n) { return roundUp(n, ObjectGrain); }
    static inline size_t roundLargeObjectAllocation(size_t n) { return roundUp(n, LargeAllocationUnit); }
    static inline size_t roundObjectResize(size_t n) { return roundUp(n, ObjectGrain); }
    static inline size_t roundPageSize(size_t n)  { return roundUp(n, PageSize); }

    // The default size for the old2new table.  Hopefully, this
    // is more than we need, but we don't want this extending frequently
    static const size_t DefaultOld2NewSize = 512;

    // The default size for the global references table.  This should generally
    // be pretty small.
    static const size_t DefaultGlobalReferenceSize = 64;

    // The minimum allocation unit for an object, which must be large
    // enought for a pair of pointer values
    static const size_t ObjectGrain = sizeof(void *) * 2;

    // The unit of granularity for large allocation
    static const size_t LargeAllocationUnit = 128 * ObjectGrain;

    // this is the granularity for objects greater than 16Mb.
    static const size_t VeryLargeObjectGrain = 32 * ObjectGrain;

    // Minimum size of an object.  This is not the actual minimum size,
    // but we allocate objects with an 8-byte granularity
    // This is the smallest object we'll allocate from storage.
    static const size_t MinimumObjectSize = 3 * ObjectGrain;

    // our largest possible object size is one object grain less than the maximum
    // possible size
    static const size_t MaximumObjectSize = SIZE_MAX - ObjectGrain;

    // default size for the live stack (in entries). We make the initial one
    // much larger if building for 64-bit mode because there is a much higher
    // possibility of getting large complex collections
#ifdef __REXX64__
    static const size_t LiveStackSize = 256 * 1024;
#else
    static const size_t LiveStackSize = 64 * 1024;
#endif
    // the number of newly created items to stack in the save stack
    static const size_t SaveStackSize = 10;
    // the maximum size for the startup image size
    static const size_t MaxImageSize = 3000000;
    // the size of a page
    static const size_t PageSize = 4096;
};

#endif
