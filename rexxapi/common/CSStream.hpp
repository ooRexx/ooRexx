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

#ifndef CSStream_Included
#define CSStream_Included

// Client Server error codes
typedef enum
{
    CSERROR_OK = 0,
    CSERROR_CONNX_EXISTS,
    CSERROR_CONNX_FAILED,
    CSERROR_IO_FAILED,
    CSERROR_OPEN_FAILED,
    CSERROR_HOSTNAME_PORT,
    CSERROR_INTERNAL,
    CSERROR_ALREADY_CONNECTED,
    CSERROR_UNKNOWN
} CSErrorCodeT;


/**
 * Base class for a client server connection. This manages the interface,
 * while platform-specific overrides manage the communication details.
 */
class ApiConnection
{
public:
    inline ApiConnection() : errcode(CSERROR_OK), messageBuffer(NULL) { }
    virtual ~ApiConnection() { disconnect(); if (messageBuffer != NULL) { free(messageBuffer); } }

    inline CSErrorCodeT getError()
    {
        return errcode;
    };
    inline bool isClean()
    {
        return errcode == CSERROR_OK;
    }

    char *getMessageBuffer(size_t size);
    void returnMessageBuffer(void *);

    // methods that concrete implementations must override
    virtual bool read(void *buf, size_t bufsize, size_t *bytesread) = 0;
    virtual bool write(void *buf, size_t bufsize, size_t *byteswritten) = 0;
    virtual bool write(void *buf, size_t bufsize, void*buf2, size_t buf2size, size_t *byteswritten) = 0;
    virtual bool disconnect() { return false; };

protected:
    enum
    {
        // somewhat arbitrary.  Should be large enough for "normal requests"
        MAX_CACHED_BUFFER = 4096
    };

    CSErrorCodeT errcode;  // error status
    char *messageBuffer;   // a buffer for message sending
};


/**
 * Base class for a server connection manager that accepts
 * incoming connections from clients. This manages the
 * interface, while platform-specific overrides manage the
 * communication details.
 */
class ServerConnectionManager
{
public:
    inline ServerConnectionManager() : errcode(CSERROR_OK) { }
    virtual ~ServerConnectionManager() { disconnect(); }

    inline CSErrorCodeT getError()
    {
        return errcode;
    };
    inline bool isClean()
    {
        return errcode == CSERROR_OK;
    }

    virtual bool disconnect() { return false; };
    virtual ApiConnection *acceptConnection() = 0;

protected:
    CSErrorCodeT errcode;  // error status
};

#endif

