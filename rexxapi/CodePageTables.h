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


/*********************************************************************/
/*                                                                   */
/*  Module Name : asctoken.h                                         */
/*                                                                   */
/*  Description : This header file contains the codepoint table used */
/*                to drive the scanner for REXX SAA PL Tokenizer.    */
/*                                                                   */
/*********************************************************************/

#include "Characters.h"

/*********************************************************************/
/* DO NOT CHANGE THE FOLLOWING #defines WITHOUT CHANGING THE SCANNER */
/* DRIVER TABLE IN TOKENIZE.C.                                       */
/* These defines are indices into an array of function pointers.     */
/* Each function performs a scanning operation as described.         */
/* Index 0 must never be used; if used it will be considered to be   */
/* the same as Invalid.                                              */
/*********************************************************************/

#define White             1
#define Name              2
#define String            3
#define Oper              4
#define Punct             5
#define Number            6
#define Dot               7
#define Invalid           8


/*********************************************************************/
/* CHANGING AN ENTRY IN THE FOLLOWING TABLE MAY CAUSE THE TOKENIZER  */
/* TO PRODUCE THE WRONG TOKENS.                                      */
/* This table is the ASCII code page. Total of 256 code points.      */
/* This table contains the above scan function numbers combined with */
/* the REXX SAA PL meaning for the code points described there in.   */
/*    STARTSYM   ---   Code point character can start a REXX symbol  */
/*    INSYM      ---   Code point character can occur as part of a   */
/*                     REXX symbol                                   */
/*    INNUM      ---   Code point character can occur as part of a   */
/*                     REXX number                                   */
/*********************************************************************/

UCHAR NEAR first_char[] = {
   /* '00'x */       White  ,
   /* '01'x */       Invalid,
   /* '02'x */       Invalid,
   /* '03'x */       Invalid,
   /* '04'x */       Invalid,
   /* '05'x */       Invalid,
   /* '06'x */       Invalid,
   /* '07'x */       Invalid,
   /* '08'x */       Invalid,
   /* '09'x */       White  ,
   /* '0a'x */       Invalid,
   /* '0b'x */       Invalid,
   /* '0c'x */       Invalid,
   /* '0d'x */       Invalid,
   /* '0e'x */       Invalid,
   /* '0f'x */       Invalid,
   /* '10'x */       Invalid,
   /* '11'x */       Invalid,
   /* '12'x */       Invalid,
   /* '13'x */       Invalid,
   /* '14'x */       Invalid,
   /* '15'x */       Invalid,
   /* '16'x */       Invalid,
   /* '17'x */       Invalid,
   /* '18'x */       Invalid,
   /* '19'x */       Invalid,
   /* '1a'x */       Invalid,
   /* '1b'x */       Invalid,
   /* '1c'x */       Invalid,
   /* '1d'x */       Invalid,
   /* '1e'x */       Invalid,
   /* '1f'x */       Invalid,
/*$PE*/
   /*[space]*/       White  ,
   /*  !    */       Name   +INSYM + STARTSYM,
   /*  "    */       String ,
   /*  #    */       Invalid,
   /*  $    */       Invalid,
   /*  %    */       Oper   ,
   /*  &    */       Oper   ,
   /*  '    */       String ,
   /*  (    */       Punct  ,
   /*  )    */       Punct  ,
   /*  *    */       Oper   ,
   /*  +    */       Oper   ,
   /*  ,    */       Punct  ,
   /*  -    */       Oper   ,
   /*  .    */       Dot    +INSYM + INNUM ,
   /*  /    */       White  , /* Must be White for comment processing*/
   /*  0    */       Number +INNUM + INSYM,
   /*  1    */       Number +INNUM + INSYM,
   /*  2    */       Number +INNUM + INSYM,
   /*  3    */       Number +INNUM + INSYM,
   /*  4    */       Number +INNUM + INSYM,
   /*  5    */       Number +INNUM + INSYM,
   /*  6    */       Number +INNUM + INSYM,
   /*  7    */       Number +INNUM + INSYM,
   /*  8    */       Number +INNUM + INSYM,
   /*  9    */       Number +INNUM + INSYM,
   /*  :    */       Punct  ,
   /*  ;    */       Punct  ,
   /*  <    */       Oper   ,
   /*  =    */       Oper   ,
   /*  >    */       Oper   ,
   /*  ?    */       Name   +INSYM + STARTSYM,
   /*  @    */       Invalid,
/*$PE*/
   /*  A    */       Name   +INSYM + STARTSYM,
   /*  B    */       Name   +INSYM + STARTSYM,
   /*  C    */       Name   +INSYM + STARTSYM,
   /*  D    */       Name   +INSYM + STARTSYM,
   /*  E    */       Name   +INSYM + STARTSYM + INNUM,
   /*  F    */       Name   +INSYM + STARTSYM,
   /*  G    */       Name   +INSYM + STARTSYM,
   /*  H    */       Name   +INSYM + STARTSYM,
   /*  I    */       Name   +INSYM + STARTSYM,
   /*  J    */       Name   +INSYM + STARTSYM,
   /*  K    */       Name   +INSYM + STARTSYM,
   /*  L    */       Name   +INSYM + STARTSYM,
   /*  M    */       Name   +INSYM + STARTSYM,
   /*  N    */       Name   +INSYM + STARTSYM,
   /*  O    */       Name   +INSYM + STARTSYM,
   /*  P    */       Name   +INSYM + STARTSYM,
   /*  Q    */       Name   +INSYM + STARTSYM,
   /*  R    */       Name   +INSYM + STARTSYM,
   /*  S    */       Name   +INSYM + STARTSYM,
   /*  T    */       Name   +INSYM + STARTSYM,
   /*  U    */       Name   +INSYM + STARTSYM,
   /*  V    */       Name   +INSYM + STARTSYM,
   /*  W    */       Name   +INSYM + STARTSYM,
   /*  X    */       Name   +INSYM + STARTSYM,
   /*  Y    */       Name   +INSYM + STARTSYM,
   /*  Z    */       Name   +INSYM + STARTSYM,
   /*  [    */       Invalid ,
   /*  \    */       Oper   ,
   /*  ]    */       Invalid ,
   /*  ^    */       Invalid,
   /*  _    */       Name   +INSYM + STARTSYM,
   /*  `    */       Invalid,
/*$PE*/
   /*  a    */       Name   +INSYM + STARTSYM,
   /*  b    */       Name   +INSYM + STARTSYM,
   /*  c    */       Name   +INSYM + STARTSYM,
   /*  d    */       Name   +INSYM + STARTSYM,
   /*  e    */       Name   +INSYM + STARTSYM + INNUM,
   /*  f    */       Name   +INSYM + STARTSYM,
   /*  g    */       Name   +INSYM + STARTSYM,
   /*  h    */       Name   +INSYM + STARTSYM,
   /*  i    */       Name   +INSYM + STARTSYM,
   /*  j    */       Name   +INSYM + STARTSYM,
   /*  k    */       Name   +INSYM + STARTSYM,
   /*  l    */       Name   +INSYM + STARTSYM,
   /*  m    */       Name   +INSYM + STARTSYM,
   /*  n    */       Name   +INSYM + STARTSYM,
   /*  o    */       Name   +INSYM + STARTSYM,
   /*  p    */       Name   +INSYM + STARTSYM,
   /*  q    */       Name   +INSYM + STARTSYM,
   /*  r    */       Name   +INSYM + STARTSYM,
   /*  s    */       Name   +INSYM + STARTSYM,
   /*  t    */       Name   +INSYM + STARTSYM,
   /*  u    */       Name   +INSYM + STARTSYM,
   /*  v    */       Name   +INSYM + STARTSYM,
   /*  w    */       Name   +INSYM + STARTSYM,
   /*  x    */       Name   +INSYM + STARTSYM,
   /*  y    */       Name   +INSYM + STARTSYM,
   /*  z    */       Name   +INSYM + STARTSYM,
   /*  {    */       Invalid,
   /*  |    */       Oper   ,
   /*  }    */       Invalid,
   /*  ~ 126*/       Invalid,
/*$PE*/
   /* '7f'x */       Invalid,
   /* '80'x */       Invalid,
   /* '81'x */       Invalid,
   /* '82'x */       Invalid,
   /* '83'x */       Invalid,
   /* '84'x */       Invalid,
   /* '85'x */       Invalid,
   /* '86'x */       Invalid,
   /* '87'x */       Invalid,
   /* '88'x */       Invalid,
   /* '89'x */       Invalid,
   /* '8a'x */       Invalid,
   /* '8b'x */       Invalid,
   /* '8c'x */       Invalid,
   /* '8d'x */       Invalid,
   /* '8e'x */       Invalid,
   /* '8f'x */       Invalid,
   /* '90'x */       Invalid,
   /* '91'x */       Invalid,
   /* '92'x */       Invalid,
   /* '93'x */       Invalid,
   /* '94'x */       Invalid,
   /* '95'x */       Invalid,
   /* '96'x */       Invalid,
   /* '97'x */       Invalid,
   /* '98'x */       Invalid,
   /* '99'x */       Invalid,
   /* '9a'x */       Invalid,
   /* '9b'x */       Invalid,
   /* '9c'x */       Invalid,
   /* '9d'x */       Invalid,
   /* '9e'x */       Invalid,
   /* '9f'x */       Invalid,
   /* 'a0'x */       Invalid,
   /* 'a1'x */       Invalid,
   /* 'a2'x */       Invalid,
   /* 'a3'x */       Invalid,
   /* 'a4'x */       Invalid,
   /* 'a5'x */       Invalid,
   /* 'a6'x */       Invalid,
   /* 'a7'x */       Invalid,
   /* 'a8'x */       Invalid,
   /* 'a9'x */       Invalid,
   /* 'aa'x */       Oper ,         /* the ibm not character */
   /* 'ab'x */       Invalid,
   /* 'ac'x */       Invalid,
   /* 'ad'x */       Invalid,
   /* 'ae'x */       Invalid,
   /* 'af'x */       Invalid,
   /* 'b0'x */       Invalid,
   /* 'b1'x */       Invalid,
   /* 'b2'x */       Invalid,
   /* 'b3'x */       Invalid,
   /* 'b4'x */       Invalid,
   /* 'b5'x */       Invalid,
   /* 'b6'x */       Invalid,
   /* 'b7'x */       Invalid,
   /* 'b8'x */       Invalid,
   /* 'b9'x */       Invalid,
   /* 'ba'x */       Invalid,
   /* 'bb'x */       Invalid,
   /* 'bc'x */       Invalid,
   /* 'bd'x */       Invalid,
   /* 'be'x */       Invalid,
   /* 'bf'x */       Invalid,
   /* 'c0'x */       Invalid,
   /* 'c1'x */       Invalid,
   /* 'c2'x */       Invalid,
   /* 'c3'x */       Invalid,
   /* 'c4'x */       Invalid,
   /* 'c5'x */       Invalid,
   /* 'c6'x */       Invalid,
   /* 'c7'x */       Invalid,
   /* 'c8'x */       Invalid,
   /* 'c9'x */       Invalid,
   /* 'ca'x */       Invalid,
   /* 'cb'x */       Invalid,
   /* 'cc'x */       Invalid,
   /* 'cd'x */       Invalid,
   /* 'ce'x */       Invalid,
   /* 'cf'x */       Invalid,
   /* 'd0'x */       Invalid,
   /* 'd1'x */       Invalid,
   /* 'd2'x */       Invalid,
   /* 'd3'x */       Invalid,
   /* 'd4'x */       Invalid,
   /* 'd5'x */       Invalid,
   /* 'd6'x */       Invalid,
   /* 'd7'x */       Invalid,
   /* 'd8'x */       Invalid,
   /* 'd9'x */       Invalid,
   /* 'da'x */       Invalid,
   /* 'db'x */       Invalid,
   /* 'dc'x */       Invalid,
   /* 'dd'x */       Invalid,
   /* 'de'x */       Invalid,
   /* 'df'x */       Invalid,
   /* 'e0'x */       Invalid,
   /* 'e1'x */       Invalid,
   /* 'e2'x */       Invalid,
   /* 'e3'x */       Invalid,
   /* 'e4'x */       Invalid,
   /* 'e5'x */       Invalid,
   /* 'e6'x */       Invalid,
   /* 'e7'x */       Invalid,
   /* 'e8'x */       Invalid,
   /* 'e9'x */       Invalid,
   /* 'ea'x */       Invalid,
   /* 'eb'x */       Invalid,
   /* 'ec'x */       Invalid,
   /* 'ed'x */       Invalid,
   /* 'ee'x */       Invalid,
   /* 'ef'x */       Invalid,
   /* 'f0'x */       Invalid,
   /* 'f1'x */       Invalid,
   /* 'f2'x */       Invalid,
   /* 'f3'x */       Invalid,
   /* 'f4'x */       Invalid,
   /* 'f5'x */       Invalid,
   /* 'f6'x */       Invalid,
   /* 'f7'x */       Invalid,
   /* 'f8'x */       Invalid,
   /* 'f9'x */       Invalid,
   /* 'fa'x */       Invalid,
   /* 'fb'x */       Invalid,
   /* 'fc'x */       Invalid,
   /* 'fd'x */       Invalid,
   /* 'fe'x */       Invalid,
   /* 'ff'x */       Invalid
} ;


UCHAR NEAR upper_case_table[] = {
   /* '00'x */      '\x00',
   /* '01'x */      '\x01',
   /* '02'x */      '\x02',
   /* '03'x */      '\x03',
   /* '04'x */      '\x04',
   /* '05'x */      '\x05',
   /* '06'x */      '\x06',
   /* '07'x */      '\x07',
   /* '08'x */      '\x08',
   /* '09'x */      '\x09',
   /* '0a'x */      '\x0a',
   /* '0b'x */      '\x0b',
   /* '0c'x */      '\x0c',
   /* '0d'x */      '\x0d',
   /* '0e'x */      '\x0e',
   /* '0f'x */      '\x0f',
   /* '10'x */      '\x10',
   /* '11'x */      '\x11',
   /* '12'x */      '\x12',
   /* '13'x */      '\x13',
   /* '14'x */      '\x14',
   /* '15'x */      '\x15',
   /* '16'x */      '\x16',
   /* '17'x */      '\x17',
   /* '18'x */      '\x18',
   /* '19'x */      '\x19',
   /* '1a'x */      '\x1a',
   /* '1b'x */      '\x1b',
   /* '1c'x */      '\x1c',
   /* '1d'x */      '\x1d',
   /* '1e'x */      '\x1e',
   /* '1f'x */      '\x1f',
   /* '20'x */      '\x20',
   /* '21'x */      '\x21',      /* ! */
   /* '22'x */      '\x22',      /* " */
   /* '23'x */      '\x23',      /* # */
   /* '24'x */      '\x24',      /* $ */
   /* '25'x */      '\x25',      /* % */
   /* '26'x */      '\x26',      /* & */
   /* '27'x */      '\x27',      /* ' */
   /* '28'x */      '\x28',      /* ( */
   /* '29'x */      '\x29',      /* ) */
   /* '2a'x */      '\x2a',      /* * */
   /* '2b'x */      '\x2b',      /* + */
   /* '2c'x */      '\x2c',      /* , */
   /* '2d'x */      '\x2d',      /* - */
   /* '2e'x */      '\x2e',      /* . */
   /* '2f'x */      '\x2f',      /* / */
   /* '30'x */      '\x30',      /* 0 */
   /* '31'x */      '\x31',      /* 1 */
   /* '32'x */      '\x32',      /* 2 */
   /* '33'x */      '\x33',      /* 3 */
   /* '34'x */      '\x34',      /* 4 */
   /* '35'x */      '\x35',      /* 5 */
   /* '36'x */      '\x36',      /* 6 */
   /* '37'x */      '\x37',      /* 7 */
   /* '38'x */      '\x38',      /* 8 */
   /* '39'x */      '\x39',      /* 9 */
   /* '3a'x */      '\x3a',      /* : */
   /* '3b'x */      '\x3b',      /* ; */
   /* '3c'x */      '\x3c',      /* < */
   /* '3d'x */      '\x3d',      /* = */
   /* '3e'x */      '\x3e',      /* > */
   /* '3f'x */      '\x3f',      /* ? */
   /* '40'x */      '\x40',      /* @ */
   /* '41'x */      '\x41',      /* A */
   /* '42'x */      '\x42',      /* B */
   /* '43'x */      '\x43',      /* C */
   /* '44'x */      '\x44',      /* D */
   /* '45'x */      '\x45',      /* E */
   /* '46'x */      '\x46',      /* F */
   /* '47'x */      '\x47',      /* G */
   /* '48'x */      '\x48',      /* H */
   /* '49'x */      '\x49',      /* I */
   /* '4a'x */      '\x4a',      /* J */
   /* '4b'x */      '\x4b',      /* K */
   /* '4c'x */      '\x4c',      /* L */
   /* '4d'x */      '\x4d',      /* M */
   /* '4e'x */      '\x4e',      /* N */
   /* '4f'x */      '\x4f',      /* O */
   /* '50'x */      '\x50',      /* P */
   /* '51'x */      '\x51',      /* Q */
   /* '52'x */      '\x52',      /* R */
   /* '53'x */      '\x53',      /* S */
   /* '54'x */      '\x54',      /* T */
   /* '55'x */      '\x55',      /* U */
   /* '56'x */      '\x56',      /* V */
   /* '57'x */      '\x57',      /* W */
   /* '58'x */      '\x58',      /* X */
   /* '59'x */      '\x59',      /* Y */
   /* '5a'x */      '\x5a',      /* Z */
   /* '5b'x */      '\x5b',      /* [ */
   /* '5c'x */      '\x5c',      /* \ */
   /* '5d'x */      '\x5d',      /* ] */
   /* '5e'x */      '\x5e',      /* ^ */
   /* '5f'x */      '\x5f',      /* _ */
   /* '60'x */      '\x60',      /* ` */
   /* '61'x */      '\x41',      /* convert a to A */
   /* '62'x */      '\x42',      /* convert b to B */
   /* '63'x */      '\x43',      /* convert c to C */
   /* '64'x */      '\x44',      /* convert d to D */
   /* '65'x */      '\x45',      /* convert e to E */
   /* '66'x */      '\x46',      /* convert f to F */
   /* '67'x */      '\x47',      /* convert g to G */
   /* '68'x */      '\x48',      /* convert h to H */
   /* '69'x */      '\x49',      /* convert i to I */
   /* '6a'x */      '\x4a',      /* convert j to J */
   /* '6b'x */      '\x4b',      /* convert k to K */
   /* '6c'x */      '\x4c',      /* convert l to L */
   /* '6d'x */      '\x4d',      /* convert m to M */
   /* '6e'x */      '\x4e',      /* convert n to N */
   /* '6f'x */      '\x4f',      /* convert o to O */
   /* '70'x */      '\x50',      /* convert p to P */
   /* '71'x */      '\x51',      /* convert q to Q */
   /* '72'x */      '\x52',      /* convert r to R */
   /* '73'x */      '\x53',      /* convert s to S */
   /* '74'x */      '\x54',      /* convert t to T */
   /* '75'x */      '\x55',      /* convert u to U */
   /* '76'x */      '\x56',      /* convert v to V */
   /* '77'x */      '\x57',      /* convert w to W */
   /* '78'x */      '\x58',      /* convert x to X */
   /* '79'x */      '\x59',      /* convert y to Y */
   /* '7a'x */      '\x5a',      /* convert z to Z */
   /* '7b'x */      '\x7b',      /* { */
   /* '7c'x */      '\x7c',      /* | */
   /* '7d'x */      '\x7d',      /* } */
   /* '7e'x */      '\x7e',      /* ~ */
   /* '7f'x */      '\x7f',
   /* '80'x */      '\x80',
   /* '81'x */      '\x81',
   /* '82'x */      '\x82',
   /* '83'x */      '\x83',
   /* '84'x */      '\x84',
   /* '85'x */      '\x85',
   /* '86'x */      '\x86',
   /* '87'x */      '\x87',
   /* '88'x */      '\x88',
   /* '89'x */      '\x89',
   /* '8a'x */      '\x8a',
   /* '8b'x */      '\x8b',
   /* '8c'x */      '\x8c',
   /* '8d'x */      '\x8d',
   /* '8e'x */      '\x8e',
   /* '8f'x */      '\x8f',
   /* '90'x */      '\x90',
   /* '91'x */      '\x91',
   /* '92'x */      '\x92',
   /* '93'x */      '\x93',
   /* '94'x */      '\x94',
   /* '95'x */      '\x95',
   /* '96'x */      '\x96',
   /* '97'x */      '\x97',
   /* '98'x */      '\x98',
   /* '99'x */      '\x99',
   /* '9a'x */      '\x9a',
   /* '9b'x */      '\x9b',
   /* '9c'x */      '\x9c',
   /* '9d'x */      '\x9d',
   /* '9e'x */      '\x9e',
   /* '9f'x */      '\x9f',
   /* 'a0'x */      '\xa0',
   /* 'a1'x */      '\xa1',
   /* 'a2'x */      '\xa2',
   /* 'a3'x */      '\xa3',
   /* 'a4'x */      '\xa4',
   /* 'a5'x */      '\xa5',
   /* 'a6'x */      '\xa6',
   /* 'a7'x */      '\xa7',
   /* 'a8'x */      '\xa8',
   /* 'a9'x */      '\xa9',
   /* 'aa'x */      '\xaa',
   /* 'ab'x */      '\xab',
   /* 'ac'x */      '\xac',
   /* 'ad'x */      '\xad',
   /* 'ae'x */      '\xae',
   /* 'af'x */      '\xaf',
   /* 'b0'x */      '\xb0',
   /* 'b1'x */      '\xb1',
   /* 'b2'x */      '\xb2',
   /* 'b3'x */      '\xb3',
   /* 'b4'x */      '\xb4',
   /* 'b5'x */      '\xb5',
   /* 'b6'x */      '\xb6',
   /* 'b7'x */      '\xb7',
   /* 'b8'x */      '\xb8',
   /* 'b9'x */      '\xb9',
   /* 'ba'x */      '\xba',
   /* 'bb'x */      '\xbb',
   /* 'bc'x */      '\xbc',
   /* 'bd'x */      '\xbd',
   /* 'be'x */      '\xbe',
   /* 'bf'x */      '\xbf',
   /* 'c0'x */      '\xc0',
   /* 'c1'x */      '\xc1',
   /* 'c2'x */      '\xc2',
   /* 'c3'x */      '\xc3',
   /* 'c4'x */      '\xc4',
   /* 'c5'x */      '\xc5',
   /* 'c6'x */      '\xc6',
   /* 'c7'x */      '\xc7',
   /* 'c8'x */      '\xc8',
   /* 'c9'x */      '\xc9',
   /* 'ca'x */      '\xca',
   /* 'cb'x */      '\xcb',
   /* 'cc'x */      '\xcc',
   /* 'cd'x */      '\xcd',
   /* 'ce'x */      '\xce',
   /* 'cf'x */      '\xcf',
   /* 'd0'x */      '\xd0',
   /* 'd1'x */      '\xd1',
   /* 'd2'x */      '\xd2',
   /* 'd3'x */      '\xd3',
   /* 'd4'x */      '\xd4',
   /* 'd5'x */      '\xd5',
   /* 'd6'x */      '\xd6',
   /* 'd7'x */      '\xd7',
   /* 'd8'x */      '\xd8',
   /* 'd9'x */      '\xd9',
   /* 'da'x */      '\xda',
   /* 'db'x */      '\xdb',
   /* 'dc'x */      '\xdc',
   /* 'dd'x */      '\xdd',
   /* 'de'x */      '\xde',
   /* 'df'x */      '\xdf',
   /* 'e0'x */      '\xe0',
   /* 'e1'x */      '\xe1',
   /* 'e2'x */      '\xe2',
   /* 'e3'x */      '\xe3',
   /* 'e4'x */      '\xe4',
   /* 'e5'x */      '\xe5',
   /* 'e6'x */      '\xe6',
   /* 'e7'x */      '\xe7',
   /* 'e8'x */      '\xe8',
   /* 'e9'x */      '\xe9',
   /* 'ea'x */      '\xea',
   /* 'eb'x */      '\xeb',
   /* 'ec'x */      '\xec',
   /* 'ed'x */      '\xed',
   /* 'ee'x */      '\xee',
   /* 'ef'x */      '\xef',
   /* 'f0'x */      '\xf0',
   /* 'f1'x */      '\xf1',
   /* 'f2'x */      '\xf2',
   /* 'f3'x */      '\xf3',
   /* 'f4'x */      '\xf4',
   /* 'f5'x */      '\xf5',
   /* 'f6'x */      '\xf6',
   /* 'f7'x */      '\xf7',
   /* 'f8'x */      '\xf8',
   /* 'f9'x */      '\xf9',
   /* 'fa'x */      '\xfa',
   /* 'fb'x */      '\xfb',
   /* 'fc'x */      '\xfc',
   /* 'fd'x */      '\xfd',
   /* 'fe'x */      '\xfe',
   /* 'ff'x */      '\xff'
};


UCHAR NEAR lower_case_table[] = {
   /* '00'x */      '\x00',
   /* '01'x */      '\x01',
   /* '02'x */      '\x02',
   /* '03'x */      '\x03',
   /* '04'x */      '\x04',
   /* '05'x */      '\x05',
   /* '06'x */      '\x06',
   /* '07'x */      '\x07',
   /* '08'x */      '\x08',
   /* '09'x */      '\x09',
   /* '0a'x */      '\x0a',
   /* '0b'x */      '\x0b',
   /* '0c'x */      '\x0c',
   /* '0d'x */      '\x0d',
   /* '0e'x */      '\x0e',
   /* '0f'x */      '\x0f',
   /* '10'x */      '\x10',
   /* '11'x */      '\x11',
   /* '12'x */      '\x12',
   /* '13'x */      '\x13',
   /* '14'x */      '\x14',
   /* '15'x */      '\x15',
   /* '16'x */      '\x16',
   /* '17'x */      '\x17',
   /* '18'x */      '\x18',
   /* '19'x */      '\x19',
   /* '1a'x */      '\x1a',
   /* '1b'x */      '\x1b',
   /* '1c'x */      '\x1c',
   /* '1d'x */      '\x1d',
   /* '1e'x */      '\x1e',
   /* '1f'x */      '\x1f',
   /* '20'x */      '\x20',
   /* '21'x */      '\x21',      /* ! */
   /* '22'x */      '\x22',      /* " */
   /* '23'x */      '\x23',      /* # */
   /* '24'x */      '\x24',      /* $ */
   /* '25'x */      '\x25',      /* % */
   /* '26'x */      '\x26',      /* & */
   /* '27'x */      '\x27',      /* ' */
   /* '28'x */      '\x28',      /* ( */
   /* '29'x */      '\x29',      /* ) */
   /* '2a'x */      '\x2a',      /* * */
   /* '2b'x */      '\x2b',      /* + */
   /* '2c'x */      '\x2c',      /* , */
   /* '2d'x */      '\x2d',      /* - */
   /* '2e'x */      '\x2e',      /* . */
   /* '2f'x */      '\x2f',      /* / */
   /* '30'x */      '\x30',      /* 0 */
   /* '31'x */      '\x31',      /* 1 */
   /* '32'x */      '\x32',      /* 2 */
   /* '33'x */      '\x33',      /* 3 */
   /* '34'x */      '\x34',      /* 4 */
   /* '35'x */      '\x35',      /* 5 */
   /* '36'x */      '\x36',      /* 6 */
   /* '37'x */      '\x37',      /* 7 */
   /* '38'x */      '\x38',      /* 8 */
   /* '39'x */      '\x39',      /* 9 */
   /* '3a'x */      '\x3a',      /* : */
   /* '3b'x */      '\x3b',      /* ; */
   /* '3c'x */      '\x3c',      /* < */
   /* '3d'x */      '\x3d',      /* = */
   /* '3e'x */      '\x3e',      /* > */
   /* '3f'x */      '\x3f',      /* ? */
   /* '40'x */      '\x40',      /* @ */
   /* '41'x */      '\x61',      /* convert A to a */
   /* '42'x */      '\x62',      /* convert B to b */
   /* '43'x */      '\x63',      /* convert C to c */
   /* '44'x */      '\x64',      /* convert D to d */
   /* '45'x */      '\x65',      /* convert E to e */
   /* '46'x */      '\x66',      /* convert F to f */
   /* '47'x */      '\x67',      /* convert G to g */
   /* '48'x */      '\x68',      /* convert H to h */
   /* '49'x */      '\x69',      /* convert I to i */
   /* '4a'x */      '\x6a',      /* convert J to j */
   /* '4b'x */      '\x6b',      /* convert K to k */
   /* '4c'x */      '\x6c',      /* convert L to l */
   /* '4d'x */      '\x6d',      /* convert M to m */
   /* '4e'x */      '\x6e',      /* convert N to n */
   /* '4f'x */      '\x6f',      /* convert O to o */
   /* '50'x */      '\x70',      /* convert P to p */
   /* '51'x */      '\x71',      /* convert Q to q */
   /* '52'x */      '\x72',      /* convert R to r */
   /* '53'x */      '\x73',      /* convert S to s */
   /* '54'x */      '\x74',      /* convert T to t */
   /* '55'x */      '\x75',      /* convert U to u */
   /* '56'x */      '\x76',      /* convert V to v */
   /* '57'x */      '\x77',      /* convert W to w */
   /* '58'x */      '\x78',      /* convert X to x */
   /* '59'x */      '\x79',      /* convert Y to y */
   /* '5a'x */      '\x7a',      /* convert Z to z */
   /* '5b'x */      '\x5b',      /* [ */
   /* '5c'x */      '\x5c',      /* \ */
   /* '5d'x */      '\x5d',      /* ] */
   /* '5e'x */      '\x5e',      /* ^ */
   /* '5f'x */      '\x5f',      /* _ */
   /* '60'x */      '\x60',      /* ` */
   /* '61'x */      '\x61',      /* a */
   /* '62'x */      '\x62',      /* b */
   /* '63'x */      '\x63',      /* c */
   /* '64'x */      '\x64',      /* d */
   /* '65'x */      '\x65',      /* e */
   /* '66'x */      '\x66',      /* f */
   /* '67'x */      '\x67',      /* g */
   /* '68'x */      '\x68',      /* h */
   /* '69'x */      '\x69',      /* i */
   /* '6a'x */      '\x6a',      /* j */
   /* '6b'x */      '\x6b',      /* k */
   /* '6c'x */      '\x6c',      /* l */
   /* '6d'x */      '\x6d',      /* m */
   /* '6e'x */      '\x6e',      /* n */
   /* '6f'x */      '\x6f',      /* o */
   /* '70'x */      '\x70',      /* p */
   /* '71'x */      '\x71',      /* q */
   /* '72'x */      '\x72',      /* r */
   /* '73'x */      '\x73',      /* s */
   /* '74'x */      '\x74',      /* t */
   /* '75'x */      '\x75',      /* u */
   /* '76'x */      '\x76',      /* v */
   /* '77'x */      '\x77',      /* w */
   /* '78'x */      '\x78',      /* x */
   /* '79'x */      '\x79',      /* y */
   /* '7a'x */      '\x7a',      /* z */
   /* '7b'x */      '\x7b',      /* { */
   /* '7c'x */      '\x7c',      /* | */
   /* '7d'x */      '\x7d',      /* } */
   /* '7e'x */      '\x7e',      /* ~ */
   /* '7f'x */      '\x7f',
   /* '80'x */      '\x80',
   /* '81'x */      '\x81',
   /* '82'x */      '\x82',
   /* '83'x */      '\x83',
   /* '84'x */      '\x84',
   /* '85'x */      '\x85',
   /* '86'x */      '\x86',
   /* '87'x */      '\x87',
   /* '88'x */      '\x88',
   /* '89'x */      '\x89',
   /* '8a'x */      '\x8a',
   /* '8b'x */      '\x8b',
   /* '8c'x */      '\x8c',
   /* '8d'x */      '\x8d',
   /* '8e'x */      '\x8e',
   /* '8f'x */      '\x8f',
   /* '90'x */      '\x90',
   /* '91'x */      '\x91',
   /* '92'x */      '\x92',
   /* '93'x */      '\x93',
   /* '94'x */      '\x94',
   /* '95'x */      '\x95',
   /* '96'x */      '\x96',
   /* '97'x */      '\x97',
   /* '98'x */      '\x98',
   /* '99'x */      '\x99',
   /* '9a'x */      '\x9a',
   /* '9b'x */      '\x9b',
   /* '9c'x */      '\x9c',
   /* '9d'x */      '\x9d',
   /* '9e'x */      '\x9e',
   /* '9f'x */      '\x9f',
   /* 'a0'x */      '\xa0',
   /* 'a1'x */      '\xa1',
   /* 'a2'x */      '\xa2',
   /* 'a3'x */      '\xa3',
   /* 'a4'x */      '\xa4',
   /* 'a5'x */      '\xa5',
   /* 'a6'x */      '\xa6',
   /* 'a7'x */      '\xa7',
   /* 'a8'x */      '\xa8',
   /* 'a9'x */      '\xa9',
   /* 'aa'x */      '\xaa',
   /* 'ab'x */      '\xab',
   /* 'ac'x */      '\xac',
   /* 'ad'x */      '\xad',
   /* 'ae'x */      '\xae',
   /* 'af'x */      '\xaf',
   /* 'b0'x */      '\xb0',
   /* 'b1'x */      '\xb1',
   /* 'b2'x */      '\xb2',
   /* 'b3'x */      '\xb3',
   /* 'b4'x */      '\xb4',
   /* 'b5'x */      '\xb5',
   /* 'b6'x */      '\xb6',
   /* 'b7'x */      '\xb7',
   /* 'b8'x */      '\xb8',
   /* 'b9'x */      '\xb9',
   /* 'ba'x */      '\xba',
   /* 'bb'x */      '\xbb',
   /* 'bc'x */      '\xbc',
   /* 'bd'x */      '\xbd',
   /* 'be'x */      '\xbe',
   /* 'bf'x */      '\xbf',
   /* 'c0'x */      '\xc0',
   /* 'c1'x */      '\xc1',
   /* 'c2'x */      '\xc2',
   /* 'c3'x */      '\xc3',
   /* 'c4'x */      '\xc4',
   /* 'c5'x */      '\xc5',
   /* 'c6'x */      '\xc6',
   /* 'c7'x */      '\xc7',
   /* 'c8'x */      '\xc8',
   /* 'c9'x */      '\xc9',
   /* 'ca'x */      '\xca',
   /* 'cb'x */      '\xcb',
   /* 'cc'x */      '\xcc',
   /* 'cd'x */      '\xcd',
   /* 'ce'x */      '\xce',
   /* 'cf'x */      '\xcf',
   /* 'd0'x */      '\xd0',
   /* 'd1'x */      '\xd1',
   /* 'd2'x */      '\xd2',
   /* 'd3'x */      '\xd3',
   /* 'd4'x */      '\xd4',
   /* 'd5'x */      '\xd5',
   /* 'd6'x */      '\xd6',
   /* 'd7'x */      '\xd7',
   /* 'd8'x */      '\xd8',
   /* 'd9'x */      '\xd9',
   /* 'da'x */      '\xda',
   /* 'db'x */      '\xdb',
   /* 'dc'x */      '\xdc',
   /* 'dd'x */      '\xdd',
   /* 'de'x */      '\xde',
   /* 'df'x */      '\xdf',
   /* 'e0'x */      '\xe0',
   /* 'e1'x */      '\xe1',
   /* 'e2'x */      '\xe2',
   /* 'e3'x */      '\xe3',
   /* 'e4'x */      '\xe4',
   /* 'e5'x */      '\xe5',
   /* 'e6'x */      '\xe6',
   /* 'e7'x */      '\xe7',
   /* 'e8'x */      '\xe8',
   /* 'e9'x */      '\xe9',
   /* 'ea'x */      '\xea',
   /* 'eb'x */      '\xeb',
   /* 'ec'x */      '\xec',
   /* 'ed'x */      '\xed',
   /* 'ee'x */      '\xee',
   /* 'ef'x */      '\xef',
   /* 'f0'x */      '\xf0',
   /* 'f1'x */      '\xf1',
   /* 'f2'x */      '\xf2',
   /* 'f3'x */      '\xf3',
   /* 'f4'x */      '\xf4',
   /* 'f5'x */      '\xf5',
   /* 'f6'x */      '\xf6',
   /* 'f7'x */      '\xf7',
   /* 'f8'x */      '\xf8',
   /* 'f9'x */      '\xf9',
   /* 'fa'x */      '\xfa',
   /* 'fb'x */      '\xfb',
   /* 'fc'x */      '\xfc',
   /* 'fd'x */      '\xfd',
   /* 'fe'x */      '\xfe',
   /* 'ff'x */      '\xff'
};
