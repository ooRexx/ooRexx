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

#include "RegistrationManager.hpp"
#include "ServiceMessage.hpp"
#include "ServiceException.hpp"


/**
 * Create registration data for a library registration item.
 *
 * @param n       The callback name.
 * @param m       The callback library.
 * @param regData The additional registration data sent with the message.
 */
RegistrationData::RegistrationData(const char *n, const char *m, SessionID s, ServiceRegistrationData *regData)
{
    next = NULL;
    name = dupString(n);
    moduleName = dupString(m);
    procedureName = dupString(regData->procedureName);
    owner = s;
    dropAuthority = regData->dropAuthority;
    userData[0] = regData->userData[0];
    userData[1] = regData->userData[1];
    entryPoint = 0;
    references = NULL;
}

/**
 * Register an inprocess item.
 *
 * @param n       The name of the callback.
 * @param s       The session id.
 * @param regData The service registration data.
 */
RegistrationData::RegistrationData(const char *n, SessionID s, ServiceRegistrationData *regData)
{
    next = NULL;
    name = dupString(n);
    moduleName = NULL;
    procedureName = NULL;
    owner = s;
    dropAuthority = regData->dropAuthority;
    userData[0] = regData->userData[0];
    userData[1] = regData->userData[1];
    entryPoint = regData->entryPoint;
    references = NULL;
}

/**
 * Destructor for a registration data item.
 */
RegistrationData::~RegistrationData()
{
    delete [] name;
    delete [] moduleName;
    delete [] procedureName;

    SessionCookie *cookie = references;
    while (cookie != NULL)
    {
        SessionCookie *localnext = cookie->next;
        delete cookie;
        cookie = localnext;
    }
}


/**
 * Copy the registration information into a message
 * data item to be returned to the client.
 *
 * @param regData The returned registration data.
 */
void RegistrationData::getRegistrationData(ServiceRegistrationData &regData)
{
    if (moduleName != NULL)
    {
        strcpy(regData.moduleName, moduleName);
    }
    else
    {
        strcpy(regData.moduleName, "");
    }

    if (procedureName != NULL)
    {
        strcpy(regData.procedureName, procedureName);
    }
    else
    {
        strcpy(regData.procedureName, "");
    }
    regData.userData[0] = userData[0];
    regData.userData[1] = userData[1];
    regData.entryPoint = entryPoint;
    regData.dropAuthority = dropAuthority;
}

/**
 * Add an additional reference to a session.
 *
 * @param s      The session id to add.
 */
void RegistrationData::addSessionReference(SessionID s)
{
    SessionCookie *cookie = findSessionReference(s);
    // already there?  just add a reference.
    if (cookie != NULL)
    {
        cookie->addReference();
    }
    else
    {
        cookie = new SessionCookie(s);
        cookie->next = references;
        references = cookie;
    }
}

/**
 * Decrement a session reference count.
 *
 * @param s      The session identifier.
 */
void RegistrationData::removeSessionReference(SessionID s)
{
    SessionCookie *cookie = findSessionReference(s);
    // already there?  just add a reference.
    if (cookie != NULL)
    {
        if (cookie->removeReference() == 0)
        {
            removeSessionReference(cookie);
        }
    }
}

/**
 * Locate a session reference cound.
 *
 * @param s      The target session identifier.
 *
 * @return The session cookie associated with the session, or NULL
 *         if the session has not been tracked yet.
 */
SessionCookie *RegistrationData::findSessionReference(SessionID s)
{
    SessionCookie *cookie = references;
    while (cookie != NULL)
    {
        if (cookie->session == s)
        {
            return cookie;
        }
        cookie = cookie->next;
    }
    return NULL;
}

/**
 * Remove a session reference cookie from the chain.
 *
 * @param s      The cookit to remove.
 */
void RegistrationData::removeSessionReference(SessionCookie *s)
{
    if (s == references)
    {
        references = s->next;
    }
    else
    {
        SessionCookie * current = references;
        while (current != NULL)
        {
            if (current->next == s)
            {
                current->next = s->next;
                break;
            }
            current = current->next;
        }
    }
    delete s;
}



// Add a library registration item to the table.
// Message arguments have the following meanings:
//
// parameter1 -- registration type
// nameArg    -- name of the registered object
void RegistrationTable::registerLibraryCallback(ServiceMessage &message)
{
    ServiceRegistrationData *regData = (ServiceRegistrationData *)message.getMessageData();
    // get the argument names
    const char *name = message.nameArg;
    const char *module = regData->moduleName;

    RegistrationData *callback = locate(name, module);
    // update the reference counts to make sure drops don't
    // clear things out for other processes.
    if (callback != NULL)
    {
        callback->addSessionReference(message.session);
        message.setResult(REGISTRATION_COMPLETED);
    }
    else
    {
        callback = new RegistrationData(name, module, message.session, regData);

        // add to the chain
        callback->next = firstLibrary;
        firstLibrary = callback;

        if (locate(name, message.session) != NULL)
        {
            message.setResult(DUPLICATE_REGISTRATION);
        }
        else
        {
            message.setResult(REGISTRATION_COMPLETED);
        }
    }

    // make sure the data message buffer is not passed back.
    message.freeMessageData();
}


// Add an exe registration item to the table.
// Message arguments have the following meanings:
//
// parameter1 -- registration type
// parameter2 -- drop authority flag
// nameArg    -- The registration name
void RegistrationTable::registerCallback(ServiceMessage &message)
{
    ServiceRegistrationData *regData = (ServiceRegistrationData *)message.getMessageData();
    // get the argument name
    const char *name = message.nameArg;

    // now locate an exe registration.
    RegistrationData *callback = locate(name, message.session);
    // update the reference counts to make sure drops don't
    // clear things out for other processes.
    if (callback != NULL)
    {
        message.setResult(DUPLICATE_REGISTRATION);
    }
    else
    {
        callback = new RegistrationData(name, message.session, regData);

        // add to the chain
        callback->next = firstEntryPoint;
        firstEntryPoint = callback;

        // see if this is duplicated in library form
        if (locate(firstLibrary, name) != NULL)
        {
            message.setResult(DUPLICATE_REGISTRATION);
        }
        else
        {
            message.setResult(REGISTRATION_COMPLETED);
        }
    }
    // make sure the data message buffer is not passed back.
    message.freeMessageData();
}

// General query by name only.  Can return either form of registraction.
// Message arguments have the following meanings:
//
// parameter1 -- registration type
// nameArg    -- The registration name
void RegistrationTable::queryCallback(ServiceMessage &message)
{
    // get the argument name (local copy only)
    const char *name = message.nameArg;

    // now check the exe version first.
    RegistrationData *callback = locate(name, message.session);
    // not found?  try a library version
    if (callback == NULL || callback->owner != message.session)
    {
        callback = locate(firstLibrary, name);
    }
    // copy the data into the buffer if we found one
    if (callback != NULL)
    {
        ServiceRegistrationData *regData = (ServiceRegistrationData *)message.allocateMessageData(sizeof(ServiceRegistrationData));
        // copy the registration information
        callback->getRegistrationData(*regData);
        message.setResult(CALLBACK_EXISTS);
    }
    else
    {
        message.setResult(CALLBACK_NOT_FOUND);
    }
}


// General query by name and qualified module name.  Can return only the EXE version
// Message arguments have the following meanings:
//
// parameter1 -- registration type
// nameArg    -- The registration name
void RegistrationTable::queryLibraryCallback(ServiceMessage &message)
{
    // we're sent an extra registration block here with input data.  We can just
    // reuse this buffer to send the information back.
    ServiceRegistrationData *regData = (ServiceRegistrationData *)message.getMessageData();
    // get the argument name (local copy only)
    const char *name = message.nameArg;
    const char *module = regData->moduleName;

    // if not requesting by module name, handle like a normal request
    if (strlen(module) == 0)
    {
        queryCallback(message);
        return;
    }

    // now check a library version first.
    RegistrationData *callback = locate(name, module);
    // copy the data if we found this
    if (callback != NULL)
    {
        // copy the registration information
        callback->getRegistrationData(*regData);
        message.setResult(CALLBACK_EXISTS);
    }
    else
    {
        message.setResult(CALLBACK_NOT_FOUND);
        // make sure the data message buffer is not passed back.
        message.freeMessageData();
    }
}


// Update a library-based callback after a successful load event.
// Message arguments have the following meanings:
//
// parameter1 -- registration type
// nameArg    -- The registration name
void RegistrationTable::updateCallback(ServiceMessage &message)
{
    ServiceRegistrationData *regData = (ServiceRegistrationData *)message.getMessageData();
    // get the argument name (local copy only)
    const char *name = message.nameArg;
    const char *module = regData->moduleName;

    // now check a library version first.
    RegistrationData *callback = locate(name, module);
    // copy the data if we found this
    if (callback != NULL)
    {
        // we're only updating the entry point data
        callback->entryPoint = regData->entryPoint;
        message.setResult(CALLBACK_EXISTS);
    }
    else
    {
        message.setResult(CALLBACK_NOT_FOUND);
    }
    // make sure the data message buffer is not passed back.
    message.freeMessageData();
}


// Drop a callback by name only.
// Message arguments have the following meanings:
//
// parameter1 -- registration type
// nameArg    -- The registration name
void RegistrationTable::dropCallback(ServiceMessage &message)
{
    // get the argument name (local copy only)
    const char *name = message.nameArg;
    RegistrationData **anchor = &firstEntryPoint;

    // now check the exe version first.
    RegistrationData *callback = locate(name, message.session);
    // not found?  try a library version
    if (callback == NULL)
    {
        callback = locate(firstLibrary, name);
        anchor = &firstLibrary;
    }
    if (callback != NULL)
    {
        // an attempt to drop by somebody other than the owner?
        if (callback->dropAuthority == OWNER_ONLY && callback->owner != message.session)
        {
            message.setResult(DROP_NOT_AUTHORIZED);
        }
        else
        {

            // remove this session reference.
            callback->removeSessionReference(message.session);
            // still referenced by other processes?
            if (callback->hasReferences())
            {
                message.setResult(DROP_NOT_AUTHORIZED);
            }
            else
            {
                remove(anchor, callback);
                delete callback;
                message.setResult(CALLBACK_DROPPED);
            }
        }
    }
    else
    {
        message.setResult(CALLBACK_NOT_FOUND);
    }
    // make sure the data message buffer is not passed back.
    message.freeMessageData();
}


// Drop a callback by qualified name.  Can only drop a library registration.
// Message arguments have the following meanings:
//
// parameter1 -- registration type
// nameArg    -- The registration name
void RegistrationTable::dropLibraryCallback(ServiceMessage &message)
{
    // we're sent an extra registration block here with input data.  We can just
    // reuse this buffer to send the information back.
    ServiceRegistrationData *regData = (ServiceRegistrationData *)message.getMessageData();
    // get the argument name (local copy only)
    const char *name = message.nameArg;
    const char *module = regData->moduleName;

    // if not requesting by module name, handle like a normal request
    if (strlen(module) == 0)
    {
        queryCallback(message);
        return;
    }

    // now check a library version first.
    RegistrationData *callback = locate(name, module);

    // copy the data into the buffer if we found one
    if (callback != NULL)
    {
        // an attempt to drop by somebody other than the owner?
        if (callback->dropAuthority == OWNER_ONLY && callback->owner != message.session)
        {
            message.setResult(DROP_NOT_AUTHORIZED);
        }
        else
        {

            // remove this session reference.
            callback->removeSessionReference(message.session);
            // still referenced by other processes?
            if (callback->hasReferences())
            {
                message.setResult(DROP_NOT_AUTHORIZED);
            }
            else
            {
                if (callback->isEntryPoint())
                {
                    remove(&firstEntryPoint, callback);
                }
                else
                {
                    remove(&firstLibrary, callback);
                }
                delete callback;
                message.setResult(CALLBACK_DROPPED);
            }
        }
    }
    else
    {
        message.setResult(CALLBACK_NOT_FOUND);
    }
    // make sure the data message buffer is not passed back.
    message.freeMessageData();
}

/**
 * search for a name-only registration
 *
 * @param anchor The chain anchor.
 * @param name   The target callback name.
 *
 * @return The callback descriptor or NULL if the item is not found.
 */
RegistrationData *RegistrationTable::locate(RegistrationData *anchor, const char *name)
{
    RegistrationData *current = anchor;
    RegistrationData *previous = NULL;

    while (current != NULL)              /* while more queues          */
    {
        // find the one we want?
        if (current->matches(name))
        {
            return current;
        }
        previous = current;                /* remember this block        */
        current = current->next;           /* step to the next block     */
    }
    return NULL;
}

/**
 * search for a name-only registration and remove it from
 * the chain.
 *
 * @param anchor The chain anchor position.
 * @param block  The block to locate.
 */
void RegistrationTable::remove(RegistrationData **anchor, RegistrationData *block)
{
    RegistrationData *current = *anchor;
    RegistrationData *previous = NULL;

    while (current != NULL)              /* while more queues          */
    {
        // find the one we want?
        if (current == block)
        {
            // is this the first one?
            if (previous == NULL)
            {
                // update the anchor position
                *anchor = current->next;
            }
            else
            {
                // close the chain and get out of here
                previous->next = current->next;
            }
            return;
        }
        previous = current;                /* remember this block        */
        current = current->next;           /* step to the next block     */
    }
}

/**
 * search for a library-type registration, qualified by
 * name and library.
 *
 * @param name   The callback name.
 * @param module The target module.
 *
 * @return The descriptor for the item, or NULL if not found.
 */
RegistrationData *RegistrationTable::locate(const char *name, const char *module)
{
    RegistrationData *current = firstLibrary;
    RegistrationData *previous = NULL;

    while (current != NULL)              /* while more queues          */
    {
        // find the one we want?
        if (current->matches(name, module))
        {
            // move this to the front so we find it quickly
            reorderBlocks(firstLibrary, current, previous);
            return current;
        }
        previous = current;                /* remember this block        */
        current = current->next;           /* step to the next block     */
    }
    return NULL;
}

/**
 * search for a library-type registration
 *
 * @param name   The target name.
 *
 * @return The descriptor for the callback, or NULL if it doesn't
 *         exist.
 */
RegistrationData *RegistrationTable::locate(const char *name)
{
    RegistrationData *callback = locate(firstLibrary, name);
    if (callback == NULL)
    {
        callback = locate(firstEntryPoint, name);
    }
    return callback;
}

/**
 * search for a local type registration
 *
 * @param name    The target registration name.
 * @param session The session identifier.
 *
 * @return The registration data for the item, or NULL if not
 *         found.
 */
RegistrationData *RegistrationTable::locate(const char *name, SessionID session)
{
    RegistrationData *current = firstEntryPoint;
    RegistrationData *previous = NULL;

    while (current != NULL)              /* while more queues          */
    {
        // find the one we want?
        if (current->matches(name, session))
        {
            // move this to the front so we find it quickly
            reorderBlocks(firstEntryPoint, current, previous);
            return current;
        }
        previous = current;                /* remember this block        */
        current = current->next;           /* step to the next block     */
    }
    return NULL;
}


/**
 * Reorder the registration blocks so that we put the
 * most recently referenced registrations at the front
 * of the queue.
 *
 * @param anchor   The chain anchor.
 * @param current  The block we're reordering.
 * @param previous The previous block in the chain (can be NULL if this
 *                 item is already at the head of the chain).
 */
void RegistrationTable::reorderBlocks(RegistrationData *& anchor, RegistrationData *current, RegistrationData *previous)
{
    if (previous != NULL)            // if we have a predecessor
    {
        // rearrange to get "most recently used" behavior
        previous->next = current->next;
        current->next = anchor;
        anchor = current;
    }
}


/**
 * It will remove all the registration entries for a specific process
 *
 * @param session The session identifier.
 */
void RegistrationTable::freeProcessEntries(SessionID session)
{
    RegistrationData *current = firstEntryPoint;
    RegistrationData *previous = NULL;

    while (current != NULL)
    {
        if (current->owner == session)
        {
            if (previous == NULL)
            {
                firstEntryPoint = current->next;
                delete current;
                current = firstEntryPoint;
            }
            else
            {
                previous->next = current->next;
                delete current;
                current = current->next;
            }
        }
        else
        {
            previous = current;
            current = current->next;
        }
    }
}



/**
 * It will remove all the registration entries for a specific process
 *
 * @param session The session identifier.
 */
void ServerRegistrationManager::freeProcessRegistrations(SessionID session)
{
    // delete all of the entries associated with this process.
    functions.freeProcessEntries(session);
    exits.freeProcessEntries(session);
    commandHandlers.freeProcessEntries(session);
}


/**
 * Dispatch a registration operation to the appropriate
 * subsystem handler.
 *
 * @param message The inbound message.
 */
void ServerRegistrationManager::dispatch(ServiceMessage &message)
{
    RegistrationTable *table = NULL;
    switch ((RegistrationType)message.parameter1)
    {
        case FunctionAPI:
            table = &functions;
            break;
        case ExitAPI:
            table = &exits;
            break;
        case SubcomAPI:
            table = &commandHandlers;
            break;
        default:
            message.setExceptionInfo(SERVER_FAILURE, "Invalid registration type");
            return;
    }


    // now that we have the appropriate subsystem targetted, dispatch the
    // real operation.
    switch (message.operation)
    {
        // registration manager operations
        case REGISTER_LIBRARY:
            table->registerLibraryCallback(message);
            break;
        case REGISTER_ENTRYPOINT:
            table->registerCallback(message);
            break;
        case REGISTER_DROP:
            table->dropCallback(message);
            break;
        case REGISTER_DROP_LIBRARY:
            table->dropLibraryCallback(message);
            break;
        case REGISTER_QUERY:
            table->queryCallback(message);
            break;
        case REGISTER_QUERY_LIBRARY:
            table->queryLibraryCallback(message);
            break;
        case REGISTER_LOAD_LIBRARY:
            table->queryCallback(message);
            break;
        case UPDATE_CALLBACK:
            table->updateCallback(message);
            break;

        default:
            message.setExceptionInfo(SERVER_FAILURE, "Invalid registration manager operation");
            // make sure the data message buffer is not passed back.
            message.freeMessageData();
            break;
    }
}


/**
 * Perform any cleanup of process-specific resources
 * when the process terminates.
 *
 * @param session The session id of the session going away.
 */
void ServerRegistrationManager::cleanupProcessResources(SessionID session)
{
    // just free up any registrations associated with this
    // session.
    freeProcessRegistrations(session);
}
