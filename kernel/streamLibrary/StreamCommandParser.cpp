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
/* REXX Kernel                                                  parse.c       */
/*                                                                            */
/* Stream option string parsing                                               */
/*                                                                            */
/******************************************************************************/
#include <stdio.h>                     /* for printf calls                  */
#include <stdlib.h>                    /* for malloc call                   */
#include <string.h>                    /* for string functions              */
#include "RexxCore.h"                    /* global REXX definitions           */
#include "RexxNativeAPI.h"
#include "StreamCommandParser.h"       /* local structures                  */

/*********************************************************************/
/*  gettoken                                                         */
/*     Will go thru the input STRING starting at the next non blank  */
/*     up to the next blank, returning the length of the next token, */
/*     the offset into the input STRING of the current token         */
/*     and the token string pointer pointing to storage that the     */
/*     caller will need to free, or a null pointer if the end of     */
/*     STRING is reached.                                            */
/*********************************************************************/

int gettoken(const char *TokenString,TOKENSTRUCT *tsp)
{
                                       /* update the offset from previous   */
                                       /*call                               */
   tsp->offset += tsp->length;
                                       /* go past any blanks                */
   for ( ; TokenString[tsp->offset] == ' '; tsp->offset++ ){
   };
                                       /* if we got to the end of string,   */
                                       /*return no_token                    */
   if (TokenString[tsp->offset] == '\0') {
      tsp->string = NULL;              /* nll string pointer                */
      tsp->offset = 0;                 /* offset is zero                    */
      tsp->length = 0;                 /* and the length is zero            */
      return no_token;                 /* return no-token indicator         */
   }

   switch (TokenString[tsp->offset]) { /* first check for special characters*/
     case '=':                         /* equals token                      */
     case '+':                         /* plus token                        */
     case '-':                         /* minus token                       */
     case '<':                         /* less than sign                    */
                                       /* point to the token start          */
       tsp->string = (char *)&TokenString[tsp->offset];
       tsp->length = 1;                /* single character token found      */
       return 0;                       /* had success                       */
   }
                                       /* set the token start position      */
   tsp->string = (char *)&TokenString[tsp->offset];
                                       /* else count the number of          */
                                       /*characters in next token           */
   for (tsp->length = 0 ; (TokenString[tsp->offset + tsp->length] != ' ') &&
                          (TokenString[tsp->offset + tsp->length] != '\0') ;
        tsp->length++   ) {
                                       /* contain a special character?      */
     if (strchr("=+-<", TokenString[tsp->offset + tsp->length]) != NULL)
       break;                          /* finished                          */
   }
   return 0;                           /* return success                    */
}
/*********************************************************************/
/*  ungettoken                                                       */
/*     Will go back thru the input STRING starting at the current    */
/*     position until a blank or the beginning of the string is      */
/*     encountered.                                                  */
/*********************************************************************/


void ungettoken(const char *TokenString,TOKENSTRUCT *tsp)
{
                                       /* just set the length to zero so    */
                                       /* that the current offset is used   */
  tsp->length = 0;                     /* for the next token scan           */
}

/*********************************************************************/
/*  parse                                                            */
/*     Will go thru the input STRING processing each token that      */
/*     matches with the TokenTable passed in. It will return either  */
/*     a zero, if all the tokens get parsed, or the error code       */
/*     from the actions table, or if there is a call what is passed  */
/*     back from it.                                                 */
/*********************************************************************/

long parser(TTS *ttsp, const char *TokenString, void *userparms)

{
   long result = 0;                    /* parse result                      */
   TTS *work_ttsp;                     /* token struct work pointer         */
   ATS *work_ATSP;                     /* action table work pointer         */
   TOKENSTRUCT ts_struct = {0,0,0};    /* single token structure            */
   TOKENSTRUCT *tsp;                   /* the current working token pointer */
   tsp = &ts_struct;                   /* set up the token pointer          */

                                       /* Process each token in tokenstring */
   while ((result = gettoken(TokenString,tsp)) == 0) {
     for (work_ttsp = ttsp;            /* check against the table           */
         (*(work_ttsp->token)) &&
         (memicmp((void *)(work_ttsp->token),(void *)(tsp->string),tsp->length));
       work_ttsp++ ) {
     }
     if (!*(work_ttsp->token)) {       /* no token found?                   */
       ungettoken(TokenString,tsp);    /* put the unknown token back        */

                                       /* call the caller specified function*/
                                       /* when a parameter is not in the    */
                                       /*token table                        */
       result = (*(work_ttsp->utp))(ttsp, TokenString, tsp, userparms);
       if (result != 0)                /* get an error?                     */
         return result;                /* return that result                */

     }
     else {
                                       /* if token is less than the valid   */
                                       /*minimum                            */
       if (work_ttsp->minlength > tsp->length)
                                       /* it is considered ambiguous since  */
                                       /*it can match more than one of the  */
                                       /*valid keywords                     */
          return 1;                    /* return a failure                  */
                                       /* Check actions table for work item */
       for (work_ATSP = (ATS *)work_ttsp->ATSP ; work_ATSP->actions && result == 0 ;
            work_ATSP++ ) {
         switch (work_ATSP->actions) { /* check out what to do              */

           case (BitOr):               /* item or'ed with the output?       */
                                       /* do for the items length           */
             switch (work_ATSP->itemlength) {

               case (sizeof(char)):    /* char                              */
                 *(char *)work_ATSP->output |= *(char *)work_ATSP->item;
                 break;

               case (sizeof(short)):   /* short                             */
                 *(short *)work_ATSP->output |= *(short *)work_ATSP->item;
                 break;

               default:                /* long                              */
                  *(long *)work_ATSP->output |= *(long *)work_ATSP->item;
             }
             break;
           case (BitAnd):              /* and the item with output          */
                                       /* do it for the length of the item  */
             switch (work_ATSP->itemlength) {

               case (sizeof(char)):    /* char                              */
                 *(char *)work_ATSP->output &= *(char *)work_ATSP->item;
                 break;

               case (sizeof(short)):   /* short                             */
                 *(short *)work_ATSP->output &= *(short *)work_ATSP->item;
                 break;

               default:                /* long                              */
                 *(long *)work_ATSP->output &= *(long *)work_ATSP->item;
             }
             break;
                                       /* for a mutual field                */
                                       /* check where the output            */
                                       /* is pointing for any value - for   */
                                       /* the length of int                 */
                                       /*  If there is a value - return the */
                                       /*  errorcode                        */
           case (MF):
                                       /* process based on length           */
             switch (work_ATSP->itemlength) {

               case (sizeof(char)):    /* character field                   */
                                       /* already set?                      */
                  if (*(char *)work_ATSP->output)
                                       /* error                             */
                     return work_ATSP->errorcode;
                  break;

               case (sizeof(short)):   /* short field                       */
                                       /* already set?                      */
                 if (*(short *)work_ATSP->output)
                                       /* error                             */
                   return work_ATSP->errorcode;
                 break;

               default:                /* long field                        */
                                       /* already set?                      */
                 if (*(long *)work_ATSP->output)
                                       /* error                             */
                   return work_ATSP->errorcode;
             }
             break;
                                       /* for mutual exclusion              */
                                       /* check the item mask against the   */
                                       /* output to see if the specified    */
                                       /* bits are on.  If any of the bits  */
                                       /* are on - return the errorcode     */

           case (ME):
                                       /* process based on length           */
             switch (work_ATSP->itemlength) {

               case (sizeof(char)):    /* character field                   */
                 if (*(char *)work_ATSP->output & *(char *)work_ATSP->item)
                                       /* get out with an error code        */
                   return work_ATSP->errorcode;
                 break;

               case (sizeof(short)):   /* short field                       */
                 if (*(short *)work_ATSP->output & *(short *)work_ATSP->item)
                                       /* get out with an error code        */
                   return work_ATSP->errorcode;
                 break;

               default:                /* long field                        */
                                       /* had a conflict?                   */
                 if (*(long *)work_ATSP->output & *(long *)work_ATSP->item)
                                       /* get out with an error code        */
                   return work_ATSP->errorcode;
                 break;
             }
             break;
                                       /* for mutual inclusion              */
                                       /* check the item mask against the   */
                                       /* output to see if the specified    */
                                       /* bits are off.  If any of the bits */
                                       /* are off - return the errorcode    */
           case (MI):
                                       /* process based on length           */
             switch (work_ATSP->itemlength) {

               case (sizeof(char)):    /* character field                   */
                                       /* have all of the required options? */
                 if ((*(char *)work_ATSP->output &
                     *(char *)work_ATSP->item) != *(char *)work_ATSP->item)
                                       /* return the error code             */
                   return work_ATSP->errorcode;
                 break;

               case (sizeof(short)):   /* short field                       */
                                       /* have all of the required options? */
                 if ((*(short *)work_ATSP->output &
                     *(short *)work_ATSP->item) != *(short *)work_ATSP->item)
                                       /* return the error code             */
                   return work_ATSP->errorcode;
                 break;

               default:                /* long field                        */
                                       /* have all of the required options? */
                 if ((*(long *)work_ATSP->output &
                     *(long *)work_ATSP->item) != *(long *)work_ATSP->item)
                                       /* return the error code             */
                   return work_ATSP->errorcode;
                 break;

             }
             break;
                                       /* for copy of item into the output  */
                                       /* use the c-lib memcpy function     */
           case (CopyItem):
             (void *)memcpy((char *)work_ATSP->output,(char *)work_ATSP->item,work_ATSP->itemlength);
             break;
                                       /* for concatenation of item into    */
                                       /* the output use the c-lib strncat  */
                                       /*function                           */
           case (ConcatItem):
             (void *)strncat((char *)work_ATSP->output,(char *)work_ATSP->item,work_ATSP->itemlength);
             break;
                                       /* for call - use item as function   */
                                       /* pointer and pass all the information available */
           case (CallItem) :
             result = (*(work_ATSP->afp))(ttsp, TokenString, tsp, userparms);
             if (result != 0)          /* have an error?                    */
               return result;          /* return the error code             */
             break;
                                       /* table error if there is an        */
                                       /* unknown actions                   */
           default :
             break;
         }
       }
     }
   }
   return 0;                           /* just return zero                  */
}
