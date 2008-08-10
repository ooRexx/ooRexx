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
/* REXX Kernel                                                  okmath.h      */
/*                                                                            */
/* REXX Math engine for NumberString Objects.                                 */
/*                                                                            */
/******************************************************************************/


#ifndef OEMATH
#define OEMATH
/* codes for arithmetic operation types */
#define OT_PLUS               201
#define OT_MINUS              202
#define OT_MULTIPLY           203
#define OT_DIVIDE             204
#define OT_INT_DIVIDE         205
#define OT_REMAINDER          206
#define OT_POWER              207
#define OT_MAX                208
#define OT_MIN                209

/* Function prototypes for NumberStringClass/StringClass */

// TODO:  make all of these functions static methods on numberstring

size_t HighBits(size_t number);
void Subtract_Numbers( RexxNumberString *larger, const char *largerPtr, wholenumber_t aLargerExp,
                       RexxNumberString *smaller, const char *smallerPtr, wholenumber_t aSmallerExp,
                       RexxNumberString *result, char **resultPtr);
                                            /* ************************************ */
                                            /* Following functions are in oemath2.c */
                                            /* ************************************ */
char *AddMultiplier( char *, wholenumber_t, char *, int);
char * SubtractDivisor(char *data1, size_t length1,
                       char *data2, size_t length2,
                       char *result, int Mult);
char *MultiplyPower(char *leftPtr, RexxNumberStringBase *left,
                     char *rightPtr, RexxNumberStringBase *right,
                     char *OutPtr, size_t OutLen, size_t NumberDigits);
char *DividePower(char *AccumPtr, RexxNumberStringBase *Accum, char *Output, size_t NumberDigits);
char * AddToBaseSixteen(int, char *, char *);
char * AddToBaseTen(int, char *, char *);
char * MultiplyBaseSixteen(char *, char *);
char * MultiplyBaseTen(char *, char *);

#ifndef ORDCOMP
#define ORDCOMP

#define BYTE_SIZE              8                      /* Number of bits in a byte   */
#define LONGBITS         (sizeof(size_t) * BYTE_SIZE) /* Number of bytes in size_t    */
#define ROUND                  true                   /* Perform rounding           */
#define NOROUND                false                  /* no Rounding                */

                                       /* temporary buffer allocation       */
#define buffer_alloc(s)  (new_buffer(s)->getData())
/* define the digits limit for "fast path" processing */
#define FASTDIGITS 35

#endif
#endif
