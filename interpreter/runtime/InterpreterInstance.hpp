/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/******************************************************************************/
/* REXX Kernel                                                                */
/*                                                                            */
/* Manage a created instance of the interpreter                               */
/*                                                                            */
/******************************************************************************/
#ifndef Included_InterpreterInstance_hpp
#define Included_InterpreterInstance_hpp

#include "RexxCore.h"
#include "ExitHandler.hpp"
#include "ActivationApiContexts.hpp"
#include "SysInterpreterInstance.hpp"
#include "CommandHandler.hpp"

class DirectoryClass;
class CommandHandler;
class PackageClass;
class RoutineClass;

class InterpreterInstance : public RexxInternalObject
{
// the SysInterpreterInstance is essentially an extension of this class,
// so it is given full access to the interpreter instance fields.
friend class SysInterpreterInstance;
public:

    void *operator new(size_t);
    inline void  operator delete(void *) {;}

    // methods associated with actual interpreter instances
    inline InterpreterInstance(RESTORETYPE restoreType) { ; }
    InterpreterInstance();

    void live(size_t) override;
    void liveGeneral(MarkReason) override;

    RexxString *getDefaultEnvironment() { return defaultEnvironment; }
    Activity *getRootActivity() { return rootActivity; }

    InterpreterInstance(ExitHandler *handlers);
    void addActivity(Activity *);
    void removeActivity(Activity *);
    void initialize(Activity *activity, RexxOption *options);
    bool terminate();
    void waitForCompletion();
    void attachToProcess();
    Activity *enterOnCurrentThread();
    Activity *attachThread();
    int attachThread(RexxThreadContext *&attachedContext);
    bool detachThread();
    bool detachThread(Activity *activity);
    Activity *spawnActivity(Activity *parent);
    void exitCurrentThread();
    Activity *findActivity(thread_id_t threadId);
    Activity *findActivity();
    DirectoryClass *getLocalEnvironment();
    void copyExits(ExitHandler *target);
    void activityDeactivated(Activity *activity);
    void addGlobalReference(RexxObject *o);
    void removeGlobalReference(RexxObject *o);
    bool poolActivity(Activity *activity);
    ExitHandler &getExitHandler(int exitNum) {  return exits[exitNum - 1]; }
    void setExitHandler(int exitNum, REXXPFN e) { getExitHandler(exitNum).setEntryPoint(e); }
    void setExitHandler(int exitNum, const char *e) { getExitHandler(exitNum).resolve(e); }
    void setExitHandler(RXSYSEXIT &e) { getExitHandler(e.sysexit_code).resolve(e.sysexit_name); }
    void setExitHandler(RexxContextExit &e) { getExitHandler(e.sysexit_code).resolve(e.handler); }
    void removeInactiveActivities();
    bool haltAllActivities(RexxString *);
    void traceAllActivities(bool on);
    RexxString *resolveProgramName(RexxString *name, RexxString *dir, RexxString *ext);
    inline SecurityManager *getSecurityManager() { return securityManager; }
    void setSecurityManager(RexxObject *m);
    RexxInstance *getInstanceContext() { return &context.instanceContext; }
    RexxThreadContext *getRootThreadContext();
    RexxObject *getLocalEnvironment(RexxString *);
    inline DirectoryClass *getLocal() { return localEnvironment; }
    void addCommandHandler(const char *name, const char *registeredName);
    void addCommandHandler(const char *name, REXXPFN entryPoint, HandlerType::Enum type);
    CommandHandler *resolveCommandHandler(RexxString *name);
    PackageClass *getRequiresFile(Activity *activity, RexxString *name);
    PackageClass *loadRequires(Activity *activity, RexxString *shortName, ArrayClass *source);
    PackageClass *loadRequires(Activity *activity, RexxString *shortName, RexxString *fullName);
    PackageClass *loadRequires(Activity *activity, RexxString *shortName, const char *data, size_t length);
    void          addRequiresFile(RexxString *shortName, RexxString *fullName, PackageClass *package);
    inline void   setupProgram(RexxActivation *activation)
    {
        sysInstance.setupProgram(activation);
    }

protected:

    bool processOptions(RexxOption *options);


    InstanceContext        context;          // our externalizied instance context
    SysInterpreterInstance sysInstance;      // our platform specific helper

    Activity            *rootActivity;       // the initial activity
    SecurityManager     *securityManager;    // the security manager for our instance
    QueueClass          *allActivities;      // all activities associated with this instance
    RexxString          *defaultEnvironment; // the default address environment
    RexxString          *searchPath;         // additional Rexx search path
    ArrayClass          *searchExtensions;   // extensions to search on for external calls
    void                *applicationData;    // application specific data
    DirectoryClass      *localEnvironment;   // the current local environment
    StringTable         *commandHandlers;    // our list of command environment handlers
    StringTable         *requiresFiles;      // our list of requires files used by this instance

    bool terminating;                        // shutdown indicator
    bool terminated;                         // last thread cleared indicator
    SysSemaphore terminationSem;             // used to signal that everything has shutdown

    // array of system exits
    ExitHandler exits[RXNOOFEXITS + 1];

    static RexxInstanceInterface interfaceVector;   // single interface vector instance
};

#endif

