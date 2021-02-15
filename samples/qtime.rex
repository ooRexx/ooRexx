#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                         */
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
/*  qtime.rex           Open Object Rexx Samples                              */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* Description:                                                               */
/* Displays or in real English                                                */
/******************************************************************************/

   -- get the current time
   time = .EnglishTime~new
   say time~englishTime

-- create a subclass of date time that can express the time as an English statement
::class "EnglishTime" subclass DateTime
-- Return the time as a value in English
::method englishTime
   expose chimeCount

   chime = 0        -- we only chime at particular boundaries

   hourNames = .array~of("one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve")

   ot="It's"

   hr = self~hours            -- get the time specifics
   mn = self~minutes
   sc = self~seconds

   if sc>29 then mn=mn+1      -- round up the minutes if we're more than halfway there
   if mn>32 then hr=hr+1      -- if in the later half of the hour, express this as "to" the next hour

   mod=mn//5                  -- find the 5-minute bracket
   select
     when mod=0 then nop      -- exact
     when mod=1 then ot=ot 'just gone'
     when mod=2 then ot=ot 'just after'
     when mod=3 then ot=ot 'nearly'
     when mod=4 then ot=ot 'almost'
     end

   mn=mn+2                    -- round up a bit
   if hr//12=0 & mn//60<=4    -- if within 4 minutes of noon or midnight
    then return ot self~midnoon(hr, mn)  -- handle as special special case
   mn=mn-(mn//5)              -- to nearest 5 mins
   if hr>12
    then hr=hr-12             -- get rid of 24-hour clock
    else
     if hr=0 then hr=12       -- cater for midnight

   select
     when mn=0  then nop      -- add O'clock later --
     when mn=60 then mn=0
     when mn= 5 then ot=ot 'five past'
     when mn=10 then ot=ot 'ten past'
     when mn=15 then ot=ot 'a quarter past'
     when mn=20 then ot=ot 'twenty past'
     when mn=25 then ot=ot 'twenty-five past'
     when mn=30 then ot=ot 'half past'
     when mn=35 then ot=ot 'twenty-five to'
     when mn=40 then ot=ot 'twenty to'
     when mn=45 then ot=ot 'a quarter to'
     when mn=50 then ot=ot 'ten to'
     when mn=55 then ot=ot 'five to'
   end

   ot=ot hourNames[hr]           -- add the hour number
   if mn=0 then ot=ot "o'clock"  -- and O'clock if exact
   ot=ot'.'                      -- and the correct punctuation

   return ot||self~chime(hr, mod, mn)  -- add on a chime if appropriate

-- Special-case Midnight and Noon
::method midNoon private
  use arg hr, mn

  -- we must be on a real 15-minute boundary to add a chime

  -- set up the chime value, if exactly at midnight
  chime = ""
  if mn//60 = 2 then chime = self~chime(12, 0, 0)

  if hr = 12 then return 'Noon.'||chime
           else return 'Midnight.'||chime

-- add on a chime value for this time
::method chime
  use arg hr, mod, mn

  if mod \= 0 | mn // 15 \= 0 then return ""

  -- hourly chime
  if mn // 60 = 0 then  do
     chime='Bong'
     num=hr
  end
  -- quarterly tinkles
  else do
     chime='Ding-Dong'
     num = mn % 15
   end
   -- parens and first chime
   ot = ' ('chime
   loop num - 1
   -- add remainder of chiming sounds
     ot=ot||',' chime
   end
   ot=ot||'!)'      -- ... and final punctuation and parenthesis
   return ot
