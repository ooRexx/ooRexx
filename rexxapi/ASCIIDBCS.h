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
/* Module Name: DBASCII1 H                                           */
/*                                                                   */
/* Description: This file REXX OS/2 DBCS Support module Defeind.     */
/*                                                                   */
/*********************************************************************/

#ifndef DBASCII1_H_INCLUDED
#define DBASCII1_H_INCLUDED

/***
 *      Max digits.
 */
#define MAX_DIGITS      10L            /* Max digits number for long.*/


/***                                                                 */
/*      MIXED string status.                                         */
/*                                                                   */

#define PURE_DBCS       'D'            /* For pure DBCS.             */
#define PURE_SBCS       'S'            /* For pure SBCS.             */
#define MIXED           'C'            /* For Mixed string.          */
#define INV_MIXED       'I'            /* For Invalid mixed.         */


/***                                                                 */
/*      Mixed string Byte type status.                               */
/*                                                                   */

#define SBCS            0L             /* byte type is SBCS.         */
#define DBCS            1L             /* byte type is SBCS.         */
#define DBCS_1ST        1L             /* byte type is DBCS 1st byte.*/
#define DBCS_2ND        2L             /* byte type is DBCS 2nd byte.*/


/***                                                                 */
/*      Constant values for MIXED_STRING type.                       */
/*                                                                   */

#define MIXED_TYPE_D    'D'            /* MIXED_STRING is pure DBCS. */
#define MIXED_TYPE_C    'C'            /* MIXED_STRING DBCS/SBCS     */
#define MIXED_TYPE_S    'S'            /* MIXED_STRING is pure SBCS. */


/***                                                                 */
/*      Constant values byte length of one SBCS/DBCS character.      */
/*                                                                   */

#define DBCS_BYTELEN    2L             /* 2 Bytes.                   */
#define SBCS_BYTELEN    1L             /* 1 Byte.                    */


/***                                                                 */
/*      Constant values for DBCS string comparison.                  */
/*                                                                   */

#define COMP_NULL       'N'            /* Null of comparison.        */
#define COMP_EQUAL      0              /* Equal.                     */
#define COMP_GREATER    1              /* 1st string is greater.     */
#define COMP_LESS       (-1)           /* 1st string is less.        */


/***                                                                 */
/*      Constant values for flag ON or OFF.                          */
/*                                                                   */

#define ON      1                      /* flag status is ON.         */
#define OFF     0                      /* flag status is OFF.        */


/***                                                                 */
/*      Constant values for extract option.                          */
/*                                                                   */

#define EXT_LEFT        'L'            /* Extract from left edge.    */
#define EXT_RIGHT       'R'            /* Extract from right edge.   */


/***                                                                 */
/*      Blank of DBCS and SBCS.                                      */
/*                                                                   */

#define DBCS_BLANK1     0x81           /* 1st. byte.                 */
#define DBCS_BLANK2     0x40           /* 2nd. byte.                 */
#define SBCS_BLANK      0x20           /* SBCS blank.                */


/***                                                                 */
/*      Parametr omission indicator                                  */
/*      when any argument is omitted, the negative value set.        */
/*                                                                   */

#define OMISSION        (-1L)          /* Omission indicator.        */


/***                                                                 */
/*      Options for STRIP VERIFY DATATYPE.                           */
/*                                                                   */

#define STRIP_L         'L'            /* Strip option for Leading.  */
#define STRIP_T         'T'            /* Strip option for Trailing. */
#define STRIP_B         'B'            /* Strip option for Both.     */
#define VERIFY_M        'M'            /* Verify option for Match.   */
#define VERIFY_N        'N'            /* Verify option for Nomatch. */
#define DATA_TYPE_D     'D'            /* Datatype D for pure DBCS.  */
#define DATA_TYPE_C     'C'            /* Datatype C for mixed.      */


/***    Function return code.                                        */
/*                                                                   */
/*      Return code                                                  */
/*      Note : This error code must have the same value as REXEQU    */
/*                                                                   */
/*                                                                   */

#define RC_NORMAL       0              /* Successfully complete.     */
#define RC_INVALID      23             /* Detect invalid mixed-string*/
#define RC_INCORRECT    40             /* Detect incorrect parameter.*/
#define RC_NOEVAL       5              /* Not acquired EVALBLOK.     */
#define RC_SYSTEMFAIL   48             /* System service failure.    */


/***    Mixed string validation result.                              */
/*                                                                   */
/*      DBASC_MixValid Return code.                                  */
/*                                                                   */
/*                                                                   */

#define VALID   1                      /* 1 for valid.               */
#define INVALID 0                      /* 0 for invalid.             */


/***                                                                 */
/*      Use in DBCS, SBCS table.                                     */
/*                                                                   */

#define C_INV   0x0000                 /* invalid DBCS/SBCS table.   */
#define C_UNDF  0x0000                 /* undef DBCS or SBCS table.  */
#define C_DBCS  0x0000                 /* DBCS character.            */


/***    DBCS Country code page.                                      */
/*                                                                   */
/*      The following constants are used in DBASC_ConvToDBCS and     */
/*      DBASC_ConvToSBCS routine to check the code page.             */
/*                                                                   */
/*                                                                   */

#define CP_JAPAN        932            /* Japan.                     */
#define CP_KOREA        934            /* Korean.                    */
#define CP_PRC          936            /* Peoples Republic of China. */
#define CP_ROC          938            /* Taiwan.                    */
#define CP_JAPAN_942    942            /* CodePage for Japan.        */
#define CP_KOREA_944    944            /* CodePage for KOREA.        */
#define CP_PRC_946      946            /* CodePage for PRC.          */
#define CP_ROC_948      948            /* CodePage for ROC.          */


/***    Use in DBASC_PosLChar.                                       */
/*                                                                   */
/*      The following constants are used to determine increment or   */
/*      decrement passing to DBASC_PosLChar.                         */
/*                                                                   */
/*                                                                   */

#define INCREMENT       1L             /* Increment.                 */
#define DECREMENT       (-1L)          /* Decrement.                 */


/***    MACRO Routine defined.                                       */
/*                                                                   */
/*                                                                   */
/*                                                                   */


/*********************************************************************/
/* Function: MAX MAX_3 MAX_4                                         */
/*                                                                   */
/* Descriptive name: Return arguments maxium number.                 */
/*                                                                   */
/* External References:                                              */
/*   Input:  Numric.                                                 */
/*   Output: None.                                                   */
/*   Effect: None.                                                   */
/*   Return: Maxium number.                                          */
/*                                                                   */
/*********************************************************************/
#define MAX(a,b)       ((a>b)?a:b)
#define MAX_3(a,b,c)   ((a>b)?MAX(a,c):MAX(b,c))
#define MAX_4(a,b,c,d) ((a>b)?MAX_3(a,c,d):MAX_3(b,c,d))


/*********************************************************************/
/* Function: MIN MIN_3 MIN_4                                         */
/*                                                                   */
/* Descriptive name: Return arguments minium number.                 */
/*                                                                   */
/* External References:                                              */
/*   Input:  Numric.                                                 */
/*   Output: None.                                                   */
/*   Effect: None.                                                   */
/*   Return: Minium number.                                          */
/*                                                                   */
/*********************************************************************/
#define MIN(a,b)       ((a<b)?a:b)
#define MIN_3(a,b,c)   ((a<b)?MIN(a,c):MIN(b,c))
#define MIN_4(a,b,c,d) ((a<b)?MIN_3(a,c,d):MIN_3(b,c,d))


/*********************************************************************/
/* Function: NUM_CHK                                                 */
/*                                                                   */
/* Descriptive name: Do number check of correct positive number.     */
/*                                                                   */
/* External References:                                              */
/*   Input:  Numric.                                                 */
/*   Output: None.                                                   */
/*   Effect: None.                                                   */
/*   Return: If incorrent positive number then do                    */
/*             'return RC_INCORRECT;'                                */
/*           else no operation.                                      */
/*                                                                   */
/*********************************************************************/
#define NUM_CHK(a) if (a<=0) return RC_INCORRECT;


/*********************************************************************/
/* Function: DBASC_CharConcat                                        */
/*                                                                   */
/* Descriptive name: This routine can be replaced by H_strconcat().  */
/*                                                                   */
/* External References:                                              */
/*   Input:  H_ptr - Concatenated string.                            */
/*           H_ptr - Concatenate string.                             */
/*   Output: None.                                                   */
/*   Effect: None.                                                   */
/*   Return: None.                                                   */
/*                                                                   */
/*********************************************************************/
#define DBASC_CharConcat(WorkBlock, Des,Src) \
                         H_strconcat(WorkBlock,Des,Src)


/*********************************************************************/
/* Function: DBASC_IncPerBlank                                       */
/*                                                                   */
/* Descriptive name: Skips DBCS and SBCS blank.                      */
/*                                                                   */
/* External References:                                              */
/*   Input:  H_ptr - String to be handled.                           */
/*   Output: H_ptr - Skiped string.                                  */
/*   Effect: Input H_ptr is output H_ptr.                            */
/*   Return: None.                                                   */
/*                                                                   */
/*********************************************************************/
#define DBASC_IncPerBlank(x)\
        if (DBASC_IsDBCS_Blank(&x)) {\
          H_INC_BY((x),DBCS_BYTELEN); }\
        else {\
          if(H_GET((x))==SBCS_BLANK) H_INC_BY((x),SBCS_BYTELEN);\
        }\

/*********************************************************************/
/* Function: DBASC_IncPerLogical                                     */
/*                                                                   */
/* Descriptive name: Increment per logical characters.               */
/*                                                                   */
/* External References:                                              */
/*   Input:  H_ptr - String to be handled.                           */
/*   Output: H_ptr - Incremented string.                             */
/*   Effect: Input H_ptr is output H_ptr.                            */
/*   Return: None.                                                   */
/*                                                                   */
/*********************************************************************/
#define DBASC_IncPerLogical(x)\
        if(DBASC_IsDBCS(H_GET(*(x)),&ES_DBLow_info)){\
                H_INC_BY(*(x),DBCS_BYTELEN); }\
        else { H_INC_BY(*(x),SBCS_BYTELEN); }\


/*********************************************************************/
/* Function: DBASC_ShouldToDBCS                                      */
/*                                                                   */
/* Descriptive name: The following macro is used in DBToDBCS routine.*/
/*                   This is used to check whether the character is  */
/*                   valid SBCS that should be converted to DBCS.    */
/*                                                                   */
/* External References:                                              */
/*   Input:  H_ptr - String to be handled.                           */
/*   Output: None.                                                   */
/*   Effect: Input H_ptr is output H_ptr.                            */
/*   Return: if return is 1 then not DBCS.                           */
/*           else DBCS.                                              */
/*                                                                   */
/*********************************************************************/
#define DBASC_ShouldToDBCS(a) \
       (!DBASC_IsDBCS(H_GET(a),&ES_DBLow_info))


/*********************************************************************/
/* Function: DBASC_ShouldToDBCS                                      */
/*                                                                   */
/* Descriptive name: The following macro is used in DBToSBCS routine.*/
/*                   This is used to check whether the character     */
/*                   should be convert to SBCS or not.               */
/*                   In ASCII, this only check whther it is DBCS or  */
/*                   not.                                            */
/*                   DBASC_DBToSBCS routine check the converted      */
/*                   character is valid SBCS.                        */
/*                                                                   */
/* External References:                                              */
/*   Input:  H_ptr - String to be handled.                           */
/*   Output: None.                                                   */
/*   Effect: Input H_ptr is output H_ptr.                            */
/*   Return: if return is 1 then DBCS.                               */
/*           else Not DBCS.                                          */
/*                                                                   */
/*********************************************************************/
#define DBASC_ShouldToSBCS(a) (DBASC_IsDBCS(H_GET((a)),&ES_DBLow_info))

#endif
