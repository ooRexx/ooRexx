/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2008-2022 Rexx Language Association. All rights reserved.    */
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

#ifndef SysCSPipeStream_Included
#define SysCSPipeStream_Included

#include <windows.h>
#include "CSStream.hpp"

class SysNamedPipeConnection : public ApiConnection
{
public:
    inline SysNamedPipeConnection() : pipeHandle(INVALID_HANDLE_VALUE), ApiConnection() { }
    inline SysNamedPipeConnection(HANDLE h) : pipeHandle(h), ApiConnection() { }
    inline SysNamedPipeConnection(const char *pipe) : pipeHandle(INVALID_HANDLE_VALUE), ApiConnection() { connect(pipe); }
    inline ~SysNamedPipeConnection() { }

    bool read(void *buf, size_t bufsize, size_t *bytesread) override;
    bool write(void *buf, size_t bufsize, size_t *byteswritten) override;
    bool write(void *buf, size_t bufsize, void*buf2, size_t buf2size, size_t *byteswritten) override;

    bool disconnect() override;

    bool connect(const char *pipeName);

protected:

    HANDLE pipeHandle;              // The pipe handle
};


/**
 * Base class for all socket-based server connections. The subclasses
 * provide all of the setup around the socket.
 */
class SysServerNamedPipeConnectionManager : public ServerConnectionManager
{
public:
    inline SysServerNamedPipeConnectionManager() : serverMutexHandle(NULL), ServerConnectionManager() { generatePipeName(); }
    inline ~SysServerNamedPipeConnectionManager() { }

    bool disconnect() override;
    ApiConnection *acceptConnection() override;

    bool bind();

    static const char *generatePipeName();

protected:
    static const char *userPipeName;  // the standard pipe name generated for each user.
    static const char *serverMutexName;  // the standard pipe name generated for each user.

    HANDLE serverMutexHandle;         // the handle of the mutex we create to ensure a single instance
};

#endif

