/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Copyright (c) 1995, 2004 IBM Corporation. All rights reserved.             */
/* Copyright (c) 2005-2014 Rexx Language Association. All rights reserved.    */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.oorexx.org/license.html                                         */
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
/* Object REXX Kernel                                                         */
/*                                                                            */
/* Main language parser transient class.                                      */
/*                                                                            */
/******************************************************************************/

/**
 * Perform garbage collection on a live object.
 *
 * @param liveMark The current live mark.
 */
void LanguageParser::live(size_t liveMark)
{
    memory_mark(this->clause);
    memory_mark(this->first);
    memory_mark(this->currentInstruction);
    memory_mark(this->last);
    memory_mark(this->savelist);
    memory_mark(this->holdstack);
    memory_mark(this->variables);
    memory_mark(this->literals);
    memory_mark(this->labels);
    memory_mark(this->strings);
    memory_mark(this->guard_variables);
    memory_mark(this->exposed_variables);
    memory_mark(this->control);
    memory_mark(this->terms);
    memory_mark(this->subTerms);
    memory_mark(this->operators);
    memory_mark(this->calls);
    memory_mark(this->class_dependencies);
    memory_mark(this->active_class);
}

/**
 * Perform generalized live marking on an object.  This is
 * used when mark-and-sweep processing is needed for purposes
 * other than garbage collection.
 *
 * @param reason The reason for the marking call.
 */
void LanguageParser::liveGeneral(int reason)
{
    memory_mark_general(this->clause);
    memory_mark_general(this->first);
    memory_mark_general(this->currentInstruction);
    memory_mark_general(this->last);
    memory_mark_general(this->savelist);
    memory_mark_general(this->holdstack);
    memory_mark_general(this->variables);
    memory_mark_general(this->literals);
    memory_mark_general(this->labels);
    memory_mark_general(this->strings);
    memory_mark_general(this->guard_variables);
    memory_mark_general(this->exposed_variables);
    memory_mark_general(this->control);
    memory_mark_general(this->terms);
    memory_mark_general(this->subTerms);
    memory_mark_general(this->operators);
    memory_mark_general(this->calls);
    memory_mark_general(this->class_dependencies);
    memory_mark_general(this->active_class);
}


/**
 * Initialize the parser before starting the parse operation.
 * NOTE:  This is a transient object, which will never be stored
 * in the oldspace, so we don't need to use OrefSet.
 */
void LanguageParser::initialize()
{
    // handy stack for temporary values...this is a push through
    holdStack = new (HOLDSIZE, false) RexxStack(HOLDSIZE);
    // a save table for holding on to things longer
    saveList = new_identity_table();

    control = new_queue();
    terms = new_queue();
    subTerms = new_queue();
    operators = new_queue();
    literals = new_directory();
    // during an image build, we have a global string table.  If this is
    // available now, use it.
    strings = memoryObject.getGlobalStrings();
    if (strings == OREF_NULL)
    {
        // no global string table, use a local copy
        strings = new_directory();
    }
    // create the singleton clause object for parsing
    clause = new RexxClause();
}


/**
 * Convert a source object into executable form.
 *
 * @param isMethod Indicates if we're processing a method context or not.
 *                 We do additional directive installs in that situation.
 *
 * @return A code object that represents this compiled source.
 */
RexxCode *LanguageParser::generateCode(bool isMethod)
{
    // initialized the parsing tables.
    initialize();
    // now translate the code
    RexxCode *newCode = translate(OREF_NULL);

    ProtectedObject p(newCode);
    // TODO:  Not sure install belongs in this context.
    // if generating a method object, then process the directive installation now
    if (isMethod)
    {
        // force this to install now
        install();
    }
    return newCode;                      /* return the method                 */
}


/**
 * Handle translating an interpret instruction.
 *
 * @param _labels Labels inherited from the parent source context.
 *
 * @return A translated code object.
 */
RexxCode *LanguageParser::translateInterpret(RexxDirectory *_labels )
{
    // setup the environment
    initialize();
    // make sure interpret is set up.
    flags.set(interpret);

    // translate, using the inherited labels
    return translate(_labels);
}


/**
 * Remove an object from the save list.
 *
 * TODO:  See if we can eliminate this.
 *
 * @param object The object to process.
 */
void LanguageParser::toss(RexxObject *object)
{
    // if we have a real object, remove it from the
    // save list and push it on to the hold stack for
    // temporary protection.
    if (object != OREF_NULL)
    {
        saveList->remove(object);
        holdObject(object);
    }
}


/**
 * Create a stack frame for this parsing context.
 *
 * @return a stack frame instance for error reporting
 */
StackFrameClass *LanguageParser::createStackFrame()
{
    // construct the traceback line before we allocate the stack frame object.
    // calling this in the constructor argument list can cause the stack frame instance
    // to be inadvertently reclaimed if a GC is triggered while evaluating the constructor
    // arguments.
    RexxString *traceback = package->traceBack(OREF_NULL, clauseLocation, 0, true);
    ProtectedObject p(traceback);
    return new StackFrameClass(FRAME_PARSE, programName, OREF_NULL, OREF_NULL, OREF_NULL, traceback, clauseLocation.getLineNumber());
}


/**
 * Validate that a given token is a variable (i.e., a symbol
 * that does not begin with a digit).
 *
 * @param token  The target token.
 */
void LanguageParser::needVariable(RexxToken  *token)
{
    // this must be a variable token.
    if (!token->isVariable())
    {
        // the error message depends on whether this begins with a dot
        // or is a numeric value
        if (token->value->getChar(0) == '.')
        {
            syntaxError(Error_Invalid_variable_period, token);
        }
        else
        {
            syntaxError(Error_Invalid_variable_number, token);
        }
    }
}


/**
 * Validate that a token is a variable or the literal
 * dot (e.g. the tokens allowed in a parsing template).
 *
 * @param token  The target token.
 */
void LanguageParser::needVariableOrDotSymbol(RexxToken  *token)
{
    // check if the correct types
    if (!token->isVariable() && !token->isDot())
    {
        syntaxError(Error_Invalid_variable_number, token);
    }
}


/**
 * Advance the current position to the next line.
 */
void LanguageParser::nextLine()
{
    if (clause != OREF_NULL)       // do we have a clause object active?
    {
        // record the current position in the clause...this marks the location end.
        clause->setEnd(lineNumber, lineOffset);
    }
    // move to the start of the next line
    position(line_number + 1, 0);
}

/**
 * Move the current scan position to a new source location.
 *
 * @param line   The target line number.
 * @param offset The offset within the new line.
 */
void LanguageParser::position(size_t line, size_t offset)
{
    lineNumber = line;                 // set the new position information
    lineOffset = offset;

    // retrieve the line specifics from the source object.  If this
    // is out of bounds, current will be nulled out.
    source->getLine(line, current, currentLength);
}

/**
 * Extract a clause from the source and return as a clause
 * object.  The clause object contains a list of all of the
 * tokens contained within the clause and is used by
 * the parser to determine the type of instruction and
 * create the instruction parse tree.
 *
 * @return true if we managed to get a clause, false otherwise.
 */
bool LanguageParser::nextClause()
{
    SourceLocation location;             // location of the clause
    SourceLocation tokenLocation;        // location of each token

    // if reclamed is true, we have previously given up on processing a clause, so
    // the tokens are already available.  This is not typically the case.
    if (!flags.test(reclaimed))
    {
        // we keep one clause object, just reset the tokens in the clause.
        clause->newClause();
        // scan over all null clauses.  These could be blank lines, comments,
        // semi-colons, etc.  Those are not interesting.
        for (;;)
        {
            // record the start position
            clause->setStart(lineNumber, lineOffset);
            // get the next source token.  White space tokens are ignored in this context,
            // as we're looking for the beginning of a real clause.
            RexxToken *token = sourceNextToken(OREF_NULL);
            // OREF_NULL indicates we've hit the end of the source.  Mark us as
            // finished and return
            if (token == OREF_NULL)
            {
                flags.set(noClause);
                return false;
            }
            // we have a token, but is it an end of clause marker (explicit or implicit
            // semicolon).  If not, we've got a real clause to process.
            if (!token->isEndOfClause())
            {
                break;
            }
            // reset the clause and start again.
            clause->newClause();
        }
        // get the clause start position from the first token, and
        // make a copy for the start
        tokenLocation = token->getLocation();
        location = tokenLocation;
        // record this in the clause for potential error reporting.
        clause->setLocation(location);

        // now consume tokens until we find an end of clause marker.
        for (;;)
        {
            // get the next token.  Blanks can now be significant inside
            // a clause.
            token = sourceNextToken(token);
            // and save the position again.
            tokenLocation = token->getLocation();
            // and quit once we've hit an end-of-clause (note, we'll never
            // see a null return here because there will be an end of clause
            // implied by the end of line.  We only see the null case when
            // we're scanning for a new clause.
            if (token->isEndOfClause())
            {
                break;
            }
        }

        // the location of the last scanned token is the clause end position.
        location.setEnd(tokenLocation);
        // and set this in the class to give it the full bounds.
        clause->setLocation(location);
    }

    // no reclaimed clause now, regardless of how we got here.
    flags.reset(reclaimed);
    // always set the error information for the clause being processed.
    clauseLocation = clause->getLocation();
    // we have a clause
    return true;
}



/**
 * Translate a source object into executable code.
 *
 * @param _labels labels inherited from the caller's context. Used only
 *                for compiling interpret instruction code.
 *
 * @return The compile code object.
 */
RexxCode *LanguageParser::translate(RexxDirectory *_labels)
{
    // create a stack frame so errors can display the parsing location.
    ParseActivationFrame frame(ActivityManager::currentActivity, this);

    // set up the package global defaults
    package->digits = Numerics::DEFAULT_DIGITS;
    package->form = Numerics::DEFAULT_FORM;
    package->fuzz = Numerics::DEFAULT_FUZZ;
    package->traceSetting = DEFAULT_TRACE_SETTING;
    package->traceFlags = RexxActivation::default_trace_flags;

    /* go translate the lead block       */
    RexxCode *newMethod = this->translateBlock(_labels);
    // we save this in case we need to explicitly run this at install time
    package->setInitCode(newMethod);
    // we might have directives to process, which adds additional stuff
    // to the package.
    if (!atEnd())                  /* have directives to process?       */
    {
        // we store a lot of stuff in the package object we're building.  Have
        // it set up to receive that information.
        package->initializeForDirectives();
        // for us to manage the class dependencies
        classDependencies = new_directory();
        // no active class definition
        activeClass = OREF_NULL;

        // translation must have been stopped by a directive.  If this is an
        // interpret, this is an error.
        if (flags.set(interpret))
        {
            // step to the next clause to report the error
            nextClause();
            syntaxError(Error_Translation_directive_interpret);
        }

        // now loop until we hit the end of the source processing directives.
        while (!atEnd())
        {
            directive();
        }
        // resolve any class dependencies
        resolveDependencies();
    }
    // we return the first section of code, but the package was updated with
    // all other pieces.
    return newMethod;
}


/**
 * Resolve dependencies between ::CLASS directives,
 * rearranging the order of the directives to preserve
 * relative ordering wherever possible.  Classes with no
 * dependencies in this source file will be done first,
 * followed by those with dependencies in the appropriate
 * order
 */
void LanguageParser::resolveDependencies()
{
    // get our class list
    if (package->classes->items() == 0)
    {
        // if the package doesn't have any classes, clear out the directory
        // so we don't have to carry this around.
        package->classes = OREF_NULL;
        return;
    }
    else                                 /* have classes to process           */
    {
        // get a local variable for easier processing
        RexxList *classes = package->classes;

        // create a directory for managing the dependencies between the classes
        RexxDirectory *classDependencies = new_directory();
        ProtectedObject p1(classDependencies);


        // run through the class list having each directive set up its
        // dependencies
        for (size_t i = classes->firstIndex(); i != LIST_END; i = classes->nextIndex(i))
        {
            ClassDirective *currentClass = (ClassDirective *)(classes->getValue(i));
            currentClass->addDependencies(classDependencies);
        }

        RexxList *classOrder = new_list();  // get a list for doing the order
        ProtectedObject p2(classOrder);

        // now we repeatedly scan the pending directory looking for a class
        // with no in-program dependencies - it's an error if there isn't one
        // as we build the classes we have to remove them (their names) from
        // pending list and from the remaining dependencies
        while (classes->items() > 0)
        {
            // this is the next one we process
            ClassDirective *nextInstall = OREF_NULL;
            for (i = classes->firstIndex(); i != LIST_END; i = classes->nextIndex(i))
            {
                // get the next directive
                ClassDirective *currentClass = (ClassDirective *)(classes->getValue(i));
                // if this class doesn't have any additional dependencies, pick it next.
                if (currentClass->dependenciesResolved())
                {
                    nextInstall = currentClass;
                    // add this to the class ordering
                    classOrder->append((RexxObject *)nextInstall);
                    // remove this from the processing list
                    classes->removeIndex(i);
                }
            }

            // if nothing was located during this pass, we must have circular dependencies
            // this is an error.
            if (nextInstall == OREF_NULL)
            {
                // directive line where we can give as the source of the error
                ClassDirective *errorClass = (ClassDirective *)(classes->getValue(classes->firstIndex()));
                clauseLocation = errorClass->getLocation();
                syntaxError(Error_Execution_cyclic, package->programName);
            }

            // ok, now go remove these from the dependencies
            RexxString *className = nextInstall->getName();

            // now go through the pending list telling each of the remaining classes that
            // they can remove this dependency from their list
            for (i = classes->firstIndex(); i != LIST_END; i = classes->nextIndex(i))
            {
                ClassDirective *currentClass = (ClassDirective *)classes->getValue(i);
                currentClass->removeDependency(className);
            }
        }

        // replace the original class list
        package->classes = classOrder;
    }

    // clear out any directories in the package that don't hold anything.
    package->clearEmptyDependencies();
}


/**
 * Flush an pending instructions from the control stack
 * for a new added instruction.
 *
 * @param _instruction
 *               The newly added instruction.
 */
void LanguageParser::flushControl(RexxInstruction *_instruction)
{
    // loop through the control stack
    for (;;)
    {
        // get the type of the instruction at the top of the control
        // stack.
        InstructionKeyword type = topDoType();   /* get the instruction type          */
        // is this a pending ELSE clause?    */
        if (type == KEYWORD_ELSE)
        {
            // pop the instruction off of the stack
            RexxInstruction *second = popDo();
            // and create a new end marker for that instruction.
            RexxInstruction second = endIfNew((RexxInstructionIf *)second);
            // have an instruction to add?
            if (_instruction != OREF_NULL)
            {
                // add to the current location and don't process any additional
                // instructions.
                addClause(_instruction);
                _instruction = OREF_NULL;
            }
            // now add the else terminator behind this.
            addClause(second);
            // we can go around again on this one.
        }
        // nested IF-THEN situation?
        else if (type == KEYWORD_IFTHEN || type == KEYWORD_WHENTHEN)
        {
            // get the top item
            RexxInstruction *second = popDo();
            // have an instruction to add?
            if (_instruction != OREF_NULL)
            {
                // insert this here and null it out.
                addClause(_instruction);
                _instruction = OREF_NULL;
            }
            // we need a new end marker
            second = endIfNew((RexxInstructionIf *)second);
            // we add this clause behine the new one, and also add this
            // to the control stack as a pending instruction
            this->addClause(second);
            this->pushDo(second);

            // we're done with this
            break;
        }
        // some other type of construct.  We just add the instruction to the
        // execution stream
        else
        {
            if (_instruction != OREF_NULL)
            {
                addClause(_instruction);
            }
            // all done flushing
            break;
        }
    }
}


/**
 * Translate a block of REXX code (delimited by possible
 * directive instructions
 *
 * @param _labels A parent label context (generally only used for
 *                interpret instructions).
 *
 * @return A RexxCode object for this block.
 */
RexxCode *LanguageParser::translateBlock(RexxDirectory *_labels )
{
    RexxInstruction *_instruction;        /* created instruction item          */
    RexxInstruction *second;             /* secondary clause for IF/WHEN      */
    RexxToken       *token;              /* current working token             */
    size_t           type;               /* instruction type information      */
    size_t           controltype;        /* type on the control stack         */

    // initialize the parsing environment.
    first = OREF_NULL;
    last = OREF_NULL;
    // get a list of all calls that might need resolution
    calls = new_list()
    // a table of variables...starting with the special variables we allocated space for.
    variables = (RexxDirectory *)TheCommonRetrievers->copy();
    // restart the variable index        */
    variableIndex = FIRST_VARIABLE_INDEX;
    exposedVariables = new_directory();

    // is this an interpret instruction?  use the provided labels
    if (flags.test(interpret))
    {
        labels = _labels;
    }
    // we need to keep a new labels directory
    else
    {
        labels = new_directory();
    }
    // until we need guard variables, we don't need the table
    guardVariables = OREF_NULL

    // clear the stack accounting fields
    maxStack = 0;
    currentStack = 0;
    // we're not at the end yet
    flags.reset(noClause);

    // add a dummy instruction at the front.  All other instructions get chained off of this.
    _instruction = new RexxInstruction(OREF_NULL, KEYWORD_FIRST);
    // this is the bottom of the control stack, and also the
    // first clause of the code stream.
    pushDo(_instruction);
    addClause(_instruction);

    // time to start actual parsing.  Continue until we reach the end
    nextClause();
    for (;;)
    {
        // start with on instruction
        _instruction = OREF_NULL;
        // At this point, we want to consume any label clauses, since they are
        // not real instructions.
        while (!(flags.test(noClause))
        {
            // resolve this clause into an instruction
            _instruction = instruction();
            // if nothing is returned, this must be a directive, which terminates
            // parsing of this block.
            if (_instruction == OREF_NULL)
            {
                break;
            }
            // not a label, break out of the loop
            if (!_instruction->isType(KEYWORD_LABEL))
            {
                break;
            }
            // append the label and try again
            addClause(_instruction);
            nextClause();
            // need to zero this out in case we break the loop for an end of file
            _instruction = OREF_NULL;
        }
        // ok, have we hit the end of the file or the end of the block?
        if (flags.test(noClause) || _instruction == OREF_NULL)
        {
            // see what we have at the top of the control stack, we probably
            // have some cleanup to do.
            InstructionKeyword controltype = topDoType();
            // handle any then or when terminators
            while (controltype == KEYWORD_ENDTHEN || controltype == KEYWORD_ENDWHEN)
            {
                // pop these from the stack and flush and pending control instructions
                popDo();
                flushControl(OREF_NULL);
                controltype = topDoType();
            }
            // In theory, at this point we should be at the first instruction.
            // if not, we have an unclosed block instruction, which is an error
            if (topDoIsType(KEYWORD_FIRST))
            {
                blockSyntaxError(topDo());
            }
            // remove the top instruction from the stack, and we're done
            popDo();
            break;
        }

        // now check if we need to adjust the control stack for this new instruction.
        InstructionKeyword type = _instruction->getType();
        // if this is not an ELSE, we might have a pending THEN to finish.
        if (type != KEYWORD_ELSE)
        {
            // get the type at the type of the stack.
            InstructionKeyword controltype = topDoType();
            // we might need to pop off multiple pending thens while end of an IF or WHEN
            while (controltype == KEYWORD_ENDTHEN || controltype == KEYWORD_ENDWHEN)
            {
                popDo();
                flushControl(OREF_NULL);
                controltype = topDoType();
            }
        }

        // now check the actual disposition of this instruction.  If it is a control type
        // add it immediately to the stream
        if (type == KEYWORD_IF || type == KEYWORD_SELECT || type == KEYWORD_DO || type == KEYWORD_LOOP)
        {
            addClause(_instruction);
        }
        // if this is an ELSE, we don't add a new level, but rather flush
        // any pending control levels.
        else if (type != KEYWORD_ELSE)
        {
            this->flushControl(_instruction);
        }
        // validate allowed instructions in a SELECT
        if (topDoType(KEYWORD_SELECT) &&
            (type != KEYWORD_WHEN && type != KEYWORD_OTHERWISE && type != KEYWORD_END ))
        {
            syntaxError(Error_When_expected_whenotherwise, topDo());
        }

        // now handle the different instructions to ensure they
        // are valid in their particular contexts.
        switch (type)
        {
            // a WHEN clause.  The top of the Do stack must be a select.
            case KEYWORD_WHEN:
            {
                // the top of the queue must be a SELECT instruction
                RexxInstruction *second = topDo();
                if (!second->isType(KEYWORD_SELECT))
                {
                    syntaxError(Error_Unexpected_when_when);
                }
                // let the select know that another WHEN was added
                ((RexxInstructionSelect *)second)->addWhen((RexxInstructionIf *)_instruction);
                // just fall into IF logic
            }

            // processing of an IF instruction, and also a WHEN (from above)
            case  KEYWORD_IF:
            {
                // we need to finish the IF instruction.
                // get the next token
                RexxToken *token = nextReal();
                // Did the line end with no THEN?  It must be on the next
                // line,
                if (token->isEndOfClause())
                {
                    // get the next full clause, which should start with
                    // the THEN keyword.
                    // did we hit the end of file?, this is an error
                    if (!nextClause())
                    {
                        syntaxError(Error_Then_expected_if, _instruction);
                    }

                    // now check the next token and ensure it is a THEN keyword.
                    token = nextReal();
                    // Not a THEN keyword?  This is an error
                    if (token->keyword() != KEYWORD_THEN)
                    {
                        syntaxError(Error_Then_expected_if, _instruction);
                    }
                    // create a new then clause attached to the IF
                    RexxInstruction *second = thenNew(token, (RexxInstructionIf *)_instruction);
                    // now get the next token.. and ensure is something after the THEN
                    token = nextReal();

                    // if this is a clause end (e.g, end of line), parse off a clause
                    // to make sure there is something there
                    if (token->isEndOfClause())
                    {
                        if (!nextClause())
                        {
                            syntaxError(Error_Incomplete_do_then, _instruction);
                        }
                    }
                    else
                    {
                        // step back a token and trim the THEN off of the clause.
                        previousToken();
                        trimClause();
                    }
                }
                // we have THEN on the same line as the IF
                else
                {
                    // attach a THEN to the IF
                    RexxInstruction *second = thenNew(token, (RexxInstructionIf *)_instruction);
                    // there might not be anything after the THEN, so we have to check

                    token = nextReal();
                    // if the THEN was the last thing, ensure we have something on the
                    // next line
                    if (token->isEndOfClause())
                    {
                        // we must have a clause, else there is an error
                        if (!nextClause())
                        {
                            syntaxError(Error_Incomplete_do_then, _instruction);
                        }
                    }
                    else
                    {
                        // ok, back up one token and trim the clause to remove the THEN
                        previousToken();
                        trimClause();
                    }
                }

                // add this to the instruction list and also add this to the
                // DO stack in case there is an ELSE to process.
                addClause(second);
                pushDo(second);
                // back to the top to get a new instruction
                continue;
            }

            // we have an ELSE instruction.  Need to verify this is in a correct context.
            case  KEYWORD_ELSE:
            {
                // ok, the top instruction is the key.  It must be the
                // tail end of a THEN instruction that we can attach to.
                second = topDo();
                if (!second->isType(KEYWORD_ENDTHEN))
                {
                    syntaxError(Error_Unexpected_then_else);
                }

                // ok, add the ELSE to the instruction list, pop the
                // THEN off of the control stack, push the ELSE on to the stack,
                // and hook them together.  The ELSE will need to be completed
                // once the next instruction has been parsed off.
                addClause(_instruction);
                second = popDo();
                // this is the ELSE
                pushDo(_instruction);
                // join the THEN and ELSE together
                ((RexxInstructionElse *)_instruction)->setParent((RexxInstructionEndIf *)second);
                ((RexxInstructionEndIf *)second)->setEndInstruction((RexxInstructionEndIf *)_instruction);

                // check for a dangling ELSE now.
                token = nextReal();

                // if the next token is the end of the line (or potentially, a semicolon)
                // we're not at the end.
                if (token->isEndOfClause())
                {
                    // dangling ELSE if we can't get another clause.
                    if (!nextClause())
                    {
                        syntaxError(Error_Incomplete_do_else, _instruction);
                    }
                }
                // ELSE followed by an instruction on the same line.  We need to
                // push the token we grabbed back on to the clause and trim off the
                // ELSE part we've already process.
                else
                {
                    previousToken();
                    trimClause();
                }
                // continue parsing instructions.
                continue;
            }


            // found the start of an OTHERWISE group.  We need to verify that
            // we're really working on a SELECT.
            case  KEYWORD_OTHERWISE:
            {
                // we must have a SELECT at the top of the control stack
                second = topDo();
                if (!second->isType(KEYWORD_SELECT))
                {
                    syntaxError(Error_Unexpected_when_otherwise);
                }

                // hook the otherwise up to the SELECT and push the otherwise
                // on to the top of the stack until we find an END.
                ((RexxInstructionSelect *)second)->setOtherwise((RexxInstructionOtherwise *)_instruction);
                pushDo(_instruction);

                // we could have the OTHERWISE on the same line as its following instruction, so
                // we need to trim the instruction
                token = nextReal();
                if (!token->isEndOfClause())
                {
                    previousToken();
                    trimClause();
                    // we have a clause, go back around and process this
                    continue;
                }
                // on the next line, fall through to the bottom of the loop
                // to get the next clause.
                break;
            }

            // An END instruction.  This could the the closure for a DO, LOOP, or SELECT.
            // its matchup should be on the top of the control stack.
            case  KEYWORD_END:
            {
                // ok, pop the top instruction.  If this is the
                // correct type, we're finished with this.  If not the
                // correct type, we have an error.
                second = popDo();
                type = second->getType();
                // verity the type
                if (type != KEYWORD_SELECT && type != KEYWORD_OTHERWISE && type != KEYWORD_DO && type != KEYWORD_LOOP)
                {
                    // we have a couple of specific errors based on what sort of instruction is
                    // on the top.
                    if (type == KEYWORD_ELSE)
                    {
                        syntaxError(Error_Unexpected_end_else);
                    }
                    else if (type == KEYWORD_IFTHEN || type == KEYWORD_WHENTHEN)
                    {
                        /* this is a different error         */
                        syntaxError(Error_Unexpected_end_then);
                    }
                    else
                    {
                        syntaxError(Error_Unexpected_end_nodo);
                    }
                }

                // ok, we have something good, now there is some additional
                // processing required for specific block instructions

                // if the top of the stack is an OTHERWISE, then the
                // END is really closing the SELECT.  Replace the Otherwise
                // with the Select.
                if (type == KEYWORD_OTHERWISE)
                {
                    second = popDo();
                }

                // Now do the apprpriate closure action based on the instruction type.
                if (second->isType(KEYWORD_SELECT))
                {
                    ((RexxInstructionSelect *)second)->matchEnd((RexxInstructionEnd *)_instruction, this);
                }
                else                           /* must be a DO block                */
                {
                    ((RexxInstructionDo *)second)->matchEnd((RexxInstructionEnd *)_instruction, this);
                }

                // We've just completed a large block instruction.  It is possible that
                // this was part of something like "if a then do;..end", in which case, the
                // conditional instruction is still in a pending state.  Flush out the control
                // stack to complete now that the block has been closed.
                flushControl(OREF_NULL);
                break;
            }

            // start of new DO group (also picks up LOOP instruction)
            // this gets pushed on to the top of the control stack until
            // we find an END instruction.
            case  KEYWORD_DO:
            case  KEYWORD_LOOP:
            {
                pushDo(_instruction);
                break;
            }

            // new select group.  Again, we push this on the start of the stack
            // while it awaits its associated WHEN, OTHERWISE, and END bits.
            case  KEYWORD_SELECT:
            {
                pushDo(_instruction);
                break;
            }

            // all other types of instructions don't require additional processing.
            default:
                break;
        }
        // grab another clause and go around.
        nextClause();
    }

    // ok, we have a stack of pending call/function calls to handle.
    // now that we've got all of the labels scanned off, we can figure out
    // what sort of targets these calls will resolve to.
    _instruction = (RexxInstruction *)(calls->removeFirst());

    // loop through the entire call list
    while (_instruction != (RexxInstruction *)TheNilObject)
    {
        // function calls are expression objects, while CALLs
        // are instructions. Similar, but have different
        // processing methods
        if (isOfClass(FunctionCallTerm, _instruction))
        {
            ((RexxExpressionFunction *)_instruction)->resolve(labels);
        }
        else
        {
            // ok, technically, this could be either a CALL or a SIGNAL.
            ((RexxInstructionCallBase *)_instruction)->resolve(labels);
        }
        _instruction = (RexxInstruction *)(calls->removeFirst());
    }

    // the first instruction is just a dummy we use to anchor
    // everything will parsing.  We can unchaind that now.
    first = first->nextInstruction;
    // if this code block does not contain labels (pretty common if
    // using an oo style), get rid of those too
    if (labels != OREF_NULL && labels->isEmpty())
    {
        labels = OREF_NULL;
    }
    // now create a code object that is attached to the package.
    // this will have all of the information needed to execute this code.
    return new RexxCode(package, first, labels, (maxStack + 10), variableIndex);
}

/**
 * Test if a named variable has been previously
 * exposed.
 *
 * @param varName The variable name.
 *
 * @return True if the variable has been exposed, false otherwise.
 */
bool LanguageParser::isExposed(RexxString *varName)
{
    return exposedVariables != OREF_NULL && exposedVariables->fastAt(varName) != OREF_NULL;
}

/**
 * Perform a variable capture operation if we're
 * evaluating a GUARD WHEN expression.
 *
 * @param varname   The name of the variable
 * @param retriever The associated variable retriever.
 */
void LanguageParser::captureGuardVariable(RexxString *varname, RexxVariableBase *retriever)
{
    // are we collecting guard variable references (usually the case when
    // parsing a GUARD WHEN expression.
    if (capturingGuardVariables())
    {
        // in the context of this method, we have a list of exposed variables.
        // if this is one of the exposed variables, then also add this to the guard
        // variables list so we know which variables will force a re-evaluation.
        if (isExposed(varname))
        {
            // add to the guard list
            guardVariables->put((RexxObject *)retriever, (RexxObject *)retriever);
        }
    }
}


/**
 * Resolve a variable name to a single common retriever object
 * per code block.
 *
 * @param varname The name of the target variable.
 *
 * @return A retrieve object for accessing the variable.  This
 *         will identify a variable slot in the code stack frame.
 */
RexxVariableBase *LanguageParser::addVariable(RexxString *varname)
{
    // we might have this already (fairly common in most programs).  If
    // not we cache a new one for the next time.
    RexxVariableBase *retriever = (RexxVariableBase *)variables->fastAt(varname);
    if (retriever == OREF_NULL)          /* not in the table yet?             */
    {
        // ok, have to create a new one.

        // if we're in normal operation, we allocate a slot in the stack frame
        // and tie a variable reference to that slot.  If this is an interpret,
        // then the variable value must be resolved dynamically.
        if (!isInterpret())
        {
            variableIndex++;
            retriever = new RexxParseVariable(varname, variableIndex);
        }
        else
        {
            // a slot index of zero tells the retriever to perform a dynamic
            // lookup
            retriever = new RexxParseVariable(varname, 0);
        }
        // and add this to the table.
        variables->put((RexxObject *)retriever, varname);
    }
    // Small optimization here.  In order for a variable to be added to the
    // guard variable list, it must have been exposed already.  That means
    // the variable will already be in this table, so we don't need to
    // perform the capturing test then.
    else
    {
        captureGuardVariable(varname, retriever);
    }
    // return the variable accesser, either a new one or one pulled from the cache.
    return retriever;
}


/**
 * Add a stem variable instance to the execution instance.
 * This is like the addVariable, but the retriever
 * in question is for a Stem variable.
 *
 * @param stemName The name of the stem (including the period).
 *
 * @return A retriever for the specified variable.
 */
RexxStemVariable *LanguageParser::addStem(RexxString *stemName)
{
    // like with normal variables, we might have this already cached.
    RexxStemVariable *retriever = (RexxStemVariable *)(variables->fastAt(stemName));
    if (retriever == OREF_NULL)
    {
        if (!isInterpret())
        {
            // non-interpret uses an allocated stack frame slot
            variableIndex++;
            retriever = new RexxStemVariable(stemName, variableIndex);
        }
        else
        {
            // interpret uses dynamic lookup
            retriever = new RexxStemVariable(stemName, 0);
        }
        variables->put((RexxObject *)retriever, stemName);
    }
    // Small optimization here.  In order for a variable to be added to the
    // guard variable list, it must have been exposed already.  That means
    // the variable will already be in this table, so we don't need to
    // perform the capturing test then.
    else
    {
        captureGuardVariable(stemName, retriever);
    }
    return retriever;                    /* return variable accesser          */
}



/**
 * Add a compound variable reference to our variable tables.
 * This is a two part operation...first we make sure the
 * stem variable is added, which uses a specific slot.  Then
 * we use that information to create a retriever for
 * the specific compound variable.
 *
 * @param name   The name of the compound variable.
 *
 * @return A retriever for this variable.
 */
RexxCompoundVariable *LanguageParser::addCompound(RexxString *name)
{
    // we cache compound variables also...see if we've encountered
    // this exact name before.
    RexxCompoundVariable *retriever = (RexxCompoundVariable *)(variables->fastAt(name));
    if (retriever \= OREF_NULL)
    {
        return retriever;
    }

    // we need to start out by scanning the name to separate the
    // stem and tail portions of the variable name.
    size_t length = name->getLength();
    const char *_position = name->getStringData();
    const char *start = _position;
    const char *end = _position + length;

    // we know this is a compound, so there must be at least one period.
    while (*_position != '.')
    {
        _position++;
    }

    // create a Rexx string version of the stem name and retrieve this
    // from the
    RexxString *stemName = new_string(start, _position - start + 1);
    RexxStemVariable *stemRetriever = this->addStem(stemName);

    ProtectedObject p(stemRetriever);

    // now split the tail piece into its component parts so that
    // we can optimize construction of the final tail lookup.
    tailCount = 0;
    do
    {
        // we're here because we just saw a previous period.  that's either the
        // stem variable period or the last tail element we processed.
        // either way, we step past it.  If this period is a trailing one,
        // we'll add a null tail element, which is exactly what we want.
        _position++;
        start = _position;

        // scan for the next period
        while (_position < end)
        {
            if (*_position == '.')         // found the next one?
            {
                break;                      // stop scanning now
            }
            _position++;                   // continue looking
        }

        // we found a tail piece.  Now figure out if this is
        // something that cannot be a variable.  Also make sure
        // this is part of the common string cache.
        RexxString *tail = commonString(new_string(start, _position - start));
        if (!(tail->getLength() == 0 || (*start >= '0' && *start <= '9')))
        {
            // this is a real variable piece.  Add this to the global
            // variable table and push its retriever on to the term stack
            // for safe keeping.
            subTerms->push((RexxObject *)(addVariable(tail)));
        }
        else
        {
            // this is a constant string value.  We can evaluate this
            // directly, so push it directly on to the stack.
            subTerms->push(tail);
        }
        // make sure we count the number of tails
        tailCount++;
    } while (_position < end);
    // This will pull the items off of the term stack and populate its internal
    // tail table for lookup processing.
    RexxCompoundVariable *retriever = new (tailCount) RexxCompoundVariable(stemName, stemRetriever->index, subTerms, tailCount);
    // add this to the retriever table so that all references to a compound variable with the
    // same full name will resolved to the same retriever.  We're safe just using the
    // variables table for this.
    variables->put((RexxObject *)retriever, name);

    // NOTE: compound variables do get get added to the guard list.
    return retriever;
}


/**
 * Add a variable name to the list of variables exposed
 * by a method.
 *
 * @param name   The name of the variable.
 */
void LanguageParser::expose(RexxString *name )
{

    exposedVariables->put(name, name);
}


/**
 * Compress all string tokens needed by a group of programs into
 * a single, common set of strings.  It is fairly common
 * for a program to have duplicate versions of strings
 * in the form of variable names, etc.  This ensures there
 * is just a single version of these available.
 *
 * @param string The string to add to the set.
 *
 * @return The commonized string.  If this string is already
 *         in the common table, the previously added one will be
 *         returned.
 */
RexxString *LanguageParser::commonString(RexxString *string)
{
    // check the global table first for this value.
    RexxString *result = (RexxString *)strings->fastAt(string);
    // if not in the table, we add this new one, otherwise we
    // return the table version
    if (result != OREF_NULL)
    {
        return result;
    }
    strings->put(string, string);
    return string;
}


/**
 * Handle adding a variable to a program.  This verifies that
 * the token is a valid variable, then creates a retriever that
 * can be used in an expression to obtain the variable value.
 *
 * @param token  The token holding the variable name.
 *
 * @return A retriever object that can be used as an expression
 *         term.
 */
RexxObject *LanguageParser::addVariable(RexxToken *token)
{
    // we first validate that this token represents a valid variable,
    needVariable(token);
    // then create the text retriever object.
    return addText(token);
}

// generate a retriever for a specific token type.  Note that we
// keep two caches of retrievers here.  There are tokens that true
// literals (strings, dot-variables, etc) and variable text tokens.  The
// literals can be kept in a common set across the entire package and
// reused.  The variables, however, contain information that is unique
// to each code block within the package.  These need to be kept in a
// different list that is rebuilt in each code block.
RexxObject *LanguageParser::addText(RexxToken *token)
{
    // these should be text type tokens that have a real value.
    RexxString *name = token->value;

    // we might already have processed this before.
    // if not, we need to examine this and find the
    // most appropriate form.
    RexxObject *retriever = literals->fastAt(name);
    if (retriever != OREF_NULL)
    {
        return retriever;
    }

    // now switch on the major token class id.
    switch (token->classId)
    {
        // various categories of symbols.
        case TOKEN_SYMBOL:
        {
            // each symbol subtype requires a
            // different retrieval method
            switch (token->subclass)
            {
                // the dummy placeholder period and symbols
                // that start with a digit are just literal strings.
                case SYMBOL_DUMMY:
                case SYMBOL_CONSTANT:
                {
                    // if this is a pure integer value within the default
                    // digits, create an integer object
                    if (token->numeric == INTEGER_CONSTANT)
                    {
                        value = name->requestInteger(Numerics::DEFAULT_DIGITS);
                        // this should not happen, given we've already validated
                        // this, but belt and braces and all that...just
                        // stick with the string value if it does occur.
                        if (value == TheNilObject)
                        {
                            value = name;
                        }
                        else
                            // snip off the string number string
                            // value that was created when the
                            // integer value was created.  This
                            // is rarely used, but contributes
                            // to the saved program size. It will
                            // be rebuilt on demand if it is needed.
                            name->setNumberString(OREF_NULL);
                    }
                    else
                    {
                        // just use the string value, but also try to create and
                        // attach the string's numeric value.
                        value = name;
                        name->setNumberString((RexxObject *)value->numberString());
                    }
                    // and stash the retriever value so we can resolve this if used again.
                    literals->put(value, name);
                    // strings and integers work directly as expression terms, so we can
                    // just return this directly.
                    return value;
                    break;
                }

                // simple variable.
                case SYMBOL_VARIABLE:
                {
                    // do the variable resolution
                    return (RexxObject *)addVariable(name);
                    break;
                }

                // stem variable, handled much like simple variables.
                case SYMBOL_STEM:
                {
                    return (RexxObject *)addStem(name);
                    break;
                }

                // compound variable...need to chop this up into its
                // component pieces.
                case SYMBOL_COMPOUND:
                {
                    return (RexxObject *)addCompound(name);
                    break;
                }

                // this is a non-numeric symbol that starts with a dot.  These
                // are treated as environment symbols.
                case SYMBOL_DOTSYMBOL:
                {
                    // create the shorter name and add to the common set
                    value = name->extract(1, name->getLength() - 1);
                    value = commonString(name->extract(1, name->getLength() - 1));
                    // create a retriever for this using the shorter name.
                    retriever = (RexxObject *)new RexxDotVariable((RexxString *)value);
                    literals->put(retriever, name);
                    return retriever;
                    break;
                }
            }
            break;
        }

        // just a straight literal string
        case TOKEN_LITERAL:
        {
            // strings are their own expression retrievers, so just add
            // this to the table and return it directly
            literals->put(name,  name);
            return name;
            break;
        }
    }

    // not a token type that can have a retriever
    return OREF_NULL;
}


/**
 * Build a retriever for a string name.  This will be
 * a version that uses dynamic lookup for variables rather
 * than using pre-assigned slots.  Generally used for
 * dynamic lookups such as from the variable pool interface.
 *
 * @param name   The variable name.
 *
 * @return A retriever object for looking up this value.
 */
RexxVariableBase *LanguageParser::getRetriever(RexxString *name)
{
    // first validate that this is a symbol and get the type of symbol.
    switch (name->isSymbol())
    {
        // TODO:  make this constants an enum type.

        // simple variable name
        case STRING_NAME:
            return (RexxVariableBase *)new RexxParseVariable(name, 0);
            break;

        // stem name.
        case STRING_STEM:
            return (RexxVariableBase *)new RexxStemVariable(name, 0);
            break;

        // compound name...more complicated.
        case STRING_COMPOUND_NAME:
            return (RexxVariableBase *)RexxVariableDictionary::buildCompoundVariable(name, true);
            break;

        default:
            // this is invalid and a syntax error
            syntaxError(Error_Translation_invalid_attribute, name);
    }
    return OREF_NULL;
}



/**
 * Add a new instruction to the code execution chain.  Instructions
 * are a linear chain of instruction objects, with all
 * branching handled by additional links within instructions.
 *
 * @param _instruction
 *               The new instruction to add.
 */
void LanguageParser::addClause(RexxInstruction *_instruction)
{
    // is this the first one in the chain?
    if (first == OREF_NULL)
    {
        // we keep track of both the first and last members
        // of the chain.
        first = _instruction;
        last =  _instruction;
    }
    // adding on to the chain.  We just need to chain these and
    // update the last pointer
    else
    {
        last->setNext(_instruction);
        last = _instruction);
    }

    // we add the instruction objects to the global table to
    // protect them from garbage collection until they are protected...
    // we can remove this now.
    toss((RexxObject *)_instruction);
}


/**
 * Add a label to our global table.  Note, in Rexx it is
 * not an error to have duplicate labels, but ony the
 * first can be used as a target.  We do not overwrite
 * a given name if it is already in the table.
 *
 * @param label     The label instruction to add.
 * @param labelname The label name.
 */
void LanguageParser::addLabel(RexxInstruction *label, RexxString *labelname )
{
    if (labels->fastAt(labelname) == OREF_NULL)
    {
        labels->put((RexxObject *)label, labelname);
    }
}


/**
 * Locate a label for a name match.
 *
 * @param labelname The label name.
 *
 * @return The label instruction, or OREF_NULL if the label doesn't exist
 */
RexxInstruction *LanguageParser::findLabel(RexxString *labelname)
{
    // it's possible we don't have a label table.
    if (labels != OREF_NULL)
    {
        return(RexxInstruction *)labels->fastAt(labelname);
    }
    return OREF_NULL;
}


/**
 * We're about to process a WHEN expression on a GUARD
 * instruction, so we need to turn on variable capture
 * so we know what exposed variables are referenced in the
 * expression.
 */
void LanguageParser::setGuard()
{
    // create the guard variable table if it is not already created...
    // it would be unusual for this to already exist.
    if (guardVariables == OREF_NULL)
    {
        guardVariables = new_identity_table();
    }
}


/**
 * Complete guard variable collection and return the table of
 * variable names.
 *
 * @return An array of the guard variable names.
 */
RexxArray *LanguageParser::getGuard()
{
    // TODO:  Might want to check if the variable retrievers might be a better
    // option here.

    // get the indices as an array of names.
    RexxArray *guards = guardVariables->makeArray();
    // turn off collection by tossing the table.
    guardVariables = OREF_NULL;
    return guards;
}

/**
 * Parse out a "constant" expression for a Rexx instruction.
 * a constant expression is a literal string, a constant
 * symbol, or an expression enclosed in parentheses.  Most
 * new instructions use constant expressions, since they
 * are not terminated by instruction subkeywords, and thus
 * more immune to breakage when new options are added.
 *
 * @return An object that can be used to evaluate this option
 *         expression.
 */
RexxObject *LanguageParser::constantExpression()
{
    // everthing keys off of the first token.
    RexxToken *token = nextReal();
    // just a literal token?
    if (token->isLiteral())              /* literal string expression?        */
    {
        // process this as text and return.  This is automatically
        // protected from GC.
        return addText(token);
    }
    // could be a constant symbol like a number or an environment variable
    // like .nil or .true.
    else if (token->isConstant())
    {
        return addText(token);
    }
    // is this missing entirely?
    else if (token->isEndOfClause())
    {
        // we don't raise an error here because we don't know that instruction
        // or context where this is needed.  Just back up one token and return
        // a NULL value.
        previousToken();
        return OREF_NULL;
    }
    // only other option here is an expression in parens.  If this
    // is not there, raise an error (this is a generic error, so
    // we can do this here.
    else if (!token->isType(TOKEN_LEFT))
    {
        syntaxError(Error_Invalid_expression_general, token);
    }
    else
    {
        // parse our a subexpression, terminating on the end of clause or
        // a right paren (the right paren is actually the required terminator)
        RexxObject *exp = this->subExpression(TERM_EOC | TERM_RIGHT);
        // now verify that the terminator token was a right paren.  If not,
        // issue an error message using the original opening token so we know
        // which one is an issue.
        if (!nextToken()->isType(TOKEN_RIGHT))
        {
            syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
        }
        // protect the expression from GC and return it.
        holdObject(exp);
        return exp;
    }
    return OREF_NULL;
}

/**
 * Evaluate a "constant" expression for REXX instruction keyword
 * values.  A constant expression is a literal string, constant
 * symbol, or an expression enclosed in parentheses.  The
 * expression inside parens can be a complex logical expression.
 * That is, something like (a = b, c = d), which acts like a
 * logical AND with evaluation shortcutting.
 *
 * @return A parsed out expression.
 */
RexxObject *LanguageParser::constantLogicalExpression()
/* Function:  Evaluate a "constant" expression for REXX instruction keyword   */
/*            values.  A constant expression is a literal string, constant    */
/*            symbol, or an expression enclosed in parentheses.  The          */
/*            expression inside parens can be a complex logical expression.   */
/******************************************************************************/
{
    // This is very similar to constant expression, until we get to the
    // expression inside the parents.
    RexxToken *token = nextReal();
    if (token->isLiteral())
    {
        return addText(token);
    }
    else if (token->isConstant())
    {
        _return addText(token);
    }
    else if (token->isEndOfClause())
    {
        previousToken();
        return OREF_NULL;
    }
    else if (!token->isType(TOKEN_LEFT))
    {
        syntaxError(Error_Invalid_expression_general, token);
    }
    else
    {
        // we use a different method for parsing the expression that knows
        // about the logical lists
        RexxObject *expression = parseLogical(token, TERM_EOC | TERM_RIGHT);
        // now verify that the terminator token was a right paren.  If not,
        // issue an error message using the original opening token so we know
        // which one is an issue.
        if (!nextToken()->isType(TOKEN_RIGHT))
        {
            syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
        }
        // protect the expression from GC and return it.
        holdObject(exp);
        return exp;
    }
    return OREF_NULL;
}


/**
 * Evaluate a "parenthetical" expression for REXX instruction
 * values.  A parenthetical expression is an expression enclosed
 * in parentheses.
 *
 * @param start  The starting token of the expression.  This will be
 *               the opening paren and is used for error reporting.
 *
 * @return The parsed expression.
 */
RexxObject *LanguageParser::parenExpression(RexxToken *start)
{
    // NB, the opening paren has already been parsed off and is in
    // the start token, which we only need for error reporting.

    RexxObject *exp = subExpression(TERM_EOC | TERM_RIGHT);

    // this must be terminated by a right paren
    if (!nextToken()->isType(TOKEN_RIGHT))
    {
        syntaxErrorAt(Error_Unmatched_parenthesis_paren, start);
    }

    // protect the expression and return it.
    holdObject(exp);
    return exp;
}

/**
 * Parse off an expression, stopping when one of the possible set
 * of terminator tokens is reached.  The terminator token is
 * placed back on the token queue.
 *
 * @param terminators
 *               An int containing the bit flag versions of the terminators.
 *
 * @return An executable expression object.
 */
RexxObject *LanguageParser::expression(int terminators)
{
    // get the first real token.  This will skip over all of the
    // white space, comments, etc. to get to the real meat of the expression.
    // we then back up so that the real expression processing will see that as
    // the first token.
    nextReal();
    previousToken();
    // and go parse this.
    return subExpression(terminators);
}


/**
 * Parse off a sub- expression, stopping when one of the possible
 * set of terminator tokens is reached.  The terminator token is
 * placed back on the token queue.  This is generally
 * called in the context of parsing different bits of
 * an expression and is frequently called recursively.
 *
 * @param terminators
 *               an int containing flags that indicate what terminators
 *               should be used to stop parsing this subexpression.
 *
 * @return
 */
RexxObject *LanguageParser::subExpression(int terminators )
{
    // generally, expressions proceed with term-operator-term, with various modifications.
    // start by processing of a term value.
    RexxObject *left = messageSubterm(terminators);
    RexxObject *right = OREF_NULL;

    // hmmm, nothing found, so we're done.
    if (left == OREF_NULL)
    {
        return OREF_NULL;
    }

    // we keep both a stack of expression terms and one of operators and process
    // things based on precedence.

    // pus the term to the stack
    pushTerm(left);

    // add a fence value to the operator stack to wall us off from other
    // active subexpressions.
    pushOperator((RexxToken *)TheNilObject);
    // now see what is next.
    RexxToken *token = nextToken();
    // loop until we find a terminator in our current token
    while (!token->isTerminator(terminators))
    {
        switch (token->type())
        {
            // either a single or double message send operation.
            // this uses a term from the stack as a target, and can possibly
            // have an argument list attached.
            case  TOKEN_TILDE:
            case  TOKEN_DTILDE:
            {
                // there must be a left term here to act as the target
                left = requiredTerm(token);

                // create a message term from the target...this also figures out
                // the message name and any arguments.
                RexxObject *subexpression = message(left, token->isType(TOKEN_DTILDE), terminators);
                // that goes back on the term stack
                pushTerm(subexpression);
                break;
            }

            // a left square bracket.  This will turn into a "[]" message.
            case  TOKEN_SQLEFT:
            {
                // the term is required
                left = requiredTerm();
                // this is a message to the left term, and may (ok, probably) have
                // arguments as well
                RexxObject *subexpression = collectionMessage(token, left, terminators);
                // and this goes back on to the term stack
                pushTerm(subexpression);
                break;
            }

            // we have a symbol, which may be a variable, a literal string, or
            // an open paren, which may be the start of a parenthetical sub-expression
            // because we're looking for an operator at this point, this is actually
            // an implied abuttal operation.
            case  TOKEN_SYMBOL:
            case  TOKEN_LITERAL:
            case  TOKEN_LEFT:
            {
                // get the token location.  We're going to turn this into a zero-length
                // token for the abuttal operation.
                SourceLocation location = token->getLocation();
                location.setEnd(location.getLineNumber(), location.getOffset());
                // create a dummy token, push the abuttal token back on the queue, and fall
                // through to the next section where the blank concatenate operator is handled.
                token = new RexxToken (TOKEN_OPERATOR, OPERATOR_ABUTTAL, OREF_NULLSTRING, location);
                previousToken();
            }
            // NOTE:  The above section falls through to this piece

            // A blank within an expression.  This is a concatenate (although, if we've
            // fallen through from the section above, this is really an abuttal operation)
            case  TOKEN_BLANK:
            {
                // this potentially could be a blank before one of the terminators (for example,
                // a blank before a WHILE keyword on a LOOP.  This is the same with abuttal.
                // We need to check for the terminators, and if we get a hit, we're done here.
                 RexxToken *second = nextReal();
                 // not a real operator if adjacent to a terminator
                 if (second->isTerminator(terminators))
                 {
                     break;
                 }
                 // back up to the operator token and fall through to the operator logic.
                 // this will then sort things out based on the sub type of the token.
                 previousToken();
            }

            // we have an operator of some sort (including the blank and abuttal concatenates)
            // At this point, these will be dyadic operators, so we need to screen out the
            // the not operator, which can only be a prefix one.
            case  TOKEN_OPERATOR:
            {
                // check for the backslash, which is a problem here.
                if (token->isSubtype(OPERATOR_BACKSLASH))
                {
                    syntaxError(Error_Invalid_expression_general, token);
                }

                // we now need to handle operator precedence with anything we have on the stack
                for (;;)
                {
                    // get the top of the operator stack.  If we hit the fence, we've
                    // unwound any pending operations to the beginning, and we can quit this
                    // loop.
                    RexxToken *second = topOperator();
                    if (second == (RexxToken *)TheNilObject)
                    {
                        break;
                    }

                    // check the token precedence,  If our new operator has higher
                    // precedenc, we're done popping.
                    if (token->precedence() > second->precedence())
                    {
                        break;
                    }

                    // these are both required terms here.
                    right = requiredTerm(token);
                    left = requiredTerm(token);

                    // pop off the top operator, and push a new term on the stack to
                    // replace this.
                    RexxToken *op = popOperator();
                    pushTerm((RexxObject *)new RexxBinaryOperator(op->subType(), left, right));
                }

                // finished popping lower precedence items.  Now push this operator on
                // to the stack.  We need to evaluate its right-hand side before this can be handled.
                pushOperator(token);
                // evaluate a right-hand side to this expression.
                right = messageSubterm(terminators);
                // Ok, we need a right term here, unless this is actually a blank operator.
                // in theory, we've already handled that possibility, but it doesn't hurt
                // to check.
                if (right == OREF_NULL && !token->isSubtype(OPERATOR_BLANK))
                {
                    syntaxError(Error_Invalid_expression_general, token);
                }
                // add the term to the stack.  We can handle this once the
                // operator gets popped off of the stack.
                pushTerm(right);
                break;
            }

            // this one of the assignment operators, out of location.  This is just
            // reported as an error here.
            case TOKEN_ASSIGNMENT:
            {
                syntaxError(Error_Invalid_expression_general, token);
                break;
            }

            // found a comma in an expression.  In all contexts where this
            // is allowed, this should be handled as a subexpression terminator.
            // In all other cases, this is an error.
            case TOKEN_COMMA:
            {
                syntaxError(Error_Unexpected_comma_comma);
                break;
            }

            // We've encountered a right paren.  Like the comma, if this is
            // expected, it acts as a terminator.  This is a stray and must
            // be reported.
            case TOKEN_RIGHT:
            {
                syntaxError(Error_Unexpected_comma_paren);
                break;
            }

            // another good character in a bad location.
            case TOKEN_SQRIGHT:
            {
                syntaxError(Error_Unexpected_comma_bracket);
                break;
            }

            // something unexpected that we have no specific error for.
            default:
            {
                syntaxError(Error_Invalid_expression_general, token);
                break;
            }
        }

        // ok, grab the next token and keep processing.
        token = nextToken();
    }

    // we've hit a terminator for this subexpression.  We probably
    // have some pending operators on the operator stack that we need
    // to handle.  These will be in proper precedence order, so we just
    // need to pop of the operators and terms and handle them.
    token = popOperator();

    // keep popping until we hit our fence value
    while (token != (RexxToken *)TheNilObject)
    {
        // we need two terms
        right = requiredTerm(token);
        left = requiredTerm(token);

        // all of these operators are binaries, and get pushed back on the stack
        pushTerm((RexxObject *)new RexxBinaryOperator(token->subType(), left, right));
        // and pop another operator.
        token = popOperator();
    }

    // our final subexpression is at the top of the term stack, so pop it
    // off and return.
    return popTerm();
}

/**
 * Parse off multiple arguments for a some sort of
 * call, function, or method invocation, returning them
 * as an array.
 *
 * @param firstToken The first token of the expresssion.
 * @param terminators
 *                   The set of terminators that will end the list.
 *
 * @return An array object holding the argument expressions.
 */
RexxArray *LanguageParser::argArray(RexxToken *firstToken, int terminators )
{
    // scan a set of arguments until we hit our terminator (likely to be
    // a ')' or ']'.  Arguments are delimited by ','
    size_t argCount = argList(firstToken, terminators);

    // The arguments are pushed on to the term stack.  We need to allocate
    // an array and copy them into the array
    RexxArray *args = new_array(argCount);

    // arguments are pushed on to the term stack in reverse order, so
    // we fill from the end.  Note that missing arguments are
    // just pushed on to the stack as OREF_NULL.
    while (argCount > 0)
    {
        args->put(subTerms->pop(), argCount--);
    }

    return args;
}

/**
 * Perform the parsing of a list of arguments where each
 * argument is separated by commas.  Each argument expression
 * is pushed on to the term stack in last-to-first order,
 * with omitted arguments represented by OREF_NULL.
 *
 * @param firstToken The first token of the expression list, which is
 *                   really the delimiter that opens the list.
 * @param terminators
 *                   The list of terminators for this expression type.
 *
 * @return The count of argument expressions we found.  Note that
 *         an empty expression following a final "," does not
 *         count as a real argument.
 */
size_t LanguageParser::argList(RexxToken *firstToken, int terminators )
{
    size_t realcount = 0;            // real count is arguments up to the last real one
    size_t total = 0;                // total is the full count of arguments we attempt to parse.
    RexxToken *terminatorToken;      // the terminator token that ended a sub expression

    // we need to skip ahead to the first real token, then backup one to be
    // properly positioned for the start.  If this is a CALL instruction, this
    // will skip the blank between the CALL keyword and the start of the expression,
    // which counts as a significant blank by the tokenizer.
    nextReal();
    previousToken();

    // now loop until we get a terminator.  Since we're processing an argument
    // list here, we add in the COMMA to whatever terminators we've been handed.
    for (;;)
    {
        // parse off an argument expression
        RexxObject *subExpr = this->subExpression(terminators | TERM_COMMA);
        // We have two term stacks.  The main term stack is used for expression evaluation.
        // the subTerm stack is used for processing expression lists like this.
        subTerms->push(subExpr);

        // now check the total.  Real count will be the last
        // expression that requires evaluation.
        total++;
        // if we got something back, this is our new real total.
        if (subExpr != OREF_NULL)
        {
            realcount = total;
        }

        // the next token will be our terminator.  If this is not
        // a comma, we have more expressions to parse.
        terminatorToken = nextToken();
        if (!terminatorToken->isType(TOKEN_COMMA))
        {
            break;
        }
    }


    // if this is a function or method invocation, we're expecting this list to be
    // ended by a right parent.  firstToken gives the position of the missing
    // left paren.
    if (terminators & TERM_RIGHT && !token->isType(TOKEN_RIGHT))
    {
        syntaxErrorAt(Error_Unmatched_parenthesis_paren, firstToken);
    }

    // same deal with square brackets, different error message.
    if (terminators&TERM_SQRIGHT && !token->isType(TOKEN_SQRIGHT))
    {
        syntaxErrorAt(Error_Unmatched_parenthesis_square, firstToken);
    }

    // all arguments are on the subterm stack now.  If we had trailing
    // commas on the list, pop off any of the extras
    while (total > realcount)
    {
        subTerms->pop();
        total--;
    }
    // return the count of real argument expressions.
    return realcount;
}

/**
 * Parse off a function call and create an expression
 * object that can process the call.
 *
 * @param token  The token that marks the start of the argument list.
 * @param name   The function name token.
 * @param terminators
 *               The list of terminators for parsing the argument list.
 *
 * @return A function expression object that can invoke this function.
 */
RexxObject *LanguageParser::function(RexxToken *token, RexxToken *name, int terminators )
{
    // parse off the argument list, leaving the arguments in the subterm stack.
    // NOTE:  we turn off the SQRIGHT terminator because this function might be
    // invoked from inside of a [] message argument list.  We don't want to get
    // the nesting wrong.
    size_t argCount = argList(token, ((terminators | TERM_RIGHT) & ~TERM_SQRIGHT));

    // create a function item.  This will also pull the argument items from the
    // subterm stack
    RexxObject *func = (RexxObject *)new (argCount) RexxExpressionFunction(name->value(), argCount,
        subTerms, name->builtin(), name->isLiteral());

    // at this point, we can't resolve the final target of this call.  It could be
    // a builtin, a call to an internal label, or an external call.  We'll resolve this
    // once we've finished this code block and have all of the labels.
    addReference(func);
    return func;
}


/**
 * Parse a collection message expression term.  This is
 * if the form term[args], and follows much the same
 * parsing rules as function calls.
 *
 * @param token  The token that starts the argument list (the square bracket)
 * @param target The target for the message term.
 * @param terminators
 *               Our terminators.
 *
 * @return A message expression object.
 */
RexxObject *LanguageParser::collectionMessage(RexxToken *token, RexxObject *target, int terminators )
{
    // this was popped from the term stack, so we need to give is a little protection
    // until we're done parsing.
    ProtectedObject p(target);

    // TODO:  Need to revisit terminators here in light of the question about
    // parentheticals and keyword terminators.  I suspect we should not pass these
    // along, but rather just say what is needed for THIS piece.

    // get the arguments.  Like with builtin function calls, we need to turn off the
    // right paren from the terminator list to keep the nesting right.
    size_t argCount = argList(token, ((terminators | TERM_SQRIGHT) & ~TERM_RIGHT));

    // create the message item.
    RexxObject *msg = (RexxObject *)new (argCount) RexxExpressionMessage(target, (RexxString *)OREF_BRACKETS,
        (RexxObject *)OREF_NULL, argCount, subTerms, false);
    // give this a little short-term GC protection.
    holdObject(msg);
    return msg;
}


/**
 * Get a token from the clause, combining terminator checks
 * and error reporting if we don't get a terminator.
 *
 * @param terminators
 *                  The terminators for the current context.
 * @param errorcode The error to issue if no token is found.  The error code
 *                  is optional, and if not specified, OREF_NULL is returned.
 *
 * @return The parsed off token.
 */
RexxToken  *LanguageParser::getToken(int terminators, int errorcode)
{
    // get the next token and perform the terminator checks.  If we were
    // terminated, issue an error message if requested.
    RexxToken *token = nextToken();
    if (token->isTerminator(terminators))
    {
        if (errorcode != 0)
        {
            syntaxError(errorcode);
        }
        // just return a null if not requested to issue an error.
        return OREF_NULL;
    }
    return token;
}


/**
 * Parse a message send term within an expression.
 *
 * @param target The target object term for the message send.
 * @param doubleTilde
 *               Indicates whether this is the "~" or "~~" form of operatior.
 * @param terminators
 *               Expression terminators.
 *
 * @return An object to execute the message send.
 */
RexxObject *LanguageParser::message(RexxObject *target, bool doubleTilde, int terminators )
{
    size_t        argCount;              /* list of function arguments        */
    RexxString   *messagename = OREF_NULL;  /* message name                      */
    RexxObject   *super;                 /* super class target                */
    RexxToken    *token;                 /* current working token             */
    RexxExpressionMessage *_message;     /* new message term                  */


    // this message might have a superclass override...default is none.
    RexxObject *super = OREF_NULL;
    // no arguments yet
    size_t argCount = 0;

    // add the term to the term stack so that stacksize calculations
    // include this in the processing.  This has the side effect of
    // protecting this object from GC while we're parsing.
    this->pushTerm(target);
    // ok, we're expecting a message name next, go get one.
    RexxToken *token = getToken(terminators, Error_Symbol_or_string_tilde);
    // this must be a string or symbol
    if (!token->isSymbolOrLiteral())
    {
        syntaxError(Error_Symbol_or_string_tilde);
    }

    // we have a message name
    RexxString *messagename = token->value();

    // ok, what else do we have.  Nothing additional is required
    // here, but this could be an argument list, or a superclass
    // override.
    token = getToken(terminators);
    if (token != OREF_NULL)
    {
        // an override is marked with a colon.
        if (token->isType(TOKEN_COLON))
        {
            // ok, if we have the colon, then we need an override spec.
            token = getToken(terminators, Error_Symbol_expected_colon);
            // this must be a variable or a dot symbol.
            if (!token->isVariableOrDot())
            {
                syntaxError(Error_Symbol_expected_colon);
            }

            // the super class is either a variable lookup or a dot variable, so
            // we need a retriever.
            super = addText(token);
            // get the next token to check for an argument list.
            token = getToken(terminators);
            // NOTE:  We fall through from here to the code
            // that follows the checks for the superclass override to
            // the code that checks for the argument list.
        }
    }

    // ok, we might have an argument list here.
    if (token != OREF_NULL)
    {
        // this is only interesting if the text token is an open paren directly
        // abutted to the name.  Process that as an argument list for the final
        // created message object
        if (token->isType(TOKEN_LEFT))
        {
            argCount = argList(token, ((terminators | TERM_RIGHT) & ~TERM_SQRIGHT));
        }
        else
        {
            previousToken();                 /* something else, step back         */
        }
    }

    // got all of the pieces, now create the message object and give it some short term
    // protection from GC.
    RexxObject *msg =  (RexxObject *)new (argCount) RexxExpressionMessage(target, messagename, super, argCount, subTerms, doubleTilde);
    holdObject(msg);
    // now safe to pop the message target object we saved.
    popTerm();
    return msg;
}


/**
 * Parse off a single variable symbol or a message term that
 * can be used for an assignment.
 *
 * NOTE:  If this is a message term, then the message term
 * will be configured as an assignment term.
 *
 * @return The object for an assignment target, or OREF_NULL if something
 *         other than a variable or a message term was found.  On return,
 *         the clause position pointer will either be unchanged or
 *         positioned at the next token of the clause.
 */
RexxObject *LanguageParser::variableOrMessageTerm()
{
    // try for a message term first.  If not successful, see if the
    // next token is a variable symbol.
    RexxObject *result = messageTerm();
    if (result == OREF_NULL)
    {
        RexxToken *_first = nextReal();
        if (_first->isSymbol())
        {
            // ok, add the variable to the processing list
            this->needVariable(_first);
            result = this->addText(_first);
        }
        else
        {
            previousToken();     // just push back on for the caller to sort out
        }
    }
    else
    {
        // we need to convert this into an assignment message.
        ((RexxExpressionMessage *)result)->makeAssignment(this);
    }
    return result;
}



/**
 * Parse off an instruction leading message term element. This
 * is an important step, as it is used to determine what sort of
 * instruction we have.  If the instruction starts with a
 * message term, then this is a messaging instruction.  Part of
 * the instruction type determination at the beginning of each
 * clause.
 *
 * @return The message term execution element.
 */
RexxObject *LanguageParser::messageTerm()
{
    // save the current position so we can reset cleanly
    size_t mark = markPosition();

    // get the first message term
    RexxObject *start = subTerm(TERM_EOC);
    // save this on the term stack
    pushTerm(start);

    RexxObject *term = OREF_NULL;         // an allocated message term
    RexxToken *token = nextToken();

    // the leading message term can be a cascade of messages, so
    // keep processing things until we hit some other type of operation.
    while (token~isMessageOperator())
    {
        // we need to perform the the message operation on this term to create
        // an new term which will be the target of the next one.
        // this could be a bracket lookup, which is a collection message.
        if (token~isType(TOKEN_SQLEFT))
        {
            term = collectionMessage(token, start, TERM_EOC);
        }
        else
        {
            // normal message twiddle message
            term = message(start, token~isType(TOKEN_DTILDE), TERM_EOC);
        }
        // this is the target for the next one in the chain.
        // remove the previous one from the term stack and push a new
        // one on.
        start = term;
        popTerm();
        pushTerm(start);
        // and see if we have another.
        token = nextToken();
    }

    // found a token that was not a message send type, so push it back.
    previousToken();
    // if we never created a message term from this, we need to take another
    // path.  Roll the position back to where we started and return.
    if (term == OREF_NULL)
    {
        resetPosition(mark);
    }
    // we still have either the starting term or our last created message
    // term on the stack...we can remove this now.
    popTerm();

    // ok, this is either OREF_NULL to indicate we found nothing,
    // or the last message term we ended up creating.
    return term;
}


/**
 * Handle a potential message subterm in the context of an
 * expression.  This is similar to messageTerm(), but this
 * occurs after we've already determined we need an expression
 * and need an expression term.
 *
 * @param terminators
 *               Terminators for this subexpression context.
 *
 * @return An object to represent this message term.  Returns
 *         OREF_NULL if no suitable term is found.
 */
RexxObject *LanguageParser::messageSubterm(int terminators)
{
    // get the first token.  If we've hit a terminator here, this could be
    // the real end of the expression.  The caller context will figure out
    // how to handle that.
    RexxToken *token = nextToken();
                                         /* this the expression end?          */
    if (token->isTerminator(terminators))
    {
        return OREF_NULL;                  /* nothing to do here                */
    }

    // is this an operator?  I this context, it will need to be a prefix operator.
    if (token->isOperator())
    {
        // prefix operators are handled as expression terms, although they themselves
        // require a term.
        switch (token->subtype())
        {
            // we only recognize 3 prefix operators, and they are handled in pretty
            // much the same way.
            case OPERATOR_PLUS:
            case OPERATOR_SUBTRACT:
            case OPERATOR_BACKSLASH:
            {
                // we need a term as a target. this is an error
                // if we don't find anything we can use.  We just recurse
                // on this, which might find a chain of prefix operators (except,
                // as I'm sure Walter Pachl will be sure to point out, --, which
                // will be treated as a line comment :-)
                RexxObject *term = messageSubterm(terminators);
                if (term == OREF_NULL)
                {
                    syntaxError(Error_Invalid_expression_prefix, token);
                }
                // create a new unary operator using the subtype code.
                return (RexxObject *)new RexxUnaryOperator(token->subtype(), term);
                break;
            }

            // other operators are invalid
            default:
                syntaxError(Error_Invalid_expression_general, token);
        }
    }

    // ok, not an operator character.  Now we need to figure out what sort of
    // term element is here.
    else
    {
        // put the token back and try parsing off a subTerm (smaller subset, no
        // prefix stuff allowed).
        previousToken();
        RexxObject *term = subTerm(terminators);
        // protect on the term stack
        pushTerm(term);
        // ok, now see if this is actually a message send target by checking
        // the next token
        token = nextToken();

        // TODO:  Old code had some bugs in with respect to keyword terminators
        // because it was not passing along the terminator types.  For example,
        // do i = -UNTIL .true would not recognize the UNTIL as a terminator because
        // the prefix code is not passing along the terminator checks. Similarly,
        // DO i = foo~WHILE will fail for the same reason.

        // we can have a long cascade of message sends.  For expression syntax,
        // this all one term (just like nested function calls would be).
        while (token~isMessageOperator())
        {
            // we have two possibilities here, a bracket message or a twiddle form.
            if (token~isType(TOKEN_SQLEFT))
            {
                term = collectionMessage(token, term, terminators);
            }
            else
            {
                term = this->message(term, classId == TOKEN_DTILDE, terminators);
            }
            popTerm();
            pushTerm(term);
            // message cascades are considered part of the same expression term.
            token = nextToken();
        }
        // back up to the token that stopped the loop
        previousToken();
        // pop our term from the stack and return the final version.
        popTerm()
        return term;
    }
}


/**
 * Parse off a subterm of an expression, from simple ones like
 * variable names and literals, to more complex such as message
 * sends.
 *
 * @param terminators
 *               The expression terminator context.
 *
 * @return An executable object for the message term.
 */
RexxObject *LanguageParser::subTerm(int terminators)
{
    // get the first token and make sure we really have something here.
    // The caller knows how to deal with a missing term.
    RexxToken *token = nextToken();
    if (token->isTerminator(terminators))
    {
        return OREF_NULL;
    }

    // ok, process based on the token category
    switch (token->type())
    {
        // parenthetical subexpression.  Note, we do not pass along our
        // terminators when requesting this, since the bracketing within the parens
        // will disambiguate keywords.  We only look for terminators and a right paren
        case  TOKEN_LEFT:
        {
            // parse off the parenthetical.  This might not return anything if there
            // are nothing in the parens.  This is an error.
            RexxObject *term = subExpression(TERM_RIGHT | TERM_EOC);
            if (term == OREF_NULL)
            {
                syntaxError(Error_Invalid_expression_general, token);
            }
            // this had better been terminated by a righ paren.
            if (!nextToken()->isType(TOKEN_RIGHT))
            {
                syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
            }
            // we're done.
            return term;
        }

        // literal or symbol.  These are generally pretty simple, but
        // we also have to account for function calls.
        case  TOKEN_SYMBOL:
        case  TOKEN_LITERAL:
        {
            // need to check if the next token is an open paren.  That turns
            // the symbol or literal token into a function invocation.
            RexxToken *second = nextToken();
            if (second->isType(TOKEN_LEFT))
            {
                return function(second, token);
            }
            else
            {
                // simple text token type...push the next token back
                // and go resolve how the token is handled.
                previousToken();
                return addText(token);
            }
            break;
        }

        // an operator token.  We do allow prefix operators here, others are
        // an error
        case  TOKEN_OPERATOR:
        {
             switch (token->type())
             {
                 // +, -, and logical NOT variants are permitted here...except
                 // we don't actually process them here, so back up and say we got nothing.
                 case OPERATOR_PLUS:
                 case OPERATOR_SUBTRACT:
                 case OPERATOR_BACKSLASH:
                     previousToken();
                     return OREF_NULL;

                 // other operators we can flag as an error now.
                 default:
                     syntaxError(Error_Invalid_expression_general, token);
             }
             break;
        }

        // a few error situations with specific error messages.
        case  TOKEN_RIGHT:
            syntaxError(Error_Unexpected_comma_paren);
            break;

        case  TOKEN_COMMA:
            syntaxError(Error_Unexpected_comma_comma);
            break;

        case  TOKEN_SQRIGHT:
            syntaxError(Error_Unexpected_comma_bracket);
            break;

        // ok, this is bad
        default:
            syntaxError(Error_Invalid_expression_general, token);
    }
    // never reach here.
    return OREF_NULL;
}

/**
 * Push a term on to the expression term stack.
 *
 * @param term   The term object.
 */
void LanguageParser::pushTerm(RexxObject *term )
{
    // push the term on to the stack.
    terms->push(term);

    // we keep track of how large the term stack gets during parsing.  This
    // tells us how much stack space we need to allocate at run time.
    currentStack++;
    maxStack = Numerics::maxVal(currentStack, maxStack);
}

/**
 * Pop a term off of the expression stack.
 *
 * @return The popped object.
 */
RexxObject *LanguageParser::popTerm()
{
    // reduce the stack count
    currentStack--;
    // pop the object off of the stack and give it some short-term
    // GC protection.
    RexxObject *term = terms->pop();
    holdObject(term);
    return term;
}

/**
 * Pop a term item off of the execution stack.  Will
 * raise an error if this is not there.
 *
 * @param token     The token giving the context location.  Used to
 *                  identify the error position.
 * @param errorCode The error code.  The default is the generic syntax error.
 *
 * @return The object popped off of the stack.
 */
RexxObject *LanguageParser::requiredTerm(RexxToken *token, int errorCode)
{
    // we track the size count when we push/pop
    currentStack--;
    // pop the term off of the stack
    term = terms->pop();
    // we need a term, if this is not here, this is a syntax error
    if (term == OREF_NULL)
    {
        syntaxError(errCode, token);
    }
    // give this term some short-term GC protection and return it.
    holdObject(term);
    return term;
}

/**
 * Pop multiple items off of the term stack.
 *
 * @param count  The count of items to pop.
 *
 * @return The last object popped.
 */
RexxObject *LanguageParser::popNTerms(size_t count)
{
    RexxObject *result = OREF_NULL;

    // reduce the stack size
    currentStack -= count;
    // and pop that many elements
    while (count--)
    {
        result = terms->pop();
    }
    // if we have a return item, protect it for a little while.
    if (result != OREF_NULL)
    {
        holdObject(result);
    }
    return result;                       /* and return it                     */
}

void LanguageParser::isExposeValid()
/******************************************************************************/
/* Function:  Validate placement of an EXPOSE instruction.  The EXPOSE must   */
/*            be the first instruction and this must not be an interpret      */
/*            invocation.  NOTE:  labels are not allowed preceeding, as that  */
/*            will give a target for SIGNAL or CALL that will result in an    */
/*            invalid EXPOSE execution.                                       */
/******************************************************************************/
{
    if (this->flags&_interpret)          /* is this an interpret?             */
    {
        /* give the interpret error          */
        syntaxError(Error_Translation_expose_interpret);
    }
    /* not the first instruction?        */
    if (this->last->getType() != KEYWORD_FIRST)
    {
        /* general placement error           */
        syntaxError(Error_Translation_expose);
    }
}

RexxArray  *LanguageParser::words(
    RexxString *string)                /* target string                     */
/******************************************************************************/
/* Function:  Break up a string into an array of words for parsing and        */
/*            interpretation.                                                 */
/******************************************************************************/
{
    RexxQueue  *wordlist;                /* created list of words             */
    RexxArray  *wordarray;               /* array version of the list         */
    RexxString *word;                    /* current word                      */
    size_t      count;                   /* count of words                    */
    size_t      i;                       /* loop counter                      */

    wordlist = this->subTerms;           /* use the subterms list             */
                                         /* get the first word                */
    word = ((RexxString *)(string->word(IntegerOne)))->upper();
    word = this->commonString(word);     /* get the common version of this    */
    wordlist->push(word);                /* add to the word list              */
    count = 1;                           /* one word so far                   */
                                         /* while still more words            */
    for (i = 3, word = (RexxString *)(string->word(IntegerTwo)); word->getLength() != 0; i++)
    {
        count++;                           /* have another word                 */
        word = this->commonString(word);   /* get the common version of this    */
        wordlist->push(word);              /* add this word to the list         */
                                           /* get the next word                 */
        word = (RexxString *)string->word(new_integer(i));
    }
    wordarray = new_array(count);        /* get an array return value         */
    while (count > 0)                    /* while more words                  */
    {
        /* copy into the array               */
        wordarray->put(wordlist->pop(), count--);
    }
    return wordarray;                    /* return as an array                */
}

void LanguageParser::errorCleanup()
/******************************************************************************/
/* Function:  Free up all of the parsing elements because of an error         */
/******************************************************************************/
{
  this->cleanup();                     /* do needed cleanup                 */
}

void LanguageParser::error(int errorcode)
/******************************************************************************/
/* Function:  Raise an error caused by source translation problems.           */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, OREF_NULL, OREF_NULL);
}

void LanguageParser::error(int errorcode, SourceLocation &location, RexxArray *subs)
/******************************************************************************/
/* Function:  Raise an error caused by source translation problems.           */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
  clauseLocation = location;           // set the error location
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, subs, OREF_NULL);
}

void LanguageParser::errorLine(
     int   errorcode,                  /* error to raise                    */
     RexxInstruction *_instruction)    /* instruction for the line number   */
/******************************************************************************/
/* Function:  Raise an error where one of the error message substitutions is  */
/*            the line number of another instruction object                   */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(new_integer(_instruction->getLineNumber())), OREF_NULL);
}

void LanguageParser::errorPosition(
     int        errorcode,             /* error to raise                    */
     RexxToken *token )                /* token value for description       */
/******************************************************************************/
/* Function:  Raise an error, displaying the location of a token associated   */
/*            with the error.                                                 */
/******************************************************************************/
{
  SourceLocation token_location = token->getLocation(); /* get the token location            */
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(new_integer(token_location.getOffset()), new_integer(token_location.getLineNumber())), OREF_NULL);
}

void LanguageParser::errorToken(
     int        errorcode,             /* error to raise                    */
     RexxToken *token )                /* token value for description       */
/******************************************************************************/
/* Function:  Raise an error, displaying the value of a token in the error    */
/*            message.                                                        */
/******************************************************************************/
{
    RexxString *value = token->value;                /* get the token value               */
    if (value == OREF_NULL)
    {
        switch (token->classId)
        {

            case TOKEN_BLANK:                /* blank operator                    */
                value = new_string(" ", 1);    /* use a blank                       */
                break;

            case TOKEN_EOC:                  /* source terminator                 */
                value = new_string(";", 1);    /* use a semicolon                   */
                break;

            case TOKEN_COMMA:                /* comma                             */
                value = new_string(",", 1);    /* display a comma                   */
                break;

            case TOKEN_LEFT:                 /* left parentheses                  */
                value = new_string("(", 1);    /* display that                      */
                break;

            case TOKEN_RIGHT:                /* right parentheses                 */
                value = new_string(")", 1);    /* display that                      */
                break;

            case TOKEN_SQLEFT:               /* left square bracket               */
                value = new_string("[", 1);    /* display that                      */
                break;

            case TOKEN_SQRIGHT:              /* right square bracket              */
                value = new_string("]", 1);    /* display that                      */
                break;

            case TOKEN_COLON:                /* colon                             */
                value = new_string(":", 1);    /* display that                      */
                break;

            case TOKEN_TILDE:                /* twiddle operator                  */
                value = new_string("~", 1);    /* display that                      */
                break;

            case TOKEN_DTILDE:               /* double twiddle operator           */
                value = new_string("~~", 2);   /* display that                      */
                break;

            case TOKEN_DCOLON:               /* double colon operator             */
                value = new_string("::", 2);   /* display that                      */
                break;

            default:                         /* ????? token                       */
                /* just use a null string            */
                value = (RexxString *)OREF_NULLSTRING;
                break;
        }
    }
    this->errorCleanup();                /* release any saved objects         */
                                         /* pass on the exception info        */
    ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(value), OREF_NULL);
}

void LanguageParser::error(
     int         errorcode,            /* error to raise                    */
     RexxObject *value )               /* value for description             */
/******************************************************************************/
/* Function:  Issue an error message with a single substitution parameter.    */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(value), OREF_NULL);
}

void LanguageParser::error(
     int         errorcode,            /* error to raise                    */
     RexxObject *value1,               /* first value for description       */
     RexxObject *value2 )              /* second value for description      */
/******************************************************************************/
/* Function:  Issue an error message with two substitution parameters.        */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(value1, value2), OREF_NULL);
}

void LanguageParser::error(
     int         errorcode,            /* error to raise                    */
     RexxObject *value1,               /* first value for description       */
     RexxObject *value2,               /* second value for description      */
     RexxObject *value3 )              /* third value for description       */
/****************************************************************************/
/* Function:  Issue an error message with three substitution parameters.    */
/****************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, new_array(value1, value2, value3), OREF_NULL);
}

void LanguageParser::blockError(
    RexxInstruction *_instruction )     /* unclosed control instruction      */
/******************************************************************************/
/* Function:  Raise an error for an unclosed block instruction.               */
/******************************************************************************/
{
    // get the instruction location and set as the current error location
    clauseLocation = this->last->getLocation();

    switch (_instruction->getType())
    {   /* issue proper message type         */
        case KEYWORD_DO:                   /* incomplete DO                     */
            /* raise an error                    */
            syntaxError(Error_Incomplete_do_do, _instruction);
            break;
        case KEYWORD_LOOP:                   /* incomplete LOOP                     */
            /* raise an error                    */
            syntaxError(Error_Incomplete_do_loop, _instruction);
            break;

        case KEYWORD_SELECT:               /* incomplete SELECT                 */
            syntaxError(Error_Incomplete_do_select, _instruction);
            break;

        case KEYWORD_OTHERWISE:            /* incomplete SELECT                 */
            syntaxError(Error_Incomplete_do_otherwise, _instruction);
            break;

        case KEYWORD_IF:                   /* incomplete IF                     */
        case KEYWORD_IFTHEN:               /* incomplete IF                     */
        case KEYWORD_WHENTHEN:             /* incomplete IF                     */
            syntaxError(Error_Incomplete_do_then, _instruction);
            break;

        case KEYWORD_ELSE:                 /* incomplete ELSE                   */
            syntaxError(Error_Incomplete_do_else, _instruction);
            break;
    }
}

size_t LanguageParser::processVariableList(
  int        type )                    /* type of instruction               */
/****************************************************************************/
/* Function:  Process a variable list for PROCEDURE, DROP, and USE          */
/****************************************************************************/
{
    RexxToken   *token;                  /* current working token             */
    int          list_count;             /* count of variables in list        */
    RexxObject  *retriever;              /* variable retriever object         */

    list_count = 0;                      /* no variables yet                  */
    token = nextReal();                  /* get the first variable            */

    /* while not at the end of the clause*/
    while (!token->isEndOfClause())
    {
        /* have a variable name?             */
        if (token->isSymbol())
        {
            /* non-variable symbol?              */
            if (token->subclass == SYMBOL_CONSTANT)
            {
                /* report the error                  */
                syntaxError(Error_Invalid_variable_number, token);
            }
            else if (token->subclass == SYMBOL_DUMMY)
            {
                /* report the error                  */
                syntaxError(Error_Invalid_variable_period, token);
            }
            retriever = this->addText(token);/* get a retriever for this          */
            this->subTerms->push(retriever); /* add to the variable list          */
            if (type == KEYWORD_EXPOSE)      /* this an expose operation?         */
            {
                this->expose(token->value);    /* add to the expose list too        */
            }
            list_count++;                    /* record the variable               */
        }
        /* have a variable reference         */
        else if (token->classId == TOKEN_LEFT)
        {
            list_count++;                    /* record the variable               */
            token = nextReal();              /* get the next token                */
                                             /* not a symbol?                     */
            if (!token->isSymbol())
            {
                /* must be a symbol here             */
                syntaxError(Error_Symbol_expected_varref);
            }
            /* non-variable symbol?              */
            if (token->subclass == SYMBOL_CONSTANT)
            {
                /* report the error                  */
                syntaxError(Error_Invalid_variable_number, token);
            }
            else if (token->subclass == SYMBOL_DUMMY)
            {
                /* report the error                  */
                syntaxError(Error_Invalid_variable_period, token);
            }

            retriever = this->addText(token);/* get a retriever for this          */
                                             /* make this an indirect reference   */
            retriever = (RexxObject *)new RexxVariableReference((RexxVariableBase *)retriever);
            this->subTerms->queue(retriever);/* add to the variable list          */
            this->currentstack++;            /* account for the varlists          */

            token = nextReal();              /* get the next token                */
            if (token->isEndOfClause()) /* nothing following?                */
            {
                /* report the missing paren          */
                syntaxError(Error_Variable_reference_missing);
            }
            /* must be a right paren here        */
            else if (token->classId != TOKEN_RIGHT)
            {
                /* this is an error                  */
                syntaxError(Error_Variable_reference_extra, token);
            }
        }
        /* something bad....                 */
        else
        {                             /* this is invalid                   */
            if (type == KEYWORD_DROP)        /* DROP form?                        */
            {
                /* give appropriate message          */
                syntaxError(Error_Symbol_expected_drop);
            }
            else                             /* else give message for EXPOSEs     */
            {
                syntaxError(Error_Symbol_expected_expose);
            }
        }
        token = nextReal();                /* get the next variable             */
    }
    if (list_count == 0)
    {               /* no variables?                     */
        if (type == KEYWORD_DROP)          /* DROP form?                        */
        {
            /* give appropriate message          */
            syntaxError(Error_Symbol_expected_drop);
        }
        else                               /* else give message for EXPOSEs     */
        {
            syntaxError(Error_Symbol_expected_expose);
        }
    }
    return list_count;                   /* return the count                  */
}

RexxObject *LanguageParser::parseConditional(
     int   *condition_type,            /* type of condition                 */
     int    error_message )            /* extra "stuff" error message       */
/******************************************************************************/
/* Function:  Allow for WHILE or UNTIL keywords following some other looping  */
/*            construct.  This returns SUBKEY_WHILE or SUBKEY_UNTIL to flag   */
/*            the caller that a conditional has been used.                    */
/******************************************************************************/
{
    RexxToken  *token;                   /* current working token             */
    int         _keyword;                /* keyword of parsed conditional     */
    RexxObject *_condition;              /* parsed out condition              */

    _condition = OREF_NULL;               /* default to no condition           */
    _keyword = 0;                         /* no conditional yet                */
    token = nextReal();                  /* get the terminator token          */

    /* real end of instruction?          */
    if (!token->isEndOfClause())
    {
        /* may have WHILE/UNTIL              */
        if (token->isSymbol())
        {
            /* process the symbol                */
            switch (token->subKeyword() )
            {

                case SUBKEY_WHILE:              /* DO WHILE exprw                    */
                    /* get next subexpression            */
                    _condition = this->parseLogical(OREF_NULL, TERM_COND);
                    if (_condition == OREF_NULL) /* nothing really there?             */
                    {
                        /* another invalid DO                */
                        syntaxError(Error_Invalid_expression_while);
                    }
                    token = nextToken();          /* get the terminator token          */
                                                  /* must be end of instruction        */
                    if (!token->isEndOfClause())
                    {
                        syntaxError(Error_Invalid_do_whileuntil);
                    }
                    _keyword = SUBKEY_WHILE;       /* this is the WHILE form            */
                    break;

                case SUBKEY_UNTIL:              /* DO UNTIL expru                    */
                    /* get next subexpression            */
                    /* get next subexpression            */
                    _condition = this->parseLogical(OREF_NULL, TERM_COND);

                    if (_condition == OREF_NULL)   /* nothing really there?             */
                    {
                        /* another invalid DO                */
                        syntaxError(Error_Invalid_expression_until);
                    }
                    token = nextToken();          /* get the terminator token          */
                                                  /* must be end of instruction        */
                    if (!token->isEndOfClause())
                    {
                        syntaxError(Error_Invalid_do_whileuntil);
                    }
                    _keyword = SUBKEY_UNTIL;       /* this is the UNTIL form            */
                    break;

                default:                        /* nothing else is valid here!       */
                    /* raise an error                    */
                    syntaxError(error_message, token);
                    break;
            }
        }
    }
    if (condition_type != NULL)          /* need the condition type?          */
    {
        *condition_type = _keyword;        /* set the keyword                   */
    }
    return _condition;                   /* return the condition expression   */
}


/**
 * Parse off a "logical list expression", consisting of a
 * list of conditionals separated by commas.
 *
 * @param terminators
 *               The set of terminators for this logical context.
 *
 * @return OREF_NULL if no expressions is found, a single expression
 *         element if a single expression is located, and a complex
 *         logical expression operator for a list of expressions.
 */
RexxObject *LanguageParser::parseLogical(RexxToken *_first, int terminators)
{
    size_t count = argList(_first, terminators);
    // arglist has swallowed the terminator token, so we need to back up one.
    previousToken();
    // let the caller deal with completely missing expressions
    if (count == 0)
    {
        return OREF_NULL;
    }

    // just a single item (common)?  Just pop the top item and return it.
    if (count == 1)
    {
        return subTerms->pop();
    }

                                       /* create a new function item        */
    return (RexxObject *)new (count) RexxExpressionLogical(this, count, this->subTerms);
}
