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

#ifndef ServiceException_HPP_INCLUDED
#define ServiceException_HPP_INCLUDED

#include "string.h"

typedef enum
{
    NO_ERROR_CODE,
    MEMORY_ERROR,
    SERVER_FAILURE,
    API_FAILURE,

    MACRO_SOURCE_NOT_FOUND,
    MACRO_SOURCE_READ_ERROR,
    MACRO_LOAD_REXX,
    MACRO_TRANSLATION_ERROR,
    MACROSPACE_FILE_READ_ERROR,
    FILE_CREATION_ERROR,
    MACROSPACE_VERSION_ERROR,
    MACROSPACE_SIGNATURE_ERROR,
    FILE_READ_ERROR,
    FILE_WRITE_ERROR,

    INVALID_QUEUE_NAME,
    BAD_FIFO_LIFO,
    BAD_WAIT_FLAG,
} ErrorCode;

class ServiceException
{
public:
    ServiceException(ErrorCode c, const char *m)
    {
        errCode = c;
        //NOTE:  this does not get released, so it MUST be a literal string
        message = m;
    }

    ~ServiceException() { }

    inline ErrorCode getErrorCode() { return errCode; }

    inline const char *getMessage() { return message; }
protected:
    ErrorCode errCode;
    const char  *message;
};

#endif
