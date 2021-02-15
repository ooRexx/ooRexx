/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2017 Rexx Language Association. All rights reserved.    */
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
/* Base class definition for Method and Routine executables.                  */
/*                                                                            */
/******************************************************************************/
#ifndef Included_BaseExecutable
#define Included_BaseExecutable

#include "BaseCode.hpp"
#include "ProtectedObject.hpp"

class StringTable;

/**
 * Base class for all executable objects.  Executable
 * objects and Methods and Routines.
 */
class BaseExecutable : public RexxObject
{
public:
    RexxInternalObject *copy() override;

    inline PackageClass *getPackageObject() { return code->getPackageObject(); };
    inline BaseCode   *getCode() { return code; }
    ArrayClass  *getSource() { return code->getSource(); }
    PackageClass *getPackage();

    ArrayClass *source();
    RexxClass *findClass(RexxString *className);
    BaseExecutable *setPackageObject(PackageClass *s);
    RexxString *getName() { return executableName; }
    ProgramSource *detachSource();
    void attachSource(ProgramSource *s);

    StringTable *getAnnotations();
    RexxString  *getAnnotation(RexxString *name);
    RexxObject  *getAnnotationRexx(RexxObject *name);

    static ArrayClass *processExecutableSource(RexxObject *source, const char *position);
    static void processNewExecutableArgs(RexxObject **&init_args, size_t &argCount, RexxString *&name,
        Protected<ArrayClass> &sourceArray, PackageClass *&sourceContext);
    static void processNewFileExecutableArgs(RexxString *&filename, PackageClass *&sourceContext);

protected:

    RexxString *executableName;         // the created name of this routine
    BaseCode   *code;                   // the backing code object
    StringTable *annotations;           // attached annotations
};


#endif
