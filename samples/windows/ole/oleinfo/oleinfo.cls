/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2002-2022 Rony G. Flatscher. All rights reserved.            */
/* Copyright (c) 2023      Rexx Language Association. All rights reserved.    */
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

/***********************************************************************
	program: oleinfo.cls

	purpose: Query the OLE/ActiveX automation interface, create an Object Rexx proxy
				which contains all relevant information in an edited and easy to use form

	usage:   require this file

	returns: an instance of class "rgf.oleinfo"

***********************************************************************/


        -- define invocation types
tmp.="???"
tmp.1="regularMethodCall"
tmp.2="getProperty"
tmp.4="putProperty"
tmp.8="letProperty"		-- a put property (assigning a reference, distinguishment not important for Rexx)
.local~rgf.invKind= tmp.        -- save stem


   -- class to parse and create sorted list of methods
::class rgf.oleinfo 				  public

::method oleObject              attribute       -- OLEobject itself

::method oleString              attribute			-- app_name
::method libname                attribute
::method libdoc                 attribute

::method allMethodDir           attribute			-- contains all methods
::method allMethodRel           attribute
::method allMethodSortedStem    attribute

::method methodDir              attribute

::method getPropertyDir         attribute
::method putPropertyDir         attribute

::method naDir                  attribute       -- directory for storing methods with unknown invocation types

::method getAndPutPropertyDir   attribute
::method getOnlyPropertyDir     attribute
::method putOnlyPropertyDir     attribute

::method eventDir               attribute
::method eventSortedStem        attribute

::method constantSortedStem     attribute

::method init
  expose oleObject oleString libname libdoc allMethodDir allMethodSortedStem methodDir getPropertyDir ,
         putPropertyDir naDir getAndPutPropertyDir getOnlyPropertyDir putOnlyPropertyDir eventDir eventSortedStem,
         constantSortedStem

  use arg oleobject, oleString

  signal on syntax      -- if OLEobject cannot be created, return .nil
  if arg(1)="" | arg(1)=.nil then oleobject=.oleobject~new(oleString, "WITHEVENTS")

  if arg(2, "Omitted") then oleString="n/a"

  methodDir      =.directory~new     -- method directory
  allMethodDir   =.directory~new
  allMethodSortedStem = .stem~new

  getPropertyDir =.directory~new     -- get property directory
  putPropertyDir =.directory~new     -- set property directory

  naDir          =.directory~new

  getAndPutPropertyDir = .directory~new
  getOnlyPropertyDir = .directory~new
  putOnlyPropertyDir = .directory~new

  eventDir       =.directory~new     -- event directory
  eventSortedStem = .stem~new
  constantSortedStem = .stem~new

  m.  = oleobject~getKnownMethods  -- get all known methods
  if m. <> .nil then
  do
     libname=choose( var("m.!LibName"), m.!LibName, "n/a")
     libdoc =choose( var("m.!LibDoc"),  m.!libDoc, "n/a"  )

     -- determine kind of methods, build collection objects
     j=0
     tmpSet=.set~new    -- used to lookup whether name already collected
     mStem.0=m.0
     do i=1 to m.0
        mo=.ole_method~new(m., i )      -- create object
        name=mo~name

        allMethodDir~setentry(name, mo) -- save method-object

        if      mo~invKind=1 then methodDir~setentry(name, mo)      -- a normal method
		  else if mo~invKind=2 then getPropertyDir~setentry(name, mo) -- a get property
        else if mo~invKind=4 | mo~invKind=8 then putPropertyDir~setentry(name, mo)	-- a put property

        uname=translate(name)
        if tmpSet~hasindex(uname) then iterate
        j=j+1
        mStem.j=name
        mStem.0=J
        tmpSet~put(uname)
     end

     call SysStemSort mStem., "Ascending", "Ignore" -- makes problem if used under MSIE (as of: 2002-05-27)
     self~allMethodSortedStem = mStem.  -- save sorted stem
  end

        -- methods which possess an unknown invocation type:
  naDir=allMethodDir~difference(methodDir)~difference(getPropertyDir)~difference(putPropertyDir)

  getAndPutPropertyDir = getPropertyDir~intersection(putPropertyDir)
  getOnlyPropertyDir   = getPropertyDir~difference(putPropertyDir)
  putOnlyPropertyDir   = putPropertyDir~difference(getPropertyDir)

  ev. = oleobject~getKnownEvents           -- get all known events

  if ev. <> .nil then
  do
     evStem.0=ev.0
     do i=1 to ev.0
        ev=.ole_event~new(ev., i )      -- create object
        eventDir~setentry(ev~name, ev)
        evStem.i=ev~name
     end

     call SysStemSort evStem., "Ascending", "Ignore"
     self~eventSortedStem = evStem.     -- save sorted stem
  end

-- call alert "oleobject="oleObject "ev.0="ev.0 "evStem.0="evStem.0 "sel~eventSortedStem="self~eventSortedStem

  c.  = oleobject~getConstant              -- OLE object needs to have been created with the "WITHEVENTS"-option
  if c. <> .nil then
  do
     i=0
     do item over c.
        i=i+1
        constStem.i=substr(item, 2)
     end
     constStem.0=i

     if constStem.0>0 then
	     call SysStemSort constStem., "A", "I" -- get constants and sort ascendingly
     self~constantSortedStem = constStem.
  end

  RETURN

syntax:
   oleObject=.nil
   return


  -- pretty print
::routine pp
  return "[" || arg(1) || "]"


  -- OLE MethodOrEvent
::class methodOrEvent
::method init
  expose name doc params
  use arg stem., idx

  name=stem.idx.!Name

  if var("stem.idx.!doc") then doc=stem.idx.!Doc
                          else doc="n/a" -- .nil

  params =.list~new
  do i=1 to stem.idx.!params.0
     params~insert( .ole_param~new( stem.idx.!params.i.!name, -
                                    stem.idx.!params.i.!type, -
                                    stem.idx.!params.i.!flags ) )
  end

::method name           attribute
::method doc            attribute
::method params         attribute -- list of params, if any

::method makestring     -- create string rendering
  expose name doc params

  tmp=""
  do param over params
     tmp=tmp "," param~makestring
  end
  tmp=substr(tmp,4)
  return name pp( doc ) "(" tmp ")"


  -- OLE Method
::class ole_method subclass MethodOrEvent

::method init
  expose invkind retType
  use arg stem., idx

  forward class (super) continue   -- let super initialize

  invkind=stem.idx.!InvKind
  if invkind=.nil then invkind="??? unknown type #" stem.idx.!InvKind "???"

  -- if invkind="4" then self~name=(self~name || "=")
  retType=stem.idx.!retType

::method invkind        attribute -- 1=method, 2=getProperty, 4=putProperty
::method retType        attribute

::method makestring     -- create string rendering
  expose invKind retType
  return (.rgf.invKind[invKind]~string) (self~makestring:super~string) "retType --->" (retType~string)


  -- OLE Event
::class ole_event subclass MethodOrEvent



  -- OLE Parameter
::class ole_param
::method init
  expose name in out opt type
  parse arg name, type, flags
  in =pos("in",  flags)>0
  out=pos("out", flags)>0
  opt=pos("opt", flags)>0

::method name   attribute
::method in     attribute
::method out    attribute
::method opt    attribute
::method type   attribute

::method makestring             -- render to a string
  expose name in out opt type type

  tmp=name
  if in  then tmp=tmp "in"
         else tmp=tmp "  "

  if out then tmp=tmp"/out"

  tmp=tmp":" type

  if opt then tmp=pp(tmp)       -- indicate optionality by enclosing in brackets
  return tmp


  -- C++/Java like choice ("...?...:...;")
:: routine choose public
  if arg(1)=.true then return arg(2)
  return arg(3)

