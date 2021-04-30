/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                              Memorystats.cpp   */
/*                                                                            */
/* Support classes for managing memory profiling                              */
/*                                                                            */
/******************************************************************************/
#include "RexxCore.h"

#include <stdio.h>

void SegmentStats::clear()
/******************************************************************************/
/* Function:  clear out the memory statistics.                                */
/******************************************************************************/
{
    count = 0;
    smallestSegment = Memory::MaximumObjectSize;
    largestSegment = 0;
    totalBytes = 0;
    deadBytes = 0;
    liveBytes = 0;
    deadObjects = 0;
    liveObjects = 0;
}


/**
 * Accumulate memory statistics for a scanned object
 *
 * @param memStats The memory stats we're recording into.
 * @param obj      The object we're recording.
 */
void SegmentStats::recordObject(MemoryStats *memStats, RexxInternalObject *obj)
{
    // tally the size of the object
    size_t bytes = obj->getObjectSize();
    totalBytes += bytes;
    // if the object is alive, log the type information.
    if (obj->isObjectLive(memoryObject.markWord))
    {
        memStats->logObject(obj);
        liveBytes += bytes;
        liveObjects++;
    }
    // this is a dead object
    else
    {
        deadObjects++;
        deadBytes += bytes;
    }
}


void MemoryStats::printSavedImageStats()
/******************************************************************************/
/* Function:  Print out accumulated image statistics                          */
/******************************************************************************/
{
    printf("    ObjectTypeNum         Total Objects       TotalBytes" line_end);
    printf("    =============         ==============      ==========" line_end);

    for (int i = 0; i <= T_Last_Class_Type; i++)
    {
        objectStats[i].printStats(i);
    }
}


void MemoryStats::printMemoryStats()
/******************************************************************************/
/* Function:  Print out statistics for a memory snapshot                      */
/******************************************************************************/
{
    printf("All Objects in Object Memory, by allocation type" line_end line_end);

    printf("    ObjectTypeNum         Total Objects       TotalBytes" line_end);
    printf("    =============         ==============      ==========" line_end);
    int i;

    for (i = 0; i <= T_Last_Class_Type; i++)
    {
        objectStats[i].printStats(i);
    }

    printf(line_end "Segment set allocation statistics" line_end line_end);

    normalStats.printStats();
    largeStats.printStats();
}


void SegmentStats::printStats()
/******************************************************************************/
/* Function:  Print out statistics for a segment set snapshot                 */
/******************************************************************************/
{
  printf("%s:  Total bytes %zu in %zu segments " line_end, name, totalBytes, count);
  printf("Largest segment is %zu bytes, smallest is %zu bytes" line_end, largestSegment, smallestSegment);
  printf("Total Live objects %zu, using %zu bytes" line_end, liveObjects, liveBytes);
  printf("Total Dead objects %zu, using %zu bytes" line_end line_end, deadObjects, deadBytes);
}


void ObjectStats::printStats(int type)
/******************************************************************************/
/* Function:  Print out accumulated statistics for an individual objec type   */
/******************************************************************************/
{
    if (count > 0 || size > 0)
    {
        printf("    %3d                     %8zu            %8zu  " line_end, type, count, size);
    }
}


void MemoryStats::clear()
/******************************************************************************/
/* Function:  clear out the memory statistics.                                */
/******************************************************************************/
{
    normalStats.clear();
    largeStats.clear();

    for (int i = 0; i <= T_Last_Class_Type; i++)
    {
        objectStats[i].clear();
    }
}


void MemoryStats::logObject(RexxInternalObject *obj)
/******************************************************************************/
/* Function:  Log the memory statistics for an individual object              */
/******************************************************************************/
{
    objectStats[obj->getObjectTypeNumber()].logObject(obj);
}
