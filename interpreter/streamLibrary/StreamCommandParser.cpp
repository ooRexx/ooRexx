/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2009 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
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
#include "RexxCore.h"                  /* global REXX definitions           */
#include "StreamCommandParser.h"       /* local structures                  */


/**
 * Get the next token from the parsed string.
 *
 * @return True if a token is available, false if we've hit the end.
 */
bool StreamToken::nextToken()
{
    // move past the previous token...if the token was pushed back,
    // the offset is 0, so we don't really move.
    offset += length;
    // go past any blanks
    skipBlanks();
    // if we got to the end of string, return the end-of-string return
    /*return no_token                    */
    if (sourceData[offset] == '\0')
    {
        string = NULL;              /* nll string pointer                */
        offset = 0;                 /* offset is zero                    */
        length = 0;                 /* and the length is zero            */
        return false;               /* return no-token indicator         */
    }

    // now check for special characters
    switch (sourceData[offset])
    {
        case '=':                         // equals token
        case '+':                         // plus token
        case '-':                         // minus token
        case '<':                         // less than sign
            // point to the token start
            string = sourceData + offset;
            length = 1;                     // single character token found
            return true;                    // had success
    }

    // set the token start position
    string = (const char *)sourceData + offset;
    // else count the number of
    //characters in next token
    for (length = 0 ; sourceData[offset + length] != '\0'; length++   )
    {
        // contain a special character or blank?
        if (strchr("=+-< ", sourceData [offset + length]) != NULL)
        {
            break;                      /* finished                          */
        }
    }
    return true;                        /* return success                    */
}


/**
 * Push a token back on to the stream.
 */
void StreamToken::previousToken()
{
                                       /* just set the length to zero so    */
                                       /* that the current offset is used   */
  length = 0;                          /* for the next token scan           */
}

/*********************************************************************/
/*  parse                                                            */
/*     Will go thru the input STRING processing each token that      */
/*     matches with the TokenTable passed in. It will return either  */
/*     a zero, if all the tokens get parsed, or the error code       */
/*     from the actions table, or if there is a call what is passed  */
/*     back from it.                                                 */
/*********************************************************************/

int parser(TokenDefinition *ttsp, const char *tokenString, void *userparms)
{
    int  result = 0;                    /* parse result                      */
    StreamToken tokenizer(tokenString); /* single token structure            */

                                        /* Process each token in tokenstring */
    while (tokenizer.nextToken())
    {
        TokenDefinition *def;
        // check each of the table elements
        for ( def = ttsp; def->isValid() && !tokenizer.equals(def->token); def++);

        if (!def->isValid())              /* no token found?                   */
        {
            tokenizer.previousToken();    // push the token back
                                          /* call the caller specified function*/
                                          /* when a parameter is not in the    */
                                          /*token table                        */
            result = def->callUnknown(tokenizer, userparms);
            if (result != 0)              /* get an error?                     */
            {
                return result;            /* return that result                */
            }
        }
        else
        {
            /* if token is less than the valid   */
            /*minimum                            */
            if (def->minlength > tokenizer.getLength())
            {
                /* it is considered ambiguous since  */
                /*it can match more than one of the  */
                /*valid keywords                     */
                return 1;                    /* return a failure                  */

            }
            /* Check actions table for work item */
            for (ParseAction *actionItem = def->actions; actionItem->isValid(); actionItem++)
            {
                // process the action
                if (actionItem->applyAction(ttsp, tokenizer, userparms) != 0)
                {
                    return 1;
                }
            }
        }
    }
    return 0;                           /* just return zero                  */
}


/**
 * Apply an a parse table action.
 *
 * @param def       The token definition this belongs to.
 * @param token     The token parsing stream.
 * @param userparms An opaque argument value.
 *
 * @return 0 if the action was processed ok, 1 if it is an error condition.
 */
int ParseAction::applyAction(TokenDefinition *def, StreamToken &token, void *userparms)
{
    switch (action)               /* check out what to do              */
    {
        case (BitOr):               /* item or'ed with the output?       */
            // Bit operations only get applied to int items
            *int_output |= int_value;
            return 0;

        case (BitAnd):              /* and the item with output          */
            // Bit operations only get applied to int items
            *int_output &= int_value;
            return 0;
            /* for a mutual field                */
            /* check where the output            */
            /* is pointing for any value - for   */
            /* the length of int                 */
            /*  If there is a value - return the */
            /*  errorcode                        */
        case (MF):
            /* for mutual exclusion              */
            /* check the item mask against the   */
            /* output to see if the specified    */
            /* bits are on.  If any of the bits  */
            /* are on - return the errorcode     */
            return (*int_output != 0) ? 1 : 0;
        case (MEB):
            /* for mutual exclusion              */
            /* check the item mask against the   */
            /* output to see if the specified    */
            /* bits are on.  If any of the bits  */
            /* are on - return the errorcode     */
            return (*bool_output) ? 1 : 0;

        case (MIB):
            /* for mutual inclusion              */
            /* check the item mask against the   */
            /* output to see if the specified    */
            /* bits are on.  If any of the bits  */
            /* are on - return the errorcode     */
            return (*bool_output) ? 0 : 1;

        case (ME):
            return (*int_output & int_value) ?  1 : 0;
            /* for mutual inclusion              */
            /* check the item mask against the   */
            /* output to see if the specified    */
            /* bits are off.  If any of the bits */
            /* are off - return the errorcode    */
        case (MI):
            return ((*int_output & int_value) != int_value) ? 1 : 0;
            /* for copy of item into the output  */
            /* use the c-lib memcpy function     */
        case SetItem:
            // just directly set the item
            *int_output = int_value;
            return 0;
        case SetBool:
            // just directly set the item
            *bool_output = bool_value;
            return 0;

            /* for call - use item as function   */
            /* pointer and pass all the information available */
        case (CallItem) :
            return (*afp)(def, token, actionParm);
            /* table error if there is an        */
            /* unknown actions                   */
        default :
            break;
    }
    return 1;
}
