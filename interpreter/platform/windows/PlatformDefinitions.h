/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2021 Rexx Language Association. All rights reserved.    */
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
/* REXX Windows support                                                       */
/*                                                                            */
/* Windows master definition file                                             */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Template for system specific declarations for Object REXX.  The following  */
/* sections are a series of required and optional sections where system       */
/* specific information is provided to the rest of the interpreter code.      */
/* Wherever possible, reasonable defaults are provided for these settings.    */
/******************************************************************************/

#ifndef PlatformDefinitions_Included
#define PlatformDefinitions_Included

// enable warnings about the use of the override keyword.
#pragma warning(default:4263)
#pragma warning(default:4266)

#define SysCall _cdecl

/******************************************************************************/
/* REQUIRED:  Define the string used for the default initial address setting. */
/******************************************************************************/

#define SYSINITIALADDRESS "CMD"

/******************************************************************************/
/* REQUIRED:  Define the name of the image file that is saved and restored.   */
/******************************************************************************/

#define BASEIMAGE     "rexx.img"

/******************************************************************************/
/* REQUIRED:  Define the name of the program called to load create the saved  */
/* image file.                                                                */
/******************************************************************************/

#define BASEIMAGELOAD "CoreClasses.orx"

/******************************************************************************/
/* REQUIRED:  define the path delimiter and end of line information           */
/* appropriate for this file system.                                          */
/******************************************************************************/

#define delimiter '\\'
#define line_end "\r\n"
#define line_end_size 2

/******************************************************************************/
/* OPTIONAL:  Overrides for any functions defined in sysdef.h.  These         */
/* can map the calls directly to inline code or comment them out all together.*/
/******************************************************************************/

#define DEFRXSTRING 256                 /* Default RXSTRING return size      */

/******************************************************************************/
/* OPTIONAL:  Finally, any other global defined constants for system specific */
/* code usage.                                                                */
/******************************************************************************/

#endif

