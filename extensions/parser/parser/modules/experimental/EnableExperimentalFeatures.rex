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
/* EnableExperimentalFeatures.rex                                             */
/* ==============================                                             */
/*                                                                            */
/* This file is part of the Rexx Parser package                               */
/* [See https://rexx.epbcn.com/rexx-parser/]                                  */
/*                                                                            */
/* Version history:                                                           */
/*                                                                            */
/* Date     Version Details                                                   */
/* -------- ------- --------------------------------------------------------- */
/* 20251113    0.3a First version                                             */
/* 20261115         Use CATCH instead of SIGNAL ON (RFE #31; needs ooRexx     */
/*                  r13514 or later)                                          */
/* 20261123         Common Public License 1.0, and copyright also of the Rexx */
/*                  Language Association, as the rest of ooRexx (RFE #878)    */
/* 20261123         Moved from bin/ to bin/parser/modules/experimental/ (RFE  */
/*                  #878)                                                     */
/*                                                                            */
/******************************************************************************/

-- We have to pass the caller's .METHODS stringtable as a parameter
-- because of https://sourceforge.net/p/oorexx/bugs/2037/

Use Strict Arg methods

/******************************************************************************/
/* Implement METHOD EXTENDS and METHOD OVERRIDES                              */
/******************************************************************************/

pkgLocal = .context~package~local
pkgLocal~classExtensions = .Set~new

callerPackage = .context~stackFrames[2]~executable~package

-- Ensure that we process extension methods only once per caller
If .ClassExtensions~hasIndex( callerPackage ) Then Return
.ClassExtensions[] = callerPackage

-- Extension methods have the following syntax:
--
--   ::METHOD methodName ... EXTENDS   class ...
--   ::METHOD methodName ... OVERRIDES class ...
--
-- where class is
--
--   [nameSpace:]className
--
-- The compiler for Experimental features removes the EXTENDS or
-- OVERRIDES phrase, and replaces the method name by
--
--   '"'methodName"<"sep">"[nameSpace:]className'"',
--
-- where sep is "+" for EXTENDS and "*" for OVERRIDES.
--

extends   = "<+>"
overrides = "<*>"

-- We first check all floating methods defined by the caller.

Do name Over methods
  Select
    When Pos(extends,   name) > 0 Then Do
      Parse Var name methodName(extends)className
      Call ProcessNameSpaceAndFindClass
      If HasMethod( class, methodName ) Then
        Raise Syntax 99.900 Array("Method" methodName "already defined in class" className)
    End
    When Pos(overrides, name) > 0 Then Do
      Parse Var name methodName(overrides)className
      Call ProcessNameSpaceAndFindClass
    End
    Otherwise Iterate
  End
  -- Define the method in the extended class...
  class~define( methodName, methods[ name ] )
  -- ...and remove it from the caller's .METHODS stringTable.
  methods~remove(name)
End

-- We now check all classes defined in the caller package.

Do With item containingClass Over callerPackage~classes
  Do With Index name Item method Over containingClass~methods
    If method == .Nil Then Iterate -- Skip over hidden methods
    Select
      When Pos(extends,   name) > 0 Then Do
        Parse Var name methodName(extends)className
        Call ProcessNameSpaceAndFindClass
        If HasMethod( class, methodName ) Then
          Raise Syntax 99.900 Array("Method" methodName "already defined in class" className)
      End
      When Pos(overrides, name) > 0 Then Do
        Parse Var name methodName(overrides)className
        Call ProcessNameSpaceAndFindClass
      End
      Otherwise Iterate
    End
    -- Define the method in the extended class...
    class~define( methodName, method )
    -- ...and remove it from the class where it was defined.
    containingClass~delete(name)
  End
End

Exit

ProcessNameSpaceAndFindClass:
  If Pos(":", className) > 0 Then Do
    Parse Var className nameSpace":"className
    package = callerPackage~findNameSpace( nameSpace)
    If package == .Nil Then
      Raise Syntax 98.987 Array(nameSpace, callerPackage~name)
  End
  Else package = callerPackage
  class = package~findClass(className)
  If class == .Nil Then Raise Syntax 98.909 Array(className)
Return

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