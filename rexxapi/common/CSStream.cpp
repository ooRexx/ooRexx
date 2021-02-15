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
#include "ServiceException.hpp"
#include "CSStream.hpp"


/**
 * Get a buffer for sending a buffered message.
 *
 * @param size   The required size.
 *
 * @return A pointer to a buffer, or NULL if unable to allocate.
 */
char *ApiConnection::getMessageBuffer(size_t size)
{
    // if larger than our cached buffer, return
    if (size > MAX_CACHED_BUFFER)
    {
        char *buffer = (char *)malloc(size);
        if (buffer == NULL)
        {
            throw new ServiceException(MEMORY_ERROR, "Error allocating message buffer");
        }
        return buffer;
    }
    // use our cached buffer, allocating it if required.
    if (messageBuffer == NULL)
    {
        messageBuffer = (char *)malloc(MAX_CACHED_BUFFER);
        if (messageBuffer == NULL)
        {
            throw new ServiceException(MEMORY_ERROR, "Error allocating message buffer");
        }
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
void ApiConnection::returnMessageBuffer(void *buffer)
{
    if (buffer != messageBuffer)
    {
        free(buffer);
    }
}
