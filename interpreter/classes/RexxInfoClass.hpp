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
/* REXX Kernel                                        RexxInfo.hpp            */
/*                                                                            */
/* Information about the interpreter implementation                           */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInfo
#define Included_RexxInfo

#include "ObjectClass.hpp"

class PackageClass;


/**
 * A class to information about the Rexx interpreter
 * context.
 */
class RexxInfo : public RexxObject
{
public:
    void *operator new(size_t);
    inline void  operator delete(void *) { ; }

    RexxInfo() { }
    inline RexxInfo(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    void initialize();

    PackageClass *getPackage();
    RexxObject *getDigits();
    RexxObject *getFuzz();
    RexxObject *getForm();
    RexxObject *getInternalDigits();
    RexxString *getLanguageLevel();
    RexxString *getInterpreterName();
    RexxString *getInterpreterVersion();
    RexxString *getInterpreterDate();
    RexxString *getPlatform();
    RexxObject *getArchitecture();
    RexxString *getFileEndOfLine();
    RexxString *getPathSeparator();
    RexxString *getDirectorySeparator();
    RexxObject *getCaseSensitiveFiles();
    RexxObject *getMajorVersion();
    RexxObject *getRelease();
    RexxObject *getModification();
    RexxObject *getRevision();
    RexxObject *getInternalMaxNumber();
    RexxObject *getInternalMinNumber();
    RexxObject *getMaxExponent();
    RexxObject *getMinExponent();
    RexxObject *getMaxPathLength();
    RexxObject *getMaxArraySize();
    RexxObject *getRexxExecutable();
    RexxObject *getRexxLibrary();
    RexxObject *getDebug();

    RexxObject *copyRexx();
    RexxObject *newRexx(RexxObject **args, size_t argc);

    static void createInstance();
    static RexxClass *classInstance;   // singleton class instance

 protected:

    RexxString *endOfLine;             // the end of line string
    RexxString *directorySeparator;    // the directory separator string
    RexxString *pathSeparator;         // the path separator string
    RexxString *interpreterName;       // full interpreter version string
    RexxString *interpreterVersion;    // the interpreter version level
    RexxString *interpreterDate;       // the interpreter build date
    RexxString *languageLevel;         // the language level string
    RexxString *platformName;          // the platform string
};

#endif

