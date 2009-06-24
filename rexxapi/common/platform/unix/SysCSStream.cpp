/*----------------------------------------------------------------------------*/
/*                                                                            */
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netdb.h>

#if defined( HAVE_STRINGS_H )
# include <strings.h>
#endif
#include <errno.h>
#include "SysCSStream.hpp"

/**
 * Read from the connection.
 *
 * @param buf       Target buffer for the read operation.
 * @param bufsize   Size of the target buffer.
 * @param bytesread Number of bytes actually read.
 *
 * @return True on an error, otherwise false
 */
bool SysSocketConnection::read(void *buf, size_t bufsize, size_t *bytesread)
{
    if (c == -1)
    {
        errcode = CSERROR_IO_FAILED;
        return false;
    }
    int actual = recv(c, (char *)buf, (int)bufsize, 0);
    if (actual == -1)
    {
        // a -1 return is a bad problem.  0 might be bad, but allow the
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
bool SysSocketConnection::write(void *buf, size_t bufsize, size_t *byteswritten)
{
    if (c == -1)
    {
        errcode = CSERROR_IO_FAILED;
        return false;
    }
    int actual = send(c, (char *)buf, (int)bufsize, 0);
    if (actual == -1)
    {
        // a -1 return is a bad problem.  0 might be bad, but allow the
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
bool SysSocketConnection::write(void *buf, size_t bufsize, void *buf2, size_t buf2size, size_t *byteswritten)
{
    // if the second buffer is of zero size, we can handle without
    // copying
    if (buf2size == 0)
    {
        return write(buf, bufsize, byteswritten);
    }

    if (c == -1)
    {
        errcode = CSERROR_IO_FAILED;
        return false;
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

    int actual = send(c, buffer, (int)bufferSize, 0);
    // we're done with the buffer, regardless of whether this works or fails
    returnMessageBuffer(buffer);
    if (actual == -1)
    {
        // a -1 return is a bad problem.  0 might be bad, but allow the
        // caller to handle that one.
        errcode = CSERROR_IO_FAILED;
        return false;
    }
    *byteswritten = (size_t)actual;
    errcode = CSERROR_OK;
    return true;
}

/**
 * Get a buffer for sending a buffered message.
 *
 * @param size   The required size.
 *
 * @return A pointer to a buffer, or NULL if unable to allocate.
 */
char *SysSocketConnection::getMessageBuffer(size_t size)
{
    // if larger than our cached buffer, return
    if (size > MAX_CACHED_BUFFER)
    {
        return (char *)malloc(size);
    }
    // use our cached buffer, allocating it if required.
    if (messageBuffer == NULL)
    {
        messageBuffer = (char *)malloc(MAX_CACHED_BUFFER);
    }
    return messageBuffer;
}


/**
 * Return a message buffer after sending a message.  This will
 * either cache the buffer, or release it, depending upon
 * how it was obtained in the first place.
 *
 * @param buffer The buffer to release.
 */
void SysSocketConnection::returnMessageBuffer(void *buffer)
{
    if (buffer != messageBuffer)
    {
        free(buffer);
    }
}


/**
 * Standard constructor.
 */
SysClientStream::SysClientStream() : SysSocketConnection()
{
    domain = AF_INET;
    type = SOCK_STREAM;
    protocol = 0;
}


/**
 * Alternate constructor.
 *
 * @param name   Hostname and port in the form "hostname:port".
 */
SysClientStream::SysClientStream(const char *name) : SysSocketConnection()
{
    domain = AF_INET;
    type = SOCK_STREAM;
    protocol = 0;
    open(name);
}


/**
 * Alternate constructor.
 *
 * @param host   String name of the host.
 * @param port   Target port number.
 */
SysClientStream::SysClientStream(const char *host, int port) : SysSocketConnection()
{
    domain = AF_INET;
    type = SOCK_STREAM;
    protocol = 0;
    open(host, port);
}


/**
 * Standard destructor.
 */
SysClientStream::~SysClientStream()
{
    if (c != -1)
    {
        close();
    }
}


/**
 * Open a connection to a host/port.
 *
 * @param name   Hostname and port in the form "hostname:port".
 *
 * @return True on an error, otherwise false.
 */
bool SysClientStream::open(const char *name)
{
    // copy the host name so we can separate the host and port values.
    char *hostname = strdup(name);

    char *portstr = strstr(hostname, ":");
    if (portstr == NULL)
    {
        free(hostname);
        errcode = CSERROR_HOSTNAME_PORT;
        return false;
    }
    // split the two pieces with a null terminator.
    *portstr = '\0';
    portstr++;
    int port = atoi(portstr);
    if (port == 0)
    {
        free(hostname);
        errcode = CSERROR_HOSTNAME_PORT;
        return false;
    }
    // do the open and free the temp name value.
    bool result = open(hostname, port);
    free(hostname);
    return result;
}


/**
 * Open a connection to a host/port.
 *
 * @param host   The target host name.
 * @param port   The connection port number.
 *
 * @return True on an error, otherwise false.
 */
bool SysClientStream::open(const char *host, int port)
{
    struct sockaddr_in  addr; // address structure
    struct hostent *phe; // pointer to a host entry

    // get a socket
    c = socket(domain, type, protocol);
    if (c == -1)
    {
        errcode = CSERROR_INTERNAL;
        return false;
    }
    // convert the host entry/name to an address
    phe = gethostbyname(host);
    if (phe)
    {
        bcopy(phe->h_addr, (char *) &addr.sin_addr, sizeof(addr.sin_addr));
    }
    else
    {
        addr.sin_addr.s_addr = inet_addr(host);
    }
    if (addr.sin_addr.s_addr == INADDR_NONE)
    {
        errcode = CSERROR_HOSTNAME_PORT;
        close();
        return false;
    }
    // connect to the remote host
    addr.sin_family = domain;
    addr.sin_port = htons(port);
    if (connect(c, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        errcode = CSERROR_OPEN_FAILED;
        close();
        return false;
    }

    errcode = CSERROR_OK;
    return true;
}


/**
 * Close the connection to the host.
 *
 * @return True on an error, otherwise false.
 */
bool SysClientStream::close()
{
    if (c != -1)
    {
        ::close(c);
    }
    else
    {
        errcode = CSERROR_UNKNOWN;
        return false;
    }
    c = -1;
    errcode = CSERROR_OK;
    return true;
}


/**
 * Standard constructor.
 */
SysServerStream::SysServerStream()
{
    errcode = CSERROR_OK;
    s = -1;
    domain = AF_INET;
    type = SOCK_STREAM;
    protocol = 0;
    backlog = 20;
}


/**
 * Alternate constructor.
 *
 * @param name   Hostname and port in the form "hostname:port".
 */
SysServerStream::SysServerStream(const char *name)
{
    errcode = CSERROR_OK;
    s = -1;
    domain = AF_INET;
    type = SOCK_STREAM;
    protocol = 0;
    backlog = 20;
    make(name);
}


/**
 * Alternate constructor.
 *
 * @param port   Port number to listen on.
 */
SysServerStream::SysServerStream(int port)
{
    errcode = CSERROR_OK;
    s = -1;
    domain = AF_INET;
    type = SOCK_STREAM;
    protocol = 0;
    backlog = 20;
    make(port);
}


/**
 * Standard destructor.
 */
SysServerStream::~SysServerStream()
{
    // close our server connection.
    close();
}


/**
 * Make a sever connection.
 *
 * @param name   Hostname and port in the form "hostname:port".
 *
 * @return True on an error, otherwise false
 */
bool SysServerStream::make(const char *name)
{
    char *hostname;
    char *portstr;
    int port;

    // get host name and port strings
    hostname = strdup(name);
    portstr = strstr(hostname, ":");
    if (portstr == NULL)
    {
        free(hostname);
        errcode = CSERROR_HOSTNAME_PORT;
        return false;
    }
    *portstr = '\0';
    portstr++;
    port = atoi(portstr);
    free(hostname);
    if (port == 0)
    {
        errcode = CSERROR_HOSTNAME_PORT;
        return false;
    }
    return make(port);
}


/**
 * Make a server connection.
 *
 * @param port   Port to use for the connection.
 *
 * @return True on an error, otherwise false
 */
bool SysServerStream::make(int port)
{
    struct sockaddr_in addr; // server address structure
    int so_reuseaddr = false; // socket reuse flag

    // get a server socket
    s = socket(domain, type, protocol);
    if (s == -1)
    {
        errcode = CSERROR_UNKNOWN;
        return false;
    }
    // set the socket option to reuse the address
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&so_reuseaddr,
        sizeof(so_reuseaddr));
    // bind the server socket to a port
    memset(&addr, 0, sizeof (addr));
    addr.sin_family = domain;
    addr.sin_port = htons(port);
//    addr.sin_addr.s_addr = INADDR_ANY;
    // The following replaces the line above. It forces the socket to be bound
    // to the local interface only. Thus only the local machine will be allowed
    // to connect to this socket.
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        errcode = CSERROR_CONNX_EXISTS;
        return false;
    }
    // listen for a client at the port
    if (listen(s, backlog) == -1)
    {
        errcode = CSERROR_INTERNAL;
        return false;
    }

    errcode = CSERROR_OK;
    return true;
}


/**
 * Accept a connection from a client.
 *
 * @return True on an error, otherwise false
 */
SysServerConnection *SysServerStream::connect(void)
{
    struct sockaddr_in addr; // address structure
    socklen_t sz = sizeof(addr);

    if (s == -1)
    {
        errcode = CSERROR_INTERNAL;
        return NULL;
    }
    int c = accept(s, (struct sockaddr *) &addr, &sz);
    if (c == -1)
    {
        errcode = CSERROR_CONNX_FAILED;
        return NULL;
    }

    errcode = CSERROR_OK;
    return new SysServerConnection(this, c);
}


/**
 * Close the server connection.
 *
 * @return True on an error, otherwise false
 */
bool SysServerStream::close()
{
    if (s != -1)
    {
        ::close(s);
        s = -1;
    }
    else
    {
        errcode = CSERROR_INTERNAL;
        return false;
    }
    errcode = CSERROR_OK;
    return true;
}


/**
 * Server connection constructor.
 *
 * @param s      The parent server connection.
 * @param socket The socket for the connection.
 */
SysServerConnection::SysServerConnection(SysServerStream *s, int socket) : SysSocketConnection(socket)
{
    server = s;
}

/**
 * Standard destructor.
 */
SysServerConnection::~SysServerConnection()
{
    disconnect();
}


/**
 * Is the connection with the localhost?
 *
 *
 * @return True if the client is at address 127.0.0.1, otherwise false
 */
bool SysServerConnection::isLocalConnection(void)
{
    sockaddr_in  addr;
    int          rc;
    socklen_t    nameLen;

    if (c == -1)
    {
        return false;
    }
    nameLen = sizeof(addr);
    rc = getpeername(c,(struct sockaddr *)&addr,&nameLen);
    if (rc)
    {
        return false;
    }
    if (strcmp("127.0.0.1", inet_ntoa(addr.sin_addr)) != 0)
    {
        return false;
    }
    return true;
}


/**
 * Close the connection to the host.
 *
 * @return True on an error, otherwise false.
 */
bool SysServerConnection::disconnect()
{
    if (c != -1)
    {
        close(c);
        c = -1;
    }
    else
    {
        errcode = CSERROR_INTERNAL;
        return false;
    }
    errcode = CSERROR_OK;
    return true;
}

