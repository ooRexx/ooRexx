/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* https://www.oorexx.org/license.html                                        */
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
/* REXX Kernel                                                ParseTarget.hpp */
/*                                                                            */
/* Primitive PARSE instruction parsing target Class Definitions               */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstructionTarget
#define Included_RexxInstructionTarget

#include "FlagSet.hpp"
#include "ParseInstruction.hpp"

class RexxTarget
{
 public:
    inline RexxTarget() { ; }
    inline RexxTarget (RESTORETYPE restoreType) { ; };

    void        init (RexxObject *, RexxObject **, size_t, FlagSet<ParseFlags, 32>, bool, RexxActivation *, ExpressionStack *);
    void        next(RexxActivation *);
    void        moveToEnd();
    void        forward(size_t);
    void        forwardLength(size_t);
    void        absolute(size_t);
    void        backward(size_t);
    void        backwardLength(size_t);
    void        search(RexxString *);
    void        caselessSearch(RexxString *);
    RexxString *getWord();
    RexxString *remainder();
    inline void skipRemainder() { subcurrent = end; }
    void        skipWord();

 protected:

    RexxString * string;                 // current string being parsed
    RexxObject **arglist;                // argument list for PARSE ARG
    ExpressionStack *stack;          // context expression stack (used for anchoring values for GC).
    size_t  stackTop;                    // top location of the epxression stack
    size_t  argcount;                    // count of arguments if PARSE ARG
    size_t  next_argument;               // next PARSE ARG argument
    FlagSet<ParseFlags, 32>  translate;  // string translation flags

    // parsing position state starts here
    size_t  string_length;         // length of the string
    size_t  start;                 // This is the start of the substring what will be parsed into variables
    size_t  end;                   // end of the substring section
    size_t  pattern_start;         // start of the last matched position
    size_t  pattern_end;           // end of the last match position (for numeric triggers, same as pattern_start)
    size_t  subcurrent;            // current location used for parsing into words
};
#endif
