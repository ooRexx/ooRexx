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
#define RXAPI_REGUPDATE         WM_USER+1
#define RXAPI_REGREGISTER       WM_USER+2
#define RXAPI_REGQUERY          WM_USER+3
#define RXAPI_REGDEREGISTER     WM_USER+4
#define RXAPI_QUEUECREATE       WM_USER+5
#define RXAPI_QUEUEDELETE       WM_USER+6
#define RXAPI_QUEUEQUERY        WM_USER+7
#define RXAPI_QUEUEADD          WM_USER+8
#define RXAPI_QUEUEPULL         WM_USER+9
#define RXAPI_REGCHECKEXE       WM_USER+10
#define RXAPI_REGCHECKDLL       WM_USER+11
#define RXAPI_QUEUECOMEXTEND    WM_USER+12
#define RXAPI_SHUTDOWN          WM_USER+13
#define RXAPI_QUEUESESSION      WM_USER+14
#define RXAPI_QUEUESESSIONDEL   WM_USER+15
#define RXAPI_MACROCOMEXTEND    WM_USER+16
#define RXAPI_MACRO             WM_USER+17
#define RXAPI_TERMINATE         WM_USER+18
#define RXAPI_PROCESSCLEANUP    WM_USER+19
#define RXAPI_PROCESSGONE       WM_USER+20 // ENG feature 1046
