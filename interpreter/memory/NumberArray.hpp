/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2017 Rexx Language Association. All rights reserved.    */
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
/*                                                                            */
/* Simple object for attaching an array of numeric values to an object        */
/* instance (for example, array dimensions).  Keeps us from needing to keep   */
/* arrays of integer objects to store numeric information.                    */
/*                                                                            */
/******************************************************************************/
#ifndef Included_NumberArray
#define Included_NumberArray

/**
 * A mapping class for keeping non-object values with
 * an object that are indexed like an array.
 */
class NumberArray : public RexxInternalObject
{
 public:

    void *operator new(size_t base, size_t entries);
    inline void  operator delete(void *) {;}

    NumberArray(size_t entries);
    inline NumberArray(RESTORETYPE restoreType) { ; };

    inline void clear() { memset((void *)&entries[0], 0, sizeof(size_t) * totalSize); }
    inline bool inBounds(size_t index) { return index > 0 && index <= totalSize; }
    size_t       size() { return totalSize; };

    size_t       get(size_t index) { return inBounds(index) ? entries[index - 1] : 0; }
    void         put(size_t value, size_t index) { if (inBounds(index)) { entries[index - 1] = value; }}

    // access the value of a field
    inline size_t &operator[] (size_t index)
    {
        return entries[index];
    }

    ArrayClass  *toArray();

protected:

    size_t   totalSize;                 // total size of the table, including the overflow area
    size_t   entries[1];                // the stored numeric values
};

#endif

