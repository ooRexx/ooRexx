
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
RexxCode *RexxSource::translateInterpret(RexxDirectory *_labels )
{
    // setup the environment
    initialize();
    // make sure interpret is set up.
    flags.set(interpret);

    // translate, using the inherited labels
    return translate(_labels);
}


RexxObject *RexxSource::toss(
    RexxObject *object)                /* object to "release"               */
/******************************************************************************/
/* Function:  Remove an object from the save list                             */
/******************************************************************************/
{
    /* have a real object                */
    if (object != OREF_NULL)
    {
        this->savelist->remove(object);    /* remove from the save table        */
        this->holdObject(object);          /* return this object as held        */
    }
    return object;                       /* return the object                 */
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
 * Test if a token qualifies as a terminator in the current
 * parsing context.
 *
 * @param terminators
 *               The list of terminators.
 * @param token  The test token.
 *
 * @return true if this is a terminator, false if it doesn't match
 *         one of the terminator classes.
 */
bool LanguageParser::terminator(int terminators,  RexxToken *token)
{
    // process based on terminator class
    switch (token->classId)
    {
        case  TOKEN_EOC:                     // end-of-clause is always a terminator
        {
            previousToken();
            return true;
        }
        case  TOKEN_RIGHT:                   // found a right paren
        {
            if (terminators&TERM_RIGHT)
            {
                previousToken();
                return true;
            }
            break;
        }
        case  TOKEN_SQRIGHT:                 // closing square bracket?
        {
            if (terminators&TERM_SQRIGHT)
            {
                previousToken();
                return true;
            }
            break;
        }
        case  TOKEN_COMMA:                   // a comma is a terminator in argument subexpressions
        {
            if (terminators&TERM_COMMA)
            {
                previousToken();
                return true;
            }
            break;
        }
        case  TOKEN_SYMBOL:                  // the token is a symbol...need to check on keyword terminators
        {
            // keyword terminators all set a special keyword flag.  We only check
            // symbols that are simple variables.
            if (terminators&TERM_KEYWORD && token->isSimpleVariable())
            {
                // map the keyword token to a key word code.  This are generally
                // keyword options on DO/LOOP, although THEN and WHEN are also terminators
                switch (this->subKeyword(token))
                {
                    case SUBKEY_TO:
                    {
                        if (terminators&TERM_TO)
                        {
                            previousToken();
                            return true;
                        }
                        break;
                    }
                    case SUBKEY_BY:
                    {
                         if (terminators&TERM_BY)
                         {
                             previousToken();
                             return true;
                         }
                         break;
                    }
                    case SUBKEY_FOR:
                    {
                        if (terminators&TERM_FOR)
                        {
                            previousToken();
                            return true;
                        }
                        break;
                    }
                    case SUBKEY_WHILE:           // a single terminator type picks up both
                    case SUBKEY_UNTIL:           // while and until
                    {
                        if (terminators&TERM_WHILE)
                        {
                            previousToken();
                            return true;
                        }
                        break;
                    }
                    case SUBKEY_WITH:            // WITH keyword in PARSE value
                    {
                        if (terminators&TERM_WITH)
                        {
                            previousToken();
                            return true;
                        }
                        break;
                    }
                    case SUBKEY_THEN:            // THEN subkeyword from IF or WHEN
                    {
                        if (terminators&TERM_THEN)
                        {
                            previousToken();
                            return true;
                        }
                        break;
                    }
                    default:                     // not a terminator type
                        break;
                }
            }
        }
        default:
            break;
    }

    return false;                    // no terminator found
}


/**
 * Advance the current position to the next line.
 */
void LanguageParser::nextLine()
{
    if (clause != OREF_NULL)       // do we have a clause object active?
    {
        // record the current position in the clause...this marks the location end.
        clause->setEnd(line_number, line_offset);
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

    // retrieve the line specifics from the source object
    source->getLine(line, current, currentLength);
}

/**
 * Extract a clause from the source and return as a clause
 * object.  The clause object contains a list of all of the
 * tokens contained within the clause and is used by
 * the parser to determine the type of instruction and
 * create the instruction parse tree.
 */
void LanguageParser::nextClause()
{
    RexxToken   *token;                  /* current token being processed     */
    SourceLocation location;             /* location of the clause            */
    SourceLocation token_location;       /* location of each token            */

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
            token = sourceNextToken(OREF_NULL);
            // OREF_NULL indicates we've hit the end of the source.  Mark us as
            // finished and return
            if (token == OREF_NULL)
            {
                flags.set(noClause);
                return;
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
        token_location = token->getLocation();
        location = token_location;
        // record this in the clause for potential error reporting.
        clause->setLocation(location);

        // now consume tokens until we find an end of clause marker.
        for (;;)
        {
            // get the next token.  Blanks can now be significant inside
            // a clause.
            token = sourceNextToken(token);
            // and save the position again.
            token_location = token->getLocation();
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
        location.setEnd(token_location);
        // and set this in the class to give it the full bounds.
        clause->setLocation(location);
    }

    // no reclaimed clause now, regardless of how we got here.
    flags.reset(reclaimed);
    // always set the error information for the clause being processed.
    clauseLocation = clause->getLocation();
}

RexxCode *RexxSource::translate(
    RexxDirectory *_labels)             /* interpret labels                  */
/******************************************************************************/
/* Function:  Translate a source object into a method object                  */
/******************************************************************************/
{
    ParseActivationFrame frame(ActivityManager::currentActivity, this);

    // set up the package global defaults
    digits = Numerics::DEFAULT_DIGITS;
    form = Numerics::DEFAULT_FORM;
    fuzz = Numerics::DEFAULT_FUZZ;
    traceSetting = DEFAULT_TRACE_SETTING;
    traceFlags = RexxActivation::default_trace_flags;

    /* go translate the lead block       */
    RexxCode *newMethod = this->translateBlock(_labels);
    // we save this in case we need to explicitly run this at install time
    OrefSet(this, this->initCode, newMethod);
    if (!this->atEnd())                  /* have directives to process?       */
    {
        /* create the routines directory     */
        OrefSet(this, this->routines, new_directory());
        /* create the routines directory     */
        OrefSet(this, this->public_routines, new_directory());
        /* and a directory of dependencies   */
        OrefSet(this, this->class_dependencies, new_directory());
        /* create the requires directory     */
        OrefSet(this, this->requires, new_list());
        // and a list of load libraries requiring loading.
        OrefSet(this, this->libraries, new_list());
        /* create the classes list           */
        OrefSet(this, this->classes, new_list());
        /* no active class definition        */
        OrefSet(this, this->active_class, OREF_NULL);
                                           /* translation stopped by a directive*/
        if (this->flags&_interpret)        /* is this an interpret?             */
        {
            this->nextClause();              /* get the directive clause          */
                                             /* raise an error                    */
            syntaxError(Error_Translation_directive_interpret);
        }
        /* create a directory for ..methods  */
        OrefSet(this, this->methods, new_directory());

        while (!this->atEnd())             /* loop until end of source          */
        {
            this->directive();               /* process the directive             */
        }
        this->resolveDependencies();       /* go resolve class dependencies     */
    }
    return newMethod;                    /* return the method                 */
}


void RexxSource::resolveDependencies()
/*********************************************************************/
/* Function:  Resolve dependencies between ::CLASS directives,       */
/*            rearranging the order of the directives to preserve    */
/*            relative ordering wherever possible.  Classes with no  */
/*            dependencies in this source file will be done first,   */
/*            followed by those with dependencies in the appropriate */
/*            order                                                  */
/*********************************************************************/
{
    // get our class list
    if (classes->items() == 0)           /* nothing to process?               */
    {
        /* clear out the classes list        */
        OrefSet(this, this->classes, OREF_NULL);
    }
    else                                 /* have classes to process           */
    {
        size_t i;
        // run through the class list having each directive set up its
        // dependencies
        for (i = classes->firstIndex(); i != LIST_END; i = classes->nextIndex(i))
        {
            /* get the next class                */
            ClassDirective *current_class = (ClassDirective *)(classes->getValue(i));
            // have the class figure out it's in-package dependencies
            current_class->addDependencies(class_dependencies);
        }

        RexxList *class_order = new_list();  // get a list for doing the order
        ProtectedObject p(class_order);

/* now we repeatedly scan the pending directory looking for a class         */
/* with no in-program dependencies - it's an error if there isn't one       */
/* as we build the classes we have to remove them (their names) from        */
/* pending list and from the remaining dependencies                         */
        while (classes->items() > 0)
        {
            // this is the next one we process
            ClassDirective *next_install = OREF_NULL;
            for (i = classes->firstIndex(); i != LIST_END; i = classes->nextIndex(i))
            {
                 /* get the next class                */
                ClassDirective *current_class = (ClassDirective *)(classes->getValue(i));
                // if this class doesn't have any additional dependencies, pick it next.
                if (current_class->dependenciesResolved())
                {
                    next_install = current_class;
                    // add this to the class ordering
                    class_order->append((RexxObject *)next_install);
                    // remove this from the processing list
                    classes->removeIndex(i);
                }
            }
            if (next_install == OREF_NULL)   /* nothing located?                  */
            {
                // directive line where we can give as the source of the error
                ClassDirective *error_class = (ClassDirective *)(classes->getValue(classes->firstIndex()));
                clauseLocation = error_class->getLocation();
                /* raise an error                    */
                syntaxError(Error_Execution_cyclic, this->programName);
            }
            RexxString *class_name = next_install->getName();

            // now go through the pending list telling each of the remaining classes that
            // they can remove this dependency from their list
            for (i = classes->firstIndex(); i != LIST_END; i = classes->nextIndex(i))
            {    /* go remove the dependencies        */
                 /* get a class                       */
                ClassDirective *current_class = (ClassDirective *)classes->getValue(i);
                current_class->removeDependency(class_name);
            }
        }

        /* replace the original class list   */
        OrefSet(this, this->classes, class_order);
        /* don't need the dependencies now   */
        OrefSet(this, this->class_dependencies, OREF_NULL);
    }

    if (this->requires->items() == 0)     /* nothing there?                    */
    {
        /* just clear it out                 */
        OrefSet(this, this->requires, OREF_NULL);
    }
    if (this->libraries->items() == 0)     /* nothing there?                    */
    {
        /* just clear it out                 */
        OrefSet(this, this->libraries, OREF_NULL);
    }
    if (this->routines->items() == 0)    /* no routines to process?           */
    {
        /* just clear it out also            */
        OrefSet(this, this->routines, OREF_NULL);
    }
    /* now finally the public routines   */
    if (this->public_routines->items() == 0)
    {
        /* just clear it out also            */
        OrefSet(this, this->public_routines, OREF_NULL);
    }
    if (this->methods->items() == 0)     /* and also the methods directory    */
    {
        /* just clear it out also            */
        OrefSet(this, this->methods, OREF_NULL);
    }
}


void RexxSource::flushControl(
    RexxInstruction *_instruction)      /* next instruction                  */
/******************************************************************************/
/* Function:  Flush any pending compound instructions from the control stack  */
/*            for new added instructions                                      */
/******************************************************************************/
{
    size_t           type;               /* instruction type                  */
    RexxInstruction *second;             /* additional created instructions   */

    /* loop through the control stack    */
    for (;;)
    {
        type = this->topDo()->getType();   /* get the instruction type          */
        /* pending ELSE close?               */
        if (type == KEYWORD_ELSE)
        {
            second = this->popDo();          /* pop the item off of the control   */
                                             /* create a new end marker           */
            second = this->endIfNew((RexxInstructionIf *)second);
            /* have an instruction?              */
            if (_instruction != OREF_NULL)
            {
                this->addClause(_instruction);  /* add this here                     */
                _instruction = OREF_NULL;       /* don't process more instructions   */
            }
            this->addClause(second);         /* add the clause to the list        */
        }
        /* nested IF-THEN situation?         */
        else if (type == KEYWORD_IFTHEN || type == KEYWORD_WHENTHEN)
        {
            second = this->popDo();          /* pop the item off of the control   */
            /* have an instruction?              */
            if (_instruction != OREF_NULL)
            {
                this->addClause(_instruction); /* add this here                     */
                _instruction = OREF_NULL;      /* don't process more instructions   */
                                               /* create a new end marker           */
                second = this->endIfNew((RexxInstructionIf *)second);
                this->addClause(second);       /* add the clause to the list        */
                this->pushDo(second);          /* add to the control stack too      */
            }
            else
            {
                /* create a new end marker           */
                second = this->endIfNew((RexxInstructionIf *)second);
                this->addClause(second);       /* add the clause to the list        */
                this->pushDo(second);          /* add to the control stack too      */
            }
            break;                           /* finish up here                    */
        }
        /* some other type of construct      */
        else
        {
            if (_instruction != OREF_NULL)    /* have an instruction?              */
            {
                this->addClause(_instruction);  /* add this here                     */
            }
            break;                           /* finished flushing                 */
        }
    }
}

RexxCode *RexxSource::translateBlock(
    RexxDirectory *_labels )            /* labels (for interpret)            */
/******************************************************************************/
/* Function:  Translate a block of REXX code (delimited by possible           */
/*            directive instructions                                          */
/******************************************************************************/
{
    RexxInstruction *_instruction;        /* created instruction item          */
    RexxInstruction *second;             /* secondary clause for IF/WHEN      */
    RexxToken       *token;              /* current working token             */
    size_t           type;               /* instruction type information      */
    size_t           controltype;        /* type on the control stack         */

                                         /* no instructions yet               */
    OrefSet(this, this->first, OREF_NULL);
    OrefSet(this, this->last, OREF_NULL);
    /* allocate the call list            */
    OrefSet(this, this->calls, new_list());
    /* create variables and lit tables   */
    OrefSet(this, this->variables, (RexxDirectory *)TheCommonRetrievers->copy());
    /* restart the variable index        */
    this->variableindex = FIRST_VARIABLE_INDEX;
    OrefSet(this, this->exposed_variables, new_directory());
    if (this->flags&_interpret)          /* this an interpret?                */
    {
        /* just use the existing label set   */
        OrefSet(this, this->labels, _labels);
    }
    else
    {
        /* create a new labels directory     */
        OrefSet(this, this->labels, new_directory());
    }
    /* not collecting guard variables yet*/
    OrefSet(this, this->guard_variables, OREF_NULL);
    this->maxstack = 0;                  /* clear all of the stack accounting */
    this->currentstack = 0;              /* fields                            */
    this->flags &= ~no_clause;           /* not reached the end yet           */

                                         /* add the first dummy instruction   */
    _instruction = new RexxInstruction(OREF_NULL, KEYWORD_FIRST);
    this->pushDo(_instruction);           /* set bottom of control stack       */
    this->addClause(_instruction);        /* add to the instruction list       */
    this->nextClause();                  /* get the next physical clause      */
    for (;;)                             /* process all clauses               */
    {
        _instruction = OREF_NULL;           /* zero the instruction pointer      */
        while (!(this->flags&no_clause))   /* scan through all labels           */
        {
            /* resolve the instruction type      */
            _instruction = this->instruction();
            if (_instruction == OREF_NULL)    /* found a directive clause?         */
            {
                break;                         /* return to higher level            */
            }
            /* is this a label?                  */
            if (_instruction->getType() != KEYWORD_LABEL)
            {
                break;                         /* have a non-label clause           */
            }
            this->addClause(_instruction);    /* add this to clause list           */
            this->nextClause();              /* get the next physical clause      */
            _instruction = OREF_NULL;         /* no instruction any more           */
        }
        /* get an end-of-clause?             */
        if (this->flags&no_clause || _instruction == OREF_NULL)
        {
            /* get the control stack type        */
            controltype = this->topDo()->getType();
            /* while end of an IF or WHEN        */
            while (controltype == KEYWORD_ENDTHEN || controltype == KEYWORD_ENDWHEN)
            {
                this->popDo();                 /* pop pending closing IFs           */
                this->flushControl(OREF_NULL); /* flush any IFs or ELSEs            */
                                               /* get the control stack type        */
                controltype = this->topDo()->getType();
            }
            /* any unclosed composite clauses?   */
            if (this->topDo()->getType() != KEYWORD_FIRST)
            {
                /* report the block error            */
                blockSyntaxError(this->topDo());
            }
            this->popDo();                   /* remove the top one                */
            break;                           /* done parsing this section         */
        }
        type = _instruction->getType();     /* get the top instruction type      */
        if (type != KEYWORD_ELSE)          /* have a pending THEN to finish     */
        {
            /* get the control stack type        */
            controltype = this->topDo()->getType();
            /* while end of an IF or WHEN        */
            while (controltype == KEYWORD_ENDTHEN || controltype == KEYWORD_ENDWHEN)
            {
                this->popDo();                 /* pop pending closing IFs           */
                this->flushControl(OREF_NULL); /* flush any IFs or ELSEs            */
                                               /* get the control stack type        */
                controltype = this->topDo()->getType();
            }
        }
        if (type == KEYWORD_IF || type == KEYWORD_SELECT || type == KEYWORD_DO || type == KEYWORD_LOOP)
        {
            this->addClause(_instruction);   /* add to instruction heap           */
        }
        else if (type != KEYWORD_ELSE)     /* not a new control level           */
        {
            this->flushControl(_instruction); /* flush any IFs or ELSEs            */
        }
        /* have a bad instruction within a   */
        /* SELECT instruction?               */
        if (this->topDo()->getType() == KEYWORD_SELECT &&
            (type != KEYWORD_WHEN && type != KEYWORD_OTHERWISE && type != KEYWORD_END ))
        {
            syntaxError(Error_When_expected_whenotherwise, this->topDo());
        }

        switch (type)                      /* post process the instructions     */
        {
            case KEYWORD_WHEN:               /* WHEN clause of SELECT             */
                second = this->topDo();        /* get the top of the queue          */
                                               /* not working on a SELECT?          */
                if (second->getType() != KEYWORD_SELECT)
                {
                    syntaxError(Error_Unexpected_when_when);
                }
                /* add this to the select list       */
                ((RexxInstructionSelect *)second)->addWhen((RexxInstructionIf *)_instruction);
                /* just fall into IF logic           */

            case  KEYWORD_IF:                /* start of an IF instruction        */
                token = nextReal();            /* get the terminator token          */
                                               /* have a terminator before the THEN?*/
                if (token->isEndOfClause())
                {
                    this->nextClause();          /* get the next physical clause      */
                    if (this->flags&no_clause)   /* get an end-of-file?               */
                    {
                        /* raise an error                    */
                        syntaxError(Error_Then_expected_if, _instruction);
                    }
                    token = nextReal();          /* get the first token               */
                                                 /* not a THEN keyword?               */
                    if (!token->isSymbol() || this->keyword(token) != KEYWORD_THEN)
                    {
                        /* have an error                     */
                        syntaxError(Error_Then_expected_if, _instruction);
                    }
                    /* create a new then clause          */
                    second = this->thenNew(token, (RexxInstructionIf *)_instruction);
                    token = nextReal();          /* get token after THEN keyword      */
                                                 /* terminator here?                  */
                    if (token->isEndOfClause())
                    {
                        this->nextClause();        /* get the next physical clause      */
                        if (this->flags&no_clause) /* get an end-of-file?               */
                        {
                            /* raise an error                    */
                            syntaxError(Error_Incomplete_do_then, _instruction);
                        }
                    }
                    else
                    {
                        previousToken();           /* step back a token                 */
                        trimClause();              /* make this start of the clause     */
                    }
                }
                else                           /* if expr THEN form                 */
                {
                    /* create a new then clause          */
                    second = this->thenNew(token, (RexxInstructionIf *)_instruction);
                    token = nextReal();          /* get token after THEN keyword      */
                                                 /* terminator here?                  */
                    if (token->isEndOfClause())
                    {
                        this->nextClause();        /* get the next physical clause      */
                        if (this->flags&no_clause) /* get an end-of-file?               */
                        {
                            /* raise an error                    */
                            syntaxError(Error_Incomplete_do_then, _instruction);
                        }
                    }
                    else
                    {
                        previousToken();           /* step back a token                 */
                        trimClause();              /* make this start of the clause     */
                    }
                }
                this->addClause(second);       /* add this to the instruction list  */
                this->pushDo(second);          /* add to top of control queue       */
                continue;                      /* straight around to process clause */
                                               /* remainder                         */
            case  KEYWORD_ELSE:              /* have an ELSE instruction          */
                second = this->topDo();        /* get the top block                 */
                if (this->topDo()->getType() != KEYWORD_ENDTHEN)
                {
                    /* have an error                     */
                    syntaxError(Error_Unexpected_then_else);
                }
                this->addClause(_instruction); /* add to instruction heap           */
                second = this->popDo();        /* pop the ENDTHEN item              */
                this->pushDo(_instruction);     /* add to the control list           */
                /* join the THEN and ELSE together   */
                ((RexxInstructionElse *)_instruction)->setParent((RexxInstructionEndIf *)second);
                ((RexxInstructionEndIf *)second)->setEndInstruction((RexxInstructionEndIf *)_instruction);
                token = nextReal();            /* get the next token                */
                                               /* have an ELSE keyword alone?       */
                if (token->isEndOfClause())
                {
                    this->nextClause();          /* get the next physical clause      */
                    if (this->flags&no_clause)   /* get an end-of-file?               */
                    {
                        /* raise an error                    */
                        syntaxError(Error_Incomplete_do_else, _instruction);
                    }
                }
                else                           /* ELSE instruction form             */
                {
                    previousToken();             /* step back a token                 */
                    trimClause();                /* make this start of the clause     */
                }
                continue;                      /* straight around to process clause */
                                               /* remainder                         */

            case  KEYWORD_OTHERWISE:         /* start of an OTHERWISE group       */
                second = this->topDo();        /* get the top of the queue          */
                                               /* not working on a SELECT?          */
                if (second->getType() != KEYWORD_SELECT)
                {
                    syntaxError(Error_Unexpected_when_otherwise);
                }
                /* hook up the OTHERWISE instruction */
                ((RexxInstructionSelect *)second)->setOtherwise((RexxInstructionOtherwise *)_instruction);
                this->pushDo(_instruction);     /* add this to the control queue     */
                token = nextReal();            /* get the next token                */
                                               /* OTHERWISE instr form?             */
                if (!token->isEndOfClause())
                {
                    previousToken();             /* step back a token                 */
                    trimClause();                /* make this start of the clause     */
                    continue;                    /* straight around to process clause */
                                                 /* remainder                         */
                }
                break;                         /* normal OTHERWISE processing       */


            case  KEYWORD_END:               /* END instruction for DO or SELECT  */
                second = this->popDo();        /* get the top of the queue          */
                type = second->getType();      /* get the instruction type          */
                                               /* not working on a block?           */
                if (type != KEYWORD_SELECT && type != KEYWORD_OTHERWISE && type != KEYWORD_DO && type != KEYWORD_LOOP)
                {
                    if (type == KEYWORD_ELSE)    /* on an else?                       */
                    {
                        /* give the specific error           */
                        syntaxError(Error_Unexpected_end_else);
                    }
                    else if (type == KEYWORD_IFTHEN || type == KEYWORD_WHENTHEN)
                    {
                        /* this is a different error         */
                        syntaxError(Error_Unexpected_end_then);
                    }
                    else
                    {
                        /* have a misplaced END              */
                        syntaxError(Error_Unexpected_end_nodo);
                    }
                }
                if (type == KEYWORD_OTHERWISE) /* OTHERWISE part of a SELECT?       */
                {
                    second = this->popDo();      /* need to pop one more item off     */
                }
                /* matching a select?                */
                if (second->getType() == KEYWORD_SELECT)
                {
                    /* match up the instruction          */
                    ((RexxInstructionSelect *)second)->matchEnd((RexxInstructionEnd *)_instruction, this);
                }
                else                           /* must be a DO block                */
                {
                    /* match up the instruction          */
                    ((RexxInstructionDo *)second)->matchEnd((RexxInstructionEnd *)_instruction, this);
                }
                this->flushControl(OREF_NULL); /* finish pending IFs or ELSEs       */
                break;

            case  KEYWORD_DO:                // start of new DO group (also picks up LOOP instruction)
            case  KEYWORD_LOOP:
                this->pushDo(_instruction);    /* add this to the control queue     */
                break;

            case  KEYWORD_SELECT:            /* start of new SELECT group         */
                this->pushDo(_instruction);    /* and also to the control queue     */
                break;

            default:                         /* other types of instruction        */
                break;
        }
        this->nextClause();                /* get the next physical clause      */
    }
    /* now go resolve any label targets  */
    _instruction = (RexxInstruction *)(this->calls->removeFirst());
    /* while still more references       */
    while (_instruction != (RexxInstruction *)TheNilObject)
    {
        /* actually a function call?         */
        if (isOfClass(FunctionCallTerm, _instruction))
        {
            /* resolve the function call         */
            ((RexxExpressionFunction *)_instruction)->resolve(this->labels);
        }
        else
        {
            /* resolve the CALL/SIGNAL/FUNCTION  */
            /* label targets                     */
            ((RexxInstructionCallBase *)_instruction)->resolve(this->labels);
        }
        /* now get the next instruction      */
        _instruction = (RexxInstruction *)(this->calls->removeFirst());
    }
    /* remove the first instruction      */
    OrefSet(this, this->first, this->first->nextInstruction);
    /* no labels needed?                 */
    if (this->labels != OREF_NULL && this->labels->items() == 0)
    {
        /* release that directory also       */
        OrefSet(this, this->labels, OREF_NULL);
    }
    /* create a rexx code object         */
    return new RexxCode(this, this->first, this->labels, (this->maxstack+ 10), this->variableindex);
}

RexxVariableBase *RexxSource::addVariable(
    RexxString *varname)               /* variable to add                   */
/******************************************************************************/
/* Function:  Resolve a variable name to a single common retriever object     */
/*            per method                                                      */
/******************************************************************************/
{
                                         /* check the directory for an entry  */
    RexxVariableBase *retriever = (RexxVariableBase *)this->variables->fastAt(varname);
    if (retriever == OREF_NULL)          /* not in the table yet?             */
    {
        if (!(this->flags&_interpret))     /* not in an interpret?              */
        {
            this->variableindex++;           /* step the counter                  */
                                             /* create a new variable retriever   */
            retriever = new RexxParseVariable(varname, this->variableindex);
        }
        else                               /* force dynamic lookup each time    */
        {
            retriever = new RexxParseVariable(varname, 0);
        }
        /* add to the variable table         */
        this->variables->put((RexxObject *)retriever, varname);
    }
    /* collecting guard variables?       */
    if (this->guard_variables != OREF_NULL)
    {
        /* in the list of exposed variables? */
        if (this->exposed_variables != OREF_NULL && this->exposed_variables->fastAt(varname) != OREF_NULL)
        {
            /* add this to the guard list        */
            this->guard_variables->put((RexxObject *)retriever, (RexxObject *)retriever);
        }
    }
    return retriever;                    /* return variable accesser          */
}

RexxStemVariable *RexxSource::addStem(
    RexxString *stemName)              /* stem to add                       */
/******************************************************************************/
/* Function:  Process creation of stem variables                              */
/******************************************************************************/
{
    /* check the table for an entry      */
    RexxStemVariable *retriever = (RexxStemVariable *)(this->variables->fastAt(stemName));
    if (retriever == OREF_NULL)          /* not in the table yet?             */
    {
        if (!(this->flags&_interpret))     /* not in an interpret?              */
        {
            this->variableindex++;           /* step the counter                  */
                                             /* create a new variable retriever   */
            retriever = new RexxStemVariable(stemName, this->variableindex);
        }
        else                               /* force dynamic lookup each time    */
        {
            retriever = new RexxStemVariable(stemName, 0);
        }
        /* add to the variable table         */
        this->variables->put((RexxObject *)retriever, stemName);
    }
    /* collecting guard variables?       */
    if (this->guard_variables != OREF_NULL)
    {
        /* in the list of exposed variables? */
        if (this->exposed_variables != OREF_NULL && this->exposed_variables->fastAt(stemName) != OREF_NULL)
        {
            /* add this to the guard list        */
            this->guard_variables->put((RexxObject *)retriever, (RexxObject *)retriever);
        }
    }
    return retriever;                    /* return variable accesser          */
}


RexxCompoundVariable *RexxSource::addCompound(
    RexxString *name)                  /* name of the compound variable     */
/******************************************************************************/
/* Function:  Parse to completion a compound variable                         */
/******************************************************************************/
{
    RexxStemVariable     *stemRetriever; /* retriever for the stem value      */
    RexxString           *stemName;      /* stem part of compound variable    */
    RexxString           *tail;          /* tail section string value         */
    const char *          start;         /* starting scan position            */
    size_t                length;        /* length of tail section            */
    const char *          _position;     /* current position                  */
    const char *          end;           // the end scanning position
    size_t                tailCount;     /* count of tails in compound        */

    length = name->getLength();          /* get the string length             */
    _position = name->getStringData();   /* start scanning at first character */
    start = _position;                   /* save the starting point           */
    end = _position + length;            // save our end marker

    // we know this is a compound, so there must be at least one period.
    /* scan to the first period          */
    while (*_position != '.')
    {
        _position++;                       /* step to the next character        */
    }
    /* get the stem string               */
    stemName = new_string(start, _position - start + 1);
    stemRetriever = this->addStem(stemName); /* get a retriever item for this     */

    tailCount = 0;                       /* no tails yet                      */
    do                                   /* process rest of the variable      */
    {
        // we're here because we just saw a previous period.  that's either the
        // stem variable period or the last tail element we processed.
        // either way, we step past it.  If this period is a trailing one,
        // we'll add a null tail element, which is exactly what we want.
        _position++;                       /* step past previous period         */
        start = _position;                 /* save the start position           */
                                           /* scan for the next period          */
        while (_position < end)
        {
            if (*_position == '.')         // found the next one?
            {
                break;                      // stop scanning now
            }
            _position++;                   // continue looking
        }
        /* extract the tail piece            */
        tail = new_string(start, _position - start);
        /* have a null tail piece or         */
        /* section begin with a digit?       */
        if (!(tail->getLength() == 0 || (*start >= '0' && *start <= '9')))
        {
            /* push onto the term stack          */
            this->subTerms->push((RexxObject *)(this->addVariable(tail)));
        }
        else
        {
            /* just use the string value directly*/
            this->subTerms->push(this->commonString(tail));
        }
        tailCount++;                       /* up the tail count                 */
    } while (_position < end);
    /* finally, create the compound var  */
    return new (tailCount) RexxCompoundVariable(stemName, stemRetriever->index, this->subTerms, tailCount);
}


void RexxSource::expose(
    RexxString *name )                 /* variable name to add to list      */
/******************************************************************************/
/* Function:  Add a variable name to the list of exposed variables for the    */
/*            method.                                                         */
/******************************************************************************/
{
                                       /* add to the exposed variables list */
    this->exposed_variables->put(name, name);
}


RexxString *RexxSource::commonString(
    RexxString *string )               /* string token to "collapse"        */
/******************************************************************************/
/* Function:  Compress all string tokens needed by a group of programs into   */
/*            a single, common set of strings.                                */
/******************************************************************************/
{
    /* check the global table first      */
    RexxString *result = (RexxString *)this->strings->fastAt(string);
    /* not in the table                  */
    if (result == OREF_NULL)
    {
        this->strings->put(string, string);/* add this to the table             */
        result = string;                   /* also the final value              */
    }
    return result;                       /* return the string                 */
}


RexxObject *RexxSource::addVariable(RexxToken *token)
{
    needVariable(token);
    return addText(token);
}


RexxObject *RexxSource::addText(
    RexxToken *token)                  /* token to process                  */
/******************************************************************************/
/* Function:  Generalized text token addition                                 */
/******************************************************************************/
{
    RexxObject       *retriever;         /* created retriever                 */
    RexxObject       *value;             /* evaluated literal value           */

    RexxString *name = token->value;     /* get the string value for this     */
    switch (token->classId)
    {

        case TOKEN_SYMBOL:                 /* various types of symbols          */
            /* each symbol subtype requires a    */
            /* different retrieval method        */
            switch (token->subclass)
            {

                case SYMBOL_DUMMY:             /* just a dot symbol                 */
                case SYMBOL_CONSTANT:          /* a literal symbol                  */

                    /* see if we've had this before      */
                    retriever = this->literals->fastAt(name);
                    /* first time literal?               */
                    if (retriever == OREF_NULL)
                    {
                        /* can we create an integer object?  */
                        if (token->numeric == INTEGER_CONSTANT)
                        {
                            /* create this as an integer         */
                            value = name->requestInteger(Numerics::DEFAULT_DIGITS);
                            /* conversion error?                 */
                            if (value == TheNilObject)
                            {
                                value = name;          /* just go with the string value     */
                            }
                            else
                                /* snip off the string number string */
                                /* value that was created when the   */
                                /* integer value was created.  This  */
                                /* is rarely used, but contributes   */
                                /* to the saved program size         */
                                name->setNumberString(OREF_NULL);
                        }
                        else
                        {
                            value = name;            /* just use the string value         */
                                                     /* give it a number string value     */
                            name->setNumberString((RexxObject *)value->numberString());
                        }
                        /* the constant is the retriever     */
                        this->literals->put(value, name);
                        retriever = value;         /* the retriever is the value itthis */
                    }
                    break;

                case SYMBOL_VARIABLE:          /* simple variable symbol            */
                    /* add variable to proper dictionary */
                    retriever = (RexxObject *)this->addVariable(name);
                    break;

                case SYMBOL_STEM:              /* stem variable                     */
                    /* add variable to proper dictionary */
                    retriever = (RexxObject *)this->addStem(name);
                    break;

                case SYMBOL_COMPOUND:          /* compound variable, need more      */
                    /* add variable to proper dictionary */
                    retriever = (RexxObject *)this->addCompound(name);
                    break;

                case SYMBOL_DOTSYMBOL:         /* variable with a leading dot       */
                    /* get a lookup object               */
                    /* see if we've had this before      */
                    retriever = this->variables->fastAt(name);
                    /* first time dot variable?          */
                    if (retriever == OREF_NULL)
                    {
                        /* create the shorter name           */
                        value = name->extract(1, name->getLength() - 1);
                        /* add this to the common pile       */
                        value = this->commonString((RexxString *)value);
                        /* create a retriever for this       */
                        retriever = (RexxObject *)new RexxDotVariable((RexxString *)value);
                        /* add this to the common table      */
                        this->variables->put(retriever, name);
                    }
                    break;

                default:                       /* all other types (shouldn't happen)*/
                    retriever = OREF_NULL;       /* return nothing                    */
                    break;
            }
            break;

        case TOKEN_LITERAL:                /* literal strings                   */
            /* get a lookup object               */
            /* see if we've had this before      */
            retriever = this->literals->fastAt(name);
            /* first time literal?               */
            if (retriever == OREF_NULL)
            {
                /* the constant is the retriever     */
                this->literals->put(name,  name);
                retriever = name;              /* use the name directly             */
            }
            break;

        default:                           /* all other tokens                  */
            retriever = OREF_NULL;           /* don't return anything             */
            break;
    }
    return retriever;                    /* return created retriever          */
}

RexxVariableBase *RexxSource::getRetriever(
    RexxString *name)                  /* name of the variable to process   */
/******************************************************************************/
/* Function:  Generalized method attribute retriever                          */
/******************************************************************************/
{
    RexxVariableBase *retriever = OREF_NULL; /* created retriever                 */

    /* go validate the symbol            */
    switch (name->isSymbol())
    {

        case STRING_NAME:                  /* valid simple name                 */
            /* get a simple dynamic retriever    */
            retriever = (RexxVariableBase *)new RexxParseVariable(name, 0);
            break;

        case STRING_STEM:                  /* this is a stem name               */
            /* force dynamic lookup each time    */
            retriever = (RexxVariableBase *)new RexxStemVariable(name, 0);
            break;

        case STRING_COMPOUND_NAME:         /* compound variable name            */
            /* get a direct retriever for this   */
            retriever = (RexxVariableBase *)RexxVariableDictionary::buildCompoundVariable(name, true);
            break;

        default:                           /* all other invalid cases           */
            /* have an invalid attribute         */
            syntaxError(Error_Translation_invalid_attribute, name);
    }
    return retriever;                    /* return created retriever          */
}


void RexxSource::addClause(
    RexxInstruction *_instruction)      /* new label to add                  */
/******************************************************************************/
/* Add an instruction to the tree code execution stream                       */
/******************************************************************************/
{
    /* is this the first one?            */
    if (this->first == OREF_NULL)
    {
        /* make this the first one           */
        OrefSet(this, this->first, _instruction);
        /* and the last one                  */
        OrefSet(this, this->last, _instruction);
    }
    /* non-root instruction              */
    else
    {
        this->last->setNext(_instruction);  /* add on to the last instruction    */
        /* this is the new last instruction  */
        OrefSet(this, this->last, _instruction);
    }
    /* now safe from garbage collection  */
    this->toss((RexxObject *)_instruction);
}


void RexxSource::addLabel(
    RexxInstruction      *label,       /* new label to add                  */
    RexxString           *labelname )  /* the label name                    */
/******************************************************************************/
/* Function:  add a label to the global label table.                          */
/******************************************************************************/
{
    /* not already in the table?         */
    if (this->labels->fastAt(labelname) == OREF_NULL)
    {
        /* add this item                     */
        this->labels->put((RexxObject *)label, labelname);
    }
}


RexxInstruction *RexxSource::findLabel(
    RexxString *labelname)             /* target label                      */
/******************************************************************************/
/* Search the label table for a label name match                              */
/******************************************************************************/
{
    if (this->labels != OREF_NULL)       /* have labels?                      */
    {
        /* just return entry from the table  */
        return(RexxInstruction *)this->labels->fastAt(labelname);
    }
    else
    {
        return OREF_NULL;                  /* don't return anything             */
    }
}

void RexxSource::setGuard()
/******************************************************************************/
/* Function:  Set on guard expression variable "gathering"                    */
/******************************************************************************/
{
    /* just starting to trap?            */
    if (this->guard_variables == OREF_NULL)
    {
        /* create the guard table            */
        OrefSet(this, this->guard_variables, new_identity_table());
    }
}

RexxArray *RexxSource::getGuard()
/******************************************************************************/
/* Function:  Complete guard expression variable collection and return the    */
/*            table of variables.                                             */
/******************************************************************************/
{
    /* convert into an array             */
    RexxArray *guards = this->guard_variables->makeArray();
    /* discard the table                 */
    OrefSet(this, this->guard_variables, OREF_NULL);
    /* just starting to trap?            */
    return guards;                       /* return the guards array           */
}

RexxObject *RexxSource::constantExpression()
/******************************************************************************/
/* Function:  Evaluate a "constant" expression for REXX instruction keyword   */
/*            values.  A constant expression is a literal string, constant    */
/*            symbol, or an expression enclosed in parentheses.               */
/******************************************************************************/
{
    RexxToken  *token;                   /* current token                     */
    RexxToken  *second;                  /* second token                      */
    RexxObject *_expression = OREF_NULL; /* parse expression                  */

    token = nextReal();                  /* get the first token               */
    if (token->isLiteral())              /* literal string expression?        */
    {
        _expression = this->addText(token); /* get the literal retriever         */
    }
    else if (token->isConstant())        /* how about a constant symbol?      */
    {
        _expression = this->addText(token); /* get the literal retriever         */
    }
    /* got an end of expression?         */
    else if (token->isEndOfClause())
    {
        previousToken();                   /* push the token back               */
        return OREF_NULL;                  /* nothing here (may be optional)    */
    }
    /* not a left paren here?            */
    else if (token->classId != TOKEN_LEFT)
    {
        /* this is an invalid expression     */
        syntaxError(Error_Invalid_expression_general, token);
    }
    else
    {
        /* get the subexpression             */
        _expression = this->subExpression(TERM_EOC | TERM_RIGHT);
        second = nextToken();              /* get the terminator token          */
                                           /* not terminated by a right paren?  */
        if (second->classId != TOKEN_RIGHT)
        {
            /* this is an error                  */
            syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
        }
    }
    this->holdObject(_expression);        /* protect the expression            */
    return _expression;                   /* and return it                     */
}

RexxObject *RexxSource::constantLogicalExpression()
/******************************************************************************/
/* Function:  Evaluate a "constant" expression for REXX instruction keyword   */
/*            values.  A constant expression is a literal string, constant    */
/*            symbol, or an expression enclosed in parentheses.  The          */
/*            expression inside parens can be a complex logical expression.   */
/******************************************************************************/
{
    RexxToken  *token;                   /* current token                     */
    RexxToken  *second;                  /* second token                      */
    RexxObject *_expression = OREF_NULL; /* parse expression                  */

    token = nextReal();                  /* get the first token               */
    if (token->isLiteral())              /* literal string expression?        */
    {

        _expression = this->addText(token); /* get the literal retriever         */
    }
    else if (token->isConstant())        /* how about a constant symbol?      */
    {
        _expression = this->addText(token); /* get the literal retriever         */
    }
    /* got an end of expression?         */
    else if (token->isEndOfClause())
    {
        previousToken();                   /* push the token back               */
        return OREF_NULL;                  /* nothing here (may be optional)    */
    }
    /* not a left paren here?            */
    else if (token->classId != TOKEN_LEFT)
    {
        /* this is an invalid expression     */
        syntaxError(Error_Invalid_expression_general, token);
    }
    else
    {
        /* get the subexpression             */
        _expression = this->parseLogical(token, TERM_EOC | TERM_RIGHT);
        second = nextToken();              /* get the terminator token          */
                                           /* not terminated by a right paren?  */
        if (second->classId != TOKEN_RIGHT)
        {
            /* this is an error                  */
            syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
        }
    }
    this->holdObject(_expression);        /* protect the expression            */
    return _expression;                   /* and return it                     */
}

RexxObject *RexxSource::parenExpression(RexxToken *start)
/******************************************************************************/
/* Function:  Evaluate a "parenthetical" expression for REXX instruction      */
/*            values.  A parenthetical expression is an expression enclosed   */
/*            in parentheses.                                                 */
/******************************************************************************/
{
  // NB, the opening paren has already been parsed off

  RexxObject *_expression = this->subExpression(TERM_EOC | TERM_RIGHT);
  RexxToken *second = nextToken();   /* get the terminator token          */
                                     /* not terminated by a right paren?  */
  if (second->classId != TOKEN_RIGHT)
  {
      syntaxErrorAt(Error_Unmatched_parenthesis_paren, start);
  }
                                     /* this is an error                  */
  this->holdObject(_expression);        /* protect the expression            */
  return _expression;                   /* and return it                     */
}

RexxObject *RexxSource::expression(
  int   terminators )                  /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off an expression, stopping when one of the possible set  */
/*            of terminator tokens is reached.  The terminator token is       */
/*            placed back on the token queue.                                 */
/******************************************************************************/
{
  nextReal();                          /* get the first real token          */
  previousToken();                     /* now put it back                   */
                                       /* parse off the subexpression       */
  return this->subExpression(terminators);
}

RexxObject *RexxSource::subExpression(
  int   terminators )                  /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off a sub- expression, stopping when one of the possible  */
/*            set of terminator tokens is reached.  The terminator token is   */
/*            placed back on the token queue.                                 */
/******************************************************************************/
{
    RexxObject    *left;                 /* left term of operation            */
    RexxObject    *right;                /* right term of operation           */
    RexxToken     *token;                /* current working token             */
    RexxToken     *second;               /* look ahead token                  */
    RexxObject    *subexpression;        /* final subexpression               */
    SourceLocation location;             /* token location info               */

                                         /* get the left term                 */
    left = this->messageSubterm(terminators);
    if (left == OREF_NULL)               /* end of the expression?            */
    {
        return OREF_NULL;                  /* done processing here              */
    }
    this->pushTerm(left);                /* add the term to the term stack    */
                                         /* add a fence item to operator stack*/
    this->pushOperator((RexxToken *)TheNilObject);
    token = nextToken();                 /* get the next token                */
                                         /* loop until end of expression      */
    while (!this->terminator(terminators, token))
    {
        switch (token->classId)
        {

            case  TOKEN_TILDE:               /* have a message send operation     */
            case  TOKEN_DTILDE:              /* have a double twiddle operation   */
                left = this->popTerm();        /* get the left term from the stack  */
                if (left == OREF_NULL)         /* not there?                        */
                {
                    /* this is an invalid expression     */
                    syntaxError(Error_Invalid_expression_general, token);
                }
                /* process a message term            */
                subexpression = this->message(left, token->classId == TOKEN_DTILDE, terminators);
                this->pushTerm(subexpression); /* push this back on the term stack  */
                break;

            case  TOKEN_SQLEFT:              /* collection syntax message         */
                left = this->popTerm();        /* get the left term from the stack  */
                if (left == OREF_NULL)         /* not there?                        */
                {
                    /* this is an invalid expression     */
                    syntaxError(Error_Invalid_expression_general, token);
                }
                /* process a message term            */
                subexpression = this->collectionMessage(token, left, terminators);
                this->pushTerm(subexpression); /* push this back on the term stack  */
                break;

            case  TOKEN_SYMBOL:              /* Symbol in the expression          */
            case  TOKEN_LITERAL:             /* Literal in the expression         */
            case  TOKEN_LEFT:                /* start of subexpression            */

                location = token->getLocation(); /* get the token start position      */
                                                 /* abuttal ends on the same line     */
                location.setEnd(location.getLineNumber(), location.getOffset());
                /* This is actually an abuttal       */
                token = new RexxToken (TOKEN_OPERATOR, OPERATOR_ABUTTAL, OREF_NULLSTRING, location);
                previousToken();               /* step back on the token list       */

            case  TOKEN_BLANK:               /* possible blank concatenate        */
                second = nextReal();           /* get the next token                */
                                               /* blank prior to a terminator?      */
                if (this->terminator(terminators, second))
                {
                    break;                       /* not a real operator               */
                }
                else                           /* have a blank operator             */
                {
                    previousToken();             /* push this back                    */
                }
                                                 /* fall through to operator logic    */

            case  TOKEN_OPERATOR:            /* have a dyadic operator            */
                /* actually a prefix only one?       */
                if (token->subclass == OPERATOR_BACKSLASH)
                {
                    /* this is an invalid expression     */
                    syntaxError(Error_Invalid_expression_general, token);
                }
                /* handle operator precedence        */
                for (;;)
                {
                    second = this->topOperator();/* get the top term                  */
                                                 /* hit the fence term?               */
                    if (second == (RexxToken *)TheNilObject)
                    {
                        break;                     /* out of here                       */
                    }
                                                   /* current have higher precedence?   */
                    if (this->precedence(token) > this->precedence(second))
                    {
                        break;                     /* finished also                     */
                    }
                    right = this->popTerm();     /* get the right term                */
                    left = this->popTerm();      /* and the left term                 */
                                                 /* not enough terms?                 */
                    if (right == OREF_NULL || left == OREF_NULL)
                    {
                        /* this is an invalid expression     */
                        syntaxError(Error_Invalid_expression_general, token);
                    }
                    /* create a new operation            */
                    RexxToken *op = popOperator();
                    subexpression = (RexxObject *)new RexxBinaryOperator(op->subclass, left, right);
                    /* push this back on the term stack  */
                    this->pushTerm(subexpression);
                }
                this->pushOperator(token);     /* push this operator onto stack     */
                right = this->messageSubterm(terminators);
                /* end of the expression?            */
                if (right == OREF_NULL && token->subclass != OPERATOR_BLANK)
                {
                    /* have a bad expression             */
                    syntaxError(Error_Invalid_expression_general, token);
                }
                this->pushTerm(right);         /* add the term to the term stack    */
                break;

            case TOKEN_ASSIGNMENT:
                // special assignment token in a bad context.  We report this as an error.
                /* this is an invalid expression     */
                syntaxError(Error_Invalid_expression_general, token);
                break;

            case TOKEN_COMMA:                /* found a comma in the expression   */
                /* should have been trapped as an    */
                /* expression terminator, so this is */
                /* not a valid expression            */
                syntaxError(Error_Unexpected_comma_comma);
                break;

            case TOKEN_RIGHT:                /* found a paren in the expression   */
                syntaxError(Error_Unexpected_comma_paren);
                break;

            case TOKEN_SQRIGHT:              /* found a bracket in the expression */
                syntaxError(Error_Unexpected_comma_bracket);
                break;

            default:                         /* something unexpected              */
                /* not a valid expression            */
                syntaxError(Error_Invalid_expression_general, token);
                break;
        }
        token = nextToken();               /* get the next token                */
    }
    token= this->popOperator();          /* get top operator token            */
                                         /* process pending operations        */
    while (token != (RexxToken *)TheNilObject)
    {
        right = this->popTerm();           /* get the right term                */
        left = this->popTerm();            /* now get the left term             */
                                           /* missing any terms?                */
        if (left == OREF_NULL || right == OREF_NULL)
        {
            /* this is an invalid expression     */
            syntaxError(Error_Invalid_expression_general, token);
        }
        /* create a new operation            */
        subexpression = (RexxObject *)new RexxBinaryOperator(token->subclass, left, right);
        this->pushTerm(subexpression);     /* push this back on the term stack  */
        token = this->popOperator();       /* get top operator token            */
    }
    return this->popTerm();              /* expression is top of term stack   */
}

RexxArray *RexxSource::argArray(
  RexxToken   *_first,                 /* token starting arglist            */
  int          terminators )           /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off an array of argument expressions                      */
/******************************************************************************/
{
    size_t     argCount;                 /* count of arguments                */
    RexxArray *_argArray;                 /* returned array                    */

    /* scan off the argument list        */
    argCount = this->argList(_first, terminators);
    _argArray = new_array(argCount);      /* get a new argument list           */
    /* now copy the argument pointers    */
    while (argCount > 0)
    {
        /* in reverse order                  */
        _argArray->put(this->subTerms->pop(), argCount--);
    }
    return _argArray;                     /* return the argument array         */
}

size_t RexxSource::argList(
  RexxToken   *_first,                  /* token starting arglist            */
  int          terminators )           /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off a list of argument expressions                        */
/******************************************************************************/
{
    RexxQueue    *arglist;               /* argument list                     */
    RexxObject   *subexpr;               /* current subexpression             */
    RexxToken    *token;                 /* current working token             */
    size_t        realcount;             /* count of real arguments           */
    size_t        total;                 /* total arguments                   */

    arglist = this->subTerms;            /* use the subterms list             */
    realcount = 0;                       /* no arguments yet                  */
    total = 0;
    /* get the first real token, which   */
    nextReal();                          /* skips any leading blanks on CALL  */
    previousToken();                     /* now put it back                   */
    /* loop until get a full terminator  */
    for (;;)
    {
        /* parse off next argument expression*/
        subexpr = this->subExpression(terminators | TERM_COMMA);
        arglist->push(subexpr);            /* add next argument to list         */
        this->pushTerm(subexpr);           /* add the term to the term stack    */
        total++;                           /* increment the total               */
        if (subexpr != OREF_NULL)          /* real expression?                  */
        {
            realcount = total;               /* update the real count             */
        }
        token = nextToken();               /* get the next token                */
        if (token->classId != TOKEN_COMMA) /* start of next argument?           */
        {
            break;                           /* no, all finished                  */
        }
    }
    /* not closed with expected ')'?     */
    if (terminators & TERM_RIGHT && token->classId != TOKEN_RIGHT)
    {
        /* raise an error                    */
        syntaxErrorAt(Error_Unmatched_parenthesis_paren, _first);
    }
    /* not closed with expected ']'?     */
    if (terminators&TERM_SQRIGHT && token->classId != TOKEN_SQRIGHT)
    {
        /* have an unmatched bracket         */
        syntaxErrorAt(Error_Unmatched_parenthesis_square, _first);
    }
    this->popNTerms(total);              /* pop all items off the term stack  */
    /* pop off any trailing omitteds     */
    while (total > realcount)
    {
        arglist->pop();                    /* just pop off the dummy            */
        total--;                           /* reduce the total                  */
    }
    return realcount;                    /* return the argument count         */
}

RexxObject *RexxSource::function(
    RexxToken     *token,              /* arglist start (for error reports) */
    RexxToken     *name,               /* function name                     */
    int            terminators )       /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off a REXX function call                                  */
/******************************************************************************/
{
  size_t        argCount;              /* count of function arguments       */
  RexxExpressionFunction *_function;    /* newly created function argument   */

  saveObject((RexxObject *)name);      // protect while parsing the argument list

                                       /* process the argument list         */
  argCount = this->argList(token, ((terminators | TERM_RIGHT) & ~TERM_SQRIGHT));

                                       /* create a new function item        */
  _function = new (argCount) RexxExpressionFunction(name->value, argCount, this->subTerms, this->resolveBuiltin(name->value), name->isLiteral());
                                       /* add to table of references        */
  this->addReference((RexxObject *)_function);
  removeObj((RexxObject *)name);       // end of protected windoww.
  return (RexxObject *)_function;      /* and return this to the caller     */
}

RexxObject *RexxSource::collectionMessage(
  RexxToken   *token,                  /* arglist start (for error reports) */
  RexxObject  *target,                 /* target term                       */
  int          terminators )           /* expression termination context    */
/******************************************************************************/
/* Function:  Process an expression term of the form "target[arg,arg]"        */
/******************************************************************************/
{
  size_t     argCount;                 /* count of function arguments       */
  RexxObject *_message;                /* new message term                  */

  this->saveObject((RexxObject *)target);   /* save target until it gets connected to message */
                                       /* process the argument list         */
  argCount = this->argList(token, ((terminators | TERM_SQRIGHT) & ~TERM_RIGHT));
                                       /* create a new function item        */
  _message = (RexxObject *)new (argCount) RexxExpressionMessage(target, (RexxString *)OREF_BRACKETS, (RexxObject *)OREF_NULL, argCount, this->subTerms, false);
  this->holdObject(_message);          /* hold this here for a while        */
  this->removeObj((RexxObject *)target);   /* target is now connected to message, remove from savelist without hold */
  return _message;                     /* return the message item           */
}

RexxToken  *RexxSource::getToken(
    int   terminators,                 /* expression termination context    */
    int   errorcode)                   /* expected error code               */
/******************************************************************************/
/* Function:  Get a token, checking to see if this is a terminatore token     */
/******************************************************************************/
{
    RexxToken *token = nextToken();                 /* get the next token                */
    /* this a terminator token?          */
    if (this->terminator(terminators, token))
    {
        if (errorcode != 0)                /* want an error raised?             */
        {
            syntaxError(errorcode);         /* report this                       */
        }
        return OREF_NULL;                  /* just return a null                */
    }
    return token;                        /* return the token                  */
}

RexxObject *RexxSource::message(
  RexxObject  *target,                 /* message send target               */
  bool         doubleTilde,            /* class of message send             */
  int          terminators )           /* expression termination context    */
/******************************************************************************/
/* Function:  Parse a full message send expression term                       */
/******************************************************************************/
{
    size_t        argCount;              /* list of function arguments        */
    RexxString   *messagename = OREF_NULL;  /* message name                      */
    RexxObject   *super;                 /* super class target                */
    RexxToken    *token;                 /* current working token             */
    RexxExpressionMessage *_message;     /* new message term                  */

    super = OREF_NULL;                   /* default no super class            */
    argCount = 0;                        /* and no arguments                  */
    this->saveObject(target);   /* save target until it gets connected to message */

    /* add the term to the term stack so that the calculations */
    /* include this in the processing. */
    this->pushTerm(target);
    /* get the next token                */
    token = this->getToken(terminators, Error_Symbol_or_string_tilde);
    /* unexpected type?                  */
    if (token->isSymbolOrLiteral())
    {
        messagename = token->value;        /* get the message name              */
    }
    else
    {
        /* error!                            */
        syntaxError(Error_Symbol_or_string_tilde);
    }
    /* get the next token                */
    token = this->getToken(terminators, 0);
    if (token != OREF_NULL)
    {            /* not reached the clause end?       */
                 /* have a super class?               */
        if (token->classId == TOKEN_COLON)
        {
            /* get the next token                */
            token = this->getToken(terminators, Error_Symbol_expected_colon);
            /* not a variable symbol?            */
            if (!token->isVariable() && token->subclass != SYMBOL_DOTSYMBOL)
            {
                /* have an error                     */
                syntaxError(Error_Symbol_expected_colon);
            }
            super = this->addText(token);    /* get the variable retriever        */
                                             /* get the next token                */
            token = this->getToken(terminators, 0);
        }
    }
    if (token != OREF_NULL)
    {            /* not reached the clause end?       */
        if (token->classId == TOKEN_LEFT)  /* have an argument list?            */
        {
            /* process the argument list         */
            argCount = this->argList(token, ((terminators | TERM_RIGHT) & ~TERM_SQRIGHT));
        }
        else
        {
            previousToken();                 /* something else, step back         */
        }
    }

    this->popTerm();                     /* it is now safe to pop the message target */
                                         /* create a message send node        */
    _message =  new (argCount) RexxExpressionMessage(target, messagename, super, argCount, this->subTerms, doubleTilde);
    /* protect for a bit                 */
    this->holdObject((RexxObject *)_message);
    this->removeObj(target);   /* target is now connected to message, remove from savelist without hold */
    return(RexxObject *)_message;        /* return the message item           */
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
RexxObject *RexxSource::variableOrMessageTerm()
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



RexxObject *RexxSource::messageTerm()
/******************************************************************************/
/* Function:  Parse off an instruction leading message term element           */
/******************************************************************************/
{
    RexxToken   *token;                  /* current working token             */
    RexxObject  *term;                   /* working term                      */
    RexxObject  *start;                  /* starting term                     */
    int          classId;                /* token class                       */

    size_t mark = markPosition();       // save the current position so we can reset cleanly

    start = this->subTerm(TERM_EOC);     /* get the first term of instruction */
    this->holdObject(start);             /* save the starting term            */
    term = OREF_NULL;                    /* default to no term                */
    token = nextToken();                 /* get the next token                */
    classId = token->classId;            /* get the token class               */
                                         /* while cascading message sends     */
    while (classId == TOKEN_TILDE || classId == TOKEN_DTILDE || classId == TOKEN_SQLEFT )
    {
        if (classId == TOKEN_SQLEFT)       /* left bracket form?                */
        {
            term = this->collectionMessage(token, start, TERM_EOC);
        }
        else
        {
            /* process a message term            */
            term = this->message(start, classId == TOKEN_DTILDE, TERM_EOC);
        }
        start = term;                      /* set for the next pass             */
        token = nextToken();               /* get the next token                */
        classId = token->classId;          /* get the token class               */
    }
    previousToken();                     /* push this term back               */
    // if this was not a valid message term, reset the position to the beginning
    if (term == OREF_NULL)
    {
        resetPosition(mark);                 // reset back to the entry conditions
    }
    /* return the message term (returns  */
    return term;                         /* OREF_NULL if not a message term)  */
}

RexxObject *RexxSource::messageSubterm(
  int   terminators )                  /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off a message subterm within an expression                */
/******************************************************************************/
{
    RexxToken   *token;                  /* current working token             */
    RexxObject  *term = OREF_NULL;       /* working term                      */
    int          classId;                /* token class                       */

    token = nextToken();                 /* get the next token                */
                                         /* this the expression end?          */
    if (this->terminator(terminators, token))
    {
        return OREF_NULL;                  /* nothing to do here                */
    }
                                           /* have potential prefix operator?   */
    if (token->classId == TOKEN_OPERATOR)
    {

        /* handle prefix operators as terms  */
        switch (token->subclass)
        {

            case OPERATOR_PLUS:              /* prefix plus                       */
            case OPERATOR_SUBTRACT:          /* prefix minus                      */
            case OPERATOR_BACKSLASH:         /* prefix backslash                  */
                /* handle following term             */
                term = this->messageSubterm(terminators);
                if (term == OREF_NULL)         /* nothing found?                    */
                {
                    /* this is an error                  */
                    syntaxError(Error_Invalid_expression_prefix, token);
                }
                /* create the new operator term      */
                term = (RexxObject *)new RexxUnaryOperator(token->subclass, term);
                break;

            default:                         /* other operators not allowed here  */
                /* this is an error                  */
                syntaxError(Error_Invalid_expression_general, token);
        }
    }
    /* non-prefix operator code          */
    else
    {
        previousToken();                   /* put back the first token          */
        term = this->subTerm(TERM_EOC);    /* get the first term of instruction */
        this->holdObject(term);            /* save the starting term            */
        token = nextToken();               /* get the next token                */
        classId = token->classId;          /* get the token class               */
                                           /* while cascading message sends     */
        while (classId == TOKEN_TILDE || classId == TOKEN_DTILDE || classId == TOKEN_SQLEFT )
        {
            if (classId == TOKEN_SQLEFT)     /* left bracket form?                */
            {
                term = this->collectionMessage(token, term, TERM_EOC);
            }
            else
            {
                /* process a message term            */
                term = this->message(term, classId == TOKEN_DTILDE, TERM_EOC);
            }
            token = nextToken();             /* get the next token                */
            classId = token->classId;        /* get the token class               */
        }
        previousToken();                   /* push this term back               */
    }
    /* return the message term (returns  */
    return term;                         /* OREF_NULL if not a message term)  */
}

RexxObject *RexxSource::subTerm(
  int   terminators )                  /* expression termination context    */
/******************************************************************************/
/* Function:  Parse off a subterm of an expression, from simple ones like     */
/*            variable names, to more complex such as message sends           */
/******************************************************************************/
{
    RexxToken    *token;                 /* current token being processed     */
    RexxObject   *term = OREF_NULL;      /* parsed out term                   */
    RexxToken    *second;                /* second token of term              */

    token = nextToken();                 /* get the next token                */
                                         /* this the expression end?          */
    if (this->terminator(terminators, token))
    {
        return OREF_NULL;                  /* nothing to do here                */
    }

    switch (token->classId)
    {

        case  TOKEN_LEFT:                  /* have a left parentheses           */
            /* get the subexpression             */
            term = this->subExpression(((terminators | TERM_RIGHT) & ~TERM_SQRIGHT));
            if (term == OREF_NULL)           /* nothing found?                    */
            {
                /* this is an error                  */
                syntaxError(Error_Invalid_expression_general, token);
            }
            second = nextToken();            /* get the terminator token          */
                                             /* not terminated by a right paren?  */
            if (second->classId != TOKEN_RIGHT)
            {
                /* this is an error                  */
                syntaxErrorAt(Error_Unmatched_parenthesis_paren, token);
            }
            break;

        case  TOKEN_SYMBOL:                /* Symbol in the expression          */
        case  TOKEN_LITERAL:               /* Literal in the expression         */
            second = nextToken();            /* get the next token                */
                                             /* have a function call?             */
            if (second->classId == TOKEN_LEFT)
            {
                /* process the function call         */
                term = this->function(second, token, terminators);
            }
            else
            {
                previousToken();               /* push the token back               */
                term = this->addText(token);   /* variable or literal access        */
            }
            break;

        case  TOKEN_RIGHT:                 /* have a right parentheses          */
            /* this is an error here             */
            syntaxError(Error_Unexpected_comma_paren);
            break;

        case  TOKEN_COMMA:                 /* have a comma                      */
            /* this is an error here             */
            syntaxError(Error_Unexpected_comma_comma);
            break;

        case  TOKEN_SQRIGHT:               /* have a right square bracket       */
            /* this is an error here             */
            syntaxError(Error_Unexpected_comma_bracket);
            break;

        case  TOKEN_OPERATOR:              /* operator token                    */
            switch (token->subclass)
            {       /* handle prefix operators as terms  */

                case OPERATOR_PLUS:            /* prefix plus                       */
                case OPERATOR_SUBTRACT:        /* prefix minus                      */
                case OPERATOR_BACKSLASH:       /* prefix backslash                  */
                    previousToken();             /* put the token back                */
                    return OREF_NULL;            /* just return null (processed later)*/

                default:                       /* other operators not allowed here  */
                    /* this is an error                  */
                    syntaxError(Error_Invalid_expression_general, token);
            }
            break;

        default:                           /* unknown thing in expression       */
            /* this is an error                  */
            syntaxError(Error_Invalid_expression_general, token);
    }
    return term;                         /* return this term                  */
}

void RexxSource::pushTerm(
    RexxObject *term )                 /* term to push                      */
/******************************************************************************/
/* Function:  Push a term onto the expression term stack                      */
/******************************************************************************/
{
    this->terms->push(term);             /* push the term on the stack      */
    this->currentstack++;                /* step the stack depth              */
                                         /* new "high water" mark?            */
    if (this->currentstack > this->maxstack)
    {
        /* make it the highest point         */
        this->maxstack = this->currentstack;
    }
}

RexxObject *RexxSource::popTerm()
/******************************************************************************/
/* Function:  Pop a term off of the expression term stack                     */
/******************************************************************************/
{
  RexxObject *term;                    /* returned term                   */

  this->currentstack--;                /* reduce the size count           */
  term = this->terms->pop();           /* pop the term                    */
  this->holdObject(term);              /* give it a little protection     */
  return term;                         /* and return it                   */
}

RexxObject *RexxSource::popNTerms(
     size_t count )                    /* number of terms to pop            */
/******************************************************************************/
/* Function:  Pop multiple terms off of the operator stack                    */
/******************************************************************************/
{
    RexxObject *result = OREF_NULL;                  /* final popped element              */

    this->currentstack -= count;         /* reduce the size count             */
    while (count--)                      /* while more to remove              */
    {
        result = this->terms->pop();       /* pop the next item               */
    }
    this->holdObject(result);            /* protect this a little             */
    return result;                       /* and return it                     */
}

void RexxSource::isExposeValid()
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

RexxArray  *RexxSource::words(
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

void RexxSource::errorCleanup()
/******************************************************************************/
/* Function:  Free up all of the parsing elements because of an error         */
/******************************************************************************/
{
  this->cleanup();                     /* do needed cleanup                 */
}

void RexxSource::error(int errorcode)
/******************************************************************************/
/* Function:  Raise an error caused by source translation problems.           */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, OREF_NULL, OREF_NULL);
}

void RexxSource::error(int errorcode, SourceLocation &location, RexxArray *subs)
/******************************************************************************/
/* Function:  Raise an error caused by source translation problems.           */
/******************************************************************************/
{
  this->errorCleanup();                /* release any saved objects         */
  clauseLocation = location;           // set the error location
                                       /* pass on the exception info        */
  ActivityManager::currentActivity->raiseException(errorcode, OREF_NULL, subs, OREF_NULL);
}

void RexxSource::errorLine(
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

void RexxSource::errorPosition(
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

void RexxSource::errorToken(
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

void RexxSource::error(
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

void RexxSource::error(
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

void RexxSource::error(
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

void RexxSource::blockError(
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
            switch (this->subKeyword(token) )
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
