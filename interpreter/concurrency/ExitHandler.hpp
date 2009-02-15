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
/******************************************************************************/
/* REXX Kernel                                                                */
/*                                                                            */
/* Manage a created instance of the interpreter                               */
/*                                                                            */
/******************************************************************************/
#ifndef Included_ExitHandler_hpp
#define Included_ExitHandler_hpp

#include "RexxCore.h"
#include "CallbackDispatcher.hpp"

class RexxActivity;

class ExitHandler
{
public:
    inline ExitHandler() : entryPoint(NULL) { type = UNRESOLVED; }
    void setEntryPoint(REXXPFN e) { entryPoint = e; }
    inline bool isEnabled()
    {
        return entryPoint != NULL;
    }

    inline void disable()
    {
        entryPoint = NULL;
    }

    int call(RexxActivity *activity, RexxActivation *activation, int major, int minor, void *parms);
    inline ExitHandler & operator= (ExitHandler &o)
    {
        entryPoint = o.entryPoint;
        type = o.type;
        return *this;
    }

    void resolve(const char *name);
    void resolve(RexxContextExitHandler *handler);

protected:

    typedef enum {
        UNRESOLVED,
        REGISTERED_NAME,
        DIRECT
    } ExitType;


    REXXPFN    entryPoint;             // resolved exit entry point
    ExitType   type;                   // the type of call
};


class ExitHandlerDispatcher : public CallbackDispatcher
{
public:
    inline ExitHandlerDispatcher(REXXPFN e, int code, int subcode, void *a) { entryPoint = e; major = code; minor = subcode; parms = a; }
    virtual ~ExitHandlerDispatcher() { ; }

    virtual void run();

    int        rc;                        // handler return code
    int        major;                     // major exit code
    int        minor;                     // minor exit code
    REXXPFN    entryPoint;                // resolved exit entry point
    void      *parms;                     // opaque arguments passed to callback handler
};


class ContextExitHandlerDispatcher : public ExitHandlerDispatcher
{
public:
    inline ContextExitHandlerDispatcher(REXXPFN e, int code, int subcode, void *a) : ExitHandlerDispatcher(e, code, subcode, a) { }
    virtual ~ContextExitHandlerDispatcher() { ; }

    virtual void run();
};

#endif

