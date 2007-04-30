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
/* REXX Kernel                                                     SelectInstruction.hpp */
/*                                                                            */
/* Primitive SELECT instruction Class Definitions                             */
/*                                                                            */
/******************************************************************************/
#ifndef Included_RexxInstructionSelect
#define Included_RexxInstructionSelect

#include "RexxInstruction.hpp"


class RexxInstructionSelect : public RexxBlockInstruction
{
public:
    inline void *operator new(size_t size, void *ptr) {return ptr;};
    RexxInstructionSelect();
    inline RexxInstructionSelect(RESTORETYPE restoreType) { ; };
    void live();
    void liveGeneral();
    void flatten(RexxEnvelope*);
    void execute(RexxActivation *, RexxExpressionStack *);

    void matchEnd(RexxInstructionEnd *, RexxSource *);
    bool isLabel(RexxString *name);
    RexxString *getLabel();
    bool isLoop();
    void terminate(RexxActivation *, RexxDoBlock *);

    void setOtherwise(RexxInstructionOtherWise *);
    void addWhen(RexxInstructionIf *);

    RexxQueue                *when_list; /* list of WHEN end targets          */
    RexxInstructionEnd       *end;       /* END matching the SELECT           */
    RexxInstructionOtherWise *otherwise; /* OTHERWISE matching the SELECT     */
};


class RexxInstructionLabeledSelect : public RexxInstructionSelect
{
public:
    inline void *operator new(size_t size, void *ptr) {return ptr;};
    RexxInstructionLabeledSelect(RexxString *);
    inline RexxInstructionLabeledSelect(RESTORETYPE restoreType) : RexxInstructionSelect(restoreType) { ; };
    void live();
    void liveGeneral();
    void flatten(RexxEnvelope*);
    virtual RexxString *getLabel();
    bool isLabel(RexxString *name);

    RexxString * label;      // the select label
};
#endif
