/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/*                                                                            */
/* Class to encapuslate the various settings that can be inherited from a     */
/* package instance to standardize how these settings are managed.            */
/*                                                                            */
/******************************************************************************/
#ifndef Included_PackageSetting
#define Included_PackageSetting

#include "TraceSetting.hpp"
#include "Numerics.hpp"
#include "FlagSet.hpp"

/**
 * Flag definitions for various package options that
 * are used to initialize RexxActivation.  Note, for
 * saved image compatibility, new flags need to be added
 * to the end of this enum.
 */
typedef enum
{
    NovalueSyntax,
    NoProlog,
    ErrorSyntax,
    FailureSyntax,
    LostdigitsSyntax,
    NostringSyntax,
    NotreadySyntax,
} PackageFlags;



/**
 * A class for processing different numeric settings
 */
class PackageSetting
{
 public:

    PackageSetting() { }

    // initialize the default values
    inline void setDefault()
    {
        numericSettings.setDefault();
        traceSettings.setDefault();
    }

    inline void   setDigits(size_t d) { numericSettings.setDigits(d); }
    inline size_t getDigits() const { return numericSettings.getDigits(); }
    inline void   setForm(bool f)  { numericSettings.setForm(f); }
    inline bool   getForm() const { return numericSettings.getForm(); }
    inline void   setFuzz(size_t f) { numericSettings.setFuzz(f); }
    inline size_t getFuzz() const { return numericSettings.getFuzz(); }
    inline void   setTraceSetting(const TraceSetting &s) { traceSettings = s; }
    inline bool   isDebug() { return traceSettings.isDebug(); }
    inline const  TraceSetting &getTraceSetting() const { return traceSettings; }
    inline RexxString *getTrace() { return traceSettings.toString(); }
    inline bool   isErrorSyntaxEnabled() { return packageOptions[ErrorSyntax]; }
    inline void   enableErrorSyntax() { packageOptions[ErrorSyntax] = true; }
    inline void   disableErrorSyntax() { packageOptions[ErrorSyntax] = false; }
    inline bool   isFailureSyntaxEnabled() { return packageOptions[FailureSyntax]; }
    inline void   enableFailureSyntax() { packageOptions[FailureSyntax] = true; }
    inline void   disableFailureSyntax() { packageOptions[FailureSyntax] = false; }
    inline bool   isLostdigitsSyntaxEnabled() { return packageOptions[LostdigitsSyntax]; }
    inline void   enableLostdigitsSyntax() { packageOptions[LostdigitsSyntax] = true; }
    inline void   disableLostdigitsSyntax() { packageOptions[LostdigitsSyntax] = false; }
    inline bool   isNostringSyntaxEnabled() { return packageOptions[NostringSyntax]; }
    inline void   enableNostringSyntax() { packageOptions[NostringSyntax] = true; }
    inline void   disableNostringSyntax() { packageOptions[NostringSyntax] = false; }
    inline bool   isNotreadySyntaxEnabled() { return packageOptions[NotreadySyntax]; }
    inline void   enableNotreadySyntax() { packageOptions[NotreadySyntax] = true; }
    inline void   disableNotreadySyntax() { packageOptions[NotreadySyntax] = false; }
    inline bool   isNovalueSyntaxEnabled() { return packageOptions[NovalueSyntax]; }
    inline void   enableNovalueSyntax() { packageOptions[NovalueSyntax] = true; }
    inline void   disableNovalueSyntax() { packageOptions[NovalueSyntax] = false; }
    inline void   enableProlog() { packageOptions[NoProlog] = false; }
    inline void   disableProlog() { packageOptions[NoProlog] = true; }
    inline bool   isPrologEnabled() { return !packageOptions[NoProlog]; }

    NumericSettings numericSettings;       // the package numeric settings
    TraceSetting    traceSettings;         // the package trace setting
    FlagSet<PackageFlags> packageOptions;  // additional enabled options
};

#endif

