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
/* REXX Kernel                                            RexxMemorystats.hpp */
/*                                                                            */
/* Primitive DeadObject class definitions                                     */
/*                                                                            */
/******************************************************************************/

#ifndef Included_MemoryStats
#define Included_MemoryStats

#include "ClassTypeCodes.h"

class MemoryStats;
/* a class for collecting object statistics */
class ObjectStats {

  public:
    inline ObjectStats() : count(0), size(0) {}

    inline void clear() { count = 0; size = 0; }
    inline void logObject(RexxObject *obj) { count++; size += obj->getObjectSize(); }
    void   printStats(int type);

  protected:

    size_t count;
    size_t size;
};

/* a class for collecting segment set statistics */
class SegmentStats {
  friend class MemoryStats;
  friend class MemorySegmentSet;

  public:
    SegmentStats(const char *id) :
        count(0), largestSegment(0), smallestSegment(0),
        totalBytes(0), liveBytes(0), deadBytes(0),
        liveObjects(0), deadObjects(0),
        name(id) { }

    void    clear();
    void    recordObject(MemoryStats *memStats, char *obj);
    void    printStats();

  protected:

    size_t count;
    size_t largestSegment;
    size_t smallestSegment;

    size_t totalBytes;
    size_t liveBytes;
    size_t deadBytes;
    size_t liveObjects;
    size_t deadObjects;
    const char *name;
};

class MemoryStats {
  public:
    inline MemoryStats() :
        normalStats("Normal allocation segment set"),
        largeStats("Large allocation segment pool") {}

    void logObject(RexxObject *obj);
    void printSavedImageStats();
    void printMemoryStats();
    void clear();

    SegmentStats normalStats;
    SegmentStats largeStats;

    ObjectStats objectStats[T_Last_Class_Type + 1];
};

#endif
