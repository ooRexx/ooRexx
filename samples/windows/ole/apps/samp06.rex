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
/* SAMP06.REX: OLE Automation with Object REXX - Sample 6             */
/*                                                                    */
/* Create a new document in WordPro 97 with a provided Smartmaster.   */
/* Fill in some "Click here" fields with data prompted by the program */
/* or queried from the system. Finally the document will be saved to  */
/* the same directory where this REXX program is located and sent to  */
/* the printer.                                                       */
/*                                                                    */
/* Since no check is done do ensure the new document does not already */
/* exist you will get a popup message from WordPro asking to          */
/* overwrite an already existing document when this sample is run     */
/* multiple times.                                                    */
/*                                                                    */
/**********************************************************************/

/* determine path of this sample program */
Parse Source . . ProgName
ProgPath = ProgName~Left(ProgName~LastPos("\"))

/* determine the version of REXX currently running */
Parse Version VersStr

/* prompt user for some information */
Say "Please enter your name:"
Parse Pull Name
If Name~Length = 0 Then
  Name = "No name entered!"

Say "Please enter your phone number:"
Parse Pull Phone
If Phone~Length = 0 Then
  Phone = "No phone entered!"

/* create a new document */
WordProApp = .OLEObject~New("WordPro.Application")
WordProApp~NewDocument("Samp06.lwp", ProgPath, ProgPath || "Samp06.mwp")

/* replace the click here blocks in the document with the new values */
WordProApp~Foundry~ClickHeres("YourName")~InsertText(Name)
WordProApp~Foundry~ClickHeres("YourPhone")~InsertText(Phone)
WordProApp~Foundry~ClickHeres("ProgramName")~InsertText(ProgName)
WordProApp~Foundry~ClickHeres("RexxVersion")~InsertText(VersStr)

WordProApp~Save
--WordProApp~PrintOut(1, 1, 1, .True)

WordProApp~Close(.False)
WordProApp~Quit


