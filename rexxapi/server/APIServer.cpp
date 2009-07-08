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


#include "APIServer.hpp"
#include "APIServerInstance.hpp"
#include "APIServerThread.hpp"
#include <new>
#include "ServiceMessage.hpp"
#include "ServiceException.hpp"
#include <stdio.h>

/**
 * Initialize the server side of the operation.
 */
void APIServer::initServer()
{
    // able to initialize our communications pipe?
    if (!server.make(REXX_API_PORT))
    {
        throw new ServiceException(SERVER_FAILURE, "RexxAPIServer::initServer() Failure creating server stream");
    }

    lock.create();         // create the mutex.
    serverActive = true;
}


/**
 * Do server shutdown processing and resource cleanup.
 */
void APIServer::terminateServer()
{
    // flip the sign over to the closed side.
    server.close();
    serverActive = false;
}


/**
 * Process the Rexx API requests as a queue of messages.  Each
 * message is handled through to completion, so the message
 * queue is the synchronization point.
 */
void APIServer::listenForConnections()
{
    ServiceMessage message;

    while (serverActive)
    {
        // get a new connection.
        SysServerConnection *connection = server.connect();
        // we might have some terminated threads waiting
        // for final resource cleanup...this is a good place to
        // check for this.
        cleanupTerminatedSessions();
        // we have some sort of resource problem...force termination and shutdown.
        if (connection == NULL)
        {
            break;
        }
        // create a new thread to service this client connection
        APIServerThread *thread = new APIServerThread(this, connection);
        thread->start();
    }
}

/**
 * Handle a session termination event.
 *
 * @param thread
 */
void APIServer::sessionTerminated(APIServerThread *thread)
{
    // we need to hold the lock while handling this
    ServerLock(this);
    // add to the queue for cleanup on the next opportunity
    terminatedThreads.push_back(thread);
}



/**
 * Cleanup the resources devoted to threads that have
 * terminated.
 */
void APIServer::cleanupTerminatedSessions()
{
    // we need to hold the lock while handling this
    ServerLock(this);

    // clean up the connection pools
    while (!terminatedThreads.empty())
    {
        APIServerThread *thread = terminatedThreads.front();
        terminatedThreads.pop_front();
        // shut down the resources for this thread and release the memory
        thread->terminate();
        delete thread;
    }
}


/**
 * Process the Rexx API requests as a queue of messages.  Each
 * message is handled through to completion, so the message
 * queue is the synchronization point.
 */
void APIServer::processMessages(SysServerConnection *connection)
{
    while (serverActive)
    {
        ServiceMessage message;
        try
        {
            // read the message.
            message.readMessage(connection);
        } catch (ServiceException *e)
        {
            // an error here is likely caused by the client closing the connection.
            // delete both the exception and the connection and terminate the thread.
            delete e;
            delete connection;
            return;
        }

        message.result = MESSAGE_OK;     // unconditionally zero the result
        // each target handles its own dispatch.
        switch (message.messageTarget)
        {
            case QueueManager:
            case RegistrationManager:
            case MacroSpaceManager:
            {
                getInstance(message)->dispatch(message);
                break;
            }

            // general API control message.
            case APIManager:
            {
                // this could be a shutdown operation
                if (message.operation == CLOSE_CONNECTION)
                {
                    connection->disconnect();
                    delete connection;
                    return;
                }

                dispatch(message);
                break;
            }
        }

        try
        {
            // ping the message back to the caller
            message.writeResult(connection);
        } catch (ServiceMessage *e)
        {
            // an error here is likely caused by the client closing the connection.
            // delete both the exception and the connection and terminate the thread.
            delete e;
            delete connection;
            return;
        }
    }
}


/**
 * Dispatch an API server control message.
 *
 * @param message The control message parameter.
 */
void APIServer::dispatch(ServiceMessage &message)
{
    message.result = MESSAGE_OK;
    switch (message.operation)
    {
        case SHUTDOWN_SERVER:
        {
            shutdown();
            break;
        }

        // TODO:  Make sure process cleanup is driven
        case PROCESS_CLEANUP:
        {
            cleanupProcessResources(message);
            break;
        }

        // This is an "are you there" ping.  Pass back the version information as a parameter.
        case CONNECTION_ACTIVE:
        {
            message.parameter1 = REXXAPI_VERSION;
            break;
        }

        default:
            message.setExceptionInfo(SERVER_FAILURE, "Invalid API manager operation");
            break;
    }
}

/**
 * Cleanup sessions specific resources after a Rexx process
 * terminates.
 *
 * @param message The service message with the session information.
 */
void APIServer::cleanupProcessResources(ServiceMessage &message)
{
    getInstance(message)->cleanupProcessResources(message);
}


/**
 * Cause the API server to shutdown.
 *
 * @return SERVER_STOPPED if a stoppage is possible, SERVER_NOT_STOPPED
 *         if it was not in a stoppable state.
 */
ServiceReturn APIServer::shutdown()
{
    // any processing running Rexx active?
    if (isStoppable())
    {
        serverActive = false;
        return SERVER_STOPPED;
    }
    return SERVER_NOT_STOPPABLE;
}

/**
 * Stop the server.
 */
void APIServer::stop()
{
    // set the stop flag and wake up the message loop
    serverActive = false;
}

/**
 * Test to see if the api server is in a state where it can be
 * stopped.  A stoppable state implies there are no session
 * specific resources currently active in the server.
 *
 * @return True if the server is stoppable, false otherwise.
 */
bool APIServer::isStoppable()
{
    APIServerInstance *current = instances;
    while (current != NULL)
    {
        if (!current->isStoppable())
        {
            return false;
        }
        current = current->next;
    }
    return true;
}


/**
 * Get the server instance associate with a particular userid,
 * creating a new instance if this is the first time we've processed
 * a request for this id.
 *
 * @param m      The service message associated with the request.
 *
 * @return A pointer to the correct instance.
 */
APIServerInstance *APIServer::getInstance(ServiceMessage &m)
{
    // synchronize access on this
    ServerLock(this);

    APIServerInstance *current = instances;
    while (current != NULL)
    {
        if (current->isUser(m))
        {
            return current;
        }
        current = current->next;
    }

    current = new APIServerInstance(m);
    current->next = instances;
    instances = current;
    return current;
}


/**
 * Request the global server API lock.
 */
void APIServer::requestLock()
{
    lock.request();
}


/**
 * Release the global server API lock.
 */
void APIServer::releaseLock()
{
    lock.release();
}
