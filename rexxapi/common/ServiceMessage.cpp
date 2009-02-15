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

#include "ServiceMessage.hpp"
#include "ServiceException.hpp"
#include "SysAPIManager.hpp"


ServiceMessage::ServiceMessage()
{
    messageTarget = APIManager;
    operation = CONNECTION_ACTIVE;
    session = 0;
    parameter1 = 0;
    parameter2 = 0;
    parameter3 = 0;
    parameter4 = 0;
    parameter5 = 0;
    result = MESSAGE_OK;
    errorCode = NO_ERROR_CODE;
    messageData = NULL;
    messageDataLength = 0;
    retainMessageData = false;
    nameArg[0] = '\0';
    userid[0] = '\0';
}

/**
 * Read a message from the server-side stream.
 *
 * @param server The server stream that has already received a connection message.
 */
void ServiceMessage::readMessage(SysServerConnection *connection)
{
    size_t actual = 0;
    size_t required = sizeof(ServiceMessage);
    while (required > 0)
    {
        if (!connection->read((void *)this, required, &actual) || actual == 0)
        {
            throw new ServiceException(SERVER_FAILURE, "ServiceMessage::readMessage() Failure reading service message");
        }
        required -= actual;
    }

    // does this message have extra data associated with it?
    if (messageDataLength != 0)
    {
        // allocate a data buffer for the extra information and read it in.
        messageData = allocateResultMemory(messageDataLength);
        required = messageDataLength;
        while (required > 0)
        {
            if (!connection->read(messageData, required, &actual) || actual == 0)
            {
                releaseResultMemory(messageData);
                // make sure these are cleared out
                messageData = NULL;
                messageDataLength = 0;
                throw new ServiceException(SERVER_FAILURE, "ServiceMessage::readMessage() Failure reading service message");
            }
            // add in the count
            required -= actual;
        }
        // this is a releasable value
        retainMessageData = false;
    }
}


/**
 * Write a server side message result back to the client.
 *
 * @param server The server message stream used to receive the original message.
 */
void ServiceMessage::writeResult(SysServerConnection *connection)
{
    size_t expected = sizeof(ServiceMessage) + messageDataLength;
    size_t actual = 0;
    if (!connection->write((void *)this, sizeof(ServiceMessage), messageData, messageDataLength, &actual) || actual != expected)
    {
        freeMessageData();
        throw new ServiceException(SERVER_FAILURE, "ServiceMessage::writeResult() Failure writing service message result");
    }
    // we might be sending a copy of data that's still resident in the connection->  If
    // we are, then don't delete the message data after doing the send.
    // free the message data after the send.
    freeMessageData();
}


/**
 * Write a message over to the connection->
 *
 * @param pipe   The pipe we've opened to write the message.
 */
void ServiceMessage::writeMessage(SysClientStream &pipe)
{
    size_t actual = 0;
    size_t expected = sizeof(ServiceMessage) + messageDataLength;
    if (!pipe.write((void *)this, sizeof(ServiceMessage), messageData, messageDataLength, &actual) || actual != expected)
    {
        freeMessageData();
        throw new ServiceException(SERVER_FAILURE, "ServiceMessage::writeResult() Failure writing service message result");
    }
    // make sure we free and release any attached data before proceeding
    freeMessageData();
}


/**
 * Read a message result back from a server message.
 *
 * @param pipe   The connection used to send the original message.
 */
void ServiceMessage::readResult(SysClientStream &pipe)
{
    size_t actual = 0;
    size_t required = sizeof(ServiceMessage);
    while (required > 0)
    {
        if (!pipe.read((void *)this, required, &actual) || actual == 0)
        {
            throw new ServiceException(SERVER_FAILURE, "ServiceMessage::readResult() Failure reading service message");
        }
        required -= actual;
    }

    // handle any errors that the server side might have raised.
    raiseServerError();

    // does this message have extra data associated with it?
    if (messageDataLength != 0)
    {
        // allocate a data buffer for the extra information and read it in.
        // we add a courtesy null terminator on this message, so we read one extra bit
        messageData = allocateResultMemory(messageDataLength + 1);
        ((char *)messageData)[messageDataLength] = '\0';
        required = messageDataLength;
        while (required > 0)
        {
            if (!pipe.read(messageData, required, &actual) || actual == 0)
            {
                releaseResultMemory(messageData);
                throw new ServiceException(SERVER_FAILURE, "ServiceMessage::readResult() Failure reading service message");
            }
            // remove the amount read
            required -= actual;
        }
    }
    else
    {
        // make sure this is nulled out
        messageData = NULL;
    }
}


/**
 * Allocate a buffer for result information returned by a service
 * message.  This is allocated using the Rexx API allocation
 * routine so that the memory can be handed out and deallocated
 * by an API caller.
 *
 * @param length The length to allocate.
 *
 * @return A data pointer with the result memory.
 */
void *ServiceMessage::allocateResultMemory(size_t length)
{
    void *data = SysAPIManager::allocateMemory(length);
    if (data == NULL)
    {
        throw new ServiceException(MEMORY_ERROR, "ServiceMessage::allocateResultMemory() Failure allocating result memory");
    }
    return data;
}

/**
 * Free result memory in the case of a read failure.
 *
 * @param data   The data to be released.
 */
void ServiceMessage::releaseResultMemory(void *data)
{
    SysAPIManager::releaseMemory(data);
}
