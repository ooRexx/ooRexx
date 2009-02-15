/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                          */
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
#include "RexxCore.h"
#include "RexxDateTime.hpp"
#include "Numerics.hpp"

// the base time used for Time('T');
RexxDateTime RexxDateTime::unixBaseTime(1970, 1, 1);
// the largest possible date we can handle
RexxDateTime RexxDateTime::maxBaseTime(9999, 12, 31, 23, 59, 59, 999999);

// formatting versions of the days
const char *RexxDateTime::dayNames[] =
{
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday",
};

// text names of the months
const char *RexxDateTime::monthNames[] =
{
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};


// the day in year starting point for each of the months (non-leap year)
wholenumber_t RexxDateTime::monthStarts[] =
{
     0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

// the day in year starting point for each of the months in a leap year.
wholenumber_t RexxDateTime::leapMonthStarts[] =
{
     0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366
};

// the number of days in each of the months
wholenumber_t RexxDateTime::monthdays[] =
{
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};


/**

 * Default constructor for a RexxDateTime instance.  This
 * initializes the time to all zeros.
 */
RexxDateTime::RexxDateTime()
{
    clear();
}


/**
 * Create a RexxDateTime instance from a basetime value.
 *
 * @param basetime The basetime for this instance.
 */
RexxDateTime::RexxDateTime(int64_t basetime)
{
    clear();
    setBaseTime(basetime);
}




/**
 * Create a RexxDateTime instance from a baseDate value.
 *
 * @param basedate
 * @param dummy    Dummy argument to allow constructor overload to work.
 */
RexxDateTime::RexxDateTime(wholenumber_t basedate, bool dummy)
{
    clear();
    setBaseDate(basedate);
}


/**
 * Create a RexxDateTime instance from a year/month/day value.
 *
 * @param y      The current year.
 * @param m      The month.
 * @param d      The day.
 */
RexxDateTime::RexxDateTime(wholenumber_t y, wholenumber_t m, wholenumber_t d)
{
    clear();
    year = y;
    month = m;
    day = d;
}


/**
 * Create a RexxDateTime instance from a fully resolved date
 * time value.
 *
 * @param y      The date year.
 * @param m      The date month.
 * @param d      The date day.
 * @param h      The time hour.
 * @param i      The time minues
 * @param s      The time secons.
 * @param u      The time microseconds.
 */
RexxDateTime::RexxDateTime(wholenumber_t y, wholenumber_t m, wholenumber_t d, wholenumber_t h, wholenumber_t i, wholenumber_t s, wholenumber_t u)
{
    year = y;
    month = m;
    day = d;
    hours = h;
    minutes = i;
    seconds = s;
    microseconds = u;
}


/**
 * Retrieve the basedate from the timestamp.  The basedate
 * is the number of days since 01 Jan 0001, calculated using
 * a Gregorian calendar system for the entire date range.
 *
 * @return The integer value for the basedate, in days.
 */
wholenumber_t RexxDateTime::getBaseDate()
{
    wholenumber_t tempYear = year - 1;
    // calculate base date up to beginning of current year
    wholenumber_t basedate = (tempYear * 365) + (tempYear / 4) - (tempYear / 100) + (tempYear / 400);
    // then add in days in this year
    basedate += getYearDay() - 1;
    return basedate;
}


/**
 * Get the time in seconds from midnight in the current
 * timestamp day.
 *
 * @return The calculated integer time, in seconds.
 */
wholenumber_t RexxDateTime::getTimeSeconds()
{
    return (hours * MINUTES_IN_HOUR + minutes) * SECONDS_IN_MINUTE + seconds;
}


/**
 * Calculate the basetime, returned as the number of microseconds
 * since 00:00:00.000000 on 01 Jan 0001.  The basetime is
 * calculated using the same Gregorian calendar system used
 * to calculate the basedate.
 *
 * @return The basetime as a 64-bit integer value.
 */
int64_t RexxDateTime::getBaseTime()
{
    // get the basedate and convert to seconds (BIG number)
    int64_t time = getBaseDate();
    time *= (int64_t)SECONDS_IN_DAY;
    // now add in the seconds in the day and convert to micro seconds
    time += getTimeSeconds();
    time *= (int64_t)MICROSECONDS;
    // and finally add in the microseconds bit
    time += microseconds;
    return time;
}


/**
 * Return this time as a UTC timestamp (adjusted for
 * the timezone offset);
 *
 * @return
 */
int64_t RexxDateTime::getUTCBaseTime()
{
    return getBaseTime() + timeZoneOffset;
}


/**
 * Calculate the basetime, returned as the number of seconds
 * since 00:00:00.000000 on 01 Jan 1970.  The basetime is
 * calculated using the same Gregorian calendar system used to
 * calculate the basedate.  Times prior to 01 Jan 1970 are
 * returned as a negative number.
 *
 * @return The unix time as a 64-bit integer value.
 */
int64_t RexxDateTime::getUnixTime()
{
    // subtract the baseline time and convert to seconds.
    return (getBaseTime() - unixBaseTime.getBaseTime()) / (int64_t)MICROSECONDS;
}


/**
 * Set the date from a basedate value.  The basedate is the
 * number of days since 01 Jan 0001.
 *
 * @param basedays The basedays value (must be a positive integer).
 */
bool RexxDateTime::setBaseDate(wholenumber_t basedays)
{
    // make sure this is in range.
    if (basedays < 0 || basedays > maxBaseTime.getBaseDate())
    {
        return false;
    }

    // reset all of the fields
    clear();

    basedays++;                  //
                                 // get to start of current 400 years
    year = (basedays / OLYMPIAD_DAYS) * OLYMPIAD;
    // adjust the input date downward
    basedays -= BASE_DAYS(year);
    // if this ended on a boundary, then this was the last day of
    // a leap year
    if (basedays == 0)
    {
        basedays = YEAR_DAYS + 1;
    }
    else
    {
        // now adjust to the start of the current century
        year += (basedays / CENTURY_DAYS) * CENTURY;
        basedays = basedays % CENTURY_DAYS;
        // another boundary condition for the century.
        // Since this is a non-olympiad, it's the last day of
        // a non-leap year.
        if (basedays == 0)
        {
            basedays = YEAR_DAYS;
        }
        else
        {
            // now get to the start of next 4-year leap cycle
            year += (basedays / LEAP_DAYS) * LEAP_CYCLE;
            basedays = basedays % LEAP_DAYS;
            // yet another boundary condition.  if 0, the
            // current day is the last day of the leap year
            if (basedays == 0)
            {
                basedays = YEAR_DAYS + 1;
            }
            else
            {
                // still may have a few more years to process
                year += basedays / YEAR_DAYS;
                // this will be the actual year day
                basedays = basedays % YEAR_DAYS;
                // if basedays is now 0, we're on the last day of the
                // year boundary, so we need to set it accordingly.
                // if not at the boundary, we need to adjust the year
                // because we're currently zero-based,
                if (basedays == 0)
                {
                    basedays = YEAR_DAYS;
                }
                else
                {
                    year++;
                }
            }
        }
    }

    // ok, the year day will differ depending on whether this is a leap year, or not.
    wholenumber_t *monthTable = LeapYear(year) ? leapMonthStarts : monthStarts;

    // now find the relevant month and calculate the month/day fields
    for (int i = 0; ; i++)
    {
        // have we reached the month yet?
        if (monthTable[i] >= basedays)
        {
            month = i;               // the index is the month number
                                     // and adjust for the days
            day = basedays - monthTable[i - 1];
            break;                   /* finished                          */
        }
    }
    return true;
}


/**
 * Set the date and time from a basetime value.  The basetime
 * is the number of microseconds from 00:00:00 on 01 Jan 0001.
 *
 * @param basetime The input timestamp, in microseconds.
 */
bool RexxDateTime::setBaseTime(int64_t basetime)
{
    // make sure this is in range
    if (basetime < 0 || basetime > maxBaseTime.getBaseTime())
    {
        return false;
    }

    // first subtract out the date portion and process it
    int64_t basedays = basetime / MICROSECONDS_IN_DAY;
    basetime -= basedays * MICROSECONDS_IN_DAY;

    // NOTE:  setting the basedate clears all of the time fields
    setBaseDate((wholenumber_t)basedays);

    // extract out the microseconds bit
    microseconds = (wholenumber_t)(basetime % MICROSECONDS);
    // and then down to a seconds field
    basetime = basetime / MICROSECONDS;
    // now pull out the other time fields
    hours = (wholenumber_t)(basetime / SECONDS_IN_HOUR);
    basetime = basetime % SECONDS_IN_HOUR;
    minutes = (wholenumber_t)(basetime / SECONDS_IN_MINUTE);
    seconds = (wholenumber_t)(basetime % SECONDS_IN_MINUTE);

    return true;
}


/**
 * Set the date and time from a unix time value.  The unix time
 * is the number of seconds from 00:00:00 on 01 Jan 1970.  The
 * value may be either positive or negative.
 *
 * @param basetime The input timestamp, in seconds.
 */
bool RexxDateTime::setUnixTime(int64_t basetime)
{
    // calculate this as a base time value.
    int64_t adjustedTime = (basetime * (int64_t)MICROSECONDS) + unixBaseTime.getBaseTime();
    // set the value based on the adjustment.
    return setBaseTime(adjustedTime);
}


/**
 * Set the time stamp using the number of seconds since midnight.
 *
 * @param basetime The basetime, in seconds.
 */
void RexxDateTime::setTimeInSeconds(wholenumber_t basetime)
{
    // clear everything
    clear();
    // extract the field information
    hours = basetime / SECONDS_IN_HOUR;
    basetime = basetime % SECONDS_IN_HOUR;
    minutes = basetime / SECONDS_IN_MINUTE;
    seconds = basetime % SECONDS_IN_MINUTE;
}

/**
 * Clear all of the values within a time object.  This will
 * reset the time to an invalid time value.
 */
void RexxDateTime::clear()
{
    year = 0;
    month = 0;
    day = 0;
    hours = 0;
    minutes = 0;
    seconds = 0;
    microseconds = 0;
}

/**
 * Set the date value using a year/day in year pair.
 *
 * @param newYear The year to set.
 * @param newDay  The day within the year to set.
 */
void RexxDateTime::setDate(wholenumber_t newYear, wholenumber_t newDay)
{
    // set the year, then use that to calculate the month and day information
    year = newYear;
    setDay(newDay);
}


/**
 * Set the day in the year from a day offset.  The year must
 * be valid in the timestamp for this to work properly, as
 * it is necessary to know if the current year is a leap year
 * to properly calculate the month and day.
 *
 * @param basedays The days from the start of the year.
 */
void RexxDateTime::setDay(wholenumber_t basedays)
{
    // ok, the year day will differ depending on whether this is a leap year, or not.
    wholenumber_t *monthTable = LeapYear(year) ? leapMonthStarts : monthStarts;

    // now find the relevant month and calculate the month/day fields
    for (int i = 0; ; i++)
    {
        // have we reached the month yet?
        if (monthTable[i] >= basedays)
        {
            month = i;               // the index is the month number
                                     // and adjust for the days
            day = basedays - monthTable[i - 1];
            break;                   /* finished                          */
        }
    }

}


/**
 * Return the number of days since the start of the year.
 *
 * @return The count of days since the beginning of the contained year.
 */
wholenumber_t RexxDateTime::getYearDay()
{
                                       /* now calculate the yearday         */
    wholenumber_t yearday = (monthStarts[month - 1] + day);
    // if after February in a leap year, add one more day
    if (month > 2 && isLeapYear())
    {
        yearday++;
    }
    return yearday;
}


/**
 * Return the day in the week as an integer value.  Sundy is 0,
 * Monday is 1, etc.
 *
 * @return The integer offset for the day of the week.
 */
wholenumber_t RexxDateTime::getWeekDay()
{
    return getBaseDate() % 7;
}


/**
 * Return the current day of the week, as a name.
 *
 * @return The string name of the timestamp day.
 */
const char *RexxDateTime::getDayName()
{
    return dayNames[getWeekDay()];
}


/**
 * Get the name of the timestamp month, as a string.
 *
 * @return The string name of the timestamp month.
 */
const char *RexxDateTime::getMonthName()
{
    return monthNames[month - 1];
}


/**
 * Parse a date in 'N'ormal format into the timestamp.
 *
 * @param date   The string version of the date.
 * @param sep    The field separator character used in the date.  This argument
 *               can be NULL, which means use the default separator.
 *
 * @return true if the date parses correctly, false for any parsing errors.
 */
bool RexxDateTime::parseNormalDate(const char *date, const char *sep)
{
    return parseDateTimeFormat(date, "DD/MMM/YYYY", sep == NULL ? " " : sep, 0);

}


/**
 * Parse a date in 'S'tandard format into the timestamp.
 *
 * @param date   The string version of the date.
 * @param sep    The field separator character used in the date.  This argument
 *               can be NULL, which means use the default separator.
 *
 * @return true if the date parses correctly, false for any parsing errors.
 */
bool RexxDateTime::parseStandardDate(const char *date, const char *sep)
{
    return parseDateTimeFormat(date, "YYYY/mm/dd", sep == NULL ? "" : sep, 0);
}


/**
 * Parse a date in 'E'uropean format into the timestamp.
 *
 * @param date   The string version of the date.
 * @param sep    The field separator character used in the date.  This argument
 *               can be NULL, which means use the default separator.
 * @param currentYear
 *               The current year used to fill in the centuries portion of the
 *               date.
 *
 * @return true if the date parses correctly, false for any parsing errors.
 */
bool RexxDateTime::parseEuropeanDate(const char *date, const char *sep, wholenumber_t currentYear)
{
    return parseDateTimeFormat(date, "dd/mm/yy",  sep == NULL ? "/" : sep, currentYear);
}


/**
 * Parse a date in 'U'sa format into the timestamp.
 *
 * @param date   The string version of the date.
 * @param sep    The field separator character used in the date.  This argument
 *               can be NULL, which means use the default separator.
 * @param currentYear
 *               The current year used to fill in the centuries portion of the
 *               date.
 *
 * @return true if the date parses correctly, false for any parsing errors.
 */
bool RexxDateTime::parseUsaDate(const char *date, const char *sep, wholenumber_t currentYear)
{
    return parseDateTimeFormat(date, "mm/dd/yy", sep == NULL ? "/" : sep, currentYear);
}


/**
 * Parse a date in 'O'rderd format into the timestamp.
 *
 * @param date   The string version of the date.
 * @param sep    The field separator character used in the date.  This argument
 *               can be NULL, which means use the default separator.
 * @param currentYear
 *               The current year used to fill in the centuries portion of the
 *               date.
 *
 * @return true if the date parses correctly, false for any parsing errors.
 */
bool RexxDateTime::parseOrderedDate(const char *date, const char *sep, wholenumber_t currentYear)
{
    return parseDateTimeFormat(date, "yy/mm/dd", sep == NULL ? "/" : sep, currentYear);
}


/**
 * Parse a time in 'N'ormal format into the timestamp.
 *
 * @param date   The string version of the date.
 *
 * @return true if the date parses correctly, false for any parsing errors.
 */
bool RexxDateTime::parseNormalTime(const char *time)
{
    return parseDateTimeFormat(time, "HH:ii:ss", "", 0);
}


/**
 * Parse a time in 'C'ivil format into the timestamp.
 *
 * @param date   The string version of the date.
 *
 * @return true if the date parses correctly, false for any parsing errors.
 */
bool RexxDateTime::parseCivilTime(const char *time)
{
    return parseDateTimeFormat(time, "cc:iiCC", "", 0);
}


/**
 * Parse a time in 'L'ong format into the timestamp.
 *
 * @param date   The string version of the date.
 *
 * @return true if the date parses correctly, false for any parsing errors.
 */
bool RexxDateTime::parseLongTime(const char *time)
{
    return parseDateTimeFormat(time, "HH:ii:ss.uuuuuu", "", 0);
}


/**
 * Set the time from an hours value.  This sets all other
 * time elements to zero.
 *
 * @param h      The hours value.
 *
 * @return true if the time was set properly.  false if the hours
 *         value was invalid.
 */
bool RexxDateTime::setHours(wholenumber_t h)
{
    if (h < 0 || h >= HOURS_IN_DAY)
    {
        return false;
    }
    hours = h;
    minutes = 0;
    seconds = 0;
    microseconds = 0;
    return true;
}


/**
 * Set the time from a seconds value.  This sets the hour and
 * minute values.  The microseconds field is set to zero.
 *
 * @param s      The seconds value.
 *
 * @return true if the time was set properly.  false if the
 *         value was invalid.
 */
bool RexxDateTime::setSeconds(wholenumber_t s)
{
   if (s < 0 || s >= SECONDS_IN_DAY)
   {
       return false;
   }

   // now break down into component parts
   hours = s / SECONDS_IN_HOUR;
   s = s % SECONDS_IN_HOUR;
   minutes = s / SECONDS_IN_MINUTE;
   seconds = s % SECONDS_IN_MINUTE;
   microseconds = 0;
   return true;
}


/**
 * Set the time from a minutes value.  This sets the hour and
 * minute values.  The seconds and microseconds field are set to
 * zero.
 *
 * @param m      The minutes value.
 *
 * @return true if the time was set properly.  false if the
 *         value was invalid.
 */
bool RexxDateTime::setMinutes(wholenumber_t m)
{
    if (m < 0 || m >= MINUTES_IN_DAY)
    {
        return false;
    }

    // now break down into component parts
    hours = m / MINUTES_IN_HOUR;
    minutes = m % MINUTES_IN_HOUR;
    seconds = 0;
    microseconds = 0;
    return true;
}


/**
 * Adjust the timestamp to a new timezone offset.
 *
 * @param o      The offset value (in microseconds)
 *
 * @return true if the time was set properly.  false if the
 *         value was invalid.
 */
bool RexxDateTime::adjustTimeZone(int64_t o)
{
    // we set the time using a UTC time adjusted by the offset,
    int64_t base = getUTCBaseTime();
    setBaseTime(base - o);
    // then set the offset afterward
    timeZoneOffset = o;
    return true;
}


/**
 * Parse an input date or time vs. a format template that
 * describes the various fields.
 *
 *  Format specifiers are:
 *
 *            '/'   A sepaarator is expected (passed in)
 *            'm'   Start of a month specification
 *            'd'   Start of a day specification
 *            'y'   Start of a 2-digit year spec
 *            'Y'   Start of a 4-digit year spec
 *            'M'   Start of a "named" 3 character month
 *            'h'   Start of a 12-hour hour field
 *            'H'   Start of a 24-hour hour field
 *            'i'   Start of a mInutes field
 *            's'   Start of a seconds field
 *            'u'   Start of a microseconds field
 *            'C'   Start of a Civil time meridian designation
 *            'c'   Start of a Civil time hour (no leading blanks)
 *            ':'   ':' expected at this position
 *            '.'   '.' expected at this position
 *
 * @param date   The input date (or time).
 * @param format The format the date/time is expected to be in.
 * @param sep    A variable separator character expected to be found in the
 *               input.  Separators are marked with "/" in the format template.
 *               The separator can be a null string "".
 * @param currentYear
 *               The current year value (used for formats that don't include
 *               century information).
 *
 * @return true if the date parses correctly, false otherwise.
 */
bool  RexxDateTime::parseDateTimeFormat(const char *date, const char *format, const char *sep, wholenumber_t currentYear)
{
    day = 1;                             // set some defaults for the date portion
    month = 1;
    year = 1;
    const char *inputscan = date;        // and get some scanning pointers
    const char *formatscan = format;

    if (strlen(date) > strlen(format))   // a mismatch on the lengths?  this can't be correct
    {
        return false;
    }
    // scan through this character-by-character, parsing out the pieces
    while (*formatscan != '\0')
    {
        switch (*formatscan)
        {
            // month spec, which requires two digits
            case 'm':
                // parse out the number version
                if (!getNumber(inputscan, MONTH_SIZE, &month, MONTHS))
                {
                    return false;
                }
                // must be in a valid range
                if (month > MONTHS)
                {
                    return false;
                }
                inputscan += MONTH_SIZE;
                formatscan += MONTH_SIZE;
                break;
            // day specifier, which also requires a fixed size
            case 'd':
                // parse out the number version
                if (!getNumber(inputscan, DAY_SIZE, &day))
                {
                    return false;
                }
                // we can't validate the day range until we know the month and year
                inputscan += DAY_SIZE;       /* step both pointers                */
                formatscan += DAY_SIZE;
                break;

            // variable length day specifier...this can be 1-2 digits
            case 'D':
                {
                    // We accept 1 or 2 digits here, so check the second to see if
                    // it's a digit, which will determine our length to scan.
                    int numberLength = 1;

                    if (isdigit(*(inputscan + 1)))
                    {
                        numberLength = 2;
                    }
                    // parse out the number version
                    if (!getNumber(inputscan, numberLength, &day))
                    {
                        return false;
                    }
                    inputscan += numberLength;    // this is stepped a variable amount
                    formatscan += DAY_SIZE;       // and this one is fixed length
                    break;
                }

            // 12 hour field format
            case 'h':
                // parse out the number version
                if (!getNumber(inputscan, HOURS_SIZE, &hours, MAXCIVILHOURS))
                {
                    return false;
                }
                inputscan += HOURS_SIZE;
                formatscan += HOURS_SIZE;
                break;

            // 24 hours format field
            case 'H':
                // parse out the number version
                if (!getNumber(inputscan, HOURS_SIZE, &hours, MAXHOURS))
                {
                    return false;
                }
                inputscan += HOURS_SIZE;     /* step both pointers                */
                formatscan += HOURS_SIZE;
                break;
            // minutes field
            case 'i':
                // parse out the number version
                if (!getNumber(inputscan, MINUTES_SIZE, &minutes, MAXMINUTES))
                {
                    return false;
                }
                inputscan += MINUTES_SIZE;   /* step both pointers                */
                formatscan += MINUTES_SIZE;
                break;

            // seconds field
            case 's':
                // parse out the number version
                if (!getNumber(inputscan, SECONDS_SIZE, &seconds, MAXSECONDS))
                {
                    return false;
                }
                inputscan += SECONDS_SIZE;   /* step both pointers                */
                formatscan += SECONDS_SIZE;
                break;

            // microseconds in a long time
            case 'u':
                // parse out the number version
                if (!getNumber(inputscan, MICRO_SIZE, &microseconds))
                {
                    return false;
                }
                inputscan += MICRO_SIZE;     /* step both pointers                */
                formatscan += MICRO_SIZE;
                break;
            // two digit year value
            case 'y':
                // parse out the number version
                if (!getNumber(inputscan, SHORT_YEAR, &year))
                {
                    return false;
                }
                // add in the current centry
                year += (currentYear / 100) * 100;
                // did we go back in time by doing that?
                // if by more than 50 years, we need to use the sliding window
                if (year < currentYear)
                {
                    if ((currentYear - year) > PAST_THRESHOLD)
                    {
                        year += CENTURY;
                    }
                }
                else
                {
                    // if we ended up too far in the future, step back a centry
                    if ((year - currentYear ) > FUTURE_THRESHOLD)
                    {
                        year -= CENTURY; /* move it back a century            */
                    }
                }
                inputscan += SHORT_YEAR;     /* step both pointers                */
                formatscan += SHORT_YEAR;
                break;

            // 4-digit year
            case 'Y':
                // parse out the number version
                if (!getNumber(inputscan, LONG_YEAR, &year))
                {
                    return false;
                }
                inputscan += LONG_YEAR;      /* step both pointers                */
                formatscan += LONG_YEAR;
                break;

            // 3 character language form
            case 'M':
            {
                month = 0;
                // can months table for a descriptive match
                for (int i = 0; i < MONTHS; i++)
                {
                    /* have a match?                     */
                    if (!memcmp(monthNames[i], inputscan, CHAR_MONTH))
                    {
                        month = i + 1;
                        break;
                    }
                }
                // no match found in the table
                if (month == 0)
                {
                    return false;
                }
                inputscan += CHAR_MONTH;     /* step over the date                */
                formatscan += CHAR_MONTH;    /* step over the date                */
                break;
            }

            // am/pm civil time modifier
            case 'C':
                // "am" time                         */
                if (!memcmp(inputscan, ANTEMERIDIAN, strlen(ANTEMERIDIAN)))
                {
                    // for parsing purposes, 12:nn is really 00:nn
                    if (hours == 12)
                    {
                        hours = 0;
                    }
                }
                // "pm"time
                else if (!memcmp(inputscan, POSTMERIDIAN, strlen(POSTMERIDIAN)))
                {
                    // if 12 something, that's at the beginning of the period.
                    // otherwise, add 12 to convert to 24 hours time internally,
                    if (hours != 12)
                    {
                        hours += 12;
                    }
                }
                // invalid marker
                else
                {
                    return false;
                }
                inputscan += strlen(ANTEMERIDIAN);
                formatscan += strlen(ANTEMERIDIAN);
                break;

            // civil time hours spec, which does not have leading zeros
            case 'c':                      /* civil time hours spec             */
                {
                    // We accept 1 or 2 digits here, so check the second to see if
                    // it's a digit, which will determine our length to scan.
                    int numberLength = 1;

                    if (isdigit(*(inputscan +1)))
                    {
                        numberLength = 2;
                    }
                    // parse out the number version
                    if (!getNumber(inputscan, numberLength, &hours, MAXHOURS))
                    {
                        return false;
                    }
                    inputscan += numberLength;    // this is stepped a variable amount
                    formatscan += HOURS_SIZE;    /* just step the format pointer      */
                    break;
                }
            // a separater
            case '/':
                if (*sep == '\0')
                {
                    // only increment the format...we're not expecting a character in the input
                    formatscan++;
                }
                else
                {
                    // the input must match the provided separator
                    if (*inputscan != *sep)
                    {
                        return false;
                    }
                    formatscan++;
                    inputscan++;
                }
                break;

            // time format separator characters...these are hard coded.
            case ':':
            case '.':
                if (*inputscan != *formatscan)
                {
                    return false;
                }
                formatscan++;
                inputscan++;
                break;
                // bad format?
            default:
                return false;
        }
    }

    // now we need to validity check the date fields.  Zero's not valid for any of them
    if (day == 0 || month == 0 || year == 0)
    {
        return false;
    }

    // now validity check the day of the month is not greater than the max for that month.  This
    // will require a special leapyear check for February
    if (month == FEBRUARY && isLeapYear())
    {
        if (day > LEAPMONTH)       // too many days?
        {
            return false;
        }
    }
    // just check against the table
    else if (day > monthdays[month - 1])
    {
        return false;
    }
    // everything validated
    return true;
}



/**
 * Internal routine used to extract short number fields from
 * a date/time format.
 *
 * @param input  The current input position.
 * @param length The length of the field.
 * @param target The returned integer value.
 *
 * @return true if the field is valid, false for any parsing error.
 */
bool RexxDateTime::getNumber(const char *input, wholenumber_t length, wholenumber_t *target)
{
    wholenumber_t value = 0;                  // the default
    // process the specified number of digits
    while (length-- > 0)
    {
        char digit = *input;
        // add to the accumulator
        if (isdigit(digit))
        {
            value = (value * 10) + (digit - '0');
        }
        else
        {
            // not valid
            return false;
        }
        // step to the next position
        input++;
    }
    *target = value;
    return true;     // good number
}


/**
 * Internal method for parsing short numeric fields from a
 * date/time value.
 *
 * @param input  The current input position.
 * @param length The length of the field.
 * @param target The returned value.
 * @param max    The max value range for the parsed number.
 *
 * @return true if the number is valid, false for any parsing/validation
 *         errors.
 */
bool RexxDateTime::getNumber(const char *input, wholenumber_t length, wholenumber_t *target, wholenumber_t max)
{
    // if this scans correctly, validate the range
    if (getNumber(input, length, target))
    {
        if (*target <= max)
        {
            return true;
        }
    }
    // either invalid characters or out of range
    return false;
}


/**
 * Format a base date into human readable form.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatBaseDate(char *buffer)
{
    // format this into the buffer as a number
    sprintf(buffer, "%d", getBaseDate());
}


/**
 * Format a base time into human readable form.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatBaseTime(char *buffer)
{
    Numerics::formatInt64(getBaseTime(), (char *)buffer);
}


/**
 * Format a unix time into human readable form.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatUnixTime(char *buffer)
{
    Numerics::formatInt64(getUnixTime(), (char *)buffer);
}


/**
 * Format a date as the number of days in the current year.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatDays(char *buffer)
{
    // format this into the buffer as a number
    sprintf(buffer, "%u", getYearDay());
}


/**
 * Format a date in 'E'uropean format.
 *
 * @param buffer The target buffer for the output.
 * @param sep    The separator character used for the fields.  This value can
 *               be NULL, in which case the default is used.  The string value
 *               can also be a null string ("").
 */
void RexxDateTime::formatEuropeanDate(char *buffer, const char *sep)
{
    // make sure we have a valid delimiter
    sep = sep == NULL ? "/" : sep;
    sprintf(buffer, "%02d%s%02d%s%02d", day, sep, month, sep, year % 100);
}


/**
 * Format a date as the name of the current month.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatMonthName(char *buffer)
{
    strcpy(buffer, getMonthName());
}


/**
 * Format a date in 'N'ormal format.
 *
 * @param buffer The target buffer for the output.
 * @param sep    The separator character used for the fields.  This value can
 *               be NULL, in which case the default is used.  The string value
 *               can also be a null string ("").
 */
void RexxDateTime::formatNormalDate(char *buffer, const char *sep)
{
    // make sure we have a valid delimiter
    sep = sep == NULL ? " " : sep;
    sprintf(buffer, "%u%s%3.3s%s%4.4u", day, sep, monthNames[month-1], sep, year);
}


/**
 * Format a date in 'O'rdered format.
 *
 * @param buffer The target buffer for the output.
 * @param sep    The separator character used for the fields.  This value can
 *               be NULL, in which case the default is used.  The string value
 *               can also be a null string ("").
 */
void RexxDateTime::formatOrderedDate(char *buffer, const char *sep)
{
    // make sure we have a valid delimiter
    sep = sep == NULL ? "/" : sep;
    sprintf(buffer, "%02d%s%02d%s%02d", year % 100, sep, month, sep, day);
}


/**
 * Format a date in 'S'tandard format.
 *
 * @param buffer The target buffer for the output.
 * @param sep    The separator character used for the fields.  This value can
 *               be NULL, in which case the default is used.  The string value
 *               can also be a null string ("").
 */
void RexxDateTime::formatStandardDate(char *buffer, const char *sep)
{
    // make sure we have a valid delimiter
    sep = sep == NULL ? "" : sep;
    sprintf(buffer, "%04d%s%02d%s%02d", year, sep, month, sep, day);
}


/**
 * Format a date in 'U'sa format.
 *
 * @param buffer The target buffer for the output.
 * @param sep    The separator character used for the fields.  This value can
 *               be NULL, in which case the default is used.  The string value
 *               can also be a null string ("").
 */
void RexxDateTime::formatUsaDate(char *buffer, const char *sep)
{
    // make sure we have a valid delimiter
    sep = sep == NULL ? "/" : sep;
    sprintf(buffer, "%02d%s%02d%s%02d", month, sep, day, sep, year % 100);
}




/**
 * Format a date as the name of the current day.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatWeekDay(char *buffer)
{

    strcpy(buffer, getDayName()); // copy over the text name
}


/**
 * Format a time in 'C'ivil format.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatCivilTime(char *buffer)
{
    wholenumber_t adjustedHours = hours;
    if (adjustedHours == 0)
    {
        adjustedHours = 12;
    }
    else if (adjustedHours > 12)
    {
        adjustedHours -= 12;
    }
    sprintf(buffer,"%u:%2.2u%s", adjustedHours, minutes, hours >= 12 ? POSTMERIDIAN : ANTEMERIDIAN);
}


/**
 * Format a time in 'H'ours format.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatHours(char *buffer)
{
    sprintf(buffer, "%u", hours);       // just format the hours
}


/**
 * Format a time in 'L'ong format.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatLongTime(char *buffer)
{
    sprintf(buffer, "%2.2u:%2.2u:%2.2u.%6.6u", hours, minutes, seconds, microseconds);
}


/**
 * Format a time in 'M'inutes format.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatMinutes(char *buffer)
{
    sprintf(buffer,"%u", hours * MINUTES_IN_HOUR + minutes);
}


/**
 * Format a time in 'N'ormal format.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatNormalTime(char *buffer)
{
    sprintf(buffer, "%2.2u:%2.2u:%2.2u", hours, minutes, seconds);
}


/**
 * Format a time in 'S'econds format.
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatSeconds(char *buffer)
{
    sprintf(buffer, "%u", (hours * MINUTES_IN_HOUR + minutes) * SECONDS_IN_MINUTE + seconds);
}


/**
 * Format a the time zone offset value
 *
 * @param buffer The target buffer for the output.
 */
void RexxDateTime::formatTimeZone(char *buffer)
{
    // the time zone is a sized value
    Numerics::formatInt64(timeZoneOffset, (char *)buffer);
}
