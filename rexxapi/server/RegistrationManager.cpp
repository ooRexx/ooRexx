/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
            message.setExceptionInfo(INVALID_OPERATION, "Invalid registration type");
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
            message.setExceptionInfo(INVALID_OPERATION, "Invalid registration manager operation");
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
