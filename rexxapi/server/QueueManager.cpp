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

#include "QueueManager.hpp"
#include "APIServer.hpp"
#include <time.h>
#include <new>
#include "stdio.h"
#include "Utilities.hpp"
#include "SynchronizedBlock.hpp"

/**
 * Set the update time for a macro item.
 */
void QueueItem::setTime()
{
    time_t timer = time(NULL);
    struct tm *time = localtime(&timer);
    addTime.year           = time->tm_year;
    addTime.month          = time->tm_mon;
    addTime.day            = time->tm_mday;
    addTime.hours          = time->tm_hour;
    addTime.minutes        = time->tm_min;
    addTime.seconds        = time->tm_sec;
    addTime.microseconds   = 0;
    addTime.weekday        = time->tm_wday;
}

/**
 * Perform cleanup for a delete data queue.
 */
DataQueue::~DataQueue()
{
    waitSem.close();     // make sure our semaphore cleans up if we've used it.
    clear();
    if (queueName != NULL)
    {
        delete [] queueName;        // also delete the name
    }
}

/**
 * Clear all entries from the data queue.
 */
void DataQueue::clear()
{
    // now clear the queue
    QueueItem *item = firstItem;
    while (item != NULL)
    {
        // move to the next one and delete
        QueueItem *localnext = item->next;
        delete item;
        item = localnext;
    }
    firstItem = NULL;
    lastItem = NULL;
    itemCount = 0;
}

/**
 * Process a queue add operation.
 *
 * @param message The service message for the add operation.
 */
void DataQueue::add(ServiceMessage &message)
{
    const char *queueData = (const char *)message.getMessageData();
    size_t itemLength = (size_t)message.parameter1;
    size_t order = (size_t)message.parameter2;
    // detach the message data from the message so the controller
    // doesn't free this.
    message.clearMessageData();
    QueueItem *item = new QueueItem(queueData, itemLength);
    if (order == QUEUE_LIFO)
    {
        addLifo(item);
    }
    else
    {
        addFifo(item);
    }
    message.setResult(QUEUE_ITEM_ADDED);
}


/**
 * Add an item to a queue in LIFO order.
 *
 * @param item   The item to add.
 */
void DataQueue::addLifo(QueueItem *item)
{
    item->next = firstItem;
    firstItem = item;
    if (lastItem == NULL)
    {
        lastItem = item;
    }
    itemCount++;
    // make sure we notify any waiters that something has arrived.
    checkWaiters();
}

/**
 * Add an item to the queue in FIFO order.
 *
 * @param item   The item to add.
 */
void DataQueue::addFifo(QueueItem *item)
{
    if (lastItem == NULL)
    {
        firstItem = item;
        lastItem = item;
    }
    else
    {
        lastItem->next = item;
        lastItem = item;
    }
    itemCount++;
    // make sure we notify any waiters that something has arrived.
    checkWaiters();
}


/**
 * Pull the first item off the queue.
 *
 * @return The QueueItem at the head of the queue, or NULL if the
 *         queue is empty.
 */
QueueItem *DataQueue::getFirst()
{
    QueueItem *item = firstItem;
    if (item != NULL)
    {
        firstItem = item->next;
        if (firstItem == NULL)
        {
            lastItem = NULL;
        }
        itemCount--;
    }
    return item;
}


/**
 * Attempt to pull data from a data queue and attach it to a
 * return message.
 *
 * @param message The message being processed.
 *
 * @return true if the queue had a data item, false if it was currently
 *         empty.
 */
bool DataQueue::pullData(ServerQueueManager *manager, ServiceMessage &message)
{
    Lock managerLock(manager->lock);   // this needs synchronization here

    // now that we have the lock, clear the wait sem unconditionally.
    // this should be safe, as it is either already clear, and has waiters camped
    // on it, or it has recently been posted, and since we're going to read data,
    // we either want it cleared for others to wait, or we're going to need to wait
    // on it ourself.
    waitSem.reset();
    QueueItem *item = getFirst();
    // if we have an item, return it.
    if (item != NULL)
    {
        // make sure we pass the total length back
        message.parameter1 = item->size;
        // copy the time stamp into the now-unused name buffer
        memcpy(message.nameArg, &item->addTime, sizeof(REXXDATETIME));
        // the message will delete the queue data once it has been sent
        // back to the client.
        message.setMessageData((void *)item->elementData, item->size);
        // we've taken the data from the item, so clear it out before we delete.
        item->clear();
        // we're done with this, let it go.
        delete item;
        message.setResult(QUEUE_ITEM_PULLED);
        return true;
    }
    // Move along, nothing to see here....
    return false;
}


/**
 * Pull an item from the front of the queue.
 *
 * @param message The message from the client.
 */
void DataQueue::pull(ServerQueueManager *manager, ServiceMessage &message)
{
    // this might take multiple times if we have to wait
    size_t noWait = (size_t)message.parameter1;

    // if the pull succeeded, return now.
    if (pullData(manager, message))
    {
        return;
    }

    // nowait value
    if (noWait != QUEUE_WAIT_FOR_DATA)
    {
        message.setResult(QUEUE_EMPTY);    // nada
        return;
    }
    else
    {
        {
            Lock managerLock(manager->lock);
            // indicate we have another waiting queue
            addWaiter();
        }
        // now keep looping until we actually get an item
        while (true)
        {
            waitForData();
            {
                // see if this is doable now without waiting...there was a window of
                // opportunity for an item to be added.
                if (pullData(manager, message))
                {
                    {
                        Lock managerLock(manager->lock);
                        // remove us as a waiter
                        removeWaiter();
                    }
                    return;
                }
            }
        }
    }
}

/**
 * locate a named data queue
 *
 * @param name   The target data queue name.
 *
 * @return The DataQueue descriptor, or NULL if it does not
 *         exist.
 */
DataQueue *QueueTable::locate(const char *name)
{
    DataQueue *current = queues;    // start the search
    DataQueue *previous = NULL;     // no previous one

    /* while more queues          */
    while (current != NULL)
    {
        // find the one we want?
        if (Utilities::strCaselessCompare(name, current->queueName) == 0)
        {
            // move this to the front so we find it quickly
            reorderQueues(current, previous);
            return current;
        }
        previous = current;                /* remember this block        */
        current = current->next;           /* step to the next block     */
    }
    return NULL;
}

/**
 * locate a named data queue
 *
 * @param id     The session ID of the queue.
 *
 * @return The DataQueue for the session, which will be created
 *         if needed.
 */
DataQueue *QueueTable::locate(SessionID id)
{
    DataQueue *current = queues;    // start the search
    DataQueue *previous = NULL;     // no previous one

    while (current != NULL)         // while more queues
    {
        // find the one we want?
        if (current->session == id)
        {
            // move this to the front so we find it quickly
            reorderQueues(current, previous);
            return current;
        }
        previous = current;         // remember this block
        current = current->next;    // to the next block
    }
    return NULL;                    // return NULL if not located
}


/**
 * locate and remove a named data queue
 *
 * @param name   The queue name.
 *
 * @return The DataQueue matching the name, or NULL if it doesn't exist.
 */
DataQueue *QueueTable::remove(const char *name)
{
    DataQueue *current = queues;    // start the search
    DataQueue *previous = NULL;     // no previous one

    while (current != NULL)              /* while more queues          */
    {
        // find the one we want?
        if (Utilities::strCaselessCompare(name, current->queueName) == 0)
        {
            // move this to the front so we find it quickly
            removeQueue(current, previous);
            return current;
        }
        previous = current;                /* remember this block        */
        current = current->next;           /* step to the next block     */
    }
    return NULL;
}


/**
 * locate and remove a specific data queue
 *
 * @param q      The queue to remove.
 */
void QueueTable::remove(DataQueue *q)
{
    DataQueue *current = queues;    // start the search
    DataQueue *previous = NULL;     // no previous one

    while (current != NULL)              /* while more queues          */
    {
        // find the one we want?
        if (current == q)
        {
            // move this to the front so we find it quickly
            removeQueue(current, previous);
        }
        previous = current;                /* remember this block        */
        current = current->next;           /* step to the next block     */
    }
}

/**
 * locate a session data queue
 *
 * @param id     The session identifier of the queue to remove.
 *
 * @return The removed queue.
 */
DataQueue *QueueTable::remove(SessionID id)
{
    DataQueue *current = queues;    // start the search
    DataQueue *previous = NULL;     // no previous one

    while (current != NULL)         // while more queues
    {
        // find the one we want?
        if (current->session == id)
        {
            // move this to the front so we find it quickly
            removeQueue(current, previous);
            return current;
        }
        previous = current;         // remember this block
        current = current->next;    // to the next block
    }
    current = new DataQueue(id);    // create a new session queue
    add(current);                   // and add it to the table.
    return current;
}


/**
 * add a named data queue to our list.
 *
 * @param queue  The new queue to add.
 */
void QueueTable::add(DataQueue *queue)
{
    queue->next = queues;
    queues = queue;
}


// Add an item to the session queue.  The message arguments have the
// following meanings:
//
// parameter1 -- length of the queue item.
// parameter2 -- lifo/fifo flag
// parameter3 -- handle of the session queue
void ServerQueueManager::addToSessionQueue(ServiceMessage &message)
{
    // We can go directly to the referenced queue.
    DataQueue *queue = (DataQueue *)message.parameter3;
    queue->add(message);
}

// Add an item to a named queue.  The message arguments have the
// following meanings:
//
// parameter1 -- length of the queue item.
// parameter2 -- lifo/fifo flag
// nameArg    -- ASCII-Z name of the queue
void ServerQueueManager::addToNamedQueue(ServiceMessage &message)
{
    DataQueue *queue = namedQueues.locate(message.nameArg);
    // not previously created?
    if (queue == NULL)
    {
        // this is an error
        message.setResult(QUEUE_DOES_NOT_EXIST);
    }
    // queue exists, so add the item
    else
    {
        queue->add(message);
    }
}


// Pull an item from a session queue.  The message arguments have the
// following meanings:
//
// parameter1 -- NOWAIT flag, indicating whether we should wait for data
// parameter2 -- the ENDWAIT flat to indicate this was a waiting process
// parameter3 -- session queue handle
void ServerQueueManager::pullFromSessionQueue(ServiceMessage &message)
{
    DataQueue *queue = (DataQueue *)message.parameter3;
    queue->pull(this, message);
}


// Pull an item from a session queue.  The message arguments have the
// following meanings:
//
// parameter1 -- NOWAIT flag, indicating whether we should wait for data
// parameter2 -- the ENDWAIT flat to indicate this was a waiting process
// nameArg    -- ASCII-Z name of the queue
void ServerQueueManager::pullFromNamedQueue(ServiceMessage &message)
{
    DataQueue *queue = namedQueues.locate(message.nameArg);
    // not previously created?
    if (queue == NULL)
    {
        // this is an error
        message.setResult(QUEUE_DOES_NOT_EXIST);
    }
    // queue exists, so add the item
    else
    {
        queue->pull(this, message);
    }
}

// Create a session queue.  The message arguments have the
// following meanings:
//
// parameter1 -- caller's session id (replaced by queue handle on return);
void ServerQueueManager::createSessionQueue(ServiceMessage &message)
{
    SessionID session = (SessionID)message.parameter1;
    DataQueue *queue = sessionQueues.locate(session);
    // not previously created?
    if (queue == NULL)
    {
        // this is easy, just create a new queue and add it to the table
        queue = new DataQueue(session);
        sessionQueues.add(queue);
    }
    // name collision...we need to update
    else
    {
        // we have nested usage of the session queues,
        // so we need to bump the nesting counter.
        queue->addReference();

    }
    // return this as a handle
    message.parameter1 = (uintptr_t)queue;
    message.setResult(QUEUE_CREATED);
}

/**
 * Create a queue with a unique name.
 *
 * @param message The inbound service message.
 */
void ServerQueueManager::createUniqueQueue(ServiceMessage &message)
{
    DataQueue *queue = new DataQueue();   // get an anonymous queue
    // the queue pointer makes a good starting point for an anonymous tag
    uintptr_t tag = (uintptr_t)queue;
    for (;;)                   // we need to loop until we get a unique one.
    {
        // message parameter1 is the session identifier.
        sprintf(message.nameArg, "S%pQ%p", (void *)message.parameter1, (void *)tag);
        if (namedQueues.locate(message.nameArg) == 0)
        {
            // set the name
            queue->setName(message.nameArg);
            // got a good one, add it and return
            namedQueues.add(queue);
            return;
        }
        tag++;                   // try a new number
    }
}


// Create a named queue.  The message arguments have the
// following meanings:
//
// parameter1 -- caller's session id, only used with duplicates
// nameArg    -- ASCII-Z name of the queue
void ServerQueueManager::createNamedQueue(ServiceMessage &message)
{
    // no user-specified name?
    if (strlen(message.nameArg) == 0)
    {
        // create a uniquely named one
        createUniqueQueue(message);
        message.setResult(QUEUE_CREATED);
        return;
    }

    DataQueue *queue = namedQueues.locate(message.nameArg);
    // not previously created?
    if (queue == NULL)
    {
        // this is easy, just create a new queue and add it to the table
        queue = new DataQueue(message.nameArg);
        namedQueues.add(queue);
        message.setResult(QUEUE_CREATED);
        return;
    }
    // name collision...we need to update
    else
    {
        message.setResult(DUPLICATE_QUEUE_NAME);
        // create a uniquely named one
        createUniqueQueue(message);
    }
}


// Create a named queue.  The message arguments have the
// following meanings:
//
// parameter1 -- caller's session id, only used with duplicates
// nameArg    -- ASCII-Z name of the queue
void ServerQueueManager::openNamedQueue(ServiceMessage &message)
{
    DataQueue *queue = namedQueues.locate(message.nameArg);
    // not previously created?
    if (queue == NULL)
    {
        // this is easy, just create a new queue and add it to the table
        queue = new DataQueue(message.nameArg);
        namedQueues.add(queue);
        message.setResult(QUEUE_CREATED);
    }
    else
    {
        // indicate this already exists
        message.setResult(QUEUE_EXISTS);
    }
}


// Query a named queue.  The message arguments have the
// following meanings:
//
// parameter1 -- caller's session id, only used with duplicates
// nameArg    -- ASCII-Z name of the queue
void ServerQueueManager::queryNamedQueue(ServiceMessage &message)
{
    DataQueue *queue = namedQueues.locate(message.nameArg);
    // not previously created?
    if (queue == NULL)
    {
        // not here
        message.setResult(QUEUE_DOES_NOT_EXIST);
    }
    else
    {
        // indicate this already exists
        message.setResult(QUEUE_EXISTS);
    }
}


// Increment the reference count on a session queue.  The message arguments have the
// following meanings:
//
// parameter1 -- session queue handle
void ServerQueueManager::nestSessionQueue(ServiceMessage &message)
{
    DataQueue *queue = (DataQueue *)message.parameter2;
    if (queue != NULL)
    {
        queue->addReference();
        // update the queue value
        message.parameter1 = (uintptr_t)queue;
        message.setResult(QUEUE_OK);
    }
    else
    {
        // process this as a create operation
        createSessionQueue(message);
    }
}


// Delete a session queue.  The message arguments have the
// following meanings:
//
// parameter1 -- session queue handle
void ServerQueueManager::deleteSessionQueue(ServiceMessage &message)
{
    DataQueue *queue = (DataQueue *)message.parameter1;
    message.setResult(QUEUE_DELETED);
    // not previously created?
    if (queue == NULL)
    {
        message.setResult(QUEUE_DOES_NOT_EXIST);
    }
    // name collision...we need to update
    else
    {
        // do we have clients waiting for pull data?
        if (queue->hasWaiters())
        {
            message.setResult(QUEUE_IN_USE);
        }
        // still have references?
        else if (queue->removeReference() == 0)
        {
            sessionQueues.remove(queue); // remove from table and
            delete queue;                // delete this
        }
    }
}


// Delete a session queue.  The message arguments have the
// following meanings:
//
// parameter1 -- session queue handle
void ServerQueueManager::deleteSessionQueue(DataQueue *queue)
{
    // do we have clients waiting for pull data?
    if (!queue->hasWaiters())
    {
        // still have references?
        if (queue->removeReference() == 0)
        {
            sessionQueues.remove(queue); // remove from table and
            delete queue;                // delete this
        }
    }
}

// Delete a named queue.  The message arguments have the
// following meanings:
//
// nameArg    -- ASCII-Z name of the queue
void ServerQueueManager::deleteNamedQueue(ServiceMessage &message)
{
    DataQueue *queue = namedQueues.locate(message.nameArg);
    message.setResult(QUEUE_DELETED);
    // not previously created?
    if (queue == NULL)
    {
        message.setResult(QUEUE_DOES_NOT_EXIST);
    }
    else
    {
        // do we have clients waiting for pull data?
        if (queue->hasWaiters())
        {
            message.setResult(QUEUE_IN_USE);
        }
        else
        {
            namedQueues.remove(queue);   // remove the queue item
            delete queue;                // delete this
        }
    }
}


// get the count for a session queue.  The message arguments have the
// following meanings:
//
// parameter1 -- handle of the session queue (updated to queue count on return)
void ServerQueueManager::getSessionQueueCount(ServiceMessage &message)
{
    DataQueue *queue = (DataQueue *)message.parameter1;
    // session queues are automatically created, so we always have
    // an item count
    message.parameter1 = queue->getItemCount();
    message.setResult(QUEUE_EXISTS);
}


// Pull an item from a session queue.  The message arguments have the
// following meanings:
//
// nameArg    -- ASCII-Z name of the queue
void ServerQueueManager::getNamedQueueCount(ServiceMessage &message)
{
    DataQueue *queue = namedQueues.locate(message.nameArg);
    // not previously created?
    if (queue == NULL)
    {
        // this is an error
        message.setResult(QUEUE_DOES_NOT_EXIST);
    }
    // queue exists, so add the item
    else
    {
        message.parameter1 = queue->getItemCount();
        message.setResult(QUEUE_EXISTS);
    }
}


// clear any entries from a session queue.  The message arguments have the
// following meanings:
//
// parameter1 -- handle of the session queue (updated to queue count on return)
void ServerQueueManager::clearSessionQueue(ServiceMessage &message)
{
    DataQueue *queue = (DataQueue *)message.parameter1;
    // session queues are automatically created, so we always have
    // an item count
    queue->clear();
    message.setResult(QUEUE_EXISTS);
}


// clear any entreis from a named queue.  The message arguments have the
// following meanings:
//
// nameArg    -- ASCII-Z name of the queue
void ServerQueueManager::clearNamedQueue(ServiceMessage &message)
{
    DataQueue *queue = namedQueues.locate(message.nameArg);
    // not previously created?
    if (queue == NULL)
    {
        // this is an error
        message.setResult(QUEUE_DOES_NOT_EXIST);
    }
    // queue exists, so add the item
    else
    {
        queue->clear();
        message.setResult(QUEUE_EXISTS);
    }
}


/**
 * Dispatch a queue manager message to the appropriate action.
 *
 * @param message The inbound message from the client.
 */
void ServerQueueManager::dispatch(ServiceMessage &message)
{
    // the pull operations might have to wait for an item to be added,
    // so they need to control their own locking mechanisms
    if (message.operation == PULL_FROM_NAMED_QUEUE)
    {
        pullFromNamedQueue(message);
    }
    else if (message.operation == PULL_FROM_SESSION_QUEUE)
    {
        pullFromSessionQueue(message);
    }
    else {
        Lock managerLock(lock);     // we need to synchronize on this instance
        switch (message.operation)
        {
            case NEST_SESSION_QUEUE:
                nestSessionQueue(message);
                break;
            case CREATE_SESSION_QUEUE:
                createSessionQueue(message);
                break;
            case CREATE_NAMED_QUEUE:
                createNamedQueue(message);
                break;
            case OPEN_NAMED_QUEUE:
                openNamedQueue(message);
                break;
            case QUERY_NAMED_QUEUE:
                queryNamedQueue(message);
                break;
            case DELETE_SESSION_QUEUE:
                deleteSessionQueue(message);
                break;
            case DELETE_NAMED_QUEUE:
                deleteNamedQueue(message);
                break;
            case GET_SESSION_QUEUE_COUNT:
                getSessionQueueCount(message);
                break;
            case GET_NAMED_QUEUE_COUNT:
                getNamedQueueCount(message);
                break;
            case CLEAR_SESSION_QUEUE:
                clearSessionQueue(message);
                break;
            case CLEAR_NAMED_QUEUE:
                clearNamedQueue(message);
                break;
            case ADD_TO_NAMED_QUEUE:
                addToNamedQueue(message);
                break;
            case ADD_TO_SESSION_QUEUE:
                addToSessionQueue(message);
                break;
            default:
                message.setExceptionInfo(SERVER_FAILURE, "Invalid queue manager operation");
                break;
        }
    }
}

void ServerQueueManager::cleanupProcessResources(SessionID session)
{
    DataQueue *queue = sessionQueues.locate(session);
    // if the queue exists for this session, turn this into a
    // delete request.
    if (queue != NULL)
    {
        deleteSessionQueue((DataQueue *)queue);
    }
}
