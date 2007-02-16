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
/* REXX Kernel                                                  Clause.hpp  */
/*                                                                            */
/* Primitive Translator Clause Class Definitions                              */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxClause
#define Included_RexxClause

#include "ArrayClass.hpp"
#include "Token.hpp"

class RexxClause : public RexxInternalObject {
 public:
  RexxClause();
  inline RexxClause(RESTORETYPE restoreType) { ; };
  void        *operator new(size_t);
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  void        live();
  void        liveGeneral();
  void        flatten(RexxEnvelope *);
  void        setStart(size_t, size_t);
  void        setEnd(size_t, size_t);
  void        trim();
  void        newClause();
  RexxToken  *newToken(int, int, RexxString *, PLOCATIONINFO);
  RexxToken  *nextRealToken();
  inline void        firstToken() {this->current = this->first;};
  inline void        getLocation(PLOCATIONINFO location) { *location = this->location; }
  inline void        setLocation(PLOCATIONINFO location) { this->location = *location; }
  inline void        previous() { this->current--; }
  inline RexxToken  *next() { return (RexxToken *)this->tokens->get(this->current++); }

  LOCATIONINFO location;               /* position of the clause            */
  size_t current;                      /* index of current token of clause  */
  size_t first;                        /* first token of clause             */
  RexxArray  *tokens;                  /* array of tokens in the clause     */
  size_t size;                         /* size of token array               */
  size_t free;                         /* location of first free token      */
};
#endif
