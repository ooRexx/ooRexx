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
/* Primitive Kernel Package class definitions                                 */
/*                                                                            */
/******************************************************************************/
#ifndef Included_PackageClass
#define Included_PackageClass

class RexxSource;

class PackageClass : public RexxObject
{
public:
    void *operator new(size_t);
    inline void *operator new(size_t size, void *ptr) { return ptr; };
    PackageClass(RexxSource *source);
    inline PackageClass(RESTORETYPE restoreType) { ; };

    void live(size_t);
    void liveGeneral(int reason);
    void flatten(RexxEnvelope*);

    static void createInstance();
    static RexxClass *classInstance;

    RexxString *getName();
    RexxArray *getSource();
    RexxString *getSourceLine(size_t);
    RexxInteger *getSourceSize();
    RexxString *getSourceLineRexx(RexxObject *);
    RexxObject  *setSecurityManager(RexxObject *);

    RexxDirectory *getClasses();
    RexxDirectory *getPublicClasses();
    RexxDirectory *getImportedClasses();
    RexxDirectory *getMethods();
    RexxDirectory *getRoutines();
    RexxDirectory *getPublicRoutines();
    RexxDirectory *getImportedRoutines();
    RexxArray     *getImportedPackages();
    PackageClass  *loadPackage(RexxString *name, RexxArray *source);
    RexxObject    *addPackage(PackageClass *package);
    RexxClass     *findClass(RexxString *name);
    RexxClass     *findClassRexx(RexxString *name);
    RoutineClass  *findRoutine(RexxString *name);
    RoutineClass  *findRoutineRexx(RexxString *name);
    RexxObject    *addRoutine(RexxString *name, RoutineClass *routine);
    RexxObject    *addPublicRoutine(RexxString *name, RoutineClass *routine);
    RexxObject    *addClass(RexxString *name, RexxClass *clazz);
    RexxObject    *addPublicClass(RexxString *name, RexxClass *clazz);
    RexxObject    *loadLibrary(RexxString *name);
    RexxObject    *digits();
    RexxObject    *fuzz();
    RexxObject    *form();
    RexxObject    *trace();

    PackageClass  *newRexx(RexxObject **init_args, size_t argCount);

    inline RexxSource *getSourceObject() { return source; }

protected:
    RexxSource *source;             // the wrappered source object


};


inline PackageClass *new_package(RexxSource *s)  { return new PackageClass(s); }
#endif


