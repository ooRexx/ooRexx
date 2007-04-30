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
/* REXX Kernel                                                   DoBlock.hpp */
/*                                                                            */
/* Primitive Do Block Class Definitions                                       */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxDoBlock
#define Included_RexxDoBlock

class RexxBlockInstruction;

class RexxDoBlock : public RexxInternalObject {
 public:

  void *operator new(size_t);
  inline void *operator new(size_t size, void *ptr) {return ptr;};
  RexxDoBlock(RexxBlockInstruction *, INT);
  inline RexxDoBlock(RESTORETYPE restoreType) { ; };
  void live();
  void liveGeneral();
  void flatten(RexxEnvelope *);

  inline RexxObject * getTo() {return this->to;};
  inline LONG getFor() {return this->forcount;};
  inline INT  getCompare() {return (INT)(this->compare);};
  inline RexxObject * getBy() {return this->by;};
  inline RexxBlockInstruction * getParent() {return this->parent;};
  inline void setTo(RexxObject * value) {this->to = value;};
  inline void setBy(RexxObject * value) {this->by = value;};
  inline void setCompare(int value) {this->compare = (USHORT)value;};
  inline void setFor(long value) {this->forcount = value;};
  inline BOOL testFor() {return (this->forcount--) <= 0;};
  inline LONG getIndent() { return this->indent; };
  inline void setPrevious(RexxDoBlock *block) { this->previous = block; }

  RexxDoBlock       *previous;         /* previous stacked Do Block         */
  RexxBlockInstruction *parent;        /* parent instruction                */
  RexxObject        *to;               /* final target value                */
  RexxObject        *by;               /* control increment value           */
  LONG               forcount;         /* number of iterations              */
  LONG               indent;           /* base indentation                  */
  LONG               compare;          /* type of comparison                */
};
#endif
