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
/* REXX Kernel                                                  ASCIIDBCSStrings.hpp     */
/*                                                                            */
/* REXX string ASCII DBCS string definitions                                  */
/*                                                                            */
/******************************************************************************/
/*********************************************************************/
/*                                                                   */
/* Conditionally execute a DBCS version of a string method           */
/*                                                                   */
/*********************************************************************/
#ifndef OKADBCS_INCLUDED
#define OKADBCS_INCLUDED

#define DBCS_MODE  (current_settings->exmode && current_settings->DBCS_codepage)
#define DBCS_SELF  (!NoDBCS(this) && DBCS_MODE)

/*********************************************************************/
/*                                                                   */
/* get the length of a string object (in bytes)                      */
/*                                                                   */
/*********************************************************************/

#define STRLEN(x) ((x)->length)

/*********************************************************************/
/*                                                                   */
/* Validate a DBCS string                                            */
/*                                                                   */
/*********************************************************************/

#define ValidDBCS(x) ((x)->validDBCS())

/*********************************************************************/
/*                                                                   */
/* convert an object into an address/length argument pair            */
/*                                                                   */
/*********************************************************************/

#define STRDESC(x) STRPTR(x), STRLEN(x)

/*********************************************************************/
/*                                                                   */
/* point to the data part of a string                                */
/*                                                                   */
/*********************************************************************/

#define STRPTR(x) ((PUCHAR)(x)->stringData)

/*********************************************************************/
/*                                                                   */
/* check if a byte is a DBCS first character                         */
/*                                                                   */
/*********************************************************************/

#define IsDBCS(ch) (current_settings->DBCS_table[ch])

/*********************************************************************/
/*                                                                   */
/* check if a pointer points to a DBCS blank                         */
/*                                                                   */
/*********************************************************************/

#define IsDBCSBlank(p) (*p == DBCS_BLANK1 && *(p + 1) == DBCS_BLANK2)

/*      MIXED string status.                                                */

#define PURE_DBCS       'D'            /* For pure DBCS.                    */
#define PURE_SBCS       'S'            /* For pure SBCS.                    */
#define MIXED           'C'            /* For Mixed string.                 */
#define INV_MIXED       'I'            /* For Invalid mixed.                */

#define DBCS_YES        'Y'            /* Yes DBCS option                   */
#define DBCS_NO         'N'            /* No DBCS option                    */
#define DBCS_COUNT      'C'            /* Count SO/SI option                */
#define DBCS_BLANK      'B'            /* B option for DBADJUST             */
#define DBCS_REMOVE     'R'            /* R option for DBADJUST             */


/*      Mixed string Byte type status.                                      */

#define SBCS            0L             /* byte type is SBCS.                */
#define DBCS            1L             /* byte type is SBCS.                */
#define DBCS_1ST        1L             /* byte type is DBCS 1st byte.       */
#define DBCS_2ND        2L             /* byte type is DBCS 2nd byte.       */

/*      Constant values byte length of one SBCS/DBCS character.             */

#define DBCS_BYTELEN    2L             /* 2 Bytes.                          */
#define SBCS_BYTELEN    1L             /* 1 Byte.                           */


/*      Constant values for DBCS string comparison.                         */

#define COMP_NULL       'N'            /* Null of comparison.               */
#define COMP_EQUAL      0              /* Equal.                            */
#define COMP_GREATER    1              /* 1st string is greater.            */
#define COMP_LESS       (-1)           /* 1st string is less.               */

/*      Blank of DBCS and SBCS.                                             */

#define DBCS_BLANK1     0x81           /* 1st. byte.                        */
#define DBCS_BLANK2     0x40           /* 2nd. byte.                        */
#define SBCS_BLANK      0x20           /* SBCS blank.                       */

/*      Use in DBCS, SBCS table.                                            */

#define C_INV   0x0000                 /* invalid DBCS/SBCS table.          */
#define C_UNDF  0x0000                 /* undef DBCS or SBCS table.         */
#define C_DBCS  0x0000                 /* DBCS character.                   */


/*      DBCS Country code page.                                             */

#define CP_JAPAN        932            /* Japan.                            */
#define CP_KOREA        934            /* Korean.                           */
#define CP_PRC          936            /* Peoples Republic of China.        */
#define CP_ROC          938            /* Taiwan.                           */
#define CP_JAPAN_942    942            /* CodePage for Japan.               */
#define CP_KOREA_944    944            /* CodePage for KOREA.               */
#define CP_PRC_946      946            /* CodePage for PRC.                 */
#define CP_ROC_948      948            /* CodePage for ROC.                 */


/*                                                                          */
/*      The following constants are used to determine increment or          */
/*      decrement passing to DBASC_PosLChar.                                */
/*                                                                          */
/*                                                                          */

#define INCREMENT       1L             /* Increment.                        */
#define DECREMENT       (-1L)          /* Decrement.                        */

void DBCS_MemUpper(PUCHAR, size_t);
void DBCS_MemLower(PUCHAR, size_t);
void DBCS_SkipNonBlanks(PUCHAR *, size_t *);
void DBCS_SkipBlanks(PUCHAR *, size_t *);
size_t DBCS_CharacterCount(PUCHAR, size_t);

#endif
