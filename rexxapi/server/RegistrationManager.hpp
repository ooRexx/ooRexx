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

#ifndef RegistrationManager_HPP_INCLUDED
#define RegistrationManager_HPP_INCLUDED

#include "rexx.h"
#include "ServiceMessage.hpp"
#include "Utilities.hpp"
#include "SysSemaphore.hpp"

class SessionCookie
{
public:
    SessionCookie(SessionID s)
    {
        next = NULL;
        session = s;
        references = 1;
    }

    inline void addReference() { references++; }
    inline size_t removeReference() { return --references; }

    SessionCookie *next;               // the next in the chain
    SessionID session;                 // the session id for the registering process
    size_t references;                 // number of nested references from this session
};


class RegistrationData
{
public:
    RegistrationData(const char * n, const char * m, SessionID s, ServiceRegistrationData *regData);
    RegistrationData(const char * n, SessionID s, ServiceRegistrationData *regData);
    ~RegistrationData();

    inline bool matches(const char *n, const char *m) { return Utilities::strCaselessCompare(name, n) == 0 && Utilities::strCaselessCompare(moduleName, m) == 0; }
    inline bool matches(const char *n, SessionID s) { return s == owner && Utilities::strCaselessCompare(name, n) == 0; }
    inline bool matches(const char *n) { return Utilities::strCaselessCompare(name, n) == 0; }
    inline bool hasReferences() { return references != 0; }
    inline bool isLibrary() { return moduleName != NULL; }
    inline bool isEntryPoint() { return moduleName == NULL; }

    void getRegistrationData(ServiceRegistrationData &regData);
    void addSessionReference(SessionID s);
    void removeSessionReference(SessionID s);
    SessionCookie *findSessionReference(SessionID s);
    void removeSessionReference(SessionCookie *s);

    RegistrationData *next;            // next block in the chaing
    const char *name;                  // the registered name
    const char *moduleName;            // module name for object (optional)
    const char *procedureName;         // procedure name (optional)
    uintptr_t userData[2];             // user area information
    uintptr_t entryPoint;              // target entry point
    size_t  dropAuthority;             // Permission to drop
    SessionID owner;                   // Pid of Registrant
    SessionCookie *references;         // references to this registration from other processes
};


class RegistrationTable
{
public:
    RegistrationTable()
    {
        firstEntryPoint = NULL;
        firstLibrary = NULL;
    }

    void registerLibraryCallback(ServiceMessage &message);
    void registerCallback(ServiceMessage &message);
    void queryCallback(ServiceMessage &message);
    void queryLibraryCallback(ServiceMessage &message);
    void dropCallback(ServiceMessage &message);
    void dropLibraryCallback(ServiceMessage &message);
    void updateCallback(ServiceMessage &message);
    RegistrationData *locate(RegistrationData *anchor, const char *name);
    RegistrationData *locate(const char *name, const char *module);
    RegistrationData *locate(const char *name);
    RegistrationData *locate(const char *name, SessionID session);
    void remove(RegistrationData **anchor, RegistrationData *block);
    void reorderBlocks(RegistrationData *& anchor, RegistrationData *current, RegistrationData *previous);
    void freeProcessEntries(SessionID session);
    inline bool hasEntries()
    {
        return firstEntryPoint != NULL || firstLibrary != NULL;
    }
    inline bool isEmpty()
    {
        return !hasEntries();
    }

protected:
    RegistrationData *firstEntryPoint;    // first reference in the entrypoint registration chain
    RegistrationData *firstLibrary;       // first reference in the library chain
};


// the server side of the registration pipe
class ServerRegistrationManager
{
public:
    ServerRegistrationManager() : functions(), exits(), commandHandlers(), lock() { lock.create(); }
    void terminateServer();
    // It will remove all the registration entries for a specific process
    void freeProcessRegistrations(SessionID session);
    void dispatch(ServiceMessage &message);
    void cleanupProcessResources(SessionID session);

    inline bool isStoppable()
    {
        return functions.isEmpty() && exits.isEmpty() && commandHandlers.isEmpty();
    }

protected:
    RegistrationTable functions;         // our tables for the 2 registration types
    RegistrationTable exits;
    RegistrationTable commandHandlers;
    SysMutex          lock;             // our subsystem lock
};

#endif
