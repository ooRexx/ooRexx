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


#include "APIServerInstance.hpp"
#include <new>
#include "ServiceMessage.hpp"
#include "ServiceException.hpp"


APIServerInstance::APIServerInstance(ServiceMessage &m) : queueManager(), registrationManager(), macroSpaceManager(), next(NULL)
{
    strcpy(userid, m.userid);
}

/**
 * Dispatch an API server control message.
 *
 * @param message The control message parameter.
 */
void APIServerInstance::dispatch(ServiceMessage &message)
{
    // each target handles its own dispatch.
    switch (message.messageTarget)
    {
        case QueueManager:
        {
            queueManager.dispatch(message);
            break;
        }
        case RegistrationManager:
        {
            registrationManager.dispatch(message);
            break;
        }
        case MacroSpaceManager:
        {
            macroSpaceManager.dispatch(message);
            break;
        }
        case APIManager:
        {
        }
    }
}

/**
 * Cleanup sessions specific resources after a Rexx process
 * terminates.
 *
 * @param message The service message with the session information.
 */
void APIServerInstance::cleanupProcessResources(ServiceMessage &message)
{
    queueManager.cleanupProcessResources(message.session);
    registrationManager.cleanupProcessResources(message.session);
    macroSpaceManager.cleanupProcessResources(message.session);
}

/**
 * Test to see if the api server is in a state where it can be
 * stopped.  A stoppable state implies there are no session
 * specific resources currently active in the server.
 *
 * @return True if the server is stoppable, false otherwise.
 */
bool APIServerInstance::isStoppable()
{
    return queueManager.isStoppable() &&
        registrationManager.isStoppable() &&
        macroSpaceManager.isStoppable();
}

