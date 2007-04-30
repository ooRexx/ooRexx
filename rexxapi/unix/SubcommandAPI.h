/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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


#ifndef AIXSEAPI_HC_INCLUDED
#define AIXSEAPI_HC_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif
#ifdef IBMC
#pragma linkage(RexxCallExit, system)
#endif
LONG APIENTRY RexxCallExit(PSZ,
                           PSZ,
                           LONG,
                           LONG,
                           PEXIT ) ;


/***    RexxLoadSubcom - Load a Subcommand environment */

#ifdef IBMC
#pragma linkage(RexxLoadSubcom, system)
#endif
APIRET APIENTRY RexxLoadSubcom(
         PSZ,                          /* Name of the Environment    */
         PSZ );                        /* DLL Module Name            */

/***   Uppercase Entry Point Name */
#define REXXLOADSUBCOM  RexxLoadSubcom

/***    RexxLoadSubcom - Load a Subcommand environment */

#ifdef IBMC
#pragma linkage(RexxLoadSubcom, system)
#endif
APIRET APIENTRY RexxLoadSubcom(
         PSZ,                          /* Name of the Environment    */
         PSZ );                        /* DLL Module Name            */

/***   Uppercase Entry Point Name */
#define REXXLOADSUBCOM  RexxLoadSubcom

/***    RexxCallSubcom - Execute a command in an environment */

#ifdef IBMC
#pragma linkage(RexxCallSubcom, system)
#endif
APIRET APIENTRY RexxCallSubcom(
         PSZ,                          /* Name of Subcommand Environ */
         PSZ,                          /* Module name of its DLL     */
         PRXSTRING,                    /* Command string to be passed*/
         PUSHORT,                      /* Stor for error flag notice */
         PUSHORT,                      /* Stor for rc from handler   */
         PRXSTRING );                  /* Stor for returned string   */

/***   Uppercase Entry Point Name */
#define REXXCALLSUBCOM  RexxCallSubcom


/***    RexxCallFunction - Call a function in the AFT */

#ifdef IBMC
#pragma linkage(RexxCallFunction, system)
#endif
APIRET APIENTRY RexxCallFunction (
        PSZ,                           /* Name of function to call   */
        ULONG ,                        /* Number of arguments        */
        PRXSTRING,                     /* Array of argument strings  */
        PUSHORT,                        /* RC from function called    */
        PRXSTRING,                     /* Storage for returned data  */
        PSZ );                         /* Name of active data queue  */

/***   Uppercase Entry Point Name */
#define REXXCALLFUNCTION  RexxCallFunction
#endif

#ifdef __cplusplus
}
#endif
