/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2020 Rexx Language Association. All rights reserved.    */
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

-- a mixin class for defining the standard constants for date and time
-- formats and parsing
::CLASS 'DateFormats' PUBLIC
::CONSTANT NormalDate "d MMM yyyy"
::CONSTANT ISODate    "yyyy'-'MM'-'dd'T'HH':'mm':'ss.ffffff"
::CONSTANT UTCISODate "yyyy'-'MM'-'dd'T'HH':'mm':'ss.ffffffzzzz"
::CONSTANT EuropeanDate "dd/MM/yy"
::CONSTANT LongEuropeanDate "dd/MM/yyyy"
::CONSTANT OrderedDate "yy/MM/dd"
::CONSTANT LongOrderedDate "yyyy/MM/dd"
::CONSTANT StandardDate "yyyy/MM/dd"
::CONSTANT UsaDate "MM/dd/yy"
::CONSTANT LongUsaDate "MM/dd/yyyy"
::CONSTANT CivilTime "h:mmtt"
::CONSTANT NormalTime "hh:mm:ss"
::CONSTANT LongTime "HH:mm:ss.ffffff"
::CONSTANT OrdinalDate "yyyy'-'ddd"
::CONSTANT WeekNumberDate "yyyy'-W'ww'-'D"
::CONSTANT DefaultDateSeparator "/"
::CONSTANT ISODateSeparator "-"
::CONSTANT DefaultTimeSeparator ":"
::CONSTANT ISOTimeSeparator ":"
::CONSTANT DefaultDayNames ("Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday")
::CONSTANT DefaultDayAbbreviations ("Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun")
::CONSTANT DefaultMonthNames ("January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December")
::CONSTANT DefaultMonthAbbreviations ("Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec")
::CONSTANT DefaultCivilLabels ("am", "pm")
::CONSTANT DefaultCivilShortLabels ("a", "p")
::CONSTANT DefaultOrdinalSuffixes ("st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th", "th", "st")
::CONSTANT DefaultDateProperties (.DateFormats~new)

::METHOD init
  use local
  use strict arg

  dateSeparator = self~DefaultDateSeparator
  timeSeparator = self~DefaultTimeSeparator
  dayNames = self~DefaultDayNames
  dayAbbreviations = self~DefaultDayAbbreviations
  monthNames = self~DefaultMonthNames
  monthAbbreviations = self~DefaultMonthAbbreviations
  civilLabels = self~defaultCivilLabels
  civilShortLabels = self~defaultCivilShortLabels
  ordinalSuffixes = self~defaultOrdinalSuffixes

-- settable parser/formatter attributes
::ATTRIBUTE dateSeparator
::ATTRIBUTE timeSeparator

::ATTRIBUTE dayNames
::ATTRIBUTE dayAbbreviations

::ATTRIBUTE monthNames
::ATTRIBUTE monthAbbreviations

::ATTRIBUTE civilShortLabels
::ATTRIBUTE civilLabels

::ATTRIBUTE ordinalSuffixes

-- a class for parsing dates and times using a template
::CLASS 'DateParser' PUBLIC
-- parse a date using the template
::METHOD parse CLASS
  use strict arg date, template, formats = (.DateFormats~DefaultDateProperties)

  date = .Validate~requestClassType("date", date, .String)
  template = .Validate~requestClassType('template', template, .String)
  .Validate~classType("format", formats, .DateFormats)

  context = .DateParserContext~new(date, template, formats)
  return context~parse


-- a class for formatting dates and times using a template
::CLASS 'DateFormatter' PUBLIC
-- formate a date using the template
::METHOD format CLASS
  use strict arg date, template, formats = (.DateFormats~DefaultDateProperties)

  .Validate~classType("date", date, .DateTime)
  template = .Validate~requestClassType('template', template, .String)
  .Validate~classType("format", formats, .DateFormats)

  context = .DateFormatterContext~new(date, template, formats)
  return context~format

-- base class for the DateParserContext and DateFormatterContexts. Implements
-- the operations related the the date template
::CLASS 'DateTemplateContext'
::METHOD init
  expose template templatePosition formats
  use strict arg template, formats
  templatePosition = 1

::ATTRIBUTE template GET
::ATTRIBUTE formats GET

-- process the template and process each the the elements.
-- to build up the result.
::METHOD processTemplate
  expose template templatePosition

  templateLength = template~length

  loop while templatePosition <= templateLength
     type = template~subChar(templatePosition)

     -- a number of the template operations are multi character
     -- with the first character repeated. If one of these, get the expanded
     -- operation
     if type~matchChar(1, 'dDfhHkmMstywz') then do
       operation = self~getPattern
     end
     -- the operations are broken into categories for clarity
     select case (type)
       when 'd', 'D' then do
         self~processDay(operation)
       end
       when 'f' then do
         self~processFraction(operation)
       end
       when 'h', 'H' then do
         self~processHours(operation)
       end
       when 'm' then do
         self~processMinutes(operation)
       end
       when 'M' then do
         self~processMonth(operation)
       end
       when 's' then do
         self~processSeconds(operation)
       end
       when 't' then do
         self~processCivil(operation)
       end
       when 'y' then do
         self~processYear(operation)
       end
       when 'w' then do
         self~processWeekNumber(operation)
       end
       when 'z' then do
         self~processOffset(operation)
       end
       when ':' then do
         templatePosition += 1
         self~processTimeSeparator(type)
       end
       when '/', '-' then do
         templatePosition += 1
         self~processDateSeparator(type)
       end
       when '"', "'" then do
         self~processLiteral(self~getStringLiteral)
       end
       -- we just ignore characters that don't correspond to a
       -- template character
       otherwise do
         templatePosition += 1
         self~processLiteral(type)
       end
     end
  end


-- report a problem with the date template used for parsing
::METHOD templateError
  expose template
  use arg element

  raise syntax 88.932 array(template, element)


-- parse off a literal specified as a string
::METHOD getStringLiteral
  expose template templatePosition

  -- we first need to parse out the literal string, then compare that against the current
  -- date position
  literal = ''
  delimiter = template~subChar(templatePosition)
  templatePosition += 1

  -- look for the end delimiter
  end = template~pos(delimiter, templatePosition)

  -- we use Rexx literal rules here, so need to handle double quotes
  loop while end \= 0
    -- first copy this segment...there may be more if the delimiter is doubled
    literal ||= template~substr(templatePosition, end - templatePosition)
    templatePosition = end + 1

    -- if the next character is not a the same as the delimiter, we are done
    if \template~match(templatePosition, delimiter) then do
      return literal
    end

    -- we have double quotes, add this to the literal and keep looking for the end
    literal ||= delimiter
    templatePosition += 1
    end = template~pos(delimiter, templatePosition)
  end

  -- a missing search means no closing delimiter
  self~templateError(delimiter)


-- get the next pattern from the template, which could be a sequence of
-- repeating characters
::METHOD getPattern private
  expose template templatePosition

  firstChar = template~subChar(templatePosition)

  nextChar = template~verify(firstChar,,templatePosition)

  if nextChar == 0 then do
     pattern = template~substr(templatePosition)
     templatePosition = template~length + 1
  end
  else do
     pattern = template~substr(templatePosition, nextChar - templatePosition)
     templatePosition = nextChar
  end

  return pattern

-- an object to hold the context of the parsing operation on a date
::CLASS 'DateParserContext' subclass DateTemplateContext
::METHOD init
  -- we set a lot of instance variables here, so make everything an object variable
  use local template formats
  use strict arg date, template, formats

  self~init:super(template, formats)

  -- our position in the date we're parsing
  datePosition = 1

  -- The different elements of a date. We set all of these to .nil to determine
  -- if we have conflicting information in the template and also to see if we
  -- have enough pieces to build a final date/time instance

  -- tracks if we have date elements
  haveDate = .false

  year = .nil
  month = .nil
  day = .nil
  offset = .nil

  hours = .nil
  minutes = .nil
  seconds = .nil
  fraction = .nil
  civil = .nil
  civilApplied = .false

  -- some pieces might be specified where date elements might need to be derived from
  weekNumber = .nil
  weekDay = .nil
  ordinal = .nil

-- perform the parsing operation
::METHOD parse
  -- process the template operation
  self~processTemplate
  -- and create a completed date object from the parsed information
  return self~createDate


-- methods that set the different fields, but also check for duplicate
-- or conflict information
::ATTRIBUTE year set
  expose year
  use arg new

  if year \= .nil, year \= new then do
    self~parsingError
  end

  year = new

::ATTRIBUTE month set
  expose month
  use arg new

  if month \= .nil, month \= new then do
    self~parsingError
  end

  month = new

::ATTRIBUTE day set
  expose day
  use arg new

  if day \= .nil, day \= new then do
    self~parsingError
  end

  day = new

::ATTRIBUTE ordinal set
  expose ordinal
  use arg new

  if ordinal \= .nil, ordinal \= new then do
    self~parsingError
  end

  -- this can be used to determine the day and the month, but
  -- we'll wait until the end to determine if it is needed.
  ordinal = new

::ATTRIBUTE hours set
  expose hours civil
  use arg new

  -- this one is a little different. If we have a duplicate, then we
  -- don't want to adjust the hours, which might have already been
  -- adjusted
  if hours \= .nil then do
    if hours \= new then do
      self~parsingError
    end
    -- nothing to process if the same
    else do
      return
    end
  end

  hours = new

  -- we've set this, now there's some additional validation that can
  -- be performed if the civil designation have been set

  if civil \= .nil then do
    self~adjustCivilTime
  end

-- reconcile the hours with a civil time designation
::METHOD adjustCivilTime
  expose hours civil civilApplied

  -- indicate that we've applied this to a time stamp
  civilApplied = .true

  select case (civil)
    -- this is an AM time. If the hours are greater than 12, this is
    -- invalid
    when 1 then do
      if hours >= 12 then do
        self~parsingError
      end
    end
    -- this is a PM time. If the hours are greater than 12, this is
    -- invalid. However we also adjust the hours to 12 hour format if it is
    -- does pass muster
    when 2 then do
      -- the time above 12 is fine for PM, since that is the low end of the range,
      -- all other hours get a 12-hour bump
      if hours < 12 then do
        hours += 12
      end
    end
  end

::ATTRIBUTE minutes set
  expose minutes
  use arg new

  if minutes \= .nil, minutes \= new then do
    self~parsingError
  end

  minutes = new

::ATTRIBUTE seconds set
  expose seconds
  use arg new

  if seconds \= .nil, seconds \= new then do
    self~parsingError
  end

  seconds = new

::ATTRIBUTE fraction set
  expose fraction
  use arg new

  if fraction \= .nil, fraction \= new then do
    self~parsingError
  end

  fraction = new

::ATTRIBUTE offset set
  expose offset
  use arg new

  if offset \= .nil, offset \= new then do
    self~parsingError
  end

  -- we parse this off into minutes. We need it in microseconds to process
  offset = new * 60000000

::ATTRIBUTE weekNumber set
  expose weekNumber
  use arg new

  if weekNumber \= .nil, weekNumber \= new then do
    self~parsingError
  end

  weekNumber = new

::ATTRIBUTE weekDay set
  expose weekDay
  use arg new

  if weekDay \= .nil, weekDay \= new then do
    self~parsingError
  end

  weekDay = new

::ATTRIBUTE civil set
  expose civil hours
  use arg new

  -- this one is a little different. If we have a duplicate, then we
  -- don't want to adjust the hours, which might have already been
  -- adjusted
  if civil \= .nil then do
    if civil \= new then do
      self~parsingError
    end
    -- nothing to process if the same
    else do
      return
    end
  end

  civil = new

  -- we've set this, now there's some additional validation that can
  -- be performed if the hours have been set
  if hours \= .nil then do
    self~adjustCivilTime
  end


-- parse off an optional sign, returning it as either a positive or negative number
::METHOD getSign
  if self~checkChar('+') then do
    return 1
  end
  else if self~checkChar('-') then do
    return -1
  end

  -- no sign, positive is the default
  return 1

-- parse off a number of variable length from a date. The
-- parsed number must between min and max, inclusive
::METHOD getVariableNumber
  expose date datePosition
  use arg length, min, max

  number = ""

  -- loop through until we reach the maximum field length or hit a non-numeric character
  loop length
     if date~matchChar(datePosition, .string~digit) then do
        number ||= date~subChar(datePosition)
        datePosition += 1
     end
     else do
        leave
     end
  end

  if number \= "" then do
     if number >= min & number <= max then do
        return number
     end
  end

  self~parsingError


-- parse off a number of variable length from a date. The
-- parsed number must between min and max, inclusive. It may also
-- have a sign
::METHOD getSignedVariableNumber

  use arg length, min, max

  -- get the sign value
  sign = self~getSign

  -- parse the number and apply the sign
  return self~getVariableNumber(length, min, max) * sign


-- parse off a number of fixed length from a date. The
-- parsed number must between min and max, inclusive
::METHOD getFixedNumber
  expose date datePosition
  use arg length, min, max

  number = date~substr(datePosition, length)
  datePosition += length

  -- must be of the required length and only contain digits
  if number~length \= length | number~verify(.string~digit) \= 0 | number < min | number > max then do
     self~parsingError()
  end

  return number


-- parse off a number of variable fixed from a date. The
-- parsed number must between min and max, inclusive. It may also
-- have a sign
::METHOD getSignedFixedNumber

  use arg length, min, max

  -- get the sign value
  sign = self~getSign

  -- parse the number and apply the sign
  return self~getFixedNumber(length, min, max) * sign


-- check a list of strings against the current date position
::METHOD checkList
  expose date datePosition
  use arg list

  loop with index i item s over list
     if date~caselessMatch(datePosition, s) then do
        datePosition += s~length
        return i
     end
  end

  -- no matches
  self~parsingError

-- checks for the presence of an optional character. If the character
-- is found, the template position is advanced
::METHOD checkChar
  expose date datePosition
  use arg char

  if date~matchChar(datePosition, char) then do
    datePosition += 1
    return .true
  end

  return .false


-- report a problem with the data being parsed
::METHOD parsingError
  expose date
  raise syntax 88.933 array(date, self~template)

-- parse off the different versions of a day specifcation from a Date
::METHOD processDay
  expose haveDate
  use arg type

  select case(type)
    -- day of the month from 1 to 31, without leading zeros
    when "d" then do
      haveDate = .true
      self~day = self~getVariableNumber(2, 1, 31)
    end
    -- day of the month from 1 to 31, with leading zeros
    when "dd" then do
      haveDate = .true
      self~day = self~getFixedNumber(2, 1, 31)
    end
    -- day of the year from 1 to 366, without leading zeros
    when "ddd" then do
      haveDate = .true
      self~ordinal = self~getVariableNumber(3, 1, 366)
    end
    -- day of the year from 1 to 366, with leading zeros
    when "dddd" then do
      haveDate = .true
      self~ordinal = self~getFixedNumber(3, 1, 366)
    end
    -- day of the month from 1 to 31, without leading zeros and an ordinal suffix
    when "ddddd" then do
      haveDate = .true
      day = self~getVariableNumber(2, 1, 31)
      self~day = day
      if \self~checkMatch(self~formats~ordinalSuffixes[day]) then do
        self~parsingError
      end
    end
    -- day of the week as a number
    when "D" then do
      self~weekDay = self~getFixedNumber(1, 1, 7)
    end
    -- day of the week as an abbreviated name
    when "DD" then do
      -- the parser provides the list of weekday abbreviations, which can be customized
      self~weekDay = self~checkList(self~formats~dayAbbreviations)
    end
    -- day of the week as a full namename
    when "DDD" then do
      -- the parser provides the list of weekday names, which can be customized
      self~weekDay = self~checkList(self~formats~dayNames)
    end

    otherwise do
      self~templateError(type)
    end
  end


-- parse the fractional part of a time specification
::METHOD processFraction
  use arg type

  select case(type)
    -- a time fraction in tenths of a second
    when "f" then do
      self~fraction = self~getFixedNumber(1, 0, 9) * 100000
    end
    -- a time fraction in hundredths of a second
    when "ff" then do
      self~fraction = self~getFixedNumber(2, 0, 99) * 10000
    end
    -- a time fraction in thousandths of a second
    when "fff" then do
      self~fraction = self~getFixedNumber(3, 0, 999) * 1000
    end
    -- a time fraction in ten thousandths of a second
    when "ffff" then do
      self~fraction = self~getFixedNumber(4, 0, 9999) * 100
    end
    -- a time fraction in hundred thousandths of a second
    when "fffff" then do
      self~fraction = self~getFixedNumber(5, 0, 99999) * 10
    end
    -- a time fraction in millionths of a second
    when "ffffff" then do
      self~fraction = self~getFixedNumber(6, 0, 999999)
    end

    otherwise do
      self~templateError(type)
    end
  end


-- parse off the different versions of a hour specifcation from a Date
::METHOD processHours
  use arg type

  select case(type)
    -- The hour in 12 hour format with no leading zeros
    when "h" then do
      hours = self~getVariableNumber(2, 1, 12)
      -- we need to turn this into 24 hour format, for now
      if hours == 12 then do
        hours = 0
      end
      self~hours = hours
    end
    -- The hour in 24 hour format with no leading zeros
    when "hh" then do
      self~hours = self~getVariableNumber(2, 0, 23)
    end
    -- The hour in 12 hour format with leading zeros
    when "H" then do
      hours = self~getFixedNumber(2, 1, 12)
      -- we need to turn this into 24 hour format, for now
      if hours == 12 then do
        hours = 0
      end
      self~hours = hours
    end
    -- The hour in 24 hour format with leading zeros
    when "HH" then do
      self~hours = self~getFixedNumber(2, 0, 23)
    end

    otherwise do
      self~templateError(type)
    end
  end


-- parse off the different versions of a minute specifcation from a Date
::METHOD processMinutes
  use arg type

  select case(type)
    -- The minute with no leading zero
    when "m" then do
      self~minutes = self~getVariableNumber(2, 0, 59)
    end
    -- The minute with leading zeros
    when "mm" then do
      self~minutes = self~getFixedNumber(2, 0, 59)
    end

    otherwise do
      self~templateError(type)
    end
  end


-- parse off the different versions of a month specifcation from a Date
::METHOD processMonth
  expose haveDate
  use arg type

  haveDate = .true

  select case(type)
    -- The month with no leading zero
    when "M" then do
      self~month = self~getVariableNumber(2, 1, 12)
    end
    -- The month with leading zeros
    when "MM" then do
      self~month = self~getFixedNumber(2, 1, 12)
    end
    -- The month as an abbreviation
    when "MMM" then do
      -- the parser provides the list of month abbreviations, which can be customized
      self~month = self~checkList(self~formats~monthAbbreviations)
    end
    -- The month as a full name
    when "MMMM" then do
      -- the parser provides the list of month names, which can be customized
      self~month = self~checkList(self~formats~monthNames)
    end

    otherwise do
      self~templateError(type)
    end
  end


-- parse off the different versions of a second specification from a Date
::METHOD processSeconds
  use arg type

  select case(type)
    -- The seconds with no leading zero
    when "s" then do
      self~seconds = self~getVariableNumber(2, 0, 59)
    end
    -- The seconds with leading zeros
    when "ss" then do
      self~seconds = self~getFixedNumber(2, 0, 59)
    end

    otherwise do
      self~templateError(type)
    end
  end


-- parse off the civil time am/pm designators.
::METHOD processCivil
  use arg type

  select case(type)
    -- Just the first letter of AM/PM. This returns 1 for AM, 2 for PM
    when "t" then do
      -- the parser provides the AM/PM labels, which can be customized
      self~civil = self~checkList(self~formats~civilShortLabels)
    end
    -- The full designator
    when "tt" then do
      -- the parser provides the AM/PM labels, which can be customized
      self~civil = self~checkList(self~formats~civilLabels)
    end

    otherwise do
      self~templateError(type)
    end
  end


-- parse off week number designators.
::METHOD processWeekNumber
  use arg type

  select case(type)
    -- Week number as a variable length number from 1 to 53, no leading zeros
    when "w" then do
      self~weekNumber = self~getVariableNumber(2, 1, 53)
    end
    -- Week number as a fixed length number from 1 to 53, with leading zeros
    when "ww" then do
      self~weekNumber = self~getFixedNumber(2, 1, 53)
    end

    otherwise do
      self~templateError(type)
    end
  end


-- resolve the century portion for a year specified as just two digits.
::METHOD resolveCentury
  use arg year

  currentYear = .DateTime~new~year
  -- add in the century from the current year
  year += (currentYear % 100) * 100
  -- did we go back in time by doing that?
  -- if by more than 50 years, we need to use the sliding window
  if year < currentYear then do
      if (currentYear - year) > 50 then do
          year += 100;
      end
  end
  else do
      -- if we ended up too far in the future, step back a century
      if (year - currentYear) > 49 then do
          year -= 100;
      end
  end

  return year


-- parse off the different versions of a year specification from a Date
::METHOD processYear
  expose haveDate
  use arg type

  haveDate = .true

  select case(type)
    -- The year from 0 to 99, no leading zero
    when "y" then do
      self~year = self~resolveCentury(self~getVariableNumber(2, 0, 99))
    end
    -- The year from 00 to 99, leading zeros required
    when "yy" then do
      self~year = self~resolveCentury(self~getFixedNumber(2, 0, 99))
    end
    -- The year from 1 to 9999, no leading zeros
    when "yyy" then do
      self~year = self~getVariableNumber(4, 0001, 9999)
    end
    -- The year from 0001 to 9999, leading zeros required
    when "yyyy" then do
      self~year = self~getFixedNumber(4, 0001, 9999)
    end

    otherwise do
      self~templateError(type)
    end
  end


-- parse off the different versions of a timezone offset specification from a Date
::METHOD processOffset
  expose haveDate
  use arg type

  -- these are all offset values, and for all of these forms "Z" indicates this
  -- is a UTC timestamp. We will check for this special case first before attempting
  -- to parse out the numeric forms.
  if self~checkChar('Z') then do
    self~offset = 0
    return
  end

  select case(type)
    -- The offset as a signed number with no leading zeros
    when "z" then do
      self~offset = self~getSignedVariableNumber(2, 0, 12) * 60
    end
    -- The the offset as a signed two digit number
    when "zz" then do
      self~offset = self~getSignedFixedNumber(2, 0, 12) * 60
    end
    -- The offset as "hhmm"
    when "zzz" then do
      -- get the hours part and convert it to minutes
      base = self~getSignedFixedNumber(2, 0, 12) * 60
      -- now add in the minutes part, respecting the sign
      minutes = self~getFixedNumber(2, 0, 59)
      -- respect the sign here. If the offset is negative, we need to subtract the minutes
      if base < 0 then do
        base -= minutes
      end
      else do
        base += minutes
      end
      self~offset = base
    end
    -- The offset as "hh:mm", where ":" is the defined time separator
    when "zzzz" then do
      -- get the hours part and convert it to minutes
      base = self~getSignedFixedNumber(2, 0, 12) * 60
      -- allow for a time separator
      if \self~checkChar(self~formats~timeSeparator) then do
        self~parsingError
      end
      -- now add in the minutes part, respecting the sign
      minutes = self~getFixedNumber(2, 0, 59)
      -- respect the sign here. If the offset is negative, we need to subtract the minutes
      if base < 0 then do
        base -= minutes
      end
      else do
        base += minutes
      end
      self~offset = base
    end
    otherwise do
      self~templateError(type)
    end
  end


-- parse an expected time separator
::METHOD processTimeSeparator
  self~processLiteral(self~formats~timeSeparator)

-- parse an expected time separator
::METHOD processDateSeparator
  use arg templateSeparator

  -- check the separator configured in the parser first
  if self~checkMatch(self~formats~dateSeparator) then do
    return
  end

  -- as a fallback, check the separator specified in the template
  -- processLiteral will also raise an error if there is no match
  self~processLiteral(templateSeparator)


-- parse off a literal in the template
::METHOD processLiteral
  expose date datePosition

  use arg literal

  if \self~checkMatch(literal) then do
    self~parsingError
  end

-- check for a string match at the given position, stepping over it if found
::METHOD checkMatch
  expose date datePosition
  use arg literal

  -- we always match a null string
  if literal == "" then do
    return .true
  end

  -- the supplied date must have a literal match at this position
  if \date~match(datePosition, literal) then do
    return .false
  end

  -- step over that string
  datePosition += literal~length
  return .true

-- create a DateTime instance from the parsed off information, raising errors
-- if there is a conflict
::METHOD createDate
  use local today

  -- we will likely need some information from the current date, get it upfront
  today = .DateTime~new

  -- if no date information was specified in the template (odd, but possible),
  -- use the January 1, 0001 default that the .DateTime class uses
  -- just fill in all of the parts upfront
  if \haveDate then do
    year = 1
    month = 1
    day = 1
  end

  -- now resolve what missing bits we can. If any parts of the date have
  -- been specified, then we use the current year. This is a situation that
  -- cannot occur with .DateTime
  if year == .nil then do
    year = today~year
  end

  -- month and day are tricky. If both are missing, we can resolve
  -- them from either a ordinal value or from week number/week day
  -- combo. If only one is missing, this is an error

  -- NB, this is an exclusive OR test
  if month == .nil && day == .nil then do
    self~parsingError
  end

  -- this actually means both the month and the day are missing.
  -- see if we can resolve this from either an ordinal or weeknumber
  if month == .nil then do
    -- try the ordinal first
    if ordinal \= .nil then do
      yearDate = .DateTime~new(year, 1, 1)
      -- this must be valid for this year
      if ordinal > yearDate~daysInYear then do
        self~parsingError
      end
      yearDate = yearDate~addDays(ordinal - 1)
      month = yearDate~month
      day = yearDate~day
      -- we'll now pretend this was never specified
      ordinal = .nil
    end
    -- we might have a week number to calculate from
    else if weekNumber \= .nil then do
      -- if only a week number then assume it is the first day of the week
      if weekDay == .nil then do
         weekDay = 1
      end
      -- January 4th of the same year is used to calculate the correction value
      pivot = .datetime~new(year, 1, 4)
      -- this will calculate the ordinal value for the day, which may need adjusting
      ordinal = weekNumber * 7 + weekDay - (pivot~weekDay + 3)

      -- if the ordinal goes less than one, then this belongs to the previous
      -- year and we can use value plus the number of days in the previous year
      -- to get the ordinal value in that year. Note that due to good fortune,
      -- the week number date for January 1, 0001 is 0001-W-01-1, so we will never
      -- have to deal with the year 0000.
      if ordinal < 1 then do
          previousYear = .dateTime~new(year - 1, 12, 31)
          year = previousYear~year
          ordinal = ordinal + previousYear~daysInYear
      end
      -- the very last day of the range we support, December 31, 9999, has
      -- a weekNumberDate of 9999-W52-5, so we never have to deal with a valid
      -- date requiring 5 digits for the year.
      else if ordinal > pivot~daysInYear then do
          year = year + 1
          ordinal -= pivot~daysInYear
      end

      yearDate = .DateTime~new(year, 1, 1)

      yearDate = yearDate~addDays(ordinal - 1)
      month = yearDate~month
      day = yearDate~day
      -- we'll now pretend this was never specified
      ordinal = .nil
      weekNumber = .nil
      weekDay = .nil
    end
  end

  -- unspecified time elements can be easily defaulted to zero
  hours = (hours == .nil)~?(0, hours)
  minutes = (minutes == .nil)~?(0, minutes)
  seconds = (seconds == .nil)~?(0, seconds)
  fraction = (fraction == .nil)~?("000000", fraction)

  -- There could be one dangling element. If we parsed off a civil
  -- time designator, but never parsed off hours, then this is dangling.
  -- this is an error, because a zero default is not really compatible with
  -- that being non-default,
  if civil \= .nil & \civilApplied then do
    self~parsingError
  end

  -- at this point, we should have all of the bits we need, so
  -- create a .DateTime object from this

  date = right(year, 4, '0')||right(month, 2, '0')||right(day, 2, '0')
  time = right(hours, 2, '0')||":"||right(minutes, 2, '0')||":"||right(seconds, 2, '0')||"."||right(fraction, 6, '0')

  -- needed because of the size of the values involved
  numeric digits 18

  -- get the timezone offset now too
  if offset == .nil then do
    offset = today~offset~totalMicroseconds
  end

  -- get this as a basedate value using the local time then create the date object using the offset
  resultDate = .DateTime~new(date('F', date, 'S') + time("F", time, "L"), offset % 60000000)

  -- now that we have this, see if there are any dangling elements to process
  if weekDay \= .nil, weekDay \= resultDate~weekDay then do
    self~parsingError
  end

  -- now that we have this, see if there are any dangling elements to process
  if weekNumber \= .nil, weekNumber \= resultDate~weekNumber then do
    self~parsingError
  end

  -- now that we have this, see if there are any dangling elements to process
  if ordinal \= .nil, ordinal \= resultDate~yearDay then do
    self~parsingError
  end

  -- whew, we can finally return something!
  return resultDate

-- a class for formatting DateTime information using a template
::CLASS 'DateFormatterContext' subclass DateTemplateContext
::METHOD init
  expose date buffer
  use strict arg date, template, formats

  self~init:super(template, formats)

  buffer = .MutableBuffer~new

-- perform the formatting operation
::METHOD format
  expose buffer

  -- process the template operation
  self~processTemplate
  -- return the final formatted version
  return buffer~string

-- append a string to the buffer
::METHOD append
  expose buffer

  use arg new
  buffer~append(new)


-- format the different day versions
::METHOD processDay
  expose date
  use arg type

  select case(type)
    -- day of the month from 1 to 31, without leading zeros
    when "d" then do
      self~append(date~day)
    end
    -- day of the month from 1 to 31, with leading zeros
    when "dd" then do
      self~append(date~day~right(2, '0'))
    end
    -- day of the year from 1 to 366, without leading zeros
    when "ddd" then do
      self~append(date~yearDay)
    end
    -- day of the year from 1 to 366, with leading zeros
    when "dddd" then do
      self~append(date~yearDay~right(3, '0'))
    end
    -- day of the month with an ordinal suffix
    when "ddddd" then do
      self~append(date~day)
      self~append(self~formats~ordinalSuffixes[date~day])
    end
    -- day of the week as a number
    when "D" then do
      self~append(date~weekDay)
    end
    -- day of the week as an abbreviated name
    when "DD" then do
      self~append(self~formats~dayAbbreviations[date~weekDay])
    end
    -- day of the week as a full namename
    when "DDD" then do
      -- the formats provides the list of weekday names, which can be customized
      self~append(self~formats~dayNames[date~weekDay])
    end

    otherwise do
      self~templateError(type)
    end
  end


-- format the fractional portion of a timestamp. The particular format
-- indicates how many digits to use. The value is truncated to that size, not rounded
-- because we have no seconds digit available to round it into
::METHOD processFraction
  expose date
  use arg type

  -- get this padded out to full length first
  base = date~microseconds~right(6, '0')

  select case(type)
    -- a time fraction in tenths of a second
    when "f" then do
      self~append(base~left(1))
    end
    -- a time fraction in hundredths of a second
    when "ff" then do
      self~append(base~left(2))
    end
    -- a time fraction in thousandths of a second
    when "fff" then do
      self~append(base~left(3))
    end
    -- a time fraction in ten thousandths of a second
    when "ffff" then do
      self~append(base~left(4))
    end
    -- a time fraction in hundred thousandths of a second
    when "fffff" then do
      self~append(base~left(5))
    end
    -- a time fraction in millionths of a second
    when "ffffff" then do
      self~append(base~left(6))
    end

    otherwise do
      self~templateError(type)
    end
  end

-- format the different versions of hours
::METHOD processHours
  expose date
  use arg type

  hours = date~hours

  select case(type)
    -- The hour in 12 hour format with no leading zeros
    when "h" then do
      -- we can't use this directly, since a zero value is expressed as
      -- twelve and values greater than 12 need to be adjusted backwards
      -- NB: if hours are == 12, we leave it alone
      if hours > 12 then do
        hours -= 12
      end
      else if hours < 1 then do
        hours = 12
      end

      self~append(hours+0)
    end
    -- The hour in 24 hour format with no leading zeros
    when "hh"then do
      self~append(date~hours+0)
    end
    -- The hour in 12 hour format with leading zeros
    when "H" then do
      -- we can't use this directly, since a zero value is expressed as
      -- twelve and values greater than 12 need to be adjusted backwards
      if hours > 11 then do
        hours -= 12
      end
      else if hours < 1 then do
        hours = 12
      end

      self~append(hours~right(2, '0'))
    end
    -- The hour in 24 hour format with leading zeros
    when "HH" then do
      self~append(hours~right(2, '0'))
    end

    otherwise do
      self~templateError(type)
    end
  end

-- format the minutes of the date
::METHOD processMinutes
  expose date
  use arg type

  minutes = date~minutes

  select case(type)
    -- The minute with no leading zero
    when "m" then do
      self~append(minutes)
    end
    -- The minute with leading zeros
    when "mm" then do
      self~append(minutes~right(2, '0'))
    end

    otherwise do
      self~templateError(type)
    end
  end


-- format the month of the date
::METHOD processMonth
  expose date
  use arg type

  month = date~month

  select case(type)
    -- The month with no leading zero
    when "M" then do
      self~append(month)
    end
    -- The month with leading zeros
    when "MM" then do
      self~append(month~right(2, '0'))
    end
    -- The month as an abbreviation
    when "MMM" then do
      -- the formats provides the list of month abbreviations, which can be customized
      self~append(self~formats~monthAbbreviations[month])
    end
    -- The month as a full name
    when "MMMM" then do
      -- the formats provides the list of month names, which can be customized
      self~append(self~formats~monthNames[month])
    end

    otherwise do
      self~templateError(type)
    end
  end


-- format the different versions of a second specification from a Date
::METHOD processSeconds
  expose date
  use arg type

  seconds = date~seconds

  select case(type)
    -- The seconds with no leading zero
    when "s" then do
      self~append(seconds)
    end
    -- The seconds with leading zeros
    when "ss" then do
      self~append(seconds~right(2, '0'))
    end

    otherwise do
      self~templateError(type)
    end
  end

-- format the civil time marker
::METHOD processCivil
  expose date
  use arg type

  marker = (date~hours > 11)~?(2, 1)

  select case(type)
    -- Just the first letter of AM/PM. This returns 1 for AM, 2 for PM
    when "t" then do
      -- the formats provides the AM/PM labels, which can be customized
      self~append(self~formats~civilShortLabels[marker])
    end
    -- The full designator
    when "tt" then do
      -- the formats provides the AM/PM labels, which can be customized
      self~append(self~formats~civilLabels[marker])
    end

    otherwise do
      self~templateError(type)
    end
  end

-- format the week number
::METHOD processWeekNumber
  expose date
  use arg type

  select case(type)
    -- Week number as a variable length number from 1 to 53, no leading zeros
    when "w" then do
      self~append(date~weekNumber)
    end
    -- Week number as a fixed length number from 1 to 53, with leading zeros
    when "ww" then do
      self~append(date~weekNumber~right(2, '0'))
    end

    otherwise do
      self~templateError(type)
    end
  end


-- format the different styles of the year
::METHOD processYear
  expose date
  use arg type

  year = date~year

  select case(type)
    -- The year from 0 to 99, no leading zero
    when "y" then do
      self~append(year~right(2) + 0)
    end
    -- The year from 00 to 99, leading zeros required
    when "yy" then do
      self~append(year~right(2, '0'))
    end
    -- The year from 1 to 9999, no leading zeros
    when "yyy" then do
      self~append(year+0)
    end
    -- The year from 0001 to 9999, leading zeros required
    when "yyyy" then do
      self~append(year~right(4, '0'))
    end

    otherwise do
      self~templateError(type)
    end
  end

-- copy a literal piece from the template to the output
::METHOD processLiteral
  use arg literal
  self~append(literal)


-- copy a time separator to the output
::METHOD processTimeSeparator
  self~processLiteral(self~formats~timeSeparator)


-- copy a date separator to the output
::METHOD processDateSeparator
  self~processLiteral(self~formats~dateSeparator)


-- format the different versions of a timezone offset specification from a Date
::METHOD processOffset
  expose date
  use arg type

  offset = date~offset~totalMicroseconds

  -- zero offset is always marked as Zulu time
  if offset == 0 then do
    -- type must be valid: "z", "zz", "zzz", or "zzzz"
    if type~length > 4 then
      self~templateError(type)
    self~append('Z')
  end
  else do
    numeric digits 18
    -- we only append hours and minutes, so pull that out
    minutes = offset / (1000000 * 60)
    if minutes < 0 then do
       self~append('-')
       minutes = -minutes
    end
    else do
       self~append('+')
    end

    hours = minutes%60
    minutes = minutes//60
    select case(type)
      -- The offset as a signed number with no leading zeros
      when "z" then do
        self~append(hours)
      end
      -- The offset as a signed two digit number
      when "zz" then do
        self~append(hours~right(2, '0'))
      end
      -- The offset as "hhmm"
      when "zzz" then do
        -- this returns the padded value
        self~append(right(hours, 2, '0')||right(minutes, 2, '0'))
      end
      -- The offset as "hh:mm", where ":" is the defined time separator
      when "zzzz" then do
        -- this returns the padded value
        self~append(right(hours, 2, '0')||self~formats~timeSeparator||right(minutes, 2, '0'))
      end
      otherwise do
        self~templateError(type)
      end
    end
  end


::OPTIONS novalue error
