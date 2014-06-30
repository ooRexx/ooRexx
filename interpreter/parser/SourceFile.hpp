/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                                SourceFile.hpp  */
/*                                                                            */
/* Translater Source Class Definitions                                        */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxSource
#define Included_RexxSource

#include "SourceLocation.hpp"
#include "ListClass.hpp"
#include "QueueClass.hpp"
#include "StackClass.hpp"
#include "Token.hpp"
#include "Clause.hpp"
#include "SecurityManager.hpp"
#include "ProgramSource.hpp"

class RexxInstruction;
class RexxInstructionDo;
class RexxInstructionIf;
class RexxInstructionForward;
class RexxExpressionMessage;
class RexxCompoundVariable;
class RoutineClass;
class RexxCode;
class PackageClass;
class ClassDirective;
class RexxActivation;
class RexxExpressionStack;
class StackFrameClass;

/******************************************************************************/
/* Constants used for setting trace                                           */
/******************************************************************************/

const size_t TRACE_ALL           = 'A';
const size_t TRACE_COMMANDS      = 'C';
const size_t TRACE_LABELS        = 'L';
const size_t TRACE_NORMAL        = 'N';
const size_t TRACE_FAILURES      = 'F';
const size_t TRACE_ERRORS        = 'E';
const size_t TRACE_RESULTS       = 'R';
const size_t TRACE_INTERMEDIATES = 'I';
const size_t TRACE_OFF           = 'O';
const size_t TRACE_IGNORE        = '0';

// a mask for accessing just the setting information
const size_t TRACE_SETTING_MASK  = 0xff;

/******************************************************************************/
/* Constants used for setting trace interactive debug.  These get merged      */
/* in with the setting value, so they must be > 256                           */
/******************************************************************************/
const int DEBUG_IGNORE      =  0x0000;
const int DEBUG_ON          =  0x0100;
const int DEBUG_OFF         =  0x0200;
const int DEBUG_TOGGLE      =  0x0400;
const int DEBUG_NOTRACE     =  0x0800;

// the mask for accessing just the debug flags
const size_t TRACE_DEBUG_MASK  = 0xff00;

/**
 * An enum for the flag positions in the flags bitset.
 * We currently only have a single flag defined, but
 * using a bitset allows us to define additional attributes
 * if required.
 */
enum
{
    installRequired;                // this requirements that require an install step
} SourceFlag;


class RexxSource: public RexxInternalObject
{
    // grant the language parser full access to our
    // protected data.
    friend class LanguageParser;
 public:
    static RexxSource *createSource(RexxString *filename, ProgramSource *s);
    static RexxSource *createSource(RexxString *filename);
    static RexxSource *createSource(RexxString *filename, RexxBuffer *b);
    static RexxSource *createSource(RexxString *filename, RexxArray *a);

    // NOTE:  for GC purposes, only simple initialization is done at this point.
    // We do the real work in the setup() method.
    RexxSource(RexxString *p, ProgramSource *s) : programName(p), source(s) { }
    inline RexxSource(RESTORETYPE restoreType) { ; };

    virtual void live(size_t);
    virtual void liveGeneral(int reason);
    virtual void flatten(RexxEnvelope *);

    void        setup();
    void        extractNameInformation();
    bool        reconnect();
    void        setReconnect();
    void        setBufferedSource(RexxBuffer *newSource) { this->initBuffered(newSource); }
    void        interpretLine(size_t);
    size_t      sourceSize();
    RexxString *get(size_t);
    RexxString *traceBack(RexxActivation *, SourceLocation &, size_t, bool);
    RexxString *extract(SourceLocation &);
    RexxArray  *extractSource(SourceLocation &);
    RexxArray  *extractSource();
    void        globalSetup();
    RexxCode   *interpretMethod(RexxDirectory *);
    RexxCode   *interpret(RexxString *, RexxDirectory *, size_t);
    void        mergeRequired(RexxSource *);
    PackageClass *loadRequires(RexxActivity *activity, RexxString *target);
    PackageClass *loadRequires(RexxActivity *activity, RexxString *target, RexxArray *s);
    void        addPackage(PackageClass *package);
    PackageClass *getPackage();
    void        inheritSourceContext(RexxSource *source);
    RoutineClass *findRoutine(RexxString *);
    RoutineClass *findLocalRoutine(RexxString *);
    RoutineClass *findPublicRoutine(RexxString *);
    RexxClass  *findClass(RexxString *);
    RexxClass  *findInstalledClass(RexxString *name);
    RexxClass  *findPublicClass(RexxString *name);
    RexxString *resolveProgramName(RexxActivity *activity, RexxString *name);
    void        processInstall(RexxActivation *);
    void        install();
    RexxCode   *translate(RexxDirectory *);
    void        setGuard();
    RexxArray  *getGuard();
    RexxVariableBase *getRetriever(RexxString *);
    RexxInstruction *sourceNewObject(size_t, RexxBehaviour *, int);

    bool        isTraceable();
    inline bool isInterpret() { return (flags & _interpret) != 0; }

    inline bool        needsInstallation() { return (this->flags&_install) != 0; }
    inline void        install(RexxActivation *activation) { if (needsInstallation()) this->processInstall(activation); };
    inline void        addReference(RexxObject *reference) { this->calls->addLast(reference); }
           void        setProgramName(RexxString *name);
    inline RexxString *getProgramName() { return this->programName; }
    inline RexxString *getProgramDirectory() { return this->programDirectory; }
    inline RexxString *getProgramExtension() { return this->programExtension; }
    inline RexxString *getProgramFile() { return this->programFile; }
    inline RexxDirectory *getMethods() { return this->methods; };
    inline RexxDirectory *getRoutines() { return this->routines; };

    inline bool        isInternalCode() { return this->isOldSpace(); }
    StackFrameClass *createStackFrame();

    void        setSecurityManager(RexxObject *manager) { OrefSet(this, this->securityManager, new SecurityManager(manager)); }
    SecurityManager *getSecurityManager() { return securityManager; }

    inline RexxDirectory *getLocalRoutines() { return routines; }
    inline RexxDirectory *getPublicRoutines() { return publicRoutines; }
    inline void setLocalRoutines(RexxDirectory *r) { routines = r; }
    inline void setPublicRoutines(RexxDirectory *r) { publicRoutines = r; }

    void addInstalledClass(RexxString *name, RexxClass *classObject, bool publicClass);
    void addInstalledRoutine(RexxString *name, RoutineClass *routineObject, bool publicRoutine);

    inline RexxDirectory *getInstalledClasses() { install(); return installedClasses; }
    inline RexxDirectory *getInstalledPublicClasses() { install(); return installedPublicClasses; }
    inline RexxDirectory *getImportedClasses() { install(); return mergedPublicClasses; }
    inline RexxDirectory *getInstalledRoutines() { install(); return routines; }
    inline RexxDirectory *getInstalledPublicRoutines() { install(); return publicRoutines; }
    inline RexxDirectory *getImportedRoutines() { install(); return mergedPublicRoutines; }
    inline RexxDirectory *getDefinedMethods() { install(); return methods; }
    inline RexxList      *getPackages() { install(); return loadedPackages; }
    inline void           setDigits(size_t d) { digits = d; }
    inline size_t         getDigits() { return digits; }
    inline void           setForm(bool f) { form = f; }
    inline bool           getForm() { return form; }
    inline void           setFuzz(size_t f) { fuzz = f; }
    inline size_t         getFuzz() { return fuzz; }
    inline void           setTraceSetting(size_t t) { traceSetting = t; }
    inline size_t         getTraceSetting() { return traceSetting; }
    inline void           setTraceFlags(size_t t) { traceFlags = t; }
    inline size_t         getTraceFlags() { return traceFlags; }
    inline RexxString    *getTrace() { return formatTraceSetting(traceSetting); }
    inline void           setInitCode(RexxCode *c) { initCode = c: }

protected:

    size_t requiredLanguageVersion;      // the language version required to run this program
    ProgramSource *source;               // the reader for the program source...different flavors of this
    RexxString *programName;             // name of the source program        */
    RexxString *programDirectory;        // the directory location of the program (used for resolving calls)
    RexxString *programFile;             // just the file name of the program
    RexxString *programExtension;        // optional program extension
    SecurityManager *securityManager;    // source execution time security

    RexxCode *initCode;                  // the initialization code

    RexxList      *loadedPackages;       // packages imported by this package
    PackageClass  *package;              // our package wrapper
    RexxSource    *parentSource;         // a parent source context environment;
    RexxDirectory *routines;             // routines found on directives
    RexxDirectory *publicRoutines;       // PUBLIC routines directive routines
    RexxArray     *libraries;            // packages requiring loading
    RexxArray     *requires;             // requires directives
    RexxArray     *classes;              // classes found on directives
    RexxDirectory *dataAssets;           // assets defined in the package

                                         // all public installed classes
    RexxDirectory *installedPublicClasses;
    RexxDirectory *installedClasses;    // entire list of installed classes
    RexxDirectory *mergedPublicClasses;  // entire merged set of classes
                                         // all public required routines
    RexxDirectory *mergedPublicRoutines;
    RexxDirectory *methods;              // methods found on directives

    bitset<32>     flags;                // flag settings.  Make it big enough for some expansion.

    // settings inherited from ::options statements
    size_t digits;                       // numeric digits setting
    size_t fuzz;                         // numeric fuzz setting
    bool form;                           // numeric form setting
    size_t traceSetting;                 // the package trace setting
    size_t traceFlags;                   // version optimized for quick setting at startup
    intptr_t reserved[12];               // some reserved values for compatible expansion
};
#endif
