/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* Manage a command handler callback                                          */
/*                                                                            */
/******************************************************************************/
#ifndef Included_CommandHandler_hpp
#define Included_CommandHandler_hpp

#include "RexxCore.h"
#include "CallbackDispatcher.hpp"

class RexxActivity;

class CommandHandler : public RexxInternalObject
{
public:
    inline void *operator new(size_t size, void *ptr) { return ptr; }
    void        *operator new(size_t size);
    inline void  operator delete(void *) { ; }
    inline void  operator delete(void *, void *) { ; }

    inline CommandHandler(RESTORETYPE restoreType) { ; };
    inline CommandHandler(REXXPFN e) : entryPoint(e) { type = DIRECT; }
    inline CommandHandler(const char *n) : entryPoint(NULL) { type = UNRESOLVED; resolve(n); }

    void call(RexxActivity *activity, RexxActivation *activation, RexxString *address, RexxString *command, ProtectedObject &rc, ProtectedObject &condition);
    void resolve(const char *name);
    inline bool isResolved() { return type != UNRESOLVED; }

protected:

    typedef enum
    {
        UNRESOLVED,
        REGISTERED_NAME,
        DIRECT
    } HandlerType;

    REXXPFN    entryPoint;             // resolved exit entry point
    HandlerType   type;                // the type of call
};


class CommandHandlerDispatcher : public CallbackDispatcher
{
public:
    CommandHandlerDispatcher(RexxActivity * a, REXXPFN e, RexxString *c);
    virtual ~CommandHandlerDispatcher() { ; }

    virtual void run();
    void complete(RexxString *command, ProtectedObject &result, ProtectedObject &condition);

    RexxActivity *activity;               // the activity we're dispatching on
    REXXPFN    entryPoint;                // resolved exit entry point
    CONSTRXSTRING  rxstrcmd;              // invoked command
    RXSTRING       retstr;                // passed back result
    wholenumber_t  sbrc;                  // direct numeric return code
    char default_return_buffer[DEFRXSTRING];
    unsigned short flags;                 // command status flags
};


class ContextCommandHandlerDispatcher : public CallbackDispatcher
{
public:
    inline ContextCommandHandlerDispatcher(REXXPFN e, RexxString *a, RexxString *c, ProtectedObject &r, ProtectedObject &co) :
        entryPoint(e), address(a), command(c), result(r), condition(co) { }
    virtual ~ContextCommandHandlerDispatcher() { ; }

    virtual void run();
    virtual void handleError(RexxDirectory *);

    REXXPFN   entryPoint;                 // the installed command handler
    RexxString *address;                  // the address environment
    RexxString *command;                  // the command to invoke
    ProtectedObject &result;              // the handled result
    ProtectedObject &condition;           // any raised condition
};

#endif

