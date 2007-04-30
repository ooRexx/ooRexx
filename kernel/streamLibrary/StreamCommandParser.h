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
/* REXX Kernel                                                  parse.h       */
/*                                                                            */
/* Data areas for parse.c                                                     */
/*                                                                            */
/******************************************************************************/
#ifndef parse
#define parse
                                                /* return code for gettoken at the end of input string */
#define no_token  1
                                                /* return code for gettoken malloc call failure */
#define no_storage 2

                    /************************************************************************/
                    /* Bit values to be used in the actions field of the actiontablestruct  */
                    /************************************************************************/

                                                /* BitOr - used when item is to be or'd with output  */
#define BitOr       0x01
                                                /* BitAnd - when item is to be and'd with output     */
#define BitAnd      0x02
                                                /* MF - when checking output for not zero            */
#define MF          0x04
                                                /* ME - when checking output using item as a mask    */
                                                /*      to specify what should not be on in output   */
#define ME          0x08
                                                /* MI - when checking output using item as a mask    */
                                                /*      to specify what should be on in output       */
#define MI          0x10
                                                /* CopyItem - when copying item to output            */
#define CopyItem    0x20
                                                /* ConcatItem - when concatenating item to output    */
#define ConcatItem  0x40
                                                /* CallItem - when a call to a specialized function  */
                                                /*            needs to be done. The function pointer */
                                                /*            goes in item                           */
#define CallItem    0x80

                    /************************************************************************/
                    /* Single token structure                                               */
                    /************************************************************************/

typedef struct  tokenstruct{                      /* single token structure */
                                                  /* pointer to the single token parsed from input */
    char *string;
                                                  /* length of the single token */
    size_t length;
                                                  /* offset into input for the single token */
    size_t offset;
   } TOKENSTRUCT;

                    /************************************************************************/
                    /* parse user parameters - this is for any user parms passed            */
                    /************************************************************************/
typedef void *userparms;

                    /***********************************************************************/
                    /* token table - holds parameter token and pointer to the action table */
                    /*                and the address of an unknown token function         */
                    /***********************************************************************/

typedef struct tokentablestruct {
  char *token;
  size_t minlength;           /* minimum length for token to be a valid match with the input token */
  void *ATSP;                 /* this is a pointer to void to get past c's limitations */
                              /* of not being able to specify control blocks that point to each other */
                              /* otherwise it would be typecast to ATS */

                                             /* type cast for calling unknown token routine */
  long (*utp)(struct tokentablestruct *ttsp, const char *TokenString, TOKENSTRUCT *tsp, void *userparms);
 } TTS;

                    /************************************************************************/
                    /* action table - information of what to do with what and to who        */
                    /************************************************************************/

typedef struct actiontablestruct {
                                                /* actions as defined by action bit defines    */
    short actions;
                                                /* length of the item that will be used against output */
    size_t itemlength;
                                                /* area to modify as specified by action */
    void  *output;
                                                /* errorcode to return when actions are in error */
    long  errorcode;
                                                /* area to be used as specified by action */
    void  *item;
                                                /* type cast for a call action-call */
    long (*afp)(TTS *ttsp, const char *TokenString, TOKENSTRUCT *tsp, void *userparms);
 }  ATS;

                    /************************************************************************/
                    /* parse routine prototype                                              */
                    /************************************************************************/
long parser(TTS *ttsp, const char *TokenString, void *userparms);

                    /************************************************************************/
                    /* gettoken routine prototype                                           */
                    /************************************************************************/
int gettoken(const char *TokenString, TOKENSTRUCT *tsp);

                    /************************************************************************/
                    /* ungettoken routine prototype                                         */
                    /************************************************************************/
void ungettoken(const char *TokenString, TOKENSTRUCT *tsp);


#endif


