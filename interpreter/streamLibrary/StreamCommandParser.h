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
/* REXX Kernel                                                  parse.h       */
/*                                                                            */
/* Data areas for parse.c                                                     */
/*                                                                            */
/******************************************************************************/
#ifndef StreamCommandParser_Included
#define StreamCommandParser_Included

#include <cctype>
#include <ctype.h>
#include "Utilities.hpp"
                                                /* return code for gettoken at the end of input string */
#define no_token  1

                    /************************************************************************/
                    /* Bit values to be used in the actions field of the actiontablestruct  */
                    /************************************************************************/
typedef enum
{
    NoAction,                 // table terminator element
    BitOr,                    // or a value into an integer
    BitAnd,                   // and a value into an integer
    MF,                       // Mutual exclusion field
    ME,                       // Mutual exclusion flag
    MI,                       // Mutual inclusion flag
    MEB,                      // Mutual exclusion bool
    MIB,                      // Mutual inclusion bool
    SetBool,                  // set a boolean item
    SetItem,                  // set an int item to a value
    CallItem,                 // additional processing required
} ActionType;

                    /************************************************************************/
                    /* Single token structure                                               */
                    /************************************************************************/

// single parsing token
class StreamToken
{
public:
    StreamToken(const char *data)
    {
        sourceData = data;
        string = NULL;
        length = 0;
        offset = 0;
    }

    bool nextToken();
    void previousToken();
    inline void skipBlanks()
    {
        while(sourceData[offset] == ' ')
        {
            offset++;
        }
    }


    inline bool equals(const char *token)
    {
        return Utilities::memicmp(token, string, length) == 0;
    }

    inline bool atEnd() { return sourceData[offset] == '\0'; }

    inline bool toNumber(int64_t &num)
    {
        int64_t off = 0;

        /* convert string into long for later*/
        for (size_t i = 0; i < length; i++)
        {
            char ch = string[i];

            if (!isdigit(ch))
            {
                return false;
            }

            off = (off * 10) + (ch - '0');
        }
        num = off;
        return true;
    }


    inline bool toNumber(int &num)
    {
        int off = 0;

        /* convert string into long for later*/
        for (size_t i = 0; i < length; i++)
        {
            char ch = string[i];

            if (!isdigit(ch))
            {
                return false;
            }

            off = (off * 10) + (ch - '0');
        }
        num = off;
        return true;
    }

    inline size_t getLength() { return length; }

protected:

    const char *sourceData;// the source parsing data
    const char *string;    // token data
    size_t length;         // length of the token
    size_t offset;         // offset into the input string for this token
};


class ParseAction;
                    /***********************************************************************/
                    /* token table - holds parameter token and pointer to the action table */
                    /*                and the address of an unknown token function         */
                    /***********************************************************************/

class TokenDefinition
{
public:
    inline TokenDefinition(const char *t, size_t l, ParseAction *a)
    {
        token = t;
        minlength = l;
        actions = a;
        actionRoutine = NULL;
    }

    inline TokenDefinition(int (*a)(TokenDefinition *, StreamToken &, void *))
    {
        token = NULL;
        minlength = 0;
        actions = NULL;
        actionRoutine = a;
    }

    bool isValid() { return token != NULL; }
    int callUnknown(StreamToken &tokenizer, void *parms)
    {
        return (*actionRoutine)(this, tokenizer, parms);
    }
    const char *token;                // token value
    size_t minlength;           // minimum length for token to be a valid match with the input token
    ParseAction *actions;       // token action definition

    // the action routine for processing this token.
    int (*actionRoutine)(TokenDefinition *, StreamToken &, void *);
};

                    /************************************************************************/
                    /* action table - information of what to do with what and to who        */
                    /************************************************************************/

class ParseAction
{
public:
    inline ParseAction()
    {
        action = NoAction;
        int_output = NULL;
        int_value = 0;
        bool_output = NULL;
        bool_value = false;
        afp = NULL;
        actionParm = NULL;
    }

    inline ParseAction(ActionType a, int &target, int source)
    {
        action = a;
        int_output = &target;
        int_value = source;
        bool_output = NULL;
        bool_value = false;
        afp = NULL;
        actionParm = NULL;
    }

    inline ParseAction(ActionType a, int &target)
    {
        action = a;
        int_output = &target;
        int_value = 0;
        bool_output = NULL;
        bool_value = false;
        afp = NULL;
        actionParm = NULL;
    }

    inline ParseAction(ActionType a, bool &target, bool source)
    {
        action = a;
        bool_output = &target;
        bool_value = source;
        int_output = NULL;
        int_value = false;
        afp = NULL;
        actionParm = NULL;
    }

    inline ParseAction(ActionType a, bool &target)
    {
        action = a;
        bool_output = &target;
        bool_value = false;
        int_value = 0;
        int_output = NULL;
        afp = NULL;
        actionParm = NULL;
    }

    inline ParseAction(ActionType a, int (*act)(TokenDefinition *, StreamToken &, void *), void *parm)
    {
        action = a;
        int_output = NULL;
        int_value = 0;
        bool_output = NULL;
        bool_value = false;
        afp = act;
        actionParm = parm;
    }

    inline bool isValid() { return action != NoAction; }
    int applyAction(TokenDefinition *def, StreamToken &token, void *userparms);

protected:

    ActionType action;          // the actions to process (defined by bit flags)
    size_t itemlength;          // length of the output item
    int   *int_output;          // address of the filled in item
    bool  *bool_output;         // address of the filled in item
    int   int_value;            // value used for any int manipulations
    bool  bool_value;           // integer valued output
                                // type cast for a call action-call
    int (*afp)(TokenDefinition *, StreamToken &, void *);
    void *actionParm;           // opaque value passed to action
};

                    /************************************************************************/
                    /* parse routine prototype                                              */
                    /************************************************************************/
int parser(TokenDefinition *ttsp, const char *TokenString, void *userparms);


#endif


