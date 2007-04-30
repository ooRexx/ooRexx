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
/* REXX Kernel                                                   CallInstruction.hpp  */
/*                                                                            */
/* Primitive CALL instruction Class Definitions                               */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstructionCall
#define Included_RexxInstructionCall

#include "RexxInstruction.hpp"

typedef struct callshortoverlay {
  UCHAR  builtin_index;                /* builtin function index            */
  UCHAR  argument_count;               /* count of arguments                */
} CALLSHORTOVERLAY;

#define call_nointernal  0x01          /* bypass internal routine calls     */
#define call_type_mask   0x0e          /* type of call                      */
#define call_internal    0x02          /* internal call                     */
#define call_builtin     0x06          /* builtin call                      */
#define call_external    0x0e          /* external call                     */
#define call_dynamic     0x10          /* dynamic call                      */
#define call_on_off      0x20          /* call ON/OFF instruction           */
                                       /* builtin function call index       */
#define call_builtin_index  (((CALLSHORTOVERLAY *)(&(this->instructionInfo.general)))->builtin_index)
#define call_argument_count (((CALLSHORTOVERLAY *)(&(this->instructionInfo.general)))->argument_count)

class RexxInstructionCallBase : public RexxInstruction {
 public:
  inline RexxInstructionCallBase() { ; };
  virtual void resolve(RexxDirectory *) { ; };
  virtual void trap(RexxActivation *, RexxDirectory *) { ; };

  RexxObject      * name;              /* name to call                      */
  RexxInstruction * target;            /* routine to call                   */
  RexxString      * condition;         /* condition trap name               */
};

class RexxInstructionCall : public RexxInstructionCallBase {
 public:

  inline void *operator new(size_t size, void *ptr) {return ptr;};
  RexxInstructionCall(RexxObject *, RexxString *, size_t, RexxQueue *, CHAR, CHAR);
  inline RexxInstructionCall(RESTORETYPE restoreType) { ; };
  void live();
  void liveGeneral();
  void flatten(RexxEnvelope*);
  void execute(RexxActivation *, RexxExpressionStack *);
  void resolve(RexxDirectory *);
  void trap(RexxActivation *, RexxDirectory *);

  RexxObject * arguments[1];           /* argument list                     */
};
#endif
