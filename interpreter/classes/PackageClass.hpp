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
/*                                                                            */
/* Primitive Kernel Package class definitions                                 */
/*                                                                            */
/******************************************************************************/
#ifndef Included_PackageClass
#define Included_PackageClass

#include "SecurityManager.hpp"
#include "PackageSetting.hpp"
#include "SourceLocation.hpp"
#include "LanguageParser.hpp"

class ProgramSource;
class RexxCode;
class RoutineClass;


/**
 * An object that represents the source context for a
 * Method or Routine object.
 */
class PackageClass : public RexxObject
{
friend class LanguageParser;
public:
    void *operator new(size_t);

    PackageClass(RexxString *p, ProgramSource *s);
    inline PackageClass(RESTORETYPE restoreType) { ; };

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope*);

    static void createInstance();
    static RexxClass *classInstance;

    void          setup();
    void          extractNameInformation();
    bool          reconnect();
    size_t        sourceSize();
    RexxString   *traceBack(RexxActivation *, SourceLocation &, size_t, bool);
    RexxString   *extract(SourceLocation &);
    ArrayClass   *extractSource(SourceLocation &);
    ArrayClass   *extractSource();
    void          mergeRequired(PackageClass *);
    PackageClass *loadRequires(Activity *activity, RexxString *target);
    PackageClass *loadRequires(Activity *activity, RexxString *target, ArrayClass *s);
    void          addPackage(PackageClass *package);
    void          inheritPackageContext(PackageClass *parent);
    RoutineClass *findRoutine(RexxString *);
    RoutineClass *findLocalRoutine(RexxString *);
    RoutineClass *findPublicRoutine(RexxString *);
    RexxClass    *findClass(RexxString *);
    RexxClass    *findInstalledClass(RexxString *name);
    RexxClass    *findPublicClass(RexxString *name);
    RexxString   *resolveProgramName(Activity *activity, RexxString *name);
    void          processInstall(RexxActivation *);
    void          install();
    bool          isTraceable();
    RexxString   *getLine(size_t position);
    void          attachSource(BufferClass *s);
    void          checkRexxPackage();

    inline bool         needsInstallation() { return installRequired; }
    inline void         setNeedsInstallation() { installRequired = true; }
    inline void         install(RexxActivation *activation) { if (needsInstallation()) processInstall(activation); };
           void         setProgramName(RexxString *name);
           RexxString  *getProgramName() { return programName; }
    inline RexxString  *getProgramDirectory() { return programDirectory; }
    inline RexxString  *getProgramExtension() { return programExtension; }
    inline RexxString  *getProgramFile() { return programFile; }
    inline StringTable *getMethods() { return unattachedMethods; };
    inline StringTable *getRoutines() { return routines; };

    inline bool        isInternalCode() { return isOldSpace(); }

    void             setSecurityManager(RexxObject *manager) { setField(securityManager, new SecurityManager(manager)); }
    SecurityManager *getSecurityManager() { return securityManager; }

    inline StringTable *getLocalRoutines() { return routines; }
    inline StringTable *getPublicRoutines() { return publicRoutines; }
    inline void setLocalRoutines(StringTable *r) { routines = r; }
    inline void setPublicRoutines(StringTable *r) { publicRoutines = r; }

    void addInstalledClass(RexxString *name, RexxClass *classObject, bool publicClass);
    void addInstalledRoutine(RexxString *name, RoutineClass *routineObject, bool publicRoutine);

    inline StringTable *getInstalledClasses() { install(); return installedClasses; }
    inline StringTable *getInstalledPublicClasses() { install(); return installedPublicClasses; }
    inline StringTable *getImportedClasses() { install(); return mergedPublicClasses; }
    inline StringTable *getInstalledRoutines() { install(); return routines; }
    inline StringTable *getInstalledPublicRoutines() { install(); return publicRoutines; }
    inline StringTable *getImportedRoutines() { install(); return mergedPublicRoutines; }
    inline StringTable *getDefinedMethods() { install(); return unattachedMethods; }
    inline ArrayClass  *getPackages() { install(); return loadedPackages; }
    inline void         setDigits(wholenumber_t d) { packageSettings.setDigits(d); }
    inline wholenumber_t getDigits() { return packageSettings.getDigits(); }
    inline void         setForm(bool f) { packageSettings.setForm(f); }
    inline bool         getForm() { return packageSettings.getForm(); }
    inline void         setFuzz(wholenumber_t f) { packageSettings.setFuzz(f); }
    inline wholenumber_t getFuzz() { return packageSettings.getFuzz(); }
    inline void         setTraceSetting(const TraceSetting &s) { packageSettings.setTraceSetting(s); }
    inline const TraceSetting &getTraceSetting() { return packageSettings.getTraceSetting(); }
    inline const PackageSetting &getSettings() { return packageSettings; }
    inline bool  isRexxPackage() { return this == TheRexxPackage; }
    inline void  setLanguageLevel(LanguageLevel l) { requiredLanguageLevel = l; }
    inline LanguageLevel getLanguageLevel() { return requiredLanguageLevel; }

           RexxString    *getTrace();
           void           detachSource();
           PackageClass  *newRexx(RexxObject **init_args, size_t argCount);
           RexxString    *getSourceLineRexx(RexxObject *position);
           RexxInteger   *getSourceSizeRexx();
           StringTable   *getClassesRexx();
           StringTable   *getPublicClassesRexx();
           StringTable   *getImportedClassesRexx();
           StringTable   *getRoutinesRexx();
           StringTable   *getPublicRoutinesRexx();
           StringTable   *getImportedRoutinesRexx();
           StringTable   *getMethodsRexx();
           ArrayClass    *getImportedPackagesRexx();
           PackageClass  *loadPackageRexx(RexxString *name, ArrayClass *s);
           RexxObject    *addPackageRexx(PackageClass *package);
           RexxObject    *addRoutineRexx(RexxString *name, RoutineClass *routine);
           RexxObject    *addPublicRoutineRexx(RexxString *name, RoutineClass *routine);
           RexxObject    *addClassRexx(RexxString *name, RexxClass *clazz);
           RexxObject    *addPublicClassRexx(RexxString *name, RexxClass *clazz);
           RexxClass     *findClassRexx(RexxString *name);
           RoutineClass  *findRoutineRexx(RexxString *name);
           RexxObject    *setSecurityManagerRexx(RexxObject *manager);
           RexxObject    *loadLibraryRexx(RexxString *name);
           RexxObject    *digitsRexx();
           RexxObject    *fuzzRexx();
           RexxObject    *formRexx();
           RexxObject    *traceRexx();
           ArrayClass    *getSourceRexx()
           {
               return extractSource();
           }

protected:

    LanguageLevel requiredLanguageLevel; // the language version required to run this program
    ProgramSource *source;               // the reader for the program source...different flavors of this
    RexxString *programName;             // name of the source program        */
    RexxString *programDirectory;        // the directory location of the program (used for resolving calls)
    RexxString *programFile;             // just the file name of the program
    RexxString *programExtension;        // optional program extension
    SecurityManager *securityManager;    // source execution time security

    RexxCode *initCode;                  // the initialization code (can be null)
    BaseExecutable *mainExecutable;      // main execution unit for this package (a method or routine)
    PackageClass  *parentPackage;        // a parent source context environment;

    // sections derived from directives

    StringTable *routines;                // routines found on directives
    StringTable *publicRoutines;          // PUBLIC routines directive routines
    ArrayClass  *libraries;               // packages requiring loading
    ArrayClass  *requires;                // requires directives
    ArrayClass  *classes;                 // classes found on directives
    StringTable *dataAssets;              // assets defined in the package
    StringTable *unattachedMethods;       // methods found on directives

    // sections resolved from the install process.

    ArrayClass  *loadedPackages;          // packages imported by this package
                                          // all public installed classes
    StringTable *installedPublicClasses;
    StringTable *installedClasses;        // entire list of installed classes
    StringTable *mergedPublicClasses;     // entire merged set of classes
                                          // all public required routines
    StringTable *mergedPublicRoutines;

    bool           installRequired;       // indicates we need to install stuff later

    PackageSetting packageSettings;       // the settings we use at run time.

    // settings inherited from ::options statements
    intptr_t reserved[12];                // some reserved values for compatible expansion
};

#endif


