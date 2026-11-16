/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2026 Rexx Language Association. All rights reserved.         */
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
/* REXX Kernel                                    BuiltinFunctionSupport.hpp  */
/*                                                                            */
/* The work of the BEEP, DIRECTORY and FILESPEC built-in functions, shared    */
/* with the routines of the REXX package that implemented them before 5.3.0.  */
/*                                                                            */
/******************************************************************************/
#ifndef BuiltinFunctionSupport_INCLUDED
#define BuiltinFunctionSupport_INCLUDED

#include "rexx.h"

class FileNameBuffer;

/**
 * BEEP, DIRECTORY and FILESPEC are built-in functions since 5.3.0.  Before,
 * they were routines of the REXX package, searched for as external routines,
 * and programs compiled by those releases still call them that way, so the
 * routines stay (runtime/InternalPackage.cpp), hidden from RxFuncQuery,
 * RxFuncAdd and RxFuncDrop (PackageManager::isRetiredRoutine()).  They can be
 * removed when MinimumLanguageLevel goes past 6.06, since then no program
 * that needs them can be run.
 *
 * So that there is one version of each function, the work is done here and
 * both callers only convert their arguments and results: the built-in
 * functions with the interpreter's objects, the routines with the native API
 * (which runs them without the interpreter lock, so nothing here creates an
 * object or raises an error).
 */
class BuiltinFunctionSupport
{
 public:
    // BEEP's limits
    static const wholenumber_t MinBeepFrequency = 37;
    static const wholenumber_t MaxBeepFrequency = 32767;
    static const wholenumber_t MinBeepDuration = 0;
    static const wholenumber_t MaxBeepDuration = 60000;

    // FILESPEC's options, for the error message
    static const char *const FilespecOptions;

    static void beep(wholenumber_t frequency, wholenumber_t duration);
    static bool directory(const char *newDirectory, FileNameBuffer &qualified, FileNameBuffer &current);
    static bool filespec(char option, const char *name, const char *&part, size_t &length);
};

#endif
