/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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


#ifndef RexxStartDispatcher_included
#define RexxStartDispatcher_included

#include "ActivityDispatcher.hpp"
#include "ProtectedObject.hpp"

class RoutineClass;

/**
 * A dispatcher for processing a RexxStart call.
 */
class RexxStartDispatcher : public ActivityDispatcher
{
public:
    inline RexxStartDispatcher() : ActivityDispatcher() { ; }
    virtual ~RexxStartDispatcher() { ; }

    void run() override;
    void handleError(wholenumber_t, DirectoryClass *) override;

    size_t     argcount;                 // Number of args in arglist
    PCONSTRXSTRING arglist;              // Array of args
    const char *programName;             // REXX program to run
    PRXSTRING  instore;                  // Instore array
    const char *envname;                 // Initial cmd environment
    int        calltype;                 // How the program is called
    short      retcode;                  // Integer form of result
    PRXSTRING  result;                   // Result returned from program
};


/**
 * A dispatcher for handling external routine calls.
 */
class CallRoutineDispatcher : public ActivityDispatcher
{
public:
    inline CallRoutineDispatcher(RoutineClass *r, ArrayClass *a) : ActivityDispatcher(), routine(r), arguments(a) { ; }
    virtual ~CallRoutineDispatcher() { ; }

    void run() override;

    ProtectedObject result;

protected:
    RoutineClass *routine;           // target routine
    ArrayClass   *arguments;         // the argument array (can be NULL)
};


/**
 * A call dispatcher for handling an external program call.
 */
class CallProgramDispatcher : public ActivityDispatcher
{
public:
    inline CallProgramDispatcher(const char *p, ArrayClass *a) : ActivityDispatcher(), program(p), arguments(a) { ; }
    virtual ~CallProgramDispatcher() { ; }

    void run() override;

    ProtectedObject result;

protected:
    const char   *program;           // target routine
    ArrayClass   *arguments;         // the argument array (can be NULL)
};


#endif

