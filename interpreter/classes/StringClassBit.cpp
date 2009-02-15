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
/* REXX Kernel                                            StringClassBit.cpp  */
/*                                                                            */
/* REXX string BITxxx methods                                                 */
/*                                                                            */
/******************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "RexxCore.h"
#include "StringClass.hpp"

/******************************************************************************/
/* Arguments:  String to bitand with self                                     */
/*             pad  character to use                                          */
/*                                                                            */
/*  Returned:  New string object                                              */
/******************************************************************************/
RexxString *RexxString::bitAnd(RexxString *string2,
                               RexxString *pad)
{
    char        PadChar;                 /* pad character                     */
    const char *String1;                 /* string 1 pointer                  */
    const char *PadString;               /* padded string part                */
    const char *String2;                 /* string 2 pointer                  */
    size_t      String1Len;              /* string 1 length                   */
    size_t      String2Len;              /* string 2 length                   */
    size_t      MinLength;               /* length of shorter string          */
    size_t      PadLength;               /* length to pad                     */
    size_t      MaxLength;               /* longest length                    */
    RexxString *Retval;                  /* return value                      */
    const char *Source;                  /* source string pointer             */
    char       *Target;                  /* target string pointer             */

                                         /* get string we will be doing bit   */
                                         /* stuff to...                       */
    string2 = optionalStringArgument(string2, OREF_NULLSTRING, ARG_ONE);
    String2Len = string2->getLength();        /* get the string length             */
    String2 = string2->getStringData();       /* get the string data pointer       */
    /* get the pad character             */
    PadChar = optionalPadArgument(pad, (char)0xff, ARG_TWO);

    String1 = this->getStringData();     /* point to the first string         */
    String1Len = this->getLength();      /* get the length                    */
    if (String1Len <= String2Len)
    {      /* string 1 shorter or equal?        */
        MinLength = String1Len;            /* string 1 is the shorter           */
        MaxLength = String2Len;            /* string 2 is the longer            */
        PadString = String2;               /* padding is done on string2        */
        Source = String1;                  /* operate from string 1             */
    }
    else
    {
        MinLength = String2Len;            /* string 2 is the shorter           */
        MaxLength = String1Len;            /* string 1 is the longer            */
        PadString = String1;               /* padding is done on string1        */
        Source = String2;                  /* operate from string 2             */
    }
    PadLength = MaxLength - MinLength;   /* get the padding length            */
                                         /* Duplicate Longer                  */
    Retval = raw_string(MaxLength);
    Target = Retval->getWritableData();  /* point to the tArget               */
    memcpy(Target, PadString, MaxLength);/* now copy in the longer one        */

    while (MinLength--)
    {                /* while shorter has data            */
                     /* and in each character             */
        *Target = *Target & *Source++;
        Target++;                          /* step the target                   */
    }

    while (PadLength--)
    {                /* while pad needed                  */
                     /* and in a pad character            */
        *Target = *Target & PadChar;
        Target++;                          /* step the target                   */
    }
    return Retval;                       /* return result string              */
}

/* the BITOR function */
/******************************************************************************/
/* Arguments:  String to bitor  with self                                     */
/*             pad  character to use                                          */
/*                                                                            */
/*  Returned:  New string object                                              */
/******************************************************************************/
RexxString *RexxString::bitOr(RexxString *string2,
                              RexxString *pad)
{
    char        PadChar;                  /* pad character                     */
    const char *String1;                  /* string 1 pointer                  */
    const char *PadString;                /* padded string part                */
    const char *String2;                  /* string 2 pointer                  */
    size_t      String1Len;               /* string 1 length                   */
    size_t      String2Len;               /* string 2 length                   */
    size_t      MinLength;                /* length of shorter string          */
    size_t      PadLength;                /* length to pad                     */
    size_t      MaxLength;                /* longest length                    */
    RexxString *Retval;                   /* return value                      */
    const char *Source;                   /* source string pointer             */
    char       *Target;                   /* tArget string pointer             */

    /* get string we will be doing bit   */
    /* stuff to...                       */
    string2 = optionalStringArgument(string2, OREF_NULLSTRING, ARG_ONE);
    String2Len = string2->getLength();   /* get the string length             */
    String2 = string2->getStringData();  /* get the string data pointer       */
                                         /* get the pad character             */
    PadChar = optionalPadArgument(pad, 0x00, ARG_TWO);

    String1 = this->getStringData();     /* point to the first string         */
    String1Len = this->getLength();      /* get the length                    */
    if (String1Len <= String2Len)
    {      /* string 1 shorter or equal?        */
        MinLength = String1Len;            /* string 1 is the shorter           */
        MaxLength = String2Len;            /* string 2 is the longer            */
        PadString = String2;               /* padding is done on string2        */
        Source = String1;                  /* operate from string 1             */
    }
    else
    {
        MinLength = String2Len;            /* string 2 is the shorter           */
        MaxLength = String1Len;            /* string 1 is the longer            */
        PadString = String1;               /* padding is done on string1        */
        Source = String2;                  /* operate from string 2             */
    }
    PadLength = MaxLength - MinLength;   /* get the padding length            */
                                         /* Duplicate Longer                  */
    Retval = raw_string(MaxLength);
    Target = Retval->getWritableData();  /* point to the tArget               */
    memcpy(Target, PadString, MaxLength);/* now copy in the longer one        */

    while (MinLength--)
    {                /* while shorter has data            */
                     /* and in each character             */
        *Target = *Target | *Source++;
        Target++;                          /* step the target                   */
    }

    while (PadLength--)
    {                /* while pad needed                  */
                     /* and in a pad character            */
        *Target = *Target | PadChar;
        Target++;                          /* step the target                   */
    }
    return Retval;                       /* return result string              */
}

/* the BITXOR function */
/******************************************************************************/
/* Arguments:  String to bitxor with self                                     */
/*             pad  character to use                                          */
/*                                                                            */
/*  Returned:  New string object                                              */
/******************************************************************************/
RexxString *RexxString::bitXor(RexxString *string2,
                               RexxString *pad)
{
    char        PadChar;                  /* pad character                     */
    const char *String1;                  /* string 1 pointer                  */
    const char *PadString;                /* padded string part                */
    const char *String2;                  /* string 2 pointer                  */
    size_t      String1Len;               /* string 1 length                   */
    size_t      String2Len;               /* string 2 length                   */
    size_t      MinLength;                /* length of shorter string          */
    size_t      PadLength;                /* length to pad                     */
    size_t      MaxLength;                /* longest length                    */
    RexxString *Retval;                   /* return value                      */
    const char *Source;                   /* source string pointer             */
    char       *Target;                   /* tArget string pointer             */

    /* get string we will be doing bit   */
    /* stuff to...                       */
    string2 = optionalStringArgument(string2, OREF_NULLSTRING, ARG_ONE);
    String2Len = string2->getLength();   /* get the string length             */
    String2 = string2->getStringData();  /* get the string data pointer       */
                                         /* get the pad character             */
    PadChar = optionalPadArgument(pad, 0x00, ARG_TWO);

    String1 = this->getStringData();     /* point to the first string         */
    String1Len = this->getLength();      /* get the length                    */
    if (String1Len <= String2Len)
    {      /* string 1 shorter or equal?        */
        MinLength = String1Len;            /* string 1 is the shorter           */
        MaxLength = String2Len;            /* string 2 is the longer            */
        PadString = String2;               /* padding is done on string2        */
        Source = String1;                  /* operate from string 1             */
    }
    else
    {
        MinLength = String2Len;            /* string 2 is the shorter           */
        MaxLength = String1Len;            /* string 1 is the longer            */
        PadString = String1;               /* padding is done on string1        */
        Source = String2;                  /* operate from string 2             */
    }
    PadLength = MaxLength - MinLength;   /* get the padding length            */
                                         /* Duplicate Longer                  */
    Retval = raw_string(MaxLength);
    Target = Retval->getWritableData();  /* point to the tArget               */
    memcpy(Target, PadString, MaxLength);/* now copy in the longer one        */

    while (MinLength--)
    {                /* while shorter has data            */
                     /* and in each character             */
        *Target = *Target ^ *Source++;
        Target++;                          /* step the target                   */
    }

    while (PadLength--)
    {                /* while pad needed                  */
                     /* and in a pad character            */
        *Target = *Target ^ PadChar;
        Target++;                          /* step the target                   */
    }
    return Retval;                       /* return result string              */
}

