#!/usr/bin/env rexx
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2022 Rexx Language Association. All rights reserved.         */
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

/* purpose: utility to read and display ooRexx tickets on Sourceforge via curl commands
            to allow to check tickets and populate/update CHANGES file
   date:    2022-12-20
*/
pkgLocal=.context~package~local
pkgLocal~null.Dev=.nullDevice~new

ticketTypes="bugs","feature-requests","documentation","patches"
pkgLocal~shortNames=.stringTable~of(("bugs"            ,"BUG"), -
                                    ("feature-requests","RFE"), -
                                    ("documentation"   ,"DOC"), -
                                    ("patches"         ,"PAT"))
proj   ="p/oorexx/"
host   ="https://sourceforge.net/"
query  ="/search/?q=status%3Aaccepted+or+status%3Apending"

outArr=.array~new    -- array for redirected results

do label types counter c1 t over ticketTypes
   resArr=.array~new
   say c1"." pp(t)
   cmd=host || proj || t || query
   address system "curl -L" cmd with output using (outArr) error using (.null.dev)
   str=outArr~makeString

   parse var str '<div class="page_list">' '<a href' '</a>' '<a href="' nextPageUrl '"' '(' . firstPage . lastPage ')' '</div>'
   currPage=1
   needle='<td><a href="/'proj || t
   do label group counter c2 while str<>""
      -- parse the received html file
      if t="patches" then  -- does not contain the "creator" in the overview list
      do
         parse var str (needle)  '/">'    nr      '</a></td>'   -
                       (needle)  '/">'    summary '</a></td>'   -
                                 '<td>'   milestone '<'         -
                       '<td class="' '>'  status '</td>'        -
                       '<span'   '>'      dateCreated '</span>' -  -- will have leading and trailing CR-LF
                       '<span'   '>'      dateUpdated '</span>' -  -- will have leading and trailing CR-LF
                       '<td>'             todo        '</'      -
                       str

         -- get creator from detail page (slow, but eventually we'll get there)
         if nr<>"" then
         do
            tmpUrl=quote("https://sourceforge.net/p/oorexx/patches/"nr"/")
            crArr=.array~new
            address system "curl -L" tmpUrl with output using (crArr) error using (.null.dev)
            parse var crArr "Creator:" "<a" ">" creator "</"
            creator=stripCrLf(creator)
         end
      end
      else
      do
         parse var str (needle)  '/">'    nr      '</a></td>'   -
                       (needle)  '/">'    summary '</a></td>'   -
                                 '<td>'   milestone '<'         -
                       '<td class="' '>'  status '</td>'        -
                       '</td>' '<td>'     creator '</td>'       -
                       '<span'   '>'      dateCreated '</span>' -  -- will have leading and trailing CR-LF
                       '<span'   '>'      dateUpdated '</span>' -  -- will have leading and trailing CR-LF
                       '<td>'             todo        '</'      -
                       str
      end

      if \datatype(nr,w) then  -- no more entries, read next page, if any
      do
          currPage+=1
          if currPage>lastPage then leave group
          cmdPage=quote(cmd || "&page=" || (currPage-1))  -- 0-based pages
          address system "curl -L" cmdPage with output using (outArr) error using (.null.dev)
          str=outArr~makeString
          iterate group
      end

      -- resArr~append(.ticket~new(t,nr,summary,status))
      resArr~append(.ticket~new(t,nr,summary,status, milestone, creator, dateCreated, dateUpdated, todo))
   end
   resArr=resArr~sort
   say
   say "sorted:"
   filler=" "~copies(4+1)
   do counter c3 i over resArr
      say c3~right(4)":" i
      say filler i~infos
   end
   say "-*-*"~copies(25)
   say
end

/* ************************************************************************* */
::routine pp
  return "["arg(1)"]"

/* ************************************************************************* */
::routine unEsc
  parse arg val
  val=val~strip
  mb=.mutableBuffer~new
  do while val<>""
    parse var val before "&#" dec ";" val
    if before \=="" then mb~append(before)
    if dec<>"" then mb~append(dec~d2c) -- unescape
  end
  if val<>"" then mb~append(val)

   -- unescape SGML entities &gt; and &amp;
  return mb~string~changeStr("&gt;",">")~changeStr("&amp;","&")


/* ************************************************************************* */
/* class to maintain the ticket information and allow for sorting */
::class     ticket
::attribute type
::attribute nr
::attribute summary
::attribute milestone
::attribute status
::attribute creator
::attribute dateCreated
::attribute dateUpdated
::attribute todo


::method init                    -- constructor
  expose type nr summary milestone status creator dateCreated dateUpdated todo
  use arg type, nr, summary, status, milestone="", creator="", dateCreated="",dateUpdated="", todo=""
  summary=unEsc(summary)
  dateCreated=stripCrLf(dateCreated)   -- remove CR-LF
  dateUpdated=stripCrLf(dateUpdated)   -- remove CR-LF
  if wordpos(todo~lower,"&nbsp; none complete")>0 then todo="" -- no TODO items

::method compareTo               -- determine the sort order: ascending by ticket category, descending by nr
  expose type nr summary status
  use arg other
   -- by type ascendingly
  if type<other~type then return -1
  if type>other~type then return  1

  -- equal, now by nr descendingly
  return -(nr-other~nr)    -- invert


::method makeString              -- default string representation
 expose type nr summary status
 -- return .shortNames[type] "#" nr~right(4) summary
 return "#" nr~right(4) summary

::method infos             -- create a string with additional infos
  expose type nr summary milestone status creator dateCreated dateUpdated todo
  str="       (entered by" creator "on" dateCreated "->" dateUpdated
  if todo<>"" then return str") ***TODO:" pp(todo)
  return str")"


/* ************************************************************************* */
/* class to allow for redirected output to be ignored    */
::class "NullDevice" subclass InputOutputStream
-- InputStream
::method charIn         unguarded   -- implement abstract method
   raise notReady
::method chars          unguarded   -- implement abstract method
   raise notReady
::method lineIn         unguarded   -- implement abstract method
   raise notReady
::method lines          unguarded   -- implement abstract method
   raise notReady

-- OutputStream
::method say            unguarded
::method lineOut        unguarded   -- implement abstract method
::method charOut        unguarded   -- implement abstract method


/* ************************************************************************* */
::routine quote         -- return argument enclosed in square brackets
  return '"'arg(1)'"'

/* ************************************************************************* */
::routine stripCrLf     -- remove CR and LF from string
  parse arg val
  return val~changeStr("0d"x,"")~changeStr("0a"x,"")


