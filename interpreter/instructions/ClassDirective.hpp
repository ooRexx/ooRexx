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
/* REXX Kernel                                          ClassDirective.hpp    */
/*                                                                            */
/* A class definition stored in a package to manage class creation.           */
/*                                                                            */
/******************************************************************************/
#ifndef Included_ClassDirective
#define Included_ClassDirective

#include "RexxDirective.hpp"

class DirectoryClass;
class RexxClass;

/**
 * A class representing a directive instruction inside
 * of a package file.
 */
class ClassDirective : public RexxDirective
{
 friend class PackageClass;
 public:
           void *operator new(size_t);
    inline void  operator delete(void *) { }

    ClassDirective(RexxString *, RexxString *, RexxClause *);
    inline ClassDirective(RESTORETYPE restoreType) { ; };

    virtual void live(size_t);
    virtual void liveGeneral(MarkReason reason);
    virtual void flatten(Envelope *);

    inline RexxString *getName() { return publicName; }
    RexxClass *install(PackageClass *package, RexxActivation *activation);

    void addDependencies(StringTable *class_directives);
    void checkDependency(RexxString *name, StringTable *class_directives);
    bool dependenciesResolved();
    void removeDependency(RexxString *name);

    inline RexxString *getMetaClass() { return metaclassName; }
    inline void setMetaClass(RexxString *m) { setField(metaclassName, m); }
    inline RexxString *getSubClass() { return subclassName; }
    inline void setSubClass(RexxString *m) { setField(subclassName, m); }
    inline void setMixinClass(RexxString *m) { setField(subclassName, m); mixinClass = true; }
    inline void setPublic() { publicClass = true; }
    void addInherits(RexxString *name);
    void addMethod(RexxString *name, MethodClass *method, bool classMethod);
    void addConstantMethod(RexxString *name, MethodClass *method);
    bool checkDuplicateMethod(RexxString *name, bool classMethod);


protected:

    StringTable *getClassMethods();
    StringTable *getInstanceMethods();

    RexxString  *publicName;         // the published name of the class
    RexxString  *idName;             // the internal ID name
    RexxString  *metaclassName;      // name of the class meta class
    RexxString  *subclassName;       // the class used for the subclassing operation.
    ArrayClass  *inheritsClasses;    // the names of inherited classes
    StringTable *instanceMethods;    // the methods attached to this class
    StringTable *classMethods;       // the set of class methods
    bool         publicClass;        // this is a public class
    bool         mixinClass;         // this is a mixin class
    StringTable  *dependencies;      // in-package dependencies
};

#endif

