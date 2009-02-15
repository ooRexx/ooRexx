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

#ifndef QueueManager_HPP_INCLUDED
#define QueueManager_HPP_INCLUDED

#include "ServiceMessage.hpp"
#include "SysSemaphore.hpp"
#include "SysThread.hpp"

class APIServer;
class ServerQueueManager;

class QueueItem
{
    friend class DataQueue;
public:
    QueueItem(const char *data, size_t s)
    {
        next = NULL;
        // we can use the memory item directly
        elementData = data;
        size = s;
        setTime();
    }

    ~QueueItem()
    {
        if (elementData != NULL)
        {
            // make sure we release this too.  This was allocated by the
            // incoming message, so we need to use the other mechanism for
            // releasing this memory
            ServiceMessage::releaseResultMemory((void *)elementData);
        }
    }

    void setTime();

    // we're passing this data back, so just detach the data buffer.
    inline void clear()
    {
        elementData = NULL;
        size = 0;
    }

protected:

    QueueItem *next;             // next item in the queue
    const char *elementData;     // the element data
    size_t     size;             // size of the element data
    REXXDATETIME  addTime;       // time the element was added
};

class DataQueue
{
    friend class QueueTable;
public:
    DataQueue()
    {
        init();      // do common initilization
    }

    DataQueue(SessionID s)
    {
        init();      // do common initilization
        session = s;
    }

    DataQueue(const char *name)
    {
        init();      // do common initilization
        setName(name);
    }

    ~DataQueue();

    inline void setName(const char *name)
    {
        queueName = dupString(name);
    }

    void add(ServiceMessage &message);
    void addLifo(QueueItem *item);
    void addFifo(QueueItem *item);
    void clear();
    QueueItem *getFirst();

    inline void addWaiter()
    {
        waiters++;
    }

    inline void removeWaiter()
    {
        waiters--;
    }

    // check to see if we have processes waiting on the queue, and wake them
    // up to get an item.
    inline void checkWaiters()
    {
        if (waiters > 0)
        {
            waitSem.post();
        }
    }

    inline void waitForData()
    {
        waitSem.wait();
    }

    inline bool hasWaiters()
    {
        return waiters > 0;
    }

    void pull(ServerQueueManager *manager, ServiceMessage &message);
    bool pullData(ServerQueueManager *manager, ServiceMessage &message);

    inline void addReference() { references++; }
    inline size_t removeReference() { return --references; }
    inline bool hasReferences() { return references != 0; }

    void init()
    {
        next = NULL;
        itemCount = 0;
        waiters = 0;
        references = 1;
        waitSem.create();
        firstItem = NULL;
        lastItem = NULL;
        queueName = NULL;
        session = 0;
    }

    size_t getItemCount() { return itemCount; }

protected:

    DataQueue *next;             // next item in the chain
    size_t     itemCount;        // number of items in the queue
    size_t     waiters;          // number of processes waiting on a queue item
    size_t     references;       // number of nested references to queue
    SysSemaphore waitSem;        // used to signal wait for item
    QueueItem *firstItem;        // first queue item
    QueueItem *lastItem;         // last queue item
    const char *queueName;       // pointer to queue name
    SessionID  session;          // session of queue
};

// a table of queues
class QueueTable
{
public:

    QueueTable()
    {
        queues = NULL;
    }

    // locate a named data queue
    DataQueue *locate(const char *name);
    // locate a named data queue
    DataQueue *locate(SessionID id);
    // locate and remove a named data queue
    DataQueue *remove(const char *name);
    // locate a named data queue
    DataQueue *remove(SessionID id);
    void remove(DataQueue *q);

    inline void reorderQueues(DataQueue *current, DataQueue *previous)
    {
        if (previous != NULL)            // if we have a predecessor
        {
            // rearrange to get "most recently used" behavior
            previous->next = current->next;
            current->next = queues;
            queues = current;
        }
    }

    inline void removeQueue(DataQueue *current, DataQueue *previous)
    {
        if (previous != NULL)            // if we have a predecessor
        {
            // rearrange to get "most recently used" behavior
            previous->next = current->next;
        }
        else
        {
            queues = current->next;
        }
    }

    inline bool isEmpty()
    {
        return queues == NULL;
    }

    // locate a named data queue
    void add(DataQueue *queue);

protected:
    DataQueue *queues;           // head of the data queue chain
};

// the server instance of the queue manager
class ServerQueueManager
{
    friend class DataQueue;     // needs access to the instance lock
public:
    ServerQueueManager() : namedQueues(), sessionQueues(), lock() { lock.create(); }

    void terminateServer();
    void addToSessionQueue(ServiceMessage &message);
    void addToNamedQueue(ServiceMessage &message);
    void pullFromSessionQueue(ServiceMessage &message);
    void pullFromNamedQueue(ServiceMessage &message);
    void createSessionQueue(ServiceMessage &message);
    void createUniqueQueue(ServiceMessage &message);
    void createNamedQueue(ServiceMessage &message);
    void openNamedQueue(ServiceMessage &message);
    void queryNamedQueue(ServiceMessage &message);
    void nestSessionQueue(ServiceMessage &message);
    void deleteSessionQueue(ServiceMessage &message);
    void deleteSessionQueue(DataQueue *queue);
    void deleteNamedQueue(ServiceMessage &message);
    void clearSessionQueue(ServiceMessage &message);
    void clearNamedQueue(ServiceMessage &message);
    void getSessionQueueCount(ServiceMessage &message);
    void getNamedQueueCount(ServiceMessage &message);
    void dispatch(ServiceMessage &message);
    void cleanupProcessResources(SessionID session);
    inline bool isStoppable()
    {
        return namedQueues.isEmpty() && sessionQueues.isEmpty();
    }


protected:
    QueueTable        namedQueues;      // our named queues
    QueueTable        sessionQueues;    // the sessions queues
    SysMutex          lock;             // our subsystem lock
};

#endif
