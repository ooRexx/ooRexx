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
/* Package definition for exported native methods in the "REXX" package       */
/*                                                                            */
/******************************************************************************/

#include "RexxCore.h"
#include "PackageManager.hpp"


#define INTERNAL_METHOD(name) REXX_METHOD_PROTOTYPE(name)

#include "NativeMethods.h"             // bring in the standard list,
#include "SysNativeMethods.h"          // plus any system extensions

#undef  INTERNAL_METHOD
#define INTERNAL_METHOD(name) REXX_METHOD(name, name),

// now build the actual entry list
RexxMethodEntry rexx_methods[] =
{
#include "NativeMethods.h"             // bring in the standard list,
#include "SysNativeMethods.h"          // plus any system extensions
    REXX_LAST_METHOD()
};


#define INTERNAL_ROUTINE(name, entry) REXX_TYPED_ROUTINE_PROTOTYPE(entry)

#include "NativeFunctions.h"             // bring in the standard list,
#include "SysNativeFunctions.h"          // plus any system extensions

#undef  INTERNAL_ROUTINE
#define INTERNAL_ROUTINE(name, entry) REXX_TYPED_ROUTINE(name, entry),

// now build the actual entry list
RexxRoutineEntry rexx_routines[] =
{
#include "NativeFunctions.h"             // bring in the standard list,
#include "SysNativeFunctions.h"          // plus any system extensions
    REXX_LAST_ROUTINE()
};

RexxPackageEntry rexx_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_INTERPRETER_4_0_0,              // anything after 4.0.0 will work
    "REXX",                              // name of the package
    "4.0",                               // package information
    NULL,                                // no load/unload functions
    NULL,
    rexx_routines,                       // the exported routines
    rexx_methods                         // the exported methods
};

// and finally plug this in to the package manager.
RexxPackageEntry *PackageManager::rexxPackage = &rexx_package_entry;
