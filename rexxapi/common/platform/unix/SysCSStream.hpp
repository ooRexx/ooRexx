/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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


#ifndef SysCSStream_Included
#define SysCSStream_Included

#include <netinet/in.h>
#include "CSStream.hpp"

/**
 * A socket implementation of a CSStream for Unix. This is a
 * base class that reads writes to a socket independently of how
 * the socket is instantiate.
 */
class SysSocketConnection : public ApiConnection
{
public:
    inline SysSocketConnection() : c(-1), ApiConnection() { }
    inline SysSocketConnection(int sock) : c(sock), ApiConnection() { }
    inline ~SysSocketConnection() { }

    bool read(void *buf, size_t bufsize, size_t *bytesread) override;
    bool write(void *buf, size_t bufsize, size_t *byteswritten) override;
    bool write(void *buf, size_t bufsize, void*buf2, size_t buf2size, size_t *byteswritten) override;

    bool disconnect() override;

protected:
    int c; // stream socket
};


/**
 * This is the client stream for an AF_UNIX style connection.
 */
class SysLocalSocketConnection : public SysSocketConnection
{
public:
    SysLocalSocketConnection() : SysSocketConnection() { };
    SysLocalSocketConnection(const char *service);
    ~SysLocalSocketConnection() { };

    bool connect(const char *serviceName);
};


/**
 * Base class for all socket-based server connections. The subclasses
 * provide all of the setup around the socket.
 */
class SysServerSocketConnectionManager : public ServerConnectionManager
{
public:
    inline SysServerSocketConnectionManager() : c(-1) { }
    inline ~SysServerSocketConnectionManager() { }

    bool disconnect() override;
    ApiConnection *acceptConnection() override;

protected:
    int c; // stream socket
    const char *boundServiceName;        // the service we are bound to
};

/**
 * Implementation class for a socket connection bound to
 * an AF_UNIX stream
 */
class SysServerLocalSocketConnectionManager : public SysServerSocketConnectionManager
{
public:
    inline SysServerLocalSocketConnectionManager() { }
    inline ~SysServerLocalSocketConnectionManager() { }

    bool bind(const char *serviceName);

    static const char *generateServiceName();
    static void getServiceLocation(char *path, size_t len);

protected:

    bool checkServiceName(const char *serviceName);

    const char *boundServiceName;        // the service name this instance is bound to.
    static const char *userServiceName;  // the standard service name generated for each user.
};
#endif
