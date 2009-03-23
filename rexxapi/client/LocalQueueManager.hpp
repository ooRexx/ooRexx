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

#ifndef LocalQueueManager_HPP_INCLUDED
#define LocalQueueManager_HPP_INCLUDED

#include "LocalAPISubsystem.hpp"
#include "rexx.h"
#include "Rxstring.hpp"
#include "ServiceMessage.hpp"
#include "Utilities.hpp"

typedef uintptr_t QueueHandle;     // type for returned queue handles

// local instance of the queue API...this is a proxy that communicates with the
// server that manages the queues.
class LocalQueueManager : public LocalAPISubsystem
{
public:

    LocalQueueManager();

    typedef uintptr_t QueueHandle;

    inline bool isSessionQueue(const char *name)
    {
        return name == NULL || Utilities::strCaselessCompare(name, "SESSION") == 0;
    }

    bool validateQueueName(const char *username);
    void initializeLocal(LocalAPIManager *a);
    virtual void terminateProcess();
    QueueHandle initializeSessionQueue(SessionID s);
    QueueHandle createSessionQueue(SessionID session);
    RexxReturnCode createNamedQueue(const char *name, size_t size, char *createdName, size_t *dup);
    RexxReturnCode openNamedQueue(const char *name, size_t *dup);
    RexxReturnCode queryNamedQueue(const char *name);
    RexxReturnCode deleteSessionQueue();
    RexxReturnCode deleteNamedQueue(const char * name);
    RexxReturnCode clearSessionQueue();
    RexxReturnCode clearNamedQueue(const char * name);
    RexxReturnCode getSessionQueueCount(size_t &);
    RexxReturnCode getQueueCount(const char *name, size_t &);
    RexxReturnCode addToNamedQueue(const char *name, CONSTRXSTRING &data, size_t lifoFifo);
    RexxReturnCode addToSessionQueue(CONSTRXSTRING &data, size_t lifoFifo);
    RexxReturnCode pullFromQueue(const char *name, RXSTRING &data, size_t waitFlag, REXXDATETIME *timeStamp);
    QueueHandle nestSessionQueue(SessionID s, QueueHandle q);
    virtual RexxReturnCode processServiceException(ServiceException *e);
    RexxReturnCode mapReturnResult(ServiceMessage &m);

protected:
    LocalAPIManager *localManager;  // our local manager instance
    QueueHandle    sessionQueue;    // our resolved session queue
    SessionID      sessionID;       // the working session id
    static bool createdSessionQueue;   // remember if we created the session queue
};

#endif
