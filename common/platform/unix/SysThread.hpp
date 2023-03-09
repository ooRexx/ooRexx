/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2023 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                              SysThread.hpp     */
/*                                                                            */
/* Abstract system thread class.  This is just a template class, which no     */
/* virtual functions defined.  This gives the implementing platform a lot of  */
/* implementation flexibility to use inline methods where appropriate,        */
/* particularly for functions that are NOPs.  The platform is also free to    */
/* define any internal data necessary to implement the thread functions.      */
/*                                                                            */
/******************************************************************************/

#ifndef Included_SysThread
#define Included_SysThread

#include <pthread.h>
#include <sys/time.h>
#include "rexx.h"


/**
 * A wrapper around a posix-style thread.
 */
class SysThread
{

public:

    enum
    {
        THREAD_STACK_SIZE = 1024*512
    };

    SysThread() : attached(false), valid(false) { ; }
    virtual ~SysThread() { ; }

    virtual void attachThread();
    virtual void dispatch();
    void terminate();
    void startup();
    void shutdown();
    void yield();
    bool equals(SysThread &other);
    void waitForTermination();

    static void sleep(int msecs);
    static int longSleep(uint64_t microseconds);
    static uint64_t getMillisecondTicks()
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        return (uint64_t)now.tv_sec * 1000 + now.tv_usec / 1000;
    }
    static int createThread(pthread_t &id, bool &idValid, size_t stackSize, void *(*startRoutine)(void *), void *startArgument);


protected:
    void createThread();

    bool           valid;      // indicates whether opaque _threadID is valid
    bool           attached;   // indicates whether this was a created thread or attached
    pthread_t     _threadID;   // thread identifier
};

#endif
