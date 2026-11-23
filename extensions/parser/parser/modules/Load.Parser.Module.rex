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
/* Load.Parser.Module.rex                                                     */
/* ======================                                                     */
/*                                                                            */
/* This program is part of the Rexx Parser package                            */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20241206    0.1  First public release                                      */
/* 20250318    0.2  Change syntax to class::method                            */
/* 20250326         Detect when a module tries to define an already-defined   */
/*                  method.                                                   */
/* 20250328         Main dir is now rexx-parser instead of rexx[.]parser      */
/* 20250714    0.2d Add 'override' parameter                                  */
/* 20251226    0.4a Send error messages to .error, not .output                */
/* 20261115         Use CATCH instead of SIGNAL ON (RFE #31; needs ooRexx     */
/*                  r13514 or later)                                          */
/* 20261123         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261123         Moved from bin/modules/ to bin/parser/modules/ (RFE #878: */
/*                  everything but parser.cls lives in parser/)               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* A common initializer for all Rexx parser modules                           */
/******************************************************************************/

--
-- Parameters
--
-- Dependencies: A list of package names (without the .cls extension),
--   separated by blanks
--
-- Override: true when existing methods can be redefined by this execution
--   of the loader.
--
Use Strict Arg dependencies = "", override = 0

-- We were called by a prolog routine, located in a certain package.
callerPackage = .context~stackFrames[2]~executable~package

-- Ensure that we run this initializer only once per package
If callerPackage~local~initializerRun == 1 Then Return
callerPackage~local~initializerRun = 1

-- Store public classes defined by the caller package in the global environment
Do With index name item class Over callerPackage~publicClasses
  .environment[name] = class
End

-- Now load the requested dependencies
Do package Over dependencies~makeArray(" ")
  callerPackage~loadPackage(package".cls")
End

-- Floating methods name format:
--
--   className"::"methodName
--
-- Such a method name will be ~defined into the className class with the
-- name methodName and the source found in the caller package.
--
methods = callerPackage~definedMethods
Do name Over methods
  If Pos("::", name) == 0 Then Iterate
  Parse Var name class"::"method
  If .environment[ class ] == .Nil Then Do
   .Error~Say( "Internal error! Class" class "not found." )
    Raise Halt
  End
  theClass = .environment[ class ]
  If \override, HasMethod( theClass, method ) Then Do
   .Error~Say( "Internal error! Method" method "already defined in class" class"." )
    Raise Halt
  End
  theClass ~ define( method, methods[ name ] )
End

Exit

--------------------------------------------------------------------------------
-- The definition of ~method is very strange: it traps when the method does   --
-- not exist...                                                               --
--------------------------------------------------------------------------------

HasMethod:
  Do
    throwAway = Arg(1)~method( Arg(2) )
  Catch Syntax
    Return 0
  End
  Return 1