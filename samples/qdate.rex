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
/*                                                                            */
/* Description:                                                               */
/* Display a date and moon phase in English                                   */
/*                                                                            */
/******************************************************************************/

-- get the current time
date = .datetime~new

-- get a phase calculator for this date
date = .MoonPhase~new

say date~moonPhase
say date~englishDate

-- A subclass of the date time class that knows about moon phases, and
-- can also display the date using English conventions
::class "MoonPhase" subclass DateTime
-- December 31, 1899...a known date for a new moon
::constant knownNewMoon 18991231
-- the number of days in a lunar cycle
::constant lunation 29.5305889
-- The fraction of a cycle for one day, defined as the
-- the reciprical of a lunation (1 / lunation)
::constant dayPhase 0.0338631907
-- Within this fraction, considered new (dayPhase / 2)
::constant newPhase 0.0169315954
-- Phase position for waxing moon (.25 + newPhase)
::constant waxPhase 0.266931595
-- Fractional position for a full moon phase (.50 + newPhase)
::constant fullPhase 0.516931595
-- and finally the fractional position for a waning moon (.75 + newPhase)
::constant wanePhase 0.766931595
-- return the phase of the moon for this date
::method moonPhase
  -- get the difference from the given date and January 1, 1900, which is
  -- the known date of a
  delta = self - .datetime~fromStandardDate(self~knownNewMoon)

  deltaDays = delta~days
  moonphase = (deltaDays / self~lunation) // 1

  -- get the essential phase range we are sitting in
  select
     when self~newphase <= moonphase & moonphase < self~waxphase
        then do
          targetPhase = self~waxphase
          phaseName = "waxing half"
        end
     when self~waxphase <= moonphase & moonphase < self~fullphase
        then do
          targetPhase = self~fullphase
          phaseName = "full"
        end
     when self~fullphase <= moonphase & moonphase < self~wanephase
        then do
           targetPhase = self~wanephase
           phaseName = "waning half"
        end
     when ((self~wanephase <= moonphase & moonphase <= 1) |,
           (0 <= moonphase & moonphase < self~newphase))
        then do
           targetPhase = self~newphase
           phaseName = "new"
        end
     otherwise /* Should never get here. */
        nop
     end

  -- now calculate the days we are off from being exactly on the phase
  if moonphase > targetPhase
     then extradays = trunc( (1 + targetphase - moonphase ) * self~lunation )
     else extradays = trunc( (targetphase - moonphase) * self~lunation )

  select
     when extradays = 0
        then return 'There will be a' phaseName 'moon tonight.'
     when extradays = 1
        then return 'There will be a' phaseName 'moon tomorrow.'
     otherwise
        dayNames = .array~of('one', 'two', 'three', 'four', 'five', 'six', 'seven')
        return 'There will be a' phaseName 'moon in' dayNames[extradays] 'days.'
  end

-- return the date as a proper English form
::method englishDate

  day = self~day
  unit = day //10

  select
    when day % 10 = 1 then day = day'th'
    when unit=1 then day = day'st'
    when unit=2 then day = day'nd'
    when unit=3 then day = day'rd'
    otherwise day = day'th'
    end
  return "It's" self~dayName 'the' day 'of' self~monthName',' self~year'.'
