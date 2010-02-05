/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
/*****************************************************************************/
/* REXX Windows Support                                                      */
/*                                                                           */
/* Semaphore support for Windows                                             */
/*                                                                           */
/*****************************************************************************/

#include "SysSemaphore.hpp"

bool SysSemaphore::usingTls = false;
DWORD SysSemaphore::tlsNoMessageLoopIndex = TLS_OUT_OF_INDEXES;

/**
 * Create a semaphore with potential creation-time
 * initialization.
 *
 * @param create Indicates whether the semaphore should be created now.
 */
SysSemaphore::SysSemaphore(bool createSem)
{
    sem = 0;
    if (createSem)
    {
        create();
    }
}


/* ********************************************************************** */
/* ***                  SysSemaphore                                  *** */
/* ********************************************************************** */
void SysSemaphore::create()
{
   if (sem == 0 )
   {
       sem = CreateEvent(NULL, true, true, NULL);
   }
}

void SysSemaphore::close()
{
    if (sem != 0)
    {
        CloseHandle(sem);
        sem = 0;
    }
}


/* ********************************************************************** */
/* ***                  SysMutex                                     *** */
/* ********************************************************************** */

/**
 * Create a semaphore with potential creation-time
 * initialization.
 *
 * @param create Indicates whether the semaphore should be created now.
 */
SysMutex::SysMutex(bool createSem)
{
    mutexMutex = 0;
    if (createSem)
    {
        create();
    }
}


void SysMutex::create()
{
    if (mutexMutex == 0)
    {
        mutexMutex = CreateMutex(NULL, false, NULL);
    }
}

void SysMutex::close()
{
    if (mutexMutex != 0)
    {
        CloseHandle(mutexMutex);
        mutexMutex = 0;
    }
}
