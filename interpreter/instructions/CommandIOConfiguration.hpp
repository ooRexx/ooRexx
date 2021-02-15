/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 2005-2018 Rexx Language Association. All rights reserved.    */
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
/*                                                 CommandIOConfiguration.hpp */
/*                                                                            */
/* The processing context for ADDRESS WITH I/O interaction                    */
/*                                                                            */
/******************************************************************************/
#ifndef Included_CommandIOConfiguration
#define Included_CommandIOConfiguration

class CommandIOContext;
class InputRedirector;
class OutputRedirector;

/**
 * The types of the output redirectors
 */
namespace RedirectionType
{
    enum Enum
    {
        NONE,
        DEFAULT,
        NORMAL,
        STEM_VARIABLE,
        USING_OBJECT,
        STEM_OBJECT,
        STREAM_NAME,
        STREAM_OBJECT,
        ARRAY_OBJECT,
        COLLECTION_OBJECT,
        REXXQUEUE_OBJECT,
    };
};


/**
 * Type options used for ADDRESS WITH output operations.
 */
namespace OutputOption
{
    // the options for how how output is handled
    enum Enum
    {
        DEFAULT,
        APPEND,
        REPLACE
    };
}


class CommandIOConfiguration : public RexxInternalObject
{
 friend class LanguageParser;
 public:

    void        *operator new(size_t);
    inline void  operator delete(void *) { }

    CommandIOConfiguration();
    inline CommandIOConfiguration(RESTORETYPE restoreType) { ; };

    void live(size_t) override;
    void liveGeneral(MarkReason reason) override;
    void flatten(Envelope *) override;

    CommandIOContext *createIOContext(RexxActivation *context, ExpressionStack *stack, CommandIOConfiguration *commandConfig);
    InputRedirector  *createInputSource(RexxActivation *context, ExpressionStack *stack, CommandIOConfiguration *mainConfig);
    OutputRedirector *createOutputTarget(RexxActivation *context, ExpressionStack *stack, CommandIOConfiguration *mainConfig);
    OutputRedirector *createErrorTarget(RexxActivation *context, ExpressionStack *stack, CommandIOConfiguration *mainConfig);
    InputRedirector  *createInputSource(RexxActivation *context, ExpressionStack *stack);
    OutputRedirector *createOutputTarget(RexxActivation *context, ExpressionStack *stack);
    OutputRedirector *createErrorTarget(RexxActivation *context, ExpressionStack *stack);
    OutputRedirector *createOutputTarget(RexxString *keyword, RexxActivation *context, ExpressionStack *stack, RexxInternalObject *outputTarget, RedirectionType::Enum type, OutputOption::Enum option);

 protected:
    RexxInternalObject *inputSource;         // The input source expression
    RexxInternalObject *outputTarget;        // The output target expression
    RexxInternalObject *errorTarget;         // The output target expression
    RedirectionType::Enum inputType;               // The type of redirection target to evaluate
    RedirectionType::Enum outputType;              // The output redirection type
    RedirectionType::Enum errorType;               // The error redirection type
    OutputOption::Enum outputOption;               // option for the output stream
    OutputOption::Enum errorOption;                // the option for the error stream
};
#endif


