/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                          ActivityTable.cpp     */
/*                                                                            */
/* Activity Storage Table for High Thread IDs                                 */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "ActivityTable.hpp"

#define INITIALSIZE 512

ActivityTable::ActivityTable()
{
    size = INITIALSIZE;
    max_element = 0;
    data = (ThreadActivityPair *)calloc(size, sizeof(ThreadActivityPair));
    if (!data)  {  report_exception(Error_System_resources); };
    MostRecentlyUsedTid = 0;
    MostRecentlyUsedActivity = OREF_NULL;
}



RexxObject *ActivityTable::put(
    RexxObject *l_value,                   /* value to add                      */
    long tid)                            /* string index of the value         */
{
    SysEnterCriticalSection();
    if (l_value == OREF_NULL)
    {
        /* seek for the existing entry */
        for (int i=0;i<max_element;i++)
        {
            if (SysIsThreadEqual(data[i].tid, tid))
            {
                MostRecentlyUsedTid = tid;
                MostRecentlyUsedActivity = OREF_NULL;
                if (i < max_element-1)
                {
                    /* copy last one to deleted slot */
                    data[i] = data[max_element-1];
                    max_element--;
                }
                else
                    max_element--;
                SysExitCriticalSection();
                return OREF_NULL;
            }
        }
    }
    else
    /* seek if entry already exists and update */
    for (int i=0;i<max_element;i++)
    {
        if (SysIsThreadEqual(data[i].tid, tid))
        {
                data[i].activity = l_value;
            MostRecentlyUsedTid = tid;
            MostRecentlyUsedActivity = l_value;
            SysExitCriticalSection();
            return data[i].activity;
        }
    }

    /* new entry. Necessary to extend? */
    if (max_element >= size) extend();
    data[max_element].tid = tid;
    data[max_element++].activity = l_value;
    MostRecentlyUsedTid = tid;
    MostRecentlyUsedActivity = l_value;
    SysExitCriticalSection();
    return OREF_NULL;                    /* this returns nothing              */
}


void ActivityTable::extend()
{
    void * tmp;
    tmp = calloc(size*2, sizeof(ThreadActivityPair));
    if (!tmp)  {  report_exception(Error_System_resources); };
    memcpy(tmp, data, size*sizeof(ThreadActivityPair));
    size = size*2;
    free(data);
    data = (ThreadActivityPair *)tmp;
}
