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
/* REXX Kernel                                                ParseTarget.hpp */
/*                                                                            */
/* Primitive PARSE instruction parsing target Class Definitions               */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstructionTarget
#define Included_RexxInstructionTarget

#define PARSE_UPPER     1              /* Uppercase parsed string           */
#define PARSE_LOWER     2              /* Lowercase parsed string           */
#define PARSE_CASELESS  1              /* do case insensitive searches      */

class RexxTarget {
 public:
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  inline RexxTarget() { ; }
  inline RexxTarget (RESTORETYPE restoreType) { ; };
  void        init (RexxObject *, RexxObject **, size_t, size_t, bool, RexxActivation *, RexxExpressionStack *);
  void        next(RexxActivation *);
  void        moveToEnd();
  void        forward(stringsize_t);
  void        forwardLength(stringsize_t);
  void        absolute(stringsize_t);
  void        backward(stringsize_t);
  void        backwardLength(stringsize_t);
  void        search(RexxString *);
  void        caselessSearch(RexxString *);
  RexxString *getWord();
  RexxString *remainder();
  void        skipRemainder() { this->subcurrent = this->end;      /* eat the remainder piece           */ }
  void        skipWord();

  RexxString * string;                 /* parsed string                     */
  RexxObject **arglist;                /* argument list for PARSE ARG       */
  RexxExpressionStack *stack;          // context expression stack (used for anchoring values for GC).
  size_t  stackTop;                    // top location of the epxression stack
  size_t  argcount;
  stringsize_t  start;                 /* start of substring                */
  stringsize_t  end;                   /* end of the substring              */
  stringsize_t  string_length;         /* length of the string              */
  stringsize_t  pattern_end;           /* end of matched pattern            */
  stringsize_t  pattern_start;         /* start of matched pattern          */
  stringsize_t  subcurrent;            /* current location for word parse   */
  size_t  next_argument;               /* next PARSE ARG argument           */
  size_t  translate;                   /* string translation flag           */
};
#endif
