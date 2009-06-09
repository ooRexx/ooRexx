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

#ifndef APIServer_HPP_INCLUDED
#define APIServer_HPP_INCLUDED

#include "ServiceMessage.hpp"
#include "MacroSpaceManager.hpp"
#include "RegistrationManager.hpp"
#include "QueueManager.hpp"
#include "SysSemaphore.hpp"
#include <list>

class APIServerInstance;
class APIServerThread;

class APIServer
{
public:
    APIServer() : lock(), server(), serverActive(false), instances(NULL), terminatedThreads() { }
    virtual ~APIServer() { ; }
    void terminateServer();
    void initServer();
    void listenForConnections();
    void processMessages(SysServerConnection *connection);
    ServiceReturn shutdown();
    virtual bool isStoppable();
    void cleanupProcessResources(ServiceMessage &message);
    virtual void stop();
    void requestLock();
    void releaseLock();
    APIServerInstance *getInstance(ServiceMessage &m);
    void dispatch(ServiceMessage &message);
    void sessionTerminated(APIServerThread *thread);
    void cleanupTerminatedSessions();

protected:

    SysMutex  lock;                   // out server sync semaphore.
    SysServerStream server;           // our server message pipeline
    bool serverActive;                // the server is running
    APIServerInstance *instances;     // our chain of active instances
    std::list<APIServerThread *> terminatedThreads; // connection pool
};


class ServerLock
{
public:
    ServerLock(APIServer *s)
    {
        server = s;
        server->requestLock();
    }

    ~ServerLock()
    {
        server->releaseLock();
    }

protected:
    APIServer *server;       // our api server instance.
};


#endif
