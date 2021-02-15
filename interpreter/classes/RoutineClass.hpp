/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2019 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                             RoutineClass.hpp   */
/*                                                                            */
/* Primitive Kernel Method Class Definitions                                  */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RoutineClass
#define Included_RoutineClass

#include "BaseExecutable.hpp"
#include "LanguageLevel.hpp"
#include "ActivationSettings.hpp"

class RoutineClass : public BaseExecutable
{
public:
    void *operator new(size_t);

    RoutineClass(RexxString *n, BaseCode *_code);
    inline RoutineClass(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope*) override;

    void call(Activity *,  RexxString *,  RexxObject **, size_t, RexxString *, RexxString *, ActivationContext, ProtectedObject &);
    void call(Activity *,  RexxString *,  RexxObject **, size_t, ProtectedObject &);
    void runProgram(Activity *activity, RexxString * calltype, RexxString * environment, RexxObject **arguments, size_t argCount, ProtectedObject &result);
    void runProgram(Activity *activity, RexxObject **arguments, size_t argCount, ProtectedObject &result);

    RexxObject   *callRexx(RexxObject **, size_t);
    RexxObject   *callWithRexx(ArrayClass *);

    BufferClass *save();
    void save(PRXSTRING outBuffer);
    void save(const char *filename, bool encode);
    RexxObject  *setSecurityManager(RexxObject *);

    RoutineClass *newRexx(RexxObject **, size_t);
    RoutineClass *newFileRexx(RexxString *, PackageClass *);
    RoutineClass *loadExternalRoutine(RexxString *name, RexxString *descriptor);
    LanguageLevel getLanguageLevel();

    // various static classes for reading from a file and restoring potential
    // compiled images.
    static RoutineClass *restore(BufferClass *, char *, size_t length);
    static RoutineClass *restore(const char *data, size_t length);
    static RoutineClass *restore(RexxString *fileName, BufferClass *buffer);
    static RoutineClass *restoreFromMacroSpace(RexxString *name);
    static RoutineClass *restore(BufferClass *buffer);
    static RoutineClass *restore(RXSTRING *inData, RexxString *name);

    static void createInstance();
    static RexxClass *classInstance;
};
#endif

