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
#ifndef REXXAPITYPES_INCLUDED
#define REXXAPITYPES_INCLUDED

#include "windows.h"
#include "limits.h"

typedef SSIZE_T   ssize_t;
typedef INT_PTR   intptr_t;
typedef UINT_PTR  uintptr_t;
#ifdef  _WIN64
#define __REXX64__
#else
#undef __REXX64__
#endif

// portable ANSI types
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef char int8_t;
typedef unsigned char uint8_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;

typedef DWORD thread_id_t;
typedef DWORD process_id_t;

#define REXXENTRY APIENTRY

#ifdef __cplusplus
extern "C" {
#endif
typedef size_t (REXXENTRY *REXXPFN)();
#ifdef __cplusplus
}
#endif

#ifndef SIZE_MAX
#define SIZE_MAX		(~((size_t)0))
#endif
#define SSIZE_MAX		((ssize_t)(SIZE_MAX >> 1))
#define SSIZE_MIN		(~SSIZE_MAX - 1)

#define UINTPTR_MAX     (~((uintptr_t)0))
#define INTPTR_MAX      ((intptr_t)(UINTPTR_MAX >> 1))
#define INTPTR_MIN      (~INTPTR_MAX)

#define UINT32_MAX      (~((uint32_t)0))
#define INT32_MAX       ((int32_t)(UINT32_MAX >> 1))
#define INT32_MIN       (~INT32_MAX)

#define UINT16_MAX      (uint16_t)(~((uint16_t)0))
#define INT16_MAX       ((int16_t)(UINT16_MAX >> 1))
#define INT16_MIN       (~INT16_MAX)

#define UINT8_MAX       (uint8_t)(~((uint8_t)0))
#define INT8_MAX        ((int8_t)(UINT8_MAX >> 1))
#define INT8_MIN        (~INT8_MAX)

#define UINT64_MAX      (~((uint64_t)0))
#define INT64_MAX       ((int64_t)(UINT64_MAX >> 1))
#define INT64_MIN       (~INT64_MAX)

#define VLARexxEntry __cdecl           /* external entry points       */

#endif /* REXXAPITYPES_INCLUDED */

