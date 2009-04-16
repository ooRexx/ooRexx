/*----------------------------------------------------------------------------*/
/* Copyright (c) 2008-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  File: OleUtils.frm

  Purpose:
    Provides some useful utilities for working with the .OLEObject class.

  Assumes:
    ooRexx version 3.1.2 as a minimum.

  Notes:
    This framework was originally written to work on ooRexx 3.1.2, and it still
    does.  Because of that it has some utility functions that would not be
    needed on later versions of ooRexx.  The toLower() function for instance.
    It also does not use some constructs like:  'use strict arg' that would make
    things easier.

  Public Routines:
    This is a list of the public routines and their syntax.  Arguments in square
    brackets indicate they are optional. Complete details on usage is in the
    header comments for each routine.

      oleOjbect = createOleOjbect(id, [verbose])
      boolean = displayKnownMethods(oleObj, [verbose])
      boolean = displayKnownConstants(oleObj)
      boolean = toLower(str)
      boolean = isRexxTrue(obj)
      boolean = isOORexx4OrLater()
      mode = getAddressingMode()

\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

-- End of entry point.

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\
  Directives, Classes, or Routines.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/* createOleObject( id ) - - - - - - - - - - - - - - - - - - - - - - - - - - -*\

  Creates an .OLEObject instance, a proxy for the specified COM object.  This
  routine is used to trap the REXX error that happens when the proxied COM
  object can not be created.

  Input:
    id       REQUIRED
      The string used to create the COM object.  I.e., the ProgID or CLSID.

    verbose  OPTIONAL
      If true and the OleObject is not created, the error message is displayed.
      If false, the default, the message is not displayed.

  Returns:
    An instance of .OLEObject on success, .nil on failure.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::routine  createOleObject public
  use arg id, verbose

  if isRexxTrue( arg( 2 ) ) then
    verbose = .true
  else
    verbose = .false

  signal on syntax name returnNil

  oleObject = .OLEObject~new( id,"NOEVENTS" )
  signal on syntax
  return oleObject

returnNil:
  signal on syntax
  if verbose then
    say "Error" rc":" errortext(rc)||'0d0a'x||condition('o')~message

  return .nil
-- End createOleObject( id, verbose )

/* displayKnownMethods( oleObj, verbose )- - - - - - - - - - - - - - - - - - -*\

  Formats and displays the known methods of an .OLEObject instance.  Known
  methods can only be displayed for OLE / COM objects that provide TypeInfo.  If
  the there is no known information, a simple string stating as much is
  displayed.

  Input:
    oleObj REQUIRED
      An instance of the .OLEObject whose known methods are to be displayed.

    verbose  OPTIONAL
      If true all information concerning the methods is displayed.  Parameters,
      parameter types, return type, etc..  If false, the default, only the
      method names are displayed.

  Returns:
    0 if the specified object is not an instance of the .OLEObject classs,
    otherwise 1.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::routine  displayKnownMethods public
  use arg oleObj, verbose

  if \ oleObj~isInstanceOf( .OLEObject ) then do
    say "Known methods can only be displayed for instances of the .OLEObject."
    say "  Arg 1 class:" oleObj~class
    return 0
  end

  if isRexxTrue( arg( 2 ) ) then
    verbose = .true
  else
    verbose = .false

  say
  j = printInstanceInfo( oleObj )

  known. = oleObj~getKnownMethods

  if known. == .nil then do
    say "There is no known method information for this object"
    return 1
  end

  say "Containing Type Library:" known.!LIBNAME
  if known.!LIBDOC~left( 2 ) <> "!L" then
    say "Library Description:    " known.!LIBDOC
  say

  say "COM Class:        " known.!COCLASSNAME
  if known.!COCLASSDOC~left( 2 ) <> "!C" then
    say "Class Description:" known.!COCLASSDOC
  say "Known methods:    " known.0
  say

  if \ verbose then do
    say "  Methods:"
    do i = 1 to known.0
      say "    " known.i.!NAME
    end
    return 1
  end

  do i = 1 to known.0
    ret  = known.i.!RETTYPE
    name = known.i.!NAME
    doc  = known.i.!DOC
    invk = known.i.!INVKIND

    say " " name

    if doc~pos( "!DOC" ) == 0 then
      say "    Decscription:" doc

    say "   " invkindToString( invk ) "returns" ret
    say

    if ret == "VT_VOID" then
      line = "      obj~"name
    else
      line = "      " || changeVariant( ret ) || "= obj~"name

    select
      when invk == 2 then
        say line

      when known.i.!PARAMS.0 == 0 then
        say line"()"

      otherwise do
        line   = line"( "
        indent = " "~copies( line~length )

        do j = 1 to known.i.!PARAMS.0
          param = known.i.!PARAMS.j.!TYPE known.i.!PARAMS.j.!FLAGS             -
                  known.i.!PARAMS.j.!NAME

          select
            when j == 1 & known.i.!PARAMS.0 == 1 then do
              say line || param" )"
            end
            when j == 1 then do
              say line || param","
            end
            when j == known.i.!PARAMS.0 then do
              say indent || param" )"
            end
            otherwise do
              say indent || param","
            end
          end
          -- End select
        end
        -- End do j = 1 to known.i.!PARAMS.0
      end
      -- End otherwise do
    end
    -- End select

    say
  end
  -- End do i = 1 to known.0

return 1
-- End displayKnownMethods( oleObj )

/* displayKnownConstants( oleObj ) - - - - - - - - - - - - - - - - - - - - - -*\

  Prints out all the known constants for the specified object, if any are
  available.

  Input:
    oleObj  REQUIRED
      An instance of the .OLEObject whose known methods are to be displayed.

  Returns:
    0 if the oleObj argument was not an instance of .OLEObject, otherwise 1.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::routine  displayKnownConstants public
  use arg oleObj

  if \ oleObj~isInstanceOf( .OLEObject ) then do
    say "Known constants can only be displayed for instances of the .OLEObject."
    say "  Arg 1 class:" oleObj~class
    return 0
  end

  say
  j = printInstanceInfo( oleObj )

  constants = oleObj~getConstant
  if constants == .nil | constants~items == 0 then do
    say "There are no known constants for this object"
    return 1
  end

  say "Known constants:" constants~items
  say

  -- Some of Microsoft's constant names are very long.
  line = " "~copies( 42 ) || "= "
  do name over constants
    say line~overlay( name~substr( 2 ), 3 ) || constants[name]
  end

return 1
-- End displayKnownConstants( oleObj )

/* toLower( str )- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\

  Changes the specified string to lower case.

  Input:
    str REQUIRED
      The string to work with.

  Returns:
    The string with all upper case letters changed to lower case.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::routine  toLower public
  use arg str

  if str~class <> .string then
    return str

  lower = str~translate( "abcdefghijklmnopqrstuvwxyz",                          -
                         "ABCDEFGHIJKLMNOPQRSTUVWXYZ" )

return lower
-- End toLower( str )

/* isRexxTrue( obj ) - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\

  Tests if obj is strictly .true.  (To some degree, actually just tests that obj
  is exactly '1'.)

  Input:
    obj REQUIRED
      The object to test.

  Returns:
    True if obj is strictly true, otherwise false.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::routine  isRexxTrue public
  use arg obj

  if obj~class == .string then
    if obj~datatype( 'W' ) then
      if obj == 1 then
        return .true

return .false
-- End isRexxTrue( obj )

/* isOORexx4OrLater( ) - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\

  Returns true if the running interpreter is ooRexx 4.0.0 or later.

  Input:
    None.

  Returns:
    True if this is ooRexx 4.0.0 or later, otherwise false.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::routine  isOORexx4OrLater public

  parse version interpreterName languageLevel interpreterDate
  parse var interpreterName junk "_" ver "." moreJunk
  if ver >= 4 then return .true

return .false
-- End isOORexx4OrLater( )

/** getAddressingMode()
 * Determine if this is a 32-bit or 64-bit interpreter.
 */
::routine  getAddressingMode public

  tmpOutFile = 'tmpXXX_delete.me'

  'rexx -v >' tmpOutFile '2>&1'

  fsObj = .stream~new(tmpOutFile)
  tmpArray = fsObj~arrayin
  parse value tmpArray[3] with . . mode
  fsObj~close

  j = SysFileDelete(tmpOutFile)

return mode

/* changeVariant( vt ) - - - - - - - - - - - - - - - - - - - - - - - - - - - -*\

  Helper function to turn a VARTYPE symbol into a "prettified" object name.

  E.g., VT_DISPATCH becomes dispatchObj, VT_I4 becomes i4Obj.

  Input:
    vt REQUIRED

  Returns:
    The prettified version of the specified VARTYPE.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::routine  changeVariant
  use arg vt

  -- TODO: it would be nice to turn VT_I4 to something like int4ByteObj, VT_R4
  -- to float4ByteObj, etc.

return toLower( vt~substr( 4 ) ) || "Obj"
-- End changeVariant( vt )

/* invkindToString( kind ) - - - - - - - - - - - - - - - - - - - - - - - - - -*\

  Helper function to return a string value for the specified invocation type.

  Input:
    kind REQUIRED
      A number representing a COM INVOKEKIND enumeration.

  Returns:
    Returns the enumeration symbol, (string symbol) for the specified kind.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::routine  invkindToString
  use arg kind

  select
    when kind == 1 then
      kindString = "<Function>"
    when kind == 2 then
      kindString = "<Property get>"
    when kind == 4 then
      kindString = "<Property put>"
    when kind == 8 then
      kindString = "<Property put by reference>"
    otherwise
      kindString = "<Error Invalid! ("kind")>"
  end
  -- End select

return kindString
-- End invkindToString( kind )

/* printInstanceInfo( oleObj ) - - - - - - - - - - - - - - - - - - - - - - - -*\

  Helper function to print out instance information for an .OLEObject object.

  Input:
    oleObj REQUIRED
      The object whose instance information is to be printed.

  Returns:
    0, always.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
::routine  printInstanceInfo
  use arg oleObj

  progID   = oleObj~!getVar( "!PROGID" )
  clsID    = oleObj~!getVar( "!CLSID" )
  disp     = oleObj~!getVar( "!IDISPATCH" )
  typeInfo = oleObj~!getVar( "!ITYPEINFO" )

  if isOORexx4OrLater() then do
    if progID == .nil then
      progID = "null"
    if clsID == .nil then
      clsID = "null"
    if disp == .nil then
      disp = "null"
    if typeInfo == .nil then
      typeInfo = "null"
  end
  else do
    if progID~left( 2 ) == "!P" then
      progID = "null"
    if clsID~left( 2 ) == "!C" then
      clsID = "null"
    if disp~left( 3 ) == "!ID" then
      disp = "null"
    if typeInfo~left( 3 ) == "!IT" then
      typeInfo = "null"
  end

  say "ProgID:          " progID
  say "ClsID:           " clsID
  say "Dispatch Pointer:" disp
  say "TypeInfo Pointer:" typeInfo
  say

return 0
-- End printInstanceInfo( oleObj )



/* - - - - - - - - - - End of file: OleUtils.rex- - - - - - - - - - - - - - - */
