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
/*  month.rex           Open Object Rexx Samples                              */
/*                                                                            */
/*  Display the days of the month for January 1994                            */
/*                                                                            */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Description:                                                              */
/*  This program is similar to the month1.cmd exec found in the OS/2 2.0      */
/*  REXX User's Guide.  This version demonstrates the use of arrays to        */
/*  replace stems.                                                            */
/******************************************************************************/

/* First, create an array initialized to the days of the week                 */
days = .array~of("Sunday",    "Monday",   "Tuesday", ,
                 "Wednesday", "Thursday", "Friday",  ,
                 "Saturday" )
month = .array~new(31)                      /* Another way to create an array */
startday = 7                                /* First day of month is Saturday */

Do dayofmonth = 1 to 31
  dayofweek = (dayofmonth+startday+days~size-2) // days~size + 1
  Select
    When right(dayofmonth,1) = 1 & dayofmonth <> 11 then th = "st"
    When right(dayofmonth,1) = 2 & dayofmonth <> 12 then th = "nd"
    When right(dayofmonth,1) = 3 & dayofmonth <> 13 then th = "rd"
    Otherwise th = "th"
  end
  /* Store text in the month array, using names in the days array             */
  month[dayofmonth] = days[dayofweek] 'the' dayofmonth||th "of January 1994"
end

month~put( month[1]', New Years day', 1 )   /* Another way to set an array    */
                                            /* element                        */
Do dayofmonth = 1 to 31
  Say month~at(dayofmonth)                  /* Another way to access array    */
                                            /* elements                       */
end
