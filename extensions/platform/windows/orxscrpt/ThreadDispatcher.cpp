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

#include "orxscript.hpp"
#include "ThreadDispatcher.hpp"


void __stdcall threadDispatch(void *arguments)
{
    // run the dispatched function and terminate
    ((ThreadDispatcher *)arguments)->run();
    _endthreadex(0);
}


ThreadDispatcher::ThreadDispatcher(OrxScript *e, int start)
{
    engine = e;
    relativeStart = start;
    memset(cd, 0, sizeof(RexxConditionData));
}


/**
 * Default virtual method for handling a run() methods on
 * an activity dispatcher.
 */
void ThreadDispatcher::run()
{
    RexxThreadContext *context = ScriptProcessEngine::getThreadContext();
    this->run(context);
    context->DetachThread();
}


/**
 * Default virtual method for handling a run() methods on
 * an activity dispatcher.
 */
void ThreadDispatcher::run(RexxThreadContext *context)
{
    // this just returns
}


/**
 * Invoke the dispatcher on a newly created interpreter instance.
 */
int ThreadDispatcher::invoke()
{
        // now create the method (runs in a different thread)
    HANDLE execution = (HANDLE)_beginthreadex(NULL, 0, (unsigned int (__stdcall *) (void*) ) threadDispatch, (LPVOID) this, 0, &dummy);
    // could not start thread?
    if (execution == 0)
    {
        return -1;
    }
    else
    {
        WaitForSingleObject(execution, INFINITE);
        CloseHandle(execution);
    }
    cd.position += startingLineNumber;
    //    The following code HAS to be after the _endthreadex(), or
    //  bad things will happen.  None of the COM calls that result
    //  from the following function calls will work.
    if (cd.rc != 0)
    {
        cd.position += relativeStart;
        // broadcast the error
        engine->rexxError(&cd);
        // an error occured: init excep info
        OrxScriptError *ErrObj = new OrxScriptError(engine->logfile, &cd, &ErrObj_Exists);
    }
    return cd.rc;
}



/**
 * Default virtual method for handling a run() methods on
 * an activity dispatcher.
 */
void ParseProcedureTextDispatcher::run(RexxThreadContext *context)
{
    engine->convertTextToCode(context, code, relativeStart, routine, &cd);
    if (cd.rc == 0)
    {
        PRCB codeBlock;
        engine->processScriptFragment(context, relativeStart, routine, codeBlock, &cd);
        if (cd.rc == 0)
        {
            engine->queueOrExecuteFragment(context, codeBlock, &cd);
        }
    }
}
