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
/******************************************************************************/
/* REXX Kernel                                        RexxDateTime.hpp        */
/*                                                                            */
/* Primitive Rexx Timestamp class                                             */
/*                                                                            */
/******************************************************************************/

#ifndef Included_RexxDateTime
#define Included_RexxDateTime

#include "RexxCore.h"

#define LEAPMONTH       29             /* days in a leap year February      */
#define MAXCIVILHOURS   12             /* maximum hours in a civil time     */
#define MAXHOURS        23             /* maximum hours in 24 hour clock    */
#define MAXSECONDS      60             /* maximum seconds in time           */
#define MAXMINUTES      60             /* maximum minutes in time           */
#define HOURS_IN_DAY    24             /* hours in a day                    */
#define MINUTES_IN_HOUR 60             /* minutes in an hour                */
#define SECONDS_IN_MINUTE 60           /* second in a minute                */
#define MONTHS  12                     /* months in a year                  */
                                       /* seconds in an hour                */
#define SECONDS_IN_HOUR (SECONDS_IN_MINUTE * MINUTES_IN_HOUR)
                                       /* seconds in a complete day         */
#define SECONDS_IN_DAY  (SECONDS_IN_HOUR * HOURS_IN_DAY)
#define MINUTES_IN_DAY  (MINUTES_IN_HOUR * HOURS_IN_DAY)
#define MICROSECONDS    1000000        /* microseconds in a second          */
#define MICROSECONDS_IN_DAY ((int64_t)SECONDS_IN_DAY * (int64_t)MICROSECONDS)

                                       /* days in a 400 year olympiad       */
#define BASE_DAYS(year) (((year) * 365) + ((year) / 4) - ((year) / 100) + ((year) / 400))
                                       /* days in a 400 year olympiad       */
#define OLYMPIAD_DAYS BASE_DAYS(400)   /* days in a 400 year Olympiad       */
#define CENTURY_DAYS  BASE_DAYS(100)   /* days in a 100 year century        */
#define LEAP_DAYS     BASE_DAYS(4)     /* days in a 4 year leap cycle       */
#define YEAR_DAYS     365              /* days in a normal year             */
#define LEAP_CYCLE    4                /* years in a leap cycle             */
#define CENTURY       100              /* years in a century                */
#define OLYMPIAD      400              /* years in an Olympiad cycle        */


#define MONTH_SIZE  2                  /* size of a month date field        */
#define DAY_SIZE    2                  /* size of a day date field          */
#define SHORT_YEAR  2                  /* size of a 2 digit year field      */
#define LONG_YEAR   4                  /* size of a 4 digit year field      */
#define CHAR_MONTH  3                  /* size of the character month field */
#define HOURS_SIZE  2                  /* size of an hours field            */
#define MINUTES_SIZE 2                 /* size of a minutes field           */
#define SECONDS_SIZE 2                 /* size of a seconds field           */
#define MICRO_SIZE   6                 /* size of micro seconds field       */

#define PAST_THRESHOLD   50            /* past threshold for 2 digit years  */
#define FUTURE_THRESHOLD 49            /* future threshold for 2 digit years*/
#define POSTMERIDIAN     "pm"          /* "pm" spec of civil time           */
#define ANTEMERIDIAN     "am"          /* "am" spec of civil time           */
                                       /* leap year calculation             */
#define LeapYear(year) ((!(year % LEAP_CYCLE)) && ((year % CENTURY) || (!(year % OLYMPIAD))))

#define JANUARY     1                  /* positions of the months           */
#define FEBRUARY    2
#define MARCH       3
#define APRIL       4
#define MAY         5
#define JUNE        6
#define JULY        7
#define AUGUST      8
#define SEPTEMBER   9
#define OCTOBER     10
#define NOVEMBER    11
#define DECEMBER    12


class RexxDateTime                      // ooRexx internal time stamp class
{
public:

    RexxDateTime();
    RexxDateTime(int64_t basetime);
    RexxDateTime(wholenumber_t basedate, bool dummy);
    RexxDateTime(wholenumber_t year, wholenumber_t month, wholenumber_t day);
    RexxDateTime(wholenumber_t year, wholenumber_t month, wholenumber_t day, wholenumber_t hour, wholenumber_t minutes, wholenumber_t seconds, wholenumber_t microseconds);

    inline bool isLeapYear() {
        return ((!(year % LEAP_CYCLE)) && ((year % CENTURY) || (!(year % OLYMPIAD))));
    }

    wholenumber_t getBaseDate();
    wholenumber_t getTimeSeconds();
    int64_t getBaseTime();
    int64_t getUTCBaseTime();
    int64_t getUnixTime();
    wholenumber_t getYearDay();
    wholenumber_t getWeekDay();
    const char *getMonthName();
    const char *getDayName();
    bool setBaseDate(wholenumber_t basedays);
    bool setBaseTime(int64_t basetime);
    bool setUnixTime(int64_t basetime);
    void setTimeInSeconds(wholenumber_t basetime);
    void clear();
    void setDate(wholenumber_t newYear, wholenumber_t newDay);
    void setDay(wholenumber_t basedays);
    bool parseNormalDate(const char *date, const char *sep);
    bool parseStandardDate(const char *date, const char *sep);
    bool parseEuropeanDate(const char *date, const char *sep, wholenumber_t currentYear);
    bool parseUsaDate(const char *date, const char *sep, wholenumber_t currentYear);
    bool parseOrderedDate(const char *date, const char *sep, wholenumber_t currentYear);
    bool parseNormalTime(const char *date);
    bool parseCivilTime(const char *date);
    bool parseLongTime(const char *date);
    bool setHours(wholenumber_t h);
    bool setSeconds(wholenumber_t s);
    bool setMinutes(wholenumber_t m);
    bool adjustTimeZone(int64_t o);
    void formatBaseDate(char *buffer);
    void formatBaseTime(char *buffer);
    void formatUnixTime(char *buffer);
    void formatDays(char *buffer);
    void formatEuropeanDate(char *buffer, const char *sep);
    void formatMonthName(char *buffer);
    void formatNormalDate(char *buffer, const char *sep);
    void formatOrderedDate(char *buffer, const char *sep);
    void formatStandardDate(char *buffer, const char *sep);
    void formatUsaDate(char *buffer, const char *sep);
    void formatWeekDay(char *buffer);
    void formatCivilTime(char *buffer);
    void formatHours(char *buffer);
    void formatLongTime(char *buffer);
    void formatMinutes(char *buffer);
    void formatNormalTime(char *buffer);
    void formatSeconds(char *buffer);
    void formatTimeZone(char *buffer);
    inline void setTimeZoneOffset(int64_t o) { timeZoneOffset = o; }
    inline int64_t getTimeZoneOffset() { return timeZoneOffset; }

    boolean         valid;
    int             year;                // current year
    int             month;               // month of the year
    int             day;                 // day of the month
    int             hours;               // hour of the day (24-hour)
    int             minutes;             // minute of the hour
    int             seconds;             // second of the minute
    int             microseconds;        // microseconds
    int64_t         timeZoneOffset;      // offset from UTC for this time stamp

protected:

    bool parseDateTimeFormat(const char *date, const char *format, const char *sep, wholenumber_t currentYear);
    bool getNumber(const char *input, wholenumber_t length, int *target);
    bool getNumber(const char *input, wholenumber_t length, int *target, int max);

    static const char  *dayNames[];      // table of day names for date formatting
    static const char  *monthNames[];    // table of month names for date formatting
    static int          monthStarts[];         // table of first day of month values for non-leap years
    static int          leapMonthStarts[];     // table of first day of month values for leap years
    static int          monthdays[];           // month number of days mapping table
    static RexxDateTime unixBaseTime;   // a base time used for Date('T')/Time('T') calculations.
    static RexxDateTime maxBaseTime;    // the largest possible date we can handle.

};

#endif
