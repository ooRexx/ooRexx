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

CRITICAL_SECTION ScriptProcessEngine::engineSection = { 0 };
HANDLE ScriptProcessEngine::engineMutex = 0; ;
DWORD  ScriptProcessEngine::engineThreadLocal = 0;
RexxInstance *ScriptProcessEngine::interpreter = NULL;
LooseLinkedList *ScriptProcessEngine::engineChain = NULL;
FILE *ScriptProcessEngine::logfile = NULL;
RexxClassObject ScriptProcessEngine::securityManager = NULLOBJECT;


void ScriptProcessEngine::initialize()
{
    engineChain = new LooseLinkedList;
    InitializeCriticalSection(&EngineSection);
#if SCRIPTDEBUG
    // TODO:  Don't open this in the hardcoded temp dir
    logfile = fopen("c:\\temp\\engine-dll.log","w");
    if (!logfile)
    {
        logfile = stderr;
    }
#endif

    // get a slot to use for anchoring engines on a thread local basis
    engineThreadLocal = TlsAlloc();

    engineMutex = CreateMutex(NULL,false,NULL); // we're in deep trouble if this is zero after the call!

    RexxOption options[2];

    RexxContextExit exits[] = {
        { RexxRetrieveVariables, RXTER},
        { RexxCatchExternalFunc, RXEXF},
        { RexxNovalueHandler, RXNOVAL},
        { RexxValueExtension, RXVALUE},
        { NULL, RXENDLST}
    };

    options[0].optionName = DIRECT_EXITS;
    options[0].option = (POINTER)&exits;

    RexxThreadContext *context;

    // create our interpreter instance
    RexxCreateInterpreter(&interpreter, &context, options);

    RexxPackageObject package = context->LoadPackage('orxscript.cls');
    securityManager = context->ResolveFindPackageClass(package, "ENGINESECURITY");

    FPRINTF2(logfile,"DllMain ATTACH - complete\n");
}


void ScriptProcessEngine::terminate()
{
    ListItem    *Current;
    OrxScript   *Content;
    FPRINTF2(logfile,"DllMain DETACH\n");
    //  If we are going away before all of the engines are gone,
    // then close them, so they will release their resources.
    ListItem *current = engineChain->FindItem(0);
    while (current != NULL)
    {
        OrxScript *content = (OrxScript *)current->GetContent();
        delete content;
        current = engineChain->FindItem();
    }
    delete engineChain;

    CloseHandle(engineMutex);
    // free the thread local for this
    TlsFree(engineThreadLocal);

    // remove critical section
    DeleteCriticalSection(&EngineSection);

    FPRINTF2(logfile,"DllMain DETACH - complete\n");
#if SCRIPTDEBUG
    if (logfile != stderr)
    {
        fclose(logfile);
    }
#endif
}


void ScriptProcessEngine::addScriptEngine(OrxScript *engine)
{
    engineChain->AddItem("ScriptEngine", LinkedList::End, (void *)engine);
    //  This will ensure this is removed from the chain when it is deleted.
    engine->SetDestructor(engineChain, (void *)pmyobject);
}



/**
 * Register a script engine in our thread local for retrieval on
 * callbacks.
 *
 * @param engine The engine to regisister.
 *
 * @return The previous engine value.  These may have nested calls on a
 *         thread, so this allows us to have a stack of these.
 */
OrxScript *ScriptProcessEngine::registerEngineForCallback(OrxScript *engine)
{
    OrxScript *previous = (OrxScript *)TlsGetValue(engineThreadLocal);
    TlsSetValue(engineThreadLocal, engine);
    return previous;
}


// remove engine pointer from thread list
void ScriptProcessEngine::deregisterEngineForCallback(OrxScript *previous)
    TlsSetValue(engineThreadLocal, previous);
}


/* this function looks for the engine that is associated with the */
/* given thread id. on failure it returns NULL.                   */
/* the list used to find this information is process-global.      */
OrxScript* findEngineForThread()
{
    return (OrxScript *)TlsGetValue(engineThreadLocal);
}
