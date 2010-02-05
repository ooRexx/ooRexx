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
/* Semaphore support for Windows systems                                     */
/*                                                                           */
/*****************************************************************************/

#ifndef Included_SysSemaphore
#define Included_SysSemaphore

#include "rexx.h"

#include <stdlib.h>

inline void waitHandle(HANDLE s);

class SysSemaphore {
public:
     SysSemaphore() : sem(0) { ; }
     SysSemaphore(bool);
     ~SysSemaphore() { ; }
     void create();
     inline void open() { ; }
     void close();
     void post() { SetEvent(sem); };
     inline void wait()
     {
         waitHandle(sem);
     }

     inline bool wait(uint32_t timeout)
     {
         return WaitForSingleObject(sem, timeout) != WAIT_TIMEOUT;
     }

     inline void reset() { ResetEvent(sem); }
     inline bool posted() { return WaitForSingleObject(sem, 0) != 0; }

     static inline bool allocTlsIndex()
     {
         tlsNoMessageLoopIndex = TlsAlloc();
         usingTls = (tlsNoMessageLoopIndex != TLS_OUT_OF_INDEXES);
         return usingTls;
     }
     static inline void deallocTlsIndex()
     {
         TlsFree(tlsNoMessageLoopIndex);
         usingTls = false;
     }

     static inline void setNoMessageLoop() { TlsSetValue(tlsNoMessageLoopIndex, (LPVOID)1); }
     static inline bool noMessageLoop() { return usingTls && ((DWORD_PTR)TlsGetValue(tlsNoMessageLoopIndex) == 1); }

protected:
     HANDLE sem;

private:
     static bool usingTls;
     static DWORD tlsNoMessageLoopIndex;

};


class SysMutex {
public:
     SysMutex() : mutexMutex(0) { }
     SysMutex(bool);
     ~SysMutex() { ; }
     void create();
     void close();
     inline void request()
     {
         waitHandle(mutexMutex);
     }

     inline void release()
     {
         ReleaseMutex(mutexMutex);
     }

     inline bool requestImmediate()
     {
         return WaitForSingleObject(mutexMutex, 0) != WAIT_TIMEOUT;
     }

protected:
     HANDLE mutexMutex;      // the actual mutex
};


/**
 *  Wait for a synchronization object to be in the signaled state.
 *
 *  Any thread that creates windows must process messages.  A thread that
 *  calls WaitForSingelObject with an infinite timeout risks deadlocking the
 *  system.  MS's solution for this is to use MsgWaitForMultipleObjects to
 *  wait on the object, or a new message arriving in the message queue. Some
 *  threads create windows indirectly, an example is COM with CoInitialize.
 *  Since we can't know if the current thread has a message queue that needs
 *  processing, we use MsgWaitForMultipleObjects.
 *
 *  However, with the introduction of the C++ native API in ooRexx 4.0.0, it
 *  became possible for external native libraries to attach a thread with an
 *  active window procedure to the interpreter. If a wait is done on that
 *  thread, here in waitHandle(), PeekMessage() causes non-queued messages to be
 *  dispatched, and the window procedure is reentered. This can cause the Rexx
 *  program to hang. In addition, in the one known extension where this problem
 *  happens, ooDialog, the messages need to be passed to the dialog manager
 *  rather than dispatched directly to the window.
 *
 *  For this special case, the thread can explicity ask, through
 *  RexxSetProcessMessages(), that messages are *not* processed during this
 *  wait. Thread local storage is used to keep track of a flag signalling this
 *  case on a per-thread basis.
 *
 *  Note that MsgWaitForMultipleObjects only returns if a new message is
 *  placed in the queue.  PeekMessage alters the state of all messages in
 *  the queue so that they are no longer 'new.'  Once PeekMessage is called,
 *  all the messages on the queue need to be processed.
 */
inline void waitHandle(HANDLE s)
{
    // If already signaled, return.
    if ( WaitForSingleObject(s, 0) == WAIT_OBJECT_0 )
    {
        return;
    }

    // If no message loop is flagged, then use WaitForSingleobject()
    if ( SysSemaphore::noMessageLoop() )
    {
        WaitForSingleObject(s, INFINITE);
        return;
    }

    MSG msg = {0};

    do
    {
        while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            // Check to see if signaled.
            if ( WaitForSingleObject(s, 0) == WAIT_OBJECT_0 )
            {
                return;
            }
        }
    } while ( MsgWaitForMultipleObjects(1, &s, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0 + 1 );
}


#endif
