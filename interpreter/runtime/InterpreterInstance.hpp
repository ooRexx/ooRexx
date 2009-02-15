/*----------------------------------------------------------------------------*/
/*                                                                            */
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
#ifndef Included_InterpreterInstance_hpp
#define Included_Interpreterinstance_hpp

#include "RexxCore.h"
#include "ExitHandler.hpp"
#include "ActivationApiContexts.hpp"
#include "SysInterpreterInstance.hpp"

class RexxDirectory;
class CommandHandler;

class InterpreterInstance : public RexxInternalObject
{
// the SysInterpreterInstance is essentially an extension of this class,
// so it is given full access to the interpreter instance fields.
friend class SysInterpreterInstance;
public:

    // methods associated with actual interpreter instances
    inline InterpreterInstance(RESTORETYPE restoreType) { ; }
    InterpreterInstance();

    inline void *operator new(size_t, void *ptr) {return ptr;}
    inline void  operator delete(void *, void *) {;}
    void *operator new(size_t);
    inline void  operator delete(void *) {;}
    void        live(size_t);
    void        liveGeneral(int);

    RexxString *getDefaultEnvironment() { return defaultEnvironment; }
    RexxActivity *getRootActivity() { return rootActivity; }

    InterpreterInstance(ExitHandler *handlers);
    void addActivity(RexxActivity *);
    void removeActivity(RexxActivity *);
    void initialize(RexxActivity *activity, RexxOption *options);
    bool terminate();
    void waitForCompletion();
    void attachToProcess();
    RexxActivity *enterOnCurrentThread();
    RexxActivity *attachThread();
    int attachThread(RexxThreadContext *&attachedContext);
    bool detachThread();
    bool detachThread(RexxActivity *activity);
    RexxActivity *spawnActivity(RexxActivity *parent);
    void exitCurrentThread();
    RexxActivity *findActivity(thread_id_t threadId);
    RexxActivity *findActivity();
    RexxDirectory *getLocalEnvironment();
    void copyExits(ExitHandler *target);
    void activityDeactivated(RexxActivity *activity);
    void addGlobalReference(RexxObject *o);
    void removeGlobalReference(RexxObject *o);
    bool poolActivity(RexxActivity *activity);
    ExitHandler &getExitHandler(int exitNum) {  return exits[exitNum - 1]; }
    void setExitHandler(int exitNum, REXXPFN e) { getExitHandler(exitNum).setEntryPoint(e); }
    void setExitHandler(int exitNum, const char *e) { getExitHandler(exitNum).resolve(e); }
    void setExitHandler(RXSYSEXIT &e) { getExitHandler(e.sysexit_code).resolve(e.sysexit_name); }
    void setExitHandler(RexxContextExit &e) { getExitHandler(e.sysexit_code).resolve(e.handler); }
    void removeInactiveActivities();
    void haltAllActivities();
    void traceAllActivities(bool on);
    inline RexxString *resolveProgramName(RexxString *name, RexxString *dir, RexxString *ext) { return sysInstance.resolveProgramName(name, dir, ext); }
    inline SecurityManager *getSecurityManager() { return securityManager; }
    void setSecurityManager(RexxObject *m);
    RexxInstance *getInstanceContext() { return &context.instanceContext; }
    RexxThreadContext *getRootThreadContext();
    RexxObject *getLocalEnvironment(RexxString *);
    inline RexxDirectory *getLocal() { return localEnvironment; }
    void addCommandHandler(const char *name, const char *registeredName);
    void addCommandHandler(const char *name, REXXPFN entryPoint);
    CommandHandler *resolveCommandHandler(RexxString *name);

protected:

    bool processOptions(RexxOption *options);


    InstanceContext      context;            // our externalizied instance context
    SysInterpreterInstance sysInstance;      // our platform specific helper

    RexxActivity        *rootActivity;       // the initial activity
    SecurityManager     *securityManager;    // the security manager for our instance
    RexxList            *allActivities;      // all activities associated with this instance
    RexxIdentityTable   *globalReferences;   // our global reference table
    RexxString          *defaultEnvironment; // the default address environment
    RexxString          *searchPath;         // additional Rexx search path
    RexxList            *searchExtensions;   // extensions to search on for external calls
    void                *applicationData;    // application specific data
    RexxDirectory       *localEnvironment;   // the current local environment
    RexxDirectory       *commandHandlers;    // our list of command environment handlers

    bool terminating;                // shutdown indicator
    bool terminated;                 // last thread cleared indicator
    SysSemaphore terminationSem;     // used to signal that everything has shutdown

    // array of system exits
    ExitHandler exits[RXNOOFEXITS + 1];

    static RexxInstanceInterface interfaceVector;   // single interface vector instance
};

#endif

