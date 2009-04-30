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
/* Primitive Rexx function/method package                                     */
/*                                                                            */
/******************************************************************************/
#ifndef PackageManager_Included
#define PackageManager_Included

#include "RexxCore.h"
#include "LibraryPackage.hpp"

class BaseCode;
class RoutineClass;
class ProtectedObject;
class RexxArray;
class RexxActivity;
class RexxNativeMethod;
class ProtectedObject;
class PackageClass;

class PackageManager
{
public:
    static void live(size_t liveMark);
    static void liveGeneral(int reason);

    static void initialize();
    static RexxArray *getImageData();
    static void restore(RexxArray *imageArray);
    static void restore();
    static LibraryPackage    *getLibrary(RexxString *name);
    static LibraryPackage    *loadLibrary(RexxString *name);
    static void        unload();
    static RexxNativeMethod  *resolveMethod(RexxString *packageName, RexxString *methodName);
    static RexxNativeMethod  *loadMethod(RexxString *packageName, RexxString *methodName);
    static RoutineClass *resolveRoutine(RexxString *function, RexxString *packageName, RexxString *procedure);
    static RoutineClass *resolveRoutine(RexxString *packageName, RexxString *function);
    static RoutineClass *loadRoutine(RexxString *packageName, RexxString *function);
    static RoutineClass *resolveRoutine(RexxString *function);
    static RoutineClass *createRegisteredRoutine(RexxString *function);
    static RoutineClass *getLoadedRoutine(RexxString *function);
    static PNATIVEMETHOD resolveMethodEntry(RexxString *package, RexxString *name);
    static PNATIVEROUTINE resolveRoutineEntry(RexxString *package, RexxString *name);
    static PREGISTEREDROUTINE resolveRegisteredRoutineEntry(RexxString *package, RexxString *name);
    static void        addPackageRoutine(RexxString *name, RoutineClass *func);
    static void        loadInternalPackage(RexxString *name, RexxPackageEntry *p);
    static bool        registerPackage(RexxString *name, RexxPackageEntry *p);
    static RexxObject *addRegisteredRoutine(RexxString *name, RexxString *module, RexxString *proc);
    static RexxObject *dropRegisteredRoutine(RexxString *name);
    static RexxObject *queryRegisteredRoutine(RexxString *name);
    static bool        callNativeRoutine(RexxActivity *activity, RexxString *name,
        RexxObject **arguments, size_t argcount, ProtectedObject &result);

    static RoutineClass *loadRequires(RexxActivity *activity, RexxString *shortName, RexxString *resolvedName, ProtectedObject &result);
    static RoutineClass *getMacroSpaceRequires(RexxActivity *activity, RexxString *name, ProtectedObject &result, RexxObject *securityManager);
    static RoutineClass *getRequiresFile(RexxActivity *activity, RexxString *name, RexxObject *securityManager, ProtectedObject &result);
    static RoutineClass *loadRequires(RexxActivity *activity, RexxString *name, const char *data, size_t length, ProtectedObject &result);
    static RoutineClass *loadRequires(RexxActivity *activity, RexxString *name, RexxArray *data, ProtectedObject &result);

protected:

    static RoutineClass *checkRequiresCache(RexxString *name, ProtectedObject &result);

    enum
    {
        IMAGE_PACKAGES = 1,
        IMAGE_PACKAGE_ROUTINES,
        IMAGE_REGISTERED_ROUTINES,
        IMAGE_REQUIRES,
        IMAGE_ARRAY_SIZE = IMAGE_REQUIRES
    };

    static RexxDirectory *imagePackages;        // our loaded packages
    static RexxDirectory *imagePackageRoutines; // table of functions loaded from packages
    static RexxDirectory *imageRegisteredRoutines;  // table of functions resolved by older registration mechanisms
    static RexxDirectory *imageLoadedRequires;      // table of previously loaded requires files


    static RexxDirectory *packages;        // our loaded packages
    static RexxDirectory *packageRoutines; // table of functions loaded from packages
    static RexxDirectory *registeredRoutines;  // table of functions resolved by older registration mechanisms
    static RexxDirectory *loadedRequires;      // table of previously loaded requires files

    static RexxPackageEntry *rexxPackage;   // internal generated REXX package
};

#endif

