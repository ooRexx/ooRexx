/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2024-2026 Josep Maria Blasco. All rights reserved.           */
/* Copyright (c) 2026 Rexx Language Association. All rights reserved.         */
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
/* CLISupport.cls -- Common CLI boilerplate for Rexx Parser utilities         */
/* ==================================================================         */
/*                                                                            */
/* This file is part of the Rexx Parser package                               */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20260314    0.5   Initial version                                          */
/* 20261115    0.7   InitCLI: the caller is the first frame that is not an    */
/*                   internal call or an INTERPRET, so that a utility can     */
/*                   call it from an internal routine ("Call Main" in a DO    */
/*                   with CATCH clauses, RFE #31)                             */
/* 20261121         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* InitCLI() -- Common utility initialization                                 */
/*                                                                            */
/* Performs the standard initialization sequence shared by all Rexx Parser    */
/* command-line utilities: extracts the program name, builds the help URL,    */
/* and resolves command-line arguments.                                       */
/*                                                                            */
/* No arguments are needed.  The routine inspects the call stack to find      */
/* the caller's program name and arguments:                                   */
/*                                                                            */
/*   .context~stackFrames[1]  is InitCLI itself                               */
/*   .context~stackFrames[2]  is the caller. If it is an internal routine     */
/*     (or an INTERPRET), the frames above it are skipped, up to the first    */
/*     one that is not: the program (type PROGRAM), providing:                */
/*     ~name       the full path of the .rex file                             */
/*     ~arguments  the Arg(1) passed to the prolog                            */
/*                                                                            */
/* .SysCArgs (in .Local) is used when available (COMMAND invocation);         */
/* otherwise the prolog's Arg(1) is parsed via ArgArray().                    */
/*                                                                            */
/* Returns: an Array with three items:                                        */
/*   callerName  - Program name without .rex extension (e.g. "highlight")     */
/*   callerHelp  - URL pointing to the utility's documentation                */
/*   args    - An Array of command-line arguments                             */
/*                                                                            */
/* Usage:                                                                     */
/*                                                                            */
/*   CLIhelper = InitCLI()                                                    */
/*   myName = CLIhelper~name                                                  */
/*   myHelp = CLIhelper~help                                                  */
/*   myArgs = CLIhelper~args                                                  */
/*                                                                            */
/******************************************************************************/

::Routine InitCLI Public

  frames    = .context~stackFrames
  stackSize =  frames~size

  -- Skip the internal routines (and INTERPRETs) of the caller: we want the
  -- program, which has the name of the file and the arguments
  n = 2
  Loop While n < stackSize, -
    WordPos( frames[n]~type, "INTERNALCALL INTERPRET" ) > 0
    n += 1
  End
  caller = frames[n]

  callerName = caller~name
  Parse Caseless Value FileSpec("Name", callerName) With callerName".rex"
  callerHelp = ChangeStr(                                          -
   "callerName",                                                   -
   "https://rexx.epbcn.com/rexx-parser/doc/utilities/callerName/", -
    callerName)

  If n == stackSize, .SysCArgs \== .Nil Then callerArgs = .SysCArgs
  Else Do
    callerArgs = caller~arguments
    If callerArgs~items > 0
      Then callerArgs =  ArgArray(callerArgs[1])
      Else callerArgs = .Array~new
  End

  CLIHelper      = .Directory~new
  CLIHelper~name =  callerName
  CLIHelper~help =  callerHelp
  CLIHelper~args =  callerArgs

  Return CLIHelper

::Requires "BaseClassesAndRoutines.cls"