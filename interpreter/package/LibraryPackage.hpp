/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/* Primitive Rexx function/method package                                     */
/*                                                                            */
/******************************************************************************/
#ifndef LibraryPackage_Included
#define LibraryPackage_Included

#include "RexxCore.h"
#include "SysLibrary.hpp"
#include "NativeCode.hpp"
#include "CallbackDispatcher.hpp"

class PackageManager;
class NativeMethod;
class StringTable;

typedef RexxPackageEntry * (RexxEntry *PACKAGE_LOADER)();

/**
 * An internal descriptor for loaded library collections.
 */
class LibraryPackage : public RexxInternalObject
{
public:
           void *operator new(size_t);
    inline void  operator delete(void *) {;}

    LibraryPackage(RexxString *n);
    LibraryPackage(RexxString *n, RexxPackageEntry *p);
    inline LibraryPackage(RESTORETYPE restoreType) { ; };

    void live(size_t liveMark) override;
    void liveGeneral(MarkReason reason) override;

    bool load();
    void unload();
    RexxPackageEntry *getPackageTable();
    void loadPackage();
    void loadRoutines(RexxRoutineEntry *table);
    RexxMethodEntry *locateMethodEntry(RexxString *name);
    RexxRoutineEntry *locateRoutineEntry(RexxString *name);
    NativeMethod *resolveMethod(RexxString *name);
    RoutineClass *resolveRoutine(RexxString *name);
    PNATIVEMETHOD resolveMethodEntry(RexxString *name);
    PNATIVEROUTINE resolveRoutineEntry(RexxString *name);
    PREGISTEREDROUTINE resolveRegisteredRoutineEntry(RexxString *name);
    void   reload();
    void   reload(RexxPackageEntry *pack);
    inline bool isLoaded() { return loaded; }
    inline bool isInternal() { return internal; }
    inline void makeInternal() { internal = true; }
    inline StringTable *getRoutines() { return publicRoutines; }
    inline bool isPackage(RexxString *n) { return n->strCaselessCompare(libraryName); }

protected:

    RexxPackageEntry *package;  // loaded package information
    RexxString *libraryName;    // the name of the library
    StringTable    *routines;   // loaded routines
    StringTable    *publicRoutines;   // loaded routines table
    StringTable    *methods;    // loaded methods
    SysLibrary  lib;            // the library management handle
    bool        loaded;         // we've at least been able to load the library
    bool        internal;       // this is an internal package...no library load required.
};


/**
 * A wrapper for dispatching Library load calls.
 */
class LibraryLoaderDispatcher : public CallbackDispatcher
{
public:
    inline LibraryLoaderDispatcher(RexxPackageLoader l) : loader(l) { }
    virtual ~LibraryLoaderDispatcher() { ; }

    void run() override;

protected:
    RexxPackageLoader loader;
};


/**
 * A wrapper for displatching library unload calls.
 */
class LibraryUnloaderDispatcher : public CallbackDispatcher
{
public:
    inline LibraryUnloaderDispatcher(RexxPackageUnloader u) : unloader(u) { }
    virtual ~LibraryUnloaderDispatcher() { ; }

    void run() override;

protected:
    RexxPackageUnloader unloader;
};

#endif

