/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2006 Rexx Language Association. All rights reserved.    */
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
/* REXX Kernel                                   RexxBuiltinFunctions.h       */
/*                                                                            */
/* REXX String method utility definitions                                     */
/*                                                                            */
/******************************************************************************/

#ifndef OKBIF_INCLUDED
#define OKBIF_INCLUDED

#define get_string(s, p) REQUIRED_STRING(s, p)
#define optional_string(o, d, p) (o == OREF_NULL ? d : get_string(o, p))
size_t get_length(RexxObject *, size_t);
#define optional_length(o, d, p) (o == OREF_NULL ? d : get_length(o, p))
size_t get_position(RexxObject *, size_t);
#define optional_position(o, d, p) (o == OREF_NULL ? d : get_position(o, p))
char get_pad_character(RexxObject *, size_t);
#define get_pad(o, d, p) (o == OREF_NULL ? d : get_pad_character(o, p))
char get_option_character(RexxObject *, size_t);
#define option_character(o, d, p) (o == OREF_NULL ? d : get_option_character(o, p))
#define optionalNonNegative(o, d, p) (o == OREF_NULL ? d : o->requiredNonNegative(p))
#define optionalPositive(o, d, p) (o == OREF_NULL ? d : o->requiredPositive(p))

// the following are to make it easer to port new features over the 4.0 code base.
#define stringArgument(s, p) REQUIRED_STRING(s,p)
#define optionalStringArgument(o, d, p) optional_string(o, d, p)
#define lengthArgument(o, d) get_length(o, d)
#define optionalLengthArgument(o, d, p) optional_length(o, d, p)
#define positionArgument(o, d) get_position(o, d)
#define optionalPositionArgument(o, d, p) optional_position(o, d, p)


#define ch_SPACE ' '
#define ch_NULL  '\0'
#define OVERFLOWSPACE 2                /* space for numeric buffer overflow */

                                       /* Strip function options     */
#define  STRIP_BOTH                'B'
#define  STRIP_LEADING             'L'
#define  STRIP_TRAILING            'T'
                                       /* Datatype function options  */
#define  DATATYPE_ALPHANUMERIC     'A'
#define  DATATYPE_BINARY           'B'
#define  DATATYPE_LOWERCASE        'L'
#define  DATATYPE_MIXEDCASE        'M'
#define  DATATYPE_NUMBER           'N'
#define  DATATYPE_SYMBOL           'S'
#define  DATATYPE_VARIABLE         'V'
#define  DATATYPE_UPPERCASE        'U'
#define  DATATYPE_WHOLE_NUMBER     'W'
#define  DATATYPE_HEX              'X'
#define  DATATYPE_9DIGITS          '9'
#define  DATATYPE_LOGICAL          'O' // lOgical.
                                       /* Verify function options    */
#define  VERIFY_MATCH              'M'
#define  VERIFY_NOMATCH            'N'

                                       /* character validation sets for the */
                                       /* datatype function                 */
#define  HEX_CHAR_STR   "0123456789ABCDEFabcdef"
#define  ALPHANUM       \
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
#define  BINARI         "01"
#define  LOWER_ALPHA    "abcdefghijklmnopqrstuvwxyz"
#define  MIXED_ALPHA    \
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define  UPPER_ALPHA    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

/*********************************************************************/
/*                                                                   */
/*      Name:                   IntToHexdigit                        */
/*                                                                   */
/*      Descriptive name:       convert int to hexadecimal digit     */
/*                                                                   */
/*      Returns:                A hexadecimal digit representing n.  */
/*                                                                   */
/*********************************************************************/

                                       /* convert the number                */
#define IntToHexDigit(n) "0123456789ABCDEF"[(int)n];

#endif
