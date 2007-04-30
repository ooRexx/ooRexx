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
/* REXX Kernel                                                   RexxInstruction.hpp */
/*                                                                            */
/* Primitive Abstract Instruction Class Definitions                           */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstruction
#define Included_RexxInstruction

class RexxInstructionEnd;
class RexxSource;

#define i_flags  (this->instructionInfo.flags)
#define i_ushort (this->instructionInfo.general)

class RexxInstruction : public RexxInternalObject {
 public:
  RexxInstruction(RexxClause *clause, int type);
  inline RexxInstruction(RESTORETYPE restoreType) { ; };
  inline RexxInstruction() { ; }
  void * operator new(size_t);
  inline void * operator new(size_t size, void *objectPtr) { return objectPtr; } ;

  void live();
  void liveGeneral();
  void flatten(RexxEnvelope *);
  void getLocation(PLOCATIONINFO location);
  void setLocation(PLOCATIONINFO location);

  virtual void execute(RexxActivation *, RexxExpressionStack *) { ; };

  inline void setNext(RexxInstruction *next) { OrefSet(this, this->nextInstruction, next); };
  void        setStart(LONG line, LONG offset) {this->lineNumber = line;
                                                this->offset = offset; };
  void        setEnd(LONG line, LONG offset) { this->endLine = line;
                                               this->endOffset = offset; };
  inline      void        setType(UCHAR type) {this->instructionInfo.type = type; };
  inline      UCHAR       getType(void)       {return this->instructionInfo.type; };
  inline      LONG        getLine() { return this->lineNumber; };

  LONG              offset;            /* offset of instruction start       */
  LONG              endLine;           /* end line of the instruction       */
  LONG              endOffset;         /* end offset of the instruction     */
  INSTRUCTIONINFO   instructionInfo;   /* instruction common information    */
  RexxInstruction  *nextInstruction;   /* next instruction object           */
};

class RexxDoBlock;

class RexxBlockInstruction : public RexxInstruction {
public:
    RexxBlockInstruction() {;};
    RexxBlockInstruction(RESTORETYPE restoreType) { ; };

    virtual bool isLabel(RexxString *) { return false; }
    virtual RexxString *getLabel() { return OREF_NULL; };
    virtual bool isLoop() { return false; };
    virtual void matchEnd(RexxInstructionEnd *, RexxSource *) { ; };
    virtual void terminate(RexxActivation *, RexxDoBlock *) { ; };
};


class RexxInstructionSet : public RexxInstruction {
 public:
  RexxInstructionSet() {;};
  RexxInstructionSet(RESTORETYPE restoreType) { ; };

  virtual void setEndInstruction(RexxInstructionEndIf *) {;}
};

class RexxInstructionExpression : public RexxInstruction {
 public:
  RexxInstructionExpression() { ; };
  RexxInstructionExpression(RESTORETYPE restoreType) { ; };

  void live();
  void liveGeneral();
  void flatten(RexxEnvelope *);

  RexxObject *expression;              /* expression to evaluate            */
};
#endif
