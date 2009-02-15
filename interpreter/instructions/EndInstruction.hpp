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
/* REXX Kernel                                             EndInstruction.hpp */
/*                                                                            */
/* Primitive END instruction Class Definitions                                */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstructionEnd
#define Included_RexxInstructionEnd

#include "RexxInstruction.hpp"
                                       /* types of block instructions       */
#define DO_BLOCK         1
#define SELECT_BLOCK     2
#define OTHERWISE_BLOCK  3
#define LOOP_BLOCK       4
#define LABELED_SELECT_BLOCK 5
#define LABELED_OTHERWISE_BLOCK 6
#define LABELED_DO_BLOCK 7

class RexxInstructionEnd : public RexxInstruction {
 public:
  inline void *operator new(size_t size, void *ptr) {return ptr;}
  inline void operator delete(void *) { }
  inline void operator delete(void *, void *) { }

  inline RexxInstructionEnd(RESTORETYPE restoreType) { ; };
  RexxInstructionEnd(RexxString *);
  void live(size_t);
  void liveGeneral(int reason);
  void flatten(RexxEnvelope *);
  void execute(RexxActivation *, RexxExpressionStack *);

  inline RexxString *endName() { return this->name; };
  inline void        setStyle(size_t type) { instructionFlags = (uint16_t)type; };
  inline size_t      getStyle(void)        { return instructionFlags; };

 RexxString * name;                    /* specified control variable name   */
};
#endif
