/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2022 Rexx Language Association. All rights reserved.    */
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


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SysCSNamedPipeStream.hpp"
#include "ServiceException.hpp"
#include "SysProcess.hpp"
#include "oorexxapi.h"

#include "aclapi.h"
#include "sddl.h"

// the pipe name used for this userid
const char *SysServerNamedPipeConnectionManager::userPipeName = NULL;
// the named mutex used for this userid
const char *SysServerNamedPipeConnectionManager::serverMutexName = NULL;


/**
 * Read from the connection.
 *
 * @param buf       Target buffer for the read operation.
 * @param bufsize   Size of the target buffer.
 * @param bytesread Number of bytes actually read.
 *
 * @return True on an error, otherwise false
 */
bool SysNamedPipeConnection::read(void *buf, size_t bufsize, size_t *bytesread)
{
    if (pipeHandle == INVALID_HANDLE_VALUE)
    {
        errcode = CSERROR_IO_FAILED;
        return false;
    }

    DWORD actual = 0;

    // try to read from the pipe
    if (!ReadFile(pipeHandle, buf, (DWORD)bufsize, &actual, NULL))
    {
        // a faiure return is a bad problem.  0 might be bad, but allow the
        // caller to handle that one.
        errcode = CSERROR_IO_FAILED;
        return false;
    }
    *bytesread = (size_t)actual;
    errcode = CSERROR_OK;
    return true;
}


/**
 * Write a buffer to the connection.
 *
 * @param buf     Source buffer for the write operation.
 * @param bufsize Size of the source buffer.
 * @param byteswritten
 *                Number of bytes actually written to the connection.
 *
 * @return True on an error, otherwise false
 */
bool SysNamedPipeConnection::write(void *buf, size_t bufsize, size_t *byteswritten)
{
    if (pipeHandle == INVALID_HANDLE_VALUE)
    {
        errcode = CSERROR_IO_FAILED;
        return false;
    }

    DWORD actual = 0;

    // write the data to the pipe
    if (!WriteFile(pipeHandle, buf, (DWORD)bufsize, &actual, NULL))
    {
        // a false return is a bad problem.  A 0 length, might be bad, but allow the
        // caller to handle that one.
        errcode = CSERROR_IO_FAILED;
        return false;
    }
    *byteswritten = (size_t)actual;
    errcode = CSERROR_OK;
    return true;
}


/**
 * Write a multi-buffer message to the connection.
 *
 * @param buf     Source buffer for the write operation.
 * @param bufsize Size of the source buffer.
 * @param byteswritten
 *                Number of bytes actually written to the connection.
 *
 * @return True on an error, otherwise false
 */
bool SysNamedPipeConnection::write(void *buf, size_t bufsize, void *buf2, size_t buf2size, size_t *byteswritten)
{
    // if the second buffer is of zero size, we can handle without
    // copying
    if (buf2size == 0)
    {
        return write(buf, bufsize, byteswritten);
    }

    size_t bufferSize = bufsize + buf2size;

    // get a buffer large enough for both buffer
    char *buffer = getMessageBuffer(bufferSize);
    // if we can't get a buffer, then try sending this in pieces
    if (buffer == NULL)
    {
        // write the first buffer
        if (!write(buf, bufsize, byteswritten))
        {
            return false;
        }
        size_t buf2written = 0;
        if (!write(buf2, buf2size, &buf2written))
        {
            return false;
        }
        *byteswritten += buf2written;
        return true;
    }

    // copy the message and attached data into a single buffer
    memcpy(buffer, buf, bufsize);
    memcpy(buffer + bufsize, buf2, buf2size);

    // perform the write now
    write(buffer, bufferSize, byteswritten);

    // we're done with the buffer, regardless of whether this works or fails
    returnMessageBuffer(buffer);
    return true;
}


/**
 * Close the server connection.
 *
 * @return True on an error, otherwise false
 */
bool SysNamedPipeConnection::disconnect()
{
    if (pipeHandle != INVALID_HANDLE_VALUE)
    {
        // this ensures no data is lost
        FlushFileBuffers(pipeHandle);
        DisconnectNamedPipe(pipeHandle);
        CloseHandle(pipeHandle);
        pipeHandle = INVALID_HANDLE_VALUE;
        errcode = CSERROR_OK;
        return true;
    }
    else
    {
        errcode = CSERROR_INTERNAL;
        return false;
    }
}


/**
 * Open a connection to a named pipe.
 *
 * @param pipeName The name of the target pipe
 *
 * @return True on success, otherwise false.
 */
bool SysNamedPipeConnection::connect(const char *pipeName)
{
    // we may need to retry this a few times if things are busy
    while (1)
    {
        // try to get a handle for the pipe
        pipeHandle = CreateFile(pipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        // if we got a valid handle back, we're done.
        if (pipeHandle != INVALID_HANDLE_VALUE)
        {
            errcode = CSERROR_OK;
            return true;
        }

      // Exit if an error other than ERROR_PIPE_BUSY occurs.
      if (GetLastError() != ERROR_PIPE_BUSY)
      {
          errcode = CSERROR_CONNX_FAILED;
          return false;
      }

      // All pipe instances are busy, so wait for a second.

      if (!WaitNamedPipe(pipeName, 1000))
      {
          errcode = CSERROR_CONNX_FAILED;
          return false;
      }
   }
}


/**
 * Accept a connection from a client.
 *
 * @return True on an error, otherwise false
 */
ApiConnection *SysServerNamedPipeConnectionManager::acceptConnection()
{
    // because daemon process is launched from a running rexx process, it
    // inherits the security profile of launching process. If that process has
    // elevated priviledges, non-priviledged processes cannot connect to the daemon
    // named pipe because of the default security profile. We need to override
    // this so all connections can be accepted. This profile will allow any connections,
    // which is not really ideal. Better would be to have a security profile that allows
    // connections only from the same logged in user, but have two months of wrestling
    // with trying to understand the Windows security APIs, I gave up and implemented
    // something that works. I welcome anybody to take a stab at doing this.

    SECURITY_ATTRIBUTES sa = {0};
    SECURITY_DESCRIPTOR sd = { 0 };

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

    sa.bInheritHandle = false;
    sa.lpSecurityDescriptor = &sd;
    sa.nLength = sizeof(sa);

    // We need to create a new named pipe instance for each inbound connection
    HANDLE clientHandle = CreateNamedPipe(userPipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE |
        PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS, PIPE_UNLIMITED_INSTANCES, 1024, 1024, 0, &sa);

    if (clientHandle == INVALID_HANDLE_VALUE)
    {
        errcode = CSERROR_CONNX_FAILED;
        return NULL;
    }

    // Wait for the client to connect; if it succeeds,
    // the function returns a nonzero value. If the function
    // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
    bool connected = ConnectNamedPipe(clientHandle, NULL) ? true : (GetLastError() == ERROR_PIPE_CONNECTED);

    // we got a connection error
    if (!connected)
    {
        errcode = CSERROR_CONNX_FAILED;
        return NULL;
    }

    errcode = CSERROR_OK;
    // now make a connection object that can be used to read and write the client requests.
    return new SysNamedPipeConnection(clientHandle);
}


/**
 * Bind to the named pipe by creating the first instance of the
 * pipe with the given name.
 *
 * @return true if the pipe could be created successfully, false for any creation failures.
 */
bool SysServerNamedPipeConnectionManager::bind()
{
    // generate the unique name for this user.
    generatePipeName();
    // Try to create our unique named mutex. If this fails, there must be another instance running,
    // so we'll just shutdown right now.
    // we are the first creator of this pipe. We use this to detect if we have more
    // than one occurrence of rxapi running for a given userid.
    serverMutexHandle = CreateMutex(NULL, FALSE, serverMutexName);
    // If the mutex already exists CreateMutex returns a handle to it,
    // but GetLastError returns ERROR_ALREADY_EXISTS
    if (serverMutexHandle == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        errcode = CSERROR_CONNX_FAILED;
        return false;
    }
    return true;
}


/**
 * Close the connection to the host.
 *
 * @return True on an error, otherwise false.
 */
bool SysServerNamedPipeConnectionManager::disconnect()
{
    // we don't use the initial instance, but do keep the handle open to
    // ensure a second instance can't start up
    if (serverMutexHandle != NULL)
    {
        CloseHandle(serverMutexHandle);
        serverMutexHandle = NULL;
        // this is only done when the server is shutting down prior
        // to termination. We don't really need to get rid of this, but
        // it is good practice
        free((void *)userPipeName);
        userPipeName = NULL;
        free((void *)serverMutexName);
        serverMutexName = NULL;
    }

    // this doesn't really rely on any persistent state for the connections, so
    // this is a noop that will always succeed.
    errcode = CSERROR_OK;
    return true;
}


/**
 * Generate a unique string to be used for interprocess
 * communications for this userid. Also generates the unique
 * mutex name used to ensure only one instance exists.
 *
 * @return A unique identifier used to create the named pipes.
 */
const char *SysServerNamedPipeConnectionManager::generatePipeName()
{
    // if we've already generated this, we're done
    if (userPipeName != NULL)
    {
        return userPipeName;
    }

    // a buffer for generating the name
    char pipeNameBuffer[MAX_PATH];
    char userid[MAX_USERID_LENGTH]; // name of the user

    // get the userid from the process
    SysProcess::getUserID(userid);

    snprintf(pipeNameBuffer, sizeof(pipeNameBuffer), "\\\\.\\pipe\\ooRexx %d.%d.%d-%s-%s", ORX_VER, ORX_REL, ORX_MOD,
#ifdef __REXX64__
	    "64",
#else
		"32",
#endif
        userid);
    userPipeName = strdup(pipeNameBuffer);
    // we also create a named mutex to ensure we have a only a single instance running.
    // we need a different name because backslashes are not allowed in mutex names.

    snprintf(pipeNameBuffer, sizeof(pipeNameBuffer), "ooRexx %d.%d.%d-%s-%s", ORX_VER, ORX_REL, ORX_MOD,
#ifdef __REXX64__
	    "64",
#else
		"32",
#endif
        userid);

    serverMutexName = strdup(pipeNameBuffer);
    return userPipeName;
}


