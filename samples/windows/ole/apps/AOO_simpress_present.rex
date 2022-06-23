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

/**********************************************************************
 AOO_simpress_present.rex using OLE (object linking and embedding) with ooRexx

 Links:  <https://OpenOffice.org>
         <https://wiki.openoffice.org/wiki/Documentation/DevGuide/ProUNO/Bridge/Automation_Bridge>
         <https://www.pitonyak.org/oo.php>
         <https://www.openoffice.org/udk/common/man/spec/ole_bridge.html>

 Using OLE create a new simpress presentation about REXX and ooRexx and then
 display it as a presentation. Demonstrates how to use enumeration to
 enumerate paragraphs of a text and query their 'NumberingLevel' property.
***********************************************************************/

/* create a presentation, then start it */
serviceManager = .OLEObject~new('com.sun.star.ServiceManager')
desktop  = serviceManager~createInstance('com.sun.star.frame.Desktop')
noProps  = .array~new   /* empty array (no properties)   */
document = desktop~loadComponentFromURL('private:factory/simpress', '_blank', 0, noProps)

drawPages = document~getDrawPages      -- get DrawPages
   -- first page
drawPage=drawPages~getByIndex(0)       -- get first (empty) page
drawPage~setPropertyValue("Layout", 0) -- "Title Slide"

title = "ooRexx"
shape = drawPage~getByIndex(0)         -- get first shape (title)
shape~setString(title)

shape = drawPage~getByIndex(1)         -- get second shape (subtitle)
cursor = shape~createTextCursor        -- get XTextCursor
cr="0d"x
shape~insertString(cursor,"Open Object Rexx (ooRexx)"cr, .false)

textRange=shape~getEnd                 -- get a XTextRange
textRange~setPropertyValue("CharHeight", 15)    -- use 15 pixel height
info="(Presentation created:" .dateTime~new")"
textRange~setString(info)              -- set it to this string

   -- add a second page
drawPage=drawPages~~insertNewByIndex(1)~getByIndex(1) -- insert at end, get access
drawPage~setPropertyValue("Layout", 1) -- "Title Content"
drawPage~getByIndex(0)~setString("Open Object Rexx (ooRexx)")  -- first shape

text=drawPage~getByIndex(1)            -- get second shape (listing)
-- add string, supply level
call addItem text, "REXX (IBM)",                                   0
call addItem text, "First released in 1979 for IBM mainframes",    1
call addItem text, "Object REXX (IBM)",                            0
call addItem text, "Object-oriented successor to REXX",            1
call addItem text, "First released in 1994 with IBM's OS/2 Warp",  1
call addItem text, "Negotiations about open-sourcing with RexxLA", 1
call addItem text, "Rexx Language Association (www.RexxLA.org)",   2
call addItem text, "Source code handed over to RexxLA in 2003",    2
call addItem text, "Open Object Rexx (ooRexx by RexxLA)",          0
call addItem text, "First released in 2004 by RexxLA",             1, .false

document~setModified(.false)                 -- inhibit save-as popup
/* OOo OLE interface does not answer hasOleMethod('start') with .true
   therefore forcing "start" to be dispatched to Windows       */
document~getPresentation~dispatch("start")   -- start presentation

say "double-check: dump NumberingLevel of each text paragraph:"
call dumpItems text                    -- demonstrates enumerating listing text


::routine addItem                -- adds string at the given (0-based outline) level
  use arg xText, string, level, bNewParagraph=.true

  xTR=xText~getEnd               -- get XTextRange
  xTR~setPropertyValue("NumberingLevel",level) -- set XTextRange level
  xTR~setString(string)          -- set string

  if bNewParagraph=.true then    -- add new paragraph
     xTR~getEnd~setString("0a"x) -- add linefeed character -> new paragraph

::routine dumpItems              -- show level and string from XText
  use arg xText

  enum=xText~createEnumeration   -- enumerate paragraphs
  do i=1 while enum~hasMoreElements
    xtr=enum~nextElement         -- we need XTextRange's string & properties
    nl=xtr~getPropertyValue("NumberingLevel")
    say "     item #" i": NumberingLevel="pp(nl) pp(xtr~getString)
  end
  return
pp: -- "pretty print", internal routine: return argument enclosed in square brackets
  return "["arg(1)"]"
