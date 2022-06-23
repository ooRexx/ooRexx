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
/*********************************************************************

 MSPowerPoint_present.rex: using OLE (object linking and embedding) with ooRexx

 Using OLE create a new MS PowerPoint presentation about REXX and
 ooRexx and then start it as a presentation.

*********************************************************************/

-- start PowerPoint
pp = .OLEObject~new("PowerPoint.Application")
pp~visible = .true                                    -- make PP visible

-- add titleSlide
pp~presentations~add~slides~add(1, 1)                 -- add: Title-Slide
titleSlide = pp~activePresentation~slides(1)
titleSlide~shapes(1)~textFrame~textRange = "ooRexx"   -- first Shape is Title
titleSlide~shapes(2)~textFrame~textRange =  "Open Object Rexx (ooRexx)"
info="0d"x"(Presentation created:" .dateTime~new")"   -- add CR plus hint
oRange=titleSlide~shapes(2)~textFrame~textRange~insertAfter(info)
oRange~Font~Size = 15

-- add another slide
activePresentation = pp~activePresentation
-- add(after,type)
slide = activePresentation~slides~add(2, 2)           -- append: Title and Text Slide
slide~shapes(1)~textFrame~textRange = "Open Object Rexx (ooRexx)" -- set title

textRange = slide~shapes(2)~textFrame~textRange
-- add string, supply level
call addItem textRange, "REXX (IBM)",                                   1
call addItem textRange, "First released in 1979 for IBM mainframes",    2
call addItem textRange, "Object REXX (IBM)",                            1
call addItem textRange, "Object-oriented successor to REXX",            2
call addItem textRange, "First released in 1994 with IBM's OS/2 Warp",  2
call addItem textRange, "Negotiations about open-sourcing with RexxLA", 2
call addItem textRange, "Rexx Language Association (www.RexxLA.org)",   3
call addItem textRange, "Source code handed over to RexxLA in 2003",    3
call addItem textRange, "Open Object Rexx (ooRexx by RexxLA)",          1
call addItem textRange, "First released in 2004 by RexxLA",             2

-- go into presentation mode
activePresentation~slideShowSettings~run

say "Press any key to quit!"
parse pull                       -- wait for key press

pp~displayAlerts = .false        -- do not ask user
pp~quit


::routine addItem    -- add string to textRange, set it to given level
  use arg textRange, string, level
  textRange~InsertAfter(string)~IndentLevel = level
  TextRange~InsertAfter("0d"x)   -- add CR, i.e. a new paragraph
