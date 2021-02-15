/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
#include <winsock2.h>
#include "SysCSStream.hpp"
#include "ServiceException.hpp"

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
 * Close the server connection.
 *
 * @return True on an error, otherwise false
 */
bool SysSocketConnection::disconnect()
{
    if (c != -1)
    {
        closesocket(c);
        c = -1;
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
 * Alternate constructor.
 *
 * @param host   String name of the host.
 * @param port   Target port number.
 */
SysInetSocketConnection::SysInetSocketConnection(const char *host, int port) : SysSocketConnection()
{
    connect(host, port);
}


/**
 * Open a connection to a host/port.
 *
 * @param host   The target host name.
 * @param port   The connection port number.
 *
 * @return True on an error, otherwise false.
 */
bool SysInetSocketConnection::connect(const char *host, int port)
{
    struct sockaddr_in addr; // address structure
    struct hostent *phe; // pointer to a host entry
    WSADATA wsaData;

    // initialize Win sockets
    if (WSAStartup(MAKEWORD(2,0), &wsaData))
    {
        errcode = CSERROR_UNKNOWN;
    }

    // make sure we're not already connected.
    if (c != -1)
    {
        errcode = CSERROR_ALREADY_CONNECTED;
        return false;
    }

    // get a socket
    c = socket(AF_INET, SOCK_STREAM, 0);
    if (c == -1)
    {
        errcode = CSERROR_INTERNAL;
        return false;
    }
    // convert the host entry/name to an address
    phe = gethostbyname(host);
    if (phe)
    {
        memcpy(&addr.sin_addr, phe->h_addr, sizeof(addr.sin_addr));
    }
    else
    {
        addr.sin_addr.s_addr = inet_addr(host);
    }
    if (addr.sin_addr.s_addr == INADDR_NONE)
    {
        errcode = CSERROR_HOSTNAME_PORT;
        closesocket(c);
        return false;
    }
    // connect to the remote host
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::connect(c, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        errcode = CSERROR_OPEN_FAILED;
        closesocket(c);
        c = -1;
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
ApiConnection *SysServerSocketConnectionManager::acceptConnection()
{
    struct sockaddr_in addr; // address structure
    int sz = sizeof(addr);

    if (c == -1)
    {
        errcode = CSERROR_INTERNAL;
        return NULL;
    }
    SOCKET client = accept(c, (struct sockaddr *) &addr, &sz);
    if (client == -1)
    {
        errcode = CSERROR_CONNX_FAILED;
        return NULL;
    }

    errcode = CSERROR_OK;
    // now create an object wrapper for this client connection.
    return new SysSocketConnection(client);
}


/**
 * Close the connection to the host.
 *
 * @return True on an error, otherwise false.
 */
bool SysServerSocketConnectionManager::disconnect()
{
    if (c != -1)
    {
        closesocket(c);
        c = -1;
        errcode = CSERROR_OK;
        return true;
    }
    else
    {
        errcode = CSERROR_UNKNOWN;
        return false;
    }
}


/**
 * Make a server connection.
 *
 * @param port   Port to use for the connection.
 *
 * @return True on an error, otherwise false
 */
bool SysServerInetSocketConnectionManager::bind(int port)
{
    struct sockaddr_in addr; // address structure
    int so_reuseaddr = true; // socket reuse flag
    WSADATA wsaData;

    // initialize Win sockets
    if (WSAStartup(MAKEWORD(2,0), &wsaData))
    {
        errcode = CSERROR_UNKNOWN;
    }
    // get a server socket
    c = socket(AF_INET, SOCK_STREAM, 0);
    if (c == -1)
    {
        errcode = CSERROR_UNKNOWN;
        return false;
    }
    // set the socket option to reuse the address
    setsockopt(c, SOL_SOCKET, SO_REUSEADDR, (char *)&so_reuseaddr,
        sizeof(so_reuseaddr));
    // bind the server socket to a port
    memset(&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    // This forces the socket to be bound
    // to the local interface only. Thus only the local machine will be allowed
    // to connect to this socket.
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (::bind(c, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        errcode = CSERROR_CONNX_EXISTS;
        return false;
    }
    // listen for a client at the port
    if (listen(c, 20) == -1)
    {
        errcode = CSERROR_INTERNAL;
        return false;
    }

    errcode = CSERROR_OK;
    return true;
}
