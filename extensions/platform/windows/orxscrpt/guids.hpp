/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
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
#ifndef REXX_GUIDS
#define REXX_GUIDS

// also contains some registry information!

// Us, in Windows Speak.  "Wie der Softwaregeier kennen uns"
DEFINE_GUID(CLSID_ObjectREXX, 0x13dad011, 0xb0c9, 0x11d4, 0xa8, 0x29, 0x00, 0x06, 0x29, 0x86, 0x97, 0x85);
#define szCLSID_ObjectREXX "{13dad011-b0c9-11d4-a829-000629869785}"

#define szLANGNAME            "Object Rexx"
#define szALTERNATELANGNAME   "ObjectRexxScript"
#define szDESCRIPTION         "Object Rexx Script Language"
#define szEXTENSION           ".RXS"
#define szLANGFILE            "ObjectRexxScriptFile"
#define szFILEDESCRIPTION     "Object Rexx Script File"
#define szDLLNAME             "ORXSCRPT.DLL"

#if 0
DEFINE_GUID(CLSID_ObjectREXX, 0x17df3540, 0xb0c9, 0x11d4, 0xa8, 0x29, 0x00, 0x06, 0x29, 0x86, 0x97, 0x85);
#define szCLSID_ObjectREXX "{17df3540-b0c9-11d4-a829-000629869785}"

DEFINE_GUID(CLSID_ObjectREXX, 0x13dad010, 0xb0c9, 0x11d4, 0xa8, 0x29, 0x00, 0x06, 0x29, 0x86, 0x97, 0x85);
#define szCLSID_ObjectREXX "{13dad010-b0c9-11d4-a829-000629869785}"

#define szLANGNAME            "T-Rexx"
#define szALTERNATELANGNAME   "TestObjectRexxScript"
#define szDESCRIPTION         "Object Rexx Test Script Language"
#define szEXTENSION           ".trx"
#define szLANGFILE            "TestObjectRexxScriptFile"
#define szFILEDESCRIPTION     "Object Rexx Test Script File"
#define szDLLNAME             "tORXSCRPT.DLL"

#endif

#endif
