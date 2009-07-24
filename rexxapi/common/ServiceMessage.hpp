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

#ifndef ServiceMessage_HPP_INCLUDED
#define ServiceMessage_HPP_INCLUDED

#include "rexx.h"
#include "ServiceException.hpp"
#include "SysCSStream.hpp"
#include "SysProcess.hpp"

typedef enum
{
    QueueWait                 // deferred because of queue wait issues.

} ExecutionDeferral;

/**
 * Duplicate a C string using C++ new char[] to allocate the storage.
 *
 * @param oldString The string we're duplicating.
 *
 * @return The duplicated string.
 */
inline char *dupString(const char *oldString)
{
    char *newString = new char[strlen(oldString) + 1];
    strcpy(newString, oldString);
    return newString;
}

// this port is assigned by IANA for the use of ooRexx, so
// in theory, we're safe to use this.
#define REXX_API_PORT 10010

// a session identifier
typedef uintptr_t SessionID;

// definitions of server message targets
typedef enum
{
    QueueManager,
    RegistrationManager,
    MacroSpaceManager,
    APIManager

} ServerManager;

typedef enum
{
    FunctionAPI,
    SubcomAPI,
    ExitAPI,
} RegistrationType;

typedef enum
{
    // macro space operations
    ADD_MACRO,
    ITERATE_MACRO_DESCRIPTORS,
    NEXT_MACRO_DESCRIPTOR,
    GET_MACRO_IMAGE,
    GET_MACRO_DESCRIPTOR,
    CLEAR_MACRO_SPACE,
    REMOVE_MACRO,
    QUERY_MACRO,
    REORDER_MACRO,
    MACRO_SEND_NEXT,
    ITERATE_MACROS,
    NEXT_MACRO_IMAGE,
    MACRO_RETRIEVE_NEXT,

    // queue manager operations
    NEST_SESSION_QUEUE,
    CREATE_SESSION_QUEUE,
    CREATE_NAMED_QUEUE,
    DELETE_SESSION_QUEUE,
    DELETE_NAMED_QUEUE,
    GET_SESSION_QUEUE_COUNT,
    GET_NAMED_QUEUE_COUNT,
    ADD_TO_NAMED_QUEUE,
    ADD_TO_SESSION_QUEUE,
    PULL_FROM_NAMED_QUEUE,
    PULL_FROM_SESSION_QUEUE,
    CLEAR_SESSION_QUEUE,
    CLEAR_NAMED_QUEUE,
    OPEN_NAMED_QUEUE,
    QUERY_NAMED_QUEUE,

    // registration manager operations
    REGISTER_LIBRARY,
    REGISTER_ENTRYPOINT,
    REGISTER_DROP,
    REGISTER_DROP_LIBRARY,
    REGISTER_QUERY,
    REGISTER_QUERY_LIBRARY,
    REGISTER_LOAD_LIBRARY,
    UPDATE_CALLBACK,

    // global API operations
    SHUTDOWN_SERVER,
    PROCESS_CLEANUP,
    CONNECTION_ACTIVE,
    CLOSE_CONNECTION,
} ServerOperation;

typedef enum
{
    MESSAGE_OK,
    SERVER_ERROR,

    // macro space results
    NO_MORE_MACROS,
    MACRO_ADDED,
    MACRO_SEND_NEXT_BUFFER,
    MACRO_REMOVED,
    MACRO_SPACE_CLEARED,
    PREORDER_MACRO,
    POSTORDER_MACRO,
    MACRO_ORDER_CHANGED,
    MACRO_ITERATION_STARTED,
    MACRO_RETURNED,
    MACRO_IMAGE_RETURNED,
    MACRO_RETRIEVE_NEXT_BUFFER,
    MACRO_DOES_NOT_EXIST,

    // queue manager results
    DUPLICATE_QUEUE_NAME,
    QUEUE_SEND_NEXT,
    QUEUE_PULL_NEXT,
    QUEUE_NO_DATA_AVAILABLE,
    QUEUE_ITEM_ADDED,
    QUEUE_ITEM_PULLED,
    QUEUE_EMPTY,
    QUEUE_CREATED,
    QUEUE_DELETED,
    QUEUE_EXISTS,
    QUEUE_OK,
    QUEUE_DOES_NOT_EXIST,
    QUEUE_IN_USE,

    // registration manager results
    REGISTRATION_COMPLETED,
    DUPLICATE_REGISTRATION,
    CALLBACK_EXISTS,
    CALLBACK_DROPPED,
    CALLBACK_NOT_FOUND,
    DROP_NOT_AUTHORIZED,

    // API manager results
    SERVER_STOPPED,
    SERVER_NOT_STOPPABLE

}  ServiceReturn;

// message specific modifiers
typedef enum
{
    QUEUE_FIFO,
    QUEUE_LIFO,
    QUEUE_WAIT_FOR_DATA,
    QUEUE_NO_WAIT,

    OWNER_ONLY,
    DROP_ANY,
    REXXAPI_VERSION = 100                 // current Rexx api version.
}  ServiceMessageParameters;


class ServiceRegistrationData
{
public:
    inline ServiceRegistrationData(const char *module, const char *proc, bool drop, const char *userPointer)
    {
        strncpy(moduleName, module, MAX_NAME_LENGTH);
        strncpy(procedureName, proc, MAX_NAME_LENGTH);
        dropAuthority = !drop ? DROP_ANY : OWNER_ONLY;
        setUserData(userPointer);
        entryPoint = 0;
    }

    inline ServiceRegistrationData(const char *module)
    {
        if (module != NULL)
        {
            strncpy(moduleName, module, MAX_NAME_LENGTH);
        }
        else
        {
            strcpy(moduleName, "");
        }
        strcpy(procedureName, "");
        dropAuthority = DROP_ANY;
        setUserData(NULL);
        entryPoint = 0;
    }

    inline ServiceRegistrationData(REXXPFN entry, const char *userPointer)
    {
        strcpy(moduleName, "");
        strcpy(procedureName, "");
        entryPoint = (uintptr_t)entry;
        dropAuthority = OWNER_ONLY;
        setUserData(userPointer);
    }


    inline void setUserData(const char *userPointer)
    {
        // we have two bits of user data to copy
        if (userPointer != NULL)
        {
            userData[0] = ((uintptr_t*)userPointer)[0];
            userData[1] = ((uintptr_t*)userPointer)[1];
        }
        else
        {
            userData[0] = 0;
            userData[1] = 0;
        }
    }


    inline void retrieveUserData(char *userPointer)
    {
        // we have two bits of user data to copy
        if (userPointer != NULL)
        {
            ((uintptr_t*)userPointer)[0] = userData[0];
            ((uintptr_t*)userPointer)[1] = userData[1];
        }
    }

    enum
    {
        MAX_NAME_LENGTH = 256
    };

    char moduleName[MAX_NAME_LENGTH];          // name of the library
    char procedureName[MAX_NAME_LENGTH];       // the procedure within the library
    size_t dropAuthority;                      // scope of drop authority
    uintptr_t userData[2];                     // saved user data
    uintptr_t entryPoint;                      // explicit entry point address
};


class ServiceMessage
{
public:
    enum
    {
        NAMESIZE = 256
    };

    ServiceMessage();
    inline ~ServiceMessage() { ; }

    inline void setResult(ServiceReturn code)
    {
        result = code;
        errorCode = NO_ERROR_CODE;
    }

    inline void setExceptionInfo(ErrorCode error, const char *message)
    {
        result = SERVER_ERROR;
        errorCode = error;
        strncpy(nameArg, message, NAMESIZE);
        freeMessageData();   // make sure we don't send back any attached data
    }

    inline void setExceptionInfo(ServiceException *e)
    {
        result = SERVER_ERROR;
        errorCode = e->getErrorCode();
        strncpy(nameArg, e->getMessage(), NAMESIZE);
        freeMessageData();   // make sure we don't send back any attached data
    }

    inline void clearExceptionInfo()
    {
        result = MESSAGE_OK;
        errorCode = NO_ERROR_CODE;
    }

    inline void raiseServerError()
    {
        if (errorCode != NO_ERROR_CODE)
        {
            // an error condition happened on the other side, raise this as an exception.
            throw new ServiceException(errorCode, nameArg);
        }
    }

    // makes a local copy of the string data stored in the argument name
    inline char *getArgumentName()
    {
        return dupString(nameArg);
    }

    inline void clearMessageData()
    {
        messageData = NULL;
        messageDataLength = 0;
    }

    inline void *getMessageData() { return messageData; }
    inline size_t getMessageDataLength() { return messageDataLength; }

    inline void transferMessageData(RXSTRING &data)
    {
        // if provided a buffer, then use it if large enough
        if (data.strptr != NULL && messageDataLength < data.strlength)
        {
            memcpy(data.strptr, messageData, messageDataLength);
            data.strlength = messageDataLength;
            // we need to free our copy
            freeMessageData();
        }
        else
        {

            MAKERXSTRING(data, (char *)messageData, messageDataLength);
            // we've given up ownership of this data, so clear out the
            // pointers so we don't try to free
            messageData = NULL;
            messageDataLength = 0;
        }
    }


    inline void setMessageData(void *data, size_t length)
    {
        freeMessageData();    // make sure any existing data is cleared.
        messageData = data;
        messageDataLength = length;
        // make sure we only set a pointer if this is non-zero, else
        // we end up transmitting a stale pointer to the other side of the connection.
        if (length == 0)
        {
            messageData = NULL;
        }
        // this is not freeable data
        retainMessageData = true;
    }


    inline void *allocateMessageData(size_t length)
    {
        messageData = allocateResultMemory(length);
        messageDataLength = length;
        retainMessageData = false;
        return messageData;
    }

    inline void freeMessageData()
    {
        if (messageData != NULL && !retainMessageData)
        {
            // this is real message information
            releaseResultMemory(messageData);
            clearMessageData();
        }
    }

    void readMessage(SysServerConnection *server);
    void writeResult(SysServerConnection *server);
    void writeMessage(SysClientStream &server);
    void readResult(SysClientStream &server);

    static void *allocateResultMemory(size_t length);
    static void  releaseResultMemory(void *mem);

    ServerManager messageTarget;         // end receiver of the message
    ServerOperation operation;           // operation to be performed
    SessionID session;                   // the sender of the message
    uintptr_t parameter1;                // first parameter passed
    uintptr_t parameter2;                // second parameter passed
    uintptr_t parameter3;                // the third parameter passed
    uintptr_t parameter4;                // the fourth parameter passed
    uintptr_t parameter5;                // the fifth parameter passed
    ServiceReturn result;                // return result
    ErrorCode errorCode;                 // error code from other side
    void     *messageData;               // extra data attached to the message.
    size_t    messageDataLength;         // size of the extra data.
    bool      retainMessageData;         // indicates the server should not release result memory.
    char      nameArg[NAMESIZE];         // buffer for name arguments
    char      userid[MAX_USERID_LENGTH];    // name of the user
};

#endif
