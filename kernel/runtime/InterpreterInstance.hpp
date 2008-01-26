/*----------------------------------------------------------------------------*/
/*                                                                            */
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
/* Manage a created instance of the interpreter                               */
/*                                                                            */
/******************************************************************************/
#ifndef Included_InterpreterInstance_hpp
#define Included_Interpreterinstance_hpp

#include "RexxCore.h"
#include "ExitHandler.hpp"

class InterpreterInstance : public RexxInternalObject
{
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

    RexxString *getDefaultAddress() { return defaultAddress; }
    RexxActivity *getRootActivity() { return rootActivity; }

    InterpreterInstance(ExitHandler *handlers);
    void addActivity(RexxActivity *);
    void removeActivity(RexxActivity *);
    void initialize(RexxActivity *activity, PRXSYSEXIT handlers, const char *defaultEnvironment);
    bool terminate();
    void waitForCompletion();
    void attachToProcess();
    RexxActivity *enterOnCurrentThread();
    RexxActivity *attachThread();
    bool detachThread();
    RexxActivity *spawnActivity(RexxActivity *parent);
    void exitCurrentThread();
    RexxActivity *findActivity(thread_id_t threadId);
    RexxActivity *findActivity();
    RexxDirectory *getLocalEnvironment();
    void copyExits(ExitHandler *target);
    void activityDeactivated(RexxActivity *activity);
    void addGlobalReference(RexxObject *o);
    void removeGlobalReference(RexxObject *o);
    inline RexxString *getDefaultCommandEnvironment() { return getDefaultAddress(); }
    bool poolActivity(RexxActivity *activity);
    ExitHandler &getExitHandler(int exitNum) {  return exits[exitNum - 1]; }
    void setExitHandler(int exitNum, REXXPFN e) { getExitHandler(exitNum).setEntryPoint(e); }
    void setExitHandler(int exitNum, const char *e) { getExitHandler(exitNum).resolve(e); }
    void setExitHandler(RXSYSEXIT &e) { getExitHandler(e.sysexit_code).resolve(e.sysexit_name); }
    void removeInactiveActivities();
    void haltAllActivities();
    void traceAllActivities(bool on);

protected:

    RexxActivity        *rootActivity;       // the initial activity
    RexxList            *allActivities;      // all activities associated with this instance
    RexxList            *activeActivities;   // the activity table
    RexxList            *attachedActivities; // our list of attached vs. spawned activities
    RexxList            *spawnedActivities;  // activities this instance has spawned off
    RexxObjectTable     *globalReferences;   // our global reference table
    RexxString          *defaultAddress;     // the default address environment

    bool terminating;                // shutdown indicator
    bool terminated;                 // last thread cleared indicator
    SEV  terminationSem;             // used to signal that everything has shutdown

    // array of system exits
    ExitHandler exits[RXNOOFEXITS + 1];
};

#endif

