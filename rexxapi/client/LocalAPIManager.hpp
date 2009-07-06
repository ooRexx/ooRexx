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

#ifndef LocalAPIManager_HPP_INCLUDED
#define LocalAPIManager_HPP_INCLUDED

#include "LocalRegistrationManager.hpp"
#include "LocalQueueManager.hpp"
#include "LocalMacroSpaceManager.hpp"
#include "SysProcess.hpp"
#include "SysSemaphore.hpp"
#include "SysCSStream.hpp"

#include <list>

class LocalAPIContext;


class LocalAPIManager
{
public:

    enum {
        MAX_CONNECTIONS = 3     // Limit on the number of connections we keep active.
    };

    LocalAPIManager()
    {
        connectionEstablished = false;
        session = 0;
    }

    inline ~LocalAPIManager() { }

    static LocalAPIManager *getInstance();
    static void deleteInstance();

    void initProcess();
    void terminateProcess();

    inline SessionID getSession() { return session; }
    inline void getUserID(char *buffer) { strcpy(buffer, userid); }
    void connectToAPIServer();
    void establishServerConnection();
    RexxReturnCode processServiceException(ServerManager t, ServiceException *e);
    void shutdown();
    SysClientStream *getConnection();
    void returnConnection(SysClientStream *);
    void closeConnection(SysClientStream *connection);

protected:

    static LocalAPIManager* singleInstance;  // the single local instance
    static SysMutex messageLock;             // threading synchronizer
    bool           connectionEstablished;    // local initialization state
    SessionID      session;                  // the session identifier
    char           userid[MAX_USERID_LENGTH]; // name of the user
    std::list<SysClientStream *> connections; // connection pool

public:                                  // let's make these public
    LocalQueueManager queueManager;      // our managers for the different API sets
    LocalRegistrationManager registrationManager;
    LocalMacroSpaceManager macroSpaceManager;
};


#endif
