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
/* REXX Kernel                                            ActivityTable.hpp   */
/*                                                                            */
/* Activity Storage Table for High Thread IDs                                 */
/*                                                                            */
/******************************************************************************/
#ifndef Included_ActivityTable
#define Included_ActivityTable

#include "ObjectClass.hpp"


typedef struct
{
    ULONG tid;
    RexxObject  *activity;
} ThreadActivityPair;

class ActivityTable {
 public:

  ActivityTable();
  ~ActivityTable() {if (data) free(data);}

  RexxObject   *put(RexxObject *, long);
  void          extend();
  long          first() {for (long i=0;i<max_element;i++) if (data[i].activity) return i; return -1;}
  long          next(long j) {for (long i=j+1;i<max_element;i++) if (data[i].activity) return i; return -1;}
  BOOL          available(long j) {if (j < 0) return FALSE; else return (BOOL) (data[j].activity != OREF_NULL);}
  long          index(long j) {return data[j].tid;}
  RexxObject   *value(long j) {return data[j].activity;}

  /* MostRecently... was not thread save. To be thread save a local variable must be used
     and the whole procedure must be a critical section */
  inline      RexxObject *fastAt(long tid)
              {
                    RexxObject * act = OREF_NULL;
                    SysEnterCriticalSection();
                    if (SysIsThreadEqual(MostRecentlyUsedTid,tid)) act = MostRecentlyUsedActivity;
                    else
                    for (register int i=0;i<max_element;i++)
                        if (SysIsThreadEqual(data[i].tid,tid))
                        {
                            MostRecentlyUsedTid = data[i].tid;
                            act = MostRecentlyUsedActivity = data[i].activity;
                            break;
                        }
                    SysExitCriticalSection();
                    return act;
              }

  long          size;
  long          max_element;
  long          MostRecentlyUsedTid;
  RexxObject   *MostRecentlyUsedActivity;
  ThreadActivityPair *data;
};
#endif

