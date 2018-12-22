/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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

#include "LocalAPIManager.hpp"
#include "SysLocalAPIManager.hpp"
#include "ClientMessage.hpp"
#include "SynchronizedBlock.hpp"
#include "SysThread.hpp"
#include <list>
#include <stdio.h>

// initialize static variables
LocalAPIManager* LocalAPIManager::singleInstance = NULL;
SysMutex LocalAPIManager::messageLock(true, true);

/**
 * Get the singleton instance of the local API manager.
 *
 * @return The pointer to the created (and initialized) API manager.
 */
LocalAPIManager *LocalAPIManager::getInstance()
{
    Lock lock(messageLock);                     // make sure we single thread this
    if (singleInstance == NULL)
    {
        // create an intialize this.  If this fails, an exception is thrown
        singleInstance = new LocalAPIManager();
        // to the process-specific initialization now.
        singleInstance->initProcess();
    }
    else {
        // if we shut everything down at interpreter termination, reestablish the connections.
        if (singleInstance->restartRequired)
        {
            // resetting this will prevent us from going into conversion loops.
            singleInstance->restartRequired = false;
            // we could have been shutdown by a previous interpreter termination.
            // make sure we have a connection
            singleInstance->establishServerConnection();
        }
    }
    return singleInstance;
}


/**
 * Shutdown the instance of the API manager.
 */
void LocalAPIManager::shutdownInstance()
{
    Lock lock(messageLock);                     // make sure we single thread this
    if (singleInstance != NULL)
    {
        // shutdown any connections with the server
        singleInstance->shutdownConnections();
        // mark that we need a restart
        singleInstance->restartRequired = true;
    }
}


/**
 * Process a service exception, with appropriate error handling.
 *
 * @param t      The context API manager.
 * @param e      The caught exception object.
 *
 * @return The appropriate return value for the exception.
 */
RexxReturnCode LocalAPIManager::processServiceException(ServerManager t, ServiceException *e)
{
    // process differently based on the target
    switch (t)
    {
        case QueueManager:
        {
            return queueManager.processServiceException(e);
        }

        case RegistrationManager:
        {
            return registrationManager.processServiceException(e);
        }

        case MacroSpaceManager:
        {
            return macroSpaceManager.processServiceException(e);
        }

        case APIManager:   // these should be screened out ahead of time
        {
            // all global exceptions are rxapi failures
            return RXAPI_NORXAPI;

        }
    }
    return RXAPI_MEMFAIL;    // a catch-all return code
}


/**
 * Perform process termination cleanup.
 */
void LocalAPIManager::terminateProcess()
{
    // terminate each of the subsystems.
    // start with the queue manager, since it may require access to
    // the server
    queueManager.terminateProcess();
    macroSpaceManager.terminateProcess();
    registrationManager.terminateProcess();

    // shutdown the connections
    shutdownConnections();
}


/**
 * Perform connection cleanup after an interpreter instance
 * stops
 */
void LocalAPIManager::shutdownConnections()
{
    // the queue manager needs to terminate the session queue first

    // clean up the connection pools
    while (!connections.empty())
    {
        ApiConnection *connection = connections.front();
        connections.pop_front();
        // tell the server we're going away and clean up
        closeConnection(connection);
    }
    // no active connections
    connectionEstablished = false;
}


/**
 * Perform process-specific client API initialization.
 */
void LocalAPIManager::initProcess()
{
    // we don't need a restart since we're just starting up
    restartRequired = false;

    // 1) get our session id and userid information.
    session = SysProcess::getPid();
    SysProcess::getUserID(userid);

    ServiceException *startError = NULL;

    try
    {
        // Initialization steps:
        // 2) make sure the global environment is started
        establishServerConnection();
    } catch (ServiceException *e)
    {
        // save this for potential rethrowing after the managers are
        startError = e;
    }

    // 3) initialize the API subsystems
    registrationManager.initializeLocal(this);
    macroSpaceManager.initializeLocal(this);
    queueManager.initializeLocal(this);

    // if we had an exception trying to connect to rxapi, reraise that exception now.
    if (startError != NULL)
    {
        throw startError;
    }
}


/**
 * Ensure that the daemon process managing the API information is initialized and running
 * before doing any API related activities.
 */
void LocalAPIManager::establishServerConnection()
{
    if (!connectionEstablished)
    {
        ClientMessage message(APIManager, CONNECTION_ACTIVE);
        try
        {
            // try to establish a connection to the api server.  If this returns without an exception,
            // we're good to go.
            message.send();

            if (message.parameter1 != REXXAPI_VERSION)
            {
                throw new ServiceException(API_FAILURE,  "Open Object REXX version conflict.  Incorrect version of RxAPI server detected");
            }
            connectionEstablished = true;
            return;
        }
        catch (ServiceException *e)
        {
            // just fall through, but get rid of the exception object first.
            delete e;
        }

        // Unable to connect, so try to start the server process.  We're unsynchronized at this point,
        // so it is possible multiple processes will launch this at the same time.  Only one will bind to
        // the listening port, the others will silently fail.
        SysLocalAPIManager::startServerProcess();

        // give this a little time to get launched before trying to connect.
        SysThread::sleep(50);

        // now loop multiple times, with a sleep interval, until we finally connect.
        int count = 200;
        while (count-- > 0)
        {
            try
            {
                // try to establish a connection to the api server.  If this returns without an exception,
                // we're good to go.
                message.send();

                if (message.parameter1 != REXXAPI_VERSION)
                {
                    throw new ServiceException(API_FAILURE,  "Open Object REXX version conflict.  Incorrect version of RxAPI server detected");
                }
                connectionEstablished = true;
                return;
            }
            catch (ServiceException *e)
            {
                // just fall through.
                delete e;
            }

            // use a minimal sleep time before trying again.
            // if the server is truly casters up, this could result in a full
            // second delay on every launch, but that really should not happen.
            SysThread::sleep(10);
        }
        throw new ServiceException(API_FAILURE, "Object REXX API Manager could not be started.  This could be due to a version conflict!");
    }
}

/**
 * Request a connection from the connection pool
 *
 * @return An active connection to the data server.
 */
ApiConnection *LocalAPIManager::getConnection()
{
    {
        Lock lock(messageLock);                     // make sure we single thread this
        // if we have an active connection, grab it from the cache and
        // reuse it.
        if (!connections.empty())
        {
            ApiConnection *connection = connections.front();
            connections.pop_front();
            return connection;
        }
    }

    // get an appropriately configured connection from the system definitions
    return SysLocalAPIManager::newClientConnection();
}


/**
 * Return a connection after use.
 *
 * @param connection The returned connection.
 */
void LocalAPIManager::returnConnection(ApiConnection *connection)
{
    // if we've encountered an error, then just delete the connection
    if (!connection->isClean())
    {
        delete connection;
        return;
    }

    {
        Lock lock(messageLock);                     // make sure we single thread this
        if (connections.size() < MAX_CONNECTIONS)
        {
            connections.push_back(connection);
            return;
        }
    }
    // not cachable, make sure this is delete.
    delete connection;
}


/**
 * Close a connection to the server.
 *
 * @param connection The connection to close.
 */
void LocalAPIManager::closeConnection(ApiConnection *connection)
{
    ClientMessage message(APIManager, CLOSE_CONNECTION);

    try
    {
        // this is a one-way message...we don't expect a reply
        message.writeMessage(*connection);
    }
    catch (ServiceException *e)
    {
        // ignored
        delete e;
    }
    delete connection;
}


/**
 * Send the shutdown message to the API daemon.
 */
void LocalAPIManager::shutdown()
{

    // first parameter for these calls is ALWAYS the type
    ClientMessage message(APIManager, SHUTDOWN_SERVER);
    message.send();
}
