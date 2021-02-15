#!@OOREXX_SHEBANG_PROGRAM@
/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
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
/*  timezone.rex             Open Object Rexx Samples                         */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  Sample of performing timezone manipulations.  Also shows a handy          */
/*  technique for embedding data in program.                                 */
/******************************************************************************/
  sampleDate = 'March 7 2009 7:30pm EST'

  Parse var sampleDate month day year time zone

  -- get a datetime object for the part
  basedate = .DateTime~fromNormalDate(day month~left(3) year)
  -- now get an object for the time portion
  basetime = .DateTime~fromCivilTime(time)
  -- this will give us this in a merged format...now we can add in the
  -- timezone informat
  mergedTime = (basedate + basetime~timeofday)~isoDate
  zone = .TimeZoneDataBase~getTimeZone(zone)

  -- parse the full iso date and add in the time zone offset
  finalTime = .DateTime~fromIsoDate(mergedTime, zone~datetimeOffset)

  say 'Original date:' finalTime~utcIsoDate
  say 'Result after adding 12 hours:' finalTime~addHours(12)~utcIsoDate
  say 'Result shifted to UTC:' finalTime~toTimeZone(0)~utcIsoDate
  say 'Result shifted to Pacific Standard Time:' finalTime~toTimeZone(.TimeZoneDataBase~getTimeZone('PST')~datetimeOffset)~utcIsoDate
  say 'Result shifted to Nepal Time:' finalTime~toTimeZone(.TimeZoneDataBase~getTimeZone('NPT')~datetimeOffset)~utcIsoDate

-- a descriptor for timezone information
::class timezone
::method init
  expose code name offset altname region
  use strict arg code, name, offset, altname, region
  code~upper

::attribute code GET
::attribute name GET
::attribute offset GET
::attribute altname GET
::attribute region GET
::attribute datetimeOffset GET
  expose offset
  return offset * 60

-- our database of timezones
::class timezonedatabase
-- initialize the class object.  This occurs when the program is first loaded
::method init class
  expose timezones

  timezones = .directory~new
  -- extract the timezone data which is conveniently stored in a method
  data = self~instanceMethod('TIMEZONEDATA')~source

  loop line over data
    -- skip over the comment delimiters, blank lines, and the 'return'
    -- lines that force the comments to be included in the source
    if line = '/*' | line = '*/' | line = '' | line = 'return' then iterate
    parse var line '{' region '}'
    if region \= '' then do
       zregion = region
       iterate
    end
    else do
       parse var line abbrev . '!' fullname '!' altname . '!' offset .
       timezone = .timezone~new(abbrev, fullname, offset, altname, zregion)
       timezones[timezone~code] = timezone
    end
  end

::method getTimezone class
  expose timezones
  use strict arg code
  return timezones[code~upper]

-- this is a dummy method containing the timezone database data.
-- we'll access the source directly and extract the data held in comments
-- the two return statements force the comment lines to be included in the
-- source rather than processed as part of comments between directives
::method timeZoneData class private
return
/*
{Universal}
UTC  ! Coordinated Universal Time          !        !   0

{Europe}
BST  ! British Summer Time                 !        !  +1
CEST ! Central European Summer Time        !        !  +2
CET  ! Central European Time               !        !  +1
EEST ! Eastern European Summer Time        !        !  +3
EET  ! Eastern European Time               !        !  +2
GMT  ! Greenwich Mean Time                 !        !   0
IST  ! Irish Standard Time                 !        !  +1
KUYT ! Kuybyshev Time                      !        !  +4
MSD  ! Moscow Daylight Time                !        !  +4
MSK  ! Moscow Standard Time                !        !  +3
SAMT ! Samara Time                         !        !  +4
WEST ! Western European Summer Time        !        !  +1
WET  ! Western European Time               !        !   0

{North America}
ADT  ! Atlantic Daylight Time              ! HAA    !  -3
AKDT ! Alaska Daylight Time                ! HAY    !  -8
AKST ! Alaska Standard Time                ! HNY    !  -9
AST  ! Atlantic Standard Time              ! HNA    !  -4
CDT  ! Central Daylight Time               ! HAC    !  -5
CST  ! Central Standard Time               ! HNC    !  -6
EDT  ! Eastern Daylight Time               ! HAE    !  -4
EGST ! Eastern Greenland Summer Time       !        !   0
EGT  ! East Greenland Time                 !        !  -1
EST  ! Eastern Standard Time               ! HNE,ET !  -5
HADT ! Hawaii-Aleutian Daylight Time       !        !  -9
HAST ! Hawaii-Aleutian Standard Time       !        ! -10
MDT  ! Mountain Daylight Time              ! HAR    !  -6
MST  ! Mountain Standard Time              ! HNR    !  -7
NDT  ! Newfoundland Daylight Time          ! HAT    !  -2.5
NST  ! Newfoundland Standard Time          ! HNT    !  -3.5
PDT  ! Pacific Daylight Time               ! HAP    !  -7
PMDT ! Pierre & Miquelon Daylight Time     !        !  -2
PMST ! Pierre & Miquelon Standard Time     !        !  -3
PST  ! Pacific Standard Time               ! HNP,PT !  -8
WGST ! Western Greenland Summer Time       !        !  -2
WGT  ! West Greenland Time                 !        !  -3

{India and Indian Ocean}
IST  ! India Standard Time                 !        !  +5.5
PKT  ! Pakistan Standard Time              !        !  +5
BST  ! Bangladesh Standard Time            !        !  +6 -- Note: collision with British Summer Time
NPT  ! Nepal Time                          !        !  +5.75
BTT  ! Bhutan Time                         !        !  +6
BIOT ! British Indian Ocean Territory Time ! IOT    !  +6
MVT  ! Maldives Time                       !        !  +5
CCT  ! Cocos Islands Time                  !        !  +6.5
TFT  ! French Southern and Antarctic Time  !        !  +5
*/
return
