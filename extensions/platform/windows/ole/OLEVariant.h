/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2006-2014 Rexx Language Association. All rights reserved.    */
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
/**
 *  ooRexx OLE Automation Support                                   OLEVariant.h
 *
 *  Constant defines, function prototypes, and globals for OLEVariant.
 */

#define DEFAULT_PARAMFLAGS     "1"
#define PARAMFLAG_ILLEGAL      -1
#define FLAG_SEPARATOR_CHAR    ','

// Function prototypes for local functions
static void convertToParamFlag(RexxMethodContext *, RexxObjectPtr, int );
static void convertToVT(RexxMethodContext *, RexxObjectPtr, int );
static RexxObjectPtr stringToVT(RexxMethodContext *,RexxObjectPtr);
static RexxObjectPtr stringToFlags(RexxMethodContext *, RexxObjectPtr);

static __inline int  countSymbols( PSZ, CHAR );
static __inline PSZ  stripNonCSyms( PSZ );
static __inline VARENUM  findVT( PSZ );
static __inline BOOL areValidVTs( VARENUM, VARENUM );
static __inline int  findFlag( PSZ );

// Some functions borrowed from orexxole.c
extern PSZ pszStringDupe( const char * );
extern VOID ORexxOleFree( PVOID );

// Global data

/**
 *  All the VT types that are valid to use as a VARIANTARG (arguments passed in
 *  DISPPARAMS.)
 *
 *  There are other VT types defined in VARENUM, but they are not valid to use
 *  through IDispatch::Invoke.  In addition there are a few other restrictions,
 *  such as VT_EMPTY and VT_NULL can not be used with VT_BYREF or VT_ARRAY;
 *  VT_VARIANT can not be used alone.
 */
static char * vtStrTable[] = {
  "VT_EMPTY",
  "VT_NULL",
  "VT_I2",
  "VT_I4",
  "VT_R4",
  "VT_R8",
  "VT_CY",
  "VT_DATE",
  "VT_BSTR",
  "VT_DISPATCH",
  "VT_ERROR",
  "VT_BOOL",
  "VT_VARIANT",
  "VT_UNKNOWN",
  "VT_DECIMAL",
  "VT_I1",
  "VT_UI1",
  "VT_UI2",
  "VT_UI4",
  "VT_I8",
  "VT_UI8",
  "VT_INT",
  "VT_UINT",
  "VT_ARRAY",
  "VT_BYREF",
  NULL
};

/**
 *  A table for the valid VT types.  This must match vtStrTable.
 */
static VARENUM vtIntTable[] = {
  VT_EMPTY,
  VT_NULL,
  VT_I2,
  VT_I4,
  VT_R4,
  VT_R8,
  VT_CY,
  VT_DATE,
  VT_BSTR,
  VT_DISPATCH,
  VT_ERROR,
  VT_BOOL,
  VT_VARIANT,
  VT_UNKNOWN,
  VT_DECIMAL,
  VT_I1,
  VT_UI1,
  VT_UI2,
  VT_UI4,
  VT_I8,
  VT_UI8,
  VT_INT,
  VT_UINT,
  VT_ARRAY,
  VT_BYREF
};

/**
 * The string names for the valid PARAMFLAGS used in a PARAMDESC structure.
 */
static char * flagStrTable[] = {
  "NONE",
  "IN",
  "OUT",
  "LCID",
  "RETVAL",
  "OPT",
  "HASDEFAULT",
  "HASCUSTDATA",
  NULL
};

/**
 * The actual PARAMFLAGS to match the flagStrTable.
 */
static int flagIntTable[] = {
  PARAMFLAG_NONE,
  PARAMFLAG_FIN,
  PARAMFLAG_FOUT,
  PARAMFLAG_FLCID,
  PARAMFLAG_FRETVAL,
  PARAMFLAG_FOPT,
  PARAMFLAG_FHASDEFAULT,
  PARAMFLAG_FHASCUSTDATA
};

