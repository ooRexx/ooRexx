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
/* REXX Kernel                                             DoInstruction.hpp  */
/*                                                                            */
/* Primitive DO instruction Class Definitions                                 */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstructionDo
#define Included_RexxInstructionDo

#include "RexxInstruction.hpp"

#define SIMPLE_DO         1
#define DO_COUNT          2
#define DO_FOREVER        3
#define DO_WHILE          4
#define DO_UNTIL          5
#define CONTROLLED_DO     6
#define CONTROLLED_WHILE  7
#define CONTROLLED_UNTIL  8
#define DO_OVER           9
#define DO_OVER_WHILE    10
#define DO_OVER_UNTIL    11
#define DO_COUNT_WHILE   12
#define DO_COUNT_UNTIL   13

#define EXP_TO           1             /* TO expression                     */
#define EXP_BY           2             /* BY expression                     */
#define EXP_FOR          3             /* FOR expression                    */

class RexxInstructionDo : public RexxBlockInstruction
{
 public:

  inline void *operator new(size_t size, void *ptr) {return ptr;}
  inline void operator delete(void *) { }
  inline void operator delete(void *, void *) { }


  inline RexxInstructionDo(void) { ; }
  inline RexxInstructionDo(RESTORETYPE restoreType) { ; };

  void matchEnd(RexxInstructionEnd *, RexxSource *);
  bool    isLabel(RexxString *name);
  RexxString *getLabel();
  bool    isLoop();
  void terminate(RexxActivation *, RexxDoBlock *);

  void live(size_t);
  void liveGeneral(int reason);
  void flatten(RexxEnvelope *);
  void execute(RexxActivation *, RexxExpressionStack *);
  void controlSetup(RexxActivation *, RexxExpressionStack *, RexxDoBlock *);
  bool checkOver(RexxActivation *, RexxExpressionStack *, RexxDoBlock *);
  bool checkControl(RexxActivation *, RexxExpressionStack *, RexxDoBlock *, bool);
  void reExecute(RexxActivation *, RexxExpressionStack *, RexxDoBlock *);
  bool whileCondition(RexxActivation *, RexxExpressionStack *);
  bool untilCondition(RexxActivation *, RexxExpressionStack *);
  RexxInstruction *getEnd();
  void matchLabel(RexxInstructionEnd *end, RexxSource *source );

  RexxObject       *initial;           /* initial control expression        */
  RexxObject       *to;                /* final target value                */
  RexxObject       *by;                /* control increment value           */
  RexxVariableBase *control;           /* control variable retriever        */
  RexxString       *label;             /* control variable name             */
  RexxObject       *conditional;       /* while/until expression            */
  RexxInstruction  *end;               /* matching END instruction          */
  RexxObject       *forcount;          /* number of iterations              */
  uint8_t           type;              /* type of loop                      */
  uint8_t           expressions[3];    /* controlled loop expression order  */
};
#endif
