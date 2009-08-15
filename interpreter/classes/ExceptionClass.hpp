/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
/*                                                                            */
/* Primitive Kernel Exception class definitions                               */
/*                                                                            */
/******************************************************************************/
#ifndef Included_ExceptionClass
#define Included_ExceptionClass

class RexxList;
class RexxDirectory;

class ExceptionClass : public RexxObject
{
public:
    void *operator new(size_t);
    inline void *operator new(size_t size, void *ptr) { return ptr; };
    ExceptionClass();
    inline ExceptionClass(RESTORETYPE restoreType) { ; };

    void live(size_t);
    void liveGeneral(int reason);
    void flatten(RexxEnvelope*);

    static void createInstance();
    static RexxClass *classInstance;

    RexxObject *init(RexxString *type, RexxString *message, RexxString *description, RexxObject *additional, ExceptionClass *exception);

    RexxString *getType();
    RexxString *getMessage();
    RexxString *getDescription();
    RexxObject *getAdditional();
    ExceptionClass *getCause();
    RexxList   *getStackFrames();
    RexxList   *getTraceBack();
    RexxDirectory *getCondition();
    RexxObject *fillInStackTrace();

    RexxObject *newRexx(RexxObject **args, size_t argc);

protected:
    RexxString *type;               // the type of frame
    RexxString *message;            // the message associated with the exception
    ExceptionClass *cause;          // An optional cause object.
    RexxList  *stackFrames;         // arguments to the method/routine
    RexxList  *traceBack;           // traceback lines
    RexxString *description;        // the exception description
    RexxObject *additional;         // the additional information
    RexxDirectory *condition;       // a created condition object
};

#endif


