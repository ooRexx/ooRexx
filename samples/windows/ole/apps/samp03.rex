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
/**********************************************************************/
/*                                                                    */
/* SAMP03.REX: OLE Automation with Object REXX - Sample 3             */
/*                                                                    */
/* Show some features of the Windows Scripting Host Network object:   */
/*  - Query computer name, user name                                  */
/*  - List network connections for drives and printers                */
/*                                                                    */
/* This sample requires the Windows Scripting Host to be installed on */
/* the system. See http://www.microsoft.com/scripting for details.    */
/*                                                                    */
/**********************************************************************/

WshNetObj = .OLEObject~New("WScript.Network")

Say "Computer Name:" WshNetObj~ComputerName
Say "User Domain:" WshNetObj~UserDomain
Say "User Name:" WshNetObj~UserName

Say "The following network drives are currently mapped:"
MappedDrives = WshNetObj~EnumNetworkDrives
Do i=0 To MappedDrives~Count/2 - 1
  Say "   Drive" MappedDrives[i*2] "is mapped to" MappedDrives[i*2 + 1]
End

Say "The following network printers are currently connected:"
Printers = WshNetObj~EnumPrinterConnections
Do i=0 To Printers~Count/2 - 1
  Say "   Port" Printers[i*2] "is connected to" Printers[i*2 + 1]
End


