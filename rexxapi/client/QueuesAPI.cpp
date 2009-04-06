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

#include "rexx.h"
#include "LocalAPIManager.hpp"
#include "LocalQueueManager.hpp"
#include "LocalAPIContext.hpp"
#include "RexxAPI.h"
#include "RexxInternalApis.h"

// the generated name pattern is
#define INTERNALNAME_MINIMUM ((sizeof(void *) * 4) + 3)

/*********************************************************************/
/*                                                                   */
/*  Function:        RexxCreateQueue()                               */
/*                                                                   */
/*  Description:     API Entry to create a queue.                    */
/*                                                                   */
/*  Function:        Create a new (empty) queue.                     */
/*                                                                   */
/*  Notes:           Queues are assigned an internal name            */
/*                   derived from the session that created them      */
/*                   and the serial number of the queue.  When a     */
/*                   queue is deleted, its serial number becomes     */
/*                   reuseable.                                      */
/*                                                                   */
/*                   The queue header blocks are stored in order     */
/*                   of their serial number.  The first queue        */
/*                   created has serial number 1.  Serial number 0   */
/*                   is reserved for the session queue.              */
/*                                                                   */
/*                   Before a queue is created, the chain of queue   */
/*                   headers is searched for gaps.  If we find a     */
/*                   gap in the sequence, we use the number that     */
/*                   belongs to the gap.  This is somewhat           */
/*                   inefficient, but has the advantages of          */
/*                   simplicity, reliability and understandability.  */
/*                                                                   */
/*                   The user can pass a NULL for the requested queue*/
/*                   name.  In that case, we assign an arbitrary     */
/*                   name.                                           */
/*                                                                   */
/*                   If the requested name already exists, we assign */
/*                   an arbitrary name to the new queue.  This name  */
/*                   is passed back to the caller, and the duplicate */
/*                   flag is set.                                    */
/*                                                                   */
/*                   Queue names must obey the rules for REXX        */
/*                   symbols.  Lower case characters in queue names  */
/*                   are translated to upper case.                   */
/*                                                                   */
/*  Input:           Buffer for the name of the created queue.       */
/*                   Size of this buffer.                            */
/*                   Requested queue name.  May be NULL.             */
/*                                                                   */
/*  Output:          Internal name for the queue.  Duplicate         */
/*                   indicator.  Status of create.                   */
/*                                                                   */
/*  Effects:         New queue created.                              */
/*                                                                   */
/*********************************************************************/
RexxReturnCode RexxEntry RexxCreateQueue(
  char *name,                          /* Internal name (returned).  */
  size_t size,                         /* Length of name buffer.     */
  const char *userRequested,           /* Desired name.              */
  size_t *pdup)                        /* Duplicate name flag.       */
{
    ENTER_REXX_API(QueueManager)
    {
        // no user requested name, we'll generate one automatically
        if (userRequested != NULL)
        {
            // we copy the queue name back into the user's buffer, so this space
            // must be at least big enough for the requested name
            if (strlen(userRequested) >= size)
            {
                throw new ServiceException(MEMORY_ERROR, "Unsufficient space for created queue name");
            }
        }
        return lam->queueManager.createNamedQueue(userRequested, size, name, pdup);
    }
    EXIT_REXX_API();
}


/**
 * Get access to a named queue, creating it if necessary.
 *
 * @param name   The name of the desired queue.
 * @param pdup   A flag indicating whether the queue already existed.
 *
 * @return 0 if the queue was accessed ok, otherwise the error code
 *         for any errors.
 */
RexxReturnCode RexxEntry RexxOpenQueue(const char *name, size_t *pdup)
{
    ENTER_REXX_API(QueueManager)
    {
        return lam->queueManager.openNamedQueue(name, pdup);
    }
    EXIT_REXX_API();
}


/**
 * Check to see if a given named queue exists
 *
 * @param name   The name of the desired queue.
 *
 * @return 0 if the queue was accessed ok, otherwise the error code
 *         for any errors.
 */
RexxReturnCode RexxEntry RexxQueueExists(const char *name)
{
    ENTER_REXX_API(QueueManager)
    {
        return lam->queueManager.queryNamedQueue(name);
    }
    EXIT_REXX_API();
}


/*********************************************************************/
/*                                                                   */
/*  Function:         RexxDeleteQueue()                              */
/*                                                                   */
/*  Description:      Delete a queue.                                */
/*                                                                   */
/*  Function:         Delete all entries in a queue, then delete     */
/*                    the queue header.                              */
/*                                                                   */
/*  Notes:            Must tell the queue data manager to            */
/*                    delete the queue entries.                      */
/*                                                                   */
/*  Input:            external queue name.                           */
/*                                                                   */
/*  Effects:          Queue and all its entries deleted.             */
/*                                                                   */
/*********************************************************************/
RexxReturnCode RexxEntry RexxDeleteQueue(
    const char *name)                  /* name of queue to delete    */
{
    ENTER_REXX_API(QueueManager)
    {
        return lam->queueManager.deleteNamedQueue(name);
    }
    EXIT_REXX_API();
}


/*********************************************************************/
/*                                                                   */
/*  Function:         RexxClearQueue()                               */
/*                                                                   */
/*  Description:      Clear a queue.                                 */
/*                                                                   */
/*  Function:         Clear all entries in a queue;                  */
/*                                                                   */
/*  Input:            external queue name.                           */
/*                                                                   */
/*  Effects:          All entries in the queue are removed.          */
/*                                                                   */
/*********************************************************************/
RexxReturnCode RexxEntry RexxClearQueue(
    const char *name)                    /* name of queue to delete    */
{
    ENTER_REXX_API(QueueManager)
    {
        // "SESSION" means get the session queue
        if (lam->queueManager.isSessionQueue(name))
        {
            return lam->queueManager.clearSessionQueue();
        }
        else
        {
            return lam->queueManager.clearNamedQueue(name);
        }
    }
    EXIT_REXX_API();
}


/*********************************************************************/
/*                                                                   */
/*  Function:         RexxQueryQueue()                               */
/*                                                                   */
/*  Description:      Return size of a named queue.                  */
/*                                                                   */
/*  Function:         Return the count of elements in a named queue. */
/*                                                                   */
/*  Input:            external queue name                            */
/*                                                                   */
/*  Effects:          Count of queue elements.                       */
/*                                                                   */
/*********************************************************************/
RexxReturnCode  RexxEntry RexxQueryQueue(
  const char *name,                   /* Queue to query.             */
  size_t *count)                      /* Length of queue (returned)  */
{
    ENTER_REXX_API(QueueManager)
    {
        // "SESSION" means get the session queue
        if (lam->queueManager.isSessionQueue(name))
        {
            return lam->queueManager.getSessionQueueCount(*count);
        }
        else
        {
            return lam->queueManager.getQueueCount(name, *count);
        }
    }
    EXIT_REXX_API();
}


/*********************************************************************/
/*                                                                   */
/*  Function:         RexxAddQueue()                                 */
/*                                                                   */
/*  Description:      Add entry to a queue.                          */
/*                                                                   */
/*  Function:         Allocate memory for queue entry and control    */
/*                    block.  Move data into entry & set up          */
/*                    control info.  Add entry to queue chain.       */
/*                                                                   */
/*  Input:            external queue name, entry data, data size,    */
/*                    LIFO/FIFO flag.                                */
/*                                                                   */
/*  Effects:          Memory allocated for entry.  Entry added to    */
/*                    queue.                                         */
/*                                                                   */
/*********************************************************************/
RexxReturnCode RexxEntry RexxAddQueue(
  const char *name,
  PCONSTRXSTRING data,
  size_t flag)
{
    ENTER_REXX_API(QueueManager)
    {
                                             /* first check the flag       */
        if (flag != RXQUEUE_FIFO && flag != RXQUEUE_LIFO)
        {
            return RXQUEUE_BADWAITFLAG;
        }
        if (lam->queueManager.isSessionQueue(name))
        {
            return lam->queueManager.addToSessionQueue(*data, flag);
        }
        else
        {
            return lam->queueManager.addToNamedQueue(name, *data, flag);
        }
    }
    EXIT_REXX_API();
}

/*********************************************************************/
/*                                                                   */
/*  Function:         RexxPullFromQueue()                            */
/*                                                                   */
/*  Description:      Pull an entry from a queue.                    */
/*                                                                   */
/*  Function:         Locate the queue, return its top entry to      */
/*                    the caller, and tell the queue data            */
/*                    manager to delete the entry.                   */
/*                                                                   */
/*                    If the queue is empty, the caller can elect    */
/*                    to wait for someone to post an entry.          */
/*                                                                   */
/*  Notes:            Caller is responsible for freeing the returned */
/*                    memory.                                        */
/*                                                                   */
/*                    The entry's control block is stored in the     */
/*                    entry's memory.  We must therefore obtain      */
/*                    addressability to the entry's memory before    */
/*                    we can process the entry.                      */
/*                                                                   */
/*  Input:            external queue name, wait flag.                */
/*                                                                   */
/*  Output:           queue element, data size, date/time stamp.     */
/*                                                                   */
/*  Effects:          Top entry removed from the queue.  Message     */
/*                    queued to the queue data manager.              */
/*                                                                   */
/*********************************************************************/
RexxReturnCode RexxEntry RexxPullFromQueue(
  const char *name,
  RXSTRING  *data_buf,
  REXXDATETIME *time,
  size_t waitflag)
{
    ENTER_REXX_API(QueueManager)
    {
                                            /* first check the flag       */
        if (waitflag != RXQUEUE_NOWAIT && waitflag != RXQUEUE_WAIT)
        {
            return RXQUEUE_BADWAITFLAG;
        }
        // we use a common path here, because pull is more complicated.
        // NULL for the name is the signal to use the session queue.
        if (lam->queueManager.isSessionQueue(name))
        {
            name = NULL;
        }
        return lam->queueManager.pullFromQueue(name, *data_buf, waitflag, time);
    }
    EXIT_REXX_API();
}

/*********************************************************************/
/*                                                                   */
/*  Function:        Indicated a process is terminating and should   */
/*                   remove a reference to the session queue         */
/*  Description:     Close the session queue                         */
/*                                                                   */
/*********************************************************************/
RexxReturnCode RexxEntry RexxDeleteSessionQueue()
{
    // this shuts down the entire environment
    LocalAPIManager::deleteInstance();
    return RXQUEUE_OK;
}


/**
 * Initialize the API subsystem at process startup.
 *
 * @return Always returns 0;
 */
RexxReturnCode RexxEntry RexxCreateSessionQueue()
{
    // this will initialize the API subsystem
    ENTER_REXX_API(QueueManager)
    {
        return 0;
    }
    EXIT_REXX_API();

}
